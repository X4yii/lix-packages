#ifndef ZLIB_H
#define ZLIB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZLIB_VERSION "1.3.1"
#define ZLIB_VERNUM 0x1310

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)

typedef void * (*alloc_func)(void *opaque, unsigned int items, unsigned int size);
typedef void   (*free_func)(void *opaque, void *address);

typedef struct z_stream_s {
    const uint8_t *next_in;
    unsigned int     avail_in;
    unsigned long    total_in;

    uint8_t       *next_out;
    unsigned int     avail_out;
    unsigned long    total_out;

    const char      *msg;
    struct internal_state *state;

    alloc_func zalloc;
    free_func  zfree;
    void      *opaque;

    int     data_type;
    unsigned long   adler;
    unsigned long   reserved;
} z_stream;

typedef z_stream *z_streamp;

const char *zlibVersion(void);
uint32_t z_adler32(uint32_t adler, const uint8_t *buf, size_t len);
uint32_t z_crc32(uint32_t crc, const uint8_t *buf, size_t len);
int compress(uint8_t *dest, size_t *destLen, const uint8_t *source, size_t sourceLen);
int uncompress(uint8_t *dest, size_t *destLen, const uint8_t *source, size_t sourceLen);

#ifdef __cplusplus
}
#endif

#endif
