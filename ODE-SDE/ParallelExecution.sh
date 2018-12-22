#!/bin/bash

net=$1
output=$2
type=$3
hini=$4
atol=$5
rtol=$6
maxtime=$7
runs=$8
enableOutput=$9
step=${10}
exec=${11}



echo
echo "======================================================="
echo "			INPUT PARAMETERS"
echo "======================================================="
echo
echo "Net:				${net}"
echo "Output:				${output}"
echo "Type:				${type}"
echo "hini:				${hini}"
echo "atol:				${atol}"
echo "rtol:				${rtol}"
echo "Final Time:			${maxtime}"
echo "Enable output trace:		${enableOutput}"
echo "Step output:			${step}"
echo "Execution:			${exec}"
echo "Runs X Execution:		${runs}"
echo 
echo "======================================================="


echo 
for (( i=0; i<${exec}; i++ ));
do
command="${net} ${output}${i} ${type} ${hini} ${atol} ${rtol} ${maxtime} ${runs} ${enableOutput} ${step} >& bash_${output}${i} &"
echo "$i--> ${command}"
echo
eval ${command}	
done
echo "======================================================="
echo

