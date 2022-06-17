set title "ring buffer speed"
set xlabel "Number of messages (x128bytes)"
set ylabel "time(usec)"
set terminal png font " Times_New_Roman,12 "
set output "tmp.png"
set xtics 100 ,100 ,512
set key left 


plot "output_msg_100" using 1 with linespoints linewidth 2 title "mmap with msg size 100", \
"output_algn" using 1 with linespoints linewidth 2 title "mmap with msg size 100 and algn", \