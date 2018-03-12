#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"
#include <math.h>

#ifdef SIMULATION
#ifndef SYMBOLIC
extern double negexp();
extern double random_generate();
extern double get_instance_rate();

extern double cur_time;

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Event_p select_event(ev_p, type)
Event_p ev_p;
int type;
{
    /* Init select_event */
    int tr = GET_TRANSITION_INDEX(ev_p);
    int nist = ev_p->enabling_degree;
    int policy, ii;
    Event_p ret_p = NULL;
    Event_p pev_p = NULL;
    Event_p nev_p = NULL;

    if (type == REUSE) {
        policy = tabt[tr].reuse_policy;
        ev_p->enabling_degree--;
    }
    if (type == DESCHEDULE)
        policy = tabt[tr].deschedule_policy;
    switch (policy) {
    /* Tipo di scelta dell'istanza da deschedulare */
    case POLICY_DIS_RANDOM :
        ii = (int)(random_generate() * nist + 1.0);
        if (ii > nist)
            ii = nist;
        for (pev_p = NULL, nev_p = (ret_p = ev_p->e_next)->t_next ; --ii ;
                pev_p = ret_p, ret_p = nev_p, nev_p = nev_p->t_next) {
        }
        break;
    case POLICY_DIS_FIRST_DRAWN :
        for (ii = nist,
                nev_p = (ret_p = ev_p->e_next)->t_next ;
                --ii ;
                nev_p = nev_p->t_next)
            if (nev_p->creation_time < ret_p->creation_time) {
                ret_p = nev_p;
            }
        nev_p = ret_p->t_next;
        pev_p = ret_p->t_prev;
        break;
    case POLICY_DIS_LAST_DRAWN :
        for (ii = nist,
                nev_p = (ret_p = ev_p->e_next)->t_next ;
                --ii ;
                nev_p = nev_p->t_next)
            if (nev_p->creation_time > ret_p->creation_time) {
                ret_p = nev_p;
            }
        nev_p = ret_p->t_next;
        pev_p = ret_p->t_prev;
        break;
    case POLICY_DIS_FIRST_SCHED :
        for (ii = nist,
                nev_p = (ret_p = ev_p->e_next)->t_next ;
                --ii ;
                nev_p = nev_p->t_next)
            if (nev_p->residual_sched_time < ret_p->residual_sched_time) {
                ret_p = nev_p;
            }
        nev_p = ret_p->t_next;
        pev_p = ret_p->t_prev;
        break;
    case POLICY_DIS_LAST_SCHED :
        for (ii = nist,
                nev_p = (ret_p = ev_p->e_next)->t_next ;
                --ii ;
                nev_p = nev_p->t_next)
            if (nev_p->residual_sched_time > ret_p->residual_sched_time) {
                ret_p = nev_p;
            }
        nev_p = ret_p->t_next;
        pev_p = ret_p->t_prev;
        break;
    }/* Tipo di scelta dell'istanza da deschedulare */
//ret:
    return (ret_p);
}/* End select_event */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
double generate_firing_time(tr, ev_p)
int tr;
Event_p ev_p;
{
    /* Init generate_firing_time */
    int i;
    double tot = 0.0, sched_time = 0.0, vv = 0.0;

    switch (tabt[tr].timing) {
    /* tipo di distribuzione */
    case TIMING_DETERMINISTIC :
        sched_time = get_instance_rate(ev_p);
        break;
    case TIMING_ERLANG :
        for (i = 0 ; i < tabt[tr].stages ; i++)
            sched_time += negexp(1.0 / (tabt[tr].stages * get_instance_rate(ev_p)));
        break;
    case TIMING_IPOEXP :
        for (i = 0 ; i < tabt[tr].stages ; i++)
            sched_time += negexp(1.0 / (tabt[tr].dist[i]));
        break;
    case TIMING_IPEREXP :
        for (i = 0 ; i < tabt[tr].stages ; i++)
            tot += tabt[tr].dist[2 * i];
        tot *= random_generate();
        for (i = 0 ; i < tabt[tr].stages ; i++)
            if ((tot -= tabt[tr].dist[2 * i]) <= 0.0)
                break;
        sched_time = negexp(1.0 / (tabt[tr].dist[i]));
        break;
    case TIMING_UNIFORM :
        sched_time = tabt[tr].dist[0] + random_generate() * (tabt[tr].dist[1] - tabt[tr].dist[0]);
        break;
    }/* tipo di distribuzione */
    return (sched_time);
}/* End generate_firing_time */
#endif
#endif
