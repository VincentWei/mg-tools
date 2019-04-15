/*
** Copyright (C) 2009 FMSoft
** Author: WEI Yongming
** Create date: 2009/12/19
*/

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define FM_SMOOTH   1

typedef struct _QPFGLYPHMETRICS {
    Uint8 linestep;
    Uint8 width;
    Uint8 height;
    Uint8 padding;

    Sint8 bearingx;     /* Difference from pen position to glyph's left bbox */
    Uint8 advance;      /* Difference between pen positions */
    Sint8 bearingy;     /* Used for putting characters on baseline */

    Sint8 reserved;     /* Do not use */
} QPFGLYPHMETRICS;

typedef struct _QPFGLYPH
{
    const QPFGLYPHMETRICS* metrics;
    const unsigned char* data;
} QPFGLYPH;

typedef struct _QPFGLYPHTREE
{
    unsigned int min, max;
    struct _QPFGLYPHTREE* less;
    struct _QPFGLYPHTREE* more;
    QPFGLYPH* glyph;
} QPFGLYPHTREE;

typedef struct _QPFMETRICS
{
    Sint8 ascent, descent;
    Sint8 leftbearing, rightbearing;
    Uint8 maxwidth;
    Sint8 leading;
    Uint8 flags;
    Uint8 underlinepos;
    Uint8 underlinewidth;
    Uint8 reserved3;
} QPFMETRICS;

typedef struct 
{
    char font_name [LEN_UNIDEVFONT_NAME + 1];
    unsigned int height;
    unsigned int width;

    unsigned int file_size;
    QPFMETRICS* fm;

    QPFGLYPHTREE* tree;
} QPFINFO;


BOOL LoadQPFont (const char* file, int fd,  QPFINFO* QPFInfo);
void UnloadQPFont (QPFINFO* QPFInfo, int fd);

#ifdef __cplusplus
}
#endif  /* __cplusplus */



