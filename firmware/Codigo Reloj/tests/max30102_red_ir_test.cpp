#include "../components/sensores/drivers/max30102_fifo.h"
#include "../components/sensores/processing/heart_rate_estimator.h"

constexpr uint8_t frame[]={0x01,0x23,0x45,0x02,0x34,0x56};
static_assert(nera_max::FRAME_BYTES==6 && nera_max::SLOT_RED_IR==0x21);
static_assert(nera_max::red_from_frame(frame)==0x12345,"Red must use the first FIFO slot");
static_assert(nera_max::ir_from_frame(frame)==0x23456,"IR must not be decoded from red bytes");
constexpr uint8_t frames[]={0,0,1,0xFF,0,2, 0,0,3,0,0,4};
static_assert(nera_max::ir_from_frame(frames)==0x30002,"Mask unused ADC bits");
static_assert(nera_max::red_from_frame(frames+nera_max::FRAME_BYTES)==3,"Red frame alignment");
static_assert(nera_max::ir_from_frame(frames+nera_max::FRAME_BYTES)==4,"Frame alignment");

constexpr uint32_t wave(unsigned phase,unsigned period) {
    const unsigned p=phase%period;
    return 80000+(p<period/2 ? int(p)*4800/int(period)-1200 :
                              3600-int(p)*4800/int(period));
}
constexpr bool estimator_test() {
    HeartRateEstimator estimator;
    for(unsigned i=0;i<1200;++i) estimator.add(wave(i,80));
    if(!estimator.valid() || estimator.bpm()<74 || estimator.bpm()>76) return false;
    bool held=false;
    for(unsigned i=0;i<200;++i) {
        const float previous=estimator.bpm();
        const auto age=estimator.samples_since_valid();
        estimator.add(wave(i,170));
        if(estimator.holding()) {
            if (estimator.bpm()!=previous) return false;
            if (estimator.samples_since_valid()!=age+1) return false;
            held=true;
            break;
        }
    }
    if(!held) return false;
    estimator.add(0); // Retiro de dedo: invalidar sin esperar la retencion.
    if(estimator.valid()) return false;
    for(unsigned i=0;i<1200;++i) estimator.add(wave(i,80));
    if(!estimator.valid()) return false;
    for(unsigned i=0;i<400;++i) estimator.add(80000); // Nivel DC sin pulsos.
    if(estimator.valid() || estimator.bpm()!=0) return false;
    for(unsigned i=0;i<1200;++i) estimator.add(wave(i,80));
    estimator.add(HeartRateEstimator::SATURATION_IR);
    return !estimator.valid();
}
static_assert(estimator_test(),"Measured BPM, isolated interval, removal, flat signal and saturation");

constexpr bool weak_signal_test() {
    HeartRateEstimator estimator;
    // Variacion optica pequena sobre un DC alto, con deriva y ruido de pocos bits.
    for(unsigned i=0;i<1800;++i) {
        const int ac=(int(wave(i,80))-80000)/4;
        estimator.add(120000-int(i/3)+ac+int(i%9)-4);
    }
    if(!estimator.valid() || estimator.bpm()<70 || estimator.bpm()>80) return false;
    // Cambio sostenido de ritmo: recuperacion por nuevas muestras, sin fijar 75 BPM.
    for(unsigned i=0;i<1400;++i) estimator.add(wave(i,100));
    if(!estimator.valid() || estimator.bpm()<59 || estimator.bpm()>61) return false;
    estimator.reset();
    for(unsigned i=0;i<1800;++i) estimator.add(120000+int(i%9)-4);
    return !estimator.valid(); // Contacto DC y ruido pequeno no son un pulso.
}
static_assert(weak_signal_test(),"Weak optical modulation, drift, rate change and DC noise");
