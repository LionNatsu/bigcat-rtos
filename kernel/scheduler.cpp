#include "MK60D10.h"
#include "kernel.h"
#include "../user_syscall.h"

extern "C" {
    #include "pit.h"
}

#include <cstdint>
#include <cstdlib>
#include <cstdio>

#include <vector>
#include <queue>

namespace kern {
    
    struct sleep_wait_t {
        int tid;
        int until_tick;
    };
    
    std::vector<thread_t> thread_pool;
    auto compare_func = [](sleep_wait_t &L, sleep_wait_t &R) { return L.until_tick >= R.until_tick;};
    std::priority_queue<sleep_wait_t, std::vector<sleep_wait_t>, decltype(compare_func)> sleep_wait_queue(compare_func);
    
    int current_tid = 0;
    bool disable_sched = false;

    thread_t &get_current_thread() {
        if (thread_pool.size() == 0)
            return thread_pool[-1];
        return thread_pool[current_tid];
    }
    
    void sched_init() {
        PIT_QuickInit(HW_PIT_CH1, 100);
        PIT_ITDMAConfig(HW_PIT_CH1, kPIT_IT_TOF, true);
    }
    
    void park() {
        asm ("mov r0, #11");
        asm ("mov r1, #10");
        asm ("mov r2, #9");
        asm ("mov r3, #8");
        asm ("mov r4, #7");
        asm ("mov r5, #6");
        asm ("mov r6, #5");
        asm ("mov r7, #4");
        asm ("mov r8, #3");
        asm ("mov r9, #2");
        asm ("mov r10, #1");
        asm ("mov r11, #0");
        while(true) {
            sys::yield();
        }
    }
    
    void create_first_thread(void *start_address, int stack_size) {
        create_thread(start_address, stack_size);
        get_current_thread().state = RUNNING;
        create_thread((void *)park, 64);
        current_tid = 0;
    }

    void yield() {
        if (disable_sched) return;
        auto &prev_thread = get_current_thread();
        if(prev_thread.state == RUNNING) {
            prev_thread.state = READY;
        }
        
        while(true) {
            if (sleep_wait_queue.size() == 0) break;
            auto sw = sleep_wait_queue.top();
            if (sw.until_tick <= tick) {
                thread_pool[sw.tid].state = READY;
                sleep_wait_queue.pop();
            } else {
                break;
            }
        }
        
        while(true) {
            current_tid = (current_tid+1) % thread_pool.size();
            if (get_current_thread().state == READY) break;
        }
        
        get_current_thread().state = RUNNING;
    }

    void create_thread(void *start_address, int stack_size) {
        auto thread_stack = new uint8_t[stack_size];
        auto new_tid = thread_pool.size();
        memset(thread_stack, 0x61+new_tid, stack_size);
        auto ctx = (context_t *)(thread_stack + stack_size - sizeof(context_t) - 16);
        ctx->r12 = new_tid + 0x10001000;
        ctx->lr = (void *)sys::exit_thread;
        ctx->pc = start_address;
        ctx->psr = 0x01000000;
        thread_t tcb = {
            .state = READY,
            .ctx = ctx,
            .ctx2 = {},
            .stack_base = thread_stack
        };
        thread_pool.push_back(tcb);
    }
    
    void exit_thread() {
        thread_pool[current_tid].state = DEAD;
        delete[] thread_pool[current_tid].stack_base;
        yield();
    }
    
    void enter_critical() {
        disable_sched = true;
    }
    
    void leave_critical() {
        disable_sched = false;
    }
    
    void sleep() {
        int duration = get_current_thread().ctx->r0;
        sleep_wait_queue.push(sleep_wait_t{
            .tid = current_tid,
            .until_tick = tick + duration
        });
        thread_pool[current_tid].state = SLEEPING;
        yield();
    }
}