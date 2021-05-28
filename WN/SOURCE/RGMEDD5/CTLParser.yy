%{
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <fstream>
#include <optional>
#include <functional>
#include <numeric>
#include "CTLLexer.ll.h"
#include "CTL.h"
#include "rgmedd5.h"
#include "parallel.h"

// #define PERFORMANCECTL 1
// #define PERFORMANCE 1

using namespace std;
using namespace ctlmdd;

// result of the formuls
static BaseFormula *g_parser_result;
// lexer instance
static FlexLexer* s_lexer;

void initialize_lexer(istringstream *p_iss) {
    assert(s_lexer == nullptr);
    s_lexer = new yyFlexLexer(p_iss);
}
void deinitialize_lexer() {
    assert(s_lexer != nullptr);
    delete s_lexer;
    s_lexer = nullptr;
}

extern const std::vector<size_t> *p_spot_ap_to_greatspn_ap_index;
extern const std::vector<Formula*> *p_greatspn_atomic_propositions;

void yyerror(const char *str) {
  cout<<"Parse error at \"" << s_lexer->YYText() << "\": " << str << "." << endl;
}
 
int yylex(void){
    int i = s_lexer->mmlex(); 
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

inline Formula* fix_unquantified_ctlstar_formulas(Formula* f) {
    // if (f->isPathFormula()) { // typeid(*f) != typeid(QuantifiedFormula) && 
    //     f = ctlnew<QuantifiedFormula>(f, QOP_ALWAYS);
	// }
    return f;
}


struct ref_formula_objid { int id; };


//-----------------------------------------------------------------------------
// Indirect non-POD object storage to overcome yacc limitation
//-----------------------------------------------------------------------------
struct objid_key_comparator {
    inline bool operator()(const formula_objid& f1, const formula_objid& f2)const { return f1.id<f2.id;} 
    inline bool operator()(const int_formula_objid& f1, const int_formula_objid& f2)const { return f1.id<f2.id;} 
    inline bool operator()(const ref_formula_objid& f1, const ref_formula_objid& f2)const { return f1.id<f2.id;} 
};
//-----------------------------------------------------------------------------
// from C++ object type to object id
ctlmdd::Formula* objid_to_object_type(formula_objid); // not implemented
ctlmdd::IntFormula* objid_to_object_type(int_formula_objid); // not implemented
ref_ptr<Formula> objid_to_object_type(ref_formula_objid); // not implemented
// from object id to C++ object type
formula_objid object_to_objid_type(ctlmdd::Formula*); // not implemented
int_formula_objid object_to_objid_type(ctlmdd::IntFormula*); // not implemented

template<typename T>
ref_formula_objid object_to_objid_type(ref_ptr<T>) { 
    static_assert(std::is_base_of<Formula, T>::value, "!"); 
    return ref_formula_objid();
}
//-----------------------------------------------------------------------------
// the per-id static map that will store the id->object mapping
template<typename T>
using map_type_of = std::map<T, decltype(objid_to_object_type(T())), objid_key_comparator>;
template<typename T> auto get_map() -> map_type_of<T>*;
// specializations (one for each object id)
template<> map_type_of<formula_objid>* get_map<formula_objid>() 
{ static map_type_of<formula_objid> the_map; return &the_map; }

template<> map_type_of<int_formula_objid>* get_map<int_formula_objid>() 
{ static map_type_of<int_formula_objid> the_map; return &the_map; }

template<> map_type_of<ref_formula_objid>* get_map<ref_formula_objid>() 
{ static map_type_of<ref_formula_objid> the_map; return &the_map; }
//-----------------------------------------------------------------------------
// covariant conversion of derived ref_ptr<> objects into their base ref_ptr<>s
template<typename T>
using ref_ptr_base = decltype(objid_to_object_type(object_to_objid_type( ref_ptr<T>() )));
ctlmdd::Formula* prepare_for_storage(ctlmdd::Formula* f) { return f; }
ctlmdd::IntFormula* prepare_for_storage(ctlmdd::IntFormula* f) { return f; }
template<typename T>
ref_ptr_base<T> prepare_for_storage(ref_ptr<T>& r) { 
    typedef typename ref_ptr_base<T>::value_type TB;
    return dynamic_pointer_cast<TB>(r);
}
//-----------------------------------------------------------------------------
// global unique id generator
static int g_mapped_id_counter = 0;
//-----------------------------------------------------------------------------
// store non-POD object and get its id
template<typename T>
static auto
mput(T f) -> decltype(object_to_objid_type(f)) {
    typedef decltype(object_to_objid_type(f)) objid_t;
    auto formula_map = get_map<objid_t>();
    objid_t oid{ .id = g_mapped_id_counter++ };
    formula_map->emplace(oid, prepare_for_storage(f));
    // cout << "mput<"<<typeid(objid_t).name()<<"> id=" <<oid.id << " size=" << formula_map->size() << endl;
    return oid;
}
//-----------------------------------------------------------------------------
// retrieve non-POD object from its id, end remove the object from the map
template<typename objid_t>
static auto
mget(objid_t oid) -> decltype(objid_to_object_type(oid)) {
    typedef decltype(objid_to_object_type(oid)) T;
    auto formula_map = get_map<objid_t>();
    auto it = formula_map->find(oid);
    assert(it != formula_map->end());
    T o = std::move(it->second);
    formula_map->erase(it);
    // cout << "mget<"<<typeid(objid_t).name()<<"> id=" <<oid.id << " size=" << formula_map->size() << endl;
    return o;
}
//-----------------------------------------------------------------------------
// sanity check for POD conversion tables (one for each object id)
static inline void parse_verify_objid_maps() {
    CTL_ASSERT(get_map<formula_objid>()->size() == 0);
    CTL_ASSERT(get_map<int_formula_objid>()->size() == 0);
}

// template<typename T>
// static decltype(mput<>)

void f(ref_ptr<BaseFormula> f) {}
void g() {
    ref_ptr<LogicalFormula> r;
    f(dynamic_pointer_cast<BaseFormula>(r));

    ref_ptr<LogicalFormula> ref;
    ref_formula_objid oid = mput(ref);
    ref_ptr<Formula> ref2 = mget(oid);
    // auto rr = prepare_for_storage2(ref);
}

//-----------------------------------------------------------------------------
%}
%union{
    // tokens
    float    num;
    char    *pVar;
    int      mpar_id;
    int      place_id;
    int      transition_id;
    // rules
    formula_objid                formula;
    int_formula_objid            int_formula;
    ctlmdd::Inequality::op_type  inop;
    std::vector<int>            *place_id_list;
    std::vector<int>            *transition_id_list;
}

%token <num> NUMBER
%token <pVar> VAR 
%token <mpar_id> MARK_PAR
%token <place_id> PLACE_ID
%token <transition_id> TRANSITION_ID

%token PROP_NAME SPOT_ACCEPT_ALL
%token PLUS MINUS TIMES DIV MINOR MAJOR MINOREQ MAJOREQ EQ NEQ 
%token OR XOR AND NOT IMPLY BIIMPLY POSSIBLY IMPOSSIBLY INVARIANT
%token HAS_DEADLOCK QUASI_LIVENESS STABLE_MARKING LIVENESS ONESAFE
%token LPARENT RPARENT TRUEv FALSEv LQPARENT RQPARENT INITIAL_STATE
%token DEADLOCK NDEADLOCK ENABLED BOUNDS COMMA
%token SHARP SEMICOLON
%token LTLStart // switch to parse spot_expression

//----------------------------------------------------------------------------
// Grammar associativy inspired from:
//   - Standard Grammars for LTL and LDL, Marco Favorito
//   - Spot’s Temporal Logic Formulas, page 14, Alexandre Duret-Lutz
// left associativity: (A + B) + C   right arrociativity: A = (B = C)
// LOWER PRECEDENCE
%left PLUS MINUS
%left TIMES DIV
%nonassoc EQ MINOR MAJOR MINOREQ MAJOREQ NEQ
%right IMPLY BIIMPLY
%left XOR
%left OR SPOT_OR
%left AND SPOT_AND
%right POSSIBLY IMPOSSIBLY INVARIANT
%right U            // W,M,R
%right F G  EF EG  AF AG
%right X    EX AX   // X[!]
%right E A
%right NOT
// HIGHER PRECEDENCE
//----------------------------------------------------------------------------
//ENABLED BOUNDS 

%type <place_id_list>       place_list
%type <transition_id_list>  transition_list
%type <inop>                ineq_op
%type <int_formula>         expression
%type <formula>             ctlstar_formula spot_expression atomic_prop

%start start_rule

%%
start_rule: expression opt_semicolon  { g_parser_result = mget($1); }
          | LTLStart spot_expression  { g_parser_result = mget($2); }
          | ctlstar_formula opt_semicolon { g_parser_result = fix_unquantified_ctlstar_formulas(mget($1)); }
          ;

opt_semicolon: /*nothing*/ | SEMICOLON ;

/** Boolean expression parser for SPOT's edge labels **/
spot_expression: spot_expression SPOT_AND spot_expression  { $$ = mput(ctlnew<LogicalFormula>(mget($1), mget($3), LogicalFormula::CBF_AND));}
           	   | spot_expression SPOT_OR spot_expression   { $$ = mput(ctlnew<LogicalFormula>(mget($1), mget($3), LogicalFormula::CBF_OR)); }
           	   | NOT spot_expression                       { $$ = mput(ctlnew<LogicalFormula>(mget($2))); }
               | LPARENT spot_expression RPARENT           { $$ = $2; }
               | SPOT_ACCEPT_ALL                           { $$ = mput(ctlnew<BoolLiteral>(true)); }
               | NUMBER
               {
                    // Atomic proposition index must be present in the corresponding array
                    assert(p_greatspn_atomic_propositions != nullptr);
                    if ($1 > p_spot_ap_to_greatspn_ap_index->size() || $1 < 0) {
                        throw "ERROR: Atomic Proposition index is not valid."; // "
                    }
                    size_t ap_index = (*p_spot_ap_to_greatspn_ap_index)[$1];
                    ctlmdd::Formula *f = (*p_greatspn_atomic_propositions)[ap_index];
                    f->addOwner();
                    $$ = mput(f);
               }
               ;

/** CTLStar (CTL*) Expression **/
ctlstar_formula: atomic_prop                       { $$ = $1; }
             | LPARENT ctlstar_formula RPARENT     { $$ = $2; }
             | ctlstar_formula AND ctlstar_formula { $$ = mput(ctlnew<LogicalFormula>(mget($1),mget($3), LogicalFormula::CBF_AND)); }
             | ctlstar_formula OR ctlstar_formula  { $$ = mput(ctlnew<LogicalFormula>(mget($1),mget($3), LogicalFormula::CBF_OR)); }
             | NOT ctlstar_formula                 { $$ = mput(ctlnew<LogicalFormula>(mget($2))); }
             | ctlstar_formula IMPLY ctlstar_formula { $$ = mput(ctlnew<LogicalFormula>(mget($1),mget($3), LogicalFormula::CBF_IMPLY)); }
             | ctlstar_formula BIIMPLY ctlstar_formula { m_assert(false, "TODO: BIIMPLY"); }
             | POSSIBLY ctlstar_formula            { $$ = mput(ctlnew<Reachability>(mget($2), Reachability::RPT_POSSIBILITY)); }
             | IMPOSSIBLY ctlstar_formula          { $$ = mput(ctlnew<Reachability>(mget($2), Reachability::RPT_IMPOSSIBILITY)); }
             | INVARIANT ctlstar_formula           { $$ = mput(ctlnew<Reachability>(mget($2), Reachability::RPT_INVARIANTLY)); }
             | A ctlstar_formula                   { $$ = mput(ctlnew<QuantifiedFormula>(mget($2), QOP_ALWAYS)); }
             | E ctlstar_formula                   { $$ = mput(ctlnew<QuantifiedFormula>(mget($2), QOP_EXISTS)); }
             | X ctlstar_formula                   { $$ = mput(ctlnew<TemporalFormula>(mget($2), POT_NEXT)); }
             | G ctlstar_formula                   { $$ = mput(ctlnew<TemporalFormula>(mget($2), POT_GLOBALLY)); }
             | F ctlstar_formula                   { $$ = mput(ctlnew<TemporalFormula>(mget($2), POT_FUTURE)); }
             | ctlstar_formula U ctlstar_formula   { $$ = mput(ctlnew<TemporalFormula>(mget($1), mget($3))); }
             | LQPARENT ctlstar_formula U ctlstar_formula RQPARENT { $$ = mput(ctlnew<TemporalFormula>(mget($2), mget($4))); }
             /* syntactic sugar */
             | EX ctlstar_formula                  { $$ = mput(ctlnew<QuantifiedFormula>(ctlnew<TemporalFormula>(mget($2), POT_NEXT), QOP_EXISTS)); }
             | EG ctlstar_formula                  { $$ = mput(ctlnew<QuantifiedFormula>(ctlnew<TemporalFormula>(mget($2), POT_GLOBALLY), QOP_EXISTS)); }
             | EF ctlstar_formula                  { $$ = mput(ctlnew<QuantifiedFormula>(ctlnew<TemporalFormula>(mget($2), POT_FUTURE), QOP_EXISTS)); }
             | AX ctlstar_formula                  { $$ = mput(ctlnew<QuantifiedFormula>(ctlnew<TemporalFormula>(mget($2), POT_NEXT), QOP_ALWAYS)); }
             | AG ctlstar_formula                  { $$ = mput(ctlnew<QuantifiedFormula>(ctlnew<TemporalFormula>(mget($2), POT_GLOBALLY), QOP_ALWAYS)); }
             | AF ctlstar_formula                  { $$ = mput(ctlnew<QuantifiedFormula>(ctlnew<TemporalFormula>(mget($2), POT_FUTURE), QOP_ALWAYS)); }
             /* global properties */
             | HAS_DEADLOCK                        { $$ = mput(ctlnew<GlobalProperty>(GPT_HAS_DEADLOCK)); }
             | QUASI_LIVENESS                      { $$ = mput(ctlnew<GlobalProperty>(GPT_QUASI_LIVENESS)); }
             | STABLE_MARKING                      { $$ = mput(ctlnew<GlobalProperty>(GPT_STABLE_MARKING)); }
             | LIVENESS                            { $$ = mput(ctlnew<GlobalProperty>(GPT_LIVENESS)); }
             | ONESAFE                             { $$ = mput(ctlnew<GlobalProperty>(GPT_ONESAFE)); }
             ;

atomic_prop: NDEADLOCK                       { $$ = mput(ctlnew<Deadlock>(false)); }
           | DEADLOCK                        { $$ = mput(ctlnew<Deadlock>(true)); }
           | TRUEv                           { $$ = mput(ctlnew<BoolLiteral>(true)); }
           | FALSEv                          { $$ = mput(ctlnew<BoolLiteral>(false)); }
           | INITIAL_STATE                   { $$ = mput(ctlnew<InitState>()); }
           | ENABLED LPARENT transition_list RPARENT  { $$ = mput(ctlnew<Fireability>($3)); delete $3; }
           | expression ineq_op expression   { $$ = mput(make_inequality(mget($1), $2, mget($3))); }
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

transition_list: /* nothing */                       { $$ = new std::vector<int>(); }
               | TRANSITION_ID                       { $$ = new std::vector<int>(); $$->push_back($1); }
               | transition_list COMMA TRANSITION_ID { $$ = $1; $$->push_back($3); }

expression: LPARENT expression RPARENT        { $$ = $2;}
          | opt_sharp PLACE_ID                { $$ = mput(ctlnew<PlaceTerm>(1, $2, PlaceTerm::EOP_TIMES)); }
          | BOUNDS LPARENT place_list RPARENT { $$ = mput(ctlnew<BoundOfPlaces>($3)); delete $3; }
          | NUMBER                            { $$ = mput(ctlnew<IntLiteral>($1)); }
          | MARK_PAR                          { $$ = mput(ctlnew<IntLiteral>(tabmp[$1].mark_val)); }
          | MINUS expression  %prec NOT       { $$ = mput(make_expression(ctlnew<IntLiteral>(0), IntFormula::EOP_MINUS, mget($2))); }
          | expression TIMES expression       { $$ = mput(make_expression(mget($1), IntFormula::EOP_TIMES, mget($3))); }
          | expression DIV expression         { $$ = mput(make_expression(mget($1), IntFormula::EOP_DIV, mget($3))); }
          | expression PLUS expression        { $$ = mput(make_expression(mget($1), IntFormula::EOP_PLUS, mget($3))); }
          | expression MINUS expression       { $$ = mput(make_expression(mget($1), IntFormula::EOP_MINUS, mget($3))); }
          ;


opt_sharp : /*nothing*/ | SHARP;


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
        e1->removeOwner();
        e2->removeOwner();
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
        return ctlnew<BoolLiteral>(result);
    }
    // constant <op> expression  ->  reverse the operator and build an inequality with constant
    else if (e1const) {
        float val1 = ((IntLiteral*)e1)->getConstant();
        e1->removeOwner();
        return ctlnew<Inequality>(reverse_ineq_op(op), e2, val1);
    }
    // expression <op> constant  ->  inequality with constant
    else if (e2const) {
        float val2 = ((IntLiteral*)e2)->getConstant();
        e2->removeOwner();
        return ctlnew<Inequality>(op, e1, val2);
    }
    // remaining case:  expression <op> expression
    // Use SIM and DIF if the two expressions are simple terms.    
    if (e1term && e2term) {
        if (op == Inequality::IOP_EQ)
            op = Inequality::IOP_SIM;
        else if (op == Inequality::IOP_NEQ)
            op = Inequality::IOP_DIF;
    }
    return ctlnew<Inequality>(op, e1, e2);

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
        e1->removeOwner();
        e2->removeOwner();
        return ctlnew<IntLiteral>(result);
    }
    // <constant> <*/> <PlaceTerm>  ->  combine into a single PlaceTerm
    else if (e1const && e2term) {
        if (op == IntFormula::EOP_TIMES || op == IntFormula::EOP_DIV) {
            int variable = ((PlaceTerm*)e2)->getVariable();
            float coeff = ((PlaceTerm*)e2)->getCoeff();
            float val1 = ((IntLiteral*)e1)->getConstant();
            assert(coeff == 1);
            e1->removeOwner();
            e2->removeOwner();
            return ctlnew<PlaceTerm>(val1, variable, op);
        }
    }
    // Otherwise, create an IntFormula* object
    return ctlnew<IntExpression>(e1, e2, op);
}

//-----------------------------------------------------------------------------

BaseFormula* parse_formula() {
    assert(g_parser_result == nullptr);
    yyparse();
    BaseFormula* f = g_parser_result;
    g_parser_result = nullptr;
    parse_verify_objid_maps();
    return f;
}

//-----------------------------------------------------------------------------
