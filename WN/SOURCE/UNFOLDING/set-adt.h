#include "generic-def.h"

#ifndef BITVECTOR
#define BITVECTOR

#define BYTESIZE 8

typedef char byte;

typedef struct {
    int setsize;
    byte *bytestream;
} set;

#define SIZE(S) ((S)->setsize)
#define BYTES(S) ((S)->bytestream)

status init_set(set *p_S, int size);
status set_insert(set *p_S, int element);
bool set_member(set *p_S, int element);

#endif

