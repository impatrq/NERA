#include "max30102.h"
#include "max30102_fifo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace {
constexpr uint8_t ADDRESS = 0x57;
constexpr uint8_t PART_ID = 0xFF, EXPECTED_PART_ID = 0x15;
constexpr uint8_t FIFO_WR_PTR = 0x04, FIFO_OVF = 0x05, FIFO_RD_PTR = 0x06;
constexpr uint8_t FIFO_DATA = 0x07, FIFO_CONFIG = 0x08, MODE_CONFIG = 0x09;
constexpr uint8_t SPO2_CONFIG = 0x0A, RED_LED = 0x0C, IR_LED = 0x0D;
constexpr uint8_t SLOT_CONFIG = 0x11, SLOT_CONFIG_2 = 0x12;
constexpr TickType_t TIMEOUT = pdMS_TO_TICKS(60);
// MODIFICABLE: rojo tenue para indicar funcionamiento; IR mantiene su corriente.
constexpr uint8_t RED_CURRENT=0x0F, IR_CURRENT=0x1F;

esp_err_t write_reg(i2c_port_t port, uint8_t reg, uint8_t value) {
    const uint8_t packet[2] = {reg, value};
    return i2c_master_write_to_device(port, ADDRESS, packet, sizeof(packet), TIMEOUT);
}
esp_err_t read_regs(i2c_port_t port, uint8_t reg, uint8_t *data, size_t length) {
    return i2c_master_write_read_device(port, ADDRESS, &reg, 1, data, length, TIMEOUT);
}
esp_err_t clear_fifo(i2c_port_t port) {
    esp_err_t err = write_reg(port, FIFO_WR_PTR, 0);
    if (err != ESP_OK) return err;
    err = write_reg(port, FIFO_OVF, 0);
    if (err != ESP_OK) return err;
    return write_reg(port, FIFO_RD_PTR, 0);
}
}

esp_err_t max30102_init(i2c_port_t port) {
    uint8_t id = 0;
    esp_err_t err = read_regs(port, PART_ID, &id, 1);
    if (err != ESP_OK) return err;
    if (id != EXPECTED_PART_ID) {
        ESP_LOGW("NERA/MAX", "PART_ID inesperado: 0x%02X", id);
        return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGI("NERA/MAX", "ACK 0x57; PART_ID=0x%02X compatible MAX30102/MAX3010x", id);

    // Reset de configuracion y espera del bit RESET autolimpiante.
    err = write_reg(port, MODE_CONFIG, 0x40);
    if (err != ESP_OK) return err;
    for (unsigned attempt = 0; attempt < 10; ++attempt) {
        vTaskDelay(pdMS_TO_TICKS(10));
        uint8_t mode = 0;
        err = read_regs(port, MODE_CONFIG, &mode, 1);
        if (err != ESP_OK) return err;
        if (!(mode & 0x40)) break;
        if (attempt == 9) return ESP_ERR_TIMEOUT;
    }
    err = write_reg(port, MODE_CONFIG, 0x80); // Apagado mientras se configura.
    if (err != ESP_OK) return err;
    err = write_reg(port, FIFO_CONFIG, 0x10); // Sin promedio; FIFO circular.
    if (err != ESP_OK) return err;
    // Rango 4096 nA, 100 muestras/s, 18 bits (411 us).
    err = write_reg(port, SPO2_CONFIG, 0x27);
    if (err != ESP_OK) return err;
    err = write_reg(port, RED_LED, RED_CURRENT);
    if (err != ESP_OK) return err;
    // 0x1F equivale a ~6.2 mA pico tipicos; no aumentar sin medir el modulo.
    err = write_reg(port, IR_LED, IR_CURRENT);
    if (err != ESP_OK) return err;
    err = write_reg(port, SLOT_CONFIG, nera_max::SLOT_RED_IR);
    if (err != ESP_OK) return err;
    err = write_reg(port, SLOT_CONFIG_2, 0x00);
    if (err != ESP_OK) return err;
    err = clear_fifo(port);
    if (err != ESP_OK) return err;
    err = write_reg(port, MODE_CONFIG, 0x07); // Rojo e IR, en slots separados.
    if (err != ESP_OK) return err;
    // FUNCIONAL: un ACK no demuestra que se conserve la configuracion.
    const uint8_t expected[][2] = {{MODE_CONFIG,0x07},{FIFO_CONFIG,0x10},
        {SPO2_CONFIG,0x27},{RED_LED,RED_CURRENT},{IR_LED,IR_CURRENT},
        {SLOT_CONFIG,nera_max::SLOT_RED_IR},{SLOT_CONFIG_2,0x00}};
    for (const auto &item : expected) {
        uint8_t actual=0;
        err=read_regs(port,item[0],&actual,1);
        if (err!=ESP_OK) return err;
        if (actual!=item[1]) {
            ESP_LOGW("NERA/MAX","CONFIG no retenida reg=0x%02X esperado=0x%02X recibido=0x%02X",
                     item[0],item[1],actual);
            return ESP_ERR_INVALID_STATE;
        }
    }
    ESP_LOGI("NERA/MAX","CONFIG ROJO+IR verificada: 100 muestras/s, 6 bytes/muestra");
    return ESP_OK;
}

esp_err_t max30102_read_fifo(i2c_port_t port, Max30102Sample *samples,
                             size_t capacity, size_t *sample_count) {
    if (!samples || !sample_count || capacity < MAX30102_FIFO_DEPTH)
        return ESP_ERR_INVALID_ARG;
    *sample_count = 0;
    uint8_t pointers[3] = {};
    esp_err_t err = read_regs(port, FIFO_WR_PTR, pointers, sizeof(pointers));
    if (err != ESP_OK) return err;
    const uint8_t wr = pointers[0] & 0x1F, overflow = pointers[1] & 0x1F;
    const uint8_t rd = pointers[2] & 0x1F;
    if (overflow) {
        err=clear_fifo(port);
        return err==ESP_OK ? ESP_ERR_INVALID_STATE : err;
    }
    const size_t available = (wr - rd) & 0x1F;
    if (available == 0) return ESP_OK;
    uint8_t bytes[MAX30102_FIFO_DEPTH * nera_max::FRAME_BYTES];
    err = read_regs(port, FIFO_DATA, bytes, available * nera_max::FRAME_BYTES);
    if (err != ESP_OK) return err;
    for (size_t i = 0; i < available; ++i) {
        const uint8_t *frame = bytes+i*nera_max::FRAME_BYTES;
        samples[i] = {nera_max::red_from_frame(frame), nera_max::ir_from_frame(frame)};
    }
    *sample_count = available;
    return ESP_OK;
}
