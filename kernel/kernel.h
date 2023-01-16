#ifndef KERNEL_H
#define KERNEL_H

#include <cstdint>
#include <vector>

struct context_t {
    uint32_t r0, r1, r2, r3, r12;
    void *lr, *pc;
    uint32_t psr;
};

struct context_extended_t {
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11;
};

enum thread_state_t {
    RUNNING,
    READY,
    SLEEPING,
    BLOCKING,
    DEAD,
};

struct thread_t {
    thread_state_t state;
    context_t *ctx;
    context_extended_t ctx2;
    uint8_t *stack_base;
};


namespace kern {
    context_t *syscall_dispatcher(context_t *ctx, context_extended_t *ctx2, int svcid);

    void init();
    void time_init();
    void sched_init();
    void create_first_thread(void *start_address, int stack_size);
    
    void yield();
    void sleep();
    int get_tick();
    void create_thread(void *start_address, int stack_size);
    void exit_thread();
    
    void enter_critical();
    void leave_critical();
    
    
    extern int tick;
    extern std::vector<thread_t> thread_pool;
    extern int current_tid;
    thread_t &get_current_thread();
}
#endif