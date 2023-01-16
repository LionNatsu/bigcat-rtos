#ifndef EVERYTHING_H
#define EVERYTHING_H

#include <cstdint>
#include "user_syscall.h"

class LoopDuration {
public:
    LoopDuration(float start = 0.01);
    float measure();
private:
    uint32_t prev_tick;
    float scale;
    float duration;
};

struct State {
    float pitch;
    float yaw;
    float pitch_rate;
    float yaw_rate;
    float speed_left;
    float speed_right;
};

extern State global_state;


class pid_controller {
public:
    pid_controller(float Kp, float Ki, float Kd, float max_i);
    float update(float e);
    float update(float e, float de);
private:
    float kp, ki, kd;
    float sum_e = 0, prev_e = 0;
    float max_i = 0;
};

namespace hardware {
    extern uint8_t l_a, l_b, r_a, r_b;
    extern float left_duty, right_duty;
    void motor_init();
    void thread_motor();
}

namespace video {
    namespace hardware {
        const int width = 240;
        const int height = 288;
        const int height_invalid = 28;
        const int pixel_nops = 100;
    }
    const int divider = 2;
    const int width = hardware::width;
    const int height = hardware::height / divider;
    extern uint8_t buffer[height][width];

    enum SCAN_STATE {
        SCAN_STANDBY,
        SCAN_FRAME,
        SCAN_COMPLETE,
    };

    extern SCAN_STATE state;
}

float trim(float minv, float iv, float maxv);

float trim_mid(float neg, float iv, float pos);
void main_worker();
void thread_attitude_resolution();
void video_init();


extern "C" {
#include "FIRE_SCCB.h"
}

#endif