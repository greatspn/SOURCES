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
	echo "             -no-unfolding  Do not unfold, just load and save."
	echo "             -eval-cache    Use expr. evaluation cache (speeds up unfolding)."
	echo "             -no-suffix     Do not add the '_unf' suffix to the unfolded net name."
	
	exit
fi

java -mx16000m -cp ${GREATSPN_BINDIR}/Editor.jar:${GREATSPN_BINDIR}/lib/antlr-runtime-4.2.1.jar \
	editor.cli.UnfoldingCommand "$@"

