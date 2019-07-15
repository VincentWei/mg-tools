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

#include "qpf.h"

void glyph_free (GLYPHBITMAP* glyph)
{
    if(glyph == NULL)
        return;
    if(glyph->bits)
        free((void*)glyph->bits);
    glyph->bits = NULL;
    return;
}

BOOL glyph_get_bit(GLYPHBITMAP* glyph_bitmap, int row, int col)
{
    unsigned char* bits = (unsigned char*)glyph_bitmap->bits;
    int k = col % 8;

    bits += row * glyph_bitmap->bmp_pitch;
    bits += (col / 8);

    return (*bits & (0x80 >> k));
}

void glyph_set_bit(GLYPHBITMAP* glyph_bitmap, int row, int col, BOOL value)
{
    unsigned char* bits = (unsigned char*)glyph_bitmap->bits;
    int k = col % 8;

    bits += row * glyph_bitmap->bmp_pitch;
    bits += (col / 8);
    if (value)
        *bits |= (0x80 >> k);
    else
        *bits &= ~(0x80 >> k);
}
#if 0
void glyph_move_left(GLYPHBITMAP* glyph)
{
    int i, j;
    BOOL value;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    for (i = 0; i<height; i++)
    {
        for (j = 0; j<width - 1; j++)
        {
            value = glyph_get_bit(glyph, i, j+1);
            glyph_set_bit(glyph, i, j, value);
        }
    }
}
#else
void glyph_move_left(GLYPHBITMAP* glyph_bitmap)
{
    int i, j;
    BOOL value;
    BOOL* first_col;
    int width  = glyph_bitmap->bbox_w;
    int height = glyph_bitmap->bbox_h;

    first_col = malloc(sizeof(BOOL) * height);

    for (i = 0; i<height; i++)
        first_col[i] = glyph_get_bit(glyph_bitmap, i, 0);

    for (i = 0; i<height; i++) {
        for (j = 0; j<width - 1; j++) {
            value = glyph_get_bit(glyph_bitmap, i, j+1);
            glyph_set_bit(glyph_bitmap, i, j, value);
        }
    }

    for (i = 0; i<height; i++)
        glyph_set_bit(glyph_bitmap, i, width-1, first_col[i]);

    free(first_col);
}
#endif
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

void glyph_move_up(GLYPHBITMAP* glyph_bitmap)
{
    int i, j;
    BOOL value;
    BOOL* first_row;
    int width  = glyph_bitmap->bbox_w;
    int height = glyph_bitmap->bbox_h;

    first_row = malloc(sizeof(BOOL)*width);

    for (j = 0; j<width; j++)
        first_row[j] = glyph_get_bit(glyph_bitmap, 0, j);

    for (i = 0; i<height - 1; i++) {
        for (j = 0; j<width; j++) {
            value = glyph_get_bit(glyph_bitmap, i+1, j);
            glyph_set_bit(glyph_bitmap, i, j, value);
        }
    }

    for (j = 0; j<width; j++)
        glyph_set_bit(glyph_bitmap, height-1, j, first_row[j]);

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

void glyph_bitmap_copy(GLYPHBITMAP* to, GLYPHBITMAP* from)
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

void glyph_change_width_right(GLYPHBITMAP* glyph_bitmap, int width)
{
    GLYPHBITMAP old_glyph_bitmap;
    int height;

    if(width == 0 || glyph_bitmap == NULL) return;

    height = glyph_bitmap->bbox_h;
    memcpy(&old_glyph_bitmap, glyph_bitmap, sizeof(GLYPHBITMAP));
    glyph_bitmap->advance_x = glyph_bitmap->advance_x + width;
	
    glyph_bitmap->bmp_pitch
	   	= (glyph_bitmap->bmp_pitch / glyph_bitmap->bbox_w) * (glyph_bitmap->bbox_w + width);

    glyph_bitmap->bbox_w = glyph_bitmap->bbox_w + width;
    glyph_bitmap->bmp_pitch = (glyph_bitmap->bbox_w + 7) >> 3;
    glyph_bitmap->bmp_size = glyph_bitmap->bmp_pitch*height;


	//FIXME
    glyph_bitmap->bits = calloc(1, glyph_bitmap->bmp_size);
    glyph_bitmap_copy(glyph_bitmap, &old_glyph_bitmap);
    //glyph_free(&old_glyph_bitmap);
}

void glyph_change_width_left(GLYPHBITMAP* glyph_bitmap, int width)
{
    GLYPHBITMAP old_glyph_bitmap;
    int height;
	
    if(width == 0) return;

    height = glyph_bitmap->bbox_h;
    memcpy(&old_glyph_bitmap, glyph_bitmap, sizeof(GLYPHBITMAP));

    glyph_move_left(&old_glyph_bitmap);

    glyph_bitmap->bbox_w    = glyph_bitmap->bbox_w + width;
    glyph_bitmap->bbox_x++;
    glyph_bitmap->advance_x = glyph_bitmap->advance_x + width;
    glyph_bitmap->bmp_pitch = (glyph_bitmap->bbox_w + 7) >> 3;
    glyph_bitmap->bmp_size  = glyph_bitmap->bmp_pitch * height;
    glyph_bitmap->bits      = calloc(1, glyph_bitmap->bmp_size);

    glyph_bitmap_copy(glyph_bitmap, &old_glyph_bitmap);
    //glyph_free(&old_glyph_bitmap);
}

void glyph_change_height_bottom(GLYPHBITMAP* glyph_bitmap, int height)
{
    GLYPHBITMAP old_glyph_bitmap;

    if(height == 0) return;

    memcpy(&old_glyph_bitmap, glyph_bitmap, sizeof(GLYPHBITMAP));
    glyph_bitmap->bbox_h = glyph_bitmap->bbox_h + height;
    glyph_bitmap->bmp_size = glyph_bitmap->bmp_pitch * glyph_bitmap->bbox_h;
	//FIXME
    glyph_bitmap->bits = calloc(1, glyph_bitmap->bmp_size);
    glyph_bitmap_copy(glyph_bitmap, &old_glyph_bitmap);
    //glyph_free(&old_glyph_bitmap);
}

void glyph_change_height_top(GLYPHBITMAP* glyph_bitmap, int height)
{
    GLYPHBITMAP old_glyph;

    if(height == 0) return;

    memcpy(&old_glyph, glyph_bitmap, sizeof(GLYPHBITMAP));
    glyph_move_up(&old_glyph);

    glyph_bitmap->bbox_h   = glyph_bitmap->bbox_h + height;
    glyph_bitmap->bbox_y--;
    glyph_bitmap->bmp_size = glyph_bitmap->bmp_pitch * glyph_bitmap->bbox_h;
    glyph_bitmap->bits     = calloc(1, glyph_bitmap->bmp_size);
    glyph_bitmap_copy(glyph_bitmap, &old_glyph);

    //glyph_free(&old_glyph);
}
