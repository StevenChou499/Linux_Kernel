set title "ring buffer speed"
set xlabel "Number of messages (x128bytes)"
set ylabel "time(usec)"
set terminal png font " Times_New_Roman,12 "
set output "Compare_mmap_msg_size_10_to_400.png"
set xtics 100 ,100 ,512
set key left 


plot "output_msg" using 1 with linespoints linewidth 2 title "mmap with msg size 10", \
"output_msg_50" using 1 with linespoints linewidth 2 title "mmap with msg size 50", \
"output_msg_100" using 1 with linespoints linewidth 2 title "mmap with msg size 100", \
"output_msg_200" using 1 with linespoints linewidth 2 title "mmap with msg size 200", \
"output_msg_400" using 1 with linespoints linewidth 2 title "mmap with msg size 400", \