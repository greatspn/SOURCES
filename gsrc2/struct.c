/*
 *  program: struct.c
 *  date: November 15, 1989
 *  programmer: Giovanni Chiola
 *  address:	Dipartimento di Informatica, Universita' di Torino
 *		corso Svizzera 185, 10149 Torino, Italy.
 *
 *  purpose: structural analysis of a GSPN; produces the following files:
 *           "netname.me"  ->  mutual exclusion relation between
 *			       transition pairs;
 *           "netname.cc"  ->  causal connection set of each transition;
 *           "netname.sc"  ->  structural conflict set of each transition;
 *           "netname.sub" ->  Partition of the net in disjoint subnets
 *			       of immediate transitions.
 *           "netname.ecs" ->  Extended Conflict Sets of the net
 *                           + possible confused sequences.
 *           "netname.bnd" ->  token bound of each place;
 *  notes:
 *    1) the net description must be in GreatSPN 1.4 format
 *    2) Net description has been extended for the new JavaGUI features, i.e.
 *       marking parameters on arcs and general transition functions.
 */


/*
#define DEBUG
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *nfp, * ifp, * ofp;

#include "const.h"


#define MAXTOKENS 2147483647

typedef unsigned int ROWT;
#define ROWT_SIZE (unsigned)(sizeof(ROWT))

typedef ROWT *ROWP;
#define ROWP_SIZE (unsigned)(sizeof(ROWP))

typedef ROWP *ARCM;
#define ARCM_SIZE (unsigned)(sizeof(ARCM))

ARCM t_inp, t_out, t_inh;
ARCM p_inp, p_out, p_inh;
ARCM t_in_count;
ARCM me_rel, sc_rel;
ROWP t_ecs, t_pri, t_sub, p_sub;
ROWP inter;

/* predeclaration */
void getname(char *name_pr);
void getarcs(ARCM t_arc, ARCM p_arc, int noar, int notr, ROWP mpars, int nompars);


struct place_obj {
    ROWT mark;
    ROWT enc;
    ROWT mult;
    ROWT covered;
    ROWT ubound;
    ROWT lbound;
    struct place_obj *next;
};

struct T_list {
    ROWT trans;
    struct T_list *next;
};

typedef struct T_list *T_L_P;

struct I_header {
    ROWT nouncov;
    ROWT cod_type;
    ROWT tcount;
    ROWP rowp;
    struct place_obj *p_l;
};

struct C_mark {
    ROWT max_c;
    ROWT cod_type;
    struct place_obj *p_l;
};

#define POB_SIZE (unsigned)(sizeof(struct place_obj))
#define T_l_SIZE (unsigned)(sizeof(struct T_list))
#define T_P_SIZE (unsigned)(sizeof(T_L_P))
#define I_H_SIZE (unsigned)(sizeof(struct I_header))
#define C_M_SIZE (unsigned)(sizeof(struct C_mark))

struct place_obj *p_list;

struct I_header *M_invars, * unused;
int no_M_invars, no_alloc_inv, no_unused;
int no_alloc_pl;

struct C_mark *encoding;
int no_cod = 0;

int	place_num, trans_num;

T_L_P *ecs_listp, * sub_plist, *sub_tlist, *sub_pelems;

char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
#ifndef Linux
    //extern char *	calloc();
#endif
    char 		*callptr;

#ifdef Linux
    if (nitm == 0)
        nitm = 1;
#endif

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}

// support to modifiable marking parameters
#define MAX_MPARS_TO_CHANGE   32
struct {
    const char *mpar_name;
    int new_val;
} mpars_to_change[MAX_MPARS_TO_CHANGE];
int num_mpars_to_change = 0;


void load_net() {


    ARCM		pp1, pp2, pp3;
    ROWP		rwp, t_pp;
    T_L_P 		*ecs_lp, ecs_p;
    struct place_obj 	*place;
    char		linebuf[LINEMAX], fieldbuf[LINEMAX], *bufptr;
    ROWP	mparp;
    float		ftemp, x, y;
    int         group_num, i, knd, noar, mark, nomp, norp, nolay;
    size_t      bufsz;
#define VBAR '|'

#ifdef DEBUG
    printf("  Start of load_net\n");
#endif

    /* skip first line containing '|0|' */
    fgets(linebuf, LINEMAX, nfp);

    /* skip comment */
    for (; ;) {
        fgets(linebuf, LINEMAX, nfp);
        if (linebuf[0] == VBAR) break;
    }

    /* read number of objects in the net */

    fscanf(nfp, "f %d %d %d", &nomp, &place_num, &norp);
    fscanf(nfp, "%d %d %d %d\n", &trans_num, &group_num, &i, &nolay);

    p_list = (struct place_obj *)ecalloc(place_num, POB_SIZE);
    mparp = (ROWP)ecalloc(nomp, short_SIZE);
    pp1 = me_rel = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    for (i = trans_num ; i-- ; pp1++) {
        register ROWP cp;
        int j;
        *pp1 = cp = (ROWP)ecalloc(trans_num, ROWT_SIZE);
        for (j = trans_num ; j-- ; cp++)
            if (i == j)
                *cp = TRUE;
            else
                *cp = FALSE;
    }
    pp1 = sc_rel = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    for (i = trans_num ; i-- ; pp1++) {
        *pp1 = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
        **pp1 = 0;
    }
    t_pp = t_pri = (ROWP)ecalloc(trans_num, ROWT_SIZE);
    rwp = p_sub = (ROWP)ecalloc(place_num, ROWT_SIZE);
    ecs_lp = sub_plist = (T_L_P *)ecalloc(trans_num, T_P_SIZE);
    for (i = trans_num ; i-- ; ecs_lp++)
        *ecs_lp = NULL;
    ecs_lp = sub_pelems = (T_L_P *)ecalloc(place_num, T_P_SIZE);
    ecs_p = (T_L_P)ecalloc(place_num, T_l_SIZE);
    for (i = place_num ; i-- ; ecs_lp++, ecs_p++, rwp++) {
        *ecs_lp = ecs_p;
        ecs_p->trans = (place_num - i);
        ecs_p->next = NULL;
        *rwp = (0);
    }
    rwp = t_sub = (ROWP)ecalloc(trans_num, ROWT_SIZE);
    ecs_lp = sub_tlist = (T_L_P *)ecalloc(trans_num, T_P_SIZE);
    ecs_p = (T_L_P)ecalloc(trans_num, T_l_SIZE);
    for (i = trans_num ; i-- ; ecs_lp++, ecs_p++, rwp++) {
        *ecs_lp = ecs_p;
        ecs_p->trans = (trans_num - i);
        ecs_p->next = NULL;
        *rwp = (trans_num - i);
    }
    rwp = t_ecs = (ROWP)ecalloc(trans_num, ROWT_SIZE);
    ecs_lp = ecs_listp = (T_L_P *)ecalloc(trans_num, T_P_SIZE);
    ecs_p = (T_L_P)ecalloc(trans_num, T_l_SIZE);
    for (i = trans_num ; i-- ; ecs_lp++, ecs_p++, rwp++) {
        *ecs_lp = ecs_p;
        ecs_p->trans = (trans_num - i);
        ecs_p->next = NULL;
        *rwp = (trans_num - i);
    }
    pp1 = p_inp = (ARCM)ecalloc(place_num, ROWP_SIZE);
    pp2 = p_out = (ARCM)ecalloc(place_num, ROWP_SIZE);
    pp3 = p_inh = (ARCM)ecalloc(place_num, ROWP_SIZE);
    t_inp = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    t_out = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    t_inh = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    t_in_count = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    inter = (ROWP)ecalloc(trans_num + trans_num + 1, ROWT_SIZE);
    for (i = place_num ; i-- ; pp1++, pp2++, pp3++) {
        register ROWP cp;
        unsigned int l = trans_num + trans_num + 1;
        *pp1 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
        *pp2 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
        *pp3 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
    }

#ifdef DEBUG
    printf("    %d marking parameters\n", nomp);
#endif
    /* read marking parameters */
    {
        register int i = 0, k;
        unsigned int mark;
        while (i < nomp) {
            getname(linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
            printf("    %d %s=%d\n", i, linebuf, mark);
#endif
            for (k = 0; k < num_mpars_to_change; k++) {
                if (0 == strcmp(mpars_to_change[k].mpar_name, linebuf)) {
                    mark = mpars_to_change[k].new_val;
                    // printf("Changing mpar value to %d.\n", mark);
                    break;
                }
            }
            if (mark == -7134) {
                printf("Marking parameter %s needs a value. Specify it with -mpar %s <value>\n", linebuf, linebuf);
                exit(1);
            }
            mparp[i++] = (ROWT)mark;
        }
    }

    /* read places */
    i = 0; place = p_list;
    while (i++ < place_num) {
        getname(linebuf);
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
        place->covered = FALSE;
        place->ubound = MAXTOKENS;
        place->lbound = 0;
        place->next = NULL;
        if (mark >= 0)
            place->mark = mark;
        else
            place->mark = mparp[-1 - mark];
#ifdef DEBUG
        printf("    place %s %d\n", linebuf, place->mark);
#endif
        place++;
    }

    /* skip rate parameters */
    i = 1;
    while (i++ <= norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp); while (getc(nfp) != '\n');
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        fscanf(nfp, "%f %f", &x, &y); while (getc(nfp) != '\n');
    }

    /* read transitions */
    for (i = trans_num ; i-- ;) {
        getname(linebuf);
        fscanf(nfp, "%s %d %d %d ", fieldbuf, &mark, &knd, &noar);
        if (knd >= DETERM)
            *(t_pp++) = 0;
        else
            *(t_pp++) = knd;
        if ((! knd || knd >= DETERM) && (i < (trans_num - 1))) {
            T_L_P ppp = *(sub_tlist + (trans_num - i - 1));
            ppp->next = *sub_tlist; *sub_tlist = ppp;
            *(t_sub + (trans_num - i - 1)) = 1;
        }
        while (getc(nfp) != '\n');
        // 13/04/2015: net/def extension - deterministic transition function 
        // is written after the transition line
        if (0 == strcmp(fieldbuf, "*")) {
            bufptr = linebuf;
            bufsz = LINEMAX-1;
            getline(&bufptr, &bufsz, nfp);
            printf("Skip line: %s\n", linebuf);
        }
        if (mark < 0) {
            int ii;
            ii = mark = -mark;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        getarcs(t_inp, p_out, noar, trans_num - i, mparp, nomp);
        fscanf(nfp, "%d\n", &noar);
        getarcs(t_out, p_inp, noar, trans_num - i, mparp, nomp);
        fscanf(nfp, "%d\n", &noar);
        getarcs(t_inh, p_inh, noar, trans_num - i, mparp, nomp);
    }
#ifdef DEBUG
    printf("  End of load_net\n");
#endif
}

void getname(char 	*name_pr) {
#define	BLANK ' '
#define	EOLN  '\0'
    short		i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != (char)0 &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}

int t_cmp(ROWP p1, ROWP  p2) {
    register int m1 = *(++p1), m2 = *(++p2);

    if (m1 == m2)
        return (0);
    return ((m2 > m1) ? -1 : 1);
}

void getarcs(ARCM t_arc, ARCM p_arc, int noar, int notr, ROWP mpars, int nompars) {
    ROWP	tp, pp;
    int		i, j, pl, mlt, ip;
    float	x, y;

    *(t_arc + (notr - 1)) = tp = (ROWP)ecalloc(noar + noar + 1, ROWT_SIZE);
    for (i = noar ; i-- ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while (getc(nfp) != '\n');
        for (j = 1; j++ <= ip; fscanf(nfp, "%f %f\n", &x, &y));

        // 24/3/2015 Add support for marking parameters on transition arcs.
        if (mlt >= 20000 && mlt < 20000 + nompars) {
            printf("Using marking parameter value %d for arc multiplicity.\n", 
                mpars[ mlt - 20000 ]);
            mlt = mpars[ mlt - 20000 ];
        }

        *(tp++) = (ABS(mlt));
        *(tp++) = pl;
        pp = *(p_arc + (pl - 1));
        while (*pp > 0)
            pp += 2;
        *(pp++) = (ABS(mlt));
        *(pp++) = notr;
        *pp = 0;
    }
    *(tp) = 0;
    qsort(*(t_arc + (notr - 1)), noar, 2 * ROWT_SIZE, (int (*)(const void *, const void *))t_cmp);
}


ROWP diffbag;

void bag_diff(ROWP bag1, ROWP  bag2, ROWP  diff) {
#ifdef DEBUG
    printf("      Start of bag_diff\n");
#endif
    while (*bag1) {
        while (*bag2 && *(bag2 + 1) < * (bag1 + 1))
            bag2 += 2;
        if (*bag2) {
            if (*(bag1 + 1) == *(bag2 + 1)) {
                if (*bag1 <= *bag2) {
                    bag1 += 2; bag2 += 2;
                }
                else {
                    *(diff++) = (*(bag1++) - *bag2); bag2 += 2;
                    *(diff++) = *(bag1++);
                }
            }
            else {
                *(diff++) = *(bag1++); *(diff++) = *(bag1++);
            }
        }
        else
            break;
    }
    while (*bag1) {
        *(diff++) = *(bag1++); *(diff++) = *(bag1++);
    }
    *diff = 0;
#ifdef DEBUG
    printf("      End of bag_diff\n");
#endif
}

void compute_cc() {
    int np, nt;
    ARCM ti_p, to_p;
    ARCM po_p, ph_p;
    ROWP cc_list, lp;
#ifdef DEBUG
    printf("  Start of compute_cc\n");
#endif
    lp = cc_list = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
    diffbag = (ROWP)ecalloc(trans_num + trans_num + 1, ROWT_SIZE);
    for (np = trans_num + 1 ; np-- ; * (lp++) = 0);
    for (ti_p = t_inp, to_p = t_out, nt = trans_num ; nt-- ; ti_p++, to_p++) {
        ROWP point;
        ROWP pp = diffbag;
        int no_con = 0;
#ifdef DEBUG
        printf("    connections of transition %d\n", trans_num - nt);
#endif
        bag_diff(*to_p, *ti_p, diffbag);
        lp = cc_list;
        for (; *pp > 0 ; pp++) {
            ROWP tp;
            po_p = p_out + (unsigned)(*(++pp) - 1);
            for (tp = *po_p; (int) * (tp++) ;) {
#ifdef DEBUG
                printf("        %d\n", (int)*tp);
#endif
                point = cc_list + (unsigned)(*(tp++) - 1);
                if (!(int)(*point))
                    no_con++;
                *point = 1;
            }
        }
#ifdef DEBUG
        printf("      out -> inp\n");
#endif
        bag_diff(*ti_p, *to_p, diffbag);
        for (pp = diffbag, lp = cc_list; *pp > 0 ; pp++) {
            ROWP tp;
            ph_p = p_inh + (unsigned)(*(++pp) - 1);
            for (tp = *ph_p; (int) * (tp++) ;) {
#ifdef DEBUG
                printf("        %d\n", (int)*tp);
#endif
                point = cc_list + (unsigned)(*(tp++) - 1);
                if (!(int)(*point))
                    no_con++;
                *point = 1;
            }
        }
#ifdef DEBUG
        printf("      inp -> inh\n");
#endif
        fprintf(ofp, "%d", no_con);
        for (lp = cc_list, np = trans_num ; np-- ; * (lp++) = 0)
            if ((int)(*lp)) {
                fprintf(ofp, " %d", trans_num - np);
            }
        fprintf(ofp, "\n");
    }
    free((char *)cc_list);
#ifdef DEBUG
    printf("  End of compute_cc\n");
#endif
}

unsigned add_places(bagp, phead1, nosub)
ROWP bagp;
T_L_P *phead1;
unsigned nosub;
{
    T_L_P hhh = NULL;
    ROWP pps;
    unsigned np, noadd = 0;

#ifdef DEBUG
    printf("      Start of add_places, nosub=%d\n", nosub);
#endif
    while (*bagp) {
        np = *(++bagp); bagp++;
#ifdef DEBUG
        printf("        np=%d\n", np);
#endif
        pps = p_sub + (np - 1);
        if (!(*pps)) {
            T_L_P ppp = *(sub_pelems + (np - 1));
#ifdef DEBUG
            printf("          adding\n");
#endif
            ppp->next = hhh; hhh = ppp;
            *pps = nosub;
            noadd++;
        }
#ifdef DEBUG
        printf("         endif\n");
#endif
    }
#ifdef DEBUG
    printf("        add list\n");
#endif
    if (*phead1 == NULL)
        *phead1 = hhh;
    else {
        T_L_P ppp = *phead1;
        for (; ppp->next != NULL ; ppp = ppp->next);
        ppp->next = hhh;
    }
#ifdef DEBUG
    printf("      End of add_places\n");
#endif
    return (noadd);
}

void add_trans(ROWP bagp, T_L_P *thead1, unsigned nosub) {
    T_L_P hhh = NULL;
    ROWP tps;
    unsigned nt;

#ifdef DEBUG
    printf("      Start of add_trans, nosub=%d\n", nosub);
#endif
    while (*bagp) {
        nt = *(++bagp); bagp++;
#ifdef DEBUG
        printf("        testing t%d\n", nt);
#endif
        tps = t_sub + (nt - 1);
        if (*tps > nosub) {
            T_L_P ttt = *(sub_tlist + (nt - 1));
            T_L_P *phead1 = sub_plist + (nosub - 1);
            ROWP pbagp = *(t_inp + (nt - 1));
#ifdef DEBUG
            printf("          adding t%d\n", nt);
#endif
            ttt->next = hhh; hhh = ttt;
            *tps = nosub;
            (void)add_places(pbagp, phead1, nosub);
            pbagp = *(t_out + (nt - 1));
            (void)add_places(pbagp, phead1, nosub);
            pbagp = *(t_inh + (nt - 1));
            (void)add_places(pbagp, phead1, nosub);
        }
    }
    {
        T_L_P ppp = *thead1;
        for (; ppp->next != NULL ; ppp = ppp->next);
        ppp->next = hhh;
    }
#ifdef DEBUG
    printf("      End of add_trans\n");
#endif
}

unsigned NSUB;

void compute_sub() {
    unsigned nt = 1,  nsub = 1;
    ROWP tsp = t_sub, tpp = t_pri;;

#ifdef DEBUG
    printf("  Start of compute_sub\n");
#endif
    while ((!(*tpp) || *tpp >= DETERM) && (nt <= trans_num)) {
        tpp++; tsp++; nt++;
    }
    while (nt <= trans_num) {
        unsigned o1offs = nt - 1;
        T_L_P *phead1 = (sub_plist + (o1offs));
        ROWP bagp = *(t_inp + o1offs);
        unsigned added = 0, nosub = *tsp;
#ifdef DEBUG
        printf("     subnet of t%d\n", nt);
#endif
        nsub++;
        added += add_places(bagp, phead1, nosub);
        bagp = *(t_out + o1offs);
        added += add_places(bagp, phead1, nosub);
        bagp = *(t_inh + o1offs);
        added += add_places(bagp, phead1, nosub);
        if (added) {
            T_L_P *thead1 = (sub_tlist + (o1offs));
            T_L_P pppp = *phead1;
            for (; pppp != NULL ; pppp = pppp->next) {
                unsigned o2offs = pppp->trans - 1;
                ROWP bagt = *(p_inp + o2offs);
                add_trans(bagt, thead1, nosub);
                bagt = *(p_out + o2offs);
                add_trans(bagt, thead1, nosub);
                bagt = *(p_inh + o2offs);
                add_trans(bagt, thead1, nosub);
            }
        }
#ifdef DEBUG
        printf("   subnet exhausted\n");
#endif
        while (((*tsp) <= nosub) && (nt <= trans_num)) {
#ifdef DEBUG
            printf("     skipping t%d\n", nt);
#endif
            tpp++; tsp++; nt++;
        }
    }
    if (*t_pri && *t_pri < DETERM)
        nsub--;
    else for (tsp = p_sub, nt = place_num; nt-- ; tsp++)
            if (!(*tsp)) {
                T_L_P ppp = *(sub_pelems + (place_num - nt - 1));
                ppp->next = *sub_plist; *sub_plist = ppp;
                *tsp = 1;
            }
    fprintf(ofp, "%d\n", nsub);
    NSUB = nsub;
    for (nt = 1 ; nsub-- ;) {
        unsigned offs = nt - 1;
        unsigned nnn = *(t_sub + offs);
        T_L_P phead = *(sub_plist + (offs));
        T_L_P thead = *(sub_tlist + (offs));
        ROWP sub_p;
        unsigned nn = 0;
        for (; phead != NULL ; nn++, phead = phead->next);
        fprintf(ofp, "%d", nn);
        for (sub_p = p_sub, nn = place_num ; nn-- ; sub_p++)
            if (*sub_p == nnn)
                fprintf(ofp, " %d", place_num - nn);
        fprintf(ofp, "\n");
        for (nn = 0 ; thead != NULL ; nn++, thead = thead->next);
        fprintf(ofp, "%d", nn);
        for (sub_p = t_sub, nn = trans_num ; nn-- ; sub_p++)
            if (*sub_p == nnn)
                fprintf(ofp, " %d", trans_num - nn);
        fprintf(ofp, "\n");
        for (nt++; * (t_sub + (nt - 1)) <= nnn ; nt++);
    }
#ifdef DEBUG
    printf("  End of compute_sub\n");
#endif
}

void compute_place_sub() {
    unsigned nt = 1, nsub = 1;

#ifdef DEBUG
    printf("  Start of compute_place_sub\n");
#endif
    for (nt = 1, nsub = NSUB ; nsub-- ;) {
        unsigned offs = nt - 1;
        unsigned nnn = *(t_sub + offs);
        T_L_P phead;
        ROWP sub_p;
        unsigned nn = 0;
        T_L_P *phead1 = (sub_plist + (offs));
        unsigned added = 0;
#ifdef DEBUG
        printf("    subnet #%d\n", nt);
#endif
        *(sub_plist + (offs)) = NULL;
        for (sub_p = p_sub, nn = place_num ; nn-- ; * (sub_p++) = 0);
        for (sub_p = t_sub, nn = trans_num ; nn-- ; sub_p++)
            if (*sub_p == nnn) {
                int o1offs = trans_num - nn - 1;
                ROWP bagp = *(t_inp + o1offs);
#ifdef DEBUG
                printf("      adding inp/inh of trans %d\n", trans_num - nn);
#endif
                added += add_places(bagp, phead1, nt);
                bagp = *(t_inh + o1offs);
                added += add_places(bagp, phead1, nt);
            }
#ifdef DEBUG
        printf("    counting places for subnet #%d\n", nt);
#endif
        phead = *(sub_plist + (offs));
        for (nn = 0 ; phead != NULL ; nn++, phead = phead->next);
        fprintf(ofp, "%d", nn);
#ifdef DEBUG
        printf("      %d places\n", nn);
#endif
        for (sub_p = p_sub, nn = place_num ; nn-- ; sub_p++)
            if ((*sub_p) == nt)
                fprintf(ofp, " %d", place_num - nn);
        fprintf(ofp, "\n");

        for (nt++; * (t_sub + (nt - 1)) <= nnn ; nt++);
    }
#ifdef DEBUG
    printf("  End of compute_place_sub\n");
#endif
}


ROWT inv_prod(plp, invp)
ROWP plp, invp;
{
    int nie = *(invp++), sum = 0;
    for (; nie-- ; invp += 2) {
        while (*plp && *(plp + 1) < * (invp + 1))
            plp += 2;
        if (*plp) {
            if (*(plp + 1) == *(invp + 1))
                sum += ((*plp) * (*invp));
        }
        else
            break;
    }
    return ((ROWT)sum);
}

void load_Pinvars() {
    struct I_header *rp;
    ROWP cp;
    int i, j, ii;
    int num, np, mult;
    unsigned tc;
    ARCM tcp, tip;

#ifdef DEBUG
    printf("  Start of load_Pinvars\n");
#endif
    fscanf(ifp, "%d\n", &no_M_invars);
#ifdef DEBUG
    printf("    no_M_invars=%d\n", no_M_invars);
#endif
    M_invars = (struct I_header *)ecalloc(no_M_invars, I_H_SIZE);
    for (rp = M_invars, i = no_M_invars; i-- ; rp++) {
        int kk = 0;

        rp->p_l = NULL;
        fscanf(ifp, "%d", &num);
        rp->nouncov = num;
#ifdef DEBUG
        printf("      reading %d-th one\n", (no_M_invars - i));
#endif
        rp->rowp = cp = (ROWP)ecalloc(2 * num + 1, ROWT_SIZE);
        for (*(cp++) = (j = num), tc = 0 ; j-- ;) {
            fscanf(ifp, "%d %d", &mult, &np);
            *(cp++) = mult;
            *(cp++) = np;
            tc += mult * (unsigned)(p_list[np - 1].mark);
        }
        fscanf(ifp, "\n");
        rp->tcount = (ROWT)tc;
#ifdef DEBUG
        for (cp = rp->rowp, j = num + num + 1 ; j-- ; cp++)
            printf(" %d", (int)(*(cp)));
        printf("\n");
#endif
        for (cp = rp->rowp, j = (int)(*(cp++)) ; j-- ;) {
            mult = *(cp++); np = *(cp++);
            kk = tc / mult;
            if ((int)(p_list[np - 1].ubound > kk))
                p_list[np - 1].ubound = (ROWT)kk;
        }
        if ((num == 1) && (kk > (int)(p_list[np - 1].lbound)))
            p_list[np - 1].lbound = (ROWT)kk;
    }
    for (tcp = t_in_count, tip = t_inp, ii = trans_num ; ii-- ;
            tcp++, tip++) {
        *tcp = cp = (ROWP)ecalloc(no_M_invars, ROWT_SIZE);
        for (rp = M_invars, j = no_M_invars ; j-- ; cp++, rp++)
            *cp = inv_prod(*tip, rp->rowp);
    }
#ifdef DEBUG
    printf("  End of load_Pinvars\n");
#endif
}

int struct_me(t1, t2)
int t1, t2;
{
    ROWP p1p = *(t_inp + (t1 - 1)), p2p = *(t_inh + (t2 - 1));

#ifdef DEBUG
    printf("    Start of struct_me %d %d\n", t1, t2);
#endif
    while (*p2p) {
        while (*p1p && *(p1p + 1) < * (p2p + 1))
#ifdef DEBUG
        {
            printf("        scan *p1p=%d *p1p+1=%d *p2p=%d *p2p+1=%d\n",
                   (int)*p1p, (int) * (p1p + 1), (int)*p2p, (int) * (p2p + 1));
#endif
            p1p += 2;
#ifdef DEBUG
        }
#endif
        if (*p1p) {
#ifdef DEBUG
            printf("      *p1p=%d *p1p+1=%d *p2p=%d *p2p+1=%d\n",
                   (int)*p1p, (int) * (p1p + 1), (int)*p2p, (int) * (p2p + 1));
#endif
            if (*(p1p + 1) == *(p2p + 1) && *p1p >= *p2p)
#ifdef DEBUG
            {
                printf("    End of struct_me %d %d true\n", t1, t2);
#endif
                return (TRUE);
#ifdef DEBUG
            }
#endif
            p2p += 2;
        }
        else break;
    }
#ifdef DEBUG
    printf("     testing %d %d\n", t2, t1);
#endif
    p1p = *(t_inh + (t1 - 1)); p2p = *(t_inp + (t2 - 1));
    while (*p2p) {
        while (*p1p && *(p1p + 1) < * (p2p + 1))
#ifdef DEBUG
        {
            printf("        scan *p1p=%d *p1p+1=%d *p2p=%d *p2p+1=%d\n",
                   (int)*p1p, (int) * (p1p + 1), (int)*p2p, (int) * (p2p + 1));
#endif
            p1p += 2;
#ifdef DEBUG
        }
#endif
        if (*p1p) {
#ifdef DEBUG
            printf("      *p1p=%d *p1p+1=%d *p2p=%d *p2p+1=%d\n",
                   (int)*p1p, (int) * (p1p + 1), (int)*p2p, (int) * (p2p + 1));
#endif
            if (*(p1p + 1) == *(p2p + 1) && *p1p <= *p2p)
#ifdef DEBUG
            {
                printf("    End of struct_me %d %d true\n", t1, t2);
#endif
                return (TRUE);
#ifdef DEBUG
            }
#endif
            p2p += 2;
        }
        else break;
    }
#ifdef DEBUG
    printf("    End of struct_me %d %d false\n", t1, t2);
#endif
    return (FALSE);
}

void bag_inters(ROWP bag1, ROWP  bag2, ROWP inters) {
#ifdef DEBUG
    ROWP pppp = inters;
    printf("      Start of bag_inters\n");
#endif
    while (*bag1) {
        while (*bag2 && *(bag2 + 1) < * (bag1 + 1))
            bag2 += 2;
        if (*bag2) {
            if (*(bag1 + 1) == *(bag2 + 1)) {
                *(inters++) = (*bag1 < *bag2) ? *bag1 : *bag2;
                *(inters++) = *(bag1 + 1);
                bag2 += 2;
            }
        }
        else
            break;
        bag1 += 2;
    }
    *inters = 0;
#ifdef DEBUG
    while (*pppp) {
        printf(" %d %d, ", (int)*pppp, (int) * (pppp + 1));
        pppp += 2;
    }
    printf("\n      End of bag_inters\n");
#endif
}

int mark_me(t1, t2)
int t1, t2;
{
    struct I_header *rp = M_invars;
    ROWP tcp1 = *(t_in_count + (--t1)), tcp2 = *(t_in_count + (--t2));
    int i = no_M_invars;

#ifdef DEBUG
    printf("    Start of mark_me %d %d\n", t1 + 1, t2 + 1);
#endif
    bag_inters(*(t_inp + t1), *(t_inp + t2), inter);
    for (; i-- ; rp++, tcp1++, tcp2++) {
        if ((*tcp1 + *tcp2 - inv_prod(inter, rp->rowp)) > rp->tcount)
#ifdef DEBUG
        {
            printf("    End of mark_me %d %d true\n", t1 + 1, t2 + 1);
            printf("      tc1=%d, tc2=%d, tcinter=%d, itc=%d\n",
                   (int)*tcp1, (int)*tcp2, (int)inv_prod(inter, rp->rowp), (int)rp->tcount);
#endif
            return (TRUE);
#ifdef DEBUG
        }
#endif
    }
#ifdef DEBUG
    printf("    End of mark_me %d %d false\n", t1 + 1, t2 + 1);
#endif
    return (FALSE);
}

void compute_me() {
    int ii = trans_num;
    ARCM t1p = me_rel;

#ifdef DEBUG
    printf("  Start of compute_me\n");
#endif
    for (; ii-- ; t1p++) {
        int it1 = (trans_num - ii);
        ROWP t2p = (*t1p) + it1;
        int jj = ii;
        for (; jj-- ; t2p++) {
            int it2 = (trans_num - jj);
            ROWP tt1p = (*(me_rel + (it2 - 1))) + (it1 - 1);
            if (struct_me(it1, it2) || mark_me(it1, it2)) {
                *t2p = TRUE;
                *tt1p = TRUE;
                fprintf(ofp, "%d %d\n", it1, it2);
            }
        }
    }
    fprintf(ofp, "0\n");
#ifdef DEBUG
    printf("  End of compute_me\n");
#endif
}

void merge_ecs(unsigned t1_ecs, unsigned t2_ecs) {
    unsigned t1offs = t1_ecs - 1, t2offs = t2_ecs - 1;

#ifdef DEBUG
    printf("      Start of merge_ecs %d %d\n", t1_ecs, t2_ecs);
#endif
    if (t1_ecs < t2_ecs) {
        T_L_P *phead1 = (ecs_listp + (t1offs));
        T_L_P *phead2 = (ecs_listp + (t2offs));
        T_L_P npp = *phead2;
        T_L_P ppp = NULL;
        for (; npp != NULL ; ppp = npp, npp = npp->next)
            * (t_ecs + (npp->trans - 1)) = t1_ecs;
        if (ppp) ppp->next = *phead1;
        *phead1 = *phead2; *phead2 = NULL;
    }
    else {
        T_L_P *phead1 = (ecs_listp + (t1offs));
        T_L_P *phead2 = (ecs_listp + (t2offs));
        T_L_P npp =  *phead1;
        T_L_P ppp = NULL;
        for (; npp != NULL ; ppp = npp, npp = npp->next)
            * (t_ecs + (npp->trans - 1)) = t2_ecs;
        if (ppp) ppp->next = *phead2;
        *phead2 = *phead1; *phead1 = NULL;
    }
#ifdef DEBUG
    printf("      End of merge_ecs\n");
#endif
}

void compute_sc() {
    int np, nt;
    ARCM ti_p, to_p ;
    ARCM po_p, ph_p;
    ARCM me_list = me_rel;
    ROWP cc_list, lp, t1_ecs = t_ecs, t1_pp = t_pri;
#ifdef DEBUG
    printf("  Start of compute_sc\n");
#endif
    lp = cc_list = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
    for (np = trans_num + 1 ; np-- ; * (lp++) = 0);
    for (ti_p = t_inp, to_p = t_out, nt = trans_num ; nt-- ;
            ti_p++, to_p++, me_list++, t1_ecs++, t1_pp++) {
        ROWP point, t2_ecs, t2_pp;
        ROWP pp = diffbag;
        int no_con = 0;
        ROWP me_t1 = *me_list;
#ifdef DEBUG
        printf("    conflicts caused by transition %d\n", trans_num - nt);
#endif
        bag_diff(*ti_p, *to_p, diffbag);
        lp = cc_list;
        for (; *pp > 0 ; pp++) {
            ROWP tp;
            po_p = p_out + (unsigned)(*(++pp) - 1);
            for (tp = *po_p; (int) * (tp++) ; tp++) {
                int sum2 = (*tp) - 1;
                if (!(int)(*(me_t1 + sum2)) && *t1_pp >= *(t_pri + sum2)) {
#ifdef DEBUG
                    printf("        %d\n", (int)*tp);
#endif
                    point = cc_list + sum2;
                    if (!(int)(*point))
                        no_con++;
                    *point = 1;
                }
#ifdef DEBUG
                else
                    printf("        %d is me\n", (int)*tp);
#endif
            }
        }
#ifdef DEBUG
        printf("      inp -> inp\n");
#endif
        bag_diff(*to_p, *ti_p, diffbag);
        for (pp = diffbag, lp = cc_list; *pp > 0 ; pp++) {
            ROWP tp;
            ph_p = p_inh + (unsigned)(*(++pp) - 1);
            for (tp = *ph_p; (int) * (tp++) ; tp++) {
                int sum2 = (*tp) - 1;
                if (!(int)(*(me_t1 + sum2)) && *t1_pp >= *(t_pri + sum2)) {
#ifdef DEBUG
                    printf("        %d\n", (int)*tp);
#endif
                    point = cc_list + sum2;
                    if (!(int)(*point))
                        no_con++;
                    *point = 1;
                }
#ifdef DEBUG
                else
                    printf("        %d is me\n", (int)*tp);
#endif
            }
        }
#ifdef DEBUG
        printf("      out -> inh\n");
#endif
        fprintf(ofp, "%d", no_con);
        for (lp = cc_list, np = trans_num, t2_ecs = t_ecs, t2_pp = t_pri ;
                np-- ;
                * (lp++) = 0, t2_ecs++, t2_pp++)
            if ((int)(*lp)) {
                ROWP pp = *(sc_rel + (trans_num - np - 1));
                fprintf(ofp, " %d", trans_num - np);
                while ((int)(*pp))
                    pp++;
                *(pp++) = (trans_num - nt);
                *pp = 0;
                if ((int)(*t1_pp) && (*t1_ecs != *t2_ecs) &&
                        (*t1_pp == *t2_pp))
                    merge_ecs((unsigned)*t1_ecs, (unsigned)*t2_ecs);
            }
        fprintf(ofp, "\n");
    }
    free((char *)cc_list);
#ifdef DEBUG
    printf("  End of compute_sc\n");
#endif
}

ARCM tstack;
ROWP tflags;


void bag_inhdiff(ROWP bag1, ROWP  bag2, ROWP  inhdiff) {
#ifdef DEBUG
    printf("      Start of bag_inhdiff\n");
#endif
    while (*bag1) {
        while (*bag2 && *(bag2 + 1) < * (bag1 + 1))
            bag2 += 2;
        if (*bag2) {
            if (*(bag1 + 1) == *(bag2 + 1)) {
                if (*bag1 >= *bag2) {
                    bag1 += 2; bag2 += 2;
                }
                else {
                    *(inhdiff++) = (*bag2 - * (bag1++)); bag2 += 2;
                    *(inhdiff++) = *(bag1++);
                }
            }
            else {
                *(inhdiff++) = *(bag1++); *(inhdiff++) = *(bag1++);
            }
        }
        else
            break;
    }
    while (*bag1) {
        *(inhdiff++) = *(bag1++); *(inhdiff++) = *(bag1++);
    }
    *inhdiff = 0;
#ifdef DEBUG
    printf("      End of bag_inhdiff\n");
#endif
}

void CCLpi(ROWP tki_p, ROWP  tkh_p, ROWP  tmi_p, ROWP  tmh_p, ARCM stack) {
    int no_con = 0, ii;
    ROWP lp = *stack;
    ROWP pp = *(stack + 1);
#ifdef DEBUG
    printf("    Start of CCLpi %d\n", (int)(stack - tstack));
#endif
    for (ii = trans_num ; ii-- ; * (lp++) = 0);
#ifdef DEBUG
    printf("     stack cleared\n");
#endif
    lp = *stack;
    bag_diff(tmi_p, tki_p, *(stack + 1));
    for (; *pp ; pp++) {
        ROWP tp;
        ARCM pi_p = p_inp + (unsigned)(*(++pp) - 1);
        ARCM po_p = p_out + (unsigned)(*(pp) - 1);
        bag_diff(*pi_p, *po_p, diffbag);
        for (tp = diffbag; (int) * (tp++) ; tp++) {
            unsigned tloffs = (*tp) - 1;
#ifdef DEBUG
            printf("        %d\n", (int)*tp);
#endif
            if (!(int)(*(tflags + tloffs))) {
                ROWP point = lp + tloffs;
                no_con++;
                *point = 1;
                *(tflags + tloffs) = 1;
            }
        }
    }
#ifdef DEBUG
    printf("      out -> inp\n");
#endif
    bag_inhdiff(tmh_p, tkh_p, *(stack + 1));
    for (pp = *(stack + 1); *pp ; pp++) {
        ROWP tp;
        ARCM po_p = p_out + (unsigned)(*(++pp) - 1);
        ARCM pi_p = p_inp + (unsigned)(*(pp) - 1);
        bag_diff(*po_p, *pi_p, diffbag);
        for (tp = diffbag; (int) * (tp++) ; tp++) {
            unsigned tloffs = (*tp) - 1;
            if (!(int)(*(tflags + tloffs))) {
                ROWP point = lp + tloffs;
#ifdef DEBUG
                printf("        %d\n", (int)*tp);
#endif
                no_con++;
                *point = 1;
                *(tflags + tloffs) = 1;
            }
#ifdef DEBUG
            else
                printf("        %d is flagged\n", (int)*tp);
#endif
        }
    }
#ifdef DEBUG
    printf("      inp -> inh\n");
#endif
    for (pp = *stack, ii = trans_num ; no_con && ii-- ; lp++)
        if ((int)(*lp)) {
            no_con--;
            *(pp++) = (trans_num - ii);
        }
    *pp = 0;
#ifdef DEBUG
    printf("    End of CCLpi\n");
#endif
}

void breadth_first(ROWP tki_p, ROWP  tkh_p, ROWP  tmi_p, ROWP  tmh_p,
                   unsigned pi, unsigned tkoffs,  unsigned tmoffs,
                   ARCM stack) {
    ROWP tl_p = *stack;
    ROWP tk_ecs = (t_ecs + tkoffs);

#ifdef DEBUG
    printf("   Start of breadth_first %d\n", (int)(stack - tstack));
#endif
    CCLpi(tki_p, tkh_p, tmi_p, tmh_p, stack);
    for (; *tl_p ; tl_p++) {
        unsigned tloffs = (*tl_p) - 1;
#ifdef DEBUG
        printf("    testing trans  %d\n", tloffs + 1);
#endif
        if (*(t_pri + tloffs) == pi) {
            if (*tk_ecs != *(t_ecs + tloffs))
                merge_ecs((unsigned)*tk_ecs,
                          (unsigned) * (t_ecs + tloffs));
#ifdef DEBUG
            else
                printf("      already in ecs\n");
#endif
        }
        else {
            breadth_first(tki_p, tkh_p,  *(t_inp + tloffs), *(t_inh + tloffs),
                          pi, tkoffs,
                          tloffs,
                          stack + 1);
        }
    }
#ifdef DEBUG
    printf("   End of breadth_first\n");
#endif
}

void compute_ecs() {
    int nt;
    ARCM ti_p, th_p;
    ARCM me_list = me_rel, sc_list = sc_rel;
    ROWP t1_ecs = t_ecs, t1_pp = t_pri;
    ARCM ppp;
    T_L_P *ecsp;
#ifdef DEBUG
    printf("  Start of compute_ecs\n");
#endif
    tflags = (ROWP)ecalloc(trans_num, ROWT_SIZE);
    tstack = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    for (ppp = tstack, nt = trans_num ; nt-- ; ppp++)
        *ppp = (ROWP)ecalloc(trans_num + trans_num + 1, ROWT_SIZE);
    for (ti_p = t_inp, th_p = t_inh, nt = trans_num ; nt-- ;
            ti_p++, th_p++, me_list++, sc_list++, t1_ecs++, t1_pp++)
        if ((int)(*t1_pp)) {
            unsigned t1offs = trans_num - nt - 1;
            ROWP t1_scp = *sc_list;
            ROWP t1_mep = *me_list;
#ifdef DEBUG
            printf("   transition %d pri=%d\n", (trans_num - nt), (int)*t1_pp);
#endif
            for (; *t1_scp ; t1_scp++) {
                unsigned t2offs = (*t1_scp) - 1;
                if (*t1_ecs != *(t_ecs + t2offs) && *t1_pp < * (t_pri + t2offs)) {
                    unsigned ii;
                    ROWP tfp;
                    for (tfp = tflags, ii = trans_num ; ii-- ;)
                        *(tfp++) = ((*t1_pp > (*(t_pri + (trans_num - ii - 1)))) ||
                                    (int)(*(t1_mep + (trans_num - ii - 1))) ||
                                    (ii == nt) ||
                                    ((trans_num - ii) == (*t1_scp)));
                    breadth_first(*ti_p, *th_p,
                                  *(t_inp + t2offs), *(t_inh + t2offs),
                                  *t1_pp, t1offs, t2offs,
                                  tstack);
                }
            }
        }
#ifdef DEBUG
    printf("    end search\n");
#endif
    for (nt = trans_num, ecsp = ecs_listp ; nt ; ecsp++) {
        T_L_P ppp = *ecsp;
        unsigned numt = 0;
        for (; ppp != NULL ; ppp = ppp->next, numt++);
        nt -= numt;
        if (numt && (int)(*(t_pri + (((*ecsp)->trans) - 1)))) {
            fprintf(ofp, "%d", numt);
            for (ppp = *ecsp ; ppp != NULL ; ppp = ppp->next) {
                fprintf(ofp, " %d", (int)ppp->trans);
            }
            fprintf(ofp, "\n");
        }
    }
    fprintf(ofp, "0\n");
#ifdef DEBUG
    printf("  End of compute_ecs\n");
#endif
}

int necs;
int cur_pri;
ARCM tstackp;
int nt1;

void depth_first(ROWP tki_p, ROWP  tkh_p, ARCM stack, int sp, int ntm) {
    ROWP tmi_p, tmh_p, ttt_p;

#ifdef DEBUG
    printf("  Start of depth_first sp=%d\n", sp);
#endif
    tmi_p = *(t_inp + ntm - 1);
    tmh_p = *(t_inh + ntm - 1);
    CCLpi(tki_p, tkh_p, tmi_p, tmh_p, stack);
    for (ttt_p = *stack ; (ntm = *ttt_p) ; ++ttt_p)
        if (cur_pri == *(t_pri + (ntm - 1))) {
            int ii;
            fprintf(ofp, "%d %d %d %d", necs, sp + 3, nt1, ntm);
            for (ii = sp ; ii ; --ii)
                fprintf(ofp, " %d", (int)*tstackp[ii]);
            fprintf(ofp, " %d\n", (int) **tstackp);
        }
        else {
            tmi_p = *(t_inp + ntm - 1);
            tmh_p = *(t_inh + ntm - 1);
            tstackp[sp + 1] = ttt_p;
            depth_first(tki_p, tkh_p, stack + 1, sp + 1, ntm);
        }
#ifdef DEBUG
    printf("  End of depth_first sp=%d\n", sp);
#endif
}

void compute_confusion() {
    ARCM me_list = me_rel;
    T_L_P *ecsp;
    int nt;
#ifdef DEBUG
    printf("  Start of compute_confusion\n");
#endif
    tstackp = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    for (nt = trans_num, ecsp = ecs_listp, necs = 0 ; nt ; ecsp++) {
        T_L_P ppp = *ecsp;
        unsigned numt = 0;
        for (; ppp != NULL ; ppp = ppp->next, numt++);
        nt -= numt;
        if (numt && (cur_pri = (int)(*(t_pri + (((*ecsp)->trans) - 1))))) {
            ++necs;
            if (numt > 1) {
                T_L_P ppp1, ppp2;
                ROWP tki_p, tkh_p;
                for (ppp1 = *ecsp ; ppp1 != NULL ; ppp1 = ppp1->next) {
                    nt1 = ppp1->trans;
                    tki_p = *(t_inp + nt1 - 1);
                    tkh_p = *(t_inh + nt1 - 1);
                    for (ppp2 = *ecsp ; ppp2 != NULL ; ppp2 = ppp2->next) {
                        ROWP t1_mep = *(me_list + (nt1 - 1));
                        int nt2 = ppp2->trans;
                        if (nt1 != nt2) {
                            T_L_P ppp3 = *ecsp;
                            ARCM psp = tstackp;
                            ROWP tfp = tflags;
                            ROWP t_pp = t_pri;
                            int iii = trans_num + 1;
                            for (t_pp = t_pri ; --iii ; ++t_pp)
                                *(tfp++) = (
                                               (*t_pp < cur_pri) ||
                                               (int)(*(t1_mep + (trans_num - iii))));
                            for (; ppp3 != NULL ; ppp3 = ppp3->next)
                                * (tflags + (ppp3->trans - 1)) = 1;
                            for (iii = trans_num + 1 ; --iii ; * (psp++) = NULL);
                            tfp = *tstack; *(tfp++) = nt2; *tfp = 0;
                            *tstackp = *tstack;
                            depth_first(tki_p, tkh_p, tstack + 1, 0 , nt2);
                        }
                    }
                }
            }
        }
    }
    fprintf(ofp, "0\n");
#ifdef DEBUG
    printf("  End of compute_confusion\n");
#endif
}

void print_bounds() {
    int i = 0;
    struct place_obj 	*place = p_list;

    while (i++ < place_num) {
        fprintf(ofp, "%d %d\n", (int)(place->lbound), (int)(place->ubound));
        place++;
    }
}

int main(int argc, char **argv) {

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[1024];
    int ii;
    int only_bounds = 0;

#ifdef DEBUG
    printf("Start\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    for (ii = 2; ii < argc; ii++) {
        if (0 == strcmp(argv[ii], "-mpar")) {
            if (ii + 2 < argc) {
                if (num_mpars_to_change > MAX_MPARS_TO_CHANGE) {
                    fprintf(stderr, "\nError: too many -mpar switches. Raise the value of MAX_MPARS_TO_CHANGE.\n");
                    exit(1);
                }
                mpars_to_change[num_mpars_to_change].mpar_name = argv[++ii];
                mpars_to_change[num_mpars_to_change].new_val = atoi(argv[++ii]);
                num_mpars_to_change++;
            }
            else {
                fprintf(stderr, "\nError:  -mpar  <marking parameter name>  <new value>\n");
                exit(1);
            }
        }
        else if (0 == strcmp(argv[ii], "-only-bnd")) {
            only_bounds = 1;
        }
        else if (0 == strcmp(argv[ii], "-detect-exp")) {
            /* ignore, flag used by pinvar. */
        }
        else {
            fprintf(stderr, "Unknown command line option: %s\n", argv[ii]);
            exit(1);
        }
    }
    load_net();
    fclose(nfp);


    if (!only_bounds) {
        sprintf(filename, "%s.cc", argv[1]);
        if ((ofp = fopen(filename, "w")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'w');
            exit(1);
        }
        compute_cc();
        fclose(ofp);
        sprintf(filename, "%s.sub", argv[1]);
        if ((ofp = fopen(filename, "w")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'w');
            exit(1);
        }
        compute_sub();
        compute_place_sub();
        fclose(ofp);
    }

    // Load P-invariants and compute place bounds
    sprintf(filename, "%s.pin", argv[1]);
    if ((ifp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_Pinvars();
    fclose(ifp);

    // Save place bounds
    sprintf(filename, "%s.bnd", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    print_bounds();
    (void) fclose(ofp);

    if (only_bounds)
        return 0;

    sprintf(filename, "%s.me", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    compute_me();
    fclose(ofp);
    sprintf(filename, "%s.sc", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    compute_sc();
    fclose(ofp);
    sprintf(filename, "%s.ecs", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    compute_ecs();
    compute_confusion();
    fclose(ofp);


#ifdef DEBUG
    printf("End\n");
#endif
    return 0;
}

