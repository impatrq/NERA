/** BLE del reloj: anuncia NERA Watch y notifica métricas ópticas y objeto IR.
 * El contrato JSON es pequeño para caber incluso con el MTU BLE mínimo.
 * No publicar una métrica cuando sus muestras no son válidas. */
#include "connectivity/ble_manager.h"
#include "connectivity/user_alert_preferences.h"
#include "connectivity/alert_delivery.h"

#include <atomic>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "sensor_service.h"
#include "services/history_service.h"
#include "services/time_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "utils/logger.h"

extern "C" void ble_store_config_init(void);
namespace {
constexpr char DEVICE_NAME[] = "NERA Watch"; // FUNCIONAL: mismo nombre que la app.
// FUNCIONAL: UUID en orden little-endian, equivalentes a los de watch_ble_service.dart.
const ble_uuid128_t SERVICE_UUID = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x7e);
const ble_uuid128_t DATA_UUID = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x7e);
const ble_uuid128_t CONFIG_UUID = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x04, 0x00, 0x40, 0x7e);
const ble_uuid128_t TIME_UUID = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x05, 0x00, 0x40, 0x7e);
const ble_uuid128_t HISTORY_UUID = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x06, 0x00, 0x40, 0x7e);
const ble_uuid128_t HISTORY_CONTROL_UUID = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x07, 0x00, 0x40, 0x7e);
const ble_uuid128_t ALERT_UUID = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x08, 0x00, 0x40, 0x7e);
std::atomic<bool> alert_subscribed{false};
uint16_t alert_handle=0;

std::atomic<bool> ready{false};
std::atomic<bool> subscribed{false};
std::atomic<bool> encrypted{false};
std::atomic<bool> history_subscribed{false};
std::atomic<bool> history_requested{false};
std::atomic<bool> history_backfill_pending{false};
std::atomic<uint32_t> history_cursor{0};
std::atomic<uint16_t> connection{BLE_HS_CONN_HANDLE_NONE};
uint16_t data_handle = 0;
uint16_t history_handle = 0;
uint8_t address_type = 0;
ble_gatt_chr_def characteristics[7] = {};
ble_gatt_svc_def services[2] = {};

int data_access(uint16_t, uint16_t, ble_gatt_access_ctxt *ctxt, void *)
{
    // Solo notificaciones. La UI/app no escribe comandos en esta etapa.
    return ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR
        ? BLE_ATT_ERR_READ_NOT_PERMITTED : BLE_ATT_ERR_UNLIKELY;
}

int alert_access(uint16_t, uint16_t, ble_gatt_access_ctxt *ctxt, void *) {
    if (ctxt->op!=BLE_GATT_ACCESS_OP_WRITE_CHR) return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
    if (OS_MBUF_PKTLEN(ctxt->om)!=NERA_LAB_SIZE) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    uint8_t bytes[NERA_LAB_SIZE]; nera_lab_packet receipt={};
    if (os_mbuf_copydata(ctxt->om,0,sizeof(bytes),bytes)!=0 ||
        !nera_lab_decode(bytes,sizeof(bytes),&receipt)) return BLE_ATT_ERR_VALUE_NOT_ALLOWED;
    return nera_delivery::receive(receipt,esp_timer_get_time()) ? 0 : BLE_ATT_ERR_VALUE_NOT_ALLOWED;
}

// FUNCIONAL: leer confirma la configuracion ya persistida y aplicada.
int config_access(uint16_t, uint16_t, ble_gatt_access_ctxt *ctxt, void *) {
    using namespace nera_preferences;
    if (ctxt->op==BLE_GATT_ACCESS_OP_READ_CHR) {
        const Config c=get();
        const uint8_t bytes[]={c.version,c.enabled,c.sensitivity,c.exercise,uint8_t(saving())};
        return os_mbuf_append(ctxt->om,bytes,sizeof(bytes))==0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    if (ctxt->op==BLE_GATT_ACCESS_OP_WRITE_CHR) {
        if (OS_MBUF_PKTLEN(ctxt->om)!=4) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        uint8_t b[4];
        if (os_mbuf_copydata(ctxt->om,0,sizeof(b),b)!=0) return BLE_ATT_ERR_UNLIKELY;
        const Config c={b[0],b[1],b[2],b[3]};
        if (!valid(c)) return BLE_ATT_ERR_VALUE_NOT_ALLOWED;
        return request(c)==ESP_OK ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

int time_access(uint16_t, uint16_t, ble_gatt_access_ctxt *ctxt, void *) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        if (OS_MBUF_PKTLEN(ctxt->om) != 7) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        uint8_t bytes[7];
        if (os_mbuf_copydata(ctxt->om, 0, sizeof(bytes), bytes) != 0)
            return BLE_ATT_ERR_UNLIKELY;
        if (bytes[0] != 1) return BLE_ATT_ERR_VALUE_NOT_ALLOWED;
        const uint32_t epoch = uint32_t(bytes[1]) | (uint32_t(bytes[2]) << 8) |
                               (uint32_t(bytes[3]) << 16) | (uint32_t(bytes[4]) << 24);
        const int16_t offset = int16_t(uint16_t(bytes[5]) | (uint16_t(bytes[6]) << 8));
        if (time_service_sync(epoch, offset) != ESP_OK) return BLE_ATT_ERR_VALUE_NOT_ALLOWED;
        history_backfill_pending.store(true);
        return 0;
    }
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        const uint32_t epoch = time_service_epoch();
        const uint16_t offset = static_cast<uint16_t>(time_service_utc_offset_minutes());
        const uint8_t bytes[] = {1, uint8_t(epoch), uint8_t(epoch >> 8),
            uint8_t(epoch >> 16), uint8_t(epoch >> 24), uint8_t(offset), uint8_t(offset >> 8)};
        return os_mbuf_append(ctxt->om, bytes, sizeof(bytes)) == 0
            ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

int history_control_access(uint16_t, uint16_t, ble_gatt_access_ctxt *ctxt, void *) {
    if (!history_service_is_ready()) return BLE_ATT_ERR_UNLIKELY;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        const uint32_t latest = history_service_latest_sequence();
        const uint32_t generation = history_service_generation();
        const uint8_t bytes[] = {uint8_t(latest), uint8_t(latest >> 8),
            uint8_t(latest >> 16), uint8_t(latest >> 24),
            uint8_t(generation), uint8_t(generation >> 8),
            uint8_t(generation >> 16), uint8_t(generation >> 24)};
        return os_mbuf_append(ctxt->om, bytes, sizeof(bytes)) == 0
            ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        if (OS_MBUF_PKTLEN(ctxt->om) != 4) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        uint8_t bytes[4];
        if (os_mbuf_copydata(ctxt->om, 0, sizeof(bytes), bytes) != 0)
            return BLE_ATT_ERR_UNLIKELY;
        const uint32_t cursor = uint32_t(bytes[0]) | (uint32_t(bytes[1]) << 8) |
                                (uint32_t(bytes[2]) << 16) | (uint32_t(bytes[3]) << 24);
        history_cursor.store(cursor);
        history_requested.store(true);
        history_backfill_pending.store(true);
        ESP_LOGI("NERA/HISTORY", "Sincronizacion solicitada desde secuencia %lu",
                 static_cast<unsigned long>(cursor));
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

int history_access(uint16_t, uint16_t, ble_gatt_access_ctxt *, void *) {
    return BLE_ATT_ERR_READ_NOT_PERMITTED;
}

int start_advertising()
{
    ble_hs_adv_fields fields = {};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = reinterpret_cast<const uint8_t *>(DEVICE_NAME);
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        NERA_LOGW(NERA_TAG_BLE, "No se pudo preparar anuncio BLE: %d", rc);
        return rc;
    }

    // FUNCIONAL: identificar por servicio aunque Android no entregue el nombre.
    // Nombre + flags + UUID de 128 bits ocuparian 32 bytes (limite: 31).
    // Conservamos el nombre en el anuncio y el servicio en la respuesta de escaneo.
    ble_hs_adv_fields response = {};
    response.uuids128 = &SERVICE_UUID;
    response.num_uuids128 = 1;
    response.uuids128_is_complete = 1;
    rc = ble_gap_adv_rsp_set_fields(&response);
    if (rc != 0) {
        NERA_LOGW(NERA_TAG_BLE, "No se pudo preparar respuesta BLE: %d", rc);
        return rc;
    }

    ble_gap_adv_params params = {};
    params.conn_mode = BLE_GAP_CONN_MODE_UND;
    params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(address_type, nullptr, BLE_HS_FOREVER,
                           &params, [](ble_gap_event *event, void *) -> int {
        switch (event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            if (event->connect.status == 0) {
                alert_subscribed.store(false);
                nera_delivery::set_link(false);
                connection.store(event->connect.conn_handle);
                subscribed.store(false);
                encrypted.store(false);
                history_subscribed.store(false);
                history_requested.store(false);
                NERA_LOGI(NERA_TAG_BLE, "Telefono conectado");
                const int security_rc = ble_gap_security_initiate(event->connect.conn_handle);
                if (security_rc != 0 && security_rc != BLE_HS_EALREADY) {
                    NERA_LOGW(NERA_TAG_BLE, "Inicio de cifrado fallo: %d", security_rc);
                    ble_gap_terminate(event->connect.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
                }
            } else {
                start_advertising();
            }
            return 0;
        }
        case BLE_GAP_EVENT_DISCONNECT:
            alert_subscribed.store(false);
            nera_delivery::set_link(false);
            subscribed.store(false);
            encrypted.store(false);
            history_subscribed.store(false);
            history_requested.store(false);
            connection.store(BLE_HS_CONN_HANDLE_NONE);
            NERA_LOGI(NERA_TAG_BLE, "Telefono desconectado; anunciando de nuevo");
            start_advertising();
            return 0;
        case BLE_GAP_EVENT_SUBSCRIBE:
            if (event->subscribe.attr_handle==alert_handle) {
                alert_subscribed.store(event->subscribe.cur_notify!=0);
                nera_delivery::set_link(alert_subscribed.load() && encrypted.load());
            }
            if (event->subscribe.attr_handle == data_handle) {
                subscribed.store(event->subscribe.cur_notify != 0);
                NERA_LOGI(NERA_TAG_BLE, "Notificaciones %s",
                          subscribed.load() ? "activadas" : "desactivadas");
            }
            if (event->subscribe.attr_handle == history_handle)
                history_subscribed.store(event->subscribe.cur_notify != 0);
            return 0;
        case BLE_GAP_EVENT_ENC_CHANGE: {
            ble_gap_conn_desc desc = {};
            const bool secure = event->enc_change.status == 0 &&
                ble_gap_conn_find(event->enc_change.conn_handle, &desc) == 0 &&
                desc.sec_state.encrypted;
            encrypted.store(secure);
            nera_delivery::set_link(secure && alert_subscribed.load());
            NERA_LOGI(NERA_TAG_BLE, "Enlace cifrado: %s", secure ? "si" : "no");
            if (!secure) ble_gap_terminate(event->enc_change.conn_handle,
                                           BLE_ERR_REM_USER_CONN_TERM);
            return 0;
        }
        case BLE_GAP_EVENT_ADV_COMPLETE:
            start_advertising();
            return 0;
        default:
            return 0;
        }
    }, nullptr);
    if (rc == 0) {
        NERA_LOGI(NERA_TAG_BLE, "Anunciando %s; BUSQUEDA BLE V2 con servicio NERA", DEVICE_NAME);
    } else {
        NERA_LOGW(NERA_TAG_BLE, "No se pudo anunciar BLE: %d", rc);
    }
    return rc;
}

void on_sync()
{
    int rc = ble_hs_util_ensure_addr(0);
    if (rc == 0) rc = ble_hs_id_infer_auto(0, &address_type);
    if (rc != 0) {
        NERA_LOGE(NERA_TAG_BLE, "Sin direccion BLE: %d", rc);
        return;
    }
    start_advertising();
}

void on_reset(int reason)
{
    alert_subscribed.store(false);
    nera_delivery::set_link(false);
    subscribed.store(false);
    encrypted.store(false);
    history_subscribed.store(false);
    history_requested.store(false);
    connection.store(BLE_HS_CONN_HANDLE_NONE);
    NERA_LOGW(NERA_TAG_BLE, "Stack BLE reiniciado: %d", reason);
}

void host_task(void *)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

bool notify_packet(uint16_t peer, const char *payload)
{
    const size_t length = strlen(payload);
    // Cada JSON debe entrar completo en una notificación, incluso con MTU 23.
    if (length == 0 || length > 20 || length > ble_att_mtu(peer) - 3) return false;
    os_mbuf *message = ble_hs_mbuf_from_flat(payload, length);
    if (message == nullptr) return false;
    const int rc = ble_gatts_notify_custom(peer, data_handle, message);
    if (rc != 0) NERA_LOGW(NERA_TAG_BLE, "Notificacion fallo: %d", rc);
    return rc == 0;
}

bool notify_history(uint16_t peer, const HistoryPoint &point) {
    if (ble_att_mtu(peer) - 3 < sizeof(point)) return false;
    os_mbuf *message = ble_hs_mbuf_from_flat(&point, sizeof(point));
    if (!message) return false;
    const int rc = ble_gatts_notify_custom(peer, history_handle, message);
    return rc == 0;
}
} // namespace

esp_err_t ble_manager_init(void)
{
    if (ready.load()) return ESP_OK;
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) return err;
    err = nera_preferences::init();
    if (err != ESP_OK) return err;
    err = nimble_port_init();
    if (err != ESP_OK) return err;

    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;
    // Just Works: cifra y conserva vínculo; no autentica por MITM.
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_store_config_init();
    ble_svc_gap_init();
    ble_svc_gatt_init();
    int rc = ble_svc_gap_device_name_set(DEVICE_NAME);
    if (rc != 0) {
        nimble_port_deinit();
        return ESP_FAIL;
    }

    characteristics[0].uuid = &DATA_UUID.u;
    characteristics[0].access_cb = data_access;
    characteristics[0].flags = BLE_GATT_CHR_F_NOTIFY |
                               BLE_GATT_CHR_F_NOTIFY_INDICATE_ENC;
    characteristics[0].val_handle = &data_handle;
    characteristics[1].uuid = &CONFIG_UUID.u;
    characteristics[1].access_cb = config_access;
    characteristics[1].flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
        BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_WRITE_ENC;
    characteristics[2].uuid = &TIME_UUID.u;
    characteristics[2].access_cb = time_access;
    characteristics[2].flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
        BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_WRITE_ENC;
    characteristics[3].uuid = &HISTORY_UUID.u;
    characteristics[3].access_cb = history_access;
    characteristics[3].flags = BLE_GATT_CHR_F_NOTIFY |
                               BLE_GATT_CHR_F_NOTIFY_INDICATE_ENC;
    characteristics[3].val_handle = &history_handle;
    characteristics[4].uuid = &HISTORY_CONTROL_UUID.u;
    characteristics[4].access_cb = history_control_access;
    characteristics[4].flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
        BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_WRITE_ENC;
    characteristics[5].uuid=&ALERT_UUID.u;
    characteristics[5].access_cb=alert_access;
    characteristics[5].flags=BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_WRITE |
        BLE_GATT_CHR_F_NOTIFY_INDICATE_ENC | BLE_GATT_CHR_F_WRITE_ENC;
    characteristics[5].val_handle=&alert_handle;
    services[0].type = BLE_GATT_SVC_TYPE_PRIMARY;
    services[0].uuid = &SERVICE_UUID.u;
    services[0].characteristics = characteristics;
    rc = ble_gatts_count_cfg(services);
    if (rc == 0) rc = ble_gatts_add_svcs(services);
    if (rc != 0) {
        nimble_port_deinit();
        NERA_LOGE(NERA_TAG_BLE, "No se pudo crear servicio GATT: %d", rc);
        return ESP_FAIL;
    }

    nimble_port_freertos_init(host_task);
    ready.store(true);
    NERA_LOGI(NERA_TAG_BLE, "Servidor GATT listo; notificaciones cifradas");
    return ESP_OK;
}

bool ble_manager_is_ready(void) { return ready.load(); }

void ble_manager_task(void *arg)
{
    (void)arg;
    bool published_temp = false;
    bool published_heart = false;
    bool published_spo2 = false;
    int64_t next_live_us = 0;
    int64_t next_alert_us=0;
    while (true) {
        const uint16_t peer = connection.load();
        const int64_t now = esp_timer_get_time();
        if (now>=next_alert_us && ready.load() && alert_subscribed.load() &&
            encrypted.load() && peer!=BLE_HS_CONN_HANDLE_NONE) {
            next_alert_us=now+500000;
            for (unsigned metric=1;metric<=2;++metric) {
                nera_lab_packet event={};
                if (!nera_delivery::snapshot(metric,event)) continue;
                uint8_t bytes[NERA_LAB_SIZE]; nera_lab_encode(bytes,&event);
                os_mbuf *message=ble_hs_mbuf_from_flat(bytes,sizeof(bytes));
                if (message) ble_gatts_notify_custom(peer,alert_handle,message);
            }
        }
        if (ready.load() && history_requested.load() && history_subscribed.load() &&
            encrypted.load() && peer != BLE_HS_CONN_HANDLE_NONE && history_handle != 0) {
            if (history_backfill_pending.exchange(false))
                history_service_backfill_current_boot();
            HistoryPoint point = {};
            if (history_service_get_after(history_cursor.load(), &point) &&
                notify_history(peer, point))
                history_cursor.store(point.sequence);
        }
        if (now >= next_live_us && ready.load() && subscribed.load() && encrypted.load() &&
            peer != BLE_HS_CONN_HANDLE_NONE && data_handle != 0) {
            next_live_us = now + 1000000;
            SensorSnapshot snapshot = {};
            if (sensor_service_get_snapshot(&snapshot) == ESP_OK) {
                char payload[21];
                if (snapshot.mlx_valid && isfinite(snapshot.object_celsius) &&
                    snapshot.object_celsius >= -70 && snapshot.object_celsius <= 380) {
                    const int length = snprintf(payload, sizeof(payload), "{\"temp\":%.2f}",
                                                snapshot.object_celsius);
                    if (length > 0 && length < sizeof(payload) && notify_packet(peer, payload))
                        published_temp = true;
                } else if (published_temp && notify_packet(peer, "{\"temp\":null}")) {
                    published_temp = false;
                }
                if (snapshot.heart_valid && isfinite(snapshot.bpm) &&
                    snapshot.bpm >= 30 && snapshot.bpm <= 240) {
                    const int length = snprintf(payload, sizeof(payload), "{\"hr\":%d}",
                                                (int)lroundf(snapshot.bpm));
                    if (length > 0 && length < sizeof(payload) && notify_packet(peer, payload))
                        published_heart = true;
                } else if (published_heart && notify_packet(peer, "{\"hr\":null}")) {
                    published_heart = false;
                }
                if (snapshot.spo2_valid && isfinite(snapshot.spo2) &&
                    snapshot.spo2 >= 70 && snapshot.spo2 <= 100) {
                    const int length = snprintf(payload, sizeof(payload), "{\"spo2\":%d}",
                                                (int)lroundf(snapshot.spo2));
                    if (length > 0 && length < sizeof(payload) && notify_packet(peer, payload))
                        published_spo2 = true;
                } else if (published_spo2 && notify_packet(peer, "{\"spo2\":null}")) {
                    published_spo2 = false;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
