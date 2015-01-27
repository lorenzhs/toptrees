#!/usr/bin/zsh
OUT=eval-size-repair-bitstring
mkdir -p $OUT
for file in data/*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,/,_,g' | sed 's,\.xml$,.txt,')
	./repair-p $file | tee $OUT/$FN
	echo ""
done
