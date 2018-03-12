/*
 *  Torino, September 18, 1987
 *  program: gmtc.c
 *  purpose: Reduction of a GSPN and construction of the EMC.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) The correctness of the reduction rely on the ordering
 *       of vanishing markings that result from depth-first search
 *       as implemented in program "grg.c".
 */


/*
#define DEBUG
*/

#define PRINT_STATISTICS
/*
*/

#define EPS 0.00000001 /* The equivalence threshold for "double" */

extern unsigned load_trs, load_vrs;

#include <stdio.h>

FILE *rgfp, * tmfp, * vmfp;

#include "const.h"

#include "grg.h"

MARKP rootm;

double eps, g_coef, l_coef;

/* CODING BYTES */ extern unsigned no_cod;

unsigned char *coded_m;

struct d_tree_itm {
    double val;
    unsigned short l_s;
    unsigned short g_s;
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

unsigned long top_tan;


rebalance(newn)
struct d_tree_itm *newn;
{
    char *p_d_p = &(pathdir[ancdepth]);
    register int ancomp = *(p_d_p++);
    register unsigned n_d_p = ((ancomp > 0) ? (unbances->g_s) :
                               (unbances->l_s));
    struct d_tree_itm *d_p = d_t_list + n_d_p;
    struct d_tree_itm *d_p_2 = d_p;
    register unsigned nn;
    register int aux;

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
        return;
    }
    if (aux != ancomp) {    /*  BALANCED TREE  */
        unbances->balance = 0;
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
}


#ifdef PRINT_STATISTICS
unsigned max_nel = 0;
unsigned max_depth = 0;
unsigned long tot_depth = 0;
unsigned long num_call = 0;
#endif


unsigned short test_ins_d(dval)
double dval;
{
    register unsigned nn = root_d_tree;
    struct d_tree_itm *d_p;
    register unsigned  udval_f, ldval_f;
    double udval, ldval;
    char *path = pathdir;

#ifdef DEBUG
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
#ifdef DEBUG
                fprintf(stderr, "      old # %d\n", nn);
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
                        fprintf(stderr, "Sorry, increase constant MAX_ITEMS\n");
                        exit(15);
                    }
                    d_p = d_t_list + nn;
                    d_p->val = dval; d_p->l_s = 0; d_p->g_s = 0;
                    d_p->balance = 0;
                    rebalance(d_p);
#ifdef DEBUG
                    fprintf(stderr, "      new # %d\n", nn);
#endif
#ifdef PRINT_STATISTICS
                    tot_depth += depth;
                    if (depth > max_depth)
                        max_depth = depth;
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
                            fprintf(stderr, "Sorry, increase constant MAX_ITEMS\n");
                            exit(15);
                        }
                        d_p = d_t_list + nn;
                        d_p->val = dval; d_p->l_s = 0; d_p->g_s = 0;
                        d_p->balance = 0;
                        rebalance(d_p);
#ifdef DEBUG
                        fprintf(stderr, "      new # %d\n", nn);
#endif
#ifdef PRINT_STATISTICS
                        tot_depth += depth;
                        if (depth > max_depth)
                            max_depth = depth;
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
#ifdef DEBUG
                    fprintf(stderr, "      old # %d\n", nn);
#endif
                    return (nn);
                }
            }
        }
    }
    d_p = d_t_list + 1;
    d_p->val = dval; d_p->l_s = 0; d_p->g_s = 0;
    d_p->balance = 0;
    return (root_d_tree = top_d_tree = 1);
}

double get_d_tree(nn)
unsigned short nn;
{
    register unsigned long ii = nn;
    return (d_t_list[ii].val);
}


struct grph_itm_r {
    unsigned long  tm;
    double rt;
};

struct grph_itm_l {
    unsigned long  tm;
    unsigned short rt;
};

struct mark_descr_itm {
    unsigned short	l_l;
    struct grph_itm_l 	*l_p
        };


//extern char * calloc();


struct mark_descr_itm *mark_p[MAX_ARR];
unsigned long vmrk_limit = MAX_MARKS;
unsigned long vmrk_top_p = MAX_ARR;

struct mark_descr_itm *new_vanmrk(nm)
unsigned long nm;
{
    struct mark_descr_itm *pp;
    register unsigned nrep = --nm / MRKSLOT;

    if (nm >= MAX_MARKS) {
        fprintf(stderr, "Sorry, increase constant MAX_MARKS\n");
        exit(16);
    }
    if (nm < vmrk_limit) {
        for (; vmrk_top_p > nrep ;) {
            struct mark_descr_itm *mdi_p;
            if ((mdi_p = (struct mark_descr_itm *)calloc(MRKSLOT,
                         sizeof(struct mark_descr_itm))) == 0) {
                fprintf(stderr, "calloc: couldn't get enough memory!\n");
                exit(50);
            }
            mark_p[--vmrk_top_p] = mdi_p;
            vmrk_limit -= MRKSLOT;
        }
    }
    pp = (nm % MRKSLOT) + mark_p[nrep];
    return (pp);
}


struct mark_descr_itm *vanmrk_pointer(nm)
unsigned long nm;
{
    struct mark_descr_itm *pp;
    register unsigned nrep = --nm / MRKSLOT;

    if (nm >= MAX_MARKS) {
        fprintf(stderr, "ERROR: trying to address marking %d\n",
                nm + 1);
        exit(20);
    }
    pp = (nm % MRKSLOT) + mark_p[nrep];
    return (pp);
}


extern load_compact(/* pp, fp */);
/*
  unsigned long * pp;
  FILE * fp;
*/

extern store_compact(/* nval, fp */);
/*
  unsigned long nval;
  FILE * fp;
*/



struct grph_itm_r row_l[MAX_ROW];
struct grph_itm_r *glp;

unsigned is_vanishing;
unsigned long frm, tmp;
unsigned nel;

unsigned long topvan = MAX_MARKS;

extern double rate_from_RG(/* rgfp */);
/*
  FILE * rgfp;
*/


insert_row(to, rate)
unsigned long to;
double rate;
{
    struct grph_itm_r *glp2 = row_l;
    register unsigned ii = glp - row_l;

    for (; ii-- ; ++glp2)
        if (glp2->tm == to) {
            glp2->rt += rate;
            return;
        }
    glp2->tm = to; glp2->rt = rate; ++nel; ++glp;
    if (nel >= MAX_ROW) {
        fprintf(stderr, "Sorry, increase constant MAX_ROW\n");
        exit(17);
    }
}


store_van_mark() {
    struct grph_itm_l *glp2;
    struct mark_descr_itm *mp = new_vanmrk(frm);

    mp->l_l = nel;
    if ((glp2 = (struct grph_itm_l *)calloc(nel,
                                            sizeof(struct grph_itm_l))) == 0) {
        fprintf(stderr, "calloc: couldn't get enough memory!\n");
        exit(50);
    }
    mp->l_p = glp2;
    for (glp = row_l ; nel-- ; ++glp, ++glp2) {
        glp2->tm = glp->tm;  glp2->rt = test_ins_d(glp->rt);
    }
}

reduce_van_path(is_one, rate)
unsigned is_one;
double rate;
{
    struct grph_itm_l *glp2;
    struct mark_descr_itm *mp = vanmrk_pointer(tmp);
    register unsigned i = mp->l_l;
    double rr;

    if (is_one)
        for (glp2 = mp->l_p ; i-- ; ++glp2) {
            insert_row(glp2->tm, get_d_tree(glp2->rt));
        }
    else
        for (glp2 = mp->l_p ; i-- ; ++glp2) {
            rr = get_d_tree(glp2->rt);
            insert_row(glp2->tm, ((rr == 1.0) ? (rate) : (rr * rate)));
        }
}


unsigned long lnel;

load_rgr() {
    double dftemp;
    double sum;
    register unsigned i;

#ifdef DEBUG
    fprintf(stderr, "  Start of load_rgr %d\n", frm);
#endif
    load_compact(&lnel, rgfp);
    nel = (unsigned)((lnel & ~ 0x1) >> 1);
    if ((is_vanishing = (lnel & 0x1))) {
        if (frm < topvan)
            topvan = frm;
        if (load_vrs) {
            unsigned char *uc_p = coded_m;

            for (i = no_cod ; i-- ; ++uc_p)
                *uc_p = getc(vmfp);
            decode_mark(coded_m);
        }
    }
    else {
        if (load_trs) {
            unsigned char *uc_p = coded_m;

            for (i = no_cod ; i-- ; ++uc_p)
                *uc_p = getc(tmfp);
            decode_mark(coded_m);
        }
    }
    i = lnel = nel; --lnel;
    nel = 0; glp = row_l;
    dftemp = rate_from_RG(rgfp);
    load_compact((&tmp), rgfp);
    if ((tmp >= topvan)) {
        if (lnel)
            reduce_van_path(FALSE, dftemp);    /* INCREMENTS "nel" and "glp" */
        else {
            if (! is_vanishing)
                sum = 1.0 / dftemp;
            reduce_van_path(TRUE);    /* INCREMENTS "nel" and "glp" */
        }
    }
    else {
        if (lnel)
            insert_row(tmp, dftemp);    /* INCREMENTS "nel" and "glp" */
        else {
            if (! is_vanishing)
                sum = 1.0 / dftemp;
            insert_row(tmp, (double)1.0);    /* INCREMENTS "nel" and "glp" */
        }
    }
    if (lnel) {
        sum = dftemp;
        while (--i) {
            dftemp = rate_from_RG(rgfp);
            sum += dftemp;
            load_compact((&tmp), rgfp);
            if ((tmp >= topvan))
                reduce_van_path(FALSE, dftemp);    /* INCREMENTS "nel" and "glp" */
            else {
                insert_row(tmp, dftemp);    /* INCREMENTS "nel" and "glp" */
            }
        }
        sum = 1.0 / sum;
        for (glp = row_l, i = nel ; i-- ; ++glp)
            glp->rt *= sum;
    }
    if (is_vanishing)
        store_van_mark();
    else {
#ifdef DEBUG
        fprintf(stderr, "      tang %d reduced\n", frm);
#endif
#ifdef PRINT_STATISTICS
        if (nel > max_nel)
            max_nel = nel;
#endif
        store_compact(nel, stdout);
        frm = test_ins_d(sum);
        store_compact(frm, stdout);
        for (glp = row_l, i = nel ; i-- ; ++glp) {
            store_compact(glp->tm, stdout);
            frm = test_ins_d(glp->rt);
            store_compact(frm, stdout);
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End of load_rgr\n");
#endif
}


#include <sys/types.h>
#include <sys/stat.h>

static char   *can_t_open = "Can't open file %s for %c\n";
char  netname[100];
char  filename[100];
extern FILE *fopen();

init() {
    struct stat stb, stb2;

    sprintf(filename, "%s.net", netname);
    if (stat(filename, &stb) < 0) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(33);
    }
    sprintf(filename, "%s.ctrs", netname);
    if ((stat(filename, &stb2) < 0)
            /* || (stb2.st_mtime <= stb.st_mtime) */) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(35);
    }

    top_tan = stb2.st_size / no_cod;

    sprintf(filename, "%s.crgr", netname);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    if (load_trs) {
        sprintf(filename, "%s.ctrs", netname);
        if ((tmfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
    }
    if (load_vrs) {
        sprintf(filename, "%s.cvrs", netname);
        if ((vmfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
    }
    coded_m = (unsigned char *)calloc(no_cod, sizeof(char));
}


#ifdef PRINT_STATISTICS
#include <sys/time.h>
#include <sys/resource.h>
#define RUSAGE_SELF 0
#endif

main(argc, argv)
int argc;
char **argv;
{
#ifdef PRINT_STATISTICS
    struct rusage ru;
    float mean_depth;
#endif

#ifdef DEBUG
    fprintf(stderr, "Start\n");
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
            sscanf((s_p + 2), "%f", &eps);
        }
    }
    g_coef = 1.0 + eps;
    l_coef = 1.0 - eps;
    sprintf(netname, "%s", argv[1]);
    init();
    store_compact(top_tan, stdout);
    for (load_compact(&frm, rgfp) ; frm > 0 ;
            load_compact(&frm, rgfp)) {
        load_rgr();
    }
    lnel = 0;  store_compact(lnel, stdout);
    (void) fclose(rgfp);
    if (load_trs)
        (void) fclose(tmfp);
    if (load_vrs)
        (void) fclose(vmfp);
    sprintf(filename, "%s.doubles", netname);
    if ((rgfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    frm = top_d_tree;
    store_compact(frm, rgfp);
    {
        unsigned i;
        for (i = 0 ; i++ < top_d_tree ;)
            store_double(&(d_t_list[i].val), rgfp);
    }
    (void) fclose(rgfp);
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
#endif
#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
}
