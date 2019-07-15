///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/*
** This file is part of mg-tools, a collection of programs to convert
** and maintain the resource for MiniGUI.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __UPF_H_
#define __UPF_H_

#if (_MINIGUI_VERSION_CODE < _VERSION_CODE(4,0,0))
#   define Uchar32 UChar32
#   define Uchar16 UChar16
#endif

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define FM_SMOOTH   1


typedef struct 
{
    /* BBOX information of the glyph */
    Sint8 bearingx;
    Sint8 bearingy;
    Uint8 width;
    Uint8 height;
    /* advance value of the glyph */
    Uint8 pitch;
    /* Paddings for alignment */
    Uint8 padding1;
    Uint8 advance;
    /* The pitch of the bitmap (bytes of one scan line) */
    Uint8 padding2;
     /* the index of the glyph bitmap in the bitmap section of this font file. */
    Uint32 bitmap_offset;
}UPFGLYPH;


typedef struct 
{
    Uint32 min;
    Uint32 max;
    Uint32 less_offset;
    Uint32 more_offset;
    Uint32 glyph_offset;
}UPFNODE;


typedef struct _GLYPHMETRICS {
    UPFGLYPH  upf_glyph;
} GLYPHMETRICS;

typedef struct _GLYPH
{
    GLYPHMETRICS*  metrics;
    unsigned char* data;
} GLYPH;

typedef struct _GLYPHTREE
{
    UPFNODE   upf_node;

    struct _GLYPHTREE* less;
    struct _GLYPHTREE* more;
    struct _GLYPHTREE* parent;

    GLYPH*             glyph;
} UPFGLYPHTREE;

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


#define LEN_VERSION_MAX       10
#define LEN_VENDER_NAME_MAX   12
#define LEN_DEVFONT_NAME_MAX  127


typedef struct 
{
    char     ver_info [LEN_VERSION_MAX];
    char     vender_name [LEN_VENDER_NAME_MAX];
    Uint16   endian;

    char     font_name [LEN_DEVFONT_NAME_MAX + 1];

    Uint8    width;
    Uint8    height;
    Sint8    ascent;
    Sint8    descent;

    Uint8    max_width;
    Uint8    min_width;
    Sint8    left_bearing;
    Sint8    right_bearing;

    Uint8    underline_pos;
    Uint8    underline_width;
    Sint8    leading;
    Uint8    mono_bitmap;

    Uint32   off_nodes;
    Uint32   off_glyphs;
    Uint32   off_bitmaps;

    Uint32   len_nodes;
    Uint32   len_glyphs;
    Uint32   len_bitmaps;

    Uint32   nr_glyph;
    Uint32   nr_zones;
    Uint32   font_size;
} UPFV1_FILE_HEADER;

typedef struct 
{
    UPFV1_FILE_HEADER   upf_file_header;
    UPFGLYPHTREE*       upf_tree;   
} UPFINFO;

typedef struct 
{
    char       font_name [LEN_UNIDEVFONT_NAME + 1];

    Uint8      width;
    Uint8      height;
    Sint8      ascent;
    Sint8      descent;
    Uint8      max_width;
    Uint8      underline_pos;
    Uint8      underline_width;
    Sint8      leading;
    Uint8      mono_bitmap;
    Uint8      reserved[3];
    void*      root_dir;
    Uint32     file_size;
} MGUPFINFO;

BOOL load_upf(UPFINFO* upf_info, const char* file);
void free_upf(UPFINFO *upf_info);

BOOL dump_upf(UPFINFO* upf_info, const char* file);

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


void free_node (UPFGLYPHTREE** node);
void free_glyph_tree(UPFGLYPHTREE** tree);

void combine_node (UPFGLYPHTREE* ancestor, UPFGLYPHTREE* offspring);

void divide_node (UPFGLYPHTREE* node, int dividing_line);

void compress (UPFGLYPHTREE* root, UPFGLYPHTREE** min_offspring, 
        UPFGLYPHTREE** max_offspring); 

int balance (UPFGLYPHTREE **root, int* l_num, int* m_num);


BOOL upf_get_char_glyph_bitmap (UPFINFO* upf_info, int ch, 
        GLYPHBITMAP* glyph_bitmap);

void upf_set_char_glyph (UPFINFO* upf_info, int ch, 
        GLYPHBITMAP* glyph_bitmap);


BOOL upf_add_char_glyph (UPFINFO* upf_info, int ch);
BOOL upf_delete_char_glyph (UPFINFO* upf_info, int ch);

//BOOL add_glyph_to_tree (UPFGLYPHTREE *root, int uc32);

//BOOL delete_glyph_from_tree (UPFGLYPHTREE** root, int uc32);

Uchar32 utf16le_conv_to_uc32 (const unsigned char* mchar);

void print_tree (UPFGLYPHTREE* tree);
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __UPF_H_ */

