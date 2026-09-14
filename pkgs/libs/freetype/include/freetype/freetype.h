#ifndef FREETYPE_H_
#define FREETYPE_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FREETYPE_MAJOR 2
#define FREETYPE_MINOR 13
#define FREETYPE_PATCH 2

typedef signed long     FT_Pos;
typedef signed long     FT_Fixed;
typedef signed long     FT_Long;
typedef unsigned long   FT_ULong;
typedef signed int      FT_Int;
typedef unsigned int    FT_UInt;
typedef signed short    FT_Short;
typedef unsigned short  FT_UShort;
typedef unsigned char   FT_Byte;
typedef const FT_Byte*  FT_Bytes;
typedef signed char     FT_Char;
typedef signed int      FT_F26Dot6;
typedef int             FT_Error;
typedef unsigned char   FT_Bool;
typedef signed int      FT_Int32;
typedef unsigned int    FT_UInt32;

#define FT_Err_Ok                            0
#define FT_Err_Cannot_Open_Resource         1
#define FT_Err_Unknown_File_Format          2
#define FT_Err_Invalid_File_Format          3
#define FT_Err_Invalid_Version              4
#define FT_Err_Lower_Module_Version         5
#define FT_Err_Invalid_Argument             6
#define FT_Err_Unimplemented_Feature        7
#define FT_Err_Invalid_Table                8
#define FT_Err_Invalid_Offset               9
#define FT_Err_Array_Too_Large              10
#define FT_Err_Missing_Module               11
#define FT_Err_Missing_Property             12
#define FT_Err_Invalid_Glyph_Index          16
#define FT_Err_Invalid_Character_Code       17
#define FT_Err_Invalid_Glyph_Format         18
#define FT_Err_Cannot_Render_Glyph          19
#define FT_Err_Invalid_Outline              20
#define FT_Err_Invalid_Composite            21
#define FT_Err_Too_Many_Hints               22
#define FT_Err_Invalid_Pixel_Size           23
#define FT_Err_Invalid_Handle               32
#define FT_Err_Invalid_Library_Handle       33
#define FT_Err_Invalid_Driver_Handle        34
#define FT_Err_Invalid_Face_Handle          35
#define FT_Err_Invalid_Size_Handle          36
#define FT_Err_Invalid_Slot_Handle          37
#define FT_Err_Invalid_CharMap_Handle       38
#define FT_Err_Out_Of_Memory                64

#define FT_LOAD_DEFAULT                      0x0
#define FT_LOAD_NO_SCALE                     (1 << 0)
#define FT_LOAD_NO_HINTING                   (1 << 1)
#define FT_LOAD_RENDER                       (1 << 2)
#define FT_LOAD_NO_BITMAP                    (1 << 3)
#define FT_LOAD_VERTICAL_LAYOUT              (1 << 4)
#define FT_LOAD_FORCE_AUTOHINT               (1 << 5)
#define FT_LOAD_CROP_BITMAP                  (1 << 6)
#define FT_LOAD_PEDANTIC                     (1 << 7)
#define FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH  (1 << 9)
#define FT_LOAD_NO_RECURSE                   (1 << 10)
#define FT_LOAD_IGNORE_TRANSFORM             (1 << 11)
#define FT_LOAD_MONOCHROME                   (1 << 12)
#define FT_LOAD_LINEAR_DESIGN                (1 << 13)
#define FT_LOAD_NO_AUTOHINT                  (1 << 15)
#define FT_LOAD_TARGET_NORMAL                (0 << 16)
#define FT_LOAD_TARGET_LIGHT                 (1 << 16)
#define FT_LOAD_TARGET_MONO                  (2 << 16)
#define FT_LOAD_TARGET_LCD                   (3 << 16)
#define FT_LOAD_TARGET_LCD_V                 (4 << 16)

typedef enum FT_Pixel_Mode_ {
    FT_PIXEL_MODE_NONE = 0,
    FT_PIXEL_MODE_MONO,
    FT_PIXEL_MODE_GRAY,
    FT_PIXEL_MODE_GRAY2,
    FT_PIXEL_MODE_GRAY4,
    FT_PIXEL_MODE_LCD,
    FT_PIXEL_MODE_LCD_V,
    FT_PIXEL_MODE_BGRA
} FT_Pixel_Mode;

typedef enum FT_Render_Mode_ {
    FT_RENDER_MODE_NORMAL = 0,
    FT_RENDER_MODE_LIGHT,
    FT_RENDER_MODE_MONO,
    FT_RENDER_MODE_LCD,
    FT_RENDER_MODE_LCD_V,
    FT_RENDER_MODE_MAX
} FT_Render_Mode;

typedef struct FT_Vector_ {
    FT_Pos x;
    FT_Pos y;
} FT_Vector;

typedef struct FT_BBox_ {
    FT_Pos xMin;
    FT_Pos yMin;
    FT_Pos xMax;
    FT_Pos yMax;
} FT_BBox;

typedef struct FT_Bitmap_ {
    unsigned int rows;
    unsigned int width;
    int pitch;
    unsigned char *buffer;
    unsigned short num_grays;
    unsigned char pixel_mode;
    unsigned char palette_mode;
    void *palette;
} FT_Bitmap;

typedef struct FT_Glyph_Metrics_ {
    FT_Pos width;
    FT_Pos height;
    FT_Pos horiBearingX;
    FT_Pos horiBearingY;
    FT_Pos horiAdvance;
    FT_Pos vertBearingX;
    FT_Pos vertBearingY;
    FT_Pos vertAdvance;
} FT_Glyph_Metrics;

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_*    FT_Face;
typedef struct FT_GlyphSlotRec_* FT_GlyphSlot;
typedef struct FT_CharMapRec_*   FT_CharMap;

typedef struct FT_GlyphSlotRec_ {
    FT_Library library;
    FT_Face face;
    struct FT_GlyphSlotRec_* next;
    FT_UInt reserved;
    FT_Glyph_Metrics metrics;
    FT_Fixed linearHoriAdvance;
    FT_Fixed linearVertAdvance;
    FT_Vector advance;
    FT_Bitmap bitmap;
    FT_Int bitmap_left;
    FT_Int bitmap_top;
    void* internal;
} FT_GlyphSlotRec;

typedef struct FT_FaceRec_ {
    FT_Long num_faces;
    FT_Long face_index;
    FT_Long face_flags;
    FT_Long style_flags;
    FT_Long num_glyphs;
    char* family_name;
    char* style_name;
    FT_Int num_fixed_sizes;
    FT_GlyphSlot glyph;
    FT_CharMap charmap;
    FT_Long units_per_EM;
    FT_Short ascender;
    FT_Short descender;
    FT_Short height;
    FT_Short max_advance_width;
    FT_Short max_advance_height;
    void* internal;
} FT_FaceRec;

FT_Error FT_Init_FreeType(FT_Library *alibrary);
FT_Error FT_Done_FreeType(FT_Library library);

FT_Error FT_New_Face(
    FT_Library library,
    const char *filepathname,
    FT_Long face_index,
    FT_Face *aface
);

FT_Error FT_New_Memory_Face(
    FT_Library library,
    const FT_Byte *file_base,
    FT_Long file_size,
    FT_Long face_index,
    FT_Face *aface
);

FT_Error FT_Done_Face(FT_Face face);

FT_Error FT_Set_Char_Size(
    FT_Face face,
    FT_F26Dot6 char_width,
    FT_F26Dot6 char_height,
    FT_UInt horz_resolution,
    FT_UInt vert_resolution
);

FT_Error FT_Set_Pixel_Sizes(
    FT_Face face,
    FT_UInt pixel_width,
    FT_UInt pixel_height
);

FT_Error FT_Load_Glyph(
    FT_Face face,
    FT_UInt glyph_index,
    FT_Int32 load_flags
);

FT_Error FT_Load_Char(
    FT_Face face,
    FT_ULong char_code,
    FT_Int32 load_flags
);

FT_Error FT_Render_Glyph(
    FT_GlyphSlot slot,
    FT_Render_Mode render_mode
);

FT_UInt FT_Get_Char_Index(
    FT_Face face,
    FT_ULong charcode
);

void FT_Library_Version(
    FT_Library library,
    FT_Int *amajor,
    FT_Int *aminor,
    FT_Int *apatch
);

#ifdef __cplusplus
}
#endif

#endif
