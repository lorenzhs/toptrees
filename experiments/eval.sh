#!/bin/zsh
OUT=eval-cr-classic
OUTR=eval-cr-repair
mkdir -p $OUT $OUTR

test() {
	m=$1
	echo "Testing with m=${m} at $(date)"
	./randomEval-p -g $OUT/ratio_${m}.dat -o $OUT/stat_${m} -n $2 -m ${m} 2>&1| tee -a $OUT/res_${m};
	#./randomEval-p -g $OUTR/ratio_${m}.dat -o $OUTR/stat_${m} -n $2 -m ${m} 2>&1| tee -a $OUTR/res_${m};
}

for m in {1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576,2097152};
do
	test $m 1000
done

for m in {4194304,8388608,16777216};
do
	test $m 100
done