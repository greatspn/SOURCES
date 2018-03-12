# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/service.h"


extern void out_free_token();
extern int marcatura;
#ifdef SWN
extern Token_p pop_token();
#endif
extern Node_p get_place_list();
extern void get_pointer_info();
extern void delete_token();
extern void push_token();
extern void insert_in_reset();
extern void insert_in_touched();
extern void init_token_values();
extern void insert_token();
extern void update_list_of_enabled_transition();
extern int at_least_one_predicate();
extern int get_request_for();

#ifdef SYMBOLIC
#ifdef SIMULATION
extern void split();
extern void group();
#endif
#endif

#ifdef FLUSH_FUNCTION
extern void get_to_first();
extern void flush_set_token_marking();
extern int matching();
#endif

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int plus_op(int  op1,  int  op2) {
    /* Init plus_op */
    return (op1 + op2);
}/* Init plus_op */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int minus_op(int  op1,  int  op2) {
    /* Init minus_op */
    return (op1 - op2);
}/* Init minus_op */
#ifdef SWN
#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int optimized_get_number_of_ordinary_tokens(Cart_p  cartes_ptr,  int  pl) {
    /* Init optimized_get_number_of_ordinary_tokens */
    int ret_value = 1;
    int ii = GET_PLACE_COMPONENTS(pl);
    int cl, ss, ds;

    for (; ii ; ii--) {
        /* Per ogni componente del dominio del posto */
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
            {
                /* Classe non ordinata */
                ds = cartes_ptr->low[ii - 1];
                ss = get_static_subclass(cl, ds);
                ret_value *= GET_CARD(cl, ss, ds);
            }/* Classe non ordinata */
    }/* Per ogni componente del dominio del posto */
    return (ret_value);
}/* End optimized_get_number_of_ordinary_tokens */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_number_of_ordinary_tokens(Cart_p  cartes_ptr,  int  pl) {
    /* Init get_number_of_ordinary_tokens */
    int ret_value = 1;
    int ii = GET_PLACE_COMPONENTS(pl);
    int cl, ss, ds;

    for (; ii ; ii--) {
        /* Per ogni componente del dominio del posto */
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
            {
                /* Classe non ordinata */
                ds = cartes_ptr->mark[ii - 1][cartes_ptr->low[ii - 1]];
                ss = get_static_subclass(cl, ds);
                ret_value *= GET_CARD(cl, ss, ds);
            }/* Classe non ordinata */
    }/* Per ogni componente del dominio del posto */
    return (ret_value);
}/* End get_number_of_ordinary_tokens */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void set_token_marking(Cart_p  cartes_ptr,  int  delta,  int  pl,  int (*op)(),  int  inc_flag,  int  tr) {
    /* Init set_token_marking */
    int old_molt , new_molt;
    int status = 0;

    Token_p tok_ptr = NULL;

    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    if (op == NULL)
        op = plus_op;
    get_pointer_info(cartes_ptr , pl , res_ptr, TRUE);
    tok_ptr = GET_TOKEN_POSITION(res_ptr);
    old_molt = GET_OLD_MOLTEPLICITY(tok_ptr);
    new_molt = (*op)(old_molt, delta);
    if (new_molt < 0) {
        /* Errore */
        out_error(ERROR_NEGATIVE_MARKING, pl, tr, 0, 0, NULL, NULL);
    }/* Errore */
    else {
        /* OK ! */
#ifdef SYMBOLIC
        int ordinary_tokens = get_number_of_ordinary_tokens(cartes_ptr, pl);

        net_mark[pl].total = (*op)(net_mark[pl].total , delta * ordinary_tokens);
#endif
#ifdef COLOURED
        net_mark[pl].total = (*op)(net_mark[pl].total, delta);
#endif
        status = (old_molt > 0) << 1;
        status |= (new_molt == 0);
        if (IS_NEUTRAL(pl)) {
            /* Posto neutro */
            SET_TOKEN_MOLTEPLICITY(new_molt, tok_ptr);
            switch (status) {
            /* Vari casi da analizzare */
            case OLD_GT_NEW_EQ :
                net_mark[pl].different = 0;
#ifdef SYMBOLIC
                net_mark[pl].ordinary = 0;
#endif
                break;
            case OLD_EQ_NEW_GT :
                net_mark[pl].different = 1;
#ifdef SYMBOLIC
                net_mark[pl].ordinary = 1;
#endif
                break;
            }/* Vari casi da analizzare */
        }/* Posto neutro */
        else {
            /* Posto colorato */
            Token_p add_ptr = NULL;

            switch (status) {
            /* Vari casi da analizzare */
            case OLD_GT_NEW_EQ :
                (net_mark[pl].different)--;
#ifdef SYMBOLIC
                net_mark[pl].ordinary -= ordinary_tokens;
#endif
                delete_token(res_ptr, pl);
#ifdef SIMULATION
                if (!inc_flag)
                    push_token(tok_ptr, pl);
                else
                    insert_in_reset(tok_ptr, pl);
#endif
#ifdef REACHABILITY
                push_token(tok_ptr, pl);
#endif
                break;
            case OLD_GT_NEW_GT :
                SET_TOKEN_MOLTEPLICITY(new_molt, tok_ptr);
#ifdef SIMULATION
                if (inc_flag)
                    insert_in_touched(tok_ptr, pl);
#endif
                break;
            case OLD_EQ_NEW_GT :
                (net_mark[pl].different)++;
#ifdef SYMBOLIC
                net_mark[pl].ordinary += ordinary_tokens;
#endif
                add_ptr = pop_token(pl);
                init_token_values(cartes_ptr, add_ptr, pl);
                SET_TOKEN_MOLTEPLICITY(new_molt, add_ptr);
                insert_token(res_ptr, add_ptr, pl);
#ifdef SIMULATION
                if (inc_flag)
                    insert_in_touched(add_ptr, pl);
#endif
                break;
            case OLD_EQ_NEW_EQ :
                break;
            }/* Vari casi da analizzare */
        }/* Posto colorato */
    }/* OK ! */
}/* End set_token_marking */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void update_marking_value(Cart_p  cartes_ptr,  int  delta,  int  pl,  int  type,  int  inc_flag,  int  tr) {
    /* Init update_marking_value */
    int (*op)();

    op = GET_SUM_OPERATOR(type);
#ifdef GSPN
    if (op == NULL)
        op = plus_op;
    net_mark[pl].total = (*op)(net_mark[pl].total, delta);
    if (net_mark[pl].total < 0) {
        /* Errore */
        out_error(ERROR_NEGATIVE_MARKING, pl, tr, 0, 0, NULL, NULL);
    }/* Errore */
#endif
#ifdef SWN
    set_token_marking(cartes_ptr, delta , pl , op, inc_flag, tr);
#endif

}/* End update_marking_value */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fire(Event_p  instance_ptr,  int  type,  int  places) {
    /* Init fire */
    Node_p node_ptr = NULL;

    int pl = 0;
    int req_value = 0;
    int tr , domain_comp, inc_flag;

#ifdef FLUSH_FUNCTION
    Token_p start_ptr = NULL;

    int (*op)();

    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    op = GET_SUM_OPERATOR(type);
#endif

    tr = GET_TRANSITION_INDEX(instance_ptr);
    node_ptr = get_place_list(tr, places);
    for (; node_ptr != NULL ;) {
        /* Per ogni posto */
        if (!IS_TEST_ARC(node_ptr)) {
            /* Se non e' un arco di test si fa lo sparo */
            pl = GET_PLACE_INDEX(node_ptr);
#ifdef SIMULATION
            inc_flag = IS_INVOLVED_IN_CC_SC(node_ptr);
#endif
#ifdef REACHABILITY
            inc_flag = FALSE;
#endif
            if (at_least_one_predicate(node_ptr, instance_ptr, pred_val)) {
                /* Almeno un predicato della C.L e' vero */
#ifdef SWN
                domain_comp = GET_PLACE_COMPONENTS(pl);
#endif

#ifdef FLUSH_FUNCTION
                if (node_ptr->type == FLUSH_TYPE) {
                    /* Caso Flush */
                    init_cartesian_product(cart_place_ptr, node_ptr, instance_ptr, pred_val);
                    get_to_first(cart_place_ptr , pl , domain_comp, res_ptr); /* Da fare */
                    for (;;) {
                        /* Ciclo di aggiornamento per ogni ennupla	*/
                        flush_set_token_marking(1 , pl , op, inc_flag, res_ptr);
                        if ((res_ptr->pos)->next != NULL) {
                            if (!matching(cart_place_ptr , ((res_ptr->pos)->next)->id , pl, domain_comp))
                                break;
                        }
                        else
                            break;
                        res_ptr->pos = (res_ptr->pos)->next;
                    }/* Ciclo di aggiornamento per ogni ennupla */
                }/* Caso Flush */
                else {
#endif
#ifdef SWN
                    init_cartesian_product(cart_place_ptr, node_ptr, instance_ptr, pred_val, TRUE);
                    do {
                        /* Ciclo di aggiornamento per ogni ennupla  */
#endif
                        req_value = get_request_for(cart_place_ptr, node_ptr, instance_ptr, pred_val, TRUE);
                        update_marking_value(cart_place_ptr, req_value, pl, type, inc_flag, tr);
#ifdef SWN
                    }/* Ciclo di aggiornamento per ogni ennupla  */
                    while (nextvec(cart_place_ptr, domain_comp));
#endif
#ifdef FLUSH_FUNCTION
                }
#endif
            }/* Almeno un predicato della C.L e' vero */
        }/* Se non e' un arco di test si fa lo sparo */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto */
}/* End fire */
#ifdef GSPN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void update_mirror(Event_p  firing_instance) {
    /* Init update_mirror */
    int tr = GET_TRANSITION_INDEX(firing_instance);
    Node_p node_ptr = NULL;
    int pl = 0;

    node_ptr = GET_INPUT_LIST(tr);
    for (; node_ptr != NULL ;) {
        /* Per ogni posto */
        if (!IS_TEST_ARC(node_ptr)) {
            /* Se non e' un arco di test si fa lo sparo */
            pl = GET_PLACE_INDEX(node_ptr);
            net_mark[pl].mirror_total = net_mark[pl].total;
        }/* Se non e' un arco di test si fa lo sparo */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto */
    node_ptr = GET_OUTPUT_LIST(tr);
    for (; node_ptr != NULL ;) {
        /* Per ogni posto */
        if (!IS_TEST_ARC(node_ptr)) {
            /* Se non e' un arco di test si fa lo sparo */
            pl = GET_PLACE_INDEX(node_ptr);
            net_mark[pl].mirror_total = net_mark[pl].total;
        }/* Se non e' un arco di test si fa lo sparo */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto */
}/* End update_mirror */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fire_trans(Event_p  firing_instance) {
    /* Init fire_trans */

#ifdef SWN
#ifdef SYMBOLIC
    split(firing_instance);
#endif
#endif
    fire(firing_instance, MINUS_OP, INPUT);
    fire(firing_instance, PLUS_OP, OUTPUT);
#ifdef GSPN
    update_mirror(firing_instance);
#endif
#ifdef SWN
#ifdef SYMBOLIC
    group(firing_instance);
#endif
#endif
#ifdef SIMULATION
    update_list_of_enabled_transition(firing_instance);
#endif
}/* End fire_trans */
