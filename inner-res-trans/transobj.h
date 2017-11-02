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

