/*
 *  PetriNet.h
 *
 *  Petri Net Structure Definitions
 *
 *  Created by Elvio Amparore
 *
 */

//=============================================================================
#ifndef __PETRI_NET_H__
#define __PETRI_NET_H__
//=============================================================================

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct MarkParStructTag;
struct PlaceStructTag;
struct RateParStructTag;
struct TransitionStructTag;
struct TokenStructTag;
struct PriorityStructTag;
typedef private_integer<size_t, MarkParStructTag>		markpar_t;
typedef private_integer<size_t, PlaceStructTag>			place_t;
typedef private_integer<size_t, RateParStructTag>		ratepar_t;
typedef private_integer<size_t, TransitionStructTag>	transition_t;
typedef private_integer<int, TokenStructTag>			tokencount_t;
typedef private_integer<size_t, PriorityStructTag>		priority_t;
#else
typedef size_t  markpar_t;
typedef size_t  place_t;
typedef size_t  ratepar_t;
typedef size_t  transition_t;
typedef int     tokencount_t;
typedef size_t  priority_t;
#endif

//=============================================================================

struct sparse_marking_tag {
    typedef place_t index_type;
    typedef tokencount_t value_type;
    typedef base_index_value_pair<index_type, value_type>  index_value_pair;
    static inline bool allow_zeros() { return false; }
};

// Sparse Marking: an assignment of tokens to the net places
typedef sparsevector<sparse_marking_tag>  SparseMarking;

//=============================================================================
//   Forward Declarations
//=============================================================================

struct MarkPar;
struct Place;
struct RatePar;
struct Arc;
struct MarkDepDelayFn;
struct Transition;
struct PN;
struct PETLT;
template<typename T> struct Expr;
template<typename T> struct MdepExpr;
template<typename T> struct ParamExpr;
struct MeasureExpr;
struct DTA;

//=============================================================================
//     Petri Net Definitions
//=============================================================================

// Marking Parameter
struct MarkPar {
    markpar_t			         index;  // Marking parameter index in the Petri Net
    shared_ptr<ParamExpr<int> >  param;  // Parameter value
};

//-----------------------------------------------------------------------------
// Place of a Petri Net
struct Place {
    place_t		           index;	   // Place index in the Petri Net
    string		           name;	   // Textual name
    shared_ptr<Expr<int> > initMarkFn; // Initial Marking value function

    inline tokencount_t getInitTokenCount() const;
};

//-----------------------------------------------------------------------------
// Rate Parameter
struct RatePar {
    ratepar_t			            index;  // Rate parameter index in the Petri Net
    shared_ptr<ParamExpr<double> >  param;  // Parameter value
};

//-----------------------------------------------------------------------------
// Kinds of transition arcs
enum ArcKind { IA, OA, HA };

// Transition arc kind names
extern const char *ArcKind_names[];

//-----------------------------------------------------------------------------
// Arc of a Transition
struct Arc {
    place_t		                plc;	// Connected place index
    shared_ptr<MdepExpr<int> >  multFn; // Multiplicity function

    inline tokencount_t multiplicity(const PN &pn, const SparseMarking &mark) const;
    inline bool isMultMarkingDep() const;
    inline tokencount_t getConstantMult() const;
    inline string getArcMultRepr(const PN &pn) const;
};

//-----------------------------------------------------------------------------

// Transition timed/untimed names ("TIMED", "IMMED")
extern const char *TrnTimed_names[2];

// Timed transition distribution
enum TrnDistrib { IMM, EXP, DET, NumTrnDistribs };

// Transition distribution names
extern const char *TrnDistrib_names[];
extern const char *TrnDistrib_names_lowercase[];

//-----------------------------------------------------------------------------
// Transition of a Petri Net
struct Transition {
    transition_t  index;   // Transition index in the Petri Net
    string		  name;	   // Textual name
    TrnDistrib	  distrib; // Distribution (untimed, exponential, ...)
    priority_t	  prio;	   // Priority (0 for timed transitions)
    vector<Arc>	  arcs[3]; // Arc sets
    shared_ptr<MarkDepDelayFn>  delayFn;  // delay/weight function
    shared_ptr<MdepExpr<bool>>  guardFn;  // additional guard

    inline bool isImmediate() const  {  return (distrib == IMM);  }
    inline bool isTimed() const  	 {  return (distrib != IMM);  }
    inline bool isGeneral() const    {  return (distrib != IMM && distrib != EXP);  }

    inline double delay(const PN &pn, const SparseMarking &mark) const;
    inline double weight(const PN &pn, const SparseMarking &mark) const;
    string generalPdf() const;
};

//-----------------------------------------------------------------------------
// Measures for place reward
struct Measure {
    string					name;
    shared_ptr<MeasureExpr>	expr;
};

//-----------------------------------------------------------------------------
// Petri Net definitions
struct PN : boost::noncopyable {
    vector<MarkPar>			  mpars;	  // Marking Parameters
    map<string, markpar_t>	  mparInds;	  // Associates marking params with indexes

    vector<Place>			  plcs;		  // Place set
    map<string, place_t>	  plcInds;	  // Associates place names with indexes

    vector<RatePar>			  rpars;	  // Rate Parameters
    map<string, ratepar_t>	  rparInds;	  // Associates rate params with indexes

    vector<Transition>		  trns;		  // Transition set
    map<string, transition_t> trnInds;	  // Associates transition names with indexes
    vector<transition_t>	  detTrnInds; // Deterministic transition indexes

    vector<Measure>			  measures;   // Measures that will be computed
};

//-----------------------------------------------------------------------------
// Possibly Enabled Transitions Look-up Table
struct PETLT {
    typedef multimap<place_t, transition_t>  TrnsByPlcTbl_t;
    // Transitions possibly enabled if a specific place has at least a token
    TrnsByPlcTbl_t        placePE[NumTrnDistribs];
    // Transitions for which nothing can be said, and thus are always
    // possibly enabled in every marking (requiring the normal enabling test)
    vector<transition_t>  alwaysPE[NumTrnDistribs];
};


//=============================================================================
//     Function Prototypes
//=============================================================================

shared_ptr<MdepExpr<double> > ParseMarkDepDelayExpr(const PN &pn, const char *exprStr,
        VerboseLevel verboseLvl);

void ReadGreatSPN_File(ifstream &net, ifstream &def, PN &pn, 
                       bool readAsPT, // ignore rates/delays
                       VerboseLevel verboseLvl);

void BuildPossiblyEnabledTransitionLookupTable(const PN &pn, PETLT &petlt,
        VerboseLevel verboseLvl);

size_t TransitionEnablingDegree(const PN &pn, const Transition &trn,
                                const SparseMarking &mark);

bool HasTransitionConcessionInMarking(const PN &pn, const transition_t trnInd,
                                      const SparseMarking &marking);

void FireTransition(const PN &pn, const transition_t &trnInd,
                    const SparseMarking &oldMarking, SparseMarking &newMarking);

int ListEnabledTransitionsInMarking(const PN &pn, const PETLT &petlt,
                                    const SparseMarking &marking,
                                    TrnDistrib tbl, set<transition_t> &enabled,
                                    double *pTotalImmWeight);

bool ChangeMarkPar(PN &pn, const string &mpName, int newVal);

bool ChangeRatePar(PN &pn, const string &rpName, double newVal);

bool ValidateMarkRatePars(const PN& pn);

void PrintParameters(const PN &pn);

const Measure &AddNewMeasure(PN &pn, const char *name, const char *expr,
                             VerboseLevel verboseLvl);

transition_t
LoadMRMC_ModelAsPN(simple_tokenizer &tra, simple_tokenizer &lab,
                   PN &pn, VerboseLevel verboseLvl);

transition_t
LoadPrismModelAsPN(simple_tokenizer &tra, simple_tokenizer &lab,
                   simple_tokenizer &sta, PN &pn, VerboseLevel verboseLvl);

//=============================================================================

// Print a Sparse Marking object
std::function<ostream& (ostream &)>
print_marking(const PN &pn, const SparseMarking &sm);

std::function<ostream& (ostream &)>
print_place(const PN &pn, place_t plc);

std::function<ostream& (ostream &)>
print_transition(const PN &pn, transition_t trn);

//=============================================================================
//   Base class of all the parsed objects
//=============================================================================

class ParserObj {
public:
    inline ParserObj()  { }
    virtual ~ParserObj()  { }
};

// Controlled entry point of the lexer/parser interface
shared_ptr<ParserObj> ParseObjectExpr(const PN *pn, const DTA *dta,
                                      const char *exprStr,
                                      const int initialToken, const char *whatIs,
                                      VerboseLevel verboseLvl);

//=============================================================================
//   Base Class of Parsed Language objects
//=============================================================================

struct PetriNetObject : public ParserObj {
    inline PetriNetObject() { }
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const = 0;
};

std::function<ostream& (ostream &)>
print_petrinetobj(const PetriNetObject &pnobj, bool expandParamNames = false);

std::function<ostream& (ostream &)>
print_petrinetobj(const shared_ptr<PetriNetObject> pnobj, bool expandParamNames = false);


//=============================================================================
//   Expression with constants and symbolic parameters
//=============================================================================

template<class T>
struct Expr : public PetriNetObject {
    typedef T result_t;
    // Get the actual value of the expression
    virtual result_t Evaluate() const = 0;
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const = 0;
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const = 0;
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const = 0;
};

//=============================================================================
//   Marking Dependent expressions
//=============================================================================

template<class T>
struct MdepExpr : public Expr<T> {
    typedef T result_t;
    // Evaluate in a given marking
    virtual result_t EvaluateInMarking(const SparseMarking &marking) const = 0;
    // Tests if this object is really marking dependent
    virtual bool IsMarkingDep() const = 0;
    // Get the marking independent value (if any)
    virtual result_t Evaluate() const = 0;
    // Print this object
    virtual void Print(ostream &os, bool expandParamNames) const = 0;
    // Converts this object in the SPNica/Cosmos formats
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const = 0;
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn) const = 0;
};

//=============================================================================
//   A Parameter with a name
//=============================================================================

template<class T>
struct ParamExpr : public Expr<T> {
    typedef T result_t;
    virtual const string &GetName() const = 0;
    virtual void SetValue(shared_ptr<Expr<T> > newvalue) = 0;
    virtual const shared_ptr<Expr<T> > GetValue() const = 0;
};

//=============================================================================
//     Marking Dependent delay/weight function objects that may require
//       the computation of the enabling degree of a transition
//=============================================================================

/// Base class of the marking dependent delay function objects
struct MarkDepDelayFn : public PetriNetObject {
    // Tells if this delay function depends on the marking
    virtual bool IsMarkingDep() const = 0;
    // Return the constant delay value (if marking independent)
    virtual double EvaluateConstantDelay() const = 0;
    // Get the delay value in the specified marking
    virtual double EvaluateDelay(const PN &pn, const SparseMarking &mark,
                                 const Transition &trn) const = 0;
    // Print a delay function value for the specified marking (without
    // expanding marking parameter names).
    virtual void PrintInMarking(ostream &os, const PN &pn,
                                const SparseMarking &mark,
                                const Transition &trn) const = 0;
    // Tells if the delay function is a probability distribution function
    virtual bool IsPDF() const = 0;
    // Converts the function in the SPNica format
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const = 0;
    virtual void PrintInCosmosFormat(ostream &os, const PN &pn,
                                     const Transition &trn) const = 0;
};

// Special non-valid delay value to identify general events.
const double GENERAL_DELAY = -750.0;

//=============================================================================
//   Petri Net language unary and binary operators and functions
//=============================================================================

enum CompareOp {
    CMPOP_EQ,
    CMPOP_NOT_EQ,
    CMPOP_LESS,
    CMPOP_LESS_EQ,
    CMPOP_GREATER,
    CMPOP_GREATER_EQ
};
extern const char *CompareOp_Names[];
extern const char *CompareOp_SPNicaNames[];
extern const char *CompareOp_CosmosNames[];

enum UnaryFunct {
    UF_NEG,	UF_ABS,	UF_SIN, UF_COS, UF_TAN,
    UF_ARCSIN, UF_ARCCOS, UF_ARCTAN, UF_EXP,
    UF_LOG, UF_SQRT, UF_CEIL, UF_FLOOR, UF_FACTORIAL,
    UF_DIRACDELTA,
    UF_NumFuncts
};
extern const char *UnaryFunct_Names[];
extern const bool  UnaryFunct_isPrefix[];
extern const char *UnaryFunct_SPNicaNames[];
extern const char *UnaryFunct_CosmosNames[];

enum BinaryFunct {
    BF_PLUS, BF_MINUS, BF_MULT, BF_DIV,
    BF_MIN, BF_MAX, BF_MOD, BF_POW, BF_LOG, BF_BINOMIAL, BF_FRACT,
    BF_RECT, BF_UNIFORM, BF_TRIANGULAR, BF_ERLANG, BF_TRUNCATED_EXP, BF_PARETO,
    BF_NumFuncts
};
extern const char *BinaryFunct_Names[];
extern const bool  BinaryFunct_isPrefix[];
extern const char *BinaryFunct_SPNicaNames[];
extern const char *BinaryFunct_CosmosNames[];

enum BinaryBoolFunct {
    BBF_AND, BBF_OR
};
extern const char *BinaryBoolFunct_Names[];
extern const char *BinaryBoolFunct_SPNicaNames[];
extern const char *BinaryBoolFunct_CosmosNames[];

enum UnaryBoolFunct {
    UBF_NOT
};
extern const char *UnaryBoolFunct_Names[];
extern const char *UnaryBoolFunct_SPNicaNames[];
extern const char *UnaryBoolFunct_CosmosNames[];


//=============================================================================

shared_ptr<MarkDepDelayFn> NewMultipleServerDelayFn(size_t numSrv, const shared_ptr<Expr<double> > &delay);
shared_ptr<MarkDepDelayFn> NewLoadDependentDelayFn(const vector<double> &delays);
shared_ptr<MarkDepDelayFn> NewMarkingDependentDelayFn(const shared_ptr<MdepExpr<double> > &mdepFn);
shared_ptr<MarkDepDelayFn> NewGeneralPdfFn(const shared_ptr<Expr<double>> &fg, const char* _expr);

//=============================================================================









//=============================================================================

shared_ptr<Expr<int> > FindMarkPar(const char *mparName, const PN *pn);
shared_ptr<Expr<double> > FindRatePar(const char *rparName, const PN *pn);
place_t FindPlace(const char *placeName, const PN *pn);
transition_t FindTransition(const char *trnName, const PN *pn);

bool IsGMarkPar(const char *name, const struct PN *pPetriNet);
bool IsGRatePar(const char *name, const struct PN *pPetriNet);
bool IsGPlaceId(const char *name, const struct PN *pPetriNet);
bool IsGTransitionId(const char *name, const struct PN *pPetriNet);

//=============================================================================




//=============================================================================
//     Inline methods
//=============================================================================

inline tokencount_t Place::getInitTokenCount() const {
    int tc = initMarkFn->Evaluate();
    if (tc < 0)
        throw program_exception("Negative initial token count");
    return tokencount_t(size_t(tc));
}

//=============================================================================

// Marking-Dependent Transition delay/weight value
inline double Transition::delay(const PN &pn, const SparseMarking &mark) const {
    if (delayFn->IsPDF())
        return GENERAL_DELAY;
    double d = delayFn->EvaluateDelay(pn, mark, *this);
    if (d <= 0.0) {
        ostringstream err;
        err << "Transition " << name << " has a non-positive delay value of ";
        err << d << " in marking " << print_marking(pn, mark);
        throw program_exception(err.str());
    }
    return d;
}

inline double Transition::weight(const PN &pn, const SparseMarking &mark) const {
    double d = delayFn->EvaluateDelay(pn, mark, *this);
    if (d <= 0.0) {
        ostringstream err;
        err << "Transition " << name << " has a non-positive weight value of ";
        err << d << " in marking " << print_marking(pn, mark);
        throw program_exception(err.str());
    }
    return d;
}

//=============================================================================

inline tokencount_t Arc::multiplicity(const PN &pn, const SparseMarking &mark) const {
    int m = multFn->EvaluateInMarking(mark);
    if (m < 0) {
        ostringstream msg;
        msg << "An arc multiplicity of " << m << " is not valid";
        throw program_exception(msg.str());
    }
    return tokencount_t(size_t(m));
}

inline bool Arc::isMultMarkingDep() const {
    return multFn->IsMarkingDep();
}

inline tokencount_t Arc::getConstantMult() const {
    assert(!isMultMarkingDep());
    return multFn->Evaluate();
}

// TODO: rimuovere??
inline string Arc::getArcMultRepr(const PN &pn) const {
    ostringstream s;
    multFn->Print(s, true);
    return s.str();
}

//=============================================================================
#endif    // __PETRI_NET_H__
