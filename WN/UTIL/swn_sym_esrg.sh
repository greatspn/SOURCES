#!/bin/sh 

. great_package.sh

echo ""
echo "Start SWN Extended Symbolic Reachability Graph construction"
#time
${GREATSPN_BINDIR}/WNESRG $1 $2 
if [ $? -ne 0 ] 
then
	exit 22
fi
if [ "$2" = "-m" ]
then 
${GREATSPN_BINDIR}/ESRG_CTMC $1 $3 -d
${GREATSPN_BINDIR}/WNESRGSOLVER $1 -p
fi

exit 0
