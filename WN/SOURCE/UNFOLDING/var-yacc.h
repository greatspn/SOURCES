/*************** var-yacc.h *******************/


#define NUM_DIGIT_TKN 5
#define DIM_STR_TKN 10
#define NUM_DIGITS_NUM 5

#ifdef 	LISP_OBJ_YACC
char *YACCobj_name;
/* variabili globali usate anche in altri moduli */
ClassObjPTR YACCparsedClass;
DomainObjPTR YACCparsedDomain;
guard_exprPTR YACCparsedGuard;
markPTR YACCparsedMarking;
arcPTR YACCparsedArc;
list YACCParsedVarList;       /* List of VariableTYPE */
list gListClasses;            /* List of ClassObjTYPE */
list gListSubClasses;            /* List of ClassObjTYPE */
#else
extern char *YACCobj_name;
extern ClassObjPTR YACCparsedClass;
extern DomainObjPTR YACCparsedDomain;
// 	char *YACCparsedMarking;
extern guard_exprPTR YACCparsedGuard;
extern markPTR YACCparsedMarking;
extern arcPTR YACCparsedArc;
extern list YACCParsedVarList;
extern list gListClasses;
extern list gListSubClasses;
extern int yyparse();
#endif

