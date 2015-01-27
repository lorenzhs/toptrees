#!/bin/zsh
OUT=eval-cr-classic
OUTR=eval-cr-repair

mkdir -p $OUT $OUTR
for m in {1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576,2097152,4194304,8388608,1677216};
do
	m=$(echo $(($m * 1.4142135623730951)) | sed 's,\..*,,')
	echo "Testing with m=${m} at $(date)"
	./randomEval-p -g $OUT/ratio_${m}.dat -o $OUT/stat_${m} -n 1000 -m ${m} 2>&1| tee -a $OUT/res_${m};
	#./randomEval-p -r -g $OUTR/ratio_${m}.dat -o $OUTR/stat_${m} -n 1000 -m ${m} -t 32 2>&1| tee -a $OUTR/res_${m};
done
