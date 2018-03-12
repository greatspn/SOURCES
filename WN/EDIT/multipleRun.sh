#!/bin/bash


if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ] || [ "$4" = "" ]
then
	echo "Use MultipleRun  netdirectory/netname <number_of_steps>  <step_increment> <initial>"
	exit 1
fi

TIME=$4
INC=$3;
echo
echo "---------------------------------------------------------"
echo "Runnning $2 experiments with initial time $4 and  step $3";
echo "---------------------------------------------------------"
echo
  for i in `seq 1 $2`;
        do
		echo "------------------------------------------";
                echo "        Step: $i with time: $TIME";
		echo "------------------------------------------";
		#single step
		randomTR $1 $TIME;
		echo $TIME
		if  ( echo $TIME | grep ^[.] ) then
			showtpd $1 > "$1.result0$TIME"
		else
			showtpd $1 > "$1.result$TIME"
		fi
		TIME=$(echo -e "$TIME+$INC" | bc -q );
		echo
        done    


