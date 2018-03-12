/*************************************************************************
 *
 *  Torino, August 26, 1993
 *  program: engine_pn.c
 *  purpose: Simulation engine for GreatSPN : PN routines
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No marking dependency is allowed for immediate transitions.
 *   2) This program is derived from "grg_stndrd.c". It uses similar
 *      data structures and the same type of optimization techniques.
 *
 *************************************************************************/

#include <stdlib.h>
#include "engine_pn.h"

/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

/*
#define DEBUGinit_pn
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
#define DEBUGupdate_en_list
*/

/*
#define DEBUGimm_enab
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

#define DETERMINISTIC_TRANS 127

#include "../gsrc2/const.h"
#include "engine_decl.h"
#include "engine_event.h"

static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[256];


FILE **traces_f_a = NULL;
int *conflict_counts = NULL;

int place_num, group_num, trans_num;
int last_timed;
Place_p decoded_mark;
Trans_p enab_list;


FILE *nfp;


/*  PLACE ARRAY  */ Place_p p_list;

unsigned sub_num;

/* CODING BYTES */ int no_cod;

/* CODED MARKING */ unsigned char *cur_mark;
unsigned char *old_mark;

Byte_p encoding;

/* TRANSITION ARRAY */ Trans_p t_list;



unsigned lastan, toptan = 0;

static unsigned tmrk_limit = 0;
static unsigned tmrk_top_p = 0;

static struct Mark_descr *tmrk_p = NULL;

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

union Val_descr rootm;


/* MARKING DESCRIPTION RECORDS */
struct Mark_descr *mrk_array[MAX_ARR];

/* predeclaration */
void getname(char   *name_pr);
union Link_descr *tst_ins_tree(unsigned char *Mp,
                                       unsigned nb,
                                       struct Byte_descr *byte_p,
                                       union Val_descr *val_p,
                                       struct Mnode_descr *father_p,
                                       struct Mnode_descr **leaf_p);
union Link_descr *tst_ins_array(unsigned char *Mp,
                                        unsigned nb,
                                        struct Byte_descr *byte_p,
                                        union Val_descr *val_p,
                                        struct Mnode_descr *father_p,
                                        struct Mnode_descr **leaf_p);



char *ecalloc(unsigned nitm, unsigned sz) {
    char *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(1);
    }
    return (callptr);
}


struct Mark_descr *marking(unsigned num) {
    unsigned mlo, mup;

    if (((mup = --num / MRKSLOT) >= MAX_ARR) || (mup >= tmrk_top_p)) {
        fprintf(stderr, "ERROR: trying to reference unallocated marking #%d\n",
                num + 1);
        fprintf(stderr, "mup=%d, tmrk_top_p=%d\n", mup, tmrk_top_p);
        exit(1);
    }
    mlo = num % MRKSLOT;
    return (mrk_array[mup] + mlo);
}


void load_mark(unsigned char *Mp, struct Mark_descr *m_p) {
    struct Mnode_descr *pnm_p;
    int nb;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of load_mark\n");
#endif
    for (pnm_p = m_p->father, nb = no_cod - 1,
            Mp = Mp + nb, *(Mp--) = m_p->pval ;
            nb-- ;
            * (Mp--) = pnm_p->pval, pnm_p = pnm_p->father);
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of load_mark\n");
#endif
}


void place_visibility(int which, int how) {
    if (which > 0) {
        p_list[which].traced = how;
#ifdef DEBUG
        fprintf(stderr, "  .. set place %d traced=%d\n", which, how);
#endif
        return;
    }
    for (which = place_num ; which ; --which) {
        p_list[which].traced = how;
#ifdef DEBUG
        fprintf(stderr, "  .. set place %d traced=%d\n", which, how);
#endif
    }
}


void trans_visibility(int which, int how) {
    if (which > 0) {
        which = t_list[which].sort_index;
        t_list[which].traced = how;
#ifdef DEBUG
        fprintf(stderr, "  .. set trans %d traced=%d\n", which, how);
#endif
        return;
    }
    for (which = trans_num ; which ; --which) {
        t_list[which].traced = how;
#ifdef DEBUG
        fprintf(stderr, "  .. set trans %d traced=%d\n", which, how);
#endif
    }
}


int decode_filter_mark(unsigned char *Mp) {
    Place_p place_p = p_list + 1;
    Place_p ppp = NULL;
    unsigned np, nb, val;
    int change_flag = FALSE;

#ifdef DEBUGCALLS
    fprintf(stderr, "     Start of decode_filter_mark\n");
#endif
    decoded_mark = NULL;
    for (np = place_num ; np-- ; place_p++) {
        if ((nb = place_p->cods.num_byte) &&
                (int)(place_p->traced)) {
            nb--;
            val = Mp[nb] & place_p->cods.mask;
            val = val >> (place_p->cods.shift);
        }
        else
            val = 0;
        val += place_p->lbound;
        if (val != (unsigned)(place_p->mark))
            change_flag = TRUE;
        if ((place_p->mark = val)) {
            if (ppp == NULL)
                decoded_mark = place_p;
            else
                ppp->mark_next = place_p;
            ppp = place_p;
            ppp->mark_next = NULL;
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "     End of decode_filter_mark <- %d\n", change_flag);
#endif
    return (change_flag);
}

unsigned char *ent_1;
unsigned char *ent_2;

unsigned test_enabl(unsigned char *Mp, unsigned nt) {
    Trans_p trans_p = t_list + nt;
    Enabl_p enabl_p;
    Test_p test_p;
    unsigned nb, bn, ntge, ntests;
    unsigned char *val_p, * ucp, * nucp;
    unsigned val, en_degree = 0;
    int no_repeat = trans_p->no_serv;

#ifdef DEBUGCALLS
    fprintf(stderr, "\n    Start of test_enabl for trans %d (repeat <=%d)\n",
            nt, no_repeat);
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
                        fprintf(stderr, "    End of test_enabl <- %d\n", en_degree);
#endif
                        return (en_degree);
                    }
#ifdef DEBUGtest_enabl
                    fprintf(stderr, "        val >= comp\n");
#endif
                    ntge--;
                    if (no_repeat) {
                        val -= test_p->test;
                        ucp[bn] = *val_p & ~(test_p->mask);
                        if (val) {
                            val = test_p->mask & (val << (test_p->shift));
                            ucp[bn] |= val;
                        }
                    }
                }
                else {
                    if (test_p->test <= val) {
#ifdef DEBUGCALLS
                        fprintf(stderr, "    End of test_enabl <- %d\n", en_degree);
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
    fprintf(stderr, "    End of test_enabl <- %d\n", en_degree);
#endif
    return (en_degree);
}


void do_fire(unsigned np,
             struct Change_descr *change_p,
             int undo,
             unsigned char *oldmrk,
             unsigned char *newmrk) {
    struct Place_descr *place_p;
    unsigned pn, nb;
    unsigned ss, mm;
    unsigned char *val_p, val;
    int ival;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of do_fire\n");
#endif
    for (val_p = newmrk, nb = no_cod ; nb-- ;)
        *(val_p++) = *(oldmrk++);
    for (; np-- ; change_p++) {
        pn = change_p->place;
        place_p = p_list + pn;
        ss = place_p->cods.shift; mm = place_p->cods.mask;
        nb = place_p->cods.num_byte - 1;
        val_p = newmrk + nb;
        if ((val = *val_p))
            * val_p &= ~mm;
        ival = ((mm & val) >> ss);
        if (undo)
            ival -= change_p->delta;
        else
            ival += change_p->delta;
        if (ival > (place_p->rubound)) {
            fprintf(stderr, "\n *** Error while firing transition:\n");
            fprintf(stderr, "      place %d is not %d-bounded (%d tokens) !\n",
                    pn, place_p->lbound + place_p->rubound, place_p->lbound + ival);
            exit(100);
        }
        if (ival)
            *val_p |= (((unsigned)(ival)) << ss);
    }

#ifdef DEBUGCALLS
    fprintf(stderr, "    End of do_fire\n");
#endif
}


static Trans_p new_list = NULL;
static Trans_p new_e = NULL;

void update_enabling(int nt,
                     int mult) {
    Trans_p t_p = t_list + nt;
    int oldm = t_p->enabled;
    int pri = t_p->pri;

#ifdef DEBUGCALLS
    fprintf(stderr, "   Start of update_enabling %d %d, oldm=%d\n",
            nt, mult, oldm);
#endif
    if (mult) {
        if (new_e == NULL)
            new_list = t_p;
        else
            new_e->enabl_next = t_p;
        new_e = t_p;
        if (pri > cur_priority)
            cur_priority = pri;
        if (mult > oldm) {
            if (pri) {
                sched_immediate(t_p, (double)(pri));
#ifdef DEBUGCALLS
                fprintf(stderr, "   End of update_enabling\n");
#endif
                return;
            }
            /* else */
            sched_instances(t_p, mult - oldm);
#ifdef DEBUGCALLS
            fprintf(stderr, "   End of update_enabling\n");
#endif
            return;
        }
    }
    if (mult == oldm) {
#ifdef DEBUGCALLS
        fprintf(stderr, "   End of update_enabling\n");
#endif
        return;
    }
    /* if ( mult < oldm ) */
    deschedule(t_p, oldm - mult);
#ifdef DEBUGCALLS
    fprintf(stderr, "   End of update_enabling\n");
#endif
}


void update_en_list(int nt,
                    unsigned short *aep,
                    unsigned short *tep,
                    unsigned char *Mp) {
    unsigned el, oel, mult;

    Trans_p old_e = enab_list;
    Trans_p old_ne;

#ifdef DEBUGCALLS
    fprintf(stderr, "   Start of update_en_list\n");
#endif
    cur_priority = 0;
    for (; old_e != NULL ; old_e = old_ne) {
        oel = (int)(old_e - t_list);
        old_ne = old_e->enabl_next;
#ifdef DEBUGupdate_en_list
        fprintf(stderr, "     oel=%d, *aep=%d, *tep=%d\n", oel, (int)*aep, (int)*tep);
#endif
        while ((el = *aep) > oel) {
#ifdef DEBUGupdate_en_list
            fprintf(stderr, "      testing el=(*aep)=%d", el);
#endif
            aep++;
            while (*tep >= el)
                tep++;
            if ((mult = test_enabl(Mp, el))) {
#ifdef DEBUGupdate_en_list
                fprintf(stderr, " yes, %d times\n", mult);
#endif
                update_enabling(el, mult);
            }
#ifdef DEBUGupdate_en_list
            else
                fprintf(stderr, " no\n");
#endif
        }
        while (*tep > oel)
            tep++;
        while (*aep >= oel)
            aep++;
#ifdef DEBUGupdate_en_list
        fprintf(stderr, "      testing oel=%d (*tep=%d)", oel, (int)*tep);
#endif
        if (*tep != oel && oel != nt && el != oel) {
            int pri;
            if (new_e == NULL)
                new_list = old_e;
            else
                new_e->enabl_next = old_e;
            new_e = old_e;
            if ((pri = t_list[oel].pri) > cur_priority)
                cur_priority = pri;
#ifdef DEBUGupdate_en_list
            fprintf(stderr, " still, %d times\n", (int)(old_e->enabled));
#endif
        }
        else {
            if ((mult = test_enabl(Mp, oel))) {
#ifdef DEBUGupdate_en_list
                fprintf(stderr, " yes, %d times\n", mult);
#endif
            }
#ifdef DEBUGupdate_en_list
            else {
                fprintf(stderr, " no\n");
            }
#endif
            update_enabling(oel, mult);
        }
    }
    while ((el = *(aep++))) {
#ifdef DEBUGupdate_en_list
        fprintf(stderr, "      testing el=%d", el);
#endif
        if ((mult = test_enabl(Mp, el))) {
#ifdef DEBUGupdate_en_list
            fprintf(stderr, " yes, %d times\n", mult);
#endif
            update_enabling(el, mult);
        }
#ifdef DEBUGupdate_en_list
        else
            fprintf(stderr, " no\n", el);
#endif
    }
    if ((enab_list = new_list) != NULL)
        new_e->enabl_next = NULL;
    new_list = new_e = NULL;
#ifdef DEBUGCALLS
    fprintf(stderr, "   End of update_en_list\n");
#endif
}


void fire_trans(int nt) {
    Trans_p trans_p = t_list + nt;
    unsigned char *ucp = old_mark;

    do_fire(trans_p->nochangeplaces, trans_p->change_places, FALSE,
            cur_mark, ucp);
    old_mark = cur_mark;
    cur_mark = ucp;
    update_en_list(nt, trans_p->add_l, trans_p->test_l, cur_mark);
}


void initialize_en_list(unsigned char *Mp) {
    int nt = trans_num;
    Trans_p t_p = t_list + trans_num;
    int mult, pri;

    cur_priority = 0;
    for (new_e = new_list = NULL ; nt ; --nt, --t_p) {
        t_p->enabled = 0;
        if ((mult = test_enabl(Mp, nt))) {
            if (new_e == NULL)
                new_list = t_p;
            else
                new_e->enabl_next = t_p;
            new_e = t_p;
            if ((pri = t_p->pri)) {
                if (pri > cur_priority)
                    cur_priority = pri;
                sched_immediate(t_p, (double)(pri));
            }
            else {
                sched_instances(t_p, mult);
            }
        }
    }
    if ((enab_list = new_list) != NULL)
        new_e->enabl_next = NULL;
    new_list = new_e = NULL;
}


void unfire_trans(int nt) {
    Trans_p trans_p = t_list + nt;
    unsigned char *ucp = old_mark;

    do_fire(trans_p->nochangeplaces, trans_p->change_places, TRUE /* undo */,
            cur_mark, ucp);
    old_mark = cur_mark;
    cur_mark = ucp;

    nt = trans_num;
    trans_p = t_list + trans_num;
    cur_priority = 0;
    for (new_e = new_list = NULL ; nt ; --nt, --trans_p) {
        if (trans_p->enabled) {
            if ((int)(trans_p->pri) > cur_priority)
                cur_priority = trans_p->pri;
            if (new_e == NULL)
                new_list = trans_p;
            else
                new_e->enabl_next = trans_p;
            new_e = trans_p;
        }
    }
    if ((enab_list = new_list) != NULL)
        new_e->enabl_next = NULL;
    new_list = new_e = NULL;
}


union Link_descr *tst_ins_array(unsigned char *Mp,
                                        unsigned nb,
                                        struct Byte_descr *byte_p,
                                        union Val_descr *val_p,
                                        struct Mnode_descr *father_p,
                                        struct Mnode_descr **leaf_p) {
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
            while (ii--)
                (elem_p++)->nmark = 0;
        }
    }
    if (*Mp && ((nopl = byte_p->noitems) > 1)) {
        int ll;
#ifdef DEBUGtst_ins_array
        fprintf(stderr, "    computing index (nopl=%d)\n", nopl);
#endif
        for (offset = 0, np = byte_p->first_item ; nopl-- ;
                np = place_p->cods.link) {
            unsigned kk;
#ifdef DEBUGtst_ins_array
            fprintf(stderr, "      marking of place %d", np);
#endif
            place_p = p_list + np;
            if (offset) {
                ll = (place_p->rubound + 1);
                offset *= ll;
#ifdef DEBUGtst_ins_array
                fprintf(stderr, "      ll=%d", ll);
                fprintf(stderr, ", offset=%d\n", offset);
#endif
            }
            if ((kk = (*Mp & place_p->cods.mask))) {
                kk = kk >> (place_p->cods.shift);
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
            if (byte_p->tst_ins)
                mrkp = tst_ins_array
                       (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
            else
                mrkp = tst_ins_tree
                       (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
#ifdef DEBUGCALLS
            fprintf(stderr, "  End of tst_ins_array\n");
#endif
            return (mrkp);
        }
#ifdef DEBUGtst_ins_array
        fprintf(stderr, "    is an old node\n");
#endif
        byte_p++; lnk = *elem_p;
        if (byte_p->tst_ins)
            mrkp = tst_ins_array
                   (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
        else
            mrkp = tst_ins_tree
                   (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
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

union Link_descr *tst_ins_tree(unsigned char *Mp,
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
            mrk_p = marking(lnk.nmark);
            cval = mrk_p->pval;
        }
        if (*Mp == cval) {
#ifdef DEBUGtst_ins_tree
            fprintf(stderr, "       is equal\n");
#endif
            if (nb) {
                byte_p++;
                if (byte_p->tst_ins)
                    mrkp = tst_ins_array
                           (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
                else
                    mrkp = tst_ins_tree
                           (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
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
        if (byte_p->tst_ins)
            mrkp = tst_ins_array
                   (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
        else
            mrkp = tst_ins_tree
                   (Mp + 1, nb - 1, byte_p, &(lnk.node->values), lnk.node, leaf_p);
#ifdef DEBUGCALLS
        fprintf(stderr, "  End of tst_ins_tree\n");
#endif
        return (mrkp);
    }
    mrkp = &(node_p->next);
    mrkp->nmark = 0;
    *leaf_p = father_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of tst_ins_tree\n");
#endif
    return (mrkp);
}



unsigned new_tanmrk() {
    if (++toptan > tmrk_limit) {
        if (++tmrk_top_p >= MAX_ARR) {
            fprintf(stderr, "Sorry, too many markings! Increase constant MAX_MARKS\n");
            exit(1);
        }
        tmrk_limit += MRKSLOT;
        if ((tmrk_p = (struct Mark_descr *)
                      calloc(MRKSLOT, sizeof(struct Mark_descr))) == 0) {
            fprintf(stderr, "calloc: couldn't alloc enough memory!\n");
            exit(1);
        }
        mrk_array[tmrk_top_p - 1] = tmrk_p;
    }
    tmrk_p = marking(toptan);
    return (toptan);
}


unsigned tst_ins_mark(unsigned char *Mp, unsigned char *new_p) {
    unsigned *mrkp, mrk;
    union Link_descr *lnk_p;
    int nb;
    struct Mnode_descr *pnm;
    Byte_p byte_p;
    struct Mark_descr *xmrk_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of tst_ins_mark\n");
#endif
    nb = no_cod - 1;
    byte_p = encoding;
    if (byte_p->tst_ins)
        lnk_p = tst_ins_array(Mp, nb, byte_p, &rootm, NULL, &pnm);
    else
        lnk_p = tst_ins_tree(Mp, nb, byte_p, &rootm, NULL, &pnm);
    mrkp = &(lnk_p->nmark);
    mrk = * mrkp;
    if ((*new_p = (mrk == 0))) { /* NEW MARKING */
#ifdef DEBUGtst_ins_mark
        fprintf(stderr, "   new marking\n");
#endif
        mrk = new_tanmrk();
        xmrk_p = marking(mrk);
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


void init_pn() {
    int ii, jj, kk, ll, mm, nn, np, ns, nt, ss;
    int num_clause;
    char cc;
    struct Byte_descr *byte_p;
    struct Place_descr *place_p;
    unsigned char *ucp;
    unsigned short *usp;
    struct Trans_descr *trans_p;
    struct Enabl_descr *enabl_p;
    struct Test_descr *test_p;
    struct Change_descr *change_p;
    int na, nb, nge, nlt, nc, n1, n2, n3, n4;
    int pri, ecs;
    double rw;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init_pn\n");
#endif
    sprintf(filename, "%s.grg", netname);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }

    /* read number of objects in the net */
    fscanf(nfp, "%d %d %d %d\n", &sub_num, &place_num, &group_num, &trans_num);

    p_list = (Place_p)ecalloc(place_num + 1, sizeof(struct Place_descr));
    t_list = (Trans_p)ecalloc(trans_num + 1, sizeof(struct Trans_descr));

    if (record_traces) {
        traces_f_a = (FILE **)ecalloc(trans_num, sizeof(FILE *));
        conflict_counts = (int *)ecalloc(group_num, sizeof(int));
        for (ii = 0 ; ii < group_num ; ii++)
            conflict_counts[ii] = 0;
    }

    for (ii = 0, place_p = p_list; ++ii <= place_num; place_p++) {
        place_p->cods.num_byte = 0;
    }

    /* read codings */
    fscanf(nfp, "%d %d\n", &ii, &jj);
#ifdef DEBUGinit_pn
    fprintf(stderr, "  subnet #0 (%d bytes):\n", jj);
#endif
    no_cod = jj;
    cur_mark = (unsigned char *)ecalloc(jj, sizeof(unsigned char));
    old_mark = (unsigned char *)ecalloc(jj, sizeof(unsigned char));
    ent_1 = (unsigned char *)ecalloc(jj, sizeof(unsigned char));
    ent_2 = (unsigned char *)ecalloc(jj, sizeof(unsigned char));
    encoding = (Byte_p)ecalloc(jj, sizeof(struct Byte_descr));
    for (byte_p = encoding, ucp = cur_mark, ii = 0 ;
            ii++ < no_cod ; byte_p++) {
        struct Codbyt_descr *cods_p;
        fscanf(nfp, "%d %d %d %d %d\n", &jj, &kk, &ll, &np, &mm);
        byte_p->max_conf = ll;
        byte_p->noitems = np;
        if ((ii == 1) || (ll < THRESHOLD_ARRAY))
            byte_p->tst_ins = TRUE;
        else
            byte_p->tst_ins = FALSE;
        *(ucp++) = mm;
        fscanf(nfp, "%d %d %d %d %d\n", &jj, &n1, &n2, &n3, &n4);
        byte_p->first_item = jj;
        for (kk = 1 ; kk++ < np ;) {
            fscanf(nfp, "%d", &ll);
            place_p = p_list + jj;
            cods_p = &(place_p->cods);
            cods_p->link = ll;
            jj = ll;
            cods_p->num_byte = ii;
            cods_p->mask = n1;
            cods_p->shift = n2;
            fscanf(nfp, "%d %d %d %d\n", &n1, &n2, &n3, &n4);
        }
        place_p = p_list + jj;
        cods_p = &(place_p->cods);
        cods_p->link = 0;
        cods_p->num_byte = ii;
        cods_p->mask = n1;
        cods_p->shift = n2;
#ifdef DEBUGinit_pn
        fprintf(stderr, "    coding #%d (%d configurations)\n",
                ii, byte_p->max_conf);
#endif
    }
    for (nn = 1 ; nn < sub_num ; nn++) {
        int no_cod_nn;
        fscanf(nfp, "%d %d\n", &ii, &no_cod_nn);
#ifdef DEBUGinit_pn
        fprintf(stderr, "  subnet #%d (%d bytes):\n", nn, no_cod_nn);
#endif
        for (ii = 0 ; ++ii <= no_cod_nn ;) {
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &kk, &ll, &np, &mm);
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &n1, &n2, &n3, &n4);
            for (kk = 1 ; kk++ < np ;) {
                fscanf(nfp, "%d", &ll);
                fscanf(nfp, "%d %d %d %d\n", &n1, &n2, &n3, &n4);
            }
#ifdef DEBUGinit_pn
            fprintf(stderr, "    coding #%d\n", ii);
#endif
        }
    }

    /* read places */
#ifdef DEBUGinit_pn
    fprintf(stderr, "   places:\n");
#endif
    for (ii = 0, place_p = p_list + 1; ii++ < place_num; place_p++) {
        fscanf(nfp, "%d %d %d %d %d %d %d",
               &np, &jj, &kk, &ll, &nt, &nn, &mm);
        while (getc(nfp) != '\n');
        place_p->lbound = nt;
        place_p->rubound = nn - nt;
#ifdef DEBUGinit_pn
        fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                ii, place_p->cods.mask, place_p->cods.shift, place_p->cods.num_byte);
        fprintf(stderr, "            lbound=%d rubound=%d notok=%d link=%d\n",
                place_p->lbound, place_p->rubound, mm, place_p->cods.link);
#endif
    }

    /* read groups */
    for (nn = sub_num, ii = 0 ;
            ii++ < group_num; nn++) {
        fscanf(nfp, "%d %d\n", &jj, &kk);
        for (jj = 0 ; ++jj <= kk ;) {
            fscanf(nfp, " %d %d %d %d\n", &n1, &n2, &n3, &n4);
            for (ll = 0 ; ll++ < n2 ;) {
                fscanf(nfp, " %d %d %d\n", &nt, &mm, &ss);
                trans_p = t_list + nt;
            }
#ifdef DEBUGinit_pn
            fprintf(stderr, "    ECS #%d, Byte #%d  nn=%d\n",
                    ii, jj, nn);
#endif
        }
    }

    last_timed = 0;
    /* read transitions */
    for (trans_p = t_list + 1, nt = 0 ; nt++ < trans_num ; trans_p++) {
        trans_p->enabled = 0;
        fscanf(nfp, "%d %d %d %d %d %d %d", &ii, &jj, &ns, &kk,
               &ll, &nb, &nc);
        while (getc(nfp) != '\n');
        trans_p->tnum = ii;
        t_list[ii].sort_index = nt;
        trans_p->ecs = jj;
        if (! jj) {
            if (last_timed < nt)
                last_timed = nt;
        }
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
        trans_p->add_l = usp =
                             (unsigned short *)ecalloc(na + 1, sizeof(unsigned short));
        *(usp += na) = 0;
        while (na--) {
            fscanf(nfp, "%d", &jj);
            *(--usp) = jj;
        }
        fscanf(nfp, "\n %d", &nc);
        trans_p->test_l = usp =
                              (unsigned short *)ecalloc(nc + 1, sizeof(unsigned short));
        *(usp += nc) = 0;
        while (nc--) {
            fscanf(nfp, "%d", &jj);
            *(--usp) = jj;
        }
        while (getc(nfp) != '\n');
    }

    (void)fclose(nfp);

    rootm.array = NULL;
    toptan = 0;

    /* read transition firing times */

    sprintf(filename, "%s.gmt", netname);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }

    fscanf(nfp, "%d\n", &nt);
    if (nt != trans_num) {
        fprintf(stderr,
                "Simulation Engine ERROR: inconsistency in '.gmt' file!\n");
        exit(1);
    }
    for (nt = 0 ; nt++ < trans_num ;) {
        fscanf(nfp, "%d", &ii);
        trans_p = t_list + ii;
        trans_p->policy = POLICY_DIS_RANDOM; /* Default setting for exponential */
        trans_p->timing = TIMING_ERLANG;;
        fscanf(nfp, "%d %d %lg %d", &ecs, &pri, &rw, &num_clause);
#ifdef DEBUGinit_pn
        fprintf(stderr, "    reading transition %d, ecs=%d, pri=%d\n", ii, ecs, pri);
#endif
        if (num_clause >= 0)
            while ((cc = getc(nfp)) != '\n');
        else {
            int kk, pol, dis;
            double dd;

            fscanf(nfp, "%d %d %d\n", &kk, &pol, &dis);
            trans_p->policy = pol;
            trans_p->timing = dis;;
            if ((dis & TIMING_DISTRIBUTION) == TIMING_ERLANG) {
                fscanf(nfp, "%lg\n", &dd);
                trans_p->f_time.mean_t = dd;
            }
            else {
                int dim = kk + kk;
                double *dp;

                if ((dis & TIMING_DISTRIBUTION) == TIMING_LINEAR)
                    dim += kk;
                dp = trans_p->f_time.f_list =
                         (double *)ecalloc(dim, sizeof(double));
                while (kk--) {
                    fscanf(nfp, "%lg", &dd); *(dp++) = dd;
                    if ((dis & TIMING_DISTRIBUTION) == TIMING_LINEAR) {
                        fscanf(nfp, " %lg", &dd); *(dp++) = dd;
                    }
                    fscanf(nfp, " %lg\n", &dd); *(dp++) = dd;
                }
            }
        }
        if (pri == DETERMINISTIC_TRANS) {
            trans_p->pri = 0;
            if (num_clause >= 0) {   /* Default setting */
                trans_p->timing = TIMING_DISCRETE;
                trans_p->policy = /* Enabling Memory, FCFS */
                    POLICY_DIS_FIRST_DRAWN;
            }
        }
        else
            trans_p->pri = pri;
        if (num_clause > 0) {
            struct Clause_def *clp;
            struct Cond_def *cop;
            struct Val_def *vap;
            char cc;
            int nco, nva, nn;
            double dd;

#ifdef DEBUGinit_pn
            fprintf(stderr, "        m-d with %d clauses\n", num_clause);
#endif
            trans_p->md_p.mdc = clp = (struct Clause_def *)ecalloc(num_clause,
                                      sizeof(struct Clause_def));
            while (num_clause--) {
                while ((cc = getc(nfp)) == ' ');
                fscanf(nfp, "%d %d\n", &nco, &nva);
#ifdef DEBUGinit_pn
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
#ifdef DEBUGinit_pn
                        fprintf(stderr, "              c type=%c p1=%d top2=%c op2=%d\n", cop->type,
                                cop->p1, cop->top2, cop->op2);
#endif
                    }
                    else {
#ifdef DEBUGinit_pn
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
#ifdef DEBUGinit_pn
                            fprintf(stderr, "              o p %d\n", nn);
#endif
                        }
                        else {
                            fscanf(nfp, "%lg\n", &dd);
                            vap->val.real = dd;
#ifdef DEBUGinit_pn
                            fprintf(stderr, "              o r %1.6lg\n", dd);
#endif
                        }
                    }
                    else {
                        vap->op = cc;
#ifdef DEBUGinit_pn
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
#ifdef DEBUGinit_pn
        fprintf(stderr, "        ecs=%d endep=%d rate=%1.6g\n", ecs, jj, rw);
#endif
        trans_p->endep = jj;
        if (ecs) {
            trans_p->f_time.mean_t = rw;
        }
        else {
            if (num_clause >= 0)
                trans_p->f_time.mean_t = 1.0 / rw;
            if (jj) {
                double *d_p;
                trans_p->md_p.prob = d_p = (double *)ecalloc(jj + 2,
                                           sizeof(double));
                *(++d_p) = rw;
                while (jj--) {
                    fscanf(nfp, "%lg", &rw);
#ifdef DEBUGinit_pn
                    fprintf(stderr, "            coef=%1.6g\n", rw);
#endif
                    *(++d_p) = rw;
                }
            }
        }
        while (getc(nfp) != '\n');
    }

    (void)fclose(nfp);

    if (record_traces) {
        char linebuf[LINEMAX];
        int n1, n2, n3, n4, n5, n6;
#define VBAR '|'

        sprintf(filename, "%s.net", netname);
        if ((nfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
        /* skip first line containing '|0|' */
        fgets(linebuf, LINEMAX, nfp);
        /* skip comment */
        for (; ;) {
            fgets(linebuf, LINEMAX, nfp);
            if (linebuf[0] == VBAR) break;
        }
        /* read number of objects in the net */
        fscanf(nfp, "f %d %d %d", &n1, &n2, &n3);
        fscanf(nfp, "%d %d %d\n", &n4, &n5, &n6);
        while (getc(nfp) != '\n');
        /* skip marking parameters */
        while (n1--)
            while (getc(nfp) != '\n');
        /* skip places */
        while (n2--)
            while (getc(nfp) != '\n');
        /* skip rate parameters */
        while (n3--)
            while (getc(nfp) != '\n');
        /* skip groups */
        while (n5--)
            while (getc(nfp) != '\n');
        /* read transition names */
        for (n1 = 0 ; n1 < n4 ; n1++) {
            getname(linebuf);
            sprintf(filename, "%s.T_%s", netname, linebuf);
            if ((traces_f_a[n1] = fopen(filename, "w")) == NULL) {
                fprintf(stderr, can_t_open, filename, 'w');
                exit(1);
            }
            fscanf(nfp, "%*g %*d %*d %d", &n2);
            while (getc(nfp) != '\n');
            /* skip input arcs */
            while (n2--) {
                fscanf(nfp, "%*d %*d %d", &n3);
                while (n3-- >= 0)
                    while (getc(nfp) != '\n');
            }
            fscanf(nfp, "%d\n", &n2);
            /* skip output arcs */
            while (n2--) {
                fscanf(nfp, "%*d %*d %d", &n3);
                while (n3-- >= 0)
                    while (getc(nfp) != '\n');
            }
            fscanf(nfp, "%d\n", &n2);
            /* skip inhibitor arcs */
            while (n2--) {
                fscanf(nfp, "%*d %*d %d", &n3);
                while (n3-- >= 0)
                    while (getc(nfp) != '\n');
            }
        }
        (void)fclose(nfp);
    }

    enab_list = NULL;

#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init_pn\n");
#endif
}


void getname(char   *name_pr) {
#define BLANK ' '
#define EOLN  '\0'
    unsigned            i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != (char)0 &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}

