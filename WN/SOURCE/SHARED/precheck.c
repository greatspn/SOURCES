#include <stdlib.h>
# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/service.h"

extern int evaluate_expression(Expr_p  code,  Event_p  ev_p);

#ifdef SWN
#ifdef SYMBOLIC
extern int get_dynamic_from_total();
#endif

// extern int evaluate_expression();
extern int verify_predicate_constraint();
extern int lesser();
extern int greater();
extern int get_static_subclass();

extern int optimized_compare_token_id();

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void optimized_get_pointer_info(int  *id,  int  pl,  TokInfo_p  ret_ptr) {
    /* Init optimized_get_pointer_info */
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
        for (pos_ptr = net_mark[pl].marking; pos_ptr != NULL ; pos_ptr = pos_ptr->next) {
            /* Per ogni token della marcatura di pl */
            status = optimized_compare_token_id(id , pos_ptr->id , pl);
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
    }/* Posto colorato */
ok: return;
}/* End optimized_get_pointer_info */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int optimized_get_marking_of(cartes_ptr, pl)
Cart_p cartes_ptr;
int pl;
{
    /* Init optimized_get_marking_of */
    int marking_value = 0;

    if (IS_NEUTRAL(pl)) {
        /* Posto neutro */
        marking_value = (net_mark[pl].marking)->molt;
    }/* Posto neutro */
    else {
        /* Posto con dominio di colore */
        struct TOKEN_INFO result;
        TokInfo_p res_ptr = &result;


#ifdef SYMBOLIC
        /////////// this bug was fixed by soheib baarir
        int cl, i, domain_comp = GET_PLACE_COMPONENTS(pl);
        int obj[domain_comp];
        for (i = 0; i < domain_comp; i++) {
            cl = GET_COLOR_COMPONENT(i, pl);
            obj[i] = get_dynamic_from_total(cl, cartes_ptr->low[i]);
        }
        optimized_get_pointer_info(obj , pl , res_ptr);
        //////////////////////////////////////////////
#else
        //// only this function was called in the original version
        optimized_get_pointer_info(cartes_ptr->low , pl , res_ptr);
        /////////////////////////////////////////////////////////
#endif
        if (res_ptr->pos != NULL)
            marking_value = (res_ptr->pos)->molt;
    }/* Posto con dominio di colore */
    return (marking_value);
}/* End optimized_get_marking_of */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void optimized_init_cartesian_product(cartes_ptr, node_ptr)
Cart_p cartes_ptr;
Node_p node_ptr;
{
    /* Init optimized_init_cartesian_product */
    int pl;
    int domain_comp;
    int i, cl;

    pl = GET_PLACE_INDEX(node_ptr);
    domain_comp = GET_PLACE_COMPONENTS(pl);
    for (i = domain_comp; i ; i--) {
        /* Per ogni classe del dominio del posto */
        cl = GET_COLOR_COMPONENT(i - 1, pl);
#ifdef SYMBOLIC
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
            cartes_ptr->up[i - 1] = GET_NUM_CL(cl) - 1;
        else
#endif
#ifdef REACHABILITY
            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                cartes_ptr->up[i - 1] = GET_NUM_CL(cl) - 1;
            else
#endif
#endif
                cartes_ptr->up[i - 1] = GET_CLASS_CARDINALITY(cl) - 1;
        cartes_ptr->low[i - 1] = 0;
    }/* Per ogni classe del dominio del posto */
}/* End optimized_init_cartesian_product */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int optimized_get_request_for(cartes_ptr, node_ptr)
Cart_p cartes_ptr;
Node_p node_ptr;
{
    /* Init optimized_get_request_for */

    int pl;
    int cl, ret_value, prod, pos, i;

    Coeff_p coeff_ptr = NULL;

    pl = GET_PLACE_INDEX(node_ptr);
    if (IS_NEUTRAL(pl)) {
        /* Posto neutro */
        ret_value = node_ptr->molt;
    }/* Posto neutro */
    else {
        /* Posto con dominio di colore */
        int domain_comp , obj;

        ret_value = 0;
        coeff_ptr = GET_PLACE_FUNCTION(node_ptr);
        domain_comp = GET_PLACE_COMPONENTS(pl);
        for (ret_value = 0 ; coeff_ptr != NULL ;) {
            /* C.L */
            for (i = 0 , prod = 1; i < domain_comp ; i++) {
                /* Calcolo molteplicita' */
                cl = GET_COLOR_COMPONENT(i, pl);
#ifdef SYMBOLIC
#ifdef SIMULATION
                if (IS_UNORDERED(cl))
                    obj = get_dynamic_from_total(cl, cartes_ptr->low[i]);
                else
#endif
#ifdef REACHABILITY
                    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                        obj = get_dynamic_from_total(cl, cartes_ptr->low[i]);
                    else
#endif
#endif
                        obj = cartes_ptr->low[i];
                pos = get_static_subclass(cl, obj);
                prod *= GET_SUBCLASS_COEFFICIENT(coeff_ptr, i, pos);
            }/* Calcolo molteplicita' */
            prod *= GET_TUPLE_COEFFICIENT(coeff_ptr);
            ret_value += prod;
            coeff_ptr = NEXT_TUPLE(coeff_ptr);
        }/* C.L */
    }/* Posto con dominio di colore */
    return (ret_value);
}/* End optimized_get_request_for */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int verify_synchronization_constraint(node_ptr, type, tr)
Node_p node_ptr;
int type;
int tr;
{
    /* Init verify_synchronization_constraint */

    int pl;
    int domain_comp;
    int enabled = TRUE;
    int marking_value, request_value;
    int (*magnitude)();


    pl = GET_PLACE_INDEX(node_ptr);
    domain_comp = GET_PLACE_COMPONENTS(pl);

    if (type == INPUT) {
        /* Posti in ingresso */
        magnitude = lesser;
    }/* Posti in ingresso */
    else {
        /* Posti inibitori */
        if (IS_S_ONLY(node_ptr)) {
            /* Solo la funzione S */
            enabled = (!IS_FULL(pl));
            goto ret;
        }/* Solo la funzione S */
        else
            magnitude = greater;
    }/* Posti inibitori */
    optimized_init_cartesian_product(cart_place_ptr, node_ptr);
    do {
        /* Ciclo di verifica per ogni ennupla possibile del posto */

        marking_value = optimized_get_marking_of(cart_place_ptr, pl);
        request_value = optimized_get_request_for(cart_place_ptr, node_ptr);
        if ((*magnitude)(marking_value, request_value))
            enabled = FALSE;
        else if (type == INPUT)
            if (request_value)
                enabled = marking_value / request_value;
    }/* Ciclo di verifica per ogni ennupla possibile del posto */
    while (nextvec(cart_place_ptr, domain_comp) && enabled);
ret: return (enabled);
}/* End verify_synchronization_constraint */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int test_neutral_place(node_ptr, type)
Node_p node_ptr;
int type;
{
    /* Init test_neutral_place */
    int enabled;
    int pl;
    int marking_value;
    int request_value;

    pl = GET_PLACE_INDEX(node_ptr);
    marking_value = (net_mark[pl].marking)->molt;
    if (node_ptr->molt >= 0)
        request_value = node_ptr->molt;
    else if ((request_value = net_mark[-node_ptr->molt - 1].total) == 0)
        request_value = 1;
    if (type == INPUT) {
        enabled = marking_value >= request_value ;
        if (request_value)
            enabled = marking_value / request_value ;
    }
    else
        enabled = marking_value < request_value ? TRUE : FALSE;
    return (enabled);
}/* Init test_neutral_place */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int pre_check(int  tr) {
    /* Inizio pre_check */


    Node_p node_ptr = NULL;

    int pl , card;
    int domain_card;
    int enabled = TRUE;
#ifdef GSPN
    Coeff_p coeff_ptr = NULL;
    int arc_molt;
#endif

    node_ptr = GET_INPUT_LIST(tr);
    for (; node_ptr != NULL && enabled;) {
        /* Per ogni posto in ingresso */
        pl = GET_PLACE_INDEX(node_ptr);
#ifdef GSPN
        if ((coeff_ptr = GET_PLACE_FUNCTION(node_ptr)) != NULL) {
            /* Arco con [P1] ex1+-... [Pn] exn */
            for (arc_molt = 0 ; coeff_ptr != NULL ; coeff_ptr = coeff_ptr->next) {
                if (verify_predicate_constraint(coeff_ptr->guard, NULL))
                    arc_molt += (coeff_ptr->enn_coef * evaluate_expression(coeff_ptr->expression, NULL));
            }
            if (arc_molt > 0)
                enabled = net_mark[pl].total / (node_ptr->molt * arc_molt);
            else if (arc_molt < 0) {
                fprintf(stdout, "Runtime error: arc molteplicity < 0 (place %s transition %s)\n", PLACE_NAME(pl), TRANS_NAME(tr));
                exit(1);
            }
        }/* Arco con [P1] ex1+-... [Pn] exn */
        else
            enabled = net_mark[pl].total / node_ptr->molt;
#endif
#ifdef SWN
        card = GET_FUNCTION_CARDINALITY(node_ptr);
        if (card != UNKNOWN)
            enabled = net_mark[pl].total >= card;
        if (IS_S_ONLY(node_ptr)) {
            /* Solo la funzione S */
            domain_card = GET_PLACE_CARDINALITY(pl);
#ifdef SYMBOLIC
            enabled = (net_mark[pl].ordinary == domain_card);
#endif
#ifdef COLOURED
            enabled = (net_mark[pl].different == domain_card);
#endif
        }/* Solo la funzione S */
#endif
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto in ingresso */

    node_ptr = GET_INHIBITOR_LIST(tr);
    for (; node_ptr != NULL && enabled;) {
        /* Per ogni posto inibitore */
        pl = GET_PLACE_INDEX(node_ptr);
#ifdef GSPN
        if ((coeff_ptr = GET_PLACE_FUNCTION(node_ptr)) != NULL) {
            /* Arco con [P1] ex1+-... [Pn] exn */
            for (arc_molt = 0 ; coeff_ptr != NULL ; coeff_ptr = coeff_ptr->next)
                if (verify_predicate_constraint(coeff_ptr->guard, NULL))
                    arc_molt += (coeff_ptr->enn_coef * evaluate_expression(coeff_ptr->expression, NULL));
            if (arc_molt > 0)
                enabled = net_mark[pl].total < (node_ptr->molt * arc_molt);
            else if (arc_molt < 0) {
                fprintf(stdout, "Runtime error: arc molteplicity < 0 (place %s transition %s)\n", PLACE_NAME(pl), TRANS_NAME(tr));
                exit(1);
            }
        }/* Arco con [P1] ex1+-... [Pn] exn */
        else
            enabled = net_mark[pl].total < node_ptr->molt;
#endif
#ifdef SWN
        if (IS_NEUTRAL(pl))
            enabled = test_neutral_place(node_ptr, INHIBITOR);
        else if (IS_INDEPENDENT(node_ptr))
            enabled = verify_synchronization_constraint(node_ptr, INHIBITOR, tr);
#endif
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto inibitore */

    if (enabled) {
        /* Calcolo del grado di abilitazione */
#ifdef GSPN
        int nserv = GET_TRANSITION_SERVER(tr);
        enabled = MIN(enabled, nserv);
#endif
#ifdef SWN
        enabled = GET_TRANSITION_SERVER(tr);
        node_ptr = GET_INPUT_LIST(tr);
        for (; node_ptr != NULL && enabled;) {
            /* Per ogni posto in ingresso */
            pl = GET_PLACE_INDEX(node_ptr);
            if (IS_NEUTRAL(pl))
                enabled = MIN(enabled, test_neutral_place(node_ptr, INPUT));
            else if (
                IS_INDEPENDENT(node_ptr)
#ifdef FLUSH_FUNCTION
                && node_ptr->type != FLUSH_TYPE
#endif
            )
                enabled = MIN(enabled, verify_synchronization_constraint(node_ptr, INPUT, tr));
            node_ptr = NEXT_NODE(node_ptr);
        }/* Per ogni posto in ingresso */
#endif
    }/* Calcolo del grado di abilitazione */
    return (enabled);
}/* Fine pre_check */
