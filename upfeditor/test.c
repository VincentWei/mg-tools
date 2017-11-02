#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mgutils/mgutils.h>

#include "upf.h"

int MiniGUIMain (int args, const char* arg[])
{
    UPFINFO upf_info;
#define OPEN_FILE "/home/wangxuguang/devel/minigui/mg-tools/upfeditor/unifont_160_50.upf"
    load_upf (&upf_info, OPEN_FILE);
    upf_add_char_glyph(&upf_info, 32837); /*32853, 32855, 32857*/
    upf_add_char_glyph(&upf_info, 32855); /*32853, 32855, 32857*/
    upf_add_char_glyph(&upf_info, 32857); /*32853, 32855, 32857*/
#define SAVE_FILE "/home/wangxuguang/devel/minigui/mg-tools/upfeditor/111.upf"
    dump_upf (&upf_info, SAVE_FILE);
    load_upf (&upf_info, SAVE_FILE);

    upf_delete_char_glyph(&upf_info, 32837); /*32853, 32855, 32857*/
    upf_delete_char_glyph(&upf_info, 32855); /*32853, 32855, 32857*/
    upf_delete_char_glyph(&upf_info, 32857); /*32853, 32855, 32857*/
    dump_upf (&upf_info, SAVE_FILE);
    load_upf (&upf_info, SAVE_FILE);
    return 0;
}
