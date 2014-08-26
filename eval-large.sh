#!/bin/zsh
OUT=eval-cr-final
mkdir -p $OUT

test() {
	m=$1
	t=$2
	echo "Testing with m=${m} t=${t} at $(date)"
	./randomEval-p -r $OUT/ratio_${m}.dat -o $OUT/stat_${m} -n 100 -m ${m} -t $t 2>&1| tee $OUT/res_${m};
}

for m in {4194304,8388608,1677216};
do
	test $m 4
done

test 33554432 2
test 67108864 1
