%{
#include "res_stndrd.y.h"

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

extern int yylval, int_val;
extern char res_name[200];

extern int checkplace (char * str);
extern int checkmpar (char * str);
extern int checkrpar (char * str);

extern FILE* extra_def;
//extern int has_extra_def;

#define DEBUG

int yywrap(void) {
	/*if (has_extra_def) { // switch to extra definitions file.
		//fclose(yyin);
		yyin = extra_def;
		has_extra_def = 0;
		return 0; // Scan next file
	} else */
	return 1; // End of input
}

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

"~"|"!"|"not" { return(L_NOT); } ;

"&"|"&&"|"and" { return(L_AND); } ;

"||"|"or" { return(L_OR); } ;

: { return(COLON); }

; { return(SEMICOLON); }

\. { return(POINT); }

\( { return(OPEN); }

\) { return(CLOSE); }

\} { return(C_BRACE); }

P\{ { return(SS_PROB); } ;

p\{ { return(TR_PROB); } ;

E\{ {
#ifdef DEBUG
    fprintf(stderr,"\n token SS_MEAN\n");
#endif /* DEBUG */
	      return(SS_MEAN); } ;

e\{ { return(TR_MEAN); } ;

^\|[a-zA-Z][a-zA-Z0-9]* { sprintf(res_name, "%s", yytext+1);
#ifdef DEBUG
    fprintf(stderr,"\n token RES_NAME '%s'\n", res_name);
#endif /* DEBUG */
			  return(RES_NAME);
			} ;

^\|[^a-zA-Z] {
#ifdef DEBUG
    fprintf(stderr,"\n token END_RES\n");
#endif /* DEBUG */
	/*if (has_extra_def) { 
		// Do not return, wait for end file and 
		// than switch to extra definitions file.
	} else */
	return(END_RES); } ;

[\n\r\t ] { /* Skip spaces */ }

^[^\|][^\n]* { /* Skip end line */ }

\| { return(COND_SIGN); } ;

#[a-zA-Z][a-zA-Z_0-9]* {  if ( yylval=checkplace(yytext+1) ) {
#ifdef DEBUG
    fprintf(stderr,"\n token PLACE_NAME '%s'\n", yytext+1);
#endif /* DEBUG */
			      return(PLACE_NAME);
			    }
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

o/[^a-zA-Z_0-9] { return(L_OR); } ;

[a-zA-Z][a-zA-Z_0-9]* { if ( yylval = checkmpar(yytext) )
			    return(MARK_PAR);
			if ( yylval = checkrpar(yytext) )
			    return(RATE_PAR);
			fprintf(stderr,"\nERROR: no such parameter '%s'\n",
				       yytext );
			return(ERROR);
		      } ;

%%

