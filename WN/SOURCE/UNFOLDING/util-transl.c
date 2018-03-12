/*   util-transl.c   */

#include "util-transl.h"

static char *gErrorCode[] = {
    " %s: Unknown object type !\n",
    " %s: Object %s redeclared !\n",
    " %s: Object %s not declared!\n",
    " %s: Unknown parameter %s !\n",

    " %s: Unknown priority group %s !\n",
    " %s: Unknown place %s !\n",
    " %s: Unknown transition %s !\n",
    " %s: Unknown arc types !\n",

    " %s: Unknown lisp types for object %s !\n",
    " %s: Unknown variable %s !\n",
    " %s: Bound not found !\n",
    " %s: %s is not a subclass !\n",
    " %s: %s is not a class !\n",

    " %s: Unknown class type !\n",
    " %s: Undefined domain for place %s\n",
    " %s: Invalid domain for place %s\n",
    " %s: Undefined marking %s\n",

    " %s: Undefined domain binded to marking %s\n",
    " %s: Unknown subclass %s \n",
    " %s: Unknown dynamic subclass %s\n",
    " %s: Unknown class %s \n",

    " %s: Unknown token %s \n",
    " %s: Missing object\n",
    " %s: Successor function apply only to variable of an ordered class\n",
    " %s: Predecessor function apply only to variable of an ordered class\n",

    " %s: color domain of place %s has color repetitions, but the arc to (from) transition has no associate function \n",
    " %s: Too many components for arc function between place and transition %s\n",
    " %s: Missing elements of the subclass %s \n"
};



void *Emalloc(size_t nbytes) {
    void *mallptr;	/* pointer returned by malloc */

    if ((mallptr = malloc(nbytes)) == NULL) {
        fprintf(stderr, "Emalloc: couldn't fill request for %lu bytes\n", nbytes);
        exit(2);
    }
    return (mallptr);
}


void *Ecalloc(size_t nelem, size_t elsize) {
    void *callptrr;	/* pointer returned bycalloc */

    if ((callptrr = calloc(nelem, elsize)) == NULL) {
        fprintf(stderr, "Ecalloc: couldn't fill request for %lu bytes\n", elsize * nelem);
        exit(2);
    }
    return (callptrr);
}


void *Erealloc(void *pointer, size_t nbytes) {
    char *reallptrr;	/* pointer returned by realloc */

    if ((reallptrr = realloc(pointer, nbytes)) == NULL) {
        fprintf(stderr, "erealloc: couldn't fill request for %lu bytes\n", nbytes);
        exit(2);
    }

    return (reallptrr);
}


char *Estrdup(char *s1) {
    char *s2;

    if (s1 == NULL)
        return (NULL);

    if ((s2 = strdup(s1)) == 0) {
        fprintf(stderr, "Estrdup: couldn't duplicate string!\n");
        exit(2);
    }
    return (s2);
}


FILE *Efopen(char *filename, char *open_mode) {
    FILE *fp;

    if ((fp = fopen(filename, open_mode)) == NULL) {
        fprintf(stderr, "Efopen: couldn't open file %s\n", filename);
        exit(2);
    }
    return (fp);
}

void Error(enum ErrorTypes err_num, char *nome_funz, char *s) {
    if (s != NULL)
        fprintf(stderr, gErrorCode[err_num], nome_funz, s);
    else
        fprintf(stderr, gErrorCode[err_num], nome_funz);

    exit(2);
}



/* Alloca una stringa vuota*/
char *EmptyString() {
    char *string;
    string = (char *)Emalloc(1);
    *string = EOS;
    return (string);
}


/* Alloca una stringa costituita dalla concatenazione delle due stringhe di input*/
char *NewStringCat(char *old_name, char *ext) {
    char *string;

    string = (char *)Emalloc(strlen(old_name) + strlen(ext) + 1);
    sprintf(string, "%s%s", old_name, ext);
    return (string);
}


/* Cancella il carattere '\n' alla fine di una stringa se e' presente*/
char *EraseFinalCR(char *string) {
    int i;

    for (i = 0; (string[i] != EOS) && (string[i] != '\n'); i++)
        ;
    string[i] = EOS;

    return (string);
}


