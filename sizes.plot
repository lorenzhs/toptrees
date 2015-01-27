# IMPORT-DATA eval eval-size*/result

set terminal pdf size 48cm,27cm linewidth 2.0
set output "sizes.pdf"

set pointsize 0.7

set logscale x 1.00001
set xrange [1:1.5]

set grid xtics ytics

set key top left

set title 'Comparison of RePair Combiner Minimum Merge Ratios'
set xlabel 'Filename'
set ylabel 'Compression ratio vs median compression ratio for this file'

## MULTIPLOT(file) SELECT minratio AS x, succinct*1.0/compressed / aggregate AS y, MULTIPLOT
## FROM eval JOIN
## (SELECT MIN(succinct*1.0/compressed) AS aggregate, MULTIPLOT as multi FROM eval group by MULTIPLOT) avgt
## ON eval.MULTIPLOT = avgt.multi GROUP BY MULTIPLOT,x ORDER BY MULTIPLOT,x
plot \
    'sizes-data.txt' index 0 title "file=data/1998statistics.xml" with linespoints, \
    'sizes-data.txt' index 1 title "file=data/JST_gene.chr1.xml" with linespoints, \
    'sizes-data.txt' index 2 title "file=data/JST_snp.chr1.xml" with linespoints, \
    'sizes-data.txt' index 3 title "file=data/NCBI_gene.chr1.xml" with linespoints, \
    'sizes-data.txt' index 4 title "file=data/SwissProt.xml" with linespoints, \
    'sizes-data.txt' index 5 title "file=data/dblp.xml" with linespoints, \
    'sizes-data.txt' index 6 title "file=data/dblp_small.xml" with linespoints, \
    'sizes-data.txt' index 7 title "file=data/enwiki-latest-pages-articles12.xml" with linespoints, \
    'sizes-data.txt' index 8 title "file=data/enwiki-new.xml" with linespoints, \
    'sizes-data.txt' index 9 title "file=data/factor0.2.xml" with linespoints, \
    'sizes-data.txt' index 10 title "file=data/factor1.xml" with linespoints, \
    'sizes-data.txt' index 11 title "file=data/factor12.xml" with linespoints, \
    'sizes-data.txt' index 12 title "file=data/factor2.xml" with linespoints, \
    'sizes-data.txt' index 13 title "file=data/factor4.8.xml" with linespoints, \
    'sizes-data.txt' index 14 title "file=data/factor4.xml" with linespoints, \
    'sizes-data.txt' index 15 title "file=data/factor7.xml" with linespoints, \
    'sizes-data.txt' index 16 title "file=data/nasa.xml" with linespoints, \
    'sizes-data.txt' index 17 title "file=data/proteins.xml" with linespoints, \
    'sizes-data.txt' index 18 title "file=data/treebank_e.xml" with linespoints, \
    'sizes-data.txt' index 19 title "file=data/uwm.xml" with linespoints, \
    'sizes-data.txt' index 20 title "file=data/wiki.xml" with linespoints
