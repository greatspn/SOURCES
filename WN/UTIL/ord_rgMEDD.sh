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

for ARG in "$@"
do
	if ([ "$ARG" = "-P" ] || [ "$ARG" = "-B" ])
	then
		NEED_INVARIANTS=1
	fi
	if [ "$ARG" = "-N" ]
	then
		RG_USING_NEXT_STATE_FUNC=1
	fi
done

if [ "${RG_USING_NEXT_STATE_FUNC}" = "1" ]
then
	echo "Start Ordinary Reachability Graph construction using Next State function"
else
	echo "Start Ordinary Reachability Graph construction and CTMC solution based on DD"
fi

if ([ "${NEED_INVARIANTS}" = "1" ] && [ ! -f ${1}.bnd ])
then
	struct $1
fi


#time
echo ${GREATSPN_BINDIR}/RGMEDD $@
${GREATSPN_BINDIR}/RGMEDD $@
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


