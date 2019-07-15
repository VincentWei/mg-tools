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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bdf.h"
#include "upf.h"

#define SET_WEIGHT_POS     0
#define SET_SLANT_POS      1
#define SET_SETWIDTH_POS   2

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

static void print_bitmap (FILE* file, const unsigned char* bits, int width, int height)
{
    int y = 0;
    int x = 0;
    const unsigned char* p_line_head;
    const unsigned char* p_cur_char;
    int pitch  = (width +7) >> 3;
    
    for (y = 0, p_line_head = bits; y < height; y++) {
        for (x = 0; x < width; x++) {
            p_cur_char = (x >> 3) + p_line_head;
            if (*p_cur_char & (128 >> (x%8)))
                fprintf (file, "@ ");
            else
                fprintf (file, ". ");
        }
        fprintf (file, "\n");
        
        p_line_head += pitch;
    }
}

#define LEN_PADD_BMP_BUFF   4

static UPFGLYPH g_upf_glyphs [0xffff];
static UPFV1_FILE_HEADER g_upf_file_hdr;
static unsigned char* g_buff_bitmap;
static unsigned int g_len_buff_bitmap;
static char g_font_rrncnn [7];
static BOOL g_verbose = FALSE;

/* 
 *  - return 0 when ok;
 *  - return other value when error occured.
 */
static int on_got_font_properties (BDF_INFO* info)
{
    if (strcasecmp (info->charset_registry, "ISO10646") != 0) {
        fprintf (stderr, "BDF2UPF: not supported charset registry: %s\n", info->charset_registry);
        return -1;
    }

    memset (g_upf_glyphs, 0, sizeof (g_upf_glyphs));
    memset (&g_upf_file_hdr, 0, sizeof (UPFV1_FILE_HEADER));

    g_len_buff_bitmap = LEN_PADD_BMP_BUFF;
    g_buff_bitmap = realloc (g_buff_bitmap, g_len_buff_bitmap);
    if (g_buff_bitmap == NULL)
        return -1;

    strcpy (g_upf_file_hdr.ver_info, "UPF1.0**");

    strncpy (g_upf_file_hdr.vender_name, info->foundry, LEN_VENDER_NAME - 1);
    g_upf_file_hdr.vender_name [LEN_VENDER_NAME - 1] = '\0';

    g_upf_file_hdr.endian = 0x1234;

    strcpy (g_font_rrncnn, "rrncnn");
    set_rrncnn (g_font_rrncnn, SET_WEIGHT_POS, info->weight_name);
    set_rrncnn (g_font_rrncnn, SET_SLANT_POS, info->slant);
    set_rrncnn (g_font_rrncnn, SET_SETWIDTH_POS, info->setwidth_name);
    sprintf (g_upf_file_hdr.font_name, "upf-%s-%s-%d-%d-%s", 
                info->family_name, g_font_rrncnn, 
                info->fnt_bbox_w, info->pixel_size, 
                "UTF-8,UTF-16LE,UTF-16BE");

    g_upf_file_hdr.width = info->fnt_bbox_w;
    g_upf_file_hdr.height = info->pixel_size;
    g_upf_file_hdr.ascent = info->font_ascent;
    g_upf_file_hdr.descent = info->font_descent;

    g_upf_file_hdr.max_width = 0; /* calcaute in on_got_one_glyph */
    g_upf_file_hdr.min_width = 128; /* calcaute in on_got_one_glyph */
    g_upf_file_hdr.left_bearing = 0;
    g_upf_file_hdr.right_bearing = 0;
    g_upf_file_hdr.underline_pos = 0;
    g_upf_file_hdr.underline_width = 0;
    g_upf_file_hdr.leading = 0;
    g_upf_file_hdr.mono_bitmap = 1;

    g_upf_file_hdr.nr_glyph = 0; /* calcaute in on_got_one_glyph */
    g_upf_file_hdr.font_size = 0;

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
    if (glyph_info->encoding < 0xFFFF) {
        int pitch, size_bitmap, offset_bitmap;

        if (g_verbose) {
            printf ("BDF2UPF: A new glyph: 0x%X:\n", glyph_info->encoding);
            print_bitmap (stdout, bitmap, glyph_info->bbox_w, glyph_info->bbox_h);
        }

        if (g_upf_glyphs [glyph_info->encoding].bitmap_offset) {
            fprintf (stderr, "BDF2UPF: A duplicated glyph: 0x%X:\n", glyph_info->encoding);
            return 1;
        }

        g_upf_glyphs [glyph_info->encoding].bearingx = glyph_info->bbox_x;
        g_upf_glyphs [glyph_info->encoding].bearingy = glyph_info->bbox_y + glyph_info->bbox_h;
        g_upf_glyphs [glyph_info->encoding].width = glyph_info->bbox_w;
        g_upf_glyphs [glyph_info->encoding].height = glyph_info->bbox_h;

        pitch  = (glyph_info->bbox_w + 7) >> 3;
        size_bitmap = pitch * glyph_info->bbox_h;
        g_upf_glyphs [glyph_info->encoding].pitch = pitch;

        g_upf_glyphs [glyph_info->encoding].padding1 = 0;
        g_upf_glyphs [glyph_info->encoding].advance = glyph_info->adv_x;
        g_upf_glyphs [glyph_info->encoding].padding2 = 0;

        g_buff_bitmap = realloc (g_buff_bitmap, g_len_buff_bitmap + size_bitmap);
        if (g_buff_bitmap == NULL)
            return -1;

        offset_bitmap = g_len_buff_bitmap;
        g_len_buff_bitmap += size_bitmap;
        memcpy (g_buff_bitmap + offset_bitmap, bitmap, size_bitmap);
        g_upf_glyphs [glyph_info->encoding].bitmap_offset = offset_bitmap;

        if (g_upf_file_hdr.max_width < glyph_info->adv_x)
            g_upf_file_hdr.max_width = glyph_info->adv_x;
        if (g_upf_file_hdr.min_width > glyph_info->adv_x)
            g_upf_file_hdr.min_width = glyph_info->adv_x;
        g_upf_file_hdr.nr_glyph ++;
    }
    else {
        fprintf (stderr, "BDF2UPF: A bad glyph: 0x%X:\n", glyph_info->encoding);
        return 1;
    }

    return 0;
}

static int count_glyph_tree_nodes (void* ctxt, UPFGLYPHTREE *node)
{
    unsigned int code;
    int* nr_nodes = (int*) ctxt;

    if (nr_nodes)
        (*nr_nodes)++;

    for (code = node->min; code <= node->max; code++) {
        g_upf_glyphs [code].bitmap_offset -= LEN_PADD_BMP_BUFF;
    }

    return 0;
}

int bdf2upf (char* file_name)
{
    char oname [256];
    BDF_INFO bdf_info;
    UPFGLYPHTREE* root;

    memset (&bdf_info, 0, sizeof (BDF_INFO));
    bdf_info.nr_glyphs = -1;
    bdf_info.on_got_font_properties = on_got_font_properties;
    bdf_info.on_got_one_glyph = on_got_one_glyph;

    if (bdf_parse_file (file_name, &bdf_info)) {
        fprintf (stderr, "BDF2UPF: error occured when parsing BDF file: %s.\n", file_name);
        return -1;
    }

    if ((root = upf_make_glyph_tree (g_upf_glyphs)) == NULL) {
        fprintf (stderr, "BDF2UPF: error occured when making glyph tree: %s.\n", file_name);
        return -1;
    }
    
    sprintf (oname, "%s-%s-%s-%d.upf", bdf_info.foundry, bdf_info.family_name, 
            g_font_rrncnn, bdf_info.pixel_size);

    /* other important fields */
    g_upf_file_hdr.nr_zones = 0;
    upf_travel_glyph_tree (root, count_glyph_tree_nodes, &g_upf_file_hdr.nr_zones);

    g_upf_file_hdr.len_nodes = sizeof (UPFNODE) * g_upf_file_hdr.nr_zones;
    g_upf_file_hdr.len_glyphs = sizeof (UPFGLYPH) * g_upf_file_hdr.nr_glyph;
    g_upf_file_hdr.len_bitmaps = g_len_buff_bitmap - LEN_PADD_BMP_BUFF;

    g_upf_file_hdr.off_nodes = sizeof (UPFV1_FILE_HEADER);
    g_upf_file_hdr.off_nodes = UPTO_MUTI_FOUR (g_upf_file_hdr.off_nodes);

    g_upf_file_hdr.off_glyphs = g_upf_file_hdr.off_nodes + g_upf_file_hdr.len_nodes;
    g_upf_file_hdr.off_glyphs = UPTO_MUTI_FOUR (g_upf_file_hdr.off_glyphs);

    g_upf_file_hdr.off_bitmaps = g_upf_file_hdr.off_glyphs + g_upf_file_hdr.len_glyphs;
    g_upf_file_hdr.off_bitmaps = UPTO_MUTI_FOUR (g_upf_file_hdr.off_bitmaps);

    g_upf_file_hdr.font_size = g_upf_file_hdr.off_bitmaps + g_upf_file_hdr.len_bitmaps;

    if (g_verbose) {
        printf ("BDF2UPF: the number of glyphs: %d\n", g_upf_file_hdr.nr_glyph);
        printf ("BDF2UPF: the number of tree nodes: %d\n", g_upf_file_hdr.nr_zones);
    }

    if (upf_generate_upf_file (oname, &g_upf_file_hdr, root, g_buff_bitmap + LEN_PADD_BMP_BUFF)) {
        fprintf (stderr, "BDF2UPF: error occured when generating UPF file: %s->%s.\n", 
                file_name, oname);
        upf_delete_glyph_tree (root);
        return -1;
    }
    
    upf_delete_glyph_tree (root);

    return 0;
}

int main (int argc, char* argv[])
{
    int i;

    if (getenv ("BDF2UPF_VERBOSE"))
        g_verbose = TRUE;

    for (i = 1; i < argc; i++)
        bdf2upf (argv[i]);

    free (g_buff_bitmap);
    g_buff_bitmap = NULL;

    return 0;
}
