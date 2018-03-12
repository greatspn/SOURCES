/* Funzioni usate in fire.c, after.c, shared1.c, increm.c, enabling.c */
#include <stdlib.h>
# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/macros.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/service.h"

extern int nextvec();
extern int lesser();
extern int greater();
extern int is_in_function();
extern int get_static_subclass();
extern int get_prec();
extern int verify_predicate_constraint();
extern int verify_marking_constraint();

extern int evaluate_expression();

extern void get_pointer_info();
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int present(int  elem,  int  *addr,  int  ptr) {
    /* Init present */
    int i;

    for (i = 0; i < ptr + 1 && addr[i] != elem; i++);
    if (i == ptr + 1)
        return (FALSE);
    else
        return (i + 1);
}/* End present */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_marking_of(Cart_p  cartes_ptr,  int  pl,  int  type) {
    /* Init get_marking_of */
    int marking_value = 0;

#ifdef GSPN
    marking_value = net_mark[pl].total;
#endif
#ifdef SWN
    if (IS_NEUTRAL(pl)) {
        /* Posto neutro */
        marking_value = (net_mark[pl].marking)->molt;
    }/* Posto neutro */
    else {
        /* Posto con dominio di colore */
        struct TOKEN_INFO result;
        TokInfo_p res_ptr = &result;

        get_pointer_info(cartes_ptr , pl , res_ptr, type);
        if (res_ptr->pos != NULL)
            marking_value = (res_ptr->pos)->molt;
    }/* Posto con dominio di colore */
#endif
    return (marking_value);
}/* End get_marking_of */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_cartesian_product(Cart_p  cartes_ptr,  Node_p  node_ptr,  Event_p  instance_ptr,  int  *pred_val,  int  element_type) {
    /* Init init_cartesian_product */
    int pl;
    int tr;
    int domain_comp, max;
    int i, k, cl, sb, ind, type, rp, pos, el, *list;

#ifdef SWN
    Coeff_p coeff_ptr = NULL;

    pl = GET_PLACE_INDEX(node_ptr);
    tr = GET_TRANSITION_INDEX(instance_ptr);
    domain_comp = GET_PLACE_COMPONENTS(pl);
    for (i = domain_comp; i ; i--) {
        /* Per ogni classe del dominio del posto */
        cartes_ptr->up[i - 1] = -1;
        cartes_ptr->low[i - 1] = 0;
    }/* Per ogni classe del dominio del posto */
    type = GET_LABELLING_FUNCTION_TYPE(node_ptr);
    switch (type) {
    /* Tipo di funzione */
    case PROJECTION :
        for (i = domain_comp; i ; i--) {
            /* Per ogni classe del dominio del posto */
            cartes_ptr->up[i - 1] = 0;
            cl = GET_COLOR_COMPONENT(i - 1, pl);
            for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++)
                if (is_in_function(rp, i - 1, node_ptr))
                    break;
            pos = GET_POSITION_OF(cl, rp, tr);
#ifdef SYMBOLIC
#ifdef SIMULATION
            if (IS_UNORDERED(cl)) {
                /* Classe non ordinata */
                if (instance_ptr->split[pos] != UNKNOWN)
                    cartes_ptr->mark[i - 1][0] = instance_ptr->split[pos];
                else
                    cartes_ptr->mark[i - 1][0] = instance_ptr->npla[pos];
            }/* Classe non ordinata */
            else
#endif
#ifdef REACHABILITY
                if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
                    /* Classe non ordinata */
                    if (instance_ptr->split[pos] != UNKNOWN)
                        cartes_ptr->mark[i - 1][0] = instance_ptr->split[pos];
                    else
                        cartes_ptr->mark[i - 1][0] = instance_ptr->npla[pos];
                }/* Classe non ordinata */
                else
#endif
#endif
                    cartes_ptr->mark[i - 1][0] = instance_ptr->npla[pos];
        }/* Per ogni classe del dominio del posto */
        break;
    case ONLY_S :
        coeff_ptr = GET_PLACE_FUNCTION(node_ptr);
        for (i = domain_comp; i ; i--) {
            /* Per ogni classe del dominio del posto */
            cl = GET_COLOR_COMPONENT(i - 1, pl);
            for (sb = 0; sb < GET_STATIC_SUBCLASS(cl) ; sb++) {
                /* Per ogni sottoclasse statica */
                if (GET_SUBCLASS_COEFFICIENT(coeff_ptr, i - 1, sb) != 0) {
                    /* Coefficiente della funzione S relativa <> 0 */
                    el = GET_STATIC_OFFSET(cl, sb);
#ifdef SYMBOLIC
                    if (IS_UNORDERED(cl))
                        max = GET_NUM_SS(cl, sb);
                    else
#endif
                        max = GET_STATIC_CARDINALITY(cl, sb);
                    for (k = 0; k < max; k++) {
                        cartes_ptr->up[i - 1]++;
                        cartes_ptr->mark[i - 1][cartes_ptr->up[i - 1]] = el;
                        el++;
                    }
                }/* Coefficiente della funzione S relativa <> 0 */
            }/* Per ogni sottoclasse statica */
        }/* Per ogni classe del dominio del posto */
        break;
    case COMPLEX :
    case GUARDED :
        ind = 0;
        coeff_ptr = GET_PLACE_FUNCTION(node_ptr);
        for (; coeff_ptr != NULL ;) {
            /* C.L */
            if (pred_val[ind]) {
                /* Predicato vero */
                for (i = domain_comp; i ; i--) {
                    /* Per ogni classe del dominio del posto */
                    cl = GET_COLOR_COMPONENT(i - 1, pl);
                    list = cartes_ptr->mark[i - 1];
                    /*limit = cartes_ptr->up[i-1];*/
                    for (sb = 0; sb < GET_STATIC_SUBCLASS(cl) ; sb++) {
                        /* Per ogni sottoclasse statica */
                        if (GET_SUBCLASS_COEFFICIENT(coeff_ptr, i - 1, sb) != 0) {
                            /* Coefficiente della funzione S relativa <> 0 */
                            el = GET_STATIC_OFFSET(cl, sb);
#ifdef SYMBOLIC
                            if (IS_UNORDERED(cl))
                                max = GET_NUM_SS(cl, sb);
                            else
#endif
                                max = GET_STATIC_CARDINALITY(cl, sb);
                            for (k = 0; k < max; k++) {
                                if (!present(el, list, cartes_ptr->up[i - 1])) {
                                    cartes_ptr->up[i - 1]++;
                                    cartes_ptr->mark[i - 1][cartes_ptr->up[i - 1]] = el;
                                }
                                el++;
                            }
                        }/* Coefficiente della funzione S relativa <> 0 */
                    }/* Per ogni sottoclasse statica */
                    for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++) {
                        /* Per ogni ripetizione */
                        pos = GET_POSITION_OF(cl, rp, tr);
                        if (GET_PROJECTION_COEFFICIENT(coeff_ptr, i - 1, rp) != 0) {
                            /* Coefficiente x <> 0 */
#ifdef SYMBOLIC
#ifdef SIMULATION
                            if (IS_UNORDERED(cl)) {
                                /* Classe non ordinata */
                                if (instance_ptr->split[pos] != UNKNOWN)
                                    el = instance_ptr->split[pos];
                                else
                                    el = instance_ptr->npla[pos];
                            }/* Classe non ordinata */
                            else
#endif
#ifdef REACHABILITY
                                if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
                                    /* Classe non ordinata */
                                    if (instance_ptr->split[pos] != UNKNOWN)
                                        el = instance_ptr->split[pos];
                                    else
                                        el = instance_ptr->npla[pos];
                                }/* Classe non ordinata */
                                else
#endif
#endif
                                    el = instance_ptr->npla[pos];
                            if (!present(el, list, cartes_ptr->up[i - 1])) {
                                cartes_ptr->up[i - 1]++;
                                cartes_ptr->mark[i - 1][cartes_ptr->up[i - 1]] = el;
                            }
                        }/* Cofficiente x <> 0 */
                        if (GET_CLASS_TYPE(cl) == ORDERED) {
                            /* Calcolo molteplicita' per funzioni successore */
                            if (GET_SUCCESSOR_COEFFICIENT(coeff_ptr, i - 1, rp) != 0) {
                                /* Coefficiente !x <>0 */
#ifdef SYMBOLIC
#ifdef REACHABILITY
                                if (GET_STATIC_SUBCLASS(cl) == 1) {
                                    if (element_type == TRUE)
                                        el = (instance_ptr->split[pos] + 1) % GET_NUM_CL(cl);
                                    else
                                        el = (instance_ptr->split[pos] + 1) % tabc[cl].card;
                                }
                                else
#endif
#endif
                                    el = (instance_ptr->npla[pos] + 1) % tabc[cl].card;
                                if (!present(el, list, cartes_ptr->up[i - 1])) {
                                    cartes_ptr->up[i - 1]++;
                                    cartes_ptr->mark[i - 1][cartes_ptr->up[i - 1]] = el;
                                }
                            }/* Coefficiente !x <>0 */
                            if (GET_PREDECESSOR_COEFFICIENT(coeff_ptr, i - 1, rp) != 0) {
                                /* Coefficiente ^x <>0 */
#ifdef SYMBOLIC
#ifdef REACHABILITY
                                if (GET_STATIC_SUBCLASS(cl) == 1) {
                                    if (element_type == TRUE) {
                                        el = get_object_from_dynamic(cl, instance_ptr->split[pos]);
                                        el = get_dynamic_from_object(cl, get_prec(cl, el));
                                    }
                                    else
                                        el = get_prec(cl, instance_ptr->split[pos]);
                                }
                                else
#endif
#endif
                                    el = get_prec(cl, instance_ptr->npla[pos]);
                                if (!present(el, list, cartes_ptr->up[i - 1])) {
                                    cartes_ptr->up[i - 1]++;
                                    cartes_ptr->mark[i - 1][cartes_ptr->up[i - 1]] = el;
                                }
                            }/* Coefficiente ^x <>0 */
                        }/* Calcolo molteplicita' per funzioni successore */
                    }/* Per ogni ripetizione */
                }/* Per ogni classe del dominio del posto */
            }/* Predicato vero */
            ind++;
            coeff_ptr = NEXT_TUPLE(coeff_ptr);
        }/* C.L */
        break;
    }/* Tipo di funzione */
#endif
}/* End init_cartesian_product */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int verify_marking_constraint_of_all_places(Node_p  start_ptr,  Event_p  instance_ptr,  int  tr_index,  int  type) {
    /* Init verify_marking_constraint_of_all_places */

    int flag = TRUE;
    Node_p node_ptr = start_ptr;


    for (; node_ptr != NULL && flag ;) {
        /* Per ogni posto */
        flag = verify_marking_constraint(node_ptr, instance_ptr, tr_index, type);
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto */
    return (flag);
}/* End verify_marking_constraint_of_of_all_places */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_request_for(Cart_p  cartes_ptr,  Node_p  node_ptr,  Event_p  instance_ptr,  int  *pred_val,  int  element_type) {
    /* Init get_request_for */

    int pl;
    int tr;
    int eq, cl, ret_value = 0, prod, sum, pos, i, rp, sb, type;
    int ind, obj1, obj2, domain_comp;
#ifdef GSPN
    int arc_molt;
#endif
    Coeff_p coeff_ptr = NULL;


    pl = GET_PLACE_INDEX(node_ptr);
    tr = GET_TRANSITION_INDEX(instance_ptr);
#ifdef GSPN
    if ((coeff_ptr = GET_PLACE_FUNCTION(node_ptr)) != NULL) {
        /* Arco con [P1] ex1+-... [Pn] exn */
        for (arc_molt = 0 ; coeff_ptr != NULL ; coeff_ptr = coeff_ptr->next)
            if (verify_predicate_constraint(coeff_ptr->guard, NULL))
                arc_molt += (coeff_ptr->enn_coef * evaluate_expression(coeff_ptr->expression, NULL));
        if (arc_molt >= 0)
            ret_value = (node_ptr->molt * arc_molt);
        else if (arc_molt < 0) {
            fprintf(stdout, "Runtime error: arc molteplicity < 0 in fire (place %s transition %s)\n", PLACE_NAME(pl), TRANS_NAME(tr));
            exit(1);
        }
    }/* Arco con [P1] ex1+-... [Pn] exn */
    else
        ret_value = node_ptr->molt;
#endif
#ifdef SWN

    if (IS_NEUTRAL(pl)) {
        /* Posto neutro */
        if (node_ptr->molt > 0)
            ret_value = node_ptr->molt;
        else
            ret_value = net_mark[-node_ptr->molt - 1].total;
    }/* Posto neutro */
    else {
        /* Posto con dominio di colore */
        type = GET_LABELLING_FUNCTION_TYPE(node_ptr);
        switch (type) {
        /* Tipo di funzione */
        case PROJECTION :
            ret_value = GET_FUNCTION_CARDINALITY(node_ptr);
            break;
        case ONLY_S:
            ret_value = 1;
            break;
        case COMPLEX:
        case GUARDED:
            ret_value = 0;
            coeff_ptr = GET_PLACE_FUNCTION(node_ptr);
            ind = 0;
            domain_comp = GET_PLACE_COMPONENTS(pl);
            while (coeff_ptr != NULL) {
                /* C.L */
                if (pred_val[ind]) {
                    /* Predicato della funzione vero */
                    sum = 0;
                    prod = 1;
                    for (i = 0; i < domain_comp ; i++) {
                        /* Calcolo molteplicita' */
                        cl = GET_COLOR_COMPONENT(i, pl);
                        eq = FALSE;
                        obj1 = cartes_ptr->mark[i][cartes_ptr->low[i]];
                        for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++) {
                            /* Per ogni ripetizione di colore */
                            pos = GET_POSITION_OF(cl, rp, tr);
#ifdef SYMBOLIC
#ifdef SIMULATION
                            if (IS_UNORDERED(cl)) {
                                /* Classe non ordinata */
                                if (instance_ptr->split[pos] != UNKNOWN)
                                    obj2 = instance_ptr->split[pos];
                                else
                                    obj2 = instance_ptr->npla[pos];
                            }/* Classe non ordinata */
                            else
#endif
#ifdef REACHABILITY
                                if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
                                    /* Classe non ordinata */
                                    if (instance_ptr->split[pos] != UNKNOWN)
                                        obj2 = instance_ptr->split[pos];
                                    else
                                        obj2 = instance_ptr->npla[pos];
                                }/* Classe non ordinata */
                                else
#endif
#endif
                                    obj2 = instance_ptr->npla[pos];
                            if (obj1 == obj2) {
                                /* Calcolo con il coefficiente della x */
                                sum += GET_PROJECTION_COEFFICIENT(coeff_ptr, i, rp);
                                eq = pos + 1;
                            }/* Calcolo con il coefficiente della x */
                        }/* Per ogni ripetizione di colore */
                        if (GET_CLASS_TYPE(cl) == ORDERED) {
                            /* Calcolo molteplicita' per funzioni successore */
                            for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++) {
                                /* Per ogni ripetizione di colore */
                                pos = GET_POSITION_OF(cl, rp, tr);
#ifdef SYMBOLIC
#ifdef REACHABILITY
                                if (GET_STATIC_SUBCLASS(cl) == 1) {
                                    if (element_type == TRUE)
                                        obj2 = (instance_ptr->split[pos] + 1) % GET_NUM_CL(cl);
                                    else
                                        obj2 = (instance_ptr->split[pos] + 1) % tabc[cl].card;
                                }
                                else
#endif
#endif
                                    obj2 = (instance_ptr->npla[pos] + 1) % tabc[cl].card;
                                if (obj1 == obj2) {
                                    sum += GET_SUCCESSOR_COEFFICIENT(coeff_ptr, i, rp);
                                    eq = pos + 1;
                                }
#ifdef SYMBOLIC
#ifdef REACHABILITY
                                if (GET_STATIC_SUBCLASS(cl) == 1) {
                                    if (element_type == TRUE) {
                                        obj2 = get_object_from_dynamic(cl, instance_ptr->split[pos]);
                                        obj2 = get_dynamic_from_object(cl, get_prec(cl, obj2));
                                    }
                                    else
                                        obj2 = get_prec(cl, instance_ptr->split[pos]);
                                }
                                else
#endif
#endif
                                    obj2 = get_prec(cl, instance_ptr->npla[pos]);
                                if (obj1 == obj2) {
                                    sum += GET_PREDECESSOR_COEFFICIENT(coeff_ptr, i, rp);
                                    eq = pos + 1;
                                }
                            }/* Per ogni ripetizione di colore */
                        }/* Calcolo molteplicita' per funzioni successore */
                        if (eq) {
                            /* Uguale ad un istanziata */
#ifdef SYMBOLIC
                            if (IS_UNORDERED(cl)) {
                                /* Classe non ordinata */
                                sb = get_static_subclass(cl, DECODE_ID(obj1));
                            }/* Classe non ordinata */
                            else
#endif
                                sb = get_static_subclass(cl, obj1);
                            sum += GET_SUBCLASS_COEFFICIENT(coeff_ptr, i, sb);
                        }/* Uguale ad un istanziata */
                        else {
                            /* Diversa dalle istanziate */
                            pos = get_static_subclass(cl, obj1);
                            sum = GET_SUBCLASS_COEFFICIENT(coeff_ptr, i, pos);
                        }/* Diversa dalle istanziate */
                        prod *= sum;
                        sum = 0;
                    }/* Calcolo molteplicita' */
                    prod *= GET_TUPLE_COEFFICIENT(coeff_ptr);
                    ret_value += prod;
                }/* Predicato della funzione vero */
                coeff_ptr = NEXT_TUPLE(coeff_ptr);
                ind++;
            }/* C.L */
            break;
        }/* Tipo di funzione */
    }/* Posto con dominio di colore */
#endif
    return (ret_value);
}/* End get_request_for */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int at_least_one_predicate(Node_p  node_ptr,  Event_p  instance_ptr,  int  *pred_val) {
    /* Init at_least_one_predicate */
#ifdef GSPN
    int do_it = TRUE;  /**** Da rivedere con funzioni su archi in GSPN ****/
#endif
#ifdef SWN
    Coeff_p coeff_ptr = NULL;

    int i = 0;
    int do_it = FALSE;

    if (node_ptr->type == GUARDED) {
        /* Pre calcolo dei valori dei predicati */
        for (coeff_ptr = node_ptr->arcfun; coeff_ptr != NULL; coeff_ptr = coeff_ptr->next) {
            /* Per ogni elemento della C.L */
            pred_val[i] = verify_predicate_constraint(coeff_ptr->guard, instance_ptr);
            do_it |= pred_val[i++];
        }/* Per ogni elemento della C.L */
    }/* Pre calcolo dei valori dei predicati */
    else {
        /* Inizializzazione valori dei predicati */
        do_it = TRUE;
        for (i = max_cl ; i ; pred_val[--i] = TRUE);
    }/* Inizializzazione valori dei predicati */
#endif
    return (do_it);
}/* End at_least_one_predicate */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int verify_marking_constraint(Node_p  node_ptr,  Event_p  instance_ptr,  int  tr,  int  type) {
    /* Init verify_marking_constraint */
    int enabled = TRUE;

    if (at_least_one_predicate(node_ptr, instance_ptr, pred_val)) {
        /* Almeno un predicato della C.L e' vero */
        int pl, card, req_value, mrk_value;
        int (*magnitude)();

        if (type != INHIBITOR)
            magnitude = lesser;
        else
            magnitude = greater;
        pl = GET_PLACE_INDEX(node_ptr);
#ifdef SWN
        card = GET_PLACE_COMPONENTS(pl);
        init_cartesian_product(cart_place_ptr, node_ptr, instance_ptr, pred_val, FALSE);
        do {
            /* Ciclo di verifica per ogni ennupla possibile del posto */
#endif
            req_value = get_request_for(cart_place_ptr, node_ptr, instance_ptr, pred_val, FALSE);
            mrk_value = get_marking_of(cart_place_ptr, pl, FALSE);
            if ((*magnitude)(mrk_value, req_value))
                enabled = FALSE;
            if (enabled) {
                /* Se la transizione e' abilitata */
                if (type == INPUT)
                    if (instance_ptr->enabling_degree > 1)
                        if (req_value != 0)
                            instance_ptr->enabling_degree = MIN(mrk_value / req_value , instance_ptr->enabling_degree);
            }/* Se la transizione e' abilitata */
            else
                instance_ptr->enabling_degree = 0;
#ifdef SWN
        }/* Ciclo di verifica per ogni ennupla possibile del posto */
        while (nextvec(cart_place_ptr, card) && enabled);
#endif
    }/* Almeno un predicato della C.L e' vero */
    return (enabled);
}/* End verify_marking_constraint */
