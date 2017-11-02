/*
** $Id: readrbf.c 219 2008-08-25 08:43:42Z weiym $
** 
** readrbf.c: Read RFB font from file.
**
** Copyright (C) 2004, Feynman Software
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
#include <limits.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgutils/mgutils.h>

#include "charset.h"
#include "rbf.h"

#define VAL_DEBUG
#define FUN_DEBUG
#define INFO_DEBUG
#include <my_debug.h>

#define IDC_CHARSET 100
#define IDC_WIDTH   110
#define IDC_HEIGHT  120

/********************** Load/Unload of raw bitmap font ***********************/
extern CHARSETOPS* charsets[];
extern int nr_charsets;

static int get_descent (int height)
{
    if (height >= 40)
        return 6;
    else if (height >= 20)
        return 3;
    else if (height >= 15)
        return 2;
    else if (height >= 10)
        return 1;

    return 0;
}

void free_rbf (RBFINFO* rbf_info)
{
    free (rbf_info->font);
    rbf_info->filefullname[0] = '\0';
    TEST_INFO("empty fillname in free_rbf");
    rbf_info->font = NULL;
}


BOOL write_rbf (RBFINFO* rbf_info)
{
    FILE* fp;
    assert (rbf_info->font && rbf_info->filefullname [0]);

    if (!(fp = fopen (rbf_info->filefullname, "wb"))) {
        return FALSE;
    }

    if (fwrite (rbf_info->font, sizeof(char), rbf_info->font_size, fp) 
            < rbf_info->font_size) {
        fclose(fp);
        return FALSE;
    }

    fclose(fp);
    rbf_info->is_saved = TRUE;
    return TRUE;
}

static DLGTEMPLATE dlg_select_font_info =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    100, 100, 300, 200,
    "create font",
    0, 0,
    5, NULL,
    0
};

static CTRLDATA ctrl_select_font_info [] =
{ 
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_CHILD | LBS_SORT | WS_VSCROLL |WS_HSCROLL, 
        10, 10, 180, 120,
        IDC_CHARSET,
        "charset",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
        200, 30, 30, 25,
        IDC_WIDTH,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
        200, 90, 30, 25,
        IDC_HEIGHT,
        "",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        10, 135, 50, 25,
        IDOK, 
        "Ok",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        80, 135, 50, 25,
        IDCANCEL,
        "Cancel",
        0
    },
};

static WNDPROC old_sledit_proc;
static int decnum_sledit_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    if (message == MSG_CHAR) {
        /*127 is char esc*/
        if ((wParam < '0' || wParam > '9') && wParam != 127) 
            return 0;
    }

    return (*old_sledit_proc) (hWnd, message, wParam, lParam);
}


static int select_font_info_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static HWND hWnd_charset;
    static HWND hWnd_width;
    static HWND hWnd_height;
    int id;
    int code;
    RBFINFO* rbf_info;
    char buf[LEN_FONT_NAME+1];

    int cursel;
    int i;

    switch (message)
    {
        case MSG_INITDIALOG:
            hWnd_charset = GetDlgItem (hWnd, IDC_CHARSET);
            hWnd_width = GetDlgItem (hWnd, IDC_WIDTH);
            hWnd_height = GetDlgItem (hWnd, IDC_HEIGHT);

            SetWindowAdditionalData (hWnd, lParam);

            old_sledit_proc = SetWindowCallbackProc (hWnd_width,
                    decnum_sledit_proc);
            SetWindowCallbackProc (hWnd_height, 
                    decnum_sledit_proc);

            for (i=0; i<nr_charsets; i++)
                SendMessage (hWnd_charset, 
                        LB_ADDSTRING, 0, (LPARAM)(charsets[i]->name));
            SendMessage (hWnd_charset, LB_SETCURSEL, 0, 0);

            SendMessage (hWnd_width, MSG_SETTEXT, 0, (LPARAM)"16");
            SendMessage (hWnd_height, MSG_SETTEXT, 0, (LPARAM)"16");
            break;

        case MSG_COMMAND:
            id = LOWORD(wParam);
            code = HIWORD(wParam);

            switch (id)
            {
                case IDOK:
                    rbf_info = (RBFINFO*)GetWindowAdditionalData (hWnd);
                    /*refresh filename static*/
                    buf[LEN_FONT_NAME] = '\0';
                    SendMessage (hWnd_width, MSG_GETTEXT, LEN_FONT_NAME, (LPARAM)buf);
                    rbf_info->width = atoi (buf);
                    SendMessage (hWnd_height, MSG_GETTEXT, LEN_FONT_NAME, (LPARAM)buf);
                    rbf_info->height = atoi (buf);
                    rbf_info->descent = get_descent(rbf_info->height);

                    cursel = SendMessage (hWnd_charset, LB_GETCURSEL, 0, 0);
                    SendMessage (hWnd_charset, LB_GETTEXT, cursel, (LPARAM)buf);
                    rbf_info->charset_ops = get_charset_ops_ex (buf);
                    rbf_info->font_size = ((rbf_info->width + 7) >> 3) * 
                        rbf_info->height * rbf_info->charset_ops->nr_chars;

                    EndDialog (hWnd, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog (hWnd, IDCANCEL);
                    break;
            }
            break;
    }

    return DefaultDialogProc (hWnd, message, wParam, lParam);
}

static BOOL select_font_info (HWND hWnd, RBFINFO* rbf)
{
    dlg_select_font_info.controls = ctrl_select_font_info;
    rbf->filefullname[0] = '\0';
    TEST_INFO("empty filefullname in select_font_info");
    
    if (DialogBoxIndirectParam (&dlg_select_font_info, hWnd, 
                select_font_info_proc, (DWORD)rbf) == IDOK) {
        return TRUE;
    }

    return FALSE;
}

BOOL create_font (HWND hWnd, RBFINFO* rbf)
{
    if (select_font_info (hWnd, rbf))
    {
        rbf->font = (unsigned char *)malloc(rbf->font_size);
        if (rbf->font)
            return TRUE;
        else 
            return FALSE;
    }
    return FALSE;
}

BOOL open_rbf (HWND hWnd, RBFINFO* rbf_info)
{
    NEWFILEDLGDATA filedlgdata = {FALSE, FALSE, ".", "*.bin", "./",
        "All file(*.*)|rbf file(*.bin)",1};
    FILE* fp;
    int file_size;

    if (ShowOpenDialog (hWnd, 0,0,400,300, &filedlgdata) != IDOK ||
            !select_font_info (hWnd, rbf_info)) {
        return FALSE;
    }

    strncpy (rbf_info->filefullname, filedlgdata.filefullname, 
            MY_NAMEMAX + MY_PATHMAX - 1);
    rbf_info->filefullname[MY_NAMEMAX + MY_PATHMAX - 1] = '\0';

    strncpy (rbf_info->filename, filedlgdata.filename, MY_NAMEMAX-1);
    rbf_info->filename[MY_NAMEMAX-1] = '\0';

    TEST_VAL(rbf_info->filefullname, %s);
    TEST_VAL(rbf_info->filename, %s);

    TEST_INFO("write filefullname in open_rbf");

    // Open font file and check font size
    if (!(fp = fopen (rbf_info->filefullname, "rb"))) {
        return FALSE;
    }

    fseek (fp, 0, SEEK_END);
    file_size = ftell (fp);

    if (file_size != rbf_info->font_size)
    {
        fprintf (stderr, "Error: file size is no equal to font size");
        fclose (fp);
        return FALSE;
    }

    fseek (fp, 0, SEEK_SET);

    /*read font*/
    rbf_info->font = (unsigned char *)malloc(rbf_info->font_size);
    if (!rbf_info->font)
        return FALSE;

    if (fread (rbf_info->font, sizeof(char), rbf_info->font_size, fp) <rbf_info->font_size) {
        free_rbf (rbf_info);
        fclose(fp);
        return FALSE;
    }

    fclose(fp);
    return TRUE;
}

BOOL save_rbf (HWND hWnd, RBFINFO* rbf_info)
{
    NEWFILEDLGDATA filedlgdata = {FALSE, FALSE};

    TEST_VAL(rbf_info->filefullname, %s);
    FUN_STEPS(1);
    if (!rbf_info->filefullname[0])
    {
        FUN_STEPS(1);
        filedlgdata.IsSave = TRUE;
        strncpy(filedlgdata.filename, rbf_info->filename, MY_NAMEMAX);
        filedlgdata.filename[MY_NAMEMAX] = '\0';

        strncpy(filedlgdata.filepath, ".", MY_NAMEMAX);
        filedlgdata.filename[MY_NAMEMAX] = '\0';
        FUN_STEPS(1);

        if (ShowOpenDialog (hWnd, 0,0,400,300, &filedlgdata) 
                == IDOK) {
            FUN_STEPS(1);
            strncpy (rbf_info->filefullname, filedlgdata.filefullname, 
                    MY_PATHMAX+MY_NAMEMAX-1);
            filedlgdata.filefullname[MY_PATHMAX+MY_NAMEMAX-1] = '\0';

        }
        else {
            FUN_STEPS(1);
            return FALSE;
        }
    }
    FUN_STEPS(1);
    write_rbf (rbf_info);
    return TRUE;
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
        "input the econding of the char in hex:",
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


static int hexnum_sledit_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    if (message == MSG_CHAR) {
        /*127 is char esc*/
        if ((!isxdigit (wParam)) && wParam != 127)
            return 0;
    }

    return (*old_sledit_proc) (hWnd, message, wParam, lParam);
}

static int SelectCharBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
    {
        SetWindowAdditionalData (hDlg, lParam);
        old_sledit_proc = SetWindowCallbackProc (GetDlgItem (hDlg, IDE_CHAR), hexnum_sledit_proc);
        return 1;
    }
        
    case MSG_COMMAND:
        switch (wParam) {
        case IDOK:
            {
                char buffer [33];
                int len;
                FUN_STEPS (1);

                len = GetDlgItemText (hDlg, IDE_CHAR, buffer, 32);
                if (len > 0 && (len % 2 == 0)) {
                    int i;
                    char one_ch [3] = {0};
                    unsigned char enconding [16];
                    RBFINFO* rbf;
                    int valid;
                    FUN_STEPS (1);

                    for (i = 0; i < len/2; i++) {
                        one_ch [0] = buffer [i*2];
                        one_ch [1] = buffer [i*2 + 1];

                        enconding [i] = (unsigned char) strtol (one_ch, NULL, 16);
                    }
                    FUN_STEPS (1);

                    rbf = (RBFINFO*) GetWindowAdditionalData (hDlg);
                    FUN_STEPS (1);
                    valid = rbf->charset_ops->len_first_char (enconding, len/2);
                    FUN_STEPS (1);
                    if (valid) {
                        rbf->char_off = rbf->charset_ops->char_glyph_value (NULL, 0,
                                enconding, 0);
                        FUN_STEPS (1);
                        EndDialog (hDlg, IDOK);
                        break;
                    }

                    MessageBox (hDlg, "Not a valid enconding for this charset.", 
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

BOOL selectChar (HWND hWnd, const RBFINFO* rbf, unsigned int* ch)
{
    DlgSelectChar.controls = CtrlSelectChar;
    
    if (DialogBoxIndirectParam (&DlgSelectChar, hWnd, 
                SelectCharBoxProc, (DWORD)rbf) == IDOK) {
        *ch = rbf->char_off;
        return TRUE;
    }

    return FALSE;
}

