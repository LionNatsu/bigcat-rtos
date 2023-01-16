#include "kernel.h"

namespace kern {

    typedef void(syscall_t)();

    void sys_get_tick() {
        kern::get_current_thread().ctx->r0 = kern::get_tick();
    }

    void sys_create_thread() {
        const auto &ctx = kern::get_current_thread().ctx;
        kern::create_thread((void *)ctx->r0, ctx->r1);
    }

    syscall_t *syscall_table[] = {
        kern::init,
        kern::yield,
        kern::sleep,
        sys_get_tick,
        sys_create_thread,
        kern::exit_thread,
        kern::enter_critical,
        kern::leave_critical
    };

    context_t *syscall_dispatcher(context_t *ctx, context_extended_t *ctx2, int svcid) {
        if(svcid != 0) {
            thread_t &tcb = kern::get_current_thread();
            tcb.ctx = ctx;
            tcb.ctx2 = *ctx2;
        } else {
            kern::create_first_thread((void *)ctx->r0, ctx->r1);
        }
        syscall_table[svcid]();
        thread_t &tcb = kern::get_current_thread();
        *ctx2 = tcb.ctx2;
        return tcb.ctx;
    }

}