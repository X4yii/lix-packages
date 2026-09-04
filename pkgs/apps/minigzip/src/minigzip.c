#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>

static int file_compress(const char *in_file, const char *out_file) {
    FILE *in = fopen(in_file, "rb");
    if (!in) {
        fprintf(stderr, "minigzip: no se pudo abrir origen: %s\n", in_file);
        return 1;
    }

    fseek(in, 0, SEEK_END);
    long in_sz = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (in_sz < 0) {
        fclose(in);
        return 1;
    }

    size_t in_size = (size_t)in_sz;
    uint8_t *in_buf = (uint8_t *)malloc(in_size > 0 ? in_size : 1);
    if (!in_buf) {
        fclose(in);
        return 2;
    }

    if (in_size > 0 && fread(in_buf, 1, in_size, in) != in_size) {
        free(in_buf);
        fclose(in);
        return 3;
    }
    fclose(in);

    size_t out_max = in_size + 64 + (in_size / 10);
    uint8_t *out_buf = (uint8_t *)malloc(out_max);
    if (!out_buf) {
        free(in_buf);
        return 4;
    }

    size_t out_size = out_max;
    int ret = compress(out_buf, &out_size, in_buf, in_size);
    if (ret != Z_OK) {
        fprintf(stderr, "minigzip: fallo de compresion con codigo %d\n", ret);
        free(in_buf);
        free(out_buf);
        return 5;
    }

    FILE *out = fopen(out_file, "wb");
    if (!out) {
        fprintf(stderr, "minigzip: no se pudo abrir destino: %s\n", out_file);
        free(in_buf);
        free(out_buf);
        return 6;
    }

    if (fwrite(out_buf, 1, out_size, out) != out_size) {
        free(in_buf);
        free(out_buf);
        fclose(out);
        return 7;
    }

    fclose(out);
    free(in_buf);
    free(out_buf);

    printf("minigzip: comprimido '%s' -> '%s' (%zu bytes -> %zu bytes, ratio: %.1f%%)\n",
           in_file, out_file, in_size, out_size,
           in_size > 0 ? ((double)out_size / (double)in_size) * 100.0 : 100.0);
    return 0;
}

static int file_decompress(const char *in_file, const char *out_file) {
    FILE *in = fopen(in_file, "rb");
    if (!in) {
        fprintf(stderr, "minigzip: no se pudo abrir archivo comprimido: %s\n", in_file);
        return 1;
    }

    fseek(in, 0, SEEK_END);
    long in_sz = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (in_sz < 0) {
        fclose(in);
        return 1;
    }

    size_t in_size = (size_t)in_sz;
    uint8_t *in_buf = (uint8_t *)malloc(in_size > 0 ? in_size : 1);
    if (!in_buf) {
        fclose(in);
        return 2;
    }

    if (in_size > 0 && fread(in_buf, 1, in_size, in) != in_size) {
        free(in_buf);
        fclose(in);
        return 3;
    }
    fclose(in);

    size_t out_max = in_size * 20 + 4096;
    uint8_t *out_buf = (uint8_t *)malloc(out_max);
    if (!out_buf) {
        free(in_buf);
        return 4;
    }

    size_t out_size = out_max;
    int ret = uncompress(out_buf, &out_size, in_buf, in_size);
    if (ret != Z_OK) {
        fprintf(stderr, "minigzip: fallo de descompresion con codigo %d\n", ret);
        free(in_buf);
        free(out_buf);
        return 5;
    }

    FILE *out = fopen(out_file, "wb");
    if (!out) {
        fprintf(stderr, "minigzip: no se pudo abrir destino descomprimido: %s\n", out_file);
        free(in_buf);
        free(out_buf);
        return 6;
    }

    if (fwrite(out_buf, 1, out_size, out) != out_size) {
        free(in_buf);
        free(out_buf);
        fclose(out);
        return 7;
    }

    fclose(out);
    free(in_buf);
    free(out_buf);

    printf("minigzip: descomprimido '%s' -> '%s' (%zu bytes -> %zu bytes)\n",
           in_file, out_file, in_size, out_size);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("minigzip para LixOS (version %s)\n", zlibVersion());
        printf("Uso: minigzip [-d] <archivo_origen> [archivo_destino]\n");
        return 0;
    }

    int decompress_mode = 0;
    int arg_idx = 1;

    if (strcmp(argv[arg_idx], "-d") == 0) {
        decompress_mode = 1;
        arg_idx++;
        if (arg_idx >= argc) {
            fprintf(stderr, "minigzip: falta especificar archivo para descomprimir\n");
            return 1;
        }
    }

    const char *in_name = argv[arg_idx];
    char out_name_buf[512];
    const char *out_name = NULL;

    if (arg_idx + 1 < argc) {
        out_name = argv[arg_idx + 1];
    } else {
        if (decompress_mode) {
            size_t len = strlen(in_name);
            if (len > 3 && strcmp(in_name + len - 3, ".gz") == 0) {
                memcpy(out_name_buf, in_name, len - 3);
                out_name_buf[len - 3] = '\0';
            } else {
                snprintf(out_name_buf, sizeof(out_name_buf), "%s.out", in_name);
            }
        } else {
            snprintf(out_name_buf, sizeof(out_name_buf), "%s.gz", in_name);
        }
        out_name = out_name_buf;
    }

    if (decompress_mode) {
        return file_decompress(in_name, out_name);
    } else {
        return file_compress(in_name, out_name);
    }
}
