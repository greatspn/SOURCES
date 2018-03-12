%{
#include <stdio.h>
#include <string.h>
#include "readingObjectiveFunction.yy.h"

%}

using namespace std;

%%

abs|sin|cos|tan|sqrt|ceiling|floor|trunc|log|exp	{mmlval.pVar = strdup(yytext);	return FUNCT;}
[0-9]+("."[0-9]*)?      			 	{mmlval.num= (long double)atof(yytext); return FLOAT;}
[A-Za-z_$]+[A-Za-z0-9_]*			 	{mmlval.pVar = strdup(yytext); return PLACE;}
"%"[ A-Za-z_$\t]*				 	{mmlval.pVar = strdup(yytext); return STRING;}
"+"						 	{mmlval.pVar = strdup(yytext); return PLUS;}
"-"						 	{mmlval.pVar = strdup(yytext); return MINUS;}
"*"						 	{mmlval.pVar = strdup(yytext); return MULT;}
"/"						 	{mmlval.pVar = strdup(yytext); return DIV;}
"("						 	{mmlval.pVar = strdup(yytext); return BOPEN;}
")"						 	{mmlval.pVar = strdup(yytext); return BCLOSE;}
[\n]                      	{return ENDLINE;}
[ \t]+                  				/* ignore whitespace */;

%%
