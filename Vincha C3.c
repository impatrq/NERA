#include <iostream>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

static const char *TAG = "VINCHA_C3";

// Pin físico que va al MOSFET (GPIO 5 en el ESP32-C3 Super Mini)
#define PIN_MOSFET GPIO_NUM_5

// Estructura idéntica para recibir datos
typedef struct struct_mensaje {
    bool dispararEstimulo;
    float bpm;
    float temperatura;
} struct_mensaje;

// Función Callback: Se ejecuta AUTOMÁTICAMENTE cada vez que llega un paquete por el aire
void al_recibir_datos(const esp_now_recv_info_t *info, const uint8_t *datos, int longitud) {
    struct_mensaje datos_recibidos;
    
    // Copiamos los bytes que llegaron por el aire a nuestra estructura de C++
    std::memcpy(&datos_recibidos, datos, sizeof(datos_recibidos));

    ESP_LOGI(TAG, "Datos recibidos -> BPM: %.1f | Temp: %.1f C", datos_recibidos.bpm, datos_recibidos.temperatura);

    if (datos_recibidos.dispararEstimulo) {
        ESP_LOGW(TAG, "¡ALERTA DE ANSIEDAD! Activando pulso en MOSFET...");
        gpio_set_level(PIN_MOSFET, 1); // Enciende el MOSFET (o LED de prueba)
    } else {
        ESP_LOGI(TAG, "Parámetros normales. MOSFET apagado.");
        gpio_set_level(PIN_MOSFET, 0); // Apaga el MOSFET
    }
}

// Inicialización obligatoria del WiFi para usar ESP-NOW
void inicializar_wifi() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // Modo Estación obligado
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Imprimir la dirección MAC de esta vincha en la consola
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    ESP_LOGW(TAG, "DIRECCIÓN MAC DE TU ESP32-C3: %02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

extern "C" void app_main(void) {
    // 1. Inicializar almacenamiento interno (NVS) requerido por el WiFi nativo
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Configurar el pin del MOSFET como salida digital
    gpio_reset_pin(PIN_MOSFET);
    gpio_set_direction(PIN_MOSFET, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_MOSFET, 0);

    // 3. Encender WiFi y registrar ESP-NOW
    inicializar_wifi();

    ESP_ERROR_CHECK(esp_now_init());
    
    // Registrar la función que creamos arriba para que el chip sepa qué hacer al recibir datos
    ESP_ERROR_CHECK(esp_now_register_recv_cb(al_recibir_datos));

    ESP_LOGI(TAG, "Vincha en modo escucha. Esperando señal del reloj...");
    
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}