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
 * KON2 - Kanji ON Console -
 * Copyright (C) 1992-1996 Takashi MANABE (manabe@papilio.tutics.tut.ac.jp)
 *
 * CCE - Console Chinese Environment -
 * Copyright (C) 1998-1999 Rui He (herui@cs.duke.edu)
 *
 * Copyright (C) 2003 Wei Yongmign (ymwei@minigui.org)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY TAKASHI MANABE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TERRENCE R. LAMBERT BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 */

#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <sys/file.h>
#include        <string.h>
#include        <unistd.h>

#include        "font.h"

static u_char  *FontLoadBdf(FILE *fp, FontInfo *fi, FILE *out);
int CodingByRegistry(char *reg);

int main(int argc,char **argv)
{
  FILE *in, *out;
  u_char *font;
  FontInfo fi;

  if (argc != 3)
  {
    fprintf(stderr,"usage: %s  <input_name> <output_name>\n",argv[0]);
    return 1;
  }

  in = fopen(argv[1],"r");
  out = fopen(argv[2], "wb");
  if (in == NULL || out == NULL)
  {
     fprintf(stderr, "Can't open input or output file!\n");
     return 1;
  }

  font = FontLoadBdf(in, &fi, out);

  //if (font != NULL)
  {
       printf("Font %s: Type = %d, High = %d, Width = %d, Size = %d\n\n",
           argv[2], fi.type, fi.high, fi.width, fi.size);
       //fwrite(font, fi.size, 1, out); 
  }
 /* else 
  {
      fprintf(stderr, "Error reading input file!\n");
  }*/

  fclose(in);
  fclose(out);

  return 0;
}

/**********************************************************************
 *            FontLoadBits/FontLoadBdf: Font Loader                   *
 **********************************************************************/

/* fp is a FILE pointer to the BDF font file, this routine will
   allocate memory and load the font bitmap info into memory
   it will return the pointer to the memory */

static u_char  *FontLoadBdf(FILE *fp, FontInfo *fi, FILE *out)
{
    char *fdata = NULL, line[256], *p, *w, reg[256];
    u_char ch, ch2;
    int num, width, high, i, j, code = 0, pitch;
    FontRegs *fReg;
    int bbw,bbh,bbx,bby, fillhigh;
    unsigned char bitmapdata;
    unsigned int data;
    char  fillchar =0x00;

    fReg = &fSRegs[0];
    fi->type = CodingByRegistry("ISO8859-1");
    num = width = high = 0;

    while(fgets(line, 256, fp)) 
    {

      /* Find the FONTBOUNDINGBOX tag */
        if (!width && !high &&
            !strncmp("FONTBOUNDINGBOX", line, strlen("FONTBOUNDINGBOX"))) 
        {
            p = line + sizeof("FONTBOUNDINGBOX");
            sscanf(p, "%d %d", &width, &high);
            break;/* Stop Here, the following are char bitmaps */
        } 

        /* Find the CHARSET_REGISTRY tag */
        else if (!strncmp("CHARSET_REGISTRY", line, 16)) 
        {
            p = line + sizeof("CHARSET_REGISTRY");
            while(*p != '"') p++;
            w = ++p;
            while(*p != '"') p++;
            *p = '\0';
            strcpy(reg, w);    /* CHARSET_REGISTRY "ISO8859" */
        } 

        /*Find the FONT tag*/
        else if(!strncmp("FONT", line, 4))
        {
            p = line + sizeof("FONT");
            //while(*p != ' ') p++;
            w = p;
            while(*p != ' ') p++;
            *p = '\0';
            strcpy(reg, w);
            if(!strcmp(reg, "GB2312"))
                strcpy(reg,"GB2312.1980-0");
            fi->type = CodingByRegistry(reg);
        }

        /* Find the CHARSET_ENCODING tag */
        else if (!strncmp("CHARSET_ENCODING", line, 16)) 
        {
            p = line + sizeof("CHARSET_ENCODING");
            while(*p != '"') p ++;
            w = ++p;
            while(*p != '"') p ++;
            *p = '\0';
            strcat(reg, "-");
            strcat(reg, w);
            fi->type = CodingByRegistry(reg);
        } 

        /* Find the CHARS tag */
        else if (!num && !strncmp("CHARS ", line, 6)) 
        {
            p = line + sizeof("CHARS");
            sscanf(p, "%d", &num);
            break;   /* Stop Here, the following are char bitmaps */
        }
    }

    fi->width = width;
    fi->high = high;
    pitch = (width + 7) / 8;

    printf("font width :%d, height: %d, pitch: %d\n",width, high, pitch);

    if (fi->type & CHR_DBC)   //double charset
    {
        fReg = &fDRegs[fi->type & ~CHR_DFLD];
        if (fReg->max)
            fi->size = fReg->addr(fReg->max >> 8, fReg->max & 0xFF)
                + 32;  // 16; 
        else
            fi->size = (width / 8 + ((width % 8 > 0) ? 1: 0)) * num * 16;
    } 
    else  // non-double charset
    {
        fReg = &fSRegs[fi->type & ~CHR_SFLD];
        if (fReg->max)
            fi->size = (pitch * high) * (fReg->max + 1);
        else
            fi->size = (pitch * high) * num;
    }

    width= 0;
    //if ((fdata = (u_char *)malloc(fi->size)) == NULL) 
    //    return(NULL);
    fi->size =0;
    num =0;

    printf("begin to read bitmap\n");
    while(fgets(line, 256, fp)) 
    {
        if(!strncmp("BBX", line, 3)) 
        {
            p = line + sizeof("BBX");
            bbw =bbh =bbx =bby =0;
            sscanf(p,"%d %d %d %d",&bbw, &bbh, &bbx, &bby);
        } 
        else if (!strncmp("BITMAP", line, strlen("BITMAP")))
        {
            if(0 ==bbw)
                continue;
            fillhigh =(fi->high -bbh -bby)/2;
            for(i=0; i< fillhigh; i++)
            {
                for(j =0;j< pitch; j++)
                {
                    fwrite(&fillchar,1,1,out);
                }
            }
            for (i = 0; i < bbh; i ++, p ++) 
            {
                fgets(line, 256, fp);
                p =line;
                while(*p && *p != '\n')
                {
                    sscanf(p, "%2X", &data);
                    bitmapdata =(data & 0xFF);
                    fwrite(&bitmapdata, 1, 1, out);
                    p +=2;
                }
            }
            for(i=0; i< (fi->high -fillhigh -bbh); i++)
            {
                for(j =0; j< pitch; j++)
                {
                    fwrite(&fillchar,1,1,out);
                }
            }
            fi->size = fi->size +pitch * fi->high;
            num ++;
        }
#if 0
        if (!strncmp("ENCODING", line, strlen("ENCODING"))) 
        {
            p = line + sizeof("ENCODING");
            code = atoi(p);
        } 
        else if (!strncmp("BITMAP", line, strlen("BITMAP"))) 
        {
            p = fdata + code * (pitch * high);
            /* Non-double charset */
            if (!(fi->type & CHR_DBC)) {
                for (i = 0; i < fi->high; i ++) {
                    for (j = 0; j < pitch; j++, p++) {
                        fscanf(fp, "%2X", &data);
                        *p = (data & 0xFF);
                    }
                }
            } 
            else  /* double charset */
            {
                ch = (code >> 8) & 0xFF;
                ch2 = code & 0xFF;
                num = fReg->addr(ch, ch2);
                if (num > width) width = num;
                p = fdata + num;
                for (i = 0; i < fi->high; i ++, p ++) 
                {
                    fscanf(fp, "%4X", &data);
                    *p = (data >> 8) & 0xFF;
                    p ++;
                    *p = data & 0xFF;
                }
            }
        }
#endif
    }
    
    printf("total glyphs: %d\n",num);

    return(fdata);
}

