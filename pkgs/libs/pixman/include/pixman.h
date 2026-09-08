#ifndef PIXMAN_H
#define PIXMAN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PIXMAN_VERSION_MAJOR 0
#define PIXMAN_VERSION_MINOR 42
#define PIXMAN_VERSION_MICRO 2

#define PIXMAN_VERSION_STRING "0.42.2"

typedef enum {
    PIXMAN_a8r8g8b8 = 0x20028888,
    PIXMAN_x8r8g8b8 = 0x20020888,
    PIXMAN_a8b8g8r8 = 0x20038888,
    PIXMAN_x8b8g8r8 = 0x20030888,
    PIXMAN_r8g8b8a8 = 0x20048888,
    PIXMAN_r8g8b8x8 = 0x20040888,
    PIXMAN_r5g6b5   = 0x10020565,
    PIXMAN_a8       = 0x08000800,
    PIXMAN_a1       = 0x01000100,
    PIXMAN_solid    = 0x00000000
} pixman_format_code_t;

typedef enum {
    PIXMAN_OP_CLEAR        = 0,
    PIXMAN_OP_SRC          = 1,
    PIXMAN_OP_DST          = 2,
    PIXMAN_OP_OVER         = 3,
    PIXMAN_OP_OVER_REVERSE = 4,
    PIXMAN_OP_IN           = 5,
    PIXMAN_OP_IN_REVERSE   = 6,
    PIXMAN_OP_OUT          = 7,
    PIXMAN_OP_OUT_REVERSE  = 8,
    PIXMAN_OP_ATOP         = 9,
    PIXMAN_OP_ATOP_REVERSE = 10,
    PIXMAN_OP_XOR          = 11,
    PIXMAN_OP_ADD          = 12,
    PIXMAN_OP_SATURATE     = 13
} pixman_op_t;

typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t alpha;
} pixman_color_t;

typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} pixman_rectangle16_t;

typedef struct {
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
} pixman_box32_t;

typedef struct {
    pixman_box32_t extents;
} pixman_region32_t;

typedef struct pixman_image pixman_image_t;

struct pixman_image {
    int ref_count;
    pixman_format_code_t format;
    int width;
    int height;
    int stride;
    uint32_t *bits;
    int owns_data;
    pixman_color_t solid_color;
};

int pixman_version(void);
const char *pixman_version_string(void);

pixman_image_t *pixman_image_create_bits(
    pixman_format_code_t format,
    int width,
    int height,
    uint32_t *bits,
    int rowstride_bytes
);

pixman_image_t *pixman_image_create_solid_fill(const pixman_color_t *color);

pixman_image_t *pixman_image_ref(pixman_image_t *image);
int pixman_image_unref(pixman_image_t *image);

void pixman_image_composite(
    pixman_op_t op,
    pixman_image_t *src,
    pixman_image_t *mask,
    pixman_image_t *dest,
    int16_t src_x,
    int16_t src_y,
    int16_t mask_x,
    int16_t mask_y,
    int16_t dest_x,
    int16_t dest_y,
    uint16_t width,
    uint16_t height
);

void pixman_image_composite32(
    pixman_op_t op,
    pixman_image_t *src,
    pixman_image_t *mask,
    pixman_image_t *dest,
    int32_t src_x,
    int32_t src_y,
    int32_t mask_x,
    int32_t mask_y,
    int32_t dest_x,
    int32_t dest_y,
    int32_t width,
    int32_t height
);

int pixman_image_fill_rectangles(
    pixman_op_t op,
    pixman_image_t *image,
    const pixman_color_t *color,
    int n_rects,
    const pixman_rectangle16_t *rects
);

uint32_t *pixman_image_get_data(pixman_image_t *image);
int pixman_image_get_width(pixman_image_t *image);
int pixman_image_get_height(pixman_image_t *image);
int pixman_image_get_stride(pixman_image_t *image);
pixman_format_code_t pixman_image_get_format(pixman_image_t *image);

#ifdef __cplusplus
}
#endif

#endif
