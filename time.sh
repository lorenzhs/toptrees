#!/usr/bin/zsh

LOG="times.log"

run() {
	job=$1
	params=$2
	name=$3
	file=$4
	printfile=$5
	echo $(/usr/bin/time -f "RESULT time=%e %" ${job} ${params} ${file}	 2>&1 >/dev/null; echo "job=${name} file=${printfile}") | sed 's,\?,,';
}

for f in data/*.xml; do
	echo -n "Reading $f... "
	fn=/tmp/ramdisk/$(echo $f | sed 's,.*/,,');
	cp $f $fn; # read file to RAM
	echo "done."
	run "./repair-p" "" "repair" $fn $f;
	run "./coding-p" "" "tt-classic" $fn $f;
	run "./coding-p" "-r" "tt-repair" $fn $f;
	run "./TreeRePair" "" "treerepair" $fn $f;
	rm $fn;
	echo ""
done | tee $LOG
