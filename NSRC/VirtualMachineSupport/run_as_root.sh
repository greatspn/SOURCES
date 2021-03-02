#!/bin/bash

HOMEDIR=/home/greatspn

if [ ! -e ${HOMEDIR}/.greatspn-on-vbox ]; then
	echo 'This script should be run only on the VirtualBox image of GreatSPN.'
	exit 1
fi

# Run as root the command passed as arguments
echo 'fedora' | sudo -p '' -S "$@"
