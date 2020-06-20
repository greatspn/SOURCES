%{

#include "CTL.h"
using namespace ctlmdd;
#include "CTLParser.yy.h"

#define register

extern RSRG *rsrg;
extern bool parsing_HOA_edge;
int yyFlexLexer::yywrap(void){
	return 1;
}
bool mark_pars_initialized = false;
std::map<std::string, int> mark_pars;
std::string stdyytext;

int lex_identifier(const std::string& token) {
	if (parsing_HOA_edge) {
		// Special HOA edge labels
		if (token == "t")
			return SPOT_ACCEPT_ALL;
	}
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
"or"|"||" 	        {return OR;} 
"and"|"&&"          {return AND;} 
"&"                 {return AMPERSAND;}
"|"                 {return PIPE;}
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
"###" 		        {return LTLStart;}
"#"                 {return SHARP;}
"A"                 {return A;}
"E"                 {return E;}
"U"                 {return U;}
"X"                 {return X;}
"F"                 {return F;}
"G"                 {return G;}
"EX"                {return EX;}
"EF"                {return EF;}
"EG"                {return EG;}
"AX"                {return AX;}
"AF"                {return AF;}
"AG"                {return AG;}
"possibly"          {return POSSIBLY;}
"impossibly"        {return IMPOSSIBLY;}
"invariantly"       {return INVARIANT;}
"HAS_DEADLOCK"      {return HAS_DEADLOCK;}
"QUASI_LIVENESS"    {return QUASI_LIVENESS;}
"STABLE_MARKING"    {return STABLE_MARKING;}
"LIVENESS"          {return LIVENESS;}
"ONESAFE"           {return ONESAFE;}
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
