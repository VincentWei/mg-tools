/*
** $Id: upfbbox: adjust the bbox of UPF glyph $
** 
** vi: tabstop=4:expandtab
**
** Copyright (C) 2009, Feynman Software.
**
** Create date: 2009/12/01 WEI Yongming
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

static void usage (int eval)
{
    fprintf(stderr, "Usage: %s [options below] font.upf\n", prog);
    fprintf(stderr, "-x n\t\tSet the offset of bearing X (default: 0).\n");
    fprintf(stderr, "-y n\t\tSet the offset of bearing Y (default: 0).\n");
    fprintf(stderr, "-w n\t\tSet the width (default: -1, not change).\n");
    fprintf(stderr, "-h n\t\tSet the height (default: -1, not change).\n");
    fprintf(stderr, "-g n1 n2\tSet glyph range (default: all).\n");
    fprintf(stderr, "-n \t\tNo prompt.\n");
    exit(eval);
}

int main (int argc, char *argv[])
{
    int prompt = 1;
    UPFINFO* upfinfo;
    char *upffile = NULL;
    int offx, offy, width, height;
    unsigned int n, n1, n2;

    offx = offy = 0;
    width = height = -1;
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
              case 'x': case 'X':
                argc--;
                argv++;
                offx = atoi (argv[0]);
                break;
              case 'y': case 'Y':
                argc--;
                argv++;
                offy = atoi (argv[0]);
                break;
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
              case 'g': case 'G':
                argc--;
                argv++;
                n1 = atoi (argv[0]);
                argc--;
                argv++;
                n2 = atoi (argv[0]);
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

    if (n2 < n1) n2 = n1;

    printf ("%s: adjust bbox (+%d, +%d, %d, %d) from glyph %d to %d in UPF %s.\n", 
                prog, offx, offy, width, height, n1, n2, upffile);
    if (prompt) {
        printf ("press <Ctrl+C> to exit or enter to continue...\n");
        fgetc (stdin);
    }

    upfinfo = upf_load_font_data (upffile);
    if (upfinfo == NULL)
        return 1;

    for (n = n1; n <= n2; n++) {
        change_glyph_bbox (upfinfo, n, offx, offy, width, height);
    }

    upf_unload_font_data (upfinfo);

    return 0;
}

#include "load_upf.c"

