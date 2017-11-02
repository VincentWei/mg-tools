
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bin2c.h"

bool Bin2C::translate(const char* infile, const char* outfile, const char* varname, const char** argv, int argc)
{
	FILE* fin;
	FILE* fout;
	int i;

	size  = 0;
	
	fin = fopen(infile, "rb");
	if(fin == NULL)
		return false;

	fseek(fin, 0, SEEK_END);
	int len = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	if(len <= 0){
		fclose(fin);
		return false;
	}
	
	//open output file
	fout = fopen(outfile, "wt");
	if(fout == NULL)
	{
		fclose(fin);
		return false;
	}
	
	unsigned char* bytes = new unsigned char[len];
	fread(bytes, 1, len, fin);

	const char* filename = strrchr(infile,'/');
	if(filename == NULL) filename = infile;
	else filename ++;

	//output
	fprintf(fout, "\n#include \"%s\"\n", CFGFILE);
	fprintf(fout, "\n#ifdef %s\n\n", MARCOR);
	fprintf(fout, "//data of \"%s\"\n\nunsigned char %s[]={", filename,varname);
	for(i=0; i<len-1; i++)
	{
		if(i%16 == 0) fprintf(fout, "\n\t");
		else if(i%8 == 0) fprintf(fout, "\t");
		fprintf(fout, "0x%02X,", bytes[i]);
	}
	
	if(i%16 == 0) fprintf(fout, "\n\t");
	else if(i%8 == 0) fprintf(fout, "\t");
	fprintf(fout, "0x%02X", bytes[i]);
	fprintf(fout,"\n};\n");
	fprintf(fout, "#endif //%s\n\n", MARCOR);
	
	delete[] bytes;
	fclose(fout);
	fclose(fin);

	size = len;

	return true;
}

