#! /bin/sh

pth=/home/baarir/TheLastVersion/dsrg/bintools 
gv=kghostview
Dot=dot
Dotty=dotty

if [ "$1" = "" ] ; then
    echo "usage : $0 model_name [-p,-d]             "
    echo "        -p  : generate and view .ps file  "
    echo "        -d  : generate and view .dot file "
    exit
fi

name=$(basename $1)

if [ "$2" = "-d" ] ; then

 $Dotty "$name".dot 
 
fi

if [ "$2" = "-p" ] ; then

 $Dot "-Tps" "$name".dot > "$name".ps
 $gv "$name".ps 

fi
