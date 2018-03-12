/*
 *  program: disab_lp.c
 *  date: August 16, 1993
 *  programmer: Giovanni Chiola
 *  address:	Dipartimento di Informatica, Universita' di Torino
 *		corso Svizzera 185, 10149 Torino, Italy.
 *
 *  purpose: Decomposition of marking vector in disabling components
 *	     for each immediate transition with more than one
 *	     input/inhibition arcs.
 *	     Production of reachability constraints M = M0 + C s.
 *	     Production of throughput flow balance constraints for every place.
 *	     Detection of vanishing places.
 *	     Production of Extended Free Choice throughput constraints.
 *	     Production of inequalities for Persistent or Age Memory
 *	     or Preselection Timed Transitions.
 *  notes:
 *    1) the net description must be in GreatSPN 1.6 format
 *    2) structural conflicts of immediate transitions are obtimized
 *    3) the case of conflict with race policy and enabling memory for
 *	 timed transitions is not properly handled. The net description is
 *	 assumed not to contain such cases.
 */


/*
#define DEBUG
*/

#ifdef DEBUG

#	define DEBUGCALL
/*
*/

/*
#	define DEBUGload_net
*/

/*
#	define DEBUGcompute_ecs
*/

#	define DEBUGcompute_disab
/*
*/

/*
#	define DEBUGcompute_mark
*/

/*
#	define DEBUGcompute_flow
*/

#endif /* DEBUG */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *nfp, * ofp;

#include "const.h"

typedef short ROWT;
#define ROWT_SIZE (unsigned)(sizeof(ROWT))

typedef ROWT *ROWP;
#define ROWP_SIZE (unsigned)(sizeof(ROWP))

typedef ROWP *ARCM;
#define ARCM_SIZE (unsigned)(sizeof(ARCM))

ARCM p_inp, p_out, p_inh;


struct trans_d {
    char   *tname;
    float ftime;
    ROWT pri;
    ROWT ecs;
    ROWT no_ih;
    ROWP inp_arcs;
    ROWP inh_arcs;
    ROWP out_arcs;
};

typedef struct trans_d Trans_d;
typedef Trans_d *Trans_p;

Trans_p t_array;
ROWP t_ord;
ROWP t_inv;

int	place_num, trans_num;
/*predeclaration */
void getname(char 	*name_pr);
void getarcs(ARCM	t_arc, ARCM p_arc,
             int	noar, int notr);
void compute_ecs();
int compare_cond(ROWP what, int which);
void print_conds(Trans_p tp, unsigned ecs_card);
void produce_mark(unsigned from, unsigned  num);

char *ecalloc(unsigned nitm, unsigned sz) {
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}


ROWP    mparp;
ROWP    M0;


int t_cmp(const ROWP p1, const ROWP p2) {
    unsigned cur_pl1 = 0, cur_pl2;
    int cur_wp1, cur_wp2;
    ROWP iap1, hap1;
    ROWP iap2, hap2;
    Trans_p t1 = t_array + (int)(*p1);
    Trans_p t2 = t_array + (int)(*p2);

    if (t1->pri > t2->pri)
        return (1);
    if (t1->pri < t2->pri)
        return (-1);
    if (t1->ecs > t2->ecs)
        return (1);
    if (t1->ecs < t2->ecs)
        return (-1);
    iap1 = (t1->inp_arcs) + 1;
    hap1 = (t1->inh_arcs) + 1;
    iap2 = (t2->inp_arcs) + 1;
    hap2 = (t2->inh_arcs) + 1;
    do {
        if ((cur_wp1 = *iap1)) {
            cur_pl1 = *(iap1 + 1);
            if (*hap1) {
                if (*(hap1 + 1) <= cur_pl1) {
                    cur_wp1 = *(hap1++);
                    cur_pl1 = -(int)(*(hap1++));
                }
                else iap1 += 2;
            }
            else iap1 += 2;
        }
        else if ((cur_wp1 = -(int)(*(hap1++))))
            cur_pl1 = *(hap1++);
        if ((cur_wp2 = *iap2)) {
            cur_pl2 = *(iap2 + 1);
            if (*hap2) {
                if (*(hap2 + 1) <= cur_pl2) {
                    cur_wp2 = *(hap2++);
                    cur_pl2 = -(int)(*(hap2++));
                }
                else iap2 += 2;
            }
            else iap2 += 2;
        }
        else if ((cur_wp2 = -(int)(*(hap2++))))
            cur_pl2 = *(hap2++);
        if (!cur_wp1 || !cur_wp2) {
            if (cur_wp2)
                return (1);
            if (cur_wp1)
                return (-1);
            return (0);
        }
        if (cur_pl1 < cur_pl2)
            return (1);
        if (cur_pl1 > cur_pl2)
            return (-1);
        if (cur_wp1 != cur_wp2) {
            if (cur_wp1 > 0) {
                if (cur_wp2 > 0)
                    return ((cur_wp1 > cur_wp2) ? 1 : -1);
                else
                    return (-1);
            }
            else {
                if (cur_wp2 > 0)
                    return (1);
                else
                    return ((cur_wp1 < cur_wp2) ? 1 : -1);
            }
        }
        if (cur_wp1 < cur_wp2)
            return (1);
        if (cur_wp1 > cur_wp2)
            return (-1);
    }
    while (TRUE);
}


float *rpars;

unsigned count_conds = 0;

char **place_nam;


void load_net() {

    ARCM		pp1, pp2, pp3;
    ROWP		rwp;
    Trans_p tdp;
    char		linebuf[LINEMAX];
    char **cpp;
    float		ftemp, x, y;
    float *pf;
    int                 group_num, i, j, knd, noar, mark, nomp, norp;
#define VBAR '|'

#ifdef DEBUGCALL
    fprintf(stderr, "  Start of load_net\n");
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
    fscanf(nfp, "%d %d %d %d\n", &trans_num, &group_num, &i, &j);

    mparp = (ROWP)ecalloc(nomp, short_SIZE);
    pp1 = p_inp = (ARCM)ecalloc(place_num, ROWP_SIZE);
    pp2 = p_out = (ARCM)ecalloc(place_num, ROWP_SIZE);
    pp3 = p_inh = (ARCM)ecalloc(place_num, ROWP_SIZE);
    for (i = place_num ; i-- ; pp1++, pp2++, pp3++) {
        register ROWP cp;
        unsigned int l = trans_num + trans_num + 1;
        *pp1 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
        *pp2 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
        *pp3 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
    }

#ifdef DEBUGload_net
    fprintf(stderr, "    %d marking parameters\n", nomp);
#endif
    /* read marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        while (i < nomp) {
            getname(linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUGload_net
            fprintf(stderr, "    %d %s=%d\n", i, linebuf, mark);
#endif
            mparp[i++] = (ROWT)mark;
        }
    }

#ifdef DEBUGload_net
    fprintf(stderr, "    %d places\n", place_num);
#endif
    /* read places */
    cpp = place_nam = (char **)ecalloc(place_num, sizeof(char *));
    rwp = M0 = (ROWP)ecalloc(place_num, short_SIZE);
    for (i = place_num; i-- ; cpp++, rwp++) {
        getname(linebuf);
        *cpp = ecalloc(strlen(linebuf) + 1, 1);
        strcpy(*cpp, linebuf);
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
        if (mark < 0)
            mark = mparp[-1 - mark];
#ifdef DEBUGload_net
        fprintf(stderr, "      place %s %d\n", linebuf, mark);
#endif
        *rwp = (short)mark;
    }

#ifdef DEBUGload_net
    fprintf(stderr, "    %d rate parameters\n", norp);
#endif
    /* read rate parameters */
    pf = rpars = (float *)ecalloc(norp, sizeof(float));
    i = 1;
    while (i++ <= norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp); while (getc(nfp) != '\n');
        *(pf++) = ftemp;
    }

#ifdef DEBUGload_net
    fprintf(stderr, "    %d groups\n", group_num);
#endif
    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        fscanf(nfp, "%f %f", &x, &y);
        while (getc(nfp) != '\n');
    }

#ifdef DEBUGload_net
    fprintf(stderr, "    %d transitions\n", trans_num);
#endif
    /* read transitions */
    tdp = t_array = (Trans_p)ecalloc(trans_num, sizeof(Trans_d));
    rwp = t_ord = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
    t_inv = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
    for (i = trans_num ; i ; i--, tdp++) {
        int nrp = 0;
        *++rwp = trans_num - i;
        getname(linebuf);
#ifdef DEBUGload_net
        fprintf(stderr, "        trans %s\n", linebuf);
#endif
        tdp->tname = ecalloc(strlen(linebuf) + 1, 1);
        strcpy(tdp->tname, linebuf);
        fscanf(nfp, "%f %d %d %d ", &ftemp, &mark, &knd, &noar);
        while (getc(nfp) != '\n');
        count_conds += noar;
        if ((ftemp < 0.0 && (nrp = (int)(-ftemp)) > norp) || mark < 0) {
            fprintf(stderr, "Sorry, no marking dependency allowed (transition %s)\n",
                    linebuf);
            exit(1);
        }
        if (nrp)
            ftemp = rpars[nrp - 1];
        if (knd == DETERM)
            tdp->ftime = ftemp;
        else
            tdp->ftime = 1.0 / ftemp;
        tdp->ecs = 0;
        tdp->no_ih = noar;
        if (!(tdp->pri = (knd == DETERM) ? 0 : knd) && mark) {
            fprintf(ofp, "   %g >= x_%s;\n", mark / (tdp->ftime), linebuf);
            (tdp->no_ih)++;
        }
        getarcs(&(tdp->inp_arcs), p_out, noar, trans_num - i + 1);
        fscanf(nfp, "%d\n", &noar);
        getarcs(&(tdp->out_arcs), p_inp, noar, trans_num - i + 1);
        fscanf(nfp, "%d\n", &noar);
        if (noar) {
            count_conds += noar;
            tdp->no_ih += noar;
        }
        getarcs(&(tdp->inh_arcs), p_inh, noar, trans_num - i + 1);
    }
    compute_ecs();
    qsort(t_ord + 1, trans_num, ROWT_SIZE, (int (*)(const void *, const void *)) t_cmp);
    for (rwp = t_ord + 1, i = trans_num ; i-- ; rwp++) {
        int j = *rwp;
        t_inv[j] = trans_num - i;
    }
    fprintf(ofp, "\n");
#ifdef DEBUGCALL
    fprintf(stderr, "  End of load_net\n");
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

int a_cmp(ROWP p1, ROWP p2) {
    register int i1 = *(p1++), i2 = *(p2++);

    if (*p1 == *p2) {
        if (i1 == i2)
            return (0);
        return ((i2 > i1) ? -1 : 1);
    }
    return ((*p2 > *p1) ? -1 : 1);
}

void getarcs(ARCM	t_arc, ARCM p_arc,
             int	noar, int notr) {
    ROWP	tp, pp;
    int		i, j, pl, mlt, ip;
    float	x, y;

#ifdef DEBUGload_net
    fprintf(stderr, "        Start of getarcs %d\n", noar);
#endif
    tp = *t_arc = (ROWP)ecalloc(noar + noar + 2, ROWT_SIZE);
    *(tp++) = noar;
    for (i = noar ; i-- ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
#ifdef DEBUGload_net
        fprintf(stderr, "          arc p%d w=%d\n", pl, mlt);
#endif
        while (getc(nfp) != '\n');
        for (j = 1; j++ <= ip; fscanf(nfp, "%f %f\n", &x, &y));
        *(tp++) = (ABS(mlt));
        *(tp++) = pl;
#ifdef DEBUGload_net
        fprintf(stderr, "            adding to place\n");
#endif
        pp = *(p_arc + (pl - 1));
        while (*pp > 0)
            pp += 2;
        *(pp++) = (ABS(mlt));
        *(pp++) = notr;
        *pp = 0;
    }
    *(tp) = 0;
#ifdef DEBUGload_net
    fprintf(stderr, "          calling qsort\n");
#endif
    tp = (*t_arc) + 1;
    qsort(tp, noar, 2 * ROWT_SIZE, (int (*)(const void *, const void *)) a_cmp);
#ifdef DEBUGload_net
    fprintf(stderr, "        End of getarcs\n");
#endif
}


int not_me(Trans_p tp1, Trans_p tp2) {
    ROWP pp1, pp2;

    for (pp1 = (tp1->inp_arcs) + 1, pp2 = (tp2->inh_arcs) + 1 ; *pp1 && *pp2 ;) {
        if (*(pp1 + 1) > *(pp2 + 1))
            pp2 += 2;
        else if (*(pp1 + 1) < * (pp2 + 1))
            pp1 += 2;
        else {
            if (*pp1 >= *pp2)
                return (FALSE);
            pp1 += 2;
            pp2 += 2;
        }
    }
    for (pp1 = (tp1->inh_arcs) + 1, pp2 = (tp2->inp_arcs) + 1 ; *pp1 && *pp2 ;) {
        if (*(pp1 + 1) > *(pp2 + 1))
            pp2 += 2;
        else if (*(pp1 + 1) < * (pp2 + 1))
            pp1 += 2;
        else {
            if (*pp1 <= *pp2)
                return (FALSE);
            pp1 += 2;
            pp2 += 2;
        }
    }
    return (TRUE);
}


void compute_ecs() {
    int nn = trans_num;
    int last_ecs = 0;
    Trans_p nex_t = t_array;
    ROWP tlist = (ROWP)ecalloc(trans_num, ROWT_SIZE);
    ROWP tlp1, tlp2;
    ROWP plp;

#ifdef DEBUGCALL
    fprintf(stderr, "     Start of compute_ecs\n");
#endif
    while (nn--) {
        int i;
        while (nex_t->ecs) {
#ifdef DEBUGcompute_ecs
            fprintf(stderr, "        compute_ecs: skipping transition\n");
#endif
            nex_t++;
        }
        nex_t->ecs = ++last_ecs;
        *(tlp1 = tlist) = (i = 1 + (nex_t - t_array));
#ifdef DEBUGcompute_ecs
        fprintf(stderr, "        compute_ecs: trans %d first of ecs %d\n",
                i - 1, last_ecs);
#endif
        *(tlp2 = tlist + 1) = 0;
        nex_t++;
        while ((i = *(tlp1++))) {
            Trans_p tp = t_array + --i;
#ifdef DEBUGcompute_ecs
            fprintf(stderr, "          compute_ecs: testing trans %d (pri=%d)\n",
                    i, tp->pri);
#endif
            for (plp = (tp->inp_arcs) + 1 ; * (plp++) ;) {
                Trans_p tp2;
                ROWP tnp;
                i = *(plp++) - 1;
#ifdef DEBUGcompute_ecs
                fprintf(stderr, "            compute_ecs: testing place %d\n", i + 1);
#endif
                for (tnp = *(p_out + i) ; * (tnp++) ;) {
                    i = *(tnp++);
#ifdef DEBUGcompute_ecs
                    fprintf(stderr, "             is input of t%d\n", i);
#endif
                    tp2 = t_array + (i = t_ord[i]);
#ifdef DEBUGcompute_ecs
                    fprintf(stderr, "               (converted to %d) ecs=%d pri=%d\n",
                            i, tp2->ecs, tp2->pri);
#endif
                    if ((!(tp2->ecs)) && (tp->pri) && (tp2->pri)) {
                        /* ?????????????????????????????
                            if ( (!(tp2->ecs)) && (tp->pri == tp2->pri) ) {
                        #ifdef DEBUGcompute_ecs
                        fprintf(stderr,"              compute_ecs: testing HME\n");
                        #endif
                        	if ( not_me(tp,tp2) ) {
                        ????????????????????????????? */
#ifdef DEBUGcompute_ecs
                        fprintf(stderr, "              compute_ecs: adding t%d\n", i);
#endif
                        nn--;
                        tp2->ecs = last_ecs;
                        *tlp2 = i + 1;
                        *(++tlp2) = 0;
                        /* ?????????????????????????????
                        	  }
                        ????????????????????????????? */
                    }
                }
            }
        }
    }
#ifdef DEBUGCALL
    fprintf(stderr, "     End of compute_ecs\n");
#endif
}


ROWP disl;

int test_disabl(int ineq,
                unsigned place,
                ROWP pp) {
    unsigned p2;
    int i2;

#ifdef DEBUGcompute_disab
    fprintf(stderr, "        Start of test_disabl ineq=%d, place=%d\n",
            ineq, place);
#endif
    while (*pp) {
        i2 = *(pp++);
        p2 = *(pp++);
        if (i2 > 0 && ineq > 0) {
            if (p2 == place && i2 >= ineq) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "        End of test_disabl <- FALSE\n");
#endif
                return (FALSE);
            }
        }
        if (i2 < 0 && ineq < 0) {
            if (p2 == place && i2 <= ineq) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "        End of test_disabl <- FALSE\n");
#endif
                return (FALSE);
            }
        }
    }
#ifdef DEBUGcompute_disab
    fprintf(stderr, "        End of test_disabl <- TRUE\n");
#endif
    return (TRUE);
}


ROWP disl2;

ARCM conds;
int cur_cond = 0;
int from_cond = 0;


void add_cond(ROWP what,
              int where,
              int test_sub) {
    ARCM cp;
    ROWP rwp;
    int i = cur_cond - where;

#ifdef DEBUGcompute_disab
    fprintf(stderr, "           Start of add_cond where=%d, cur_cond=%d, tst=%d\n",
            where, cur_cond, test_sub);
#endif
    if (test_sub < cur_cond) {
        int j = (*what) + 1;
        int sub = 0;
        ROWP aux = rwp = (ROWP)ecalloc((int)(j << 1), ROWT_SIZE);
        ROWP aux2 = what;
        ARCM cp2 = conds + test_sub;
        while (*aux2) {
            *(rwp++) = *(aux2++);
            *(rwp++) = *(aux2++);
        }
        *rwp = 0;
        for (cp = conds + where ; cp < cp2 ; cp++, i--) {
#ifdef DEBUGcompute_disab
            fprintf(stderr, "             add_cond: shifting cond%d\n",
                    (int)(cp - conds));
#endif
            aux2 = *cp;
            *cp = aux;
            aux = aux2;
        }
        j = cp - conds;
        while (i-- > 0) {
#ifdef DEBUGcompute_disab
            fprintf(stderr, "             add_cond: testing cond%d for subset\n", j);
#endif
            if (compare_cond(what, j) == -1) {
                if (aux != NULL) {
                    *(cp++) = aux;
                    aux = NULL;
                }
                sub++;
            }
            else {
                if (aux != NULL) {
                    aux2 = *cp;
                    *(cp++) = aux;
                    aux = aux2;
                }
                else
                    *(cp++) = *cp2;
            }
            cp2++; j++;
        }
        if (sub > 0)
            cur_cond -= sub;
        else
            *cp = aux;
#ifdef DEBUGcompute_disab
        fprintf(stderr, "           End of add_cond (removed %d conds)\n", sub);
#endif
        return;
    }
    for (cp = conds + cur_cond ; i-- ; cp--)
        *cp = *(cp - 1);
    for (rwp = what, i = 1 ; *rwp ; rwp += 2, i += 2);
    *cp = rwp = (ROWP)ecalloc(i, ROWT_SIZE);
    while (*what) {
        *(rwp++) = *(what++);
        *(rwp++) = *(what++);
    }
    *rwp = 0;
#ifdef DEBUGcompute_disab
    fprintf(stderr, "           End of add_cond (no conds to test)\n");
#endif
}


void subst_cond(ROWP what,
                int where) {
    ARCM cp = conds + where;
    ARCM cp2 = cp - 1;
    ARCM cp3 = cp;
    ARCM cp4;
    int j = (*what) + 1;
    ROWP rwp;
    ROWP aux = rwp = (ROWP)ecalloc((int)(j << 1), ROWT_SIZE);
    ROWP aux2 = what;
    int i = where;
    int count = 0;
    int sub = 0;

#ifdef DEBUGcompute_disab
    fprintf(stderr, "           Start of subst_cond where=%d\n", where);
#endif
    while (*aux2) {
        *(rwp++) = *(aux2++);
        *(rwp++) = *(aux2++);
    }
    *rwp = 0;
    free(*cp);
    *cp = NULL;
    while (i-- && **cp2 > *what) {
        if (compare_cond(what, i) == -1) {
            free(*cp2);
            *cp2 = NULL;
            cp3 = cp2;
            sub++;
        }
        else
            count++;
        cp2--;
    }
    if (count > 0 && (cp4 = cp3 - 1) > cp2) {
        count--;
        while (cp4 > cp2) {
            *(cp4 + 1) = *cp3;
            *cp4 = NULL;
            cp4--;
        }
        while (*cp3 != NULL)
            cp3++;
    }
    for (cp4 = cp3 + 1 ; count > 0 ; cp4++) {
        if (*cp4 != NULL) {
            *(cp3++) = *cp4;
            *cp4 = NULL;
            count--;
        }
    }
    for (i = where + 1, cp++ ; i < cur_cond ; i++, cp++) {
        if (compare_cond(what, i) == -1) {
            free(*cp);
            sub++;
        }
        else
            *(cp3++) = *cp;
        *cp = NULL;
    }
    cur_cond -= sub;
    for (i = cp2 - conds ; i >= 0 ; cp2--, i--) {
        if (compare_cond(what, i) == 2) {
            *(cp2 + 1) = aux;
#ifdef DEBUGcompute_disab
            fprintf(stderr, "           End of subst_cond\n");
#endif
            return;
        }
        *(cp2 + 1) = *cp2;
    }
    if (i < 0)
        cp2++;
    *cp2 = aux;
#ifdef DEBUGcompute_disab
    fprintf(stderr, "           End of subst_cond\n");
#endif
}

int compare_cond(ROWP what, int which) {
    int in1, in2;
    ROWP rwp = *(conds + which);
    int subset = FALSE;
    int supset = FALSE;
    int lenght = 0;

#ifdef DEBUGcompute_disab
    fprintf(stderr, "           Start of compare_cond which=%d\n", which);
    {
        ROWP xx = what + 1;
        int i1, i2;
        fprintf(stderr, "             what=%d ", *what);
        while (*xx) {
            i1 = *(xx++);
            i2 = *(xx++);
            fprintf(stderr, " %d %d", i1, i2);
        }
        fprintf(stderr, " 0\n             *which=%d", *rwp);
        xx = rwp + 1;
        while (*xx) {
            i1 = *(xx++);
            i2 = *(xx++);
            fprintf(stderr, " %d %d", i1, i2);
        }
        fprintf(stderr, " 0\n");
    }
#endif
    if (*what > *rwp) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "            compare_cond: longer\n");
#endif
        lenght = 1;
    }
    else if (*what < *rwp) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "            compare_cond: shorter\n");
#endif
        lenght = -1;
    }
#ifdef DEBUGcompute_disab
    else
        fprintf(stderr, "            compare_cond: same size\n");
#endif
    what++; rwp++;
    in1 = *(rwp++);
    in2 = *(what++);
    while (in1 && in2) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "             compare_cond: *what=%d, *rwp=%d\n",
                *what, *rwp);
#endif
        if (*what < *rwp) {
            if (lenght < 0) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "           End of compare_cond <- -4\n");
#endif
                return (-4);
            }
            if (lenght == 0) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "           End of compare_cond <- -2\n");
#endif
                return (-2);
            }
            while ((in2 = *(++what)) && *(++what) < *rwp);
            if (! in2) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "           End of compare_cond <- +3\n");
#endif
                return (3);
            }
            else {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "               compare_cond: supset, *what=%d, *rwp=%d\n",
                        *what, *rwp);
#endif
                supset = TRUE;
            }
        }
        if (*what > *rwp) {
            if (lenght > 0) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "           End of compare_cond <- 4\n");
#endif
                return (4);
            }
            if (lenght == 0) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "           End of compare_cond <- 2\n");
#endif
                return (2);
            }
            while ((in1 = *(++rwp)) && *what > *(++rwp));
            if (! in1 || *what < *rwp) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "           End of compare_cond <- -3\n");
#endif
                return (-3);
            }
            else {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "               compare_cond: subset, *what=%d, *rwp=%d\n",
                        *what, *rwp);
#endif
                subset = TRUE;
            }
        }
#ifdef DEBUGcompute_disab
        fprintf(stderr, "               compare_cond: in1=%d, in2=%d, sub=%d, sup=%d\n",
                in1, in2, subset, supset);
#endif
        if (in1 != in2) {
            if (in1 > 0 && in2 > 0) {
                if (in1 > in2) {
                    if (lenght > 0) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- +3\n");
#endif
                        return (3);
                    }
                    if (supset) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- -2\n");
#endif
                        return (-2);
                    }
                    subset = TRUE;
                }
                else {
                    if (lenght < 0) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- -3\n");
#endif
                        return (-3);
                    }
                    if (subset) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- +2\n");
#endif
                        return (2);
                    }
                    supset = TRUE;
                }
            }
            else if (in1 < 0 && in2 < 0) {
                if (in1 < in2) {
                    if (lenght > 0) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- +3\n");
#endif
                        return (3);
                    }
                    if (supset) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- -2\n");
#endif
                        return (-2);
                    }
                    subset = TRUE;
                }
                else {
                    if (lenght < 0) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- -3\n");
#endif
                        return (-3);
                    }
                    if (subset) {
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "           End of compare_cond <- +2\n");
#endif
                        return (2);
                    }
                    supset = TRUE;
                }
            }
            else if (in1 < 0) {
                if (lenght > 0) {
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "           End of compare_cond <- +4\n");
#endif
                    return (4);
                }
                if (lenght == 0) {
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "           End of compare_cond <- +2\n");
#endif
                    return (2);
                }
                in1 = *(++rwp);
                if (in1 >= in2 && *(++rwp) == *what) {
                    subset = TRUE;
                }
                else {
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "           End of compare_cond <- -3\n");
#endif
                    return (-3);
                }
            }
            else { /* in1>0 && in2<0 */
                if (lenght < 0) {
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "           End of compare_cond <- -4\n");
#endif
                    return (-4);
                }
                if (lenght == 0) {
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "           End of compare_cond <- -2\n");
#endif
                    return (-2);
                }
                in2 = *(++what);
                if (in2 > 0 && *(++what) == *rwp && in1 <= in2) {
                    supset = TRUE;
                }
                else {
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "           End of compare_cond <- +3\n");
#endif
                    return (3);
                }
            }
        }
        if ((in1 = *(++rwp)))
            rwp++;
        if ((in2 = *(++what)))
            what++;
    }
    if (!in2 && (subset || in1)) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "           End of compare_cond <- -1\n");
#endif
        return (-1);
    }
    if (!in1 && (supset || in2)) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "           End of compare_cond <- +1\n");
#endif
        return (1);
    }
    if (!in1 && !in2) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "           End of compare_cond <- 0\n");
#endif
        return (0);
    }
    if (lenght > 0) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "           End of compare_cond <- +4\n");
#endif
        return (4);
    }
#ifdef DEBUGcompute_disab
    fprintf(stderr, "           End of compare_cond <- -4\n");
#endif
    return (-4);
}


int search_insert(ROWP what, int from, int to) {
    int sub_test;
    int f2, mid;
#ifdef DEBUGcompute_disab
    fprintf(stderr, "         Start of search_insert from=%d, to=%d\n", from, to);
#endif
    while (to > from) {
        switch (compare_cond(what, from)) {
        case -4 :
        case -3 :
        case -2 : add_cond(what, from, from + 1);
#ifdef DEBUGcompute_disab
            fprintf(stderr, "         End of search_insert <- 1\n");
#endif
            return (1);
        case -1 : /* subset */
            subst_cond(what, from);
#ifdef DEBUGcompute_disab
            fprintf(stderr, "         End of search_insert <- 0\n");
#endif
            return (0);
        case  0 :
        case  1 :
#ifdef DEBUGcompute_disab
            fprintf(stderr, "         End of search_insert <- 0\n");
#endif
            return (0);
        case  2 :
            sub_test = f2 = from + 1;
            while ((to - f2) > 0) {
                mid = (int)((to + f2) >> 1);
                switch (compare_cond(what, mid)) {
                case  2 :
                    from = mid;
                    if (sub_test < (f2 = mid + 1))
                        sub_test = f2;
                    break;
                case  1 :
                case  0 :
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "         End of search_insert <- 0\n");
#endif
                    return (0);
                case -2 :
                    to = mid - 1;
                    if (sub_test < mid + 1)
                        sub_test = mid + 1;
                    break;
                case -3 :
                case -4 :
                    to = mid - 1;
                    break;
                default :
                    fprintf(stderr, "ERROR: search_insert inconsistency!\n");
                    exit(1);
                }
            }
            if (to > from) {
                switch (compare_cond(what, to)) {
                case  2 :
                    if (sub_test < to + 1)
                        sub_test = to + 1;
                    add_cond(what, to + 1, sub_test);
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "         End of search_insert <- 1\n");
#endif
                    return (1);
                case  1 :
                case  0 :
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "         End of search_insert <- 0\n");
#endif
                    return (0);
                case -2 :
                case -3 :
                case -4 :
                    if (sub_test < to + 1)
                        sub_test = to + 1;
                    add_cond(what, to, sub_test);
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "         End of search_insert <- 1\n");
#endif
                    return (1);
                default :
                    fprintf(stderr, "ERROR: search_insert inconsistency!\n");
                    exit(1);
                }
            }
            add_cond(what, from + 1, sub_test);
#ifdef DEBUGcompute_disab
            fprintf(stderr, "         End of search_insert <- 1\n");
#endif
            return (1);
        case  3 :
        case  4 :
            from++;
            break;
        }
    }

    if (to == from) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "           search_insert: to==from\n");
#endif
        switch (compare_cond(what, to)) {
        case -4 :
        case -3 :
        case -2 : add_cond(what, from, to + 1);
#ifdef DEBUGcompute_disab
            fprintf(stderr, "         End of search_insert <- 1\n");
#endif
            return (1);
        case -1 : /* subset */
            subst_cond(what, to);
        case  0 :
        case  1 :
#ifdef DEBUGcompute_disab
            fprintf(stderr, "         End of search_insert <- 0\n");
#endif
            return (0);
        case  2 :
        case  3 :
        case  4 : add_cond(what, to + 1, to + 2);
#ifdef DEBUGcompute_disab
            fprintf(stderr, "         End of search_insert <- 1\n");
#endif
            return (1);

        }
    }
    add_cond(what, from, from + 1);
#ifdef DEBUGcompute_disab
    fprintf(stderr, "         End of search_insert <- 1 (first entry)\n");
#endif
    return (1);
}


void produce_dis(ROWP pp) {
    ROWP qq = disl2 + 1;
    ROWP q2;
    unsigned count = 0;
    int add;

#ifdef DEBUGcompute_disab
    fprintf(stderr, "      Start of produce_dis\n");
#endif
    while (pp > disl) {
        unsigned pla = *--pp;
        int ine = *--pp;

        for (add = TRUE, q2 = disl2 + 1 ; q2 < qq ;) {
            int in2 = *(q2++);
            unsigned pl2 = *(q2++);
            if (pl2 == pla) {
                if (ine > 0 && in2 > 0) {
                    if (ine > in2)
                        *(q2 - 2) = ine;
                    add = FALSE;
                    pp = disl;
                    q2 = qq;
                }
                else if (ine < 0 && in2 < 0) {
                    if (ine > in2)
                        *(q2 - 2) = ine;
                    add = FALSE;
                    pp = disl;
                    q2 = qq;
                }
            }
        }
        if (add) {
#ifdef DEBUGcompute_disab
            fprintf(stderr, "          adding p%d coef=%d\n", pla, ine);
#endif
            count++;
            *(qq++) = ine;
            *(qq++) = pla;
        }
    }
    if ((*disl2 = count) > 1) {
#ifdef DEBUGcompute_disab
        fprintf(stderr, "          calling qsort (count=%d)\n", count);
#endif
        qsort(disl2 + 1, count, 2 * ROWT_SIZE, (int (*)(const void *, const void *)) a_cmp);
    }
    *qq = 0;
    cur_cond += search_insert(disl2, from_cond, cur_cond - 1);
#ifdef DEBUGcompute_disab
    fprintf(stderr, "      End of produce_dis, cur_cond=%d\n", cur_cond);
#endif
}


void rec_dis(Trans_p ctp,
             unsigned n, unsigned j,
             ROWP pp,
             ROWP aux_in,
             int flag) {
    unsigned place;
    int ineq;
    ROWP iap, hap;
    ROWP qq;
    ROWP aux_out;

#ifdef DEBUGCALL
    fprintf(stderr, "    Start of rec_dis n=%d j=%d\n", n, j);
#endif
    if (! *(aux_in + n)) {
        if (j--) {
            int k = t_ord[++n];
            rec_dis(t_array + k, n, j, pp, aux_in, flag);
        }
        else if (flag)
            produce_dis(pp);
        *pp = 0;
#ifdef DEBUGCALL
        fprintf(stderr, "    End of rec_dis (already disabled)\n");
#endif
        return;
    }
#ifdef DEBUGcompute_disab
    fprintf(stderr, "      rec_dis: still not disabled\n");
#endif
    iap = (ctp->inp_arcs) + 1;
    hap = (ctp->inh_arcs) + 1;
    do {
        if ((ineq = *iap)) {
            place = *(iap + 1);
            iap += 2;
        }
        else if ((ineq = - (*hap))) {
            place = *(hap + 1);
            hap += 2;
        }
        else {
            *pp = 0;
#ifdef DEBUGCALL
            fprintf(stderr, "    End of rec_dis (no more arcs)\n");
#endif
            return;
        }
#ifdef DEBUGcompute_disab
        fprintf(stderr, "      rec_dis: arc p%d w=%d\n", place, ineq);
#endif
        *(qq = pp) = 0;
        aux_out = aux_in;
        if (test_disabl(ineq, place, pp)) {
            ROWP ttt;
            int ine2;
#ifdef DEBUGcompute_disab
            fprintf(stderr, "      rec_dis: adding place disabling\n");
#endif
            if ((*(qq++) = ineq) < 0)
                ttt = *(p_inh + (place - 1));
            else
                ttt = *(p_out + (place - 1));
            *(qq++) = place;
            if (j) {
#ifdef DEBUGcompute_disab
                fprintf(stderr, "      rec_dis: testing place\n");
#endif
                for (ine2 = *(ttt++) ; ine2 ; ine2 = *(ttt++)) {
                    unsigned ntt = *(ttt++);

                    if ((ineq > 0 && ine2 >= ineq) ||
                            (ineq < 0 && -ine2 <= ineq)) {
                        if (aux_out == aux_in) {
                            ROWP r1, r2;
                            int ii;
#ifdef DEBUGcompute_disab
                            fprintf(stderr, "      rec_dis: allocating aux_out\n");
#endif
                            r1 = aux_out = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
                            for (r2 = aux_in + n, r1 += n, ii = j + 1 ; ii-- ;
                                    * (r1++) = *(r2++))
#ifdef DEBUGcompute_disab
                                fprintf(stderr, "        copy loop ii=%d\n", ii);
#endif
                            ;
                        }
                        ntt = t_inv[ntt - 1];
#ifdef DEBUGcompute_disab
                        fprintf(stderr, "       ntt=%d\n", ntt);
#endif
                        *(aux_out + ntt) = FALSE;
                    }
                }
            }
        }
        *qq = 0;
        if (j) {
            int k = t_ord[n + 1];
#ifdef DEBUGcompute_disab
            fprintf(stderr, "      rec_dis: recursive call k=%d\n", k);
#endif
            rec_dis(t_array + k, n + 1, j - 1, qq, aux_out, TRUE);
            if (aux_out != aux_in) {
                free(aux_out);
                aux_out = aux_in;
            }
        }
        else
            produce_dis(qq);
        *pp = 0;
    }
    while (TRUE);
}


static int reachability_flag = FALSE;

void compute_disab() {
    unsigned i;
    unsigned nn = 0;
    unsigned last_ecs;
    ROWP aux_out;

#ifdef DEBUGCALL
    fprintf(stderr, "  Start of compute_disab\n");
#endif
    disl = (ROWP)ecalloc(trans_num + trans_num + 1, ROWT_SIZE);
    disl2 = (ROWP)ecalloc(trans_num + trans_num + 2, ROWT_SIZE);
    conds = (ARCM)ecalloc(count_conds, ROWP_SIZE);
    aux_out = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
    while (nn++ < trans_num) {
        Trans_p tp = t_array + (i = t_ord[nn]);
        Trans_p tp2;
        unsigned count = 0;
        unsigned j = nn;
        ROWP r1;

        last_ecs = tp->ecs;
#ifdef DEBUGcompute_disab
        fprintf(stderr, "    compute_disab: ecs=%d, nn=%d, converted to %d\n",
                last_ecs, nn, i);
#endif
        while (j++ < trans_num) {
            tp2 = t_array + (i = t_ord[j]);
            if (last_ecs == tp2->ecs) {
                count++;
                if (t_cmp(t_ord + j, t_ord + (j - 1)) == 0) {
                    Trans_p tp3 = t_array + (i = t_ord[j - 1]);
                    /* Extended Free-choice Conflict */
#ifdef DEBUGcompute_disab
                    fprintf(stderr, "      compute_disab: t%d EFC t%d, (converted to %d and %d\n",
                            j, j - 1, t_ord[j], i);
#endif
                    fprintf(ofp, "   %g x_%s = %g x_%s;\n",
                            tp2->ftime, tp2->tname,
                            tp3->ftime, tp3->tname);
                }
            }
            else
                break;
        }
        for (r1 = aux_out + nn, j = count + 1 ; j-- ; * (r1++) = TRUE);
        if (tp->pri) {
            rec_dis(tp, nn, count, disl, aux_out, FALSE);
            print_conds(tp, count + 1);
        }
        /*
        	else
        	    print_timed( tp, count+1 );
        */
        from_cond = cur_cond;
        nn += count;
    }
    free(aux_out);
    if (! reachability_flag)
        produce_mark(0, 1);
#ifdef DEBUGCALL
    fprintf(stderr, "  End of compute_disab\n");
#endif
}

static unsigned from_mark = 1;

void print_conds(Trans_p tp, unsigned ecs_card) {
    ARCM ccp = conds + from_cond;
    int ii = cur_cond - from_cond;
    int num = ii;
    ROWP rwp;
    int coef;
    unsigned place;
    char *pn;

#ifdef DEBUGCALL
    fprintf(stderr, "    Start of print_conds from_cond=%d, cur_cond=%d\n",
            from_cond, cur_cond);
#endif
    if (num > 1) {
        produce_mark(from_mark, num);
        reachability_flag = TRUE;
    }
    while (ii--) {
        for (rwp = (*(ccp++)) + 1 ; (coef = *(rwp++)) ;) {
            place = *(rwp++);
            pn = *(place_nam + (place - 1));
            if (num > 1)
                fprintf(ofp, "     M%d_%s ", from_mark, pn);
            else
                fprintf(ofp, "     M_%s ", pn);
            if (coef > 0) {
                if (--coef)
                    if (num > 1) {
                        if (coef > 1)
                            fprintf(ofp, "<= %d a_%d;\n", coef, from_mark);
                        else
                            fprintf(ofp, "<= a_%d;\n", from_mark);
                    }
                    else
                        fprintf(ofp, "<= %d;\n", coef);
                else
                    fprintf(ofp, "= 0;\n");
            }
            else {
                coef = -coef;
                if (num > 1) {
                    if (coef > 1)
                        fprintf(ofp, ">= %d a_%d;\n", coef, from_mark);
                    else
                        fprintf(ofp, ">= a_%d;\n", from_mark);
                }
                else
                    fprintf(ofp, ">= %d;\n", coef);
            }
        }
        if (num > 1)
            from_mark++;
    }
    fprintf(ofp, "\n");
#ifdef DEBUGCALL
    fprintf(stderr, "    End of print_conds\n");
#endif
}


void produce_mark(unsigned from, unsigned  num) {
    int nmn = 0;
    char **place;
    int npl;
    ARCM pia, poa;
    ROWP m0p;
    ROWP arcp;
    int nmark;

#ifdef DEBUGCALL
    fprintf(stderr, "      Start of produce_mark from=%d, num=%d\n", from, num);
#endif
    for (place = place_nam, npl = place_num, pia = p_inp, poa = p_out, m0p = M0 ;
            npl-- ; place++, pia++, poa++, m0p++) {
        for (nmark = num ; nmark-- ;) {
            int plus = FALSE;
            if (num > 1) {
                nmn = from + num - nmark - 1;
                fprintf(ofp, "    M%d_%s =", nmn, *place);
            }
            else
                fprintf(ofp, "    M_%s =", *place);
            if (*m0p > 0) {
                plus = TRUE;
                if (*m0p > 1 || num == 1)
                    fprintf(ofp, " %d", *m0p);
                if (num > 1)
                    fprintf(ofp, " a_%d", nmn);
            }
            for (arcp = *pia ; *arcp ; arcp++) {
                int nt;
                if (plus)
                    fprintf(ofp, " +");
                else {
                    fprintf(ofp, " ");
                    plus = TRUE;
                }
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                nt = *++arcp;
                if (num > 1)
                    fprintf(ofp, "s%d_%s", nmn, t_array[nt - 1].tname);
                else
                    fprintf(ofp, "s_%s", t_array[nt - 1].tname);
            }
            for (arcp = *poa ; *arcp ; arcp++) {
                int nt;
                fprintf(ofp, " -");
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                nt = *++arcp;
                if (num > 1)
                    fprintf(ofp, "s%d_%s", nmn, t_array[nt - 1].tname);
                else
                    fprintf(ofp, "s_%s", t_array[nt - 1].tname);
            }
            fprintf(ofp, ";\n");
        }
        if (num > 1) {
            fprintf(ofp, "  M_%s = M%d_%s", *place, from, *place);
            for (nmark = num ; --nmark ;) {
                nmn = from + num - nmark;
                fprintf(ofp, " +M%d_%s", nmn, *place);
            }
            fprintf(ofp, ";\n");
        }
    }
    if (num > 1) {
        fprintf(ofp, "a_%d", from);
        for (nmark = num ; --nmark ;) {
            nmn = from + num - nmark;
            fprintf(ofp, " +a_%d", nmn);
        }
        fprintf(ofp, " = 1;\n");
    }
#ifdef DEBUGCALL
    fprintf(stderr, "      End of produce_mark\n");
#endif
}


void produce_flow() {
    Trans_p t_p;
    int npl;
    ARCM pia, poa;
    ROWP arcp;
    char **place;

#ifdef DEBUGCALL
    fprintf(stderr, "  Start of produce_flow\n");
#endif
    for (place = place_nam, npl = place_num, pia = p_inp, poa = p_out ;
            npl-- ; pia++, poa++, place++) {
        if (**poa > 0) {
            int nt;
            if (*(arcp = *pia) > 0) {
                char comp[3];
                int ii;
                int timed = FALSE;
                int jj;
                int mintw = -1;

                arcp = *poa;
                strcpy(comp, ">=");
                while ((ii = *(arcp++)) > 0) {
                    t_p = t_array + ((jj = *(arcp++)) - 1);
                    if (!(t_p->pri)) {
                        timed = TRUE;
                        if (t_p->no_ih == 1) {
                            if (--ii) {
                                if (mintw < 0 || mintw > ii)
                                    mintw = ii;
                            }
                            else {
                                mintw = 0;
                                strcpy(comp, "=");
                            }
                        }
                    }
                    /* REDUNDANT ?????????
                        else {
                    	if ( t_p->no_ih == 1 ) {
                    	    if ( vanishing < 0 )
                    		vanishing = ii-1;
                    	    else {
                    		if ( (--ii) < vanishing )
                    		    vanishing = ii;
                    	      }
                    	  }
                          }
                    */
                }
                /* REDUNDANT ?????????
                if ( vanishing == 0 ) {
                    fprintf(ofp,"    M_%s = 0;\n", *place);
                  }
                else if ( vanishing > 0 )
                	fprintf(ofp,"    M_%s <= %d;\n", *place, vanishing);
                */
                if (timed) {
                    /* THE FOLLOWING IS OK FOR PERSISTENT OR AGE MEMORY
                       OR PRESELECTION TIMED TRANSITIONS; IT DOES NOT APPLY
                       TO CONFLICTING TRANSITION WITH RACE AND ENABLING
                       MEMORY POLICY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  */
                    if (mintw > 0) {
                        fprintf(ofp, "    M_%s -%d <= ", *place, mintw);
                        timed = FALSE;
                        for (arcp = *poa ; (ii = *(arcp++)) ;) {
                            t_p = t_array + ((jj = *(arcp++)) - 1);
                            if (!(t_p->pri)) {
                                if (timed)
                                    fprintf(ofp, " +");
                                else
                                    timed = TRUE;
                                fprintf(ofp, " %g x_%s",
                                        ii * (t_p->ftime), t_p->tname);
                            }
                        }
                        fprintf(ofp,
                                "; /* ONLY FOR Persistent OR Age Memory OR Preselection TRANSITIONS */\n");
                    }
                    fprintf(ofp, "    M_%s %s ", *place, comp);
                    for (arcp = *poa ; (ii = *(arcp++)) ;) {
                        t_p = t_array + ((jj = *(arcp++)) - 1);
                        if (!(t_p->pri)) {
                            if (timed)
                                timed = FALSE;
                            else
                                fprintf(ofp, " +");
                            fprintf(ofp, " %g x_%s",
                                    ii * (t_p->ftime), t_p->tname);
                        }
                    }
                    fprintf(ofp,
                            "; /* ONLY FOR Persistent OR Age Memory OR Preselection TRANSITIONS */\n");
                }
                arcp = *pia;
                fprintf(ofp, "   ");
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                t_p = t_array + ((nt = *++arcp) - 1);
                fprintf(ofp, "x_%s", t_p->tname);
                for (arcp++ ; *arcp ; arcp++) {
                    fprintf(ofp, " +");
                    if (*arcp > 1)
                        fprintf(ofp, "%d ", *arcp);
                    t_p = t_array + ((nt = *++arcp) - 1);
                    fprintf(ofp, "x_%s", t_p->tname);
                }
                fprintf(ofp, " >= ");
            }
            else
                fprintf(ofp, "   0 = ");
            if (*(arcp = *poa) > 1)
                fprintf(ofp, "%d ", *arcp);
            t_p = t_array + ((nt = *++arcp) - 1);
            fprintf(ofp, "x_%s", t_p->tname);
            for (arcp++ ; *arcp ; arcp++) {
                fprintf(ofp, " +");
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                t_p = t_array + ((nt = *++arcp) - 1);
                fprintf(ofp, "x_%s", t_p->tname);
            }
            fprintf(ofp, ";\n");
        }
    }
    fprintf(ofp, "\n");
#ifdef DEBUGCALL
    fprintf(stderr, "  End of produce_flow\n");
#endif
}


int main(int argc, char **argv) {


    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[100];

#ifdef DEBUGCALL
    fprintf(stderr, "Start\n");
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
    sprintf(filename, "%s.lp_disab", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    load_net();
    (void) fclose(nfp);
    compute_disab();
    produce_flow();
    (void) fclose(ofp);
#ifdef DEBUGCALL
    fprintf(stderr, "End\n");
#endif
    return 0;
}

