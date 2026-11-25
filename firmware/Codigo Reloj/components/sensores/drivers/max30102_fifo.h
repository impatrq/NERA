#pragma once
#include <stddef.h>
#include <stdint.h>
namespace nera_max {
// FUNCIONAL: slot 1 rojo, slot 2 IR. Dos canales = 6 bytes por muestra.
constexpr size_t FRAME_BYTES=6;
constexpr uint8_t SLOT_RED_IR=0x21;
constexpr uint32_t red_from_frame(const uint8_t *frame) {
    return (uint32_t(frame[0]&0x03)<<16) | (uint32_t(frame[1])<<8) | frame[2];
}
constexpr uint32_t ir_from_frame(const uint8_t *frame) {
    return (uint32_t(frame[3]&0x03)<<16) | (uint32_t(frame[4])<<8) | frame[5];
}
}
