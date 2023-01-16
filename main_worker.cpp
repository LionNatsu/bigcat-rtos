#include <cstdio>
#include <cmath>

#include "everything.h"
#include "image.h"

extern "C" {
#include "gpio.h"
}

State global_state;
float pitch_default = -38;
float pitch_target = -38;
float speed_target = 0;
float duty_difference = 0;
float yaw_target = 0;


void thread_monitor() {
    GPIO_ToggleBit(HW_GPIOA, 15);
    while(true) {
        sys::sleep(10 * 100);
        printf("%03.1f %03.1f %03.1f %03.1f  \r",
//               pitch_target,
               global_state.pitch,
               global_state.yaw,
//               global_state.roll,
//               hardware::left_duty,
               global_state.pitch_rate,
               global_state.yaw_rate
//               hardware::right_duty,
//               global_state.speed_right
        );
        GPIO_ToggleBit(HW_GPIOA, 17);
    }
}

void thread_balance_feedback() {
    GPIO_ToggleBit(HW_GPIOA, 16);
    pid_controller balance_control(40, 0, 1, 0);
    while(true) {
        sys::sleep(5 * 100);
        sys::enter_critical();
        auto pitch_error = pitch_target - global_state.pitch;
        auto pitch_error_rate = -global_state.pitch_rate;
        sys::leave_critical();
        //pitch_error = trim(-20, pitch_error, 20);
        auto u = balance_control.update(pitch_error, pitch_error_rate);
        //u = trim(-90, u, 90);
        hardware::left_duty = u + duty_difference;
        hardware::right_duty = u - duty_difference;
    }
}

void thread_speed_feedback() {
    pid_controller speed_control(0.3, 0.01, 0, 20);
    while(true) {
        sys::sleep(23 * 100);
        
        sys::enter_critical();
        auto speed = (global_state.speed_left + global_state.speed_right) / 2;
        sys::leave_critical();
        
        auto speed_error = atanf(speed_target - speed);
        auto u = speed_control.update(speed_error);
        
        pitch_target = pitch_default - u;
    }
}

void thread_direction_feedback() {
    pid_controller direction_control(0.1, 0, 0.0001, 0);
    while(true) {
        sys::sleep(7 * 100);
        
        sys::enter_critical();
        auto yaw = global_state.yaw;
        auto yaw_error_rate = global_state.yaw_rate;
        sys::leave_critical();
        
        auto yaw_error = yaw_target - yaw;
        auto u = direction_control.update(yaw_error, yaw_error_rate);
        
        duty_difference = -u;
    }
}

//void get_rotation(){
//    while(true) {
//        sys::sleep(10 * 100);
//        if (video::state == video::SCAN_COMPLETE) {
//            auto angle = image_process();
//            printf("%f \r", angle.now);
//        }
//    }
//}

void thread_left_turn() {
    sys::sleep(1000 * 100);
    yaw_target = -20;
    while(true) {
        sys::sleep(500 * 100);
        GPIO_ToggleBit(HW_GPIOA, 17);
        GPIO_ToggleBit(HW_GPIOA, 16);
    }
}

void main_worker() {
    sys::create_thread(thread_attitude_resolution, 2048);
    sys::sleep(500 * 100);  // Wait for converging
    sys::create_thread(thread_monitor, 1024);
    sys::create_thread(hardware::thread_motor, 512);
    sys::create_thread(thread_balance_feedback, 512);
    sys::create_thread(thread_speed_feedback, 512);
//    sys::create_thread(thread_direction_feedback, 512);
    sys::create_thread(thread_left_turn, 512);
    //sys::create_thread(get_rotation,10240);
}
