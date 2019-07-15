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
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
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

/* font.h -- font information */

#ifndef	FONT_H
#define	FONT_H

#define CHR_SFLD        0x80
#define CHR_DBC         0x20
#define CHR_DFLD        (CHR_SFLD|CHR_DBC)

#define SHMEM_NAME      CONFIG_NAME

typedef struct  _FontInfo
{
    u_int size;           /* Allocated/Shared memory size */
    u_char high, width;   /* Font Width/Height */
    u_char type;          /* Font Coding,  Index|CHR_SFLD or Index|CHR_DFLD */
}FontInfo;

#define FR_ATTACH       1
#define FR_PROXY        2

typedef struct _FontRegs 
{
    u_int (*addr)(u_char ch1, u_char ch2);   /* calculate address */
    u_int size;                              /* memory size */ 
    char *registry, *bitmap;                 /* registry and bitmap pointer */
    u_char high, width;                      /* char parameters */
    u_char sign0, sign1;
    u_char stat;                             /* FR_ATTACH or FR_PROXY */
    u_int max;                               /* Number of Chars */
}FontRegs;

typedef struct _LangInfo 
{
    u_char
	sb,  /* single byte font index , without CHR_SFLD, CHR_DFLD */
	db,  /* double byte font index */
	sysCoding;  /* system coding index CODE_BIG5, CODE_GB*/
}LangInfo;

extern LangInfo lInfo;

extern FontRegs fSRegs[], fDRegs[];
extern FontRegs *sbFReg, *dbFReg;

void FontInit(void);
void FontAttach(void);
void FontDetach(int remove);

#endif
