%option noyywrap
%{
#include "res_gram.y.h"

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

extern int checkplace (char * str);
extern int checkmpar (char * str);
extern int checkrpar (char * str);

extern int yylval, int_val;
extern char res_name[100];
%}

%%

= { yylval = EQUAL_TO ; return(COMP_OP); } ;

\/= { yylval = NOTEQUAL_TO ; return(COMP_OP); } ;

\>= { yylval = GREATER_EQUAL ; return(COMP_OP); } ;

\>/[^=] { yylval = GREATER_THAN ; return(COMP_OP); } ;

\<= { yylval = LESS_EQUAL ; return(COMP_OP); } ;

\</[^=] { yylval = LESS_THAN ; return(COMP_OP); } ;

\+ { yylval = SUM ; return(SUM_OP); } ;

\- { yylval = SUB ; return(SUM_OP); } ;

\* { yylval = MUL ; return(MULT_OP); } ;

\//[^=] { yylval = DIV ; return(MULT_OP); } ;

~ { return(NOT_OP); } ;

& { yylval = L_AND; return(LOGIC_OP); } ;

: { return(COLON); }

; { return(SEMICOLON); }

\. { return(POINT); }

\( { return(OPEN); }

\) { return(CLOSE); }

\} { return(C_BRACE); }

P\{ { return(SS_PROB); } ;

p\{ { return(TR_PROB); } ;

E\{ { return(SS_MEAN); } ;

e\{ { return(TR_MEAN); } ;

^\|[a-zA-Z][a-zA-Z0-9]* { sprintf(res_name,yytext+1);
			  return(RES_NAME);
			} ;

^\|[^a-zA-Z] { return(END_RES); } ;

\| { return(COND_SIGN); } ;

#[a-zA-Z][a-zA-Z_0-9]* {  if ( (yylval=checkplace(yytext+1)) )
			      return(PLACE_NAME);
			  fprintf(stderr,"\nERROR: no such place '%s'\n",
					 yytext+1 );
			  return(ERROR);
		       } ;

-?[0-9]+ { int_val = atoi(yytext); return(INTEGER) ; } ;

[eE]" "*[+-]?[0-9]+ { char*cp=yytext;
		      while ( (*cp < '0' || *cp > '9')&&(*cp != '-') )
			  cp++;
		      yylval = atoi( cp );
		      return( EXP );
		    } ;

o/[^a-zA-Z_0-9] { yylval = L_OR; return(LOGIC_OP); } ;

[a-zA-Z][a-zA-Z_0-9]* { if ( (yylval = checkmpar(yytext)) )
			    return(MARK_PAR);
                        if ( (yylval = checkrpar(yytext)) )
			    return(RATE_PAR);
			fprintf(stderr,"\nERROR: no such parameter '%s'\n",
				       yytext );
			return(ERROR);
		      } ;

%%

