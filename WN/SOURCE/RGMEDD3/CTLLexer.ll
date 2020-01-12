%{

#include "CTL.h"
using namespace ctlmdd;
#include "CTLParser.yy.h"

#define register

extern RSRG *rsrg;
int yyFlexLexer::yywrap(void){
	return 1;
}
bool mark_pars_initialized = false;
std::map<std::string, int> mark_pars;
std::string stdyytext;

int lex_identifier(const std::string& token) {
	if (!mark_pars_initialized) {
		for (int j=0; j<nmp; j++)
			mark_pars.insert(make_pair(std::string(tabmp[j].mark_name), j));
		mark_pars_initialized = true;
	}
	// PLACE name
	int id = rsrg->findPlaceIdByName(token.c_str());
	if (-1 != id) {
		assert(id >= 0 && id < npl);
		mmlval.place_id = id; //CTLMDD::getInstance()->convertPlaceToMDDLevel(id) + 1;
		return PLACE_ID;
	}
	// TRANSITION name
	id = rsrg->findTransitionIdByName(token.c_str());
	if (-1 != id) {
		assert(id >= 0 && id < ntr);
		mmlval.transition_id = id;
		return TRANSITION_ID;
	}
	// MARKING PARAMETER name
	std::map<std::string, int>::const_iterator mpit;
	mpit = mark_pars.find(token);
	if (mark_pars.end() != mpit) {
		mmlval.mpar_id = mpit->second;
		return MARK_PAR;
	}
	// GENERIC TEXT
	mmlval.pVar = strdup(token.c_str()); 
	return VAR;
}
%} 
%%
[%]                 {return PROP_NAME;}
"//"[^\n]*          { /* skip comment */ }
[ \t]+              /* ignore whitespace */;
<<EOF>>             {return -1;}
"en"                {return ENABLED;}
"bounds"            {return BOUNDS;}
"deadlock"          {return DEADLOCK;}
"ndeadlock"         {return NDEADLOCK;}
"true"|"True"  		{return TRUEv;}
"false"|"False"     {return FALSEv;}
"or"|"||"           {return OR;}
"and"|"&&"          {return AND;}
"not"|"!"           {return NOT;}
"xor"               {return XOR;}
"->"                {return IMPLY;}
"<->"               {return BIIMPLY;}
"+"                 {return PLUS;}
"-"                 {return MINUS;}
"*"                 {return TIMES;}
"/"                 {return DIV;}
","                 {return COMMA;}
";"                 {return SEMICOLON;}
"<"                 {return MINOR;}
">"                 {return MAJOR;}
"<="                {return MINOREQ;}
">="                {return MAJOREQ;}
"="|"=="            {return EQ;}
"!="                {return NEQ;}
"("                 {return LPARENT;}
")"                 {return RPARENT;}
"["                 {return LQPARENT;}
"]"                 {return RQPARENT;}
"#"                 {return SHARP;}
"A"                 {return A;}
"E"                 {return E;}
"A"[ \t]*"X"        {return AX;}
"A"[ \t]*"F"        {return AF;}
"A"[ \t]*"G"        {return AG;}
"E"[ \t]*"X"        {return EX;}
"E"[ \t]*"F"        {return EF;}
"E"[ \t]*"G"        {return EG;}
"U"                 {return U;}
"possibly"          {return POSSIBLY;}
"impossibly"        {return IMPOSSIBLY;}
"invariantly"       {return INVARIANT;}
[0-9]+("."[0-9]*)?      { mmlval.num= atof(yytext); return NUMBER; }
[A-Za-z_][A-Za-z0-9_]* { 
			stdyytext = yytext; 
			return lex_identifier(stdyytext); }
"\'"[^\']*"\'" { 
			stdyytext = yytext; 
			stdyytext = stdyytext.substr(1, stdyytext.size() - 2); 
			return lex_identifier(stdyytext); }
"\""[^\"]*"\"" { 
			stdyytext = yytext; 
			stdyytext = stdyytext.substr(1, stdyytext.size() - 2); 
			return lex_identifier(stdyytext); }
.           { printf("Parse error: unrecognized token: \"%s\".\n", yytext); exit(-1); }
%%
