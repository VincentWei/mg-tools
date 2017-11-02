
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vbf2c.h"

#define HAVE_MMAP 1
//#ifdef HAVE_MMAP
    
    #include <sys/types.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
//#endif

#define HEADER_LEN 68

typedef unsigned short Uint16;
typedef unsigned int   Uint32;

typedef struct _VBF_BBOX
{
        char x, y, w, h;
} VBF_BBOX;

typedef struct
{
    char ver_info [4];  /* font version*/
    const char* name;         /* font name*/

    char max_width;     /* The max width in pixels. */
    char ave_width;     /* The average width in pixels. */
    char height;        /* The height in pixels. */
    char descent;       /* The pixels below the base line. */

    int first_glyph;    /* The glyph number of the first character in this font.*/
    int last_glyph;     /* The glyph number of the last character in this font.*/
    int def_glyph;      /* The glyph number of the default character in this font.*/

    unsigned int font_size; /* used by mmap. It should be zero for in-core vbfs. */

    const VBF_BBOX* bbox;     /* The pointer to the glyph bounding box array or NULL. */

    const char* advance_x;    /* The pointer to the glyph advance x array. 
                           If bbox is NULL, the array contains the width of each glyph.
                           If advance_x is NULL, the glyph has the universal width, 
                           that's max_width. */
    const char* advance_y;    /* The pointer to the glyph advance y array.
                           If bbox is NULL, advance_y should be NULL.
                           If bbox is not NULL and advance_y is NULL, 
                           all glyph has the universal advance_y, that is, zero. */

    const unsigned int* bits_offset;      /* The pointer to the glyph bitmap offset array 
                                       whose number will be used to fetch glyph bitmap 
                                       from bits array or NULL. */
    const unsigned char* all_glyph_bits;  /* The 8-bit right-padded bitmap data for all glyphs. */
} VBFINFO;

#define LEN_VERSION_INFO 10
#define LEN_VENDER_INFO  12

#define VBF_VERSION3       "vbf-3.0**"


typedef int BOOL;
#define TRUE    1
#define FALSE   0

#define MGUI_LIL_ENDIAN  1234
#define MGUI_BIG_ENDIAN  4321

/* Assume that the host is little endian */
#define MGUI_BYTEORDER   MGUI_LIL_ENDIAN

#define LEN_DEVFONT_NAME            79

/********************** Load/Unload of var bitmap font ***********************/
typedef struct _FILE_LAYOUT {
    Uint32 off_bboxs; 
    Uint32 len_bboxs; 

    Uint32 off_advxs; 
    Uint32 len_advxs; 

    Uint32 off_advys; 
    Uint32 len_advys; 

    Uint32 off_bit_offs; 
    Uint32 len_bit_offs; 

    Uint32 off_bits; 
    Uint32 len_bits; 

    Uint32 font_size; 
} FILE_LAYOUT;

static FILE_LAYOUT layout;
    

typedef struct _FONT_PROPT {
    char font_name [LEN_DEVFONT_NAME + 1];
    char max_width; 
    char ave_width; 
    char height; 
    char descent; 

    int first_glyph; 
    int last_glyph; 
    int def_glyph; 
} FONT_PROPT;

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
static void swap_intdata (Uint32* data, int num)
{
    while (num)
    {
        *data = ArchSwap32 (*data);
        data++;
        num--;
    }

}
#endif

static BOOL LoadVarBitmapFont (const char* file, VBFINFO* info)
{
    FILE* fp = NULL;

    char version[LEN_VERSION_INFO + 1];
    char vender[LEN_VENDER_INFO+1];

    //FILE_LAYOUT layout;
    const FONT_PROPT* propt;
    char* temp = NULL;
    Uint16 len_header;

    // Open font file and read information of font.
    if (!(fp = fopen (file, "rb")))
        return FALSE;

    if (fread (version, 1, LEN_VERSION_INFO, fp) < LEN_VERSION_INFO)
        goto error;
    version [LEN_VERSION_INFO] = '\0'; 

    if (strcmp (version, VBF_VERSION3) != 0)
    {
        fprintf (stderr, "Error on loading vbf: %s, version: %s, invalid version.\n", file, version);
        goto error;
    }

    if (fread(vender, 1, LEN_VENDER_INFO, fp) < LEN_VENDER_INFO)
        goto error;

    if (fread(&len_header, sizeof(short), 1, fp) < 1)
        goto error;

    if (fread (&layout, sizeof (FILE_LAYOUT), 1, fp) < 1)
        goto error;
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    len_header = ArchSwap16 (len_header);
    swap_intdata ((Uint32*)&layout, sizeof(layout)/sizeof(int));
#endif

#ifdef HAVE_MMAP
    if ((temp = (char*) mmap (NULL, layout.font_size, PROT_READ, MAP_SHARED, 
            fileno(fp), 0)) == MAP_FAILED)
        goto error;
    temp += len_header;
#else
    layout.font_size -= len_header;
    if ((temp = (char *)malloc (layout.font_size)) == NULL)
        goto error;
    if (fread (temp, sizeof (char), layout.font_size, fp) < layout.font_size)
        goto error;
#endif

    fclose (fp);
    propt = (const FONT_PROPT*) temp;

    strcpy (info->ver_info, "3.0");
    info->name = (char*)propt;
    info->max_width = propt->max_width;
    info->ave_width = propt->ave_width;
    info->height = propt->height;
    info->descent = propt->descent;

    info->font_size = layout.font_size;

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    info->first_glyph = ArchSwap32 (propt->first_glyph);
    info->last_glyph = ArchSwap32 (propt->last_glyph);
    info->def_glyph = ArchSwap32 (propt->def_glyph);
#else
    info->first_glyph = propt->first_glyph;
    info->last_glyph = propt->last_glyph;
    info->def_glyph = propt->def_glyph;
#endif

    info-> bbox = (const VBF_BBOX*)
            (layout.len_bboxs ? temp+layout.off_bboxs-HEADER_LEN : NULL);
    info-> advance_x = (const char*)
            (layout.len_advxs ? temp+layout.off_advxs-HEADER_LEN : NULL);
    info-> advance_y = (const char*)
            (layout.len_advys ? temp+layout.off_advys-HEADER_LEN : NULL);
    info-> bits_offset = (const Uint32*)
            (layout.len_bit_offs ? temp+layout.off_bit_offs-HEADER_LEN : NULL);

    if (layout.len_bits <= 0)
        goto error;
    info-> all_glyph_bits = (const unsigned char*)(temp + 
                            layout.off_bits-HEADER_LEN);

    return TRUE;

error:
#ifdef HAVE_MMAP
    if (temp)
        munmap (temp, layout.font_size);
#else
    free (temp);
#endif
    fclose (fp);
    return FALSE;
}

static void UnloadVarBitmapFont (VBFINFO* info)
{
#ifdef HAVE_MMAP
    /*FIXME*/
    if (info->name)
        munmap ((char*)(info->name) - HEADER_LEN, info->font_size);
#else
    free ((void*)info->name);
#endif
}

static void dump_incore_bits (FILE* fo,const char* name, const VBFINFO* vbf_info)
{
    int i;
    const unsigned char *glyph_bits=vbf_info->all_glyph_bits;

    fprintf (fo,"static const unsigned char %s_all_glyph_bits [] = {\n", name);
    i =0;
    while(i < layout.len_bits) {
        fprintf (fo,"0x%02hhx", *glyph_bits);
        i +=1;
        glyph_bits ++;
        if(i< layout.len_bits)
            fprintf(fo, ",");
        if (i%8 == 0)
            fprintf (fo,"\n\t");
    }

    if(i%8 ==0)
        fprintf(fo, "};\n\n");
    else
        fprintf(fo, "\n};\n\n");
}

static void dump_incore_bbox (FILE* fo, const char* name,const VBFINFO* vbf_info)
{
    int i;
    const VBF_BBOX *vbf_bbox =vbf_info->bbox;

    fprintf(fo,"static const VBF_BBOX %s_bbox[]={\n", name);
    i =0;
    while(i*sizeof(VBF_BBOX) < layout.len_bboxs){
        fprintf(fo,"{0x%02hhx,0x%02hhx,0x%02hhx,0x%02hhx}",vbf_bbox->x,vbf_bbox->y,vbf_bbox->w,vbf_bbox->h);
        vbf_bbox ++;
        i+=1;
        if(i*sizeof(VBF_BBOX) <layout.len_bboxs){
            fprintf(fo, ",");
        }
        if(i%2 ==0){
            fprintf(fo, "\n");
        }
    }

    if(i%2 ==0){
        fprintf(fo, "};\n\n");
    }
    else{
        fprintf(fo, "\n};\n\n");
    }
}


static void dump_incore_advance_x(FILE* fo,const char* name, const VBFINFO*  vbf_info)
{
    int i;
    const char *  advance_x =vbf_info->advance_x;

    fprintf(fo,"static const char %s_advance_x[]={\n",name);
    i=0;
    while(i<layout.len_advxs){
        fprintf(fo,"0x%02hhx",*advance_x);
        advance_x ++;
        i +=1;
        if(i< layout.len_advxs)
           fprintf(fo,",");
        if(i%8 ==0)
            fprintf(fo,"\n");
    }

    if(i%8 ==0)
        fprintf(fo,"};\n\n");
    else
        fprintf(fo,"\n};\n\n");
}


static void dump_incore_advance_y(FILE* fo, const char* name, const VBFINFO* vbf_info)
{
    int i;
    const char *advance_y =vbf_info->advance_y;

    fprintf(fo,"static const char %s_advance_y[]={\n",name);
    i=0;
    while(i<layout.len_advys){
        fprintf(fo,"0x%02x",*advance_y);
        advance_y ++;
        i +=1;
        if(i< layout.len_advys)
            fprintf(fo,",");
        if(i%8 ==0)
            fprintf(fo,"\n");
    }

    if(i%8 ==0)
        fprintf(fo,"};\n\n");
    else
        fprintf(fo,"\n};\n\n");
}

static void dump_incore_bits_offset(FILE* fo,const char* name, const VBFINFO *vbf_info)
{
    int i;
    const unsigned int *bits_offset =vbf_info->bits_offset;

    fprintf (fo,"static const unsigned int %s_bits_offset [] = {\n",name);
    i =0;
    while(i*4<layout.len_bit_offs){
        fprintf(fo,"0x%04x",*bits_offset);
        bits_offset ++;
        i +=1;
        if(i*4< layout.len_bit_offs)
            fprintf(fo,",");
        if(i%8 ==0)
            fprintf(fo,"\n");
    }

    if(i%8 ==0)
        fprintf(fo,"};\n\n");
    else
        fprintf(fo,"\n};\n\n");
}

static void dumpVBF2C (const char* outfile, const char* name, const VBFINFO* vbf_info, int len_bits)
{
	FILE* fo = fopen(outfile, "wt");
	if(fo == NULL)
		return ;
	
	fprintf(fo, "\n#include \"%s\"\n", CFGFILE);
	fprintf(fo, "\n#ifdef %s\n", MARCOR);

    fprintf (fo,"/*\n");
    fprintf (fo,"** In-core VBF file for %s.\n", name);
    fprintf (fo,"**\n");
    fprintf (fo,"** This file is created by 'vbf2c' by FMSoft (http://www.fmsoft.cn).\n");
    fprintf (fo,"** Please do not modify it manually.\n");
    fprintf (fo,"**\n");
    fprintf (fo,"** Copyright (C) 2007 Feynman Software\n");
    fprintf (fo,"**\n");
    fprintf (fo,"** All right reserved by Feynman Software.\n");
    fprintf (fo,"**\n");
    fprintf (fo,"*/\n");

	fprintf (fo,"\n#include \"varbitmap.h\"\n");

    if(vbf_info->bbox){
        dump_incore_bbox(fo,name, vbf_info);
    }

    if(vbf_info->advance_x){
        dump_incore_advance_x(fo,name, vbf_info);
    }

    if(vbf_info->advance_y){
        dump_incore_advance_y(fo,name, vbf_info);
    }

    if (vbf_info->bits_offset) {
        
        dump_incore_bits_offset (fo,name, vbf_info);
    }

    
    dump_incore_bits (fo,name, vbf_info);

    fprintf (fo,"VBFINFO %s[1] = {{\n", name);
    fprintf (fo,"\t\"%s\", \n", vbf_info->ver_info);
    fprintf (fo,"\t\"%s\", \n", vbf_info->name);
    fprintf (fo,"\t%d, \n", vbf_info->max_width);
    fprintf (fo,"\t%d, \n", vbf_info->ave_width);
    fprintf (fo,"\t%d, \n", vbf_info->height);
    fprintf (fo,"\t%d, \n", vbf_info->descent);
    fprintf (fo,"\t%d, \n", vbf_info->first_glyph);
    fprintf (fo,"\t%d, \n", vbf_info->last_glyph);
    fprintf (fo,"\t%d, \n", vbf_info->def_glyph);
    fprintf (fo,"\t%d, \n", vbf_info->font_size);

    if (vbf_info->bbox)
        fprintf (fo,"\t%s_bbox, \n",name);
    else
        fprintf (fo,"\tNULL, \n");

    if (vbf_info->advance_x)
        fprintf (fo,"\t%s_advance_x, \n",name);
    else
        fprintf (fo,"\tNULL, \n");

    if (vbf_info->advance_y)
        fprintf (fo,"\t%s_advance_y, \n",name);
    else
        fprintf (fo,"\tNULL, \n");

    if (vbf_info->bits_offset)
        fprintf (fo,"\t%s_bits_offset, \n",name);
    else
        fprintf (fo,"\tNULL, \n");

    if (vbf_info->all_glyph_bits)
        fprintf (fo,"\t%s_all_glyph_bits, \n",name);
    else
        fprintf (fo,"\tNULL, \n");

    fprintf (fo,"}};\n\n");

	fprintf (fo, "#endif // %s \n\n", MARCOR);

	fclose(fo);

}


bool VBF2C::translate(const char* infile, const char* outfile,const char* varname, const char** argv, int argc)
{
    int len_bits;
    VBFINFO vbf_info;

    if ((len_bits = LoadVarBitmapFont (infile, &vbf_info)) == 0) {
        fprintf (stderr, "vbf2c: can't load VBF file: %s\n", infile);
        return false;
    }
    
	dumpVBF2C (outfile, varname, &vbf_info, len_bits);

    UnloadVarBitmapFont (&vbf_info);

    return true;
}

const char* VBF2C::_support_list = "vbf\0";

