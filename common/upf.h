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

#include "types.h"

#ifndef UPF_H
#define UPF_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct
{
    /* BBOX information of the glyph */
    Sint8 bearingx;
    Sint8 bearingy;
    Uint8 width;
    Uint8 height;
    /* The pitch of the bitmap (bytes of one scan line) */
    Uint8 pitch;
    /* Paddings for alignment */
    Uint8 padding1;
    /* advance value of the glyph */
    Uint8 advance;
    /* Paddings for alignment */
    Uint8 padding2;

    /* the offset of the glyph bitmap from the start of this font file. */
    Uint32 bitmap_offset;
} UPFGLYPH;

typedef struct
{
    Uint32 min;
    Uint32 max;
    Uint32 less_offset;
    Uint32 more_offset;
    Uint32 glyph_offset;
} UPFNODE;

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
} UPFINFO;

typedef struct 
{
    char     ver_info [LEN_VERSION_INFO];
    char     vender_name [LEN_VENDER_NAME];
    Uint16   endian;

    char     font_name [LEN_DEVFONT_NAME + 1];

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

#define UPTO_MUTI_FOUR(i) (((i)+3)>>2<<2)
#define ARCHSWAP32_SELF(i) ((i) = ArchSwap32(i))

typedef struct _UPFGLYPHTREE {
    unsigned int min, max;
    struct _UPFGLYPHTREE *less;
    struct _UPFGLYPHTREE *more;
    UPFGLYPH *glyph;
} UPFGLYPHTREE;

UPFGLYPHTREE* upf_make_glyph_tree (UPFGLYPH* glyphs);
void upf_delete_glyph_tree (UPFGLYPHTREE* root);

typedef int (*UPF_CB_TRAVEL_GLYPH_TREE) (void* ctxt, UPFGLYPHTREE *node); 
void upf_travel_glyph_tree (UPFGLYPHTREE *root, UPF_CB_TRAVEL_GLYPH_TREE cb, void* ctxt);

int upf_generate_upf_file (const char* oname, const UPFV1_FILE_HEADER* file_hdr,
                const UPFGLYPHTREE* root, const unsigned char* buff_bitmap);

UPFINFO* upf_load_font_data (const char* file_name);
void upf_unload_font_data (void* data);
UPFGLYPH* upf_get_glyph (Uint8 *upf_root, UPFNODE * tree, unsigned int ch);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* UPF_H */

