#include "kernel.h"

extern "C" {
#include "pit.h"
    
    // System call exception routine
    context_t *syscall_wrapper(context_t* ctx, int svcid);
    void SVC_Handler() {
        asm volatile (
            "mrs    r0, psp         \n"
            "ldr    r1, [r0, #0x18] \n"
            "ldrb   r1, [r1, #-2]   \n"
            "push   {lr}            \n"
            "blx    syscall_wrapper \n"
            "pop    {lr}            \n"
            "msr    psp, r0         \n"
            "dsb                    \n"
            "isb                    \n"
            "bx     lr"
        );
    }
    // Scheduling exception routine
    context_t *sched_wrapper(context_t *);
    void PIT1_IRQHandler() {
        asm volatile (
            "mrs    r0, psp         \n"
            "push   {lr}            \n"
            "blx    sched_wrapper   \n"
            "pop    {lr}            \n"
            "msr    psp, r0         \n"
            "dsb                    \n"
            "isb                    \n"
            "bx     lr"
        );
    }
    
    #pragma required=SVC_Handler
    context_t *syscall_wrapper(context_t *ctx, int svcid) {
        context_extended_t *ctx2;
        if (svcid > 10) {
            return nullptr;
        }
        
        asm volatile (
            "push {r4-r11} \n"
            "mov %0, sp"
        : "=r"(ctx2));
        ctx = kern::syscall_dispatcher(ctx, ctx2, svcid);
        asm volatile ("pop {r4-r11}");
        return ctx;
    }
    #pragma required=PIT1_IRQHandler
    context_t *sched_wrapper(context_t *ctx) {
        context_extended_t *ctx2;
        asm volatile (
            "push {r4-r11} \n"
            "mov %0, sp"
        : "=r"(ctx2));
        ctx = kern::syscall_dispatcher(ctx, ctx2, 1);
        PIT->CHANNEL[1].TFLG |= PIT_TFLG_TIF_MASK;
        asm volatile ("pop {r4-r11}");
        return ctx;
    }
    
    // SysTick exception routine
    void SysTick_Handler() {
        kern::tick++;
    }
}