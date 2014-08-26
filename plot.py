#!/usr/bin/env python3

# generate input file from statistics dumps with:
# for f in res_*; do echo -n "$f\t" | sed 's,.*res_,,'; cat $f | grep "Edges" | cut -d ' ' -f 3; done > eval

from pylab import *
from math import log

sigma = 2  # alphabet size

def plotstats(inputfile):
	with open(inputfile, 'r') as infile:
		lines = infile.readlines()
	orig, comp = zip(*map(lambda x: map(int, x.split()), lines))

	cr = [orig/comp for (orig, comp) in zip(orig, comp)]
	y = [ratio / log(n, 4*sigma) for (ratio, n) in zip(cr, orig)]

	elems = list(sorted(zip(orig, y)))
	for (o,res) in elems:
		print('({orig}, {res})'.format(orig=o, res=res))

	fig, ax = subplots()
	plot(orig, y, 'o')
	ax.set_xscale('log', basex = 2.0)
	ax.set_xlabel('tree size (n)')
	ax.set_ylabel('compression ratio / log_4σ(n)')
	show()


if __name__ == '__main__':
	import sys
	if len(sys.argv) < 2:
		print('Usage: {0} inputfile'.format(sys.argv[0]))
		sys.exit(1)
	plotstats(sys.argv[1])