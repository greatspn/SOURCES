#!/bin/sh
. great_package.sh

${GREATSPN_BINDIR}/showtpd $1
if [ $? -ne 0 ]
then
	 echo "Solution failed in module showtpd"
fi
exit 0
