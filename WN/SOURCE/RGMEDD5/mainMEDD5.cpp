#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h> // getpid
#include <fstream>

#include "rgmedd5.h"

extern "C" {

#include "WN/INCLUDE/const.h"
#include "WN/INCLUDE/struct.h"
#include "WN/INCLUDE/decl.h"
#include "WN/INCLUDE/macros.h"
#include "WN/INCLUDE/fun_ext.h"
#include "WN/INCLUDE/ealloc.h"

    extern int get_pl_in(char  pl_name[MAX_TAG_SIZE]);
#ifdef LIBSPOT
#include "../../INCLUDE/gspnlib.h"
    extern int prop_parser();
    extern int IS_WN_SYMETRIC();
#endif

#ifdef ESYMBOLIC
    extern void INIT_GLOBAL_VARS();
#endif

    /*extern void * calloc();*/
#ifdef DEBUG_malloc
#ifdef REACHABILITY
    extern void out_reach_info();
#ifdef SYMBOLIC
    extern void out_canonic_info();
#endif
#endif
    extern void out_token_info();
    extern void out_event_info();
#endif

#ifdef SYMBOLIC
    extern void initialize_split_data_structure();
    extern void initialize_merge_data_structure();
#endif

#ifdef SIMULATION
    extern void forward_sim();
    extern int init_events(); /* Controllare se si puo' usare nel reach */
    extern void fill_preselection_rate_field();
#endif
#ifdef REACHABILITY

    extern int init_events(); /* Controllare se si puo' usare nel reach */
    extern int belong(int,  Node_p);
#endif
    extern void lexer_set_buffer(const char *b);
    extern int parser();
    extern int get_class_card(int);
    extern int already_exist(char *, int, int);
    extern int intersection(Static_p, int, int);
    extern int read_DEF_file() ;
    extern int read_NET_file(int read_postproc);
    extern void add_marking_param_to_change(const char *name, int new_val);
    extern void add_rate_param_to_change(const char *name, double new_val);
    extern int get_max_cardinality();
    extern int get_max_place_domain();
    extern int get_max_transition_domain();
    extern int get_max_colour_repetitions();
    extern int get_max_elements(int);
    extern int get_max_cl();
    extern void initialize_free_token_list();
    extern void initialize_free_event_list();
    extern void out_error(int, int, int, int, int, char *, char *);
    extern int totm0;

//FILE *mfp ;
//FILE *nfp,*nfp2 ;
    FILE *pinfp;
    FILE *cfp, *sfp;
    FILE *disfp = NULL;
#ifdef REACHABILITY
#ifdef DEBUG_THROUGHPUT
    FILE *thr;
    FILE *vnum;
    FILE *arc;
#endif
    FILE *outtype;
    FILE *mark;
    FILE *rht;
    FILE *throu;
    FILE *srg;
    FILE *rgr_aux;
    FILE *wngr;
    FILE *wngrR;
    FILE *van_path;
    FILE *denom;
    FILE *ctrs;
    FILE *grg;
#ifdef SYMBOLIC
    FILE *f_cap_fp;
    FILE *value_fp;
    FILE *min_value_fp;
#endif
    int out_mc = FALSE;
    int exp_set = FALSE;
    int fast_solve = TRUE;
#endif
    /*SMART*/
    FILE *f_smart;
    bool SMART = false;
    double _prec = 0.01;
    int _iter = 10000;
    double _min = 0.0, _max = 10.0, _step = 1.0;
    /*SMART*/
    /*DOT
    *File uses to store the SRG in dot format
    */
    FILE *f_dot;
    /*DOT
    *File uses to store the SRG in dot format
    */

    /*DOT
    *Flag to set the store in the f_dot file
    */
    int dot_flag = 0;
    char bname[MAX_TAG_SIZE];
    /*DOT
    *Flag to set the store in the f_dot file
    */

    /*MDWN
    File is used to store the SRG
    */
#ifdef SWN
    FILE *f_MDWN;
#endif
    /*MDWN
    File is used to store the SRG
    */

    /*MDWN
    *Flag seting the store the SRG
    */
#ifdef SWN
    int MDWN_flag = 0;
#endif
    /*MDWN
    *Flag seting the store the SRG
    */

//MDWN
//*File uses to compute the places reward
#ifdef SWN
    FILE *f_cvrs, *f_cvrsoff;
    int cvrs_flag = 0;
#endif
//MDWN


} // extern "C"

void print_banner(const char* title);
/*MDD*/
extern bool build_graph(RSRG &);

bool print_CTL_counterexamples = false;
bool sort_CTL_queries = false;
bool CTL_atoms_use_potential_state_space = false;
bool g_dot_RS = false;
bool g_dot_open_RS = false;
const char* g_dot_file = nullptr;

bool CTL = false;
bool eval_CTL_using_SatELTL = false; // model check CTL operators as CTL* operators
bool CTL_quiet = false;
bool CTL_print_intermediate_sat_sets = false;
bool CTL_print_sat_sets = false;
bool implicitNextForCTLstar = false;
bool CTL_preimg_stays_in_RS = false;
bool LTL_implicit_RSxBA = false;
// bool LTL_weak_next = false;
bool CTMC = false;
bool use_m0_min = false;
bool g_count_firings = true;
bool g_show_var_order = false;
bool g_save_var_order_for_ltsmin = false;
bool g_save_var_order_for_rgmedd = false;
var_order_selector g_var_order_sel;
// bool g_save_incidence_as_image = false;
bool g_save_incidence_as_eps = false;
bool g_open_saved_incidence = false;
const char* g_incidence_file = nullptr;
bool g_exit_after_varorder = false;
bool g_print_varorder_metrics = false;
bool g_print_varorder_metaheuristic_scores = false;
bool g_print_pbasis_metrics = false;

bool g_save_ext_incidence = false;
bool g_open_saved_ext_incidence = false;


unsigned long long g_random_order_seeds[2] = { 0, 0 };
extern size_t g_sim_ann_num_tentatives;
int g_sloan_W1 = 1;
int g_sloan_W2 = 2;
extern MEDDLY::forest::policies::node_deletion mdd_node_del_policy;
extern MEDDLY::forest::policies::node_deletion mxd_node_del_policy;

// The technique used to compute the variable order of the decision diagrams
const char* g_given_varorder = nullptr;

int ctl_name = -1;
static long meddly_cache_size = -1;

static RsMethod initRsMethod = RSM_SAT_PREGEN;
static LrsMethod initLrsMethod = LRSM_NONE;
static BoundGuessPolicy bound_policy = BoundGuessPolicy::LOAD_FROM_FILE;
static bool fast_NSF_gen = false;
static bool print_guessed_bounds = false;
bool test_LRS_RS_equiv = false;
int g_num_extra_levels = 1;
bool ilcp_model = false;
void ilcp_add_slack_variables_to_model();
size_t ilcp_num_lin_dep_constr = 0;
void ilcp_add_lin_dep_constr(size_t num_constrs);

double GP_WEIGHT = 1.0;
double GP_EXP = 0.0;
double GP_GRAD = 1.0;
double GP_WEIGHT2 = 0.0;
bool GP_SIGN = true;

/*AUTOMA*/
bool AUTOMA = false;
int AState = 25;
// extern void build_graphAutoma(RSRGAuto &);
FILE *f_outState;
/*AUTOMA*/
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_tr_in(char  tr_name[MAX_TAG_SIZE]) {
    /* Init get_tr_in */
    int i, ret = UNKNOWN;

    for (i = 0 ; i < ntr ; i++)
        if (!strcmp(tr_name, tabt[i].trans_name)) {
            ret = i;
            break;
        }
    return (ret);
}/* End get_tr_in */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
// void create_statistics_data_structure(int  ok) {
//     /* Init create_statistics_data_structure */
//     int ind;
//     char name[MAX_TAG_SIZE];

//     if (ok) {
//         /* Il file .stat e' presente */
// #ifdef GREATSPN
//         for (ind = 0; ind < npl; ind++)
//             tabp[ind].tagged = FALSE;
//         for (ind = 0; ind < ntr; ind++)
//             tabt[ind].tagged = FALSE;
// #endif
//         while (TRUE) {
//             fscanf(pinfp, "%s", name);
//             if (isalpha(name[0])) {
//                 /* Non e' una lettera dell'alfabeto */
//                 ind = get_tr_in(name);
//                 if (ind == UNKNOWN) {
//                     ind = get_pl_in(name);
//                     if (ind == UNKNOWN)
//                         fprintf(stdout, "Warning: unknown name %s in file .stat\n", name);
//                     else
//                         tabp[ind].tagged = TRUE;
//                 }
//                 else
//                     tabt[ind].tagged = TRUE;
//             }/* Non e' una lettera dell'alfabeto */
//             getc(pinfp);
//             if (feof(pinfp))
//                 break;
//         }
//     }/* Il file .stat e' presente */
// }/* End create_statistics_data_structure */
#ifdef SIMULATION
#ifndef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : FORMATO FILE: nome_t policy reuse_p deschedule_p distr_type lista_par */
/*                                                            */
/*  nome AGE RANDOM FIRST_DRAWN DET  (in questo caso il parameter e' nel .net e puo' essere inst-dep)    */
/*  nome AGE LAST_DRAWN FIRST_SCHED ERL K-STAGES (in questo caso il parameter e' nel .net e puo' essere inst-dep)    */
/*  nome ENABLING RANDOM LAST_SCHED IPO K-STAGES M1 M2 .... MK  */
/*  nome ENABLING RANDOM FIRST_DRAWN IPER K-STAGES p1 M1 p2 M2 ..... pk Mk  */
/*  nome ENABLING RANDOM RANDOM UNIF lower upper  */
/*                                                            */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
#if 0
void read_transition_general_distributions(int  ok) {
    /* Init read_transition_general_distributions */
    int ind, ii;
    int n_stages;
    int char_read;
    char *str_p = NULL;
    char name[MAX_TAG_SIZE];
    char policy[MAX_TAG_SIZE];
    char reuse_policy[MAX_TAG_SIZE];
    char deschedule_policy[MAX_TAG_SIZE];
    char type[MAX_TAG_SIZE];
    char tmp[MAXSTRING];

    if (ok) {
        /* Il file .dis e' presente */
        while (TRUE) {
ndef: if (fgets(tmp, MAXSTRING - 1, disfp) == NULL)
                if (feof(disfp))
                    break;
            sscanf(tmp, "%s %s %s %s %s", name, policy, reuse_policy, deschedule_policy, type);
            ind = get_tr_in(name);
            if (ind == UNKNOWN) {
                fprintf(stdout, "Warning: unknown transition name %s in file .dis\n", name);
                fprintf(stdout, "firing distribution definition ignored\n");
                goto ndef;
            }
            if (strcasecmp(policy, "AGE") && strcasecmp(policy, "ENABLING")  && strcasecmp(policy, "RESAMPLING")) {
                fprintf(stdout, "Warning: unknown firing policy (%s) for transition %s in file .dis\n", policy, name);
                fprintf(stdout, "firing distribution definition ignored\n");
                goto ndef;
            }
            if (strcasecmp(type, "DET") && strcasecmp(type, "ERL")  && strcasecmp(type, "UNIF") && strcasecmp(type, "IPO") && strcasecmp(type, "IPER")) {
                fprintf(stdout, "Warning: unknown firing distribution (%s) for transition %s in file .dis\n", type, name);
                fprintf(stdout, "firing distribution definition ignored\n");
                goto ndef;
            }

            if (strcasecmp(reuse_policy, "RANDOM") && strcasecmp(reuse_policy, "FIRST_DRAWN")  && strcasecmp(reuse_policy, "LAST_DRAWN") && strcasecmp(reuse_policy, "FIRST_SCHED")  && strcasecmp(reuse_policy, "LAST_SCHED")) {
                fprintf(stdout, "Warning: unknown reuse policy (%s) for transition %s in file .dis\n", reuse_policy, name);
                fprintf(stdout, "firing distribution definition ignored\n");
                goto ndef;
            }

            if (strcasecmp(deschedule_policy, "RANDOM") && strcasecmp(deschedule_policy, "FIRST_DRAWN")  && strcasecmp(deschedule_policy, "LAST_DRAWN") && strcasecmp(deschedule_policy, "FIRST_SCHED")  && strcasecmp(deschedule_policy, "LAST_SCHED")) {
                fprintf(stdout, "Warning: unknown deschedule policy (%s) for transition %s in file .dis\n", deschedule_policy, name);
                fprintf(stdout, "firing distribution definition ignored\n");
                goto ndef;
            }

#ifdef SWN
            if (tabt[ind].input_preselectable)
                tabt[ind].input_preselectable = FALSE;
            if (tabt[ind].skippable)
                tabt[ind].skippable = FALSE;
#endif

            /**** Firing policy ****/
            if (!strcasecmp(policy, "AGE")) {
                /* Transizione con age memory */
                tabt[ind].policy = AGE_M;
            }/* Transizione con age memory */
            else if (!strcasecmp(policy, "ENABLING")) {
                /* Transizione con enabling memory */
                tabt[ind].policy = ENABLING_M;
            }/* Transizione con enabling memory */
            else if (!strcasecmp(policy, "RESAMPLING")) {
                /* Transizione con resampling memory */
                tabt[ind].policy = RESAMPLING_M;
            }/* Transizione con resampling memory */

            /**** Reuse policy ****/
            if (!strcasecmp(reuse_policy, "RANDOM")) {
                /* Transizione con random selection policy */
                tabt[ind].reuse_policy = POLICY_DIS_RANDOM;
            }/* Transizione con random selection policy */
            else if (!strcasecmp(reuse_policy, "FIRST_DRAWN")) {
                /* Transizione con first drawn policy */
                tabt[ind].reuse_policy = POLICY_DIS_FIRST_DRAWN;
            }/* Transizione con first drawn policy */
            else if (!strcasecmp(reuse_policy, "LAST_DRAWN")) {
                /* Transizione con last drawn policy */
                tabt[ind].reuse_policy = POLICY_DIS_LAST_DRAWN;
            }/* Transizione con last drawn policy */
            else if (!strcasecmp(reuse_policy, "FIRST_SCHED")) {
                /* Transizione con first sched policy */
                tabt[ind].reuse_policy = POLICY_DIS_FIRST_SCHED;
            }/* Transizione con first sched policy */
            else if (!strcasecmp(reuse_policy, "LAST_SCHED")) {
                /* Transizione con last sched policy */
                tabt[ind].reuse_policy = POLICY_DIS_LAST_SCHED;
            }/* Transizione con last sched policy */

            /**** Deschedule policy ****/
            if (!strcasecmp(deschedule_policy, "RANDOM")) {
                /* Transizione con random selection policy */
                tabt[ind].deschedule_policy = POLICY_DIS_RANDOM;
            }/* Transizione con random selection policy */
            else if (!strcasecmp(deschedule_policy, "FIRST_DRAWN")) {
                /* Transizione con first drawn policy */
                tabt[ind].deschedule_policy = POLICY_DIS_FIRST_DRAWN;
            }/* Transizione con first drawn policy */
            else if (!strcasecmp(deschedule_policy, "LAST_DRAWN")) {
                /* Transizione con last drawn policy */
                tabt[ind].deschedule_policy = POLICY_DIS_LAST_DRAWN;
            }/* Transizione con last drawn policy */
            else if (!strcasecmp(deschedule_policy, "FIRST_SCHED")) {
                /* Transizione con first sched policy */
                tabt[ind].deschedule_policy = POLICY_DIS_FIRST_SCHED;
            }/* Transizione con first sched policy */
            else if (!strcasecmp(deschedule_policy, "LAST_SCHED")) {
                /* Transizione con last sched policy */
                tabt[ind].deschedule_policy = POLICY_DIS_LAST_SCHED;
            }/* Transizione con last sched policy */
            /**** Firing distribution ****/

            if (!strcasecmp(type, "DET")) {
                /* Transizione deterministica */
                tabt[ind].timing = TIMING_DETERMINISTIC;
                tabt[ind].stages = 0;
            }/* Transizione deterministica */
            else if (!strcasecmp(type, "ERL")) {
                /* Transizione Erlang */
                tabt[ind].timing = TIMING_ERLANG;
                sscanf(tmp, "%s %s %s %s %s %d", name, policy, reuse_policy, deschedule_policy, type, &n_stages);
                tabt[ind].stages = n_stages;
            }/* Transizione Erlang */
            else if (!strcasecmp(type, "IPO")) {
                /* Transizione ipo-exponential */
                tabt[ind].timing = TIMING_IPOEXP;
                sscanf(tmp, "%s %s %s %s %s %d %n", name, policy, reuse_policy, deschedule_policy, type, &n_stages, &char_read);
                tabt[ind].dist = (double *)ecalloc(n_stages, sizeof(double));
                tabt[ind].stages = n_stages;
                str_p = tmp + char_read;
                for (ii = 1; ii <= n_stages; ii++) {
                    sscanf(str_p, "%lg%n", &tabt[ind].dist[ii - 1], &char_read);
                    str_p += char_read;
                }
            }/* Transizione ipo-exponential */
            else if (!strcasecmp(type, "IPER")) {
                /* Transizione iper-exponential */
                tabt[ind].timing = TIMING_IPEREXP;
                sscanf(tmp, "%s %s %s %s %s %d %n", name, policy, reuse_policy, deschedule_policy, type, &n_stages, &char_read);
                tabt[ind].dist = (double *)ecalloc(2 * n_stages, sizeof(double));
                tabt[ind].stages = n_stages;
                str_p = tmp + char_read;
                for (ii = 1; ii <= n_stages; ii++) {
                    sscanf(str_p, "%lg%lg%n", &tabt[ind].dist[2 * ii - 2], &tabt[ind].dist[2 * ii - 1], &char_read);
                    str_p += char_read;
                }
            }/* Transizione iper-exponential */
            else if (!strcasecmp(type, "UNIF")) {
                /* Transizione uniforme */
                tabt[ind].timing = TIMING_UNIFORM;
                tabt[ind].dist = (double *)ecalloc(2, sizeof(double));
                sscanf(tmp, "%s %s %s %s %s %lg %lg", name, policy, reuse_policy, deschedule_policy, type, &tabt[ind].dist[0], &tabt[ind].dist[1]);
                tabt[ind].stages = 0;
            }/* Transizione uniforme */
        }
    }/* Il file .dis e' presente */
}/* End read_transition_general_distributions */
#endif // 0
#endif
#endif

#ifdef SIMULATION

#define TRANS_LENGTH 1000
#define MIN_BATCH_LENGTH 1000
#define MAX_BATCH_LENGTH 2000
#define APPROX_DEGREE 15
#define CONF_LEVEL 4

int seed = 31415;
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int inters(Node_p  list1,  Node_p  list2) {
    /* Init inters */
    Node_p p1 = NULL;

    if (list1 == NULL || list2 == NULL)
        return (FALSE);
    for (p1 = list1; p1 != NULL ; p1 = NEXT_NODE(p1))
        if (belong(p1->place_no, list2))
            return (TRUE);
    return (FALSE);
}/* End inters */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int CC_SC_belong(int  pl,  Node_p  list,  int  tr) {
    /* Init CC_SC_belong */
    Node_p p = NULL;
    int ret_value = FALSE;

    for (p = list; p != NULL ; p = NEXT_NODE(p))
        if (p->place_no == pl) {
            ret_value = TRUE;
#ifdef SWN
#ifdef SIMULATION
            if (IS_SKIPPABLE(tr))
                p->involved_in_CC_SC = TRUE;
#endif
#endif
        }
    return (ret_value);
}/* End CC_SC_belong */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int CC_SC_inters(Node_p  list1,  Node_p  list2,  int  tr1,  int  tr2) {
    /* Init CC_SC_inters */
    Node_p p1 = NULL;
    int ret_value = FALSE;

    if (!(list1 == NULL || list2 == NULL))
        for (p1 = list1; p1 != NULL ; p1 = NEXT_NODE(p1))
            if (CC_SC_belong(p1->place_no, list2, tr2)) {
                ret_value = TRUE;
#ifdef SWN
#ifdef SIMULATION
                if (IS_SKIPPABLE(tr2))
                    p1->involved_in_CC_SC = TRUE;
#endif
#endif
            }
    return (ret_value);
}/* End CC_SC_inters */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_CC_SC_relations() {
    /* Init get_CC_SC_relations */
    int tr1, tr2, num_cc, num_sc;
    int int1, int2;
    int *trans_cc = (int*)emalloc(sizeof(int) * ntr);
    int *trans_sc = (int*)emalloc(sizeof(int) * ntr);

    for (tr1 = 0; tr1 < ntr; tr1++) {
        /* Per ogni transizione */
        for (tr2 = ntr - 1, num_cc = num_sc = 0; tr2 >= 0; tr2--) {
            /* Main loop */
            if (tr1 != tr2) {
                /* Transizioni diverse */
                int1 = CC_SC_inters(tabt[tr1].inptr, tabt[tr2].inptr, tr1, tr2);
                int2 = CC_SC_inters(tabt[tr1].outptr, tabt[tr2].inibptr, tr1, tr2);
                if (int1 || int2)
                    trans_sc[num_sc++] = tr2;
                int1 = CC_SC_inters(tabt[tr1].inptr, tabt[tr2].inibptr, tr1, tr2);
                int2 = CC_SC_inters(tabt[tr1].outptr, tabt[tr2].inptr, tr1, tr2);
                if (int1 || int2)
                    trans_cc[num_cc++] = tr2;
            }/* Transizioni diverse */
        }/* Main loop */
        /* Write on cc */
        fprintf(cfp, "%d ", num_cc);
        for (; num_cc; num_cc--)
            fprintf(cfp, "%d ", trans_cc[num_cc - 1] + 1);
        fprintf(cfp, "\n");
        /* Write on sc */
        fprintf(sfp, "%d ", num_sc);
        for (; num_sc; num_sc--)
            fprintf(sfp, "%d ", trans_sc[num_sc - 1] + 1);
        fprintf(sfp, "\n");
    }/* Per ogni transizione */
    free(trans_cc);
    free(trans_sc);
}/* End get_CC_SC_relations */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
// void fill_CC_SC_data_structure(int  ok_me) {
//     /* Init fill_CC_SC_data_structure */
//     int *usp;
//     int ii, jj, na, nc;
//     char trans_nm[MAX_TAG_SIZE];

//     for (ii = 0; ii < ntr; ii++) {
//         fscanf(cfp, " %d", &na);
//         tabt[ii].add_l = usp = (int *)ecalloc(na + 1, sizeof(int));
//         *(usp += na) = UNKNOWN;
//         while (na--) {
//             fscanf(cfp, "%d", &jj);
//             *(--usp) = jj - 1;
//         }
//         while (getc(cfp) != '\n');
//     }
//     for (ii = 0; ii < ntr; ii++) {
//         fscanf(sfp, "%d", &nc);
//         tabt[ii].test_l = usp = (int *)ecalloc(nc + 1, sizeof(int));
//         *(usp += nc) = UNKNOWN;
//         while (nc--) {
//             fscanf(sfp, "%d", &jj);
//             *(--usp) = jj - 1;
//         }
//         while (getc(sfp) != '\n');
//     }
//     for (ii = 0 ; ii < ntr ; ii++)
//         tabt[ii].me_l = NULL;
//     if (ok_me) {
//         /* Lettura file di me */
//         int ind;

//         while (TRUE) {
//             fscanf(pinfp, " %s", trans_nm);
//             fscanf(pinfp, " %d", &na);
//             ind = get_tr_in(trans_nm);
//             if (ind == UNKNOWN)
//                 out_error(ERROR_UNKNOWN_TRANSITION_NAME, 0, 0, 0, 0, trans_nm, NULL);
//             tabt[ind].me_l = usp = (int *)ecalloc(na + 1, sizeof(int));
//             *(usp += na) = UNKNOWN;
//             while (na--) {
//                 fscanf(pinfp, "%s", trans_nm);
//                 ind = get_tr_in(trans_nm);
//                 if (ind == UNKNOWN)
//                     out_error(ERROR_UNKNOWN_TRANSITION_NAME, 0, 0, 0, 0, trans_nm, NULL);
//                 *(--usp) = ind;
//             }
//             getc(pinfp);
//             if (feof(pinfp))
//                 break;
//         }
//     }/* Lettura file di me */
// }/* End fill_CC_SC_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void create_marking_data_structure() {
    /* Init create_marking_data_structure */
    int i;

    net_mark = (Net_Mark_p)ecalloc(npl, sizeof(struct NET_MARKING));
#ifdef SWN
    initialize_free_token_list();
#endif
    for (i = 0; i < npl; i++) {
        /* Per ogni posto */
        net_mark[i].total = 0;
#ifdef SWN
        net_mark[i].marking = NULL;
        net_mark[i].marking_t = NULL;
        net_mark[i].different = 0;
#ifdef SYMBOLIC
        net_mark[i].ordinary = 0;
#endif
#ifdef SIMULATION
        net_mark[i].touched_h = NULL;
        net_mark[i].reset_h = NULL;
        net_mark[i].touched_t = NULL;
        net_mark[i].reset_t = NULL;
#ifdef SYMBOLIC
        net_mark[i].group = NULL;
#endif
        net_mark[i].num_of_touched = 0;
        net_mark[i].num_of_reset = 0;
#endif
        if (IS_NEUTRAL(i)) {
            /* Posto neutro */
            Token_p mark_ptr = NULL;

            mark_ptr = (Token_p)emalloc(sizeof(struct COLOURED_TOKEN));
            mark_ptr->id = NULL;
            mark_ptr->molt = 0;
            net_mark[i].marking = mark_ptr;
        }/* Posto neutro */
#endif
    }/* Per ogni posto */
}/* End create_marking_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_marking_data_structure() {
    /* Init fill_marking_data_structure */
    int i, h;
    char lex_buffer[MAXSTRING+100];

    for (i = 0; i < npl; i++) {
        /* Per ogni posto */
        if (tabp[i].position < 0) {
            /* Posto marcato */
            if (tabp[i].position < -10000) {
                /* Posto colorato marcato */
                parse_DEF = INITIAL_MARKING;
                pl_ind = i; /* pl_ind e' usato nella grammatica */
                for (h = 0; h < nmr && tabm[h].position != -((tabp[i].position) + 10000); h++);
                // fseek(mfp, tabm[h].file_pos, SEEK_SET);
                // fgets(tmp, MAXSTRING - 1, mfp);
                num_of_token = 1;

                // Initialize Lex buffer
                assert(strlen(tabm[h].mark_expr) <= MAXSTRING);
                sprintf(lex_buffer, "~m %s", tabm[h].mark_expr);
                lexer_set_buffer(lex_buffer);
                
                parser();
                lexer_set_buffer(NULL);
#ifdef GSPN
                if ((net_mark[i].total = totm0) < 0) {
                    if (running_for_MCC())
                        fprintf(stdout, "CANNOT_COMPUTE\n")
                    else
                        fprintf(stdout, "Error: negative initial marking for place %s\n", PLACE_NAME(i));

                    exit(1);
                }
#endif

#ifdef SWN
#ifdef SIMULATION
                net_mark[pl_ind].touched_h = net_mark[pl_ind].touched_t = NULL;
                net_mark[pl_ind].reset_h = net_mark[pl_ind].reset_t = NULL;
                net_mark[i].num_of_touched = net_mark[i].num_of_reset = 0;
#endif
#endif
            }/* Posto colorato marcato */
            else {
                /* Posto neutro marcato */
                net_mark[i].total = tabmp[-(tabp[i].position) - 1].mark_val;
#ifdef SWN
                if (IS_FULL(i)) {
                    net_mark[i].different = 1;
#ifdef SYMBOLIC
                    net_mark[i].ordinary = 1;
#endif
                }
                SET_TOKEN_MOLTEPLICITY(net_mark[i].total, net_mark[i].marking);
#endif
            }/* Posto neutro marcato */
        }/* Posto marcato */
        else
#ifdef SWN
            if (IS_NEUTRAL(i)) {
                /* Posto neutro */
#endif
                net_mark[i].total = tabp[i].position;
#ifdef SWN
                if (IS_FULL(i)) {
                    net_mark[i].different = 1;
#ifdef SYMBOLIC
                    net_mark[i].ordinary = 1;
#endif
                }
                SET_TOKEN_MOLTEPLICITY(net_mark[i].total , net_mark[i].marking);
            }/* Posto neutro */
#endif
    }/* Per ogni posto */
}/* End fill_marking_data_structure */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int compare_class(const struct COLORS  *c1, const  struct COLORS  *c2) {
    /* Init compare_class */
    if (c1->type == c2->type)
        return (0);
    else if (c1->type == ORDERED)
        return (-1);
    else
        return (1);
}/* End compare_class */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void  order_color_table() {
    /* Init order_color_table */
    qsort((char *)tabc, ncl, sizeof(struct COLORS), (int (*)(const void *, const void *)) compare_class);
}/* End order_color_table */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_color_class_data_structure() {
    /* Inizio fill_color_class_data_structure */
    struct SBCBUF *sbptr;
    int sbin, ind, stop, h, disj;

    for (sbptr = buf_list; sbptr != NULL; sbptr = sbptr->next) {
        /* Scansione lista buffer */
        stop = FALSE;
        for (ind = 0; ind < ncl && !stop; ind++)
            if ((sbin = already_exist(sbptr->name, ind, STAT)) != 0)
                stop = TRUE;
        ind--;
        if (!stop) {
            /* Sottoclasse inesistente */
            out_error(ERROR_UNKNOWN_STATIC_SUBCLASS_NAME, 0, 0, 0, 0, sbptr->name, NULL);
        }/* Sottoclasse inesistente */
        else if (tabc[ind].sbclist[sbin - 1].card != 0) {
            /* Ridichiarazione sottoclasse */
            out_error(ERROR_REDECLARATION, 0, 0, ind, 0, sbptr->name, NULL);
        }/* Ridichiarazione sottoclasse */
        else {
            /* Aggiornamento tabella */
            // This part is fixed by S.baarir
            if (tabc[ind].sbclist[sbin - 1].name == NULL) {
                tabc[ind].sbclist[sbin - 1].name =
                    (char *)ecalloc(strlen(sbptr->name) + 1, sizeof(char));
            }
            // end of the fixed part
            strcpy(tabc[ind].sbclist[sbin - 1].name, sbptr->name);
            tabc[ind].sbclist[sbin - 1].card = sbptr->card;
            if (sbptr->obj_list == NULL) {
                /* Descrizione array-like */
                tabc[ind].sbclist[sbin - 1].obj_list = NULL;
                tabc[ind].sbclist[sbin - 1].low = sbptr->low;
                tabc[ind].sbclist[sbin - 1].obj_name =
                    (char *)ecalloc(strlen(sbptr->obj_name) + 1, sizeof(char));
                strcpy(tabc[ind].sbclist[sbin - 1].obj_name, sbptr->obj_name);
            }/* Descrizione array-like */
            else {
                /* Descrizione per enumerazione */
                tabc[ind].sbclist[sbin - 1].low = 0;
                tabc[ind].sbclist[sbin - 1].obj_name = NULL;
                tabc[ind].sbclist[sbin - 1].obj_list = sbptr->obj_list;
            }/* Descrizione per enumerazione */
            if ((disj = intersection(tabc[ind].sbclist, sbin - 1, tabc[ind].sbc_num)) < 0) {
                /* Sottoclassi non disgiunte */
                disj++; disj = -disj;
                out_error(ERROR_INTERSECTION, 0, 0, ind, 0, GET_STATIC_NAME(ind, sbin - 1), GET_STATIC_NAME(ind, disj));
            }/* Sottoclassi non disgiunte */
        }/* Aggiornamento tabella */
        sbc_def_num++;
    }/* Scansione lista buffer */
    if (tot_sbc_num != sbc_def_num) {
        /* Dichiarazioni non complete */
        out_error(ERROR_INCOMPLETE_DEFINITION, 0, 0, 0, 0, NULL, NULL);
    }/* Dichiarazioni non complete */
    else
        for (ind = 0 ; ind < ncl ; ind++) {
            /* Per ogni classe */
            tabc[ind].card = get_class_card(ind);
            for (sbin = 0; sbin < GET_STATIC_SUBCLASS(ind); sbin++) {
                /* Per ogni sottoclasse statica si calcola il campo offset */
                tabc[ind].sbclist[sbin].offset = 0;
                for (h = 0; h < sbin; h++)
                    tabc[ind].sbclist[sbin].offset += tabc[ind].sbclist[h].card;
            }/* Per ogni sottoclasse statica si calcola il campo offset */
        }/* Per ogni classe */
#ifdef SYMBOLIC
    order_color_table();
#endif
}/* Fine fill_color_class_data_structure */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_global_variables() {
    /* Init initialize_global_variables */
    int i;

    nodeptr = NULL;
#ifdef SWN
    buf_list = NULL;
    mbuf = NULL;
    ncl = 0;
    n_ord = 0;
    n_unord = 0;
#endif
    tot_sbc_num = 0;
    sbc_def_num = 0;
    nmp = 0;
    npl = 0;
    ntr = 0;
    nrp = 0;
    nmd = 0;
    nmr = 0;
    ngr = 0;
    cnt = 0;
    for (i = 0; i < MAX_OBJ; i++) el[i] = 0;
}/* End initialize_global_variables */
#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_dynamic_subclass_data_structure() {
    /* Init fill_dynamic_subclass_data_structure */
    int ss, ok, cl, par;
    struct MARKBUF *mpt = NULL, *frmpt = NULL;
    struct DYNSC *dpt = NULL;

    for (cl = 0 ; cl < ncl ; cl++) {
        /* Per ogni classe di colore */
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (ISED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
            {
                /* Classe non ordinata */
                RESET_NUM_CL(cl);
                for (ss = 0 ; ss < tabc[cl].sbc_num ; ss++) {
                    /* Controllo cardinalita'dsc per ogni sottoclasse */
                    RESET_NUM_SS(cl, ss);
                    dpt = GET_DYNAMIC_LIST(cl, ss);
                    for (; dpt != NULL ;) {
                        /* Per ogni sottoclasse dinamica definita */
                        SET_CARD(cl, ss, GET_NUM_SS(cl, ss), dpt->card);
                        dpt->map = GET_NUM_SS(cl, ss);
                        INCREASE_NUM_SS(cl, ss);
                        INCREASE_NUM_CL(cl);
                        dpt = NEXT_DYNAMIC(dpt);
                    }/* Per ogni sottoclasse dinamica definita */
                }/* Controllo cardinalita'dsc per ogni sottoclasse */
            }/* Classe non ordinata */
    }/* Per ogni classe di colore */
    for (mpt = mbuf ; mpt != NULL ; mpt = mpt->next) {
        /* Per ogni sottoclasse dinamica bufferizzata */
        frmpt = mpt;
        for (cl = 0 , ok = FALSE ; !ok && cl < ncl ; cl++)
            if ((ss = already_exist(mpt->sbc, cl, STAT)) != 0)
                ok = TRUE;
        if (!ok) {
            /* Errore: una sottoclasse statica non esiste */
            out_error(ERROR_UNEXISTING_SUBCLASS_IN_DYNAMIC, 0, 0, 0, 0, mpt->sbc, mpt->name);
        }/* Errore: una sottoclasse statica non esiste */
        else {
            /* La dichiarazione della sottoclasse statica e' corretta */
            cl--; ss--;
#ifdef REACHABILITY
            if (IS_UNORDERED(cl) || (ISED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
#endif
#ifdef SIMULATION
                if (IS_UNORDERED(cl)) {
#endif
                    if (!already_exist(mpt->name, cl, ss)) {
                        /* Corretta per la sottoclasse dinamica */
                        dpt = (struct DYNSC *)emalloc(sizeof(struct DYNSC));
                        dpt->next = tabc[cl].sbclist[ss].dynlist;
                        dpt->card = SET_CARD(cl, ss, GET_NUM_SS(cl, ss), mpt->card);
                        dpt->name = (char *)ecalloc(strlen(mpt->name) + 1, sizeof(char));
                        strcpy(dpt->name, mpt->name);
                        tabc[cl].sbclist[ss].dynlist = dpt;
                        dpt->map = GET_NUM_SS(cl, ss);
                        INCREASE_NUM_SS(cl, ss);
                        INCREASE_NUM_CL(cl);
                    }/* Corretta per la sottoclasse dinamica */
                    else {
                        /* Errore: ridichiarazione della stessa sottoclasse dinamica */
                        out_error(ERROR_REDECLARATION_DYNAMIC, 0, 0, 0, 0, mpt->name, mpt->sbc);
                    }/* Errore: ridichiarazione della stessa sottoclasse dinamica */
                }/* La dichiarazione della sottoclasse statica e' corretta */
            }/* Per ogni sottoclasse dinamica bufferizzata */
#ifdef REACHABILITY
        }
#endif
#ifdef SIMULATION
    }
#endif

    for (cl = 0 ; cl < ncl ; cl++) {
        /* Per ogni classe di colore */
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (ISED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
#endif
            {
                /* Classe non ordinata */
                for (ss = 0 ; ss < tabc[cl].sbc_num ; ss++) {
                    /* Controllo cardinalita'dsc per ogni sottoclasse */
                    par = 0;
                    for (dpt = tabc[cl].sbclist[ss].dynlist; dpt != NULL; dpt = dpt->next)
                        par += dpt->card;
                    if (par > tabc[cl].sbclist[ss].card) {
                        /* Errore definizione cardinalita'sottoclassi dinamiche */
                        out_error(ERROR_INCONSISTENT_CARDINALITIES, 0, 0, cl, ss, NULL, NULL);
                    }/* Errore definizione cardinalita'sottoclassi dinamiche */
                    if (par < tabc[cl].sbclist[ss].card) {
                        /* Manca una sottoclasse dinamica */
#ifndef LIBSPOT
#ifndef LIBESRG
                        fprintf(stdout, "WARNING : a default dynamic subclass has been added for static subclass %s\n", tabc[cl].sbclist[ss].name);
#endif
#endif
                        SET_CARD(cl, ss, GET_NUM_SS(cl, ss), tabc[cl].sbclist[ss].card - par);
                        INCREASE_NUM_SS(cl, ss);
                        INCREASE_NUM_CL(cl);
                    }/* Manca una sottoclasse dinamica */
                }/* Controllo cardinalita'dsc per ogni sottoclasse */
            }/* Classe non ordinata */
    }/* Per ogni classe di colore */
}/* Fine fill_dynamic_subclass_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void create_dynamic_subclass_data_structure() {
    /* Init create_dynamic_subclass_data_structure */
    int i, j;

    if (ncl > 0) {
        /* C'e' almeno una classe */
        card = (int ** *)ecalloc(ncl, sizeof(int **));
        num = (int **)ecalloc(ncl, sizeof(int *));
        tot = (int *)ecalloc(ncl, sizeof(int));
        for (i = 0 ; i < ncl; i++) {
#ifdef REACHABILITY
            if (IS_UNORDERED(i) || (ISED(i) && GET_STATIC_SUBCLASS(i) == 1))
#endif
#ifdef SIMULATION
                if (IS_UNORDERED(i))
#endif
                {
                    /* Classe non ordinata */
#ifndef LIBSPOT
                    card[i] = (int **)ecalloc(GET_STATIC_SUBCLASS(i), sizeof(int *));
                    num[i] = (int *)ecalloc(GET_STATIC_SUBCLASS(i), sizeof(int));
#else
#ifdef ESYMBOLIC
                    card[i] = (int **)ecalloc(get_max_cardinality(), sizeof(int *));
                    num[i] = (int *)ecalloc(get_max_cardinality(), sizeof(int));
#else
                    card[i] = (int **)ecalloc(GET_STATIC_SUBCLASS(i), sizeof(int *));
                    num[i] = (int *)ecalloc(GET_STATIC_SUBCLASS(i), sizeof(int));
#endif
#endif
                }/* Classe non ordinata */
        }
        for (i = 0 ; i < ncl; i++)
#ifdef REACHABILITY
            if (IS_UNORDERED(i) || (ISED(i) && GET_STATIC_SUBCLASS(i) == 1))
#endif
#ifdef SIMULATION
                if (IS_UNORDERED(i))
#endif
#ifndef LIBSPOT
                    for (j = 0; j < GET_STATIC_SUBCLASS(i) ; j++)
#else
#ifdef ESYMBOLIC
                    for (j = 0; j < get_max_cardinality() ; j++)
#else
                    for (j = 0; j < GET_STATIC_SUBCLASS(i) ; j++)
#endif
#endif
#ifndef ESYMBOLIC
                        card[i][j] = (int *)ecalloc(GET_STATIC_CARDINALITY(i, j) , sizeof(int));
#else
                        card[i][j] = (int *)ecalloc(get_max_cardinality() , sizeof(int));
#endif

    }/* C'e' almeno una classe */
}/* End create_dynamic_subclass_data_structure */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void create_cartesian_product_data_structure() {
    /* Init create_cartesian_product_data_structure */
#ifdef SWN
    int ii;
    int max_dimension = UNKNOWN;


    max_cardinality = get_max_cardinality();
    max_place_domain = get_max_place_domain();
    max_transition_domain = get_max_transition_domain();
    max_colour_repetitions = get_max_colour_repetitions();
    max_cl = get_max_cl();

    if (max_cl > 0)
        pred_val = (int *)ecalloc(max_cl, sizeof(int));
#endif

    cart_place_ptr = (Cart_p)emalloc(sizeof(struct CARTESIAN_PRODUCT));
    cart_trans_ptr = (Cart_p)emalloc(sizeof(struct CARTESIAN_PRODUCT));

#ifdef SWN
    if (max_place_domain > 0) {
        cart_place_ptr->low = (int *)ecalloc(max_place_domain, sizeof(int));
        cart_place_ptr->up = (int *)ecalloc(max_place_domain, sizeof(int));
        cart_place_ptr->mark = (int **)ecalloc(max_place_domain, sizeof(int *));
        for (ii = 0; ii < max_place_domain; ii++) {
            max_dimension = MAX(max_cardinality, get_max_elements(ii));
            cart_place_ptr->mark[ii] = (int *)ecalloc(max_dimension, sizeof(int));
        }
    }
    else {
        cart_place_ptr->low = NULL;
        cart_place_ptr->up = NULL;
        cart_place_ptr->mark = NULL;
    }
    if (max_transition_domain > 0) {
        cart_trans_ptr->low = (int *)ecalloc(max_transition_domain, sizeof(int));
        cart_trans_ptr->up = (int *)ecalloc(max_transition_domain, sizeof(int));
        cart_trans_ptr->mark = (int **)ecalloc(max_transition_domain, sizeof(int *));
        for (ii = 0; ii < max_transition_domain; ii++)
#ifdef COLOURED
            cart_trans_ptr->mark[ii] = NULL;
#endif
#ifdef SYMBOLIC
        cart_trans_ptr->mark[ii] = (int *)ecalloc(max_colour_repetitions + 1, sizeof(int));
#endif
    }
    else {
        cart_trans_ptr->low = NULL;
        cart_trans_ptr->up = NULL;
        cart_trans_ptr->mark = NULL;
    }
#endif
}/* End create_cartesian_product_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void compose_name(char  *name,  const char  *path,  const char  *postfix) {
    /* Init compose_name */
#ifdef GREATSPN
    sprintf(name, "%s.%s", path, postfix);
#else
    sprintf(name, "nets/%s.%s", path, postfix);
#endif
}/* End compose_name */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void remove_command(char  *comm, const char  *path,  const char  *postfix) {
    /* Init remove_command */
#ifdef GREATSPN
    sprintf(comm, "/bin/rm \"%s.%s\"", path, postfix);
#else
    sprintf(comm, "/bin/rm \"nets/%s.%s\"", path, postfix);
#endif
}/* End remove_command */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void create_table() {
    /* Init create_table */
    if (nmr > 0)
        tabm = (struct MARKING *)ecalloc(nmr, sizeof(struct MARKING));
    nmr = 0;
    if (nmd > 0)
        tabmd = (struct TRANS_MD_RATE *)ecalloc(nmd, sizeof(struct TRANS_MD_RATE));
    nmd = 0;
#ifdef SWN
    if (ncl > 0)
        tabc = (struct COLORS *)ecalloc(ncl, sizeof(struct COLORS));
    ncl = 0;
#endif
}/* End create_table */

/***************************************************************/
/* Prints help/usage  message */
/**************************************************************/
void usage(bool long_banner) 
{
    if (long_banner && !running_for_MCC()) {
        print_banner("");
        cout << "USE :  RGMEDD5  <netname>  [options...]\n\n"
                "Options description:\n"
                "  -mpar <mpar> <val>  Change the value of a marking parameter.\n"
                "  -rpar <rpar> <val>  Change the value of a rate parameter.\n"
                "  -h <N>       Set the Meddly cache size to <N> entries.\n"
                "  -no-rs       Do not build the RS.\n"
                "  -no-firings  Skip transition firings count.\n"
                "  -sat-mono    Saturation with monolithic NSF.\n"
                "  -sat-event   Saturation with per-event NSF (sat-pregen). [default]\n"
                "  -sat-otf     Saturation with on-the-fly NSF (sat-otf). \n"
                "  -sat-impl    Saturation with implicit NSF (sat-impl). \n"
                "  -bfs-mono    Use BFS with monolithic NSF. Use -bfs-mono-cov for coverability test. \n"
                "  -bfs-impl    Use BFS with implicit NSF. Use -bfs-impl-cov for coverability test.\n"
                "  -lrs         Build the Linearized Reachability Graph from the P-flow basis. \n"
                // "  -o           Save RS and CTL results on a file (<netname>.rs and .ctloutput)\n"
                "  -varord(-only)  Print variable ordering (and optionally quit).\n"
                "  -metrics(-only) Print ordering metrics (and optionally quit).\n"
                "  -meta-scores Print the variable ordering meta-heuristic score table.\n"
                "  -order-image Save a bitmap representing the variable order.\n"
                "  -inc(-open)  Save a EPS of the incidence matrix (and open it).\n"
                "  -xinc(-open) Save a EPS of the incidence matrix (and open it) with RS node counts.\n"
                "  -dot(-open)  Save a representation of the RS DD (and open it).\n"
                "\n"
                "Model checking:\n"
                "  -C           enable model checking (verbose mode).\n"
                "  -Cv          enable model checking (quite mode).\n"
                "  -c           print counter-examples and witnesses of each checked formula.\n"
                "  -f <file>    specify the file containing CTL/LTL formulae. \n"
                "\n"
                "Available variable order heuristics:\n"
                "  -NR          Do not reorder variables.\n"
                "  -META        Metric based meta-heuristics. [default]\n"
                "  -F           Read the variable order from the <netname>.place file. \n"
                "  -RND(-S x)   Random order, with an optionally specified seed. \n"
                "  -RND-0       Fixed, repetible random order.\n"
                "  -P           Derive order using the P-semiflows chaining method.\n"
                // "  -FR          Use standard FORCE algorithm.\n"
                // "  -FR-P        Use FORCE algorithm with P-semiflows.\n"
                // "  -FR-NU       Use FORCE algorithm with  Nested Units.\n"
                "  -BFS         Use breadth-first search order (poor performances).\n"
                "  -DFS         Use depth-first search order (poor performances).\n"
                "  -TS          Use topological sort.\n"
                "  -CM          Use Cuthill-McKee method (boost version).\n"
                "  -CM2         Use Cuthill-McKee method (ViennaCL version).\n"
                "  -ACM         Use Advanced Cuthill-McKee method.\n"
                "  -GPS         Use Gibbs-Poole-Stockmeyer.\n"
                "  -KING        Use King ordering method.\n"
                "  -SLO(-16)    Use Sloan ordering (two parametric variations).\n"
                "  -NOACK(2)    Use Noack ordering.\n"
                "  -TOV(2)      Use Tovchigrechko ordering.\n"
                "  -GP          Use Gradient-P ordering with P-semiflows.\n"
                "  -GNU         Use Gradient-NU ordering with Nested Units.\n"
                "  -MCL         Use Markov Clustering Algorithm.\n"
                "  -scc         Use SCC post-heuristic variable re-ordering.\n"
                "  -sloan-W     <W1> <W2>  Specify integer weights of Sloan function.\n"
                "  -refine-best Refine the selected variable ordering with FORCE iterations.\n"
                "               Also -refine, -refine-psf, -refine-ti, -refine-i.\n"
                "  -rev         Use reverse order.\n"
                "  -test-rev    Test if the reverse order is better than the actual.\n"
                "  -rnd-swap n  Perform n random swaps on the selected variable order.\n"
                ;
                // "  -A         RS * Automaton.\n"
    }
    // Print GreatSPN banner & maintainer email
    print_banner("");
    cout << "GreatSPN/Meddly." << endl;
    cout << "  Copyright (C) 1987-2022, University of Torino, Italy." << endl;
    cout << "  website: https://github.com/greatspn/SOURCES" << endl;
    if (!running_for_MCC()) {
        cout << "  Send files netname.net, .def to e-mail address" << endl;
        cout << "  amparore(at)di.unito.it if you find any bug." << endl;
        print_banner("");
    }
    else cout << endl;
    // Print Meddly banner
    cout << "Based on " << MEDDLY::getLibraryInfo(0) << endl;
    cout << "  " << MEDDLY::getLibraryInfo(1) << endl;
    cout << "  website: " << MEDDLY::getLibraryInfo(3) << endl;
    print_banner("");
}

#ifdef SIMULATION
# error "SIMULATION should not be enabled in RGMEDD5."
#endif

#ifdef LIBSPOT
# error "LIBSPOT should not be enabled in RGMEDD5."
#endif

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int initialize(int  argc,  char  *argv[]) {
    char net[MAXSTRING], comm[MAXSTRING];

    // int ok, disok;
    int ii;
    // char cc;

//MARIETA
    int ret_read_pin_file;
//MARIETA


// #ifdef SIMULATION
//     int first_tr_l = TRANS_LENGTH;
//     int tr_l = TRANS_LENGTH;
//     int min_b = MIN_BATCH_LENGTH;
//     int max_b = MAX_BATCH_LENGTH;
//     int appr = APPROX_DEGREE;
//     int conf_l = CONF_LEVEL;
//     int old_conf_l = 95;
// #endif

// #ifdef LIBSPOT
// #define exit return
// #endif
    bname[0] = '\0';
    if (argc < 2) {
        usage(true);
        exit(1);
    }

    // Initialize random generator seeds
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    g_random_order_seeds[0] = tv.tv_sec;
    g_random_order_seeds[1] = tv.tv_usec;
    // Set meta-heuristics as default variable reorder method
    g_var_order_sel.heuristics = VOC_META_BY_SCORE;


// #ifdef SIMULATION
//     for (ii = 2; ii < argc; ii += 2) {
//         /* Lettura delle opzioni */
//         if (argv[ii + 1] != NULL) {
//             /* Comando (forse) corretto */
//             cc = argv[ii][1];
//             switch (cc) {
//             /* Casistica */
//             case 'f' : first_tr_l = atoi(argv[ii + 1]);
//                 break;
//             case 't' : tr_l = atoi(argv[ii + 1]);
//                 break;
//             case 'm' : min_b = atoi(argv[ii + 1]);
//                 break;
//             case 'M' : max_b = atoi(argv[ii + 1]);
//                 break;
//             case 'a' : appr = atoi(argv[ii + 1]);
//                 break;
//             case 'c' : old_conf_l = atoi(argv[ii + 1]);
//                 switch (old_conf_l) {
//                 /* Codifica da percent. a offset nella tabella t_student */
//                 case 60 : conf_l = 0;
//                     break;
//                 case 70 : conf_l = 1;
//                     break;
//                 case 80 : conf_l = 2;
//                     break;
//                 case 90 : conf_l = 3;
//                     break;
//                 case 95 : conf_l = 4;
//                     break;
//                 case 99 : conf_l = 6;
//                     break;
//                 default : fprintf(stderr, "Incorrect confidence level value %d\n", old_conf_l);
//                     break;
//                 }/* Codifica da percent. a offset nella tabella t_student */
//                 break;
//             case 's' : seed = atoi(argv[ii + 1]);
//                 break;
//             case 'o' : output_flag = TRUE;
//                 start = atoi(argv[ii + 1]);
//                 break;
//             default  :
//                 fprintf(stderr, "Unknown option %s\n", argv[ii]);

//                 exit(1);
//                 break;
//             }/* Casistica */
//         }/* Comando (forse) corretto */
//         else {
//             /* Comando sicuramente scorretto */
//             fprintf(stderr, "Incorrect command line\n");
//             exit(1);
//         }/* Comando sicuramente scorretto */
//     }/* Lettura delle opzioni */
// #endif
#ifdef REACHABILITY
    for (ii = 2; ii < argc; ii++) {
        /* Lettura delle opzioni */
        // cc = argv[ii][1];
        /* Casistica */
        if (0 == strcmp(argv[ii], "-o")) {
            output_flag = TRUE;
        }
        else if (0 == strcmp(argv[ii], "-M")) {
            out_mc = TRUE;
        }
        else if (0 == strcmp(argv[ii], "-l")) {
            fast_solve = FALSE;
            out_mc = TRUE;
        }
        else if (0 == strcmp(argv[ii], "-b")) {
            out_mc = TRUE;
            exp_set = TRUE;
            fast_solve = FALSE;
        }
        else if (0 == strcmp(argv[ii], "-d")) {
            dot_flag = TRUE;
            output_flag = TRUE;
        }
#ifdef SWN
        // else if (0 == strcmp(argv[ii], "-w")) {
        //     cvrs_flag = TRUE;
        //     MDWN_flag = TRUE;
        //     //output_flag = TRUE;
        //     out_mc = TRUE;
        // }
#endif
        /*MDD*/
        // else if (0 == strcmp(argv[ii], "-B")) {
        //     if (ii + 1 < argc) {
        //         ii = ii + 1;
        //         Max_Token_Bound = atoi(argv[ii]) + 1;
        //     }
        //     else {
        //         cerr << "\nError:  -B <integer>\n";
        //         exit(-1);
        //     }
        // }
        else if (0 == strcmp(argv[ii], "-C")) {
            CTL = true;
        }
        else if (0 == strcmp(argv[ii], "-CTL-with-SatELTL")) {
            eval_CTL_using_SatELTL = true;
        }
        else if (0 == strcmp(argv[ii], "-CTL-pure")) {
            eval_CTL_using_SatELTL = false;
        }
        else if (0 == strcmp(argv[ii], "-satsets")) {
            CTL_print_sat_sets = true;
        }
        else if (0 == strcmp(argv[ii], "-preimg-rs")) {
            CTL_preimg_stays_in_RS = true;
        }
        else if (0 == strcmp(argv[ii], "-CTL-satsets")) {
            CTL_print_intermediate_sat_sets = true;
        }
        // else if (0 == strcmp(argv[ii], "-LTL-weak-next")) {
        //     LTL_weak_next = true;
        // }
        else if (0 == strcmp(argv[ii], "-impl-next")) {
            implicitNextForCTLstar = true;
        }
        else if (0 == strcmp(argv[ii], "-Cv")) {
            CTL = true;
            CTL_quiet = true;
        }
        else if (0 == strcmp(argv[ii], "-c")) {
            print_CTL_counterexamples = true;
        }
        else if (0 == strcmp(argv[ii], "-sort-queries")) {
            sort_CTL_queries = true;
        }
        else if (0 == strcmp(argv[ii], "-pss")) {
            CTL_atoms_use_potential_state_space = true;
        }
        else if (0 == strcmp(argv[ii], "-dot") || 0 == strcmp(argv[ii], "-dot-open")) {
            g_dot_RS = true;
            g_dot_open_RS = (nullptr != strstr(argv[ii], "open"));
        }
        else if (0 == strcmp(argv[ii], "-dot-F")) { // used by Java GUI
            g_dot_RS = true;
            g_dot_open_RS = false;
            g_dot_file = argv[++ii];
        }
        else if (0 == strcmp(argv[ii], "-h")) {
            if (ii + 1 < argc) {
                ii = ii + 1;
                meddly_cache_size = atol(argv[ii]);
            }
            else {
                cerr << "\nError:  -h <num_bytes>\n";
                exit(-1);
            }
        }
        else if (0 == strcmp(argv[ii], "-S")) {
            if (ii + 1 < argc) {
                ii = ii + 1;
                g_random_order_seeds[0] = g_random_order_seeds[1] = atol(argv[ii]);
            }
            else {
                cerr << "\nError:  -S <integer seed>\n";
                exit(-1);
            }
        }
        else if (0 == strcmp(argv[ii], "-S2")) {
            if (ii + 2 < argc) {
                ii = ii + 2;
                g_random_order_seeds[0] = atol(argv[ii-1]);
                g_random_order_seeds[1] = atol(argv[ii]);
            }
            else {
                cerr << "\nError:  -S2 <seed1> <seed2>\n";
                exit(-1);
            }
        }
        else if (0 == strcmp(argv[ii], "-GP-W") && ii + 1 < argc) {
            GP_WEIGHT = atof(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-GP-W2") && ii + 1 < argc) {
            GP_WEIGHT2 = atof(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-GP-E") && ii + 1 < argc) {
            GP_EXP = atof(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-GP-SIGN") && ii + 1 < argc) {
            GP_SIGN = atoi(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-GP-G") && ii + 1 < argc) {
            GP_GRAD = atof(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-P-old")) {
            g_var_order_sel.heuristics = VOC_PINV;
        }
        else if (0 == strcmp(argv[ii], "-NR")) {
            g_var_order_sel.heuristics = VOC_NO_REORDER;
        }
        else if (0 == strcmp(argv[ii], "-P")) {
            g_var_order_sel.heuristics = VOC_PCHAINING;
        }
        else if (0 == strcmp(argv[ii], "-RND-0")) {
            g_var_order_sel.heuristics = VOC_RANDOM;
            // fix the random seed
             g_random_order_seeds[0] = 0x3a11323e3a120f4a;
             g_random_order_seeds[1] = 0x6037444b05280d50;
        }
        else if (0 == strcmp(argv[ii], "-RND")) {
            g_var_order_sel.heuristics = VOC_RANDOM;
        }
        else if (0 == strcmp(argv[ii], "-RND-S")) {
            g_var_order_sel.heuristics = VOC_RANDOM;
            if (ii + 1 < argc) {
                ii = ii + 1;
                g_random_order_seeds[0] = g_random_order_seeds[1] = atol(argv[ii]);
            }
            else {
                cerr << "\nError:  -RND-S <integer seed>\n";
                exit(-1);
            }
        }
        else if (0 == strcmp(argv[ii], "-F")) {
            g_var_order_sel.heuristics = VOC_FROM_FILE;
        }
        else if (0 == strcmp(argv[ii], "-set-varord") || 0 == strcmp(argv[ii], "-set-varorder")) {
            if (ii + 1 < argc) {
                g_var_order_sel.heuristics = VOC_FROM_FILE;
                g_given_varorder = argv[++ii];
            }
            else {
                cerr << "\nError:  -set-varord \"<list of place names>\"\n";
                exit(1);
            }
        }
        else if (0 == strcmp(argv[ii], "-BFS")) {
            g_var_order_sel.heuristics = VOC_BFS;
        }
        else if (0 == strcmp(argv[ii], "-DFS")) {
            g_var_order_sel.heuristics = VOC_DFS;
        }
        else if (0 == strcmp(argv[ii], "-FR")) {
            g_var_order_sel.heuristics = VOC_FORCE;
        }
        else if (0 == strcmp(argv[ii], "-FR-P")) {
            g_var_order_sel.heuristics = VOC_FORCE_PINV;
        }
        else if (0 == strcmp(argv[ii], "-FR-NU")) {
            g_var_order_sel.heuristics = VOC_FORCE_NU;
        }
       else if (0 == strcmp(argv[ii], "-CM")) {
            g_var_order_sel.heuristics = VOC_CUTHILL_MCKEE;
        }
        else if (0 == strcmp(argv[ii], "-TS")) {
            g_var_order_sel.heuristics = VOC_TOPOLOGICAL;
        }
        else if (0 == strcmp(argv[ii], "-CM2")) {
            g_var_order_sel.heuristics = VOC_VCL_CUTHILL_MCKEE;
        }
        else if (0 == strcmp(argv[ii], "-ACM")) {
            g_var_order_sel.heuristics = VOC_VCL_ADVANCED_CUTHILL_MCKEE;
        }
        else if (0 == strcmp(argv[ii], "-GPS")) {
            g_var_order_sel.heuristics = VOC_VCL_GIBBS_POOLE_STOCKMEYER;
        }
        else if (0 == strcmp(argv[ii], "-KING")) {
            g_var_order_sel.heuristics = VOC_KING;
        }
        else if (0 == strcmp(argv[ii], "-SLO")) {
            g_var_order_sel.heuristics = VOC_SLOAN;
        }
        else if (0 == strcmp(argv[ii], "-SLO-16")) {
            g_var_order_sel.heuristics = VOC_SLOAN_1_16;
        }
        else if (0 == strcmp(argv[ii], "-sloan-W") && ii + 2 < argc) {
            g_var_order_sel.heuristics = VOC_SLOAN;
            g_sloan_W1 = atoi(argv[++ii]);
            g_sloan_W2 = atoi(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-NOACK")) {
            g_var_order_sel.heuristics = VOC_NOACK;
        }
        else if (0 == strcmp(argv[ii], "-TOV")) {
            g_var_order_sel.heuristics = VOC_TOVCHIGRECHKO;
        }
        else if (0 == strcmp(argv[ii], "-NOACK2")) {
            g_var_order_sel.heuristics = VOC_NOACK2;
        }
        else if (0 == strcmp(argv[ii], "-TOV2")) {
            g_var_order_sel.heuristics = VOC_TOVCHIGRECHKO2;
        }
        else if (0 == strcmp(argv[ii], "-GP")) {
            g_var_order_sel.heuristics = VOC_GRADIENT_P;
        }
        // else if (0 == strcmp(argv[ii], "-MCL")) {
        //     g_var_order_sel.heuristics = VOC_MARKOV_CLUSTER;
        // }
        else if (0 == strcmp(argv[ii], "-TS")) {
            g_var_order_sel.heuristics = VOC_TOPOLOGICAL;
        }
        else if (0 == strcmp(argv[ii], "-GNU")) {
            g_var_order_sel.heuristics = VOC_GRADIENT_NU;
        }
        else if (0 == strcmp(argv[ii], "-rev")) {
            g_var_order_sel.reverse_order = true;
        }
        else if (0 == strcmp(argv[ii], "-test-rev")) {
            g_var_order_sel.reverse_if_better_soups = true;
        }
        else if (0 == strcmp(argv[ii], "-refine")) {
            g_var_order_sel.refinement = ForceBasedRefinement::FORCE;
        }
        else if (0 == strcmp(argv[ii], "-refine-psf")) {
            g_var_order_sel.refinement = ForceBasedRefinement::FORCE_CONSTR;
        }
        else if (0 == strcmp(argv[ii], "-refine-ti")) {
            g_var_order_sel.refinement = ForceBasedRefinement::FORCE_TI;
        }
        else if (0 == strcmp(argv[ii], "-refine-i")) {
            g_var_order_sel.refinement = ForceBasedRefinement::FORCE_I;
        }
        else if (0 == strcmp(argv[ii], "-refine-best")) {
            g_var_order_sel.refinement = ForceBasedRefinement::BEST_AVAILABLE;
        }
        else if (0 == strcmp(argv[ii], "-grpann")) {
            g_var_order_sel.group_annealing = GroupAnnealing::GRPANN_INV;
        }
        else if (0 == strcmp(argv[ii], "-grpann-force")) {
            g_var_order_sel.group_annealing = GroupAnnealing::GRPANN_INV_FORCE;
        }
        else if (0 == strcmp(argv[ii], "-ann-pbasis")) {
            g_var_order_sel.annealing = Annealing::ANN_PBASIS;
        }
        else if (0 == strcmp(argv[ii], "-ann-pbasis-min")) {
            g_var_order_sel.annealing = Annealing::ANN_PBASIS_MIN;
        }
        else if (0 == strcmp(argv[ii], "-ann-soups")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_SOUPS;
        }
        else if (0 == strcmp(argv[ii], "-ann-swir")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_SWIR;
        }
        else if (0 == strcasecmp(argv[ii], "-ann-iRank")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_iRank;
        }
        else if (0 == strcmp(argv[ii], "-ann-nes")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_NES;
        }
        else if (0 == strcmp(argv[ii], "-ann-wes1")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_WES1;
        }
        else if (0 == strcmp(argv[ii], "-ann-psf")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_PSF;
        }
        else if (0 == strcmp(argv[ii], "-ann-pf")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_PF;
        }
        else if (0 == strcmp(argv[ii], "-ann-sot")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_SOT;
        }
        else if (0 == strcmp(argv[ii], "-ann-pts")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_PTS;
        }
        else if (0 == strcmp(argv[ii], "-ann-pts_p")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_PTS_P;
        }
        else if (0 == strcmp(argv[ii], "-ann-bw")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_BW;
        }
        else if (0 == strcmp(argv[ii], "-ann-prof")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_PROF;
        }
        else if (0 == strcmp(argv[ii], "-ann-avgwf")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_AVGWF;
        }
        else if (0 == strcmp(argv[ii], "-ann-maxwf")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_MAXWF;
        }
        else if (0 == strcmp(argv[ii], "-ann-rmswf")) {
            g_var_order_sel.annealing = Annealing::ANN_MINIMIZE_METRIC;
            g_var_order_sel.ann_metric = VariableOrderMetric::METRIC_RMSWF;
        }
        else if (0 == strcmp(argv[ii], "-ann-N") && ii + 1 < argc) {
            // g_refine_annealing = true;
            g_sim_ann_num_tentatives = atoi(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-no-refine")) { }
        else if (0 == strcmp(argv[ii], "-no-ann")) { }
        else if (ii + 1 < argc && 
                 (0 == strcmp(argv[ii], "-annfr") ||
                  0 == strcmp(argv[ii], "-annfrp") ||
                  0 == strcmp(argv[ii], "-annfrti"))) 
        {
            if (0 == strcmp(argv[ii], "-annfr"))
                g_var_order_sel.annealing = Annealing::ANN_FORCE;
            else if (0 == strcmp(argv[ii], "-annfrp"))
                g_var_order_sel.annealing = Annealing::ANN_FORCE_P;
            else
                g_var_order_sel.annealing = Annealing::ANN_FORCE_TI;

            for (int i=0; i<NUM_METRICS; i++) {
                if (0 == strcmp(argv[ii+1], metric_name(VariableOrderMetric(i)))) {
                    g_var_order_sel.ann_metric = VariableOrderMetric(i);
                    break;
                }
            }
            if (g_var_order_sel.ann_metric == NO_METRIC) {
                cerr << "Unknown metric named " << argv[ii+1] << endl;
                exit(1);
            }
            ii += 1;
        }
        else if (0 == strcmp(argv[ii], "-rnd-swaps") && ii + 1 < argc) {
            g_var_order_sel.num_random_swaps = atoi(argv[++ii]);
        }
        // else if (0 == strcmp(argv[ii], "-NOACK-NU")) {
        //     g_var_order_sel.heuristics = VOC_NOACK_NU;
        // }
        // else if (0 == strcmp(argv[ii], "-TOV-NU")) {
        //     g_var_order_sel.heuristics = VOC_TOVCHIGRECHKO_NU;
        // }
        // Meta-heuristics
        else if (0 == strcmp(argv[ii], "-META")) { 
            g_var_order_sel.heuristics = VOC_META_BY_SCORE;
        }
        else if (0 == strcmp(argv[ii], "-parallel-vo") && ii + 1 < argc) {
            g_par_vo_num_parallel_procs = atoi(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-parallel-sp") && ii + 4 < argc) {
            g_var_order_sel.num_tested_varorders = atoi(argv[++ii]);
            g_var_order_sel.time_for_each_test = atoi(argv[++ii]);
            g_var_order_sel.num_parallel_procs = atoi(argv[++ii]);
            g_max_MB_statespace = atoi(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-parallel-mc") && ii + 3 < argc) {
            g_par_mc_max_time_round0 = atoi(argv[++ii]);
            g_par_mc_num_parallel_procs = atoi(argv[++ii]);
            g_par_mc_max_MB_statespace = atoi(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-parallel-mc-no-2nd-round")) {
            g_par_mc_do_second_round = false;
        }
        else if (0 == strcmp(argv[ii], "-MFR") && ii + 1 < argc) {
            g_var_order_sel.heuristics = VOC_META_FORCE;
            for (int i=0; i<NUM_METRICS; i++) {
                if (0 == strcmp(argv[ii+1], metric_name(VariableOrderMetric(i)))) {
                    g_var_order_sel.target_metric = VariableOrderMetric(i);
                    break;
                }
            }
            if (g_var_order_sel.target_metric == NO_METRIC) {
                cerr << "Unknown metric named " << argv[ii+1] << endl;
                exit(1);
            }
            ii += 1;
        }
        // else if (0 == strcmp(argv[ii], "-META-NES")) {
        //     g_var_order_sel.heuristics = VOC_META_BY_SCORE_NES;
        // }
        // else if (0 == strcmp(argv[ii], "-META-WES1")) {
        //     g_var_order_sel.heuristics = VOC_META_BY_SCORE_WES1;
        // }
        // else if (0 == strcmp(argv[ii], "-META-NLWES")) {
        //     g_var_order_sel.heuristics = VOC_META_BY_SCORE_NLWES;
        // }
        // else if (0 == strcmp(argv[ii], "-META-CRS")) {
        //     g_var_order_sel.heuristics = VOC_META_BY_SCORE_CRS;
        // }
        // else if (0 == strcmp(argv[ii], "-META-CRS1")) {
        //     g_var_order_sel.heuristics = VOC_META_BY_SCORE_CRS1;
        // }
        // else if (0 == strcmp(argv[ii], "-META-ANN")) {
        //     g_var_order_sel.heuristics = VOC_META_USE_ANN;
        // }
        // else if (0 == strcmp(argv[ii], "-print-metrics-quit")) {
        //     g_var_order_sel.heuristics = VOC_PRINT_METRICS_AND_QUIT;
        // }
        else if (0 == strcmp(argv[ii], "-no-firings")) {
            g_count_firings = false;
        }
        else if (0 == strcmp(argv[ii], "-varord")) {
            g_show_var_order = true;
        }
        else if (0 == strcmp(argv[ii], "-varord-ltsmin")) {
            g_save_var_order_for_ltsmin = true;
        }
        else if (0 == strcmp(argv[ii], "-varord-rgmedd")) {
            g_save_var_order_for_rgmedd = true;
        }
        else if (0 == strcmp(argv[ii], "-varord-only")) {
            g_show_var_order = true;
            g_exit_after_varorder = true;
        }
        else if (0 == strcmp(argv[ii], "-metrics")) {
            g_print_varorder_metrics = true;
        }
        else if (0 == strcmp(argv[ii], "-metrics-only")) {
            g_print_varorder_metrics = true;
            g_exit_after_varorder = true;
        }
        else if (0 == strcmp(argv[ii], "-meta-scores")) {
            g_print_varorder_metaheuristic_scores = true;
        }
        // else if (0 == strcmp(argv[ii], "-inc-ppm") || 
        //          0 == strcmp(argv[ii], "-inc-ppm-open")) {
        //     g_print_varorder_metrics = true;
        //     g_save_incidence_as_image = true;
        //     g_open_saved_incidence = (nullptr != strstr(argv[ii], "open"));
        // }
        else if (0 == strcmp(argv[ii], "-inc") || 
                 0 == strcmp(argv[ii], "-inc-open")) {
            g_print_varorder_metrics = true;
            g_save_incidence_as_eps = true;
            g_open_saved_incidence = (nullptr != strstr(argv[ii], "open"));
        }
        else if (0 == strcmp(argv[ii], "-xinc") || 
                 0 == strcmp(argv[ii], "-xinc-open")) {
            g_save_ext_incidence = true;
            g_open_saved_ext_incidence = (nullptr != strstr(argv[ii], "open"));
        }
        else if (0 == strcmp(argv[ii], "-inc-F")) { // used by Java GUI
            g_print_varorder_metrics = true;
            g_save_incidence_as_eps = true;
            g_open_saved_incidence = false;
            g_incidence_file = argv[++ii];
        }
        else if (0 == strcmp(argv[ii], "-flow")) {
            g_print_pbasis_metrics = true;
        }
        else if (0 == strcmp(argv[ii], "-scc")) {
            g_var_order_sel.reorder_SCC = true;
        }
        else if (0 == strcmp(argv[ii], "-mdd-fp-optimistic")) 
        { mdd_node_del_policy = forest::policies::node_deletion::OPTIMISTIC_DELETION; }
        else if (0 == strcmp(argv[ii], "-mdd-fp-pessimistic")) 
        { mdd_node_del_policy = forest::policies::node_deletion::PESSIMISTIC_DELETION; }
        else if (0 == strcmp(argv[ii], "-mdd-fp-never-delete")) 
        { mdd_node_del_policy = forest::policies::node_deletion::NEVER_DELETE; }
        else if (0 == strcmp(argv[ii], "-mxd-fp-optimistic")) 
        { mxd_node_del_policy = forest::policies::node_deletion::OPTIMISTIC_DELETION; }
        else if (0 == strcmp(argv[ii], "-mxd-fp-pessimistic")) 
        { mxd_node_del_policy = forest::policies::node_deletion::PESSIMISTIC_DELETION; }
        else if (0 == strcmp(argv[ii], "-mxd-fp-never-delete")) 
        { mxd_node_del_policy = forest::policies::node_deletion::NEVER_DELETE; }
        else if (0 == strcmp(argv[ii], "-f")) {
            if (ii + 1 < argc) {
                ii = ii + 1;
                ctl_name = ii;
            }
            else {
                if (running_for_MCC())
                    fprintf(stdout, "CANNOT_COMPUTE\n");
                else
                    cerr << "\nError CTL file name\n";
                exit(1);
            }
        }
        else if (0 == strcmp(argv[ii], "-A")) {
            AUTOMA = true;
            out_mc = TRUE;
        }
        else if (0 == strcmp(argv[ii], "-e")) {
            if (ii + 1 < argc) {
                ii = ii + 1;
                _prec = atof(argv[ii]);
            }
            else {
                cerr << "\nError precision:  -e <integer>\n";
                exit(1);
            }
        }
        else if (0 == strcmp(argv[ii], "-i")) {
            if (ii + 1 < argc) {
                ii = ii + 1;
                _iter = atoi(argv[ii]);
            }
            else {
                cerr << "\nError iterations' number:  -i <integer>\n";
                exit(1);
            }
        }
        else if (0 == strcmp(argv[ii], "-mpar")) {
            if (ii + 2 < argc) {
                int new_val = atoi(argv[ii + 2]);
                printf("Overriding marking parameter %s to value %d.\n", argv[ii + 1], new_val);
                add_marking_param_to_change(argv[ii + 1], new_val);
                ii += 2;
                continue;
            }
            else {
                fprintf(stderr, "\nError:  -mpar  <marking parameter name>  <new value>\n");
                exit(1);
            }
        }
        else if (0 == strcmp(argv[ii], "-rpar")) {
            if (ii + 2 < argc) {
                double new_val = atof(argv[ii + 2]);
                printf("Overriding rate parameter %s to value %lf.\n", argv[ii + 1], new_val);
                add_rate_param_to_change(argv[ii + 1], new_val);
                ii += 2;
                continue;
            }
            else {
                fprintf(stderr, "\nError:  -rpar  <rate parameter name>  <new value>\n");
                exit(1);
            }
        }
        else if (0 == strcmp(argv[ii], "-gui-stat")) {
            // The Java GUI wants the statistics printed in its format.
            set_print_stat_for_gui();
        }
        else if (0 == strcmp(argv[ii], "--smart")) {
            SMART = true;
            if (ii + 5 < argc) {
                ii++;
                _prec = atof(argv[ii]);
                ii++;
                _iter = atoi(argv[ii]);
                ii++;
                _min = atof(argv[ii]);
                ii++;
                _max = atof(argv[ii]);
                ii++;
                _step = atof(argv[ii]);
            }
            else {
                cerr << "\n --smart <double_precision> <integer_iteration> <transient_double_min> <transient_double_max> <transient_int_step>\n";
            }
        }
        else if (0 == strcmp(argv[ii], "-bfs-mono")){
            initRsMethod = RSM_BFS_MONOLITHIC;
        }
        else if (0 == strcmp(argv[ii], "-bfs-mono-cov")){
            initRsMethod = RSM_BFS_MONOLITHIC_COV;
        }
        else if (0 == strcmp(argv[ii], "-bfs-impl")){
            initRsMethod = RSM_BFS_IMPLICIT;
        }
        else if (0 == strcmp(argv[ii], "-bfs-impl-cov")){
            initRsMethod = RSM_BFS_IMPLICIT_COV;
        }
        else if (0 == strcmp(argv[ii], "-sat-mono")){
            initRsMethod = RSM_SAT_MONOLITHIC;
        }
        else if (0 == strcmp(argv[ii], "-sat-event")){
            initRsMethod = RSM_SAT_PREGEN;
        }
        else if (0 == strcmp(argv[ii], "-sat-impl")){
            initRsMethod = RSM_SAT_IMPLICIT;
        }
        else if (0 == strcmp(argv[ii], "-sat-otf")){
            initRsMethod = RSM_SAT_OTF;
        }
        else if (0 == strcmp(argv[ii], "-m0min")) {
            use_m0_min = true;
        }
        else if (0 == strcmp(argv[ii], "-no-rs")){
            initRsMethod = RSM_NONE;
        }
        else if (0 == strcmp(argv[ii], "-lrs")){
            initLrsMethod = LRSM_PBASIS_CONSTRAINTS;
        }
        else if (0 == strcmp(argv[ii], "-ilcp")) {
            initRsMethod = RSM_NONE;
            initLrsMethod = LRSM_PBASIS_CONSTRAINTS;
            ilcp_model = true;
        }
        else if (0 == strcmp(argv[ii], "-ilcp-ldc") && ii + 1 < argc) {
            ilcp_num_lin_dep_constr = atoi(argv[++ii]);
        }
        // else if (0 == strcmp(argv[ii], "-old-nsf-gen")) {
        //     fast_NSF_gen = false;
        // }
        else if (0 == strcmp(argv[ii], "-rgmedd4-nsf-gen")) {
            fast_NSF_gen = true;
        }
        else if (0 == strcmp(argv[ii], "-print-guess")) {
            print_guessed_bounds = true;
        }
        else if (0 == strcmp(argv[ii], "-lrs-equiv")) {
            test_LRS_RS_equiv = true;
        }
        else if (0 == strcmp(argv[ii], "-extra-lvls") && ii + 1 < argc) {
            g_num_extra_levels = atoi(argv[++ii]);
        }
        else if (0 == strcmp(argv[ii], "-guess-m0-linear")){
            bound_policy = BoundGuessPolicy::M0_LINEAR;
        }
        else if (0 == strcmp(argv[ii], "-guess-m0-exp")){
            bound_policy = BoundGuessPolicy::M0_EXP;
        }
        else if (0 == strcmp(argv[ii], "-guess-max0-linear")){
            bound_policy = BoundGuessPolicy::MAX0_LINEAR;
        }
        else if (0 == strcmp(argv[ii], "-guess-max0-exp")){
            bound_policy = BoundGuessPolicy::MAX0_EXP;
        }
        else if (0 == strcmp(argv[ii], "-guess-m0-max0-exp")){
            bound_policy = BoundGuessPolicy::M0_MAX0_EXP;
        }
        else if (0 == strcmp(argv[ii], "-guess-load-from-file")){
            bound_policy = BoundGuessPolicy::LOAD_FROM_FILE;
        }
        else if (0 == strcmp(argv[ii], "-safe")){
            bound_policy = BoundGuessPolicy::SAFE;
        }
        /*MDD*/
// #ifdef ESYMBOLIC
//         case 'r'  : read_files = TRUE; break;
// #endif

// #ifdef LIBSPOT
// #ifdef ESYMBOLIC
//         case 'i'  : incl = COMPL; break;
//         case 'p'  : incl = PART; break;
//         case 'k'  : break;
// #endif
// #endif
        else {
            fprintf(stderr, "Unknown option %s\n", argv[ii]);
            exit(1);
        }/* Lettura delle opzioni */
    }
#endif
// #ifdef SIMULATION
//     if (tr_l <= 0) {
//         /* Transitorio errato */
//         fprintf(stderr, "Incorrect transitory value %d\n", tr_l);
//         exit(1);
//     }/* Transitorio errato */
//     if (min_b <= 0 || max_b <= 0) {
//         /* limiti errati */
//         fprintf(stderr, "Incorrect lower and upper values %d-%d\n", min_b, max_b);
//         exit(1);
//     }/* limiti errati */
//     if (min_b > max_b) {
//         /* limiti errati */
//         fprintf(stderr, "Error: lower > upper %d-%d\n", min_b, max_b);
//         exit(1);
//     } limiti errati
//     if (seed <= 0) {
//         /* Seme errato */
//         fprintf(stderr, "Incorrect initial seed %d\n", seed);
//         exit(1);
//     }/* Seme errato */
//     if (start < 0) {
//         /* Seme errato */
//         fprintf(stderr, "Incorrect tracing starting point%d\n", start);
//         exit(1);
//     }/* Seme errato */
//     if (appr <= 0 || appr > 100) {
//         /* Grado di approssimazione errato */
//         fprintf(stderr, "Incorrect accuracy value %d\n", appr);
//         exit(1);
//     }/* Grado di approssimazione errato */
// #endif

    // Initialize random generator using the seed
    if (!running_for_MCC()) {
        cout << "Random seeds: " << g_random_order_seeds[0] << " " << g_random_order_seeds[1] << endl;
    }
    init_by_array64(g_random_order_seeds, 
                    sizeof(g_random_order_seeds) / sizeof(g_random_order_seeds[0]));

    usage(false);
    // Check priorities support
    if (ngr > 1 && 
        (initRsMethod == RSM_SAT_IMPLICIT || initRsMethod == RSM_SAT_OTF))
    {
        cout << "\nWARNING: the selected RS method does not support priorities, used in the model.\n";
        cout << "         Switching back to monolithic NSF (-mono-nsf).\n" << endl;
        initRsMethod = RSM_SAT_MONOLITHIC;
    }
    bool build_NSF = false;
    if (!running_for_MCC()) {
        switch (initRsMethod) {
            case RSM_NONE:
                break;
            case RSM_BFS_MONOLITHIC:
            case RSM_BFS_MONOLITHIC_COV:
                cout << "Using BFS with monolithic NSF." << endl;
                build_NSF = true;
                break;
            case RSM_SAT_MONOLITHIC:
                cout << "Using saturation with monolithic NSF." << endl;
                build_NSF = true;
                break;
            case RSM_SAT_PREGEN:
                cout << "Using per-event saturation (sat-pregen)." << endl;
                build_NSF = true;
                break;
            case RSM_BFS_IMPLICIT:
            case RSM_BFS_IMPLICIT_COV:
                cout << "Using BFS with implicit NSF." << endl;
                break;
            case RSM_SAT_IMPLICIT:
                cout << "Using saturation with implicit NSF (sat-impl)." << endl;
                break;
            case RSM_SAT_OTF:
                cout << "Using saturation with on-the-fly NSF generation (sat-otf)." << endl;
                break;
        }
        if (fast_NSF_gen && build_NSF)
            cout << "Using fast NSF generation." << endl;
    }
    else cout << endl;

    compose_name(net_name, argv[1], "");
    /*strcpy(net_name,argv[1]);*/
    initialize_global_variables();

    /* LETTURA DEL FILE .def E CREAZIONE DEL .net CON SOSTITUZIONE DELLE MACRO */
    compose_name(net, argv[1], "def");
    parse_DEF = FALSE;
    read_DEF_file();                          /* Conteggio delle definizioni */
    create_table();

    /*DOT
    *open file dot
    */
    if (dot_flag) {
        compose_name(net, argv[1], "dot");
        f_dot = efopen(net, "w");
        fprintf(f_dot, "digraph RG {");
    }
    /*DOT
    *open file dot
    */

    /*AUTOMA
    open output file*/
    if (AUTOMA) {
        compose_name(net, argv[1], "outT");
        if ((f_outState = fopen(net, "w")) == NULL) {
            printf("AUTOMA OUTPUT:  file:'%s cannot be opened' \n", net);
            exit(EXIT_FAILURE);
        }
    }
    /*AUTOMA*/

    /*SMARTexport*/
    if (SMART) {
        compose_name(net, argv[1], "sm");
        f_smart = efopen(net, "w");
        fprintf(f_smart, "# MaxNumericalIters %d\n# NumericalPrecision %f \n\n", _iter, _prec);
        fprintf(f_smart, "spn PN := {\n");
    }
    /*SMARTexport*/
    /*MDWN
    *open file MDWN
    */
#ifdef SWN
    if (MDWN_flag) {
        compose_name(net, argv[1], "mdwn");
        f_MDWN = fopen(net, "w");
    }
#endif
    /*MDWN
    *open file MDWN
    */

    /*MDWN
    *open file cvrs
    */
#ifdef SWN
    if (cvrs_flag) {
        compose_name(net, argv[1], "cvrs");
        f_cvrs = efopen(net, "w");
        //fprintf(f_cvrs,"%d\n",ncl);
        compose_name(net, argv[1], "cvrsoff");
        f_cvrsoff = efopen(net, "w");
        //fprintf(f_cvrs,"%d\n",ncl);
    }
#endif
    /*MDWN
    *open file cvrs
    */

    // compose_name(net, argv[1], "mfp");
    // mfp = efopen(net, "w") ;


//     compose_name(net, argv[1], "net2");
//     nfp2 = efopen(net, "w") ;

// #ifdef SWN
//     parse_DEF = COLOR_CLASS;
//     read_DEF_file();                          // Parsificazione delle definizioni delle classi
//     fill_color_class_data_structure();        // OPERAZIONI
//     parse_DEF = FUNCTION;
//     read_DEF_file();                          // Parsificazione delle definizioni delle classi
//     parse_DEF = DYNAMIC_SUBCLASS;
//     read_DEF_file();                          // Parsificazione delle definizioni delle classi
// #ifdef SYMBOLIC
//     create_dynamic_subclass_data_structure(); // OPERAZIONI
//     fill_dynamic_subclass_data_structure();   // OPERAZIONI
// #endif
// #endif

//     fclose(nfp2);


    /* LETTURA DEL FILE .def E CREAZIONE DEL .net CON SOSTITUZIONE DELLE MACRO */

    /* LETTURA DEL FILE .net E DELLA MARCATURA INIZIALE */

// #ifdef GREATSPN
//     sprintf(comm, "/bin/cat \"%s.net2\" \"%s.net\" > \"%s.ntp\"", argv[1], argv[1], argv[1]);
// #else
//     sprintf(comm, "/bin/cat \"nets/%s.net2\" \"nets/%s.net\" > \"nets/%s.ntp\"", argv[1], argv[1], argv[1]);
// #endif
//     system(comm);
//     remove_command(comm, argv[1], "net2");
//     system(comm);

// #ifdef GREATSPN
//     sprintf(comm, "cpp \"%s.ntp\" > \"%s.par\"", argv[1], argv[1]);
// #else
//     sprintf(comm, "cpp \"nets/%s.ntp\" > \"nets/%s.par\"", argv[1], argv[1]);
// #endif
//     system(comm);
//     remove_command(comm, argv[1], "ntp");
//     system(comm);

    // compose_name(net, argv[1], "stat");
    // ok = ((pinfp = fopen(net, "r")) != NULL) ;
    // compose_name(net, argv[1], "dis");
    // disok = ((disfp = fopen(net, "r")) != NULL) ;



    read_NET_file(FALSE /* read <netname>.net directly */);


    parse_DEF = MDRATE;
    read_DEF_file();                          /* OPERAZIONI */
    // initialize_free_event_list();

    // if (g_var_order_sel.heuristics == VOC_PINV) {
    //     ret_read_pin_file = read_PIN_file(npl);
    //     if (ret_read_pin_file != 0) {
    //         fprintf(stdout, "Error in read_PIN_file.. you should generate P-invariant\n");
    //         exit(EXIT_FAILURE);
    //     }
    // }

    // If the input problem is an ILCP, add the slack variables
    if (ilcp_model) {
        ilcp_add_slack_variables_to_model();
        if (ilcp_num_lin_dep_constr > 0)
            ilcp_add_lin_dep_constr(ilcp_num_lin_dep_constr);
    }

    /* DIMENSIONAMENTO DELLE STRUTTURE DATI DEL PRODOTTO CARTESIANO */
    // create_cartesian_product_data_structure();
    /* DIMENSIONAMENTO DELLE STRUTTURE DATI DEL PRODOTTO CARTESIANO */

#ifdef SIMULATION
#ifndef SYMBOLIC
    /* LETTURA FILE .dis PER DISTRIBUZIONI GENERALI */
    //read_transition_general_distributions(disok);     /* OPERAZIONI */
    /* LETTURA FILE .dis PER DISTRIBUZIONI GENERALI */
#endif
#endif

#ifdef SWN
#ifdef SIMULATION
    // fill_preselection_rate_field();
#endif
#endif
    /* CREAZIONE STRUTTURE DATI PER LE STATISTICHE E LETTURA FILE .stat */
    // create_statistics_data_structure(ok);     /* OPERAZIONI */
    /* CREAZIONE STRUTTURE DATI PER LE STATISTICHE E LETTURA FILE .stat */


    parse_DEF = INITIAL_MARKING;
    read_DEF_file();                          /* Parsificazione delle definizioni delle classi */
    // fclose(mfp);
    // compose_name(net, argv[1], "mfp");
    // mfp = efopen(net, "r");
    create_marking_data_structure();    /* OPERAZIONI */
    fill_marking_data_structure();           /* OPERAZIONI */

    // fclose(mfp);
    // if (ok)
    //     fclose(pinfp);
    // if (disok)
    //     fclose(disfp);
    // remove_command(comm, argv[1], "mfp");
    // system(comm);
    // remove_command(comm, argv[1], "par");
    // system(comm);

    // Net name & basic info.
    cout << "Process ID: " << getpid() << endl;
    cout << "MODEL NAME: " << argv[1] << endl;
    cout << "  " << npl << " places, " << ntr << " transitions." << endl;

    // LETTURA DEL FILE .net E DELLA MARCATURA INIZIALE

    // CALCOLO RELAZIONI cc E sc E SCRITTURA  SUI RELATIVI FILE 
    /*compose_name(net, argv[1], "cc");
    cfp = efopen(net, "w");
    compose_name(net, argv[1], "sc");
    sfp = efopen(net, "w");

    get_CC_SC_relations(); 

    fclose(cfp);
    fclose(sfp);

    // CALCOLO RELAZIONI cc E sc E SCRITTURA  SUI RELATIVI FILE 
    // LETTURA DEI FILE .sc,.cc,.me E CREAZIONE DELLE RELATIVE STRUTTURE DATI 
    compose_name(net, argv[1], "cc");
    cfp = efopen(net, "r");
    compose_name(net, argv[1], "sc");
    sfp = efopen(net, "r");

    fill_CC_SC_data_structure(FALSE);


    fclose(cfp);
    fclose(sfp);*/

    /* LETTURA DEI FILE .sc,.cc,.me E CREAZIONE DELLE RELATIVE STRUTTURE DATI */
#ifdef DSDR
    N_STAZ = tabc[0].card;
    M_DIST = net_mark[5].total / N_STAZ;
#endif
#ifdef SYMBOLIC
    initialize_split_data_structure();
    initialize_merge_data_structure();
#endif
#ifdef SIMULATION
    init_events(); /* OPERAZIONI */
    CODICE OLD: non usare forward_sim in questo modo.
    forward_sim(argv[1], first_tr_l, tr_l, min_b, max_b, appr, conf_l); /* OPERAZIONI */
#endif
#ifdef REACHABILITY
    if (output_flag) {
        /* Creazione del file srg */
        compose_name(net, argv[1], "srg");
#ifdef SWN
        {
            char str[MAXSTRING];
            for (ii = 0; ii < ncl ; ii++) {
                sprintf(str, "%s%d", tabc[ii].col_name, tabc[ii].card);
                strcat(net, str);
            }
        }
#endif
        srg = efopen(net, "w") ;
    }/* Creazione del file srg */
    if (out_mc) {
        /* Opening of files for Markov Chain construction */


        compose_name(net, argv[1], "rht");
        rht = efopen(net, "w") ;
        fclose(rht);
        compose_name(net, argv[1], "rht");
        rht = efopen(net, "r+") ;

        compose_name(net, argv[1], "van_path");
        van_path = efopen(net, "w") ;
        if (fast_solve) {
            fclose(van_path);
            compose_name(net, argv[1], "van_path");
            van_path = efopen(net, "r+") ;
        }
        compose_name(net, argv[1], "denom");
        denom = efopen(net, "w") ;

        compose_name(net, argv[1], "throu");
        throu = efopen(net, "w") ;

#ifdef DEBUG_THROUGHPUT
        compose_name(net, argv[1], "thr");
        thr = efopen(net, "w") ;
        compose_name(net, argv[1], "vnum");
        vnum = efopen(net, "w") ;
        compose_name(net, argv[1], "arc");
        arc = efopen(net, "w") ;
#endif
        compose_name(net, argv[1], "wngr");
        wngr = efopen(net, "w") ;
        compose_name(net, argv[1], "wngrR");
        wngrR = efopen(net, "w");
        compose_name(net, argv[1], "rgr_aux");
        rgr_aux = efopen(net, "w") ;
        compose_name(net, argv[1], "ctrs");
        ctrs = efopen(net, "w") ;
        compose_name(net, argv[1], "grg");
        grg = efopen(net, "w") ;
    }/* Opening of files for Markov Chain construction */

/* NOT USED BY RGMEDD
#ifdef ESYMBOLIC
    if (!read_files) {
#endif

#ifndef CTMC
        compose_name(net, argv[1], "mark");
        mark = efopen(net, "w") ;
        fclose(mark);
#endif

        compose_name(net, argv[1], "mark");
        mark = efopen(net, "r+") ;

#ifdef ESYMBOLIC
    }
    else {
        compose_name(net, argv[1], "mark");
        mark = efopen(net, "r");
    }

#endif

    compose_name(net, argv[1], "outtype");
    outtype = efopen(net, "w") ;
#ifdef SYMBOLIC
    fprintf(outtype, "1");
#endif
#ifdef COLOURED
    fprintf(outtype, "0");
#endif
    fclose(outtype);
    */

#ifdef SYMBOLIC
    compose_name(net, argv[1], "cap");
    f_cap_fp = efopen(net, "w") ;
    fclose(f_cap_fp);
    f_cap_fp = efopen(net, "r+") ;
    compose_name(net, argv[1], "val");
    value_fp = efopen(net, "w") ;
    fclose(value_fp);
    value_fp = efopen(net, "r+") ;
    compose_name(net, argv[1], "minval");
    min_value_fp = efopen(net, "w") ;
    fclose(min_value_fp);
    min_value_fp = efopen(net, "r+") ;


#ifdef ESYMBOLIC
#ifdef LIBSPOT

// EXCEPTION ();
// output_flag=TRUE;

    /** creating temprary file for intermediar event ****/
    compose_name(net, argv[1], "tmp_event");
    TEMP_EVENT     = fopen(net, "w+");
    fclose(TEMP_EVENT);
    TEMP_EVENT     = fopen(net, "r+");
    /***************************************************/
#endif


    /*********** For ESRG *************/
    if (output_flag || out_mc) {
        /**** creating text out put file ******/
        compose_name(net, argv[1], "esrg");
        OUTPUT_FILE     = fopen(net, "w+");
        fclose(OUTPUT_FILE);
        OUTPUT_FILE     = fopen(net, "r+");
    }

    if (out_mc) {
        /**** creating text out put file ******/
        compose_name(net, argv[1], "off_sr");
        OFFSET_SR    = fopen(net, "w+");
        fclose(OFFSET_SR);
        OFFSET_SR     = fopen(net, "r+");

        compose_name(net, argv[1], "off_ev");
        OFFSET_EVENT  = fopen(net, "w+");
        fclose(OFFSET_EVENT);
        OFFSET_EVENT     = fopen(net, "r+");

        compose_name(net, argv[1], "cmst");
        STATISTICS  = fopen(net, "w+");
        fclose(STATISTICS);
        STATISTICS  = fopen(net, "r+");

    }
    /***** creating file of eventualties ******/
    compose_name(net, argv[1], "event");

    if (!read_files) {
#ifndef CTMC
        EVENT_MARK  = efopen(net, "w+");
        fclose(EVENT_MARK);
#endif
        EVENT_MARK  = efopen(net, "r+") ;

    }
    else
        EVENT_MARK   = efopen(net, "r+");
    /*************************************/

#endif

#endif
#endif

#ifdef LIBSPOT
    MY_INIT_ALLOC();
#ifdef ESYMBOLIC
    INIT_GLOBAL_VARS();

    if (!read_files) {
        if (output_flag) {
            /**** creating text out put file ******/
            compose_name(net, argv[1], "dsrg");
            OUTPUT_FILE     = fopen(net, "w+");
            fclose(OUTPUT_FILE);
            OUTPUT_FILE     = fopen(net, "r+");
        }

#ifndef LIBMCDSRG
        FILE *prop;
        compose_name(net, argv[1], "prop");

        if ((prop = fopen(net, "r+")) == NULL) {
            printf("\n There is no file:'%s' \n", net);
            exit(1);
        }
        else if (prop_parser(prop) != 0)
            fclose(prop);

#endif
    }

    FILE *guard;
    compose_name(net, argv[1], "gd");

    if ((guard = fopen(net, "r+")) != NULL) {
        (void)guard_parser(guard);
        fclose(guard);
    }

#endif

#ifdef ESYMBOLIC
    if (!read_files) {
#endif


        pState M0 = malloc(sizeof(State));
        initial_state(M0);
        free(M0);

#ifdef ESYMBOLIC
    }
#endif




#endif


#ifdef LIBSPOT
#undef exit
#endif
    return 0;
} /* End Initialize */


int finalize(void) {

#ifdef ESYMBOLIC
    if (output_flag || out_mc)
        fclose(OUTPUT_FILE);
    if (out_mc) {
        fclose(OFFSET_SR);
        fclose(OFFSET_EVENT);
    }
#ifdef LIBSPOT
    fclose(TEMP_EVENT);
#endif
#endif

#ifdef REACHABILITY
    if (output_flag)
#ifndef LIBSPOT
        fclose(srg);
#endif
    if (out_mc) {
#ifdef DEBUG_THROUGHPUT
        fclose(thr);
        fclose(vnum);
        fclose(arc);
#endif
        fclose(rgr_aux);
        fclose(wngr);
        fclose(wngrR);
        fclose(rht);
        fclose(throu);
        fclose(van_path);
        fclose(denom);
        fclose(ctrs);
        fclose(grg);
    }
    //fclose(mark);
#ifdef SYMBOLIC
    fclose(f_cap_fp);
    fclose(value_fp);
    fclose(min_value_fp);
#endif
#endif


    /*DOT
    *close file dot
    */
    if (dot_flag) {
        fprintf(f_dot, "\n}");
        fclose(f_dot);
    }
    /*DOT
    *close file dot
    */

    /*AUTOMA
    close output file*/
    if (AUTOMA) {
        fclose(f_outState);
    }
    /*AUTOMA
    close output file*/

    /*MDWN
    *close file MDWN
    */
#ifdef SWN
    if (MDWN_flag) {
        fclose(f_MDWN);
    }
#endif
    /*MDWN
    *close file cvrs
    */

    /*MDWN
    *close file cvrs
    */
#ifdef SWN
    if (cvrs_flag) {
        fclose(f_cvrs);
        fclose(f_cvrsoff);
    }
#endif
    /*MDWN
    *close file cvrs
    */

    return 0;
} /* end finalize */


void export_smart(FILE *f_smart) {
    int ii;
//places
    if (npl > 0) {
        fprintf(f_smart, "place %s", tabp[0].place_name);
    }
    for (ii = 1; ii < npl; ii++) {
        fprintf(f_smart, ",%s", tabp[ii].place_name);
    }
    fprintf(f_smart, ";\n");
//places
//trans
    if (ntr > 0) {
        fprintf(f_smart, "trans %s", tabt[0].trans_name);
    }
    for (ii = 1; ii < ntr; ii++) {
        fprintf(f_smart, ",%s", tabt[ii].trans_name);
    }
    fprintf(f_smart, ";\n");
//trans
//arcs
//trans
    bool first = true;
    if (ntr > 0) {
        fprintf(f_smart, "arcs (");

        Node_p tmp;
        tmp = tabt[0].inptr;
        while (tmp != NULL) {
            if (first) {
                fprintf(f_smart, " %s:%s:%d", tabp[(*tmp).place_no].place_name, tabt[0].trans_name, (*tmp).molt);
                first = false;
            }
            else
                fprintf(f_smart, ", %s:%s:%d", tabp[(*tmp).place_no].place_name, tabt[0].trans_name, (*tmp).molt);
            tmp = tmp->next;
        }
        tmp = tabt[0].outptr;
        while (tmp != NULL) {
            if (first) {
                fprintf(f_smart, " %s:%s:%d", tabt[0].trans_name, tabp[(*tmp).place_no].place_name, (*tmp).molt);
                first = false;
            }
            else
                fprintf(f_smart, ", %s:%s:%d", tabt[0].trans_name, tabp[(*tmp).place_no].place_name, (*tmp).molt);
            tmp = tmp->next;
        }
    }
    for (ii = 1; ii < ntr; ii++) {
        Node_p tmp;
        tmp = tabt[ii].inptr;
        while (tmp != NULL) {
            if (first) {
                fprintf(f_smart, " %s:%s:%d", tabp[(*tmp).place_no].place_name, tabt[ii].trans_name, (*tmp).molt);
                first = false;
            }
            else
                fprintf(f_smart, ", %s:%s:%d", tabp[(*tmp).place_no].place_name, tabt[ii].trans_name, (*tmp).molt);
            tmp = tmp->next;
        }
        tmp = tabt[ii].outptr;
        while (tmp != NULL) {
            if (first) {
                fprintf(f_smart, " %s:%s:%d", tabt[ii].trans_name, tabp[(*tmp).place_no].place_name, (*tmp).molt);
                first = false;
            }
            else
                fprintf(f_smart, ", %s:%s:%d", tabt[ii].trans_name, tabp[(*tmp).place_no].place_name, (*tmp).molt);
            tmp = tmp->next;
        }
    }
    fprintf(f_smart, ");\n");
//trans

//init
    first = true;
    for (ii = 0; ii < npl; ii++) {
        if (first) {
            if (tabp[ii].position > 0) {
                fprintf(f_smart, "init (%s:%d", tabp[ii].place_name, tabp[ii].position);
                first = false;
            }
        }
        else {
            if (tabp[ii].position > 0)
                fprintf(f_smart, ", %s:%d", tabp[ii].place_name, tabp[ii].position);
        }
    }
    fprintf(f_smart, ");\n");
//init

//firing
    if (ntr > 0) {
        fprintf(f_smart, "firing ( (%s:expo(%f))", tabt[0].trans_name, tabt[0].mean_t);
    }
    for (ii = 1; ii < ntr; ii++) {
        fprintf(f_smart, ", (%s:expo(%f))", tabt[ii].trans_name, tabt[ii].mean_t);
    }
//firing
    fprintf(f_smart, ");\n");
    for (ii = 0; ii < npl; ii++) {
        fprintf(f_smart, "real m%d := avg_ss(tk(%s));\n", ii, tabp[ii].place_name);
    }
    int for_step = (_max - _min) / _step;
    double increm = _min;
    for (int j = 0; j <= for_step; j++) {
        for (ii = 0; ii < npl; ii++) {
            fprintf(f_smart, "real t%dX%d := avg_at(tk(%s),%f);\n", ii, j, tabp[ii].place_name, increm);
        }
        increm = increm + _step;
    }

    fprintf(f_smart, "};\n");
    for (ii = 0; ii < npl; ii++) {
        fprintf(f_smart, "print(\"%s = \",PN.m%d,\"\\n\");\n", tabp[ii].place_name, ii);
    }

    fprintf(f_smart, "print(\"-------------------------------------------\\n\");\n");
    increm = _min;
    fprintf(f_smart, "print(\"Time\\t");
    for (ii = 0; ii < npl; ii++) {
        fprintf(f_smart, "%s\\t", tabp[ii].place_name);
    }
    fprintf(f_smart, "\\n\");\n");
    for (int j = 0; j <= for_step; j++) {
        fprintf(f_smart, "print(\"%f\\t", increm);
        for (ii = 0; ii < npl; ii++) {
            //fprintf(f_smart,"print(\"%s T(%f) = \",PN.t%dX%d,\"\\n\");\n", tabp[ii].place_name,increm,ii,j);
            fprintf(f_smart, "\",PN.t%dX%d,\"\\t", ii, j);
        }
        fprintf(f_smart, "\\n\");\n");
        increm = increm + _step;
    }
}

/**************************************************************/

// Write the generated variable order of RGMEDD to a file, in various formats
void save_variable_order(RSRG& rs) {
    // Save in the format expected by RGMEDD, i.e. a list of place names
    if (g_save_var_order_for_rgmedd) {
        char fname[MAXSTRING];
        snprintf(fname, MAXSTRING, "%splace", net_name);
        FILE* out = fopen(fname, "w");

        // Build the inverse relation
        int rel[npl];
        for (size_t i=0; i<npl; i++)
            rel[rs.convertPlaceToMDDLevel(i)] = i;

        // Write the relation to file
        for (size_t i=0; i<npl; i++)
            fprintf(out, "%s ", tabp[rel[i]].place_name);
        fprintf(out, "\n");
        fclose(out);

        printf("Variable order in RGMEDD format saved to %s\n", fname);
    }

    // ltsmin format: a comma separated list of place indices
    if (g_save_var_order_for_ltsmin) {
        char fname[MAXSTRING];
        snprintf(fname, MAXSTRING, "%sltsmin-colperm", net_name);
        FILE* out = fopen(fname, "w");

        // Build the inverse relation
        int rel[npl];
        for (size_t i=0; i<npl; i++)
            rel[rs.convertPlaceToMDDLevel(i)] = i;

        // Write the relation to file
        for (size_t i=0; i<npl; i++)
            fprintf(out, "%s%d", i==0?"":",", rel[i]);
        fprintf(out, "\n");
        fclose(out);

        printf("Variable order in ltsmin format saved to %s\n", fname);
    }
}

/**************************************************************/

// // parse a cardinality written as a double-like number
// cardinality_t parse_card_exp(const char* str) {
//     mpf_class ff;
//     mpf_t *out = reinterpret_cast<mpf_t*>(&ff);
//     cout << "str="<<str<<flush;
//     gmp_sscanf(str, "%Fe", *out);
//     cout << " out="<<ff<<flush;
//     cardinality_t n;
//     // mpf_trunc(*out, *out);
//     mpz_set_f(cardinality_ref(n), *out);
//     cout << " n="<<n<<endl;
//     return n;
// }

/**************************************************************/

void load_expected_results(RSRG& rs) {
    char fname[MAXSTRING], buf[MAXSTRING];
    snprintf(fname, MAXSTRING, "%sexpected", net_name);

    // cout << parse_card_exp("1234") << endl;
    // cout << parse_card_exp("12.34") << endl;
    // cout << parse_card_exp("1.234e03") << endl;
    // cout << parse_card_exp("1.234567E+06") << endl;
    // cout << parse_card_exp("1.234567E+07") << endl;
    // cout << parse_card_exp("1.234567E+80") << endl;
    // cout << parse_card_exp("1.234567E+2") << endl;

    ifstream ifs(fname);
    if (ifs) { // expected results for statespace
        // read line by line
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.size() < MAXSTRING) {
                char name[MAXSTRING], type, value[MAXSTRING];
                if (3 == sscanf(line.c_str(), "%s %c %s", name, &type, value)) {
                    // cout << name << " " << type << " " << value << endl;

                    bool good = true;
                    result_t r;
                    switch (type) {
                        case 'I':
                            if (0==strcmp(value, "inf"))
                                r = result_t{(ssize_t)INFINITE_CARD};
                            else
                                r = result_t{(ssize_t)atoi(value)};
                            break;

                        case 'D':
                            if (0==strcmp(value, "inf"))
                                r = result_t{(double)INFINITE_CARD};
                            else {
                                double parsed = atof(value);
                                r = result_t{parsed};
                                if (isinf(parsed) || isnan(parsed))
                                    good = false;
                            }
                            break;

                        case 'B':
                            r = result_t{value[0]=='T'};
                            break;

                        default:
                            throw rgmedd_exception("Unknown expected result type.");
                    }

                    if (good) {
                        // cout << "EXPECTED: " << r << " (parsed " << value <<  ") for " << name << endl;
                        rs.expected_results.insert(make_pair(name, r));
                    }
                }
            }
        }
    }
}

/**************************************************************/

void print_banner(const char* title) {
    if (running_for_MCC())
        return;
    int len = strlen(title);
    const int ROW = 79;
    for (int i=0; i<(ROW-len)/2; i++)
        cout << "=";
    cout << title;
    for (int i=0; i<(ROW-len+1)/2; i++)
        cout << "=";
    cout << endl;
}

/**************************************************************/

int main(int argc, char **argv) {
    initialize(argc, argv);

    if (!running_for_MCC()) {
        int who = RUSAGE_SELF;
        struct rusage usage;
        getrusage(who, &usage);
        cout << "Used Memory for encoding net: " << usage.ru_maxrss << "KB" << endl;

        if (SMART) {
            export_smart(f_smart);
            fclose(f_smart);
            return 0;
        }
    }

    bool failed_regressions = false;
    RSRG rs;
    try {
        rs.setFastNSFGen(fast_NSF_gen);
        rs.printGuessedBounds(print_guessed_bounds);
        rs.initialize(initRsMethod, initLrsMethod, g_var_order_sel, 
                        meddly_cache_size, bound_policy);
        if (ctl_name > 0)
            rs.setPropName(std::string(argv[ctl_name]));

        load_expected_results(rs);
        save_variable_order(rs);
        failed_regressions = build_graph(rs);
    }
    catch (std::exception& obj) {
        cerr << "ERROR: " << obj.what() << endl << endl;
        if (running_for_MCC() && !is_child_subprocess() && !CTL)
            cout << "CANNOT_COMPUTE" << endl;
        exit(EXIT_FAILURE_RGMEDD);
    }
    catch (MEDDLY::error& obj) {
        cerr << "MEDDLY ERROR: " << obj.getName() << endl << endl;
        if (running_for_MCC() && !is_child_subprocess() && !CTL)
            cout << "CANNOT_COMPUTE" << endl;
        exit(EXIT_FAILURE_MEDDLY);
    }
    catch (...) {
        cerr << "INTERNAL ERROR." << endl << endl;
        if (running_for_MCC() && !is_child_subprocess() && !CTL)
            cout << "CANNOT_COMPUTE" << endl;
        exit(EXIT_FAILURE_GENERIC);            
    }
#if DEBUG
        rs.statistic();
#endif
    finalize();

    if (failed_regressions) {
        cout << "Some regression test failed." << endl;
        return EXIT_FAILURE_REGRESSION;
    }
    return EXIT_SUCCESS;
}


