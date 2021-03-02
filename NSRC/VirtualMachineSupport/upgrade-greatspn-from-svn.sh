#!/bin/bash

HOMEDIR=/home/greatspn

if [ ! -e ${HOMEDIR}/.greatspn-on-vbox ]; then
	echo 'This script should be run only on the VirtualBox image of GreatSPN.'
	exit 1
fi
#touch ${HOMEDIR}/.greatspn-on-vbox

# This script is intended to be executed in the virtualBox image
# of GreatSPN by double-clicking on the Desktop icon 'Upgrade GreatSPN'
# It executes the svn update and make/make install procedure

B1=$(tput bold)
B0=$(tput sgr0)
C1=$(tput setaf 4)
C0=$(tput setaf 9)
LN="${B1}===============================================================================${B0}"

echo $LN
echo "${B1}Updating GreatSPN sources from github...${B0}"
echo " WARNING: Repository will be reset to HEAD..."
echo $LN
read -p "Press any key to start upgrading..." -n1 -s
echo

# SVNLOGIN="--username vboxuser -password vboxuser --non-interactive"
# SVNLOGIN="--username vboxuser"
# (cd ${HOMEDIR}/GreatSPN/SOURCES && svn revert ${SVNLOGIN} --depth infinity . )
# (cd ${HOMEDIR}/GreatSPN/SOURCES && svn update ${SVNLOGIN} --accept theirs-full )
(cd ${HOMEDIR}/GreatSPN/SOURCES && git reset --hard HEAD && git pull )
if [ $? -eq 0 ]; then
	echo
	echo "${B1}Source tree has been upgraded. ${B0}"
else
	echo
	echo "${B1}Error: Could not upgrade GreatSPN from github. ${B0}"
	read -p "Press any key to quit..." -n1 -s
	exit 1
fi

# The rest of the installation is done by the updated vbox_install_script.sh script
${HOMEDIR}/GreatSPN/SOURCES/NSRC/VirtualMachineSupport/vbox_install_script.sh





