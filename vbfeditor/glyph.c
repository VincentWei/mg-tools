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

/*
**  This source is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public
**  License as published by the Free Software Foundation; either
**  version 2 of the License, or (at your option) any later version.
**
**  This software is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public
**  License along with this library; if not, write to the Free
**  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
**  MA 02111-1307, USA
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mgutils/mgutils.h>

#include "vbf.h"

void glyph_free (GLYPHBITMAP* glyph)
{
    if(glyph == NULL)
        return;
    if(glyph->bits)
        free((void*)glyph->bits);
    glyph->bits = NULL;
    return;
}

BOOL glyph_get_bit(GLYPHBITMAP* glyph, int row, int col)
{
    unsigned char* bits = (unsigned char*)glyph->bits;
    int k = col % 8;
    bits += row * glyph->bmp_pitch;
    bits += (col / 8);
    return (*bits & (0x80 >> k));
}

void glyph_set_bit(GLYPHBITMAP* glyph, int row, int col, BOOL value)
{
    unsigned char* bits = (unsigned char*)glyph->bits;
    int k = col % 8;
    bits += row * glyph->bmp_pitch;
    bits += (col / 8);
    if (value)
        *bits |= (0x80 >> k);
    else
        *bits &= ~(0x80 >> k);
}
void glyph_move_left(GLYPHBITMAP* glyph)
{
    int i, j;
    BOOL value;
    BOOL* first_col;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    first_col = malloc(sizeof(BOOL)*height);
    for (i = 0; i<height; i++)
        first_col[i] = glyph_get_bit(glyph, i, 0);
    for (i = 0; i<height; i++)
    {
        for (j = 0; j<width - 1; j++)
        {
            value = glyph_get_bit(glyph, i, j+1);
            glyph_set_bit(glyph, i, j, value);
        }
    }
    for (i = 0; i<height; i++)
        glyph_set_bit(glyph, i, width-1, first_col[i]);
    free(first_col);
}
void glyph_move_right(GLYPHBITMAP* glyph)
{
    int i, j;
    BOOL value;
    BOOL* last_col;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    last_col = malloc(sizeof(BOOL)*height);
    for (i = 0; i<height; i++)
        last_col[i] = glyph_get_bit(glyph, i, width-1);
    for (i = 0; i<height; i++)
    {
        for (j=width-1; j>0; j--)
        {
            value = glyph_get_bit(glyph, i, j-1);
            glyph_set_bit(glyph, i, j, value);
        }
    }
    for (i = 0; i<height; i++)
        glyph_set_bit(glyph, i, 0, last_col[i]);
    free(last_col);
}

void glyph_move_up(GLYPHBITMAP* glyph)
{
    int i, j;
    BOOL value;
    BOOL* first_row;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    first_row = malloc(sizeof(BOOL)*width);
    for (j = 0; j<width; j++)
        first_row[j] = glyph_get_bit(glyph, 0, j);
    for (i = 0; i<height - 1; i++)
    {
        for (j = 0; j<width; j++)
        {
            value = glyph_get_bit(glyph, i+1, j);
            glyph_set_bit(glyph, i, j, value);
        }
    }
    for (j = 0; j<width; j++)
        glyph_set_bit(glyph, height-1, j, first_row[j]);
    free(first_row);
}

void glyph_move_down(GLYPHBITMAP* glyph)
{
    int i, j;
    BOOL value;
    BOOL* last_row;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    last_row = malloc(sizeof(BOOL)*width);
    for (j = 0; j<width; j++)
        last_row[j] = glyph_get_bit(glyph, height-1, j);

    for (i = height-1; i > 0; i--)
    {
        for (j = 0; j<width; j++)
        {
            value = glyph_get_bit(glyph, i-1, j);
            glyph_set_bit(glyph, i, j, value);
        }
    }
    for (j = 0; j<width; j++)
        glyph_set_bit(glyph, 0, j, last_row[j]);
    free(last_row);
}

void glyph_draw(GLYPHBITMAP* glyph, HDC hdc, int x, int y, int pixel_size, gal_pixel color)
{
    int i, j;
    int pixel_x, pixel_y;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            pixel_x = x + pixel_size * (glyph->bbox_x + j);
            pixel_y = y + pixel_size * (-glyph->bbox_y + i);
            if (pixel_size == 1)
            {
                if(glyph_get_bit(glyph, i, j))
                    SetPixel(hdc, pixel_x, pixel_y, color);
                else
                    SetPixel(hdc, pixel_x, pixel_y, PIXEL_lightwhite);
            }
            else
            {
                if(glyph_get_bit(glyph, i, j))
                    SetBrushColor(hdc, color);
                else
                    SetBrushColor(hdc, PIXEL_lightwhite);
                FillBox (hdc, pixel_x + 1, pixel_y + 1, pixel_size - 1, pixel_size - 1);
            }
        }
    }
}

void glyph_copy(GLYPHBITMAP* to, GLYPHBITMAP* from)
{
    int min_width = MIN(to->bbox_w, from->bbox_w);
    int min_height = MIN(to->bbox_h, from->bbox_h);
    int i,j;
    memset((unsigned char*)to->bits, 0, to->bmp_size);
    for (i = 0; i<min_height; i++)
    {
        for (j = 0; j<min_width;j++)
            glyph_set_bit(to, i, j, glyph_get_bit(from, i, j));
    }
}

void glyph_change_width_right(GLYPHBITMAP* glyph, int width)
{
    GLYPHBITMAP old_glyph;
    int height;
    height = glyph->bbox_h;
    if(width == glyph->bbox_w)
        return;
    memcpy(&old_glyph, glyph, sizeof(GLYPHBITMAP));
    glyph->bbox_w = width;
    glyph->advance_x = glyph->bbox_w;
    glyph->bmp_pitch = (width + 7)>>3;
    glyph->bmp_size = glyph->bmp_pitch*height;
    glyph->bits = calloc(1, glyph->bmp_size);
    glyph_copy(glyph, &old_glyph);
    glyph_free(&old_glyph);
}

void glyph_change_width_left(GLYPHBITMAP* glyph, int width)
{
    GLYPHBITMAP old_glyph;
    int height;
    height = glyph->bbox_h;
    if(width == glyph->bbox_w)
        return;
    memcpy(&old_glyph, glyph, sizeof(GLYPHBITMAP));
    glyph_move_left(&old_glyph);
    glyph->bbox_w = width;
    glyph->bbox_x++;
    glyph->advance_x = glyph->bbox_w;
    glyph->bmp_pitch = (width + 7)>>3;
    glyph->bmp_size = glyph->bmp_pitch*height;
    glyph->bits = calloc(1, glyph->bmp_size);
    glyph_copy(glyph, &old_glyph);
    glyph_free(&old_glyph);
}

void glyph_change_height_bottom(GLYPHBITMAP* glyph, int height)
{
    GLYPHBITMAP old_glyph;
    if(height == glyph->bbox_h)
        return;
    memcpy(&old_glyph, glyph, sizeof(GLYPHBITMAP));
    glyph->bbox_h = height;
    glyph->bmp_size = glyph->bmp_pitch * height;
    glyph->bits = malloc(glyph->bmp_size);
    glyph_copy(glyph, &old_glyph);
    glyph_free(&old_glyph);
}

void glyph_change_height_top(GLYPHBITMAP* glyph, int height)
{
    GLYPHBITMAP old_glyph;
    if(height == glyph->bbox_h)
        return;
    memcpy(&old_glyph, glyph, sizeof(GLYPHBITMAP));
    glyph_move_up(&old_glyph);
    glyph->bbox_h = height;
    glyph->bbox_y--;
    glyph->bmp_size = glyph->bmp_pitch * height;
    glyph->bits = malloc(glyph->bmp_size);
    glyph_copy(glyph, &old_glyph);
    glyph_free(&old_glyph);
}


