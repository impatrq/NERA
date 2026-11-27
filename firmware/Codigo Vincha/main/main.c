/* NERA Vincha — prueba aislada de radio ESP-NOW para ESP32-C3.
 * FUNCIONAL: escucha balizas, muestra MAC y secuencia por monitor serial.
 * GPIO5 apagado al inicio, activacion con orden vigente, timeout de 5 s.
 */
#include <inttypes.h>
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "gpio5_signal_test.h"
#include "../../Protocolo Reloj Vincha/nera_lab_protocol.h"
#include "../../Protocolo Reloj Vincha/nera_output_state.h"

static const char *TAG = "NERA/VINCHA";
static const uint8_t CHANNEL = 1; // MODIFICABLE: igual que en el reloj.
static QueueHandle_t received_queue;
// MODIFICABLE: reloj usado en la prueba fisica. No aceptar eventos de otros emisores.
static const uint8_t WATCH_MAC[6]={0x28,0x84,0x85,0x47,0xbc,0x98};

typedef struct {
    uint8_t source[6];
    uint16_t sequence;
    uint8_t is_alert;
    nera_lab_packet alert;
} diagnostic_packet_t;

// Callback del driver Wi-Fi: copiar solo datos pequeños; el log va en otra tarea.
static void on_receive(const esp_now_recv_info_t *info, const uint8_t *data, int length)
{
    if (!info || !data || length<0 || memcmp(info->src_addr,WATCH_MAC,6)!=0) return;
    diagnostic_packet_t packet={0};
    if (length==8 && memcmp(data,"NERA",4)==0 && data[4]==1 && data[5]==1)
        packet.sequence=(uint16_t)(data[6] | (data[7]<<8));
    else if (nera_lab_decode(data,(size_t)length,&packet.alert) && packet.alert.type==NERA_LAB_ALERT)
        packet.is_alert=1;
    else return;
    memcpy(packet.source, info->src_addr, sizeof(packet.source));
    xQueueSend(received_queue, &packet, 0);
}

static void log_received(void *arg)
{
    (void)arg;
    diagnostic_packet_t packet;
    nera_output_state output={0};
    while (true) {
        const int was_on=nera_output_on(&output);
        nera_output_expire(&output,esp_timer_get_time());
        if (was_on && !nera_output_on(&output)) {
            ESP_ERROR_CHECK(gpio5_signal_test_set_active(false));
            ESP_LOGW(TAG,"ENLACE VENCIDO: GPIO5 apagado por falta de renovacion (5 s)");
        }
        if (xQueueReceive(received_queue, &packet, pdMS_TO_TICKS(100))!=pdTRUE) continue;
        if (packet.is_alert) {
            nera_lab_packet *event=&packet.alert;
            const unsigned index=event->metric-1;
            const int duplicate=output.seen[index] && output.last[index].session==event->session &&
                                event->id==output.last[index].id;
            nera_output_state candidate=output;
            if (!nera_output_receive(&candidate,event,esp_timer_get_time())) continue;
            // Confirmar solamente DESPUES de aplicar la salida solicitada.
            if (gpio5_signal_test_set_active(nera_output_on(&candidate))!=ESP_OK) {
                ESP_LOGE(TAG,"GPIO5 ERROR: orden sin confirmar");
                continue;
            }
            output=candidate;
            if (!duplicate) {
                const char *metric=event->metric==NERA_LAB_HEART ? "PULSO BPM" : "TEMPERATURA OBJETO C";
                if (event->state==NERA_LAB_INVALID)
                    ESP_LOGW(TAG,"ALERTA INACTIVA por dato invalido/vencido: %s evento=%lu",
                             metric,(unsigned long)event->id);
                else
                    ESP_LOGW(TAG,"ALERTA %s: %s valor=%.2f evento=%lu",
                             event->state==NERA_LAB_ACTIVE ? "ACTIVA" : "RESTABLECIDA", metric,
                             event->centi_value/100.0,(unsigned long)event->id);
            }
            // FUNCIONAL: confirmar tambien duplicados. Si se pierde el ACK,
            // el reloj puede repetir el evento, pero no volver a procesarlo.
            esp_err_t result=ESP_OK;
            if (!esp_now_is_peer_exist(packet.source)) {
                esp_now_peer_info_t peer={0};
                memcpy(peer.peer_addr,packet.source,6);
                peer.channel=CHANNEL; peer.ifidx=WIFI_IF_STA;
                result=esp_now_add_peer(&peer);
            }
            if (result==ESP_OK) {
                uint8_t bytes[NERA_LAB_SIZE];
                event->type=NERA_LAB_ACK;
                nera_lab_encode(bytes,event);
                result=esp_now_send(packet.source,bytes,sizeof(bytes));
            }
            ESP_LOGI(TAG,"ACK GPIO5 evento=%lu %s salida=%s: %s",(unsigned long)event->id,
                     duplicate ? "renovacion" : "nuevo",nera_output_on(&output) ? "ACTIVA" : "APAGADA",esp_err_to_name(result));
            continue;
        }
        ESP_LOGI(TAG, "Baliza recibida de %02X:%02X:%02X:%02X:%02X:%02X sec=%u",
                 packet.source[0], packet.source[1], packet.source[2],
                 packet.source[3], packet.source[4], packet.source[5],
                 packet.sequence);
    }
}

void app_main(void)
{
    // Arrancar siempre apagado; no generar PWM hasta recibir una orden.
    ESP_ERROR_CHECK(gpio5_signal_test_start());
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS: %s", esp_err_to_name(err));
        return;
    }
    received_queue = xQueueCreate(12, sizeof(diagnostic_packet_t));
    if (!received_queue) {
        ESP_LOGE(TAG, "Sin memoria para cola de radio");
        return;
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE));
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_receive));

    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));
    ESP_LOGI(TAG, "Vincha C3 lista; MAC %02X:%02X:%02X:%02X:%02X:%02X canal %u",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], CHANNEL);
    ESP_LOGI(TAG, "FLUJO V3 + ACK GPIO5; apagado inicial; timeout 5 s");
    if (xTaskCreate(log_received, "nera_radio_log", 4096, NULL, 2, NULL) != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear la tarea de log");
    }
}
