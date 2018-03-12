%option noyywrap
%{
#include <string.h>
#include "parser.y.h"

//extern char* lexstr;

//#undef input
//#undef unput
//#define input() (*lexstr++)
//#define unput(c) (*--lexstr=c)
                         
%}

id		[^|\n{}=]+

%%

\n              {}
"|"             {return(BAR);}
"{"             {return(B_WAVE);}
"}"             {return(E_WAVE);}
"="             {return(EQUAL);}
transition      {return(TR);}
place           {return(PL);}
{id}		{ yylval.SVal = strdup(yytext); return(ID);}

%%




