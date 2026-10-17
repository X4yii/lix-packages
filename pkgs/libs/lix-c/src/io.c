#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <lix/syscall.h>

extern int64_t __lix_syscall1(uint64_t n, uint64_t a1);
extern int64_t __lix_syscall2(uint64_t n, uint64_t a1, uint64_t a2);
extern int64_t __lix_syscall3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3);
extern int64_t __lix_syscall5(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);
extern void *memset(void *s, int c, size_t n);

int open(const char *pathname, int flags, ...) {
    int64_t ret = __lix_syscall2(SYS_OPEN, (uint64_t)pathname, (uint64_t)flags);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int close(int fd) {
    int64_t ret = __lix_syscall1(SYS_HANDLE_CLOSE, (uint64_t)fd);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

ssize_t read(int fd, void *buf, size_t count) {
    int64_t ret = __lix_syscall3(SYS_READ, (uint64_t)fd, (uint64_t)buf, (uint64_t)count);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    int64_t ret = __lix_syscall3(SYS_WRITE, (uint64_t)fd, (uint64_t)buf, (uint64_t)count);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (ssize_t)ret;
}

off_t lseek(int fd, off_t offset, int whence) {
    int64_t ret = __lix_syscall3(SYS_LSEEK, (uint64_t)fd, (uint64_t)offset, (uint64_t)whence);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (off_t)ret;
}

int stat(const char *pathname, struct stat *statbuf) {
    if (!statbuf) {
        errno = EFAULT;
        return -1;
    }
    int fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    int ret = fstat(fd, statbuf);
    close(fd);
    return ret;
}

int fstat(int fd, struct stat *statbuf) {
    if (!statbuf) {
        errno = EFAULT;
        return -1;
    }
    memset(statbuf, 0, sizeof(struct stat));
    statbuf->st_mode = S_IFREG | 0644;
    statbuf->st_nlink = 1;
    off_t size = lseek(fd, 0, SEEK_END);
    if (size >= 0) {
        statbuf->st_size = size;
        lseek(fd, 0, SEEK_SET);
    }
    return 0;
}

int lstat(const char *pathname, struct stat *statbuf) {
    return stat(pathname, statbuf);
}

int dup(int oldfd) {
    int64_t ret = __lix_syscall1(SYS_HANDLE_DUP, (uint64_t)oldfd);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int dup2(int oldfd, int newfd) {
    int64_t ret = __lix_syscall2(SYS_HANDLE_DUP2, (uint64_t)oldfd, (uint64_t)newfd);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int pipe(int pipefd[2]) {
    int64_t ret = __lix_syscall1(SYS_PIPE, (uint64_t)pipefd);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int mkdir(const char *pathname, mode_t mode) {
    (void)mode;
    int64_t ret = __lix_syscall1(SYS_MKDIR, (uint64_t)pathname);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int unlink(const char *pathname) {
    int64_t ret = __lix_syscall1(SYS_REMOVE_FILE, (uint64_t)pathname);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int rmdir(const char *pathname) {
    return unlink(pathname);
}

char *getcwd(char *buf, size_t size) {
    if (!buf || size < 2) {
        errno = EINVAL;
        return NULL;
    }
    buf[0] = '/';
    buf[1] = '\0';
    return buf;
}

int chdir(const char *path) {
    (void)path;
    return 0;
}

int access(const char *pathname, int mode) {
    (void)mode;
    struct stat st;
    return stat(pathname, &st);
}

int isatty(int fd) {
    return (fd >= 0 && fd <= 2) ? 1 : 0;
}

int ioctl(int fd, unsigned long request, ...) {
    va_list ap;
    va_start(ap, request);
    uint64_t arg = va_arg(ap, uint64_t);
    va_end(ap);
    int64_t ret = __lix_syscall3(SYS_IOCTL, (uint64_t)fd, (uint64_t)request, arg);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int mount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data) {
    int64_t ret = __lix_syscall5(SYS_MOUNT, (uint64_t)source, (uint64_t)target, (uint64_t)filesystemtype, (uint64_t)mountflags, (uint64_t)data);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int umount(const char *target) {
    return umount2(target, 0);
}

int umount2(const char *target, int flags) {
    int64_t ret = __lix_syscall2(SYS_UMOUNT, (uint64_t)target, (uint64_t)flags);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}
