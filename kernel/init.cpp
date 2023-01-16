#include "MK60D10.h"
#include "kernel.h"

extern "C" {
#include "uart.h"
}


namespace kern {
    void init() {
        NVIC_SetPriority(PIT1_IRQn, 3);
        NVIC_SetPriority(SVCall_IRQn, 1);
        NVIC_SetPriority(SysTick_IRQn, 0);
        time_init();
        sched_init();
    }
}