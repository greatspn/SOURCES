#!/bin/sh 
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
. great_package.sh

echo ""
echo "Start Ordinary Reachability Graph construction using Next State function"

if [ "$2" != "-B" ] && [ "$3" != "-B" ] && [ "$4" != "-B" ] && [ "$5" != "-B" ] && [ "$6" != "-B" ] && [ "$7" != "-B" ]
then

pinv $1
struct $1

fi


#time
${GREATSPN_BINDIR}/RGMEDD $1 $2 $3 $4 $5 $6 $7
if [ $? -ne 0 ] 
then
	exit 23
fi
/bin/cat /dev/null >  $1.gst
if [ $? -ne 0 ] 
then
	exit 32 
fi


if [ "$2" = "-N" ] || [ "$3" = "-N" ]||[ "$4" = "-N" ]
then
	exit 0
fi


