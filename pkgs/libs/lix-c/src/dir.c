#include <dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <lix/syscall.h>

extern int64_t __lix_syscall4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4);

struct __DIR {
    int fd;
    char *buf;
    size_t buf_len;
    size_t offset;
    struct dirent entry;
};

DIR *opendir(const char *name) {
    if (!name) {
        errno = ENOENT;
        return NULL;
    }
    int fd = open(name, O_RDONLY | O_DIRECTORY);
    if (fd < 0) return NULL;
    DIR *d = (DIR *)malloc(sizeof(DIR));
    if (!d) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    d->fd = fd;
    d->offset = 0;
    d->buf = (char *)malloc(8192);
    if (!d->buf) {
        free(d);
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    size_t path_len = strlen(name);
    int64_t ret = __lix_syscall4(SYS_LIST_DIR, (uint64_t)name, path_len, (uint64_t)d->buf, 8192);
    if (ret < 0) {
        d->buf_len = 0;
    } else {
        d->buf_len = (size_t)ret;
    }
    return d;
}

DIR *fdopendir(int fd) {
    DIR *d = (DIR *)malloc(sizeof(DIR));
    if (!d) {
        errno = ENOMEM;
        return NULL;
    }
    d->fd = fd;
    d->offset = 0;
    d->buf = NULL;
    d->buf_len = 0;
    return d;
}

struct dirent *readdir(DIR *dirp) {
    if (!dirp || !dirp->buf || dirp->offset >= dirp->buf_len) {
        return NULL;
    }
    size_t start = dirp->offset;
    while (dirp->offset < dirp->buf_len && dirp->buf[dirp->offset] != '\n') {
        dirp->offset++;
    }
    size_t len = dirp->offset - start;
    if (len >= sizeof(dirp->entry.d_name)) {
        len = sizeof(dirp->entry.d_name) - 1;
    }
    memcpy(dirp->entry.d_name, &dirp->buf[start], len);
    dirp->entry.d_name[len] = '\0';
    dirp->entry.d_ino = 1;
    dirp->entry.d_off = (off_t)dirp->offset;
    dirp->entry.d_reclen = sizeof(struct dirent);
    dirp->entry.d_type = DT_UNKNOWN;
    if (dirp->offset < dirp->buf_len && dirp->buf[dirp->offset] == '\n') {
        dirp->offset++;
    }
    return &dirp->entry;
}

int closedir(DIR *dirp) {
    if (!dirp) return -1;
    if (dirp->buf) free(dirp->buf);
    int res = close(dirp->fd);
    free(dirp);
    return res;
}

int dirfd(DIR *dirp) {
    return dirp ? dirp->fd : -1;
}

void rewinddir(DIR *dirp) {
    if (dirp) dirp->offset = 0;
}

long telldir(DIR *dirp) {
    return dirp ? (long)dirp->offset : -1;
}

void seekdir(DIR *dirp, long loc) {
    if (dirp && loc >= 0 && (size_t)loc <= dirp->buf_len) {
        dirp->offset = (size_t)loc;
    }
}
