/*
** $Id: upfmetrics: adjust the metrics of UPF font $
** 
** vi: tabstop=4:expandtab
**
** Copyright (C) 2009, FMSoft.
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
static void change_glyph_bbox (UPFINFO* upfinfo, unsigned int uc16, 
            int offx, int offy, int width, int height)
{
    UPFGLYPH* glyph;
    Uint8   * p_upf;
    
    p_upf = (Uint8 *)upfinfo->root_dir;

    glyph = upf_get_glyph (p_upf, (UPFNODE *)(((UPFV1_FILE_HEADER *)p_upf)->off_nodes + p_upf), 
                    uc16);

    if (glyph) {
        if (offx)
            glyph->bearingx += offx;
        if (offy)
            glyph->bearingy += offy;
        if (width >= 0)
            glyph->width = width;
        if (height >= 0)
            glyph->height = height;
    }
}

static const char* upffile = NULL;
static const char* fontname = NULL;
static int ascent = -1, descent = -1, height = -1, width = -1;

static void adjust_font_metrics (UPFINFO* upfinfo)
{
    UPFV1_FILE_HEADER* header = (UPFV1_FILE_HEADER*)upfinfo->root_dir;

    if (fontname)
        strcpy (header->font_name, fontname);
    if (width > 0)
        header->width = width;
    if (height > 0)
        header->height = height;
    if (ascent > 0)
        header->ascent = ascent;
    if (descent >= 0)
        header->descent = descent;

    printf ("Header information of UPF file after adjusting: %s\n", upffile);
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

static void usage (int eval)
{
    fprintf(stderr, "Usage: %s [options below] font.upf\n", prog);
    fprintf(stderr, "-w n\t\tSet the width of the font (default: -1, not change).\n");
    fprintf(stderr, "-h n\t\tSet the height of the font (default: -1, not change).\n");
    fprintf(stderr, "-a n\t\tSet the ascent of the font (default: -1, not change).\n");
    fprintf(stderr, "-d n\t\tSet the descent of the font (default: -1, not change).\n");
    fprintf(stderr, "-f s\t\tSet the new font name (default null, not change).\n");
    fprintf(stderr, "-n \t\tNo prompt.\n");
    exit(eval);
}

int main (int argc, char *argv[])
{
    int prompt = 1;
    UPFINFO* upfinfo;

    if ((prog = strrchr(argv[0], '/')))
      prog++;
    else
      prog = argv[0];

    argc--;
    argv++;

    while (argc > 0) {
        if (argv[0][0] == '-') {
            switch (argv[0][1]) {
              case 'w': case 'W':
                argc--;
                argv++;
                width = atoi (argv[0]);
                break;
              case 'h': case 'H':
                argc--;
                argv++;
                height = atoi (argv[0]);
                break;
              case 'a': case 'A':
                argc--;
                argv++;
                ascent = atoi (argv[0]);
                break;
              case 'd': case 'D':
                argc--;
                argv++;
                descent = atoi (argv[0]);
                break;
              case 'f': case 'F':
                argc--;
                argv++;
                fontname = argv[0];
                break;
              case 'n': case 'N':
                prompt = 0;
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

    printf ("%s: adjust metrics to be (w: %d, h: %d, a: %d, d: %d) and fontname: %s of UPF %s.\n", 
                prog, width, height, ascent, descent, fontname, upffile);
    if (prompt) {
        printf ("press <Ctrl+C> to exit or enter to continue...\n");
        fgetc (stdin);
    }

    upfinfo = upf_load_font_data (upffile);
    if (upfinfo == NULL)
        return 1;

    adjust_font_metrics (upfinfo);

    upf_unload_font_data (upfinfo);

    return 0;
}

#include "load_upf.c"

