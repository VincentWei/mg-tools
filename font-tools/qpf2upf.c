/*
** $Id: qpf2toupf.c  mgwang $
** 
** vi: tabstop=4:expandtab
**
**
** Copyright (C) 2005 ~ 2008, FMSoft.
**
** Create date: 2008/01/26 mgwang
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "qpf.h"
#include "upf.h"

#define QPF_FLAG_MODE_SMOOTH 1

static unsigned off_nodes   =0;
static unsigned off_glyphs  =0;
static unsigned off_bitmaps =0;


//static unsigned len_nodes   =0;
//static unsigned len_glyphs  =0;
static unsigned len_bitmaps =0;

#undef  REVISE_WIDTH_HEIGHT

static unsigned nr_nodes   =0;
static unsigned nr_glyphs  =0;

void help (void)
{
    printf ("################USAGE######################\n");
    printf ("COMMANDLINE :qpf2upf filename fontname  \n");
    printf ("EXAMPLE:\n");
    printf ("\t qpf2upf unifont_160_50.qpf upf-unifont-rrncnn-16-16-ISO8859-1,ISO8859-15,GB2312-0,GBK,BIG5,UTF-8\n");
}

static void readNode (QPFGLYPHTREE* tree, Uint8** data)
{
    Uint8 rw, cl;
    int flags, n;

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    tree->min = (rw << 8) | cl;

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    tree->max = (rw << 8) | cl;

    flags = **data; (*data)++;
    if (flags & 1)
        tree->less = (QPFGLYPHTREE*)calloc (1, sizeof (QPFGLYPHTREE));
    else
        tree->less = NULL;

    if (flags & 2)
        tree->more = (QPFGLYPHTREE*)calloc (1, sizeof (QPFGLYPHTREE));
    else
        tree->more = NULL;

    n = tree->max - tree->min + 1;
    tree->glyph = calloc (n, sizeof (QPFGLYPH));

    if (tree->less)
        readNode (tree->less, data);
    if (tree->more)
        readNode (tree->more, data);
}

static void readMetrics (QPFGLYPHTREE* tree, Uint8** data)
{
    int i;
    int n = tree->max - tree->min + 1;

    for (i = 0; i < n; i++) {
        tree->glyph[i].metrics = (QPFGLYPHMETRICS*) *data;

        *data += sizeof (QPFGLYPHMETRICS);
    }

    if (tree->less)
        readMetrics (tree->less, data);
    if (tree->more)
        readMetrics (tree->more, data);
}
static void readData (QPFGLYPHTREE* tree, Uint8** data)
{
    int i;
    int n = tree->max - tree->min + 1;

    for (i = 0; i < n; i++) {
        int datasize;

        datasize = tree->glyph[i].metrics->linestep *
                tree->glyph[i].metrics->height;
        tree->glyph[i].data = *data; *data += datasize;
    }

    if (tree->less)
        readData (tree->less, data);
    if (tree->more)
        readData (tree->more, data);
}

static void BuildGlyphTree (QPFGLYPHTREE* tree, Uint8** data)
{
    readNode (tree, data);
    readMetrics (tree, data);
    readData (tree, data);
}

BOOL LoadQPFont (const char* file, int fd,  QPFINFO* QPFInfo)
{
    struct stat st;
    Uint8* data;

    if ((fd = open (file, O_RDONLY)) < 0) return FALSE;
    if (fstat (fd, &st)) return FALSE;
    QPFInfo->file_size = st.st_size;


    QPFInfo->fm = (QPFMETRICS*) mmap( 0, st.st_size,
                    PROT_READ, MAP_PRIVATE, fd, 0 );

    if (!QPFInfo->fm || QPFInfo->fm == (QPFMETRICS *)MAP_FAILED)
        goto error;


    QPFInfo->tree = (QPFGLYPHTREE*)calloc (1, sizeof (QPFGLYPHTREE));

    data = (Uint8*)QPFInfo->fm;
    data += sizeof (QPFMETRICS);
    BuildGlyphTree (QPFInfo->tree, &data);

    return TRUE;

error:
    munmap (QPFInfo->fm, QPFInfo->file_size);
    close (fd);
    return FALSE;
}

static void ClearGlyphTree (QPFGLYPHTREE* tree)
{
    if (tree->less) {
        ClearGlyphTree (tree->less);
    }
    if (tree->more) {
        ClearGlyphTree (tree->more);
    }

    free (tree->glyph);
    free (tree->less);
    free (tree->more);
}

void UnloadQPFont (QPFINFO* QPFInfo, int fd)
{
    if (QPFInfo->file_size == 0)
        return;

    ClearGlyphTree (QPFInfo->tree);
    free (QPFInfo->tree);
    munmap (QPFInfo->fm, QPFInfo->file_size);
    close (fd);
}

static void AddDate (QPFGLYPHTREE* tree, FILE *upfile)
{
    UPFNODE  upfnode;
    UPFGLYPH upfglyph;
    int n, i;
    unsigned int datalen;

    fread (&upfnode, sizeof (UPFNODE), 1, upfile); 
    
    n = upfnode.max - upfnode.min + 1;
    
    for (i =0; i<n; i++)
    {
        fseek (upfile, (upfnode.glyph_offset + i * sizeof (UPFGLYPH)), SEEK_SET );
        fread (&upfglyph, sizeof (UPFGLYPH), 1, upfile); 
        fseek (upfile, 0L, SEEK_END );
        upfglyph.bitmap_offset =ftell (upfile); 

        fseek (upfile, (upfnode.glyph_offset + i * sizeof (UPFGLYPH)), SEEK_SET );
        fwrite ((const void*)&upfglyph, sizeof (UPFGLYPH), 1, upfile );
        fseek (upfile, 0L, SEEK_END );
        
        if (off_bitmaps== 0)
            off_bitmaps = ftell (upfile);

        datalen = (upfglyph.height * upfglyph.pitch);
#if 0
        //for 4 bytes allignment
        fillbytes = (datalen % 4) ? (4 - (upfglyph.height * upfglyph.pitch) % 4) : 0;
        len_bitmaps += datalen + fillbytes;
#endif

        fwrite ((const void*)(tree->glyph[i].data), datalen, 1, upfile );
#if 0
        for(j =0; j < fillbytes ;j++){
            fwrite ((const void *)&fillcont, 1,1, upfile);
        }
#endif
    }
    
    if (upfnode.less_offset)
    {
        fseek (upfile, upfnode.less_offset, SEEK_SET); 
        AddDate (tree->less, upfile);
    }
    if (upfnode.more_offset)
    {
        fseek (upfile, upfnode.more_offset, SEEK_SET); 
        AddDate (tree->more, upfile);
    }
    
}


static void CreatGlyph (QPFGLYPHTREE* tree, FILE *upfile)
{
    UPFNODE  upfnode;
    static   UPFGLYPH upfglyph;
    unsigned int curr_pos;
    int n, i;
    
    curr_pos = ftell (upfile);
    fread (&upfnode, sizeof (UPFNODE), 1, upfile); 
    n = upfnode.max - upfnode.min + 1;


    fseek (upfile, 0L, SEEK_END );
    upfnode.glyph_offset = ftell (upfile);

    if (off_glyphs== 0)
        off_glyphs = ftell (upfile);

    for (i = 0; i<n; i++)
    {
        nr_glyphs ++;

        upfglyph.bearingx      = tree->glyph[i].metrics->bearingx;
        upfglyph.bearingy      = tree->glyph[i].metrics->bearingy;
        upfglyph.width         = tree->glyph[i].metrics->width;
        upfglyph.height        = tree->glyph[i].metrics->height;
        //printf("upfglyph.height=%d,upfglyph.width=%d\n",upfglyph.height,upfglyph.width);
        upfglyph.pitch         = tree->glyph[i].metrics->linestep;
        upfglyph.padding1      = tree->glyph[i].metrics->padding;
        upfglyph.advance       = tree->glyph[i].metrics->advance;
        upfglyph.padding2      = tree->glyph[i].metrics->reserved;
        upfglyph.bitmap_offset = 0;

        fwrite ((const void*)&upfglyph, sizeof (UPFGLYPH), 1, upfile );
    }   
    
    fseek (upfile, curr_pos, SEEK_SET); 
    fwrite ((const void*)&upfnode, sizeof (UPFNODE), 1, upfile);
    
    if (upfnode.less_offset)
    {
        fseek (upfile, upfnode.less_offset, SEEK_SET); 
        CreatGlyph (tree->less, upfile);
    }
    if (upfnode.more_offset)
    {
        fseek (upfile, upfnode.more_offset, SEEK_SET); 
        CreatGlyph (tree->more, upfile);
    }
    
}
static void CreateNodes (QPFGLYPHTREE* tree, FILE *upfile )
{
    UPFNODE upfnode; 
    unsigned int curr_pos;
    
    curr_pos = ftell (upfile);
    upfnode.min = tree->min;
    upfnode.max = tree->max;
    upfnode.less_offset = 0;
    upfnode.more_offset = 0;
    
    nr_nodes ++;
    
    if (off_nodes==0)
        off_nodes = curr_pos;

    fwrite ((const void*)&upfnode,  sizeof (UPFNODE),  1, upfile);
    
    if (tree->less)
    {
        upfnode.less_offset = ftell (upfile) ;
        fseek (upfile, curr_pos, SEEK_SET );
        fwrite ((const void*)&upfnode,  sizeof (UPFNODE),  1, upfile);
        fseek (upfile, 0L, SEEK_END);
        CreateNodes (tree->less, upfile);
    }

    if (tree->more)
    {
        fseek (upfile, 0L, SEEK_END);
        upfnode.more_offset = ftell (upfile);
        fseek (upfile, curr_pos, SEEK_SET );
        fwrite ((const void*)&upfnode,  sizeof (UPFNODE),  1, upfile);
        fseek (upfile, 0L, SEEK_END);
        CreateNodes (tree->more, upfile);
    }
     
}


#define   NR_LOOP_FOR_WIDTH       3
#define   NR_LOOP_FOR_HEIGHT      4
#define   NR_LOOP_FOR_CHARSET     5
#define   LEN_FONT_NAME           31

static void CreateFontname(int width, int height, char* newfontname, const char* oldfontname)
{
    char tmpbuf[20]={0};
    const char* point;
    int len,i;

    if(!newfontname || !oldfontname)
        return ;

    sprintf(tmpbuf,"%d-%d",width,height);

    point =oldfontname;
    for (i = 0; i < NR_LOOP_FOR_WIDTH; i++) {
        if ((point = strchr (point, '-')) == NULL)
            goto direct_process;
        if (*(++point) == '\0')
            goto direct_process;
    }

    len =point -oldfontname;
    strncpy(newfontname,oldfontname,len);
    strcat(newfontname,tmpbuf);

    for (i = 0; i < (NR_LOOP_FOR_CHARSET -NR_LOOP_FOR_WIDTH); i++) {
        if ((point = strchr (point, '-')) == NULL)
            goto direct_process;
        if (*(++point) == '\0')
            goto direct_process;
    }
    strcat(newfontname,(point -1));
    printf("newfontname:%s\n",newfontname);

    return ;

direct_process:
        snprintf(newfontname, LEN_DEVFONT_NAME, "%s", oldfontname);
}

int fontGetWidthFromName (const char* name)
{
    int i;
    const char* width_part = name;
    char width [LEN_FONT_NAME + 1];

    for (i = 0; i < NR_LOOP_FOR_WIDTH; i++) {
        if ((width_part = strchr (width_part, '-')) == NULL)
            return -1;

        if (*(++width_part) == '\0')
            return -1;
    }

    i = 0;
    while (width_part [i]) {
        if (width_part [i] == '-') {
            width [i] = '\0';
            break;
        }

        width [i] = width_part [i];
        i++;
    }

    if (width_part [i] == '\0')
        return -1;

    return atoi (width);
}

int fontGetHeightFromName (const char* name)
{
    int i;
    const char* height_part = name;
    char height [LEN_FONT_NAME + 1];

    for (i = 0; i < NR_LOOP_FOR_HEIGHT; i++) {
        if ((height_part = strchr (height_part, '-')) == NULL)
            return -1;
        if (*(++height_part) == '\0')
            return -1;
    }

    i = 0;
    while (height_part [i]) {
        if (height_part [i] == '-') {
            height [i] = '\0';
            break;
        }

        height [i] = height_part [i];
        i++;
    }

    if (height_part [i] == '\0')
        return -1;

    return atoi (height);
}


static void ReviseWidthAndHeight(QPFINFO* qpfinfo, UPFV1_FILE_HEADER* pupfheade, const char *font_name)
{
    int  c_width =0, c_height = 0;
    
    c_width =fontGetWidthFromName(font_name);
    c_height =fontGetHeightFromName(font_name);

    pupfheade->ascent =qpfinfo->fm->ascent * c_height / pupfheade->height;
    //pupfheade->descent =qpfinfo->fm->descent * c_height / pupfheade->height;
    pupfheade->descent = c_height - pupfheade->ascent;
    pupfheade->height = c_height;
    pupfheade->width = c_width;
    //printf("pupfhead->ascent=%d,pupfheade->descent=%d,width=%d,height=%d\n",pupfheade->ascent,
    //        pupfheade->descent,pupfheade->width,pupfheade->height);
}

static void CreatUpfFile (QPFINFO *qpfinfo, FILE *upfile ,const char *font_name)
{
    UPFV1_FILE_HEADER  upfheade={{0}};
    Uint32 curr_pos, start_pos;
   
    sprintf (upfheade.ver_info, "upf-1.0**");
    sprintf (upfheade.vender_name, "FMSoft");
    //if(font_name)
    //    snprintf (upfheade.font_name, LEN_DEVFONT_NAME, "%s", font_name);

    upfheade.endian            = 0x1234;

    upfheade.width             = qpfinfo->fm->maxwidth;
    //upfheade.height            = qpfinfo->fm->ascent +qpfinfo->fm->descent + qpfinfo->fm->leading;
    upfheade.height            = qpfinfo->fm->ascent +qpfinfo->fm->descent;
    //printf("qpfinfo->fm->ascent =%d,qpfinfo->fm->descent=%d\nupfheade.widht=%d, upfheade.height=%d\n",qpfinfo->fm->ascent,qpfinfo->fm->descent,upfheade.width,upfheade.height);
    upfheade.ascent            = qpfinfo->fm->ascent;
    upfheade.descent           = qpfinfo->fm->descent;  

#ifdef    REVISE_WIDTH_HEIGHT
    ReviseWidthAndHeight(qpfinfo, &upfheade, font_name);
#endif

    upfheade.max_width         = qpfinfo->fm->maxwidth ;
    upfheade.underline_pos     = qpfinfo->fm->underlinepos;
    upfheade.underline_width   = qpfinfo->fm->underlinewidth;
    upfheade.leading           = qpfinfo->fm->leading;
    //upfheade.mono_bitmap       = !qpfinfo->fm->flags;
    upfheade.mono_bitmap        =!(qpfinfo->fm->flags & QPF_FLAG_MODE_SMOOTH);
    upfheade.off_nodes         = sizeof (UPFV1_FILE_HEADER);

    CreateFontname(upfheade.width,upfheade.height,upfheade.font_name,font_name);

    start_pos = ftell (upfile);
    fwrite((const void*)&upfheade,  sizeof (UPFV1_FILE_HEADER),  1, upfile);
    curr_pos = ftell (upfile);

    CreateNodes (qpfinfo->tree, upfile);
    fseek (upfile, curr_pos, SEEK_SET);
    CreatGlyph  (qpfinfo->tree, upfile);
    fseek (upfile, curr_pos, SEEK_SET);
    AddDate (qpfinfo->tree, upfile);

    fseek (upfile, 0, SEEK_END);
    upfheade.font_size = ftell (upfile);

    fseek (upfile, start_pos, SEEK_SET);

    upfheade.off_nodes   = off_nodes;
    upfheade.off_glyphs  = off_glyphs;
    upfheade.off_bitmaps = off_bitmaps;
    upfheade.len_bitmaps = len_bitmaps;
    upfheade.len_nodes   = nr_nodes  * sizeof (UPFNODE);
    upfheade.len_glyphs  = nr_glyphs * sizeof (UPFGLYPH);

    fwrite((const void*)&upfheade,  sizeof (UPFV1_FILE_HEADER),  1, upfile);
}
/******************************main*************************************/
int main (int args, char *arg[])
{
    QPFINFO qpfinfo;
    char upf_name [128];
    FILE *upf_file;
    int  fd =0;
    char *s;
    
    if (args <3)
    {
        help ();
        printf ("Please  specify  the 'qpf' file and the fontname\n");
        return 1;
    }

    sprintf (upf_name, "%s", arg[1]);
    s = strrchr ((const char *)upf_name, '.');

    if (s==NULL)
    {
        help ();
        printf ("Please  specify  the 'qpf' file\n");
        return 1;
    }

    *s ='\0';
    sprintf (s, "%s", ".upf");
    
    if (!LoadQPFont (arg[1], fd, &qpfinfo))
    {
        help ();
        printf ("Load %s faile !\n", arg[1]);
        return 2;
    }
    
/*****************create upf file********************************/
    upf_file = fopen ((const char *)upf_name, "w+"); 
    CreatUpfFile (&qpfinfo, upf_file, arg[2]);
    fclose (upf_file);
/****************************************************************/    
    UnloadQPFont (&qpfinfo, fd);
    
         
    return 0;
}
