
#define MAX_mult 8

typedef unsigned short ROWT;
#define ROWT_SIZE sizeof(ROWT)

typedef ROWT *ROWP;
#define ROWP_SIZE sizeof(ROWP)

struct place_obj {
    ROWT mark;
    ROWT enc;
    ROWT mult;
    ROWT covered;
    struct place_obj *next;
};

struct I_header {
    ROWT nouncov;
    ROWT cod_type;
    ROWT max_c;
    ROWT tcount;
    ROWP rowp;
    struct place_obj *p_l;
};

struct C_mark {
    ROWT max_c;
    ROWT cod_type;
    ROWT nobit;
    ROWT max_val;
    ROWT from0;
    ROWT flag;
    ROWT covered;
    ROWT displ;
    unsigned char offset[MAX_mult];
    unsigned char mask[MAX_mult];
    struct place_obj *p_l;
    struct C_mark *link;
};

#define POB_SIZE (unsigned)(sizeof(struct place_obj))
#define I_H_SIZE (unsigned)(sizeof(struct I_header))
#define C_M_SIZE (unsigned)(sizeof(struct C_mark))

