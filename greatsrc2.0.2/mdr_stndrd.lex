%{
/*#define YY_INPUT(buf,result,max_size) \
    { \
    int c = getc(yyin); \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
    }*/
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

~ { return(L_NOT); } ;

& { return(L_AND); } ;

: { return(COLON); }

; { return(SEMICOLON); }

\. { return(POINT); }

\( { return(OPEN); }

\) { return(CLOSE); }

when { return(WHEN); } ;

ever { return(EVER); } ;

gd { return(GD); } ;

age_m { return(AGE); } ;

en_m { return(ENABL); } ;

reen_p { return(REEN); } ;

pree_p { return(PREE); } ;

erlang { return(ERL); } ;

discrete { return(DIS); } ;

linear { return(LIN); } ;

cox { return(COX); } ;

hyperexp { return(IPE); } ;

redraw { return(PO_REDRAW); } ;

random { return(PO_RAND); } ;

first_drawn { return(PO_FD); } ;

last_drawn { return(PO_LD); } ;

first_time { return(PO_FT); } ;

last_time { return(PO_LT); } ;

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

o/[^a-zA-Z_0-9] { return(L_OR); } ;

[a-zA-Z][a-zA-Z_0-9]* { if ( yylval = checkmpar(yytext) )
			    return(MARK_PAR);
			if ( yylval = checkrpar(yytext) )
			    return(RATE_PAR);
		        fprintf(stderr,"ERROR: no such parameter '%s'\n",
			     yytext );
			return(ERROR);
		      } ;
[\n\t ] {;}
. {return;}
%%

