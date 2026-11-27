#pragma once
#include "nera_lab_protocol.h"

// Estado puro; el llamador protege el acceso entre tareas BLE/radio.
class NeraDeliveryState {
public:
    nera_lab_packet events[2]={};
    constexpr void set_link(bool connected) {
        linked=connected;
        if (!connected) { ready_at=0; received_at[0]=received_at[1]=0; }
    }
    constexpr bool phone_ready(int64_t now) const { return linked && fresh(ready_at,now); }
    constexpr void publish(nera_lab_packet event) {
        if (event.metric<1 || event.metric>2) return;
        event.type=event.state==NERA_LAB_ACTIVE ? NERA_WAIT_APP : NERA_SENDING;
        events[event.metric-1]=event; received_at[event.metric-1]=0;
    }
    constexpr bool receive(const nera_lab_packet &receipt, int64_t now) {
        if (!linked) return false;
        if (receipt.type==NERA_PHONE_READY && receipt.session==0 && receipt.id==0 &&
            receipt.metric==1 && receipt.state==0 && receipt.centi_value==0) {
            ready_at=now; return true;
        }
        if (receipt.type!=NERA_APP_RECEIPT || receipt.metric<1 || receipt.metric>2) return false;
        const auto &event=events[receipt.metric-1];
        if (!event.id || event.session!=receipt.session || event.id!=receipt.id ||
            event.state!=receipt.state || event.centi_value!=receipt.centi_value ||
            event.type==NERA_FAILED) return false;
        received_at[receipt.metric-1]=now; return true;
    }
    constexpr bool approved(unsigned metric, int64_t now) const {
        return metric>=1 && metric<=2 && phone_ready(now) && fresh(received_at[metric-1],now);
    }
private:
    int64_t received_at[2]={}, ready_at=0;
    bool linked=false;
    static constexpr bool fresh(int64_t at, int64_t now) {
        return at>0 && now>=at && now-at<3000000;
    }
};
