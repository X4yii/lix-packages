#include <stdlib.h>
#include <stddef.h>

int atoi(const char *nptr) {
    int res = 0;
    int sign = 1;
    while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r') nptr++;
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        res = res * 10 + (*nptr - '0');
        nptr++;
    }
    return res * sign;
}

long atol(const char *nptr) {
    long res = 0;
    long sign = 1;
    while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r') nptr++;
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        res = res * 10 + (*nptr - '0');
        nptr++;
    }
    return res * sign;
}

int abs(int j) {
    return j < 0 ? -j : j;
}

long labs(long j) {
    return j < 0 ? -j : j;
}

unsigned long long strtoull(const char *nptr, char **endptr, int base) {
    unsigned long long res = 0;
    while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r') nptr++;
    if (base == 0) {
        if (nptr[0] == '0' && (nptr[1] == 'x' || nptr[1] == 'X')) {
            base = 16;
            nptr += 2;
        } else if (nptr[0] == '0') {
            base = 8;
            nptr += 1;
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (nptr[0] == '0' && (nptr[1] == 'x' || nptr[1] == 'X')) {
            nptr += 2;
        }
    }
    while (*nptr) {
        int val;
        if (*nptr >= '0' && *nptr <= '9') val = *nptr - '0';
        else if (*nptr >= 'a' && *nptr <= 'z') val = *nptr - 'a' + 10;
        else if (*nptr >= 'A' && *nptr <= 'Z') val = *nptr - 'A' + 10;
        else break;
        if (val >= base) break;
        res = res * base + val;
        nptr++;
    }
    if (endptr) *endptr = (char *)nptr;
    return res;
}

unsigned long strtoul(const char *nptr, char **endptr, int base) {
    return (unsigned long)strtoull(nptr, endptr, base);
}

long long strtoll(const char *nptr, char **endptr, int base) {
    while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r') nptr++;
    int sign = 1;
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }
    long long res = (long long)strtoull(nptr, endptr, base);
    return res * sign;
}

long strtol(const char *nptr, char **endptr, int base) {
    return (long)strtoll(nptr, endptr, base);
}

static unsigned int __next_rand = 1;
int rand(void) {
    __next_rand = __next_rand * 1103515245 + 12345;
    return (int)((__next_rand / 65536) % 32768);
}

void srand(unsigned int seed) {
    __next_rand = seed;
}
