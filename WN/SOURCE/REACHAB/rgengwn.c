#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"

#ifdef REACHABILITY
extern Result_p pop_result();
extern Event_p get_new_event();

extern Result_p enabled_head;

extern int cur_priority;

extern int *code_place;
extern int *min_place;
extern int *max_place;
extern int *init_place;

extern void *ecalloc(size_t  nitm, size_t sz);
void compute_instances_from_scratch(int  tr,  Result_p  result_ptr);

static int en_ptr = 0;
static int *en_list = NULL;
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
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int GET_ENABLING_DEGREE(Event_p  ev_p) {
    /* Init GET_ENABLING_DEGREE */
    int ret_value = 0;

    if (ev_p != NULL) {
#ifdef GSPN
        ret_value = ev_p->enabling_degree;
#endif
#ifdef SWN
#ifdef SYMBOLIC
        ret_value = ev_p->enabling_degree * ev_p->ordinary_instances;
#endif
#ifdef COLOURED
        ret_value = ev_p->enabling_degree;
#endif
#endif
    }
    return (ret_value);
}/* End GET_ENABLING_DEGREE */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_in_SC(tr, check)
int tr;
int check;
{
    /* Init is_in_SC */
    int ii;

    for (ii = 0; tabt[check].test_l[ii] != UNKNOWN ; ii++)
        if (tr == tabt[check].test_l[ii])
            return (TRUE);
    return (FALSE);
}/* End is_in_SC */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_in_CC(tr, check)
int tr;
int check;
{
    /* Init is_in_CC */
    int ii;

    for (ii = 0; tabt[check].add_l[ii] != UNKNOWN ; ii++)
        if (tr == tabt[check].add_l[ii])
            return (TRUE);
    return (FALSE);
}/* End is_in_CC */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void update_priority(tr, res, duplicate)
int tr;
Result_p res;
int duplicate;
{
    /* Init update_priority */
    Result_p enabled = NULL;
    int ii;

    for (ii = 0 ; ii < en_ptr ; ii++)
        if (tr == en_list[ii])
            return;;

    enabled = pop_result();
    enabled->hold = res->hold;
    enabled->list = NULL;
    if (duplicate) {
        /* Elementi da copiare fisicamente */
        Event_p add_ptr;
        Event_p ev_p;

        for (ev_p = res->list; ev_p != NULL; ev_p = NEXT_EVENT(ev_p)) {
            /* per ogni evento da copiare */
            add_ptr = get_new_event(tr);
            copy_event(add_ptr, ev_p);
            add_ptr->next = enabled->list;
            enabled->list = add_ptr;
        }/* per ogni evento da copiare */
    }/* Elementi da copiare fisicamente */
    else
        enabled->list = res->list;
    if (enabled_head == NULL)
        enabled_head = enabled;
    else {
        enabled->next = enabled_head;
        enabled_head = enabled;
    }
    en_list[en_ptr++] = tr;
    if (tabt[tr].pri > cur_priority)
        cur_priority = tabt[tr].pri;
}/* End update_priority */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void manage_enabling(checked_tr)
int checked_tr;
{
    /* Init manage_enabling */
    struct RESULT res;
    Result_p res_ptr = &res;
    int ii;

    for (ii = 0 ; ii < en_ptr ; ii++)
        if (checked_tr == en_list[ii])
            return;;
    compute_instances_from_scratch(checked_tr, res_ptr);
    if (res_ptr->hold) {
        /* Transizione abilitata */
        update_priority(checked_tr, res_ptr, FALSE);
    }/* Transizione abilitata */
}/* End manage_enabling */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void update_en_list(Event_p  fired_ptr,  Result_p  previously_enabled_list) {
    /* Init update_en_list */
    Result_p previously_enabled_ptr = previously_enabled_list;
    Result_p next_previously_enabled_ptr;
    int CC_transition, enabled_before_firing;
    int fired_tr = GET_TRANSITION_INDEX(fired_ptr);
    int *CC_list = NULL, *SC_list = NULL;
    int tr;


    cur_priority = 0; en_ptr = 0;
#ifdef GSPN
    for (tr = ntr; tr ; tr--)
        manage_enabling(tr - 1);
#endif
#ifdef SWN
    CC_list = tabt[fired_tr].add_l;
    SC_list = tabt[fired_tr].test_l;
    for (; previously_enabled_ptr != NULL ; previously_enabled_ptr = next_previously_enabled_ptr) {
        /* Per ogni transizione abilitata nella marcatura precedente */
        enabled_before_firing = previously_enabled_ptr->list->trans;
        next_previously_enabled_ptr = previously_enabled_ptr->next;
        if (is_in_CC(enabled_before_firing, fired_tr)) {
            /* Transizione abilitata e in CC */
            manage_enabling(enabled_before_firing);
        }/* Transizione abilitata e in CC */
        else if (is_in_SC(enabled_before_firing, fired_tr)) {
            /* Transizione abilitata e in SC */
            manage_enabling(enabled_before_firing);
        }/* Transizione abilitata e in SC */
        else if (enabled_before_firing == fired_tr) {
            /* Transizione che ha sparato */
            manage_enabling(enabled_before_firing);
        }/* Transizione che ha sparato */
        else {
            /* transizione non condizionata dallo sparo */
#ifdef GSPN_DA_RIMETTERE
            update_priority(enabled_before_firing, previously_enabled_ptr, TRUE);
#endif
#ifdef COLOURED
            update_priority(enabled_before_firing, previously_enabled_ptr, TRUE);
#endif
#ifdef SYMBOLIC
            manage_enabling(enabled_before_firing);
#endif
        }/* transizione non condizionata dallo sparo */
    }/* Per ogni transizione abilitata nella marcatura precedente */
    while ((CC_transition = *(CC_list++)) != UNKNOWN) {
        /* Potenzialmente abilitata */
        if (!mutual_exclusion(CC_transition)) {
            /* Se non in mutua esclusione con una tr. gia' abilitata */
            manage_enabling(CC_transition);
        }/* Se non in mutua esclusione con una tr. gia' abilitata */
    }/* Potenzialmente abilitata */
#endif
}/* End update_en_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_en_list() {
    /* Init initialize_en_list */
    /* Khalil: computation of the enabled trans of the initial marking */
    int tr;
    struct RESULT res;
    struct RESULT *result_ptr = &res;

    cur_priority = 0;
    en_list = (int *)ecalloc(ntr, sizeof(int));
    code_place = (int *)ecalloc(npl, sizeof(int));
    min_place = (int *)ecalloc(npl, sizeof(int));
    max_place = (int *)ecalloc(npl, sizeof(int));
    init_place = (int *)ecalloc(npl, sizeof(int));
    en_ptr = 0;
    for (tr = ntr ; tr ; tr--) {
        /* Per ogni transizione */
        compute_instances_from_scratch(tr - 1, result_ptr);
        if (result_ptr->hold) {
            /* Se e' abilitata */
            update_priority(tr - 1, result_ptr, FALSE);
        }/* Se e' abilitata */
    }/* Per ogni transizione */
}/* End initialize_en_list */

/********************* For ESRG ****************************************************************/
void MY_INIT_ALLOC() {
    en_list = (int *)ecalloc(ntr, sizeof(int));
    code_place = (int *)ecalloc(npl, sizeof(int));
    min_place = (int *)ecalloc(npl, sizeof(int));
    max_place = (int *)ecalloc(npl, sizeof(int));
    init_place = (int *)ecalloc(npl, sizeof(int));
}

void my_initialize_en_list() {
    /* Init initialize_en_list */
    /* Khalil: computation of the enabled trans of the initial marking */
    int tr;
    struct RESULT res;
    struct RESULT *result_ptr = &res;

    cur_priority = 0;
    en_ptr = 0;
    for (tr = ntr ; tr ; tr--) {
        /* Per ogni transizione */
        compute_instances_from_scratch(tr - 1, result_ptr);
        if (result_ptr->hold) {
            /* Se e' abilitata */
            update_priority(tr - 1, result_ptr, FALSE);
        }/* Se e' abilitata */
    }/* Per ogni transizione */
}/* End initialize_en_list */
/**************************** for ESRG *********************************************************/

void my_en_list(int tr) {
    struct RESULT res;
    struct RESULT *result_ptr = &res;

    cur_priority = 0;
    en_ptr = 0;
    compute_instances_from_scratch(tr - 1, result_ptr);
    if (result_ptr->hold) {
        update_priority(tr - 1, result_ptr, FALSE);
    }
}
/**************************** for ESRG *********************************************************/

#ifdef LIBSPOT

static void dispose_all_instances(Result_p res_ptr) {
    /* Init dispose_all_instances */
    Event_p ev_p, nev_p;

    if (res_ptr != NULL) {
        /* Puntatore consistente */
        ev_p = res_ptr->list;
        while (ev_p != NULL) {
            nev_p = ev_p->next;
            dispose_old_event(ev_p);
            ev_p = nev_p;
        }
    }/* Puntatore consistente */
}/* End dispose_all_instances */

extern int is_enabled(int tr) {
    struct RESULT res;
    struct RESULT *result_ptr = &res;

    cur_priority = 0;
    en_ptr = 0;
    compute_instances_from_scratch(tr , result_ptr);
    if (result_ptr->hold) {
        /* Se e' abilitata */
        dispose_all_instances(result_ptr);
        return 1;
    }/* Se e' abilitata */
    else {
        /* non e' abilitata !! */
        return 0;
    }
}

#endif





#endif
