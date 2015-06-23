#!/bin/zsh
sp-process edges.plot 2>&1 | grep -v Cached
gnuplot edges.plot
sp-process sizes.plot 2>&1 | grep -v Cached
gnuplot sizes.plot
sp-process sizes_oldvsnew.plot 2>&1 | grep -v Cached
gnuplot sizes_oldvsnew.plot
sp-process edges_oldvsnew.plot 2>&1 | grep -v Cached
gnuplot edges_oldvsnew.plot
