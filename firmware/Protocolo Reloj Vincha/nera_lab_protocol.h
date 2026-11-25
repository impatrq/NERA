#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
#define NERA_INLINE static constexpr
#else
#define NERA_INLINE static inline
#endif
// FUNCIONAL: formato comun S3/C3 sin padding. Solo laboratorio, sin cifrado.
// IMPORTANTE: recibir estos mensajes nunca autoriza una salida fisica.
enum { NERA_LAB_SIZE=20, NERA_LAB_ALERT=2, NERA_LAB_ACK=3,
       NERA_LAB_HEART=1, NERA_LAB_TEMP=2,
       NERA_LAB_CLEAR=0, NERA_LAB_ACTIVE=1, NERA_LAB_INVALID=2 };
typedef struct {
    uint8_t type, metric, state;
    uint32_t session, id;
    int32_t centi_value;
} nera_lab_packet;
NERA_INLINE void nera_put32(uint8_t *p, uint32_t value) {
    for (unsigned i=0; i<4; ++i) p[i]=(uint8_t)(value >> (8*i));
}
NERA_INLINE uint32_t nera_get32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
NERA_INLINE void nera_lab_encode(uint8_t *p, const nera_lab_packet *e) {
    p[0]='N'; p[1]='E'; p[2]='R'; p[3]='A'; p[4]=2;
    p[5]=e->type; p[6]=e->metric; p[7]=e->state;
    nera_put32(p+8,e->session); nera_put32(p+12,e->id); nera_put32(p+16,(uint32_t)e->centi_value);
}
NERA_INLINE int nera_lab_decode(const uint8_t *p, size_t size, nera_lab_packet *e) {
    if (!p || !e || size!=NERA_LAB_SIZE || p[0]!='N' || p[1]!='E' || p[2]!='R' || p[3]!='A' ||
        p[4]!=2 || (p[5]!=NERA_LAB_ALERT && p[5]!=NERA_LAB_ACK) ||
        p[6]<1 || p[6]>2 || p[7]>2) return 0;
    e->type=p[5]; e->metric=p[6]; e->state=p[7];
    e->session=nera_get32(p+8); e->id=nera_get32(p+12);
    const uint32_t raw=nera_get32(p+16);
    e->centi_value=(int32_t)(raw>INT32_MAX ? (int64_t)raw-4294967296LL : raw);
    return 1;
}
#undef NERA_INLINE
