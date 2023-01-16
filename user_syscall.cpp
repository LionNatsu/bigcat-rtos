

namespace sys {
    void kernel_init(void(*userspace_main)(), int stack_size) {
        asm (
             "mov r2, r0        \n"
             "mov r1, sp        \n"
             "msr psp, r1       \n"
             "sub sp, sp, #32   \n"
             "mov r1, #2        \n"
             "msr control, r1   \n"
             "dsb               \n"
             "isb               \n"
             "mov r0, r2        \n"
             "mov r1, r3        \n"
             "svc 0"
             :: "R2" (userspace_main)
                "R3" (stack_size)
        );
    }
    
    
    void yield() {
        asm("svc 1");
    }
    void sleep(int ms) {
        asm("svc 2");
    }
    int get_tick() {
        int r = 0;
        asm(
            "svc 3  \n"
            "mov %0, r0"
            : "=r"(r)
        );
        return r;
    }
    void create_thread(void (*start_address)(), int stack_size) {
        asm("svc 4");
    }
    void exit_thread() {
        asm("svc 5");
    }
    void enter_critical() {
        asm("svc 6");
    }
    void leave_critical() {
        asm("svc 7");
    }
}