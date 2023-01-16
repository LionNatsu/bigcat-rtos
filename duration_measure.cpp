#include "everything.h"

extern "C" {
    #include "common.h"
    #include "SysTick.h"
}

LoopDuration::LoopDuration(float start) {
    duration = start;
    scale = 0.00001;
    prev_tick = 0;
};

float LoopDuration::measure() {
    uint32_t current_tick = sys::get_tick();
    float new_duration = (current_tick - prev_tick) * scale;
    if (new_duration > 0 && new_duration < 1)
        duration = new_duration;
    prev_tick = current_tick;
    return duration;
}