#!/bin/bash

LOG="times_10.log"

run() {
	job=$1
	params=$2
	name=$3
	file=$4
	printfile=$5
	for iteration in {0..9}; do
		echo $(/usr/bin/time -f "RESULT time=%e %" ${job} ${params} ${file} 2>&1 >/dev/null; echo "job=${name} file=${printfile}") | sed 's,\?,,';
	done
}

# Run on stripped files
for f in xml/*.xml; do
	echo -n "Reading $f... "
	fn=/tmp/ramdisk/$(echo $f | sed 's,.*/,,');
	cp $f $fn; # read file to RAM
	echo "done."
	run "gzip" "-fk9" "gzip9" $fn $f;
	run "bzip2" "-fk" "bzip2" $fn $f;
	run "./repair-p" "" "repair" $fn $f;
	run "./coding-p" "" "tt-classic" $fn $f;
	run "./coding-p" "-r" "tt-repair" $fn $f;
	run "./TreeRePair" "" "treerepair" $fn $f;
	rm $fn;
	echo ""
done | tee $LOG
