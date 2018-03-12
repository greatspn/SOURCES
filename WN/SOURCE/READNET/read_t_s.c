# include <stdio.h>
# include <string.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/shared.h"
# include "../../INCLUDE/service.h"

#ifdef SWN
#ifdef SIMULATION
extern void compute_domain();
extern void dispose_old_event();

extern int other_domain();
extern int all_projection_functions();
extern int verify_predicate_constraint();
extern int is_in_function();
extern int is_source();
extern int nextvec();
extern int nextvec_with_fix();
extern Event_p get_new_event();

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int overlapping_domains(int tr) {
    /* Init overlapping_domains */
    int ret_value = FALSE;
    int skeleton[MAX_DOMAIN];
    int i, pl, cl, rp, pos_in_domain;
    Node_p node_ptr = NULL;

    for (i = GET_TRANSITION_COMPONENTS(tr); i ; skeleton[--i] = NEW);
    for (node_ptr = GET_INPUT_LIST(tr); node_ptr != NULL; node_ptr = NEXT_NODE(node_ptr)) {
        /* Per ogni posto in ingresso */
        pl = GET_PLACE_INDEX(node_ptr);
        if (!(IS_NEUTRAL(pl) || IS_INDEPENDENT(node_ptr))) {
            /* Posto colorato e con funzione */
            for (i = 0; i < GET_PLACE_COMPONENTS(pl);  i++) {
                /* Per ogni componente del dominio del posto */
                cl = GET_COLOR_COMPONENT(i, pl);
                for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++)
                    if (is_in_function(rp, i, node_ptr))
                        break;
                pos_in_domain = GET_POSITION_OF(cl, rp, tr);
                if (skeleton[pos_in_domain] == OLD) {
                    /* Componente fissata in precedenza */
                    ret_value = TRUE;
                    goto ret;
                }/* Componente fissata in precedenza */
                else
                    skeleton[pos_in_domain] = OLD;
            }/* Per ogni componente del dominio del posto */
        }/* Posto colorato e con funzione */
    }/* Per ogni posto in ingresso */
ret: return (ret_value);
}/* End overlapping_domains */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int is_input_preselectable(int tr) {
    /* Init is_input_preselectable */
    int ret_value = FALSE;

    if (IS_EXPONENTIAL(tr) || IS_IMMEDIATE(tr))
        if (GET_TRANSITION_PREDICATE(tr) == NULL)
            if (GET_INHIBITOR_LIST(tr) == NULL)
                if (all_projection_functions(tr))
                    if (!overlapping_domains(tr))
                        ret_value = TRUE;
    return (ret_value);
}/* End is_input_preselectable */
/*****************************************************************
nome	       :
descrizione    :
parametri      :

valori ritorno :
******************************************************************/
static int has_output_components(int tr) {
    /* Init has_output_components */
    int dom[MAX_DOMAIN];

    compute_domain(tr, dom);
    return (other_domain(tr, dom));
}/* End has_output_components */
/***************************************************************/
/* NAME 	: no_guarded_input			       */
/* DESCRIPTION	: Controlla se una transizione possiede almeno */
/*		  un arco in ingresso o inibitore etichettato  */
/*		  con una funzione con predicato.	       */
/* PARAMETERS	: Indice della transizione in esame.	       */
/* RETURN VALUE : TRUE se la condizione e' verificata, FALSE   */
/*		  altrimenti.				       */
/***************************************************************/
static int no_guarded_input(int tr) {
    /* Init no_guarded_input */
    Node_p node_ptr = NULL;
    int ret_value = TRUE;

    for (node_ptr = GET_INPUT_LIST(tr); node_ptr != NULL;) {
        /* Controllo di ogni posto in ingresso */
        if (node_ptr->type == GUARDED) {
            /* Una funzione con predicato */
            ret_value = FALSE;
            goto ret;
        }/* Una funzione con predicato */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Controllo di ogni posto in ingresso */
    for (node_ptr = GET_INHIBITOR_LIST(tr); node_ptr != NULL;) {
        /* Controllo di ogni posto inibitore */
        if (node_ptr->type == GUARDED) {
            /* Una funzione con predicato */
            ret_value = FALSE;
            goto ret;
        }/* Una funzione con predicato */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Controllo di ogni posto inibitore */
ret : return (ret_value);
}/* End no_guarded_input */
/**************************************************************/
/* NAME 	: predicate_has_not_OR			      */
/* DESCRIPTION	: Controlla se il predicato della transizione */
/*		  contiene almeno un operatore OR.	      */
/* PARAMETERS	: Indice della transizione da controllare.    */
/* RETURN VALUE : TRUE se la condizione e' verificata, FALSE  */
/*		  altrimenti.				      */
/**************************************************************/
static int predicate_has_not_OR(int tr) {
    /* Init predicate_has_not_OR */
    Pred_p p = NULL;
    int ret_value = TRUE;

    p = GET_TRANSITION_PREDICATE(tr);
    if (p == NULL)
        goto ret;
    else {
        /* C'e' un predicato */
        for (; p->type != TYPENULL && p->type != TYPEEND ; p = p->next)
            if (p->type == TYPEOR) {
                /* C'e' un connettore OR */
                ret_value = FALSE;
                goto ret;
            }/* C'e' un connettore OR */
    }/* C'e' un predicato */
ret : return (ret_value);
}/* End predicate_has_not_OR */
/*******************************************************************/
/* NAME 	: get_output_type				   */
/* DESCRIPTION	: Assegna il tipo di output corrispondente ad	   */
/*		  una transizione: senza output, transizione	   */
/*		  sorgente, transizione con output e capacita'     */
/*		  di preselection e transizione senza preselection */
/* PARAMETERS	: Indice della transizione da controllare.	   */
/* RETURN VALUE : NO_OUTPUT_COMPONENTS				   */
/*		  SOURCE_TRANSITION				   */
/*		  PRESELECTION_TRANSITION			   */
/*		  COMPLEX_OUTPUT				   */
/*******************************************************************/
static int get_output_type(int tr) {
    /* Init get_output_type */
    int ret_value = UNKNOWN;

    if (has_output_components(tr)) {
        /* Parti del dominio determinate dalle funzioni in uscita */
        if ((IS_EXPONENTIAL(tr) || IS_IMMEDIATE(tr)) && predicate_has_not_OR(tr) && no_guarded_input(tr)) {
            /* Possibilita' di preselection */
            if (is_source(tr))
                ret_value = SOURCE_TRANSITION;
            else
                ret_value = PRESELECTION_TRANSITION;
        }/* Possibilita' di preselection */
        else
            ret_value = COMPLEX_OUTPUT;
    }/* Parti del dominio determinate dalle funzioni in uscita */
    else
        ret_value = NO_OUTPUT_COMPONENTS;
    return (ret_value);
}/* End get_output_type */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_transition_preselection_rate(int tr,  int out_type) {
    /* Init get_transition_preselection_rate */
    int ret_value = 0;

    if (out_type == SOURCE_TRANSITION || out_type == PRESELECTION_TRANSITION) {
        /* Transizioni per cui e' possibile calcolare un tasso di preselection */
        Event_p trial_ptr = NULL;

        Pred_p pred_ptr = GET_TRANSITION_PREDICATE(tr);

        int fix[MAX_PRODUCT];
        int dom[MAX_DOMAIN];

        int comp = GET_TRANSITION_COMPONENTS(tr);
        int i, cl;


        compute_domain(tr, dom);
        for (i = 0; i < comp ; i++) {
            cl = GET_COLOR_IN_POSITION(i, tr);
            cart_trans_ptr->low[i] = 0;
            cart_trans_ptr->up[i] = GET_CLASS_CARDINALITY(cl) - 1;
        }
        trial_ptr = get_new_event(tr);
        switch (out_type) {
        /* Vari casi di tipo di uscita */
        case SOURCE_TRANSITION :
            for (i = 0 ; i < comp ; i++)
                fix[i] = FALSE;
            break;
        case PRESELECTION_TRANSITION :
            do {
                for (i = 0 ; i < comp ; i++) {
                    cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef SYMBOLIC
                    if (IS_UNORDERED(cl)) {
                        trial_ptr->npla[i] = get_dynamic_from_object(cl, cart_trans_ptr->low[i]);
                        trial_ptr->split[i] = cart_trans_ptr->low[i];
                    }
                    else
#endif
                        trial_ptr->npla[i] = cart_trans_ptr->low[i];
                }
            }
            while (nextvec(cart_trans_ptr, comp) && !verify_predicate_constraint(pred_ptr, trial_ptr));

            for (i = 0 ; i < comp ; i++)
                if (dom[i]) {
                    cart_trans_ptr->low[i] = cart_trans_ptr->up[i] = trial_ptr->npla[i];
                    fix[i] = TRUE;
                }
                else {
                    cl = GET_COLOR_IN_POSITION(i, tr);
                    cart_trans_ptr->low[i] = 0;
                    cart_trans_ptr->up[i] = GET_CLASS_CARDINALITY(cl) - 1;
                    fix[i] = FALSE;
                }
            break;
        }/* Vari casi di tipo di uscita */
        do {
            for (i = 0 ; i < comp ; i++) {
                cl = GET_COLOR_IN_POSITION(i, tr);
#ifdef SYMBOLIC
                if (IS_UNORDERED(cl)) {
                    if (!dom[i]) {
                        trial_ptr->npla[i] = get_dynamic_from_object(cl, cart_trans_ptr->low[i]);
                        trial_ptr->split[i] = cart_trans_ptr->low[i];
                    }
                }
                else
#endif
                    trial_ptr->npla[i] = cart_trans_ptr->low[i];
            }
            ret_value += verify_predicate_constraint(pred_ptr, trial_ptr);
        }
        while (nextvec_with_fix(cart_trans_ptr, comp, fix));
        dispose_old_event(trial_ptr);
    }/* Transizioni per cui e' possibile calcolare un tasso di preselection */
    return (ret_value);
}/* End get_transition_preselection_rate */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int predicate_without_identity_constraint(int cl, Pred_p p) {
    /* Init predicate_without_identity_constraint */
    int ret_value = TRUE;

    if (p == NULL)
        goto ret;
    else {
        /* C'e' un predicato */
        for (; p->type != TYPENULL && p->type != TYPEEND ; p = p->next)
            /**************** CONTROLLARE MOOOOOOOOOLTO BENE ***************
                 if( p->type == TYPEX && p->col_ind == cl)
            ***************************************************************/
            if (p->type == TYPESTRING && p->col_ind == cl) {
                /* C'e' un x<>y o x = y	*/
                ret_value = FALSE;
                goto ret;
            }/* C'e' un x<>y o x = y	 */
    }/* C'e' un predicato */
ret : return (ret_value);
}/* End predicate_without_identity_constraint */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int places_without_sum_functions(int cl,  int tr) {
    /* Init places_without_sum_functions */
    int ret_value = TRUE;
    int pl, rp, pos, num_of_coef;
    Node_p node_ptr = NULL;
    Coeff_p c_ptr = NULL;

    node_ptr = GET_INPUT_LIST(tr);
    for (; node_ptr != NULL;) {
        /* Per ogni posto con funzione complessa */
        if (node_ptr->type == COMPLEX) {
            /* Funzione complessa */
            c_ptr = node_ptr->arcfun;
            if (c_ptr->next != NULL) {
                /* Somma di tuple */
                ret_value = FALSE;
                goto ret;
            }/* Somma di tuple */
            else {
                /* Non c'e' una somma di tuple */
                pl = GET_PLACE_INDEX(node_ptr);
                for (pos = 0; pos < GET_PLACE_COMPONENTS(pl); pos++)
                    if (GET_COLOR_COMPONENT(pos, pl) == cl) {
                        /* La classe e' nel dominio del posto */
                        num_of_coef = 0;
                        for (rp = 0; rp < tabt[tr].rip[cl] ; rp++)
                            num_of_coef = is_in_function(rp, pos, node_ptr);
                        if (num_of_coef > 1) {
                            /* C'e' una somma di proiezioni */
                            ret_value = FALSE;
                            goto ret;
                        }/* C'e' una somma di proiezioni */
                    }/* La classe e' nel dominio del posto */
            }/* Non c'e' una somma di tuple */
        }/* Funzione complessa */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto con funzione complessa */
    node_ptr = GET_INHIBITOR_LIST(tr);
    for (; node_ptr != NULL;) {
        /* Per ogni posto con funzione complessa */
        if (node_ptr->type == COMPLEX) {
            /* Funzione complessa */
            c_ptr = node_ptr->arcfun;
            if (c_ptr->next != NULL) {
                /* Somma di tuple */
                ret_value = FALSE;
                goto ret;
            }/* Somma di tuple */
            else {
                /* Non c'e' una somma di tuple */
                pl = GET_PLACE_INDEX(node_ptr);
                for (pos = 0; pos < GET_PLACE_COMPONENTS(pl); pos++)
                    if (GET_COLOR_COMPONENT(pos, pl) == cl) {
                        /* La classe e' nel dominio del posto */
                        num_of_coef = 0;
                        for (rp = 0; rp < tabt[tr].rip[cl] ; rp++)
                            num_of_coef = is_in_function(rp, pos, node_ptr);
                        if (num_of_coef > 1) {
                            /* C'e' una somma di proiezioni */
                            ret_value = FALSE;
                            goto ret;
                        }/* C'e' una somma di proiezioni */
                    }/* La classe e' nel dominio del posto */
            }/* Non c'e' una somma di tuple */
        }/* Funzione complessa */
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto con funzione complessa */
ret: return (ret_value);
}/* End places_without_sum_functions */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int places_without_guarded_functions_with_identity_constraint(int cl, int tr) {
    /* Init places_without_guarded_functions_with_identity_constraint */
    Node_p node_ptr = NULL;
    Pred_p pred_ptr = NULL;
    Coeff_p coef_ptr = NULL;
    int ret_value = TRUE;

    node_ptr = GET_INPUT_LIST(tr);
    for (; node_ptr != NULL;) {
        /* Per ogni posto con funzione con guardia */
        if (node_ptr->type == GUARDED) {
            for (coef_ptr = node->arcfun; coef_ptr != NULL ; coef_ptr = coef_ptr->next) {
                pred_ptr = coef_ptr->guard;
                ret_value = predicate_without_identity_constraint(cl, pred_ptr);
                if (!ret_value)
                    goto ret;
            }
        }
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto con funzione con guardia */
    node_ptr = GET_INHIBITOR_LIST(tr);
    for (; node_ptr != NULL;) {
        /* Per ogni posto con funzione con guardia */
        if (node_ptr->type == GUARDED) {
            for (coef_ptr = node->arcfun; coef_ptr != NULL ; coef_ptr = coef_ptr->next) {
                pred_ptr = coef_ptr->guard;
                ret_value = predicate_without_identity_constraint(cl, pred_ptr);
                if (!ret_value)
                    goto ret;
            }
        }
        node_ptr = NEXT_NODE(node_ptr);
    }/* Per ogni posto con funzione con guardia */
ret: return (ret_value);
}/* End places_without_guarded_functions_with_identity_constraint */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_split_type_field(int tr) {
    /* Init fill_split_type_field */
    int cl;

#ifdef SYMBOLIC
    tabt[tr].pre_split = FALSE;
    for (cl = 0; cl < ncl; cl++)
        if (IS_UNORDERED(cl)) {
            /* Classe non ordinata */
            if (tabt[tr].rip[cl]) {
                /* La classe appartiene al dominio */
                if (!(
                            predicate_without_identity_constraint(cl, GET_TRANSITION_PREDICATE(tr)) &&
                            places_without_sum_functions(cl, tr) &&
                            places_without_guarded_functions_with_identity_constraint(cl, tr)
                        )
                   ) {
                    tabt[tr].split_type[cl] = AT_ENABLING_TIME;
                    tabt[tr].pre_split = TRUE;
                }
                else
                    tabt[tr].split_type[cl] = AT_FIRING_TIME;
            }/* La classe appartiene al dominio */
        }/* Classe non ordinata */
#endif
}/* End fill_split_type_field */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_preselection_rate_field() {
    /* Init fill_preselection_rate_field */
    int out_type;
    int tr;

    for (tr = 0; tr < ntr ; tr ++) {
        tabt[tr].output_type = out_type = get_output_type(tr);
        tabt[tr].en_deg = get_transition_preselection_rate(tr, out_type);
    }
}/* End fill_preselection_rate_field */
#endif
#ifdef SYMBOLIC
#ifdef REACHABILITY
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void successor_or_predecessor(int tr) {
    /* Init successor_or_predecessor */
    Node_p node_ptr = NULL;
    Coeff_p c_ptr = NULL;
    int comp, pl;
    int i, c, r, cl;
    int pre = 0;
    int suc = 0;

    for (cl = 0; cl < ncl; cl++)
        if (IS_ORDERED(cl))
            if (tabt[tr].rip[cl]) {
                for (r = 0; r < tabt[tr].rip[cl]; r++) {
                    for (node_ptr = get_place_list(tr, INPUT); node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
                        /* Per ogni nodo della lista */
                        pl = GET_PLACE_INDEX(node_ptr);
                        comp = GET_PLACE_COMPONENTS(pl);
                        for (c_ptr = node_ptr->arcfun; c_ptr != NULL ; c_ptr = c_ptr->next) {
                            /* Per ogni tupla della C.L. */
                            for (i = 0; i < comp ; i++) {
                                /* Per ogni componente del dominio del posto */
                                c = GET_COLOR_COMPONENT(i, pl);
                                if (c == cl) {
                                    if (c_ptr->coef[i].xsucc_coef[3 * i + 1] != 0) {
                                        if (tabt[tr].split_type[OFF(tr, cl) + r] == UNKNOWN) {
                                            tabt[tr].split_type[OFF(tr, cl) + r] = SUCCESSOR;
                                        }
                                        else {
                                            if (tabt[tr].split_type[OFF(tr, cl) + r] == PREDECESSOR)
                                                tabt[tr].split_type[OFF(tr, cl) + r] = COMPLEX;
                                        }
                                    }
                                    if (c_ptr->coef[i].xsucc_coef[3 * i + 2] != 0) {
                                        if (tabt[tr].split_type[OFF(tr, cl) + r] == UNKNOWN)
                                            tabt[tr].split_type[OFF(tr, cl) + r] = PREDECESSOR;
                                        else {
                                            if (tabt[tr].split_type[OFF(tr, cl) + r] == SUCCESSOR)
                                                tabt[tr].split_type[OFF(tr, cl) + r] = COMPLEX;
                                        }
                                    }
                                }
                            }/* Per ogni componente del dominio del posto */
                        }/* Per ogni tupla della C.L. */
                    }/* Per ogni nodo della lista */
                    for (node_ptr = get_place_list(tr, OUTPUT); node_ptr != NULL ; node_ptr = NEXT_NODE(node_ptr)) {
                        /* Per ogni nodo della lista */
                        pl = GET_PLACE_INDEX(node_ptr);
                        comp = GET_PLACE_COMPONENTS(pl);
                        for (c_ptr = node_ptr->arcfun; c_ptr != NULL ; c_ptr = c_ptr->next) {
                            /* Per ogni tupla della C.L. */
                            for (i = 0; i < comp ; i++) {
                                /* Per ogni componente del dominio del posto */
                                c = GET_COLOR_COMPONENT(i, pl);
                                if (c == cl) {
                                    if (c_ptr->coef[i].xsucc_coef[3 * i + 1] != 0) {
                                        if (tabt[tr].split_type[OFF(tr, cl) + r] == UNKNOWN)
                                            tabt[tr].split_type[OFF(tr, cl) + r] = SUCCESSOR;
                                        else {
                                            if (tabt[tr].split_type[OFF(tr, cl) + r] == PREDECESSOR)
                                                tabt[tr].split_type[OFF(tr, cl) + r] = COMPLEX;
                                        }
                                    }
                                    if (c_ptr->coef[i].xsucc_coef[3 * i + 2] != 0) {
                                        if (tabt[tr].split_type[OFF(tr, cl) + r] == UNKNOWN)
                                            tabt[tr].split_type[OFF(tr, cl) + r] = PREDECESSOR;
                                        else {
                                            if (tabt[tr].split_type[OFF(tr, cl) + r] == SUCCESSOR)
                                                tabt[tr].split_type[OFF(tr, cl) + r] = COMPLEX;
                                        }
                                    }
                                }
                            }/* Per ogni componente del dominio del posto */
                        }/* Per ogni tupla della C.L. */
                    }/* Per ogni nodo della lista */
                }
            }
}/* End successor_or_predecessor */
#endif
#endif
#endif
