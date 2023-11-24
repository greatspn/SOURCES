%{

int int_val;
int yylex();

char res_name[200];

#define EQUAL_TO 1
#define NOTEQUAL_TO 2
#define GREATER_EQUAL 3
#define GREATER_THAN 4
#define LESS_EQUAL 5
#define LESS_THAN 6

#define SUM 11
#define SUB 12
#define MUL 13
#define DIV 14

%}

%start gram

%token RES_NAME COLON END_RES SEMICOLON
%token SUM_OP
%token MULT_OP
%token SS_PROB TR_PROB SS_MEAN TR_MEAN C_BRACE OPEN CLOSE COND_SIGN
%token L_NOT L_AND L_OR
%token PLACE_NAME COMP_OP
%token INTEGER MARK_PAR
%token RATE_PAR
%token EXP
%token POINT
%token ERROR

%%

gram : sequence END_RES
     | END_RES
     ;

sequence : sequence result
	 | result
	 ;

result : RES_NAME coord coord COLON
		{
#ifdef DEBUG
  fprintf(stderr,"\nresult:%s\n", res_name);
#endif
		  ++no_res;
		  new_res = (struct Res_def*)ecalloc(1,sizeof(struct Res_def));
		  new_res->next = NULL;
		  if ( cur_res == NULL )
		      first_res = new_res;
		  else
		      cur_res->next = new_res;
		  cur_res = new_res;
		  cur_res->name = ecalloc(strlen(res_name)+1,1);
		  sprintf(cur_res->name, "%s", res_name);
		  cur_res->nprobs = 0;
		  cur_prob = NULL;
		}
	    add SEMICOLON
       ;


add : add SUM_OP
		{
		  sumop[level_op] = yylval;
		}
	  fact
		{
		  if ( sumop[level_op] == SUM ) {
		      add_op('+');
		    }
		  else {
		      add_op('-');
		    }
		}
    | fact
    ;

fact : fact MULT_OP
		{
		  multop[level_op] = yylval;
		}
	  term
		{
		  if ( multop[level_op] == MUL ) {
		      add_op('*');
		    }
		  else {
		      add_op('/');
		    }
		}
     | fact
	  term
		{
		      add_op('*');
		}
     | term
     ;

term : SS_PROB
		{
		  add_op('o');
		  cur_prob->type = 'P';
		  cur_prob->num = ++no_pro;
		}
	  logic_cond C_BRACE
     | TR_PROB
		{
		  add_op('o');
		  cur_prob->type = 'p';
		  cur_prob->num = ++no_pro;
		}
	   logic_cond C_BRACE
     | SS_MEAN PLACE_NAME COND_SIGN
		{
		  add_op('o');
		  cur_prob->type = 'C';
		  cur_prob->num = ++no_pro;
		  cur_prob->val.place = yylval;
		}
	   logic_cond C_BRACE
     | TR_MEAN PLACE_NAME COND_SIGN
		{
		  add_op('o');
		  cur_prob->type = 'c';
		  cur_prob->num = ++no_pro;
		  cur_prob->val.place = yylval;
		}
	   logic_cond C_BRACE
     | SS_MEAN PLACE_NAME C_BRACE
		{
		  add_op('o');
		  cur_prob->type = 'E';
		  cur_prob->num = ++no_pro;
		  cur_prob->val.place = yylval;
		}
     | TR_MEAN PLACE_NAME C_BRACE
		{
		  add_op('o');
		  cur_prob->type = 'e';
		  cur_prob->num = ++no_pro;
		  cur_prob->val.place = yylval;
		}
     | real_val
		{
		  add_op('o');
		  cur_prob->type = 'r';
		  cur_prob->val.realval = cur_real;
		}
     | OPEN
		{
		  ++level_op;
		}
	    add CLOSE
		{
		  --level_op;
		}
     ;

logic_cond : logic_cond L_OR
		 logic_and
		{
		  add_cond('|');
		}
	   | logic_and
	   ;

logic_and : logic_and L_AND
		 logic_not
		{
		  add_cond('&');
		}
	   | logic_not
	   ;

logic_not : L_NOT
		 logic_term
		{
		  add_cond('~');
		}
	   | logic_term
	   ;

logic_term : OPEN
		logic_cond CLOSE
	   | compare
	   ;

compare : int_expr
		{
		  add_cond('c');
		  cur_cond->top1 = cur_inttype;
		  cur_cond->op1 = cur_intval;
		}
	      COMP_OP
		  { switch (yylval) {
		      case EQUAL_TO :
			  cur_cond->type = '=';
			break;
		      case NOTEQUAL_TO :
			  cur_cond->type = 'n';
			break;
		      case GREATER_EQUAL :
			  cur_cond->type = 'g';
			break;
		      case GREATER_THAN :
			  cur_cond->type = '>';
			break;
		      case LESS_EQUAL :
			  cur_cond->type = 'l';
			break;
		      case LESS_THAN :
			  cur_cond->type = '<';
			break;
		    }
		  }
	      int_expr
		{
		  cur_cond->top2 = cur_inttype;
		  cur_cond->op2 = cur_intval;
		}
	;

int_expr : INTEGER
		{
		  cur_intval = int_val;
		  cur_inttype = 'i';
		}
	  | MARK_PAR
		{
		  cur_intval = MP[yylval-1];
		  cur_inttype = 'i';
		}
	  | PLACE_NAME
		{
		  cur_intval = yylval;
		  cur_inttype = 'p';
		}
	  ;

real_val : real
	 | INTEGER
	      {
		cur_real = int_val;
	      }
	  | MARK_PAR
	      {
		cur_real = MP[yylval-1];
	      }
	 | RATE_PAR
	      {
		cur_real = RP[yylval-1];
	      }
	 ;

real : rational EXP
	      {
	        sprintf(sreal,"1.0e%d ", yylval );
		cur_real = atof(sreal);
		cur_real *= cur_rat;
	      }
     | rational
		{
		  cur_real = cur_rat;
		}
     ;

rational : INTEGER POINT
		{
		  sprintf(srat,"%d.", int_val );
		}
	      mant
		{
		  strcat(srat,smant );
		  cur_rat = atof(srat);
		}
	 | POINT
	      mant
		{
		  sprintf(srat,"0.%s",smant);
		  cur_rat = atof(srat);
		}
	 ;

mant : INTEGER
	    {
	      sprintf(smant,"%d ", int_val );
	    }
     |
	    {
	      sprintf(smant,"0 ");
	    }
     ;

coord : INTEGER POINT INTEGER
      ;

%%

#include "res_stndrd.y.h"
#include "gre_stndrd.c"

