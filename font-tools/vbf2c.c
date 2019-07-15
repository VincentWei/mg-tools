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

/*
** TODO:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define HEADER_LEN 68

typedef unsigned short Uint16;
typedef unsigned int   Uint32;

//typedef struct
//{
//    unsigned char   version;        /* font version */
//    const char*     name;           /* font name */
//    unsigned char   max_width;      /* max width in pixels */
//    unsigned char   ave_width;      /* average width in pixels */
//    int             height;         /* height in pixels */
//    int             descent;        /* pixels below the base line */
//    unsigned char   first_char;     /* first character in this font */
//    unsigned char   last_char;      /* last character in this font */
//    unsigned char   def_char;       /* default character in this font */
//    const unsigned short* offset;   /* character glyph offsets into bitmap data or NULL */
//    const unsigned char*  width;    /* character widths or NULL */
//    const unsigned char*  bits;     /* 8-bit right-padded bitmap data */
//    unsigned int    font_size;      /* used by mmap. It should be zero for in-core vbfs. */
//    int*            bbox_x;
//    int*            bbox_y;
//    int*            bbox_w;
//    int*            bbox_h;
//} OLD_VBFINFO;

/*#define LEN_VERSION_INFO    10

#define VBF_VERSION         "vbf-1.0**"
#define VBF_VERSION2        "vbf-2.0**"*/


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
    if ((temp = mmap (NULL, layout.font_size, PROT_READ, MAP_SHARED, 
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
        munmap ((void*)(info->name) - HEADER_LEN, info->font_size);
#else
    free ((void*)info->name);
#endif
}



/*static int LoadVarBitmapFont (const char* file, OLD_VBFINFO* info)
{
#ifdef HAVE_MMAP
    int fd;
#else
    FILE* fp;
#endif
    char* temp = NULL;
    int len_header, len_offsets, len_widths, len_bits, len_bboxs = 0;
    int font_size;
    char version[LEN_VERSION_INFO + 1];

#ifdef HAVE_MMAP
    if ((fd = open (file, O_RDONLY)) < 0)
        return FALSE;

    if (read (fd, version, LEN_VERSION_INFO) == -1)
        goto error;
    version[LEN_VERSION_INFO] = '\0';

    if (!strcmp (version, VBF_VERSION2))
        info->version = 2;
    else
    {
        info->version = 1;
        if (strcmp (version, VBF_VERSION))
            fprintf (stderr, "Error on loading vbf: %s, version: %s, invalid version.\n", file, version);
    }

    if (read (fd, &len_header, sizeof (int)) == -1)
        goto error;
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    len_header = ArchSwap32 (len_header);
#endif

    if (read (fd, &info->max_width, sizeof (char) * 2) == -1) goto error;
    if (read (fd, &info->height, sizeof (int) * 2) == -1) goto error;

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    info->height = ArchSwap32 (info->height);
    info->descent = ArchSwap32 (info->descent);
#endif

    if (read (fd, &info->first_char, sizeof (char) * 3) == -1) goto error;

    if (lseek (fd, len_header - (((info->version == 2)?5:4) * sizeof (int)), SEEK_SET) == -1)
        goto error;

    if (read (fd, &len_offsets, sizeof (int)) == -1
            || read (fd, &len_widths, sizeof (int)) == -1
            || (info->version == 2 && read (fd, &len_bboxs, sizeof (int)) == -1)
            || read (fd, &len_bits, sizeof (int)) == -1
            || read (fd, &font_size, sizeof (int)) == -1)
        goto error;
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    len_offsets = ArchSwap32 (len_offsets);
    len_widths = ArchSwap32 (len_widths);
    if (info->version == 2)
        len_bboxs = ArchSwap32 (len_bboxs);
    len_bits = ArchSwap32 (len_bits);
    font_size = ArchSwap32 (font_size);
#endif

    if ((temp = mmap (NULL, font_size, PROT_READ, MAP_SHARED, 
            fd, 0)) == MAP_FAILED)
        goto error;

    temp += len_header;
    close (fd);
#else
    // Open font file and read information of font.
    if (!(fp = fopen (file, "rb")))
        return FALSE;

    if (fread (version, sizeof (char), LEN_VERSION_INFO, fp) < LEN_VERSION_INFO)
        goto error;
    version [LEN_VERSION_INFO] = '\0'; 

    if (!strcmp (version, VBF_VERSION2))
        info->version = 2;
    else
    {
        info->version = 1;
        if (strcmp (version, VBF_VERSION))
            fprintf (stderr, "Error on loading vbf: %s, version: %s, invalid version.\n", file, version);
    }

    if (fread (&len_header, sizeof (int), 1, fp) < 1)
        goto error;
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    len_header = ArchSwap32 (len_header);
#endif

    if (fread (&info->max_width, sizeof (char), 2, fp) < 2) goto error;
    if (fread (&info->height, sizeof (int), 2, fp) < 2) goto error;
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    info->height = ArchSwap32 (info->height);
    info->descent = ArchSwap32 (info->descent);
#endif
    if (fread (&info->first_char, sizeof (char), 3, fp) < 3) goto error;

    if (fseek (fp, len_header - (((info->version == 2)?5:4)*sizeof (int)), SEEK_SET) != 0)
            goto error;

    if (fread (&len_offsets, sizeof (int), 1, fp) < 1
            || fread (&len_widths, sizeof (int), 1, fp) < 1
            || (info->version == 2 && fread (&len_bboxs, sizeof (int), 1, fp) < 1)
            || fread (&len_bits, sizeof (int), 1, fp) < 1
            || fread (&font_size, sizeof (int), 1, fp) < 1)
        goto error;
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    len_offsets = ArchSwap32 (len_offsets);
    len_widths = ArchSwap32 (len_widths);
    if (info->version == 2)
        len_bboxs = ArchSwap32 (len_bboxs);
    len_bits = ArchSwap32 (len_bits);
    font_size = ArchSwap32 (font_size);
#endif

    // Allocate memory for font data.
    font_size -= len_header;
    if ((temp = (char *)malloc (font_size)) == NULL)
        goto error;

    if (fseek (fp, len_header, SEEK_SET) != 0)
        goto error;

    if (fread (temp, sizeof (char), font_size, fp) < font_size)
        goto error;

    fclose (fp);
#endif
    info->name = temp;
    info->offset = (unsigned short*) (temp + LEN_DEVFONT_NAME + 1);
    info->width = (unsigned char*) (temp + LEN_DEVFONT_NAME + 1 + len_offsets);
    if (info->version == 2)
    {
        info->bbox_x = (int*) (temp + LEN_DEVFONT_NAME + 1 + len_offsets + len_widths);
        info->bbox_y = (int*) (temp + LEN_DEVFONT_NAME + 1 + len_offsets + len_widths + len_bboxs);
        info->bbox_w = (int*) (temp + LEN_DEVFONT_NAME + 1 + len_offsets + len_widths + 2*len_bboxs);
        info->bbox_h = (int*) (temp + LEN_DEVFONT_NAME + 1 + len_offsets + len_widths + 3*len_bboxs);
        info->bits = (unsigned char*) (temp + LEN_DEVFONT_NAME + 1 + len_offsets + len_widths + 4*len_bboxs);
    }
    else
        info->bits = (unsigned char*) (temp + LEN_DEVFONT_NAME + 1 + len_offsets + len_widths);
    info->font_size = font_size;

#if 0
    fprintf (stderr, "VBF: %s-%dx%d-%d (%d~%d:%d).\n", 
            info->name, info->max_width, info->height, info->descent,
            info->first_char, info->last_char, info->def_char);
#endif

    return len_bits;

error:
#ifdef HAVE_MMAP
    if (temp)
        munmap (temp, font_size);
    close (fd);
#else
    free (temp);
    fclose (fp);
#endif
    
    return 0;
}

static void UnloadVarBitmapFont (OLD_VBFINFO* info)
{
#ifdef HAVE_MMAP
    if (info->name)
        munmap ((void*)(info->name), info->font_size);
#else
    free ((void*)info->name);
#endif
}*/

/*static void dump_incore_offset (const OLD_VBFINFO* vbf_info)
{
    int ch;
    for (ch = vbf_info->first_char; ch <= vbf_info->last_char; ch++) {
        printf ("%d, \n", vbf_info->offset[ch]);
    }
}

static void dump_incore_width (const OLD_VBFINFO* vbf_info)
{
    int ch;
    for (ch = vbf_info->first_char; ch <= vbf_info->last_char; ch++) {
        printf ("%d, \n", vbf_info->width[ch]);
    }
}*/

static void dump_incore_bits (const VBFINFO* vbf_info)
{
    int i;
    const unsigned char *glyph_bits=vbf_info->all_glyph_bits;

    printf ("static const unsigned char all_glyph_bits [] = {\n");
    i =0;
    while(i < layout.len_bits) {
        printf ("0x%02x", *glyph_bits);
        i +=1;
        glyph_bits ++;
        if(i< layout.len_bits)
            printf(",");
        if (i%8 == 0)
            printf ("\n\t");
    }

    if(i%8 ==0)
        printf("};\n\n");
    else
        printf("\n};\n\n");
}

static void dump_incore_bbox (const VBFINFO* vbf_info)
{
    int i;
    const VBF_BBOX *vbf_bbox =vbf_info->bbox;

    printf("static const VBF_BBOX bbox[]={\n");
    i =0;
    while(i*sizeof(VBF_BBOX) < layout.len_bboxs){
        printf("{0x%02x,0x%02x,0x%02x,0x%02x}",vbf_bbox->x,vbf_bbox->y,vbf_bbox->w,vbf_bbox->h);
        vbf_bbox ++;
        i+=1;
        if(i*sizeof(VBF_BBOX) <layout.len_bboxs){
            printf(",");
        }
        if(i%2 ==0){
            printf("\n");
        }
    }

    if(i%2 ==0){
        printf("};\n\n");
    }
    else{
        printf("\n};\n\n");
    }
}


static void dump_incore_advance_x(const VBFINFO*  vbf_info)
{
    int i;
    const char *  advance_x =vbf_info->advance_x;

    printf("static const char advance_x[]={\n");
    i=0;
    while(i<layout.len_advxs){
        printf("0x%02x",*advance_x);
        advance_x ++;
        i +=1;
        if(i< layout.len_advxs)
            printf(",");
        if(i%8 ==0)
            printf("\n");
    }

    if(i%8 ==0)
        printf("};\n\n");
    else
        printf("\n};\n\n");
}


static void dump_incore_advance_y(const VBFINFO* vbf_info)
{
    int i;
    const char *advance_y =vbf_info->advance_y;

    printf("static const char advance_y[]={\n");
    i=0;
    while(i<layout.len_advys){
        printf("0x%02x",*advance_y);
        advance_y ++;
        i +=1;
        if(i< layout.len_advys)
            printf(",");
        if(i%8 ==0)
            printf("\n");
    }

    if(i%8 ==0)
        printf("};\n\n");
    else
        printf("\n};\n\n");
}

static void dump_incore_bits_offset(const VBFINFO *vbf_info)
{
    int i;
    const unsigned int *bits_offset =vbf_info->bits_offset;

    printf ("static const unsigned int bits_offset [] = {\n");
    i =0;
    while(i*4<layout.len_bit_offs){
        printf("0x%04x",*bits_offset);
        bits_offset ++;
        i +=1;
        if(i*4< layout.len_bit_offs)
            printf(",");
        if(i%8 ==0)
            printf("\n");
    }

    if(i%8 ==0)
        printf("};\n\n");
    else
        printf("\n};\n\n");
}

static void dumpVBF2C (const char* name, const VBFINFO* vbf_info, int len_bits)
{
    printf ("/*\n");
    printf ("** In-core VBF file for %s.\n", name);
    printf ("**\n");
    printf ("** This file is created by 'vbf2c' by FMSoft (http://www.fmsoft.cn).\n");
    printf ("** Please do not modify it manually.\n");
    printf ("**\n");
    printf ("** Copyright (C) 2007 FMSoft\n");
    printf ("**\n");
    printf ("** All right reserved by FMSoft.\n");
    printf ("**\n");
    printf ("*/\n");

    printf ("#include <stdio.h>\n");
    printf ("#include <stdlib.h>\n");
    printf ("\n");
    printf ("#include \"common.h\"\n");
    printf ("#include \"minigui.h\"\n");
    printf ("#include \"gdi.h\"\n");
    printf ("\n");
    printf ("#ifdef _MGFONT_VBF\n");
    printf ("\n");
    printf ("#include \"varbitmap.h\"\n");
    printf ("\n");

    if(vbf_info->bbox){
        dump_incore_bbox(vbf_info);
    }

    if(vbf_info->advance_x){
        dump_incore_advance_x(vbf_info);
    }

    if(vbf_info->advance_y){
        dump_incore_advance_y(vbf_info);
    }

    if (vbf_info->bits_offset) {
        
        dump_incore_bits_offset (vbf_info);
    }

    
    dump_incore_bits (vbf_info);

    /*if (vbf_info->version == 2) {
        printf ("static const unsigned char bbox_x [] = {\n");
        dump_incore_bbox (vbf_info, 1);
        printf ("};\n\n");

        printf ("static const unsigned char bbox_y [] = {\n");
        dump_incore_bbox (vbf_info, 2);
        printf ("};\n\n");

        printf ("static const unsigned char bbox_w [] = {\n");
        dump_incore_bbox (vbf_info, 3);
        printf ("};\n\n");

        printf ("static const unsigned char bbox_h [] = {\n");
        dump_incore_bbox (vbf_info, 4);
        printf ("};\n\n");
    }*/

    printf ("VBFINFO %s = {\n", name);
    printf ("\t\"%s\", \n", vbf_info->ver_info);
    printf ("\t\"%s\", \n", vbf_info->name);
    printf ("\t%d, \n", vbf_info->max_width);
    printf ("\t%d, \n", vbf_info->ave_width);
    printf ("\t%d, \n", vbf_info->height);
    printf ("\t%d, \n", vbf_info->descent);
    printf ("\t%d, \n", vbf_info->first_glyph);
    printf ("\t%d, \n", vbf_info->last_glyph);
    printf ("\t%d, \n", vbf_info->def_glyph);
    printf ("\t%d, \n", vbf_info->font_size);

    if (vbf_info->bbox)
        printf ("\tbbox, \n");
    else
        printf ("\tNULL, \n");

    if (vbf_info->advance_x)
        printf ("\tadvance_x, \n");
    else
        printf ("\tNULL, \n");

    if (vbf_info->advance_y)
        printf ("\tadvance_y, \n");
    else
        printf ("\tNULL, \n");

    if (vbf_info->bits_offset)
        printf ("\tbits_offset, \n");
    else
        printf ("\tNULL, \n");

    if (vbf_info->all_glyph_bits)
        printf ("\tall_glyph_bits, \n");
    else
        printf ("\tNULL, \n");

    printf ("};\n\n");

    printf ("#endif /* _MGFONT_VBF */\n");
}

int main (int argc, char **argv)
{
    int len_bits;
    char name [256];
    //OLD_VBFINFO vbf_info;
    VBFINFO  vbf_info;

    if (argc < 3) {
        fprintf (stderr, "Usage: vbf2c <vbffile> <name> > outputfilename\n");
        fprintf(stderr,"Example: ./vbf2c Helvetica-rr-15-16.vbf Helvetica1516 >vbf_helvetica1516.c \n");
        return 1;
    }

    if ((len_bits = LoadVarBitmapFont (argv[1], &vbf_info)) == 0) {
        fprintf (stderr, "vbf2c: can't load VBF file: %s\n", argv[1]);
        return 2;
    }
  
    strcpy (name, "__mgif_vbf_");
    strcat (name, argv[2]);

    dumpVBF2C (name, &vbf_info, len_bits);

    UnloadVarBitmapFont (&vbf_info);

    return 0;
}

