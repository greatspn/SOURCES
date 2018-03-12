#!/bin/bash
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
echo
echo
echo "--------------------------------------------------------------------"
echo "          Generation PS"
echo "--------------------------------------------------------------------"
echo
echo 

if [ "$1" = "" ]
then
	echo "Use directory/net name [option]"
	echo "	-o compute RG"
	echo "	-s compute SRG"
	echo "	-p ready to print"
	exit
fi


if [ "$2" = "-s" ] || [ "$3" = "-s" ]
then 
${GREATSPN_BINDIR}/WNSRG $1 -d
else
${GREATSPN_BINDIR}/WNRG $1 -d
fi

if [ "$3" = "-p" ] || [ "$4" = "-p" ]
then
	echo "# Generation PS file: Compact"
	echo
	dot -Gratio=fill -Gsize=7,17 $1.dot -Tps -o $1.ps
else
	echo "#Generation PS file: Normal"
	echo
	dot $1.dot -Tps -o $1.ps
fi


