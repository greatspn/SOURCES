#!/bin/sh 
. great_package.sh

echo ""
echo "Start SWN Dinamic Symbolic Reachability Graph construction"
#time

${GREATSPN_BINDIR}/WNDSRG $1 $2

if [ $? -ne 0 ] 
then
	exit 22
fi

if [ "$2" = "-m" ]
then 
${GREATSPN_BINDIR}/WNDSRGSOLVER $1 -p
fi

exit 0
