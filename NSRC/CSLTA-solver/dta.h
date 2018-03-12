/*
 *  dta.h
 *  CslTA-Solver
 *
 *  Definitions of the Deterministic Timed Automata
 *
 */

//-----------------------------------------------------------------------------
#ifndef __DTA_H__
#define __DTA_H__
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

// Clock Constraint evaluation function
enum GuardFunction {
    GF_LESS,
    GF_GREATER,
    GF_BETWEEN,
    GF_EQUAL
};
typedef enum GuardFunction  GuardFunction;

// DTA locations flags
enum DtaLocFlags {
    DLF_INITIAL = 1,
    DLF_FINAL   = 2,
};
typedef enum DtaLocFlags  DtaLocFlags;

// Type of an Action Set
enum ActionSetType {
    AST_INCLUDED_ACTs,
    AST_EXCLUDED_ACTs,
    AST_BOUNDARY
};
typedef enum ActionSetType  ActionSetType;

// Location labelling state (for every State Proposition)
enum SpLocationState {
    SPLS_ALWAYS,
    SPLS_NEVER,
    SPLS_UNRESTRICTED
};
typedef enum SpLocationState  SpLocationState;

// Kind of DTA Determinism
enum DtaDeterminismKind {
    DDK_NOT_YET_VERIFIED,
    DDK_NEVER_DET,	// Never deterministic
    DDK_SEMI_DET,	// Determinism cannot be verified in parametric form, but
    //   only when the DTA follows a specific path of a
    //   corresponding ASMC
    DDK_ALWAYS_DET	// Always deterministic, for every valid instantiation
};
typedef enum DtaDeterminismKind  DtaDeterminismKind;



//-----------------------------------------------------------------------------
// DTA Declaration API:
//-----------------------------------------------------------------------------
PUBLIC_API void         DtaLocLabel_Delete(DtaLocLabel *dll);
PUBLIC_API DtaLocLabel *DtaLocLabel_NewTrue();
PUBLIC_API DtaLocLabel *DtaLocLabel_NewSpExpr(const char *sp);
PUBLIC_API DtaLocLabel *DtaLocLabel_NewNotExpr(DtaLocLabel *dll);
PUBLIC_API DtaLocLabel *DtaLocLabel_NewAndExpr(DtaLocLabel *d1, DtaLocLabel *d2);
PUBLIC_API DtaLocLabel *DtaLocLabel_NewOrExpr(DtaLocLabel *d1, DtaLocLabel *d2);
PUBLIC_API DtaLocLabel *DtaLocLabel_NewImplyExpr(DtaLocLabel *d1, DtaLocLabel *d2);

PUBLIC_API void    DtaLoc_Delete(DtaLoc *dl);
PUBLIC_API DtaLoc *DtaLoc_New(DtaLocFlags fl, const char *locName, DtaLocLabel *ll);

PUBLIC_API void      DtaGuard_Delete(DtaGuard *dcc);
PUBLIC_API DtaGuard *DtaGuard_NewA(GuardFunction f, const char *a);
PUBLIC_API DtaGuard *DtaGuard_NewAB(GuardFunction f, const char *a,
                                    const char *b);

PUBLIC_API void       DtaActSet_Delete(DtaActSet *das);
PUBLIC_API DtaActSet *DtaActSet_NewEmpty();
PUBLIC_API DtaActSet *DtaActSet_NewID(const char *actID);
PUBLIC_API DtaActSet *DtaActSet_AddAction(DtaActSet *das, const char *actID);
PUBLIC_API DtaActSet *DtaActSet_SetType(DtaActSet *das, ActionSetType ty);

PUBLIC_API void     DtaEdge_Delete(DtaEdge *de);
PUBLIC_API DtaEdge *DtaEdge_New(const char *from, const char *to,
                                DtaGuard *dcc, DtaActSet *das, BOOL reset);

PUBLIC_API void         DtaCCNamOrd_Delete(DtaCCNamOrd *dtaccno);
PUBLIC_API DtaCCNamOrd *DtaCCNamOrd_New();
PUBLIC_API DtaCCNamOrd *DtaCCNamOrd_AddSymbols(DtaCCNamOrd *dtaccno, IDList *idl,
        BOOL sym_are_ordered);
PUBLIC_API DtaCCNamOrd *DtaCCNamOrd_AddPartialOrder(DtaCCNamOrd *dtaccno, IDList *idl);

PUBLIC_API void	Dta_Delete(DTA *dta);
PUBLIC_API DTA *Dta_New();
PUBLIC_API DTA *Dta_AddLocation(DTA *dta, DtaLoc *loc);
PUBLIC_API DTA *Dta_AddEdge(DTA *dta, DtaEdge *edge);


//-----------------------------------------------------------------------------
#ifdef __cplusplus


//-----------------------------------------------------------------------------
// DTA Helper structures
//-----------------------------------------------------------------------------

struct DtaLoc {
    DtaLoc(DtaLocFlags f, const char *n, DtaLocLabel *l) :
/**/		flags(f), locName(n), label(l) {}
    DtaLocFlags flags;
    string locName;
    std::shared_ptr<DtaLocLabel> label;
};
struct DtaGuard {
    DtaGuard(const char *a) : funct(GF_EQUAL), alpha(a), beta(a) {}
    DtaGuard(GuardFunction f, const char *a, const char *b) :
/**/	funct(f), alpha(a), beta(b) {}
    GuardFunction	funct;
    string			alpha, beta;
};
struct DtaActSet {
    ActionSetType type;
    vector<string> actions;
};
struct DtaEdge {
    DtaEdge(const char *f, const char *t, DtaGuard *c, DtaActSet *s,
            bool r) : from(f), to(t), dcc(c), das(s), reset(r) {}
    ~DtaEdge() { safe_delete(dcc); safe_delete(das);  }
    string from, to;
    DtaGuard *dcc;
    DtaActSet *das;
    bool reset;
};
struct DtaCCNamOrd {
    list<string>					CCNames;
    vector< pair<string, string> >  PartialOrds;
};
struct DtaStatePropValues {
    vector< std::pair< string, vector<bool>* > > SPValues;
    ASMC *pAsmc;
};

//-----------------------------------------------------------------------------
// DTA Location Label: boolean expression over State Propositions
//-----------------------------------------------------------------------------
struct DtaLocLabel {
    virtual ~DtaLocLabel() { }

    // Evaluate the Bool expression given State Propositon values
    virtual bool Evaluate(const vector<bool> &SPVals) const = 0;
    // Print the expression in a readable form
    virtual void PrintExpr(ostream &os, const SymbolTable &SPs) const = 0;
    // List the SP used in this expression
    virtual void GatherSPsUsed(set<StatePropID> &spUsed) const = 0;
};

//------ True Expression ----------------------------------
struct DtaLocLabel_True : public DtaLocLabel {
    bool Evaluate(const vector<bool> &SPVals) const;
    void PrintExpr(ostream &os, const SymbolTable &SPs) const;
    void GatherSPsUsed(set<StatePropID> &spUsed) const;
};

//------ State Proposition Expression ---------------------
struct DtaLocLabel_SP : public DtaLocLabel {
    DtaLocLabel_SP(StatePropID _spID) : spID(_spID) {}
    bool Evaluate(const vector<bool> &SPVals) const;
    void PrintExpr(ostream &os, const SymbolTable &SPs) const;
    void GatherSPsUsed(set<StatePropID> &spUsed) const;

    StatePropID spID;
};

//------ Not Expression -----------------------------------
struct DtaLocLabel_Not : public DtaLocLabel {
    DtaLocLabel_Not(DtaLocLabel *e) : expr(e) {}
    bool Evaluate(const vector<bool> &SPVals) const;
    void PrintExpr(ostream &os, const SymbolTable &SPs) const;
    void GatherSPsUsed(set<StatePropID> &spUsed) const;

    std::shared_ptr<DtaLocLabel>  expr;
};

//------ Binary Operator Expression -----------------------
struct DtaLocLabel_BinOp : public DtaLocLabel {
    DtaLocLabel_BinOp(DtaLocLabel *e1, DtaLocLabel *e2, BinaryLogicOp b) :
/**/   expr1(e1), expr2(e2), bop(b) {}
    bool Evaluate(const vector<bool> &SPVals) const;
    void PrintExpr(ostream &os, const SymbolTable &SPs) const;
    void GatherSPsUsed(set<StatePropID> &spUsed) const;

    std::shared_ptr<DtaLocLabel>  expr1, expr2;
    BinaryLogicOp bop;
};


typedef pair<ClockValID, ClockValID>  CCPair_t;
typedef vector<CCPair_t>  PartialOrdSet;


//-----------------------------------------------------------------------------
// DTA: Deterministic Timed Automata
//-----------------------------------------------------------------------------
struct DTA : public MCObject {
public:
    // Location Properties
    struct LocationP {
        string					locName;  // location name
        DtaLocFlags				flags;    // initial, final or normal
        std::shared_ptr<DtaLocLabel> label;    // expression over state propositions
    };
    struct LocationPropertyKind { typedef vertex_property_tag kind; };
    typedef property<LocationPropertyKind, LocationP> LocationProperty;

    // Edge Properties
    struct EdgeP {
        vector<ActionID>  actions;		// Vector of actions (permitted or denied)
        ActionSetType     actSetType;	// How to interpret actions vector

        GuardFunction	  funct;		// Guard function enum
        ClockValID		  alpha, beta;  // Guard function params

        bool			  reset;		// if this edge resets the clock X

        inline bool isInnerEdge() const  { return actSetType != AST_BOUNDARY; }
        inline bool isBoundEdge() const  { return actSetType == AST_BOUNDARY; }
        string GetActSetDescription(const SymbolTable &Acts) const;
    };
    struct EdgePropertyKind { typedef edge_property_tag kind; };
    typedef property<EdgePropertyKind, EdgeP> EdgeProperty;


    // Type definitions for the adjacency list of the DTA graph
    typedef adjacency_list<vecS, vecS, bidirectionalS,
            /**/				   LocationProperty, EdgeProperty> DTAGraph;
    typedef graph_traits<DTAGraph>::vertex_iterator     location_iter_t;
    typedef graph_traits<DTAGraph>::edge_iterator       edge_iter_t;
    typedef graph_traits<DTAGraph>::vertex_descriptor   location_t;
    typedef graph_traits<DTAGraph>::edge_descriptor     edge_t;

    DTA();

    StatePropID AddStateProposition(const string &spName);
    ActionID AddActionSymbol(const string &actName);
    ClockValID AddClockConstrName(const string &cvName);
    bool AddClockConstrOrder(const string &cc1, const string &cc2);

    bool AddLocation(const string &locName, DtaLocFlags flags,
                     std::shared_ptr<DtaLocLabel> &label);
    bool AddEdge(const string &from, const string &to, ActionSetType actType,
                 const vector<string> &actions, GuardFunction gf,
                 const string &alpha, const string &beta, bool reset);

    string VerifyDeterminism(const DtaStatePropValues *optSPVals) const;
    bool IsPartialOrderValid(string &reason) const;

    MCObjectType GetObjectType() const { return MCOT_DTA; }
    bool isValidObject(string &reason) const;
    bool WriteAsGmlFile(ostream &os, const char *name, const char *args);

    // Instantiate a parametric DTA into a usable DTA
    bool Instantiate(const ASMC &asmc, const vector<double> &InstCCVals,
                     const vector< vector<string> > &InstActSets,
                     DTA &instDta, string &failReason) const;

public:
    DTAGraph		graph;		// The DTA graph
    SymbolTable		SPs;		// Alphabet of State Propositions
    SymbolTable		Acts;		// Alphabet of action names
    SymbolTable		CCNs;		// Clock constraint names U {0,inf}
    PartialOrdSet   PartialOrd;	// Partial order of clock contraint names

    // Instantiated DTA data:
    bool			instantiated;
    vector<double>  CCVals;	// Clock constraint values (excluded inf)

    mutable DtaDeterminismKind detKind; // Always or Semi-deterministic

    // Accessors for location & edge mapped data in the ASMC graph
    property_map<DTAGraph, LocationPropertyKind>::type  locationPMap;
    property_map<DTAGraph, EdgePropertyKind>::type      edgePMap;

protected:
    location_t FindLocation(const string &locName) const;
    bool AreLocationLabelsMutuallyExcl(const LocationP &lp1,
                                       const LocationP &lp2,
                                       const DtaStatePropValues *optSPVals,
                                       ostream &reason) const;
    bool AreActionSetsMutuallyExcl(const EdgeP &ep1, const EdgeP &ep2,
                                   ostream &reason) const;
    bool AreGuardsMutuallyExcl(const EdgeP &ep1, const EdgeP &ep2,
                               ostream &reason) const;

    int  AreCCNsOrdered(ClockValID v1, ClockValID v2) const;
    bool IsCCNsContained(ClockValID A, ClockValID V, ClockValID B) const;

    string GetGmlNameForLocation(const LocationP &lp) const;
    string GetGmlNameForEdge(const EdgeP &ep) const;
};

template <class T> inline
pair<typename graph_traits<T>::vertex_iterator,
     typename graph_traits<T>::vertex_iterator >
locations(const T &t)  {  return vertices(t);  }

//-----------------------------------------------------------------------------
#endif  // __cplusplus
#endif  // __DTA_H__