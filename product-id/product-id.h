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
#ifndef _PRODUCT_ID_H
#define _PRODUCT_ID_H

typedef struct _product_id {
    /* Fixed 8-byte code, help us to locate the struct in .so file */
    unsigned char prefix[8];
    /* the ID of the customer */
    int customer_id;
    /* svn version */
    int version;
    /* When does we compile the .so, in seconds */
    int compile_date;
    /* The size of the .so file */
    int file_size;
    /* The check sum of the .so file */
    unsigned char checksum[16];
} product_id_t;

#define PRODUCT_ID_PREFIX 0xca, 0x3f, 0x2b, 0x43, 0x00, 0x33, 0xb0, 0xc3

#endif
