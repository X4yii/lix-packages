#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <lix/syscall.h>

extern int64_t __lix_syscall1(uint64_t n, uint64_t a1);

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!req || req->tv_nsec < 0 || req->tv_nsec >= 1000000000L) {
        errno = EINVAL;
        return -1;
    }
    uint64_t ms = (uint64_t)req->tv_sec * 1000 + (uint64_t)req->tv_nsec / 1000000;
    if (ms == 0 && req->tv_nsec > 0) ms = 1;
    __lix_syscall1(SYS_SLEEP, ms);
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    return 0;
}

unsigned int sleep(unsigned int seconds) {
    struct timespec req = { .tv_sec = (time_t)seconds, .tv_nsec = 0 };
    nanosleep(&req, NULL);
    return 0;
}

int usleep(unsigned long usec) {
    struct timespec req = {
        .tv_sec = (time_t)(usec / 1000000),
        .tv_nsec = (long)((usec % 1000000) * 1000)
    };
    return nanosleep(&req, NULL);
}

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
    (void)clk_id;
    if (!tp) {
        errno = EINVAL;
        return -1;
    }
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    uint64_t tsc = ((uint64_t)hi << 32) | lo;
    uint64_t tsc_freq = 2000000000ULL;
    uint64_t sec = tsc / tsc_freq;
    uint64_t nsec = ((tsc % tsc_freq) * 1000000000ULL) / tsc_freq;
    if (clk_id == CLOCK_REALTIME) {
        tp->tv_sec = 1730000000L + (time_t)sec;
    } else {
        tp->tv_sec = (time_t)sec;
    }
    tp->tv_nsec = (long)nsec;
    return 0;
}

time_t time(time_t *tloc) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    if (tloc) *tloc = ts.tv_sec;
    return ts.tv_sec;
}

clock_t clock(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (clock_t)(ts.tv_sec * CLOCKS_PER_SEC + ts.tv_nsec / (1000000000L / CLOCKS_PER_SEC));
}
