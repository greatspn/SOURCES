%option noyywrap
%{
extern int int_val, yylval;

#include "mdr_gram.y.h"

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

extern int checkplace (char * str);
extern int checkmpar (char * str);
extern int checkrpar (char * str);

#define L_AND 22
#define L_OR 23
%}
%%

= { yylval = EQUAL_TO ; return(COMP_OP); } ;

\/= { yylval = NOTEQUAL_TO ; return(COMP_OP); } ;

\>= { yylval = GREATER_EQUAL ; return(COMP_OP); } ;

\>/[^=] { yylval = GREATER_THAN ; return(COMP_OP); } ;

\<= { yylval = LESS_EQUAL ; return(COMP_OP); } ;

\</[^=] { yylval = LESS_THAN ; return(COMP_OP); } ;

\+ { yylval = SUM ; return(ARITHM_OP); } ;

\- { yylval = SUB ; return(ARITHM_OP); } ;

\* { yylval = MUL ; return(ARITHM_OP); } ;

\//[^=] { yylval = DIV ; return(ARITHM_OP); } ;

~ { return(NOT_OP); } ;

& { yylval = L_AND; return(LOGIC_OP); } ;

: { return(COLON); }

; { return(SEMICOLON); }

\. { return(POINT); }

\( { return(OPEN); }

\) { return(CLOSE); }

when { return(WHEN); } ;

ever { return(EVER); } ;

#[a-zA-Z][a-zA-Z_0-9]* { if ( yylval = checkplace(yytext+1) )
			     return(PLACE_NAME);
			 fprintf(stderr,"ERROR: no such place '%s'\n",
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

[a-zA-Z][a-zA-Z_0-9]* { if ( yylval = checkmpar(yytext) )
			    return(MARK_PAR);
			if ( yylval = checkrpar(yytext) )
			    return(RATE_PAR);
		        fprintf(stderr,"ERROR: no such parameter '%s'\n",
			     yytext );
			return(ERROR);
		      } ;

%%

