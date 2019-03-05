#!/bin/bash

net=$1
output=$2
exec=$3
array=$@
echo ${@:4}
remaing=${@:4}  
echo $remaing




echo
echo "======================================================="
echo "			INPUT PARAMETERS"
echo "======================================================="
echo
echo "Net:				${net}"
echo "Output:				${output}"
echo "Parallel executions		${exec}"
echo "Other parameters			${remaing}"

echo 
echo "======================================================="


echo 
for (( i=0; i<${exec}; i++ ));
do
command="${net} ${output}${i} ${remaing} >& bash_${output}${i} &"
echo "$i--> ${command}"
echo
eval ${command}	
done
echo "======================================================="
echo

