# include <stdio.h>
# include <string.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Node_p get_place_list(int tr, int type) {
    /* Init get_place_list */
    Node_p ret_value = NULL;

    switch (type) {
    /* Tipo di arco */
    case INPUT :     ret_value = GET_INPUT_LIST(tr);
        break;
    case OUTPUT :    ret_value = GET_OUTPUT_LIST(tr);
        break;
    case INHIBITOR : ret_value = GET_INHIBITOR_LIST(tr);
        break;
    }/* Tipo di arco */
    return (ret_value);
}/* End get_place_list */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int are_the_same_tuple(Coeff_p fun1,
                              Coeff_p fun2,
                              int pl,
                              int tr) {
    /* Init are_the_same_tuple */
    int ii, jj, cl;
    int ret_value = TRUE;

    if (GET_TUPLE_COEFFICIENT(fun1) != GET_TUPLE_COEFFICIENT(fun2)) {
        ret_value = FALSE;
        goto ret;
    }
    if (GET_TUPLE_PREDICATE(fun1) != NULL || GET_TUPLE_PREDICATE(fun2) != NULL) {
        ret_value = FALSE;
        goto ret;
    }
    for (ii = 0; ii < GET_PLACE_COMPONENTS(pl); ii++) {
        cl = GET_COLOR_COMPONENT(ii, pl);
        for (jj = 0; jj < GET_STATIC_SUBCLASS(cl); jj++)
            if (GET_SUBCLASS_COEFFICIENT(fun1, ii, jj) != GET_SUBCLASS_COEFFICIENT(fun2, ii, jj)) {
                ret_value = FALSE;
                goto ret;
            }
        for (jj = 0; jj < GET_COLOR_REPETITIONS(cl, tr); jj++)
            if (GET_PROJECTION_COEFFICIENT(fun1, ii, jj) != GET_PROJECTION_COEFFICIENT(fun2, ii, jj)
                    || GET_SUCCESSOR_COEFFICIENT(fun1, ii, jj) != GET_SUCCESSOR_COEFFICIENT(fun2, ii, jj)
                    || GET_PREDECESSOR_COEFFICIENT(fun1, ii, jj) != GET_PREDECESSOR_COEFFICIENT(fun2, ii, jj)
               ) {
                ret_value = FALSE;
                goto ret;
            }
    }
ret: return (ret_value);
}/* End are_the_same_tuple */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int are_the_same_function(Coeff_p fun1,
                                 Coeff_p fun2,
                                 int pl,
                                 int tr) {
    /* Init are_the_same_function */
    int ret_value = TRUE;

    for (; fun1 != NULL && fun2 != NULL;) {
        if (!are_the_same_tuple(fun1, fun2, pl, tr)) {
            ret_value = FALSE;
            goto ret;
        }
        fun1 = NEXT_TUPLE(fun1);
        fun2 = NEXT_TUPLE(fun2);
    }
    if (fun1 != NULL || fun2 != NULL)
        ret_value = FALSE;
ret: return (ret_value);
}/* End are_the_same_function */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_test_arc(Node_p my_node_ptr,
                       int type,
                       int tr) {
    /* Init is_test_arc */
    int ret_value = FALSE;
    int my_pl, other_pl;
    Node_p list, other_ptr;
    Coeff_p my_function, other_function;

    if (type == INPUT)
        list = get_place_list(tr, OUTPUT);
    else
        list = get_place_list(tr, INPUT);
    my_pl = GET_PLACE_INDEX(my_node_ptr);
    my_function = GET_PLACE_FUNCTION(my_node_ptr);
    for (other_ptr = list; other_ptr != NULL; other_ptr = NEXT_NODE(other_ptr)) {
        other_pl = GET_PLACE_INDEX(other_ptr);
        if (my_pl == other_pl) {
            if (GET_ARC_MOLTEPLICITY(my_node_ptr) == GET_ARC_MOLTEPLICITY(other_ptr)) {
                other_function = GET_PLACE_FUNCTION(other_ptr);
                if (are_the_same_function(my_function, other_function, my_pl, tr)) {
                    ret_value = TRUE;
                    goto ret;
                }
            }
        }
    }
ret: return (ret_value);
}/* End is_test_arc */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_non_null_coefficient(Coeff_p tuple_ptr,
                                    int tr,
                                    int pl,
                                    int comp) {
    /* Init get_non_null_coefficient */
    int ret_value = UNKNOWN;
    int c, r, i;

    c = GET_COLOR_COMPONENT(comp, pl);
    r = GET_COLOR_REPETITIONS(c, tr);
    for (i = 0; i < r ; i++)
        if (GET_PROJECTION_COEFFICIENT(tuple_ptr, comp, i) != 0) {
            ret_value = i;
            break;
        }
    return (ret_value);
}/* End get_non_null_coefficient */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_the_same(Coeff_p tuple_ptr,
                       int tr,
                       int pl,
                       int comp,
                       int non_null) {
    /* Init is_the_same */
    int ret_value = FALSE;
    int i;

    for (i = 0 ; i < comp ; i++)
        if (GET_COLOR_COMPONENT(comp, pl) == GET_COLOR_COMPONENT(i, pl)) {
            /* Stesso colore nel dominio del posto */
            if (non_null == get_non_null_coefficient(tuple_ptr, tr, pl, i)) {
                ret_value = TRUE;
                break;
            }
        }/* Stesso colore nel dominio del posto */
    return (ret_value);
}/* End is_the_same */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int there_is_implicit_predicate(Coeff_p tuple_ptr,
                                       int tr,
                                       int pl) {
    /* Init there_is_implicit_predicate */
    int comp = GET_PLACE_COMPONENTS(pl);
    int ret_value = FALSE;
    int i, non_null;

    for (i = 0 ; i < comp ; i++) {
        /* Per ogni componente del dominio del posto */
        non_null = get_non_null_coefficient(tuple_ptr, tr, pl, i);
        if (is_the_same(tuple_ptr, tr, pl, i, non_null)) {
            ret_value = TRUE;
            break;
        }
    }/* Per ogni componente del dominio del posto */
    return (ret_value);
}/* End there_is_implicit_predicate */
/********************************************************************/
/* NAME 	: */
/* DESCRIPTION	: */
/* PARAMETERS	: */
/* RETURN VALUE : */
/********************************************************************/
static int count_synchronization_coefficients(Coeff_p tuple_ptr,
        int comp,
        int s) {
    /* Init count_synchronization_coefficients */
    int ret_value = 0;
    int i;

    for (i = 0; i < s ; i++)
        if (tuple_ptr->coef[comp].sbc_coef[i] != 0)
            ret_value++;
    return (ret_value);
}/* End count_synchronization_coefficients */
/********************************************************************/
/* NAME 	: */
/* DESCRIPTION	: */
/* PARAMETERS	: */
/* RETURN VALUE : */
/********************************************************************/
int count_coefficients(Coeff_p tuple_ptr,
                       int comp,
                       int r,
                       int t) {
    /* Init count_coefficients */
    int ret_value = 0;
    int i, off = 0;

    switch (t) {
    case PROJECTION  : off = 0;
        break;
    case SUCCESSOR   : off = 1;
        break;
    case PREDECESSOR : off = 2;
        break;
    default	     : break;
    }
    for (i = 0; i < r ; i++)
        if (tuple_ptr->coef[comp].xsucc_coef[3 * i + off] != 0)
            ret_value++;
    return (ret_value);
}/* End count_coefficients */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int only_S(Coeff_p tuple_ptr,
                  int comp,
                  int s) {
    /* Init only_S */
    int ret_value = TRUE;
    int i;

    for (i = 0; i < s; i++)
        if (tuple_ptr->coef[comp].sbc_coef[i] != 1) {
            ret_value = FALSE;
            goto ret;
        }
ret : return (ret_value);
}/* End only_S */
#ifdef FLUSH_FUNCTION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_flush(Coeff_p tuple_ptr,
                    int comp,
                    int s) {
    /* Init is_flush */
    int ret_value = TRUE;
    int i;

    for (i = 0; i < s; i++)
        if (tuple_ptr->coef[comp].sbc_coef[i] != FLUSH_VALUE) {
            ret_value = FALSE;
            goto ret;
        }
ret : return (ret_value);
}/* End is_flush */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : PROJECTION				      */
/*		  SUCCESSOR				      */
/*		  PREDECESSOR				      */
/*		  SYNCHRONIZATION (SOLO S)		      */
/*		  MIXED (tutto il resto )		      */
/**************************************************************/
static int get_component_type(Coeff_p tuple_ptr,
                              int comp,
                              int tr,
                              int pl) {
    /* Init get_component_type */
    int c, r, s;
    int sync = 0;
    int proj = 0;
    int succ = 0;
    int prec = 0;
    int ret_value = UNKNOWN;

    c = GET_COLOR_COMPONENT(comp, pl);
    r = GET_COLOR_REPETITIONS(c, tr);
    s = GET_STATIC_SUBCLASS(c);

    sync = count_synchronization_coefficients(tuple_ptr, comp, s);
    proj = count_coefficients(tuple_ptr, comp, r, PROJECTION);
    succ = count_coefficients(tuple_ptr, comp, r, SUCCESSOR);
    prec = count_coefficients(tuple_ptr, comp, r, PREDECESSOR);

    if (sync && !proj && !succ && !prec) {
        if (only_S(tuple_ptr, comp, s))
            ret_value = SYNCHRONIZATION;
#ifdef FLUSH_FUNCTION
        else if (is_flush(tuple_ptr, comp, s))
            ret_value = FLUSH_VALUE;
#endif
        else
            ret_value = MIXED;
    }
    else if (!sync && proj && !succ && !prec) {
        if (proj == 1)
            ret_value = PROJECTION;
        else
            ret_value = MIXED;
    }
    else if (!sync && !proj && succ && !prec) {
        if (succ == 1)
            ret_value = SUCCESSOR;
        else
            ret_value = MIXED;
    }
    else if (!sync && !proj && !succ && prec) {
        if (prec == 1)
            ret_value = PREDECESSOR;
        else
            ret_value = MIXED;
    }
    else
        ret_value = MIXED;
    return (ret_value);
}/* End get_component_type */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : PROJECTION				      */
/*		  COMPLEX				      */
/*		  ONLY_S				      */
/**************************************************************/
static int get_tuple_type(Coeff_p tuple_ptr,
                          int tr,
                          int pl) {
    /* Init get_tuple_type */
    int comp = GET_PLACE_COMPONENTS(pl);
    int ret_value = UNKNOWN;
    int only_sync = TRUE;
    int only_proj = TRUE;
    int i, type;

    for (i = 0; i < comp ; i++) {
        /* Per ogni componente del dominio del posto */
        type = get_component_type(tuple_ptr, i, tr, pl);
#ifdef FLUSH_FUNCTION
        if (type == FLUSH_VALUE) {
            /* Caso FLUSH */
            ret_value = FLUSH_TYPE;
            goto ret;
        }/* Caso FLUSH */
#endif
        if (type != PROJECTION && type != SYNCHRONIZATION) {
            /* Caso difficile */
            ret_value = COMPLEX;
            goto ret;
        }/* Caso difficile */
        else {
            /* Possibile ottimizzazione */
            if (type == PROJECTION)
                only_sync = FALSE;
            else
                only_proj = FALSE;
        }/* Possibile ottimizzazione */
    }/* Per ogni componente del dominio del posto */
    if (only_sync && !only_proj && tuple_ptr->enn_coef == 1)
        ret_value = ONLY_S;
    else if (!only_sync  && only_proj)
        ret_value = PROJECTION;
    else
        ret_value = COMPLEX;
    if (ret_value == PROJECTION) {
        /* Controllo del caso <x,x,x ..> */
        if (there_is_implicit_predicate(tuple_ptr, tr, pl))
            ret_value = COMPLEX;
    }/* Controllo del caso <x,x,x ..> */
ret : return (ret_value);
}/* End get_tuple_type */
/**********************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : PROJECTION --> posto neutro o da get_tuple_type     */
/*		  COMPLEX -----> nel caso di C.L o da get_tuple_type  */
/*		  GUARDED -----> nel caso di funzioni con predicati   */
/*		  ONLY_S ------> nel caso di funzione costante	      */
/**********************************************************************/
static int get_node_type(Node_p node_ptr,
                         int tr) {
    /* Init get_node_type */
    int ret_value = UNKNOWN;
    int pl = GET_PLACE_INDEX(node_ptr);

    if (IS_NEUTRAL(pl)) {
        /* Posto neutro */
        ret_value = PROJECTION;
    }/* Posto neutro */
    else {
        /* Posto colorato */
        Coeff_p c_ptr = NULL;

        for (c_ptr = node_ptr->arcfun; c_ptr != NULL ; c_ptr = c_ptr->next) {
            /* Per ogni tupla della C.L. */
            if (c_ptr->guard != NULL) {
                /* Funzione con predicati */
                ret_value = GUARDED;
                goto ret;
            }/* Funzione con predicati */
        }/* Per ogni tupla della C.L. */
        if (node_ptr->arcfun->next != NULL) {
            /* Normale C.L */
            ret_value = COMPLEX;
        }/* Normale C.L */
        else {
            /* Una sola tupla di funzioni */
            ret_value = get_tuple_type(node_ptr->arcfun, tr, pl);
        }/* Una sola tupla di funzioni */
    }/* Posto colorato */
ret: switch (ret_value) {
        /* Tipo di funzione */
#ifdef FLUSH_FUNCTION
    case FLUSH_TYPE :
#endif
    case ONLY_S     :
    case PROJECTION :
    case COMPLEX    :
    case GUARDED    : break;
    default	       : out_error(ERROR_UNKNOWN_FUNCTION_TYPE, pl, tr, 0, 0, NULL, NULL);
    }/* Tipo di funzione */
    return (ret_value);
}/* End get_node_type */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : cardinalita' della ennupla di funzioni       */
/***************************************************************/
static int get_tuple_cardinality(Coeff_p coef_ptr,
                                 int pl,
                                 int tr) {
    /* Init get_tuple_cardinality */
    int ret_value = 1;
    int comp = GET_PLACE_COMPONENTS(pl);
    int i, j, sum, cl;

    for (i = 0; i < comp; i++) {
        /* Per ogni componente del dominio del posto */
        sum = 0;
        cl =  GET_COLOR_COMPONENT(i, pl);
        for (j = 0; j < GET_STATIC_SUBCLASS(cl) ; j++)
            sum += (GET_STATIC_CARDINALITY(cl, j) * coef_ptr->coef[i].sbc_coef[j]);
        for (j = 0; j < GET_COLOR_REPETITIONS(cl, tr) ; j++) {
            sum += coef_ptr->coef[i].xsucc_coef[3 * j];
            sum += coef_ptr->coef[i].xsucc_coef[3 * j + 1];
            sum += coef_ptr->coef[i].xsucc_coef[3 * j + 2];
        }
        ret_value *= sum;
    }/* Per ogni componente del dominio del posto */
    return (ret_value);
}/* End get_tuple_cardinality */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : cardinalita' della funzione che etichetta    */
/*		  l'arco, UNKNOWN nel caso di funzione con     */
/*		  guardia. Si basa sul valore del campo TYPE e */
/*		  sul fatto che la molteplicita' dell'arco sia */
/*		  stata letta in NET.LEX		       */
/***************************************************************/
static int get_node_cardinality(Node_p node_ptr,
                                int tr) {
    /* Init get_node_cardinality */
    int ret_value = 0;
    int pl = GET_PLACE_INDEX(node_ptr);
    Coeff_p coef_ptr = GET_PLACE_FUNCTION(node_ptr);

    for (; coef_ptr != NULL ;) {
        /* Per ogni C.L. */
        coef_ptr->card = get_tuple_cardinality(coef_ptr, pl, tr);
        ret_value += (coef_ptr->card * coef_ptr->enn_coef);
        coef_ptr = NEXT_TUPLE(coef_ptr);
    }/* Per ogni C.L. */
    ret_value *= GET_NODE_MOLTEPLICITY(node_ptr);
    switch (node_ptr->type) {
    /* Tipo di funzione */
    case ONLY_S     :
    case PROJECTION :
    case COMPLEX    : break;
#ifdef FLUSH_FUNCTION
    case FLUSH_TYPE :
#endif
    case GUARDED    : ret_value = UNKNOWN;
        break;
    }/* Tipo di funzione */
    return (ret_value);
}/* End get_node_cardinality */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_independent_from_instance(Node_p node_ptr) {
    /* Init is_independent_from_instance */
    int pl = GET_PLACE_INDEX(node_ptr);
    int ret_value = FALSE;

    if (
        IS_NEUTRAL(pl) || node_ptr->type == ONLY_S
#ifdef FLUSH_FUNCTION
        || node_ptr->type == FLUSH_TYPE
#endif
    )
        ret_value = TRUE;
    return (ret_value);
}/* End is_independent_from_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void order_nodes(Node_p start,  int type) {
    /* Init order_nodes */
    int i, j;
    Node_p point[8];
    Node_p node_pt = NULL, nxt_pt = NULL;

    for (i = 0; i < 8; i++) point[i] = NULL;
    for (node_pt = start; node_pt != NULL; node_pt = nxt_pt) {
        nxt_pt = node_pt->next;
        if (IS_INDEPENDENT(node_pt)) {
            /* Posto neutro o con S */
            if (point[6] == NULL) {
                point[6] = point[7] = node_pt;
                point[7]->next = NULL;
            }
            else {
                node_pt->next = point[6];
                point[6] = node_pt;
            }
        }/* Posto neutro o con S */
        else {
            /* Posto normale */
            switch (node_pt->type) {
            case PROJECTION :   if (point[0] == NULL) {
                    point[0] = point[1] = node_pt;
                    point[1]->next = NULL;
                }
                else {
                    node_pt->next = point[0];
                    point[0] = node_pt;
                }
                break;
            case COMPLEX :	    if (point[2] == NULL) {
                    point[2] = point[3] = node_pt;
                    point[3]->next = NULL;
                }
                else {
                    node_pt->next = point[2];
                    point[2] = node_pt;
                }
                break;
            default :	    if (point[4] == NULL) {
                    point[4] = point[5] = node_pt;
                    point[5]->next = NULL;
                }
                else {
                    node_pt->next = point[4];
                    point[4] = node_pt;
                }
                break;
            }
        }/* Posto normale */
    }
    for (i = 0; i < 8;)
        if (point[i] != NULL) {
            for (j = i + 2; j < 8; j += 2)
                if (point[j] != NULL)
                    break;
            if (j < 8)
                point[i + 1]->next = point[j];
            else
                point[i + 1]->next = NULL;
            i = j;
        }
        else
            i += 2;
    for (i = 0; i < 8; i += 2)
        if (point[i] != NULL) {
            node_pt = point[i];
            break;
        }
    switch (type) {
    case INHIBITOR : tabt[ntr].inibptr = node_pt;
        break;
    case INPUT :     tabt[ntr].inptr = node_pt;
        break;
    case OUTPUT :    tabt[ntr].outptr = node_pt;
        break;
    }
}/* End order_nodes */
#endif
#ifdef GSPN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_test_arc(Node_p my_node_ptr,
                       int type,
                       int tr) {
    /* Init is_test_arc */
    int ret_value = FALSE;
    int my_pl, other_pl;
    Node_p list, other_ptr;

#ifdef ARCO_CON_ESPRESSIONE_PER_GSPN
    if (type == INPUT)
        list = get_place_list(tr, OUTPUT);
    else
        list = get_place_list(tr, INPUT);
    my_pl = GET_PLACE_INDEX(my_node_ptr);
#ifdef ARCO_CON_ESPRESSIONE_PER_GSPN
    Coeff_p my_function, other_function;
    my_function = GET_PLACE_FUNCTION(my_node_ptr);
#endif
    for (other_ptr = list; other_ptr != NULL; other_ptr = NEXT_NODE(other_ptr)) {
        other_pl = GET_PLACE_INDEX(other_ptr);
        if (my_pl == other_pl) {
            if (GET_ARC_MOLTEPLICITY(my_node_ptr) == GET_ARC_MOLTEPLICITY(other_ptr)) {
                ret_value = TRUE;
                goto ret;
#ifdef ARCO_CON_ESPRESSIONE_PER_GSPN
                other_function = GET_PLACE_FUNCTION(other_ptr);
                if (are_the_same_function(my_function, other_function, my_pl, tr)) {
                    ret_value = TRUE;
                    goto ret;
                }
#endif
            }
        }
    }
#endif
ret: return (ret_value);
}/* End is_test_arc */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_node_data_structures(int tr,
                               int type) {
    /* Init fill_node_data_structures */
    Node_p list = NULL;
    Node_p node_ptr = NULL;

    list = node_ptr = get_place_list(tr, type);
    for (; node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
        /* Per ogni nodo della lista */
#ifdef SWN
        node_ptr->type = get_node_type(node_ptr, tr);
        node_ptr->fun_card = get_node_cardinality(node_ptr, tr);
        node_ptr->skip = is_independent_from_instance(node_ptr);
#endif
        if (type == INPUT || type == OUTPUT)
            node_ptr->test_arc = is_test_arc(node_ptr, type, tr);
#ifdef SWN
        if (!node_ptr->skip && type == INHIBITOR)
            node_ptr->type = COMPLEX;
#endif
    }/* Per ogni nodo della lista */
#ifdef SWN
    order_nodes(list, type);
#endif
}/* End fill_node_data_structures */
