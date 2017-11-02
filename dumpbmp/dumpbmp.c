/*
** $Id: dumpbmp.c 260 2009-05-23 10:33:26Z weiym $
**
** vbfeditor.c: A VBF font editor.
**
** Copyright (C) 2003 Feynman Software.
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
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef WIN32
#include <io.h>
#include <string.h>
#include "FreeImage.h"
#endif
#include <time.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgutils.h>
#include "dumpbmp.h"

#define IDC_TB_SELF            100
#define IDC_TB_OPEN            110
#define IDC_TB_SAVE            120
#define IDC_TB_PREVIOUS        130
#define IDC_TB_NEXT            140
#define IDC_TB_ZOOMOUT         150
#define IDC_TB_ZOOMIN          160

static FILE *fp_header = NULL, *fp_bmps = NULL, *fp_inc = NULL;
static FILE *fp_loader = NULL, *fp_unloader = NULL;
static int bitmaps_number = 0;

BOOL RLE_ENCODE = FALSE;

static struct toolbar_items
{
    int id;
    char* name;
} toolbar_items [] =
{
        {IDC_TB_OPEN, "open"},
        {IDC_TB_SAVE, "save"},
        {IDC_TB_PREVIOUS, "previous"},
        {IDC_TB_NEXT, "next"}
};

static void InitToolBar (HWND toolbar)
{
    int i;
    char chtmp[MAX_PATH+1];
    TOOLBARITEMINFO pData;

    strcpy (chtmp, "../res/");

    for (i = 0; i < TABLESIZE (toolbar_items); i++) {
        pData.id = toolbar_items [i].id;
        pData.insPos = i + 1;
        sprintf (pData.NBmpPath, "%s%s%s", chtmp, toolbar_items [i].name, "1.bmp");
        sprintf (pData.HBmpPath, "%s%s%s", chtmp, toolbar_items [i].name, "2.bmp");
        sprintf (pData.DBmpPath, "%s%s%s", chtmp, toolbar_items [i].name, "3.bmp");
        SendMessage (toolbar, TBM_ADDITEM, 0, (LPARAM)&pData);
    }
}

#define TB_HEIGHT              16
#define TB_WIDTH               22

static MYBITMAP my_dib;
static BITMAP my_bmp;
static RGB my_pal [256];

static int ViewWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
#ifndef WIN32 
    static FILEDLGDATA filedlgdata = {FALSE, ".", "*.bmp", "."};

    switch (message) {
        case MSG_CREATE:
        {
            HWND  hTBSelf;
            DWORD WdHt = MAKELONG (TB_HEIGHT, TB_WIDTH);

            hTBSelf = CreateWindow ("toolbar", " ",
                              WS_CHILD | WS_VISIBLE , IDC_TB_SELF,
                              0, 0, HIWORD(WdHt)*6,
                              LOWORD(WdHt), hWnd, WdHt);

            InitToolBar (hTBSelf);
        }
        break;

        case MSG_COMMAND:
        {
            int tool_id;

            if (LOWORD(wParam) != IDC_TB_SELF) {
                break;
            }

            tool_id = HIWORD(wParam);
            switch (tool_id) {
                case IDC_TB_OPEN:
                    if (OpenFileDialog (hWnd, FALSE, &filedlgdata) == IDOK) {
                        if (my_dib.bits) {
                            UnloadMyBitmap (&my_dib);
                            UnloadBitmap (&my_bmp);
                            my_dib.bits = NULL;
                        }
                        if (LoadMyBitmapFromFile (&my_dib, my_pal, filedlgdata.filefullname) == 0) {
                            ExpandMyBitmap (HDC_SCREEN, &my_bmp, &my_dib, my_pal, 0);
                            InvalidateRect (hWnd, NULL, TRUE);
                        }
                    }
                    break;

                case IDC_TB_SAVE:
                    if (my_dib.bits) {
                        HDC mem_dc;
                        BITMAP bitmap;
                        struct mem_dc_info info;

                        if (get_dump_info (hWnd, &info))
                            break;

                        mem_dc = CreateMemDC (8, 8, info.bpp, MEMDC_FLAG_SWSURFACE,
                                    info.rmask, info.gmask, info.bmask, info.amask);
                        if (info.bpp <= 8)
                            SetPalette (mem_dc, 0, 1 << info.bpp, info.pal);

                        ExpandMyBitmap (mem_dc, &bitmap, &my_dib, my_pal, 0);
                        dump_bitmap (&my_bmp, "a", fp_header);
                        UnloadBitmap (&bitmap);
                        DeleteMemDC (mem_dc);
                    }
                    break;

                case IDC_TB_PREVIOUS:
                    break;

                case IDC_TB_NEXT:
                    break;
            }
        }
        break;

        case MSG_PAINT:
        {
            HDC hdc = BeginPaint (hWnd);
            if (my_dib.bits) {
                FillBoxWithBitmap (hdc, 0, TB_HEIGHT + 10, 0, 0, &my_bmp);
            }
            EndPaint (hWnd, hdc);
            return 0;
        }

        case MSG_CLOSE:
            UnloadMyBitmap (&my_dib);
            UnloadBitmap (&my_bmp);
            DestroyAllControls (hWnd);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
#endif
	return 0;
}

#ifdef WIN32
static const char* get_extension (const char* filename)
{
    const char* ext;
	
    ext = strrchr (filename, '.');
	
    if (ext)
        return ext + 1;
	
    return NULL;
}
#endif

static void dump_from_file (const char* file)
{
    BITMAP bmp;
    char path [MAX_PATH + 1];
    char prefix [LEN_PREFIX + 1];
    char *tmp;
	int flag = 0, ret;
#ifdef WIN32
	FIBITMAP *fibitmap;
	const char *ext;
	FREE_IMAGE_FORMAT image_format = FIF_UNKNOWN;
	struct _finddata_t c_file; 
	char filefullname[PATH_MAX+NAME_MAX+1];	
	long hFile; 
	char tmp_path[MAX_PATH + 1];
	int path_flag = 0;
	char *tmp2;
#endif
	
    strncpy (path, file, MAX_PATH);
    tmp = strrchr (path, '.');
	if (tmp) {
        *tmp = '\0';
	}
#ifdef WIN32
	else {
		path_flag = 1;
		sprintf (tmp_path, "%s", strtok (path, "*"));
		strcpy (path, tmp_path);
	}		
#endif
	tmp = path;	
	while (*tmp) {
		if (*tmp == ' ' || *tmp == '/' || *tmp == '.' || *tmp == '-' || *tmp == '\\')
			*tmp = '_';
		tmp++;
	}
	strncpy (prefix, path, LEN_PREFIX);
#ifdef WIN32
	if (path_flag) {
		char find_path[MAX_PATH + 1];
		memset (find_path, 0, MAX_PATH + 1);
		sprintf (find_path, "%s*.*", tmp_path);
		if( (hFile = _findfirst(find_path, &c_file )) == -1L ) 
			printf( "No files in %s!\n" , tmp_path); 
		else 
		{ 
			_findnext( hFile, &c_file );
			while( _findnext( hFile, &c_file ) == 0 ) 
			{ 
				file = c_file.name;
				memset (filefullname, 0, PATH_MAX+NAME_MAX+1);
				sprintf (filefullname, "%s%s", tmp_path, file);
				if ((ext = get_extension (file)) == NULL)
					continue;
				if (strcmp (ext, "c") == 0)
					continue;
				if(strcmp(ext, "bmp") != 0 && strcmp(ext, "gif") != 0)
				{
					if(strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0 )		
						image_format = FIF_JPEG;
					else if (strcmp (ext, "png") ==0)
						image_format = FIF_PNG;
					else if (strcmp (ext, "tiff") == 0)		
						image_format = FIF_TIFF;
					else if (strcmp (ext, "psd") == 0)
						image_format = FIF_PSD;
					
					if(image_format != FIF_UNKNOWN)
					{
						fibitmap = FreeImage_Load (image_format, filefullname, 0);
						printf ("fibitmap = %x\n", fibitmap);
						FreeImage_Save (FIF_BMP, fibitmap, "tmp.bmp", 0);
						FreeImage_Unload (fibitmap);
						flag = 1;
					}
				}
				
				if(flag)
					ret = LoadBitmapFromFile (HDC_SCREEN, &bmp, "tmp.bmp");
				else {
					ret = LoadBitmapFromFile (HDC_SCREEN, &bmp, filefullname);
				}
		
				if (ret == 0) 
				{
					char fp_header_path[PATH_MAX+NAME_MAX+1];
					char *prev = NULL;
					
					if (fp_header) 
					{
						strcpy (fp_header_path, filefullname);
						tmp = fp_header_path;
						while (tmp)
						{
							tmp = strchr (tmp, '\\');
							if (tmp) {
								*tmp ++ = '_';
							    prev = tmp;
							}
						}
						
						tmp = strchr (prev, '.');
						if (tmp) {
							*(++tmp) = 'c';
							*(++tmp) = '\0';
						}
						
						fprintf (fp_inc, "#include \"%s\"\n", fp_header_path);
						tmp = strchr (fp_header_path, '.');
						if (tmp)
							*tmp = '\0';
						bmp.bmType = BMP_TYPE_COLORKEY;
						bmp.bmColorKey = RGB2Pixel (HDC_SCREEN, 248, 13, 240);
						tmp = fp_header_path;

						while (*tmp) {
							if (*tmp == ' ' || *tmp == '/' || *tmp == '.' || *tmp == '-' || *tmp == '\\')
								*tmp = '_';
							tmp++;
						}

						add_bmp_header_entry (fp_header, fp_header_path);
					}
					if (fp_loader) 
					{
						add_bmp_loader_entry (fp_loader, filefullname, fp_header_path);
						add_bmp_unloader_entry (fp_unloader, fp_header_path);
					}
					bitmaps_number ++;
					dump_bitmap (&bmp, fp_header_path, fp_bmps);
					if (!fp_header && !fp_loader)
						add_hash_entry (filefullname, fp_header_path);
				}
				else 
					fprintf (stderr, "Skip file: %s\n", filefullname);
			} 
		}
		remove ("tmp.bmp");	
		return;
	}
#endif

#ifdef WIN32
	if ((ext = get_extension (file)) == NULL) 
		strcpy (ext, "bmp");

	printf ("ext = %s\n", ext);
	if(strcmp(ext, "bmp") != 0 || strcmp(ext, "gif") != 0)
	{
		if(strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0 )		
			image_format = FIF_JPEG;
		else if (strcmp (ext, "png") ==0)
			image_format = FIF_PNG;
		else if (strcmp (ext, "tiff") == 0)		
			image_format = FIF_TIFF;
		else if (strcmp (ext, "psd") == 0)
			image_format = FIF_PSD;
		
		if(image_format != FIF_UNKNOWN)
		{
			fibitmap = FreeImage_Load (image_format, file, 0);			
			FreeImage_Save (FIF_BMP, fibitmap, "tmp.bmp", 0);
			FreeImage_Unload (fibitmap);
			flag = 1;
		}
	}

	if(flag) 
	ret = LoadBitmapFromFile (HDC_SCREEN, &bmp, "tmp.bmp");

	else 		
	ret = LoadBitmapFromFile (HDC_SCREEN, &bmp, file);

#else
	ret = LoadBitmapFromFile (HDC_SCREEN, &bmp, file);
#endif
	
    if (ret == 0) 
	{
        if (fp_header) 
		{
            fprintf (fp_inc, "#include \"%s.c\"\n", prefix);
            bmp.bmType = BMP_TYPE_COLORKEY;
            bmp.bmColorKey = RGB2Pixel (HDC_SCREEN, 248, 13, 240);
            add_bmp_header_entry (fp_header, prefix);
        }
        if (fp_loader) 
		{
            add_bmp_loader_entry (fp_loader, file, prefix);
            add_bmp_unloader_entry (fp_unloader, prefix);
        }
        bitmaps_number ++;
        dump_bitmap (&bmp, prefix, fp_bmps);
        if (!fp_header && !fp_loader)
            add_hash_entry (file, prefix);
    }
    else 
        fprintf (stderr, "Skip file: %s\n", file);
}

static int make_file_begin (void)
{
   /* make header file begin part */
   if ((fp_header = fopen ("fh_bitmaps_defs.h", "w+")) == NULL)
       return -1;
   fprintf (fp_header, "/*\n");
   fprintf (fp_header, "    FHAS phone edition bitmap resource ID definitions\n");
   fprintf (fp_header, "    This file is generated automatically by dumpbmp tool, do not edit.\n");
   fprintf (fp_header, "*/\n\n");
   fprintf (fp_header, "extern BITMAP fhas_bitmaps[];\n\n");

   /* make bitmap struct file begin part */
   if ((fp_bmps = fopen ("fh_bitmaps_struct.c", "w+")) == NULL)
       return -1;
   fprintf (fp_bmps, "/*\n");
   fprintf (fp_bmps, "    FHAS phone edition bitmap resource\n");
   fprintf (fp_bmps, "    This file is generated automatically by dumpbmp tool, do not edit.\n");
   fprintf (fp_bmps, "*/\n\n");
   fprintf (fp_bmps, "#include \"fh_bitmaps_inc.c\"\n\n");
   fprintf (fp_bmps, "BITMAP fhas_bitmaps[] = {\n\n");

   if ((fp_inc = fopen ("fh_bitmaps_inc.c", "w+")) == NULL)
       return -1;
   fprintf (fp_inc, "/*\n");
   fprintf (fp_inc, "    FHAS phone edition bitmap include files\n");
   fprintf (fp_inc, "    This file is generated automatically by dumpbmp tool, do not edit.\n");
   fprintf (fp_inc, "*/\n\n");

   return 0;
}

static int make_file_end (void)
{
   fprintf (fp_bmps, "};\n\n");
   fprintf (fp_bmps, "BITMAP* fhbmps = fhas_bitmaps;\n\n");
   fclose (fp_bmps);
   fclose (fp_inc);

   fprintf (fp_header, "#define fhbmp_number    %d\n", bitmaps_number);
   fprintf (fp_header, "\n");
   fclose (fp_header);
   return 0;
}

static void make_loader_begin (void)
{
   if ((fp_loader = fopen ("fh_bitmaps_loader.c", "w+")) == NULL)
       return;
   fprintf (fp_loader, "/*\n");
   fprintf (fp_loader, "    Bitmap loading functions.\n");
   fprintf (fp_loader, "    This file is generated automatically by dumpbmp tool, do not edit.\n");
   fprintf (fp_loader, "*/\n\n");

   fprintf (fp_loader, "BITMAP fhas_bitmaps[fhbmp_number];\n\n");

   fprintf (fp_loader, "BOOL fhbmp_load_all (const char *res_path)\n");
   fprintf (fp_loader, "{\n");
   fprintf (fp_loader, "    char buffer[256];\n");
   fprintf (fp_loader, "    char *tmp;\n\n");
   fprintf (fp_loader, "    if (res_path)\n");
   fprintf (fp_loader, "        strcpy(buffer, res_path);\n");
   fprintf (fp_loader, "    else\n");
   fprintf (fp_loader, "        buffer[0] = 0;\n");
   fprintf (fp_loader, "    tmp = buffer + strlen(buffer);\n\n");

   if ((fp_unloader = fopen ("fh_bitmaps_unloader.c", "w+")) == NULL)
       return;
   fprintf (fp_unloader, "/*\n");
   fprintf (fp_unloader, "    Bitmap unloading functions.\n");
   fprintf (fp_unloader, "    This file is generated automatically by dumpbmp tool, do not edit.\n");
   fprintf (fp_unloader, "*/\n\n");

   fprintf (fp_unloader, "void fhbmp_unload_all (void)\n");
   fprintf (fp_unloader, "{\n");
}

static void make_loader_end (void)
{
   fprintf (fp_loader, "\n");
   fprintf (fp_loader, "    return TRUE;\n");
   fprintf (fp_loader, "}\n");
   fclose (fp_loader);

   fprintf (fp_unloader, "}\n");
   fprintf (fp_unloader, "\n");
   fclose (fp_unloader);
}

int MiniGUIMain (int args, const char* arg[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;
    int ibegin = 1;
    BOOL bI = FALSE, bL = FALSE;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER, arg[0], 0, 0);
#endif

    printf ("%s\n", arg[0]);
	if (!arg[ibegin] || !arg[ibegin + 1])
		return -1;
	if (strcmp(arg[ibegin], "-i") == 0 || strcmp(arg[ibegin], "-l") == 0) {
		ibegin ++;
		bI = TRUE;
	}

	if (strcmp(arg[ibegin], "-i") == 0 || strcmp(arg[ibegin], "-l") == 0) {
		ibegin ++;
		bL = TRUE;
	}

	if (strcmp(arg[ibegin], "-r") == 0) {
		ibegin ++;
		RLE_ENCODE = TRUE;
	}

    if (bI)
        make_file_begin();
    if (bL)
        make_loader_begin();

    if (args >= ibegin+1) {
        int i;

        for (i = ibegin; i < args; i++) 
            dump_from_file (arg [i]);

        if (ibegin == 1)
            dump_hash_table ("fhas");

        if (bI)
            make_file_end();
        if (bL)
            make_loader_end();

        printf ("============== dump bitmap over! ================\n");
        return 0;
    }

    if (!InitMiniGUIExt ())
        return 2;
        
    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "Dump BITMAP Object";
    CreateInfo.hMenu = 0;
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

    MiniGUIExtCleanUp ();
        
    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

