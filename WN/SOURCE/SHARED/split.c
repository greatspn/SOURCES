# include <stdio.h>
#include "../../INCLUDE/const.h"
#ifdef SWN
#ifdef SYMBOLIC
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/ealloc.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/service.h"

extern int marcatura;


int **nofs = NULL;
int **original = NULL;
int *original_split = NULL;
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_split_data_structure() {
    /* Init initialize_split_data_structure */
    int ii;

    if (ncl != 0) {
        nofs = (int **)ecalloc(ncl, sizeof(int *));
        original = (int **)ecalloc(ncl, sizeof(int *));
        for (ii = 0; ii < ncl; ii++)
#ifdef REACHABILITY
            if (IS_UNORDERED(ii) || (IS_ORDERED(ii) && GET_STATIC_SUBCLASS(ii) == 1))
#endif
#ifdef SIMULATION
                if (IS_UNORDERED(ii))
#endif
#ifndef LIBSPOT
                    if (GET_STATIC_SUBCLASS(ii)) {
                        nofs[ii] = (int *)ecalloc(GET_STATIC_SUBCLASS(ii), sizeof(int));
                        original[ii] = (int *)ecalloc(GET_STATIC_SUBCLASS(ii), sizeof(int));
                    }
#else
#ifdef ESYMBOLIC
                {
                    nofs[ii] = (int *)ecalloc(tabc[ii].card, sizeof(int));
                    original[ii] = (int *)ecalloc(tabc[ii].card, sizeof(int));
                }
#else
                    if (GET_STATIC_SUBCLASS(ii)) {
                        nofs[ii] = (int *)ecalloc(GET_STATIC_SUBCLASS(ii), sizeof(int));
                        original[ii] = (int *)ecalloc(GET_STATIC_SUBCLASS(ii), sizeof(int));
                    }
#endif
#endif
    }
    if (max_transition_domain)
        original_split = (int *)ecalloc(max_transition_domain, sizeof(int));
}/* End initialize_split_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void get_pointer_info_not_from_scratch(Token_p start_ptr,
        Token_p add_ptr,
        int pl,
        TokInfo_p ret_ptr) {
    /* Init get_pointer_info_not_from_scratch */
    Token_p pos_ptr = NULL;
    Token_p prv_ptr = NULL;

    int domain_comp;
    int status = 0;

    domain_comp = GET_PLACE_COMPONENTS(pl);
    ret_ptr->pos = ret_ptr->prv = NULL;
    if (IS_NEUTRAL(pl)) {
        /* Posto neutro */
        ret_ptr->pos = net_mark[pl].marking;
    }/* Posto neutro */
    else {
        /* Posto colorato */
        // if(IS_FULL(pl))
        if (net_mark[pl].marking) {
            /* C'e' almeno un token */
            for (pos_ptr = start_ptr; pos_ptr != NULL ; pos_ptr = pos_ptr->next) {
                /* Per ogni token della marcatura di pl */
                status = optimized_compare_token_id(add_ptr->id , pos_ptr->id , pl);
                switch (status) {
                case EQUAL_TO     : ret_ptr->pos = pos_ptr;
                    ret_ptr->prv = prv_ptr;
                    goto ok;
                case LESSER_THAN  : ret_ptr->pos = NULL;
                    ret_ptr->prv = prv_ptr;
                    goto ok;
                case GREATER_THAN : break;
                }
                prv_ptr = pos_ptr;
                ret_ptr->prv = prv_ptr;
            }/* Per ogni token della marcatura di pl */
        }/* C'e' almeno un token */
    }/* Posto colorato */
ok: return;
}/* End get_pointer_info_not_from_scratch */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void split_of_marking(int pl,
                             int fix,
                             int src,
                             int dst) {
    /* Init split_of_marking */
    int over = FALSE;
    Token_p tok_ptr = net_mark[pl].marking;
    Token_p add_ptr = NULL;

    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    for (; tok_ptr != NULL && !over;) {
        /* Per ogni token nella marcatura di pl */
        if (tok_ptr->id[fix] == src) {
            /* Da duplicare */
            (net_mark[pl].different)++;
            add_ptr = pop_token(pl);
            optimized_init_token_values(tok_ptr->id, add_ptr, pl);
            add_ptr->id[fix] = dst;
            SET_TOKEN_MOLTEPLICITY(tok_ptr->molt, add_ptr);
            get_pointer_info_not_from_scratch(tok_ptr, add_ptr , pl , res_ptr);
            insert_token(res_ptr, add_ptr, pl);
        }/* Da duplicare */
        tok_ptr = NEXT_TOKEN(tok_ptr);
    }/* Per ogni token nella marcatura di pl */
}/* End split_of_marking */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void copy_marking_distribution(int cl,
                                      int ss,
                                      int src,
                                      int dst) {
    /* Init copy_marking_distribution */
    int pl, fix;

    for (pl = 0 ; pl < npl ; pl++) {
        /* Splitting marcatura in ogni posto */
        //if(IS_FULL(pl))
        if (net_mark[pl].marking) {
            /* Se il posto e'pieno */
            if (IS_COLOURED(pl)) {
                /* se il posto e' colorato */
                for (fix = 0 ; fix < GET_PLACE_COMPONENTS(pl) ; fix++)
                    if (GET_COLOR_COMPONENT(fix, pl) == cl)
                        split_of_marking(pl, fix, src, dst);
            } /* se il posto e' colorato */
        }/* Se il posto e'pieno */
    }/* Splitting marcatura in ogni posto */
}/* End copy_marking_distribution */
#ifdef SIMULATION
/**********************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**********************************************************/
static void fix_split_elements(Event_p instance_ptr) {
    /* Init fix_split_elements */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int pos, prv, base, n_ist, random_id;
    int cl, ss, ds;

    for (cl = ncl; cl ; cl--) {
        /* Per ogni classe */
        if (IS_UNORDERED(cl - 1)) {
            /* Classe non ordinata */
            if (tabt[tr].rip[cl - 1]) {
                /* La classe e' presente nel dominio della transizione */
                if (tabt[tr].split_type[cl - 1] == AT_FIRING_TIME) {
                    /* Si puo' fare lo split ritardato */
                    base = tabt[tr].off[cl - 1];
                    for (pos = 0 ; pos < tabt[tr].rip[cl - 1] ; pos++) {
                        /* Per ogni ripetizione nel dominio della transizione */
                        ds = GET_DYN_ID(instance_ptr, base + pos);
                        ss = get_static_subclass(cl - 1, ds);
                        n_ist = 0;
                        for (prv = 0; prv < pos ; prv++)
                            n_ist += (GET_DYN_ID(instance_ptr, base + prv) == ds);
                        random_id = get_random_integer(1, GET_CARD(cl - 1, ss, ds));
                        if (random_id > n_ist)
                            instance_ptr->split[base + pos] = ENCODE_ID(ds, n_ist);
                        else
                            instance_ptr->split[base + pos] = ENCODE_ID(ds, random_id - 1);
                    }/* Per ogni ripetizione nel dominio della transizione */
                }/* Si puo' fare lo split ritardato */
            }/* La classe e' presente nel dominio della transizione */
        }/* Classe non ordinata */
    }/* Per ogni classe */
}/* End fix_split_elements */
#endif
/**********************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**********************************************************/
static int already_split(int ds,
                         int pos,
                         int cl,
                         int tr,
                         Event_p ev_p) {
    /* Init already_split */
    int ret_value = UNKNOWN;
    int start = OFF(tr, cl);

    for (; start < pos ; start++)
        if (ds == original_split[start]) {
            ret_value = start;
            goto ret;
        }
ret: return (ret_value);
}/* End already_split */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void change_identity_of_tokens(int cl,
                                      int ss,
                                      int old_id,
                                      int new_id) {
    /* Init change_identity_of_tokens */
    int pl, k;
    Token_p tok_ptr;
    Token_p prv_ptr;
    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    for (pl = npl ; pl ; pl--) {
        /* Per ogni posto */
        // if(IS_FULL(pl - 1))
        if (net_mark[pl - 1].marking) {
            /* Posto con almeno un token */
            if (IS_COLOURED(pl - 1)) {
                /* Posto con dominio di colore */
                for (k = 0 ; k < GET_PLACE_COMPONENTS(pl - 1) ; k++)
                    if (tabp[pl - 1].dominio[k] == cl) {
                        /* La classe fa parte del dominio del posto */
                        tok_ptr = net_mark[pl - 1].marking;
                        prv_ptr = NULL;
                        for (; tok_ptr != NULL ;) {
                            /* Per ogni token della lista */
                            if (tok_ptr->id[k] == old_id) {
                                res_ptr->prv = prv_ptr;
                                res_ptr->pos = tok_ptr;
                                delete_token(res_ptr, pl - 1);
                                tok_ptr->id[k] = new_id;
                                get_new_position_of_token(tok_ptr, pl - 1, res_ptr);
                                insert_token(res_ptr, tok_ptr, pl - 1);
                            }
                            prv_ptr = tok_ptr;
                            tok_ptr = tok_ptr->next;
                        }/* Per ogni token della lista */
                    }/* La classe fa parte del dominio del posto */
            }/* Posto con dominio di colore */
        }/* Posto con almeno un token */
    }/* Per ogni posto */
}/* End change_identity_of_tokens */
/**********************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**********************************************************/
void shift_dynamic_subclasses(int  cl,  int  split_pos,  int  pos,  int  dir) {
    /* Init shift_dynamic_subclasses */
    int ii;

    if (dir == TRUE)
        for (ii = GET_NUM_CL(cl) - 1; ii > split_pos ; ii--) {
            SET_CARD(cl, 0, ii + pos, GET_CARD(cl, 0, ii));
            change_identity_of_tokens(cl, 0, ii, ii + pos);
        }
    else
        for (ii = split_pos + pos; ii <= GET_NUM_CL(cl)  ; ii++) {
            SET_CARD(cl, 0, ii - pos, GET_CARD(cl, 0, ii));
            change_identity_of_tokens(cl, 0, ii, ii - pos);
        }
}/* End shift_dynamic_subclasses */
/**********************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**********************************************************/
static void split_object(int cl,
                         int ds,
                         int split_id,
                         Event_p instance_ptr,
                         int comp) {
    /* Init split_object */
    int nds, delta;

    nds = get_object_from_dynamic(cl, ds);
    if (split_id != nds && split_id != nds + GET_CARD(cl, 0, ds) - 1) {
        /* In mezzo ad una sottoclasse */
        shift_dynamic_subclasses(cl, ds, 2, TRUE);
        if (comp != UNKNOWN)
            instance_ptr->split[comp] = ds + 1;
        delta =  GET_CARD(cl, 0, ds) - split_id + nds ;
        SET_CARD(cl, 0, ds, GET_CARD(cl, 0, ds) - delta); /* Sinistra */
        SET_CARD(cl, 0, ds + 1, 1);                     /* Centro */
        SET_CARD(cl, 0, ds + 2, delta - 1);             /* Destra */
        INCREASE_NUM_SS(cl, 0);
        INCREASE_NUM_SS(cl, 0);
        INCREASE_NUM_CL(cl);
        INCREASE_NUM_CL(cl);
        nofs[cl][0]++;
        nofs[cl][0]++;
        copy_marking_distribution(cl, 0, ds, ds + 1);
        copy_marking_distribution(cl, 0, ds, ds + 2);
    }/* In mezzo ad una sottoclasse */
    else if (split_id == nds) {
        /* A sinistra della sottoclasse */
        shift_dynamic_subclasses(cl, ds, 1, TRUE);
        if (comp != UNKNOWN)
            instance_ptr->split[comp] = ds;
        delta =  GET_CARD(cl, 0, ds) - split_id + nds ;
        SET_CARD(cl, 0, ds, 1);      /* Centro */
        SET_CARD(cl, 0, ds + 1, delta - 1); /* Destra */
        INCREASE_NUM_SS(cl, 0);
        INCREASE_NUM_CL(cl);
        nofs[cl][0]++;
        copy_marking_distribution(cl, 0, ds, ds + 1);
    }/* A sinistra della sottoclasse */
    else if (split_id == nds + GET_CARD(cl, 0, ds) - 1) {
        /* A destra della sottoclasse */
        shift_dynamic_subclasses(cl, ds, 1, TRUE);
        if (comp != UNKNOWN)
            instance_ptr->split[comp] = ds + 1;
        delta =  GET_CARD(cl, 0, ds) - split_id + nds ;
        DECREASE_CARD(cl, 0, ds, delta); /* Sinistra */
        SET_CARD(cl, 0, ds + 1, 1);      /* Centro */
        INCREASE_NUM_SS(cl, 0);
        INCREASE_NUM_CL(cl);
        nofs[cl][0]++;
        copy_marking_distribution(cl, 0, ds, ds + 1);
    }/* A destra della sottoclasse */
}/* End split_object */
/**********************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**********************************************************/
void split(Event_p  instance_ptr) {
    /* Init split */
    int i, cl, ss, ds, nds;
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int comp = GET_TRANSITION_COMPONENTS(tr);
    int split_id, rp;
    int position;

    for (cl = 0; cl < ncl; cl++)
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
                for (ss = 0 ; ss < GET_STATIC_SUBCLASS(cl); ss++) {
                    nofs[cl][ss] = 0;
                    original[cl][ss] = GET_STATIC_OFFSET(cl, ss) + num[cl][ss] - 1;
                }

#ifdef SIMULATION
    fix_split_elements(instance_ptr);
#endif
    for (i = 0 ; i < comp; i++) {
        /* per ogni componente del dominio della transizione */
        cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
#endif
#ifdef REACHABILITY
            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
            {
                /* Classe non ordinata */
                split_id = GET_SPLIT_ID(instance_ptr, i);
                original_split[i] = split_id;
                if ((position = already_split(split_id, i, cl, tr, instance_ptr)) == UNKNOWN) {
                    /* Non analizzata in precedenza */
                    if (IS_UNORDERED(cl)) {
                        /* Classe non ordinata */
                        ds = GET_DYN_ID(instance_ptr, i);
                        ss = get_static_subclass(cl, ds);
                        if (GET_CARD(cl, ss, ds) > 1) {
                            /* sottoclasse con card. > 1 */
                            instance_ptr->split[i] = nds = tabc[cl].sbclist[ss].offset + GET_NEW_ID(cl, ss);
                            DECREASE_CARD(cl, ss, ds - tabc[cl].sbclist[ss].offset, 1);
                            SET_CARD(cl, ss, GET_NEW_ID(cl, ss), 1);
                            INCREASE_NUM_SS(cl, ss);
                            INCREASE_NUM_CL(cl);
                            nofs[cl][ss]++;
                            copy_marking_distribution(cl, ss, ds, nds);
                        } /* sottoclasse con card. > 1 */
                        else {
                            /* sottoclasse con card. = 1 */
                            instance_ptr->split[i] = instance_ptr->npla[i];
                        } /* sottoclasse con card. = 1 */
                    }/* Classe non ordinata */
#ifdef REACHABILITY
                    else {
                        /* Classe ordinata */
                        ds = get_dynamic_from_object(cl, split_id);
                        if (GET_CARD(cl, 0, ds) > 1) {
                            /* sottoclasse con card. > 1 */
                            split_object(cl, ds, split_id, instance_ptr, i);
                            for (rp = 0; rp < (i - tabt[tr].off[cl]); rp++)
                                if (instance_ptr->split[tabt[tr].off[cl] + rp] > ds)
                                    instance_ptr->split[tabt[tr].off[cl] + rp]++;
                        }/* sottoclasse con card. > 1 */
                        else {
                            /* sottoclasse con card. = 1 */
                            instance_ptr->split[i] = ds;
                        } /* sottoclasse con card. = 1 */
                        switch (tabt[tr].split_type[i]) {
                        case COMPLEX:     ds = get_dynamic_from_object(cl, (split_id + 1) % GET_CLASS_CARDINALITY(cl));
                            if (GET_CARD(cl, 0, ds) > 1) {
                                split_object(cl, ds, (split_id + 1) % GET_CLASS_CARDINALITY(cl), instance_ptr, UNKNOWN);
                                if (split_id == GET_CLASS_CARDINALITY(cl) - 1)
                                    instance_ptr->split[i]++;
                                for (rp = 0; rp < (i - tabt[tr].off[cl]); rp++)
                                    if (instance_ptr->split[tabt[tr].off[cl] + rp] > ds)
                                        instance_ptr->split[tabt[tr].off[cl] + rp]++;
                            }
                            ds = get_dynamic_from_object(cl, get_prec(cl, split_id));
                            if (GET_CARD(cl, 0, ds) > 1) {
                                split_object(cl, ds, get_prec(cl, split_id), instance_ptr, UNKNOWN);
                                if (split_id != 0)
                                    instance_ptr->split[i]++;
                                for (rp = 0; rp < (i - tabt[tr].off[cl]); rp++)
                                    if (instance_ptr->split[tabt[tr].off[cl] + rp] > ds)
                                        instance_ptr->split[tabt[tr].off[cl] + rp]++;
                            }
                            break;
                        case SUCCESSOR:   ds = get_dynamic_from_object(cl, (split_id + 1) % GET_CLASS_CARDINALITY(cl));
                            if (GET_CARD(cl, 0, ds) > 1) {
                                split_object(cl, ds, (split_id + 1) % GET_CLASS_CARDINALITY(cl), instance_ptr, UNKNOWN);
                                if (split_id == GET_CLASS_CARDINALITY(cl) - 1)
                                    instance_ptr->split[i]++;
                                for (rp = 0; rp < (i - tabt[tr].off[cl]); rp++)
                                    if (instance_ptr->split[tabt[tr].off[cl] + rp] > ds)
                                        instance_ptr->split[tabt[tr].off[cl] + rp]++;
                            }
                            break;
                        case PREDECESSOR: ds = get_dynamic_from_object(cl, get_prec(cl, split_id));
                            if (GET_CARD(cl, 0, ds) > 1) {
                                split_object(cl, ds, get_prec(cl, split_id), instance_ptr, UNKNOWN);
                                if (split_id != 0)
                                    instance_ptr->split[i]++;
                                for (rp = 0; rp < (i - tabt[tr].off[cl]); rp++)
                                    if (instance_ptr->split[tabt[tr].off[cl] + rp] > ds)
                                        instance_ptr->split[tabt[tr].off[cl] + rp]++;
                            }
                            break;
                        }
                    }/* Classe ordinata */
#endif
                }/* Non analizzata in precedenza */
                else
                    instance_ptr->split[i] = instance_ptr->split[position];
            }/* Classe non ordinata */
            else {
                /* Classe ordinata */
                instance_ptr->split[i] = instance_ptr->npla[i];
            }/* Classe ordinata */
    } /* per ogni componente del dominio della transizione */
} /* End split */
#endif
#endif
