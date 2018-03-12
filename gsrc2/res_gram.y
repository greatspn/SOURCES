%{

int int_val;
int yylex();

char res_name[100];

extern int checkplace (char *str);
extern int checkmpar (char *str);
extern int checkrpar (char *str);

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

#define L_AND 22
#define L_OR 23
%}

%start gram

%token RES_NAME COLON END_RES SEMICOLON
%token SUM_OP
%token MULT_OP
%token SS_PROB TR_PROB SS_MEAN TR_MEAN C_BRACE OPEN CLOSE COND_SIGN
%token NOT_OP LOGIC_OP
%token PLACE_NAME COMP_OP
%token INTEGER MARK_PAR
%token RATE_PAR
%token EXP
%token POINT
%token ERROR

%%

gram : sequence END_RES
     ;

sequence : sequence result
	 | result
	 ;

result : RES_NAME coord coord COLON
		{
/*
#define DEBUG
*/
#ifdef DEBUG
    fprintf(stderr,"... parsing result %s\n", res_name);
#endif /* DEBUG */
		  ++no_res;
		  fprintf(rfp,"  fprintf(ofp,\"%s = %%f +- %%f\\n\", STIMA[%d], ERROR[%d]);\n",
				res_name, no_res, no_res );
		  fprintf(sfp,"  PROD[%d] =\n", no_res );
		}
	    add SEMICOLON
		{ 
#define DEBUG
#ifdef DEBUG
    fprintf(stderr,"... end result definition %s\n", res_name);
#endif /* DEBUG */
		  fprintf(sfp,";\n");
		}
       ;


add : add SUM_OP
		{
		  if ( yylval == SUM ) {
		      fprintf(sfp," + ");
		    }
		  else {
		      fprintf(sfp," - ");
		    }
		}
	  fact
    | fact
    ;

fact : fact MULT_OP
		{
		  if ( yylval == MUL ) {
		      fprintf(sfp," * ");
		    }
		  else {
		      fprintf(sfp," / ");
		    }
		}
	  term
     | fact
		{
		      fprintf(sfp," * ");
		}
	  term
     | term
     ;

term : SS_PROB
		{
		  fprintf(sfp,"PRO[%d]", no_pro); ++no_pro;
		  fprintf(ofp,"  if (\n       " );
		}
	  logic_cond C_BRACE
		{
		  fprintf(ofp,"\n   )  *pp += ival;\n  ++pp;\n");
		}
     | TR_PROB
		{
		  fprintf(sfp,"PRO[%d]", no_pro); ++no_pro;
		  fprintf(ofp,"  if (\n" );
		}
	   logic_cond C_BRACE
		{
		  fprintf(ofp,"\n   )  *pp += val;\n  ++pp;\n");
		}
     | SS_MEAN PLACE_NAME COND_SIGN
		{
		  fprintf(sfp,"PRO[%d]", no_pro); ++no_pro;
		  p_num = yylval;
		}
	   logic_cond C_BRACE
		{
		  fprintf(ofp,"\n   )  *pp += DP[%d]*ival;\n  ++pp;\n",
			      p_num );
		}
     | TR_MEAN PLACE_NAME COND_SIGN
		{
		  fprintf(sfp,"PRO[%d]", no_pro); ++no_pro;
		  p_num = yylval;
		}
	   logic_cond C_BRACE
		{
		  fprintf(ofp,"\n   )  *pp += DP[%d]*val;\n  ++pp;\n",
			      p_num );
		}
     | SS_MEAN PLACE_NAME C_BRACE
		{
#define DEBUG
#ifdef DEBUG
    fprintf(stderr,"  ... average marking of place %d\n", yylval);
#endif /* DEBUG */
		  fprintf(sfp,"PRO[%d]", no_pro); ++no_pro;
		  fprintf(ofp,"  *(pp++) += DP[%d]*ival;\n",
			      yylval );
		}
     | TR_MEAN PLACE_NAME C_BRACE
		{
		  fprintf(sfp,"tr_mean_tok(%d)", yylval);
		}
     | real_val
     | OPEN
		{
		  fprintf(sfp," ( ");
		}
	    add CLOSE
		{
		  fprintf(sfp," ) ");
		}
     ;

logic_cond : OPEN
		  { fprintf(ofp," ( "); }
		logic_cond CLOSE
		  { fprintf(ofp," ) "); }
	   | NOT_OP
		  { fprintf(ofp," ! "); }
		 logic_cond
	   | logic_cond LOGIC_OP
		    { if ( yylval == L_AND )
			  fprintf(ofp," && ");
		      else
			  fprintf(ofp," || ");
		    }
		 logic_cond
	   | compare
	   ;

compare : int_expr COMP_OP
		  { switch (yylval) {
		      case EQUAL_TO :
			  fprintf(ofp," == ");
			break;
		      case NOTEQUAL_TO :
			  fprintf(ofp," != ");
			break;
		      case GREATER_EQUAL :
			  fprintf(ofp," >= ");
			break;
		      case GREATER_THAN :
			  fprintf(ofp," > ");
			break;
		      case LESS_EQUAL :
			  fprintf(ofp," <= ");
			break;
		      case LESS_THAN :
			  fprintf(ofp," < ");
			break;
		    }
		  }
	      int_expr
	;

int_expr : INTEGER
		{ fprintf(ofp,"%d", int_val ); }
	  | MARK_PAR
		{ fprintf(ofp,"%d", MP[yylval-1] ); }
	  | PLACE_NAME
		{ fprintf(ofp,"DP[%d]", yylval ); }
	  ;

real_val : real
	 | INTEGER
		{
		  fprintf(sfp,"%d", int_val );
		}
	  | MARK_PAR
		{
		  fprintf(sfp,"%d", MP[yylval-1] );
		}
	 | RATE_PAR
		{
		  fprintf(sfp,"%f", RP[yylval-1] );
		}
	 ;

real : rational EXP
	      {
	        fprintf(sfp,"e%d", yylval );
	      }
     | rational
     ;

rational : INTEGER POINT
		{
		  fprintf(sfp,"%d.", int_val );
		}
	      mant
	 | POINT
		{
		  fprintf(sfp,"0.");
		}
	      mant
	 ;

mant : INTEGER
	    {
	      fprintf(sfp,"%d", int_val );
	    }
     |
	    {
	      fprintf(sfp,"0");
	    }
     ;

coord : INTEGER POINT INTEGER
      ;

%%

#include "res_gram.y.h"
#include "gre.c"

