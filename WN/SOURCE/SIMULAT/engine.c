#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"

#ifdef SIMULATION
#define MAX_HOST_NAME 256
/********* Funzioni extern **********/
#ifdef DEBUG_malloc
extern void out_token_info();
extern void out_event_info();
#endif

extern FILE *efopen();

extern double random_generate();
extern double negexp();
extern double generate_firing_time();
extern double get_instance_rate();

extern void initialize_list_of_enabled_transition();
extern void dispose_old_event();
extern void out_cur_simulation_state();
extern void out_single_instance_detailed();
extern void out_error();
extern void init_stat();
extern void copy_event();
extern void adjust_enable_degree();
extern void set_output_components();
extern void deschedule_fired_instance();
extern void fire_trans();
extern void add_to_list_of_enabled();
extern void schedule_in_transition_list();
extern void deschedule_from_transition_list();
extern void schedule_in_d_transition_list();
extern void deschedule_from_d_transition_list();
extern void schedule_in_EL();
extern void schedule_instance();
extern void deschedule_from_EL();
extern void deschedule_instance();
extern void deschedule_all_instances();
extern void deschedule_from_same_instance();
extern void schedule_in_same_instance();

extern int end_transitory();
extern int compute_batch_statistics();
extern int compare_input_event_id();

void rearrange_and_schedule_symbolic_instances(int tr, Event_p fired_ptr);
void rearrange_symbolic_instances(Event_p list, int tr,  Event_p fired_ptr);

extern Event_p select_event();
extern Event_p get_new_event();
/********* Funzioni extern **********/

/********* Variabili extern **********/
extern Trans_p enab_list;
extern Event_p old_enabled_list;
/********* Variabili extern **********/

int cur_priority;
int old_made = 1;
int events_made = 1;
time_t newt;
time_t oldt;

double cur_time;
double old_time = 0.0;

Event_p event_list_h = NULL, event_list_t = NULL;

FILE *statfp = NULL;
FILE *thptfp = NULL;
FILE *timefp = NULL;

#if STATISTICFEL
extern long  numEvFEL;
extern long numEvOrdFEL;
#endif

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int compare_event_id(ev_1, ev_2, tr)
Event_p ev_1;
Event_p ev_2;
int tr;
{
    /* Init compare_event_id */
    int ii , equal = TRUE, ret_value = UNKNOWN;

#ifdef GSPN
    return (EQUAL_TO);
#endif
#ifdef SWN
    if (HAS_PRESELECTABLE_OUTPUT(tr)) {
        /* Contano solo le componenti in ingresso */
#ifdef SYMBOLIC
        for (ii = tabt[tr].comp_num ; ii && equal ; ii--) {
            /* Per ogni componente del dominio della transizione */
            equal = (ev_1->npla[ii - 1] == ev_2->npla[ii - 1]);
            equal &= (ev_1->split[ii - 1] == ev_2->split[ii - 1]);
        }/* Per ogni componente del dominio della transizione */
#endif
#ifdef COLOURED
        for (ii = tabt[tr].comp_num ; ii && equal ; ii--) {
            /* Per ogni componente del dominio della transizione */
            if (tabt[tr].comp_type[ii - 1] == IN)
                equal = (ev_1->npla[ii - 1] == ev_2->npla[ii - 1]);
        }/* Per ogni componente del dominio della transizione */
#endif
    }/* Contano solo le componenti in ingresso */
    else {
        /* Tutte le componenti */
        for (ii = tabt[tr].comp_num ; ii && equal ; ii--) {
            /* Per ogni componente del dominio della transizione */
            equal = (ev_1->npla[ii - 1] == ev_2->npla[ii - 1]);
#ifdef SYMBOLIC
            equal &= (ev_1->split[ii - 1] == ev_2->split[ii - 1]);
#endif
        }/* Per ogni componente del dominio della transizione */
    }/* Tutte le componenti */
    if (equal) {
        /* Ricerca con successo */
        ret_value = EQUAL_TO;
    }/* Ricerca con successo */
    return (ret_value);
#endif
}/* End compare_event_id */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int GET_ENABLING_DEGREE(Event_p ev_p) {
    /* Init GET_ENABLING_DEGREE */
    int ret_value = 0;

    if (ev_p != NULL) {
#ifdef GSPN
        ret_value = ev_p->enabling_degree;
#endif
#ifdef SWN
#ifdef SYMBOLIC
        if (IS_INPUT_PRESELECTABLE(ev_p->trans))
            ret_value = ev_p->enabling_degree;
        else
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
static void locate_event(ev_p , list , ret_ptr, type)
Event_p ev_p;
Event_p list;
EventInfo_p ret_ptr;
{
    /* Init locate_event */
    int tr = GET_TRANSITION_INDEX(ev_p);

    Event_p pos_ptr = NULL;
    Event_p prv_ptr = NULL;

    ret_ptr->pos = ret_ptr->prv = NULL;
    for (pos_ptr = list; pos_ptr != NULL ;) {
        /* Per ogni elemento nella lista */
        if (compare_event_id(ev_p , pos_ptr, tr) == EQUAL_TO) {
            ret_ptr->pos = pos_ptr;
            ret_ptr->prv = prv_ptr;
            return;
        }
        prv_ptr = pos_ptr;
        ret_ptr->prv = prv_ptr;
        if (type == OLD)
            pos_ptr = pos_ptr->t_next;
        else
            pos_ptr = pos_ptr->next;
    }/* Per ogni elemento nella lista */
    ret_ptr->pos = ret_ptr->prv = NULL;
    return;
}/* End locate_event */
#ifdef SIMULATION
#ifndef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Event_p locate_in_list(ev_p , type)
Event_p ev_p;
int type;
{
    /* Init locate_in_list */
    int tr = GET_TRANSITION_INDEX(ev_p);
    Event_p list = NULL;
    Event_p retev_p = NULL;
    Event_p cmpev_p;

    if (type == REUSE)
        list = tabt[tr].d_instances_h;
    else
        list = tabt[tr].instances_h;
    for (cmpev_p = list; cmpev_p != NULL ;) {
        /* Per ogni elemento nella lista */
        if (compare_event_id(ev_p , cmpev_p, tr) == EQUAL_TO) {
            retev_p = cmpev_p;
            goto ret;
        }
        cmpev_p = cmpev_p->t_next;
    }/* Per ogni elemento nella lista */
ret:  return (retev_p);
}/* End locate_in_list */
#endif
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void reschedule_instance(ev_p , old_deg, new_deg, nptr)
Event_p ev_p;
int old_deg;
int new_deg;
Event_p nptr;
{
    /* Init reschedule_instance */
    int tr = GET_TRANSITION_INDEX(ev_p);
    int ii;
    double rate;
    Event_p ehev_p, shev_p, new_ev_p;

    if (IS_IMMEDIATE(tr) || IS_EXPONENTIAL(tr)) {
        /* Transizione immediata o esponenziale */
        ev_p->enabling_degree = new_deg;
#ifdef SWN
#ifdef SYMBOLIC
        if (nptr != NULL)
            ev_p->ordinary_instances = nptr->ordinary_instances;
#endif
#endif
        if (IS_IMMEDIATE(tr))
            ev_p->residual_sched_time = -GET_TRANSITION_PRIORITY(tr);
        else
            ev_p->residual_sched_time = negexp(1.0 / (new_deg * get_instance_rate(ev_p)));

        if (old_deg == 0) {
            /* Istanza non presente in EL */
            schedule_instance(ev_p);
        }/* Istanza non presente in EL */
        else {
            /* Istanza gia' presente in EL */
            deschedule_from_EL(ev_p);
            schedule_in_EL(ev_p);
        }/* Istanza gia' presente in EL */
    }/* Transizione immediata o esponenziale */
#ifdef SIMULATION
#ifndef SYMBOLIC
    else {
        /* Transizione con distribuzione generale  */
        /********** Individua le liste relative all'istanza **********/
        ehev_p = locate_in_list(ev_p, DESCHEDULE);
        if (ehev_p != NULL)
            ehev_p->enabling_degree = new_deg;
        if (IS_AGE_MEMORY(tr))
            shev_p = locate_in_list(ev_p, REUSE);
        /********** Individua le liste relative all'istanza **********/

        if (new_deg > old_deg) {
            /* Grado di abilitazione aumentato */
            if (old_deg == 0) {
                if (ehev_p == NULL) {
                    schedule_in_transition_list(ev_p);
                    ehev_p = ev_p;
                }
            }
            for (ii = new_deg - old_deg ; ii ; ii--) {
                /* Genera i tempi di servizio che mancano */
                if (IS_AGE_MEMORY(tr)) {
                    /* retrieve possible stored times (AGE POLICY)*/
                    if (shev_p != NULL) {
                        /* At least on stored firing time */
                        new_ev_p = select_event(shev_p, REUSE);
                        deschedule_from_same_instance(new_ev_p, shev_p);
                        if (shev_p->e_prev == NULL && shev_p->e_next == NULL) {
                            deschedule_from_d_transition_list(shev_p);
                            dispose_old_event(shev_p);
                            shev_p = NULL;
                        }
                    }/* At least on stored firing time */
                    else {
                        /* No stored firing times */
                        new_ev_p = get_new_event(tr);
                        copy_event(new_ev_p, ev_p);
                        new_ev_p->residual_sched_time = generate_firing_time(tr, ev_p);
                    }/* No stored firing times */
                }/* retrieve possible stored times (AGE POLICY)*/
                else {
                    /* generate new firing times (ENABLING POLICY)*/
                    new_ev_p = get_new_event(tr);
                    copy_event(new_ev_p, ev_p);
                    new_ev_p->residual_sched_time = generate_firing_time(tr, ev_p);
                }/* generate new firing times (ENABLING POLICY)*/
                schedule_in_EL(new_ev_p);
                schedule_in_same_instance(new_ev_p , ehev_p);
            }/* Genera i tempi di servizio che mancano */
        }/* Grado di abilitazione aumentato */
        else {
            /* Grado di abilitazione diminuito */
            if (IS_AGE_MEMORY(tr)) {
                /* Cancella ma conserva */
                if (shev_p == NULL) {
                    /* Inserisci ev_p in d_instances */
                    shev_p = get_new_event(tr);
                    copy_event(shev_p, ev_p);
                    schedule_in_d_transition_list(shev_p);
                    shev_p->enabling_degree = old_deg - new_deg;
                }/* Inserisci ev_p in d_instances */
                else
                    shev_p->enabling_degree += old_deg - new_deg;
            }/* Cancella ma conserva */
            for (ii = old_deg - new_deg ; ii ; ii--) {
                /* Selezione di vittime */
                new_ev_p = select_event(ehev_p, DESCHEDULE);
                deschedule_from_same_instance(new_ev_p, ehev_p);
                deschedule_from_EL(new_ev_p);
                new_ev_p->residual_sched_time -= cur_time;
                if (IS_AGE_MEMORY(tr))
                    schedule_in_same_instance(new_ev_p, shev_p);
                else
                    dispose_old_event(new_ev_p);
            }/* Selezione di vittime */
        }/* Grado di abilitazione diminuito */
    }/* Transizione con distribuzione generale  */
#endif
#endif
}/* End reschedule_instance */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void total_scheduling(tr , list, type)
int tr;
Event_p list;
int type;
{
    /* Init total_scheduling */
    struct EVENT_INFO info;
    EventInfo_p info_ptr = &info;

    Event_p old_ptr, new_ptr, next_ptr;

    int old_deg, new_deg;

    old_ptr = tabt[tr].instances_h;
    for (; old_ptr != NULL ; old_ptr = next_ptr) {
        /* Per ogni vecchia istanza gia' nella event list */
        next_ptr = old_ptr->t_next;
        locate_event(old_ptr, list, info_ptr, NEW); /* Da scrivere */
        new_ptr = info_ptr->pos;
        new_deg = GET_ENABLING_DEGREE(new_ptr);
        if (new_deg == 0) {
            /* Non piu' presente nella lista delle istanze */
            deschedule_instance(old_ptr);
        }/* Non piu' presente nella lista delle istanze */
        else {
            /* Ancora presente nella lista delle istanze */

            old_deg = GET_ENABLING_DEGREE(old_ptr);
            if (new_deg != old_deg) {
                /* Grado di abilitazione variato */
                reschedule_instance(old_ptr, old_deg, new_deg, new_ptr);
            }/* Grado di abilitazione variato */

            if (info_ptr->prv == NULL)
                list = list->next;
            else
                (info_ptr->prv)->next = new_ptr->next;
            dispose_old_event(new_ptr);

        }/* Ancora presente nella lista delle istanze */
    }/* Per ogni vecchia istanza gia' nella event list */
    for (new_ptr = list ; new_ptr != NULL; new_ptr = next_ptr) {
        /* Per ogni istanza nuova */
        next_ptr = new_ptr->next;
        new_deg = GET_ENABLING_DEGREE(new_ptr);
        reschedule_instance(new_ptr, 0, new_deg, NULL);
    }/* Per ogni istanza nuova */
}/* End total_scheduling */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_complete(ev_p, output_type)
Event_p ev_p;
int output_type;
{
    /* Init is_complete */
    int ret_value = TRUE;

    if (output_type != COMPLEX_OUTPUT) {
        /* Ci puo' essere una sola istanza */
        int ii;
        int tr = GET_TRANSITION_INDEX(ev_p);

        for (ii = GET_TRANSITION_COMPONENTS(tr); ii; ii--)
            if (tabt[tr].comp_type[ii - 1] == IN)
                if (ev_p->npla[ii - 1] == UNKNOWN) {
                    ret_value = FALSE;
                    break;
                }
    }/* Ci puo' essere una sola istanza */
    else
        ret_value = FALSE;
    return (ret_value);
}/* End is_complete */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void incremental_scheduling(tr , list, type)
int tr;
Event_p list;
int type;
{
    /* Init incremental_scheduling */
    Event_p new_ptr, next_ptr, ev_p, nev_p;

    int old_deg, new_deg, output_type, rescheduled;
    int complete_flag = FALSE;

    new_ptr = list;
    for (; new_ptr != NULL ; new_ptr = next_ptr) {
        /* Per ogni nuova istanza */
        next_ptr = new_ptr->next;
        new_deg = new_ptr->enabling_degree; /* C'era la MACRO */
        output_type = GET_TRANSITION_OUTPUT_TYPE(tr);
        if (new_deg) {
            /* L'istanza abilita la transizione */
            rescheduled = FALSE;
            new_ptr->next = NULL;
            adjust_enable_degree(new_ptr);
            new_deg = GET_ENABLING_DEGREE(new_ptr);
            ev_p = tabt[tr].instances_h;
            for (; ev_p != NULL;) {
                /* Per ogni istanza della transizione */
                nev_p = ev_p->t_next;
                if (compare_input_event_id(ev_p , new_ptr, tr) == EQUAL_TO) {
                    /* Chiedersi se rischedulare o meno */
                    old_deg = GET_ENABLING_DEGREE(ev_p);
                    if (new_deg != old_deg) {
                        /* Da rischedulare */
                        ev_p->enabling_degree = new_ptr->enabling_degree;
#ifdef SYMBOLIC
                        ev_p->ordinary_instances = new_ptr->ordinary_instances;
#endif
                        reschedule_instance(ev_p, old_deg, new_deg, NULL);
                    }/* Da rischedulare */
                    rescheduled = TRUE; /* Spostato per le prove */
                    if (output_type != COMPLEX_OUTPUT)
                        goto finish;
                }/* Chiedersi se rischedulare o meno */
                ev_p = nev_p;
            }/* Per ogni istanza della transizione */
finish:
            if (rescheduled)
                dispose_old_event(new_ptr);
            else {
                /* Se l'istanza non c'era ed ha l'output */
                struct RESULT res;
                struct RESULT *result_ptr = &res;

                old_enabled_list = new_ptr;
                set_output_components(tr, result_ptr);
                ev_p = result_ptr->list;
                for (; ev_p != NULL; ev_p = nev_p) {
                    nev_p = ev_p->next;
                    reschedule_instance(ev_p, 0, new_deg, NULL);
                }
            }/* Se l'istanza non c'era ed ha l'output */
        }/* L'istanza abilita la transizione */
        else {
            /* Istanze da cancellare */
            complete_flag = is_complete(new_ptr, output_type);
            ev_p = tabt[tr].instances_h;
            for (; ev_p != NULL;) {
                /* Per ogni istanza della transizione */
                nev_p = ev_p->t_next;
                if (compare_input_event_id(ev_p , new_ptr, tr) == EQUAL_TO) {
                    /* Istanza da cancellare */
                    deschedule_instance(ev_p);
                    if (complete_flag)
                        goto finish2;
                }/* Istanza da cancellare */
                ev_p = nev_p;
            }/* Per ogni istanza della transizione */
finish2: dispose_old_event(new_ptr);
        }/* Istanze da cancellare */
    }/* Per ogni istanza nuova */
}/* End incremental_scheduling */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void total_update_enabling(tr, fired_ptr, list, type)
int tr;
Event_p fired_ptr;
Event_p list;
int type;
{
    /* Init total_update_enabling */

    if (type == SELF)
        deschedule_fired_instance(fired_ptr);
    add_to_list_of_enabled(tr);
    total_scheduling(tr, list, type);
}/* End total_update_enabling */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void incremental_update_enabling(tr, fired_ptr, list, type)
int tr;
Event_p fired_ptr;
Event_p list;
int type;
{
    /* Init incremental_update_enabling */


#ifdef SYMBOLIC
    rearrange_and_schedule_symbolic_instances(tr, fired_ptr);
#endif
#ifdef COLOURED
    if (type == SELF)
        deschedule_fired_instance(fired_ptr);
#endif
#ifdef SYMBOLIC
    rearrange_symbolic_instances(list, tr, fired_ptr);
#endif
    incremental_scheduling(tr, list, type);
    if (THERE_ARE_OTHER_INSTANCES(tr))
        add_to_list_of_enabled(tr);
    else
        deschedule_all_instances(tr);
#ifdef PRIMA
    if (type == SELF)
        deschedule_fired_instance(fired_ptr);
    incremental_scheduling(tr, list, type);
    if (THERE_ARE_OTHER_INSTANCES(tr))
        add_to_list_of_enabled(tr);
    else
        deschedule_all_instances(tr);
#endif
}/* End incremental_update_enabling */
#endif

static struct ENABLING end_simulation = {
    EVENT_END_SIM,
    NULL, NULL, NULL, NULL,
#ifndef SYMBOLIC
#ifdef DEBUG_simulation
    -9999,
#endif
#endif
    0.0, 0.0, 0.0,
    NULL, 0, 
#ifdef SWN
    NULL
#endif
};

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static Event_p select_immediate() {
    /* Init select_immediate */
    Event_p ev_p;
    int go;
    Trans_p t_p;
    double sss;

    for (t_p = enab_list, sss = 0.0; t_p != NULL; t_p = t_p->enabl_next)
        if (t_p->pri == cur_priority)
#ifdef GSPN
        {
            ev_p = t_p->instances_h;
            sss += (ev_p->enabling_degree * get_instance_rate(ev_p));
        }
#endif
#ifdef SWN
    for (ev_p = t_p->instances_h ; ev_p != NULL ; ev_p = ev_p->t_next)
#ifdef COLOURED
        sss += (ev_p->enabling_degree * get_instance_rate(ev_p));
#endif
#ifdef SYMBOLIC
    sss += (ev_p->ordinary_instances * ev_p->enabling_degree * get_instance_rate(ev_p));
#endif
#endif
    sss *= random_generate();
    for (t_p = enab_list, go = TRUE; go; t_p = t_p->enabl_next)
        if (t_p->pri == cur_priority)
#ifdef GSPN
        {
            ev_p = t_p->instances_h;
            if ((sss -= (ev_p->enabling_degree * get_instance_rate(ev_p))) <= 0.0)
                goto ret;
        }
#endif
#ifdef SWN
    for (ev_p = t_p->instances_h ; ev_p != NULL ; ev_p = ev_p->t_next)
#ifdef COLOURED
        if ((sss -= (ev_p->enabling_degree * get_instance_rate(ev_p))) <= 0.0)
#endif
#ifdef SYMBOLIC
            if ((sss -= (ev_p->ordinary_instances * ev_p->enabling_degree * get_instance_rate(ev_p))) <= 0.0)
#endif
                goto ret;
#endif
ret: return (ev_p);
}/* End select_immediate */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static Event_p get_next_event() {
    /* Init get_next_event */
    Event_p ev_p;
#if STATISTICFEL
    ev_p = event_list_h;
    while (ev_p != NULL) {
        if (ev_p->ordinary_instances != -2) {
            numEvFEL++;
            numEvOrdFEL += ev_p->ordinary_instances;
        }
        ev_p = ev_p->e_next;

    }
#endif

    if (cur_priority)
        return (select_immediate());
    if ((ev_p = event_list_h) == NULL) {
        end_simulation.residual_sched_time = cur_time;
        return (&end_simulation);
    }
    cur_time = ev_p->residual_sched_time;
    if (ev_p->trans < 0)
        dispose_old_event(ev_p);
    return (ev_p);
}/* End get_next_event */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void 
close_intermediate_files (char file[MAXSTRING])
{
    /* Init close_intermediate_files */
    char comm[MAXSTRING];
    char hostname[MAX_HOST_NAME];
    FILE *trs_fp = NULL;

    fclose(statfp);
    fclose(timefp);
    fclose(thptfp);

    gethostname(hostname, MAX_HOST_NAME - 1);

#ifdef GREATSPN
    sprintf(comm, "/bin/rm \"%s%strash_sim\"", file, hostname);
#else
    sprintf(comm, "/bin/rm \"nets/%strash_sim\"", hostname);
#endif
    system(comm);
#ifdef GREATSPN
    sprintf(comm, "/bin/rm \"%s%strash_time\"", file, hostname);
#else
    sprintf(comm, "/bin/rm \"nets/%strash_time\"", hostname);
#endif
    system(comm);
#ifdef GREATSPN
    sprintf(comm, "/bin/rm \"%s%strash_thp\"", file, hostname);
#else
    sprintf(comm, "/bin/rm \"nets/%strash_thp\"", hostname);
#endif
    system(comm);
}/* End close_intermediate_files */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void 
open_intermediate_files (char file[MAXSTRING])
{
    /* Init open_intermediate_files */
    char hostname[MAX_HOST_NAME], comm[MAXSTRING], trs_file[MAXSTRING];
    FILE *trs_fp = NULL;

    gethostname(hostname, MAX_HOST_NAME - 1);
    
#ifdef GREATSPN
    sprintf(trs_file, "%s%strash_sim", file, hostname);
#else
    sprintf(trs_file, "nets/%strash_sim", hostname);
#endif
    if ((statfp = fopen(trs_file, "r")) != NULL) {
        fclose(statfp);
        sprintf(comm, "/bin/rm \"%s\"", trs_file);
        system(comm);
    }
#ifdef DACANCELLARE_FCLOSE_SU_NULL_DA_ERRORE
    else
        fclose(statfp);
#endif
#ifdef GREATSPN
    sprintf(trs_file, "%s%strash_time", file, hostname);
#else
    sprintf(trs_file, "nets/%strash_time", hostname);
#endif
    if ((timefp = fopen(trs_file, "r")) != NULL) {
        fclose(timefp);
        sprintf(comm, "/bin/rm \"%s\"", trs_file);
        system(comm);
    }
#ifdef DACANCELLARE_FCLOSE_SU_NULL_DA_ERRORE
    else
        fclose(timefp);
#endif
#ifdef GREATSPN
    sprintf(trs_file, "%s%strash_thp", file, hostname);
#else
    sprintf(trs_file, "nets/%strash_thp", hostname);
#endif
    if ((thptfp = fopen(trs_file, "r")) != NULL) {
        fclose(thptfp);
        sprintf(comm, "/bin/rm \"%s\"", trs_file);
        system(comm);
    }
#ifdef DACANCELLARE_FCLOSE_SU_NULL_DA_ERRORE
    else
        fclose(thptfp);
#endif
#ifdef GREATSPN
    sprintf(trs_file, "%s%strash_sim", file, hostname);
#else
    sprintf(trs_file, "nets/%strash_sim", hostname);
#endif
    statfp = efopen(trs_file, "a+");
#ifdef GREATSPN
    sprintf(trs_file, "%s%strash_time", file, hostname);
#else
    sprintf(trs_file, "nets/%strash_time", hostname);
#endif
    timefp = efopen(trs_file, "a+");
#ifdef GREATSPN
    sprintf(trs_file, "%s%strash_thp", file, hostname);
#else
    sprintf(trs_file, "nets/%strash_thp", hostname);
#endif
    thptfp = efopen(trs_file, "a+");
}/* End open_intermediate_files */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void forward_sim(char file[MAXSTRING], int first_trans_length, int trans_length, 
                 const BatchLengthMode batch_length_mode,
                 int min_len, int max_len, double min_batch_duration, double max_batch_duration,
                 int approx, int conf)
{
    /* Init forward_sim */
    Event_p ev_p;
    int ev_type, again_flag = TRUE;

    init_stat();
    open_intermediate_files(file);
    time(&oldt);
    if (output_flag && start == 0) {
        fprintf(stdout, "INITIAL MARKING\n");
        out_cur_simulation_state(stdout);
    }

    while (again_flag) {
        /* Loop del simulatore */
        old_time = cur_time;
        ev_p = get_next_event();
        if ((ev_type = ev_p->trans) >= 0) {
            /* Evento legale */
            if (end_transitory(first_trans_length, trans_length, min_len, max_len, 
                               min_batch_duration, max_batch_duration, batch_length_mode)) 
            {
                // Fine del transitorio ed inizio raccolta statistiche 
                again_flag = compute_batch_statistics(ev_type, approx, conf, file, batch_length_mode);
            }

            if (output_flag && events_made >= start) {
                fprintf(stdout, "Marking %d after firing of %s: \n", events_made, TRANS_NAME(ev_p->trans));
                out_single_instance_detailed(ev_p, stdout);
            }
            fire_trans(ev_p);
#ifdef SELF_SIMILAR
            if (tabt[ev_p->trans].trans_name[0] == 'l' &&
                    tabt[ev_p->trans].trans_name[1] == 'd')
                fprintf(stdout, "%10.12f\n", cur_time);
#endif

            if (output_flag && events_made >= start)
                out_cur_simulation_state(stdout);

            events_made++;
        }/* Evento legale */
        else {
            /* Evento di fine simulazione o breakpoint */
            out_error(ERROR_DEAD_MARKING, 0, 0, 0, 0, NULL, NULL);
        }/* Evento di fine simulazione o breakpoint */
    }/* Loop del simulatore */
    time(&newt);
    close_intermediate_files(file);
}/* End forward_sim */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void 
init_events (void) {
    /* Init init_events */

    cur_priority = 0;
    initialize_list_of_enabled_transition();
#ifdef DEBUG_malloc
    min_ev = max_ev = num_event;
#endif
}/* End init_events */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initial_scheduling(tr , list, type)
int tr;
Event_p list;
int type;
{
    /* Init initial_scheduling */
    Event_p new_ptr, next_ptr;

    int new_deg;

    for (new_ptr = list ; new_ptr != NULL; new_ptr = next_ptr) {
        /* Per ogni istanza nuova */
        next_ptr = new_ptr->next;
        new_deg = GET_ENABLING_DEGREE(new_ptr);
        reschedule_instance(new_ptr, 0, new_deg, NULL);
    }/* Per ogni istanza nuova */
}/* End initial_scheduling */
#endif
