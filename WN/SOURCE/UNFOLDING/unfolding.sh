#!/bin/sh
. great_package.sh
${GREATSPN_BINDIR}/swn-translator $1
if [ $? -ne 0 ]
then
	rm -f $1_unf.*
	exit 5
fi
