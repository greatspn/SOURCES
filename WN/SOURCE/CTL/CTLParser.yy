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
#include "../SHARED/medd.h"
#define PERFORMANCECTL 1
#define PERFORMANCE 1

using namespace std;
using namespace ctlmdd;

CTLMDD *ctl = CTLMDD::getInstance();
CTLFormula *ris = NULL;
bool comment = false;
std::string NameProp; 
dddRS::RSRG *rsrg;
istringstream *Ibuffer;
FlexLexer* lexer;
// tempi di clock quando inizia la generazione dell'MDD della formula, 
// quando finisce e dopo aver controllato la presenza dello stato iniziale nella formula
clock_t startMDD, endMDD, endMDD2; 
// From the command line
extern bool print_CTL_counterexamples;


void yyerror(const char *str) {
  cout<<"Parse error at \"" << lexer->YYText() << "\": " << str << "." << endl;
}
 
int yylex(void){
    int i = lexer->yylex(); 
    return i;
}
extern int yyparse(void);
extern void getPreTR(const int tID, int *preset);
extern void getInhTR(const int tID, int *set);
extern int output_flag;
extern int Max_Token_Bound;

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
  Term *term;
  Expression *expr;
  // Inequality *ineq;
  AtomicProposition *AP;
  CTLFormula *formula;
  Inequality::op_type inop;
  std::vector<int>* place_id_list;
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
%type <expr> expression
//%type <term> term
%type <place_id_list> place_list
// %type <num> number_expr
%type <AP> atomic_prop
// %type <AP> inequality
%type <inop> ineq_op;
// %type <AP> boolvalue
%type <formula> ctl_formula
%start inizio

%% 
inizio: expression opt_semicolon  { comment = false; ris = new ConstantScalarCTLFormula($1); }
      | ctl_formula opt_semicolon { comment = false; ris = $1; }
      | PROP_NAME VAR             { comment = true; ris = NULL; NameProp = $2; free($2); }
      | /*empty*/                 { comment = true; ris = NULL; }
      ;

opt_semicolon: /*nothing*/ | SEMICOLON ;

l_paren: LPARENT | LQPARENT;
r_paren: RPARENT | RQPARENT;

ctl_formula: atomic_prop                  { $$ = $1; }
          | LPARENT ctl_formula RPARENT   { $$ = $2; }
          | ctl_formula AND ctl_formula   { $$ = new ComplexBoolFormula($1,$3, ComplexBoolFormula::CBF_AND); }
          | ctl_formula OR ctl_formula    { $$ = new ComplexBoolFormula($1,$3, ComplexBoolFormula::CBF_OR); }
          | NOT ctl_formula               { $$ = new ComplexBoolFormula($2); }
          | ctl_formula IMPLY ctl_formula { 
                    // (not ctl_formula1) or ctl_formula2
                    ComplexBoolFormula *nf1 = new ComplexBoolFormula($1);
                    $$ = new ComplexBoolFormula(nf1,$3, ComplexBoolFormula::CBF_OR); }
          | ctl_formula XOR ctl_formula   {
                    // (A or B) and not (A and B)
                    ComplexBoolFormula *AorB = new ComplexBoolFormula($1,$3, ComplexBoolFormula::CBF_OR);
                    ComplexBoolFormula *AandB = new ComplexBoolFormula($1,$3, ComplexBoolFormula::CBF_AND);
                    ComplexBoolFormula *not_AandB = new ComplexBoolFormula(AandB);
                    $$ = new ComplexBoolFormula(AorB,not_AandB, ComplexBoolFormula::CBF_AND); }
          | ctl_formula BIIMPLY ctl_formula {
                    // (A and B) or (not A and not B)
                    ComplexBoolFormula *AandB = new ComplexBoolFormula($1,$3, ComplexBoolFormula::CBF_AND);
                    ComplexBoolFormula *notA = new ComplexBoolFormula($1);
                    ComplexBoolFormula *notB = new ComplexBoolFormula($1);
                    ComplexBoolFormula *notA_and_notB = new ComplexBoolFormula(notA,notB,ComplexBoolFormula::CBF_AND);
                    $$ = new ComplexBoolFormula(AandB,notA_and_notB, ComplexBoolFormula::CBF_OR); }
          | EX ctl_formula              { $$ = new CTLOperation($2, CTLOperation::CTLOP_EX); }
          | EF ctl_formula              { $$ = new CTLOperation($2, CTLOperation::CTLOP_EF); }
          | EG ctl_formula              { $$ = new CTLOperation($2, CTLOperation::CTLOP_EG); }
          | E l_paren ctl_formula U ctl_formula r_paren { $$ = new CTLOperation($3, $5); }
          | AX ctl_formula              { 
                    // not EX not CTLFormula 
                    ComplexBoolFormula *cbf = new ComplexBoolFormula($2); //not CTLFormula
                    CTLOperation *co = new CTLOperation(cbf, CTLOperation::CTLOP_EX); // EX not CTLFormula
                    $$ = new ComplexBoolFormula(co); //not EX not CTLFormula
                }
          | AF ctl_formula              { 
                    // not EG not CTLFormula 
                    ComplexBoolFormula *cbf = new ComplexBoolFormula($2); //not CTLFormula
                    CTLOperation *co = new CTLOperation(cbf, CTLOperation::CTLOP_EG); // EG not CTLFormula
                    $$ = new ComplexBoolFormula(co); //not EG not CTLFormula
                }
          | AG ctl_formula              { 
                    // not (E true U not CTLFormula )  =  not EF not CTLFormula
                    ComplexBoolFormula *cbf = new ComplexBoolFormula($2); //not CTLFormula
                    CTLOperation *co = new CTLOperation(cbf, CTLOperation::CTLOP_EF);
                    $$ = new ComplexBoolFormula(co);
                }
          | A l_paren ctl_formula U ctl_formula r_paren {
                    //not (E not CTLFormula2 U (not CTLFormula1 and not CTLFormula2) ) and not EG not CTLFormula2
                    ComplexBoolFormula *f1 = new ComplexBoolFormula($3); //not CTLFormula1
                    ComplexBoolFormula *f2 = new ComplexBoolFormula($5); //not CTLFormula2
                    ComplexBoolFormula *f1f2 = new ComplexBoolFormula(f1,f2, ComplexBoolFormula::CBF_AND); //(not CTLFormula1 and not CTLFormula2)
                    CTLOperation *eu = new CTLOperation(f2, f1f2); //E not CTLFormula2 U (not CTLFormula1 and not CTLFormula2) 
                    ComplexBoolFormula *neu = new ComplexBoolFormula(eu); //not (E not CTLFormula2 U (not CTLFormula1 and not CTLFormula2) )
                    CTLOperation *eg = new CTLOperation(f2, CTLOperation::CTLOP_EG); // EG not CTLFormula2
                    ComplexBoolFormula *neg = new ComplexBoolFormula(eg); //not EG not CTLFormula2
                    $$ = new ComplexBoolFormula(neu,neg, ComplexBoolFormula::CBF_AND); //total
                }
          | POSSIBLY ctl_formula         { $$ = new Reachability($2, Reachability::RPT_POSSIBILITY); }
          | IMPOSSIBLY ctl_formula       { $$ = new Reachability($2, Reachability::RPT_IMPOSSIBILITY); }
          | INVARIANT ctl_formula        { $$ = new Reachability($2, Reachability::RPT_INVARIANTLY); }
          | ENABLED LPARENT TRANSITION_ID RPARENT  { $$ = enableTR($3); }
          ;

atomic_prop: NDEADLOCK                       { $$ = new Deadlock(false); }
           | DEADLOCK                        { $$ = new Deadlock(true); }
           | TRUEv                           { $$ = new BoolValue(true); }
           | FALSEv                          { $$ = new BoolValue(false); }
           | expression ineq_op expression   { $$ = make_inequality($1, $2, $3); }
           //| term SIM term                   { $$ = new Inequality(Inequality::IOP_SIM, $1, $3); } // TODO: remove ?
           //| term DIF term                   { $$ = new Inequality(Inequality::IOP_DIF, $1, $3); } // TODO: remove ?
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

expression: LPARENT expression RPARENT        { $$ = $2;}
          | opt_sharp PLACE_ID                { $$ = new Term(1, $2, Term::EOP_TIMES); }
          | BOUNDS LPARENT place_list RPARENT { $$ = new ConstantExpr(compute_bound_of_places(*($3))); delete $3; }
          | NUMBER                            { $$ = new ConstantExpr($1); }
          | MARK_PAR                          { $$ = new ConstantExpr(tabmp[$1].mark_val); }
          | MINUS expression  %prec NOT       { $$ = make_expression(new ConstantExpr(0), Expression::EOP_MINUS, $2); }
          | expression TIMES expression       { $$ = make_expression($1, Expression::EOP_TIMES, $3); }
          | expression DIV expression         { $$ = make_expression($1, Expression::EOP_DIV, $3); }
          | expression PLUS expression        { $$ = make_expression($1, Expression::EOP_PLUS, $3); }
          | expression MINUS expression       { $$ = make_expression($1, Expression::EOP_MINUS, $3); }
          ;

//term: opt_sharp PLACE_ID                   { $$ = new Term(1, $2, Term::EOP_TIMES); }
//    // | number_expr TIMES VAR   { $$ = new Term($1, getVarID(std::string($3)), Term::EOP_TIMES); free($3); }
//    // | number_expr DIV VAR     { $$ = new Term($1, getVarID(std::string($3)), Term::EOP_DIV); free($3); }
//    ; 

opt_sharp : /*nothing*/ | SHARP;

// number_expr: NUMBER                         { $$ = $1; }
//            | number_expr PLUS number_expr   { $$ = $1 + $3; }
//            | number_expr MINUS number_expr  { $$ = $1 - $3; }
//            | number_expr TIMES number_expr  { $$ = $1 * $3; }
//            | number_expr DIV number_expr    { $$ = $1 / $3; }
//            ; 

//Comment: VAR                 { NameProp += " " + std::string($1); free($1); }
//       | StringComment VAR   { NameProp += " " + std::string($2); free($2); }
//       ;

//StringComment: VAR                 { NameProp += std::string($1); free($1); }
//             | StringComment VAR   { NameProp += std::string($2); free($2); }
//             ;

%%

// Create an Inequality* object, with some optimizations for the special cases
AtomicProposition* make_inequality(Expression* e1, Inequality::op_type op, Expression* e2) {
    bool e1const = (typeid(*e1) == typeid(ConstantExpr));
    bool e2const = (typeid(*e2) == typeid(ConstantExpr));
    bool e1term = (typeid(*e1) == typeid(Term));
    bool e2term = (typeid(*e2) == typeid(Term));
    // constant <op> constant   ->   can be replaced with true/false
    if (e1const && e2const) {
        float val1 = ((ConstantExpr*)e1)->getConstant();
        float val2 = ((ConstantExpr*)e2)->getConstant();
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
        return new BoolValue(result);
    }
    // constant <op> expression  ->  reverse the operator and build an inequality with constant
    else if (e1const) {
        float val1 = ((ConstantExpr*)e1)->getConstant();
        delete e1;
        return new Inequality(reverse_ineq_op(op), e2, val1);
    }
    // expression <op> constant  ->  inequality with constant
    else if (e2const) {
        float val2 = ((ConstantExpr*)e2)->getConstant();
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

Expression* make_expression(Expression* e1, Expression::op_type op, Expression* e2) {
    bool e1const = (typeid(*e1) == typeid(ConstantExpr));
    bool e2const = (typeid(*e2) == typeid(ConstantExpr));
    bool e2term = (typeid(*e2) == typeid(Term));
    // Terms are constants -> combine them directly
    if (e1const && e2const) {
        float result;
        float val1 = ((ConstantExpr*)e1)->getConstant();
        float val2 = ((ConstantExpr*)e2)->getConstant();
        switch (op) {
            case Expression::EOP_TIMES:   result = val1 * val2;     break;
            case Expression::EOP_DIV:     result = val1 / val2;     break;
            case Expression::EOP_PLUS:    result = val1 + val2;     break;
            case Expression::EOP_MINUS:   result = val1 - val2;     break;
            default: throw;
        }
        delete e1;
        delete e2;
        return new ConstantExpr(result);
    }
    // <constant> <*/> <Term>  ->  combine into a single Term
    else if (e1const && e2term) {
        if (op == Expression::EOP_TIMES || op == Expression::EOP_DIV) {
            int variable = ((Term*)e2)->getVariable();
            float coeff = ((Term*)e2)->getCoeff();
            float val1 = ((ConstantExpr*)e1)->getConstant();
            assert(coeff == 1);
            delete e1;
            delete e2;
            return new Term(val1, variable, op);
        }
    }
    // Otherwise, create a new Expression* object
    return new ComplexExpr(e1, e2, op);
}

int compute_bound_of_places(const std::vector<int>& places) {
    // Just one place in the list:
    if (places.size() == 1)
        return rsrg->getRealBound(places[0]);
    
    // Compute the bound of the set of places (the maximum number of tokens that these
    // places may have in the TRG at the same time). -> MAX( p1 + p2 + .. + pn)
    std::vector<bool> selected_places(npl);
    std::fill(selected_places.begin(), selected_places.end(), false);
    for (int i=0; i<places.size(); i++) {
        //cout << "adding " << rsrg->getPL(places[i] - 1) << " to the list of places for the bound." << endl;
        //cout << "selected_places.size()="<<selected_places.size()<<"  places[i] - 1 = " << (places[i] - 1) << endl;
        assert(selected_places.size() > places[i] - 1);
        if (selected_places[places[i] - 1]) {
            cerr << "The same place cannot appear multiple times in the bounds() list of places." << endl;
            exit(-1);
        }
        selected_places[places[i] - 1] = true;
    }

    return rsrg->computeRealBoundOfPlaces(selected_places);
}


bool is_scalar_formula(const CTLFormula* formula) {
    return (typeid(*formula) == typeid(ConstantScalarCTLFormula));
}

char ctl_result_buffer[32];
const char* format_result(bool is_scalar, bool b_result, int i_result, bool uppercase) {
    if (is_scalar) {
        sprintf(ctl_result_buffer, "%d", i_result);
        return ctl_result_buffer; 
    }
    else
        return b_result ? (uppercase ? "TRUE" : "true") :
                          (uppercase ? "FALSE" : "false");
}

/**
 * Parse and evaluate a CTL formula
 */
CTLFormula *parse_formula(const std::string& formula, bool *b_result, int *i_result) {
    assert(ris == nullptr);
    Ibuffer = new istringstream(formula);
    lexer = new yyFlexLexer(Ibuffer);
    yyparse();
    delete lexer;
    delete Ibuffer;
    CTLFormula *parsedFrm = ris;
    ris = NULL;
    if(parsedFrm != NULL) {
        startMDD = clock();
        bool is_scalar = is_scalar_formula(parsedFrm);
        if (!running_for_MCC()) {
            cout << "Processing: " << (*parsedFrm) << "  ->  " << (is_scalar?"int":"bool") << endl;
        }

        if (is_scalar) { // scalar formula
            ConstantScalarCTLFormula* csf = dynamic_cast<ConstantScalarCTLFormula*>(parsedFrm);
            *i_result = csf->getResult();
            *b_result = true;
        }
        else { // boolean formula
            dd_edge *dd = parsedFrm->getMDD();
            assert(dd != nullptr);
            endMDD = clock();
            parsedFrm->addOwner();

            // Evaluate the CTL expression:  s0 |= formula
            dd_edge r(ctl->getMDDForest());
            try {
                apply(INTERSECTION, *ctl->getInitMark(), *dd, r);
            }
            catch (MEDDLY::error e) {
                cerr << "parse_formula: MIN. MEDDLY Error: " << e.getName() << endl;
                return NULL;
            }
            *b_result = (r.getCardinality() != 0);
            *i_result = -1;
        }
        endMDD2=clock();
    }
    return parsedFrm;
}

/**
 * It parses all formulas into file net_name.ctl and returns a dd_edge
 *
 **/
void CTLParser(dddRS::RSRG *r) {
    rsrg = r;
    std::string filename = rsrg->getPropName();
    ifstream in;
    if (filename == "")
        filename = rsrg->getNetName() + std::string(".ctl");
    in.open(filename.c_str());
    if (!in)
    {
        cout<<"Error opening CTL file: "<<filename<<"\n";
        if (running_for_MCC())
            cout<<"CANNOT_COMPUTE"<<endl;
        exit(EXIT_FAILURE);
    }
    //inizializzo la classe CTL dove ho i riferimenti a rs foreste e domini per tutte le altre classi
    ctl->CTLinit(rsrg->getRS(),rsrg->getInitMark(), rsrg->getNSFt(), 
                 rsrg->getDomain(), rsrg->getPlaceOrd());
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
    while(!in.eof()) {
        getline(in, line);
        if(!line.empty()) {
            bool b_result;
            int i_result;
            comment = false;
            CTLFormula *formula = parse_formula(line, &b_result, &i_result);
            if (!comment)
            {
                if (!running_for_MCC())
                    cout<<"--- "<<line<<" ---"<<endl;
                if (output_flag)
                    fout << "\n ********** " << line << " *********" << endl;
            }
            if(formula == NULL) {
                if (!comment)
                {
                    if (output_flag) 
                        fout << "Error during the computation" << endl;
                    cout<<"Error during the computation"<<endl;
                    if (running_for_MCC()) {
                        if (NameProp.empty())
                            cout<<"CANNOT_COMPUTE DECISION_DIAGRAMS"<<endl;
                        else
                            cout << "FORMULA " << NameProp << " CANNOT_COMPUTE" << endl;
                    }
                    NameProp="";
                }
                //else
                //{
                //    comment=false;
                //}
            }
            else {
                // We have a valid evaluated CTL formula
                bool is_scalar = is_scalar_formula(formula);

                if (!running_for_MCC()) {
                    if (!NameProp.empty())
                        cout << "\tFormula name: " << NameProp << endl;
                    cout << "\tEvaluation: " << format_result(is_scalar, b_result, i_result, false) << endl;
                    if (output_flag) {
                        if (!NameProp.empty())
                            fout << "Formula name: " << NameProp << endl;
                        fout <<  "Evaluation: " << format_result(is_scalar, b_result, i_result, false) << endl;
                    }
                    if (invoked_from_gui()) {
                        cout << "#{GUI}# RESULT " << NameProp << " " 
                             << format_result(is_scalar, b_result, i_result, true) << endl;
                    }
                }
                else { 
                    cout << "FORMULA " << NameProp << " "
                         << format_result(is_scalar, b_result, i_result, true)
                         << " TECHNIQUES DECISION_DIAGRAMS SEQUENTIAL_PROCESSING\n";
                }
                NameProp="";
                // Print all the satisfying markings
                if(output_flag && !is_scalar) {
                    dd_edge dd = *formula->getMDD();
                    enumerator i(dd);
                    int nvar = ctl->getDomain()->getNumVariables();
                    while(i != 0) { // for each marking in the sat set
                        int j;
                        for(j=1; j <= nvar; j++) { // for each place
                            int val = *(i.getAssignments() + j);
                            const std::string& s = rsrg->getPL(j - 1);
                            if(val!=0) 
                                fout << s << "(" << val << ")";
                        }
                        ++i;
                        fout << endl;
                    }
                }
                
                if (!running_for_MCC()) {
#if PERFORMANCECTL
                    if (!is_scalar)
                        cout<<"\tSat-set generation time: "<<setprecision(7)<<(double(endMDD-startMDD))/CLOCKS_PER_SEC<<" sec"<<endl;
                    cout<<"\tEvaluation time: "<<setprecision(7)<<(double(endMDD2-startMDD))/CLOCKS_PER_SEC<<" sec"<<endl;
#endif
                    // Counter-example/witness generation
                    if (print_CTL_counterexamples && !is_scalar) {
                        cout << "\nGenerated " << (b_result ? "witness: " : "counter-example: ") << endl;
                        vector<int> state0(npl + 1);
                        enumerator it0(*rsrg->getInitMark());
                        const int* tmp =it0.getAssignments();
                        std::copy(tmp, tmp+npl + 1, state0.begin());
                        try {
                            TraceType traceTy = (b_result ? TT_WITNESS : TT_COUNTEREXAMPLE);
                            TreeTraceNode *ttn = formula->generateTrace(state0, traceTy);
                            cout << "====== Trace ======" << endl;
                            cout << "Initial state is: ";
                            ctl->print_state(state0.data());
                            cout << endl;
                            ttn->print_trace();
                            cout << endl;
                            delete ttn;
                        }
                        catch(char const* c) {
                            cerr<<"Error: "<<c<<endl; 
                        }
                    }
#if PERFORMANCE
                    ctl->getStatistic();
#endif
                }
                // Release the memory occupied by the CTL formula tree
                formula->removeOwner();
                formula = NULL;
            }

            if (!running_for_MCC())
                cout << endl;
        }
    } // for each line of the input CTL file
    in.close();
    if (fout)
        fout.close();
    cout << "Ok." << endl;
}






// int getVarID(const std::string& v){
//     //MARIETA
//     int val = ctl->getVal(rsrg->getPL(v));
//     //int val = rsrg->getPL(v);
//     //MARIETA
//     if(val == -1)
//     {
//         cout<<"Place "<<v<<" not found"<<endl;
//         exit(EXIT_FAILURE);
//     }
//     return val + 1;
// }

CTLFormula * enableTR(int trn_id)
{
    int nvar = ctl->getDomain()->getNumVariables();
    //int b = Max_Token_Bound;
    int *pre = new int[nvar], *inhib = new int[nvar];
    std::fill_n(pre, nvar, 0);
    std::fill_n(inhib, nvar, 0);
    getPreTR(trn_id,pre);
    getInhTR(trn_id,inhib);
    bool first = true;
    CTLFormula *formula = NULL;
    for(int i = 0; i<nvar; i++) {
        int level_of_i = ctl->getVal(i) + 1;
        if(pre[i] != 0) {
            Term *t = new Term(1, level_of_i, Term::EOP_TIMES);
            Inequality *in = new Inequality(Inequality::IOP_MAJEQ, t, pre[i]);
            CTLFormula *ce;
            if(first) {
                first=false;
                ce = in;
            }
            else
                ce = new ComplexBoolFormula(formula,in, ComplexBoolFormula::CBF_AND);
            formula = ce;
        }
        if(inhib[i] != 0) {
            Term *t = new Term(1, level_of_i, Term::EOP_TIMES);
            Inequality *in = new Inequality(Inequality::IOP_MIN, t, inhib[i]);
            CTLFormula *ce;
            if(first) {
                first=false;
                ce = in;
            }
            else
                ce = new ComplexBoolFormula(formula,in, ComplexBoolFormula::CBF_AND);
            formula = ce;
        }
    }
    delete[] pre;
    delete[] inhib;
    return formula;
}


