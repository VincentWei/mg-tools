
/*
** TODO:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "vbf.h"

/********************** Load/Unload of var bitmap font ***********************/

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
static void swap_intdata (Uint32* data, int num)
{
    while (num)
    {
        *data = ArchSwap32 (*data);
        data++;
        num--;
    }

}
#endif

static BOOL LoadVarBitmapFont (const char* file, MYVBFINFO* info)
{
    FILE* fp = NULL;

    char version[LEN_VERSION_INFO + 1];
    char vender[LEN_VENDER_INFO+1];

    FILE_LAYOUT layout;
    const FONT_PROPT* propt;
    char* temp = NULL;
    Uint16 len_header;

    if (!(fp = fopen (file, "rb")))
        return FALSE;

    if (fread (version, 1, LEN_VERSION_INFO, fp) < LEN_VERSION_INFO)
        goto error;
    version [LEN_VERSION_INFO] = '\0'; 

    if (strcmp (version, VBF_VERSION3) != 0)
    {
        fprintf (stderr, "Error on loading vbf: %s, version: %s,"
                         " invalid version.\n", file, version);
        goto error;
    }

    if (fread(vender, 1, LEN_VENDER_INFO, fp) < LEN_VENDER_INFO)
        goto error;

    if (fread(&len_header, sizeof(short), 1, fp) < 1)
        goto error;

    if (fread (&layout, sizeof(FILE_LAYOUT), 1, fp) < 1)
        goto error;

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    len_header = ArchSwap16 (len_header);
    swap_intdata ((Uint32*)&layout, sizeof(layout)/sizeof(int));
#endif

#ifdef HAVE_MMAP
    if ((temp = mmap (NULL, layout.font_size, PROT_READ, MAP_SHARED, 
            fileno(fp), 0)) == MAP_FAILED)
        goto error;
    temp += len_header;
    printf ("layout.font_size=%d\n", layout.font_size);
#else
    layout.font_size -= len_header;
    if ((temp = (char *)malloc (layout.font_size)) == NULL)
        goto error;
    if (fread (temp, sizeof (char), layout.font_size, fp) < layout.font_size)
        goto error;
#endif

    fclose (fp);
    propt = (const FONT_PROPT*) temp;
    
    printf ("layout.font_size=%d\n", layout.font_size);

    strcpy (info->ver_info, "3.0");
    info->name = (char*)propt;
    info->max_width = propt->max_width;
    info->ave_width = propt->ave_width;
    info->height = propt->height;
    info->descent = propt->descent;

    info->font_size = layout.font_size;

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    info->first_glyph = ArchSwap32 (propt->first_glyph);
    info->last_glyph = ArchSwap32 (propt->last_glyph);
    info->def_glyph = ArchSwap32 (propt->def_glyph);
#else
    info->first_glyph = propt->first_glyph;
    info->last_glyph = propt->last_glyph;
    info->def_glyph = propt->def_glyph;
#endif

    info-> bbox = (const VBF_BBOX*)
            (layout.len_bboxs ? temp+layout.off_bboxs-HEADER_LEN : NULL);
    info-> advance_x = (const char*)
            (layout.len_advxs ? temp+layout.off_advxs-HEADER_LEN : NULL);
    info-> advance_y = (const char*)
            (layout.len_advys ? temp+layout.off_advys-HEADER_LEN : NULL);
    info-> bits_offset = (const Uint32*)
            (layout.len_bit_offs ? temp+layout.off_bit_offs-HEADER_LEN : NULL);

    if (layout.len_bits <= 0)
        goto error;
    info-> all_glyph_bits = (const unsigned char*)(temp + 
                            layout.off_bits-HEADER_LEN);

    return TRUE;

error:
#ifdef HAVE_MMAP
    if (temp)
        munmap (temp, layout.font_size);
#else 
    free (temp);
#endif
    fclose (fp);
    return FALSE;
}

static void UnloadVarBitmapFont (MYVBFINFO* info)
{
#ifdef HAVE_MMAP
    if (info->name)
        munmap ((void*)(info->name) - HEADER_LEN, info->font_size);
#else
    free ((void*)info->name);
#endif
}


static MYVBFINFO myvbfinfo;

BOOL loadVBF (VBFINFO* vbfinfo, const char* file)
{
    int    n, i;

    if(!LoadVarBitmapFont (file, &myvbfinfo))
        return FALSE;
    
    memcpy (vbfinfo->ver_info,(const void*) myvbfinfo.ver_info, 4);

    vbfinfo->name        = myvbfinfo.name;
    vbfinfo->max_width   = myvbfinfo.max_width;
    vbfinfo->ave_width   = myvbfinfo.ave_width;
    vbfinfo->height      = myvbfinfo.height;
    vbfinfo->descent     = myvbfinfo.descent;
    vbfinfo->first_glyph = myvbfinfo.first_glyph;
    vbfinfo->last_glyph  = myvbfinfo.last_glyph;
    vbfinfo->def_glyph   = myvbfinfo.def_glyph;
    vbfinfo->font_size   = myvbfinfo.font_size;

    n= vbfinfo->last_glyph - vbfinfo->first_glyph+1 ;
    /***/ 
    if (myvbfinfo.bbox)
    {
        VBF_BBOX *p;
        
        p = malloc (sizeof (VBF_BBOX));
        vbfinfo->bbox = p;
        p->next = NULL;
        

        for (i=0; i<n; i++)
        {
           p->x = myvbfinfo.bbox[i].x; 
           p->y = myvbfinfo.bbox[i].y;  
           p->w = myvbfinfo.bbox[i].w;  
           p->h = myvbfinfo.bbox[i].h;  
           
           if (i<n)
           {
              p->next = malloc (sizeof (VBF_BBOX));
              p->next->next = NULL;
              p= p->next;
           }

        }
    }else
        vbfinfo->bbox = NULL;
    /***/
    
    if (myvbfinfo.advance_x)
    {
        ADVANCE_X *p;
        
        p = malloc (sizeof (ADVANCE_X));
        vbfinfo->advance_x =p;
        p->next = NULL;
        
        for (i=0; i<n; i++)
        {
          p->advance_x = myvbfinfo.advance_x[i];

          if (i<n)
           {
              p->next = malloc (sizeof (ADVANCE_X));
              p= p->next;
              p->next =NULL;
           }

        }
    }else
        vbfinfo->advance_x =NULL;
    
    /***/
    
    if (myvbfinfo.advance_y)
    {
        ADVANCE_Y *p;
        p = malloc (sizeof (ADVANCE_Y));
        vbfinfo->advance_y =p;
        p->next = NULL;
        
        for (i=0; i<n; i++)
        {
         p->advance_y = myvbfinfo.advance_y[i];
    
           if (i<n)
           {
              p->next = malloc (sizeof (ADVANCE_Y));
              p= p->next;
              p->next = NULL;
           }

        }
    }else
        vbfinfo->advance_y = NULL;
    /***/
    
    if (myvbfinfo.all_glyph_bits) 
    {
        GLYPH_BITS * p;
        p = malloc (sizeof (GLYPH_BITS));
        vbfinfo->all_glyph_bits =p;
        p->next =NULL;
     
        for (i=0; i<n; i++)
        {
           int w,h;
        
           if (vbfinfo->bbox)
           {
              w = myvbfinfo.bbox[i].w; 
              h = myvbfinfo.bbox[i].h; 
           }
           else if (vbfinfo->advance_x)      
           {
              w = myvbfinfo.advance_x[i];
              h = myvbfinfo.height;
           }
           else 
           {
              w = myvbfinfo.max_width;
              h = myvbfinfo.height;
           }

           p->bmp_size = h*((w+7)>>3);
           p->glyph_bits = malloc (p->bmp_size);
            
           if (myvbfinfo.bits_offset[i]==myvbfinfo.bits_offset[myvbfinfo.def_glyph])
                p->is_default_glyph = TRUE;
            else
                p->is_default_glyph = FALSE;
        
           memcpy (p->glyph_bits,(const unsigned char *)(myvbfinfo.all_glyph_bits+myvbfinfo.bits_offset[i]), p->bmp_size );
    
           if (i<n)
           {
              p->next = malloc (sizeof (GLYPH_BITS));
              p= p->next;
              p->next =NULL;
              p->glyph_bits =NULL;
           }

        }

    }else
        vbfinfo->all_glyph_bits = NULL;
    
    /***/ 
    return TRUE;
}

void freeVBF (VBFINFO* info)
{
    
    //free ((void*)info->name);
    if(info->bbox){
        VBF_BBOX *temp_cur =NULL,*temp_next =NULL;

        temp_cur=info->bbox;
        temp_next =temp_cur->next;
        info->bbox =NULL;
        free(temp_cur);
        while(temp_next){
            temp_cur =temp_next;
            temp_next =temp_cur->next;
            free(temp_cur);
        }
    }

    if(info->advance_x){
        ADVANCE_X *temp_cur =NULL, *temp_next =NULL;

        temp_cur=info->advance_x;
        temp_next =temp_cur->next;
        info->advance_x =NULL;
        free(temp_cur);
        while(temp_next){
            temp_cur =temp_next;
            temp_next =temp_cur->next;
            free(temp_cur);
        }
    }
    
    if(info->advance_y){
        ADVANCE_Y *temp_cur =NULL, *temp_next =NULL;

        temp_cur=info->advance_y;
        temp_next =temp_cur->next;
        info->advance_y =NULL;
        free(temp_cur);
        while(temp_next){
            temp_cur =temp_next;
            temp_next =temp_cur->next;
            free(temp_cur);
        }
    }
    
    if(info->all_glyph_bits){
        GLYPH_BITS *temp_cur =NULL, *temp_next =NULL;

        temp_cur=info->all_glyph_bits;
        temp_next =temp_cur->next;
        info->all_glyph_bits =NULL;
        free(temp_cur->glyph_bits);
        free(temp_cur);

        while(temp_next){
            temp_cur  =temp_next;
            temp_next =temp_cur->next;
            free(temp_cur->glyph_bits);
            free(temp_cur);
        }
    }
    UnloadVarBitmapFont (&myvbfinfo);
}


