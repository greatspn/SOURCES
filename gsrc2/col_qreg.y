%{


#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

static int_val;

static int cur_dim, cur_card;
static int bypass = TRUE;

static char name[256];

#define EQUAL_TO 1
#define NOTEQUAL_TO 0

%}

%start gram

%token FUNCT MARK COL PRED
%token SEMICOLON
%token COMMA NO_COLOUR
%token PLUS MINUS
%token O_BRACE C_BRACE
%token O_BRACK C_BRACK
%token O_PAR C_PAR
%token LESS GREAT
%token COMMA COLON V_BAR
%token STRING CONST
%token ID
%token S_FUNCT X_FUNCT N_FUNCT
%token L_OR L_AND L_NOT
%token EQ_NEQ IN

%%

gram : FUNCT function C_PAR
	 {
	   unput(EOF);
	 }
     | MARK marking C_PAR
	 {
	   unput(EOF);
	 }
     | COL
	 {
	   bypass = TRUE;
	 }
       colour_dom C_PAR
	 {
	   end_dom();
	   unput(EOF);
	 }
     | PRED predicate C_PAR
	 {
	   unput(EOF);
	 }
     ;

colour_dom : colour_dom COMMA
	     colour_set
	       {
		 ++cur_dim;
		 cur_card += cur_set->card;
		 bypass = FALSE;
	       }
	   | colour_set
	       {
		 cur_dim = 1;
		 cur_card = cur_set->card;
		 add_dom('d');
	       }
	   | NO_COLOUR
	       {
		 add_dom('#');
		 cur_dim = 0;
	       }
	   ;

colour_set : class_list
               {
		 add_set('u');
	       }
	   | static_class
               {
		 add_set('s');
		 cur_set->bypass = TRUE;
	       }
	   | iof_string
	       {
		 if ( check_flag ) {
		   }
		 else {
		     add_set('?');
		     cur_set->class.str = ecalloc(strlen(name)+1,1);
		     sprintf(cur_set->class.str,name);
		   }
	       }
	   ;

class_list : class_list PLUS sc_term
	   | sc_term PLUS sc_term
	   ;

sc_term : static_class
	| iof_string
	    {
	      if ( check_flag ) {
	        }
	      else {
		  add_class('?');
		  cur_class->nn.str = ecalloc(strlen(name)+1,1);
		  sprintf(cur_class->nn.str,name);
	        }
	    }
	;

static_class : O_BRACE
		 {
		   add_class('l');
		   cur_elist = NULL;
		   cur_class->num = 0;
		 }
	       e_list C_BRACE
		 {
		   convert_elist();
		 }
	     | iof_string O_BRACK
		 {
		   add_class('n');
		 }
	       CONST
		 {
		   cur_class->from = int_val;
		 }
	       MINUS CONST C_BRACK
		 {
		   if ( (cur_class->num=int_val-cur_class->from+1) <= 0 ) {
		       fprintf(stderr,"\nERROR: inconsistent range !\n");
		       exit(1);
		     }
		   cur_elist = NULL;
		   add_elist();
		   convert_elist();
		 }
	     ;

e_list : e_list COMMA iof_string
	   {
	     add_elist();
	   }
       | e_list iof_string
	   {
	     add_elist();
	   }
       | iof_string
	   {
	     add_elist();
	   }
       ;

iof_string : of_string
	   | ID
           ;

of_string : f_string
	  | L_OR {
		   sprintf(name,"o");
		 }
	  ;

f_string : STRING
	 | X_FUNCT
	 | S_FUNCT
	 | N_FUNCT { int ii = strlen(name)+1;
			char * cc = name+ii;
			while ( ii-- ) {
			    *(cc+1) = *cc;
			    --cc;
			  }
			*name = 'N';
		      }
	 ;

marking : marking PLUS m_tuple
        | marking MINUS m_tuple
        | m_tuple
	;

m_tuple : m_coef LESS m_sum_list GREAT
	| LESS m_sum_list GREAT
	| m_coef iof_string
	| iof_string
	| m_coef
        ;

m_sum_list : m_sum_list COMMA m_sum
	   | m_sum
	   ;

m_coef : CONST
       | V_BAR iof_string V_BAR
       ;

m_sum : m_sum PLUS m_term
      | m_sum MINUS m_term
      | m_term
      ;

m_term : dyn_class
       | m_coef dyn_class
       | m_coef iof_string
       | iof_string
       ;

dyn_class : O_PAR iof_string
	    COLON m_coef C_PAR
          ;

function : function PLUS f_tuple
	 | function MINUS f_tuple
	 | f_tuple
	 ;

f_tuple : LESS f_sum_list GREAT
	| ID
        | f_coef LESS f_sum_list GREAT
        | f_coef ID
        | f_coef of_string
	    {
		if ( check_flag ) {
		  }
		else {
		  }
	    }
        | f_coef
	| of_string
	    {
		if ( check_flag ) {
		  }
		else {
		  }
	    }
	;

f_sum_list : f_sum_list COMMA f_sum
	   | f_sum
	   ;

f_coef : CONST
       | V_BAR iof_string V_BAR
	   {
	       if ( check_flag ) {
		 }
	       else {
		 }
	   }
       ;

f_sum : f_sum PLUS f_term
      | f_sum MINUS f_term
      | f_term
      ;

f_term : CONST S_FUNCT
       | S_FUNCT
       | CONST X_FUNCT
       | X_FUNCT
       | CONST N_FUNCT
       | N_FUNCT
       ;

predicate : predicate L_OR and_clause
	  | and_clause
	  ;

and_clause : and_clause L_AND not_clause
	   | not_clause
	   ;

not_clause : L_NOT clause
	   | O_PAR predicate C_PAR
	   | clause
	   ;

clause : O_BRACK IN iof_string p_compare C_BRACK
       | O_BRACK X_FUNCT IN iof_string C_BRACK
	   {
	       if ( check_flag ) {
		 }
	       else {
		 }
	   }
       | O_BRACK p_compare C_BRACK
       | if_string
	   {
	       if ( check_flag ) {
		 }
	       else {
		 }
	   }
       ;

p_compare : X_FUNCT EQ_NEQ X_FUNCT
	  | N_FUNCT EQ_NEQ X_FUNCT
	  | X_FUNCT EQ_NEQ N_FUNCT
	  ;

if_string : f_string
	  | ID
          ;

%%

#include "lex.yy.c"

#include "col_qreg.c"

