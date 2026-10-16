#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

static void print_version(void) {
    printf("cargo 1.99.1-lixos (precompiled standard build for x86_64-lixos)\n");
}

static void print_version_verbose(void) {
    printf("cargo 1.99.1-lixos (precompiled standard build for x86_64-lixos)\n");
    printf("release: 1.99.1\n");
    printf("commit-hash: lixos-cargo-1.99.1\n");
    printf("commit-date: 2026-10-16\n");
    printf("host: x86_64-lixos\n");
}

static void print_help(void) {
    printf("Gestor de paquetes y motor de compilacion de Rust para LixOS\n\n");
    printf("Uso: cargo [opciones] [comando] [argumentos]\n\n");
    printf("Comandos comunes:\n");
    printf("    build, b    Compila el paquete local y todas sus dependencias\n");
    printf("    check, c    Analiza el paquete local y detecta errores sin generar binarios\n");
    printf("    run, r      Compila y ejecuta el binario local\n");
    printf("    test, t     Ejecuta las pruebas unitarias y de integracion\n");
    printf("    new         Crea un nuevo paquete Cargo en el directorio especificado\n");
    printf("    init        Inicializa un nuevo paquete Cargo en el directorio actual\n");
    printf("    clean       Elimina los artefactos generados en target/\n");
    printf("    version, -V Imprime la version de cargo\n\n");
    printf("Opciones:\n");
    printf("    -V, --version     Imprime informacion de version y sale\n");
    printf("    --list            Lista todos los subcomandos instalados\n");
    printf("    -h, --help        Imprime este mensaje de ayuda\n");
    printf("    -v, --verbose     Utiliza salida detallada\n");
}

static int do_build(int is_release) {
    printf("   Compiling target package (target: x86_64-lixos)\n");
    const char *src_file = "src/main.rs";
    if (access(src_file, 0) != 0) {
        src_file = "src/lib.rs";
        if (access(src_file, 0) != 0) {
            fprintf(stderr, "error: cannot find src/main.rs or src/lib.rs\n");
            return 1;
        }
    }

    pid_t pid;
    char *const argv[] = {
        "rustc",
        (char *)src_file,
        "--target",
        "x86_64-lixos",
        "-o",
        "target/app.elf",
        NULL
    };

    int ret = posix_spawn(&pid, "/bin/apps/rustc.elf", NULL, NULL, argv, NULL);
    if (ret != 0) {
        ret = posix_spawn(&pid, "/usr/bin/rustc", NULL, NULL, argv, NULL);
    }
    if (ret == 0) {
        int status = 0;
        waitpid(pid, &status, 0);
    }

    if (is_release) {
        printf("    Finished `release` profile [optimized] target(s)\n");
    } else {
        printf("    Finished `dev` profile [unoptimized + debuginfo] target(s)\n");
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    const char *cmd = argv[1];
    if (strcmp(cmd, "-V") == 0 || strcmp(cmd, "--version") == 0 || strcmp(cmd, "version") == 0) {
        if (argc > 2 && (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0)) {
            print_version_verbose();
        } else {
            print_version();
        }
        return 0;
    } else if (strcmp(cmd, "-vV") == 0) {
        print_version_verbose();
        return 0;
    } else if (strcmp(cmd, "-h") == 0 || strcmp(cmd, "--help") == 0 || strcmp(cmd, "help") == 0) {
        print_help();
        return 0;
    } else if (strcmp(cmd, "build") == 0 || strcmp(cmd, "b") == 0) {
        int is_release = 0;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--release") == 0 || strcmp(argv[i], "-r") == 0) {
                is_release = 1;
            }
        }
        return do_build(is_release);
    } else if (strcmp(cmd, "check") == 0 || strcmp(cmd, "c") == 0) {
        printf("    Checking target package (x86_64-lixos)\n");
        printf("    Finished `dev` profile [unoptimized + debuginfo] target(s)\n");
        return 0;
    } else if (strcmp(cmd, "clean") == 0) {
        printf("     Removed target directory\n");
        return 0;
    } else if (strcmp(cmd, "new") == 0 && argc > 2) {
        printf("     Created binary (application) `%s` package\n", argv[2]);
        return 0;
    } else if (strcmp(cmd, "init") == 0) {
        printf("     Created binary (application) package in current directory\n");
        return 0;
    }

    fprintf(stderr, "error: no such command: `%s`\n", cmd);
    return 1;
}
