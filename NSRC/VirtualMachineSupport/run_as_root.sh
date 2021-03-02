#!/bin/bash

if [ ! -e /home/user/.greatspn-on-vbox ]; then
	echo 'This script should be run only on the VirtualBox image of GreatSPN.'
	exit 1
fi

# Run as root the command passed as arguments
echo 'user' | sudo -p '' -S "$@"
