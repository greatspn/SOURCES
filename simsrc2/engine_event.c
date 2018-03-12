/********************************************************************
 *
 * Date: Torino, August 25, 1993
 * Program: engine_event.c
 * Description: event-driven simulation kernel for simulation engine
 *              of GreatSPN 1.6 package
 * Programmer: Giovanni Chiola
 * notes:
 *   1) This program allows both normal (forward) and reversed
 *      (backward) simulation. Periodic checkpoints are stored on file
 *      in order to allow both extended backtracking and possibility
 *      of continuation and resume of previous runs.
 *   2) In simulation the timing semantics of transitions is extended
 *      in order to include non-exponential distributions.
 *
 *      The available distributions are:
 *      - Exponential
 *      - Deterministic
 *      - Discrete with up to 32 points
 *      - Step-wise Uniform with up to 32 intervals
 *      - Step-wise Linear with up to 32 intervals
 *      - Cox in normalized form with up to 32 stages
 *      - Iperexponential with up to 32 stages
 *      - Erlang with up to 32 stages.
 *
 *      Both Enabling and Age Memory policies are implemented.
 *      In both cases the instances of firing for multiple server non
 *      Exponential transition can be selected for disabling according
 *      to one of the following criteria:
 *      - Random
 *      - First drawn instance among enabled ones
 *      - Last drawn instance among enabled ones
 *      - First scheduled instance among the enabled ones
 *      - Last scheduled instance among the enabled ones;
 *
 *      Similarly, in case of Age Memory the instance to be re-scheduled
 *      can be selected according to one of the following criteria:
 *      - Random
 *      - First drawn instance among stored ones
 *      - Last drawn instance among stored ones
 *      - First de-scheduled instance among stored ones
 *      - Last de-scheduled instance among stored ones;
 *      In case of Erlang, Iperexponential, and Cox distributions,
 *      the remaining time can be either kept from the previous event
 *      or redrawn from a distribution with adjusted parameters in order
 *      to have the same mean value as the previous instance of remaining
 *      time.
 *
 ********************************************************************/


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../WN/INCLUDE/compact.h"

/*#define SunOS5x*/


/*#define DEBUG*/


/*
#define DEBUGpipe
*/


#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#define DEBUG_events
/*
*/

/*
#define DEBUG_graphics
*/

/*
#define DEBUG_socket
*/

#endif

/*#if defined(Linux) || defined(SunOS5x)*/
#if !defined(SunOS4x)
#	include <errno.h>
#endif

#ifdef DEBUGpipe
#	define STORE_COMPACT(a,f) store_compact(a,f); \
				fprintf(stderr,"engine: store_compact(%ld)\n",a)
#	define STORE_DOUBLE(a,f) store_double(a,f); \
			fprintf(stderr,"engine: store_double(%lg)\n",*(a))
#else
#	define STORE_COMPACT(a,f) store_compact(a,f)
#	define STORE_DOUBLE(a,f) store_double(a,f)
#endif /* DEBUGpipe */

#include "engine_event.h"
#include "engine_decl.h"
#include "engine_pn.h"
#include "engine_control.h"

#define a   16807
#define modulo 2147483647
#define mi 127774
#define r 13971

static double q = 1.0 / modulo;
static int seed;

void set_seed(int ii) {
    if (ii > 0)
        seed = ii;
    else
        seed = 31415;
}


int skip_immediate = 255;

double random_generate()
/*
    congruent multiplicative random number generator.
    Returns 0 < value <= 1
    Side effects on "seed".
    Uses "q" as a constant.
*/
{
    int h, l, la, s, hr;
    double u;

#ifdef DEBUGCALLS
    fprintf(stderr, "                Start of random_generate seed=%d\n", seed);
#endif
    if ((h = seed / mi)) {
        la = a * (l = seed - h * mi);
        hr = h * r;
        s = modulo - hr;
        if (la > s)
            u = (double)(seed = la - s);
        else
            u = (double)(seed = hr + la);
#ifdef DEBUGCALLS
        fprintf(stderr, "                End of random_generate <- %1.6g (seed=%d)\n",
                u * q, seed);
#endif
        return (u * q);
    }
    u = (double)(seed *= a);
#ifdef DEBUGCALLS
    fprintf(stderr, "                End of random_generate <- %1.6g (seed=%d)\n",
            u * q, seed);
#endif
    return (u * q);
}

#define negexp(mean) ( \
	  (double)(-log(random_generate())*((double)(mean))))

double cur_time;

int cur_priority;

static double last_checkpoint_time = -1.0;
static double cur_checkpoint_time = -1.0;
double max_checkpoint_time = -1000000.0;
static int noevents_to_checkpoint = HOLD_EVENTS;

static Event_p event_list_h = NULL, event_list_t = NULL;
static Event_p event_hold_h = NULL, event_hold_t = NULL;
static Event_p free_list = NULL;

static Past_state_p past_stack_top = NULL, past_stack_bot = NULL;
static Past_state_p past_free = NULL;

unsigned short *enabled_list;


Event_p create_event(int trans) {
    Event_p new_p, p_p;
    int ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "                Start of create_event trans=%d\n", trans);
#endif
    if ((new_p = free_list) == NULL) {
#ifdef DEBUG_events
        fprintf(stderr, "                  ... create_event allocated new bunch\n");
#endif
        free_list = new_p = (Event_p)ecalloc(16, sizeof(struct Event_descr));
        for (ii = 14 ; ii-- ;) {
            p_p = new_p + 1;
            new_p->e_next = p_p;
            new_p = p_p;
        }
        (new_p++)->e_next = NULL;
    }
    else {
#ifdef DEBUG_events
        fprintf(stderr, "                  ... create_event: free_list re-cycled\n");
#endif
        free_list = new_p->e_next;
    }
    new_p->trans = trans;
    new_p->e_next = new_p->e_prev =
                        new_p->t_next = new_p->t_prev =
                                            new_p->p_a_link = new_p->p_d_link =
                                                    new_p->p_d_where = new_p->p_t_where = NULL;
    new_p->creation_time = cur_time;
    new_p->last_sched_desched_time =
        new_p->residual_sched_time = -1.0;
#ifdef DEBUGCALLS
    fprintf(stderr, "                End of create_event %p\n", new_p);
#endif
    return (new_p);
}


void create_state() {
    Past_state_p new_p, p_p;
    int ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "              Start of create_state\n");
#endif
    if ((new_p = past_free) == NULL) {
        past_free = new_p = (Past_state_p)ecalloc(16,
                            sizeof(struct Past_state_descr));
        for (ii = 14 ; ii-- ;) {
            p_p = new_p + 1;
            new_p->prev = p_p;
            new_p = p_p;
        }
        (new_p++)->prev = NULL;
    }
    else {
        past_free = new_p->prev;
    }
    new_p->cur_time = cur_time;
    new_p->seed = seed;
    if ((new_p->prev = past_stack_top) == NULL)
        past_stack_bot = new_p;
    past_stack_top = new_p;
    new_p->sched_events = new_p->desched_events =
                              new_p->add_age_events = new_p->rem_age_events = NULL;
#ifdef DEBUGCALLS
    fprintf(stderr, "              End of create_state\n");
#endif
}


void out_event_list(FILE *fp) {
    Event_p ev_p = event_list_h;

#ifdef DEBUGCALLS
    /*
    fprintf(stderr,"              Start of out_event_list\n" );
    */
#endif
    while (ev_p != NULL) {
        fprintf(fp, "%d %1.6lg %1.6lg %1.6lg\n", ev_p->trans,
                ev_p->creation_time, ev_p->last_sched_desched_time,
                ev_p->residual_sched_time);
        ev_p = ev_p->e_next;
    }
    fprintf(fp, "0\n");
#ifdef DEBUGCALLS
    /*
    fprintf(stderr,"              End of out_event_list\n" );
    */
#endif
}


static char prev_fel[4096];
static char s_buf[GI_SIM_BUF_SIZE];

#ifdef DEBUG_socket
#define write_s_buf_in_(socket) \
    if ( write(socket,s_buf,GI_SIM_BUF_SIZE) == -1 ) { \
	fprintf(stderr,"Engine ERROR: couldn't write socket %d\n", errno ); \
	/* putchar('E'); */ STORE_COMPACT( CMD_E, stdout ); \
	exit(1); \
      } \
  fprintf(stderr,"... engine sending :%s\n",s_buf);
#else
#define write_s_buf_in_(socket) \
    if ( write(socket,s_buf,GI_SIM_BUF_SIZE) == -1 ) { \
	fprintf(stderr,"Engine ERROR: couldn't write socket %d\n", errno ); \
	/* putchar('E'); */ STORE_COMPACT( CMD_E, stdout ); \
	exit(1); \
      }
#endif


void out_filtered_event_list(FILE *fp,
                             int all,
                             int sock,
                             int anyway) {
    Event_p ev_p = event_list_h;
    int nt;
    char buf[4096];
    Trans_p t_p;
    int nnn = 0;

#ifdef DEBUGCALLS
    fprintf(stderr, "              Start of out_filtered_event_list\n");
#endif
    sprintf(buf, "l ");
    if (cur_priority) {
        if (skip_immediate <= cur_priority) {
#ifdef DEBUGCALLS
            fprintf(stderr,
                    "              End of out_filtered_event_list skip_immediate (%d)\n",
                    cur_priority);
#endif
            return;
        }
#ifdef DEBUG
        fprintf(stderr, "                ... cur_priority=%d\n", cur_priority);
#endif
        for (t_p = enab_list ;
                (t_p != NULL) && (t_p->pri == cur_priority) ;
                t_p = t_p->enabl_next) {
            if ((unsigned)(t_p->traced) || all) {
                sprintf(s_buf, "%d 0.0\n", t_p->tnum);
#ifdef DEBUG
                fprintf(stderr, "                  ... trans %s", s_buf);
#endif
                if (strlen(buf) + strlen(s_buf) < 4096)
                    strcat(buf, s_buf);
            }
        }
    }
    else {
#ifdef DEBUG
        fprintf(stderr, "                ... !cur_priority\n");
#endif
        for (t_p = enab_list ; (t_p != NULL) ; t_p = t_p->enabl_next) {
            if (!(t_p->nnn) &&
                    (t_p->nnn = ((unsigned)(t_p->traced) || all)))
                ++nnn;
        }
#ifdef DEBUG
        fprintf(stderr, "                  ... nnn=%d\n", nnn);
#endif
        while (ev_p != NULL) {
            nt = ev_p->trans;
            t_p = t_list + nt;
            if ((unsigned)(t_p->nnn)) {
#ifdef DEBUG
                fprintf(stderr, "                  ... trans %d sched %1.6g\n",
                        t_p->tnum, ev_p->residual_sched_time);
#endif
                --nnn;
                sprintf(s_buf, "%d %1.6lg\n", t_p->tnum,
                        ev_p->residual_sched_time);
#ifdef DEBUG
                fprintf(stderr, "                  ... trans %s", s_buf);
#endif
                if (strlen(buf) + strlen(s_buf) < 4096)
                    strcat(buf, s_buf);
                t_p->nnn = FALSE;
            }
            if (nnn)
                ev_p = ev_p->e_next;
            else
                ev_p = NULL;
        }
    }
    if (strlen(buf) < 4094)
        strcat(buf, "0\n");
    if (anyway || strncmp(buf, prev_fel, 4095)) {
        strcpy(prev_fel, buf);
        if (sock) {
            char *cp1 = buf, *cp2;
            while (*cp1 != '\0') {
                for (cp2 = s_buf ; *cp1 != '\n' && *cp1 != '\0' ;
                        * (cp2++) = *(cp1++));
                if (*cp1 == '\n')
                    ++cp1;
                *cp2 = '\0';
                write_s_buf_in_(sock)
            }
        }
        else
            fprintf(fp, buf);
#ifdef DEBUGCALLS
        fprintf(stderr, "              End of out_filtered_event_list !=\n");
#endif
        return;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "              End of out_filtered_event_list ==\n");
#endif
}


void out_cur_marking(FILE *fp) {
    unsigned char *uc_p = cur_mark;
    int nc = no_cod;

#ifdef DEBUGCALLS
    fprintf(stderr, "              Start of out_cur_marking\n");
#endif
    for (; nc-- ; ++uc_p)
        fprintf(fp, " %d", (unsigned)(*uc_p));
    fprintf(fp, "\n");
#ifdef DEBUGCALLS
    fprintf(stderr, "              End of out_cur_marking\n");
#endif
}


void pipe_marking() {
    unsigned char *uc_p = cur_mark;
    int nc = no_cod;

#ifdef DEBUGCALLS
    fprintf(stderr, "              Start of pipe_marking\n");
#endif
    for (; nc-- ; ++uc_p) {
#ifdef DEBUGpipe
        fprintf(stderr, "engine: piping marking: %d\n", (unsigned)(*uc_p));
#endif /* DEBUGpipe */
        putchar(*uc_p);
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "              End of pipe_marking\n");
#endif
}


void out_cur_simulation_state(FILE *fp) {
    int nt = trans_num;
    Trans_p t_p;
    int ni;
    Event_p ev_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "             Start of out_cur_simulation_state\n");
#endif
    fprintf(fp, "%1.6lg %1.6lg %d %d %d\n", cur_time, cur_checkpoint_time,
            cur_priority, seed, nt);
    out_cur_marking(fp);
    for (t_p = &(t_list[1]); nt; --nt, ++t_p) {
        fprintf(fp, "%d %d", t_p->tnum, t_p->enabled);
        for (ni = t_p->enabled, ev_p = t_p->instances_h ; ni-- ;
                ev_p = ev_p->t_next);
        while (ev_p != NULL) {
            fprintf(fp, " %1.6lg %1.6lg %1.6lg",
                    ev_p->creation_time,
                    ev_p->last_sched_desched_time,
                    ev_p->residual_sched_time);
            ev_p = ev_p->t_next;
        }
        fprintf(fp, " -1\n");
    }
    out_event_list(fp);
#ifdef DEBUGCALLS
    fprintf(stderr, "             End of out_cur_simulation_state\n");
#endif
}


void out_filtered_simulation_state(FILE *fp,
                                   int sockid,
                                   int anyway) {
    Place_p p_p;
    int offs;

#ifdef DEBUGCALLS
    fprintf(stderr, "             Start of out_filtered_simulation_state\n");
#endif
    if (skip_immediate <= cur_priority) {
#ifdef DEBUGCALLS
        fprintf(stderr,
                "              End of out_filtered_simulation_state skip_immediate (%d)\n",
                cur_priority);
#endif
        return;
    }
    sprintf(s_buf, "s %1.6lg %d", cur_time, cur_priority);

    if (decode_filter_mark(cur_mark) || anyway) {
#ifdef DEBUG
        fprintf(stderr, "               .. decode_filter_mark==TRUE\n");
#endif
        for (p_p = decoded_mark ; p_p != NULL ; p_p = p_p->mark_next) {
            offs = strlen(s_buf);
            sprintf((s_buf + offs), " %d %d\n", (int)(p_p - p_list), p_p->mark);
            if (sockid) {
                write_s_buf_in_(sockid)
            }
            else
                fprintf(fp, s_buf);
            s_buf[0] = '\0';
        }
        sprintf(s_buf, "0\n");
        if (sockid) {
            write_s_buf_in_(sockid)
        }
        else
            fprintf(fp, s_buf);
    }
#ifdef DEBUG
    else
        fprintf(stderr, "               .. decode_filter_mark==FALSE\n");
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "             End of out_filtered_simulation_state\n");
#endif
}


void out_filtered_event(FILE *fp,
                        int sockid,
                        int nt,
                        int anyway) {
    Trans_p t_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "             Start of out_filtered_event\n");
#endif
    if (nt <= 0)
        return;
    t_p = t_list + nt;
    if ((skip_immediate <= cur_priority) ||
            (!anyway && !(unsigned)(t_p->traced))) {
#ifdef DEBUGCALLS
        fprintf(stderr, "             End of out_filtered_event %d not traced\n",
                nt);
#endif
        return;
    }
    sprintf(s_buf, "t %d\n", t_p->tnum);
    if (sockid) {
        write_s_buf_in_(sockid)
    }
    else
        fprintf(fp, s_buf);
#ifdef DEBUGCALLS
    fprintf(stderr, "             End of out_filtered_event\n");
#endif
}


void schedule(Event_p ev_p) {
    Event_p pev_p, nev_p;
    Event_p pevh_p, nevh_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "            Start of schedule\n");
#endif
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
    ev_p->last_sched_desched_time = cur_time;
    if ((ev_p->residual_sched_time) >= 0.0)
        ev_p->residual_sched_time += cur_time;
    ev_p->p_a_link = past_stack_top->sched_events;
    past_stack_top->sched_events = ev_p;
    if ((pev_p = event_list_t) == NULL) {
        event_list_t = event_list_h = ev_p;
        ev_p->e_prev = ev_p->e_next = NULL;
#ifdef DEBUG_events
        out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
        fprintf(stderr, "            End of schedule 1 element\n");
#endif
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
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "            End of schedule many elements\n");
#endif
}


void modify_ev_list(int nt) {
    Trans_p t_p;
    Event_p ev_p, pev_p, nev_p;
    double ddd;

#ifdef DEBUGCALLS
    fprintf(stderr, "            Start of modify_ev_list nt=%d\n", nt);
#endif
    nt = t_list[nt].sort_index;
    t_p = t_list + nt;
    if (!(t_p->enabled) || (t_p->pri < cur_priority)) {
#ifdef DEBUGCALLS
        fprintf(stderr, "            End of modify_ev_list: NOT enabled!\n");
#endif
        return;
    }
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
    for (ev_p = event_list_h, pev_p = NULL, nev_p = ev_p->e_next ;
            ev_p != NULL && ev_p->trans != nt ;
            pev_p = ev_p, ev_p = nev_p, nev_p = nev_p->e_next);
    if (pev_p != NULL) {
        if ((pev_p->e_next = nev_p) == NULL)
            event_list_t = pev_p;
        else
            nev_p->e_prev = pev_p;
        ev_p->e_next = nev_p = event_list_h;
        ev_p->e_prev = NULL;
        event_list_h = ev_p;
        nev_p->e_prev = ev_p;
        if ((ddd = nev_p->residual_sched_time) > cur_time) {
            ddd += cur_time;
            ev_p->residual_sched_time = ddd * 0.5;
        }
        else
            ev_p->residual_sched_time = ddd;
    }
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "            End of modify_ev_list: done\n");
#endif
}


void deschedule(Trans_p t_p, unsigned noev) {
    unsigned dis_policy = (unsigned)(t_p->policy) & POLICY_DISABLE;
    Event_p ev_p = NULL, pev_p = NULL, nev_p = NULL;
    int ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "          Start of deschedule noev=%d\n", noev);
#endif
    if (! noev) {
#ifdef DEBUGCALLS
        fprintf(stderr, "          End of deschedule (nothing)\n");
#endif
        return;
    }
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
    if (noev > t_p->enabled)
        noev = t_p->enabled;
    while (noev) {
        if (noev == (int)(t_p->enabled)) {
            pev_p = NULL; ev_p = t_p->instances_h; nev_p = ev_p->t_next;
        }
        else switch (dis_policy) {
            case POLICY_DIS_RANDOM :
                ii = (int)(random_generate() * t_p->enabled + 1.0);
                if (ii > t_p->enabled)
                    ii = t_p->enabled;
#ifdef DEBUG
                fprintf(stderr,
                        "          ... deschedule: POLICY_DIS_RANDOM chose %d-th element (out of %d)\n",
                        ii, t_p->enabled);
#endif
                for (pev_p = NULL, nev_p = (ev_p = t_p->instances_h)->t_next ;
                        --ii ;
                        pev_p = ev_p, ev_p = nev_p, nev_p = nev_p->t_next) {
#ifdef DEBUG
                    fprintf(stderr, "            ... deschedule: scanning list ii=%d\n", ii);
#endif
                }
                break;
            case POLICY_DIS_FIRST_DRAWN :
                for (ii = t_p->enabled,
                        nev_p = (ev_p = t_p->instances_h)->t_next ;
                        ii-- ;
                        nev_p = nev_p->t_next)
                    if (nev_p->creation_time < ev_p->creation_time) {
                        ev_p = nev_p;
                    }
                nev_p = ev_p->t_next;
                pev_p = ev_p->t_prev;
                break;
            case POLICY_DIS_LAST_DRAWN :
                for (ii = t_p->enabled,
                        nev_p = (ev_p = t_p->instances_h)->t_next ;
                        ii-- ;
                        nev_p = nev_p->t_next)
                    if (nev_p->creation_time > ev_p->creation_time) {
                        ev_p = nev_p;
                    }
                nev_p = ev_p->t_next;
                pev_p = ev_p->t_prev;
                break;
            case POLICY_DIS_FIRST_SCHED :
                for (ii = t_p->enabled,
                        nev_p = (ev_p = t_p->instances_h)->t_next ;
                        ii-- ;
                        nev_p = nev_p->t_next)
                    if (nev_p->residual_sched_time <
                            ev_p->residual_sched_time) {
                        ev_p = nev_p;
                    }
                nev_p = ev_p->t_next;
                pev_p = ev_p->t_prev;
                break;
            case POLICY_DIS_LAST_SCHED :
                for (ii = t_p->enabled,
                        nev_p = (ev_p = t_p->instances_h)->t_next ;
                        ii-- ;
                        nev_p = nev_p->t_next)
                    if (nev_p->residual_sched_time >
                            ev_p->residual_sched_time) {
                        ev_p = nev_p;
                    }
                nev_p = ev_p->t_next;
                pev_p = ev_p->t_prev;
                break;
            }
#ifdef DEBUG
        fprintf(stderr,
                "          ... deschedule: end switch policy\n");
#endif
        ev_p->p_t_where = pev_p;
        if (pev_p == NULL)
            t_p->instances_h = nev_p;
        else
            pev_p->t_next = nev_p;
        if (nev_p == NULL)
            t_p->instances_t = pev_p;
        else
            nev_p->t_prev = pev_p;
        --(t_p->enabled);
        ev_p->p_d_where = ev_p->e_prev;
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
        ev_p->p_d_link = past_stack_top->desched_events;
        past_stack_top->desched_events = ev_p;
        if ((ev_p->e_next = event_hold_h) == NULL)
            event_hold_t = ev_p;
        else
            event_hold_h->e_prev = ev_p;
        event_hold_h = ev_p;
        if (((unsigned)(t_p->policy)&POLICY_AGE) &&
                (ev_p->residual_sched_time > cur_time)) {
            Event_p aev_p = create_event((int)(t_p - t_list));
            aev_p->creation_time = ev_p->creation_time;
            aev_p->last_sched_desched_time = cur_time;
            aev_p->residual_sched_time = ev_p->residual_sched_time - cur_time;
            /* aev_p->t_next = NULL; redundant */
            aev_p->t_prev = pev_p = t_p->instances_t;
            pev_p->t_next = aev_p;
            t_p->instances_t = aev_p;
            ++(t_p->held);
            aev_p->p_a_link = past_stack_top->add_age_events;
            past_stack_top->add_age_events = aev_p;
        }
        --noev;
    }
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "          End of deschedule (some)\n");
#endif
}


void sched_immediate(Trans_p t_p, double ddd) {
    Event_p ev_p;


#ifdef DEBUGCALLS
    fprintf(stderr,
            "          Start of sched_immediate trans=%d, ddd=%1.6lg, enabled=%d\n",
            (int)(t_p - t_list), ddd, (unsigned)(t_p->enabled));
#endif
    ev_p = create_event((int)(t_p - t_list));
    if ((ev_p->t_next = t_p->instances_h) == NULL) {
        t_p->instances_t = ev_p;
    }
    else {
        t_p->instances_h->t_prev = ev_p;
    }
    t_p->instances_h = ev_p;
    ++(t_p->enabled);
    ev_p->residual_sched_time = -ddd;
    schedule(ev_p);

#ifdef DEBUGCALLS
    fprintf(stderr, "          End of sched_immediate\n");
#endif
}


void sched_instances(Trans_p t_p, unsigned noev) {
    unsigned en_policy = (unsigned)(t_p->policy) & POLICY_RE_ENABLE;
    Event_p ev_p = NULL, pev_p = NULL, nev_p = NULL;
    int ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "          Start of sched_instances trans=%d, noev=%d\n",
            (int)(t_p - t_list), noev);
#endif
    while (noev) {
        if (((unsigned)(t_p->policy) & POLICY_AGE) &&
                ((unsigned)(t_p->held))) {
            Event_p aev_p;
            if (t_p->held > noev) {
                switch (en_policy) {
                case POLICY_RE_EN_RANDOM :
                    ii = (int)((double)(t_p->held) * random_generate());
                    for (nev_p = NULL, ev_p = t_p->instances_t,
                            pev_p = ev_p->t_prev; ii-- ;
                            nev_p = ev_p, ev_p = pev_p, pev_p = pev_p->t_prev);
                    break;
                case POLICY_RE_EN_FIRST_DRAWN :
                    for (ii = t_p->held, ev_p = t_p->instances_t,
                            pev_p = ev_p->t_prev; ii-- ;
                            pev_p = pev_p->t_prev)
                        if (pev_p->creation_time < ev_p->creation_time)
                            ev_p = pev_p;
                    pev_p = ev_p->t_prev;
                    nev_p = ev_p->t_next;
                    break;
                case POLICY_RE_EN_LAST_DRAWN :
                    for (ii = t_p->held, ev_p = t_p->instances_t,
                            pev_p = ev_p->t_prev; ii-- ;
                            pev_p = pev_p->t_prev)
                        if (pev_p->creation_time > ev_p->creation_time)
                            ev_p = pev_p;
                    pev_p = ev_p->t_prev;
                    nev_p = ev_p->t_next;
                    break;
                case POLICY_RE_EN_FIRST_DESCHED :
                    for (ii = t_p->held, ev_p = t_p->instances_t,
                            pev_p = ev_p->t_prev; ii-- ;
                            pev_p = pev_p->t_prev)
                        if (pev_p->last_sched_desched_time < ev_p->last_sched_desched_time)
                            ev_p = pev_p;
                    pev_p = ev_p->t_prev;
                    nev_p = ev_p->t_next;
                    break;
                case POLICY_RE_EN_LAST_DESCHED :
                    for (ii = t_p->held, ev_p = t_p->instances_t,
                            pev_p = ev_p->t_prev; ii-- ;
                            pev_p = pev_p->t_prev)
                        if (pev_p->last_sched_desched_time > ev_p->last_sched_desched_time)
                            ev_p = pev_p;
                    pev_p = ev_p->t_prev;
                    nev_p = ev_p->t_next;
                    break;
                }
            }
            else {
                nev_p = NULL;
                ev_p = t_p->instances_t;
                pev_p = ev_p->t_prev;
            }
            ev_p->p_t_where = pev_p;
            if (nev_p == NULL)
                t_p->instances_t = ev_p->t_prev;
            else
                nev_p->t_prev = ev_p->t_prev;
            pev_p->t_next = ev_p->t_next;
            if ((--(t_p->held)) == 0)
                t_p->instances_h = NULL;
            ev_p->p_d_link = past_stack_top->rem_age_events;
            past_stack_top->rem_age_events = ev_p;
            if ((nev_p = ev_p->e_next) == NULL)
                event_hold_t = (pev_p = ev_p->e_prev);
            else
                nev_p->e_prev = (pev_p = ev_p->e_prev);
            if (pev_p == NULL)
                event_hold_h = nev_p;
            else
                pev_p->e_next = nev_p;
            aev_p = create_event((int)(t_p - t_list));
            aev_p->creation_time = ev_p->creation_time;
            aev_p->residual_sched_time = ev_p->residual_sched_time;
            ev_p = aev_p;
        }
        else {
            unsigned ii, nos;
            double rr;
            ev_p = create_event((int)(t_p - t_list));
            switch ((unsigned)(t_p->timing) & TIMING_DISTRIBUTION) {
            case TIMING_DISCRETE :
                if ((nos = (unsigned)(t_p->timing) & TIMING_NOSTAGES)) {
                    rr = random_generate();
                    for (++nos, ii = 0 ;
                            nos-- && (t_p->f_time.f_list[ii] < rr) ;
                            rr += t_p->f_time.f_list[ii], ii += 2);
                    ev_p->residual_sched_time = t_p->f_time.f_list[ii + 1];
                }
                else { /* Deterministic */
                    ev_p->residual_sched_time = t_p->f_time.mean_t;
                }
                break;
            case TIMING_LINEAR :
                nos = (unsigned)(t_p->timing) & TIMING_NOSTAGES;
                rr = random_generate();
                for (++nos, ii = 0 ; --nos && (t_p->f_time.f_list[ii] < rr) ;
                        rr += t_p->f_time.f_list[ii], ii += 3);
                rr = t_p->f_time.f_list[++ii] * random_generate();
                ev_p->residual_sched_time = t_p->f_time.f_list[++ii] + rr;
                break;
            case TIMING_COX_normalized :
                for (nos = (unsigned)(t_p->timing)&TIMING_NOSTAGES, ++nos,
                        rr = 0.0, ii = 0 ; nos-- ; ii += 2) {
                    if (random_generate() <= t_p->f_time.f_list[ii])
                        rr += negexp(t_p->f_time.f_list[ii + 1]);
                }
                ev_p->residual_sched_time = rr;
                break;
            case TIMING_IPEREXP :
                nos = (unsigned)(t_p->timing) & TIMING_NOSTAGES;
                rr = random_generate();
                for (ii = 0 ; nos-- && (t_p->f_time.f_list[ii] < rr);
                        rr += t_p->f_time.f_list[ii], ii += 2);
                ev_p->residual_sched_time = negexp(t_p->f_time.f_list[ii + 1]);
                break;
            case TIMING_ERLANG : /* includes Exponential if "nos" = 0 */
#ifdef DEBUG
                fprintf(stderr, "           Exponential distrib. trans=%d, mean_t=%1.6g\n",
                        (int)(t_p - t_list), t_p->f_time.mean_t);
#endif
                nos = (unsigned)(t_p->timing) & TIMING_NOSTAGES;
                for (rr = negexp(t_p->f_time.mean_t) ; nos-- ;
                        rr += negexp(t_p->f_time.mean_t));
#ifdef DEBUG
                fprintf(stderr, "             firing time=%1.6g\n", rr);
#endif
                ev_p->residual_sched_time = rr;
                break;
            }
        }
        if ((ev_p->t_next = t_p->instances_h) == NULL) {
            t_p->instances_t = ev_p;
        }
        else {
            t_p->instances_h->t_prev = ev_p;
        }
        t_p->instances_h = ev_p;
        ++(t_p->enabled);
        if (record_traces) {
            int nt = (t_p->tnum) - 1;
            fprintf(traces_f_a[nt], "%g\t%g\t%g\n",
                    ev_p->residual_sched_time, cur_time,
                    ev_p->residual_sched_time + cur_time);
        }
        schedule(ev_p);
        --noev;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "          End of sched_instances\n");
#endif
}


static struct Event_descr end_simulation = {
    EVENT_END_SIM,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    0.0, 0.0, 0.0
};

static struct Event_descr backup_event = {
    EVENT_BACKUP,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    0.0, 0.0, 0.0
};

static struct Event_descr breakpoint_event = {
    EVENT_ASYNCHR_BRKP,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    0.0, 0.0, 0.0
};


Event_p select_immediate() {
    Event_p ev_p, nev_p, pev_p;
    int nt;
    Trans_p t_p;
    double sss;

#ifdef DEBUGCALLS
    fprintf(stderr, "         Start of select_immediate\n");
#endif
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
    ev_p = event_list_h; pev_p = NULL;
    if ((nev_p = ev_p->e_next) != NULL) {
        if ((nt = nev_p->trans) > 0 &&
                ((t_p = t_list + nt)->pri == cur_priority)) {
            if (record_traces)
                conflict_counts[cur_priority - 1]++;
            sss = t_p->f_time.mean_t + t_list[(nt = ev_p->trans)].f_time.mean_t;
            for (nev_p = nev_p->e_next ;
                    nev_p != NULL && (nt = nev_p->trans) > 0 &&
                    ((t_p = t_list + nt)->pri) == cur_priority ;
                    nev_p = nev_p->e_next) {
                sss += t_p->f_time.mean_t;
            }
            sss *= random_generate();
            for (ev_p = event_list_h, nev_p = ev_p->e_next; nev_p != NULL;) {
                nt = ev_p->trans;
                t_p = t_list + nt;
                if ((sss -= t_p->f_time.mean_t) > 0.0) {
                    pev_p = ev_p; ev_p = nev_p; nev_p = nev_p->e_next;
                }
                else {
                    if (record_traces) {
                        int nt = (t_p->tnum) - 1;
                        fprintf(traces_f_a[nt], "%d\t%g\n",
                                conflict_counts[cur_priority - 1], cur_time);
                    }
                    nev_p = NULL;
                }
            }
            nev_p = ev_p->e_next;
        }
    }
    create_state();
    ev_p->p_d_link = /* past_stack_top->desched_events = */ NULL;
    past_stack_top->desched_events = ev_p;
    ev_p->p_d_where = pev_p;
    ev_p->p_t_where = ev_p->t_prev;
    if (pev_p == NULL) {
        if ((event_list_h = nev_p) == NULL)
            event_list_t = NULL;
        else
            nev_p->e_prev = NULL;
    }
    else {
        if ((pev_p->e_next = nev_p) == NULL)
            event_list_t = pev_p;
        else
            nev_p->e_prev = pev_p;
    }
    if ((ev_p->e_next = event_hold_h) == NULL)
        event_hold_t = ev_p;
    else
        event_hold_h->e_prev = ev_p;
    event_hold_h = ev_p;
    nt = ev_p->trans;
    t_p = t_list + nt;
#ifdef DEBUG
    fprintf(stderr, "                 nt=%d, enabled =%d\n",
            nt, (unsigned)(t_p->enabled));
#endif
    --(t_p->enabled);
#ifdef DEBUG
    fprintf(stderr, "                 ... nt=%d, enabled =%d\n",
            nt, (unsigned)(t_p->enabled));
#endif
    if ((nev_p = ev_p->t_next) == NULL)
        t_p->instances_t = pev_p = ev_p->t_prev;
    else
        nev_p->t_prev = pev_p = ev_p->t_prev;
    if (pev_p == NULL)
        t_p->instances_h = nev_p;
    else
        pev_p->t_next = nev_p;
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "         End of select_immediate <- trans %d\n",
            (unsigned)(t_p->tnum));
#endif
    return (ev_p);
}


Event_p get_next_event() {
    Event_p ev_p;
    int nt;
    Trans_p t_p;

#ifdef DEBUGCALLS
#define DBGC
#else
#ifdef DEBUG_events
#define DBGC
#endif
#endif

#ifdef DBGC
    fprintf(stderr, "         Start of next_event\n");
#endif
    if (!(--noevents_to_checkpoint) || backup_flag) {
#ifdef DBGC
        fprintf(stderr, "         End of next_event <- backup\n");
#endif
        backup_event.residual_sched_time = cur_time;
        return (&backup_event);
    }
    if (break_point_flag) {
#ifdef DBGC
        fprintf(stderr, "         End of next_event <- breakpoint\n");
#endif
        breakpoint_event.residual_sched_time = cur_time;
        return (&breakpoint_event);
    }
    if (cur_priority) {
#ifdef DBGC
        fprintf(stderr, "         End of next_event <- select_immediate()\n");
#endif
        return (select_immediate());
    }
    if ((ev_p = event_list_h) == NULL) {
#ifdef DBGC
        fprintf(stderr, "         End of next_event <- end_simulation\n");
#endif
        end_simulation.residual_sched_time = cur_time;
        return (&end_simulation);
    }
#ifdef DEBUG_events
    fprintf(stderr, "           ... is a scheduled event\n");
#endif
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
    if ((event_list_h = ev_p->e_next) == NULL)
        event_list_t = NULL;
    else
        event_list_h->e_prev = NULL;
    if ((nt = ev_p->trans) > 0) {
        Event_p pev_p, nev_p;
        create_state();
        cur_time = ev_p->residual_sched_time;
        ev_p->p_d_link = past_stack_top->desched_events;
        past_stack_top->desched_events = ev_p;
        if ((ev_p->e_next = event_hold_h) == NULL)
            event_hold_t = ev_p;
        else
            event_hold_h->e_prev = ev_p;
        event_hold_h = ev_p;
#ifdef DEBUG_events
        fprintf(stderr, "             trans. %d firing at time %1.6g\n",
                ev_p->trans, ev_p->residual_sched_time);
#endif
        t_p = t_list + nt;
        --(t_p->enabled);
        ev_p->p_t_where = ev_p->t_prev;
        if ((nev_p = ev_p->t_next) == NULL)
            t_p->instances_t = pev_p = ev_p->t_prev;
        else
            nev_p->t_prev = pev_p = ev_p->t_prev;
        if (pev_p == NULL)
            t_p->instances_h = nev_p;
        else
            pev_p->t_next = nev_p;
    }
    else {
#ifdef DEBUG_events
        fprintf(stderr, "             control event #%d occurring at time %1.6g\n",
                ev_p->trans, ev_p->residual_sched_time);
#endif
        cur_time = ev_p->residual_sched_time;
        ev_p->e_next = free_list;
        free_list = ev_p;
    }
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
#ifdef DBGC
    fprintf(stderr, "         End of next_event <- ev_p\n");
#undef DBGC
#endif
    return (ev_p);
}


int undo_state_transition() { /* returns # of corresponding transition */
    /* to unfire */
    Event_p ev_p, nev_p, pev_p;
    Trans_p t_p;
    int nt;

#ifdef DEBUGCALLS
    fprintf(stderr, "       Start of undo_state_transition\n");
#endif
    if (past_stack_top->cur_time < 0.0) {
#ifdef DEBUGCALLS
        fprintf(stderr, "       End of undo_state_transition <- 0 (time 0.0 reached)\n");
#endif
        return (0);
    }
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
    cur_time = past_stack_top->cur_time;
    seed = past_stack_top->seed;
    for (ev_p = past_stack_top->add_age_events ; ev_p != NULL ;
            ev_p = ev_p->p_a_link) {  /* delete event */
        t_p = &(t_list[(nt = ev_p->trans)]);
        if ((nev_p = ev_p->t_next) == NULL)
            t_p->instances_t = (pev_p = ev_p->t_prev);
        else
            nev_p->t_prev = (pev_p = ev_p->t_prev);
        if (pev_p == NULL)
            t_p->instances_h = nev_p;
        else
            pev_p->t_next = nev_p;
        --(t_p->held);
        ev_p->e_next = free_list;
        free_list = ev_p;
    }
    for (ev_p = past_stack_top->rem_age_events ; ev_p != NULL ;
            ev_p = ev_p->p_d_link) {  /* re-add age memory */
        t_p = &(t_list[(nt = ev_p->trans)]);
        if ((ev_p->t_prev = pev_p = ev_p->p_t_where) == NULL) {
            if ((ev_p->t_next = nev_p = t_p->instances_h) == NULL)
                t_p->instances_t = ev_p;
            else
                nev_p->t_prev = ev_p;
            t_p->instances_h = ev_p;
        }
        else {
            if ((ev_p->t_next = nev_p = pev_p->t_next) == NULL)
                t_p->instances_t = ev_p;
            else
                nev_p->t_prev = ev_p;
            pev_p->t_next = ev_p;
        }
        ++(t_p->held);
        if ((nev_p = ev_p->e_next) == NULL)
            event_hold_t = (pev_p = ev_p->e_prev);
        else
            nev_p->e_prev = (pev_p = ev_p->e_prev);
        if (pev_p == NULL)
            event_hold_h = nev_p;
        else
            pev_p->e_next = nev_p;
    }
    for (ev_p = past_stack_top->sched_events ; ev_p != NULL ;
            ev_p = ev_p->p_a_link) {  /* delete event */
        t_p = &(t_list[(nt = ev_p->trans)]);
#ifdef DEBUG
        fprintf(stderr, "         ... descheduling nt=%d\n", nt);
#endif
        if ((nev_p = ev_p->e_next) == NULL)
            event_list_t = (pev_p = ev_p->e_prev);
        else
            nev_p->e_prev = (pev_p = ev_p->e_prev);
        if (pev_p == NULL)
            event_list_h = nev_p;
        else
            pev_p->e_next = nev_p;
        --(t_p->enabled);
        ev_p->e_next = free_list;
        free_list = ev_p;
    }
    for (ev_p = past_stack_top->desched_events ; ev_p != NULL ;
            ev_p = ev_p->p_d_link) {  /* schedule again */
        t_p = &(t_list[(nt = ev_p->trans)]);
#ifdef DEBUG
        fprintf(stderr, "         ... rescheduling nt=%d\n", nt);
#endif
        if ((nev_p = ev_p->e_next) == NULL)
            event_hold_t = (pev_p = ev_p->e_prev);
        else
            nev_p->e_prev = (pev_p = ev_p->e_prev);
        if (pev_p == NULL)
            event_hold_h = nev_p;
        else
            pev_p->e_next = nev_p;
        if ((ev_p->t_prev = pev_p = ev_p->p_t_where) == NULL) {
            if ((ev_p->t_next = nev_p = t_p->instances_h) == NULL)
                t_p->instances_t = ev_p;
            else
                nev_p->t_prev = ev_p;
            t_p->instances_h = ev_p;
        }
        else {
            if ((ev_p->t_next = nev_p = pev_p->t_next) == NULL)
                t_p->instances_t = ev_p;
            else
                nev_p->t_prev = ev_p;
            pev_p->t_next = ev_p;
        }
        ++(t_p->enabled);
        if ((ev_p->e_prev = pev_p = ev_p->p_d_where) == NULL) {
            if ((ev_p->e_next = nev_p = event_list_h) == NULL)
                event_list_t = ev_p;
            else
                nev_p->e_prev = ev_p;
            event_list_h = ev_p;
        }
        else {
            if ((ev_p->e_next = nev_p = pev_p->e_next) == NULL)
                event_list_t = ev_p;
            else
                nev_p->e_prev = ev_p;
            pev_p->e_next = ev_p;
        }
        if (ev_p->p_d_link == NULL) {
            Past_state_p cur_p;
            if ((cur_p = past_stack_top) == past_stack_bot) {
                cur_p->sched_events =
                    cur_p->desched_events =
                        cur_p->add_age_events =
                            cur_p->rem_age_events = NULL;
            }
            else {
                past_stack_top = cur_p->prev;
                cur_p->prev = past_free;
                past_free = cur_p;
            }
            enab_list = NULL;
            for (nt = trans_num, t_p = &(t_list[1]) ; nt-- ; ++t_p) {
                if ((unsigned)(t_p->enabled)) {
                    t_p->enabl_next = enab_list;
                    enab_list = t_p;
                }
            }
#ifdef DEBUG_events
            out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
            fprintf(stderr, "       End of undo_state_transition <- %d\n",
                    ev_p->trans);
#endif
            return ((nt = ev_p->trans));
        }
    }
    /*  ???????????????????????????????????
    enab_list = NULL;
    for ( nt = trans_num, t_p = &(t_list[1]) ; nt-- ; ++t_p ) {
    if ( (unsigned)(t_p->enabled) ) {
        t_p->enabl_next = enab_list;
        enab_list = t_p;
      }
      }
    */
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "       End of undo_state_transition <- 0 !!!\n");
#endif
    return (0);
}


static FILE *ckpf = NULL;
char cur_checkpoint_name[256];
char tmp_checkpoint_name[256];
char history_name[256];
char cat_checkpoint_history[1600];

void do_checkpoint() {

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of do_checkpoint\n");
#endif
    if (cur_time <= max_checkpoint_time)
        return;
    if ((ckpf = fopen(cur_checkpoint_name, "w")) == NULL) {
        fprintf(stderr, "\n Simulation Engine ERROR: couldn't open %s file\n",
                cur_checkpoint_name);
        return;
    }
    /* putchar('b'); */
    STORE_COMPACT(CMD_b, stdout);
    STORE_DOUBLE(&cur_time, stdout);
    out_cur_simulation_state(ckpf);
    (void)fclose(ckpf); ckpf = NULL;
    system(cat_checkpoint_history);
    max_checkpoint_time = last_checkpoint_time =
                              cur_checkpoint_time = cur_time;
    noevents_to_checkpoint = HOLD_EVENTS;
    backup_flag = FALSE;
    if (event_hold_t != NULL) {
#ifdef DEBUG
        fprintf(stderr, "             ... do_checkpoint: freeing event_hold=%p",
                event_hold_h);
#endif
        event_hold_t->e_next = free_list;
        free_list = event_hold_h;
        event_hold_h = event_hold_t = NULL;
    }
    if (past_stack_bot != NULL) {
        past_stack_bot->prev = past_free;
        past_free = past_stack_top;
        past_stack_top = past_stack_bot = NULL;
    }
    create_state();
    setup_alarm();
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of do_checkpoint\n");
#endif
}

int allowed_steps;

Event_p forward_sim(int trace) {
    Event_p ev_p;
    int ev_type, again_flag;

#ifdef DEBUGCALLS
    fprintf(stderr, "   Start of forward_sim trace=%d\n", trace);
#endif
    again_flag = FALSE;
    while (again_flag || allowed_steps--) {
        ev_p = get_next_event();
        if ((ev_type = ev_p->trans) > 0) {
            if (trace) {
                if (allowed_steps <= 0) {
#ifdef DEBUG_graphics
                    out_filtered_event(stderr, 0, ev_type, FALSE);
#else
                    out_filtered_event(stderr, out_sock_id, ev_type, FALSE);
#endif
                }
            }
            {
                long nn = t_list[ev_type].tnum;
                nn = (nn << 1);
                fire_trans(ev_type);
#ifdef DEBUG_events
                out_event_list(stderr);
#endif
                again_flag = (int)(cur_priority >= skip_immediate);
                if (! cur_priority)
                    nn |= 1;
                /* putchar('t'); */
                STORE_COMPACT(nn, stdout);
                fflush(stdout);
            }
            if (! cur_priority) {
                /* putchar('s');
                */
                STORE_DOUBLE(&cur_time, stdout);
                pipe_marking(); fflush(stdout);
            }
            if (trace) {
                if (allowed_steps <= 0) {
#ifdef DEBUG_graphics
                    out_filtered_simulation_state(stderr, 0, TRUE);
#else
                    out_filtered_simulation_state(stderr, out_sock_id, TRUE);
#endif
#ifdef DEBUG_graphics
                    out_filtered_event_list(stderr, FALSE, 0, TRUE);
#else
                    out_filtered_event_list(stderr, FALSE, out_sock_id, TRUE);
#endif
                }
            }
        }
        else {
#ifdef DEBUGCALLS
            fprintf(stderr,
                    "    cntrl event #%d, last_checkpoint_time=%1.6g, max_checkpoint_time=%1.6g\n",
                    ev_type, last_checkpoint_time, max_checkpoint_time);
#endif
            if (!(again_flag = (ev_type == EVENT_BACKUP))) {
                if (trace) {
#ifdef DEBUG_graphics
                    fprintf(stderr, "&\n");
#else
                    sprintf(s_buf, "&");
                    write_s_buf_in_(out_sock_id)
#endif
#ifdef DEBUG_graphics
                    out_filtered_simulation_state(stderr, 0, TRUE);
#else
                    out_filtered_simulation_state(stderr, out_sock_id, TRUE);
#endif
#ifdef DEBUG_graphics
                    out_filtered_event_list(stderr, FALSE, 0, TRUE);
#else
                    out_filtered_event_list(stderr, FALSE, out_sock_id, TRUE);
#endif
                }
            }
            if ((ev_type == EVENT_BACKUP) &&
                    (cur_checkpoint_time >= max_checkpoint_time)) {
                do_checkpoint();
            }
            if (ev_type <= EVENT_ASYNCHR_BRKP) {
#ifdef DEBUGCALLS
                fprintf(stderr, "   End of forward_sim\n");
#endif
                return (ev_p);
            }
        }
    }
    return NULL;
}


double search_simulation_state(double time, FILE *fp, double *max_p) {
    double ttt;
    int nt;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of search_simulation_state time=%1.6g\n", time);
#endif
    fscanf(fp, "%lg", &ttt);
    if (ttt > *max_p)
        *max_p = ttt;
    for (; ttt > time ; fscanf(fp, "%lg", &ttt)) {
        for (nt = trans_num + 2 ; nt-- ;)
            while (getc(fp) != '\n');
        for (fscanf(fp, "%d", &nt); nt ; fscanf(fp, "%d", &nt))
            while (getc(fp) != '\n');
        while (getc(fp) != '\n');
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of search_simulation_state <- %1.6g\n", ttt);
#endif
    return (ttt);
}

/* checkpoint file should be positioned by "search_simulation_state" */
void restore_checkpoint(double time,
                        FILE *fp) {
    Event_p ev_p;
    Trans_p t_p;
    int nt, ii;
    double dd;
    unsigned char *uc_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of restore_checkpoint time=%1.6g\n", time);
#endif
    /* putchar('c'); */
    STORE_COMPACT(CMD_c, stdout);
    STORE_DOUBLE(&time, stdout);
    fflush(stdout);
    if (past_stack_bot != NULL) {
        past_stack_bot->prev = past_free;
        past_free = past_stack_top;
    }
    past_stack_top = past_stack_bot = NULL;
    if (event_hold_t != NULL) {
#ifdef DEBUG
        fprintf(stderr, "             ... restore_checkpoint: freeing event_hold=%p",
                event_hold_h);
#endif
        event_hold_t->e_next = free_list;
        free_list = event_hold_h;
        event_hold_h = event_hold_t = NULL;
    }
    if (event_list_t != NULL) {
#ifdef DEBUG
        fprintf(stderr,
                "             ... restore_checkpoint: freeing scheduled instances (%p)",
                event_list_h);
#endif
        event_list_t->e_next = free_list;
        free_list = event_list_h;
        event_list_h = event_list_t = NULL;
    }
    for (nt = trans_num, t_p = &(t_list[1]) ; nt-- ; ++t_p) {
        t_p->instances_h = t_p->instances_t = NULL;
        t_p->enabled = 0; t_p->held = 0;
    }
    enab_list = NULL;
    cur_checkpoint_time = cur_time = time;
    fscanf(fp, "%lg %d %d %d\n", &last_checkpoint_time, &cur_priority,
           &seed, &nt);
    if (nt != trans_num) {
        fprintf(stderr, "Simulation Engine ERROR: inconsistency in history file!\n");
        /* putchar('E'); */
        STORE_COMPACT(CMD_E, stdout);
        fflush(stdout);
        if (out_sock_id) {
            sprintf(s_buf, "E");
            write_s_buf_in_(out_sock_id)
        }
        exit(1);
    }
    for (ii = no_cod, uc_p = cur_mark ; ii-- ; ++uc_p) {
        fscanf(fp, "%d", &nt); *uc_p = (unsigned)nt;
    }
    while (getc(fp) != '\n');
    for (nt = trans_num, t_p = &(t_list[1]) ; nt-- ; ++t_p) {
        fscanf(fp, "%*d %*d %lg", &dd);
        while (dd >= 0.0) {
#ifdef DEBUG
            fprintf(stderr, "       old instance trans.%d remaining=%1.6g\n",
                    (int)(t_p - t_list), dd);
#endif
            ev_p = create_event(trans_num - nt);
            ev_p->creation_time = dd;
            fscanf(fp, "%lg", &dd); ev_p->last_sched_desched_time = dd;
            fscanf(fp, "%lg", &dd); ev_p->residual_sched_time = dd;
            if ((ev_p->t_prev = t_p->instances_t) == NULL)
                t_p->instances_h = ev_p;
            else
                t_p->instances_t->t_next = ev_p;
            t_p->instances_t = ev_p;
            ++(t_p->held);
            fscanf(fp, "%lg", &dd);
        }
        while (getc(fp) != '\n');
    }
    for (fscanf(fp, "%d", &nt) ; nt ; fscanf(fp, "%d", &nt)) {
#ifdef DEBUG
        fprintf(stderr, "        current instance trans.%d\n", nt);
#endif
        ev_p = create_event(nt);
        fscanf(fp, "%lg", &dd); ev_p->creation_time = dd;
#ifdef DEBUG
        fprintf(stderr, "          creation_time=%lg", dd);
#endif
        fscanf(fp, "%lg", &dd); ev_p->last_sched_desched_time = dd;
#ifdef DEBUG
        fprintf(stderr, " last_sched_desched_time=%lg", dd);
#endif
        fscanf(fp, "%lg\n", &dd); ev_p->residual_sched_time = dd;
#ifdef DEBUG
        fprintf(stderr, " residual_sched_time=%lg\n", dd);
#endif
        if ((ev_p->e_prev = event_list_t) == NULL)
            event_list_h = ev_p;
        else
            event_list_t->e_next = ev_p;
        event_list_t = ev_p;
        ev_p->e_next = NULL;
        if (nt > 0) {
            t_p = t_list + nt;
            if ((ev_p->t_next = t_p->instances_h) == NULL)
                t_p->instances_t = ev_p;
            else
                t_p->instances_h->t_prev = ev_p;
            t_p->instances_h = ev_p;
            ev_p->t_prev = NULL;
            ++(t_p->enabled);
        }
    }
    while (getc(fp) != '\n');
    create_state();
    for (nt = trans_num, t_p = &(t_list[1]) ; nt-- ; ++t_p) {
        if ((unsigned)(t_p->enabled)) {
            t_p->enabl_next = enab_list;
            enab_list = t_p;
        }
    }
#ifdef DEBUG_events
    out_event_list(stderr);
#endif
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of restore_checkpoint\n");
#endif
}


void backward_sim(double target_time, int trace) {
    Event_p ev_p;
    int nt, again_flag;

#ifdef DEBUGCALLS
    fprintf(stderr, "   Start of backward_sim target_time=%1.6g, trace=%d\n",
            target_time, trace);
#endif
    nt = 1;
    again_flag = FALSE;
    while ((!break_point_flag) && (cur_time >= target_time) && nt &&
            (again_flag || allowed_steps--)) {
#ifdef DEBUG
        fprintf(stderr, "       ... doing one step\n");
#endif
        if (last_checkpoint_time >= 0.0 && cur_time <= cur_checkpoint_time) {
            double dd = cur_time - cur_checkpoint_time;
            Event_p pev_p;
#ifdef DEBUG
            fprintf(stderr, "         ... across checkpoint\n");
#endif
            if (ckpf == NULL) {
                if ((ckpf = fopen(history_name, "r")) == NULL) {
                    fprintf(stderr,
                            "\n Simulation Engine ERROR: couldn't open %s file\n",
                            history_name);
                    /* putchar('E'); */
                    STORE_COMPACT(CMD_E, stdout);
                    fflush(stdout);
                    if (out_sock_id) {
                        sprintf(s_buf, "E\n");
                        write_s_buf_in_(out_sock_id)
                    }
                    return;
                }
            }
#ifdef DEBUG
            fprintf(stderr, "         ... search previous checkpoint\n");
#endif
            cur_checkpoint_time =
                search_simulation_state(last_checkpoint_time, ckpf,
                                        &max_checkpoint_time);
#ifdef DEBUG
            fprintf(stderr, "         ... restore previous checkpoint\n");
#endif
            restore_checkpoint(cur_checkpoint_time, ckpf);
            ev_p = create_event(EVENT_SIMTIME_BRKP);
            ev_p->residual_sched_time = dd;
            dd += cur_time;
            for (pev_p = event_list_t ; pev_p->residual_sched_time > dd ;
                    pev_p = pev_p->e_prev);
            if ((ev_p->e_next = pev_p->e_next) == NULL)
                event_list_t = ev_p;
            else
                ev_p->e_next->e_prev = ev_p;
            ev_p->e_prev = pev_p;
            pev_p->e_next = ev_p;
#ifdef DEBUG_events
            out_event_list(stderr);
#endif
#ifdef DEBUG
            fprintf(stderr, "         ... forward simulation\n");
#endif
            if (allowed_steps >= 0) {
                int remember = allowed_steps;
                allowed_steps = -1;
                ev_p = forward_sim(FALSE);
                allowed_steps = remember;
            }
            else
                ev_p = forward_sim(FALSE);
#ifdef DEBUG
            fprintf(stderr, "         ... done\n");
#endif
        }
        if ((nt = undo_state_transition())) {
            if (trace) {
                if (allowed_steps <= 0) {
#ifdef DEBUG_graphics
                    out_filtered_event(stderr, 0, nt, FALSE);
#else
                    out_filtered_event(stderr, out_sock_id, nt, FALSE);
#endif
                }
            }
            {
                long nn = (t_list[nt].tnum) << 1;
                fflush(stdout);
                unfire_trans(nt);
                if (! cur_priority)
                    nn |= 1;
                /* putchar('T'); */
                STORE_COMPACT(-nn, stdout);
            }
            again_flag = (int)(cur_priority >= skip_immediate);
            if (! cur_priority) {
                /* putchar('s');
                */
                STORE_DOUBLE(&cur_time, stdout);
                pipe_marking(); fflush(stdout);
            }
            if (trace) {
                if (allowed_steps <= 0) {
#ifdef DEBUG_graphics
                    out_filtered_simulation_state(stderr, 0, TRUE);
#else
                    out_filtered_simulation_state(stderr, out_sock_id, TRUE);
#endif
#ifdef DEBUG_graphics
                    out_filtered_event_list(stderr, FALSE, 0, TRUE);
#else
                    out_filtered_event_list(stderr, FALSE, out_sock_id, TRUE);
#endif
                }
            }
        }
        else {
#ifdef DEBUG
            fprintf(stderr, "         ??? nt=0\n");
#endif
            if (trace) {
#ifdef DEBUG_graphics
                fprintf(stderr, "&\n");
#else
                sprintf(s_buf, "&");
                write_s_buf_in_(out_sock_id)
#endif
#ifdef DEBUG_graphics
                out_filtered_simulation_state(stderr, 0, TRUE);
#else
                out_filtered_simulation_state(stderr, out_sock_id, TRUE);
#endif
#ifdef DEBUG_graphics
                out_filtered_event_list(stderr, FALSE, 0, TRUE);
#else
                out_filtered_event_list(stderr, FALSE, out_sock_id, TRUE);
#endif
            }
        }
    }
    if (ckpf != NULL) {
        (void)fclose(ckpf); ckpf = NULL;
    }
    if (cur_time < target_time)
        cur_time = target_time;
#ifdef DEBUGCALLS
    fprintf(stderr, "   End of backward_sim\n");
#endif
}


void set_clock(double at_time) {
    Event_p ev_p, pev_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of set_clock at time %1.6g\n", at_time);
#endif
    if (((at_time >= 0.0 && (at_time <= cur_checkpoint_time)) ||
            ((at_time > cur_time && cur_checkpoint_time < max_checkpoint_time)))) {
        if ((ckpf = fopen(history_name, "r")) == NULL) {
            fprintf(stderr,
                    "\n Simulation Engine ERROR: couldn't open %s file\n",
                    history_name);
            /* putchar('E'); */
            STORE_COMPACT(CMD_E, stdout);
            fflush(stdout);
            if (out_sock_id) {
                sprintf(s_buf, "E\n");
                write_s_buf_in_(out_sock_id)
            }
#ifdef DEBUGCALLS
            fprintf(stderr, "  End of set_clock\n");
#endif
            return;
        }
        cur_checkpoint_time = search_simulation_state(at_time, ckpf,
                              &max_checkpoint_time);
        restore_checkpoint(cur_checkpoint_time, ckpf);
        (void)fclose(ckpf); ckpf = NULL;
    }
    else if (at_time < cur_time) {
        backward_sim(at_time, trace_flag);
#ifdef DEBUGCALLS
        fprintf(stderr, "  End of set_clock\n");
#endif
        return;
    }
    for (pev_p = event_list_t ;
            pev_p != NULL && pev_p->residual_sched_time > at_time ;
            pev_p = pev_p->e_prev);
    if (pev_p != NULL) {
#ifdef DEBUG_events
        fprintf(stderr, "    creating EVENT_SIMTIME_BRKP at time %1.6lg\n", at_time);
#endif
        ev_p = create_event(EVENT_SIMTIME_BRKP);
        ev_p->residual_sched_time = at_time;
        if ((ev_p->e_next = pev_p->e_next) == NULL)
            event_list_t = ev_p;
        else
            pev_p->e_next->e_prev = ev_p;
        ev_p->e_prev = pev_p;
        pev_p->e_next = ev_p;
#ifdef DEBUG_events
        out_event_list(stderr);
#endif
        ev_p = forward_sim(trace_flag);
    }
    else {
        cur_time = at_time;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of set_clock\n");
#endif
}


void purge_history(double at_time) {
    FILE *fp2;
    char ccc[600];
    char c;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of purge_history at time %1.6g\n", at_time);
#endif
    if ((ckpf = fopen(history_name, "r")) == NULL) {
        fprintf(stderr, "\n Simulation Engine ERROR: couldn't open %s file\n",
                history_name);
        return;
    }
    at_time = search_simulation_state(at_time, ckpf,
                                      &max_checkpoint_time);
    if (at_time < max_checkpoint_time) {
        if ((fp2 = fopen(tmp_checkpoint_name, "w")) == NULL) {
            fprintf(stderr, "\n Simulation Engine ERROR: couldn't open %s file\n",
                    tmp_checkpoint_name);
            (void)fclose(ckpf); ckpf = NULL;
            return;
        }
        fprintf(fp2, "%1.6lg ", at_time);
        for (c = getc(ckpf) ; c != EOF ; putc(c, fp2), c = getc(ckpf));
        (void)fclose(ckpf); ckpf = NULL;
        (void)fclose(fp2);
        sprintf(ccc, "/bin/mv %s %s", tmp_checkpoint_name, history_name);
        system(ccc);
        max_checkpoint_time = at_time;
    }
    /* putchar('p'); */
    STORE_COMPACT(CMD_p, stdout);
    STORE_DOUBLE(&at_time, stdout);
    fflush(stdout);
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of purge_history\n");
#endif
}


void init_events(int from_scratch) {
    FILE *fp;
    extern double target_time;


#ifdef DEBUGCALLS
    fprintf(stderr, " Start of init_events\n");
#endif
    cur_priority = 0;
    sprintf(history_name, "%s.etrace", netname);
    if ((fp = fopen(history_name, "r")) == NULL) {
        fp = fopen(history_name, "w");
        fprintf(fp, "-1\n");
        from_scratch = TRUE;
        cur_checkpoint_time = -1.0;
    }
    fclose(fp);
    sprintf(history_name, "%s_%s_%d.etrace", netname, hostname, my_pid);
    sprintf(cat_checkpoint_history, "/bin/cp %s.etrace %s",
            netname, history_name);
    system(cat_checkpoint_history);
    sprintf(cur_checkpoint_name, "%s_%s_%d.sckp", netname,
            hostname, my_pid);
    sprintf(tmp_checkpoint_name, "%s_%s_%d.stmp", netname,
            hostname, my_pid);
    sprintf(cat_checkpoint_history,
            "cat %s %s > %s; /bin/mv %s %s ; /bin/rm -f %s",
            cur_checkpoint_name, history_name, tmp_checkpoint_name,
            tmp_checkpoint_name, history_name,
            cur_checkpoint_name);
    if (!from_scratch) {
        if ((ckpf = fopen(history_name, "r")) == NULL) {
            fprintf(stderr,
                    "\n Simulation Engine ERROR: couldn't open %s file\n",
                    history_name);
            /* putchar('E'); */
            STORE_COMPACT(CMD_E, stdout);
            fflush(stdout);
            if (out_sock_id) {
                sprintf(s_buf, "E\n");
                write_s_buf_in_(out_sock_id)
            }
            return;
        }
        fprintf(stderr, "cur_checkpoint_time\n");
        cur_checkpoint_time =
            search_simulation_state(target_time, ckpf,
                                    &max_checkpoint_time);
        from_scratch = (max_checkpoint_time < 0.0);
        fclose(ckpf);
    }
    if (from_scratch) {
        fprintf(stderr, "create_state\n");
        create_state();

        fprintf(stderr, "initialize_en_list\n");
        initialize_en_list(cur_mark);

        fprintf(stderr, "do_checkpoint\n");
        do_checkpoint();

        fprintf(stderr, "clear_alarm\n");
        clear_alarm();
    }
    if ((ckpf = fopen(history_name, "r")) == NULL) {
        fprintf(stderr,
                "\n Simulation Engine ERROR: couldn't open %s file\n",
                history_name);
        /* putchar('E'); */
        STORE_COMPACT(CMD_E, stdout);
        fflush(stdout);
        if (out_sock_id) {
            sprintf(s_buf, "E");
            write_s_buf_in_(out_sock_id)
        }
        exit(1);
    }
    cur_checkpoint_time = search_simulation_state(cur_checkpoint_time, ckpf,
                          &max_checkpoint_time);
    restore_checkpoint(cur_checkpoint_time, ckpf);
    (void)fclose(ckpf); ckpf = NULL;
#ifdef DEBUGCALLS
    fprintf(stderr, " End of init_events\n");
#endif
}


void finish_events() {
    char copy_history[1024];

    sprintf(copy_history, "/bin/mv %s %s.etrace", history_name, netname);
    system(copy_history);
}

