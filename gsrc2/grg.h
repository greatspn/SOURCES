#ifndef __GRG_H_
#define __GRG_H_

/* CODING POINTERS */

typedef unsigned char MARKP1;
#define INT1(x) ((unsigned long)(x))
#define ZERO1(x) (!((int)(x)))

struct s_mark_p2 {
    unsigned char b0;
    unsigned char b1;
};
typedef struct s_mark_p2 MARKP2;
#define INT2(x) (((unsigned long)((x).b0))+(((unsigned long)((x).b1))<<8))
#define ZERO2(x) (!((unsigned)((x).b0 | (x).b1) & 0xFF ))

struct s_mark_p3 {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
};
typedef struct s_mark_p3 MARKP3;
#define INT3(x) (((unsigned long)((x).b0))+(((unsigned long)((x).b1))<<8)\
			+(((unsigned long)((x).b2))<<16))
#define ZERO3(x) (!((unsigned)((x).b0 | (x).b1 | (x).b2) & 0xFF ))

union mark_p {
    MARKP1 c1;
    MARKP2 c2;
    MARKP3 c3;
};
typedef union mark_p MARKP;


//extern char * calloc();

/* MARKING DESCRIPTION RECORDS */ struct Mark_P {
    unsigned short *en_list;
    MARKP link;
    unsigned char pval;
};
typedef struct Mark_P MARK_P;


/* TRANSITION DESCRIPTION */ struct T_descr {
    int (*e_f)();
    int (*f_f)();
    unsigned short *add_l;
    unsigned short *test_l;
    unsigned short en_dep;
    unsigned short pri;
};


#endif
