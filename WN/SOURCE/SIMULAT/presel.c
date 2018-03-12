# include <stdio.h>
# include <stdlib.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/shared.h"

#ifdef SWN
#ifdef SIMULATION
extern int pre_check();
extern Node_p get_place_list();
extern void check_for_enabling();
extern void adjust_enable_degree();
extern void set_output_components();

extern Event_p new_enabled_list;
extern Event_p old_enabled_list;

static int initial_enabling_degree = MAX_INT;

#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_ordinary_tokens(tok_ptr, pl)
Token_p tok_ptr;
int pl;
{
    /* Init get_ordinary_tokens */
    int ret_value = 1;
    int ii = GET_PLACE_COMPONENTS(pl);
    int cl, ss, ds;

    for (; ii ; ii--) {
        /* Per ogni componente del dominio del posto */
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
        if (IS_UNORDERED(cl)) {
            /* Classe non ordinata */
            ds = tok_ptr->id[ii - 1];
            ss = get_static_subclass(cl, ds);
            ret_value *= GET_CARD(cl, ss, ds);
        }/* Classe non ordinata */
    }/* Per ogni componente del dominio del posto */
    return (ret_value);
}/* End get_ordinary_tokens */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static Token_p get_preselected_token(node_ptr, tr, total)
Node_p node_ptr;
int tr;
int *total;
{
    /* Init get_preselected_token */
    int pl = GET_PLACE_INDEX(node_ptr);
    int arc_molt = GET_FUNCTION_CARDINALITY(node_ptr);
    int server = GET_TRANSITION_SERVER(tr);
    int ratio;

    Token_p tok_ptr = NULL;

    int ret_int = 0;
    Token_p ret_ptr = NULL;

    if (arc_molt == 1 && server == 1) {
        /* Single_server ---> total = different tokens */
#ifdef COLOURED
        ret_int = net_mark[pl].different;
#endif
#ifdef SYMBOLIC
        ret_int = net_mark[pl].ordinary;
#endif
    }/* Single_server ---> total = different tokens */
    else if (arc_molt == 1 && server == MAX_INT) {
        /* Infinite_server ---> total = total tokens */
        ret_int = net_mark[pl].total;
    }/* Infinite_server ---> total = total tokens */
    else {
        /* Molteplicita' > 1 e/o K-server */
        for (tok_ptr = net_mark[pl].marking; tok_ptr != NULL;) {
            /* Per ogni token della marcatura */
            ratio = tok_ptr->molt / arc_molt;
            if (ratio)
#ifdef COLOURED
                ret_int += MIN(ratio, server);
#endif
#ifdef SYMBOLIC
            ret_int += (MIN(ratio, server) * get_ordinary_tokens(tok_ptr, pl));
#endif
            tok_ptr = NEXT_TOKEN(tok_ptr);
        }/* Per ogni token della marcatura */
    }/* Molteplicita' > 1 e/o K-server */
    if (ret_int) {
        /* Il posto abilita la transizione */
        int chosen = UNKNOWN;
        int sum = 0;

        *total = ret_int;
        chosen = GET_RANDOM_INTEGER(1, ret_int);
        if (arc_molt == 1 && server == 1) {
            /* Single_server ---> total = different tokens */
            for (tok_ptr = net_mark[pl].marking; tok_ptr != NULL && sum < chosen;) {
                /* Per ogni token */
#ifdef COLOURED
                sum++;
#endif
#ifdef SYMBOLIC
                sum += get_ordinary_tokens(tok_ptr, pl);
#endif
                ret_ptr = tok_ptr;
                tok_ptr = NEXT_TOKEN(tok_ptr);
            }/* Per ogni token */
        }/* Single_server ---> total = different tokens */
        else if (arc_molt == 1 && server == MAX_INT) {
            /* Infinite_server ---> total = total tokens */
            for (tok_ptr = net_mark[pl].marking; tok_ptr != NULL && sum < chosen;) {
                /* Per ogni token */
#ifdef COLOURED
                sum += tok_ptr->molt;
#endif
#ifdef SYMBOLIC
                sum += tok_ptr->molt * get_ordinary_tokens(tok_ptr, pl);
#endif
                ret_ptr = tok_ptr;
                tok_ptr = NEXT_TOKEN(tok_ptr);
            }/* Per ogni token */
        }/* Infinite_server ---> total = total tokens */
        else {
            /* Molteplicita' > 1 e/o K-server */
            for (tok_ptr = net_mark[pl].marking; tok_ptr != NULL && sum < chosen;) {
                /* Per ogni token */
                ratio = tok_ptr->molt / arc_molt;
                if (ratio)
#ifdef COLOURED
                    sum += MIN(ratio, server);
#endif
#ifdef SYMBOLIC
                sum += (MIN(ratio, server) * get_ordinary_tokens(tok_ptr, pl));
#endif
                ret_ptr = tok_ptr;
                tok_ptr = NEXT_TOKEN(tok_ptr);
            }/* Per ogni token */
        }/* Molteplicita' > 1 e/o K-server */
    }/* Il posto abilita la transizione */
    return (ret_ptr);
}/* End get_preselected_token */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void test_enabl(tr, type, result)
int tr;
int type;
struct RESULT *result;
{
    /* Init test_enabl */
    int mapper[MAX_DOMAIN];

    Node_p node_ptr = NULL;

    Token_p tok_ptr = NULL;

    int enabled = TRUE;
    int pl, total = 0;

    node_ptr = GET_INPUT_LIST(tr);
    for (; node_ptr != NULL && enabled ;) {
        /* Per ogni posto */
        pl = GET_PLACE_INDEX(node_ptr);
        if (!IS_NEUTRAL(pl) && !IS_INDEPENDENT(node_ptr)) {
            /* Posto con dominio colorato */
            project_function_on_domain(node_ptr, PROJECTION, tr, mapper);
            if ((tok_ptr = get_preselected_token(node_ptr, tr, &total)) != NULL) {
                /* Posto che abilita */
                project_token_to_instance(tok_ptr, old_enabled_list, mapper, pl);
                old_enabled_list->enabling_degree *= total;
            }/* Posto che abilita */
            else {
                /* Transizione disabilitata */
                enabled = FALSE;
                dispose_old_event(old_enabled_list);
                old_enabled_list = NULL;
            }/* Transizione disabilitata */
        }/* Posto con dominio colorato */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto */
    result->hold = enabled;
    result->next = NULL;
    result->list = old_enabled_list;
}/* End test_enabl */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void compute_instances_by_preselection(tr, result_ptr)
int tr;
Result_p result_ptr;
{
    /* Init compute_instances_by_preselection */
    result_ptr->list = NULL;
    result_ptr->next = NULL;
    result_ptr->hold = FALSE;
    if ((initial_enabling_degree = pre_check(tr))) {
        /* Non ci sono condizioni semplici di disabilitazione */
        init_enabling_phase(tr); /* skeleton initializaion */
        old_enabled_list = get_new_event(tr);
        if (initial_enabling_degree == MAX_INT)
            initial_enabling_degree = 1;
        old_enabled_list->enabling_degree = initial_enabling_degree;
        test_enabl(tr, INPUT, result_ptr);
        if (result_ptr->hold) {
            set_output_components(tr, result_ptr);
            adjust_enable_degree(result_ptr->list);
        }
    }/* Non ci sono condizioni semplici di disabilitazione */
}/* End compute_instances_by_preselection */
#endif
#endif
