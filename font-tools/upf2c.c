/*
** $Id: upf2c.c $
**
** vi: tabstop=4:expandtab
**
**
** Copyright (C) 2005 ~ 2008, FMSoft.
**
** Create date: 2008/01/26
*/
#include <stdio.h>
#include <string.h>


#define LEN_UNIDEVFONT_NAME         127
typedef unsigned char Uint8;
typedef char Sint8;
typedef unsigned int    Uint32;

typedef struct
{
    char       font_name [LEN_UNIDEVFONT_NAME + 1];

    Uint8      width;
    Uint8      height;
    Sint8      ascent;
    Sint8      descent;
    Uint8      max_width;
    Uint8      underline_pos;
    Uint8      underline_width;
    Sint8      leading;
    Uint8      mono_bitmap;
    Uint8      reserved[3];
    Uint32     root_dir;
    Uint32     file_size;
} UPFINFO;


int main (int arg, char *argv[])
{
    FILE * font_file =NULL;
    FILE * c_file =NULL;
    unsigned int cha;
    unsigned int n =0;
    char     upf_cfile[128],name[256]={0};
    char    *s;

    if (arg < 3) {
        fprintf (stderr, "Usage: upf2c <upffile> <name>\n");
        fprintf (stderr, "./upf2c unifont_160_50.upf unifont_160_50\n");
        return 1;
    }

    strcpy(name, "_incore_font_upf_");
    strcat(name, argv[2]);
    s = strrchr ((const char *)argv[1], '.');
    if (s == NULL) {
        printf ("Please specify a 'upf' file\n");
        return 1;
    }

    font_file = fopen (argv[1], "rb");
    if (font_file == NULL) {
        printf ("Cant open the font file: %s\n", argv[1]);
        return 2;
    }

    strcpy(upf_cfile, argv[2]);
    strcat(upf_cfile, ".c");
    c_file = fopen (upf_cfile, "w");
    if (c_file == NULL) {
        printf ("Cant open the C file: %s\n", upf_cfile);
        return 2;
    }

    fprintf (c_file, "/*\n");
    fprintf (c_file, "** In-core UPF file for %s.\n", name);
    fprintf (c_file, "**\n");
    fprintf (c_file, "** This file is created by 'upf2c'\n");
    fprintf (c_file, "** Please do not modify it manually.\n");
    fprintf (c_file, "**\n");
    fprintf (c_file, "*/\n");
    fprintf (c_file, "#include <minigui/common.h>\n");
    fprintf (c_file, "#include <minigui/minigui.h>\n");
    fprintf (c_file, "#include <minigui/gdi.h>\n");
    fprintf (c_file, "\n");
    fprintf (c_file, "#ifdef _MGFONT_UPF\n");
    fprintf (c_file, "\n");
    fprintf (c_file, "typedef struct\n");
    fprintf (c_file, "{\n");
    fprintf (c_file, "    Uint8      width;\n");
    fprintf (c_file, "    Uint8      height;\n");
    fprintf (c_file, "    Sint8      ascent;\n");
    fprintf (c_file, "    Sint8      descent;\n");
    fprintf (c_file, "    Uint8      max_width;\n");
    fprintf (c_file, "    Uint8      underline_pos;\n");
    fprintf (c_file, "    Uint8      underline_width;\n");
    fprintf (c_file, "    Sint8      leading;\n");
    fprintf (c_file, "    Uint8      mono_bitmap;\n");
    fprintf (c_file, "    Uint8      reserved[3];\n");
    fprintf (c_file, "    void*      root_dir;\n");
    fprintf (c_file, "    Uint32     file_size;\n");
    fprintf (c_file, "} UPFINFO;\n");
    fprintf (c_file, "\n");

    fprintf (c_file, "static const unsigned char font_data[] = {\n");
    cha =0;

    while (1) {
        if (fread (&cha, 1, 1, font_file)!=1) {
            fprintf (c_file, "0x%02x", 0 );
            break;
        }

        fprintf (c_file, "0x%02x", cha);
        fprintf (c_file, ", ");

        if (++n % 10 == 0)
            fprintf (c_file, "\n");

        cha =0;
    }
    fprintf (c_file, "};\n\n" );

    fprintf (c_file,"const UPFINFO %s = {\n", name);
    fprintf (c_file,"    0, 0, 0, 0, 0, 0, 0, 0, 0, {0, 0, 0},\n");
    fprintf (c_file,"    font_data, sizeof (font_data)\n");
    fprintf (c_file,"};\n\n");

    fprintf (c_file,"#endif /* _MGFONT_UPF */\n");

    printf ("OK! translation is successful! there are %d bytes in the %s.c\n", n, upf_cfile);

    fclose (font_file);
    fclose (c_file);
    return 0;
}
