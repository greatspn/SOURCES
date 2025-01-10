#!/bin/bash

LIBSOLVE55_DIR=${1:-lp_solve_5.5}
if [ ! -d "${LIBSOLVE55_DIR}" ]; then
  echo "Missing LIBSOLVE55_DIR directory(${LIBSOLVE55_DIR})."
  exit 1
fi

if [ "$(uname)" = "Darwin" ]; then
  echo "OSX"
  RUN=ccc.osx
elif [ "$(uname)" = "Linux" ]; then
  echo "LINUX"
  RUN=ccc
else
  echo "Unsupported OS: $(uname)"
  exit 1
fi

echo build lpsolve lib
(
	cd ${LIBSOLVE55_DIR}/lpsolve55
	sh ${RUN}
)

echo build colamd
(
	cd ${LIBSOLVE55_DIR}/colamd
	gcc -c colamd.c -o colamd.o
	ar rcs libcolamd.a  colamd.o
)

echo install
(
	# only copy static libraries, to improve portability
	sudo cp ${LIBSOLVE55_DIR}/colamd/libcolamd.a \
			${LIBSOLVE55_DIR}/lpsolve55/bin/*/liblpsolve55.a \
			/usr/local/lib/
	# copy headers
	sudo cp ${LIBSOLVE55_DIR}/*.h lp_solve_5.5/colamd/colamd.h \
			/usr/local/include/
)
echo ok.






