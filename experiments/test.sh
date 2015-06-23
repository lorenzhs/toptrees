#!/usr/bin/zsh
OUT=ext/out
for file in data/*.xml; do
	echo "Testing with file $file at `date`"
	./test-p $file
	echo ""
done | tee $1
