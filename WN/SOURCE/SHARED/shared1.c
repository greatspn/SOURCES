/* Modulo utilizzato in comune da increm.c, presel.c ed enabling.c */
# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"

#ifdef SWN
#define LESSER_THAN -1
#define EQUAL_TO 0
#define GREATER_THAN 1

#ifdef SYMBOLIC
extern int get_dynamic_from_total();
#endif

extern int verify_marking_constraint();
extern int verify_marking_constraint_of_all_places();

Event_p new_enabled_list = NULL;
Event_p old_enabled_list = NULL;

int skeleton[MAX_DOMAIN];
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void add_instance(Event_p  instance_ptr) {
    /* Init add_instance */
    Event_p new_ptr = NULL;
    int i;

    new_ptr = get_new_event(instance_ptr->trans);
    new_ptr->enabling_degree = instance_ptr->enabling_degree;
    new_ptr->next = new_enabled_list;
    new_enabled_list = new_ptr;
    new_enabled_list->trans = instance_ptr->trans;
    for (i = tabt[instance_ptr->trans].comp_num; i ; i--) {
        new_enabled_list->npla[i - 1] = instance_ptr->npla[i - 1];
#ifdef SYMBOLIC
        new_enabled_list->split[i - 1] = instance_ptr->split[i - 1];
#endif
    }
}/* End add_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_enabling_phase(int  tr) {
    /* Init init_enabling_phase */
    int i_ind;

    for (i_ind = GET_TRANSITION_COMPONENTS(tr); i_ind ; skeleton[--i_ind] = NEW);
}/* End init_enabling_phase */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int is_in_function(int  rp,  int  i,  Node_p  node_ptr) {
    /* Init is_in_function */
    Coeff_p coeff_ptr = NULL;
    int present_flag = FALSE;

    for (coeff_ptr = node_ptr->arcfun ; coeff_ptr != NULL; coeff_ptr = coeff_ptr->next)
        if (coeff_ptr->coef[i].xsucc_coef[3 * rp]
                || coeff_ptr->coef[i].xsucc_coef[3 * rp + 1]
                || coeff_ptr->coef[i].xsucc_coef[3 * rp + 2]) {
            present_flag = TRUE;
            break;
        }
    return (present_flag);
}/* End is_in_function */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int project_function_on_domain(Node_p  node_ptr,  int  function_type,  int  tr,  int  mapper[MAX_DOMAIN]) {
    /* Init project_function_on_domain */
    int generate = FALSE;
    int pl , i , cl , rp , pos_in_domain;

    pl = GET_PLACE_INDEX(node_ptr);
    switch (function_type) {
    /* Tipo di funzione */
    case PROJECTION :
        for (i = 0; i < GET_PLACE_COMPONENTS(pl);  i++) {
            /* Per ogni componente del dominio del posto */
            cl = GET_COLOR_COMPONENT(i, pl);
            for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++)
                if (is_in_function(rp, i, node_ptr))
                    break;
            pos_in_domain = GET_POSITION_OF(cl, rp, tr);
            if (skeleton[pos_in_domain] == NEW) {
                /* Componente non fissata in precedenza */
                skeleton[pos_in_domain] = OLD;
                mapper[i] = pos_in_domain;
                generate = TRUE;
            }/* Componente non fissata in precedenza */
            else
                mapper[i] = -(pos_in_domain + 1);
        }/* Per ogni componente del dominio del posto */
        break;
    case COMPLEX :
        for (i = GET_TRANSITION_COMPONENTS(tr) ; i ; mapper[--i] = FALSE);
        for (i = 0; i < GET_PLACE_COMPONENTS(pl);  i++) {
            /* Per ogni componente del dominio del posto */
            cl = GET_COLOR_COMPONENT(i, pl);
            for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++) {
                /* Per ogni ripetizione */
                if (is_in_function(rp, i, node_ptr)) {
                    /* Se e' presente nella funzione */
                    pos_in_domain = GET_POSITION_OF(cl, rp, tr);
                    if (skeleton[pos_in_domain] == NEW) {
                        /* Componente non fissata in precedenza */
                        skeleton[pos_in_domain] = OLD;
                        mapper[pos_in_domain] = TRUE;
                        generate = TRUE;
                    }/* Componente non fissata in precedenza */
                }/* Se e' presente nella funzione */
            }/* Per ogni ripetizione */
        }/* Per ogni componente del dominio del posto */
        break;
    case GUARDED :
        for (i = GET_TRANSITION_COMPONENTS(tr) ; i ; i--) {
            /* Per ogni componente del dominio della transizione */
            if (skeleton[i - 1] == NEW) {
                /* Parti del dominio ancora non fissate dopo l'analisi dei posti */
                mapper[i - 1] = TRUE;
                skeleton[i - 1] = OLD;
                generate = TRUE;
            }/* Parti del dominio ancora non fissate dopo l'analisi dei posti */
            else
                mapper[i - 1] = FALSE;
        }/* Per ogni componente del dominio della transizione */
        break;
    }/* Tipo di funzione */
    return (generate);
}/* End project_function_on_domain */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int project_token_to_instance(Token_p  tok_ptr,  Event_p  trial_ptr,  int  mapper[MAX_DOMAIN],  int  pl) {
    /* Init project_token_to_instance */
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
}/* End project_token_to_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void setup_cartesian_product_of_tr(Cart_p  cartes_ptr,  int  tr,  int  mapper[MAX_DOMAIN]) {
    /* Init setup_cartesian_product_of_tr */
    int i;
#ifdef SYMBOLIC
    int cl;
#endif

    for (i = 0; i < GET_TRANSITION_COMPONENTS(tr); i++) {
        /* Per ogni componente del dominio della transizione */
        cartes_ptr->low[i] = cartes_ptr->up[i] = 0;
        if (mapper[i]) {
            /* Componente del dominio da fissare */
#ifdef SYMBOLIC
            cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
#ifdef REACHABILITY
                if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
                {
                    /* Classe non ordinata */
                    cartes_ptr->up[i] = GET_NUM_CL(cl) - 1;
                }/* Classe non ordinata */
                else
#endif
                    cartes_ptr->up[i] = GET_CARDINALITY_OF_CLASS(i, tr);
        }/* Componente del dominio da fissare */
    }/* Per ogni componente del dominio della transizione */
}/* End setup_cartesian_product_of_tr */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void project_element_to_instance(Cart_p  cartes_ptr,  Event_p  trial_ptr,  int  mapper[MAX_DOMAIN]) {
    /* Init project_element_to_instance */
    int i , tr;
#ifdef SYMBOLIC
    int cl;
#endif

    tr = GET_TRANSITION_INDEX(trial_ptr);
    for (i = GET_TRANSITION_COMPONENTS(tr); i ; i--) {
        /* Per ogni componente del dominio della transizione */
        if (mapper[i - 1]) {
            /* Componente del dominio da fissare --> copia dell'elemento */
#ifdef SYMBOLIC
            cl = GET_COLOR_IN_POSITION(i - 1, tr);
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
#ifdef REACHABILITY
                if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
                {
                    /* Classe non ordinata */
                    trial_ptr->npla[i - 1] = get_dynamic_from_total(cl, cartes_ptr->low[i - 1]);
                }/* Classe non ordinata */
                else
#endif
                    trial_ptr->npla[i - 1] = cartes_ptr->low[i - 1];
        }/* Componente del dominio da fissare --> copia dell'elemento */
    }/* Per ogni componente del dominio della transizione */
}/* End project_element_to_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void test_old_instances(Node_p  node_ptr,  int  function_type,  int  tr,  int  type) {
    /* Init test_old_instances */
    int (*marking_test)() = NULL;
    Event_p trial_ptr = NULL;
    Event_p next_ptr = NULL;

    switch (function_type) {
    /* Tipo di test */
    case PROJECTION:
    case COMPLEX:
        marking_test = verify_marking_constraint;
        break;
    case GUARDED:
        marking_test = verify_marking_constraint_of_all_places;
        break;
    }/* Tipo di test */
    trial_ptr = old_enabled_list;
    for (; trial_ptr != NULL;) {
        if (!(*marking_test)(node_ptr, trial_ptr, tr, type))
            trial_ptr = next_instance(trial_ptr);
        else {
            next_ptr = trial_ptr->next;
            trial_ptr->next = new_enabled_list;
            new_enabled_list = trial_ptr;
            trial_ptr = next_ptr;
        }
    }
}/* Init test_old_instances */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int new_instances_generated() {
    /* Init new_instances_generated */
    int ret_value = TRUE;

    if (new_enabled_list == NULL)
        ret_value = FALSE;
    else {
        /* Nuove ennuple generate */
        old_enabled_list = new_enabled_list;
        new_enabled_list = NULL;
    }/* Nuove ennuple generate */
    return (ret_value);
}/* End new_instances_generated */
#endif

