#ifndef __SERVICE_H_
#define __SERVICE_H_

// service.c
int get_static_subclass(int  cl,  int  dsc);
void initialize_free_event_list();
Event_p get_new_event(int  tr);
void dispose_old_event(Event_p  ptr);
int compare_input_event_id(Event_p  ev_1,  Event_p  ev_2,  int  tr);
Event_p next_instance(Event_p  pt);
void copy_event(Event_p  dst,  Event_p  src);
int get_object_from_dynamic(int  cl,  int  ds);
int get_dynamic_from_object(int  cl,  int  obj);
int get_dynamic_from_total(int  cl,  int  ds);
int get_static_subclass(int  cl,  int  dsc);
int optimized_nextvec(Cart_p  cartes_ptr,  int  comp,  int  *base);
int nextvec(Cart_p  cartes_ptr,  int  comp);
int nextvec_with_fix(Cart_p  cartes_ptr,  int  comp,  int  fix[MAX_DOMAIN]);
int get_obj_dist(int  ob1,  int  ob2,  int  cl);
int get_prec(int  cl,  int  ds);
int get_obj_off(int  sb,  int  cl);
double evaluate_rate_expression(Expr_p  code,  Event_p  ev_p);
// int evaluate_expression(Expr_p  code,  Event_p  ev_p);
int verify_predicate_constraint(Pred_p  code,  Event_p  ev_p);
double get_instance_rate(Event_p  ev_p);
void get_object_name(int  cl,  int  obj_num,  char  name[MAX_TAG_SIZE]);

// int verify_predicate_constraint(Pred_p  code,  Event_p  ev_p);
// int get_dynamic_from_object(int  cl,  int  obj);
// int get_object_from_dynamic(int  cl,  int  ds);
// int get_obj_off(int  sb,  int  cl);
// int get_prec(int  cl,  int  ds);
// int get_dynamic_from_total(int  cl,  int  ds);
// void dispose_old_event(Event_p  ptr);

// dimensio.c
int get_max_cardinality();

// graph_se.c
#ifdef REACHABILITY
void push_throu_element(Throughput_p  throu_ptr);
int GET_ENABLING_DEGREE(Event_p  ev_p);
void push_throu_list(Throughput_p  throu_head,  Throughput_p  throu_tail);
#endif

#ifdef SWN
#ifdef SYMBOLIC
void reset_to_M0(unsigned long mark_ptr, unsigned long dyn_ptr, unsigned long ilength, int tr);
#endif
#ifdef COLOURED
void reset_to_M0(unsigned long mark_ptr, unsigned long ilength, int tr);
#endif
#endif

// stack.c
#ifdef REACHABILITY
void insert_tree(Tree_p  *root,  int  *h,  unsigned long  mark_ptr,
                 unsigned long  ilength,  unsigned long  id_ptr);
void inqueue_stack(Tree_p  *top,  Tree_p  *bottom,  Tree_p  ins_node);
void pop(Tree_p  *top);
#endif

// rg_files.c
void code_marking();
void write_on_srg(FILE  *srg,  int  dir);

// swn_stndrd.c
#ifdef REACHABILITY
void push_rate_list(MRate_p rate_head, MRate_p rate_tail);
void push_rate_element(MRate_p  rate_ptr);
#endif

// rgengwn.c
void my_initialize_en_list();

// shared2.c
int pre_split_instance(Event_p  instance_ptr,  int  type);

// schemes.c
#ifdef ESYMBOLIC
#ifdef SWN
#ifdef SYMBOLIC
void fill_RIP(Cart_p  dyn,  Canonic_p  ptr,  Cart_p  token,  int  pl);
#endif
#endif
#endif

// common.c
int get_marking_of(Cart_p  cartes_ptr,  int  pl,  int  type);
int present(int  elem,  int  *addr,  int  ptr);

// token.c
#ifdef SWN
int optimized_compare_token_id(int  *tok1,  int  *tok2,  int  pl);
Token_p pop_token(int  pl);
void push_token(Token_p  tok_ptr,  int  pl);
void push_token_list(int  pl);
void optimized_init_token_values(int  *id,  Token_p  tok_ptr,  int  pl);
void insert_token(TokInfo_p  res_ptr,  Token_p  tok_ptr,  int  pl);
void delete_token(TokInfo_p  res_ptr,  int  pl);
#endif

//group.c
#ifdef SWN
void get_new_position_of_token(Token_p  tok_ptr,  int  pl,  TokInfo_p  ret_ptr);
void group(Event_p  instance_ptr);
#endif

// split.c
#ifdef SWN
void shift_dynamic_subclasses(int  cl,  int  split_pos,  int  pos,  int  dir);
void split(Event_p  instance_ptr);
#endif

// random.c
#ifdef SWN
int get_random_dynamic_subclass(int cl);
int get_random_integer(int min,  int max);
#endif

// report.c
void get_dynamic_subclass_name(int  cl,  int  dyn_num,  char  name[MAX_TAG_SIZE]);

// errors.c
void out_error(int  code,  int  pl,  int  tr,  int  cl,  int  sb,  char  *name1,  char  *name2);

#ifdef ESYMBOLIC
INT_LISTP *DYN_LIST_ALLOCATION();
void DYN_LIST_FREE(INT_LISTP to_free);
void *ARRAY_ALLOCATION(int NB);
int  EXCEPTION();
void ADD_EVENTUALITIE_TO_FILE(int size);
void MY_INIT_ALLOC();
int TEST_SYMETRIC_INITIAL(TO_MERGEP *merg);
void EVENTUALITIE_TO_STRING();
void GET_EVENTUALITIE_FROM_FILE(int pos_ptr, int size);
void PARM_EVENTUALITIE_TO_STRING(int sbc, int *tot_sbc,  MATRIX **EVENT);
void ADD_TEMP_EVENTUALITIE_TO_FILE(int size, FILE *fd);
#ifndef LIBSPOT
void EVENTUALITIES(int class, int sbc, int lin, int col, void (*traitement)());
#else
void EVENTUALITIES(int class, int sbc, int lin, int col, void (*traitement)(),
                   TO_MERGEP *merg, STORE_STATICS_ARRAY OLD_STATIC_CONF);
#endif
void GET_TEMP_EVENTUALITIE_FROM_FILE(int pos_ptr, int size, FILE *fd);
int Number_of_Instances(Result_p enb_h , Result_p enb_h_store, int pri, int greatest, int type);
void WRITE_HEAD_SR(FILE *fp, Tree_p ESM, Result_p enb_h_store, int great, int NB_INST);
void FreeMerg(TO_MERGEP *merg);
int NEW_COMPARE_EVENTUALITIES_TEMP(int ptr1, int length_ptr1, FILE *fd1,
                                   int ptr2, int length_ptr2, FILE *fd2);
int NEW_COMPARE_EVENTUALITIES(int ptr1, int length_ptr1,
                              int ptr2, int length_ptr2, FILE *fd);
void FreeStoreStructs(STORE_STATICS_ARRAY tab);
void PrintESM(FILE *fp, int nu);
void WRITE_INSTANCE(FILE *fp, Event_p ev_p);
void Print_Gen_Arc(FILE *fp, int tr, int source, int reach);
void WRITE_HEAD_EVENT(FILE *fp, Tree_p esm, Tree_Esrg_p event, int NB_INST);
void STRING_TO_EVENTUALITIE();
void PrintESMEVENT(FILE *fp, int nuev, int nuesm);
void PrintEvent(FILE *fp, int nuev, int nuesm);
void INIT_LIST_STATICS(struct MY_STATICS **head, struct COLORS *tabc, int **num, int Class);
void MY_CHAR_STORE(unsigned long nval);
void NEW_ASYM_MARKING(struct COLORS **tabc,
                      struct NET_MARKING **net_marking,
                      int ** **card, int *** num, int **tot,
                      DECOMP_TO *Decomp,
                      STORE_STATICS_ARRAY  STATIC_STORE,
                      int ** **EVENTS);
void END_EV(FILE *fp);
void COPY_CACHE(int size);
void STORE_SYM_TOT(int *tot, int *sym_tot);
void NEW_ASYM_TOT(int **tot, int **num, struct COLORS *tabc);
void SR_EVENT_POS_STORE(FILE *fp, Tree_p ESM, Tree_Esrg_p event);
void WRITE_REACHED_ESM(FILE *fp, Tree_p ESM, Tree_Esrg_p event);
void Print_Ins_Arc(FILE *fp, int tr, int source, int reach, int flg);
int GET_STATIC_FROM_LIST(int Class);
int GET_CARD_FROM_LIST(int sbc_num, int Class);
int GET_OFFSET_FROM_LIST(int sbc_num, int Class);
void ORDER(LIST_MY_STATICS head);
void INIT_DSC_SSBC(void);
void ALL_MAPING_AFTER_DECOMPOSING(DECOMP_TO *Decomp, struct COLORS *tabc, int **num, STORE_STATICS_ARRAY  STATIC_STORE,
                                  int ** **EVENTS , INT_LISTP *** MAPPE_ARRAY);
#endif

int guard_parser(FILE *f);

#endif
