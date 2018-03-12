 %{
  	#include <stdio.h>
	#include <string.h>
	
	#define LISP_OBJ_YACC	
	#include "attrib-yacc.h"
	
	#include "var-lex.h"
	#include "var-yacc.h"
	
	list backpacking = NULL;
	list curr = NULL;
	bool parsedGuard;	
	
	void yyerror(const char*);
%}

%union
{
	int num_int;
	DomainObjPTR domain;
	char *String;
	enum ClassTypes classtype;
	struct {
		list elements;
		int num_el;
	       } tmp;
	ClassObjPTR class;
	guard_exprPTR guard_e;       
	guardPTR guard;
	enum Tipi_Relaz eq;
	TermPTR term;	
	ElTuplePTR el_tuple;
	TuplePTR tuple;
	arcPTR arc;
	char c;
	markPTR mrk;	
	MrkTuplePTR mt;
}


%token O US CO MA FU OBRCK CBRCK MINUS PLUS COMMA
%token LESSER GREATER OPAR CPAR COLON VBAR S MOLT DIV CARET
%token OR AND NOT D EQUAL POINT ESCL MD PD TD WHEN EVER
%token SEMICOLON REALNUM DIESIS ID OBRCS CBRCS FLUSH NEWLINE

%token <num_int> NUM
%token <String> STRING

%type <domain> placedom
%type <classtype> prefix
%type <tmp> namelist static objects
%type <class> colors
%type <guard_e> guard_expr, guard_expr_a, guard_expr_b, opzguardexpr, transdom
%type <guard> guard
%type <eq> eqsign
%type <term> xterm, nterm, pterm ,sterm, gdterm, gxterm, gnterm, gpterm, gterm, term
%type <num_int> opzcoef
%type <c> plusop
%type <el_tuple> kernel
%type <tuple> funzseq 
%type <arc> funlist function
%type <mrk> marking objmark 
%type <mt> oblist 

%%
grammar   : CO
	 colors
	{  
	   YACCparsedClass = $2;         
	}
	   | FU 
	   function
	{
	   YACCparsedArc = $2;
	} 
	   | PD 
	     placedom
	{  
	   reverse(&($2->class_list));
	   YACCparsedDomain = $2;
	   YACCparsedDomain->place_name = YACCobj_name;
	}	   
	   | MA  marking
	{
	  YACCparsedMarking = $2;
	}  
	   | TD
	{
	   parsedGuard = TRUE;
	}   
	   transdom
	{
	   YACCparsedGuard = $3;
	   parsedGuard = FALSE;
	}	   
	   ;
	   
placedom   : placedom COMMA STRING
	{
	   list l;
	    
	   init_list(&l);  	   
	   if((find_key(gListClasses, (generic_ptr) $3 , CmpClassName, &l))==OK)
	   {
	     head_insert(&($$->class_list), DATA(l));
	     $$->num_el = $1->num_el+1;
	   }
	   else
	     Error(UNKN_CLASS_ERR, "yyparse", $3);	   
	
	}
	   | STRING
	{
	   list l;
	   
	   init_list(&l);	   	   
	   $$ = NewDomain(YACCobj_name, NULL, 0);
	   
	   if((find_key(gListClasses, (generic_ptr) $1 , CmpClassName, &l))==OK)
	   {
	     head_insert(&($$->class_list), DATA(l));
	     $$->num_el=1;
	   }
	   else
	     Error(UNKN_CLASS_ERR, "yyparse", $1);
	}
	   ;


transdom   : OBRCK
	     guard_expr
	     CBRCK
	{
	 $$ = $2;
	}
	   ;
	   
	   
	   
	   
colors	   : prefix
             namelist
	{
	 reverse(&($2.elements));
	 $$ = NewClass(YACCobj_name, $1, $2.elements, NULL, $2.num_el);
	}
	   | static 
	{
	 reverse(&($1.elements));
	 $$ = NewClass(YACCobj_name, SUBCLASS_TYPE, NULL, $1.elements, $1.num_el);
	 
	}
	   ;

prefix	   : O
	{
	 $$ = ORD_CLASS_TYPE;
	}
	   | US 
	{
	 $$ = UNORD_CLASS_TYPE;
	}
	   ;


namelist   : namelist COMMA STRING
	{ 
	  Class_elementPTR e;
	  
	  e= NewClassElement($3);
	  head_insert(&($1.elements), (generic_ptr) e);
	  $$.elements = $1.elements;
	  $$.num_el = $1.num_el+1;
	}
	   | STRING
	{ 
	  Class_elementPTR e;
	  
	  e= NewClassElement($1);
	  init_list(&($$.elements));
	  head_insert(&($$.elements), (generic_ptr) e);
	  $$.num_el = 1;
	}
 

static	   : 
	   OBRCS objects CBRCS
	{
	 $$ = $2;
	}
	  | STRING
	   OBRCS NUM 
	   MINUS NUM CBRCS
	{
	 Class_elementPTR e;
	 char *string;
	 int i;
	 
	 $$.num_el = 0;
	 init_list(&($$.elements));
	 for(i = $3; i <= $5 ; i++)
	 {
	  string = (char *)Emalloc(strlen($1)+ NUM_DIGIT_TKN +1);
	  string = Estrdup($1);
	  sprintf(string, "%s%d", string, i);	  
	  e = NewClassElement(string);
/*	  printf("\nName %s \n", e->name); */
	  head_insert(&($$.elements), (generic_ptr) e);
	  $$.num_el++;
	 }
	}
	   ;

	   
objects   : objects COMMA STRING
	{ 
	  Class_elementPTR e;
	  
	  e= NewClassElement($3);
	  head_insert(&($1.elements), (generic_ptr) e);
	  $$.elements = $1.elements;
	  $$.num_el = $1.num_el+1;
	}
	   | STRING
	{ 
	  Class_elementPTR e;
	  
	  e= NewClassElement($1);
	  init_list(&($$.elements));
	  head_insert(&($$.elements), (generic_ptr) e);
	  $$.num_el = 1;
	}


marking    : objmark
	{
	  reverse(&($1->mrk_operand));
	  $$ = $1;
	}
	   | 
	{
	   if((curr = list_iterator(YACCparsedDomain->class_list, curr))==NULL)
	     printf("Error\n");
	}
	     sterm
	{
	  list l=NULL;	  
	  mrk_operandPTR o;

	  head_insert(&l, (generic_ptr) $1);
	  o = NewMrkOperand(1, NewMrkTuple(l));  
	  l = NULL;
	  head_insert(&l, (generic_ptr) o);
	  $$ = NewMarking(YACCobj_name, l);
	}	   
	   ;

objmark	   : opzcoef 
        {
	  curr = NULL;
	} 
	  LESSER oblist GREATER
	{ 
	  list l=NULL;	
	  mrk_operandPTR o;
	  
	  reverse(&($4->term_list));
	  o = NewMrkOperand($1, $4 );
	  head_insert(&l, (generic_ptr) o);
	  $$ = NewMarking(YACCobj_name, l);
	}
	   | objmark plusop opzcoef 
	{
	  curr = NULL;
	}
	     LESSER oblist GREATER
	{
	  mrk_operandPTR o;
	  
	  reverse(&($6->term_list));
	  o = NewMrkOperand($3, $6 );
	  if($2=='-')
	   o->coef = -(o->coef);
	  head_insert(&($1->mrk_operand), (generic_ptr) o);
	  $$ =$1;
	}
	   ;
	   
oblist	   : oblist COMMA
	{
	  if((curr = list_iterator(YACCparsedDomain->class_list, curr))==NULL)
	    printf("Error\n");	
	}	
	     sterm
	{	  
	  head_insert(&($1->term_list), (generic_ptr) $4);
	  $$ = $1;
	}
	|
	{
	  if((curr = list_iterator(YACCparsedDomain->class_list, curr))==NULL)
	    printf("Error\n");	
	}
	     sterm
	{
	  list l=NULL;
	
	  head_insert(&l, (generic_ptr) $2);	  
	  $$ = NewMrkTuple(l);	  
	}
	   ;

function   : 
	funlist
	{ 
	  list curr = NULL;
	  list l = NULL;
	  TermPTR t;
	  VariablePTR  v;
	  
	  while ( (curr = list_iterator(backpacking, curr)) != NULL ){
	   t = (TermPTR) DATA(curr);	   
	   if((find_key(YACCParsedVarList, (generic_ptr) t->str, CmpVarName, &l)) == OK){
	    v = (VariablePTR) DATA(l);
	    t->class = v->class;	   
	   }
	   else  
	    Error(UNKN_VAR_ERR, "yyparse", t->str);
	  }
	  destroy(&backpacking, NULL);
	  reverse(&($1->operand));	
	  $$ = $1;
	}
	   ;
	
plusop	   : PLUS
	{
	 $$ = '+';
	}
	   | MINUS
	{
	 $$ = '-';
	}
	   ;	   
	   
funlist    : opzcoef opzguardexpr LESSER 
        {
	 curr = NULL;
	}
	funzseq GREATER
	{
	 operandPTR o_ptr;
	 
	 reverse(&($5->el_list));	 	
	 o_ptr = NewOperand($1, $2, $5);	 
	 $$ = NewArcExpression(YACCparsedDomain, NULL );
	 head_insert(&($$->operand),(generic_ptr) o_ptr);
	}
	
	   | funlist plusop opzcoef opzguardexpr LESSER 
	{
	 curr = NULL;
	}  	   
	   funzseq GREATER
	{
	 operandPTR o_ptr;
	 
	 if($2=='-')
	     $3 = -$3;
	 reverse(&($7->el_list)); 
	 o_ptr = NewOperand($3, $4, $7);
	 head_insert(&($1->operand),(generic_ptr) o_ptr);
	 $$ = $1;
	}
	   ;	
	
	
opzguardexpr   : 
        {
         parsedGuard = TRUE;
        }
            OBRCK 
	    guard_expr
	    CBRCK
	{
	 $$ = $3;
	 parsedGuard = FALSE;
	}
           |
	{
	 $$ = NULL;
	}
	   ;
	   
	   
funzseq    :  
	{ 
	   if((curr = list_iterator(YACCparsedDomain->class_list, curr))==NULL)
	     printf("Error\n");
	}
	   kernel
	{
	  
	   $$ = NewTuple(NULL);
	   reverse(&($2->term_list));   
	   head_insert(&($$->el_list), (generic_ptr) $2);	   	   
	}
	   | funzseq COMMA 
	{
	   if((curr = list_iterator(YACCparsedDomain->class_list, curr))==NULL)
	     printf("Error\n");
	}	   
	   kernel
	{
	   reverse(&($4->term_list));   
	   head_insert(&($1->el_list), (generic_ptr) $4);
	   $$=$1;
	}
	   ;
	   
kernel	   : term
	{  
	   $$ = NewElTuple(NULL);
	   
	   // printf("Term: %s \n", PrintTerm($1));
	   init_list(&($$->term_list));	   
	   head_insert(&($$->term_list), (generic_ptr) $1);
	   // printf("El Tupla: %s \n", PrintElTuple($$));
	}
	   | kernel plusop term
	{
	   if($2=='-')
	     $3->coef = -$3->coef; 
	   head_insert(&($1->term_list), (generic_ptr) $3);
	   $$=$1;
	}
	   ;
	   
	   
guard_expr:   guard_expr
	      OR 
	      guard_expr_a
	{
	 $$ = NewGuardExpr(L_OR);
	 $$->Val.Bi.L = $1;
	 $$->Val.Bi.R = $3;     
	}
	   |  guard_expr_a
	{
	 $$ = $1;
	}
	   ;
	   
guard_expr_a :guard_expr_a
	      AND  
	      guard_expr_b
	{
	 $$ = NewGuardExpr(L_AND);
	 $$->Val.Bi.L = $1;
	 $$->Val.Bi.R = $3;     
	}	
	   | guard_expr_b
	{
	 $$ = $1;
	}
	   ;
	   
guard_expr_b: OPAR 
	      guard_expr
	      CPAR
	{
	 $$ = $2;
	}
	    | guard
	{
	 $$ = NewGuardExpr(NO_OP); 
	 $$->Val.term = $1;
         
	}

	  ;

guard     : gterm eqsign gterm
        {
	   $$ = NewGuard($2, $1, $3);
	 #if DEBUG_UNFOLD
	   printf("La guardia %s\n", PrintGuard($$));
	 #endif
        }
	  ;
	  

	  
term     : nterm
	{
	 $$ = $1;
	}
	 | pterm
	{
	 $$ = $1;
	}
	 | xterm
	{
	 $$ = $1;
	}
	 | sterm
	{
	 $$ = $1;
	} 
           ;	  
	  
gterm    : gdterm
	{
	 $$ = $1;
	}
	 | gnterm
	{
	 $$ = $1;
	}
	 | gpterm
	{
	 $$ = $1;
	}
	 | gxterm
	{
	 $$ = $1;
	}	 
           ;

	  
eqsign    : EQUAL
	{
	 $$ = R_EQUAL;
	}
	  | LESSER
	    GREATER
	{
	 $$ = R_NOT_EQUAL;
	}
           ;

	   
gdterm      : opzcoef D OPAR STRING CPAR
	{	  
	  list l=NULL;
	  VariablePTR v;
	  
	  if((find_key(YACCParsedVarList, (generic_ptr) $4, CmpVarName, &l))==OK){
	    v = (VariablePTR) DATA(l);
	    $$ = NewTerm($1, D_TYPE, v->class, $4);
	  } 
	  else
	  {	   	    
            $$ = NewTerm($1, D_TYPE, NULL, $4);
	    head_insert(&backpacking, (generic_ptr) $$);
	  }
 	}
           ;

	   
sterm	   : opzcoef S NUM
	{
 	 printf("Warning non implemented function\n");
	 $$ = NULL;
 	}
	   | opzcoef S STRING
	{
	 ClassObjPTR  cl;
	 list l;
	 
	 init_list(&l);	   
	 if((find_key(gListSubClasses, (generic_ptr) $3 , CmpClassName, &l))==OK)
	   cl = (ClassObjPTR) DATA(l);
	 else
	   Error(UNKN_CLASS_ERR, "yyparse", $3);
	 cl = (ClassObjPTR) DATA(curr);	   
	 $$ = NewTerm($1, S_SUBCLASS_TYPE, cl, $3);	 

	}
	   | opzcoef S
	{	 
	 ClassObjPTR  cl;
	 
	 cl = (ClassObjPTR) DATA(curr);
	 $$ = NewTerm($1, S_TYPE, cl, ""); 
	 
	}
	   ;
	   

nterm	   : opzcoef ESCL STRING
	{
	 VariablePTR  v;
	 ClassObjPTR  cl;
	 list l = NULL;
	 	   
	 cl = (ClassObjPTR) DATA(curr); 
	 $$ = NewTerm($1, SUCC_TYPE, cl, $3); 
	 if((find_key(YACCParsedVarList, (generic_ptr) $3, CmpVarName, &l))==ERROR) { 	   
	   v = NewVar($3, cl);
           head_insert(&YACCParsedVarList, (generic_ptr) v); 
	 }
	}
	   ;
	   
gnterm	   : opzcoef ESCL STRING
	{
         VariablePTR  v;
         list l = NULL; 
	 	
	 if((find_key(YACCParsedVarList, (generic_ptr) $3, CmpVarName, &l))==ERROR) {
	   $$ = NewTerm($1, SUCC_TYPE, NULL, $3);	 	   
           head_insert(&backpacking, (generic_ptr) $$); 
	 }
	 else{	   
	   v = (VariablePTR) DATA(l);
	   $$ = NewTerm($1, SUCC_TYPE, v->class, $3);	   
	 }

	}
	   ;
	   
	   
	   
pterm	   : opzcoef CARET STRING
	{
	 VariablePTR  v;
	 ClassObjPTR  cl;
	 list l = NULL;
	 	   
	 cl = (ClassObjPTR) DATA(curr); 
	 $$ = NewTerm($1, PRED_TYPE, cl, $3); 
	 if((find_key(YACCParsedVarList, (generic_ptr) $3, CmpVarName, &l))==ERROR) { 	   
	   v = NewVar($3, cl);           
           head_insert(&YACCParsedVarList, (generic_ptr) v); 
	 }
	}
	   ;
	   
gpterm	   : opzcoef CARET STRING
	{
         VariablePTR  v;
         list l = NULL;	
	 
	 if((find_key(YACCParsedVarList, (generic_ptr) $3, CmpVarName, &l))==ERROR) {
	   $$ = NewTerm($1, PRED_TYPE, NULL, $3);	 	   
           head_insert(&backpacking, (generic_ptr) $$); 
	 }
	 else{	   
	   v = (VariablePTR) DATA(l);
	   $$ = NewTerm($1, PRED_TYPE, v->class, $3);	   
	 }

	}
	   ;
	   
	   
xterm	   : opzcoef STRING
	{
	 VariablePTR  v;
	 ClassObjPTR  cl;
	 list l = NULL;
	 	   
	 cl = (ClassObjPTR) DATA(curr); 
	 $$ = NewTerm($1, ID_TYPE, cl, $2); 
	 if((find_key(YACCParsedVarList, (generic_ptr) $2, CmpVarName, &l))==ERROR) { 	   
	   v = NewVar($2, cl);           
           head_insert(&YACCParsedVarList, (generic_ptr) v);  	 
	   
	   
	 }
        }  
           ;

gxterm	   : opzcoef STRING
	{
         VariablePTR  v;
	 ClassObjPTR  c;
	 list l = NULL;
	 
	 if((find_key(YACCParsedVarList, (generic_ptr) $2, CmpVarName, &l))==ERROR) {
	   if((find_key(gListSubClasses, (generic_ptr) $2, CmpVarName, &l))==ERROR) {
	     $$ = NewTerm($1, ID_TYPE, NULL, $2);	 	   
             head_insert(&backpacking, (generic_ptr) $$); 
	   }
	   else
	   {
	     c = (ClassObjPTR) DATA(l);
	     $$ = NewTerm($1, ID_TYPE, c, $2);
	   }  
	 }
	 else{	   
	   v = (VariablePTR) DATA(l);
	   $$ = NewTerm($1, ID_TYPE, v->class, $2);	   
	 }
	
        }  
           ;
	   
	   
opzcoef	   : NUM
	{
	 $$ = $1;
	}
	   | 
	{
	 $$ = 1;
	}
	   ;
	   

%%
    
void yyerror(const char* error)
{
 fprintf(stdout,"Syntax error while parsing a color object\n");
 exit(1);
}
