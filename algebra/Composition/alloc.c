#define ___ALLOC___
#include "global.h"

char *emalloc(unsigned nbytes) {
    char 		*mallptr;	/* pointer returned by malloc */

    if ((mallptr = malloc(nbytes)) == NULL) {
        printf("emalloc: couldn't fill request for %d bytes\n",
               nbytes);
        exit(2);
    }

    return (mallptr);
}


char *erealloc(char *pointer, unsigned nbytes) {
    char 	*reallptrr;	/* pointer returned by malloc */

    if ((reallptrr = realloc(pointer, nbytes)) == NULL) {
        fprintf(stderr, "erealloc: couldn't fill request for %d bytes\n",
                nbytes);
        exit(2);
    }
    return (reallptrr);
}

