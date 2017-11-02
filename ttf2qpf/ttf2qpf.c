/*
 * Copyright 1996, 1997, 1998, 1999 Computing Research Labs,
 * New Mexico State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef lint
#ifdef __GNUC__
static char rcsid[] __attribute__ ((unused)) = "$Id: ttf2bdf.c,v 1.25 1999/10/21 16:31:54 mleisher Exp $";
#else
static char rcsid[] = "$Id: ttf2bdf.c,v 1.25 1999/10/21 16:31:54 mleisher Exp $";
#endif
#endif

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <unistd.h>
#endif

#include <string.h>

#include "freetype/freetype.h"
#include "freetype/ftxsbit.h"

#define MAX_UCS     0x10ffff

/*
 * Include the remapping support.
 */
#include "remap.h"

/**************************************************************************
 *
 * Macros.
 *
 **************************************************************************/

/*
 * The version of ttf2bdf.
 */
#define TTF2BDF_VERSION "2.8"

/*
 * Set the default values used to generate a BDF font.
 */
#ifndef DEFAULT_PLATFORM_ID
#define DEFAULT_PLATFORM_ID 3
#endif

#ifndef DEFAULT_ENCODING_ID
#define DEFAULT_ENCODING_ID 1
#endif

#ifndef DEFAULT_POINT_SIZE
#define DEFAULT_POINT_SIZE 12
#endif

#ifndef DEFAULT_RESOLUTION
#define DEFAULT_RESOLUTION 100
#endif

/*
 * Used as a fallback for XLFD names where the character set/encoding can not
 * be determined.
 */
#ifndef DEFAULT_XLFD_CSET
#define DEFAULT_XLFD_CSET "-FontSpecific-0"
#endif

/*
 * nameID macros for getting strings from the TT font.
 */
#define TTF_COPYRIGHT 0
#define TTF_TYPEFACE  1
#define TTF_PSNAME    6

#ifndef MAX
#define MAX(h,i) ((h) > (i) ? (h) : (i))
#endif

#ifndef MIN
#define MIN(l,o) ((l) < (o) ? (l) : (o))
#endif

/**************************************************************************
 *
 * General globals set from command line.
 *
 **************************************************************************/

/*
 * The program name.
 */
static char *prog;

/*
 * The flag indicating whether messages should be printed or not.
 */
static int verbose = 0;

/*
 * Flags used when loading glyphs.
 */
static int load_flags = TTLOAD_DEFAULT | TTLOAD_HINT_GLYPH;

/*
 * The default platform and encoding ID's.
 */
static int pid = DEFAULT_PLATFORM_ID;
static int eid = DEFAULT_ENCODING_ID;

/*
 * Default point size and resolutions.
 */
static int point_size = DEFAULT_POINT_SIZE;
static int hres = DEFAULT_RESOLUTION;
static int vres = DEFAULT_RESOLUTION;

/*
 * The user supplied foundry name to use in the XLFD name.
 */
static char *foundry_name = 0;

/*
 * The user supplied typeface name to use in the XLFD name.
 */
static char *face_name = 0;

/*
 * The user supplied weight name to use in the XLFD name.
 */
static char *weight_name = 0;

/*
 * The user supplied slant name to use in the XLFD name.
 */
static char *slant_name = 0;

/*
 * The user supplied width name to use in the XLFD name.
 */
static char *width_name = 0;

/*
 * The user supplied additional style name to use in the XLFD name.
 */
static char *style_name = 0;

/*
 * The user supplied spacing (p = proportional, c = character cell,
 * m = monospace).
 */
static int spacing = 0;

/*
 * The dash character to use in the names retrieved from the font.  Default is
 * the space.
 */
static int dashchar = ' ';

/*
 * Flag, bitmask, and max code for generating a subset of the glyphs in a font.
 */
static int donot_use_sbit = 0;
static int glyph_has_sbit = 0;

static TT_SBit_Image*  sbit;

/**************************************************************************
 *
 * Internal globals.
 *
 **************************************************************************/

/*
 * Structure used for calculating the font bounding box as the glyphs are
 * generated.
 */
typedef struct {
    short minlb;
    short maxlb;
    short maxrb;
    short maxas;
    short maxds;
    short rbearing;
} bbx_t;

/*
 * The Units Per Em value used in numerous places.
 */
static TT_UShort upm;

/*
 * A flag indicating if a CMap was found or not.
 */
static TT_UShort nocmap;

/**************************************************************************
 *
 * Freetype globals.
 *
 **************************************************************************/

static TT_Engine engine;
static TT_Face face;
static TT_Face_Properties properties;

static TT_Instance instance;

static TT_Glyph glyph;
static TT_Big_Glyph_Metrics metrics;
static TT_Instance_Metrics imetrics;

static TT_Raster_Map raster;

static TT_CharMap cmap;

static void usage(int eval)
{
    fprintf(stderr, "Usage: %s [options below] font.ttf\n", prog);
    fprintf(stderr, "-h\t\tThis message.\n");
    fprintf(stderr, "-v\t\tPrint warning messages during conversion.\n");
    fprintf(stderr,
            "-l \"subset\"\tSpecify a subset of glyphs to generate.\n");
    fprintf(stderr, "-m mapfile\tGlyph reencoding file.\n");
    fprintf(stderr, "-n\t\tTurn off glyph hinting.\n");
    fprintf(stderr,
            "-c c\t\tSet the character spacing (default: from font).\n");
    fprintf(stderr,
            "-f name\t\tSet the foundry name (default: freetype).\n");
    fprintf(stderr,
            "-t name\t\tSet the typeface name (default: from font).\n");
    fprintf(stderr, "-w name\t\tSet the weight name (default: Medium).\n");
    fprintf(stderr, "-s name\t\tSet the slant name (default: R).\n");
    fprintf(stderr, "-k name\t\tSet the width name (default: Normal).\n");
    fprintf(stderr,
            "-d name\t\tSet the additional style name (default: empty).\n");
    fprintf(stderr, "-u char\t\tSet the character to replace '-' in names ");
    fprintf(stderr, "(default: space).\n");
    fprintf(stderr,
            "-pid id\t\tSet the platform ID for encoding (default: %d).\n",
            DEFAULT_PLATFORM_ID);
    fprintf(stderr,
            "-eid id\t\tSet the encoding ID for encoding (default: %d).\n",
            DEFAULT_ENCODING_ID);
    fprintf(stderr, "-p n\t\tSet the point size (default: %dpt).\n",
            DEFAULT_POINT_SIZE);
    fprintf(stderr, "-r n\t\tSet the horizontal and vertical resolution ");
    fprintf(stderr, "(default: %ddpi).\n", DEFAULT_RESOLUTION);
    fprintf(stderr, "-rh n\t\tSet the horizontal resolution ");
    fprintf(stderr, "(default: %ddpi)\n", DEFAULT_RESOLUTION);
    fprintf(stderr, "-rv n\t\tSet the vertical resolution ");
    fprintf(stderr, "(default: %ddpi)\n", DEFAULT_RESOLUTION);
    fprintf(stderr,
            "-o outfile\tSet the output filename (default: stdout).\n");
    exit(eval);
}

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
static GLYPH g_glyph [MAX_UCS];
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
static void WriteFreeType2QPF (FILE *fp)
{
    WriteQPFHeader (fp);
    WriteQPFNodes (root, fp);
    WriteQPFMetrics (root, fp);
    WriteQPFData (root, fp);
}

static int generate_qpf (FILE *out, char *iname, char *oname)
{
    TT_Long i;
    TT_UShort p, e;

    TT_Short minx, miny, maxx, maxy, xoff, yoff, ymax;
    TT_Short y_off, x_off;
    TT_UShort sx, sy, ex, ey, wd, ht, max_wd = 0;
    TT_Long code, idx, ng, aw, com_code;
    int l, m, pitch;
    unsigned char *bmap;
    FILE *fp = NULL;

    if (!glyph_has_sbit) {
        /*
         * Calculate the font bounding box again so enough storage for the largest
         * bitmap can be allocated.
         */
        minx = (properties.header->xMin * imetrics.x_ppem) / upm;
        miny = (properties.header->yMin * imetrics.y_ppem) / upm;
        maxx = (properties.header->xMax * imetrics.x_ppem) / upm;
        maxy = (properties.header->yMax * imetrics.y_ppem) / upm;

        maxx -= minx; ++maxx;
        maxy -= miny; ++maxy;

        /*
         * Use the upward flow because the version of FreeType being used when
         * this was written did not support TT_Flow_Down.  This insures that this
         * routine will not mess up if TT_Flow_Down is implemented at some point.
         */
        raster.flow = TT_Flow_Down;
        raster.width = maxx;
        raster.rows = maxy;
        raster.cols = (maxx + 7) >> 3;
        raster.size = raster.cols * raster.rows;
        raster.bitmap = (void *) malloc(raster.size);

        memset (&qmetrics, 0, sizeof (QPFMETRICS));
    }

    /* initialize the qpf metrics informations */
    qmetrics.ascent = (((properties.horizontal->Ascender * \
                                    imetrics.y_scale)/0x10000) >> 6) + 1;
    qmetrics.descent = point_size - qmetrics.ascent;

    /*
     * Get the requested cmap.
     */
    for (i = 0; i < TT_Get_CharMap_Count(face); i++) {
        if (!TT_Get_CharMap_ID(face, i, &p, &e) &&
            p == pid && e == eid)
          break;
    }
    if (i == TT_Get_CharMap_Count(face) && pid == 3 && eid == 1) {
        /*
         * Make a special case when this fails with pid == 3 and eid == 1.
         * Change to eid == 0 and try again.  This captures the two possible
         * cases for MS fonts.  Some other method should be used to cycle
         * through all the alternatives later.
         */
        for (i = 0; i < TT_Get_CharMap_Count(face); i++) {
            if (!TT_Get_CharMap_ID(face, i, &p, &e) &&
                p == pid && e == 0)
              break;
        }
        if (i < TT_Get_CharMap_Count(face)) {
            if (!TT_Get_CharMap(face, i, &cmap))
              eid = 0;
            else
              nocmap = 1;
        }
    } else {
        /*
         * A CMap was found for the platform and encoding IDs.
         */
        if (i < TT_Get_CharMap_Count(face) && TT_Get_CharMap(face, i, &cmap))
          nocmap = 1;
        else
          nocmap = 0;
    }

    if (nocmap && verbose) {
        fprintf(stderr,
                    "%s: no character map for platform %d encoding %d.  ",
                    prog, pid, eid);
        fprintf(stderr, "Generating all glyphs.\n");
    }

    /* initialize the global glyph arrays' informations */
    for (ng = 0, code = 0, aw = 0; code < MAX_UCS; code++) {

        if (nocmap) {
            if (code >= properties.num_Glyphs)

              /*
               * At this point, all the glyphs are done.
               */
              break;
            idx = code;
        } else
          idx = TT_Char_Index(cmap, code);

        if (idx <= 0)
            continue;

        if (glyph_has_sbit) {
            if (TT_Load_Glyph_Bitmap (face, instance, idx, sbit))
                continue;

            metrics = sbit->metrics;
            raster = sbit->map;

            /*
             * Grid fit to determine the x and y offsets that will force the
             * bitmap to fit into the storage provided.
             */
            xoff = (63 - metrics.bbox.xMin) & -64;
            yoff = (63 - metrics.bbox.yMin) & -64;
            ymax = (63 - metrics.bbox.yMax) & -64;

        }
        else {
            /*
             * If the glyph could not be loaded for some reason, or a subset is
             * being generated and the index is not in the subset bitmap, just
             * continue.
             */

            if (TT_Load_Glyph(instance, glyph, idx, load_flags))
                continue;

            (void) TT_Get_Glyph_Big_Metrics(glyph, &metrics);

            /*
             * Clear the raster bitmap.
             */
            (void) memset((char *) raster.bitmap, 0, raster.size);

            /*
             * Grid fit to determine the x and y offsets that will force the
             * bitmap to fit into the storage provided.
             */
            xoff = (63 - metrics.bbox.xMin) & -64;
            yoff = (63 - metrics.bbox.yMin) & -64;
            ymax = (63 - metrics.bbox.yMax) & -64;

            /*
             * If the bitmap cannot be generated, simply continue.
             */
            if (TT_Get_Glyph_Bitmap(glyph, &raster, xoff, yoff))
                continue;
        }

        /*
         * Determine the actual bounding box of the glyph bitmap.  Do not
         * forget that the glyph is rendered upside down!
         */
        sx = ey = 0xffff;
        sy = ex = 0;
        bmap = (unsigned char *) raster.bitmap;
        for (miny = 0; miny < raster.rows; miny++) {
            for (minx = 0; minx < raster.width; minx++) {
                if (bmap[(miny * raster.cols) + (minx >> 3)] &
                    (0x80 >> (minx & 7))) {
                    if (minx < sx)
                        sx = minx;
                    if (minx > ex)
                        ex = minx;
                    if (miny > sy)
                        sy = miny;
                    if (miny < ey)
                        ey = miny;
                }
            }
        }

        /*
         * If the glyph is actually an empty bitmap, set the size to 0 all
         * around.
         */
        if (sx == 0xffff && ey == 0xffff && sy == 0 && ex == 0) {
            sx = ex = sy = ey = 0;
            wd = 0;
            ht = 0;
        }
        else {
            wd = (ex - sx) + 1;
            ht = (sy - ey) + 1;
        }

        /*
        * Increment the number of glyphs generated.
        */
        ng++;

        /*
         * Adjust the font bounding box.
         */
        x_off = sx - (xoff >> 6);
        y_off = ey - (yoff >> 6);

        /*
         * Add to the average width accumulator.
         */
        if (wd > max_wd) max_wd = wd;
        aw += wd;

        g_glyph [code].metrics = (GLYPHMETRICS *) calloc (1, 
                        sizeof (GLYPHMETRICS));

        g_glyph [code].metrics->linestep = pitch = (raster.width + 7) >> 3; 
        g_glyph [code].metrics->height = ht;
        g_glyph [code].metrics->width = wd;
#if 1
        g_glyph [code].metrics->bearingx = -(xoff >> 6);
        g_glyph [code].metrics->bearingy = -(ymax >> 6);
#else
        g_glyph [code].metrics->bearingx = metrics.horiBearingX/64;
        g_glyph [code].metrics->bearingy = metrics.horiBearingY/64 - qmetrics.descent;
#endif
        g_glyph [code].metrics->advance = metrics.horiAdvance >> 6;

        g_glyph [code].data = (unsigned char *) calloc (raster.size, 
                        sizeof (BYTE));

        for (i = 0; i < ht; i++) {
            memcpy ((BYTE*)g_glyph [code].data + pitch * i, 
                            (BYTE*)raster.bitmap + raster.cols * i, 
                            raster.cols);
        }
    }
    
    /*
     * Calculate the average width.
     */
    aw = (TT_Long) ((((double) aw / (double) ng) + 0.5) * 10.0);

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
        WriteFreeType2QPF (fp);
        fclose (fp);
    }

    return 0;
}

int main (int argc, char *argv[])
{
    int res;
    char *infile, *outfile, *iname, *oname;
    FILE *out;

    if ((prog = strrchr(argv[0], '/')))
      prog++;
    else
      prog = argv[0];

    out = stdout;
    infile = outfile = 0;

    argc--;
    argv++;

    while (argc > 0) {
        if (argv[0][0] == '-') {
            switch (argv[0][1]) {
              case 'b': case 'B':
                donot_use_sbit = 1;
                break;
              case 'v': case 'V':
                verbose = 1;
                break;
              case 'n': case 'N':
                load_flags &= ~TTLOAD_HINT_GLYPH;
                break;
              case 'c': case 'C':
                argc--;
                argv++;
                spacing = argv[0][0];
                break;
              case 't': case 'T':
                argc--;
                argv++;
                face_name = argv[0];
                break;
              case 'w': case 'W':
                argc--;
                argv++;
                weight_name = argv[0];
                break;
              case 's': case 'S':
                argc--;
                argv++;
                slant_name = argv[0];
                break;
              case 'k': case 'K':
                argc--;
                argv++;
                width_name = argv[0];
                break;
              case 'd': case 'D':
                argc--;
                argv++;
                style_name = argv[0];
                break;
              case 'f': case 'F':
                argc--;
                argv++;
                foundry_name = argv[0];
                break;
              case 'u': case 'U':
                argc--;
                argv++;
                dashchar = argv[0][0];
                break;
              case 'p': case 'P':
                res = argv[0][2];
                argc--;
                argv++;
                if (res == 'i' || res == 'I')
                  /*
                   * Set the platform ID.
                   */
                  pid = atoi(argv[0]);
                else
                  /*
                   * Set the point size.
                   */
                  point_size = atoi(argv[0]);
                break;
              case 'e': case 'E':
                /*
                 * Set the encoding ID.
                 */
                argc--;
                argv++;
                eid = atoi(argv[0]);
                break;
              case 'r':
                /*
                 * Set the horizontal and vertical resolutions.
                 */
                if (argv[0][2] == 'h')
                  hres = atoi(argv[1]);
                else if (argv[0][2] == 'v')
                  vres = atoi(argv[1]);
                else
                  hres = vres = atoi(argv[1]);
                argc--;
                argv++;
                break;
              case 'o': case 'O':
                /*
                 * Set the output file name.
                 */
                argc--;
                argv++;
                outfile = argv[0];
                break;
              default:
                usage(1);
            }
        } else
          /*
           * Set the input file name.
           */
          infile = argv[0];

        argc--;
        argv++;
    }

    /*
     * Validate the values passed on the command line.
     */
    if (infile == 0) {
        fprintf(stderr, "%s: no input file provided.\n", prog);
        usage(1);
    }
    /*
     * Set the input filename that will be passed to the generator
     * routine.
     */
    if ((iname = strrchr(infile, '/')))
      iname++;
    else
      iname = infile;

    /*
     * Check the platform and encoding IDs.
     */
    if (pid < 0 || pid > 255) {
        fprintf(stderr, "%s: invalid platform ID '%d'.\n", prog, pid);
        exit(1);
    }
    if (eid < 0 || eid > 65535) {
        fprintf(stderr, "%s: invalid encoding ID '%d'.\n", prog, eid);
        exit(1);
    }

    /*
     * Arbitrarily limit the point size to a minimum of 2pt and maximum of
     * 256pt.
     */
    if (point_size < 2 || point_size > 256) {
        fprintf(stderr, "%s: invalid point size '%dpt'.\n", prog, point_size);
        exit(1);
    }

    /*
     * Arbitrarily limit the resolutions to a minimum of 10dpi and a maximum
     * of 1200dpi.
     */
    if (hres < 10 || hres > 1200) {
        fprintf(stderr, "%s: invalid horizontal resolution '%ddpi'.\n",
                prog, hres);
        exit(1);
    }
    if (vres < 10 || vres > 1200) {
        fprintf(stderr, "%s: invalid vertical resolution '%ddpi'.\n",
                prog, vres);
        exit(1);
    }

    /*
     * Open the output file if specified.
     */
    if (outfile != 0) {
        /*
         * Attempt to open the output file.
         */
        if ((out = fopen(outfile, "w")) == 0) {
            fprintf(stderr, "%s: unable to open the output file '%s'.\n",
                    prog, outfile);
            exit(1);
        }
        /*
         * Set the output filename to be passed to the generator routine.
         */
        if ((oname = strrchr(outfile, '/')))
          oname++;
        else
          oname = outfile;
    } else
      /*
       * Set the default output file name to <stdout>.
       */
      oname = "<stdout>";

    /*
     * Intialize Freetype.
     */
    if ((res = TT_Init_FreeType(&engine))) {
        /*
         * Close the output file.
         */
        if (out != stdout) {
            fclose(out);
            (void) unlink(outfile);
        }
        fprintf(stderr, "%s[%d]: unable to initialize renderer.\n",
                prog, res);
        exit(1);
    }

    TT_Init_SBit_Extension (engine);

    /*
     * Open the input file.
     */
    if ((res = TT_Open_Face(engine, infile, &face))) {
        if (out != stdout) {
            fclose(out);
            (void) unlink(outfile);
        }
        fprintf(stderr, "%s[%d]: unable to open input file '%s'.\n",
                prog, res, infile);
        exit(1);
    }

    /*
     * Create a new instance.
     */
    if ((res = TT_New_Instance(face, &instance))) {
        (void) TT_Close_Face(face);
        if (out != stdout) {
            fclose(out);
            (void) unlink(outfile);
        }
        fprintf(stderr, "%s[%d]: unable to create instance.\n",
                prog, res);
        exit(1);
    }

    /*
     * Set the instance resolution and point size and the relevant
     * metrics.
     */
    (void) TT_Set_Instance_Resolutions(instance, hres, vres);
#if 0
    (void) TT_Set_Instance_CharSize(instance, point_size*64);
    (void) TT_Get_Instance_Metrics(instance, &imetrics);
#else
    {
        TT_EBLC eblc;
        TT_SBit_Strike strike;

        if (donot_use_sbit == 0) {
            glyph_has_sbit = 0;
            if (TT_Get_Face_Bitmaps (face, &eblc) == TT_Err_Ok)
                glyph_has_sbit = 1;

            TT_Set_Instance_PixelSizes (instance, point_size, point_size, 0);
            TT_Get_Instance_Metrics (instance, &imetrics);

            if (TT_Get_SBit_Strike (face, instance, &strike))
                glyph_has_sbit = 0;
            else
                glyph_has_sbit = 1;
        }
        else {
            glyph_has_sbit = 0;
        }

        if (glyph_has_sbit) {
            TT_New_SBit_Image (&sbit);

            printf ("using embedded bitmap table\n");

            printf (" version of embedded bitmap table:  0x%lx\n", eblc.version);
            printf (" number of embedded bitmap strikes: %lu\n", eblc.num_strikes);

            printf ("%hux%hu pixels, %hu-bit depth, glyphs [%hu..%hu]\n",
                            strike.x_ppem, strike.y_ppem, strike.bit_depth,
                            strike.start_glyph, strike.end_glyph);
            {
                TT_SBit_Range*  range = strike.sbit_ranges;
                TT_SBit_Range*  limit = range + strike.num_ranges;

                for ( ; range < limit; range++ )
                    printf("      range format (%hu:%hu) glyphs %hu..%hu\n",
                        range->index_format,
                        range->image_format,
                        range->first_glyph,
                        range->last_glyph);
            }
            printf( "\n" );
        }
        else {
            printf ("Not using embedded bitmap table\n");
            (void) TT_Set_Instance_PixelSizes (instance, point_size, point_size, 0);
            (void) TT_Get_Instance_Metrics (instance, &imetrics);
        }
    }
#endif

    /*
     * Get the face properties and set the global units per em value for
     * convenience.
     */
    (void) TT_Get_Face_Properties(face, &properties);
    upm = properties.header->Units_Per_EM;

    /*
     * Create a new glyph container.
     */
    if ((res = TT_New_Glyph(face, &glyph))) {
        (void) TT_Done_Instance(instance);
        (void) TT_Close_Face(face);
        if (out != stdout) {
            fclose(out);
            (void) unlink(outfile);
        }
        fprintf(stderr, "%s[%d]: unable to create glyph.\n",
                prog, res);
        exit(1);
    }

    /*
     * Generate the QPF font from the TrueType font.
     */
    res = generate_qpf (out, iname, oname);

    /*
     * Free up the mapping table if one was loaded.
     */
    ttf2bdf_free_map();

    /*
     * Close the input and output files.
     */
    (void) TT_Close_Face(face);
    if (out != stdout) {
        fclose(out);
        if (res < 0)
          /*
           * An error occured when generating the font, so delete the
           * output file.
           */
          (void) unlink(outfile);
    }

    /*
     * Shut down the renderer.
     */
    (void) TT_Done_FreeType(engine);

    exit(res);

    return 0;
}

