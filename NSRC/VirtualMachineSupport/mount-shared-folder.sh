#!/bin/sh

# This script is intended to be executed in the virtualBox image
# of GreatSPN by double-clicking on the Desktop icon 'Mount Shared Folder'

if [ ! -e /home/user/.greatspn-on-vbox ]; then
	echo 'This script should be run only on the VirtualBox image of GreatSPN.'
	exit 1
fi

B1=$(tput bold)
B0=$(tput sgr0)
C1=$(tput setaf 4)
C0=$(tput setaf 9)
LN="${B1}===============================================================================${B0}"

echo $LN
echo "${B1}  Mounting shared folder ...${B0}"
#echo "${B1}  Type '${C1}fedora${C0}' when asked for a password${B0}"
echo $LN
echo
RUN=/home/user/GreatSPN/SOURCES/NSRC/VirtualMachineSupport/run_as_root.sh
${RUN} mount -t vboxsf /media/shared /media/vbox_Dati -o uid=`id -u user`

if [ $? -eq 0 ]; then
	echo "${B1}  Shared folder mounted. ${B0}"
	sleep 1
else
	echo "${B1}  Error: Shared folder NOT mounted. ${B0}"
	read -p "Press any key to quit..." -n1 -s
fi
