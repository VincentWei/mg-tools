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
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>

#include "../common/bdf.h"

#define SET_WEIGHT_POS     0
#define SET_SLANT_POS      1
#define SET_SETWIDTH_POS   2

char g_font_rrncnn [7];
FILE *g_out_fp = NULL;
iconv_t g_cd;

#define IS_GB2312_CHAR(ch1, ch2) \
        if (((ch1 >= 0xA1 && ch1 <= 0xA9) || (ch1 >= 0xB0 && ch1 <= 0xF7)) \
                        && ch2 >= 0xA1 && ch2 <= 0xFE)

static int gb_index (unsigned short encoding)
{
    unsigned char ch1, ch2;
    int area;

    ch1 = encoding >> 8;
    ch2 = (unsigned char)encoding;

    area = ch1 - 0xA1;
    IS_GB2312_CHAR(ch1, ch2) {
        if (area < 9) {
            return (area * 94 + ch2 - 0xA1);
        }
        else if (area >= 15)
            return ((area - 6)* 94 + ch2 - 0xA1);
    }

    return -1;
}

static void set_rrncnn (char* rrncnn, int set_pos, char* value)
{
    switch (set_pos)
    {
        case SET_WEIGHT_POS:
            if (strcasecmp(value, "black") == 0)
                rrncnn[SET_WEIGHT_POS] = 'c';
            else if (strcasecmp(value, "bold") == 0)
                rrncnn[SET_WEIGHT_POS] = 'b';
            else if (strcasecmp(value, "book") == 0)
                rrncnn[SET_WEIGHT_POS] = 'k';
            else if (strcasecmp(value, "black") == 0)
                rrncnn[SET_WEIGHT_POS] = 'c';
            else if (strcasecmp(value, "demibold") == 0)
                rrncnn[SET_WEIGHT_POS] = 'd';
            else if (strcasecmp(value, "light") == 0)
                rrncnn[SET_WEIGHT_POS] = 'l';
            else if (strcasecmp(value, "medium") == 0)
                rrncnn[SET_WEIGHT_POS] = 'm';
            else if (strcasecmp(value, "regular") == 0)
                rrncnn[SET_WEIGHT_POS] = 'r';
            break;

        case SET_SLANT_POS:
            if (strcasecmp(value, "i") == 0)
                rrncnn[SET_SLANT_POS] = 'i';
            else if (strcasecmp(value, "o") == 0)
                rrncnn[SET_SLANT_POS] = 'o';
            else if (strcasecmp(value, "r") == 0)
                rrncnn[SET_SLANT_POS] = 'r';
            break;

        case SET_SETWIDTH_POS:
            if (strcasecmp(value, "bold") == 0)
                rrncnn[SET_SETWIDTH_POS] = 'b';
            else if (strcasecmp(value, "condensed") == 0)
                rrncnn[SET_SETWIDTH_POS] = 'c';
            else if (strcasecmp(value, "semicondensed") == 0)
                rrncnn[SET_SETWIDTH_POS] = 's';
            else if (strcasecmp(value, "normal") == 0)
                rrncnn[SET_SETWIDTH_POS] = 'n';
            break;
    }
}

/* 
 *  - return 0 when ok;
 *  - return other value when error occured.
 */
static int on_got_font_properties (BDF_INFO* info)
{
    char oname [256];
    int width;
    int height;
    int bmp_pitch;
    int bmp_size;

    strcpy (g_font_rrncnn, "rrncnn");
    set_rrncnn (g_font_rrncnn, SET_WEIGHT_POS, info->weight_name);
    set_rrncnn (g_font_rrncnn, SET_SLANT_POS, info->slant);
    set_rrncnn (g_font_rrncnn, SET_SETWIDTH_POS, info->setwidth_name);

    width = info->pixel_size;
    height = info->font_ascent + info->font_descent;
    sprintf (oname, "rbf-%s_%s-%s-%dx%d-gb2312.rbf", info->foundry, info->family_name, 
            g_font_rrncnn, width, height);

    g_out_fp = fopen(oname, "w");

    bmp_pitch = (width + 7)/8;
    bmp_size = bmp_pitch * height;

    fseek (g_out_fp, 94 * 81 * bmp_size - 1, SEEK_SET); /* gb2312 */
    // fseek (g_out_fp, 256 * bmp_size - 1, SEEK_SET); /* 8859-1 */
    fwrite (" ", 1, 1, g_out_fp);

    printf("oname=%s\n", oname);
    printf("width=%d\n", width);
    printf("height=%d\n", height);
    printf("foundry=%s\n", info->foundry);
    printf("family_name=%s\n", info->family_name);
    printf("weight_name=%s\n", info->weight_name);
    printf("slant=%s\n", info->slant);
    printf("setwidth_name=%s\n", info->setwidth_name);
    printf("charset_registry=%s\n", info->charset_registry);
    printf("charset_encoding=%s\n", info->charset_encoding);
    printf("face_name=%s\n", info->face_name);
    printf("nr_glyphs=%d\n", info->nr_glyphs);

    g_cd = iconv_open("GB2312//", "ISO-10646/UCS2/");
    assert(g_cd != (iconv_t)-1);
    return 0;
}

/* 
 *  - return 0 when ok;
 *  - return >0 when igore the glyph;
 *  - return <0 when error occured.
 */
static int on_got_one_glyph (BDF_INFO* info,
                const BDF_GLYPH_INFO* glyph_info, const unsigned char* bitmap)
{
    int width;
    int height;
    int bmp_pitch;
    int bmp_size;

    int j, k, skip_lines, skip_bytes;
    unsigned int bb_lines [128];
    int bb_pitch;
    int index;
    unsigned char rbf_bitmap[1024*4];
    int bmp_pos;

    width = info->pixel_size;
    height = info->font_ascent + info->font_descent;
    bmp_pitch = (width + 7)/8;
    bmp_size = bmp_pitch * height;

    bb_pitch = (glyph_info->bbox_w + 7)/8;

    for (j = 0; j < glyph_info->bbox_h; j++) {
        bb_lines [j] = 0;
        for (k = 0; k < bb_pitch; k++)
            bb_lines [j] |= bitmap [j*bb_pitch + k] << ((bb_pitch - k - 1) * 8);

        bb_lines [j] <<= (bmp_pitch - bb_pitch) * 8;
        bb_lines [j] >>= glyph_info->bbox_x;
    }

    skip_lines = height - (glyph_info->bbox_h + glyph_info->bbox_y) - info->font_descent;
    skip_bytes = skip_lines * bmp_pitch;

    memset (rbf_bitmap, 0, 72);
    for (j = 0; j < glyph_info->bbox_h; j++) {
        unsigned char* bmp = &rbf_bitmap [skip_bytes + j*bmp_pitch];
        for (k = 0; k < bmp_pitch; k++)
            bmp [k] = (unsigned char)(bb_lines [j] >> ((bmp_pitch - k - 1) * 8));
    }

    {
        unsigned char inbuf[2];
        unsigned char outbuf[2];
        size_t in_len = sizeof(inbuf);
        size_t out_len = sizeof(outbuf);
        char *pin=(char *)inbuf;
        char *pout=(char *)outbuf;
        size_t ret;

        memcpy(inbuf, &glyph_info->encoding, sizeof(inbuf));
        /*
        if (glyph_info->encoding == 21834) {
            printf("inbuf=%02x %02x\n", (unsigned char)inbuf[0], (unsigned char)inbuf[1]);
            int w, h;
            for (h=0; h<13; ++h) {
                for (w=0; w<2; ++w) {
                    union {
                        struct {
                            unsigned char b0:1;
                            unsigned char b1:1;
                            unsigned char b2:1;
                            unsigned char b3:1;
                            unsigned char b4:1;
                            unsigned char b5:1;
                            unsigned char b6:1;
                            unsigned char b7:1;
                        } bits;
                        unsigned char B;
                    } a;
                    a.B = rbf_bitmap[h*2 + w];
                    printf("%c %c %c %c %c %c %c %c ",
                            a.bits.b7 ? '*' : ' ',
                            a.bits.b6 ? '*' : ' ',
                            a.bits.b5 ? '*' : ' ',
                            a.bits.b4 ? '*' : ' ',
                            a.bits.b3 ? '*' : ' ',
                            a.bits.b2 ? '*' : ' ',
                            a.bits.b1 ? '*' : ' ',
                            a.bits.b0 ? '*' : ' ');
                }
                printf("\n");
            }
        }
        */
        *((short *)outbuf) = 0;
        ret = iconv(g_cd, &pin, &in_len, &pout, &out_len);
        if (ret == 0) {
            index = 0;
            *(((unsigned char *)&index)) = outbuf[1];
            *(((unsigned char *)&index)+1) = outbuf[0];

            index = gb_index (index);
            if (index >= 0 && index < 7614) {
                bmp_pos = bmp_size * index;
                fseek (g_out_fp, bmp_pos, SEEK_SET);
                fwrite (rbf_bitmap, bmp_size, 1, g_out_fp);
            }
        }
    }

    return 0;
}

int bdf2rbf (char* bd2_file_name)
{
    BDF_INFO bdf_info;

    memset (&bdf_info, 0, sizeof (BDF_INFO));
    bdf_info.nr_glyphs = -1;
    bdf_info.on_got_font_properties = on_got_font_properties;
    bdf_info.on_got_one_glyph = on_got_one_glyph;

    if (bdf_parse_file (bd2_file_name, &bdf_info)) {
        fprintf (stderr, "BDF2UPF: error occured when parsing BDF file: %s.\n", bd2_file_name);
        return -1;
    }

    fclose(g_out_fp);
    iconv_close(g_cd);
    return 0;
}

int main (int argc, char* argv[])
{
    int i;

    for (i = 1; i < argc; i++)
        bdf2rbf (argv[i]);

    return 0;
}
