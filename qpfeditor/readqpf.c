/*
** $Id: readqpf.c 153 2007-08-03 07:55:49Z xgwang $
** 
** readqpf.c: Read QPF font from file.
**
** Copyright (C) 2007 Feynman Software
**
** Create date: 2007/07/27
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "qpf.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

static void read_data (GLYPHTREE* tree, unsigned char** data)
{
    int i;
    int n = tree->max - tree->min + 1;

    for (i = 0; i < n; i++) {
        int data_size;

        data_size = tree->glyph[i].metrics->linestep * 
                tree->glyph[i].metrics->height;
        tree->glyph[i].data = *data;
	   	*data += data_size;
    }

    if (tree->less)
        read_data (tree->less, data);
    if (tree->more)
        read_data (tree->more, data);
}

static void read_metrics (GLYPHTREE* tree, unsigned char** data)
{
    int i;
    int n = tree->max - tree->min + 1;

    for (i = 0; i < n; i++) {
        tree->glyph[i].metrics = (GLYPHMETRICS*) *data;

        *data += sizeof (GLYPHMETRICS);
    }

    if (tree->less)
        read_metrics (tree->less, data);
    if (tree->more)
        read_metrics (tree->more, data);
}

static int read_node (GLYPHTREE* tree, unsigned char** data)
{
    unsigned char rw, cl;
    int flags, n;
	int nr_node = 0;

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    tree->min_rw = rw;
    tree->min_cl = cl;
    tree->min = (rw << 8) | cl;

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    tree->max_rw = rw;
    tree->max_cl = cl;
    tree->max = (rw << 8) | cl;

    flags = **data; 
    tree->flags =  **data;
    (*data)++;

	nr_node++;

    if (flags & 1) {
        tree->less = calloc (1, sizeof (GLYPHTREE));
		tree->less->parent = tree;
	} else {
        tree->less = NULL;
	}

    if (flags & 2) {
        tree->more = calloc (1, sizeof (GLYPHTREE));
		tree->more->parent = tree;
	} else {
        tree->more = NULL;
	}

    n = tree->max - tree->min + 1;
    tree->glyph = calloc (n, sizeof (GLYPH));

    if (tree->less)
        nr_node += read_node (tree->less, data);
    if (tree->more)
        nr_node += read_node (tree->more, data);

	return nr_node;
}

/*
static void build_glyph_tree (GLYPHTREE* tree, unsigned char** data)
{
    read_node (tree, data);
    read_metrics (tree, data);
    read_data (tree, data);
}
*/

static long file_size(FILE *stream)
{
   long curpos, length;

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);

   return length;
}

/********************** Load qpf font ***********************/
BOOL load_qpf(QPFINFO* qpf_info, const char* file)
{
    FILE *fp;
    long size = 0;
    unsigned char* data;

    if ((fp = fopen (file, "rb")) == NULL) return FALSE;
	if ((size = file_size(fp)) == 0) return FALSE;

    qpf_info->file_size = size;

    qpf_info->fm = calloc (1, size);
	if (qpf_info->fm == NULL) {
		fclose (fp);
		return FALSE;
	}

	if (fread (qpf_info->fm, sizeof(unsigned char), size, fp) < size)
		return FALSE;

    qpf_info->tree = calloc (1, sizeof (GLYPHTREE));

    data = (unsigned char*)qpf_info->fm;
    data += sizeof (QPFMETRICS);
    //build_glyph_tree (qpf_info->tree, &data);
    qpf_info->nr_node = read_node (qpf_info->tree, &data);
    read_metrics (qpf_info->tree, &data);
    read_data (qpf_info->tree, &data);

	qpf_info->height = qpf_info->fm->ascent +
		qpf_info->fm->descent + qpf_info->fm->leading;
	
	qpf_info->first_char = '\0';
	strncpy(qpf_info->font_name, file, MIN(strlen(file), LEN_UNIDEVFONT_NAME));
	
	qpf_info->font_name[MIN(LEN_UNIDEVFONT_NAME, strlen(file))] = '\0';

    fclose (fp);
    return TRUE;
}

static void free_glyph_tree(GLYPHTREE* tree)
{
    if (tree->less) {
        free_glyph_tree(tree->less);
    }
    if (tree->more) {
        free_glyph_tree(tree->more);
    }

    free (tree->glyph);
    free (tree->less);
    free (tree->more);
}

void free_qpf(QPFINFO *qpf_info)
{
	free_glyph_tree(qpf_info->tree);
	free(qpf_info->fm);
	free(qpf_info->tree);

    memset (qpf_info, 0, sizeof (QPFINFO));
}
