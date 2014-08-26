#!/usr/bin/zsh
OUT=eval-size-repair-real
mkdir -p $OUT
for file in ext/{,forlorenz/}*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,/,_,g' | sed 's,\.xml$,.txt,')
	./repair-p $file | tee $OUT/$FN
	echo ""
done
