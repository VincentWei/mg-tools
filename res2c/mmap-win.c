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

#ifdef WIN32

#include <windows.h>

void *win_mmap(const char *file)
{
	HANDLE obj;
	HANDLE hFile;
	int fileSize;
	void *data = NULL;

	hFile = CreateFile(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE){
		return NULL;
	}

	fileSize = GetFileSize( hFile, NULL);

	obj = CreateFileMapping( hFile, NULL, PAGE_READWRITE,
           0, fileSize, NULL);

	GetLastError();
	if (obj){
	    data = MapViewOfFile( obj, FILE_MAP_WRITE, 0, 0, 0);
	}

	CloseHandle(obj);
	CloseHandle(hFile);
	return data;
}


void win_munmap(void *mem)
{
	UnmapViewOfFile(mem);
}

#endif
