#include "display.h"

#include "esp_check.h"
#include "esp_lcd_io_i2c.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "pines_config.h"

static const char *TAG = "nera_display";

static esp_lcd_panel_handle_t panel_lcd;
static esp_lcd_panel_io_handle_t io_lcd;
static i2c_master_bus_handle_t bus_touch;
static esp_lcd_touch_handle_t touch;
static lv_display_t *display_lvgl;

static esp_err_t inicializar_backlight(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = BIT64(NERA_PIN_LCD_BL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "No se pudo configurar backlight");
    return gpio_set_level(NERA_PIN_LCD_BL, 1);
}

static esp_err_t inicializar_panel_lcd(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = NERA_PIN_LCD_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = NERA_PIN_LCD_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = NERA_LCD_H_RES * NERA_LCD_BUFFER_LINEAS * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(NERA_LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO),
                        TAG, "No se pudo iniciar el bus SPI del LCD");

    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = NERA_PIN_LCD_CS,
        .dc_gpio_num = NERA_PIN_LCD_DC,
        .spi_mode = 0,
        .pclk_hz = NERA_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = NERA_LCD_CMD_BITS,
        .lcd_param_bits = NERA_LCD_PARAM_BITS,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)NERA_LCD_SPI_HOST,
                                                 &io_config, &io_lcd),
                        TAG, "No se pudo crear el IO SPI del LCD");

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = NERA_PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(io_lcd, &panel_config, &panel_lcd),
                        TAG, "No se pudo crear el panel ST7789");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel_lcd), TAG, "No se pudo resetear el LCD");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_lcd), TAG, "No se pudo inicializar el LCD");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(panel_lcd, true), TAG, "No se pudo invertir color");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel_lcd, true), TAG, "No se pudo encender el LCD");

    return ESP_OK;
}

static esp_err_t inicializar_touch(void)
{
    i2c_master_bus_config_t i2c_cfg = {
        .i2c_port = NERA_TOUCH_I2C_PORT,
        .sda_io_num = NERA_PIN_TOUCH_SDA,
        .scl_io_num = NERA_PIN_TOUCH_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i2c_cfg, &bus_touch),
                        TAG, "No se pudo iniciar el I2C del touch");

    esp_lcd_panel_io_handle_t io_touch = NULL;
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    io_cfg.scl_speed_hz = NERA_TOUCH_I2C_HZ;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(bus_touch, &io_cfg, &io_touch),
                        TAG, "No se pudo crear el IO I2C del touch");

    esp_lcd_touch_config_t touch_cfg = {
        .x_max = NERA_LCD_H_RES,
        .y_max = NERA_LCD_V_RES,
        .rst_gpio_num = NERA_PIN_TOUCH_RST,
        .int_gpio_num = NERA_PIN_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_cst816s(io_touch, &touch_cfg, &touch),
                        TAG, "No se pudo inicializar CST816D/CST816S");

    return ESP_OK;
}

esp_err_t nera_display_inicializar(void)
{
    ESP_LOGI(TAG, "Inicializando LCD ST7789T3 y LVGL");
    ESP_RETURN_ON_ERROR(inicializar_backlight(), TAG, "Fallo backlight");
    ESP_RETURN_ON_ERROR(inicializar_panel_lcd(), TAG, "Fallo LCD");

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "No se pudo iniciar esp_lvgl_port");

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_lcd,
        .panel_handle = panel_lcd,
        .buffer_size = NERA_LCD_H_RES * NERA_LCD_BUFFER_LINEAS,
        .double_buffer = true,
        .hres = NERA_LCD_H_RES,
        .vres = NERA_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
        },
    };
    display_lvgl = lvgl_port_add_disp(&disp_cfg);
    ESP_RETURN_ON_FALSE(display_lvgl != NULL, ESP_FAIL, TAG, "No se pudo registrar display LVGL");

    esp_err_t err_touch = inicializar_touch();
    if (err_touch == ESP_OK) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = display_lvgl,
            .handle = touch,
        };
        lv_indev_t *entrada_touch = lvgl_port_add_touch(&touch_cfg);
        ESP_RETURN_ON_FALSE(entrada_touch != NULL, ESP_FAIL, TAG, "No se pudo registrar touch LVGL");
    } else {
        ESP_LOGW(TAG, "Touch no inicializado (%s). La UI sigue visible sin entrada tactil.",
                 esp_err_to_name(err_touch));
    }

    return ESP_OK;
}

lv_display_t *nera_display_obtener(void)
{
    return display_lvgl;
}
