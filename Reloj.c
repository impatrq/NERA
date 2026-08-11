#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/i2c.h"

static const char *TAG = "RELOJ_S3";

// Configuración de I2C para ESP32-S3
#define I2C_MASTER_NUM       I2C_NUM_0
#define PIN_SDA              GPIO_NUM_8
#define PIN_SCL              GPIO_NUM_9
#define I2C_MASTER_FREQ_HZ   100000

// Dirección I2C típica de las pantallas LCD con mochila I2C (suele ser 0x27 o 0x3F)
#define LCD_ADDR             0x27

// REEMPLAZA ESTO con la MAC de tu Vincha (ESP32-C3)
static const uint8_t mac_vincha[6] = {0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56};

typedef struct struct_mensaje {
    bool dispararEstimulo;
    float bpm;
    float temperatura;
} struct_mensaje;

// --- FUNCIONES BÁSICAS PARA MANEJO DE LCD I2C ---
esp_err_t lcd_enviar_byte(uint8_t data, uint8_t mode) {
    uint8_t data_u, data_l;
    uint8_t buf[4];
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);
    buf[0] = data_u | mode | 0x08; // 0x08 mantiene el backlight encendido
    buf[1] = buf[0] | 0x04;        // Enable en High
    buf[2] = buf[0] & ~0x04;       // Enable en Low
    buf[3] = data_l | mode | 0x08;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buf, 3, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void lcd_comando(uint8_t cmd) { lcd_enviar_byte(cmd, 0); }
void lcd_datos(uint8_t data) { lcd_enviar_byte(data, 1); }

void lcd_init(void) {
    vTaskDelay(pdMS_TO_TICKS(50));
    lcd_comando(0x30);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_comando(0x30);
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_comando(0x32);
    lcd_comando(0x28); // Modo 4-bits, 2 líneas
    lcd_comando(0x0C); // Encender display, ocultar cursor
    lcd_comando(0x01); // Limpiar pantalla
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_texto(const char *str) {
    while (*str) {
        lcd_datos((uint8_t)(*str++));
    }
}

void lcd_set_cursor(uint8_t fila, uint8_t columna) {
    uint8_t addr = (fila == 0) ? (0x80 + columna) : (0xC0 + columna);
    lcd_comando(addr);
}

// --- CONFIGURACIÓN DE I2C Y WIFI ---
void inicializar_i2c(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void inicializar_wifi_emisor(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 1. Inicializar Hardware (I2C y Pantalla LCD)
    inicializar_i2c();
    lcd_init();

    // 2. Mensaje de Bienvenida en Pantalla
    lcd_set_cursor(0, 0);
    lcd_texto("RELOJ ANSIEDAD");
    lcd_set_cursor(1, 0);
    lcd_texto("Iniciando...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    lcd_comando(0x01); // Limpiar pantalla

    // 3. Inicializar ESP-NOW
    inicializar_wifi_emisor();
    ESP_ERROR_CHECK(esp_now_init());

    esp_now_peer_info_t informacion_peer = {0};
    memcpy(informacion_peer.peer_addr, mac_vincha, 6);
    informacion_peer.channel = 0;
    informacion_peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&informacion_peer));

    struct_mensaje mis_datos;
    float pulso_simulado = 75.0;
    float temp_simulada = 36.5;
    char linea_buff[17];

    while (true) {
        // --- SIMULACIÓN Y LECTURA ---
        pulso_simulado += 5.0;
        if (pulso_simulado > 125.0) {
            pulso_simulado = 75.0;
            temp_simulada = 36.5;
        }

        if (pulso_simulado > 110.0) {
            temp_simulada = 34.0;
        }

        mis_datos.bpm = pulso_simulado;
        mis_datos.temperatura = temp_simulada;

        // Evaluamos el estado clínico
        if (mis_datos.bpm > 110.0 && mis_datos.temperatura < 34.5) {
            mis_datos.dispararEstimulo = true;
        } else {
            mis_datos.dispararEstimulo = false;
        }

        // --- ACTUALIZAR PANTALLA LCD EN TIEMPO REAL ---
        // Línea 1: Muestra los BPM y la Temperatura
        snprintf(linea_buff, sizeof(linea_buff), "BPM:%.0f T:%.1fC", mis_datos.bpm, mis_datos.temperatura);
        lcd_set_cursor(0, 0);
        lcd_texto(linea_buff);

        // Línea 2: Muestra si el estímulo está activo o en estado normal
        lcd_set_cursor(1, 0);
        if (mis_datos.dispararEstimulo) {
            lcd_texto("ESTADO: ALERTA! ");
        } else {
            lcd_texto("ESTADO: NORMAL  ");
        }

        // --- TRANSMISIÓN ESP-NOW ---
        esp_now_send(mac_vincha, (uint8_t *) &mis_datos, sizeof(mis_datos));

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}