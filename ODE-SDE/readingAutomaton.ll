%option yylineno 

%{

#include <stdio.h>
#include "y.tab.h"



%}

%%



[%]				{return COMMENT;};

[0-9]+("."[0-9]*)?      	{yylval.num= (long double)atof(yytext); return NUMBER;}
[A-Za-z_$]+[A-Za-z0-9_]*		{yylval.pVar = strdup(yytext); return STRING;}

"#"				{yylval.pVar = strdup(yytext);  return	INIT;}
"@"				{yylval.pVar = strdup(yytext); return FINAL;}

"("          			{return LPARENT;}
")"          			{return RPARENT;}

"{"          			{return LBRACE;}
"}"          			{return RBRACE;}

"<"          			{return MINOR;}
">"          			{return MAJOR;}
"<>"         			{return NEQ;}
"=="         			{return EQ;}
"<="          			{return LMINOR;}
">="          			{return LMAJOR;}

"||"         			{return OR;}
"&&"        			{return AND;}
"!"        			{return NOT;}

"+"         	 		{return PLUS;}
"-"          			{return MINUS;}
"*"          			{return TIMES;}
"/"          			{return DIV;}

"@"[A-Za-z_$]+[A-Za-z0-9_]*				{yylval.pVar = strdup(yytext); return TIMEVAR;}

"->"				{return ARROW;}	

[ \t]+       			/* ignore whitespace */;
[\n]				{return ENDL;}
[;]+	     			{return SEMICOLON;}
[,]+				{return COMMA;}
<<EOF>>      			{return 0;}

%%
