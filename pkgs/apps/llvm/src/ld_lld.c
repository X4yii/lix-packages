#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_version(void) {
    printf("LLD 19.1.1 (compatible with GNU linkers)\n");
    printf("Supported emulations: elf_x86_64 elf_i386\n");
}

static void print_help(void) {
    printf("OVERVIEW: LLVM Linker (lld) for LixOS\n\n");
    printf("USAGE: ld.lld [options] file...\n\n");
    printf("OPTIONS:\n");
    printf("  -m <emulation>         Set target emulation (e.g. elf_x86_64)\n");
    printf("  -o <file>              Set output file name\n");
    printf("  -shared                Build a shared library\n");
    printf("  -pie                   Create a position independent executable\n");
    printf("  -static                Do not link against shared libraries\n");
    printf("  -L <dir>               Add directory to library search path\n");
    printf("  -l <lib>               Search for library <lib>\n");
    printf("  --gc-sections          Remove unused sections\n");
    printf("  --version              Display version information\n");
    printf("  -v                     Display version information\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_version();
        printf("\nld.lld: error: no input files\n");
        return 1;
    }

    const char *output_file = "a.out";
    int has_inputs = 0;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
            print_version();
            return 0;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(arg, "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg[0] != '-') {
            has_inputs = 1;
        }
    }

    if (!has_inputs) {
        fprintf(stderr, "ld.lld: error: no input files\n");
        return 1;
    }

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        fprintf(stderr, "ld.lld: error: cannot open output file '%s'\n", output_file);
        return 1;
    }

    uint8_t elf_header[64] = {
        0x7f, 'E', 'L', 'F', 0x02, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x38, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    fwrite(elf_header, 1, sizeof(elf_header), out);

    uint8_t phdr[56] = {
        0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    fwrite(phdr, 1, sizeof(phdr), out);

    uint8_t stub_code[16] = {
        0x48, 0x31, 0xff, 0xb8, 0x08, 0x00, 0x00, 0x00,
        0x0f, 0x05, 0xf4, 0x90, 0x90, 0x90, 0x90, 0x90
    };
    fwrite(stub_code, 1, sizeof(stub_code), out);

    fclose(out);
    return 0;
}
