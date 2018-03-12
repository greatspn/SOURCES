#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/ealloc.h"
#include "../../INCLUDE/service.h"

#ifdef SWN
#ifdef DEBUG_malloc
static int tok_mall = 0;
static int tok_pop = 0;
static int tok_push = 0;
#endif

static Token_p *token_free = NULL;  /* Array di free list per i token */


#ifdef SIMULATION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_reset_token_list(int  tr) {
    /* Init push_reset_token_list */
    int ii;
    Node_p p = NULL;

    p = GET_INPUT_LIST(tr);
    for (; p != NULL ; p = NEXT_NODE(p))
        if (IS_INVOLVED_IN_CC_SC(p)) {
            ii = GET_PLACE_INDEX(p);
            if (net_mark[ii].reset_t != NULL) {
                (net_mark[ii].reset_t)->next = token_free[ii];
                token_free[ii] = net_mark[ii].reset_h;
            }
            net_mark[ii].reset_t = net_mark[ii].reset_h = NULL;
            net_mark[ii].touched_t = net_mark[ii].touched_h = NULL;
#ifdef DEBUG_malloc
            tok_push += RESET(ii);
#endif
            net_mark[ii].num_of_touched = net_mark[ii].num_of_reset = 0;
        }
    p = GET_OUTPUT_LIST(tr);
    for (; p != NULL ; p = NEXT_NODE(p))
        if (IS_INVOLVED_IN_CC_SC(p)) {
            ii = GET_PLACE_INDEX(p);
            if (net_mark[ii].reset_t != NULL) {
                (net_mark[ii].reset_t)->next = token_free[ii];
                token_free[ii] = net_mark[ii].reset_h;
            }
            net_mark[ii].reset_t = net_mark[ii].reset_h = NULL;
            net_mark[ii].touched_t = net_mark[ii].touched_h = NULL;
#ifdef DEBUG_malloc
            tok_push += RESET(ii);
#endif
            net_mark[ii].num_of_touched = net_mark[ii].num_of_reset = 0;
        }
}/* Init push_reset_token_list */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_token_list(int  pl) {
    /* Init push_token_list */


    if (!IS_NEUTRAL(pl)) {
        /* Coloured places only */
        if (net_mark[pl].marking_t != NULL) {
            (net_mark[pl].marking_t)->next = token_free[pl];
            token_free[pl] = net_mark[pl].marking;
        }
        net_mark[pl].marking = net_mark[pl].marking_t = NULL;
#ifdef DEBUG_malloc
        tok_push += DIFF_TOKENS(pl);
#endif
    }/* Coloured places only */
    net_mark[pl].different = net_mark[pl].total = 0;
#ifdef SYMBOLIC
    net_mark[pl].ordinary = 0;
#endif
}/* Init push_token_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_free_token_list() {
    /* Init initialize_free_token_list */
    int ii;

    token_free = (Token_p *)ecalloc(npl, sizeof(Token_p));
    for (ii = 0 ; ii < npl ; ii++)
        token_free[ii] = NULL;
}/* Init initialize_free_token_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void optimized_init_token_values(int  *id,  Token_p  tok_ptr,  int  pl) {
    /* Init optimized_init_token_values */
    int domain_comp;
    int i;

    domain_comp = GET_PLACE_COMPONENTS(pl);
    for (i = domain_comp; i ; i--)
        tok_ptr->id[i - 1] = id[i - 1];
    tok_ptr->next = tok_ptr->next_reset = tok_ptr->next_touched = NULL;
#ifdef SYMBOLIC
    tok_ptr->nxt_group = tok_ptr->prv_group = NULL;
#endif

}/* End optimized_init_token_values */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_token_values(Cart_p  cartes_ptr,  Token_p  tok_ptr,  int  pl) {
    /* Init init_token_values */
    int domain_comp;
    int i;

    domain_comp = GET_PLACE_COMPONENTS(pl);
    for (i = domain_comp; i ; i--)
        tok_ptr->id[i - 1] = cartes_ptr->mark[i - 1][cartes_ptr->low[i - 1]];
    tok_ptr->next = tok_ptr->next_reset = tok_ptr->next_touched = NULL;
#ifdef SYMBOLIC
    tok_ptr->nxt_group = tok_ptr->prv_group = NULL;
#endif
}/* End init_token_values */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Token_p pop_token(int  pl) {
    /* Init pop_token */
    Token_p ret_ptr = NULL;
    int domain_comp;

#ifdef DEBUG_malloc
    tok_pop++;
#endif
    domain_comp = GET_PLACE_COMPONENTS(pl);
    if (token_free[pl] == NULL) {
        /* Lista libera per il posto e' vuota */
        ret_ptr = (Token_p)emalloc(sizeof(struct COLOURED_TOKEN));
        ret_ptr->id = (int *)ecalloc(domain_comp , sizeof(int));
#ifdef DEBUG_malloc
        tok_mall++;
#endif
    }/* Lista libera per il posto e' vuota */
    else {
        ret_ptr = token_free[pl];
        token_free[pl] = token_free[pl]->next;
    }
    return (ret_ptr);
}/* End pop_token */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_token(Token_p  tok_ptr,  int  pl) {
    /* Init push_token */

#ifdef DEBUG_malloc
    tok_push++;
#endif
    tok_ptr->next = token_free[pl];
    token_free[pl] = tok_ptr;
}/* End push_token */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int optimized_compare_token_id(int  *tok1,  int  *tok2,  int  pl) {
    /* Init optimized_compare_token_id */
    int ret_value = EQUAL_TO;
    int length = GET_PLACE_COMPONENTS(pl);
    int i, id1;
#ifdef SYMBOLIC
    int cl;
#endif

    for (i = 0; i < length; i++) {
        /* Per ogni componente dell'identita' del token */
#ifdef SYMBOLIC
        cl = GET_COLOR_COMPONENT(i, pl);
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
                /*id1 = get_dynamic_from_total(cl,tok1[i]);*/
                id1 = tok1[i];
            else
#endif
                id1 = tok1[i];
        if (id1 < tok2[i]) {
            /* tok1 < tok2 */
            ret_value = LESSER_THAN;
            goto ok;
        }/* tok1 < tok2 */
        else if (id1 > tok2[i]) {
            /* tok1 > tok2 */
            ret_value = GREATER_THAN;
            goto ok;
        }/* tok1 > tok2 */
    }/* Per ogni componente dell'identita' del token */
ok: return (ret_value);
}/* End optimized_compare_token_id */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int compare_token_id(Cart_p  cartes_ptr,  int  *tok2,  int  pl,  int  type) {
    /* Init compare_token_id */
    int ret_value = EQUAL_TO;
    int length = GET_PLACE_COMPONENTS(pl);
    int i, id1;
#ifdef SYMBOLIC
    int cl;
#endif

    for (i = 0; i < length ; i++) {
        /* Per ogni componente dell'identita' del token */
#ifdef REACHABILITY
#ifdef SYMBOLIC
        cl = GET_COLOR_COMPONENT(i, pl);
        if (IS_UNORDERED(cl)) {
            /* Classe non ordinata */
            id1 = DECODE_ID(cartes_ptr->mark[i][cartes_ptr->low[i]]);
        }/* Classe non ordinata */
        else if (GET_STATIC_SUBCLASS(cl) == 1) {
            if (type == TRUE)
                id1 = cartes_ptr->mark[i][cartes_ptr->low[i]];
            else
                id1 = get_dynamic_from_object(cl, cartes_ptr->mark[i][cartes_ptr->low[i]]);
        }
        else
#endif
#endif
#ifdef SIMULATION
#ifdef SYMBOLIC
            cl = GET_COLOR_COMPONENT(i, pl);
        if (IS_UNORDERED(cl)) {
            /* Classe non ordinata */
            id1 = DECODE_ID(cartes_ptr->mark[i][cartes_ptr->low[i]]);
        }/* Classe non ordinata */
        else
#endif
#endif
            id1 = cartes_ptr->mark[i][cartes_ptr->low[i]];
        if (id1 < tok2[i]) {
            /* tok1 < tok2 */
            ret_value = LESSER_THAN;
            goto ok;
        }/* tok1 < tok2 */
        else if (id1 > tok2[i]) {
            /* tok1 > tok2 */
            ret_value = GREATER_THAN;
            goto ok;
        }/* tok1 > tok2 */
    }/* Per ogni componente dell'identita' del token */
ok: return (ret_value);
}/* End compare_token_id */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_pointer_info(Cart_p  cartes_ptr,  int  pl,  TokInfo_p  ret_ptr,  int  type) {
    /* Init get_pointer_info */
    Token_p pos_ptr = NULL;
    Token_p prv_ptr = NULL;
    int status = 0;

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
            for (pos_ptr = net_mark[pl].marking; pos_ptr != NULL ; pos_ptr = pos_ptr->next) {
                /* Per ogni token della marcatura di pl */
                status = compare_token_id(cartes_ptr , pos_ptr->id , pl, type);
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
}/* End get_pointer_info */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void insert_token(TokInfo_p  res_ptr,  Token_p  tok_ptr,  int  pl) {
    /* Init insert_token */

    if (res_ptr->prv == NULL) {
        /* Inserimento in testa */
        if (net_mark[pl].marking_t == NULL && net_mark[pl].marking == NULL) {
            /* Posto vuoto */
            tok_ptr->next = tok_ptr->prv = net_mark[pl].marking;
            net_mark[pl].marking = net_mark[pl].marking_t = tok_ptr;
        }/* Posto vuoto */
        else {
            /* Posto con token */
            tok_ptr->next = net_mark[pl].marking;
            tok_ptr->prv = NULL;
            (net_mark[pl].marking)->prv = tok_ptr;
            net_mark[pl].marking = tok_ptr;
        }/* Posto con token */
    }/* Inserimento in testa */
    else {
        /* Inserimento in mezzo */
        Token_p next_ptr = NULL;

        next_ptr = (res_ptr->prv)->next;
        if ((res_ptr->prv)->next != NULL)
            ((res_ptr->prv)->next)->prv = tok_ptr;
        (res_ptr->prv)->next = tok_ptr;

        tok_ptr->next = next_ptr;
        tok_ptr->prv = res_ptr->prv;
        if (next_ptr == NULL) {
            /* Tail insertion */
            net_mark[pl].marking_t = tok_ptr;
        }/* Tail insertion */
    }/* Inserimento in mezzo */
}/* End insert_token */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void delete_token(TokInfo_p  res_ptr,  int  pl) {
    /* Init delete_token */

    if (res_ptr->prv == NULL) {
        /* Cancellazione in testa */
        net_mark[pl].marking = (res_ptr->pos)->next;
        if (net_mark[pl].marking == NULL)
            net_mark[pl].marking_t = NULL;
        else
            (net_mark[pl].marking)->prv = NULL;
    }/* Cancellazione in testa */
    else {
        /* Cancellazione in mezzo */
        (res_ptr->prv)->next = (res_ptr->pos)->next;
        if ((res_ptr->pos->next) != NULL)
            ((res_ptr->pos)->next)->prv = (res_ptr->prv);
        if ((res_ptr->pos->next) == NULL)
            net_mark[pl].marking_t = res_ptr->prv;
    }/* Cancellazione in mezzo */
}/* End delete_token */
#ifdef DEBUG_malloc
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_token_info() {
    /* Init out_token_info */

    fprintf(stdout, "--------------- TOKEN STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , tok_mall, tok_push, tok_pop);
}/* End out_token_info */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int same_token_id(int  *tok1,  int  *tok2,  int  length) {
    /* Init same_token_id */
    int ret_value = FALSE;
    int i;

    for (i = length; i ; i--)
        if (tok1[i - 1] != tok2[i - 1])
            goto ok;
    ret_value = TRUE;
ok: return (ret_value);
}/* End same_token_id */
#ifdef SIMULATION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int is_in_reset_list(Token_p  tok_ptr,  int  pl) {
    /* Init is_in_reset_list */
    Token_p trial_ptr = NULL;
    int ret_value = FALSE;
    int length = GET_PLACE_COMPONENTS(pl);

    trial_ptr = net_mark[pl].reset_h;
    for (; trial_ptr != NULL ;) {
        /* Per ogni elemento in lista reset */
        if (same_token_id(tok_ptr->id, trial_ptr->id, length)) {
            ret_value = TRUE;
            goto ret;
        }
        trial_ptr = NEXT_RESET(trial_ptr);
    }/* Per ogni elemento in lista reset */
ret: return (ret_value);
}/* End is_in_reset_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int is_in_touched_list(Token_p  tok_ptr,  int  pl) {
    /* Init is_in_touched_list */
    Token_p trial_ptr = NULL;
    int ret_value = FALSE;
    int length = GET_PLACE_COMPONENTS(pl);

    trial_ptr = net_mark[pl].touched_h;
    for (; trial_ptr != NULL ;) {
        /* Per ogni elemento in lista touched */
        if (same_token_id(tok_ptr->id, trial_ptr->id, length)) {
            ret_value = TRUE;
            goto ret;
        }
        trial_ptr = NEXT_TOUCHED(trial_ptr);
    }/* Per ogni elemento in lista touched */
ret: return (ret_value);
}/* End is_in_touched_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void insert_in_reset(Token_p  tok_ptr,  int  pl) {
    /* Init insert_in_reset */
    if (!is_in_reset_list(tok_ptr, pl)) {
        tok_ptr->next_reset = tok_ptr->next = tok_ptr->next_touched = net_mark[pl].reset_h;
        net_mark[pl].reset_h = tok_ptr;
        if (net_mark[pl].reset_t == NULL)
            net_mark[pl].reset_t = tok_ptr;
        net_mark[pl].num_of_reset++;
    }
    else
        push_token(tok_ptr, pl);
}/* End insert_in_reset */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void insert_in_touched(Token_p  tok_ptr,  int  pl) {
    /* Init insert_in_touched */
    if (!is_in_touched_list(tok_ptr, pl)) {
        tok_ptr->next_touched = net_mark[pl].touched_h;
        net_mark[pl].touched_h = tok_ptr;
        if (net_mark[pl].touched_t == NULL)
            net_mark[pl].touched_t = tok_ptr;
        net_mark[pl].num_of_touched++;
    }
}/* End insert_in_touched */
#endif
#endif
