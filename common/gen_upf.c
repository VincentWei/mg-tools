/*
** $Id$
** 
** gen_upf.c: a upf file generator
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

static UPFGLYPH* g_glyph;

/** =====================================================================
  * Get the related glyphs of a node.
  * ===================================================================== */
static void get_GLYPH (int ch, UPFGLYPHTREE *root)
{
    if (ch < root->min) {
        if (!root->less) {
            root->less = (UPFGLYPHTREE *) calloc (1, sizeof (UPFGLYPHTREE));
            root->less->min = root->less->max = ch;
        }
        
        get_GLYPH (ch, root->less);
    }
    else if (ch > root->max) {
        if (!root->more) {
            root->more = (UPFGLYPHTREE *) calloc (1, sizeof (UPFGLYPHTREE));
            root->more->min = root->more->max = ch;
        }

        get_GLYPH (ch, root->more);
    }

    root->glyph = &g_glyph [ch]; 
}

/** =====================================================================
  * Concat the continugous items.
  * ===================================================================== */
static UPFGLYPH* concatGlyphs(UPFGLYPHTREE* a, UPFGLYPHTREE* b, unsigned int *min, unsigned int *max)
{
    *min = a->min;
    *max = b->max;

    return &g_glyph [a->min];
}

/** =====================================================================
  * Combines the continugous items.
  * ===================================================================== */
static void compress (UPFGLYPHTREE *root) 
{
    if (root->less) {
        compress (root->less);

        if (root->less->max == root->min + 1) {
            UPFGLYPH *newGlyph = concatGlyphs (root->less, root, &root->min, &root->max);
            UPFGLYPHTREE *t = root->less->less;

            root->less->less = NULL;
            free (root->less);
            root->less = t;

            root->glyph = newGlyph;
        }
    }

    if (root->more) {
        compress (root->more);

        if (root->more->min == root->max + 1) {
            UPFGLYPH *newGlyph = concatGlyphs (root, root->more, &root->min, &root->max);
            UPFGLYPHTREE *t = root->more->more;

            root->more->more = NULL;

            free (root->more);
            root->more = t;

            root->glyph = newGlyph;
        }
    }
}

/** =====================================================================
  * To establish an AVL tree.
  * ===================================================================== */
static int balance (UPFGLYPHTREE **root, int *l, int *m)
{
	if ( *root ) {
	    int ll, lm, ml, mm;
	    *l = balance (&(*root)->less, &ll, &lm);
	    *m = balance (&(*root)->more, &ml, &mm);

	    if ( (*root)->more ) {
            if ( *l + ml + 1 < mm ) {
                /* Shift less-ward */
                UPFGLYPHTREE* b = *root;
                UPFGLYPHTREE* c = (*root)->more;
                *root = c;
                b->more = c->less;
                c->less = b;
            }
	    }

	    if ( (*root)->less ) {
            if ( *m + lm + 1 < ll ) {
                /* Shift more-ward */
                UPFGLYPHTREE* c = *root;
                UPFGLYPHTREE* b = (*root)->less;
                *root = b;
                c->less = b->more;
                b->more = c;
            }
	    }

	    return 1 + *l + *m;
	} 
    else {
	    *l = *m = 0;
	    return 0;
	}
}

void upf_travel_glyph_tree (UPFGLYPHTREE *root, UPF_CB_TRAVEL_GLYPH_TREE cb, void* ctxt)
{
    if (root) {
        cb (ctxt, root);

        if (root->less) {
            upf_travel_glyph_tree (root->less, cb, ctxt);
        }

        if (root->more) {
            upf_travel_glyph_tree (root->more, cb, ctxt);
        }
    }
}

UPFGLYPHTREE* upf_make_glyph_tree (UPFGLYPH* glyphs)
{
    unsigned int code, com_code;
    int l, m;
    UPFGLYPHTREE* root = NULL;

    g_glyph = glyphs;

    com_code = 0; l = 0; m = 0;

    /* build the AVL tree */
    if (!root) {
        root = (UPFGLYPHTREE *) calloc (1, sizeof (UPFGLYPHTREE));
        if (!root) {
            fprintf (stderr, "UPF Generator: calloc memory space for root tree node fail.\n");
            return NULL;
        }
    }

    for (code = 0; code < 0xFFFF; code ++) {
        if (g_glyph [code].bitmap_offset) {
            root->min = root->max = code;
            root->glyph = &g_glyph [code];
            code++;
            break;
        }
    }

    for (; code < 0xFFFF; code ++) {
        if (g_glyph [code].bitmap_offset) {
           get_GLYPH (code, root); 
           if (! (com_code ++ & 0x3f)) {
               compress (root);
               balance (&root, &l, &m);
           }
        }
    }

    compress (root);
    balance (&root, &l, &m);

    /* write the data to the related upf file */
    return root;
}

static void clear_glyph_tree (UPFGLYPHTREE* tree)
{
    if (tree->less) {
        clear_glyph_tree (tree->less);
    }
    if (tree->more) {
        clear_glyph_tree (tree->more);
    }

    free (tree->less);
    free (tree->more);
}

void upf_delete_glyph_tree (UPFGLYPHTREE* root)
{
    if (root) {
        clear_glyph_tree (root);
        free (root);
    }
}

/** =====================================================================
  * Write the upf header data to the upf file's header.
  * ===================================================================== */
static int WriteUPFHeader (const UPFV1_FILE_HEADER* file_hdr, FILE *fp)
{
    int ret = 0;

    ret = fwrite (file_hdr, sizeof (UPFV1_FILE_HEADER), 1, fp);

    return ret;
}

/** =====================================================================
  * Write the nodes data to the upf file.
  * ===================================================================== */
static int written_glyphs;
static int WriteUPFNodes (const UPFGLYPHTREE *root, 
                const UPFV1_FILE_HEADER* file_hdr, FILE *fp)
{
    int ret = 0;
    UPFNODE upf_node;
    long curr_pos;

    upf_node.min = root->min;
    upf_node.max = root->max;
    upf_node.less_offset = 0;
    upf_node.more_offset = 0;
    upf_node.glyph_offset = file_hdr->off_glyphs + sizeof (UPFGLYPH) * written_glyphs;
    written_glyphs += root->max - root->min + 1;

    curr_pos = ftell (fp);
    ret = fwrite (&upf_node, sizeof (UPFNODE), 1, fp);

    if (root->less) {
        upf_node.less_offset = ftell (fp) ;
        fseek (fp, curr_pos, SEEK_SET);
        fwrite (&upf_node,  sizeof (UPFNODE),  1, fp);
        fseek (fp, 0L, SEEK_END);
        WriteUPFNodes (root->less, file_hdr, fp);
    }

    if (root->more) {
        upf_node.more_offset = ftell (fp) ;
        fseek (fp, curr_pos, SEEK_SET);
        fwrite (&upf_node,  sizeof (UPFNODE),  1, fp);
        fseek (fp, 0L, SEEK_END);
        WriteUPFNodes (root->more, file_hdr, fp);
    }

    return ret;
}

/** =====================================================================
  * Write the metrics data to the upf file.
  * ===================================================================== */
static int WriteUPFMetrics (const UPFGLYPHTREE *root, 
                const UPFV1_FILE_HEADER* file_hdr, FILE *fp)
{
    int n = root->max - root->min + 1;
    volatile int i = root->min;
    int ret = 0;
   
    for (; (i < n + root->min); i ++) {
        UPFGLYPH glyph = g_glyph [i];
        glyph.bitmap_offset += file_hdr->off_bitmaps;
        ret = fwrite (&glyph, sizeof (UPFGLYPH), 1, fp);
    }

    if (root->less) WriteUPFMetrics (root->less, file_hdr, fp);
    if (root->more) WriteUPFMetrics (root->more, file_hdr, fp);

    return ret;
}

#if 0
static const unsigned char* g_buff_bitmap;

/** =====================================================================
  * Write the bitmap data to the upf file.
  * ===================================================================== */
static int WriteUPFData (const UPFGLYPHTREE* root, FILE *fp)
{
    int n = root->max - root->min + 1;
    int i = root->min;
    int ret = 0;

    for (; (i < n + root->min); i ++) {
        ret = fwrite (g_buff_bitmap + g_glyph [i].bitmap_offset, 
               g_glyph [i].pitch * g_glyph [i].height, 1, fp); 
    }

    if (root->less) WriteUPFData (root->less, fp);
    if (root->more) WriteUPFData (root->more, fp);

    return ret;
}
#endif

/** =====================================================================
  * Write the upf data stored in the ram to the upf file.
  * ===================================================================== */
static void WriteUPF (FILE *fp, const UPFV1_FILE_HEADER* file_hdr,
                 const UPFGLYPHTREE* root, const unsigned char* buff_bitmap)
{
    WriteUPFHeader (file_hdr, fp);

    written_glyphs = 0;
    fseek (fp, file_hdr->off_nodes, SEEK_SET);
    WriteUPFNodes (root, file_hdr, fp);

    fseek (fp, file_hdr->off_glyphs, SEEK_SET);
    WriteUPFMetrics (root, file_hdr, fp);

    fseek (fp, file_hdr->off_bitmaps, SEEK_SET);
#if 0
    WriteUPFData (root, fp);
#else
    fwrite (buff_bitmap, file_hdr->len_bitmaps, 1, fp);
#endif
}

int upf_generate_upf_file (const char* oname, const UPFV1_FILE_HEADER* file_hdr,
                const UPFGLYPHTREE* root, const unsigned char* buff_bitmap)
{
    FILE* fp;

    fp = fopen (oname, "wb");

    if (!fp) {
        fprintf (stderr, "UPF Generator: can not open the file: %s in write mode!\n", oname);
        return -1;
    }
    else {
        long file_size;

#if 0
        g_buff_bitmap = buff_bitmap;
#endif
        WriteUPF (fp, file_hdr, root, buff_bitmap);
        file_size = ftell (fp);
        fclose (fp);

        if (file_size != file_hdr->font_size) {
            fprintf (stderr, "UPF Generator: internal error occured: %s\n", oname);
            return -1;
        }
    }

    return 0;
}


