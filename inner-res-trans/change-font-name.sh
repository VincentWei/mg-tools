#!/bin/bash
i=0
for name in `ls`
do
	value=`echo "$name" | egrep  "\.[a-z]+\.c"`
	if [ "X$value" != "X" ]
	then
		mv $name "incore-font$i.c"
		i=`expr $i + 1`
	fi
done
