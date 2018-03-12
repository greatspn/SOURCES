#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"

#ifdef SIMULATION
#define DETERMINISTIC_TRANS 127

#define IS_IN_CC  (CC_transition == enabled_before_firing)
#define IS_IN_SC  (*SC_list == enabled_before_firing)
#define HAS_FIRED (fired_tr == enabled_before_firing)

Trans_p enab_list;

/********* Variabili static *********/
static Trans_p new_list = NULL;
static Trans_p new_e = NULL;

static int en_ptr = 0;
static int *en_list = NULL;
/********* Variabili static *********/

/********* Variabili extern *********/
extern int cur_priority;
extern double cur_time;
/********* Variabili extern *********/

/********* Funzioni extern *********/
extern void *ecalloc();

extern void push_reset_token_list();
extern void compute_instances_by_preselection();
extern void compute_instances_incrementally();
extern void compute_instances_from_scratch();
extern void dispose_old_event();
extern void rearrange_and_schedule_symbolic_instances();
extern void total_update_enabling();
extern void incremental_update_enabling();
extern void schedule_in_transition_list();
extern void deschedule_from_transition_list();
extern void schedule_in_d_transition_list();
extern void deschedule_from_d_transition_list();
extern void schedule_in_EL();
extern void deschedule_from_EL();
extern void deschedule_all_instances();
extern void deschedule_fired_instance();

extern void total_scheduling();
extern void copy_event();
extern void initial_scheduling();
extern void out_error();

extern Event_p get_new_event();

/********* Funzioni extern *********/
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int mutual_exclusion(tr)
int tr;
{
    /* Init mutual_exclusion */
    int i_index, j_index;

    if (tabt[tr].me_l != NULL) {
        /* Le relazioni di ME sono state specificate */
        for (i_index = 0; i_index < en_ptr; i_index++) {
            /* Controllo delle transizioni gia' abilitate */
            for (j_index = 0; tabt[tr].me_l[j_index] != UNKNOWN; j_index++)
                if (en_list[i_index] == tabt[tr].me_l[j_index])
                    return (TRUE);
        }/* Controllo delle transizioni gia' abilitate */
    }/* Le relazioni di ME sono state specificate */
    return (FALSE);
}/* End mutual_exclusion */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int token_varied(checked_tr)
int checked_tr;
{
    /* Init token_varied */
    int ret_value = FALSE;
    int pl;
    int c = 0;
    Node_p node_ptr = GET_INPUT_LIST(checked_tr);

    for (; node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
        /* Per ogni posto in ingresso */
        pl = GET_PLACE_INDEX(node_ptr);
        if (!IS_INDEPENDENT(node_ptr))
            if (net_mark[pl].touched_h != NULL) {
                /* Posto con proiezione e che ha la marcatura variata */
                c++;
                if (c > 1) {
                    ret_value = FALSE;
                    goto ret;
                }
            }/* Posto con proiezione e che ha la marcatura variata */
    }/* Per ogni posto in ingresso */
ret: if (c == 1)
        ret_value = TRUE;
    return (ret_value);
}/* End token_varied */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void add_to_list_of_enabled(tr)
int tr;
{
    /* Init add_to_list_of_enabled */
    Trans_p t_p = tabt + tr;
    int pri = GET_TRANSITION_PRIORITY(tr);

    if (new_e == NULL)
        new_list = t_p;
    else
        new_e->enabl_next = t_p;
    new_e = t_p;
    en_list[en_ptr++] = tr;
    if (pri > cur_priority)
        cur_priority = pri;
}/* End add_to_list_of_enabled */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void manage_enabling(checked_tr, fired_ptr, type, enabled_before)
int checked_tr;
Event_p fired_ptr;
int type;
int enabled_before;
{
    /* Init manage_enabling */
    struct RESULT res;
    Result_p res_ptr = &res;

    switch (type) {
    /* Tipo di relazione */
    case NONE :
#ifdef SWN
#ifdef SYMBOLIC
        if (IS_INPUT_PRESELECTABLE(checked_tr)) {
            /* Input preselezionabile */
            deschedule_all_instances(checked_tr);
            compute_instances_by_preselection(checked_tr, res_ptr);
            total_scheduling(checked_tr, res_ptr->list, type);
        }/* Input preselezionabile */
        else
            rearrange_and_schedule_symbolic_instances(checked_tr, fired_ptr);
#endif
#endif
        break;
    default   :
#ifdef SWN
        if (IS_INPUT_PRESELECTABLE(checked_tr)) {
            /* Preselection per l'input */
            deschedule_all_instances(checked_tr);
            compute_instances_by_preselection(checked_tr, res_ptr);
            if (res_ptr->hold) {
                /* Transizione abilitata */
                add_to_list_of_enabled(checked_tr);
                total_scheduling(checked_tr, res_ptr->list, type);
            }/* Transizione abilitata */
        }/* Preselection per l'input */
        else if (IS_SKIPPABLE(checked_tr) && token_varied(checked_tr) && enabled_before) {
            /* Transizione ottimizzabile */
            compute_instances_incrementally(checked_tr, res_ptr);
            if (res_ptr->hold)
                incremental_update_enabling(checked_tr, fired_ptr, res_ptr->list, type);
            else {
                /* Transizione disabilitata */
                deschedule_all_instances(checked_tr);
            }/* Transizione disabilitata */
        }/* Transizione ottimizzabile */
        else
#endif
        {
            /* Transizione generale */
            compute_instances_from_scratch(checked_tr, res_ptr);
            if (res_ptr->hold)
                total_update_enabling(checked_tr, fired_ptr, res_ptr->list, type);
            else {
                /* Transizione disabilitata */
                deschedule_all_instances(checked_tr);
            }/* Transizione disabilitata */
        }/* Transizione generale */
        break;
    }/* Tipo di relazione */
}/* End manage_enabling */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void update_list_of_enabled_transition(fired_ptr)
Event_p fired_ptr;
{
    /* Init update_list_of_enabled_transition */
    int CC_transition, enabled_before_firing, ii;
    int fired_tr = GET_TRANSITION_INDEX(fired_ptr);
    int *CC_list = NULL, *SC_list = NULL;
    int type;

    Event_p copy_of_fired_ptr = NULL;

    Trans_p previously_enabled_ptr = enab_list;
    Trans_p next_previously_enabled_ptr;

    Node_p p = NULL;

#ifdef SWN
    copy_of_fired_ptr = get_new_event(fired_tr);
    copy_event(copy_of_fired_ptr, fired_ptr);
    p = GET_INPUT_LIST(fired_tr);
    for (; p != NULL ; p = NEXT_NODE(p))
        if (IS_INVOLVED_IN_CC_SC(p)) {
            ii = GET_PLACE_INDEX(p);
            if (net_mark[ii].touched_t != NULL)
                (net_mark[ii].touched_t)->next_touched = net_mark[ii].reset_h;
            else if (net_mark[ii].reset_h != NULL) {
                net_mark[ii].touched_h = net_mark[ii].reset_h;
                net_mark[ii].touched_t = net_mark[ii].reset_t;
            }
        }
    p = GET_OUTPUT_LIST(fired_tr);
    for (; p != NULL ; p = NEXT_NODE(p))
        if (IS_INVOLVED_IN_CC_SC(p)) {
            ii = GET_PLACE_INDEX(p);
            if (net_mark[ii].touched_t != NULL)
                (net_mark[ii].touched_t)->next_touched = net_mark[ii].reset_h;
            else if (net_mark[ii].reset_h != NULL) {
                net_mark[ii].touched_h = net_mark[ii].reset_h;
                net_mark[ii].touched_t = net_mark[ii].reset_t;
            }
        }
#endif
    cur_priority = 0;
    CC_list = tabt[fired_tr].add_l;
    SC_list = tabt[fired_tr].test_l;
    en_ptr = 0;
    for (; previously_enabled_ptr != NULL ; previously_enabled_ptr = next_previously_enabled_ptr) {
        /* Per ogni transizione abilitata nella marcatura precedente */
        enabled_before_firing = (int)(previously_enabled_ptr - tabt);
        next_previously_enabled_ptr = previously_enabled_ptr->enabl_next;
        while ((CC_transition = *CC_list) > enabled_before_firing) {
            /* Controllo delle transizione in CC */
            CC_list++;
            while (*SC_list >= CC_transition)
                SC_list++;
            manage_enabling(CC_transition, copy_of_fired_ptr, CC, FALSE);
        }/* Controllo delle transizione in CC */
        while (*SC_list > enabled_before_firing)
            SC_list++;
        while (*CC_list >= enabled_before_firing)
            CC_list++;
        if (!IS_IN_CC && !IS_IN_SC && !HAS_FIRED) {
            /* Transizione non in relazione */
            int pri;

            if (new_e == NULL)
                new_list = previously_enabled_ptr;
            else
                new_e->enabl_next = previously_enabled_ptr;
            new_e = previously_enabled_ptr;
            if ((pri = GET_TRANSITION_PRIORITY(enabled_before_firing)) > cur_priority)
                cur_priority = pri;
#ifdef SWN
#ifdef SYMBOLIC
            manage_enabling(enabled_before_firing, copy_of_fired_ptr, NONE, TRUE);
#endif
#endif
        }/* Transizione non in relazione */
        else {
            /* Decidere il tipo di controllo */
            if (IS_IN_CC)
                type = CC;
            else if (IS_IN_SC)
                type = SC;
            else if (HAS_FIRED)
                type = SELF;
#ifdef SWN
            if (type != SELF)
                manage_enabling(enabled_before_firing, copy_of_fired_ptr, type, TRUE);
            else
#endif
                manage_enabling(enabled_before_firing, fired_ptr, type, TRUE);
        }/* Decidere il tipo di controllo */
    }/* Per ogni transizione abilitata nella marcatura precedente */
    while ((CC_transition = *(CC_list++)) != UNKNOWN) {
        /* Potenzialmente abilitata */
        if (!mutual_exclusion(CC_transition)) {
            /* Se non in mutua esclusione con una tr. gia' abilitata */
            manage_enabling(CC_transition, copy_of_fired_ptr, CC, FALSE);
        }/* Se non in mutua esclusione con una tr. gia' abilitata */
    }/* Potenzialmente abilitata */
    if ((enab_list = new_list) != NULL)
        new_e->enabl_next = NULL;
    new_list = new_e = NULL;
#ifdef SWN
    dispose_old_event(copy_of_fired_ptr);
    push_reset_token_list(fired_tr);
#endif
#ifdef DEBUG_malloc
    tot_event += num_event;
    if (num_event > max_ev)
        max_ev = num_event;
    if (num_event < min_ev)
        min_ev = num_event;
#endif
}/* End update_list_of_enabled_transition */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_list_of_enabled_transition() {
    /* Init initialize_list_of_enabled_transition */

    int tr = ntr;

    Trans_p t_p = tabt + ntr - 1;

    struct RESULT res;
    Result_p res_ptr = &res;

    en_list = (int *)ecalloc(ntr, sizeof(int));
    cur_priority = 0;
    en_ptr = 0;
    for (new_e = new_list = NULL ; tr ; --tr, --t_p) {
        /* Per ogni transizione */
        if (!mutual_exclusion(tr - 1)) {
            /* Controllo su mutua esclusione */
#ifdef SWN
            if (IS_INPUT_PRESELECTABLE(tr - 1))
                compute_instances_by_preselection(tr - 1, res_ptr);
            else
#endif
                compute_instances_from_scratch(tr - 1, res_ptr);
            if (res_ptr->hold) {
                /* Transizione abilitata */
                add_to_list_of_enabled(tr - 1);
                initial_scheduling(tr - 1, res_ptr->list, UNKNOWN);
            }/* Transizione abilitata */
        }/* Controllo su mutua esclusione */
    }/* Per ogni transizione */
    if ((enab_list = new_list) != NULL)
        new_e->enabl_next = NULL;
    else {
        /* Dead marking */
        out_error(ERROR_INITIAL_DEAD_MARKING, 0, 0, 0, 0, NULL, NULL);
    }/* Dead marking */
    new_list = new_e = NULL;
}/* End initialize_list_of_enabled_transition */
#endif
