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
#ifndef BIN2C_H
#define BIN2C_H

#include "transobj.h"

class Bin2C: public TransObj
{
	int size;
public:
	bool translate(const char* infile, const char* outfile, const char* varname, const char** argv, int argc);

	const char* getAddtional(){
		const char* ext = strrchr(szInfileName.c_str(),'.');
		if(ext)
			ext ++;
		return ext?ext:str_null;
	}

	virtual int getDataSize(){ return size; }
};

#endif

