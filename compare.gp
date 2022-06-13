set title "ring buffer speed"
set xlabel "Number of messages (x10^2)"
set ylabel "time(usec)"
set terminal png font " Times_New_Roman,12 "
set output "Compare_what.png"
set xtics 100 ,100 ,600
set key left 


plot "huh" using 1 with linespoints linewidth 2 title "mmap with msg size 10", \
#plot "output" using 1 with linespoints linewidth 2 title "mmap", \
#"valid.txt" using 1 with linespoints linewidth 2 title "malloc", \