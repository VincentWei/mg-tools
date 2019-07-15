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

#define MAX_UCS     0xffff

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

/*
 * A generic routine to get a name from the TT name table.  This routine
 * always looks for English language names and checks three possibilities:
 * 1. English names with the MS Unicode encoding ID.
 * 2. English names with the MS unknown encoding ID.
 * 3. English names with the Apple Unicode encoding ID.
 *
 * The particular name ID mut be provided (e.g. nameID = 0 for copyright
 * string, nameID = 6 for Postscript name, nameID = 1 for typeface name.
 *
 * If the `dash' flag is non-zero, all dashes (-) in the name will be replaced
 * with the character passed.
 *
 * Returns the number of bytes added.
 */
static int ttf_get_english_name(char *name, int nameID, int dash)
{
    TT_UShort slen;
    int i, j, encid, nrec;
    unsigned short nrPlatformID, nrEncodingID, nrLanguageID, nrNameID;
    char *s;

    nrec = TT_Get_Name_Count(face);

    for (encid = 1, j = 0; j < 2; j++, encid--) {
        /*
         * Locate one of the MS English font names.
         */
        for (i = 0; i < nrec; i++) {
            TT_Get_Name_ID(face, i, &nrPlatformID, &nrEncodingID,
                           &nrLanguageID, &nrNameID);
            if (nrPlatformID == 3 &&
                nrEncodingID == encid &&
                nrNameID == nameID &&
                (nrLanguageID == 0x0409 || nrLanguageID == 0x0809 ||
                 nrLanguageID == 0x0c09 || nrLanguageID == 0x1009 ||
                 nrLanguageID == 0x1409 || nrLanguageID == 0x1809)) {
                TT_Get_Name_String(face, i, &s, &slen);
                break;
            }
        }

        if (i < nrec) {
            /*
             * Found one of the MS English font names.  The name is by
             * definition encoded in Unicode, so copy every second byte into
             * the `name' parameter, assuming there is enough space.
             */
            for (i = 1; s != 0 && i < slen; i += 2) {
                if (dash)
                  *name++ = (s[i] == '-' || s[i] == ' ') ? dash : s[i];
                else if (s[i] == '\r' || s[i] == '\n') {
                    if (s[i] == '\r' && i + 2 < slen && s[i + 2] == '\n')
                      i += 2;
                    *name++ = ' ';
                    *name++ = ' ';
                } else
                  *name++ = s[i];
            }
            *name = 0;
            return (slen >> 1);
        }
    }

    /*
     * No MS English name found, attempt to find an Apple Unicode English
     * name.
     */
    for (i = 0; i < nrec; i++) {
        TT_Get_Name_ID(face, i, &nrPlatformID, &nrEncodingID,
                       &nrLanguageID, &nrNameID);
        if (nrPlatformID == 0 && nrLanguageID == 0 &&
            nrNameID == nameID) {
            TT_Get_Name_String(face, i, &s, &slen);
            break;
        }
    }

    if (i < nrec) {
        /*
         * Found the Apple Unicode English name.  The name is by definition
         * encoded in Unicode, so copy every second byte into the `name'
         * parameter, assuming there is enough space.
         */
        for (i = 1; s != 0 && i < slen; i += 2) {
            if (dash)
              *name++ = (s[i] == '-' || s[i] == ' ') ? dash : s[i];
            else if (s[i] == '\r' || s[i] == '\n') {
                if (s[i] == '\r' && i + 2 < slen && s[i + 2] == '\n')
                  i += 2;
                *name++ = ' ';
                *name++ = ' ';
            } else
              *name++ = s[i];
        }
        *name = 0;
        return (slen >> 1);
    }

    return 0;
}

#define SET_WEIGHT_POS     0
#define SET_SLANT_POS      1
#define SET_SETWIDTH_POS   2

static void set_rrncnn (char* rrncnn, int set_pos, char* value)
{
    switch (set_pos)
    {
        case SET_WEIGHT_POS:
            if (value == NULL)
                rrncnn[SET_WEIGHT_POS] = 'r';
            else if (strcasecmp(value, "black") == 0)
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
            if (value == NULL)
                rrncnn[SET_SLANT_POS] = 'r';
            else if (strcasecmp(value, "i") == 0)
                rrncnn[SET_SLANT_POS] = 'i';
            else if (strcasecmp(value, "o") == 0)
                rrncnn[SET_SLANT_POS] = 'o';
            else if (strcasecmp(value, "r") == 0)
                rrncnn[SET_SLANT_POS] = 'r';
            break;

        case SET_SETWIDTH_POS:
            if (value == NULL)
                rrncnn[SET_SETWIDTH_POS] = 'n';
            else if (strcasecmp(value, "bold") == 0)
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

#include "upf.h"

#define LEN_PADD_BMP_BUFF   4

static UPFGLYPH g_upf_glyphs [MAX_UCS];
static UPFV1_FILE_HEADER g_upf_file_hdr;
static unsigned char* g_buff_bitmap;
static unsigned int g_len_buff_bitmap;
static char g_font_rrncnn [7] = "rrncnn";

static int count_glyph_tree_nodes (void* ctxt, UPFGLYPHTREE *node)
{
    unsigned int code;
    UPFV1_FILE_HEADER* file_hdr = (UPFV1_FILE_HEADER*) ctxt;

    if (file_hdr) {
        file_hdr->nr_zones++;

        for (code = node->min; code <= node->max; code++) {
            g_upf_glyphs [code].bitmap_offset -= LEN_PADD_BMP_BUFF;
        }
    }

    return 0;
}

static int generate_upf (FILE *out, char *iname, char *oname)
{
    TT_Long i;
    TT_UShort p, e;

    TT_Short minx, miny, maxx, maxy, xoff, yoff, ymax;
    TT_Short y_off, x_off;
    TT_UShort sx, sy, ex, ey, wd, ht, max_wd = 0;
    TT_Long code, idx, ng, aw;
    int pitch, size_bitmap, offset_bitmap;
    unsigned char *bmap;
    UPFGLYPHTREE* root;
    char name [LEN_DEVFONT_NAME + 1];

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
    }

    /* initialize glyph bitmap buffer */
    g_len_buff_bitmap = LEN_PADD_BMP_BUFF;
    g_buff_bitmap = realloc (g_buff_bitmap, g_len_buff_bitmap);
    if (g_buff_bitmap == NULL)
        return -1;

    /* initialize file header */
    memset (g_upf_glyphs, 0, sizeof (g_upf_glyphs));
    memset (&g_upf_file_hdr, 0, sizeof (UPFV1_FILE_HEADER));

    strcpy (g_upf_file_hdr.ver_info, "UPF1.0**");

    if (foundry_name)
        strncpy (g_upf_file_hdr.vender_name, foundry_name, LEN_VENDER_NAME - 1);
    else
        strncpy (g_upf_file_hdr.vender_name, "FMSoft", LEN_VENDER_NAME - 1);
    g_upf_file_hdr.vender_name [LEN_VENDER_NAME - 1] = '\0';

    g_upf_file_hdr.endian = 0x1234;

    g_upf_file_hdr.width = 0; /* calculate later */
    g_upf_file_hdr.height = point_size;
    g_upf_file_hdr.ascent = (((properties.horizontal->Ascender * \
                                imetrics.y_scale)/0x10000) >> 6) + 1;
    g_upf_file_hdr.descent = (((properties.horizontal->Descender * \
                                imetrics.y_scale)/0x10000) >> 6) + 1;
    g_upf_file_hdr.descent = -g_upf_file_hdr.descent;

    g_upf_file_hdr.max_width = 0; /* calculate later */
    g_upf_file_hdr.min_width = 128; /* calculate later */
    g_upf_file_hdr.left_bearing = 0;
    g_upf_file_hdr.right_bearing = 0;
    g_upf_file_hdr.underline_pos = 0;
    g_upf_file_hdr.underline_width = 0;
    g_upf_file_hdr.leading = 0;
    g_upf_file_hdr.mono_bitmap = 1;

    g_upf_file_hdr.nr_glyph = 0;
    g_upf_file_hdr.font_size = 0;

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

        g_upf_glyphs [code].height = ht;
        g_upf_glyphs [code].width = wd;
        g_upf_glyphs [code].bearingx = -(xoff >> 6);
        g_upf_glyphs [code].bearingy = -(ymax >> 6);

        pitch = (raster.width + 7) >> 3; 
        size_bitmap = pitch * ht;

        g_upf_glyphs [code].pitch = pitch;
        g_upf_glyphs [code].padding1 = 0;
        g_upf_glyphs [code].advance = metrics.horiAdvance >> 6;
        g_upf_glyphs [code].padding2 = 0;

        g_buff_bitmap = realloc (g_buff_bitmap, g_len_buff_bitmap + size_bitmap);
        if (g_buff_bitmap == NULL)
            return -1;

        offset_bitmap = g_len_buff_bitmap;
        g_len_buff_bitmap += size_bitmap;
        for (i = 0; i < ht; i++) {
            memcpy (g_buff_bitmap + offset_bitmap + pitch * i, 
                            (BYTE*)raster.bitmap + raster.cols * i, 
                            pitch);
        }
        g_upf_glyphs [code].bitmap_offset = offset_bitmap;

        if (g_upf_file_hdr.max_width < g_upf_glyphs [code].advance)
            g_upf_file_hdr.max_width = g_upf_glyphs [code].advance;
        if (g_upf_file_hdr.min_width > g_upf_glyphs [code].advance)
            g_upf_file_hdr.min_width = g_upf_glyphs [code].advance;

        aw += g_upf_glyphs [code].advance;
        g_upf_file_hdr.nr_glyph ++;
    }
    
    /*
     * Calculate the average width.
     */
    aw = (TT_Long) ((((double) aw / (double) ng) + 0.5) * 10.0);
    g_upf_file_hdr.width = aw/10;

    /*
     * generate font_name.
     */
    if (face_name == 0) {
        if (ttf_get_english_name (name, TTF_TYPEFACE, dashchar))
            face_name = name;
        else {
            face_name = "unknown";
        }
    }
    strcpy (g_font_rrncnn, "rrncnn");
    set_rrncnn (g_font_rrncnn, SET_WEIGHT_POS, weight_name);
    set_rrncnn (g_font_rrncnn, SET_SLANT_POS, slant_name);
    set_rrncnn (g_font_rrncnn, SET_SETWIDTH_POS, width_name);
    sprintf (g_upf_file_hdr.font_name, "upf-%s-%s-%d-%d-%s", 
                face_name, g_font_rrncnn, g_upf_file_hdr.width, point_size, 
                "UTF-8,UTF-16LE,UTF-16BE");

    /*
     * Build the glyph tree.
     */
    if ((root = upf_make_glyph_tree (g_upf_glyphs)) == NULL) {
        fprintf (stderr, "%s: error occured when making glyph tree: %s.\n", prog, iname);
        return -1;
    }
    
    /* other important fields */
    g_upf_file_hdr.nr_zones = 0;
    upf_travel_glyph_tree (root, count_glyph_tree_nodes, &g_upf_file_hdr);

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

    if (verbose) {
        printf ("%s: the number of glyphs: %d\n", prog, g_upf_file_hdr.nr_glyph);
        printf ("%s: the number of tree nodes: %d\n", prog, g_upf_file_hdr.nr_zones);
    }

    if (upf_generate_upf_file (oname, &g_upf_file_hdr, root, g_buff_bitmap + LEN_PADD_BMP_BUFF)) {
        fprintf (stderr, "%s: error occured when generating UPF file: %s->%s.\n", 
                prog, iname, oname);
        upf_delete_glyph_tree (root);
        return -1;
    }
    
    upf_delete_glyph_tree (root);
    free (g_buff_bitmap);
    g_buff_bitmap = NULL;

    return 0;
}

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
     * Generate the UPF font from the TrueType font.
     */
    res = generate_upf (out, iname, oname);

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

