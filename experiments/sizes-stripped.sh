#!/usr/bin/zsh
MASTER=eval-size
OUT=$MASTER/input
mkdir -p $OUT
for file in xml/*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,/,_,g' | sed 's,\.xml$,.txt,')
	./strip -i $file | tee $OUT/$FN
	echo ""
done
grep -h RESULT $OUT/*.txt > $MASTER/input.txt
