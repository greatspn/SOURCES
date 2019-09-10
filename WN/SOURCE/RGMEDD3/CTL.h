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

#ifdef NDEBUG
# define CTL_ASSERT(x) {}
#endif

#include "rgmedd3.h"
#define DOUBLELEVEL false

extern RSRG *rsrg;
extern bool CTL_quiet;

#ifndef CTL_ASSERT
inline void AssertFailed(const char *f, int l) {
    std::cerr << "Assertion failed at " << f << ":" << l << std::endl;
    throw - 1;
}
# define CTL_ASSERT(x) if(!(x)) { AssertFailed(__FILE__, __LINE__); }
#endif

namespace ctlmdd {
using namespace MEDDLY;

enum TraceType {
    TT_COUNTEREXAMPLE,
    TT_WITNESS
};
inline TraceType oppositeTraceType(TraceType tt) {
    return (tt == TT_COUNTEREXAMPLE) ? TT_WITNESS : TT_COUNTEREXAMPLE;
}
class TreeTraceNode;

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
};

/*---------------------
 --- Base class of all CTL formulas
 -----------------------*/

class BaseFormula {
public:
    virtual void addOwner() = 0;
    virtual void removeOwner() = 0;

    virtual bool isBoolFormula() const = 0;
    virtual bool isIntFormula() const = 0;

    virtual void print(std::ostream &os) const = 0;
};

class IntFormula;
class StateFormula;
class AtomicProposition;

// State formulas & atomic propositions
class LogicalFormula;
class CTLStateFormula;
class BoolLiteral;
class Deadlock;
class Reachability;
class Fireability ;
class Inequality ;

// Integer expressions
class IntExpression;
class PlaceTerm ;
class IntLiteral;
class BoundOfPlaces ;

/*---------------------
 ---	StateFormula	---
 -----------------------*/
/**
 * CTL Formula.
 * */

class StateFormula : public BaseFormula {
private:
    int countOwner;
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
public:
    StateFormula();
    virtual ~StateFormula();
    virtual void print(std::ostream &os) const = 0;

    virtual bool isBoolFormula() const;
    virtual bool isIntFormula() const;

    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MDD representation of the term
     * */
    const dd_edge& getMDD();
    bool hasStoredMDD() const;
    virtual void addOwner();
    virtual void removeOwner();

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
    StateFormula  *formula;
    // Counterexample of an Existential CTL formula
    bool           isECTL_cntexample;
    // Is a circular trace?
    bool           isCircular;
    // Is this a witness or a counterexample?
    TraceType      traceTy;
public:
    TreeTraceNode();
    TreeTraceNode(const vector<int> &mark, StateFormula *f, TraceType tt);
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
 * Bool expression compose by two CTL formulae linked through operator.
 * This class represent bool expression with form formula1 op formula2 for binary operator (AND and OR).
 * This class represent bool expression with form op formula1 for unary operator (only NOT).
 * */
class LogicalFormula: public StateFormula {
public:
    /**
     * Type of logic operator
     * */
    enum op_type {
        CBF_NOT = 0, CBF_AND = 1, CBF_OR = 2
    };
private:
    StateFormula *formula1;
    StateFormula *formula2; // if op = NOT this value is null
    op_type op;
    virtual void createMDD() override;
    virtual void print(std::ostream &os) const override;
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    /**
     * Constructor for bool expression with binary operator.
     * @param formula1 value of the first CTL formula
     * @param formula2 value of the second CTL formula
     * @param op value of operator (AND or OR)
     * */
    LogicalFormula(StateFormula *formula1, StateFormula *formula2, op_type op);
    /**
     * Constructor for bool expression with unary operator. The operator is NOT.
     * @param formula1 value of the first CTL formula
     *
     * */
    LogicalFormula(StateFormula *formula1);
    virtual ~LogicalFormula();

    /**
     * Return the first CTL formula.
     * @return formula1
     * */
    StateFormula *getFormula1() const;
    /**
     * Set the first formula.
     * @param formula1 the CTL formula to set
     * */
    void setFormula1(StateFormula *formula1);
    /**
     * Return the second formula.
     * @return formula2
     * */
    StateFormula *getFormula2() const;
    /**
     * Set the second formula.
     * @param formula2 the CTL formula to set
     * */
    void setFormula2(StateFormula *formula2);
    /**
     * Return the operator value.
     * @return op
     * */
    LogicalFormula::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(LogicalFormula::op_type op);
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
class CTLStateFormula: public StateFormula {
public:
    /**
     * This is the type of CTL operator.
     * */
    enum op_type {
        CTLOP_EX = 0, CTLOP_EU = 1, CTLOP_EG = 2, CTLOP_EF = 3
    };
private:
    StateFormula *formula1;
    StateFormula *formula2;
    op_type op;
    virtual void createMDD() override;
    virtual void print(std::ostream &os) const override;

    void createEXMDD();
    void createEGMDD();
    void createEUMDD();
    void createEFMDD(StateFormula *formula);
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    /**
     * Constructor for CTL formula with binary operator EU.
     * @param formula1 value of the first CTL formula
     * @param formula2 value of the second CTL formula
     * */
    CTLStateFormula(StateFormula *formula1, StateFormula *formula2);
    /**
     * Constructor for CTL formula with unary operator.
     * @param formula1 value of the CTL formula
     * @param op value of operator (EX or EG)
     * */
    CTLStateFormula(StateFormula *formula1, CTLStateFormula::op_type op);
    virtual ~CTLStateFormula();
    /**
     * Return the first CTL formula.
     * @return formula1
     * */
    StateFormula *getFormula1() const;
    /**
     * Set the first formula.
     * @param formula1 the CTL formula to set
     * */
    void setFormula1(StateFormula *formula1);
    /**
     * Return the second formula.
     * @return formula2
     * */
    StateFormula *getFormula2() const;
    /**
     * Set the second formula.
     * @param formula2 the CTL formula to set
     * */
    void setFormula2(StateFormula *formula2);
    /**
     * Return the operator value.
     * @return op
     * */
    CTLStateFormula::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(CTLStateFormula::op_type op);
};

/*-----------------------------
 ---	AtomicProposition	---
 ------------------------------*/
/**
 * Representing atomic proposition.
 * */
class AtomicProposition: public StateFormula {

public:
    AtomicProposition() { }
    virtual ~AtomicProposition() { }
};

/*-------------------------
 ---	BoolLiteral	---
 ---------------------------*/
/**
 * Atomic proposition as boolean value.
 * */

class BoolLiteral: public AtomicProposition {
private:
    bool value;
    virtual void createMDD() override;
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    /**
     * Constructor.
     * @param value a boolean value
     * */
    BoolLiteral(bool value);
    ~BoolLiteral();
    virtual void print(std::ostream &os) const override;
    /**
     * Set a new boolean value.
     * @param value a boolean value
     * */
    void setValue(bool value);
    /**
     * Return the boolean value.
     * @return boolean value
     * */
    bool getValue();
};

/*-------------------------
 ---	Deadlock	---
 ---------------------------*/
/**
 * Atomic proposition for deadlock state o deadlock free state.
 * If value is true it finds deadlock state, otherwise it finds deadlock free state.
 * */

class Deadlock: public AtomicProposition {
private:
    bool value;
    virtual void createMDD() override;
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
    /**
     * Constructor.
     * @param value a boolean value
     * */
    Deadlock(bool value);
    ~Deadlock();
    virtual void print(std::ostream &os) const override;

    /**
     * Set a new boolean value.
     * @param value a boolean value
     * */
    void setValue(bool value);
    /**
     * Return the boolean value.
     * @return boolean value
     * */
    bool getValue();
};

/*-------------------------
 ---    Reachability    ---
 --------------------------*/
/**
 * Tells if a sub-formula is invariantly/possibly true/false
 * in all states of the TRG. Its result is always true or false.
 * */

class Reachability: public AtomicProposition {
public:
    enum prop_type {
        RPT_INVARIANTLY,   // evaluates to true if *subf is verified for all states of the system
        RPT_IMPOSSIBILITY, // evaluates to true if *subf is never verified for all states of the system
        RPT_POSSIBILITY    // evaluates to true if *subf is verified for some states of the system (at least one)
    };
private:
    StateFormula *subf;
    prop_type   type;
    virtual void createMDD() override;
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;

    Reachability(StateFormula *subf, prop_type type);
    ~Reachability();
    virtual void print(std::ostream &os) const override;
};


//----------------------------------------------------------------------------
// Enabling of a set of transitions

class Fireability : public AtomicProposition {
private:
    std::vector<int> transitions;
public:
    Fireability(const std::vector<int>* t) : transitions(*t)  { }

    virtual void createMDD() override;
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
public:
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy) override;
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
     * Set a new constant value.
     * @param constant value to set
     * */
    void setConstant(float constant);
    /**
     * Set a new expression.
     * @param expr value to set
     * */
    // void setExpr(IntFormula *expr);
    /**
     * Return the op value.
     * @return op value.
     * */
    op_type getOp() const;
    /**
     * Set a new op value.
     * @param op value to set
     * */
    void setOp(op_type op);
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
    int countOwner; // number of owners
    /**
     * expression MTMDD (real)
     * */
    dd_edge MTMDD;
    bool computedMTMDD = false;
    /**
     * create expression MTMDD (real)
     * */
    virtual void createMTMDD() = 0;

public:
    IntFormula();
    virtual ~IntFormula();
    virtual void print(std::ostream &os) const = 0;
    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MTMDD representation of the term
     * */
    const dd_edge& getMTMDD();
    void setMTMDD(dd_edge newMTMDD);
    void clearMTMDD();
    bool hasStoredMTMDD() const;
    virtual void addOwner();
    virtual void removeOwner();
    virtual bool isBoolFormula() const;
    virtual bool isIntFormula() const;
};

/*---------------------
 ---	IntExpression	---
 ----------------------*/
/**
 * IntFormula compose by two sub expression linked through operator.
 * This class represent expression with form expr1 op expr2
 * */

class IntExpression: public IntFormula {
private:
    IntFormula *expr1;
    IntFormula *expr2;
    IntFormula::op_type op;
    virtual void createMTMDD() override;
public:
    /**
     * Constructor.
     * @param expr1 value of the first expression
     * @param expr2 value of the second expression
     * @param op value of operator
     * */
    IntExpression(IntFormula *expr1, IntFormula *expr2, IntFormula::op_type op =
                  IntFormula::EOP_PLUS);
    virtual ~IntExpression();
    virtual void print(std::ostream &os) const override;
    /**
     * Return the first expression.
     * @return expr1
     * */
    IntFormula *getExpr1() const;
    /**
     * Set the first expression.
     * @param expr1 the expression to set
     * */
    void setExpr1(IntFormula *expr1);
    /**
     * Return the second expression.
     * @return expr2
     * */
    IntFormula *getExpr2() const;
    /**
     * Set the second expression.
     * @param expr2 the expression to set
     * */
    void setExpr2(IntFormula *expr2);
    /**
     * Return the operator value.
     * @return op
     * */
    IntFormula::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(IntFormula::op_type op);
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
private:
    int place; // place index, not MDD variable level
    float coeff;
    IntFormula::op_type op;
    virtual void createMTMDD() override;
public:
    /**
     * Constructor.
     * @param coeff value of the term coefficient
     * @param variable value of id variable
     * @param op value of operator
     * */
    PlaceTerm(float coeff, int place, IntFormula::op_type op = IntFormula::EOP_TIMES);
    ~PlaceTerm();
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
     * Set a new coefficient value.
     * @param coeff coefficient value*/
    void setCoeff(float coeff);
    /**
     * Set a new variable id.
     * @param variable a variable id*/
    // void setVariable(int variable);
    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MDD representation of the term*/
    IntFormula::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(IntFormula::op_type op);

    virtual void print(std::ostream &os) const override;
};

/*-------------------------
 ---	IntLiteral	---
 --------------------------*/
/**
 * Representing a constant
 * */
class IntLiteral: public IntFormula {
private:
    mutable float _constant;
    virtual void createMTMDD() override;
protected:
    void setConstant(float constant) const;
public:
    /**
     * Constructor
     * @param constant constant value
     * */
    IntLiteral(float constant);
    virtual ~IntLiteral();

    virtual float getConstant() const;

    virtual void print(std::ostream &os) const override;
};

//----------------------------------------------------------------------------
// Upper bound of a set of places

class BoundOfPlaces : public IntLiteral {
private:
    std::vector<int> places;
public:
    BoundOfPlaces(const std::vector<int>* p);

    virtual float getConstant() const override;
    virtual void print(std::ostream &os) const override;
};

//----------------------------------------------------------------------------

inline std::ostream &operator << (std::ostream &os, const BaseFormula &ex) { ex.print(os); return os; }

}

#endif /* CTL_H_ */
