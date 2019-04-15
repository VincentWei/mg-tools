/*
** $Id: varbitmap.h 8944 2007-12-29 08:29:16Z xwyan $
**
** varbitmap.h: the head file of raw bitmap font operation set.
**
** Copyright (C) 2000 ~ 2002, Wei Yongming.
** Copyright (C) 2003 ~ 2007  FMSoft.
** 
*/

#ifndef GUI_FONT_VARBITMAP_H
    #define GUI_FONT_VARBITMAP_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


typedef struct _VBF_BBOX
{
        char x, y, w, h;
} VBF_BBOX;

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

    const VBF_BBOX* bbox;     /* The pointer to the glyph bounding box array or NULL. */

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
} VBFINFO;

#define LEN_VERSION_INFO 10
#define LEN_VENDER_INFO  12

#define VBF_VERSION3       "vbf-3.0**"

#define SBC_VARFONT_INFO(logfont) ((VBFINFO*)(((DEVFONT*) (logfont.sbc_devfont))->data))
#define MBC_VARFONT_INFO(logfont) ((VBFINFO*)(((DEVFONT*) (logfont.mbc_devfont))->data))

#define SBC_VARFONT_INFO_P(logfont) ((VBFINFO*)(((DEVFONT*) (logfont->sbc_devfont))->data))
#define MBC_VARFONT_INFO_P(logfont) ((VBFINFO*)(((DEVFONT*) (logfont->mbc_devfont))->data))

#define VARFONT_INFO_P(devfont) ((VBFINFO*)(devfont->data))
#define VARFONT_INFO(devfont) ((VBFINFO*)(devfont.data))

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_FONT_VARBITMAP_H

