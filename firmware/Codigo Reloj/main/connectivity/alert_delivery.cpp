#include "connectivity/alert_delivery.h"
#include "../../../Protocolo Reloj Vincha/nera_delivery_state.h"
#include "freertos/FreeRTOS.h"

namespace nera_delivery {
namespace {
portMUX_TYPE lock=portMUX_INITIALIZER_UNLOCKED;
NeraDeliveryState state;
}
void set_link(bool connected) {
    portENTER_CRITICAL(&lock);
    state.set_link(connected);
    portEXIT_CRITICAL(&lock);
}
bool phone_ready(int64_t now) {
    portENTER_CRITICAL(&lock);
    const bool ready=state.phone_ready(now);
    portEXIT_CRITICAL(&lock);
    return ready;
}
void publish(nera_lab_packet event) {
    portENTER_CRITICAL(&lock);
    state.publish(event);
    portEXIT_CRITICAL(&lock);
}
void phase(unsigned metric, uint8_t value) {
    if (metric<1 || metric>2) return;
    portENTER_CRITICAL(&lock);
    state.events[metric-1].type=value;
    portEXIT_CRITICAL(&lock);
}
bool snapshot(unsigned metric, nera_lab_packet &event) {
    if (metric<1 || metric>2) return false;
    portENTER_CRITICAL(&lock);
    event=state.events[metric-1];
    portEXIT_CRITICAL(&lock);
    return event.id!=0;
}
bool receive(const nera_lab_packet &receipt, int64_t now) {
    portENTER_CRITICAL(&lock);
    const bool accepted=state.receive(receipt,now);
    portEXIT_CRITICAL(&lock);
    return accepted;
}
bool approved(unsigned metric, int64_t now) {
    portENTER_CRITICAL(&lock);
    const bool ok=state.approved(metric,now);
    portEXIT_CRITICAL(&lock);
    return ok;
}
}
