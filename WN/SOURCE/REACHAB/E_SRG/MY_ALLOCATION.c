
/************************************************************/
/*    All called functions for the memory allocation are    */
/*              représented in this file                    */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
// #include <malloc.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/ealloc.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"


extern Tree_p treenode_pop();

LIST_MY_STATICS FREE_STATICS_LIST = NULL;
INT_LISTP       FREE_DYN_LIST = NULL;
Tree_Esrg_p     FREE_TREE_EVENT_LIST = NULL;
static int      count = 0;
/******* memory  allocation of  matrix of int *******/

MATRIX MATRIX_ALLOCATION() {
    int i = 0;
    int **tmp;

    tmp = (int **)calloc(ncl, sizeof(int *));
    for (i = 0; i < ncl; i++)
        tmp[i] = (int *)calloc(MAX_CARD, sizeof(int));
    return tmp;
}


MATRIX *MATRIX_3D_ALLOCATION() {
    int i = 0, j = 0;
    int *** tmp;

    tmp = (int ** *)calloc(ncl, sizeof(int **));
    for (i = 0; i < ncl; i++) {
        tmp[i] = (int **)calloc(MAX_CARD, sizeof(int *));
        for (j = 0; j < MAX_CARD; j++)
            tmp[i][j] = (int *)calloc(MAX_CARD, sizeof(int));

    }

    return tmp;
}

MATRIX **MATRIX_4D_ALLOCATION() {
    int i = 0, j = 0, k = 0;
    int ** **tmp;

    tmp = (int ** **)calloc(ncl, sizeof(int ** *));

    for (i = 0; i < ncl; i++) {
        tmp[i] = (int ** *)calloc(MAX_CARD, sizeof(int **));
        for (j = 0; j < MAX_CARD; j++) {
            tmp[i][j] = (int **)calloc(MAX_CARD, sizeof(int *));
            for (k = 0; k < MAX_CARD; k++)
                tmp[i][j][k] = (int *)calloc(MAX_CARD, sizeof(int));
        }

    }

    return tmp;
}


/*****************************************************/


/******** memory allocation of an array of int  *****/

void *ARRAY_ALLOCATION(int NB) {
    return calloc(NB, sizeof(int));
}

/****************************************************/

/******** memory allocation of for a stored  static s-classes conf, and the card structure  *****/

void *STATICSBC_ALLOCATION(int NB) {
    return  calloc(NB, sizeof(struct STATICSBC));
}

STORE_STATICS_ARRAY CREATE_STORE_STATIC_STRUCTURE() {
    return (STORE_STATICS_ARRAY) calloc(ncl, sizeof(struct  STORE_STATICs));
}

int ** *CREATE_STORE_CARD_STRUCTURE() {
    int i, j;
    int *** CARD;

    CARD      = (ARRAY **)  ARRAY_ALLOCATION(ncl);
    for (i = 0 ; i < ncl ; i++) {
        CARD[i] = (ARRAY *) ARRAY_ALLOCATION(MAX_CARD);
        for (j = 0; j < MAX_CARD ; j++)
            CARD[i][j] = (ARRAY) ARRAY_ALLOCATION(MAX_CARD);
    }
    return CARD;
}

/*************************************************************************************************/

/************* Memory allocation  of an array of pointers and  elements used to find            **********/
/************* the correcte positions of the various conf. of the SBCs (also the free function) *********/

struct MY_STATICS  **ARRAY_LMS_ALLOCATION() {
    return (struct MY_STATICS **)calloc(ncl, sizeof(struct MY_STATICS *));
}

struct MY_STATICS *ELEM_LMS_ALLOCATION() {
    struct MY_STATICS *res = NULL;

    if (FREE_STATICS_LIST == NULL) res = (struct MY_STATICS *)malloc(sizeof(struct MY_STATICS));
    else {
        res = FREE_STATICS_LIST;
        FREE_STATICS_LIST =  FREE_STATICS_LIST->next;
    };
    res->sbc_num = UNKNOWN;
    res->card = UNKNOWN;
    res->offset = UNKNOWN;
    res->dyn_sbc = UNKNOWN;
    res->to_test = UNKNOWN;
    res->Org = NULL;
    res->next = NULL;
    res->prev = NULL;

    return res;
}

void ELEM_LMS_FREE(struct MY_STATICS *to_free) {
    to_free->next = FREE_STATICS_LIST;
    FREE_STATICS_LIST = to_free;
}

void FREE_ALL_LMS_ELEMS() {
    int i;
    LIST_MY_STATICS Free, Next;
    if (STATICS_LIST_ARRAY) {
        for (i = 0; i < ncl; i++) {
            Free = STATICS_LIST_ARRAY[i];
            while (Free) {
                Next = Free->next;
                ELEM_LMS_FREE(Free);
                Free = Next;
            };
            STATICS_LIST_ARRAY[i] = NULL;
        }
        free(STATICS_LIST_ARRAY);
        STATICS_LIST_ARRAY = NULL;
    }
}

/********************************************************************************************/

/**************** memory allaction and free for structure used as mapps between DSCs *******/
/**************                 in two different conf. of SBCs                       *******/

INT_LISTP DYN_LIST_ALLOCATION() {
    INT_LISTP res;
    if (FREE_DYN_LIST == NULL) {res = (INT_LISTP)malloc(sizeof(struct INT_LIST)); ALL_DYN++;}
    else {
        res = FREE_DYN_LIST;
        FREE_DYN_LIST = FREE_DYN_LIST->next;
        res->next = NULL;
        res->group = NULL;
        res->dyn_sbc = 0;
        POP_DYN++;
    };
    return res;
}

int COUNT_DYN_LIST() {
    int count = 0;
    INT_LISTP res = FREE_DYN_LIST;
    while (res) {count++; res = res->next;};
    return count;
}
void DYN_LIST_FREE(INT_LISTP to_free) {
    to_free->next = FREE_DYN_LIST;
    FREE_DYN_LIST = to_free;
    FREE_DYN++;
}

int  **MAP_MERGING_ALLOCATION() {
    int i = 0;
    int **tmp;

    tmp = (int **)calloc(ncl, sizeof(int *));
    for (i = 0; i < ncl; i++)
        tmp[i] = (int *)calloc(MAX_CARD, sizeof(int));

    return tmp;
}

INT_LISTP **MAP_DECOMPOSING_ALLOCATION() {
    int i, j;
    INT_LISTP **MAPPE_ARRAY;

    MAPPE_ARRAY = (INT_LISTP **)calloc(ncl, sizeof(INT_LISTP *));
    for (i = 0; i < ncl; i++)
        MAPPE_ARRAY[i] = (INT_LISTP *)calloc(MAX_CARD, sizeof(INT_LISTP));

    return MAPPE_ARRAY;
}

void MAP_DECOMPOSING_FREE() {
    int i, j;
    INT_LISTP ptr1, Next;



    for (i = 0; i < ncl; i++)
        for (j = 0; j < MAX_CARD; j++) {
            ptr1 = Next = DECOMP_MAPPE[i][j];
            DECOMP_MAPPE[i][j] = NULL;
            while (Next) {
                ptr1 = Next->next;
                DYN_LIST_FREE(Next);
                Next = ptr1;

            }
        }


}
/*******************************************************************************************/


/*********** Memory allocation for RG SET *************************************************/
Tree_Esrg_p TREE_EVENT_ALLOCATION(int buff, int length) {

    Tree_Esrg_p res = NULL;

    NB_EV_IN_GRAPH++;

    if (FREE_TREE_EVENT_LIST == NULL) {

        TREENODEEVENT_MALL_TOT++;
        res = (Tree_Esrg_p)malloc(sizeof(struct TREE_ESRG));
        res->marking = (MInfo_p)emalloc(sizeof(struct MARKING_INFO));

    }
    else {
        TREENODEEVENT_POP_TOT++;
        res = FREE_TREE_EVENT_LIST;
        FREE_TREE_EVENT_LIST = FREE_TREE_EVENT_LIST->Head_Next_Event;
    }
    res->marking->marking_as_string = buff;
    res->marking->length = length;

    return res;
}

void TREE_EVENT_FREE(Tree_Esrg_p to_free) {
    NB_EV_IN_GRAPH--;
    to_free->Head_Next_Event = FREE_TREE_EVENT_LIST;
    FREE_TREE_EVENT_LIST = to_free;
}



Tree_p my_treenode_pop(int buff, int length) {

    Tree_p res = NULL;

    res = (Tree_p)malloc(sizeof(struct TREE));
    res->marking = (MInfo_p)emalloc(sizeof(struct MARKING_INFO));
    res->left = res->right = res->last = NULL;
    res->enabled_head = NULL;
    res->marking->marking_as_string = buff;
    res->marking->length = length;

    return res;
}
/******************************************************************************************/
