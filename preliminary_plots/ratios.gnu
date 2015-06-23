set terminal svg size 1280,800 dynamic enhanced fname 'arial'  fsize 10 mousing name "pareto" butt solid
set output "ratios.svg"

set xlabel "Edge Compression Ratio"
set ylabel "Frequency"

width=0.025
# histogram function
hist(x,width)=width*(floor(x/width)+0.5)

set boxwidth width
set style fill solid 0.65

plot "ratios.dat" u (hist($1,width)):(1.0) smooth freq with boxes notitle