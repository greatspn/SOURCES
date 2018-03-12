# include <stdio.h>
#include "../../INCLUDE/const.h"
#ifdef SWN
#ifdef SYMBOLIC
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/ealloc.h"
# include "../../INCLUDE/service.h"

# define ALREADY_EXAMINED(pos) considered_for_merge[pos] != UNKNOWN
# define GET_DYNAMIC_IDENTITY_FROM_OFFSET(c,s,d) tabc[c].sbclist[s].offset+d
# define CURRENT_LAST_ID(c,s) num[c][s] - 1
# define WAS_ORIGINAL(cl,ss,ds) ds <= original[cl][ss]
# define GET_NUM_OF_BUNG(cl,ss) original[cl][ss]  - GET_STATIC_OFFSET(cl,ss) - num[cl][ss] + 2
# define GET_TO_FIRST_TOKEN(p,prv,pos,d) for(; p != NULL ;)\
				      if(p->id[pos] == d) break;\
				      else {prv = p;p = NEXT_TOKEN(p);}
extern int marcatura;


static int *considered_for_merge = NULL;
int *iag_split = NULL;
int *iag_npla = NULL;
int *objects = NULL;

#ifdef SIMULATION
int ** *bung = NULL;
#endif

extern int **nofs;
extern int **original;

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_merge_data_structure() {
    /* Init initialize_merge_data_structure */
    int ii, ss;

#ifdef SIMULATION
    if (ncl != 0) {
        bung = (int ** *)ecalloc(ncl, sizeof(int **));
        for (ii = 0; ii < ncl ; ii ++)
#ifdef REACHABILITY
            if (IS_UNORDERED(ii) || (IS_ORDERED(ii) && GET_STATIC_SUBCLASS(ii) == 1))
#endif
#ifdef SIMULATION
                if (IS_UNORDERED(ii))
#endif
                    if (GET_STATIC_SUBCLASS(ii) > 0) {
                        bung[ii] = (int **)ecalloc(GET_STATIC_SUBCLASS(ii), sizeof(int *));
                    }
        if (max_colour_repetitions > 0) {
            for (ii = 0; ii < ncl ; ii ++)
#ifdef REACHABILITY
                if (IS_UNORDERED(ii) || (IS_ORDERED(ii) && GET_STATIC_SUBCLASS(ii) == 1))
#endif
#ifdef SIMULATION
                    if (IS_UNORDERED(ii))
#endif
                        for (ss = 0; ss < GET_STATIC_SUBCLASS(ii); ss++)
                            bung[ii][ss] = (int *)ecalloc(max_colour_repetitions, sizeof(int));
        }
    }
#endif
    if (max_transition_domain > 0) {
        iag_split = (int *)ecalloc(max_transition_domain, sizeof(int));
        iag_npla = (int *)ecalloc(max_transition_domain, sizeof(int));
        considered_for_merge = (int *)ecalloc(max_transition_domain, sizeof(int));
        objects = (int *)ecalloc(max_transition_domain, sizeof(int));
    }
}/* End initialize_merge_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_new_position_of_token(Token_p  tok_ptr,  int  pl,  TokInfo_p  ret_ptr) {
    /* Init get_new_position_of_token */
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
        //net_mark[pl].marking
        // if(IS_FULL(pl))
        if (net_mark[pl].marking) {
            /* C'e' almeno un token */
            for (pos_ptr = net_mark[pl].marking; pos_ptr != NULL ; pos_ptr = pos_ptr->next) {
                /* Per ogni token della marcatura di pl */
                status = optimized_compare_token_id(tok_ptr->id , pos_ptr->id , pl);
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
}/* End get_new_position_of_token */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int same_token_except_one_component(Token_p tok1,
        Token_p tok2,
        int pos,
        int length) {
    /* Init same_token_except_one_component */
    int ret_value = FALSE;
    int i;

    for (i = length; i ; i--)
        if (i - 1 != pos)
            if (tok1->id[i - 1] != tok2->id[i - 1])
                goto ok;
    ret_value = TRUE;
ok: return (ret_value);
}/* End same_token_except_one_component */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int check_of_marking_distribution(int tocompact,
        int try,
        int pl,
        int fix) {
    /* Init check_of_marking_distribution */
    int ii, found;
    int ret_value = TRUE;
    int comp = GET_PLACE_COMPONENTS(pl);
    Token_p tocompact_ptr = net_mark[pl].marking;
    Token_p prv_tocompact_ptr = NULL;
    Token_p try_ptr = net_mark[pl].marking;
    Token_p prv_try_ptr = NULL;


    for (; ;) {
        /* Finche' non c'e' una condizione di fine */
        GET_TO_FIRST_TOKEN(tocompact_ptr, prv_tocompact_ptr, fix, tocompact);
        GET_TO_FIRST_TOKEN(try_ptr, prv_try_ptr, fix, try);
        if (tocompact_ptr != NULL && try_ptr != NULL) {
            /* Hanno gli stessi token */
            if (same_token_except_one_component(tocompact_ptr, try_ptr, fix, comp)) {
                /* Due token uguali tranne che nella componente da controllare */
                if (tocompact_ptr->molt != try_ptr->molt) {
                    /* Non hanno la stessa distribuzione */
                    ret_value = FALSE;
                    goto ret;
                }/* Non hanno la stessa distribuzione */
                else {
                    /* Inserimento nella lista che potrebbe essere cancellata */
                    /* All'inizio tutti a NULL  net[pl].group  */
                    for (ii = 0, found = FALSE; ii < fix && !found; ii++)
                        if (tabp[pl].dominio[ii] == tabp[pl].dominio[fix])
                            if (tocompact_ptr->id[ii] == tocompact) {
                                found = TRUE;
                                break;
                            }
                    if (!found) {
                        /* Se non e' gia` presente */
                        tocompact_ptr->nxt_group = net_mark[pl].group;
                        tocompact_ptr->prv_group = prv_tocompact_ptr;
                        net_mark[pl].group = tocompact_ptr;
                    }/* Se non e' gia` presente */
                }/* Inserimento nella lista che potrebbe essere cancellata */
            }/* Due token uguali tranne che nella componente da controllare */
            else {
                /* Non hanno la stessa distribuzione */
                ret_value = FALSE;
                goto ret;
            }/* Non hanno la stessa distribuzione */
        }/* Hanno gli stessi token */
        else if (tocompact_ptr == NULL && try_ptr == NULL)
            goto ret;
        else {
            /* Non hanno la stessa distribuzione */
            ret_value = FALSE;
            goto ret;
        }/* Non hanno la stessa distribuzione */
        prv_tocompact_ptr = tocompact_ptr;
        prv_try_ptr = try_ptr;
        tocompact_ptr = NEXT_TOKEN(tocompact_ptr);
        try_ptr = NEXT_TOKEN(try_ptr);
    }/* Finche' non c'e' una condizione di fine */
ret: return (ret_value);
}/* End check_of_marking_distribution */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int arecompactable(int ds1,
                          int ds2,
                          int cl,
                          int ss) {
    /* Init arecompactable */
    int ret_value, pl, k;

    for (pl = npl , ret_value = TRUE ; pl && ret_value ; pl--) {
        /* Per ogni posto */
        //if(IS_FULL(pl - 1))
        if (net_mark[pl - 1].marking) {
            /* Posto con almeno un token */
            if (IS_COLOURED(pl - 1)) {
                /* Posto con dominio di colore */
                for (k = 0 ; k < GET_PLACE_COMPONENTS(pl - 1) && ret_value ; k++)
                    if (tabp[pl - 1].dominio[k] == cl)
                        ret_value &= check_of_marking_distribution(ds1, ds2, pl - 1, k);
            }/* Posto con dominio di colore */
        }/* Posto con almeno un token */
    }/* Per ogni posto */
    return (ret_value);
}/* End arecompactable */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void reset_group_pointers() {
    /* Init reset_group_pointers */
    int pl;

    for (pl = npl ; pl ; pl--)

    {
        /* Per ogni posto */
        //  if(IS_FULL(pl - 1))
        if (net_mark[pl - 1].marking) {
            /* Posto con almeno un token */
            if (IS_COLOURED(pl - 1)) {
                /* Posto con dominio di colore */
                net_mark[pl - 1].group = NULL;
            }/* Posto con dominio di colore */
        }/* Posto con almeno un token */
    }/* Per ogni posto */
}/* End reset_group_pointers */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void push_tokens_with_non_existing_id() {
    /* Init push_tokens_with_non_existing_id */
    int pl;
    Token_p tok_ptr = NULL;
    Token_p next_ptr = NULL;
    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    for (pl = npl ; pl ; pl--) {
        /* Per ogni posto */
        //if(IS_FULL(pl - 1))
        if (net_mark[pl - 1].marking) {
            /* Posto con almeno un token */
            if (IS_COLOURED(pl - 1)) {
                /* Posto con dominio di colore */
                if (net_mark[pl - 1].group != NULL) {
                    /* Per ogni token nella lista */
                    tok_ptr = net_mark[pl - 1].group;
                    for (; tok_ptr != NULL;) {
                        next_ptr = tok_ptr->nxt_group;

                        /*res_ptr->prv = tok_ptr->prv_group;*/
                        res_ptr->prv = tok_ptr->prv;
                        res_ptr->pos = tok_ptr;
                        delete_token(res_ptr, pl - 1);
                        /*
                        	    if(tok_ptr->next != NULL)
                        	     (tok_ptr->next)->prv_group = tok_ptr->prv_group;
                        	    if(tok_ptr->prv_group == NULL)
                        	     net_mark[pl - 1].marking = tok_ptr->next;
                        	    else
                        	     (tok_ptr->prv_group)->next = tok_ptr->next;
                        */
                        push_token(tok_ptr, pl - 1);
                        (net_mark[pl - 1].different)--;
                        tok_ptr = next_ptr;
                    }
                }/* Per ogni token nella lista */
            }/* Posto con dominio di colore */
        }/* Posto con almeno un token */
    }/* Per ogni posto */
}/* End push_tokens_with_non_existing_id */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void change_last_identity_of_token(int cl,
        int ss,
        int ds) {
    /* Init change_last_identity_of_token */
    int pl, k;
    Token_p tok_ptr;
    Token_p prv_ptr;
    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    for (pl = npl ; pl ; pl--) {
        /* Per ogni posto */
        //if(IS_FULL(pl - 1))
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
                            if (tok_ptr->id[k] == tabc[cl].sbclist[ss].offset + CURRENT_LAST_ID(cl, ss)) {
                                res_ptr->prv = prv_ptr;
                                res_ptr->pos = tok_ptr;
                                delete_token(res_ptr, pl - 1);
                                tok_ptr->id[k] = ds;
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
}/* End change_last_identity_of_token */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void adjust_dynamic_subclasses_identities(int cl,
        int ss,
        int pos,
        int old_id,
        int new_id,
        Event_p fired_ptr) {
    /* Init adjust_dynamic_subclasses_identities */
    int tr = GET_TRANSITION_INDEX(fired_ptr);
    int start;

#ifdef SIMULATION
    if (WAS_ORIGINAL(cl, ss, old_id))
        for (start = 0; start < GET_NUM_OF_BUNG(cl, ss) ; start++)
            if (bung[cl][ss][start] == old_id)
                bung[cl][ss][start] = new_id;
#endif
    for (start = OFF(tr, cl); start < OFF(tr, cl) + GET_COLOR_REPETITIONS(cl, tr) ; start++) {
        if (iag_split[start] == old_id)
            iag_split[start] = new_id;
        if (iag_npla[start] == old_id)
            iag_npla[start] = new_id;
    }
    for (start = pos; start < OFF(tr, cl) + GET_COLOR_REPETITIONS(cl, tr) ; start++) {
        if (fired_ptr->npla[start] == old_id)
            iag_npla[start] = new_id;
        if (fired_ptr->split[start] == old_id)
            iag_split[start] = new_id;
    }
}/* End adjust_dynamic_subclasses_identities */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void group_object(int cl,
                         int ds,
                         int object) {
    /* Init group_object */
    int prec;

    ds = get_dynamic_from_object(cl, object);
    if (GET_NUM_CL(cl) > 1) {
        reset_group_pointers();
        if (arecompactable(ds, (ds + 1) % GET_NUM_CL(cl), cl, 0)) {
            /* Si puo' compattare */
            push_tokens_with_non_existing_id();
            INCREASE_CARD(cl, 0, (ds + 1) % GET_NUM_CL(cl), GET_CARD(cl, 0, ds));
            shift_dynamic_subclasses(cl, ds, 1, FALSE);
            DECREASE_NUM_SS(cl, 0);
            DECREASE_NUM_CL(cl);
        }/* Si puo' compattare */
    }
    ds = get_dynamic_from_object(cl, object);
    if (GET_NUM_CL(cl) > 1) {
        reset_group_pointers();
        if (ds)
            prec = ds - 1;
        else
            prec = GET_NUM_CL(cl) - 1;
        if (arecompactable(ds, prec, cl, 0)) {
            /* Si puo' compattare */
            push_tokens_with_non_existing_id();
            INCREASE_CARD(cl, 0, prec, GET_CARD(cl, 0, ds));
            shift_dynamic_subclasses(cl, ds, 1, FALSE);
            DECREASE_NUM_SS(cl, 0);
            DECREASE_NUM_CL(cl);
        }/* Si puo' compattare */
    }
    ds = 0;
    if (GET_NUM_CL(cl) > 1) {
        reset_group_pointers();
        if (arecompactable(GET_NUM_CL(cl) - 1, ds, cl, 0)) {
            /* Si puo' compattare */
            push_tokens_with_non_existing_id();
            INCREASE_CARD(cl, 0, ds, GET_CARD(cl, 0, GET_NUM_CL(cl) - 1));
            DECREASE_NUM_SS(cl, 0);
            DECREASE_NUM_CL(cl);
        }/* Si puo' compattare */
    }
}/* End group_object */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void group(Event_p  instance_ptr) {
    /* Init group */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int comp = GET_TRANSITION_COMPONENTS(tr);
    int cl, ss, ds, dyn_offset, i;
    int try_id, tocheck, prec;

    for (i = 0 ; i < comp; i++) {
        cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            considered_for_merge[i] = iag_npla[i] = iag_split[i] = UNKNOWN;
            objects[i] = get_object_from_dynamic(cl, instance_ptr->split[i]);
        }
#endif
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
            considered_for_merge[i] = iag_npla[i] = iag_split[i] = UNKNOWN;
#endif
    }
    for (i = 0 ; i < comp; i++) {
        /* Per ogni componente del dominio della transizione */
        cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
#endif
#ifdef REACHABILITY
            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
            {
                /* Classe non ordinata */
                ds = GET_SPLIT_ID(instance_ptr, i); /* Oggetto istanziato (split)*/
#ifdef PER_ADESSO_LE_SCORRO_IN_SEQUENZA
                /* O meglio, selezionare in modo "furbo" le dynamic subclass da fondere */
                if (!ALREADY_EXAMINED(i)) {
                    /* Non analizzata in precedenza */
#endif
#ifdef REACHABILITY
                    if (IS_UNORDERED(cl)) {
#endif
                        if (iag_split[i] != UNKNOWN)
                            ds = iag_split[i];
                        ss = get_static_subclass(cl, ds);	 /* Controllare se esiste */

                        for (dyn_offset = 0 ; dyn_offset < GET_NUM_SS(cl, ss) ; dyn_offset++) {
                            /* Per tutte le sottoclassi dinamiche di ss */
                            try_id = GET_DYNAMIC_IDENTITY_FROM_OFFSET(cl, ss, dyn_offset);
                            if (try_id != ds) {
                                /* Se sono diverse */
                                reset_group_pointers();
                                if (arecompactable(ds, try_id, cl, ss)) {
                                    /* Si puo' compattare */
                                    push_tokens_with_non_existing_id();
                                    INCREASE_CARD(cl, ss, dyn_offset, GET_CARD(cl, ss, ds));
                                    adjust_dynamic_subclasses_identities(cl, ss, i, ds, try_id, instance_ptr);
                                    if (CURRENT_LAST_ID(cl, ss) != ds - tabc[cl].sbclist[ss].offset) {
                                        /* E' una d.s centrale */
                                        SET_CARD(cl, ss, ds - tabc[cl].sbclist[ss].offset, card[cl][ss][CURRENT_LAST_ID(cl, ss)]);
                                        change_last_identity_of_token(cl, ss, ds);
#ifdef SIMULATION
                                        if (WAS_ORIGINAL(cl, ss, CURRENT_LAST_ID(cl, ss))) {
                                            /* L'ultima id attuale e' una originale che cambiera' */
                                            bung[cl][ss][original[cl][ss] - num[cl][ss] + 1 - GET_STATIC_OFFSET(cl, ss)] = ds;
                                        }/* L'ultima id attuale e' una originale che cambiera' */
#endif
                                        adjust_dynamic_subclasses_identities(cl, ss, i, GET_STATIC_OFFSET(cl, ss) + CURRENT_LAST_ID(cl, ss), ds, instance_ptr);
                                    }/* E' una d.s centrale */
                                    DECREASE_NUM_SS(cl, ss);
                                    DECREASE_NUM_CL(cl);
                                    break;
                                }/* Si puo' compattare */
                            }/* Se sono diverse */
                        }/* Per tutte le sottoclassi dinamiche di ss */
#ifdef REACHABILITY
                    }
                    else {
                        group_object(cl, ds, objects[i]);
                        switch (tabt[tr].split_type[i]) {
                        case COMPLEX:     ds = get_dynamic_from_object(cl, (objects[i] + 1) % GET_CLASS_CARDINALITY(cl));
                            if (GET_NUM_CL(cl) > 1)
                                group_object(cl, ds, (objects[i] + 1) % GET_CLASS_CARDINALITY(cl));
                            ds = get_dynamic_from_object(cl, get_prec(cl, objects[i]));
                            if (GET_NUM_CL(cl) > 1)
                                group_object(cl, ds, get_prec(cl, objects[i]));
                            break;
                        case SUCCESSOR:   ds = get_dynamic_from_object(cl, (objects[i] + 1) % GET_CLASS_CARDINALITY(cl));
                            if (GET_NUM_CL(cl) > 1)
                                group_object(cl, ds, (objects[i] + 1) % GET_CLASS_CARDINALITY(cl));
                            break;
                        case PREDECESSOR: ds = get_dynamic_from_object(cl, get_prec(cl, objects[i]));
                            if (GET_NUM_CL(cl) > 1)
                                group_object(cl, ds, get_prec(cl, objects[i]));
                            break;
                        }
                    }
#endif
#ifdef PER_ADESSO_LE_SCORRO_IN_SEQUENZA
                }/* Non analizzata in precedenza */
#endif
            }/* Classe non ordinata */
    } /* Per ogni componente del dominio della transizione */
#ifdef SIMULATION
    for (i = 0 ; i < comp; i++) {
        /* Per ogni componente del dominio della transizione */
        if (iag_split[i] != UNKNOWN)
            instance_ptr->split[i] = iag_split[i];
    }/* Per ogni componente del dominio della transizione */
#endif
}/* End group */

#ifdef ESYMBOLIC
/************************* For ESRG ***********************************/
static void ADD_DYN_SBC(INT_LISTP *HEAD, int dyn_sbc) {
    INT_LISTP ptr = (*HEAD);
    (*HEAD) = DYN_LIST_ALLOCATION();
    (*HEAD)->dyn_sbc = dyn_sbc;
    (*HEAD)->next = ptr;
    (*HEAD)->group = NULL;

}
void Group(INT_LISTP *HEAD, int to_group, int group_in) {
    INT_LISTP prec, Next, TO_GROUP, GROUP_IN;
    Next = prec = (*HEAD);
    while (Next) {
        if (!group_in)  GROUP_IN = Next;
        if (!to_group) {TO_GROUP = Next; break;}
        to_group--;
        group_in--;
        prec = Next;
        Next = Next->next;
    }
    prec->next = TO_GROUP->next;
    while (GROUP_IN->group) GROUP_IN = GROUP_IN->group;
    TO_GROUP->next = NULL;
    GROUP_IN->group = TO_GROUP;
}

INT_LISTP GET_GROUP_WITH_POSITION(INT_LISTP HEAD, int position) {
    while (position) {HEAD = HEAD->next; position--;}
    return HEAD;
}

void FREE_ALL(INT_LISTP *HEAD) {
    INT_LISTP Group, prec, prec1, Next = (*HEAD);

    while (Next) {
        prec1 = Next;
        Group = Next->group;
        Next->group = NULL;
        Next = Next->next;
        while (Group) {
            prec = Group->group;
            Group->group = NULL;
            DYN_LIST_FREE(Group);
            Group = prec;
        }
        DYN_LIST_FREE(prec1);
    }

}
void AFTER_CANONISATION(int **sfl_h_min, int *** MAPPE_ARRAY) {
    int ds, cl, k, j;
    int Trait[MAX_CARD];
    for (cl = 0; cl < ncl; cl++) {
        for (k = 0; k < MAX_CARD; k++) Trait[k] = 0;
        for (j = 0; j < tot[cl]; j++) {
            ds = get_dynamic_from_total(cl, sfl_h_min[cl][j]);
            for (k = 0; k < tabc[cl].card; k++)
                if (((*MAPPE_ARRAY)[cl][k] == ds) && (Trait[k] == 0)) {
                    (*MAPPE_ARRAY)[cl][k] = get_dynamic_from_total(cl, j);
                    Trait[k] = 1;
                }
        }
    }
}

void Decal_positions(int pos, int class, int sbc) {
    int i;
    for (i = pos; i < num[class][sbc] - 1; i++)

        card[class][sbc][i] = card[class][sbc][i + 1];

}
void CHANGE_POSITION(int dyn_in, INT_LISTP head) {
    INT_LISTP Next = head;
    INT_LISTP Prec;
    INT_LISTP POS = head;
    int count = dyn_in - 1;

    while (Next->next) {Prec = Next; Next = Next->next;}
    Prec->next = NULL;

    while (count) {POS = POS->next; count--;}
    Next->next = POS->next;
    POS->next = Next;
}
void MY_GROUP(int cl, int wds, int begin, INT_LISTP *head) {
    int dyn_offset = begin, i, ss;
    int try_id, j, ds, dyn_in;


    ds = get_dynamic_from_total(cl, wds);
    ss = get_static_subclass(cl, ds);
    for (; dyn_offset < GET_NUM_CL(cl);) {
        if (get_static_subclass(cl, dyn_offset) == ss) {
            dyn_in = get_dynamic_from_total(cl, dyn_offset) - (tabc[cl].sbclist[ss].offset);
            try_id = get_dynamic_from_total(cl, dyn_offset);
            if (try_id != ds) {
                reset_group_pointers();
                if (arecompactable(try_id, ds, cl, ss)) {
                    Group(head, dyn_offset, ds);
                    push_tokens_with_non_existing_id();
                    INCREASE_CARD(cl, ss, wds, GET_CARD(cl, ss, try_id));
                    if (CURRENT_LAST_ID(cl, ss) != try_id - tabc[cl].sbclist[ss].offset) {
                        SET_CARD(cl, ss, dyn_in, card[cl][ss][CURRENT_LAST_ID(cl, ss)]);
                        change_last_identity_of_token(cl, ss, try_id);
                        CHANGE_POSITION(dyn_in, (*head));
                    }
                    //	 Decal_positions(dyn_in,cl, ss);
                    DECREASE_NUM_SS(cl, ss);
                    DECREASE_NUM_CL(cl);
                }
                else dyn_offset++;
            }
        }
        else dyn_offset++;
    }

}
void ALL_MY_GROUP(int *** MAPPE_ARRAY) {
    int i, j, s, ds, ss;
    INT_LISTP HEAD, tmp;
    for (i = 0; i < ncl; i++) {
        HEAD = NULL;
        for (j = GET_NUM_CL(i) - 1; j >= 0; j--) ADD_DYN_SBC(&HEAD, j);

        for (j = 0; j < GET_NUM_CL(i) - 1; j++)
            MY_GROUP(i, j, j + 1, &HEAD);

        for (j = 0; j < GET_NUM_CL(i); j++) {
            ds = get_dynamic_from_total(i, j);
            ss = get_static_subclass(i, ds);
            tmp = GET_GROUP_WITH_POSITION(HEAD, j);
            while (tmp) {
                for (s = 0; s < tabc[i].card; s++)
                    if ((*MAPPE_ARRAY)[i][s] == (tabc[i].sbclist[ss].offset) + (tmp->dyn_sbc))
                        (*MAPPE_ARRAY)[i][s] = ds;
                tmp = tmp->group;
            }
        }

        FREE_ALL(&HEAD);

    }
}
#endif
/************************************** For ESRG *********************************************/

#endif
#endif





