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

#define IDC_FILE_OPEN           110
#define IDC_FILE_SAVE           120
#define IDC_FILE_RENAME         130
#define IDC_CHAR_PREV           140
#define IDC_CHAR_NEXT           150
#define IDC_ZOOM_OUT            160
#define IDC_ZOOM_IN             170

#define IDC_CHARWIDTHADD        180
#define IDC_CHARWIDTHDEC        190
#define IDC_CHARHEIGHTADD       200
#define IDC_CHARHEIGHTDEC       210
#define IDC_STATICWIDTH         220
#define IDC_STATICHEIGHT        230
#define IDC_UPDATE              240

#define IDC_STATIC_NAME         260
#define IDC_EDIT_NAME           270
#define IDOK_NAME               280

#define IDC_CHAR_INC_BB_X       290
#define IDC_CHAR_DEC_BB_X       300
#define IDC_CHAR_INC_BB_Y       310
#define IDC_CHAR_DEC_BB_Y       320
#define IDC_CHAR_INC_BB_W       330
#define IDC_CHAR_DEC_BB_W       340
#define IDC_CHAR_INC_BB_H       350
#define IDC_CHAR_DEC_BB_H       360

#define IDC_CHAR_DEC_BB_H_FROM_TOP       370
#define IDC_CHAR_DEC_BB_W_FROM_LEFT      380


#define TXT_X                   5
#define TXT_Y                   10

#define CHARWIDTH_X             (TXT_X + 2) 
#define CHARWIDTH_Y             (TXT_Y + 70)
#define CHARHEIGHT_Y            (CHARWIDTH_Y + 50)
#define CHARWIDTH_W             60
#define CHARWIDTH_H             20

#define GRID_X                  CHARWIDTH_X
#define GRID_Y                  CHARWIDTH_Y
#define GRID_SIZE               12 

#define TABLE_X                 400
#define TABLE_Y                 CHARWIDTH_Y
#define TABLE_WIDTH             16
#define TABLE_HEIGHT            (256/TABLE_WIDTH)

#define GRID_MARGIN             5

#define FRAMEOFFSET              (rect_gridframe.left + (rect_gridframe.right - rect_gridframe.left)/3)

static int file_menu_ids[] =
{
    IDC_FILE_OPEN,
    IDC_FILE_SAVE,
    IDC_FILE_RENAME,
    0
};
static char* file_menu_texts[] =
{
    "Open",
    "Save As",
    "Rename",
};
static int char_menu_ids[] =
{
    IDC_CHAR_PREV,
    IDC_CHAR_NEXT,
    IDC_CHAR_INC_BB_X,
    IDC_CHAR_DEC_BB_X,
    IDC_CHAR_INC_BB_Y,
    IDC_CHAR_DEC_BB_Y,
    IDC_CHAR_INC_BB_W,
    IDC_CHAR_DEC_BB_W,
    IDC_CHAR_INC_BB_H,
    IDC_CHAR_DEC_BB_H,
    IDC_CHAR_DEC_BB_W_FROM_LEFT,
    IDC_CHAR_DEC_BB_H_FROM_TOP,
    IDC_CHARWIDTHADD,
    IDC_CHARWIDTHDEC,
    IDC_CHARHEIGHTADD,
    IDC_CHARHEIGHTDEC,
    0
};
static char* char_menu_texts[] =
{
    "Previous",
    "Next",
    "Bbox_x ++",
    "Bbox_x --",
    "Bbox_y ++",
    "Bbox_y --",
    "Bbox_w ++",
    "Bbox_w --",
    "Bbox_h ++",
    "Bbox_h --",
    "Bbox_w -- (from left)",
    "Bbox_h -- (from top)",
    "CHAR WIDTHADD",
    "CHAR WIDTHDEC",
    "CHAR HEIGHTADD",
    "CHAR HEIGHTDEC",
};

static int zoom_menu_ids[] =
{
    IDC_ZOOM_OUT,
    IDC_ZOOM_IN,
    0
};
static char* zoom_menu_texts[] =
{
    "Zoom Out",
    "Zoom In"
};

static int update_menu_ids[] =
{
    IDC_UPDATE,
    0
};
static char* update_menu_texts[] =
{
    "Update to version 2.0",
};

static DLGTEMPLATE RenameDlg=
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    200, 200, 300, 140,
    "输入新字体名:",
    0, 0,
    3, NULL,
    0 
};

static CTRLDATA CtrlRenameDlg [] =
{
    {
        "static",
        WS_VISIBLE,
        20, 20, 270, 25,
        IDC_STATIC_NAME,
        "新字体名:",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        30, 45, 240, 25,
        IDC_EDIT_NAME,
        "",
        0
    },
    {
        "button",
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        110, 80, 80, 25,
        IDOK_NAME,
        "确定",
        0
    },
};

static HMENU create_popup_menu(char* name, int* ids, char** texts)
{
    int i = 0;
    HMENU hMenu;
    MENUITEMINFO mi;
    memset(&mi, 0, sizeof(mi));
    mi.type = MFT_STRING;
    mi.id = 0;
    mi.typedata = (DWORD)name;
    hMenu = CreatePopupMenu(&mi);
    while(ids[i] != 0)
    {
        memset(&mi, 0, sizeof(mi));
        mi.type = MFT_STRING;
        mi.id = ids[i];
        mi.typedata = (DWORD)texts[i];
        InsertMenuItem(hMenu, i, TRUE, &mi);
        i++;
    }
    return hMenu;
}

static HMENU create_main_menu(void)
{
    HMENU hMenu;
    MENUITEMINFO mi;
    hMenu = CreateMenu();
    
    memset(&mi, 0, sizeof(mi));
    mi.type = MFT_STRING;
    mi.id = 100;
    mi.typedata = (DWORD)"Update";
    mi.hsubmenu = create_popup_menu("Update", update_menu_ids, update_menu_texts);
    InsertMenuItem(hMenu, 0, TRUE, &mi);

    memset(&mi, 0, sizeof(mi));
    mi.type = MFT_STRING;
    mi.id = 100;
    mi.typedata = (DWORD)"Zoom";
    mi.hsubmenu = create_popup_menu("Zoom", zoom_menu_ids, zoom_menu_texts);
    InsertMenuItem(hMenu, 0, TRUE, &mi);

    memset(&mi, 0, sizeof(mi));
    mi.type = MFT_STRING;
    mi.id = 100;
    mi.typedata = (DWORD)"Char";
    mi.hsubmenu = create_popup_menu("Char", char_menu_ids, char_menu_texts);
    InsertMenuItem(hMenu, 0, TRUE, &mi);

    memset(&mi, 0, sizeof(mi));
    mi.type = MFT_STRING;
    mi.id = 100;
    mi.typedata = (DWORD)"File";
    mi.hsubmenu = create_popup_menu("File", file_menu_ids, file_menu_texts);
    InsertMenuItem(hMenu, 0, TRUE, &mi);
    
    return hMenu;
}

static unsigned char cur_ch;
static GLYPHBITMAP glyph;
static VBFINFO vbf;
static HWND hMainWnd;

static RECT rect_gridframe;
static RECT rect_table;
static RECT rect_char;
static RECT rect_status;


static VBF_BBOX *vbf_get_bbox(int ch)
{
    VBF_BBOX *temp =NULL;
    int i;

    if(!vbf.bbox || ch <vbf.first_glyph || ch >vbf.last_glyph)
        return  NULL;

    temp =vbf.bbox;
    if(ch ==vbf.first_glyph)
        return temp;

    for(i=1;i<=ch -vbf.first_glyph && temp;i++){
        temp = temp->next;
    }

    return temp;
}

static GLYPH_BITS *vbf_get_glyph_bmp_bits(int ch)
{
    GLYPH_BITS *temp =NULL;
    int i;

    if(!vbf.all_glyph_bits || ch <vbf.first_glyph || ch >vbf.last_glyph)
        return  NULL;

    temp =vbf.all_glyph_bits;
    if(ch ==vbf.first_glyph)
        return temp;

    for(i=1;i<=ch -vbf.first_glyph && temp;i++){
        temp =temp->next;
    }

    if(temp)
        return temp;
    else
        return NULL;
}

static char vbf_get_advance_x(int ch)
{
    ADVANCE_X *temp =NULL;
    int i;

    if(ch < vbf.first_glyph || ch > vbf.last_glyph)
        return 0;

    temp =vbf.advance_x;
    if(ch ==vbf.first_glyph)
        return temp->advance_x;

    for(i=1;i<ch -vbf.first_glyph && temp !=NULL;i++){
        temp =temp->next;
    }
    if(!temp)
        return 0;

    return temp->advance_x;
}

static int vbf_get_width(int ch)
{
    if(ch < vbf.first_glyph || ch > vbf.last_glyph)
        return 0;

    if(vbf.bbox){
        VBF_BBOX *temp =NULL;
        int i;

        temp =vbf.bbox;
        if(ch ==vbf.first_glyph)
            return temp->w;
        for(i=1;i<=ch -vbf.first_glyph && temp !=NULL;i++){
            temp =temp->next;
        }
        if(!temp)
            return 0;

        return temp->w;
    }

    if(vbf.advance_x){
        ADVANCE_X *temp =NULL;
        int i;

        temp =vbf.advance_x;
        if(ch ==vbf.first_glyph)
            return temp->advance_x;

        for(i=1;i<ch -vbf.first_glyph && temp !=NULL;i++){
            temp =temp->next;
        }
        if(!temp)
            return 0;

        return temp->advance_x;
    }

    return vbf.max_width;
}

static char get_pitch_from_width(int bits)
{
    return (bits + 7) >> 3;
}

static void vbf_update_max_width(void)
{
    int i;
    int max_width = 0;
    for(i = 0; i < 256; i++)
    {
        if (max_width < vbf_get_width(i))
            max_width = vbf_get_width(i);
    }
    if (max_width != vbf.max_width)
    {
        vbf.max_width = max_width;
    }
}

static BOOL vbf_get_char_glyph (int ch, GLYPHBITMAP* glyph)
{
    VBF_BBOX *vbf_bbox;
    GLYPH_BITS *glyph_bits;
    if (ch < vbf.first_glyph || ch > vbf.last_glyph)
        return FALSE;
    vbf_bbox =vbf_get_bbox(ch);
 
    if(vbf_bbox){   
        glyph->bbox_x = vbf_bbox->x;
        glyph->bbox_y = vbf_bbox->y;
        glyph->bbox_w = vbf_bbox->w;
        glyph->bbox_h = vbf_bbox->h;
    }
    else{
        glyph->bbox_x = 0;
        glyph->bbox_y = vbf.descent;
        glyph->bbox_w = vbf_get_width(ch);
        glyph->bbox_h = vbf.height;
    }
    glyph->advance_x = vbf_get_advance_x(ch);
    glyph->advance_y = 0;

    glyph->bmp_pitch = get_pitch_from_width(glyph->bbox_w);
    glyph->bmp_size = glyph->bmp_pitch * glyph->bbox_h;
    if (glyph->bits != NULL)
        free((unsigned char*)glyph->bits);
    glyph->bits = malloc(glyph->bmp_size);
    glyph_bits =vbf_get_glyph_bmp_bits(ch);

      memcpy((unsigned char*)glyph->bits, glyph_bits->glyph_bits, glyph->bmp_size);
    return TRUE;
}

static void vbf_set_char_glyph(int ch, GLYPHBITMAP* glyph)
{
    VBF_BBOX *bbox;
    GLYPH_BITS *glyph_bits;

    bbox =vbf_get_bbox(ch);
    glyph_bits =vbf_get_glyph_bmp_bits(ch);
    free(glyph_bits->glyph_bits);
    glyph_bits->glyph_bits =(unsigned char *)calloc(glyph->bmp_size,1);
    memcpy(glyph_bits->glyph_bits, glyph->bits, glyph->bmp_size);
    glyph_bits->bmp_size =glyph->bmp_size;

    if(bbox){
        bbox->x = glyph->bbox_x;
        bbox->y = glyph->bbox_y;
        bbox->w = glyph->bbox_w;
        bbox->h = glyph->bbox_h;
    }
}

static void initGridFrame (void)
{
    rect_gridframe.left    = GRID_X;
    rect_gridframe.top     = GRID_Y;
    rect_gridframe.right   = rect_gridframe.left + vbf.max_width*GRID_SIZE*3;
    rect_gridframe.bottom  = rect_gridframe.top + vbf.height*GRID_SIZE;

    rect_table.top = TABLE_Y;
    rect_table.left = TABLE_X;
    rect_table.right = TABLE_X + (vbf.max_width + 1)*TABLE_WIDTH;
    rect_table.bottom = TABLE_Y + (vbf.height + 1)*TABLE_HEIGHT;

    rect_char.left = rect_gridframe.right + GRID_MARGIN * 2;
    rect_char.top = rect_gridframe.top;
    rect_char.right = rect_char.left + vbf.max_width;
    rect_char.bottom = rect_char.top + vbf.height;

    rect_status.left = TXT_X;
    rect_status.top = TXT_Y;
    rect_status.right = TXT_X + 800;
    rect_status.bottom = TXT_Y + 40;
}

static BOOL initGlyph (unsigned char ch)
{
    vbf_get_char_glyph(ch, &glyph);
    cur_ch = ch;
    return TRUE;
}

#define RECT_GRIDFRAME  1
#define RECT_CHARTABLE  2
#define RECT_CHAR       4
#define RECT_STATUS     8
#define RECT_ALL        0xFFFFFFFF

static void invalidate_window(DWORD mask)
{
    if(mask == RECT_ALL)
    {
        InvalidateRect(hMainWnd, NULL, TRUE);
        return;
    }
    if(mask & RECT_GRIDFRAME)
        InvalidateRect(hMainWnd, &rect_gridframe, FALSE);
    if(mask & RECT_CHARTABLE)
        InvalidateRect(hMainWnd, &rect_table, FALSE);
    if(mask & RECT_CHAR)
        InvalidateRect(hMainWnd, &rect_char, FALSE);
    if(mask & RECT_STATUS)
        InvalidateRect(hMainWnd, &rect_status, TRUE);
}

static void drawGridLine (HDC hdc, RECT* frame) 
{
    int x, y;
    SetPenColor (hdc, PIXEL_lightgray);
    for (y = frame->top; y <= frame->bottom; y += GRID_SIZE) {
        MoveTo (hdc, frame->left, y);
        LineTo (hdc, frame->right, y);
    }
    for (x = frame->left; x <= frame->right; x += GRID_SIZE) {
        MoveTo (hdc, x, frame->top);
        LineTo (hdc, x, frame->bottom);
    }
}

static void drawBaseLine (HDC hdc, RECT* frame)
{
    int baseline_y;
    baseline_y = frame->bottom - GRID_SIZE * vbf.descent;
    SetPenColor (hdc, PIXEL_red);
    MoveTo (hdc, frame->left, baseline_y);
    LineTo (hdc, frame->right, baseline_y);
}

static void drawBoundingBox (HDC hdc, RECT* frame)
{
    int bound_left, bound_right, bound_top, bound_bottom;
    VBF_BBOX *bbox =NULL;

    bbox =vbf_get_bbox(cur_ch);

    if(bbox){
        bound_left = FRAMEOFFSET + bbox->x*GRID_SIZE;
        bound_right = bound_left + bbox->w*GRID_SIZE;
        bound_top = frame->top + (vbf.height - vbf.descent
                - bbox->y)*GRID_SIZE;
        bound_bottom = bound_top + bbox->h*GRID_SIZE;
    }
    else{
        bound_left = FRAMEOFFSET + 0*GRID_SIZE;
        bound_right = bound_left + vbf_get_advance_x(cur_ch)*GRID_SIZE;
        bound_top = frame->top + (vbf.height - vbf.descent
                - (vbf.height -vbf.descent))*GRID_SIZE;
        bound_bottom = bound_top + vbf.height*GRID_SIZE;
    }

    SetPenColor (hdc, PIXEL_green);
    MoveTo (hdc, bound_left, bound_top);
    LineTo (hdc, bound_right, bound_top);
    LineTo (hdc, bound_right, bound_bottom);
    LineTo (hdc, bound_left, bound_bottom);
    LineTo (hdc, bound_left, bound_top);
}

static void drawFontRectBox (HDC hdc, RECT* frame) 
{
    int font_left, font_top, font_right, font_bottom;

    font_left = FRAMEOFFSET;
    font_right = font_left + vbf_get_width(cur_ch)*GRID_SIZE;
    font_top = frame->top;
    font_bottom = font_top + vbf.height*GRID_SIZE;

    SetPenColor (hdc, PIXEL_red);

    MoveTo (hdc, font_left, font_top);
    LineTo (hdc, font_right, font_top);
    LineTo (hdc, font_right, font_bottom);
    LineTo (hdc, font_left, font_bottom);
    LineTo (hdc, font_left, font_top);
}

static void drawGridFrame (HWND hWnd, HDC hdc, RECT* frame, GLYPHBITMAP* glyph)
{
    WINDOWINFO* pWin =NULL;

    pWin =(WINDOWINFO*)GetWindowInfo(hWnd);
    pWin->we_rdr->draw_3dbox(hdc,frame,PIXEL_lightwhite,LFRDR_3DBOX_THICKFRAME|LFRDR_3DBOX_FILLED);
    drawGridLine (hdc, frame);
    drawBaseLine (hdc, frame);
    drawFontRectBox (hdc, frame); 
    drawBoundingBox (hdc, frame); 
}

static void drawCharTable (HDC hdc, int table_x, int table_y)
{
    GLYPHBITMAP tmp_glyph;
    int i;
    int x, y;
    int x_step = vbf.max_width + 1;
    int y_step = vbf.height + 1;
    SetPenColor(hdc, PIXEL_darkgray);
    for (i = 0; i <= TABLE_WIDTH; i++)
    {
        LineEx(hdc, table_x + x_step*i, table_y,
                    table_x + x_step*i, table_y + y_step*TABLE_HEIGHT);
    }
    for (i = 0; i <= TABLE_HEIGHT; i++)
    {
        LineEx(hdc, table_x,                      table_y + y_step*i,
                    table_x + x_step*TABLE_WIDTH, table_y + y_step*i);
    }
    memset(&tmp_glyph, 0, sizeof(tmp_glyph));
    for (i = 0; i < 256; i++)
    {
        if(i < vbf.first_glyph || i > vbf.last_glyph)
            continue;
        vbf_get_char_glyph(i, &tmp_glyph);
        x = table_x + x_step * (i%TABLE_WIDTH) + 1;
        y = table_y + y_step * (i/TABLE_WIDTH) + 1;
        glyph_draw(&tmp_glyph, hdc, x, y+tmp_glyph.bbox_y, 1, PIXEL_black);
        glyph_free(&tmp_glyph);
    }
}

static void drawStatusText (HDC hdc, int txt_x, int txt_y)
{
    SetBkMode (hdc, BM_TRANSPARENT);
    if (vbf.name&&vbf.name [0]) {
        VBF_BBOX *info =vbf_get_bbox(cur_ch);
        char text [256];
        char bbox_text1 [256];
        char bbox_text2 [256];
        sprintf (text, "font: %s, character: 0x%x: %c", vbf.name, cur_ch, cur_ch);
        TextOut (hdc, txt_x, txt_y, text);
        if (info != NULL)
        {
            sprintf (bbox_text1, 
                    "bbox_x: %d, bbox_y: %d,"
                    "bbox_w: %d, bbox_h: %d,",
                    info->x, info->y, 
                    info->w, info->h);
            TextOut (hdc, txt_x, txt_y + 20, bbox_text1);
            sprintf (bbox_text2, 
                    "width: %d, height: %d, "
                    "max_width: %d, ave_width: %d",
                    vbf_get_advance_x(cur_ch), vbf.height, vbf.max_width, vbf.ave_width);
            TextOut (hdc, txt_x, txt_y + 40, bbox_text2);
        }
    }
    else {
        TextOut (hdc, txt_x, txt_y, "Please load a VBF font");
    }
}

static void drawEditorWindow (HWND hWnd, HDC hdc, RECT* frame)
{
    int baseline_y;

    drawStatusText (hdc, TXT_X, TXT_Y);
    if (vbf.name&&vbf.name [0])
    {
        baseline_y = (vbf.height - vbf.descent);
        drawCharTable(hdc, TABLE_X, TABLE_Y);
        drawGridFrame(hWnd, hdc, frame, &glyph); 
        glyph_draw(&glyph, hdc, FRAMEOFFSET,
                frame->top + GRID_SIZE*glyph.bbox_y, GRID_SIZE, PIXEL_darkgray);
        glyph_draw(&glyph, hdc, rect_char.left,
                rect_char.top , 1, PIXEL_black); 
    }
}

static void setCharWidth(int width)
{
    ADVANCE_X *temp =NULL;
    int i;

    if (width < 1)
        return;

    temp =vbf.advance_x;
    if(cur_ch ==vbf.first_glyph)
        temp->advance_x =width;

    for(i=1;i<=cur_ch -vbf.first_glyph && temp;i++){
        temp =temp->next;
    }

    if(temp)
        temp->advance_x =width;
    vbf_update_max_width();
    initGridFrame();
}

static void setCharHeight(int height)
{
    if (height < 1)
        return;
    vbf.height = height;
    initGridFrame();
}

static int RenameProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    int id;
    switch( message ) {
        case MSG_INITDIALOG:
            SendMessage (GetDlgItem(hDlg, IDC_EDIT_NAME), EM_LIMITTEXT, LEN_DEVFONT_NAME, 0);
            SendMessage(GetDlgItem(hDlg, IDC_EDIT_NAME), MSG_SETTEXT, 0, (LPARAM) vbf.name);
            return 0;
            
        case MSG_COMMAND:
        {
            id = LOWORD(wParam);
            switch (id)
            {
                case IDOK_NAME:
                    SendMessage(GetDlgItem(hDlg, IDC_EDIT_NAME), MSG_GETTEXT, LEN_DEVFONT_NAME,
                                (LPARAM) vbf.name);
                    EndDialog (hDlg, 0);
                    break;
            }
        }
            break;
    }
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

static HWND create_rename_dlg(HWND hWnd)
{
    RenameDlg.controls = CtrlRenameDlg;
    return DialogBoxIndirectParam(&RenameDlg, hWnd, RenameProc, 0L);
}

static int ViewWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static NEWFILEDLGDATA filedlgdata ={FALSE, FALSE, ".", "*.vbf", "./", "All file(*.*)|vbf file(*.vbf)",1};

    switch (message) {
        case MSG_CREATE:
            break;

        case MSG_ACTIVEMENU:
            if (vbf.name&&vbf.name[0] == '\0')
            {
                EnableMenuItem((HMENU)lParam, IDC_FILE_RENAME, MF_DISABLED);
                EnableMenuItem((HMENU)lParam, IDC_UPDATE, MF_DISABLED);
            }
            else
            {
                EnableMenuItem((HMENU)lParam, IDC_FILE_RENAME, MF_ENABLED);
                if (strcmp (vbf.ver_info, VBF_VERSION3))
                    EnableMenuItem((HMENU)lParam, IDC_UPDATE, MF_ENABLED);
                else
                    EnableMenuItem((HMENU)lParam, IDC_UPDATE, MF_DISABLED);
            }
            break;

        case MSG_COMMAND:
        {
            switch (LOWORD(wParam)) {
                case IDC_FILE_OPEN:
					if (ShowOpenDialog (hWnd, 0,0,400,300, &filedlgdata) == IDOK) {
                        freeVBF (&vbf);
                        if (loadVBF (&vbf, filedlgdata.filefullname)&& initGlyph (vbf.first_glyph)) {
                            initGridFrame ();
                            invalidate_window(RECT_ALL);
                        }
                    }
                    break;

                case IDC_FILE_SAVE:
                    filedlgdata.IsSave =TRUE;
					if (vbf.name&&vbf.name[0] && (ShowOpenDialog (hWnd, 0,0,400,300, &filedlgdata) == IDOK)) {
                        dumpVBF (&vbf, filedlgdata.filefullname);
                        
                    }
                    break;
                    
                case IDC_FILE_RENAME:
                    create_rename_dlg(hWnd);
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;

                case IDC_CHAR_PREV:
                    if (vbf.name [0] && cur_ch > vbf.first_glyph) {
                        cur_ch --;
                        initGlyph (cur_ch);
                        invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_STATUS);
                    }
                    break;

                case IDC_CHAR_NEXT:
                    if (vbf.name [0] && cur_ch < vbf.last_glyph) {
                        cur_ch ++;
                        initGlyph (cur_ch);
                        invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_STATUS);
                    }
                    break;

                case IDC_UPDATE:
                    strcpy(vbf.ver_info, VBF_VERSION3);
                    break;

                case IDC_CHAR_INC_BB_X:
                    glyph.bbox_x++;
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_DEC_BB_X:
                    glyph.bbox_x--;
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_INC_BB_Y:
                    glyph.bbox_y++;
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_DEC_BB_Y:
                    glyph.bbox_y--;
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_INC_BB_W:
                    glyph_change_width_right(&glyph, glyph.bbox_w+1);
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_DEC_BB_W:
                    glyph_change_width_right(&glyph, glyph.bbox_w-1);
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_INC_BB_H:
                    glyph_change_height_bottom(&glyph, glyph.bbox_h+1);
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_DEC_BB_H:
                    glyph_change_height_bottom(&glyph, glyph.bbox_h-1);
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_DEC_BB_W_FROM_LEFT:
                    glyph_change_width_left(&glyph, glyph.bbox_w-1);
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHAR_DEC_BB_H_FROM_TOP:
                    glyph_change_height_top(&glyph, glyph.bbox_h-1);
                    vbf_set_char_glyph(cur_ch, &glyph);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHARWIDTHADD:
                    setCharWidth(vbf_get_advance_x(cur_ch)+1);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHARWIDTHDEC:
                    setCharWidth(vbf_get_advance_x(cur_ch)+1);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHARHEIGHTADD:
                    setCharHeight(vbf.height + 1);
                    invalidate_window(RECT_ALL);
                    break;

                case IDC_CHARHEIGHTDEC:
                    setCharHeight(vbf.height - 1);
                    invalidate_window(RECT_ALL);
                    break;

            }
        }
        break;

        case MSG_KEYDOWN:
            if (LOWORD(wParam) == SCANCODE_ENTER&& glyph.bmp_size)
            {
                GLYPH_BITS *glyph_bits;

                glyph_bits =vbf_get_glyph_bmp_bits(cur_ch);
                memcpy((unsigned char*)glyph_bits->glyph_bits, glyph.bits, glyph.bmp_size);
                break;
            }
            if (LOWORD(wParam) == SCANCODE_F1)
                return 0;
            if (lParam & KS_SHIFT)
            {
                switch (LOWORD(wParam)) {
                    case SCANCODE_CURSORBLOCKLEFT:
                        glyph_move_left(&glyph);
                        invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_CHARTABLE);
                        break;

                    case SCANCODE_CURSORBLOCKRIGHT:
                        glyph_move_right(&glyph);
                        invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_CHARTABLE);
                        break;

                    case SCANCODE_CURSORBLOCKUP:
                        glyph_move_up(&glyph);
                        invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_CHARTABLE);
                        break;

                    case SCANCODE_CURSORBLOCKDOWN:
                        glyph_move_down(&glyph);
                        invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_CHARTABLE);
                        break;
                }
            }
            else if (lParam & KS_CTRL)
            {
                switch (LOWORD(wParam)) {
                    case SCANCODE_CURSORBLOCKLEFT:
                        glyph.bbox_x--;
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_CURSORBLOCKRIGHT:
                        glyph.bbox_x++;
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_CURSORBLOCKUP:
                        glyph.bbox_y++;
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_CURSORBLOCKDOWN:
                        glyph.bbox_y--;
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                }
            }
            else
            {
                int ch = cur_ch;
                switch (LOWORD(wParam)) {
                    case SCANCODE_CURSORBLOCKUP:
                        ch --;
                        break;
                    case SCANCODE_CURSORBLOCKDOWN:
                        ch ++;
                        break;
                    case SCANCODE_PAGEUP:
                        ch -= TABLE_WIDTH;
                        break;
                    case SCANCODE_PAGEDOWN:
                        ch += TABLE_WIDTH;
                        break;
                    case SCANCODE_H:
                        glyph_change_width_left(&glyph, glyph.bbox_w-1);
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_J:
                        glyph_change_height_bottom(&glyph, glyph.bbox_h-1);
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_K:
                        glyph_change_height_top(&glyph, glyph.bbox_h-1);
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_L:
                        glyph_change_width_right(&glyph, glyph.bbox_w-1);
                        vbf_set_char_glyph(cur_ch, &glyph);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_A:
                        setCharWidth(vbf_get_advance_x(cur_ch)+1);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_D:
                        setCharWidth(vbf_get_advance_x(cur_ch)+1);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_W:
                        setCharHeight(vbf.height+1);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_S:
                        setCharHeight(vbf.height-1);
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_Y:
                        vbf.descent++;
                        invalidate_window(RECT_ALL);
                        break;
                    case SCANCODE_B:
                        vbf.descent--;
                        invalidate_window(RECT_ALL);
                        break;

                }
                if (cur_ch != ch)
                {
                    if (ch < vbf.first_glyph)
                        ch = vbf.first_glyph;
                    if (ch > vbf.last_glyph)
                        ch = vbf.last_glyph;
                    cur_ch = ch;
                    initGlyph (cur_ch);
                    invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_STATUS);
                }
            }
            return 0;

        case MSG_LBUTTONDOWN:
        {
            int x = LOWORD (lParam);
            int y = HIWORD (lParam);
            RECT rect_bound;
            rect_bound.left = FRAMEOFFSET + glyph.bbox_x*GRID_SIZE;
            rect_bound.right = rect_bound.left + glyph.bbox_w*GRID_SIZE;
            rect_bound.top = rect_gridframe.top ;
            rect_bound.bottom = rect_bound.top + glyph.bbox_h*GRID_SIZE;

            if (PtInRect (&rect_bound, x, y)) {
                int i, j;
                i = (y - rect_bound.top) / GRID_SIZE;
                j = (x - rect_bound.left) / GRID_SIZE;
                if (j > glyph.bbox_w)
                {
                    Ping ();
                    break;
                }
                glyph_set_bit(&glyph, i, j, !glyph_get_bit(&glyph, i, j));
                invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_CHARTABLE);
            }
            else if (PtInRect(&rect_table, x, y))
            {
                cur_ch = (y - rect_table.top)/(vbf.height + 1)*TABLE_WIDTH
                    + (x - rect_table.left)/(vbf.max_width + 1);
                initGlyph(cur_ch);
                invalidate_window(RECT_ALL);
            }
            break;
        }

        case MSG_PAINT:
        {
            HDC hdc = BeginPaint (hWnd);
            drawEditorWindow (hWnd, hdc, &rect_gridframe);
            EndPaint (hWnd, hdc);
            return 0;
        }

        case MSG_CLOSE:
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
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , arg[0], 0 , 0);
#endif

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "VBF Editor";
    CreateInfo.hMenu = create_main_menu();
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ViewWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 640;
    CreateInfo.by = 480;
    CreateInfo.iBkColor = PIXEL_lightgray;
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

