%{
//#include "../../INCLUDE/var_ext.h"
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <fstream>
#include "CTLLexer.ll.h"
#include "CTL.h"
#include "rgmedd3.h"
#define PERFORMANCECTL 1
#define PERFORMANCE 1

using namespace std;
using namespace ctlmdd;

extern const char* MCC_TECHNIQUES;
CTLMDD *ctl = CTLMDD::getInstance();
static BaseFormula *ris = NULL;
// Is the last parsed line in the CTL file a comment? Or is it a query?
static bool parsed_comment = false;
// Last parsed comment in the CTL file
static std::string property_name; 
RSRG *rsrg;
static istringstream *Ibuffer;
static FlexLexer* lexer;
// tempi di clock quando inizia la generazione dell'MDD della formula, 
// quando finisce e dopo aver controllato la presenza dello stato iniziale nella formula
static clock_t startMDD, endMDD, endMDD2; 
// From the command line
extern bool print_CTL_counterexamples;


void yyerror(const char *str) {
  cout<<"Parse error at \"" << lexer->YYText() << "\": " << str << "." << endl;
}
 
extern "C" int yylex(void){
    int i = lexer->mmlex(); 
    return i;
}
extern int yyparse(void);
extern int output_flag;

// reverse the sign of an inequality (i.e. a < b -> b > a)
inline Inequality::op_type reverse_ineq_op(Inequality::op_type inop) {
    switch (inop) {
        case Inequality::IOP_MIN:       return Inequality::IOP_MAJ;
        case Inequality::IOP_MAJ:       return Inequality::IOP_MIN;
        case Inequality::IOP_MINEQ:     return Inequality::IOP_MAJEQ;
        case Inequality::IOP_MAJEQ:     return Inequality::IOP_MINEQ;
        default:                        return inop; // [not] equal
    }
}

%}
%union{
  float num;
  char *pVar;
  int place_id;
  int mpar_id;
  int transition_id;
  PlaceTerm *term;
  StateFormula *state_formula;
  IntFormula *int_formula;
  Inequality::op_type inop;
  std::vector<int>* place_id_list;
  std::vector<int>* transition_id_list;
}
%token <num> NUMBER
%token <pVar> VAR 
%token <mpar_id> MARK_PAR
%token <place_id> PLACE_ID
%token <transition_id> TRANSITION_ID
%token PROP_NAME
%token PLUS MINUS TIMES DIV MINOR MAJOR MINOREQ MAJOREQ EQ NEQ 
%token OR XOR AND NOT IMPLY BIIMPLY POSSIBLY IMPOSSIBLY INVARIANT
%token LPARENT RPARENT TRUEv FALSEv LQPARENT RQPARENT 
%token DEADLOCK NDEADLOCK ENABLED BOUNDS COMMA
%token SIM DIF SHARP SEMICOLON
%right E A U AX AF AG EX EF EG ENABLED BOUNDS POSSIBLY IMPOSSIBLY INVARIANT
%left IMPLY BIIMPLY
%left OR XOR
%left AND
%right NOT
//%nonassoc SIM DIF
%nonassoc EQ MINOR MAJOR MINOREQ MAJOREQ NEQ 
%left PLUS MINUS
%left TIMES DIV

%type <int_formula> expression
%type <place_id_list> place_list
%type <transition_id_list> transition_list
%type <state_formula> atomic_prop
%type <inop> ineq_op;
%type <state_formula> ctl_formula

%start inizio

%% 
inizio: expression opt_semicolon  { parsed_comment = false; ris = $1; }
      | ctl_formula opt_semicolon { parsed_comment = false; ris = $1; }
      | PROP_NAME VAR             { parsed_comment = true; ris = NULL; property_name = $2; free($2); }
      | /*empty*/                 { parsed_comment = true; ris = NULL; }
      ;

opt_semicolon: /*nothing*/ | SEMICOLON ;

l_paren: LPARENT | LQPARENT;
r_paren: RPARENT | RQPARENT;

ctl_formula: atomic_prop                  { $$ = $1; }
           | LPARENT ctl_formula RPARENT   { $$ = $2; }
           | ctl_formula AND ctl_formula   { $$ = new LogicalFormula($1,$3, LogicalFormula::CBF_AND); }
           | ctl_formula OR ctl_formula    { $$ = new LogicalFormula($1,$3, LogicalFormula::CBF_OR); }
           | NOT ctl_formula               { $$ = new LogicalFormula($2); }
           | ctl_formula IMPLY ctl_formula { 
                    // (not ctl_formula1) or ctl_formula2
                    LogicalFormula *nf1 = new LogicalFormula($1);
                    $$ = new LogicalFormula(nf1,$3, LogicalFormula::CBF_OR); }
           | ctl_formula XOR ctl_formula   {
                    // (A or B) and not (A and B)
                    LogicalFormula *AorB = new LogicalFormula($1,$3, LogicalFormula::CBF_OR);
                    LogicalFormula *AandB = new LogicalFormula($1,$3, LogicalFormula::CBF_AND);
                    LogicalFormula *not_AandB = new LogicalFormula(AandB);
                    $$ = new LogicalFormula(AorB,not_AandB, LogicalFormula::CBF_AND); }
           | ctl_formula BIIMPLY ctl_formula {
                    // (A and B) or (not A and not B)
                    LogicalFormula *AandB = new LogicalFormula($1,$3, LogicalFormula::CBF_AND);
                    LogicalFormula *notA = new LogicalFormula($1);
                    LogicalFormula *notB = new LogicalFormula($1);
                    LogicalFormula *notA_and_notB = new LogicalFormula(notA,notB,LogicalFormula::CBF_AND);
                    $$ = new LogicalFormula(AandB,notA_and_notB, LogicalFormula::CBF_OR); }
           | EX ctl_formula              { $$ = new CTLStateFormula($2, CTLStateFormula::CTLOP_EX); }
           | EF ctl_formula              { $$ = new CTLStateFormula($2, CTLStateFormula::CTLOP_EF); }
           | EG ctl_formula              { $$ = new CTLStateFormula($2, CTLStateFormula::CTLOP_EG); }
           | E l_paren ctl_formula U ctl_formula r_paren { $$ = new CTLStateFormula($3, $5); }
           | AX ctl_formula              { 
                    // not EX not CTLFormula 
                    LogicalFormula *cbf = new LogicalFormula($2); //not CTLFormula
                    CTLStateFormula *co = new CTLStateFormula(cbf, CTLStateFormula::CTLOP_EX); // EX not CTLFormula
                    $$ = new LogicalFormula(co); //not EX not CTLFormula
                }
           | AF ctl_formula              { 
                    // not EG not CTLFormula 
                    LogicalFormula *cbf = new LogicalFormula($2); //not CTLFormula
                    CTLStateFormula *co = new CTLStateFormula(cbf, CTLStateFormula::CTLOP_EG); // EG not CTLFormula
                    $$ = new LogicalFormula(co); //not EG not CTLFormula
                }
           | AG ctl_formula              { 
                    // not (E true U not CTLFormula )  =  not EF not CTLFormula
                    LogicalFormula *cbf = new LogicalFormula($2); //not CTLFormula
                    CTLStateFormula *co = new CTLStateFormula(cbf, CTLStateFormula::CTLOP_EF);
                    $$ = new LogicalFormula(co);
                }
           | A l_paren ctl_formula U ctl_formula r_paren {
                    //not (E not CTLFormula2 U (not CTLFormula1 and not CTLFormula2) ) and not EG not CTLFormula2
                    LogicalFormula *f1 = new LogicalFormula($3); //not CTLFormula1
                    LogicalFormula *f2 = new LogicalFormula($5); //not CTLFormula2
                    LogicalFormula *f1f2 = new LogicalFormula(f1,f2, LogicalFormula::CBF_AND); //(not CTLFormula1 and not CTLFormula2)
                    CTLStateFormula *eu = new CTLStateFormula(f2, f1f2); //E not CTLFormula2 U (not CTLFormula1 and not CTLFormula2) 
                    LogicalFormula *neu = new LogicalFormula(eu); //not (E not CTLFormula2 U (not CTLFormula1 and not CTLFormula2) )
                    CTLStateFormula *eg = new CTLStateFormula(f2, CTLStateFormula::CTLOP_EG); // EG not CTLFormula2
                    LogicalFormula *neg = new LogicalFormula(eg); //not EG not CTLFormula2
                    $$ = new LogicalFormula(neu,neg, LogicalFormula::CBF_AND); //total
                }
           | POSSIBLY ctl_formula         { $$ = new Reachability($2, Reachability::RPT_POSSIBILITY); }
           | IMPOSSIBLY ctl_formula       { $$ = new Reachability($2, Reachability::RPT_IMPOSSIBILITY); }
           | INVARIANT ctl_formula        { $$ = new Reachability($2, Reachability::RPT_INVARIANTLY); }
           ;

atomic_prop: NDEADLOCK                       { $$ = new Deadlock(false); }
           | DEADLOCK                        { $$ = new Deadlock(true); }
           | TRUEv                           { $$ = new BoolLiteral(true); }
           | FALSEv                          { $$ = new BoolLiteral(false); }
           | ENABLED LPARENT transition_list RPARENT  { $$ = new Fireability($3); delete $3; }
           | expression ineq_op expression   { $$ = make_inequality($1, $2, $3); }
           ;

ineq_op: EQ        { $$ = Inequality::IOP_EQ; }
       | MINOR     { $$ = Inequality::IOP_MIN; }
       | MINOREQ   { $$ = Inequality::IOP_MINEQ; }
       | MAJOR     { $$ = Inequality::IOP_MAJ; }
       | MAJOREQ   { $$ = Inequality::IOP_MAJEQ; }
       | NEQ       { $$ = Inequality::IOP_NEQ; }
       ;

place_list: opt_sharp PLACE_ID                  { $$ = new std::vector<int>(); $$->push_back($2); }
          | place_list COMMA opt_sharp PLACE_ID { $$ = $1; $$->push_back($4); }

transition_list: TRANSITION_ID                       { $$ = new std::vector<int>(); $$->push_back($1); }
               | transition_list COMMA TRANSITION_ID { $$ = $1; $$->push_back($3); }

expression: LPARENT expression RPARENT        { $$ = $2;}
          | opt_sharp PLACE_ID                { $$ = new PlaceTerm(1, $2, PlaceTerm::EOP_TIMES); }
          | BOUNDS LPARENT place_list RPARENT { $$ = new BoundOfPlaces($3); delete $3; }
          | NUMBER                            { $$ = new IntLiteral($1); }
          | MARK_PAR                          { $$ = new IntLiteral(tabmp[$1].mark_val); }
          | MINUS expression  %prec NOT       { $$ = make_expression(new IntLiteral(0), IntFormula::EOP_MINUS, $2); }
          | expression TIMES expression       { $$ = make_expression($1, IntFormula::EOP_TIMES, $3); }
          | expression DIV expression         { $$ = make_expression($1, IntFormula::EOP_DIV, $3); }
          | expression PLUS expression        { $$ = make_expression($1, IntFormula::EOP_PLUS, $3); }
          | expression MINUS expression       { $$ = make_expression($1, IntFormula::EOP_MINUS, $3); }
          ;

//term: opt_sharp PLACE_ID                   { $$ = new PlaceTerm(1, $2, PlaceTerm::EOP_TIMES); }
//    // | number_expr TIMES VAR   { $$ = new PlaceTerm($1, getVarID(std::string($3)), PlaceTerm::EOP_TIMES); free($3); }
//    // | number_expr DIV VAR     { $$ = new PlaceTerm($1, getVarID(std::string($3)), PlaceTerm::EOP_DIV); free($3); }
//    ; 

opt_sharp : /*nothing*/ | SHARP;

// number_expr: NUMBER                         { $$ = $1; }
//            | number_expr PLUS number_expr   { $$ = $1 + $3; }
//            | number_expr MINUS number_expr  { $$ = $1 - $3; }
//            | number_expr TIMES number_expr  { $$ = $1 * $3; }
//            | number_expr DIV number_expr    { $$ = $1 / $3; }
//            ; 

//Comment: VAR                 { property_name += " " + std::string($1); free($1); }
//       | StringComment VAR   { property_name += " " + std::string($2); free($2); }
//       ;

//StringComment: VAR                 { property_name += std::string($1); free($1); }
//             | StringComment VAR   { property_name += std::string($2); free($2); }
//             ;

%%

//-----------------------------------------------------------------------------

// Create an Inequality* object, with some optimizations for the special cases
AtomicProposition* make_inequality(IntFormula* e1, Inequality::op_type op, IntFormula* e2) {
    bool e1const = (typeid(*e1) == typeid(IntLiteral));
    bool e2const = (typeid(*e2) == typeid(IntLiteral));
    bool e1term = (typeid(*e1) == typeid(PlaceTerm));
    bool e2term = (typeid(*e2) == typeid(PlaceTerm));
    // constant <op> constant   ->   can be replaced with true/false
    if (e1const && e2const) {
        float val1 = ((IntLiteral*)e1)->getConstant();
        float val2 = ((IntLiteral*)e2)->getConstant();
        delete e1;
        delete e2;
        bool result;
        switch (op) {
            case Inequality::IOP_MIN:     result = val1 < val2;    break;
            case Inequality::IOP_MINEQ:   result = val1 <= val2;   break;
            case Inequality::IOP_MAJ:     result = val1 > val2;    break;
            case Inequality::IOP_MAJEQ:   result = val1 >= val2;   break;
            case Inequality::IOP_EQ:      result = val1 == val2;   break;
            case Inequality::IOP_NEQ:     result = val1 != val2;   break;
            case Inequality::IOP_SIM:     result = val1 == val2;   break;
            case Inequality::IOP_DIF:     result = val1 != val2;   break;
            default: throw;
        }
        return new BoolLiteral(result);
    }
    // constant <op> expression  ->  reverse the operator and build an inequality with constant
    else if (e1const) {
        float val1 = ((IntLiteral*)e1)->getConstant();
        delete e1;
        return new Inequality(reverse_ineq_op(op), e2, val1);
    }
    // expression <op> constant  ->  inequality with constant
    else if (e2const) {
        float val2 = ((IntLiteral*)e2)->getConstant();
        delete e2;
        return new Inequality(op, e1, val2);
    }
    // remaining case:  expression <op> expression
    // Use SIM and DIF if the two expressions are simple terms.    
    if (e1term && e2term) {
        if (op == Inequality::IOP_EQ)
            op = Inequality::IOP_SIM;
        else if (op == Inequality::IOP_NEQ)
            op = Inequality::IOP_DIF;
    }
    return new Inequality(op, e1, e2);

}

//-----------------------------------------------------------------------------

IntFormula* make_expression(IntFormula* e1, IntFormula::op_type op, IntFormula* e2) {
    bool e1const = (typeid(*e1) == typeid(IntLiteral));
    bool e2const = (typeid(*e2) == typeid(IntLiteral));
    bool e2term = (typeid(*e2) == typeid(PlaceTerm));
    // Terms are constants -> combine them directly
    if (e1const && e2const) {
        float result;
        float val1 = ((IntLiteral*)e1)->getConstant();
        float val2 = ((IntLiteral*)e2)->getConstant();
        switch (op) {
            case IntFormula::EOP_TIMES:   result = val1 * val2;     break;
            case IntFormula::EOP_DIV:     result = val1 / val2;     break;
            case IntFormula::EOP_PLUS:    result = val1 + val2;     break;
            case IntFormula::EOP_MINUS:   result = val1 - val2;     break;
            default: throw;
        }
        delete e1;
        delete e2;
        return new IntLiteral(result);
    }
    // <constant> <*/> <PlaceTerm>  ->  combine into a single PlaceTerm
    else if (e1const && e2term) {
        if (op == IntFormula::EOP_TIMES || op == IntFormula::EOP_DIV) {
            int variable = ((PlaceTerm*)e2)->getVariable();
            float coeff = ((PlaceTerm*)e2)->getCoeff();
            float val1 = ((IntLiteral*)e1)->getConstant();
            assert(coeff == 1);
            delete e1;
            delete e2;
            return new PlaceTerm(val1, variable, op);
        }
    }
    // Otherwise, create a new IntFormula* object
    return new IntExpression(e1, e2, op);
}

//-----------------------------------------------------------------------------

char ctl_result_buffer[32];
const char* format_result(const CTLResult& result, bool uppercase) {
    if (result.is_int()) {
        sprintf(ctl_result_buffer, "%d", result.get_int());
        return ctl_result_buffer; 
    }
    else if (result.is_bool())
        return result.get_bool() ? (uppercase ? "TRUE" : "true") :
                                   (uppercase ? "FALSE" : "false");
    else 
        return "?";
}

//-----------------------------------------------------------------------------

// Parse and evaluate a CTL formula
BaseFormula *parse_formula(const std::string& formula, CTLResult *out_result) {
    assert(ris == nullptr);
    Ibuffer = new istringstream(formula);
    lexer = new yyFlexLexer(Ibuffer);
    yyparse();
    delete lexer;
    delete Ibuffer;
    BaseFormula *parsedFrm = ris;
    ris = NULL;
    if(parsedFrm != NULL) {
        startMDD = clock();
        if (!running_for_MCC() && !CTL_quiet) {
            cout << "Processing: " << (*parsedFrm) << "  ->  " 
                 << (parsedFrm->isIntFormula()?"int":"bool") << endl;
        }

        if (parsedFrm->isIntFormula()) { 
            IntLiteral* intFrm = dynamic_cast<IntLiteral*>(parsedFrm);
            out_result->set_int(intFrm->getConstant());
        }
        else { // boolean formula
            StateFormula* boolFrm = dynamic_cast<StateFormula*>(parsedFrm);
            dd_edge dd = boolFrm->getMDD();
            assert(dd.getForest() != nullptr);
            endMDD = clock();
            boolFrm->addOwner();

            // Evaluate the CTL expression:  s0 |= formula
            dd_edge r(rsrg->getForestMDD());

            /*cout << "Markings that satisfy the formula: \n";
            enumerator i(*dd);
            int nvar = rsrg->getDomain()->getNumVariables();
            while(i != 0) { // for each marking in the sat set
                int j;
                for(j=1; j <= nvar; j++) { // for each place
                    int val = *(i.getAssignments() + j);
                    const std::string& s = rsrg->getPL(j - 1);
                    if(val!=0) 
                        cout << s << "(" << val << ")";
                }
                ++i;
                cout << endl;
            }
            cout << endl;*/

            apply(INTERSECTION, rsrg->getInitMark(), dd, r);
            out_result->set_bool(r.getCardinality() != 0);
        }
        endMDD2=clock();
    }
    return parsedFrm;
}

//-----------------------------------------------------------------------------

// Parse input ctl queries read from file. Print the formula result on screen.
void CTLParser(RSRG *r) {
    rsrg = r;
    std::string filename = rsrg->getPropName();
    ifstream in;
    if (filename == "")
        filename = rsrg->getNetName() + std::string("ctl");
    in.open(filename.c_str());
    if (!in)
    {
        cout<<"Error opening CTL file: "<<filename<<"\n";
        if (running_for_MCC())
            cout<<"CANNOT_COMPUTE"<<endl;
        exit(EXIT_FAILURE);
    }
    //inizializzo la classe CTL dove ho i riferimenti a rs foreste e domini per tutte le altre classi
    ctl->CTLinit();
    filename = filename + ".output";
    ofstream fout;
    if (output_flag)
    {
        fout.open(filename.c_str());
        if (!fout)
        {
            cout<<"Error opening output CTL file: "<<filename<<"\n";
            if (running_for_MCC())
                cout<<"CANNOT_COMPUTE"<<endl;
            exit(EXIT_FAILURE);
        }
    }
    std::string line;
    size_t num_eval = 0;
    while(!in.eof()) {
        getline(in, line);
        if(!line.empty()) {
            CTLResult result;
            parsed_comment = false;
            BaseFormula *formula = parse_formula(line, &result);
            if (!parsed_comment) // line is a CTL query
            {
                if (!running_for_MCC() && !CTL_quiet)
                    cout<<"--- "<<line<<" ---"<<endl;
                if (output_flag)
                    fout << "\n ********** " << line << " *********" << endl;
            }

            // Show the result of this CTL formula
            if (formula == NULL) {
                if (!parsed_comment) // line is a CTL query that we couldn't parse
                {
                    if (output_flag) {
                        fout << "Parse error." << endl;
                    }
                    if (!running_for_MCC()) {
                        cout<<"Parse error."<<endl;
                        if (!property_name.empty())
                            cout << "Formula: " << property_name << "  " << (CTL_quiet ? "" : "\n");
                        cout << "\tEvaluation: -" << endl;
                    }
                    else {
                        if (property_name.empty())
                            cout<<"CANNOT_COMPUTE "<<MCC_TECHNIQUES << endl;
                        else
                            cout << "FORMULA " << property_name << " CANNOT_COMPUTE " << MCC_TECHNIQUES << endl;
                    }
                    property_name = "";
                }
            }
            else {
                // We have a valid evaluated CTL formula
                bool is_int_formula = formula->isIntFormula();

                // Regression test
                const char *regression_res = "";
                if (num_eval < rsrg->expected_ctl.size()) { // Regression test
                    if (rsrg->expected_ctl[num_eval].is_undef())
                        regression_res = "   [[??]]";
                    else if (rsrg->expected_ctl[num_eval] == result)
                        regression_res = "  [[OK]]";
                    else {
                        cerr << "\n\nExpected CTL value is " << rsrg->expected_ctl[num_eval] <<", found "<<result<<endl;
                        throw rgmedd_exception("Regression test failed.");
                    }
                }

                if (!running_for_MCC()) {
                    if (!property_name.empty())
                        cout << "Formula name: " << property_name << "  " << (CTL_quiet ? "" : "\n");
                    cout << "\tEvaluation: " << left << setw(8) << format_result(result, false) << regression_res << endl;
                    if (output_flag) {
                        if (!property_name.empty())
                            fout << "Formula name: " << property_name << endl;
                        fout <<  "Evaluation: " << format_result(result, false) << endl;
                    }
                    if (invoked_from_gui()) {
                        cout << "#{GUI}# RESULT " << property_name << " " 
                             << format_result(result, true) << endl;
                    }
                }
                else { 
                    cout << "FORMULA " << property_name << " "
                         << format_result(result, true)
                         << " " << MCC_TECHNIQUES << "\n";
                }
                property_name = "";

                // Print all the satisfying markings
                if(output_flag && !is_int_formula) {
                    StateFormula* state_formula = dynamic_cast<StateFormula*>(formula);
                    dd_edge dd = state_formula->getMDD();
                    rsrg->show_markings(fout, dd);
                }
                
                if (!running_for_MCC() && !CTL_quiet) {
#if PERFORMANCECTL
                    if (!is_int_formula)
                        cout<<"\tSat-set generation time: "<<setprecision(7)<<(double(endMDD-startMDD))/CLOCKS_PER_SEC<<" sec"<<endl;
                    cout<<"\tEvaluation time: "<<setprecision(7)<<(double(endMDD2-startMDD))/CLOCKS_PER_SEC<<" sec"<<endl;
#endif
                    // Counter-example/witness generation
                    if (print_CTL_counterexamples && !is_int_formula) {
                        StateFormula* state_formula = dynamic_cast<StateFormula*>(formula);
                        cout << "\nGenerated " << (result.get_bool() ? "witness: " : "counter-example: ") << endl;
                        vector<int> state0(npl + 1);
                        enumerator it0(rsrg->getInitMark());
                        const int* tmp =it0.getAssignments();
                        std::copy(tmp, tmp+npl + 1, state0.begin());
                        
                        TraceType traceTy = (result.get_bool() ? TT_WITNESS : TT_COUNTEREXAMPLE);
                        TreeTraceNode *ttn = state_formula->generateTrace(state0, traceTy);
                        print_banner(" Trace ");
                        cout << "Initial state is: ";
                        ctl->print_state(state0.data());
                        cout << endl;
                        ttn->print_trace();
                        cout << endl;
                        delete ttn;
                    }
#if PERFORMANCE
                    if (!CTL_quiet)
                        ctl->printStatistics();
#endif
                }
                // Release the memory occupied by the CTL formula tree
                formula->removeOwner();
                formula = NULL;
                num_eval++;
            }

            if (!running_for_MCC() && !CTL_quiet)
                cout << endl;
        }
    } // for each line of the input CTL file
    in.close();
    if (fout)
        fout.close();
    cout << "Ok." << endl;
}

//-----------------------------------------------------------------------------

