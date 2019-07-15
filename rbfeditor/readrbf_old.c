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
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
** TODO:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "rbf.h"

/********************** Load/Unload of raw bitmap font ***********************/
BOOL loadRBF (RBFINFO* rbf, const char* file)
{
    rbf->len_bits = ((rbf->width + 7) >> 3) * rbf->height * rbf->nr_chars;

    if ((rbf->fd = open (file, O_RDWR)) < 0)
        return FALSE;

    rbf->bits = mmap (NULL, rbf->len_bits, PROT_READ | PROT_WRITE, MAP_SHARED, rbf->fd, 0);

    if (rbf->bits == MAP_FAILED) {
        close (rbf->fd);
        rbf->bits = NULL;
    }

    printf ("RBF loaded: %s, %p, %d\n", rbf->name, rbf->bits, rbf->len_bits);

    if (rbf->bits == NULL)
        return FALSE;
    return TRUE;
}

void freeRBF (RBFINFO* rbf)
{
    if (rbf->log_font) {
        DestroyLogFont (rbf->log_font);
    }

    if (rbf->bits) {
        munmap (rbf->bits, rbf->len_bits);
        close (rbf->fd);
    }

    memset (rbf, 0, sizeof (RBFINFO));
}

#define IDL_RBF     100

static DLGTEMPLATE DlgSelectRBF =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    100, 100, 324, 225,
    "Select RBF to edit",
    0, 0,
    4, NULL,
    0
};

static CTRLDATA CtrlSelectRBF [] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 130, 15,
        IDC_STATIC,
        "RBFs:",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY,
        10, 30, 300, 120,
        IDL_RBF,
        "",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        10, 170, 140, 25,
        IDOK, 
        "Ok",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        170, 170, 140, 25,
        IDCANCEL,
        "Cancel",
        0
    },
};

static void fill_rbf_box (HWND hDlg)
{
    const DEVFONT* dev_font = NULL;

    while ((dev_font = GetNextDevFont (dev_font))) {
        if (strncmp (dev_font->name, FONT_TYPE_NAME_BITMAP_RAW, 3) == 0)
            SendDlgItemMessage (hDlg, IDL_RBF, LB_ADDSTRING, 0, (LPARAM)dev_font->name);
    }
}

static void rbf_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    if (nc == LBN_DBLCLK || nc == LBN_ENTER) {
        SendNotifyMessage (GetParent (hwnd), MSG_COMMAND, IDOK, 0L);
    }
}

static int SelectRBFBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
    {
        SetNotificationCallback (GetDlgItem (hDlg, IDL_RBF), rbf_notif_proc);
        fill_rbf_box (hDlg);
        SetWindowAdditionalData (hDlg, lParam);
        return 1;
    }
        
    case MSG_COMMAND:
        switch (wParam) {
        case IDOK:
        {
            int cur_sel = SendDlgItemMessage (hDlg, IDL_RBF, LB_GETCURSEL, 0, 0L);
            if (cur_sel >= 0) {
                RBFINFO* rbf = (RBFINFO*) GetWindowAdditionalData (hDlg);
                SendDlgItemMessage (hDlg, IDL_RBF, LB_GETTEXT, cur_sel, (LPARAM)rbf->name);
                EndDialog (hDlg, IDOK);
            }
            break;
        }

        case IDCANCEL:
            EndDialog (hDlg, IDCANCEL);
            break;
        }
        break;
        
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

BOOL selectRBF (HWND hwnd, RBFINFO* rbf)
{
    DlgSelectRBF.controls = CtrlSelectRBF;
    
    if (DialogBoxIndirectParam (&DlgSelectRBF, hwnd, SelectRBFBoxProc, (DWORD)rbf) == IDOK) {
        rbf->log_font = CreateLogFontByName (rbf->name);
        if (rbf->log_font) {
            DEVFONT* dev_font = (rbf->log_font->mbc_devfont)?
                        rbf->log_font->mbc_devfont:rbf->log_font->sbc_devfont;

            rbf->nr_chars = dev_font->charset_ops->nr_chars;
            rbf->width = dev_font->font_ops->get_char_width (rbf->log_font, dev_font, NULL, 0);
            rbf->height = dev_font->font_ops->get_font_height (rbf->log_font, dev_font);
            rbf->descent = dev_font->font_ops->get_font_descent (rbf->log_font, dev_font);
            rbf->dev_font = dev_font;

            printf ("The RBF to edit: %s, nr_chars: %d, width: %d, height: %d, descent: %d\n", 
                    rbf->name, rbf->nr_chars, rbf->width, rbf->height, rbf->descent);
            return TRUE;
        }

        MessageBox (hwnd, "Can not create logical font from this device font.", 
                    "Error!", 
                    MB_OK|MB_ICONEXCLAMATION);
    }

    return FALSE;
}

#define IDE_CHAR    100

static DLGTEMPLATE DlgSelectChar =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    100, 100, 324, 125,
    "Select a character to edit",
    0, 0,
    4, NULL,
    0
};

static CTRLDATA CtrlSelectChar [] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 300, 15,
        IDC_STATIC,
        "Please input the econding of the char in hex:",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_BORDER | ES_UPPERCASE | WS_TABSTOP,
        10, 30, 300, 25,
        IDE_CHAR,
        "",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        10, 70, 140, 25,
        IDOK, 
        "Ok",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        170, 70, 140, 25,
        IDCANCEL,
        "Cancel",
        0
    },
};

static WNDPROC old_edit_proc;

static int RestrictedEditBox (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    if (message == MSG_CHAR) {
        if ((wParam < '0' || wParam > '9') &&
                !((wParam >= 'A' && wParam <= 'F') || (wParam >= 'a' && wParam <= 'f')))
            return 0;
    }

    return (*old_edit_proc) (hwnd, message, wParam, lParam);
}

static int SelectCharBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
    {
        SetWindowAdditionalData (hDlg, lParam);
        old_edit_proc = SetWindowCallbackProc (GetDlgItem (hDlg, IDE_CHAR), RestrictedEditBox);
        return 1;
    }
        
    case MSG_COMMAND:
        switch (wParam) {
        case IDOK:
        {
            char buffer [33];
            int len;

            len = GetDlgItemText (hDlg, IDE_CHAR, buffer, 32);
            if (len > 0 && (len % 2 == 0)) {
                int i;
                char one_ch [3] = {0};
                unsigned char enconding [16];
                RBFINFO* rbf;
                int valid;

                for (i = 0; i < len/2; i++) {
                    one_ch [0] = buffer [i*2];
                    one_ch [1] = buffer [i*2 + 1];
                    
                    enconding [i] = (unsigned char) strtol (one_ch, NULL, 16);
                }

                rbf = (RBFINFO*) GetWindowAdditionalData (hDlg);
                valid = rbf->dev_font->charset_ops->len_first_char (enconding, len/2);
                if (valid) {
                    rbf->char_off = rbf->dev_font->charset_ops->char_offset (enconding);
                    EndDialog (hDlg, IDOK);
                    break;
                }

                MessageBox (hDlg, "Not a valid enconding for this device font.", 
                    "Error",
                    MB_OK | MB_ICONEXCLAMATION);
            }
            break;
        }

        case IDCANCEL:
            EndDialog (hDlg, IDCANCEL);
            break;
        }
        break;
        
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

BOOL selectChar (HWND hwnd, const RBFINFO* rbf, unsigned int* ch)
{
    DlgSelectChar.controls = CtrlSelectChar;
    
    if (DialogBoxIndirectParam (&DlgSelectChar, hwnd, SelectCharBoxProc, (DWORD)rbf) == IDOK) {
        *ch = rbf->char_off;
        return TRUE;
    }

    return FALSE;
}

