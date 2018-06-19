#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sys/resource.h>

extern "C" {


#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/decl.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/ealloc.h"



    extern int belong(int,  Node_p);

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

#endif
    extern void lexer_set_buffer(const char *b);
    extern int parser();
    extern int get_class_card(int);
    extern int already_exist(char *, int, int);
    extern int intersection(Static_p, int, int);
    extern int read_DEF_file() ;
    extern int read_NET_file(int read_postproc);
//MARIETA
    extern int read_PIN_file(int);
//MARIETA
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
    int dot_flag = 0;
    FILE *f_dot;
    char bname[MAX_TAG_SIZE];
    bool MASSACTION = false;
    bool MATLAB = false;
    bool AUTOMATON = false;
    bool R = false;
    bool P = false;
    bool GPU=false;
    std::string TRANS_PATH;
    std::string OBJ_PATH;
}


using namespace std;
extern void build_ODE(ofstream &out, string path, string net);
extern void build_ODEM(ofstream &out, string net);
extern void build_ODER(ofstream &out, string net);
extern void build_ODEOPT(ofstream &out, string net, string transitions, string objective_function);
extern void build_ODEGPU(string net);
int Max_Token_Bound = 255; //max number of different tokens for places

/****************************************************/
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

static int from_GUI = -1;

int invoked_from_gui() { // Is invoked from the new Java-based GUI?

    const char *env;

    if (from_GUI == -1) { // Not yet determined

        env = getenv("FROM_GUI");

        from_GUI = (env != NULL && 0 == strcmp(env, "1"));

    }

    return (from_GUI != 0);

}

static int gui_stat_flag = 0;

extern "C" int print_stat_for_gui() {

    return (invoked_from_gui() && gui_stat_flag);

}

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
    int tr1, tr2, num_cc, num_sc, trans_cc[MAX_TRANS], trans_sc[MAX_TRANS];
    int int1, int2;

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
}/* End get_CC_SC_relations */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_CC_SC_data_structure(int  ok_me) {
    /* Init fill_CC_SC_data_structure */
    int *usp;
    int ii, jj, na, nc;
    char trans_nm[MAX_TAG_SIZE];

    for (ii = 0; ii < ntr; ii++) {
        fscanf(cfp, " %d", &na);
        tabt[ii].add_l = usp = (int *)ecalloc(na + 1, sizeof(int));
        *(usp += na) = UNKNOWN;
        while (na--) {
            fscanf(cfp, "%d", &jj);
            *(--usp) = jj - 1;
        }
        while (getc(cfp) != '\n');
    }
    for (ii = 0; ii < ntr; ii++) {
        fscanf(sfp, "%d", &nc);
        tabt[ii].test_l = usp = (int *)ecalloc(nc + 1, sizeof(int));
        *(usp += nc) = UNKNOWN;
        while (nc--) {
            fscanf(sfp, "%d", &jj);
            *(--usp) = jj - 1;
        }
        while (getc(sfp) != '\n');
    }
    for (ii = 0 ; ii < ntr ; ii++)
        tabt[ii].me_l = NULL;
    if (ok_me) {
        /* Lettura file di me */
        int ind;

        while (TRUE) {
            fscanf(pinfp, " %s", trans_nm);
            fscanf(pinfp, " %d", &na);
            ind = get_tr_in(trans_nm);
            if (ind == UNKNOWN)
                out_error(ERROR_UNKNOWN_TRANSITION_NAME, 0, 0, 0, 0, trans_nm, NULL);
            tabt[ind].me_l = usp = (int *)ecalloc(na + 1, sizeof(int));
            *(usp += na) = UNKNOWN;
            while (na--) {
                fscanf(pinfp, "%s", trans_nm);
                ind = get_tr_in(trans_nm);
                if (ind == UNKNOWN)
                    out_error(ERROR_UNKNOWN_TRANSITION_NAME, 0, 0, 0, 0, trans_nm, NULL);
                *(--usp) = ind;
            }
            getc(pinfp);
            if (feof(pinfp))
                break;
        }
    }/* Lettura file di me */
}/* End fill_CC_SC_data_structure */
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
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
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
            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
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
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
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
            if (IS_UNORDERED(i) || (IS_ORDERED(i) && GET_STATIC_SUBCLASS(i) == 1))
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
            if (IS_UNORDERED(i) || (IS_ORDERED(i) && GET_STATIC_SUBCLASS(i) == 1))
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


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int initialize(int  argc,  char  *argv[]) {
    char net[MAXSTRING], comm[MAXSTRING];

    int ok, disok;
    int ii;
    char cc;




#ifdef SIMULATION
    int first_tr_l = TRANS_LENGTH;
    int tr_l = TRANS_LENGTH;
    int min_b = MIN_BATCH_LENGTH;
    int max_b = MAX_BATCH_LENGTH;
    int appr = APPROX_DEGREE;
    int conf_l = CONF_LEVEL;
    int old_conf_l = 95;
#endif

#ifdef LIBSPOT
#define exit return
#endif



    if (argc < 2) {
//automaton
        std::cerr << "\n\nUSE: PN2ODE <path/net_name> [option]\n\tOption:\n\t\t-R ->\tExport R file\n\t\t-M ->\tGenelarized Mass Action policy\n\t\t-I ->\tInfinity servers policy (default)\n\t\t-O ->\tExport in Matlab format\n\n\n\t\t-A ->\tAutomaton verification";
        exit(EXIT_FAILURE);
//automaton
    }

    for (ii = 2; ii < argc; ii++) {
        /* Lettura delle opzioni */
        cc = argv[ii][1];

        switch (cc) {

        /* Casistica */
        case 'M' :
            MASSACTION = true;
            break;

        case 'I' :
            MASSACTION = false;
            break;

        case 'A' :
            AUTOMATON = true;
            break;

        case 'O' :
            MATLAB = true;
            break;

        case 'G' :
            GPU = true;
            break;

        case 'R' :
            R = true;
            break;

        case 'P' :
            P = true;
            break;

        case 'T' :
            if(argv[ii+1]!=NULL)
                TRANS_PATH = argv[ii+1];
            else
                exit(1);
            break;

        case 'F' :
             if(argv[ii+1]!=NULL)
                OBJ_PATH = argv[ii+1];
            else
                exit(1);
            break;

        default :
            if(argv[2][1] == 'P'){
                P = true;
                if(argv[4]!=NULL){
                    if(argv[4][1]== 'M')
                        MASSACTION = true;
                }
            }
            else
                {
                fprintf(stderr, "Unknown option %s\n", argv[ii]);
                exit(1);
                }
        }/* Casistica */
    }/* Lettura delle opzioni */



    compose_name(net_name, argv[1], "");
    /*strcpy(net_name,argv[1]);*/
    initialize_global_variables();

    /* LETTURA DEL FILE .def E CREAZIONE DEL .net CON SOSTITUZIONE DELLE MACRO */
    compose_name(net, argv[1], "def");
    parse_DEF = FALSE;
    read_DEF_file();			   /* Conteggio delle definizioni */
    create_table();



    // compose_name(net, argv[1], "mfp");
    // mfp = efopen(net, "w") ;
    compose_name(net, argv[1], "net2");
    nfp2 = efopen(net, "w") ;

#ifdef SWN
    parse_DEF = COLOR_CLASS;
    read_DEF_file();			   /* Parsificazione delle definizioni delle classi */
    fill_color_class_data_structure();	   /* OPERAZIONI */
    parse_DEF = FUNCTION;
    read_DEF_file();			   /* Parsificazione delle definizioni delle classi */
    parse_DEF = DYNAMIC_SUBCLASS;
    read_DEF_file();			   /* Parsificazione delle definizioni delle classi */
#ifdef SYMBOLIC
    create_dynamic_subclass_data_structure(); /* OPERAZIONI */
    fill_dynamic_subclass_data_structure();   /* OPERAZIONI */
#endif
#endif

    fclose(nfp2);


    /* LETTURA DEL FILE .def E CREAZIONE DEL .net CON SOSTITUZIONE DELLE MACRO */

    /* LETTURA DEL FILE .net E DELLA MARCATURA INIZIALE */

#ifdef GREATSPN
    sprintf(comm, "/bin/cat \"%s.net2\" \"%s.net\" > \"%s.ntp\"", argv[1], argv[1], argv[1]);
#else
    sprintf(comm, "/bin/cat \"nets/%s.net2\" \"nets/%s.net\" > \"nets/%s.ntp\"", argv[1], argv[1], argv[1]);
#endif
    system(comm);
    remove_command(comm, argv[1], "net2");
    system(comm);
#ifdef GREATSPN
    sprintf(comm, "cpp \"%s.ntp\" > \"%s.par\"", argv[1], argv[1]);
#else
    sprintf(comm, "cpp \"nets/%s.ntp\" > \"nets/%s.par\"", argv[1], argv[1]);
#endif
    system(comm);
    remove_command(comm, argv[1], "ntp");
    system(comm);

    compose_name(net, argv[1], "stat");
    ok = ((pinfp = fopen(net, "r")) != NULL) ;
    compose_name(net, argv[1], "dis");
    disok = ((disfp = fopen(net, "r")) != NULL) ;



    read_NET_file(TRUE /* read post-processed <netname>.par */);

    parse_DEF = MDRATE;
    read_DEF_file();			   /* OPERAZIONI */
    initialize_free_event_list();



    /* DIMENSIONAMENTO DELLE STRUTTURE DATI DEL PRODOTTO CARTESIANO */
    create_cartesian_product_data_structure();
    /* DIMENSIONAMENTO DELLE STRUTTURE DATI DEL PRODOTTO CARTESIANO */

#ifdef SIMULATION
#ifndef SYMBOLIC
    /* LETTURA FILE .dis PER DISTRIBUZIONI GENERALI */
    read_transition_general_distributions(disok);	   /* OPERAZIONI */
    /* LETTURA FILE .dis PER DISTRIBUZIONI GENERALI */
#endif
#endif

#ifdef SWN
#ifdef SIMULATION
    fill_preselection_rate_field();
#endif
#endif



    parse_DEF = INITIAL_MARKING;
    read_DEF_file();			   /* Parsificazione delle definizioni delle classi */
    // fclose(mfp);
    // compose_name(net, argv[1], "mfp");
    // mfp = efopen(net, "r");
    create_marking_data_structure();    /* OPERAZIONI */
    fill_marking_data_structure();	     /* OPERAZIONI */

    // fclose(mfp);
    if (ok)
        fclose(pinfp);
    if (disok)
        fclose(disfp);
    // remove_command(comm, argv[1], "mfp");
    // system(comm);
    remove_command(comm, argv[1], "par");
    system(comm);

    /* LETTURA DEL FILE .net E DELLA MARCATURA INIZIALE */

    /* CALCOLO RELAZIONI cc E sc E SCRITTURA  SUI RELATIVI FILE */

    compose_name(net, argv[1], "cc");
    cfp = efopen(net, "w");
    compose_name(net, argv[1], "sc");
    sfp = efopen(net, "w");

    get_CC_SC_relations();	      /* OPERAZIONI */

    fclose(cfp);
    fclose(sfp);

    /* CALCOLO RELAZIONI cc E sc E SCRITTURA  SUI RELATIVI FILE */

    /* LETTURA DEI FILE .sc,.cc,.me E CREAZIONE DELLE RELATIVE STRUTTURE DATI */

    compose_name(net, argv[1], "cc");
    cfp = efopen(net, "r");
    compose_name(net, argv[1], "sc");
    sfp = efopen(net, "r");

    fill_CC_SC_data_structure(FALSE);    /* OPERAZIONI */


    fclose(cfp);
    fclose(sfp);

    return 0;
} /* End Initialize */


int finalize(void) {

    return 0;
} /* end finalize */



int main(int argc, char **argv) {

    cout << "\n\n =========================================================\n";
    cout << "|	      	        PN to ODE        	          |\n";
    cout << " =========================================================\n";
    cout << "\n If you find any bug, send an email to beccuti@di.unito.it!\n";
    initialize(argc, argv);
    string net(argv[1]);
    //creo file .R

    if (!GPU){
    if (R)
        net = net + ".R";
    else if (P)
        net = net + ".opt.R";
    else if (MATLAB)
        net = net + ".m";
    else
        net = net + ".cpp";
    ofstream out(net.c_str());

    if (!out) {
        cerr << "Error: it is not possible to open output file\n\n";
        exit(EXIT_FAILURE);
    }

    char *pch;
    string path(argv[1]);
    pch = strtok(argv[1], "/.");
    while (pch != NULL) {
        pch = strtok(NULL, "/.");
        if (pch != NULL)
            net = string(pch);
    }

    if (R)
        build_ODER(out, net);

    else if (P){
        build_ODEOPT(out, net, TRANS_PATH, OBJ_PATH);
        }

    else if (MATLAB)
        build_ODEM(out, net);
    else
        build_ODE(out, path, net); 				    /* OPERAZIONI */

    out.close();
    }
    else{
    build_ODEGPU(net);
    }


    finalize();
    return 0;
}



