#!/bin/bash

LIBSOLVE55_DIR=lp_solve_5.5
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

echo "Compilazione di lpsolve lib"
(
  cd ${LIBSOLVE55_DIR}/lpsolve55
  sh ${RUN} || (echo "Errore durante la compilazione di lpsolve!" && exit 1)
)

echo "Compilazione di colamd"
(
  cd ${LIBSOLVE55_DIR}/colamd
  gcc -c colamd.c -o colamd.o || (echo "Errore durante la compilazione di colamd!" && exit 1)
  ar rcs libcolamd.a colamd.o || (echo "Errore durante la creazione di libcolamd.a!" && exit 1)
)

echo "Installazione delle librerie"
(
  cp ${LIBSOLVE55_DIR}/colamd/libcolamd.a \
     ${LIBSOLVE55_DIR}/lpsolve55/bin/*/liblpsolve55.a \
     /usr/local/lib/ || (echo "Errore durante la copia delle librerie!" && exit 1)
  cp ${LIBSOLVE55_DIR}/*.h ${LIBSOLVE55_DIR}/colamd/colamd.h \
     /usr/local/include/ || (echo "Errore durante la copia degli header!" && exit 1)
)

echo "Verifica finale"
ls -l /usr/local/lib/libcolamd.a || (echo "libcolamd.a non trovato!" && exit 1)
echo "Build completata correttamente."

