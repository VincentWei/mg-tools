/*
** $Id: glyph.c 217 2008-08-25 08:14:50Z weiym $
** 
** glyph.c: handle font glyph file.
**
** Copyright (C) 2007 Feynman Software
**
** Create date: 2007/07/27
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mgutils/mgutils.h>

#include "upf.h"
//#include "upftree.h"

#define VAL_DEBUG
#define FUN_DEBUG
#include "my_debug.h"
//static GLYPHMETRICS def_metrics = {{1, 8, 2, 0, 0, 8, 0}};
static GLYPHMETRICS def_metrics = {{0, 0, 8, 2, 1, 0, 8, 0, 0}};
static unsigned char def_bitmap [] = {0xFE, 0x7F};
static GLYPH def_glyph = {&def_metrics, def_bitmap};

UChar32 utf16le_conv_to_uc32 (const unsigned char* mchar)
{
    UChar16 w1, w2;
    UChar32 wc;

    w1 = MAKEWORD (mchar[0], mchar[1]);

    if (w1 < 0xD800 || w1 > 0xDFFF)
        return w1;

    w2 = MAKEWORD (mchar[2], mchar[3]);

    wc = w1;
    wc <<= 10;
    wc |= (w2 & 0x03FF);
    wc += 0x10000;

    return wc;
}

static void inline fill_def_glyph (GLYPH* glyph)
{
    glyph->data = (unsigned char*) malloc (sizeof(def_bitmap));
    memcpy(glyph->data, def_bitmap, sizeof(def_bitmap));

	glyph->metrics = (GLYPHMETRICS*) malloc(sizeof(GLYPHMETRICS));
	memcpy(glyph->metrics, &def_metrics, sizeof(GLYPHMETRICS));
}

static void inline make_empty_glyph (GLYPH* glyph)
{
    free (glyph->metrics);
    free (glyph->data);
    glyph->metrics = NULL;
    glyph->data = NULL;
}

static UPFGLYPHTREE* create_one_node (unsigned int uc32)
{
    UPFGLYPHTREE* node;
    node = calloc(1, sizeof(UPFGLYPHTREE));

    node->upf_node.min = uc32;
    node->upf_node.max = uc32;

    node->glyph = (GLYPH*)malloc(sizeof(GLYPH));
    fill_def_glyph (node->glyph);

    node->less = NULL;
    node->more = NULL;
    node->parent = NULL;
    return node;
}

static void add_glyph_to_node (UPFGLYPHTREE* node, int uc32)
{
    int old_num = node->upf_node.max - node->upf_node.min + 1;

    if (uc32 == node->upf_node.min - 1 ){
        node->upf_node.min = uc32;

        node->glyph = (GLYPH*) realloc (node->glyph, 
                (old_num+1) * sizeof(GLYPH));
        memmove (node->glyph+1, node->glyph, old_num * sizeof(GLYPH));
        fill_def_glyph (node->glyph);
    }
    else if (node->upf_node.max + 1 == uc32) {
        node->upf_node.max = uc32;

        node->glyph = (GLYPH*) realloc (node->glyph, 
                (old_num+1) * sizeof(GLYPH));
        fill_def_glyph (node->glyph + old_num);
    }

    else {
        assert (0);
    }
}

static void delete_glyph_from_node (UPFGLYPHTREE** node, int uc32)
{
    int pos;
    int left_num;
    GLYPH* all_glyphs;

    assert ((*node)->upf_node.min <=uc32
            && uc32 <= (*node)->upf_node.max);

    left_num = (*node)->upf_node.max - (*node)->upf_node.min;
    all_glyphs = (*node)->glyph;

    TEST_VAL((*node)->upf_node.min, %d);
    TEST_VAL((*node)->upf_node.max, %d);

    pos = uc32 - (*node)->upf_node.min;
    make_empty_glyph ((*node)->glyph + pos);

    if (left_num == 0)
    {
        free_node (node);
        return;
    }

    if ((*node)->upf_node.min == uc32)
    {
        (*node)->upf_node.min ++;
        FUN_STEPS(1);
        (*node)->glyph = (GLYPH*) malloc (left_num * sizeof(GLYPH));
        memcpy ((*node)->glyph, all_glyphs + 1, left_num*sizeof(GLYPH));
        free (all_glyphs);
    }
    else if ((*node)->upf_node.max == uc32)
    {
        (*node)->upf_node.max --;
        FUN_STEPS(1);
        (*node)->glyph = (GLYPH*) malloc (left_num * sizeof(GLYPH));
        memcpy ((*node)->glyph, all_glyphs, left_num*sizeof(GLYPH));
        free (all_glyphs);
    }
    else {
        FUN_STEPS(1);
        divide_node ((*node), uc32);
    }
}

    //FIXME uc32 = conv_to_uc32 ((unsigned char *)&ch);
    
/*return FALSE --- uc32 is in the tree*/
BOOL add_glyph_to_tree(UPFGLYPHTREE *root, int uc32)
{
    assert (root);

    /*less tree*/
    if (uc32 < root->upf_node.min - 1) {
        if (root->less)
            return add_glyph_to_tree (root->less, uc32);
        else {
            root->less = create_one_node (uc32);
            root->less->parent = root;
            return TRUE;
        }
            
    }
    /*more tree*/
    else if (root->upf_node.max + 1 < uc32){
        if (root->more)
            return add_glyph_to_tree (root->more, uc32);
        else {
            root->more = create_one_node (uc32);
            root->more->parent = root;
            return TRUE;
        }
    }
    /*in this node*/
    else if (root->upf_node.min <= uc32 
            && uc32 <= root->upf_node.max) {
        return FALSE;
    }
    /*add to this node*/
    else {
        add_glyph_to_node (root, uc32);
        return TRUE;
    }
}

/*
void add_to_tree_by_encoding (UPFGLYPHTREE* root, int ch)
{
    int uc32;

    uc32 = conv_to_uc32 ((unsigned char *)&ch);
    add_to_tree (root, uc32);
}
*/

/*return FALSE--- there is no uc32 in the tree*/
BOOL delete_glyph_from_tree (UPFGLYPHTREE** root, int uc32)
{
    if (uc32 < (*root)->upf_node.min)
    {
        if ((*root)->less)
            return delete_glyph_from_tree (&(*root)->less, uc32);
        else
            return FALSE;
    }
    else if ((*root)->upf_node.max < uc32)
    {
        if ((*root)->more)
            return delete_glyph_from_tree (&(*root)->more, uc32);
        else
            return FALSE;
    }

    delete_glyph_from_node (root, uc32);
    return TRUE;
}


static UPFGLYPHTREE* find_node_including_glyph (UPFGLYPHTREE* tree, unsigned int uc32)
{
    if (uc32 < tree->upf_node.min) {

        if (!tree->less)
            return NULL;

        return find_node_including_glyph (tree->less, uc32);
    } else if ( uc32 > tree->upf_node.max ) {

        if (!tree->more) {
            return NULL;
        }
        return find_node_including_glyph (tree->more, uc32);
    }

    return tree;
}

static GLYPH* find_glyph_from_tree (UPFGLYPHTREE* tree, unsigned int uc32)
{
    UPFGLYPHTREE* node;
    node = find_node_including_glyph (tree, uc32);
    if (node)
        return node->glyph +(uc32 - node->upf_node.min);
    else
        return NULL;
}

BOOL upf_add_char_glyph (UPFINFO* upf_info, int ch)
{
    unsigned int uc32;
    uc32 = utf16le_conv_to_uc32 ((unsigned char *)&ch);
    return add_glyph_to_tree(upf_info->upf_tree, uc32);
}

BOOL upf_delete_char_glyph (UPFINFO* upf_info, int ch)
{
    unsigned int uc32;
    uc32 = utf16le_conv_to_uc32 ((unsigned char *)&ch);
    return delete_glyph_from_tree (&(upf_info->upf_tree), uc32);

}
/*return 1: make glyph_bitmap to  def_glyph
 * return 0: make glypy_bitmap to realy glyph*/
BOOL upf_get_char_glyph_bitmap (UPFINFO* upf_info, int ch, 
        GLYPHBITMAP* glyph_bitmap)
{
    GLYPH* glyph;
    unsigned int uc32;
    BOOL result;
    int scale = 1;

    if (glyph_bitmap == NULL) return FALSE;

    uc32 = utf16le_conv_to_uc32 ((unsigned char *)&ch);
    glyph = find_glyph_from_tree (upf_info->upf_tree, uc32);

    if (glyph == NULL) {
        //printf("qpf_get_char_glyphbitmap glyph == NULL ------\n");
        glyph = &def_glyph;
        def_metrics.upf_glyph.bearingy = (upf_info->upf_file_header.ascent+2)/2;
        result = FALSE;
    }
    else {
        result = TRUE;
    }

    glyph_bitmap->bbox_x = glyph->metrics->upf_glyph.bearingx * scale;
    glyph_bitmap->bbox_y = glyph->metrics->upf_glyph.bearingy * scale;
    glyph_bitmap->bbox_w = glyph->metrics->upf_glyph.width * scale;
    glyph_bitmap->bbox_h = glyph->metrics->upf_glyph.height * scale;
    glyph_bitmap->advance_x = glyph->metrics->upf_glyph.advance;
    glyph_bitmap->advance_y = 0;
    glyph_bitmap->bmp_pitch = glyph->metrics->upf_glyph.pitch;
    glyph_bitmap->bmp_size = glyph->metrics->upf_glyph.pitch * glyph->metrics->upf_glyph.height;
    glyph_bitmap->bits = glyph->data;

    return result;
}

/*set glyph_bitmap to upf tree
 * if the glyph is not in the font, glyph_bitmap is def_glyph, return*/
void upf_set_char_glyph (UPFINFO* upf_info, int ch, 
        GLYPHBITMAP* glyph_bitmap)
{
    unsigned int uc32;
    GLYPH*          glyph = NULL;
    UPFGLYPHTREE* glyph_tree = NULL;
    int scale = 1;

    uc32 = utf16le_conv_to_uc32 ((unsigned char *)&ch);
    glyph      = find_glyph_from_tree (upf_info->upf_tree, uc32);
    glyph_tree = find_node_including_glyph (upf_info->upf_tree, uc32);

    if (glyph == NULL) {
        return;
    }

    /*
    old_size = glyph->metrics->upf_glyph.pitch * glyph->metrics->upf_glyph.height;
    new_size = glyph_bitmap->bmp_pitch * (glyph_bitmap->bbox_h / scale);

    if (old_size != new_size)
        size_change = TRUE;
    else
        size_change = FALSE;
        */

    //FIXME
    glyph->metrics->upf_glyph.pitch  = glyph_bitmap->bmp_pitch;
    glyph->metrics->upf_glyph.width  = glyph_bitmap->bbox_w / scale;
    glyph->metrics->upf_glyph.height = glyph_bitmap->bbox_h / scale;

    glyph->metrics->upf_glyph.bearingx  = glyph_bitmap->bbox_x / scale;
    glyph->metrics->upf_glyph.advance   = glyph_bitmap->advance_x;
    glyph->metrics->upf_glyph.bearingy  = glyph_bitmap->bbox_y / scale;

    /*
    if (size_change)
    {
        free (glyph->data);
        glyph->data = (unsigned char* ) malloc (glyph->metrics->upf_glyph.pitch
                * glyph->metrics->upf_glyph.height)
    }
    */
    glyph->data  = (unsigned char *)glyph_bitmap->bits;
}

void glyph_free (GLYPHBITMAP* glyph)
{
    if (glyph == NULL)
        return;
    if (glyph->bits)
        free((void*)glyph->bits);
    glyph->bits = NULL;
    return;
}

BOOL glyph_get_bit(GLYPHBITMAP* glyph_bitmap, int row, int col)
{
    unsigned char* bits = (unsigned char*)glyph_bitmap->bits;
    int k = col % 8;

    bits += row * glyph_bitmap->bmp_pitch;
    bits += (col / 8);

    return (*bits & (0x80 >> k));
}

void glyph_set_bit(GLYPHBITMAP* glyph_bitmap, int row, int col, BOOL value)
{
    unsigned char* bits = (unsigned char*)glyph_bitmap->bits;
    int k = col % 8;

    bits += row * glyph_bitmap->bmp_pitch;
    bits += (col / 8);
    if (value)
        *bits |= (0x80 >> k);
    else
        *bits &= ~(0x80 >> k);
}

void glyph_move_left(GLYPHBITMAP* glyph_bitmap)
{
    int i, j;
    BOOL value;
    BOOL* first_col;
    int width  = glyph_bitmap->bbox_w;
    int height = glyph_bitmap->bbox_h;

    first_col = malloc(sizeof(BOOL) * height);

    for (i = 0; i<height; i++)
        first_col[i] = glyph_get_bit(glyph_bitmap, i, 0);

    for (i = 0; i<height; i++) {
        for (j = 0; j<width - 1; j++) {
            value = glyph_get_bit(glyph_bitmap, i, j+1);
            glyph_set_bit(glyph_bitmap, i, j, value);
        }
    }

    for (i = 0; i<height; i++)
        glyph_set_bit(glyph_bitmap, i, width-1, first_col[i]);

    free(first_col);
}

void glyph_move_right(GLYPHBITMAP* glyph)
{
    int i, j;
    BOOL value;
    BOOL* last_col;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    last_col = malloc(sizeof(BOOL)*height);
    for (i = 0; i<height; i++)
        last_col[i] = glyph_get_bit(glyph, i, width-1);
    for (i = 0; i<height; i++)
    {
        for (j=width-1; j>0; j--)
        {
            value = glyph_get_bit(glyph, i, j-1);
            glyph_set_bit(glyph, i, j, value);
        }
    }
    for (i = 0; i<height; i++)
        glyph_set_bit(glyph, i, 0, last_col[i]);
    free(last_col);
}

void glyph_move_up(GLYPHBITMAP* glyph_bitmap)
{
    int i, j;
    BOOL value;
    BOOL* first_row;
    int width  = glyph_bitmap->bbox_w;
    int height = glyph_bitmap->bbox_h;

    first_row = malloc(sizeof(BOOL)*width);

    for (j = 0; j<width; j++)
        first_row[j] = glyph_get_bit(glyph_bitmap, 0, j);

    for (i = 0; i<height - 1; i++) {
        for (j = 0; j<width; j++) {
            value = glyph_get_bit(glyph_bitmap, i+1, j);
            glyph_set_bit(glyph_bitmap, i, j, value);
        }
    }

    for (j = 0; j<width; j++)
        glyph_set_bit(glyph_bitmap, height-1, j, first_row[j]);

    free(first_row);
}

void glyph_move_down(GLYPHBITMAP* glyph)
{
    int i, j;
    BOOL value;
    BOOL* last_row;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;
    last_row = malloc(sizeof(BOOL)*width);
    for (j = 0; j<width; j++)
        last_row[j] = glyph_get_bit(glyph, height-1, j);

    for (i = height-1; i > 0; i--)
    {
        for (j = 0; j<width; j++)
        {
            value = glyph_get_bit(glyph, i-1, j);
            glyph_set_bit(glyph, i, j, value);
        }
    }
    for (j = 0; j<width; j++)
        glyph_set_bit(glyph, 0, j, last_row[j]);
    free(last_row);
}

void glyph_draw(GLYPHBITMAP* glyph, HDC hdc, int x, int y, int pixel_size, gal_pixel color)
{
    int i, j;
    int pixel_x, pixel_y;
    int width = glyph->bbox_w;
    int height = glyph->bbox_h;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            pixel_x = x + pixel_size * (glyph->bbox_x + j);
            pixel_y = y + pixel_size * (-glyph->bbox_y + i);
            if (pixel_size == 1)
            {
                if(glyph_get_bit(glyph, i, j))
                    SetPixel(hdc, pixel_x, pixel_y, color);
                else
                    SetPixel(hdc, pixel_x, pixel_y, PIXEL_lightwhite);
            }
            else
            {
                if(glyph_get_bit(glyph, i, j))
                    SetBrushColor(hdc, color);
                else
                    SetBrushColor(hdc, PIXEL_lightwhite);
                FillBox (hdc, pixel_x + 1, pixel_y + 1, pixel_size - 1, pixel_size - 1);
            }
        }
    }
}

void glyph_bitmap_copy(GLYPHBITMAP* to, GLYPHBITMAP* from)
{
    int min_width = MIN(to->bbox_w, from->bbox_w);
    int min_height = MIN(to->bbox_h, from->bbox_h);
    int i,j;

    memset((unsigned char*)to->bits, 0, to->bmp_size);
    for (i = 0; i<min_height; i++)
    {
        for (j = 0; j<min_width;j++)
            glyph_set_bit(to, i, j, glyph_get_bit(from, i, j));
    }
}

void glyph_change_width_right(GLYPHBITMAP* glyph_bitmap, int width)
{
    GLYPHBITMAP old_glyph_bitmap;
    int height;

    if(width == 0 || glyph_bitmap == NULL) return;

    height = glyph_bitmap->bbox_h;
    memcpy(&old_glyph_bitmap, glyph_bitmap, sizeof(GLYPHBITMAP));
    glyph_bitmap->advance_x = glyph_bitmap->advance_x + width;
	
    glyph_bitmap->bmp_pitch
	   	= (glyph_bitmap->bmp_pitch / glyph_bitmap->bbox_w) * (glyph_bitmap->bbox_w + width);

    glyph_bitmap->bbox_w = glyph_bitmap->bbox_w + width;
    glyph_bitmap->bmp_pitch = (glyph_bitmap->bbox_w + 7) >> 3;
    glyph_bitmap->bmp_size = glyph_bitmap->bmp_pitch*height;


	//FIXME
    glyph_bitmap->bits = calloc(1, glyph_bitmap->bmp_size);
    glyph_bitmap_copy(glyph_bitmap, &old_glyph_bitmap);
    glyph_free(&old_glyph_bitmap);
}

void glyph_change_width_left(GLYPHBITMAP* glyph_bitmap, int width)
{
    GLYPHBITMAP old_glyph_bitmap;
    int height;
	
    if(width == 0) return;

    height = glyph_bitmap->bbox_h;
    memcpy(&old_glyph_bitmap, glyph_bitmap, sizeof(GLYPHBITMAP));

    glyph_move_left(&old_glyph_bitmap);

    glyph_bitmap->bbox_w    = glyph_bitmap->bbox_w + width;
    glyph_bitmap->bbox_x++;
    glyph_bitmap->advance_x = glyph_bitmap->advance_x + width;
    glyph_bitmap->bmp_pitch = (glyph_bitmap->bbox_w + 7) >> 3;
    glyph_bitmap->bmp_size  = glyph_bitmap->bmp_pitch * height;
    glyph_bitmap->bits      = calloc(1, glyph_bitmap->bmp_size);

    glyph_bitmap_copy(glyph_bitmap, &old_glyph_bitmap);
    glyph_free(&old_glyph_bitmap);
}

void glyph_change_height_bottom (GLYPHBITMAP* glyph_bitmap, int height)
{
    GLYPHBITMAP old_glyph_bitmap;

    if(height == 0) return;

    memcpy(&old_glyph_bitmap, glyph_bitmap, sizeof(GLYPHBITMAP));
    glyph_bitmap->bbox_h = glyph_bitmap->bbox_h + height;
    glyph_bitmap->bmp_size = glyph_bitmap->bmp_pitch * glyph_bitmap->bbox_h;
	//FIXME
    glyph_bitmap->bits = calloc(1, glyph_bitmap->bmp_size);
    glyph_bitmap_copy(glyph_bitmap, &old_glyph_bitmap);
    glyph_free(&old_glyph_bitmap);
}

void glyph_change_height_top(GLYPHBITMAP* glyph_bitmap, int height)
{
    GLYPHBITMAP old_glyph;

    if(height == 0) return;

    memcpy(&old_glyph, glyph_bitmap, sizeof(GLYPHBITMAP));
    glyph_move_up(&old_glyph);

    glyph_bitmap->bbox_h   = glyph_bitmap->bbox_h + height;
    glyph_bitmap->bbox_y--;
    glyph_bitmap->bmp_size = glyph_bitmap->bmp_pitch * glyph_bitmap->bbox_h;
    glyph_bitmap->bits     = calloc(1, glyph_bitmap->bmp_size);
    glyph_bitmap_copy(glyph_bitmap, &old_glyph);

    glyph_free(&old_glyph);
}
