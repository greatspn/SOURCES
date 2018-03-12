/*
 *  program: struct2.c
 *  date: July 29, 1993
 *  programmer: Giovanni Chiola
 *  address:	Dipartimento di Informatica, Universita' di Torino
 *		corso Svizzera 185, 10149 Torino, Italy.
 *
 *  purpose: structural analysis of a GSPN; produces the following files:
 *           "netname.cc"  ->  causal connection set of each transition;
 *           "netname.sc"  ->  structural conflict set of each transition;
 *           "netname.esc" ->  transitive closure of "sc."
 *  notes:
 *    1) the net description must be in GreatSPN 1.6 format
 */


/*
#define DEBUG
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *nfp, * ifp, * ofp;

#include "const.h"

typedef unsigned short ROWT;
#define ROWT_SIZE (unsigned)(sizeof(ROWT))

typedef ROWT *ROWP;
#define ROWP_SIZE (unsigned)(sizeof(ROWP))

typedef ROWP *ARCM;
#define ARCM_SIZE (unsigned)(sizeof(ARCM))

ARCM t_inp, t_out, t_inh;
ARCM p_inp, p_out, p_inh;
ARCM t_in_count;
ARCM sc_rel;
ROWP t_esc;
ROWP inter;

struct T_list {
    ROWT trans;
    struct T_list *next;
};

typedef struct T_list *T_L_P;

#define T_l_SIZE (unsigned)(sizeof(struct T_list))
#define T_P_SIZE (unsigned)(sizeof(T_L_P))


int	place_num, trans_num;

T_L_P *esc_listp;

char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *	calloc();
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}

load_net() {
    extern              getname();

    ARCM		pp1, pp2, pp3;
    ROWP		rwp, t_pp;
    T_L_P 		*esc_lp, esc_p;
    struct matrix 	*m_p;
    struct matrix 	*prev_m_p;
    char		linebuf[LINEMAX];
    float		ftemp, x, y;
    int                 group_num, i, j, knd, noar, mark, nomp, norp, orien;
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
    fscanf(nfp, "%d %d %d %d\n", &trans_num, &group_num, &i, &j);

    pp1 = sc_rel = (ARCM)ecalloc(trans_num, ROWP_SIZE);
    for (i = trans_num ; i-- ; pp1++) {
        *pp1 = (ROWP)ecalloc(trans_num + 1, ROWT_SIZE);
        **pp1 = 0;
    }
    esc_lp = sub_plist = (T_L_P *)ecalloc(trans_num, T_P_SIZE);
    for (i = trans_num ; i-- ; esc_lp++)
        *esc_lp = NULL;
    esc_lp = sub_pelems = (T_L_P *)ecalloc(place_num, T_P_SIZE);
    esc_p = (T_L_P)ecalloc(place_num, T_l_SIZE);
    for (i = place_num ; i-- ; esc_lp++, esc_p++) {
        *esc_lp = esc_p;
        esc_p->trans = (place_num - i);
        esc_p->next = NULL;
    }
    rwp = t_esc = (ROWP)ecalloc(trans_num, ROWT_SIZE);
    esc_lp = esc_listp = (T_L_P *)ecalloc(trans_num, T_P_SIZE);
    esc_p = (T_L_P)ecalloc(trans_num, T_l_SIZE);
    for (i = trans_num ; i-- ; esc_lp++, esc_p++, rwp++) {
        *esc_lp = esc_p;
        esc_p->trans = (trans_num - i);
        esc_p->next = NULL;
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
    /* skip marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        while (i < nomp) {
            getname(linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
            printf("    %d %s=%d\n", i, linebuf, mark);
#endif
        }
    }

    /* skip places */
    i = 0;
    while (i++ < place_num) {
        getname(linebuf);
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
        printf("    place %s %d\n", linebuf, mark);
#endif
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
        fscanf(nfp, "%f %d %d %d ", &ftemp, &mark, &knd, &noar);
        while (getc(nfp) != '\n');
        if (mark < 0) {
            int ii;
            ii = mark = -mark;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        getarcs(t_inp, p_out, noar, trans_num - i);
        fscanf(nfp, "%d\n", &noar);
        getarcs(t_out, p_inp, noar, trans_num - i);
        fscanf(nfp, "%d\n", &noar);
        getarcs(t_inh, p_inh, noar, trans_num - i);
    }
#ifdef DEBUG
    printf("  End of load_net\n");
#endif
}

getname(name_pr)
char 	*name_pr;
{
#define	BLANK ' '
#define	EOLN  '\0'
    short		i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != NULL &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}

int t_cmp(p1, p2)
ROWP p1, p2;
{
    register int m1 = *(++p1), m2 = *(++p2);

    if (m1 == m2)
        return (0);
    return ((m2 > m1) ? -1 : 1);
}

getarcs(t_arc, p_arc, noar, notr)
ARCM	t_arc, p_arc;
int	noar, notr;
{
    ROWP	tp, pp;
    int		i, j, pl, mlt, ip;
    float	x, y;

    *(t_arc + (notr - 1)) = tp = (ROWP)ecalloc(noar + noar + 1, ROWT_SIZE);
    for (i = noar ; i-- ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while (getc(nfp) != '\n');
        for (j = 1; j++ <= ip; fscanf(nfp, "%f %f\n", &x, &y));
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
    qsort(*(t_arc + (notr - 1)), noar, 2 * ROWT_SIZE, t_cmp);
}


ROWP diffbag;

bag_diff(bag1, bag2, diff)
ROWP bag1, bag2, diff;
{
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

compute_cc() {
    int np, nt;
    ARCM ti_p, to_p, th_p;
    ARCM pi_p, po_p, ph_p;
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

add_trans(bagp, thead1, nosub)
ROWP bagp;
T_L_P *thead1;
unsigned nosub;
{
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

compute_sub() {
    unsigned nt = 1, ii, nsub = 1;
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
            T_L_P tppp = *thead1;
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

compute_place_sub() {
    unsigned nt = 1, ii, nsub = 1;
    ROWP tsp = t_sub, tpp = t_pri;;

#ifdef DEBUG
    printf("  Start of compute_place_sub\n");
#endif
    for (nt = 1, nsub = NSUB ; nsub-- ;) {
        unsigned offs = nt - 1;
        unsigned nnn = *(t_sub + offs);
        T_L_P phead;
        T_L_P thead = *(sub_tlist + (offs));
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

bag_inters(bag1, bag2, inters)
ROWP bag1, bag2, inters;
{
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


compute_me() {
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
            if (struct_me(it1, it2)) {
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

merge_ecs(t1_ecs, t2_ecs)
unsigned t1_ecs, t2_ecs;
{
    unsigned t1offs = t1_ecs - 1, t2offs = t2_ecs - 1;

#ifdef DEBUG
    printf("      Start of merge_ecs %d %d\n", t1_ecs, t2_ecs);
#endif
    if (t1_ecs < t2_ecs) {
        T_L_P *phead1 = (ecs_listp + (t1offs));
        T_L_P *phead2 = (ecs_listp + (t2offs));
        T_L_P npp = *phead2;
        T_L_P ppp;
        for (; npp != NULL ; ppp = npp, npp = npp->next)
            * (t_ecs + (npp->trans - 1)) = t1_ecs;
        ppp->next = *phead1;
        *phead1 = *phead2; *phead2 = NULL;
    }
    else {
        T_L_P *phead1 = (ecs_listp + (t1offs));
        T_L_P *phead2 = (ecs_listp + (t2offs));
        T_L_P npp =  *phead1;
        T_L_P ppp;
        for (; npp != NULL ; ppp = npp, npp = npp->next)
            * (t_ecs + (npp->trans - 1)) = t2_ecs;
        ppp->next = *phead2;
        *phead2 = *phead1; *phead1 = NULL;
    }
#ifdef DEBUG
    printf("      End of merge_ecs\n");
#endif
}

compute_sc() {
    int np, nt;
    ARCM ti_p, to_p, th_p;
    ARCM pi_p, po_p, ph_p;
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


bag_inhdiff(bag1, bag2, inhdiff)
ROWP bag1, bag2, inhdiff;
{
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


breadth_first(tki_p, tkh_p, pi, tkoffs, tmi_p, tmh_p, tmoffs, stack)
ROWP tki_p, tkh_p, tmi_p, tmh_p;
unsigned pi, tkoffs, tmoffs;
ARCM stack;
{
    unsigned ii;
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
            breadth_first(tki_p, tkh_p, pi, tkoffs,
                          *(t_inp + tloffs), *(t_inh + tloffs), tloffs,
                          stack + 1);
        }
    }
#ifdef DEBUG
    printf("   End of breadth_first\n");
#endif
}

compute_ecs() {
    int np, nt;
    ARCM ti_p, th_p;
    ARCM pi_p, po_p, ph_p;
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
                    breadth_first(*ti_p, *th_p, *t1_pp, t1offs,
                                  *(t_inp + t2offs), *(t_inh + t2offs), t2offs,
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

depth_first(tki_p, tkh_p, ntm, stack, sp)
ROWP tki_p, tkh_p;
ARCM stack;
int sp, ntm;
{
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
            depth_first(tki_p, tkh_p, ntm, stack + 1, sp + 1);
        }
#ifdef DEBUG
    printf("  End of depth_first sp=%d\n", sp);
#endif
}


main(argc, argv)
int argc;
char **argv;
{
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[100];

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
    load_net();
    (void) fclose(nfp);
    sprintf(filename, "%s.cc", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    compute_cc();
    sprintf(filename, "%s.me", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    compute_me();
    (void) fclose(ofp);
    sprintf(filename, "%s.sc", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    compute_sc();
    (void) fclose(ofp);
    sprintf(filename, "%s.ecs", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    compute_ecs();
    (void) fclose(ofp);
#ifdef DEBUG
    printf("End\n");
#endif
    return 0;
}

