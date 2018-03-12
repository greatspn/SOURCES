/*********************** attrib-yacc.h ***************/

#include "lista-adt.h"
#include "SWN-types.h"


// Term Type
enum TipiTerm {ID_TYPE, SUCC_TYPE, PRED_TYPE, S_TYPE, S_SUBCLASS_TYPE, D_TYPE, D_SUBCLASS_TYPE };

// Logic operator Type
enum Tipi_Oper_Log  {L_AND, L_OR, NO_OP};

// Relation Type
enum Tipi_Relaz {R_EQUAL, R_NOT_EQUAL};


// Term Type. Example: 2x, S, 2 S Cl
typedef struct term_tag {
    int 		coef;
    enum TipiTerm 	type;
    ClassObjPTR    class;
    char 		*str;
} TermTYPE;
typedef TermTYPE *TermPTR;


// Guard Type. Example: x=y, d(x)<>y
typedef struct guard_tag {
    enum Tipi_Relaz Type;
    TermPTR L;
    TermPTR R;
} guardTYPE;
typedef guardTYPE *guardPTR;


// Guard expression Type. Example: x=y, d(x)<>y OR x<>y, !x=y AND d(x)= Cl
typedef struct guard_expr_tag {
    enum Tipi_Oper_Log Type;
    union {
        guardPTR term;
        struct {
            struct guard_expr_tag *L;
            struct guard_expr_tag *R;
        } Bi;
    } Val;
} guard_exprTYPE;
typedef guard_exprTYPE *guard_exprPTR;

// Tuple element Type. Example: 2x+S, S-x, x+y
typedef struct ElTuple_tag {
    list term_list;		/* List of TermTYPE */
} ElTupleTYPE;
typedef ElTupleTYPE *ElTuplePTR;

// Tuple Type. Example: < 2x+S, S-x>, <x+y, k ,z>
typedef struct Tuple_tag {
    list el_list;           /* List of ElTupleTYPE */
} TupleTYPE;
typedef TupleTYPE *TuplePTR;


// Operand Type. Example: 4[d(x)<>y OR x<>y]< 2x+S, S-x>, <x+y, k ,z>
typedef struct operand_tag {
    int coef;
    guard_exprPTR guard;
    TuplePTR tuple;
} operandTYPE;
typedef operandTYPE *operandPTR;

// Tuple Type. Example: 4[d(x)<>y OR x<>y]< 2x+S, S-x> + 2 <x+y, k ,z>
typedef struct arc_tag {
    list operand;		/* List of operandTYPE */
    DomainObjPTR domain;
} arcTYPE;
typedef arcTYPE *arcPTR;


typedef struct variable_tag {
    char 		     *name;
    ClassObjPTR 	     class;
} VariableTYPE;
typedef VariableTYPE *VariablePTR;


typedef struct MrkTuple_tag {
    list term_list;           /* List of TermTYPE */
} MrkTupleTYPE;
typedef MrkTupleTYPE *MrkTuplePTR;

typedef struct mrk_operand_tag {
    int coef;
    MrkTuplePTR mrk_tuple;
} mrk_operandTYPE;
typedef mrk_operandTYPE *mrk_operandPTR;

typedef struct mark_tag {
    char *name;
    struct place_object *place;
    int type;
    union {
        generic_ptr mrk;
        list  mrk_operand;		/* List of mrk_operandTYPE */
    };
} markTYPE;
typedef markTYPE *markPTR;



TermPTR NewTerm(int coef, enum TipiTerm type, ClassObjPTR c, char *str);
char *PrintTerm(TermPTR t);
guardPTR NewGuard(enum Tipi_Relaz type, TermPTR L, TermPTR R);
char *PrintGuard(guardPTR g);
guard_exprPTR NewGuardExpr(enum Tipi_Oper_Log type);
char *PrintGuardExpr(guard_exprPTR g);
ElTuplePTR NewElTuple(TermPTR term_list);
char *PrintElTuple(ElTuplePTR p);
TuplePTR NewTuple(ElTuplePTR el_list);
char *PrintTuple(TuplePTR t);
operandPTR NewOperand(int coef, guard_exprPTR guard, TuplePTR tuple);
char *PrintOperand(operandPTR o);
arcPTR NewArcExpression(DomainObjPTR domain, list operand_list);
char *PrintArcExpression(arcPTR a);
MrkTuplePTR NewMrkTuple(list term_list);
char *PrintMrkTuple(MrkTuplePTR mrkt);
mrk_operandPTR NewMrkOperand(int coef, MrkTuplePTR mrk_tuple);
char *PrintMrkOperand(mrk_operandPTR mrk_o);
markPTR NewMarking(char *name, list operand_list);
char *PrintMarking(markPTR m);
VariablePTR NewVar(char *str, ClassObjPTR class);
bool CmpVarName(char *name, VariablePTR v);

