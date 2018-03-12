/*****************************************************/
/*  functions used to extracte the correspendance    */
/* between DSCs of to different versions of SSC      */
/*                while decomposing                  */
/*****************************************************/


#include <stdio.h>

#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"

extern void     *DYN_LIST_ALLOCATION();
extern int       GET_OFFSET_FROM_LIST();




void ADD_DYN_SBC(INT_LISTP *HEAD, int dyn_sbc) {
    INT_LISTP ptr = (*HEAD);
    (*HEAD) = DYN_LIST_ALLOCATION();
    (*HEAD)->dyn_sbc = dyn_sbc;
    (*HEAD)->next = ptr;
}

void INIT_DECOMPOSING_MAP() {
    int i, j;

    for (i = 0; i < ncl; i++)
        for (j = 0; j < MAX_CARD; j++) DECOMP_MAPPE[i][j] = NULL;

}

int TEST_IF_ORG(int sbc_num, TO_MERGEP merg) {
    if (merg->sbc_num == sbc_num) return TRUE;
    return FALSE;
}

int GET_DYN_FROM_TOTAL(int cl, int ds, struct COLORS *tabc, int **num)

{
    int ss , dtot = 0 , ret_value = UNKNOWN, diff;

    for (ss = 0, diff = ds ; ss < tabc[cl].sbc_num ; ss++) {
        dtot += num[cl][ss];
        if (ds < dtot)
            return (tabc[cl].sbclist[ss].offset + diff);
        diff -= num[cl][ss];
    }
    return -1000;
}

int GET_STATIC_SUBCLASSES(int cl, int dsc, struct COLORS *tabc)

{
    int i , dtot = 0 , ret_value = UNKNOWN;

    for (i = 0 ; i < tabc[cl].sbc_num ; i++) {
        dtot += tabc[cl].sbclist[i].card;
        if (dsc < dtot)
            return i;
    }
    return -1000;
}

void MAPING_AFTER_DECOMPOSING(DECOMP_TO Decomp, struct COLORS *tabc, int **num, STORE_STATICS_ARRAY  STATIC_STORE,
                              int *** eventuality, INT_LISTP *** MAPPE_ARRAY, int Class)

{

    int i, j, ss, New_Offset, bouc;
    int No_Offset;
    DECOMP_TO Dec;
    INT_LISTP ptr;

    Dec = Decomp;
    while (Dec) {
        ss = Dec->sbc_num;
        for (i = 0; i < Dec->NB; i++) {
            New_Offset = STATIC_STORE[Class].ptr[Dec->SSCs[i]].offset;
            No_Offset = 0;
            for (j = 0; j < num[Class][ss]; j++)
                if (eventuality[ss][j][i] > 0) {
                    ADD_DYN_SBC(&((*MAPPE_ARRAY)[Class][j + tabc[Class].sbclist[ss].offset]), No_Offset + New_Offset);
                    No_Offset++;
                }
        }
        Dec = Dec->next;
    }
}

void ALL_MAPING_AFTER_DECOMPOSING(DECOMP_TO *Decomp, struct COLORS *tabc, int **num, STORE_STATICS_ARRAY  STATIC_STORE,
                                  int ** **EVENTS , INT_LISTP *** MAPPE_ARRAY) {
    int i;

    for (i = 0; i < ncl; i++)
        MAPING_AFTER_DECOMPOSING(Decomp[i], tabc, num, STATIC_STORE, EVENTS[i], MAPPE_ARRAY, i);

}

/************** Verifier    ************/
