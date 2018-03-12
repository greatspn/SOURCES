#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"

#ifdef SIMULATION
/********* Funzioni extern **********/
#ifdef DEBUG_malloc
extern void out_token_info();
extern void out_event_info();
#endif
extern void dispose_old_event();
extern void out_error();
extern Event_p locate_in_list();
/********* Funzioni extern **********/

/********* Variabili extern **********/
extern double cur_time;
extern Event_p event_list_h;
extern Event_p event_list_t;
/********* Variabili extern **********/

int event_inst = 0;
#ifndef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void schedule_in_d_transition_list(ev_p)
Event_p ev_p;
{
    /* Init schedule_in_d_transition_list */
    int tr = GET_TRANSITION_INDEX(ev_p);

    if ((ev_p->t_next = tabt[tr].d_instances_h) == NULL) {
        /* Prima istanza */
        tabt[tr].d_instances_t = ev_p;
    }/* Prima istanza */
    else {
        /* Altra istanza */
        tabt[tr].d_instances_h->t_prev = ev_p;
    }/* Altra istanza */
    tabt[tr].d_instances_h = ev_p;
}/* End schedule_in_d_transition_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void deschedule_from_d_transition_list(ev_p)
Event_p ev_p;
{
    /* Init deschedule_from_d_transition_list */
    int tr = GET_TRANSITION_INDEX(ev_p);
    Event_p nev_p, pev_p;

    nev_p = ev_p->t_next;
    pev_p = ev_p->t_prev;
    if (pev_p == NULL)
        tabt[tr].d_instances_h = nev_p;
    else
        pev_p->t_next = nev_p;
    if (nev_p == NULL)
        tabt[tr].d_instances_t = pev_p;
    else
        nev_p->t_prev = pev_p;
    /*ev_p->e_next = ev_p->e_prev = NULL;*/
    ev_p->t_next = ev_p->t_prev = NULL;
}/* End deschedule_from_d_transition_list */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void schedule_in_transition_list(ev_p)
Event_p ev_p;
{
    /* Init schedule_in_transition_list */
    int tr = GET_TRANSITION_INDEX(ev_p);

    if ((ev_p->t_next = tabt[tr].instances_h) == NULL) {
        /* Prima istanza */
        tabt[tr].instances_t = ev_p;
    }/* Prima istanza */
    else {
        /* Altra istanza */
        tabt[tr].instances_h->t_prev = ev_p;
    }/* Altra istanza */
    tabt[tr].instances_h = ev_p;
}/* End schedule_in_transition_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void deschedule_from_transition_list(ev_p)
Event_p ev_p;
{
    /* Init deschedule_from_transition_list */
    int tr = GET_TRANSITION_INDEX(ev_p);
    Event_p nev_p, pev_p;

    nev_p = ev_p->t_next;
    pev_p = ev_p->t_prev;
    if (pev_p == NULL)
        tabt[tr].instances_h = nev_p;
    else
        pev_p->t_next = nev_p;
    if (nev_p == NULL)
        tabt[tr].instances_t = pev_p;
    else
        nev_p->t_prev = pev_p;
    /*ev_p->e_next = ev_p->e_prev = NULL;*/
    ev_p->t_next = ev_p->t_prev = NULL;
}/* End deschedule_from_transition_list */
#ifndef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void schedule_in_same_instance(ev_p , head_p)
Event_p ev_p;
Event_p head_p;
{
    /* Init schedule_in_same_instance */

    if ((head_p->e_next) == NULL && (head_p->e_prev) == NULL) {
        /* First element */
        (head_p->e_next) = (head_p->e_prev) = ev_p;
        ev_p->t_next = ev_p->t_prev = NULL;
    }/* First element */
    else {
        /* Non-First element */
        ev_p->t_prev = NULL;
        ev_p->t_next = (head_p->e_next);
        (head_p->e_next)->t_prev = ev_p;
        (head_p->e_next) = ev_p;
    }/* Non-First element */

}/* End schedule_in_same_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void deschedule_from_same_instance(ev_p , head_p)
Event_p ev_p;
Event_p head_p;
{
    /* Init deschedule_from_same_instance */
    Event_p nev_p, pev_p;

    nev_p = ev_p->t_next;
    pev_p = ev_p->t_prev;
    if (pev_p == NULL)
        (head_p->e_next) = nev_p;
    else
        pev_p->t_next = nev_p;
    if (nev_p == NULL)
        (head_p->e_prev) = pev_p;
    else
        nev_p->t_prev = pev_p;
    ev_p->t_next = ev_p->t_prev = NULL;
}/* End deschedule_from_same_instance */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void schedule_in_EL(ev_p)
Event_p ev_p;
{
    /* Init schedule_in_EL */
    Event_p pev_p, nev_p;
    Event_p pevh_p, nevh_p;

    ev_p->last_sched_desched_time = cur_time;
    if ((ev_p->residual_sched_time) >= 0.0)
        ev_p->residual_sched_time += cur_time;
    if ((pev_p = event_list_t) == NULL) {
        event_list_t = event_list_h = ev_p;
        ev_p->e_prev = ev_p->e_next = NULL;
#ifdef DEBUG_malloc
        num_event++;
#endif
        event_inst++;
        return;
    }
    for (pevh_p = nev_p = NULL, nevh_p = event_list_h ;
            pev_p != NULL && nevh_p != NULL ;
            nev_p = pev_p, pev_p = nev_p->e_prev,
            pevh_p = nevh_p, nevh_p = pevh_p->e_next) {
        if (pev_p->residual_sched_time <= ev_p->residual_sched_time)
            goto found;
        if (nevh_p->residual_sched_time > ev_p->residual_sched_time) {
            pev_p = pevh_p; nev_p = nevh_p;
            goto found;
        }
    }
found:
    if ((ev_p->e_prev = pev_p) == NULL)
        event_list_h = ev_p;
    else
        pev_p->e_next = ev_p;
    if ((ev_p->e_next = nev_p) == NULL)
        event_list_t = ev_p;
    else
        nev_p->e_prev = ev_p;
    ev_p->e_next = nev_p;
#ifdef DEBUG_malloc
    num_event++;
#endif
    event_inst++;
}/* End schedule_in_EL */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void deschedule_from_EL(ev_p)
Event_p ev_p;
{
    /* Init deschedule_from_EL */
    Event_p  pev_p, nev_p;

    if ((pev_p = ev_p->e_prev) == NULL) {
        if ((event_list_h = nev_p = ev_p->e_next) != NULL)
            nev_p->e_prev = NULL;
    }
    else
        pev_p->e_next = nev_p = ev_p->e_next;
    if (nev_p == NULL) {
        if ((event_list_t = pev_p) != NULL)
            pev_p->e_next = NULL;
    }
    else
        nev_p->e_prev = pev_p;
    ev_p->e_next = ev_p->e_prev = NULL;
#ifdef DEBUG_malloc
    num_event--;
#endif
}/* End deschedule_from_EL */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void schedule_instance(ev_p)
Event_p ev_p;
{
    /* Init schedule_instance */
    schedule_in_EL(ev_p);
    schedule_in_transition_list(ev_p);
}/* End schedule_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void deschedule_fired_instance(ev_p)
Event_p ev_p;
{
    /* Init deschedule_fired_instance */
    int tr = GET_TRANSITION_INDEX(ev_p);
    Event_p ehev_p;

    if (IS_EXPONENTIAL(tr) || IS_IMMEDIATE(tr)) {
        deschedule_from_transition_list(ev_p);
        deschedule_from_EL(ev_p);
        dispose_old_event(ev_p);
    }
#ifndef SYMBOLIC
    else {
        /* Distribuzione generale */
        ehev_p = locate_in_list(ev_p, DESCHEDULE);
        deschedule_from_EL(ev_p);
        deschedule_from_same_instance(ev_p, ehev_p);
        ehev_p->enabling_degree--;
        dispose_old_event(ev_p);
    }/* Distribuzione generale */
#endif
}/* Init deschedule_fired_instance */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void deschedule_instance(ev_p)
Event_p ev_p;
{
    /* Init deschedule_instance */
    int tr = GET_TRANSITION_INDEX(ev_p);
    Event_p xev_p, nev_p, shev_p;

#ifdef SWN
#ifdef SYMBOLIC
    deschedule_from_transition_list(ev_p);
    deschedule_from_EL(ev_p);
    dispose_old_event(ev_p);
#endif
#endif
#ifndef SYMBOLIC
    if (IS_EXPONENTIAL(tr) || IS_IMMEDIATE(tr)) {
        /* Exponential or immediate */
        deschedule_from_transition_list(ev_p);
        deschedule_from_EL(ev_p);
        dispose_old_event(ev_p);
    }/* Exponential or immediate */
    else {
        /* Gestione dei tempi di servizio per transizioni generali */
        if (IS_AGE_MEMORY(tr)) {
            /* Store remaining times if greater than zero  */
            int cnt = 0;

            shev_p = locate_in_list(ev_p, REUSE);
            xev_p = ev_p->e_next;
            for (; xev_p != NULL ;) {
                nev_p = xev_p->t_next;
                deschedule_from_EL(xev_p);
                if ((xev_p->residual_sched_time = xev_p->residual_sched_time - cur_time) <= EPSILON) {
                    deschedule_from_same_instance(xev_p, ev_p);
                    dispose_old_event(xev_p);
                }
                cnt++;
                xev_p = nev_p;
            }
            deschedule_from_transition_list(ev_p);

            if (ev_p->e_next == NULL && ev_p->e_prev == NULL)
                dispose_old_event(ev_p);
            else {
                if (shev_p == NULL) {
                    schedule_in_d_transition_list(ev_p);
                    ev_p->enabling_degree = cnt;
                }
                else {
                    shev_p->enabling_degree += cnt;
                    (shev_p->e_prev)->t_next = (ev_p->e_next);
                    (ev_p->e_next)->t_prev = (shev_p->e_prev);
                    shev_p->e_prev = ev_p->e_prev;
                    dispose_old_event(ev_p);
                }
            }
        }/* Store remaining times if greater than zero  */
        else {
            /* Flush every firing time */
            xev_p = ev_p->e_next;
            for (; xev_p != NULL ;) {
                nev_p = xev_p->t_next;
                deschedule_from_EL(xev_p);
                deschedule_from_same_instance(xev_p, ev_p);
                dispose_old_event(xev_p);
                xev_p = nev_p;
            }
            deschedule_from_transition_list(ev_p);
            dispose_old_event(ev_p);
        }/* Flush every firing time */
    }/* Gestione dei tempi di servizio per transizioni generali */
#endif
}/* End deschedule_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void deschedule_all_instances(tr)
int tr;
{
    /* Init deschedule_all_instances */
    Event_p ev_p, nev_p;

    for (; tabt[tr].instances_h != NULL ;) {
        /* Per ogni istanza */
        ev_p = tabt[tr].instances_h;
        nev_p = ev_p->t_next;
        deschedule_instance(ev_p);
        ev_p = nev_p;
    }/* Per ogni istanza */
}/* End deschedule_all_instances */
#endif
