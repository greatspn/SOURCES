#ifndef __MEASURE_PN_H_
#define __MEASURE_PN_H_

#include "measure_decl.h"

int evaluate_cond(struct Cond_descr *cond, unsigned short *mrk);
void decode_mark(unsigned char *Mp, unsigned short *dmp);

#endif
