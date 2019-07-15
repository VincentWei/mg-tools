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

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>

#include	"font.h"

FontRegs *dbFReg, *sbFReg;

/**************************************************************************
 *              Coding Processing Routines and Structures                 *
 **************************************************************************/

/* These are char internal codes to address(offset) conversion routines
   Chinese, Japanese, Korean coding */

static u_int JISX0208(u_char ch1, u_char ch2)
{
    ch1 &= 0x7F;
    ch2 &= 0x7F;

    if (ch1 > 0x2A)
        return((ch2 - 0x41 + (ch1 - 0x26) * 96) << 5);
    else
        return((ch2 - 0x21 + (ch1 - 0x21) * 96) << 5);
}

static u_int GB2312(u_char ch1, u_char ch2)
{
    ch1 &= 0x7F;  // - 0x80
    ch2 &= 0x7F;  // - 0x80 A0A0 - 8080=2020

    if (ch1 > 0x29)
        return(((ch1 - 0x27) * 94 + ch2 - 0x21) << 5);
    else
        return(((ch1 - 0x21) * 94 + ch2 - 0x21) << 5);
}

static u_int BIG5(u_char ch1, u_char ch2)
{
    if (ch2 < 0xA1)
        return(((ch1 - 0xA1) * 157 + ch2 - 0x40) << 5);
    else
        return(((ch1 - 0xA1) * 157 + 63 + ch2 - 0xA1) << 5);
}

static u_int KSC5601(u_char ch1, u_char ch2)
{
    ch1 &= 0x7F;
    ch2 &= 0x7F;

    if (ch1 > 0x2D)
        return((ch2 - 0x21 + (ch1 - 0x24) * 96) << 5);
    else
        return((ch2 - 0x21 + (ch1 - 0x21) * 96) << 5);
}

/* single byte fontRegs */
FontRegs fSRegs[] = {
    /* latin1(French, Spanish, ...) */
    {    NULL, -1,      "ISO8859-1", NULL, 0, 0, 'B', 'A', 0, 0xFF},
    /* latin2 */
    {    NULL, 0,      "ISO8859-2", NULL, 0, 0, 'B', 'B', 0, 0xFF},
    /* latin3 */
    {    NULL, 0,      "ISO8859-3", NULL, 0, 0, 'B', 'C', 0, 0xFF},
    /* latin4 */
    {    NULL, 0,      "ISO8859-4", NULL, 0, 0, 'B', 'D', 0, 0xFF},
    /* Russian */
    {    NULL, 0,      "ISO8859-5", NULL, 0, 0, 'B', 'L', 0, 0xFF},
    /* Arabic */
    {    NULL, 0,      "ISO8859-6", NULL, 0, 0, 'B', 'G', 0, 0xFF},
    /* Greek */
    {    NULL, 0,      "ISO8859-7", NULL, 0, 0, 'B', 'F', 0, 0xFF},
    /* Hebrew */
    {    NULL, 0,      "ISO8859-8", NULL, 0, 0, 'B', 'H', 0, 0xFF},
    /* latin5 */
    {    NULL, 0,      "ISO8859-9", NULL, 0, 0, 'B', 'M', 0, 0xFF},
    /* latin6 */
    {    NULL, 0,      "ISO8859-10", NULL, 0, 0, 'B', 'M', 0, 0xFF},
    /* Thai */
    {    NULL, 0,      "ISO8859-11", NULL, 0, 0, 'B', 'M', 0, 0xFF},
    /* latin7 */
    {    NULL, 0,      "ISO8859-13", NULL, 0, 0, 'B', 'M', 0, 0xFF},
    /* latin8 */
    {    NULL, 0,      "ISO8859-14", NULL, 0, 0, 'B', 'M', 0, 0xFF},
    /* latin9 */
    {    NULL, 0,      "ISO8859-15", NULL, 0, 0, 'B', 'M', 0, 0xFF},
    /* VIETNAMESE */
    {    NULL, 0,      "TCVN5712-0", NULL, 0, 0, 'J', 'I', 0, 0xFF},
    /* Japanese */
    {    NULL, 0,"JISX0201.1976-0", NULL, 0, 0, 'J', 'I', 0, 0xFF},
    /* DUMB */
    {    NULL, 0,             NULL, NULL, 0, 0,   0,   0, 0,  0x0}
};

/* double byte fontRegs */
FontRegs fDRegs[] = {
    /* DF_GB2312 */
    { GB2312  ,  0,  "GB2312.1980-0", NULL, 0, 0, 'A', 0, 0, 0xF7FE},
    /* DF_JISX0208 */
    { JISX0208 , 0,"JISX0208.1983-0", NULL, 0, 0, 'B', 0, 0, 0x7424},
    /* DF_KSC5601 */
    { KSC5601 ,  0, "KSC5601.1987-0", NULL, 0, 0, 'C', 0, 0, 0x7D7E},
    /* DF_JISX0212 */
    { JISX0208 , 0,       "JISX0212", NULL, 0, 0, 'D', 0, 0, 0x7424},
    /* DF_BIG5_0 */
    {    BIG5,   0,     "BIG5.ET-0", NULL, 0, 0, '0', 0, 0,    0x0},
    /* DF_BIG5_1 */
    {    BIG5,   0,     "BIG5.HKU-0", NULL, 0, 0, '1', 0, 0,    0x0},
    /* DUMB */
    {    NULL,   0,             NULL, NULL, 0, 0,   0, 0, 0,    0x0}
};

int CodingByRegistry(char *reg)
{
    int i;

    i = 0;
    while (fSRegs[i].registry) 
    {
        if (!strncasecmp(fSRegs[i].registry, reg, strlen(reg)))
            return(i|CHR_SFLD);
        i ++;
    }

    i = 0;
    while (fDRegs[i].registry) 
    {
        if (!strncasecmp(fDRegs[i].registry, reg, strlen(reg)))
            return(i|CHR_DFLD);
        i ++;
    }
    return(-1);
}
