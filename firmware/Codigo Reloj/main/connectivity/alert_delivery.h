#pragma once
#include "../../../Protocolo Reloj Vincha/nera_lab_protocol.h"
#include <stdint.h>

// Dos estados actuales, nunca una cola historica de ordenes de activacion.
namespace nera_delivery {
void set_link(bool connected);
bool phone_ready(int64_t now);
void publish(nera_lab_packet event);
void phase(unsigned metric, uint8_t phase);
bool snapshot(unsigned metric, nera_lab_packet &event);
bool receive(const nera_lab_packet &receipt, int64_t now);
bool approved(unsigned metric, int64_t now);
}
