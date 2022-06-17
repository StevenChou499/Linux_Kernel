#!/bin/bash

#rm -f output_dyn
#rm -f output
rm -f output_msg_100
rm -f output_algn

#gcc -o test_dyn -pthread test_dyn.c
#gcc -o test -pthread test.c
gcc -o test_msg -pthread test_msg.c
gcc -o test_algn -pthread test_algn.c

for i in {128..65536..128};
do
    #./test_dyn $i >> output_dyn;
    #./test $i >> output;
    ./test_msg $i >> output_msg_100;
    ./test_algn m$i >> output_algn;
done

gnuplot compare.gp