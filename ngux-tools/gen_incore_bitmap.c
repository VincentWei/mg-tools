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
 * This command uses the zlib library to compress each file given on
 * the command line, and outputs the compressed data as C source code
 * to the file 'data.c' in the current directory
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifndef __NOUNIX__
#include <unistd.h>
#include <pwd.h>
#endif

// !!! same as GifAnimate.h : BitmapFrame
typedef struct _tagBitmapFrame {
        int off_x;
        int off_y;
        int disposal;
        unsigned int delay_time;
        PBITMAP bmp;
        struct _tagBitmapFrame* next;
        struct _tagBitmapFrame* prev;
}BitmapFrame;

typedef struct _tagBitmapFrameArray
{
    int nr_frames;
    BitmapFrame* frames;
}BitmapFrameArray;
//////////////////////////////// for gif ///////////////////////////////////
#define MAXCOLORMAPSIZE         256
#define MAX_LWZ_BITS            12
#define INTERLACE               0x40
#define LOCALCOLORMAP           0x80

#define CM_RED                  0
#define CM_GREEN                1
#define CM_BLUE                 2

#define BitSet(byte, bit)               (((byte) & (bit)) == (bit))
#define ReadOK(file,buffer,len)         MGUI_RWread(file, buffer, len, 1)
#define LM_to_uint(a,b)                 (((b)<<8)|(a))
#define PIX2BYTES(n)                    (((n)+7)/8)

typedef struct tagGIFSCREEN {
    unsigned int Width;
    unsigned int Height;
    RGB ColorMap [MAXCOLORMAPSIZE];
    unsigned int BitPixel;
    unsigned int ColorResolution;
    unsigned int Background;
    unsigned int AspectRatio;
    int transparent;
    int delayTime;
    int inputFlag;
    int disposal;
} GIFSCREEN;

typedef struct tagIMAGEDESC {
    int Top;
    int Left;
    int Width;
    int Height;
    BOOL haveColorMap;
    int bitPixel;
    int grayScale;
    RGB ColorMap [MAXCOLORMAPSIZE];
    BOOL interlace;
} IMAGEDESC;

static int ZeroDataBlock = 0;

static int bmpComputePitch (int bpp, Uint32 width, Uint32* pitch, BOOL does_round);
static int LWZReadByte (MG_RWops *area, int flag, int input_code_size);
static int GetCode (MG_RWops *area, int code_size, int flag);
static int GetDataBlock (MG_RWops *area, unsigned char *buf);
static int DoExtension (MG_RWops *area, int label, GIFSCREEN* GifScreen);
static int ReadColorMap (MG_RWops *area, int number, RGB* ColorMap);
static int ReadImageDesc (MG_RWops *area, IMAGEDESC* ImageDesc, GIFSCREEN* GifScreen);

static int bmpComputePitch (int bpp, Uint32 width, Uint32* pitch, BOOL does_round)
{
    Uint32 linesize;
    int bytespp = 1;

    if(bpp == 1)
        linesize = PIX2BYTES (width);
    else if(bpp <= 4)
        linesize = PIX2BYTES (width << 2);
    else if (bpp <= 8)
        linesize = width;
    else if(bpp <= 16) {
        linesize = width * 2;
        bytespp = 2;
    } else if(bpp <= 24) {
        linesize = width * 3;
        bytespp = 3;
    } else {
        linesize = width * 4;
        bytespp = 4;
    }

    /* rows are DWORD right aligned*/
    if (does_round)
        *pitch = (linesize + 3) & -4;
    else
        *pitch = linesize;
    return bytespp;
}

static int LWZReadByte (MG_RWops *area, int flag, int input_code_size)
{
    int code, incode;
    register int i;
    static int fresh = FALSE;
    static int code_size, set_code_size;
    static int max_code, max_code_size;
    static int firstcode, oldcode;
    static int clear_code, end_code;
    static int table[2][(1 << MAX_LWZ_BITS)];
    static int stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;

    if (flag) {
        set_code_size = input_code_size;
        code_size = set_code_size + 1;
        clear_code = 1 << set_code_size;
        end_code = clear_code + 1;
        max_code_size = 2 * clear_code;
        max_code = clear_code + 2;

        GetCode(area, 0, TRUE);

        fresh = TRUE;

        for (i = 0; i < clear_code; ++i) {
            table[0][i] = 0;
            table[1][i] = i;
        }
        for (; i < (1 << MAX_LWZ_BITS); ++i)
            table[0][i] = table[1][0] = 0;

        sp = stack;

        return 0;
    } else if (fresh) {
        fresh = FALSE;
        do {
            firstcode = oldcode = GetCode(area, code_size, FALSE);
        } while (firstcode == clear_code);
        return firstcode;
    }
    if (sp > stack)
        return *--sp;

    while ((code = GetCode(area, code_size, FALSE)) >= 0) {
        if (code == clear_code) {
            for (i = 0; i < clear_code; ++i) {
                table[0][i] = 0;
                table[1][i] = i;
            }
            for (; i < (1 << MAX_LWZ_BITS); ++i)
                table[0][i] = table[1][i] = 0;
            code_size = set_code_size + 1;
            max_code_size = 2 * clear_code;
            max_code = clear_code + 2;
            sp = stack;
            firstcode = oldcode = GetCode(area, code_size, FALSE);
            return firstcode;
        } else if (code == end_code) {
            int count;
            unsigned char buf[260];

            if (ZeroDataBlock)
                return -2;

            while ((count = GetDataBlock(area, buf)) > 0);

            if (count != 0) {
                /*
                 * fprintf (stderr,"missing EOD in data stream (common occurence)");
                 */
            }
            return -2;
        }
        incode = code;

        if (code >= max_code) {
            *sp++ = firstcode;
            code = oldcode;
        }
        while (code >= clear_code) {
            *sp++ = table[1][code];
            if (code == table[0][code]) {
                return -1;
            }
            code = table[0][code];
        }

        *sp++ = firstcode = table[1][code];

        if ((code = max_code) < (1 << MAX_LWZ_BITS)) {
            table[0][code] = oldcode;
            table[1][code] = firstcode;
            ++max_code;
            if ((max_code >= max_code_size) &&
                (max_code_size < (1 << MAX_LWZ_BITS))) {
                max_code_size *= 2;
                ++code_size;
            }
        }
        oldcode = incode;

        if (sp > stack)
            return *--sp;
    }
    return code;
}


static int GetCode(MG_RWops *area, int code_size, int flag)
{
    static unsigned char buf[280];
    static int curbit, lastbit, done, last_byte;
    int i, j, ret;
    unsigned char count;

    if (flag) {
        curbit = 0;
        lastbit = 0;
        done = FALSE;
        return 0;
    }
    if ((curbit + code_size) >= lastbit) {
        if (done) {
            if (curbit >= lastbit)
            return -1;
        }
        buf[0] = buf[last_byte - 2];
        buf[1] = buf[last_byte - 1];

        if ((count = GetDataBlock(area, &buf[2])) == 0)
            done = TRUE;

        last_byte = 2 + count;
        curbit = (curbit - lastbit) + 16;
        lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = curbit, j = 0; j < code_size; ++i, ++j)
        ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}

static int GetDataBlock (MG_RWops *area, unsigned char *buf)
{
    unsigned char count;

    if (!ReadOK(area, &count, 1))
        return 0;
    ZeroDataBlock = (count == 0);

    if ((count != 0) && (!ReadOK(area, buf, count)))
        return 0;
    return count;
}

static int DoExtension (MG_RWops *area, int label, GIFSCREEN* GifScreen)
{
    static unsigned char buf[256];

    switch (label) {
    case 0x01:                        /* Plain Text Extension */
        while (GetDataBlock (area, (unsigned char *) buf) != 0);
        break;
    case 0xff:                        /* Application Extension */
        while (GetDataBlock (area, (unsigned char *) buf) != 0);
        break;
    case 0xfe:                        /* Comment Extension */
        while (GetDataBlock (area, (unsigned char *) buf) != 0);
        return 0;
    case 0xf9:                        /* Graphic Control Extension */
        GetDataBlock (area, (unsigned char *) buf);
        GifScreen->disposal = (buf[0] >> 2) & 0x7;//000 000 0 0 the middle 2 bit is disposal
        GifScreen->inputFlag = (buf[0] >> 1) & 0x1;//000 000 0 0 the secand last bit 
                            //is user input flag
        GifScreen->delayTime = LM_to_uint(buf[1], buf[2]);
        if ((buf[0] & 0x1) != 0)// 000 000 0 0 the last bit is transparent flag
            GifScreen->transparent = buf[3];
        else
            GifScreen->transparent = -1;

        while (GetDataBlock (area, (unsigned char *) buf) != 0);
        return 0;
    default:
        while (GetDataBlock (area, (unsigned char *) buf) != 0);
        break;
    }

    return 0;
}

static int ReadColorMap (MG_RWops *area, int number, RGB* ColorMap)
{
    int i;
    unsigned char rgb[3];

    for (i = 0; i < number; ++i) {
        if (!ReadOK (area, rgb, sizeof(rgb))) {
            return -1;
        }

        ColorMap [i].r = rgb[0];
        ColorMap [i].g = rgb[1];
        ColorMap [i].b = rgb[2];
    }

    return 0;
}

static int ReadGIFGlobal (MG_RWops *area, GIFSCREEN* GifScreen)
{
    unsigned char buf[9];
    unsigned char version[4];

    if (!ReadOK (area, buf, 6))
        return -1;                /* not gif image*/

    if (strncmp((char *) buf, "GIF", 3) != 0)
        return -1;

    strncpy ((char*)version, (char *) buf + 3, 3);
    version [3] = '\0';

    if (strcmp ((const char*)version, "87a") != 0 && strcmp ((const char*)version, "89a") != 0) {
        return -1;                /* image loading error*/
    }

    GifScreen->Background = -1;
    GifScreen->transparent = -1;
    GifScreen->delayTime = -1;
    GifScreen->inputFlag = -1;
    GifScreen->disposal = 0;

    if (!ReadOK (area, buf, 7)) {
        return -1;                /* image loading error*/
    }
    GifScreen->Width = LM_to_uint (buf[0], buf[1]);
    GifScreen->Height = LM_to_uint (buf[2], buf[3]);
    GifScreen->BitPixel = 2 << (buf[4] & 0x07);
    GifScreen->ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
    GifScreen->Background = buf[5];
    GifScreen->AspectRatio = buf[6];

    if (BitSet(buf[4], LOCALCOLORMAP)) {        /* Global Colormap */
        if (ReadColorMap (area, GifScreen->BitPixel, GifScreen->ColorMap)) {
            return -1;                /* image loading error*/
        }
    }

    return 0;
}

static int ReadImageDesc (MG_RWops *area, IMAGEDESC* ImageDesc, GIFSCREEN* GifScreen)
{
    unsigned char buf[16];
    if (!ReadOK (area, buf, 9)) {
        return -1;
    }

    ImageDesc->Top = LM_to_uint (buf[0], buf[1]);
    ImageDesc->Left = LM_to_uint (buf[2], buf[3]);
    ImageDesc->Width = LM_to_uint (buf[4], buf[5]);
    ImageDesc->Height = LM_to_uint (buf[6], buf[7]);
    ImageDesc->haveColorMap = BitSet (buf[8], LOCALCOLORMAP);

    ImageDesc->bitPixel = 1 << ((buf[8] & 0x07) + 1);

    ImageDesc->interlace = BitSet(buf[8], INTERLACE);

    if (ImageDesc->haveColorMap) {
        if (ReadColorMap (area, ImageDesc->bitPixel, ImageDesc->ColorMap) < 0) {
            return -1;
        }
    } else {
        memcpy (ImageDesc->ColorMap, GifScreen->ColorMap, MAXCOLORMAPSIZE*sizeof (RGB));
    }

    return 0;
}



static int ReadImage (MG_RWops* area, MYBITMAP* bmp, IMAGEDESC* ImageDesc, GIFSCREEN* GifScreen, int ignore)
{

    unsigned char c;
    int v;
    int xpos = 0, ypos = 0, pass = 0;

    /*
     * initialize the compression routines
     */
    if (!ReadOK (area, &c, 1)) {
        return -1;
    }

    if (LWZReadByte (area, TRUE, c) < 0) {
        return -1;
    }

    /*
     * if this is an "uninteresting picture" ignore it.
     */
    if (ignore) {
        while (LWZReadByte (area, FALSE, c) >= 0);
        return 0;
    }

    bmp->w = ImageDesc->Width;
    bmp->h = ImageDesc->Height;

    bmp->flags = MYBMP_FLOW_DOWN;
    if (GifScreen->transparent >= 0) {
        bmp->flags |= MYBMP_TRANSPARENT;
        bmp->transparent = GifScreen->transparent;
    }
    bmp->frames = 1;
    bmp->depth = 8;
    bmpComputePitch (bmp->depth, bmp->w, &bmp->pitch, TRUE);
    bmp->bits = (BYTE *)malloc (bmp->h * bmp->pitch);

    if(!bmp->bits)
        return -1;

    while ((v = LWZReadByte (area, FALSE, c)) >= 0) {
        bmp->bits[ypos * bmp->pitch + xpos] = v;
        ++xpos;
        if (xpos == ImageDesc->Width) {
            xpos = 0;
            if (ImageDesc->interlace) {
                switch (pass) {
                case 0:
                case 1:
                    ypos += 8;
                    break;
                case 2:
                    ypos += 4;
                    break;
                case 3:
                    ypos += 2;
                    break;
                }

                if (ypos >= ImageDesc->Height) {
                    ++pass;
                    switch (pass) {
                    case 1:
                        ypos = 4;
                        break;
                    case 2:
                        ypos = 2;
                        break;
                    case 3:
                        ypos = 1;
                        break;
                    default:
                        goto fini;
                    }
                }
            } else {
                ++ypos;
            }
        }
        if (ypos >= ImageDesc->Height)
            break;
    }

fini:
    if (v >= 0) return 0;
    return -1;
}

// return error
int createGifAnimateFromFile (BitmapFrameArray* res, const char* file)
{
    unsigned char c;
    int ok = 0;
    MYBITMAP mybmp;
    GIFSCREEN GifScreen;
    IMAGEDESC ImageDesc;
    BitmapFrame* frame, *frames = NULL, *current = NULL;
    int frame_count = 0;
    MG_RWops* area = NULL;

    if (!(area = MGUI_RWFromFile (file, "rb"))) {
        return -1;
    }

    if (ReadGIFGlobal (area, &GifScreen) < 0)
        return -1;

    if ((ok = ReadOK (area, &c, 1)) == 0) {
        return -1;
    }

    while (c != ';' && ok > 0) {
        switch (c) {
            case '!':
                if ( (ok = ReadOK (area, &c, 1)) == 0) {
                    return 0;
                }
                DoExtension (area, c, &GifScreen);
                break;

            case ',':
                if (ReadImageDesc (area, &ImageDesc, &GifScreen) < 0) {
                    return 0;
                }
                else {
                    if (ReadImage (area, &mybmp, &ImageDesc, &GifScreen, 0) < 0)
                        return 0;
                }

                frame = (BitmapFrame*) calloc(1, sizeof(BitmapFrame));


                if(!frame)
                    return -1;

                frame->bmp = (BITMAP*) calloc(1, sizeof(BITMAP));

                if(!frame->bmp)
                    return -1;

                frame->next = NULL;
                frame->off_y = ImageDesc.Left;
                frame->off_x = ImageDesc.Top;
                frame->disposal = GifScreen.disposal;

                frame->delay_time = (GifScreen.delayTime>10)?GifScreen.delayTime:10;

                if(ExpandMyBitmap(HDC_SCREEN, frame->bmp, &mybmp, ImageDesc.ColorMap, 0) != 0)
                {
                    free(frame);
                    free(mybmp.bits);
                    return -1;
                }

                if(frames == NULL)
                {
                    frames = frame;
                    current = frame;
                    current->prev = NULL;
                }
                else
                {
                    frame->prev = current;
                    current->next = frame;
                    current = current->next;
                }

                //m_last_frame = frame;

                //m_nr_frames++;
                frame_count++;
                break;
        }
        ok = ReadOK (area, &c, 1);
    }
    //m_current_frame = m_frames;
    //m_max_width = GifScreen.Width;
    //m_max_height = GifScreen.Height;
    res->nr_frames = frame_count;
    res->frames = frames;

    MGUI_RWclose (area);

    return 0;
}
/////////////////////////////////// for gif end /////////////////////////////////////////////

typedef unsigned char Bytef;
typedef unsigned long uLongf;



#define SUFFIXLEN 8

const char* usage =
    "\nUsage: ./gen_incore_bitmap bmp-file output-file dest-name pkg-name\n\n"
    "    Example: ./gen_incore_bitmap file1.png data.c file1_data desktop\n\n";

static const char* gen_name(const char* filename, const char* suffix)
{
    int idx = 0;
    int i;
    const char* begin;
    const char* ext;
    //static char strname[1024];
    begin = strrchr(filename, '/');
    if(begin == NULL) begin = filename;

    ext = strrchr(begin, '.');
    i = (ext?(ext-begin):strlen(begin))+10;
    char* strname = (char*)malloc(i+100);

    strname[idx++] = '_';
    if(ext) {
        for(i=1; ext[i]; i++)
            strname[idx++] = ext[i];
        strname[idx++] = '_';
    }

    for(i=0; (ext && &begin[i]<ext)||(!ext && begin[i]); i++) {
        if(isalnum(begin[i]))
            strname[idx++] = begin[i];
        else
            strname[idx++] = '_';
    }

    if(strname[idx-1] != '_') {
        strcpy(strname+idx, "_");
        idx++;
    }

    strcpy(strname+idx, suffix);

    return strname;
}

static void output_data(Bytef *dest, uLongf destLen, FILE *fp, const char* name)
{
    int j;

    fprintf (fp, "static const unsigned char %s[] = {\n", name);
    for (j=0; j<destLen-1; j++) {
        switch (j%8) {
            case 0:
                fprintf (fp, "  0x%02x, ", ((unsigned) dest[j]) & 0xffu);
                break;
            case 7:
                fprintf (fp, "0x%02x,\n", ((unsigned) dest[j]) & 0xffu);
                break;
            default:
                fprintf (fp, "0x%02x, ", ((unsigned) dest[j]) & 0xffu);
                break;
        }
    }

    if ((destLen-1)%8 == 0)
        fprintf (fp, "  0x%02x\n};\n\n", ((unsigned) dest[destLen-1]) & 0xffu);
    else
        fprintf (fp, "0x%02x\n};\n\n", ((unsigned) dest[destLen-1]) & 0xffu);
}

void OutputBitmapData(FILE* outfile, BITMAP mgBmp, const char* bmpname)
{
    unsigned int destLen = 0;
    char szname[256];
    Bytef *dest = 0;
    destLen = mgBmp.bmPitch * mgBmp.bmHeight;
    dest =  mgBmp.bmBits;
    sprintf (szname, "%s_data", bmpname);
    output_data(dest, destLen, outfile, szname);

    if (mgBmp.bmAlphaMask) {
        dest = mgBmp.bmAlphaMask;
        destLen = mgBmp.bmHeight * mgBmp.bmAlphaPitch;
        sprintf (szname, "%s_alpha", bmpname);
        output_data(dest, destLen, outfile, szname);
    }
}
void OutputBitmapDesc(FILE* outfile, BITMAP mgBmp, const char* bmpname)
{
    fprintf (outfile, "const BITMAP %s = {\n", bmpname);
    fprintf (outfile, "     %d, %d, %d, 0x%0x, 0x%0x,\n", 
            mgBmp.bmType, mgBmp.bmBitsPerPixel, mgBmp.bmBytesPerPixel,
            mgBmp.bmAlpha, mgBmp.bmColorKey);
#ifdef _FOR_MONOBITMAP
    fprintf (outfile, "0x%0x, ", mgBmp.bmColorRep); 
#endif
    fprintf (outfile, "     %d, %d, %d, \n", 
            mgBmp.bmWidth, mgBmp.bmHeight, mgBmp.bmPitch);
    fprintf (outfile, "     (Uint8*)%s_data, ", bmpname);

    if (mgBmp.bmAlphaMask) {
        fprintf (outfile, "\n     (Uint8*)%s_alpha,\n     ", bmpname);
    }
    else
        fprintf (outfile, "NULL, ");
    fprintf (outfile, "0x%0x\n};\n\n\n", mgBmp.bmAlphaPitch);
}

void OutputHead(FILE* outfile)
{
    time_t t = time(NULL);
    struct tm* _tm = localtime(&t);
    char datetime[12];
    strftime(datetime, 12, "%F", _tm);
#ifndef __NOUNIX__
    struct passwd *pwd = getpwuid(getuid());
#endif

    fprintf(outfile, "// Create For Apollo Inner Resource\n");
    fprintf(outfile, "// Create Date : %s\n", datetime);
#ifndef __NOUNIX__
    fprintf(outfile, "// Create By : %s\n\n", pwd->pw_name);
#endif
    //fprintf(outfile, "#include <Apollo.h>\n\n");
    //fprintf(outfile, "#include \"NguxCommon.h\"\n");
    //fprintf(outfile, "#include \"Ngux.h\"\n");
    fprintf(outfile, "#include \"NGUX.h\"\n");
    fprintf(outfile, "#include \"apolloconfig.h\"\n\n");
    fprintf(outfile, "#ifdef _APOLLO_INNER_RES\n\n");
}

void OutputTail(FILE* outfile)
{
    fprintf (outfile, "\n\n#endif   // _APOLLO_INNER_RES\n\n");
}

void OutputExternCHead(FILE* outfile)
{
    fprintf(outfile, "#ifdef __cplusplus\nextern \"C\"\n{\n#endif // __cplusplus\n\n");
}

void OutputExternCTail(FILE* outfile)
{
    fprintf(outfile, "#ifdef __cplusplus\n}\n#endif // __cplusplus \n\n");
}

void OutputExternCBitmapFrameArrayEntry(FILE* outfile, const char* bmpname)
{
    // extern C
    OutputExternCHead(outfile);
    fprintf(outfile, "extern const BitmapFrameArray %s;\n\n", bmpname);
    OutputExternCTail(outfile);
}



void OutputArrayEntryHead(FILE* outfile, const char* bmpname)
{
    fprintf(outfile, "const BitmapFrameArray %s = {\n", bmpname);
}
void OutputArrayEntryTail(FILE* outfile)
{
    fprintf(outfile, "};\n\n");
}

void OutputArrayFramesHead(FILE* outfile, const char* bmpname)
{
    fprintf(outfile, "const BitmapFrame %s_array_frames [] = {\n", bmpname);
}

void OutputArrayFramesTail(FILE* outfile)
{
    fprintf(outfile, "};\n\n");
}

void OutputFakeData(FILE* outfile, const char* bmpname)
{
    // head
    OutputHead(outfile);
    // extern C
    OutputExternCBitmapFrameArrayEntry(outfile, bmpname);
    // ONLY entry
    OutputArrayEntryHead(outfile, bmpname);
    fprintf(outfile, "%d, \n", 0); 
    fprintf(outfile, "(BitmapFrame *)NULL,\n"); 
    OutputArrayEntryTail(outfile);
    // tail
    OutputTail(outfile);
}

int main (int argc, char** argv)
{
    FILE *outfile = NULL;  
    BITMAP mgBmp;
    //char *dataName, *alphaName;
    //uLongf destLen; 
    //Bytef *dest = NULL; 
    int len = 0;
    int i = 0;

    if (argc != 5) {
        printf("%s", usage);
        return 1;
    }

    const char *imagefile = argv[1];
    const char *outputfile = argv[2];
    const char *destname = argv[3];
    const char *pkgname = argv[4];
    char bmpname[256]; 
    char dataname[256];
    BitmapFrameArray frameArray;

    outfile = fopen (outputfile, "w");
    if (outfile == NULL) {
        fprintf (stderr, "%s: can't open '%s' for writing\n", argv[0], outputfile);
        return 1;
    }

    /*
    fprintf(stderr, "imagefile is %s\n", imagefile);
    fprintf(stderr, "outfile is %s\n", outputfile);
    fprintf(stderr, "destname is %s\n", destname);
    fprintf(stderr, "pkgname is %s\n", pkgname);
    */

    if (InitGUI(argc, (const char**)argv) != 0) {
        fprintf (stderr, "InitGUI failure\n");
        fclose(outfile);
        return 1;
    }

    len = strlen(imagefile);

    if (len <= 0)
    {
        fprintf (stderr, "file name len <= 0\n");
        fclose(outfile);
        return 1;
    }

    sprintf (bmpname, "_%s_%s_bitmap", pkgname, destname);
    /* Process each file given on command line */
    if (len >= 3)
    {
        //////////////////////////////////////////////// GIF ///////////////////////////////////////////////
        if ('g' == imagefile[len - 3] && 'i' == imagefile[len - 2] && 'f' == imagefile[len - 1])
        {
            //////////////////////////////// 0 means ok, -1 means error /////////////////////////////////////
            if (0 == createGifAnimateFromFile(&frameArray, imagefile))
            {

                // file head
                OutputHead(outfile);
                // extern C entry
                OutputExternCBitmapFrameArrayEntry(outfile, bmpname);


                ///// Step 1 : output bitmaps, they are real BITMAP and data array
                // ------------------------ bitmaps
                i = 0;
                BitmapFrame* fr = frameArray.frames;
                while(fr)
                {
                    sprintf(dataname, "%s_array_frames_bmp%d", bmpname, i);
                    OutputBitmapData(outfile, *(fr->bmp), dataname);
                    OutputBitmapDesc(outfile, *(fr->bmp), dataname);
                    fr = fr->next;
                    ++i;
                }
                // ------------------------- bitmaps end
                ///// Step 2 : output frames , it is a array of BitmapFrame
                OutputArrayFramesHead(outfile, bmpname);
                fr = frameArray.frames;
                // --------------------- frames
                i = 0;
                while(fr)
                {
                    fprintf(outfile, "{\n");
                    fprintf(outfile, "/* off_x */ %d, /* off_y */ %d, /* disposal */ %d, /* delay_time */ %d, \n", fr->off_x, fr->off_y, fr->disposal, fr->delay_time);
                    fprintf(outfile, "/* bmp */ (BITMAP *)&%s_array_frames_bmp%d, /* next */ NULL, /* prev */ NULL\n", bmpname, i);
                    fprintf(outfile, "},\n");
                    fr = fr->next;
                    ++i;
                }
                OutputArrayFramesTail(outfile);
                // ------------------------ frames end

                ///// Step 3 : output user entry BitmapFrameArray, this is used outside.
                OutputArrayEntryHead(outfile, bmpname);
                fprintf(outfile, "%d, \n", frameArray.nr_frames); 
                fprintf(outfile, "(BitmapFrame *)%s_array_frames,\n", bmpname); 
                OutputArrayEntryTail(outfile);
                OutputTail(outfile);
                return 0;
            }
        }
    }
    //////////////////////////////////////////////// NOT GIF ///////////////////////////////////////////////
    if (LoadBitmap (HDC_SCREEN, &mgBmp, imagefile)) {
        fprintf (stderr, "MiniGUI load %s failure \n", imagefile); 
        OutputFakeData(outfile, bmpname);
#if 0
        fprintf (stderr, "MiniGUI load %s failure \n", imagefile); 
        sprintf (bmpname, "_%s_%s_bitmap", pkgname, destname);

        fprintf(outfile, "/*\n** $Id: gen_incore_bitmap.c 2161 2011-12-08 09:27:37Z ylwang $\n**\n** %s_%s.cpp : TODO\n**\n** Copyright (C) 2002 ~ ", pkgname, destname);
        struct passwd *pwd = getpwuid(getuid());
        fprintf(outfile, "Beijing FMSoft Technology Co., Ltd.\n**\n** All rights reserved by FMSoft.");
        fprintf(outfile, "\n**\n** Current Maintainer : %s", pwd->pw_name);
        fprintf(outfile, "\n**\n");

        time_t t = time(NULL);
        struct tm* _tm = localtime(&t);
        char datetime[12];
        strftime(datetime, 12, "%F", _tm);

        fprintf(outfile, "** Create Date : %s\n*/\n\n", datetime);

        fprintf(outfile, "#include <Apollo.h>\n\n");
        fprintf(outfile, "#ifdef _APOLLO_INNER_RES\n\n");
        fprintf(outfile, "#ifdef __cplusplus\nextern \"C\"\n{\n#endif /* __cplusplus */\n\n");
        fprintf(outfile, "extern const BITMAP %s;\n\n", bmpname);
        fprintf(outfile, "#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n\n");

        fprintf (outfile, "const BITMAP %s = {\n", bmpname);
        fprintf (outfile, "     0, 0, 0, 0x0, 0x0,\n"); 
#ifdef _FOR_MONOBITMAP
        fprintf (outfile, "0x0, "); 
#endif
        fprintf (outfile, "     0, 0, 0, NULL, NULL, 0\n\n};\n\n\n" );
        fprintf (outfile, "#endif   // _APOLLO_INNER_RES\n\n");
#endif
        return 1;
    }

    sprintf (bmpname, "_%s_%s_bitmap", pkgname, destname);

    // file head
    OutputHead(outfile);
    // extern C entry
    OutputExternCBitmapFrameArrayEntry(outfile, bmpname);
    // bitmaps
    sprintf(dataname, "%s_array_frames_bmp%d", bmpname, 0);
    OutputBitmapData(outfile, mgBmp, dataname);
    OutputBitmapDesc(outfile, mgBmp, dataname);
    // frames
    OutputArrayFramesHead(outfile, bmpname);
    fprintf(outfile, "{\n");
    fprintf(outfile, "/* off_x */ %d, /* off_y */ %d, /* disposal */ %d, /* delay_time */ %d, \n", 0, 0, 0, 0);
    fprintf(outfile, "/* bmp */ (BITMAP *)&%s_array_frames_bmp%d, /* next */ NULL, /* prev */ NULL\n", bmpname, 0);
    fprintf(outfile, "},\n");
    OutputArrayFramesTail(outfile);
    // entry
    OutputArrayEntryHead(outfile, bmpname);
    fprintf(outfile, "%d, \n", 1); 
    fprintf(outfile, "(BitmapFrame *)%s_array_frames,\n", bmpname); 
    OutputArrayEntryTail(outfile);
    // file tail
    OutputTail(outfile);

#if 0
    /* Output dest buffer as C source code to outfile */
    dataName = (char*)gen_name(imagefile, "data");
    destLen = mgBmp.bmPitch * mgBmp.bmHeight;
    dest =  mgBmp.bmBits;
    output_data(dest, destLen, outfile, dataName);

    if (mgBmp.bmAlphaMask) {
        dest = mgBmp.bmAlphaMask;
        destLen = mgBmp.bmHeight * mgBmp.bmAlphaPitch;
        alphaName = (char*)gen_name(imagefile, "alpha");
        output_data(dest, destLen, outfile, alphaName);
    }

    fprintf (outfile, "const BITMAP %s = {\n", bmpname);
    fprintf (outfile, "     %d, %d, %d, 0x%0x, 0x%0x,\n", 
            mgBmp.bmType, mgBmp.bmBitsPerPixel, mgBmp.bmBytesPerPixel,
            mgBmp.bmAlpha, mgBmp.bmColorKey);
#ifdef _FOR_MONOBITMAP
    fprintf (outfile, "0x%0x, ", mgBmp.bmColorRep); 
#endif
    fprintf (outfile, "     %d, %d, %d, \n", 
            mgBmp.bmWidth, mgBmp.bmHeight, mgBmp.bmPitch);
    fprintf (outfile, "     (Uint8*)%s, ", dataName);

    if (mgBmp.bmAlphaMask) {
        fprintf (outfile, "\n     (Uint8*)%s,\n     ", alphaName);
    }
    else
        fprintf (outfile, "NULL, ");
    fprintf (outfile, "0x%0x\n};\n\n\n", mgBmp.bmAlphaPitch);
    fprintf (outfile, "#endif   // _APOLLO_INNER_RES\n\n");
#endif

    UnloadBitmap(&mgBmp);

    fclose (outfile);

    ExitGUISafely(0);
    return 0;
}
