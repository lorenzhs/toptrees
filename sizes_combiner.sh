#!/usr/bin/zsh

for file in xml/*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,.*/,,g' | sed 's,\.xml$,.txt,')

	for M in {1.000001,1.00001,1.0001,1.001,1.01,1.03,1.05,1.075,1.1,1.125,1.15,1.175,1.19,1.2,1.21,1.215,1.22,1.225,1.23,1.24,1.25,1.26,1.27,1.28,1.3,1.33,1.36,1.39,1.42,1.46,1.5,2}; do
		OUT="eval-size/repair-m-$M"
		mkdir -p $OUT
		./coding-p -r $file -m $M | tee $OUT/$FN &
		echo ""
	done
	wait
	cat $OUT/*.txt > $OUT/result
done
