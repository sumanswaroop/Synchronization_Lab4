set term postscript eps color

#set output 'case1.eps'
set output 'case2.eps'

#used to make the fonts appear larger;  makes the figure smaller but keeps the fonts same size
set size 0.6, 0.6
set key right bottom
set grid ytics lc rgb "#bbbbbb" lw 1 lt 0
set grid xtics lc rgb "#bbbbbb" lw 1 lt 0
set xlabel "Number of Workers"
set yrange [0:6]


set ylabel "Average Response Time(sec/res)"

#plot 'case1.txt' using 1:3 title "Average Response Time" with linespoints ls 1
#set ylabel "Throughput(in req/sec)"
#plot 'case1.txt' using 1:2 title "Throughput" with linespoints ls 1

#second case

set yrange [2:10]
plot 'case2.txt' using 1:3 title "Average Response Time" with linespoints ls 1
set yrange [0:6]
set ylabel "Throughput(in req/sec)"
plot 'case2.txt' using 1:2 title "Throughput" with linespoints ls 1
set ylabel "Number of Denied Request"
set yrange [800:2000]
plot 'case2.txt' using 1:4 title "Number of Denied Req" with linespoints ls 1


