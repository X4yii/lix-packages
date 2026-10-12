#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <pwd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <time.h>

ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    long cur = lseek(fd, 0, SEEK_CUR);
    if (cur < 0) return -1;
    if (lseek(fd, offset, SEEK_SET) < 0) return -1;
    long res = read(fd, buf, count);
    lseek(fd, cur, SEEK_SET);
    return (ssize_t)res;
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    long cur = lseek(fd, 0, SEEK_CUR);
    if (cur < 0) return -1;
    if (lseek(fd, offset, SEEK_SET) < 0) return -1;
    long res = write(fd, buf, count);
    lseek(fd, cur, SEEK_SET);
    return (ssize_t)res;
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    if (!iov || iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }
    ssize_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        if (iov[i].iov_len > 0 && iov[i].iov_base) {
            long res = read(fd, iov[i].iov_base, iov[i].iov_len);
            if (res < 0) {
                if (total > 0) return total;
                return -1;
            }
            total += res;
            if ((size_t)res < iov[i].iov_len) break;
        }
    }
    return total;
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    if (!iov || iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }
    ssize_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        if (iov[i].iov_len > 0 && iov[i].iov_base) {
            long res = write(fd, iov[i].iov_base, iov[i].iov_len);
            if (res < 0) {
                if (total > 0) return total;
                return -1;
            }
            total += res;
            if ((size_t)res < iov[i].iov_len) break;
        }
    }
    return total;
}

int ftruncate(int fd, off_t length) {
    (void)fd;
    (void)length;
    return 0;
}

int fsync(int fd) {
    (void)fd;
    return 0;
}

int openat(int dirfd, const char *pathname, int flags, ...) {
    (void)dirfd;
    return open(pathname, flags, 0666);
}

int unlinkat(int dirfd, const char *pathname, int flags) {
    (void)dirfd;
    (void)flags;
    return unlink(pathname);
}

int rename(const char *oldpath, const char *newpath) {
    (void)oldpath;
    (void)newpath;
    return 0;
}

int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath) {
    (void)olddirfd;
    (void)oldpath;
    (void)newdirfd;
    (void)newpath;
    return 0;
}

int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags) {
    (void)olddirfd;
    (void)oldpath;
    (void)newdirfd;
    (void)newpath;
    (void)flags;
    return 0;
}

int symlink(const char *target, const char *linkpath) {
    (void)target;
    (void)linkpath;
    return 0;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    (void)pathname;
    (void)buf;
    (void)bufsiz;
    errno = EINVAL;
    return -1;
}

char *realpath(const char *path, char *resolved_path) {
    if (!path) {
        errno = EINVAL;
        return NULL;
    }
    size_t len = strlen(path);
    if (!resolved_path) {
        resolved_path = (char *)malloc(len + 1);
        if (!resolved_path) {
            errno = ENOMEM;
            return NULL;
        }
    }
    memcpy(resolved_path, path, len + 1);
    return resolved_path;
}

int chmod(const char *pathname, mode_t mode) {
    (void)pathname;
    (void)mode;
    return 0;
}

int fchmod(int fd, mode_t mode) {
    (void)fd;
    (void)mode;
    return 0;
}

int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
    (void)dirfd;
    (void)pathname;
    (void)mode;
    (void)flags;
    return 0;
}

int chown(const char *pathname, uid_t owner, gid_t group) {
    (void)pathname;
    (void)owner;
    (void)group;
    return 0;
}

int fchown(int fd, uid_t owner, gid_t group) {
    (void)fd;
    (void)owner;
    (void)group;
    return 0;
}

int lchown(const char *pathname, uid_t owner, gid_t group) {
    (void)pathname;
    (void)owner;
    (void)group;
    return 0;
}

int chroot(const char *path) {
    (void)path;
    return 0;
}

int mkfifo(const char *pathname, mode_t mode) {
    (void)pathname;
    (void)mode;
    return 0;
}

int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags) {
    (void)dirfd;
    (void)pathname;
    (void)times;
    (void)flags;
    return 0;
}

int futimens(int fd, const struct timespec times[2]) {
    (void)fd;
    (void)times;
    return 0;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    (void)fds;
    (void)nfds;
    (void)timeout;
    return 1;
}

int ioctl(int fd, unsigned long request, ...) {
    (void)fd;
    (void)request;
    return 0;
}

int fcntl(int fd, int cmd, ...) {
    (void)fd;
    (void)cmd;
    return 0;
}

long sysconf(int name) {
    (void)name;
    return 4096;
}

static struct passwd dummy_pwd = {
    .pw_name = "root",
    .pw_passwd = "x",
    .pw_uid = 0,
    .pw_gid = 0,
    .pw_gecos = "root",
    .pw_dir = "/root",
    .pw_shell = "/bin/sh"
};

struct passwd *getpwuid(uid_t uid) {
    (void)uid;
    return &dummy_pwd;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result) {
    (void)uid;
    (void)buf;
    (void)buflen;
    if (pwd) {
        *pwd = dummy_pwd;
        if (result) *result = pwd;
    } else {
        if (result) *result = NULL;
    }
    return 0;
}

int setgroups(size_t size, const gid_t *list) {
    (void)size;
    (void)list;
    return 0;
}

int setpgid(pid_t pid, pid_t pgid) {
    (void)pid;
    (void)pgid;
    return 0;
}

pid_t setsid(void) {
    return 1;
}

int killpg(pid_t pgrp, int sig) {
    (void)pgrp;
    (void)sig;
    return 0;
}

sighandler_t signal(int signum, sighandler_t handler) {
    (void)signum;
    (void)handler;
    return SIG_DFL;
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    (void)node;
    (void)service;
    (void)hints;
    if (res) *res = NULL;
    return EAI_NONAME;
}

void freeaddrinfo(struct addrinfo *res) {
    (void)res;
}

const char *gai_strerror(int errcode) {
    (void)errcode;
    return "Unknown error";
}

void *__tls_get_addr(void *arg) {
    (void)arg;
    return NULL;
}
