#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transobj.h"
#include "bin2c.h"
#include "qpf2c.h"
#include "upf2c.h"
#include "vbf2c.h"
#include "rbf2c.h"

#define RES_KEY unsigned long
RES_KEY Str2Key(const char* str)
{
	int i,l;
	RES_KEY ret = 0;
	unsigned short *s;

	if(str == NULL)
		return (RES_KEY)-1;
	
	l = (strlen(str)+1) / 2;
	s = (unsigned short*) str;

	for(i=0; i<l; i++)
		ret ^= (s[i]<<(i&0x0f));

	return ret;
}

bool find_in_str(const char* str_list, const char* ext)
{
	while(*str_list)
	{
		if(strcmp(str_list, ext) == 0)
			return true;
		str_list += strlen(str_list)+1;
	}
	return false;
}

TransObj* TransObj::createTransObj(const char* ext)
{
	if(find_in_str(UPF2C::_support_list, ext))
		return new UPF2C;
	else if(find_in_str(QPF2C::_support_list, ext))
		return new QPF2C;
	else if(find_in_str(VBF2C::_support_list, ext))
		return new VBF2C;
	else if(find_in_str(RBF2C::_support_list, ext))
		return new RBF2C;
	else
		return new Bin2C;
}

const char* TransObj::str_null = "\0";

int FontTransObj::font_count = 0;

