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

#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "upf.h"

//#define VAL_DEBUG
#include "my_debug.h"
/*sum size of bmp data of all the tree*/
static int cal_glyph_changed_size(UPFGLYPHTREE *glyph_tree)
{
    int i;
    int n = glyph_tree->upf_node.max - glyph_tree->upf_node.min + 1;
	unsigned int size = 0;

	size += n * sizeof(GLYPHMETRICS);
    for (i = 0; i < n; i++) {
        size += glyph_tree->glyph[i].metrics->upf_glyph.pitch * 
                glyph_tree->glyph[i].metrics->upf_glyph.height;
    }

    if (glyph_tree->less)
        size += cal_glyph_changed_size(glyph_tree->less);
    if (glyph_tree->more)
        size += cal_glyph_changed_size(glyph_tree->more);

	return size;
}

/*collect bmp data of all the tree nodes to *ptr */
static void glyph_copy(unsigned char** ptr, UPFGLYPHTREE* glyph_tree)
{
	int i = 0;
	int n = glyph_tree->upf_node.max - glyph_tree->upf_node.min + 1;

	for (i = 0; i < n; i++) {
		int data_size;

		data_size = glyph_tree->glyph[i].metrics->upf_glyph.pitch
			        * glyph_tree->glyph[i].metrics->upf_glyph.height;
		
		memcpy(*ptr, glyph_tree->glyph[i].data, data_size); 
		(*ptr) += data_size;
	}

	if (glyph_tree->less)
		glyph_copy(ptr, glyph_tree->less);
	if (glyph_tree->more)
		glyph_copy(ptr, glyph_tree->more);
}

/*collect metrics data of all the tree nodes to *ptr */
static void glyph_metrics_copy(unsigned char** ptr, UPFGLYPHTREE* glyph_tree)
{
	int i = 0;
	int n = glyph_tree->upf_node.max - glyph_tree->upf_node.min + 1;

	for (i = 0; i < n; i++) {
		memcpy(*ptr, glyph_tree->glyph[i].metrics, sizeof(GLYPHMETRICS)); 
		(*ptr) += sizeof(GLYPHMETRICS);
	}

	if (glyph_tree->less)
		glyph_metrics_copy(ptr, glyph_tree->less);
	if (glyph_tree->more)
		glyph_metrics_copy(ptr, glyph_tree->more);
}

/*collect all nodes[min-max pair] of the tree to *ptr 
 * only (min, max), not (min_offset, max_offset, glyph_offset)*/
static void glyph_nodes_copy(unsigned char **ptr, UPFGLYPHTREE* glyph_tree)
{
	/***ptr = glyph_tree->min_rw; (*ptr)++;
	**ptr = glyph_tree->min_cl; (*ptr)++;
	**ptr = glyph_tree->max_rw; (*ptr)++;
	**ptr = glyph_tree->max_cl; (*ptr)++;
	**ptr = glyph_tree->flags;  (*ptr)++;*/
    *((int *)*ptr) =glyph_tree->upf_node.min; (*ptr) +=4;
    *((int *)*ptr) =glyph_tree->upf_node.max; (*ptr) +=4;

    /*FIXME where are 3 offset ? */

	if (glyph_tree->less)
		glyph_nodes_copy(ptr, glyph_tree->less);
	if (glyph_tree->more)
		glyph_nodes_copy(ptr, glyph_tree->more);
}


/*write bmp datas to file (low address is root)*/
static void write_data (UPFGLYPHTREE* tree, FILE* fp)
{
    int i;
    int n = tree->upf_node.max - tree->upf_node.min + 1;

    for (i = 0; i < n; i++) {
        int data_size;
        data_size = tree->glyph[i].metrics->upf_glyph.pitch * 
                tree->glyph[i].metrics->upf_glyph.height;
		fwrite (tree->glyph[i].data, data_size, 1, fp);
    }

    if (tree->less)
        write_data (tree->less, fp);
    if (tree->more)
        write_data (tree->more, fp);
}


/*write all bmp metrics of trees to file (low address is root)*/
static void write_metrics (UPFGLYPHTREE* tree, FILE* fp)
{
    int i;
    int n = tree->upf_node.max - tree->upf_node.min + 1;

    for (i = 0; i < n; i++) {
		fwrite (&tree->glyph[i].metrics->upf_glyph, sizeof(UPFGLYPH), 1, fp);
    }

    /*FIXME if the bitmap_offset is changed ? */

    if (tree->less)
        write_metrics (tree->less, fp);
    if (tree->more)
        write_metrics (tree->more, fp);
}

static void write_node (UPFGLYPHTREE* tree, FILE* fp)
{
    /*int n;

    memcpy(&tree->upf_node, (*data), sizeof(UPFNODE));
    data += sizeof(UPFNODE);*/

	fwrite (&tree->upf_node, sizeof(UPFNODE), 1, fp);
    /*if (tree->upf_node.less_offset) {
        tree->less = calloc (1, sizeof (UPFGLYPHTREE));
		tree->less->parent = tree;
	} else {
        tree->less = NULL;
	}

    if (tree->upf_node.more_offset) {
        tree->more = calloc (1, sizeof (UPFGLYPHTREE));
		tree->more->parent = tree;
	} else {
        tree->more = NULL;
	}

    n = tree->upf_node.max - tree->upf_node.min + 1;
    tree->glyph = calloc (n, sizeof (GLYPH));*/

    if (tree->less)
        write_node (tree->less, fp);
    if (tree->more)
        write_node (tree->more, fp);
}

static void calc_node_offset_num (UPFGLYPHTREE* root, int* cur_offset, int* num)
{
    if (!root)
        return;

    *cur_offset += sizeof (UPFNODE);
    (*num)++;

    if (root->less) {
        root->upf_node.less_offset = *cur_offset;
        calc_node_offset_num (root->less, cur_offset, num);
    }
    else {
        root->upf_node.less_offset = 0;
    }

    if (root->more) {
        root->upf_node.more_offset = *cur_offset;
        calc_node_offset_num (root->more, cur_offset, num);
    }
    else {
        root->upf_node.more_offset = 0;
    }
}

static void calc_glyph_offset_num (UPFGLYPHTREE* root, int* cur_offset, int* num)
{
    int cur_glyph_num = 0;

    if (!root) 
        return;

    root->upf_node.glyph_offset = *cur_offset;

    cur_glyph_num = root->upf_node.max - root->upf_node.min + 1;
    *num += cur_glyph_num;

    *cur_offset += cur_glyph_num * sizeof (UPFGLYPH);

    if (root->less) 
        calc_glyph_offset_num (root->less, cur_offset, num);

    if (root->more) 
        calc_glyph_offset_num (root->more, cur_offset, num);
}

/* param: cur_offset, current offset of root 
 * return glyph num of cur tree*/
static void calc_bitmap_offset (UPFGLYPHTREE* root, int* cur_offset)
{
    int cur_glyph_num = 0;
    int i;

    if (!root) 
        return;

    cur_glyph_num = root->upf_node.max - root->upf_node.min + 1;

    for (i=0; i<cur_glyph_num; i++)
    {
        root->glyph[i].metrics->upf_glyph.bitmap_offset = *cur_offset;
        *cur_offset += root->glyph[i].metrics->upf_glyph.pitch 
                * root->glyph[i].metrics->upf_glyph.height;
    }

    if (root->less)
        calc_bitmap_offset (root->less, cur_offset);
    if (root->more)
        calc_bitmap_offset (root->more, cur_offset);
}

/*write all the tree data to file*/
BOOL dump_upf (UPFINFO* upf_info, const char* file)
{
	FILE* fp;
    int offset = 0;
    int num;
    UPFV1_FILE_HEADER* upf_file_header;

    UPFGLYPHTREE* min_offspring;
    UPFGLYPHTREE* max_offspring;
    int l_num;
    int m_num;

    upf_file_header = &(((UPFINFO*)upf_info)->upf_file_header);

	if (upf_info == NULL || file == NULL || upf_info->upf_tree == NULL) 
		return FALSE;

	if ((fp = fopen (file, "w+")) == NULL)
		return FALSE;

    compress (upf_info->upf_tree, &min_offspring, &max_offspring);
    balance (&(upf_info->upf_tree), &l_num, &m_num);

    //print_tree(upf_info->upf_tree);
    num = 0;
    offset = sizeof(UPFV1_FILE_HEADER);

    upf_file_header->off_nodes = offset;
    calc_node_offset_num (upf_info->upf_tree, &offset, &num);
    upf_file_header->nr_zones = num;
    upf_file_header->len_nodes = offset - upf_file_header->off_nodes;

    num = 0;
    upf_file_header->off_glyphs = offset;

    calc_glyph_offset_num (upf_info->upf_tree, &offset, &num);
    upf_file_header->nr_glyph = num;
    upf_file_header->len_glyphs = offset - upf_file_header->off_glyphs;

    upf_file_header->off_bitmaps = offset;
    calc_bitmap_offset (upf_info->upf_tree, &offset);
    upf_file_header->len_bitmaps = offset - upf_file_header->off_bitmaps;
    upf_file_header->font_size = offset;

    TEST_VAL (upf_info->upf_file_header.off_nodes, %d);
    TEST_VAL (upf_info->upf_file_header.len_nodes, %d);
    TEST_VAL (upf_info->upf_file_header.nr_zones, %d);

    TEST_VAL (upf_info->upf_file_header.off_glyphs, %d);
    TEST_VAL (upf_info->upf_file_header.len_glyphs, %d);
    TEST_VAL (upf_info->upf_file_header.nr_glyph, %d);

    
    TEST_VAL (upf_info->upf_file_header.off_bitmaps, %d);
    TEST_VAL (upf_info->upf_file_header.len_bitmaps, %d);

    TEST_VAL (upf_info->upf_file_header.font_size, %d);

    fwrite (&(upf_info->upf_file_header), 1, sizeof(UPFV1_FILE_HEADER), fp);
    write_node(upf_info->upf_tree,fp);
    write_metrics(upf_info->upf_tree,fp);
    write_data(upf_info->upf_tree,fp);

	fclose (fp);
	return TRUE;
}
