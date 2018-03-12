#!/bin/sh
. great_package.sh
############ GreatSPN2.0 ######################
#case $GSPN2VERSION in
#"")     path1=$0
#	path2=$path1:h
#	path1=$path2:h
#        GSPN2VERSION = $path1/install/version.csh
#	unset path1
#	unset path2
#        export GSPN2VERSION
#;;
#esac
#
#if ( ! $?GSPN2BINS ) then
#        GSPN2BINS = `$GSPN2VERSION`
#        echo $GSPN2BINS
#endif
#if ( ! $?GSPN2HOME ) then
#        source ~/.GreatSPN2_$GSPN2BINS
#        echo GSPN2HOME
#endif
${GREATSPN_BINDIR}/deadlock $1
GSPN2STATUS=$?
if [ $GSPN2STATUS -ne 0 ]
then
	# echo "Solution failed in module deadlock"
	rm -f $1.mdead
	exit 3 # $GSPN2STATUS
fi

exit 0
