#include "connectivity/espnow_diagnostic.h"

#include <stdint.h>
#include <string.h>
#include <math.h>
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "sensor_service.h"
#include "connectivity/lab_alert_rules.h"
#include "connectivity/user_alert_preferences.h"
#include "connectivity/alert_delivery.h"
#include "../../../Protocolo Reloj Vincha/nera_lab_protocol.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#include "freertos/task.h"

namespace {
constexpr char TAG[] = "NERA/RADIO";
constexpr uint8_t BROADCAST[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
constexpr uint8_t CHANNEL = 1;       // MODIFICABLE: igual que en la vincha.
constexpr uint32_t PERIOD_MS = 5000; // MODIFICABLE: intervalo de prueba.
uint16_t sequence = 0;
// MODIFICABLE: MAC de la vincha comprobada en su monitor. Cambiar si cambia el C3.
constexpr uint8_t VINCHA_MAC[6]={0x30,0xed,0xa0,0x65,0xf9,0x68};
constexpr unsigned MAX_ATTEMPTS=3;
constexpr int64_t ACK_WAIT_US=1000000;
QueueHandle_t ack_queue=nullptr;
bool radio_ready=false;
struct Pending {
    nera_lab_packet packet={};
    bool waiting=false;
    unsigned attempts=0;
    int64_t next_send=0;
    bool confirmed=false, failed=false;
};

void receive_ack(const esp_now_recv_info_t *info, const uint8_t *bytes, int size) {
    nera_lab_packet ack={};
    if (!info || !bytes || size<0 || memcmp(info->src_addr,VINCHA_MAC,6)!=0 ||
        !nera_lab_decode(bytes,(size_t)size,&ack) || ack.type!=NERA_LAB_ACK) return;
    xQueueSend(ack_queue,&ack,0); // Callback: solo copiar; procesar en la tarea.
}

bool fresh(bool ok, int64_t sample, int64_t now) {
    return ok && sample>0 && now>=sample && now-sample<=nera_lab::FRESH_US;
}

void beacon_task(void *)
{
    const uint32_t random_session=esp_random();
    const uint32_t session=random_session ? random_session : 1;
    uint32_t event_id=0;
    nera_lab::Rule rules[]={ {nera_lab::HEART_ON,nera_lab::HEART_OFF},
                            {nera_lab::TEMP_ON,nera_lab::TEMP_OFF} };
    Pending pending[2];
    int64_t next_beacon=0;
    auto previous_preferences=nera_preferences::get();
    ESP_LOGW(TAG,"FLUJO V3: reloj -> app -> reloj -> C3 -> ACK GPIO5");
    ESP_LOGI(TAG,"Perfil inicial: avisos=%u confirmacion=%lld s ejercicio=%u",
        previous_preferences.enabled,(long long)(nera_preferences::hold_us(previous_preferences)/1000000),
        previous_preferences.exercise);
    while (true) {
        const int64_t now=esp_timer_get_time();
        const auto preferences=nera_preferences::get();
        if (preferences.enabled!=previous_preferences.enabled ||
            preferences.sensitivity!=previous_preferences.sensitivity ||
            preferences.exercise!=previous_preferences.exercise) {
            for (auto &rule:rules) rule.reset_qualification();
            previous_preferences=preferences;
            ESP_LOGI(TAG,"Perfil aplicado: avisos=%u confirmacion=%lld s ejercicio=%u",
                preferences.enabled,(long long)(nera_preferences::hold_us(preferences)/1000000),preferences.exercise);
        }
        nera_lab_packet ack={};
        while (xQueueReceive(ack_queue,&ack,0)==pdTRUE) {
            Pending &p=pending[ack.metric-1];
            if (p.waiting && ack.session==p.packet.session && ack.id==p.packet.id &&
                ack.state==p.packet.state && ack.centi_value==p.packet.centi_value) {
                p.waiting=false;
                p.confirmed=true; p.attempts=0; p.next_send=now+ACK_WAIT_US;
                nera_delivery::phase(ack.metric,NERA_CONFIRMED);
                ESP_LOGI(TAG,"ACK C3 CONFIRMADO evento=%lu metrica=%u estado=%u",
                         (unsigned long)ack.id,ack.metric,ack.state);
            }
        }
        // FUNCIONAL: consumir el snapshot real actual, sin restaurar AppState
        // ni los drivers anteriores. No leer I2C desde la tarea de radio.
        SensorSnapshot snapshot={};
        const bool snapshot_ok=sensor_service_get_snapshot(&snapshot)==ESP_OK;
        for (unsigned i=0;i<2;++i) {
            const float value=i==0 ? snapshot.bpm : snapshot.object_celsius;
            const int64_t sample=i==0 ? snapshot.heart_updated_us : snapshot.mlx_updated_us;
            const bool sensor=i==0 ? snapshot.max_online && snapshot.heart_valid : snapshot.mlx_valid;
            const bool valid=nera_delivery::phone_ready(now) && snapshot_ok && isfinite(value) && fresh(sensor,sample,now) &&
                             nera_preferences::eligible(preferences,i);
            const int event=rules[i].update(valid,value,sample,nera_preferences::hold_us(preferences));
            Pending &p=pending[i];
            if (event) {
                if (p.waiting) ESP_LOGW(TAG,"Evento pendiente reemplazado por estado mas reciente");
                p.packet={NERA_LAB_ALERT,(uint8_t)(i+1),
                    (uint8_t)(event==1 ? NERA_LAB_ACTIVE : event==3 ? NERA_LAB_INVALID : NERA_LAB_CLEAR),
                    session,++event_id,valid ? (int32_t)lroundf(value*100.f) : 0};
                p.waiting=true; p.attempts=0; p.next_send=now;
                p.confirmed=false; p.failed=false;
                nera_delivery::publish(p.packet);
                ESP_LOGW(TAG,"EVENTO LAB id=%lu metrica=%u estado=%u valor=%.2f",
                         (unsigned long)p.packet.id,p.packet.metric,p.packet.state,valid ? value : 0.f);
            }
            // La activacion requiere recepcion real en la app. El apagado es inmediato.
            const bool authorized=p.packet.state!=NERA_LAB_ACTIVE ||
                nera_delivery::approved(i+1,now);
            if (p.packet.id && p.packet.state==NERA_LAB_ACTIVE && p.confirmed && !authorized && !p.failed) {
                p.failed=true; p.waiting=false;
                nera_delivery::phase(i+1,NERA_FAILED);
                ESP_LOGW(TAG,"Recepcion por app vencida; se deja de renovar GPIO5");
            }
            if (!p.failed && p.packet.id && authorized && now>=p.next_send &&
                (p.waiting || p.packet.state==NERA_LAB_ACTIVE)) {
                if (p.attempts>=MAX_ATTEMPTS) {
                    ESP_LOGW(TAG,"SIN CONFIRMACION C3 evento=%lu tras %u intentos",
                             (unsigned long)p.packet.id,p.attempts);
                    p.waiting=false;
                    p.failed=true;
                    nera_delivery::phase(i+1,NERA_FAILED);
                } else {
                    if (!p.confirmed) nera_delivery::phase(i+1,NERA_SENDING);
                    p.waiting=true;
                    uint8_t bytes[NERA_LAB_SIZE]; nera_lab_encode(bytes,&p.packet);
                    const esp_err_t sent=esp_now_send(VINCHA_MAC,bytes,sizeof(bytes));
                    ESP_LOGI(TAG,"ENVIO LAB id=%lu intento=%u: %s",(unsigned long)p.packet.id,
                             ++p.attempts,esp_err_to_name(sent));
                    p.next_send=now+ACK_WAIT_US;
                }
            }
        }
        if (now>=next_beacon) {
        // Protocolo diagnóstico v1: "NERA", versión 1, tipo 1, secuencia LE.
        // IMPORTANTE: no es una orden; no contiene datos de sensores.
        const uint8_t packet[8] = {'N', 'E', 'R', 'A', 1, 1,
                                   static_cast<uint8_t>(sequence),
                                   static_cast<uint8_t>(sequence >> 8)};
        const esp_err_t err = esp_now_send(BROADCAST, packet, sizeof(packet));
        if (err != ESP_OK) ESP_LOGW(TAG, "Baliza no enviada: %s", esp_err_to_name(err));
        else ESP_LOGI(TAG, "Baliza encolada sec=%u; confirmar recepcion en C3", sequence);
        ++sequence;
        next_beacon=now+(int64_t)PERIOD_MS*1000;
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
}

esp_err_t espnow_diagnostic_init(void)
{
    if (radio_ready) return ESP_OK;
    ESP_LOGI(TAG, "RADIO UI V2: inicio despues de UI; interna libre=%u, mayor bloque DMA=%u",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    // FUNCIONAL: recuperar preferencias existentes; nunca borrar NVS ante error.
    esp_err_t err=nvs_flash_init();
    if (err!=ESP_OK) return err;
    err=nera_preferences::init();
    if (err!=ESP_OK) return err;
    err = esp_netif_init();
    if (err != ESP_OK) return err;
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    // MODIFICABLE: baliza de 8 bytes cada 5 s, sin conexion a router ni TCP.
    // Reservas acotadas para coexistir con pantalla y BLE. AMPDU no se usa aqui.
    config.static_rx_buf_num = 4;
    config.dynamic_rx_buf_num = 8;
    config.dynamic_tx_buf_num = 8;
    config.ampdu_rx_enable = 0;
    config.ampdu_tx_enable = 0;
    err = esp_wifi_init(&config);
    if (err != ESP_OK) return err;
    // FUNCIONAL: si un paso posterior falla, devolver a la interfaz la memoria
    // reservada por la radio; no dejar Wi-Fi parcialmente inicializado.
    bool started = false, now_ready = false;
    const auto fail = [&](esp_err_t cause) {
        if (now_ready) { esp_now_unregister_recv_cb(); esp_now_deinit(); }
        if (ack_queue) { vQueueDelete(ack_queue); ack_queue=nullptr; }
        if (started) esp_wifi_stop();
        esp_wifi_deinit();
        return cause;
    };
    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (err != ESP_OK) return fail(err);
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) return fail(err);
    err = esp_wifi_start();
    if (err != ESP_OK) return fail(err);
    started = true;
    err = esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
    if (err != ESP_OK) return fail(err);
    err = esp_now_init();
    if (err != ESP_OK) return fail(err);
    now_ready = true;
    esp_now_peer_info_t peer = {};
    for (int i = 0; i < 6; ++i) peer.peer_addr[i] = BROADCAST[i];
    peer.ifidx = WIFI_IF_STA;
    peer.channel = CHANNEL;
    err = esp_now_add_peer(&peer);
    if (err != ESP_OK) return fail(err);
    memcpy(peer.peer_addr,VINCHA_MAC,6);
    err=esp_now_add_peer(&peer);
    if (err!=ESP_OK) return fail(err);
    ack_queue=xQueueCreate(8,sizeof(nera_lab_packet));
    if (!ack_queue) return fail(ESP_ERR_NO_MEM);
    err=esp_now_register_recv_cb(receive_ack);
    if (err!=ESP_OK) return fail(err);
    if (xTaskCreate(beacon_task, "nera_radio_test", 6144, nullptr, 2, nullptr) != pdPASS)
        return fail(ESP_ERR_NO_MEM);
    radio_ready=true;
    ESP_LOGI(TAG, "RADIO UI V2: activa; interna libre=%u, mayor bloque DMA=%u",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Canal %u; GPIO5 condicionado a recepcion por app y vigencia del enlace", CHANNEL);
    ESP_LOGI(TAG,"Destino C3: %02X:%02X:%02X:%02X:%02X:%02X; encolado no significa recibido, esperar ACK",
        VINCHA_MAC[0],VINCHA_MAC[1],VINCHA_MAC[2],VINCHA_MAC[3],VINCHA_MAC[4],VINCHA_MAC[5]);
    return ESP_OK;
}
