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
