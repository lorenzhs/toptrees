#!/usr/bin/zsh

for file in data/*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,.*/,,g' | sed 's,\.xml$,.txt,')

	for M in {1.0001,1.001,1.01,1.05,1.075,1.1,1.125,1.15,1.175,1.19,1.2,1.21,1.225,1.25,1.28,1.333,1.42,1.5}; do
		OUT=eval-size-repair-m-$M
		mkdir -p $OUT
		./coding-p -r $file -m $M | tee $OUT/$FN | tee -a $OUT/result
		echo ""
	done
done
