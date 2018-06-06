#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/service.h"

#ifdef REACHABILITY
#if !defined(MDD)
extern int belong();
#endif

#ifdef SWN
extern void push_token();
extern void push_token_list();
extern void optimized_get_pointer_info();
extern void insert_token();
extern Token_p pop_token();

#ifdef SYMBOLIC
extern Canonic_p sfl_h;
extern Cart_p token;
extern Cart_p dyn;
#endif
#endif

#define MAX_DIGIT 11

char cache_string[MAX_CACHE];
char *lp = NULL;


char *get_cache_string() {
    return cache_string;
}

#if defined MDD
int get_pl_in(char  pl_name[MAX_TAG_SIZE]) {
    /* Init get_pl_in */
    int i, ret = UNKNOWN;
    for (i = 0 ; i < npl ; i++)
        if (!strcmp(pl_name, tabp[i].place_name)) {
            ret = i;
            break;
        }
    return (ret);
}/* End get_pl_in */


int belong(int  pl,  Node_p  list) {
    /* Init belong */
    Node_p p = NULL;

    for (p = list; p != NULL ; p = NEXT_NODE(p))
        if (p->place_no == pl)
            return (TRUE);
    return (FALSE);
}/* End belong */
#endif

void char_store_compact(unsigned long  nval) {
    /* Init char_store_compact */
    register unsigned char cc;

    int space_left = MAX_CACHE - (int)(lp - cache_string)  - 1;

    if (nval < 64) {
        if (space_left < 1) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0x3F & nval);
        *lp = cc;
        lp++;
    }
    else if (nval < 16384) {
        if (space_left < 2) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0x40 | ((0x3F00 & nval) >> 8));
        *lp = cc;
        lp++;
        cc = (unsigned char)(0xFF & nval);
        *lp = cc;
        lp++;
    }
    else if (nval < 4194304) {
        if (space_left < 3) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0x80 | ((0x3F0000 & nval) >> 16));
        *lp = cc;
        lp++;
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        *lp = cc;
        lp++;
        cc = (unsigned char)(0xFF & nval);
        *lp = cc;
        lp++;
    }
    else if (nval < 1073741824) {
        if (space_left < 4) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0xC0 | ((0x3F000000 & nval) >> 24));
        *lp = cc;
        lp++;
        cc = (unsigned char)((0xFF0000 & nval) >> 16);
        *lp = cc;
        lp++;
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        *lp = cc;
        lp++;
        cc = (unsigned char)(0xFF & nval);
        *lp = cc;
        lp++;
    }
    else {
        fprintf(stderr, "char_store_compact error: value overflow\n");
        exit(1);
    }
}/* End char_store_compact */

void char_load_compact_int(int   *pp) {
    /* Init char_load_compact */
    register unsigned char cc0 = 0 ;
    register unsigned long uu = (cc0 & 0x3F);
    register unsigned long ii = (cc0 & 0xC0) >> 6;

    cc0 = *lp; lp++;
    uu = (cc0 & 0x3F);
    ii = (cc0 & 0xC0) >> 6;

    for (; ii ; ii--) {
        cc0 = *lp;
        lp++;
        uu = uu << 8;
        uu = uu + cc0;
    }
    *pp = uu;
}/* End char_load_compact */

void char_load_compact_long(unsigned long   *pp) {
    /* Init char_load_compact */
    register unsigned char cc0 = 0  ;
    register unsigned long uu = (cc0 & 0x3F);
    register unsigned long ii = (cc0 & 0xC0) >> 6;

    cc0 = *lp; lp++;
    uu = (cc0 & 0x3F);
    ii = (cc0 & 0xC0) >> 6;

    for (; ii ; ii--) {
        cc0 = *lp;
        lp++;
        uu = uu << 8;
        uu = uu + cc0;
    }
    *pp = uu;
}/* End char_load_compact */



#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_ordinary_tokens(tk_p, pl)
Token_p tk_p;
int pl;
{
    /* Init get_ordinary_tokens */
    int ret_value = 1;
    int ii = GET_PLACE_COMPONENTS(pl);
    int cl, ss, ds;

    for (; ii ; ii--) {
        /* Per ogni componente del dominio del posto */
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            /* Classe non ordinata */
            ds = tk_p->id[ii - 1];
            ss = get_static_subclass(cl, ds);
            ret_value *= GET_CARD(cl, ss, ds);
        }/* Classe non ordinata */
    }/* Per ogni componente del dominio del posto */
    return (ret_value);
}/* End get_ordinary_tokens */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
#ifdef SWN
#ifdef SYMBOLIC
void reset_to_M0(unsigned long mark_ptr, unsigned long dyn_ptr, unsigned long ilength, int tr)
#endif
#ifdef COLOURED
void reset_to_M0(unsigned long mark_ptr, unsigned long ilength, int tr)
#endif
{
    /* Init reset_to_M0 */

    register int pl, k;
    int comp, tok;
    unsigned long tk_m;
    int new_molt = 0;
    int element_id;
#ifdef SYMBOLIC
    int ordinary_tokens;
    int cl, sb, ds;
    int nds, crd;
    struct TOKEN_INFO info;
    TokInfo_p info_ptr = &info;
#endif
    Token_p tk_p = NULL;
    Token_p ntk_p = NULL;
    Token_p last_ptr = NULL;

    for (pl = 0; pl < npl; pl++) {
        /* Per ogni posto */
        if (!belong(pl, GET_OUTPUT_LIST(tr)))
            push_token_list(pl);
    }/* Per ogni posto */


    fseek(mark, mark_ptr, 0);
    size_t n_reads = fread(cache_string, 1, ilength, mark);
#ifdef SYMBOLIC
    lp = cache_string + (dyn_ptr - mark_ptr);
#endif

#ifdef SYMBOLIC
    for (cl = 0; cl < ncl; cl++) {
        /* Per ogni classe di colore */
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            /* Classe non ordinata */
            RESET_NUM_CL(cl);
            for (sb = 0; sb < GET_STATIC_SUBCLASS(cl); sb++) {
                /* Per ogni sottoclasse statica */
                RESET_NUM_SS(cl, sb);
                char_load_compact_int(&nds);
                for (ds = 0; ds < nds; ds++) {
                    /* Per ogni sottoclasse dinamica */
                    INCREASE_NUM_SS(cl, sb);
                    INCREASE_NUM_CL(cl);
                    char_load_compact_int(&crd);
                    SET_CARD(cl, sb, ds, crd);
                }/* Per ogni sottoclasse dinamica */
            }/* Per ogni sottoclasse statica */
        }/* Classe non ordinata */
    }/* Per ogni classe di colore */
#endif
    lp = cache_string;
    for (pl = 0; pl < npl; pl++) {
        /* Per ogni posto */
        last_ptr = NULL;
        comp = tabp[pl].comp_num;
        char_load_compact_int(&new_molt);
        if (IS_NEUTRAL(pl)) {
            /* Posto neutro */
            if (!belong(pl, GET_OUTPUT_LIST(tr))) {
                SET_TOKEN_MOLTEPLICITY(new_molt, net_mark[pl].marking);
                net_mark[pl].total = new_molt;
                net_mark[pl].different = 1;
#ifdef SYMBOLIC
                net_mark[pl].ordinary = 1;
#endif
            }
        }/* Posto neutro */
        else {
            /* Posto colorato */
            net_mark[pl].different = new_molt;

            for (tok = 1; tok <= new_molt; tok++) {
                /* Per ogni token */
                tk_p = pop_token(pl);
                char_load_compact_int(&(tk_p->molt));
                for (k = 0; k < comp ; k++)
                    char_load_compact_int(&(tk_p->id[k]));
#ifdef COLOURED
                net_mark[pl].total += tk_p->molt;
                if (net_mark[pl].marking == NULL) {
                    /* Inserimento in testa */
                    tk_p->next = net_mark[pl].marking;
                    net_mark[pl].marking = net_mark[pl].marking_t = tk_p;
                }/* Inserimento in testa */
                else {
                    /* Inserimento al fondo */
                    last_ptr->next = tk_p;
                    tk_p->next = NULL;
                    net_mark[pl].marking_t = tk_p;
                }/* Inserimento al fondo */
                last_ptr = tk_p;
#endif
#ifdef SYMBOLIC
                ordinary_tokens = get_ordinary_tokens(tk_p, pl);
                net_mark[pl].total += (tk_p->molt * ordinary_tokens);
                net_mark[pl].ordinary += ordinary_tokens;
                optimized_get_pointer_info(tk_p->id, pl, info_ptr);
                insert_token(info_ptr, tk_p, pl);
#endif
            }/* Posto colorato */
        }/* Informazioni su di un posto */
    }/* Per ogni posto */
}/* End reset_to_M0 */
#endif
#ifdef GSPN
void reset_to_M0() {
    /* Init reset_to_M0 */
    /** Ricordarsi di mirror_total **/
}/* End reset_to_M0 */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void string_to_marking(unsigned long  mark_ptr,  unsigned long  dyn_ptr,  unsigned long  ilength) {
    /* Init string_to_marking */

    register int pl, k;
    int comp, tok;
    unsigned long tk_m;
    int new_molt = 0;
    int element_id;
#ifdef SWN
#ifdef SYMBOLIC
    int ordinary_tokens;
    int cl, sb, ds;
    int nds, crd;
    struct TOKEN_INFO info;
    TokInfo_p info_ptr = &info;
#endif
    Token_p tk_p = NULL;
    Token_p ntk_p = NULL;
    Token_p last_ptr = NULL;

    for (pl = 0; pl < npl; pl++) {
        /* Per ogni posto */
        push_token_list(pl);
    }/* Per ogni posto */
#endif

    if (!SPEC_TRAI) {
        fseek(mark, mark_ptr, 0);
        size_t n_reads = fread(cache_string, 1, ilength, mark);
        lp = cache_string + (dyn_ptr - mark_ptr);
    }
    else { lp = cache_string + dyn_ptr; }
#ifdef SWN
#ifdef SYMBOLIC
    for (cl = 0; cl < ncl; cl++) {
        /* Per ogni classe di colore */
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            /* Classe non ordinata */
            RESET_NUM_CL(cl);
            for (sb = 0; sb < GET_STATIC_SUBCLASS(cl); sb++) {
                /* Per ogni sottoclasse statica */
                RESET_NUM_SS(cl, sb);
                char_load_compact_int(&nds);
                for (ds = 0; ds < nds; ds++) {
                    /* Per ogni sottoclasse dinamica */
                    INCREASE_NUM_SS(cl, sb);
                    INCREASE_NUM_CL(cl);
                    char_load_compact_int(&crd);
                    SET_CARD(cl, sb, ds, crd);
                }/* Per ogni sottoclasse dinamica */
            }/* Per ogni sottoclasse statica */
        }/* Classe non ordinata */
    }/* Per ogni classe di colore */
#endif
#endif
    lp = cache_string;
    for (pl = 0; pl < npl; pl++) {
        /* Per ogni posto */
#ifdef SWN
        last_ptr = NULL;
        comp = tabp[pl].comp_num;
#endif
        char_load_compact_int(&new_molt);
#ifdef GSPN
        net_mark[pl].total = new_molt;
        net_mark[pl].mirror_total = new_molt;
#endif
#ifdef SWN
        if (IS_NEUTRAL(pl)) {
            /* Posto neutro */
            SET_TOKEN_MOLTEPLICITY(new_molt, net_mark[pl].marking);
            net_mark[pl].total = new_molt;
            net_mark[pl].different = 1;
#ifdef SYMBOLIC
            net_mark[pl].ordinary = 1;
#endif
        }/* Posto neutro */
        else {
            /* Posto colorato */
            net_mark[pl].different = new_molt;

            for (tok = 1; tok <= new_molt; tok++) {
                /* Per ogni token */
                tk_p = pop_token(pl);
                char_load_compact_int(&(tk_p->molt));
                // printf("->\t%d: ",tk_p->molt);
                for (k = 0; k < comp ; k++) {
                    char_load_compact_int(&(tk_p->id[k]));
                    //printf("%d ",tk_p->id[k]);
                }
                //printf("\n");
                fflush(stdout);
#ifdef COLOURED
                net_mark[pl].total += tk_p->molt;
                if (net_mark[pl].marking == NULL) {
                    /* Inserimento in testa */
                    tk_p->next = net_mark[pl].marking;
                    net_mark[pl].marking = net_mark[pl].marking_t = tk_p;
                }/* Inserimento in testa */
                else {
                    /* Inserimento al fondo */
                    last_ptr->next = tk_p;
                    tk_p->next = NULL;
                    net_mark[pl].marking_t = tk_p;
                }/* Inserimento al fondo */
                last_ptr = tk_p;
#endif
#ifdef SYMBOLIC
                ordinary_tokens = get_ordinary_tokens(tk_p, pl);
                net_mark[pl].total += (tk_p->molt * ordinary_tokens);
                net_mark[pl].ordinary += ordinary_tokens;
                optimized_get_pointer_info(tk_p->id, pl, info_ptr);
                insert_token(info_ptr, tk_p, pl);
#endif
            }/* Posto colorato */
        }/* Informazioni su di un posto */
#endif
    }/* Per ogni posto */
}/* End string_to_marking */




/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
unsigned long marking_to_string() {
    /* Init marking_to_string */
    register int i, k, tk;
#ifdef SYMBOLIC
    register int d;
#endif
#ifdef SYMBOLIC
    int molt, cl, offset;
#endif
    unsigned long old_ptr;
    unsigned long ret_val;

    COMP_TEST++;
    fseek(mark, 0, 2);
    old_ptr = ftell(mark);
    lp = cache_string;


    for (i = 0 ; i < npl ; i++) {
        /* Per ogni posto */
#ifdef GSPN
        char_store_compact(net_mark[i].total);
#endif
#ifdef SWN
        if (IS_NEUTRAL(i))
            char_store_compact(net_mark[i].total);
        else {
            /* Posto colorato */

            // char_store_compact(net_mark[i].different);
            //if(IS_FULL(i))

            if (net_mark[i].marking)
                char_store_compact(net_mark[i].different);
            else
                char_store_compact(0);

            if (net_mark[i].marking) {
                /* Posto pieno colorato */
#ifdef COLOURED
                Token_p tk_p;

                for (tk_p = net_mark[i].marking; tk_p != NULL; tk_p = NEXT_TOKEN(tk_p)) {
                    /* Per ogni token della marcatura */
                    char_store_compact(tk_p->molt);
                    for (k = 0 ; k < tabp[i].comp_num; k++)
                        char_store_compact(tk_p->id[k]);
                }/* Per ogni token della marcatura */
#endif
#ifdef SYMBOLIC
                for (k = 0 ; k < tabp[i].comp_num ; k++) {
                    dyn->low[k] = token->low[k] = token->up[k] = 0;
                    cl = tabp[i].dominio[k];
                    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                        dyn->up[k] = GET_NUM_CL(cl) - 1;
                    else
                        dyn->up[k] = tabc[cl].card - 1;
                }
                do {
                    for (k = 0 ; k < tabp[i].comp_num ; k++) {
                        cl = tabp[i].dominio[k];
                        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                            token->mark[k][0] = sfl_h->min[cl][dyn->low[k]];
                        else
                            token->mark[k][0] = dyn->low[k];
                    }
                    molt = get_marking_of(token, i, TRUE);
                    if (molt) {
                        char_store_compact(molt);
                        // printf("%d: ",molt);
                        for (k = 0 ; k < tabp[i].comp_num; k++) {
                            cl = tabp[i].dominio[k];
                            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                                tk = get_dynamic_from_total(cl, dyn->low[k]);
                            else
                                tk = dyn->low[k];

                            char_store_compact(tk);
                            //printf("%d ",tk);
                        }
                        //printf("\n");
                    }
                }
                while (nextvec(dyn, tabp[i].comp_num));
#endif
            }/* Posto pieno colorato */
        }/* Posto colorato */
#endif
    }/* Per ogni posto */
#ifdef SWN
#ifdef SYMBOLIC
    d_ptr = lp - cache_string;
    for (i = 0 ; i < ncl; i++)
        if (IS_UNORDERED(i) || (IS_ORDERED(i) && GET_STATIC_SUBCLASS(i) == 1)) {
            /* Classe non ordinata */
            for (k = 0 , offset = 0; k < GET_STATIC_SUBCLASS(i); k++) {
                /* Per ogni sottoclasse statica */
                char_store_compact(GET_NUM_SS(i, k));
                for (d = 0; d < GET_NUM_SS(i, k); d++, offset++) {
                    /* Per ogni sottoclasse dinamica */
                    char_store_compact(GET_CARD(i, k, sfl_h->min[i][offset]));
                }/* Per ogni sottoclasse dinamica */
            }/* Per ogni sottoclasse statica */
        }/* Classe non ordinata */
#endif
#endif
    length = lp - cache_string;
    return (old_ptr);
}/* End marking_to_string */
#endif
