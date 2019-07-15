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

#include "charset.h"
#include "rbf.h"

#define IDC_CREATE          100
#define IDC_OPEN            110
#define IDC_SAVE            120
#define IDC_PREVIOUS        130
#define IDC_NEXT            140
#define IDC_ZOOMOUT         150
#define IDC_ZOOMIN          160
#define IDC_SELECTCHAR      170
#define IDC_COPY            180
#define IDC_PASTE           190

#define BTN_HEIGHT          30
#define BTN_WIDTH           100

#define TXT_X               10
#define TXT_Y               (BTN_HEIGHT + 10)

#define GRID_X              (TXT_X + 2)
#define GRID_Y              (TXT_Y + 32)
#define GRID_W              10
#define GRID_H              10

#define GRID_MARGIN         5

#define FUN_DEBUG
#define VAL_DEBUG
#include <my_debug.h>


static struct button_item
{
    int id;
    char* name;
} button_items [] =
{
    {IDC_CREATE, "create"},
    {IDC_OPEN, "open"},
    {IDC_SAVE, "save"},
    {IDC_COPY, "copy"},
    {IDC_PASTE, "paste"},
    {IDC_SELECTCHAR, "select char"},
    {IDC_PREVIOUS, "previous"},
    {IDC_NEXT, "next"},
    {IDC_ZOOMOUT, "small"},
    {IDC_ZOOMIN, "big"},
};

static void create_buttons(HWND hwnd)
{
    int x = 0;
    int i;
    int button_num = sizeof(button_items)/sizeof(struct button_item);
    for (i=0; i<button_num; i++)
    {
        CreateWindow("button", button_items[i].name, 
                WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
                button_items[i].id, x, 0, BTN_WIDTH, BTN_HEIGHT, hwnd, 0);
        x += BTN_WIDTH + 1;
    }
}

#define MAX_CHAR_LEN        16

static unsigned int cur_ch;
static GLYPHBITMAP glyph;
static RBFINFO rbf;
static unsigned char* copy_bits;

static void inline prepare_copy_bits (void)
{
    assert (rbf.font);
    copy_bits = (unsigned char*)realloc(copy_bits, 
            rbf.height * ((rbf.width+7)>>3));
}

static void inline copy_glyph_bits (void)
{
    memcpy (copy_bits, glyph.bits, 
            rbf.height * ((rbf.width+7)>>3));
}

static void inline paste_glyph_bits (void)
{
    memcpy ((unsigned char*)glyph.bits, copy_bits, 
            rbf.height * ((rbf.width+7)>>3));
}

static BOOL init_glyph (unsigned int offset)
{
    glyph.bbox_x = 0;
    glyph.bbox_y = -rbf.descent;
    glyph.bbox_w = rbf.width;
    glyph.bbox_h = rbf.height;

    glyph.advance_x = glyph.bbox_w;
    glyph.advance_y = 0;

    glyph.bmp_pitch = (rbf.width + 7) >> 3;
    glyph.bmp_size = glyph.bmp_pitch * rbf.height;
    glyph.bits= rbf.font + glyph.bmp_size * offset;

    cur_ch = offset;
    return TRUE;
}


static RECT grid_frame;
static void init_grid_frame (void)
{
    grid_frame.left    = GRID_X;
    grid_frame.top     = GRID_Y;
    grid_frame.right   = grid_frame.left + rbf.width*GRID_W;
    grid_frame.bottom  = grid_frame.top + rbf.height*GRID_H;
}

static void draw_glyph (HDC hdc)
{
    int i, j, k;
    int x, y, x1, x2, y1, y2;
    int baseline_y, top_y;
    const unsigned char* glyph_bits;

    if (rbf.font == NULL)
        return;

    SetBkMode (hdc, BM_TRANSPARENT);

    if (rbf.font) {
        char text [256];
        sprintf (text, 
                "charset:%s, width:%d, height:%d, glyph value: 0x%04x", 
                rbf.charset_ops->name,
                rbf.width, rbf.height, cur_ch);
        TextOut (hdc, TXT_X, TXT_Y, text);
    }
    else {
        TextOut (hdc, TXT_X, TXT_Y, "Please load or create a RBF font");
        return;
    }

    SetPenColor (hdc, PIXEL_lightwhite);
    Rectangle(hdc, grid_frame.left - GRID_MARGIN,
                    grid_frame.top - GRID_MARGIN, 
                    grid_frame.right + GRID_MARGIN,
                    grid_frame.bottom + GRID_MARGIN);

    /* draw grid */
    SetPenColor (hdc, PIXEL_lightgray);

    x1 = grid_frame.left;
    x2 = grid_frame.left + rbf.width*GRID_W;
    y = grid_frame.top;
    for (i = 0; i <= rbf.height; i ++) {
        MoveTo (hdc, x1, y);
        LineTo (hdc, x2, y);
        y += GRID_H;
    }

    y1 = grid_frame.top;
    y2 = grid_frame.top + rbf.height*GRID_H;
    x = grid_frame.left;
    for (i = 0; i <= rbf.width; i ++) {
        MoveTo (hdc, x, y1);
        LineTo (hdc, x, y2);
        x += GRID_W;
    }

    /* draw base line */
    SetPenColor (hdc, PIXEL_red);

    baseline_y = (rbf.height - rbf.descent);
    y = grid_frame.top + GRID_H * baseline_y;
    MoveTo (hdc, x1, y);
    LineTo (hdc, x2, y);

    /* draw bounding box */
    x1 = grid_frame.left;
    y1 = grid_frame.top;
    x2 = grid_frame.left + rbf.width * GRID_W;
    y2 = grid_frame.top + rbf.height * GRID_H;

    MoveTo (hdc, x1, y1);
    LineTo (hdc, x2, y1);
    LineTo (hdc, x2, y2);
    LineTo (hdc, x1, y2);

    /* draw glyph */
    top_y = baseline_y - glyph.bbox_h - glyph.bbox_y;

    SetBrushColor (hdc, PIXEL_darkgray);

    glyph_bits = glyph.bits;
    for (i = 0; i < glyph.bbox_h; i++) {
        int width = 0;

        for (j = 0; j < glyph.bmp_pitch; j++) {
            unsigned char c = glyph_bits [j];

            if (c) {
                for (k = 0; k < 8 && (width + k) < glyph.bbox_w; k++) {
                    if (c  & (0x80>>k)) {
                        x = grid_frame.left + GRID_W *(j*8 + k);
                        y = grid_frame.top + GRID_H * (top_y + i);
                        FillBox (hdc, x, y, GRID_W, GRID_H);

                        x = grid_frame.right + GRID_MARGIN * 2 + j*8 + k;
                        y = grid_frame.top + top_y + i;
                        SetPixel (hdc, x, y, PIXEL_black);
                    }
                }
            }

            width += 8;
        }

        glyph_bits += glyph.bmp_pitch;
    }
}

static void set_glyph_bit (HWND hWnd, int x, int y)
{
    int i, j, k;
    unsigned char* glyph_bits = (unsigned char*) glyph.bits;

    if (x > grid_frame.left + GRID_W * rbf.width) {
        Ping ();
        return;
    }

    i = (x - grid_frame.left) / GRID_W;
    j = (y - grid_frame.top) / GRID_H;
    k = i % 8;

    glyph_bits += j * glyph.bmp_pitch;
    glyph_bits += (i / 8);

    if (*glyph_bits & (0x80 >> k)) {
        *glyph_bits &= ~(0x80 >> k);
    }
    else {
        *glyph_bits |= 0x80 >> k;
    }

    InvalidateRect (hWnd, NULL, TRUE);
}

BOOL glyph_get_bit(int row, int col)
{
    unsigned char* bits = (unsigned char*)glyph.bits;
    int k = col % 8;

    bits += row * glyph.bmp_pitch;
    bits += (col / 8);

    return (*bits & (0x80 >> k));
}

void glyph_set_bit(int row, int col, BOOL value)
{
    unsigned char* bits = (unsigned char*)glyph.bits;
    int k = col % 8;

    bits += row * glyph.bmp_pitch;
    bits += (col / 8);
    if (value)
        *bits |= (0x80 >> k);
    else
        *bits &= ~(0x80 >> k);
}

static void move_glyph_left (HWND hWnd)
{
    int i, j;
    BOOL value;
    BOOL* first_col;
    int width  = glyph.bbox_w;
    int height = glyph.bbox_h;

    first_col = malloc(sizeof(BOOL) * height);

    for (i = 0; i<height; i++)
        first_col[i] = glyph_get_bit(i, 0);

    for (i = 0; i<height; i++) {
        for (j = 0; j<width - 1; j++) {
            value = glyph_get_bit(i, j+1);
            glyph_set_bit(i, j, value);
        }
    }

    for (i = 0; i<height; i++)
        glyph_set_bit(i, width-1, first_col[i]);

    free(first_col);
    InvalidateRect (hWnd, NULL, TRUE);
}

static void move_glyph_right (HWND hWnd)
{
    int i, j;
    BOOL value;
    BOOL* last_col;
    int width = glyph.bbox_w;
    int height = glyph.bbox_h;
    last_col = malloc(sizeof(BOOL)*height);
    for (i = 0; i<height; i++)
        last_col[i] = glyph_get_bit(i, width-1);
    for (i = 0; i<height; i++)
    {
        for (j=width-1; j>0; j--)
        {
            value = glyph_get_bit (i, j-1);
            glyph_set_bit (i, j, value);
        }
    }
    for (i = 0; i<height; i++)
        glyph_set_bit (i, 0, last_col[i]);
    free(last_col);
    InvalidateRect (hWnd, NULL, TRUE);
}

static void move_glyph_up (HWND hWnd)
{
    int i;
    unsigned char* glyph_bits = (unsigned char*) glyph.bits;

    for (i = 0; i < rbf.height - 1; i ++) {
        memcpy (glyph_bits, glyph_bits + glyph.bmp_pitch, glyph.bmp_pitch);
        glyph_bits += glyph.bmp_pitch;
    }

    memset (glyph_bits, 0, glyph.bmp_pitch);

    InvalidateRect (hWnd, NULL, TRUE);
}

static void move_glyph_down (HWND hWnd)
{
    int i;
    unsigned char* glyph_bits 
            = (unsigned char*) glyph.bits +  glyph.bmp_pitch * (rbf.height - 1);

    for (i = 0; i < rbf.height - 1; i ++) {
        memcpy (glyph_bits, glyph_bits - glyph.bmp_pitch, glyph.bmp_pitch);
        glyph_bits -= glyph.bmp_pitch;
    }

    memset (glyph_bits, 0, glyph.bmp_pitch);

    InvalidateRect (hWnd, NULL, TRUE);
}

static int view_win_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_CREATE:
            create_buttons (hWnd);
            rbf.is_saved = TRUE;
            break;

        case MSG_COMMAND:
        {
            if (wParam != IDC_OPEN && wParam != IDC_CREATE 
                    && !rbf.font)
                return 0;

            switch (wParam) {
                case IDC_CREATE:
                    if (rbf.font && rbf.is_saved == FALSE) {
                        MessageBox (hWnd , "Please save current font!", "error", 
                                MB_OK|MB_ICONSTOP|MB_ALIGNCENTER);
                    }
                    else {
                        if (create_font (hWnd, &rbf))
                        {
                            prepare_copy_bits();
                            init_grid_frame ();
                            cur_ch = 0;
                            init_glyph (cur_ch);
                            InvalidateRect (hWnd, NULL, TRUE);
                        }
                    }
                    break;

                case IDC_OPEN:
                    if (rbf.font && rbf.is_saved == FALSE) {
                        MessageBox (hWnd , "Please save current font!", "error", 
                                MB_OK|MB_ICONSTOP|MB_ALIGNCENTER);
                            break;
                    }

                    free_rbf (&rbf);
                    if (open_rbf(hWnd, &rbf))
                    {
                        prepare_copy_bits();

                        init_grid_frame ();
                        cur_ch = 0;
                        init_glyph (cur_ch);
                    }
                    else
                    {
                        /*TODO*/
                    }
                    InvalidateRect (hWnd, NULL, TRUE);
                    break;

                case IDC_SAVE:
                    save_rbf (hWnd, &rbf);
                    break;

                case IDC_COPY:    
                    copy_glyph_bits ();
                    break;

                case IDC_PASTE:

                    paste_glyph_bits ();
                    InvalidateRect (hWnd, NULL, TRUE);
                    break;

                case IDC_SELECTCHAR:
                    if (rbf.font && selectChar (hWnd, &rbf, &cur_ch)) {
                        TEST_VAL (cur_ch, %d);
                        init_glyph (cur_ch);
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    break;

                case IDC_PREVIOUS:
                    if (rbf.font && cur_ch > 0) {
                        cur_ch --;
                        init_glyph (cur_ch);
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    break;

                case IDC_NEXT:
                    if (rbf.font && cur_ch < rbf.charset_ops->nr_chars) {
                        cur_ch ++;
                        init_glyph (cur_ch);
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    break;
            }
        }
        break;

        case MSG_KEYDOWN:
            switch (LOWORD(wParam)) {
                case SCANCODE_F1:
                return 0;

                case SCANCODE_CURSORBLOCKLEFT:
                    if (lParam & KS_SHIFT) {
                        move_glyph_left (hWnd);
                        rbf.is_saved = FALSE;
                        break;
                    }

                case SCANCODE_CURSORBLOCKRIGHT:
                    if (lParam & KS_SHIFT) {
                        move_glyph_right (hWnd);
                        rbf.is_saved = FALSE;
                        break;
                    }

                case SCANCODE_CURSORBLOCKUP:
                    if (lParam & KS_SHIFT) {
                        move_glyph_up (hWnd);
                        rbf.is_saved = FALSE;
                        break;
                    }

                    if (rbf.font && cur_ch > 0) {
                        cur_ch --;
                        init_glyph (cur_ch);
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    break;

                case SCANCODE_CURSORBLOCKDOWN:
                    if (lParam & KS_SHIFT) {
                        move_glyph_down (hWnd);
                        rbf.is_saved = FALSE;
                        break;
                    }

                    if (rbf.font && cur_ch < rbf.charset_ops->nr_chars) {
                        cur_ch ++;
                        init_glyph (cur_ch);
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    break;

                case SCANCODE_PAGEUP:
                    if (rbf.font && cur_ch > 10 && (cur_ch - 10) >= 0) {
                        cur_ch -= 10;
                        init_glyph (cur_ch);
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    break;

                case SCANCODE_PAGEDOWN:
                    if (rbf.font && cur_ch < 245 
                            && (cur_ch + 10) <= rbf.charset_ops->nr_chars) {
                        cur_ch += 10;
                        init_glyph (cur_ch);
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    break;

                case SCANCODE_EQUAL:
                    break;

                case SCANCODE_MINUS:
                    break;
            }
            break;

        case MSG_LBUTTONDOWN:
        {
            int x = LOWORD (lParam);
            int y = HIWORD (lParam);

            if (PtInRect (&grid_frame, x, y)) {
                set_glyph_bit (hWnd, x, y);
                rbf.is_saved = FALSE;
            }
            break;
        }

        case MSG_PAINT:
        {
            HDC hdc = BeginPaint (hWnd);
            draw_glyph (hdc);
            EndPaint (hWnd, hdc);
            return 0;
        }

        case MSG_CLOSE:
            if (rbf.is_saved == FALSE) {
                MessageBox (hWnd , "Please save current font!", "error", 
                        MB_OK|MB_ICONSTOP|MB_ALIGNCENTER);
                    return 0;
            }

            DestroyAllControls (hWnd);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int args, const char* arg[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    LOGFONT* ctrl_font;
    LOGFONT* utf8_font;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , arg[0], 0 , 0);
#endif

    //utf8_font = CreateLogFontByName("*-unifont-rrncnn-*-12-UTF-8");
    utf8_font = CreateLogFont ("*", "unifont", "UTF-8", FONT_WEIGHT_REGULAR,
            FONT_SLANT_ROMAN, FONT_FLIP_NIL, FONT_OTHER_NIL,
            FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
            12, 0);
    assert (utf8_font);
    ctrl_font = g_SysLogFont[SYSLOGFONT_CONTROL];
    //g_SysLogFont[SYSLOGFONT_CONTROL] = utf8_font;

        
    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "MiniGUI RBF Editor";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = view_win_proc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 800;
    CreateInfo.by = 480;
    CreateInfo.iBkColor = PIXEL_green;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return 3;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

