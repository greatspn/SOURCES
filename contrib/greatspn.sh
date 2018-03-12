#!/bin/sh
. great_package.sh
#PATH=$PATH:$GREATSPN_SCRIPTDIR
UIDPATH=${GREATSPN_BINDIR}/%U
export UIDPATH
export GREATSPN_BINDIR
export GREATSPN_SCRIPTDIR
echo ${GREATSPN_SCRIPTDIR}
echo ${GREATSPN_BINDIR}

STATUS=0

if [ ! -f $HOME/.greatspn ]
	then
		echo INSTALLING
		${GREATSPN_BINDIR}/first
		STATUS=$?
		if [ $STATUS -eq 1 ]
			then
				echo status 1
				exit $STATUS
		elif [ $STATUS -eq 2 ]
			then
				echo 
				exit $STATUS
		elif [ $STATUS -eq 3 ] 
			then
				echo
				exit $STATUS
		elif [ $STATUS -ne 0 ]
			then
				echo ERROR
				exit $STATUS
		fi
	else
		echo NOT INSTALLING
fi


unset GSPN_DEFAULT_PRINTER 
unset GSPN_NET_DIRECTORY 
unset GSPN_PS_DIRECTORY 
unset GSPN_EPS_DIRECTORY 
. $HOME/.greatspn
export GSPN_DEFAULT_PRINTER 
export GSPN_NET_DIRECTORY 
export GSPN_PS_DIRECTORY 
export GSPN_EPS_DIRECTORY 

echo ""
echo "# SETTING GSPN'S ENVIRONMENT VARIABLES TO :"
echo ""
echo GREATSPN_SCRIPTDIR=$GREATSPN_SCRIPTDIR 
echo GSPN_DEFAULT_PRINTER=$GSPN_DEFAULT_PRINTER 
echo GSPN_NET_DIRECTORY=$GSPN_NET_DIRECTORY
echo GSPN_EPS_DIRECTORY=$GSPN_EPS_DIRECTORY
echo GSPN_PS_DIRECTORY= $GSPN_PS_DIRECTORY



 
${GREATSPN_BINDIR}/GreatSPN
exit $STATUS

