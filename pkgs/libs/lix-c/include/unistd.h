#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>
#include <stdint.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

long read(int fd, void *buf, size_t count);
long write(int fd, const void *buf, size_t count);
int close(int fd);
long lseek(int fd, long offset, int whence);
void _exit(int status);

unsigned int sleep(unsigned int seconds);
int usleep(unsigned long usec);
int access(const char *pathname, int mode);
int unlink(const char *pathname);
int pipe(int pipefd[2]);
uint64_t getpid(void);
uint32_t getuid(void);
uint32_t getgid(void);

#endif
