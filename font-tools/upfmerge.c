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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "upf.h"

void help (void)
{
    printf ("SYNOPSIS:\n");
    printf ("\tupfmerge <dst_upf> <src_upf> <new_upf>\n");
    printf ("\tMerge the glpyhs in UPF <src_upf> and <dst_upf> and create a new UPF file.\n");
    printf ("NOTE:\n");
    printf ("\tMerge only when the source UPF size is same as the destination UPF.\n");
    printf ("EXAMPLE:\n");
    printf ("\t./upfmerge fmsong-gbk-12.upf Adobe-Helvetica-12.upf fmsong-latin-gbk-12.upf\n");
}

#define LEN_PADD_BMP_BUFF   4
#define MAX_UCS             0xFFFF

static UPFGLYPH g_upf_glyphs [MAX_UCS];
static UPFV1_FILE_HEADER g_upf_file_hdr;
static unsigned char* g_buff_bitmap;
static unsigned int g_len_buff_bitmap;

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

static BOOL g_adjust_ascent;

UPFGLYPHTREE* MergeUPFs (UPFINFO *dst_upf, UPFINFO *src_upf)
{
    unsigned int code, nr_merged_glyphs;
    Uint8* p_upf;
    UPFV1_FILE_HEADER* hdr;
    UPFGLYPHTREE* root = NULL;
    UPFGLYPH* glyph;
    unsigned int size_bitmap, offset_bitmap;
    double total_width = 0.0f;

    memset (g_upf_glyphs, 0, sizeof (g_upf_glyphs));
    memcpy (&g_upf_file_hdr, dst_upf->root_dir, sizeof (UPFV1_FILE_HEADER));

    g_len_buff_bitmap = LEN_PADD_BMP_BUFF;
    g_buff_bitmap = realloc (g_buff_bitmap, g_len_buff_bitmap);
    if (g_buff_bitmap == NULL)
        return NULL;

    /* calculate later */
    g_upf_file_hdr.width = 0;
    g_upf_file_hdr.max_width = 0;
    g_upf_file_hdr.min_width = 128;
    g_upf_file_hdr.nr_glyph = 0;
    g_upf_file_hdr.font_size = 0;

    if (g_adjust_ascent) {
        if (g_upf_file_hdr.ascent < src_upf->ascent)
            g_upf_file_hdr.ascent = src_upf->ascent;
        if (g_upf_file_hdr.descent < src_upf->descent)
            g_upf_file_hdr.descent = src_upf->descent;
    }

    /* use values of destination UPF 
    g_upf_file_hdr.height = info->pixel_size;
    g_upf_file_hdr.ascent = info->font_ascent;
    g_upf_file_hdr.descent = info->font_descent;
    g_upf_file_hdr.mono_bitmap = 0;

    g_upf_file_hdr.left_bearing = 0;
    g_upf_file_hdr.right_bearing = 0;
    g_upf_file_hdr.underline_pos = 0;
    g_upf_file_hdr.underline_width = 0;
    g_upf_file_hdr.leading = 0;
    */

    /* copy glyps from destination UPF */
    p_upf = (Uint8 *)dst_upf->root_dir;
    hdr = (UPFV1_FILE_HEADER*)dst_upf->root_dir;
    for (code = 0; code < MAX_UCS; code++) {
    
        glyph = upf_get_glyph (p_upf, (UPFNODE *)(p_upf + hdr->off_nodes), code);
        if (glyph) {
            g_upf_glyphs [code] = *glyph;

            size_bitmap = glyph->pitch * glyph->height;
            g_buff_bitmap = realloc (g_buff_bitmap, g_len_buff_bitmap + size_bitmap);
            if (g_buff_bitmap == NULL)
                return NULL;

            offset_bitmap = g_len_buff_bitmap;
            g_len_buff_bitmap += size_bitmap;
            memcpy (g_buff_bitmap + offset_bitmap, p_upf + glyph->bitmap_offset, size_bitmap);
            g_upf_glyphs [code].bitmap_offset = offset_bitmap;

            if (g_upf_file_hdr.max_width < glyph->advance)
                g_upf_file_hdr.max_width = glyph->advance;
            if (g_upf_file_hdr.min_width > glyph->advance)
                g_upf_file_hdr.min_width = glyph->advance;
            total_width += glyph->advance;
            g_upf_file_hdr.nr_glyph ++;
        }
    }

    /* merge glyps from source UPF */
    nr_merged_glyphs = 0;
    p_upf = (Uint8 *)src_upf->root_dir;
    hdr = (UPFV1_FILE_HEADER*)src_upf->root_dir;
    for (code = 0; code < MAX_UCS; code++) {
    
        glyph = upf_get_glyph (p_upf, (UPFNODE *)(p_upf + hdr->off_nodes), code);
        if (glyph) {
            if (g_upf_glyphs [code].bitmap_offset) {
                fprintf (stderr, "UPFMerge: duplicated glyph: %X.\n", code);
                continue;
            }

            g_upf_glyphs [code] = *glyph;

            size_bitmap = glyph->pitch * glyph->height;
            g_buff_bitmap = realloc (g_buff_bitmap, g_len_buff_bitmap + size_bitmap);
            if (g_buff_bitmap == NULL)
                return NULL;

            offset_bitmap = g_len_buff_bitmap;
            g_len_buff_bitmap += size_bitmap;
            memcpy (g_buff_bitmap + offset_bitmap, p_upf + glyph->bitmap_offset, size_bitmap);
            g_upf_glyphs [code].bitmap_offset = offset_bitmap;

            if (g_upf_file_hdr.max_width < glyph->advance)
                g_upf_file_hdr.max_width = glyph->advance;
            if (g_upf_file_hdr.min_width > glyph->advance)
                g_upf_file_hdr.min_width = glyph->advance;
            total_width += glyph->advance;
            g_upf_file_hdr.nr_glyph ++;

            nr_merged_glyphs++;
        }
    }

    if ((root = upf_make_glyph_tree (g_upf_glyphs)) == NULL) {
        fprintf (stderr, "UPFMerge: error occured when making glyph tree.\n");
        return NULL;
    }

    /* other important fields */
    g_upf_file_hdr.width = (int)(total_width/g_upf_file_hdr.nr_glyph + 0.5f);
    printf ("the width: %d=%f/%d\n", g_upf_file_hdr.width, total_width, g_upf_file_hdr.nr_glyph);

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

    printf ("UPFMerge: merge from %s to %s\n", src_upf->font_name, dst_upf->font_name);
    printf ("UPFMerge: the number of merged glyphs: %d\n", nr_merged_glyphs);
    printf ("UPFMerge: the number of glyphs after merging: %d\n", g_upf_file_hdr.nr_glyph);
    printf ("UPFMerge: the number of tree nodes after merging: %d\n", g_upf_file_hdr.nr_zones);

    return root;
}

/******************************main*************************************/
int main (int args, char *arg[])
{
    int i;
    UPFINFO *dst_upf = NULL, *src_upf = NULL;
    UPFV1_FILE_HEADER *dst_upf_hdr, *src_upf_hdr;
    UPFGLYPHTREE* root = NULL;

    if (args < 4) {
        help ();
        return 1;
    }

    /*********** check args *******************/
    for (i = 1; i < 4; i++) {
        const char* s = strrchr ((const char *)arg[1], '.');
        if (s == NULL || strcmp (s,".upf")) {
            help ();
            return 1;
        }
    }

    if (getenv ("UPFMERGE_ADJUST_ASCENT"))
        g_adjust_ascent = TRUE;

    dst_upf = upf_load_font_data (arg[1]);
    src_upf = upf_load_font_data (arg[2]);

    if (dst_upf == NULL) {
        fprintf (stderr, "UPFMerge: can not load UPF file: %s.\n", arg[1]);
        goto error_process;
    }

    if (src_upf == NULL) {
        fprintf (stderr, "UPFMerge: can not load UPF file: %s.\n", arg[2]);
        goto error_process;
    }

    dst_upf_hdr = (UPFV1_FILE_HEADER *)dst_upf->root_dir;
    src_upf_hdr = (UPFV1_FILE_HEADER *)src_upf->root_dir;

    if (strcmp (dst_upf_hdr->ver_info, src_upf_hdr->ver_info)) {
        fprintf (stderr, "UPFMerge: file %s does not match file %s in ver_info filed, "
                "they can not be merged.\n", arg[1], arg[2]);
        goto error_process;
    }

    if (dst_upf_hdr->endian != src_upf_hdr->endian) {
        fprintf (stderr, "UPFMerge: file %s does not match file %s in endian filed, "
                "they can not be merged.\n", arg[1], arg[2]);
        goto error_process;
    }

    if (dst_upf_hdr->mono_bitmap != src_upf_hdr->mono_bitmap) {
        fprintf (stderr, "UPFMerge: file %s does not match file %s in mono_bitmap filed, "
                "they can not be merged.\n", arg[1], arg[2]);
        goto error_process;
    }

    if (!getenv ("UPFMERGE_IGNORE_HEIGHT")) {
        if (dst_upf_hdr->height != src_upf_hdr->height) {
            fprintf (stderr, "UPFMerge: file %s does not match file %s in height filed, "
                    "they can not be merged.\n", arg[1], arg[2]);
            goto error_process;
        }
    }

    if ((root = MergeUPFs (dst_upf, src_upf)) == NULL) {
        fprintf (stderr, "UPFMerge: merge failed from %s to %s", arg[2], arg[1]);
        goto error_process;
    }

    if (upf_generate_upf_file (arg[3], &g_upf_file_hdr, root, g_buff_bitmap + LEN_PADD_BMP_BUFF)) {
        fprintf (stderr, "UPFMerge: error occured when generating UPF file: %s.\n", 
                arg[3]);
        goto error_process;
    }
    
    free (g_buff_bitmap);
    upf_delete_glyph_tree (root);
    upf_unload_font_data (dst_upf);
    upf_unload_font_data (src_upf);
    return 0;

error_process:
    free (g_buff_bitmap);
    upf_delete_glyph_tree (root);
    upf_unload_font_data (dst_upf);
    upf_unload_font_data (src_upf);
    return 2;
}

#include "load_upf.c"
#include "gen_upf.c"

