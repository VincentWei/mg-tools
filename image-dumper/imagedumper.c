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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

#include "dumpbmp.h"

#define IDC_PROPSHEET  200

static int MainWinProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd_prop;
    switch (message)
    {
        case MSG_CREATE:
            {
                hwnd_prop =
                CreateWindowEx (CTRL_PROPSHEET, 
                        "",
                        WS_VISIBLE | PSS_COMPACTTAB,
                        0,
                        IDC_PROPSHEET,
                        0, 0, g_rcScr.right, g_rcScr.bottom,
                        hwnd, 0);

                if (HWND_INVALID == hwnd_prop)
                    return -1;

                dlg_propsheet1.controls = propsheet1_data;
                dlg_propsheet2.controls = propsheet2_data;

                SendMessage (hwnd_prop, PSM_ADDPAGE, 
                        (WPARAM)&dlg_propsheet1, (LPARAM)PropsheetWinProc1);

                SendMessage (hwnd_prop, PSM_ADDPAGE, 
                        (WPARAM)&dlg_propsheet2, (LPARAM)PropsheetWinProc2);
            }
            break;

        case MSG_PAINT:
            {
                printf ("main window receive MSG_PAINT\n");
            
            }
            break;

        case MSG_CLOSE:
            {
                DestroyMainWindow (hwnd);
                PostQuitMessage (hwnd);
            }
            return 0;
    }

    return DefaultMainWinProc(hwnd, message, wParam, lParam);
}

extern BOOL RLE_ENCODE;

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;
    int ibegin = 1;
    BOOL bI = FALSE, bL = FALSE;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "image-dumper" , 0 , 0);
#endif

    if (argc > 1) {
        printf ("%s\n", argv[0]);
        if (!argv[ibegin] || !argv[ibegin + 1])
            return -1;
        if (strcmp(argv[ibegin], "-i") == 0 || strcmp(argv[ibegin], "-l") == 0) {
            ibegin ++;
            bI = TRUE;
        }

        if (strcmp(argv[ibegin], "-i") == 0 || strcmp(argv[ibegin], "-l") == 0) {
            ibegin ++;
            bL = TRUE;
        }

        if (strcmp(argv[ibegin], "-r") == 0) {
            ibegin ++;
            RLE_ENCODE = TRUE;
        }

        if (bI)
            make_file_begin();
        if (bL)
            make_loader_begin();

        if (argc >= ibegin+1) {
            int i;
            for (i = ibegin; i < argc; i++) 
                dump_from_file (HDC_SCREEN, argv[i]);

            if (ibegin == 1)
                dump_hash_table ("fhas");

            if (bI)
                make_file_end();
            if (bL)
                make_loader_end();

            printf ("============== dump bitmap over! ================\n");
            return 0;
        }
    }
    CreateInfo.dwStyle = 
        WS_VISIBLE | WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "image-dumper";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = MainWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif

