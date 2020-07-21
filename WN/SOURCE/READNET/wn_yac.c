# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <string.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/macros.h"

extern Pred_p prd_h;	/* Variabili usate nel parser WN.YAC */
extern Pred_p prd_t;	/*	  "                    "	  */
extern Expr_p pcode_h;	/* Variabili usate nel parser WN.YAC */
extern Expr_p pcode_t;	/*	  "                    "	  */
#ifdef SYMBOLIC
extern int optimized_get_number_of_ordinary_tokens();
#endif

extern void out_error();
extern void delete_token();
extern void push_token();
extern void insert_in_reset();
extern void insert_in_touched();
extern void insert_token();
extern void *ecalloc();
extern char *emalloc();


extern int plus_op();
extern int get_pl_in();
extern void optimized_get_pointer_info();
extern void optimized_init_token_values();
#ifdef SWN
extern Token_p pop_token();
#endif
extern int parse_DEF;

static Expr_p exp_free = NULL;
static int exp_pop = 0;
static int exp_mall = 0;

static Pred_p pred_free = NULL;
static int pred_pop = 0;
static int pred_mall = 0;

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void optimized_set_token_marking(cartes_ptr , delta , pl , op, inc_flag)
Cart_p cartes_ptr;
int delta;
int pl;
int (*op)();
int inc_flag;
{
    /* Init optimized_set_token_marking */

#ifdef GSPN
    if (op == NULL)
        op = plus_op;
    net_mark[pl].total = (*op)(net_mark[pl].total, delta);
    if (net_mark[pl].total < 0)
        out_error(ERROR_NEGATIVE_INITIAL_MARKING, pl, 0, 0, 0, NULL, NULL);
    net_mark[pl].mirror_total = (*op)(net_mark[pl].mirror_total, delta);
#endif
#ifdef SWN
    int old_molt , new_molt;
    int status = 0;

    Token_p tok_ptr = NULL;

    struct TOKEN_INFO result;
    TokInfo_p res_ptr = &result;

    if (op == NULL)
        op = plus_op;
    optimized_get_pointer_info(cartes_ptr->low , pl , res_ptr);
    tok_ptr = GET_TOKEN_POSITION(res_ptr);
    old_molt = GET_OLD_MOLTEPLICITY(tok_ptr);
    new_molt = (*op)(old_molt, delta);
    if (new_molt < 0)
        out_error(ERROR_NEGATIVE_INITIAL_MARKING, pl, 0, 0, 0, NULL, NULL);
    else {
        /* OK ! */
#ifdef SYMBOLIC
        int ordinary_tokens = optimized_get_number_of_ordinary_tokens(cartes_ptr, pl);

        net_mark[pl].total = (*op)(net_mark[pl].total , delta * ordinary_tokens);
#endif
#ifdef COLOURED
        net_mark[pl].total = (*op)(net_mark[pl].total, delta);
#endif
        status = (old_molt > 0) << 1;
        status |= (new_molt == 0);
        if (IS_NEUTRAL(pl)) {
            /* Posto neutro */
            SET_TOKEN_MOLTEPLICITY(new_molt, tok_ptr);
            switch (status) {
            /* Vari casi da analizzare */
            case OLD_GT_NEW_EQ :
                net_mark[pl].different = 0;
#ifdef SYMBOLIC
                net_mark[pl].ordinary = 0;
#endif
                break;
            case OLD_EQ_NEW_GT :
                net_mark[pl].different = 1;
#ifdef SYMBOLIC
                net_mark[pl].ordinary = 1;
#endif
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
#ifdef SYMBOLIC
                net_mark[pl].ordinary -= ordinary_tokens;
#endif
                delete_token(res_ptr, pl);
#ifdef SIMULATION
                if (!inc_flag)
                    push_token(tok_ptr, pl);
                else
                    insert_in_reset(tok_ptr, pl);
#endif
#ifdef REACHABILITY
                push_token(tok_ptr, pl);
#endif
                break;
            case OLD_GT_NEW_GT :
                SET_TOKEN_MOLTEPLICITY(new_molt, tok_ptr);
#ifdef SIMULATION
                if (inc_flag)
                    insert_in_touched(tok_ptr, pl);
#endif
                break;
            case OLD_EQ_NEW_GT :
                (net_mark[pl].different)++;
#ifdef SYMBOLIC
                net_mark[pl].ordinary += ordinary_tokens;
#endif
                add_ptr = pop_token(pl);
                optimized_init_token_values(cartes_ptr->low, add_ptr, pl);
                SET_TOKEN_MOLTEPLICITY(new_molt, add_ptr);
                insert_token(res_ptr, add_ptr, pl);
#ifdef SIMULATION
                if (inc_flag)
                    insert_in_touched(add_ptr, pl);
#endif
                break;
            case OLD_EQ_NEW_EQ :
                break;
            }/* Vari casi da analizzare */
        }/* Posto colorato */
    }/* OK ! */
#endif
}/* End optimized_set_token_marking */
#ifdef SWN
/*****************************************************************
nome	       : fill_with_elements
descrizione    :
parametri      :
valori ritorno :
******************************************************************/
int fill_with_elements(pos, cl)
int pos;
int cl;
{
    /* Init fill_with_elements */
    int ret_value;

#ifdef SYMBOLIC
#ifdef REACHABILITY
    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
#endif
            ret_value = GET_NUM_CL(cl) - 1;
        else
#endif
            ret_value = tabc[cl].card - 1;
    return (ret_value);
}/* End fill_with_elements */
/*****************************************************************
nome	       : set_number_of_elements
descrizione    :
parametri      :
valori ritorno :
******************************************************************/
int set_number_of_elements(pos, cl, sbc)
int pos;
int cl;
int sbc;
{
    /* Init set_number_of_elements */

#ifdef SYMBOLIC
#ifdef REACHABILITY
    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
#endif
            return (cart_place_ptr->low[pos] + GET_NUM_SS(cl, sbc) - 1);
        else
#endif
            return (cart_place_ptr->low[pos] + GET_STATIC_CARDINALITY(cl, sbc) - 1);
}/* End set_number_of_elements */
/*****************************************************************
nome	       : get_obj_id
descrizione    :
parametri      :
valori ritorno :
******************************************************************/
int get_obj_id(name, cl)
char *name;
int cl;
{
    int num, i, j, k, h, found = FALSE, off = 0, isol = FALSE;
    char comp[MAX_TAG_SIZE], number[MAX_TAG_SIZE];
    struct OBJ *opt;
#ifdef SYMBOLIC
    struct DYNSC *ptr = NULL;
#endif

#ifdef SYMBOLIC
#ifdef REACHABILITY
    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
#endif
#ifdef SIMULATION
        if (IS_UNORDERED(cl))
#endif
        {
            /* Classe non ordinata */
            for (j = 0; j < tabc[cl].sbc_num; j++)
                for (ptr = tabc[cl].sbclist[j].dynlist; ptr != NULL; ptr = ptr->next)
                    if (!strcmp(name, ptr->name))
                        return (ptr->map + GET_STATIC_OFFSET(cl, j));
            return (UNKNOWN);
        }/* Classe non ordinata */
        else
#endif
        {
            for (k = 0; name[k] != '\0' && !isol; k++) {
                if (isalpha(name[k]))
                    comp[k] = name[k];
                else {
                    isol = TRUE;
                    comp[k] = '\0';
                }
            }
            if (isol) {
                for (h = k - 1; name[h] != '\0'; h++)
                    number[h - (k - 1)] = name[h];
                number[h - k + 1] = '\0';
                num = atoi(number);
            }
            for (i = 0; i < tabc[cl].sbc_num && !found; i++) {
                if ((opt = tabc[cl].sbclist[i].obj_list) == NULL) {
                    if (strcmp(comp, tabc[cl].sbclist[i].obj_name))
                    {}
                    else if (num >= tabc[cl].sbclist[i].low && num < tabc[cl].sbclist[i].low + tabc[cl].sbclist[i].card)
                        return (off + num - tabc[cl].sbclist[i].low);
                }
                else {
                    for (j = 0; opt != NULL && strcmp(name, opt->nome); opt = opt->next, j++);
                    if (opt != NULL)
                        return (off + j);
                }
                if (!found)
                    off += tabc[cl].sbclist[i].card;
            }
            if (!found)
                return (UNKNOWN);
        }
    return (UNKNOWN);
}
/*****************************************************************
nome	       : get_index
descrizione    : cerca nella tabella delle classi di colore il nome
		 con cui viene chiamata e ne restituisce l'offset
		 all'interno della tabella.
parametri      : name
valori ritorno : index
******************************************************************/
int get_index(name)
char *name ;
{
    int index;

    for (index = 0; index < ncl && strcmp(name, tabc[index].col_name); index++);
    if (index == ncl) index = -1 ;
    return (index) ;
}
/*****************************************************************
nome	       : already_exist
descrizione    :
parametri      :
valori ritorno :
******************************************************************/
int already_exist(name, col, type)
char *name;
int col;
int type;
{
    int i = 0, j, ok = TRUE;
    struct DYNSC *ptr;

    switch (type) {
    case STAT : {
        for (i = 0; i < tabc[col].sbc_num && strcmp(name, tabc[col].sbclist[i].name); i++);
        if (i == tabc[col].sbc_num) i = -1 ;
        break;
    }
    case DYN : {
        for (j = 0; j < tabc[col].sbc_num && ok; j++)
            for (ptr = tabc[col].sbclist[j].dynlist; ptr != NULL; ptr = ptr->next) {
                if (!strcmp(name, ptr->name)) {
                    ok = FALSE;
                    i = ptr->map;
                    break;
                }
            }
        if (ok)
            i = -1;
        break;
    }
    default : {
        for (ptr = tabc[col].sbclist[type].dynlist; ptr != NULL; ptr = ptr->next)
            if (!strcmp(name, ptr->name))
                break;
        if (ptr == NULL)
            i = -1;
        break;
    }
    }
    return (i + 1);
}
/*****************************************************************
nome	       : get_class_card
descrizione    : calcola la cardinalita'della classe di colori di
		 indice class_ind nella tabella dei colori
parametri      : class_ind
valori ritorno : class_card
******************************************************************/
int get_class_card(class_ind)
int class_ind;

{
    int class_card = 0 ;
    int index ;

    for (index = 0; index < tabc[class_ind].sbc_num; index++)
        class_card += tabc[class_ind].sbclist[index].card;
    return (class_card);
}
/*****************************************************************
nome	       : insert_buf
descrizione    : inserisce l'elemento puntato da node in testa alla
		 lista di descrizioni di sottoclassi da completare
parametri      : node
valori ritorno : modifica il puntatore al primo elemento della lista
******************************************************************/
void insert_buf(node)
struct SBCBUF *node;

{
    node->next = buf_list;
    buf_list = node;
}
/*****************************************************************
nome	       : already_transformed
descrizione    :
parametri      :
valori ritorno :
******************************************************************/
int already_transformed(fun_name, cl_name)
char *fun_name ;
int cl_name;
{
    int index;
    int tr;

    tr = (parse_DEF == MDRATE) ? nmd : ntr;
    for (index = 0; index < comp_check; index++) {
        if (occ[tr][index].cclass == cl_name)
            if (strcmp(occ[tr][index].p_fun_name, fun_name) == 0)
                return (TRUE);
    }
    return (FALSE);
}
/*****************************************************************
nome	       : get_proj_num
descrizione    : Associa al nome della funzione l'occorrenza della
		 classe a cui si riferisce
parametri      : nome funzione
valori ritorno : occorrenza corrispondente
******************************************************************/
int get_proj_num(fun_name, cl_name)
char *fun_name ;
int cl_name;
{
    int index;
    int proj = 0;
    int max_occ = 0;
    int tr;

    tr = (parse_DEF == MDRATE) ? nmd : ntr;

    for (index = 0; index < comp_check; index++) {
        if ((occ[tr][index].cclass != cl_name) && (strcmp(occ[tr][index].p_fun_name, fun_name) == 0)) {
            if (strcmp(fun_name, "?") != 0) {
                proj = ERROR_SHARED_NAME;
                break;
            }
        }
        else if ((occ[tr][index].cclass == cl_name) && (strcmp(occ[tr][index].p_fun_name, fun_name) == 0)) {
            proj = occ[tr][index].occ_num;
            break;
        }
        else if ((occ[tr][index].cclass == cl_name) && (strcmp(occ[tr][index].p_fun_name, fun_name) != 0)) {
            if (strcmp(fun_name, "?") != 0 && strcmp(occ[tr][index].p_fun_name, "?") == 0) {
                proj = ERROR_AMBIGUITY_IN_ARC;
                break;
            }
            else max_occ = occ[tr][index].occ_num;
        }
    }
    if (proj == 0) {
        if (comp_check == MAX_DOMAIN)
            out_error(ERROR_LOW_MAX_DOMAIN, 0, ntr, 0, 0, NULL, NULL);
        occ[tr][comp_check].cclass = cl_name;
        // strcpy(occ[tr][comp_check].fun_name, fun_name);
        occ[tr][comp_check].p_fun_name = strdup(fun_name);
        occ[tr][comp_check].occ_num = max_occ + 1;
        proj = max_occ + 1;
        comp_check++;
    }
    return (proj);
}
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_predicate(head, tail)
Pred_p head;
Pred_p tail;
{
    /* Init push_predicate */

    if (tail != NULL) {
        tail->next = pred_free;
        pred_free = head;
    }
}/* End push_predicate */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Pred_p pop_predicate() {
    /* Init pop_predicate */

    Pred_p ret_ptr = NULL;

#ifdef DEBUG_malloc
    pred_pop++;
#endif
    if (pred_free == NULL) {
        /* Lista libera e' vuota */
        ret_ptr = (Pred_p)emalloc(sizeof(struct PRED));
#ifdef DEBUG_malloc
        pred_mall++;
#endif
    }/* Lista libera e' vuota */
    else {
        ret_ptr = pred_free;
        pred_free = pred_free->next;
    }
    return (ret_ptr);

}/* End pop_predicate */
/*****************************************************************
nome	       : gen
descrizione    : Genera il codice intermedio per valutare i predicati
parametri      : Puntatore all'array contenente il codice,tipo di
		 operazione,stringa da memorizzare
valori ritorno : Nessuno
******************************************************************/
void gen(type, string)
int type;
char *string;
{
    /* Init gen */
    Pred_p pred_pt = NULL;

    pred_pt = pop_predicate();

#ifdef SWN
    pred_pt->occ = UNKNOWN;
    pred_pt->col_ind = UNKNOWN;
    pred_pt->fun_name = (char *)ecalloc(strlen(string) + 1, sizeof(char));
    strcpy(pred_pt->fun_name, string);
#endif
#ifdef GSPN
    pred_pt->expression = pcode_h;
#endif
    pred_pt->type = type;
    pred_pt->next = NULL;
    if (prd_h == NULL)
        prd_h = prd_t = pred_pt;
    else {
        prd_t->next = pred_pt;
        prd_t = pred_pt;
    }
}/* End gen */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
double get_rate_parameter_value(par_name)
char par_name[MAX_TAG_SIZE];
{
    /* Init get_rate_parameter_value */
    int i;
    double ret = -1.00000;

    for (i = 0 ; i < nrp ; i++)
        if (!strcmp(par_name, tabrp[i].rate_name)) {
            ret = tabrp[i].rate_val;
            break;
        }
    return (ret);
}/* End get_rate_parameter_value */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_marking_parameter_value(par_name)
char par_name[MAX_TAG_SIZE];
{
    /* Init get_marking_parameter_value */
    int i;
    int ret = UNKNOWN;

    for (i = 0 ; i < nmp ; i++)
        if (!strcmp(par_name, tabmp[i].mark_name)) {
            ret = tabmp[i].mark_val;
            break;
        }
    return (ret);
}/* End get_marking_parameter_value */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_marking_parameter_offset(par_name)
char par_name[MAX_TAG_SIZE];
{
    /* Init get_marking_parameter_offset */
    int i;
    int ret = UNKNOWN;

    for (i = 0 ; i < nmp ; i++)
        if (!strcmp(par_name, tabmp[i].mark_name)) {
            ret = i;
            break;
        }
    return (ret);
}/* End get_marking_parameter_offset */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_expression(head, tail)
Expr_p head;
Expr_p tail;
{
    /* Init push_expression */

    if (tail != NULL) {
        tail->next = exp_free;
        exp_free = head;
    }
}/* End push_expression */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Expr_p pop_expression() {
    /* Init pop_expression */

    Expr_p ret_ptr = NULL;

#ifdef DEBUG_malloc
    exp_pop++;
#endif
    if (exp_free == NULL) {
        /* Lista libera e' vuota */
        ret_ptr = (Expr_p)emalloc(sizeof(struct EXPR_DEF));
#ifdef DEBUG_malloc
        exp_mall++;
#endif
    }/* Lista libera e' vuota */
    else {
        ret_ptr = exp_free;
        exp_free = exp_free->next;
    }
    return (ret_ptr);

}/* End pop_expression */
/*****************************************************************
nome	       : emit
descrizione    : Genera il codice intermedio per valutare i ratemd
parametri      : Puntatore all'array contenente il codice,tipo di
		 operazione,stringa da memorizzare,valore
valori ritorno : Nessuno
******************************************************************/
void emit(type, string, val, ival)
int type;
char string[MAX_TAG_SIZE] ;
double val ;
int ival;
{
    Expr_p expr_pt = NULL;
    int irate;
    double rate;


    expr_pt = pop_expression();

    expr_pt->val = val;
    expr_pt->ival = ival;
    expr_pt->op_type = type;
    switch (type) {
    /* Code type */
    case TYPEPAR  : if ((rate = get_rate_parameter_value(string)) < 0.0) {
            if ((irate = get_marking_parameter_value(string)) < 0.0) {
                fprintf(stdout, "Error: unknown parameter %s in expression\n", string);
                exit(-1);
            }
            else {
                expr_pt->ival = irate;
                expr_pt->val = irate;
            }
        }
        else if (parse_DEF != MDRATE) {
            fprintf(stdout, "Error: floating in expression\n");
            exit(1);
        }
        else
            expr_pt->val = rate;
        break;
    case TYPEMARK : expr_pt->ival = get_pl_in(string);
        break;
    }/* Code type */
    /*expr_pt->name = (char *)ecalloc(strlen(string)+1,sizeof(char));
    strcpy(expr_pt->name,string);*/
    expr_pt->next = NULL;
    if (pcode_h == NULL)
        pcode_h = pcode_t = expr_pt;
    else {
        pcode_t->next = expr_pt;
        pcode_t = expr_pt;
    }
}
#ifdef SWN
/*****************************************************************
nome	       : check_obj_name
descrizione    : Controlla che un oggetto non sia dichiarato due
		 volte nella stessa sottoclasse statica.
parametri      : name
valori ritorno : -1 se l'oggetto e' gia stato dichiarato
******************************************************************/
int check_obj_name(name, list)
char name[MAX_TAG_SIZE] ;
struct OBJ *list;
{
    struct OBJ *index ;

    for (index = list; index != NULL; index = index->next)
        if (strcmp(name, index->nome) == 0) break;
    if (index == NULL) return (0) ;
    else return (-1);
}
/*****************************************************************
nome	       : get_obj_raw_name
descrizione    :
parametri      :
valori ritorno :
******************************************************************/
void get_obj_raw_name(name, raw, number, num)
char *name;
char *raw;
char *number;
int *num;
{
    /* Init get_obj_raw_name */
    int h = 0, k, l;
    int length = strlen(name);

    for (k = length - 1, l = 0; k >= 0; k--)
        if (isdigit(name[k])) {
            number[k] = name[k];
            l++;
        }
        else {
            h = k;
            break;
        }
    for (k = 0; k < l; k++)
        number[k] = name[h + k + 1];
    number[k] = '\0';
    *num = atoi(number);
    for (k = 0; k <= h; k++)
        raw[k] = name[k];
    raw[k] = '\0';
}/* End get_obj_raw_name */
/*****************************************************************
nome	       : intersection
descrizione    : Controlla che l'intersezione della sottoclasse in
		 esame sia disgiunta da quelle gia' dichiarate per
		 quella classe di colore
parametri      : puntatore all'array che contiene le descrizioni delle
		 sottoclassi,offset della sottoclasse da analizzare,
		 numero sottoclassi.
valori ritorno : -1 se l'intersezione non e'l'insieme vuoto
******************************************************************/
int intersection(array, sbc, upperbound)
struct STATICSBC *array;
int sbc;
int upperbound;

{
    int index, min_ind, max_ind, res = 0;
    int sbccard, indcard, indlow, sbclow;
    struct OBJ *min_pun;
    char number[MAX_TAG_SIZE];
    char raw[MAX_TAG_SIZE];
    int obj_index;

    for (index = 0; index < upperbound; index++) {
        if (index != sbc) {
            /* Skip se stessa */
            if (array[index].card != 0) {
                /* Gia'dichiarata */
                if ((array[index].obj_list == NULL) && (array[sbc].obj_list == NULL)) {
                    /* Short intersection */
                    if (strcmp(array[index].obj_name, array[sbc].obj_name) == 0) {
                        sbccard = array[sbc].card;
                        indcard = array[index].card;
                        sbclow = array[sbc].low;
                        indlow = array[index].low;
                        if (!((sbclow >= indlow + indcard) || (sbclow + sbccard <= indlow))) {
                            res = -index - 1;
                            break;
                        }
                    }
                }/* Short intersection */
                else if ((array[index].obj_list != NULL) && (array[sbc].obj_list != NULL)) {
                    /* Long intersection */
                    min_ind = array[index].card <= array[sbc].card ? index : sbc ;
                    max_ind = array[index].card > array[sbc].card ? index : sbc ;
                    for (min_pun = array[min_ind].obj_list; min_pun != NULL; min_pun = min_pun->next)
                        if (check_obj_name(min_pun->nome, array[max_ind].obj_list) == -1) {
                            res = -index - 1;
                            index = upperbound;
                            break;
                        }
                }/* Long intersection */
                else if ((array[index].obj_list == NULL) && (array[sbc].obj_list != NULL)) {
                    /* Mixed intersection */
                    for (min_pun = array[sbc].obj_list; min_pun != NULL; min_pun = min_pun->next) {
                        get_obj_raw_name(min_pun->nome, raw, number, &obj_index);
                        if (!strcmp(raw, array[index].obj_name)) {
                            /* Stesso nome */
                            if (obj_index >= array[index].low && obj_index < array[index].low + array[index].card) {
                                /* Stesso nome e indice uguale */
                                res = -index - 1;
                                index = upperbound;
                                break;
                            }/* Stesso nome e indice uguale */
                        }/* Stesso nome */
                    }
                }/* Mixed intersection */
                else {
                    /* Mixed intersection */
                    for (min_pun = array[index].obj_list; min_pun != NULL; min_pun = min_pun->next) {
                        get_obj_raw_name(min_pun->nome, raw, number, &obj_index);
                        if (!strcmp(raw, array[sbc].obj_name)) {
                            /* Stesso nome */
                            if (obj_index >= array[sbc].low && obj_index < array[sbc].low + array[sbc].card) {
                                /* Stesso nome e indice uguale */
                                res = -index - 1;
                                index = upperbound;
                                break;
                            }/* Stesso nome e indice uguale */
                        }/* Stesso nome */
                    }
                }/* Mixed intersection */
            }/* Gia'dichiarata */
        }/* Skip se stessa */
    }/* Fine loop */
    return (res);
}
#endif
/*****************************************************************
nome	       : emit
descrizione    : Genera il codice intermedio per valutare i ratemd
parametri      : Puntatore all'array contenente il codice,tipo di
		 operazione,stringa da memorizzare,valore
valori ritorno : Nessuno
******************************************************************/
struct MD_RATE *get_md_rate(tr_ind)
int tr_ind;
{
    int ind ;

    for (ind = 0; ((ntr + 1) != tabmd[ind].trans_ind) && (ind < nmd); ind++);
    if (ind == nmd)
        out_error(ERROR_MISSING_MDRATE_DEFINITION, 0, ntr, 0, 0, NULL, NULL);
    return (tabmd[ind].mdpun);
}
