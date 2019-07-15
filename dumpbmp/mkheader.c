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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "dumpbmp.h"


int add_bmp_header_entry (FILE *fp, const char* prefix)
{
    char buffer[128], buffer_org[128], *tmp;
    static int index = 0;

    sprintf (buffer, "%s", prefix);
    sprintf (buffer_org, "%s", prefix);

    tmp = buffer;

    while (*tmp != '\0') {
	    if (*tmp == '/')
            *tmp = '_';
	    else
            *tmp = toupper (*tmp);
	    tmp ++;
    }

    fprintf (fp, "#define  %-24s        %d\n", buffer, index);
    fprintf (fp, "#define    bmp_%-24s  (fhas_bitmaps+%s)\n", buffer_org, buffer);
    index ++;
    return 0;
}

int add_bmp_loader_entry (FILE *fp, const char* file, const char* prefix)
{
    char buffer_org[128];

    sprintf (buffer_org, "%s", prefix);

    fprintf (fp, "    strcpy(tmp, \"%s\");\n", file);
    fprintf (fp, "    LoadBitmapFromFile(HDC_SCREEN, bmp_%s, buffer);\n", buffer_org);

    return 0;
}

int add_bmp_unloader_entry (FILE *fp, const char* prefix)
{
    char buffer_org[128];

    sprintf (buffer_org, "%s", prefix);

    fprintf (fp, "    UnloadBitmap (bmp_%s);\n", buffer_org);

    return 0;
}

