#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

struct FILE {
    int fd;
};

static FILE __stdin_f = {0};
static FILE __stdout_f = {1};
static FILE __stderr_f = {2};

FILE *stdin = &__stdin_f;
FILE *stdout = &__stdout_f;
FILE *stderr = &__stderr_f;

int putchar(int c) {
    char ch = (char)c;
    if (write(1, &ch, 1) == 1) return c;
    return EOF;
}

int fputc(int c, FILE *stream) {
    if (!stream) return EOF;
    char ch = (char)c;
    if (write(stream->fd, &ch, 1) == 1) return c;
    return EOF;
}

int fputs(const char *s, FILE *stream) {
    if (!stream || !s) return EOF;
    size_t len = strlen(s);
    if (write(stream->fd, s, len) >= 0) return 0;
    return EOF;
}

int puts(const char *s) {
    if (!s) return EOF;
    size_t len = strlen(s);
    write(1, s, len);
    char nl = '\n';
    write(1, &nl, 1);
    return 0;
}

FILE *fopen(const char *pathname, const char *mode) {
    int flags = 0;
    if (strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0) {
        flags = O_RDONLY;
    } else if (strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0 || strcmp(mode, "ab") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (strcmp(mode, "r+") == 0 || strcmp(mode, "rb+") == 0) {
        flags = O_RDWR;
    } else if (strcmp(mode, "w+") == 0 || strcmp(mode, "wb+") == 0) {
        flags = O_RDWR | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a+") == 0 || strcmp(mode, "ab+") == 0) {
        flags = O_RDWR | O_CREAT | O_APPEND;
    } else {
        flags = O_RDONLY;
    }
    int fd = open(pathname, flags, 0666);
    if (fd < 0) return NULL;
    FILE *f = (FILE *)malloc(sizeof(FILE));
    if (!f) {
        close(fd);
        return NULL;
    }
    f->fd = fd;
    return f;
}

int fclose(FILE *stream) {
    if (!stream) return EOF;
    int res = close(stream->fd);
    if (stream != stdin && stream != stdout && stream != stderr) {
        free(stream);
    }
    return res < 0 ? EOF : 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!stream || size == 0 || nmemb == 0) return 0;
    size_t total = size * nmemb;
    long bytes = read(stream->fd, ptr, total);
    if (bytes <= 0) return 0;
    return (size_t)bytes / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!stream || size == 0 || nmemb == 0) return 0;
    size_t total = size * nmemb;
    long bytes = write(stream->fd, ptr, total);
    if (bytes <= 0) return 0;
    return (size_t)bytes / size;
}

int fseek(FILE *stream, long offset, int whence) {
    if (!stream) return -1;
    long res = lseek(stream->fd, offset, whence);
    return res < 0 ? -1 : 0;
}

long ftell(FILE *stream) {
    if (!stream) return -1;
    return lseek(stream->fd, 0, SEEK_CUR);
}

int fflush(FILE *stream) {
    (void)stream;
    return 0;
}

static void num_to_str(uint64_t val, int base, char *buf, int *pos, size_t max) {
    char tmp[64];
    int tpos = 0;
    const char digits[] = "0123456789abcdef";
    if (val == 0) {
        tmp[tpos++] = '0';
    } else {
        while (val > 0) {
            tmp[tpos++] = digits[val % base];
            val /= base;
        }
    }
    while (tpos > 0 && (size_t)(*pos + 1) < max) {
        buf[(*pos)++] = tmp[--tpos];
    }
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    if (!str || size == 0) return 0;
    int pos = 0;
    while (*format && (size_t)(pos + 1) < size) {
        if (*format == '%') {
            format++;
            if (*format == 's') {
                const char *s = va_arg(ap, const char *);
                if (!s) s = "(null)";
                while (*s && (size_t)(pos + 1) < size) {
                    str[pos++] = *s++;
                }
            } else if (*format == 'd' || *format == 'i') {
                int64_t v = (int64_t)va_arg(ap, int);
                if (v < 0) {
                    if ((size_t)(pos + 1) < size) str[pos++] = '-';
                    v = -v;
                }
                num_to_str((uint64_t)v, 10, str, &pos, size);
            } else if (*format == 'u') {
                uint64_t v = (uint64_t)va_arg(ap, unsigned int);
                num_to_str(v, 10, str, &pos, size);
            } else if (*format == 'x') {
                uint64_t v = (uint64_t)va_arg(ap, unsigned int);
                num_to_str(v, 16, str, &pos, size);
            } else if (*format == 'p') {
                uint64_t v = (uint64_t)(uintptr_t)va_arg(ap, void *);
                if ((size_t)(pos + 2) < size) {
                    str[pos++] = '0';
                    str[pos++] = 'x';
                }
                num_to_str(v, 16, str, &pos, size);
            } else if (*format == 'c') {
                char c = (char)va_arg(ap, int);
                if ((size_t)(pos + 1) < size) str[pos++] = c;
            } else if (*format == '%') {
                if ((size_t)(pos + 1) < size) str[pos++] = '%';
            }
        } else {
            str[pos++] = *format;
        }
        format++;
    }
    str[pos] = '\0';
    return pos;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int res = vsnprintf(str, size, format, ap);
    va_end(ap);
    return res;
}

int sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int res = vsnprintf(str, 65536, format, ap);
    va_end(ap);
    return res;
}

int fprintf(FILE *stream, const char *format, ...) {
    if (!stream) return -1;
    char buf[4096];
    va_list ap;
    va_start(ap, format);
    int res = vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    if (res > 0) {
        write(stream->fd, buf, (size_t)res);
    }
    return res;
}

int printf(const char *format, ...) {
    char buf[4096];
    va_list ap;
    va_start(ap, format);
    int res = vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    if (res > 0) {
        write(1, buf, (size_t)res);
    }
    return res;
}
