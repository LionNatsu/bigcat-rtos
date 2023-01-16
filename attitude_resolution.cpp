#include <cmath>
#include <cstdio>

#include "everything.h"
#include "bigcat-blas.h"


namespace hardware {
    const blas::vector<3> accelerometer();
    const blas::vector<3> magnetometer();
    const blas::vector<3> gyroscope();
}

void Madgwick_update(blas::quaternion &, const float);

void thread_attitude_resolution() {
    LoopDuration dur(0.01);
    blas::quaternion q = {1, 0, 0, 0};
    while (true) {
//        printf("%s\r\n", hardware::gyroscope().to_string());
        Madgwick_update(q, dur.measure());
        sys::enter_critical();
        auto euler = q.to_euler_angles();
        global_state.pitch = euler(1);
        global_state.yaw = euler(0);
        sys::leave_critical();
    }
}

void Madgwick_update(blas::quaternion &q, const float duration) {
    using blas::quaternion;
    using blas::vector;
    using blas::matrix;

    const float beta = 0.8;
    const bool
        USE_GYR = true,
        USE_ACC = true,
        USE_MAG = false;

    vector<3> gyr;
    
    quaternion gradient;
    
    if (USE_ACC || USE_MAG) {
        if (USE_ACC) {
            const vector<3> acc = hardware::accelerometer().normalized();
            const vector<3> acc_expect = q.rotate_inv({0, 0, 1});
            const quaternion acc_error = vector<3>(acc_expect - acc);
            const matrix<4> acc_jacobian = q.rotate_inv_jacobian({0, 0, 1});
            gradient += vector<4>(acc_jacobian.trans() * acc_error.to_vector());
        }
        if (USE_MAG) {
            const vector<3> mag = hardware::magnetometer().normalized();
            const vector<3> mag_0 = q.rotate(mag);
            const float mag_h = sqrtf(mag_0(0) * mag_0(0) + mag_0(1) * mag_0(1));
            const float mag_v = mag_0(2);
            const vector<3> mag_expect = q.rotate_inv({mag_h, 0, mag_v});
            const quaternion mag_error = vector<3>(mag_expect - mag);
            const matrix<4> mag_jacobian = q.rotate_inv_jacobian({mag_h, 0, mag_v});
            gradient += vector<4>(mag_jacobian.trans() * mag_error.to_vector());
        }
        gradient = gradient.normalized();
    }
    
    if (USE_GYR) {
        gyr = hardware::gyroscope();
        sys::enter_critical();
        global_state.pitch_rate = gyr(1);
        global_state.yaw_rate = gyr(2);
        sys::leave_critical();
    }
    
    quaternion q_new = 0.5 * q * gyr;
    if (!isnan(gradient.b()))
        q_new += beta * gradient;
    // Approximate the integral
    
    q += q_new * duration;
    q = q.normalized();
}

extern void (*ICM20948_Read)(uint8_t, uint8_t *, uint32_t);
extern void (*AK09916_Read)(uint8_t, uint8_t *, uint32_t);
extern void (*MPU6050_Read)(uint8_t, uint8_t *, uint32_t);
namespace hardware {
    blas::matrix<3> direction_correction = {
        {  0, -1,  0 },
        { -1,  0,  0 },
        {  0,  0,  1 }
    };
    
    static float int16_to_float(uint8_t *buf) {
        return (int16_t)(buf[0] * 256 + buf[1]) / 32768.0;
    }
    
    const blas::vector<3> accelerometer() {
        uint8_t buf[6] = {};
//        ICM20948_Read(0x2d, buf, 6);
        MPU6050_Read(0x3b, buf, 6);
        const float scale = 2;
        return direction_correction * blas::vector<3> {
            int16_to_float(&buf[0]) * scale,
            int16_to_float(&buf[2]) * scale,
            int16_to_float(&buf[4]) * scale
        };
    } 

    const blas::vector<3> gyroscope() {
        const float PI = 3.141593f;
        const float DEG2ARC = PI/180;
        uint8_t buf[6];
//        ICM20948_Read(0x33, buf, 6);
        MPU6050_Read(0x43, buf, 6);
        const float scale = 250;
        return direction_correction * blas::vector<3> {
            -int16_to_float(&buf[0]) * scale * DEG2ARC,
            -int16_to_float(&buf[2]) * scale * DEG2ARC,
            int16_to_float(&buf[4]) * scale * DEG2ARC
        };
    }

    const blas::vector<3> magnetometer() {
        uint8_t buf[6];
        AK09916_Read(0x11, buf, 6);
        uint8_t overflow_flag;
        AK09916_Read(0x18, &overflow_flag, 1);  // Must read after every measurement.
        overflow_flag &= 0x8;
        const float scale = 0.15;
        static blas::vector<3> prev;
        blas::vector<3> cur = {
            int16_to_float(&buf[0]) * scale,
            int16_to_float(&buf[2]) * scale,
            int16_to_float(&buf[4]) * scale
        };
        return direction_correction * 0.5 * (prev + cur);
    }
}
