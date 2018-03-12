#include "object.h"
#include "evaluate.h"
#include "defines.h"
#include "layer.h"
#include "option.h"

extern FILE *gTr_Ind_fp;
extern OptionType opt_list[];


struct trans_object *unfolding_transition(struct trans_object *t, int num, AssignmentPTR a, struct rpar_object **rpars);
struct place_object *create_place(struct place_object *place, char *tag);
list expand_arc(struct arc_object *arc, multisetPTR p_MS, set *p_S, struct net_object *net);

