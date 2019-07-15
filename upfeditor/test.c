/*
** This file is part of mg-tools, a collection of programs to convert
** and maintain the resource for MiniGUI.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
