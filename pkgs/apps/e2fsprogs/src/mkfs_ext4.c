#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define EXT4_SUPER_MAGIC 0xEF53
#define EXT4_BLOCK_SIZE 4096
#define EXT4_BLOCKS_PER_GROUP 32768
#define EXT4_INODES_PER_GROUP 8192
#define EXT4_INODE_SIZE 256

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
    char     s_last_mounted[64];
    uint32_t s_algorithm_usage_bitmap;
    uint8_t  s_prealloc_blocks;
    uint8_t  s_prealloc_dir_blocks;
    uint16_t s_reserved_gdt_blocks;
    uint8_t  s_journal_uuid[16];
    uint32_t s_journal_inum;
    uint32_t s_journal_dev;
    uint32_t s_last_orphan;
    uint32_t s_hash_seed[4];
    uint8_t  s_def_hash_version;
    uint8_t  s_jnl_backup_type;
    uint16_t s_desc_size;
    uint32_t s_default_mount_opts;
    uint32_t s_first_meta_bg;
    uint32_t s_mkfs_time;
    uint32_t s_jnl_blocks[17];
    uint32_t s_blocks_count_hi;
    uint32_t s_r_blocks_count_hi;
    uint32_t s_free_blocks_count_hi;
    uint16_t s_min_extra_isize;
    uint16_t s_want_extra_isize;
    uint32_t s_flags;
    uint16_t s_raid_stride;
    uint16_t s_mmp_interval;
    uint64_t s_mmp_block;
    uint32_t s_raid_stripe_width;
    uint8_t  s_log_groups_per_flex;
    uint8_t  s_checksum_type;
    uint16_t s_reserved_pad;
    uint64_t s_kbytes_written;
    uint8_t  s_padding[824];
};

struct Ext4GroupDesc {
    uint32_t bg_block_bitmap_lo;
    uint32_t bg_inode_bitmap_lo;
    uint32_t bg_inode_table_lo;
    uint16_t bg_free_blocks_count_lo;
    uint16_t bg_free_inodes_count_lo;
    uint16_t bg_used_dirs_count_lo;
    uint16_t bg_flags;
    uint32_t bg_exclude_bitmap_lo;
    uint16_t bg_block_bitmap_csum_lo;
    uint16_t bg_inode_bitmap_csum_lo;
    uint16_t bg_itable_unused_lo;
    uint16_t bg_checksum;
    uint32_t bg_block_bitmap_hi;
    uint32_t bg_inode_bitmap_hi;
    uint32_t bg_inode_table_hi;
    uint16_t bg_free_blocks_count_hi;
    uint16_t bg_free_inodes_count_hi;
    uint16_t bg_used_dirs_count_hi;
    uint16_t bg_itable_unused_hi;
    uint32_t bg_exclude_bitmap_hi;
    uint16_t bg_block_bitmap_csum_hi;
    uint16_t bg_inode_bitmap_csum_hi;
    uint32_t bg_reserved;
};

struct Ext4ExtentHeader {
    uint16_t eh_magic;
    uint16_t eh_entries;
    uint16_t eh_max;
    uint16_t eh_depth;
    uint32_t eh_generation;
};

struct Ext4Extent {
    uint32_t ee_block;
    uint16_t ee_len;
    uint16_t ee_start_hi;
    uint32_t ee_start_lo;
};

struct Ext4Inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size_lo;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks_lo;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl_lo;
    uint32_t i_size_high;
    uint32_t i_obso_faddr;
    uint8_t  i_osd2[24];
    uint16_t i_extra_isize;
    uint16_t i_checksum_hi;
    uint32_t i_ctime_extra;
    uint32_t i_mtime_extra;
    uint32_t i_atime_extra;
    uint32_t i_crtime;
    uint32_t i_crtime_extra;
    uint32_t i_version_hi;
    uint8_t  i_padding[100];
};

struct Ext4DirEntry2 {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[256];
};
#pragma pack(pop)

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("mke2fs / mkfs.ext4 1.47.0 (LixOS Storage Suite)\n");
        printf("Uso: mkfs.ext4 [-F] [-L etiqueta] <dispositivo>\n");
        return 1;
    }
    const char *dev = NULL;
    const char *label = "";
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            label = argv[++i];
        } else if (argv[i][0] != '-') {
            dev = argv[i];
        }
    }
    if (!dev) {
        fprintf(stderr, "mkfs.ext4: falta el dispositivo objetivo\n");
        return 1;
    }

    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "mkfs.ext4: error al abrir '%s'\n", dev);
        return 1;
    }

    uint64_t total_bytes = 0;
    if (ioctl(fd, BLKGETSIZE64, &total_bytes) < 0) {
        off_t end = lseek(fd, 0, SEEK_END);
        if (end > 0) total_bytes = (uint64_t)end;
    }
    if (total_bytes < 16 * 1024 * 1024) {
        fprintf(stderr, "mkfs.ext4: dispositivo demasiado pequeno (minimo 16 MiB)\n");
        close(fd);
        return 1;
    }

    uint64_t total_blocks = total_bytes / EXT4_BLOCK_SIZE;
    uint32_t num_groups = (uint32_t)((total_blocks + EXT4_BLOCKS_PER_GROUP - 1) / EXT4_BLOCKS_PER_GROUP);
    uint32_t total_inodes = num_groups * EXT4_INODES_PER_GROUP;

    printf("mke2fs 1.47.0 (17-Oct-2026)\n");
    printf("Creando sistema de ficheros con %llu bloques de 4k y %u inodos\n",
           (unsigned long long)total_blocks, total_inodes);
    printf("UUID del sistema de ficheros: ");

    struct Ext4SuperBlock sb;
    memset(&sb, 0, sizeof(sb));
    sb.s_inodes_count = total_inodes;
    sb.s_blocks_count_lo = (uint32_t)(total_blocks & 0xFFFFFFFF);
    sb.s_blocks_count_hi = (uint32_t)(total_blocks >> 32);
    sb.s_r_blocks_count_lo = (uint32_t)(total_blocks / 20);
    sb.s_free_blocks_count_lo = (uint32_t)((total_blocks - (num_groups * (1 + 1 + 1 + 512 + 1))) & 0xFFFFFFFF);
    sb.s_free_inodes_count = total_inodes - 11;
    sb.s_first_data_block = 0;
    sb.s_log_block_size = 2;
    sb.s_log_cluster_size = 2;
    sb.s_blocks_per_group = EXT4_BLOCKS_PER_GROUP;
    sb.s_clusters_per_group = EXT4_BLOCKS_PER_GROUP;
    sb.s_inodes_per_group = EXT4_INODES_PER_GROUP;
    sb.s_magic = EXT4_SUPER_MAGIC;
    sb.s_state = 1;
    sb.s_errors = 1;
    sb.s_rev_level = 1;
    sb.s_first_ino = 11;
    sb.s_inode_size = EXT4_INODE_SIZE;
    sb.s_feature_compat = 0x240;
    sb.s_feature_incompat = 0x242;
    sb.s_feature_ro_compat = 0x7B;
    sb.s_desc_size = sizeof(struct Ext4GroupDesc);

    int urand = open("/dev/urandom", O_RDONLY);
    if (urand >= 0) {
        read(urand, sb.s_uuid, 16);
        close(urand);
    } else {
        for (int i = 0; i < 16; ++i) sb.s_uuid[i] = (uint8_t)(i + 0x42);
    }
    for (int i = 0; i < 16; ++i) {
        printf("%02x%s", sb.s_uuid[i], (i == 3 || i == 5 || i == 7 || i == 9) ? "-" : "");
    }
    printf("\n");
    strncpy(sb.s_volume_name, label, 15);

    lseek(fd, 1024, SEEK_SET);
    write(fd, &sb, sizeof(sb));

    size_t gdt_size = num_groups * sizeof(struct Ext4GroupDesc);
    struct Ext4GroupDesc *gdt = (struct Ext4GroupDesc *)calloc(num_groups, sizeof(struct Ext4GroupDesc));
    uint32_t current_blk = 1 + (uint32_t)((gdt_size + EXT4_BLOCK_SIZE - 1) / EXT4_BLOCK_SIZE);

    for (uint32_t g = 0; g < num_groups; ++g) {
        uint32_t base_blk = g * EXT4_BLOCKS_PER_GROUP;
        if (base_blk == 0) base_blk = current_blk;
        gdt[g].bg_block_bitmap_lo = base_blk;
        gdt[g].bg_inode_bitmap_lo = base_blk + 1;
        gdt[g].bg_inode_table_lo = base_blk + 2;
        gdt[g].bg_free_blocks_count_lo = EXT4_BLOCKS_PER_GROUP - (512 + 2);
        gdt[g].bg_free_inodes_count_lo = EXT4_INODES_PER_GROUP;
        gdt[g].bg_used_dirs_count_lo = (g == 0) ? 2 : 0;
        gdt[g].bg_itable_unused_lo = (g == 0) ? (EXT4_INODES_PER_GROUP - 11) : EXT4_INODES_PER_GROUP;
    }

    lseek(fd, EXT4_BLOCK_SIZE, SEEK_SET);
    write(fd, gdt, gdt_size);

    uint8_t blk_bmp[EXT4_BLOCK_SIZE];
    memset(blk_bmp, 0, sizeof(blk_bmp));
    for (int i = 0; i < 515; ++i) {
        blk_bmp[i / 8] |= (1 << (i % 8));
    }
    lseek(fd, (uint64_t)gdt[0].bg_block_bitmap_lo * EXT4_BLOCK_SIZE, SEEK_SET);
    write(fd, blk_bmp, sizeof(blk_bmp));

    uint8_t ino_bmp[EXT4_BLOCK_SIZE];
    memset(ino_bmp, 0, sizeof(ino_bmp));
    for (int i = 0; i < 11; ++i) {
        ino_bmp[i / 8] |= (1 << (i % 8));
    }
    lseek(fd, (uint64_t)gdt[0].bg_inode_bitmap_lo * EXT4_BLOCK_SIZE, SEEK_SET);
    write(fd, ino_bmp, sizeof(ino_bmp));

    uint32_t root_dir_blk = gdt[0].bg_inode_table_lo + 512;
    uint8_t itbl[EXT4_BLOCK_SIZE];
    memset(itbl, 0, sizeof(itbl));

    struct Ext4Inode *root_ino = (struct Ext4Inode *)(itbl + (1 * EXT4_INODE_SIZE));
    root_ino->i_mode = 0040755;
    root_ino->i_links_count = 3;
    root_ino->i_size_lo = EXT4_BLOCK_SIZE;
    root_ino->i_blocks_lo = 8;
    root_ino->i_flags = 0x80000;
    struct Ext4ExtentHeader *eh = (struct Ext4ExtentHeader *)root_ino->i_block;
    eh->eh_magic = 0xF30A;
    eh->eh_entries = 1;
    eh->eh_max = 4;
    eh->eh_depth = 0;
    struct Ext4Extent *ext = (struct Ext4Extent *)(root_ino->i_block + 3);
    ext->ee_block = 0;
    ext->ee_len = 1;
    ext->ee_start_lo = root_dir_blk;

    lseek(fd, (uint64_t)gdt[0].bg_inode_table_lo * EXT4_BLOCK_SIZE, SEEK_SET);
    write(fd, itbl, sizeof(itbl));

    uint8_t dir_blk[EXT4_BLOCK_SIZE];
    memset(dir_blk, 0, sizeof(dir_blk));
    struct Ext4DirEntry2 *d1 = (struct Ext4DirEntry2 *)dir_blk;
    d1->inode = 2;
    d1->name_len = 1;
    d1->file_type = 2;
    d1->name[0] = '.';
    d1->rec_len = 12;

    struct Ext4DirEntry2 *d2 = (struct Ext4DirEntry2 *)(dir_blk + 12);
    d2->inode = 2;
    d2->name_len = 2;
    d2->file_type = 2;
    d2->name[0] = '.';
    d2->name[1] = '.';
    d2->rec_len = 12;

    struct Ext4DirEntry2 *d3 = (struct Ext4DirEntry2 *)(dir_blk + 24);
    d3->inode = 11;
    d3->name_len = 10;
    d3->file_type = 2;
    memcpy(d3->name, "lost+found", 10);
    d3->rec_len = EXT4_BLOCK_SIZE - 24;

    lseek(fd, (uint64_t)root_dir_blk * EXT4_BLOCK_SIZE, SEEK_SET);
    write(fd, dir_blk, sizeof(dir_blk));

    free(gdt);
    ioctl(fd, BLKFLSBUF, 0);
    close(fd);

    printf("Escribiendo las tablas de inodos: hecho\n");
    printf("Escribiendo la informacion del superbloque y la contabilidad: hecho\n\n");
    return 0;
}
