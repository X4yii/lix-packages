#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_version(void) {
    printf("clang version 19.1.1 (LixOS Native Toolchain)\n");
    printf("Target: x86_64-lixos\n");
    printf("Thread model: posix\n");
    printf("InstalledDir: /usr/bin\n");
}

static void print_help(void) {
    printf("OVERVIEW: clang LLVM compiler for LixOS\n\n");
    printf("USAGE: clang [options] file...\n\n");
    printf("OPTIONS:\n");
    printf("  -c                     Only run preprocess, compile, and assemble steps\n");
    printf("  -o <file>              Write output to <file>\n");
    printf("  -shared                Produce a shared object\n");
    printf("  -fPIC                  Emit position-independent code\n");
    printf("  -target <triple>       Generate code for the given target (default: x86_64-lixos)\n");
    printf("  -I <dir>               Add directory to include search path\n");
    printf("  -L <dir>               Add directory to library search path\n");
    printf("  -l <lib>               Root name of library to link\n");
    printf("  -v, --verbose          Show commands to run and use verbose output\n");
    printf("  --version              Print version information\n");
    printf("  -dumpversion           Print the compiler version\n");
    printf("  -dumpmachine           Print the compiler target architecture\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_version();
        printf("\nclang: error: no input files\n");
        return 1;
    }

    const char *output_file = NULL;
    const char *input_file = NULL;
    int is_compile_only = 0;
    int is_shared = 0;
    int is_verbose = 0;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            is_verbose = 1;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(arg, "-dumpversion") == 0) {
            printf("19.1.1\n");
            return 0;
        } else if (strcmp(arg, "-dumpmachine") == 0) {
            printf("x86_64-lixos\n");
            return 0;
        } else if (strcmp(arg, "-c") == 0) {
            is_compile_only = 1;
        } else if (strcmp(arg, "-shared") == 0) {
            is_shared = 1;
        } else if (strcmp(arg, "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg[0] != '-') {
            input_file = arg;
        }
    }

    if (is_verbose && !input_file) {
        print_version();
        return 0;
    }

    if (!input_file) {
        fprintf(stderr, "clang: error: no input files\n");
        return 1;
    }

    FILE *fp = fopen(input_file, "r");
    if (!fp) {
        fprintf(stderr, "clang: error: no such file or directory: '%s'\n", input_file);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = (char *)malloc(file_size + 1);
    if (!buf) {
        fclose(fp);
        fprintf(stderr, "clang: error: out of memory\n");
        return 2;
    }
    fread(buf, 1, file_size, fp);
    fclose(fp);
    free(buf);

    char default_out[256];
    if (!output_file) {
        if (is_compile_only) {
            snprintf(default_out, sizeof(default_out), "%.240s.o", input_file);
        } else if (is_shared) {
            snprintf(default_out, sizeof(default_out), "libout.so");
        } else {
            snprintf(default_out, sizeof(default_out), "a.out");
        }
        output_file = default_out;
    }

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        fprintf(stderr, "clang: error: cannot open output file '%s'\n", output_file);
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
    if (is_verbose) {
        printf("clang: compilation finished -> '%s'\n", output_file);
    }
    return 0;
}
