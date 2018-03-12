#ifndef _CONSSPOT_H
#define _CONSSPOT_H


#define VAL_NUMB        100
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

#define DF              0
#define INCLUDED        1
#define MAXIMUM         2
#define MAX_COMP        3


#define NOP             -1
#define ANDOP            1
#define OROP             2
#define EQOP             3
#define LQOP             4
#define LOP              5
#define GQOP             6
#define GOP              7
#define NEQOP            8
#define NLQOP            9
#define NLOP             10
#define NGQOP            11
#define NGOP             12

/********************** for perfs************************/
#define NBTOKEN          13
#define TR                0
#define PL                1
/*********************************************************/





#endif




