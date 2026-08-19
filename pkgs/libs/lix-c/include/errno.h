#ifndef _ERRNO_H
#define _ERRNO_H

int *__errno_location(void);
int *errno_location(void);
#define errno (*__errno_location())

#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENOEXEC 8
#define EBADF 9
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define ENODEV 19
#define EINVAL 22
#define EMFILE 24
#define ENOSYS 38
#define ERANGE 34
#define EADDRINUSE 98
#define ETIMEDOUT 110
#define ECONNREFUSED 111

#endif
