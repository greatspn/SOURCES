# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/shared.h"

#ifdef SWN
#ifdef SIMULATION
extern int pre_check();
extern int compare_input_event_id();
extern int verify_predicate_constraint();
extern int verify_marking_constraint();

extern Event_p new_enabled_list;
extern Event_p old_enabled_list;

static int initial_enabling_degree = MAX_INT;
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void increm_add_instance(in_p)
Event_p in_p;
{
    /* Init increm_add_instance */
    Event_p new_ptr = NULL;
    Event_p ev_p = NULL;
    int tr = GET_TRANSITION_INDEX(in_p);
    int exists = FALSE;
    int i;

    /******** PEZZO SUL QUALE MEDITARE PARECCHIO *************/
    for (ev_p = new_enabled_list; ev_p != NULL; ev_p = ev_p->next)
        if (compare_input_event_id(in_p, ev_p, tr) == EQUAL_TO) {
            if (in_p->enabling_degree)
                if (in_p->enabling_degree != ev_p->enabling_degree)
                    ev_p->enabling_degree = in_p->enabling_degree;
            exists = TRUE;
            goto finish;
        }
    /******** PEZZO SUL QUALE MEDITARE PARECCHIO *************/
finish:
    if (!exists) {
        new_ptr = get_new_event(tr);
        new_ptr->enabling_degree = in_p->enabling_degree;
        new_ptr->next = new_enabled_list;
        new_enabled_list = new_ptr;
        new_enabled_list->trans = tr;
        for (i = tabt[tr].comp_num; i ; i--) {
            new_enabled_list->npla[i - 1] = in_p->npla[i - 1];
#ifdef SYMBOLIC
            new_enabled_list->split[i - 1] = in_p->split[i - 1];
#endif
        }
    }
}/* End increm_add_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int increm_project_token_to_instance(tok_ptr, trial_ptr, mapper, pl)
Token_p tok_ptr;
Event_p trial_ptr;
int mapper[MAX_DOMAIN];
int pl;
{
    /* Init increm_project_token_to_instance */
    int match_flag = TRUE;
    int i;

    for (i = GET_PLACE_COMPONENTS(pl); i ; i--) {
        /* Per ogni componente del dominio del posto */
        if (mapper[i - 1] >= 0) {
            /* Componente da fissare con il token in esame */
            trial_ptr->npla[mapper[i - 1]] = tok_ptr->id[i - 1];
        }/* Componente da fissare con il token in esame */
        else if (trial_ptr->npla[-(mapper[i - 1] + 1)] != tok_ptr->id[i - 1]) {
            /* Componente non uguale all'istanza gia' costruita --> fail */
            match_flag = FALSE;
        }/* Componente non uguale all'istanza gia' costruita --> fail */
    }/* Per ogni componente del dominio del posto */
    return (match_flag);
}/* End increm_project_token_to_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int incremental_check_for_enabling(instance_ptr, node_ptr, type)
Event_p instance_ptr;
Node_p node_ptr;
int type;
{
    /* Init incremental_check_for_enabling */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    Pred_p pred_ptr = GET_TRANSITION_PREDICATE(tr);

    if (verify_predicate_constraint(pred_ptr, instance_ptr))
        verify_marking_constraint(node_ptr, instance_ptr, tr, INPUT);
    else
        instance_ptr->enabling_degree = 0;
    return (instance_ptr->enabling_degree);
}/* End incremental_check_for_enabling */
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
    int function_type = UNKNOWN, min_value = MAX_INT, pl, compare_value;
    int touched_flag = FALSE;
    Node_p node_ptr = GET_INPUT_LIST(tr);
    Node_p min_ptr = NULL;

    for (; node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
        /* Per ogni posto */
        if (!node_ptr->analyzed) {
            if (!IS_INDEPENDENT(node_ptr))
                function_type = node_ptr->type;
            break;
        }
    }/* Per ogni posto */
    if (function_type != UNKNOWN) {
        /* C'e' almeno un posto */
        node_ptr = GET_INPUT_LIST(tr);
        for (; node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
            /* Per ogni posto */
            if (!node_ptr->analyzed && !IS_INDEPENDENT(node_ptr))
                if (node_ptr->type == function_type) {
                    /* Posto dello stesso tipo di funzione */
                    pl = GET_PLACE_INDEX(node_ptr);
                    if (net_mark[pl].touched_h != NULL) {
                        /* Posti con marcatura variata */
                        compare_value = GET_PLACE_COMPONENTS(pl);
                        touched_flag = TRUE;
                        if (min_value > compare_value) {
                            /* Meno token */
                            min_value = compare_value;
                            min_ptr = node_ptr;
                        }/* Meno token */
                    }/* Posti con marcatura variata */
                }/* Posto dello stesso tipo di funzione */
        }/* Per ogni posto */
        if (!touched_flag) {
            /* Posti con marcatura invariata */
            node_ptr = GET_INPUT_LIST(tr);
            for (; node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
                /* Per ogni posto */
                if (!node_ptr->analyzed && !IS_INDEPENDENT(node_ptr))
                    if (node_ptr->type == function_type) {
                        /* Posto dello stesso tipo di funzione */
                        pl = GET_PLACE_INDEX(node_ptr);
                        compare_value = net_mark[pl].different;
                        if (min_value > compare_value) {
                            /* Meno token */
                            min_value = compare_value;
                            min_ptr = node_ptr;
                        }/* Meno token */
                    }/* Posto dello stesso tipo di funzione */
            }/* Per ogni posto */
        }/* Posti con marcatura invariata */
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

        int pl, generate;

        node_ptr = get_a_node_to_analyze(tr, type, node_ptr);
        for (; node_ptr != NULL && enabled ;) {
            /* Per ogni posto */
            enabled = FALSE;
            pl = GET_PLACE_INDEX(node_ptr);
            if (!IS_NEUTRAL(pl) && !IS_INDEPENDENT(node_ptr)) {
                /* Posto con dominio colorato */
                generate = project_function_on_domain(node_ptr, PROJECTION, tr, mapper);
                if (generate) {
                    /* Il posto fissa alcune componenti del dominio */
                    inst_ptr = old_enabled_list;
                    for (; inst_ptr != NULL;) {
                        /* Per ogni istanza parziale gia' costruita */
                        copy_event(trial_ptr, inst_ptr);
                        if (inst_ptr->enabling_degree) {
                            /* Se l'istanza parziale e' abilitata */
                            if (net_mark[pl].touched_h != NULL)
                                tok_ptr = net_mark[pl].touched_h;
                            else
                                tok_ptr = net_mark[pl].marking;
                            for (; tok_ptr != NULL;) {
                                /* Per ogni token della marcatura */
                                if (increm_project_token_to_instance(tok_ptr, trial_ptr, mapper, pl)) {
                                    /* Token corretto rispetto alle istanze parziali */
                                    if (incremental_check_for_enabling(trial_ptr, node_ptr, INPUT))
                                        enabled = TRUE;
                                }/* Token corretto rispetto alle istanze parziali */
                                else
                                    trial_ptr->enabling_degree = 0;
                                increm_add_instance(trial_ptr);
                                trial_ptr->enabling_degree = inst_ptr->enabling_degree;
                                if (net_mark[pl].touched_h != NULL)
                                    tok_ptr = NEXT_TOUCHED(tok_ptr);
                                else
                                    tok_ptr = NEXT_TOKEN(tok_ptr);
                            }/* Per ogni token della marcatura */
                        }/* Se l'istanza parziale e' abilitata */
                        else
                            increm_add_instance(trial_ptr);
                        inst_ptr = next_instance(inst_ptr);
                    }/* Per ogni istanza parziale gia' costruita */
                }/* Il posto fissa alcune componenti del dominio */
                else {
                    /* Verificare la marcatura di istanze gia' formate */
                    inst_ptr = old_enabled_list;
                    for (; inst_ptr != NULL;) {
                        /* Per ogni istanza */
                        copy_event(trial_ptr, inst_ptr);
                        if (inst_ptr->enabling_degree) {
                            /* Istanza che abilita */
                            if (verify_marking_constraint(node_ptr, trial_ptr, tr, INPUT))
                                enabled = TRUE;
                        }/* Istanza che abilita */
                        increm_add_instance(trial_ptr);
                        inst_ptr = next_instance(inst_ptr);
                    }/* Per ogni istanza */
                }/* Verificare la marcatura di istanze gia' formate */
                old_enabled_list = new_enabled_list;
                new_enabled_list = NULL;
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
    result->hold = TRUE;
    result->next = NULL;
    result->list = old_enabled_list;
}/* End test_enabl */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void compute_instances_incrementally(tr, result_ptr)
int tr;
Result_p result_ptr;
{
    /* Init compute_instances_incrementally */

    result_ptr->list = NULL;
    result_ptr->next = NULL;
    result_ptr-> hold = FALSE;
    if ((initial_enabling_degree = pre_check(tr))) {
        /* Non ci sono condizioni semplici di disabilitazione */
        init_enabling_phase(tr); /* skeleton initializaion */
        old_enabled_list = get_new_event(tr);
        old_enabled_list->enabling_degree = initial_enabling_degree;
        test_enabl(tr, INPUT, result_ptr);
    }/* Non ci sono condizioni semplici di disabilitazione */
}/* End compute_instances_incrementally */
#endif
#endif
