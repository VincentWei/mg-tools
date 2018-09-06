
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transobj.h"
#include <vector>
using namespace std;

static void tr(char* str)
{
	if(str == NULL) 
		return ;
	while(*str)
	{
		if(!isalnum(*str))
		{
			*str = '_';
		}
		str ++;
	}
}

const char* MARCOR = "_MGINCORE_RES";
const char* LISTNAME = "_reses";
const char* CFGFILE = "common.h";
const char* PREFIX = "_mgir";
const char* indir;
const char* outdir;
const char* listfile;

/* param:
 *   -i indir
 *   -l listfile
 *   -o outdir
 *   -m marcor
 *   -n out name
 *   -c cfg-file
 *   -h|? help
 */
const char* str_help = " inner-trans param: \n"
	"-i indir\n"
	"-l listfile\n"
	"-o outdir\n"
	"-m marcor\n"
	"-n out name\n"
	"-c cfg-file\n"
	"-p name prefix\n"
	"-h|? help\n\n";

static void show_help(){
	printf(str_help);
}
static bool parser_args(int argc, char* argv[])
{
	int i = 1;
	while(i<argc)
	{
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
			case 'i': indir = argv[++i]; break;
			case 'l': listfile = argv[++i]; break;
			case 'o': outdir = argv[++i]; break;
			case 'n': LISTNAME = argv[++i]; break;
			case 'm': MARCOR = argv[++i]; break;
			case 'c': CFGFILE = argv[++i]; break; 
			case 'p': PREFIX = argv[++i]; break;
			case 'h':
			case '?': show_help(); break;
			}
		}
		i ++;
	}

	return (indir && listfile && outdir);
}

static void file2name(const char* file, char* name)
{
	if(file == NULL || name == NULL)
		return;
	
	int idx = 0;
	const char* ext;
	const char* begin;
	int len;
	begin = strrchr(file, '/');
	if(begin == NULL) begin = file;
	else begin ++;

	if(PREFIX){
		strcpy(name, PREFIX);
		idx = strlen(name);
	}

	name[idx++] = '_';
	ext = strrchr(begin, '.');
	if(ext)
	{
		strcpy(name+idx, ext+1);
		idx += strlen(name+idx);
	}
	
	name[idx++] = '_';
	len = ext?(ext-begin):(strlen(begin));
	strncpy(name+idx, begin, len);
	name[idx+len] = '\0';
	tr(name+idx);
	idx += len;
	if(name[idx] != '_')
		name[idx++] = '_';
	strcpy(name+idx, "data");
}

static int splitestr(char* str, char** arr, const char* chsp)
{
	int count = 0;
	if(str == NULL || arr == NULL)
		return 0;

	while(*str && strchr(chsp, *str)) str++;
	if(!*str)
		return 0;

	arr[count++] = str;
	str ++;
	while(*str)
	{
		if(strchr(chsp,*str)){
			*str = 0;
			str++;
			while(*str && strchr(chsp,*str)) str++;
			if(str == '\0') break;
			arr[count ++] = str;
		}
		str ++;
	}
	return count;
}

void get_ext(const char* file, char* ext)
{
	const char* str = strrchr(file, '.');
	if(str)
		strcpy(ext, str+1);
	else
		ext[0] = 0;
	return;
}

vector<TransObj*>  list;

int main(int argc, char* argv[])
{
	if(!parser_args(argc, argv)){
		show_help();
		return -1;
	}

	char szline[1024];
	char * strarray[16];
	int arrsize;
	char szinfile[256];
	char szoutfile[256];
	char* outname;
	char* inname;
	char szname[256];
	FILE* f;

	sprintf(szoutfile, "%s/", outdir);
	outname = szoutfile+strlen(szoutfile);
	sprintf(szinfile, "%s/", indir);
	inname = szinfile + strlen(szinfile);
	
	f = fopen(listfile, "rt");
	if(f == NULL)
		return -1;

	while(fgets(szline, sizeof(szline)-1,f))
	{
		char ext[20];
		char* str = (char*)strrchr(szline,'\n');
		if(str)*str = 0;
		arrsize = splitestr(szline, strarray, " \t\n");
		if(arrsize <= 0)
			continue;
		get_ext(strarray[0], ext);	
		TransObj* obj = TransObj::createTransObj(ext);
		if(obj == NULL)
			continue;
		file2name(strarray[0], szname);
		obj->getOutputFile(strarray[0], outname);
		strcpy(inname, strarray[0]);
		if(!obj->translate(szinfile, szoutfile, szname, (const char**)(strarray+1), arrsize-1))
		{
			delete obj;
			continue;
		}
		obj->setName(inname, outname, szname);
		list.push_back(obj);
	}
	fclose(f);
	
	sprintf(szline, "%s/%s.c", outdir, LISTNAME);
	FILE *fout = fopen(szline, "wt");
	vector<TransObj*>::iterator i;
	
	fprintf(fout, "//inner list file\n");
	fprintf(fout, "//create by inner-trans\n\n");

	fprintf(fout, "#ifdef %s\n\n", MARCOR);

	fprintf(fout, "//include files\n");
	for(i =list.begin(); i!= list.end(); ++i)
	{
		TransObj* obj = *i;
		char extenName[256];
//		fprintf(fout, "#include \"%s\" //%s\n", obj->getOutfileName(),
//		obj->getInfileName());
		obj->getExtendName(extenName);
		fprintf(fout, "extern %s; //%s \n", extenName, obj->getOutfileName());
	}
	
	fprintf(fout, "//declear arrays\n");
	fprintf(fout, "static INNER_RES %s%s[]={\n",PREFIX, LISTNAME);
	for(i =list.begin(); i!= list.end(); ++i)
	{
		TransObj* obj = *i;
		int size = obj->getDataSize();
		char szSize[100];
		if(size == 0)
			sprintf(szSize, "sizeof(%s)", obj->getVarName());
		else
			sprintf(szSize, "%d", size);

		fprintf(fout, "\t{ 0x%lX/* %s */, (void*)%s, %s, \"%s\"}, //%s\n",
			Str2Key(obj->getInfileName()),
			obj->getInfileName(),
			obj->getVarName(),
			szSize,
			obj->getAddtional(),
			obj->getOutfileName());
		delete obj;
	}
	fprintf(fout, "};\n\n");

	fprintf(fout, "#endif // %s\n\n", MARCOR);

	fclose(fout);
	return 0;
}


