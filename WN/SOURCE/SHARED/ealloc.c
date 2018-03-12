#include <stdio.h>
#include <stdlib.h>
#include "../../INCLUDE/ealloc.h"

#define EXIT_FAILURE_ALLOC  250

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
char *emalloc(size_t sz) {
    /* Init emalloc */
    char *callptr;

    if ((callptr = malloc(sz)) == NULL) {
        /* Errore */
        fprintf(stdout, "Runtime Error: emalloc: couldn't fill request for %zu\n", sz);
        exit(EXIT_FAILURE_ALLOC);
    }/* Errore */
    return (callptr);
}/* End emalloc */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
FILE *efopen(const char *file, const char *mode) {
    /* Init efopen */
    FILE *callptr;
    if ((callptr = fopen(file, mode)) == NULL) {
        /* Errore */
        fprintf(stdout, "Runtime Error: fopen couldn't open %s, in mode %s\n", file, mode);
        exit(EXIT_FAILURE_ALLOC);
    }/* Errore */
    return (callptr);
}/* End efopen */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void *ecalloc(size_t nitm, size_t sz) {
    /* Init ecalloc */
    void *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        /* Errore */
        fprintf(stdout, "Runtime Error : ecalloc couldn't fill request for %zu, %zu\n", nitm, sz);
        exit(EXIT_FAILURE_ALLOC);
    }/* Errore */
    return (callptr);
}/* End ecalloc */
