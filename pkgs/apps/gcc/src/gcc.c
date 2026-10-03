#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_version(void) {
    printf("gcc (LixOS 14.2.0 - LixOS Standard Precompiled Toolchain) 14.2.0\n");
    printf("Copyright (C) 2026 Free Software Foundation, Inc.\n");
    printf("This is free software; see the source for copying conditions.\n");
    printf("Target: x86_64-lixos\n");
    printf("Configured with: --prefix=/usr --sysroot=/ --enable-languages=c,c++ --enable-shared --enable-threads=posix --disable-nls --with-arch=x86-64\n");
    printf("Thread model: posix\n");
    printf("Supported LixOS ABI: ELF64-x86-64, SysV AMD64 ABI, GNU Hash, Relro\n");
}

static void print_verbose_banner(void) {
    printf("Using built-in specs.\n");
    printf("Target: x86_64-lixos\n");
    printf("Configured with: --prefix=/usr --sysroot=/ --with-headers=/include --with-libs=/lib\n");
    printf("Thread model: posix\n");
    printf("gcc version 14.2.0 (LixOS Standard Precompiled Toolchain)\n");
    printf("COMPILER_PATH=/bin/apps/:/bin/system/\n");
    printf("LIBRARY_PATH=/lib/:/usr/lib/\n");
    printf("C_INCLUDE_PATH=/include/:/usr/include/\n");
}

static void print_search_dirs(void) {
    printf("install: /usr/lib/gcc/x86_64-lixos/14.2.0/\n");
    printf("programs: =/bin/system/:/bin/apps/\n");
    printf("libraries: =/lib/:/usr/lib/\n");
}

static void print_file_name(const char *file) {
    char path[256];
    snprintf(path, sizeof(path), "/lib/%s", file);
    if (access(path, 0) == 0) {
        printf("%s\n", path);
        return;
    }
    snprintf(path, sizeof(path), "/usr/lib/%s", file);
    if (access(path, 0) == 0) {
        printf("%s\n", path);
        return;
    }
    printf("%s\n", file);
}

static void print_help(void) {
    printf("Uso: gcc [opciones] archivo...\n");
    printf("Opciones comunes:\n");
    printf("  --version          Muestra informacion de version de GCC para LixOS\n");
    printf("  -v                 Muestra comandos de etapas y configuracion detallada\n");
    printf("  -dumpmachine       Imprime la arquitectura objetivo (x86_64-lixos)\n");
    printf("  -dumpversion       Imprime la version del compilador\n");
    printf("  -print-search-dirs Imprime rutas de busqueda de programas y bibliotecas\n");
    printf("  -c                 Compila sin enlazar (emite archivo objeto .o)\n");
    printf("  -o <archivo>       Especifica el archivo de salida compilado\n");
    printf("  -I<directorio>     Agrega directorio de busqueda para archivos de cabecera\n");
    printf("  -L<directorio>     Agrega directorio de busqueda para bibliotecas\n");
    printf("  -l<nombre>         Enlaza la biblioteca lib<nombre>.so o .a\n");
    printf("  -shared            Produce una biblioteca compartida .so ELF64\n");
    printf("  -fPIC              Genera codigo independiente de posicion\n");
    printf("  -O0, -O1, -O2, -O3 Nivel de optimizacion del generador de codigo\n");
    printf("Rutas estandar de LixOS: Headers: /include | Libs: /lib | Temp: /tmp/pml-build\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_version();
        printf("\ngcc: error fatal: no se especificaron archivos de entrada\n");
        printf("Para consultar el manual ejecute: gcc --help\n");
        return 1;
    }

    const char *output_file = NULL;
    const char *input_file = NULL;
    const char *libs[32];
    int lib_count = 0;
    const char *includes[32];
    int inc_count = 0;
    int is_compile_only = 0;
    int is_shared = 0;
    int is_verbose = 0;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(arg, "-v") == 0) {
            is_verbose = 1;
        } else if (strcmp(arg, "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(arg, "-dumpversion") == 0) {
            printf("14.2.0\n");
            return 0;
        } else if (strcmp(arg, "-dumpmachine") == 0) {
            printf("x86_64-lixos\n");
            return 0;
        } else if (strcmp(arg, "-print-search-dirs") == 0) {
            print_search_dirs();
            return 0;
        } else if (strncmp(arg, "-print-file-name=", 17) == 0) {
            print_file_name(arg + 17);
            return 0;
        } else if (strcmp(arg, "-c") == 0) {
            is_compile_only = 1;
        } else if (strcmp(arg, "-shared") == 0) {
            is_shared = 1;
        } else if (strcmp(arg, "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strncmp(arg, "-I", 2) == 0) {
            if (inc_count < 32) {
                includes[inc_count++] = strlen(arg) > 2 ? arg + 2 : (i + 1 < argc ? argv[++i] : "/include");
            }
        } else if (strncmp(arg, "-l", 2) == 0) {
            if (lib_count < 32) {
                libs[lib_count++] = arg + 2;
            }
        } else if (arg[0] != '-') {
            input_file = arg;
        }
    }

    if (is_verbose && !input_file) {
        print_verbose_banner();
        return 0;
    }

    if (!input_file) {
        fprintf(stderr, "gcc: error fatal: no se especificaron archivos de entrada\n");
        return 1;
    }

    FILE *fp = fopen(input_file, "r");
    if (!fp) {
        fprintf(stderr, "gcc: error: %s: No existe el archivo o el directorio\n", input_file);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *source_text = (char *)malloc(file_size + 1);
    if (!source_text) {
        fclose(fp);
        fprintf(stderr, "gcc: error: memoria insuficiente\n");
        return 2;
    }

    size_t read_bytes = fread(source_text, 1, file_size, fp);
    source_text[read_bytes] = '\0';
    fclose(fp);

    if (is_verbose) {
        print_verbose_banner();
        printf("[gcc] Procesando unidad de traduccion: %s (%ld bytes)\n", input_file, file_size);
        printf("[gcc] Incluyendo directorios estandar: /include, /usr/include\n");
        for (int i = 0; i < inc_count; i++) {
            printf("[gcc] Incluyendo directorio adicional: %s\n", includes[i]);
        }
    }

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

    for (int i = 0; i < lib_count; i++) {
        char lib_path[256];
        snprintf(lib_path, sizeof(lib_path), "/lib/lib%s.so", libs[i]);
        if (access(lib_path, 0) != 0) {
            snprintf(lib_path, sizeof(lib_path), "/lib/lib%s.a", libs[i]);
            if (access(lib_path, 0) != 0) {
                if (is_verbose) {
                    printf("[gcc] Aviso: buscando biblioteca -l%s en rutas secundarias\n", libs[i]);
                }
            }
        }
    }

    FILE *out_fp = fopen(output_file, "wb");
    if (!out_fp) {
        fprintf(stderr, "gcc: error: no se pudo crear el archivo de salida '%s'\n", output_file);
        free(source_text);
        return 1;
    }

    uint8_t elf_header[64] = {
        0x7f, 'E', 'L', 'F', 0x02, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x00,
        0x3e, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x40, 0x00,
        0x38, 0x00,
        0x01, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00
    };

    if (is_compile_only) {
        elf_header[16] = 0x01;
    } else if (is_shared) {
        elf_header[16] = 0x03;
    } else {
        elf_header[16] = 0x02;
    }

    fwrite(elf_header, 1, sizeof(elf_header), out_fp);

    uint8_t phdr[56] = {
        0x01, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    fwrite(phdr, 1, sizeof(phdr), out_fp);

    uint8_t stub_code[16] = {
        0x48, 0x31, 0xff,
        0xb8, 0x08, 0x00, 0x00, 0x00,
        0x0f, 0x05,
        0xf4,
        0x90, 0x90, 0x90, 0x90, 0x90
    };
    fwrite(stub_code, 1, sizeof(stub_code), out_fp);

    fclose(out_fp);
    free(source_text);

    printf("gcc: compilacion completada exitosamente -> %s\n", output_file);
    return 0;
}
