#include "evaluate.h"

extern list  gListSubClasses;

status eval_term(TermPTR t, AssignmentPTR gamma, multisetPTR *p_MS) {
    Assignment_el_PTR a;
    list l;
    DomainObjPTR d;
    ClassObjPTR cl;
    int pos;


//  printf("Termine da valutare: %s \n",PrintTerm(t));
    switch (t->type) {
    case SUCC_TYPE: {

        if (evaluate_variable(gamma, t->str, &a) == ERROR)
            printf("Error\n");

        init_list(&l);
        head_insert(&l, (generic_ptr) VAR(a)->class);
        d = NewDomain("tmp", l, 1);
//       PrintDomain(d);
        create_multiset(d, p_MS);


        if (t->class->type == ORD_CLASS_TYPE) {
            pos = (INDEX(a) + 1) % t->class->num_el;
// 	printf("SUCC POS %d\n", pos);
        }
        else
            Error(SUCCESSOR_FUNCTION_ERR, "eval_term", t->str);
        init_list(&l);
        head_insert(&l, (generic_ptr) &pos);
        VALUE(SET(*p_MS)[position(l , *p_MS)]) = t->coef;
//    printf("Ms term \n");
//    printMultiset(*p_MS);
        break;
    }
    case PRED_TYPE: {

        if (evaluate_variable(gamma, t->str, &a) == ERROR)
            printf("BEE\n");

        init_list(&l);
        head_insert(&l, (generic_ptr) VAR(a)->class);
        d = NewDomain("tmp", l, 1);
//       PrintDomain(d);
        create_multiset(d, p_MS);


        if (t->class->type == ORD_CLASS_TYPE) {
            pos = (t->class->num_el + INDEX(a) - 1) % t->class->num_el;
// 	printf("PRED POS %d\n", pos);

        }
        else
            Error(PREDECESSOR_FUNCTION_ERR, "eval_term", t->str);
        init_list(&l);
        head_insert(&l, (generic_ptr) &pos);
        VALUE(SET(*p_MS)[position(l , *p_MS)]) = t->coef;
//    printf("Ms term \n");
//    printMultiset(*p_MS);
        break;
    }
    case ID_TYPE: {
        if (evaluate_variable(gamma, t->str, &a) == ERROR)
            printf("BEE\n");


        init_list(&l);
        head_insert(&l, (generic_ptr) VAR(a)->class);
        d = NewDomain("tmp", l, 1);

//       PrintDomain(d);

        create_multiset(d, p_MS);

        pos = INDEX(a);
        init_list(&l);
        head_insert(&l, (generic_ptr) &pos);
        VALUE(SET(*p_MS)[position(l , *p_MS)]) = t->coef;
//     printf("Ms term \n");
//     printMultiset(*p_MS);
        break;

    }
    case S_TYPE: {
#if DEBUG_UNFOLD
        printf("S_Class\n");
#endif
        init_list(&l);
        head_insert(&l, (generic_ptr) t->class);
        d = NewDomain("tmp", l, 1);
//       PrintDomain(d);
        create_multiset(d, p_MS);
        for (pos = t->class->min_idx; pos < t->class->max_idx; VALUE(SET(*p_MS)[pos++]) = t->coef);
        break;
    }
    case S_SUBCLASS_TYPE: {
#if DEBUG_UNFOLD
        printf("S_SubClass\n");
#endif
        init_list(&l);
        head_insert(&l, (generic_ptr) t->class);
        d = NewDomain("tmp", l, 1);
        create_multiset(d, p_MS);

        init_list(&l);
        if ((find_key(gListSubClasses, (generic_ptr) t->str , CmpClassName, &l)) == OK)
            cl = (ClassObjPTR) DATA(l);
        else
            Error(UNKN_CLASS_ERR, "eval_term", t->str);

//       printf("Eval Index SubCl %d : %d \n",cl->min_idx, cl->max_idx);
        for (pos = cl->min_idx; pos < cl->max_idx; VALUE(SET(*p_MS)[pos++]) = t->coef);
        break;
    }
    default:
        Error(UNKN_SUBCL_ERR, "eval_term", NULL);

    }
    return (OK);
}

status eval_el_tuple(ElTuplePTR e, AssignmentPTR gamma, multisetPTR *p_MS) {
    list curr = NULL;
    multisetPTR p_prec_MS, p_curr_MS;



    curr = list_iterator(e->term_list, curr);
    if (curr == NULL)
        return (ERROR);
    else {
        alloc_multiset(&p_prec_MS);
        alloc_multiset(&p_curr_MS);
        eval_term((TermPTR) DATA(curr), gamma, &p_prec_MS);
        *p_MS = p_prec_MS;
        /*       printf("Ms prec  eval_el_tuple \n");
               printMultiset(*p_MS);*/
        while ((curr = list_iterator(e->term_list, curr)) != NULL) {
            eval_term((TermPTR) DATA(curr), gamma, &p_curr_MS);

//        printf("Ms curr eval_eltuple \n");
//        printMultiset(p_curr_MS);

            create_multiset(DOM(p_curr_MS), p_MS);
            sum(p_prec_MS, p_curr_MS, *p_MS);
            destroy_multiset(p_curr_MS);
            p_prec_MS = *p_MS;

//        printf("Ms SUM  eval_el_tuple \n");
//        printMultiset(p_prec_MS);


//       destroy_multiset(p_curr_MS);
        }
        return (OK);
    }


}

status eval_tuple(TuplePTR t, AssignmentPTR gamma, multisetPTR *p_MS) {
    list curr = NULL;
    multisetPTR p_prec_MS, p_curr_MS;


    curr = list_iterator(t->el_list, curr);
    if (curr == NULL)
        return (ERROR);
    else {

        alloc_multiset(&p_prec_MS);
        alloc_multiset(&p_curr_MS);

        eval_el_tuple((ElTuplePTR) DATA(curr), gamma, &p_prec_MS);
        *p_MS = p_prec_MS;

//       printf("Ms  prec eval tuple\n");
//       printMultiset(p_prec_MS);


        while ((curr = list_iterator(curr, curr)) != NULL) {
            eval_el_tuple((ElTuplePTR) DATA(curr), gamma, &p_curr_MS);

            /*      printf("Ms curr eval tuple\n");
                  printMultiset(p_curr_MS); */

            cartesian_product(p_prec_MS, p_curr_MS, p_MS);


//       printf("Ms  produc 2 evaltuple\n");
//       printMultiset(p_MS);

            destroy_multiset(p_curr_MS);
            p_prec_MS = *p_MS;

            /*      printf("Ms PRODUCT eval tuple\n");
                  printMultiset(*p_MS); */

//       destroy_multiset(p_MS);
//       destroy_multiset(p_curr_MS);
//       printf("2Tupla da valutare: %s \n",PrintTuple(t));



        }
    }

    /*      printf("Ms eval tuple\n");
          printMultiset(*p_MS); */
    return (OK);
}

status eval_operand(operandPTR o, AssignmentPTR gamma, multisetPTR *p_MS) {

    if (eval_guard_expr(o->guard, gamma) == TRUE) {  /* DA COMPLETARE PER FAR FUNZIONARE GUARDIE IN ESPRESSIONE  */
        alloc_multiset(p_MS);
//   printf("Operando da valutare: %s \n",PrintOperand(o));
        eval_tuple((TuplePTR) o->tuple, gamma, p_MS);
//   printf("Ms eval oper\n");
//   printMultiset(*p_MS);
        if (o->coef != 1)
            scalar_product(*p_MS, o->coef);
    }

    return (OK);
}


status eval_arc(arcPTR a, AssignmentPTR gamma, multisetPTR *p_MS) {
    list curr = NULL;
    multisetPTR p_prec_MS, p_curr_MS;
    int i;

    *p_MS = NULL;
//   printf("Arco da valutare: %s \n",PrintArcExpression(a));
    curr = list_iterator(a->operand, curr);
    if (curr == NULL)
        return (ERROR);
    else {
//     alloc_multiset(p_MS);
        /*    alloc_multiset(&p_prec_MS);
            alloc_multiset(&p_curr_MS);*/
        eval_operand((operandPTR) DATA(curr), gamma, p_MS);
        while ((curr = list_iterator(a->operand, curr)) != NULL) {
            eval_operand((operandPTR) DATA(curr), gamma, &p_curr_MS);
            if (p_curr_MS == NULL)
                create_multiset(a->domain, &p_curr_MS);
            p_prec_MS = *p_MS;
            create_multiset(DOM(p_curr_MS), p_MS);
            sum(p_prec_MS, p_curr_MS, *p_MS);
            destroy_multiset(p_curr_MS);
        }
    }

    if (*p_MS == NULL)
        create_multiset(a->domain, p_MS);

    for (i = 0; i < CARD(*p_MS); i++)
        STR_INDEX(SET(*p_MS)[i]) = index_elements(i, *p_MS);


#if DEBUG_UNFOLD
    printf("Result Arc\n");
    printMultiset(*p_MS);
#endif
    return (OK);
}


status evalMrkTuple(MrkTuplePTR m, multisetPTR *p_MS) {
    list curr = NULL;
    multisetPTR p_prec_MS, p_curr_MS;


    curr = list_iterator(m->term_list, curr);
    if (curr == NULL)
        return (ERROR);
    else {

        alloc_multiset(&p_prec_MS);
        alloc_multiset(&p_curr_MS);

        eval_term((TermPTR) DATA(curr), NULL, &p_prec_MS);
        *p_MS = p_prec_MS;

//       printf("Ms  prec eval tuple\n");
//       printMultiset(p_prec_MS);


        while ((curr = list_iterator(curr, curr)) != NULL) {
            eval_term((TermPTR) DATA(curr), NULL, &p_curr_MS);

            /*      printf("Ms curr eval tuple\n");
                  printMultiset(p_curr_MS); */

            cartesian_product(p_prec_MS, p_curr_MS, p_MS);


//       printf("Ms  produc 2 evaltuple\n");
//       printMultiset(p_MS);

            destroy_multiset(p_curr_MS);
            p_prec_MS = *p_MS;

            /*      printf("Ms PRODUCT eval tuple\n");
                  printMultiset(*p_MS); */

//       destroy_multiset(p_MS);
//       destroy_multiset(p_curr_MS);
//       printf("2Tupla da valutare: %s \n",PrintTuple(t));



        }
    }
    return (OK);
}

status evalMrkOperand(mrk_operandPTR mrk_o, multisetPTR *p_MS) {


    evalMrkTuple(mrk_o->mrk_tuple, p_MS);
    if (mrk_o->coef != 1)
        scalar_product(*p_MS, mrk_o->coef);

    return (OK);
}

status evalMarking(markPTR m, multisetPTR *p_MS) {
    list curr = NULL;
    multisetPTR p_prec_MS, p_curr_MS;
    int i;

//   printf("Arco da valutare: %s \n",PrintArcExpression(a));
    curr = list_iterator(m->mrk_operand, curr);
    if (curr == NULL)
        return (ERROR);
    else {
        alloc_multiset(p_MS);
        alloc_multiset(&p_prec_MS);
        alloc_multiset(&p_curr_MS);
        evalMrkOperand((mrk_operandPTR) DATA(curr), p_MS);
        while ((curr = list_iterator(m->mrk_operand, curr)) != NULL) {
            evalMrkOperand((mrk_operandPTR) DATA(curr), &p_curr_MS);
            p_prec_MS = *p_MS;
            create_multiset(DOM(p_curr_MS), p_MS);
            sum(p_prec_MS, p_curr_MS, *p_MS);
            destroy_multiset(p_curr_MS);
        }
    }

    for (i = 0; i < CARD(*p_MS); i++)
        STR_INDEX(SET(*p_MS)[i]) = index_elements(i, *p_MS);


#if DEBUG_UNFOLD
    printf("Result Marking\n");
    printMultiset(*p_MS);
#endif
    return (OK);
}



bool eval_guard(guardPTR g, AssignmentPTR gamma) {
    Assignment_el_PTR a, b;
    char *sub_class_a, *sub_class_b;
    list l = NULL;
    int ind_a, ind_b;
    ClassObjPTR c;

    if ((g->L->type == ID_TYPE || g->L->type == SUCC_TYPE || g->L->type == PRED_TYPE) &&
            (g->R->type == ID_TYPE || g->R->type == SUCC_TYPE || g->R->type == PRED_TYPE)) {
        evaluate_variable(gamma, g->L->str, &a);
        evaluate_variable(gamma, g->R->str, &b);

        switch (g->L->type) {
        case   ID_TYPE: {
            ind_a = INDEX(a);
            break;
        }
        case SUCC_TYPE: {
            ind_a  = (INDEX(a) + 1) % g->L->class->num_el;
            break;
        }
        case PRED_TYPE: {
            ind_a = (g->L->class->num_el + INDEX(a) - 1) % g->L->class->num_el;
            break;
        }

        }

        switch (g->R->type) {
        case   ID_TYPE: {
            ind_b = INDEX(b);
            break;
        }
        case SUCC_TYPE: {
            ind_b  = (INDEX(b) + 1) % g->R->class->num_el;
            break;
        }
        case PRED_TYPE: {
            ind_b = (g->R->class->num_el + INDEX(b) - 1) % g->R->class->num_el;
            break;
        }

        }

        switch (g->Type) {
        case R_EQUAL: {
            if (ind_a == ind_b)
                return (TRUE);
            else
                return (FALSE);
            break;
        }
        case R_NOT_EQUAL: {
            if (ind_a != ind_b)
                return (TRUE);
            else
                return (FALSE);
            break;
        }
        default :
            Error(UNKN_LISP_ERR, "eval_guard", NULL);
        }
    }
    else {


        if (g->L->type == D_TYPE) {
            list curr = NULL;

            evaluate_variable(gamma, g->L->str, &a);
            ind_a = INDEX(a);

            if ((find_key(g->L->class->sub_classes, (generic_ptr) &ind_a , inSubClass, &l)) == OK)
                sub_class_a = ((ClassObjPTR) DATA(l))->name;
            else
                Error(MISS_SUBCL_EL_ERR, "eval_guard", NULL);

        }
        else
            sub_class_a = g->L->str;

        if (g->R->type == D_TYPE) {
            evaluate_variable(gamma, g->R->str, &b);
            ind_b = INDEX(b);

            if ((find_key(c->sub_classes, (generic_ptr) &ind_b , inSubClass, &l)) == OK)
                sub_class_b = ((ClassObjPTR) DATA(l))->name;
            else
                Error(MISS_SUBCL_EL_ERR, "eval_guard", NULL);
        }
        else
            sub_class_b = g->R->str;


        switch (g->Type) {
        case R_EQUAL: {
            if (strcmp(sub_class_a, sub_class_b) == 0)
                return (TRUE);
            else
                return (FALSE);
            break;
        }
        case R_NOT_EQUAL: {
            if (strcmp(sub_class_a, sub_class_b) != 0)
                return (TRUE);
            else
                return (FALSE);
            break;
        }
        default :
            Error(UNKN_LISP_ERR, "eval_guard", NULL);
        }
    }
}


bool eval_guard_expr(guard_exprPTR g, AssignmentPTR gamma) {
    Assignment_el_PTR a, b;


    if (g == NULL)
        return (TRUE);
    switch (g->Type) {
    case NO_OP: {
        return (eval_guard((g->Val).term, gamma));
        break;
    }
    case L_AND: {
        return (eval_guard_expr(g->Val.Bi.L, gamma) && eval_guard_expr(g->Val.Bi.R, gamma));
        break;
    }
    case L_OR: {
        return (eval_guard_expr(g->Val.Bi.L, gamma) || eval_guard_expr(g->Val.Bi.R, gamma));
        break;
    }
    default :
        Error(UNKN_LISP_ERR, "eval_guard_expr", NULL);
    }

}

