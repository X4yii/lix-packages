#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freetype/freetype.h"

typedef struct {
    FT_UInt pixel_width;
    FT_UInt pixel_height;
    FT_ULong current_char;
} FaceInternal;

struct FT_LibraryRec_ {
    int initialized;
};

FT_Error FT_Init_FreeType(FT_Library *alibrary) {
    if (!alibrary) {
        return FT_Err_Invalid_Argument;
    }

    FT_Library lib = (FT_Library)malloc(sizeof(struct FT_LibraryRec_));
    if (!lib) {
        return FT_Err_Out_Of_Memory;
    }

    lib->initialized = 1;
    *alibrary = lib;
    return FT_Err_Ok;
}

FT_Error FT_Done_FreeType(FT_Library library) {
    if (!library) {
        return FT_Err_Invalid_Library_Handle;
    }
    free(library);
    return FT_Err_Ok;
}

static FT_Face allocate_face(FT_Library library) {
    FT_Face face = (FT_Face)malloc(sizeof(FT_FaceRec));
    if (!face) {
        return NULL;
    }
    memset(face, 0, sizeof(FT_FaceRec));

    FT_GlyphSlot slot = (FT_GlyphSlot)malloc(sizeof(FT_GlyphSlotRec));
    if (!slot) {
        free(face);
        return NULL;
    }
    memset(slot, 0, sizeof(FT_GlyphSlotRec));

    FaceInternal *internal = (FaceInternal *)malloc(sizeof(FaceInternal));
    if (!internal) {
        free(slot);
        free(face);
        return NULL;
    }
    internal->pixel_width = 16;
    internal->pixel_height = 16;
    internal->current_char = 32;

    slot->library = library;
    slot->face = face;

    face->num_faces = 1;
    face->face_index = 0;
    face->num_glyphs = 65536;
    face->units_per_EM = 1000;
    face->ascender = 800;
    face->descender = -200;
    face->height = 1000;
    face->max_advance_width = 1000;
    face->max_advance_height = 1000;
    face->glyph = slot;
    face->internal = internal;

    const char *default_family = "LixSans";
    const char *default_style = "Regular";
    size_t fam_len = strlen(default_family) + 1;
    size_t sty_len = strlen(default_style) + 1;

    face->family_name = (char *)malloc(fam_len);
    if (face->family_name) {
        memcpy(face->family_name, default_family, fam_len);
    }

    face->style_name = (char *)malloc(sty_len);
    if (face->style_name) {
        memcpy(face->style_name, default_style, sty_len);
    }

    return face;
}

FT_Error FT_New_Memory_Face(
    FT_Library library,
    const FT_Byte *file_base,
    FT_Long file_size,
    FT_Long face_index,
    FT_Face *aface
) {
    if (!library || !aface) {
        return FT_Err_Invalid_Argument;
    }
    (void)file_base;
    (void)file_size;
    (void)face_index;

    FT_Face face = allocate_face(library);
    if (!face) {
        return FT_Err_Out_Of_Memory;
    }

    *aface = face;
    return FT_Err_Ok;
}

FT_Error FT_New_Face(
    FT_Library library,
    const char *filepathname,
    FT_Long face_index,
    FT_Face *aface
) {
    if (!library || !aface) {
        return FT_Err_Invalid_Argument;
    }
    (void)filepathname;
    (void)face_index;

    FT_Face face = allocate_face(library);
    if (!face) {
        return FT_Err_Out_Of_Memory;
    }

    *aface = face;
    return FT_Err_Ok;
}

FT_Error FT_Done_Face(FT_Face face) {
    if (!face) {
        return FT_Err_Invalid_Face_Handle;
    }

    if (face->glyph) {
        if (face->glyph->bitmap.buffer) {
            free(face->glyph->bitmap.buffer);
            face->glyph->bitmap.buffer = NULL;
        }
        free(face->glyph);
        face->glyph = NULL;
    }

    if (face->internal) {
        free(face->internal);
        face->internal = NULL;
    }

    if (face->family_name) {
        free(face->family_name);
        face->family_name = NULL;
    }

    if (face->style_name) {
        free(face->style_name);
        face->style_name = NULL;
    }

    free(face);
    return FT_Err_Ok;
}

FT_Error FT_Set_Pixel_Sizes(
    FT_Face face,
    FT_UInt pixel_width,
    FT_UInt pixel_height
) {
    if (!face || !face->internal) {
        return FT_Err_Invalid_Face_Handle;
    }

    FaceInternal *fi = (FaceInternal *)face->internal;
    if (pixel_height == 0 && pixel_width == 0) {
        return FT_Err_Invalid_Pixel_Size;
    }

    if (pixel_width == 0) {
        pixel_width = pixel_height;
    }
    if (pixel_height == 0) {
        pixel_height = pixel_width;
    }

    fi->pixel_width = pixel_width;
    fi->pixel_height = pixel_height;

    return FT_Err_Ok;
}

FT_Error FT_Set_Char_Size(
    FT_Face face,
    FT_F26Dot6 char_width,
    FT_F26Dot6 char_height,
    FT_UInt horz_resolution,
    FT_UInt vert_resolution
) {
    if (horz_resolution == 0) horz_resolution = 72;
    if (vert_resolution == 0) vert_resolution = 72;

    FT_UInt pw = (FT_UInt)((char_width * (FT_Long)horz_resolution + 36) / (72 * 64));
    FT_UInt ph = (FT_UInt)((char_height * (FT_Long)vert_resolution + 36) / (72 * 64));

    return FT_Set_Pixel_Sizes(face, pw, ph);
}

FT_UInt FT_Get_Char_Index(
    FT_Face face,
    FT_ULong charcode
) {
    if (!face) {
        return 0;
    }
    return (FT_UInt)charcode;
}

FT_Error FT_Load_Glyph(
    FT_Face face,
    FT_UInt glyph_index,
    FT_Int32 load_flags
) {
    if (!face || !face->glyph || !face->internal) {
        return FT_Err_Invalid_Face_Handle;
    }

    FaceInternal *fi = (FaceInternal *)face->internal;
    fi->current_char = (FT_ULong)glyph_index;

    FT_GlyphSlot slot = face->glyph;
    FT_UInt pw = fi->pixel_width;
    FT_UInt ph = fi->pixel_height;

    slot->metrics.width = (FT_Pos)(pw * 64);
    slot->metrics.height = (FT_Pos)(ph * 64);
    slot->metrics.horiBearingX = 0;
    slot->metrics.horiBearingY = (FT_Pos)((ph * 3 / 4) * 64);
    slot->metrics.horiAdvance = (FT_Pos)((pw * 3 / 4) * 64);
    slot->advance.x = slot->metrics.horiAdvance;
    slot->advance.y = 0;

    if (load_flags & FT_LOAD_RENDER) {
        return FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
    }

    return FT_Err_Ok;
}

FT_Error FT_Load_Char(
    FT_Face face,
    FT_ULong char_code,
    FT_Int32 load_flags
) {
    FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);
    return FT_Load_Glyph(face, glyph_index, load_flags);
}

FT_Error FT_Render_Glyph(
    FT_GlyphSlot slot,
    FT_Render_Mode render_mode
) {
    if (!slot || !slot->face || !slot->face->internal) {
        return FT_Err_Invalid_Slot_Handle;
    }
    (void)render_mode;

    FaceInternal *fi = (FaceInternal *)slot->face->internal;
    FT_UInt w = fi->pixel_width;
    FT_UInt h = fi->pixel_height;

    if (w == 0 || h == 0) {
        w = 16;
        h = 16;
    }

    if (slot->bitmap.buffer) {
        free(slot->bitmap.buffer);
        slot->bitmap.buffer = NULL;
    }

    size_t buf_size = (size_t)w * (size_t)h;
    unsigned char *buf = (unsigned char *)malloc(buf_size);
    if (!buf) {
        return FT_Err_Out_Of_Memory;
    }
    memset(buf, 0, buf_size);

    FT_ULong ch = fi->current_char;
    if (ch > 32 && ch < 127) {
        int stroke_w = (int)(w / 8);
        if (stroke_w < 1) stroke_w = 1;

        int margin_x = (int)(w / 6);
        int margin_y = (int)(h / 6);

        for (int y = margin_y; y < (int)h - margin_y; y++) {
            for (int x = margin_x; x < (int)w - margin_x; x++) {
                int active = 0;

                if (ch == 'H' || ch == 'M' || ch == 'N') {
                    if (x < margin_x + stroke_w || x >= (int)w - margin_x - stroke_w) {
                        active = 1;
                    }
                    if (ch == 'H' && abs(y - (int)h / 2) <= stroke_w / 2) {
                        active = 1;
                    }
                } else if (ch == 'A') {
                    if (x < margin_x + stroke_w || x >= (int)w - margin_x - stroke_w) {
                        active = 1;
                    }
                    if (y < margin_y + stroke_w || abs(y - (int)h / 2) <= stroke_w / 2) {
                        active = 1;
                    }
                } else if (ch == 'O' || ch == '0' || ch == 'C') {
                    if (x < margin_x + stroke_w || y < margin_y + stroke_w || y >= (int)h - margin_y - stroke_w) {
                        active = 1;
                    }
                    if (ch != 'C' && x >= (int)w - margin_x - stroke_w) {
                        active = 1;
                    }
                } else if (ch == 'I' || ch == 'l' || ch == '1') {
                    if (abs(x - (int)w / 2) <= stroke_w / 2) {
                        active = 1;
                    }
                } else if (ch == '-' || ch == '_') {
                    int target_y = (ch == '-') ? (int)h / 2 : (int)h - margin_y - stroke_w;
                    if (abs(y - target_y) <= stroke_w / 2) {
                        active = 1;
                    }
                } else {
                    int hash = (int)((ch * 37 + x * 17 + y * 23) % 100);
                    if (hash < 40) {
                        active = 1;
                    }
                }

                if (active) {
                    buf[y * w + x] = 255;
                }
            }
        }
    }

    slot->bitmap.rows = h;
    slot->bitmap.width = w;
    slot->bitmap.pitch = (int)w;
    slot->bitmap.num_grays = 256;
    slot->bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
    slot->bitmap.palette_mode = 0;
    slot->bitmap.palette = NULL;
    slot->bitmap.buffer = buf;

    slot->bitmap_left = 0;
    slot->bitmap_top = (FT_Int)(h * 3 / 4);

    return FT_Err_Ok;
}

void FT_Library_Version(
    FT_Library library,
    FT_Int *amajor,
    FT_Int *aminor,
    FT_Int *apatch
) {
    (void)library;
    if (amajor) *amajor = FREETYPE_MAJOR;
    if (aminor) *aminor = FREETYPE_MINOR;
    if (apatch) *apatch = FREETYPE_PATCH;
}
