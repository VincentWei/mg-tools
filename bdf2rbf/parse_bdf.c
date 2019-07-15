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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bdf.h"

#define LINE_BUF_LEN       256

/* get one line from file */
static char* my_getline (char* buf, FILE* fp)
{
    int len;
    fgets (buf, LINE_BUF_LEN, fp);
    len = strlen (buf);

    while (len > 1 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
        buf[--len] = '\0';

    /* void line */
    if (!*buf)
        return NULL;

    while (*buf && !isspace(*buf))
        buf++;
    *buf++ = '\0';

    while (*buf && isspace(*buf))
        buf++;

    if (*buf == '\"') {
        buf++;
        *(strchr(buf, '\"')) = '\0';
    }

    return (*buf) ? buf : NULL;
}

#define FIND_LINE(line_buf, fp, header, body)   \
do  \
{   \
    body = my_getline (line_buf, fp);  \
} while (!feof(fp) && strcmp (line_buf, header) != 0)

static BOOL get_font_propt (FILE* fp, BDF_INFO* info)
{
    char line_buf [LINE_BUF_LEN];
    char* body;

    FIND_LINE (line_buf, fp, "FONT", body);

    FIND_LINE (line_buf, fp , "FONTBOUNDINGBOX", body);
    sscanf (body, "%d %d %d %d", &info->fnt_bbox_w, &info->fnt_bbox_h, 
            &info->fnt_bbox_x, &info->fnt_bbox_y);

    FIND_LINE (line_buf, fp, "STARTPROPERTIES", body);
    while (1) {
        body = my_getline (line_buf, fp);

        if (strcmp (line_buf, "ENDPROPERTIES") == 0)
            break;

        if (strcmp (line_buf, "FOUNDRY") == 0) {
            if (strlen (body) > LEN_FONT_NAME) {
                fprintf (stderr, "BDF Parsor: FOUNDRY is too long: %s\n", body);
                return FALSE;
            }
            strncpy (info->foundry, body, LEN_FONT_NAME);
            info->foundry [LEN_FONT_NAME] = '\0';
        }
        else if (strcmp (line_buf, "FAMILY_NAME") == 0) {
            if (strlen (body) > LEN_FONT_NAME) {
                fprintf (stderr, "BDF Parsor: FAMILY_NAME is too long: %s\n", body);
                return FALSE;
            }
            strncpy (info->family_name, body, LEN_FONT_NAME);
            info->family_name [LEN_FONT_NAME] = '\0';
        }
        else if (strcmp (line_buf, "WEIGHT_NAME") == 0) {
            strncpy (info->weight_name, body, LEN_FONT_NAME);
            info->weight_name [LEN_FONT_NAME] = '\0';
        }
        else if (strcmp (line_buf, "SLANT") == 0) {
            strncpy (info->slant, body, LEN_FONT_NAME);
            info->slant [LEN_FONT_NAME] = '\0';
        }
        else if (strcmp (line_buf, "SETWIDTH_NAME") == 0) {
            strncpy (info->setwidth_name, body, LEN_FONT_NAME);
            info->setwidth_name [LEN_FONT_NAME] = '\0';
        }
        else if (strcmp (line_buf, "CHARSET_REGISTRY") == 0) {
            strncpy (info->charset_registry, body, LEN_FONT_NAME);
            info->charset_registry [LEN_FONT_NAME] = '\0';
        }
        else if (strcmp (line_buf, "CHARSET_ENCODING") == 0) {
            strncpy (info->charset_encoding, body, LEN_FONT_NAME);
            info->charset_encoding [LEN_FONT_NAME] = '\0';
        }
        else if (strcmp (line_buf, "FACE_NAME") == 0) {
            strncpy (info->face_name, body, LEN_FONT_NAME);
            info->face_name [LEN_FACE_NAME] = '\0';
        }
        else if (strcmp (line_buf, "PIXEL_SIZE") == 0)
            info->pixel_size = atoi (body);
        else if (strcmp (line_buf, "POINT_SIZE") == 0)
            info->point_size = atoi (body);
        else if (strcmp (line_buf, "FONT_ASCENT") == 0)
            info->font_ascent = atoi (body);
        else if (strcmp (line_buf, "FONT_DESCENT") == 0)
            info->font_descent = atoi (body);
        else if (strcmp (line_buf, "DEFAULT_CHAR") == 0)
            info->default_char = atoi (body);
    }

    FIND_LINE (line_buf, fp, "CHARS", body);
    info->nr_glyphs = atoi (body);

    return TRUE;
}

#define HEX_CHAR_VAL(ch)        \
do {                            \
    if (ch & 0x40) /*'a'--'f'*/ \
        ch = (ch & 0x0F) + 9;   \
    else /*'0'--'9'*/           \
        ch &= 0x0F;             \
} while (0)

int bdf_parse_file (const char* file_name, BDF_INFO* info)
{
    int i;
    long bdf_file_size;
    FILE* fp;
    unsigned char *bitmap;

    if (info == NULL)
        return -1;

    fp = fopen (file_name, "rb");
    if (fp == NULL)
        return -1;

    fseek (fp, 0, SEEK_END);
    bdf_file_size = ftell (fp);
    fseek (fp, 0, SEEK_SET);

    if (!fp) {
        fprintf (stderr, "BDF Parsor: open bdf file (%s) error.\n", file_name);
        return -1;
    }

    if (!get_font_propt (fp, info)) {
        fprintf (stderr, "BDF Parsor: can not get font properties from %s.\n", file_name);
        return -1;
    }

    if (info->pixel_size > 128) {
        fprintf (stderr, "BDF Parsor: the font size in file %s is too big (%d)\n", 
                    file_name, info->pixel_size);
        return -1;
    }

    if (info->on_got_font_properties (info))
        return -1;

    /* allocate buffer for bitmap */
    bitmap = (unsigned char*) alloca ((info->fnt_bbox_w + 7)/8 * info->fnt_bbox_h);

    for (i = 0; i < info->nr_glyphs; i++) {
        char line_buf [LINE_BUF_LEN];
        char* body;
        BDF_GLYPH_INFO glyph;

        FIND_LINE (line_buf, fp, "STARTCHAR", body);
        while (1) {
            body = my_getline (line_buf, fp);

            if (strcmp (line_buf, "ENCODING") == 0)
                glyph.encoding = atoi (body);
            else if (strcmp (line_buf, "DWIDTH") == 0)
                sscanf (body, "%d %d", &glyph.adv_x, &glyph.adv_y);
            else if (strcmp (line_buf, "BBX") == 0)
                sscanf (body, "%d %d %d %d", &glyph.bbox_w, &glyph.bbox_h,
                        &glyph.bbox_x, &glyph.bbox_y);
            else if (strcmp (line_buf, "BITMAP") == 0) {
                char* cp;
                char ch_left, ch_right;
                unsigned char* cur_bits = bitmap;
                while (1) {
                    my_getline (line_buf, fp);

                    if (strcmp (line_buf, "ENDCHAR") == 0)
                        goto end_one_glyph;

                    cp = line_buf;

                    ch_left = *cp++;
                    ch_right = *cp++;
                    while (ch_left) {
                        HEX_CHAR_VAL (ch_left);
                        HEX_CHAR_VAL (ch_right);
                        ch_right |= ch_left << 4;

                        *cur_bits++ = ch_right;

                        ch_left = *cp++;
                        ch_right = *cp++;
                    }
                }
            }
        }

end_one_glyph:
        if (info->on_got_one_glyph (info, &glyph, bitmap) < 0) {
            fclose (fp);
            return -1;
        }
    }

    fclose (fp);
    return 0;

}

