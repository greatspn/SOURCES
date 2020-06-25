/*
 * CTL.h
 *
 *  Created on: 11/gen/2011
 *      Author: Erica Turin
 */

#ifndef CTL_H_
#define CTL_H_

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

#include "rgmedd4.h"
#include "LTL.h"

#define DOUBLELEVEL false

extern RSRG *rsrg;
extern bool CTL_quiet;
// Model check CTL formulas using CTL operators or CTL* operators
extern bool CTL_as_CTLstar;
extern bool CTL_print_intermediate_sat_sets;
extern bool CTL_print_sat_sets;
extern bool LTL_weak_next;

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
void initialize_lexer(istringstream *p_iss);
void deinitialize_lexer();
ctlmdd::BaseFormula* parse_formula();


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
class Reachability;
class Fireability ;
class Inequality ;

// Integer expressions
class IntExpression;
class PlaceTerm ;
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

//-----------------------------------------------------------------------------

inline bool isEmptySet(const dd_edge& e) {
    expert_forest *forest = static_cast<expert_forest *>(e.getForest());
    return e.getNode() == forest->handleForValue(false);
}

//----------------------------------------------------------------------------

// deep comparison of formulas
bool equals(const BaseFormula *pf1, const BaseFormula *pf2);

struct formula_equal_to {
    inline bool operator()(const BaseFormula *pf1, const BaseFormula *pf2) const 
    { return equals(pf1, pf2); }
};

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

    inline bool SatSetContains(const dd_edge &dd, const int *marking) {
        bool isContained;
        rsrg->getForestMDD()->evaluate(dd, marking, isContained);
        return isContained;
    }

    bool cache_contains(BaseFormula* f) const;
    BaseFormula* cache_insert(BaseFormula* f);
    void cache_remove(BaseFormula* f);
    size_t cache_size() const;
};

/*---------------------
 --- CTL Evaluation Context
 -----------------------*/
class Context {
public:
    dd_edge             RS;         // the reference RS
    // dd_edge             NSF;        // next-state function
    std::list<dd_edge>  fair_sets;  // fair sets for fair CTL evaluation
    bool                stutter_EG; // EG phi should be true on dead phi-states
    bool                verbose;    // should print intermediate steps

    inline Context() {} 
    inline Context(dd_edge _RS, bool _stutter_EG, bool _verbose)
    /**/: RS(_RS), stutter_EG(_stutter_EG), verbose(_verbose) { }

    // Avoid accidental copies of this object
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;
};


/*---------------------
 --- Base class of all CTL formulas
 -----------------------*/

class BaseFormula {
private:
    int countOwner;
    mutable size_t computed_hash;
    mutable bool _is_cached;
protected:
    BaseFormula();
    virtual ~BaseFormula();
    // BaseFormula objects are not copyable.
    BaseFormula(const BaseFormula&) = delete;
    BaseFormula& operator=(const BaseFormula&) = delete;
public:
    void addOwner();
    void removeOwner();

    BaseFormula(BaseFormula&&) = default;
    BaseFormula& operator=(BaseFormula&&) = default;

    virtual bool isBoolFormula() const = 0;
    virtual bool isIntFormula() const = 0;

    // get a hash value for this formula object
    size_t hash() const;

    void set_cached(bool);
    inline bool is_cached() const { return _is_cached; }

private:
    virtual bool equals(const BaseFormula* pf) const = 0;
    virtual size_t compute_hash() const = 0;

private:
    virtual void print(std::ostream &os) const = 0;

    friend std::ostream &operator << (std::ostream &os, const BaseFormula &ex);
    friend bool equals(const BaseFormula*, const BaseFormula*);

};

//----------------------------------------------------------------------------

#define DECLARE_CTL_CLASS \
    template <typename T, typename... Args> \
    friend inline enable_if_t<is_base_of<BaseFormula, T>::value, T*> \
    ctlnew(Args&&... args)

#define safe_removeOwner(p) {   if (p != nullptr) { p->removeOwner(); p = nullptr; }   }
#define safe_addOwner(p)    {   if (p != nullptr) { p->addOwner(); }   }

//----------------------------------------------------------------------------

template<typename T> 
class ref_ptr {
    T*    ptr;       // ref-counted pointer to a subclass of BaseFormula
public:
    inline ref_ptr() : ptr(nullptr) {
        static_assert(std::is_base_of<BaseFormula, T>::value, 
                      "type parameter is not a CTL BaseFormula");
    }
    inline ref_ptr(T* _ptr) : ptr(_ptr) { }
    inline ref_ptr(const ref_ptr<T>& ref) : ptr(ref.ptr) { safe_addOwner(ptr); }
    inline ref_ptr(ref_ptr<T>&&) = default;

    inline ~ref_ptr() { safe_removeOwner(ptr); }

    inline T& operator* () { return *ptr; }
    inline T* operator-> () { return ptr; }
    
    inline ref_ptr<T>& operator=(const ref_ptr<T>& ref) {
        if (this != &ref) { // no self assignments
            safe_removeOwner(ptr); 
            ptr = ref.ptr;
            safe_addOwner(ptr);
        }
        return *this;
    }
    inline ref_ptr<T>& operator=(T* _ptr) {
        if (this != _ptr) {
            safe_removeOwner(ptr); 
            ptr = _ptr;
        }
        return *this;
    }
    inline ref_ptr<T>& operator=(ref_ptr<T>&&) = default;

    inline operator bool() const { return (ptr != nullptr); }

    inline const T* get() const { return ptr; }
    inline T* get() { return ptr; }
};

//----------------------------------------------------------------------------

// Allocate CTL expression nodes
template <typename T, typename... Args>
inline enable_if_t<is_base_of<BaseFormula, T>::value, T*> 
ctlnew(Args&&... args) {
    T* p = new T(args...);
    p->addOwner();
    return (T*)(CTLMDD::getInstance()->cache_insert(p));
}


//--------------------------
enum quant_type {
    QOP_EXISTS, QOP_ALWAYS
};
extern const char* g_quant_type_str[2];

enum path_op_type {
    POT_NEXT, POT_GLOBALLY, POT_FUTURE, POT_UNTIL
};
extern const char* g_path_op_type[4];

/*---------------------
 ---	Formula	---
 -----------------------*/
// Base for all Boolean formulae
class Formula : public BaseFormula {
    /**
     * proposition MDD (boolean)
     * */
    dd_edge SatMDD;
    bool computedMDD = false;
    /**
     * create proposition MDD (boolean)
     * */
    virtual void createMDD() = 0;
protected:
    void setMDD(dd_edge newMDD);
    void clearMDD();
    Formula();
    virtual ~Formula();
public:
    virtual void print(std::ostream &os) const override = 0;

    // Generate a string representation of the maximal path subformula,
    // and collect all the atomic propositions below such subformula
    virtual void maximal_path_subformula(std::ostream &os, quant_type quantifier,
                                         std::vector<Formula*>& subformulas) = 0;
    void add_this_as_subformula(std::ostream &os, 
                                std::vector<Formula*>& subformulas);
    // void add_deadlock_subformula(std::ostream &os, 
    //                              std::vector<Formula*>& subformulas);

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
    const dd_edge& getMDD();
    bool hasStoredMDD() const;

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
    Formula  *formula;
    // Counterexample of an Existential CTL formula
    bool           isECTL_cntexample;
    // Is a circular trace?
    bool           isCircular;
    // Is this a witness or a counterexample?
    TraceType      traceTy;
public:
    TreeTraceNode();
    TreeTraceNode(const vector<int> &mark, Formula *f, TraceType tt);
    ~TreeTraceNode();

    inline void set_sub_trace1(TreeTraceNode *ttn) { CTL_ASSERT(sub_trace1 == NULL); sub_trace1 = ttn; }
    inline void set_sub_trace2(TreeTraceNode *ttn) { CTL_ASSERT(sub_trace2 == NULL); sub_trace2 = ttn; }
    inline void set_next(TreeTraceNode *tn)        { CTL_ASSERT(next == NULL); next = tn; }
    inline void set_ECTL_cntexample()              { isECTL_cntexample = true; }
    inline void set_circular()                     { isCircular = true; }
    inline bool has_next()                         { return next != NULL; }

    void print_trace(const char *prefix = "", int nest_level = 0) const;
};

/*-----------------------------
 ---	LogicalFormula	---
 ------------------------------*/
/**
 * Bool expression composed by two CTL formulae linked through operator.
 * This class represent bool expression with form formula1 op formula2 for binary operator (AND and OR).
 * This class represent bool expression with form op formula1 for unary operator (only NOT).
 * */
class LogicalFormula: public Formula {
    DECLARE_CTL_CLASS;
public:
    /**
     * Type of logic operator
     * */
    enum op_type {
        CBF_NOT = 0, CBF_AND = 1, CBF_OR = 2, CBF_IMPLY = 3
    };
private:
    Formula *formula1;
    Formula *formula2; // if op = NOT this value is null
    op_type op;
    virtual void createMDD() override;
    virtual void print(std::ostream &os) const override;
    virtual void maximal_path_subformula(std::ostream&, quant_type,
                                         std::vector<Formula*>&) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor for bool expression with binary operator.
     * @param formula1 value of the first CTL formula
     * @param formula2 value of the second CTL formula
     * @param op value of operator (AND or OR)
     * */
    LogicalFormula(Formula *formula1, Formula *formula2, op_type op);
    /**
     * Constructor for bool expression with unary operator. The operator is NOT.
     * @param formula1 value of the first CTL formula
     *
     * */
    LogicalFormula(Formula *formula1);
    virtual ~LogicalFormula();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;

    /**
     * Return the first CTL formula.
     * @return formula1
     * */
    Formula *getFormula1() const;
   /**
     * Return the second formula.
     * @return formula2
     * */
    Formula *getFormula2() const;
    /**
     * Return the operator value.
     * @return op
     * */
    LogicalFormula::op_type getOp() const;
};

/*-----------------------------
 ---    QuantifiedFormula   ---
 ------------------------------*/
// A quantified State formula
class QuantifiedFormula: public Formula {
    DECLARE_CTL_CLASS;
private:
    Formula *formula;
    quant_type quantifier;

    bool do_CTL_model_checking();
    virtual void createMDD() override;
    virtual void print(std::ostream &os) const override;
    virtual void maximal_path_subformula(std::ostream&, quant_type,
                                         std::vector<Formula*>&) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    QuantifiedFormula(Formula *_formula, quant_type _quantifier);
    virtual ~QuantifiedFormula();
public:

    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;

    Formula *getPathFormula() const;
    quant_type getQuantifier() const;
};

/*-----------------------------
 ---    TemporalFormula   ---
 ------------------------------*/
// A formula with a temporal operator (PathFormula)
class TemporalFormula : public Formula {
    DECLARE_CTL_CLASS;
private:
    Formula *formula1, *formula2;
    path_op_type op;


    virtual void createMDD() override;
    virtual void print(std::ostream &os) const override;
    virtual void maximal_path_subformula(std::ostream&, quant_type,
                                         std::vector<Formula*>&) override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructors for quantified expressionr.
     * */
    TemporalFormula(Formula *_formula, path_op_type _op);
    TemporalFormula(Formula *_formula1, Formula *_formula2/*, path_op_type _op*/);
public:

    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;

    virtual ~TemporalFormula();

    Formula *getFormula1() const;
    Formula *getFormula2() const;
    path_op_type getOp() const;

    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;
};


/*-------------------------
 ---	CTLStateFormula	---
 --------------------------*/
/**
 * Representing operation CTL.
 * Representing operation CTL:
 * EX formula
 * E formula U formula
 * EG formula
 * */
// class CTLStateFormula: public Formula {
// public:
//     /**
//      * This is the type of CTL operator.
//      * */
//     enum op_type {
//         CTLOP_EX = 0, CTLOP_EU = 1, CTLOP_EG = 2, CTLOP_EF = 3
//     };
// private:
//     Formula *formula1;
//     Formula *formula2;
//     op_type op;
//     virtual void createMDD() override;
//     virtual void print(std::ostream &os) const override;

//     void createEXMDD();
//     void createEGMDD();
//     void createEUMDD();
//     void createEFMDD(Formula *formula);
// public:
//     virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
//     /**
//      * Constructor for CTL formula with binary operator EU.
//      * @param formula1 value of the first CTL formula
//      * @param formula2 value of the second CTL formula
//      * */
//     CTLStateFormula(Formula *formula1, Formula *formula2);
//     *
//      * Constructor for CTL formula with unary operator.
//      * @param formula1 value of the CTL formula
//      * @param op value of operator (EX or EG)
//      * 
//     CTLStateFormula(Formula *formula1, CTLStateFormula::op_type op);
//     virtual ~CTLStateFormula();
//     /**
//      * Return the first CTL formula.
//      * @return formula1
//      * */
//     Formula *getFormula1() const;
//     /**
//      * Set the first formula.
//      * @param formula1 the CTL formula to set
//      * */
//     void setFormula1(Formula *formula1);
//     /**
//      * Return the second formula.
//      * @return formula2
//      * */
//     Formula *getFormula2() const;
//     /**
//      * Set the second formula.
//      * @param formula2 the CTL formula to set
//      * */
//     void setFormula2(Formula *formula2);
//     /**
//      * Return the operator value.
//      * @return op
//      * */
//     CTLStateFormula::op_type getOp() const;
//     /**
//      * Set the operator value.
//      *@param op value to set
//      * */
//     void setOp(CTLStateFormula::op_type op);
// };

/*-----------------------------
 ---	AtomicProposition	---
 ------------------------------*/
/**
 * Base class of all atomic propositions.
 * */
class AtomicProposition: public Formula {
    // DECLARE_CTL_CLASS;
protected:
    AtomicProposition() { }
    virtual ~AtomicProposition();
public:
    virtual bool isStateFormula() const override;
    virtual bool isPathFormula() const override;
    virtual bool isAtomicPropos() const override;
    virtual void maximal_path_subformula(std::ostream&, quant_type,
                                         std::vector<Formula*>&) override;
};

/*-------------------------
 ---	BoolLiteral	---
 ---------------------------*/
/**
 * Atomic proposition as boolean value.
 * */

class BoolLiteral: public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    bool value;
    virtual void createMDD() override;
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
    /**
     * Return the boolean value.
     * @return boolean value
     * */
    bool getValue() const;
};

/*-------------------------
 ---	Deadlock	---
 ---------------------------*/
/**
 * Atomic proposition for deadlock state o deadlock free state.
 * If value is true it finds deadlock state, otherwise it finds deadlock free state.
 * */

class Deadlock: public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    bool value;
    virtual void createMDD() override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor.
     * @param value a boolean value
     * */
    Deadlock(bool value);
    virtual ~Deadlock();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    virtual void print(std::ostream &os) const override;
    /**
     * Return the boolean value.
     * @return boolean value
     * */
    bool getValue() const;
};

/*-------------------------
 ---    GlobalProperty    ---
 ---------------------------*/
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
    virtual void createMDD() override;
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

/*-------------------------
 ---    Reachability    ---
 --------------------------*/
/**
 * Tells if a sub-formula is invariantly/possibly true/false
 * in all states of the TRG. Its result is always true or false.
 * */

class Reachability: public AtomicProposition {
    DECLARE_CTL_CLASS;
public:
    enum prop_type {
        RPT_INVARIANTLY,   // evaluates to true if *subf is verified for all states of the system
        RPT_IMPOSSIBILITY, // evaluates to true if *subf is never verified for all states of the system
        RPT_POSSIBILITY    // evaluates to true if *subf is verified for some states of the system (at least one)
    };
private:
    Formula *subf;
    prop_type   type;
    virtual void createMDD() override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    Reachability(Formula *subf, prop_type type);
    virtual ~Reachability();
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;

    virtual void print(std::ostream &os) const override;
};


//----------------------------------------------------------------------------
// Enabling of a set of transitions

class Fireability : public AtomicProposition {
    DECLARE_CTL_CLASS;
private:
    std::vector<int> transitions;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
    virtual void createMDD() override;
protected:
    inline Fireability(const std::vector<int>* t) : transitions(*t)  { }
    virtual ~Fireability();

public:
    virtual void print(std::ostream &os) const override;
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
};

/*---------------------
 ---    Inequality  ---
 ----------------------*/
/**
 * Inequality class. This class representing inequality with the follow form:
 * expression op constant
 *
 * */

class Inequality : public AtomicProposition {
    DECLARE_CTL_CLASS;
public:
    /**
     * Type of compare operator*/
    enum op_type {
        IOP_MIN = 0, IOP_MAJ = 1, IOP_MINEQ = 2, IOP_MAJEQ = 3,
        IOP_EQ = 4,  IOP_NEQ = 5, IOP_SIM = 6,   IOP_DIF = 7
    };
    static const char *OP_Names[8]; // symbols of op_type
private:
    op_type op;
    IntFormula *expr1;
    IntFormula *expr2;
    float constant;
    virtual void createMDD() override;
    void createMDDByComplement();
    virtual void print(std::ostream &os) const override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor.
     * @param op operator
     * @param expr1 expression
     * @param constant value of constant
     * */
    Inequality(op_type op, IntFormula *expr1, IntFormula *expr2);
    /**
     * Constructor.
     * @param op operator
     * @param expr1 expression
     * @param constant value of constant
     * */
    Inequality(op_type op, IntFormula *expr1, float constant);
    virtual ~Inequality();    
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    /**
     * Return the constant value.
     * @return constant value.
     * */
    float getConstant() const;
    /**
     * Return the expression.
     * @return expression.
     * */
    // IntFormula *getExpr() const;
    /**
     * Return the op value.
     * @return op value.
     * */
    op_type getOp() const;
};

/*---------------------
 ---	IntFormula	---
 ----------------------*/
/**
 * Abstract class. This class representing arithmetic expression with tree structure.
 * */
class IntFormula : public BaseFormula {
public:
    /**
     * This is the type of operator
     * */
    enum op_type {
        EOP_PLUS = 0, EOP_MINUS = 1, EOP_TIMES = 2, EOP_DIV = 3
    };
    static const char *OP_Names[4]; // symbols of op_type

    /**
     * Array for translate op_type to string.
     * */
private:
    /**
     * expression MTMDD (real)
     * */
    dd_edge MTMDD;
    bool computedMTMDD = false;
    /**
     * create expression MTMDD (real)
     * */
    virtual void createMTMDD() = 0;

protected:
    IntFormula();
    virtual ~IntFormula();
public:
    virtual void print(std::ostream &os) const override = 0;
    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MTMDD representation of the term
     * */
    const dd_edge& getMTMDD();
    void setMTMDD(dd_edge newMTMDD);
    void clearMTMDD();
    bool hasStoredMTMDD() const;
    virtual bool isBoolFormula() const override;
    virtual bool isIntFormula() const override;
    // virtual bool isStateFormula() const override;
    // virtual bool isPathFormula() const override;
    // virtual bool isAtomicPropos() const override;
};

/*---------------------
 ---	IntExpression	---
 ----------------------*/
/**
 * IntFormula compose by two sub expression linked through operator.
 * This class represent expression with form expr1 op expr2
 * */

class IntExpression: public IntFormula {
    DECLARE_CTL_CLASS;
private:
    IntFormula *expr1;
    IntFormula *expr2;
    IntFormula::op_type op;
    virtual void createMTMDD() override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor.
     * @param expr1 value of the first expression
     * @param expr2 value of the second expression
     * @param op value of operator
     * */
    IntExpression(IntFormula *expr1, IntFormula *expr2, IntFormula::op_type op =
                  IntFormula::EOP_PLUS);
    virtual ~IntExpression();
public:
    virtual void print(std::ostream &os) const override;
    /**
     * Return the first expression.
     * @return expr1
     * */
    IntFormula *getExpr1() const;
    /**
     * Return the second expression.
     * @return expr2
     * */
    IntFormula *getExpr2() const;
    /**
     * Return the operator value.
     * @return op
     * */
    IntFormula::op_type getOp() const;
};

/*-----------------
 ---	PlaceTerm	---
 ------------------*/
/**
 * PlaceTerm Class. This class representing term with the follow form:
 * coeff op place
 *
 * */
class PlaceTerm : public IntFormula {
    DECLARE_CTL_CLASS;
private:
    int place; // place index, not MDD variable level
    float coeff;
    IntFormula::op_type op;
    virtual void createMTMDD() override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor.
     * @param coeff value of the term coefficient
     * @param variable value of id variable
     * @param op value of operator
     * */
    PlaceTerm(float coeff, int place, IntFormula::op_type op = IntFormula::EOP_TIMES);
    virtual ~PlaceTerm();
public:
    /**
     * Return the coefficient value.
     * @return coefficient value.
     * */
    float getCoeff() const;
    /**
     * Return the variable id value.
     * @return variable id value
     * */
    int getVariable() const;
    int getPlace() const;
    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MDD representation of the term*/
    IntFormula::op_type getOp() const;

    virtual void print(std::ostream &os) const override;
};

/*-------------------------
 ---	IntLiteral	---
 --------------------------*/
/**
 * Representing a constant
 * */
class IntLiteral: public IntFormula {
    DECLARE_CTL_CLASS;
private:
    mutable float _constant;
    virtual void createMTMDD() override;
    virtual bool equals(const BaseFormula* pf) const override;
    virtual size_t compute_hash() const override;
protected:
    /**
     * Constructor
     * @param constant constant value
     * */
    IntLiteral(float constant);
    virtual ~IntLiteral();
public:

    virtual float getConstant() const;

    virtual void print(std::ostream &os) const override;
};

//----------------------------------------------------------------------------
// Upper bound of a set of places

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
