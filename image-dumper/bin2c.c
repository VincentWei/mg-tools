/*
 *  bin2c - compresses data files & converts the result to C source code
 *  Copyright (C) 1998-2000  Anders Widell  <awl@hem.passagen.se>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This command uses the zlib library to compress each file given on
 * the command line, and outputs the compressed data as C source code
 * to the file 'data.c' in the current directory
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef USE_LIBZ
#include <zlib.h>
#else
typedef unsigned char Bytef;
typedef unsigned long uLongf;
#endif

#include "common.h"

#define BUFSIZE 16384            /* Increase buffer size by this amount */

#define SUFFIXLEN 8

static Bytef *source=NULL;       /* Buffer containing uncompressed data */
static Bytef *dest=NULL;         /* Buffer containing compressed data */
static uLongf sourceBufSize=0;   /* Buffer size */
#ifdef USE_LIBZ
static uLongf destBufSize=0;     /* Buffer size */
#endif

static uLongf sourceLen;         /* Length of uncompressed data */
static uLongf destLen;           /* Length of compressed data */

static FILE *infile=NULL;        /* The input file containing binary data */
static FILE *outfile=NULL;       /* The output file 'data.c' */

static const char *programName="convert to c file";


/*
 * Print error message and free allocated resources
 *
 */

static int
error (msg1, msg2, msg3)
     char *msg1;
     char *msg2;
     char *msg3;
{
  fprintf (stderr, "%s: %s%s%s\n", programName, msg1, msg2, msg3);

  if (infile != NULL) fclose (infile);
  if (outfile != NULL) fclose (outfile);
  remove ("data.c");
  free (dest);
  free (source);

  return 1;
}

/*
 * Replacement for strrchr in case it isn't present in libc
 *
 */

static char *
my_strrchr (s, c)
     char *s;
     int c;
{
  char *ptr = NULL;

  while (*s) {
    if (*s == c) ptr = s;
    s++;
  }

  return ptr;
}

#ifdef USE_LIBZ
/*
 * NOTE: my_compress2 is taken directly from zlib 1.1.3
 *
 * This is for compability with early versions of zlib that
 * don't have the compress2 function.
 *
 */

/* ===========================================================================
     Compresses the source buffer into the destination buffer. The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer. Upon entry, destLen is the total size of the
   destination buffer, which must be at least 0.1% larger than sourceLen plus
   12 bytes. Upon exit, destLen is the actual size of the compressed buffer.

     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.
*/
int my_compress2 (dest, destLen, source, sourceLen, level)
    Bytef *dest;
    uLongf *destLen;
    const Bytef *source;
    uLong sourceLen;
    int level;
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
#ifdef MAXSEG_64K
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;
#endif
    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    if (err != Z_OK) return err;

    err = deflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        deflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *destLen = stream.total_out;

    err = deflateEnd(&stream);
    return err;
}
#endif

int image_to_c (int nr, char ** src_file, 
        char * src_parent_path, char * dst_file_full_path,
        char * target_name)
{
  int i;
  char suffix[SUFFIXLEN];
  char filename[MAX_NAME + 1];
  char target_name_upper[MAX_NAME + 1];

  int path_len;
  char mysrc_file[MAX_PATH + 1];
  char dst_header_file[MAX_PATH + 1];

#ifdef USE_LIBZ
  int result;
#endif
  unsigned j;
  char *ptr;
//  int position;

  printf ("\n"
          "file number  : [%i]\n"
          "target_name  : [%s]\n"
          "parent path  : [%s]\n"
          "dst file     : [%s]\n",
          nr, target_name, src_parent_path,
          dst_file_full_path);

  if (nr < 1)
  {
      return -1;
  }

  for (i=0; i<nr; ++i) {
    printf ("---src_file[%i] = [%s]\n", i, src_file[i]);
  }

  outfile = fopen (dst_file_full_path, "w");
  if (outfile == NULL) {
      fprintf (stderr, "can't open '%s' for writing\n", 
              dst_file_full_path);
      return -1;
  }

  fprintf (outfile, 
          "#include <minigui/common.h>\n"
          "#include <minigui/minigui.h>\n"
          "#include <minigui/gdi.h>\n"
          "#include <minigui/window.h>\n\n");

  path_len = strlen (src_parent_path);
  strcpy (mysrc_file, src_parent_path);
  mysrc_file[path_len] = '\0';

  /* Process each file given on command line */
  for (i=0; i<nr; ++i) {
    if (src_parent_path[path_len - 1] != '/') {
      mysrc_file[path_len] = '/';
      mysrc_file[path_len + 1] = '\0';
//    strcpy (mysrc_file + path_len + 1, src_file[i]);
    }
    else {
      mysrc_file[path_len] = '\0';
//    strcpy (mysrc_file + path_len, src_file[i]);
    }

    strcat (mysrc_file, src_file[i]);
    mysrc_file[MAX_NAME] = '\0';
    printf ("source file full path[%i] = [%s]\n", i, mysrc_file);

    infile = fopen (mysrc_file, "rb");
    if (infile == NULL) return error ("can't open '", mysrc_file, "' for reading");

    /* Read infile to source buffer */
    sourceLen = 0;
    while (!feof (infile)) {
      if (sourceLen + BUFSIZE > sourceBufSize) {
	sourceBufSize += BUFSIZE;
	source = realloc (source, sourceBufSize);
	if (source == NULL) return error ("memory exhausted", "", "");
      }
      sourceLen += fread (source+sourceLen, 1, BUFSIZE, infile);
      if (ferror (infile)) return error ("error reading '", mysrc_file, "'");
    }
    fclose (infile);

#ifdef USE_LIBZ

    /* (Re)allocate dest buffer */
    destLen = sourceBufSize + (sourceBufSize+9)/10 + 12;
    if (destBufSize < destLen) {
      destBufSize = destLen;
      dest = realloc (dest, destBufSize);
      if (dest == NULL) return error ("memory exhausted", "", "");
    }

    /* Compress dest buffer */
    destLen = destBufSize;
    result = my_compress2 (dest, &destLen, source, sourceLen, 9);
    if (result != Z_OK) return error ("error compressing '", mysrc_file, "'");

#else

    destLen = sourceLen;
    dest = source;

#endif

    /* Output dest buffer as C source code to outfile */
    strcpy (filename, src_file[i]);
    ptr = my_strrchr (filename, '.');
    suffix[0] = '\0';
    if (ptr != NULL) {
        strncpy (suffix, ptr + 1, SUFFIXLEN - 1);
        suffix[SUFFIXLEN - 1] = '\0';
        *ptr = '\0';
    }
    /* use only the file 2name and throw away the path name */
#if 0
    position = strlen(src_file[i]) - 1;
    while (position && src_file[i][position] != '/') position--;
    if (src_file[i][position] == '/') position++;
#endif

    fprintf (outfile, "static const unsigned char %s_%s[] = {\n", 
            filename, suffix);

    for (j=0; j<destLen-1; j++) {
      switch (j%8) {
      case 0:
	fprintf (outfile, "  0x%02x, ", ((unsigned) dest[j]) & 0xffu);
	break;
      case 7:
	fprintf (outfile, "0x%02x,\n", ((unsigned) dest[j]) & 0xffu);
	break;
      default:
	fprintf (outfile, "0x%02x, ", ((unsigned) dest[j]) & 0xffu);
	break;
      }
    }

    if ((destLen-1)%8 == 0) fprintf (outfile, "  0x%02x\n};\n\n", ((unsigned) dest[destLen-1]) & 0xffu);
    else fprintf (outfile, "0x%02x\n};\n\n", ((unsigned) dest[destLen-1]) & 0xffu);
  }

  fprintf (outfile, 
          "#define PATH_PREFIX \"\"\n\n"
          "BOOL add_%s_images_to_minigui (HDC hdc)\n{\n",
          target_name);

  for (i = 0; i < nr; ++i) {

    strcpy (filename, src_file[i]);
    ptr = my_strrchr (filename, '.');
    suffix[0] = '\0';
    if (ptr != NULL) {
        strncpy (suffix, ptr + 1, SUFFIXLEN - 1);
        suffix[SUFFIXLEN - 1] = '\0';
        *ptr = '\0';
    }

    fprintf (outfile, 
      "    "
      "if (!RegisterResFromMem (hdc, PATH_PREFIX \"%s\", \n"
      "        %s_%s, sizeof (%s_%s)))\n"
      "        return FALSE;\n",
      src_file[i], filename, suffix, filename, suffix);
  }

  fprintf (outfile, "\n    return TRUE;\n}");

  fclose (outfile);

  /* write header file */
  strcpy (dst_header_file, dst_file_full_path);
  i = strlen (dst_header_file);
  dst_header_file[i - 1] = 'h';

  outfile = fopen (dst_header_file, "w");
  if (outfile == NULL) {
      fprintf (stderr, "can't open '%s' for writing\n", 
              dst_header_file);
      return -1;
  }

  strcpy (target_name_upper, target_name);

  i = 0;
  while (target_name_upper[i] != '\0') {
      target_name_upper[i] =
          toupper (target_name_upper[i]);
      ++i;
  }

  fprintf (outfile, 
    "#ifndef %s_H\n"
    "#define %s_H\n\n"
    "#ifdef __cplusplus\n"
    "extern \"C\" {\n", target_name_upper, target_name_upper);

  fprintf (outfile,
    "#endif /* __cplusplus */\n\n"
    "    BOOL add_%s_to_minigui (HDC hdc);\n\n",
    target_name);

  fprintf (outfile,
    "#ifdef __cplusplus\n"
    "}\n"
    "#endif /* __cplusplus */\n\n"
    "#endif /* %s_H */",
    target_name_upper);

  fclose (outfile);

#ifdef USE_LIBZ
  free (dest);
#endif
  free (source);

  return 0;
}
