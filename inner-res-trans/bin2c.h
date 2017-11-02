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

