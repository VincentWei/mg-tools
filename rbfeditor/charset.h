/*
** $Id: charset.h 8944 2007-12-29 08:29:16Z xwyan $
**
** charset.h: the head file of charset operation set.
**
** Copyright (C) 2000 ~ 2002 Wei Yongming.
** Copyright (C) 2003 ~ 2007 Feynman Software.
*/

#ifndef GUI_FONT_CHARSET_H
    #define GUI_FONT_CHARSET_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define _MGCHARSET_ARABIC 1

#define _MGCHARSET_BIG5 1

#define _MGCHARSET_CYRILLIC 1

#define _MGCHARSET_EUCJP 1

#define _MGCHARSET_EUCKR 1

#define _MGCHARSET_GB 1

#define _MGCHARSET_GB18030 1

#define _MGCHARSET_GBK 1

#define _MGCHARSET_GREEK 1

#define _MGCHARSET_HEBREW 1

#define _MGCHARSET_LATIN10 1

#define _MGCHARSET_LATIN2 1

#define _MGCHARSET_LATIN3 1

#define _MGCHARSET_LATIN4 1

#define _MGCHARSET_LATIN5 1

#define _MGCHARSET_LATIN6 1

#define _MGCHARSET_LATIN7 1

#define _MGCHARSET_LATIN8 1

#define _MGCHARSET_LATIN9 1

//#define _MGCHARSET_SHIFTJIS 1
#undef _MGCHARSET_SHIFTJIS

#define _MGCHARSET_THAI 1

//#define _MGCHARSET_UNICODE 1
#undef _MGCHARSET_UNICODE

//typedef int Glyph32;

struct _CHARSETOPS
{
	/** The character number of the character set. */
    int nr_chars;

	/** The byte number of the max length character. */
    int bytes_maxlen_char;

	/** The name of the character set. */
    const char* name;

	/** Default character. */
    Glyph32 def_glyph_value;

	/** The method to get the length of the first character function. */
    int (*len_first_char) (const unsigned char* mstr, int mstrlen);

	/** The method to get character offset function. */
    Glyph32 (*char_glyph_value) (const unsigned char* pre_mchar, int pre_len,
                const unsigned char* cur_mchar, int cur_len);

	/** The method to get the type of one glyph. */
    unsigned int (*glyph_type) (Glyph32 glyph_value);

	/** The method to get character number in the string function. */
    int (*nr_chars_in_str) (const unsigned char* mstr, int mstrlen);

	/** The method to judge the \a charset is belong to the character set
	 *  function.
	 */
    int (*is_this_charset) (const unsigned char* charset);

	/** The method to get  the length of the first substring. */
    int (*len_first_substr) (const unsigned char* mstr, int mstrlen);

	/** The method to get next word in the specitied length string function. */
    const unsigned char* (*get_next_word) (const unsigned char* mstr, 
                int strlen, WORDINFO* word_info);

	/** The method to get the position of the first character in the
	 *  specified length string function.
	 */
    int (*pos_first_char) (const unsigned char* mstr, int mstrlen);

	 /** reorder bidi string specified length function.
	 */
    void (*bidi_reorder) (Glyph32* glyphs, int len);

#ifdef _MGCHARSET_UNICODE
	/** The method to convert \a mchar to 32 bit UNICODE function. */
    UChar32 (*conv_to_uc32) (Glyph32 glyph_value);

	/** The method to convert \a wc to multily byte character function. */
    int (*conv_from_uc32) (UChar32 wc, unsigned char* mchar);
#endif /* _UNICODE_SUPPORT */
};

//typedef struct _CHARSETOPS CHARSETOPS;
CHARSETOPS* get_charset_ops (const char* charset);
CHARSETOPS* get_charset_ops_ex (const char* charset);
BOOL is_compatible_charset (const char* charset, CHARSETOPS* ops);

#ifdef _MGCHARSET_GB
extern unsigned short gbunicode_map [];
#endif

#ifdef _MGCHARSET_GBK
extern unsigned short gbkunicode_map [];
#endif

#ifdef _MGCHARSET_GB18030
extern unsigned short gb18030_0_unicode_map [];
#endif

#ifdef _MGCHARSET_BIG5
extern unsigned short big5_unicode_map [];
#endif

#ifdef _MGCHARSET_EUCKR
extern unsigned short ksc5601_0_unicode_map [];
#endif

#ifdef _MGCHARSET_EUCJP
extern unsigned short jisx0208_0_unicode_map [];
#endif

#ifdef _MGCHARSET_SHIFTJIS
extern unsigned short jisx0208_1_unicode_map [];
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_FONT_CHARSET_H

