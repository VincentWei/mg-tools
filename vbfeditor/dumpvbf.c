#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "vbf.h"

extern int my_len_bits;
/*
static BOOL fontGetFamilyFromName (const char* name, char* family)
{
    int i = 0;
    const char* family_part;

    if ((family_part = strchr (name, '-')) == NULL)
        return FALSE;
    if (*(++family_part) == '\0')
        return FALSE;

    while (family_part [i] && i <= LEN_FONT_NAME) {
        if (family_part [i] == '-') {
            family [i] = '\0';
            break;
        }

        family [i] = family_part [i];
        i++;
    }

    return TRUE;
}
static BOOL fontGetStyleFromName (const char* name, char* style)
{
    int i = 0;
    const char* part;

    if ((part = strchr (name, '-')) == NULL)
        return FALSE;
    if (*(++part) == '\0')
        return FALSE;

    if ((part = strchr (part, '-')) == NULL)
        return FALSE;
    if (*(++part) == '\0')
        return FALSE;

    while (part [i] && i <= LEN_FONT_NAME) {
        if (part [i] == '-') {
            style [i] = '\0';
            break;
        }

        style [i] = part [i];
        i++;
    }

    return TRUE;
}
*/
/*VBFCHARINFO* get_char_info(const VBFINFO* vbf, int ch)
{
    if(ch < vbf->first_char)
        return NULL;
    if(ch > vbf->last_char)
        return NULL;
    return & vbf->char_info[ch - vbf->first_char];
}
static VBF_BBOX *vbf_get_bbox(const VBFINFO* vbf, int ch)
{
    VBF_BBOX *temp =NULL;
    int i;

    if(!vbf->bbox || ch <vbf->first_glyph || ch >vbf->last_glyph)
        return  NULL;

    temp =vbf->bbox;
    if(ch ==vbf->first_glyph)
        return temp;

    for(i=1;i<=ch -vbf->first_glyph && temp;i++){
        temp = temp->next;
    }

    return temp;
}
*/
static GLYPH_BITS *vbf_get_glyph_bmp_bits(const VBFINFO* vbf, int ch)
{
    GLYPH_BITS *temp =NULL;
    int i;

    if(!vbf->all_glyph_bits || ch <vbf->first_glyph || ch >vbf->last_glyph)
        return  NULL;

    temp =vbf->all_glyph_bits;
    if(ch ==vbf->first_glyph)
        return temp;

    for(i=1;i<=ch -vbf->first_glyph && temp;i++){
        temp =temp->next;
    }

    if(temp)
        return temp;
    else
        return NULL;
}
/*
static int vbf_get_width(const VBFINFO* vbf, int ch)
{
    if(ch < vbf->first_glyph || ch > vbf->last_glyph)
        return 0;

    if(vbf->bbox){
        VBF_BBOX *temp =NULL;
        int i;

        temp =vbf->bbox;
        if(ch ==vbf->first_glyph)
            return temp->w;
        for(i=1;i<=ch -vbf->first_glyph && temp !=NULL;i++){
            temp =temp->next;
        }
        if(!temp)
            return 0;

        return temp->w;
    }

    if(vbf->advance_x){
        ADVANCE_X *temp =NULL;
        int i;

        temp =vbf->advance_x;
        if(ch ==vbf->first_glyph)
            return temp->advance_x;

        for(i=1;i<ch -vbf->first_glyph && temp !=NULL;i++){
            temp =temp->next;
        }
        if(!temp)
            return 0;

        return temp->advance_x;
    }

    return vbf->max_width;
}
*/

static char vbf_get_advance_x(const VBFINFO* vbf_info, int ch)
{
    ADVANCE_X *temp =NULL;
    int i;

    if(ch < vbf_info->first_glyph || ch > vbf_info->last_glyph)
        return 0;

    temp =vbf_info->advance_x;
    if(ch ==vbf_info->first_glyph)
        return temp->advance_x;

    for(i=1;i<ch -vbf_info->first_glyph && temp !=NULL;i++){
        temp =temp->next;
    }
    if(!temp)
        return 0;

    return temp->advance_x;
}


int get_pitch(int width)
{
    return (width+7) >> 3;
}

int get_bits_length(int width, int height)
{
    return get_pitch(width) * height;
}

int get_char_bits_length(const VBFINFO* vbf, int ch)
{
    //return get_bits_length(get_char_info(vbf, ch)->width, vbf->height);
    return get_bits_length(vbf_get_advance_x(vbf,ch), vbf->height);
}

int get_same_char(const VBFINFO* vbf, int ch)
{
    int i;
    for(i = vbf->first_glyph; i<ch; i++)
    {
        int length;
        int length2;
        length = get_char_bits_length(vbf, ch);
        length2 = ((vbf_get_advance_x(vbf,i) + 7) >> 3)*vbf->height;
        if(length != length2)
            continue;
        //if( ! memcmp(vbf->char_info[i-vbf->first_glyph].bits, vbf->char_info[ch-vbf->first_glyph].bits, length))
        if( ! memcmp(vbf_get_glyph_bmp_bits(vbf,i)->glyph_bits, vbf_get_glyph_bmp_bits(vbf,vbf->first_glyph)->glyph_bits, length))
            return i;
    }
    return -1;
}

void get_offset_array(const VBFINFO* vbf, unsigned short* offsets)
{
    int ch;
    int next_offset = 0;
    memset(offsets, 0, sizeof(char)*256);
    for(ch=vbf->first_glyph; ch <= vbf->last_glyph; ch++)
    {
        int same_ch = get_same_char(vbf, ch);
        if(same_ch >= 0)
        {
            offsets[ch] = offsets[same_ch];
            continue;
        }
        offsets[ch] = next_offset;
        //next_offset += ((vbf->char_info[ch-vbf->first_glyph].width + 7) >> 3) * vbf->height;
        next_offset += ((vbf_get_advance_x(vbf,ch) + 7) >> 3) * vbf->height;
    }
}

BOOL dumpVBF (const VBFINFO* vbf, char* file)
{
    FILE*     fp;
    char      str[20];
    static    FILE_LAYOUT filelayout;
    static    FONT_PROPT  font_propt;
    unsigned  short len_header = 68;
    unsigned  int   off_filelayout;
    unsigned  int   off_filename;
    unsigned  int   *bits_offset;
    unsigned  int   off_bits_offset;
    int n,i;
    
    memset (&filelayout, 0, sizeof (FILE_LAYOUT));
    
    fp = fopen (file, "w+");
    sprintf (str, "%s","vbf-3.0**");
    fwrite (str, 1, LEN_VERSION_INFO, fp); 
    sprintf (str, "%s","FMSoft*****");
    fwrite (str, 1, LEN_VENDER_INFO, fp); 
    fwrite (&len_header, sizeof (unsigned short), 1, fp); 
    
    n = vbf->last_glyph - vbf->first_glyph+1;

    off_filelayout = ftell (fp); 
    
    fwrite (&filelayout, sizeof (FILE_LAYOUT), 1, fp);
    off_filename = ftell (fp); 
    
    sprintf (font_propt.font_name, "%s",vbf->name);
    font_propt.max_width =vbf->max_width;
    font_propt.ave_width =vbf->ave_width;
    font_propt.height    =vbf->height;
    font_propt.descent   =vbf->descent;
    font_propt.first_glyph = vbf->first_glyph;
    font_propt.last_glyph  = vbf->last_glyph;
    font_propt.def_glyph   = vbf->def_glyph;
    
    fwrite (&font_propt, sizeof (FONT_PROPT), 1, fp);
    
    if (vbf->bbox)
    {
        VBF_BBOX *p;
        MYVBF_BBOX myvbf;
        p = vbf->bbox;
        filelayout.off_bboxs = ftell (fp);

        for (i=0; i<n; i++)
        {
            filelayout.len_bboxs ++;
            myvbf.x =p->x;
            myvbf.y =p->y;
            myvbf.w =p->w;
            myvbf.h =p->h;
            fwrite (&myvbf, sizeof (MYVBF_BBOX), 1, fp);
            
            if (p->next)
                p = p->next;
            else
                break;
        }
         
    }
    
    if (vbf->advance_x)
    {
        ADVANCE_X *p;
        filelayout.off_advxs = ftell (fp);
        p = vbf->advance_x;
        
        for (i=0; i<n; i++)
        {
            filelayout.len_advxs++;
            fwrite (&(p->advance_x), 1, 1, fp);
            
            if (p->next!=NULL)
                p =p->next;
            else
                break;
        }
    }

    if (vbf->advance_y)
    {
        ADVANCE_Y *p;
 
        filelayout.off_advys = ftell (fp);
        p = vbf->advance_y;
        
        for(i=0; i<n; i++)
        {
            filelayout.len_advys++;
            fwrite (&(p->advance_y), 1, 1, fp);
            
            if (p->next)
                p =p->next;
            else
                break;
        }
    }

    bits_offset = malloc (sizeof (unsigned int) * n);
    off_bits_offset = ftell (fp);
    fwrite (bits_offset, sizeof (unsigned int), n, fp);
    filelayout.off_bit_offs = off_bits_offset;
    filelayout.len_bit_offs = n;
    


    if (vbf->all_glyph_bits)
    {
        GLYPH_BITS *p;
        static BOOL done_glyph =FALSE;
        p = vbf->all_glyph_bits;
        filelayout.off_bits = ftell (fp);

        for (i=0; i<n; i++)
        {
            
            if (p->is_default_glyph)
            {

                if (!done_glyph)
                {
                    bits_offset[i] = ftell (fp) - filelayout.off_bits;
                    fwrite (p->glyph_bits, 1, p->bmp_size, fp); 
                    filelayout.len_bits ++;
                    done_glyph =TRUE;
                    p =p->next;
                    continue;

                }
                bits_offset[i] =bits_offset[vbf->def_glyph] ;
                p =p->next;
                continue;
        
            }
            
            bits_offset[i] = ftell (fp) - filelayout.off_bits;
            fwrite (p->glyph_bits, 1, p->bmp_size, fp); 
            filelayout.len_bits ++;
            p =p->next;

        };
        
    }
    filelayout.font_size =ftell (fp);

    fseek (fp, off_filelayout, SEEK_SET);
    fwrite (&filelayout, sizeof (FILE_LAYOUT), 1, fp);
    fseek (fp, off_bits_offset, SEEK_SET);
    fwrite (bits_offset, sizeof (unsigned int), n, fp);
    free (bits_offset);
    
    fclose (fp);
    return TRUE;
}

