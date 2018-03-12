#ifndef _ENGINE_EVENT_H
#define _ENGINE_EVENT_H

#include "engine_decl.h"

void set_seed(int ii);
void out_filtered_event_list(FILE *fp,
                             int all,
                             int sock,
                             int anyway);
void out_filtered_simulation_state(FILE *fp,
                                   int sockid,
                                   int anyway);
void set_clock(double at_time);
void modify_ev_list(int nt);
void purge_history(double at_time);
void do_checkpoint();
Event_p forward_sim(int trace);
void backward_sim(double target_time, int trace);

void init_events(int from_scratch);
void finish_events();

void sched_instances(Trans_p t_p, unsigned noev);
void sched_immediate(Trans_p t_p, double ddd);
void deschedule(Trans_p t_p, unsigned noev);

extern int skip_immediate;

#endif
