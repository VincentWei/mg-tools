#ifndef QPF2C_H
#define QPF2C_H

#include "transobj.h"
class QPF2C:public FontTransObj
{
public:
	static const char* _support_list;
	bool translate(const char* infile, const char* outfile,const char* varname, const char** argv, int argc);
	void getExtendName(char* name){ 
		sprintf(name, "QPFINFO %s[1]", szVarName.c_str());
	}
};

#endif

