# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"

#ifdef SWN
#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int product(value, times)
int value;
int times;
{
    /* Init product */
    int ret_value = 1;
    int ii;

    for (ii = 0; ii < times ; ii++)
        ret_value *= (value - ii);
    return (ret_value);
}/* End product */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_number_of_ordinary_instances(instance_ptr, type)
Event_p instance_ptr;
int type;
{
    /* Init get_number_of_ordinary_instances */
    int tr = GET_TRANSITION_INDEX(instance_ptr);
    int pos, prv, base, n_ist;
    int cl, ss, ds;
    int ret_value = 1;
    int member[MAX_COL_RIP];

    for (cl = ncl; cl ; cl--) {
        /* Per ogni classe non ordinata */
        if (IS_UNORDERED(cl - 1)) {
            /* Classe non ordinata */
            if (tabt[tr].rip[cl - 1]) {
                /* La classe e' presente nel dominio della transizione */
                base = tabt[tr].off[cl - 1];
#ifdef SIMULATION
                if (tabt[tr].split_type[cl - 1] == AT_FIRING_TIME) {
                    /* Si puo' fare lo split ritardato */
                    for (pos = 0 ; pos < tabt[tr].rip[cl - 1] ; pos++) {
                        /* Per ogni ripetizione nel dominio della transizione */
                        if (tabt[tr].comp_type[base + pos] == type) {
                            /* Componente in Uscita */
                            ds = GET_DYN_ID(instance_ptr, base + pos);
                            ss = get_static_subclass(cl - 1, ds);
                            ret_value *= GET_CARD(cl - 1, ss, ds);
                        }/* Componente in Uscita */
                    }/* Per ogni ripetizione nel dominio della transizione */
                }/* Si puo' fare lo split ritardato */
                else
#endif
                {
                    /* Split gia' fissato */
                    for (pos = 0 ; pos < tabt[tr].rip[cl - 1] ; pos++) {
                        /* Per ogni ripetizione nel dominio della transizione */
#ifdef SIMULATION
                        if (tabt[tr].comp_type[base + pos] == type) {
                            /* Componente in Uscita */
#endif
                            ds = GET_DYN_ID(instance_ptr, base + pos);
                            ss = get_static_subclass(cl - 1, ds);
                            n_ist = 0;
                            for (prv = 0; prv < pos ; prv++)
                                n_ist += (GET_DYN_ID(instance_ptr, base + prv) == ds);
                            if (n_ist == 0) {
                                /* Non gia' calcolata */
                                for (prv = 0; prv < tabt[tr].rip[cl - 1] ; member[prv++] = FALSE);
                                for (prv = 0; prv < tabt[tr].rip[cl - 1] ; prv++)
                                    if (ds == GET_DYN_ID(instance_ptr, base + prv))
#ifdef SIMULATION
                                        if (tabt[tr].comp_type[base + prv] == type)
#endif
                                            member[DECODE_OBJECT(GET_SPLIT_ID(instance_ptr, base + prv))] = TRUE;
                                for (prv = 0; !member[prv] ; prv++);
                                for (n_ist = prv; n_ist <  tabt[tr].rip[cl - 1] &&  member[n_ist]  ; n_ist++);
                                ret_value *= product(GET_CARD(cl - 1, ss, ds) - prv , n_ist - prv);
                            }/* Non gia' calcolata */
#ifdef SIMULATION
                        }/* Componente in Uscita */
#endif
                    }/* Per ogni ripetizione nel dominio della transizione */
                }/* Split gia' fissato */
            }/* La classe e' presente nel dominio della transizione */
        }/* Classe non ordinata */
    }/* Per ogni classe */
    return (ret_value);
}/* End get_number_of_ordinary_instances */
#endif
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/* Per le transizioni SOURCE o nel caso [P]<f> con P = FALSE	      */
/* allora non ha senso il no_serv cioe' non serve calcolare il MIN. */
/* Inoltre, nel caso [P]<f> enabling_degree = MAX_INT quindi	      */
/* controllarlo e assegnare il valore 1.			      */
/**************************************************************/
void adjust_enable_degree(Event_p  list) {
    /* Init adjust_enable_degree */
    Event_p ev_p = NULL;
    int tr = GET_TRANSITION_INDEX(list);

#ifdef SWN
#ifdef SYMBOLIC
    for (ev_p = list; ev_p != NULL; ev_p = ev_p->next) {
        /* Per ogni elemento della lista */
        if (ev_p->enabling_degree > 0)
            ev_p->ordinary_instances = get_number_of_ordinary_instances(ev_p, IN);
    }/* Per ogni elemento della lista */
#endif
#endif
#ifdef SIMULATION
#ifdef SWN
    if (IS_OUTPUT_TRANSITION(tr) || IS_SOURCE_TRANSITION(tr))
#endif
#ifdef GSPN
        if (IS_SOURCE_TRANSITION(tr)) /*** Se funzioni su arco, allora caso [P]<e> e P falso
                                    ovvero si ha una source, come per le SWN del resto **/
#endif
        {
            /* Parte di preselection */
            for (ev_p = list; ev_p != NULL; ev_p = ev_p->next) {
                /* Per ogni elemento della lista */
                if (ev_p->enabling_degree) {
                    /* L'istanza abilita veramente */
                    if (ev_p->enabling_degree == MAX_INT)
                        ev_p->enabling_degree = 1;
#ifdef SWN
                    ev_p->enabling_degree *= GET_TRANSITION_DEGREE(tr);
#endif
                }/* L'istanza abilita veramente */
            }/* Per ogni elemento della lista */
        }/* Parte di preselection */
#endif
}/* End adjust_enable_degree */
