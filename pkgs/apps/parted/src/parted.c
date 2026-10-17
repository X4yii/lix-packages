#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#pragma pack(push, 1)
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
#pragma pack(pop)

static uint32_t crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

static void print_version(void) {
    printf("parted (GNU parted / LixOS Disk Utility) 3.6.0\n");
    printf("Copyright (C) 2026 Free Software Foundation, Inc. / LixOS Project\n");
}

static void print_help(void) {
    printf("Uso: parted [opciones] [dispositivo [comando [opciones...]...]]\n\n");
    printf("Opciones:\n");
    printf("  -h, --help                    Muestra este mensaje de ayuda\n");
    printf("  -l, --list                    Lista las particiones de todos los dispositivos de bloque\n");
    printf("  -v, --version                 Muestra la version\n\n");
    printf("Comandos:\n");
    printf("  print                         Imprime la tabla de particiones del disco\n");
    printf("  mklabel, mktable <tipo>       Crea una nueva tabla de particiones (gpt o msdos)\n");
    printf("  mkpart <nombre> <ini> <fin>   Crea una nueva particion GPT\n");
    printf("  rm <numero>                   Elimina la particion indicada\n");
}

static int query_disk_info(int fd, uint64_t *out_size_bytes, uint32_t *out_ssz) {
    uint64_t total_bytes = 0;
    uint32_t sector_size = 512;
    if (ioctl(fd, BLKGETSIZE64, &total_bytes) < 0) {
        off_t end = lseek(fd, 0, SEEK_END);
        if (end > 0) {
            total_bytes = (uint64_t)end;
        } else {
            return -1;
        }
    }
    if (ioctl(fd, BLKSSZGET, &sector_size) < 0) {
        sector_size = 512;
    }
    if (sector_size == 0) sector_size = 512;
    *out_size_bytes = total_bytes;
    *out_ssz = sector_size;
    return 0;
}

static int do_print(const char *dev_path) {
    int fd = open(dev_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "parted: error al abrir '%s'\n", dev_path);
        return 1;
    }
    uint64_t size_bytes = 0;
    uint32_t ssz = 512;
    if (query_disk_info(fd, &size_bytes, &ssz) < 0) {
        fprintf(stderr, "parted: no se pudo obtener geometria de '%s'\n", dev_path);
        close(fd);
        return 1;
    }

    printf("Modelo: Dispositivo de Bloques LixOS (%s)\n", dev_path);
    printf("Disco %s: %.1f GB (%llu bytes)\n", dev_path, (double)size_bytes / 1000000000.0, (unsigned long long)size_bytes);
    printf("Tamano de sector (logico/fisico): %uB / %uB\n", ssz, ssz);

    struct GptHeader gpt;
    lseek(fd, ssz, SEEK_SET);
    int is_gpt = 0;
    if (read(fd, &gpt, sizeof(gpt)) == sizeof(gpt)) {
        if (memcmp(gpt.signature, "EFI PART", 8) == 0) {
            is_gpt = 1;
        }
    }

    if (is_gpt) {
        printf("Tabla de particiones: gpt\n\n");
        printf("Numero  Inicio        Fin           Tamano        Nombre\n");
        uint64_t entries_offset = gpt.partition_entries_lba * ssz;
        lseek(fd, entries_offset, SEEK_SET);
        for (uint32_t i = 0; i < gpt.num_partition_entries && i < 128; ++i) {
            struct GptEntry entry;
            if (read(fd, &entry, sizeof(entry)) != sizeof(entry)) break;
            int non_zero = 0;
            for (int k = 0; k < 16; ++k) {
                if (entry.type_guid[k] != 0) { non_zero = 1; break; }
            }
            if (!non_zero) continue;
            uint64_t start_b = entry.starting_lba * ssz;
            uint64_t end_b = (entry.ending_lba + 1) * ssz;
            uint64_t part_sz = end_b - start_b;
            char name_ascii[37];
            for (int n = 0; n < 36; ++n) {
                name_ascii[n] = (char)entry.name[n];
            }
            name_ascii[36] = '\0';
            printf("%-7u %-13llu %-13llu %-13llu %s\n",
                   i + 1,
                   (unsigned long long)start_b,
                   (unsigned long long)end_b,
                   (unsigned long long)part_sz,
                   name_ascii);
        }
    } else {
        struct MbrSector mbr;
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &mbr, sizeof(mbr)) == sizeof(mbr) && mbr.boot_signature == 0xAA55) {
            printf("Tabla de particiones: msdos\n\n");
            printf("Numero  Inicio        Fin           Tamano        Tipo  Sistema\n");
            for (int i = 0; i < 4; ++i) {
                if (mbr.partitions[i].system_id == 0) continue;
                uint64_t start_b = (uint64_t)mbr.partitions[i].start_lba * ssz;
                uint64_t part_sz = (uint64_t)mbr.partitions[i].total_sectors * ssz;
                uint64_t end_b = start_b + part_sz;
                printf("%-7d %-13llu %-13llu %-13llu 0x%02x  primaria\n",
                       i + 1,
                       (unsigned long long)start_b,
                       (unsigned long long)end_b,
                       (unsigned long long)part_sz,
                       mbr.partitions[i].system_id);
            }
        } else {
            printf("Tabla de particiones: desconocida / sin formato\n");
        }
    }

    close(fd);
    return 0;
}

static int do_mklabel(const char *dev_path, const char *label_type) {
    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "parted: no se pudo abrir '%s' para escritura\n", dev_path);
        return 1;
    }
    uint64_t size_bytes = 0;
    uint32_t ssz = 512;
    if (query_disk_info(fd, &size_bytes, &ssz) < 0) {
        fprintf(stderr, "parted: no se pudo obtener tamano de '%s'\n", dev_path);
        close(fd);
        return 1;
    }
    uint64_t total_sectors = size_bytes / ssz;
    if (total_sectors < 100) {
        fprintf(stderr, "parted: disco demasiado pequeno\n");
        close(fd);
        return 1;
    }

    if (strcmp(label_type, "gpt") == 0) {
        struct MbrSector pmbr;
        memset(&pmbr, 0, sizeof(pmbr));
        pmbr.boot_signature = 0xAA55;
        pmbr.partitions[0].bootable = 0x00;
        pmbr.partitions[0].start_sector = 0x02;
        pmbr.partitions[0].system_id = 0xEE;
        pmbr.partitions[0].start_lba = 1;
        pmbr.partitions[0].total_sectors = (uint32_t)((total_sectors > 0xFFFFFFFF) ? 0xFFFFFFFF : (total_sectors - 1));
        lseek(fd, 0, SEEK_SET);
        write(fd, &pmbr, sizeof(pmbr));

        struct GptHeader gpt;
        memset(&gpt, 0, sizeof(gpt));
        memcpy(gpt.signature, "EFI PART", 8);
        gpt.revision = 0x00010000;
        gpt.header_size = 92;
        gpt.current_lba = 1;
        gpt.backup_lba = total_sectors - 1;
        gpt.first_usable_lba = 34;
        gpt.last_usable_lba = total_sectors - 34;
        gpt.partition_entries_lba = 2;
        gpt.num_partition_entries = 128;
        gpt.partition_entry_size = sizeof(struct GptEntry);

        int urandom = open("/dev/urandom", O_RDONLY);
        if (urandom >= 0) {
            read(urandom, gpt.disk_guid, 16);
            close(urandom);
        } else {
            for (int i = 0; i < 16; ++i) gpt.disk_guid[i] = (uint8_t)(i + 1);
        }

        size_t array_bytes = gpt.num_partition_entries * gpt.partition_entry_size;
        uint8_t *entries = (uint8_t *)calloc(1, array_bytes);
        gpt.partition_array_crc32 = crc32(entries, array_bytes);
        gpt.header_crc32 = 0;
        gpt.header_crc32 = crc32((const uint8_t *)&gpt, gpt.header_size);

        lseek(fd, ssz, SEEK_SET);
        write(fd, &gpt, sizeof(gpt));
        lseek(fd, 2 * ssz, SEEK_SET);
        write(fd, entries, array_bytes);

        gpt.current_lba = total_sectors - 1;
        gpt.backup_lba = 1;
        gpt.partition_entries_lba = total_sectors - 33;
        gpt.header_crc32 = 0;
        gpt.header_crc32 = crc32((const uint8_t *)&gpt, gpt.header_size);
        lseek(fd, (total_sectors - 33) * ssz, SEEK_SET);
        write(fd, entries, array_bytes);
        lseek(fd, (total_sectors - 1) * ssz, SEEK_SET);
        write(fd, &gpt, sizeof(gpt));

        free(entries);
        ioctl(fd, BLKFLSBUF, 0);
        ioctl(fd, BLKRRPART, 0);
        printf("Tabla de particiones GPT creada exitosamente en %s\n", dev_path);
    } else if (strcmp(label_type, "msdos") == 0) {
        struct MbrSector mbr;
        memset(&mbr, 0, sizeof(mbr));
        mbr.boot_signature = 0xAA55;
        lseek(fd, 0, SEEK_SET);
        write(fd, &mbr, sizeof(mbr));
        ioctl(fd, BLKFLSBUF, 0);
        ioctl(fd, BLKRRPART, 0);
        printf("Tabla de particiones MBR (msdos) creada exitosamente en %s\n", dev_path);
    } else {
        fprintf(stderr, "parted: tipo de etiqueta '%s' no soportado (use gpt o msdos)\n", label_type);
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}

static int do_mkpart(const char *dev_path, const char *part_name, uint64_t start_sector, uint64_t end_sector) {
    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "parted: no se pudo abrir '%s' para escritura\n", dev_path);
        return 1;
    }
    uint64_t size_bytes = 0;
    uint32_t ssz = 512;
    if (query_disk_info(fd, &size_bytes, &ssz) < 0) {
        fprintf(stderr, "parted: no se pudo obtener tamano de '%s'\n", dev_path);
        close(fd);
        return 1;
    }
    uint64_t total_sectors = size_bytes / ssz;

    struct GptHeader gpt;
    lseek(fd, ssz, SEEK_SET);
    if (read(fd, &gpt, sizeof(gpt)) != sizeof(gpt) || memcmp(gpt.signature, "EFI PART", 8) != 0) {
        fprintf(stderr, "parted: '%s' no tiene una tabla GPT valida (ejecute mklabel gpt primero)\n", dev_path);
        close(fd);
        return 1;
    }

    size_t array_bytes = gpt.num_partition_entries * gpt.partition_entry_size;
    uint8_t *entries = (uint8_t *)malloc(array_bytes);
    lseek(fd, gpt.partition_entries_lba * ssz, SEEK_SET);
    if (read(fd, entries, array_bytes) != (ssize_t)array_bytes) {
        fprintf(stderr, "parted: error al leer entradas de particion\n");
        free(entries);
        close(fd);
        return 1;
    }

    int free_slot = -1;
    struct GptEntry *slot = NULL;
    for (uint32_t i = 0; i < gpt.num_partition_entries; ++i) {
        struct GptEntry *e = (struct GptEntry *)(entries + (i * gpt.partition_entry_size));
        int is_empty = 1;
        for (int k = 0; k < 16; ++k) {
            if (e->type_guid[k] != 0) { is_empty = 0; break; }
        }
        if (is_empty) {
            free_slot = (int)i;
            slot = e;
            break;
        }
    }

    if (free_slot < 0 || !slot) {
        fprintf(stderr, "parted: no quedan ranuras libres de particion\n");
        free(entries);
        close(fd);
        return 1;
    }

    static const uint8_t LINUX_FS_GUID[16] = {
        0xAF, 0x3D, 0xC6, 0x0F, 0x83, 0x84, 0x72, 0x47,
        0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4
    };
    memcpy(slot->type_guid, LINUX_FS_GUID, 16);
    int urandom = open("/dev/urandom", O_RDONLY);
    if (urandom >= 0) {
        read(urandom, slot->unique_guid, 16);
        close(urandom);
    } else {
        for (int i = 0; i < 16; ++i) slot->unique_guid[i] = (uint8_t)(i + free_slot + 1);
    }
    slot->starting_lba = start_sector;
    slot->ending_lba = end_sector;
    slot->attributes = 0;
    memset(slot->name, 0, sizeof(slot->name));
    for (size_t i = 0; i < strlen(part_name) && i < 35; ++i) {
        slot->name[i] = (uint16_t)part_name[i];
    }

    gpt.partition_array_crc32 = crc32(entries, array_bytes);
    gpt.header_crc32 = 0;
    gpt.header_crc32 = crc32((const uint8_t *)&gpt, gpt.header_size);

    lseek(fd, ssz, SEEK_SET);
    write(fd, &gpt, sizeof(gpt));
    lseek(fd, gpt.partition_entries_lba * ssz, SEEK_SET);
    write(fd, entries, array_bytes);

    gpt.current_lba = total_sectors - 1;
    gpt.backup_lba = 1;
    gpt.partition_entries_lba = total_sectors - 33;
    gpt.header_crc32 = 0;
    gpt.header_crc32 = crc32((const uint8_t *)&gpt, gpt.header_size);
    lseek(fd, (total_sectors - 33) * ssz, SEEK_SET);
    write(fd, entries, array_bytes);
    lseek(fd, (total_sectors - 1) * ssz, SEEK_SET);
    write(fd, &gpt, sizeof(gpt));

    free(entries);
    ioctl(fd, BLKFLSBUF, 0);
    ioctl(fd, BLKRRPART, 0);
    printf("Particion %d '%s' creada (%llu a %llu) en %s\n",
           free_slot + 1, part_name, (unsigned long long)start_sector, (unsigned long long)end_sector, dev_path);
    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();
        return 0;
    }
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        print_version();
        return 0;
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }
    if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0) {
        const char *candidates[] = {"/dev/sda", "/dev/nvme0n1", "/dev/sdb", NULL};
        for (int i = 0; candidates[i] != NULL; ++i) {
            if (access(candidates[i], 0) == 0) {
                do_print(candidates[i]);
                printf("\n");
            }
        }
        return 0;
    }

    const char *dev = argv[1];
    if (argc == 2 || strcmp(argv[2], "print") == 0) {
        return do_print(dev);
    }
    if (strcmp(argv[2], "mklabel") == 0 || strcmp(argv[2], "mktable") == 0) {
        const char *type = (argc > 3) ? argv[3] : "gpt";
        return do_mklabel(dev, type);
    }
    if (strcmp(argv[2], "mkpart") == 0) {
        const char *name = (argc > 3) ? argv[3] : "primary";
        uint64_t start_sec = 2048;
        uint64_t end_sec = 204800;
        if (argc > 4) start_sec = strtoull(argv[4], NULL, 10);
        if (argc > 5) end_sec = strtoull(argv[5], NULL, 10);
        return do_mkpart(dev, name, start_sec, end_sec);
    }

    fprintf(stderr, "parted: comando '%s' no reconocido\n", argv[2]);
    return 1;
}
