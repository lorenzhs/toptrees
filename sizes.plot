# IMPORT-DATA eval eval-size/result.txt

set terminal pdf size 48cm,27cm linewidth 2.0
set output "sizes.pdf"

set pointsize 0.7

set logscale x 1.00001
set xrange [1:1.5]

set grid xtics ytics

set key top left

set title 'Comparison of RePair Combiner Minimum Merge Ratios'
set xlabel 'Filename'
set ylabel 'Compression ratio vs minimum compression ratio'

## MULTIPLOT(file) SELECT minratio AS x,  2 - 1.0*compressed/refval AS y, MULTIPLOT
## FROM eval JOIN
## (SELECT compressed AS refval, MULTIPLOT as multi FROM eval WHERE minratio=1.25 GROUP BY MULTIPLOT) avgt
## ON eval.MULTIPLOT = avgt.multi GROUP BY MULTIPLOT,x ORDER BY MULTIPLOT,x
plot \
    'sizes-data.txt' index 0 title "file=xml/1998statistics.xml" with linespoints, \
    'sizes-data.txt' index 1 title "file=xml/JST_gene.chr1.xml" with linespoints, \
    'sizes-data.txt' index 2 title "file=xml/JST_snp.chr1.xml" with linespoints, \
    'sizes-data.txt' index 3 title "file=xml/NCBI_gene.chr1.xml" with linespoints, \
    'sizes-data.txt' index 4 title "file=xml/SwissProt.xml" with linespoints, \
    'sizes-data.txt' index 5 title "file=xml/dblp.xml" with linespoints, \
    'sizes-data.txt' index 6 title "file=xml/dblp_small.xml" with linespoints, \
    'sizes-data.txt' index 7 title "file=xml/enwiki-latest-pages-articles12.xml" with linespoints, \
    'sizes-data.txt' index 8 title "file=xml/enwiki-new.xml" with linespoints, \
    'sizes-data.txt' index 9 title "file=xml/factor0.2.xml" with linespoints, \
    'sizes-data.txt' index 10 title "file=xml/factor1.xml" with linespoints, \
    'sizes-data.txt' index 11 title "file=xml/factor12.xml" with linespoints, \
    'sizes-data.txt' index 12 title "file=xml/factor2.xml" with linespoints, \
    'sizes-data.txt' index 13 title "file=xml/factor4.8.xml" with linespoints, \
    'sizes-data.txt' index 14 title "file=xml/factor4.xml" with linespoints, \
    'sizes-data.txt' index 15 title "file=xml/factor7.xml" with linespoints, \
    'sizes-data.txt' index 16 title "file=xml/large/NCBI_snp.chr1.xml" with linespoints, \
    'sizes-data.txt' index 17 title "file=xml/nasa.xml" with linespoints, \
    'sizes-data.txt' index 18 title "file=xml/proteins.xml" with linespoints, \
    'sizes-data.txt' index 19 title "file=xml/treebank_e.xml" with linespoints, \
    'sizes-data.txt' index 20 title "file=xml/uwm.xml" with linespoints, \
    'sizes-data.txt' index 21 title "file=xml/wiki.xml" with linespoints
