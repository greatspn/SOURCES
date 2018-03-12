/*
 *  Language.h
 *
 *  Extended class definitions for the Petri Net language objects
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__
//=============================================================================


//=============================================================================
//   COMPLETE PETRI NET LANGUAGE DEFINITIONS
//=============================================================================

//=============================================================================
//   Type traits for Expr<> specialization
//=============================================================================

template<typename T>
struct ExprTraits {};

template<>
struct ExprTraits<int> {
    // Prints the constant
    static inline void Print(ostream &os, int value) 				{ os << value; }
    static inline void PrintInSPNicaFormat(ostream &os, int value)  { os << value; }
    static inline void PrintInCosmosFormat(ostream &os, int value)  { os << value; }
    // Cast operator printed with Print()
    static inline const char *CastOp();
};

template<>
struct ExprTraits<double> {
    // Prints the constant
    static inline void Print(ostream &os, double value) 			   { os << value; }
    static inline void PrintInSPNicaFormat(ostream &os, double value)  { os << value; }
    static inline void PrintInCosmosFormat(ostream &os, double value)  { os << value; }
    // Cast operator printed with Print()
    static inline const char *CastOp() { return "(double)"; }
};

template<>
struct ExprTraits<bool> {
    // Prints the constant
    static inline void Print(ostream &os, int value) {
        os << (value ? "True" : "False");
    }
    static inline void PrintInSPNicaFormat(ostream &os, int value) {
        os << (value ? "True" : "False");
    }
    static inline void PrintInCosmosFormat(ostream &os, int value) {
        os << (value ? "TRUE" : "FALSE");
    }

    // Cast operator printed with Print()
    static inline const char *CastOp();
};


#define ThrowProgramExceptionAt(file, line) \
	throw program_exception("Internal application error: file " file ":" #line)

#define InternalError(F,L) ThrowProgramExceptionAt(F,L)


//=============================================================================
//   Type traits for Unary and Binary operators
//=============================================================================

namespace Operator {
inline bool IsPrefix(UnaryFunct uf) 			{ return UnaryFunct_isPrefix[uf]; }
inline bool IsPrefix(UnaryBoolFunct ubf) 		{ return true; }
inline bool IsPrefix(BinaryFunct bf) 			{ return BinaryFunct_isPrefix[bf]; }
inline bool IsPrefix(CompareOp co) 				{ return false; }
inline bool IsPrefix(BinaryBoolFunct bbf) 		{ return false; }

inline const char *GetName(UnaryFunct uf) 			{ return UnaryFunct_Names[uf]; }
inline const char *GetName(UnaryBoolFunct ubf) 		{ return UnaryBoolFunct_Names[ubf]; }
inline const char *GetName(BinaryFunct bf) 			{ return BinaryFunct_Names[bf]; }
inline const char *GetName(CompareOp co) 			{ return CompareOp_Names[co]; }
inline const char *GetName(BinaryBoolFunct bbf) 	{ return BinaryBoolFunct_Names[bbf]; }

inline const char *GetSPNicaName(UnaryFunct uf) 		{ return UnaryFunct_SPNicaNames[uf]; }
inline const char *GetSPNicaName(UnaryBoolFunct ubf) 	{ return UnaryBoolFunct_SPNicaNames[ubf]; }
inline const char *GetSPNicaName(BinaryFunct bf) 		{ return BinaryFunct_SPNicaNames[bf]; }
inline const char *GetSPNicaName(CompareOp co) 			{ return CompareOp_SPNicaNames[co]; }
inline const char *GetSPNicaName(BinaryBoolFunct bbf) 	{ return BinaryBoolFunct_SPNicaNames[bbf]; }

inline const char *GetCosmosName(UnaryFunct uf) 		{ return UnaryFunct_CosmosNames[uf]; }
inline const char *GetCosmosName(UnaryBoolFunct ubf) 	{ return UnaryBoolFunct_CosmosNames[ubf]; }
inline const char *GetCosmosName(BinaryFunct bf) 		{ return BinaryFunct_CosmosNames[bf]; }
inline const char *GetCosmosName(CompareOp co) 			{ return CompareOp_CosmosNames[co]; }
inline const char *GetCosmosName(BinaryBoolFunct bbf) 	{ return BinaryBoolFunct_CosmosNames[bbf]; }
};

// Base of unary operator functors
template<typename UnaryEnum>
struct UnaryOp {
    virtual ~UnaryOp() { }
    virtual UnaryEnum GetOpCode() const = 0;

    inline bool IsPrefix() const    			{ return Operator::IsPrefix(GetOpCode()); }
    inline const char *GetName() const   		{ return Operator::GetName(GetOpCode()); }
    inline const char *GetSPNicaName() const   	{ return Operator::GetSPNicaName(GetOpCode()); }
    inline const char *GetCosmosName() const   	{ return Operator::GetCosmosName(GetOpCode()); }
};

template<typename UnaryEnum, UnaryEnum opcode>
struct UnaryOpFixed : public UnaryOp<UnaryEnum> {
    inline UnaryEnum GetOpCode() const  { return opcode; }
};

template<typename UnaryEnum>
struct UnaryOpCode : public UnaryOp<UnaryEnum> {
    UnaryOpCode(UnaryEnum _opcode) : opcode(_opcode) { }
    inline UnaryEnum GetOpCode() const  { return opcode; }
    UnaryEnum opcode;
};

// Base of binary operator functors
template<typename BinaryEnum>
struct BinaryOp {
    virtual ~BinaryOp() { }
    virtual BinaryEnum GetOpCode() const = 0;

    inline bool IsPrefix() const    			{ return Operator::IsPrefix(GetOpCode()); }
    inline const char *GetName() const   		{ return Operator::GetName(GetOpCode()); }
    inline const char *GetSPNicaName() const   	{ return Operator::GetSPNicaName(GetOpCode()); }
    inline const char *GetCosmosName() const   	{ return Operator::GetCosmosName(GetOpCode()); }
};

template<typename BinaryEnum, BinaryEnum opcode>
struct BinaryOpFixed : public BinaryOp<BinaryEnum> {
    virtual BinaryEnum GetOpCode() const  { return opcode; }
};

template<typename BinaryEnum>
struct BinaryOpCode : public BinaryOp<BinaryEnum> {
    BinaryOpCode(BinaryEnum _opcode) : opcode(_opcode) { }
    virtual BinaryEnum GetOpCode() const  { return opcode; }
    BinaryEnum opcode;
};

//=============================================================================
//   Complex unary functions with memoized operators
//=============================================================================

template<class T>
static inline T
factorial_memoized(int i, map<int, T> &mem) {
    if (i <= 1)
        return 1;
    typename map<int, T>::const_iterator it = mem.find(i);
    if (it != mem.end())
        return it->second;
    T val = factorial_memoized(i - 1, mem) * T(i);
    mem.insert(make_pair(i, val));
    return val;
}

//=============================================================================

template<class T>
static inline T
binomial_memoized(int n, int k, map<pair<int, int>, T> &mem) {
    if (k <= 0 || k >= n)
        return T(0);
    if (k == 1 || k == n - 1)
        return T(n);
    k = min(k, n - k);

    typename map<pair<int, int>, T>::const_iterator it;
    it = mem.find(make_pair(n, k));
    if (it != mem.end())
        return it->second;

    T val = binomial_memoized(n - 1, k, mem) + binomial_memoized(n - 1, k - 1, mem);
    mem.insert(make_pair(make_pair(n, k), val));
    return val;
}

//=============================================================================
//   Unary Operator Functors
//=============================================================================

template<typename T> struct abs_unary_op : public UnaryOpFixed<UnaryFunct, UF_ABS>
{   inline T operator()(const T val) const { return abs(val); } };

template<typename T> struct neg_unary_op : public UnaryOpFixed<UnaryFunct, UF_NEG>
{   inline T operator()(const T val) const { return -val; } };

struct int_unary_op : public UnaryOpCode<UnaryFunct> {
    int_unary_op(UnaryFunct _opcode) : UnaryOpCode<UnaryFunct>(_opcode) { }
    inline int operator()(const int val) const {
        switch (GetOpCode()) {
        case UF_NEG:		return -val;
        case UF_ABS:		return abs(val);
        case UF_FACTORIAL:	return factorial_memoized<int>(val, factorial_mem);
        default:			InternalError(__FILE__, __LINE__);
        }
    }
    mutable map<int, int> factorial_mem;
};

struct double_unary_op : public UnaryOpCode<UnaryFunct> {
    double_unary_op(UnaryFunct _opcode) : UnaryOpCode<UnaryFunct>(_opcode) { }
    inline double operator()(const double val) const {
        switch (opcode) {
        case UF_NEG:		return -val;
        case UF_ABS:		return abs(val);
        case UF_SIN:		return sin(val);
        case UF_COS:		return cos(val);
        case UF_TAN:		return tan(val);
        case UF_ARCSIN:		return asin(val);
        case UF_ARCCOS:		return acos(val);
        case UF_ARCTAN:		return atan(val);
        case UF_EXP:		return exp(val);
        case UF_LOG:		return log(val);
        case UF_SQRT:		return sqrt(val);
        case UF_CEIL:		return ceil(val);
        case UF_FLOOR:		return floor(val);
        case UF_FACTORIAL:	return factorial_memoized<double>(int(val), factorial_mem);
        default:			InternalError(__FILE__, __LINE__);
        }
    }
    mutable map<int, double> factorial_mem;
};


struct log_unary_op : public UnaryOpFixed<UnaryFunct, UF_LOG>
{   inline double operator()(const double val) const { return log(val); } };

template<typename T> struct not_unary_op : public UnaryOpFixed<UnaryBoolFunct, UBF_NOT> {
    inline T operator()(const T val) const { return !val; }
};

//=============================================================================
// Evaluation of unary operators
//=============================================================================

template<typename T, typename OP, typename FN>
struct UnaryEvaluate {
    T operator()(const shared_ptr<Expr<OP> > &ex, const FN &func) {
        return func(ex->Evaluate());
    }
};
// Evaluation of unary operators for Marking-dependent expressions
template<typename T, typename OP, typename FN>
struct UnaryEvaluateInMarking {
    T operator()(const shared_ptr<MdepExpr<OP> > &ex, const FN &func, const SparseMarking &marking) {
        return func(ex->EvaluateInMarking(marking));
    }
};

//=============================================================================
//   Binary Operator Functors
//=============================================================================

template<typename T> struct plus_binary_op : public BinaryOpFixed<BinaryFunct, BF_PLUS>
{	inline T operator()(const T val1, const T val2) const { return val1 + val2; } };
template<typename T> struct minus_binary_op : public BinaryOpFixed<BinaryFunct, BF_MINUS>
{	inline T operator()(const T val1, const T val2) const { return val1 - val2; } };
template<typename T> struct mult_binary_op : public BinaryOpFixed<BinaryFunct, BF_MULT>
{	inline T operator()(const T val1, const T val2) const { return val1 * val2; } };
template<typename T> struct div_binary_op : public BinaryOpFixed<BinaryFunct, BF_DIV>
{	inline T operator()(const T val1, const T val2) const { return val1 / val2; } };

struct log_binary_op : public BinaryOpFixed<BinaryFunct, BF_LOG>
{	inline double operator()(const double val1, const double val2) const { return log(val1) / log(val2); } };

struct int_binary_op : public BinaryOpCode<BinaryFunct> {
    int_binary_op(BinaryFunct _opcode) : BinaryOpCode<BinaryFunct>(_opcode) { }
    inline int operator()(const int val1, const int val2) const {
        switch (opcode) {
        case BF_PLUS:		return val1 + val2;
        case BF_MINUS:		return val1 - val2;
        case BF_MULT:		return val1 * val2;
        case BF_DIV:		return val1 / val2;
        case BF_MIN:		return min(val1, val2);
        case BF_MAX:		return max(val1, val2);
        case BF_MOD:		return val1 % val2;
        case BF_BINOMIAL:	return binomial_memoized<int>(val1, val2, mem_binom);
        case BF_FRACT:      return val1 / val2;
        default:			InternalError(__FILE__, __LINE__);
        }
    }
    mutable map<pair<int, int>, int> mem_binom;
};

struct double_binary_op : public BinaryOpCode<BinaryFunct> {
    double_binary_op(BinaryFunct _opcode) : BinaryOpCode<BinaryFunct>(_opcode) { }
    inline double operator()(const double val1, const double val2) const {
        switch (opcode) {
        case BF_PLUS:		return val1 + val2;
        case BF_MINUS:		return val1 - val2;
        case BF_MULT:		return val1 * val2;
        case BF_DIV:		return val1 / val2;
        case BF_MIN:		return min(val1, val2);
        case BF_MAX:		return max(val1, val2);
        case BF_MOD:		return fmod(val1, val2);
        case BF_POW:		return pow(val1, val2);
        case BF_LOG:		return log(val1) / log(val2);
        case BF_BINOMIAL:	return binomial_memoized<double>(int(val1), int(val2), mem_binom);
        case BF_FRACT:      return val1 / val2;
        default:			InternalError(__FILE__, __LINE__);
        }
    }
    mutable map<pair<int, int>, double> mem_binom;
};

template<typename T>
struct bool_binary_op : public BinaryOpCode<CompareOp> {
    bool_binary_op(CompareOp _opcode) : BinaryOpCode<CompareOp>(_opcode) { }
    inline bool operator()(const T val1, const T val2) const {
        switch (opcode) {
        case CMPOP_EQ:			return val1 == val2;
        case CMPOP_NOT_EQ:		return val1 != val2;
        case CMPOP_LESS:		return val1 <  val2;
        case CMPOP_LESS_EQ:		return val1 <= val2;
        case CMPOP_GREATER:		return val1 >  val2;
        case CMPOP_GREATER_EQ:	return val1 >= val2;
        default:				InternalError(__FILE__, __LINE__);
        }
    }
};

struct and_binary_op : public BinaryOpFixed<BinaryBoolFunct, BBF_AND>
{	private: inline bool operator()(const bool val1, const bool val2) const; /* DON'T IMPLEMENT */ };
struct or_binary_op : public BinaryOpFixed<BinaryBoolFunct, BBF_OR>
{	private: inline bool operator()(const bool val1, const bool val2) const; /* DON'T IMPLEMENT */ };


//=============================================================================
// Evaluation of binary operators
//=============================================================================

template<typename T, typename OP, typename FN>
struct BinaryEvaluate {
    T operator()(const shared_ptr<Expr<OP> > &ex1, const shared_ptr<Expr<OP> > &ex2,
                 const FN &func) {
        return func(ex1->Evaluate(), ex2->Evaluate());
    }
};
// Evaluation of binary operators for Marking-dependent expressions
template<typename T, typename OP, typename FN>
struct BinaryEvaluateInMarking {
    T operator()(const shared_ptr<MdepExpr<OP> > &ex1, const shared_ptr<MdepExpr<OP> > &ex2,
                 const FN &func, const SparseMarking &marking) {
        return func(ex1->EvaluateInMarking(marking), ex2->EvaluateInMarking(marking));
    }
};

// Specialized versions for AND and OR binary operators
template<typename T, typename OP>
struct BinaryEvaluate<T, OP, and_binary_op> {
    T operator()(const shared_ptr<Expr<OP> > &ex1, const shared_ptr<Expr<OP> > &ex2,
                 const and_binary_op &func) {
        return ex1->Evaluate() && ex2->Evaluate();
    }
};
template<typename T, typename OP>
struct BinaryEvaluate<T, OP, or_binary_op> {
    T operator()(const shared_ptr<Expr<OP> > &ex1, const shared_ptr<Expr<OP> > &ex2,
                 const or_binary_op &func) {
        return ex1->Evaluate() || ex2->Evaluate();
    }
};

template<typename T, typename OP>
struct BinaryEvaluateInMarking<T, OP, and_binary_op> {
    T operator()(const shared_ptr<MdepExpr<OP> > &ex1, const shared_ptr<MdepExpr<OP> > &ex2,
                 const and_binary_op &func, const SparseMarking &marking) {
        return ex1->EvaluateInMarking(marking) && ex2->EvaluateInMarking(marking);
    }
};
template<typename T, typename OP>
struct BinaryEvaluateInMarking<T, OP, or_binary_op> {
    T operator()(const shared_ptr<MdepExpr<OP> > &ex1, const shared_ptr<MdepExpr<OP> > &ex2,
                 const or_binary_op &func, const SparseMarking &marking) {
        return ex1->EvaluateInMarking(marking) || ex2->EvaluateInMarking(marking);
    }
};



//=============================================================================
//   A constant value of type T
//=============================================================================

template<class T>
struct ConstValExpr : public Expr<T> {
    typedef T result_t;
    ConstValExpr(T _value) : value(_value) { }
    // Get the actual value of the expression
    virtual result_t Evaluate() const {
        return value;
    }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        ExprTraits<T>::Print(os, value);
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        ExprTraits<T>::PrintInSPNicaFormat(os, value);
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        ExprTraits<T>::PrintInCosmosFormat(os, value);
    }
protected:
    T   value;	// The constant value
};


//=============================================================================
//   A parameter in the context of the PN (i.e. a named constant, a mpar/rpar, etc)
//=============================================================================

template<class T>
struct ParamValExpr : public ParamExpr<T> {
    typedef T result_t;
    ParamValExpr(const char *n, const shared_ptr<Expr<T> > v) : name(n), value(v) { }

    virtual const string &GetName() const				{   return name;   }
    virtual void SetValue(shared_ptr<Expr<T> > newvalue) {   value = newvalue;   }
    virtual const shared_ptr<Expr<T> > GetValue() const {   return value;   }

    // Get the actual value of the expression
    virtual result_t Evaluate() const {
        return value->Evaluate();
    }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        if (expandParamNames)
            value->Print(os, expandParamNames);
        else
            os << name;
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        os << name;
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        os << name;
    }
protected:
    string   			   name;  // The name of the parameters
    shared_ptr<Expr<T> >   value; // The value of the parameter
};

//=============================================================================
//   The special parameter 'x' that appears in general functions f_g(x)
//=============================================================================

struct PdfXExpr : public ParamValExpr<double> {
    PdfXExpr() : ParamValExpr<double>("x", nullptr) { }

    virtual void Print(ostream &os, bool expandParamNames) const override {
        os << name; // never expand the parameter namegenExpr    
    }
    virtual result_t Evaluate() const override { 
        throw program_exception("Cannot evaluate the 'x' value of a PDF function");
    }
    virtual void SetValue(shared_ptr<Expr<double>> newvalue) override {
        throw program_exception("Cannot assign a value to the 'x' of a PDF function");
    }
};

//=============================================================================
//   A cast from another expression type
//=============================================================================

template<class T, class TFrom>
struct CastExpr : public Expr<T> {
    typedef T result_t;
    CastExpr(const shared_ptr<Expr<TFrom> > _ex) : ex(_ex) { }
    // Get the actual value of the expression
    virtual result_t Evaluate() const {
        return T(ex->Evaluate());
    }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        os << ExprTraits<T>::CastOp();
        ex->Print(os, expandParamNames);
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        ex->PrintInSPNicaFormat(os, pn);
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        ex->PrintInCosmosFormat(os, pn);
    }
protected:
    shared_ptr<Expr<TFrom> >   ex;	// The constant value
};





//=============================================================================
//   A cast from another marking-dependent expression type
//=============================================================================

template<class T, class TFrom>
struct CastMdepExpr : public MdepExpr<T> {
    typedef T result_t;
    CastMdepExpr(const shared_ptr<MdepExpr<TFrom> > _ex) : ex(_ex) { }
    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const {
        return T(ex->EvaluateInMarking(marking));
    }
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const {
        return ex->IsMarkingDep();
    }
    // Get the actual value of the expression
    virtual result_t Evaluate() const {
        return T(ex->Evaluate());
    }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        os << ExprTraits<T>::CastOp();
        ex->Print(os, expandParamNames);
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        ex->PrintInSPNicaFormat(os, pn);
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        ex->PrintInCosmosFormat(os, pn);
    }
protected:
    shared_ptr<MdepExpr<TFrom> >   ex;	// The constant value
};


//=============================================================================
//   A place marking (Marking-dependent)
//=============================================================================

template<class T>
struct ExprMdepExpr : public MdepExpr<T> {
    typedef T result_t;
    ExprMdepExpr(const shared_ptr<Expr<T> > &_ex) : ex(_ex) { }
    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const {
        return ex->Evaluate();
    }
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const {
        return false;
    }
    // Get the actual value of the expression
    virtual result_t Evaluate() const {
        return ex->Evaluate();
    }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        ex->Print(os, expandParamNames);
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        ex->PrintInSPNicaFormat(os, pn);
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        ex->PrintInCosmosFormat(os, pn);
    }
protected:
    shared_ptr<Expr<T> >  ex;  // Non marking dependant expression
};


//=============================================================================
//   A place marking (Marking-dependent)
//=============================================================================

template<class T>
struct PlaceMarkingMdepExpr : public MdepExpr<T> {
    typedef T result_t;
    PlaceMarkingMdepExpr(const string &_plName, place_t _plInd)
    /**/ : plName(_plName), plInd(_plInd) { }
    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const {
        return marking[ plInd ];
    }
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const { return true; }
    // Get the actual value of the expression
    virtual result_t Evaluate() const { InternalError(__FILE__, __LINE__); }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        if (expandParamNames)
            os << "#" << (plInd + place_t(1));
        else
            os << "#" << plName;

    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        os << "rr[" << plName << "]";
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        os << " " << plName << " ";
    }
protected:
    string		plName;  // The name of the place, as written in the expr.
    place_t		plInd;	 // Index of the place in the PN
};


//=============================================================================
//   Unary Expression
//=============================================================================

template<class T, class OP, class FN>
struct UnaryMdepExpr : public MdepExpr<T> {
    typedef T result_t;
    typedef OP operand_t;
    typedef FN unary_funct_t;

    UnaryMdepExpr(const shared_ptr<MdepExpr<OP> > _ex, const FN &_fn = FN())
    /**/ : ex(_ex), func(_fn) { }

    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const {
        return UnaryEvaluateInMarking<T, OP, FN>()(ex, func, marking);
    }
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const { return ex->IsMarkingDep(); }
    // Get the actual value of the expression
    virtual result_t Evaluate() const { return UnaryEvaluate<T, OP, FN>()(ex, func); }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        if (func.IsPrefix()) {
            os << "(" << func.GetName();
            ex->Print(os, expandParamNames);
            os << ")";
        }
        else {
            os << func.GetName() << "[";
            ex->Print(os, expandParamNames);
            os << "]";
        }
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        if (func.IsPrefix()) {
            os << "(" << func.GetSPNicaName();
            ex->PrintInSPNicaFormat(os, pn);
            os << ")";
        }
        else {
            os << func.GetSPNicaName() << "[";
            ex->PrintInSPNicaFormat(os, pn);
            os << "]";
        }
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        if (func.IsPrefix()) {
            os << "(" << func.GetCosmosName();
            ex->PrintInCosmosFormat(os, pn);
            os << ")";
        }
        else {
            os << func.GetCosmosName() << "(";
            ex->PrintInCosmosFormat(os, pn);
            os << ")";
        }
    }
protected:
    shared_ptr<MdepExpr<OP> >  ex;    // Argument
    unary_funct_t  			   func;  // Unary function
};


//=============================================================================
//   Binary Expression
//=============================================================================

template<class T, class OP, class FN>
struct BinaryMdepExpr : public MdepExpr<T> {
    typedef T result_t;
    typedef OP operand_t;
    typedef FN binary_funct_t;

    BinaryMdepExpr(const shared_ptr<MdepExpr<OP> > _ex1,
                   const shared_ptr<MdepExpr<OP> > _ex2,
                   const FN &_fn = FN())
    /**/ : ex1(_ex1), ex2(_ex2), func(_fn) { }

    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const {
        return BinaryEvaluateInMarking<T, OP, FN>()(ex1, ex2, func, marking);
    }
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const { return ex1->IsMarkingDep() || ex2->IsMarkingDep(); }
    // Get the actual value of the expression
    virtual result_t Evaluate() const { return BinaryEvaluate<T, OP, FN>()(ex1, ex2, func); }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        if (func.IsPrefix()) {
            os << func.GetName() << "[";
            ex1->Print(os, expandParamNames);
            os << ",";
            ex2->Print(os, expandParamNames);
            os << "]";
        }
        else {
            os << "(";
            ex1->Print(os, expandParamNames);
            os << " " << func.GetName() << " ";
            ex2->Print(os, expandParamNames);
            os << ")";
        }
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        if (func.IsPrefix()) {
            os << func.GetSPNicaName() << "[";
            ex1->PrintInSPNicaFormat(os, pn);
            os << ",";
            ex2->PrintInSPNicaFormat(os, pn);
            os << "]";
        }
        else {
            os << "(";
            ex1->PrintInSPNicaFormat(os, pn);
            os << " " << func.GetSPNicaName() << " ";
            ex2->PrintInSPNicaFormat(os, pn);
            os << ")";
        }
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        if (func.IsPrefix()) {
            os << func.GetCosmosName() << "(";
            ex1->PrintInCosmosFormat(os, pn);
            os << ",";
            ex2->PrintInCosmosFormat(os, pn);
            os << ")";
        }
        else {
            os << "(";
            ex1->PrintInCosmosFormat(os, pn);
            os << " " << func.GetCosmosName() << " ";
            ex2->PrintInCosmosFormat(os, pn);
            os << ")";
        }
    }
protected:
    shared_ptr<MdepExpr<OP> >  ex1, ex2;  // Arguments
    binary_funct_t  		   func;  	  // Binary function
};

//=============================================================================
//   If-Then-Else conditional expression
//=============================================================================

template<class T>
struct IfThenElseMdepExpr : public MdepExpr<T> {
    typedef T result_t;

    IfThenElseMdepExpr(const shared_ptr<MdepExpr<bool> > _cond,
                       const shared_ptr<MdepExpr<T> > _exT,
                       const shared_ptr<MdepExpr<T> > _exF)
    /**/ : cond(_cond), exT(_exT), exF(_exF) { }
    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const {
        if (cond->EvaluateInMarking(marking))
            return exT->EvaluateInMarking(marking);
        return exF->EvaluateInMarking(marking);
    }
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const {
        return cond->IsMarkingDep() || exT->IsMarkingDep() || exF->IsMarkingDep();
    }
    // Get the actual value of the expression
    virtual result_t Evaluate() const {
        return cond->Evaluate() ? exT->Evaluate() : exF->Evaluate();
    }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        os << "If[";	cond->Print(os, expandParamNames);
        os << ", ";		exT->Print(os, expandParamNames);
        os << ", ";		exF->Print(os, expandParamNames);
        os << "]";
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        os << "If[";	cond->PrintInSPNicaFormat(os, pn);
        os << ", ";		exT->PrintInSPNicaFormat(os, pn);
        os << ", ";		exF->PrintInSPNicaFormat(os, pn);
        os << "]";
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        os << "If(";	cond->PrintInCosmosFormat(os, pn);
        os << ", ";		exT->PrintInCosmosFormat(os, pn);
        os << ", ";		exF->PrintInCosmosFormat(os, pn);
        os << ")";
    }
protected:
    shared_ptr<MdepExpr<bool> >	 cond;  	// Condition
    shared_ptr<MdepExpr<T> >  	 exT, exF;	// Argument
};

//=============================================================================
//   Indirect acces to a MdepExpr* object pointer
//=============================================================================

template<class T>
struct PtrToMdepExpr : public MdepExpr<T> {
    typedef T result_t;
    PtrToMdepExpr(const char *s,
                  const shared_ptr<MdepExpr<T> > _ex = shared_ptr<MdepExpr<T> >())
        : name(s), ex(_ex) { }

    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const {
        assert(ex);
        return ex->EvaluateInMarking(marking);
    }
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const {
        assert(ex);
        return ex->IsMarkingDep();
    }
    // Get the actual value of the expression
    virtual result_t Evaluate() const {
        assert(ex);
        return ex->Evaluate();
    }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const {
        if (!ex)
            os << "<<" << name << ">>";
        else
            ex->Print(os, expandParamNames);
    }
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const {
        assert(ex);
        ex->PrintInSPNicaFormat(os, pn);
    }
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const {
        assert(ex);
        ex->PrintInCosmosFormat(os, pn);
    }
    inline const string &get_name() const { return name; }
    inline void bind(const shared_ptr<MdepExpr<T> > &_ex) { ex = _ex; }
protected:
    string 					 name;  // Name of this pointer object
    shared_ptr<MdepExpr<T> >  ex;    // Pointed expression (can be unbounded)
};

//=============================================================================














//=============================================================================
//      MultipleServerDelayFn transition delay
//=============================================================================

// Single/multiple/infinite server delay policy
struct MultipleServerDelayFn : public MarkDepDelayFn {
    MultipleServerDelayFn(size_t _numSrv, const shared_ptr<Expr<double> > &_delay)
    /**/ : delay(_delay), numSrv(_numSrv) { }

    virtual bool IsMarkingDep() const;
    virtual double EvaluateConstantDelay() const;
    virtual double EvaluateDelay(const PN &pn, const SparseMarking &mark,
                                 const Transition &trn) const;
    virtual bool IsPDF() const;
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInMarking(ostream &os, const PN &pn,
                                const SparseMarking &mark,
                                const Transition &trn) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;

    virtual void PrintInCosmosFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;

protected:
    shared_ptr<Expr<double> >  delay;	 /// Base delay value
    size_t				       numSrv;	 /// Number of available servers
};

//=============================================================================
//    LoadDependentDelayFn transition delay
//=============================================================================

// Load-dependent policy with user-supplied values
struct LoadDependentDelayFn : public MarkDepDelayFn {
    LoadDependentDelayFn(const vector<double> &_delays) : delays(_delays) { }

    virtual bool IsMarkingDep() const;
    virtual double EvaluateConstantDelay() const;
    virtual double EvaluateDelay(const PN &pn, const SparseMarking &mark,
                                 const Transition &trn) const;
    virtual bool IsPDF() const;
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInMarking(ostream &os, const PN &pn,
                                const SparseMarking &mark,
                                const Transition &trn) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;
protected:
    vector<double>	delays;  /// Array of delay values
};

//=============================================================================
//    MarkingDependentDelayFn transition delay
//=============================================================================

// Load-dependent policy with user-supplied values
struct MarkingDependentDelayFn : public MarkDepDelayFn {
    MarkingDependentDelayFn(const shared_ptr<MdepExpr<double> > &_mdepFn)
    /**/ : mdepFn(_mdepFn) { }

    virtual bool IsMarkingDep() const;
    virtual double EvaluateConstantDelay() const;
    virtual double EvaluateDelay(const PN &pn, const SparseMarking &mark,
                                 const Transition &trn) const;
    virtual bool IsPDF() const;
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInMarking(ostream &os, const PN &pn,
                                const SparseMarking &mark,
                                const Transition &trn) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;
protected:
    shared_ptr<MdepExpr<double> > mdepFn;  /// Marking-dependent function
};

//=============================================================================
//    The pdf of a general transition
//=============================================================================

// The pdf function f_g(x) of a general event
struct GeneralPdfFn : public MarkDepDelayFn {
    GeneralPdfFn(const shared_ptr<Expr<double> > &_genFn, const char* _expr)
    /**/ : genFn(_genFn), genExpr(_expr) { }

    virtual bool IsMarkingDep() const;
    virtual double EvaluateConstantDelay() const;
    virtual double EvaluateDelay(const PN &pn, const SparseMarking &mark,
                                 const Transition &trn) const;
    virtual bool IsPDF() const;
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInMarking(ostream &os, const PN &pn,
                                const SparseMarking &mark,
                                const Transition &trn) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const;
protected:
    shared_ptr<Expr<double> > genFn;  /// The pdf function
    string genExpr; /// The text of the expression
};

//=============================================================================










//=============================================================================
// Measure expression: real constant value
//=============================================================================

struct MeasureExpr_Real : public MeasureExpr {
    MeasureExpr_Real(const shared_ptr<Expr<double> > &rc) : value(rc) { }

    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG &rg);
    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG2 &rg);
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const;
protected:
    shared_ptr<Expr<double> > value;
};

//=============================================================================
// Measure expression: P{cond}  => reward is the probability that *cond is true
//=============================================================================

struct MeasureExpr_P : public MeasureExpr {
    MeasureExpr_P(const shared_ptr<MdepExpr<bool> > &p) : cond(p) { }

    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG &rg);
    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG2 &rg);
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const;
protected:
    shared_ptr<MdepExpr<bool> > cond;
};

//=============================================================================
// Measure expression: E{place/cond}  => reward is a token count with cond
//=============================================================================

struct MeasureExpr_E : public MeasureExpr {
    MeasureExpr_E(const shared_ptr<MdepExpr<int> > &v,
                  /**/		  const shared_ptr<MdepExpr<bool> > &p)
    /**/ : place(v), cond(p) {  }

    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG &rg);
    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG2 &rg);
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const;
protected:
    shared_ptr<MdepExpr<int> >  place;
    shared_ptr<MdepExpr<bool> > cond;
};

//=============================================================================
// Throughput expression: X{trn/cond}  => Impulse reward of transition
//=============================================================================

struct MeasureExpr_X : public MeasureExpr {
    MeasureExpr_X(const char *trn, const shared_ptr<MdepExpr<bool> > &p)
    /**/ : trnName(trn), cond(p) {  }

    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG &rg);
    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG2 &rg);
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const;
protected:
    string                trnName;
    shared_ptr<MdepExpr<bool> > cond;
};

//=============================================================================
// Measure binary operator: measure +/- measure
//=============================================================================

struct MeasureExpr_Binary : public MeasureExpr {
    MeasureExpr_Binary(const shared_ptr<MeasureExpr> &e1, int o,
                       /**/			   const shared_ptr<MeasureExpr> &e2)
    /**/ : expr1(e1), expr2(e2), op(o) { }

    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG &rg);
    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG2 &rg);
    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const;
protected:
    shared_ptr<MeasureExpr> expr1, expr2;
    int op;
};

//=============================================================================




//=============================================================================
// Preprocessed measure: Real value
//=============================================================================

struct PpMeasureExpr_Real : public PpMeasureExpr {
    PpMeasureExpr_Real(double _value) : value(_value) { }

    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual double EvaluateMeasure(const PetriNetSolution &pns);
    virtual void SetupRewardVectors(size_t numStates, RewardVectors &rv);
protected:
    double		value;
};

//=============================================================================
// Preprocessed measure: reward vectors
//=============================================================================

struct PpMeasureExpr_Reward : public PpMeasureExpr {
    PpMeasureExpr_Reward(ublas::compressed_vector<double> &_rate,
                         ublas::compressed_vector<double> &_impulse)
    {  rateReward.swap(_rate);  impulseReward.swap(_impulse);  }

    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual double EvaluateMeasure(const PetriNetSolution &pns);
    virtual void SetupRewardVectors(size_t numStates, RewardVectors &rv);
protected:
    ublas::compressed_vector<double>   rateReward;
    ublas::compressed_vector<double>   impulseReward;
};

//=============================================================================
// Preprocessed measure: Binary operator
//=============================================================================

struct PpMeasureExpr_Binary : public PpMeasureExpr {
    PpMeasureExpr_Binary(const shared_ptr<PpMeasureExpr> &e1, int o,
                         /**/				 const shared_ptr<PpMeasureExpr> &e2)
        : expr1(e1), expr2(e2), op(o) { }

    virtual void Print(ostream &os, bool expandParamNames) const;
    virtual double EvaluateMeasure(const PetriNetSolution &pns);
    virtual void SetupRewardVectors(size_t numStates, RewardVectors &rv);
protected:
    shared_ptr<PpMeasureExpr>  expr1, expr2;
    int op;
};

//=============================================================================





//=============================================================================
#endif   // __LANGUAGE_H__
