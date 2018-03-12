#!/bin/sh
. great_package.sh
############ GreatSPN2.0 ######################
#if ( ! $?GSPN2VERSION ) then
#	set path1 = $0;
#	set path2 = $path1:h
#	set path1 = $path2:h
#        setenv GSPN2VERSION $path1/install/version.csh
#	unset path1
#	unset path2
#endif
#if ( ! $?GSPN2BINS ) then
#        setenv GSPN2BINS `$GSPN2VERSION`
#endif
#if ( ! $?GSPN2HOME ) then
#        source ~/.GreatSPN2_$GSPN2BINS
#endif

${GREATSPN_BINDIR}/mark_lp $1 -i
GSPN2STATUS=$?
if [ $GSPN2STATUS -ne 0 ]
then
	# echo "Solution failed in module mark_lp"
	rm -f $1.lp_mark
	exit 19
fi 

exit 0
