#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// FUNCIONAL: LVGL pide su pool una vez. lvgl_port_init lo reserva antes en PSRAM.
// IMPORTANTE: esto no es el buffer DMA que se envia a la pantalla.
void *nera_lvgl_pool_alloc(size_t size);
#ifdef __cplusplus
}
#endif
