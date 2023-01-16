#include <queue>
#include "kernel.h"

extern "C" {
#include "SysTick.h"
}

namespace kern {
    int tick;
//    
//    struct wakeup_job_t;
//    
//    std::priority_queue<>
    
    void time_init() {
        SYSTICK_Init(10);
        SYSTICK_Cmd(true);
        SYSTICK_ITConfig(true);
    }
    
    int get_tick() {
        return kern::tick;
    }
}
