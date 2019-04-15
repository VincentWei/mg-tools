/*
** Copyright (C) 2009 FMSoft
** Author: WEI Yongming
** Create date: 2009/12/19
*/

#ifndef TYPES_H
#define TYPES_H

#define TRUE   1
#define FALSE  0

typedef  int            BOOL;
typedef  unsigned char  BYTE;
typedef  unsigned char  Uint8;
typedef  signed   char  Sint8;
typedef  unsigned short Uint16;
typedef  unsigned int   Uint32;

#define LEN_FONT_NAME           31
#define LEN_DEVFONT_NAME        127
#define LEN_UNIDEVFONT_NAME     127

#define LEN_VERSION_INFO        10
#define LEN_VENDER_NAME         12

static inline Uint16 ArchSwap16(Uint16 D) {
        return((D<<8)|(D>>8));
}

static inline Uint32 ArchSwap32(Uint32 D) {
        return((D<<24)|((D<<8)&0x00FF0000)|((D>>8)&0x0000FF00)|(D>>24));
}

#endif  // TYPES_H

