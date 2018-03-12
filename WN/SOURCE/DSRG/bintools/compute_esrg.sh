#!/bin/sh
Localpth=/home/baarir/TheLastVersion
pthexe=$Localpth/dsrg/bin
greatexe=$Localpth/greatspn/$($Localpth/greatspn/SOURCES/contrib/version.sh)
pth=$Localpth/dsrg/bintools
Esrg=$greatexe/WNESRG
draw=$pth/"draw-ctmc.sh"

if  [ "$2" != "" ] && [ "$2" != "-r" ] && [ "$2" != "-m" ] ; then
    echo "--------------------------------------------------------------------------------"
    echo " Usage : $0  model_name [-r,-m] [-p]          "
    echo "        -r   : compute and print the ESRG structure in dot format               "
    echo "        -m   : compute and print the ESRG structure in compressed format        "
    echo "--------------------------------------------------------------------------------"

    exit
fi

if  [ "$2" = "-r" ] && [ "$3" != "" ] && [ "$3" != "-p" ] ; then
    echo "--------------------------------------------------------------------------------"
    echo " Usage : $0  model_name  [-r,-m] [-p]        "      
    echo "        -p  : draw the markov chain using ghostview	            "
    echo "--------------------------------------------------------------------------------"
    exit
fi

name=$(basename $1)

if [ "$2" = "-r" ] ; then
    if [ "$3" = "-p" ] ; then 
	$Esrg $name -o 
	$pth/gen-ctmc.pl "$1".esrg > tmp 
	rm -f "$1".esrg
	mv tmp "$name"esrg.dot
	$draw  "$name"esrg "$3"
    else
	$Esrg $name
    fi
fi

if [ "$2" = "-m" ] ; then
	$Esrg $name -m		
fi

