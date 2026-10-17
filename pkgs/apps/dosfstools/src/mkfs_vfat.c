#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#pragma pack(push, 1)
struct Fat32Bpb {
    uint8_t  jmp_boot[3];
    char     oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
    uint8_t  boot_code[420];
    uint16_t signature;
};

struct Fat32FsInfo {
    uint32_t lead_sig;
    uint8_t  reserved1[480];
    uint32_t struc_sig;
    uint32_t free_count;
    uint32_t next_free;
    uint8_t  reserved2[12];
    uint32_t trail_sig;
};
#pragma pack(pop)

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("mkfs.fat 4.2.0 (LixOS DOS/FAT32 Suite)\n");
        printf("Uso: mkfs.vfat [-F 32] [-n etiqueta] <dispositivo>\n");
        return 1;
    }
    const char *dev = NULL;
    const char *label = "NO NAME    ";
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            label = argv[++i];
        } else if (argv[i][0] != '-') {
            dev = argv[i];
        }
    }
    if (!dev) {
        fprintf(stderr, "mkfs.vfat: falta el dispositivo objetivo\n");
        return 1;
    }

    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "mkfs.vfat: no se pudo abrir '%s'\n", dev);
        return 1;
    }

    uint64_t total_bytes = 0;
    if (ioctl(fd, BLKGETSIZE64, &total_bytes) < 0) {
        off_t end = lseek(fd, 0, SEEK_END);
        if (end > 0) total_bytes = (uint64_t)end;
    }
    if (total_bytes < 32 * 1024 * 1024) {
        fprintf(stderr, "mkfs.vfat: dispositivo demasiado pequeno para FAT32 (minimo 32 MiB)\n");
        close(fd);
        return 1;
    }

    uint32_t total_sectors = (uint32_t)(total_bytes / 512);
    uint8_t spc = 8;
    uint16_t reserved_sectors = 32;
    uint8_t num_fats = 2;
    uint32_t data_sectors = total_sectors - reserved_sectors;
    uint32_t total_clusters = data_sectors / spc;
    uint32_t fat_size_bytes = (total_clusters + 2) * 4;
    uint32_t fat_size_sectors = (fat_size_bytes + 511) / 512;

    struct Fat32Bpb bpb;
    memset(&bpb, 0, sizeof(bpb));
    bpb.jmp_boot[0] = 0xEB;
    bpb.jmp_boot[1] = 0x58;
    bpb.jmp_boot[2] = 0x90;
    memcpy(bpb.oem_name, "MSWIN4.1", 8);
    bpb.bytes_per_sector = 512;
    bpb.sectors_per_cluster = spc;
    bpb.reserved_sector_count = reserved_sectors;
    bpb.num_fats = num_fats;
    bpb.media = 0xF8;
    bpb.sectors_per_track = 32;
    bpb.num_heads = 64;
    bpb.total_sectors_32 = total_sectors;
    bpb.fat_size_32 = fat_size_sectors;
    bpb.root_cluster = 2;
    bpb.fs_info = 1;
    bpb.backup_boot_sector = 6;
    bpb.drive_number = 0x80;
    bpb.boot_signature = 0x29;

    int urand = open("/dev/urandom", O_RDONLY);
    if (urand >= 0) {
        read(urand, &bpb.volume_id, 4);
        close(urand);
    } else {
        bpb.volume_id = 0x12345678;
    }

    memset(bpb.volume_label, ' ', 11);
    for (size_t i = 0; i < strlen(label) && i < 11; ++i) {
        bpb.volume_label[i] = label[i];
    }
    memcpy(bpb.fs_type, "FAT32   ", 8);
    bpb.signature = 0xAA55;

    lseek(fd, 0, SEEK_SET);
    write(fd, &bpb, sizeof(bpb));
    lseek(fd, 6 * 512, SEEK_SET);
    write(fd, &bpb, sizeof(bpb));

    struct Fat32FsInfo info;
    memset(&info, 0, sizeof(info));
    info.lead_sig = 0x41615252;
    info.struc_sig = 0x61417272;
    info.free_count = total_clusters - 1;
    info.next_free = 3;
    info.trail_sig = 0xAA550000;

    lseek(fd, 1 * 512, SEEK_SET);
    write(fd, &info, sizeof(info));
    lseek(fd, 7 * 512, SEEK_SET);
    write(fd, &info, sizeof(info));

    uint8_t first_fat_sector[512];
    memset(first_fat_sector, 0, sizeof(first_fat_sector));
    uint32_t *fat = (uint32_t *)first_fat_sector;
    fat[0] = 0x0FFFFFF8;
    fat[1] = 0xFFFFFFFF;
    fat[2] = 0x0FFFFFFF;

    uint64_t fat1_offset = (uint64_t)reserved_sectors * 512;
    lseek(fd, fat1_offset, SEEK_SET);
    write(fd, first_fat_sector, 512);

    uint64_t fat2_offset = fat1_offset + ((uint64_t)fat_size_sectors * 512);
    lseek(fd, fat2_offset, SEEK_SET);
    write(fd, first_fat_sector, 512);

    uint8_t zero_cluster[4096];
    memset(zero_cluster, 0, sizeof(zero_cluster));
    uint64_t root_dir_offset = fat2_offset + ((uint64_t)fat_size_sectors * 512);
    lseek(fd, root_dir_offset, SEEK_SET);
    write(fd, zero_cluster, sizeof(zero_cluster));

    ioctl(fd, BLKFLSBUF, 0);
    close(fd);

    printf("mkfs.fat 4.2.0 (17-Oct-2026)\n");
    printf("%s tiene %u cabezas y %u sectores por pista,\n", dev, bpb.num_heads, bpb.sectors_per_track);
    printf("tamano del cluster: %u sectores (%u bytes)\n", spc, spc * 512);
    printf("FAT32 formateado exitosamente en %s (VolID: %08X)\n", dev, bpb.volume_id);
    return 0;
}
