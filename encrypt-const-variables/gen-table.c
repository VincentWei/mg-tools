#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, const char *argv[]) {
    int table[256];
    int i;

    srand(time(NULL));

    for (i=0; i<sizeof(table)/sizeof(table[0]); ++i) {
        table[i] = i;
    }

    for (i=sizeof(table)/sizeof(table[0]); i>0; --i) {
        float tmp = (rand() * 1.0f / RAND_MAX);
        int index = tmp * i;
        printf("0x%02x\n", table[index]);
        table[index] = table[i-1];
    }
    return 0;
}
