# include <stdio.h>
#include "../../INCLUDE/const.h"
#ifdef SWN
#ifdef SYMBOLIC
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"

# define ALREADY_EXAMINED(pos) iag[pos] != UNKNOWN
# define GET_DYNAMIC_IDENTITY_FROM_OFFSET(c,s,d) tabc[c].sbclist[s].offset+d
# define CURRENT_LAST_ID(c,s) num[c][s] - 1
# define WAS_ORIGINAL(cl,ss,ds) ds <= original[cl][ss]
# define GET_NUM_OF_BUNG(cl,ss) original[cl][ss]  - GET_STATIC_OFFSET(cl,ss) - num[cl][ss] + 2
# define GET_TO_FIRST_TOKEN(p,prv,pos,d) for(; p != NULL ;)\
				      if(p->id[pos] == d) break;\
				      else {prv = p;p = NEXT_TOKEN(p);}

int *iag = NULL;
int *objects = NULL;

int **original = NULL;

int ** *bung = NULL;

extern int **nofs;

extern int optimized_compare_token_id();
extern void delete_token();
extern void insert_token();
extern void shift_dynamic_subclasses();

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_merge_data_structure() {
    /* Init initialize_merge_data_structure */
    int ii, ss;

    if (ncl != 0) {
        original = (int **)ecalloc(ncl, sizeof(int *));
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
                        original[ii] = (int *)ecalloc(GET_STATIC_SUBCLASS(ii), sizeof(int));
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
    if (max_transition_domain > 0) {
        iag = (int *)ecalloc(max_transition_domain, sizeof(int));
        objects = (int *)ecalloc(max_transition_domain, sizeof(int));
    }
}/* End initialize_merge_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_new_position_of_token(Token_p tok_ptr, int pl, TokInfo_p ret_ptr) {
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
        if (IS_FULL(pl)) {
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
static int same_token_except_one_component(tok1, tok2, pos, length)
Token_p tok1;
Token_p tok2;
int pos;
int length;
{
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
static int check_of_marking_distribution(tocompact, try, pl, fix)
int tocompact;
int try;
int pl;
int fix;
{
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
static int arecompactable(ds1, ds2, cl, ss)
int ds1;
int ds2;
int cl;
int ss;
{
    /* Init arecompactable */
    int ret_value, pl, k;

    for (pl = npl , ret_value = TRUE ; pl && ret_value ; pl--) {
        /* Per ogni posto */
        if (IS_FULL(pl - 1)) {
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

    for (pl = npl ; pl ; pl--) {
        /* Per ogni posto */
        if (IS_FULL(pl - 1)) {
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

    for (pl = npl ; pl ; pl--) {
        /* Per ogni posto */
        if (IS_FULL(pl - 1)) {
            /* Posto con almeno un token */
            if (IS_COLOURED(pl - 1)) {
                /* Posto con dominio di colore */
                if (net_mark[pl - 1].group != NULL) {
                    /* Per ogni token nella lista */
                    tok_ptr = net_mark[pl - 1].group;
                    for (; tok_ptr != NULL;) {
                        next_ptr = tok_ptr->nxt_group;
                        if (tok_ptr->next != NULL)
                            (tok_ptr->next)->prv_group = tok_ptr->prv_group;
                        if (tok_ptr->prv_group == NULL)
                            net_mark[pl - 1].marking = tok_ptr->next;
                        else
                            (tok_ptr->prv_group)->next = tok_ptr->next;
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
static void change_last_identity_of_token(cl, ss, ds)
int cl;
int ss;
int ds;
{
    /* Init change_last_identity_of_token */
    int pl, k;
    Token_p tok_ptr;
    Token_p prv_ptr;
    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    for (pl = npl ; pl ; pl--) {
        /* Per ogni posto */
        if (IS_FULL(pl - 1)) {
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
static void adjust_dynamic_subclasses_identities(cl, ss, pos, old_id, new_id, fired_ptr, yes)
int cl;
int ss;
int pos;
int old_id;
int new_id;
Event_p fired_ptr;
int yes;
{
    /* Init adjust_dynamic_subclasses_identities */
    int tr = GET_TRANSITION_INDEX(fired_ptr);
    int start;

    if (WAS_ORIGINAL(cl, ss, old_id))
        for (start = 0; start < GET_NUM_OF_BUNG(cl, ss) ; start++)
            if (bung[cl][ss][start] == old_id)
                bung[cl][ss][start] = new_id;
    if (yes) {
        for (start = OFF(tr, cl); start < OFF(tr, cl) + GET_COLOR_REPETITIONS(cl, tr) ; start++) {
            if (iag[start] == old_id)
                iag[start] = new_id;
            if (fired_ptr->npla[start] == old_id) {
                iag[start] = new_id;
            }
            if (fired_ptr->split[start] == old_id) {
                fired_ptr->split[start] = new_id;
                iag[start] = new_id;
            }
        }
    }
}/* End adjust_dynamic_subclasses_identities */
#ifdef VECCHIA
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void adjust_dynamic_subclasses_identities(cl, ss, pos, old_id, new_id, fired_ptr)
int cl;
int ss;
int pos;
int old_id;
int new_id;
Event_p fired_ptr;
{
    /* Init adjust_dynamic_subclasses_identities */
    int tr = GET_TRANSITION_INDEX(fired_ptr);
    int start;

    if (WAS_ORIGINAL(cl, ss, old_id))
        for (start = 0; start < GET_NUM_OF_BUNG(cl, ss) ; start++)
            if (bung[cl][ss][start] == old_id)
                bung[cl][ss][start] = new_id;
    for (start = OFF(tr, cl); start < OFF(tr, cl) + GET_COLOR_REPETITIONS(cl, tr) ; start++) {
        if (pos == start)
            iag[start] = new_id;
        if (iag[start] == old_id)
            iag[start] = new_id;
        if (pos != start)
            if (fired_ptr->split[start] == old_id) {
                fired_ptr->split[start] = new_id;
                iag[start] = new_id;
            }
    }
}/* End adjust_dynamic_subclasses_identities */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void group_object(cl, ds, object)
int cl;
int ds;
int object;
{
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
void group(Event_p instance_ptr) {
    /* Init group */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int comp = GET_TRANSITION_COMPONENTS(tr);
    int cl, ss, ds, dyn_offset, i;
    int try_id, tocheck, prec;

    for (cl = 0; cl < ncl; cl++)
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
            {
                /* Classe non ordinata */
                for (ss = 0 ; ss < GET_STATIC_SUBCLASS(cl); ss++) {
                    original[cl][ss] = GET_STATIC_OFFSET(cl, ss) + GET_NUM_SS(cl, ss) - nofs[cl][ss] - 1;
                }
            }/* Classe non ordinata */
    for (i = 0 ; i < comp; i++) {
        cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            iag[i] = UNKNOWN;
            objects[i] = get_object_from_dynamic(cl, instance_ptr->split[i]);
        }
#endif
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
            iag[i] = UNKNOWN;
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
                /* O meglio, selezionare in modo "furbo" le dynamic subclass da fondere */
                if (!ALREADY_EXAMINED(i)) {
                    /* Non analizzata in precedenza */
#ifdef REACHABILITY
                    if (IS_UNORDERED(cl)) {
#endif
                        ss = get_static_subclass(cl, ds);	 /* Controllare se esiste */
                        for (dyn_offset = 0 ; dyn_offset < GET_NUM_SS(cl, ss) ; dyn_offset++) {
                            /* Per tutte le sottoclassi dinamiche di ss */
                            try_id = GET_DYNAMIC_IDENTITY_FROM_OFFSET(cl, ss, dyn_offset);
                            if (try_id != instance_ptr->split[i]) {
                                /* Se sono diverse */
                                reset_group_pointers();
                                if (arecompactable(ds, try_id, cl, ss)) {
                                    /* Si puo' compattare */
                                    push_tokens_with_non_existing_id();
                                    INCREASE_CARD(cl, ss, dyn_offset, GET_CARD(cl, ss, ds));
                                    adjust_dynamic_subclasses_identities(cl, ss, i, ds, try_id, instance_ptr, TRUE);
                                    if (CURRENT_LAST_ID(cl, ss) != ds - tabc[cl].sbclist[ss].offset) {
                                        /* E' una d.s centrale */
                                        SET_CARD(cl, ss, ds - tabc[cl].sbclist[ss].offset, card[cl][ss][CURRENT_LAST_ID(cl, ss)]);
                                        change_last_identity_of_token(cl, ss, ds);
                                        if (WAS_ORIGINAL(cl, ss, CURRENT_LAST_ID(cl, ss))) {
                                            /* L'ultima id attuale e' una originale che cambiera' */
                                            bung[cl][ss][original[cl][ss] - num[cl][ss] + 1 - GET_STATIC_OFFSET(cl, ss)] = ds;
                                        }/* L'ultima id attuale e' una originale che cambiera' */
                                        adjust_dynamic_subclasses_identities(cl, ss, i, GET_STATIC_OFFSET(cl, ss) + CURRENT_LAST_ID(cl, ss), ds, instance_ptr, TRUE);
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
                }/* Non analizzata in precedenza */
            }/* Classe non ordinata */
    } /* Per ogni componente del dominio della transizione */
}/* End group */
#endif
#endif
