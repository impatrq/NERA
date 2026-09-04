/**
 * @file display_driver.cpp
 * @brief Implementación del driver ST7789T3 sobre SPI para la pantalla Waveshare 2".
 */

#include "display_driver.h"
#include <string.h>
#include <stdlib.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/logger.h"

static const char *TAG = NERA_TAG_DISPLAY;

static esp_lcd_panel_handle_t s_panel_handle = NULL;
static esp_lcd_panel_io_handle_t s_io_handle = NULL;
static bool s_is_initialized = false;

static inline uint16_t rgb565_color(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (uint16_t)((c << 8) | (c >> 8)); // Swap de bytes para SPI ST7789
}

esp_err_t display_driver_init(void) {
    if (s_is_initialized) {
        NERA_LOGW(TAG, "Display ya inicializado");
        return ESP_OK;
    }

    NERA_LOGI(TAG, "Inicializando bus SPI para LCD (MOSI=%d, SCLK=%d, CS=%d, DC=%d)...",
              NERA_LCD_PIN_MOSI, NERA_LCD_PIN_SCLK, NERA_LCD_PIN_CS, NERA_LCD_PIN_DC);

    // 1. Bus SPI
    spi_bus_config_t bus_cfg = {};
    bus_cfg.sclk_io_num     = (gpio_num_t)NERA_LCD_PIN_SCLK;
    bus_cfg.mosi_io_num     = (gpio_num_t)NERA_LCD_PIN_MOSI;
    bus_cfg.miso_io_num     = (gpio_num_t)-1;
    bus_cfg.quadwp_io_num   = -1;
    bus_cfg.quadhd_io_num   = -1;
    bus_cfg.max_transfer_sz = NERA_LCD_WIDTH * 40 * sizeof(uint16_t);

    esp_err_t ret = spi_bus_initialize(NERA_LCD_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Fallo al inicializar el bus SPI: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Panel IO SPI
    esp_lcd_panel_io_spi_config_t io_cfg = {};
    io_cfg.dc_gpio_num         = (gpio_num_t)NERA_LCD_PIN_DC;
    io_cfg.cs_gpio_num         = (gpio_num_t)NERA_LCD_PIN_CS;
    io_cfg.pclk_hz             = NERA_LCD_SPI_FREQ_HZ; // 40 MHz
    io_cfg.lcd_cmd_bits        = 8;
    io_cfg.lcd_param_bits      = 8;
    io_cfg.spi_mode            = 0;
    io_cfg.trans_queue_depth   = 10;

    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)NERA_LCD_SPI_HOST, &io_cfg, &s_io_handle);
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Fallo al crear panel IO SPI: %s", esp_err_to_name(ret));
        return ret;
    }

    // 3. Controlador ST7789T3
    esp_lcd_panel_dev_config_t panel_cfg = {};
    panel_cfg.reset_gpio_num = (gpio_num_t)NERA_LCD_PIN_RST;
    panel_cfg.rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_BGR;
    panel_cfg.data_endian    = LCD_RGB_DATA_ENDIAN_BIG;
    panel_cfg.bits_per_pixel = 16;

    ret = esp_lcd_new_panel_st7789(s_io_handle, &panel_cfg, &s_panel_handle);
    if (ret != ESP_OK) {
        NERA_LOGE(TAG, "Fallo al instanciar panel ST7789: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Secuencia de arranque del panel LCD
    if (NERA_LCD_PIN_RST >= 0) {
        ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    }
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(s_panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel_handle, true));

    s_is_initialized = true;
    NERA_LOGI(TAG, "Panel LCD ST7789T3 inicializado con éxito (%dx%d px)", NERA_LCD_WIDTH, NERA_LCD_HEIGHT);

    return ESP_OK;
}

esp_lcd_panel_handle_t display_driver_get_panel_handle(void) {
    return s_panel_handle;
}

esp_err_t display_driver_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data) {
    if (!s_is_initialized || s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_draw_bitmap(s_panel_handle, x_start, y_start, x_end, y_end, color_data);
}

esp_err_t display_driver_fill_screen(uint16_t rgb565) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;

    const int lines_per_chunk = 20;
    const size_t chunk_pixels = NERA_LCD_WIDTH * lines_per_chunk;
    const size_t chunk_bytes  = chunk_pixels * sizeof(uint16_t);

    uint16_t *buffer = (uint16_t *)heap_caps_malloc(chunk_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (buffer == NULL) {
        NERA_LOGE(TAG, "Memoria insuficiente para buffer de dibujo");
        return ESP_ERR_NO_MEM;
    }

    for (size_t i = 0; i < chunk_pixels; i++) {
        buffer[i] = rgb565;
    }

    for (int y = 0; y < NERA_LCD_HEIGHT; y += lines_per_chunk) {
        int y_end = y + lines_per_chunk;
        if (y_end > NERA_LCD_HEIGHT) y_end = NERA_LCD_HEIGHT;
        display_driver_draw_bitmap(0, y, NERA_LCD_WIDTH, y_end, buffer);
    }

    free(buffer);
    return ESP_OK;
}

esp_err_t display_driver_draw_test_pattern(void) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;

    NERA_LOGI(TAG, "Generando y dibujando patron de prueba NERA...");

    const int lines_per_chunk = 20;
    const size_t chunk_pixels = NERA_LCD_WIDTH * lines_per_chunk;
    uint16_t *buffer = (uint16_t *)heap_caps_malloc(chunk_pixels * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (buffer == NULL) return ESP_ERR_NO_MEM;

    uint16_t col_bg      = rgb565_color(10, 10, 15);   // Fondo oscuro
    uint16_t col_accent  = rgb565_color(0, 229, 255);  // Cian NERA
    uint16_t col_heart   = rgb565_color(255, 68, 88);  // Rojo corazón
    uint16_t col_green   = rgb565_color(0, 230, 118);  // Verde salud
    uint16_t col_warning = rgb565_color(255, 171, 0);  // Ámbar aviso
    uint16_t col_card    = rgb565_color(24, 24, 38);   // Superficie tarjeta

    for (int y = 0; y < NERA_LCD_HEIGHT; y += lines_per_chunk) {
        for (int line = 0; line < lines_per_chunk; line++) {
            int cur_y = y + line;
            if (cur_y >= NERA_LCD_HEIGHT) break;

            for (int x = 0; x < NERA_LCD_WIDTH; x++) {
                uint16_t px_color = col_bg;

                // 1. Franja superior
                if (cur_y < 30) {
                    if (cur_y >= 28) {
                        px_color = col_accent;
                    } else if (x > 20 && x < 60 && cur_y > 10 && cur_y < 20) {
                        px_color = col_green;
                    } else if (x > 180 && x < 220 && cur_y > 10 && cur_y < 20) {
                        px_color = col_accent;
                    }
                }
                // 2. Tarjeta central de ritmo cardíaco (Y: 50 - 130)
                else if (cur_y >= 50 && cur_y < 130 && x >= 20 && x < 220) {
                    if (cur_y == 50 || cur_y == 129 || x == 20 || x == 219) {
                        px_color = col_accent;
                    } else if (x > 40 && x < 70 && cur_y > 75 && cur_y < 105) {
                        px_color = col_heart;
                    } else {
                        px_color = col_card;
                    }
                }
                // 3. Tarjeta de temperatura / salud (Y: 145 - 225)
                else if (cur_y >= 145 && cur_y < 225 && x >= 20 && x < 220) {
                    if (cur_y == 145 || cur_y == 224 || x == 20 || x == 219) {
                        px_color = col_warning;
                    } else if (x > 40 && x < 70 && cur_y > 170 && cur_y < 200) {
                        px_color = col_warning;
                    } else {
                        px_color = col_card;
                    }
                }
                // 4. Barras de color inferior (Y: 250 - 290)
                else if (cur_y >= 250 && cur_y < 290 && x >= 20 && x < 220) {
                    int col_idx = (x - 20) / 40;
                    switch (col_idx) {
                        case 0: px_color = col_accent;  break;
                        case 1: px_color = col_green;   break;
                        case 2: px_color = col_warning; break;
                        case 3: px_color = col_heart;   break;
                        default: px_color = col_bg;     break;
                    }
                }
                // 5. Borde perimetral
                if (cur_y == 0 || cur_y == NERA_LCD_HEIGHT - 1 || x == 0 || x == NERA_LCD_WIDTH - 1) {
                    px_color = col_accent;
                }

                buffer[line * NERA_LCD_WIDTH + x] = px_color;
            }
        }

        display_driver_draw_bitmap(0, y, NERA_LCD_WIDTH, y + lines_per_chunk, buffer);
    }

    free(buffer);
    NERA_LOGI(TAG, "Patron de prueba visual dibujado en la pantalla fisica OK");
    return ESP_OK;
}
