#!/bin/bash

GSPN2PACKAGE=/home/marco/ESRG+DSRG/greatspn/i686_R2.6.26.8-57.fc8/

echo
echo
echo "--------------------------------------------------------------------"
echo "          Generation WN model"
echo "--------------------------------------------------------------------"
echo
echo 

if [ "$1" = "" ]
then
	echo "Use MDWN2WN.sh probabilistic_net, non_deterministic_net [option]"
	echo "-h [num] to solve the MDP at finite horizon. "
	echo "-s to use the SRG approach"
	echo "-g to use the GUI"
	exit
fi
if [ "$2" = "" ]
then
	echo "Use MDWN2WN.sh probabilistic_net, non_deterministic_net"
	echo "-h [num] solve the MDP at finite horizon"
	echo "-s to use the SRG approach"
	echo "-g to use the GUI"
	exit 
fi


if [ "$3" = "-h" ]
then 
	horizon=$4
else
	if [ "$4" = "-h" ]
	then
		horizon=$5

	else
		if [ "$5" = "-h" ]
		then
			horizon=$6
			
		else
			horizon=0
			
		fi
	fi
fi




echo "# Copy files .def: "
echo $1
cp $1.def $1M.def
echo $2
cp $2.def $2M.def

if [ "$?" -ne 0 ]
then 
	exit 1
fi 

./MDWN2WN $1 $2


if [ "$?" -ne 0 ]
then 
	exit 1
fi 

echo "# Compose the two model: "
algebra $1M $2M p 1 $1GL temp 

if [ "$?" -ne 0 ]
then 
	exit
fi 



