#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No Color
SEP="${RED}---${NC}"


function help_msg {
	echo -e "${RED}=== GreatSPN Toolkit build script ===${NC}"
	echo -e "Usage:"
	echo -e "./run.sh build          # start build process"
	echo -e "./run.sh editor         # build and run the GreatSPN Java Editor"
	echo -e "./run.sh clean          # clean build files and quit"
	echo -e "./run.sh [-h|--help]    # display this help and quit"
	echo -e $SEP
	echo -e "Once the build process is complete, build files can be found in:"
	echo -e "$(pwd)/bin"
	echo -e $SEP
	echo -e "Installation directory (requires superuser privileges): ${RED}/usr/local/GreatSPN${NC}"
	echo -e "To install, type:"
	echo -e "(root)$ make install"
	echo
	exit 0
}
export -f help_msg

set -e

if [[ $# -eq 1 && $1 == "clean" ]]; then
	echo
	echo -e "${RED}=== Cleaning build files ===${NC}"
	echo
	make clean
	echo
	echo -e "${RED}=== Build files cleaned successfully ===${NC}"
	echo
	exit 0
elif [[ $# -eq 1 && ( $1 == "build" || $1 == "editor" ) ]]; then
	echo
	echo -e "${RED}=== Building the toolkit ===${NC}"
	echo
	make

	echo
	echo -e "${RED}=== Build completed successfully ===${NC}"
	echo
	if [[ $1 == "build" ]]; then
		echo -e "Build files written to $(pwd)/bin"
		echo -e $SEP
		echo -e "To run the Java Editor:"
		echo -e "./run.sh editor"
		echo -e "Full command:"
		echo -e "$ java -jar $(pwd)/objects/JavaGUI/bin/Editor.jar"
		echo -e $SEP
		echo -e "To install build files to /usr/local/GreatSPN:"
		echo -e "(root)$ make install"
		echo -e $SEP
		echo -e "Once installed, run the GreatSPN editor by typing:"
		echo -e "$ greatspn_editor         # default look and feel"
		echo -e "$ greatspn_editor -gtk    # GTK look and feel"
		echo -e $SEP
		echo -e "To clean object and build files:"
		echo -e "$ make clean"
		echo
	elif [[ $1 == "editor" ]]; then
		echo -e "${RED}=== Running GreatSPN Editor ===${NC}"
		echo
		java -jar $(pwd)/objects/JavaGUI/bin/Editor.jar
	fi
else
	help_msg
fi
