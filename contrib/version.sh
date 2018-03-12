#!/bin/sh
# SSS=`grep SunOS /etc/motd | awk '{ print \$3 }'`

SSS=`uname -r`

if env arch > /dev/null 2>&1
then
	echo `arch`_R$SSS
else
	if [ "`uname`" = "HP-UX" ]
	then
		echo HPPA_R$SSS
	else
		if [ "`uname --machine 2>/dev/null`" = "i686" ]
		then
			echo i686_R$SSS
		else
			#
			# default to IRIX
			#
			echo mips3_R$SSS
		fi
	fi
fi
