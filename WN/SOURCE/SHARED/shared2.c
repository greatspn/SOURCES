/* Modulo per la simulazione simbolica condiviso da after.c, increm.c  */
/* presel.c ed enabling.c					       */
# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/service.h"

#ifdef SWN
#ifdef SYMBOLIC
Event_p new_split = NULL;
Event_p old_split = NULL;

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void add_partial_split(Event_p  instance_ptr) {
    /* Init add_partial_split */
    Event_p new_ptr = NULL;
    int i;

    new_ptr = get_new_event(instance_ptr->trans);
    new_ptr->enabling_degree = instance_ptr->enabling_degree;
    new_ptr->next = new_split;
    new_split = new_ptr;
    for (i = tabt[instance_ptr->trans].comp_num; i ; i--) {
        new_split->npla[i - 1] = instance_ptr->npla[i - 1];
        new_split->split[i - 1] = instance_ptr->split[i - 1];
    }
}/* End add_partial_split */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int new_partial_split_instances_generated() {
    /* Init new_partial_split_instances_generated */
    int ret_value = TRUE;

    if (new_split == NULL)
        ret_value = FALSE;
    else {
        /* Nuove ennuple generate */
        old_split = new_split;
        new_split = NULL;
    }/* Nuove ennuple generate */
    return (ret_value);
}/* End new_partial_split_instances_generated */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int count_number_of_different_objects(Event_p  instance_ptr,  int  ds,  int  cl) {
    /* Init count_number_of_different_objects */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int base = tabt[tr].off[cl];
    int ret_value = 0, off, dyn;
    int member[MAX_COL_RIP];

    for (dyn = 0 ; dyn < tabt[tr].rip[cl] ; member[dyn++] = FALSE);
    for (dyn = 0 ; dyn < tabt[tr].rip[cl] ; dyn++)
        if (ds != dyn)
            if (instance_ptr->npla[base + dyn] == instance_ptr->npla[base + ds]) {
                if (instance_ptr->split[base + dyn] != UNKNOWN) {
                    off = DECODE_OBJECT(instance_ptr->split[base + dyn]);
                    if (!member[off]) {
                        /* Ripetizione non contata prima */
                        member[off] = TRUE;
                        ret_value++;
                    }/* Ripetizione non contata prima */
                }
            }
    return (ret_value);
}/* End count_number_of_different_objects */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int pre_split_instance(Event_p  instance_ptr,  int  type) {
    /* Init pre_split_instance */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    Pred_p pred_ptr = GET_TRANSITION_PREDICATE(tr);

    int cl, ss, ds, dyn_id, base, max_rep, obj;
    int enabled = TRUE;

    Event_p trial_ptr = NULL;
    Event_p first_ptr = NULL;
    Event_p inst_ptr = NULL;

    trial_ptr = get_new_event(tr);
    first_ptr = get_new_event(tr);
    copy_event(first_ptr, instance_ptr);
    old_split = first_ptr;
    for (cl = 0; cl < ncl ; cl++) {
        /* Per ogni classe di colore */
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
#endif
#ifdef REACHABILITY
            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
            {
                /* Classe non ordinata */
                if (tabt[tr].rip[cl] > 0) {
                    /* La classe e' presente nel dominio della transizione */
#ifdef SIMULATION
                    if (tabt[tr].split_type[cl] == AT_ENABLING_TIME) {
                        /* Bisogna fare lo splitting prima */
#endif
                        base = tabt[tr].off[cl];
                        for (ds = 0; ds < tabt[tr].rip[cl] ; ds++) {
                            /* Per ogni ripetizione nel dominio della transizione */
                            if (instance_ptr->npla[base + ds] != UNKNOWN)
                                if (instance_ptr->split[base + ds] == UNKNOWN) {
                                    /* Nuova dsc istanziata */
                                    inst_ptr = old_split;
                                    new_split = NULL;
                                    for (; inst_ptr != NULL ;) {
                                        /* Per ogni parziale */
                                        copy_event(trial_ptr, inst_ptr);
                                        if (IS_UNORDERED(cl)) {
                                            /* Classe non ordinata */
                                            max_rep = count_number_of_different_objects(inst_ptr, ds, cl);
                                            max_rep++;
                                            dyn_id = instance_ptr->npla[base + ds];
                                            ss = get_static_subclass(cl, dyn_id);
                                            max_rep = MIN(max_rep, GET_CARD(cl, ss, dyn_id));
                                            for (obj = 0 ; obj < max_rep ; obj++) {
                                                /* Per ogni possibile oggetto */
                                                if (GET_CARD(cl, ss, trial_ptr->npla[base + ds]) > 1)
                                                    trial_ptr->split[base + ds] =
                                                        ENCODE_ID(instance_ptr->npla[base + ds], obj);
                                                else
                                                    trial_ptr->split[base + ds] = instance_ptr->npla[base + ds];
                                                if (instance_ptr->enabling_degree) {
                                                    /* Vale la pena valutare il predicato */
                                                    if (type == INCREMENTAL) {
                                                        /* Caso di una transizione incrementale */
                                                        if (!verify_predicate_constraint(pred_ptr, trial_ptr))
                                                            trial_ptr->enabling_degree = 0;
                                                        add_partial_split(trial_ptr);
                                                    }/* Caso di una transizione incrementale */
                                                    else if (verify_predicate_constraint(pred_ptr, trial_ptr)) {
                                                        /* L'istanza parziale verifica il predicato */
                                                        add_partial_split(trial_ptr);
                                                    }/* L'istanza parziale verifica il predicato */
                                                }/* Vale la pena valutare il predicato */
                                                else {
                                                    /* Non vale la pena valutare il predicato */
                                                    if (type == INCREMENTAL) {
                                                        /* Caso di una transizione incrementale */
                                                        trial_ptr->enabling_degree = 0;
                                                        add_partial_split(trial_ptr);
                                                    }/* Caso di una transizione incrementale */
                                                }/* Non vale la pena valutare il predicato */
                                            }/* Per ogni possibile oggetto */
                                        }/* Classe non ordinata */
#ifdef REACHABILITY
                                        else {
                                            /* Classe ordinata */
                                            obj = get_object_from_dynamic(cl, instance_ptr->npla[base + ds]);
                                            dyn_id = obj + GET_CARD(cl, 0, instance_ptr->npla[base + ds]);
                                            for (; obj < dyn_id ; obj++) {
                                                /* Per ogni possibile oggetto */
                                                trial_ptr->split[base + ds] = obj;
                                                if (verify_predicate_constraint(pred_ptr, trial_ptr)) {
                                                    /* L'istanza parziale verifica il predicato */
                                                    add_partial_split(trial_ptr);
                                                }/* L'istanza parziale verifica il predicato */
                                            }/* Per ogni possibile oggetto */
                                        }/* Classe ordinata */
#endif
                                        inst_ptr = next_instance(inst_ptr);
                                    }/* Per ogni parziale */
                                    if (!new_partial_split_instances_generated()) {
                                        /* Non ci sono istanze parziali */
                                        enabled = FALSE;
                                        goto ret;
                                    }/* Non ci sono istanze parziali */
                                }/* Nuova dsc istanziata */
                        }/* Per ogni ripetizione nel dominio della transizione */
#ifdef SIMULATION
                    }/* Bisogna fare lo splitting prima */
#endif
                }/* La classe e' presente nel dominio della transizione */
            }/* Classe non ordinata */
    }/* Per ogni classe di colore */
ret: dispose_old_event(trial_ptr);
    return (enabled);
}/* End pre_split_instance */
#endif
#endif
