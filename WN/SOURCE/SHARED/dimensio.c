#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"

extern Node_p get_place_list();

#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_cardinality() {
    /* Init get_max_cardinality */
    int ret_value = 0;
    int ii;

    for (ii = 0; ii < ncl ; ii++)
        if (GET_CLASS_CARDINALITY(ii) > ret_value)
            ret_value = GET_CLASS_CARDINALITY(ii);
    return (ret_value);
}/* End get_max_cardinality */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_place_domain() {
    /* Init get_max_place_domain */
    int ret_value = 0;
    int ii;

    for (ii = 0; ii < npl ; ii++)
        if (GET_PLACE_COMPONENTS(ii) > ret_value)
            ret_value = GET_PLACE_COMPONENTS(ii);
    return (ret_value);
}/* End get_max_place_domain */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int at_least_one_sync_coefficient(node_ptr, pos, sb)
Node_p node_ptr;
int pos;
int sb;
{
    /* Init at_least_one_sync_coefficient */
    int ret_value = FALSE;
    Coeff_p coef_ptr = GET_PLACE_FUNCTION(node_ptr);

    for (; coef_ptr != NULL; coef_ptr = NEXT_TUPLE(coef_ptr)) {
        /* Per ogni elemento della C.L. */
        if (GET_SUBCLASS_COEFFICIENT(coef_ptr, pos, sb) != 0
#ifdef FLUSH_FUNCTION
                && GET_SUBCLASS_COEFFICIENT(coef_ptr, pos, sb) != FLUSH_VALUE
#endif
           ) {
            ret_value = TRUE;
            goto ret;
        }
    }/* Per ogni elemento della C.L. */
ret: return (ret_value);
}/* End at_least_one_sync_coefficient */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int at_least_one_coefficient(node_ptr, pos, rp, type)
Node_p node_ptr;
int pos;
int rp;
int type;
{
    /* Init at_least_one_coefficient */
    int ret_value = FALSE;
    Coeff_p coef_ptr = GET_PLACE_FUNCTION(node_ptr);

    for (; coef_ptr != NULL; coef_ptr = NEXT_TUPLE(coef_ptr)) {
        /* Per ogni elemento della C.L. */
        switch (type) {
        case PROJECTION  :
            if (GET_PROJECTION_COEFFICIENT(coef_ptr, pos, rp) != 0) {
                ret_value = TRUE;
                goto ret;
            }
            break;
        case SUCCESSOR :
            if (GET_SUCCESSOR_COEFFICIENT(coef_ptr, pos, rp) != 0) {
                ret_value = TRUE;
                goto ret;
            }
            break;
        case PREDECESSOR :
            if (GET_PREDECESSOR_COEFFICIENT(coef_ptr, pos, rp) != 0) {
                ret_value = TRUE;
                goto ret;
            }
            break;
        }
    }/* Per ogni elemento della C.L. */
ret: return (ret_value);
}/* End at_least_one_coefficient */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_dimension(int  pos,  int  type) {
    /* Init get_max_dimension */
    Node_p node_ptr = NULL;
    int ret_value = 0, count;
    int tr, pl, rp, sb, cl;

    for (tr = 0; tr < ntr ; tr++)
        for (node_ptr = get_place_list(tr, type) ; node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
            /* Per ogni nodo */
            pl = GET_PLACE_INDEX(node_ptr);
            if (GET_PLACE_COMPONENTS(pl) > pos) {
                /* Il posto ha un dominio sufficientemente grande */
                cl = GET_COLOR_COMPONENT(pos, pl); count = 0;
                for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++) {
                    /* Per ogni ripetizione di colore */
                    if (count < GET_CLASS_CARDINALITY(cl)) {
                        /* Se ha ancora senso contare */
                        if (at_least_one_coefficient(node_ptr, pos, rp, PROJECTION))
                            count++;
                        if (GET_CLASS_TYPE(cl) == ORDERED) {
                            /* Per le classi ordinate */
                            if (at_least_one_coefficient(node_ptr, pos, rp, SUCCESSOR))
                                count++;
                            if (at_least_one_coefficient(node_ptr, pos, rp, PREDECESSOR))
                                count++;
                        }/* Per le classi ordinate */
                    }/* Se ha ancora senso contare */
                    else
                        count = GET_CLASS_CARDINALITY(cl);
                }/* Per ogni ripetizione di colore */
                for (sb = 0; sb < GET_STATIC_SUBCLASS(cl); sb++) {
                    /* Per ogni sottoclasse statica */
                    if (count < GET_CLASS_CARDINALITY(cl)) {
                        /* Se ha ancora senso contare */
                        if (at_least_one_sync_coefficient(node_ptr, pos, sb))
                            count += GET_STATIC_CARDINALITY(cl, sb);
                    }/* Se ha ancora senso contare */
                    else
                        count = GET_CLASS_CARDINALITY(cl);
                }/* Per ogni sottoclasse statica */
                ret_value = MAX(ret_value, count);
            }/* Il posto ha un dominio sufficientemente grande */
        }/* Per ogni nodo */
    return (ret_value);
}/* End get_max_dimension */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_elements(int  pos) {
    /* Init get_max_elements */
    int ret_value = 0;

    ret_value = MAX(ret_value, get_max_dimension(pos, INPUT));
    ret_value = MAX(ret_value, get_max_dimension(pos, INHIBITOR));
    ret_value = MAX(ret_value, get_max_dimension(pos, OUTPUT));
    return (ret_value);
}/* End get_max_elements */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_transition_domain() {
    /* Init get_max_transition_domain */
    int ret_value = 0;
    int ii;

    for (ii = 0; ii < ntr ; ii++)
        if (GET_TRANSITION_COMPONENTS(ii) > ret_value)
            ret_value = GET_TRANSITION_COMPONENTS(ii);
    return (ret_value);
}/* End get_max_transition_domain */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_colour_repetitions() {
    /* Init get_max_colour_repetitions */
    int ret_value = 0;
    int ii, cl;

    for (ii = 0; ii < ntr ; ii++)
        for (cl = 0; cl < ncl ; cl++)
            if (GET_COLOR_REPETITIONS(cl, ii) > ret_value)
                ret_value = GET_COLOR_REPETITIONS(cl, ii);
    return (ret_value);
}/* End get_max_colour_repetitions */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_terms(int  type) {
    /* Init get_max_terms */
    int tr, cnt;
    int ret_value = 0;
    Node_p node_ptr = NULL;
    Coeff_p coef_ptr = NULL;

    for (tr = 0; tr < ntr ; tr++)
        for (node_ptr = get_place_list(tr, type) ; node_ptr != NULL ;) {
            /* Per ogni nodo */
            coef_ptr = GET_PLACE_FUNCTION(node_ptr);
            for (cnt = 0; coef_ptr != NULL; cnt++, coef_ptr = NEXT_TUPLE(coef_ptr));
            ret_value = MAX(ret_value, cnt);
            node_ptr = NEXT_NODE(node_ptr);
        }/* Per ogni nodo */
    return (ret_value);
}/* End get_max_terms */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_max_cl() {
    /* Init get_max_cl */
    int ret_value = 0;

    ret_value = MAX(ret_value, get_max_terms(INPUT));
    ret_value = MAX(ret_value, get_max_terms(INHIBITOR));
    ret_value = MAX(ret_value, get_max_terms(OUTPUT));
    return (ret_value);
}/* End get_max_cl */
#endif
