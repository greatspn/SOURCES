
%%

\=  { yylval = EQUAL_TO ; return(EQ_NEQ); } ;

\/\=  { yylval = NOTEQUAL_TO ; return(EQ_NEQ); } ;

\+  { return(PLUS); } ;

\-  { return(MINUS); } ;

\~  { return(L_NOT); } ;

\&  { return(L_AND); } ;

\,  { return(COMMA); } ;

\:  { return(COLON); } ;

\;  { return(SEMICOLON); } ;

\(  { return(O_PAR); } ;

\)  { return(C_PAR); } ;

\{  { return(O_BRACE); } ;

\}  { return(C_BRACE); } ;

\[  { return(O_BRACK); } ;

\]  { return(C_BRACK); } ;

\<  { return(LESS); } ;

\>  { return(GREAT); } ;

\!  { return(IN); } ;

\|  { return(V_BAR); } ;

\#  { return(NO_COLOUR); } ;

\@f  { return(FUNCT); } ;

\@m  { return(MARK); } ;

\@c  { return(COL); } ;

\@p  { return(PRED); } ;

o/[^a-zA-Z0-9_]  { return(L_OR); } ;

id/[^a-zA-Z0-9_]  { sprintf(name,yytext); return(ID); };

I[dD]/[^a-zA-Z0-9_]  { sprintf(name,yytext); return(ID); };

[sS]/[^a-rt-zA-RT-Z0-9_]  { sprintf(name,yytext);
			   return(S_FUNCT);
			 } ;

[xXyYzZ]+/[^a-wA-W0-9_]  { sprintf(name,yytext);
			  return(X_FUNCT);
		        } ;

N[xXyYzZ]*/[^a-wA-W0-9_]  { sprintf(name,yytext+1);
			   return(N_FUNCT);
			 } ;

[a-zA-Z][a-zA-Z0-9_]*  { sprintf(name,yytext);
			 return(STRING);
		       } ;

[0-9]+  { int_val = atoi(yytext);
	  return(CONST);
	} ;

%%

