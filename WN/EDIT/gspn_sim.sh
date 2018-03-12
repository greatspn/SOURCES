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


echo ""
echo "Start EGSPN Simulation ..."
#time
${GREATSPN_BINDIR}/GSPNSIM $1 $2 $3 $4 $5 $6 $7 $8 $9
if [ $? -ne 0 ] 
then
	exit 35 
fi
exit 0
