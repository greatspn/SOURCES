
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
using namespace std;

#include <boost/optional.hpp>

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
// #include "PetriNet2.h"
// #include "Measure.h"
// #include "ReachabilityGraph.h"
// #include "PackedMemoryPool.h"
// #include "compact_table.h"
// #include "NewReachabilityGraph.h"
// #include "PetriExport.h"
// #include "SolveCTMC.h"
// #include "MRP.h"
// #include "MRP_Implicit.h"
// #include "MRP_Explicit.h"
// #include "MRP_SccDecomp.h"
// #include "DTA.h"
// #include "SynchProduct.h"
// #include "SynchProductSCC.h"
// #include "CSLTA.h"














// #include "compact_table.h"

// #include "Language2.h"

// #define PetriNetObject  NewPetriNetObject
// #define UnaryEvaluate   NewUnaryEvaluate
// #define BinaryEvaluate  NewBinaryEvaluate
// #define Expr   			NewExpr
// #define ParamExpr       NewParamExpr
// #define ConstValExpr 	NewConstValExpr
// #define ParamValExpr    NewParamValExpr
// #define CastExpr 		NewCastExpr
// #define PlaceMarkingExpr  NewPlaceMarkingExpr
// #define UnaryExpr		NewUnaryExpr
// #define BinaryExpr		NewBinaryExpr
// #define IfThenElseExpr	NewIfThenElseExpr
// #define PtrToExpr		NewPtrToExpr
// #define UF_MSET_CARDINALITY  UF_LOG
// #define UnaryFunct      NewUnaryFunct
// #define BinaryFunct     NewBinaryFunct

// #define UF_NEG  NewUF_NEG
// #define UF_ABS  NewUF_ABS
// #define UF_SIN  NewUF_SIN
// #define UF_COS  NewUF_COS
// #define UF_TAN  NewUF_TAN
// #define UF_ARCSIN  NewUF_ARCSIN
// #define UF_ARCCOS  NewUF_ARCCOS
// #define UF_ARCTAN  NewUF_ARCTAN
// #define UF_EXP  NewUF_EXP
// #define UF_LOG  NewUF_LOG
// #define UF_SQRT  NewUF_SQRT
// #define UF_CEIL  NewUF_CEIL
// #define UF_FLOOR  NewUF_FLOOR
// #define UF_FACTORIAL  NewUF_FACTORIAL
// #define UF_NOT  NewUF_NOT
// #define UF_NumFuncts  NewUF_NumFuncts
// #define BF_PLUS  NewBF_PLUS
// #define BF_MINUS  NewBF_MINUS
// #define BF_MULT  NewBF_MULT
// #define BF_DIV  NewBF_DIV
// #define BF_MIN  NewBF_MIN
// #define BF_MAX  NewBF_MAX
// #define BF_MOD  NewBF_MOD
// #define BF_POW  NewBF_POW
// #define BF_LOG  NewBF_LOG
// #define BF_BINOMIAL  NewBF_BINOMIAL
// #define BF_AND  NewBF_AND
// #define BF_OR  NewBF_OR
// #define BF_NumFuncts  NewBF_NumFuncts

// #define UnaryFunct_Names NewUnaryFunct_Names
// #define UnaryFunct_isPrefix NewUnaryFunct_isPrefix
// #define BinaryFunct_Names NewBinaryFunct_Names
// #define BinaryFunct_isPrefix NewBinaryFunct_isPrefix
// #define ExprTraits NewExprTraits


// template<class T> struct Expr;


// struct ColorClassTag       { typedef size_t value_type; };
// struct StaticSubclassTag   { typedef size_t value_type; };
// struct DomainCompTag       { typedef size_t value_type; };
// struct ColorTupleTag       { typedef size_t value_type; };
// struct ColorTag            { typedef size_t value_type; };
// struct ColorVarTag         { typedef size_t value_type; };
// struct MultiSetElemTag     { typedef size_t value_type; };
// struct PrintExprFlagsTag   { typedef size_t value_type; };

// typedef opaque_int<ColorClassTag>		colorclass_t;
// typedef opaque_int<StaticSubclassTag>	static_subclass_t;
// typedef opaque_int<DomainCompTag>	    domain_comp_t;
// typedef opaque_int<ColorTupleTag>	    color_tuple_t; // packed color tuple
// typedef opaque_int<ColorTag>		    color_t;
// typedef opaque_int<ColorVarTag>		    colorvar_t;
// typedef opaque_int<MultiSetElemTag>		mset_elem_t;
// typedef opaque_int<PrintExprFlagsTag>	printflags_t;

// inline color_tuple_t operator + (const color_tuple_t c1, const color_t c2)
// { return color_tuple_t(get_value(c1) + get_value(c2)); }
// inline color_tuple_t operator * (const color_tuple_t c1, const color_t c2)
// { return color_tuple_t(get_value(c1) * get_value(c2)); }
// inline color_tuple_t operator - (const color_tuple_t c1, const color_t c2)
// { return color_tuple_t(get_value(c1) - get_value(c2)); }
// inline color_tuple_t operator / (const color_tuple_t c1, const color_t c2)
// { return color_tuple_t(get_value(c1) / get_value(c2)); }
// inline color_t operator % (const color_tuple_t c1, const color_t c2)
// { return color_t(get_value(c1) % get_value(c2)); }

// //=============================================================================

// // Runtime exception raised for semantic parse errors
// class semantic_parse_exception : public std::exception {
// 	string expl;	// explanation
// public:
// 	inline semantic_parse_exception(const char* e) : expl(e) { }
// 	inline semantic_parse_exception(const string& e) : expl(e) { }
// 	virtual ~semantic_parse_exception() throw()   { }
// 	virtual const char *what() const throw()   { return expl.c_str(); }
// };

// //=============================================================================

// // Runtime exception raised for evaluation errors
// class evaluation_exception : public std::exception {
// 	string expl;	// explanation
// public:
// 	inline evaluation_exception(const char* e) : expl(e) { }
// 	inline evaluation_exception(const string& e) : expl(e) { }
// 	virtual ~evaluation_exception() throw()   { }
// 	virtual const char *what() const throw()   { return expl.c_str(); }
// };

// //=============================================================================
// // A static subclass in a simple color class
// //=============================================================================

// struct StaticSubclass {
// 	StaticSubclass() { }
// 	StaticSubclass(const StaticSubclass&) = delete;
// 	StaticSubclass(StaticSubclass&&) = default;

// 	std::string      name;     // Name of this static subclass (empty = anonymous)
// 	// Def. mode 1: explicit list of color names
// 	ivector2<std::string, color_t>   colors;
// 	// Def. mode 2: prefix { start .. end }
// 	std::string            prefix;
// 	shared_ptr<Expr<int>>  start;
// 	shared_ptr<Expr<int>>  end;

// 	inline bool isNamed() const { return !name.empty(); }
// 	inline bool isList() const { return !colors.empty(); }
// 	inline bool isInterval() const { return !isList(); }

// 	color_t numColors() const;
// 	const std::string& getColorName(color_t clr, std::string& buf) const;
// 	ostream& printColorName(color_t clr, ostream& os) const;
// };

// //=============================================================================
// //Color class of a Petri net, which could be either simple or cross-domain
// //=============================================================================

// struct ColorClass {
// 	ColorClass() { }
// 	ColorClass(const ColorClass&) = delete;
// 	ColorClass(ColorClass&&) = default;

// 	colorclass_t     index;    // Index of this color class in the color class table
// 	std::string      name;     // Color name
// 	// Def. 1: a simple color class with a list of static subclasses
// 	ivector2<StaticSubclass, static_subclass_t>   subclasses;
// 	// Def. 2: a product of simple color classes
// 	ivector2<colorclass_t, domain_comp_t>  cross;  // Color class indexes of this product domain

// 	inline bool isSimple() const { return !subclasses.empty(); }
// 	inline bool isCrossDomain() const { return !isSimple(); }
// 	inline domain_comp_t numClassesInDomain() const
// 	{ return isSimple() ? domain_comp_t(1) : cross.size(); }
// 	inline colorclass_t getIthColorClass(domain_comp_t ith) const
// 	{ return isSimple() ? index : cross[ith]; }

// 	color_t numColors() const;
// 	const std::string& getColorName(color_t clr, std::string& buf) const;
// 	ostream& printColorName(color_t clr, ostream& os) const;
// 	// For static subclasses (simple color classes only)
// 	// Starting index of a color subclass
// 	color_t startOfSubclass(static_subclass_t subCl) const;
// };

// const colorclass_t NEUTRAL_COLOR = colorclass_t(-100);

// //=============================================================================
// // Interface for translating color class indexes in color class objects
// //=============================================================================

// struct ColorClassContainer {
// 	// Get a color class object given a color index
// 	virtual const ColorClass& colorClassOf(colorclass_t ccInd) const = 0;
// 	// Get the color domain of a place
// 	virtual const colorclass_t colorDomainOf(place_t plInd) const = 0;
// };

//=============================================================================

// struct NewPN : PN, ColorClassContainer {
// 	ivector2<ColorClass, colorclass_t>		clrClasses;	  // Color Classes

// 	virtual const ColorClass& colorClassOf(colorclass_t ccInd) const override {
// 		return clrClasses[ccInd];
// 	}
// 	virtual const colorclass_t colorDomainOf(place_t place) const override {
// 		//TODO: change this.
// 		return NEUTRAL_COLOR;
// 	}
// };
// #define PN              NewPN


// //=============================================================================
// // A color variable
// //=============================================================================

// struct ColorVar {
// 	colorvar_t    index;   // Index of the color variable in the Petri net
// 	std::string   name;    // Variable name
// 	colorclass_t  domain;  // Domain index of this color variable
// };

// //=============================================================================
// // A tuple of colors in a color domain
// //=============================================================================

// struct DomainElement {
// 	colorclass_t                      domain;    // Domain of this tuple
// 	ivector2<color_t, domain_comp_t>  elems;     // Tuple of color indexes

// 	inline bool operator==(const DomainElement& de) const
// 	{ return domain==de.domain && elems==de.elems; }
// };

// // Encode a domain element tuple into a single integer value
// color_tuple_t encodeTuple(const DomainElement& elem, const ColorClassContainer& ccc);
// // Decode a compressed tuple into a DomainElement
// void decodeTuple(color_tuple_t tuple, const colorclass_t domain,
// 	             const ColorClassContainer& ccc, DomainElement& out);

// std::function<ostream& (ostream&)>
// print_domain_elem(const DomainElement& de, const ColorClassContainer& ccc);

// //=============================================================================
// // A multiset of colors in a color domain
// //=============================================================================

// template<class T>
// struct MultiSet {
// 	colorclass_t                         domain; // Domain of this multiset
// 	ivector2<T, mset_elem_t>             mult;   // Multiplicities
// 	ivector2<color_tuple_t, mset_elem_t> elems;  // Domain elements, as (encoded) color tuples

// 	inline bool empty() const { return mult.empty(); }
// 	inline mset_elem_t numElements() const { return mult.size(); }

// 	inline bool operator == (const MultiSet& ms) const {
// 		return (domain == ms.domain) && (mult == ms.mult) && (elems == ms.elems);
// 	}

// 	// Append a new color tuple to the multiset, if its multiplicity is not zero.
// 	inline void push_back(const color_tuple_t new_elem, const T new_mult) {
// 		if (new_mult == T(0))
// 			return;
// 		mult.push_back(new_mult);
// 		elems.push_back(new_elem);
// 	}
// };

// template<class T>
// std::function<ostream& (ostream&)>
// print_multiset(const MultiSet<T>& mset, const ColorClassContainer& ccc);

//=============================================================================
// Context of expressions evaluation & print
//=============================================================================

// struct FiringBinding {
// 	color_t bindingOf(colorvar_t cvar) const;
// };

//-----------------------------------------------------------------------------
// struct PetriNetContainer : ColorClassContainer {
// 	PetriNetContainer(const PN* _pn) : pn(_pn) { }
// 	PetriNetContainer(const PetriNetContainer&) = delete;
// 	PetriNetContainer(PetriNetContainer&&) = default;

// 	virtual const ColorClass& colorClassOf(colorclass_t ccInd) const override
// 	{ return pn->colorClassOf(ccInd); }
// 	virtual const colorclass_t colorDomainOf(place_t plInd) const override
// 	{ return pn->colorDomainOf(plInd); }
// private:
// 	const PN* pn;
// };

// //-----------------------------------------------------------------------------
// struct EvaluationContext {
// 	// EvaluationContext(const PN* _pn, const SparseMarking *_marking,
// 	// 				  const FiringBinding *_binding)
// 	// : PetriNetContainer(_pn), marking(_marking), binding(_binding) { }
// 	// EvaluationContext(const EvaluationContext&) = delete;
// 	// EvaluationContext(EvaluationContext&&) = default;

// 	// const PN*  pn;
// 	const SparseMarking *marking;
// 	const FiringBinding *binding;

// 	// Transient color domain of multiset expressions
// 	// Some multiset expressions are evaluated knowing in advance the domain
// 	// of the result. This allows the decoding of the <All> symbol.
// 	mutable colorclass_t colorDomainOfExpr;
// };

// //-----------------------------------------------------------------------------
// struct PrintExprContext {
// 	// PrintExprContext(const PN* _pn, const SparseMarking *_marking)
// 	// : PetriNetContainer(_pn), marking(_marking) { }
// 	// PrintExprContext(const PrintExprContext&) = delete;
// 	// PrintExprContext(PrintExprContext&&) = default;

// 	// Current marking
// 	const SparseMarking *marking;
// 	// Should expand names of all variables/parameters?
// 	bool  expandParamNames;
// };

//=============================================================================
//   Base Class of Parsed Language objects
//=============================================================================

// struct PetriNetObject : public ParserObj {
// 	inline PetriNetObject() { }
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const = 0;
// };

// std::function<ostream& (ostream&)>
// print_petrinetobj(const PetriNetObject& pnobj, const SparseMarking *marking = nullptr,
// 				  bool expandParamNames = false);

// std::function<ostream& (ostream&)>
// print_petrinetobj(const shared_ptr<PetriNetObject> pnobj, const SparseMarking *marking = nullptr,
// 				  bool expandParamNames = false);


// //=============================================================================
// //   Expression with constants and symbolic parameters
// //=============================================================================

// template<class T>
// struct Expr : public PetriNetObject {
// 	typedef T result_t;
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const = 0;
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override = 0;
// 	// Tests if this expression is marking dependent
// 	virtual bool IsMarkingDep() const = 0;
// 	// Test if this expression is firing dependent, i.e. needs the
// 	// fired transition id or its colorvar binding.
// 	virtual bool IsFiringDep() const = 0;
// 	// Evaluate a constant expression
// 	inline result_t EvaluateConstant() const {
// 		return Evaluate(nullptr);
// 	}
// 	// The color domain of this expression object (neutral for uncolored)
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const = 0;

// 	// Converts this object in the SPNica/Cosmos formats
// 	// virtual void PrintInSPNicaFormat(ostream& os, const PN& pn) const = 0;
// 	// virtual void PrintInCosmosFormat(ostream& os, const PN& pn) const = 0;
// };

// //=============================================================================
// //   A Parameter with a name
// //=============================================================================

// template<class T>
// struct ParamExpr : public Expr<T> {
// 	typedef T result_t;
// 	virtual const string& GetName() const = 0;
// 	virtual void SetValue(shared_ptr<Expr<T> > newvalue) = 0;
// 	virtual const shared_ptr<Expr<T> > GetValue() const = 0;
// };

// //=============================================================================
// // Base class of multiset elements
// //=============================================================================

// using colorset_t = std::set<color_t>;

// struct MsetTerm : public Expr<color_t> {
// 	// Some multiset terms are single colors, other are sets.
// 	virtual bool IsSet() const = 0;
// 	// Special evaluation function for multiset terms that are color sets
// 	virtual colorset_t EvaluateSet(colorclass_t termClass, const EvaluationContext* ctx) const = 0;
// 	// Return the color class of this term object
// 	virtual colorclass_t GetColorClass() const = 0;
// };

// // const color_t ALL_COLORS = color_t(-150);
// // Special color class code for the ALL term.
// const colorclass_t ALL_COLOR_CLASS = colorclass_t(-160);

// //=============================================================================














//=============================================================================

// enum UnaryFunct {
// 	// algebraic
// 	UF_NEG,	UF_ABS,	UF_SIN, UF_COS, UF_TAN,
// 	UF_ARCSIN, UF_ARCCOS, UF_ARCTAN, UF_EXP,
// 	UF_LOG, UF_SQRT, UF_CEIL, UF_FLOOR, UF_FACTORIAL,
// 	// boolean
// 	UF_NOT,
// 	// total count
// 	UF_NumFuncts
// };
// extern const char* UnaryFunct_Names[];
// extern const bool  UnaryFunct_isPrefix[];

// enum BinaryFunct {
// 	// algebraic
// 	BF_PLUS, BF_MINUS, BF_MULT, BF_DIV,
// 	BF_MIN, BF_MAX, BF_MOD, BF_POW, BF_LOG, BF_BINOMIAL,
// 	// boolean
// 	BF_AND, BF_OR,
// 	// total count
// 	BF_NumFuncts
// };
// extern const char* BinaryFunct_Names[];
// extern const bool  BinaryFunct_isPrefix[];

// enum ColorVarFunct {
// 	CVF_NONE, CVF_NEXT, CVF_PREV,
// 	CVF_NumFuncts
// };
// extern const char* ColorVarFunct_Names[];

// enum MSetElemFunct {
// 	MEF_PLUS, MEF_MINUS,
// 	MEF_NumFuncts
// };
// extern const char* MSetElemFunct_Names[];

// //=============================================================================

// const char* UnaryFunct_Names[UF_NumFuncts] = {
// 	"-", "Abs", "Sin", "Cos", "Tan",
// 	"ArcSin", "ArcCos", "ArcTan", "Exp",
// 	"Log", "Sqrt", "Ceil", "Floor", "Factorial",
// 	// boolean
// 	"~"
// };
// const bool  UnaryFunct_isPrefix[UF_NumFuncts] = {
// 	true, false, false, false, false, false,
// 	false, false, false, false, false, false,
// 	false, false,
// 	// boolean
// 	true
// };

// const char* BinaryFunct_Names[BF_NumFuncts] = {
// 	"+", "-", "*", "/", "Min", "Max", "Mod", "Pow", "Log", "Binomial",
// 	// boolean
// 	"&", "|"
// };
// const bool  BinaryFunct_isPrefix[BF_NumFuncts] = {
// 	false, false, false, false, true, true,
// 	true, true, true, true,
// 	// boolean
// 	false, false
// };

// const char* ColorVarFunct_Names[CVF_NumFuncts] = {
// 	"", "++", "--"
// };
// const char* MSetElemFunct_Names[MEF_NumFuncts] = {
// 	"+", "-"
// };

//=============================================================================

// template<class T, class OP>
// struct UnaryOp2 {
// 	inline bool IsPrefix() const;
// 	inline const char* GetName() const;
// 	// inline T operator()(const OP& op) const;
// 	// inline colorclass_t ColorDomainOfResult(const colorclass_t opDomain) const;
// };

// template<class T, class OP, enum UnaryFunct Funct>
// struct UnaryOpBase {
// 	inline bool IsPrefix() const { return UnaryFunct_isPrefix[Funct]; }
// 	inline const char* GetName() const { return UnaryFunct_Names[Funct]; }
// 	// inline T operator()(const OP& op) const;
// 	// inline colorclass_t ColorDomainOfResult(const colorclass_t opDomain) const;
// };

// template<class T, class OP, enum UnaryFunct Funct>
// struct UnaryOpBaseSD : UnaryOpBase<T, OP, Funct> {
// 	// Return the same color domain of its operand
// 	inline colorclass_t ColorDomainOfResult(const colorclass_t opDomain) const
// 	{ return opDomain; }
// };


// //=============================================================================

// template<class T> struct UnaryNegate : UnaryOpBaseSD<T, T, UF_NEG> {
// 	inline T operator()(const T op) const { return -op; }
// };

// template<class T> struct UnaryAbs : UnaryOpBaseSD<T, T, UF_ABS> {
// 	inline T operator()(const T op) const { return std::abs(op); }
// };

// struct UnarySin : UnaryOpBaseSD<double, double, UF_SIN> {
// 	inline double operator()(const double op) const { return std::sin(op); }
// };
// struct UnaryCos : UnaryOpBaseSD<double, double, UF_COS> {
// 	inline double operator()(const double op) const { return std::cos(op); }
// };
// struct UnaryTan : UnaryOpBaseSD<double, double, UF_TAN> {
// 	inline double operator()(const double op) const { return std::tan(op); }
// };
// struct UnaryArcSin : UnaryOpBaseSD<double, double, UF_ARCSIN> {
// 	inline double operator()(const double op) const { return std::asin(op); }
// };
// struct UnaryArcCos : UnaryOpBaseSD<double, double, UF_ARCCOS> {
// 	inline double operator()(const double op) const { return std::acos(op); }
// };
// struct UnaryArcTan : UnaryOpBaseSD<double, double, UF_ARCTAN> {
// 	inline double operator()(const double op) const { return std::atan(op); }
// };

// struct UnaryExp : UnaryOpBaseSD<double, double, UF_EXP> {
// 	inline double operator()(const double op) const { return std::exp(op); }
// };
// struct UnaryLog: UnaryOpBaseSD<double, double, UF_LOG> {
// 	inline double operator()(const double op) const { return std::log(op); }
// };
// struct UnarySqrt: UnaryOpBaseSD<double, double, UF_SQRT> {
// 	inline double operator()(const double op) const { return std::sqrt(op); }
// };

// struct UnaryCeil: UnaryOpBaseSD<double, double, UF_CEIL> {
// 	inline double operator()(const double op) const { return std::ceil(op); }
// };
// struct UnaryFloor: UnaryOpBaseSD<double, double, UF_FLOOR> {
// 	inline double operator()(const double op) const { return std::floor(op); }
// };
// // struct UnaryRound: UnaryOpBaseSD<double, double, UF_ROUND> {
// // 	inline double operator()(const double op) const { return std::round(op); }
// // };

// struct UnaryFactorial: UnaryOpBaseSD<int, int, UF_FACTORIAL> {
// 	inline int operator()(const int op) const {
// 		return factorial_memoized(op, mem_factorial);
// 	}
// protected:
// 	mutable map<int, int> mem_factorial;
// };

// //=============================================================================

// struct UnaryNot : UnaryOpBaseSD<bool, bool, UF_NOT> {
// 	inline bool operator()(const bool& op) const { return !op; }
// };

// //=============================================================================

// template<class T, class OP1, class OP2>
// struct BinaryOp2 {
// 	inline bool IsPrefix() const;
// 	inline const char* GetName() const;
// };

// template<class T, class OP1, class OP2, enum BinaryFunct Funct>
// struct BinaryOpBase {
// 	inline bool IsPrefix() const { return BinaryFunct_isPrefix[Funct]; }
// 	inline const char* GetName() const { return BinaryFunct_Names[Funct]; }
// 	// inline T operator()(const shared_ptr<Expr>& op1, const shared_ptr<Expr>& op1) const;
// 	// inline colorclass_t ColorDomainOfResult(const colorclass_t, const colorclass_t) const;
// };

// template<class T, class OP1, class OP2, enum BinaryFunct Funct>
// struct BinaryOpBaseSD : BinaryOpBase<T, OP1, OP2, Funct>  {
// 	// Require the same color domain for both operands, and return the same
// 	inline colorclass_t ColorDomainOfResult(const colorclass_t opDomain1,
// 											const colorclass_t opDomain2) const
// 	{
// 		assert(opDomain1 == opDomain2);
// 		return opDomain1;
// 	}
// };

// //=============================================================================

// template<class T> struct BinaryPlus : BinaryOpBaseSD<T, T, T, BF_PLUS> {
// 	inline T operator()(const shared_ptr<Expr<T>>& op1, const shared_ptr<Expr<T>>& op2,
// 						const EvaluationContext* ctx) const
// 	{ return op1->Evaluate(ctx) + op2->Evaluate(ctx); }
// };

// template<class T> struct BinaryMinus : BinaryOpBaseSD<T, T, T, BF_MINUS> {
// 	inline T operator()(const shared_ptr<Expr<T>>& op1, const shared_ptr<Expr<T>>& op2,
// 						const EvaluationContext* ctx) const
// 	{ return op1->Evaluate(ctx) - op2->Evaluate(ctx); }
// };

// template<class T> struct BinaryMult : BinaryOpBaseSD<T, T, T, BF_MULT> {
// 	inline T operator()(const shared_ptr<Expr<T>>& op1, const shared_ptr<Expr<T>>& op2,
// 						const EvaluationContext* ctx) const
// 	{ return op1->Evaluate(ctx) * op2->Evaluate(ctx); }
// };

// template<class T> struct BinaryDiv : BinaryOpBaseSD<T, T, T, BF_DIV> {
// 	inline T operator()(const shared_ptr<Expr<T>>& op1, const shared_ptr<Expr<T>>& op2,
// 						const EvaluationContext* ctx) const
// 	{
// 		if (op2 == T(0))
// 			throw evaluation_exception("Division by zero in function evaluation.");
// 		return op1->Evaluate(ctx) / op2->Evaluate(ctx);
// 	}
// };

// template<class T> struct BinaryMin : BinaryOpBaseSD<T, T, T, BF_MIN> {
// 	inline T operator()(const shared_ptr<Expr<T>>& op1, const shared_ptr<Expr<T>>& op2,
// 						const EvaluationContext* ctx) const
// 	{ return std::min(op1->Evaluate(ctx), op2->Evaluate(ctx)); }
// };

// template<class T> struct BinaryMax : BinaryOpBaseSD<T, T, T, BF_MAX> {
// 	inline T operator()(const shared_ptr<Expr<T>>& op1, const shared_ptr<Expr<T>>& op2,
// 						const EvaluationContext* ctx) const
// 	{ return std::max(op1->Evaluate(ctx), op2->Evaluate(ctx)); }
// };

// struct BinaryMod : BinaryOpBaseSD<int, int, int, BF_MOD> {
// 	inline int operator()(const shared_ptr<Expr<int>>& op1, const shared_ptr<Expr<int>>& op2,
// 						 const EvaluationContext* ctx) const
// 	{
// 		if (op2 == 0)
// 			throw evaluation_exception("Modulo by zero in function evaluation.");
// 		return op1->Evaluate(ctx) % op2->Evaluate(ctx);
// 	}
// };

// template<class T> struct BinaryPow : BinaryOpBaseSD<T, T, T, BF_POW> {
// 	inline T operator()(const shared_ptr<Expr<T>>& op1, const shared_ptr<Expr<T>>& op2,
// 						const EvaluationContext* ctx) const
// 	{ return std::pow(op1->Evaluate(ctx), op2->Evaluate(ctx)); }
// };

// struct BinaryLog : BinaryOpBaseSD<double, double, double, BF_POW> {
// 	inline double operator()(const shared_ptr<Expr<double>>& op1, const shared_ptr<Expr<double>>& op2,
// 						     const EvaluationContext* ctx) const
// 	{ return log(op1->Evaluate(ctx)) / log(op2->Evaluate(ctx)); }
// };

// struct BinaryBinomial : BinaryOpBaseSD<int, int, int, BF_BINOMIAL> {
// 	inline int operator()(const shared_ptr<Expr<int>>& op1, const shared_ptr<Expr<int>>& op2,
// 						     const EvaluationContext* ctx) const
// 	{ return binomial_memoized(op1->Evaluate(ctx), op2->Evaluate(ctx), mem_binom); }

// protected:
// 	mutable map<pair<int,int>, int> mem_binom;
// };

// //=============================================================================

// struct BinaryAnd : BinaryOpBaseSD<bool, bool, bool, BF_AND> {
// 	inline bool operator()(const shared_ptr<Expr<bool>>& op1, const shared_ptr<Expr<bool>>& op2,
// 						   const EvaluationContext* ctx) const
// 	{ return op1->Evaluate(ctx) && op2->Evaluate(ctx); }
// };

// struct BinaryOr : BinaryOpBaseSD<bool, bool, bool, BF_OR> {
// 	inline bool operator()(const shared_ptr<Expr<bool>>& op1, const shared_ptr<Expr<bool>>& op2,
// 						   const EvaluationContext* ctx) const
// 	{ return op1->Evaluate(ctx) || op2->Evaluate(ctx); }
// };

// //=============================================================================
// //   Type traits for Expr<> specialization
// //=============================================================================

// template<typename T>
// struct ExprTraits {};

// template<>
// struct ExprTraits<int> {
// 	// Prints the constant
// 	static inline void Print(ostream& os, const PrintExprContext& ctx, int value)
// 	{ os << value; }
// 	static inline void PrintInSPNicaFormat(ostream& os, int value)  { os << value; }
// 	static inline void PrintInCosmosFormat(ostream& os, int value)  { os << value; }
// 	// Cast operator printed with Print()
// 	static inline const char* CastOp();
// };

// template<>
// struct ExprTraits<double> {
// 	// Prints the constant
// 	static inline void Print(ostream& os, const PrintExprContext& ctx, double value)
// 	{ os << value; }
// 	static inline void PrintInSPNicaFormat(ostream& os, double value)  { os << value; }
// 	static inline void PrintInCosmosFormat(ostream& os, double value)  { os << value; }
// 	// Cast operator printed with Print()
// 	static inline const char* CastOp() { return "(double)"; }
// };

// template<>
// struct ExprTraits<bool> {
// 	// Prints the constant
// 	static inline void Print(ostream& os, const PrintExprContext& ctx, int value) {
// 		os << (value ? "True" : "False");
// 	}
// 	static inline void PrintInSPNicaFormat(ostream& os, int value) {
// 		os << (value ? "True" : "False");
// 	}
// 	static inline void PrintInCosmosFormat(ostream& os, int value) {
// 		os << (value ? "TRUE" : "FALSE");
// 	}

// 	// Cast operator printed with Print()
// 	static inline const char* CastOp();
// };

// template<class T>
// struct ExprTraits<MultiSet<T>> {
// 	// Prints the constant
// 	static inline void Print(ostream& os, const PrintExprContext& ctx, const MultiSet<T>& mset)
// 	{ os << print_multiset(mset, ctx); }
// 	static inline void PrintInSPNicaFormat(ostream& os, int value)  { os << value; }
// 	static inline void PrintInCosmosFormat(ostream& os, int value)  { os << value; }
// 	// Cast operator printed with Print()
// 	static inline const char* CastOp();
// };


// #define ThrowProgramExceptionAt(file, line) \
// 	throw program_exception("Internal application error: file " file ":" #line)

// #define InternalError(F,L) ThrowProgramExceptionAt(F,L)

// //=============================================================================








//=============================================================================

// template<class T>
// inline colorclass_t ColorClassOf(const MultiSet<T>& mset) { return mset.domain; }
// inline colorclass_t ColorClassOf(const int)    { return NEUTRAL_COLOR; }
// inline colorclass_t ColorClassOf(const double) { return NEUTRAL_COLOR; }

// //=============================================================================
// // Multiset operations
// //=============================================================================

// template<class T, class UnaryOP>
// inline MultiSet<T> mset_unary(const MultiSet<T>& mset, const UnaryOP op)
// {
// 	MultiSet<T> res;
// 	mset_elem_t N = mset.numElements();

// 	for (mset_elem_t i(0); i<N; i++) {
// 		res.push_back(mset, i, op(mset.mult[i]));
// 	}
// 	return res;
// }

// template<typename T> struct card_mset_unary_op : public UnaryOpFixed<UnaryFunct, UF_MSET_CARDINALITY> {
// 	inline T operator() (const MultiSet<T>& mset) const {
// 		T card = 0;
// 		mset_elem_t N = mset.numElements();
// 		for (mset_elem_t i(0); i<N; i++)
// 			card += mset.mult[i];
// 		return card;
// 	}
// };

// //=============================================================================

// template<class T, class BinaryOP>
// inline MultiSet<T> mset_mset_binary(const MultiSet<T>& mset1, const MultiSet<T>& mset2,
// 							        const BinaryOP op, const T neutralTerm)
// {
// 	MultiSet<T> res;
// 	assert(mset1.domain == mset2.domain);
// 	assert(mset1.elems.size() == mset2.elems.size());
// 	mset_elem_t N1 = mset1.numElements(), N2 = mset2.numElements(), i1(0), i2(0);

// 	while (i1 < N1 && i2 < N2) {
// 		color_tuple_t ct1 = mset1.elems[i1], ct2 = mset2.elems[i2];
// 		if (ct1 < ct2) {
// 			res.push_back(ct1, op(mset1.mult[i1], neutralTerm));
// 			++i1;
// 		}
// 		else if (ct1 > ct2) {
// 			res.push_back(ct2, op(neutralTerm, mset2.mult[i2]));
// 			++i2;
// 		}
// 		else { // ct1 == ct2
// 			res.push_back(ct1, op(mset1.mult[i1], mset2.mult[i2]));
// 			++i1;
// 			++i2;
// 		}
// 	}
// 	while (i1 < N1) {
// 		res.push_back(mset1.elems[i1], op(mset1.mult[i1], neutralTerm));
// 		++i1;
// 	}
// 	while (i2 < N2) {
// 		res.push_back(mset2.elems[i2], op(neutralTerm, mset2.mult[i2]));
// 		++i2;
// 	}
// 	return res;
// }

// //=============================================================================

// template<typename T>
// struct BinaryMsetPlus : public BinaryOpBaseSD<MultiSet<T>, MultiSet<T>, MultiSet<T>, BF_PLUS> {
// 	inline MultiSet<T> operator()(const shared_ptr<Expr<MultiSet<T>>>& op1,
// 							      const shared_ptr<Expr<MultiSet<T>>>& op2,
// 							      const EvaluationContext* ctx) const
// 	{
// 		return mset_mset_binary(op1->Evaluate(ctx), op2->Evaluate(ctx),
// 								plus_binary_op<T>(), T(0));
// 	}
// };

// //=============================================================================



// //=============================================================================
// // Evaluation of unary operators
// //=============================================================================

// template<typename T, typename OP, typename FN>
// struct UnaryEvaluate {
// 	inline T operator() (const shared_ptr<Expr<OP> >& ex, const FN& func,
// 			             const EvaluationContext* ctx) {
// 		return func(ex->Evaluate(ctx));
// 	}
// };

// //=============================================================================
// // Evaluation of binary operators
// //=============================================================================

// template<typename T, typename OP, typename FN>
// struct BinaryEvaluate {
// 	inline T operator() (const shared_ptr<Expr<OP> >& ex1, const shared_ptr<Expr<OP> >& ex2,
// 			             const FN& func, const EvaluationContext* ctx) {
// 		return func(ex1->Evaluate(ctx), ex2->Evaluate(ctx));
// 	}
// };

// // Specialized versions for AND and OR binary operators
// template<typename T, typename OP>
// struct BinaryEvaluate<T, OP, and_binary_op> {
// 	inline T operator() (const shared_ptr<Expr<OP> >& ex1, const shared_ptr<Expr<OP> >& ex2,
// 			             const and_binary_op& func, const EvaluationContext* ctx) {
// 		return ex1->Evaluate(ctx) && ex2->Evaluate(ctx);
// 	}
// };
// template<typename T, typename OP>
// struct BinaryEvaluate<T, OP, or_binary_op> {
// 	inline T operator() (const shared_ptr<Expr<OP> >& ex1, const shared_ptr<Expr<OP> >& ex2,
// 			             const or_binary_op& func, const EvaluationContext* ctx) {
// 		return ex1->Evaluate(ctx) || ex2->Evaluate(ctx);
// 	}
// };








// //=============================================================================
// //   A constant value of type T
// //=============================================================================

// template<class T>
// struct ConstValExpr : public Expr<T> {
// 	typedef T result_t;
// 	ConstValExpr(T _value) : value(_value) { }
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		return value;
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		ExprTraits<T>::Print(os, ctx, value);
// 	}
// 	// Test for marking-dependency
// 	virtual bool IsMarkingDep() const override {
// 		return false;
// 	}
// 	// Color class of this constant
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		return ColorClassOf(value);
// 	}
// protected:
// 	T   value;	// The constant value
// };

// //=============================================================================
// //   A parameter in the context of the PN (i.e. a named constant, a mpar/rpar, etc)
// //=============================================================================

// template<class T>
// struct ParamValExpr : public ParamExpr<T> {
// 	typedef T result_t;
// 	ParamValExpr(const char* n, const shared_ptr<Expr<T>> v) : name(n), value(v) { }

// 	const string& GetName() const				{   return name;   }
// 	void SetValue(shared_ptr<Expr<T>> newvalue) {   value = newvalue;   }
// 	const shared_ptr<Expr<T>> GetValue() const {   return value;   }

// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		return value->Evaluate(ctx);
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (ctx.expandParamNames)
// 			value->Print(os, ctx);
// 		else
// 			os << name;
// 	}
// 	// Test for marking-dependency
// 	virtual bool IsMarkingDep() const override {
// 		return value->IsMarkingDep();
// 	}
// 	// Color class of this parameter
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		return value->GetColorDomain(ccc);
// 	}
// protected:
// 	string   			   name;  // The name of the parameters
// 	shared_ptr<Expr<T>>    value; // The value of the parameter
// };

// //=============================================================================
// //   A cast from another expression type
// //=============================================================================

// template<class T, class TFrom>
// struct CastExpr : public Expr<T> {
// 	typedef T result_t;
// 	CastExpr(const shared_ptr<Expr<TFrom> > _ex) : ex(_ex) { }
// 	// Get the actual value of the expression, with the type cast
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		return T(ex->Evaluate(ctx));
// 	}
// 	// Print this cast expression
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		os << ExprTraits<T>::CastOp();
// 		ex->Print(os, ctx);
// 	}
// 	// Test for marking-dependency
// 	virtual bool IsMarkingDep() const override {
// 		return ex->IsMarkingDep();
// 	}
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		return ex->GetColorDomain(ccc);
// 	}
// protected:
// 	shared_ptr<Expr<TFrom>>   ex;	// The cast'ed expression
// };

// //=============================================================================
// //   A place marking (Marking-dependent)
// //=============================================================================

// template<class T>
// struct PlaceMarkingExpr : public Expr<T> {
// 	typedef T result_t;
// 	PlaceMarkingExpr(const string& _plName, place_t _plInd)
// 	/**/ : plName(_plName), plInd(_plInd) { }

// 	// This is the base of all marking-dependent expression
// 	virtual bool IsMarkingDep() const override { return true; }
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// #ifndef NDEBUG
// 		if (ctx == nullptr)
// 			throw program_exception("Evaluating a marking-dependent expression without a marking.");
// #endif
// 		return (*ctx->marking)[ plInd ];
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (ctx.expandParamNames)
// 			os << "#" << (plInd + place_t(1));
// 		else
// 			os << "#" << plName;
// 	}
// 	// Color domain of the referenced place
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		return ccc.colorDomainOf(plInd);
// 	}
// protected:
// 	string		plName;  // The name of the place, as written in the expr.
// 	place_t		plInd;	 // Index of the place in the PN
// };


// //=============================================================================
// //   Unary Expression
// //=============================================================================

// template<class T, class OP, class FN>
// struct UnaryExpr : public Expr<T> {
// 	typedef T result_t;
// 	typedef OP operand_t;
// 	typedef FN unary_funct_t;

// 	UnaryExpr(const shared_ptr<Expr<OP> > _ex, const FN& _fn = FN())
// 	/**/ : ex(_ex), func(_fn) { }

// 	// Tests if this object is really marking dependent
// 	virtual bool IsMarkingDep() const override { return ex->IsMarkingDep(); }
// 	// Evaluate the unary operator
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		return func(ex->Evaluate(ctx));
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (func.IsPrefix()) {
// 			os << "(" << func.GetName();
// 			ex->Print(os, ctx);
// 			os << ")";
// 		} else {
// 			os << func.GetName() << "[";
// 			ex->Print(os, ctx);
// 			os << "]";
// 		}
// 	}
// 	// Color class after the application of the unary function
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		return func.ColorDomainOfResult(ex->GetColorDomain(ccc));
// 	}
// protected:
// 	shared_ptr<Expr<OP>>  ex;    // Argument
// 	unary_funct_t  		  func;  // Unary function ID
// };


// //=============================================================================
// //   Binary Expression
// //=============================================================================

// template<class T, class OP, class FN>
// struct BinaryExpr : public Expr<T> {
// 	typedef T result_t;
// 	typedef OP operand_t;
// 	typedef FN binary_funct_t;

// 	BinaryExpr(const shared_ptr<Expr<OP> > _ex1,
// 			   const shared_ptr<Expr<OP> > _ex2,
// 			   const FN& _fn = FN())
// 	/**/ : ex1(_ex1), ex2(_ex2), func(_fn) { }

// 	// Tests if this object is really marking dependent
// 	virtual bool IsMarkingDep() const {
// 		return ex1->IsMarkingDep() || ex2->IsMarkingDep();
// 	}
// 	// Evaluate this binary expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		return func(ex1, ex2, ctx);
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (func.IsPrefix()) {
// 			os << func.GetName() << "[";
// 			ex1->Print(os, ctx);
// 			os << ",";
// 			ex2->Print(os, ctx);
// 			os << "]";
// 		} else {
// 			os << "(";
// 			ex1->Print(os, ctx);
// 			os << " " << func.GetName() << " ";
// 			ex2->Print(os, ctx);
// 			os << ")";
// 		}
// 	}
// 	// Color class after the application of the binary function
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		return func.ColorDomainOfResult(ex1->GetColorDomain(ccc),
// 										ex2->GetColorDomain(ccc));
// 	}
// protected:
// 	shared_ptr<Expr<OP>>  ex1, ex2;  // Arguments
// 	binary_funct_t  	  func;  	 // Binary function
// };

// //=============================================================================
// //   If-Then-Else conditional expression
// //=============================================================================

// template<class T>
// struct IfThenElseExpr : public Expr<T> {
// 	typedef T result_t;

// 	IfThenElseExpr(const shared_ptr<Expr<bool>> _cond,
// 				   const shared_ptr<Expr<T>> _exT,
// 				   const shared_ptr<Expr<T>> _exF)
// 	/**/ : cond(_cond), exT(_exT), exF(_exF) { }

// 	// Tests if this object is really marking dependent
// 	virtual bool IsMarkingDep() const {
// 		return cond->IsMarkingDep() || exT->IsMarkingDep() || exF->IsMarkingDep();
// 	}
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		return cond->Evaluate(ctx) ? exT->Evaluate(ctx) : exF->Evaluate(ctx);
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		os << "If[";	cond->Print(os, ctx);
// 		os << ", ";		exT->Print(os, ctx);
// 		os << ", ";		exF->Print(os, ctx);
// 		os << "]";
// 	}
// 	// Color class of the result
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		colorclass_t cT = exT->GetColorDomain(ccc);
// 		colorclass_t cF = exF->GetColorDomain(ccc);
// 		verify(cT == cF);
// 		return cT;
// 	}
// protected:
// 	shared_ptr<Expr<bool>>	 cond;  	// Scalar condition
// 	shared_ptr<Expr<T>>  	 exT, exF;	// Arguments for the true/false cases
// };

// //=============================================================================
// //   Indirect acces to a Expr* object pointer
// //=============================================================================

// template<class T>
// struct PtrToExpr : public Expr<T> {
// 	typedef T result_t;
// 	PtrToExpr(const char* s,
// 		      const shared_ptr<Expr<T>> _ex = shared_ptr<Expr<T>>())
// 	: name(s), ex(_ex) { }

// 	// Tests if this object is really marking dependent
// 	virtual bool IsMarkingDep() const {
// 		assert(ex);
// 		return ex->IsMarkingDep();
// 	}
// 	// Evaluate the indirect expression pointer
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		assert(ex);
// 		return ex->Evaluate(ctx);
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (!ex)
// 			os << "<<" << name << ">>";
// 		else
// 			ex->Print(os, ctx);
// 	}
// 	// Color class of the result
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		assert(ex);
// 		return ex->GetColorDomain(ccc);
// 	}

// 	// Get the symbolic name of this pointer object
// 	inline const string& get_name() const { return name; }
// 	// Bind a new expression to this pointer
// 	inline void bind(const shared_ptr<Expr<T>>& _ex) { ex = _ex; }
// protected:
// 	string 			     name;  // Name of this pointer object
// 	shared_ptr<Expr<T>>  ex;    // Pointed expression (can be unbounded, i.e. null)
// };

// //=============================================================================





// //=============================================================================
// // A multiset term that is a color variable
// //=============================================================================

// struct ColorVarMsetTerm : public MsetTerm {
// 	ColorVarMsetTerm(const string& _cvarName, colorvar_t _cvarInd,
// 					 ColorVarFunct _funct, colorclass_t _clClass)
// 	/**/ : cvarName(_cvarName), cvarInd(_cvarInd), funct(_funct), clClass(_clClass) { }

// 	// This is firing-dependent and marking dependent
// 	virtual bool IsMarkingDep() const override { return true; }
// 	// This is firing-dependent and marking dependent
// 	virtual bool IsFiringDep() const override { return true; }
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// #ifndef NDEBUG
// 		if (ctx == nullptr)
// 			throw program_exception("Evaluating a firing-dependent expression "
// 									"without a valid context.");
// #endif
// 		color_t clr = (*ctx->binding).bindingOf(cvarInd);
// 		switch (funct) {
// 			case CVF_NONE:
// 				return clr;
// 			case CVF_NEXT: {
// 				const ColorClass& cc = ctx->colorClassOf(clClass);
// 				return (clr + color_t(1)) % cc.numColors();
// 			}
// 			case CVF_PREV: {
// 				const ColorClass& cc = ctx->colorClassOf(clClass);
// 				color_t num = cc.numColors();
// 				return (clr - color_t(1) + num) % num;
// 			}
// 			default:
// 				throw program_exception("Internal error in ColorVarMsetTerm.");
// 		}
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (ctx.expandParamNames)
// 			os << (cvarInd + colorvar_t(1)) << ColorVarFunct_Names[funct] ;
// 		else
// 			os << cvarName << ColorVarFunct_Names[funct] ;
// 	}
// 	// A single color var is a single color
// 	virtual bool IsSet() const override { return false; }
// 	// Special evaluation function for multiset terms that are color sets
// 	virtual colorset_t EvaluateSet(colorclass_t termClass, const EvaluationContext* ctx) const override {
// 		verify(termClass == clClass);
// 		return colorset_t{ Evaluate(ctx) };
// 	}
// 	// Color class of this multiset term
// 	virtual colorclass_t GetColorClass() const override { return clClass; }

// protected:
// 	string		cvarName;  // The name of the colorvar, as written in the expr.
// 	colorvar_t	cvarInd;   // Index of the colorvar in the PN.
// 	ColorVarFunct funct;   // Function applied to this colorvar (++, --, none)
// 	colorclass_t clClass;  // Index of the color class of the color variable.
// };

// //=============================================================================
// // A multiset term with ALL colors of a color class
// //=============================================================================

// struct AllColorsMsetTerm : MsetTerm {
// 	AllColorsMsetTerm() {}
// 	virtual bool IsMarkingDep() const override { return false; }
// 	virtual bool IsFiringDep() const override { return false; }
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		throw program_exception("Evaluates to a set of colors.");
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		os << "All";
// 	}
// 	// Evaluates to a set that contains all the colors of a color class
// 	virtual bool IsSet() const override { return true; }
// 	// Returns all the colors of a color class @termClass
// 	virtual colorset_t EvaluateSet(colorclass_t termClass, const EvaluationContext* ctx) const override {
// 		colorset_t all;
// 		const ColorClass& clClass = ctx->colorClassOf(termClass);
// 		verify(clClass.isSimple());
// 		for (color_t c(0); c < clClass.numColors(); c++)
// 			all.insert(c);
// 		return all;
// 	}
// 	virtual colorclass_t GetColorClass() const override {
// 		return ALL_COLOR_CLASS;
// 	}
// };

// //=============================================================================
// // A multiset term with a single color of a color class
// //=============================================================================

// struct SingleColorMsetTerm : public MsetTerm {
// 	SingleColorMsetTerm(const string& _colorName, color_t _colorInd, colorclass_t _clClass)
// 	/**/ : colorName(_colorName), colorInd(_colorInd), clClass(_clClass) { }

// 	virtual bool IsMarkingDep() const override { return false; }
// 	virtual bool IsFiringDep() const override { return false; }
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		return colorInd;
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (ctx.expandParamNames)
// 			os << (colorInd + color_t(1));
// 		else
// 			os << colorName;
// 	}
// 	// Evaluates to a single color
// 	virtual bool IsSet() const override { return false; }
// 	// Special evaluation function for multiset terms that are color sets
// 	virtual colorset_t EvaluateSet(colorclass_t termClass, const EvaluationContext* ctx) const override {
// 		verify(termClass == clClass);
// 		return colorset_t{ Evaluate(ctx) };
// 	}
// 	// Color class of this multiset term
// 	virtual colorclass_t GetColorClass() const override { return clClass; }

// protected:
// 	string		colorName;  // The name of the single color.
// 	color_t		colorInd;   // Index of the color in the color class
// 	colorclass_t clClass;   // Index of the color class of this color.
// };

// //=============================================================================
// // A multiset term with an entire static subclass of a color class
// //=============================================================================

// struct StaticSubclassMsetTerm : public MsetTerm {
// 	StaticSubclassMsetTerm(const string& _subClName, static_subclass_t _sSubInd,
// 						   colorclass_t _clClass)
// 	/**/ : subClName(_subClName), sSubInd(_sSubInd), clClass(_clClass) { }

// 	virtual bool IsMarkingDep() const override { return false; }
// 	virtual bool IsFiringDep() const override { return false; }
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		throw program_exception("Evaluates to a set of colors.");
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (ctx.expandParamNames)
// 			os << (sSubInd + static_subclass_t(1));
// 		else
// 			os << subClName;
// 	}
// 	// Evaluates to a set that contains all the colors of this static subclass
// 	virtual bool IsSet() const override { return true; }
// 	// Returns all the colors of the static subclass
// 	virtual colorset_t EvaluateSet(colorclass_t termClass, const EvaluationContext* ctx) const override {
// 		verify(termClass == clClass);
// 		colorset_t subClCols;
// 		const ColorClass& clClass = ctx->colorClassOf(termClass);
// 		verify(clClass.isSimple() && sSubInd < clClass.subclasses.size());
// 		color_t start = clClass.startOfSubclass(sSubInd);
// 		color_t num = clClass.subclasses[sSubInd].numColors();
// 		for (color_t c(0); c < num; c++)
// 			subClCols.insert(start + c);
// 		return subClCols;
// 	}
// 	// Color class of this multiset term
// 	virtual colorclass_t GetColorClass() const override { return clClass; }

// protected:
// 	string			   subClName; // The name of the static color subclass.
// 	static_subclass_t  sSubInd;   // Index in the static subclasses list of the color class
// 	colorclass_t 	   clClass;   // Index of the color class of this subclass.
// };

// //=============================================================================
// // A multiset term with an entire color class
// //=============================================================================

// struct ColorClassMsetTerm : public MsetTerm {
// 	ColorClassMsetTerm(const string& _clClassName, colorclass_t _clClass)
// 	/**/ : clClassName(_clClassName), clClass(_clClass) { }

// 	virtual bool IsMarkingDep() const override { return false; }
// 	virtual bool IsFiringDep() const override { return false; }
// 	// Get the actual value of the expression
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		throw program_exception("Evaluates to a set of colors.");
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		if (ctx.expandParamNames)
// 			os << (clClass + colorclass_t(1));
// 		else
// 			os << clClassName;
// 	}
// 	// Evaluates to a set that contains all the colors of this  color class
// 	virtual bool IsSet() const override { return true; }
// 	// Returns all the colors of the color class
// 	virtual colorset_t EvaluateSet(colorclass_t termClass, const EvaluationContext* ctx) const override {
// 		verify(termClass == clClass);
// 		colorset_t all;
// 		const ColorClass& clClass = ctx->colorClassOf(termClass);
// 		verify(clClass.isSimple());
// 		for (color_t c(0); c < clClass.numColors(); c++)
// 			all.insert(c);
// 		return all;
// 	}
// 	// Color class of this multiset term
// 	virtual colorclass_t GetColorClass() const override { return clClass; }

// protected:
// 	string		  clClassName; // The name of the color class.
// 	colorclass_t  clClass;     // Index of the color class.
// };

// //=============================================================================
// // Union or subtraction of multiset elements
// //=============================================================================

// struct BinaryMsetTerm : public MsetTerm {
// 	BinaryMsetTerm(const shared_ptr<MsetTerm> _ex1, const shared_ptr<MsetTerm> _ex2,
// 				   MSetElemFunct _fn) : ex1(_ex1), ex2(_ex2), fn(_fn) { }

// 	virtual bool IsMarkingDep() const override {
// 		return ex1->IsMarkingDep() || ex2->IsMarkingDep();
// 	}
// 	virtual bool IsFiringDep() const override {
// 		return ex1->IsFiringDep() || ex2->IsFiringDep();
// 	}
// 	// Binary mset terms always evaluate to a set
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		throw program_exception("Evaluates to a set of colors.");
// 	}
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		ex1->Print(os, ctx);
// 		os << " " << MSetElemFunct_Names[fn] << " ";
// 		ex2->Print(os, ctx);
// 	}
// 	// Evaluates to a set that contains the result of the binary operation
// 	virtual bool IsSet() const override { return true; }
// 	// Returns all the colors of the color class
// 	virtual colorset_t EvaluateSet(colorclass_t termClass, const EvaluationContext* ctx) const override {
// 		colorset_t cs1 = ex1->EvaluateSet(termClass, ctx);
// 		colorset_t cs2 = ex2->EvaluateSet(termClass, ctx);
// 		switch (fn) {
// 			case MEF_PLUS: {
// 				for (color_t c : cs2)
// 					cs1.insert(c);
// 				return cs1;
// 			}
// 			case MEF_MINUS: {
// 				for (color_t c : cs2)
// 					cs1.erase(c);
// 				return cs1;
// 			}
// 			default:
// 				throw program_exception("Internal error");
// 		}
// 	}
// 	// Color class of this multiset term
// 	virtual colorclass_t GetColorClass() const override {
// 		colorclass_t c1 = ex1->GetColorClass(), c2 = ex2->GetColorClass();
// 		verify(c1 == c2 || (c1 == ALL_COLOR_CLASS) || (c2 == ALL_COLOR_CLASS));
// 		return (c1 != ALL_COLOR_CLASS) ? c1 : c2;
// 	}

// protected:
// 	shared_ptr<MsetTerm> ex1; // Left operand
// 	shared_ptr<MsetTerm> ex2; // Right operand
// 	MSetElemFunct fn;         // multiset binary function (union, subtraction)
// };


// //=============================================================================





// //=============================================================================
// // Element product made of multiset terms, like:  < x, y++, All, All - x >
// //=============================================================================

// template<class T>
// struct MsetExprElemProduct : Expr<MultiSet<T>> {
// 	typedef MultiSet<T> result_t;
// 	MsetExprElemProduct(const shared_ptr<Expr<T>> _multEx,
// 						const shared_ptr<MsetTerm> firstTerm,
// 						colorclass_t _clClass)
// 	: multEx(_multEx), terms{firstTerm}, clClass(_clClass) { }

// 	virtual bool IsMarkingDep() const override {
// 		if (multEx->IsMarkingDep())
// 			return true;
// 		for (auto& term : terms)
// 			if (term->IsMarkingDep())
// 				return true;
// 		return false;
// 	}
// 	virtual bool IsFiringDep() const override {
// 		if (multEx->IsFiringDep())
// 			return true;
// 		for (auto& term : terms)
// 			if (term->IsFiringDep())
// 				return true;
// 		return false;
// 	}

// 	// create the multiset of terms generated by this element product
// 	virtual result_t Evaluate(const EvaluationContext* ctx) const override {
// 		T mult = multEx->Evaluate(ctx);
// 		MultiSet<T> outSet;
// 		const ColorClass& cl = ctx->colorClassOf(clClass);
// 		verify(cl.numClassesInDomain() == terms.size());
// 		domain_comp_t numCl = cl.numClassesInDomain();
// 		DomainElement tuple;
// 		tuple.domain = clClass;
// 		tuple.elems.resize(numCl);

// 		makeMultiSetRecursively(mult, domain_comp_t(0), numCl, ctx, cl, tuple, outSet);
// 		return outSet;
// 	}

// private:
// 	void makeMultiSetRecursively(const T mult, domain_comp_t level, const domain_comp_t numCl,
// 								 const EvaluationContext* ctx, const ColorClass& cl,
// 								 DomainElement& tuple, MultiSet<T>& outSet) const
// 	{
// 		if (level == numCl) {
// 			// Insert this tuple in the multiset
// 			color_tuple_t new_elem = encodeTuple(tuple, *ctx);
// 			outSet.push_back(new_elem, mult);
// 			return;
// 		}
// 		const shared_ptr<MsetTerm>& term = terms[level];
// 		if (term->IsSet()) { // Set of many colors
// 			colorset_t cset = term->EvaluateSet(cl.getIthColorClass(level), ctx);
// 			for (color_t c : cset) {
// 				tuple.elems[level] = c;
// 				makeMultiSetRecursively(mult, level + domain_comp_t(1), numCl,
// 									ctx, cl, tuple, outSet);
// 			}
// 		}
// 		else { // Single color element
// 			tuple.elems[level] = term->Evaluate(ctx);
// 			makeMultiSetRecursively(mult, level + domain_comp_t(1), numCl,
// 									ctx, cl, tuple, outSet);
// 		}
// 	}

// public:
// 	// Print this object
// 	virtual void Print(ostream& os, const PrintExprContext& ctx) const override {
// 		for (domain_comp_t i(0); i<terms.size(); i++) {
// 			os << (i==domain_comp_t(0) ? "<" : ", ");
// 			terms[i]->Print(os, ctx);
// 		}
// 		os << ">";
// 	}
// 	// The color domain of this tuple of multiset term product
// 	virtual colorclass_t GetColorDomain(const ColorClassContainer& ccc) const override {
// 		return clClass;
// 	}

// 	// Add a new term to this element product
// 	void AddElem(shared_ptr<MsetTerm> t) { terms.push_back(t); }

// 	// Determine the color domain of this element product
// 	// Only for the case were the color domain is not known a-priori
// 	void DetermineDomain(const PN& pn) {
// 		for (const ColorClass& cl : pn.clrClasses) {
// 			if (cl.numClassesInDomain() == terms.size()) {
// 				bool match = true;
// 				for (domain_comp_t d(0); d<terms.size(); d++) {
// 					verify(terms[d]->GetColorClass() != ALL_COLOR_CLASS);
// 					if (terms[d]->GetColorClass() != cl.getIthColorClass(d)) {
// 						match = false;
// 						break;
// 					}
// 				}
// 				if (match) {
// 					clClass = cl.index;
// 					return;
// 				}
// 			}
// 		}
// 		ostringstream oss;
// 		oss << "Multiset " << print_petrinetobj(*this) << " is not in a defined color domain.";
// 		throw semantic_parse_exception(oss.str());
// 	}

// protected:
// 	shared_ptr<Expr<T>>    multEx;  // Multiplicity expression
// 	ivector2<shared_ptr<MsetTerm>, domain_comp_t>  terms; // Terms of the multiset tuple
// 	colorclass_t           clClass; // Color of this tuple object
// };

// //=============================================================================











// template struct MultiSet<int>;
// template struct ParamExpr<int>;
// template struct ConstValExpr<int>;
// template struct ParamValExpr<int>;
// template struct CastExpr<double, int>;
// template struct PlaceMarkingExpr<int>;
// template struct UnaryExpr<int, int, UnaryNegate<int>>;
// template struct BinaryExpr<int, int, BinaryPlus<int>>;
// template struct BinaryExpr<bool, bool, BinaryAnd>;
// template struct BinaryExpr<bool, bool, BinaryOr>;
// template struct IfThenElseExpr<int>;
// template struct PtrToExpr<int>;
// template struct MsetExprElemProduct<int>;
// template struct BinaryMsetPlus<int>;
// template struct ParamExpr<MultiSet<int>>;
// template struct ConstValExpr<MultiSet<int>>;
// template struct ParamValExpr<MultiSet<int>>;
// template struct BinaryExpr<MultiSet<int>, MultiSet<int>, BinaryMsetPlus<int>>;






































// //-----------------------------------------------------------------------------

// // std::function<ostream& (ostream&)>
// // print_petrinetobj(const PetriNetObject& pnobj, const SparseMarking *marking,
// // 				  bool expandParamNames)
// // {
// // 	return [&](ostream& os) -> ostream& {
// // 		PrintExprContext pec;
// // 		pec.marking          = marking;
// // 		pec.expandParamNames = expandParamNames;
// // 		pnobj.Print(os, pec);
// // 		return os;
// // 	};
// // }

// // std::function<ostream& (ostream&)>
// // print_petrinetobj(const shared_ptr<PetriNetObject> pnobj, const SparseMarking *marking,
// // 				  bool expandParamNames)
// // {
// // 	return print_petrinetobj(*pnobj, marking, expandParamNames);
// // }



// // struct ColorClassTag { typedef size_t value_type; };

// // typedef opaque_int<ColorClassTag>		colorclass_t;
// // using colorclass_t = opaque_int<ColorClassTag>;


// void Experiment2(const char* arg) {
// 	PN pn;

// 	colorclass_t indC1(0), indC2(1), indC3(2), indC1xC2(3), indC1xC2xC3(4);
// 	shared_ptr<ColorClass> cl_C1 = make_shared<ColorClass>();
// 	cl_C1->index = indC1;
// 	cl_C1->name = "C1";
// 	StaticSubclass S11;
// 	S11.colors = { "a", "b", "c", "d" };
// 	cl_C1->subclasses.emplace_back(std::move(S11));
// 	pn.clrClasses.emplace_back(std::move(cl_C1));

// 	shared_ptr<ColorClass> cl_C2 = make_shared<ColorClass>();
// 	cl_C2->index = indC2;
// 	cl_C2->name = "C2";
// 	StaticSubclass S21;
// 	S21.colors = { "x", "y", "z" };
// 	cl_C2->subclasses.emplace_back(std::move(S21));
// 	pn.clrClasses.emplace_back(std::move(cl_C2));

// 	shared_ptr<ColorClass> cl_C3 = make_shared<ColorClass>();
// 	cl_C3->index = indC3;
// 	cl_C3->name = "C3";
// 	StaticSubclass S31;
// 	S31.colors = { "m", "n", "o", "p", "q" };
// 	cl_C3->subclasses.emplace_back(std::move(S31));
// 	pn.clrClasses.emplace_back(std::move(cl_C3));

// 	shared_ptr<ColorClass> cl_C1xC2 = make_shared<ColorClass>();
// 	cl_C1xC2->index = indC1xC2;
// 	cl_C1xC2->name = "C1xC2";
// 	cl_C1xC2->cross = { cl_C1, cl_C2 };
// 	pn.clrClasses.emplace_back(std::move(cl_C1xC2));

// 	shared_ptr<ColorClass> cl_C1xC2xC3 = make_shared<ColorClass>();
// 	cl_C1xC2xC3->index = indC1xC2xC3;
// 	cl_C1xC2xC3->name = "C1xC2xC3";
// 	cl_C1xC2xC3->cross = { cl_C1, cl_C2, cl_C3 };
// 	pn.clrClasses.emplace_back(std::move(cl_C1xC2xC3));

// 	for (color_t c1(0); c1 < color_t(4); c1++) {
// 		for (color_t c2(0); c2 < color_t(3); c2++) {
// 			DomainElement de, decDe;
// 			de.domain = cl_C1xC2.get();
// 			de.elems = { c1, c2 };

// 			cout << print_domain_elem(de) << "  " << flush;
// 			color_tuple_t tuple_de = encodeTuple(de);
// 			decodeTuple(tuple_de, de.domain, decDe);
// 			cout << print_domain_elem(decDe) << "  " << tuple_de << endl;
// 		}
// 	}
// 	for (color_t c1(0); c1 < color_t(4); c1++) {
// 		for (color_t c2(0); c2 < color_t(3); c2++) {
// 			for (color_t c3(0); c3 < color_t(5); c3++) {
// 				DomainElement de, decDe;
// 				de.domain = cl_C1xC2xC3.get();
// 				de.elems = { c1, c2, c3 };

// 				cout << print_domain_elem(de) << "  " << flush;
// 				color_tuple_t tuple_de = encodeTuple(de);
// 				decodeTuple(tuple_de, de.domain, decDe);
// 				cout << print_domain_elem(decDe) << "  " << tuple_de << "  " << (de == decDe) << endl;
// 			}
// 		}
// 	}

// 	MultiSet<int> mset;
// 	mset.domain = indC1xC2xC3;
// 	mset.push_back(color_tuple_t(13), 2);
// 	mset.push_back(color_tuple_t(0), 1);
// 	mset.push_back(color_tuple_t(1), 4);
// 	mset.push_back(color_tuple_t(18), 3);
// 	cout << print_multiset(mset) << endl;
// }


// struct ABC {
// 	int val;
// 	ABC(int v = 0) : val(v) {}
// };

// template class numerical::opaque_int<ColorClassTag>;
// template class numerical::ivector2<ABC, colorclass_t>;

// void experiment() {
// 	colorclass_t cc2(2), cc0(0);
// 	// cc2 = -4;
// 	cc2 = cc0 + cc2;
// 	for (colorclass_t c(0); c<cc2; c++) {
// 		if (c > colorclass_t(10))
// 			c += colorclass_t(1);
// 	}

// 	ivector2<ABC, colorclass_t> vec;
// 	vec.push_back(ABC(2));
// 	for (colorclass_t c(0); c<vec.size(); c++) {
// 		cout << c << "  " << vec[c].val << endl;
// 		// if (c > (unsigned short)0)
// 		// 	cout << endl;
// 		// if (vec[c])
// 		// 	cout << endl;
// 		// if (vec[c] > 0)
// 		// 	cout << endl;
// 	}
// 	cout << vec[cc0].val << endl;
// 	// cout << vec[2].val << endl;

// }


#if 0

#include "compact_table_impl.h"

struct A {
    A(int i = 0) {}

    size_t p1;
    int p2;
    double p3;

    bool operator == (const A &a) const { return p1 == a.p1 && p2 == a.p2 && p3 == a.p3; }
    bool operator != (const A &a) const { return !(*this == a); }
};
ostream &operator << (ostream &os, const A &a) { return os; }

// template<>
// struct tuple_conv<A, std::tuple<size_t, int, double>> {
// 	inline A empty_elem() const { return A(); }
// 	inline std::tuple<size_t, int, double>
// 	to_tuple(const A& arg) const
// 	{ return make_tuple(arg.p1, arg.p2, arg.p3); }

// 	inline A
// 	from_tuple(std::tuple<size_t, int, double> tuple) const {
// 		A arg;
// 		arg.p1 = std::get<0>(tuple);
// 		arg.p2 = std::get<1>(tuple);
// 		arg.p3 = std::get<2>(tuple);
// 		return arg;
// 	}
// };

// template<>
// struct tuple_conv<tokencount_t, std::tuple<size_t>> {
// 	inline tokencount_t empty_elem() const { return tokencount_t(0); }
// 	inline std::tuple<size_t>
// 	to_tuple(const tokencount_t& arg) const
// 	{ return make_tuple((size_t)arg); }

// 	inline tokencount_t
// 	from_tuple(std::tuple<size_t> tuple) const {
// 		return tokencount_t(std::get<0>(tuple));
// 	}
// };

template<>
struct sequence_traits<std::vector<A>> {
    typedef std::vector<A>        			  vector_type;
    typedef A  		 			 			  value_type;
    typedef table<col::uint_col<size_t>,
            col::int_col<int>,
            col::indexed_col<double>>   table_type;
    typedef typename table_type::tuple_type   tuple_type;

    inline A empty_elem() const { return A(); }
    inline std::tuple<size_t, int, double>
    to_tuple(const A &arg) const
    { return make_tuple(arg.p1, arg.p2, arg.p3); }

    inline A
    from_tuple(std::tuple<size_t, int, double> tuple) const {
        A arg;
        arg.p1 = std::get<0>(tuple);
        arg.p2 = std::get<1>(tuple);
        arg.p3 = std::get<2>(tuple);
        return arg;
    }
};
template<>
struct sequence_traits<sparsevector<size_t, A>> : sequence_traits<std::vector<A>> {
    typedef sparsevector<size_t, A>        	  vector_type;
};

template<>
struct sequence_traits<std::vector<tokencount_t>> {
    typedef std::vector<tokencount_t>        	vector_type;
    typedef tokencount_t  		 			 	value_type;
    typedef table<col::uint_col<tokencount_t>>  table_type;
    typedef typename table_type::tuple_type  	tuple_type;

    inline value_type empty_elem() const { return tokencount_t(0); }
    inline tuple_type to_tuple(const value_type &arg) const {
        return make_tuple((size_t)arg);
    }
    inline value_type from_tuple(tuple_type tuple) const {
        return tokencount_t(std::get<0>(tuple));
    }
};
template<>
struct sequence_traits<SparseMarking> : sequence_traits<std::vector<tokencount_t>> {
    typedef SparseMarking        				vector_type;
};
template<>
struct sequence_traits<std::set<tokencount_t>> : sequence_traits<std::vector<tokencount_t>> {
    typedef std::set<tokencount_t> 				vector_type;
};


//=============================================================================





typedef table<col::int_col<int>, col::uint_col<size_t>, col::indexed_col<double>> mytable;
template class table<col::int_col<int>, col::uint_col<size_t>, col::indexed_col<double>>;
template class table<col::int_col<float>>;

void f() {
    mytable tab, tab2;

    tab.push_back(1, 0ul, 0.0);
    cout << tab.num_bytes() << endl;
    tab2 = tab;
    tab2.swap(tab);
    tab.set(0, 3, 2ul, 1.0);

    cout << get_at<0>(tab, 0) << " ";
    cout << get_at<1>(tab, 0) << " ";
    cout << get_at<2>(tab, 0) << endl;

    set_at<0>(tab, 0, -10);
    set_at<1>(tab, 0, 15ul);
    set_at<2>(tab, 0, 9.0);

    cout << get_at<0>(tab, 0) << " ";
    cout << get_at<1>(tab, 0) << " ";
    cout << get_at<2>(tab, 0) << endl;

    for (size_t u = 0; u < 20; u++)
        tab.push_back((rand() % 20) - 10, rand() % 60, (rand() % 20) * 0.5);

    tab.push_back(5, 0, 45.2);
    tab.push_back(-1, 17, 3.7);
    mytable::const_iterator it;
    for (it = tab.begin(); it != tab.end(); ++it)
        cout << it.index() << ": " << it.get<0>() << " " << it.get<1>() << " " << it.get<2>() << endl;

    cout << "SIZE: " << tab.num_bytes() << endl;
}




// Declare a specialized template version
//extern template struct sparse_seq_store<SparseMarking>;

// Declare a specialized template version
// extern template struct seq_store<std::vector<tokencount_t>>;


//=============================================================================
// Global instantiation of seq_store<>
//=============================================================================

// Implements the specialized template version
// template struct seq_store<std::vector<tokencount_t>, col::uint_col<tokencount_t>>;
// template struct seq_store<std::vector<tokencount_t>>;



template struct seq_store<std::vector<A>>;
typedef struct seq_store<std::vector<A>> seq_A;

typedef seq_store<std::vector<tokencount_t>> seq_B;
template struct seq_store<std::vector<tokencount_t>>;


template struct sparse_seq_store<sparsevector<size_t, A>>;
typedef struct sparse_seq_store<sparsevector<size_t, A>> sparse_seq_A;

typedef sparse_seq_store<SparseMarking> sparse_seq_B;
template struct sparse_seq_store<SparseMarking>;

typedef seq_store<std::set<tokencount_t>> seq_set_B;
template struct seq_store<std::set<tokencount_t>>;

void f2() {
    seq_A a;
    seq_B b;

    sparse_seq_A a2(10);
    sparse_seq_B b2(15);

    seq_set_B sb;
}


//=============================================================================
// Global instantiation of sparse_seq_store<>
//=============================================================================

// Implements the specialized template version
//template struct sparse_seq_store<SparseMarking>;

//=============================================================================




//template class sgm_table<size_t, 4096>;

//=============================================================================

int iscolon(int ch) { return (ch == ':'); }
int _iscommaorparen(int ch) { return (ch == ',' || ch == ')'); }
int _isnotcommaorparen(int ch) { return !_iscommaorparen(ch); }

void print_sm(const SparseMarking &sm) {
    for (const auto &ivp : sm)
        cout << ivp.value << "*P" << ivp.index << " ";
    cout << endl;
}

#include <bitset>

template<typename SpVec, typename... Args>
void print_seq(const sparse_seq_store<SpVec, Args...> &seq,
               typename sparse_seq_store<SpVec, Args...>::sequence_id sid) {
    typename sparse_seq_store<SpVec, Args...>::const_iterator it = seq.begin_seq(sid);
    while (it != seq.end()) {
        cout << *it << "*P" << it.index() << " ";
        ++it;
    }
}
template<typename SpVec, typename... Args>
void print_seq(const seq_store<SpVec, Args...> &seq,
               typename seq_store<SpVec, Args...>::sequence_id sid) {
    typename seq_store<SpVec, Args...>::const_iterator it = seq.begin_seq(sid);
    size_t i = 0;
    while (it != seq.end()) {
        cout << *it << "*P" << (i++) << " ";
        ++it;
    }
}
void Experiment2(const char *arg) {
    f();
    f2();


    // vector<vector<ssize_t>> values = {
    // 	{ 0, 0, 0, 0, 0 },
    // 	{ 1, 0, 1, 0, 1, 1, 0, 1, 0,0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1,1,1,1 ,0,1,0,1,0,1,0,1 },
    // 	{ 5, 4, 3, 2, 1, 0, 10, 15 },
    // 	{ 5, 4, 3, 2, 1, 0, 10, 25 },
    // 	{ 0, 1, 0, 1, 0 },
    // 	{ 1, 2, 3, 4, 5 },
    // 	{ 0, 1, 2, 16, 50, 300, 1000, 80000 }
    // };
    // sgm_table<double, 16> dtab;
    // for (size_t k = 0; k<values.size(); k++) {
    // 	cout << "\nVECTOR " << k << endl;
    // 	sgm_table<size_t, 16> tab;
    // 	for (ssize_t v : values[k]) {
    // 		tab.push_back(v);
    // 		dtab.push_back(double(v));
    // 	}
    // 	cout << print_vec(tab) << endl;
    // 	// cout << print_vec(dtab) << endl;
    // }
    // cout << "\n\n\n\n" << endl;





    // // Example: TBL_BITS=32, TBL_SHIFT=5, TBL_MASK=31
    // // sz = 20
    // // shift1 = 18:
    // //
    // // /-----shift1-----\
    // //                   ************** ******
    // // 01234567890123456789012345678901 23456789012345678901234567890123
    // // \--------elems[h1ind]----------/ \--------elems[h2ind]----------/
    // //
    // //
    // // shift1 = 3:
    // //
    // // /--\ = shift1
    // //     ********************
    // // 01234567890123456789012345678901
    // // \--elems[h1ind] = elems[h2ind]-/
    // //
    // // CASE 1: Single word storage:
    // //   shift1 = 3, sz = 20:
    // //                                <-- shift1
    // //            ********************
    // //   10987654321098765432109876543210
    // //   \--elems[h1ind] = elems[h2ind]-/
    // //
    // // CASE 2: Double word storage:
    // //   shift1 = 18, sz = 20:
    // //                                                <----------------- shift1
    // //                           ****** **************
    // // 32109876543210987654321098765432 10987654321098765432109876543210
    // // \--------elems[h2ind]----------/ \--------elems[h1ind]----------/
    // //
    // constexpr size_t TBL_BITS = sizeof(size_t) * CHAR_BIT;
    // constexpr size_t TBL_SHIFT = (TBL_BITS == 32 ? 5 : (TBL_BITS == 64 ? 6 : 0));
    // constexpr size_t TBL_MASK = (1ul << TBL_SHIFT) - 1;
    // static_assert((TBL_BITS == 32 && TBL_SHIFT == 5) || (TBL_BITS == 64 && TBL_SHIFT == 6), "Incorrect TBL_BITS/SHIFT");
    // const size_t bpv = 27, N=200;
    // vector<size_t> elems((N * bpv + TBL_BITS - 1) >> TBL_SHIFT);
    // auto get_value = [&](const size_t index) -> size_t {
    // 	cout << console::white_fgnd() << "get_value(" << index << ")" << endl;
    // 	const size_t h1ind = (index * bpv) >> TBL_SHIFT;
    // 	const size_t h2ind = ((index+1) * bpv - 1) >> TBL_SHIFT;
    // 	const size_t shift1 = (index * bpv) & TBL_MASK;
    // 	cout << "  h1ind = " << h1ind << endl;
    // 	cout << "  h2ind = " << h2ind << endl;
    // 	cout << "  shift1 = " << shift1 << endl;

    // 	size_t value = elems[h1ind];
    // 	cout << "  reading " << std::bitset<TBL_BITS>(value) << endl;
    // 	value >>= shift1;
    // 	cout << "  value " << std::bitset<TBL_BITS>(value) << endl;
    // 	if (h2ind != h1ind)
    // 		value |= elems[h2ind] << (TBL_BITS - shift1);
    // 	cout << "  elems[h2ind] << (bpv - (TBL_BITS - shift1)) = " << ((elems[h2ind] << (bpv - (TBL_BITS - shift1))) & ((1ul << bpv) - 1)) <<endl;
    // 	value &= (1ul << bpv) - 1;
    // 	cout << "  returning " << std::bitset<TBL_BITS>(value) << endl;
    // 	cout << "get_value(" << index << ") = " << value << console::default_disp() << endl;
    // 	return value;
    // };
    // auto set_value = [&](const size_t index, const size_t value) {
    // 	assert(value < (1ul << bpv));
    // 	cout << "set_value(" << index << ", " << value << ")" << endl;
    // 	cout << "  setting    " << std::bitset<TBL_BITS>(value) << endl;
    // 	const size_t h1ind = (index * bpv) >> TBL_SHIFT;
    // 	const size_t shift1 = (index * bpv) & TBL_MASK;
    // 	const size_t SZMASK1 = (1ul << bpv) - 1;
    // 	cout << "  h1ind = " << h1ind << endl;
    // 	cout << "  shift1 = " << shift1 << endl;
    // 	cout << "  SZMASK1 = " << SZMASK1 << endl;

    // 	size_t elem1 = elems[h1ind];
    // 	cout << "  reading(1) " << std::bitset<TBL_BITS>(elem1) << endl;
    // 	elem1 &= ~(SZMASK1 << shift1);
    // 	elem1 |= value << shift1;
    // 	cout << "  writing(1) " << std::bitset<TBL_BITS>(elem1) << endl;
    // 	elems[h1ind] = elem1;

    // 	if (bpv + shift1 > TBL_BITS) {
    // 		const size_t h2ind = ((index+1) * bpv - 1) >> TBL_SHIFT;
    // 		const size_t shift2 = (bpv + shift1 - TBL_BITS);
    // 		const size_t SZMASK2 = (1ul << shift2) - 1;
    // 		cout << "  h2ind = " << h2ind << endl;
    // 		cout << "  shift2 = " << shift2 << endl;
    // 		size_t elem2 = elems[h2ind];
    // 		cout << "  reading(2) " << std::bitset<TBL_BITS>(elem2) << endl;
    // 		elem2 &= ~SZMASK2;
    // 		elem2 |= value >> (bpv - shift2);
    // 		cout << "  writing(2) " << std::bitset<TBL_BITS>(elem2) << endl;
    // 		elems[h2ind] = elem2;
    // 	}
    // };
    /*srand(0);
    vector<size_t> v2(N);
    for (size_t i=0; i<N; i++) {
    	size_t val = rand() & ((1ul << bpv) - 1);
    	set_value(i, val);
    	assert(get_value(i) == val);
    	v2[i] = val;
    }
    for (size_t i=0; i<N; i++) {
    	cout << "should read: " << std::bitset<TBL_BITS>(v2[i]) << endl;
    	assert(get_value(i) == v2[i]);
    }
    for (size_t i=0; i<N; i++) {
    	size_t val = rand() & ((1ul << bpv) - 1);
    	set_value(i, val);
    	assert(get_value(i) == val);
    	v2[i] = val;
    }
    for (size_t i=0; i<N; i++) {
    	cout << "should read: " << std::bitset<TBL_BITS>(v2[i]) << endl;
    	assert(get_value(i) == v2[i]);
    }
    cout << "  *** " << console::green_fgnd() << "OK." << console::default_disp() << " ***\n\n" << endl;

    return;*/

    // sparse_seq_store<SparseMarking> spss(10);
    // SparseMarking sm(10);
    // sm.add_element(1, 10);
    // sm.add_element(3, 20);
    // sm.add_element(5, 30);
    // sm.add_element(7, 40);
    // print_sm(sm); cout << endl;
    // sparse_seq_store<SparseMarking>::sequence_id sid = spss.insert(sm);

    // sparse_seq_store<SparseMarking>::const_iterator it = spss.begin_seq(sid);
    // sparse_seq_store<SparseMarking>::const_iterator itE = spss.end();
    // cout << "  ITER: ";
    // for (; it != itE; ++it) {
    // 	cout << *it << "*" << it.index() << " ";
    // }
    // cout << endl;






    // compact_column cs;
    // cs.resize(1, 1);
    // cs.set(0, 0x6FFFFFFFFFFFFFFF);
    // cs.set(0, 0);
    // cs.set(0, 0x7FFFFFFFFFFFFFFF);
    // cs.set(0, 0x0F0F0F0F0F0F0F0F);
    // cs.set(0, 0x70F0F0F0F0F0F0F0);
    // cs.set(0, 1);
    // cout << "compact_column test is ok. " << cs[0] << endl;




    typedef seq_store<std::vector<tokencount_t>> seq_tokencounts_t;
    typedef sparse_seq_store<SparseMarking> sparse_seq_tokencounts_t;

    ifstream ifs(arg);
    simple_tokenizer sta(&ifs);
    vector<string> labels;
    string sep;
    sta.readtok("(");
    do {
        string label;
        sta.gettok(label, _isnotcommaorparen).gettok(sep, _iscommaorparen);
        if (label.size() == 0)
            throw program_exception(sta.pos() + ": Cannot read variable name.");
        labels.emplace_back(label);
    }
    while (sta && sep == ",");

    sparse_seq_tokencounts_t all_seqs(labels.size());
    seq_tokencounts_t all_dseqs;
    size_t numStates = 0;
    while (true) {
        SparseMarking mark(labels.size());
        std::vector<tokencount_t> dmark(labels.size(), tokencount_t(0));
        size_t pos, val;
        sta.read(pos).gettok(sep, iscolon);
        if (sep != ":")
            break;
        sta.readtok("(");
        for (size_t i = 0; i < mark.size(); i++) {
            sta.read(val).readtok(i == size_t(mark.size()) - 1 ? ")" : ",");
            if (val)
                mark.add_element(i, val);
            dmark[i] = tokencount_t(val);
        }
        //print_sm(mark);

        // cout << "INSERTING:  " << print_vec(mark) << endl;
        sparse_seq_tokencounts_t::sequence_id sid = all_seqs.insert(mark);
        seq_tokencounts_t::sequence_id dsid = all_dseqs.insert(dmark);
        if (all_seqs.num_inserted_seq() < 100) {
            // cout<< "  INS.s_next = " << print_vec(all_seqs.tabs[INSERTED].s_next) << endl;
            // cout<< "  INS.s_values = " << print_vec(all_seqs.tabs[INSERTED].s_values) << endl;
            // cout<< "  INS.s_index = " << print_vec(all_seqs.tabs[INSERTED].s_index) << endl;
            // cout<< "  INS.s_left = " << print_vec(all_seqs.tabs[INSERTED].s_left) << endl;
            // cout<< "  INS.s_right = " << print_vec(all_seqs.tabs[INSERTED].s_right) << endl;
            // cout<< "  INS.hm_htab = " << print_vec(all_seqs.tabs[INSERTED].hm_htab) << endl;
            // cout<< "  INS.Empty = " << all_seqs.tabs[INSERTED].EmptyElemId << endl;
            // cout<< "  FRG.s_next = " << print_vec(all_seqs.tabs[FRAGMENTS].s_next) << endl;
            // cout<< "  FRG.s_values = " << print_vec(all_seqs.tabs[FRAGMENTS].s_values) << endl;
            // cout<< "  FRG.s_index = " << print_vec(all_seqs.tabs[FRAGMENTS].s_index) << endl;
            // cout<< "  FRG.s_left = " << print_vec(all_seqs.tabs[FRAGMENTS].s_left) << endl;
            // cout<< "  FRG.s_right = " << print_vec(all_seqs.tabs[FRAGMENTS].s_right) << endl;
            // cout<< "  FRG.hm_htab = " << print_vec(all_seqs.tabs[FRAGMENTS].hm_htab) << endl;
            // cout<< "  FRG.Empty = " << all_seqs.tabs[FRAGMENTS].EmptyElemId << endl;
            cout << "SID: " << size_t(sid) << "  HASH: " << all_seqs.make_hash(mark) << "  MARK: ";
            print_seq(all_seqs, sid);
            cout << endl;
            // print_sm(mark);
        }

        SparseMarking mark2(mark.size());
        // cout << "RETRIEVING SSID: " << sid << endl;
        all_seqs.retrieve(sid, mark2);
        // cout << "  =>  "; print_sm(mark2);
        if (mark != mark2) {
            // cout<< "s_hash = " << print_vec(all_seqs.s_hash) << endl;
            // cout<< "s_values = " << print_vec(all_seqs.s_values) << endl;
            // cout<< "s_index = " << print_vec(all_seqs.s_index) << endl;
            // cout<< "s_left = " << print_vec(all_seqs.s_left) << endl;
            // cout<< "s_right = " << print_vec(all_seqs.s_right) << endl;
            throw program_exception("retrieve(1) is bugged.");
        }

        std::vector<tokencount_t> dmark2;
        all_dseqs.retrieve(dsid, dmark2);
        if (dmark != dmark2) {
            // cout << endl;
            // cout<< "s_next = " << print_vec(all_dseqs.s_next) << endl;
            // cout<< "s_values = " << print_vec(all_dseqs.s_values) << endl;
            // cout<< "s_length = " << print_vec(all_dseqs.s_length) << endl;
            // cout<< "s_left = " << print_vec(all_dseqs.s_left) << endl;
            // cout<< "s_right = " << print_vec(all_dseqs.s_right) << endl;
            // cout << endl;
            cout << "       dsid: " << size_t(dsid) << endl;
            cout << "   inserted: " << print_vec(dmark) << endl;
            cout << "  retrieved: " << print_vec(dmark2) << endl;
            throw program_exception("retrieve(2) is bugged.");
        }

        {
            SparseMarking mark3(mark.size());
            sparse_seq_tokencounts_t::const_iterator it = all_seqs.begin_seq(sid);
            while (it != all_seqs.end()) {
                mark3.add_element(it.index(), *it);
                ++it;
            }
            if (mark != mark3) {
                cout << "numStates: " << numStates << endl;
                print_sm(mark); cout << endl;
                print_sm(mark3); cout << endl;
                //cout << all_seqs.s_index[size_t(sid)] << endl;
                print_seq(all_seqs, sid); cout << endl;
                throw program_exception("iterators(1) are bugged.");
            }
            for (size_t i = 0; i < mark.size(); i++) {
                tokencount_t ithVal = all_seqs.get_at(sid, i);
                if (ithVal != mark[i])
                    throw program_exception("get_at(1) is bugged.");
                //cout << all_seqs.get_at(sid, i) << " ";
            }
        }

        {
            std::vector<tokencount_t> dmark3;
            seq_tokencounts_t::const_iterator it = all_dseqs.begin_seq(dsid);
            while (it != all_dseqs.end()) {
                dmark3.push_back(*it);
                ++it;
            }
            if (dmark != dmark3) {
                cout << "numStates: " << numStates << endl;
                cout << print_vec(dmark) << endl;
                cout << print_vec(dmark3) << endl;
                print_seq(all_dseqs, dsid); cout << endl;
                throw program_exception("iterators(2) are bugged.");
            }

            for (size_t i = 0; i < dmark.size(); i++) {
                tokencount_t ithVal = all_dseqs.get_at(dsid, i);
                if (ithVal != dmark[i])
                    throw program_exception("get_at(2) is bugged.");
                //cout << all_seqs.get_at(sid, i) << " ";
            }
        }
        //cout << endl;

        // cout << "SSID = " << sid << ":  ";
        // all_seqs.print(sid);
        // cout << endl;

        // if (all_seqs.seqs.size() > 1)
        // 	return;
        numStates++;
    }

    using namespace console;

    cout << "ALL " << numStates << " STATES LOADED.\n" << endl;

    cout << underline_on() << green_fgnd() << "SPARSE SEQUENCE STORAGE:\n" << default_disp();
    cout << "SEQS: " << all_seqs.num_inserted_seq() << endl;
    cout << "MEMORY: " << all_seqs.num_bytes() << " BYTES." << endl;
    // cout << "MEMORY OF s_hash: " << all_seqs.s_hash.size() * sizeof(size_t) << " BYTES." << endl;
    all_seqs.print_memory_occupation();
    cout << "\n" << endl;

    cout << underline_on() << green_fgnd() << "DENSE SEQUENCE STORAGE:\n" << default_disp();
    cout << "SEQS: " << all_dseqs.num_inserted_seq() << endl;
    cout << "MEMORY: " << all_dseqs.num_bytes() << " BYTES." << endl;
    all_dseqs.print_memory_occupation();
    cout << endl;

    all_seqs.freeze();
    all_dseqs.freeze();
}





#endif // 0


class A {
public:
	A() {}
	A(const A&) = delete;
	A(A&&) = default;

	A& operator = (const A&) = default;
	A& operator = (A&&) = default;
};

class B {
public:
	B() {}
	B(const B&) = delete;
	B(B&&) = default;

	B& operator = (const B&) = default;
	B& operator = (B&&) = default;

	A inner;
};

class C {
public:
	C() {}
	explicit C(const C&) = default;
	C(C&&) = default;

	C& operator = (const C&) = default;
	C& operator = (C&&) = default;
};

class D : public C {
public:
	D() {}
	explicit D(const D&) = default;
	D(D&&) = default;

	D& operator = (const D&) = default;
	D& operator = (D&&) = default;
};

void f1(A a) { }
void f2(const A& a) { }
void f3(A&& a) { }

void f4(const B& b) { }
void f5(B&& b) { }

void f6(C c) { }
void f7(const C& c) { }
void f8(C&& c) { }

void f9(D d) { }
void f10(const D& d) { }
void f11(D&& d) { }


void Experiment2(const char *arg) {
	A a1;
	// A a2(a1);
	A a3;
	a3 = a1;

	// f1(a1);
	f2(a1);
	f3(std::move(a3));

	B b1;
	B b2;
	b2 = b1;
	f4(b1);
	f5(std::move(b2));

	C c1;
	C c2(c1), /*c3 = c1,*/ c4;
	c4 = c2;

	// f6(c1);
	f7(c2);
	f8(std::move(c1));

	D d1;
	D d2(d1), /*d3 = d1,*/ d4;
	d4 = d2;

	//f9(d1);
	f10(d2);
	f11(std::move(d1));
}











