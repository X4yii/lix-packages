#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <pixman.h>

static const char *shades[] = {
    " ", ".", ":", "-", "=", "+", "*", "#", "%", "@"
};

int main(int argc, char **argv) {
    const char *text = "LixOS 0.0.4";
    if (argc > 1) {
        text = argv[1];
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft) != 0) {
        fprintf(stderr, "fontview: error al inicializar FreeType\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "default", 0, &face) != 0) {
        fprintf(stderr, "fontview: error al cargar tipografia\n");
        FT_Done_FreeType(ft);
        return 2;
    }

    int pt_size = 16;
    if (FT_Set_Pixel_Sizes(face, pt_size, pt_size) != 0) {
        fprintf(stderr, "fontview: error al configurar tamaño de fuente\n");
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return 3;
    }

    size_t len = strlen(text);
    int char_spacing = (pt_size * 3) / 4;
    int canvas_w = (int)len * char_spacing + pt_size;
    int canvas_h = pt_size + 8;

    pixman_image_t *canvas = pixman_image_create_bits(
        PIXMAN_a8r8g8b8,
        canvas_w,
        canvas_h,
        NULL,
        canvas_w * 4
    );

    if (!canvas) {
        fprintf(stderr, "fontview: error al crear lienzo Pixman\n");
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return 4;
    }

    pixman_color_t bg = { 0x0800, 0x0800, 0x1000, 0xffff };
    pixman_rectangle16_t bg_rect = { 0, 0, (uint16_t)canvas_w, (uint16_t)canvas_h };
    pixman_image_fill_rectangles(PIXMAN_OP_SRC, canvas, &bg, 1, &bg_rect);

    pixman_color_t fg = { 0x2000, 0xe000, 0xffff, 0xffff };
    pixman_image_t *fg_solid = pixman_image_create_solid_fill(&fg);

    int cur_x = 4;
    for (size_t i = 0; i < len; i++) {
        char ch = text[i];
        if (FT_Load_Char(face, (FT_ULong)ch, FT_LOAD_RENDER) != 0) {
            cur_x += char_spacing;
            continue;
        }

        FT_GlyphSlot slot = face->glyph;
        if (slot && slot->bitmap.buffer && slot->bitmap.width > 0 && slot->bitmap.rows > 0) {
            pixman_image_t *mask = pixman_image_create_bits(
                PIXMAN_a8,
                slot->bitmap.width,
                slot->bitmap.rows,
                (uint32_t *)slot->bitmap.buffer,
                slot->bitmap.pitch
            );

            double wave = sin((double)i * 0.7) * 2.0;
            int cur_y = 4 + (int)round(wave);
            if (cur_y < 0) cur_y = 0;

            pixman_image_composite(
                PIXMAN_OP_OVER,
                fg_solid,
                mask,
                canvas,
                0, 0,
                0, 0,
                (int16_t)cur_x,
                (int16_t)cur_y,
                (uint16_t)slot->bitmap.width,
                (uint16_t)slot->bitmap.rows
            );

            pixman_image_unref(mask);
        }

        cur_x += char_spacing;
    }

    printf("\n--- Renderizado Tipografico Vectorial LixOS (FreeType + Pixman + Libm) ---\n\n");
    uint32_t *pixels = pixman_image_get_data(canvas);

    for (int y = 0; y < canvas_h; y++) {
        for (int x = 0; x < canvas_w; x++) {
            uint32_t px = pixels[y * canvas_w + x];
            uint32_t g = (px >> 8) & 0xFF;
            uint32_t b = px & 0xFF;
            int brightness = (int)((g + b) / 2);
            int idx = (brightness * 9) / 255;
            if (idx < 0) idx = 0;
            if (idx > 9) idx = 9;
            fputs(shades[idx], stdout);
        }
        fputc('\n', stdout);
    }
    printf("\n[LixOS Fontview] Dimensiones: %dx%d pixeles | Glifos renderizados: %zu\n\n",
           canvas_w, canvas_h, len);

    pixman_image_unref(fg_solid);
    pixman_image_unref(canvas);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 0;
}
