#ifndef __GSPN_STRUCT_H__
#define __GSPN_STRUCT_H__

#include "const.h"

#ifdef ESYMBOLIC
#ifdef LIBSPOT
#include "SCONSSPOT.h"
#include "SSTRUCTSPOT.h"
#endif 
#endif
/***** TABELLA DEI GRUPPI DI PRIORITA' *****/
struct GROUP
  {
   char *name;
   int priority;
  } ;
/***** CODICE PER LA VALUTAZIONE DI UN'ESPRESSIONE *****/
struct EXPR_DEF
  {
   int op_type;
   double val ;
   int ival;
   struct EXPR_DEF * next;
  } ;
typedef struct EXPR_DEF * Expr_p;
/***** CODICE DI VALUTAZIONE DEI PREDICATI *****/
struct PRED
  {
#ifdef SWN
   char *fun_name;
   int occ ;
   int col_ind ;
#endif
#ifdef GSPN
   Expr_p expression;
#endif
   int type ;
   struct PRED *next;
  } ;
typedef struct PRED * Pred_p;
/***** BUFFER PER LA LETTURA DEL DOMINIO DI TRANSIZIONE *****/
struct FUNBUF
  {
   char fun_name[MAX_TAG_SIZE];
   int cclass; /* class is a reserved C++ keyword */
   int occ_num;
  } ;
#ifdef SWN
/***** COEFFICIENTI DI UN ELEMENTO DELL'ENNUPLA TIPO ,x+y, *****/
struct COEFF
  {
   int *sbc_coef;
   int *xsucc_coef;
  } ;
typedef struct COEFF * Func_p;
#endif
/***** ENNUPLA DI UNA FUNZIONE TIPO 3 [x=y] <x,y+!z> *****/
struct ENN_COEFF
  {
   Pred_p guard ;
#ifdef GSPN
   Expr_p expression;
#endif
   int enn_coef ;
#ifdef SWN
   Func_p coef ;
   int card;
#endif
   struct ENN_COEFF *next ;
  } ;
typedef struct ENN_COEFF * Coeff_p;
/***** NODO CHE CONNETTE UN POSTO AD UNA TRANSIZIONE *****/
struct NODO
  {
   Coeff_p arcfun ;
   int molt ;
   int fun_card ;
   int place_no ;
   int skip ;
   int analyzed;
   int type;
   int involved_in_CC_SC; 
   int test_arc;
   struct NODO * next;
  } ;
typedef struct NODO * Node_p;
#ifdef SWN
/***** DEFINIZIONE SOTTOCLASSI DINAMICHE *****/
struct DYNSC
 {
  char *name;
  int card;
  int map;
  struct DYNSC *next;
 } ;
typedef struct DYNSC * Dyn_p;
/***** NOMI OGGETTI *****/
struct OBJ
  {
   char *nome;
   struct OBJ * next;
  } ;
typedef struct OBJ * Obj_p;
/***** DEFINIZIONE SOTTOCLASSI STATICHE	*****/
struct STATICSBC
  {
   char *name;
   char *obj_name;
   Obj_p obj_list;
   Dyn_p dynlist;	/* Lista delle sottoclassi dinamiche */
   int low;
   int card ;
   int offset;
  } ;
typedef struct STATICSBC * Static_p;
/***** TABELLA DEI COLORI *****/
struct COLORS
  {
   char *col_name;
   Static_p sbclist ;
   int sbc_num ;
   int card ;	 
   int type ;	
  } ;
#endif
/***** BUFFER PER LA LETTURA DELLA RETE *****/
struct MARKBUF
 {
  char name[MAX_TAG_SIZE];
  char sbc[MAX_TAG_SIZE];
  int card;
  struct MARKBUF *next;
 } ;
#ifdef SWN
struct SBCBUF
  {
   char name[MAX_TAG_SIZE] ;
   char obj_name[MAX_TAG_SIZE];
   Obj_p obj_list;
   int card ;
   int low;
   struct SBCBUF *next ;
  } ;
#endif
/***** TABELLA DEI POSTI *****/
struct PLACES
  {
   char *place_name;
   int *dominio ;
   int comp_num ;
   int card;
   int position ;
   int tagged ;
  } ;
/***** DEFINIZIONE DI EVENTO *****/
struct ENABLING	{
    int trans;
#ifdef SIMULATION
    struct ENABLING * e_prev;
    struct ENABLING * e_next;
    struct ENABLING * t_prev;
    struct ENABLING * t_next;
    double creation_time;
    double residual_sched_time;
    double last_sched_desched_time;
#ifdef DEBUG_simulation
    int tag;
#endif
#endif
#ifdef SWN
#ifdef SYMBOLIC
    int * split;
#endif
    int * npla;
#ifdef SYMBOLIC
    int ordinary_instances;
#endif
#endif
    int enabling_degree;
    struct ENABLING * next;
};
typedef struct ENABLING * Event_p;
/***** INFORMAZIONI SULLA POSIZIONE IN UNA LISTA DI EVENTI *****/
struct EVENT_INFO
 {
  Event_p pos;
  Event_p prv;
 };
typedef struct EVENT_INFO * EventInfo_p;
/***** ELEMENTO DEL CASE RATE DIPENDNTI DAL COLORE *****/
struct MD_RATE
  {
   Pred_p guard;
   Expr_p code;
   struct MD_RATE *next ;
  } ;
typedef struct MD_RATE * MDrate_p;
/***** TABELLA DELLE TRANSIZIONI *****/
struct TRANS
  {
   char *trans_name;
#ifdef SWN
   char ** names;
#endif

   int in_arc ; 	   /* In net.lex */
   int out_arc ;	   /* In net.lex */
   int inib_arc ;	   /* In net.lex */

   int tagged;
   int reset;
#ifdef SWN
#ifdef SIMULATION
   int skippable;	   /* In fill_transition_data_structure (parser.c) */
   int input_preselectable;/* In fill_transition_data_structure (parser.c) */
   int en_deg;		   /* In fill_transition_data_structure (parser.c) */
#endif
#endif
   int output_type;	   /* In fill_transition_data_structure (parser.c) */
   int no_serv; 	   /* In net.lex */
   int pri ;		   /* In net.lex */
   int timing;		   /* In net.lex */
#ifdef SWN
   int comp_num ;	   /* In fill_transition_data_structure (parser.c) */
#endif

#ifdef SWN
   int * dominio;   /* In fill_transition_data_structure (parser.c) */
   int * comp_type; /* In other_domain net_lex.c */
   int * rip;	    /* In fill_transition_data_structure (parser.c) */
   int * off;	    /* In fill_transition_data_structure (parser.c) */
   int * split_type; /* In fill_transition_data_structure (parser.c) */
   int pre_split;
#endif
   int * add_l;
   int * test_l;
   int * me_l;

   Node_p inptr ;	    /* In net.lex */
   Node_p outptr ;	    /* In net.lex */
   Node_p inibptr ;	    /* In net.lex */

   Pred_p guard ;	    /* In net.lex */

   double mean_t ;	    /* In net.lex */

   MDrate_p md_rate_val;

#ifdef SIMULATION
#ifndef SYMBOLIC
   int policy;
   int deschedule_policy;
   int reuse_policy;
   double *dist;
   int stages;
   Event_p d_instances_h;
   Event_p d_instances_t;
#endif
   Event_p instances_h;
   Event_p instances_t;
   struct TRANS * enabl_next;
#endif

#ifdef ESYMBOLIC
  /****** For ESRG ****/
   int trans_type;
  /*******************/
#endif
  } ;
typedef struct TRANS * Trans_p;
/***** TABELLA DEI RATE PARAMETERS *****/
struct RATE_PAR
  {
   char *rate_name;
   double rate_val ;
  } ;
/***** TABELLA DEI MARKING PARAMETERS *****/
struct MARK_PAR
  {
   char *mark_name;
   int mark_val ;
  } ;
/***** TABELLA DEI RATE DIPENDENTI DAL COLORE *****/
struct TRANS_MD_RATE
  {
   int trans_ind;
   struct MD_RATE *mdpun;
  } ;
/****** TABELLA DELLE DEFINIZIONI DELLE MARCATURE INIZIALI *****/
struct MARKING
 {				 
  int position ;	
  int file_pos ;
 } ;			
struct RESULT
 {
  int hold;
  Event_p list;
  struct RESULT *next;
 } ;
typedef struct RESULT * Result_p;
struct CARTESIAN_PRODUCT
 {
  int *low;
  int *up;
  int **mark;
 } ;
typedef struct CARTESIAN_PRODUCT * Cart_p;

#ifdef SWN
struct COLOURED_TOKEN
 {
  int molt;
  int *id;
  struct COLOURED_TOKEN *next;
  struct COLOURED_TOKEN *prv;
  struct COLOURED_TOKEN *next_touched;
  struct COLOURED_TOKEN *next_reset;
#ifdef SYMBOLIC
  struct COLOURED_TOKEN *nxt_group;
  struct COLOURED_TOKEN *prv_group;
#endif
 };
typedef struct COLOURED_TOKEN * Token_p;
#endif

struct NET_MARKING
 {
#ifdef SWN
  Token_p marking;
  Token_p marking_t;
  int different;
#ifdef SYMBOLIC
  int ordinary;
  Token_p group;
#endif
#endif
#ifdef GSPN
  int mirror_total;
#endif
  int total;
#ifdef SWN
#ifdef SIMULATION
  Token_p touched_h;
  Token_p touched_t;
  Token_p reset_h;
  Token_p reset_t;
  int num_of_reset;
  int num_of_touched;
#endif
#endif
 };
typedef struct NET_MARKING * Net_Mark_p; 

#ifdef SWN
struct TOKEN_INFO
 {
  Token_p pos;
  Token_p prv;
 };
typedef struct TOKEN_INFO * TokInfo_p;
#endif

#ifdef REACHABILITY

struct PATH_COMP	   /* nodo utilizzato per il calcolo dei tassi */
 {
  unsigned long denominator;
  unsigned long fired_transition;
  unsigned long enabling_degree;
#ifdef SWN
#ifdef SYMBOLIC
  unsigned long ordinary_m;
#endif
#endif
  struct PATH_COMP *next;
 };
typedef struct PATH_COMP * PComp_p;

struct TRANSITION_WEIGHT    /* nodo utilizzato per il calcolo dei throughput */
 {
  int tr;
  double weight;
  PComp_p path;
  unsigned long cnt;
  struct TRANSITION_WEIGHT *next;
 };
typedef struct TRANSITION_WEIGHT * Throughput_p;

struct MARC_RAGG	   /* nodo utilizzato per il calcolo dei tassi */
 {
  int flag;
  unsigned long cont_tang;
  double mean_t;
  PComp_p path;
  unsigned long cnt;
  struct MARC_RAGG *next;
 };
typedef struct MARC_RAGG * MRate_p;

struct MARKING_INFO
 {
  unsigned long cont_tang;  /* num. d'ordine relativo marc. tangibili   */
  int pri;	      /* priorita' marcatura                      */
#ifdef DATOGLIERE
  char * marking_as_string;   /* marcatura sotto forma di stringa	  */
#endif
  unsigned long marking_as_string;
  unsigned long length;
#ifdef SWN
#ifdef SYMBOLIC
  unsigned long d_ptr;
#endif
#endif
  int path;
  int throu;
#ifdef DATOGLIERE
  MRate_p path_head;	      /* Cammini vanishing			  */
  MRate_p path_tail;	      /* Cammini vanishing			  */
#endif
#ifdef SWN
#ifdef SYMBOLIC
  int ordinary;	 /* marcature ordinarie corrispondenti	  */
#endif
#endif
 };
typedef struct MARKING_INFO * MInfo_p;

#ifdef ESYMBOLIC

/********************* for esrg **************/
struct TREE_ESRG
 {
  MInfo_p marking;               /* Informazioni sulla marcatura */
  Result_p enabled_head;	 /* Per lo stack */
  
  struct TREE_ESRG *  Head_Next_Event;  
  int   Marking_Type;   
  
 };
typedef struct TREE_ESRG * Tree_Esrg_p;

/********************************************/

#endif


struct TREE
 {
  MInfo_p marking;     /* Informazioni sulla marcatura */
  
  Result_p enabled_head;	/* Per lo stack */
  struct TREE *last;		/* Per lo stack */

  struct TREE *left;   /* Per l'albero di ricerca */
  struct TREE *right;  /* Per l'albero di ricerca */
  char balance;	       /* Per l'albero di ricerca */
 
#ifdef ESYMBOLIC
   /************** For ESRG **************/ 
   Tree_Esrg_p   Head_Next_Event;  
   int           Marking_Type;   
   int           Num_Event;
   Tree_Esrg_p   NOT_Treated; 
  /**************************************/
  #ifdef LIBSPOT
  TYPE_P** PM;
   TYPE_P NbElPM;
  
#endif
#endif

 };
typedef struct TREE * Tree_p;
 
#ifdef SWN
#ifdef SYMBOLIC
struct MIN_STR              /* nodo per realizzare la lista dei minimi */
     {                   /* nel calcolo della marcatura canonica    */
       int **min;
       int *ord;
       int *file_pos;
       int first;
       int free;
       int num_of_min;
       struct MIN_STR *next;
     };
typedef struct MIN_STR * Canonic_p;
#endif
#endif
#endif



#ifdef ESYMBOLIC
/*************** For ESRG **********************/

typedef int  * ARRAY;
typedef int ** MATRIX;

struct  STORE_STATICs
        {
          Static_p ptr;
          int sbc_num;
        };

typedef struct STORE_STATICs* STORE_STATICS_ARRAY; 
           
 
struct TO_MERGE  { 
       int * SSCs;                  /* Array of the static subclasses to merge **/ 
       int NB;
       int sbc_num;                /* Dimension of the array **/
       struct TO_MERGE * next ;
       struct TO_MERGE * prev ;
};

typedef struct TO_MERGE *     TO_MERGEP;
typedef struct TO_MERGE *     DECOMP_TO;
typedef TO_MERGEP *    ARRAY_MERG ;   

struct EVENT {
       int** event;
       struct EVENT* next;
};

typedef struct EVENT* EVENT_P;

struct LIST_EVENTS {
        TO_MERGEP* merging_p;
        EVENT_P   Head;
};     

typedef struct LIST_EVENTS* LIST_EVENTS_P;



struct INT_LIST {
       int dyn_sbc;
       struct INT_LIST *group;
       struct INT_LIST *next;
};

typedef struct INT_LIST* INT_LISTP;

struct MY_STATICS {
        int       sbc_num;
        int       card;
        int       offset;
        int       dyn_sbc;
        int       to_test;
        TO_MERGEP Org;   
        struct MY_STATICS* next;
        struct MY_STATICS* prev;
};

typedef struct MY_STATICS* LIST_MY_STATICS;
/***********************************************/
typedef struct MarkSym{
  char* marking_as_string;
  unsigned long length;
  unsigned long d_ptr;
  Tree_Esrg_p  Eventp;

}MarkSym;


#endif


#endif /* __GSPN_STRUCT_H__ */

