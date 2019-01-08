%top{ 
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <cfloat>
#include <memory>
using namespace std;

#include <boost/noncopyable.hpp>
#include <boost/algorithm/string/predicate.hpp>  // boost::iequals
#include <boost/optional.hpp>

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
// ParserDefs containts the definition of YY_EXTRA_TYPE
#include "ParserDefs.h"
#include "PetriNet.h"
#include "Measure.h"
#include "DTA.h"

// Use STRINGSTREAM instead of the stdio FILE interface 
#define FILE    STRINGSTREAM
	
#ifdef stdin
#  undef stdin
#endif
#define stdin   NULL
	
#ifdef stdout
#  undef stdout
#endif
#define stdout  NULL

#undef fread
#undef fwrite
#undef clearerr
#undef ferror
#undef getc
#undef fileno
#undef isatty
	
#define fread(a,b,c,d)		d->stream_fread(a,(size_t)(b),(size_t)(c))
#define fwrite(a,b,c,d)		d->stream_fwrite(a,(size_t)(b),(size_t)(c))
#define clearerr(a)			a->stream_clearerr()
#define ferror(a)			a->stream_ferror()
#define getc(a)				a->stream_getc()
#define fileno(a)			a->stream_fileno()
#define isatty(a)			STRINGSTREAM::stream_isatty(a)

#define register
	
//#include "parser.hh"
#include "newparser.lyy.h"

typedef Token YYSTYPE;

// Allow the definition of NULL just here for flex.
#ifndef NULL
# define NULL  nullptr
#endif

}

/* Don't use global variables */
%option  reentrant
/* Never in interactive mode */
%option  batch		 
/* Accept 8-bit characters */
%option  8bit		 
/* pass YYSTYPE * yylval_param as first argument */
%option  bison-bridge	
/* pass YYLTYPE * yylloc_param as second argument */
/*%option  bison-locations	*/
/* Don't include <unistd.h> */
%option  nounistd
/* Prefix for the Lex function */
/*%option prefix="gspnFile"*/
/* No multiple file wrap() functionality */
%option noyywrap

LTR		[a-zA-Z_]|(\xCE[\x91-\xA9\xB1-\xBF])|(\xCF[\x80-\x89\x91-\x96])
LTRX    {LTR}|[0-9"'"]

%%
"Min"				return TOKEN_MIN_FN;
"Max"				return TOKEN_MAX_FN;
"Mod"				return TOKEN_MOD_FN;
"Pow"				return TOKEN_POW_FN;
"Factorial"			return TOKEN_FACTORIAL_FN;
"DiracDelta"        return TOKEN_DIRACDELTA_FN;

"Abs"				return TOKEN_ABS_FN;
"Sin"				return TOKEN_SIN_FN;
"Cos"				return TOKEN_COS_FN;
"Tan"				return TOKEN_TAN_FN;
"Asin"				return TOKEN_ARCSIN_FN;
"Acos"				return TOKEN_ARCCOS_FN;
"Atan"				return TOKEN_ARCTAN_FN;
"Exp"				return TOKEN_EXP_FN;
"Log"				return TOKEN_LOG_FN;
"Sqrt"				return TOKEN_SQRT_FN;
"Ceil"				return TOKEN_CEIL_FN;
"Floor"				return TOKEN_FLOOR_FN;
"Binomial"			return TOKEN_BINOMIAL_FN;
"Fract"			    return TOKEN_FRACT_FN;
"Rect"              return TOKEN_RECT_FN;
"Uniform"           return TOKEN_UNIFORM_FN;
"Triangular"        return TOKEN_TRIANGULAR_FN;
"Erlang"            return TOKEN_ERLANG_FN;
"TruncatedExp"      return TOKEN_TRUNCATED_EXP_FN;
"Pareto"            return TOKEN_PARETO_FN;

"If"				return TOKEN_IF_FN;
"when"				return TOKEN_WHEN;
"ever"				return TOKEN_EVER;
"&"					return TOKEN_AND;
"|"					return TOKEN_OR;
"~"					return TOKEN_NOT;
"("					return TOKEN_OP_PAREN;
")"					return TOKEN_CL_PAREN;
"["					return TOKEN_OP_SQPAR;
"]"					return TOKEN_CL_SQPAR;
"{"					return TOKEN_OP_BRACK;
"}"					return TOKEN_CL_BRACK;
","					return TOKEN_COMMA;
";"					return TOKEN_SEMICOLON;
":"					return TOKEN_COLON;
"#"					return TOKEN_SHARP;
"+"					return TOKEN_PLUS;
"-"					return TOKEN_MINUS;
"*"					return TOKEN_TIMES;
"/"					return TOKEN_DIV;
"="					return TOKEN_EQ;
"=="				return TOKEN_EQ;
"/="				return TOKEN_NOT_EQ;
">"					return TOKEN_GREATER;
"<"					return TOKEN_LESS;
">="				return TOKEN_GREATER_EQ;
"<="				return TOKEN_LESS_EQ;

"E{"				return TOKEN_OPEN_E;
"P{"				return TOKEN_OPEN_P;
"X{"				return TOKEN_OPEN_X;

"&&"				return TOKEN_AND;
"||"				return TOKEN_OR;
"!"					return TOKEN_NOT;
"\\"				return TOKEN_SETMINUS;
"True"				return TOKEN_TRUE;
"False"				return TOKEN_FALSE;

"PROB_TA"			return TOKEN_PROB_TA;

\n					//lex_line_count++;
[ \t]+				/* ignore whitespace */;
"//"[^\n]*			/* ignore comments */;
[0-9]+"."[0-9]*		yylval_param->real = atof(yytext);    return TOKEN_REALNUM;
[0-9]+				yylval_param->integer = atoi(yytext); return TOKEN_INTEGER;
{LTR}{LTRX}* {
	yylval_param->text = yytext;
	const PN* pPN = yyget_extra(yyscanner)->pPN;
	if (pPN != NULL) {
		if (IsGMarkPar(yytext, pPN))
			return TOKEN_G_MARK_PARAM;
		if (IsGRatePar(yytext, pPN))
			return TOKEN_G_RATE_PARAM;
		if (IsGPlaceId(yytext, pPN))
			return TOKEN_G_PLACE_ID;
		if (IsGTransitionId(yytext, pPN))
			return TOKEN_G_TRANSITION_ID;
	}
	const DTA* pDTA = yyget_extra(yyscanner)->pDTA;
	if (pDTA != NULL) {
		if (IsAtomicPropositionId(yytext, pDTA))
			return TOKEN_G_ATOMIC_PROP_ID;
		if (IsActionId(yytext, pDTA))
			return TOKEN_G_ACTION_ID;
		if (IsClockValueId(yytext, pDTA))
			return TOKEN_G_CLOCKVALUE_ID;
		if (boost::iequals(yytext, "x"))
			return TOKEN_XCLK;
		if (boost::iequals(yytext, "ACTS") || boost::iequals(yytext, "ACT"))
			return TOKEN_ACTS;
		if (boost::iequals(yytext, "RESET"))
			return TOKEN_RESET;
	}
	if (yyget_extra(yyscanner)->initialToken == TOKEN_GREATSPN_GENERALPDF) {
		if (boost::iequals(yytext, "x"))
			return TOKEN_PDF_X_VAR;
		if (boost::equals(yytext, "I"))
			return TOKEN_DIRACDELTA_FN;
		if (boost::equals(yytext, "R"))
			return TOKEN_RECT_FN;
	}
	return TOKEN_ID;
	//return TOKEN_INVALID;
}
"\""[^\n"\""]*"\""		{   /* return the string content, without the "" */
	UNUSED(yyunput); auto x = yyinput; UNUSED(x); // Make GCC happy...
	yylval_param->text.assign(yytext+1, yytext+strlen(yytext)-1);
	return TOKEN_STRING;
	//return TOKEN_INVALID;
}
<<EOF>>				{   return TOKEN_END;   }
.					{	return TOKEN_INVALID; /* The catch-what-remains rule. */  }
%%



