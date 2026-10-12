#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>

typedef unsigned long pthread_t;

typedef struct {
    size_t stacksize;
    int detachstate;
} pthread_attr_t;

typedef struct {
    int lock;
} pthread_mutex_t;

typedef struct {
    int type;
} pthread_mutexattr_t;

typedef struct {
    int dummy;
} pthread_cond_t;

typedef struct {
    int clock_id;
} pthread_condattr_t;

typedef unsigned int pthread_key_t;
typedef int pthread_once_t;

pthread_t pthread_self(void) {
    return 1;
}

int pthread_equal(pthread_t t1, pthread_t t2) {
    return t1 == t2;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    (void)attr;
    (void)start_routine;
    (void)arg;
    if (thread) {
        *thread = 1;
    }
    return 0;
}

int pthread_join(pthread_t thread, void **retval) {
    (void)thread;
    if (retval) {
        *retval = NULL;
    }
    return 0;
}

int pthread_detach(pthread_t thread) {
    (void)thread;
    return 0;
}

void pthread_exit(void *retval) {
    (void)retval;
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

int pthread_attr_init(pthread_attr_t *attr) {
    if (!attr) return EINVAL;
    attr->stacksize = 2 * 1024 * 1024;
    attr->detachstate = 0;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
    if (!attr) return EINVAL;
    attr->stacksize = stacksize;
    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize) {
    if (!attr || !stacksize) return EINVAL;
    *stacksize = attr->stacksize;
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate) {
    if (!attr) return EINVAL;
    attr->detachstate = detachstate;
    return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    (void)attr;
    if (!mutex) return EINVAL;
    mutex->lock = 0;
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex) {
    (void)mutex;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
    if (!mutex) return EINVAL;
    while (__sync_lock_test_and_set(&mutex->lock, 1)) {}
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
    if (!mutex) return EINVAL;
    if (__sync_lock_test_and_set(&mutex->lock, 1) == 0) {
        return 0;
    }
    return EBUSY;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    if (!mutex) return EINVAL;
    __sync_lock_release(&mutex->lock);
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
    if (!attr) return EINVAL;
    attr->type = 0;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
    if (!attr) return EINVAL;
    attr->type = type;
    return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
    (void)cond;
    (void)attr;
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond) {
    (void)cond;
    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond) {
    (void)cond;
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
    (void)cond;
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    (void)cond;
    pthread_mutex_unlock(mutex);
    pthread_mutex_lock(mutex);
    return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const void *abstime) {
    (void)abstime;
    return pthread_cond_wait(cond, mutex);
}

int pthread_condattr_init(pthread_condattr_t *attr) {
    if (!attr) return EINVAL;
    attr->clock_id = 0;
    return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, int clock_id) {
    if (!attr) return EINVAL;
    attr->clock_id = clock_id;
    return 0;
}

static void *key_storage[64];
static int key_used[64] = {0};

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
    (void)destructor;
    if (!key) return EINVAL;
    for (int i = 0; i < 64; ++i) {
        if (!key_used[i]) {
            key_used[i] = 1;
            key_storage[i] = NULL;
            *key = (pthread_key_t)i;
            return 0;
        }
    }
    return EAGAIN;
}

int pthread_key_delete(pthread_key_t key) {
    if (key >= 64) return EINVAL;
    key_used[key] = 0;
    return 0;
}

void *pthread_getspecific(pthread_key_t key) {
    if (key >= 64 || !key_used[key]) return NULL;
    return key_storage[key];
}

int pthread_setspecific(pthread_key_t key, const void *value) {
    if (key >= 64 || !key_used[key]) return EINVAL;
    key_storage[key] = (void *)value;
    return 0;
}

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
    if (!once_control || !init_routine) return EINVAL;
    if (__sync_bool_compare_and_swap(once_control, 0, 1)) {
        init_routine();
    }
    return 0;
}

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset) {
    (void)how;
    (void)set;
    (void)oldset;
    return 0;
}

int sched_yield(void) {
    return 0;
}
