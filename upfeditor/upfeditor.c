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

#include "upf.h"

#define IDC_FILE_OPEN           110
#define IDC_FILE_SAVE           120
#define IDC_FILE_RENAME         130
#define IDC_CHAR_PREV           140
#define IDC_CHAR_NEXT           150
#define IDC_ZOOM_OUT            160
#define IDC_ZOOM_IN             170
#define IDC_FILE_NEW          171

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

#define IDC_CHAR_ADD                     390
#define IDC_CHAR_DEL                     400


#define TXT_X                   5
#define TXT_Y                   10

#define CHARWIDTH_X             (TXT_X + 2) 
#define CHARWIDTH_Y             (TXT_Y + 70)
#define CHARHEIGHT_Y            (CHARWIDTH_Y + 50)
#define CHARWIDTH_W             60
#define CHARWIDTH_H             20

#define GRID_X                  CHARWIDTH_X
#define GRID_Y                  CHARWIDTH_Y
#define GRID_SIZE               10

//#define TABLE_X                 510
#define TABLE_X                 0
//#define TABLE_Y                 (GRID_Y + GRID_SIZE * 25)
//#define TABLE_Y                 CHARWIDTH_Y
#define TABLE_Y                 0
#define TABLE_WIDTH             16
//#define TABLE_HEIGHT            (256/TABLE_WIDTH)
#define TABLE_HEIGHT            (32768 / TABLE_WIDTH)

#define GRID_MARGIN             5

#define FRAMEOFFSET              (rect_gridframe.left + (rect_gridframe.right - rect_gridframe.left)/3)

#define WIN_WIDTH               800
#define WIN_HEIGHT              600


#define VAL_DEBUG
#include <my_debug.h>

typedef int (*CONVTO32)(const unsigned char* mchar);
CONVTO32 conv_to_uc32 = utf16le_conv_to_uc32;
BOOL scrollwnd1_fresh;
BOOL scrollwnd2_fresh;

static HWND hscroll_wnd1;
static HDC  mem_dc1 = 0; 
static HWND hscroll_wnd2;
static HDC  mem_dc2 = 0; 

static int file_menu_ids[] =
{
    IDC_FILE_OPEN,
    IDC_FILE_SAVE,
    IDC_FILE_NEW,
 //   IDC_FILE_RENAME,
    0
};
static char* file_menu_texts[] =
{
    "Open",
    "Save As",
    "new font"
//    "Rename",
};
static int char_menu_ids[] =
{
    //IDC_CHAR_PREV,
    //IDC_CHAR_NEXT,
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
    IDC_CHAR_ADD,
    IDC_CHAR_DEL,
	/*
    IDC_CHARWIDTHADD,
    IDC_CHARWIDTHDEC,
    IDC_CHARHEIGHTADD,
    IDC_CHARHEIGHTDEC,
	*/
    0
};
static char* char_menu_texts[] =
{
    //"Previous",
    //"Next",
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
    "Add a Char",
    "del a char"
	/*
    "CHAR WIDTHADD",
    "CHAR WIDTHDEC",
    "CHAR HEIGHTADD",
    "CHAR HEIGHTDEC",
	*/
};

/*
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
*/


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
    
	/*
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
	*/

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

static int cur_ch;
static BOOL file_size_change; 
static GLYPHBITMAP g_glyph_bitmap;
//static QPFINFO qpf_info;
static UPFINFO  upf_info;
//static UPFV1_FILE_HEADER  upf_file_header;
//static UPFGLYPHTREE*  upf_tree=NULL;
static HWND hMainWnd;

static RECT rect_gridframe;
static RECT rect_table;
static RECT rect_char;
static RECT rect_status;




#define MAX_GRIDFRAME_WIDTH         160 

/*init 3 rects (display all glyphs in scrollwnd,  
 * display pixels of current glyph ((3*w) * h),
 * display infomation text)*/
static void init_display_rects (void)
{
    rect_gridframe.left    = GRID_X;
    rect_gridframe.top     = GRID_Y - 3;

    /*gridframe is max_width of the font*/
    rect_gridframe.right   = rect_gridframe.left + upf_info.upf_file_header.max_width*GRID_SIZE*3;

    rect_gridframe.right   = rect_gridframe.left + upf_info.upf_file_header.max_width*GRID_SIZE*3;
    /*gridframe is height of the font*/
    rect_gridframe.bottom  = rect_gridframe.top + upf_info.upf_file_header.height*GRID_SIZE;

    /*table in scrollwnd to display all glyph*/
    rect_table.top = TABLE_Y;
    rect_table.left = TABLE_X;
    rect_table.right = TABLE_X + (upf_info.upf_file_header.max_width + 1)*TABLE_WIDTH;
    rect_table.bottom = TABLE_Y + (upf_info.upf_file_header.height + 1)*TABLE_HEIGHT;

    /*rect to display current glyph*/
    rect_char.left = rect_gridframe.right / 3;
    rect_char.top = rect_gridframe.top - GRID_MARGIN*2 - upf_info.upf_file_header.height;
    rect_char.right = rect_char.left + upf_info.upf_file_header.max_width;
    rect_char.bottom = rect_char.top + upf_info.upf_file_header.height;

    /*rect to display infomation text*/
    rect_status.left = TXT_X;
    rect_status.top = TXT_Y;
    rect_status.right = TXT_X + 800;
    rect_status.bottom = TXT_Y + 40;
}

static BOOL init_glyph (int ch)
{
    upf_get_char_glyph_bitmap(&upf_info, ch, &g_glyph_bitmap);
    cur_ch = ch;
    return TRUE;
}

#define RECT_GRIDFRAME  1
#define RECT_CHARTABLE  2
#define RECT_CHAR       4
#define RECT_STATUS     8
#define RECT_ALL        0xFFFFFFFF

/*invalidate some rects of main window*/
static void invalidate_window(DWORD mask)
{
    if(mask == RECT_ALL)
    {
        InvalidateRect(hMainWnd, NULL, TRUE);
		//printf("invalidate_window ALL\n");
        return;
    }
    if(mask & RECT_GRIDFRAME)
        InvalidateRect(hMainWnd, &rect_gridframe, FALSE);
    if(mask & RECT_CHARTABLE)
    {
        InvalidateRect(hscroll_wnd2, NULL, FALSE);
        InvalidateRect(hscroll_wnd1, NULL, FALSE);
    }
    if(mask & RECT_CHAR)
        InvalidateRect(hMainWnd, &rect_char, FALSE);
    if(mask & RECT_STATUS)
        InvalidateRect(hMainWnd, &rect_status, TRUE);
}

static void draw_grid_line (HDC hdc, RECT* frame) 
{
    int x, y;
    SetPenColor (hdc, PIXEL_lightgray);
    /*draw h-lines*/
    for (y = frame->top; y <= frame->bottom; y += GRID_SIZE) {
        MoveTo (hdc, frame->left, y);
        LineTo (hdc, frame->right, y);
    }
    /*draw v-lines*/
    for (x = frame->left; x <= frame->right; x += GRID_SIZE) {
        MoveTo (hdc, x, frame->top);
        LineTo (hdc, x, frame->bottom);
    }
}

static void draw_base_line (HDC hdc, RECT* frame)
{
    int baseline_y;
    baseline_y = frame->bottom - GRID_SIZE * upf_info.upf_file_header.descent;
    SetPenColor (hdc, PIXEL_red);
    MoveTo (hdc, frame->left, baseline_y);
    LineTo (hdc, frame->right, baseline_y);
}

static void draw_bounding_box (HDC hdc, RECT* frame, GLYPHBITMAP *glyph_bitmap)
{
    int bound_left, bound_right, bound_top, bound_bottom;
    int baseline_y;

    baseline_y = frame->bottom - GRID_SIZE * upf_info.upf_file_header.descent;

    bound_left = FRAMEOFFSET + glyph_bitmap->bbox_x*GRID_SIZE;
    bound_right = bound_left + glyph_bitmap->bbox_w*GRID_SIZE;
    bound_top = frame->top + (upf_info.upf_file_header.ascent + upf_info.upf_file_header.leading - glyph_bitmap->bbox_y)*GRID_SIZE;
    bound_bottom = bound_top + glyph_bitmap->bbox_h*GRID_SIZE;

    SetPenColor (hdc, PIXEL_green);
    MoveTo (hdc, bound_left, bound_top);
    LineTo (hdc, bound_right, bound_top);
    LineTo (hdc, bound_right, bound_bottom);
    LineTo (hdc, bound_left, bound_bottom);
    LineTo (hdc, bound_left, bound_top);
}

static void draw_font_rect_box (HDC hdc, RECT* frame, GLYPHBITMAP *glyph) 
{
    int font_left, font_top, font_right, font_bottom;

    font_left = FRAMEOFFSET;
    font_right = font_left + glyph->advance_x * GRID_SIZE;
    font_top = frame->top;
    font_bottom = font_top + (upf_info.upf_file_header.height)*GRID_SIZE;


    SetPenColor (hdc, PIXEL_red);

    MoveTo (hdc, font_left, font_top);
    LineTo (hdc, font_right, font_top);
    LineTo (hdc, font_right, font_bottom);
    LineTo (hdc, font_left, font_bottom);
    LineTo (hdc, font_left, font_top);
}

static void draw_grid_frame (HWND hWnd, HDC hdc, RECT* frame, GLYPHBITMAP* glyph)
{
    const WINDOWINFO*  pWin=NULL;
    pWin =GetWindowInfo(hWnd);;

    /*draw a big 3dbox on frame (rect_gridframe)*/
    pWin->we_rdr->draw_3dbox(hdc,frame,PIXEL_lightwhite,
            LFRDR_3DBOX_THICKFRAME|LFRDR_3DBOX_FILLED);
    draw_grid_line (hdc, frame);
    draw_base_line (hdc, frame);
    draw_font_rect_box (hdc, frame, glyph); 
    draw_bounding_box (hdc, frame, glyph); 
}

static void draw_char_table1 (HDC hdc, int table_x, int table_y)
{
    GLYPHBITMAP tmp_glyph;
    int i;
    int x, y;
    int x_step; 
    int y_step;

    x_step = upf_info.upf_file_header.max_width + 1;
    y_step = upf_info.upf_file_header.height + 1;
    SetPenColor(hdc, PIXEL_darkgray);

    for (i = 0; i <= TABLE_WIDTH; i++) {
        MoveTo (hdc, table_x + x_step*i, table_y);
        LineTo (hdc, table_x + x_step*i, table_y + y_step*TABLE_HEIGHT);
    }

    for (i = 0; i <= TABLE_HEIGHT; i++) {
        MoveTo(hdc, table_x, table_y + y_step*i);
        LineTo(hdc, table_x + x_step*TABLE_WIDTH, table_y + y_step*i);
    }

    memset(&tmp_glyph, 0, sizeof(tmp_glyph));
    for (i = 0; i < 32768; i++) {
        upf_get_char_glyph_bitmap(&upf_info, i, &tmp_glyph);
        x = table_x + x_step * (i%TABLE_WIDTH) + 1;
        y = table_y + y_step * (i/TABLE_WIDTH) + 1;
        glyph_draw(&tmp_glyph, hdc, x, y+upf_info.upf_file_header.ascent, 1, PIXEL_black);
    }
    //FIXME
    //glyph_free(&tmp_glyph);
}

static void draw_char_table2 (HDC hdc, int table_x, int table_y)
{
    GLYPHBITMAP tmp_glyph;
    int i;
    int x, y;
	int x_step; 
	int y_step;

    x_step = upf_info.upf_file_header.max_width + 1;
    y_step = upf_info.upf_file_header.height + 1;
    SetPenColor(hdc, PIXEL_darkgray);

    for (i = 0; i <= TABLE_WIDTH; i++) {
        MoveTo (hdc, table_x + x_step*i, table_y);
        LineTo (hdc, table_x + x_step*i, table_y + y_step*TABLE_HEIGHT);
    }

    for (i = 0; i <= TABLE_HEIGHT; i++) {
        MoveTo (hdc, table_x,table_y + y_step*i);
        LineTo (hdc, table_x + x_step*TABLE_WIDTH, table_y + y_step*i );
    }

    memset(&tmp_glyph, 0, sizeof(tmp_glyph));
    for (i = 32768; i < 65536; i++) {
        upf_get_char_glyph_bitmap(&upf_info, i, &tmp_glyph);
        x = table_x + x_step * ((i - 32768)%TABLE_WIDTH) + 1;
        y = table_y + y_step * ((i - 32768)/TABLE_WIDTH) + 1;
        glyph_draw(&tmp_glyph, hdc, x, y+upf_info.upf_file_header.ascent, 1, PIXEL_black);
		//FIXME
        //glyph_free(&tmp_glyph);
    }
}

static void draw_status_text (HDC hdc, int txt_x, int txt_y)
{
	GLYPHBITMAP glyph_bitmap;
	char text [256];
	char bbox_text1 [256];
	PLOGFONT log_font = NULL;

	log_font = GetSystemFont(SYSLOGFONT_CONTROL);
	SelectFont(hdc, log_font);
    SetBkMode (hdc, BM_TRANSPARENT);

    if (upf_info.upf_tree != NULL) {
		upf_get_char_glyph_bitmap(&upf_info, cur_ch, &glyph_bitmap);

        printf("font file:%s\n",upf_info.upf_file_header.font_name);
        printf("ver_info:%s\n",upf_info.upf_file_header.ver_info);
        printf("vender_name:%s\n",upf_info.upf_file_header.vender_name);
        sprintf (text, "font file: %s", upf_info.upf_file_header.font_name);
        TextOut (hdc, txt_x, txt_y, text);
		{
			sprintf (bbox_text1, 
					"character: Hex:0x%x Dec:%d ,  "
					"bbox_x: %d,  bbox_y: %d,  "
					"bbox_w: %d,  bbox_h: %d,  "
					"advance: %d,  height: %d,  "
					"max_width: %d",
					cur_ch, cur_ch,
					glyph_bitmap.bbox_x, glyph_bitmap.bbox_y, 
					glyph_bitmap.bbox_w, glyph_bitmap.bbox_h,
					glyph_bitmap.advance_x, upf_info.upf_file_header.height, upf_info.upf_file_header.max_width);
			TextOut (hdc, txt_x, txt_y + 20, bbox_text1);
		}

		TextOut (hdc, 30, 265, "Uincode:");
		TextOut (hdc, 30, 285, "  0-32767");
		TextOut (hdc, 395, 265, "Uincode:");
		TextOut (hdc, 395, 285, "  32768-65535");
    }
    else {
        TextOut (hdc, txt_x, txt_y, "Please load a UPF font");
    }
}

/*draw edit area*/
static void draw_editor_window (HWND hWnd,HDC hdc, RECT* frame)
{
    int baseline_y;

	draw_status_text (hdc, TXT_X, TXT_Y);

    if (upf_info.upf_tree != NULL)
    {
        baseline_y = (upf_info.upf_file_header.ascent + upf_info.upf_file_header.leading);
        draw_grid_frame(hWnd,hdc, frame, &g_glyph_bitmap); 
        glyph_draw(&g_glyph_bitmap, hdc, FRAMEOFFSET,
                frame->top + GRID_SIZE*baseline_y, GRID_SIZE, PIXEL_darkgray);
        glyph_draw(&g_glyph_bitmap, hdc, rect_char.left,
                rect_char.top + baseline_y, 1, PIXEL_black); 
    }
}

/*show a winentry to add a char,
 * return the unicode*/
static int add_char_mywinentry(HWND hWnd)
{
    int result;
    char uc[10];
    char* unicode_char = uc;
    int unicode;

    myWINENTRY EntryItems [] = {
        { "Dec Unicode in DEC", &unicode_char, 0, 0 },
        { NULL, NULL, 0, 0 }
    };

    myWINBUTTON buttons[] = {
        { "OK", IDOK, WS_VISIBLE | BS_PUSHBUTTON },
        { "Cancel", IDCANCEL, WS_VISIBLE },
        { NULL, 0, 0 }
    };

    sprintf(uc, "%d", 0);
    result = myWinEntries(hWnd, "Add a Char" , "" , 
            120 , 80 , FALSE, EntryItems, buttons); 

    if (unicode_char != NULL) {
        //FIXME is number???
		int i = 0;

		for (i = 0; unicode_char[i] != '\0'; i++) {
			if (!isdigit(unicode_char[i])) {
				free (unicode_char);
			   	return -3;
			}
		}

        unicode = atoi (unicode_char);
        free (unicode_char);
    } else {
        return -1;
    }

    if (result != IDOK) return -2;
    if (unicode < 0 || unicode > 65535) {
        return -3;
    }

    return unicode;
}



#define IDC_SCROLLWND1          500
#define IDC_SCROLLWND2          600

#define MSG_RECREATE_MEMDC      (MSG_USER + 1)
static int char_table_container_proc1 (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {

		case MSG_PAINT:
            {
                RECT rc;
                if (upf_info.upf_tree == NULL) break;

                HDC hdc = BeginPaint (hWnd);
                GetClientRect(hWnd, &rc);

                if (mem_dc1 == 0) {
                    mem_dc1 = CreateCompatibleDC(hdc);
                    SetBrushColor(mem_dc1, PIXEL_lightgray);
                    FillBox(mem_dc1, 0, 0, RECTW(rc), RECTH(rc));
                    draw_char_table1(mem_dc1, TABLE_X, TABLE_Y);
                } 
                else if (cur_ch < 32768){
                    GLYPHBITMAP tmp_glyph;
                    int x_step;
                    int y_step;
                    int x,y;

                    x_step = upf_info.upf_file_header.max_width + 1;
                    y_step = upf_info.upf_file_header.height + 1;

                    x = TABLE_X + x_step * (cur_ch%TABLE_WIDTH) + 1;
                    y = TABLE_Y + y_step * (cur_ch/TABLE_WIDTH) + 1;

                    upf_get_char_glyph_bitmap(&upf_info, cur_ch, &tmp_glyph);
                    Rectangle (mem_dc1, x-1, y-1, x-1+x_step, y-1+y_step);

                    FillBox(mem_dc1, x, y, x_step-1, y_step-1);
                    glyph_draw(&tmp_glyph, mem_dc1, x, y+upf_info.upf_file_header.ascent, 1, PIXEL_black);

                }
                /*TODO draw the current glyph, if it in this table
                 * .........
                 * */

                BitBlt(mem_dc1, TABLE_X, TABLE_Y, 
                        RECTW(rc), RECTH(rc),
                        hdc, TABLE_X, TABLE_Y, 0);

                EndPaint (hWnd, hdc);
                return 0;
            }

		case MSG_CLOSE:
			{
				if (mem_dc1 != 0) {
					DeleteMemDC(mem_dc1);
					mem_dc1 = 0;
				}
				break;
			}

		case MSG_LBUTTONDOWN:
			{
				int x = LOWORD (lParam);
				int y = HIWORD (lParam);

				if (upf_info.upf_tree == NULL) return -1;

                /*get cur_ch from mouse pos*/
                cur_ch = (y - rect_table.top)/(upf_info.upf_file_header.height + 1)*TABLE_WIDTH
                    + (x - rect_table.left)/(upf_info.upf_file_header.max_width + 1);

                init_glyph(cur_ch);
                /*invalidate parent*/
                invalidate_window(RECT_ALL);
			}

		case MSG_ERASEBKGND:
			return 0;

	}

	return DefaultContainerProc (hWnd, message, wParam, lParam);
}


static int char_table_container_proc2 (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case MSG_PAINT:
            {
                RECT rc;
                if (upf_info.upf_tree == NULL) break;

                HDC hdc = BeginPaint (hWnd);
                GetClientRect(hWnd, &rc);

                if (mem_dc2 == 0) {
                    mem_dc2 = CreateCompatibleDC(hdc);
                    SetBrushColor(mem_dc2, PIXEL_lightgray);
                    FillBox(mem_dc2, 0, 0, RECTW(rc), RECTH(rc));
                    draw_char_table2(mem_dc2, TABLE_X, TABLE_Y);
                }
                else if (cur_ch >= 32768){
                    GLYPHBITMAP tmp_glyph;
                    int x_step;
                    int y_step;
                    int x,y;

                    x_step = upf_info.upf_file_header.max_width + 1;
                    y_step = upf_info.upf_file_header.height + 1;

                    x = TABLE_X + x_step * ((cur_ch - 32768)%TABLE_WIDTH) + 1;
                    y = TABLE_Y + y_step * ((cur_ch - 32768)/TABLE_WIDTH) + 1;

                    upf_get_char_glyph_bitmap(&upf_info, cur_ch, &tmp_glyph);

                    Rectangle (mem_dc2, x-1, y-1, x-1+x_step, y-1+y_step);
                    FillBox(mem_dc2, x, y, x_step-1, y_step-1);
                    glyph_draw(&tmp_glyph, mem_dc2, x, y+upf_info.upf_file_header.ascent, 1, PIXEL_black);

                }

                /*TODO draw the current glyph, if it in this table
                 * .........
                 * */
                BitBlt(mem_dc2, TABLE_X, TABLE_Y, 
                        RECTW(rc), RECTH(rc),
                        hdc, TABLE_X, TABLE_Y, 0);

                EndPaint (hWnd, hdc);
                return 0;
            }

		case MSG_CLOSE:
			{
				if (mem_dc2 != 0) {
					DeleteMemDC(mem_dc2);
					mem_dc2 = 0;
				}

				break;
			}

		case MSG_LBUTTONDOWN:
			{
				int x = LOWORD (lParam);
				int y = HIWORD (lParam);

				if (upf_info.upf_tree == NULL) return -1;

                cur_ch = (y - rect_table.top)/(upf_info.upf_file_header.height + 1)*TABLE_WIDTH
                    + (x - rect_table.left)/(upf_info.upf_file_header.max_width + 1);
				cur_ch += 32768;
                init_glyph(cur_ch);
                invalidate_window(RECT_ALL);
			}

		case MSG_ERASEBKGND:
			return 0;

	}

	return DefaultContainerProc (hWnd, message, wParam, lParam);
}

static int ViewWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static NEWFILEDLGDATA filedlgdata =
            {FALSE,FALSE,".","*.upf","./","All file(*.*)|upf file(*.upf)",1};

	switch (message) {
		case MSG_CREATE:
			{
				CreateWindow("button", char_menu_texts[0],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[0],
						WIN_WIDTH - 280, GRID_Y, 60, 25, 
						hWnd, 0);
				CreateWindow("button", char_menu_texts[1],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[1],
						WIN_WIDTH - 210, GRID_Y, 60, 25, 
						hWnd, 0);
				CreateWindow("button", char_menu_texts[2],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[2],
						WIN_WIDTH - 140, GRID_Y, 60, 25, 
						hWnd, 0);

				CreateWindow("button", char_menu_texts[3],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[3],
						WIN_WIDTH - 70, GRID_Y, 60, 25, 
						hWnd, 0);

				CreateWindow("button", char_menu_texts[4],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[4],
						WIN_WIDTH - 280, GRID_Y + 35, 60, 25, 
						hWnd, 0);
				CreateWindow("button", char_menu_texts[5],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[5],
						WIN_WIDTH - 210, GRID_Y + 35, 60, 25, 
						hWnd, 0);
				CreateWindow("button", char_menu_texts[6],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[6],
						WIN_WIDTH - 140, GRID_Y + 35, 60, 25, 
						hWnd, 0);

				CreateWindow("button", char_menu_texts[7],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[7],
						WIN_WIDTH - 70, GRID_Y + 35, 60, 25, 
						hWnd, 0);

				CreateWindow("button", char_menu_texts[8],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[8],
						WIN_WIDTH - 280, GRID_Y + 70, 115, 25, 
						hWnd, 0);

				CreateWindow("button", char_menu_texts[9],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[9],
						WIN_WIDTH - 125, GRID_Y + 70, 115, 25,
						hWnd, 0);

				CreateWindow("button", char_menu_texts[10],
						WS_CHILD | WS_VISIBLE,
						char_menu_ids[10],
						WIN_WIDTH - 280, GRID_Y + 105, 70, 25,
						hWnd, 0);

				hscroll_wnd1
					= CreateWindow("scrollwnd", "Char Table 1", 
							WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
							IDC_SCROLLWND1,
							85, 262, 
							TABLE_WIDTH * 18, 
							16 * 18,
							hWnd, 0);

				SendMessage (hscroll_wnd1, SVM_SETCONTAINERPROC, 0, (LPARAM)char_table_container_proc1);
				SendMessage (hscroll_wnd1, SVM_SETCONTRANGE, 
						TABLE_WIDTH * 18 + 8, 
						16 * 18 + 15);
				hscroll_wnd2
					= CreateWindow("scrollwnd", "Char Table 2", 
							WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
							IDC_SCROLLWND2,
							465, 262, 
							TABLE_WIDTH * 18, 
							16 * 18,
							hWnd, 0);

				SendMessage (hscroll_wnd2, SVM_SETCONTAINERPROC, 0, (LPARAM)char_table_container_proc2);
				SendMessage (hscroll_wnd2, SVM_SETCONTRANGE, 
						TABLE_WIDTH * 18 + 8, 
						16 * 18 + 15);

				ShowWindow(hscroll_wnd1, SW_HIDE);
				ShowWindow(hscroll_wnd2, SW_HIDE);

				break;
			}

		case MSG_ACTIVEMENU:
			if (upf_info.upf_tree == NULL)
			{
				EnableMenuItem((HMENU)lParam, IDC_FILE_RENAME, MF_DISABLED);
				EnableMenuItem((HMENU)lParam, IDC_UPDATE, MF_DISABLED);
			}
			break;

		case MSG_COMMAND:
			{
                int id = LOWORD(wParam) ;
				if (id != IDC_FILE_OPEN 
                    && id != IDC_FILE_NEW
                    && upf_info.upf_tree == NULL) return -1;

				switch (id) {
                    case IDC_FILE_NEW:
                        break;
					case IDC_FILE_OPEN:
						if (ShowOpenDialog (hWnd, 0,0,400,300, &filedlgdata) == IDOK) {
							if (upf_info.upf_tree != NULL) {
								free_upf(&upf_info);

								if (mem_dc1 != 0) {
									DeleteMemDC(mem_dc1);
									mem_dc1 = 0;
								}

								if (mem_dc2 != 0) {
									DeleteMemDC(mem_dc2);
									mem_dc2 = 0;
								}

							}

							if (load_upf(&upf_info, filedlgdata.filefullname)
									&& init_glyph (0)) {
								init_display_rects ();


								SendMessage (hscroll_wnd1, SVM_SETCONTRANGE, 
										TABLE_WIDTH * (upf_info.upf_file_header.max_width + 1) + 1,
										( TABLE_HEIGHT ) * (upf_info.upf_file_header.height + 1));
								SendMessage(hscroll_wnd1, SVM_SETSCROLLVAL, 
										TABLE_WIDTH * upf_info.upf_file_header.max_width,
										upf_info.upf_file_header.height / 2);

								SendMessage (hscroll_wnd2, SVM_SETCONTRANGE, 
										TABLE_WIDTH * (upf_info.upf_file_header.max_width + 1) + 1, 
										( TABLE_HEIGHT) * (upf_info.upf_file_header.height + 1));
								SendMessage(hscroll_wnd2, SVM_SETSCROLLVAL, 
										TABLE_WIDTH * upf_info.upf_file_header.max_width,
										upf_info.upf_file_header.height / 2);

								ShowWindow(hscroll_wnd1, SW_SHOWNORMAL);
								ShowWindow(hscroll_wnd2, SW_SHOWNORMAL);


								InvalidateRect(hscroll_wnd1, NULL, FALSE);
								InvalidateRect(hscroll_wnd2, NULL, FALSE);
								invalidate_window(RECT_ALL);
							}
						}
						break;

					case IDC_FILE_SAVE:
                        filedlgdata.IsSave =TRUE;
						if ((upf_info.upf_tree != NULL) && (ShowOpenDialog (hWnd, 0,0,400,300, &filedlgdata) == IDOK)) {
							dump_upf(&upf_info, filedlgdata.filefullname);
							printf("dump_qpf\n");
						}
						break;

					case IDC_CHAR_INC_BB_X:
						g_glyph_bitmap.bbox_x++;
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = FALSE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_DEC_BB_X:
						g_glyph_bitmap.bbox_x--;
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = FALSE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_INC_BB_Y:
						g_glyph_bitmap.bbox_y++;
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = FALSE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_DEC_BB_Y:
						g_glyph_bitmap.bbox_y--;
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = FALSE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_INC_BB_W:
						glyph_change_width_right(&g_glyph_bitmap, 1);
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = TRUE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_DEC_BB_W:
						glyph_change_width_right(&g_glyph_bitmap, -1);
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = TRUE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_INC_BB_H:
						glyph_change_height_bottom(&g_glyph_bitmap, 1);
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = TRUE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_DEC_BB_H:
						glyph_change_height_bottom(&g_glyph_bitmap, -1);
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = TRUE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_DEC_BB_W_FROM_LEFT:
						glyph_change_width_left(&g_glyph_bitmap, -1);
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = TRUE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_DEC_BB_H_FROM_TOP:
						glyph_change_height_top(&g_glyph_bitmap, -1);
						upf_set_char_glyph (&upf_info, cur_ch, &g_glyph_bitmap);
						file_size_change = TRUE;
						invalidate_window(RECT_ALL);
						break;

					case IDC_CHAR_ADD:
                        {
                            int result;

                            result = add_char_mywinentry(hWnd);
                            if (result > 0) {
                                cur_ch = result;
                                if (upf_get_char_glyph_bitmap(&upf_info, cur_ch, 
                                            &g_glyph_bitmap) == FALSE) {
                                    upf_add_char_glyph(&upf_info, cur_ch);
                                    upf_get_char_glyph_bitmap (&upf_info, cur_ch, &g_glyph_bitmap);
                                    file_size_change = TRUE;
                                }else {
                                    MessageBox (hWnd, "The character has already exist", "Error",
                                            MB_OK | MB_ICONINFORMATION);
                                }

                                invalidate_window(RECT_ALL);

                            } else if (result == -3) {
                                MessageBox (hWnd, "Unicode Error", "Error",
                                        MB_OK | MB_ICONINFORMATION);
                            }

                            break;
                        }

					case IDC_CHAR_DEL:
						{
							int result;

							result = add_char_mywinentry(hWnd);
							if (result > 0) {
								cur_ch = result;
								if (upf_get_char_glyph_bitmap(&upf_info, cur_ch, 
                                            &g_glyph_bitmap) == TRUE) {
                                    upf_delete_char_glyph(&upf_info, cur_ch);
                                    if (!upf_info.upf_tree)
                                    {
                                       MessageBox (hWnd, "The font is empty now", "Error",
                                                MB_OK | MB_ICONINFORMATION);
                                    }
									file_size_change = TRUE;
								}else {
									MessageBox (hWnd, "The character is not exist", "Error",
											MB_OK | MB_ICONINFORMATION);
								}

								invalidate_window(RECT_ALL);

							} else if (result == -3) {
								MessageBox (hWnd, "Unicode Error", "Error",
										MB_OK | MB_ICONINFORMATION);
							}

							break;
						}
				  }
			}
			return 0;

		case MSG_LBUTTONDOWN:
			{

				int x = LOWORD (lParam);
				int y = HIWORD (lParam);
				RECT rect_bound;

				if (upf_info.upf_tree == NULL) return -1;

				rect_bound.left = FRAMEOFFSET + g_glyph_bitmap.bbox_x*GRID_SIZE;
				rect_bound.right = rect_bound.left + g_glyph_bitmap.bbox_w*GRID_SIZE;
				rect_bound.top = rect_gridframe.top + (upf_info.upf_file_header.ascent + upf_info.upf_file_header.leading - g_glyph_bitmap.bbox_y)*GRID_SIZE;
				rect_bound.bottom = rect_bound.top + g_glyph_bitmap.bbox_h*GRID_SIZE;

				if (PtInRect (&rect_bound, x, y)) {
					int i, j;
					i = (y - rect_bound.top) / GRID_SIZE;
					j = (x - rect_bound.left) / GRID_SIZE;
					if (j > g_glyph_bitmap.bbox_w)
					{
						Ping ();
						break;
					}
                    if (upf_get_char_glyph_bitmap(&upf_info, cur_ch, 
                                &g_glyph_bitmap) == FALSE) {
                        MessageBox (hWnd, "The character is not exist", "Error",
                                MB_OK | MB_ICONINFORMATION);
                        break;

                    }
					glyph_set_bit(&g_glyph_bitmap, i, j, !glyph_get_bit(&g_glyph_bitmap, i, j));
					invalidate_window(RECT_GRIDFRAME|RECT_CHAR|RECT_CHARTABLE);
				} else if (PtInRect(&rect_table, x, y)) {//click in the char table
					/*
					   cur_ch = (y - rect_table.top)/(qpf_info.height + 1)*TABLE_WIDTH
					   + (x - rect_table.left)/(qpf_info.fm->maxwidth + 1);
					   init_glyph(cur_ch);
					   invalidate_window(RECT_ALL);
					   */
				}
				break;
			}

		case MSG_PAINT:
			{
				HDC hdc = BeginPaint (hWnd);
				draw_editor_window (hWnd,hdc, &rect_gridframe);
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

    //if (!InitMiniGUIExt ())
    //    return 2;
        
    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "UPF Editor";
    CreateInfo.hMenu = create_main_menu();
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ViewWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = WIN_WIDTH;
    CreateInfo.by = WIN_HEIGHT;
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

    //MiniGUIExtCleanUp ();
        
    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

