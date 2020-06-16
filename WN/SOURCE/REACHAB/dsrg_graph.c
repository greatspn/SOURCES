#include <stdio.h>
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
extern double get_ordinary_marking(int k);

/******************************* For ESRG *********************************/
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
extern char       cache_string [];
extern PART_MAT_P FindDisjParts3();
extern PART_MAT_P GetRefinedPartitions();
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
static int    COUNTERS = 0;
#ifdef LIBSPOT
static int    NB_SR = 0;
static int    NOT_YET = true;
TO_MERGEP    *MERG_ALL  = NULL;
STORE_STATICS_ARRAY Sym_StaticConf = NULL;
static int                 stop = false;
double             *tab_corr = NULL;
int                 nb_tab_corr = 0;
State               d_srg_top = NULL;
int                 DEAD_MARKINGS = 0;
int                 esm_count = 0;
FILE               *OUTFILE = NULL ;
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

static MRate_p path_head_ptr = NULL;
static unsigned long c_ph = 0;
static MRate_p path_tail_ptr = NULL;

static MRate_p tangible_path_head_ptr = NULL;
static unsigned long c_tph = 0;
static MRate_p tangible_path_tail_ptr = NULL;

static Throughput_p throu_head_ptr = NULL;
static unsigned long c_th = 0;
static Throughput_p throu_tail_ptr = NULL;

static Throughput_p total_throu_head_ptr = NULL;
static unsigned long c_tth = 0;
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

void handler() {
    stop = true;
}



void print_dsrg(FILE *dsrg, State source,
                State dests, double rate,
                int flag) {
    char *st;

    if (output_flag) {
        if (flag == 1) {
            print_state(source, &st);
            fprintf(dsrg, "S%lu [fontsize=7, label=\"S%lu[%lu]: %s (dead)\"];\n",
                    source->marking->cont_tang, source->marking->cont_tang,
                    source->Parent->marking->cont_tang,
                    st);
            free(st);
        }
        else {
            print_state(source, &st);

            fprintf(dsrg, "S%lu [fontsize=7, label=\"S%lu[%lu]: %s\"];\n",
                    source->marking->cont_tang, source->marking->cont_tang,
                    source->Parent->marking->cont_tang,
                    st);
            free(st)   ;

            print_state(dests, &st);

            fprintf(dsrg, "S%lu [fontsize=7, label=\"S%lu[%lu]: %s\"];\n",
                    dests->marking->cont_tang, dests->marking->cont_tang,
                    dests->Parent->marking->cont_tang,
                    st);
            free(st)   ;

            if (!out_mc) {
                if (flag == 2)
                    fprintf(dsrg, "S%lu -> S%lu  [style=dashed];\n",
                            source->marking->cont_tang,
                            dests->marking->cont_tang);
                else
                    fprintf(dsrg, "S%lu -> S%lu ;\n",
                            source->marking->cont_tang,
                            dests->marking->cont_tang);
            }
            else {
                if (flag == 2)
                    fprintf(dsrg, "S%lu -> S%lu [style=dashed,fontsize=10, label=\"%g\"];\n",
                            source->marking->cont_tang,
                            dests->marking->cont_tang, rate);
                else
                    fprintf(dsrg, "S%lu -> S%lu [ fontsize=10, label=\"%g\"];\n",
                            source->marking->cont_tang,
                            dests->marking->cont_tang, rate);

            }
        }
    }
    else {
        if (flag == 1)
            fprintf(dsrg, "%lu 0 \n", source->marking->cont_tang);

        if (flag == 0) {
            if (source->type == VANISHING)
                fprintf(dsrg, "%lu %lu %lu %lu %d 1 %g %lu %lu %lu %lu %d \n",
                        source->Parent->marking->cont_tang,
                        source->Parent->marking->ofset,
                        source->marking->cont_tang,
                        source->marking->ofset,
                        source->marking->ordinary, rate,
                        dests->Parent->marking->cont_tang,
                        dests->Parent->marking->ofset,
                        dests->marking->cont_tang,
                        dests->marking->ofset,
                        dests->marking->ordinary);
            else
                fprintf(dsrg, "%lu %lu %lu %lu %d 2 %g %lu %lu %lu %lu %d \n",
                        source->Parent->marking->cont_tang,
                        source->Parent->marking->ofset,
                        source->marking->cont_tang,
                        source->marking->ofset,
                        source->marking->ordinary, rate,
                        dests->Parent->marking->cont_tang,
                        dests->Parent->marking->ofset,
                        dests->marking->cont_tang,
                        dests->marking->ofset,
                        dests->marking->ordinary);
        }


        if (flag == 2)
            fprintf(dsrg, "%lu %lu %lu %lu %d 3 %g %lu %lu %lu %lu %d \n",
                    source->Parent->marking->cont_tang,
                    source->Parent->marking->ofset,
                    source->marking->cont_tang,
                    source->marking->ofset,
                    source->marking->ordinary, rate,
                    dests->Parent->marking->cont_tang,
                    dests->Parent->marking->ofset,
                    dests->marking->cont_tang,
                    dests->marking->ofset,
                    dests->marking->ordinary);

    }

}



void
print_state_const(Tree_p d_srg_top, Tree_p top, int trans) {
    stop = false;
    char *st;
    int ct = 1;

    printf(" \n THE CURRENT TREATED SYMBOLIC MARKING IS %lu :\n ",
           d_srg_top->marking->cont_tang);
    //  print_state(d_srg_top,&st);
    printf("%s \n", st);
    free(st)   ;
    Tree_p ptr = top;
    if (ptr) {
        if (trans == 0)
            printf("\n ITS SYMMETRIC SUCCESSORS ARE : \n");
        else
            printf("\n ITS ASYMMETRIC SUCCESSORS ARE : \n");

        while (ptr) {
            print_state(ptr, &st);
            printf("%d: %s \n", ct, st);
            free(st)   ;
            ct++;
            ptr = ptr->last;
        }
    }
    else if (trans == 0)
        printf("\n THIS SYMEBOLIC MARKING HAS NO SYMMETRIC SUCCESSORS   \n");
    else
        printf("\n THIS SYMEBOLIC MARKING HAS NO ASYMMETRIC SUCCESSORS \n");

    fprintf(stdout, "\n NUMBER OF REACHED SYMBOLIC MARKINGS  : %d ",
            MARKING_COUNTER + 1);

    if (out_mc)
        fprintf(stdout, "\n NUMBER OF VANINSHING SYMBOLIC MARKINGS : %d ",
                VANISHING_COUNT);

    fprintf(stdout, "\n NUMBER OF DEAD SYMBOLIC MARKINGS : %d ",
            DEAD_MARKINGS);

    printf("\n\n PRESS A KEY TO CONTINUE... ");

    getchar();

}

void
print_final_info() {
    fprintf(stdout, "\n ***** RESULTS OF THE DSRG CONSTRUCTION ****");
    fprintf(stdout, "\n NUMBER OF ESMS  :%d ",
            esm_count);
    fprintf(stdout, "\n NUMBER OF REACHED SYMBOLIC MARKINGS  :%d ",
            MARKING_COUNTER + 1);

    fprintf(stdout, "\n NUMBER OF VANINSHING SYMBOLIC MARKINGS : %d ",
            VANISHING_COUNT);

    fprintf(stdout, "\n NUMBER OF DEAD SYMBOLIC MARKINGS : %d ", DEAD_MARKINGS);

    fprintf(stdout, "\n TIME REQUIRED = %ds\n", new_time - old_time);

    if (out_mc) {
        fprintf(STATISTICS, "%d %d %d %d", NB_SR, MARKING_COUNTER + 1,
                VANISHING_COUNT, DEAD_MARKINGS);
        fclose(STATISTICS);
    }

}

static void dispose_all_instances(Result_p res_ptr) {
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

void my_garbage_collect(Tree_Esrg_p fire_ptr) {
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
    SBC_ARR          = ARRAY_ALLOCATION(ncl) ;
    RESULT           = MATRIX_4D_ALLOCATION();

    MERGING_MAPPE    = MAP_MERGING_ALLOCATION();
    DECOMP_MAPPE     = MAP_DECOMPOSING_ALLOCATION();
    STORED_CARD      = CREATE_STORE_CARD_STRUCTURE();

#ifndef LIBSPOT
    ASYM_STATIC_STORE = CREATE_STORE_STATIC_STRUCTURE();
    SYM_STATIC_STORE = CREATE_STORE_STATIC_STRUCTURE();
    SYM_TOT          = (int *) ARRAY_ALLOCATION(ncl);
#else

    hash_init();
    cache_init();
    raff_init();
    DynDis           = MATRIX_ALLOCATION();
    MTCL             = GetMatRepOfClObj();
    TEMP             = MATRIX_4D_ALLOCATION() ;
#endif
}

#endif

#ifdef LIBSPOT

unsigned long NB_EFF_PROP = 0;

int print_state(const State s, char **st) {

    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");

    STORE_STATICS_ARRAY StaticConf = NULL;
    StaticConf = NewTabc(s->PM, s->NbElPM, MTCL);

    /**********  change the current tabc by the one of s and load s state *******/
    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    string_to_marking(s->Parent->marking->marking_as_string,
                      s->Parent->marking->d_ptr, s->Parent->marking->length);
    /****************************************************************************/

    /********** load the i'th eventuality from  the file ************************/
    GET_EVENTUALITIE_FROM_FILE(s->marking->marking_as_string,
                               s->marking->length);
    COPY_CACHE(s->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);
    /****************************************************************************/
    POP_STATIC_CONF(StaticConf, &tabc);
    MERG = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG, tabc, num);

    /************* instanciate the marking s with the eventuality i *************/
    NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot, MERG, StaticConf, RESULT);
    /****************************************************************************/

    get_canonical_marking();
    write_on_srg(fd, (s == initial_marking ? 0 : 1));
    pos = ftell(fd);
    *st = malloc((pos + 1) * sizeof(char));

    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);

    FreeMerg(MERG);
    FREE_ALL_LMS_ELEMS();

    return 0;
}


int print_simple_state(const State s, char **st) {
    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");

    write_on_srg(fd, (s == initial_marking ? 0 : 1));
    pos = ftell(fd);
    *st = malloc((pos + 1) * sizeof(char));

    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);

    return 0;
}

Tree_Esrg_p
MY_TREE_POP(int buff, int length) {
    Tree_Esrg_p ptr = NULL;
    ptr = (Tree_Esrg_p)TREE_EVENT_ALLOCATION(buff, length);
    ptr->Head_Next_Event = NULL;
    ptr->enabled_head    = NULL;
    ptr->Marking_Type    = -1;
    ptr->gr = 0;
    return (ptr);
}

void
INIT_ARRAYS(TO_MERGEP *merg, int **num, int *** card,
            STORE_STATICS_ARRAY STORED_CONF) {
    int i, j;
    TO_MERGEP ptr = NULL;

    for (i = 0; i < ncl; i++) {
        ptr = merg[i];
        while (ptr) {
            for (j = 0; j < ptr->NB; j++)
                STATICS[i][ptr->sbc_num][j] = STORED_CONF[i].ptr[ptr->SSCs[j]].card;
            for (j = 0; j < num[i][ptr->sbc_num]; j++)
                DYNAMIC[i][ptr->sbc_num][j] = card[i][ptr->sbc_num][j];
            NB_DSC[i][ptr->sbc_num] = num[i][ptr->sbc_num];
            NB_SSBC[i][ptr->sbc_num] = ptr->NB;
            ptr = ptr->next;
        }
    }
}

Tree_Esrg_p
TEST_EXISTENCE(Tree_Esrg_p List_Events, Tree_Esrg_p *prec) {
    (*prec) = List_Events;

    while (List_Events) {
        if (COMPARE_EVENTUALITIES(List_Events->marking->marking_as_string,
                                  List_Events->marking->length) == EQUAL)
            return List_Events;
        (*prec) = List_Events;
        List_Events = List_Events->Head_Next_Event;
    }

    return NULL;
}

void
Free_EVENT_ARRAY() {
    int i;
    for (i = 0; i < SIZE_OF_EVENT_ARR; i++)
        free(EVENT_ARRAY[i].marking);

    free(EVENT_ARRAY);

}

void
Free_ResList(int i) {
    int cl;
    int ord = 0;
    float rate = 0;
    free(ResultList[i]->indx);
    for (cl = 0; cl < ncl; cl++)
        free(ResultList[i]->group[cl]);
    free(ResultList[i]->group);
    free(ResultList[i]-> NbGroup);
    LIBERER_ELEM(ResultList[i]->list, &ord, &rate);
    free(ResultList[i]);
}

void
Free_TAB_SYM(MarkSym *tab, int size) {
    int i;
    Tree_Esrg_p ptr, next;
    for (i = 0; i < size; i++) {
        free(tab[i].marking_as_string);
        ptr = tab[i].Eventp;
        while (ptr) {
            next = ptr->Head_Next_Event;
            free(ptr->input_tr);
            free(ptr->marking);
            free(ptr);
            ptr = next;
        }
    }
    free(tab);
}

void
Free_events(Tree_Esrg_p EVENT_ARRAY, int SIZE_OF_EVENT_ARR) {
    int i;
    for (i = 0; i < SIZE_OF_EVENT_ARR; i++)
        free(EVENT_ARRAY[i].marking);
    free(EVENT_ARRAY);
}

int my_strcmp_modified(char *compare,
                       unsigned long length_to_compare,
                       unsigned long length_to_insert) {
    unsigned long op1;
    unsigned long op2;
    unsigned long ii, min;

    char *pp = cache_string;
    char *cp = compare;

    if (length_to_insert != length_to_compare)
        return (-1);
    else {
        for (ii = length_to_insert; ii; ii--) {
            op1 = *pp; pp++;
            op2 = *cp; cp++;
            if (op1 != op2)
                return (-1);
        }
        return (0);
    }
}



void
TRAITEMENT_3(TO_MERGEP *merg) {
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
    ADD_TEMP_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE, TEMP_EVENT);
    EVENT_ARRAY = realloc(EVENT_ARRAY,
                          (SIZE_OF_EVENT_ARR + 1) * sizeof(struct TREE_ESRG));

    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking = malloc(sizeof(struct MARKING_INFO));
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking->marking_as_string = FILE_POS_PTR;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking->length = LEGTH_OF_CACHE;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].enabled_head = NULL;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking->pri = UNKNOWN;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].marking_for_prob = NULL;
    EVENT_ARRAY[SIZE_OF_EVENT_ARR].length_for_prob = 0;
    SIZE_OF_EVENT_ARR++;
    free(tot_sbc);
}

void
set_state_cache(char *str, int id_ptr, int ilength) {
    int i;
    d_ptr = id_ptr;
    length = ilength;
    for (i = 0; i < length; i++) cache_string[i] = str[i];
    fseek(mark, 0, SEEK_END);
    f_mark = ftell(mark);

}

void
TO_STORE_INIT_ARRAYS(STORE_STATICS_ARRAY STORED_CONF) {
    int i, j;
    for (i = 0; i < ncl; i++) {
        NB_DSC[i][0] = num[i][0];
        NB_SSBC[i][0] = STORED_CONF[i].sbc_num;
    }

}


void
INSERT_POS_STORE(FILE *fp, Tree_p ESM, Tree_p event) {
    int i, j, k;

    LP_ESRG = CACHE_STRING_ESRG;
    fseek(fp, 0, SEEK_END);

    unsigned long ofpos = ftell(fp);

    if (!event) {
        ESM->marking->ofset = ofpos;

        MY_CHAR_STORE((unsigned long)ESM->marking->marking_as_string);
        MY_CHAR_STORE((unsigned long)ESM->marking->length);
        MY_CHAR_STORE((unsigned long)ESM->marking->d_ptr);
    }
    else {

        event->marking->ofset = ofpos;

        MY_CHAR_STORE((unsigned long)ESM->marking->cont_tang);

        for (i = 0; i < ncl; i++) {
            MY_CHAR_STORE((unsigned long)event->NbElPM[i]);
            for (j = 0; j < event->NbElPM[i]; j++)
                for (k = 0; k < MAX_IND; k++)
                    MY_CHAR_STORE((unsigned long)event->PM[i][j][k]);
        }

        MY_CHAR_STORE((unsigned long)event->marking->marking_as_string);
        MY_CHAR_STORE((unsigned long)event->marking->length);
    }

    fwrite(CACHE_STRING_ESRG, 1, LP_ESRG - CACHE_STRING_ESRG, fp);
}

extern int initial_state(pState M0) {

    TYPE_P NbElPM = NULL;
    TYPE_P **PM = NULL;
    STORE_STATICS_ARRAY initial_statics = NULL;

    int flg;

    if (! initial_marking) {

        STORE_STATICS_ARRAY tmp = calloc(ncl, sizeof(struct  STORE_STATICs));
        create_canonical_data_structure();

        NbElPM = calloc(ncl, sizeof(TYPE));
        PM = GetMatRepOfClParts(MTCL, NbElPM) ;

        new_part(&PM, &NbElPM, MTCL);
        set_ref(PM, NbElPM);
        initial_statics = NewTabc(PM, NbElPM, MTCL);

        SPECIAL_CANISATION();
        int ms = mark_ordinarie;

        MERG = MERG_ALL = TO_MERGE(ncl, tabc);
        GROUPING_ALL_STATICS(MERG_ALL, tabc, num);
        Sym_StaticConf = CREATE_STORE_STATIC_STRUCTURE();
        NEW_SIM_STATIC_CONF(&Sym_StaticConf);
        STORE_CARD(card, & STORED_CARD);
        STORE_STATIC_CONF(&tmp, tabc);
        GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num,
                             &tot, Sym_StaticConf, MERG_ALL);
        FreeStoreStructs(tmp);
        ALL_MY_GROUP(&MERGING_MAPPE);
        SPECIAL_CANISATION();
        AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);
        /*** end compute symetrical representation ********/
        insert_tree(&root, &h, f_mark, length, d_ptr);

        /******* Compute the eventuality, *******************/
        /******* corresponding to the asym�tric marking *****/
        GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE,
                               initial_statics, STORED_CARD, MERG);
        /******************************************************/

        INIT_ARRAYS(MERG_ALL, num, card, initial_statics);
        EVENTUALITIE_TO_STRING(Sym_StaticConf);
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        reached_marking->marking->cont_tang = NB_SR ; NB_SR++;
        reached_marking->Head_Next_Event =
            (Tree_p)treenode_pop(FILE_POS_PTR, LEGTH_OF_CACHE, 0);
        reached_marking->Head_Next_Event->Head_Next_Event = NULL;
        reached_marking->Head_Next_Event->PM = PM;
        reached_marking->Head_Next_Event->NbElPM = NbElPM;
        reached_marking->Head_Next_Event->Parent = reached_marking;
        reached_marking->Head_Next_Event->marking->cont_tang = MARKING_COUNTER + 1;
        MARKING_COUNTER++;
        reached_marking->Head_Next_Event->marking->ordinary = ms;
        inqueue(&top, &bottom, reached_marking->Head_Next_Event);


        if (out_mc) {
            INSERT_POS_STORE(OFFSET_SR, reached_marking, NULL);
            INSERT_POS_STORE(OFFSET_EVENT, reached_marking,
                             reached_marking->Head_Next_Event);
        }

        FreeMerg(MERG_ALL);
        FREE_ALL_LMS_ELEMS();
        initial_marking =  top;
        esm_count++;
    }

    *M0 = initial_marking ;

    return 0;
}

double
Transition_Rate(Event_p  ev_p) {
    return (double)(ev_p->enabling_degree * tabt[ev_p->trans].mean_t);
}


void
TEST_AND_ADD_MC(int nb_source, int source, MarkSym **tab_sym,
                int *Nbtabsym, int ord_class,
                int ord_event, int ord_new, Event_p ev_p) {
    Tree_Esrg_p prec, ptr, cur;
    int i = 0;

    while (i < (*Nbtabsym))
        if (my_strcmp_modified((*tab_sym)[i].marking_as_string,
                               (*tab_sym)[i].length, length
                              ) == 0
           ) {
            if ((cur = TEST_EXISTENCE((*tab_sym)[i].Eventp, &prec)) == NULL) {
                ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
                if (prec)
                    prec->Head_Next_Event = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
                else
                    prec = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);

                ptr->marking->ordinary = ord_new;
                ptr->input_tr = (double *)calloc(nb_source, sizeof(double));
                ptr->input_tr[source] = ((double) ord_event / (double) ord_class) *
                                        (double) ev_p->ordinary_instances *
                                        Transition_Rate(ev_p) ;
                return ;
            }
            else {
                cur->input_tr[source] += ((double) ord_event / (double) ord_class) *
                                         (double) ev_p->ordinary_instances *
                                         Transition_Rate(ev_p) ;
                return ;
            }
        }
        else i++ ;

    ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
    (*tab_sym) = realloc((*tab_sym), ((*Nbtabsym) + 1) * sizeof(MarkSym));
    (*tab_sym)[(*Nbtabsym)].marking_as_string = calloc(length, sizeof(char));
    (*tab_sym)[(*Nbtabsym)].ord = mark_ordinarie;
    for (i = 0; i < length; i++)
        (*tab_sym)[(*Nbtabsym)].marking_as_string[i] = cache_string[i];

    (*tab_sym)[(*Nbtabsym)].length = length;
    (*tab_sym)[(*Nbtabsym)].d_ptr = d_ptr;
    (*tab_sym)[(*Nbtabsym)].Eventp = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);

    ptr->marking->ordinary = ord_new;
    ptr->input_tr = (double *)calloc(nb_source, sizeof(double));
    ptr->input_tr[source] = ((double) ord_event / (double) ord_class) *
                            (double) ev_p->ordinary_instances *
                            Transition_Rate(ev_p) ;


    (*Nbtabsym)++;
}


int
local_seach_string_marking(Tree_Esrg_p  sm, int sm_size,
                           char *s_string, int s_size) {

    int i;
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    for (i = 0; i < sm_size; i++)
        if (s_size == sm[i].length_for_prob) {
            cp = sm[i].marking_for_prob ;
            pp = s_string ;
            for (ii = s_size; ii; ii--) {
                op1 = *cp; cp++;
                op2 = *pp; pp++;
                if (op1 != op2)
                    break;
            }
            if (ii == 0) {

                return EQUAL;
            }
        }

    return NOT_EQUAL;;
}


int local_copy_cache_string(char **new_string) {
    int i;
    (*new_string) = (char *)calloc(length, sizeof(char));

    for (i = 0; i < length; i++)
        (*new_string)[i] = cache_string[i];
    return length;
}

void local_free_temp_ord(Tree_Esrg_p  sr_ordinary, int sr_size_ordinary) {
    int i;
    for (i = 0; i < sr_size_ordinary; i++)
        if (sr_ordinary[i].length_for_prob != 0) {
            free(sr_ordinary[i].marking_for_prob);
            sr_ordinary[i].marking_for_prob = NULL;
            sr_ordinary[i].length_for_prob = 0;
        }
}

void
free_temp_marking(Tree_Esrg_p  eventuality) {
    Tree_Esrg_p p = eventuality;
    while (p) {
        free(p->marking_for_prob);
        p->marking_for_prob = NULL;
        p->length_for_prob = 0;
        p = p->Head_Next_Event;
    }
}

Tree_Esrg_p NEW_TEST_EXISTENCE(Tree_Esrg_p List_Events,
                               char *marking, int size,
                               Tree_Esrg_p *prec) {
    int i;
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    (*prec) = List_Events;
    while (List_Events) {
        if (size == List_Events->length_for_prob) {
            cp = List_Events-> marking_for_prob ;
            pp = marking ;
            for (ii = size; ii; ii--) {
                op1 = *cp; cp++;
                op2 = *pp; pp++;
                if (op1 != op2)
                    break;
            }
            if (ii == 0) {
                return List_Events ;
            }
        }

        (*prec) = List_Events;
        List_Events = List_Events->Head_Next_Event;
    }

    return NULL;
}


void
NEW_TEST_AND_ADD_MC(int nb_source, int source, MarkSym **tab_sym,
                    int *Nbtabsym, int ord_class, int ord_event,
                    char *marking, int size,
                    int ord_new, Event_p ev_p) {

    Tree_Esrg_p prec, ptr, cur;
    int i = 0;

    while (i < (*Nbtabsym))
        if (my_strcmp_modified((*tab_sym)[i].marking_as_string,
                               (*tab_sym)[i].length, length
                              ) == 0
           ) {
            if ((cur = NEW_TEST_EXISTENCE((*tab_sym)[i].Eventp, marking, size, &prec)
                ) == NULL) {
                ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
                if (prec)
                    prec->Head_Next_Event = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
                else
                    prec = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);

                ptr->marking_for_prob = marking;
                ptr->length_for_prob = size;
                ptr->marking->ordinary = ord_new;
                ptr->input_tr = (double *)calloc(nb_source, sizeof(double));
                ptr->input_tr[source] = ((double) ord_event / (double) ord_class) *
                                        (double) ev_p->ordinary_instances *
                                        Transition_Rate(ev_p) ;
                return ;
            }
            else {
                free(marking);
                cur->input_tr[source] += ((double) ord_event / (double) ord_class) *
                                         (double) ev_p->ordinary_instances *
                                         Transition_Rate(ev_p) ;
                return ;
            }
        }
        else i++ ;

    ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
    (*tab_sym) = realloc((*tab_sym), ((*Nbtabsym) + 1) * sizeof(MarkSym));
    (*tab_sym)[(*Nbtabsym)].marking_as_string = calloc(length, sizeof(char));
    (*tab_sym)[(*Nbtabsym)].ord = mark_ordinarie;
    for (i = 0; i < length; i++)
        (*tab_sym)[(*Nbtabsym)].marking_as_string[i] = cache_string[i];

    (*tab_sym)[(*Nbtabsym)].length = length;
    (*tab_sym)[(*Nbtabsym)].d_ptr = d_ptr;
    (*tab_sym)[(*Nbtabsym)].Eventp = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);

    ptr->marking->ordinary = ord_new;
    ptr->input_tr = (double *)calloc(nb_source, sizeof(double));
    ptr->input_tr[source] = ((double) ord_event / (double) ord_class) *
                            (double) ev_p->ordinary_instances *
                            Transition_Rate(ev_p) ;
    ptr->marking_for_prob = marking;
    ptr->length_for_prob = size;
    (*Nbtabsym)++;
}

void
NEW_TEST_AND_ADD(MarkSym **tab_sym, int *Nbtabsym,
                 char *marking, int size,
                 int ord_new) {
    Tree_Esrg_p prec, ptr, cur;
    int i = 0;

    while (i < (*Nbtabsym))
        if (my_strcmp_modified((*tab_sym)[i].marking_as_string,
                               (*tab_sym)[i].length, length
                              ) == 0
           ) {
            if ((cur = NEW_TEST_EXISTENCE((*tab_sym)[i].Eventp, marking, size, &prec))
                    == NULL
               ) {
                ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
                if (prec)
                    prec->Head_Next_Event = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
                else
                    prec = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);


                ptr->marking_for_prob = marking;
                ptr->length_for_prob = size;
                ptr->marking->ordinary = ord_new;
                ptr->input_tr = NULL;
                return ;
            }
            else {
                free(marking);
                return ;
            }
        }
        else i++ ;

    ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
    (*tab_sym) = realloc((*tab_sym), ((*Nbtabsym) + 1) * sizeof(MarkSym));
    (*tab_sym)[(*Nbtabsym)].marking_as_string = calloc(length, sizeof(char));
    (*tab_sym)[(*Nbtabsym)].ord = mark_ordinarie;
    for (i = 0; i < length; i++)
        (*tab_sym)[(*Nbtabsym)].marking_as_string[i] = cache_string[i];

    (*tab_sym)[(*Nbtabsym)].length = length;
    (*tab_sym)[(*Nbtabsym)].d_ptr = d_ptr;
    (*tab_sym)[(*Nbtabsym)].Eventp = ptr = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);

    ptr->marking->ordinary = ord_new;
    ptr->input_tr = NULL;
    ptr->marking_for_prob = marking;
    ptr->length_for_prob = size;
    (*Nbtabsym)++;
}



void change_marking_stack(State s) {
    if (s->nbtr)
        inqueue(&top_ins, &bottom_ins, s);
}

void d_srg_enabling_and_guards(State s) {

    STORE_STATICS_ARRAY s_StaticConf = NULL;
    s_StaticConf = NewTabc(s->PM, s->NbElPM, MTCL);
    POP_STATIC_CONF(s_StaticConf, &tabc);
    MERG = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG, tabc, num);

    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    string_to_marking(s->Parent->marking->marking_as_string,
                      s->Parent->marking->d_ptr,
                      s->Parent->marking->length);

    /********** load the s'th eventuality from  the file *****/
    GET_EVENTUALITIE_FROM_FILE(s->marking->marking_as_string,
                               s->marking->length);
    COPY_CACHE(s->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);

    /****** instanciate the marking s with the eventuality i */
    NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num,
                     &tot, MERG, s_StaticConf, RESULT);
    /*********************************************************/

    enabled_head = NULL;
    my_initialize_en_list();
    /***********************************************************/

    Result_p last = enabled_head, next = NULL, ptr = enabled_head, s_ptr;
    int *tabtrans = NULL;
    int nbtr = 0;

    s->asym = false;
    s->max_pri = 0;
    if (!out_mc) {
        while (ptr) {
            next = ptr->next;
            if ((tabt[ptr->list->trans].dyn_guard &&
                    tabt[ptr->list->trans].dyn_guard->PART_MAT) ||
                    (tabt[ptr->list->trans].pri < cur_priority)
               ) {
                tabtrans = (int *)realloc(tabtrans, (nbtr + 1) * sizeof(int));
                tabtrans[nbtr] = ptr->list->trans;
                nbtr++;

                if (ptr == enabled_head)
                    last = enabled_head = enabled_head->next;
                else
                    last->next = ptr->next;

                Event_p ev = ptr->list, ev1;
                while (ev) {
                    ev1 = ev->next;
                    dispose_old_event(ev);

                    ev = ev1;
                }

                push_result(ptr);
            }
            else {
                last = ptr;
                if (tabt[ptr->list->trans].pri ==  cur_priority)
                    s->max_pri = cur_priority;
            }

            ptr = next;
        }
        if (nbtr) s->asym = true;
        s->enabled_head = enabled_head;
    }
    else {
        while (ptr) {
            next = ptr->next;
            tabtrans = (int *)realloc(tabtrans, (nbtr + 1) * sizeof(int));
            tabtrans[nbtr] = ptr->list->trans;
            nbtr++;

            /****************** New Optmization **************/
            if (tabt[ptr->list->trans].dyn_guard &&
                    tabt[ptr->list->trans].dyn_guard->PART_MAT)
                s->asym = true;
            /************************************************/

            Event_p ev = ptr->list, ev1;
            while (ev) {
                ev1 = ev->next;
                dispose_old_event(ev);
                ev = ev1;
            }
            push_result(ptr);
            ptr = next;
        }

        s->max_pri = cur_priority;
        s->enabled_head = NULL;
    }

    s->marking->pri = cur_priority;
    s->asm_tr = tabtrans;
    s->nbtr = nbtr;
    FreeMerg(MERG);
    FREE_ALL_LMS_ELEMS();
}

int
d_srg_stack(Tree_p marking, Tree_p head) {
    Tree_p ptr = head;

    while (ptr)
        if (ptr == marking)
            return true;
        else
            ptr = ptr->last;

    return false;
}

void
d_srg_inqueue_stack(Tree_p ins_node, int cas) {
    switch (cas) {

    case INSERT_SAT :
        inqueue(&top_sat, &bottom_sat, ins_node);
        break;

    case INSERT_NO_SAT :
        inqueue(&top_ins, &bottom_ins, ins_node);
        break;
    }
}

void d_srg_pop(Tree_p *top) {

    State st = (*top);

    if (top_sat == (*top)) {

        (*top) = top_sat = top_sat->last;

        if (st->nbtr)
            inqueue(&top_ins, &bottom_ins, st);

        if (!top_sat)(*top) = top_ins;

        return ;
    }

    if (top_ins == (*top)) {

        (*top) = top_ins = top_ins->last;

        if (st->nbtr)
            inqueue(&top_ins, &bottom_ins, st);

        if (top_sat)(*top) = top_sat;

        return ;
    }

}

void
asym_d_srg_pop(Tree_p *top) {

    State st = (*top);

    if (top_ins == (*top)) {
        (*top) = top_ins = top_ins->last;

        if (!top_ins)
            (*top) = top_sat;

        return ;
    }

    if (top_sat == (*top)) {

        (*top) = top_sat = top_sat->last;

        if (top_ins)
            (*top) = top_ins;

        return ;
    }
}

void
dispache_succs(Tree_p succ) {

    Tree_p top = succ, next = NULL;
    char *st;

    if (top) {
        while (top) {

            next = top->last;
            d_srg_enabling_and_guards(top);

            if (!top->asym)
                d_srg_inqueue_stack(top, INSERT_SAT);
            else
                d_srg_inqueue_stack(top, INSERT_NO_SAT);

            top = next;
        }
    }
}


void mc_dsrg_insert_tree(int *** event, double rate,
                         int ord, STORE_STATICS_ARRAY asym_static_conf,
                         TYPE_P **PART_MAT, TYPE_P NbElPM, int *flg) {

    Tree_p ptr = NULL, new_ptr = NULL, last_ptr = NULL;
    PSUPPORT Supp1 = NULL, Supp2 = NULL;
    int new_mark_inc_old_mark, old_mark_inc_new_mark;
    int stack_sym, stack_asym, found = false;
    char cache[MAX_CACHE];
    int count = 0;
    int desc = fileno(TEMP_EVENT);
    ftruncate(desc, 0);

    add_free_candidate_part(PART_MAT, NbElPM);

    ptr = reached_marking->Head_Next_Event;
    while (ptr) {
        Tree_Esrg_p  event_arry_event_inclued = NULL;
        int          size_arry_event_inclued = 0;
        Tree_Esrg_p  event_arry_event_inclued_in = NULL;
        int          size_arry_event_inclued_in = 0;
        PART_MAT_P PM = NULL;

        if (ptr->PM != PART_MAT) {

            PM = GetRefinedPartitions(ptr->PM , ptr->NbElPM, PART_MAT, NbElPM);
            add_free_candidate_part(PM->PART_MAT , PM->NbElPM);

            GET_EVENTUALITIE_FROM_FILE(ptr->marking->marking_as_string,
                                       ptr->marking->length);
            COPY_CACHE(ptr->marking->length);
            STRING_TO_EVENTUALITIE(Sym_StaticConf);

            get_refined_eventualities(ptr->PM, ptr->NbElPM,
                                      PM->PART_MAT , PM->NbElPM,
                                      &event_arry_event_inclued_in,
                                      &size_arry_event_inclued_in);

            INIT_RESULT_STRUCT(event);
            get_refined_eventualities(PART_MAT, NbElPM,
                                      PM->PART_MAT, PM->NbElPM,
                                      &event_arry_event_inclued,
                                      &size_arry_event_inclued);

            new_mark_inc_old_mark  = Inclusion(event_arry_event_inclued,
                                               size_arry_event_inclued,
                                               event_arry_event_inclued_in,
                                               size_arry_event_inclued_in);

            old_mark_inc_new_mark  = Inclusion(event_arry_event_inclued_in,
                                               size_arry_event_inclued_in,
                                               event_arry_event_inclued,
                                               size_arry_event_inclued);
        }
        else {

            TO_STORE_INIT_ARRAYS(asym_static_conf);
            INIT_RESULT_STRUCT(event);
            EVENTUALITIE_TO_STRING(Sym_StaticConf);
            ADD_TEMP_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE, TEMP_EVENT) ;

            if (NEW_COMPARE_EVENTUALITIES_TEMP(ptr->marking->marking_as_string,
                                               ptr->marking->length , EVENT_MARK,
                                               FILE_POS_PTR, LEGTH_OF_CACHE,
                                               TEMP_EVENT) == EQUAL)
                new_mark_inc_old_mark = old_mark_inc_new_mark = TRUE;
            else
                new_mark_inc_old_mark = old_mark_inc_new_mark = FALSE;

        }


        if (new_mark_inc_old_mark &&
                ((incl == COMPL) || (incl == PART) || old_mark_inc_new_mark)
           ) {

            if (ptr->PM != PART_MAT) {
                Free_events(event_arry_event_inclued_in, size_arry_event_inclued_in);
                Free_events(event_arry_event_inclued, size_arry_event_inclued);
            }

            //      if(output_flag)
            if (!old_mark_inc_new_mark)
                print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 2);
            else
                print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 0);

            return ;
        }


        if (ptr->PM != PART_MAT) {
            Free_events(event_arry_event_inclued_in, size_arry_event_inclued_in);
            Free_events(event_arry_event_inclued, size_arry_event_inclued);
        }

        last_ptr = ptr;
        ptr = ptr->Head_Next_Event;
        count++;
    }

    if (last_ptr != NULL) {
        TO_STORE_INIT_ARRAYS(asym_static_conf);
        INIT_RESULT_STRUCT(event);
        EVENTUALITIE_TO_STRING(Sym_StaticConf);
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        last_ptr->Head_Next_Event =
            (Tree_p)my_treenode_pop(FILE_POS_PTR, LEGTH_OF_CACHE, 0);
        last_ptr->Head_Next_Event->Head_Next_Event = NULL;
        set_ref(PART_MAT, NbElPM);
        last_ptr->Head_Next_Event->PM = PART_MAT;
        last_ptr->Head_Next_Event->NbElPM = NbElPM;
        last_ptr->Head_Next_Event->Parent = reached_marking;
        last_ptr->Head_Next_Event->dead = 1;
        last_ptr->Head_Next_Event->type = TANGIBLE;
        last_ptr->Head_Next_Event->marking->ordinary = ord;
        inqueue(&top, &bottom, last_ptr->Head_Next_Event);

        last_ptr->Head_Next_Event->marking->cont_tang = MARKING_COUNTER + 1;
        MARKING_COUNTER++;
        if (count >= reached_marking->marking->ordinary) {
            COUNTERS ++;
        }

        if (out_mc)
            INSERT_POS_STORE(OFFSET_EVENT, reached_marking, last_ptr->Head_Next_Event);

        print_dsrg(OUTPUT_FILE, d_srg_top, last_ptr->Head_Next_Event, rate, 0);


        *flg = false;
    }
    else {

        INIT_RESULT_STRUCT(event);
        TO_STORE_INIT_ARRAYS(asym_static_conf);
        EVENTUALITIE_TO_STRING(Sym_StaticConf);
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        reached_marking->marking->cont_tang = NB_SR ; NB_SR++;
        reached_marking->Head_Next_Event =
            (Tree_p)my_treenode_pop(FILE_POS_PTR, LEGTH_OF_CACHE, 0);
        reached_marking->Head_Next_Event->Head_Next_Event = NULL;
        set_ref(PART_MAT, NbElPM);
        reached_marking->Head_Next_Event->PM = PART_MAT;
        reached_marking->Head_Next_Event->NbElPM = NbElPM;
        reached_marking->Head_Next_Event->Parent = reached_marking;
        reached_marking->Head_Next_Event->marking->cont_tang = MARKING_COUNTER + 1;
        reached_marking->Head_Next_Event->dead = 1;
        reached_marking->Head_Next_Event->type = TANGIBLE;
        reached_marking->Head_Next_Event->marking->ordinary = ord;
        MARKING_COUNTER++;

        if (count >= reached_marking->marking->ordinary) COUNTERS ++;
        inqueue(&top, &bottom, reached_marking->Head_Next_Event);
        *flg = false;

        if (out_mc) {
            INSERT_POS_STORE(OFFSET_EVENT, reached_marking,
                             reached_marking->Head_Next_Event);
        }

        print_dsrg(OUTPUT_FILE, d_srg_top,
                   reached_marking->Head_Next_Event, rate, 0);
    }
    return ;
}

void dsrg_insert_tree(int *** event,
                      double rate,
                      int ord,
                      STORE_STATICS_ARRAY asym_static_conf,
                      TYPE_P **PART_MAT,
                      TYPE_P NbElPM,
                      int *flg) {

    Tree_p ptr = NULL, new_ptr = NULL, last_ptr = NULL;
    PSUPPORT Supp1 = NULL, Supp2 = NULL;
    int new_mark_inc_old_mark, old_mark_inc_new_mark;
    int stack_sym, stack_asym, found = false;
    char cache[MAX_CACHE];
    int *disc = NULL;
    int nb_disc = 0;

    int desc = fileno(TEMP_EVENT);
    ftruncate(desc, 0);

    if (tro == NEW_MARKING) {

        INIT_RESULT_STRUCT(event);
        TO_STORE_INIT_ARRAYS(asym_static_conf);
        EVENTUALITIE_TO_STRING(Sym_StaticConf);
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        reached_marking->marking->cont_tang = NB_SR ; NB_SR++;
        reached_marking->Head_Next_Event =
            (Tree_p)my_treenode_pop(FILE_POS_PTR, LEGTH_OF_CACHE, 0);
        reached_marking->Head_Next_Event->Head_Next_Event = NULL;
        reached_marking->Head_Next_Event->PM = PART_MAT;
        reached_marking->Head_Next_Event->NbElPM = NbElPM;
        reached_marking->Head_Next_Event->Parent = reached_marking;
        reached_marking->Head_Next_Event->marking->cont_tang = MARKING_COUNTER + 1;
        reached_marking->Head_Next_Event->dead = 1;
        reached_marking->Head_Next_Event->type = TANGIBLE;
        reached_marking->Head_Next_Event->marking->ordinary = ord;
        MARKING_COUNTER++;
        inqueue(&top, &bottom, reached_marking->Head_Next_Event);
        *flg = false;

        if (output_flag)
            print_dsrg(OUTPUT_FILE, d_srg_top, reached_marking->Head_Next_Event, rate, 0);

        return ;
    }
    else {
        ptr = reached_marking->Head_Next_Event;
        while (ptr) {
            int test = FALSE;
            Tree_Esrg_p  event_arry_event_inclued = NULL;
            int          size_arry_event_inclued = 0;
            Tree_Esrg_p  event_arry_event_inclued_in = NULL;
            int          size_arry_event_inclued_in = 0;
            PART_MAT_P   PM = NULL;

            if (ptr->PM != PART_MAT) {
                PM = GetRefinedPartitions(ptr->PM , ptr->NbElPM, PART_MAT, NbElPM);
            }
            else {
                PM = malloc(sizeof(struct Part_Mat));
                PM->PART_MAT = ptr->PM;
                PM->NbElPM  = ptr->NbElPM;
                test = TRUE;
            }

            GET_EVENTUALITIE_FROM_FILE(ptr->marking->marking_as_string,
                                       ptr->marking->length);
            COPY_CACHE(ptr->marking->length);
            STRING_TO_EVENTUALITIE(Sym_StaticConf);

            get_refined_eventualities(ptr->PM, ptr->NbElPM, PM->PART_MAT , PM->NbElPM,
                                      &event_arry_event_inclued_in,
                                      &size_arry_event_inclued_in);
            INIT_RESULT_STRUCT(event);
            get_refined_eventualities(PART_MAT, NbElPM, PM->PART_MAT , PM->NbElPM,
                                      &event_arry_event_inclued,
                                      &size_arry_event_inclued);

            new_mark_inc_old_mark  = Inclusion(event_arry_event_inclued,
                                               size_arry_event_inclued,
                                               event_arry_event_inclued_in,
                                               size_arry_event_inclued_in);

            old_mark_inc_new_mark  = Inclusion(event_arry_event_inclued_in,
                                               size_arry_event_inclued_in,
                                               event_arry_event_inclued,
                                               size_arry_event_inclued);

            if (incl == COMPL) {

                if (new_mark_inc_old_mark && old_mark_inc_new_mark) {

                    if (test) free(PM);

                    Free_events(event_arry_event_inclued_in,
                                size_arry_event_inclued_in);
                    Free_events(event_arry_event_inclued,
                                size_arry_event_inclued);

                    if (output_flag)
                        print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 0);

                    return ;
                }

                if (new_mark_inc_old_mark &&
                        !old_mark_inc_new_mark) {

                    if (test) free(PM);

                    Free_events(event_arry_event_inclued_in,
                                size_arry_event_inclued_in);
                    Free_events(event_arry_event_inclued,
                                size_arry_event_inclued);

                    if (output_flag)
                        print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 2);

                    return ;
                }

                stack_sym  = d_srg_stack(ptr, top_sat);
                stack_asym = d_srg_stack(ptr, top_ins);

                if (old_mark_inc_new_mark && (stack_sym || stack_asym)) {
                    if (stack_sym) retreve(&top_sat, &bottom_sat, ptr);
                    if (stack_asym) retreve(&top_ins, &bottom_ins, ptr);

                    // disc=(int*)realloc(disc,(nb_disc+1)*sizeof(int));
                    // disc[nb_disc]=ptr->marking->cont_tang;
                    // nb_disc++;
                    ////////////////////////////////////////////////////
                    //// Note : il faut les supprimer du graph /////////
                    ////////////////////////////////////////////////////
                    MARKING_COUNTER--;
                }
            }

            if (incl == PART) {
                if (new_mark_inc_old_mark)
                    if (old_mark_inc_new_mark) {
                        if (test) free(PM);
                        Free_events(event_arry_event_inclued_in, size_arry_event_inclued_in);
                        Free_events(event_arry_event_inclued, size_arry_event_inclued);

                        if (output_flag)
                            print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 0);
                        return ;
                    }
                    else if (ptr->asym)
                        found = true;

            }

            if (incl == NO) {
                if (new_mark_inc_old_mark &&  old_mark_inc_new_mark) {
                    if (test) free(PM);
                    Free_events(event_arry_event_inclued_in,
                                size_arry_event_inclued_in);
                    Free_events(event_arry_event_inclued,
                                size_arry_event_inclued);

                    if (output_flag)
                        print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 0);

                    return ;
                }
            }

            if (test) free(PM);
            Free_events(event_arry_event_inclued_in,
                        size_arry_event_inclued_in);
            Free_events(event_arry_event_inclued,
                        size_arry_event_inclued);
            last_ptr = ptr;
            ptr = ptr->Head_Next_Event;
        }

        TO_STORE_INIT_ARRAYS(asym_static_conf);
        INIT_RESULT_STRUCT(event);
        EVENTUALITIE_TO_STRING(Sym_StaticConf);
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        last_ptr->Head_Next_Event = (Tree_p)my_treenode_pop
                                    (FILE_POS_PTR, LEGTH_OF_CACHE, 0);
        last_ptr->Head_Next_Event->Head_Next_Event = NULL;
        last_ptr->Head_Next_Event->PM = PART_MAT;
        last_ptr->Head_Next_Event->NbElPM = NbElPM;
        last_ptr->Head_Next_Event->Parent = reached_marking;
        last_ptr->Head_Next_Event->dead = 1;
        last_ptr->Head_Next_Event->type = TANGIBLE;
        last_ptr->Head_Next_Event->marking->ordinary = ord;

        if (!found)
            inqueue(&top, &bottom, last_ptr->Head_Next_Event);

        last_ptr->Head_Next_Event->marking->cont_tang = MARKING_COUNTER + 1;
        MARKING_COUNTER++;

        *flg = false;

        if (output_flag) {

            print_dsrg(OUTPUT_FILE, d_srg_top,
                       last_ptr->Head_Next_Event,
                       rate, 0);

            if (disc) {
                int i;
                for (i = 0; i < nb_disc; i++) {
                    if (disc[i] != last_ptr->Head_Next_Event->marking->cont_tang)
                        fprintf(OUTPUT_FILE , "S%d -> S%d [style=dotted,dir=none];\n",
                                disc[i],
                                last_ptr->Head_Next_Event->marking->cont_tang, 0);
                }
                free(disc);
            }
        }
        return ;
    }

}

PART_MAT_P
GetRefinedMultPartitions(Tree_p ptr, TYPE_P **PART_MAT, TYPE_P NbElPM) {

    PART_MAT_P PM = NULL;
    PM = GetRefinedPartitions(ptr->PM , ptr->NbElPM, PART_MAT, NbElPM);
    add_free_candidate_part(PM->PART_MAT , PM->NbElPM);
    ptr = ptr->Head_Next_Event;

    while (ptr) {
        PM = GetRefinedPartitions(ptr->PM , ptr->NbElPM, PM->PART_MAT , PM->NbElPM);
        add_free_candidate_part(PM->PART_MAT, PM->NbElPM);
        ptr = ptr->Head_Next_Event;
    }
    return PM;
}

int pre_mc_dsrg_insert_tree(Tree_Esrg_p event, double rate,
                            int ord, STORE_STATICS_ARRAY asym_static_conf,
                            TYPE_P **PART_MAT, TYPE_P NbElPM, int *inc) {

    Tree_p ptr = NULL, new_ptr = NULL, last_ptr = NULL;
    PSUPPORT Supp1 = NULL, Supp2 = NULL;
    int new_mark_inc_old_mark, old_mark_inc_new_mark;
    int stack_sym, stack_asym, found = false;
    char cache[MAX_CACHE];
    int desc = fileno(TEMP_EVENT);

    ftruncate(desc, 0);
    add_free_candidate_part(PART_MAT, NbElPM);

    if (tro == NEW_MARKING) {
        return FALSE;
    }
    else {
        Tree_Esrg_p  event_arry_event_inclued = NULL;
        int          size_arry_event_inclued = 0;
        Tree_Esrg_p  event_arry_event_inclued_in = NULL;
        int          size_arry_event_inclued_in = 0;
        PART_MAT_P   PM = NULL;

        ptr = reached_marking->Head_Next_Event;
        PM = GetRefinedMultPartitions(ptr, PART_MAT, NbElPM);
        GET_EVENTUALITIE_FROM_FILE(event->marking->marking_as_string,
                                   event->marking->length);
        COPY_CACHE(event->marking->length);
        STRING_TO_EVENTUALITIE(Sym_StaticConf);

        get_refined_eventualities(PART_MAT, NbElPM,
                                  PM->PART_MAT , PM->NbElPM,
                                  &event_arry_event_inclued ,
                                  &size_arry_event_inclued);
        while (ptr) {

            if (ptr->PM != PART_MAT) {

                event_arry_event_inclued_in = NULL;
                size_arry_event_inclued_in = 0;

                GET_EVENTUALITIE_FROM_FILE(ptr->marking->marking_as_string,
                                           ptr->marking->length);
                COPY_CACHE(ptr->marking->length);
                STRING_TO_EVENTUALITIE(Sym_StaticConf);

                get_refined_eventualities(ptr->PM, ptr->NbElPM,
                                          PM->PART_MAT , PM->NbElPM,
                                          &event_arry_event_inclued_in ,
                                          &size_arry_event_inclued_in);


                new_mark_inc_old_mark = Inclusion(event_arry_event_inclued,
                                                  size_arry_event_inclued,
                                                  event_arry_event_inclued_in,
                                                  size_arry_event_inclued_in);

                old_mark_inc_new_mark = Inclusion(event_arry_event_inclued_in,
                                                  size_arry_event_inclued_in,
                                                  event_arry_event_inclued,
                                                  size_arry_event_inclued);

            }
            else {
                if (NEW_COMPARE_EVENTUALITIES(ptr->marking->marking_as_string,
                                              ptr->marking->length,
                                              event->marking->marking_as_string,
                                              event->marking->length,
                                              EVENT_MARK) == EQUAL)
                    new_mark_inc_old_mark = old_mark_inc_new_mark = TRUE;
                else
                    new_mark_inc_old_mark = old_mark_inc_new_mark = FALSE;
            }


            if (new_mark_inc_old_mark &&
                    ((incl == COMPL) || (incl == PART) ||
                     old_mark_inc_new_mark)
               ) {

                Free_events(event_arry_event_inclued , size_arry_event_inclued);
                if (ptr->PM != PART_MAT)
                    Free_events(event_arry_event_inclued_in, size_arry_event_inclued_in);

                //	if(output_flag)
                if (!old_mark_inc_new_mark)
                    print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 2);
                else
                    print_dsrg(OUTPUT_FILE, d_srg_top, ptr, rate, 0);

                return TRUE;
            }

            if (old_mark_inc_new_mark && !new_mark_inc_old_mark) {

                Tree_Esrg_p list_diff = Get_Diff_Mark_DSRG(event_arry_event_inclued_in,
                                        size_arry_event_inclued_in,
                                        event_arry_event_inclued,
                                        size_arry_event_inclued);

                Free_events(event_arry_event_inclued_in,
                            size_arry_event_inclued_in);
                event_arry_event_inclued_in = NULL ;

                event_arry_event_inclued_in = calloc(size_arry_event_inclued -
                                                     size_arry_event_inclued_in,
                                                     sizeof(struct TREE_ESRG)) ;
                size_arry_event_inclued_in = 0;

                while (list_diff) {

                    event_arry_event_inclued_in[size_arry_event_inclued_in].marking =
                        list_diff->marking;
                    event_arry_event_inclued_in[size_arry_event_inclued_in].enabled_head =
                        NULL;
                    event_arry_event_inclued_in[size_arry_event_inclued_in].Head_Next_Event =
                        NULL;
                    size_arry_event_inclued_in++;
                    list_diff = list_diff->Head_Next_Event;

                }

                free(event_arry_event_inclued);
                event_arry_event_inclued = event_arry_event_inclued_in;
                size_arry_event_inclued = size_arry_event_inclued_in;
            }

            if (!old_mark_inc_new_mark)
                if (ptr->PM != PART_MAT)
                    Free_events(event_arry_event_inclued_in, size_arry_event_inclued_in);
            ptr = ptr->Head_Next_Event;
        }

        Free_events(event_arry_event_inclued , size_arry_event_inclued);
    }

    return FALSE;
}


void
timing_prob() {

    Result_p enabled = enabled_head;
    while (enabled) {
        if (enabled->next)
            if (tabt[enabled->list->trans].timing !=
                    tabt[enabled->next->list->trans].timing)
                printf("we have a problem\n");

        enabled = enabled->next;
    }

}

void MARQUAGE() {
    int i;
    int j;
    int k;
    Token_p ptr;

    for (i = 0; i < npl; i++) {
        ptr = net_mark[i].marking;
        printf("\n Place (%s):", tabp[i].place_name);
        while (ptr) {
            printf("%d<", ptr->molt);
            for (k = 0; k < tabp[i].comp_num; k++) printf("%d,", ptr->id[k]);
            ptr = ptr->next;
            if (ptr)printf(">+"); else printf(">");
        }
    }
}

PART_MAT_P
d_srg_asym_succ_arc(State s,
                    MarkSym **TAB_SYM,
                    int *NbTAB_SYM,
                    int comp
                   ) {
    int tr, marking_pri, i, j, first = false, flg;
    Result_p current_transition;
    Result_p next_transition;
    Event_p ev_p, nev_p, copy_of_ev_p;
    char *st;
    int *Order;
    int VAN = false;
    int max_dom = get_max_place_domain();

    PART_MAT_P s_prop_Part  = NULL,
               Disj_Part    = NULL;
    TO_MERGEP *merg_s_prop  = NULL;
    TO_MERGEP *merg_group   = NULL;

    STORE_STATICS_ARRAY s_prop_StaticConf = NULL,
                        gr_s_prop_StaticConf = NULL,
                        old_StaticConf = NULL;

    EVENT_ARRAY = NULL;
    SIZE_OF_EVENT_ARR = 0;

    int desc = fileno(TEMP_EVENT);
    ftruncate(desc, 0);


    if (!comp) {
        s_prop_Part = calloc(s->nbtr + 1, sizeof(PART_MAT));
        s_prop_Part[0].PART_MAT = s->PM;
        s_prop_Part[0].NbElPM  = s->NbElPM;

        for (i = 0, j = 1; i < s->nbtr; i++)
            if (tabt[s->asm_tr[i]].dyn_guard) {
                s_prop_Part[j].PART_MAT = tabt[s->asm_tr[i]].dyn_guard->PART_MAT;
                s_prop_Part[j].NbElPM = tabt[s->asm_tr[i]].dyn_guard->NbElPM ;
                j++;
            }
    }
    else {
        s_prop_Part = calloc(ntr + 1, sizeof(PART_MAT));
        s_prop_Part[0].PART_MAT = s->PM;
        s_prop_Part[0].NbElPM  = s->NbElPM;

        for (i = 0, j = 1; i < ntr; i++)
            if (tabt[i].dyn_guard) {
                s_prop_Part[j].PART_MAT = tabt[i].dyn_guard->PART_MAT;
                s_prop_Part[j].NbElPM = tabt[i].dyn_guard->NbElPM ;
                j++;
            }
    }

    Disj_Part = FindDisjParts3(s_prop_Part, j);
    free(s_prop_Part);
    s_prop_Part = Disj_Part;

    //load eventuality in string format
    GET_EVENTUALITIE_FROM_FILE(s->marking->marking_as_string,
                               s->marking->length);
    COPY_CACHE(s->marking->length);

    // translate the string eventuality in matrix representation
    STRING_TO_EVENTUALITIE(Sym_StaticConf);

    // load partion of the symbolic marking s
    old_StaticConf = NewTabc(s->PM, s->NbElPM, MTCL);
    // refined partition in tabc format
    s_prop_StaticConf = NewTabc(s_prop_Part->PART_MAT,
                                s_prop_Part->NbElPM, MTCL);

    // merg_s_prop structure, identifies the mapping
    // between s parition and s_prop one
    merg_s_prop = ToMergArr(s_prop_Part->PART_MAT, s_prop_Part->NbElPM,
                            s->PM, s->NbElPM);

    // Init. structures to compute refined enentualitis
    RAF_INIT_ARRAYS(merg_s_prop, s_prop_StaticConf);

    // compute refined eventualities
    EVENTUALITIES(0, 0, 1, 1, TRAITEMENT_3, merg_s_prop, old_StaticConf);
    POP_STATIC_CONF(s_prop_StaticConf, &tabc);
    MERG = MERG_ALL = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG_ALL, tabc, num);
    /***** main loop, computes all valid successors from each eventuality ******/

    if (out_mc) {
        tab_corr = (double *)calloc(SIZE_OF_EVENT_ARR, sizeof(double));
        nb_tab_corr = SIZE_OF_EVENT_ARR;
    }

    for (i = 0; i < SIZE_OF_EVENT_ARR; i++) {
        /*********  change the current tabc by the one of s and load s state ***/
        POP_STATIC_CONF(Sym_StaticConf, &tabc);
        string_to_marking(s->Parent->marking->marking_as_string,
                          s->Parent->marking->d_ptr,
                          s->Parent->marking->length);
        /**********************************************************************/

        /******* load the i'th eventuality from the file *********************/
        GET_TEMP_EVENTUALITIE_FROM_FILE(EVENT_ARRAY[i].marking->marking_as_string,
                                        EVENT_ARRAY[i].marking->length, TEMP_EVENT);
        COPY_CACHE(EVENT_ARRAY[i].marking->length);
        STRING_TO_EVENTUALITIE(Sym_StaticConf);
        /***********************************************************************/

        /************* instanciate the marking s with the eventuality i *******/
        NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot,
                         MERG_ALL, s_prop_StaticConf, RESULT);
        /**********************************************************************/
        SPECIAL_CANISATION();
        /**********************************************************************/



        if (local_seach_string_marking(EVENT_ARRAY, i,
                                       cache_string, length) != EQUAL) {

            EVENT_ARRAY[i].length_for_prob =
                local_copy_cache_string(& EVENT_ARRAY[i].marking_for_prob);

            enabled_head = NULL;

            for (j = 0; j < s->nbtr; j++) {
                my_en_list(s->asm_tr[j] + 1);
            }


            enabled_head = trait_gurded_transitions(enabled_head, s_prop_Part);
            if (enabled_head) {
                s->marking->pri = adjust_priority(0/*not signif*/, enabled_head);
                if (!out_mc) {
                    if (s->marking->pri < s->max_pri)
                        s->marking->pri =  s->max_pri;
                }
            }

            EVENT_ARRAY[i].enabled_head = enabled_head;
            current_transition = EVENT_ARRAY[i].enabled_head;

            int mark_event = 0;
            if (out_mc)
                if (current_transition && max_dom > 1)  {
                    mark_event = mark_ordinarie;
                }
                else
                    mark_event = (int)get_ordinary_marking(1);

            while (current_transition != NULL) {
                next_transition = current_transition->next;
                tr = GET_TRANSITION_INDEX(current_transition->list);

                if (tabt[tr].pri == s->marking->pri) {

                    copy_of_ev_p = get_new_event(tr);
                    ev_p = current_transition->list;

                    if (tabt[tr].timing == TIMING_IMMEDIATE && !VAN)
                        VAN = true;

                    while (ev_p != NULL) {
                        nev_p = ev_p->next;
                        copy_event(copy_of_ev_p, ev_p);

                        if (IS_RESET_TRANSITION(tr))
                            reset_to_M0(initial_marking->marking->marking_as_string,
                                        initial_marking->marking->d_ptr,
                                        initial_marking->marking->length, tr);
                        else
                            fire_trans(ev_p);

                        int new_mark = 0;

                        SPECIAL_CANISATION();


                        new_mark = mark_ordinarie;
                        char *marking_string;
                        int marking_size = local_copy_cache_string(& marking_string);

                        /**** begin :  test satisfaction of the property************/

                        /*** begin computation symetrical representation  **************/
                        STORE_CARD(card, & STORED_CARD);
                        GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num,
                                             &tot, Sym_StaticConf, MERG_ALL);
                        ALL_MY_GROUP(&MERGING_MAPPE);
                        SPECIAL_CANISATION();
                        AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);

                        /******* Compute the eventuality,      ********************/
                        /*******  corresponding to the asym�tric marking *****/
                        GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE,
                                               s_prop_StaticConf, STORED_CARD, MERG);
                        /**********************************************************/

                        // tranformation  of the
                        // eventuality to it string forme
                        INIT_ARRAYS(MERG_ALL, num, card, s_prop_StaticConf);
                        EVENTUALITIE_TO_STRING(Sym_StaticConf);

                        /*********************************************************/
                        //  if the symtrical and it eventuality
                        // doesn't exist, add them

                        if (out_mc) {
                            NEW_TEST_AND_ADD_MC(SIZE_OF_EVENT_ARR, i, TAB_SYM, NbTAB_SYM,
                                                s->marking->ordinary, mark_event,
                                                marking_string, marking_size, new_mark, ev_p);
                            tab_corr[i] += ev_p->ordinary_instances * Transition_Rate(ev_p);
                        }
                        else
                            NEW_TEST_AND_ADD(TAB_SYM, NbTAB_SYM, marking_string,
                                             marking_size, new_mark);
                        /****************************************************************/

                        /**** load the tabc structure and load the marking s **********/

                        POP_STATIC_CONF(Sym_StaticConf, &tabc);
                        string_to_marking(s->Parent->marking->marking_as_string,
                                          s->Parent->marking->d_ptr,
                                          s->Parent->marking->length);
                        /**********************************************************************/

                        /******* load the i'th eventuality from the file *********************/
                        GET_TEMP_EVENTUALITIE_FROM_FILE(EVENT_ARRAY[i].marking->marking_as_string,
                                                        EVENT_ARRAY[i].marking->length, TEMP_EVENT);
                        COPY_CACHE(EVENT_ARRAY[i].marking->length);
                        STRING_TO_EVENTUALITIE(Sym_StaticConf);
                        /***********************************************************************/

                        /************* instanciate the marking s with the eventuality i *******/
                        NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot,
                                         MERG_ALL, s_prop_StaticConf, RESULT);
                        /**********************************************************************/

                        SPECIAL_CANISATION();
                        /**********************************************************************/
                        /*******************************************************************/
                        ev_p = nev_p;
                    }
                    dispose_old_event(copy_of_ev_p);
                }
                current_transition = next_transition;
            }
            my_garbage_collect(&EVENT_ARRAY[i]);
        }
    }

    FreeMerg(merg_s_prop);
    FreeMerg(MERG_ALL);
    FREE_ALL_LMS_ELEMS();
    local_free_temp_ord(EVENT_ARRAY, SIZE_OF_EVENT_ARR);
    Free_EVENT_ARRAY();

    if (s->nbtr) {
        free(s->asm_tr);
        s->asm_tr = NULL;
        s->nbtr = 0;
    }

    if ((*NbTAB_SYM)) s->dead = 0;

    if (out_mc && VAN) {
        s->type = VANISHING;
        VANISHING_COUNT++;
    }

    return s_prop_Part;
}

void
d_srg_sym_succ_arc(State s, MarkSym **TAB_SYM, int *NbTAB_SYM) {

    int tr, marking_pri, i, j, first = false, flg;
    Result_p current_transition;
    Result_p next_transition;
    Event_p ev_p, nev_p, copy_of_ev_p;
    char *st;
    int *Order;
    int VAN = false;
    int max_dom = get_max_place_domain();

    STORE_STATICS_ARRAY   old_StaticConf = NULL;

    // load partion of the symbolic marking s
    old_StaticConf = NewTabc(s->PM, s->NbElPM, MTCL);

    POP_STATIC_CONF(old_StaticConf, &tabc);
    MERG = MERG_ALL = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG_ALL, tabc, num);

    /**********  change the current tabc by the one of s and load s state *******/
    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    string_to_marking(s->Parent->marking->marking_as_string,
                      s->Parent->marking->d_ptr, s->Parent->marking->length);
    /***************************************************************************/

    /********** load the i'th eventuality from the file ************************/
    GET_EVENTUALITIE_FROM_FILE(s->marking->marking_as_string, s->marking->length);
    COPY_CACHE(s->marking->length);
    STRING_TO_EVENTUALITIE(Sym_StaticConf);
    /****************************************************************************/

    /************* instanciate the marking s with the eventuality i *************/
    NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot,
                     MERG_ALL, old_StaticConf, RESULT);
    /****************************************************************************/

    current_transition = s->enabled_head;

    while (current_transition != NULL) {
        next_transition = current_transition->next;
        tr = GET_TRANSITION_INDEX(current_transition->list);

        if (tabt[tr].pri == s->marking->pri) {
            copy_of_ev_p = get_new_event(tr);
            ev_p = current_transition->list;

            if (tabt[tr].timing == TIMING_IMMEDIATE && !VAN)
                VAN = true;

            while (ev_p != NULL) {
                nev_p = ev_p->next;
                copy_event(copy_of_ev_p, ev_p);

                if (IS_RESET_TRANSITION(tr))
                    reset_to_M0(initial_marking->marking->marking_as_string,
                                initial_marking->marking->d_ptr,
                                initial_marking->marking->length, tr);
                else
                    fire_trans(ev_p);

                int new_mark = 0;

                /*   if(out_mc) */
                /* 		if(max_dom > 1)	       */
                /* 		  { */
                /* 		    SPECIAL_CANISATION();  */
                /* 		    new_mark= mark_ordinarie; */
                /* 		  } */
                /* 		else */
                /* 		  new_mark= (int)get_ordinary_marking(1); */

                SPECIAL_CANISATION();
                new_mark = mark_ordinarie;
                char *marking_string;
                int marking_size = local_copy_cache_string(& marking_string);

                /*** begin compute symetrical representation  ****************/
                STORE_CARD(card, & STORED_CARD);
                GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num,
                                     &tot, Sym_StaticConf, MERG_ALL);
                ALL_MY_GROUP(&MERGING_MAPPE);
                SPECIAL_CANISATION();
                AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);
                /*** end compute symetrical representation ******************/

                /************** Compute the eventuality,             *******/
                /************** corresponding to the asym�tric marking *****/
                GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE,
                                       old_StaticConf, STORED_CARD, MERG);
                /**********************************************************/

                /************ tranformation  of an           *****************/
                /************ eventuality to it string forme *****************/
                INIT_ARRAYS(MERG_ALL, num, card, old_StaticConf);
                EVENTUALITIE_TO_STRING(Sym_StaticConf);
                /**************************************************************/

                /*** if the symtrical and it                ***************/
                /**  eventuality doesn't exist, add them    **************/
                NEW_TEST_AND_ADD(TAB_SYM, NbTAB_SYM,
                                 marking_string, marking_size,
                                 new_mark);
                /*********************************************************/

                /**** change the tabc structure and load the markings  ******/

                POP_STATIC_CONF(Sym_StaticConf, &tabc);
                string_to_marking(s->Parent->marking->marking_as_string,
                                  s->Parent->marking->d_ptr,
                                  s->Parent->marking->length);
                /************************************************************/

                /***** load the i'th eventuality from the file *************/
                GET_EVENTUALITIE_FROM_FILE(s->marking->marking_as_string,
                                           s->marking->length);
                COPY_CACHE(s->marking->length);
                STRING_TO_EVENTUALITIE(Sym_StaticConf);
                /***********************************************************/

                /******* instanciate the marking s by the i'th eventuality **/
                NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot,
                                 MERG_ALL, old_StaticConf, RESULT);
                /************************************************************/

                /************************************************************/
                ev_p = nev_p;
            }
            dispose_old_event(copy_of_ev_p);
        }
        current_transition = next_transition;
    }

    garbage_collect(s);
    FreeMerg(MERG_ALL);
    FREE_ALL_LMS_ELEMS();

    if ((*NbTAB_SYM)) s->dead = 0;

    if (out_mc && VAN) {
        s->type = VANISHING;
        VANISHING_COUNT++ ;

    }
}



double probability(Tree_Esrg_p node) {
    int i;
    double input_prob = 0,
           pr = 0;
    for (i = 0; i < nb_tab_corr; i++) {
        pr = node->input_tr[i] / tab_corr[i];
        input_prob += pr;
    }
    return input_prob;
}


int WeakLump_cond(Tree_Esrg_p *EventArr, MarkSym TAB_SYM,
                  STORE_STATICS_ARRAY s_prop_StaticConf,
                  TYPE_P **PM, TYPE_P NbElPM) {
    Tree_Esrg_p ptr, first, p, pred;
    ptr = pred = (*EventArr);
    int i = 0, inc = FALSE;

    /***** load  the symetric configuration on tabc and load a symetric marking */
    POP_STATIC_CONF(Sym_StaticConf, &tabc);
    set_state_cache(TAB_SYM.marking_as_string, TAB_SYM.d_ptr, TAB_SYM.length);
    insert_tree(&root, &h, f_mark, length, d_ptr);
    /****************************************************************************/

    if (tro == NEW_MARKING) {
        esm_count++;


        reached_marking->Head_Next_Event = NULL;
        if (out_mc)
            INSERT_POS_STORE(OFFSET_SR, reached_marking, NULL);
    }

    while (ptr) {
        free(ptr->marking_for_prob);
        if (pre_mc_dsrg_insert_tree(ptr, probability(ptr), ptr->marking->ordinary,
                                    s_prop_StaticConf, PM, NbElPM, &inc)) {
            if (ptr == (*EventArr)) {
                (*EventArr) = (*EventArr)->Head_Next_Event;
                free(ptr->input_tr);
                free(ptr->marking);
                free(ptr);
                ptr = pred = (*EventArr);

            }
            else {
                pred->Head_Next_Event = ptr->Head_Next_Event;
                free(ptr->input_tr);
                free(ptr->marking);
                free(ptr);
                ptr = pred->Head_Next_Event;
            }

        }
        else {
            pred = ptr;
            ptr = ptr->Head_Next_Event;
        }
    }
    ptr = first = (*EventArr);

    if (ptr) {
        ptr->gr = i;
        ptr = ptr->Head_Next_Event;
    }
    while (ptr) {

        int found = 0;

        for (p = first; (p != ptr && !found); p = p->Head_Next_Event)

            if ((probability(ptr) / (double) ptr->marking->ordinary) ==
                    (probability(p) / (double) p->marking->ordinary)
               ) {
                ptr->gr = p->gr;
                found = 1;
            }
        if (!found) {
            ptr->gr = i + 1;
            i++;
        }

        ptr = ptr->Head_Next_Event;

    }
    return i;

}

void mc_dsrg_graph(int comp) {

    int pr, conj, i, j, flg, k = 0;
    PART_MAT_P s_prop_Part = NULL;
    PART_MAT_P Disj_Part = NULL, PM;
    PART_MAT_P gard_part = NULL;
    TO_MERGEP *merg_group = NULL;
    MarkSym   *TAB_SYM = NULL;
    int        NbTAB_SYM = 0;

    STORE_STATICS_ARRAY s_prop_StaticConf = NULL,
                        gr_s_prop_StaticConf = NULL;
    MarkAsEventp list;
    int cmp = 20;
    OUTFILE = fopen("ESM.DSRG", "w+");

    (void)initial_state(&d_srg_top);
    d_srg_enabling_and_guards(d_srg_top);
    top = NULL; top_ins = bottom_ins = d_srg_top; d_srg_top->type = TANGIBLE;

    if (output_flag)
        fprintf(OUTPUT_FILE, "digraph dtmc{\n");

    time(&old_time);
    do {
        TAB_SYM = NULL;
        NbTAB_SYM = 0;
        Disj_Part = d_srg_asym_succ_arc(d_srg_top , &TAB_SYM, &NbTAB_SYM, comp) ;
        s_prop_StaticConf = NewTabc(Disj_Part->PART_MAT, Disj_Part->NbElPM, MTCL);

        if (d_srg_top->dead) {
            DEAD_MARKINGS++;
            if (output_flag)
                print_dsrg(OUTPUT_FILE , d_srg_top, NULL, 0, 1);
        }

        for (i = 0; i < NbTAB_SYM; i++) {
            int nb_groups = 0, nbgr;
            nb_groups = WeakLump_cond(&(TAB_SYM[i].Eventp), TAB_SYM[i],
                                      s_prop_StaticConf,
                                      Disj_Part->PART_MAT,
                                      Disj_Part->NbElPM);

            if (TAB_SYM[i].Eventp)
                for (nbgr = 0; nbgr <= nb_groups; nbgr++) {

                    /***** find valid symbolic marking's groups *******************/
                    GROUP_MARKINGS_MC(TAB_SYM[i].Eventp, s_prop_StaticConf,
                                      Sym_StaticConf, MTCL, nbgr);
                    /**************************************************************/

                    for (j = 0; j < NbResList; j++) {
                        gr_s_prop_StaticConf = NewTabc(ResultList[j]->PART_MAT,
                                                       ResultList[j]->NbElPM, MTCL);
                        POP_STATIC_CONF(gr_s_prop_StaticConf, &tabc);
                        merg_group = TO_MERGE(ncl, tabc);
                        GROUPING_ALL_STATICS(merg_group, tabc, num);
                        list = ResultList[j]->list;
                        flg = true;

                        while (list) {
                            // load  the symetric configuration
                            // on tabc and load a symetric marking.
                            POP_STATIC_CONF(Sym_StaticConf, &tabc);

                            string_to_marking(reached_marking->marking->marking_as_string,
                                              reached_marking->marking->d_ptr,
                                              reached_marking->marking->length);
                            reached_marking->marking->ordinary = TAB_SYM[i].ord;
                            mc_dsrg_insert_tree(list->Event, list->ord * list->rate,
                                                list->ord, gr_s_prop_StaticConf,
                                                ResultList[j]->PART_MAT,
                                                ResultList[j]->NbElPM, &flg);
                            list = list->next;
                        }
                        Free_ResList(j);
                        FREE_ALL_LMS_ELEMS();
                        FreeMerg(merg_group);
                    }

                    Free_DSC_SSC();
                    free(ResultList);
                    ResultList = NULL;
                    NbResList = 0;
                }
        }
        Free_TAB_SYM(TAB_SYM, NbTAB_SYM);
        free(tab_corr);
        nb_tab_corr = 0;
        dispache_succs(top);
        top = NULL;
        asym_d_srg_pop(&d_srg_top);
        hash_table_garbage();
    }
    while (d_srg_top);

    time(&new_time);
    if (output_flag)
        fprintf(OUTPUT_FILE, "}\n");
    print_final_info();
    return ;
}

void mc_dsrg_partial_graph(int comp) {

    int pr, conj, i, j, flg, k = 0;
    PART_MAT_P s_prop_Part = NULL;
    PART_MAT_P Disj_Part = NULL, PM;
    PART_MAT_P gard_part = NULL;
    TO_MERGEP *merg_group = NULL;
    MarkSym   *TAB_SYM = NULL;
    int        NbTAB_SYM = 0;

    STORE_STATICS_ARRAY s_prop_StaticConf = NULL,
                        gr_s_prop_StaticConf = NULL;
    MarkAsEventp list;
    int cmp = 20;



    (void)initial_state(&d_srg_top);
    d_srg_enabling_and_guards(d_srg_top);
    top = NULL; top_ins = bottom_ins = d_srg_top; d_srg_top->type = TANGIBLE;

    if (output_flag)
        fprintf(OUTPUT_FILE, "digraph dtmc{\n");

    time(&old_time);
    do {
        TAB_SYM = NULL;
        NbTAB_SYM = 0;
        Disj_Part = d_srg_asym_succ_arc(d_srg_top , &TAB_SYM, &NbTAB_SYM, comp) ;
        s_prop_StaticConf = NewTabc(Disj_Part->PART_MAT, Disj_Part->NbElPM, MTCL);

        if (d_srg_top->dead) {
            DEAD_MARKINGS++;
            if (output_flag)
                print_dsrg(OUTPUT_FILE , d_srg_top, NULL, 0, 1);
        }

        for (i = 0; i < NbTAB_SYM; i++) {
            int nb_groups = 0, nbgr;
            nb_groups = WeakLump_cond(&(TAB_SYM[i].Eventp), TAB_SYM[i],
                                      s_prop_StaticConf,
                                      Disj_Part->PART_MAT,
                                      Disj_Part->NbElPM);

            if (TAB_SYM[i].Eventp)
                for (nbgr = 0; nbgr <= nb_groups; nbgr++) {

                    /***** find valid symbolic marking's groups *******************/
                    GROUP_MARKINGS_MC(TAB_SYM[i].Eventp, s_prop_StaticConf,
                                      Sym_StaticConf, MTCL, nbgr);
                    /**************************************************************/

                    for (j = 0; j < NbResList; j++) {
                        gr_s_prop_StaticConf = NewTabc(ResultList[j]->PART_MAT,
                                                       ResultList[j]->NbElPM, MTCL);
                        POP_STATIC_CONF(gr_s_prop_StaticConf, &tabc);
                        merg_group = TO_MERGE(ncl, tabc);
                        GROUPING_ALL_STATICS(merg_group, tabc, num);
                        list = ResultList[j]->list;
                        flg = true;

                        while (list) {
                            // load  the symetric configuration
                            // on tabc and load a symetric marking.
                            POP_STATIC_CONF(Sym_StaticConf, &tabc);

                            string_to_marking(reached_marking->marking->marking_as_string,
                                              reached_marking->marking->d_ptr,
                                              reached_marking->marking->length);
                            reached_marking->marking->ordinary = TAB_SYM[i].ord;
                            mc_dsrg_insert_tree(list->Event, list->ord * list->rate,
                                                list->ord, gr_s_prop_StaticConf,
                                                ResultList[j]->PART_MAT,
                                                ResultList[j]->NbElPM, &flg);
                            list = list->next;
                        }
                        Free_ResList(j);
                        FREE_ALL_LMS_ELEMS();
                        FreeMerg(merg_group);
                    }

                    Free_DSC_SSC();
                    free(ResultList);
                    ResultList = NULL;
                    NbResList = 0;
                }
        }
        Free_TAB_SYM(TAB_SYM, NbTAB_SYM);
        free(tab_corr);
        nb_tab_corr = 0;
        dispache_succs(top);
        top = NULL;
        d_srg_pop(&d_srg_top);
        hash_table_garbage();
    }
    while (d_srg_top);

    time(&new_time);
    if (output_flag)
        fprintf(OUTPUT_FILE, "}\n");
    print_final_info();
    return ;
}

void reach_dsrg_graph(int comp) {

    int pr, conj, i, j, flg, k = 0;
    PART_MAT_P s_prop_Part = NULL;
    PART_MAT_P Disj_Part = NULL, PM;
    PART_MAT_P gard_part = NULL;
    TO_MERGEP *merg_group = NULL;
    MarkSym   *TAB_SYM = NULL;
    int        NbTAB_SYM = 0;

    STORE_STATICS_ARRAY s_prop_StaticConf = NULL,
                        gr_s_prop_StaticConf = NULL;
    MarkAsEventp list;
    int cmp = 20;


    (void)initial_state(&d_srg_top);

    d_srg_enabling_and_guards(d_srg_top);

    if (!(d_srg_top->enabled_head))
    {top = NULL; top_ins = bottom_ins = d_srg_top; d_srg_top->type = TANGIBLE;}
    else
    {top = NULL; top_sat = bottom_sat = d_srg_top; d_srg_top->type = TANGIBLE;};

    time(&old_time);
    if (output_flag)
        fprintf(OUTPUT_FILE, "digraph dtmc{\n");
    do {

        TAB_SYM = NULL;
        NbTAB_SYM = 0;
        int trans;

        if (d_srg_top->enabled_head) {
            d_srg_sym_succ_arc(d_srg_top , &TAB_SYM, &NbTAB_SYM);
            s_prop_StaticConf = NewTabc(d_srg_top->PM, d_srg_top->NbElPM, MTCL);
            trans = 0;
        }
        else {
            Disj_Part = d_srg_asym_succ_arc(d_srg_top , &TAB_SYM, &NbTAB_SYM, comp) ;
            s_prop_StaticConf = NewTabc(Disj_Part->PART_MAT, Disj_Part->NbElPM, MTCL);
            trans = 1;
        }

        if (d_srg_top->dead && !d_srg_top->nbtr) {
            DEAD_MARKINGS++;
            if (output_flag)
                print_dsrg(OUTPUT_FILE , d_srg_top, NULL, 0, 1);
        }

        for (i = 0; i < NbTAB_SYM; i++) {
            int nb_groups = 0, nbgr;

            if (TAB_SYM[i].Eventp) {
                free_temp_marking(TAB_SYM[i].Eventp);
                /***** find valid symbolic marking's group *******************/
                GROUP_MARKINGS_MC(TAB_SYM[i].Eventp, s_prop_StaticConf,
                                  Sym_StaticConf, MTCL, 0);
                /**************************************************************/

                for (j = 0; j < NbResList; j++) {
                    gr_s_prop_StaticConf = NewTabc(ResultList[j]->PART_MAT,
                                                   ResultList[j]->NbElPM, MTCL);
                    POP_STATIC_CONF(gr_s_prop_StaticConf, &tabc);
                    merg_group = TO_MERGE(ncl, tabc);
                    GROUPING_ALL_STATICS(merg_group, tabc, num);
                    list = ResultList[j]->list;
                    flg = true;

                    while (list) {
                        // load  the symetric configuration
                        // on tabc and load a symetric marking
                        POP_STATIC_CONF(Sym_StaticConf, &tabc);
                        set_state_cache(TAB_SYM[i].marking_as_string,
                                        TAB_SYM[i].d_ptr, TAB_SYM[i].length);
                        insert_tree(&root, &h, f_mark, length, d_ptr);
                        if (tro == NEW_MARKING) {
                            esm_count++;
                        }
                        string_to_marking(reached_marking->marking->marking_as_string,
                                          reached_marking->marking->d_ptr,
                                          reached_marking->marking->length);
                        dsrg_insert_tree(list->Event, list->rate, list->ord,
                                         gr_s_prop_StaticConf,
                                         ResultList[j]->PART_MAT,
                                         ResultList[j]->NbElPM, &flg);

                        list = list->next;
                    }

                    Free_ResList(j);
                    FREE_ALL_LMS_ELEMS();
                    FreeMerg(merg_group);
                }

                Free_DSC_SSC();
                free(ResultList);
                ResultList = NULL;
                NbResList = 0;
            }

        }

        Free_TAB_SYM(TAB_SYM, NbTAB_SYM);
        dispache_succs(top);
        top = NULL;
        d_srg_pop(&d_srg_top);
        hash_table_garbage();
    }
    while (d_srg_top);

    time(&new_time);
    if (output_flag)
        fprintf(OUTPUT_FILE, "}\n");
    print_final_info();

    return ;
}



#endif















