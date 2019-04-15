/*
** Copyright (C) 2003, FMSoft.
*/

#ifndef _DUMPBMP_H

#define _DUMPBMP_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define LEN_PREFIX      29

struct mem_dc_info {
    int bpp;
    Uint32 rmask;
    Uint32 gmask;
    Uint32 bmask;
    Uint32 amask;
    GAL_Color pal [256];

    char prefix [LEN_PREFIX + 1];
};

extern int dump_bitmap (BITMAP* bmp, const char* prefix, FILE *fp);
extern int get_dump_info (HWND hwnd, struct mem_dc_info* info);
extern int add_hash_entry (const char* path, const char* prefix);
extern int dump_hash_table (const char* prefix);
extern int add_bmp_header_entry (FILE *fp, const char* prefix);
extern int add_bmp_loader_entry (FILE *fp, const char* file, const char* prefix);
extern int add_bmp_unloader_entry (FILE *fp, const char* prefix);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _DUMPBMP_H */

