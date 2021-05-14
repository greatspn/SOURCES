

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/service.h"

#ifdef LIBSPOT
#include "../../INCLUDE/gspnlib.h"
#include "../../INCLUDE/SDECLSPOT.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#endif

#ifdef REACHABILITY

extern Throughput_p fill_throughput_node();
extern MRate_p fill_mark_node();
extern PComp_p fill_factor();
extern  void store_compact();
extern  void load_compact();
extern  void store_double();
extern  void load_double();

extern void initialize_en_list();
extern void update_en_list();
extern void fire_trans();
extern void push_result();
extern void write_ctrs();
extern void write_grg();
extern void write_on_srg();
extern void write_final_results();
extern void update_rg_files();
extern void out_cur_marking();
extern void out_single_instance();
extern void string_to_marking();
extern double get_instance_rate();
extern unsigned long marking_to_string();

extern MRate_p rate_pop();
extern PComp_p path_pop();
extern void push_path_element();
extern void push_path_list();

extern Event_p get_new_event();
extern int get_max_place_domain();


/******************************* For ESRG *************************************/
/**********new version ********************/
extern MATRIX  *MATRIX_3D_ALLOCATION();
extern MATRIX **MATRIX_4D_ALLOCATION();
extern MATRIX   MATRIX_ALLOCATION();
extern void     TEST_EMMEDIATE();
extern int    **MAP_MERGING_ALLOCATION();
extern INT_LISTP **MAP_DECOMPOSING_ALLOCATION();
extern STORE_STATICS_ARRAY CREATE_STORE_STATIC_STRUCTURE();
extern int ** *CREATE_STORE_CARD_STRUCTURE();
extern void   STORE_CARD();
extern void   POP_STATIC_CONF();
extern void   NEW_SIM_STATIC_CONF();
extern void   STORE_STATIC_CONF();
extern TO_MERGEP   *TO_MERGE();
extern void   GROUPING_ALL_STATICS();
extern void   GET_SYMETRIC_MARKING();
extern void   ALL_MY_GROUP();
extern void   AFTER_CANONISATION();
extern void   GET_EVENT_FROM_MARKING();
extern void   NEW_ASYM_MARKING();
extern int    SYM_SAT_2_NOT_EXIST();
extern int    SYM_SAT_2_EXIST();
extern int    ASYM_2_NOT_EXIST_SYM();
extern int    ASYM_2_EXIST_SYM();
extern int    GET_MAX_CARD();
extern void   MARQUAGE();
extern void   FREE_EVENTUALITIES();
extern Tree_Esrg_p MY_TREE_POP();
extern Cart_p create_cartesian_product();
extern STORE_STATICS_ARRAY NewTabc();

#ifdef LIBSPOT
extern void       POP_STATIC_CONF();
extern TO_MERGEP *ToMergArr();
extern void       INIT_ARRAYS();
extern void       EVENTUALITIES();
extern PART_MAT_P GetPartitions();
extern int        Satisfaction();
extern TYPE_P   **GetMatRepOfClParts();
extern char         ** *GetMatRepOfClObj();
extern Tree_Esrg_p TEST_EXISTENCE();
extern void        GROUP_MARKINGS();

extern void       FreeStoreStructs();
extern void       FreePartMAt();
extern void       FreeMerg();
extern void       FreeStTabc();
extern void       Free_DSC_SSC();
extern void       FREE_ALL_LMS_ELEMS();
extern void       LIBERER_ELEM();
extern char cache_string [];

#endif
/*********************************************/
static Tree_p bottom_sat = NULL;
static Tree_p top_sat = NULL;
static Tree_p top_ins = NULL;
static Tree_p bottom_ins = NULL;
static Tree_p *roots_arry = NULL;
static int    Nb_roots_arry = 0;
static int    MARKING_COUNTER = -1;
static int    VANISHING_COUNT = 0;
static int    PILE = 0;
static int    POSSIBLE_DOUBLE_REP = 0;

#ifdef LIBSPOT
static int   NB_SR = 0;
static int   NOT_YET = true;
TO_MERGEP *MERG_ALL  = NULL;
STORE_STATICS_ARRAY Sym_StaticConf = NULL;
int stop = false;
#endif
/****************************************************************/


/*************************************************************************/
static Tree_p fire_ptr = NULL;	 /* Puntatore alla marcatura di sparo	 */
static Tree_p top = NULL;	 /* Puntatori al top e al bottom dello	 */
static Tree_p bottom = NULL;	 /* Stack di marcature da espandere	 */
static Tree_p root = NULL;	 /* Albero delle marcature raggiunte	 */



static int err_fseek;

Tree_p reached_marking = NULL;
Tree_p initial_marking = NULL;
Tree_p current_marking = NULL;

Result_p enabled_head = NULL;	/* Puntatori alla lista delle tr. abil. */
int COUNTER = 1;
int tro;			/* 1 marc. gia' raggiunta, 0 altrimenti */
int marcatura = 0;	/* contatore per le marcature		*/
int h = 0;			/* per bilanciamento nella insert_tree	*/
unsigned long tang = 0;		/* contatori tipi di marc. raggiunte	*/
unsigned long evan = 0;
unsigned long dead = 0;
int home = 0;
unsigned long cont_tang;
int count_arc = 0;
#ifdef SYMBOLIC
extern void create_canonical_data_structure();
extern void get_canonical_marking();

double mark_ordinarie;	    /* num. marc. ord. per marc. simbolica  */
double ord_tang = 0.0;
double ord_evan = 0.0;
double ord_dead = 0.0;

extern Canonic_p sfl_h;
#endif

int cur_priority;

int *code_place = NULL;
int *min_place = NULL;
int *max_place = NULL;
int *init_place = NULL;

int max_priority = 0;
int old_time, new_time;

static MRate_p path_head_ptr = NULL; static unsigned long c_ph = 0;
static MRate_p path_tail_ptr = NULL;

static MRate_p tangible_path_head_ptr = NULL; static unsigned long c_tph = 0;
static MRate_p tangible_path_tail_ptr = NULL;

static Throughput_p throu_head_ptr = NULL; static unsigned long c_th = 0;
static Throughput_p throu_tail_ptr = NULL;

static Throughput_p total_throu_head_ptr = NULL; static unsigned long c_tth = 0;
static Throughput_p total_throu_tail_ptr = NULL;

static unsigned long in_tr;
static unsigned long in_enabling_degree;
static unsigned long in_ordinary;
static unsigned long in_denom_p;


static unsigned long tot_path = 0;

unsigned long d_ptr;
unsigned long length;
unsigned long f_mark;
int f_bot;
int f_throu;
int f_tang;

extern int out_mc;
extern int exp_set;
extern int fast_solve;


int exceeded_markings_bound() { return FALSE; }

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void arrcpy(s, t, lim)
int *s;
int *t;
int lim;
{
    /* Init arrcpy */
    int i;

    for (i = lim ; i ; i--)
        s[i - 1] = t[i - 1];
}/* End arrcpy */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static unsigned long append_to_list_of_throu(head, tail, add_head, add_tail)
Throughput_p *head;
Throughput_p *tail;
Throughput_p add_head;
Throughput_p add_tail;
{
    /* Init append_to_list_of_throu */
    Throughput_p list = NULL;
    Throughput_p cur = NULL;
    Throughput_p nxt = NULL;
    int found = FALSE;
    unsigned long appended = 0;

    if (*head == NULL && *tail == NULL) {
        *head = add_head;
        *tail = add_tail;
        for (cur = add_head; cur != NULL; cur = cur->next, appended++);
    }
    else {
        if (add_head != NULL && add_tail != NULL) {
            if (exp_set) {
                /* Opzione per set di esperimenti */
                (*tail)->next = add_head;
                *tail = add_tail;
                for (cur = add_head; cur != NULL; cur = cur->next, appended++);
            }/* Opzione per set di esperimenti */
            if (!exp_set) {
                /* Opzione per unico run */
                for (cur = add_head; cur != NULL; cur = nxt) {
                    /* Per ogni elemento da aggiungere */
                    nxt = cur->next;
                    found = FALSE;
                    for (list = *head; list != NULL; list = list->next)
                        if (cur->tr == list->tr) {
                            found = TRUE;
                            list->weight += cur->weight;
                            push_throu_element(cur);
                            goto finish;
                        }
finish:   if (found == FALSE) {
                        (*tail)->next = cur;
                        *tail = cur;
                        (*tail)->next = NULL;
                        appended++;
                    }
                }/* Per ogni elemento da aggiungere */
            }/* Opzione per unico run */
        }
    }
    if (*tail != NULL)
        (*tail)->next = NULL;
    return (appended);
}/* End append_to_list_of_throu */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static unsigned long append_to_list_of_tangible_reachable(head, tail, add_head, add_tail)
MRate_p *head;
MRate_p *tail;
MRate_p add_head;
MRate_p add_tail;
{
    /* Init append_to_list_of_tangible_reachable */
    MRate_p list = NULL;
    MRate_p cur = NULL;
    MRate_p nxt = NULL;
    int found = FALSE;
    unsigned long appended = 0;

    if (*head == NULL && *tail == NULL) {
        *head = add_head;
        *tail = add_tail;
        for (cur = add_head; cur != NULL; cur = cur->next, appended++);
    }
    else {
        if (add_head != NULL && add_tail != NULL) {
            if (exp_set) {
                /* Opzione per set di esperimenti */
                (*tail)->next = add_head;
                *tail = add_tail;
                for (cur = add_head; cur != NULL; cur = cur->next, appended++);
            }/* Opzione per set di esperimenti */
            if (!exp_set) {
                /* Opzione per unico run */
                for (cur = add_head; cur != NULL; cur = nxt) {
                    /* Per ogni elemento da aggiungere */
                    nxt = cur->next;
                    found = FALSE;
                    for (list = *head; list != NULL; list = list->next)
                        if (cur->cont_tang == list->cont_tang && cur->flag == list->flag) {
                            found = TRUE;
                            list->mean_t += cur->mean_t;
                            push_rate_element(cur);
                            goto finish;
                        }
finish:   if (found == FALSE) {
                        (*tail)->next = cur;
                        *tail = cur;
                        (*tail)->next = NULL;
                        appended++;
                    }
                }/* Per ogni elemento da aggiungere */
            }/* Opzione per unico run */
        }
    }
    if (*tail != NULL)
        (*tail)->next = NULL;
    return (appended);
}/* End append_to_list_of_tangible_reachable */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void dispose_all_instances(res_ptr)
Result_p res_ptr;
{
    /* Init dispose_all_instances */
    Event_p ev_p, nev_p;

    if (res_ptr != NULL) {
        /* Puntatore consistente */
        ev_p = res_ptr->list;
        while (ev_p != NULL) {
            nev_p = ev_p->next;
            dispose_old_event(ev_p);
            ev_p = nev_p;
        }
    }/* Puntatore consistente */
}/* End dispose_all_instances */
/* ********************************************************************* */
/*                                                                       */
/*    Funzioni che permettono il calcolo dei tassi associati alle        */
/*    transizioni abilitate                                              */
/*                                                                       */
/* ********************************************************************* */
static double normalizzazione(pun, pri)
Result_p pun;
int pri;
{
    Result_p tmp = pun;
    double prod;
    double den = 0;
    Event_p ev_p = NULL;
    int tr;
    int tot = 0;

    while (tmp != NULL) {
        /* Per ogni transizione */
        ev_p = tmp->list;
        prod = 0;
        tr = GET_TRANSITION_INDEX(ev_p);
        if (tabt[tr].pri == pri) {
            /* Se sono allo stesso livello di priorita' */
            while (ev_p != NULL) {
                /* Per ogni istanza */
                tot++;
                tr = GET_TRANSITION_INDEX(ev_p);
                prod += get_instance_rate(ev_p) * GET_ENABLING_DEGREE(ev_p);
                ev_p = NEXT_EVENT(ev_p);
            }/* Per ogni istanza */
            den += prod;
        }/* Se sono allo stesso livello di priorita' */
        tmp = tmp->next;
    }/* Per ogni transizione */
    if (exp_set) {
        /* Opzione per set di esperimenti */
        store_compact(tot, denom);
        while (pun != NULL) {
            /* Per ogni transizione */
            ev_p = pun->list;
            tr = GET_TRANSITION_INDEX(ev_p);
            if (tabt[tr].pri == pri) {
                /* Se sono allo stesso livello di priorita' */
                while (ev_p != NULL) {
                    /* Per ogni istanza */
                    store_compact(ev_p->trans, denom);
#ifdef SWN
#ifdef SYMBOLIC
                    store_compact(ev_p->ordinary_instances, denom);
#endif
#endif
                    ev_p = NEXT_EVENT(ev_p);
                }/* Per ogni istanza */
            }/* Se sono allo stesso livello di priorita' */
            pun = pun->next;
        }/* Per ogni transizione */
    }/* Opzione per set di esperimenti */
    return (den);
}
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void garbage_collect(fire_ptr)
Tree_p fire_ptr;
{
    /* Init garbage_collect */
    Result_p current_transition, next_transition;

    current_transition = fire_ptr->enabled_head;
    while (current_transition != NULL) {
        next_transition = current_transition->next;
        dispose_all_instances(current_transition);
        current_transition->list = NULL;
        push_result(current_transition);
        current_transition = next_transition;
    }
    fire_ptr->enabled_head = NULL;
}/* End garbage_collect */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void DFS_evanescenti(int  f_mark) {
    /* Init DFS_evanescenti */
    char cc;
    int tr;
    int marking_pri;

    Result_p current_transition;

    Event_p ev_p, nev_p, copy_of_ev_p;

    Tree_p new_reached_marking;
    Tree_p dfs_fire_ptr;
    PComp_p comp_p = NULL;
    MRate_p pun = NULL;
    MRate_p cur = NULL;
    MRate_p marc_ragg = NULL;
    MRate_p head_marc = NULL; unsigned long  c_hm = 0;
    MRate_p tail_marc = NULL;
    Throughput_p pun_throu = NULL;
    Throughput_p cur_throu = NULL;
    Throughput_p throu_ragg = NULL;
    Throughput_p head_throu = NULL; unsigned long c_ht = 0;
    Throughput_p tail_throu = NULL;
    int tot_p = 0;
    int temp_p ;

    int nmark;
    int nk;
    int st;
    double ra;
    int tnmark;
    int tnk, nfatt;
    int tst;
    int en_degree = 1;
    int ordinary_m = 1;
    unsigned long denom_p = 0;
    double tra;
    double dval;


    double denomin, numer, mean_t;

    dfs_fire_ptr = new_reached_marking = reached_marking;
    marking_pri = dfs_fire_ptr->marking->pri;

    if (IS_VANISHING(marking_pri) && out_mc) {
        denom_p = ftell(denom);
        denomin = normalizzazione(dfs_fire_ptr->enabled_head, marking_pri);
    }
    current_transition = dfs_fire_ptr->enabled_head;
    while (current_transition != NULL) {
        /* per tutte le transizioni abilitate */
        tr = GET_TRANSITION_INDEX(current_transition->list);
        if (tabt[tr].pri == marking_pri) {
            /* Solo per la stessa priorita' */
            copy_of_ev_p = get_new_event(tr);
            ev_p = current_transition->list;
            while (ev_p != NULL) {
                /* per ogni istanza */
                nev_p = ev_p->next;
                if (output_flag) {
                    /* Scrittura abilitata */
                    update_rg_files(TANGIBLE_OR_VANISHING, dfs_fire_ptr, tr, marking_pri);
                }/* Scrittura abilitata */
                if (out_mc) {
                    /* Construction of Markov Chain */
                    if (ev_p != NULL) {
                        /*en_degree = ev_p->enabling_degree ;*/
#ifdef SWN
#ifdef SYMBOLIC
                        ordinary_m = ev_p->ordinary_instances;
#endif
#endif
                    }
                    numer = en_degree *  ordinary_m * get_instance_rate(ev_p);
                    if (IS_VANISHING(marking_pri))
                        mean_t = numer / denomin;
                    else
                        mean_t = numer;
                }/* Construction of Markov Chain */
                copy_event(copy_of_ev_p, ev_p);
                if (IS_RESET_TRANSITION(tr))
#ifdef SWN
#ifdef SYMBOLIC
                    reset_to_M0(initial_marking->marking->marking_as_string, initial_marking->marking->d_ptr, initial_marking->marking->length, tr);
#endif
#ifdef COLOURED
                reset_to_M0(initial_marking->marking->marking_as_string, initial_marking->marking->length, tr);
#endif
#endif
#ifdef GSPN
                reset_to_M0(initial_marking->marking->marking_as_string, initial_marking->marking->length, tr);
#endif
                else
                    fire_trans(ev_p);
                if (output_flag) {
                    /* Scrittura abilitata */
                    out_single_instance(ev_p, srg);
                    if (out_mc) {
                        /* Construction of Markov Chain */
                        fprintf(srg, "--->   (rate %lg)\n", mean_t);
                    }/* Construction of Markov Chain */
                }/* Scrittura abilitata */
#ifdef SWN
#ifdef SYMBOLIC
                get_canonical_marking();
#endif
#endif
                f_mark = marking_to_string();
                tro = 0;
                insert_tree(&root, &h, f_mark, length, d_ptr); /* Piu' info nel symbolic */
                if (current_marking == initial_marking && !home)
                    home = 1;
                switch (tro) {
                /* Vari tipi di marcatura raggiunta */
                case DEAD_OLD:
                    if (output_flag) {
                        /* Scrittura abilitata */
                        update_rg_files(NORMAL, NULL, UNKNOWN, -1);
                    }/* Scrittura abilitata */
                    if (out_mc) {
                        /* Construction of Markov Chain */
                        marc_ragg = fill_mark_node(FALSE, cont_tang, mean_t);
                        if (exp_set) {
                            /* Opzione per set di esperimenti */
                            comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                            marc_ragg->path = comp_p;
                            marc_ragg->cnt++;
                        }/* Opzione per set di esperimenti */
                        c_hm += append_to_list_of_tangible_reachable(&head_marc, &tail_marc, marc_ragg, marc_ragg);
                        if (tabt[tr].tagged) {
                            throu_ragg = fill_throughput_node(tr, mean_t);
                            if (exp_set) {
                                /* Opzione per set di esperimenti */
                                comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                throu_ragg->path = comp_p;
                                throu_ragg->cnt++;
                            }/* Opzione per set di esperimenti */
                            c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_ragg, throu_ragg);
                        }
                    }/* Construction of Markov Chain */
                    break;
                case VANISHING_OLD:
                    if (output_flag) {
                        /* Scrittura abilitata */
                        update_rg_files(NORMAL, NULL, UNKNOWN, 1);
                    }/* Scrittura abilitata */
                    if (out_mc) {
                        /* Construction of Markov Chain */
                        if (fast_solve) {
                            /* Opzione per soluzione veloce ma files grossi */
                            err_fseek = fseek(van_path, reached_marking->marking->path, 0);
                            load_compact(&nmark, van_path);
                            for (nk = 1; nk <= nmark; nk++) {
                                load_compact(&st, van_path);
                                load_double(&ra, van_path);
                                marc_ragg = fill_mark_node(FALSE, st, ra * mean_t);
                                c_hm += append_to_list_of_tangible_reachable(&head_marc, &tail_marc, marc_ragg, marc_ragg);
                            }
                        }/* Opzione per soluzione veloce ma files grossi */
                        else {
                            /* Opzione per soluzione lenta ma files contenuti */
                            marc_ragg = fill_mark_node(TRUE, reached_marking->marking->path, mean_t);
                            if (exp_set) {
                                /* Opzione per set di esperimenti */
                                comp_p = fill_factor(tr, ordinary_m, en_degree, denom_p);
                                marc_ragg->path = comp_p;
                                marc_ragg->cnt++;
                            }/* Opzione per set di esperimenti */
                            c_hm += append_to_list_of_tangible_reachable(&head_marc, &tail_marc, marc_ragg, marc_ragg);
                        }/* Opzione per soluzione lenta ma files contenuti */
                        err_fseek = fseek(rht, reached_marking->marking->throu, 0);
                        load_compact(&tnmark, rht);
                        for (tnk = 1; tnk <= tnmark; tnk++) {
                            load_compact(&tst, rht);
                            if (!exp_set) {
                                /* Opzione per unico run */
                                load_double(&tra, rht);
                            }/* Opzione per unico run */
                            throu_ragg = fill_throughput_node(tst, tra * mean_t);
                            if (exp_set) {
                                /* Opzione per set di esperimenti */
                                load_compact(&nfatt, rht);
                                for (; nfatt ; nfatt--) {
                                    load_compact(&in_tr, rht);
                                    in_enabling_degree = 1;
#ifdef SWN
#ifdef SYMBOLIC
                                    load_compact(&in_ordinary, rht);
#endif
#endif
                                    load_compact(&in_denom_p, rht);
                                    comp_p = fill_factor(in_tr, in_ordinary, in_enabling_degree, in_denom_p);
                                    comp_p->next = throu_ragg->path;
                                    throu_ragg->path = comp_p;
                                    throu_ragg->cnt++;
                                }
                                comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                comp_p->next = throu_ragg->path;
                                throu_ragg->path = comp_p;
                                throu_ragg->cnt++;
                            }/* Opzione per set di esperimenti */
                            c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_ragg, throu_ragg);
                        }
                        if (tabt[tr].tagged) {
                            throu_ragg = fill_throughput_node(tr, mean_t);
                            if (exp_set) {
                                /* Opzione per set di esperimenti */
                                comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                throu_ragg->path = comp_p;
                                throu_ragg->cnt++;
                            }/* Opzione per set di esperimenti */
                            c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_ragg, throu_ragg);
                        }
                    }/* Construction of Markov Chain */

                    break;
                case TANGIBLE_OLD:
                    if (output_flag) {
                        /* Scrittura abilitata */
                        update_rg_files(NORMAL, NULL, UNKNOWN, 0);
                    }/* Scrittura abilitata */
                    if (out_mc) {
                        /* Construction of Markov Chain */
                        marc_ragg = fill_mark_node(FALSE, cont_tang, mean_t);
                        if (exp_set) {
                            /* Opzione per set di esperimenti */
                            comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                            marc_ragg->path = comp_p;
                            marc_ragg->cnt++;
                        }/* Opzione per set di esperimenti */
                        c_hm += append_to_list_of_tangible_reachable(&head_marc, &tail_marc, marc_ragg, marc_ragg);
                        if (tabt[tr].tagged) {
                            throu_ragg = fill_throughput_node(tr, mean_t);
                            if (exp_set) {
                                /* Opzione per set di esperimenti */
                                comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                throu_ragg->path = comp_p;
                                throu_ragg->cnt++;
                            }/* Opzione per set di esperimenti */
                            c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_ragg, throu_ragg);
                        }
                    }/* Construction of Markov Chain */
                    break;
                case VANISHING_LOOP:
                    fprintf(stdout, "error : Vanishing loop for marking:\n");
                    fprintf(stdout, "***********************************\n");
                    out_cur_marking(stdout);
                    exit(1);
                    break;
                case NEW_MARKING:
                    enabled_head = NULL;
#ifdef SWN
#ifdef SYMBOLIC
                    string_to_marking(reached_marking->marking->marking_as_string, reached_marking->marking->d_ptr, reached_marking->marking->length);
#endif
#endif
                    update_en_list(ev_p, dfs_fire_ptr->enabled_head);
                    if (enabled_head == NULL) {
                        /* Dead marking */
                        dead++;
                        tang++;
                        inqueue_stack(&top, &bottom, reached_marking);
                        reached_marking->marking->cont_tang = tang;
                        reached_marking->marking->pri = -1;
                        reached_marking->enabled_head = enabled_head;
#ifdef SWN
#ifdef SYMBOLIC
                        ord_dead += mark_ordinarie;
#endif
#endif
                        if (output_flag)
                            update_rg_files(DEAD, NULL, UNKNOWN, UNKNOWN);
                        if (out_mc) {
                            /* Construction of Markov Chain */
                            marc_ragg = fill_mark_node(FALSE, tang, mean_t);
                            if (exp_set) {
                                /* Opzione per set di esperimenti */
                                comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                marc_ragg->path = comp_p;
                                marc_ragg->cnt++;
                            }/* Opzione per set di esperimenti */
                            c_hm += append_to_list_of_tangible_reachable(&head_marc, &tail_marc, marc_ragg, marc_ragg);
                            if (tabt[tr].tagged) {
                                throu_ragg = fill_throughput_node(tr, mean_t);
                                if (exp_set) {
                                    /* Opzione per set di esperimenti */
                                    comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                    throu_ragg->path = comp_p;
                                    throu_ragg->cnt++;
                                }/* Opzione per set di esperimenti */
                                c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_ragg, throu_ragg);
                            }
                        }/* Construction of Markov Chain */
                    }/* Dead marking */
                    else {
                        /* Live marking */
                        reached_marking->marking->pri = cur_priority;
                        if (!cur_priority) {
                            /* Marcatura tangibile */
                            tang++;
#ifdef SWN
#ifdef SYMBOLIC
                            ord_tang += mark_ordinarie;
#endif
#endif
                            inqueue_stack(&top, &bottom, reached_marking);
                            reached_marking->enabled_head = enabled_head;
                            reached_marking->marking->cont_tang = tang;
                            if (output_flag) {
                                /* Scrittura abilitata */
                                update_rg_files(NORMAL, NULL, UNKNOWN, cur_priority);
                            }/* Scrittura abilitata */
                            if (out_mc) {
                                /* Construction of Markov Chain */
                                marc_ragg = fill_mark_node(FALSE, tang, mean_t);
                                if (exp_set) {
                                    /* Opzione per set di esperimenti */
                                    comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                    marc_ragg->path = comp_p;
                                    marc_ragg->cnt++;
                                }/* Opzione per set di esperimenti */
                                c_hm += append_to_list_of_tangible_reachable(&head_marc, &tail_marc, marc_ragg, marc_ragg);
                                if (tabt[tr].tagged) {
                                    throu_ragg = fill_throughput_node(tr, mean_t);
                                    if (exp_set) {
                                        /* Opzione per set di esperimenti */
                                        comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                        throu_ragg->path = comp_p;
                                        throu_ragg->cnt++;
                                    }/* Opzione per set di esperimenti */
                                    c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_ragg, throu_ragg);
                                }
                            }/* Construction of Markov Chain */
                        }/* Marcatura tangibile */
                        else {
                            /* Marcatura evanescente */
                            evan++;
#ifdef SWN
#ifdef SYMBOLIC
                            ord_evan += mark_ordinarie;
#endif
#endif
                            reached_marking->enabled_head = enabled_head;
                            reached_marking->marking->cont_tang = evan;
                            if (output_flag) {
                                /* Scrittura abilitata */
                                update_rg_files(NORMAL, NULL, UNKNOWN, cur_priority);
                            }/* Scrittura abilitata */
                            DFS_evanescenti(f_mark);
                            if (out_mc) {
                                /* Construction of Markov Chain */
                                pun = path_head_ptr;
                                while (pun != NULL) {
                                    if (fast_solve) {
                                        /* Opzione per soluzione veloce ma files grossi */
                                        pun->mean_t *= mean_t;
                                    }/* Opzione per soluzione veloce ma files grossi */
                                    else {
                                        /* Opzione per soluzione lenta ma files contenuti */
                                        if (exp_set) {
                                            /* Opzione per set di esperimenti */
                                            comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                            comp_p->next = pun->path;
                                            pun->path = comp_p;
                                            pun->cnt++;
                                        }/* Opzione per set di esperimenti */
                                        if (!exp_set) {
                                            /* Opzione per unico run */
                                            pun->mean_t *= mean_t;
                                        }/* Opzione per unico run */
                                    }/* Opzione per soluzione lenta ma files contenuti */
                                    pun = pun->next;
                                }
                                c_hm += append_to_list_of_tangible_reachable(&head_marc, &tail_marc, path_head_ptr, path_tail_ptr);
                                /*c_hm += c_ph;*/
                                path_head_ptr = NULL;
                                path_tail_ptr = NULL;
                                c_ph = 0;
                                pun_throu = throu_head_ptr;
                                while (pun_throu != NULL) {
                                    if (exp_set) {
                                        /* Opzione per set di esperimenti */
                                        comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                        comp_p->next = pun_throu->path;
                                        pun_throu->path = comp_p;
                                        pun_throu->cnt++;
                                    }/* Opzione per set di esperimenti */
                                    if (!exp_set) {
                                        /* Opzione per unico run */
                                        pun_throu->weight *= mean_t;
                                    }/* Opzione per unico run */
                                    pun_throu = pun_throu->next;
                                }
                                c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_head_ptr, throu_tail_ptr);
                                /*c_ht += c_th;*/
                                if (tabt[tr].tagged) {
                                    throu_ragg = fill_throughput_node(tr, mean_t);
                                    if (exp_set) {
                                        /* Opzione per set di esperimenti */
                                        comp_p = fill_factor(tr, ordinary_m, 1, denom_p);
                                        throu_ragg->path = comp_p;
                                        throu_ragg->cnt++;
                                    }/* Opzione per set di esperimenti */
                                    c_ht += append_to_list_of_throu(&head_throu, &tail_throu, throu_ragg, throu_ragg);
                                }
                                throu_head_ptr = NULL;
                                throu_tail_ptr = NULL;
                                c_th = 0;
                            }/* Construction of Markov Chain */
                        }/* Marcatura evanescente */
                    }/* Live marking */
                    break;
                }/* Vari tipi di marcatura raggiunta */
#ifdef GSPN
                string_to_marking(dfs_fire_ptr->marking->marking_as_string, UNKNOWN, dfs_fire_ptr->marking->length);
#endif
#ifdef SWN
#ifdef SYMBOLIC
                string_to_marking(dfs_fire_ptr->marking->marking_as_string, dfs_fire_ptr->marking->d_ptr, dfs_fire_ptr->marking->length);
#endif
#ifdef COLOURED
                string_to_marking(dfs_fire_ptr->marking->marking_as_string, UNKNOWN, dfs_fire_ptr->marking->length);
#endif
#endif
                ev_p = nev_p;
            }/* per ogni istanza */
            dispose_old_event(copy_of_ev_p);
        }/* Solo per la stessa priorita' */
        current_transition = current_transition->next;
    }/* Per tutte le transizioni abilitate */
    if (out_mc) {
        /* Construction of Markov Chain */
        if (new_reached_marking != NULL) {
            if (fast_solve) {
                new_reached_marking->marking->path = f_bot;
                err_fseek = fseek(van_path, f_bot, 0);
            }
            else
                new_reached_marking->marking->path = ftell(van_path);

            store_compact(c_hm, van_path);
            cur = head_marc;
            for (; cur != NULL ; cur = pun) {
                pun = cur->next;
                store_compact(cur->cont_tang, van_path);
                if (exp_set) {
                    /* Opzione per set di esperimenti */
                    store_compact(cur->cnt, van_path);
                    for (comp_p = cur->path; comp_p != NULL; comp_p = comp_p->next) {
                        if (cur->flag == FALSE)
                            store_compact(comp_p->fired_transition, van_path);
                        else
                            store_compact(ntr + comp_p->fired_transition, van_path);
#ifdef SWN
#ifdef SYMBOLIC
                        store_compact(comp_p->ordinary_m, van_path);
#endif
#endif
                        store_compact(comp_p->denominator, van_path);
                    }
                }/* Opzione per set di esperimenti */
                if (!exp_set) {
                    /* Opzione per unico run */
                    if (cur->flag == FALSE)
                        store_double(&(cur->mean_t), van_path);
                    else {
                        dval = -cur->mean_t ;
                        store_double(&dval, van_path);
                    }
                }/* Opzione per unico run */
            }
            c_ph += append_to_list_of_tangible_reachable(&path_head_ptr, &path_tail_ptr, head_marc, tail_marc);
            f_bot = ftell(van_path);
            /*c_ph += c_hm;*/

            head_marc = tail_marc = NULL;
            c_hm = 0;

            new_reached_marking->marking->throu = f_throu;
            err_fseek = fseek(rht, f_throu, 0);

            store_compact(c_ht, rht);
            cur_throu = head_throu;
            for (tnk = 1 ; tnk <= c_ht; tnk++, cur_throu = pun_throu) {
                pun_throu = cur_throu->next;
                store_compact(cur_throu->tr, rht);
                if (exp_set) {
                    /* Opzione per set di esperimenti */
                    store_compact(cur_throu->cnt, rht);
                    for (comp_p = cur_throu->path; comp_p != NULL; comp_p = comp_p->next) {
                        store_compact(comp_p->fired_transition, rht);
#ifdef SWN
#ifdef SYMBOLIC
                        store_compact(comp_p->ordinary_m, rht);
#endif
#endif
                        store_compact(comp_p->denominator, rht);
                    }
                }/* Opzione per set di esperimenti */
                if (!exp_set) {
                    /* Opzione per unico run */
                    store_double(&(cur_throu->weight), rht);
                }/* Opzione per unico run */
            }
            f_throu = ftell(rht);
            c_th += append_to_list_of_throu(&throu_head_ptr, &throu_tail_ptr, head_throu, tail_throu);
            /*c_th += c_ht;*/
            head_throu = tail_throu = NULL;
            c_ht = 0;
        }
    }/* Construction of Markov Chain */
    garbage_collect(dfs_fire_ptr);
}/* End DFS_evanescenti */


void inqueue(Tree_p *top, Tree_p *bottom,  Tree_p ins_node) {
    ins_node->last = NULL;
    if ((*top) == NULL)(*top) = (*bottom) = ins_node;
    else {
        (*bottom)->last = ins_node;
        (*bottom) = ins_node;
    }
}

void retreve(Tree_p *top, Tree_p *bottom,  Tree_p ins_node) {
    Tree_p Next = (*top), Curr;

    if (((*top) == ins_node) && ((*bottom) == ins_node)) { *top = *bottom = NULL;}
    else if ((*top) == ins_node) {*top = (*top)->last;}
    else {
        while (Next != ins_node) {Curr = Next; Next = Next->last;}
        if ((*bottom) == ins_node) {Curr->last = NULL; (*bottom) = Curr;}
        else {Curr->last = ins_node->last;}
    }
}

void  my_inqueue_stack(Tree_p ins_node, int cas) {
    switch (cas) {
    case INSERT_SAT :
        inqueue(&top_sat, &bottom_sat, ins_node);
        break;
    case INSERT_NO_SAT :
        inqueue(&top_ins, &bottom_ins, ins_node);

        break;

    case DEPLACE        :
        retreve(&top_ins, &bottom_ins, ins_node);
        inqueue(&top_sat, &bottom_sat, ins_node);

        break;
    }

}

void my_pop(Tree_p *top) {

    if (PILE == INSERT_SAT) {
        (*top) = top_sat = top_sat->last;
        if (!top_sat) {
            (*top) = top_ins;
            PILE = INSERT_NO_SAT;
        }
    }
    else {
        (*top) = top_ins = top_ins->last;
        if (top_sat) {
            (*top) = top_sat;
            PILE = INSERT_SAT;
        }
    }
}

static void my_garbage_collect(fire_ptr)
Tree_Esrg_p fire_ptr;
{
    /* Init garbage_collect */
    Result_p current_transition, next_transition;

    current_transition = fire_ptr->enabled_head;
    while (current_transition != NULL) {
        next_transition = current_transition->next;
        dispose_all_instances(current_transition);
        current_transition->list = NULL;
        push_result(current_transition);
        current_transition = next_transition;
    }
    fire_ptr->enabled_head = NULL;
}/* End garbage_collect */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
#ifndef LIBSPOT
int set_greatest_priority(Tree_p esm) {
    int priority = UNKNOWN;
    Tree_Esrg_p event = esm->Head_Next_Event;
    priority = esm->marking->pri;
    while (event) {

        //   if((event->marking->pri > priority)&&(event->enabled_head))
        if (event->marking->pri > priority)
            priority = event->marking->pri;
        event = event->Head_Next_Event;
    }
    return priority;
}
#endif

void SPECIAL_CANISATION() {
    SPEC_TRAI = 1;
    get_canonical_marking();
    f_mark = marking_to_string();
    string_to_marking(f_mark, d_ptr, length);
    SPEC_TRAI = 0;
}

void INIT_GLOBAL_VARS() {

    Counter          = create_cartesian_product(get_max_place_domain(), 0);
    Res              = create_cartesian_product(get_max_place_domain(), 0);


    MAX_CARD         = GET_MAX_CARD(tabc);
    NB_DSC           = MATRIX_ALLOCATION();
    NB_SSBC          = MATRIX_ALLOCATION();
    STATICS          = MATRIX_3D_ALLOCATION();
    DYNAMIC          = MATRIX_3D_ALLOCATION();
    RESULT           = MATRIX_4D_ALLOCATION();

    MERGING_MAPPE    = MAP_MERGING_ALLOCATION();
    DECOMP_MAPPE     = MAP_DECOMPOSING_ALLOCATION();
// STORED_CARD      = CREATE_STORE_CARD_STRUCTURE();
    STORED_CARD      = (int **) malloc(ncl * sizeof(int **));
    int i, j;
    for (i = 0 ; i < ncl ; i++) {
        STORED_CARD[i] = (int *)  malloc(MAX_CARD * sizeof(int *));
        for (j = 0; j < MAX_CARD ; j++)
            STORED_CARD[i][j] = (int)malloc(MAX_CARD * sizeof(int));
    }

#ifndef LIBSPOT
    ASYM_STATIC_STORE = CREATE_STORE_STATIC_STRUCTURE();
    SYM_STATIC_STORE = CREATE_STORE_STATIC_STRUCTURE();
    SYM_TOT          = (int *) ARRAY_ALLOCATION(ncl);
#else
    DynDis           = MATRIX_ALLOCATION();
    MTCL             = GetMatRepOfClObj();
    TEMP             = MATRIX_4D_ALLOCATION() ;
#endif
}
#ifndef LIBSPOT
void build_graph() {
    /* Init build_graph */
    char cc;
    int j;
    int tr;

    int first_fire;
    int marking_pri;
    int greatest_pri;

    Result_p current_transition;
    Result_p next_transition;

    Event_p ev_p, nev_p, copy_of_ev_p;

    Tree_p new_reached_marking;


    double denomin, numer, mean_t;

    int nmark;
    int nk;
    int st, nfatt;
    unsigned long en_degree = 1;
    unsigned long ordinary_m = 1;
    double ra;

    unsigned long tnmark;
    int tnk;
    int tst;
    double tra;
    double dval;
    tro              = 0;

    TO_MERGEP *merg  = NULL;
    int Index, i, CASES, ESM_COUNTER = 1;
    Tree_Esrg_p eventualities = NULL;
    Result_p enb_h_store;
    int DEAD_MARKING = 0;

    INIT_GLOBAL_VARS();

    EXCEPTION(); /** for Ordred class with more then 1 sub-class **/

    time(&old_time);
    if (output_flag)
        fprintf(OUTPUT_FILE, "digraph dtmc{\n");
    create_canonical_data_structure();

    /***************** different from the orginal version  *******************/
    SPECIAL_CANISATION();
    /************************************************************************/

    /************* The initial and the changed static config. *****/
    STORE_CARD(card, & STORED_CARD);
    STORE_STATIC_CONF(& ASYM_STATIC_STORE, tabc);
    merg = MERG = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(merg, tabc, num);
    NEW_SIM_STATIC_CONF(& SYM_STATIC_STORE);
    MY_INIT_ALLOC();
    /*********************************************************/

    /****************** Get the symetric marking ****************/
    GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num, &tot,
                         SYM_STATIC_STORE, merg);
    /***************************************************************/

    /************ Group and canonize the initial symetric marking ************/
    ALL_MY_GROUP(&MERGING_MAPPE);
    get_canonical_marking();
    AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);

    f_mark = marking_to_string();
    insert_tree(&root, &h, f_mark, length, d_ptr);
    root->marking->cont_tang = ESM_COUNTER; ESM_COUNTER++;
    if (out_mc)
        SR_EVENT_POS_STORE(OFFSET_SR, root, NULL);
    root->visited = 1;
    root->dead = 1;
    string_to_marking(root->marking->marking_as_string,
                      root->marking->d_ptr, root->marking->length);
    /****************************************************************************/


    /********************** Find the initial set of firing instances **********/
    int nb = 0;
    if ((nb = TEST_SYMETRIC_INITIAL(merg)) == 1) {
        MARKING_TYPE = SATURED_SYM;
        COMPT_SAT++;
        my_initialize_en_list();
        root->enabled_head = enabled_head;
        root->Marking_Type = SATURED_SYM;
        root->marking->pri = cur_priority;
    }
    else {
        root->Num_Event = nb - 1;
        root->Marking_Type = MARKING_TYPE = NO_SATURED_SYM;
        NO_SAT_INI = TRUE;
    }

    /******************* Geting the first and unique eventuality of the initial marking ****/
    GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE,
                           ASYM_STATIC_STORE, STORED_CARD, merg);
    /*****************************************************************************************/

    if (MARKING_TYPE == SATURED_SYM) {
        STORE_SYM_TOT(tot, SYM_TOT) ;
        /******** Write the Output file *****/
        //  if(output_flag) ESM_REACHED(OUTPUT_FILE,root, 0);
        my_inqueue_stack(root, INSERT_SAT);
        top = root;
        PILE = INSERT_SAT;
    }
    else {
        STORE_SYM_TOT(tot, SYM_TOT) ;
        /**** Write the output file ****/
        //  if(output_flag) ESM_REACHED(OUTPUT_FILE,root, 1);
        my_inqueue_stack(root, INSERT_NO_SAT);
        top = root;
        PILE = INSERT_NO_SAT;
    }

    /*********** Get the asymetric marking to compute the asymetric firing instances ****/
    NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot, merg,
                     ASYM_STATIC_STORE, RESULT);

    if (MARKING_TYPE == SATURED_SYM)
        MARKING_TYPE = SATURED_INS;
    else
        MARKING_TYPE = NO_SATURED_INS;

    enabled_head = NULL;
    my_initialize_en_list();
    COMPT_EVENT++;

    if (enabled_head) {
        EVENTUALITIE_TO_STRING();
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        eventualities = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
        eventualities->enabled_head = enabled_head;
        eventualities->Marking_Type = MARKING_TYPE;
        eventualities->marking->pri = cur_priority;
        eventualities->marking->cont_tang = COMPT_STORED_EVENT + 1;
        COMPT_STORED_EVENT++;
        if (out_mc)
            SR_EVENT_POS_STORE(OFFSET_EVENT, root, eventualities);

    }
    root->Head_Next_Event = root->NOT_Treated = eventualities;

    /*****************************************************************/


    do {

        fire_ptr = top;
        int active = 0;

        if (fire_ptr->Marking_Type == SATURED_SYM) {
            greatest_pri = set_greatest_priority(fire_ptr);
        }
        else
            greatest_pri = UNKNOWN;

        enb_h_store = fire_ptr->enabled_head;

        /************* Output for the lumpability check (LC) **********/
        if (out_mc) {
            int NB = Number_of_Instances(fire_ptr->enabled_head,
                                         NULL, greatest_pri , 0, 1);
            WRITE_HEAD_SR(OUTPUT_FILE, fire_ptr,
                          enb_h_store, greatest_pri, NB);
        }
        /**************************************************************/


        if (fire_ptr->Marking_Type == SATURED_SYM) {

            MARKING_TYPE = SATURED_SYM;
            POP_STATIC_CONF(SYM_STATIC_STORE, &tabc);
            string_to_marking(fire_ptr->marking->marking_as_string,
                              fire_ptr->marking->d_ptr,
                              fire_ptr->marking->length);

            current_transition = fire_ptr->enabled_head;

            if (output_flag) {
                PrintESM(OUTPUT_FILE, fire_ptr->marking->cont_tang);
            }

            while (current_transition != NULL) {
                next_transition = current_transition->next;

                tr = GET_TRANSITION_INDEX(current_transition->list);

                if (tabt[tr].pri == greatest_pri) {
                    active = 1;

                    fire_ptr->dead = 0;

                    copy_of_ev_p = get_new_event(tr);
                    ev_p = current_transition->list;

                    while (ev_p != NULL) {

                        nev_p = ev_p->next;
                        copy_event(copy_of_ev_p, ev_p);

                        if (IS_RESET_TRANSITION(tr))
                            reset_to_M0(initial_marking->marking->marking_as_string,
                                        initial_marking->marking->d_ptr,
                                        initial_marking->marking->length, tr);
                        else
                            fire_trans(ev_p);


                        if (out_mc)
                            WRITE_INSTANCE(OUTPUT_FILE, ev_p);
                        /*** writing of the firing instance for L.C. *****/

                        get_canonical_marking();

                        f_mark = marking_to_string();

                        tro = 0;
                        insert_tree(&root, &h, f_mark, length, d_ptr);

                        if (current_marking == initial_marking && !home)  home = 1;
                        string_to_marking(reached_marking->marking->marking_as_string,
                                          reached_marking->marking->d_ptr,
                                          reached_marking->marking->length);

                        if (tro == NEW_MARKING) {

                            reached_marking->marking->cont_tang = ESM_COUNTER;
                            ESM_COUNTER++;
                            reached_marking->visited = 1;
                            reached_marking->dead = 1;
                            if (out_mc)
                                SR_EVENT_POS_STORE(OFFSET_SR, reached_marking, NULL);
                        }


                        if (out_mc)
                            WRITE_REACHED_ESM(OUTPUT_FILE, reached_marking, NULL);
                        /*** writing of the Reached ESM for L.C. *****/

                        switch (tro) {
                        case NEW_MARKING   :
                            CASES = SYM_SAT_2_NOT_EXIST(reached_marking, fire_ptr, ev_p, merg);
                            break;
                        default            :
                            CASES = SYM_SAT_2_EXIST(reached_marking, fire_ptr, ev_p, merg);
                            break;
                        }

                        if (output_flag)
                            Print_Gen_Arc(OUTPUT_FILE, tr, fire_ptr->marking->cont_tang
                                          , reached_marking->marking->cont_tang);
                        /**** OUTPUT FILE ******/

                        if (CASES != NO_THINK)
                            my_inqueue_stack(reached_marking, CASES);


                        string_to_marking(fire_ptr->marking->marking_as_string,
                                          fire_ptr->marking->d_ptr,
                                          fire_ptr->marking->length);
                        ev_p = nev_p;
                    }
                    dispose_old_event(copy_of_ev_p);
                }
                current_transition = next_transition;
            }

            garbage_collect(fire_ptr);
        }

        eventualities = fire_ptr->NOT_Treated;

        /***************** count dead markings **************/
        if (fire_ptr->Marking_Type == SATURED_SYM &&
                fire_ptr->dead && !eventualities) {
            DEAD_MARKING += fire_ptr->Num_Event;
            //   printf("Number of the dead marking :%d \n",fire_ptr->marking->cont_tang);
        }
        /****************************************************/

        while (eventualities) {
            int dead = 1;
            int nb = 0;

            marking_pri = eventualities->marking->pri;

            if (out_mc) {
                if ((nb = Number_of_Instances(eventualities->enabled_head ,
                                              enb_h_store, marking_pri, greatest_pri, 2)) != 0)
                    WRITE_HEAD_EVENT(OUTPUT_FILE, fire_ptr, eventualities , nb);
            }

            /***************** count dead markings **************/
            if (!eventualities->enabled_head) {
                DEAD_MARKING ++;
                //	 printf("Number of dead marking :%d \n",fire_ptr->marking->cont_tang);
            }
            /****************************************************/

            if (eventualities->enabled_head) {
                /****************GET THE ASYMETRIC MARKING***************************/
                POP_STATIC_CONF(SYM_STATIC_STORE, & tabc);
                string_to_marking(fire_ptr->marking->marking_as_string,
                                  fire_ptr->marking->d_ptr, fire_ptr->marking->length);
                GET_EVENTUALITIE_FROM_FILE(eventualities->marking->marking_as_string,
                                           eventualities->marking->length);
                COPY_CACHE(eventualities->marking->length);
                STRING_TO_EVENTUALITIE();
                STORE_SYM_TOT(tot, SYM_TOT) ;
                NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot,
                                 merg, ASYM_STATIC_STORE, RESULT);


                if (output_flag && eventualities->asym_reach) {
                    PrintESMEVENT(OUTPUT_FILE, eventualities->marking->cont_tang,
                                  fire_ptr->marking->cont_tang);
                    PrintEvent(OUTPUT_FILE, eventualities->marking->cont_tang,
                               fire_ptr->marking->cont_tang)  ;
                }

                current_transition = eventualities->enabled_head;

                while (current_transition != NULL) {
                    next_transition = current_transition->next;
                    tr = GET_TRANSITION_INDEX(current_transition->list);

                    int v = IS_ASYMETRIC(tr);

                    if ((tabt[tr].pri == marking_pri && v) ||
                            (tabt[tr].pri == marking_pri && (!v) &&
                             ((enb_h_store == NULL) || (greatest_pri != marking_pri))
                            )
                       ) {

                        if (output_flag) {

                            if (!eventualities->asym_reach) {
                                PrintESMEVENT(OUTPUT_FILE, eventualities->marking->cont_tang,
                                              fire_ptr->marking->cont_tang);
                                eventualities->asym_reach = 1;
                            }

                            PrintEvent(OUTPUT_FILE, eventualities->marking->cont_tang,
                                       fire_ptr->marking->cont_tang)  ;
                        }

                        dead = 0;
                        copy_of_ev_p = get_new_event(tr);
                        ev_p = current_transition->list;
                        Index = 0;

                        while (ev_p != NULL) {

                            Index++;
                            nev_p = ev_p->next;
                            copy_event(copy_of_ev_p, ev_p);

                            if (IS_RESET_TRANSITION(tr))
                                reset_to_M0(initial_marking->marking->marking_as_string,
                                            initial_marking->marking->d_ptr,
                                            initial_marking->marking->length, tr);
                            else
                                fire_trans(ev_p);

                            /********** Writing firing instances *****************/
                            if (out_mc)
                                WRITE_INSTANCE(OUTPUT_FILE, ev_p);
                            /*****************************************************/
                            SPECIAL_CANISATION();


                            STORE_CARD(card, &STORED_CARD);
                            GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num,
                                                 &tot, SYM_STATIC_STORE, merg);
                            ALL_MY_GROUP(&MERGING_MAPPE);
                            get_canonical_marking();
                            AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);

                            f_mark = marking_to_string();

                            tro = 0;
                            insert_tree(&root, &h, f_mark, length, d_ptr);

                            if (current_marking == initial_marking && !home)  home = 1;
                            string_to_marking(reached_marking->marking->marking_as_string,
                                              reached_marking->marking->d_ptr,
                                              reached_marking->marking->length);

                            if (tro == NEW_MARKING) {


                                reached_marking->marking->cont_tang = ESM_COUNTER;
                                ESM_COUNTER++;
                                reached_marking->visited = 1;
                                reached_marking->dead = 1;

                                if (out_mc)
                                    SR_EVENT_POS_STORE(OFFSET_SR, reached_marking, NULL);
                            }

                            GET_EVENT_FROM_MARKING(tabc, tot, num, card,
                                                   MERGING_MAPPE,
                                                   ASYM_STATIC_STORE,
                                                   STORED_CARD, merg);

                            switch (tro) {
                            case NEW_MARKING   : CASES = ASYM_2_NOT_EXIST_SYM(reached_marking, fire_ptr,
                                                             eventualities, ev_p,
                                                             Index, merg);
                                break;
                            default            : CASES = ASYM_2_EXIST_SYM(reached_marking, fire_ptr,
                                                             eventualities, ev_p, merg);
                                break;
                            }

                            if (CASES != NO_THINK)   my_inqueue_stack(reached_marking, CASES);



                            /***********************************************************************/
                            POP_STATIC_CONF(SYM_STATIC_STORE, & tabc);
                            string_to_marking(fire_ptr->marking->marking_as_string,
                                              fire_ptr->marking->d_ptr,
                                              fire_ptr->marking->length);
                            GET_EVENTUALITIE_FROM_FILE(eventualities->marking->marking_as_string,
                                                       eventualities->marking->length);
                            COPY_CACHE(eventualities->marking->length);
                            STRING_TO_EVENTUALITIE();
                            STORE_SYM_TOT(tot, SYM_TOT) ;
                            NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot, merg,
                                             ASYM_STATIC_STORE, RESULT);

                            /**********************************************************************/

                            Index++;

                            ev_p = nev_p;
                        }
                        dispose_old_event(copy_of_ev_p);
                    }
                    current_transition = next_transition;
                }
                my_garbage_collect(eventualities);

                /***************** count dead markings **************/
                if (((fire_ptr->Marking_Type == SATURED_SYM && fire_ptr->dead) ||
                        (fire_ptr->Marking_Type != SATURED_SYM)) &&
                        dead) {
                    DEAD_MARKING ++;
                    printf("Number of the dead marking :%lu \n",
                           fire_ptr->marking->cont_tang);
                }
                /****************************************************/
            }
            eventualities = eventualities->Head_Next_Event;
        }
        if (output_flag || out_mc)
            END_EV(OUTPUT_FILE);

        fire_ptr->NOT_Treated = NULL;

        if (fire_ptr->Marking_Type == SATURED_SYM) {
            FREE_EVENTUALITIES(fire_ptr->Head_Next_Event)  ;
            fire_ptr->Head_Next_Event = NULL;
        }

        my_pop(&top);
    }
    while (top != NULL);

    if (output_flag)
        fprintf(OUTPUT_FILE, "}\n");

    time(&new_time);

    fclose(EVENT_MARK);
// if(output_flag)
//  fclose(OUTPUT_FILE);

    fprintf(stdout, "\n:::::::::::::::        ESRG RESULTS           :::::::::::::::\n");

    fprintf(stdout, "\n---------------  ESMs ------------------\n");
    fprintf(stdout, "TOTAL NUMBER OF ESMs : %d\n",  ESM_COUNTER - 1);

    fprintf(stdout, "\n--------------- SATURATED ESMs ------------------\n");
    fprintf(stdout, "NUMBER OF SATURATED ESMs: %d\n", COMPT_SAT);

    fprintf(stdout, "\n---------------  EVENTUALITIES NODE STRUCTURES ------------------\n");
    fprintf(stdout, "NUMBER OF EVENTUALITIES ALLOCATED : \nIN_GRAPH  %d \nPOP %d \nPUSH %d \n"
            , NB_EV_IN_GRAPH, TREENODEEVENT_MALL_TOT, TREENODEEVENT_POP_TOT);

    fprintf(stdout, "\nNUMBER OF COMPUTED EVENTUALITIES (=|SRG|) : %d\n", COMPT_EVENT);

    fprintf(stdout, "\nDEAD SYMBOLIC MARKINGS (in SRG): %d\n", DEAD_MARKING);

    fprintf(stdout, "\nTIME REQUIRED ----------> %d\n", new_time - old_time);

    fprintf(stdout, "\n:::::::::::::::     END OF ESRG RESULTS        ::::::::::::::::\n\n");

    if (out_mc) {
        fprintf(STATISTICS, "%d %d %d",  ESM_COUNTER - 1, COMPT_EVENT + 1, COMPT_STORED_EVENT + 1);
    }
}
#endif
#endif



