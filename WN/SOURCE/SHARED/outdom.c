# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/service.h"

#ifdef SWN
extern Event_p new_enabled_list;
extern Event_p old_enabled_list;

extern Event_p old_split;

extern int skeleton[];

extern int verify_predicate_constraint();
extern int get_random_object();
extern int new_instances_generated();
extern void setup_cartesian_product_of_tr();
extern void project_element_to_instance();

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int simple_check_for_enabling(instance_ptr)
Event_p instance_ptr;
{
    /* Init simple_check_for_enabling */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int ret_value = FALSE;
    Pred_p pred_ptr = GET_TRANSITION_PREDICATE(tr);

#ifdef SYMBOLIC
#ifdef SIMULATION
    int at_least_one_class_requires_presplit = tabt[tr].pre_split;
#endif
    int enabled;
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
                Event_p inst_ptr = old_split;

                for (; inst_ptr->next != NULL; inst_ptr = inst_ptr->next);
                inst_ptr->next = new_enabled_list;
                new_enabled_list = old_split;
                ret_value = TRUE;
            }/* Se si sono prodotte delle istanze */
        }/* Non si puo' fare solo preselection */
        else {
            /* Solo preselection */
            ret_value = TRUE;
            add_instance(instance_ptr);
        }/* Solo preselection */
#endif
#ifdef REACHABILITY
        enabled = pre_split_instance(instance_ptr, TOTAL);
        if (enabled) {
            /* Se si sono prodotte delle istanze */
            Event_p inst_ptr = old_split;

            for (; inst_ptr->next != NULL; inst_ptr = inst_ptr->next);
            inst_ptr->next = new_enabled_list;
            new_enabled_list = old_split;
            ret_value = TRUE;
        }/* Se si sono prodotte delle istanze */
#endif
#endif
#ifdef COLOURED
        ret_value = TRUE;
        add_instance(instance_ptr);
#endif
    }/* Verifica il predicato */
    return (ret_value);
}/* End simple_check_for_enabling */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : SOLO PER ISTANZE ABILITATE		      */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void get_output_components(tr, mapper)
int tr;
int mapper[MAX_DOMAIN];
{
    /* Init get_output_components */
    int i;

    for (i = GET_TRANSITION_COMPONENTS(tr) ; i ; i--) {
        /* Per ogni componente del dominio della transizione */
        if (tabt[tr].comp_type[i - 1] == OUT) {
            /* Parti del dominio ancora non fissate dopo l'analisi dei posti */
            mapper[i - 1] = TRUE;
            skeleton[i - 1] = OLD;
        }/* Parti del dominio ancora non fissate dopo l'analisi dei posti */
        else
            mapper[i - 1] = FALSE;
    }/* Per ogni componente del dominio della transizione */
}/* End get_output_components */
#ifdef SIMULATION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : SOLO PER ISTANZE ABILITATE		      */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void project_a_random_element(Event_p  instance_ptr,  int  mapper[MAX_DOMAIN]) {
    /* Init project_a_random_element */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int domain_comp = GET_TRANSITION_COMPONENTS(tr);
    int i, cl;

    for (i = 0; i < domain_comp; i++) {
        /* Per ogni componente del dominio della transizione */
        if (mapper[i]) {
            /* Componente non ancora determinata */
            cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef SYMBOLIC
            if (IS_UNORDERED(cl)) {
                /* Classe non ordinata */
                instance_ptr->npla[i] = get_random_dynamic_subclass(cl);
            }/* Classe non ordinata */
            else
#endif
                instance_ptr->npla[i] = get_random_object(cl);
        }/* Componente non ancora determinata */
    }/* Per ogni componente del dominio della transizione */
}/* End project_a_random_element */
#endif
#ifdef REACHABILITY
/**************************************************************/
/* NAME : */
/* DESCRIPTION : SOLO PER ISTANZE ABILITATE		      */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void set_output_components(int  tr,  Result_p  result) {
    /* Init set_output_components */
    int mapper[MAX_DOMAIN];

    Event_p trial_ptr = NULL;
    Event_p inst_ptr = NULL;

    int enabled = TRUE;
    int domain_comp = GET_TRANSITION_COMPONENTS(tr);
    int output_type = UNKNOWN;

    trial_ptr = get_new_event(tr);
#ifdef SIMULATION
    output_type = GET_TRANSITION_OUTPUT_TYPE(tr);
#else
    output_type = COMPLEX_OUTPUT;
#endif
    if (output_type != NO_OUTPUT_COMPONENTS) {
        /* C'e' output */
        get_output_components(tr, mapper);
        inst_ptr = old_enabled_list;
        new_enabled_list = NULL;
        setup_cartesian_product_of_tr(cart_trans_ptr, tr, mapper);
        do {
            /* Per ogni ennupla parziale gia'costruita */
            copy_event(trial_ptr, inst_ptr);
            do {
                /* Per ogni elemento del P.C sul dominio di t */
                project_element_to_instance(cart_trans_ptr, trial_ptr, mapper);
                simple_check_for_enabling(trial_ptr);
            }/* Per ogni elemento del P.C sul dominio di t */
            while (nextvec(cart_trans_ptr, domain_comp));
            inst_ptr = next_instance(inst_ptr);
        }/* Per ogni ennupla parziale gia'costruita */
        while (inst_ptr != NULL);
        enabled = new_instances_generated();
    }/* C'e' output */
    dispose_old_event(trial_ptr);
    result->hold = enabled;
    result->next = NULL;
    result->list = old_enabled_list;
}/* End set_output_components */
#endif
#ifdef SIMULATION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : SOLO PER ISTANZE ABILITATE		      */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void set_output_components(int  tr,  Result_p  result) {
    /* Init set_output_components */
    int mapper[MAX_DOMAIN];

    Event_p trial_ptr = NULL;
    Event_p inst_ptr = NULL;

    int enabled = TRUE, over = FALSE;
    int domain_comp = GET_TRANSITION_COMPONENTS(tr);
    int output_type = UNKNOWN;

    trial_ptr = get_new_event(tr);
    output_type = GET_TRANSITION_OUTPUT_TYPE(tr);
    if (output_type != NO_OUTPUT_COMPONENTS) {
        /* C'e' output */
        get_output_components(tr, mapper);
        inst_ptr = old_enabled_list;
        new_enabled_list = NULL;
        switch (output_type) {
        /* Vari casi */
        case COMPLEX_OUTPUT :
            setup_cartesian_product_of_tr(cart_trans_ptr, tr, mapper);
            do {
                /* Per ogni ennupla parziale gia'costruita */
                copy_event(trial_ptr, inst_ptr);
                do {
                    /* Per ogni elemento del P.C sul dominio di t */
                    project_element_to_instance(cart_trans_ptr, trial_ptr, mapper);
                    simple_check_for_enabling(trial_ptr);
                }/* Per ogni elemento del P.C sul dominio di t */
                while (nextvec(cart_trans_ptr, domain_comp));
                inst_ptr = next_instance(inst_ptr);
            }/* Per ogni ennupla parziale gia'costruita */
            while (inst_ptr != NULL);
            break;
        case SOURCE_TRANSITION :
        case PRESELECTION_TRANSITION :
            do {
                /* Per ogni ennupla parziale gia'costruita */
                copy_event(trial_ptr, inst_ptr);
                do {
                    /* Per ogni ennupla costruibile */
                    project_a_random_element(trial_ptr, mapper);
                    over = simple_check_for_enabling(trial_ptr);
                }/* Per ogni ennupla costruibile */
                while (!over);
                inst_ptr = next_instance(inst_ptr);
            }/* Per ogni ennupla parziale gia'costruita */
            while (inst_ptr != NULL);
            break;
        }/* Vari casi */
        enabled = new_instances_generated();
    }/* C'e' output */
    dispose_old_event(trial_ptr);
    result->hold = enabled;
    result->next = NULL;
    result->list = old_enabled_list;
}/* End set_output_components */
#endif
#endif
