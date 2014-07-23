#!/usr/bin/env python3

# generate input file from statistics dumps with:
# for f in res_*; do echo -n "$f\t" | sed 's,.*res_,,'; cat $f | grep "Edges" | cut -d ' ' -f 3; done > eval

from pylab import *
import matplotlib.pyplot as plt
from math import log

def plotstats(inputfile):
	with open(inputfile, 'r') as infile:
		lines = infile.readlines()
	orig, comp = zip(*map(lambda x: map(int, x.split()), lines))

	cr = [orig/comp for (orig, comp) in zip(orig, comp)]
	y = [log(ratio) / log(log(n)) for (ratio, n) in zip(cr, orig)]

	fig, ax = subplots()
	plot(orig, cr, 'o')
	show()


if __name__ == '__main__':
	import sys
	if len(sys.argv) < 2:
		print('Usage: {1} inputfile'.format(sys.argv[0]))
		sys.exit(1)
	plotstats(sys.argv[1])