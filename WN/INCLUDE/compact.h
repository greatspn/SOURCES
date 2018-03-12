#ifndef _COMPACT_H_
#define _COMPACT_H_

#include <stdio.h>

void store_compact(unsigned long nval, FILE *fp);
void load_compact(unsigned long *pp, FILE *fp);
void store_double(double *dval, FILE *fp);
void load_double(double *dval, FILE *fp);


#endif
