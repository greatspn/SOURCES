/*****************************************************/
/*   functions used to store and extracte differente */
/*      versions of static sub-classes               */
/*****************************************************/

#include <stdio.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/service.h"

extern void *ARRAY_ALLOCATION();
extern void *STATICSBC_ALLOCATION();


void STORE_CARD(int *** card_to_store, int ** **stored_card) {
    int i, j, k;

    for (i = 0; i < ncl; i++) {
        for (j = 0; j < tabc[i].sbc_num; j++) {
            for (k = 0; k < num[i][j]; k++)
                (*stored_card)[i][j][k] = card_to_store[i][j][k];
        }
    }
}

void STORE_SYM_TOT(int *tot, int *sym_tot) {
    int i;
    for (i = 0; i < ncl; i++)
        sym_tot[i] = tot[i];
}

void  NEW_SIM_STATIC_CONF(STORE_STATICS_ARRAY *STATIC_STORE) {
    int i, j;

    for (i = 0; i < ncl; i++) {
        (*STATIC_STORE)[i].sbc_num = GET_STATIC_FROM_LIST(i) ;
        (*STATIC_STORE)[i].ptr = (Static_p)STATICSBC_ALLOCATION(MAX_CARD);
        for (j = 0; j < (*STATIC_STORE)[i].sbc_num; j++) {
            (*STATIC_STORE)[i].ptr[j].card = GET_CARD_FROM_LIST(j, i);
            (*STATIC_STORE)[i].ptr[j].offset = GET_OFFSET_FROM_LIST(j, i);
        }
    }

}


void STORE_STATIC_CONF(STORE_STATICS_ARRAY *STATIC_STORE, struct COLORS *tabc) {
    int i, j, k;

    for (i = 0; i < ncl; i++) {
        (*STATIC_STORE)[i].ptr = tabc[i].sbclist;
        (*STATIC_STORE)[i].sbc_num = tabc[i].sbc_num;
    }
}


void POP_STATIC_CONF(STORE_STATICS_ARRAY STATIC_STORE, struct COLORS **tabc) {
    int i;
    for (i = 0; i < ncl; i++) {
        (*tabc)[i].sbclist = STATIC_STORE[i].ptr;
        (*tabc)[i].sbc_num = STATIC_STORE[i].sbc_num;
    }

}
