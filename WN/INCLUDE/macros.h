# define GET_INHIBITOR_LIST(x) tabt[x].inibptr
# define GET_INPUT_LIST(x) tabt[x].inptr
# define GET_ENABLING_LIST(t,k) (k==INPUT)?tabt[t].inptr:tabt[t].inibptr
# define GET_FIRE_LIST(t,k) (k==MINUS_OP)?tabt[t].inptr:tabt[t].outptr
# define GET_OUTPUT_LIST(t) tabt[t].outptr

# define GET_TRANSITION_INDEX(x) x->trans
#ifdef SWN
# define GET_TRANSITION_COMPONENTS(x) tabt[x].comp_num
# define GET_TRANSITION_OUTPUT_TYPE(x) tabt[x].output_type
# define OFF(t,c) tabt[t].off[c]
# define GET_COLOR_REPETITIONS(c,t) tabt[t].rip[c]
# define GET_COLOR_IN_POSITION(p,t) tabt[t].dominio[p]
# define GET_POSITION_OF(c,r,t) tabt[t].off[c]+r
#ifdef SIMULATION
# define GET_TRANSITION_DEGREE(x) tabt[x].en_deg
#endif
#endif
# define GET_TRANSITION_PREDICATE(x) tabt[x].guard
# define GET_TRANSITION_SERVER(x) tabt[x].no_serv ? tabt[x].no_serv : MAX_INT
# define GET_TRANSITION_PRIORITY(x) tabt[x].pri


#ifdef SWN
# define GET_PLACE_COMPONENTS(x) tabp[x].comp_num
# define GET_PLACE_CARDINALITY(x) tabp[x].card
# define GET_COLOR_COMPONENT(i,p) tabp[p].dominio[i]
#endif
# define GET_PLACE_INDEX(x) x->place_no
# define GET_PLACE_FUNCTION(x) x->arcfun


#ifdef SWN
# define GET_CLASS_TYPE(c) tabc[c].type
# define GET_CLASS_CARDINALITY(c) tabc[c].card
# define GET_CLASS_NAME(c) tabc[c].col_name
# define GET_STATIC_SUBCLASS(c) tabc[c].sbc_num
# define GET_STATIC_NAME(c,s) tabc[c].sbclist[s].name
# define GET_STATIC_CARDINALITY(c,s) tabc[c].sbclist[s].card
# define GET_STATIC_OFFSET(c,s) tabc[c].sbclist[s].offset
# define GET_CARDINALITY_OF_CLASS(c,t) tabc[tabt[t].dominio[c]].card-1

# define GET_FUNCTION_CARDINALITY(x) x->fun_card
# define GET_LABELLING_FUNCTION_TYPE(x) x->type
#endif
# define GET_ARC_MOLTEPLICITY(x) x->molt
# define GET_NODE_MOLTEPLICITY(x) (x==NULL)?0:x->molt

#ifdef SWN
# define GET_SUBCLASS_COEFFICIENT(arc,cmp,sb) arc->coef[cmp].sbc_coef[sb]
# define GET_PROJECTION_COEFFICIENT(arc,cmp,rpt) arc->coef[cmp].xsucc_coef[3*rpt]
# define GET_SUCCESSOR_COEFFICIENT(arc,cmp,rpt) arc->coef[cmp].xsucc_coef[3*rpt+1]
# define GET_PREDECESSOR_COEFFICIENT(arc,cmp,rpt) arc->coef[cmp].xsucc_coef[3*rpt+2]
# define GET_TUPLE_COEFFICIENT(arc) arc->enn_coef
# define GET_TUPLE_PREDICATE(arc) arc->guard
#endif

# define GET_SUM_OPERATOR(x) (x==MINUS_OP)?minus_op:plus_op

# define GET_OLD_MOLTEPLICITY(x) (x==NULL)?0:x->molt
#ifdef SWN
# define GET_TOKEN_POSITION(x) x->pos
# define SET_TOKEN_MOLTEPLICITY(m,p) p->molt=m
#endif

# define NEXT_NODE(x) x->next
# define NEXT_TUPLE(x) x->next
# define NEXT_TOKEN(x) x->next
# define NEXT_EVENT(x) x->next
#ifdef REACHABILITY
# define NEXT_PATH(x) x->next
#endif

# define IS_FULL(p) net_mark[p].total
#ifdef SWN
# define IS_COLOURED(p) tabp[p].comp_num
# define IS_NEUTRAL(x) !tabp[x].comp_num
# define IS_INDEPENDENT(x) x->skip
# define IS_S_ONLY(x) (x->type==ONLY_S)?TRUE:FALSE
#endif
# define IS_EXPONENTIAL(x) (!tabt[x].pri && tabt[x].timing == TIMING_EXPONENTIAL)
# define IS_IMMEDIATE(x) tabt[x].pri
# define IS_INFINITE_SERVER(x) !tabt[x].no_serv
# define IS_RESET_TRANSITION(x) tabt[x].reset
#ifdef SIMULATION
#ifndef SYMBOLIC
# define IS_AGE_MEMORY(x) tabt[x].policy==AGE_M
#endif
#endif


# define IS_SOURCE_TRANSITION(t) (tabt[t].output_type==SOURCE_TRANSITION)?TRUE:FALSE
#ifdef SWN
# define IS_OUTPUT_TRANSITION(t) (tabt[t].output_type==PRESELECTION_TRANSITION)?TRUE:FALSE
# define HAS_PRESELECTABLE_OUTPUT(t) (tabt[t].output_type==PRESELECTION_TRANSITION)?TRUE:FALSE
#ifdef SIMULATION
# define IS_SKIPPABLE(x) tabt[x].skippable
# define IS_INPUT_PRESELECTABLE(x) tabt[x].input_preselectable
#endif
#endif
# define GET_INSTANCE_DEGREE(p) p->degree

#define THERE_ARE_OTHER_INSTANCES(tr) tabt[tr].instances_h != NULL
#define TRANS_NAME(x) tabt[x].trans_name
#define PLACE_NAME(x) tabp[x].place_name
#define IS_INVOLVED_IN_CC_SC(x) x->involved_in_CC_SC
#define IS_TEST_ARC(x) x->test_arc

#ifdef SWN
#ifdef SIMULATION
#define NEXT_TOUCHED(x) x->next_touched
#define NEXT_RESET(x) x->next_reset

#define TOUCH(x) net_mark[x].num_of_touched
#define RESET(x) net_mark[x].num_of_reset
#endif
#define DIFF_TOKENS(x) net_mark[x].different
#endif

# define MIN(a,b) (a <= b) ? a : b
# define MAX(a,b) (a >= b) ? a : b

# define GET_RANDOM_INTEGER(a,b) ((rand() % (int)(((b)+1) - (a))) + (a))

#ifdef SWN
#ifdef SYMBOLIC
/* retrieve di informazioni sulle cardinalita' e sul numero di sottoclassi */
#define GET_CARD(c,s,d) card[c][s][d - tabc[c].sbclist[s].offset]
#define GET_CARD_BY_TOTAL(c,s,d) card[c][s][d]
#define GET_CARD_BY_OFFSET(c,s,d) card[c][s][tabc[c].sbclist[s].offset + d]
#define GET_NUM_SS(c,s) num[c][s]
#define GET_NUM_CL(c) tot[c]

#define RESET_NUM_SS(c,s) num[c][s] = 0
#define RESET_NUM_CL(c) tot[c] = 0

#define SET_CARD(c,s,d,v) card[c][s][d] = v

/* operazioni per lo splitting nel caso di classi non ordinate */
#define INCREASE_NUM_SS(c,s) num[c][s]++
#define INCREASE_NUM_CL(c) tot[c]++

#define DECREASE_CARD(c,s,d,v) card[c][s][d]-=v

/* operazioni per il merging nel caso di classi non ordinate */
#define DECREASE_NUM_SS(c,s) num[c][s]--
#define DECREASE_NUM_CL(c) tot[c]--

#define INCREASE_CARD(c,s,ds,v) card[c][s][ds]+=v

/* informazioni identita' sottoclassi  */

#define GET_DYN_ID(ev,pos) ev->npla[pos]
#define GET_SPLIT_ID(ev,pos) ev->split[pos]
#define GET_NEW_ID(c,s) num[c][s]

#define GET_DYNAMIC_LIST(c,s) tabc[c].sbclist[s].dynlist
#define NEXT_DYNAMIC(p) p->next

#define IS_UNORDERED(c) tabc[c].type == UNORDERED
#define IS_ORDERED(c) tabc[c].type == ORDERED

#define ENCODE_ID(d,o) d*10000000+1000000+o
#define DECODE_ID(d) (d>=1000000)?(d-1000000)/10000000:d
#define DECODE_OBJECT(d) (d>=1000000)?(d-1000000)%10000000:0
#ifdef SIMULATION
#define GET_SPLIT_TYPE(t) tabt[t].pre_split;
#endif
#endif
#endif

#ifdef REACHABILITY
#define IS_VANISHING(pri) pri > 0
#define IS_TANGIBLE(pri) !pri
#define IS_DEAD(pri) pri < 0
#endif

#ifdef ESYMBOLIC
/****************** For ESRG ******************/
# define MY_GET_STATIC_SUBCLASS(tab,c)      tab[c].sbc_num
# define MY_GET_STATIC_CARDINALITY(tab,c,s) tab[c].sbclist[s].card
# define MY_GET_STATIC_OFFSET(tab,cl,ss)    tab[cl].sbclist[ss].offset
# define MY_GET_NUM_SS(num,cl,ss)           num[cl][ss]
# define GET_MIN(FIRST,SECEND)              (FIRST<=SECEND)?FIRST:SECEND
# define GET_ORG(merg)                      merg->sbc_num
# define IS_SYMETRIC(t)                     (tabt[t].trans_type==SYM_T)?1:0
# define IS_ASYMETRIC(t)                    (tabt[t].trans_type==ASYM_T)?1:0
/**********************************************/
#endif
