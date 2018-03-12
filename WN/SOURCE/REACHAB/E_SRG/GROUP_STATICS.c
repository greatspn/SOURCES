
/**************************************************/
/*    functions used for manipulating SBCs while  */
/*                       grouping                 */
/**************************************************/



#include <stdio.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/service.h"


extern struct MY_STATICS **ARRAY_LMS_ALLOCATION();
extern void ELEM_LMS_FREE();

void COMPUTE_CARD_OFFSET_DYN_AFTER_GROUPING(LIST_MY_STATICS first, LIST_MY_STATICS second) {
    LIST_MY_STATICS ptr, last;
    ptr = last = first;

    ptr->card += second->card;
    ptr->dyn_sbc += second->dyn_sbc;
    ptr = ptr->next;

    while (ptr) {
        ptr->offset = (last->offset) + (last->card);
        last = ptr;
        ptr = ptr->next;
    }
}


void GROUP_2_STATICS(TO_MERGEP merg, int first, int second, int Class) {
    int end = FALSE;

    LIST_MY_STATICS ptr = STATICS_LIST_ARRAY[Class];
    LIST_MY_STATICS Frt, Scd;

    while ((ptr) && (!end)) {
        if (ptr->sbc_num == first)  {Frt = ptr; Frt->Org = merg;}
        if (ptr->sbc_num == second) {Scd = ptr; end = TRUE;}

        ptr = ptr->next;
    }

    if (Scd->next == NULL) {
        (Scd->prev)->next = NULL;
        COMPUTE_CARD_OFFSET_DYN_AFTER_GROUPING(Frt, Scd);
        ELEM_LMS_FREE(Scd);
    }

    else                {
        (Scd->next)->prev = Scd->prev;
        (Scd->prev)->next = Scd->next;
        COMPUTE_CARD_OFFSET_DYN_AFTER_GROUPING(Frt, Scd);
        ELEM_LMS_FREE(Scd);
    }

}



void GROUP_STATICS(TO_MERGEP Merg, int Class) {
    int i;

    i = (Merg->NB) - 1;

    while (i) {
        GROUP_2_STATICS(Merg, Merg->SSCs[i - 1], Merg->SSCs[i], Class);
        i--;
    }
}

void GROUPING_ALL_STATICS(TO_MERGEP *merg, struct COLORS *tabc, int **num) {
    int i, j, Offset;
    TO_MERGEP last, ptr = NULL;

    if (!STATICS_LIST_ARRAY) {

        STATICS_LIST_ARRAY = (LIST_MY_STATICS *) ARRAY_LMS_ALLOCATION();

        for (i = 0; i < ncl; i++) {
            INIT_LIST_STATICS(&(STATICS_LIST_ARRAY[i]), tabc, num, i);

            ptr = merg[i];

            while (ptr) {
                GROUP_STATICS(ptr, i);
                ptr = ptr->next;
            }

            ORDER(STATICS_LIST_ARRAY[i]);
        }
    }
}

