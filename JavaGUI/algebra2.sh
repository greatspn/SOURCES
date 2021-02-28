#!/bin/bash

. great_package.sh


if [[ $# == 0 ]] ; then
	echo "algebra2  <options>  net1  net2  outNet"
	echo "  Options:"
	echo "    -p <tags>  Comma-separated set of place tags."
	echo "    -t <tags>  Comma-separated set of transition tags."
	echo "    -out-pnml  Save in PNML format instead of using the GreatSPN format."
	echo "    -horiz     Lay out composed nets horizontally."
	echo "    -vert      Lay out composed nets vertically."
	echo "    --         End of options."
	
	exit
fi

java -mx16000m -cp ${GREATSPN_BINDIR}/Editor.jar:${GREATSPN_BINDIR}/lib/antlr-runtime-4.2.1.jar \
	editor.AlgebraCommandLineTool "$@"

