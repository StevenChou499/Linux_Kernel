#!/bin/bash

rm -f output
rm -f valid.txt
rm -f huh

gcc -o test -pthread test.c
gcc -o test_dyn -pthread test_dyn.c
gcc -o test_msg -pthread test_msg.c

for i in {100..60000..100};
do
    #./test $i >> output;
    #./test_dyn $i >> valid.txt;
    ./test_msg $i >> huh;
    #echo $i;
done

#gnuplot compare.gp