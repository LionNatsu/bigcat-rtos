#include "everything.h"

pid_controller::pid_controller(float Kp, float Ki, float Kd, float Maxi) {
    kp = Kp;
    ki = Ki;
    kd = Kd;
    max_i = Maxi;
}
float pid_controller::update(float e) {
    float u;
    if (sum_e > -max_i && sum_e < max_i) {
        sum_e += ki * e;
    }
    u = kp * e + sum_e + kd * (e - prev_e);
    prev_e = e;
    return u;
}
float pid_controller::update(float e, float de) {
    float u;
    if (sum_e > -max_i && sum_e < max_i) {
        sum_e += ki * e;
    }
    u = kp * e + sum_e + kd * de;
    prev_e = e;
    return u;
}