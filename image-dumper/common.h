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
#ifndef _PAGEFUN_H
#define _PAGEFUN_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>


extern DLGTEMPLATE dlg_propsheet1;
extern DLGTEMPLATE dlg_propsheet2;

extern CTRLDATA propsheet1_data[];
extern CTRLDATA propsheet2_data[];

int PropsheetWinProc1 (HWND hDlg, int message, WPARAM wParam, LPARAM lParam);

int PropsheetWinProc2 (HWND hDlg, int message, WPARAM wParam, LPARAM lParam);

#if 0
extern char _path_dst_select [MAX_PATH + 1];
extern int  _sel_count;
extern char *_sel_files[MAX_NAME + 1];
#endif

int image_to_c (int nr, char **src_file, 
        char * src_parent_path, char * dst_file_full_path,
        char * target_name);


#endif
