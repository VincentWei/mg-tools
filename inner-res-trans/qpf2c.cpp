#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "qpf2c.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

    typedef unsigned char Uint8;
    typedef signed char Sint8;
    typedef int BOOL;
    typedef unsigned char uchar;

#define FALSE   0
#define TRUE    1 


#ifdef __NOUNIX__
    typedef struct _QPF_GLYPHMETRICS 
#else
    typedef struct __attribute__ ((packed)) _QPF_GLYPHMETRICS 
#endif
    {
        Uint8 linestep;
        Uint8 width;
        Uint8 height;
        Uint8 padding;

        Sint8 bearingx;     /* Difference from pen position to glyph's left bbox */
        Uint8 advance;      /* Difference between pen positions */
        Sint8 bearingy;     /* Used for putting characters on baseline */

        Sint8 reserved;     /* Do not use */
    } QPF_GLYPHMETRICS;

    typedef struct _QPF_GLYPH
    {
        const QPF_GLYPHMETRICS* metrics;
        const unsigned char* data;
    } QPF_GLYPH;

    typedef struct _QPF_GLYPHTREE
    {
        unsigned int min, max;
        int less_tree_index;
        int more_tree_index;
        int glyph_index;
    } QPF_GLYPHTREE;

#ifdef __NOUNIX__
    typedef struct _QPFMETRICS
#else
    typedef struct __attribute__ ((packed)) _QPFMETRICS
#endif
    {
        Sint8 ascent, descent;
        Sint8 leftbearing, rightbearing;
        Uint8 maxwidth;
        Sint8 leading;
        Uint8 flags;
        Uint8 underlinepos;
        Uint8 underlinewidth;
        Uint8 reserved3;
    } QPFMETRICS;

    typedef struct 
    {
        unsigned int height;
        unsigned int width;

        unsigned int file_size;
        QPFMETRICS* fm;

        QPF_GLYPHTREE* tree;
    } QPFINFO;

    typedef struct _ARRAY{
        void* array;
        int item_size;
        int item_num;
        int alloced_num;
    } ARRAY;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#define LINE_BYTE_NUM 8
static inline long get_file_size (char* filename)
{
    FILE* fp = fopen (filename, "rb");
    if (fp == NULL)
        return 0;

    fseek (fp, 0, SEEK_END);
    long len = ftell (fp);
	fclose(fp);
	return len;
}

static inline BOOL init_array (ARRAY* array, int item_size, int item_num)
{
    array->array = malloc (item_size * item_num);
    if (! array->array)
        return FALSE;

    array->item_size = item_size;
    array->item_num = item_num;
    array->alloced_num = 0;
    return TRUE;
}

static inline BOOL reinit_array (ARRAY* array, int item_num)
{
    array->array = realloc (array->array, array->item_size * item_num);
    if (! array->array)
        return FALSE;

    array->item_num = item_num;
    return TRUE;
}

static inline void destroy_array (ARRAY* array)
{
    free (array->array);
    array->array = NULL;
    array->item_num = 0;
    array->alloced_num = 0;
}

static inline int alloc_from_array (ARRAY* array, int num)
{
    int tmp;
    if (array->alloced_num == array->item_num) {
        array->item_num = array->item_size * 2 + num;
        if (reinit_array (array, array->item_size) == FALSE)
            return -1;
    }

    tmp = array->alloced_num;
    array->alloced_num += num;
    return tmp;
}

static inline void* get_item (ARRAY* array, int index)
{
    if (index >= array->alloced_num)
        return NULL;

    return (char*)array->array + array->item_size * index;
}

static int read_node (ARRAY* a_node, ARRAY* a_glyph, uchar** data)
{
    uchar rw, cl;
    int flags;
    QPF_GLYPHTREE* root;
    int root_index;

    root_index = alloc_from_array (a_node, 1);
    root = (QPF_GLYPHTREE*)get_item (a_node, root_index);

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    root->min = (rw << 8) | cl;

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    root->max = (rw << 8) | cl;

    root->glyph_index = alloc_from_array (a_glyph, root->max-root->min+1);

    flags = **data; (*data)++;

    if (flags & 1)
        root->less_tree_index = read_node (a_node, a_glyph, data);
    else
        root->less_tree_index = -1;

    if (flags & 2)
        root->more_tree_index = read_node (a_node, a_glyph, data);
    else
        root->more_tree_index = -1;

    return root_index;
}

static void read_metrics (ARRAY* a_node, int node_index,
       ARRAY* a_glyph, uchar** data)
{
    int i;
    int n;
    int glyph_index;
    QPF_GLYPHTREE* node;
    QPF_GLYPH* glyph;

    if (node_index == -1)
        return;

    node = (QPF_GLYPHTREE* ) get_item (a_node, node_index);
    n = node->max - node->min + 1;
    glyph_index = node->glyph_index;

    for (i = 0; i < n; i++) {
        glyph = (QPF_GLYPH*) get_item (a_glyph, glyph_index+i);
        glyph->metrics = (QPF_GLYPHMETRICS*) *data;

        *data += sizeof (QPF_GLYPHMETRICS);
    }

    read_metrics (a_node, node->less_tree_index, a_glyph, data);
    read_metrics (a_node, node->more_tree_index, a_glyph, data);
}

static void read_data (ARRAY* a_node, int node_index,
       ARRAY* a_glyph, uchar** data)
{
    int i;
    int n;
    int glyph_index;
    QPF_GLYPHTREE* node;
    QPF_GLYPH* glyph;

    if (node_index == -1)
        return;

    node = (QPF_GLYPHTREE* ) get_item (a_node, node_index);
    n = node->max - node->min + 1;
    glyph_index = node->glyph_index;

    for (i = 0; i < n; i++) {
        glyph = (QPF_GLYPH*) get_item (a_glyph, glyph_index+i);
        glyph->data =  *data;

        *data += glyph->metrics->linestep * glyph->metrics->height;
    }

    read_data (a_node, node->less_tree_index, a_glyph, data);
    read_data (a_node, node->more_tree_index, a_glyph, data);
}


static void build_glyph_tree (ARRAY* a_node, ARRAY* a_glyph, 
        unsigned char** data)
{
    read_node (a_node, a_glyph, data);
    read_metrics (a_node, 0, a_glyph, data);
    read_data (a_node, 0, a_glyph, data);
}

static void printf_data_array (FILE* fp_c, char* prefix, uchar* data, int data_size)
{
    int i;
    int j;
    int left;
    fprintf (fp_c, "static unsigned char %s_data [ ] = {\n", prefix);
    for (i=0; i<data_size; i+=LINE_BYTE_NUM)
    {
        fprintf (fp_c, "\t");
        for (j=0; j<LINE_BYTE_NUM; j++)
        {
            fprintf (fp_c, "0x%.2x, ", *data);
            data++;
        }
        fprintf (fp_c, "\n");
    }

    left = data_size % LINE_BYTE_NUM;

    fprintf (fp_c, "\t");
    for (i=0; i<left; i++)
    {
        fprintf (fp_c, "0x%x, ", *data);
        data++;
    }

    fprintf (fp_c, "};\n\n");
}

static void printf_glyph_array (FILE* fp_c, char* prefix, ARRAY* a_glyph, uchar* data)
{
    int i;
    QPF_GLYPH* glyph;

    fprintf (fp_c, "static QPF_GLYPH %s_glyph [ ] = {\n", prefix);
    for (i=0; i<a_glyph->alloced_num; i++)
    {
        glyph = (QPF_GLYPH*) get_item (a_glyph, i);

        fprintf (fp_c, "\t{(QPF_GLYPHMETRICS*)(%s_data + %d), %s_data + %d},\n", 
                prefix, ((uchar*)glyph->metrics) - data,
                prefix, glyph->data - data);

    }
    fprintf (fp_c, "};\n\n");
}

static void printf_node_array (FILE* fp_c, char* prefix, ARRAY* a_node)
{
    int i;
    QPF_GLYPHTREE* node;
    fprintf (fp_c, "static QPF_GLYPHTREE %s_tree [ ] = {\n", prefix);

    for (i=0; i<a_node->alloced_num; i++)
    {
        node = (QPF_GLYPHTREE*) get_item (a_node, i);

        fprintf (fp_c, "\t{%d, %d, ", node->min, node->max);

        if (node->less_tree_index == -1 )
            fprintf (fp_c, "NULL, ");
        else
            fprintf (fp_c, "%s_tree + %d, ", prefix, node->less_tree_index);

        if (node->more_tree_index == -1 )
            fprintf (fp_c, "NULL, ");
        else
            fprintf (fp_c, "%s_tree + %d, ", prefix, node->more_tree_index);

        fprintf (fp_c, "%s_glyph + %d},\n", prefix, node->glyph_index);
    }

    fprintf (fp_c, "};\n\n");
}

static void printf_qpfinfo (FILE* fp_c, char* prefix, int height, int width)
{
    fprintf (fp_c, "QPFINFO %s[1] = {{\n", prefix);
    fprintf (fp_c, "\t%d,\n"
            "\t%d,\n"
            "\t%d,\n"
            "\t(QPFMETRICS*) %s_data,\n"
            "\t%s_tree,\n",
            width, height, 0,
            prefix, prefix);
    fprintf (fp_c, "}};\n");
    
}

bool QPF2C::translate(const char* infile, const char* outfile,const char* varname, const char** argv, int argc)
{
    FILE* fp;
    FILE* fp_c;
    ARRAY a_glyph;
    ARRAY a_node;
    unsigned char* data;
    unsigned char* real_data;
    QPFMETRICS* font_metrics;

    long file_size = get_file_size ((char*)infile);
    if (file_size <=0)
        return false;

    data = (unsigned char*) malloc (file_size);
    init_array (&a_glyph, sizeof(QPF_GLYPH), file_size/sizeof(QPF_GLYPH));
    init_array (&a_node, sizeof(QPF_GLYPHTREE), file_size/sizeof(QPF_GLYPHTREE));

    fp = fopen (infile, "rb");
    fp_c = fopen (outfile, "w+");

    fread (data, 1, file_size, fp);
    font_metrics = (QPFMETRICS*)data;
    real_data = data + sizeof(QPFMETRICS);

    build_glyph_tree (&a_node, &a_glyph, &real_data);

	fprintf(fp_c, "\n#include \"%s\"\n", CFGFILE);
	fprintf(fp_c, "\n#ifdef %s\n\n", MARCOR);

	fprintf(fp_c, "#include \"qpf.h\"\n");

    printf_data_array (fp_c, (char*)varname, data, file_size);

    printf_glyph_array (fp_c, (char*)varname, &a_glyph, data);
    printf_node_array (fp_c, (char*)varname, &a_node);
    printf_qpfinfo (fp_c, (char*)varname, font_metrics->maxwidth, 
            font_metrics->ascent + font_metrics->descent);

	destroy_array(&a_node);
	destroy_array(&a_glyph);

	fprintf(fp_c, "\n#endif //%s\n\n", MARCOR);

    fclose (fp_c);
	fclose (fp);
	return true;

    return TRUE;
}

const char* QPF2C::_support_list = "qpf\0";

