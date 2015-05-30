#!/bin/bash
#
# This script prints out how many times each box has been used in all scenarios
#
#

SYSTEM=`uname -o`
if [ $SYSTEM == "Cygwin" ]; then
  EXT="cmd"
else
  EXT="sh"
fi

cd ../dist/share/openvibe/scenarios/
../../../openvibe-plugin-inspector.$EXT --no-pause -l | grep -e "^BoxAlgorithm " | sort | while read DUMMY NAME ID; do
	numLines=`grep -R -i "$ID" | wc -l`
	echo $NAME : $numLines occurrences
	if [ $numLines -eq 0 ]; then
		echo  !!! No scenario for $NAME $ID
	fi
done

