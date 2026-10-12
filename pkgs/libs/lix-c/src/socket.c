#include <sys/socket.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <lix/syscall.h>

extern int64_t __lix_syscall0(uint64_t n);
extern int64_t __lix_syscall1(uint64_t n, uint64_t a1);
extern int64_t __lix_syscall2(uint64_t n, uint64_t a1, uint64_t a2);
extern int64_t __lix_syscall3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3);
extern int64_t __lix_syscall4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4);
extern int64_t __lix_syscall5(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);

int socket(int domain, int type, int protocol) {
    (void)domain;
    (void)protocol;
    int64_t ret;
    if (type == SOCK_STREAM) {
        ret = __lix_syscall0(SYS_SOCKET_TCP);
    } else {
        ret = __lix_syscall1(SYS_SOCKET_OPEN, (uint64_t)type);
    }
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int64_t ret = __lix_syscall3(SYS_SOCKET_BIND, (uint64_t)sockfd, (uint64_t)addr, (uint64_t)addrlen);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int64_t ret = __lix_syscall3(SYS_SOCKET_CONNECT, (uint64_t)sockfd, (uint64_t)addr, (uint64_t)addrlen);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int listen(int sockfd, int backlog) {
    int64_t ret = __lix_syscall2(SYS_SOCKET_LISTEN, (uint64_t)sockfd, (uint64_t)backlog);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int64_t ret = __lix_syscall3(SYS_SOCKET_ACCEPT, (uint64_t)sockfd, (uint64_t)addr, (uint64_t)addrlen);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (int)ret;
}

long send(int sockfd, const void *buf, size_t len, int flags) {
    int64_t ret = __lix_syscall4(SYS_SOCKET_SEND, (uint64_t)sockfd, (uint64_t)buf, (uint64_t)len, (uint64_t)flags);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (long)ret;
}

long recv(int sockfd, void *buf, size_t len, int flags) {
    int64_t ret = __lix_syscall4(SYS_SOCKET_RECV, (uint64_t)sockfd, (uint64_t)buf, (uint64_t)len, (uint64_t)flags);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (long)ret;
}

long sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
    (void)flags;
    int64_t ret = __lix_syscall4(SYS_SOCKET_SENDTO, (uint64_t)sockfd, (uint64_t)buf, (uint64_t)len, (uint64_t)dest_addr);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (long)ret;
}

long recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    (void)flags;
    int64_t ret = __lix_syscall4(SYS_SOCKET_RECVFROM, (uint64_t)sockfd, (uint64_t)buf, (uint64_t)len, (uint64_t)src_addr);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (long)ret;
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    (void)sockfd;
    (void)level;
    (void)optname;
    if (optval && optlen && *optlen >= sizeof(int)) {
        *(int *)optval = 0;
        *optlen = sizeof(int);
    }
    return 0;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    (void)sockfd;
    (void)level;
    (void)optname;
    (void)optval;
    (void)optlen;
    return 0;
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    (void)sockfd;
    if (addr && addrlen && *addrlen >= sizeof(struct sockaddr)) {
        memset(addr, 0, sizeof(struct sockaddr));
        addr->sa_family = AF_INET;
    }
    return 0;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    (void)sockfd;
    if (addr && addrlen && *addrlen >= sizeof(struct sockaddr)) {
        memset(addr, 0, sizeof(struct sockaddr));
        addr->sa_family = AF_INET;
    }
    return 0;
}

int shutdown(int sockfd, int how) {
    (void)sockfd;
    (void)how;
    return 0;
}

int socketpair(int domain, int type, int protocol, int sv[2]) {
    (void)domain;
    (void)type;
    (void)protocol;
    return pipe(sv);
}

int gethostname(char *name, size_t len) {
    if (!name || len == 0) {
        errno = EINVAL;
        return -1;
    }
    const char host[] = "lixos";
    size_t host_len = sizeof(host);
    if (host_len > len) host_len = len;
    memcpy(name, host, host_len);
    name[len - 1] = '\0';
    return 0;
}
