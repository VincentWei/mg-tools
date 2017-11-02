#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "rbf.h"

BOOL dumpRBF (const RBFINFO* rbf, char* file)
{
    FILE* fp;
    if ((fp = fopen (file, "w+")) == NULL)
        return FALSE;

    fclose (fp);
    return TRUE;
}

