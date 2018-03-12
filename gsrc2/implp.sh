#!/bin/sh
. great_package.sh

############ GreatSPN2.0 ######################
if ( ! $?GSPN2VERSION ) then
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

${GREATSPN_BINDIR}/implp  $1 $2
if [ $? -ne 0 ]
then
	# echo Solution failed in module implp
	rm -f $1.impl
        exit 17
fi

exit 0
