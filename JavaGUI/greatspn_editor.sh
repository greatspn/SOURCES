#!/bin/bash

. great_package.sh

echo "----------------------------------------------------------------------"
echo "  greatspn_editor: a Java-based interface for GreatSPN"
echo "    Options: -gtk    Use the GTK Look&Feel"
echo "----------------------------------------------------------------------"

LnF=""
if [[ $1 == '-gtk' ]] ; then
	LnF="-Dswing.defaultlaf=com.sun.java.swing.plaf.gtk.GTKLookAndFeel"
	shift 1
fi

# java -mx2000m -cp ${GREATSPN_BINDIR}/Editor.jar:${GREATSPN_BINDIR}/lib/antlr-runtime-4.2.1.jar \
# 	editor.Main "$@"

java -mx2000m ${LnF} -splash:${GREATSPN_BINDIR}/lib/splash.png \
	 -jar ${GREATSPN_BINDIR}/Editor.jar "$@"
