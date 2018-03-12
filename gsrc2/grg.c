/*
 *  Torino, December 28, 1987
 *  program: grg.c
 *  purpose: RG construction of a GSPN2.
 *  programmer: Giovanni Chiola
 *  notes:
 */


/*
#define DEBUG
*/


#include <stdio.h>

FILE *rgfp, * tmfp, * vmfp;

#include "const.h"


struct Stack_el {
    unsigned char *cm;
    unsigned short *el;
    unsigned long *tm;
    unsigned long nm;
};

struct Stack_el stack[MAX_STACK];

/* DECODED MARKING */ extern unsigned char DP[];

/* CODING BYTES */ extern unsigned no_cod;

extern unsigned load_vrs;

#include "grg.h"

/* TRANSITION ARRAY */ extern struct T_descr trans[];

extern unsigned char M0[];

extern unsigned short first_trans[];

extern unsigned place_num, trans_num;

MARKP rootm;


/* MARKING DESCRIPTION RECORDS */
MARK_P *mrk_array[MAX_ARR];

unsigned long  lastan, toptan = 0;
unsigned long  lasvan, topvan = MAX_MARKS;

static unsigned tmrk_limit = 0;
static unsigned tmrk_top_p = 0;
static unsigned vmrk_limit = MAX_MARKS;
static unsigned vmrk_top_p = MAX_ARR;

static MARK_P   *tmrk_p = NULL;


MARK_P   *marking(num)
unsigned num;
{
    unsigned mlo, mup;

    if ((mup = --num / MRKSLOT) >= tmrk_top_p) {
        fprintf(stderr, "ERROR: trying to reference unallocated marking #%d\n",
                num + 1);
        exit(20);
    }
    mlo = num % MRKSLOT;
    return (mrk_array[mup] + mlo);
}


load_mark(Mp, m_p)
unsigned char *Mp;
MARK_P   *m_p;
{

#ifdef DEBUG
    printf("  Start of load_mark\n");
#endif
    loadm_f(Mp,  m_p);
#ifdef DEBUG
    printf("  End of load_mark\n");
#endif
}

unsigned new_enab(old_e, new_e, nt, markp, tpri)
unsigned short *old_e, *new_e;
int nt;
unsigned char *markp;
unsigned *tpri;
{
    int el, oel, mult;
    unsigned short *aep, * tep, * first = new_e;
    struct T_descr *tdp = trans + nt;

#ifdef DEBUG
    printf("   Start of new_enab\n");
    {
        int ii = no_cod; unsigned char *cp = markp;
        decode_mark(cp);
        printf("     cmark=");
        for (; ii-- ; cp++)
            printf(" %d,", (int)*cp);
        printf("\n  mark=");
        for (ii = place_num, cp = &(DP[1]) ; ii-- ; cp++)
            printf(" %d,", (int)*cp);
    }
    printf("\n");
#endif
    aep = tdp->add_l; tep = tdp->test_l;
    for (oel = *old_e ; oel ; oel = *(++old_e)) {
#ifdef DEBUG
        printf("     oel=%d, *aep=%d\n", oel, (int)*aep);
#endif
        while ((el = *aep) && el < oel) {
#ifdef DEBUG
            printf("      testing el=%d", el);
#endif
            aep++;
            while (*tep < el)
                tep++;
            if ((mult = (*((trans + el)->e_f))(markp))) {
#ifdef DEBUG
                printf(" yes, %d times\n", mult);
#endif
                *(new_e++) = el; *(new_e++) = mult;
            }
#ifdef DEBUG
            else
                printf(" no\n");
#endif
        }
        if (el == oel)
            aep++;
        while (*tep && *tep < oel)
            tep++;
#ifdef DEBUG
        printf("      testing oel=%d (*tep=%d)", oel, (int)*tep);
#endif
        if (*tep != oel && oel != nt) {
            *(new_e++) = oel; *(new_e++) = *(++old_e);
#ifdef DEBUG
            printf(" yes, %d times\n", (int)*old_e);
#endif
        }
        else if ((mult = (*((trans + oel)->e_f))(markp))) {
#ifdef DEBUG
            printf(" yes, %d times\n", mult);
#endif
            *(new_e++) = oel; *(new_e++) = mult; old_e++;
        }
        else {
            old_e++;
#ifdef DEBUG
            printf(" no\n");
#endif
        }
    }
    while ((el = *(aep++))) {
#ifdef DEBUG
        printf("      testing el=%d", el);
#endif
        if ((mult = (*((trans + el)->e_f))(markp))) {
#ifdef DEBUG
            printf(" yes\n", el);
#endif
            *(new_e++) = el; *(new_e++) = mult;
        }
#ifdef DEBUG
        else
            printf(" no\n", el);
#endif
    }
    *(new_e++) = 0;
    *tpri = (unsigned)((trans + (*(new_e - 3)))->pri);
#ifdef DEBUG
    printf("   End of new_enab\n");
#endif
    return ((unsigned)(new_e - first));
}


new_tanmrk(nm)
MARKP3 *nm;
{
    if (++toptan <= tmrk_limit)
        tmrk_p++;
    else {
        if (++tmrk_top_p >= vmrk_top_p) {
            fprintf(stderr, "Sorry, too many markings! Increase constant MAX_MARKS\n");
            exit(3);
        }
        tmrk_limit += MRKSLOT;
        if ((tmrk_p = (MARK_P *)calloc(MRKSLOT, sizeof(MARK_P))) == 0) {
            fprintf(stderr, "calloc: couldn't alloc enough memory!\n");
            exit(50);
        }
        mrk_array[tmrk_top_p - 1] = tmrk_p;
    }
    tmrk_p->en_list = NULL;
    nm->b0 = (unsigned char)(toptan & 0xFF);
    nm->b1 = (unsigned char)((toptan >> 8) & 0xFF);
    nm->b2 = (unsigned char)((toptan >> 16) & 0xFF);
}

new_vanmrk(nm)
MARKP3 *nm;
{
    if (--topvan <= vmrk_limit) {
        if (--vmrk_top_p <= tmrk_top_p) {
            fprintf(stderr, "Sorry, too many markings! Increase constant MAX_MARKS\n");
            exit(3);
        }
        vmrk_limit -= MRKSLOT;
    }
    nm->b0 = (unsigned char)(topvan & 0xFF);
    nm->b1 = (unsigned char)((topvan >> 8) & 0xFF);
    nm->b2 = (unsigned char)((topvan >> 16) & 0xFF);
}

store_enlist(elp , noel, mrkp)
unsigned short *elp;
unsigned noel;
MARK_P *mrkp;
{
    unsigned short *cp;

#ifdef DEBUG
    printf("  Start of store_enlist\n");
#endif
    if ((cp = (unsigned short *)calloc(noel, short_SIZE)) == 0) {
        fprintf(stderr, "calloc: couldn't alloc enough memory!\n");
        exit(50);
    }
    mrkp->en_list = cp;
    while (*elp)
        *(cp++) = *(elp++);
    *cp = (unsigned short)0;
#ifdef DEBUG
    printf("  End of store_enlist\n");
#endif
}

unsigned tst_ins(Mp, nt, old_e, new_e, tpri)
unsigned char *Mp;
unsigned nt;
unsigned short *old_e;
unsigned short *new_e;
unsigned *tpri;
{
    unsigned short ii;
    unsigned char mp;
    MARKP3 *mrkp, mrk;
    MARKP pnm;
    extern MARKP3 *tstins_f();

#ifdef DEBUG
    printf("  Start of tst_ins\n");
#endif
    mrkp = tstins_f(Mp, &pnm, &mp);
    mrk = * mrkp; *tpri = 0;
    if (ZERO3(mrk)) { /* NEW MARKING */
        if ((ii = new_enab(old_e, new_e, nt, Mp, tpri)) <= 1) {
            fprintf(stderr, "\nError: dead marking !\n");
            exit(4);
        }
        if (*tpri) { /* VANISHING */
#ifdef DEBUG
            printf("    vanishing\n");
#endif
            new_vanmrk(&mrk);
        }
        else { /* TANGIBLE */
#ifdef DEBUG
            printf("    tangible\n");
#endif
            new_tanmrk(&mrk);
            tmrk_p->link = pnm;
            tmrk_p->pval = mp;
            store_enlist(new_e , ii, tmrk_p);
        }
        *mrkp = mrk;
    }
#ifdef DEBUG
    printf("  End of tst_ins\n");
#endif
    return (INT3(mrk));
}

init() {
    unsigned char mp, *MM0 = M0;
    MARKP3 *mrkp, mrk;
    MARKP pnm;
    extern MARKP3 *tstins_f();
    unsigned short *elp1;
    unsigned nt;
    struct T_descr *tdp = trans + 1;
    unsigned short ii;
    MARKP mrk1;
    struct Stack_el *sp;

#ifdef DEBUG
    printf("  Start of init\n");
#endif
    for (ii = MAX_STACK, sp = stack ; ii -- ; sp++) {
        sp->cm = (unsigned char *)calloc(no_cod, 1);
        sp->el = (unsigned short *)calloc(trans_num + trans_num + 1, short_SIZE);
        sp->tm = (unsigned long *)calloc(trans_num, long_SIZE);
    }
#ifdef DEBUG
    printf("  Start of new_m1\n");
#endif
    new_m1(MM0, mrk1, &rootm);
#ifdef DEBUG
    printf("  End of new_m1\n");
    {
        int ii; unsigned char *mp = MM0;
        for (ii = no_cod; ii-- ; mp++)
            printf(" %d,", (int)*mp);
        printf("\n");
    }
#endif
    mrkp = tstins_f(MM0, &pnm, &mp);
#ifdef DEBUG
    printf("  End of tstins_f\n");
#endif

    for (elp1 = stack->el, nt = trans_num ; nt-- ; tdp++) {
        unsigned mult;
#ifdef DEBUG
        printf("    Start of e_f%d\n", trans_num - nt);
#endif
        if ((mult = (*(tdp->e_f))(M0))) {
            *(elp1++) = (unsigned short)(trans_num - nt);
            *(elp1++) = (unsigned short)mult;
#ifdef DEBUG
            printf("    End of e_f%d, %d-enabled\n", trans_num - nt, mult);
#endif
        }
#ifdef DEBUG
        else
            printf("    End of e_f%d, disabled\n", trans_num - nt);
#endif
    }
    *(elp1++) = (unsigned short)0;
    if ((trans + (*(elp1 - 3)))->pri) { /* VANISHING INITIAL MARKING */
        fprintf(stderr, "Sorry, initial marking is vanishing !\n");
        exit(2);
    }
    new_tanmrk(&mrk);
    tmrk_p->link = pnm; tmrk_p->pval = mp;
    *mrkp = mrk;
    store_enlist(stack->el , (elp1 - stack->el), tmrk_p);
    free(stack->el); stack->el = NULL;
#ifdef DEBUG
    printf("  End of init\n");
#endif
}


extern store_compact(/* ll, fp */);
/*
  unsigned long ll;
  FILE * fp;
*/


save_rgr(tlp, nel, stckp)
unsigned short *tlp;
unsigned long nel;
struct Stack_el *stckp;
{
    unsigned long *tmp = stckp->tm;
    unsigned long nn = nel << 1;

#ifdef DEBUG
    printf("      Start of save_rgr stacktop=%d\n", (int)(stckp - stack));
#endif
    store_compact((stckp->nm), rgfp);
    nn = ((stckp->nm >= topvan) ? (nn | 0x1) : (nn & ~ 0x1));
    store_compact(nn, rgfp);
#ifdef DEBUG
    printf("        stckp->nm=%d, nn=%d\n", (int)(stckp->nm), (int)nn);
#endif
    for (nel++ ; --nel ; ++tmp) {
        register unsigned long lt = *tlp;
        unsigned long ll = *++tlp;
#ifdef DEBUG
        printf("          lt=%d,", (int)(lt));
#endif
        store_compact(lt, rgfp);
        ++tlp;
        if (trans[lt].en_dep) {
            store_compact(ll, rgfp);
#ifdef DEBUG
            printf(" ll=%d,", (int)(ll));
#endif
        }
        store_compact(*(tmp), rgfp);
#ifdef DEBUG
        printf(" *tmp=%d\n", (int)(*tmp));
#endif
    }
#ifdef DEBUG
    printf("      End of save_rgr stacktop=%d\n", (int)(stckp - stack));
#endif
}

fire_trans(en_tr, stckp, to_mp)
unsigned short en_tr;
struct Stack_el *stckp;
unsigned long *to_mp;
{
    unsigned char *oldmrk = stckp->cm;
    unsigned short *old_e = stckp->el;
    unsigned char *newmrk;
    unsigned short *new_e;
    unsigned long nm;
    unsigned pri, ii;
#ifdef DEBUG
    printf("  Start of fire_trans %d\n", (int)en_tr);
    printf("    lasvan=%d, topvan=%d, lastan=%d, toptan=%d\n",
           lasvan, topvan, lastan, toptan);
#endif
    if (((unsigned)(stckp - stack) + 1) >= MAX_STACK) {
        fprintf(stderr, "Sorry, stack overflow. Increase constant MAX_STACK\n");
        exit(5);
    }
    newmrk = (stckp + 1)->cm;
    new_e = (stckp + 1)->el;
    (*((trans + en_tr)->f_f))(oldmrk, newmrk);
#ifdef DEBUG
    {
        int ii = no_cod; unsigned char *cp = newmrk;
        decode_mark(cp);
        printf("     cmark=");
        for (; ii-- ; cp++)
            printf(" %d,", (int)*cp);
        printf("\n  mark=");
        for (ii = place_num, cp = &(DP[1]) ; ii-- ; cp++)
            printf(" %d,", (int)*cp);
    }
    printf("\n");
#endif
    if ((nm = tst_ins(newmrk, en_tr, old_e, new_e, &pri)) > toptan) /* VANISHING */ {
        if (pri) {   /* NEW VANISHING: RECURSIVE FIRING USING THE STACK */
            unsigned long from = first_trans[pri], num;
            unsigned char *cc = newmrk;
            unsigned short *ss;
            struct Stack_el *sp = stckp + 1;
            unsigned long *to_m_l = sp->tm;
#ifdef DEBUG
            printf("    new vanishing %d\n", (int)nm);
#endif
            topvan = nm;
            sp->nm = nm;
            while (*new_e && *new_e < from)
                new_e += 2;
            ss = new_e;  num = 0;
            while (*new_e) {
                fire_trans(*new_e, sp, to_m_l++);
                new_e += 2;  num++;
            }
            if (load_vrs)    /* WRITE NEW VANISHING CODING */
#ifdef DEBUG
            {
                printf("      writing 'vrs' file\n");
#endif
                for (ii = no_cod; ii-- ; putc(*cc, vmfp), cc++);
#ifdef DEBUG
                printf("      saving rgr\n");
            }
#endif
            save_rgr(ss, num, sp);
        }
        else { /*  OLD VANISHING  */
            if (nm < lasvan) {   /* CHECK LOOP IN THE STACK */
                struct Stack_el *sp = stack + 1;
                while (sp < stckp)
                    if ((sp++)->nm == nm) {
                        fprintf(stderr, "Sorry, vanishing marking loop !\n");
                        exit(5);
                    }
            }
#ifdef DEBUG
            printf("    old vanishing %d\n", (int)nm);
#endif
        }
    }
#ifdef DEBUG
    else { /* TANGIBLE */
        printf("    tangible %d\n", (int)nm);
    }
#endif
    *to_mp = nm;
#ifdef DEBUG
    printf("    lasvan=%d, topvan=%d, lastan=%d, toptan=%d\n",
           lasvan, topvan, lastan, toptan);
    printf("  End of fire_trans\n");
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
        printf("ERROR: no net name !\n");
        exit(1);
    }
    init();
    sprintf(filename, "%s.crgr", argv[1]);
    if ((rgfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s.ctrs", argv[1]);
    if ((tmfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    if (load_vrs) {
        sprintf(filename, "%s.cvrs", argv[1]);
        if ((vmfp = fopen(filename, "w")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'w');
            exit(1);
        }
    }
    for (lastan = 0, toptan = 1 ; lastan++ < toptan ;) {
        MARK_P   *m_p = marking(lastan);
        unsigned short *en_tr = m_p->en_list, * ss;
        unsigned char *cc = stack->cm;
        unsigned long *to_mp = stack->tm;
        unsigned num;
        stack->el = en_tr; stack->nm = lastan;
        load_mark(stack->cm, m_p);
        /* WRITE TANGIBLE CODING */
        for (num = no_cod; num-- ; putc(*cc, tmfp), cc++);
#ifdef DEBUG
        {
            int ii = no_cod; unsigned char *cp = stack->cm;
            decode_mark(cp);
            printf("     cmark=");
            for (; ii-- ; cp++)
                printf(" %d,", (int)*cp);
            printf("\n  mark=");
            for (ii = place_num, cp = &(DP[1]) ; ii-- ; cp++)
                printf(" %d,", (int)*cp);
        }
        printf("\n");
#endif
        ss = en_tr;  num = 0;
        if (*ss == 0) {
            fprintf(stderr, "ERROR: dead marking\n");
            {
                int ii = no_cod; unsigned char *cp = stack->cm;
                decode_mark(cp);
                fprintf(stderr, "     cmark=");
                for (; ii-- ; cp++)
                    fprintf(stderr, " %d,", (int)*cp);
                fprintf(stderr, "\n  mark=");
                for (ii = place_num, cp = &(DP[1]) ; ii-- ; cp++)
                    fprintf(stderr, " %d,", (int)*cp);
            }
            fprintf(stderr, "\n");
            exit(10);
        }
        while (*en_tr) {
            lasvan = topvan;
            fire_trans(*en_tr, stack, to_mp++);
            en_tr += 2;  num++;
        }
        /* WRITE RG */
        save_rgr(ss, num, stack);
        free(stack->el);
    }
    toptan = 0;
    store_compact(toptan, rgfp);
    (void) fclose(rgfp);
    (void) fclose(tmfp);
    if (load_vrs)
        (void) fclose(vmfp);
#ifdef DEBUG
    printf("End\n");
#endif
}

/*
struct d_tree_itm d_t_list[MAX_ITEMS];
char pathdir[L2_MAX_ITEMS];
unsigned top_d_tree = 0, root_d_tree = 0;
struct d_tree_itm * unbances;
struct d_tree_itm * unanfath;
struct d_tree_itm * father;
unsigned ancdepth;
    unsigned depth = 0;

unsigned long top_tan;


rebalance( newn )
struct d_tree_itm * newn;
    {
    char * p_d_p = &(pathdir[ancdepth]);
    register int ancomp = *(p_d_p++);
    register unsigned n_d_p = ((ancomp>0)?(unbances->g_s):
					  (unbances->l_s));
    struct d_tree_itm * d_p = d_t_list + n_d_p;
    struct d_tree_itm * d_p_2 = d_p;
    register unsigned nn;
    register int aux;

    while ( d_p_2 != newn ) {  /* ADJUST BRANCH BALANCING  */
/*
	if ( *(p_d_p++) > 0 ) {
	    d_p_2->balance = 1;
	    nn = d_p_2->g_s;
	  }
	else {
	    d_p_2->balance = -1;
	    nn = d_p_2->l_s;
	  }
	d_p_2 = d_t_list + nn;
      }
    if ( ! (aux = unbances->balance) ) {  /* HIGHER TREE  */
/*
	unbances->balance = ancomp;
	return;
      }
    if ( aux != ancomp ) {  /*  BALANCED TREE  */
/*
	unbances->balance = 0;
	return;
      }
    /*  UNBALANCED TREE  */
/*
    aux = d_p->balance;
    if ( ancomp == aux ) {  /*  SINGLE ROTATION  */
/*
	d_p_2 = d_p;
	if ( ancomp > 0 ) {
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
/*
	if ( ancomp > 0 ) {
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
	if ( aux == ancomp ) {
	    unbances->balance = -ancomp;
	    d_p_2->balance = d_p->balance = 0;
	  }
	else if ( aux ) {
	    d_p_2->balance = unbances->balance = 0;
	    d_p->balance = ancomp;
	  }
	else
	    unbances->balance = d_p->balance = 0;
      }
    /*  ROOT ADJUSTMENT  */
/*
    nn = d_p_2 - d_t_list;
    if ( unanfath == NULL )
	root_d_tree = nn;
    else {
	if ( pathdir[ancdepth-1] > 0 )
	  unanfath->g_s = nn;
	else
	  unanfath->l_s = nn;
      }
}

*/
