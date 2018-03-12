%{
	/* Scanner definitions for the lex lexer generator */
	#include <math.h>
	#include "common-defs.h"
	#include "asmc.h"
	#include "dta.h"
	#include "cslta.h"
	#include "parser.y.h"
		
	#define MAX_NESTED_LOAD 32
	int lex_line_count[MAX_NESTED_LOAD];
	int lex_nest_level = 0;

	#ifndef UNUSED
	#define UNUSED(x) ((void)(x))
	#endif
	
	void lexer_open_file(FILE *file) {
		yy_switch_to_buffer(yy_create_buffer( file, YY_BUF_SIZE ));
	}
%}
%option  nounistd	 

/* State for loading external files */
%x load_file

LTR		[a-zA-Z_]|(\xCE[\x91-\xA9\xB1-\xBF])|(\xCF[\x80-\x89\x91-\x96])
LTRX    {LTR}|[0-9"'"]

%%
[Ll][Oo][Aa][Dd]					BEGIN(load_file);

[Aa][Nn][Dd]						return AND;
[Oo][Rr]							return OR;
[Nn][Oo][Tt]						return NOT;
[Ii][Mm][Pp][Ll][Yy]				return IMPLY;
[Ss][Tt][Ee][Aa][Dd][Yy]			return STEADY;
[Pp][Rr][Oo][Bb]_[Tt][Aa]			return PROB_TA;
[Aa][Ss][Mm][Cc]					return DECLASMC;
[Dd][Tt][Aa]						return DECLDTA;
[Cc][Oo][Nn][Ss][Tt]				return DECLCONST;
[Rr][Ee][Ss][Uu][Ll][Tt]			return DECLRESULT;
[Ee][Vv][Aa][Ll]					return EVAL;
[Ss][Aa][Vv][Ee]_[Gg][Mm][Ll]		return SAVE_GML;
[Ss][Aa][Vv][Ee]_[Tt][Rr][Gg]		return SAVE_TRG;
[Ss][Aa][Vv][Ee]_[Dd][Ss][Pp][Nn]	return SAVE_DSPN;
[Ss][Ee][Tt]_[Oo][Uu][Tt][Pp][Uu][Tt]_[Dd][Ii][Rr]			  return SETOUTPUTDIR;
[Ll][Oo][Aa][Dd]_[Cc][Oo][Oo][Rr][Dd][Ii][Nn][Aa][Tt][Ee][Ss] return LOAD_COORDS;
[Pp][Ll][Aa][Nn][Aa][Rr][Ii][Zz][Ee] return PLANARIZE;
[Qq][Uu][Ii][Tt]					return QUIT_APP;
[Ee][Xx][Ii][Tt]					return QUIT_APP;
[Ss][Tt][Aa][Tt][Ee]				return STATE;
[Tt][Rr][Aa][Nn][Ss][Ii][Tt][Ii][Oo][Nn] return TRANSITION;
[Ll][Oo][Cc][Aa][Tt][Ii][Oo][Nn]    return LOCATION;
[Ee][Dd][Gg][Ee]					return EDGE;
[Ii][Nn][Ii][Tt][Ii][Aa][Ll]		return INITLOC;
[Ff][Ii][Nn][Aa][Ll]				return FINAL;
[Aa][Cc][Tt][Ss]					return ACTS;
[Aa][Cc][Tt]						return ACTS;
[Rr][Ee][Ss][Ee][Tt]				return RESET;
[Cc][Ll][Ee][Aa][Rr]				return CLEAR;
[Rr][Ee][Nn][Aa][Mm][Ee]			return RENAME;
[Xx]								return XCLOCK;
[Tt][Rr][Uu][Ee]					return EXPTRUE;
[Ff][Aa][Ll][Ss][Ee]				return EXPFALSE;
"#"									return BOUNDARY_ACTION;
"&&"								return AND;
"||"								return OR;
"!"									return NOT;
"∧"									return AND;
"∨"									return OR;
"¬"									return NOT;
"->"								return ARROW;
"|="								return SATISFIES;
"<="								return LESS_EQ;
"<"									return LESS;
"=="								return EQ;
"!="								return NOT_EQ;
">="								return GREATER_EQ;
">"									return GREATER;
"("									return ('(');
")"									return (')');
"["									return ('[');
"]"									return (']');
"{"									return ('{');
"}"									return ('}');
"+"									return ('+');
"-"									return ('-');
"*"									return ('*');
"/"									return ('/');
"%"									return ('%');
"="									return ('=');
";"									return (';');
":"									return (':');
","									return (',');
"|"									return ('|');
\n									lex_line_count[lex_nest_level]++;
[ \t]+								/* ignore whitespace */;
"//"[^\n]*							/* ignore comments */;
[0-9]+"."[0-9]*						yylval.str=strdup(yytext); return REALNUM;
[0-9]+								yylval.str=strdup(yytext); return INTEGER;
{LTR}{LTRX}* {
	yylval.str=strdup(yytext); 
	if (Constant_IsDefined(yytext))
		return CONST_NAME;
	return ID;
}

"\""[^\n"\""]*"\""		{   /* return the string content, without the "" */
	UNUSED(yyunput); /* Make GCC happy... */
	size_t len = strlen(yytext);
	yylval.str=(char*)malloc(len-1);
	strncpy(yylval.str, yytext+1, len-2);
	yylval.str[len-2] = '\0';
	return STRING;
}


<load_file>[ \t]*			/* eat the whitespace */
<load_file>[^ \t\n]+   {	/* got the file name to load */
	if (++lex_nest_level >= MAX_NESTED_LOAD) {
		printf("Too much files loaded.\n");
		exit(-1);
	}
	lex_line_count[lex_nest_level] = 0;
	yyin = fopen( yytext, "r" );	
	if ( !yyin ) {
		printf("Error: Cannot open file %s\n.", yytext);
		exit(-1);
	}		
	yypush_buffer_state(yy_create_buffer( yyin, YY_BUF_SIZE ));
	BEGIN(INITIAL);
}

<<EOF>> {
	lex_nest_level--;
	yypop_buffer_state();	
	if ( !YY_CURRENT_BUFFER ) {
		yyterminate(); // End of the last file reached
	}
}
%%