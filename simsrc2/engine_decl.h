/***********************************************************************
 *  File: engine_decl.h
 *  Date: Paris, July 17 1990
 *  Programmer: Giovanni Chiola
 ***********************************************************************/
#ifndef _ENGINE_DECL_H_
#define _ENGINE_DECL_H_

#include "simul_decl.h"

#define HOLD_EVENTS 2048
#define CHECKPOINT_SECS (unsigned long)15

#define EVENT_BACKUP		-1

#define EVENT_ASYNCHR_BRKP	-3
#define EVENT_SIMTIME_BRKP	-4

#define EVENT_END_SIM		-6


extern char *ecalloc();

struct Codbyt_descr {
    unsigned char shift;
    unsigned char mask;
    unsigned short num_byte;
    unsigned short link;
};

struct Place_descr {
    unsigned char traced;
    unsigned char mark;
    unsigned char lbound;
    unsigned char rubound;
    struct Place_descr *mark_next;
    struct Codbyt_descr cods;
};

typedef struct Place_descr *Place_p;

union Link_descr {
    struct Mnode_descr *node;
    unsigned nmark;
};


struct Mark_descr {
    unsigned char pval;
    struct Mnode_descr *father;
};


#define TST_INS_ARRAY 1
#define TST_INS_TREE 2

#define THRESHOLD_ARRAY 32

struct Byte_descr {
    unsigned char tst_ins;
    unsigned char noitems;
    unsigned char max_conf;
    unsigned short first_item;
};

typedef struct Byte_descr *Byte_p;


struct Test_descr {
    unsigned char mask;
    unsigned char shift;
    unsigned char offset;
    unsigned char test;
};

typedef struct Test_descr *Test_p;


struct Enabl_descr {
    unsigned char notestge;
    unsigned char notestlt;
    unsigned short nobyte;
    Test_p test_list;
};

typedef struct Enabl_descr *Enabl_p;

struct Change_descr {
    unsigned short place;
    short delta;
};

typedef struct Change_descr *Change_p;

/* Event list items */
struct Event_descr {
    short trans;
    struct Event_descr *e_prev;
    struct Event_descr *e_next;
    struct Event_descr *t_prev;
    struct Event_descr *t_next;
    struct Event_descr *p_a_link;
    struct Event_descr *p_d_link;
    struct Event_descr *p_d_where;
    struct Event_descr *p_t_where;
    double creation_time;
    double last_sched_desched_time;
    double residual_sched_time;
};

typedef struct Event_descr *Event_p;

struct Past_state_descr {
    int seed;
    Event_p sched_events;
    Event_p desched_events;
    Event_p add_age_events;
    Event_p rem_age_events;
    struct Past_state_descr *prev;
    double cur_time;
};

typedef struct Past_state_descr *Past_state_p;


union firing_param {
    double mean_t;
    double *f_list;
};

#include "../gsrc2/trans_def.h"


struct Cond_def {
    char op;
    char type;
    char top2;
    int p1;
    int op2;
};


union Val_type {
    double real;
    unsigned place;
};


struct Val_def {
    char op;
    char type;
    union Val_type val;
};


struct Clause_def {
    char type;
    int nocond;
    int noval;
    struct Val_def *vals;
    struct Cond_def *conds;
};


union MD_prob {
    double *prob;
    struct Clause_def *mdc;
};


/* Trans Description */
struct Trans_descr {
    unsigned char policy;
    unsigned char timing;
    unsigned char pri;
    unsigned char no_serv;
    unsigned char endep;
    unsigned char notestbytes;
    unsigned char nochangeplaces;
    unsigned char traced;
    unsigned char nnn;
    unsigned char enabled;
    unsigned char held;

    unsigned short tnum;
    unsigned short sort_index;
    unsigned short ecs;

    union firing_param f_time;
    union MD_prob md_p;

    Enabl_p test_bytes;
    Change_p change_places;
    unsigned short *add_l;
    unsigned short *test_l;

    Event_p instances_h;
    Event_p instances_t;

    struct Trans_descr *enabl_next;
};

typedef struct Trans_descr *Trans_p;


struct bre  {
    int nome;
    int next;
    int sub;
    int flag;
    int con;
    int lim;
    int segno;
    int abi;
    int log;
    unsigned char *pun;
};

typedef struct bre BRE;

extern int record_traces;
extern FILE **traces_f_a;
extern int *conflict_counts;

extern int break_point_flag;
extern int backup_flag;
extern int trace_flag;
extern int allowed_steps;

extern int cur_priority;
extern double cur_time;
extern double max_checkpoint_time;

extern int trans_num, place_num, no_cod;
extern unsigned char *cur_mark;

extern Trans_p t_list;
extern Place_p p_list;

extern Trans_p enab_list;
extern Place_p decoded_mark;

#endif
