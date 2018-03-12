# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/shared.h"
# include "../../INCLUDE/service.h"


int verify_marking_constraint(Node_p  node_ptr,  Event_p  instance_ptr,  int  tr,  int  type);
void set_output_components(int  tr,  Result_p  result);

extern Event_p new_enabled_list;
extern Event_p old_enabled_list;

extern Event_p old_split;

static int initial_enabling_degree = MAX_INT;

#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void check_for_enabling(Event_p  instance_ptr,  Node_p  node_ptr,  int  type) {
    /* Init check_for_enabling */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    Pred_p pred_ptr = GET_TRANSITION_PREDICATE(tr);

#ifdef SYMBOLIC
#ifdef SIMULATION
    int at_least_one_class_requires_presplit = tabt[tr].pre_split;
#endif
    int enabled;
    Event_p next_ptr;
#endif
    if (verify_predicate_constraint(pred_ptr, instance_ptr)) {
        /* Verifica il predicato */
#ifdef SYMBOLIC
#ifdef SIMULATION
        if (at_least_one_class_requires_presplit) {
            /* Non si puo' fare solo preselection */
            enabled = pre_split_instance(instance_ptr, TOTAL);
            if (enabled) {
                /* Se si sono prodotte delle istanze */
                for (; old_split != NULL ;) {
                    /* Per ogni istanza costruita */
                    next_ptr = old_split->next;
                    if (verify_marking_constraint(node_ptr, old_split, tr, type)) {
                        /* Verifica la marcatura */
                        old_split->next = new_enabled_list;
                        new_enabled_list = old_split;
                    }/* Verifica la marcatura */
                    else
                        dispose_old_event(old_split);
                    old_split = next_ptr;
                }/* Per ogni istanza costruita */
            }/* Se si sono prodotte delle istanze */
        }/* Non si puo' fare solo preselection */
        else {
            /* Solo preselection */
            if (verify_marking_constraint(node_ptr, instance_ptr, tr, type))
                add_instance(instance_ptr);
        }/* Solo preselection */
#endif
#ifdef REACHABILITY
        enabled = pre_split_instance(instance_ptr, TOTAL);
        if (enabled) {
            /* Se si sono prodotte delle istanze */
            for (; old_split != NULL ;) {
                /* Per ogni istanza costruita */
                next_ptr = old_split->next;
                if (verify_marking_constraint(node_ptr, old_split, tr, type)) {
                    /* Verifica la marcatura */
                    old_split->next = new_enabled_list;
                    new_enabled_list = old_split;
                }/* Verifica la marcatura */
                else
                    dispose_old_event(old_split);
                old_split = next_ptr;
            }/* Per ogni istanza costruita */
        }/* Se si sono prodotte delle istanze */
#endif
#endif
#ifdef COLOURED
        if (verify_marking_constraint(node_ptr, instance_ptr, tr, type))
            add_instance(instance_ptr);
#endif
    }/* Verifica il predicato */
}/* End check_for_enabling */
/*************************************************************************/
/* NAME : */
/* DESCRIPTION : Distinguere i casi input e inibitore e ordinamento	 */
/*		 globale (tenere conto del campo type). 		 */
/* PARAMETERS : */
/* RETURN VALUE : NULL se non ci sono nodi.				 */
/*		  NULL quando non ce ne sono piu'; in questo caso        */
/*		  rimettere il campo analyzed = FALSE.			 */
/*		  PUNTATORE al migliore nodo da analizzare e		 */
/*		  mettere il campo analyzed a TRUE.			 */
/*************************************************************************/
static Node_p get_a_node_to_analyze(tr, type, old_node_ptr)
int tr;
int type;
Node_p old_node_ptr;
{
    /* Init get_a_node_to_analyze */
    int function_type = UNKNOWN, min_value = MAX_INT, pl;
    Node_p node_ptr = NULL;
    Node_p min_ptr = NULL;

    for (node_ptr = get_place_list(tr, type); node_ptr != NULL; node_ptr = NEXT_NODE(node_ptr)) {
        /* Per ogni posto */
        if (!node_ptr->analyzed) {
            if (!IS_INDEPENDENT(node_ptr))
                function_type = node_ptr->type;
            break;
        }
    }/* Per ogni posto */
    if (function_type != UNKNOWN) {
        /* C'e' almeno un posto */
        for (node_ptr = get_place_list(tr, type); node_ptr != NULL; node_ptr = NEXT_NODE(node_ptr)) {
            /* Per ogni posto */
            if (!node_ptr->analyzed && !IS_INDEPENDENT(node_ptr)) {
                if (node_ptr->type == function_type) {
                    /* Posto dello stesso tipo di funzione */
                    pl = GET_PLACE_INDEX(node_ptr);
                    if (min_value > net_mark[pl].different) {
                        /* Meno token */
                        min_value = net_mark[pl].different;
                        min_ptr = node_ptr;
                    }/* Meno token */
                }/* Posto dello stesso tipo di funzione */
                else
                    break;
            }
        }/* Per ogni posto */
    }/* C'e' almeno un posto */
    if (min_ptr != NULL)
        min_ptr->analyzed = TRUE;
    return (min_ptr);
}/* End get_a_node_to_analyze */
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
    Event_p trial_ptr = NULL;
    int enabled = TRUE;

    new_enabled_list = NULL;
    trial_ptr = get_new_event(tr);
    if (get_place_list(tr, type) != NULL) {
        /* C'e' almeno un posto */
        int mapper[MAX_DOMAIN];

        Event_p inst_ptr = NULL;

        Node_p node_ptr = NULL;

        Token_p tok_ptr = NULL;

        int pl, domain_component, function_type, generate;

        domain_component = GET_TRANSITION_COMPONENTS(tr);

        node_ptr = get_a_node_to_analyze(tr, type, node_ptr);
        for (; node_ptr != NULL && enabled ;) {
            /* Per ogni posto */
            pl = GET_PLACE_INDEX(node_ptr);
            if (!IS_NEUTRAL(pl) && !IS_INDEPENDENT(node_ptr)) {
                /* Posto con dominio colorato */
                function_type = GET_LABELLING_FUNCTION_TYPE(node_ptr);
                generate = project_function_on_domain(node_ptr, function_type, tr, mapper);
                if (generate) {
                    /* Il posto fissa alcune componenti del dominio */
                    inst_ptr = old_enabled_list;
                    switch (function_type) {
                    /* Tipo di funzione sull'arco */
                    case PROJECTION :
                        for (; inst_ptr != NULL;) {
                            /* Per ogni istanza parziale gia' costruita */
                            copy_event(trial_ptr, inst_ptr);
                            tok_ptr = net_mark[pl].marking;
                            for (; tok_ptr != NULL;) {
                                /* Per ogni token della marcatura */
                                if (project_token_to_instance(tok_ptr, trial_ptr, mapper, pl)) {
                                    /* Token corretto rispetto alle istanze parziali */
                                    check_for_enabling(trial_ptr, node_ptr, type);
                                }/* Token corretto rispetto alle istanze parziali */
                                tok_ptr = NEXT_TOKEN(tok_ptr);
                                trial_ptr->enabling_degree = inst_ptr->enabling_degree;
                            }/* Per ogni token della marcatura */
                            inst_ptr = next_instance(inst_ptr);
                        }/* Per ogni istanza parziale gia' costruita */
                        break;
                    case COMPLEX          :
                        setup_cartesian_product_of_tr(cart_trans_ptr, tr, mapper);
                        for (; inst_ptr != NULL;) {
                            /* Per ogni istanza parziale gia' costruita */
                            copy_event(trial_ptr, inst_ptr);
                            do {
                                /* Per ogni elemento del P.C sul dominio di t */
                                project_element_to_instance(cart_trans_ptr, trial_ptr, mapper);
                                trial_ptr->enabling_degree = inst_ptr->enabling_degree;
                                check_for_enabling(trial_ptr, node_ptr, type);
                            }/* Per ogni elemento del P.C sul dominio di t */
                            while (nextvec(cart_trans_ptr, domain_component));
                            inst_ptr = next_instance(inst_ptr);
                        }/* Per ogni istanza parziale gia' costruita */
                        break;
                    case GUARDED          :
                        setup_cartesian_product_of_tr(cart_trans_ptr, tr, mapper);
                        for (; inst_ptr != NULL;) {
                            /* Per ogni istanza parziale gia' costruita */
                            copy_event(trial_ptr, inst_ptr);
                            do {
                                /* Per ogni elemento del P.C sul dominio di t */
                                project_element_to_instance(cart_trans_ptr, trial_ptr, mapper);
                                trial_ptr->enabling_degree = inst_ptr->enabling_degree;
                                check_for_enabling(trial_ptr, node_ptr, type);
                            }/* Per ogni elemento del P.C sul dominio di t */
                            while (nextvec(cart_trans_ptr, domain_component));
                            inst_ptr = next_instance(inst_ptr);
                        }/* Per ogni istanza parziale gia' costruita */
                        break;
                    }/* Tipo di funzione sull'arco */
                }/* Il posto fissa alcune componenti del dominio */
                else
                    test_old_instances(node_ptr, function_type, tr, type);
                enabled = new_instances_generated();
            }/* Posto con dominio colorato */
            node_ptr = get_a_node_to_analyze(tr, type, node_ptr);
        }/* Per ogni posto */
        for (node_ptr = get_place_list(tr, type); node_ptr != NULL;) {
            /* Rimettere a posto il campo analyzed */
            node_ptr->analyzed = FALSE;
            node_ptr = NEXT_NODE(node_ptr);
        }/* Rimettere a posto il campo analyzed */
    }/* C'e' almeno un posto */
    dispose_old_event(trial_ptr);
    result->hold = enabled;
    result->next = NULL;
    result->list = old_enabled_list;
}/* End test_enabl */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void compute_instances_from_scratch(int  tr,  Result_p  result_ptr) {
    /* Init compute_instances_from_scratch */

    result_ptr->list = NULL;
    result_ptr->next = NULL;
    result_ptr-> hold = FALSE;
#ifdef GSPN
    if ((initial_enabling_degree = pre_check(tr))) {
        /* Non ci sono condizioni semplici di disabilitazione */
        if (verify_predicate_constraint(tabt[tr].guard, NULL)) {
            /* La guardia e' soddisfatta */
            result_ptr->list = get_new_event(tr);
            result_ptr-> hold = TRUE;
            result_ptr->list->enabling_degree = initial_enabling_degree;
            adjust_enable_degree(result_ptr->list);
        }/* La guardia e' soddisfatta */
    }/* Non ci sono condizioni semplici di disabilitazione */
#endif
#ifdef SWN

#ifndef LIBSPOT
#ifdef ESYMBOLIC
    if ((MARKING_TYPE == SATURED_SYM) && IS_ASYMETRIC(tr)) return;
#endif
#endif
    Event_p ev_p = NULL;
    if ((initial_enabling_degree = pre_check(tr))) {
        /* Non ci sono condizioni semplici di disabilitazione */
        init_enabling_phase(tr);
        old_enabled_list = get_new_event(tr);
        old_enabled_list->enabling_degree = initial_enabling_degree;
        test_enabl(tr, INPUT, result_ptr);
        ev_p = result_ptr->list;
        if (result_ptr->hold) {
            test_enabl(tr, INHIBITOR, result_ptr);
            ev_p = result_ptr->list;
            if (result_ptr->hold) {
                set_output_components(tr, result_ptr);
                ev_p = result_ptr->list;
                if (result_ptr->hold) {
                    adjust_enable_degree(result_ptr->list);
                    ev_p = result_ptr->list;
                }
            }
        }
    }/* Non ci sono condizioni semplici di disabilitazione */
#endif
}/* End compute_instances_from_scratch */

