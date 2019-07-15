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

