/*
 *  Torino, April 3, 1991
 *  program: grg_stndrd.c
 *  purpose: TRG construction of a GSPN without net-dependent files
 *           compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No marking dependency is allowed for immediate transitions.
 */
#include <stdlib.h>
#include <stdio.h>
#include "../WN/INCLUDE/compact.h"

/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

/*
#define DEBUGtest_enabl
*/

/*
#define DEBUGtst_ins_array
*/

/*
#define DEBUGtst_ins_tree
*/

/*
#define DEBUGnew_enab
*/

/*
#define DEBUGtst_ins_mark
*/

/*
#define DEBUGtst_ins_ECS
*/

/*
#define DEBUGstore_vanpath
*/

/*
#define DEBUGimmediate_firing
*/

/*
#define DEBUGsave_immpath
*/

#endif


#define THRESHOLD_ARRAY 32

#include "const.h"

#include "grg.h"

int optimize_flag = 0;

unsigned place_num, group_num, trans_num;
int pri_0, first_immediate, transnump1;

#include <stdio.h>

FILE *nfp, * rgfp, * tmfp, * aefp;


struct Codbyt_descr {
    unsigned char shift;
    unsigned char mask;
    unsigned short num_byte;
    unsigned short link;
};

struct Place_descr {
    unsigned short lbound;
    unsigned short rubound;
    struct Codbyt_descr *cods;
};

/*  PLACE ARRAY  */ struct Place_descr *p_list;
short *chngp;

unsigned sub_num;

/* CODING BYTES */ unsigned *no_cod;

/* CODED MARKING */ unsigned char **CM;
unsigned char *ent_1, *ent_2;


union Link_descr {
    struct Mnode_descr *node;
    struct ECS_descr *ecsp;
    unsigned nmark;
};

union Vanpath {
    unsigned short noel;
    unsigned short *arr;
    struct Change_descr *change_places;
};


union List_descr {
    union Vanpath *vp;
    unsigned short *en_list;
};

struct Mark_descr {
    union List_descr list;
    struct Mnode_descr *father;
    unsigned char pval;
};

struct ECS_descr {
    unsigned char pval;
    unsigned short number;
    struct Mnode_descr *father;
};

struct Vantrans_array {
    unsigned short tt;
    unsigned short aecs;
    struct Vantrans_array *next;
};

struct Vantrans_path {
    unsigned short path_length;
    struct Vantrans_array *path;
    struct Vantrans_path *next;
};

union Val_descr;
struct Mnode_descr;

struct Byte_descr {
    unsigned char noitems;
    unsigned char max_conf;
    unsigned short first_item;
    union Link_descr *(*tst_ins)(unsigned, unsigned char *,
                      unsigned, struct Byte_descr *,
                      union Val_descr *, struct Mnode_descr *, struct Mnode_descr **);
};


struct Byte_descr **encoding;

struct Test_descr {
    unsigned short mask;
    unsigned short shift;
    unsigned short offset;
    unsigned short test;
};

struct Enabl_descr {
    unsigned char notestge;
    unsigned char notestlt;
    unsigned short nobyte;
    struct Test_descr *test_list;
};

struct Change_descr {
    unsigned short place;
    short delta;
};

struct Trans_descr {
    unsigned char prior;
    unsigned char nosub;
    unsigned char no_serv;
    unsigned char notestbytes;
    unsigned char nochangeplaces;
    unsigned short tnum;
    struct Codbyt_descr ecs_cod;
    struct Enabl_descr *test_bytes;
    struct Change_descr *change_places;
    unsigned short *add_list;
    unsigned short *tst_list;
};

/* TRANSITION ARRAY */ struct Trans_descr *t_list;
unsigned short *addl, *tstl;

unsigned short *ECS_counters;

struct Stack_el {
    unsigned char is_new;
    unsigned short firing_t;
    unsigned short ecs_num;
    unsigned short sub_num;
    unsigned nm;
    unsigned char *cm;
    unsigned short *el;
    unsigned short *trans_arr;
};

struct Stack_el stack[MAX_STACK];

/* DECODED MARKING */ unsigned short *DM;

unsigned lastan, toptan = 0;
unsigned lasvan, topvan = MAX_MARKS;

static unsigned tmrk_limit = 0;
static unsigned tmrk_top_p = 0;
static unsigned vmrk_limit = MAX_MARKS;
static unsigned vmrk_top_p = MAX_ARR;

static struct Mark_descr *tmrk_p = NULL;
static struct Mark_descr *vmrk_p = NULL;

struct Val_tree_descr {
    union Link_descr next;
    struct Val_tree_descr *great;
    struct Val_tree_descr *less;
};

union Val_descr {
    struct Val_tree_descr *root;
    union Link_descr *array;
};

struct Mnode_descr {
    unsigned char pval;
    union Val_descr values;
    struct Mnode_descr *father;
};

union Val_descr *rootm;


void terminate() {
    fprintf(stderr, "\n... Abort with lastan=%d, toptan=%d\n\n",
            lastan, toptan); exit(1);
}


/* MARKING DESCRIPTION RECORDS */
struct Mark_descr *mrk_array[MAX_ARR];


char *ecalloc(unsigned nitm, unsigned  sz) {
    char               *callptr;

#ifdef Linux

    if (nitm == 0)
        nitm = 1;
#endif

    if ((callptr = (char*)calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        /*        (*terminate)(); */
        terminate();
    }
    return (callptr);
}


struct Mark_descr *marking(unsigned num) {
    unsigned mlo, mup;

    if (((mup = --num / MRKSLOT) >= MAX_ARR) ||
            ((mup >= tmrk_top_p) && (mup < vmrk_top_p))) {
        fprintf(stderr, "ERROR: trying to reference unallocated marking #%d\n",
                num + 1);
        fprintf(stderr, "mup=%d, tmrk_top_p=%d, vmrk_top_p=%d\n",
                mup, tmrk_top_p, vmrk_top_p);
        /*        (*terminate)(); */
        terminate();
    }
    mlo = num % MRKSLOT;
    return (mrk_array[mup] + mlo);
}


void load_mark(unsigned char *Mp, struct Mark_descr *m_p, unsigned nn) {
    struct Mnode_descr *pnm_p;
    int nb;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of load_mark %d\n", nn);
#endif
    for (pnm_p = m_p->father, nb = no_cod[nn] - 1,
            Mp = Mp + nb, *(Mp--) = m_p->pval ;
            nb-- ;
            * (Mp--) = pnm_p->pval, pnm_p = pnm_p->father);
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of load_mark\n");
#endif
}


void decode_mark(unsigned char *Mp, unsigned short *dmp, unsigned nn) {
    struct Place_descr *place_p = p_list;
    unsigned np, nb, val;

    for (np = place_num ; np-- ; place_p++) {
        if ((nb = place_p->cods[nn].num_byte)) {
            nb--;
            val = Mp[nb] & place_p->cods[nn].mask;
            val = val >> (place_p->cods[nn].shift);
            val += place_p->lbound;
            *(++dmp) = val;
        }
        else
            *(++dmp) = 0;
    }
}


void transcode_mark(unsigned char *C1p, unsigned l1, unsigned char *C2p, unsigned l2) {
    struct Place_descr *place_p = p_list;
    struct Byte_descr *byte2_p = *(encoding + l2);
    unsigned nb2 = no_cod[l2];
    unsigned np, pn, bn1, val;
    int ss;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of transcode_mark from %d to %d\n", l1, l2);
#endif
    for (; nb2-- ; byte2_p++, C2p++) {
        *C2p = 0;
        for (np = byte2_p->noitems, pn = byte2_p->first_item ;
                np-- ; pn = place_p->cods[l2].link) {
            place_p = p_list + (pn - 1);
            if ((bn1 = place_p->cods[l1].num_byte)) {
                val = C1p[bn1 - 1];
                if ((val &= place_p->cods[l1].mask)) {
                    if ((ss = place_p->cods[l1].shift - place_p->cods[l2].shift)) {
                        if (ss > 0)
                            val = val >> ss;
                        else
                            val = val << (-ss);
                    }
                    *C2p |= val;
                }
            }
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of transcode_mark\n");
#endif
}


unsigned test_enabl(unsigned char *Mp, unsigned nt) {
    struct Trans_descr *trans_p = t_list + (nt - 1);
    struct Enabl_descr *enabl_p;
    struct Test_descr *test_p;
    unsigned nb, bn, ntge, ntests;
    unsigned char *val_p, * ucp, * nucp;
    unsigned val, en_degree = 0;
    int no_repeat = trans_p->no_serv;


#ifdef DEBUGCALLS
    unsigned uuu;
    uuu = t_list[nt - 1].tnum;
    fprintf(stderr, "\n    Start of test_enabl for trans %d (repeat <=%d)\n",
            uuu, no_repeat);
#endif
    ucp = ent_1; nucp = ent_2;
    while (no_repeat--) {
        for (nb = trans_p->notestbytes, enabl_p = trans_p->test_bytes ; nb-- ;
                enabl_p++) {
            bn = (enabl_p->nobyte) - 1;
#ifdef DEBUGtest_enabl
            fprintf(stderr, "      testing byte #%d\n", bn + 1);
#endif
            val_p = Mp + bn;
            test_p = enabl_p->test_list;
            ucp[bn] = *val_p;
            for (ntge = enabl_p->notestge, ntests = ntge + (enabl_p->notestlt) ;
                    ntests-- ; test_p++) {
#ifdef DEBUGtest_enabl
                fprintf(stderr, "        ntests=%d\n", ntests);
#endif
                if ((val = (*val_p) & (test_p->mask)))
                    val = val >> (test_p->shift);
                if (no_repeat == (trans_p->no_serv - 1)) {
#ifdef DEBUGtest_enabl
                    fprintf(stderr, "        first time");
#endif
                    val += test_p->offset;
                }
#ifdef DEBUGtest_enabl
                fprintf(stderr, "        val=%d, comp=%d, ntge=%d\n", val, test_p->test, ntge);
#endif
                if (ntge) {
                    if (test_p->test > val) {
#ifdef DEBUGCALLS
                        fprintf(stderr, "    End of test_enabl\n");
#endif
                        return (en_degree);
                    }
#ifdef DEBUGtest_enabl
                    fprintf(stderr, "        val >= comp\n");
#endif
                    ntge--;
                    if (no_repeat) {
                        val -= test_p->test;
                        ucp[bn] = ucp[bn] & ~(test_p->mask);
#ifdef DEBUGtest_enabl
                        fprintf(stderr, "        *val_p=%d, ucp[%d]=%d\n", *val_p, bn, ucp[bn]);
#endif
                        if (val) {
                            val = test_p->mask & (val << (test_p->shift));
                            ucp[bn] |= val;
#ifdef DEBUGtest_enabl
                            fprintf(stderr, "          val=%d, ucp[%d]=%d\n", val, bn, ucp[bn]);
#endif
                        }
                    }
                }
                else {
                    if (test_p->test <= val) {
#ifdef DEBUGCALLS
                        fprintf(stderr, "    End of test_enabl\n");
#endif
                        return (en_degree);
                    }
#ifdef DEBUGtest_enabl
                    fprintf(stderr, "        val < comp\n");
#endif
                }
            }
        }
        en_degree++;
        if (no_repeat) {
#ifdef DEBUGtest_enabl
            fprintf(stderr, "      repeating\n");
#endif
            Mp = ucp; ucp = nucp;
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of test_enabl\n");
#endif
    return (en_degree);
}


void do_fire(unsigned np, struct Change_descr *change_p, unsigned char *oldmrk, unsigned char *newmrk) {
    struct Place_descr *place_p;
    unsigned pn, nb;
    unsigned ss, mm;
    unsigned char *val_p, val;
    int ival;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of do_fire\n");
#endif
    for (val_p = newmrk, nb = *no_cod ; nb-- ;)
        *(val_p++) = *(oldmrk++);
    for (; np-- ; change_p++) {
        pn = change_p->place;
        place_p = p_list + (pn - 1);
        ss = place_p->cods->shift; mm = place_p->cods->mask;
        nb = place_p->cods->num_byte - 1;
        val_p = newmrk + nb;
        if ((val = *val_p))
            * val_p &= ~mm;
        ival = ((mm & val) >> ss);
        if ((ival += change_p->delta) > (place_p->rubound)) {
            fprintf(stderr, "\n *** Error while firing transition:\n");
            fprintf(stderr, "      place %d is not %d-bounded (%d tokens) !\n",
                    pn, place_p->lbound + place_p->rubound, place_p->lbound + ival);
            /*        (*terminate)(); */
            terminate();
        }
        if (ival)
            *val_p |= (((unsigned)(ival)) << ss);
    }

#ifdef DEBUGCALLS
    fprintf(stderr, "    End of do_fire\n");
#endif
}


union Link_descr *
        tst_ins_array(unsigned nn, unsigned char *Mp,
                      unsigned nb, struct Byte_descr *byte_p,
                      union Val_descr *val_p, struct Mnode_descr *father_p, struct Mnode_descr **leaf_p) {
    union Link_descr *elem_p;
    struct Place_descr *place_p;
    unsigned np, nopl, offset;
    union Link_descr lnk;
    union Link_descr *mrkp;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of tst_ins_array on byte %d\n", nb);
#endif
    if (val_p->array == NULL) {
        unsigned ii = byte_p->max_conf + 1;
#ifdef DEBUGtst_ins_array
        fprintf(stderr, "    adding new array of %d elements\n", ii);
#endif
        val_p->array = elem_p = (union Link_descr *)
                                ecalloc(ii, sizeof(union Link_descr));
        if (nb)
            while (ii--)
                (elem_p++)->node = NULL;
        else {
            if (nn < sub_num)
                while (ii--)
                    (elem_p++)->nmark = 0;
            else
                while (ii--)
                    (elem_p++)->ecsp = NULL;
        }
    }
    if (*Mp && ((nopl = byte_p->noitems) > 1) && (nn < sub_num)) {
        int ll;
#ifdef DEBUGtst_ins_array
        fprintf(stderr, "    computing index (nn=%d nopl=%d)\n", nn, nopl);
#endif
        for (offset = 0, np = byte_p->first_item ; nopl-- ;
                np = place_p->cods[nn].link) {
            unsigned kk;
#ifdef DEBUGtst_ins_array
            fprintf(stderr, "      marking of place %d", np);
#endif
            place_p = p_list + (np - 1);
            if (offset) {
                ll = (place_p->rubound + 1);
                offset *= ll;
#ifdef DEBUGtst_ins_array
                fprintf(stderr, "      ll=%d", ll);
                fprintf(stderr, ", offset=%d\n", offset);
#endif
            }
            if ((kk = (*Mp & place_p->cods[nn].mask))) {
                kk = kk >> (place_p->cods[nn].shift);
                offset += kk;
#ifdef DEBUGtst_ins_array
                fprintf(stderr, "      kk=%d\n", kk);
#endif
            }
#ifdef DEBUGtst_ins_array
            else
                fprintf(stderr, "      kk == 0\n");
#endif
        }
#ifdef DEBUGtst_ins_array
        fprintf(stderr, "    index =%d (computed)\n", offset);
#endif
    }
    else {
        offset = *Mp;
#ifdef DEBUGtst_ins_array
        fprintf(stderr, "    index =%d (assigned)\n", offset);
#endif
    }
    elem_p = val_p->array + offset;
    if (nb) {
        if (elem_p->node == NULL) {   /*  insert new node  */
#ifdef DEBUGtst_ins_array
            fprintf(stderr, "    insert new node\n");
#endif
            lnk.node = (struct Mnode_descr *)
                       ecalloc((unsigned)1, sizeof(struct Mnode_descr));
            *elem_p = lnk;
            lnk.node->pval = *Mp;
            lnk.node->father = father_p;
            byte_p++;
            if (byte_p->max_conf < THRESHOLD_ARRAY)
                lnk.node->values.array = NULL;
            else
                lnk.node->values.root = NULL;
            mrkp = (byte_p->tst_ins)
                   (nn, Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
#ifdef DEBUGCALLS
            fprintf(stderr, "  End of tst_ins_array\n");
#endif
            return (mrkp);
        }
#ifdef DEBUGtst_ins_array
        fprintf(stderr, "    is an old node\n");
#endif
        byte_p++; lnk = *elem_p;
        mrkp = (byte_p->tst_ins)
               (nn, Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
#ifdef DEBUGCALLS
        fprintf(stderr, "  End of tst_ins_array\n");
#endif
        return (mrkp);
    }
#ifdef DEBUGtst_ins_array
    fprintf(stderr, "    leaf reached\n");
#endif
    *leaf_p = father_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of tst_ins_array\n");
#endif
    return (elem_p);
}

union Link_descr *
        tst_ins_tree(unsigned nn,
                     unsigned char *Mp,
                     unsigned nb,
                     struct Byte_descr *byte_p,
                     union Val_descr *val_p,
                     struct Mnode_descr *father_p,
                     struct Mnode_descr **leaf_p) {
    struct Val_tree_descr *node_p, ** son_pp;
    union Link_descr lnk;
    unsigned cval;
    struct Mark_descr *mrk_p;
    union Link_descr *mrkp;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of tst_ins_tree on byte %d\n", nb);
#endif
    for (son_pp = &(val_p->root), node_p = val_p->root; node_p != NULL ;) {
#ifdef DEBUGtst_ins_tree
        fprintf(stderr, "     testing existing node\n");
#endif
        lnk = node_p->next;
        if (nb)
            cval = lnk.node->pval;
        else {
            if (nn < sub_num) {
                mrk_p = marking(lnk.nmark);
                cval = mrk_p->pval;
            }
            else {
                cval = lnk.ecsp->pval;
            }
        }
        if (*Mp == cval) {
#ifdef DEBUGtst_ins_tree
            fprintf(stderr, "       is equal\n");
#endif
            if (nb) {
                byte_p++;
                mrkp = (byte_p->tst_ins)
                       (nn, Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
            }
            else mrkp = &(node_p->next);
#ifdef DEBUGCALLS
            fprintf(stderr, "  End of tst_ins_tree\n");
#endif
            return (mrkp);
        }
        else {
            if (*Mp > cval)  {
#ifdef DEBUGtst_ins_tree
                fprintf(stderr, "       is greater\n");
#endif
                son_pp = &(node_p->great);
            }
            else {
#ifdef DEBUGtst_ins_tree
                fprintf(stderr, "       is lower\n");
#endif
                son_pp = &(node_p->less);
            }
            node_p = *son_pp;
        }
    }
    /*  insert new node  */
#ifdef DEBUGtst_ins_tree
    fprintf(stderr, "    insert new node\n");
#endif
    *son_pp = node_p = (struct Val_tree_descr *)
                       ecalloc((unsigned)1, sizeof(struct Val_tree_descr));
    node_p->great = NULL;
    node_p->less = NULL;
    if (nb) {
        lnk.node = (struct Mnode_descr *)
                   ecalloc((unsigned)1, sizeof(struct Mnode_descr));
        node_p->next = lnk;
        lnk.node->pval = *Mp;
        lnk.node->father = father_p;
        byte_p++;
        if (byte_p->max_conf < THRESHOLD_ARRAY)
            lnk.node->values.array = NULL;
        else
            lnk.node->values.root = NULL;
        mrkp = (byte_p->tst_ins)
               (nn, Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
#ifdef DEBUGCALLS
        fprintf(stderr, "  End of tst_ins_tree\n");
#endif
        return (mrkp);
    }
    mrkp = &(node_p->next);
    if (nn < sub_num)
        mrkp->nmark = 0;
    else
        mrkp->ecsp = NULL;
    *leaf_p = father_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of tst_ins_tree\n");
#endif
    return (mrkp);
}



unsigned
new_tanmrk() {
    if (++toptan > tmrk_limit) {
        if (++tmrk_top_p >= vmrk_top_p) {
            fprintf(stderr, "Sorry, too many markings! Increase constant MAX_MARKS\n");
            /*        (*terminate)(); */
            terminate();
        }
        tmrk_limit += MRKSLOT;
        if ((tmrk_p = (struct Mark_descr *)
                      calloc(MRKSLOT, sizeof(struct Mark_descr))) == 0) {
            fprintf(stderr, "calloc: couldn't alloc enough memory!\n");
            /*        (*terminate)(); */
            terminate();
        }
        mrk_array[tmrk_top_p - 1] = tmrk_p;
    }
    tmrk_p = marking(toptan);
    tmrk_p->list.en_list = NULL;
    return (toptan);
}

unsigned
new_vanmrk() {
    if (--topvan <= vmrk_limit) {
        if (--vmrk_top_p <= tmrk_top_p) {
            fprintf(stderr, "Sorry, too many markings! Increase constant MAX_MARKS\n");
            /*        (*terminate)(); */
            terminate();
        }
        vmrk_limit -= MRKSLOT;
        if ((vmrk_p = (struct Mark_descr *)
                      calloc(MRKSLOT, sizeof(struct Mark_descr))) == 0) {
            fprintf(stderr, "calloc: couldn't alloc enough memory!\n");
            /*        (*terminate)(); */
            terminate();
        }
        mrk_array[vmrk_top_p] = vmrk_p;
    }
    vmrk_p = marking(topvan);
    vmrk_p->list.vp = NULL;
    return (topvan);
}


void store_enlist(unsigned short *elp,
                  unsigned noel,
                  struct Mark_descr *mrkp) {
    unsigned short *cp;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of store_enlist\n");
#endif
    if ((cp = (unsigned short *)calloc(noel + 1, short_SIZE)) == 0) {
        fprintf(stderr, "calloc: couldn't alloc enough memory!\n");
        /*        (*terminate)(); */
        terminate();
    }
    mrkp->list.en_list = cp;
    noel = (noel >> 1);
    *(cp) = (unsigned short)noel;
    while (*elp)
        *(++cp) = *(elp++);
    *(++cp) = (unsigned short)0;
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of store_enlist\n");
#endif
}


unsigned
new_enab(unsigned short *old_e, unsigned short  *new_e,
         int nt,
         unsigned short *aep,
         unsigned short *tep,
         int maxt,
         unsigned char *Mp,
         struct Trans_descr **firsten) {
    unsigned el, oel, mult;
    int curpri = -1;
    struct Trans_descr *entdp;
    unsigned short *first = new_e;

#ifdef DEBUGCALLS
    fprintf(stderr, "   Start of new_enab\n");
#endif
    for (oel = *old_e ; oel && (oel < maxt) ; oel = *(++old_e)) {
#ifdef DEBUGnew_enab
        fprintf(stderr, "     oel=%d, *aep=%d, *tep=%d\n", oel, (int)*aep, (int)*tep);
#endif
        while ((el = *aep) && el < oel) {
#ifdef DEBUGnew_enab
            fprintf(stderr, "      testing el(*aep)=%d", el);
            uuu = t_list[el - 1].tnum;
            fprintf(stderr, " (t%d)", uuu);
#endif
            aep++;
            while (*tep && *tep <= el)
                tep++;
            if ((mult = test_enabl(Mp, el))) {
#ifdef DEBUGnew_enab
                fprintf(stderr, " yes, %d times (curpri=%d)\n", mult, curpri);
#endif
                *(new_e++) = el; *(new_e++) = mult;
                entdp = t_list + (el - 1);
                if ((int)(entdp->prior) > curpri) {
                    curpri = entdp->prior;
#ifdef DEBUGnew_enab
                    fprintf(stderr, "        curpri updated to %d\n", curpri);
#endif
                    *firsten = entdp;
                }
            }
#ifdef DEBUGnew_enab
            else
                fprintf(stderr, " no\n");
#endif
        }
        while (*tep && *tep < oel)
            tep++;
        while (*aep && *aep <= oel)
            aep++;
#ifdef DEBUGnew_enab
        fprintf(stderr, "      testing oel=%d (*tep=%d)", oel, (int)*tep);
        uuu = t_list[oel - 1].tnum;
        fprintf(stderr, " (t%d)", uuu);
#endif
        if (*tep != oel && oel != nt && el != oel) {
            *(new_e++) = oel; *(new_e++) = *(++old_e);
#ifdef DEBUGnew_enab
            fprintf(stderr, " yes, %d times (curpri=%d)\n", (int)*old_e, curpri);
#endif
            entdp = t_list + (oel - 1);
            if ((int)(entdp->prior) > curpri) {
                curpri = entdp->prior;
#ifdef DEBUGnew_enab
                fprintf(stderr, "        curpri updated to %d\n", curpri);
#endif
                *firsten = entdp;
            }
        }
        else if ((mult = test_enabl(Mp, oel))) {
#ifdef DEBUGnew_enab
            fprintf(stderr, " yes, %d times (curpri=%d)\n", mult, curpri);
#endif
            *(new_e++) = oel; *(new_e++) = mult; old_e++;
            entdp = t_list + (oel - 1);
            if ((int)(entdp->prior) > curpri) {
                curpri = entdp->prior;
#ifdef DEBUGnew_enab
                fprintf(stderr, "        curpri updated to %d\n", curpri);
#endif
                *firsten = entdp;
            }
        }
        else {
            old_e++;
#ifdef DEBUGnew_enab
            fprintf(stderr, " no\n");
#endif
        }
    }
    while ((el = *(aep++)) && (el < maxt)) {
#ifdef DEBUGnew_enab
        fprintf(stderr, "      testing el=%d", el);
        uuu = t_list[el - 1].tnum;
        fprintf(stderr, " (t%d)", uuu);
#endif
        if ((mult = test_enabl(Mp, el))) {
#ifdef DEBUGnew_enab
            fprintf(stderr, " yes, %d times (curpri=%d)\n", mult, curpri);
#endif
            *(new_e++) = el; *(new_e++) = mult;
            entdp = t_list + (el - 1);
            if ((int)(entdp->prior) > curpri) {
                curpri = entdp->prior;
#ifdef DEBUGnew_enab
                fprintf(stderr, "        curpri updated to %d\n", curpri);
#endif
                *firsten = entdp;
            }
        }
#ifdef DEBUGnew_enab
        else
            fprintf(stderr, " no\n", el);
#endif
    }
    *(new_e++) = 0;
#ifdef DEBUGCALLS
    fprintf(stderr, "   End of new_enab\n");
#endif
    return ((unsigned)(new_e - first));
}


unsigned
tst_ins_mark(unsigned char *Mp,
             unsigned short *new_e,
             unsigned ne,
             unsigned subnet,
             unsigned char *new_p) {
    unsigned *mrkp, mrk;
    union Link_descr *lnk_p;
    int nb;
    struct Mnode_descr *pnm;
    struct Byte_descr *byte_p;
    struct Mark_descr *xmrk_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of tst_ins_mark subnet=%d\n", subnet);
#endif
    nb = (no_cod[subnet]) - 1;
    byte_p = encoding[subnet];
    lnk_p = (byte_p->tst_ins)(subnet, Mp, nb, byte_p, rootm + subnet, NULL, &pnm);
#ifdef DEBUGtst_ins_mark
    fprintf(stderr, "        ... returned from byte_p->tst_ins\n");
#endif
    mrkp = &(lnk_p->nmark);
#ifdef DEBUGtst_ins_mark
    fprintf(stderr, "        ... got mrkp = %d\n", (unsigned)mrkp);
#endif
    mrk = * mrkp;
#ifdef DEBUGtst_ins_mark
    fprintf(stderr, "        ... got mrk\n");
#endif
    if ((*new_p = (mrk == 0))) /* NEW MARKING */ {
#ifdef DEBUGtst_ins_mark
        fprintf(stderr, "   new marking\n");
#endif
        if (subnet) {
            mrk = new_vanmrk();
            xmrk_p = marking(mrk);
        }
        else {
            unsigned char *cc = Mp;
            int num = *no_cod;
            /* WRITE TANGIBLE CODING */
            for (; num-- ; putc(*cc, tmfp), cc++);
            mrk = new_tanmrk();
            xmrk_p = marking(mrk);
            store_enlist(new_e , ne, xmrk_p);
        }
        xmrk_p->father = pnm;
        xmrk_p->pval = Mp[nb];
        *mrkp = mrk;
    }
#ifdef DEBUGtst_ins_mark
    else
        fprintf(stderr, "   old marking # %d\n", mrk);
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of tst_ins_mark\n");
#endif
    return (mrk);
}

unsigned
immediate_firing(unsigned char *newmrk,
                 unsigned pri,
                 unsigned short *new_e,
                 struct Trans_descr *firsten,
                 struct Stack_el *stckp1);

void init() {
    int ii, jj, kk, ll, mm, nn, np, ns, nt, ss, nm;
    struct Byte_descr **byte_pp;
    struct Byte_descr *byte_p;
    struct Place_descr *place_p;
    unsigned char *ucp;
    unsigned short *usp;
    struct Trans_descr *trans_p;
    struct Enabl_descr *enabl_p;
    struct Test_descr *test_p;
    struct Change_descr *change_p;
    int na, nb, nge, nlt, nc, n1, n2, n3, n4;
    struct Stack_el *sp;
    union Val_descr *rootm_p;
    int pri;
    struct Trans_descr *firsten = NULL;

#ifdef DEBUG
    unsigned uuu;
    fprintf(stderr, "  Start of init\n");
#endif

    /* read number of objects in the net */
    fscanf(nfp, "%d %d %d %d\n", &sub_num, &place_num, &group_num, &trans_num);

    pri_0 = sub_num - 1;
    ii = sub_num + group_num;
    encoding = (struct Byte_descr **)ecalloc(ii, sizeof(struct Byte_descr *));
    no_cod = (unsigned *)ecalloc(ii, sizeof(unsigned));
    CM = (unsigned char **)ecalloc(ii, sizeof(unsigned char *));
    rootm = (union Val_descr *)ecalloc(ii, sizeof(union Val_descr));
    DM = (unsigned short *)ecalloc(place_num + 1, sizeof(unsigned short));
    chngp = (short *)ecalloc(place_num + 1, sizeof(short));
    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));
    t_list = (struct Trans_descr *)ecalloc(trans_num, sizeof(struct Trans_descr));
    ECS_counters = (unsigned short *)ecalloc(group_num,
                   sizeof(unsigned short));
    for (usp = ECS_counters, ii = 0 ; ii++ < group_num ; * (usp++) = 0);

    for (ii = 0, place_p = p_list; ++ii <= place_num; place_p++) {
        struct Codbyt_descr *cods_p;
        place_p->cods = (struct Codbyt_descr *)ecalloc(sub_num,
                        sizeof(struct Codbyt_descr));
        for (cods_p = place_p->cods, nn = 0 ; nn++ < sub_num ;)
            (cods_p++)->num_byte = 0;
    }

    /* read codings */
    for (byte_pp = encoding, nn = 0 ; nn < sub_num ; nn++, byte_pp++) {
        fscanf(nfp, "%d %d\n", &ii, &jj);
#ifdef DEBUG
        fprintf(stderr, "  subnet #%d (%d bytes):\n",
                nn, jj);
#endif
        *byte_pp = (struct Byte_descr *)ecalloc(jj, sizeof(struct Byte_descr));
        CM[nn] = (unsigned char *)ecalloc(jj, sizeof(unsigned char));
        no_cod[nn] = jj;
        if (nn == 0) {
            ent_1 = (unsigned char *)ecalloc(jj, sizeof(unsigned char));
            ent_2 = (unsigned char *)ecalloc(jj, sizeof(unsigned char));
        }
        for (byte_p = *byte_pp, ucp = CM[nn], ii = 0 ;
                ++ii <= no_cod[nn] ; byte_p++) {
            struct Codbyt_descr *cods_p;
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &kk, &ll, &np, &mm);
            byte_p->max_conf = ll;
            byte_p->noitems = np;
            if ((ii == 1) || (ll < THRESHOLD_ARRAY))
                byte_p->tst_ins = tst_ins_array;
            else
                byte_p->tst_ins = tst_ins_tree;
            *(ucp++) = mm;
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &n1, &n2, &n3, &n4);
            byte_p->first_item = jj;
            for (kk = 1 ; ++kk <= np ;) {
                fscanf(nfp, "%d", &ll);
                place_p = p_list + (jj - 1);
                cods_p = place_p->cods + nn;
                cods_p->link = ll;
                jj = ll;
                cods_p->num_byte = ii;
                cods_p->mask = n1;
                cods_p->shift = n2;
                fscanf(nfp, "%d %d %d %d\n", &n1, &n2, &n3, &n4);
            }
            place_p = p_list + (jj - 1);
            cods_p = place_p->cods + nn;
            cods_p->link = 0;
            cods_p->num_byte = ii;
            cods_p->mask = n1;
            cods_p->shift = n2;
#ifdef DEBUG
            fprintf(stderr, "    coding #%d (%d configurations)\n",
                    ii, byte_p->max_conf);
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
        while (getc(nfp) != '\n');
        place_p->lbound = nt;
        place_p->rubound = nn /* -nt*/;
#ifdef DEBUG
        fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                ii, place_p->cods->mask, place_p->cods->shift, place_p->cods->num_byte);
        fprintf(stderr, "            lbound=%d rubound=%d notok=%d link=%d\n",
                place_p->lbound, place_p->rubound, mm, place_p->cods->link);
#endif
    }

    /* read groups */
    for (nn = sub_num, byte_pp = encoding + nn, ii = 0 ;
            ii++ < group_num; byte_pp++, nn++) {
        fscanf(nfp, "%d %d\n", &jj, &kk);
        *byte_pp = (struct Byte_descr *)ecalloc(kk, sizeof(struct Byte_descr));
        no_cod[nn] = kk;
        CM[nn] = (unsigned char *)ecalloc(kk, sizeof(unsigned char));
        for (byte_p = *byte_pp, jj = 0 ; ++jj <= kk ; byte_p++) {
            struct Codbyt_descr *cods_p;
            fscanf(nfp, " %d %d %d %d\n", &n1, &n2, &n3, &n4);
            byte_p->max_conf = n3;
            byte_p->noitems = n4;
            if ((jj == 1) || (n3 < THRESHOLD_ARRAY))
                byte_p->tst_ins = tst_ins_array;
            else
                byte_p->tst_ins = tst_ins_tree;
            byte_p->first_item = 0;
            for (ll = 0 ; ll++ < n2 ;) {
                fscanf(nfp, " %d %d %d\n", &nt, &mm, &ss);
                trans_p = t_list + (nt - 1);
                cods_p = &(trans_p->ecs_cod);
                cods_p->link = byte_p->first_item;
                byte_p->first_item = nt;
                cods_p->num_byte = n1;
                cods_p->mask = mm;
                cods_p->shift = ss;
            }
#ifdef DEBUG
            fprintf(stderr, "    ECS #%d, Byte #%d (%d configurations) nn=%d\n",
                    ii, jj, byte_p->max_conf, nn);
#endif
        }
    }

    first_immediate = 0;
    transnump1 = trans_num + 1;
    /* read transitions */
    for (trans_p = t_list, nt = 0 ; nt++ < trans_num ; trans_p++) {
        fscanf(nfp, "%d %d %d %d %d %d %d", &ii, &jj, &ns, &kk,
               &ll, &nb, &nc);
        while (getc(nfp) != '\n');
        trans_p->tnum = ii;
        trans_p->prior = jj;
        if (jj && !first_immediate)
            first_immediate = nt;
        trans_p->nosub = ns;
        trans_p->no_serv = ll;
        trans_p->notestbytes = nb;
        trans_p->nochangeplaces = nc;
        trans_p->test_bytes = enabl_p =
                                  (struct Enabl_descr *)ecalloc(nb, sizeof(struct Enabl_descr));
        trans_p->change_places = change_p =
                                     (struct Change_descr *)ecalloc(nc, sizeof(struct Change_descr));

        for (ii = 0 ; ++ii <= nb ; enabl_p++) {
            fscanf(nfp, "  %d %d %d\n", &jj, &nge, &nlt);
            enabl_p->nobyte = jj;
            enabl_p->notestge = nge;
            enabl_p->notestlt = nlt;
            nge += nlt;
            enabl_p->test_list = test_p =
                                     (struct Test_descr *)ecalloc(nge, sizeof(struct Test_descr));
            for (jj = 0 ; ++jj <= nge ; test_p++) {
                fscanf(nfp, "    %d %d %d %d\n", &mm, &kk, &ll, &nn);
                test_p->mask = mm;
                test_p->shift = kk;
                test_p->offset = ll;
                test_p->test = nn;
            }
        }
        for (ii = 0 ; ++ii <= nc ; change_p++) {
            fscanf(nfp, " %d %d\n", &np, &jj);
            change_p->place = np;
            change_p->delta = jj;
        }
        fscanf(nfp, " %d", &na);
        trans_p->add_list = usp =
                                (unsigned short *)ecalloc(na + 1, sizeof(unsigned short));
        for (ii = 0 ; ++ii <= na ;) {
            fscanf(nfp, "%d", &jj);
            *(usp++) = jj;
        }
        *(usp) = 0;
        fscanf(nfp, "\n %d", &nc);
        trans_p->tst_list = usp =
                                (unsigned short *)ecalloc(nc + 1, sizeof(unsigned short));
        for (ii = 0 ; ++ii <= nc ;) {
            fscanf(nfp, "%d", &jj);
            *(usp++) = jj;
        }
        *(usp) = 0;
        while (getc(nfp) != '\n');
    }

    addl = (unsigned short *)ecalloc(transnump1, sizeof(unsigned short));
    tstl = (unsigned short *)ecalloc(transnump1, sizeof(unsigned short));

    /* alloc stack */
    stack->cm = (unsigned char *)calloc(*no_cod, 1);
    stack->el = NULL;
    stack->ecs_num = stack->sub_num = 0;
    for (ii = MAX_STACK, sp = stack + 1 ; --ii ; sp++) {
        sp->cm = (unsigned char *)calloc(*no_cod, 1);
        sp->el = (unsigned short *)calloc(trans_num + trans_num + 1, short_SIZE);
    }

    /* initialize coding trees */
    for (rootm_p = rootm, ii = 0 ; ii++ < (sub_num + group_num) ;)
        (rootm_p++)->array = NULL;
    toptan = 0;

#ifdef DEBUG
    fprintf(stderr, "    Initial Marking:\n");
    {
        int ii = *no_cod; unsigned char *cp = *CM;
        unsigned short *up;
        decode_mark(cp, DM, 0);
        fprintf(stderr, "     cmark=");
        for (; ii-- ; cp++)
            fprintf(stderr, " %d,", (int)*cp);
        fprintf(stderr, "\n  mark=");
        for (ii = place_num, up = &(DM[1]) ; ii-- ; up++)
            fprintf(stderr, " %d,", (int)*up);
    }
    fprintf(stderr, "\n\n*** Insert initial marking in RG\n");
#endif

    /* compute enabled transitions list for M0 */
    for (sp = stack + 1, usp = sp->el, nt = 0, trans_p = t_list,
            pri = -1 ; nt++ < trans_num ; trans_p++) {
        unsigned mult;
#ifdef DEBUG
        fprintf(stderr, "    Start of enabling test for t%d (pri=%d)", nt, pri);
        uuu = t_list[nt - 1].tnum;
        fprintf(stderr, " (t%d)\n", uuu);
#endif
        if ((mult = test_enabl(*CM, (unsigned)nt))) {
            int pp = trans_p->prior;
            if (pp > pri) {
                pri = pp;
                firsten = trans_p;
            }
            *(usp++) = (unsigned short)(nt);
            *(usp++) = (unsigned short)mult;
#ifdef DEBUG
            fprintf(stderr, "    End of enabling test for t%d, %d-enabled (pp=%d, pri=%d)\n",
                    uuu, mult, pp, pri);
#endif
        }
#ifdef DEBUG
        else
            fprintf(stderr, "    End of enabling test for t%d, disabled (pri=%d)\n",
                    uuu, pri);
#endif
    }
    *(usp++) = (unsigned short)0;
    if (pri) { /* VANISHING INITIAL MARKING */
        unsigned char *ucp, *ucp1;
        int ii;
#ifdef DEBUG
        fprintf(stderr, "    ... vanishing initial marking\n");
#endif
        for (ucp1 = sp->cm, ucp = *CM, ii = *no_cod ; ii-- ;
                * (ucp1++) = *(ucp++));
        store_compact((unsigned long)0, rgfp);
        nm = immediate_firing(sp->cm, pri, sp->el, firsten, sp);
        store_compact((unsigned long)0, rgfp);
    }
    else {
        unsigned char is_new;
#ifdef DEBUG
        fprintf(stderr, "      ... tangible initial marking\n");
#endif
        nm = tst_ins_mark(*CM, sp->el, (unsigned)(usp - sp->el),
                          0, &is_new);
        store_compact((unsigned long)nm, rgfp);
    }


#ifdef DEBUG
    fprintf(stderr, "\ntrans_num=%d, first_immediate=%d\n", trans_num,
            first_immediate);
    for (trans_p = t_list, nt = 0 ; nt++ < trans_num ; trans_p++) {
        fprintf(stderr, "   trans #%d, prior=%d, nosub=%d, no_serv=%d, notestbytes=%d\n",
                nt, trans_p->prior, trans_p->nosub, trans_p->no_serv,
                trans_p->notestbytes);
    }
#endif
#ifdef DEBUG
    fprintf(stderr, "  End of init\n");
#endif
}


void save_immpath(struct Stack_el *stckp) {
    struct Stack_el *sp;
    int num;
    unsigned long uu;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of save_immpath stacktop=%d\n", (int)(stckp - stack));
#endif
    for (num = 0, sp = stack ; sp++ < stckp ;)
        if ((int)(sp->is_new)) {
#ifdef DEBUGsave_immpath
            fprintf(stderr, "          sp=%d is new\n", sp - stack);
#endif
            if (! optimize_flag || sp->ecs_num)
                num++;
        }
        else {
#ifdef DEBUGsave_immpath
            printf("          sp=%d is old (num=%d)\n", sp - stack, (int)(*(sp->trans_arr)));
#endif
            if (optimize_flag) {
                unsigned short *usp = sp->trans_arr;
                int ii = *usp;
                while (ii--) {
                    usp += 2;
                    if (*usp)
                        num++;
                }
            }
            else
                num += (int)(*(sp->trans_arr));
        }
#ifdef DEBUGsave_immpath
    fprintf(stderr, "        path-length=%d\n", num);
#endif
    store_compact((unsigned long)num, rgfp);
    for (sp = stack ; sp++ < stckp ;)
        if ((int)(sp->is_new)) {
            if (! optimize_flag || sp->ecs_num) {
                uu = sp->firing_t - 1;
#ifdef DEBUGsave_immpath
                fprintf(stderr, "             (new) firing %d\n", uu + 1);
#endif
                uu = t_list[uu].tnum;
#ifdef DEBUGsave_immpath
                fprintf(stderr, "          trans=%d ecs=%d\n", (int)uu, (int)(sp->ecs_num));
#endif
                store_compact(uu, rgfp);
                store_compact((unsigned long)(sp->ecs_num), rgfp);
            }
        }
        else {
            unsigned short *usp;
            unsigned long ee;
            for (num = *(usp = sp->trans_arr) ; num-- ;) {
                uu = *(++usp);
                if ((ee = *(++usp)) || !optimize_flag) {
#ifdef DEBUGsave_immpath
                    fprintf(stderr, "             (old) firing %d\n", uu);
#endif
                    uu = t_list[uu - 1].tnum;
                    store_compact(uu, rgfp);
                    store_compact(ee, rgfp);
#ifdef DEBUGsave_immpath
                    fprintf(stderr, "          trans=%d ecs=%d\n", (int)uu, (int)(*usp));
#endif
                }
            }
        }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of save_immpath stacktop=%d\n", (int)(stckp - stack));
#endif
}

unsigned short
tst_ins_ECS(struct Trans_descr *trans_p,
            unsigned short *new_e,
            unsigned ecs) {
    struct ECS_descr *ecsp;
    unsigned short *new_e2 = new_e;
    int ii, nb, nt, indx = pri_0 + ecs;
    unsigned char *ecs_p;
    struct Mnode_descr *pnm;
    struct Byte_descr *byte_p;
    union Link_descr *lnk_p;

#ifdef DEBUGCALLS
    uuu = t_list[(uuu = *new_e) - 1].tnum;
    fprintf(stderr, "  Start of tst_ins_ECS trans=%d, indx=%d\n", uuu, indx);
#endif
    if (optimize_flag &&
            (!(nt = *(new_e + 2)) || (t_list[nt - 1].prior != ecs))) {
#ifdef DEBUGCALLS
        fprintf(stderr, "  End of tst_ins_ECS\n");
#endif
        return (0);
    }
    for (nb = no_cod[indx], nt = 0, ecs_p = CM[indx], byte_p = encoding[indx];
            nb-- ; byte_p++) {
#ifdef DEBUGtst_ins_ECS
        fprintf(stderr, "       nb=%d nt=%d\n", nb, nt);
#endif
        nt += byte_p->noitems;
        *(ecs_p++) = 0;
    }
    if (nt == 1) {
#ifdef DEBUGCALLS
        fprintf(stderr, "  End of tst_ins_ECS\n");
#endif
        return (0);
    }
    for (nt = 0, ii = *new_e ; ii ;) {
        trans_p = t_list + (ii - 1);
        if ((ii = (trans_p->prior == ecs))) {
#ifdef DEBUGtst_ins_ECS
            uuu = t_list[(uuu = *new_e) - 1].tnum;
            fprintf(stderr, "         adding trans %d\n", uuu);
#endif
            new_e += 2;
            ii = *new_e;
            nt++;
            nb = trans_p->ecs_cod.num_byte;
            ecs_p = CM[indx] + (nb - 1);
            *ecs_p |= trans_p->ecs_cod.mask;
        }
    }
    nb = (no_cod[indx]) - 1;
    byte_p = encoding[indx];
    lnk_p = (byte_p->tst_ins)(indx, CM[indx], nb, byte_p, rootm + indx, NULL, &pnm);
    ecsp = lnk_p->ecsp;
    if (ecsp == NULL) { /* NEW ACTUAL ECS */
#ifdef DEBUGtst_ins_ECS
        fprintf(stderr, "   new actual ECS\n");
#endif
        ecsp = (struct ECS_descr *)ecalloc((unsigned)1, sizeof(struct ECS_descr));
        ecsp->father = pnm;
#ifdef DEBUGtst_ins_ECS
        fprintf(stderr, "         ecs=%d, indx=%d, nb=%d\n", ecs, indx, nb);
#endif
        ecsp->number = ii = ++(ECS_counters[ecs - 1]);
        ecsp->pval = (CM[indx])[nb];
#ifdef DEBUGtst_ins_ECS
        fprintf(stderr, "         ii=%d, lnk_p=%d, nt=%d\n", ii, (int)lnk_p, nt);
#endif
        lnk_p->ecsp = ecsp;
        fprintf(aefp, "%d %d %d", ecs, ii, nt);
        for (; nt-- ; new_e2 += 2) {
#ifdef DEBUGtst_ins_ECS
            fprintf(stderr, "            nt=%d\n", nt);
#endif
            ii = *new_e2;
            fprintf(aefp, " %d", t_list[ii - 1].tnum);
        }
        fprintf(aefp, "\n");
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of tst_ins_ECS\n");
#endif
    return (ecsp->number);
}


static struct Vantrans_array *free_vantransarray = NULL;

struct Vantrans_array *
new_vantransarray() {
    struct Vantrans_array *vap;
    if ((vap = free_vantransarray) == NULL) {
        vap = (struct Vantrans_array *)ecalloc((unsigned)1,
                                               sizeof(struct Vantrans_array));
        return (vap);
    }
    free_vantransarray = vap->next;
    return (vap);
}

static struct Vantrans_path *free_vantranspath = NULL;

struct Vantrans_path *
new_vantranspath() {
    struct Vantrans_path *vap;
    if ((vap = free_vantranspath) == NULL) {
        vap = (struct Vantrans_path *)ecalloc((unsigned)1, sizeof(struct Vantrans_path));
        return (vap);
    }
    free_vantranspath = vap->next;
    return (vap);
}


void
compute_vanpath(unsigned ntr,
                unsigned necs,
                unsigned nmark,
                int is_tang,
                unsigned *num_p,
                struct Vantrans_path **paths_pp) {
    struct Vantrans_array *array_p;
    struct Vantrans_array *narray_p;
    struct Vantrans_path *paths_p;
    union Vanpath *vpl;
    int ii, jj;
    unsigned short *usp;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of compute_vanpath (nmark=%d, is_tang=%d, num=%d)\n",
            nmark, is_tang, *num_p);
#endif
    if (is_tang) {
        paths_p = new_vantranspath();
        paths_p->path_length = 1;
        paths_p->next = *paths_pp;
        *paths_pp = paths_p;
        ++(*num_p);
        paths_p->path = array_p = new_vantransarray();
        array_p->tt = ntr;
        array_p->aecs = necs;
        array_p->next = NULL;
#ifdef DEBUGCALLS
        fprintf(stderr, "      End of compute_vanpath\n");
#endif
        return;
    }
    vpl = marking(nmark)->list.vp;
    for ((*num_p) += (ii = (vpl++)->noel) ; ii-- ; vpl += 5) {
        paths_p = new_vantranspath();
        paths_p->next = *paths_pp;
        *paths_pp = paths_p;
        usp = vpl->arr;
        paths_p->path_length = 1 + (jj = *usp);
        paths_p->path = array_p = new_vantransarray();
        array_p->tt = ntr;
        array_p->aecs = necs;
        while (jj--) {
            array_p->next = narray_p = new_vantransarray();
            array_p = narray_p;
            array_p->tt = *(++usp);
            array_p->aecs = *(++usp);
        }
        array_p->next = NULL;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of compute_vanpath (num=%d)\n", *num_p);
#endif
}


void
store_vanpath(unsigned nvan,
              struct Vantrans_path *paths_p,
              unsigned num) {
    struct Vantrans_array *array_p;
    union Vanpath *vpl;
    struct Vantrans_path *first_p = paths_p;
    unsigned short *usp;
    short *sp;
    int numtrans, compute_firing, dd;
    unsigned short *add_lst = NULL, * tst_lst;
    struct Change_descr *change_p = NULL, * chp;
    unsigned nochangeplaces = 0;
    unsigned ii, nt, np;
    struct Trans_descr *trans_p;
    struct Mark_descr *mark_p;
    int nnnn;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of store_vanpath nvan=%d, num=%d\n", nvan, num);
#endif
    ii = (unsigned)(num) << 2;
    ii += num + 1;
    vpl = (union Vanpath *)ecalloc(ii, sizeof(union Vanpath));
    mark_p = marking(nvan);
#ifdef DEBUGstore_vanpath
    fprintf(stderr, "        %d Vanpaths allocated (%d) and assigned to %d\n",
            ii, (int)vpl, (int)mark_p);
#endif
    mark_p->list.vp = vpl;
    for ((vpl++)->noel = num; num-- ;) {
        numtrans = paths_p->path_length;
#ifdef DEBUGstore_vanpath
        fprintf(stderr, "          path of %d transitions:\n", numtrans);
#endif
        nnnn = numtrans;
        (vpl++)->arr = usp = (unsigned short *)ecalloc(nnnn + nnnn + 1,
                             sizeof(unsigned short));
        if ((compute_firing = (numtrans > 1))) {
            for (add_lst = addl, ii = transnump1 ; --ii ;
                    * (++add_lst) = 0);
            for (sp = chngp, ii = place_num ; ii-- ; * (++sp) = 0);
        }
        for (tst_lst = tstl, ii = transnump1 ; --ii ;
                * (++tst_lst) = 0);
        for (*(usp++) = nnnn, array_p = paths_p->path ; numtrans-- ;) {
            *(usp++) = nt = array_p->tt;
            *(usp++) = array_p->aecs;
#ifdef DEBUGstore_vanpath
            uuu = t_list[nt - 1].tnum;
            fprintf(stderr, "            trans %d, aecs=%d\n", uuu, array_p->aecs);
#endif
            trans_p = t_list + (nt - 1);
            for (ii = *(tst_lst = trans_p->tst_list) ; ii ;) {
                tstl[ii] = 1;
                ii = *(++tst_lst);
            }
            tstl[nt] = 1;
            if (compute_firing) {
                for (ii = *(add_lst = trans_p->add_list) ; ii ;) {
                    addl[ii] = 1;
                    ii = *(++add_lst);
                }
                for (ii = trans_p->nochangeplaces,
                        change_p = trans_p->change_places; ii-- ; change_p++) {
                    np = change_p->place;
                    chngp[np] += change_p->delta;
                }
            }
            else {
                add_lst = trans_p->add_list;
                nochangeplaces = trans_p->nochangeplaces;
                change_p = trans_p->change_places;
            }
            if (numtrans)
                array_p = array_p->next;
        }
        array_p->next = free_vantransarray;
        free_vantransarray = paths_p->path;
#ifdef DEBUGstore_vanpath
        fprintf(stderr, "          path completed\n");
#endif
        if (compute_firing) {
            for (ii = nt = 0, add_lst = addl + 1 ; ii++ < trans_num ;
                    add_lst++)
                if (*add_lst)
                    nt++;
            add_lst = usp = (unsigned short *)ecalloc(nt + 1,
                            sizeof(unsigned short));
            for (ii = 1 ; nt ; ii++)
                if (addl[ii]) {
                    nt--;
                    *(usp++) = ii;
                }
            *usp = 0;
            for (sp = chngp + 1, ii = place_num, nochangeplaces = 0 ; ii-- ;
                    ++sp)
                if ((int)(*sp) != 0)
                    nochangeplaces++;
            change_p = chp = (struct Change_descr *)ecalloc(nochangeplaces,
                             sizeof(struct Change_descr));
            for (ii = 1, np = nochangeplaces ; np ; ii++)
                if ((dd = chngp[ii])) {
                    np--;
                    chp->place = ii;
                    (chp++)->delta = dd;
                }
        }
        for (ii = nt = 0, tst_lst = tstl + 1 ; ii++ < trans_num ;
                tst_lst++)
            if (*tst_lst)
                nt++;
        tst_lst = usp = (unsigned short *)ecalloc(nt + 1,
                        sizeof(unsigned short));
        for (ii = 1 ; nt ; ii++)
            if (tstl[ii]) {
                nt--;
                *(usp++) = ii;
            }
        *usp = 0;
        (vpl++)->arr = add_lst;
        (vpl++)->arr = tst_lst;
        (vpl++)->noel = nochangeplaces;
        (vpl++)->change_places = change_p;
#ifdef DEBUGstore_vanpath
        fprintf(stderr, "          path stored\n");
#endif
        if (num)
            paths_p = paths_p->next;
    }
    paths_p->next = free_vantranspath;
    free_vantranspath = first_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of store_vanpath\n");
#endif
}


unsigned fire_trans(struct Stack_el *stckp, int *same_path);

void recurs_firing(unsigned nm,
                   unsigned pri,
                   unsigned from,
                   unsigned short *new_e,
                   struct Stack_el *sp) {
    unsigned num;
    unsigned to_m;
    struct Vantrans_path *to_tang_paths = NULL;
    /*    unsigned different = (((sp-1) != stack) &&
    			  (sp->sub_num != (sp-1)->sub_num)); */
    int same_path;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of recurs_firing on vanishing #%d\n", nm);
#endif
    while (*new_e && *new_e < from)
        new_e += 2;
    sp->ecs_num = tst_ins_ECS(t_list + (from - 1), new_e, pri);
    num = 0;
    while (*new_e) {
        int is_tang;
        sp->firing_t = *new_e;
        to_m = fire_trans(sp, &same_path);
        is_tang = (to_m <= toptan) || (!same_path);
        compute_vanpath(*new_e, sp->ecs_num, to_m, is_tang,
                        &num, &to_tang_paths);
        new_e += 2;
    }
    store_vanpath(nm, to_tang_paths, num);
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of recurs_firing\n");
#endif
}


unsigned
immediate_firing(unsigned char *newmrk,
                 unsigned pri,
                 unsigned short *new_e,
                 struct Trans_descr *firsten,
                 struct Stack_el *stckp1) {
    unsigned newsubn = firsten->nosub;
    unsigned nm, ne;

#ifdef DEBUGCALLS
    fprintf(stderr, "   Start of immediate_firing\n");
#endif
    transcode_mark(newmrk, 0, CM[newsubn], newsubn);
    stckp1->nm = nm = tst_ins_mark(CM[newsubn], new_e, 0, newsubn,
                                   &(stckp1->is_new));
    stckp1->sub_num = newsubn;
    /*
    KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK
    */

    if ((int)(stckp1->is_new)) {   /* RECURSIVE FIRING USING THE STACK */
#ifdef DEBUGimmediate_firing
        fprintf(stderr, "    new vanishing %d subnet %d ecs %d\n", nm, newsubn, pri);
#endif
        recurs_firing(nm, pri, (unsigned)(firsten - t_list) + 1, new_e, stckp1);
    }
    else { /*  OLD VANISHING  */
        union Vanpath *vlp;
        unsigned short *add_lst, * tst_lst;
        unsigned nochanges;
        struct Change_descr *chp;
        struct Mark_descr *vmp;
        struct Stack_el *stckp2 = stckp1 + 1;
        unsigned short *old_e;
        unsigned char *oldmrk;
        int ii;

        if (nm < lasvan) {   /* CHECK LOOP IN THE STACK */
            struct Stack_el *sp = stack + 1;
            while (sp < stckp1)
                if ((sp++)->nm == nm) {
                    fprintf(stderr, "Sorry, vanishing marking loop !\n");
                    /*        (*terminate)(); */
                    terminate();
                }
        }
        vmp = marking(nm);
        vlp = vmp->list.vp;
        oldmrk = newmrk;
        newmrk = stckp2->cm;
        old_e = stckp1->el;
        new_e = stckp2->el;
#ifdef DEBUGimmediate_firing
        fprintf(stderr, "    old vanishing %d (%d contains %d)\n",
                (int)nm, (int)vmp, (int)vlp);
#endif
        for (ii = (vlp++)->noel ; ii-- ;) {
#ifdef DEBUGimmediate_firing
            fprintf(stderr, "      following path %d\n", ii);
#endif
            stckp1->trans_arr = (vlp++)->arr;
            add_lst = (vlp++)->arr;
            tst_lst = (vlp++)->arr;
            nochanges = (vlp++)->noel;
            chp = (vlp++)->change_places;
            do_fire(nochanges, chp, oldmrk, newmrk);
#ifdef DEBUGimmediate_firing
            {
                int ii = (*no_cod); unsigned char *cp = newmrk;
                unsigned short *up;
                decode_mark(cp, DM, 0);
                fprintf(stderr, "     cmark=");
                for (; ii-- ; cp++)
                    fprintf(stderr, " %d,", (int)*cp);
                fprintf(stderr, "\n  mark=");
                for (ii = place_num, up = &(DM[1]) ; ii-- ; up++)
                    fprintf(stderr, " %d,", (int)*up);
            }
            fprintf(stderr, "\n");
#endif
            if ((ne = new_enab(old_e, new_e, 0, add_lst,
                               tst_lst, transnump1, newmrk, &firsten)) <= 1) {
                pri = 0;
                /*
                fprintf(stderr,"\nError: dead marking !\n");
                (*terminate)();
                */
            }
            else (pri = firsten->prior);
            if (pri) { /* VANISHING */
                unsigned uu;
#ifdef DEBUGimmediate_firing
                printf("    vanishing\n");
#endif
                uu = immediate_firing(newmrk, pri, new_e, firsten, stckp2);
            }
            else {
                unsigned char is_new;
                unsigned tmn;
                tmn = tst_ins_mark(newmrk, new_e, ne, 0, &is_new);
#ifdef DEBUGimmediate_firing
                printf("        reached tangible %d (is_new=%d)\n", tmn, is_new);
#endif
                store_compact((unsigned long)tmn, rgfp);
                save_immpath(stckp1);
            }
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "   End of immediate_firing\n");
#endif
    return (nm);
}


unsigned
fire_trans(struct Stack_el *stckp,
           int *same_path) {
    struct Stack_el *stckp1 = stckp + 1;
    unsigned char *oldmrk = stckp->cm;
    unsigned short *old_e = stckp->el;
    unsigned char *newmrk;
    unsigned short *new_e;
    unsigned short ne;
    unsigned nm;
    unsigned pri;
    struct Trans_descr *firsten;


#ifdef DEBUG
    uuu = t_list[(uuu = stckp->firing_t) - 1].tnum;
    fprintf(stderr, "  Start of fire_trans %d\n", uuu);
    fprintf(stderr, "    lasvan=%d, topvan=%d, lastan=%d, toptan=%d\n",
            lasvan, topvan, lastan, toptan);
#endif
    if (((unsigned)(stckp1 - stack) + 1) >= MAX_STACK) {
        fprintf(stderr, "Sorry, stack overflow. Increase constant MAX_STACK\n");
        /*        (*terminate)(); */
        terminate();
    }
    newmrk = stckp1->cm;
    new_e = stckp1->el;
    firsten = t_list + (stckp->firing_t - 1);
    do_fire((unsigned)(firsten->nochangeplaces), firsten->change_places,
            oldmrk, newmrk);
#ifdef DEBUG
    {
        int ii = (*no_cod); unsigned char *cp = newmrk;
        unsigned short *up;
        decode_mark(cp, DM, 0);
        fprintf(stderr, "     cmark=");
        for (; ii-- ; cp++)
            fprintf(stderr, " %d,", (int)*cp);
        fprintf(stderr, "\n  mark=");
        for (ii = place_num, up = &(DM[1]) ; ii-- ; up++)
            fprintf(stderr, " %d,", (int)*up);
    }
    fprintf(stderr, "\n");
#endif
    *same_path = TRUE;
    if ((ne = new_enab(old_e, new_e, (int)(stckp->firing_t), firsten->add_list,
                       firsten->tst_list, transnump1, newmrk, &firsten)) <= 1) {
        pri = 0;
        /*
        fprintf(stderr,"\nError: dead marking !\n");
               (*terminate)();
             */
    }
    else pri = firsten->prior;
    if (pri) { /* VANISHING */
#ifdef DEBUG
        fprintf(stderr, "    vanishing\n");
#endif
        if (stckp == stack)
            store_compact((unsigned long)0, rgfp);
        else
            *same_path = (firsten->nosub ==
                          t_list[stckp->firing_t - 1].nosub);
        nm = immediate_firing(newmrk, pri, new_e, firsten, stckp1);
    }
    else { /* TANGIBLE */
        unsigned char is_new;
        nm = tst_ins_mark(newmrk, new_e, ne, 0, &is_new);
        store_compact((unsigned long)nm, rgfp);
        if (stckp > stack)
            save_immpath(stckp);
#ifdef DEBUG
        fprintf(stderr, "    tangible %d is_new=%d\n", (int)nm, is_new);
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "    lasvan=%d, topvan=%d, lastan=%d, toptan=%d\n",
            lasvan, topvan, lastan, toptan);
    fprintf(stderr, "  End of fire_trans (same_path=%d) <-- %d\n",
            *same_path, nm);
#endif
    return (nm);
}


static const char   *can_t_open = "Can't open file %s for %c\n";
char  filename[100];

int main(int argc, char **argv, char **envp) {
    int no_trans;
    int next_print;
    int same_path;

#ifdef DEBUG
    fprintf(stderr, "Start of grg_stndrd\n");
#endif
    /*    if ( 0 ) { * dirty trick to terminate properly *
          int i = 0;

      term_lab:
    	{
    	  goto end_main;
    	}
          }
    *    terminate = (void (*)())(term_lab); */
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    optimize_flag = 0;
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii--) {
            s_p = *(a_p++);
            if (*s_p == '-' || *(s_p + 1) == 'o')
                optimize_flag = 1;
        }
    }
    sprintf(filename, "%s.grg", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
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
    sprintf(filename, "%s.aecs", argv[1]);
    if ((aefp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    init();
    (void)fclose(nfp);

#ifdef DEBUG
    fprintf(stderr, "Start RG computation\n");
#endif
    for (lastan = 0, next_print = 50 ; lastan++ < toptan ;) {
        struct Mark_descr *m_p = marking(lastan);
        unsigned short *en_tr = m_p->list.en_list;

        load_mark(stack->cm, m_p, 0);
        store_compact((unsigned long)(lastan), rgfp);
#ifdef DEBUG
        fprintf(stderr, "\nfiring from tangible #%d\n", lastan);
        {
            int ii = (*no_cod); unsigned char *cp = stack->cm;
            unsigned short *up;
            decode_mark(cp, DM, 0);
            fprintf(stderr, "     cmark=");
            for (; ii-- ; cp++)
                fprintf(stderr, " %d,", (int)*cp);
            fprintf(stderr, "\n  mark=");
            for (ii = place_num, up = &(DM[1]) ; ii-- ; up++)
                fprintf(stderr, " %d,", (int)*up);
        }
        fprintf(stderr, "\n");
#endif
        stack->nm = lastan;
        if ((no_trans = *(en_tr++)) == 0) {
            fprintf(stderr, " %6d *** dead marking\n", lastan);
            {
                int ii = (*no_cod); unsigned char *cp = stack->cm;
                unsigned short *up;
                decode_mark(cp, DM, 0);
                fprintf(stderr, "        mark=");
                for (ii = place_num, up = &(DM[1]) ; ii-- ; up++)
                    if (ii)
                        fprintf(stderr, " %d,", (int)*up);
                    else
                        fprintf(stderr, " %d\n\n", (int)*up);
            }
            /*
                goto end_main;
            */
        }
        store_compact((unsigned long)(no_trans), rgfp);
        stack->el = en_tr;
        while (no_trans--) {
            unsigned to_m;
            unsigned long uu;
            lasvan = topvan;
            uu = (*en_tr) - 1;
            uu = t_list[uu].tnum;
            store_compact(uu, rgfp);
            stack->firing_t = *(en_tr++);
            store_compact((unsigned long)(*(en_tr++)), rgfp);
            to_m = fire_trans(stack, &same_path);
            if (to_m > toptan)   /* END VANISHING PATHS */
                store_compact((unsigned long)0, rgfp);
        }
        free((m_p->list.en_list));
        if (lastan >= next_print) {
            next_print += 50;
            //fprintf(stderr," %6d\n",lastan);
        }
    }

// end_main:
    fprintf(stderr, " %6d Tangible markings\n\n", toptan);
    store_compact((unsigned long)0, rgfp);
#ifdef DEBUG
    fprintf(stderr, "End RG computation\n");
#endif
    (void) fclose(aefp);
    (void) fclose(rgfp);
    (void) fclose(tmfp);
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((rgfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    fprintf(rgfp, "toptan= %d\n", toptan);
    fprintf(rgfp, "topvan= %d\n", topvan);
    fprintf(rgfp, "maxmark= %d\n", MAX_MARKS);
    fprintf(rgfp, "aecs_conf[%d]=", group_num);
    for (; group_num-- ; ECS_counters++)
        fprintf(rgfp, " %d", *ECS_counters);
    fprintf(rgfp, "\n");
    (void) fclose(rgfp);
#ifdef DEBUG
    fprintf(stderr, "End of grg_stndrd\n");
#endif
    return 0;
}


/* kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk */


//
//struct d_tree_itm d_t_list[MAX_ITEMS];
//char pathdir[L2_MAX_ITEMS];
//unsigned top_d_tree = 0, root_d_tree = 0;
//struct d_tree_itm * unbances;
//struct d_tree_itm * unanfath;
//struct d_tree_itm * father;
//unsigned ancdepth;
//    unsigned depth = 0;
//
//unsigned top_tan;
//
//
//rebalance( newn )
//struct d_tree_itm * newn;
//    {
//    char * p_d_p = &(pathdir[ancdepth]);
//    register int ancomp = *(p_d_p++);
//    register unsigned n_d_p = ((ancomp>0)?(unbances->g_s):
//					  (unbances->l_s));
//    struct d_tree_itm * d_p = d_t_list + n_d_p;
//    struct d_tree_itm * d_p_2 = d_p;
//    register unsigned nn;
//    register int aux;
//
//    while ( d_p_2 != newn ) {  /* ADJUST BRANCH BALANCING  */
//
//	if ( *(p_d_p++) > 0 ) {
//	    d_p_2->balance = 1;
//	    nn = d_p_2->g_s;
//	  }
//	else {
//	    d_p_2->balance = -1;
//	    nn = d_p_2->l_s;
//	  }
//	d_p_2 = d_t_list + nn;
//      }
//    if ( ! (aux = unbances->balance) ) {  /* HIGHER TREE  */
//
//	unbances->balance = ancomp;
//	return;
//      }
//    if ( aux != ancomp ) {  /*  BALANCED TREE  */
//
//	unbances->balance = 0;
//	return;
//      }
//    /*  UNBALANCED TREE  */
//
//    aux = d_p->balance;
//    if ( ancomp == aux ) {  /*  SINGLE ROTATION  */
//
//	d_p_2 = d_p;
//	if ( ancomp > 0 ) {
//	    unbances->g_s = d_p->l_s;
//	    nn = unbances - d_t_list;
//	    d_p->l_s = nn;
//	  }
//	else {
//	    unbances->l_s = d_p->g_s;
//	    nn = unbances - d_t_list;
//	    d_p->g_s = nn;
//	  }
//	unbances->balance = d_p->balance = 0;
//      }
//    else {  /*  DOUBLE ROTATION  */
//
//	if ( ancomp > 0 ) {
//	    nn = d_p->l_s;
//	    d_p_2 = d_t_list + nn;
//	    d_p->l_s = d_p_2->g_s;
//	    d_p_2->g_s = n_d_p;
//	    unbances->g_s = d_p_2->l_s;
//	    nn = unbances - d_t_list;
//	    d_p_2->l_s = nn;
//	  }
//	else {
//	    nn = d_p->g_s;
//	    d_p_2 = d_t_list + nn;
//	    d_p->g_s = d_p_2->l_s;
//	    d_p_2->l_s = n_d_p;
//	    unbances->l_s = d_p_2->g_s;
//	    nn = unbances - d_t_list;
//	    d_p_2->g_s = nn;
//	  }
//	aux = d_p_2->balance;
//	if ( aux == ancomp ) {
//	    unbances->balance = -ancomp;
//	    d_p_2->balance = d_p->balance = 0;
//	  }
//	else if ( aux ) {
//	    d_p_2->balance = unbances->balance = 0;
//	    d_p->balance = ancomp;
//	  }
//	else
//	    unbances->balance = d_p->balance = 0;
//      }
//    /*  ROOT ADJUSTMENT  */
//
//    nn = d_p_2 - d_t_list;
//    if ( unanfath == NULL )
//	root_d_tree = nn;
//    else {
//	if ( pathdir[ancdepth-1] > 0 )
//	  unanfath->g_s = nn;
//	else
//	  unanfath->l_s = nn;
//      }
//}
//
//
