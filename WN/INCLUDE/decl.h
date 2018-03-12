/*******************************
  TABELLE VARIE
*******************************/
struct PLACES *tabp = NULL;
struct TRANS *tabt = NULL;
struct GROUP *tabg = NULL;
struct MARKING *tabm = NULL;
struct RATE_PAR *tabrp = NULL;
struct MARK_PAR *tabmp = NULL;
struct TRANS_MD_RATE *tabmd = NULL;
#ifdef SWN
struct COLORS *tabc = NULL;
#endif
struct NESTED_UNIT *nu_root, **nu_array;
/*******************************
  GRAMMAR INTERFACE
*******************************/
#ifdef SWN
struct SBCBUF *sbc_pun, *buf_list ;
struct MARKBUF *mbuf;
struct FUNBUF **occ ;
#endif
Coeff_p fun_ptr;
Coeff_p ptr;
Node_p nodeptr;
Node_p node;
int comp_check, pl_ind, cnt, num_of_token;
int tot_sbc_num, sbc_def_num;
int str_val, input_flag;
int parse_DEF = UNKNOWN;
int parse_MD = UNKNOWN;
double real_val;
char read_name[MAX_TAG_SIZE];
char bufname[MAX_TAG_SIZE];
char str_in[MAX_TAG_SIZE];
char net_name[MAX_TAG_SIZE];
char temp2[MAX_TAG_SIZE];
/********************************************
  CONTATORI OGGETTI DELLE TABELLE
********************************************/
int nmp, npl, ntr, nrp, nmd, nmr, ngr;
int num_nested_units;
#ifdef LIBSPOT
int nSpottr;
#endif
#ifdef SWN
int ncl;
int n_ord = 0 , n_unord = 0;
#endif
int el[MAX_OBJ];
/********************************************
             FILES DI SUPPORTO
********************************************/
FILE *nfp2, *mfp;
/********************************************
	    STRUTTURA PER LA MARCATURA
********************************************/
Net_Mark_p net_mark = NULL;
/********************************************
	    CONTATORI DI EVENTI
********************************************/
#ifdef SIMULATION
#ifdef DEBUG_malloc
int max_ev = 0;
int min_ev = 0;
int tot_event = 0;
int num_event = 0;
#endif
#endif
/********************************************
	    STRUTTURE PER PRODOTTI CARTESIANI
********************************************/
Cart_p cart_place_ptr = NULL;
Cart_p cart_trans_ptr = NULL;
/********************************************
	    VARIABILI DI MASSIME DIMENSIONI
********************************************/
#ifdef SWN
int max_place_domain = 0;
int max_transition_domain = 0;
int max_cardinality = 0;
int max_colour_repetitions = 0;
int max_cl = 0;
#endif
int *pred_val = NULL;
/********************************************
	    FLAG PER LA LINEA DI COMANDO
********************************************/
int output_flag = FALSE;
int start = 0;
/********************************************
	    STRUTTURE PER LE SOTTOCLASSI DINAMICHE
********************************************/
#ifdef SWN
#ifdef SYMBOLIC
int ** *card = NULL;  /*cardinalita' sottoclassi dinamiche */
int **num = NULL;     /*numero sott. dinamiche per sott. statica */
int *tot = NULL;      /*numero sott. dinamiche per classe */
#endif
#endif
int COMP_TEST = 0;
/*********** Special traitement for esrg ******************/
int SPEC_TRAI = 0;
/************************************************/


#ifdef ESYMBOLIC
/***************for ESRG ******************/
ARRAY                  *MAPPE_ARRAY = NULL;                  /* TO find the maping between the indexes **/
LIST_MY_STATICS        *STATICS_LIST_ARRAY = NULL;            /* Used to find the mapping      **/
INT_LISTP               DYN_LIST = NULL;
int                     MARKING_TYPE;
int                     TEST_SYM = SYM_T;
int                    ** *STORED_CARD = NULL;
STORE_STATICS_ARRAY     SYM_STATIC_STORE = NULL;
STORE_STATICS_ARRAY     ASYM_STATIC_STORE = NULL;
int                   **MERGING_MAPPE = NULL;
INT_LISTP             **DECOMP_MAPPE = NULL;

int                     MARKING_NUM = 0;
int                     EVENTS_NUM = 0;
FILE                   *ESRG = NULL;

int                     NO_SAT_INI = FALSE;
/************* new version ***************/
FILE                   *EVENT_MARK = NULL;                  /* pointer to the file of eventualities */
FILE                   *OUTPUT_FILE = NULL;                /* pointer to ESRG output file */
FILE                   *OFFSET_SR = NULL;
FILE                   *OFFSET_EVENT = NULL;
FILE                   *STATISTICS = NULL;
char                    CACHE_STRING_ESRG[MAX_CACHE];       /* cache string for the compacte forme of the eventuality */
char                    compare[MAX_CACHE];
int                     LEGTH_OF_CACHE = 0;                 /* legth of the string form of the eventuality */
char                   *LP_ESRG = NULL;
int                     FILE_POS_PTR = 0;
int                     MARK_INDEX = 0;                     /* number of the symetric symbolic marking */
int                      MAX_CARD = 0;                      /* maximum cardinality               */

MATRIX                  **RESULT = NULL;                     /* the matrix representation of an eventuality */
MATRIX                  **TEMP = NULL;
ARRAY                   **STATICS = NULL;                     /* array of the static sub classes   */
ARRAY                   **DYNAMIC = NULL;                     /* array of the dynamic sub classes  */
ARRAY                    *NB_DSC = NULL;                      /* number of the dynamic sub classes */
ARRAY                    *NB_SSBC = NULL;                      /* number of the static sub classes  */
int                      *SBC_ARR = NULL;
int                      *SYM_TOT;

int                       MARKINFO_POP_TOT = 0;
int                       MARKINFO_MALL_TOT = 0;
TO_MERGEP                *MERG = NULL;
Cart_p                    Counter = NULL;
Cart_p                    Res = NULL;

int                       COMPT_EVENT = 0;
int                       COMPT_STORED_EVENT = -1;
int                       COMPT_SAT = 0;
int                       ALL_DYN = 0;
int                       POP_DYN = 0;
int                       FREE_DYN = 0;
int                       TREENODEEVENT_MALL_TOT = 0;
int                       TREENODEEVENT_POP_TOT = 0;

/****** For output ***********/

char                       SESM_STRING[MAXSTRING ];
char                       RESM_STRING[MAXSTRING ];
char                       SEVENT_STRING[MAXSTRING ];
char                       REVENT_STRING[MAXSTRING ];
int                        NB_EV_IN_GRAPH = 0;
/****************************/
int read_files = FALSE;
#endif

#ifdef LIBSPOT

int *tab_event_prop = NULL;
int tab_event_prop_size = 0;

#ifdef ESYMBOLIC
FILE *TEMP_EVENT = NULL ;
struct TRANS *tabt_sym = NULL;
int incl = NO;
int mcgroup = 0;
PART_MAT_P ref = NULL;
#endif

#endif
