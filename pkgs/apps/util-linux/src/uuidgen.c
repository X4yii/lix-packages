#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
    uint8_t u[16];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        read(fd, u, 16);
        close(fd);
    } else {
        for (int i = 0; i < 16; ++i) u[i] = (uint8_t)rand();
    }
    u[6] = (u[6] & 0x0F) | 0x40;
    u[8] = (u[8] & 0x3F) | 0x80;
    printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
           u[0], u[1], u[2], u[3],
           u[4], u[5], u[6], u[7],
           u[8], u[9], u[10], u[11],
           u[12], u[13], u[14], u[15]);
    return 0;
}
