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

#define IDL_DIR             100
#define IDL_FILE            110
#define IDC_PATH            120
#define IDC_RLE             130
#define IDC_FORMAT          140
#define IDC_PREFIX          150
#define IDC_PREFIX_STATIC   160
#define IDC_PATH_STATIC     170

DLGTEMPLATE dlg_propsheet2 =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    400, 100, 304, 225,
#ifdef _LANG_ZHCN
    "转储位图对象",
#else
    "Convert To File",
#endif
    0, 0,
    12, NULL,
    0
};

CTRLDATA propsheet2_data[] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 200, 15,
        IDC_STATIC,
#ifdef _LANG_ZHCN
        "目录列表框",
#else
        "Directories:",
#endif
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY,
        10, 30, 200, 150,
        IDL_DIR,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        220, 10, 200, 15, 
        IDC_STATIC, 
#ifdef _LANG_ZHCN
       "文件列表框",
#else
       "Files:",
#endif
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_AUTOCHECKBOX,
        220, 30, 200, 150,
        IDL_FILE,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 200, 120, 30, 
        IDC_PATH_STATIC, 
#ifdef _LANG_ZHCN
       "当前路径：",
#else
       "Current Path: ",
#endif
        0
    },

    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 240, 500, 30, 
        IDC_PATH, 
#ifdef _LANG_ZHCN
       "路径：",
#else
       "Path: ",
#endif
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 280, 160, 30, 
        IDC_PREFIX_STATIC, 
#ifdef _LANG_ZHCN
       "哈希表文件前缀：",
#else
       "Prefix of hash file: ",
#endif
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_BORDER,
        180, 280, 200, 30,
        IDC_PREFIX,
        "autodump",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_CHECKBOX | WS_TABSTOP | BS_AUTOCHECKBOX,
        10, 330, 150, 25,
        IDC_RLE,
#ifdef _LANG_ZHCN
        "采用 RLE 编码"
#else
        "use RLE encode",
#endif
        0
    },
    {
        CTRL_COMBOBOX,
        WS_VISIBLE | CBS_READONLY | WS_TABSTOP | CBS_AUTOFOCUS | CBS_DROPDOWNLIST,
        200, 330, 130, 30,
        IDC_FORMAT,
        "",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        10, 390, 130, 25,
        IDOK, 
#ifdef _LANG_ZHCN
        "保存"
#else
        "Save",
#endif
        0
    },
    {
        "button",
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        180, 390, 130, 25,
        IDCANCEL,
#ifdef _LANG_ZHCN
        "取消"
#else
        "Cancel",
#endif
        0
    }
};

static void fill_boxes (HWND hDlg, const char* path)
{
#ifdef __NOUNIX__
    LISTBOXITEMINFO lbii;

    lbii.string = "file.1";
    lbii.cmFlag = CMFLAG_BLANK;
    lbii.hIcon = 0;
    SendDlgItemMessage (hDlg, IDL_FILE, LB_ADDSTRING, 0, (LPARAM)&lbii);

    lbii.string = "file.2";
    SendDlgItemMessage (hDlg, IDL_FILE, LB_ADDSTRING, 0, (LPARAM)&lbii);

    lbii.string = "file.3";
    SendDlgItemMessage (hDlg, IDL_FILE, LB_ADDSTRING, 0, (LPARAM)&lbii);
#else
    struct dirent* dir_ent;
    DIR*   dir;
    struct stat ftype;
    char   fullpath [PATH_MAX + 1];

    SendDlgItemMessage (hDlg, IDL_DIR, LB_RESETCONTENT, 0, (LPARAM)0);
    SendDlgItemMessage (hDlg, IDL_FILE, LB_RESETCONTENT, 0, (LPARAM)0);
    SetWindowText (GetDlgItem (hDlg, IDC_PATH), path);
    
    if ((dir = opendir (path)) == NULL)
    {
        printf ("ERROR: fail to open dir %s\n", path);
        return;
    }

    while ( (dir_ent = readdir ( dir )) != NULL ) {

        /* Assemble full path name. */
        strncpy (fullpath, path, PATH_MAX);
        strcat (fullpath, "/");
        strcat (fullpath, dir_ent->d_name);
        
        if (stat (fullpath, &ftype) < 0 ) {
           continue;
        }

        if (S_ISDIR (ftype.st_mode))
            SendDlgItemMessage (hDlg, IDL_DIR, LB_ADDSTRING, 0, (LPARAM)dir_ent->d_name);
        else if (S_ISREG (ftype.st_mode)) {
            LISTBOXITEMINFO lbii;

            lbii.string = dir_ent->d_name;
            lbii.cmFlag = CMFLAG_BLANK;
            lbii.hIcon = 0;
            SendDlgItemMessage (hDlg, IDL_FILE, LB_ADDSTRING, 0, (LPARAM)&lbii);
        }
    }

    closedir (dir);
#endif
}

static void dir_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    //printf ("DIR receive code %i\n", nc);
    if (nc == LBN_DBLCLK || nc == LBN_ENTER) {
        int cur_sel = SendMessage (hwnd, LB_GETCURSEL, 0, 0L);
        if (cur_sel >= 0) {
            char cwd [MAX_PATH + 1];
            char dir [MAX_NAME + 1];
            GetWindowText (GetDlgItem (GetParent (hwnd), IDC_PATH), cwd, MAX_PATH);
            SendMessage (hwnd, LB_GETTEXT, cur_sel, (LPARAM)dir);

            if (strcmp (dir, ".") == 0)
                return;

            if (strcmp (dir, "..") == 0) {
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
            else {
                if (strcmp (cwd, "/") != 0)
                    strcat (cwd, "/");
                strcat (cwd, dir);
            }
            fill_boxes (GetParent (hwnd), cwd);
        }
    }
}

typedef enum 
{
    RGB8,
    RGB565,
    RGB888,
    ARGB8888,
    RGB_FORMAT_MAX
}bitmap_format;

static char* bitmap[] =
{
    "8bit",
    "RGB565-16bit",
    "RGB888-24bit",
    "ARGB8888-32bit"
};

static void file_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    /* Do nothing */
}

extern BOOL RLE_ENCODE;

static void save_bmp_2c(HWND hDlg)
{
    int i, filenum;
#ifdef _LANG_ZHCN
    char files [1024] = "你确定要转储？\n";
#else
    char files [1024] = "Are you sure to save?\n";
#endif
    char file [MAX_NAME + 1]={0};
    char prefix[MAX_NAME+1]={0};
    int     status;
    HDC     mem_dc=HDC_INVALID;
    int     bmp_format=0;
    DWORD   flags = MEMDC_FLAG_SWSURFACE;

    char path[MAX_PATH+1];

#ifdef _LANG_ZHCN
    MessageBox (hDlg, files, "确认转储", MB_OK | MB_ICONINFORMATION);
#else
    MessageBox (hDlg, files, "save files", MB_OK | MB_ICONINFORMATION);
#endif
    
    if( SendMessage(GetDlgItem(hDlg, IDC_RLE), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        RLE_ENCODE = TRUE;
        flags |= MEMDC_FLAG_RLEACCEL;
    }
    
    bmp_format = SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_GETCURSEL, 0, 0);
    fprintf(stderr, " bmp_format=%d\n", bmp_format);
    GetWindowText(GetDlgItem(hDlg, IDC_PREFIX), prefix, MAX_NAME);
    fprintf(stderr, "prefix=%s\n", prefix);

    if (bmp_format) {
        int depth = 8;
        switch (bmp_format) {
        case RGB565:
            depth = 16;
            break;
        case RGB888:
            depth = 24;
            break;
        case ARGB8888:
            depth = 32;
            break;
        default:
            break;
        }
        mem_dc = CreateMemDC (1024, 768, depth, flags, 0x00, 0x00, 0x00, 0x00);
    }

    GetWindowText (GetDlgItem (hDlg, IDC_PATH), path, MAX_PATH);
    chdir(path);

    //make_file_begin();
    //make_loader_begin();
    
    filenum = SendDlgItemMessage (hDlg, IDL_FILE, LB_GETCOUNT, 0, 0L);
    for (i = 0; i < filenum; i++) {
        status = SendDlgItemMessage (hDlg, IDL_FILE, LB_GETCHECKMARK, i, 0);
        if (status == CMFLAG_CHECKED) {
            SendDlgItemMessage (hDlg, IDL_FILE, LB_GETTEXT, i, (LPARAM)file);
            if (mem_dc)
                dump_from_file(mem_dc, file);
            else
                dump_from_file(HDC_SCREEN, file);
	    }
    }
    dump_hash_table(prefix);

    //make_file_end();
    //make_loader_end();
    
    if (bmp_format)
        DeleteMemDC(mem_dc);
}

int PropsheetWinProc2 (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
        case MSG_INITPAGE:
        {
            int i;
            HWND hctrl;
            char cwd [MAX_PATH + 1];
            SetNotificationCallback 
                (GetDlgItem (hDlg, IDL_DIR), dir_notif_proc);

            SetNotificationCallback 
                (GetDlgItem (hDlg, IDL_FILE), file_notif_proc);

            fill_boxes (hDlg, getcwd (cwd, MAX_PATH));
            hctrl = GetDlgItem(hDlg, IDC_FORMAT);
            for(i=0;i<RGB_FORMAT_MAX;i++)
                SendMessage(hctrl, CB_ADDSTRING, 0, (LPARAM)bitmap[i]);
            SendMessage(hctrl, CB_SETCURSEL, 0, 0);
        }
        return 1;
            
        case MSG_SHOWPAGE:
        {
        }
        break;

        case MSG_SHEETCMD:
        {
        }
        break;

        case MSG_COMMAND:
        {
            switch (wParam) 
            {
            case IDOK:
                save_bmp_2c(hDlg);
                break;
            case IDCANCEL:
                EndDialog (hDlg, wParam);
                break;
            }
        }
        break;
    }
    return DefaultPageProc (hDlg, message, wParam, lParam);
}

