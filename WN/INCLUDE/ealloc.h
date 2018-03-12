#ifndef _EALLOC_H_
#define _EALLOC_H_

#include <stdio.h>

char *emalloc(size_t sz);
FILE *efopen(const char *file, const char *mode);
void *ecalloc(size_t nitm, size_t sz);

#endif
