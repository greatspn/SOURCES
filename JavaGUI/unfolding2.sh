#!/bin/bash

. great_package.sh


if [[ $# == 0 ]] ; then
	echo "unfolding2  <netname>  <options>"
	echo "  Options:   -name-map      Save name map for PNML -> net/def name conversion."
	echo "             -shuffle       Randomly shuffle net elements."
	echo "             -out-pnml      Save in pnml format instead of using the GreatSPN format."
	echo "             -no-pnml-gfx   No graphics tags in the output pnml."
	echo "             -anonimize     Remove all names of places and transitions."
	echo "             -long-names    Unfold using long names, like 'move_x_c1_y_s2'."
	echo "             -short-names   Unfold using short names, like 'move_1_2'. [default]"
	
	exit
fi

java -mx16000m -cp /usr/local/GreatSPN/bin/Editor.jar:/usr/local/GreatSPN/bin/lib/antlr-runtime-4.2.1.jar \
	editor.UnfoldPNML2NetDefCommandLineTool "$@"

