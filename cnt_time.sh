#!/bin/bash

#rm -f output_dyn
#rm -f output
#rm -f output_msg_100
#rm -f output_algn
#rm -f output_con
rm -f output_b_con_100
rm -f output_b_con_200
rm -f output_b_con_400
rm -f output_b_con_500

#gcc -o test_dyn -pthread test_dyn.c
#gcc -o test -pthread test.c
#gcc -o test_msg -pthread test_msg.c
#gcc -o test_algn -pthread test_algn.c
gcc -o test_con_100 -pthread test_con_100.c
gcc -o test_con_200 -pthread test_con_200.c
gcc -o test_con_400 -pthread test_con_400.c
gcc -o test_con_500 -pthread test_con_500.c

# for i in {128..65536..128};
# do
#     #./test_dyn $i >> output_dyn;
#     #./test $i >> output;
#     #./test_msg $i >> output_msg_100;
#     ./test_algn m$i b4096 >> output_algn;
#     ./test_con m$i b4096 >> output_con;
# done

for i in {4096..49152..4096};
do
    #./test_algn b$i >> output_b_algn;
    ./test_con_100 b$i >> output_b_con_100;
    ./test_con_200 b$i >> output_b_con_200;
    ./test_con_400 b$i >> output_b_con_400;
    ./test_con_500 b$i >> output_b_con_500;
done

gnuplot compare.gp