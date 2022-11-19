#!/bin/bash

. great_package.sh

echo
echo
echo "--------------------------------------------------------------------"
echo "                   Variability analysis for Flux Balance"
echo "--------------------------------------------------------------------"
echo
echo

if [ "$1" = "" ] || [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$#" -ne "6" ]
then

echo "Use Variability.sh <Flux_Balance_Problem> <Solutions_Flux_Balance> <Flux_names> <GAMMA> <Variability_Ouput> <CORES>"
	echo "       where:"
    echo "       <Flux_Balance_Problem>   ->  the file describing the LP problem"
    echo "       <Solutions_Flux_Balance> ->  the file storing the LP solution computed by PNODE (i.e .flux)"
    echo "       <Flux_name>              ->  the file storing the list of fluxes names (one for line) on which the variability is computed"
    echo "       <GAMMA>                  ->  a double value used as gamma in the variability analysis"
    echo "       <CORES>                  ->  the number of processes executed in parallel"
    echo
 	exit
fi

FBProblem=$1
FBPSolution=$2
FName=$3
GAMMA=$4
Output=$5
Cores=$6

readarray -t FluxNameArray < "${FName}"

echo "Flux names for variability:"
echo ${FluxNameArray[@]}
echo
echo "Running:"
for i in "${FluxNameArray[@]}"
do
	echo ${GREATSPN_BINDIR}/VARIABILITY ${Output}$i ${FBProblem} ${FBPSolution} $i ${GAMMA} "&"
	${GREATSPN_BINDIR}/VARIABILITY ${Output}$i ${FBProblem} ${FBPSolution} $i ${GAMMA} &
	background=( $(jobs -p) )
	if (( ${#background[@]} == Cores )); then
        	wait -n
	fi
done
wait
echo "--------------------------------------------------------------------"

