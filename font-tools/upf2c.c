/*
** $Id: upf2c.c $
** 
** vi: tabstop=4:expandtab
**
**
** Copyright (C) 2005 ~ 2008, Feynman Software.
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
	FILE * tab_file =NULL;	
	FILE * c_file =NULL;	
	unsigned int cha;
	unsigned int n =0;
	char     upf_cfile[128],name[256]={0};
	char    *s;
	
	if (arg < 3) {
        fprintf (stderr, "Usage: upf2c <upffile> <name> [new_font_name]\n");
		fprintf (stderr, "./upf2c unifont_160_50.upf unifont_160_50\n");
		return 1;
	}

    strcpy(name, "__mgif_upf_");
    strcat(name, argv[2]);
    strcpy(upf_cfile, "upf_");
    strcat(upf_cfile, argv[1]);
 	s = strrchr ((const char *)upf_cfile, '.');
	
	if (s==NULL) {
        printf ("Please  specify  the 'upf' file\n");
        return 1;
    }

    *s ='\0';
    sprintf (s, "%s", ".c");

	tab_file = fopen (argv[1], "rb");

	if (tab_file == NULL) {
		printf ("Cant open the table file!\n");
		return 2;
	}
	
	c_file = fopen (upf_cfile, "w");
 	s = strrchr ((const char *)upf_cfile, '.');
    *s ='\0';
    fprintf (c_file, "/*\n");
    fprintf (c_file, "** In-core UPF file for %s.\n", name);
    fprintf (c_file, "**\n");
    fprintf (c_file, "** This file is created by 'upf2c' by FMSoft (http://www.fmsoft.cn).\n");
    fprintf (c_file, "** Please do not modify it manually.\n");
    fprintf (c_file, "**\n");
    fprintf (c_file, "** Copyright (C) 2009 Feynman Software\n");
    fprintf (c_file, "**\n");
    fprintf (c_file, "** All right reserved by Feynman Software.\n");
    fprintf (c_file, "**\n");
    fprintf (c_file, "*/\n");

    fprintf (c_file, "#include <stdio.h>\n");
    fprintf (c_file, "#include <stdlib.h>\n");
    fprintf (c_file, "\n");
    fprintf (c_file, "#include \"common.h\"\n");
    fprintf (c_file, "#include \"minigui.h\"\n");
    fprintf (c_file, "#include \"gdi.h\"\n");
    fprintf (c_file, "\n");
    fprintf (c_file, "#ifdef _MGFONT_UPF\n");
    fprintf (c_file, "\n");
    fprintf (c_file, "#include \"upf.h\"\n");
    fprintf (c_file, "\n");

	fprintf (c_file, "static unsigned char %s[] = {\n", upf_cfile);
	cha =0;

	while (1) {
		if (fread (&cha, 1, 1, tab_file)!=1) {
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

    fprintf (c_file,"UPFINFO %s = {\n", name);
    fprintf (c_file,"    0, 0, 0, 0, 0, 0, 0, 0, 0, {0, 0, 0},\n");
    fprintf (c_file,"    %s, \n    sizeof (%s)\n", upf_cfile, upf_cfile);
    fprintf (c_file,"};\n\n");

    fprintf (c_file,"#endif /* _MGFONT_UPF */\n");
	
	printf ("OK! translation is successful! there are %d bytes in the %s.c\n", n, upf_cfile);
	
	fclose (tab_file);
	fclose (c_file);
	return 0;
}
