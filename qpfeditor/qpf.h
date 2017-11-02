/*
** $Id: qpf.h 153 2007-08-03 07:55:49Z xgwang $
** 
** qpf.h: qpfeditor header file.
**
** Copyright (C) 2007 Feynman Software
**
** Create date: 2007/07/27
*/

#ifndef __QPF_H_
#define __QPF_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define FM_SMOOTH   1

typedef struct _GLYPHMETRICS {
    Uint8 linestep;
    Uint8 width;
    Uint8 height;
    Uint8 padding;

    Sint8 bearingx;     /* Difference from pen position to glyph's left bbox */
    Uint8 advance;      /* Difference between pen positions */
    Sint8 bearingy;     /* Used for putting characters on baseline */

    Sint8 reserved;     /* Do not use */
} GLYPHMETRICS;

typedef struct _GLYPH
{
    GLYPHMETRICS*  metrics;
    unsigned char* data;
	BOOL           size_change;
} GLYPH;

typedef struct _GLYPHTREE
{
    unsigned int min, max;
    unsigned char min_rw, min_cl;
    unsigned char max_rw, max_cl;
    unsigned char flags;

    struct _GLYPHTREE* less;
    struct _GLYPHTREE* more;
	struct _GLYPHTREE* parent;

    GLYPH*             glyph;
	BOOL               glyph_size_change;
} GLYPHTREE;

typedef struct _QPFMETRICS
{
    Sint8 ascent, descent;
    Sint8 leftbearing, rightbearing;
    Uint8 maxwidth;
    Sint8 leading;
    Uint8 flags;
    Uint8 underlinepos;
    Uint8 underlinewidth;
    Uint8 reserved3;
} QPFMETRICS;

typedef struct 
{
    char font_name [LEN_UNIDEVFONT_NAME + 1];
    unsigned int height;
    unsigned int width;

    unsigned char   first_char;     /* first character in this font */
    unsigned char   last_char;      /* last character in this font */

    unsigned int    file_size;
    QPFMETRICS*     fm;

    GLYPHTREE*      tree;

	BOOL            file_size_change;
	BOOL            file_add_char;
	int             nr_node;

} QPFINFO;

BOOL load_qpf(QPFINFO* vbf, const char* file);
void free_qpf(QPFINFO *qpf_info);

BOOL dump_qpf(const QPFINFO* qpf_info, char* file);

void glyph_free (GLYPHBITMAP* glyph);
BOOL glyph_get_bit(GLYPHBITMAP* glyph, int row, int col);
void glyph_set_bit(GLYPHBITMAP* glyph, int row, int col, BOOL value);
void glyph_move_left(GLYPHBITMAP* glyph);
void glyph_move_right(GLYPHBITMAP* glyph);
void glyph_move_up(GLYPHBITMAP* glyph);
void glyph_move_down(GLYPHBITMAP* glyph);
void glyph_draw(GLYPHBITMAP* glyph, HDC hdc, int x, int y, int pixel_size, gal_pixel color);

void glyph_bitmap_copy(GLYPHBITMAP* to, GLYPHBITMAP* from);
void glyph_change_width_right(GLYPHBITMAP* glyph, int width);
void glyph_change_width_left(GLYPHBITMAP* glyph, int width);
void glyph_change_height_bottom(GLYPHBITMAP* glyph, int height);
void glyph_change_height_top(GLYPHBITMAP* glyph, int height);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __QPF_H_ */

