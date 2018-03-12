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

${GREATSPN_BINDIR}/pinvar $@
GSPN2STATUS=$?
if [ $GSPN2STATUS -ne 0 ]
then
	# echo "Solution failed in module pinvar"
	rm -f $1.pin
	exit 1
fi

${GREATSPN_BINDIR}/struct $@
GSPN2STATUS=$?
if [ $GSPN2STATUS -ne 0 ]
then
	# echo "Solution failed in module struct"
	rm -f $1.cc $1.me $1.sc $1.sub $1.ecs $1.bnd
	exit 7
fi

exit 0
