#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#pragma pack(push, 1)
struct MbrPartition {
    uint8_t bootable;
    uint8_t start_head;
    uint8_t start_sector;
    uint8_t start_cylinder;
    uint8_t system_id;
    uint8_t end_head;
    uint8_t end_sector;
    uint8_t end_cylinder;
    uint32_t start_lba;
    uint32_t total_sectors;
};

struct MbrSector {
    uint8_t bootstrap[446];
    struct MbrPartition partitions[4];
    uint16_t boot_signature;
};

struct GptHeader {
    char signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t disk_guid[16];
    uint64_t partition_entries_lba;
    uint32_t num_partition_entries;
    uint32_t partition_entry_size;
    uint32_t partition_array_crc32;
};

struct GptEntry {
    uint8_t type_guid[16];
    uint8_t unique_guid[16];
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    uint16_t name[36];
};
#pragma pack(pop)

static void dump_device(const char *dev) {
    int fd = open(dev, O_RDONLY);
    if (fd < 0) return;
    uint64_t size_bytes = 0;
    uint32_t ssz = 512;
    if (ioctl(fd, BLKGETSIZE64, &size_bytes) < 0) {
        off_t end = lseek(fd, 0, SEEK_END);
        if (end > 0) size_bytes = (uint64_t)end;
    }
    if (ioctl(fd, BLKSSZGET, &ssz) < 0 || ssz == 0) ssz = 512;
    uint64_t sectors = size_bytes / ssz;

    printf("Disco %s: %.2f GiB, %llu bytes, %llu sectores\n",
           dev, (double)size_bytes / (1024.0 * 1024.0 * 1024.0),
           (unsigned long long)size_bytes, (unsigned long long)sectors);
    printf("Unidades: sectores de 1 * %u = %u bytes\n", ssz, ssz);
    printf("Tamano de sector (logico/fisico): %u bytes / %u bytes\n", ssz, ssz);

    struct GptHeader gpt;
    lseek(fd, ssz, SEEK_SET);
    int is_gpt = 0;
    if (read(fd, &gpt, sizeof(gpt)) == sizeof(gpt) && memcmp(gpt.signature, "EFI PART", 8) == 0) {
        is_gpt = 1;
    }

    if (is_gpt) {
        printf("Tipo de etiqueta de disco: gpt\n");
        printf("Identificador del disco: ");
        for (int i = 0; i < 16; ++i) {
            printf("%02X%s", gpt.disk_guid[i], (i == 3 || i == 5 || i == 7 || i == 9) ? "-" : "");
        }
        printf("\n\n");
        printf("Dispositivo    Inicio        Fin   Sectores  Tamano Tipo\n");
        size_t array_bytes = gpt.num_partition_entries * gpt.partition_entry_size;
        uint8_t *entries = (uint8_t *)malloc(array_bytes);
        lseek(fd, gpt.partition_entries_lba * ssz, SEEK_SET);
        if (read(fd, entries, array_bytes) == (ssize_t)array_bytes) {
            for (uint32_t i = 0; i < gpt.num_partition_entries && i < 128; ++i) {
                struct GptEntry *e = (struct GptEntry *)(entries + (i * gpt.partition_entry_size));
                int non_zero = 0;
                for (int k = 0; k < 16; ++k) {
                    if (e->type_guid[k] != 0) { non_zero = 1; break; }
                }
                if (!non_zero) continue;
                uint64_t count = e->ending_lba - e->starting_lba + 1;
                uint64_t b = count * ssz;
                printf("%s%u   %10llu %10llu %10llu  %6.1fM Sistema de ficheros de Linux\n",
                       dev, i + 1,
                       (unsigned long long)e->starting_lba,
                       (unsigned long long)e->ending_lba,
                       (unsigned long long)count,
                       (double)b / (1024.0 * 1024.0));
            }
        }
        free(entries);
    } else {
        struct MbrSector mbr;
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &mbr, sizeof(mbr)) == sizeof(mbr) && mbr.boot_signature == 0xAA55) {
            printf("Tipo de etiqueta de disco: dos\n\n");
            printf("Dispositivo  Arrancable    Inicio       Fin   Sectores  Tamano Id Tipo\n");
            for (int i = 0; i < 4; ++i) {
                if (mbr.partitions[i].system_id == 0) continue;
                uint32_t start = mbr.partitions[i].start_lba;
                uint32_t count = mbr.partitions[i].total_sectors;
                uint32_t end = start + count - 1;
                uint64_t b = (uint64_t)count * ssz;
                printf("%s%d             %c  %10u %10u %10u  %6.1fM %02x Linux\n",
                       dev, i + 1,
                       (mbr.partitions[i].bootable == 0x80) ? '*' : ' ',
                       start, end, count,
                       (double)b / (1024.0 * 1024.0),
                       mbr.partitions[i].system_id);
            }
        }
    }
    printf("\n");
    close(fd);
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-l") == 0) {
        if (argc > 2) {
            dump_device(argv[2]);
        } else {
            const char *disks[] = {"/dev/sda", "/dev/nvme0n1", "/dev/sdb", NULL};
            for (int i = 0; disks[i]; ++i) {
                if (access(disks[i], 0) == 0) {
                    dump_device(disks[i]);
                }
            }
        }
        return 0;
    }
    if (argc > 1) {
        dump_device(argv[1]);
        return 0;
    }
    printf("fdisk (util-linux 2.40.1 / LixOS)\n");
    printf("Uso: fdisk -l [dispositivo]\n");
    return 0;
}
