#!/usr/bin/zsh
CLASSIC_OUT=eval-size-classic
OLDREP_OUT=eval-size-repair-old
REPAIR_OUT=eval-size-repair-new
mkdir -p $CLASSIC_OUT
mkdir -p $REPAIR_OUT
mkdir -p $OLDREP_OUT
for file in data/*.xml(.); do
	echo "Testing with file $file at `date`"
	FN=$(echo $file | sed 's,/,_,g' | sed 's,\.xml$,.txt,')
	./coding-p $file | tee $CLASSIC_OUT/$FN
	./coding-p -r $file | tee $REPAIR_OUT/$FN
	#./coding-p-alt -r $file | tee $OLDREP_OUT/$FN
	echo ""
done
