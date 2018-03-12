//extern void out_error();
extern int nextvec();
extern int indix();
extern int markindix();
extern int get_static_subclass(int, int);
//extern void *ecalloc();
//extern char *emalloc();
//extern FILE *efopen();
extern int get_object();
extern void init_cartesian_product();
extern Event_p next_instance();
extern void initialize_event_fields();
extern void dispose_old_event(struct ENABLING *);
extern void copy_event(Event_p,  Event_p);
extern void fill();
extern void add_instance();
extern int extract_output();
extern int verify_neutre();
extern int verify_syncr();
extern int verify_pred();
extern int verify_mark();
extern int greater();
extern int lesser();
extern void plus();
extern void minus();
extern int b_plus();
extern int b_minus();
extern int get_token_offset();
extern Event_p get_new_event(int);

#ifdef REACHABILITY
extern Result_p pop_result();
extern MRate_p rate_pop();
extern Tree_p treenode_pop(unsigned long,  unsigned long,  unsigned long);
extern void push_result(Result_p);
extern void rate_push();
extern void treenode_push();
#endif

void out_error(int  code,  int  pl,  int  tr,  int  cl,  int  sb,  char  *name1,  char  *name2);
