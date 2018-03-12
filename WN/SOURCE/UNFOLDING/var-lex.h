/****************** var-lex.h ******************/

#ifdef LISP_OBJ_LEX
char *LEXtoParsifyString;
#else
extern char *LEXtoParsifyString;
extern int yylex(void);

#endif

