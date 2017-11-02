/*
** Copyright (C) 2003, Feynman Software.
*/

#ifndef _VBF_H

#define _VBF_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*#define VBF_VERSION                 "vbf-1.0**"
#define VBF_VERSION2                "vbf-2.0**"
#define LEN_VERSION_INFO            10*/

/*typedef struct
{
    unsigned char width;
    int bbox_x;
    int bbox_y;
    int bbox_w;
    int bbox_h;
    unsigned char* bits;
} VBFCHARINFO;*/

//typedef struct
//{
//    char            name [LEN_DEVFONT_NAME + 1];
//                                    /* font name */
//    char            version [LEN_VERSION_INFO + 1];
//                                    /* font version */
//    unsigned char   max_width;      /* max width in pixels */
//    unsigned char   ave_width;      /* average width in pixels */
//    int             height;         /* height in pixels */
//    int             descent;        /* pixels below the base line */
//    unsigned char   first_char;     /* first character in this font */
//    unsigned char   last_char;      /* last character in this font */
//    unsigned char   def_char;       /* default character in this font */
//    VBFCHARINFO*    char_info;
//    unsigned short  offset [256];   /* glyph offsets into bitmap data or NULL */
//    unsigned char   width [256];    /* character widths or NULL */
//    unsigned char*  bits [256];     /* 8-bit right-padded bitmap data */
//    int             len_bits;       /* lenght of bits */
//} VBFINFO;


#define HEADER_LEN 68

typedef struct _VBF_BBOX
{
        char x, y, w, h;
        struct _VBF_BBOX*  next;
} VBF_BBOX;

typedef struct _ADVANCE_X{
    char advance_x;
    struct _ADVANCE_X*  next;
} ADVANCE_X;

typedef struct _ADVANCE_Y{
    char advance_y;
    struct _ADVANCE_Y* next;
} ADVANCE_Y;

typedef struct _GLYPH_BITS{
    unsigned char*  glyph_bits;
    int             bmp_size;
    BOOL            is_default_glyph;
    struct _GLYPH_BITS*     next;
}GLYPH_BITS;

typedef struct _MYVBF_BBOX
{
        char x, y, w, h;
} MYVBF_BBOX;

typedef struct
{
    char ver_info [4];  /* font version*/
    const char* name;         /* font name*/

    char max_width;     /* The max width in pixels. */
    char ave_width;     /* The average width in pixels. */
    char height;        /* The height in pixels. */
    char descent;       /* The pixels below the base line. */

    int first_glyph;    /* The glyph number of the first character in this font.*/
    int last_glyph;     /* The glyph number of the last character in this font.*/
    int def_glyph;      /* The glyph number of the default character in this font.*/

    unsigned int font_size; /* used by mmap. It should be zero for in-core vbfs. */

    VBF_BBOX* bbox;     /* The pointer to the glyph bounding box array or NULL. */

    ADVANCE_X* advance_x;    /* The pointer to the glyph advance x array. 
                           If bbox is NULL, the array contains the width of each glyph.
                           If advance_x is NULL, the glyph has the universal width, 
                           that's max_width. */
    ADVANCE_Y* advance_y;    /* The pointer to the glyph advance y array.
                           If bbox is NULL, advance_y should be NULL.
                           If bbox is not NULL and advance_y is NULL, 
                           all glyph has the universal advance_y, that is, zero. */

    //const unsigned int* bits_offset;      /* The pointer to the glyph bitmap offset array 
    //                                   whose number will be used to fetch glyph bitmap 
    //                                   from bits array or NULL. */
    GLYPH_BITS* all_glyph_bits;  /* The 8-bit right-padded bitmap data for all glyphs. */
} VBFINFO;

typedef struct
{
    char ver_info [4];  /* font version*/
    const char* name;         /* font name*/

    char max_width;     /* The max width in pixels. */
    char ave_width;     /* The average width in pixels. */
    char height;        /* The height in pixels. */
    char descent;       /* The pixels below the base line. */

    int first_glyph;    /* The glyph number of the first character in this font.*/
    int last_glyph;     /* The glyph number of the last character in this font.*/
    int def_glyph;      /* The glyph number of the default character in this font.*/

    unsigned int font_size; /* used by mmap. It should be zero for in-core vbfs. */

    const MYVBF_BBOX* bbox;     /* The pointer to the glyph bounding box array or NULL. */

    const char* advance_x;    /* The pointer to the glyph advance x array. 
                           If bbox is NULL, the array contains the width of each glyph.
                           If advance_x is NULL, the glyph has the universal width, 
                           that's max_width. */
    const char* advance_y;    /* The pointer to the glyph advance y array.
                           If bbox is NULL, advance_y should be NULL.
                           If bbox is not NULL and advance_y is NULL, 
                           all glyph has the universal advance_y, that is, zero. */

    const unsigned int* bits_offset;      /* The pointer to the glyph bitmap offset array 
                                       whose number will be used to fetch glyph bitmap 
                                       from bits array or NULL. */
    const unsigned char* all_glyph_bits;  /* The 8-bit right-padded bitmap data for all glyphs. */
} MYVBFINFO;


#define LEN_VERSION_INFO 10
#define LEN_VENDER_INFO  12

#define VBF_VERSION3       "vbf-3.0**"
//#define VBF_VERSION3       "3.0"


typedef int BOOL;
#define TRUE    1
#define FALSE   0

#define MGUI_LIL_ENDIAN  1234
#define MGUI_BIG_ENDIAN  4321

/* Assume that the host is little endian */
#define MGUI_BYTEORDER   MGUI_LIL_ENDIAN

#define LEN_DEVFONT_NAME            79

/********************** Load/Unload of var bitmap font ***********************/
typedef struct _FILE_LAYOUT {
    Uint32 off_bboxs; 
    Uint32 len_bboxs; 

    Uint32 off_advxs; 
    Uint32 len_advxs; 

    Uint32 off_advys; 
    Uint32 len_advys; 

    Uint32 off_bit_offs; 
    Uint32 len_bit_offs; 

    Uint32 off_bits; 
    Uint32 len_bits; 

    Uint32 font_size; 
} FILE_LAYOUT;

//static FILE_LAYOUT layout;
    

typedef struct _FONT_PROPT {
    char font_name [LEN_DEVFONT_NAME + 1];
    char max_width; 
    char ave_width; 
    char height; 
    char descent; 

    int first_glyph; 
    int last_glyph; 
    int def_glyph; 
} FONT_PROPT;



BOOL loadVBF (VBFINFO* vbf, const char* file);
void freeVBF (VBFINFO* vbf);
BOOL dumpVBF (const VBFINFO* vbf, char* file);

void glyph_free (GLYPHBITMAP* glyph);
BOOL glyph_get_bit(GLYPHBITMAP* glyph, int row, int col);
void glyph_set_bit(GLYPHBITMAP* glyph, int row, int col, BOOL value);
void glyph_move_left(GLYPHBITMAP* glyph);
void glyph_move_right(GLYPHBITMAP* glyph);
void glyph_move_up(GLYPHBITMAP* glyph);
void glyph_move_down(GLYPHBITMAP* glyph);
void glyph_draw(GLYPHBITMAP* glyph, HDC hdc, int x, int y, int pixel_size, gal_pixel color);

void glyph_copy(GLYPHBITMAP* to, GLYPHBITMAP* from);
void glyph_change_width_right(GLYPHBITMAP* glyph, int width);
void glyph_change_width_left(GLYPHBITMAP* glyph, int width);
void glyph_change_height_bottom(GLYPHBITMAP* glyph, int height);
void glyph_change_height_top(GLYPHBITMAP* glyph, int height);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _VBF_H */

