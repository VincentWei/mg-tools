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
#ifdef _MGCHARSET_ARABIC

//#include "charset-bidi.h"

typedef struct _SHAPEINFO {
    unsigned short int  isolated;
    unsigned short int  final;
    unsigned short int  initial;
    unsigned short int  medial;
}SHAPEINFO;

#define SHAPENUMBER 36
static const SHAPEINFO shape_info[SHAPENUMBER] = {
      /*        Base                 Isol    Final    Initial    Medial */
    { /* 0  0xC1 (0x0621) */ 0xC1/*0xFE80*/, 0x00,           0x00,           0x00 /* Arabic letter Hamza                 */ },
    { /* 1  0xC2 (0x0622) */ 0xC2/*0xFE81*/, 0xC2/*0xFE82*/, 0x00,           0x00 /* Arabic letter Alef with Madda above */ },
    { /* 2  0xC3 (0x0623) */ 0xC3/*0xFE83*/, 0xC3/*0xFE84*/, 0x00,           0x00 /* Arabic letter Alef with Hamza above */ },
    { /* 3  0xC4 (0x0624) */ 0xC4/*0xFE85*/, 0xC4/*0xFE86*/, 0x00,           0x00 /* Arabic letter Waw with Hamza above  */ },
    { /* 4  0xC5 (0x0625) */ 0xC5/*0xFE87*/, 0xC5/*0xFE88*/, 0x00,           0x00 /* Arabic letter Alef with Hamza below */ },
    { /* 5  0xC6 (0x0626) */ 0xC6/*0xFE89*/, 0xC6/*0xFE8A*/, 0xC0/*0xFE8B*/, 0xC0 /*0xFE8C*/ /* Arabic letter Yeh with Hamza above  */ },
    { /* 6  0xC7 (0x0627) */ 0xC7/*0xFE8D*/, 0xC7/*0xFE8E*/, 0x00,           0x00 /* Arabic letter Alef                             */ },
    { /* 7  0xC8 (0x0628) */ 0xC8/*0xFE8F*/, 0xC8/*0xFE90*/, 0xEB/*0xFE91*/, 0xEB /*0xFE92*/ /* Arabic letter Beh                   */ },
    { /* 8  0xC9 (0x0629) */ 0xC9/*0xFE93*/, 0x8E/*0xFE94*/, 0x00,           0x00 /* Arabic letter Teh Marbuta                      */ },
    { /* 9  0xCA (0x062A) */ 0xCA/*0xFE95*/, 0xCA/*0xFE96*/, 0xEC/*0xFE97*/, 0xEC /*0xFE98*/ /* Arabic letter Teh                   */ },
    { /* 10 0xCB (0x062B) */ 0xCB/*0xFE99*/, 0xCB/*0xFE9A*/, 0xED/*0xFE9B*/, 0xED /*0xFE9C*/ /* Arabic letter Theh                  */ },
    { /* 11 0xCC (0x062C) */ 0xCC/*0xFE9D*/, 0xCC/*0xFE9E*/, 0xEE/*0xFE9F*/, 0xEE /*0xFEA0*/ /* Arabic letter Jeem                  */ },
    { /* 12 0xCD (0x062D) */ 0xCD/*0xFEA1*/, 0xCD/*0xFEA2*/, 0xEF/*0xFEA3*/, 0xEF /*0xFEA4*/ /* Arabic letter Hah                   */ },
    { /* 13 0xCE (0x062E) */ 0xCE/*0xFEA5*/, 0xCE/*0xFEA6*/, 0xF0/*0xFEA7*/, 0xF0 /*0xFEA8*/ /* Arabic letter Khah                  */ },
    { /* 14 0xCF (0x062F) */ 0xCF/*0xFEA9*/, 0xCF/*0xFEAA*/, 0x00,           0x00 /* Arabic letter Dal                   */ },
    { /* 15 0xD0 (0x0630) */ 0xD0/*0xFEAB*/, 0xD0/*0xFEAC*/, 0x00,           0x00 /* Arabic letter Thal                  */ },
    { /* 16 0xD1 (0x0631) */ 0xD1/*0xFEAD*/, 0xD1/*0xFEAE*/, 0x00,           0x00 /* Arabic letter Reh                   */ },
    { /* 17 0xD2 (0x0632) */ 0xD2/*0xFEAF*/, 0xD2/*0xFEB0*/, 0x00,           0x00 /* Arabic letter Zain                  */ },
    { /* 18 0xD3 (0x0633) */ 0xD3/*0xFEB1*/, 0x8F/*0xFEB2*/, 0xF1/*0xFEB3*/, 0xF1 /*0xFEB4*/ /* Arabic letter Seen                  */ },
    { /* 19 0xD4 (0x0634) */ 0xD4/*0xFEB5*/, 0x90/*0xFEB6*/, 0xF2/*0xFEB7*/, 0xF2 /*0xFEB8*/ /* Arabic letter Sheen                 */ },
    { /* 20 0xD5 (0x0635) */ 0xD5/*0xFEB9*/, 0x91/*0xFEBA*/, 0xF3/*0xFEBB*/, 0xF3 /*0xFEBC*/ /* Arabic letter Sad                   */ },
    { /* 21 0xD6 (0x0636) */ 0xD6/*0xFEBD*/, 0x92/*0xFEBE*/, 0xF4/*0xFEBF*/, 0xF4 /*0xFEC0*/ /* Arabic letter Dad                   */ },
    { /* 22 0xD7 (0x0637) */ 0xD7/*0xFEC1*/, 0x93/*0xFEC2*/, 0xD7/*0xFEC3*/, 0x93 /*0xFEC4*/ /* Arabic letter Tah                   */ },
    { /* 23 0xD8 (0x0638) */ 0xD8/*0xFEC5*/, 0x94/*0xFEC6*/, 0xD8/*0xFEC7*/, 0x94 /*0xFEC8*/ /* Arabic letter Zah                   */ },
    { /* 24 0xD9 (0x0639) */ 0xD9/*0xFEC9*/, 0x96/*0xFECA*/, 0xF5/*0xFECB*/, 0x95 /*0xFECC*/ /* Arabic letter Ain                   */ },
    { /* 25 0xDA (0x063A) */ 0xDA/*0xFECD*/, 0x98/*0xFECE*/, 0xF6/*0xFECF*/, 0x97 /*0xFED0*/ /* Arabic letter Ghain                 */ },
    { /* 26 0xE1 (0x0641) */ 0xE1/*0xFED1*/, 0xE1/*0xFED2*/, 0xF7/*0xFED3*/, 0x99 /*0xFED4*/ /* Arabic letter Feh                   */ },
    { /* 27 0xE2 (0x0642) */ 0xE2/*0xFED5*/, 0xE2/*0xFED6*/, 0xF8/*0xFED7*/, 0x9A /*0xFED8*/ /* Arabic letter Qaf                   */ },
    { /* 28 0xE3 (0x0643) */ 0xE3/*0xFED9*/, 0xE3/*0xFEDA*/, 0xF9/*0xFEDB*/, 0x9B /*0xFEDC*/ /* Arabic letter Kaf                   */ },
    { /* 29 0xE4 (0x0644) */ 0xE4/*0xFEDD*/, 0xE4/*0xFEDE*/, 0xFA/*0xFEDF*/, 0xFA /*0xFEE0*/ /* Arabic letter Lam                   */ },
    { /* 30 0xE5 (0x0645) */ 0xE5/*0xFEE1*/, 0xE5/*0xFEE2*/, 0xFB/*0xFEE3*/, 0xFB /*0xFEE4*/ /* Arabic letter Meem                  */ },
    { /* 31 0xE6 (0x0646) */ 0xE6/*0xFEE5*/, 0xE6/*0xFEE6*/, 0xFC/*0xFEE7*/, 0xFC /*0xFEE8*/ /* Arabic letter Noon                  */ },
    { /* 32 0xE7 (0x0647) */ 0xE7/*0xFEE9*/, 0x9D/*0xFEEA*/, 0xFD/*0xFEEB*/, 0x9C /*0xFEEC*/ /* Arabic letter Heh                   */ },
    { /* 33 0xE8 (0x0648) */ 0xE8/*0xFEED*/, 0xE8/*0xFEEE*/, 0x00,           0x00 /* Arabic letter Waw                   */ },
    { /* 34 0xE9 (0x0649) */ 0x8D/*0xFEEF*/, 0xE9/*0xFEF0*/, 0x00,           0x00 /* Arabic letter Alef Maksura          */ },
    { /* 35 0xEA (0x064A) */ 0x9E/*0xFEF1*/, 0xEA/*0xFEF2*/, 0xFE/*0xFEF3*/, 0xFE /*0xFEF4*/ /* Arabic letter Yeh                   */ },
};

/************************* ISO8859-6 Specific Operations **********************/
static int iso8859_6_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-6"))
        return 0;

    if (strstr (name, "ARABIC"))
        return 0;

    return 1;
}

#ifdef _MGCHARSET_UNICODE
static unsigned short iso8859_68x_unicode_map [] =
{
    0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff,
    0xfeef, 0xfe94, 0xfeb2, 0xfeb6,   /* 0x8d~0x90*/
    0xfeba, 0xfebe,
    0xfec2, 0xfec8,   /*0x93, 0x94*/ /*0xfec2/xfec4 0xfec8/0xfec6*/
    0xfecc, 0xfeca, 0xfed0, 0xfece,
    0xfed4, 0xfed8, 0xfedc, 0xfeec,  
    0xfeea, 0xfef1, 0xfe89, 0x0020,
    0xffff, 0xffff, 0xffff, 0xffff,   /*0xa1~0xa4*/  
    0xffff, 0xffff, 0xffff,           /*0xa5, 0xa6, 0xa7*/
    0xfe70, 0xfe72, 0xfe74, 0xfe76,
    0xfe78, 0xfe7a, 0xfe7c, 0xfe7e,
    0x0660, 0x0661, 0x0662, 0x0663,
    0x0664, 0x0665, 0x0666, 0x0667,
    0x0668, 0x0669,
    0x060c, 0x061b, 0xfe71,
    0xffff, 0xffff,    /*0xbd, 0xbe*/
    0x061f,
    0xfe8b, /*0xc0*/
    0xfe80,
    0xfe81, 0xfe83,  0xfe85, 0xfe87,/*0xc2~0xc5*/
    0xfe8a, 0xfe8d,  0xfe8f, 0xfe93,/*0xc6~0xc9*/
    0xfe95, 0xfe99,  0xfe9d, 0xfea1,/*0xca~0xcd*/
    0xfea5, 0xfeaa,  0xfeab, 0xfead,/*0xce~0xd1*/
    0xfeaf, 0xfeb1,  0xfeb5, 0xfeb9,/*0xd2~0xd5*/
    0xfebd, 0xfec1,  0xfec5, 0xfec9,/*0xd6~0xd9*/
    0xfecd, /*0xda*/
    0xfe77, 0xfe79,  0xfe7b, 0xfe7d,
    0xfe7f, 0x0640,
    0xfed1, 0xfed5,  0xfed9, 0xfedd,/*0xe1~0xe4*/
    0xfee1, 0xfee5,  0xfee9, 0xfeed,/*0xe5~0xe8*/
    0xfeef, 0xfef1,  0xfe91, 0xfe97,/*0xe9~0xec*/
    0xfe9b, 0xfe9f,  0xfea3, 0xfea7,/*0xed~0xf0*/
    0xfeb3, 0xfeb7,  0xfebb, 0xfebf,/*0xf1~0xf4*/
    0xfecb, 0xfecf,  0xfed3, 0xfed7,/*0xf5~0xf8*/
    0xfedb, 0xfedf,  0xfee3, 0xfee7,/*0xf9~0xfc*/
    0xfeeb, 0xfef4,  /*0xfd~0xfe*/
};

static UChar32 iso8859_6_conv_to_uc32 (Glyph32 glyph_value)
{
    if (glyph_value < 0x81)
        return (Glyph32) (glyph_value);
    else{
        static Glyph32 prev_glyph = 0xffff, ucs2_glyph = 0xffff;
        ucs2_glyph = (Glyph32) iso8859_68x_unicode_map[glyph_value - 0x81];

        /* fontset 6.8x 0xa1~0xa6 is translate to unicode here.*/
        if(prev_glyph >= 0xa1 && prev_glyph <= 0xa4){
            if(ucs2_glyph == 0xa5){       /* ligature initial shape.*/
                switch(prev_glyph){
                    case 0xa1:
                        ucs2_glyph = 0xfefb;
                        break;
                    case 0xa2:
                        ucs2_glyph = 0xfef5;
                        break;
                    case 0xa3:
                        ucs2_glyph = 0xfef7;
                        break;
                    case 0xa4:
                        ucs2_glyph = 0xfef9;
                        break;
                }
            }
            else if(ucs2_glyph == 0xa6){ /* ligature final shape.*/
                switch(prev_glyph){
                    case 0xa1:
                        ucs2_glyph = 0xfefc;
                        break;
                    case 0xa2:
                        ucs2_glyph = 0xfef6;
                        break;
                    case 0xa3:
                        ucs2_glyph = 0xfef8;
                        break;
                    case 0xa4:
                        ucs2_glyph = 0xfefa;
                        break;
                }
            }
        }
        return ucs2_glyph;
    }

}

static int iso8859_6_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    switch (wc) {
        case 0x060C:
            *mchar = 0xAC;
            return 1;
        case 0x061B:
            *mchar = 0xBB;
            return 1;
        case 0x061F:
            *mchar = 0xBF;
            return 1;
    }

    if (wc < 0xC1) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    if (wc >= 0x0621 && wc <= (0x0621 + 0xF2 - 0xC1)) {
        *mchar = (unsigned char) (wc - 0x0621 + 0xC1);
        return 1;
    }

    return 0;
}
#endif

/* must attetion the para s is arabic glyph value.*/
static int is_arabic_glyph_vowel(Uint8 c)
{

    if ((c >= 0x81) && (c <= 0x86)) return 1;
    if ((c >= 0xa8) && (c <= 0xaf)) return 1;

    /* unicode vowel range. */
    /* if ((s >= 0x64B) && (s <= 0x655)) return 1;
       if ((s >= 0xFC5E) && (s <= 0xFC63)) return 1;
       if ((s >= 0xFE70) && (s <= 0xFE7F)) return 1;
    */
    return 0;
}

/* ISO8859-6 charset vowel relative define and judge */
#define ISARABIC_VOWEL(s) ((s >= FATHATAN) && (s <= SUKUN))
#define ISARABIC_PUNC(s)  ((s == COMMA) || (s == SEMICOLON) || (s == DOLLAR))

#define ALIF       0xC7
#define ALIFHAMZA  0xC5
#define ALIFAHAMZA 0xC3
#define ALIFMADDA  0xC2  //ARABIC LETTER ALEF WITH MADDA ABOVE
#define LAM        0xE4

/* ISO 8859-6 punctuation mark.*/
#define COMMA      0xAC
#define SEMICOLON  0xBB
#define QUESTION   0xBF
#define DOLLAR     0xA4

#define TADWEEL    0xE0
#define FATHATAN   0xEB
#define DAMMATAN   0xEC
#define KASRATAN   0xED
#define FATHA      0xEE
#define DAMMA      0xEF
#define KASRA      0xF0
#define SHADDA     0xF1
#define SUKUN      0xF2

/* this define is relative with fontset 0xa1~0xa7.
 * it it used for ligature such as LAM+ALEF, one ligature
 * have two fontset glyphs.*/
#define LAM_ALIF         0xA1A5
#define LAM_ALIF_F       0xA1A6
#define LAM_ALIFMADDA    0xA2A5
#define LAM_ALIFMADDA_F  0xA2A6
#define LAM_ALIFHAMZA    0xA4A5
#define LAM_ALIFHAMZA_F  0xA4A6  //ARABIC LETTER ALEF WITH HAMZA ABOVE
#define LAM_ALIFAHAMZA   0xA3A5
#define LAM_ALIFAHAMZA_F 0xA3A6  //ARABIC LETTER ALEF WITH HAMZA BELOW


/* Because the get_ligature is close relative with fontset 6.8x, so
 * do it in the follow five functions, if the fontset is change, you only
 * need to implement follow five interface.
 * 1. get_vowel_glyph().
 * 2. get_twovowel_glyph().
 * 3. get_tadweel_glyph().
 * 4. get_ligature_glyph().
 * 5. get_punpoint_glyph().
 * houhh 20080128.
 * */

static int fontset_68x_get_punpoint_glyph(Uint8 c)
{
    int ligature = -1;
    switch(c){
        case  COMMA:     ligature = 0xba; break;
        case  SEMICOLON: ligature = 0xbb; break;
        case  QUESTION:  ligature = 0xbf; break;
        case  DOLLAR:    ligature = 0x24; break;
        default:         ligature = -1;   break; // this will not happen.
    }
    return ligature;
}

static int fontset_68x_get_vowel_glyph(Uint8 c)
{
    int ligature = -1;
    switch(c){
        case  FATHATAN: ligature = 0xa8; break;
        case  DAMMATAN: ligature = 0xa9; break;
        case  KASRATAN: ligature = 0xaa; break;
        case  FATHA:    ligature = 0xab; break;
        case  DAMMA:    ligature = 0xac; break;
        case  KASRA:    ligature = 0xad; break;
        case  SHADDA:   ligature = 0xae; break;
        case  SUKUN:    ligature = 0xaf; break;
        default:        ligature = -1;   break; // this will not happen.
    }
    return ligature;
}

static int fontset_68x_get_twovowel_glyph(unsigned char c, unsigned char next, int* ignore)
{
    int ligature = -1;
    if(c == SHADDA){
        switch(next){
            case  FATHATAN: *ignore = 1; ligature = 0x81; break;
            case  DAMMATAN: *ignore = 1; ligature = 0x82; break;
            case  KASRATAN: *ignore = 1; ligature = 0x83; break;
            case  FATHA:    *ignore = 1; ligature = 0x84; break;
            case  DAMMA:    *ignore = 1; ligature = 0x85; break;
            case  KASRA:    *ignore = 1; ligature = 0x86; break;
            default:        *ignore = 0; ligature = 0xae; break; //FIX BUG, only SHADDA.
        }
    }
    else {
        *ignore = 0;
        ligature = fontset_68x_get_vowel_glyph(c);
    }
    return ligature;
}

static int fontset_68x_get_ligature_glyph(unsigned char c, BOOL prev_affects_joining, int* ignore)
{
    int ligature = -1;
    if(prev_affects_joining){
        switch (c){
            case ALIF:       *ignore = 1; ligature = LAM_ALIF_F; break;
            case ALIFHAMZA:  *ignore = 1; ligature = LAM_ALIFHAMZA_F; break;
            case ALIFAHAMZA: *ignore = 1; ligature = LAM_ALIFAHAMZA_F;break;
            case ALIFMADDA:  *ignore = 1; ligature = LAM_ALIFMADDA_F; break;
            default:         *ignore = 0; ligature = -1; break; // FIX BUG, later do the shape continue.
        }
    }
    else{
        switch (c){
            case ALIF:       *ignore = 1; ligature = LAM_ALIF; break;
            case ALIFHAMZA:  *ignore = 1; ligature = LAM_ALIFHAMZA; break;
            case ALIFAHAMZA: *ignore = 1; ligature = LAM_ALIFAHAMZA;break;
            case ALIFMADDA:  *ignore = 1; ligature = LAM_ALIFMADDA; break;
            default:         *ignore = 0; ligature = -1; break;
        }
    }
    return ligature;
}

static int fontset_68x_get_tadweel_glyph(unsigned char c, unsigned char next, int* ignore)
{
    int ligature = -1;
    if(c == SHADDA){
        /* TADWEEL combine with two vowel except SUKUN 
         * can not combine with SHADDA.*/
        switch(next){
            case  FATHATAN: *ignore = 2; ligature = 0x87; break;
            case  DAMMATAN: *ignore = 2; ligature = 0x88; break;
            case  KASRATAN: *ignore = 2; ligature = 0x89; break;
            case  FATHA:    *ignore = 2; ligature = 0x8a; break;
            case  DAMMA:    *ignore = 2; ligature = 0x8b; break;
            case  KASRA:    *ignore = 2; ligature = 0x8c; break;
            default:        *ignore = 1; ligature = 0xde; break; // FIX BUG combine of TADWELL SHADDA.
        }
    }
    else {
        /* TADWEEL combine withe one vowel. */
        switch(next){
            case  FATHATAN: *ignore = 1; ligature = 0xbc; break;
            case  DAMMATAN: *ignore = 1; ligature = 0xbd; break;
            case  KASRATAN: *ignore = 1; ligature = 0xbe; break;
            case  FATHA:    *ignore = 1; ligature = 0xdb; break;
            case  DAMMA:    *ignore = 1; ligature = 0xdc; break;
            case  KASRA:    *ignore = 1; ligature = 0xdd; break;
            case  SHADDA:   *ignore = 1; ligature = 0xde; break;
            case  SUKUN:    *ignore = 1; ligature = 0xdf; break;
            default:        *ignore = 0; ligature = TADWEEL; break; // FIX BUG of only TADWEEL.
        }
    }
    return ligature;
}

#ifndef _DEBUG
static 
#endif
int get_ligature(const unsigned char* mchar, int len, BOOL prev_affects_joining, int* ignore)
{
    int ligature = -1;
    Uint8 cur_char, next, next_next;

    if(ignore) *ignore = 0;

    cur_char = *mchar;
    
    ligature = fontset_68x_get_punpoint_glyph(cur_char);
    if(ligature > 0) return ligature;
    
    if(len == 1){
        if (ISARABIC_VOWEL(cur_char)){
            ligature = fontset_68x_get_vowel_glyph(cur_char);
        }
    }
    else if(len > 1){
        next = *(mchar+1);
        if (ISARABIC_VOWEL(cur_char)){ /* two VOWEL, one must be SHADDA first. */
            ligature = fontset_68x_get_twovowel_glyph(cur_char, next, ignore);
        }
        else if (cur_char == LAM) {    /* LAM+ALEF+HAMAZ+MADDA ligature. */
            ligature = fontset_68x_get_ligature_glyph(cur_char, prev_affects_joining, ignore);
        } 
        else if(cur_char == TADWEEL){  /* TADWEEL combine with VOWEL*/
            if(len < 3) next_next = *(mchar+2);
            else next_next = 0;
            ligature = fontset_68x_get_tadweel_glyph(next, next_next, ignore);
        }
    }
    else{
        *ignore = 0;
    }

    return ligature;
}

static int iso8859_6_len_first_char (const unsigned char* mstr, int len)
{
    int ignore = 0;

    /* if ligature, ligature will have two or three bytes.*/
    get_ligature (mstr, len, FALSE, &ignore);

    if (ignore == 1)
        return 2;
    else if (ignore == 2)
        return 3;
    else 
        return 1;

}

#define ISARABIC_LIG_HALF(s) ((s == 0xa5) || (s == 0xa6))

/* note here mchar is fontset code glyph value. */ 
static unsigned int iso8859_6_glyph_type (Glyph32 glyph_value)
{
    unsigned int ch_type = MCHAR_TYPE_UNKNOWN;

    if (is_arabic_glyph_vowel (glyph_value)){  /* is vowel */
        ch_type = MCHAR_TYPE_VOWEL;
    }
    else if (ISARABIC_LIG_HALF (glyph_value)){ /* is 0xA5 or 0xA6 */
        ch_type = sb_glyph_type (glyph_value);
    }
    else{
        ch_type = sb_glyph_type (glyph_value);
    }

    return ch_type;
}

static int get_table_index(Uint8 c)
{
    /* is arabic letter range. */
    if (c < 0xc0 || c == 0xe0) {
        return -1;
    }
    if (c > 0xea) {
        return -1;
    }
    /* first continue char range.*/
    if ((c >= 0xc1) && (c <= 0xda)) {
        return c - 0xc1;
    }
    /* second continue char shape range.*/
    if ((c > 0xe0) && (c <= 0xea)) {
        return (c - 0xe0) + (0xda- 0xc1);
    }
    return -1;

}

/* must attetion the para s is arabic letter value.*/
static int is_arabic_letter_vowel(Uint8 s)
{
    if ((s >= 0xeb) && (s <= 0xf2)) return 1;
    return 0;
}

/* arabic letter will affect shape, ascii or space will not. */
static int is_char_transparent(Uint8 c)
{
    BOOL is_ascii = (c < 0x7f) ? 1 : 0;
    BOOL is_space  = ((c == 0x20) || (c == 0xa0)) ? 1 : 0;
    BOOL is_punctuation = ((c == 0xac) || (c == 0xbb) || (c == 0xbf)) ? 1 : 0;
    BOOL is_vowel = is_arabic_letter_vowel(c);
    if(is_ascii || is_space || is_punctuation || is_vowel)
        return 0;
    else return 1;
}

static int get_next_char(const unsigned char* mchar, int len)
{
    int next_char = *mchar;
    int left_bytes = len, len_cur_char = 0;

    /* skip all vowel, get the next_char. */
    while (left_bytes > 0) {
        len_cur_char = iso8859_6_len_first_char(mchar, left_bytes);
        if (len_cur_char > 0 && is_arabic_letter_vowel(*(mchar+len_cur_char))) {
            left_bytes -= len_cur_char;
            mchar += len_cur_char;
        }
        else{
            next_char = *(mchar+len_cur_char);
            break;
        }
    }
    return next_char;
}

Glyph32 iso8859_6_char_glyph_value (const unsigned char* prev_mchar, int prev_len, const unsigned char* mchar, int len)
{
    BOOL next_affects_joining = FALSE, prev_affects_joining = FALSE;
    int char_index, prev_index;
    int final, initial, medial, ligature;
    int ignore;
    char next_char = 0, prev_char = 0;

    if(*mchar < 0x7f) return *mchar;

    char_index = get_table_index(*mchar);

    if (char_index < 0 && !ISARABIC_PUNC(*mchar) && !ISARABIC_VOWEL(*mchar) && !(*mchar == TADWEEL)) {
        return *mchar;
    }

    if(prev_mchar){
        prev_index = get_table_index(*prev_mchar);
        prev_affects_joining = ( prev_index >= 0 || is_char_transparent(*prev_mchar)) && (shape_info[prev_index].medial);
        prev_char = *prev_mchar;
    }

    /* processing ligature first.*/
    ligature = get_ligature(mchar, len, prev_affects_joining, &ignore);

    if (ligature > 0) 
        return ligature;

    /* if not ligature char, get it's relative shape from shape_info table.*/
    //next_char = *(mchar + iso8859_6_len_first_char(mchar, len));
    next_char = get_next_char(mchar, len);
    next_affects_joining = ((get_table_index(next_char) > 0 || is_char_transparent(next_char)) && (shape_info[char_index].medial));

    /* 1.prev and next char not affect, return isoliate */
    if ((!prev_affects_joining) && (!next_affects_joining)) {
        return shape_info[char_index].isolated;
    }
    /* 2.only next char affect，if has Initial，return Initial; else return Isolated */
    else if ((!prev_affects_joining) && (next_affects_joining)) { 
        initial = shape_info[char_index].initial;
        if (initial) 
            return initial;
        else 
            return shape_info[char_index].isolated;  
    } 
    /* 3.prev and next char affect all，if has Medial， return Medial; else Isolated */
    else if ((prev_affects_joining) && (next_affects_joining)) { 
        medial = shape_info[char_index].medial;
        if (medial) 
            return medial;
        else 
            return shape_info[char_index].isolated;
    } 
    /* 4.only prev char affect，if has Final, return Final; else Isolated */
    else if ((prev_affects_joining) && (!next_affects_joining)) { 
        final = shape_info[char_index].final;
        if (final)
            return final;
        else 
            return shape_info[char_index].isolated;
    }

    return *mchar;
}

static const unsigned char* iso8859_6_get_next_word (const unsigned char* mstr,
        int mstrlen, WORDINFO* word_info)
{
    int i;

    word_info->len = 0;
    word_info->delimiter = '\0';
    word_info->nr_delimiters = 0;

    if (mstrlen == 0) return NULL;

    for (i = 0; i < mstrlen; i++) {
        switch (mstr[i]) {
            case 0xa0:
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if (word_info->delimiter == '\0') {
                    word_info->delimiter = mstr[i];
                    word_info->nr_delimiters ++;
                }
                else if (word_info->delimiter == mstr[i])
                    word_info->nr_delimiters ++;
                else
                    return mstr + word_info->len + word_info->nr_delimiters;
                break;

            default:
                if (word_info->delimiter != '\0')
                    break;

                word_info->len++;
        }
    }
            
    return mstr + word_info->len + word_info->nr_delimiters;

    //return sb_get_next_word(mstr, mstrlen, word_info);
}

static CHARSETOPS CharsetOps_iso8859_6 = {
    256,
    3,
    FONT_CHARSET_ISO8859_6,
    0,
    iso8859_6_len_first_char,
    iso8859_6_char_glyph_value,
    iso8859_6_glyph_type,
    sb_nr_chars_in_str,
    iso8859_6_is_this_charset,
    sb_len_first_substr,
    iso8859_6_get_next_word,
    sb_pos_first_char,
    NULL,
#ifdef _MGCHARSET_UNICODE
    iso8859_6_conv_to_uc32,
    iso8859_6_conv_from_uc32
#endif
};

#ifdef _DEBUG
int test_glyph_value (int char_index, Glyph32 glyph_value)
{
    int i = 0;
    for (i = 0; i<SHAPENUMBER; i++) {
        if (shape_info[i].isolated == glyph_value ||
            shape_info[i].initial == glyph_value ||
            shape_info[i].medial == glyph_value ||
            shape_info[i].final == glyph_value )
            if( char_index == i)
                return 1;
    }
    return 0;
}
#endif


#endif /* _ARABIC */
