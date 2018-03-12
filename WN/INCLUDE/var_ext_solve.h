/*******************************
  TABELLE VARIE
*******************************/
extern struct PLACES *tabp;
#ifdef SWN
extern struct COLORS *tabc;
#endif
extern struct GROUP *tabg;
extern struct TRANS *tabt;
extern struct MARKING *tabm;
extern struct RATE_PAR *tabrp;
extern struct MARK_PAR *tabmp;
extern struct TRANS_MD_RATE *tabmd;
/*******************************
  GRAMMAR INTERFACE
*******************************/
#ifdef SWN
extern struct FUNBUF **occ;
extern struct SBCBUF *sbc_pun, *buf_list;
extern struct MARKBUF *mbuf;
#endif
extern Coeff_p fun_ptr, ptr;
extern struct NODO *nodeptr, *node;
extern int ck, str_val, input_flag;
extern int parse_DEF;
extern int parse_MD;
extern double real_val;
extern char temp2[];
extern char str_in[];
extern char read_name[] ;
extern char net_name[] ;
extern char bufname[] ;
extern char error_name[] ;
extern int comp_check, pl_ind, cnt, sbc_def_num, tot_sbc_num, num_of_token;
/********************************************
  CONTATORI OGGETTI DELLE TABELLE
********************************************/
extern int nmp, npl, nrp, ncl, nmd, nmr, ngr;
extern int n_ord, n_unord;
extern int el[];

#ifdef LIBSPOT
extern int nSpottr;
#endif

/********************************************
             FILES DI SUPPORTO
********************************************/
/********************************************
	    STRUTTURA PER LA MARCATURA
********************************************/
extern Net_Mark_p net_mark;
/********************************************
	    CONTATORI DI EVENTI
********************************************/
#ifdef SIMULATION
#ifdef DEBUG_malloc
extern int tot_event;
extern int num_event;
extern int max_ev;
extern int min_ev;
#endif
#endif
/********************************************
	    STRUTTURE PER PRODOTTI CARTESIANI
********************************************/
extern Cart_p cart_place_ptr;
extern Cart_p cart_trans_ptr;
/********************************************
	    VARIABILI DI MASSIME DIMENSIONI
********************************************/
#ifdef SWN
extern int max_place_domain;
extern int max_transition_domain;
extern int max_cardinality;
extern int max_colour_repetitions;
extern int max_cl;
#endif
extern int *pred_val;
/********************************************
	    FLAG PER LA LINEA DI COMANDO
********************************************/
extern int output_flag;
extern int start;
/********************************************
	    STRUTTURE PER LE SOTTOCLASSI DINAMICHE
********************************************/
#ifdef SWN
#ifdef SYMBOLIC
extern int ** *card;
extern int **num;
extern int *tot;
#endif
#endif
/********************************************
	    EXTERN DEL GRAFO DI RAGGIUNGIBILITA'
********************************************/
#ifdef REACHABILITY
#ifdef SWN
#ifdef SYMBOLIC

extern double mark_ordinarie;
extern double ord_tang;
extern double ord_evan;
extern double ord_dead;


#endif
#endif
extern unsigned long d_ptr;
extern int dim_max;
extern int marcatura;
extern int tro;
extern int h;
extern unsigned long tang;
extern unsigned long evan;
extern unsigned long dead;
extern int home;
extern unsigned long cont_tang;
extern unsigned long length;

extern Tree_p initial_marking;
extern Tree_p current_marking;

#ifdef DEBUG_THROUGHPUT
extern FILE *thr;
extern FILE *vnum;
extern FILE *arc;
#endif

extern FILE *srg;
extern FILE *rgr_aux;
extern FILE *throu;
extern FILE *rht;
extern FILE *denom;
extern FILE *wngr;
extern FILE *mark;
extern FILE *van_path;
extern FILE *ctrs;
extern FILE *grg;
extern FILE *string;
#endif
extern int COMP_TEST;
/*********** Special traitement for esrg ******************/
extern  int SPEC_TRAI;
/************************************************/

#ifdef ESYMBOLIC

/*************** For ESRG ****************/
extern ARRAY                 *MAPPE_ARRAY;                          /* TO find the maping between the indexes **/
extern LIST_MY_STATICS       *STATICS_LIST_ARRAY;                  /* Used for finding the mapping      **/
extern INT_LISTP              DYN_LIST;
extern int                    MARKING_TYPE;
extern int                    TEST_SYM;
extern int                   ** *STORED_CARD;
extern STORE_STATICS_ARRAY    SYM_STATIC_STORE;
extern STORE_STATICS_ARRAY    ASYM_STATIC_STORE;
extern int                  **MERGING_MAPPE;
extern INT_LISTP            **DECOMP_MAPPE;
extern int                  **CODES_OF_EVENTS;
extern int                    MARKING_NUM;
extern int                    EVENTS_NUM;
extern FILE                  *ESRG;

extern int                   NO_SAT_INI;

/************* new version ***************/
extern FILE                  *EVENT_MARK;
extern FILE                  *OUTPUT_FILE;
extern FILE                  *OFFSET_SR;
extern FILE                  *OFFSET_EVENT;
extern FILE                  *STATISTICS;
extern char                   CACHE_STRING_ESRG[];
extern char                   compare[];
extern int                    LEGTH_OF_CACHE;
extern char                  *LP_ESRG;
extern int                    FILE_POS_PTR;
extern int                    MARK_INDEX;
extern int                    MAX_CARD;
extern MATRIX               **RESULT;
extern MATRIX               **TEMP;
extern MATRIX                *STATICS;
extern MATRIX                *DYNAMIC;
extern ARRAY                 *NB_DSC ;
extern ARRAY                 *NB_SSBC;
extern int                   *SYM_TOT;
extern int                    MARKINFO_POP_TOT;
extern int                    MARKINFO_MALL_TOT;
extern TO_MERGEP             *MERG;
extern Cart_p                 Counter;
extern Cart_p                 Res;

extern int                    COMPT_EVENT;
extern int                    COMPT_SAT;
extern int                    COMPT_STORED_EVENT;
extern int                    ALL_DYN;
extern int                    POP_DYN;
extern int                    FREE_DYN;
extern int                    TREENODEEVENT_MALL_TOT;
extern int                    TREENODEEVENT_POP_TOT;


extern char                   SESM_STRING[];
extern char                   RESM_STRING[];
extern char                   SEVENT_STRING[];
extern char                   REVENT_STRING[];
extern int                    NB_EV_IN_GRAPH;
/*****************************************/

/*****************************************/

#endif


#ifdef LIBSPOT

extern int *tab_event_prop;
extern int tab_event_prop_size;

#ifdef ESYMBOLIC
extern FILE *TEMP_EVENT ;
extern struct TRANS *tabt_sym ;
#endif

#endif
