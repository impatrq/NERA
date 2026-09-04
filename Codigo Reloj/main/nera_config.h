/**
 * @file nera_config.h
 * @brief Configuración central del firmware NERA.
 *
 * Mapeo 100% verificado contra el código fuente oficial del demo
 * de Waveshare para la placa ESP32-S3-Touch-LCD-2.
 */

#pragma once

// =============================================================================
// VERSIÓN DEL FIRMWARE
// =============================================================================

#define NERA_FW_VERSION_MAJOR   0
#define NERA_FW_VERSION_MINOR   1
#define NERA_FW_VERSION_PATCH   0
#define NERA_FW_VERSION_STR     "0.1.0-alpha"
#define NERA_DEVICE_NAME        "NERA Smartwatch"

// =============================================================================
// PANTALLA — LCD ST7789T3 (240x320, SPI) — Pines Oficiales Waveshare
// =============================================================================

/// Ancho de la pantalla en píxeles.
#define NERA_LCD_WIDTH          240

/// Alto de la pantalla en píxeles.
#define NERA_LCD_HEIGHT         320

/// GPIO de datos MOSI (Master Out Slave In)
#define NERA_LCD_PIN_MOSI       38

/// GPIO de reloj SPI (SCLK)
#define NERA_LCD_PIN_SCLK       39

/// GPIO de Data/Command (DC)
#define NERA_LCD_PIN_DC         42

/// GPIO de Chip Select (CS)
#define NERA_LCD_PIN_CS         45

/// GPIO de Reset (-1: conectado a EN/Reset del sistema)
#define NERA_LCD_PIN_RST        -1

/// GPIO de Backlight (BL)
#define NERA_LCD_PIN_BL         1

/// Frecuencia del bus SPI para la pantalla en Hz (40 MHz)
#define NERA_LCD_SPI_FREQ_HZ    (40 * 1000 * 1000)

/// Canal SPI2 del ESP32-S3
#define NERA_LCD_SPI_HOST       SPI2_HOST

/// Tamaño del buffer de transferencia DMA en bytes
#define NERA_LCD_DMA_BUFFER_SIZE (NERA_LCD_WIDTH * 20 * 2)

// =============================================================================
// TOUCH (CST816D) & IMU (QMI8658) — Bus I2C Compartido
// =============================================================================

/// GPIO de datos I2C (SDA)
#define NERA_TOUCH_PIN_SDA      48

/// GPIO de reloj I2C (SCL)
#define NERA_TOUCH_PIN_SCL      47

/// GPIO de interrupción del touch
#define NERA_TOUCH_PIN_INT      -1

/// GPIO de Reset del controlador táctil
#define NERA_TOUCH_PIN_RST      -1

/// Dirección I2C del CST816D
#define NERA_TOUCH_I2C_ADDR     0x15

/// Dirección I2C del QMI8658
#define NERA_IMU_I2C_ADDR       0x6B

/// Frecuencia del bus I2C (400 KHz)
#define NERA_I2C_FREQ_HZ        (400 * 1000)

/// Puerto I2C 0
#define NERA_I2C_PORT           I2C_NUM_0

// =============================================================================
// BACKLIGHT — Control de brillo por PWM (LEDC)
// =============================================================================

#define NERA_BL_LEDC_CHANNEL    LEDC_CHANNEL_0
#define NERA_BL_LEDC_TIMER      LEDC_TIMER_0
#define NERA_BL_PWM_FREQ_HZ     5000
#define NERA_BL_PWM_RESOLUTION  LEDC_TIMER_8_BIT
#define NERA_BL_MAX             255
#define NERA_BL_MIN             10
#define NERA_BL_DEFAULT         220

// =============================================================================
// POWER MANAGEMENT
// =============================================================================

#define NERA_SCREEN_DIM_TIMEOUT_MS      CONFIG_NERA_SCREEN_TIMEOUT_MS
#define NERA_SCREEN_OFF_TIMEOUT_MS      10000

// =============================================================================
// SENSORES
// =============================================================================

#define NERA_SENSOR_UPDATE_INTERVAL_MS  1000
#define NERA_HISTORY_BUFFER_SIZE        60
#define NERA_USE_MOCK_SENSORS           1

// =============================================================================
// TAREAS FREERTOS
// =============================================================================

#define NERA_UI_TASK_STACK_SIZE         8192
#define NERA_SENSOR_TASK_STACK_SIZE     4096
#define NERA_BLE_TASK_STACK_SIZE        4096
#define NERA_STORAGE_TASK_STACK_SIZE    4096

#define NERA_UI_TASK_PRIORITY           5
#define NERA_SENSOR_TASK_PRIORITY       3
#define NERA_BLE_TASK_PRIORITY          2
#define NERA_STORAGE_TASK_PRIORITY      2

// =============================================================================
// UI — PALETA DE COLORES NERA
// =============================================================================

#define NERA_COLOR_BG              0x0A0A0F  // Fondo casi negro
#define NERA_COLOR_SURFACE         0x141420  // Superficie tarjetas
#define NERA_COLOR_SURFACE_2       0x1E1E2E  // Superficie secundaria

#define NERA_COLOR_TEXT_PRIMARY    0xF0F0F5  // Texto principal
#define NERA_COLOR_TEXT_SECONDARY  0x8888A0  // Texto secundario
#define NERA_COLOR_TEXT_DISABLED   0x444455  // Texto deshabilitado

#define NERA_COLOR_ACCENT          0x00E5FF  // Cian eléctrico
#define NERA_COLOR_ACCENT_DIM      0x0099BB  // Cian atenuado

#define NERA_COLOR_HEALTH_OK       0x00E676  // Verde normal
#define NERA_COLOR_WARNING         0xFFAB00  // Ámbar aviso
#define NERA_COLOR_DANGER          0xFF1744  // Rojo alerta
#define NERA_COLOR_INFO            0x40C4FF  // Azul información

#define NERA_COLOR_HEART           0xFF4458  // Rojo corazón
#define NERA_COLOR_TEMP            0xFF9800  // Naranja temperatura
#define NERA_COLOR_SLEEP           0x7C4DFF  // Violeta sueño
#define NERA_COLOR_BATTERY         0x69F0AE  // Verde batería

// =============================================================================
// LVGL — Configuración del motor gráfico
// =============================================================================

/// Tamaño de cada buffer de dibujo en bytes (240 x 320 x 2 bytes RGB565)
/// Se asignan 2 buffers en PSRAM para double buffering sin parpadeos
#define NERA_LVGL_BUFFER_SIZE_BYTES (NERA_LCD_WIDTH * NERA_LCD_HEIGHT * 2)
