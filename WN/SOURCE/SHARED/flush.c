# include <stdio.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/fun_ext.h"
# include "../../INCLUDE/macros.h"

#ifdef FLUSH_FUNCTION

extern int plus_op();
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int matching(Cart_p  cartes_ptr,  int  *id,  int  pl,  int  comp) {
    int i, ret_value = TRUE;

    for (i = comp; i; i--)
        if (!cartes_ptr->up[i - 1])
            if (cartes_ptr->mark[i - 1][0] != id[i - 1])
            {ret_value = FALSE; goto ret;}
ret: return (ret_value);
}
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_to_first(Cart_p  cartes_ptr,  int  pl,  int  comp,  TokInfo_p  ret_ptr) {
    /* Init get_to_first */
    Token_p pos_ptr = NULL;
    Token_p prv_ptr = NULL;

    int status = 0;

    ret_ptr->pos = ret_ptr->prv = NULL;
    if (IS_NEUTRAL(pl)) {
        /* Posto neutro */
        ret_ptr->pos = net_mark[pl].marking;
    }/* Posto neutro */
    else {
        /* Posto colorato */
        // if(IS_FULL(pl))
        if (net_mark[pl - 1].marking) {
            /* C'e' almeno un token */
            for (pos_ptr = net_mark[pl].marking; pos_ptr != NULL ; pos_ptr = pos_ptr->next) {
                /* Per ogni token della marcatura di pl */
                if (matching(cartes_ptr , pos_ptr->id , pl, comp)) {
                    ret_ptr->pos = pos_ptr;
                    ret_ptr->prv = prv_ptr;
                    goto ok;
                }
                prv_ptr = pos_ptr;
                ret_ptr->prv = prv_ptr;
            }/* Per ogni token della marcatura di pl */
        }/* C'e' almeno un token */
    }/* Posto colorato */
ok: return;
}/* End get_to_first */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void flush_set_token_marking(int  delta,  int  pl,  int (*op)(),  int  inc_flag,  TokInfo_p  res_ptr) {
    /* Init flush_set_token_marking */
    int old_molt , new_molt;
    int status = 0;

    Token_p tok_ptr = NULL;


    if (op == NULL)
        op = plus_op;
    tok_ptr = GET_TOKEN_POSITION(res_ptr);
    old_molt = GET_OLD_MOLTEPLICITY(tok_ptr);
    new_molt = (*op)(old_molt, delta);
    if (new_molt < 0) {
        /* Errore */
        out_error(ERROR_NEGATIVE_MARKING, pl, 0, 0, 0, NULL, NULL);
    }/* Errore */
    else {
        /* OK ! */
        net_mark[pl].total = (*op)(net_mark[pl].total, delta);
        status = (old_molt > 0) << 1;
        status |= (new_molt == 0);
        if (IS_NEUTRAL(pl)) {
            /* Posto neutro */
            SET_TOKEN_MOLTEPLICITY(new_molt, tok_ptr);
            switch (status) {
            /* Vari casi da analizzare */
            case OLD_GT_NEW_EQ :
                net_mark[pl].different = 0;
                break;
            case OLD_EQ_NEW_GT :
                net_mark[pl].different = 1;
                break;
            }/* Vari casi da analizzare */
        }/* Posto neutro */
        else {
            /* Posto colorato */
            Token_p add_ptr = NULL;

            switch (status) {
            /* Vari casi da analizzare */
            case OLD_GT_NEW_EQ :
                (net_mark[pl].different)--;
                delete_token(res_ptr, pl);
                if (!inc_flag)
                    push_token(tok_ptr, pl);
                else
                    insert_in_reset(tok_ptr, pl);
                break;
            case OLD_EQ_NEW_EQ :
                break;
            }/* Vari casi da analizzare */
        }/* Posto colorato */
    }/* OK ! */
}/* End flush_set_token_marking */
#endif
