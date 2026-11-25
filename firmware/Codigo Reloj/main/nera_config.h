/** Configuracion del ESP32-S3 Waveshare Touch LCD 2. */
#pragma once
#include "sdkconfig.h"

// IMPORTANTE: pines existentes del display; no cambiar por apariencia.
#define NERA_LCD_WIDTH 240
#define NERA_LCD_HEIGHT 320
#define NERA_LCD_PIN_MOSI 38
#define NERA_LCD_PIN_SCLK 39
#define NERA_LCD_PIN_DC 42
#define NERA_LCD_PIN_CS 45
#define NERA_LCD_PIN_RST -1
#define NERA_LCD_PIN_BL 1
#define NERA_LCD_SPI_FREQ_HZ (40*1000*1000)
#define NERA_LCD_SPI_HOST SPI2_HOST
#define NERA_LCD_DMA_BUFFER_SIZE (NERA_LCD_WIDTH*20*2)

// FUNCIONAL: bus propio del tactil integrado.
#define NERA_TOUCH_PIN_SDA 48
#define NERA_TOUCH_PIN_SCL 47
#define NERA_TOUCH_PIN_INT -1
#define NERA_TOUCH_PIN_RST -1
#define NERA_TOUCH_I2C_ADDR 0x15
#define NERA_I2C_FREQ_HZ (400*1000)
#define NERA_I2C_PORT I2C_NUM_0

// MODIFICABLE: brillo inicial y limites del control en pantalla.
#define NERA_BL_LEDC_CHANNEL LEDC_CHANNEL_0
#define NERA_BL_LEDC_TIMER LEDC_TIMER_0
#define NERA_BL_PWM_FREQ_HZ 5000
#define NERA_BL_PWM_RESOLUTION LEDC_TIMER_8_BIT
#define NERA_BL_MIN 10
#define NERA_BL_MAX 255
#define NERA_BL_DEFAULT 220
#define NERA_UI_TASK_STACK_SIZE 8192
#define NERA_UI_TASK_PRIORITY 5
#define NERA_LVGL_BUFFER_SIZE_BYTES (NERA_LCD_WIDTH*NERA_LCD_HEIGHT*2)
