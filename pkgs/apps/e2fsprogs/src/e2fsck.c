#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define EXT4_SUPER_MAGIC 0xEF53

#pragma pack(push, 1)
struct Ext4SuperBlock {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count_lo;
    uint32_t s_r_blocks_count_lo;
    uint32_t s_free_blocks_count_lo;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_cluster_size;
    uint32_t s_blocks_per_group;
    uint32_t s_clusters_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t  s_uuid[16];
    char     s_volume_name[16];
};
#pragma pack(pop)

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("e2fsck 1.47.0 (LixOS Ext4 Check Suite)\n");
        printf("Uso: e2fsck [-p|-n|-y|-f] <dispositivo>\n");
        return 1;
    }
    const char *dev = NULL;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            dev = argv[i];
            break;
        }
    }
    if (!dev) {
        fprintf(stderr, "e2fsck: falta el dispositivo a comprobar\n");
        return 1;
    }

    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "e2fsck: no se pudo abrir '%s'\n", dev);
        return 8;
    }

    struct Ext4SuperBlock sb;
    lseek(fd, 1024, SEEK_SET);
    if (read(fd, &sb, sizeof(sb)) != sizeof(sb)) {
        fprintf(stderr, "e2fsck: error al leer superbloque en '%s'\n", dev);
        close(fd);
        return 4;
    }

    if (sb.s_magic != EXT4_SUPER_MAGIC) {
        fprintf(stderr, "e2fsck: numero magico incorrecto (0x%04x) en superbloque de '%s'\n", sb.s_magic, dev);
        close(fd);
        return 4;
    }

    printf("%s: limpio, %u/%u ficheros, %u/%u bloques\n",
           dev,
           sb.s_inodes_count - sb.s_free_inodes_count,
           sb.s_inodes_count,
           sb.s_blocks_count_lo - sb.s_free_blocks_count_lo,
           sb.s_blocks_count_lo);

    close(fd);
    return 0;
}
