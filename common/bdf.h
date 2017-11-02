/*
** Copyright (C) 2009 Feynman Software
** Author: WEI Yongming
** Create date: 2009/12/19
*/

#include "types.h"

#ifndef BDF_H
#define BDF_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct _BDF_GLYPH_INFO
{
    unsigned int encoding;

    /* advance */
    int adv_x, adv_y;

    /* BBOX information of the glyph */
    int bbox_x, bbox_y, bbox_w, bbox_h;
    
} BDF_GLYPH_INFO;

#define LEN_FONT_NAME           31
#define LEN_FACE_NAME           31

typedef struct _BDF_INFO
{
    char    foundry [LEN_FONT_NAME + 1];
    char    family_name [LEN_FONT_NAME + 1];
    char    weight_name [LEN_FONT_NAME + 1];
    char    slant [LEN_FONT_NAME + 1];
    char    setwidth_name [LEN_FONT_NAME + 1];
    char    charset_registry [LEN_FONT_NAME + 1];
    char    charset_encoding [LEN_FONT_NAME + 1];
    char    face_name [LEN_FACE_NAME + 1];
    int     pixel_size;
    int     point_size;
    int     font_ascent;
    int     font_descent;
    int     default_char;

    int     fnt_bbox_w, fnt_bbox_h, fnt_bbox_x, fnt_bbox_y;

    int     nr_glyphs;

    /* callbacks */
    int     (*on_got_font_properties) (struct _BDF_INFO* info);
    int     (*on_got_one_glyph) (struct _BDF_INFO* info, 
                const BDF_GLYPH_INFO* glyph_info, const unsigned char* bitmap);

    void* context;
} BDF_INFO;

int bdf_parse_file (const char* file_name, BDF_INFO* info);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* BDF_H */

