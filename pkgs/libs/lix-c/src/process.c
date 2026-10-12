#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <lix/syscall.h>

void _exit(int status);

extern int64_t __lix_syscall0(uint64_t n);
extern int64_t __lix_syscall1(uint64_t n, uint64_t a1);
extern int64_t __lix_syscall2(uint64_t n, uint64_t a1, uint64_t a2);
extern int64_t __lix_syscall3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3);
extern int64_t __lix_syscall4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4);
extern int64_t __lix_syscall5(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);

typedef struct {
    void *actions;
} posix_spawn_file_actions_t;

typedef struct {
    short flags;
    pid_t pgroup;
    sigset_t default_signals;
    sigset_t signal_mask;
} posix_spawnattr_t;

int posix_spawn(pid_t *pid, const char *path, const posix_spawn_file_actions_t *file_actions, const posix_spawnattr_t *attrp, char *const argv[], char *const envp[]) {
    (void)file_actions;
    (void)attrp;
    (void)argv;
    (void)envp;
    if (!path) {
        errno = ENOENT;
        return ENOENT;
    }
    size_t len = 0;
    while (path[len]) len++;
    int64_t ret = __lix_syscall2(SYS_PROCESS_SPAWN, (uint64_t)path, (uint64_t)len);
    if (ret < 0) {
        errno = (int)(-ret);
        return errno;
    }
    if (pid) {
        *pid = (pid_t)ret;
    }
    return 0;
}

int posix_spawnp(pid_t *pid, const char *file, const posix_spawn_file_actions_t *file_actions, const posix_spawnattr_t *attrp, char *const argv[], char *const envp[]) {
    return posix_spawn(pid, file, file_actions, attrp, argv, envp);
}

int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions) {
    if (!file_actions) return EINVAL;
    file_actions->actions = NULL;
    return 0;
}

int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions) {
    if (!file_actions) return EINVAL;
    file_actions->actions = NULL;
    return 0;
}

int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions, int fildes, int newfildes) {
    (void)file_actions;
    (void)fildes;
    (void)newfildes;
    return 0;
}

int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions, int fildes) {
    (void)file_actions;
    (void)fildes;
    return 0;
}

int posix_spawnattr_init(posix_spawnattr_t *attr) {
    if (!attr) return EINVAL;
    attr->flags = 0;
    attr->pgroup = 0;
    return 0;
}

int posix_spawnattr_destroy(posix_spawnattr_t *attr) {
    if (!attr) return EINVAL;
    return 0;
}

int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags) {
    if (!attr) return EINVAL;
    attr->flags = flags;
    return 0;
}

int posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup) {
    if (!attr) return EINVAL;
    attr->pgroup = pgroup;
    return 0;
}

pid_t fork(void) {
    errno = ENOSYS;
    return -1;
}

pid_t vfork(void) {
    errno = ENOSYS;
    return -1;
}

int execve(const char *path, char *const argv[], char *const envp[]) {
    pid_t pid;
    int err = posix_spawn(&pid, path, NULL, NULL, argv, envp);
    if (err != 0) {
        errno = err;
        return -1;
    }
    _exit(0);
    return 0;
}

int execv(const char *path, char *const argv[]) {
    return execve(path, argv, NULL);
}

int execvp(const char *file, char *const argv[]) {
    return execve(file, argv, NULL);
}

pid_t waitpid(pid_t pid, int *status, int options) {
    int64_t ret = __lix_syscall3(SYS_WAITPID, (uint64_t)pid, (uint64_t)status, (uint64_t)options);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (pid_t)ret;
}

pid_t wait(int *status) {
    return waitpid(-1, status, 0);
}

void _exit(int status) {
    __lix_syscall1(SYS_PROCESS_EXIT, (uint64_t)status);
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

void exit(int status) {
    _exit(status);
}

void abort(void) {
    _exit(134);
}

pid_t getpid(void) {
    return 1;
}

pid_t getppid(void) {
    return 1;
}

uid_t getuid(void) {
    return (uid_t)__lix_syscall0(SYS_GETUID);
}

int setuid(uid_t uid) {
    int64_t ret = __lix_syscall1(SYS_SETUID, (uint64_t)uid);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

gid_t getgid(void) {
    return (gid_t)__lix_syscall0(SYS_GETGID);
}

int setgid(gid_t gid) {
    int64_t ret = __lix_syscall1(SYS_SETGID, (uint64_t)gid);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int kill(pid_t pid, int sig) {
    int64_t ret = __lix_syscall2(SYS_PROCESS_KILL, (uint64_t)pid, (uint64_t)sig);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}
