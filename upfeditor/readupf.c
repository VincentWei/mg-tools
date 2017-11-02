/*
** $Id: readupf.c 286 2009-12-10 04:56:03Z weiym $
** 
** readupf.c: Read UPF font from file.
**
** Copyright (C) 2009 Feynman Software
**
** Create date: 2007/07/27
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "upf.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define VAL_DEBUG
#include <my_debug.h>

/*read bmpdata to  tree (after the tree is created, metrix is readed),
 * data size is pitch * height*/
static void read_data (UPFGLYPHTREE* tree, unsigned char* base)
{
    int i;
    int n = tree->upf_node.max - tree->upf_node.min + 1;

    for (i = 0; i < n; i++) {
        int data_size;

        data_size = tree->glyph[i].metrics->upf_glyph.pitch * 
                tree->glyph[i].metrics->upf_glyph.height;
        tree->glyph[i].data = calloc (1,data_size);
        if (NULL == tree->glyph[i].data){
            printf("Serious error: There is no more memory for use!\n");
            return ;
        }

        memcpy (tree->glyph[i].data, base + tree->glyph[i].metrics->upf_glyph.bitmap_offset, 
                data_size);
    }

    if (tree->less)
        read_data (tree->less, base);
    if (tree->more)
        read_data (tree->more, base);
}

/*read bmpdata to tree (after the tree is created)*/
static void read_metrics (UPFGLYPHTREE* tree, unsigned char** data)
{
    int i;
    int n = tree->upf_node.max - tree->upf_node.min + 1;

    for (i = 0; i < n; i++) {
        tree->glyph[i].metrics =calloc(1,sizeof(GLYPHMETRICS));
        if(NULL == tree->glyph[i].metrics){
            printf("\n########################Serious error: There is no more memory for use!!file:%s,function:%s###############################\n",__FILE__,__func__);
            return ;
        }
        memcpy(&tree->glyph[i].metrics->upf_glyph,(*data),sizeof(UPFGLYPH));

        *data += sizeof (UPFGLYPH);
    }

    if (tree->less)
        read_metrics (tree->less, data);
    if (tree->more)
        read_metrics (tree->more, data);
}

/*read nodes and finish creating the tree from root node*/
static void read_node (UPFGLYPHTREE* tree, unsigned char** data)
{
    int n;

    memcpy(&tree->upf_node,(*data),sizeof(UPFNODE));
    *data += sizeof(UPFNODE);

    if (tree->upf_node.less_offset) {
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
    tree->glyph = calloc (n, sizeof (GLYPH));

    if (tree->less)
         read_node (tree->less, data);
    if (tree->more)
        read_node (tree->more, data);
}

static inline long get_file_size (const char* filename)
{
    FILE* fp = fopen (filename, "rb");
    if (fp == NULL)
        return 0;

    fseek (fp, 0, SEEK_END);
    return ftell (fp);
}

static MGUPFINFO* load_font_data (const char* file_name)
{
    FILE* fp = NULL;
    UPFV1_FILE_HEADER * filehead;
    long file_size;
    MGUPFINFO* upf_info = (MGUPFINFO*) calloc (1, sizeof(MGUPFINFO));

    if ((fp = fopen (file_name, "r")) == NULL) {
        printf ("upfeditor: can't open font file: %s\n", file_name);
        goto error;
    }

    if ((file_size = get_file_size (file_name)) <= 0) {
        printf ("upfeditor: empty font file: %s\n", file_name);
        goto error;
    }

    upf_info->file_size = file_size;

    upf_info->root_dir = mmap (0, file_size, PROT_READ, 
            MAP_SHARED, fileno(fp), 0);

    if (!upf_info->root_dir || upf_info->root_dir == MAP_FAILED) {
        printf ("upfeditor: mmap failed: %s\n", file_name);
        goto error;
    }

    filehead = (UPFV1_FILE_HEADER *)upf_info->root_dir;
    if (filehead->endian !=0x1234) {
        printf ("upfeditor: endian does not matched: %s\n", file_name);
        munmap (upf_info->root_dir, upf_info->file_size);
        goto error;
    }
    
    fclose (fp);
    return upf_info;

error:
    if (fp)
        fclose (fp);
    free (upf_info);
    return NULL;
}

static void unload_font_data (MGUPFINFO* data)
{
    munmap (data->root_dir, data->file_size);
    free (data);
}

/********************** Load qpf font ***********************/
BOOL load_upf (UPFINFO* upf_info, const char* file)
{
    unsigned char* dataoffs;
    MGUPFINFO* mg_upf_info;

    mg_upf_info = load_font_data (file);
    if (mg_upf_info == NULL)
        return FALSE;

    memcpy (&upf_info->upf_file_header, mg_upf_info->root_dir, 
            sizeof (UPFV1_FILE_HEADER));
    upf_info->upf_tree = calloc (1, sizeof (UPFGLYPHTREE));
    upf_info->upf_tree->parent = NULL;

    dataoffs = mg_upf_info->root_dir + upf_info->upf_file_header.off_nodes;
    read_node (upf_info->upf_tree, &dataoffs);

    dataoffs = mg_upf_info->root_dir + upf_info->upf_file_header.off_glyphs;
    read_metrics (upf_info->upf_tree, &dataoffs);

    read_data (upf_info->upf_tree, mg_upf_info->root_dir);

    unload_font_data (mg_upf_info);
    return TRUE;
}

void free_upf(UPFINFO *upf_info)
{
	free_glyph_tree(&(upf_info->upf_tree));
    memset (upf_info, 0, sizeof (UPFINFO));
}

