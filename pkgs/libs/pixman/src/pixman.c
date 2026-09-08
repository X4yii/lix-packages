#include <stdlib.h>
#include <string.h>
#include "pixman.h"

int pixman_version(void) {
    return ((PIXMAN_VERSION_MAJOR << 16) | (PIXMAN_VERSION_MINOR << 8) | PIXMAN_VERSION_MICRO);
}

const char *pixman_version_string(void) {
    return PIXMAN_VERSION_STRING;
}

pixman_image_t *pixman_image_create_bits(
    pixman_format_code_t format,
    int width,
    int height,
    uint32_t *bits,
    int rowstride_bytes
) {
    if (width <= 0 || height <= 0) {
        return NULL;
    }

    pixman_image_t *img = (pixman_image_t *)malloc(sizeof(pixman_image_t));
    if (!img) {
        return NULL;
    }

    img->ref_count = 1;
    img->format = format;
    img->width = width;
    img->height = height;

    int min_stride = width * 4;
    if (format == PIXMAN_a8 || format == PIXMAN_a1) {
        min_stride = width;
    } else if (format == PIXMAN_r5g6b5) {
        min_stride = width * 2;
    }

    if (rowstride_bytes == 0) {
        img->stride = min_stride;
    } else {
        img->stride = rowstride_bytes;
    }

    if (bits != NULL) {
        img->bits = bits;
        img->owns_data = 0;
    } else {
        size_t total_bytes = (size_t)img->stride * (size_t)height;
        img->bits = (uint32_t *)malloc(total_bytes);
        if (!img->bits) {
            free(img);
            return NULL;
        }
        memset(img->bits, 0, total_bytes);
        img->owns_data = 1;
    }

    memset(&img->solid_color, 0, sizeof(pixman_color_t));
    return img;
}

pixman_image_t *pixman_image_create_solid_fill(const pixman_color_t *color) {
    if (!color) {
        return NULL;
    }

    pixman_image_t *img = (pixman_image_t *)malloc(sizeof(pixman_image_t));
    if (!img) {
        return NULL;
    }

    img->ref_count = 1;
    img->format = PIXMAN_solid;
    img->width = 1;
    img->height = 1;
    img->stride = 0;
    img->bits = NULL;
    img->owns_data = 0;
    img->solid_color = *color;

    return img;
}

pixman_image_t *pixman_image_ref(pixman_image_t *image) {
    if (image) {
        image->ref_count++;
    }
    return image;
}

int pixman_image_unref(pixman_image_t *image) {
    if (!image) {
        return 0;
    }

    image->ref_count--;
    if (image->ref_count <= 0) {
        if (image->owns_data && image->bits) {
            free(image->bits);
        }
        free(image);
        return 1;
    }
    return 0;
}

uint32_t *pixman_image_get_data(pixman_image_t *image) {
    return image ? image->bits : NULL;
}

int pixman_image_get_width(pixman_image_t *image) {
    return image ? image->width : 0;
}

int pixman_image_get_height(pixman_image_t *image) {
    return image ? image->height : 0;
}

int pixman_image_get_stride(pixman_image_t *image) {
    return image ? image->stride : 0;
}

pixman_format_code_t pixman_image_get_format(pixman_image_t *image) {
    return image ? image->format : (pixman_format_code_t)0;
}

static inline uint32_t get_pixel_argb(pixman_image_t *img, int x, int y) {
    if (!img) {
        return 0;
    }

    if (img->format == PIXMAN_solid) {
        uint32_t a = img->solid_color.alpha >> 8;
        uint32_t r = img->solid_color.red >> 8;
        uint32_t g = img->solid_color.green >> 8;
        uint32_t b = img->solid_color.blue >> 8;
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    if (x < 0 || x >= img->width || y < 0 || y >= img->height || !img->bits) {
        return 0;
    }

    uint8_t *row = ((uint8_t *)img->bits) + (y * img->stride);

    if (img->format == PIXMAN_a8r8g8b8 || img->format == PIXMAN_x8r8g8b8) {
        return ((uint32_t *)row)[x];
    } else if (img->format == PIXMAN_a8b8g8r8 || img->format == PIXMAN_x8b8g8r8) {
        uint32_t px = ((uint32_t *)row)[x];
        uint32_t a = (px >> 24) & 0xFF;
        uint32_t b = (px >> 16) & 0xFF;
        uint32_t g = (px >> 8) & 0xFF;
        uint32_t r = px & 0xFF;
        return (a << 24) | (r << 16) | (g << 8) | b;
    } else if (img->format == PIXMAN_r8g8b8a8) {
        uint32_t px = ((uint32_t *)row)[x];
        uint32_t r = (px >> 24) & 0xFF;
        uint32_t g = (px >> 16) & 0xFF;
        uint32_t b = (px >> 8) & 0xFF;
        uint32_t a = px & 0xFF;
        return (a << 24) | (r << 16) | (g << 8) | b;
    } else if (img->format == PIXMAN_a8) {
        uint8_t a = row[x];
        return (uint32_t)a << 24;
    } else if (img->format == PIXMAN_r5g6b5) {
        uint16_t px = ((uint16_t *)row)[x];
        uint32_t r = ((px >> 11) & 0x1F) * 255 / 31;
        uint32_t g = ((px >> 5) & 0x3F) * 255 / 63;
        uint32_t b = (px & 0x1F) * 255 / 31;
        return (0xFF000000) | (r << 16) | (g << 8) | b;
    }

    return 0;
}

static inline void set_pixel_argb(pixman_image_t *img, int x, int y, uint32_t val) {
    if (!img || x < 0 || x >= img->width || y < 0 || y >= img->height || !img->bits) {
        return;
    }

    uint8_t *row = ((uint8_t *)img->bits) + (y * img->stride);

    if (img->format == PIXMAN_a8r8g8b8 || img->format == PIXMAN_x8r8g8b8) {
        ((uint32_t *)row)[x] = val;
    } else if (img->format == PIXMAN_a8b8g8r8 || img->format == PIXMAN_x8b8g8r8) {
        uint32_t a = (val >> 24) & 0xFF;
        uint32_t r = (val >> 16) & 0xFF;
        uint32_t g = (val >> 8) & 0xFF;
        uint32_t b = val & 0xFF;
        ((uint32_t *)row)[x] = (a << 24) | (b << 16) | (g << 8) | r;
    } else if (img->format == PIXMAN_a8) {
        row[x] = (uint8_t)(val >> 24);
    } else if (img->format == PIXMAN_r5g6b5) {
        uint32_t r = ((val >> 16) & 0xFF) * 31 / 255;
        uint32_t g = ((val >> 8) & 0xFF) * 63 / 255;
        uint32_t b = (val & 0xFF) * 31 / 255;
        ((uint16_t *)row)[x] = (uint16_t)((r << 11) | (g << 5) | b);
    }
}

static inline uint32_t blend_porter_duff(pixman_op_t op, uint32_t src, uint32_t dst) {
    if (op == PIXMAN_OP_CLEAR) {
        return 0;
    }
    if (op == PIXMAN_OP_SRC) {
        return src;
    }
    if (op == PIXMAN_OP_DST) {
        return dst;
    }

    uint32_t sa = (src >> 24) & 0xFF;
    uint32_t sr = (src >> 16) & 0xFF;
    uint32_t sg = (src >> 8) & 0xFF;
    uint32_t sb = src & 0xFF;

    uint32_t da = (dst >> 24) & 0xFF;
    uint32_t dr = (dst >> 16) & 0xFF;
    uint32_t dg = (dst >> 8) & 0xFF;
    uint32_t db = dst & 0xFF;

    if (op == PIXMAN_OP_OVER) {
        if (sa == 255) {
            return src;
        }
        if (sa == 0) {
            return dst;
        }

        uint32_t inv_sa = 255 - sa;
        uint32_t out_a = sa + (da * inv_sa + 127) / 255;
        uint32_t out_r = sr + (dr * inv_sa + 127) / 255;
        uint32_t out_g = sg + (dg * inv_sa + 127) / 255;
        uint32_t out_b = sb + (db * inv_sa + 127) / 255;

        if (out_a > 255) out_a = 255;
        if (out_r > 255) out_r = 255;
        if (out_g > 255) out_g = 255;
        if (out_b > 255) out_b = 255;

        return (out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
    }

    if (op == PIXMAN_OP_ADD) {
        uint32_t out_a = sa + da;
        uint32_t out_r = sr + dr;
        uint32_t out_g = sg + dg;
        uint32_t out_b = sb + db;

        if (out_a > 255) out_a = 255;
        if (out_r > 255) out_r = 255;
        if (out_g > 255) out_g = 255;
        if (out_b > 255) out_b = 255;

        return (out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
    }

    if (op == PIXMAN_OP_XOR) {
        uint32_t inv_sa = 255 - sa;
        uint32_t inv_da = 255 - da;
        uint32_t out_a = (sa * inv_da + da * inv_sa + 127) / 255;
        uint32_t out_r = (sr * inv_da + dr * inv_sa + 127) / 255;
        uint32_t out_g = (sg * inv_da + dg * inv_sa + 127) / 255;
        uint32_t out_b = (sb * inv_da + db * inv_sa + 127) / 255;

        if (out_a > 255) out_a = 255;
        if (out_r > 255) out_r = 255;
        if (out_g > 255) out_g = 255;
        if (out_b > 255) out_b = 255;

        return (out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
    }

    return src;
}

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
) {
    if (!dest || width <= 0 || height <= 0) {
        return;
    }

    int32_t clip_x1 = dest_x;
    int32_t clip_y1 = dest_y;
    int32_t clip_x2 = dest_x + width;
    int32_t clip_y2 = dest_y + height;

    if (clip_x1 < 0) clip_x1 = 0;
    if (clip_y1 < 0) clip_y1 = 0;
    if (clip_x2 > dest->width) clip_x2 = dest->width;
    if (clip_y2 > dest->height) clip_y2 = dest->height;

    if (clip_x1 >= clip_x2 || clip_y1 >= clip_y2) {
        return;
    }

    for (int32_t dy = clip_y1; dy < clip_y2; dy++) {
        int32_t sy = src_y + (dy - dest_y);
        int32_t my = mask_y + (dy - dest_y);

        for (int32_t dx = clip_x1; dx < clip_x2; dx++) {
            int32_t sx = src_x + (dx - dest_x);
            int32_t mx = mask_x + (dx - dest_x);

            uint32_t s_pixel = get_pixel_argb(src, sx, sy);

            if (mask) {
                uint32_t m_pixel = get_pixel_argb(mask, mx, my);
                uint32_t m_alpha = (m_pixel >> 24) & 0xFF;
                if (m_alpha == 0) {
                    continue;
                }

                if (m_alpha != 255) {
                    uint32_t sa = (s_pixel >> 24) & 0xFF;
                    uint32_t sr = (s_pixel >> 16) & 0xFF;
                    uint32_t sg = (s_pixel >> 8) & 0xFF;
                    uint32_t sb = s_pixel & 0xFF;

                    sa = (sa * m_alpha + 127) / 255;
                    sr = (sr * m_alpha + 127) / 255;
                    sg = (sg * m_alpha + 127) / 255;
                    sb = (sb * m_alpha + 127) / 255;

                    s_pixel = (sa << 24) | (sr << 16) | (sg << 8) | sb;
                }
            }

            uint32_t d_pixel = get_pixel_argb(dest, dx, dy);
            uint32_t out_pixel = blend_porter_duff(op, s_pixel, d_pixel);
            set_pixel_argb(dest, dx, dy, out_pixel);
        }
    }
}

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
) {
    pixman_image_composite32(
        op,
        src,
        mask,
        dest,
        (int32_t)src_x,
        (int32_t)src_y,
        (int32_t)mask_x,
        (int32_t)mask_y,
        (int32_t)dest_x,
        (int32_t)dest_y,
        (int32_t)width,
        (int32_t)height
    );
}

int pixman_image_fill_rectangles(
    pixman_op_t op,
    pixman_image_t *image,
    const pixman_color_t *color,
    int n_rects,
    const pixman_rectangle16_t *rects
) {
    if (!image || !color || n_rects <= 0 || !rects) {
        return 0;
    }

    pixman_image_t *solid = pixman_image_create_solid_fill(color);
    if (!solid) {
        return 0;
    }

    for (int i = 0; i < n_rects; i++) {
        pixman_image_composite32(
            op,
            solid,
            NULL,
            image,
            0,
            0,
            0,
            0,
            rects[i].x,
            rects[i].y,
            rects[i].width,
            rects[i].height
        );
    }

    pixman_image_unref(solid);
    return 1;
}
