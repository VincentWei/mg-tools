/*
** $Id: upfdump: dump the information of UPF font $
** 
** vi: tabstop=4:expandtab
**
** Copyright (C) 2009, Feynman Software.
**
** Create date: 2009/12/09 WEI Yongming
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

static char *prog;
static char *upffile = NULL;

static void print_font_info (UPFINFO* upfinfo)
{
    const UPFV1_FILE_HEADER* header = (const UPFV1_FILE_HEADER*)upfinfo->root_dir;

    printf ("** Font information of file %s:\n", upffile);
    printf ("\tver_info: %s\n", header->ver_info);
    printf ("\tvender_name: %s\n", header->vender_name);
    printf ("\tendian: %x\n", header->endian);
    printf ("\tfont_name: %s\n", header->font_name);

    printf ("\twidth: %d\n", header->width);
    printf ("\theight: %d\n", header->height);
    printf ("\tascent: %d\n", header->ascent);
    printf ("\tdescent: %d\n", header->descent);

    printf ("\tmax_width: %d\n", header->max_width);
    printf ("\tmin_width: %d\n", header->min_width);
    printf ("\tleft_bearing: %d\n", header->left_bearing);
    printf ("\tright_bearing: %d\n", header->right_bearing);

    printf ("\tunderline_pos: %d\n", header->underline_pos);
    printf ("\tunderline_width: %d\n", header->underline_width);
    printf ("\tleading: %d\n", header->leading);
    printf ("\tmono_bitmap: %d\n", header->mono_bitmap);

    printf ("\tnr_glyph: %d\n", header->nr_glyph);
    printf ("\tfont_size: %d\n", header->font_size);
    printf ("** End of font information\n");
}

static int max_bearingy;
static int max_advance;

static BOOL print_glyph_bbox (UPFINFO* upfinfo, unsigned int uc16)
{
    UPFGLYPH* glyph;
    Uint8   * p_upf;
    
    p_upf = (Uint8 *)upfinfo->root_dir;

    glyph = upf_get_glyph (p_upf, (UPFNODE *)(((UPFV1_FILE_HEADER *)p_upf)->off_nodes + p_upf), uc16);

    if (glyph) {
        printf ("** BBox information of glyph %x:\n", uc16);
        printf ("\tBBox: (%dx%d) at (%d, %d), advance: %d.\n", 
                glyph->width, glyph->height, 
                glyph->bearingx, glyph->bearingy,
                glyph->advance);

        if (glyph->bearingy > max_bearingy)
            max_bearingy = glyph->bearingy;
        if (glyph->advance > max_advance)
            max_advance = glyph->advance;
        printf ("** End of bbox information of glyph %x:\n", uc16);
        return TRUE;
    }

    return FALSE;
}

static void usage (int eval)
{
    fprintf(stderr, "Usage: %s [options below] font.upf\n", prog);
    fprintf(stderr, "-h\t\tThis message.\n");
    fprintf(stderr, "-g n1 n2\tSet glyph range (default: all).\n");
    exit(eval);
}

int main (int argc, char *argv[])
{
    UPFINFO* upfinfo;
    unsigned int n, n1, n2, nr_glyphs;

    n1 = 0;
    n2 = 0xFFFE;

    if ((prog = strrchr(argv[0], '/')))
      prog++;
    else
      prog = argv[0];

    argc--;
    argv++;

    while (argc > 0) {
        if (argv[0][0] == '-') {
            switch (argv[0][1]) {
              case 'g': case 'G':
                argc--;
                argv++;
                n1 = atoi (argv[0]);
                argc--;
                argv++;
                n2 = atoi (argv[0]);
                break;
              default:
                usage(1);
            }
        } else
          /*
           * Set the input file name.
           */
          upffile = argv[0];

        argc--;
        argv++;
    }

    /*
     * Validate the values passed on the command line.
     */
    if (upffile == NULL) {
        fprintf(stderr, "%s: no input file provided.\n", prog);
        usage(1);
    }

    if (n2 < n1) n2 = n1;

    printf ("%s, print UPF font metrics and bbox from glyph %d to %d in UPF %s.\n", 
                prog, n1, n2, upffile);

    upfinfo = upf_load_font_data (upffile);
    if (upfinfo == NULL)
        return 1;

    print_font_info (upfinfo);

    nr_glyphs = 0;
    for (n = n1; n <= n2; n++) {
        if (print_glyph_bbox (upfinfo, n))
            nr_glyphs++;
    }

    printf ("%s, number of total glyphs in UPF %s: %d.\n", 
                prog, upffile, nr_glyphs);
    printf ("%s, max of bearing Y and advance: %d, %d in UPF %s.\n", 
                prog, max_bearingy, max_advance, upffile);
    
    upf_unload_font_data (upfinfo);

    return 0;
}

#include "load_upf.c"

