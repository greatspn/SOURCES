# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/macros.h"

#ifdef SIMULATION
extern int is_input_preselectable();
#ifdef SYMBOLIC
extern void fill_split_type_field();
#endif
#endif
extern void *ecalloc();
extern void out_error();
extern void fill_node_data_structures();
extern void fill_split_type_field();
extern int already_exist();
extern int get_pl_in();

extern void successor_or_predecessor();
extern int is_in_function();

#define ERROR_UNKNOWN_FUNCTION -1
#define ERROR_DIFFERENT_CLASSES  -2
#define ERROR_WRONG_INDEX -3
#define ERROR_ILLEGAL_FUNCTION -4
#define ERROR_WRONG_OPERATOR -5

#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void init_domain(tr, dom)
int tr;
int dom[MAX_DOMAIN];
{
    /* Init init_domain */
    int i;

    for (i = 0; i < tabt[tr].comp_num; i++)
        dom[i] = FALSE;
}/* Init init_domain */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void cancel_domain(pun, tr, dom)
Node_p pun;
int tr;
int dom[MAX_DOMAIN];
{
    /* Init cancel_domain */
    int pl, i, cl, j;
    Node_p pt;
    struct ENN_COEFF *ptr;

    for (pt = pun; pt != NULL; pt = pt->next) {
        pl = pt->place_no;
        if (tabp[pl].comp_num && !pt->skip) {
            for (i = 0; i < tabp[pl].comp_num; i++) {
                cl = tabp[pl].dominio[i];
                for (j = 0; j < tabt[tr].rip[cl]; j++)
                    for (ptr = pt->arcfun; ptr != NULL; ptr = ptr->next)
                        if ((ptr->coef[i].xsucc_coef[3 * j] != 0
                                || ptr->coef[i].xsucc_coef[3 * j + 1] != 0
                                || ptr->coef[i].xsucc_coef[3 * j + 2] != 0)
                                && !dom[tabt[tr].off[cl] + j]) {
                            dom[tabt[tr].off[cl] + j] = TRUE;
                        }
            }
        }
    }

}/* Init cancel_domain */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int other_domain(tr, dom)
int tr;
int dom[MAX_DOMAIN];
{
    /* Init other_domain */
    int i, cl, j;
    int ret_value = FALSE;

    for (i = 0; i < tabt[tr].comp_num; i += tabt[tr].rip[cl]) {
        cl = tabt[tr].dominio[i];
        for (j = 0; j < tabt[tr].rip[cl]; j++)
            if (!dom[i + j]) {
                /* Componente di output */
                ret_value = TRUE;
                tabt[tr].comp_type[i + j] = OUT;
            }/* Componente di output */
            else
                tabt[tr].comp_type[i + j] = IN;
    }
    return (ret_value);
}/* Init other_domain */
/*****************************************************************
nome	       :
descrizione    :
parametri      :

valori ritorno :
******************************************************************/
void compute_domain(tr, dom)
int tr;
int dom[MAX_DOMAIN];
{
    /* Init compute_domain */

    init_domain(tr, dom);
    cancel_domain(tabt[tr].inibptr, tr, dom);
    cancel_domain(tabt[tr].inptr, tr, dom);
}/* End compute_domain */
/*****************************************************************
nome	       :
descrizione    :
parametri      :

valori ritorno :
******************************************************************/
static void fill_output_components(tr)
int tr;
{
    /* Init fill_output_components */
    int dom[MAX_DOMAIN];

    compute_domain(tr, dom);
    other_domain(tr, dom);
}/* End fill_output_components */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int all_projection_functions(tr)
int tr;
{
    /* Init all_projection_functions */
    int ret_value = TRUE;
    int pl;
    Node_p node_ptr = NULL;

    for (node_ptr = GET_INPUT_LIST(tr); node_ptr != NULL; node_ptr = NEXT_NODE(node_ptr)) {
        pl = GET_PLACE_INDEX(node_ptr);
        if (!(node_ptr->type == PROJECTION || IS_NEUTRAL(pl) || IS_INDEPENDENT(node_ptr))) {
            ret_value = FALSE;
            break;
        }
    }
    return (ret_value);
}/* End all_projection_functions */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int is_source(tr)
int tr;
{
    /* Init is_source */
    int ret_value = FALSE;

    if (GET_INPUT_LIST(tr) == NULL && GET_INHIBITOR_LIST(tr) == NULL)
        ret_value = TRUE;
    return (ret_value);
}/* End is_source */
#ifdef SWN
#ifdef SIMULATION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int same_domain(tr)
int tr;
{
    /* Init same_domain */
    int ret_value = TRUE;
    int skeleton[MAX_DOMAIN];
    int i, pl, cl, rp, pos_in_domain, max = -1, max_pl = UNKNOWN;
    Node_p node_ptr = NULL;
    Node_p max_node_ptr = NULL;

    for (node_ptr = GET_INPUT_LIST(tr); node_ptr != NULL; node_ptr = NEXT_NODE(node_ptr)) {
        /* Per ogni posto in ingresso */
        pl = GET_PLACE_INDEX(node_ptr);
        if (!(IS_NEUTRAL(pl) || IS_INDEPENDENT(node_ptr))) {
            /* Posto colorato e con funzione */
            if (GET_PLACE_COMPONENTS(pl) > max) {
                max = GET_PLACE_COMPONENTS(pl);
                max_pl = pl;
                max_node_ptr = node_ptr;
            }
        }/* Posto colorato e con funzione */
    }/* Per ogni posto in ingresso */
    if (max_pl != UNKNOWN) {
        for (i = GET_TRANSITION_COMPONENTS(tr); i ; skeleton[--i] = NEW);
        for (i = 0; i < GET_PLACE_COMPONENTS(max_pl);  i++) {
            /* Per ogni componente del dominio del posto */
            cl = GET_COLOR_COMPONENT(i, max_pl);
            for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++)
                if (is_in_function(rp, i, max_node_ptr))
                    break;
            pos_in_domain = GET_POSITION_OF(cl, rp, tr);
            skeleton[pos_in_domain] = OLD;
        }/* Per ogni componente del dominio del posto */
        for (node_ptr = GET_INPUT_LIST(tr); node_ptr != NULL; node_ptr = NEXT_NODE(node_ptr)) {
            /* Per ogni posto in ingresso */
            pl = GET_PLACE_INDEX(node_ptr);
            if (!(IS_NEUTRAL(pl) || IS_INDEPENDENT(node_ptr) || max_pl == pl)) {
                /* Posto colorato e con funzione */
                for (i = 0; i < GET_PLACE_COMPONENTS(pl);  i++) {
                    /* Per ogni componente del dominio del posto */
                    cl = GET_COLOR_COMPONENT(i, pl);
                    for (rp = 0; rp < GET_COLOR_REPETITIONS(cl, tr); rp++)
                        if (is_in_function(rp, i, node_ptr))
                            break;
                    pos_in_domain = GET_POSITION_OF(cl, rp, tr);
                    if (skeleton[pos_in_domain] == NEW) {
                        ret_value = FALSE;
                        goto ret;
                    }
                }/* Per ogni componente del dominio del posto */
            }/* Posto colorato e con funzione */
        }/* Per ogni posto in ingresso */
    }
ret: return (ret_value);
}/* End same_domain */
/**************************************************************/
/* NAME 	: is_skippable_after_firing		      */
/* DESCRIPTION	: Controlla se la transizione verifica le     */
/*		  condizione per fare un test parziale dopo   */
/*		  lo sparo di una sua istanza.		      */
/* PARAMETERS	: Indice della transizione in esame.	      */
/* RETURN VALUE : TRUE se la condizione e' verificata, FALSE  */
/*		  altrimenti.				      */
/**************************************************************/
static int is_skippable_after_firing(tr)
int tr;
{
    /* Init is_skippable_after_firing */
    int ret_value = FALSE;

    if (GET_TRANSITION_COMPONENTS(tr) == 0)
        ret_value = FALSE;
    else {
        /* Transizione colorata */
        if (GET_INHIBITOR_LIST(tr) == NULL)
            if (all_projection_functions(tr))
                if (!is_source(tr))
                    if (same_domain(tr))
                        ret_value = TRUE;
    }/* Transizione colorata */
    return (ret_value);
}/* End is_skippable_after_firing */
#endif
/***********************			     ***********************/
/* PROCEDURA get_transition_domain CHE ASSEGNA I VALORI AI CAMPI:	   */
/* dominio ---> array di interi contenente il dominio di una transizione;  */
/* comp_num --> numero di elementi del dominio; 			   */
/* rip -------> array di interi contenente il numero di ripetizioni di	   */
/*		ogni classe;						   */
/* off -------> array di interi con un offset per ogni classe all'interno  */
/*		del dominio.						   */
/***********************			     ***********************/
/*****************************************************************
nome	       :
descrizione    :
parametri      :

valori ritorno :
******************************************************************/
static void order_domain(dom, up, names)
int *dom;
int up;
char **names;
{
    /* Init order_domain */
    int i, buf, flag;
    char *name_buf = NULL;

    do {
        for (i = 0, flag = FALSE; i < up - 1; i++)
            if (dom[i] > dom[i + 1]) {
                buf = dom[i];
                dom[i] = dom[i + 1];
                dom[i + 1] = buf;
                name_buf = names[i];
                names[i] = names[i + 1];
                names[i + 1] = name_buf;
                flag = TRUE;
            }
    }
    while (flag && (--up) > 1);
}/* End order_domain */
/***************************************************************/
/* NAME 	: get_transition_domain 		       */
/* DESCRIPTION	: Assegna i valori ai campi del descrittore di */
/*		  transizione DOMINIO, COMP_NUM, RIP e OFF.    */
/*		  Procede all'ordinamento del dominio.         */
/* PARAMETERS	: Indice della transizione.		       */
/* RETURN VALUE : Nessuno.				       */
/***************************************************************/
static void get_transition_domain(tr)
int tr;
{
    /* Init get_transition_domain */
    int ind;

    if (ncl != 0) {
        /* C'e' almeno una classe */
        tabt[tr].rip = (int *)ecalloc(ncl, sizeof(int));
        tabt[tr].off = (int *)ecalloc(ncl, sizeof(int));
#ifdef SYMBOLIC
        tabt[tr].split_type = (int *)ecalloc(ncl, sizeof(int));
#endif
        for (ind = 0 ; ind < ncl ; ind++) {
            tabt[tr].rip[ind] = tabt[tr].off[ind] = 0;
#ifdef SYMBOLIC
            tabt[tr].split_type[ind] = UNKNOWN;
#endif
        }
    }/* C'e' almeno una classe */
    if (comp_check != 0) {
        /* Il dominio non e' neutro */
        tabt[tr].dominio = (int *)ecalloc(comp_check, sizeof(int));
        tabt[tr].comp_type = (int *)ecalloc(comp_check, sizeof(int));
        tabt[tr].names = (char **)ecalloc(comp_check, sizeof(char *));
        for (ind = 0; ind < comp_check ; ind++) {
            tabt[tr].dominio[ind] = occ[tr][ind].cclass;
            tabt[tr].rip[tabt[tr].dominio[ind]]++;
            tabt[tr].names[ind] = (char *)ecalloc(strlen(occ[tr][ind].p_fun_name) + 1, sizeof(char));
            strcpy(tabt[tr].names[ind], occ[tr][ind].p_fun_name);
            if (tabt[tr].rip[tabt[tr].dominio[ind]] > MAX_COL_RIP)
                out_error(ERROR_LOW_MAX_COL_RIP, 0, tr, 0, 0, NULL, NULL);
        }
    }/* Il dominio non e' neutro */
    tabt[tr].comp_num = comp_check;
    order_domain(tabt[tr].dominio, tabt[tr].comp_num, tabt[tr].names);
    for (ind = 0; ind < tabt[ntr].comp_num; ind += tabt[ntr].rip[tabt[ntr].dominio[ind]])
        tabt[ntr].off[tabt[ntr].dominio[ind]] = ind;
}/* End get_transition_domain */
#endif
/*****************************************************************
nome	       :
descrizione    :
parametri      :

valori ritorno :
******************************************************************/
static void report(stat, type)
int stat;
int type;
{
    /* Init report */
    char name[MAXSTRING];
    int tr;

    if (parse_DEF == MDRATE)
        tr = nmd;
    else
        tr = ntr;
    switch (type) {
    case INHIBITOR  : strcpy(name, "of an inhibitor function");
        break;
    case INPUT      : strcpy(name, "of an input function");
        break;
    case OUTPUT     : strcpy(name, "of an output function");
        break;
    case TRANSITION : strcpy(name, " ");
        break;
    case MDRATE     : strcpy(name, "of a marking dependent definition");
        break;
    }
    switch (stat) {
#ifdef SWN
    case ERROR_UNKNOWN_FUNCTION :
        fprintf(stdout, "ERROR : unknown function name\n%s of transition %s\n", name, tabt[tr].trans_name);
        exit(1);
        break ;
    case ERROR_DIFFERENT_CLASSES :
        fprintf(stdout, "ERROR : different colour classes used in the predicate \n%s of transition %s\n", name, tabt[tr].trans_name);
        exit(1);
        break ;
    case ERROR_WRONG_INDEX :
        fprintf(stdout, "ERROR : wrong static subclass index used in the predicate\n%s of transition %s\n", name, tabt[tr].trans_name);
        exit(1);
        break ;
#endif
    case ERROR_WRONG_OPERATOR :
        fprintf(stdout, "ERROR : wrong relational operator used in the predicate\n%s of transition %s\n", name, tabt[tr].trans_name);
        exit(1);
        break ;
#ifdef SWN
    case ERROR_ILLEGAL_FUNCTION :
        fprintf(stdout, "ERROR : illegal use of function for ordered classes in the predicate \n%s of transition %s\n", name, tabt[tr].trans_name);
        exit(1);
        break;
#endif
    default :	break;
    }
}/* End report */
/*****************************************************************
nome	       : emit
descrizione    : Genera il codice intermedio per valutare i ratemd
parametri      : Puntatore all'array contenente il codice,tipo di
		 operazione,stringa da memorizzare,valore
valori ritorno : Nessuno
******************************************************************/
static int fill_pred(pred)
Pred_p pred;
{
    Pred_p c = pred;
    Pred_p n = pred->next;
    int not_found = 1;
    int occ_ind, status = 0;
    int i, ind;
    int tr;

    if (parse_DEF == MDRATE) {
        tr = nmd;
#ifdef SWN
        comp_check = tabt[tr].comp_num;
#endif
    }
    else
        tr = ntr;
    do {
        /* Init loop */
        if (c->type > TYPEOP) {
            /* Predicato base */
#ifdef DACANCELLARE
            if (c->type == TYPENUM)
                c->col_ind = atoi(c->fun_name);
            if (c->type == TYPEPAR) {
                /* Static subclass name or function name ? */
                c->type = TYPENUM;
                ind = get_marking_parameter_offset(c->fun_name);
                sprintf(c->fun_name, "%d", tabmp[ind].mark_val);
            }/* Static subclass name or function name ? */
            if (c->type == TYPEPLACEMARKING)
                if ((c->col_ind = get_pl_in(c->fun_name)) == UNKNOWN) {
                    out_error(ERROR_UNKNOWN_PLACE_IN_PREDICATE, UNKNOWN, UNKNOWN, 0, 0, c->fun_name, NULL);
                    break;
                }
            if (n->type == TYPEPLACEMARKING)
                if ((n->col_ind = get_pl_in(n->fun_name)) == UNKNOWN) {
                    out_error(ERROR_UNKNOWN_PLACE_IN_PREDICATE, UNKNOWN, UNKNOWN, 0, 0, n->fun_name, NULL);
                    break;
                }
#endif
#ifdef SWN
            if (c->type == TYPESTRING) {
                /* Static subclass name or function name ? */
                not_found = TRUE;
                for (i = 0; i < ncl; i++)
                    if ((ind = already_exist(c->fun_name, i, STAT)) != 0) {
                        not_found = FALSE;
                        break;
                    }
                if (!not_found) {
                    c->type = TYPENUM;
                    sprintf(c->fun_name, "%d", ind - 1);
                }
            }/* Static subclass name or function name ? */
            if (c->type == TYPED ||
                    (c->type == TYPESTRING && not_found) ||
                    c->type == TYPESUCCESSOR ||
                    c->type == TYPEPREDECESSOR ||
                    c->type == TYPEDISTANCE) {
                /* Check of function name */
                not_found = TRUE;
                for (occ_ind = 0 ; occ_ind < comp_check && not_found ; occ_ind++)
                    if (!strcmp(c->fun_name, occ[tr][occ_ind].p_fun_name))
                        not_found = FALSE;
                if (not_found) {
                    /* Occorrenza inesistente */
                    status = ERROR_UNKNOWN_FUNCTION;
                    break;
                } /* Occorrenza inesistente */
                c->occ = occ[tr][occ_ind - 1].occ_num;
                c->col_ind = occ[tr][occ_ind - 1].cclass;
            }/* Check of function name */
            if (n->type == TYPESTRING) {
                /* Static subclass name or function name ? */
                not_found = TRUE;
                for (i = 0; i < ncl; i++)
                    if ((ind = already_exist(n->fun_name, i, STAT)) != 0) {
                        not_found = FALSE;
                        break;
                    }
                if (!not_found) {
                    n->type = TYPENUM;
                    sprintf(n->fun_name, "%d", ind - 1);
                }
            }/* Static subclass name or function name ? */
            if (n->type == TYPED ||
                    (n->type == TYPESTRING && not_found) ||
                    n->type == TYPESUCCESSOR ||
                    n->type == TYPEPREDECESSOR ||
                    n->type == TYPEDISTANCE) {
                /* Check of function name */
                not_found = TRUE;
                for (occ_ind = 0 ; occ_ind < comp_check && not_found ; occ_ind++)
                    if (!strcmp(n->fun_name, occ[tr][occ_ind].p_fun_name))
                        not_found = FALSE;
                if (not_found) {
                    /* Occorrenza inesistente */
                    status = ERROR_UNKNOWN_FUNCTION;
                    break;
                }/* Occorrenza inesistente */
                n->occ = occ[tr][occ_ind - 1].occ_num;
                n->col_ind = occ[tr][occ_ind - 1].cclass;
            }/* Check of function name */
#ifdef DOPO
            if (c->type == TYPED && n->type == TYPENUM) {
                /* Predicate of type d(x) relop K */
                if (atoi(n->fun_name) + 1 < 1
                        || atoi(n->fun_name) + 1 > tabc[c->col_ind].sbc_num) {
                    /* Indice sottoclasse errato */
                    status = ERROR_WRONG_INDEX;
                    break;
                }/* Indice sottoclasse errato */
                if (n->next->type != TYPEEQ && n->next->type != TYPENE) {
                    /* Wrong operator */
                    status = ERROR_WRONG_OPERATOR;
                    break;
                }/* Wrong operator */
            }/* Predicate of type d(x) relop K */

            if (c->type == TYPENUM && n->type == TYPED)) {
                /* Predicate of type K relop d(x) */
                if (atoi(c->fun_name) + 1 < 1
                        || atoi(c->fun_name) + 1 > tabc[n->col_ind].sbc_num) {
                    /* Indice sottoclasse errato */
                    status = ERROR_WRONG_INDEX;
                    break;
                }/* Indice sottoclasse errato */
                if (n->next->type != TYPEEQ && n->next->type != TYPENE) {
                    /* Wrong operator */
                    status = ERROR_WRONG_OPERATOR;
                    break;
                }/* Wrong operator */
            }/* Predicate of type K relop d(x) */
            if (
                (c->type == TYPED ||
                 c->type == TYPESTRING ||
                 c->type == TYPESUCCESSOR ||
                 c->type == TYPEPREDECESSOR
                )
                    &&
                    (n->type == TYPED ||
                     n->type == TYPESTRING ||
                     n->type == TYPESUCCESSOR ||
                     n->type == TYPEPREDECESSOR
                    )
                )









                n->occ = occ[tr][occ_ind - 1].occ_num;
                n->col_ind = occ[tr][occ_ind - 1].class;
                if (n->col_ind != c->col_ind) {
                /* Diverse classi */
                status = ERROR_DIFFERENT_CLASSES;
                break;
            }/* Diverse classi */
        if ((c->type == TYPESUCCESSOR
                    || c->type == TYPEPREDECESSOR
                    || c->type == TYPEDISTANCE
                )
                        && (tabc[c->col_ind].type != ORDERED)
                   ) {
                /* Funzione successore illegale */
                status = ERROR_ILLEGAL_FUNCTION;
                break;
            }/* Funzione successore illegale*/
            if ((n->type == TYPESUCCESSOR
                    || n->type == TYPEPREDECESSOR
                    || n->type == TYPEDISTANCE
                )
                        && (tabc[n->col_ind].type != ORDERED)
                   ) {
                /* Funzione successore illegale */
                status = ERROR_ILLEGAL_FUNCTION;
                break;
            }/* Funzione successore illegale*/
        }/* Basic SWN Predicate  */
#endif
#endif
    }/* Predicato base */
    c = c->next;
    n = n->next;
}
while (c->type != TYPEEND);
return (status);
}
/*****************************************************************/
/* NAME 	: check_predicate				 */
/* DESCRIPTION	: Richiama le funzioni che aggiornano e 	 */
/*		  controllano i codici dei predicati.		 */
/* PARAMETERS	: Puntatore alla struttura in cui e' memorizzato */
/*		  il codice di valutazione del predicato.	 */
/* RETURN VALUE : Nessuno.					 */
/*****************************************************************/
void check_predicate(pred_ptr, type)
Pred_p pred_ptr;
int type;
{
    /* Init check_predicate */
    int stat = UNKNOWN;

    if (pred_ptr != NULL)
        if (pred_ptr->type != TYPENULL) {
            stat = fill_pred(pred_ptr);
            report(stat, type);
        }
}/* End check_predicate */
/*********************************************************************/
/* NAME 	: check_predicates_consistency			     */
/* DESCRIPTION	: Richiama la funzione check_predicate sui predicati */
/*		  delle funzioni sugli archi o sul predicato della   */
/*		  transizione.					     */
/* PARAMETERS	: Indice della transizione e selettore. 	     */
/* RETURN VALUE : Nessuno.					     */
/*********************************************************************/
static void check_predicates_consistency(tr, type)
int tr;
int type;
{
    /* Init check_predicates_consistency */
    Node_p list = NULL;
    Coeff_p coeff_ptr = NULL;

    switch (type) {
    /* Tipo di arco */
    case INPUT :      list = GET_INPUT_LIST(tr);
        break;
    case OUTPUT :     list = GET_OUTPUT_LIST(tr);
        break;
    case INHIBITOR :  list = GET_INHIBITOR_LIST(tr);
        break;
    case TRANSITION : check_predicate(GET_TRANSITION_PREDICATE(tr), type);
        list = NULL;
        break;
    }/* Tipo di arco */
    for (; list != NULL ; list = NEXT_NODE(list))
        for (coeff_ptr = list->arcfun ; coeff_ptr != NULL ;) {
            check_predicate(coeff_ptr->guard, type);
            coeff_ptr = coeff_ptr->next;
        }
}/* End check_predicates_consistency */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_transition_data_structure(tr)
int tr;
{
    /* Init fill_transition_data_structure */
#ifdef SWN
    /* Campi: dominio, comp_num, rip, off e ordinamento del dominio */
    get_transition_domain(tr);
    /* Campi: dominio, comp_num, rip, off e ordinamento del dominio */
#endif

    check_predicates_consistency(tr, TRANSITION);
    check_predicates_consistency(tr, INPUT);
    check_predicates_consistency(tr, INHIBITOR);
    check_predicates_consistency(tr, OUTPUT);

    fill_node_data_structures(tr, INPUT);
    fill_node_data_structures(tr, INHIBITOR);
    fill_node_data_structures(tr, OUTPUT);

#ifdef SWN
    fill_output_components(tr);  /* Serve per IN o OUT */
#endif
    if (is_source(tr) && tabt[tr].no_serv != 1)
        tabt[tr].no_serv = 1;
#ifdef SWN
#ifdef SYMBOLIC
#ifdef REACHABILITY
    successor_or_predecessor(tr);
#endif
#endif
#ifdef SIMULATION
    tabt[tr].skippable = is_skippable_after_firing(tr);
    tabt[tr].input_preselectable = is_input_preselectable(tr);
    fill_split_type_field(tr);
#endif
#endif
}/* End fill_transition_data_structure */
