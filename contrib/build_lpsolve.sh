#!/bin/bash

# Script to build lp_solve from sources on Linux and macOS.
# The folder of the lp_solve sources are expected to be in the cwd.
LIBSOLVE55_DIR=lp_solve_5.5
if [ ! -d "${LIBSOLVE55_DIR}" ]; then
	echo "Missing LIBSOLVE55_DIR directory(${LIBSOLVE55_DIR})."
	exit 1
fi

# Determine which os-dependent script to run.
case "$OSTYPE" in
  solaris*) 
	echo "SOLARIS"
	exit 1 
	;;
  darwin*)  
	echo "OSX"
	RUN=ccc.osx
	;;
  linux*)   
	echo "LINUX"
	RUN=ccc
	;;
  bsd*)     
	echo "BSD"
	exit 1 
	;;
  msys*)    
	echo "WINDOWS"
	exit 1 
	;;
  cygwin*)  
	echo "CYGWIN"
	RUN=ccc
	exit 1 
	;;
  *)        
	echo "unknown: $OSTYPE"
	exit 1 
	;;
esac


echo build lpsolve lib
(
	cd lp_solve_5.5/lpsolve55
	sh ${RUN}
)

echo build colamd
(
	cd lp_solve_5.5/colamd
	gcc -c colamd.c -o colamd.o
	ar rcs libcolamd.a  colamd.o
)

echo install
(
	# only copy static libraries, to improve portability
	sudo cp lp_solve_5.5/colamd/libcolamd.a \
			lp_solve_5.5/lpsolve55/bin/*/liblpsolve55.a \
			/usr/local/lib/
	# copy headers
	sudo cp lp_solve_5.5/*.h lp_solve_5.5/colamd/colamd.h \
			/usr/local/include/
)
echo ok.









