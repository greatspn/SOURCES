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
	echo "-s to use the SRG approach"
	exit
fi
if [ "$2" = "" ]
then
	echo "Use MDWN2WN.sh probabilistic_net, non_deterministic_net"
	echo "-s to use the SRG approach"
	exit 
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

{GREATSPN_BINDIR}/MDWN2WN $1 $2


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
	${GREATSPN_BINDIR}/MDWNSRG $1GL -w
else
	echo "# Generate the RG for the model: " 
	echo $1GL	
	${GREATSPN_BINDIR}/MDWNRG $1GL -w
fi

if [ "$?" -ne 0 ]
then 
	exit
fi 

echo "# Generate the POMDP"
${GREATSPN_BINDIR}/RG2RRG $1GL -f -p -h$horizon -P

if [ "$?" -ne 0 ]
then 
	exit
fi 
