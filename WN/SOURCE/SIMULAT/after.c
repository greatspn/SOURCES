# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/service.h"

#ifdef SWN
#ifdef SIMULATION
#ifdef SYMBOLIC
extern void deschedule_instance();
extern void reschedule_instance();
extern void set_output_components();
extern void adjust_enable_degree();
extern int verify_marking_constraint_of_all_places();
extern int GET_ENABLING_DEGREE();

extern int **nos;
extern int **nofs;

extern int **nom;
extern int **lost;
extern int **original;

extern int ** *bung;

extern int *iag_split;

extern Event_p new_enabled_list;
extern Event_p old_enabled_list;
extern Event_p old_split;

#define THERE_WAS_BUNGING(cl,ss) (nom[cl][ss] - nofs[cl][ss] > 0)
#define USED_TO_BUNG(cl,ss,ds) ds >= GET_NUM_SS(cl,ss) + GET_STATIC_OFFSET(cl,ss)
#define GET_ID_AFTER_BUNGING(cl,ss,ds) bung[cl][ss][original[cl][ss] - ds]

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void total_check_for_enabling(instance_ptr)
Event_p instance_ptr;
{
    /* Init total_check_for_enabling */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    Pred_p pred_ptr = GET_TRANSITION_PREDICATE(tr);
    Event_p next_ptr;
    int at_least_one_class_requires_presplit = tabt[tr].pre_split;
    int enabled;

    if (verify_predicate_constraint(pred_ptr, instance_ptr)) {
        /* Verifica il predicato */
        if (at_least_one_class_requires_presplit) {
            /* Non si puo' fare solo preselection */
            enabled = pre_split_instance(instance_ptr, TOTAL);
            if (enabled) {
                /* Se si sono prodotte delle istanze */
                for (; old_split != NULL ;) {
                    /* Per ogni istanza costruita */
                    next_ptr = old_split->next;
                    if (verify_marking_constraint_of_all_places(GET_INPUT_LIST(tr), old_split, tr, INPUT)) {
                        if (verify_marking_constraint_of_all_places(GET_INHIBITOR_LIST(tr), old_split, tr, INHIBITOR)) {
                            /* Verifica la marcatura */
                            old_split->next = new_enabled_list;
                            new_enabled_list = old_split;
                        }/* Verifica la marcatura */
                        else
                            dispose_old_event(old_split);
                    }
                    else
                        dispose_old_event(old_split);
                    old_split = next_ptr;
                }/* Per ogni istanza costruita */
            }/* Se si sono prodotte delle istanze */
        }/* Non si puo' fare solo preselection */
        else {
            /* Solo preselection */
            if (verify_marking_constraint_of_all_places(GET_INPUT_LIST(tr), instance_ptr, tr, INPUT))
                if (verify_marking_constraint_of_all_places(GET_INHIBITOR_LIST(tr), instance_ptr, tr, INHIBITOR))
                    add_instance(instance_ptr);
        }/* Solo preselection */
    }/* Verifica il predicato */
}/* End total_check_for_enabling */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_position_in_split(fired_instance, start, cl, ds)
Event_p fired_instance;
int start;
int cl;
int ds;
{
    /* Init get_position_in_split */
    int tr = GET_TRANSITION_INDEX(fired_instance);
    int ret_value = UNKNOWN;
    int ii = start;

    for (; ii < OFF(tr, cl) + GET_COLOR_REPETITIONS(cl, tr) ; ii++)
        if (fired_instance->split[ii] == ds) {
            ret_value = ii;
            goto ret;
        }
ret: return (ret_value);
}/* End get_position_in_split */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_position_in_fired_instance(fired_instance, start, cl, ds)
Event_p fired_instance;
int start;
int cl;
int ds;
{
    /* Init get_position_in_fired_instance */
    int tr = GET_TRANSITION_INDEX(fired_instance);
    int ret_value = UNKNOWN;
    int ii = start;

    for (; ii < OFF(tr, cl) + GET_COLOR_REPETITIONS(cl, tr) ; ii++)
        if (fired_instance->npla[ii] == ds) {
            ret_value = ii;
            goto ret;
        }
ret: return (ret_value);
}/* End get_position_in_fired_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_lost(cl, ds, fired_instance)
int cl;
int ds;
Event_p fired_instance;
{
    /* Init is_lost */
    int tr = GET_TRANSITION_INDEX(fired_instance);
    int comp = GET_TRANSITION_COMPONENTS(tr);
    int start = OFF(tr, cl);
    int ret_value = FALSE;
    int pos;

    do {
        if ((pos = get_position_in_fired_instance(fired_instance, start, cl, ds)) != UNKNOWN)
            ret_value = (iag_split[pos] != UNKNOWN && GET_SPLIT_ID(fired_instance, pos) == GET_DYN_ID(fired_instance, pos));
        start = pos + 1;
    }
    while (pos != UNKNOWN && !ret_value && start < comp);
    return (ret_value);
}/* End is_lost */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int has_been_augmented(cl, ds, tr)
int cl;
int ds;
int tr;
{
    /* Init has_been_augmented */
    int ret_value = FALSE;
    int ii = OFF(tr, cl);

    for (; ii < OFF(tr, cl) + GET_COLOR_REPETITIONS(cl, tr) ; ii++)
        if (iag_split[ii] == ds) {
            ret_value = TRUE;
            goto ret;
        }
ret: return (ret_value);
}/* End has_been_augmented */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_in_partial_list(trial_ptr)
Event_p trial_ptr;
{
    /* Init is_in_partial_list */
    int tr = GET_TRANSITION_INDEX(trial_ptr);
    int ret_value = FALSE;
    int equal, ii;
    Event_p comp_ptr = NULL;

    for (comp_ptr = new_enabled_list ; comp_ptr != NULL ; comp_ptr = comp_ptr->next) {
        equal = TRUE;
        for (ii = tabt[tr].comp_num ; ii && equal ; ii--)
            if (tabt[tr].comp_type[ii - 1] == IN)
                equal = (trial_ptr->npla[ii - 1] == comp_ptr->npla[ii - 1]);
        if (equal) {
            ret_value = TRUE;
            goto ret;
        }
    }
ret: return (ret_value);
}/* End is_in_partial_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void rearrange_symbolic_instances(Event_p list, int tr,  Event_p fired_ptr) {
    /* Init rearrange_symbolic_instances */
    int fire_tr = GET_TRANSITION_INDEX(fired_ptr);
    int cl, ss, ds, pos, i, new_id;
    int comp = GET_TRANSITION_COMPONENTS(tr);

    Event_p inst_ptr = NULL;
    Event_p next_ptr = NULL;

    for (inst_ptr = list ; inst_ptr != NULL; inst_ptr = next_ptr) {
        /* Per ogni istanza della transizione */
        next_ptr = inst_ptr->next;
        if (inst_ptr->enabling_degree > 0) {
            /* L'istanza abilita */
            for (i = 0 ; i < comp; i++) {
                /* Per ogni componente del dominio della transizione */
                if (tabt[tr].comp_type[i] == IN) {
                    /* Componente in ingresso */
                    cl  = GET_COLOR_IN_POSITION(i, tr);
                    if (IS_UNORDERED(cl)) {
                        /* Classe non ordinata */
                        ds = new_id = GET_DYN_ID(inst_ptr, i);
                        ss = get_static_subclass(cl, ds);
                        if ((pos = get_position_in_split(fired_ptr, OFF(fire_tr, cl), cl, ds)) != UNKNOWN) {
                            if (iag_split[pos] != UNKNOWN)
                                new_id = iag_split[pos];
                        }
                        else if (USED_TO_BUNG(cl, ss, ds))
                            new_id = GET_ID_AFTER_BUNGING(cl, ss, ds);
                        inst_ptr->npla[i] = new_id;
                        if (GET_SPLIT_ID(inst_ptr, i) != UNKNOWN)
                            inst_ptr->split[i] = ENCODE_ID(new_id, DECODE_OBJECT(inst_ptr->split[i]));
                    }/* Classe non ordinata */
                    else
                        break;
                }/* Componente in ingresso */
            }/* Per ogni componente del dominio della transizione */
        }/* L'istanza abilita */
    }/* Per ogni istanza della transizione */
}/* End rearrange_symbolic_instances */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void rearrange_and_schedule_symbolic_instances(int tr, Event_p fired_ptr) {
    /* Init rearrange_and_schedule_symbolic_instances */
    int fire_tr = GET_TRANSITION_INDEX(fired_ptr);
    int at_least_one_subclass, bung_flag, cl, ss, ds, new_id, pos, i, new_deg;
    int comp = GET_TRANSITION_COMPONENTS(tr);

    Event_p inst_ptr = NULL;
    Event_p next_ptr = NULL;
    Event_p trial_ptr = NULL;

    trial_ptr = get_new_event(tr);
    new_enabled_list = NULL;
    for (inst_ptr = tabt[tr].instances_h ; inst_ptr != NULL; inst_ptr = next_ptr) {
        /* Per ogni istanza della transizione */
        next_ptr = inst_ptr->t_next;
        at_least_one_subclass = FALSE;
        for (i = 0 ; i < comp; i++) {
            /* Per ogni componente del dominio della transizione */
            cl = GET_COLOR_IN_POSITION(i, tr);
            if (IS_UNORDERED(cl)) {
                /* Classe non ordinata */
                cart_trans_ptr->low[i] = 0;
                if (tabt[tr].comp_type[i] == IN) {
                    /* Dominio in ingresso */
                    bung_flag = FALSE;
                    ds = GET_DYN_ID(inst_ptr, i);
                    ss = get_static_subclass(cl, ds);
                    cart_trans_ptr->up[i] = -1;
                    if (get_position_in_fired_instance(fired_ptr, OFF(tr, cl), cl, ds) != UNKNOWN) {
                        /* E' stata istanziata */
                        at_least_one_subclass = TRUE;
                        for (pos = OFF(fire_tr, cl); pos < OFF(fire_tr, cl) + GET_COLOR_REPETITIONS(cl, fire_tr); pos++)
                            if (fired_ptr->npla[pos] == ds) {
                                if (iag_split[pos] != UNKNOWN)
                                    new_id = iag_split[pos];
                                else
                                    new_id = fired_ptr->split[pos];
                                if (!present(new_id, cart_trans_ptr->mark[i], cart_trans_ptr->up[i]))
                                    cart_trans_ptr->mark[i][++cart_trans_ptr->up[i]] = new_id;
                            }
                    }/* E' stata istanziata */
                    else {
                        /* Non istanziata */
                        if (USED_TO_BUNG(cl, ss, ds)) {
                            ds = GET_ID_AFTER_BUNGING(cl, ss, ds);
                            bung_flag = TRUE;
                        }
                        if (has_been_augmented(cl, ds, fire_tr) || bung_flag) {
                            /* Setup del P.C. */
                            at_least_one_subclass = TRUE;
                        }/* Setup del P.C. */
                    }/* Non istanziata */
                    if (bung_flag)
                        ds = GET_ID_AFTER_BUNGING(cl, ss, GET_DYN_ID(inst_ptr, i));
                    else
                        ds = GET_DYN_ID(inst_ptr, i);
                    if (bung_flag) {
                        if (!present(ds, cart_trans_ptr->mark[i], cart_trans_ptr->up[i]))
                            cart_trans_ptr->mark[i][++cart_trans_ptr->up[i]] = ds;
                    }
                    else {
                        if (!is_lost(cl, ds, fired_ptr))
                            if (!present(ds, cart_trans_ptr->mark[i], cart_trans_ptr->up[i]))
                                cart_trans_ptr->mark[i][++cart_trans_ptr->up[i]] = ds;
                    }
                }/* Dominio in ingresso */
                else {
                    /* Dominio in uscita */
                    cart_trans_ptr->up[i] = 0;
                    cart_trans_ptr->mark[i][0] = UNKNOWN;
                }/* Dominio in uscita */
            }/* Classe non ordinata */
            else
                break;
        }/* Per ogni componente del dominio della transizione */
        if (at_least_one_subclass) {
            /* Bisogna generare e analizzare */
            deschedule_instance(inst_ptr);
            do {
                /* Per ogni elemento del P.C. */
                for (i = 0 ; i < comp; i++) {
                    cl = GET_COLOR_IN_POSITION(i, tr);
                    if (IS_UNORDERED(cl)) {
                        /* Classe non ordinata */
                        trial_ptr->npla[i] = cart_trans_ptr->mark[i][cart_trans_ptr->low[i]];
                        trial_ptr->split[i] = UNKNOWN;
                    }/* Classe non ordinata */
                }
                for (i = 0 ; i < comp; i++) {
                    cl = GET_COLOR_IN_POSITION(i, tr);
                    if (IS_ORDERED(cl)) {
                        /* Classe ordinata */
                        trial_ptr->npla[i] = inst_ptr->npla[i];
                        trial_ptr->split[i] = UNKNOWN;
                    }/* Classe ordinata */
                }
                if (!is_in_partial_list(trial_ptr)) {
                    /* Vale la pena di testare */
                    trial_ptr->enabling_degree = GET_TRANSITION_SERVER(tr);
                    total_check_for_enabling(trial_ptr);
                }/* Vale la pena di testare */
            }/* Per ogni elemento del P.C. */
            while (nextvec(cart_trans_ptr, comp));
        }/* Bisogna generare e analizzare */
        else {
            /* Istanza che non e' stata influenzata */
            if (!is_in_partial_list(inst_ptr)) {
                if (tabt[tr].en_deg)
                    inst_ptr->enabling_degree /= tabt[tr].en_deg;
                for (i = 0 ;  i < comp; i++) {
                    inst_ptr->split[i] = UNKNOWN;
                    if (tabt[tr].comp_type[i] == OUT)
                        inst_ptr->npla[i] = UNKNOWN;
                }
                add_instance(inst_ptr);
            }
            deschedule_instance(inst_ptr);
        }/* Istanza che non e' stata influenzata */
    }/* Per ogni istanza della transizione */
    if (new_enabled_list != NULL) {
        struct RESULT res;
        Result_p result_ptr = & res;

        result_ptr->hold = TRUE;
        result_ptr->next = NULL;
        result_ptr->list = NULL;
        old_enabled_list = new_enabled_list;
        set_output_components(tr, result_ptr);
        adjust_enable_degree(result_ptr->list);
        new_enabled_list = result_ptr->list;
    }
    for (; new_enabled_list != NULL; new_enabled_list = next_ptr) {
        next_ptr = new_enabled_list->next;
        new_deg = GET_ENABLING_DEGREE(new_enabled_list);
        reschedule_instance(new_enabled_list, 0, new_deg, NULL);
    }
    dispose_old_event(trial_ptr);
}/* End rearrange_and_schedule_symbolic_instances */
#endif
#endif
#endif
