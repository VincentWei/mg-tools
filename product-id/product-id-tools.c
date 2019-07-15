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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <openssl/md5.h>

#define USE_MMAP

#ifdef USE_MMAP
#include <sys/mman.h>
#endif

#include "product-id.h"

static unsigned char s_magic_number[MD5_DIGEST_LENGTH] = {
    0xca, 0x3f, 0x2b, 0x00, 0x9f, 0x33, 0xb0, 0xc3,
    0x9f, 0x8a, 0xbe, 0xd0, 0x11, 0x11, 0x59, 0x63
};

static product_id_t *find_struct(unsigned char *file_buffer, int len)
{
    static const unsigned char prefix[8] = {
        PRODUCT_ID_PREFIX
    };
    int index;

    for (index=0; index<len; index++)
    {
        int i = 0;
        for (i=0; i<8; i++)
        {
            if (*(file_buffer+index+i) != prefix[i])
            {
                break;
            }
        }
        if (i >= 8)
        {
            return (product_id_t *)(file_buffer+index);
        }
    }
    return NULL;
}

static unsigned char *load_file(int fd, int *pLen, int do_write)
{
    struct stat sbuf;
    unsigned char *buf;

    if (fstat(fd, &sbuf) < 0)
    {
        fprintf(stderr, "failed to fstat(): %s\n", strerror(errno));
        return NULL;
    }
    *pLen = (int)sbuf.st_size;

#ifdef USE_MMAP
    buf = (unsigned char *)mmap(NULL, sbuf.st_size, PROT_READ|PROT_WRITE,
            do_write ? MAP_SHARED : MAP_PRIVATE, fd, 0);
    if (! buf)
    {
        fprintf(stderr, "failed to mmap(): %s\n", strerror(errno));
        return NULL;
    }
#else
    int n=0;
    buf = (unsigned char *)malloc(*pLen);

    while (n < *pLen)
    {
        int nread = read(fd, buf+n, *pLen-n);
        if (nread < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                free(buf);
                fprintf(stderr, "read error: %s\n", strerror(errno));
                return NULL;
            }
        }
        else if (nread == 0)
        {
            if (n != *pLen)
            {
                free(buf);
                fprintf(stderr, "size mismatch %d!=%d\n", n, *pLen);
                return NULL;
            }
        }
        else
        {
            n += nread;
        }
    }
#endif
    return buf;
}

static int unload_file(unsigned char *file_buffer, int len)
{
#ifdef USE_MMAP
    return munmap(file_buffer, len);
#else
    free(file_buffer);
    return 0;
#endif
}

static int commit_file_buffer(int fd, unsigned char *file_buffer, int offset, int len)
{
#ifdef USE_MMAP
    int aligned;
    int pagesize = sysconf(_SC_PAGE_SIZE);
    aligned = offset - (offset % pagesize);
    return msync(file_buffer + aligned, len+(offset-aligned), MS_SYNC);
#else
    if (lseek(fd, offset, SEEK_SET) < 0)
    {
        return -1;
    }

    if (write(fd, file_buffer+offset, len) != len
            || fsync(fd) < 0)
    {
        return -1;
    }
    return 0;
#endif
}

static int patch_product_id(const char *path, int customer_id, int version, 
        int compile_date) 
{
    unsigned char *file_buffer = NULL;
    product_id_t *product_id;
    int file_size;
    unsigned char md[MD5_DIGEST_LENGTH];
    int fd = -1;
    int ret = -1;

    fd = open(path, O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        goto out;
    }
    file_buffer = load_file(fd, &file_size, 1);
    if (! file_buffer)
    {
        goto out;
    }
    product_id = find_struct(file_buffer, file_size);
    if (! product_id)
    {
        fprintf(stderr, "product_id not found in %s\n", path);
        goto out;
    }

    product_id->customer_id = customer_id;
    product_id->version = version;
    product_id->compile_date = compile_date;
    product_id->file_size = file_size;
    memcpy(product_id->checksum, s_magic_number, sizeof(s_magic_number));

    MD5(file_buffer, file_size, md);
    memcpy(product_id->checksum, md, sizeof(md));

    if (commit_file_buffer(fd, file_buffer, (int)(((unsigned char *)product_id) - file_buffer), sizeof(*product_id)) < 0)
    {
        fprintf(stderr, "failed to write %s: %s\n", path, strerror(errno));
        goto out;
    }
    printf("Done.\n");

    ret = 0;

out:
    if (file_buffer)
    {
        unload_file(file_buffer, file_size);
    }

    if (fd >= 0)
    {
        close(fd);
    }
    return ret;
}

static const char *hex2str(const unsigned char *hex, int len)
{
    int i;
    static char s[1024];
    assert (len < (sizeof(s) / 2 - 1));
    for (i=0; i<len; i++)
    {
        s[2*i] = hex[i] / 16 + 'A';
        s[2*i+1] = hex[i] % 16 + 'A';
    }
    s[2*i] = 0;
    return s;
}

static int check_product_id(const char *path)
{
    unsigned char *file_buffer = NULL;
    product_id_t *product_id;
    int file_size;
    unsigned char md[MD5_DIGEST_LENGTH], md0[MD5_DIGEST_LENGTH];
    int fd = -1;
    int ret = -1;

    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        goto out;
    }
    file_buffer = load_file(fd, &file_size, 0);
    if (! file_buffer)
    {
        goto out;
    }
    product_id = find_struct(file_buffer, file_size);
    if (! product_id)
    {
        fprintf(stderr, "product_id not found in %s\n", path);
        goto out;
    }

    printf(
            "file name:    %s\n"
            "customer id:  %d\n"
            "version:      %d\n"
            "compile date: %d\n"
            "file size:    %d ",
            path, 
            product_id->customer_id,
            product_id->version,
            product_id->compile_date,
            product_id->file_size);
    if (product_id->file_size == file_size)
    {
        printf("OK\n");
    }
    else
    {
        printf("mismatch %d\n", file_size);
        goto out;
    }

    memcpy(md0, product_id->checksum, sizeof(md0));
    memcpy(product_id->checksum, s_magic_number, sizeof(s_magic_number));
    MD5(file_buffer, file_size, md);

    printf(
            "check sum:    %s ",
            hex2str(md0, sizeof(md0)));
    if (memcmp(md, md0, sizeof(md)))
    {
        printf("mismatch %s\n", hex2str(md, sizeof(md)));
        goto out;
    }
    else
    {
        printf("OK\n");
    }
    printf("Done.\n");

    ret = 0;

out:
    if (file_buffer)
    {
        unload_file(file_buffer, file_size);
    }

    if (fd >= 0)
    {
        close(fd);
    }
    return ret;
}

static void usage(void)
{
    fprintf(stderr, 
            "usage: p|patch-product-id <path> <custom_id> <version> <compile_date>\n"
            "   or: c|check-product-id <path>\n");
}

int main(int argc, const char *argv[])
{
    const char *command;
    if (argc < 2)
    {
        usage();
        return 1;
    }
    argc--;
    argv++;
    command = argv[0];
    if (strcmp(command, "patch-product-id") == 0 || strcmp(command, "p") == 0)
    {
        const char *path;
        int custom_id;
        int version;
        int compile_date;

        if (argc != 5)
        {
            usage();
            return 1;
        }
        path = argv[1];
        custom_id = atoi(argv[2]);
        version = atoi(argv[3]);
        compile_date = atoi(argv[4]);
        return patch_product_id(path, custom_id, version, compile_date) == 0 ? 0 : 1;
    }
    else if (strcmp(command, "check-product-id") == 0 || strcmp(command, "c") == 0)
    {
        const char *path = argv[1];
        return check_product_id(path) == 0 ? 0 : 1;
    }
    else
    {
        usage();
        return 1;
    }
}
