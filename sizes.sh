#!/usr/bin/zsh
CLASSIC_OUT=eval-size-classic-final
REPAIR_OUT=eval-size-repair-final
mkdir -p $CLASSIC_OUT
mkdir -p $REPAIR_OUT
for file in ext/{,forlorenz/}*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,/,_,g' | sed 's,\.xml$,.txt,')
	./coding-p $file | tee $CLASSIC_OUT/$FN
	./coding-p -r $file | tee $REPAIR_OUT/$FN
	echo ""
done
