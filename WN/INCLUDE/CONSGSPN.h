#ifndef _CONS_H
#define _CONS_H


#define VAL_NUMB        10
#define TYPE       unsigned int
#define TYPE_P     TYPE *
#define SIZE_TYPE (sizeof(TYPE))
#define BIT_TYPE  (SIZE_TYPE*8)
#define MAX_IND   (unsigned int)((VAL_NUMB/BIT_TYPE)+1)
#define IND(x)    (unsigned int)((x-1) / BIT_TYPE)
#define POS(x)    (((x-1) % BIT_TYPE))
#define  min(a,b)((a <= b) ? a : b)

#define false            0
#define true             1

#define STR_SIZE       2048
#define PRESENT          -1
#define NOT_PRESENT      -2

#define IDF              -2
#define NOTYPE           -1
#define PRO              0
#define TOK              1
#define IDE              2
#define DEC              3

#define PROJECT           0
#define ELIMINATE        1

#define NOP             -1
#define ANDOP            1
#define OROP             2
#define EQOP             3
#define LQOP             4
#define GQOP             5
#define NEQOP            6
#define NLQOP            7
#define NGQOP            8

#endif




