#!/bin/sh
Localpth=/home/baarir
pthexe=$Localpth/TheLastVersion/dsrg/bin
pth=$Localpth/TheLastVersion/dsrg/bintools
Dsrg=$pthexe/dsrg
Rdsrg=$pthexe/rdsrg
draw=$pth/"draw-ctmc.sh"

if [ "$2" != "-r" ] && [ "$2" != "-m" ] && \
   [ "$2" != "-pm" ] && [ "$2" != "-pr" ] && \
   [ "$2" != "-pmk" ] && [ "$2" != "-prk" ] && [ "$2" != "-mk" ]; then
    echo "--------------------------------------------------------------------------------"
    echo " Usage : $0  model_name [-r,-pm,-pmk,-pr,-prk,-m,-mk] [-d,-p]          "
    echo "        -r   : compute DSRG structure (with inclusion)                 "
    echo "        -pm  : compute partially lumped Markov chain                   "
    echo "        -pmk : compute partially lumped Markov chain  (<=|RG|)         "
    echo "        -pr  : compute refined partially lumped Markov chain           "  
    echo "        -prk : compute refined partially lumped Markov chain (<=|RG|)  " 
    echo "        -m   : compute lumped Markov chain                             "
    echo "        -mk  : compute lumped Markov chain (<=|RG|)                    " 
    echo "        -d   : draw the markov chain using dotty 	                 "  
    echo "        -p   : draw the markov chain using ghostview	                 "
    echo "--------------------------------------------------------------------------------"

    exit
fi

if  [ "$3" != "" ] && [ "$3" != "-d" ] && [ "$3" != "-p" ] ; then
    echo "--------------------------------------------------------------------------------"
    echo " Usage : $0  model_name  [-r,-pm,-pmk,-pr,-prk,-m] [-d,-p]        "      
    echo "        -d  : draw the markov chain using dotty 	            "
    echo "        -p  : draw the markov chain using ghostview	            "
    echo "--------------------------------------------------------------------------------"
    exit
fi

name=$(basename $1)

if [ "$2" = "-r" ] ; then
    if [ "$3" = "-d" ] || [ "$3" = "-p" ] ; then 
	$Dsrg $name -i -o 
	$pth/gen-ctmc.pl "$1".dsrg > tmp 
	rm -f "$1".dsrg
	mv tmp "$name"idsrg.dot
	$draw  "$name"idsrg "$3"
    else
	$Dsrg $name -i 
    fi
fi

if [ "$2" = "-pm" ] ; then
    if [ "$3" = "-d" ] || [ "$3" = "-p" ]; then 
	$Dsrg $name -o -i -m
	$pth/gen-ctmc.pl "$1".dsrg > tmp 
	rm -f  "$1".dsrg
	mv tmp "$name"pdsrg.dot
	$draw  "$name"pdsrg "$3"
    else
	$Dsrg $name -i -m		
    fi
fi

if [ "$2" = "-pmk" ] ; then
    if [ "$3" = "-d" ] || [ "$3" = "-p" ]; then 
	$Dsrg $name -o -i -m -k
	$pth/gen-ctmc.pl "$1".dsrg > tmp 
	rm -f  "$1".dsrg
	mv tmp "$name"pkdsrg.dot
	$draw  "$name"pkdsrg "$3"
    else
	$Dsrg $name -i -m -k		
    fi
fi


if [ "$2" = "-pr" ] ; then
    $Dsrg $name  -i -m
    if [ "$3" = "-d" ] || [ "$3" = "-p" ]; then 
	$Rdsrg $name -a
	$pth/gen-ctmc.pl "$1".asciidsrg > tmp 
	rm -f "$1".asciidsrg
	mv tmp "$name"rpdsrg.dot
	$draw  "$name"rpdsrg "$3"
    else
	$Rdsrg $name -p		
    fi
    
fi

if [ "$2" = "-prk" ] ; then
    $Dsrg $name  -i -m -k
    if [ "$3" = "-d" ] || [ "$3" = "-p" ]; then 
	$Rdsrg $name -a
	$pth/gen-ctmc.pl "$1".asciidsrg > tmp 
	rm -f "$1".asciidsrg
	mv tmp "$name"rpkdsrg.dot
	$draw  "$name"rpkdsrg "$3"
    else
	$Rdsrg $name -p	
    fi
    
fi



if [ "$2" = "-m" ] ; then
    if [ "$3" = "-d" ] || [ "$3" = "-p" ] ; then 
	$Dsrg $name -o -m	
	$pth/gen-ctmc.pl "$1".dsrg > tmp 
	rm -f "$1".dsrg
	mv tmp "$name"cdsrg.dot
	$draw  "$name"cdsrg "$3"
    else
	$Dsrg $name -m
    fi
fi
if [ "$2" = "-mk" ] ; then
    if [ "$3" = "-d" ] || [ "$3" = "-p" ] ; then 
	$Dsrg $name -o -m -k	
	$pth/gen-ctmc.pl "$1".dsrg > tmp 
	rm -f "$1".dsrg
	mv tmp "$name"ckdsrg.dot
	$draw  "$name"ckdsrg "$3"
    else
	$Dsrg $name -m -k
    fi
fi
