#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "rc4.h"

#define FRAGMENT_SIZE 512

static unsigned char calc_sum(int len, const unsigned char *data) {
    unsigned char sum;
    int i;
    for (i=0, sum=0; i<len; ++i) {
        sum -= data[i];
    }
    return sum;
}

static int encrypted_len(int len) {
    return len + ((len-1) / (FRAGMENT_SIZE - sizeof(unsigned char)) + 1) * sizeof(unsigned char);
}

static int decrypted_len(int len) {
    return len - ((len-1) / FRAGMENT_SIZE + 1) * sizeof(unsigned char);
}

int my_encrypt(RC4_KEY *key, int in_len, const unsigned char *in, unsigned char **pOut) {
    unsigned char sum;
    int i;
    unsigned char *out;
    int out_len = 0;

    out = (unsigned char *)malloc(encrypted_len(in_len));
    if (! out) {
        assert(0);
        return -1;
    }

    for (i=0; i<in_len; /* NULL */) {
        int n = (in_len - i > FRAGMENT_SIZE - sizeof(unsigned char)) ? (FRAGMENT_SIZE - sizeof(unsigned char)) : (in_len - i);
        sum = calc_sum(n, in+i);
        RC4(key, sizeof(unsigned char), (const unsigned char *)&sum, &out[out_len]);
        out_len += sizeof(unsigned char);
        RC4(key, n, in+i, &out[out_len]);
        out_len += n;

        i += n;
    }

    *pOut = out;
    assert(out_len == encrypted_len(in_len));
    return out_len;
}

int my_decrypt(RC4_KEY *key, int in_len, unsigned char *in) {
    int out_len = 0;
    int i;
    unsigned char sum;

    for (i=0; i<in_len; /* NULL */) {
        int n;

        RC4(key, sizeof(unsigned char), &in[i], (unsigned char *)&sum);
        i += sizeof(unsigned char);

        n = (in_len - i > FRAGMENT_SIZE - sizeof(unsigned char)) ? (FRAGMENT_SIZE - sizeof(unsigned char)) : (in_len - i);
        RC4(key, n, &in[i], &in[out_len]);
        if (calc_sum(n, &in[out_len]) != sum) {
            assert(0);
            return -1;
        }
        i += n;
        out_len += n;
    }

    assert(out_len == decrypted_len(in_len));
    return out_len;
}

#if 1
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

static void usage(void)
{
    fprintf(stderr,
            "usage: cryptool -e infile outfile key\n"
            "   or: cryptool -d infile outfile key\n"
            "key is 16 bytes in hex (32 chars)\n"
            "cryptool -e file.in file.out 00112233445566778899AABBCCDDEEFF\n");
}

int main(int argc, const char *argv[])
{
    int fd_in;
    int fd_out;
    struct stat st;
    unsigned char *in;
    unsigned char *out;
    int in_len;
    int out_len = 0;
    int i;
    char cnum[3];

    unsigned char rc4key[16];

    RC4_KEY key;

    if (argc != 5)
    {
        usage();
        exit(1);
    }

    if (strcmp (argv[1], "-e") != 0 && strcmp (argv[1], "-d") != 0)
    {
        usage();
        exit(1);
    }

    /* argv[1] opt
     * argv[2] infile
     * argv[3] outfile
     * argv[4] key
     */

    for (i = 0; i < 16; i++)
    {
        cnum[0]=argv[4][i*2];
        cnum[1]=argv[4][i*2+1];
        cnum[2]=0;
        rc4key[i] = strtol( cnum, NULL, 16 );
    }

    RC4_set_key(&key, 16, (const unsigned char *)rc4key);

    fd_in = open(argv[2], O_RDONLY);
    assert(fd_in >= 0);
    if (fstat(fd_in, &st) < 0) {
        exit(1);
    }
    in = (unsigned char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd_in, 0);
    assert(in);
    in_len = st.st_size;

    if (strcmp (argv[1], "-e") == 0) {
        out_len = my_encrypt(&key, in_len, in, &out);
    } else if (strcmp (argv[1], "-d") == 0){
        out = (unsigned char *)malloc(in_len);
        memcpy(out, in, in_len);
        out_len = my_decrypt(&key, in_len, out);
    }

    assert(out_len > 0);

    munmap(in, in_len);
    close(fd_in);

    fd_out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd_out >= 0);
    if (write(fd_out, out, out_len) != out_len) {
        exit(1);
    }

    free(out);
    close(fd_out);

    for (i = 0; i < 16; i++)
    {
        printf ("0x%0.2X ", rc4key[i]);
    }
    printf ("\n");

    return 0;
}
#endif
