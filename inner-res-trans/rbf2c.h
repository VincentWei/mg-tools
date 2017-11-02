#ifndef RBF2C_H
#define RBF2C_H

#include "transobj.h"

class RBF2C:public FontTransObj
{
public:
	static const char* _support_list;
	bool translate(const char* infile, const char* outfile,const char* varname, const char** argv, int argc);
	void getExtendName(char* name){ 
		sprintf(name, "RBFINFO %s[1]", szVarName.c_str());
	}
};

#endif

