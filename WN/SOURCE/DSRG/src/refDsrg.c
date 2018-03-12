#include "refDsrg.h"
#include <string.h>
extern int                  nb_sr;
extern int                  nb_sm;
extern FILE                *dsrg ;
extern FILE                *off_ev;
extern FILE                *off_sr;
extern FILE                *resultat;
extern TO_MERGEP           *MERG_ALL;
extern STORE_STATICS_ARRAY  Sym_StaticConf;
extern Result_p             enabled_head;
extern int                  cur_priority;
extern char                 cache_string [];



extern void      load_partial_dtmc(unsigned long nb_sm,
                                   FILE *dsrg,
                                   pDtmcN *head,
                                   pDtmcN *headinc,
                                   pDtmcN **mtx_el,
                                   int *nb_mtx_el);
extern void      init_global_variables(char *net_name);
extern void      event_add(pDtmcN nd,
                           pDtmcA ev,
                           int InOrOut);
extern double    Transition_Rate(Event_p ev);
extern pDtmcA    add_ev(pDtmcN sr, int v, pDtmcN
                        ds, int inc, int treated,
                        double pr);
extern Result_p  trait_gurded_transitions(Result_p enabled_head,
        PART_MAT_P s_prop_Part);
extern int       adjust_priority(int cur_priority,
                                 Result_p enabled_head);
extern Event_p   get_new_event(int tr);
extern pDtmcN    create_new_node(pDtmcN nd, MarkAsEventp li,
                                 int EVENTS, int gr);
extern pDtmcN    add_incl(pDtmcN head, pDtmcN node);
extern int       get_refined_eventualities_prob(TYPE_P **PM_mark ,
        TYPE_P NbElPM_mark,
        TYPE_P **PM , TYPE_P NbElPM,
        pDecomp   *EVENT_ARRAY_ ,
        int *SIZE_OF_EVENT_ARR_);
extern void      GROUP_MARKINGS_MC_PART(Tree_Esrg_p EventArr,
                                        int nb,
                                        STORE_STATICS_ARRAY ASYM_CONF,
                                        STORE_STATICS_ARRAY SYM_CONF,
                                        char *** MTCL, int gr);

pDtmcN head = NULL;
pDtmcN headinc = NULL;
int NUMBER_OF_MARKINGS = 0;
int NUMBER_OF_BLOCKS = 0;

int SelfLoop = false;
pDtmcN **mtx_el = NULL;
int  *nb_mtx_el  = NULL;
int  *nb_block   = NULL;
pDecomp tmp = NULL;
int nb = 0;

// Compare "s_string" to the ones in
// the "sm" array.
int
search_string_marking(pDecomp sm, int sm_size,
                      char *s_string, int s_size) {

    int i;
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    for (i = 0; i < sm_size; i++)
        if (s_size == sm[i].length_for_prob
                && sm[i].marking_for_prob) {

            cp = sm[i].marking_for_prob ;
            pp = s_string ;

            for (ii = s_size; ii; ii--) {
                op1 = *cp; cp++;
                op2 = *pp; pp++;
                if (op1 != op2)
                    break;
            }

            if (ii == 0) return i;
        }

    return -1;;
}

int
copy_cache_string(char **new_string) {
    int i;

    (*new_string) = (char *)calloc(length, sizeof(char));

    for (i = 0; i < length; i++)
        (*new_string)[i] = cache_string[i];
    return length;
}

// construction of the local
// color partition needed to
// get ordinary states.
void
get_worst_partition(TYPE_P *** pm,
                    TYPE_P *nbpm) {
    int i, j;

    (*nbpm) = (TYPE_P)calloc(ncl, sizeof(TYPE));
    (*pm) = (TYPE_P **)calloc(ncl, sizeof(TYPE_P *));

    for (i = 0; i < ncl; i++) {
        (*pm)[i] = (TYPE_P *)calloc(tabc[i].card, sizeof(TYPE_P));
        for (j = 0; j < tabc[i].card; j++)
            (*pm)[i][j] = (TYPE_P)AllocateBV();
    }

    for (i = 0; i < ncl; i++) {
        for (j = 0; j < tabc[i].card; j++)
            Add((*pm)[i][j], j + 1);
        (*nbpm)[i] = tabc[i].card;
    }
}

// Test enabling of transitions in
// "trans" w.r.t. the current loaded
// symbolic marking.
// PS :if trans ==NULL then test
//     all transitions
int
test_enabling(int *trans,
              int nb_trans) {
    int j, pri = 0;


    PART_MAT_P s_prop_Part  = (PART_MAT_P) malloc(sizeof(PART_MAT));
    get_worst_partition(& s_prop_Part->PART_MAT,
                        & s_prop_Part->NbElPM);
    enabled_head = NULL;

    if (nb_trans)
        for (j = 0; j < nb_trans; j++)
            my_en_list(trans[j] + 1);
    else
        my_initialize_en_list();

    enabled_head = trait_gurded_transitions(enabled_head, s_prop_Part);
    if (enabled_head) {
        pri = adjust_priority(0/*not signif.*/, enabled_head);
        //if (s->marking->pri < s->max_pri)
        //  s->marking->pri =  s->max_pri;
    }
    FreePartMAt(s_prop_Part->PART_MAT,
                s_prop_Part->NbElPM);
    free(s_prop_Part);

    return pri;

}

// load the symmetrical representation
// from the ".mark" file using the traditional
// params. stored  in the ".off_sr" file.
unsigned long
load_sym_rep(FILE *fd, int pos,
             unsigned long fpos,
             unsigned long   *mark_pos,
             unsigned long *length,
             unsigned long *d_ptr) {
    int i, ps = 0;


    fseek(fd, fpos, SEEK_SET);
    fread(CACHE_STRING_ESRG, 1, 100, fd);
    LP_ESRG = CACHE_STRING_ESRG;
    CHAR_LOAD_COMPACT(mark_pos);
    CHAR_LOAD_COMPACT(length);
    CHAR_LOAD_COMPACT(d_ptr);

    return fpos;

}


// load an eventuality from the .event
// file, using the "pos" position in the "off_ev"
// file to get the position "d_ptr" and the length
// "length" of the eventuality.

unsigned long
load_event(FILE *fd,
           int pos,
           unsigned long fpos,
           unsigned long *sr,
           unsigned long *llength,
           unsigned long *ld_ptr,
           TYPE_P *** PM,
           TYPE_P   *NB_PM) {

    int i, cl, sc, k, ps = 0;

    (*PM)    = (TYPE_P **) calloc(ncl, sizeof(TYPE_P *));
    (*NB_PM) = (TYPE_P) calloc(ncl, sizeof(TYPE));


    fseek(fd, fpos, SEEK_SET);
    fread(CACHE_STRING_ESRG, 1, 100, fd);
    LP_ESRG = CACHE_STRING_ESRG;

    CHAR_LOAD_COMPACT(sr);
    for (cl = 0; cl < ncl; cl++) {
        unsigned long nbst;
        CHAR_LOAD_COMPACT(&nbst);
        (*NB_PM)[cl] = (TYPE)nbst;
        (*PM)[cl] = (TYPE_P *)calloc((*NB_PM)[cl], sizeof(TYPE_P));
        for (sc = 0; sc < (int)(*NB_PM)[cl]; sc++) {
            (*PM)[cl][sc] = (TYPE_P)AllocateBV();
            for (k = 0; k < MAX_IND; k++) {
                unsigned long v;
                CHAR_LOAD_COMPACT(&v);
                (*PM)[cl][sc][k] = (TYPE)v;
            }
        }
    }
    CHAR_LOAD_COMPACT(ld_ptr);
    CHAR_LOAD_COMPACT(llength);


    return fpos;
}

void
load_tmp_symb_marking(FILE *fsr,
                      FILE *fev,
                      pDtmcN nd,
                      STORE_STATICS_ARRAY
                      old_StaticConf,
                      TO_MERGEP *MERG_ALL,
                      int k,
                      int cnz) {

    unsigned long sr;
    unsigned long llength;
    unsigned long ld_ptr;
    unsigned long mark_pos;

    (void)load_sym_rep(fsr, nd->sr, nd->fsrpos,
                       &mark_pos, &llength,  &ld_ptr);

    // Change the current "tabc"
    // by the symmetric one ,then load
    // the symmetric symbolic state.
    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    string_to_marking(mark_pos, ld_ptr, llength);

    GET_TEMP_EVENTUALITIE_FROM_FILE(nd->cur[k].marking->marking_as_string,
                                    nd->cur[k].marking->length ,
                                    TEMP_EVENT);
    COPY_CACHE(nd->cur[k].marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);

    NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot,
                     MERG_ALL, old_StaticConf, RESULT);

    if (cnz) SPECIAL_CANISATION();
}


// Computation of ordinary states
// represented by a symbolic state
void
get_refined_markings(FILE *fsr,
                     FILE *fev,
                     pDtmcN nd,
                     PART_MAT_P s_prop_Part,
                     pDecomp  *ordinary,
                     int *size_ordinary) {
    unsigned long sr;
    unsigned long llength;
    unsigned long ld_ptr;
    unsigned long mark_pos;

    TYPE_P **PM = NULL;
    TYPE_P NB_PM = NULL;

    int ev_pos = nd->st;

    if (nd->cur) return;

    // Load params of "ev_pos"
    // eventuality from the "fev" file
    (void)load_event(fev, ev_pos, nd->fstpos,
                     &sr, &llength, &ld_ptr, &PM, &NB_PM);
    GET_EVENTUALITIE_FROM_FILE(ld_ptr, llength);
    COPY_CACHE(llength);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);

    // Compute all eventualities
    // reffined with respespect to
    // PM |-| PART_MAT .
    get_refined_eventualities_prob(PM , NB_PM ,
                                   s_prop_Part->PART_MAT,
                                   s_prop_Part->NbElPM,
                                   ordinary ,
                                   size_ordinary);
    int i;
    for (i = 0; i < (*size_ordinary); i++) {
        (*ordinary)[i].marking->ordinary = 1;
        (*ordinary)[i].marking_for_prob = NULL;
        (*ordinary)[i].length_for_prob = 0;
    }

    FreePartMAt(PM, NB_PM);
}

void
get_ref_with_compare(FILE *off_sr,
                     FILE *off_ev,
                     pDtmcN nd,
                     int flg) {

    if (nd->cur) return;

    int i, j, k, tr, ord = 0;
    unsigned long sr;
    unsigned long llength;
    unsigned long ld_ptr;
    unsigned long mark_pos;


    /********* load partion of the symbolic marking s **********/
    STORE_STATICS_ARRAY old_StaticConf = NULL;
    PART_MAT_P s_prop_Part  = (PART_MAT_P) malloc(sizeof(PART_MAT));
    get_worst_partition(& s_prop_Part->PART_MAT, &  s_prop_Part->NbElPM);

    old_StaticConf = (STORE_STATICS_ARRAY)NewTabc__(s_prop_Part->PART_MAT,
                     s_prop_Part->NbElPM,
                     MTCL);
    POP_STATIC_CONF(old_StaticConf, &tabc);
    MERG = MERG_ALL = (TO_MERGEP *)TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG_ALL, tabc, num);
    /*********************************************************/
    if (flg == 1) {
        tmp = NULL; nb = 0;
    }
    get_refined_markings(off_sr, off_ev, nd,
                         s_prop_Part, &(nd->cur), &(nd->nbcur));


    for (k = 0; k < nd->nbcur; k++) {
        load_tmp_symb_marking(off_sr, off_ev, nd, old_StaticConf, MERG_ALL, k, 1);
        if (search_string_marking(tmp, nb, cache_string, length) != EQUAL) {

            tmp = (pDecomp)realloc(tmp, (nb + 1) * sizeof(Decomp));
            tmp[nb].marking         = nd->cur[k].marking;
            tmp[nb].enabled_head    = nd->cur[k].enabled_head;
            tmp[nb].marking->pri    = nd->cur[k].marking->pri ;
            tmp[nb].Head_Next_Event = nd->cur[k].Head_Next_Event;
            tmp[nb].length_for_prob =
                copy_cache_string(& (tmp[nb].marking_for_prob));
            nb++;
        }
        else
            free(nd->cur[k].marking);
    }

    if (flg == 1) {
        free(nd->cur);
        nd->cur = tmp;
        nd->nbcur = nb;
        tmp = NULL;
        nb = 0;
    }

    if (flg == 3) {
        free(nd->cur);
        nd->cur = NULL;
        nd->nbcur = 0;
    }

    FreePartMAt(s_prop_Part->PART_MAT,
                s_prop_Part->NbElPM);
    free(s_prop_Part);
    FreeStoreStructs(old_StaticConf);
    FreeMerg(MERG_ALL);
    FREE_ALL_LMS_ELEMS();
}


// Updating the input arcs for each eventuality
// of "ndd" w.r.t to those obtained by "nds".
void
get_probs_in(pDtmcN nds,
             pDtmcN ndd) {

    int i, j;

    if (!(ndd->nbinforcur)) {
        free(nds->gl_out);
        nds->gl_out = NULL;
        return;
    }

    for (i = 0; i < ndd->nbcur; i++)
        for (j = 0; j < ndd->nbinforcur[i]; j++)
            if (ndd->inforcur[i][j]->sr == nds) {
                ndd->inforcur[i][j]->pb /=
                    nds->gl_out[ndd->inforcur[i][j]->ev];
            }
    free(nds->gl_out);
    nds->gl_out = NULL;
}

// Computing the successors and probs.
// of "nds" w.r.t "ndd".
// PS: "inOrOut==1" => input are treated
//     "inOrOut==2" => output are treated
void
compute_arc_probs(FILE *off_sr,
                  pDtmcN nds,
                  pDtmcN ndd) {

    int tr, marking_pri, j, i;
    Result_p current_transition;
    Result_p next_transition;
    Event_p ev_p, nev_p, copy_of_ev_p;
    STORE_STATICS_ARRAY old_StaticConf = NULL  ;

    /********* load partion of the symbolic marking s **********/
    PART_MAT_P s_prop_Part  = (PART_MAT_P) malloc(sizeof(PART_MAT));
    get_worst_partition(& s_prop_Part->PART_MAT,
                        & s_prop_Part->NbElPM);

    old_StaticConf = (STORE_STATICS_ARRAY)
                     NewTabc__(s_prop_Part->PART_MAT,
                               s_prop_Part->NbElPM, MTCL);
    POP_STATIC_CONF(old_StaticConf, &tabc);
    MERG = MERG_ALL = (TO_MERGEP *)TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG_ALL, tabc, num);
    /*********************************************************/

    if (!(nds->gl_out)) {
        nds->gl_out = (double *)calloc(nds->nbcur,
                                       sizeof(double));
    }

    for (i = 0; i < nds->nbcur; i++) {
        //  change the current tabc by the one of s
        // and load s symbolic state.
        load_tmp_symb_marking(off_sr, off_ev, nds,
                              old_StaticConf,
                              MERG_ALL, i, 1);
        // Compute enabled events
        // from the "trans" array.
        int pri = test_enabling(NULL, 0);

        current_transition =
            nds->cur[i].enabled_head = enabled_head;

        // Loop to compute successors for
        // transtions that are in the "tofire" array.
        while (current_transition != NULL) {

            next_transition = current_transition->next;
            tr = GET_TRANSITION_INDEX(current_transition->list);

            if (tabt[tr].pri == pri) {

                copy_of_ev_p = get_new_event(tr);
                ev_p = current_transition->list;

                while (ev_p != NULL) {

                    // Store the global output rate
                    // from the i^th eventuality.
                    nds->gl_out[i] += ev_p->ordinary_instances *
                                      Transition_Rate(ev_p);

                    nev_p = ev_p->next;
                    copy_event(copy_of_ev_p, ev_p);

                    fire_trans(ev_p);
                    SPECIAL_CANISATION();


                    if ((j = search_string_marking(ndd->cur, ndd->nbcur,
                                                   cache_string, length)) != -1) {

                        if (!(ndd->inforcur)) {
                            ndd->inforcur =
                                (pDtmcA **)calloc(ndd->nbcur, sizeof(pDtmcA *));
                            ndd->nbinforcur = (int *)calloc(ndd->nbcur, sizeof(int));
                        }

                        ndd->inforcur[j] = (pDtmcA *)realloc(ndd->inforcur[j],
                                                             ((ndd->nbinforcur[j]) + 1)
                                                             * sizeof(pDtmcA));
                        ndd->inforcur[j][ndd->nbinforcur[j]] =
                            add_ev(nds, i, NULL, 0, 0, ev_p->ordinary_instances *
                                   Transition_Rate(ev_p));
                        (ndd->nbinforcur[j])++;
                    }

                    // Re-load the source symolic marking.
                    // for the next firing.
                    load_tmp_symb_marking(off_sr, off_ev, nds,
                                          old_StaticConf,
                                          MERG_ALL, i, 1);
                    ev_p = nev_p;
                }
                dispose_old_event(copy_of_ev_p);
            }
            current_transition = next_transition;
        }
        my_garbage_collect(& nds->cur[i]);
    }
    get_probs_in(nds, ndd);

    FreePartMAt(s_prop_Part->PART_MAT,
                s_prop_Part->NbElPM);
    free(s_prop_Part);
    FreeStoreStructs(old_StaticConf);
    FreeMerg(MERG_ALL);
    FREE_ALL_LMS_ELEMS();

}

double
probs_for_a_node(pDtmcN nd,
                 pDtmcA *p,
                 int nb) {

    int i;
    double pb = 0;
    for (i = 0; i < nb; i++) {
        if (p[i]->sr == nd)
            pb += p[i]->pb;
    }

    return pb;
}

int
compare_prob(pDtmcN nd, int i, int s) {

    int k;
    for (k = 0; k < nd->nbinforcur[i]; k++)
        if (probs_for_a_node(nd->inforcur[i][k]->sr,
                             nd->inforcur[i],
                             nd->nbinforcur[i]) !=
                probs_for_a_node(nd->inforcur[i][k]->sr,
                                 nd->inforcur[s],
                                 nd->nbinforcur[s]))
            return 0;

    for (k = 0; k < nd->nbinforcur[s]; k++)
        if (probs_for_a_node(nd->inforcur[s][k]->sr,
                             nd->inforcur[i],
                             nd->nbinforcur[i]) !=
                probs_for_a_node(nd->inforcur[s][k]->sr,
                                 nd->inforcur[s],
                                 nd->nbinforcur[s]))
            return 0;

    return 1;
}

int
pre_condition_grouping_in(pDtmcN nd) {

    int i, k, s, t, gr = -1;

    for (i = 0; i < nd->nbcur; i++) {
        int found = 0;
        for (s = 0; s < i && !found; s++)
            if (compare_prob(nd, i, s)) {
                nd->cur[i].gr = nd->cur[s].gr;
                found = 1;
            }

        if (!found) {
            gr++;
            nd->cur[i].gr = gr;
        }
    }
    return gr;
}

int
pre_condition_grouping_out(pDtmcN nd) {

    int i, s;
    nd->cur[0].gr = 0;
    for (i = 1; i < nd->nbcur; i++)
        for (s = 0; s < i; s++)
            if (!compare_prob(nd, i, s))
                return 1;
    return 0;
}

int
get_index_from_group(pDtmcN nd, int gr) {

    int i;
    for (i = 0; i < nd->nbcur; i++)
        if (nd->cur[i].gr == gr)
            return i;
}


pDtmcN
grouping_and_store(FILE *fsr, FILE *fev, pDtmcN nd) {

    pDtmcN head = NULL;

    int gr = pre_condition_grouping_in(nd);
    int nbgr; int i;

    unsigned long llength;
    unsigned long ld_ptr;
    unsigned long mark_pos;

    STORE_STATICS_ARRAY gr_s_prop_StaticConf,
                        s_prop_StaticConf;
    PART_MAT_P  s_prop_Part = (PART_MAT_P) malloc(sizeof(PART_MAT));

    get_worst_partition(&(s_prop_Part->PART_MAT),
                        &(s_prop_Part->NbElPM));

    s_prop_StaticConf = (STORE_STATICS_ARRAY)NewTabc__(s_prop_Part->PART_MAT,
                        s_prop_Part->NbElPM, MTCL);


    (void)load_sym_rep(fsr, nd->sr, nd->fsrpos,
                       &mark_pos, &llength,  &ld_ptr);
    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    string_to_marking(mark_pos, ld_ptr, llength);

    nb_block[nd->sr] = gr + 1;

    for (nbgr = 0; nbgr <= gr; nbgr++) {
        GROUP_MARKINGS_MC_PART(nd->cur, nd->nbcur,
                               s_prop_StaticConf,
                               Sym_StaticConf,
                               MTCL, nbgr);
        int j;
        for (j = 0; j < NbResList; j++) {
            TO_MERGEP *merg_group = NULL;
            gr_s_prop_StaticConf = (STORE_STATICS_ARRAY)
                                   NewTabc__(ResultList[j]->PART_MAT,
                                             ResultList[j]->NbElPM, MTCL);
            POP_STATIC_CONF(gr_s_prop_StaticConf, &tabc);
            merg_group = (TO_MERGEP *)TO_MERGE(ncl, tabc);
            GROUPING_ALL_STATICS(merg_group, tabc, num);
            MarkAsEventp list = ResultList[j]->list;

            while (list) {
                INIT_RESULT_STRUCT(list->Event);
                TO_STORE_INIT_ARRAYS(gr_s_prop_StaticConf);
                EVENTUALITIE_TO_STRING(Sym_StaticConf);
                ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);

                pDtmcN el = create_new_node(nd, list, nb_sm + 1, nbgr);
                nb_sm++; el->nxt_list = head; head = el;
                NUMBER_OF_MARKINGS++;

                LP_ESRG = CACHE_STRING_ESRG;
                fseek(fev, 0, SEEK_END);

                el->fstpos = ftell(fev);
                el->fsrpos = nd->fsrpos;

                MY_CHAR_STORE((unsigned long)nd->sr);
                int i, k;
                for (i = 0; i < ncl; i++) {
                    MY_CHAR_STORE((unsigned long)ResultList[j]->NbElPM[i]);
                    int s;
                    for (s = 0; s < ResultList[j]->NbElPM[i]; s++)
                        for (k = 0; k < MAX_IND; k++)
                            MY_CHAR_STORE((unsigned long)
                                          ResultList[j]->PART_MAT[i][s][k]);
                }

                MY_CHAR_STORE((unsigned long)FILE_POS_PTR);
                MY_CHAR_STORE((unsigned long)LEGTH_OF_CACHE);
                fwrite(CACHE_STRING_ESRG, 1, LP_ESRG - CACHE_STRING_ESRG, fev);

                list = list->next;
            }

            FreePartMAt(ResultList[j]->PART_MAT,
                        ResultList[j]->NbElPM);
            Free_ResList(j);
            FREE_ALL_LMS_ELEMS();
            FreeMerg(merg_group);
            FreeStoreStructs(gr_s_prop_StaticConf);
        }
        Free_DSC_SSC();
        free(ResultList);
        ResultList = NULL;
        NbResList = 0;
    }

    FreePartMAt(s_prop_Part->PART_MAT,
                s_prop_Part->NbElPM);
    free(s_prop_Part);
    FreeStoreStructs(s_prop_StaticConf);

    return head;
}


// Updating the inputs
// of the node "nd"
// PS: we assume that "li"
// reffinement set is computed.
pDtmcN
Update_in_nodes(FILE *fsr,
                FILE *fev,
                pDtmcN li,
                int nbidx,
                pDtmcN *idx) {

    int k, i;

    for (k = 0; k < nbidx ; k++) {

        pDtmcN  nd = idx[k];
        pDtmcA *in = nd->in;

        for (i = 0; i < nd->nbin; i++) {

            pDtmcA ni = in[i];

            if (!ni) continue ;

            if (li->first && ni->sr->sr == ni->ds->sr) {
                SelfLoop = true;
                continue;
            }

            if (!ni->treated) {

                marking_treated_ev(in, i, nd->nbin, ni->sr, 0);
                get_ref_with_compare(fsr, fev, ni->sr, 1);
                compute_arc_probs(fsr, ni->sr, li);

                if (li != ni->sr) {
                    local_free_temp_ord(ni->sr->cur, ni->sr->nbcur);
                    free_a_ref_elements(ni->sr);
                }

                if (!li->first && ni->sr == ni->ds) {
                    SelfLoop = true;
                }

                eleminate_arcs(ni->sr, nd, 1);
            }
        }
    }
    return grouping_and_store(fsr, fev, li);
}

pDtmcN
add_incl2(pDtmcN head,
          pDtmcN node) {

    if (node->first)
        return (add_incl(head, node));
    else {
        pDtmcN hd = head;
        while (hd) {
            if (hd->st == node->st)
                return head;
            hd = hd->nxt_inc;
        }
        node->nxt_inc = head;
        return node;
    }
}

pDtmcN
Update_out_nodes(FILE *fsr,
                 FILE *fev,
                 pDtmcN nd,
                 int nbidx,
                 pDtmcN *idx,
                 pDtmcN head) {

    int       k;
    pDtmcN    li   = head;

    for (k = 0; k < nbidx; k++) {

        pDtmcN oldnd = idx[k];
        pDtmcA  *out = oldnd->out;
        int      nb = oldnd->nbout;
        int i;

        for (i = 0; i < nb; i++) {

            pDtmcA ni = out[i];

            if (!ni) continue;

            if (!(ni->treated)) {

                eleminate_arcs(ni->ds, oldnd, 0);
                marking_treated_ev(out, i, oldnd->nbout, ni->ds, 1);
                get_ref_with_compare(fsr, fev, ni->ds, 1);
                compute_arc_probs(fsr, nd, ni->ds);

                if (ni->ds->inforcur) {
                    double pr = 0;
                    pDtmcA n;

                    if (pre_condition_grouping_out(ni->ds) == 0) {

                        int   idx = get_index_from_group(ni->ds, 0);
                        double pr = (double) ni->ds->ord / (double)nd->ord *
                                    (double) probs_for_a_node(nd,
                                                              ni->ds->inforcur[idx],
                                                              ni->ds->nbinforcur[idx]);
                        n = add_ev(nd, -1, ni->ds, 0, 0, pr);
                    }
                    else {
                        n = add_ev(nd, -1, ni->ds, 1, 0, pr);
                        li = add_incl2(head, ni->ds);
                    }

                    event_add(ni->ds, n, 0);
                    event_add(nd, n, 1);
                }
                free_temp_in(ni->ds);
            }

            if (ni->ds != nd)
                free_temp_cur(ni->ds);
        }

        for (i = 0; i < nb; i++)
            if (out[i])
                out[i]->treated = 0;
    }

    return li;
}

void
treat_loop(pDtmcN split, int nbidx, pDtmcN *idx) {

    int i, k, sr = idx[0]->sr;

    for (k = 0; k < nbidx ; k++) {
        pDtmcN nd = idx[k];
        for (i = 0; i < nd->nbout; i++)
            if (nd->out[i] &&
                    ((nd->first && nd->out[i]->ds->sr == nd->sr) ||
                     (!nd->first && nd->out[i]->ds == nd)
                    )
               ) {
                nd->out[i] = NULL;
            }
    }

    pDtmcN nd = idx[0];
    pDtmcN sp = split;

    while (sp) {
        pDtmcA ev = add_ev(nd, -1, sp, 0, 0, 0);
        event_add(nd, ev, 1);
        sp = sp->nxt_list;
    }
}

pDtmcN *
get_idx_of_sm(pDtmcN el) {
    return mtx_el[el->sr];
}

pDtmcN
ref_algorithm(FILE *fsr,
              FILE *fev,
              pDtmcN Inc_List,
              pDtmcN Head) {

    pDtmcN li = Inc_List;
    pDtmcN split;
    pDtmcN loop;
    int i;
    pDtmcN *idx;
    int nbidx;

    while (li) {
        //  printf("\n debut : %d", li->sr);
        SelfLoop = false;
        pDtmcN hdi = li->nxt_inc;

        tmp = NULL; nb = 0;
        int desc = fileno(TEMP_EVENT);
        ftruncate(desc, 0);

        if (li->first) {
            idx = get_idx_of_sm(li);
            for (i = 0; i < nb_mtx_el[li->sr]; i++) {
                get_ref_with_compare(fsr, fev, idx[i], 3);
                NUMBER_OF_MARKINGS--;
            }
            li->cur = tmp;
            li->nbcur = nb;
            tmp = NULL; nb = 0;
            nbidx = nb_mtx_el[li->sr];
        }
        else {
            idx = &li;
            nbidx = 1;
            get_ref_with_compare(fsr, fev, li, 1);
            NUMBER_OF_MARKINGS--;
        }

        split = Update_in_nodes(fsr, fev, li, nbidx, idx);

        if (SelfLoop) {
            treat_loop(split, nbidx, idx);
        }

        if (Head == li) {
            Head = split;
        }

        pDtmcN hd = split;

        while (split) {
            get_ref_with_compare(fsr, fev, split, 1);
            hdi = Update_out_nodes(fsr, fev, split, nbidx, idx, hdi);
            free_temp_cur(split);
            split = split->nxt_list;
        }


        int nb = nb_mtx_el[li->sr];
        int sr = li->sr;
        int s;

        if (li->first) {

            for (s = 0; s < nb; s++) {
                free_node(idx[s]);
                nb_mtx_el[sr]--;
            }

            free(mtx_el[sr]);
            mtx_el[sr] = NULL;

            int ord = 0;
            while (hd) {
                pDtmcN   nxt = hd->nxt_list;
                mtx_el[sr] = (pDtmcN *) realloc(mtx_el[sr],
                                                (nb_mtx_el[sr] + 1) * sizeof(pDtmcN));
                mtx_el[sr][nb_mtx_el[sr]] = hd;
                ord += hd->ord;
                nb_mtx_el[sr]++;
                hd->nxt_list = NULL;
                hd = nxt;
            }
        }
        else {

            for (s = 0; s < nb; s++)
                if (li == mtx_el[sr][s]) {
                    mtx_el[sr][s] = NULL ;
                    break;
                }

            free_node(li);
            mtx_el[sr][s] = hd;
            hd = hd->nxt_list;
            mtx_el[sr][s]->nxt_list = NULL;

            int ord = 0;
            while (hd) {
                pDtmcN  nxt = hd->nxt_list;
                mtx_el[sr] = (pDtmcN *) realloc(mtx_el[sr],
                                                (nb_mtx_el[sr] + 1) * sizeof(pDtmcN));
                mtx_el[sr][nb_mtx_el[sr]] = hd;
                ord += hd->ord;
                nb_mtx_el[sr]++;
                hd->nxt_list = NULL;
                hd = nxt;
            }
        }

        //  printf("----> fin : %d", sr);
        li = hdi;
    }
    return Head;
}


int
print_symbolic_marking(char **st) {

    FILE *fd;
    int   pos;
    char *mark = "states.mark";
    fd = fopen(mark, "w+");

    write_on_srg(fd, 1);
    pos = ftell(fd);
    *st = malloc((pos + 1) * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);
    system("/bin/rm -f states.mark");

    return 0;
}
// Load a symbolic state in the
// internal structures  of
// GreatSPN and print it
char *
load_symbolic_marking(FILE *STD,
                      FILE *fsr,
                      FILE *fev,
                      pDtmcN nd) {
    unsigned long sr;
    unsigned long llength;
    unsigned long ld_ptr;
    unsigned long mark_pos;
    TYPE_P **PM = NULL;
    TYPE_P NB_PM = NULL;
    STORE_STATICS_ARRAY old_StaticConf = NULL;
    int ev_pos = nd->st;
    nd->fstpos = load_event(fev, ev_pos, nd->fstpos,
                            &sr, &llength, &ld_ptr, &PM, &NB_PM);

    // load the i'th eventuality from the file
    GET_EVENTUALITIE_FROM_FILE(ld_ptr, llength);
    COPY_CACHE(llength);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);

    /****************************************************************/
    old_StaticConf = (STORE_STATICS_ARRAY)NewTabc__(PM, NB_PM, MTCL);
    POP_STATIC_CONF(old_StaticConf, &tabc);
    MERG = MERG_ALL = (TO_MERGEP *)TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG_ALL, tabc, num);
    /****************************************************************/
    nd->fsrpos = load_sym_rep(fsr, sr , nd->fsrpos,
                              &mark_pos, &llength,  &ld_ptr);

    /**********  change the current tabc by the one of s and load s state **/
    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    string_to_marking(mark_pos, ld_ptr, llength);
    /**********************************************************************/

    /********* load partion of the symbolic marking s *********************/

    /************* instanciate the marking s with the eventuality i *******/
    NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot,
                     MERG_ALL, old_StaticConf, RESULT);
    /***********************************************************************/
    SPECIAL_CANISATION();

    char *st;
    print_symbolic_marking(&st);

    FreeStoreStructs(old_StaticConf);
    FreeMerg(MERG_ALL);
    FREE_ALL_LMS_ELEMS();
    FreePartMAt(PM, NB_PM);
    return st;
}


void raff_usage(void) {

    fprintf(stdout, "\n\n-------------------------------------------------------------\n");
    fprintf(stdout, "Usage : rdsrg netname [-p,-a]                                    \n");
    fprintf(stdout, "      -p : store the DTMC in compressed format (Used for solving)\n");
    fprintf(stdout, "      -a : store the DTMC in ascii format      (Used for drawing)\n");
    fprintf(stdout, "-----------------------------------------------------------------\n");

}

int
main(int argc, char **argv) {

    int ii,
        print = 0,
        printascii = 0,
        peresm = 0;
    int old_time, new_time;

    if (argc < 2) {
        raff_usage();
        exit(1);
    }

    for (ii = 2; ii < argc; ii++) {

        if (argv[ii][1] == 'p') print = 1;
        if (argv[ii][1] == 'a') printascii = 1;
        if (argv[ii][1] != 'p' &&
                argv[ii][1] != 'a') {
            raff_usage();
            exit(1);
        }
    }

    char **arg = (char **)calloc(3, sizeof(char *));
    arg[2] = (char *)calloc(2, sizeof(char));
    arg[0] = argv[0]; arg[1] = argv[1];
    arg[2][0] = '-'; arg[2][1] = 'r';

    time(&old_time);
    initialize(3, arg);
    init_global_variables(arg[1]);
    mtx_el     = (pDtmcN **)calloc(nb_sr, sizeof(pDtmcN *));
    nb_mtx_el  = (int *) calloc(nb_sr, sizeof(int));
    nb_block   = (int *) calloc(nb_sr, sizeof(int));
    load_partial_dtmc(nb_sm, dsrg, & head, &headinc,
                      mtx_el, nb_mtx_el);
    NUMBER_OF_MARKINGS = nb_sm;
    nb_sm = nb_sm - 1;
    pDtmcN hd = ref_algorithm(off_sr, off_ev, headinc, head);
    time(&new_time);

    int i, not_touched = 0;
    for (i = 0; i < nb_sr; i++)
        if (nb_block[i])
            NUMBER_OF_BLOCKS += nb_block[i];
        else {
            not_touched ++;
            peresm += nb_mtx_el[i];
            NUMBER_OF_BLOCKS ++;
        }

    printf("\n ***** RESULTS OF THE REFINEMENT ALGORITHEM ****\n");
    printf(" NUMBER SYMBOLIC MARKINGS IN DTMC = %d\n", NUMBER_OF_MARKINGS);
    printf(" NUMBER BLOCKS IS COMPRIZED IN [%d,%d] \n", NUMBER_OF_BLOCKS,
           NUMBER_OF_BLOCKS - not_touched + peresm);
    printf(" NUMBER NOT REFINED ESMs = %d \n", not_touched);
    printf(" TIME REQUIRED = %ds\n", new_time - old_time);


    if (print)
        store_dtmc(arg[1], mtx_el, nb_mtx_el);

    if (printascii) {
        store_dtmc(arg[1], mtx_el, nb_mtx_el);
        print_dtmc(arg[1], mtx_el, nb_mtx_el);
    }

    finalize_session();
    free(arg[2]);
    free(arg);
}


