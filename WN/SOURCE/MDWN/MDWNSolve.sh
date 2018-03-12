#!/bin/bash

. great_package.sh

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
	exit
fi
if [ "$2" = "" ]
then
	echo "Use MDWN2WN.sh probabilistic_net, non_deterministic_net"
	echo "-h [num] solve the MDP at finite horizon"
	echo "-s to use the SRG approach"
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

${GREATSPN_BINDIR}/MDWN2WN $1 $2


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



if [ "$3" = "-s" ] || [ "$4" = "-s" ]||[ "$5" = "-s" ] || [ "$6" = "-s" ]
then
	echo "# Generate the SRG for the model: " 
	echo $1GL
	${GREATSPN_BINDIR}/MDWNSRG $1GL -w 0.0
else
	echo "# Generate the RG for the model: " 
	echo $1GL	
	${GREATSPN_BINDIR}/MDWNRG $1GL -w 0.0
fi

if [ "$?" -ne 0 ]
then 
	exit
fi 

echo "# Generate the MDP"
${GREATSPN_BINDIR}/RG2RRG $1GL -f -p -h$horizon -a -d -t

if [ "$?" -ne 0 ]
then 
	exit
fi 

if [ "$?" -ne 0 ]
then 
	exit
fi 

echo "# Solve the MDP"
if [ "$3" = "-g" ] || [ "$4" = "-g" ]||[ "$5" = "-g" ] || [ "$6" = "-g" ]
then
	echo "# Solve the MDP with GUI"
	echo
	${GREATSPN_BINDIR}/MDP $1GL -g
else
	echo "# Solve the MDP without GUI"
	echo
	${GREATSPN_BINDIR}/MDP $1GL -o -t -h $horizon
fi
