#ifndef _ENGINE_PN_H
#define _ENGINE_PN_H

#include "engine_decl.h"

void place_visibility(int which, int how);
void trans_visibility(int which, int how);
void init_pn();
int decode_filter_mark(unsigned char *Mp);
void initialize_en_list(unsigned char *Mp);
void unfire_trans(int nt);
void fire_trans(int nt);

#endif
