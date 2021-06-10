//----------------------------------------------------------------------------
#ifndef CTL_H_
#define CTL_H_
//----------------------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <vector>
#include <iterator>
#include <unordered_set>

#ifdef NDEBUG
# define CTL_ASSERT(x) {}
#endif

#include "rgmedd5.h"
#include "LTL.h"

#define DOUBLELEVEL false

extern RSRG *g_rsrg;
extern bool CTL_quiet;
// Model check CTL formulas using CTL operators or CTL* operators
extern bool eval_CTL_using_SatELTL;
extern bool CTL_print_intermediate_sat_sets;
extern bool CTL_print_sat_sets;
extern bool implicitNextForCTLstar;
extern bool CTL_preimg_stays_in_RS;

#ifndef CTL_ASSERT
inline void AssertFailed(const char *f, int l) {
    std::cerr << "Assertion failed at " << f << ":" << l << std::endl;
    throw - 1;
}
# define CTL_ASSERT(x) if(!(x)) { AssertFailed(__FILE__, __LINE__); }
#endif

namespace ctlmdd { class BaseFormula; }
namespace std {
  template<> struct hash<ctlmdd::BaseFormula*>{
    std::size_t operator()(const ctlmdd::BaseFormula* pf) const;
  };
}

// parser/lexer interface
// void initialize_lexer(istringstream *p_iss);
// void deinitialize_lexer();

#undef DEAD

namespace ctlmdd {

using namespace MEDDLY;

//----------------------------------------------------------------------------

class BaseFormula;
class Formula;
class IntFormula;
class PathFormula;
class AtomicProposition;

// State formulas & atomic propositions
class LogicalFormula;
class TemporalFormula;
class QuantifiedFormula;
class BoolLiteral;
class Deadlock;
class InitState;
class Reachability;
class Fireability;
class Inequality;
class GlobalProperty;

// Integer expressions
class IntExpression;
class PlaceTerm;
class IntLiteral;
class BoundOfPlaces;

//----------------------------------------------------------------------------

enum Language {
    CTL, LTL, CTLSTAR,
    NUM_LANGUAGES
};
extern const char* s_languageName[3];

//----------------------------------------------------------------------------

enum TraceType {
    TT_COUNTEREXAMPLE,
    TT_WITNESS
};
inline TraceType oppositeTraceType(TraceType tt) {
    return (tt == TT_COUNTEREXAMPLE) ? TT_WITNESS : TT_COUNTEREXAMPLE;
}
class TreeTraceNode;

//----------------------------------------------------------------------------

// deep comparison of formulas
bool equals(const BaseFormula* pf1, const BaseFormula* pf2);

struct formula_equal_to {
    inline bool operator()(const BaseFormula* pf1, const BaseFormula* pf2) const 
    { return equals(pf1, pf2); }
};

inline bool print_intermediate_expr() {
    return !running_for_MCC() && !CTL_quiet;
}

//-----------------------------------------------------------------------------

/*-----------------
 ---	CTLMDD	---
 ------------------*/

// Additional data (forests) needed for CTL not present in RSRG
class CTLMDD {
private:
    // Singleton instance
    static CTLMDD *instance;
    // The MTMDD forest for integer expression objects
    forest *forestMTMDD;
    // Inserter array (for optimized createEdge operations)
    int **ins;

    CTLMDD();

    // Formula objects cache
    typedef std::unordered_set<BaseFormula*, 
    /**/                       std::hash<ctlmdd::BaseFormula*>, 
    /**/                       formula_equal_to>  formula_cache_t;
    formula_cache_t formula_cache;

public:
    static CTLMDD *getInstance();

    virtual ~CTLMDD();
    void CTLinit();

    // Multi-terminal MDD used by integer formulas
    forest *getMTMDDForest() const;

    int **getIns() const;
    void printStatistics();

    void print_state(const int *marking) const;

    // verify if f is already cached
    bool cache_contains(BaseFormula* f) const;
    // cache or replace f if an identical copy already exists
    BaseFormula* cache_insert(BaseFormula* f);
    // remove from cache
    void cache_remove(BaseFormula* f);
    // number of cached BaseFormula* objects
    size_t cache_size() const;
};

//----------------------------------------------------------------------------

#define DECLARE_CTL_CLASS \
    template <typename T, typename... Args> \
    friend inline ref_ptr<T> \
    ctlnew(Args&&... args)

// Allocate CTL expression nodes
template <typename T, typename... Args>
inline ref_ptr<T> 
ctlnew(Args&&... args) {
    // enable_if_t<is_base_of<BaseFormula, T>::value, T*> 
    static_assert(std::is_base_of<BaseFormula, T>::value, "T must inherit from BaseFormula");
    T* p = new T(args...);
    // cout << "ctlnew("<<(p)<<") "<<(typeid(T).name())<<endl;
    // insert od replace with an existing cached object
    p = (T*)(CTLMDD::getInstance()->cache_insert(p));
    // encapsulate p inside a smart pointer
    return move_to_ref_ptr<T>(std::move(p));
}

//----------------------------------------------------------------------------

enum class StateSetPolicy { ACTUAL, POTENTIAL };

dd_edge MDD_INTERSECT(dd_edge f1, dd_edge f2);
dd_edge MDD_DIFFERENCE(dd_edge f1, dd_edge f2);
dd_edge MDD_UNION(dd_edge f1, dd_edge f2);

/*---------------------
 --- CTL Evaluation Context
 -----------------------*/
class Context {
    mutable dd_edge         fair_states; // set of fair states (that satisfy all fairness constraints)
    mutable unique_ptr<VirtualNSF>  stuttered_NSF;
public:
    RSRG                   *rsrg;  // forest, domains, model data, ...
    dd_edge                 RS;    // the reference RS
    dd_edge                 PS;    // the reference potential state set
    shared_ptr<VirtualNSF>  vNSF;  // pre/post image operator with the NSF
    std::list<dd_edge>      fair_sets;  // fair sets for fair CTL evaluation
    bool                    stutter_EG; // EG phi should be true on dead phi-states
    bool                    stutter_EX; // EX phi should be true on dead phi-states
    bool                    always_use_SatELTL; // use SatELTL for all queries. Otherwise use CTL when possible.
    Language                language;   // sub-language used
    StateSetPolicy          statePol;   // use actual or potential state space in query evaluation
    bool                    verbose;    // should print intermediate steps

    inline Context() {} 
    inline Context(RSRG *_rsrg, dd_edge _RS, dd_edge _PS, shared_ptr<VirtualNSF>&& p, bool _stutter_EG, 
    /**/           bool _stutter_EX, bool _always_use_SatELTL, Language l, 
    /**/           StateSetPolicy _sp, bool _verbose)
    /**/: rsrg(_rsrg), RS(_RS), PS(_PS), vNSF(std::move(p)), stutter_EG(_stutter_EG), 
    /**/  stutter_EX(_stutter_EX), always_use_SatELTL(_always_use_SatELTL), 
    /**/  language(l), statePol(_sp), verbose(_verbose) { }

    // Avoid accidental copies of this object
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

    // add a new fairness constraint and update fair_states
    void add_fairness_constraint(dd_edge fair_set);
    // get the set of fair states
    dd_edge get_fair_states() const;
    // has fairness constraints?
    inline bool has_fairness_constraints() const { return !fair_sets.empty(); }

    // forest MDD of the RS
    inline forest* get_MDD_forest() const { return RS.getForest(); }
    inline forest* get_MxD_forest() const { return vNSF->getForestMxD(); }
    inline const domain* get_domain() const { return get_MDD_forest()->getDomain(); }

    // test for true/false in the context RS
    inline bool is_true(const dd_edge& e) const {
        expert_forest *forest = static_cast<expert_forest *>(e.getForest());
        return e.getNode() == RS.getNode() || e.getNode() == forest->handleForValue(true);
    }
    inline bool is_false(const dd_edge& e) const {
        expert_forest *forest = static_cast<expert_forest *>(e.getForest());
        return e.getNode() == forest->handleForValue(false);
    }

    // empty sat-set MDD
    inline dd_edge empty_set() const {
        return dd_edge(RS.getForest());
    }
    // full potential set MDD
    inline dd_edge potential_set() const { return PS; }
    
    // get actual or potential RS
    inline dd_edge get_state_set() const {
        return (statePol==StateSetPolicy::ACTUAL) ? RS : PS;
    }

    // get stuttered NSF (build if necessary)
    const VirtualNSF* get_stuttered_nsf() const;

    // Computational CTL core algorithms
    dd_edge SELECT_REAL(dd_edge f1) const;
    dd_edge PREIMG(dd_edge f1, bool stuttered) const;
    dd_edge NON_DEAD() const;
    dd_edge DEAD() const;

    dd_edge NOT(dd_edge f1) const;
    dd_edge AND(dd_edge f1, dd_edge f2) const;
    dd_edge AND_NOT(dd_edge f1, dd_edge f2) const;
    dd_edge OR(dd_edge f1, dd_edge f2) const;

    dd_edge EG(dd_edge f1) const;
    dd_edge EU(dd_edge f1, dd_edge f2) const;
    dd_edge EX(dd_edge f1) const;
    dd_edge EF(dd_edge f1) const;

    dd_edge EGfair(dd_edge f1) const;
    dd_edge EXfair(dd_edge f1) const;
    dd_edge EUfair(dd_edge f1, dd_edge f2) const;
    dd_edge ERfair(dd_edge f1, dd_edge f2) const;
    dd_edge EFfair(dd_edge f1) const;

    dd_edge AXfair(dd_edge f1) const;
    dd_edge AFfair(dd_edge f1) const;
    dd_edge AGfair(dd_edge f1) const;
    dd_edge AUfair(dd_edge f1, dd_edge f2) const;
    dd_edge ARfair(dd_edge f1, dd_edge f2) const;
};

dd_edge MDD_INTERSECT(dd_edge f1, dd_edge f2);

// potential state on each model variable, and fixed 0 values for the extra levels
dd_edge mdd_potential_state_set(RSRG* rsrg, forest* forestMDD, bool zero_extra_lvls);

//-----------------------------------------------------------------------------
// NSF with stuttering (self-loops) on the deadlocked markings 
// for the pre_image operation. This NSF is a regular transition relation
// for a Kripke structure.
//-----------------------------------------------------------------------------

class StutteredNSF : public VirtualNSF {
    shared_ptr<VirtualNSF>  petri_net_NSF;
    dd_edge                 dead_markings;
public:
    StutteredNSF(const shared_ptr<VirtualNSF> &_pn_nsf, dd_edge _dm) 
    : petri_net_NSF(_pn_nsf), dead_markings(_dm) { }

    forest* getForestMxD() const override;
    dd_edge pre_image(const dd_edge& set) const override;
    dd_edge post_image(const dd_edge& set) const override;
    dd_edge forward_reachable(const dd_edge& s0) const override;
};

//-----------------------------------------------------------------------------
// Base class of all CTL formulas
//-----------------------------------------------------------------------------

class BaseFormula : public refcounted_base {
private:
    mutable size_t computed_hash;
    mutable bool _is_cached;
    // is this a root/top-level node in the AST? (modifiable, not hashed)
    bool top_level = false;
protected:
    BaseFormula();
    virtual ~BaseFormula();
    void before_delete() override;
    // BaseFormula objects are neither copyable nor relocatable
    BaseFormula(const BaseFormula&) = delete;
    BaseFormula& operator=(const BaseFormula&) = delete;
public:
    BaseFormula(BaseFormula&&) = delete;
    BaseFormula& operator=(BaseFormula&&) = delete;

    virtual bool isBoolFormula() const = 0;
    virtual bool isIntFormula() const = 0;

    // get a hash value for this formula object
    size_t hash() const;

    void set_cached(bool);
    inline bool is_cached() const { return _is_cached; }

    inline bool is_top_level() const { return top_level; };
    inline void set_top_level(bool t) { top_level = t; }
private:
    virtual bool equals(const BaseFormula* pf) const = 0;
    virtual size_t compute_hash() const = 0;

private:
    virtual void print(std::ostream &os) const = 0;

    friend std::ostream &operator << (std::ostream &os, const BaseFormula &ex);
    friend bool equals(const BaseFormula*, const BaseFormula*);
};

//----------------------------------------------------------------------------

// Used by parser to indirectly store non-POD objects
// struct int_formula_objid { int id; };
// struct formula_objid { int id; };
struct ref_formula_objid { int id; };
struct ref_int_formula_objid { int id; };

ref_ptr<BaseFormula> parse_formula(const std::string& formula);

//--------------------------
enum quant_type {
    QOP_EXISTS, QOP_ALWAYS
};
extern const char* g_quant_type_str[2];

enum path_op_type {
    POT_NEXT, POT_GLOBALLY, POT_FUTURE, POT_UNTIL
};
extern const char* g_path_op_type[4];

//-----------------------------------------------------------------------------
// State/path formulae
//-----------------------------------------------------------------------------
// Base for all Boolean formulae
class Formula : public BaseFormula {
    // sat-set of the formula
    dd_edge SatMDD;
    bool computedMDD = false;
    
    // create the sat-set by evaluating the formula and its sub-formulas
    virtual void createMDD(Context& ctx) = 0;
protected:
    void setMDD(dd_edge newMDD);
    void clearMDD();
    Formula();
    virtual ~Formula();
public:
    virtual void print(std::ostream &os) const override = 0;

    // Generate a string representation of the maximal path subformula,
    // and collect all the atomic propositions below such subformula
    virtual void maximal_path_subformula(Context& ctx, std::ostream &os, quant_type quantifier,
                                         std::vector<ref_ptr<Formula>>& subformulas) = 0;
    void add_this_as_subformula(std::ostream &os, 
                                std::vector<ref_ptr<Formula>>& subformulas);
    // void add_deadlock_subformula(std::ostream &os, 
    //                              std::vector<ref_ptr<Formula>>& subformulas);

    // Type classification
    virtual bool isBoolFormula() const override;
    virtual bool isIntFormula() const override;

    // Subtype classification of all boolean formulae
    virtual bool isStateFormula() const = 0;
    virtual bool isPathFormula() const = 0;
    virtual bool isAtomicPropos() const = 0;

    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MDD representation of the term
     * */
    const dd_edge& getMDD(Context& ctx);
    bool hasStoredMDD() const;
    const dd_edge& getStoredMDD() const;

    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) = 0;
};

/*---------------------------------
 ---    Tree-like Trace node    ---
 ----------------------------------*/

class TreeTraceNode {
private:
    // Marking
    vector<int>    marking;
    // Next node in the linear trace
    TreeTraceNode *next;
    // Sub formulas (null if formula is an atomic proposition)
    TreeTraceNode *sub_trace1, *sub_trace2;
    // Which formula is this trace  demostraiting ?
    ref_ptr<Formula>  formula;
    // Counterexample of an Existential CTL formula
    bool           isECTL_cntexample;
    // Is a circular trace?
    bool           isCircular;
    // Is this a witness or a counterexample?
    TraceType      traceTy;
public:
    TreeTraceNode();
    TreeTraceNode(const vector<int> &mark, ref_ptr<Formula> f, TraceType tt);
    ~TreeTraceNode();

    inline void set_sub_trace1(TreeTraceNode *ttn) { CTL_ASSERT(sub_trace1 == NULL); sub_trace1 = ttn; }
    inline void set_sub_trace2(TreeTraceNode *ttn) { CTL_ASSERT(sub_trace2 == NULL); sub_trace2 = ttn; }
    inline void set_next(TreeTraceNode *tn)        { CTL_ASSERT(next == NULL); next = tn; }
    inline void set_ECTL_cntexample()              { isECTL_cntexample = true; }
    inline void set_circular()                     { isCircular = true; }
    inline bool has_next()                         { return next != NULL; }

    void print_trace(const char *prefix = "", int nest_level = 0) const;
};

//-----------------------------------------------------------------------------
// Logical formula (and,or,not,imply)
//-----------------------------------------------------------------------------
class LogicalFormula: public Formula {
    DECLARE_CTL_CLASS;
public:
    // Type of logic operator
    enum op_type {
        CBF_NOT = 0, CBF_AND = 1, CBF_OR = 2, CBF_IMPLY = 3
    };
private:
    ref_ptr<Formula> formula1;
    ref_ptr<Formula> formula2; // if op = NOT this value is null
    op_type op;
    virtual void createMDD(Context& ctx) override;
    virtual void print(std::ostream &os) const override;
    virtual void maximal_path_subformula(Context& ctx, std::ostream&, quant_type,
                                         std::vector<ref_ptr<Formula>>&) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    LogicalFormula(ref_ptr<Formula> formula1, ref_ptr<Formula> formula2, op_type op);
    // Constructor for the unary NOT
    LogicalFormula(ref_ptr<Formula> formula1);
    virtual ~LogicalFormula();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;

    inline Formula* getFormula1() const { return formula1.get(); }
    inline Formula* getFormula2() const { return formula2.get(); }
    inline op_type getOp() const { return op; }
};

//-----------------------------------------------------------------------------
// A quantified State formula (exists/forall)
//-----------------------------------------------------------------------------
class QuantifiedFormula: public Formula {
    DECLARE_CTL_CLASS;
private:
    ref_ptr<Formula> formula;
    quant_type quantifier;

    bool do_CTL_model_checking(Context& ctx);
    bool is_CTL() const;
    virtual void createMDD(Context& ctx) override;
    virtual void print(std::ostream &os) const override;
    virtual void maximal_path_subformula(Context& ctx, std::ostream&, quant_type,
                                         std::vector<ref_ptr<Formula>>&) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    QuantifiedFormula(ref_ptr<Formula> _formula, quant_type _quantifier);
    virtual ~QuantifiedFormula();
public:

    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;

    inline Formula* getPathFormula() const { return formula.get(); }
    inline quant_type getQuantifier() const { return quantifier; }
};

//-----------------------------------------------------------------------------
// A formula with a temporal operator (PathFormula)
//-----------------------------------------------------------------------------
class TemporalFormula : public Formula {
    DECLARE_CTL_CLASS;
private:
    ref_ptr<Formula> formula1, formula2;
    path_op_type op;

    virtual void createMDD(Context& ctx) override;
    virtual void print(std::ostream &os) const override;
    virtual void maximal_path_subformula(Context& ctx, std::ostream&, quant_type,
                                         std::vector<ref_ptr<Formula>>&) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    TemporalFormula(ref_ptr<Formula> _formula, path_op_type _op);
    TemporalFormula(ref_ptr<Formula> _formula1, ref_ptr<Formula> _formula2/*, path_op_type _op*/);
public:

    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;

    virtual ~TemporalFormula();

    inline Formula* getFormula1() const { return formula1.get(); }
    inline Formula* getFormula2() const { return formula2.get(); }
    path_op_type getOp() const { return op; }

    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;
};

//-----------------------------------------------------------------------------
// Base class of all atomic propositions.
//-----------------------------------------------------------------------------
class AtomicProposition: public Formula {
    // DECLARE_CTL_CLASS;
protected:
    AtomicProposition() { }
    virtual ~AtomicProposition();
public:
    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;
    virtual void maximal_path_subformula(Context& ctx, std::ostream&, quant_type,
                                         std::vector<ref_ptr<Formula>>&) override;
};

//-----------------------------------------------------------------------------
// Atomic proposition as boolean value.
//-----------------------------------------------------------------------------
class BoolLiteral: public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    bool value;
    virtual void createMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor.
     * @param value a boolean value
     * */
    BoolLiteral(bool value);
    virtual ~BoolLiteral();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual void print(std::ostream &os) const override;
    virtual void maximal_path_subformula(Context& ctx, std::ostream&, quant_type,
                                         std::vector<ref_ptr<Formula>>&) override;
    /**
     * Return the boolean value.
     * @return boolean value
     * */
    bool getValue() const;
};

//-----------------------------------------------------------------------------
// Atomic proposition for deadlock state o deadlock free state.
// If value is true it finds deadlock state, otherwise it finds deadlock free state.
//-----------------------------------------------------------------------------
class Deadlock: public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    bool value;
    virtual void createMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    Deadlock(bool value);
    virtual ~Deadlock();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual void print(std::ostream &os) const override;

    inline bool getValue() const { return  value; }
};

//-----------------------------------------------------------------------------
// Atomic proposition true in the initial marking, and false otherwise.
//-----------------------------------------------------------------------------
class InitState: public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    virtual void createMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    InitState();
    virtual ~InitState();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual void print(std::ostream &os) const override;
};

//-----------------------------------------------------------------------------
// GlobalProperty
//-----------------------------------------------------------------------------
enum global_property_type {
    GPT_HAS_DEADLOCK,
    GPT_QUASI_LIVENESS,
    GPT_STABLE_MARKING,
    GPT_LIVENESS,
    GPT_ONESAFE
};
extern const char *g_global_property_type_name[];
// A global property is either true or false for the whole model
class GlobalProperty: public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    global_property_type type;
    virtual void createMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor.
     * @param value a boolean value
     * */
    GlobalProperty(global_property_type type);
    virtual ~GlobalProperty();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual void print(std::ostream &os) const override;
    /**
     * Return the boolean value.
     * @return boolean value
     * */
    global_property_type getType() const;
};

//-----------------------------------------------------------------------------
// Reachability property
// Tells if a sub-formula is invariantly/possibly true/false in all states. 
// The result is either true or false
//-----------------------------------------------------------------------------
class Reachability: public AtomicProposition {
    DECLARE_CTL_CLASS;
public:
    enum prop_type {
        RPT_INVARIANTLY,   // evaluates to true if *subf is verified for all states of the system
        RPT_IMPOSSIBILITY, // evaluates to true if *subf is never verified for all states of the system
        RPT_POSSIBILITY    // evaluates to true if *subf is verified for some states of the system (at least one)
    };
private:
    ref_ptr<Formula> subf;
    prop_type   type;
    virtual void createMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    Reachability(ref_ptr<Formula> subf, prop_type type);
    virtual ~Reachability();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;

    virtual void print(std::ostream &os) const override;
};


//----------------------------------------------------------------------------
// Enabling of a set of transitions
//-----------------------------------------------------------------------------
class Fireability : public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    std::vector<int> transitions;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
    virtual void createMDD(Context& ctx) override;
protected:
    inline Fireability(const std::vector<int>* t) : transitions(*t)  { }
    virtual ~Fireability();

public:
    virtual void print(std::ostream &os) const override;
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
};

//-----------------------------------------------------------------------------
// Inequality of two integer expressions.
//-----------------------------------------------------------------------------
class Inequality : public AtomicProposition {
    DECLARE_CTL_CLASS;
public:
    // Type of compare operator
    enum op_type {
        IOP_MIN = 0, IOP_MAJ = 1, IOP_MINEQ = 2, IOP_MAJEQ = 3,
        IOP_EQ = 4,  IOP_NEQ = 5, IOP_SIM = 6,   IOP_DIF = 7
    };
    static const char *OP_Names[8]; // symbols of op_type
private:
    op_type op;
    ref_ptr<IntFormula> expr1;
    ref_ptr<IntFormula> expr2;
    float constant;
    virtual void createMDD(Context& ctx) override;
    void createMDDByComplement(Context& ctx);
    virtual void print(std::ostream &os) const override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    Inequality(op_type op, ref_ptr<IntFormula> expr1, ref_ptr<IntFormula> expr2);
    Inequality(op_type op, ref_ptr<IntFormula> expr1, float constant);
    virtual ~Inequality();    
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;

    inline float getConstant() const { return constant; }
    inline IntFormula *getExpr1() const { return expr1.get(); }
    inline IntFormula *getExpr2() const { return expr2.get(); }
    inline op_type getOp() const { return op; }
};

//-----------------------------------------------------------------------------
// Base class of all integrl expressions, computed on MTMDDs
//-----------------------------------------------------------------------------
class IntFormula : public BaseFormula {
public:
    enum op_type {
        EOP_PLUS = 0, EOP_MINUS = 1, EOP_TIMES = 2, EOP_DIV = 3
    };
    static const char *OP_Names[4]; // symbols of op_type
private:
    dd_edge MTMDD;
    bool computedMTMDD = false;
    virtual void createMTMDD(Context& ctx) = 0;

protected:
    IntFormula();
    virtual ~IntFormula();
public:
    virtual void print(std::ostream &os) const override = 0;
    // Creates the MTMDD if it does not exists yet.
    const dd_edge& getMTMDD(Context& ctx);
    void setMTMDD(dd_edge newMTMDD);
    void clearMTMDD();
    bool hasStoredMTMDD() const;

    virtual bool isBoolFormula() const override;
    virtual bool isIntFormula() const override;
};

//-----------------------------------------------------------------------------
// Expression over integer terms (+, -, *, /)
//-----------------------------------------------------------------------------
class IntExpression: public IntFormula {
    DECLARE_CTL_CLASS;
private:
    ref_ptr<IntFormula> expr1;
    ref_ptr<IntFormula> expr2;
    IntFormula::op_type op;
    virtual void createMTMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    IntExpression(ref_ptr<IntFormula> expr1, ref_ptr<IntFormula> expr2, IntFormula::op_type op =
                  IntFormula::EOP_PLUS);
    virtual ~IntExpression();
public:
    virtual void print(std::ostream &os) const override;
    
    inline IntFormula* getExpr1() const { return expr1.get(); }
    inline IntFormula* getExpr2() const { return expr2.get(); }
    inline IntFormula::op_type getOp() const { return op; }
};

//-----------------------------------------------------------------------------
// Evaluates to the marking of a specified place in every state,
// multiplied/divided by a coeff term:   coeff <op> place
//-----------------------------------------------------------------------------
class PlaceTerm : public IntFormula {
    DECLARE_CTL_CLASS;
private:
    int place; // place index, not MDD variable level
    float coeff;
    IntFormula::op_type op;
    
    virtual void createMTMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    PlaceTerm(float coeff, int place, IntFormula::op_type op = IntFormula::EOP_TIMES);
    virtual ~PlaceTerm();
public:
    inline float getCoeff() const { return coeff; }
    int getMeddlyLevel1based() const;
    inline int getPlace() const { return place; }
    inline IntFormula::op_type getOp() const { return op; }

    virtual void print(std::ostream &os) const override;
};

//-----------------------------------------------------------------------------
// Constant integer values.
// Also base class of BoundOfPlaces.
//-----------------------------------------------------------------------------
class IntLiteral: public IntFormula {
    DECLARE_CTL_CLASS;
private:
    mutable float _constant;
    virtual void createMTMDD(Context& ctx) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    IntLiteral(float constant);
    virtual ~IntLiteral();
public:
    // The actual constant value can be defined differently
    virtual float getConstant() const;

    virtual void print(std::ostream &os) const override;
};

//----------------------------------------------------------------------------
// Upper bound of a set of places
//-----------------------------------------------------------------------------
class BoundOfPlaces : public IntLiteral {
    DECLARE_CTL_CLASS;
private:
    std::vector<int> places;
    mutable std::pair<int, int> _computedBound;

    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    BoundOfPlaces(const std::vector<int>* p);
    virtual ~BoundOfPlaces();
    void compute_bounds() const;
public:
    virtual float getConstant() const override;
    virtual void print(std::ostream &os) const override;

    // methods specific to BoundOfPlaces
    inline int getLowerBound() const { compute_bounds(); return _computedBound.first; }
    inline int getUpperBound() const { compute_bounds(); return _computedBound.second; }
};

//----------------------------------------------------------------------------

inline std::ostream &operator << (std::ostream &os, const BaseFormula &ex) { 
    ex.print(os); 
    return os; 
}

//----------------------------------------------------------------------------
} // namespace ctlmdd
#endif /* CTL_H_ */
