# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/macros.h"
# include "../../INCLUDE/ealloc.h"
# include "../../INCLUDE/service.h"


#define MAX_PREDICATE_STACK 100
#define MAX_EXPRESSION_STACK 100

extern double cur_time;

#ifdef DEBUG_malloc
static int event_mall = 0;
static int event_push = 0;
static int event_pop = 0;

#ifdef DEBUG_simulation
int *em = NULL;
int *ep = NULL;
int *eo = NULL;
#endif

#endif

/* predeclaration */
int get_static_subclass(int  cl,  int  dsc);

Event_p *event_free = NULL;   /* Array di free list per gli eventi */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int lesser(int  n1,  int  n2) {
    /* Init lesser */
    return (n1 < n2);
}/* End lesser */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int greater(int  n1,  int  n2) {
    /* Init greater */
    if (n2 > 0)
        return (n1 >= n2);
    else
        return (FALSE);
}/* End greater */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void initialize_free_event_list() {
    /* Init initialize_free_event_list */
    int ii;
#ifdef LIBSPOT
    int a = ntr;
    int ntr = a + nSpottr ;
#endif /* LIBSPOT */
    event_free = (Event_p *)ecalloc(ntr, sizeof(Event_p));


#ifdef DEBUG_simulation
    em = (int *)ecalloc(ntr, sizeof(int));
    ep = (int *)ecalloc(ntr, sizeof(int));
    eo = (int *)ecalloc(ntr, sizeof(int));
#endif
    for (ii = 0; ii < ntr ; ii++) {
        event_free[ii] = NULL;
#ifdef DEBUG_simulation
        em[ii] = ep[ii] = eo[ii] = 0;
#endif
    }
}/* Init initialize_free_event_list */
#ifdef DEBUG_malloc
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_event_info() {
    /* Init out_event_info */
    fprintf(stdout, "--------------- EVENT NOTICE STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , event_mall, event_push, event_pop);
}/* End out_event_info */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Event_p get_new_event(int  tr) {
    /* Init get_new_event */
    int i;
    Event_p ptr;

#ifdef SIMULATION
#ifndef SYMBOLIC
#ifdef DEBUG_simulation
    eo[tr]++;
#endif
#endif
#endif
#ifdef DEBUG_malloc
    event_pop++;
#endif
    if (event_free[tr] == NULL) {
        ptr = (Event_p)emalloc(sizeof(struct ENABLING));
#ifdef SWN
        if (tabt[tr].comp_num) {
            ptr->npla = (int *)ecalloc(tabt[tr].comp_num, sizeof(int));
#ifdef SYMBOLIC
            ptr->split = (int *)ecalloc(tabt[tr].comp_num, sizeof(int));
#endif
        }
        else {
            ptr->npla = NULL;
#ifdef SYMBOLIC
            ptr->split = NULL;
#endif
        }
#endif
        ptr->trans = tr;
#ifdef DEBUG_malloc
        event_mall++;
#endif
#ifdef SIMULATION
#ifndef SYMBOLIC
#ifdef DEBUG_simulation
        em[tr]++;
        ptr->tag = em[tr];
#endif
#endif
#endif
    }
    else {
        ptr = event_free[tr];
        event_free[tr] = event_free[tr]->next;
    }
    ptr->next = NULL;
#ifdef SIMULATION
    ptr->t_next = ptr->t_prev = NULL;
    ptr->e_prev = ptr->e_next = NULL;
    ptr->last_sched_desched_time = ptr->residual_sched_time = -1;
    ptr->creation_time = cur_time;
#endif
    if (IS_IMMEDIATE(tr))
        ptr->enabling_degree = 1;
    else {
        /* Transizione temporizzata */
        if (IS_INFINITE_SERVER(tr))
            ptr->enabling_degree = MAX_INT;
        else
            ptr->enabling_degree = tabt[tr].no_serv;
    }/* Transizione temporizzata */
#ifdef SWN
#ifdef SYMBOLIC
    ptr->ordinary_instances = UNKNOWN;
#endif
    for (i = tabt[tr].comp_num; i; i--) {
        ptr->npla[i - 1] = UNKNOWN;
#ifdef SYMBOLIC
        ptr->split[i - 1] = UNKNOWN;
#endif
    }
#endif
    return (ptr);
}/* End get_new_event */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void dispose_old_event(Event_p  ptr) {
    /* Init dispose_old_event */
    int tr = ptr->trans;

    ptr->next = event_free[tr];
    event_free[tr] = ptr;
#ifdef DEBUG_malloc
    event_push++;
#endif
#ifdef DEBUG_simulation
    ep[tr]++;
#endif
}/* End dispose_old_event */

#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int compare_input_event_id(Event_p  ev_1,  Event_p  ev_2,  int  tr) {
    /* Init compare_input_event_id */
    int ii , equal = TRUE, ret_value = UNKNOWN;

    for (ii = tabt[tr].comp_num ; ii && equal ; ii--) {
        /* Per ogni componente del dominio della transizione */
        if (tabt[tr].comp_type[ii - 1] == IN) {
            if (ev_1->npla[ii - 1] != UNKNOWN && ev_2->npla[ii - 1] != UNKNOWN) {
                /* Componenti fissate */
                equal = (ev_1->npla[ii - 1] == ev_2->npla[ii - 1]);
#ifdef SYMBOLIC
                equal &= (ev_1->split[ii - 1] == ev_2->split[ii - 1]);
#endif
            }/* Componenti fissate */
        }
    }/* Per ogni componente del dominio della transizione */
    if (equal) {
        /* Ricerca con successo */
        ret_value = EQUAL_TO;
    }/* Ricerca con successo */
    return (ret_value);
}/* End compare_input_event_id */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Event_p next_instance(Event_p  pt) {
    /* Init next_instance */
    Event_p ret_ptr;

    ret_ptr = pt->next;
    dispose_old_event(pt);
    return (ret_ptr);
}/* End next_instance */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void copy_event(Event_p  dst,  Event_p  src) {
    /* Init copy_event */
    int i;

#ifdef SWN
    for (i = tabt[src->trans].comp_num; i; i--) {
        dst->npla[i - 1] = src->npla[i - 1];
#ifdef SYMBOLIC
        dst->split[i - 1] = src->split[i - 1];
#endif
    }
#endif
    dst->trans = src->trans;
    dst->enabling_degree = src->enabling_degree;
}/* End copy_event */
#ifdef SWN
#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_object_from_dynamic(int  cl,  int  ds) {
    /* Init get_object_from_dynamic */
    int ss, ds_ind, ret_value = UNKNOWN;

    ss = get_static_subclass(cl, ds);
    ret_value = GET_STATIC_OFFSET(cl, ss);
    for (ds_ind = 0 ; ds_ind < ds ; ds_ind++)
        ret_value += GET_CARD_BY_TOTAL(cl, ss, ds_ind);
    return (ret_value);
}/* End get_object_from_dynamic */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_dynamic_from_object(int  cl,  int  obj) {
    /* Init get_dynamic_from_object */
    int ss , dtot = 0 , ret_value = UNKNOWN, ds;

    for (ss = 0 ; ss < tabc[cl].sbc_num ; ss++)
        for (ds = 0; ds < GET_NUM_SS(cl, ss) ; ds++) {
            dtot += GET_CARD_BY_TOTAL(cl, ss, ds);
            if (obj < dtot) {
                ret_value = GET_STATIC_OFFSET(cl, ss) + ds;
                goto ret;
            }
        }
ret: return (ret_value);
}/* End get_dynamic_from_object */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_dynamic_from_total(int  cl,  int  ds) {
    /* Init get_dynamic_from_total */
    int ss , dtot = 0 , ret_value = UNKNOWN, diff;

    for (ss = 0, diff = ds ; ss < tabc[cl].sbc_num ; ss++) {
        dtot += GET_NUM_SS(cl, ss);
        if (ds < dtot) {
            ret_value = GET_STATIC_OFFSET(cl, ss) + diff;
            goto ret;
        }
        diff -= GET_NUM_SS(cl, ss);
    }
    if (ret_value == UNKNOWN)
        printf("sous classe unconue");
ret: return (ret_value);
}/* End get_dynamic_from_total */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_static_subclass(int  cl,  int  dsc) {
    /* Init get_static_subclass */
    int i , dtot = 0 , ret_value = UNKNOWN;

    if (dsc != UNKNOWN) {
        /* Elemento noto */
        for (i = 0 ; i < tabc[cl].sbc_num ; i++) {
            dtot += tabc[cl].sbclist[i].card;
            if (dsc < dtot) {
                ret_value = i;
                goto ret;
            }
        }
    }/* Elemento noto */
ret: return (ret_value);
}/* End get_static_subclass */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int optimized_nextvec(Cart_p  cartes_ptr,  int  comp,  int  *base) {
    /* Init optimized_nextvec */
    int r, flag;

    if (comp) {
        r = 0;
        do {
            flag = TRUE;
            if (cartes_ptr->low[r] + 1 > cartes_ptr->up[r]) {
                cartes_ptr->low[r] = base[r];
                r++;
                flag = FALSE;
            }
            else
                cartes_ptr->low[r]++;
        }
        while ((r < comp) && (!flag));
        if (r < comp)
            return (TRUE);
        else
            return (FALSE);
    }
    else
        return (FALSE);
}/* End optimized_nextvec */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int nextvec(Cart_p  cartes_ptr,  int  comp) {
    /* Init nextvec */
    int r, flag;

    if (comp) {
        r = 0;
        do {
            flag = TRUE;
            if (cartes_ptr->low[r] + 1 > cartes_ptr->up[r]) {
                cartes_ptr->low[r] = 0;
                r++;
                flag = FALSE;
            }
            else
                cartes_ptr->low[r]++;
        }
        while ((r < comp) && (!flag));
        if (r < comp)
            return (TRUE);
        else
            return (FALSE);
    }
    else
        return (FALSE);
}/* End nextvec */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int nextvec_with_fix(Cart_p  cartes_ptr,  int  comp,  int  fix[MAX_DOMAIN]) {
    /* Init nextvec_with_fix */
    int r, flag;

    if (comp) {
        r = 0;
        if (r != comp) {
            do {
                flag = TRUE;
                if (cartes_ptr->low[r] + 1 > cartes_ptr->up[r]) {
                    if (!fix[r])
                        cartes_ptr->low[r] = 0;
                    r++;
                    flag = FALSE;
                }
                else
                    cartes_ptr->low[r]++;
            }
            while ((r < comp) && (!flag));
        }
        if (r < comp)
            return (TRUE);
        else
            return (FALSE);
    }
    else
        return (FALSE);
}/* End nextvec_with_fix */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_obj_dist(int  ob1,  int  ob2,  int  cl) {
    /* Init get_obj_dist */
    int dist;

    if (ob1 <= ob2)
        dist = ob2 - ob1;
    else
        dist = tabc[cl].card - (ob1 - ob2);
    return (dist);
}/* End get_obj_dist */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_prec(int  cl,  int  ds) {
    /* Init get_prec */
    int prec;

    if (ds == 0)
        prec = tabc[cl].card - 1;
    else
        prec = ds - 1;
    return (prec);
}/* End get_prec */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_obj_off(int  sb,  int  cl) {
    /* Init get_obj_off */
    int ii, card = 0;

    for (ii = 0; ii < sb; ii++)
        card += tabc[cl].sbclist[ii].card;
    return (card);
}/* End get_obj_off */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int fact(int  n) {
    /* Factorial function */
    int i;
    int ret = 1;

    for (i = 2; i < n + 1; i++)
        ret *= i;
    return (ret);
}/* Factorial function */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int molt(int  n,  int  k) {
    /* Factorial function */
    int i;
    int ret = 1;

    for (i = k; i < n + 1 ; i++)
        ret *= i;
    return (ret);
}/* Factorial function */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int comb(int  n,  int  k) {
    /* Combination function */
    int ret = 1;
    int ik = k;
    int in = n;

    if (ik == 0)
        goto retu;
    if (ik <= in / 2)
        ret = molt(n, n - k + 1) / molt(k, 1);
    else
        ret = molt(n, k + 1) / molt(n - k, 1);
retu: return (ret);
}/* Combination function */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
double evaluate_rate_expression(Expr_p  code,  Event_p  ev_p) {
    /* Init evaluate_rate_expression */
    int tr;
    int sp = -1, end = FALSE;
    double st[MAX_EXPRESSION_STACK];
    int pl;
    Expr_p c = NULL;

    if (code == NULL) {
        fprintf(stdout, "Warning: expression code empty\n");
        exit(1);
    }
    if (ev_p != NULL)
        tr = GET_TRANSITION_INDEX(ev_p);
    c = code;
    for (; !end;) {
        /* Ciclo di valutazione */
        switch (c->op_type) {
        /* Decodifica istruzione */
        case TYPEINV : {
            /* Inverse */
            st[sp] = 1.0 / st[sp];
            break;
            }/* Inverse */
        case TYPEFACT : {
            /* Factorial */
            st[sp] = fact(st[sp]);
            break;
            }/* Factorial */
        case TYPECOMB : {
            /* Combinatorial */
            sp--;
            st[sp] = comb(st[sp], st[sp + 1]);
            break;
            }/* Combinatorial */
        case TYPESQRT : {
            /* Sqrt */
            st[sp] = sqrt(st[sp]);
            break;
            }/* Sqrt */
        case TYPEPOW : {
            /* Power */
            sp--;
            st[sp] = pow(st[sp], st[sp + 1]);
            break;
            }/* Power */
        case TYPEMAX : {
            /* Maximum */
            sp--;
            st[sp] = MAX(st[sp], st[sp + 1]);
            break;
            }/* Maximum */
        case TYPEMIN : {
            /* Minimum */
            sp--;
            st[sp] = MIN(st[sp], st[sp + 1]);
            break;
            }/* Minimum */
        case TYPEPLUS : {
            /* Sum */
            sp--;
            st[sp] += st[sp + 1];
            break;
            }/* Sum */
        case TYPEMINUS : {
            /* Substraction */
            sp--;
            st[sp] -= st[sp + 1];
            break;
            }/* Substraction */
        case TYPEMOLT : {
            /* Multiplication */
            sp--;
            st[sp] *= st[sp + 1];
            break;
            }/* Multiplication */
        case TYPEDIV : {
            /* Division */
            sp--;
            st[sp] /= st[sp + 1];
            break;
            }/* Division */
        case TYPEEND : {
            /* Fine codice predicato */
            end = TRUE;
            break;
            }/* Fine codice predicato */
        case TYPEREALNUM : {
            /* Real Value */
            st[++sp] = c->val;
            break;
            }/* Real Value */
        case TYPEPAR : {
            /* Marking or Rate Parameter */
            st[++sp] = c->val;
            break;
            }/* Marking or Rate Parameter */
        case TYPENUM : {
            /* Integer Number */
            st[++sp] = c->val;
            break;
            }/* Integer Number */
        case TYPEMARK : {
            /* Place Marking */
            pl = c->ival;
            st[++sp] = net_mark[pl].total;
            break;
            }/* Place Marking */
        default : {
            fprintf(stdout, "Error: expression has unknown opcode\n");
            exit(1);
        }
        }/* Decodifica istruzione */
        c = c->next;
    }/* Ciclo di valutazione */
    return (st[sp]);
}/* End evaluate_rate_expression */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int evaluate_expression(Expr_p  code,  Event_p  ev_p) {
    /* Init evaluate_expression */
    int tr;
    int sp = -1, end = FALSE;
#ifdef DAPREVEDERE_IL_DOUBLE
    double st[MAX_EXPRESSION_STACK];
#endif
    int st[MAX_EXPRESSION_STACK];
    int pl;
    Expr_p c = NULL;

    if (code == NULL) {
        fprintf(stdout, "Warning: expression code empty\n");
        exit(1);
    }
    if (ev_p != NULL)
        tr = GET_TRANSITION_INDEX(ev_p);
    c = code;
    for (; !end;) {
        /* Ciclo di valutazione */
        switch (c->op_type) {
        /* Decodifica istruzione */
        case TYPEINV : {
            /* Inverse */
            st[sp] = 1.0 / st[sp];
            break;
            }/* Inverse */
        case TYPEFACT : {
            /* Factorial */
            st[sp] = fact(st[sp]);
            break;
            }/* Factorial */
        case TYPECOMB : {
            /* Combinatorial */
            sp--;
            st[sp] = comb(st[sp], st[sp + 1]);
            break;
            }/* Combinatorial */
        case TYPESQRT : {
            /* Sqrt */
            st[sp] = (int)sqrt(st[sp]);
            break;
            }/* Sqrt */
        case TYPEPOW : {
            /* Power */
            sp--;
            st[sp] = (int)pow(st[sp], st[sp + 1]);
            break;
            }/* Power */
        case TYPEMAX : {
            /* Maximum */
            sp--;
            st[sp] = MAX(st[sp], st[sp + 1]);
            break;
            }/* Maximum */
        case TYPEMIN : {
            /* Minimum */
            sp--;
            st[sp] = MIN(st[sp], st[sp + 1]);
            break;
            }/* Minimum */
        case TYPEPLUS : {
            /* Sum */
            sp--;
            st[sp] += st[sp + 1];
            break;
            }/* Sum */
        case TYPEMINUS : {
            /* Substraction */
            sp--;
            st[sp] -= st[sp + 1];
            break;
            }/* Substraction */
        case TYPEMOLT : {
            /* Multiplication */
            sp--;
            st[sp] *= st[sp + 1];
            break;
            }/* Multiplication */
        case TYPEDIV : {
            /* Division */
            sp--;
            st[sp] /= st[sp + 1];
            break;
            }/* Division */
        case TYPEEND : {
            /* Fine codice predicato */
            end = TRUE;
            break;
            }/* Fine codice predicato */
        case TYPEREALNUM : {
            /* Real Value */
            st[++sp] = c->val;
            break;
            }/* Real Value */
        case TYPEPAR : {
            /* Marking or Rate Parameter */
            st[++sp] = c->ival;
            break;
            }/* Marking or Rate Parameter */
        case TYPENUM : {
            /* Integer Number */
            st[++sp] = c->ival;
            break;
            }/* Integer Number */
        case TYPEMARK : {
            /* Place Marking */
            pl = c->ival;
#ifdef GSPN
            st[++sp] = net_mark[pl].mirror_total;
#endif
#ifdef SWN
            st[++sp] = net_mark[pl].total;
#endif
            break;
            }/* Place Marking */
        default : {
            fprintf(stdout, "Error: expression has unknown opcode\n");
            exit(1);
        }
        }/* Decodifica istruzione */
        c = c->next;
    }/* Ciclo di valutazione */
    return (st[sp]);
}/* End evaluate_expression */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int  evaluate_op(o1, o2, code)
int o1;
int o2;
Pred_p code;
{
    /* Init evaluate_op */
    int ret_value = TRUE;

    if (o1 != UNKNOWN && o2 != UNKNOWN) {
        /* Both operands are known */
        switch (code->type) {
        case TYPEEQ: if (!(o1 == o2)) ret_value = FALSE; break;
        case TYPENE: if (!(o1 != o2)) ret_value = FALSE; break;
        case TYPEGT: if (!(o1 > o2)) ret_value = FALSE; break;
        case TYPEGE: if (!(o1 >= o2)) ret_value = FALSE; break;
        case TYPELT: if (!(o1 < o2)) ret_value = FALSE; break;
        case TYPELE: if (!(o1 <= o2)) ret_value = FALSE; break;
        }
    }/* Both operands are known */
    return (ret_value);
}/* End evaluate_op */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_operand(c, ev_p)
Pred_p *c;
Event_p ev_p;
{
    /* Init get_operand */

    int ret_val = UNKNOWN;
#ifdef GSPN
    ret_val = evaluate_expression((*c)->expression, NULL);
    (*c) = (*c)->next;
#endif
#ifdef SWN
    int tr = GET_TRANSITION_INDEX(ev_p);

    int cl, cl2, oc, oc2, o, o2;
    int p, p2;

    switch ((*c)->type) {
    case TYPENUM:
        ret_val = atoi((*c)->fun_name);
        (*c) = (*c)->next;
        break;
    case TYPESTRING :
        cl = (*c)->col_ind;
        oc = (*c)->occ - 1;
        p = tabt[tr].off[cl] + oc;
        o = ev_p->npla[p];
#ifdef SYMBOLIC
#ifdef SIMULATION
        if (IS_UNORDERED(cl)) {
            /* Non split a fire time */
            o = ev_p->split[p];
        }/* Non split a fire time */
#endif
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            /* Non split a fire time */
            o = ev_p->split[p];
        }/* Non split a fire time */
#endif
#endif
        ret_val = o;
        (*c) = (*c)->next;
        break;
    case TYPED :
        cl = (*c)->col_ind;
        oc = (*c)->occ - 1;
        p = tabt[tr].off[cl] + oc;
        ret_val = get_static_subclass(cl, ev_p->npla[p]);
        (*c) = (*c)->next;
        break;
    case TYPEDISTANCE :
        cl = (*c)->col_ind;
        oc = (*c)->occ - 1;
        p = tabt[tr].off[cl] + oc;
        o = ev_p->npla[p];

        (*c) = (*c)->next;
        cl2 = (*c)->col_ind;
        oc2 = (*c)->occ - 1;
        p2 = tabt[tr].off[cl2] + oc2;
        o2 = ev_p->npla[p2];

        if (o != UNKNOWN && o2 != UNKNOWN)
            ret_val = get_obj_dist(o, o2, cl2);
        (*c) = (*c)->next;
        break;
    case TYPEPREDECESSOR :
        cl = (*c)->col_ind;
        oc = (*c)->occ - 1;
        p = tabt[tr].off[cl] + oc;
        o = ev_p->npla[p];
#ifdef SYMBOLIC
#ifdef SIMULATION
        if (IS_UNORDERED(cl)) {
            /* Non split a fire time */
            o = ev_p->split[p];
        }/* Non split a fire time */
#endif
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            /* Non split a fire time */
            o = ev_p->split[p];
        }/* Non split a fire time */
#endif
#endif
        ret_val = get_prec(cl, o);
        (*c) = (*c)->next;
        break;
    case TYPESUCCESSOR :
        cl = (*c)->col_ind;
        oc = (*c)->occ - 1;
        p = tabt[tr].off[cl] + oc;
        o = ev_p->npla[p];
#ifdef SYMBOLIC
#ifdef SIMULATION
        if (IS_UNORDERED(cl)) {
            /* Non split a fire time */
            o = ev_p->split[p];
        }/* Non split a fire time */
#endif
#ifdef REACHABILITY
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            /* Non split a fire time */
            o = ev_p->split[p];
        }/* Non split a fire time */
#endif
#endif
        ret_val = (o + 1) % tabc[cl].card;
        (*c) = (*c)->next;
        break;
    }
#endif
    return (ret_val);
}/* End get_operand */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int verify_predicate_constraint(Pred_p  code,  Event_p  ev_p) {
    /* Init verify_predicate_constraint */
    int sp = -1, end = FALSE;
    int o1, o2, st[MAX_PREDICATE_STACK];
    Pred_p c;

    if (code == NULL)
        return (TRUE);
    c = code;
    for (; !end;) {
        /* Ciclo di valutazione */
        switch (c->type) {
        /* Decodifica istruzione */
        case TYPEAND : {
            /* AND logico */
            sp--;
            st[sp] &= st[sp + 1];
            c = c->next;
            break;
            }/* AND logico */
        case TYPEOR : {
            /* OR logico */
            sp--;
            st[sp] |= st[sp + 1];
            c = c->next;
            break;
            }/* OR logico */
        case TYPEEND : {
            /* Fine codice predicato */
            end = TRUE;
            break;
            }/* Fine codice predicato */
        default: {
            /* Operators */
            o1 = get_operand(&c, ev_p);
            o2 = get_operand(&c, ev_p);
            st[++sp] = evaluate_op(o1, o2, c);
            c = c->next;
            }/* Operators */
        }/* Decodifica istruzione */
    }/* Ciclo di valutazione */
    return (st[sp]);
}/* End verify_predicate_constraint */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
double get_instance_rate(Event_p  ev_p) {
    /* Init get_instance_rate */
    int tr = GET_TRANSITION_INDEX(ev_p);
    MDrate_p ptr = tabt[tr].md_rate_val;
    MDrate_p ever_ptr = tabt[tr].md_rate_val;
    double ret_val = tabt[tr].mean_t;
    int found = FALSE;

    if (ptr != NULL) {
        /* Transizione con rate dipendente dall'istanza */
        if (ever_ptr->next != NULL) {
            /* C'e' una lista di when */
            ptr = ptr->next;
            for (; ptr != NULL; ptr = ptr->next) {
                if (verify_predicate_constraint(ptr->guard, ev_p)) {
                    ret_val = evaluate_rate_expression(ptr->code, ev_p);
                    found = TRUE;
                    break;
                }
            }
            if (!found)
                ret_val = evaluate_rate_expression(ever_ptr->code, ev_p);
        }/* C'e' una lista di when */
        else {
            /* C'e' solo ever */
            ret_val = evaluate_rate_expression(ptr->code, ev_p);
        }/* C'e' solo ever */
    }/* Transizione con rate dipendente dall'istanza */
//finish:
    return (ret_val);
}/* End get_instance_rate */
