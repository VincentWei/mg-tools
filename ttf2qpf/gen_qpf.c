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
typedef unsigned char uchar;
typedef unsigned char Uint8;
typedef char Sint8;
typedef unsigned char BYTE;

/**********************************************/
/* QPF structures.                            */
/**********************************************/
typedef struct _GLYPHMETRICS {
    Uint8 linestep;
    Uint8 width;
    Uint8 height;
    Uint8 padding;

    Sint8 bearingx;
    Uint8 advance;
    Sint8 bearingy;

    Sint8 reserved;
}GLYPHMETRICS;

typedef struct _GLYPH {
    GLYPHMETRICS *metrics;
    uchar *data;
}GLYPH;

typedef struct _GLYPHTREE {
    unsigned int min, max;
    struct _GLYPHTREE *less;
    struct _GLYPHTREE *more;
    GLYPH *glyph;

    unsigned int index_node;
    unsigned int index_glyph_metrics;
    unsigned int index_glyph_data;
}GLYPHTREE;

typedef struct _QPFMETRICS {
    Sint8 ascent, descent;
    Sint8 leftbearing, rightbearing;
    Uint8 maxwidth;
    Sint8 leading;
    Uint8 flags;
    Uint8 underlinepos;
    Uint8 underlinewidth;
    Uint8 reserved3;
}QPFMETRICS;

typedef struct _QPFINFO {
    char *font_name;
    unsigned int height;
    unsigned int width;

    unsigned int file_size;
    QPFMETRICS *fm;

    GLYPHTREE *tree;
    int max_bmp_size;
    unsigned char *std_bmp;
}QPFINFO;

typedef struct _QPFNODE {
    unsigned char min_l;
    unsigned char min_h;
    unsigned char max_l;
    unsigned char max_h;
    unsigned char flag;
}QPFNODE;

static GLYPHTREE *root = NULL;
static GLYPH g_glyph [0xffff];
static QPFMETRICS qmetrics;

/** =====================================================================
  * Get the related glyphs of a node.
  * ===================================================================== */
static void get_GLYPH (int ch, GLYPHTREE *root)
{
    if (ch < root->min) {
        if (!root->less) {
            root->less = (GLYPHTREE *) calloc (sizeof (GLYPHTREE), 1);
            root->less->min = root->less->max = ch;
        }
        
        get_GLYPH (ch, root->less);
    }
    else if (ch > root->max) {
        if (!root->more) {
            root->more = (GLYPHTREE *) calloc (sizeof (GLYPHTREE), 1);
            root->more->min = root->more->max = ch;
        }

        get_GLYPH (ch, root->more);
    }

    root->glyph = &g_glyph [ch]; 
}

/** =====================================================================
  * Concat the continugous items.
  * ===================================================================== */
static GLYPH* concatGlyphs(GLYPHTREE* a, GLYPHTREE* b, unsigned int *min, unsigned int *max)
{
    *min = a->min;
    *max = b->max;

    return &(a->glyph [a->min]);
}

/** =====================================================================
  * Combines the continugous items.
  * ===================================================================== */
static void compress (GLYPHTREE *root) 
{
    if (root->less) {
        compress (root->less);

        if (root->less->max == root->min + 1) {
            GLYPH *newGlyph = concatGlyphs (root->less, root, &root->min, &root->max);
            GLYPHTREE *t = root->less->less;

            root->less->less = NULL;
            free (root->less);
            root->less = t;

            root->glyph = newGlyph;
        }
    }

    if (root->more) {
        compress (root->more);

        if (root->more->min == root->max + 1) {
            GLYPH *newGlyph = concatGlyphs (root, root->more, &root->min, &root->max);
            GLYPHTREE *t = root->more->more;

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
static int balance (GLYPHTREE **root, int *l, int *m)
{
	if ( *root ) {
	    int ll, lm, ml, mm;
	    *l = balance (&(*root)->less, &ll, &lm);
	    *m = balance (&(*root)->more, &ml, &mm);

	    if ( (*root)->more ) {
            if ( *l + ml + 1 < mm ) {
                /* Shift less-ward */
                GLYPHTREE* b = *root;
                GLYPHTREE* c = (*root)->more;
                *root = c;
                b->more = c->less;
                c->less = b;
            }
	    }

	    if ( (*root)->less ) {
            if ( *m + lm + 1 < ll ) {
                /* Shift more-ward */
                GLYPHTREE* c = *root;
                GLYPHTREE* b = (*root)->less;
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

/** =====================================================================
  * Write the qpf metrics data to the qpf file's header.
  * ===================================================================== */
static int WriteQPFHeader (FILE *fp)
{
    int ret = 0;

    ret = fwrite (&qmetrics, sizeof (QPFMETRICS), 1, fp);

    return ret;
}

/** =====================================================================
  * Write the nodes data to the qpf file.
  * ===================================================================== */
static int WriteQPFNodes (GLYPHTREE *root, FILE *fp)
{
    int ret = 0;
    QPFNODE qnode;
   
    memset (&qnode, 0, sizeof (QPFNODE)); 

    qnode.min_l = (root->min & 0xff00) >> 8;
    qnode.min_h = (root->min & 0x00ff);
    qnode.max_l = (root->max & 0xff00) >> 8;
    qnode.max_h = (root->max & 0x00ff);

    if (root->less) qnode.flag |= 1;
    if (root->more) qnode.flag |= 2;
    
    ret = fwrite (&qnode, sizeof (QPFNODE), 1, fp);  /* carefully! */

    if (root->less) WriteQPFNodes (root->less, fp);
    if (root->more) WriteQPFNodes (root->more, fp);

    return ret;
}

/** =====================================================================
  * Write the metrics data to the qpf file.
  * ===================================================================== */
static int WriteQPFMetrics (GLYPHTREE *root, FILE *fp)
{
    int n = root->max - root->min + 1;
    volatile int i = root->min;
    int ret = 0;
   
    for (; (i < n + root->min); i ++)
        ret = fwrite (g_glyph [i].metrics, sizeof (GLYPHMETRICS), 1, fp);

    if (root->less) WriteQPFMetrics (root->less, fp);
    if (root->more) WriteQPFMetrics (root->more, fp);

    return ret;
}

/** =====================================================================
  * Write the bitmap data to the qpf file.
  * ===================================================================== */
static int WriteQPFData (GLYPHTREE *root, FILE *fp)
{
    int n = root->max - root->min + 1;
    int i = root->min;
    int ret = 0;

    for (; (i < n + root->min); i ++) {
        ret = fwrite ( g_glyph [i].data, 
               g_glyph [i].metrics->linestep * g_glyph [i].metrics->height,
                1, fp); 
    }

    if (root->less) WriteQPFData (root->less, fp);
    if (root->more) WriteQPFData (root->more, fp);

    return ret;
}

/** =====================================================================
  * Write the qpf data stored in the ram to the qpf file.
  * ===================================================================== */
static void WriteQPF (FILE *fp)
{
    WriteQPFHeader (fp);
    WriteQPFNodes (root, fp);
    WriteQPFMetrics (root, fp);
    WriteQPFData (root, fp);
}

static int generate_qpf (FILE *out, char *oname)
{
    unsigned short code, com_code, max_wd = 0;
    int l, m;
    FILE *fp = NULL;

    /* TODO: initialize the qpf metrics informations
    memset (&qmetrics, 0, sizeof (QPFMETRICS));
    qmetrics.ascent = ;
    qmetrics.descent = ;
    */

    /* TODO: prepare the metrics and data for glyphs
    for (code = 0; code < 0xFFFF; code++) {

        if (code has glyph) {
            g_glyph [code].metrics = (GLYPHMETRICS *) calloc (1, sizeof (GLYPHMETRICS));
            g_glyph [code].metrics->linestep = ;
            g_glyph [code].metrics->height = ;
            g_glyph [code].metrics->width = ;
            g_glyph [code].metrics->bearingx = ;
            g_glyph [code].metrics->bearingy = ;
            g_glyph [code].metrics->advance = ;

            g_glyph [code].data = (unsigned char *) calloc (
                    g_glyph [code].metrics->linestep*g_glyph [code].metrics->height, 1);
            // copy glyph data to glyph [code].data here

            if (g_glyph [code].metrics->width > max_wd) 
                max_wd = g_glyph [code].metrics->width;
        }
    */

    com_code = 0; l = 0; m = 0;

    /* build the AVL tree */
    if (!root) {
        root = (GLYPHTREE *) calloc (1, sizeof (GLYPHTREE));
        if (!root) {
            fprintf (stderr, "calloc memory space for root tree node fail\n");
            exit (1);
        }
    }

    for (code = 0; code < 0xFFFF; code ++) {
        if (g_glyph [code].data) {
            root->min = root->max = code;
            root->glyph = &g_glyph [code];
            code ++;
            break;
        }
    }

    for (; code < 0xFFFF; code ++) {
        if (g_glyph [code].data) {
           get_GLYPH (code, root); 
           if (! (com_code ++ & 0x3f)) {
               compress (root);
               balance (&root, &l, &m);
           }
        }
    }
    
    compress (root);
    balance (&root, &l, &m);

    qmetrics.maxwidth = max_wd;

    printf ("qmetrics.ascent:%d, descent:%d, maxwidth:%d\n",
            qmetrics.ascent, qmetrics.descent, qmetrics.maxwidth);

    /* write the data to the related qpf file */
    fp = fopen (oname, "wb");
    if (!fp) {
        printf ("can not open the file: %s in write mode!\n", oname);
    }
    else {
        WriteQPF (fp);
        fclose (fp);
    }

    return 0;
}

