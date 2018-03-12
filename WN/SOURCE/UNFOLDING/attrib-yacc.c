/************************ attrib-yacc.c *******************************/
#include <stdlib.h>	/* per free()*/

#include "attrib-yacc.h"


TermPTR NewTerm(int coef, enum TipiTerm type, ClassObjPTR c, char *str) {
    TermPTR t;

    t = (TermPTR)Ecalloc(1, sizeof(TermTYPE));
    t->str = Estrdup(str);
    t->type = type;
    t->class = c;
    t->coef = coef;
    return (t);
}

char *PrintTerm(TermPTR t) {
    char *string;

    string = (char *) Ecalloc(100, sizeof(char));
    switch (t->type) {
    case ID_TYPE: {
        sprintf(string, "%d %s", t->coef, t->str);
        break;
    }
    case SUCC_TYPE: {
        sprintf(string, "%d !%s", t->coef, t->str);
        break;
    }

    case PRED_TYPE: {
        sprintf(string, "%d ^%s", t->coef, t->str);
        break;
    }

    case S_TYPE: {
        sprintf(string, "%d S %s", t->coef, t->str);
        break;
    }

    case S_SUBCLASS_TYPE: {
        sprintf(string, "%d S %s", t->coef, t->str);
        break;
    }

    case D_TYPE: {
        sprintf(string, "%d D(%s)", t->coef, t->str);
        break;
    }

    case D_SUBCLASS_TYPE: {
        sprintf(string, "%s", t->str);
        break;
    }
    default:
        Error(UNKN_TOKEN_ERR, "PrintTerm", NULL);
    }
    return (string);
}

guardPTR NewGuard(enum Tipi_Relaz type, TermPTR L, TermPTR R) {
    guardPTR g;

    g = (guardPTR)Emalloc(sizeof(guardTYPE));
    g->Type = type;
    g->L = L;
    g->R = R;
    return (g);
}

char *PrintGuard(guardPTR g) {
    char *string;

    string = (char *) Ecalloc(350, sizeof(char));
    if (g->Type == R_EQUAL)
        sprintf(string, "%s = %s", PrintTerm(g->L), PrintTerm(g->R));
    else
        sprintf(string, "%s <> %s", PrintTerm(g->L), PrintTerm(g->R));
    return (string);
}

guard_exprPTR NewGuardExpr(enum Tipi_Oper_Log type) {
    guard_exprPTR g;

    g = (guard_exprPTR)Emalloc(sizeof(guard_exprTYPE));
    g->Type = type;

    switch (type) {
    case NO_OP: {
#if DEBUG_UNFOLD
        printf("NewGuardExpr NO_OP %d\n", type);
#endif
        break;
    }
    case L_OR: {
#if DEBUG_UNFOLD
        printf("OR%d\n", type);
#endif
        break;
    }
    case L_AND: {
#if DEBUG_UNFOLD
        printf("AND%d\n", type);
#endif
        break;
    }
    default:
        Error(UNKN_TOKEN_ERR, "PrintGuard", NULL);


    }

    return (g);
}

char *PrintGuardExpr(guard_exprPTR g) {

    char *string;

    string = (char *) Ecalloc(350, sizeof(char));
    switch (g->Type) {
    case NO_OP: {
        sprintf(string, "%s", PrintGuard((g->Val).term));
        break;
    }
    case L_OR: {
        sprintf(string, "%s OR %s", PrintGuardExpr((g->Val).Bi.L), PrintGuardExpr((g->Val).Bi.R));
        break;
    }
    case L_AND: {
        sprintf(string, "%s AND %s", PrintGuardExpr((g->Val).Bi.L), PrintGuardExpr((g->Val).Bi.R));
        break;
    }
    default:
        Error(UNKN_TOKEN_ERR, "PrintGuardExpr", NULL);

    }
    return (string);
}




ElTuplePTR NewElTuple(TermPTR term_list) {
    ElTuplePTR p;

    p = (ElTuplePTR)Ecalloc(1, sizeof(ElTupleTYPE));
    p->term_list = (generic_ptr) term_list;
    return (p);
}


char *PrintElTuple(ElTuplePTR p) {
    list curr = NULL;
    char *string;

    string = (char *) Ecalloc(350, sizeof(char));
    while ((curr = list_iterator(p->term_list, curr)) != NULL)
        sprintf(string, "%s %s", string, PrintTerm((TermPTR) DATA(curr)));
    return (string);
}


TuplePTR NewTuple(ElTuplePTR el_list) {
    TuplePTR p;

    p = (TuplePTR)Ecalloc(1, sizeof(TupleTYPE));
    p->el_list = (generic_ptr) el_list;
    return (p);
}

char *PrintTuple(TuplePTR t) {
    list curr = NULL;
    char *string;

    string = (char *) Ecalloc(200, sizeof(char));
    while ((curr = list_iterator(t->el_list, curr)) != NULL)
//     if(curr == t->el_list)
//       sprintf(string,"%s", PrintElTuple( (ElTuplePTR) DATA(curr)));
//     else
        sprintf(string, "%s, %s", string, PrintElTuple((ElTuplePTR) DATA(curr)));
    return (string);
}



operandPTR NewOperand(int coef, guard_exprPTR guard, TuplePTR tuple) {
    operandPTR o;

    o = (operandPTR)Ecalloc(1, sizeof(operandTYPE));
    o->coef = coef;
    o->guard = guard;
    o->tuple = tuple;
    return (o);
}

char *PrintOperand(operandPTR o) {
    char *string;

    string = (char *) Ecalloc(200, sizeof(char));
    sprintf(string, "%+d [] <%s>", o->coef, PrintTuple(o->tuple));
    return (string);
}


arcPTR NewArcExpression(DomainObjPTR domain, list operand_list) {
    arcPTR a;

    a = (arcPTR)Ecalloc(1, sizeof(arcTYPE));
    a->domain = domain;
    a->operand = operand_list;
    return (a);
}

char *PrintArcExpression(arcPTR a) {
    list curr = NULL;
    char *string;

    string = (char *) Ecalloc(200, sizeof(char));
    while ((curr = list_iterator(a->operand, curr)) != NULL)
        sprintf(string, "%s %s", string, PrintOperand((operandPTR) DATA(curr)));
    return (string);
}

MrkTuplePTR NewMrkTuple(list term_list) {
    MrkTuplePTR p;

    p = (MrkTuplePTR)Ecalloc(1, sizeof(MrkTupleTYPE));
    p->term_list = term_list;
    return (p);
}

char *PrintMrkTuple(MrkTuplePTR mrkt) {
    list curr = NULL;
    char *string;

    string = (char *) Ecalloc(200, sizeof(char));
    while ((curr = list_iterator(mrkt->term_list, curr)) != NULL)
        sprintf(string, "%s, %s", string, PrintTerm((TermPTR) DATA(curr)));
    return (string);
}


mrk_operandPTR NewMrkOperand(int coef, MrkTuplePTR mrk_tuple) {
    mrk_operandPTR o;

    o = (mrk_operandPTR)Ecalloc(1, sizeof(mrk_operandTYPE));
    o->coef = coef;
    o->mrk_tuple = mrk_tuple;
    return (o);
}

char *PrintMrkOperand(mrk_operandPTR mrk_o) {
    char *string;

    string = (char *) Ecalloc(200, sizeof(char));
    sprintf(string, "%+d<%s>", mrk_o->coef, PrintMrkTuple(mrk_o->mrk_tuple));
    return (string);
}


markPTR NewMarking(char *name, list operand_list) {
    markPTR m;

    m = (markPTR)Ecalloc(1, sizeof(markTYPE));
    m->name = (char *) Estrdup(name);
    m->mrk_operand = operand_list;
    return (m);
}

char *PrintMarking(markPTR m) {
    list curr = NULL;
    char *string;

    string = (char *) Ecalloc(200, sizeof(char));
    while ((curr = list_iterator(m->mrk_operand, curr)) != NULL)
        sprintf(string, "%s %s", string, PrintMrkOperand((mrk_operandPTR) DATA(curr)));
    return (string);
}


VariablePTR NewVar(char *str, ClassObjPTR class) {
    VariablePTR  v;
    ClassObjPTR  c;
    list curr = NULL;

    v = (VariablePTR)Ecalloc(1, sizeof(VariableTYPE));
    v->name = Estrdup(str);
    v->class = class;

#if DEBUG_UNFOLD
    printf("Var %s\n", v->name);
    while ((curr = list_iterator(class->sub_classes, curr)) != NULL) {
        c = (ClassObjPTR) DATA(curr);
        printf(" New Var Min %d Max %d\n", c->min_idx, c->max_idx);
    }
#endif

    return (v);
}



bool CmpVarName(char *name, VariablePTR v) {

    if (strcmp(name, v->name) == 0)
        return (TRUE);
    else
        return (FALSE);
}



