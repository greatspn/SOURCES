#include "multiset-adt.h"
#include "assignment.h"


extern list gListSubClasses;

status eval_term(TermPTR t, AssignmentPTR gamma, multisetPTR *p_MS);
status eval_el_tuple(ElTuplePTR e, AssignmentPTR gamma, multisetPTR *p_MS) ;
status eval_tuple(TuplePTR t, AssignmentPTR gamma, multisetPTR *p_MS);
status eval_operand(operandPTR o, AssignmentPTR gamma, multisetPTR *p_MS);
status eval_arc(arcPTR a, AssignmentPTR gamma, multisetPTR *p_MS);
status evalMrkTuple(MrkTuplePTR m, multisetPTR *p_MS);
status evalMrkOperand(mrk_operandPTR mrk_o, multisetPTR *p_MS);
status evalMarking(markPTR m, multisetPTR *p_MS);
bool eval_guard(guardPTR g, AssignmentPTR gamma);
bool eval_guard_expr(guard_exprPTR g, AssignmentPTR gamma);
