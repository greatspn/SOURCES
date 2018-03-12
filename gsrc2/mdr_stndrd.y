%{

#include <string.h>
#include "trans_def.h"

int int_val;
int yylex();
static int policy_type;
static int distrib_type;

extern int checkplace (char * str);
extern int checkmpar (char * str);
extern int checkrpar (char * str);


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

%token WHEN EVER PLACE_NAME COMP_OP INTEGER MARK_PAR SUM_OP MULT_OP
%token RATE_PAR EXP ERROR COLON SEMICOLON POINT OPEN CLOSE L_AND L_OR L_NOT
%token GD AGE ENABL REEN PREE PO_REDRAW PO_RAND PO_FD PO_LD PO_FT PO_LT
%token DIS LIN COX IPE ERL

%%

gram : assign
     | GD
	 {
	    policy_type = 0;
	 }
       gen_distr
     ;

gen_distr : AGE
	      {
		policy_type = POLICY_AGE;
	      }
            age_m dis_p
          | ENABL dis_p
          | dis_p
          ;

age_m : ree_p
      |
      ;

ree_p : REEN PO_REDRAW
	 {
	   policy_type |= POLICY_REDRAW;
	 }
      | REEN PO_RAND
	 {
	   policy_type |= POLICY_RE_EN_RANDOM;
	 }
      | REEN PO_FD
	 {
	   policy_type |= POLICY_RE_EN_FIRST_DRAWN;
	 }
      | REEN PO_LD
	 {
	   policy_type |= POLICY_RE_EN_LAST_DRAWN;
	 }
      | REEN PO_FT
	 {
	   policy_type |= POLICY_RE_EN_FIRST_DESCHED;
	 }
      | REEN PO_LT
	 {
	   policy_type |= POLICY_RE_EN_LAST_DESCHED;
	 }
      ;

dis_p : pree_po distr_type
      | distr_type
      ;

pree_po : PREE PO_RAND
	   {
	     policy_type |= POLICY_DIS_RANDOM;
	   }
        | PREE PO_FD
	   {
	     policy_type |= POLICY_DIS_FIRST_DRAWN;
	   }
        | PREE PO_LD
	   {
	     policy_type |= POLICY_DIS_LAST_DRAWN;
	   }
        | PREE PO_FT
	   {
	     policy_type |= POLICY_DIS_FIRST_SCHED;
	   }
        | PREE PO_LT
	   {
	     policy_type |= POLICY_DIS_LAST_SCHED;
	   }
        ;

distr_type : ERL INTEGER
	      {
		if ( --int_val < 0 || int_val > 31 ) {
		    fprintf(stderr,
		        "ERROR: number of Erlang stages %d out of range!\n",
			    int_val+1 );
		    exit(1);
		  }
	        distrib_type = TIMING_ERLANG | int_val;
		read_distrib(policy_type,distrib_type);
	      }
           | DIS INTEGER
	      {
		if ( --int_val < 0 || int_val > 31 ) {
		    fprintf(stderr,
		        "ERROR: number of Discrete values %d out of range!\n",
			    int_val+1 );
		    exit(1);
		  }
	        distrib_type = TIMING_DISCRETE | int_val;
		read_distrib(policy_type,distrib_type);
	      }
           | LIN INTEGER
	      {
		if ( --int_val < 0 || int_val > 31 ) {
		    fprintf(stderr,
		        "ERROR: number of Linear intervals %d out of range!\n",
			    int_val+1 );
		    exit(1);
		  }
	        distrib_type = TIMING_LINEAR | int_val;
		read_distrib(policy_type,distrib_type);
	      }
           | COX INTEGER
	      {
		if ( --int_val < 0 || int_val > 31 ) {
		    fprintf(stderr,
		        "ERROR: number of Cox stages %d out of range!\n",
			    int_val+1 );
		    exit(1);
		  }
	        distrib_type = TIMING_COX_normalized | int_val;
		read_distrib(policy_type,distrib_type);
	      }
           | IPE INTEGER
	      {
		if ( --int_val < 0 || int_val > 31 ) {
		    fprintf(stderr,
		  "ERROR: number of iperexponential stages %d out of range!\n",
			    int_val+1 );
		    exit(1);
		  }
	        distrib_type = TIMING_IPEREXP | int_val;
		read_distrib(policy_type,distrib_type);
	      }
           ;

assign : EVER
	    {
	      add_clause('e');
	    }
	  sum SEMICOLON
	    {
	      yyin = eof;
	    }
       | when_clause EVER
	    {
	      add_clause('e');
	    }
	  sum SEMICOLON
	    {
	      yyin = eof;
	    }
       ;

when_clause : when_clause WHEN
		  {
		    add_clause('w');
		  }
		logic_cond COLON
		sum SEMICOLON
	    | WHEN
		  {
		    add_clause('w');
		  }
		logic_cond COLON
		sum SEMICOLON
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

compare : PLACE_NAME
		  {
		    add_cond('c');
		    cur_cond->p1 = yylval;
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
	      int_const
		  {
		    cur_cond->top2 = cur_inttype;
		    cur_cond->op2 = cur_intval;
		  }
	;

int_const : INTEGER
		{
		  cur_inttype = 'i';
		  cur_intval = int_val;
		}
	  | MARK_PAR
		{
		  cur_inttype = 'i';
		  cur_intval = MP[yylval-1];
		}
	  | PLACE_NAME
		{
		  cur_inttype = 'p';
		  cur_intval = yylval;
		}
	  ;

sum : sum SUM_OP
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
          real_val
                {
                  if ( multop[level_op] == MUL ) {
                      add_op('*');
                    }
                  else {
                      add_op('/');
                    }
                }
     | fact
          real_val
                {
                      add_op('*');
                }
     | real_val
     ; 

real_val : OPEN
		{
		  ++level_op;
		}
	    sum CLOSE
		{
		  --level_op;
		}
         | real
                {
                  add_op('o');
                  cur_val->type = 'r';
                  cur_val->val.real = cur_real;
                }
	 | int_const
		{
                  add_op('o');
		  if ( cur_inttype == 'p' ) {
		      cur_val->type = 'p';
		      cur_val->val.place = cur_intval;
		    }
                  else {
		      cur_val->type = 'r';
		      cur_val->val.real = (double)cur_intval;
		    }
		}
	 | RATE_PAR
		{
                  add_op('o');
                  cur_val->type = 'r';
                  cur_val->val.real = RP[yylval-1];
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

%%

#include "mdr_stndrd.y.h"
#include "gmt_prep.c"

