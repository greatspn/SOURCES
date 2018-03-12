/*
 *  Paris, February 12, 1990
 *  program: gmt_stndrd.c
 *  purpose: Conversion of the TRG of a GSPN into a MC
 *           without net-dependent files compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No marking dependency is allowed for immediate transition weights.
 */

#define MAXSTRING 1024

/*
#define DEBUG
*/

#ifdef DEBUG
#define DEBUGWNGR 1
#define DEBUGCALLSINS 1

#define DEBUGCALLS
/*
*/

#endif

/*
#define PRINT_STATISTICS
*/


#define EPS 0.000000000001 /* The equivalence threshold for "double" */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sconst.h"
#include "../INCLUDE/compact.h"

/*#define MRKSLOT 100*/


/*#include "grg.h"*/

/* void (*terminate)(); */
void terminate() { exit(1); }

/*group_num, trans_num;*/

#include <stdio.h>
static double *rates = NULL;

FILE *nfp, * rgfp, * auxfp, *aefp, *trsfp, *van_path, *denom, *net_fp;
FILE *emcfp, * doufp, * marfp, *clmfp, *mptfp, *altth, *ratesfp, *outtype;


static int rate_mall_tot = 0;
static int rate_pop_tot = 0;
static int rate_push_tot = 0;


int load_trs, no_cod;

unsigned char *CMp;
unsigned short *DMp;

double eps, g_coef, l_coef;

double *M0, * d_p;


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

struct Trans_weight {
    unsigned short noecs;
    unsigned short endep;
    double rate;
    union MD_prob md_p;
};

struct ECS_descr {
    unsigned num;
    unsigned *set;
};

struct MARC_RAGG {         /* nodo utilizzato per il calcolo dei tassi */
    unsigned long from;
    unsigned long rate;
    unsigned long cont_tang;
    double mean_t;
    int f_p;
    struct MARC_RAGG *next;
};
typedef struct MARC_RAGG *MRate_p;

static MRate_p rate_free = NULL;   /* Free list per le result */
static MRate_p tang_head;
static MRate_p tang_tail;

#define UNKNOWN -2

static int ntr;
static int symbolic;
int exp_set = FALSE;
int fast_solve = TRUE;


/* TRANSITION ARRAY */ struct Trans_weight *t_list;
struct ECS_descr *aecs, *aecs_p;
char  filename[1024];

unsigned lastan, toptan;
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
char *emalloc(unsigned sz) {
    /* Init emalloc */
    char *callptr;

    if ((callptr = malloc(sz)) == NULL) {
        /* Errore */
        fprintf(stdout, "Runtime Error: emalloc: couldn't fill request for %d\n", sz);
        exit(1);
    }/* Errore */
    return (callptr);
}/* End emalloc */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
char *ecalloc(unsigned nitm, unsigned sz) {
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        /*        (*terminate)(); */
        terminate();
    }
    return (callptr);
}


struct d_tree_itm {
    double val;
    unsigned long l_s;
    unsigned long g_s;
    char balance;
};

struct d_tree_itm d_t_list[MAX_ITEMS];
char pathdir[L2_MAX_ITEMS];
unsigned top_d_tree = 0, root_d_tree = 0;
struct d_tree_itm *unbances;
struct d_tree_itm *unanfath;
struct d_tree_itm *father;
unsigned ancdepth;
unsigned depth = 0;


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void rebalance(struct d_tree_itm *newn) {
    char *p_d_p = &(pathdir[ancdepth]);
    register int ancomp = *(p_d_p++);
    register unsigned n_d_p = ((ancomp > 0) ? (unbances->g_s) :
                               (unbances->l_s));
    struct d_tree_itm *d_p = d_t_list + n_d_p;
    struct d_tree_itm *d_p_2 = d_p;
    register unsigned nn;
    register int aux;
#ifdef DEBUGCALLS
    fprintf(stderr, "        Start of rebalance newn=%p\n", newn);
#endif

    while (d_p_2 != newn) {    /* ADJUST BRANCH BALANCING  */
        if (*(p_d_p++) > 0) {
            d_p_2->balance = 1;
            nn = d_p_2->g_s;
        }
        else {
            d_p_2->balance = -1;
            nn = d_p_2->l_s;
        }
        d_p_2 = d_t_list + nn;
    }
    if (!(aux = unbances->balance)) {     /* HIGHER TREE  */
        unbances->balance = ancomp;
#ifdef DEBUGCALLS
        fprintf(stderr, "        End of rebalance (higher)\n");
#endif
        return;
    }
    if (aux != ancomp) {    /*  BALANCED TREE  */
        unbances->balance = 0;
#ifdef DEBUGCALLS
        fprintf(stderr, "        End of rebalance (balanced)\n");
#endif
        return;
    }
    /*  UNBALANCED TREE  */
    aux = d_p->balance;
    if (ancomp == aux) {    /*  SINGLE ROTATION  */
        d_p_2 = d_p;
        if (ancomp > 0) {
            unbances->g_s = d_p->l_s;
            nn = unbances - d_t_list;
            d_p->l_s = nn;
        }
        else {
            unbances->l_s = d_p->g_s;
            nn = unbances - d_t_list;
            d_p->g_s = nn;
        }
        unbances->balance = d_p->balance = 0;
    }
    else {  /*  DOUBLE ROTATION  */
        if (ancomp > 0) {
            nn = d_p->l_s;
            d_p_2 = d_t_list + nn;
            d_p->l_s = d_p_2->g_s;
            d_p_2->g_s = n_d_p;
            unbances->g_s = d_p_2->l_s;
            nn = unbances - d_t_list;
            d_p_2->l_s = nn;
        }
        else {
            nn = d_p->g_s;
            d_p_2 = d_t_list + nn;
            d_p->g_s = d_p_2->l_s;
            d_p_2->l_s = n_d_p;
            unbances->l_s = d_p_2->g_s;
            nn = unbances - d_t_list;
            d_p_2->g_s = nn;
        }
        aux = d_p_2->balance;
        if (aux == ancomp) {
            unbances->balance = -ancomp;
            d_p_2->balance = d_p->balance = 0;
        }
        else if (aux) {
            d_p_2->balance = unbances->balance = 0;
            d_p->balance = ancomp;
        }
        else
            unbances->balance = d_p->balance = 0;
    }
    /*  ROOT ADJUSTMENT  */
    nn = d_p_2 - d_t_list;
    if (unanfath == NULL)
        root_d_tree = nn;
    else {
        if (pathdir[ancdepth - 1] > 0)
            unanfath->g_s = nn;
        else
            unanfath->l_s = nn;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "        End of rebalance (normal)\n");
#endif
}


#ifdef PRINT_STATISTICS
unsigned max_nel = 0;
unsigned max_depth = 0;
unsigned long tot_depth = 0;
unsigned long num_call = 0;
#endif


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
unsigned long test_ins_d(double dval) {
    register unsigned nn = root_d_tree;
    struct d_tree_itm *d_p;
    register unsigned  udval_f, ldval_f;
    double udval, ldval;
    char *path = pathdir;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of test_ins_d %f\n", dval);
#endif
    if (nn) {
        depth = 0;
#ifdef PRINT_STATISTICS
        ++num_call;
#endif
        father = unanfath = NULL;
        unbances = d_t_list + nn;
        ancdepth = 0;
        udval_f = TRUE;;
        ldval_f = TRUE;;
        for (; ; father = d_p, ++depth, ++path) {
            d_p = d_t_list + nn;
            if (d_p->val == dval) {
#ifdef PRINT_STATISTICS
                tot_depth += depth;
                if (depth > max_depth)
                    max_depth = depth;
#endif
#ifdef DEBUGCALLS
                fprintf(stderr, "    End of test_ins_d (old #%d\n", nn);
#endif
                return (nn);
            }
            if (udval_f) {
                udval = dval * g_coef;
                udval_f = FALSE;
            }
            if (d_p->val > udval) {
                *path = -1;
                if (! d_p->l_s) {
                    d_p->l_s = nn = ++top_d_tree;
                    if (nn >= MAX_ITEMS) {
                        fprintf(stderr, "Sorry, increase constant MAX_ITEMS from %d to %d\n", MAX_ITEMS, nn);
                        exit(15);
                    }
                    d_p = d_t_list + nn;
                    d_p->val = dval; d_p->l_s = 0; d_p->g_s = 0;
                    d_p->balance = 0;
                    rebalance(d_p);
#ifdef PRINT_STATISTICS
                    tot_depth += depth;
                    if (depth > max_depth)
                        max_depth = depth;
#endif
#ifdef DEBUGCALLS
                    fprintf(stderr, "    End of test_ins_d (new #%d\n", nn);
#endif
                    return (nn);
                }
                nn = d_p->l_s;
                if (d_p->balance) {
                    unanfath = father; unbances = d_p; ancdepth = depth;
                }
            }
            else {
                if (ldval_f) {
                    ldval = dval * l_coef;
                    ldval_f = FALSE;
                }
                if (d_p->val < ldval) {
                    *path = 1;
                    if (! d_p->g_s) {
                        d_p->g_s = nn = ++top_d_tree;
                        if (nn >= MAX_ITEMS) {
                            fprintf(stderr, "Sorry, increase constant MAX_ITEMS from %d to %d\n", MAX_ITEMS, nn);
                            exit(15);
                        }
                        d_p = d_t_list + nn;
                        d_p->val = dval; d_p->l_s = 0; d_p->g_s = 0;
                        d_p->balance = 0;
                        rebalance(d_p);
#ifdef PRINT_STATISTICS
                        tot_depth += depth;
                        if (depth > max_depth)
                            max_depth = depth;
#endif
#ifdef DEBUGCALLS
                        fprintf(stderr, "    End of test_ins_d (new #%d\n", nn);
#endif
                        return (nn);
                    }
                    nn = d_p->g_s;
                    if (d_p->balance) {
                        unanfath = father; unbances = d_p; ancdepth = depth;
                    }
                }
                else {
#ifdef PRINT_STATISTICS
                    tot_depth += depth;
                    if (depth > max_depth)
                        max_depth = depth;
#endif
#ifdef DEBUGCALLS
                    fprintf(stderr, "    End of test_ins_d (old #%d\n", nn);
#endif
                    return (nn);
                }
            }
        }
    }
    d_p = d_t_list + 1;
    d_p->val = dval; d_p->l_s = 0; d_p->g_s = 0;
    d_p->balance = 0;
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of test_ins_d (first #1\n");
#endif
    return (root_d_tree = top_d_tree = 1);
}


struct grph_itm_r {
    unsigned long  tm;
    double rt;
};



struct grph_itm_r row_l[MAX_ROW];
struct grph_itm_r *glp;

unsigned long frm, tmp;
unsigned nel;




/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void insert_row(unsigned long to,
                double rate) {
    struct grph_itm_r *glp2 = row_l;
    register unsigned ii = glp - row_l;

#ifdef DEBUGCALLS
    fprintf(stderr, "            Start of insert_row to=%d rate=%f\n",
            (int)to, (float)rate);
#endif
    for (; ii-- ; ++glp2)
        if (glp2->tm == to) {
            glp2->rt += rate;
#ifdef DEBUGCALLS
            fprintf(stderr, "            End of insert_row (added to already present)\n");
#endif
            return;
        }
    glp2->tm = to; glp2->rt = rate; ++nel; ++glp;
#ifdef DEBUGCALLS
    fprintf(stderr, "            End of insert_row (normal)\n");
#endif
    if (nel >= MAX_ROW) {
        fprintf(stderr, "Sorry, increase constant MAX_ROW\n");
        exit(17);
    }
}


unsigned long lnel;


struct grph_itm_l {
    unsigned long	fm;
    unsigned long	rt;
    struct grph_itm_l 	*link;
};

struct grph_itm_l *free_p = NULL;

#ifdef TAIL
struct grph_itm_l **tail;
#endif
unsigned long  *head_p = NULL;

unsigned long *weight;
unsigned long max_tan = 0;


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void insert_graph(unsigned long from,
                  unsigned long to,
                  unsigned long rate) {
    unsigned long  old_head_p = 0;

    old_head_p = head_p[to];
    head_p[to] = ftell(emcfp);
    store_compact(from, emcfp);
    store_compact(rate, emcfp);
    store_compact(old_head_p, emcfp);
    M0[to] += 1.0;
}


unsigned long tmp;


unsigned long *w_p;
struct grph_itm_l **t_p;

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void store_transpose(unsigned long col) {
    unsigned long nel = 0;

    nel = M0[col];
    store_compact(nel, clmfp);
    store_compact(weight[col], clmfp);
    store_compact(head_p[col], mptfp);
    return;

#ifdef DEBUGCALLS
    fprintf(stderr, "    End of store_transpose\n");
#endif
}
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void read_NET_file() {
    /* Init read_NET_file */

    char tmp[MAXSTRING];
    char read_name[32];
    char type;
    int char_read;
    int inarc, outarc, inibarc;
    int int_val1, int_val2, int_val3, int_val4;
    int item_arc, item_skip, item;
    double float_val;
    float xcoord1, ycoord1;
    float xcoord2, ycoord2;
    float xcoord3, ycoord3;
    int el[6];

    while (TRUE) {
        /* Scanning .net file */
        if (fgets(tmp, MAXSTRING - 1, net_fp) == NULL)
            if (feof(net_fp))
                break;
        if (tmp[0] == '|' && tmp[1] == '\n') {
            /* Parsing della prima riga */
            fscanf(net_fp, "%c %d %d %d %d %d %d", &type, &el[0], &el[1], &el[2], &el[4], &el[3], &el[5]);
            while (getc(net_fp) != '\n');
            break;
        }/* Parsing della prima riga */
    }/* Scanning .def file */
    /*********** MARKING PARAMETERS ***********/
    if (el[0] > 0) {
        /* Ci sono marking parameters */
        for (item = 1; item <= el[0]; item++) {
            /* Lettura dei Marking Parameters */
            fgets(tmp, MAXSTRING - 1, net_fp);
        }/* Lettura dei Marking Parameters */
    }/* Ci sono marking parameters */
    /*********** PLACES ***********/
    if (el[1] > 0) {
        /* Ci sono posti */
        for (item = 1; item <= el[1]; item++) {
            /* Lettura dei Posti */
            fgets(tmp, MAXSTRING - 1, net_fp);
        }/* Lettura dei Posti */
    }/* Ci sono posti */
    /*********** RATE PARAMETERS ***********/
    if (el[2] > 0) {
        /* Ci sono rate parameters */
        for (item = 1; item <= el[2]; item++) {
            /* Lettura dei Rate Parameters */
            fgets(tmp, MAXSTRING - 1, net_fp);
        }/* Lettura dei Rate Parameters */
    }/* Ci sono rate parameters */
    /*********** PRIORITY GROUPS ***********/
    if (el[3] > 0) {
        /* Ci sono groups */
        for (item = 1; item <= el[3]; item++) {
            /* Lettura dei Groups */
            fgets(tmp, MAXSTRING - 1, net_fp);
        }/* Lettura dei Groups */
    }/* Ci sono groups */
    /*********** TRANSITIONS ***********/
    if (el[4] > 0) {
        /* Ci sono transizioni */
        for (item = 1; item <= el[4]; item++) {
            /* Lettura delle transizioni */
            fgets(tmp, MAXSTRING - 1, net_fp);

            sscanf(tmp, "%s %lg %d %d %d %d %f %f %f %f %f %f %n",
                   read_name,
                   &float_val,
                   &int_val1, &int_val2, &inarc, &int_val4,
                   &xcoord1, &ycoord1, &xcoord2, &ycoord2, &xcoord3, &ycoord3,
                   &char_read);
            if (int_val2)
                float_val = - float_val;
            if (float_val == -510) {
                /*tabt[ntr].md_rate_val=get_md_rate(ntr);*/
                rates[ntr] = float_val;
                fprintf(ratesfp, "%lg\n", float_val);
            }
            else if (float_val < 0) {
                rates[ntr] = float_val;
                fprintf(ratesfp, "%lg\n", float_val);
                /*rte = -float_val - 1;*/
                /*tabt[ntr].mean_t=tabrp[rte].rate_val;*/
            }
            else {
                /*tabt[ntr].mean_t = float_val;*/
                rates[ntr] = float_val;
                fprintf(ratesfp, "%lg\n", float_val);
            }
            /*********** INPUT ARCS *************/
            if (inarc != 0) {
                /* Ci sono archi di ingresso */
                for (item_arc = 1; item_arc <= inarc; item_arc++) {
                    /* Lettura di ogni arco di ingresso */
                    fgets(tmp, MAXSTRING - 1, net_fp);
                    sscanf(tmp, "%d %d %d %n",
                           &int_val1, &int_val2, &int_val3,
                           &char_read);
                    for (item_skip = 1; item_skip <= int_val3; item_skip++)
                        fgets(tmp, MAXSTRING - 1, net_fp);
                }/* Lettura di ogni arco di ingresso */
            }/* Ci sono archi di ingresso */
            /*********** OUTPUT ARCS *************/
            fgets(tmp, MAXSTRING - 1, net_fp);
            sscanf(tmp, "%d", &outarc);
            if (outarc != 0) {
                /* Ci sono archi di uscita */
                for (item_arc = 1; item_arc <= outarc; item_arc++) {
                    /* Lettura di ogni arco di ingresso */
                    fgets(tmp, MAXSTRING - 1, net_fp);
                    sscanf(tmp, "%d %d %d %n",
                           &int_val1, &int_val2, &int_val3,
                           &char_read);
                    for (item_skip = 1; item_skip <= int_val3; item_skip++)
                        fgets(tmp, MAXSTRING - 1, net_fp);
                }/* Lettura di ogni arco di ingresso */
            }/* Ci sono archi di uscita */
            /*********** INHIBITOR ARCS *************/
            fgets(tmp, MAXSTRING - 1, net_fp);
            sscanf(tmp, "%d", &inibarc);

            if (inibarc != 0) {
                /* Ci sono archi inibitori */
                for (item_arc = 1; item_arc <= inibarc; item_arc++) {
                    /* Lettura di ogni arco di ingresso */
                    fgets(tmp, MAXSTRING - 1, net_fp);
                    sscanf(tmp, "%d %d %d %n",
                           &int_val1, &int_val2, &int_val3,
                           &char_read);
                    for (item_skip = 1; item_skip <= int_val3; item_skip++)
                        fgets(tmp, MAXSTRING - 1, net_fp);
                }/* Lettura di ogni arco di ingresso */
            }/* Ci sono archi inibitori */
            ntr++;
        }/* Lettura delle transizioni */
    }/* Ci sono transizioni */
}/* End read_NET_file */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init() {
    int ii;
    char temp[1024];

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif

    fscanf(outtype, "%d", &symbolic);
    fgets(temp, 1024, altth);
    ntr = atoi(temp);
    if (exp_set) {
        /* Opzione per set di esperimenti */
        rates = (double *)ecalloc(ntr, sizeof(double));
        ntr = 0;
        read_NET_file();
        ntr = atoi(temp);
    }/* Opzione per set di esperimenti */

    fscanf(auxfp, "toptan= %d\n", &toptan);
#ifdef TAIL
    tail = (struct grph_itm_l **)ecalloc(toptan + 1, sizeof(struct grph_itm_l *));
#endif
    head_p = (unsigned long *)ecalloc(toptan + 1, sizeof(unsigned long));
    weight = (unsigned long *)ecalloc(toptan + 1, sizeof(unsigned long));
    M0 = (double *)ecalloc(toptan + 1, sizeof(double));
    for (d_p = M0, ii = toptan ; ii-- ; * (++d_p) = 0.0);
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}




/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void load_initialmark() {
    int nm;
    double pp;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of load_initialmark\n");
#endif
    fgets(filename, 1024, rgfp);
    nm = atoi(filename);
    /*fscanf( rgfp,"%d\n", &nm );*/
    if (nm) {
        M0[nm] = 1.0;
#ifdef DEBUG
        fprintf(stderr, "      M1 with prob. 1.0\n");
#endif
    }
    else {
        for (fscanf(rgfp, "%d", &nm) ; nm ;
                fscanf(rgfp, "%d", &nm)) {
            fscanf(rgfp, "%lg\n", &pp);
#ifdef DEBUG
            fprintf(stderr, "      M%d with prob. %f\n", nm, pp);
#endif
            M0[nm] = pp;
        }
        while (getc(rgfp) != '\n');
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of load_initialmark\n");
#endif
}
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_rate_list(rate_head, rate_tail)
MRate_p rate_head;
MRate_p rate_tail;
{
    /* Init push_rate_list */
    if (rate_head != NULL) {
        rate_tail->next = rate_free;
        rate_free = rate_head;
    }
}/* End push_rate_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_rate_element(rate_ptr)
MRate_p rate_ptr;
{
    /* Init push_rate_element */

    rate_push_tot++;
    rate_ptr->next = rate_free;
    rate_free = rate_ptr;
}/* End push_rate_element */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
MRate_p rate_pop() {
    /* Init rate_pop */
    MRate_p ptr;

    rate_pop_tot++;
    if (rate_free == NULL) {
        rate_mall_tot++;
        ptr = (MRate_p)emalloc(sizeof(struct MARC_RAGG));
        ptr->cont_tang = UNKNOWN;
        ptr->mean_t = UNKNOWN;
        ptr->f_p = UNKNOWN;
        ptr->next = NULL;
    }
    else {
        ptr = rate_free;
        rate_free = rate_free->next;
    }
    return (ptr);
}/* End rate_pop */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void append_to_list_of_tangible_reachable(unsigned long nm,
        double rr,
        MRate_p *head,
        MRate_p *tail) {
    /* Init append_to_list_of_tangible_reachable */
    MRate_p pun = NULL;
    MRate_p add_p = NULL;


    if (*head == NULL && *tail == NULL) {
        add_p = rate_pop();
        add_p->cont_tang = nm;
        add_p->mean_t =  rr;
        *head = add_p;
        *tail = add_p;
    }
    else {
        for (pun = *head; pun != NULL; pun = pun->next)
            if (pun->cont_tang == nm) {
                pun->mean_t += rr;
                return;
            }
        add_p = rate_pop();
        add_p->cont_tang = nm;
        add_p->mean_t =  rr;
        (*tail)->next = add_p;
        *tail = add_p;
        (*tail)->next = NULL;
    }
    if ((*tail) != NULL)
        (*tail)->next = NULL;
}/* End append_to_list_of_tangible_reachable */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void extract_list_of_tangible(int file_pos) {
    MRate_p current, add_p, top_mark;
    unsigned long nm, tot;
    unsigned long nfatt, ndenom;
    unsigned long tr;
    unsigned long en_degree  = 1;
    unsigned long ordinary_m = 1;
    unsigned long denom_p;

    unsigned long tr_d;
    unsigned long ordinary_m_d = 1;
    int isfile_p;
    double rr, denomin;




    top_mark = NULL;
    fseek(rgfp, file_pos, 0);
    load_compact(&tot, rgfp);
    for (; tot ; --tot) {
        load_compact(&nm, rgfp);
        if (exp_set) {
            /* Opzione per set di esperimenti */
            load_compact(&tr, rgfp);
            load_compact(&en_degree, rgfp);
            if (symbolic)
                load_compact(&ordinary_m, rgfp);
            if (tr < ntr) {
                /* vera marcatura tangibile raggiunta */
                rr = rates[tr] * en_degree * ordinary_m;
                append_to_list_of_tangible_reachable(nm, rr, &tang_head, &tang_tail);
            }/* vera marcatura tangibile raggiunta */
            else {
                /* chiamata ricorsiva perche' puntatore a lista */
                add_p = rate_pop();
                rr = rates[tr - ntr] * en_degree * ordinary_m;
                add_p->mean_t = rr;
                add_p->f_p = nm; /* c'e il -nm nel vecchio */
                add_p->next = top_mark;
                top_mark = add_p;
            }/* chiamata ricorsiva perche' puntatore a lista */
        }/* Opzione per set di esperimenti */
        if (!exp_set) {
            /* Opzione per unico run */
            load_double(&rr, rgfp);
            if (rr >= 0) {
                /* vera marcatura tangibile raggiunta */
                append_to_list_of_tangible_reachable(nm, rr, &tang_head, &tang_tail);
            }/* vera marcatura tangibile raggiunta */
            else {
                /* chiamata ricorsiva perche' puntatore a lista */
                add_p = rate_pop();
                add_p->mean_t = -rr;
                add_p->f_p = nm; /* c'e il -nm nel vecchio */
                add_p->next = top_mark;
                top_mark = add_p;
            }/* chiamata ricorsiva perche' puntatore a lista */
        }/* Opzione per unico run */
    }
    if (!fast_solve) {
        /* Opzione per soluzione veloce ma files grossi */
        while (top_mark != NULL) {
            /* Per ogni elemento dello stack */
            /***** POP DELLO STACK ******/
            current = top_mark;
            top_mark = top_mark->next;
            /***** POP DELLO STACK ******/

            fseek(van_path, current->f_p, 0);
            load_compact(&tot, van_path);
            for (; tot ; --tot) {
                load_compact(&nm, van_path);
                if (exp_set) {
                    /* Opzione per set di esperimenti */
                    load_compact(&nfatt, van_path);
                    rr = 1.0;
                    isfile_p = FALSE;
                    for (; nfatt ; --nfatt) {
                        load_compact(&tr, van_path);
                        if (symbolic)
                            load_compact(&ordinary_m, van_path);
                        load_compact(&denom_p, van_path);
                        denomin = 0.0;
                        fseek(denom, denom_p, 0);
                        load_compact(&ndenom, denom);
                        for (; ndenom ; --ndenom) {
                            load_compact(&tr_d, denom);
                            if (symbolic)
                                load_compact(&ordinary_m_d, denom);
                            denomin += rates[tr_d] * ordinary_m_d;
                        }
                        if (tr >= ntr) {
                            isfile_p = TRUE;
                            tr -= ntr;
                        }
                        rr *= ((rates[tr] * ordinary_m) / denomin);
                    }
                    if (isfile_p == FALSE) {
                        /* vera marcatura tangibile raggiunta */
                        append_to_list_of_tangible_reachable(nm, current->mean_t *rr, &tang_head, &tang_tail);
                    }/* vera marcatura tangibile raggiunta */
                    else {
                        /* chiamata ricorsiva perche' puntatore a lista */
                        add_p = rate_pop();
                        add_p->mean_t = current->mean_t *rr;  /* c'e rr nel vecchio */
                        add_p->f_p = nm; /* c'e' il -nm nel vecchio */
                        add_p->next = top_mark;
                        top_mark = add_p;
                    }/* chiamata ricorsiva perche' puntatore a lista */
                }/* Opzione per set di esperimenti */
                if (!exp_set) {
                    /* Opzione per unico run */
                    load_double(&rr, van_path);
                    if (rr >= 0) {
                        /* vera marcatura tangibile raggiunta */
                        append_to_list_of_tangible_reachable(nm, current->mean_t *rr, &tang_head, &tang_tail);
                    }/* vera marcatura tangibile raggiunta */
                    else {
                        /* chiamata ricorsiva perche' puntatore a lista */
                        add_p = rate_pop();
                        add_p->mean_t = current->mean_t *(-rr);  /* c'e rr nel vecchio */
                        add_p->f_p = nm; /* c'e' il -nm nel vecchio */
                        add_p->next = top_mark;
                        top_mark = add_p;
                    }/* chiamata ricorsiva perche' puntatore a lista */
                }/* Opzione per unico run */
            }
            push_rate_element(current);
        }/* Per ogni elemento dello stack */
    }/* Opzione per soluzione veloce ma files grossi */
}

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void load_trg() {
    unsigned long no_trans;
    double sum;
    int next_print;
    int prevtan;
    MRate_p pun, nxt;
#ifndef DEBUGWNGR
    //  FILE *wnfp;
    // wnfp=fopen("chain","w");
#endif

    prevtan = 0;
    next_print = 0;
    for (lastan = 1 ; ; lastan++) {
        /* Reading loop */
        if (lastan <= toptan) {
            tang_head  = NULL;
            tang_tail  = NULL;
            extract_list_of_tangible(ftell(rgfp));
            pun = tang_head;
        }
        do {
            if (lastan > prevtan) {
                if (prevtan > 0) {
                    if (nel) {
                        sum = 1.0 / sum;
                        frm = test_ins_d(sum);
                        weight[prevtan] = frm;
                        for (glp = row_l, no_trans = nel ; no_trans-- ; ++glp) {
                            glp->rt *= sum;
                            frm = test_ins_d(glp->rt);
                            insert_graph(prevtan, glp->tm, frm);
#ifndef DEBUGWNGR
                            //      fprintf(wnfp,"DA %d A %d RATE %lg\n",prevtan,glp->tm,d_t_list[frm].val);
#endif
                        }
                    }
                    else {
                        frm = test_ins_d(0.0);
                        weight[prevtan] = frm;
                    }
                }
                if (lastan > toptan)
                    goto finish;
                prevtan = lastan;
                sum = 0.0;
                nel = 0; glp = row_l;
            }
            if (tang_head != NULL) {
                nxt = pun->next;
                if (pun->cont_tang) {
                    if (pun->cont_tang != lastan) {
                        sum += pun->mean_t;
                        insert_row(pun->cont_tang, pun->mean_t);    /* INCREMENTS "nel" and "glp" */
                    }
                }
                /*else while ( getc(rgfp) != '\n' );*/
                pun = nxt;
            }
            else
                nxt = NULL;
#ifdef PRINT_STATISTICS
            if (nel > max_nel)
                max_nel = nel;
#endif
        }
        while (nxt != NULL);
        push_rate_list(tang_head, tang_tail);
        if (lastan >= next_print) {
            next_print += 100;
            /*fprintf(stderr," %6d\r", lastan);*/
        }
    }/* Reading loop */
finish: fprintf(stderr, " %6d rows computed\n\n", toptan);
#ifndef DEBUGWNGR
    //fclose(wnfp);
#endif
}


static char   *can_t_open = "Can't open file %s for %c\n";


#include <sys/types.h>
#include <sys/stat.h>


#ifdef PRINT_STATISTICS
#include <sys/time.h>
#include <sys/resource.h>
#define RUSAGE_SELF 0
#endif


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int main(int argc,
         char **argv,
         char **envp) {
    unsigned long jj;
#ifdef PRINT_STATISTICS
    struct rusage ru;
    float mean_depth;
#endif


#ifdef DEBUG
    fprintf(stderr, "Start of gmt_stndrd\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    eps = EPS;
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii--) {
            s_p = *(a_p++);
            if (*(s_p++) != '-' ||
                    (*(s_p) != 'e' && *(s_p) != 'b' && *(s_p) != 'l')) {
                fprintf(stderr, "ERROR: unknown parameter '%s'\n", s_p - 1);
                exit(33);
            }
            switch (*s_p) {
            case 'e': sscanf((s_p + 2), "%lf", &eps);
                break;
            case 'l': fast_solve = FALSE;
                break;
            case 'b': exp_set = TRUE;
                fast_solve = FALSE;
                break;
            }
        }
    }
    g_coef = 1.0 + eps;
    l_coef = 1.0 - eps;
    sprintf(filename, "%s.net", argv[1]);
    if ((net_fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.outtype", argv[1]);
    if ((outtype = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((auxfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.throu", argv[1]);
    if ((altth = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.rates", argv[1]);
    if ((ratesfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init();
    (void)fclose(auxfp);
    (void)fclose(outtype);
    (void)fclose(ratesfp);
    sprintf(filename, "%s.van_path", argv[1]);
    if ((van_path = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.wngr", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.denom", argv[1]);
    if ((denom = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_initialmark();
    sprintf(filename, "%s.epd", argv[1]);
    if ((marfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    M0[0] = 0.0; store_double(M0, marfp);
    store_double(M0, marfp);
    {
        unsigned ii;
        for (d_p = M0, ii = toptan ; ii-- ;) {
            store_double(++d_p, marfp);
            store_double(d_p, marfp);
            *d_p = 0.0;
        }
    }
    (void) fclose(marfp);

    sprintf(filename, "%s.emc", argv[1]);
    if ((emcfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    load_trg();
    (void) fclose(rgfp);
    (void) fclose(van_path);
    (void) fclose(denom);
    (void) fclose(altth);
    (void) fclose(emcfp);
    (void) fclose(net_fp);

    sprintf(filename, "%s.mpt", argv[1]);
    if ((mptfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s.clm", argv[1]);
    if ((clmfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    store_compact(toptan, clmfp);

    {
        unsigned ii;
        for (ii = toptan, jj = 1 ; ii-- ; jj++) {
            store_transpose(jj);
        }
    }

    nel = 0;  store_compact(nel, clmfp);
    store_compact(nel, clmfp);
    (void) fclose(clmfp);
    (void) fclose(mptfp);

    sprintf(filename, "%s.doubles", argv[1]);
    if ((doufp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    frm = top_d_tree;
    store_compact(frm, doufp);
    {
        unsigned ii;
        for (ii = 0 ; ii++ < top_d_tree ;)
            store_double(&(d_t_list[ii].val), doufp);
    }
    (void) fclose(doufp);

    /* end_main: */
#ifdef PRINT_STATISTICS
    getrusage(RUSAGE_SELF, &ru);
    fprintf(stderr, "\nnumber of doubles : %d\n", frm);
    fprintf(stderr, "maximum row length : %d\n", max_nel);
    fprintf(stderr, "maximum tree depth : %d\n", max_depth);
    mean_depth = tot_depth;
    mean_depth /= num_call;
    fprintf(stderr, "average tree depth : %f\n", mean_depth);
    fprintf(stderr, "\nCPU time (s.) : user %d, system %d\n",
            ru.ru_utime.tv_sec, ru.ru_stime.tv_sec);
    fprintf(stderr, "Max memory size : %d Kbyte\n",
            ru.ru_maxrss * 2);
    fprintf(stderr, "Page faults : reclaims %d, faults %d\n",
            ru.ru_minflt, ru.ru_majflt);
    fprintf(stderr, "Context Switches : voluntary %d, forced %d\n",
            ru.ru_nvcsw, ru.ru_nivcsw);
    fprintf(stderr, "Swap : %d\n\n",
            ru.ru_nswap);
    fprintf(stdout, "--------------- RATE PATH STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , rate_mall_tot, rate_push_tot, rate_pop_tot);
#endif
#ifdef DEBUG
    fprintf(stderr, "End of gmt_stndrd\n");
#endif
    return 0;
}

