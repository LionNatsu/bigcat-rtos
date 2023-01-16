#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

namespace sys {
    void kernel_init(void(*)(), int);
    
    void yield();
    void sleep(int ms);
    int get_tick();
    void create_thread(void (*)(), int);
    void exit_thread();
    void enter_critical();
    void leave_critical();
}

#endif