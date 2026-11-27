#include "../../Protocolo Reloj Vincha/nera_delivery_state.h"
#include "../../Protocolo Reloj Vincha/nera_lab_protocol.h"
#include "../main/connectivity/lab_alert_rules.h"
#include "../main/connectivity/user_alert_preferences.h"
using namespace nera_lab;
constexpr bool rules_test() {
    Rule heart(HEART_ON,HEART_OFF);
    if (heart.update(true,101,1000000)) return false;
    // Repetir la misma muestra no suma tiempo.
    for (int i=0;i<50;++i) if (heart.update(true,101,1000000)) return false;
    if (heart.update(true,100,6000000)) return false; // igualdad no activa
    if (heart.update(true,110,7000000)) return false;
    if (heart.update(false,110,8000000)) return false; // invalida rompe continuidad
    if (heart.update(true,110,9000000)) return false;
    if (heart.update(true,110,13999999)) return false;
    if (heart.update(true,110,14000000)!=1) return false;
    if (heart.update(true,110,15000000) || heart.update(true,95,16000000)) return false;
    if (heart.update(true,90,17000000)) return false; // igualdad mantiene histeresis
    if (heart.update(true,89,18000000)!=2) return false;
    Rule temp(TEMP_ON,TEMP_OFF);
    if (temp.update(true,34,1000000) || temp.update(true,34,5999999)) return false;
    if (temp.update(true,34,6000000)!=1) return false;
    if (temp.update(true,32,7000000)) return false;
    if (temp.update(false,0,0)!=3 || temp.update(false,0,0)) return false;
    return true;
}
static_assert(rules_test(),"Duration, repeated samples, invalidation, hysteresis and single transition");
constexpr bool preferences_test() {
    using namespace nera_preferences;
    if (!valid({1,1,2,1}) || valid({2,1,0,0}) || valid({1,2,0,0}) ||
        valid({1,1,3,0}) || valid({1,1,0,2})) return false;
    if (eligible({1,0,0,0},0) || eligible({1,0,0,0},1) ||
        eligible({1,1,0,1},0) || !eligible({1,1,0,1},1)) return false;
    Rule rule(HEART_ON,HEART_OFF);
    if (rule.update(true,110,1000000,hold_us({1,1,2,0}))) return false;
    if (rule.update(true,110,20000000,hold_us({1,1,2,0}))) return false;
    if (rule.update(true,110,21000000,hold_us({1,1,2,0}))!=1) return false;
    Rule changed(HEART_ON,HEART_OFF);
    changed.update(true,110,1000000);
    changed.reset_qualification();
    if (changed.update(true,110,6000000) || changed.update(true,110,10999999)) return false;
    return changed.update(true,110,11000000)==1;
}
static_assert(preferences_test(),"Config validation, exercise gating and profile duration");
constexpr bool protocol_test() {
    nera_lab_packet input={NERA_LAB_ALERT,NERA_LAB_TEMP,NERA_LAB_ACTIVE,0x12345678,9,-1234};
    uint8_t bytes[NERA_LAB_SIZE]={};
    nera_lab_encode(bytes,&input);
    nera_lab_packet output={};
    if (!nera_lab_decode(bytes,sizeof(bytes),&output) || output.session!=input.session ||
        output.id!=9 || output.centi_value!=-1234 || output.state!=NERA_LAB_ACTIVE) return false;
    if (nera_lab_decode(bytes,sizeof(bytes)-1,&output)) return false;
    bytes[4]=1; if (nera_lab_decode(bytes,sizeof(bytes),&output)) return false;
    bytes[4]=3; bytes[6]=3; if (nera_lab_decode(bytes,sizeof(bytes),&output)) return false;
    bytes[6]=1; bytes[7]=9; if (nera_lab_decode(bytes,sizeof(bytes),&output)) return false;
    return true;
}
static_assert(protocol_test(),"Wire format, negative value, truncated frame, version and invalid fields");

constexpr bool delivery_test() {
    NeraDeliveryState state;
    nera_lab_packet ready={NERA_PHONE_READY,1,0,0,0,0};
    nera_lab_packet event={NERA_LAB_ALERT,1,1,99,1,11000};
    if (state.receive(ready,1000000)) return false;
    state.set_link(true);
    if (!state.receive(ready,1000000) || !state.phone_ready(1000000)) return false;
    state.publish(event);
    if (state.approved(1,1000000) || state.events[0].type!=NERA_WAIT_APP) return false;
    event.type=NERA_APP_RECEIPT; event.id=2;
    if (state.receive(event,1000000)) return false;
    event.id=1; event.centi_value=10000;
    if (state.receive(event,1000000)) return false;
    event.centi_value=11000;
    if (!state.receive(event,1000000) || !state.approved(1,1000000)) return false;
    if (state.approved(2,1000000) || state.approved(1,4000000)) return false;
    state.set_link(false); state.set_link(true);
    if (state.approved(1,2000000)) return false;
    state.receive(ready,2000000);
    event.id=2; event.type=NERA_LAB_ALERT; state.publish(event);
    event.type=NERA_APP_RECEIPT; event.id=1;
    if (state.receive(event,2000000)) return false;
    event.id=2; state.events[0].type=NERA_FAILED;
    return !state.receive(event,2000000);
}
static_assert(delivery_test(),"No activation before app receipt; expiry, reconnect, replaced and failed events");
