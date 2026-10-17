#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

static void probe_partition(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    uint8_t sb[1024 + 256];
    lseek(fd, 0, SEEK_SET);
    if (read(fd, sb, sizeof(sb)) >= (ssize_t)sizeof(sb)) {
        uint16_t ext_magic = *(uint16_t *)(sb + 1024 + 0x38);
        if (ext_magic == 0xEF53) {
            uint8_t *uuid = sb + 1024 + 0x68;
            char uuid_str[37];
            snprintf(uuid_str, sizeof(uuid_str),
                     "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                     uuid[0], uuid[1], uuid[2], uuid[3],
                     uuid[4], uuid[5], uuid[6], uuid[7],
                     uuid[8], uuid[9], uuid[10], uuid[11],
                     uuid[12], uuid[13], uuid[14], uuid[15]);
            printf("%s: UUID=\"%s\" BLOCK_SIZE=\"4096\" TYPE=\"ext4\"\n", path, uuid_str);
            close(fd);
            return;
        }

        if (memcmp(sb + 82, "FAT32   ", 8) == 0 || memcmp(sb + 54, "FAT16   ", 8) == 0 || memcmp(sb + 54, "FAT12   ", 8) == 0) {
            uint32_t vol_id = *(uint32_t *)(sb + 67);
            printf("%s: UUID=\"%04X-%04X\" BLOCK_SIZE=\"512\" TYPE=\"vfat\"\n",
                   path, (vol_id >> 16) & 0xFFFF, vol_id & 0xFFFF);
            close(fd);
            return;
        }
    }
    close(fd);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            probe_partition(argv[i]);
        }
        return 0;
    }
    const char *candidates[] = {
        "/dev/sda1", "/dev/sda2", "/dev/sda3", "/dev/sda4",
        "/dev/nvme0n1p1", "/dev/nvme0n1p2", "/dev/nvme0n1p3", "/dev/nvme0n1p4",
        "/dev/sdb1", "/dev/sdb2", NULL
    };
    for (int i = 0; candidates[i]; ++i) {
        if (access(candidates[i], 0) == 0) {
            probe_partition(candidates[i]);
        }
    }
    return 0;
}
