/*
** $Id: dumpqpf.c 153 2007-08-03 07:55:49Z xgwang $
** 
** dumpqpf.c: dump QPF font to file.
**
** Copyright (C) 2007 FMSoft
**
** Create date: 2007/07/27
*/

#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "qpf.h"

static int cal_glyph_changed_size(GLYPHTREE *glyph_tree)
{
    int i;
    int n = glyph_tree->max - glyph_tree->min + 1;
	unsigned int size = 0;

	size += n * sizeof(GLYPHMETRICS);
    for (i = 0; i < n; i++) {
        size += glyph_tree->glyph[i].metrics->linestep * 
                glyph_tree->glyph[i].metrics->height;
    }

    if (glyph_tree->less)
        size += cal_glyph_changed_size(glyph_tree->less);
    if (glyph_tree->more)
        size += cal_glyph_changed_size(glyph_tree->more);

	return size;
}

static void glyph_copy(unsigned char** ptr, GLYPHTREE* glyph_tree)
{
	int i = 0;
	int n = glyph_tree->max - glyph_tree->min + 1;

	for (i = 0; i < n; i++) {
		int data_size;

		data_size = glyph_tree->glyph[i].metrics->linestep
			        * glyph_tree->glyph[i].metrics->height;
		
		memcpy(*ptr, glyph_tree->glyph[i].data, data_size); 
		(*ptr) += data_size;
	}

	if (glyph_tree->less)
		glyph_copy(ptr, glyph_tree->less);
	if (glyph_tree->more)
		glyph_copy(ptr, glyph_tree->more);
}

static void glyph_metrics_copy(unsigned char** ptr, GLYPHTREE* glyph_tree)
{
	int i = 0;
	int n = glyph_tree->max - glyph_tree->min + 1;

	for (i = 0; i < n; i++) {
		memcpy(*ptr, glyph_tree->glyph[i].metrics, sizeof(GLYPHMETRICS)); 
		(*ptr) += sizeof(GLYPHMETRICS);
	}

	if (glyph_tree->less)
		glyph_metrics_copy(ptr, glyph_tree->less);
	if (glyph_tree->more)
		glyph_metrics_copy(ptr, glyph_tree->more);
}

static void glyph_nodes_copy(unsigned char **ptr, GLYPHTREE* glyph_tree)
{
	**ptr = glyph_tree->min_rw; (*ptr)++;
	**ptr = glyph_tree->min_cl; (*ptr)++;
	**ptr = glyph_tree->max_rw; (*ptr)++;
	**ptr = glyph_tree->max_cl; (*ptr)++;
	**ptr = glyph_tree->flags;  (*ptr)++;

	if (glyph_tree->less)
		glyph_nodes_copy(ptr, glyph_tree->less);
	if (glyph_tree->more)
		glyph_nodes_copy(ptr, glyph_tree->more);
}

BOOL dump_qpf(const QPFINFO* qpf_info, char* file)
{
	FILE* fp;
	if (qpf_info == NULL || file == NULL || qpf_info->fm == NULL) 
		return FALSE;

	if ((fp = fopen (file, "w+")) == NULL)
		return FALSE;

	if (!qpf_info->file_size_change) {
		printf("qpf file size is not changed\n");
		fwrite (qpf_info->fm, qpf_info->file_size, 1, fp);
	}else {
		unsigned char *start_ptr = NULL;
		unsigned char *move_ptr  = NULL;
		int qpf_metrics_size     = 0;
		int nr_node_size         = 0;
		int glyph_changed_size   = 0;
		int file_size            = 0;

		qpf_metrics_size = sizeof(QPFMETRICS);
		/*
		   cal rw cl flag size
		   | rw | cl | rw | cl | flag |
		   1B   1B   1B   1B    1B
		   */
		nr_node_size = qpf_info->nr_node * (5*sizeof(unsigned char));
		glyph_changed_size = cal_glyph_changed_size(qpf_info->tree);
		file_size = qpf_metrics_size + nr_node_size + glyph_changed_size;

		if ((start_ptr = calloc(1, file_size)) == NULL) {
			fprintf(stderr, "Not enough memory dump qpf file\n");
			fclose (fp);
			return FALSE;
		}

		move_ptr = start_ptr;
		if (!qpf_info->file_add_char) {
			memcpy(move_ptr, qpf_info->fm, (qpf_metrics_size + nr_node_size));
			move_ptr += qpf_metrics_size + nr_node_size;
		} else {
			printf("dump qpf: add a  char \n");
			memcpy(move_ptr, qpf_info->fm, qpf_metrics_size);
			move_ptr += qpf_metrics_size;
			glyph_nodes_copy(&move_ptr, qpf_info->tree);
		}

		glyph_metrics_copy(&move_ptr, qpf_info->tree);
		glyph_copy(&move_ptr, qpf_info->tree);

		int size = 0;
		if ((size = fwrite (start_ptr, file_size, 1, fp)) != 1) {
			fprintf(stderr, "Write data to file:  %s failed ---size = %d\n", file, size);
			fclose (fp);
			return FALSE;
		}
	}

	fclose (fp);
	return TRUE;
}
