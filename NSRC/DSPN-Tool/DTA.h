/*
 *  MRP.h
 *
 *  Definitions of the Deterministic Timed Automata (DTA) data structures.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __DETERMINISTIC_TIMED_AUTOMATA_H__
#define __DETERMINISTIC_TIMED_AUTOMATA_H__
//=============================================================================

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct ActionNameStructTag;
struct ClockValueStructTag;
struct AtomicPropStructTag;
struct LocationStructTag;
struct EdgeStructTag;
struct ClockZoneStructTag;
struct ReachMarkStructTag;
typedef private_integer<size_t, ActionNameStructTag>	actname_t;
typedef private_integer<size_t, ClockValueStructTag>	clockval_t;
typedef private_integer<size_t, AtomicPropStructTag>	atomicprop_t;
typedef private_integer<size_t, LocationStructTag>		location_t;
typedef private_integer<size_t, EdgeStructTag>			edge_t;
typedef private_integer<size_t, ClockZoneStructTag>		clockzone_t;
typedef private_integer<size_t, ReachMarkStructTag>		marking_t;
#else
typedef size_t  actname_t;
typedef size_t  clockval_t;
typedef size_t  atomicprop_t;
typedef size_t  location_t;
typedef size_t  edge_t;
typedef size_t  clockzone_t;
typedef size_t  marking_t;
#endif

//=============================================================================

struct rgstate_t;
struct RG2;

// Satisfiability set
struct SAT : public PetriNetObject {
    inline SAT(size_t initN = 0, boost::optional<marking_t> s0 = boost::optional<marking_t>())
    /**/ : sat(initN), state(s0) { }
    inline SAT(const SAT &s) = default;
    inline SAT(SAT &&s) = default;
    virtual ~SAT();

    SAT &operator = (SAT && s) = default;

    inline size_t N() const { return sat.size(); }
    inline bool operator [](size_t i) const { return sat.at(i); }
    inline bool IsSingleInitState() const { return (bool)state; }

    void WriteToFile(ofstream &ofs) const;
    bool ReadFromFile(ifstream &ifs);

    SAT operator !() const;
    SAT operator && (const SAT &s) const;
    SAT operator || (const SAT &s) const;

    // Inherited by PetriNetObject
    virtual void Print(ostream &os, bool expandParamNames) const override;

    std::vector<bool>     		sat;   // Satisfiability of each model state
    ublas::vector<double> 		probs; // Associated probabilities

    boost::optional<marking_t>  state; // Defined if the SAT set is for a single state
};

bool CompareSATs(const SAT &s1, const SAT &s2, double epsilon);

//=============================================================================

// Base class of Atomic proposition expressions for DTA state propositions
struct AtomicPropExpr : public PetriNetObject {
    virtual void Print(ostream &os, bool expandParamNames) const = 0;

    inline bool IsStored() const { return isStored; }
    inline const SAT &GetStoredSAT() const { assert(isStored); return storedSat; }
    inline bool EvaluateInRgState(size_t i) const { assert(isStored); return storedSat[i]; }

    void StoreEvaluation(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl);

    // Evaluate the expression and generate the entire SAT set
    virtual SAT Evaluate(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl) = 0;

protected:
    bool  	isStored = false;
    SAT		storedSat;   // Stored satisfiability set
};

//=============================================================================

// Bound/unbound ptr to an atomic proposition expression
struct PtrToAtomicPropExpr : public AtomicPropExpr {
    PtrToAtomicPropExpr(const char *s,
                        std::shared_ptr<AtomicPropExpr> e = std::shared_ptr<AtomicPropExpr>())
        : name(s), apExpr(e) { }

    virtual void Print(ostream &os, bool expandParamNames) const override;

    inline bool isBound() const { return apExpr != nullptr; }
    inline void bind(std::shared_ptr<AtomicPropExpr> _apExpr) { apExpr = _apExpr; }
    inline std::shared_ptr<AtomicPropExpr> get_expr() const { return apExpr; }
    inline const string &get_name() const { return name; }
protected:
    virtual SAT Evaluate(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl) override;

    string 					 		name;   // Name of this pointer object
    std::shared_ptr<AtomicPropExpr> apExpr; // Pointed APexpr. Can be unbound.
};

//=============================================================================

struct DTA;

/// Type of action set
enum ActSetType {
    AT_ACCEPTED_ACTIONS,		// { a, b, c, ... }
    AT_REJECTED_ACTIONS,		// Act \ { a, b, c, ... }
    AT_BOUNDARY					// #
};

const transition_t UNBOUNDED_TRANSITION = transition_t(size_t(-1));
const double UNBOUNDED_CLOCK_VALUE = -1.0;

/// Type of clock zone
enum ClockZoneType {
    CZT_TIMED,			// (alpha, beta)
    CZT_ENTER_IMMED,    // [alpha], entering
    CZT_EXIT_IMMED,		// [alpha], exiting
    CZT_INVALID
};

/// Type of Final DTA location
enum FinalLocationType {
    FLT_NOT_FINAL,			// Normal non-final location
    FLT_FINAL_ACCEPTING,	// Final, accepts the path
    FLT_FINAL_REJECTING		// Final, rejects the path
};

//=============================================================================
// Bounded values (action names, clock values, state expressions)
//=============================================================================

struct ActionName {
    ActionName() : index(actname_t(size_t(-1))), boundedTrn(UNBOUNDED_TRANSITION) { }
    ActionName(actname_t i, const char *s)
        : index(i), name(s), boundedTrn(UNBOUNDED_TRANSITION) { }

    actname_t		index;		// Index in the DTA vector acts
    string 			name;		// Action label
    transition_t 	boundedTrn; // Bounded PN transition
};

struct ClockValue {
    ClockValue() : index(clockval_t(size_t(-1))) { }
    ClockValue(clockval_t i, const char *s) : index(i), name(s) { }

    clockval_t 		           index;		 // Index in the DTA vector clkvals
    string  		           name;		 // Clock value label
    shared_ptr<Expr<double> >  boundedValue; // Bounded real value

    bool isBounded() const { return boundedValue != nullptr; }
};

extern const char *CLOCK_VALUE_NAME_0;			// Name of the default clock value 0
extern const char *CLOCK_VALUE_NAME_INFINITY;	// Name of the default clock value INF

template<typename T> struct PtrToMdepExpr;

struct AtomicProp {
    AtomicProp() : index(atomicprop_t(size_t(-1))) { }
    AtomicProp(atomicprop_t i, const char *s);

    atomicprop_t	                 index;       // Index in the DTA vector aps
    shared_ptr<PtrToAtomicPropExpr>  boundedExpr; // Bounded PN proposition
};

// represents a pair of clock values that have to be in order.
struct ClockValueConstr {
    ClockValueConstr() : smaller(clockval_t(size_t(-1))), bigger(clockval_t(size_t(-1))) { }
    ClockValueConstr(clockval_t sm, clockval_t bg) : smaller(sm), bigger(bg) { }

    clockval_t		smaller, bigger;
};

// clock zone descriptor, initialized after a DTA binding.
struct ClockZone {
    ClockZone(clockzone_t i, clockval_t _cv1 = clockval_t(size_t(-1)),
              clockval_t _cv2 = clockval_t(size_t(-1)),
              double _v1 = -1, double _v2 = -1)
        : index(i), cv1(_cv1), cv2(_cv2), v1(_v1), v2(_v2) { }

    inline double duration() const { return v2 - v1; }
    inline bool isInfinite() const { return std::isinf(v2); }

    clockzone_t index;      // Index of this clock zone
    clockval_t 	cv1, cv2;	// Clock value indexes
    double 		v1,  v2;	// Actual precomputed values
};

//=============================================================================
// Set of accepted actions
//=============================================================================

struct ActSet {
    ActSet() : type(AT_ACCEPTED_ACTIONS) { }
    ActSet(ActSetType at) : type(at) { }

    ActSetType  	        type;    // The set @acts list accepting/rejecting actions
    std::vector<actname_t>  actInds; // List of actions
};

//=============================================================================
// DTA Edge (Inner=reads CTMC actions, Boundary=follows the clock)
//=============================================================================

struct DtaEdge : public ParserObj {
    DtaEdge() : reset(false), alpha(clockval_t(size_t(-1))),
/**/	    beta(clockval_t(size_t(-1))), destLoc(location_t(size_t(-1))) { }
    DtaEdge(const ActSet &_acts, bool r, clockval_t a, clockval_t b, location_t dst)
        : actset(_acts), reset(r), alpha(a), beta(b), destLoc(dst) { }

    ActSet   	actset;       // Accepted CTMC actions
    bool     	reset;        // Reset of the clock
    clockval_t  alpha, beta;  // Clock zone (alpha < x < beta or x=alpha)
    location_t  destLoc;	  // Destination location index

    inline bool isInner() const      { return actset.type != AT_BOUNDARY; }
    inline bool isBoundary() const   { return actset.type == AT_BOUNDARY; }
    inline double alphaValue(const DTA &) const;
    inline double betaValue(const DTA &) const;
};

//=============================================================================
// DTA Location
//=============================================================================

struct Location {
    Location(location_t ind, bool _init, FinalLocationType _flt, const char *l,
             const shared_ptr<AtomicPropExpr> &apex)
        : index(ind), isInitial(_init), finalType(_flt), label(l), spExpr(apex),
          prevIndex(location_t(size_t(-1))), zone(clockzone_t(size_t(-1))), zoneType(CZT_INVALID) { }

    location_t 					  index;	  // Index in the DTA vector locs
    bool						  isInitial;  // Initial location flag
    FinalLocationType			  finalType;  // Is final location? (accepting/rejecting)
    string						  label;      // Name of this location
    shared_ptr<AtomicPropExpr>    spExpr;     // State proposition expression with atomic props
    std::vector<DtaEdge>	      innerEdges; // Outgoing inner edges
    std::vector<DtaEdge>     	  boundEdges; // Outgoing boundary edges

    // Filed used for zoned DTAs
    location_t                    prevIndex;  // Index in the un-zoned DTA
    clockzone_t  				  zone;       // Zone of this location
    ClockZoneType 				  zoneType;   // Which part of the zone (timed/immed)

    inline bool isFinal() const   {  return finalType != FLT_NOT_FINAL;  }
    inline bool isTimed() const   {  assert(zoneType != CZT_INVALID); return zoneType == CZT_TIMED;  }
};

//=============================================================================
// DTA
//=============================================================================

struct DTA : boost::noncopyable {
    DTA();

    string 					      name;    // name of this DTA
    std::vector<ActionName>       acts;    // Set of actions (bounded to transitions)
    std::vector<ClockValue>       clkvals; // Set of clock values (bounded to reals)
    std::vector<ClockValueConstr> cvc;     // Constraints in the clock values
    std::vector<AtomicProp>       aps;     // Set of atomic proposition names
    std::vector<Location>         locs;    // Set of locations
    bool 					      isBound; // is DTA instantiated?
    bool                          hasClockZones; // is zones[] initialized?
    bool                          isZoned; // Are Locations split into separate zones?

    // Fields available after the binding with a PN
    std::vector<ClockZone>        zones;   // List of timed clock zones (after binding)

    inline clockzone_t c0() const    {  assert(hasClockZones); return clockzone_t(0);  }
    inline clockzone_t cInf() const  {  assert(hasClockZones); return clockzone_t(zones.size() - 1);  }
};

void LoadDTAFromFile(istream &is, DTA &dta, VerboseLevel verboseLvl);

void SaveDTA_AsDotFile(const DTA &dta, ofstream &dot);

void MakeZonedDTA(const DTA &dta, DTA &zdta, VerboseLevel verboseLvl);

//=============================================================================

// Binds a symbolic action name to a named PN transition
struct BoundAction {
    BoundAction(const string &n, transition_t i) : trnName(n), trnIndex(i) { }

    string 		  trnName;  // Name of the bounded PN transition
    transition_t  trnIndex; // Index of the transition in the PN
};

typedef std::vector<std::pair<std::string, std::shared_ptr<Expr<double>>>> clock_value_bindings;
typedef std::vector<std::pair<std::string, BoundAction>> action_bindings;
typedef std::vector<std::pair<std::string, std::shared_ptr<AtomicPropExpr>>> ap_expr_bindings;

// DTA Binding
struct DtaBinding : public PetriNetObject {
    clock_value_bindings	clock_values;
    action_bindings			trn_actions;
    ap_expr_bindings  		ap_exprs;

    virtual void Print(ostream &os, bool expandParamNames) const override;
};

bool BindDTAToPN(DTA &dta, const PN *pPn, const string &rules, VerboseLevel verboseLvl);

bool BindDTA(DTA &dta, const shared_ptr<DtaBinding> bindings, VerboseLevel verboseLvl);

//=============================================================================

bool IsAtomicPropositionId(const char *name, const DTA *pDTA);
bool IsActionId(const char *name, const DTA *pDTA);
bool IsClockValueId(const char *name, const DTA *pDTA);

shared_ptr<AtomicPropExpr> FindAtomicProposition(const char *apName, const DTA *pDTA);
actname_t FindActionInd(const char *actName, const DTA *pDTA);
clockval_t FindClockValue(const char *cvName, const DTA *pDTA);

//=============================================================================

// Atomic proposition expression from a MdepExpr<bool>
struct AtomicPropExpr_MdepExpr : public AtomicPropExpr {
    AtomicPropExpr_MdepExpr(shared_ptr<MdepExpr<bool>> e) : expr(e) { }

    virtual void Print(ostream &os, bool expandParamNames) const override;
protected:
    virtual SAT Evaluate(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl) override;
    shared_ptr<MdepExpr<bool>>  expr;
};

//=============================================================================

// Atomic proposition TRUE expression
struct AtomicPropExpr_Const : public AtomicPropExpr {
    AtomicPropExpr_Const(bool cval) : value(cval) { }

    virtual void Print(ostream &os, bool expandParamNames) const override;
protected:
    virtual SAT Evaluate(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl) override;
    bool value;
};

//=============================================================================

// Atomic proposition NOT expression
struct AtomicPropExpr_NOT : public AtomicPropExpr {
    AtomicPropExpr_NOT(shared_ptr<AtomicPropExpr> e) : expr(e) { }

    virtual void Print(ostream &os, bool expandParamNames) const override;
protected:
    virtual SAT Evaluate(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl) override;
    shared_ptr<AtomicPropExpr>  expr;
};

//=============================================================================

// Atomic proposition AND/OR binary expressions
struct AtomicPropExpr_BOP : public AtomicPropExpr {
    AtomicPropExpr_BOP(shared_ptr<AtomicPropExpr> e1, shared_ptr<AtomicPropExpr> e2,
                       BinaryBoolFunct _op) : expr1(e1), expr2(e2), op(_op) { }

    virtual void Print(ostream &os, bool expandParamNames) const override;

protected:
    virtual SAT Evaluate(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl) override;
    shared_ptr<AtomicPropExpr>  expr1, expr2;
    BinaryBoolFunct op;
};

//=============================================================================
// Inline implementations:
//=============================================================================

inline double DtaEdge::alphaValue(const DTA &dta) const {
    return dta.clkvals[alpha].boundedValue->Evaluate();
}

inline double DtaEdge::betaValue(const DTA &dta) const  {
    return dta.clkvals[beta].boundedValue->Evaluate();
}

//=============================================================================
#endif   // __DETERMINISTIC_TIMED_AUTOMATA_H__