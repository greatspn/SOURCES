#ifndef __MEASURE_CHECKPOINT_H_
#define __MEASURE_CHECKPOINT_H_

void do_state_transition(double tt);
void do_checkpoint();
void init_checkpoint();
void out_results();
void restore_checkpoint(double time);
void s_purge_history(double at_time);
void purge_history(double at_time);
void finish_checkpoint();

#endif
