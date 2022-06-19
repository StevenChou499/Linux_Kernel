set title "ring buffer speed (tranfer size = 65536bytes)"
set xlabel "Size of ring buffer (x4096bytes)"
set ylabel "time(usec)"
set terminal png font " Times_New_Roman,12 "
set output "Compare_algn_con_bs_all.png"
set key left 

plot \
"output_b_con_100" u ($0+1):1 with linespoints linewidth 2 title "mmap with msg size 100 and algn", \
"output_b_con_200" u ($0+1):1 with linespoints linewidth 2 title "mmap with msg size 200 and algn", \
"output_b_con_400" u ($0+1):1 with linespoints linewidth 2 title "mmap with msg size 400 and algn", \
"output_b_con_500" u ($0+1):1 with linespoints linewidth 2 title "mmap with msg size 500 and algn", \