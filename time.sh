#!/bin/bash

LOG_C="times_10_ttc.log"
LOG_R="times_10_ttr.log"
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

rm -f $LOG $LOG_C $LOG_R

# Run on stripped files
for f in xml/*.xml; do
	echo -n "Reading $f... " | tee -a $LOG_C | tee -a $LOG_R | tee -a $LOG
	fn=/tmp/ramdisk/$(echo $f | sed 's,.*/,,');
	cp $f $fn; # read file to RAM
	echo "done." | tee -a $LOG_C | tee -a $LOG_R | tee -a $LOG
	run "gzip" "-fk9" "gzip9" $fn $f          | tee -a $LOG
	run "bzip2" "-fk" "bzip2" $fn $f          | tee -a $LOG
	run "./repair-p" "" "repair" $fn $f       | tee -a $LOG
	run "./coding-p" "" "tt-classic" $fn $f   | tee -a $LOG_C
	run "./coding-p" "-r" "tt-repair" $fn $f  | tee -a $LOG_R
	run "./TreeRePair" "" "treerepair" $fn $f | tee -a $LOG
	rm $fn;
	echo "" | tee -a $LOG_C | tee -a $LOG_R | tee -a $LOG
done
