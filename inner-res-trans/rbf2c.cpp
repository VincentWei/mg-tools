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
#include <stdio.h>
#include <stdlib.h>
#include "rbf2c.h"

#define LEN_FONT_NAME  64
#define NR_LOOP_FOR_STYLE   2
#define NR_LOOP_FOR_WIDTH   3
#define NR_LOOP_FOR_HEIGHT  4
#define NR_LOOP_FOR_CHARSET 5


int fontGetWidthFromName (const char* name)
{
    int i;
    const char* width_part = name;
    char width [LEN_FONT_NAME + 1];

    for (i = 0; i < NR_LOOP_FOR_WIDTH; i++) {
        if ((width_part = strchr (width_part, '-')) == NULL)
            return -1;

        if (*(++width_part) == '\0')
            return -1;
    }

    i = 0;
    while (width_part [i]) {
        if (width_part [i] == '-') {
            width [i] = '\0';
            break;
        }

        width [i] = width_part [i];
        i++;
    }

    if (width_part [i] == '\0')
        return -1;

    return atoi (width);
}

int fontGetHeightFromName (const char* name)
{
    int i;
    const char* height_part = name;
    char height [LEN_FONT_NAME + 1];

    for (i = 0; i < NR_LOOP_FOR_HEIGHT; i++) {
        if ((height_part = strchr (height_part, '-')) == NULL)
            return -1;
        if (*(++height_part) == '\0')
            return -1;
    }

    i = 0;
    while (height_part [i]) {
        if (height_part [i] == '-') {
            height [i] = '\0';
            break;
        }

        height [i] = height_part [i];
        i++;
    }

    if (height_part [i] == '\0')
        return -1;

    return atoi (height);
}



bool RBF2C::translate(const char* infile, const char* outfile, const char* varname, const char** argv, int argc)
{
	FILE* fin;
	FILE* fout;
	int i;

	if(argc <= 0 || argv == NULL)
	{
		fprintf(stderr, "RBF translator need <font name>\n");
		return false;
	}
	
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

	fprintf(fout, "#include \"rawbitmap.h\"\n");

	fprintf(fout, "//data of \"%s\"\n\nstatic unsigned char %s_data []={", filename,varname);
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
	
	delete[] bytes;

	int width, height, graph;
	width = fontGetWidthFromName(argv[0]);
	height = fontGetHeightFromName(argv[0]);
	
	//printf rbf struct
	fprintf(fout, "RBFINFO %s[1] = {{\n"
		"\tsizeof(%s_data)/%d,\n"
		"\t%d,\n"
		"\t%d,\n"
		"\t%s_data,\n"
		"\tsizeof(%s)\n}};\n",
		varname,
		varname,
		((width+7)>>3)*height,
		width,
		height,
		varname,
		varname);

	fprintf(fout, "\n#endif //%s\n\n", MARCOR);

	fclose(fout);
	fclose(fin);

	return true;
}

const char* RBF2C::_support_list = "bin\0rbf\0";

