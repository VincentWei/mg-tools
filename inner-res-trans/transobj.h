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
#ifndef TRANSOBJ_H
#define TRANSOBJ_H

#include "common.h"

#include <string.h>
#include <string>
using namespace std;

class TransObj
{
protected:
	string szInfileName;
	string szOutfileName;
	string szVarName;
	
	static const char*str_null;

public:
	static TransObj* createTransObj(const char* ext);
	
	void setName(const char* infilename, const char* outfilename, const char* varname)
	{
		szInfileName = infilename;
		szOutfileName = outfilename;
		szVarName = varname;
	}

	virtual bool translate(const char *infile, const char* outfile, const char* varname, const char** argv, int argc) = 0;

	virtual const char* getAddtional(){  return str_null; }

	virtual void getExtendName(char* name){ 
		sprintf(name, "unsigned char %s[]", szVarName.c_str());
	}
	
	const char* getInfileName(){ return szInfileName.c_str(); }
	const char* getOutfileName(){ return szOutfileName.c_str(); }
	const char* getVarName() { return szVarName.c_str(); }
	
	virtual int getDataSize(){ return 0; }

	virtual char* getOutputFile(const char* infile, char* outfile)
	{
		const char* begin = strrchr(infile,'/');
		if(begin == NULL) begin = infile;
		else begin ++;
		sprintf(outfile, "%s.c", begin);
		return outfile;
	}
};

unsigned long Str2Key(const char* str);

class FontTransObj: public TransObj
{
	static int font_count;
public:
	virtual char* getOutputFile(const char* infile, char* outfile)
	{
		sprintf(outfile, "incore-font%02d.c",font_count++);
		return outfile;
	}
};


#endif

