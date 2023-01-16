#include <cstdint>
#include "everything.h"

extern "C" {
#include "ftm.h"
}

namespace hardware {
    uint8_t l_a, l_b, r_a, r_b;
    float left_duty = 0;
    float right_duty = 0;
        
    void motor_init() {
        FTM_PWM_ChangeDuty(r_a, 1, 1000);
    }
    
    void left_motor(float duty) {
        duty = duty * -50;
        int out = (int)trim(-4500, (int)duty, 4500);
        FTM_PWM_ChangeDuty(l_a, 0, 5000 - out);
        FTM_PWM_ChangeDuty(l_b, 2, 5000 + out);
    }
    void right_motor(float duty) {
        duty = duty * -50;
        int out = (int)trim(-4500, (int)duty, 4500);
        FTM_PWM_ChangeDuty(r_a, 1, 5000 - out);
        FTM_PWM_ChangeDuty(r_b, 3, 5000 + out);
    }
    void thread_motor() {
        while(true) {
            sys::sleep(10 * 100);
            if (global_state.pitch > -4 || global_state.pitch < -60) {
                left_motor(0);
                right_motor(0);
                sys::sleep(2000 * 100);
                continue;
            }
            left_motor(trim_mid(-20, left_duty, 20));
            right_motor(trim_mid(-20, right_duty, 20));
        }
    }
}

