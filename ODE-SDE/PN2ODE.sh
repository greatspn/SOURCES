#!/bin/bash

. great_package.sh

echo
echo
echo "--------------------------------------------------------------------"
echo "                      Generation ODE/SDE model"
echo "--------------------------------------------------------------------"
echo
echo

if [ "$1" = "" ] || [ "$1" = "-h" ] || [ "$1" = "-help" ]
then
	echo "Use PN2ODE <path_net>/<net> [options] [format] -T <path_transitions>/<transitions> -F <path_obj_funct>/<obj_funct>"
	echo "       Options:"
    echo "       -M ->   Genelarized Mass Action policy"
    echo "       -I ->   Infinity servers policy (default)"
    echo "       -A ->   Automaton verification"
    echo "       -F ->   Specifies objective function file (only with -P)"
    echo "       -T ->   Specifies transition bounds file (only with -P)"
    echo "       Format:"
    echo "       -R ->   Export in R format "
    echo "       -P ->   Export in R format with optimization "
    echo "       -O ->   Export in Matlab format"
    echo

    echo "Please, be aware that:"
    echo "-Format options can be either -O or -R or -P."
    echo "-Policy can be either -M or -I. When not specified -I is by default."
    echo "-Options -M and -A can be used simultaneously. "
    echo "-Export in -P must contain -T and -F options each one followed by the designed path."
	echo "-All Place Names must start with a letter."
	echo "-Time vector in R must be initialized from the user."

	echo
	exit
fi




NET_PATH=$1
TRANS_POLICY="-I"

shift

while [[ $# -gt 0 ]]
do

case "$1" in
    -O|-P|-R)
    EXPORT_FORMAT="$1"
    ;;

    -M|-I)
    TRANS_POLICY="$1"
    ;;

    -A)
    AUT="$1"
    ;;


    -T)
    if [ $# -gt 1 ]
    then
        TRANS_PATH="$2"
    else
        printf "**ERROR** You must specify a legal transition file path after -T. \n\n"
        exit 1
    fi
    shift
    ;;

    -F)
    if [ $# -gt 1 ]
    then
        FUN_PATH="$2"
    else
        printf "**ERROR** You must specify a legal objective function file path after -F. \n\n"
        exit 1
    fi
    shift
    ;;


    *)
    printf "**ERROR** Invalid Options.You can check syntax rules by executing PN2ODE.sh -help.\n\n"
	exit 1
    ;;

esac

shift

done



echo "#Computing p-semiflows and place bounds: "
struct $NET_PATH
if [ "$?" -ne 0 ]
then
	exit 1
fi


if [ "$EXPORT_FORMAT" == "-P" ]
then
    if [ -z $TRANS_PATH ] || [ -z $FUN_PATH ]
    then
     printf "**ERROR** Objective function and Transition bound files must be specified when using option -P.\n\n"
     exit 1
    fi
echo ${GREATSPN_BINDIR}/PN2ODE $NET_PATH $EXPORT_FORMAT $TRANS_POLICY -T $TRANS_PATH -F $FUN_PATH
${GREATSPN_BINDIR}/PN2ODE $NET_PATH $EXPORT_FORMAT $TRANS_POLICY -T $TRANS_PATH -F $FUN_PATH
else
echo ${GREATSPN_BINDIR}/PN2ODE $NET_PATH $EXPORT_FORMAT $TRANS_POLICY $AUT
${GREATSPN_BINDIR}/PN2ODE $NET_PATH $EXPORT_FORMAT $TRANS_POLICY $AUT
fi

if [ $? -ne 0 ]
then
	 echo "Solution failed in module PN2ODE"
	 exit 1
fi

if  [ "$EXPORT_FORMAT" == "-O" ]
then
	echo "Matlab file: $NET_PATH.m"
	echo
	exit 1
fi

if   [ "$EXPORT_FORMAT" == "-R" ]
then
	echo "R file: $NET_PATH.R"
	echo
	exit 1
fi

if   [ "$EXPORT_FORMAT" == "-P" ]
then
	echo "R file with opt: $NET_PATH.opt.R"
	echo
	exit 1
fi



NET_PATH="$(readlink $NET_PATH -m)"


export name_file=$NET_PATH;

echo $name_file



if [ "$AUT" == "-A" ]
then

  cd ${GREATSPN_SCRIPTDIR}/../inst_src
  echo "#cd ${GREATSPN_SCRIPTDIR}/../inst_src"
  MyTempDir=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`
  echo "#Copying file form ${GREATSPN_SCRIPTDIR}/../inst_src to ${MyTempDir}"
  echo
  cp ./class.* ${MyTempDir}
  cp ./makefile ${MyTempDir}
  cp ./readingAutomaton.* ${MyTempDir}
  cp ./automa.* ${MyTempDir}
  cd ${MyTempDir}
  echo "#cd ${MyTempDir}"
  echo
  echo "#Compiling ... "
  make  automa
else
  cd ${GREATSPN_SCRIPTDIR}/../inst_src
  echo "#cd ${GREATSPN_SCRIPTDIR}/../inst_src"
  MyTempDir=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`
  echo
  echo "#Copying file form ${GREATSPN_SCRIPTDIR}/../inst_src to ${MyTempDir}"
  echo
  cp ./class.* ${MyTempDir}
  cp ./makefile ${MyTempDir}
   cd ${MyTempDir}
  echo "#cd ${MyTempDir}"
  echo
  echo "#Compiling ... "
  make  normal
fi

echo
echo "#Executable file: $NET_PATH.solver"
echo
