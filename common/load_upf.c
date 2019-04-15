/*
** $Id$
** 
** load_upf.c: a upf loader
**
** Copyright (C) 2009 FMSoft
**
** Author: WEI Yongming
**
** Create date: 2009/12/19
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "upf.h"

static inline long get_opened_file_size (FILE* fp)
{
    long size;
    long pos = ftell (fp);

    fseek (fp, 0, SEEK_END);
    size = ftell (fp);
    fseek (fp, pos, SEEK_SET);
    return size;
}

#define prog "UPF Loader"

UPFINFO* upf_load_font_data (const char* file_name)
{
    FILE* fp = NULL;
    UPFV1_FILE_HEADER * filehead;
    long file_size;
    UPFINFO* upf_info = (UPFINFO*) calloc (1, sizeof(UPFINFO));

    if ((fp = fopen (file_name, "r")) == NULL) {
        printf ("%s: can't open font file: %s\n", prog, file_name);
        goto error;
    }

    if ((file_size = get_opened_file_size (fp)) <= 0) {
        printf ("%s: empty font file: %s\n", prog, file_name);
        goto error;
    }

    upf_info->file_size = file_size;

    upf_info->root_dir = mmap (0, file_size, PROT_READ, 
            MAP_SHARED, fileno(fp), 0);

    if (!upf_info->root_dir || upf_info->root_dir == MAP_FAILED) {
        printf ("%s: mmap failed: %s\n", prog, file_name);
        goto error;
    }

    filehead = (UPFV1_FILE_HEADER *)upf_info->root_dir;
    if (filehead->endian !=0x1234) {
        printf ("%s: endian does not matched: %s\n", prog, file_name);
        munmap (upf_info->root_dir, upf_info->file_size);
        goto error;
    }

    strcpy (upf_info->font_name, filehead->font_name);
    
    fclose (fp);
    return upf_info;

error:
    if (fp)
        fclose (fp);
    free (upf_info);
    return NULL;
}

void upf_unload_font_data (void* data)
{
    if (data) {
        munmap (((UPFINFO*) data)->root_dir, ((UPFINFO*) data)->file_size);
        free (((UPFINFO*) data));
    }
}

UPFGLYPH* upf_get_glyph (Uint8 *upf_root, UPFNODE * tree, unsigned int ch)
{
    if (ch < tree->min) {
        if (!tree->less_offset)
            return NULL;

        return upf_get_glyph (upf_root, (UPFNODE*)(upf_root + tree->less_offset), ch);
    }
    else if ( ch > tree->max ) {

        if (!tree->more_offset) {
            return NULL;
        }
        return upf_get_glyph (upf_root, (UPFNODE*)(upf_root + tree->more_offset), ch);
    }

    return (UPFGLYPH*)(upf_root + tree->glyph_offset + sizeof (UPFGLYPH) * (ch - tree->min));
}

