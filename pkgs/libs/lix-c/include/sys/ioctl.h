#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#include <stdint.h>

#define BLKGETSIZE64 0x80081272
#define BLKSSZGET    0x00001268
#define BLKRRPART    0x0000125f
#define BLKFLSBUF    0x00001261
#define TIOCGWINSZ   0x5413

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

int ioctl(int fd, unsigned long request, ...);

#endif
