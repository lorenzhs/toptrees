#!/usr/bin/zsh

DIR="eval-size"

for file in xml{,/large}/*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,.*/,,g' | sed 's,\.xml$,.txt,')

	for M in {1.00001,1.0001,1.001,1.01,1.03,1.05,1.075,1.1,1.13,1.15,1.17,1.19,1.2,1.21,1.215,1.22,1.225,1.23,1.235,1.24,1.25,1.26,1.27,1.28,1.3,1.33,1.36,1.39,1.42,1.46,1.5,2}; do
		OUT="$DIR/repair-m-$M"
		mkdir -p $OUT
		./coding-p -r $file -m $M | tee $OUT/$FN &
		echo ""
	done
	wait
done

grep -h RESULT $DIR/*/*.txt > $DIR/result.txt
