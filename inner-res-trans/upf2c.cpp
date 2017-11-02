
#include <stdio.h>
#include <stdlib.h>

#include "upf2c.h"

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
    Uint8      reserve[3];
    Uint32     root_dir;
    Uint32     file_size;
} UPFINFO;


bool UPF2C::translate(const char* infile, const char* outfile,const char* varname, const char** argv, int argc)
{
    FILE * tab_file =NULL;  
    FILE * c_file =NULL;    
    unsigned int cha;
    unsigned int n =0;
    char    *s;

    tab_file =fopen (infile,"rb");

    if (tab_file==NULL)
    {
        fprintf (stderr,"Cant open the table file!\n");
        return false;
    }

    c_file =fopen (outfile,"wt");
	fprintf (c_file, "\n#include \"%s\"\n", CFGFILE);	
	fprintf (c_file, "\n#ifdef %s\n\n", MARCOR);
    fprintf (c_file, "/*\n");
    fprintf (c_file, "** In-core UPF file for %s.\n", varname);
    fprintf (c_file, "**\n");
    fprintf (c_file, "** This file is created by 'upf2c' by FMSoft (http://www.fmsoft.cn).\n");
    fprintf (c_file, "** Please do not modify it manually.\n");
    fprintf (c_file, "**\n");
    fprintf (c_file, "** Copyright (C) 2009 Feynman Software\n");
    fprintf (c_file, "**\n");
    fprintf (c_file, "** All right reserved by Feynman Software.\n");
    fprintf (c_file, "**\n");
    fprintf (c_file, "*/\n");

    fprintf (c_file, "\n");

	fprintf (c_file, "#include \"upf.h\"\n");

    fprintf (c_file, "static unsigned char %s_info[] = {\n",varname );
    cha =0;

    while (1)
    {

        if (fread (&cha,1,1,tab_file)!=1)
        {
            fprintf (c_file, "0x%02hhx", 0 );
            break;
        }

        fprintf (c_file, "0x%02hhx",cha );
        fprintf (c_file, ", " );

        if (++n%10==0)
            fprintf (c_file, "\n");

        cha =0;
    }
    fprintf (c_file, "};\n\n" );

    fprintf (c_file,"UPFINFO %s[1] = {{\n", varname);
    fprintf (c_file,"\t0, 0, 0, 0, 0, 0, 0, 0, 0, {0, 0, 0}, \n");
    fprintf (c_file,"\t%s_info,\n\t%d \n", varname,n);
    fprintf (c_file,"}};\n\n");

	fprintf (c_file, "#endif // %s\n\n", MARCOR);

    fclose (tab_file);
    fclose (c_file);
    return true;
}



const char* UPF2C::_support_list = "upf\0";
