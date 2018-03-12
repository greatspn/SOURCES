
/***************************************************************/
/* In this file are coded the functions to get the eventuality */
/* which  correspends to the asymetry of a symbilic marking    */
/***************************************************************/


#include <stdio.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/service.h"

extern void INIT_4D_MATRIX();

int GET_MAX_CARD(struct COLORS *tabc) {
    int ret_value = 0;
    int ii;

    for (ii = 0; ii < ncl ; ii++)
        if (tabc[ii].card > ret_value)
            ret_value = tabc[ii].card ;
    return (ret_value);

}


int TEST_IF_EXIST(int sbc_num, TO_MERGEP merg) {
    int i;
    for (i = 0; i < merg->NB; i++)
        if (merg->SSCs[i] == sbc_num)
            return i;
    return END;
}


/****** THIS function works correctly if the symetrical marcking is got from the archive ****/

void GET_EVENT_FROM_MARKING(struct COLORS *tabc, int *tot, int **num, int *** card, int **MAPPE_ARRAY,
                            STORE_STATICS_ARRAY  STATIC_STORE, int *** STORE_CARD, TO_MERGEP *merg) {

    int i = 0 , j = 0 , k = 0, ss, s, ds;
    int dyn_sym, Class, line, tmp;
    TO_MERGEP ptr = NULL;

    INIT_4D_MATRIX(RESULT);
    for (Class = 0; Class < ncl; Class++)
        for (i = 0; i < tot[Class]; i++) {
            ds = get_dynamic_from_total(Class, i);
            ss = get_static_subclass(Class, ds);
            dyn_sym = ds - tabc[Class].sbclist[ss].offset;

            for (j = 0; j < tabc[Class].card; j++)
                if (MAPPE_ARRAY[Class][j] == ds) {
                    for (k = 0; k < STATIC_STORE[Class].sbc_num; k++)
                        if ((j >= STATIC_STORE[Class].ptr[k].offset) &&
                                (j < ((STATIC_STORE[Class].ptr[k].offset) + (STATIC_STORE[Class].ptr[k].card)))) {
                            ptr = merg[Class];
                            do {
                                s = ptr->sbc_num;
                                line = TEST_IF_EXIST(k, ptr);
                                ptr = ptr->next;
                            }
                            while (line == END);
                            /** to keep the cardinality **/
                            RESULT[Class][s][dyn_sym][line] = STORE_CARD[Class][k][j - (STATIC_STORE[Class].ptr[k].offset)];

                            break;
                        }
                }
        }
}



/************************ Verifier ****************/

