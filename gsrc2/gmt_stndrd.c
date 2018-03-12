/*
 *  Paris, February 12, 1990
 *  program: gmt_stndrd.c
 *  purpose: Conversion of the TRG of a GSPN into a MC
 *           without net-dependent files compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No marking dependency is allowed for immediate transition weights.
 *  WARNING: increased constant L2_MAX_ITEMS in const.h to avoid segmentation
 *           fault in procedure rebalance.
 */


/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif

#define PRINT_STATISTICS
/*
*/


#define EPS 0.00000001 /* The equivalence threshold for "double" */

#include <stdlib.h>
#include <math.h>
#include "const.h"
#include "../WN/INCLUDE/compact.h"

#define MYMRKSLOT 100


#include "grg.h"

static int group_num, trans_num;

#include <stdio.h>

FILE *nfp, * rgfp, * auxfp, *aefp, *trsfp;
FILE *matfp, * doufp, * marfp;


static int load_trs, no_cod;

static unsigned char *CMp;
static unsigned short *DMp;
static double eps, g_coef, l_coef;
static double *M0, * d_p;


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


/* TRANSITION ARRAY */ struct Trans_weight *t_list;
struct ECS_descr *aecs, *aecs_p;

static unsigned lastan, toptan;

void *
ecalloc(unsigned nitm, unsigned sz) {
    void               *callptr;

// #ifdef Linux
    if (nitm == 0)
        nitm = 1;
// #endif

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(1);
    }
    return (callptr);
}

#ifdef PRINT_STATISTICS
static unsigned max_nel = 0;
static unsigned max_depth = 0;
static unsigned long tot_depth = 0;
static unsigned long num_call = 0;
#endif

struct d_tree_itm {
    double val;
    unsigned short l_s;
    unsigned short g_s;
    char balance;
};
static struct d_tree_itm d_t_list[MAX_ITEMS];
static char pathdir[L2_MAX_ITEMS];
static unsigned top_d_tree = 0, root_d_tree = 0;
static struct d_tree_itm *unbances;
static struct d_tree_itm *unanfath;
static struct d_tree_itm *father;
static unsigned ancdepth;
static unsigned depth = 0;


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
    fprintf(stderr, "        Start of rebalance newn=%d\n", (int)newn);
#endif

    while (d_p_2 != newn) {    /* ADJUST BRANCH BALANCING  */
        if (p_d_p - pathdir >= L2_MAX_ITEMS) {
            fprintf(stderr, "rebalance(): Increase L2_MAX_ITEMS!  depth=%ld\n", p_d_p - pathdir);
            exit(1);
        }
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

unsigned short test_ins_d(double dval) {
#ifdef DEBUG3
    fprintf(stderr, "ins_d %.15lf %d\n", dval, top_d_tree);
#endif
    register unsigned nn = root_d_tree;
    struct d_tree_itm *d_p;
    register unsigned  udval_f, ldval_f;
    double udval, ldval = 0;
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
            if (path - pathdir > L2_MAX_ITEMS) {
                fprintf(stderr, "test_ins_d(): Increase L2_MAX_ITEMS!  depth=%ld\n", path - pathdir);
                exit(1);
            }
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
                        fprintf(stderr, "(1) Sorry, increase constant MAX_ITEMS.\n");
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
                            fprintf(stderr, "(2) Sorry, increase constant MAX_ITEMS.\n");
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
unsigned num_doubles() { return top_d_tree; }
double ith_double(unsigned short ii) { return d_t_list[ii].val; }

// unsigned short test_ins_d2(double dval);
// #include <map>
// #include <vector>
// std::map<double, short> double_map;
// std::vector<double> double_vec;
// unsigned short test_ins_d2(double dval) {
//     std::map<double, short>::const_iterator it = double_map.find(dval);
//     if (double_map.end() != it) {
//         return it->second;
//     }
//     double_map[dval] = double_map.size();
//     double_vec.push_back(dval);
//     fprintf(stderr, "size=%d %lf\n", double_map.size(), dval);
//     return double_map[dval];
// }
// #define test_ins_d test_ins_d2
// unsigned num_doubles() { return double_map.size(); }
// double ith_double(unsigned short ii) { return double_vec[ii]; }




struct grph_itm_r {
    unsigned long  tm;
    double rt;
};

static struct grph_itm_r row_l[MAX_ROW];
static struct grph_itm_r *glp;

static unsigned long frm;
static unsigned nel;




void insert_row(unsigned long to, double rate) {
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
    glp2->tm = to;
    glp2->rt = rate;
    ++nel;
    ++glp;
#ifdef DEBUGCALLS
    fprintf(stderr, "            End of insert_row (normal)\n");
#endif
    if (nel >= MAX_ROW) {
        fprintf(stderr, "Sorry, increase constant MAX_ROW\n");
        exit(17);
    }
}


// static unsigned long lnel;


struct grph_itm_l {
    unsigned long	fm;
    unsigned short	rt;
    struct grph_itm_l 	*link;
};

static struct grph_itm_l *free_p = NULL;

static struct grph_itm_l **tail;
static unsigned short *weight;
static unsigned long max_tan = 0;


struct grph_itm_l *new_grph() {
    struct grph_itm_l *g_p = free_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "          Start of new_grph\n");
#endif
    if (g_p == NULL) {
        struct grph_itm_l *ret_p;
        register unsigned ii = MYMRKSLOT;
        g_p = (struct grph_itm_l *)ecalloc(ii, sizeof(struct grph_itm_l));
        ret_p = g_p;
        free_p = ++g_p;
        for (--ii ; --ii ; ++g_p)
            g_p->link = g_p + 1;
        g_p->link = NULL;
#ifdef DEBUGCALLS
        fprintf(stderr, "          End of new_grph (alloc slot)\n");
#endif
        return (ret_p);
    }
    free_p = g_p->link;
#ifdef DEBUGCALLS
    fprintf(stderr, "          End of new_grph (normal)\n");
#endif
    return (g_p);
}

void insert_graph(unsigned long from, unsigned long  to,
                  unsigned short rate) {
    struct grph_itm_l *g_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of insert_graph from=%d to=%d rate=%d\n",
            (int)from, (int)to, (int)rate);
#endif
    if (to > max_tan) {    /* NEW COLUMN */
        register unsigned ii = toptan - max_tan;
        struct grph_itm_l **g_pp = tail + max_tan;
        unsigned long incr = ((ii > MYMRKSLOT) ? MYMRKSLOT : ii);
        incr = ii + to - max_tan;
        for (++ii, max_tan = to + incr ; --ii ;)
            *++g_pp = NULL;
        g_pp = tail + to;  *g_pp = g_p = new_grph();
        g_p->link = g_p; g_p->fm = from; g_p->rt = rate;
#ifdef DEBUGCALLS
        fprintf(stderr, "      End of insert_graph (new column)\n");
#endif
        return;
    }
    if ((g_p = tail[to]) == NULL) {    /* INSERT FIRST ITEM */
        tail[to] = g_p = new_grph();
        g_p->link = g_p; g_p->fm = from; g_p->rt = rate;
#ifdef DEBUGCALLS
        fprintf(stderr, "      End of insert_graph (first item)\n");
#endif
        return;
    }
    if (g_p->fm < from) {    /* TAIL INSERTION */
        struct grph_itm_l *g_p2 = new_grph();
        tail[to] = g_p2;
        g_p2->link = g_p->link; g_p2->fm = from; g_p2->rt = rate;
        g_p->link = g_p2;
#ifdef DEBUGCALLS
        fprintf(stderr, "      End of insert_graph (tail insertion)\n");
#endif
        return;
    }
    {
        struct grph_itm_l *g_p2 = g_p->link;
        while (g_p2->fm < from) {
            g_p = g_p2;  g_p2 = g_p2->link;
        }
        g_p = g_p->link = new_grph();
        g_p->link = g_p2; g_p->fm = from; g_p->rt = rate;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of insert_graph (normal)\n");
#endif
}



//unsigned long tmp;


static unsigned short *w_p;
static struct grph_itm_l **t_p;

void store_transpose() {
    unsigned long tmp = *w_p;
    unsigned long nel = 0;
    struct grph_itm_l *g_p = *t_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of store_transpose %d\n", w_p - weight);
#endif
    if (g_p == NULL) {
#ifdef DEBUG
        fprintf(stderr, "      not reachable\n");
#endif
    }
    else {
        for (++nel, g_p = g_p->link ; g_p != *t_p ;
                ++nel, g_p = g_p->link);
#ifdef DEBUG
        fprintf(stderr, "      reachable from %d\n", nel);
#endif
    }
    store_compact(nel, matfp);
    store_compact(tmp, matfp);
    if (! nel) {
#ifdef DEBUGCALLS
        fprintf(stderr, "    End of store_transpose\n");
#endif
        return;
    }
    for (g_p = g_p->link ; nel-- ; g_p = g_p->link) {
        store_compact(g_p->fm, matfp);
        tmp = g_p->rt;  store_compact(tmp, matfp);
#ifdef DEBUG
        fprintf(stderr, "          from %d rate %d\n", g_p->fm, tmp);
#endif
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of store_transpose\n");
#endif
}



void init() {
    int nm, Naecs, ii, ecs, nt, jj, pri;
    float rw;
    struct Trans_weight *trans_p;
    unsigned *ttt;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif

    fscanf(auxfp, "toptan= %d\n", &toptan);
    fscanf(auxfp, "topvan= %*d\n");
    fscanf(auxfp, "maxmark= %*d\n");
    fscanf(auxfp, "aecs_conf[%d]=", &group_num);
    aecs = (struct ECS_descr *)ecalloc(group_num + 1, sizeof(struct ECS_descr));
    aecs->num = 0;
    for (aecs_p = aecs + 1, Naecs = 0, nm = group_num ; nm-- ; aecs_p++) {
        fscanf(auxfp, " %d", &nt);
        aecs_p->num = nt;
        aecs_p->set = (unsigned *)ecalloc(nt + 1, sizeof(unsigned));
#ifdef DEBUG
        fprintf(stderr, "      ECS #%d  contains %d AECS\n", group_num - nm, nt);
#endif
        Naecs += nt;
    }

    /* read transitions */
    load_trs = FALSE;
    fscanf(nfp, "%d\n", &trans_num);
    t_list = (struct Trans_weight *)ecalloc(trans_num + 1,
                                            sizeof(struct Trans_weight));
    for (trans_p = t_list + 1, nt = 0 ; nt++ < trans_num ; trans_p++) {
        fscanf(nfp, "%d %d %d %f %d", &ii, &ecs, &pri, &rw, &jj);
#ifdef DEBUG
        fprintf(stderr, "    reading transition %d\n", nt);
#endif
        if (jj) {
            struct Clause_def *clp;
            struct Cond_def *cop;
            struct Val_def *vap;
            char cc;
            int nco, nva, nn;
            double dd;

#ifdef DEBUG
            fprintf(stderr, "        m-d with %d clauses\n", jj);
#endif
            if (jj < 0) {
                fprintf(stderr, "ERROR: no general distribution allowed!\n");
                exit(1);
            }
            while ((cc = getc(nfp)) != '\n');
            load_trs = TRUE;
            trans_p->md_p.mdc = clp = (struct Clause_def *)ecalloc(jj,
                                      sizeof(struct Clause_def));
            while (jj--) {
                while ((cc = getc(nfp)) == ' ');
                fscanf(nfp, "%d %d\n", &nco, &nva);
#ifdef DEBUG
                fprintf(stderr, "          type=%c nco=%d nva=%d\n", cc, nco, nva);
#endif
                clp->type = cc;
                clp->nocond = nco;
                clp->noval = nva;
                clp->conds = cop = (struct Cond_def *)ecalloc(nco,
                                   sizeof(struct Cond_def));
                clp->vals = vap = (struct Val_def *)ecalloc(nva,
                                  sizeof(struct Val_def));
                while (nco--) {
                    while ((cc = getc(nfp)) == ' ');
                    if (cc == 'c') {
                        cop->op = cc;
                        while ((cc = getc(nfp)) == ' ');
                        cop->type = cc;
                        fscanf(nfp, "%d", &nn);
                        cop->p1 = nn;
                        while ((cc = getc(nfp)) == ' ');
                        cop->top2 = cc;
                        fscanf(nfp, "%d\n", &nn);
                        cop->op2 = nn;
#ifdef DEBUG
                        fprintf(stderr, "              c type=%c p1=%d top2=%c op2=%d\n", cop->type,
                                cop->p1, cop->top2, cop->op2);
#endif
                    }
                    else {
#ifdef DEBUG
                        fprintf(stderr, "              %c\n", cc);
#endif
                        cop->op = cc;
                        while (getc(nfp) != '\n');
                    }
                    ++cop;
                }
                while (nva--) {
                    while ((cc = getc(nfp)) == ' ');
                    if (cc == 'o') {
                        vap->op = cc;
                        while ((cc = getc(nfp)) == ' ');
                        if ((vap->type = cc) == 'p') {
                            fscanf(nfp, "%d\n", &nn);
                            vap->val.place = nn;
#ifdef DEBUG
                            fprintf(stderr, "              o p %d\n", nn);
#endif
                        }
                        else {
                            fscanf(nfp, "%lf\n", &dd);
                            vap->val.real = dd;
#ifdef DEBUG
                            fprintf(stderr, "              o r %lf\n", dd);
#endif
                        }
                    }
                    else {
                        vap->op = cc;
#ifdef DEBUG
                        fprintf(stderr, "              %c\n", cc);
#endif
                        while (getc(nfp) != '\n');
                    }
                    ++vap;
                }
                ++clp;
            }
        }
        fscanf(nfp, " %d", &jj);
#ifdef DEBUG
        fprintf(stderr, "        ecs=%d endep=%d rate=%f\n", ecs, jj, rw);
#endif
        trans_p->noecs = ecs;
        trans_p->endep = jj;
        trans_p->rate = rw;
        fprintf(stderr, "gmt_stndrd: transition %d has rate %.15lf\n", nt, trans_p->rate);
        if (ecs) {
            trans_p->md_p.prob = (double *)ecalloc(aecs[ecs].num + 1,
                                                   sizeof(double));
        }
        else if (jj) {
            double *d_p;
            trans_p->md_p.prob = d_p = (double *)ecalloc(jj + 2,
                                       sizeof(double));
            *(++d_p) = rw;
            while (jj--) {
                fscanf(nfp, "%f", &rw);
#ifdef DEBUG
                fprintf(stderr, "            coef=%f\n", rw);
#endif
                *(++d_p) = rw;
            }
        }
        while (getc(nfp) != '\n');
    }

    ttt = (unsigned *)ecalloc(trans_num + 1, sizeof(unsigned));
    /* read aecs */
    while (Naecs--) {
        fscanf(aefp, "%d %d %d", &ecs, &nm, &ii);
#ifdef DEBUG
        fprintf(stderr, "      ECS=%d AECS=%d nt=%d\n", ecs, nm, ii);
#endif
        aecs[ecs].set[nm] = ii;
        if (ii == 1)
            fscanf(aefp, "%*d\n");
        else {
            ttt[ii] = 0; rw = 0.0;
            while (ii--) {
                fscanf(aefp, "%d", &nt);
                ttt[ii] = nt;
                rw += t_list[nt].rate;
            }
            while (getc(aefp) != '\n');
            rw = 1.0 / rw;
            for (ii = 0; (nt = ttt[ii++]) ;) {
                trans_p = t_list + nt;
                trans_p->md_p.prob[nm] = trans_p->rate * rw;
#ifdef DEBUG
                fprintf(stderr, "          trans=%d prob=%f\n", nt, trans_p->md_p.prob[nm]);
#endif
            }
        }
    }
    free(ttt);

    tail = (struct grph_itm_l **)ecalloc(toptan + 1, sizeof(struct grph_itm_l *));
    weight = (unsigned short *)ecalloc(toptan + 1, sizeof(unsigned short));
    M0 = (double *)ecalloc(toptan + 1, sizeof(double));
    for (ii = 0; ii < toptan + 1; ii++)
        M0[ii] = 0.0;
    // for (d_p = M0, ii = toptan ; ii-- ; * (++d_p) = 0.0);
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}

static int place_num;


struct Codbyt_descr {
    unsigned char shift;
    unsigned char mask;
    unsigned short num_byte;
    unsigned short link;
};

struct Place_descr {
    unsigned short lbound;
    unsigned short rubound;
    struct Codbyt_descr cod;
};

/*  PLACE ARRAY  */ static struct Place_descr *p_list;


struct Byte_descr {
    unsigned char noitems;
    unsigned char max_conf;
    unsigned short first_item;
};

static struct Byte_descr *encoding;


void decode_mark(unsigned char *Mp, unsigned short *dmp) {
    struct Place_descr *place_p = p_list;
    unsigned np, nb, val;

    for (np = place_num ; np-- ; place_p++) {
        if ((nb = place_p->cod.num_byte)) {
            nb--;
            val = Mp[nb] & place_p->cod.mask;
            val = val >> (place_p->cod.shift);
            val += place_p->lbound;
            *(++dmp) = val;
        }
        else
            *(++dmp) = 0;
    }
}


void init_markings() {
    int sub_num;
    int ii, nn, nt, jj, jj2, kk, ll, mm, np, n1, n2, n3, n4;
    struct Byte_descr *byte_p;
    struct Place_descr *place_p;
    struct Codbyt_descr *cods_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init_markings\n");
#endif

    /* read number of objects in the net */
    fscanf(nfp, "%d %d", &sub_num, &place_num);
    while (getc(nfp) != '\n');

    DMp = (unsigned short *)ecalloc(place_num + 1, sizeof(unsigned short));
    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));

    /* read codings */
    for (nn = 0 ; nn < sub_num ; nn++) {
        fscanf(nfp, "%d %d\n", &ii, &jj2);
#ifdef DEBUG
        fprintf(stderr, "  subnet #%d (%d bytes):\n",
                nn, jj2);
#endif
        if (nn == 0) {
            encoding = (struct Byte_descr *)ecalloc(jj2, sizeof(struct Byte_descr));
            CMp = (unsigned char *)ecalloc(jj2, sizeof(unsigned char));
            no_cod = jj2;
        }
        for (byte_p = encoding, ii = 0 ;
                ++ii <= jj2 ; byte_p++) {
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &kk, &ll, &np, &mm);
            if (nn == 0) {
                byte_p->max_conf = ll;
                byte_p->noitems = np;
            }
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &n1, &n2, &n3, &n4);
            if (nn == 0)
                byte_p->first_item = jj;
            for (kk = 1 ; ++kk <= np ;) {
                fscanf(nfp, "%d", &ll);
                if (nn == 0) {
                    place_p = p_list + (jj - 1);
                    cods_p = &(place_p->cod);
                    cods_p->link = ll;
                    jj = ll;
                    cods_p->num_byte = ii;
                    cods_p->mask = n1;
                    cods_p->shift = n2;
                }
                fscanf(nfp, "%d %d %d %d\n", &n1, &n2, &n3, &n4);
            }
            if (nn == 0) {
                place_p = p_list + (jj - 1);
                cods_p = &(place_p->cod);
                cods_p->link = 0;
                cods_p->num_byte = ii;
                cods_p->mask = n1;
                cods_p->shift = n2;
            }
#ifdef DEBUG
            fprintf(stderr, "    coding #%d (%d configurations)\n",
                    ii, jj2);
#endif
        }
    }

    /* read places */
#ifdef DEBUG
    fprintf(stderr, "   places:\n");
#endif
    for (ii = 0, place_p = p_list; ++ii <= place_num; place_p++) {
        fscanf(nfp, "%d %d %d %d %d %d %d",
               &np, &jj, &kk, &ll, &nt, &nn, &mm);
        place_p->lbound = nt;
        place_p->rubound = nn - nt;
        while (getc(nfp) != '\n');
#ifdef DEBUG
        fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                ii, place_p->cod.mask, place_p->cod.shift, place_p->cod.num_byte);
        fprintf(stderr, "            lbound=%d rubound=%d notok=%d link=%d\n",
                place_p->lbound, place_p->rubound, mm, place_p->cod.link);
#endif
    }

#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init_markings\n");
#endif
}



double
compute_immpath() {
    unsigned long ul;
    int plength, nt, ecs, aecsn;
    double ppp;
    struct Trans_weight *trans_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of compute_immpath\n");
#endif

    ppp = 1.0;
    load_compact(&ul, rgfp); plength = ul;
    while (plength--) {
        load_compact(&ul, rgfp); nt = ul;
        load_compact(&ul, rgfp); aecsn = ul;
#ifdef DEBUG
        fprintf(stderr, "          trans %d aecs %d\n", nt, aecsn);
#endif
        if (aecsn) {
            aecs_p = aecs + (ecs = (trans_p = t_list + nt)->noecs);
#ifdef DEBUG
            fprintf(stderr, "              in ecs %d\n", ecs);
#endif
            if (((aecs_p->set)[aecsn]) > 1) {
                if (ppp == 1.0)
                    ppp = trans_p->md_p.prob[aecsn];
                else
                    ppp *= trans_p->md_p.prob[aecsn];
            }
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of compute_immpath %f\n", ppp);
#endif
    return (ppp);
}


void load_initialmark() {
    unsigned long ul;
    int nm;
    double pp;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of load_initialmark\n");
#endif
    load_compact(&ul, rgfp);
    if ((nm = ul)) {
        M0[nm] = 1.0;
#ifdef DEBUG
        fprintf(stderr, "      M1 with prob. 1.0\n");
#endif
    }
    else {
        for (load_compact(&ul, rgfp), nm = ul ; nm ;
                load_compact(&ul, rgfp), nm = ul) {
            pp = compute_immpath();
#ifdef DEBUG
            fprintf(stderr, "      M%d with prob. %f\n", nm, pp);
#endif
            M0[nm] = pp;
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of load_initialmark\n");
#endif
}


static struct Cond_def *cond_stack;


int
evaluate_cond(unsigned short *Mp) {
    int val1, val2;
    char op = (++cond_stack)->op;

    if (cond_stack->op == 'c') {
        val1 = Mp[cond_stack->p1];
        val2 = cond_stack->op2;
        if (cond_stack->top2 == 'p')
            val2 = Mp[val2];
        switch (cond_stack->type) {
        case '=' : return (val1 == val2);
        case 'n' : return (val1 != val2);
        case 'g' : return (val1 >= val2);
        case '>' : return (val1 > val2);
        case 'l' : return (val1 <= val2);
        case '<' : return (val1 < val2);
        }
    }

    val1 = evaluate_cond(Mp);
    if (op == '~')
        return (! val1);
    val2 = evaluate_cond(Mp);
    if (op == '&')
        return (val1 && val2);
    return (val1 || val2);
}


static struct Val_def *val_stack;


double
evaluate_val(unsigned short *Mp) {
    struct Val_def *vap = val_stack++;
    double val1, val2;

    if (vap->op == 'o') {
        if (vap->type == 'p')
            return ((double)(Mp[vap->val.place]));
        return (vap->val.real);
    }
    val2 = evaluate_val(Mp);
    val1 = evaluate_val(Mp);
    switch (vap->op) {
    case '+' : return (val1 + val2);
    case '-' : return (val1 - val2);
    case '*' : return (val1 * val2);
    case '/' : return (val1 / val2);
    default : return 0;
    }
}


double
md_rate(struct Clause_def *mdc, unsigned short *Mp) {
    int cond = (mdc->type == 'e');
    int val1, val2;

    while (! cond) {
        struct Cond_def *cop = mdc->conds;
        if (cop->op == 'c') {
            val1 = Mp[cop->p1];
            val2 = cop->op2;
            if (cop->top2 == 'p')
                val2 = Mp[val2];
            switch (cop->type) {
            case '=' : cond = (val1 == val2);
                break;
            case 'n' : cond = (val1 != val2);
                break;
            case 'g' : cond = (val1 >= val2);
                break;
            case '>' : cond = (val1 > val2);
                break;
            case 'l' : cond = (val1 <= val2);
                break;
            case '<' : cond = (val1 < val2);
                break;
            }
        }
        else {
            cond_stack = cop;
            val1 = evaluate_cond(Mp);
            if (cop->op == '~')
                cond = ! val1;
            else if (cop->op == '|' && val1)
                cond = TRUE;
            else if (cop->op == '&' && ! val1)
                cond = FALSE;
            else {
                cond = evaluate_cond(Mp);
            }
        }
        if (! cond) {
            cond = ((++mdc)->type == 'e');
        }
    }
    val_stack = mdc->vals;
    return (evaluate_val(Mp));

}


void load_trg() {
    unsigned long ul;
    int no_trans, nt, nm;
    double pp, rr, sum;
    int next_print;
    struct Clause_def *mdc;
    int decoded = 0;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of load_trg\n");
#endif
    for (load_compact(&ul, rgfp), lastan = ul, next_print = 0 ; lastan ;
            load_compact(&ul, rgfp), lastan = ul) {
        load_compact(&ul, rgfp);
        no_trans = ul;
        sum = 0.0;
#ifdef DEBUG2
        fprintf(stderr, "  [init] sum=%.15lf  ", sum);
#endif
#ifdef DEBUG
        fprintf(stderr, "\nfrom  #%d  (%d timed trans)\n", lastan, no_trans);
#endif
        nel = 0; glp = row_l;
        if (load_trs) {
            int ii = no_cod;
            unsigned char *cp = CMp;

            decoded = FALSE;
            while (ii--)
                *(cp++) = getc(trsfp);
        }
        while (no_trans--) {
            struct Trans_weight *t_p;

            load_compact(&ul, rgfp); nt = ul;
            load_compact(&ul, rgfp); nm = ul;
#ifdef DEBUG
            fprintf(stderr, "     timed #%d  (enabling=%d)\n", nt, nm);
#endif
            t_p = &(t_list[nt]);
            if (nm > 1) {
                if (t_p->endep) {
                    rr = t_p->md_p.prob[nm];
#ifdef DEBUG2
                    fprintf(stderr, "rate[1]: nt=%d  rate=%.15lf  sum=%.15lf\n", nt, rr, sum);
#endif
#ifdef DEBUG
                    fprintf(stderr, "      timed #%d  is enabling-dependent %d, rate=%f\n",
                            nt, nm, rr);
#endif
                }
                else {
                    rr = t_p->rate;
#ifdef DEBUG
                    fprintf(stderr, "          t_p=%d t_p->rate=%f rr=%f\n",
                            (int)t_p, t_p->rate, rr);
#endif
                    rr *= (double)nm;
#ifdef DEBUG2
                    fprintf(stderr, "rate[2]: nt=%d  rate=%.15lf  nm=%d  sum=%.15lf\n", nt, rr, nm, sum);
#endif
#ifdef DEBUG
                    fprintf(stderr, "      timed #%d  is multiple-server %d, rate=%f\n",
                            nt, nm, rr);
#endif
                }
            }
            else if (!(t_p->endep) && (mdc = t_p->md_p.mdc) != NULL) {
                if (! decoded) {
                    decoded = TRUE;
                    decode_mark(CMp, DMp);
                }
                rr = md_rate(mdc, DMp);
#ifdef DEBUG2
                fprintf(stderr, "rate[3]: nt=%d  rate=%.15lf  sum=%.15lf\n", nt, rr, sum);
#endif
#ifdef DEBUG
                fprintf(stderr, "      timed #%d  is marking-dependent %d %f\n", nt, nm, rr);
#endif
            }
            else {
                rr = t_list[nt].rate;
#ifdef DEBUG2
                fprintf(stderr, "rate[4]: nt=%d  rate=%.15lf  sum=%.15lf\n", nt, rr, sum);
#endif
#ifdef DEBUG
                fprintf(stderr, "      timed #%d  is single-server %d, rate=%f\n",
                        nt, nm, rr);
#endif
            }
            load_compact(&ul, rgfp);
            if ((nm = ul)) {
#ifdef DEBUG
                fprintf(stderr, "    (T) to  #%d with rate %f\n", nm, rr);
#endif
                if (nm != lastan) {
                    sum += rr;
#ifdef DEBUG2
                    fprintf(stderr, "  sum=%.15lf  after adding rr=%.15lf\n", sum, rr);
#endif
                    insert_row(nm, rr);    /* INCREMENTS "nel" and "glp" */
                }
            }
            else {
                for (load_compact(&ul, rgfp), nm = ul ; nm ;
                        load_compact(&ul, rgfp), nm = ul) {
                    if ((pp = compute_immpath()) == 1.0)
                        pp = rr;
                    else
                        pp *= rr;
#ifdef DEBUG
                    fprintf(stderr, "        (V) to  #%d with rate %f\n", nm, pp);
#endif
                    if (nm != lastan) {
                        sum += pp;
#ifdef DEBUG2
                        fprintf(stderr, "  sum=%.15lf  after adding pp=%.15lf\n", sum, pp);
#endif
                        insert_row(nm, pp);    /* INCREMENTS "nel" and "glp" */
                    }
                }
            }
#ifdef DEBUG
            fprintf(stderr, "             nel=%d\n", nel);
#endif
        }
        if (nel) {
#ifdef DEBUG2
            fprintf(stderr, "  sum=%.15lf  ", sum);
#endif
            sum = 1.0 / sum;
#ifdef DEBUG2
            fprintf(stderr, "  1.0/sum=%.15lf\n", sum);
#endif
            frm = test_ins_d(sum);
            weight[lastan] = frm;
            for (glp = row_l, no_trans = nel ; no_trans-- ; ++glp) {
                glp->rt *= sum;
                frm = test_ins_d(glp->rt);
                insert_graph(lastan, glp->tm, frm);
            }
        }
        else {
            frm = test_ins_d(0.0);
            weight[lastan] = frm;
        }
#ifdef PRINT_STATISTICS
        if (nel > max_nel)
            max_nel = nel;
#endif
        if (lastan >= next_print) {
            next_print += 100;
            //fprintf(stderr," %6d\n", lastan);
        }
    }
    fprintf(stderr, " %6d rows computed\n\n", toptan);
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of load_trg\n");
#endif
}


static const char *can_t_open = "Can't open file %s for %c\n";
static char  filename[100];


#include <sys/types.h>
#include <sys/stat.h>


#ifdef PRINT_STATISTICS
#ifdef SOLARIS24
#	include <sys/times.h>
#	include <limits.h>
#else
#	include <sys/time.h>
#	include <sys/resource.h>
#ifndef RUSAGE_SELF
#	define RUSAGE_SELF 0
#endif
#endif
#endif


int main(int argc, char **argv) {
#ifdef PRINT_STATISTICS
#ifdef SOLARIS24
    struct tms buffer;
#else
    struct rusage ru;
#endif
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
            if (*s_p != '-' || *(s_p + 1) != 'e') {
                fprintf(stderr, "ERROR: unknown parameter '%s'\n", s_p);
                exit(33);
            }
            sscanf((s_p + 2), "%lf", &eps);
        }
    }
    g_coef = 1.0 + eps;
    l_coef = 1.0 - eps;
    sprintf(filename, "%s.gmt", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((auxfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.aecs", argv[1]);
    if ((aefp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init();
    (void)fclose(nfp);
    (void)fclose(auxfp);
    (void)fclose(aefp);
    if (load_trs) {
        sprintf(filename, "%s.grg", argv[1]);
        if ((nfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
        init_markings();
        (void)fclose(nfp);
        sprintf(filename, "%s.ctrs", argv[1]);
        if ((trsfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
    }
    sprintf(filename, "%s.crgr", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
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
        }
    }
    free(M0);
    (void) fclose(marfp);

    load_trg();
    (void) fclose(rgfp);
    if (load_trs)
        (void) fclose(trsfp);

    sprintf(filename, "%s.emc", argv[1]);
    if ((matfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    store_compact(toptan, matfp);
    {
        unsigned ii;
        for (ii = toptan, w_p = &(weight[1]), t_p = &(tail[1]) ;
                ii-- ; ++w_p, ++t_p) {
            store_transpose();
        }
    }
    nel = 0;  store_compact(nel, matfp);
    store_compact(nel, matfp);
    (void) fclose(matfp);

    sprintf(filename, "%s.doubles", argv[1]);
    if ((doufp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    frm = num_doubles();
    store_compact(frm, doufp);
    {
        unsigned ii;
        double dbl;
        for (ii = 0 ; ii++ < frm ;) {
            dbl = ith_double(ii);
            store_double(&dbl, doufp);
        }
    }
    (void) fclose(doufp);

//end_main:
#ifdef PRINT_STATISTICS
#ifdef SOLARIS24
    (void) times(&buffer);
#else
    getrusage(RUSAGE_SELF, &ru);
#endif
    fprintf(stderr, "\nnumber of doubles : %ld\n", frm);
    fprintf(stderr, "maximum row length : %d\n", max_nel);
    fprintf(stderr, "maximum tree depth : %d\n", max_depth);
    mean_depth = tot_depth;
    mean_depth /= num_call;
    fprintf(stderr, "average tree depth : %f\n", mean_depth);
#ifdef SOLARIS24
    mean_depth = 1.0 / CLK_TCK;
    fprintf(stderr, "\nCPU time (s.) : user %g, system %g\n\n",
            mean_depth * buffer.tms_utime, mean_depth * buffer.tms_stime);
#else
    fprintf(stderr, "\nCPU time (s.) : user %zu, system %zu\n",
            ru.ru_utime.tv_sec, ru.ru_stime.tv_sec);
    fprintf(stderr, "Max memory size : %ld Kbyte\n",
            ru.ru_maxrss * 2);
    fprintf(stderr, "Page faults : reclaims %ld, faults %ld\n",
            ru.ru_minflt, ru.ru_majflt);
    fprintf(stderr, "Context Switches : voluntary %ld, forced %ld\n",
            ru.ru_nvcsw, ru.ru_nivcsw);
    fprintf(stderr, "Swap : %ld\n\n",
            ru.ru_nswap);
#endif
#endif
#ifdef DEBUG
    fprintf(stderr, "End of gmt_stndrd\n");
#endif
    return 0;
}

