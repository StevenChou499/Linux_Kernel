#!/bin/bash

rm -f output
rm -f valid.txt

gcc -o test -pthread test.c
gcc -o test_dyn -pthread test_dyn.c

for i in {100..60000..100};
do
    ./test $i >> output;
    ./test_dyn $i >> valid.txt;
    #echo $i;
done

gnuplot compare.gp