%skeleton "lalr1.cc"                          /*  -*- C++ -*- */
%require "2.1a"
%defines
%define "parser_class_name" "GspnFileParser"
//%name-prefix="gspnFile"
%{
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
using namespace std;

#include <boost/smart_ptr.hpp>
using namespace boost;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "ParserDefs.h"
#include "PetriNet.h"
#include "Measure.h"
#include "Language.h"

//#define YYERROR_VERBOSE 1

//#define YYPARSE_PARAM  pLexScanner, struct ParserObj **ppOutObj	
//#define YYLEX_PARAM	   pLexScanner
#define GetPN()		   yyget_extra(pLexScanner)
	
	/*void yyerrorloc(const char *str, struct YYLTYPE *llocp);
#define yyerror(str) yyerrorloc(str, &yyloc)	
	int yywrap() {
		return 1;
	}*/	
%}

%parse-param { yyscan_t pLexScanner }
%parse-param { ParserObj** ppOutObj }
%lex-param   { yyscan_t pLexScanner }
%define "api.pure"
%pure_parser
%locations

%token GREATSPN_MEASURE GREATSPN_MDEPDELAY 
%token GREATSPN_MDEARCMULT GREATSPN_TRNGUARD
%token OPEN_E OPEN_P OPEN_X WHEN EVER IF_FN 
%token ABS_FN MIN_FN MAX_FN MOD_FN SIN_FN COS_FN TAN_FN 
%token ARCSIN_FN ARCCOS_FN ARCTAN_FN EXP_FN LOG_FN 
%token POW_FN SQRT_FN CEIL_FN FLOOR_FN FACTORIAL_FN BINOMIAL_FN
%token LESS LESS_EQ EQ NOT_EQ GREATER GREATER_EQ

%union 
{
	char	*str;
	double   real;
	int      integ;
	
	struct ParserObj			*parser_obj;
	struct MeasureExpr			*msr_expr;
	struct IntConst				*ic;
	struct RealConst			*rc;
	struct IntMDepValue			*mdiv;
	struct RealMDepValue		*mdrc;
	struct MdepCondition		*mdc;
	enum CompareOp				 cmp_op;
	enum UnaryFunct				 una_fn;
	enum BinaryFunct			 bin_fn;
};
%{
	int yylex(yy::GspnFileParser::semantic_type *lvalp, 
			  yy::location *llocp, yyscan_t pLexScanner);
%}

%type <parser_obj>		parser_entry
%type <msr_expr>		measure
%type <ic>				int_const
%type <rc>				real_const
%type <mdiv>			int_marking int_mdexpr
%type <mdrc>			real_mdexpr
%type <mdc>				logic_mdcond
%type <cmp_op>			comp_op
%type <una_fn>			unary_int_fn unary_real_fn
%type <bin_fn>			binary_int_fn binary_real_fn

%token <str>	ID
%token <str>	STRING
%token <real>	REALNUM
%token <integ>	INTEGER
%token <str>    G_RATE_PARAM
%token <str>    G_MARK_PARAM
%token <str>    G_PLACE_ID
%token <str>    G_TRANSITION_ID

%destructor   { delete $$; }	real_const int_const
%destructor   { delete $$; }	int_marking int_mdexpr
%destructor   { delete $$; }	logic_mdcond real_mdexpr
%destructor   { delete $$; }	measure

%destructor   { free($$); }					  ID
%destructor   { free($$); }					  STRING
%destructor   {  }							  REALNUM
%destructor   {  }							  INTEGER

/* Operator precedence */
%nonassoc '='
%left '+' '-'
%left '*' '/'
%left LESS LESS_EQ EQ NOT_EQ GREATER GREATER_EQ 
%left UNARY_NEG
%left '|'
%left '&'
%left '~'

%start parser_entry

/*****************************************************************************/
%%

/* ------------------ PARSEABLE OBJECTS ------------------ */

parser_entry: GREATSPN_MEASURE measure opt_semicolon		{ *ppOutObj = (ParserObj*)$2; }
/**/        | GREATSPN_MDEPDELAY real_mdexpr opt_semicolon	{ *ppOutObj = (ParserObj*)$2; }
/**/        | GREATSPN_MDEARCMULT int_mdexpr opt_semicolon	{ *ppOutObj = (ParserObj*)$2; }
/**/        | GREATSPN_TRNGUARD logic_mdcond opt_semicolon	{ *ppOutObj = (ParserObj*)$2; }

opt_semicolon: | ';'

/* ---------- INT/REAL VALUES AND EXPRESSIONS ------------ */

real_const: REALNUM								{ $$ = new RealConst_Value($1);}
/**/      | INTEGER								{ $$ = new RealConst_Value((double) $1);}
/**/      | G_RATE_PARAM						{ $$ = new RealConst_Param(FindRatePar($1, GetPN())); free($1); }
/**/      | G_MARK_PARAM						{ $$ = new RealConst_IntParam(FindMarkPar($1, GetPN())); free($1); }

int_const: INTEGER								{ $$ = new IntConst_Value($1); }
/**/     | G_MARK_PARAM							{ $$ = new IntConst_Param(FindMarkPar($1, GetPN())); free($1); }

int_marking: '#' G_PLACE_ID						{ $$ = new IntMDepValue_Marking($2, FindPlace($2, GetPN())); free($2); }

int_mdexpr: int_const							{ $$ = new IntMDepValue_Const($1); }
/**/      | int_marking							{ $$ = $1; }
/**/      | '(' int_mdexpr ')'					{ $$ = $2; }
/**/      | int_mdexpr '+' int_mdexpr			{ $$ = new IntMDepValue_Binary($1, BF_PLUS, $3); }
/**/      | int_mdexpr '-' int_mdexpr			{ $$ = new IntMDepValue_Binary($1, BF_MINUS, $3); }
/**/      | int_mdexpr '*' int_mdexpr			{ $$ = new IntMDepValue_Binary($1, BF_MULT, $3); }
/**/      | int_mdexpr '/' int_mdexpr			{ $$ = new IntMDepValue_Binary($1, BF_DIV, $3); }
/**/      | binary_int_fn '[' int_mdexpr ',' int_mdexpr ']'	{ $$ = new IntMDepValue_Binary($3, $1, $5); }
/**/      | '-' int_mdexpr %prec UNARY_NEG		{ $$ = new IntMDepValue_Unary(UF_NEG, $2); }
/**/      | unary_int_fn '[' int_mdexpr ']'		{ $$ = new IntMDepValue_Unary($1, $3); }
/**/	  | IF_FN '[' logic_mdcond ',' int_mdexpr ',' int_mdexpr ']' {
/**/											  $$ = new IntMDepValue_IfThenElse($3, $5, $7); }
/**/      | '{' WHEN logic_mdcond ':' int_mdexpr ';' '}' EVER int_mdexpr ';' {
/**/											  $$ = new IntMDepValue_IfThenElse($3, $5, $9); }

unary_int_fn: ABS_FN							{ $$ = UF_ABS; }
/**/        | FACTORIAL_FN						{ $$ = UF_FACTORIAL; }

binary_int_fn: MIN_FN							{ $$ = BF_MIN; }
/**/         | MAX_FN							{ $$ = BF_MAX; }
/**/         | MOD_FN							{ $$ = BF_MOD; }
/**/         | BINOMIAL_FN						{ $$ = BF_BINOMIAL; }

real_mdexpr: real_const							{ $$ = new RealMDepValue_Const($1); }
/**/       | int_marking						{ $$ = new RealMDepValue_FromMDepInt($1); }
/**/       | '(' real_mdexpr ')'				{ $$ = $2; }
/**/       | real_mdexpr '+' real_mdexpr		{ $$ = new RealMDepValue_Binary($1, BF_PLUS, $3); }
/**/       | real_mdexpr '-' real_mdexpr		{ $$ = new RealMDepValue_Binary($1, BF_MINUS, $3); }
/**/       | real_mdexpr '*' real_mdexpr		{ $$ = new RealMDepValue_Binary($1, BF_MULT, $3); }
/**/       | real_mdexpr '/' real_mdexpr		{ $$ = new RealMDepValue_Binary($1, BF_DIV, $3); }
/**/       | binary_real_fn '[' real_mdexpr ',' real_mdexpr ']'	{ $$ = new RealMDepValue_Binary($3, $1, $5); }
/**/       | '-' real_mdexpr %prec UNARY_NEG	{ $$ = new RealMDepValue_Unary(UF_NEG, $2); }
/**/       | unary_real_fn '[' real_mdexpr ']'	{ $$ = new RealMDepValue_Unary($1, $3); }
/**/       | LOG_FN '[' real_mdexpr ']'	        { $$ = new RealMDepValue_Unary(UF_LOG, $3); }
/**/       | LOG_FN '[' real_mdexpr ',' real_mdexpr ']'	{ $$ = new RealMDepValue_Binary($3, BF_LOG, $5); }
/**/	   | IF_FN '[' logic_mdcond ',' real_mdexpr ',' real_mdexpr ']' {
/**/											  $$ = new RealMDepValue_IfThenElse($3, $5, $7); }
/**/       | '{' WHEN logic_mdcond ':' real_mdexpr ';' '}' EVER real_mdexpr ';' {
/**/											  $$ = new RealMDepValue_IfThenElse($3, $5, $9); }

unary_real_fn: unary_int_fn						{ $$ = $1; }
/**/         | SIN_FN							{ $$ = UF_SIN; }
/**/         | COS_FN							{ $$ = UF_COS; }
/**/         | TAN_FN							{ $$ = UF_TAN; }
/**/         | ARCSIN_FN						{ $$ = UF_ARCSIN; }
/**/         | ARCCOS_FN						{ $$ = UF_ARCCOS; }
/**/         | ARCTAN_FN						{ $$ = UF_ARCTAN; }
/**/         | EXP_FN							{ $$ = UF_EXP; }
/** /         | LOG_FN							{ $$ = UF_LOG; } */
/**/         | SQRT_FN							{ $$ = UF_SQRT; }
/**/         | CEIL_FN							{ $$ = UF_CEIL; }
/**/         | FLOOR_FN							{ $$ = UF_FLOOR; }

binary_real_fn: binary_int_fn					{ $$ = $1; }
/** /          | LOG_FN							{ $$ = BF_LOG; }*/
/**/          | POW_FN							{ $$ = BF_POW; }

/* ----------- LOGIC CONDITIONS OVER MARKINGS ------------ */

logic_mdcond: int_mdexpr comp_op int_mdexpr		{ $$ = new MdepCondition_Compare($1, $2, $3); }
/**/        | '~' logic_mdcond					{ $$ = new MdepCondition_Unary('~', $2); }
/**/        | '(' logic_mdcond ')'				{ $$ = $2; }
/**/        | logic_mdcond '&' logic_mdcond		{ $$ = new MdepCondition_Binary($1, '&', $3); }
/**/        | logic_mdcond '|' logic_mdcond		{ $$ = new MdepCondition_Binary($1, '|', $3); }

comp_op: EQ										{ $$ = CMPOP_EQ; }
/**/   | NOT_EQ									{ $$ = CMPOP_NOT_EQ; }
/**/   | GREATER								{ $$ = CMPOP_GREATER; }
/**/   | LESS									{ $$ = CMPOP_LESS; }
/**/   | GREATER_EQ								{ $$ = CMPOP_GREATER_EQ; }
/**/   | LESS_EQ								{ $$ = CMPOP_LESS_EQ; }

/* ---------------------- MEASURES ----------------------- */

measure: measure '+' measure					{ $$ = new MeasureExpr_Binary($1, '+', $3); }
/**/   | measure '-' measure					{ $$ = new MeasureExpr_Binary($1, '-', $3); }
/**/   | OPEN_P logic_mdcond '}'				{ $$ = new MeasureExpr_P($2); }
/**/   | real_const OPEN_P logic_mdcond '}'		{ $$ = new MeasureExpr_Binary(new MeasureExpr_Real($1), '*', new MeasureExpr_P($3)); }
/**/   | OPEN_E int_mdexpr '}'					{ $$ = new MeasureExpr_E($2, NULL); }
/**/   | real_const OPEN_E int_mdexpr '}'		{ $$ = new MeasureExpr_Binary(new MeasureExpr_Real($1), '*', new MeasureExpr_E($3, NULL)); }
/**/   | OPEN_E int_mdexpr '/' logic_mdcond '}'	{ $$ = new MeasureExpr_E($2, $4); }
/**/   | real_const OPEN_E int_mdexpr '/' logic_mdcond '}' { $$ = new MeasureExpr_Binary(new MeasureExpr_Real($1), '*', new MeasureExpr_E($3, $5)); }
/**/   | OPEN_X G_TRANSITION_ID '}'				{ $$ = new MeasureExpr_X($2, NULL); }
/**/   | real_const OPEN_X G_TRANSITION_ID '}'	{ $$ = new MeasureExpr_Binary(new MeasureExpr_Real($1), '*', new MeasureExpr_X($3, NULL)); }
/**/   | OPEN_X G_TRANSITION_ID '/' logic_mdcond '}'			{ $$ = new MeasureExpr_X($2, $4); }
/**/   | real_const OPEN_X G_TRANSITION_ID '/' logic_mdcond '}'	{ $$ = new MeasureExpr_Binary(new MeasureExpr_Real($1), '*', new MeasureExpr_X($3, $5)); }


%%
/*****************************************************************************/

void
yy::GspnFileParser::error (const yy::GspnFileParser::location_type& l,
                           const std::string& m)
{
	std::cerr << l << ": " << m << std::endl;
}

int yyparse(yyscan_t pLexScanner, ParserObj **ppOutObj) 
{
	yy::GspnFileParser parser(pLexScanner, ppOutObj);
	return parser.parse();
}

/*void yyerrorloc(const char *str, struct YYLTYPE *llocp) {
	fprintf(stderr,"error(line %d): %s\n", llocp->first_line, str);
}*/


