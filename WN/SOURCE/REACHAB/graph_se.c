# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/macros.h"

#ifdef REACHABILITY
extern char *emalloc();

#ifdef DEBUG_malloc
static int result_mall = 0;
static int result_pop = 0;
static int result_push = 0;

static int markinfo_mall_tot = 0;
static int markinfo_pop_tot = 0;

static int throu_mall_tot = 0;
static int throu_pop_tot = 0;
static int throu_push_tot = 0;

static int rate_mall_tot = 0;
static int rate_pop_tot = 0;
static int rate_push_tot = 0;

static int path_mall_tot = 0;
static int path_pop_tot = 0;
static int path_push_tot = 0;

static int treenode_mall_tot = 0;
static int treenode_pop_tot = 0;

#endif
// extern char cache_string[MAX_CACHE];
char *get_cache_string();
unsigned long d_ptr;
unsigned long length;


static Result_p result_free = NULL;   /* Free list per le result */
static MRate_p rate_free = NULL;   /* Free list per le result */
static PComp_p path_free = NULL;   /* Free list per le result */
static Throughput_p throu_free = NULL;   /* Free list per le result */


Throughput_p throu_pop(void);
void push_path_list(PComp_p path_head);
MRate_p rate_pop(void);
PComp_p path_pop(void);


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
PComp_p fill_factor(int  tr,  unsigned long  ordinary_m,  unsigned long  en_degree,  unsigned long  denom_p) {
    /* Init fill_factor */
    PComp_p path = NULL;

    path = path_pop();
    path->fired_transition = tr ;
#ifdef SWN
#ifdef SYMBOLIC
    path->ordinary_m = ordinary_m ;
#endif
#endif
    path->enabling_degree = en_degree ;
    path->denominator = denom_p ;
    return (path);

}/* End fill_factor */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
MRate_p fill_mark_node(int  file_p,  unsigned long  path_p,  double  mean_t) {
    /* Init fill_mark_node */
    MRate_p marc_ragg = NULL;

    marc_ragg = rate_pop();
    marc_ragg->flag = file_p;
    marc_ragg->cont_tang = path_p;
    marc_ragg->mean_t =  mean_t;
    return (marc_ragg);

}/* End fill_mark_node */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Throughput_p fill_throughput_node(int  tr,  double  mean_t) {
    /* Init fill_throughput_node */
    Throughput_p throu_ragg = NULL;

    throu_ragg = throu_pop();
    throu_ragg->tr = tr;
    throu_ragg->weight =  mean_t;
    return (throu_ragg);

}/* End fill_throughput_node */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Result_p pop_result() {
    /* Init pop_result */
    Result_p ret_ptr = NULL;

#ifdef DEBUG_malloc
    result_pop++;
#endif
    if (result_free == NULL) {
        /* Lista libera vuota */
        ret_ptr = (Result_p)emalloc(sizeof(struct RESULT));
#ifdef DEBUG_malloc
        result_mall++;
#endif
    }/* Lista libera vuota */
    else {
        ret_ptr = result_free;
        result_free = result_free->next;
    }
    ret_ptr->list = NULL;
    ret_ptr->next = NULL;
    ret_ptr->hold = FALSE;
    return (ret_ptr);
}/* End pop_result */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_result(Result_p  result_ptr) {
    /* Init push_result */

#ifdef DEBUG_malloc
    result_push++;
#endif
    result_ptr->next = result_free;
    result_free = result_ptr;
}/* End push_result */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
MInfo_p markinfo_pop(unsigned long  mark_ptr,  unsigned long  ilength,  unsigned long  id_ptr) {
    /* Init markinfo_pop */
    MInfo_p ptr;

    int stat;
#ifdef DEBUG_malloc
    markinfo_pop_tot++;
    markinfo_mall_tot++;
#endif
    ptr = (MInfo_p)emalloc(sizeof(struct MARKING_INFO));
    ptr->marking_as_string = mark_ptr;


    fseek(mark, mark_ptr, 0);
    fwrite(get_cache_string(), 1, ilength, mark);
    ptr->length = ilength;


    ptr->path = ptr->throu = ptr->cont_tang = ptr->pri = UNKNOWN;
#ifdef SWN
#ifdef SYMBOLIC
    ptr->d_ptr = mark_ptr + id_ptr;
    ptr->ordinary = UNKNOWN;
#endif
#endif
    return (ptr);
}/* End markinfo_pop */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_throu_list(Throughput_p  throu_head,  Throughput_p  throu_tail) {
    /* Init push_throu_list */
    Throughput_p scan;

    if (throu_head != NULL) {
        for (scan = throu_head ; scan != NULL; scan = scan->next)
            push_path_list(scan->path);
        throu_tail->next = throu_free;
        throu_free = throu_head;
    }
}/* End push_throu_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_throu_element(Throughput_p  throu_ptr) {
    /* Init push_throu_element */

#ifdef DEBUG_malloc
    throu_push_tot++;
#endif
    throu_ptr->next = throu_free;
    throu_free = throu_ptr;
}/* End push_throu_element */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Throughput_p throu_pop(void) {
    /* Init throu_pop */
    Throughput_p ptr;

#ifdef DEBUG_malloc
    throu_pop_tot++;
#endif
    if (throu_free == NULL) {
#ifdef DEBUG_malloc
        throu_mall_tot++;
#endif
        ptr = (Throughput_p)emalloc(sizeof(struct TRANSITION_WEIGHT));
    }
    else {
        ptr = throu_free;
        throu_free = throu_free->next;
    }
    ptr->tr = UNKNOWN;
    ptr->weight = UNKNOWN;
    ptr->path = NULL;
    ptr->cnt = 0;
    ptr->next = NULL;
    return (ptr);
}/* End throu_pop */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_path_list(PComp_p path_head) {
    /* Init push_path_list */
    PComp_p ptr;

    if (path_head != NULL) {
        for (ptr = path_head; ptr->next != NULL ; ptr = ptr->next);
        ptr->next = path_free;
        path_free = path_head;
    }
}/* End push_path_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_path_element(PComp_p  path_ptr) {
    /* Init push_path_element */

#ifdef DEBUG_malloc
    path_push_tot++;
#endif
    path_ptr->next = path_free;
    path_free = path_ptr;
}/* End push_path_element */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
PComp_p path_pop(void) {
    /* Init path_pop */
    PComp_p ptr;

#ifdef DEBUG_malloc
    path_pop_tot++;
#endif
    if (path_free == NULL) {
#ifdef DEBUG_malloc
        path_mall_tot++;
#endif
        ptr = (PComp_p)emalloc(sizeof(struct PATH_COMP));
    }
    else {
        ptr = path_free;
        path_free = path_free->next;
    }
    ptr->denominator = 0;
    ptr->fired_transition = UNKNOWN;
    ptr->enabling_degree = 1;
#ifdef SWN
#ifdef SYMBOLIC
    ptr->ordinary_m = 1;
#endif
#endif
    ptr->next = NULL;
    return (ptr);
}/* End path_pop */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_rate_list(MRate_p  rate_head,  MRate_p  rate_tail) {
    /* Init push_rate_list */
    MRate_p scan;

    if (rate_head != NULL) {
        for (scan = rate_head ; scan != NULL; scan = scan->next)
            push_path_list(scan->path);
        rate_tail->next = rate_free;
        rate_free = rate_head;
    }
}/* End push_rate_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_rate_element(MRate_p  rate_ptr) {
    /* Init push_rate_element */

#ifdef DEBUG_malloc
    rate_push_tot++;
#endif
    rate_ptr->next = rate_free;
    rate_free = rate_ptr;
}/* End push_rate_element */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
MRate_p rate_pop(void) {
    /* Init rate_pop */
    MRate_p ptr;

#ifdef DEBUG_malloc
    rate_pop_tot++;
#endif
    if (rate_free == NULL) {
#ifdef DEBUG_malloc
        rate_mall_tot++;
#endif
        ptr = (MRate_p)emalloc(sizeof(struct MARC_RAGG));
    }
    else {
        ptr = rate_free;
        rate_free = rate_free->next;
    }
    ptr->flag = FALSE;
    ptr->cont_tang = UNKNOWN;
    ptr->path = NULL;
    ptr->cnt = 0;
    ptr->mean_t = UNKNOWN;
    ptr->next = NULL;
    return (ptr);
}/* End rate_pop */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Tree_p treenode_pop(unsigned long  mark_ptr,  unsigned long  ilength,  unsigned long  id_ptr) {
    /* Init treenode_pop */
    Tree_p ptr;

#ifdef DEBUG_malloc
    treenode_pop_tot++;
    treenode_mall_tot++;
#endif
    ptr = (Tree_p)emalloc(sizeof(struct TREE));
    ptr->marking = markinfo_pop(mark_ptr, ilength, id_ptr);
    ptr->enabled_head = NULL;
    ptr->last = ptr->left = ptr->right = NULL;
    ptr->balance = UNKNOWN;
    return (ptr);
}/* End treenode_pop */

#ifdef DEBUG_malloc
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_reach_info() {
    /* Init out_reach_info */
    fprintf(stdout, "--------------- RESULT STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , result_mall, result_push, result_pop);
    fprintf(stdout, "--------------- MARKING INFO STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPOP %d\n"
            , markinfo_mall_tot, markinfo_pop_tot);
    fprintf(stdout, "--------------- RATE PATH STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , rate_mall_tot, rate_push_tot, rate_pop_tot);
    fprintf(stdout, "--------------- PATH COMPUTATION STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , path_mall_tot, path_push_tot, path_pop_tot);
    fprintf(stdout, "--------------- THROUGHPUT PATH STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , throu_mall_tot, throu_push_tot, throu_pop_tot);
    fprintf(stdout, "--------------- TREE NODE STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPOP %d\n"
            , treenode_mall_tot, treenode_pop_tot);
}/* End out_reach_info */
#endif
#endif
