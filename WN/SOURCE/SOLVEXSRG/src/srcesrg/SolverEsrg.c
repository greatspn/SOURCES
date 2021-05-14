#include <stdio.h>
#include <stdlib.h>
#include "const.h"
#include "SCONSSPOT.h"
#include "struct.h"
#include "SSTRUCTSPOT.h"
#include "var_ext.h"
#include "SVAR_EXTSPOT.h"
#include "macros.h"
#include "structSolv.h"
#include "service.h"

#undef  NORMAL

#define true 1
#define false 0
#define MAXSTR 256

#include "../../../../INCLUDE/struct.h"
#include "../../../../INCLUDE/var_ext.h"
#include "../../../../INCLUDE/const.h"
//#include "../../../../INCLUDE/decl.h"
//#include "../../../../INCLUDE/macros.h"
//#include "../../../../INCLUDE/fun_ext.h"
#include "../../../../INCLUDE/ealloc.h"

extern char *emalloc(size_t sz);
extern FILE *efopen(const char *file, const char *mode);
extern void *ecalloc(size_t nitm, size_t sz);
extern Event_p get_new_event(int);
extern TO_MERGEP *TO_MERGE(int, struct COLORS *);
extern int  initialize(int  argc,  char **);
extern void INIT_GLOBAL_VARS();
extern void STORE_STATIC_CONF(STORE_STATICS_ARRAY *, struct COLORS *);
extern void GROUPING_ALL_STATICS(TO_MERGEP *, struct COLORS *, int **);
extern void NEW_SIM_STATIC_CONF(STORE_STATICS_ARRAY *);
extern void MY_INIT_ALLOC();
extern void create_canonical_data_structure();
extern void CHAR_LOAD_COMPACT(unsigned long *);
extern void POP_STATIC_CONF(STORE_STATICS_ARRAY, struct COLORS **);
extern void string_to_marking(unsigned long,  unsigned long,  unsigned long);
extern void GET_EVENTUALITIE_FROM_FILE(int, int);
extern void COPY_CACHE(int);
extern void STRING_TO_EVENTUALITIE();
extern int  X_L_MOINS_1(int, int, int, int);
extern int  C_L_PLUS_1(int, int, int);
extern int  X_2(int, int, int, int);
extern int  X_I_MOINS_1(int, int, int, int);
extern void NEW_ASYM_MARKING(struct COLORS **, struct NET_MARKING **, int ** **, int ** *, int **,
                             DECOMP_TO *, STORE_STATICS_ARRAY, int ** **);
extern void my_en_list(int);
extern void copy_event(Event_p,  Event_p);
extern void fire_trans(Event_p);
extern void SPECIAL_CANISATION();
extern void STORE_CARD(int ** *, int ** **);
extern void GET_SYMETRIC_MARKING(struct COLORS **, struct NET_MARKING **, int ** **, int ** *, int **,
                                 STORE_STATICS_ARRAY, TO_MERGEP *);
extern void ALL_MY_GROUP(int ** *);
extern void get_canonical_marking();
extern void AFTER_CANONISATION(int **, int ** *);
extern void GET_EVENT_FROM_MARKING(struct COLORS *tabc, int *, int **, int ** *, int **,
                                   STORE_STATICS_ARRAY, int ** *, TO_MERGEP *);
extern void INIT_ARRAYS(TO_MERGEP *, int **, int ** *, STORE_STATICS_ARRAY);
extern void dispose_old_event(Event_p);
extern void EVENTUALITIE_TO_STRING();
extern void ADD_EVENTUALITIE_TO_FILE(int);
extern void MY_CHAR_STORE(unsigned long);
extern void INIT_4D_MATRIX(MATRIX **);
extern void INIT_ARRAYS(TO_MERGEP *, int **, int ** *, STORE_STATICS_ARRAY);

extern void write_on_srg(FILE *, int);
extern void write_ctrs(FILE *);
extern void store_compact(int, FILE *);
extern void store_double(double *, FILE *);
extern void code_marking(void);
extern int my_strcmp(unsigned long, unsigned long, unsigned long, unsigned long);
extern unsigned long marking_to_string();
extern void WRITE_ON_ESRG(FILE *);

extern void compose_name(char  *name,  const char  *path,  const char  *postfix);
extern int perf_index_parser(FILE* f);
extern void push_result(Result_p  result_ptr);
struct Marking;
struct MarkingStruct;
extern struct Marking *_seach_event_marking(struct MarkingStruct *tab, int tab_size, char *s_string, int s_size);
extern int _copy_cache_string(char **new_string);
extern void _free_event_marking(marking_struct *tab, int tab_size);
extern void free_mtrx(mtrx *U, unsigned long nb);


typedef struct Event {
    unsigned long  len;
    unsigned long  ld;
} events;

typedef struct Block {
    unsigned long  mark_pos;
    unsigned long  length;
    unsigned long  d_ptr;

    events *evt;
    int nbev;
} Block;

Block *blk     = NULL;
int *vanishing = NULL;
int nb_sr      = 0;
int nb_sm      = 0;
int nb_va      = 0;
int nb_dead    = 0;
int nb_v       = 0;
double normal  = 0.0;
double rate    = 0.0;


int indices    = FALSE;
FILE   *resrg   = NULL ;
FILE   *resultat = NULL;
int     midx    = 0;
int     nbordm  = 0;
int     glb_mark = 0;
double *prob    = NULL;

PerfIndex_P *PERFINDICES = NULL;
int NbPind = 0;

static marking_struct *tabtmp = NULL;
static int sizetmp = 0;
/********************** extern function ************************/
extern char    *strtok();
extern Result_p trait_gurded_transitions(Result_p enabled_head,
        PART_MAT_P s_prop_Part);
extern int      adjust_priority(int cur_priority,
                                Result_p enabled_head);
extern Event_p  get_new_event(int tr);
extern double  *gauss_seidel(double *mprob,
                             mtrx *column,
                             unsigned long top_tan);

/***************************************************************/

/************** exetrn variables ***********/
extern char                cache_string [];
extern TO_MERGEP          *MERG_ALL;
extern STORE_STATICS_ARRAY Sym_StaticConf;
extern Result_p            enabled_head;
extern int                 cur_priority;
/******************************************/

char         net[MAXSTRING];
Tree_Esrg_p  sr_ordinary = NULL;
int          sr_size_ordinary = 0;
int          counter = 0;



// closes all files at the end
// of a session.
void
finalize_session(FILE *fp) {
    fclose(fp);
    fclose(resultat);
}

// Store of symbolic states
// in the "blk" structure
void
blocks_add(int id,
           unsigned long  mark_pos,
           unsigned long  length,
           unsigned long  d_ptr,
           int ev,
           unsigned long ld_ptr,
           unsigned long llength,
           int flg
          ) {

    blk[id].mark_pos = mark_pos;
    blk[id].length = length;
    blk[id].d_ptr = d_ptr;

    if (flg) {
        blk[id].evt[ev].len = llength;
        blk[id].evt[ev].ld = ld_ptr;
    }
}

void solver_usage(void) {

    fprintf(stdout, "\n\n------------------------------------------------------\n");
    fprintf(stdout, "USE : solverEsrg netname [options]                        \n");
    fprintf(stdout, "      options :                                           \n");
    fprintf(stdout, "        -s : to solve the symbolic DTMC                   \n");
    fprintf(stdout, "        -o : to solve the ordinary CTMC                   \n");
    fprintf(stdout, "        -p : to solve the ordinary CTMC with                ");
    fprintf(stdout, "             perf. indices computation.                   \n");
    fprintf(stdout, "----------------------------------------------------------\n");

}

void
output_dtmc_solution(double *pi) {

    unsigned long i, j;

    fprintf(resultat, "\n /*********** Symbolic markings steady probabilities");
    fprintf(resultat, "(Symbolic DTMC) ***********/ \n");

    for (i = 0 ; i < nb_sm ; i++)
        fprintf(resultat, "probability block[%d]=%lf \n", i + 1, pi[i]);

    fprintf(resultat, "\n /*********** End symbolic markings steady probabilities");
    fprintf(resultat, " (Symbolic DTMC) ***********/ \n");
}


// Parse of  params. (in order) :
// sym. rep. position of source
// source
// nb ordinary of the source
// type of source (tangible or vanishing)
// out put prob.
// sym. rep. position of destination
// destination
// nb ordinary of the destination
int
parse_binding(char *string,
              int *source ,
              int *dest ,
              double *prob) {

    char *tok;
    char *delim = " ";
    tok = strtok(string , delim);

    sscanf(tok, "%d", source);
    tok = strtok(NULL, delim);

    sscanf(tok, "%lf", prob);
    tok = strtok(NULL, delim);

    sscanf(tok, "%d", dest);
    return true;
}

void
parse_blocks(FILE *Blocks) {
    int id, nbev, sr, possr,
        lensr, dptr, posev, lenev;

    char tmp[MAXSTR];
    mtrx *U = NULL;
    unsigned long ofsr, ofst, ofsrd, ofstd;

    fgets(tmp, MAXSTR - 1,  Blocks);
    sscanf(tmp, "%d", &nb_sm);

    blk = (Block *)calloc(nb_sm, sizeof(Block));

    while (fgets(tmp, MAXSTR - 1,  Blocks)) {
        sscanf(tmp, "%d %d", &id, &nbev);

        id--;

        if (nbev != -1) {
            int i;

            blk[id].evt = (events *)calloc(nbev, sizeof(events));
            blk[id].nbev = nbev;

            for (i = 0; i < nbev; i++) {
                fgets(tmp, MAXSTR - 1,  Blocks);
                sscanf(tmp, "%d %d %d %d %d %d",
                       &sr, &possr, &lensr, &dptr, &posev, &lenev);
                blocks_add(id, possr, lensr, dptr, i, posev, lenev, 1);
            }
        }
        else {
            fgets(tmp, MAXSTR - 1,  Blocks);
            sscanf(tmp, "%d %d %d %d", &sr, &possr, &lensr, &dptr);
            blocks_add(id, possr, lensr, dptr, 0, 0, 0, 0);
        }
    }
}

int print_state(char **st) {
    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");
    write_on_srg(fd, 1);
    pos = ftell(fd);
    *st = (char *)malloc((pos + 1) * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);
    return 0;
}


void
init_global_variables(char *net_name) {

    INIT_GLOBAL_VARS();
    STORE_STATIC_CONF(&ASYM_STATIC_STORE, tabc);
    MERG = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG, tabc, num);
    NEW_SIM_STATIC_CONF(& SYM_STATIC_STORE);
    MY_INIT_ALLOC();
    create_canonical_data_structure();

    // output params. of the
    // dsrg construction (with options: -o, -m)
    compose_name(net, net_name, "bk");
    FILE *stat = fopen(net, "r");
    parse_blocks(stat);
    fclose(stat);
    compose_name(net, net_name, "resrg");
    resrg = fopen(net, "r");

    compose_name(net, net_name, "prob");
    resultat = fopen(net, "w+");

    if (indices) {
        compose_name(net, net_name, "ind");
        FILE *INDICES = fopen(net, "r");
        perf_index_parser(INDICES);
    }

}

// load the symmetrical representation
// from the ".mark" file using the traditional
// params. stored  in the ".off_sr" file.
void load_sym_rep(unsigned long  mark_pos,
                  unsigned long  length,
                  unsigned long  d_ptr) {

    // load the correct symbolic marking from
    // the ".mark" file.
    POP_STATIC_CONF(SYM_STATIC_STORE, & tabc);
    string_to_marking(mark_pos, d_ptr, length);

}

// load an eventuality from the .event
// file, using the "pos" position in the "off_ev"
// file to get the position "d_ptr" and the length
// "length" of the eventuality.
int load_event(unsigned long  llength,
               unsigned long  ld_ptr,
               int flag) {
    // load the string form of the eventuality
    // from the ".event" file in "compare" cache string.
    GET_EVENTUALITIE_FROM_FILE(ld_ptr, llength);
    if (flag) {
        COPY_CACHE(llength);
        STRING_TO_EVENTUALITIE();
    }
    return 0;
}

// Load a symbolic state in the
// internal structures  of
// GreatSPN and print it
void
load_symbolic_marking(int sour,
                      int ev,
                      int flg) {

    load_sym_rep(blk[sour].mark_pos,
                 blk[sour].length,
                 blk[sour].d_ptr);
    if (flg) {
        load_event(blk[sour].evt[ev].len,
                   blk[sour].evt[ev].ld, 1);
        NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                         MERG, ASYM_STATIC_STORE, RESULT);
    }
}

// Get the number of ordinary tokens
// represented by the symbolic token
// tk_p
int
get_ordinary_tokens_perfs(Token_p tk_p, int pl) {
    int ret_value = 1;
    int ii = GET_PLACE_COMPONENTS(pl);
    int cl, ss, ds;
    for (; ii ; ii--) {
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
        if (IS_UNORDERED(cl) ||
                (IS_ORDERED(cl) &&
                 GET_STATIC_SUBCLASS(cl) == 1)) {
            ds = tk_p->id[ii - 1];
            ss = get_static_subclass(cl, ds);
            ret_value *= GET_CARD(cl, ss, ds);
        }
    }
    return (ret_value);
}

// Get the number of ordinary tokens
// represented by the symbolic token
// tk_p, w.r.t. some projection pr.
int
prog_get_ordinary_tokens(Token_p tk_p, int *pr, int p) {

    int ret_value = 1;
    int ii = GET_PLACE_COMPONENTS(p);
    int cl, ss, ds;

    for (; ii ; ii--)
        if (pr[ii - 1] == PROJECT) {
            cl = GET_COLOR_COMPONENT(ii - 1, p);
            if (IS_UNORDERED(cl) ||
                    (IS_ORDERED(cl) &&
                     GET_STATIC_SUBCLASS(cl) == 1)) {
                ds = tk_p->id[ii - 1];
                ss = get_static_subclass(cl, ds);
                ret_value *= GET_CARD(cl, ss, ds);
            }
        }
    return (ret_value);
}


// compare tok1 to tok2 w.r.t.
// the projection pr.
int get_proj(int p,
             int *tok1,
             int *tok2,
             int *pr) {

    int size = GET_PLACE_COMPONENTS(p);
    int i, j;

    for (i = 0, j = 0; i < size; i++) {
        int cl = GET_COLOR_COMPONENT(i, p);
        if (pr[i] == PROJECT) {
            if (tok2[j] != get_static_subclass(cl, tok1[i]))
                return false;
            else j++;
        }
    }

    return true;
}


// Get the total number of
// ordinay tokens for the perf index p
int
place_tokens_counter(PerfIndex_P p) {

    int i = 0;
    CToken_p ptr, ptr1, ptr2;
    Token_p marking = net_mark[p->pt_index].marking;

    if (p->cond)
        while (marking) {
            if (get_proj(p->pt_index, marking->id,
                         ((p->cond)->filsd->FONCT->tok_dom).Token,
                         ((p->cond)->filsg->FONCT->tok_dom).Token))

                i += (marking->molt) *
                     prog_get_ordinary_tokens(marking,
                                              ((p->cond)->filsg->FONCT->tok_dom).Token,
                                              p->pt_index);

            marking = marking->next;

        }
    else {

        int place = p->pt_index ;
        Token_p marking = net_mark[place].marking;
        while (marking) {
            i += ((marking->molt) *
                  get_ordinary_tokens_perfs(marking, place));
            marking = marking->next;
        }
    }
    return i;
}

// Get the total number of ordinay
// instances of a symbolic instance
int
transition_tokens_counter(PerfIndex_P p,
                          Event_p list) {
    Event_p ptr = list;
    int i, sbc = UNKNOWN;

    if (p->cond) {
        CToken_p Token = p->cond->FONCT;
        int cl = tabt[list->trans].dominio[(Token->mult) - 1];
        int dy = list->npla[(Token->mult) - 1];

        if (get_static_subclass(cl, dy) == (Token->tok_dom).Token[0]) {
            return (list->ordinary_instances * list->enabling_degree);
        }
        return 0;
    }
    return (list->ordinary_instances * list->enabling_degree);
}


// Compute all infos. for all transition based
// perf. indices.
void ParcourtStructuretrans(Event_p ev) {
    int i;

    for (i = 0 ; i < NbPind ; i++)
        if (PERFINDICES[i]->type == TR &&
                ev->trans == PERFINDICES[i]->pt_index)
            PERFINDICES[i]->sum +=
                transition_tokens_counter(PERFINDICES[i], ev);
}

// Compute all infos. for all places based
// perf. indices.
void ParcourtStructureplaces() {
    int i;

    for (i = 0 ; i < NbPind ; i++)
        if (PERFINDICES[i]->type == PL) {
            PERFINDICES[i]->sum =
                place_tokens_counter(PERFINDICES[i]);
        }
}

// Compute the perf indices w.r.t.
// the current symbolic marking.
void CollectInfo(double prob, int nbord) {

    int i;
    for (i = 0 ; i < NbPind ; i++)
        if (PERFINDICES[i]->sum) {

            if (PERFINDICES[i]->type == TR)
                PERFINDICES[i]->mean += nbord * PERFINDICES[i]->sum *
                                        tabt[PERFINDICES[i]->pt_index].mean_t *
                                        prob;
            else
                PERFINDICES[i]->mean += nbord * PERFINDICES[i]->sum * prob;

            PERFINDICES[i]->sum = 0;
        }
}

char *GET_FUNC(int t, int ind) {
    return tabt[t].names[ind];
}

// Print conditions of perf. indices.
void PrintCondition(int i) {
    int k;
    if (PERFINDICES[i]->type == TR) {
        printf("%s in ", GET_FUNC(PERFINDICES[i]->pt_index,
                                  (PERFINDICES[i]->cond->FONCT->mult) - 1));
        for (k = 0; k < (PERFINDICES[i]->cond->FONCT->tok_dom).NbCl; k++)
            printf("%s", ASYM_STATIC_STORE[(PERFINDICES[i]->cond->FONCT->tok_dom).Domain[k]].ptr
                   [(PERFINDICES[i]->cond->FONCT->tok_dom).Token[k]].name);
    }
    else {
        printf("[");
        for (k = 0; k < (PERFINDICES[i]->cond->filsg->FONCT->tok_dom).NbCl; k++)
            if ((PERFINDICES[i]->cond->filsg->FONCT->tok_dom).Token[k] == ELIMINATE)
                printf("-");
            else
                printf("*");

        printf("] in <");

        for (k = 0; k < (PERFINDICES[i]->cond->filsd->FONCT->tok_dom).NbCl; k++)
            printf("%s", ASYM_STATIC_STORE[(PERFINDICES[i]->cond->filsd->FONCT->tok_dom).Domain[k]].ptr
                   [(PERFINDICES[i]->cond->filsd->FONCT->tok_dom).Token[k]].name);

        printf(">");
    }
}


// Print the final resulting perf indices.
void PrintIndicies(double normal) {
    int i;
    for (i = 0 ; i < NbPind ; i++)
        if (PERFINDICES[i]->type == TR) {
            if (!PERFINDICES[i]->cond)
                printf(" The throughput of the transtion \"%s\" = %g\n",
                       tabt[PERFINDICES[i]->pt_index].trans_name,
                       PERFINDICES[i]->mean / normal);
            else {
                printf(" The throughput of the transtion \"%s\"",
                       tabt[PERFINDICES[i]->pt_index].trans_name);
                printf(" w.r.t. conditon \"");
                PrintCondition(i);
                printf("\" = %g\n", PERFINDICES[i]->mean / normal);
            }
        }
        else {
            if (!PERFINDICES[i]->cond)
                printf(" The mean number of tokens in the place \"%s\" = %g\n",
                       tabp[PERFINDICES[i]->pt_index].place_name,
                       PERFINDICES[i]->mean / normal);
            else {
                printf(" The mean number of tokens in the place \"%s\" ",
                       tabp[PERFINDICES[i]->pt_index].place_name);
                printf(" w.r.t. conditon \"");
                PrintCondition(i);
                printf("\"  = %g\n", PERFINDICES[i]->mean / normal);

            }
        }
}

//Free symbolic instances
void
dispose_instances(Result_p res_ptr) {

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

void
__collect() {
    Result_p current_transition, next_transition;
    current_transition = enabled_head;
    while (current_transition != NULL) {
        next_transition = current_transition->next;
        dispose_instances(current_transition);
        current_transition->list = NULL;
        push_result(current_transition);
        current_transition = next_transition;
    }
    enabled_head = NULL;
}


// Compute the global output rate
// of an ordinay marking.
double
get_rate(int nbord) {

    Event_p  nev_p , ev_p, copy_of_ev_p;
    Result_p current_transition,
             next_transition;
    double   rate = 0.0;
    int      tr;

    my_initialize_en_list();

    if (indices) ParcourtStructureplaces();

    if (enabled_head) {
        current_transition = enabled_head;
        while (current_transition != NULL) {
            next_transition = current_transition->next;
            tr = GET_TRANSITION_INDEX(current_transition->list);

            if (tabt[tr].pri == cur_priority) {
                copy_of_ev_p = get_new_event(tr);
                ev_p = current_transition->list;

                while (ev_p != NULL) {
                    nev_p = ev_p->next;
                    copy_event(copy_of_ev_p, ev_p);

                    if (tabt[tr].timing != TIMING_IMMEDIATE)
                        rate += ev_p->ordinary_instances *
                                (ev_p->enabling_degree * tabt[tr].mean_t);
                    else
                        rate = 0;

                    if (indices) ParcourtStructuretrans(ev_p);

                    ev_p = nev_p;
                }
                dispose_old_event(copy_of_ev_p);
            }
            current_transition = next_transition;
        }
    }
    __collect();

    char *st = NULL;
    print_state(&st);

    if (rate) {

        int nb = 0;

        if (blk[midx].nbev) {
            nb = blk[midx].nbev * nbord;
            normal += nbord * prob[midx] / (nb * rate);

        }
        else {
            nb = glb_mark;
            normal += nbord * prob[midx] / (nb * rate);
        }

        fprintf(resultat,
                "\n Prob of each state in SM (card=%d): %s is [%lf]\n",
                nbord, st, prob[midx] / (nb * rate));

        if (indices) CollectInfo(prob[midx] / (nb * rate), nbord);
    }
    else {
        fprintf(resultat,
                "\n Prob of each state in SM (card=%d): %s is [0.0]\n",
                nbord, st);
    }
    free(st);

    return rate;
}

void
compute_global_rate() {

    enabled_head = NULL;
    rate = 0;

    NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                     MERG, ASYM_STATIC_STORE, RESULT);

    if (_seach_event_marking(tabtmp, sizetmp,
                             cache_string,
                             length) != EQUAL) {

        tabtmp = realloc(tabtmp,
                         (sizetmp + 1) * sizeof(marking_struct));
        tabtmp[sizetmp].length =
            _copy_cache_string(&tabtmp[sizetmp].marking);

        sizetmp++;

        (void)get_rate((int)mark_ordinarie);
    }

    load_symbolic_marking(midx, 0, 0);
}


// Load of the transition matrix U
// from the dsrg ASCII file (netname.resrg).
mtrx *
load_mat(unsigned long nb_sm, FILE  *resrg) {
    int sr_source, source, sord,
        sr_dest,   dest,   dord, type;
    double rate;
    char tmp[MAXSTR];
    mtrx *U = NULL;
    unsigned long ofsr, ofst, ofsrd, ofstd;

    /*************** Allocation of the matrix of transition U *****/
    U   = (mtrx *)calloc(nb_sm + 1, sizeof(mtrx));
    /************** End allocation ********************************/

    /*************** upload of the U matrix **********************/
    while (fgets(tmp, MAXSTR - 1, resrg)) {

        (void) parse_binding(tmp, &source,
                             &dest, &rate);
        dest;
        source;
        U[dest].ninn++;
        U[dest].first = (itm *)realloc(U[dest].first,
                                       U[dest].ninn * sizeof(itm));
        U[dest].first[(U[dest].ninn) - 1].fm = source;
        U[dest].first[(U[dest].ninn) - 1].rt = rate;
    }
    /*************************************************************/
    return U;
}


// Compute the CTMC probabilities
void
compute_oridinary_probs() {
    int i, j;
    char *st = NULL;

    MARKING_TYPE = SATURED_INS;

    for (i = 0; i < nb_sm; i++) {

        midx = i;

        if (blk[i].nbev) {

            fprintf(resultat, "\n********* Block[%d]**********\n", i);

            for (j = 0; j < blk[i].nbev; j++) {
                load_symbolic_marking(i, j, 1);
                get_rate((int)mark_ordinarie);
            }

            fprintf(resultat, "\n******************************\n");

        }
        else {

            load_symbolic_marking(i, 0, 0);
            SPECIAL_CANISATION();
            glb_mark = (int)mark_ordinarie;

            fprintf(resultat, "\n*********Block[%d]************\n", i);

            INIT_4D_MATRIX(RESULT);
            INIT_ARRAYS(MERG , num, card, ASYM_STATIC_STORE);

            tabtmp = NULL; sizetmp = 0;
            EVENTUALITIES(0, 0, 1, 1, compute_global_rate);
            _free_event_marking(tabtmp, sizetmp);

            fprintf(resultat, "\n******************************\n");
        }
    }

}

// The pricipal function of the solver.
// To use the solver, use this function
// with the number of dead states (nb_dead),
// the number of states (nb_sm),
// the esrg file (netname.bk)
// It computes the Lumped DTMC probabilities.
double *
symbolic_solv(unsigned long nb_dead,
              unsigned long nb_sm,
              FILE *resrg) {

    mtrx *U = NULL;
    unsigned long i;
    double *prob = NULL;

    if (nb_dead) {
        printf("THE MARCOV CHAIN IS NOT ERGODIC");
        return NULL;
    }

    U = load_mat(nb_sm, resrg);
    prob = (double *)calloc(nb_sm + 1, sizeof(double));
    for (i = nb_sm  ; i-- ; prob[i] = 1.0 / (double)nb_sm);

    prob = gauss_seidel(prob, U , nb_sm);

    free_mtrx(U, nb_sm + 1);

    return prob;

}

int
main(int argc, char *argv[]) {
    int i, ordi = FALSE;
    double *pi1 ;
    char **arg = (char **)calloc(3, sizeof(char *));


    arg[2] = (char *)calloc(2, sizeof(char));
    arg[0] = argv[0];
    arg[1] = argv[1];
    arg[2][0] = '-';
    arg[2][1] = 'r';

    if (argc > 2)
        switch (argv[2][1]) {
        case 's' : break;
        case 'o' : ordi = TRUE; break;
        case 'p' : ordi = TRUE; indices = TRUE; break;
        default  : solver_usage(); exit(1);
        }

    initialize(3, arg);
    init_global_variables(argv[1]);

    pi1 = symbolic_solv(0, nb_sm, resrg);
    prob = pi1 + 1;

    output_dtmc_solution(prob);

    if (ordi) {

        fprintf(resultat, "\n /*********** Ordinary markings steady probabilities");
        fprintf(resultat, "(Ordinary CTMC)***********/ \n");

        compute_oridinary_probs();

        fprintf(resultat, "\n The final CTMC probabilities are obtained by");
        fprintf(resultat, " normalisation: dividing by  %f ) \n", normal);
        fprintf(resultat, "\n /*********** End ordinary markings steady probabilities");
        fprintf(resultat, "(Ordinary CTMC)***********/ \n");
        if (indices) PrintIndicies(normal);
    }



    free(arg[2]);
    free(arg);
    free(pi1);
    finalize_session(resrg);
}

