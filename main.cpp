#include <math.h>
#include <iostream>

#include "everything.h"
#include "FIRE_SCCB.h"


extern "C" {
    #include "gpio.h"
    #include "common.h"
    #include "uart.h"
    #include "i2c.h"
    #include "pit.h"
    #include "ftm.h"
    #include "SysTick.h"
    #include "dma.h"
}

static uint32_t i2c_handle;

template <uint8_t A>
void sensor_read(uint8_t subAddress, uint8_t* buf, uint32_t len) {
    sys::enter_critical();
    I2C_BurstRead(i2c_handle, A, subAddress, 1, buf, len);
    sys::leave_critical();
}

// auto ICM20948_Read = sensor_read<0x68>;
auto MPU6050_Read = sensor_read<0x68>;
auto AK09916_Read = sensor_read<0x0c>;

void AK09916_Init() {
    I2C_WriteSingleRegister(i2c_handle, 0x0c, 0x32, 0x00);  // Reset
    sys::sleep(5 * 100);
    uint8_t dat = 0;
    I2C_ReadSingleRegister(i2c_handle, 0x0c, 0x00, &dat);
    printf("Who am I: 0x%2X\r\n", dat);
    I2C_WriteSingleRegister(i2c_handle, 0x0c, 0x31, 0x08);  // Continuous measurement mode, 100 kHz
    return;
}

void MPU6050_Init() {
    I2C_WriteSingleRegister(i2c_handle, 0x68, 0x6b, 0x87);  // Reset
    sys::sleep(150 * 100);
    uint8_t dat = 0;
    I2C_ReadSingleRegister(i2c_handle, 0x68, 0x75, &dat);
    printf("Who am I: 0x%2X\r\n", dat);
    if (dat != 0x68) {
        while(true) {
            GPIO_ToggleBit(HW_GPIOA, 14);
            sys::sleep(50 * 100);
            GPIO_ToggleBit(HW_GPIOA, 14);
            sys::sleep(500 * 100);
        }
    }
    I2C_WriteSingleRegister(i2c_handle, 0x68, 0x6b, 0x01);  // Wake, set clock source
}

const float lambda = 0.1;
static void PIT_ISR() {
    int v1, v2;
    uint8_t dummy;
    FTM_QD_GetData(HW_FTM1, &v1, &dummy);
    FTM_QD_GetData(HW_FTM2, &v2, &dummy);
    global_state.speed_left = global_state.speed_left * lambda + (int16_t)v1 * (1 - lambda);
    global_state.speed_right = global_state.speed_right * lambda + -(int16_t)v2 * (1 - lambda);
    FTM_QD_ClearCount(HW_FTM1);
    FTM_QD_ClearCount(HW_FTM2);
}

void userspace_main() {
    GPIO_QuickInit(HW_GPIOA, 14, kGPIO_Mode_OPP);
    GPIO_QuickInit(HW_GPIOA, 15, kGPIO_Mode_OPP);
    GPIO_QuickInit(HW_GPIOA, 16, kGPIO_Mode_OPP);
    GPIO_QuickInit(HW_GPIOA, 17, kGPIO_Mode_OPP);
    GPIO_SetBit(HW_GPIOA, 14);
    GPIO_SetBit(HW_GPIOA, 15);
    GPIO_SetBit(HW_GPIOA, 16);
    GPIO_SetBit(HW_GPIOA, 17);
    UART_QuickInit(UART0_RX_PD06_TX_PD07, 115200);
    i2c_handle = I2C_QuickInit(I2C0_SCL_PB02_SDA_PB03, 376000);
    
    FTM_QD_QuickInit(FTM1_QD_PHA_PA08_PHB_PA09, kFTM_QD_NormalPolarity, kQD_CountDirectionEncoding);
    FTM_QD_QuickInit(FTM2_QD_PHA_PA10_PHB_PA11, kFTM_QD_NormalPolarity, kQD_CountDirectionEncoding);
    
    PIT_QuickInit(HW_PIT_CH0, 100 * 1000);
    PIT_CallbackInstall(HW_PIT_CH0, PIT_ISR);
    PIT_ITDMAConfig(HW_PIT_CH0, kPIT_IT_TOF, true);
    
    hardware::l_a = FTM_PWM_QuickInit(FTM0_CH0_PC01, kPWM_EdgeAligned, 10000);
    hardware::l_b = FTM_PWM_QuickInit(FTM0_CH2_PC03, kPWM_EdgeAligned, 10000);
    hardware::r_a = FTM_PWM_QuickInit(FTM0_CH1_PC02, kPWM_EdgeAligned, 10000);
    hardware::r_b = FTM_PWM_QuickInit(FTM0_CH3_PC04, kPWM_EdgeAligned, 10000);
    
    
    printf("\r\n<Serial Bus Test>\r\n\n");
//    ICM20948_Init();
    MPU6050_Init();
    main_worker();
}

int main() {
//    video_init(); 
//    while(true);
    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
    
    sys::kernel_init(userspace_main, 4096);
}