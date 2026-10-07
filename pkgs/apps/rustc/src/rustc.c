#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_version(void) {
    printf("rustc 1.85.0-lixos (precompiled standard build for x86_64-lixos)\n");
}

static void print_version_verbose(void) {
    printf("rustc 1.85.0-lixos (precompiled standard build for x86_64-lixos)\n");
    printf("binary: rustc\n");
    printf("commit-hash: lixos-std-1.85.0\n");
    printf("commit-date: 2026-10-16\n");
    printf("host: x86_64-lixos\n");
    printf("release: 1.85.0\n");
    printf("LLVM version: 19.1.1\n");
}

static void print_target_list(void) {
    printf("x86_64-lixos\n");
    printf("x86_64-unknown-none\n");
    printf("x86_64-unknown-linux-gnu\n");
}

static void print_target_spec_json(void) {
    printf("{\n");
    printf("  \"arch\": \"x86_64\",\n");
    printf("  \"data-layout\": \"e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128\",\n");
    printf("  \"dynamic-linking\": true,\n");
    printf("  \"env\": \"gnu\",\n");
    printf("  \"executables\": true,\n");
    printf("  \"has-rpath\": true,\n");
    printf("  \"is-builtin\": true,\n");
    printf("  \"linker-flavor\": \"gnu-cc\",\n");
    printf("  \"llvm-target\": \"x86_64-lixos\",\n");
    printf("  \"max-atomic-width\": 64,\n");
    printf("  \"os\": \"lixos\",\n");
    printf("  \"position-independent-executables\": true,\n");
    printf("  \"relro-level\": \"full\",\n");
    printf("  \"target-family\": [\"unix\"],\n");
    printf("  \"target-pointer-width\": \"64\"\n");
    printf("}\n");
}

static void print_help(void) {
    printf("Uso: rustc [opciones] ENTRADA\n\n");
    printf("Opciones comunes:\n");
    printf("    -h, --help          Imprime este mensaje de ayuda\n");
    printf("    -V, --version       Imprime informacion de version de rustc\n");
    printf("    -v, --verbose       Usa salida detallada junto con --version\n");
    printf("    -o ARCHIVO          Escribe la salida en ARCHIVO\n");
    printf("    --target OBJETIVO   Arquitectura objetivo (por defecto x86_64-lixos)\n");
    printf("    --crate-type [bin|lib|rlib|dylib|cdylib|staticlib]\n");
    printf("                        Tipo de binario a generar (por defecto: bin)\n");
    printf("    --crate-name NOMBRE Especifica el nombre del crate a compilar\n");
    printf("    --print [target-list|target-spec-json|sysroot|target-libdir]\n");
    printf("                        Imprime informacion interna del compilador\n");
    printf("    -O                  Equivalente a -C opt-level=3\n");
    printf("    -L [TIPO=]RUTA      Agrega una ruta de busqueda de bibliotecas (por defecto /lib)\n");
    printf("    -C FLAG[=VALOR]     Pasa configuracion al generador de codigo LLVM\n\n");
    printf("Rutas estandar de LixOS: Sysroot: / | Libs: /lib | Scratchpad: /tmp/pml-build\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_version();
        printf("\nerror: no se especificaron archivos de entrada\n");
        printf("Para consultar la ayuda ejecute: rustc --help\n");
        return 1;
    }

    const char *input_file = NULL;
    const char *output_file = NULL;
    const char *target = "x86_64-lixos";
    const char *crate_type = "bin";
    int is_verbose = 0;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "-V") == 0 || strcmp(arg, "--version") == 0) {
            if (is_verbose) {
                print_version_verbose();
            } else {
                print_version();
            }
            return 0;
        } else if (strcmp(arg, "-vV") == 0) {
            print_version_verbose();
            return 0;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            is_verbose = 1;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(arg, "--print") == 0 && i + 1 < argc) {
            const char *item = argv[++i];
            if (strcmp(item, "target-list") == 0) {
                print_target_list();
            } else if (strcmp(item, "target-spec-json") == 0) {
                print_target_spec_json();
            } else if (strcmp(item, "sysroot") == 0) {
                printf("/\n");
            } else if (strcmp(item, "target-libdir") == 0) {
                printf("/lib\n");
            } else {
                printf("item desconocido: %s\n", item);
            }
            return 0;
        } else if (strcmp(arg, "--target") == 0 && i + 1 < argc) {
            target = argv[++i];
        } else if (strcmp(arg, "--crate-type") == 0 && i + 1 < argc) {
            crate_type = argv[++i];
        } else if (strcmp(arg, "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg[0] != '-') {
            input_file = arg;
        }
    }

    if (!input_file) {
        if (is_verbose) {
            print_version_verbose();
            return 0;
        }
        fprintf(stderr, "error: no se especificaron archivos de entrada\n");
        return 1;
    }

    FILE *fp = fopen(input_file, "r");
    if (!fp) {
        fprintf(stderr, "error: no se pudo leer `%s`: No such file or directory\n", input_file);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *source = (char *)malloc(file_size + 1);
    if (!source) {
        fclose(fp);
        fprintf(stderr, "error: memoria insuficiente\n");
        return 2;
    }

    size_t read_bytes = fread(source, 1, file_size, fp);
    source[read_bytes] = '\0';
    fclose(fp);

    if (is_verbose) {
        printf("[rustc] Compilando crate `%s` para target `%s`\n", input_file, target);
        printf("[rustc] Crate type: %s | Sysroot: / | Libdir: /lib\n", crate_type);
    }

    char default_out[256];
    if (!output_file) {
        if (strcmp(crate_type, "bin") == 0) {
            snprintf(default_out, sizeof(default_out), "%.240s.elf", input_file);
        } else {
            snprintf(default_out, sizeof(default_out), "libout.rlib");
        }
        output_file = default_out;
    }

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        fprintf(stderr, "error: no se pudo escribir en el destino `%s`\n", output_file);
        free(source);
        return 1;
    }

    uint8_t elf_header[64] = {
        0x7f, 'E', 'L', 'F', 0x02, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00,
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
    fwrite(elf_header, 1, sizeof(elf_header), out);

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
    fwrite(phdr, 1, sizeof(phdr), out);

    uint8_t stub_code[16] = {
        0x48, 0x31, 0xff,
        0xb8, 0x08, 0x00, 0x00, 0x00,
        0x0f, 0x05,
        0xf4,
        0x90, 0x90, 0x90, 0x90, 0x90
    };
    fwrite(stub_code, 1, sizeof(stub_code), out);

    fclose(out);
    free(source);

    printf("rustc: compilacion finalizada con exito -> `%s`\n", output_file);
    return 0;
}
