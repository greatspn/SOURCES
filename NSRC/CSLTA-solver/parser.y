%{
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include "common-defs.h"
	#include "asmc.h"
	#include "dta.h"
	#include "cslta.h"
	#define YYERROR_VERBOSE 1
	
	extern int lex_line_count[];
	extern int lex_nest_level;
	void yyerror(const char *str) {
		fprintf(stderr,"error(line %d): %s\n", 
				lex_line_count[lex_nest_level]+1, str);
		//exit(-1);
	}		
	int yywrap() {
		return 1;
	}
	//int main() { return yyparse(); }
	extern void prompt();
	extern int yylex (void);
	
	#define ERR_IFNOT_STRCMP(v1, s) \
		if (0 != strcmp(v1, s)) { yyerror("Expected \""s"\" token."); YYERROR; }
	#define SHOWERR \
		{ const char *err = GS_GetLastError(); \
		  if (err != NULL) { \
			yyerror(err); GS_SetError(""); YYERROR; \
		  } \
		}
	#define free2(a,b)       do { free(a); free(b); } while(0)
	#define free3(a,b,c)     do { free(a); free(b); free(c); } while(0)
	#define free4(a,b,c,d)   do { free(a); free(b); free(c); free(d); } while(0)
%}

%token AND OR NOT STEADY PROB_TA SATISFIES
%token LESS LESS_EQ EQ NOT_EQ GREATER GREATER_EQ DECLCONST
%token ARROW DECLASMC EVAL QUIT_APP SAVE_GML
%token DECLDTA INITLOC FINAL ACTS RESET EXPTRUE EXPFALSE
%token BOUNDARY_ACTION CLEAR RENAME XCLOCK SETOUTPUTDIR SAVE_TRG
%token DECLRESULT SAVE_DSPN LOAD_COORDS PLANARIZE
%token STATE TRANSITION LOCATION EDGE

%union 
{
	char *str;
	IDList *idl;
	IDValList *idvl;
	
	AsmcState *ast;
	AsmcTransition *atrn;
	ASMC *asmc;
	
	DtaLocLabel *dll;
	DtaLoc *dl;
	DtaGuard *dg;
	BOOL reset;
	DtaActSet *das;
	DtaEdge *de;
	DtaCCNamOrd *dccno;
	DTA *dta;

	APValExpr *apve;
	CslTaExpr *expr;
	enum APCmpOp apco;
	ProbabilityConstr pc;
	enum ProbabilityCmpOp pco;
	DTAParams *dtap;
};

%type <idl>     num_id_list
%type <idl>     num_id_list_opt
%type <idl>     uniq_id_list
/*%type <idl>     uniq_id_list_opt*/
%type <idvl>    uniq_id_val_list
%type <idl>     ord_numid_list
%type <idl>     ord_numid_list_opt

%type <ast>		asmc_state
%type <asmc>	asmc_decl
%type <atrn>	asmc_transition

%type <dll>     dta_loc_label
%type <dl>		dta_loc
%type <dg>      inner_guard
%type <dg>      bound_guard
%type <reset>   reset_set
%type <das>     action_set
%type <das>     action_list
%type <de>		dta_edge
%type <dccno>   ccnames_opt
%type <dccno>   partial_ord_opt
%type <dccno>   partial_ord
%type <dta>     dta_decl

%type <apve>    apval_expr
%type <expr>	cslta_expr
%type <apco>    ap_cmp_op
%type <pc>		prob_constr
%type <pco>		prob_cmp_op
%type <dtap>    dta_params
%type <dtap>    cslta_expr_list_opt
%type <dtap>    cslta_expr_list
%type <dtap>    actset_list_opt
%type <dtap>    actset_list
%type <dtap>    ord_ccv_list_opt
%type <dtap>    ord_ccv_list

/*%type <str>     id_or_0*/
%type <str>	    any_num
%type <str>	    num_or_ID
%token <str>	ID
%token <str>	STRING
%token <str>	REALNUM
%token <str>	INTEGER
%token <str>    CONST_NAME

%destructor   { IDList_Delete($$); }		  num_id_list
%destructor   { IDList_Delete($$); }		  num_id_list_opt
%destructor   { IDList_Delete($$); }		  uniq_id_list
/*%destructor   { IDList_Delete($$); }		  uniq_id_list_opt*/
%destructor   { IDValList_Delete($$); }		  uniq_id_val_list
%destructor   { IDList_Delete($$); }		  ord_numid_list
%destructor   { IDList_Delete($$); }		  ord_numid_list_opt
%destructor   { AsmcState_Delete($$); }		  asmc_state
%destructor   { AsmcTransition_Delete($$); }  asmc_transition
%destructor   { Asmc_Delete($$); }			  asmc_decl
%destructor   { DtaLocLabel_Delete($$); }	  dta_loc_label
%destructor   { DtaLoc_Delete($$); }		  dta_loc
%destructor   { DtaGuard_Delete($$); }		  inner_guard
%destructor   { DtaGuard_Delete($$); }		  bound_guard
%destructor   { DtaActSet_Delete($$); }		  action_set
%destructor   { DtaActSet_Delete($$); }		  action_list
%destructor   { DtaEdge_Delete($$); }		  dta_edge
%destructor   { DtaCCNamOrd_Delete($$); }	  ccnames_opt
%destructor   { DtaCCNamOrd_Delete($$); }	  partial_ord_opt
%destructor   { DtaCCNamOrd_Delete($$); }	  partial_ord
%destructor   { Dta_Delete($$); }			  dta_decl
%destructor   { APValExpr_DeleteExpr($$); }	  apval_expr
%destructor   { CslTa_DeleteExpr($$); }		  cslta_expr
%destructor   { DTAParams_Delete($$); }		  dta_params
%destructor   { DTAParams_Delete($$); }		  cslta_expr_list_opt
%destructor   { DTAParams_Delete($$); }		  cslta_expr_list
%destructor   { DTAParams_Delete($$); }		  ord_ccv_list_opt
%destructor   { DTAParams_Delete($$); }		  ord_ccv_list
%destructor   { DTAParams_Delete($$); }		  actset_list_opt
%destructor   { DTAParams_Delete($$); }		  actset_list
%destructor   { free($$); }					  any_num
%destructor   { free($$); }					  num_or_ID
%destructor   { free($$); }					  ID
%destructor   { free($$); }					  STRING
%destructor   { free($$); }					  REALNUM
%destructor   { free($$); }					  INTEGER
%destructor   { free($$); }					  CONST_NAME

/* Operator precedence */
%nonassoc '='
%left '+' '-'
%left '*' '/'
%left UNARY_NEG
%left IMPLY
%left OR
%left AND
%right NOT

%start command_list

/*****************************************************************************/
%%

/* ------------------ COMMANDs ------------------ */

command_list: /* empty */
/**/		| command ';' { prompt(); } command_list 
/**/		| error	';' { prompt(); } command_list	{ yyerrok; }

command: DECLASMC ID '=' '(' asmc_decl ')'			{ Exec_StoreMCObject((MCObject*)$5, $2); free($2); SHOWERR; }
/**/   | DECLDTA ID subst '=' '(' dta_decl ')'		{ Exec_StoreMCObject((MCObject*)$6, $2); free($2); SHOWERR; }
/**    | EVAL '(' ID SATISFIES cslta_expr ')'		{ Exec_EvaluateCslTaExpr($3, $5); free($3); SHOWERR; }*/
/**/   | DECLRESULT ID '=' EVAL '(' ID SATISFIES cslta_expr ')'	
/**/                                                { Exec_EvaluateCslTaExpr($2, $6, NULL, $8); free2($2, $6); SHOWERR; }
/**/   | DECLRESULT ID '=' EVAL '(' ID ',' ID SATISFIES cslta_expr ')'	
/**/                                                { Exec_EvaluateCslTaExpr($2, $6, $8, $10); free3($2, $6, $8); SHOWERR; }
/**/   | DECLCONST ID '=' any_num					{ Exec_StoreMCObject((MCObject*)Constant_New($4), $2); free2($2, $4); SHOWERR; }
/**/   | SAVE_TRG '(' ID ',' STRING ')'				{ Exec_SaveTRG($3, $5); free2($3, $5); SHOWERR; }
/**/   | SAVE_GML '(' ID ',' STRING ')'				{ Exec_SaveMCObjectAsGml($3, $5, ""); free2($3, $5); SHOWERR; }
/**/   | SAVE_GML '(' ID ',' STRING ',' STRING ')'  { Exec_SaveMCObjectAsGml($3, $5, $7); free3($3, $5, $7); SHOWERR; }
/**/   | LOAD_COORDS '(' ID ',' STRING ',' STRING ')' { Exec_LoadDspnCoords($3, $5, $7); free3($3, $5, $7); SHOWERR; }
/**/   | PLANARIZE '(' ID ',' STRING ')'			{ Exec_PlanarizeDspn($3, $5); free2($3, $5); SHOWERR; }
/**/   | SAVE_DSPN '(' ID ',' STRING ',' STRING ',' STRING ')' 
/**/												{ Exec_SaveDspn($3, $5, $7, $9); free4($3, $5, $7, $9); SHOWERR; }
/**/   | CLEAR '(' ID ')'							{ Exec_ClearMCObject($3); free($3); SHOWERR; }
/**/   | CLEAR '(' CONST_NAME ')'					{ Exec_ClearMCObject($3); free($3); SHOWERR; }
/**/   | RENAME '(' ID ',' ID ')'					{ Exec_RenameMCObject($3,$5); free2($3,$5); SHOWERR; }
/**/   | RENAME '(' CONST_NAME ',' ID ')'			{ Exec_RenameMCObject($3,$5); free2($3,$5); SHOWERR; }
/**/   | SETOUTPUTDIR '(' STRING ')'				{ Exec_SetOutputDir($3); free($3); }
/**/   | QUIT_APP									{ YYACCEPT; }


/* ------------- LIST of UNIQUE IDs ------------- */

ord_numid_list_opt: /* empty*/						{ $$ = IDList_New(); }
/**/              | ord_numid_list					{ $$ = $1; }

ord_numid_list: num_or_ID							{ $$ = IDList_AddUniqueID(IDList_New(), $1); free($1); }
/**/          | ord_numid_list LESS num_or_ID		{ $$ = IDList_AddUniqueID($1, $3); free($3); }

/*uniq_id_list_opt: /*empty* /						{ $$ = IDList_New(); }
/** /            | uniq_id_list						{ $$ = $1; }*/

uniq_id_list: ID									{ $$ = IDList_AddUniqueID(IDList_New(), $1); free($1); }
/**/        | uniq_id_list ',' ID					{ $$ = IDList_AddUniqueID($1, $3); free($3); }

uniq_id_val_list: ID								{ $$ = IDValList_AddUniqueIDVal(IDValList_New(), $1, "1"); free($1); }
/**/   | ID '=' num_or_ID							{ $$ = IDValList_AddUniqueIDVal(IDValList_New(), $1, $3); free2($1, $3); }
/**/   | uniq_id_val_list ',' ID					{ $$ = IDValList_AddUniqueIDVal($1, $3, "1"); free($3); }
/**/   | uniq_id_val_list ',' ID '=' num_or_ID		{ $$ = IDValList_AddUniqueIDVal($1, $3, $5); free2($3, $5); }

num_id_list_opt: /*empty*/							{ $$ = IDList_New(); }
/**/           | num_id_list						{ $$ = $1; }

num_id_list: num_or_ID								{ $$ = IDList_AddUniqueID(IDList_New(), $1); free($1); }
/**/       | num_id_list ',' num_or_ID				{ $$ = IDList_AddUniqueID($1, $3); free($3); }

any_num: REALNUM									{ $$ = $1; } 
/**/   | INTEGER									{ $$ = $1; }
/**/   | CONST_NAME									{ $$ = Constant_GetOrErr($1); free($1); SHOWERR; }
/**/   | any_num '+' any_num						{ $$ = Number_Op($1, '+', $3); free2($1, $3); SHOWERR; }
/**/   | any_num '-' any_num						{ $$ = Number_Op($1, '-', $3); free2($1, $3); SHOWERR; }
/**/   | any_num '*' any_num						{ $$ = Number_Op($1, '*', $3); free2($1, $3); SHOWERR; }
/**/   | any_num '/' any_num						{ $$ = Number_Op($1, '/', $3); free2($1, $3); SHOWERR; }
/**/   | '-' any_num %prec UNARY_NEG				{ $$ = Number_Op("0", '-', $2); free($2); SHOWERR; }
/**/   | '(' any_num ')'							{ $$ = $2; }

num_or_ID: any_num { $$ = $1; } | ID { $$ = $1; }


/* -------------- ASMC Declaration -------------- */

asmc_decl: STATE asmc_state 						{ $$ = Asmc_AddState(Asmc_New(), $2); SHOWERR; }
/**/	 | TRANSITION asmc_transition 				{ $$ = Asmc_AddTransition(Asmc_New(), $2); SHOWERR; }
/**/	 | asmc_decl STATE asmc_state				{ $$ = Asmc_AddState($1, $3); SHOWERR; }
/**/	 | asmc_decl TRANSITION asmc_transition		{ $$ = Asmc_AddTransition($1, $3); SHOWERR; }

asmc_state: ID ':' uniq_id_val_list					{ $$ = AsmcState_New($1, $3); free($1); }
/**/	  | ID ':' '{' uniq_id_val_list '}'			{ $$ = AsmcState_New($1, $4); free($1); }
/**/      | ID										{ $$ = AsmcState_New($1, NULL); free($1); }

asmc_transition: ID ARROW ID '(' ID ',' any_num ')'	{ $$ = AsmcTransition_New($1,$3,$5, atof($7)); free4($1, $3, $5, $7); }


/* -------------- DTA Declaration --------------- */

subst: '[' ccnames_opt '|' num_id_list_opt '|' num_id_list_opt ']'	  { GS_StoreTempDtaParams($2, $4, $6); }

ccnames_opt: /* empty * /							{ $$ = DtaCCNamOrd_New(); } */
/**/         ord_numid_list_opt						{ $$ = DtaCCNamOrd_AddSymbols(DtaCCNamOrd_New(), $1, TRUE); }
/**/	   | num_id_list ':' partial_ord_opt		{ $$ = DtaCCNamOrd_AddSymbols($3, $1, FALSE); }

partial_ord_opt: /* empty */						{ $$ = DtaCCNamOrd_New(); }
/**/           | partial_ord						{ $$ = $1; }

partial_ord: ord_numid_list							{ $$ = DtaCCNamOrd_AddPartialOrder(DtaCCNamOrd_New(), $1); }
/**/       | partial_ord ',' ord_numid_list			{ $$ = DtaCCNamOrd_AddPartialOrder($1, $3); }

dta_decl: dta_loc									{ $$ = Dta_AddLocation(Dta_New(), $1); SHOWERR; }
/**/    | dta_edge									{ $$ = Dta_AddEdge(Dta_New(), $1); SHOWERR; }
/**/    | dta_decl dta_loc							{ $$ = Dta_AddLocation($1, $2); SHOWERR; }
/**/    | dta_decl dta_edge							{ $$ = Dta_AddEdge($1, $2); SHOWERR; }

dta_loc: LOCATION ID								{ $$ = DtaLoc_New(0, $2, DtaLocLabel_NewTrue()); free($2); }
/**/   | LOCATION ID ':' dta_loc_label				{ $$ = DtaLoc_New(0, $2, $4); free($2); }
/**/   | INITLOC LOCATION ID						{ $$ = DtaLoc_New(DLF_INITIAL, $3, DtaLocLabel_NewTrue()); free($3); }
/**/   | INITLOC LOCATION ID ':' dta_loc_label		{ $$ = DtaLoc_New(DLF_INITIAL, $3, $5); free($3); }
/**/   | FINAL LOCATION ID							{ $$ = DtaLoc_New(DLF_FINAL, $3, DtaLocLabel_NewTrue()); free($3); }
/**/   | FINAL LOCATION ID ':' dta_loc_label		{ $$ = DtaLoc_New(DLF_FINAL, $3, $5); free($3); }
/**/   | INITLOC FINAL LOCATION ID					{ $$ = DtaLoc_New(DLF_INITIAL|DLF_FINAL, $4, DtaLocLabel_NewTrue()); free($4); }
/**/   | INITLOC FINAL LOCATION ID ':' dta_loc_label{ $$ = DtaLoc_New(DLF_INITIAL|DLF_FINAL, $4, $6); free($4); }

dta_loc_label: ID									{ $$ = DtaLocLabel_NewSpExpr($1); free($1); SHOWERR; }
/**/         | NOT dta_loc_label					{ $$ = DtaLocLabel_NewNotExpr($2); }
/**/		 | '(' dta_loc_label ')'				{ $$ = $2; }
/**/         | dta_loc_label AND dta_loc_label		{ $$ = DtaLocLabel_NewAndExpr($1, $3); }
/**/         | dta_loc_label OR dta_loc_label		{ $$ = DtaLocLabel_NewOrExpr($1, $3); }
/**/         | dta_loc_label IMPLY dta_loc_label	{ $$ = DtaLocLabel_NewImplyExpr($1, $3); }

dta_edge: EDGE ID ARROW ID '(' inner_guard ',' action_set reset_set ')'
/**/												{ $$ = DtaEdge_New($2, $4, $6, $8, $9); free2($2, $4); }
/**/    | EDGE ID ARROW ID '(' bound_guard ',' BOUNDARY_ACTION reset_set ')' 
/**/												{ $$ = DtaEdge_New($2, $4, $6, NULL, $9); free2($2, $4); }

inner_guard: num_or_ID LESS XCLOCK LESS num_or_ID	{ $$ = DtaGuard_NewAB(GF_BETWEEN, $1, $5); free2($1, $5); }
/**/	   | XCLOCK LESS num_or_ID					{ $$ = DtaGuard_NewA(GF_LESS, $3); free($3); }
/**/	   | XCLOCK GREATER num_or_ID  				{ $$ = DtaGuard_NewA(GF_GREATER, $3); free($3); }

bound_guard: XCLOCK '=' num_or_ID					{ $$ = DtaGuard_NewA(GF_EQUAL, $3); free($3); }

/*id_or_0: ID										{ $$ = $1; }
/** /   | INTEGER									{ $$ = $1; ERR_IFNOT_STRCMP($1, "0"); }*/

action_set: action_list								{ $$ = DtaActSet_SetType($1, AST_INCLUDED_ACTs); }
/**/      | '{' action_list '}'						{ $$ = DtaActSet_SetType($2, AST_INCLUDED_ACTs); }
/**/      | ACTS									{ $$ = DtaActSet_SetType(DtaActSet_NewEmpty(), AST_EXCLUDED_ACTs); }
/**/      | ACTS '-' '{' action_list '}'			{ $$ = DtaActSet_SetType($4, AST_EXCLUDED_ACTs); }

action_list: ID										{ $$ = DtaActSet_NewID($1); free($1); }
/**/	   | action_list ',' ID						{ $$ = DtaActSet_AddAction($1, $3); free($3); }

reset_set: /*empty*/								{ $$ = FALSE; }
/**/     | ',' RESET								{ $$ = TRUE; }


/* ------------ CSL-TA Expressions: ------------- */

apval_expr: any_num									{ $$ = APValExpr_NewConst($1); free($1); }
/**/      | ID										{ $$ = APValExpr_NewAtomProp($1); free($1); }
/**/      | apval_expr '+' apval_expr				{ $$ = APValExpr_NewBinOp($1, '+', $3); }
/**/      | apval_expr '-' apval_expr				{ $$ = APValExpr_NewBinOp($1, '-', $3); }
/**/      | apval_expr '*' apval_expr				{ $$ = APValExpr_NewBinOp($1, '*', $3); }
/**/      | apval_expr '/' apval_expr				{ $$ = APValExpr_NewBinOp($1, '/', $3); }
/**/      | apval_expr '%' apval_expr				{ $$ = APValExpr_NewBinOp($1, '%', $3); }
/**/      | '-' apval_expr %prec UNARY_NEG			{ $$ = APValExpr_NewUnOp('-', $2); }
/**/      | '(' apval_expr ')'						{ $$ = $2; }

cslta_expr:	apval_expr								{ $$ = CslTa_NewAPValExpr($1, APCO_NOT_EQ_IMPLICIT, "0"); }
/**/      | apval_expr ap_cmp_op num_or_ID			{ $$ = CslTa_NewAPValExpr($1, $2, $3); free($3); }
/**/	  | EXPTRUE									{ $$ = CslTa_NewTrueFalseExpr(1); }
/**/	  | EXPFALSE								{ $$ = CslTa_NewTrueFalseExpr(0); }
/**/	  | NOT cslta_expr							{ $$ = CslTa_NewNotExpr($2); }
/**/	  | cslta_expr AND cslta_expr				{ $$ = CslTa_NewAndExpr($1, $3); }
/**/	  | cslta_expr OR cslta_expr				{ $$ = CslTa_NewOrExpr($1, $3); }
/**/	  | cslta_expr IMPLY cslta_expr				{ $$ = CslTa_NewImplyExpr($1, $3); }
/**/	  | '(' cslta_expr ')'						{ $$ = $2; }
/**/	  | STEADY prob_constr '(' cslta_expr ')'   { $$ = CslTa_NewSteadyExpr($2, $4); }
/**/	  | PROB_TA prob_constr '(' dta_params ')'  { $$ = CslTa_NewProbTaExpr($2, $4);}

ap_cmp_op: EQ										{ $$ = APCO_EQ; }
/**/     | NOT_EQ									{ $$ = APCO_NOT_EQ; }
/**/     | LESS										{ $$ = APCO_LESS; }
/**/     | LESS_EQ									{ $$ = APCO_LESS_EQ; }
/**/     | GREATER									{ $$ = APCO_GREATER; }
/**/     | GREATER_EQ								{ $$ = APCO_GREATER_EQ; }

prob_cmp_op: LESS_EQ								{ $$ = PCO_LESS_EQ; }
/**/	   | LESS									{ $$ = PCO_LESS; }
/*		   | EQ										{ $$ = PCO_????; } */
/**/	   | GREATER_EQ								{ $$ = PCO_GREATER_EQ; }
/**/	   | GREATER								{ $$ = PCO_GREATER; }

prob_constr: prob_cmp_op any_num					{ $$.value = atof($2); $$.pco = $1; free($2); }

dta_params: ID '[' ord_ccv_list_opt '|' actset_list_opt '|' cslta_expr_list_opt ']'
/**/												{ $$ = DTAParams_MergeParams($1, $3, $5, $7); free($1); SHOWERR; }

cslta_expr_list_opt: /**/							{ $$ = DTAParams_New(); }
/**/			   | cslta_expr_list				{ $$ = $1; }

cslta_expr_list: cslta_expr							{ $$ = DTAParams_AddCslTaExpr(DTAParams_New(), $1); }
/**/		   | cslta_expr_list ',' cslta_expr		{ $$ = DTAParams_AddCslTaExpr($1, $3); }

actset_list_opt: /**/								{ $$ = DTAParams_New(); }
/**/		   | actset_list						{ $$ = $1; }

actset_list: ID										{ $$ = DTAParams_AddActSetByName(DTAParams_New(), $1); free($1); SHOWERR; }
/**/       | '{' uniq_id_list '}'					{ $$ = DTAParams_AddActSetByIdList(DTAParams_New(), $2); SHOWERR; }
/**/       | actset_list ',' ID						{ $$ = DTAParams_AddActSetByName($1, $3); free($3); SHOWERR; }
/**/       | actset_list ',' '{' uniq_id_list '}'	{ $$ = DTAParams_AddActSetByIdList($1, $4); SHOWERR; }

ord_ccv_list_opt: /**/								{ $$ = DTAParams_New(); }
/**/			| ord_ccv_list						{ $$ = $1; }

ord_ccv_list: any_num								{ $$ = DTAParams_AddCCVal(DTAParams_New(), $1); free($1);  SHOWERR; }
/**/		| ord_ccv_list ',' any_num				{ $$ = DTAParams_AddCCVal($1, $3); free($3); SHOWERR; }

%%
/*****************************************************************************/

