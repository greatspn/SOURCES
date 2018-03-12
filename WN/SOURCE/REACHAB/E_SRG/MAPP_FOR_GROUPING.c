/*****************************************************/
/*  functions used to extracte the correspendance    */
/* between DSCs of to different versions of SSC      */
/*                while grouping                     */
/*****************************************************/


#include <stdio.h>

#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"

extern int       GET_OFFSET_FROM_LIST();

void INIT_MERGING_MAP() {
    int i, j;

    for (i = 0; i < ncl; i++)
        for (j = 0; j < MAX_CARD; j++) MERGING_MAPPE[i][j] = -1;

}

void MAPING_AFTER_MERGING(int Class, struct COLORS *tabc, TO_MERGEP Merg, int *** MAPPE_ARRAY, int **num) {
    int i, j, tmp, count;

    tmp = (Merg->NB) - 1;
    for (j = tmp; j >= 0; j--) {
        count = 0;
        for (i = 0; i < j; i++) count += num[Class][Merg->SSCs[i]];

        for (i = 0; i < num[Class][Merg->SSCs[j]]; i++)
            (*MAPPE_ARRAY)[Class][MY_GET_STATIC_OFFSET(tabc, Class, Merg->SSCs[j]) + i] = GET_OFFSET_FROM_LIST(Merg->sbc_num, Class) + count + i;
    }

}

void ALL_MAPING_AFTER_MERGING(TO_MERGEP *merg, struct COLORS *tabc, int **num, int *** MAPPE_ARRAY) {
    int i;
    TO_MERGEP last, ptr = NULL;

    for (i = 0; i < ncl; i++) {
        ptr = merg[i];

        while (ptr) {

            MAPING_AFTER_MERGING(i, tabc, ptr, MAPPE_ARRAY, num);
            ptr = ptr->next;
        }

    }
}


/***************** Verifier ***************/
