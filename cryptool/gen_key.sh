#!/bin/sh

#RC4KEY=`echo -n $RANDOM | md5sum | awk '{print $1}'`
#echo $RC4KEY

RC4KEY=`awk 'BEGIN {srand();for (i=1;i<=16;i++) printf("%02x",int(256*rand()))}'`
echo $RC4KEY

TMPRC4KEY=`echo -n $RC4KEY | awk '{for(i=1;i<=32;i+=2) {print "0x"substr($1,i,2)}}'`

echo $TMPRC4KEY

CRC4KEY=`echo -n $TMPRC4KEY | sed "s/ /,/g"`

echo $CRC4KEY

echo "MG_LOCAL unsigned char splash_crypto_key[16] = {" > key.c
echo "    "$CRC4KEY >> key.c
echo "};" >> key.c
echo "" >> key.c
