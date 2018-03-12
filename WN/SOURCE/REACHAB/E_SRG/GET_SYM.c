

/************************************************************/
/*    these functions are used to compute the symetric conf.*/
/*             a symbolic marking of asymtries              */
/************************************************************/



#include <stdio.h>

#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"

extern int    GET_STATIC_FROM_LIST() ;
extern int    GET_DYN_FROM_LIST();
extern int    GET_CARD_FROM_LIST();
extern int    GET_OFFSET_FROM_LIST();
extern void   ALL_MAPING_AFTER_MERGING();
extern void   INIT_MERGING_MAP();
extern int    get_static_subclass();


void MERGING_SSBC(TO_MERGEP ST, int *** Card_C, int **Num_C, int Class) {
    int i, j, count = 0;
    int *tmp;


    count = Num_C[Class][ST->SSCs[0]];

    for (i = 1; i < ST->NB; i++)
        for (j = 0; j < Num_C[Class][ST->SSCs[i]]; j++) {
            Card_C[Class][ST->SSCs[0]][count] = Card_C[Class][ST->SSCs[i]][j];
            Card_C[Class][ST->SSCs[i]][j] = 0;
            Num_C[Class][ST->SSCs[0]]++;
            count++;
        }

    if (ST->sbc_num != ST->SSCs[0]) {
        tmp = Card_C[Class][ST->sbc_num];
        Card_C[Class][ST->sbc_num] = Card_C[Class][ST->SSCs[0]];
        Card_C[Class][ST->SSCs[0]] = tmp;
    }

}
void NEW_SYM_TOT(STORE_STATICS_ARRAY STORED_CONF, int **tot, int **num) {
    int i, j;

    for (i = 0; i < ncl; i++) {
        (*tot)[i] = 0;
        for (j = 0; j < STORED_CONF[i].sbc_num; j++)
            (*tot)[i] += num[i][j];

    }

}
void NEW_SYM_TABC(struct COLORS **tabc, STORE_STATICS_ARRAY STORED_STATIC_CONF) {
    int i;
    for (i = 0; i < ncl; i++) {
        (*tabc)[i].sbclist =  STORED_STATIC_CONF[i].ptr;
        (*tabc)[i].sbc_num =  STORED_STATIC_CONF[i].sbc_num;
    }

}


//void NEW_SYM_CARD_NUM( TO_MERGEP * merg,int**** card, int*** num,struct COLORS* tabc)
void NEW_SYM_CARD_NUM(TO_MERGEP *merg, int *** card, int **num, struct COLORS *tabc) {
    int i;

    TO_MERGEP ptr = NULL;

    for (i = 0; i < ncl; i++) {
        ptr = merg[i];

        while (ptr) {

            MERGING_SSBC(ptr, card, num, i);
            ptr = ptr->next;
        }

    }
}

static int optimized_get_number_of_ordinary_tokens(Token_p ptr, int pl)

{
    int ret_value = 1;
    int ii = GET_PLACE_COMPONENTS(pl);
    int cl, ss;

    for (; ii ; ii--) {
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
        ss = get_static_subclass(cl, ptr->id[ii - 1]);
        ret_value *=  card[cl][ss][ptr->id[ii - 1] - tabc[cl].sbclist[ss].offset];

    }
    return (ret_value);
}


void NEW_SYMETRIC_NET_MARKING(struct NET_MARKING **net_mark) {
    int i, j;
    int ordinary_tokens;
    Token_p ptr;

    for (i = 0; i < npl; i++) {
        if (!IS_NEUTRAL(i)) {
            ptr = (*net_mark)[i].marking;
            /*********** nouvelle modif ********/
            (*net_mark)[i].different = 0;
            (*net_mark)[i].total = 0;
            (*net_mark)[i].ordinary = 0;
            /***********************************/
            while (ptr) {
                for (j = 0; j < tabp[i].comp_num; j++) {
                    ptr->id[j] = MERGING_MAPPE[tabp[i].dominio[j]][ptr->id[j]];
                }

                ordinary_tokens = optimized_get_number_of_ordinary_tokens(ptr, i);
                (*net_mark)[i].ordinary += ordinary_tokens;
                (*net_mark)[i].total += (ptr->molt) *  ordinary_tokens;
                (*net_mark)[i].different++;
                ptr = ptr->next;
            }
        }
    }

}

void  GET_SYMETRIC_MARKING(struct COLORS **tabc, struct NET_MARKING **net_marking, int ** **card, int *** num, int **tot,
                           STORE_STATICS_ARRAY STORED_STATIC_CONF, TO_MERGEP *merg)

{


    INIT_MERGING_MAP();
    ALL_MAPING_AFTER_MERGING(merg, (*tabc), (*num), &MERGING_MAPPE);
    NEW_SYM_CARD_NUM(merg, (*card), (*num), (*tabc));
    NEW_SYM_TOT(STORED_STATIC_CONF, tot, (*num));
    NEW_SYM_TABC(tabc, STORED_STATIC_CONF);
    NEW_SYMETRIC_NET_MARKING(net_marking);


}
/************* Verifier ************/
