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

#ifndef _RBF_H

#define _RBF_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct
{
    CHARSETOPS* charset_ops;

    int width;
    int height;
    int descent;
    unsigned char* font;
    long font_size;

    /* internal used */
    unsigned int char_off;       

    BOOL is_saved;
    /** The full path name of the file returned. */
    char filefullname[MY_NAMEMAX + MY_PATHMAX + 1];
    /** The name of the file to be opened. */
    char filename[MY_NAMEMAX + 1];
} RBFINFO;

BOOL load_rbf (RBFINFO* rbf_info, const char* filename, 
        const char* filefullname);

void free_rbf (RBFINFO* vbf);
//BOOL dumpRBF (const RBFINFO* vbf, char* file);
BOOL selectChar (HWND hWnd, const RBFINFO* vbf, unsigned int* cur_ch);

BOOL create_font (HWND hWnd, RBFINFO* rbf);

BOOL save_rbf (HWND hWnd, RBFINFO* rbf_info);
BOOL open_rbf (HWND hWnd, RBFINFO* rbf_info);
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _RBF_H */

