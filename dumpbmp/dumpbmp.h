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

#ifndef _DUMPBMP_H

#define _DUMPBMP_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define LEN_PREFIX      29

struct mem_dc_info {
    int bpp;
    Uint32 rmask;
    Uint32 gmask;
    Uint32 bmask;
    Uint32 amask;
    GAL_Color pal [256];

    char prefix [LEN_PREFIX + 1];
};

extern int dump_bitmap (BITMAP* bmp, const char* prefix, FILE *fp);
extern int get_dump_info (HWND hwnd, struct mem_dc_info* info);
extern int add_hash_entry (const char* path, const char* prefix);
extern int dump_hash_table (const char* prefix);
extern int add_bmp_header_entry (FILE *fp, const char* prefix);
extern int add_bmp_loader_entry (FILE *fp, const char* file, const char* prefix);
extern int add_bmp_unloader_entry (FILE *fp, const char* prefix);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _DUMPBMP_H */

