#!/bin/sh
. great_package.sh
#cd ${GREATSPN_SCRIPTDIR}/../multisolve
CLASSPATH=${GREATSPN_BINDIR}/multisolve:${CLASSPATH}
export CLASSPATH

unset GSPN_NET_DIRECTORY
unset GSPN_PS_DIRECTORY
unset GSPN_EPS_DIRECTORY

GSPN_NET_DIRECTORY=${HOME}

if [ -f $HOME/.greatspn ]
then
. $HOME/.greatspn
fi    
                     
echo ""
echo " Trying to Start MultiSolve"
echo "  (for proper use gnuplot 3.7 or higher is required)"
echo ""

export GREATSPN_SCRIPTDIR GREATSPN_BINDIR

java MultiSolve "$GSPN_NET_DIRECTORY" "${GREATSPN_BINDIR}" &
exit
