#!/usr/bin/zsh
OUT=data/out
for file in data/*.xml; do
	echo "Testing with file $file at `date`"
	./testTT-p -i $file -o $OUT
	md5sum $OUT/{orig,unpacked,toptree,uncomp_toptree}.xml
	rm -f $OUT/*.xml
	echo ""
done | tee $1
