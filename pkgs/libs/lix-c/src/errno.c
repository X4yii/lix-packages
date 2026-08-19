#include <errno.h>

#undef errno
__thread int errno = 0;

int *__errno_location(void) {
    return &errno;
}

int *errno_location(void) {
    return &errno;
}

unsigned long __stack_chk_guard = 0x595e9fbd94fda766;

void __stack_chk_fail(void) {
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
