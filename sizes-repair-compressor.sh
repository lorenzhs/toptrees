#!/usr/bin/zsh
MASTER=eval-size
OUT=$MASTER/repair
mkdir -p $OUT
for file in xml/*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,/,_,g' | sed 's,\.xml$,.txt,')
	./repair-p $file | tee $OUT/$FN &
	echo ""
done
wait
grep -h $OUT/*.txt > $MASTER/repair.txt
