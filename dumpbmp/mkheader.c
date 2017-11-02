/* make a header file of bitmaps */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "dumpbmp.h"


int add_bmp_header_entry (FILE *fp, const char* prefix)
{
    char buffer[128], buffer_org[128], *tmp;
    static int index = 0;

    sprintf (buffer, "%s", prefix);
    sprintf (buffer_org, "%s", prefix);

    tmp = buffer;

    while (*tmp != '\0') {
	    if (*tmp == '/')
            *tmp = '_';
	    else
            *tmp = toupper (*tmp);
	    tmp ++;
    }

    fprintf (fp, "#define  %-24s        %d\n", buffer, index);
    fprintf (fp, "#define    bmp_%-24s  (fhas_bitmaps+%s)\n", buffer_org, buffer);
    index ++;
    return 0;
}

int add_bmp_loader_entry (FILE *fp, const char* file, const char* prefix)
{
    char buffer_org[128];

    sprintf (buffer_org, "%s", prefix);

    fprintf (fp, "    strcpy(tmp, \"%s\");\n", file);
    fprintf (fp, "    LoadBitmapFromFile(HDC_SCREEN, bmp_%s, buffer);\n", buffer_org);

    return 0;
}

int add_bmp_unloader_entry (FILE *fp, const char* prefix)
{
    char buffer_org[128];

    sprintf (buffer_org, "%s", prefix);

    fprintf (fp, "    UnloadBitmap (bmp_%s);\n", buffer_org);

    return 0;
}

