#include <stdio.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/service.h"

extern void MAP_DECOMPOSING_FREE();

extern Token_p pop_token();
extern int get_max_place_domain();
extern Cart_p create_cartesian_product();
extern void optimized_set_token_marking();
extern void optimized_get_pointer_info();
extern int  optimized_get_number_of_ordinary_tokens();
extern void optimized_init_token_values();
extern void insert_token();
extern void push_token_list();
extern void push_token();
extern void SPECIAL_CANISATION();
extern void ALL_MY_GROUP();
extern void AFTER_CANONISATION();

extern Canonic_p sfl_h;

void int_cartesian_product() {
    int i;
    for (i = 0; i < get_max_place_domain(); i++) {
        Res->low[i] = 0; Res->up[i] = 0;
        Counter->low[i] = 0; Counter->low[i] = 0;
    }
}

void NEW_ASYM_CARD_NUM(int ** **card, int *** num, struct COLORS *tabc, int Class, DECOMP_TO Dec, int *** eventuality) {
    int i, j, store, ss, k, ind, bouc;
    DECOMP_TO Decomp = Dec;

    if (IS_ORDERED(Class) && (NO_SAT_INI)) {
        store = (*num)[Class][0];
        (*num)[Class][0] = 0;
        for (k = 0; k < MAX_CARD; k++)
            (*card)[Class][0][k] = 0;
        ind = 0;
        for (i = 0; i < store; i++)
            for (j = 0; j < store; j++)
                if (eventuality[0][j][i] > 0) {
                    (*card)[Class][0][ind] = eventuality[0][j][i];
                    (*num)[Class][0]++;
                    ind++;
                }
    }

    else {
        while (Decomp->next) {Decomp = Decomp->next;}
        do {
            ss = Decomp->sbc_num;
            store = (*num)[Class][ss];
            for (j = 0; j < Decomp->NB; j++) {
                (*num)[Class][Decomp->SSCs[j]] = 0;
                for (k = 0; k < MAX_CARD; k++)
                    (*card)[Class][Decomp->SSCs[j]][k] = 0;

            }

            for (i = 0; i < Decomp->NB; i++) {
                ind = 0;
                for (j = 0; j < store; j++)
                    if (eventuality[ss][j][i] > 0) {
                        (*card)[Class][Decomp->SSCs[i]][ind] = eventuality[ss][j][i];
                        (*num)[Class][Decomp->SSCs[i]]++;
                        ind++;
                    }
            }

            Decomp = Decomp->prev;
        }
        while (Decomp != NULL);
    }


}

void NEW_ASYM_TOT(int **tot, int **num, struct COLORS *tabc) {
    int i, j;

    for (i = 0; i < ncl; i++) {
        (*tot)[i] = 0;
        for (j = 0; j < tabc[i].sbc_num; j++)
            (*tot)[i] += num[i][j];
    }

}
void NEW_ASYM_TABC(struct COLORS **tabc, STORE_STATICS_ARRAY  STATIC_STORE) {
    int i;
    for (i = 0; i < ncl; i++) {
        (*tabc)[i].sbclist = STATIC_STORE[i].ptr;
        (*tabc)[i].sbc_num = STATIC_STORE[i].sbc_num;
    }

}

/****************************************************************************/
static int get_num_elem(INT_LISTP ptr) {
    int result = 0;
    while (ptr) {
        result++;
        ptr = ptr->next;
    }
    return result;
}

static int get_dsc_with_position(INT_LISTP ptr, int pos) {
    int result = 1;
    while ((ptr) && (result != pos))
    {result++; ptr = ptr->next;}
    return ptr->dyn_sbc;
}

static void Init_UP_Array(Cart_p Counter, Cart_p Res, INT_LISTP **MAPPE_ARRAY, int place, Token_p token) {
    int j, res;
    for (j = 0; j < tabp[place].comp_num; j++) {
        res = get_num_elem(MAPPE_ARRAY[tabp[place].dominio[j]][token->id[j]]);
        if (res == 0) { Counter->low[j] = 0; Res->low[j] = token->id[j];}
        else {
            Counter->low[j] = 1;
            Res->low[j] = get_dsc_with_position(MAPPE_ARRAY[tabp[place].dominio[j]][token->id[j]], 1);
        }

        Counter->up[j] = res;
    }
}


int my_optimized_nextvec(Cart_p Counter, Cart_p Res, int comp, Token_p token , INT_LISTP **MAPPE_ARRAY, int place) {
    int r, flag;

    if (comp) {
        r = 0;
        do {
            flag = TRUE;
            if (Counter->low[r] != 0)
                if (Counter->low[r] + 1  > Counter->up[r]) {
                    Res->low[r] = get_dsc_with_position(MAPPE_ARRAY[tabp[place].dominio[r]][token->id[r]], 1);
                    Counter->low[r] = 1;
                    r++;
                    flag = FALSE;
                }
                else {
                    Counter->low[r]++;
                    Res->low[r] = get_dsc_with_position(MAPPE_ARRAY[tabp[place].dominio[r]][token->id[r]], Counter->low[r]);
                }
        }
        while ((r < comp) && (!flag));
        if (r < comp)
            return (TRUE);
        else
            return (FALSE);
    }
    else
        return (FALSE);
}

void my_optimized_set_token_marking(Cart_p cartes_ptr , int delta , int pl , int inc_flag)

{

    int old_molt , new_molt;
    int status = 0;

    Token_p tok_ptr = NULL;

    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;


    optimized_get_pointer_info(cartes_ptr->low , pl , res_ptr);
    tok_ptr = GET_TOKEN_POSITION(res_ptr);
    old_molt = GET_OLD_MOLTEPLICITY(tok_ptr);
    new_molt = old_molt + delta;
    if (new_molt < 0)
        out_error(ERROR_NEGATIVE_MARKING, pl, 0, 0, 0, NULL, NULL);
    else {

        int ordinary_tokens = optimized_get_number_of_ordinary_tokens(cartes_ptr, pl);
        net_mark[pl].total = net_mark[pl].total + (delta * ordinary_tokens);


        status = (old_molt > 0) << 1;
        status |= (new_molt == 0);


        Token_p add_ptr = NULL;

        switch (status) {
        case OLD_GT_NEW_EQ :
            (net_mark[pl].different)--;
            net_mark[pl].ordinary -= ordinary_tokens;
            delete_token(res_ptr, pl);
            push_token(tok_ptr, pl);
            break;
        case OLD_GT_NEW_GT :
            SET_TOKEN_MOLTEPLICITY(new_molt, tok_ptr);
            break;
        case OLD_EQ_NEW_GT :
            (net_mark[pl].different)++;
            net_mark[pl].ordinary += ordinary_tokens;
            add_ptr = pop_token(pl);
            optimized_init_token_values(cartes_ptr->low, add_ptr, pl);
            SET_TOKEN_MOLTEPLICITY(new_molt, add_ptr);
            insert_token(res_ptr, add_ptr, pl);
            break;
        case OLD_EQ_NEW_EQ :
            break;
        }

    }
}
void NEW_ASYMETRIC_NET_MARK(INT_LISTP **MAPPE_ARRAY) {
    int i, j;
    Token_p token, ptr, to_dispose;

    int_cartesian_product();
    for (i = 0; i < npl; i++) {
        if (!IS_NEUTRAL(i)) {
            ptr = net_mark[i].marking;
            net_mark[i].marking = NULL;
            net_mark[i].marking_t = NULL;
            net_mark[i].different = 0;
            net_mark[i].total = 0;
            net_mark[i].ordinary = 0;

            while (ptr) {
                Init_UP_Array(Counter, Res, MAPPE_ARRAY, i, ptr) ;
                do {
                    my_optimized_set_token_marking(Res , ptr->molt , i , TOTAL);
                }
                while (my_optimized_nextvec(Counter, Res, tabp[i].comp_num, ptr, MAPPE_ARRAY, i));
                to_dispose = ptr;
                ptr = ptr->next;
                push_token(to_dispose, i);
            }
        }
    }
}
/*****************************************************************************/
void NEW_ASYM_MARKING(struct COLORS **tabc,
                      struct NET_MARKING **net_marking,
                      int ** **card, int *** num, int **tot,
                      DECOMP_TO *Decomp,
                      STORE_STATICS_ARRAY  STATIC_STORE,
                      int ** **EVENTS) {
    int i;

    INIT_DSC_SSBC();
    ALL_MAPING_AFTER_DECOMPOSING(Decomp, (*tabc), (*num),
                                 STATIC_STORE, EVENTS,
                                 &DECOMP_MAPPE);
    NEW_ASYM_TABC(tabc, STATIC_STORE);
    for (i = 0; i < ncl; i++)
        NEW_ASYM_CARD_NUM(card, num, (*tabc), i,
                          Decomp[i], EVENTS[i]);
    NEW_ASYM_TOT(tot, (*num), (*tabc));
    NEW_ASYMETRIC_NET_MARK(DECOMP_MAPPE);
    MAP_DECOMPOSING_FREE();

    SPECIAL_CANISATION();


    // SPECIAL_CANISATION();
}

/******** Verifier ************/
/*****************************************************************************/
void TEST_NEW_ASYM_MARKING(struct COLORS **tabc,
                           struct NET_MARKING **net_marking,
                           int ** **card, int *** num, int **tot,
                           DECOMP_TO *Decomp,
                           STORE_STATICS_ARRAY  STATIC_STORE,
                           int ** **EVENTS) {
    int i;

    INIT_DSC_SSBC();
    ALL_MAPING_AFTER_DECOMPOSING(Decomp, (*tabc), (*num),
                                 STATIC_STORE,
                                 EVENTS,
                                 &DECOMP_MAPPE);

    NEW_ASYM_TABC(tabc, STATIC_STORE);
    for (i = 0; i < ncl; i++)
        NEW_ASYM_CARD_NUM(card, num, (*tabc), i, Decomp[i], EVENTS[i]);
    NEW_ASYM_TOT(tot, (*num), (*tabc));
    NEW_ASYMETRIC_NET_MARK(DECOMP_MAPPE);
    MAP_DECOMPOSING_FREE();

    SPECIAL_CANISATION();
}
/******** Verifier ************/
