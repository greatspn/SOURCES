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

#include <meddly.h>
#include <meddly_expert.h>

#ifdef NDEBUG
# define CTL_ASSERT(x) {}
#endif

#include "../SHARED/medd.h"
#define DOUBLELEVEL false

extern dddRS::RSRG *rsrg;

#ifndef CTL_ASSERT
inline void AssertFailed(const char *f, int l) {
    std::cerr << "Assertion failed at " << f << ":" << l << std::endl;
    throw - 1;
}
# define CTL_ASSERT(x) if(!(x)) { AssertFailed(__FILE__, __LINE__); }
#endif

namespace ctlmdd {

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
/**
 * This class contains all reference about RS next state function and their forest.
 *
 **/
using namespace MEDDLY;

class CTLMDD {
private:
    static CTLMDD *instance;
    domain *my_domain;
    forest *my_mtmdd_forest;
    forest *my_mdd_forest;
    dd_edge *rs;
    dd_edge *initMark;
    dd_edge *nsf;
    int **ins;
    //MARIETA
    const int *pl2mdd;
    int *mdd2pl;
    //MARIETA
    CTLMDD() {}

public:
    static CTLMDD *getInstance() {
        if (instance == NULL) {
            static CTLMDD tmp;
            instance = &tmp;
        }
        return instance;
    }

    virtual ~CTLMDD();
    //MARIETA
    void CTLinit(dd_edge *rs, dd_edge *initMark, dd_edge *nsf, domain *d, const int *map_vec);
    //void CTLinit(dd_edge *rs,dd_edge *initMark,dd_edge *nsf, domain *d);
    //MARIETA
    domain *getDomain() const;
    forest *getMTMDDForest() const;
    forest *getMDDForest() const;
    inline expert_forest *getMTMDDForestExpert() const {
        return static_cast<expert_forest *>(getMTMDDForest());
    }
    inline expert_forest *getMDDForestExpert() const {
        return static_cast<expert_forest *>(getMDDForest());
    }
    dd_edge *getRS() const;
    dd_edge *getNsf() const;
    dd_edge *getInitMark() const;
    int **getIns() const;
    void getStatistic(void);
    //MARIETA
    int  getVal(int pl) const { return pl2mdd[pl];};

    inline dd_edge *newEmptyMDD() { return new dd_edge(getMDDForest()); }
    inline dd_edge *newRS_MDD()   { return new dd_edge(*getRS()); }
    inline dd_edge *newEmptyMTMDD() { return new dd_edge(getMTMDDForest()); }

    void print_state(const int *marking) const;
    inline bool SatSetContains(const dd_edge &dd, const int *marking) {
        bool isContained;
        getMDDForest()->evaluate(dd, marking, isContained);
        return isContained;
    }

    //MARIETA
    //protected:
    //forest* my_mtmdd_forest;
};

/*---------------------
 ---	CTLFormula	---
 -----------------------*/
/**
 * CTL Formula.
 * */

class CTLFormula {
private:
    int countOwner;
    /**
     * proposition MDD (boolean)
     * */
    dd_edge *pMDD;
    /**
     * create proposition MDD (boolean)
     * */
    virtual void createMDD() = 0;
protected:
    void setMDD(dd_edge *newMDD);
public:
    CTLFormula();
    virtual ~CTLFormula();
    virtual void print(std::ostream &os) const = 0;

    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MDD representation of the term
     * */
    dd_edge *getMDD();
    void addOwner();
    void removeOwner();

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
    CTLFormula    *formula;
    // Counterexample of an Existential CTL formula
    bool           isECTL_cntexample;
    // Is a circular trace?
    bool           isCircular;
    // Is this a witness or a counterexample?
    TraceType      traceTy;
public:
    TreeTraceNode();
    TreeTraceNode(const vector<int> &mark, CTLFormula *f, TraceType tt);
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
 ---	ComplexBoolFormula	---
 ------------------------------*/
/**
 * Bool expression compose by two CTL formulae linked through operator.
 * This class represent bool expression with form formula1 op formula2 for binary operator (AND and OR).
 * This class represent bool expression with form op formula1 for unary operator (only NOT).
 * */
class ComplexBoolFormula: public CTLFormula {
public:
    /**
     * Type of logic operator
     * */
    enum op_type {
        CBF_NOT = 0, CBF_AND = 1, CBF_OR = 2
    };
private:
    CTLFormula *formula1;
    CTLFormula *formula2; // if op = NOT this value is null
    op_type op;
    void createMDD();
    void print(std::ostream &os) const;
public:
    TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy);
    /**
     * Constructor for bool expression with binary operator.
     * @param formula1 value of the first CTL formula
     * @param formula2 value of the second CTL formula
     * @param op value of operator (AND or OR)
     * */
    ComplexBoolFormula(CTLFormula *formula1, CTLFormula *formula2, op_type op);
    /**
     * Constructor for bool expression with unary operator. The operator is NOT.
     * @param formula1 value of the first CTL formula
     *
     * */
    ComplexBoolFormula(CTLFormula *formula1);
    virtual ~ComplexBoolFormula();

    /**
     * Return the first CTL formula.
     * @return formula1
     * */
    CTLFormula *getFormula1() const;
    /**
     * Set the first formula.
     * @param formula1 the CTL formula to set
     * */
    void setFormula1(CTLFormula *formula1);
    /**
     * Return the second formula.
     * @return formula2
     * */
    CTLFormula *getFormula2() const;
    /**
     * Set the second formula.
     * @param formula2 the CTL formula to set
     * */
    void setFormula2(CTLFormula *formula2);
    /**
     * Return the operator value.
     * @return op
     * */
    ComplexBoolFormula::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(ComplexBoolFormula::op_type op);
};
/*-------------------------
 ---	CTLOperation	---
 --------------------------*/
/**
 * Representing operation CTL.
 * Representing operation CTL:
 * EX formula
 * E formula U formula
 * EG formula
 * */
class CTLOperation: public CTLFormula {
public:
    /**
     * This is the type of CTL operator.
     * */
    enum op_type {
        CTLOP_EX = 0, CTLOP_EU = 1, CTLOP_EG = 2, CTLOP_EF = 3
    };
private:
    CTLFormula *formula1;
    CTLFormula *formula2;
    op_type op;
    virtual void createMDD();
    void print(std::ostream &os) const;

    void createEXMDD();
    void createEGMDD();
    void createEUMDD();
    void createEFMDD(CTLFormula *formula);
public:
    TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy);
    /**
     * Constructor for CTL formula with binary operator EU.
     * @param formula1 value of the first CTL formula
     * @param formula2 value of the second CTL formula
     * */
    CTLOperation(CTLFormula *formula1, CTLFormula *formula2);
    /**
     * Constructor for CTL formula with unary operator.
     * @param formula1 value of the CTL formula
     * @param op value of operator (EX or EG)
     * */
    CTLOperation(CTLFormula *formula1, CTLOperation::op_type op);
    virtual ~CTLOperation();
    /**
     * Return the first CTL formula.
     * @return formula1
     * */
    CTLFormula *getFormula1() const;
    /**
     * Set the first formula.
     * @param formula1 the CTL formula to set
     * */
    void setFormula1(CTLFormula *formula1);
    /**
     * Return the second formula.
     * @return formula2
     * */
    CTLFormula *getFormula2() const;
    /**
     * Set the second formula.
     * @param formula2 the CTL formula to set
     * */
    void setFormula2(CTLFormula *formula2);
    /**
     * Return the operator value.
     * @return op
     * */
    CTLOperation::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(CTLOperation::op_type op);
    dd_edge *getMDD();
};

/*------------------------------------
 ---    ConstantScalarCTLFormula  ---
 ------------------------------------*/
/**
 * This class contains a single scalar value, encapsulated as a CTL formula.
 * It has been created to support MCC'2014 ReachabilityComputeBounds property,
 * whose evaluation value is a number and not a boolean result.
 * */

class ConstantExpr; // forward declaration
class Expression; // forward declaration
class ConstantScalarCTLFormula : public CTLFormula {
private:
    ConstantExpr *expr;  // Can only be a ConstantExpr object
    virtual void createMDD();
public:
    ConstantScalarCTLFormula(Expression *ce);
    virtual ~ConstantScalarCTLFormula();
    virtual void print(std::ostream &os) const;
    virtual TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy);
    float getResult();
};

/*-----------------------------
 ---	AtomicProposition	---
 ------------------------------*/
/**
 * Representing atomic proposition.
 * */
class AtomicProposition: public CTLFormula {

public:
    AtomicProposition() { }
    virtual ~AtomicProposition() { }
};

/*-------------------------
 ---	BoolValue	---
 ---------------------------*/
/**
 * Atomic proposition as boolean value.
 * */

class BoolValue: public AtomicProposition {
private:
    bool value;
    void createMDD();
public:
    TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy);
    /**
     * Constructor.
     * @param value a boolean value
     * */
    BoolValue(bool value);
    ~BoolValue();
    void print(std::ostream &os) const;
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
    void createMDD();
public:
    TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy);
    /**
     * Constructor.
     * @param value a boolean value
     * */
    Deadlock(bool value);
    ~Deadlock();
    void print(std::ostream &os) const;

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
    CTLFormula *subf;
    prop_type   type;
    void createMDD();
public:
    TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy);

    Reachability(CTLFormula *subf, prop_type type);
    ~Reachability();
    void print(std::ostream &os) const;
};


/*---------------------
 ---	Expression	---
 ----------------------*/
/**
 * Abstract class. This class representing arithmetic expression with tree structure.
 * */
class Expression {
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
    //static char op_string[4];  // TODO: unused
private:
    int countOwner; // number of owners
    /**
     * expression MTMDD (real)
     * */
    dd_edge *pMTMDD;
    /**
     * create expression MTMDD (real)
     * */
    virtual void createMTMDD() = 0;

public:
    Expression();
    virtual ~Expression();
    virtual void print(std::ostream &os) const = 0;
    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MTMDD representation of the term
     * */
    dd_edge *getMTMDD();
    void setMTMDD(dd_edge *newMTMDD);
    void addOwner();
    void removeOwner();
};

/*---------------------
 ---	Inequality	---
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
    Expression *expr1;
    Expression *expr2;
    float constant;
    void createMDD();
    void createMDDByComplement();
    void print(std::ostream &os) const;
public:
    TreeTraceNode *generateTrace(const vector<int> &state, TraceType traceTy);
    /**
     * Constructor.
     * @param op operator
     * @param expr1 expression
     * @param constant value of constant
     * */
    Inequality(op_type op, Expression *expr1, Expression *expr2);
    /**
     * Constructor.
     * @param op operator
     * @param expr1 expression
     * @param constant value of constant
     * */
    Inequality(op_type op, Expression *expr1, float constant);
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
    // Expression *getExpr() const;
    /**
     * Set a new constant value.
     * @param constant value to set
     * */
    void setConstant(float constant);
    /**
     * Set a new expression.
     * @param expr value to set
     * */
    // void setExpr(Expression *expr);
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
 ---	ComplexExpr	---
 ----------------------*/
/**
 * Expression compose by two sub expression linked through operator.
 * This class represent expression with form expr1 op expr2
 * */

class ComplexExpr: public Expression {
private:
    Expression *expr1;
    Expression *expr2;
    Expression::op_type op;
    void createMTMDD();
public:
    /**
     * Constructor.
     * @param expr1 value of the first expression
     * @param expr2 value of the second expression
     * @param op value of operator
     * */
    ComplexExpr(Expression *expr1, Expression *expr2, Expression::op_type op =
                    Expression::EOP_PLUS);
    virtual ~ComplexExpr();
    void print(std::ostream &os) const;
    /**
     * Return the first expression.
     * @return expr1
     * */
    Expression *getExpr1() const;
    /**
     * Set the first expression.
     * @param expr1 the expression to set
     * */
    void setExpr1(Expression *expr1);
    /**
     * Return the second expression.
     * @return expr2
     * */
    Expression *getExpr2() const;
    /**
     * Set the second expression.
     * @param expr2 the expression to set
     * */
    void setExpr2(Expression *expr2);
    /**
     * Return the operator value.
     * @return op
     * */
    Expression::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(Expression::op_type op);
};

/*-----------------
 ---	Term	---
 ------------------*/
/**
 * Term Class. This class representing term with the follow form:
 * coeff op variable
 *
 * */
class Term : public Expression {
private:
    int variable; //indice della variabile in MDD e non label!!! Maching tra indice e
    // variabile a livello di parser!
    float coeff;
    Expression::op_type op;
    void createMTMDD();
public:
    /**
     * Constructor.
     * @param coeff value of the term coefficient
     * @param variable value of id variable
     * @param op value of operator
     * */
    Term(float coeff, int variable, Expression::op_type op = Expression::EOP_TIMES);
    ~Term();
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
    /**
     * Set a new coefficient value.
     * @param coeff coefficient value*/
    void setCoeff(float coeff);
    /**
     * Set a new variable id.
     * @param variable a variable id*/
    void setVariable(int variable);
    /**
     * Create, if not yet, and return a MDD representation of the term.
     * @return MDD representation of the term*/
    Expression::op_type getOp() const;
    /**
     * Set the operator value.
     *@param op value to set
     * */
    void setOp(Expression::op_type op);

    void print(std::ostream &os) const;
};


/*-------------------------
 ---	ConstantExpr	---
 --------------------------*/
/**
 * Representing a constant
 * */
class ConstantExpr: public Expression {
private:
    float constant;
    void createMTMDD();
public:
    /**
     * Constructor
     * @param constant constant value
     * */
    ConstantExpr(float constant);
    virtual ~ConstantExpr();
    /**
     * Set constant value.
     * @param constant constant value
     * */
    void setConstant(float constant);
    /**
     * Return constant value
     * @return constant value
     * */
    float getConstant();

    void print(std::ostream &os) const;
};

inline std::ostream &operator << (std::ostream &os, const Expression &ex) { ex.print(os); return os; }
inline std::ostream &operator << (std::ostream &os, const CTLFormula &ex) { ex.print(os); return os; }

}

#endif /* CTL_H_ */
