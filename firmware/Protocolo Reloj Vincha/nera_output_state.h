#pragma once
#include "nera_lab_protocol.h"
#ifdef __cplusplus
#define NERA_STATE_FN static constexpr
#else
#define NERA_STATE_FN static inline
#endif
enum { NERA_OUTPUT_LEASE_US=5000000 };
typedef struct {
    nera_lab_packet last[2];
    uint8_t seen[2], active[2];
    int64_t until[2];
} nera_output_state;
NERA_STATE_FN int nera_output_on(const nera_output_state *s) {
    return s->active[0] || s->active[1];
}
NERA_STATE_FN void nera_output_expire(nera_output_state *s, int64_t now) {
    for (unsigned i=0;i<2;++i) if (s->active[i] && now>=s->until[i]) s->active[i]=0;
}
NERA_STATE_FN int nera_output_receive(nera_output_state *s, const nera_lab_packet *p, int64_t now) {
    if (p->type!=NERA_LAB_ALERT || p->metric<1 || p->metric>2 || !p->id || !p->session || p->state>2) return 0;
    nera_output_expire(s,now);
    unsigned i=p->metric-1;
    if (s->seen[i] && s->last[i].session==p->session) {
        if (p->id<s->last[i].id) return 0;
        if (p->id==s->last[i].id && (p->state!=s->last[i].state ||
            p->centi_value!=s->last[i].centi_value ||
            (p->state==NERA_LAB_ACTIVE && !s->active[i]))) return 0;
    }
    // Un reinicio del reloj cancela ambas ordenes de la sesion anterior.
    for (unsigned j=0;j<2;++j) if (s->seen[j] && s->last[j].session!=p->session) {
        s->active[j]=0; s->seen[j]=0;
    }
    s->last[i]=*p; s->seen[i]=1;
    s->active[i]=p->state==NERA_LAB_ACTIVE;
    s->until[i]=now+NERA_OUTPUT_LEASE_US;
    return 1;
}
#undef NERA_STATE_FN
