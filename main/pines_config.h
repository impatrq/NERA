#pragma once

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"

#define NERA_LCD_H_RES                 240
#define NERA_LCD_V_RES                 320
#define NERA_LCD_PIXEL_CLOCK_HZ        (40 * 1000 * 1000)
#define NERA_LCD_CMD_BITS              8
#define NERA_LCD_PARAM_BITS            8
#define NERA_LCD_SPI_HOST              SPI2_HOST
#define NERA_LCD_BUFFER_LINEAS         40

// Pines del Waveshare ESP32-S3-Touch-LCD-2, pantalla ST7789T3 por SPI.
#define NERA_PIN_LCD_MOSI              GPIO_NUM_45
#define NERA_PIN_LCD_SCLK              GPIO_NUM_40
#define NERA_PIN_LCD_CS                GPIO_NUM_42
#define NERA_PIN_LCD_DC                GPIO_NUM_41
#define NERA_PIN_LCD_RST               GPIO_NUM_39
#define NERA_PIN_LCD_BL                GPIO_NUM_5

// Touch CST816D por I2C. El driver de Espressif para CST816S es compatible a nivel de bus.
#define NERA_TOUCH_I2C_PORT            I2C_NUM_0
#define NERA_PIN_TOUCH_SDA             GPIO_NUM_1
#define NERA_PIN_TOUCH_SCL             GPIO_NUM_3
#define NERA_PIN_TOUCH_INT             GPIO_NUM_4
#define NERA_PIN_TOUCH_RST             GPIO_NUM_2
#define NERA_TOUCH_I2C_HZ              (400 * 1000)

// IMU QMI8658C. Queda para la integracion real de sensores; no se usa en Etapa 1.
#define NERA_IMU_I2C_PORT              I2C_NUM_1
#define NERA_PIN_IMU_SDA               GPIO_NUM_11
#define NERA_PIN_IMU_SCL               GPIO_NUM_10
#define NERA_IMU_I2C_ADDR              0x6B

// TODO: confirmar pin/canal ADC real de bateria para esta revision del hardware.
#define NERA_PIN_BATERIA_ADC           GPIO_NUM_NC
