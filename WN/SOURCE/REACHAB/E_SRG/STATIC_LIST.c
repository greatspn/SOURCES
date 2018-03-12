/******************************************************/
/* basic functions for manipulating list of SBCs      */
/******************************************************/

#include <stdio.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"

extern struct MY_STATICS *ELEM_LMS_ALLOCATION();

void INIT_LIST_STATICS(struct MY_STATICS **head, struct COLORS *tabc, int **num, int Class) {
    int i;
    LIST_MY_STATICS ptr = NULL, last = NULL;
    (*head) = ptr = last = ELEM_LMS_ALLOCATION();

    ptr->sbc_num = 0;
    ptr->card = tabc[Class].sbclist[0].card;
    ptr->offset = tabc[Class].sbclist[0].offset;
    ptr->dyn_sbc = num[Class][0];
    ptr->Org = NULL;

    for (i = 1; i < tabc[Class].sbc_num; i++) {
        last->next = ptr = ELEM_LMS_ALLOCATION();
        ptr->prev = last;
        last = ptr;
        ptr->sbc_num = i;
        ptr->card = tabc[Class].sbclist[i].card;
        ptr->offset = tabc[Class].sbclist[i].offset;
        ptr->dyn_sbc = num[Class][i];
        ptr->Org = NULL;
    }
}

void ORDER(LIST_MY_STATICS head) {
    int i = 0;
    LIST_MY_STATICS ptr = head;

    while (ptr) {
        ptr->sbc_num = i;
        if (ptr->Org)(ptr->Org)->sbc_num = i;
        i++;
        ptr = ptr->next;
    }
}

int GET_STATIC_FROM_LIST(int Class) {
    int count = 0;
    LIST_MY_STATICS ptr = STATICS_LIST_ARRAY[Class];

    while (ptr) {
        count++;
        ptr = ptr->next;
    }
    return count;
}

int GET_CARD_FROM_LIST(int sbc_num, int Class) {
    LIST_MY_STATICS ptr = STATICS_LIST_ARRAY[Class];

    while (ptr) {
        if ((ptr->sbc_num) == sbc_num) return ptr->card;
        ptr = ptr->next;
    }
    return -1000;
}

int GET_OFFSET_FROM_LIST(int sbc_num, int Class) {
    LIST_MY_STATICS ptr = STATICS_LIST_ARRAY[Class];

    while (ptr) {
        if ((ptr->sbc_num) == sbc_num) return ptr->offset;
        ptr = ptr->next;
    }
    return -1000;
}

int GET_DYN_FROM_LIST(int sbc_num, int Class) {
    LIST_MY_STATICS ptr = STATICS_LIST_ARRAY[Class];

    while (ptr) {
        if ((ptr->sbc_num) == sbc_num) return ptr->dyn_sbc ;
        ptr = ptr->next;
    }
    return -1000;
}

