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

#define IDC_DIR_SRC  100
#define IDC_DIR_DST  110
#define IDC_FILELIST 120
#define IDC_PATH_SRC 130
#define IDC_PATH_DST 140
#define IDC_FILENAME 150

static char _path_dst_select [MAX_PATH + 1];
static char _file_dst_select [MAX_PATH + 1];
static char _target_name [MAX_NAME + 1];
static int  _sel_count;
static char **_sel_files;

static char _cwd_src [MAX_PATH + 1];
static char _cwd_dst [MAX_PATH + 1];

DLGTEMPLATE dlg_propsheet1 =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    100, 100, 304, 225,
    "Convert To Binary",
    0, 0,
    12, NULL,
    0
};

CTRLDATA propsheet1_data[] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 250, 18,
        IDC_STATIC,
        "Select source image files :",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY,
        10, 40, 130, 100,
        IDC_DIR_SRC,
        "",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT 
            | LBS_AUTOCHECKBOX | LBS_MULTIPLESEL,
        150, 40, 130, 100,
        IDC_FILELIST,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 145, 150, 18,
        //300, 100, 250, 18, 
        IDC_STATIC,
        //"The file to be converted: ",
        "Source file Path: ",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 160, 640, 18,
        //300, 120, 200, 60, 
        IDC_PATH_SRC,
        NULL,
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 190, 200, 18,
        IDC_STATIC,
        "Select target directory :",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY,
        10, 220, 130, 100,
        IDC_DIR_DST,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        200, 230, 130, 18,
        IDC_STATIC,
        "Target name :",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_CENTER,
        200, 250, 130, 25,
        IDC_FILENAME, 
        NULL,
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 330, 150, 18,
        //300, 120, 200, 60, 
        IDC_STATIC,
        "Target file:",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 350, 600, 18,
        //300, 120, 200, 60, 
        IDC_PATH_DST,
        NULL,
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        200, 370, 130, 25,
        IDOK, 
        "Convert",
        0
    },
};

static int fill_boxes_src (HWND hDlg, const char* path)
{
#ifdef __NOUNIX__
    LISTBOXITEMINFO lbii;

    lbii.string = "file.1";
    lbii.cmFlag = CMFLAG_BLANK;
    lbii.hIcon = 0;
    SendDlgItemMessage (hDlg, IDC_FILELIST, LB_ADDSTRING, 0, (LPARAM)&lbii);

    lbii.string = "file.2";
    SendDlgItemMessage (hDlg, IDC_FILELIST, LB_ADDSTRING, 0, (LPARAM)&lbii);

    lbii.string = "file.3";
    SendDlgItemMessage (hDlg, IDC_FILELIST, LB_ADDSTRING, 0, (LPARAM)&lbii);
#else
    struct dirent* dir_ent;
    DIR*   dir;
    struct stat ftype;
    char   fullpath [PATH_MAX + 1];
    
    if ((dir = opendir (path)) == NULL)
    {
        printf ("ERROR: fail to open dir %s\n", path);
        return -1;
    }

    SendDlgItemMessage (hDlg, IDC_DIR_SRC, LB_RESETCONTENT, 0, (LPARAM)0);
    SendDlgItemMessage (hDlg, IDC_FILELIST, LB_RESETCONTENT, 0, (LPARAM)0);

    while ( (dir_ent = readdir ( dir )) != NULL ) 
    {
        /* Assemble full path name. */
        strncpy (fullpath, path, PATH_MAX);
        strcat (fullpath, "/");
        strcat (fullpath, dir_ent->d_name);
        
        if (stat (fullpath, &ftype) < 0 ) 
        {
           continue;
        }

        if (S_ISDIR (ftype.st_mode))
        {
            SendDlgItemMessage (hDlg, IDC_DIR_SRC, 
                    LB_ADDSTRING, 0, (LPARAM)dir_ent->d_name);
        }
        else if (S_ISREG (ftype.st_mode)) 
        {
            LISTBOXITEMINFO lbii;

            lbii.string = dir_ent->d_name;
            lbii.cmFlag = CMFLAG_BLANK;
            lbii.hIcon = 0;
            SendDlgItemMessage (hDlg, IDC_FILELIST, LB_ADDSTRING, 0, (LPARAM)&lbii);
        }
    }

    closedir (dir);
    return 0;
#endif
}

static int fill_boxes_dst (HWND hDlg, const char* path)
{
#ifndef __NOUNIX__
    struct dirent* dir_ent;
    DIR*   dir;
    struct stat ftype;
    char   fullpath [PATH_MAX + 1];

    if ((dir = opendir (path)) == NULL)
    {
        printf ("ERROR: fail to open dir %s\n", path);
        return -1;
    }

    SendDlgItemMessage (hDlg, IDC_DIR_DST, LB_RESETCONTENT, 0, (LPARAM)0);

    while ( (dir_ent = readdir ( dir )) != NULL ) 
    {
        /* Assemble full path name. */
        strncpy (fullpath, path, PATH_MAX);
        strcat (fullpath, "/");
        strcat (fullpath, dir_ent->d_name);
        
        if (stat (fullpath, &ftype) < 0 ) 
        {
           continue;
        }

        if (S_ISDIR (ftype.st_mode))
        {
            SendDlgItemMessage (hDlg, IDC_DIR_DST, 
                    LB_ADDSTRING, 0, (LPARAM)dir_ent->d_name);
        }
    }

    closedir (dir);
    return 0;
#endif
}

static void show_target_file (HWND hwnd)
{
    int i;
    /** get file name */
    GetWindowText (GetDlgItem (GetParent (hwnd), IDC_FILENAME), 
            _target_name, MAX_NAME);

    if (_target_name[0] == '\0')
        strcpy (_target_name, "sample.c");
    _target_name[MAX_NAME] = '\0';

    i = strlen (_target_name);

    if (_target_name[i - 1] == 'c' && 
        _target_name[i - 2] == '.')
        _target_name[i - 2] = '\0';

    strcpy (_file_dst_select, _path_dst_select);
    i = strlen (_path_dst_select);
    _file_dst_select[i + 1] = '\0';

    if (_file_dst_select [i - 1] != '/')
    {
        _file_dst_select[i] = '/';
        _file_dst_select[i + 1] = '\0';
    }
    strcat (_file_dst_select, _target_name);
    strcat (_file_dst_select, ".c");

    SetWindowText (GetDlgItem (GetParent (hwnd), IDC_PATH_DST), 
            _file_dst_select);
}

static void show_source_file (HWND hwnd)
{
    SetWindowText (hwnd, _cwd_src);
}

static void dir_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    if (nc == LBN_DBLCLK || nc == LBN_ENTER) 
    {
        //printf ("DIR receive code %i\n", nc);
        int cur_sel = SendMessage (hwnd, LB_GETCURSEL, 0, 0L);
        if (cur_sel < 0)
            return;

        char cwd [MAX_PATH + 1];
        char dir [MAX_NAME + 1];
        //GetWindowText (GetDlgItem (GetParent (hwnd), IDC_PATH_SRC), cwd, MAX_PATH);

        if (id == IDC_DIR_SRC)
        {
            strcpy (cwd, _cwd_src);
        }
        else if (id == IDC_DIR_DST)
        {
            strcpy (cwd, _cwd_dst);
        }
        cwd [MAX_PATH] = '\0';

        SendMessage (hwnd, LB_GETTEXT, cur_sel, (LPARAM)dir);

        if (strcmp (dir, ".") == 0)
            return;

        if (strcmp (dir, "..") == 0) 
        {
            char* slash;

            if (strcmp (cwd, "/") == 0)
                return;

            slash = strrchr (cwd, '/');
            if (slash == NULL)
                return;
            if (slash == cwd)
                strcpy (cwd, "/");
            else
                *slash = '\0';
        }
        else 
        {
            if (strcmp (cwd, "/") != 0)
                strcat (cwd, "/");
            strcat (cwd, dir);
        }

        if (id == IDC_DIR_SRC)
        {
            if (0 == fill_boxes_src (GetParent (hwnd), cwd))
            {
                /** set cwd */
                strcpy (_cwd_src, cwd);
                _cwd_src[MAX_PATH] = '\0';

                show_source_file (GetDlgItem 
                        (GetParent (hwnd), IDC_PATH_SRC));
            }
                    
        }
        else if (id == IDC_DIR_DST)
        {
            if (0 == fill_boxes_dst (GetParent (hwnd), cwd))
            {
                strcpy (_cwd_dst, cwd);
                _cwd_dst[MAX_PATH] = '\0';

                strcpy (_path_dst_select, _cwd_dst);

                show_target_file (GetDlgItem 
                        (GetParent (hwnd), IDC_PATH_DST));
            }
        }
    }
    else if (nc == LBN_SELCHANGE && id == IDC_DIR_DST)
    {
        int cur_sel = SendMessage (hwnd, LB_GETCURSEL, 0, 0L);
        if (cur_sel < 0) 
            return;

        char dir [MAX_NAME + 1];
        if (LB_OKAY != SendMessage (hwnd, LB_GETTEXT, 
                    cur_sel, (LPARAM)dir))
            return;

        strcpy (_path_dst_select, _cwd_dst);
        _path_dst_select[MAX_PATH] = '\0';

        if (strcmp (dir, ".") == 0)
            return;

        if (strcmp (dir, "..") == 0) 
        {
            char* slash;

            if (strcmp (_path_dst_select, "/") == 0)
                return;

            slash = strrchr (_path_dst_select, '/');
            if (slash == NULL)
                return;
            if (slash == _path_dst_select)
                strcpy (_path_dst_select, "/");
            else
                *slash = '\0';
        }
        else 
        {
            if (strcmp (_path_dst_select, "/") != 0)
                strcat (_path_dst_select, "/");
            strcat (_path_dst_select, dir);
        }

        show_target_file (GetDlgItem (GetParent (hwnd), IDC_FILENAME));
    }
}

static void target_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    //printf ("target_notif_proc message = 0x%i\n", id);
    if (nc == EN_CHANGE)
    {
        //printf ("show_target_file\n");
        show_target_file (hwnd);
    }
}

static void file_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
#if 0
    int * sel_item_id;
    printf ("file_notif_proc message %i\n", nc);
    if (nc == LBN_CLICKCHECKMARK)
    {
        printf ("LBN_CLICKCHECKMARK \n");
        _sel_count = SendMessage (hwnd, LB_GETSELCOUNT, 0, 0);
        printf ("select number : %i\n", _sel_count);

    }
#endif
#if 0
    LB_GETSELCOUNT
    LB_GETSELITEMS
    LB_GETTEXT
#endif
}

static void convert_over (void)
{
    printf ("convert_over begin\n");
    int i;
    for (i = 0; i < _sel_count; ++i)
    {
        free (_sel_files[i]);
        _sel_files[i] = NULL;
    }
    free (_sel_files);
    printf ("convert_over end\n");
}

static void convert_to_file (HWND hDlg)
{
    int i;
    char info [1024] = "All files to be converted is: \n";
	char file [MAX_NAME + 1];
    int *index;

    _sel_count = SendDlgItemMessage (hDlg, IDC_FILELIST, LB_GETSELCOUNT, 0, 0L);
    printf ("-----------  _sel_count = %i\n", _sel_count);

    if (_sel_count > 0)
    {
        //_sel_files = (char **)calloc (_sel_count, sizeof (char*));
        _sel_files = (char **)malloc (_sel_count * sizeof (char*));
        
        if (NULL == _sel_files)
        {
            printf ("ERROR: not enough memory!\n");
            return;
        }
    }

    index = malloc (_sel_count * sizeof (int));
    SendDlgItemMessage (hDlg,
        IDC_FILELIST, LB_GETSELITEMS, (WPARAM)_sel_count, (LPARAM)index);

    for (i = 0; i < _sel_count; ++i)
    {
        if (LB_OKAY == SendDlgItemMessage (hDlg, IDC_FILELIST,
                LB_GETTEXT, (WPARAM)index[i], (LPARAM)file))
        {
            file [MAX_NAME] = '\0';

            //_sel_files[i] = (char *)calloc (MAX_NAME + 1, sizeof(char));
            _sel_files[i] = (char *)malloc ((MAX_NAME + 1) * sizeof(char));
            if (NULL == _sel_files)
            {
                printf ("ERROR: not enough memory!\n");
                convert_over ();
                return;
            }
            memset (_sel_files[i], 0, (MAX_NAME + 1) * sizeof (char));

            strcpy (_sel_files[i], file);
            _sel_files[MAX_NAME] = '\0';

            strcat (info, file);
            strcat (info, "\n");
        }
    }

#if 0
    for (i = 0; i < _sel_count; ++i) 
    {
        if (CMFLAG_CHECKED == SendDlgItemMessage (hDlg,
                IDC_FILELIST, LB_GETCHECKMARK, i, 0))
        {
            if (LB_OKAY == SendDlgItemMessage (hDlg, IDC_FILELIST,
                    LB_GETTEXT, i, (LPARAM)file))
            {
                file [MAX_NAME] = '\0';

                //_sel_files[i] = (char *)calloc (MAX_NAME + 1, sizeof(char));
                _sel_files[i] = (char *)malloc ((MAX_NAME + 1) * sizeof(char));
                if (NULL == _sel_files)
                {
                    printf ("ERROR: not enough memory!\n");
                    return;
                }
                memset (_sel_files[i], 0, (MAX_NAME + 1) * sizeof (char));

                strcpy (_sel_files[i], file);
                _sel_files[MAX_NAME] = '\0';

                printf ("find file [%i] = [%s]\n",
                        i, file);
                strcat (info, file);
                strcat (info, "\n");
                printf ("----find file [%i] = [%s]\n",
                        i, file);
            }
	    }
    }
#endif

    if (IDOK == MessageBox (hDlg, info, "", 
                MB_OKCANCEL | MB_ICONINFORMATION))
    {
        //printf ("Now convert files...\n"); 
        image_to_c (_sel_count, _sel_files, _cwd_src, 
                _file_dst_select, _target_name);
    }
    else
    {
        //printf ("Cancel convert.\n");
    }
    convert_over ();
}

int PropsheetWinProc1 (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
        case MSG_INITPAGE:
        {
            SetNotificationCallback 
                (GetDlgItem (hDlg, IDC_DIR_SRC), dir_notif_proc);

            SetNotificationCallback 
                (GetDlgItem (hDlg, IDC_DIR_DST), dir_notif_proc);

            SetNotificationCallback 
                (GetDlgItem (hDlg, IDC_FILELIST), file_notif_proc);

            SetNotificationCallback 
                (GetDlgItem (hDlg, IDC_FILENAME), target_notif_proc);

            char * cwd;
            cwd = getcwd (_cwd_src, MAX_PATH);
            if (NULL == cwd)
            {
                printf ("ERROR: fail to get current word directory.\n");
                return -1;
            }

            if (0 == fill_boxes_src (hDlg, cwd))
            {
                /** set cwd */
                strcpy (_cwd_src, cwd);
                _cwd_src[MAX_PATH] = '\0';

                show_source_file (GetDlgItem 
                        (hDlg, IDC_PATH_SRC));
            }
            else
                return -1;

            if (0 == fill_boxes_dst (hDlg, cwd))
            {
                strcpy (_cwd_dst, cwd);
                _cwd_dst[MAX_PATH] = '\0';

                strcpy (_path_dst_select, _cwd_dst);

                show_target_file (GetDlgItem (hDlg, IDC_PATH_DST));
            }
            else
                return -1;
        }
        return 1;
            
        case MSG_SHOWPAGE:
        {
            //printf ("PropsheetWinProc1 receive MSG_SHOWPAGE\n");
        }
        break;

        case MSG_SHEETCMD:
        {
            //printf ("PropsheetWinProc1 receive MSG_SHEETCMD\n");
        }
        break;

        case MSG_COMMAND:
        {
            //printf ("PropsheetWinProc1 receive MSG_COMMAND\n");
            switch (wParam) 
            {
                case IDOK:
                {
                    convert_to_file (hDlg);
                }
                break;
            }
        }
        break;

        case MSG_DESTROY:
        {
            //free (_sel_files);
            //_sel_files = 0; 
        }
        return 0;
    }
    
    return DefaultPageProc (hDlg, message, wParam, lParam);
}
