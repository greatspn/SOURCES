

#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"

#ifdef LIBSPOT
#include "../../INCLUDE/gspnlib.h"
#include "../../INCLUDE/SDECLSPOT.h"
//include  "../../INCLUDE/SSTRUCTSPOT.h"
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


/******************************* For ESRG ********************************************/
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
extern char **    *GetMatRepOfClObj();
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
static int    PILE = 0;
#ifdef LIBSPOT
static int   NOT_YET = true;
TO_MERGEP *MERG_ALL  = NULL;
STORE_STATICS_ARRAY Sym_StaticConf = NULL;
#endif
/****************************************************************/


/************************************************************************************/
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
        if ((event->marking->pri > priority) && (event->enabled_head))
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
    STORED_CARD      = CREATE_STORE_CARD_STRUCTURE();

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

    INIT_GLOBAL_VARS();

    EXCEPTION(); /** for Ordred class with more then 1 sub-class **/

    time(&old_time);
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
    GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num, &tot, SYM_STATIC_STORE, merg);
    /***************************************************************/

    /******************* Group and canonaize the initial symetric marking *********************/
    ALL_MY_GROUP(&MERGING_MAPPE);
    get_canonical_marking();
    AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);

    f_mark = marking_to_string();
    insert_tree(&root, &h, f_mark, length, d_ptr); root->marking->cont_tang = ESM_COUNTER; ESM_COUNTER++;


    string_to_marking(root->marking->marking_as_string, root->marking->d_ptr, root->marking->length);
    /***********************************************************************************************/

    /********************** Finding the initial set of firing instances **********/
    if (TEST_SYMETRIC_INITIAL(merg) == 1) {
        MARKING_TYPE = SATURED_SYM;
        COMPT_SAT++;
        my_initialize_en_list();
        root->enabled_head = enabled_head;
        root->Marking_Type = SATURED_SYM;
        root->marking->pri = cur_priority;
    }
    else {MARKING_TYPE = NO_SATURED_SYM; NO_SAT_INI = TRUE;}

    /************************* Geting the first and unique eventuality of the initial marking ****/
    GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE, ASYM_STATIC_STORE, STORED_CARD, merg);
    /*********************************************************************************************/
    if (MARKING_TYPE == SATURED_SYM) {
        STORE_SYM_TOT(tot, SYM_TOT) ;
        if (output_flag)
            ESM_REACHED(OUTPUT_FILE, root, 0); /******** Writing the Output file *****/
        my_inqueue_stack(root, INSERT_SAT); top = root; PILE = INSERT_SAT;
    }
    else {
        STORE_SYM_TOT(tot, SYM_TOT) ;
        if (output_flag)
            ESM_REACHED(OUTPUT_FILE, root, 1);
        my_inqueue_stack(root, INSERT_NO_SAT); top = root; PILE = INSERT_NO_SAT;
    }

    /************************* Get the asymetric marking to comput the asymetric firing instances ****/
    NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot, merg, ASYM_STATIC_STORE, RESULT);

    if (MARKING_TYPE == SATURED_SYM) MARKING_TYPE = SATURED_INS;
    else MARKING_TYPE = NO_SATURED_INS;

    enabled_head = NULL;
    my_initialize_en_list();
    COMPT_EVENT++;

    if (enabled_head) {
        EVENTUALITIE_TO_STRING();
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        eventualities = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
        eventualities->enabled_head = enabled_head;
        eventualities->Marking_Type = SATURED_INS;
        eventualities->marking->pri = cur_priority;
    }
    root->Head_Next_Event = root->NOT_Treated = eventualities;

    /******************************************************************************************/

    do {

        fire_ptr = top;

        if (fire_ptr->Marking_Type == SATURED_SYM)
            greatest_pri = set_greatest_priority(fire_ptr);
        else greatest_pri = UNKNOWN;

        if (fire_ptr->Marking_Type == SATURED_SYM) {

            MARKING_TYPE = SATURED_SYM;
            POP_STATIC_CONF(SYM_STATIC_STORE, &tabc);
            string_to_marking(fire_ptr->marking->marking_as_string, fire_ptr->marking->d_ptr, fire_ptr->marking->length);

            current_transition = fire_ptr->enabled_head;

            while (current_transition != NULL) {
                next_transition = current_transition->next;

                tr = GET_TRANSITION_INDEX(current_transition->list);

                if (tabt[tr].pri == greatest_pri) {
                    copy_of_ev_p = get_new_event(tr);
                    ev_p = current_transition->list;

                    while (ev_p != NULL) {
                        if (output_flag)
                            ESM_SOURCE(OUTPUT_FILE, fire_ptr, 0)  ; /**** OUTPUT FILE ******/
                        nev_p = ev_p->next;
                        copy_event(copy_of_ev_p, ev_p);

                        if (IS_RESET_TRANSITION(tr))
                            reset_to_M0(initial_marking->marking->marking_as_string,
                                        initial_marking->marking->d_ptr, initial_marking->marking->length, tr);
                        else fire_trans(ev_p);
                        if (output_flag)
                            FIRE_INSTANCE(ev_p, OUTPUT_FILE, 1); /**** OUTPUT FILE ******/

                        get_canonical_marking();

                        f_mark = marking_to_string();

                        tro = 0;
                        insert_tree(&root, &h, f_mark, length, d_ptr);

                        if (current_marking == initial_marking && !home)  home = 1;
                        string_to_marking(reached_marking->marking->marking_as_string,
                                          reached_marking->marking->d_ptr, reached_marking->marking->length);

                        if (tro == NEW_MARKING) {reached_marking->marking->cont_tang = ESM_COUNTER; ESM_COUNTER++; }
                        if (output_flag)
                            ESM_REACHED(OUTPUT_FILE, reached_marking, 0); /**** OUTPUT FILE ******/

                        switch (tro) {
                        case NEW_MARKING   : CASES = SYM_SAT_2_NOT_EXIST(reached_marking, fire_ptr, ev_p, merg); break;
                        default            : CASES = SYM_SAT_2_EXIST(reached_marking, fire_ptr, ev_p, merg); break;
                        }

                        if (CASES != NO_THINK)  my_inqueue_stack(/*&top,*/reached_marking, CASES);

                        string_to_marking(fire_ptr->marking->marking_as_string, fire_ptr->marking->d_ptr, fire_ptr->marking->length);
                        ev_p = nev_p;
                    }
                    dispose_old_event(copy_of_ev_p);
                }
                current_transition = next_transition;
            }

            garbage_collect(fire_ptr);
        }

        eventualities = fire_ptr->NOT_Treated;

        while (eventualities) {

            marking_pri = eventualities->marking->pri;

            if (eventualities->enabled_head) {
                /**********************************GET THE ASYMETRIC MARKING************************************/
                POP_STATIC_CONF(SYM_STATIC_STORE, & tabc);
                string_to_marking(fire_ptr->marking->marking_as_string, fire_ptr->marking->d_ptr, fire_ptr->marking->length);
                GET_EVENTUALITIE_FROM_FILE(eventualities->marking->marking_as_string, eventualities->marking->length);
                COPY_CACHE(eventualities->marking->length);
                STRING_TO_EVENTUALITIE();
                STORE_SYM_TOT(tot, SYM_TOT) ;
                NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot, merg, ASYM_STATIC_STORE, RESULT);
                /********************************************************************************************************/

                current_transition = eventualities->enabled_head;

                while (current_transition != NULL) {
                    next_transition = current_transition->next;
                    tr = GET_TRANSITION_INDEX(current_transition->list);

                    if (((tabt[tr].pri == marking_pri) && (IS_ASYMETRIC(tr))) ||
                            ((tabt[tr].pri == marking_pri) && (IS_SYMETRIC(tr)) && (greatest_pri != marking_pri))
                       ) {
                        copy_of_ev_p = get_new_event(tr);
                        ev_p = current_transition->list;
                        Index = 0;
                        while (ev_p != NULL) {
                            if (output_flag)
                                ESM_SOURCE(OUTPUT_FILE, fire_ptr, 1)  ; /**** OUTPUT FILE ******/
                            Index++;
                            nev_p = ev_p->next;
                            copy_event(copy_of_ev_p, ev_p);

                            if (IS_RESET_TRANSITION(tr))
                                reset_to_M0(initial_marking->marking->marking_as_string,
                                            initial_marking->marking->d_ptr, initial_marking->marking->length, tr);
                            else
                                fire_trans(ev_p);
                            if (output_flag)
                                FIRE_INSTANCE(ev_p, OUTPUT_FILE, 0); /**** OUTPUT FILE ******/

                            SPECIAL_CANISATION();

                            STORE_CARD(card, &STORED_CARD);
                            GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num, &tot, SYM_STATIC_STORE, merg);
                            ALL_MY_GROUP(&MERGING_MAPPE);
                            get_canonical_marking();
                            AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);

                            f_mark = marking_to_string();

                            tro = 0;
                            insert_tree(&root, &h, f_mark, length, d_ptr);

                            if (current_marking == initial_marking && !home)  home = 1;
                            string_to_marking(reached_marking->marking->marking_as_string,
                                              reached_marking->marking->d_ptr, reached_marking->marking->length);

                            if (tro == NEW_MARKING) {reached_marking->marking->cont_tang = ESM_COUNTER; ESM_COUNTER++; }


                            GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE, ASYM_STATIC_STORE, STORED_CARD, merg);
                            if (output_flag)
                                ESM_REACHED(OUTPUT_FILE, reached_marking, 1); /**** OUTPUT FILE ******/

                            switch (tro) {
                            case NEW_MARKING   : CASES = ASYM_2_NOT_EXIST_SYM(reached_marking, fire_ptr, eventualities, ev_p, Index, merg); break;
                            default            : CASES = ASYM_2_EXIST_SYM(reached_marking, fire_ptr, eventualities, ev_p, merg); break;
                            }

                            if (CASES != NO_THINK)   my_inqueue_stack(reached_marking, CASES);

                            /**********************************************************************************************************/
                            POP_STATIC_CONF(SYM_STATIC_STORE, & tabc);
                            string_to_marking(fire_ptr->marking->marking_as_string, fire_ptr->marking->d_ptr, fire_ptr->marking->length);
                            GET_EVENTUALITIE_FROM_FILE(eventualities->marking->marking_as_string, eventualities->marking->length);
                            COPY_CACHE(eventualities->marking->length);
                            STRING_TO_EVENTUALITIE();
                            STORE_SYM_TOT(tot, SYM_TOT) ;
                            NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot, merg, ASYM_STATIC_STORE, RESULT);
                            /**********************************************************************************************************/

                            Index++;

                            ev_p = nev_p;
                        }
                        dispose_old_event(copy_of_ev_p);
                    }
                    current_transition = next_transition;
                }
                my_garbage_collect(eventualities);
            }
            eventualities = eventualities->Head_Next_Event;
        }

        fire_ptr->NOT_Treated = NULL;

        if (fire_ptr->Marking_Type == SATURED_SYM) {
            FREE_EVENTUALITIES(fire_ptr->Head_Next_Event)  ;
            fire_ptr->Head_Next_Event = NULL;
        }

        my_pop(&top);
    }
    while (top != NULL);


    time(&new_time);

    fclose(EVENT_MARK);
    if (output_flag)
        fclose(OUTPUT_FILE);

    fprintf(stdout, "\n:::::::::::::::        ESRG RESULTS           :::::::::::::::\n");

    fprintf(stdout, "\n--------------- NB OF ESMs ------------------\n");
    fprintf(stdout, "TOTAL NUMBER OF ESMs : %d\n",  ESM_COUNTER - 1);

    fprintf(stdout, "\n--------------- NB OF SATURED ESMs ------------------\n");
    fprintf(stdout, "NB OF STAURED ESMs: %d\n", COMPT_SAT);

    fprintf(stdout, "\n--------------- NB OF EVENTUALITIES NODE STRUCTURES ------------------\n");
    fprintf(stdout, "NUMBER OF EVENTUALITIES ALLOCATED :IN_GRAPH  %d  POP %d  PUCH %d\n", NB_EV_IN_GRAPH, TREENODEEVENT_MALL_TOT, TREENODEEVENT_POP_TOT);

    fprintf(stdout, "\n--------------- NB OF EVENTUALITIES CALCULATED ------------------\n");
    fprintf(stdout, "NUMBER OF EVENTUALITIES CALCULATED: %d\n", COMPT_EVENT);

    fprintf(stdout, "\n:::::::::::::::     END OF ESRG RESULTS         ::::::::::::::::\n");

    fprintf(stdout, "\n TIME REQUIRED ----------> %d\n", new_time - old_time);



}
#endif
#endif




/************************* API FUNCTIONS FOR SPOT *******************************/
#ifdef LIBSPOT

Tree_Esrg_p MY_TREE_POP(int buff, int length) {
    Tree_Esrg_p ptr = NULL;

    ptr = (Tree_Esrg_p)TREE_EVENT_ALLOCATION(buff, length);
    ptr->Head_Next_Event = NULL;
    ptr->enabled_head    = NULL;
    ptr->Marking_Type    = -1;

    return (ptr);
}
void INIT_ARRAYS(TO_MERGEP *merg, int **num, int *** card, STORE_STATICS_ARRAY STORED_CONF) {

    int i, j;
    TO_MERGEP ptr = NULL;
    for (i = 0; i < ncl; i++) {
        ptr = merg[i];

        while (ptr) {
            for (j = 0; j < ptr->NB; j++) STATICS[i][ptr->sbc_num][j] = STORED_CONF[i].ptr[ptr->SSCs[j]].card;
            for (j = 0; j < num[i][ptr->sbc_num]; j++) DYNAMIC[i][ptr->sbc_num][j] = card[i][ptr->sbc_num][j];
            NB_DSC[i][ptr->sbc_num] = num[i][ptr->sbc_num];
            NB_SSBC[i][ptr->sbc_num] = ptr->NB;
            ptr = ptr->next;
        }

    }

}
Tree_Esrg_p TEST_EXISTENCE(Tree_Esrg_p List_Events, Tree_Esrg_p *prec) {
    (*prec) = List_Events;

    while (List_Events) {
        if (COMPARE_EVENTUALITIES(List_Events->marking->marking_as_string, List_Events->marking->length) == EQUAL)return List_Events;
        (*prec) = List_Events;
        List_Events = List_Events->Head_Next_Event;
    }

    return NULL;
}
int IS_WN_SYMETRIC() {
    int i;

    for (i = 0; i < ncl; i++)
        if (tabc[i].sbc_num > 1) return 1;

    return 0;
}

void Free_EVENT_ARRAY() {
    int i;
    for (i = 0; i < SIZE_OF_EVENT_ARR; i++) {
        free(EVENT_ARRAY[i].marking);
    }
    free(EVENT_ARRAY);

}

void Free_events(Tree_Esrg_p EVENT_ARRAY, int SIZE_OF_EVENT_ARR) {
    int i;
    for (i = 0; i < SIZE_OF_EVENT_ARR; i++) {
        free(EVENT_ARRAY[i].marking);
    }
    free(EVENT_ARRAY);
}

void TRAITEMENT_3(TO_MERGEP *merg) {
    int cl, i, j;
    TO_MERGEP ptr = NULL;
    int *tot_sbc = calloc(ncl, sizeof(int));

    for (cl = 0; cl < ncl; cl++) {
        ptr = merg[cl];

        while (ptr) {
            for (i = 0; i < NB_DSC[cl][0]; i++)
                for (j = 0; j < ptr->NB; j++)
                    TEMP[cl][0][i][ptr->SSCs[j]] =  RESULT[cl][ptr->sbc_num][i][j];

            tot_sbc[cl] += ptr->NB;
            ptr = ptr->next;
        }

    }

    PARM_EVENTUALITIE_TO_STRING(1, tot_sbc, TEMP);
    ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
    EVENT_ARRAY = realloc(EVENT_ARRAY, (SIZE_OF_EVENT_ARR + 1) * sizeof(struct TREE_ESRG));

    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking = malloc(sizeof(struct MARKING_INFO));
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking->marking_as_string = FILE_POS_PTR;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking->length = LEGTH_OF_CACHE;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].enabled_head = NULL;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking->pri = UNKNOWN;
    SIZE_OF_EVENT_ARR++;

    free(tot_sbc);
}
void init_CONS_(signed char *prop) {
    int i;
    for (i = 0; i < NbProp; i++)
        switch (prop[i]) {
        case 1  : _CONS_[i] = 1  ; break;
        case 0  : _CONS_[i] = 0  ; break;
        default : _CONS_[i] = -1 ; break;
        }
}
int my_strcmp_modified(char *compare, unsigned long length_to_compare, unsigned long length_to_insert)

{
    unsigned long op1;
    unsigned long op2;
    unsigned long ii, min;

    char *pp = cache_string;
    char *cp = compare;

    if (length_to_insert != length_to_compare)
        return (-1);
    else {
        for (ii = length_to_insert; ii; ii--) {
            op1 = *pp;
            pp++;
            op2 = *cp;
            cp++;
            if (op1 != op2)
                return (-1);
        }
        return (0);
    }
}

void Free_ResList(int i) {
    int cl;

    free(ResultList[i]->indx);
    for (cl = 0; cl < ncl; cl++)
        free(ResultList[i]->group[cl]);
    free(ResultList[i]->group);
    free(ResultList[i]-> NbGroup);
    LIBERER_ELEM(ResultList[i]->list);
    free(ResultList[i]);


}
void Free_TAB_SYM(MarkSym *tab, int size) {
    int i;
    Tree_Esrg_p ptr, next;
    for (i = 0; i < size; i++) {
        free(tab[i].marking_as_string);
        ptr = tab[i]. Eventp;
        while (ptr) {
            next = ptr->Head_Next_Event;
            free(ptr->marking);
            free(ptr);
            ptr = next;
        }
    }
    free(tab);


}

void TEST_AND_ADD(MarkSym **tab_sym, int *Nbtabsym) {
    Tree_Esrg_p prec;
    int i;

    i = 0;
    while (i < (*Nbtabsym))
        if (my_strcmp_modified((*tab_sym)[i].marking_as_string, (*tab_sym)[i].length, length) == 0) {
            if (!TEST_EXISTENCE((*tab_sym)[i].Eventp, &prec)) {
                ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
                if (prec) prec->Head_Next_Event = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
                else prec = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
                return ;
            }
            else return ;

        }
        else i++ ;

    ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
    (*tab_sym) = realloc((*tab_sym), ((*Nbtabsym) + 1) * sizeof(MarkSym));
    (*tab_sym)[(*Nbtabsym)].marking_as_string = calloc(length, sizeof(char));
    for (i = 0; i < length; i++)
        (*tab_sym)[(*Nbtabsym)].marking_as_string[i] = cache_string[i];

    (*tab_sym)[(*Nbtabsym)].length = length;
    (*tab_sym)[(*Nbtabsym)].d_ptr = d_ptr;
    (*tab_sym)[(*Nbtabsym)].Eventp = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
    (*Nbtabsym)++;

}


void set_state_cache(char *str, int id_ptr, int ilength) {
    int i;

    d_ptr = id_ptr;
    length = ilength;
    for (i = 0; i < length; i++) cache_string[i] = str[i];
    fseek(mark, 0, SEEK_END);
    f_mark = ftell(mark);

}

int comp_partitions(Tree_p root, TYPE_P **PART_MAT, TYPE_P NbElPM) {
    int cl, sb;

    for (cl = 0; cl < ncl; cl++)
        if (root->NbElPM[cl] == NbElPM[cl]) {
            for (sb = 0; sb < NbElPM[cl]; sb++)
                if (!COMP(root->PM[cl][sb], PART_MAT[cl][sb]))
                    return false;
        }
        else
            return false;

    return true;
}
void save_cache_string(char *cache) {
    int i;
    for (i = 0; i < length; i++)
        cache[i] = cache_string[i];
}
void restore_cache_string(char *cache) {
    int i;
    for (i = 0; i < length; i++)
        cache_string[i] = cache[i];
}

int in_stack(Tree_p marking) {
    Tree_p ptr = top;

    while (ptr)
        if (ptr == marking)
            return true;
        else
            ptr = ptr->last;

    return false;
}

void remove_stack(Tree_p marking) {
    Tree_p ptr = top;
    Tree_p old_ptr = NULL;

    if (marking == top) {
        top = top->last;
        return ;
    }

    old_ptr = top;
    ptr = top->last;

    while (ptr) {
        if (ptr == marking) {
            if (ptr->last == NULL)
                old_ptr->last = NULL;
            else
                old_ptr = ptr->last;
            return ;
        }

        old_ptr = ptr;
        ptr = ptr->last;
    }
}
void TO_STORE_INIT_ARRAYS(STORE_STATICS_ARRAY STORED_CONF) {
    int i, j;

    TO_MERGEP ptr = NULL;

    for (i = 0; i < ncl; i++) {
        NB_DSC[i][0] = num[i][0];
        NB_SSBC[i][0] = STORED_CONF[i].sbc_num;
    }

}
void INIT_INITIAL_RESULT() {
    int cl, ds;

    INIT_4D_MATRIX(RESULT);

    for (cl = 0; cl < ncl; cl++) {
        for (ds = 0; ds < num[cl][0]; ds++)
            RESULT[cl][0][ds][0] = card[cl][0][ds];

        NB_DSC[cl][0] = num[cl][0];
        NB_SSBC[cl][0] = 1;
    }
}

void my_insert_tree(int *** event, STORE_STATICS_ARRAY asym_static_conf, TYPE_P **PART_MAT, TYPE_P NbElPM, int *flg) {

    Tree_p ptr = NULL, new_ptr = NULL, last_ptr = NULL;
    PSUPPORT Supp1 = NULL, Supp2 = NULL;

    int new_mark_inc_old_mark, old_mark_inc_new_mark, instack, greater = false;
    char cache[MAX_CACHE];

    if (tro == NEW_MARKING) {
        INIT_RESULT_STRUCT(event);
        TO_STORE_INIT_ARRAYS(asym_static_conf);
        EVENTUALITIE_TO_STRING(Sym_StaticConf);
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        reached_marking->Head_Next_Event = (Tree_p)my_treenode_pop(FILE_POS_PTR, LEGTH_OF_CACHE, 0);
        reached_marking->Head_Next_Event->Head_Next_Event = NULL;
        reached_marking->Head_Next_Event->PM = PART_MAT;
        reached_marking->Head_Next_Event->NbElPM = NbElPM;
        reached_marking->Head_Next_Event->Parent = reached_marking;
        reached_marking->Head_Next_Event->marking->cont_tang = MARKING_COUNTER + 1;
        inqueue(&top, &bottom, reached_marking->Head_Next_Event);
        *flg = false;
        /****** increase the number of eventualities ***************************/
        /*   */
        /*        */
        /*       write_on_srg(OUTPUT_FILE, NULL );	   */
        /*       MARKING_COUNTER++; */
        /***********************************************************************/
        return ;
    }
    else {
        ptr = reached_marking->Head_Next_Event;

        while (ptr) {

            PART_MAT_P PM = GetRefinedPartitions(ptr->PM , ptr->NbElPM, PART_MAT, NbElPM);
            Tree_Esrg_p  event_arry_event_inclued = NULL  ; int size_arry_event_inclued = 0;
            Tree_Esrg_p  event_arry_event_inclued_in = NULL  ; int size_arry_event_inclued_in = 0;

            GET_EVENTUALITIE_FROM_FILE(ptr->marking->marking_as_string, ptr->marking->length);
            COPY_CACHE(ptr->marking->length);
            STRING_TO_EVENTUALITIE(Sym_StaticConf);

            get_refined_eventualities(ptr->PM, ptr->NbElPM, PM->PART_MAT , PM->NbElPM,
                                      &event_arry_event_inclued_in  , &size_arry_event_inclued_in);
            INIT_RESULT_STRUCT(event);
            get_refined_eventualities(PART_MAT, NbElPM, PM->PART_MAT , PM->NbElPM,
                                      &event_arry_event_inclued  , &size_arry_event_inclued);

            new_mark_inc_old_mark  = Inclusion(event_arry_event_inclued , size_arry_event_inclued,
                                               event_arry_event_inclued_in  , size_arry_event_inclued_in);

            old_mark_inc_new_mark	= Inclusion(event_arry_event_inclued_in  , size_arry_event_inclued_in,
                                                event_arry_event_inclued , size_arry_event_inclued);

            instack = in_stack(ptr);

            if (new_mark_inc_old_mark &&  old_mark_inc_new_mark && !greater) {
                new_ptr = ptr;
            }
            else if (new_mark_inc_old_mark && instack) {
                greater = true;
                new_ptr = ptr;

            }
            else if (old_mark_inc_new_mark && instack) {
                retreve(&top, &bottom, ptr);
            }

            FreePartMAt(PM->PART_MAT, PM->NbElPM);
            free(PM);
            Free_events(event_arry_event_inclued_in  , size_arry_event_inclued_in);
            Free_events(event_arry_event_inclued  , size_arry_event_inclued);

            last_ptr = ptr;
            ptr = ptr->Head_Next_Event;
        }


        if (new_ptr) {
            if (!in_stack(new_ptr))
                inqueue(&top, &bottom, new_ptr);
            return ;
        }
        else {
            TO_STORE_INIT_ARRAYS(asym_static_conf);
            INIT_RESULT_STRUCT(event);
            EVENTUALITIE_TO_STRING(Sym_StaticConf);
            ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
            last_ptr->Head_Next_Event = (Tree_p)my_treenode_pop(FILE_POS_PTR, LEGTH_OF_CACHE, 0);
            last_ptr->Head_Next_Event->Head_Next_Event = NULL;
            last_ptr->Head_Next_Event->PM = PART_MAT;
            last_ptr->Head_Next_Event->NbElPM = NbElPM;
            last_ptr->Head_Next_Event->Parent = reached_marking;
            inqueue(&top, &bottom, last_ptr->Head_Next_Event);

            last_ptr->Head_Next_Event->marking->cont_tang = MARKING_COUNTER + 1;


            *flg = false;
            /* 	   MARKING_COUNTER++; */
            /* 	   write_on_srg(OUTPUT_FILE, NULL ); */
            return ;
        }
    }
}

void OrderSuccs1(State **succ, size_t *succ_size) {
    int st, j, nbsb = 0, S, Treat = *succ_size, Ord = (* succ_size) - 1;

    int *Arr = calloc(* succ_size , sizeof(int));
    State *ordred_succ = calloc(*succ_size, sizeof(State)); ;

    while (Treat) {
        for (j = 0; j < *succ_size ; j++)
            if (Arr[j] == 0 && (S = somme((*succ)[j]->NbElPM)) > nbsb) {
                nbsb = S;
                ordred_succ[Ord] = (*succ)[j];
                st = j;
            }

        Arr[st] = 1;
        Treat --;
        nbsb = 0;
        Ord--;
    }
    free(Arr);
    free(*succ);
    *succ = ordred_succ;
}

extern void Diff_succ(State sym_mark1, State sym_mark2, State **succ, size_t *succ_size) {
    int j, flg;

    PART_MAT_P s_prop_Part = NULL, PM = NULL;

    TO_MERGEP *merg_group   = NULL;

    STORE_STATICS_ARRAY Raff_StaticConf = NULL,
                        gr_StaticConf = NULL;

    Tree_Esrg_p  event_arry_event_inclued = NULL  ; int size_arry_event_inclued = 0;
    Tree_Esrg_p  event_arry_event_inclued_in = NULL  ; int size_arry_event_inclued_in = 0;

    MarkAsEventp list;

    top = NULL;


    PM = GetRefinedPartitions(sym_mark1->PM, sym_mark1->NbElPM, sym_mark2->PM, sym_mark2->NbElPM);
    Raff_StaticConf = NewTabc(PM->PART_MAT, PM->NbElPM, MTCL);

    GET_EVENTUALITIE_FROM_FILE(sym_mark1->marking->marking_as_string, sym_mark1->marking->length);
    COPY_CACHE(sym_mark1->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);
    get_refined_eventualities(sym_mark1->PM, sym_mark1->NbElPM, PM->PART_MAT , PM->NbElPM,
                              &event_arry_event_inclued  , &size_arry_event_inclued);


    GET_EVENTUALITIE_FROM_FILE(sym_mark2->marking->marking_as_string, sym_mark2->marking->length);
    COPY_CACHE(sym_mark2->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);
    get_refined_eventualities(sym_mark2->PM, sym_mark2->NbElPM, PM->PART_MAT , PM->NbElPM,
                              &event_arry_event_inclued_in  , &size_arry_event_inclued_in);

    Tree_Esrg_p list_diff = Get_Diff_Mark(event_arry_event_inclued , size_arry_event_inclued,
                                          event_arry_event_inclued_in , size_arry_event_inclued_in);

    if (list_diff) {

        GROUP_MARKINGS(list_diff, Raff_StaticConf, Sym_StaticConf, MTCL);

        for (j = 0; j < NbResList; j++) {
            gr_StaticConf = NewTabc(ResultList[j]->PART_MAT, ResultList[j]->NbElPM, MTCL);
            POP_STATIC_CONF(gr_StaticConf, &tabc);
            merg_group = TO_MERGE(ncl, tabc);
            GROUPING_ALL_STATICS(merg_group, tabc, num);
            list = ResultList[j]->list;

            flg = true;

            while (list) {

                reached_marking = sym_mark2->Parent;
                my_insert_tree(list->Event, gr_StaticConf, ResultList[j]->PART_MAT, ResultList[j]->NbElPM, &flg)  ;
                list = list->next;
            }

            if (flg) FreePartMAt(ResultList[j]->PART_MAT, ResultList[j]->NbElPM);

            Free_ResList(j);
            FreeStoreStructs(gr_StaticConf);
            FREE_ALL_LMS_ELEMS();
            FreeMerg(merg_group);
        }

        Free_DSC_SSC();
        free(ResultList);
        ResultList = NULL;
        NbResList = 0;
        FreeStoreStructs(Raff_StaticConf);
    }

    FreePartMAt(PM->PART_MAT, PM->NbElPM); free(PM);

    Free_events(event_arry_event_inclued_in  , size_arry_event_inclued_in);
    Free_events(event_arry_event_inclued  , size_arry_event_inclued);

    while (top) {
        (*succ_size)++ ;
        (*succ) = realloc((*succ), sizeof(State) * (*succ_size));
        (*succ)[(*succ_size) - 1] = top;
        top = top->last;
    }




    if (*succ_size > 1)
        OrderSuccs1(succ, succ_size);


}


int spot_inclusion(State sym_mark1, State sym_mark2) {
    int inclu = false;
    Tree_Esrg_p  event_arry_event_inclued = NULL  ; int size_arry_event_inclued = 0;
    Tree_Esrg_p  event_arry_event_inclued_in = NULL  ; int size_arry_event_inclued_in = 0;

    if (sym_mark1->Parent != sym_mark2->Parent) return false ;

    PART_MAT_P PM = GetRefinedPartitions(sym_mark1->PM, sym_mark1->NbElPM,
                                         sym_mark2->PM, sym_mark2->NbElPM);


    GET_EVENTUALITIE_FROM_FILE(sym_mark1->marking->marking_as_string, sym_mark1->marking->length);
    COPY_CACHE(sym_mark1->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);
    get_refined_eventualities(sym_mark1->PM, sym_mark1->NbElPM, PM->PART_MAT , PM->NbElPM,
                              &event_arry_event_inclued  , &size_arry_event_inclued);


    GET_EVENTUALITIE_FROM_FILE(sym_mark2->marking->marking_as_string, sym_mark2->marking->length);
    COPY_CACHE(sym_mark2->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);
    get_refined_eventualities(sym_mark2->PM, sym_mark2->NbElPM, PM->PART_MAT , PM->NbElPM,
                              &event_arry_event_inclued_in  , &size_arry_event_inclued_in);


    inclu = Inclusion(event_arry_event_inclued , size_arry_event_inclued,
                      event_arry_event_inclued_in  , size_arry_event_inclued_in);
    char *st;
    printf(" \n spot_inclution \n ");
    printf("marquage à inclure \n");
    print_state(sym_mark1 , &st);
    printf("%s \n", st);
    free(st);
    printf("marquage incluant  \n");
    print_state(sym_mark2 , &st);
    printf("%s \n", st);
    free(st);
    if (inclu) printf("il y a inclusion");
    else  printf("il n'y a pas  inclusion");

    FreePartMAt(PM->PART_MAT, PM->NbElPM);
    free(PM);
    Free_events(event_arry_event_inclued_in  , size_arry_event_inclued_in);
    Free_events(event_arry_event_inclued  , size_arry_event_inclued);


    return inclu;
}

/* ------------------ State Space Exploration -----------------*/
static void set_state(const State s, int force) {
    static State current = NULL;

    if (current != s || force) {
#ifdef SWN
#ifdef SYMBOLIC
        string_to_marking(s->marking->marking_as_string, s->marking->d_ptr, s->marking->length);
#endif
#ifdef COLOURED
        string_to_marking(s->marking->marking_as_string, UNKNOWN, s->marking->length);
#endif
#endif
    }
    current = s;
}

/* Returns the identifier of the initial marking state */
extern int initial_state(pState M0) {

    TYPE_P NbElPM = NULL;
    TYPE_P **PM = NULL;
    int flg;

    if (! initial_marking) {

        create_canonical_data_structure();
        get_canonical_marking();

        f_mark = marking_to_string();
        insert_tree(&root, &h, f_mark, length, d_ptr);
        NbElPM = calloc(ncl, sizeof(TYPE));
        PM = GetMatRepOfClParts(MTCL, NbElPM) ;
        FreeStTabc();
        SYM_STATIC_STORE = Sym_StaticConf = NewTabc(PM, NbElPM, MTCL);
        INIT_INITIAL_RESULT();
        EVENTUALITIE_TO_STRING(Sym_StaticConf);
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        reached_marking->Head_Next_Event = (Tree_p)treenode_pop(FILE_POS_PTR, LEGTH_OF_CACHE, 0);
        reached_marking->Head_Next_Event->Head_Next_Event = NULL;
        reached_marking->Head_Next_Event->PM = PM;
        reached_marking->Head_Next_Event->NbElPM = NbElPM;
        reached_marking->Head_Next_Event->Parent = reached_marking;
        inqueue(&top, &bottom, reached_marking->Head_Next_Event);

        /****** increase the number of eventualities ***************************/
        //  reached_marking->Head_Next_Event->marking->cont_tang = MARKING_COUNTER+1;
        // write_on_srg(OUTPUT_FILE, NULL );
        // MARKING_COUNTER++;
        /**********************************************************************/

        initial_marking =  top;

    }

    *M0 = initial_marking ;

    return 0;
}


extern int satisfy(const State s, const AtomicProp  props [],  unsigned char **truth, size_t props_size) {
    /*  sizeof(unsigned char) = 1 */
    truth = malloc(props_size) ;

    (*truth)[0] = 1; /* TRUE = 1 */


    return 0;

}

int prop_index(const char *name, pAtomicProp  propindex) {

    int i;

    for (i = 0; i < NbProp; i++)
        if (strcmp(PROPOSITIONS[i].id, name) == 0) {
            (*propindex) = i;
            return 0;
        }

    return 1;
}


/* free the "truth" vector allocated by satisfy
   !!! NB: Don't forget to call me, or you will get a memory leak !!!
*/

extern int satisfy_free(unsigned char *truth) {
    free(truth);
    return 0;
}

int print_state(const State s, char **st) {

    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");

    STORE_STATICS_ARRAY StaticConf = NULL;
    StaticConf = NewTabc(s->PM, s->NbElPM, MTCL);

    /**********  change the current tabc by the one of s and load s state *********/
    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    string_to_marking(s->Parent->marking->marking_as_string, s->Parent->marking->d_ptr, s->Parent->marking->length);
    /******************************************************************************/

    /********** load the i'th eventuality from  the file *****************************************/
    GET_EVENTUALITIE_FROM_FILE(s->marking->marking_as_string, s->marking->length);
    COPY_CACHE(s->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);
    /********************************************************************************************/
    POP_STATIC_CONF(StaticConf, &tabc);
    MERG = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG, tabc, num);

    /************* instanciate the marking s with the eventuality i ****************************/
    NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot, MERG, StaticConf, RESULT);
    /*******************************************************************************************/

    get_canonical_marking();
    write_on_srg(fd, (s == initial_marking ? 0 : 1));
    pos = ftell(fd);
    *st = malloc((pos + 1) * sizeof(char));

    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);

    FreeStoreStructs(StaticConf);
    FreeMerg(MERG);
    FREE_ALL_LMS_ELEMS();

    return 0;
}
int somme(TYPE_P NBS) {
    int cl, s = 0;
    for (cl = 0; cl < ncl; cl++) {
        s += NBS[cl];
    }
    return s;
}
void OrderSuccList(int *Order) {
    int j, nbsb = 0, S, Treat = NbResList, Ord = 0;

    int *Arr = calloc(NbResList, sizeof(int));

    while (Treat) {
        for (j = 0; j < NbResList; j++)
            if (Arr[j] == 0 && (S = somme(ResultList[j]->NbElPM)) > nbsb) {
                nbsb = S;
                Order[Ord] = j;
            }

        Arr[Order[Ord]] = 1;
        Treat --;
        nbsb = 0;
        Ord++;
    }
    free(Arr);



}

void print_matrix(int length) {
    int i, j;
    for (i = 0; i < NB_DSC[0][0]; i++) {
        printf("\n");
        for (j = 0; j < NB_SSBC[0][0]; j++)
            printf(" %d ", RESULT[0][0][i][j]);
    }
    printf("\n");
    for (i = 0; i < length; i++)
        printf("%d", (int)CACHE_STRING_ESRG[i]);
}


void succ_arc(State s, signed char *prop, MarkSym **TAB_SYM, int *NbTAB_SYM , PART_MAT_P s_prop_Part) {

    int tr, marking_pri, i, j, first = false, flg;
    Result_p current_transition;
    Result_p next_transition;
    Event_p ev_p, nev_p, copy_of_ev_p;
    char *st;
    int *Order;


    TO_MERGEP *merg_s_prop  = NULL;
    TO_MERGEP *merg_group   = NULL;

    STORE_STATICS_ARRAY s_prop_StaticConf = NULL,
                        gr_s_prop_StaticConf = NULL,
                        old_StaticConf = NULL;

    //  PART_MAT_P s_prop_Part=NULL;

    EVENT_ARRAY = NULL;
    SIZE_OF_EVENT_ARR = 0;

    if (s == NULL) {first = true ; s = initial_marking;}

    // load eventuality in string format
    GET_EVENTUALITIE_FROM_FILE(s->marking->marking_as_string, s->marking->length);
    COPY_CACHE(s->marking->length);
    // transform the string eventuality in matrix representation
    STRING_TO_EVENTUALITIE(Sym_StaticConf); // p.r.a la partion symetrique
    //  printf("\n initialisation \n");
    // print_matrix(s->marking->length);
    // array of atomic propositions
    init_CONS_(prop);

    // get the rafinement of the partion of s w.r.t prop
    //  s_prop_Part=GetPartitions(s);
    // load partion of symbolic marking s
    old_StaticConf = NewTabc(s->PM, s->NbElPM, MTCL);
    // refined partition in tabc format
    s_prop_StaticConf = NewTabc(s_prop_Part->PART_MAT, s_prop_Part->NbElPM, MTCL);

    // merg_s_prop structure, identifies the mapping between s parition ans s_prop one
    merg_s_prop = ToMergArr(s_prop_Part->PART_MAT, s_prop_Part->NbElPM, s->PM, s->NbElPM);

    // Initial structures to compute refined enentualitis
    RAF_INIT_ARRAYS(merg_s_prop, s_prop_StaticConf);

    // compute refined eventualities
    EVENTUALITIES(0, 0, 1, 1, TRAITEMENT_3, merg_s_prop, old_StaticConf);


    POP_STATIC_CONF(s_prop_StaticConf, &tabc);
    MERG = MERG_ALL = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG_ALL, tabc, num);

    /***** main loop, computes all valid successors from each eventuality *************/
    for (i = 0; i < SIZE_OF_EVENT_ARR; i++) {
        /**********  change the current tabc by the one of s and load s state *********/
        POP_STATIC_CONF(Sym_StaticConf, &tabc);
        string_to_marking(s->Parent->marking->marking_as_string, s->Parent->marking->d_ptr, s->Parent->marking->length);
        /******************************************************************************/

        /********** load the i'th eventuality from  the file *****************************************/
        GET_EVENTUALITIE_FROM_FILE(EVENT_ARRAY[i].marking->marking_as_string, EVENT_ARRAY[i].marking->length);
        COPY_CACHE(EVENT_ARRAY[i].marking->length);
        STRING_TO_EVENTUALITIE(Sym_StaticConf);
        /********************************************************************************************/
        //  printf("\n aprés reffinent \n");
        // print_matrix(EVENT_ARRAY[i].marking->length);

        /************* instanciate the marking s with the eventuality i ****************************/
        NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot, MERG_ALL, s_prop_StaticConf, RESULT);
        /*******************************************************************************************/
        //  printf("\n passer l'instantiation \n");
        if (!first) {
            /*********** compute firing instances of the resulted marking *********/
            enabled_head = NULL;
            my_initialize_en_list();
            /*********************************************************************/

            EVENT_ARRAY[i].enabled_head = enabled_head;
            current_transition = EVENT_ARRAY[i].enabled_head;

            while (current_transition != NULL) {
                next_transition = current_transition->next;
                tr = GET_TRANSITION_INDEX(current_transition->list);

                if (tabt[tr].pri == cur_priority) {
                    copy_of_ev_p = get_new_event(tr);
                    ev_p = current_transition->list;

                    while (ev_p != NULL) {

                        nev_p = ev_p->next;
                        copy_event(copy_of_ev_p, ev_p);

                        if (IS_RESET_TRANSITION(tr))
                            reset_to_M0(initial_marking->marking->marking_as_string,
                                        initial_marking->marking->d_ptr, initial_marking->marking->length, tr);
                        else
                            fire_trans(ev_p);

                        SPECIAL_CANISATION(); /***** canonisation of the marking after firing *********/
                        // printf("\n successeur calculer correctement \n");
                        /**** begin :  test satisfaction of the properties *********************************************/
                        if (Satisfaction(_CONS_, NbProp, s_prop_Part->PART_MAT, s_prop_Part->NbElPM, card, num, DynDis)) {
                            //   printf("\n je satisfait \n");

                            /*** begin compute symetrical representation  **************/
                            STORE_CARD(card, & STORED_CARD);
                            GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num, &tot, Sym_StaticConf, MERG_ALL);
                            ALL_MY_GROUP(&MERGING_MAPPE);
                            SPECIAL_CANISATION();
                            AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);
                            /*** end compute symetrical representation *************/

                            // printf("\n j'ai reussi à calculer la representation symétrique \n");
                            /************** Compute the eventuality, corresponding to the asymétric marking *****/
                            GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE, s_prop_StaticConf, STORED_CARD, MERG);
                            /************************************************************************************/

                            /************ tranformation  of the eventuality to it string forme *****************/
                            INIT_ARRAYS(MERG_ALL, num, card, s_prop_StaticConf);
                            EVENTUALITIE_TO_STRING(Sym_StaticConf);
                            /*********************************************************************************/

                            /***  if the symtrical and it eventuality does'nt exist, add them   *************/
                            TEST_AND_ADD(TAB_SYM, NbTAB_SYM);
                            /********************************************************************************/
                        }
                        /**** end: test satisfaction of the properties *********************************************/

                        /**************** change the tabc structure and load the marking s  ****************************/

                        POP_STATIC_CONF(Sym_StaticConf, &tabc);
                        string_to_marking(s->Parent->marking->marking_as_string, s->Parent->marking->d_ptr, s->Parent->marking->length);
                        /**********************************************************************************************/

                        /**************** load the i'th eventuality from the file **************************************/
                        GET_EVENTUALITIE_FROM_FILE(EVENT_ARRAY[i].marking->marking_as_string, EVENT_ARRAY[i].marking->length);
                        COPY_CACHE(EVENT_ARRAY[i].marking->length);
                        STRING_TO_EVENTUALITIE(Sym_StaticConf);
                        /**********************************************************************************************/

                        //  print_matrix(EVENT_ARRAY[i].marking->length);

                        /******* instanciate the marking s by the i'th eventuality *****************************************/
                        NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot, MERG_ALL, s_prop_StaticConf, RESULT);
                        /**************************************************************************************************/
                        //	printf("\n je suis repasser par mon marquage initial  \n");
                        /**********************************************************************************************************/
                        ev_p = nev_p;
                    }
                    dispose_old_event(copy_of_ev_p);
                }
                current_transition = next_transition;
            }
            my_garbage_collect(&EVENT_ARRAY[i]);
        }
        else if (Satisfaction(_CONS_, NbProp, s_prop_Part->PART_MAT, s_prop_Part->NbElPM, card, num, DynDis)) {

            /*** begin compute symetrical marking **************/
            STORE_CARD(card, & STORED_CARD);
            GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num, &tot, Sym_StaticConf, MERG_ALL);
            ALL_MY_GROUP(&MERGING_MAPPE);
            SPECIAL_CANISATION();
            AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);
            /*** end compute symetrical marking*************/

            /************ tranformation  of the eventuality to it string form *****************/
            INIT_ARRAYS(MERG_ALL, num, card, s_prop_StaticConf);
            EVENTUALITIE_TO_STRING(Sym_StaticConf);
            /*********************************************************************************/

            /***  if the symtrical and it eventuality does'nt exist, add them   *************/
            TEST_AND_ADD(TAB_SYM, NbTAB_SYM);
            /********************************************************************************/
        }
    }

    FreeStoreStructs(s_prop_StaticConf);
    FreeStoreStructs(old_StaticConf);
    FreeMerg(merg_s_prop);
    FreeMerg(MERG_ALL);
    FREE_ALL_LMS_ELEMS();
    Free_EVENT_ARRAY();

}

void OrderSuccs(Succ_ **succ, size_t *succ_size) {
    int st, j, nbsb = 0, S, Treat = *succ_size, Ord = (* succ_size) - 1;

    int *Arr = calloc(* succ_size , sizeof(int));
    Succ_ *ordred_succ = calloc(*succ_size, sizeof(Succ_)); ;

    while (Treat) {
        for (j = 0; j < *succ_size ; j++)
            if (Arr[j] == 0 && (S = somme((*succ)[j].succ_->NbElPM)) > nbsb) {
                nbsb = S;
                ordred_succ[Ord].succ_ = (*succ)[j].succ_;
                ordred_succ[Ord].arc = (*succ)[j].arc;
                st = j;
            }

        Arr[st] = 1;
        Treat --;
        nbsb = 0;
        Ord--;
    }
    free(Arr);
    free(*succ);
    *succ = ordred_succ;

}

void print_top(Succ_ **succ , size_t *succ_size) {
    size_t i; char *st;
    for (i = 0; i < *succ_size; i++) {
        printf("curr_acc_conds :%d , curr_state : %d \n", (*succ)[i].arc->curr_acc_conds, (*succ)[i].arc->curr_state);
        print_state((*succ)[i].succ_, &st);
        printf("%s \n", st);
        free(st);
    }
}
extern int succ(State s, Props_* prop, size_t prop_size, Succ_ **succ , size_t *succ_size) {
    *succ = NULL;
    *succ_size = 0;

    int pr, conj, i, j, flg;
    top = NULL;

    PART_MAT_P s_prop_Part = NULL;
    PART_MAT_P Disj_Part = NULL, PM;
    TO_MERGEP *merg_group   = NULL;
    MarkSym *TAB_SYM = NULL;
    int      NbTAB_SYM = 0;
    STORE_STATICS_ARRAY s_prop_StaticConf = NULL,
                        gr_s_prop_StaticConf = NULL;
    MarkAsEventp list;

    for (pr = 0; pr < prop_size; pr++) {
        TAB_SYM = NULL;
        NbTAB_SYM = 0;

        s_prop_Part = calloc(prop[pr].nb_conj, sizeof(PART_MAT));
        for (conj = 0; conj < prop[pr].nb_conj; conj++) {
            init_CONS_(prop[pr].prop[conj]);

            if (s == NULL)
                PM = GetPartitions(initial_marking);
            else
                PM = GetPartitions(s);

            s_prop_Part[conj].PART_MAT = PM->PART_MAT;
            s_prop_Part[conj].NbElPM = PM->NbElPM;
        }

        Disj_Part = FindDisjParts3(s_prop_Part , prop[pr].nb_conj);

        for (conj = 0; conj < prop[pr].nb_conj; conj++) {
            FreePartMAt(s_prop_Part[conj].PART_MAT, s_prop_Part[conj].NbElPM);
            succ_arc(s, prop[pr].prop[conj], &TAB_SYM, &NbTAB_SYM , Disj_Part) ;
        }

        s_prop_StaticConf = NewTabc(Disj_Part->PART_MAT, Disj_Part->NbElPM, MTCL);

        for (i = 0; i < NbTAB_SYM; i++) {
            /***** find valid markings' groups *******************/
            GROUP_MARKINGS(TAB_SYM[i].Eventp, s_prop_StaticConf, Sym_StaticConf, MTCL);
            /*****************************************************/

            for (j = 0; j < NbResList; j++) {

                gr_s_prop_StaticConf = NewTabc(ResultList[j]->PART_MAT, ResultList[j]->NbElPM, MTCL);
                POP_STATIC_CONF(gr_s_prop_StaticConf, &tabc);
                merg_group = TO_MERGE(ncl, tabc);
                GROUPING_ALL_STATICS(merg_group, tabc, num);
                list = ResultList[j]->list;

                flg = true;

                while (list) {
                    /***** load  the symetric configuration on tabc and load a symetric marking  *********/
                    POP_STATIC_CONF(Sym_StaticConf, &tabc);
                    set_state_cache(TAB_SYM[i].marking_as_string, TAB_SYM[i].d_ptr, TAB_SYM[i].length);
                    insert_tree(&root, &h, f_mark, length, d_ptr);
                    string_to_marking(reached_marking->marking->marking_as_string,
                                      reached_marking->marking->d_ptr, reached_marking->marking->length);
                    /*************************************************************************************/

                    my_insert_tree(list->Event, gr_s_prop_StaticConf, ResultList[j]->PART_MAT, ResultList[j]->NbElPM, &flg)  ;

                    list = list->next;
                }

                if (flg) FreePartMAt(ResultList[j]->PART_MAT, ResultList[j]->NbElPM);

                Free_ResList(j);
                FreeStoreStructs(gr_s_prop_StaticConf);
                FREE_ALL_LMS_ELEMS();
                FreeMerg(merg_group);
            }
            Free_DSC_SSC();
            free(ResultList);
            ResultList = NULL;
            NbResList = 0;
        }
        Free_TAB_SYM(TAB_SYM, NbTAB_SYM);
        FreeStoreStructs(s_prop_StaticConf);

        while (top) {
            (*succ_size)++ ;
            (*succ) = realloc((*succ), sizeof(Succ_) * (*succ_size));
            (*succ)[(*succ_size) - 1].succ_ = top;
            (*succ)[(*succ_size) - 1].arc   = prop[pr].arc ;
            top = top->last;
        }

    }


    if (*succ_size > 1)
        OrderSuccs(succ, succ_size);
    printf("\n ---------------------------\n");
    print_top(succ , succ_size)  ;
    printf("\n ---------------------------\n");


    return 0;
}

/* free the "succ" vector allocated by succ
   !!! NB: Don't forget to call me, or you will get a memory leak !!!
*/
extern int succ_free(Succ_ * succ) {
    free(succ);

    return 0;
}
extern int diff_succ_free(State *succ) {
    free(succ);

    return 0;
}

#endif















