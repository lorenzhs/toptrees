#!/bin/bash
OUT=eval-cr-classic
mkdir -p $OUT

test() {
	m=$1
	t=$2
	echo "Testing with m=${m} t=${t} at $(date)"
	./randomEval-p -g $OUT/ratio_${m}.dat -o $OUT/stat_${m} -n 100 -m ${m} -t $t 2>&1| tee $OUT/res_${m};
}

for m in {4194304,8388608,16777216,33554432,67108864};
do
	test $m 48
done

test 134217728 24
test 268435456 12
