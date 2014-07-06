#!/usr/bin/zsh
OUT=ext/out
for file in ext/forlorenz/*.xml; do
	echo "Testing with file $file at `date`"
	./testTT $file $OUT
	md5sum $OUT/{orig,unpacked,toptree,uncomp_toptree}.xml
	rm -f $OUT/*.xml
	echo ""
done