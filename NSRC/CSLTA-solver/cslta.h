/*
 *  cslta.h
 *  CslTA-Solver
 *
 *  Definitions of the CSL over Timed Automata model checker
 *
 */

//-----------------------------------------------------------------------------
#ifndef __CSLTA_H__
#define __CSLTA_H__
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

struct DspnSolver;
typedef struct DspnSolver  DspnSolver;

// Probability comparison operator
enum ProbabilityCmpOp {
    PCO_LESS,
    PCO_LESS_EQ,
    PCO_GREATER,
    PCO_GREATER_EQ
};
typedef enum ProbabilityCmpOp  ProbabilityCmpOp;
extern const char *ProbabilityCmpOp_Str[];

// Probability constraint
struct ProbabilityConstr {
    ProbabilityCmpOp	pco;	// operator used in comparison
    double				value;	// compared value
};
typedef struct ProbabilityConstr  ProbabilityConstr;

// Atomic proposition comparation
enum APCmpOp {
    APCO_EQ, APCO_NOT_EQ,
    APCO_LESS, APCO_LESS_EQ,
    APCO_GREATER, APCO_GREATER_EQ,
    APCO_NOT_EQ_IMPLICIT
};
typedef enum APCmpOp  APCmpOp;
extern const char *APCmpOp_Str[];


//-----------------------------------------------------------------------------
// CSLTA Expression API:
//-----------------------------------------------------------------------------
PUBLIC_API void       APValExpr_DeleteExpr(APValExpr *expr);
PUBLIC_API APValExpr *APValExpr_NewConst(const char *val);
PUBLIC_API APValExpr *APValExpr_NewAtomProp(const char *ap);
PUBLIC_API APValExpr *APValExpr_NewUnOp(char op, APValExpr *expr);
PUBLIC_API APValExpr *APValExpr_NewBinOp(APValExpr *expr1, char op, APValExpr *expr2);


PUBLIC_API void		  CslTa_DeleteExpr(CslTaExpr *expr);
PUBLIC_API CslTaExpr *CslTa_NewTrueFalseExpr(int val);
PUBLIC_API CslTaExpr *CslTa_NewAPValExpr(APValExpr *e, APCmpOp op, const char *cmpVal);
PUBLIC_API CslTaExpr *CslTa_NewNotExpr(CslTaExpr *expr);
PUBLIC_API CslTaExpr *CslTa_NewAndExpr(CslTaExpr *e1, CslTaExpr *e2);
PUBLIC_API CslTaExpr *CslTa_NewOrExpr(CslTaExpr *e1, CslTaExpr *e2);
PUBLIC_API CslTaExpr *CslTa_NewImplyExpr(CslTaExpr *e1, CslTaExpr *e2);
PUBLIC_API CslTaExpr *CslTa_NewSteadyExpr(ProbabilityConstr pc, CslTaExpr *e1);
PUBLIC_API CslTaExpr *CslTa_NewProbTaExpr(ProbabilityConstr pc, DTAParams *dtap);


PUBLIC_API void       DTAParams_Delete(DTAParams *dtap);
PUBLIC_API DTAParams *DTAParams_New();
PUBLIC_API DTAParams *DTAParams_AddCCVal(DTAParams *dtap, const char *val);
PUBLIC_API DTAParams *DTAParams_AddActSetByName(DTAParams *dtap, const char *actName);
PUBLIC_API DTAParams *DTAParams_AddActSetByIdList(DTAParams *dtap, IDList *idl);
PUBLIC_API DTAParams *DTAParams_AddCslTaExpr(DTAParams *dtap, CslTaExpr *expr);
PUBLIC_API DTAParams *DTAParams_MergeParams(const char *dtaName, DTAParams *ccvs,
        DTAParams *acts, DTAParams *expr);


//-----------------------------------------------------------------------------
#ifdef __cplusplus

// Names of absorbing "places"
extern const char *s_absorbPlaces[];
extern const size_t NUM_ABSORBING_PLACES;



//-----------------------------------------------------------------------------
// Result of a CslTa expression evaluation
//-----------------------------------------------------------------------------
struct CslTaResult : public MCObject {
public:
    CslTaResult() { }
    virtual ~CslTaResult() { }

    MCObjectType GetObjectType() const  { return MCOT_RESULT; }
    bool isValidObject(string &reason) const  { return true; }
    bool WriteAsGmlFile(ostream &os, const char *name, const char *args);
    bool LoadDspnCoords(istream &is, const char *args);
    bool PlanarizeDspn(const char *args);
    bool GetDspnToSave(const char *args, DSPN &outDspn);

public:
    vector<bool>	eval;	// Evaluation results
    vector<double>	probs;	// Last probabilities computed

    std::shared_ptr<DspnSolver>  solverUsed; // Solver used for DSPN computation, if any
};


//-----------------------------------------------------------------------------
// DTA Parameterization Structure
//-----------------------------------------------------------------------------
struct DTAParams {
    string							DtaName; // Object name of the DTA
    vector<double>					CCVals;  // Clock constant values
    vector<std::shared_ptr<CslTaExpr> >	SPsExpr; // State proposition expressions
    vector<vector<string> >			ActSets; // Action sets
};

//-----------------------------------------------------------------------------
// Atomic Proposition Values Expression
//-----------------------------------------------------------------------------
struct APValExpr {
public:
    virtual ~APValExpr()  { }

    // Tests the expression before the evaluation starts
    virtual bool PrepareEvaluation(ASMC &asmc) const = 0;
    // Evaluates the CslTA expression
    virtual bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                          vector<int> &exprVal) const = 0;
    // Prints the expression in human-readable format
    virtual void PrintExpr(ostream &os) const = 0;

    friend ostream &operator<<(ostream &os, const APValExpr &e);
};
inline ostream &operator<<(ostream &os, const APValExpr &e)
{   e.PrintExpr(os); return os;   }

// ------------------ Constant ------------------
struct APValExpr_Const : public APValExpr {
    APValExpr_Const(int _val) : val(_val) { }
    virtual bool PrepareEvaluation(ASMC &asmc) const;
    virtual bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                          vector<int> &exprVal) const;
    virtual void PrintExpr(ostream &os) const;

    int val;
};
// ---------- ASMC Atomic Proposition -----------
struct APValExpr_AP : public APValExpr {
    APValExpr_AP(const char *_ap) : ap(_ap) { }
    virtual bool PrepareEvaluation(ASMC &asmc) const;
    virtual bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                          vector<int> &exprVal) const;
    virtual void PrintExpr(ostream &os) const;

    string ap;
};
// --------- Unary Operator expression ---------
struct APValExpr_UnOp : public APValExpr {
    APValExpr_UnOp(char o, APValExpr *e) : expr(e), op(o) { }
    virtual bool PrepareEvaluation(ASMC &asmc) const;
    virtual bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                          vector<int> &exprVal) const;
    virtual void PrintExpr(ostream &os) const;

    std::shared_ptr<APValExpr> expr;
    char op;
};
// --------- Binary Operator expression ---------
struct APValExpr_BinOp : public APValExpr {
    APValExpr_BinOp(APValExpr *e1, char o, APValExpr *e2) :
/**/ expr1(e1), expr2(e2), op(o) { }
    virtual bool PrepareEvaluation(ASMC &asmc) const;
    virtual bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                          vector<int> &exprVal) const;
    virtual void PrintExpr(ostream &os) const;

    std::shared_ptr<APValExpr> expr1, expr2;
    char op;
};

//-----------------------------------------------------------------------------
// CslTa Expression
//-----------------------------------------------------------------------------
struct CslTaExpr {
public:
    virtual ~CslTaExpr() { }

    // Tests the expression before the evaluation starts
    virtual bool PrepareEvaluation(ASMC &asmc) = 0;
    // Evaluates the CslTA expression
    virtual bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                          CslTaResult &res) const = 0;
    // Prints the expression in human-readable format
    virtual void PrintExpr(ostream &os) const = 0;

    friend ostream &operator<<(ostream &os, const CslTaExpr &e);
};
inline ostream &operator<<(ostream &os, const CslTaExpr &e)
{   e.PrintExpr(os); return os;   }

// ----------- True/False expression -----------
struct CslTaExpr_TF : public CslTaExpr {
    CslTaExpr_TF(bool _val) : val(_val) {}
    bool PrepareEvaluation(ASMC &asmc);
    bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                  CslTaResult &res) const;
    void PrintExpr(ostream &os) const;

    bool val;
};

// ----- Atomic Proposition expression ---------
struct CslTaExpr_AP : public CslTaExpr {
    CslTaExpr_AP(APValExpr *e, APCmpOp _op, int cv) :
/**/ apExpr(e), op(_op), cmpVal(cv) {}
    bool PrepareEvaluation(ASMC &asmc);
    bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                  CslTaResult &res) const;
    void PrintExpr(ostream &os) const;

    std::shared_ptr<APValExpr> apExpr;
    APCmpOp op;
    int cmpVal;
};

// ----- NOT expression ------------------------
struct CslTaExpr_NOT : public CslTaExpr {
    CslTaExpr_NOT(CslTaExpr *e) : expr(e) {}
    bool PrepareEvaluation(ASMC &asmc);
    bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                  CslTaResult &res) const;
    void PrintExpr(ostream &os) const;

    std::shared_ptr<CslTaExpr> expr;
};

// ----- Binary Operator expression ------------
struct CslTaExpr_BOP : public CslTaExpr {
    CslTaExpr_BOP(CslTaExpr *e1, CslTaExpr *e2, BinaryLogicOp p) :
/**/ expr1(e1), expr2(e2), op(p) {}
    bool PrepareEvaluation(ASMC &asmc);
    bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                  CslTaResult &res) const;
    void PrintExpr(ostream &os) const;

    std::shared_ptr<CslTaExpr> expr1, expr2;
    BinaryLogicOp op;
};
// ----- STEADY expression ---------------------
struct CslTaExpr_STEADY : public CslTaExpr {
    CslTaExpr_STEADY(ProbabilityConstr &_pc, CslTaExpr *e) : pc(_pc), expr(e) {}
    bool PrepareEvaluation(ASMC &asmc);
    bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                  CslTaResult &res) const;
    void PrintExpr(ostream &os) const;

    ProbabilityConstr		pc;
    std::shared_ptr<CslTaExpr>	expr;
};
// ----- PROB_TA expression --------------------
struct CslTaExpr_PROB_TA : public CslTaExpr {
    CslTaExpr_PROB_TA(ProbabilityConstr &_pc, DTAParams *p) : pc(_pc), params(p) {}
    bool PrepareEvaluation(ASMC &asmc);
    bool Evaluate(ASMC &asmc, const vector<bool> &evalStates,
                  CslTaResult &res) const;
    void PrintExpr(ostream &os) const;

    ProbabilityConstr		pc;
    std::shared_ptr<DTAParams>	params;
    DTA						instDta;
};


//-----------------------------------------------------------------------------
// DSPN Solver: base class for all DSPN solvers (STEADY and PROB_TA)
//-----------------------------------------------------------------------------
struct DspnSolver {
public:
    virtual ~DspnSolver() { }

    // Writes as a DSPN file one of the solver DSPNs
    virtual bool WriteSolverDspnAsGml(ostream &os, const char *name,
                                      const char *args) = 0;
    // Load DSPN coordinates from a GML file
    virtual bool LoadDspnCoords(istream &is, const char *args) = 0;
    // Planarize a DSPN
    virtual bool PlanarizeDspn(const char *args) = 0;
    // Get a DSPN, ready to be saved to disc for inspection
    virtual bool GetDspnToSave(const char *args, DSPN &outDspn) = 0;

};

//-----------------------------------------------------------------------------
// CslTa ProbTA Solver : intermediate class that creates a DSPN that can be
//   used to solve a ProbTA expression throughout a proper DSPN solver.
//-----------------------------------------------------------------------------

typedef vector< vector<size_t> >  VectorOfInitMarks;

struct ProbTaSolver : public DspnSolver {
public:
    ProbTaSolver(const ASMC *_asmc, const DTA *_dta, bool _semiDet,
                 const vector<CslTaResult> *_subExp)
    /**/ : asmc(_asmc), dta(_dta), isDtaSemiDet(_semiDet), subExprRes(_subExp) { }

    // Create the solving DSPN. On return, initMarks contains the additional
    // places that should be marked in the DSPN initial marking to compute
    // the probability of the ASMC state s(i).
    void CreateDSPN();

    bool WriteSolverDspnAsGml(ostream &os, const char *name, const char *args);
    bool LoadDspnCoords(istream &is, const char *args);
    bool PlanarizeDspn(const char *args);
    bool GetDspnToSave(const char *args, DSPN &outDspn);

    bool SolveInState(size_t s, double &outProb) const;

protected:
    void CreateStatePropDSPN(DSPN &spDspn);
    void CreateFinalStatesDSPN(DSPN &finDspn);
    void CreateActionSetExpressionDSPN(DSPN &actDspn);
    void CreateAsmcDSPN(DSPN &asmcDspn);
    void CreateClockDSPN(DSPN &clkDspn);
    void CreateDtaDSPN(DSPN &dtaDspn);
    void PrepareInitMarkVector();

protected:
    // --- Main Variables ---
    const ASMC		*asmc;	 // Stochastic automata that models the problem
    const DTA		*dta;	 // Instantiated DTA
    const bool		 isDtaSemiDet; // Is *dta semi-deterministic or fully det.?
    const vector<CslTaResult> *subExprRes; // Results of the CslTa sub-expr
    // used as parameters in the DTA

    // --- Additional Variables ---

    // Place Labels of State Proposition Expressions (i.e. phi OR NOT psi, etc...)
    vector<string>	spExprNames;
    map<string, DTA::location_t> spExprDtaLocs;

    // Place Labels of Action Set Expressions (like "Act - {a,b}"). ActSetExpr
    // are shared between multiple arcs of the DTA subnet
    vector<string>  actSetNames; // subexpressions
    map<string, DTA::edge_t> actSetDtaEdges; // DTA edge associated to

    // Place labels of the clock net when x=CCV (for boundary edge guards)
    vector<string>  clkBoundNames;

    // Intermediate DSPNs
    DSPN  spDspn, finDspn, actDspn, asmcDspn, clkDspn, dtaDspn;

    // Priorities used by immediate transitions
    size_t Pri_BOT, Pri_Fin, Pri_Clk0, Pri_Clk;
    size_t Pri_DtaSEdgesBase, Pri_DtaAEdgesBase, Pri_ActDrainBase;
    size_t Pri_ResetClk, Pri_DetTest, Pri_StopSPs, Pri_StopClkBase;
    size_t Pri_StopAsmcBase, Pri_StopDtaSelBase, Pri_StopDtaLocBase;

    // Final DSPN and initial marking vector
    DSPN  dspn;
    VectorOfInitMarks  initMarks;

public:
    // Special Place names:
    static const char *PLN_RUNNING, *PLN_TOP, *PLN_BOT, *PLN_NONDET;
    static const char *PLN_ASMCMOVED, *PLN_CLOCK, *PLN_DTAMOVED;
    static const char *PLN_BSCCSINK;

    // Cluster names: (clusters are logic/graphic groups of places & transitions)
    static const char *CLNET_SP, *CLNET_FIN, *CLNET_ASMC;
    static const char *CLNET_ACT, *CLNET_CLK, *CLNET_DTA;

protected:
    // Test satisfiability of a DTA location label in a given ASMC state,
    // using as state proposition truth values the ones computed with
    // the sub-CslTa expression.
    bool isSpExprSatisfiedInAsmcState(const DtaLocLabel *spExpr,
                                      ASMC::vertex_t asmcState);

    bool _GetDspnByName(const char *name, DSPN *&p);
};


//-----------------------------------------------------------------------------
// CslTa Steady Solver : intermediate class for CslTa expr:  S < lambda (phi)
//-----------------------------------------------------------------------------
struct SteadySolver : public DspnSolver {
public:
    SteadySolver(ASMC *a, CslTaResult *r);

    void CreateDSPN();

    bool WriteSolverDspnAsGml(ostream &os, const char *name, const char *args);
    bool LoadDspnCoords(istream &is, const char *args);
    bool PlanarizeDspn(const char *args);
    bool GetDspnToSave(const char *args, DSPN &outDspn);

    bool SolveInState(size_t s, const vector<bool> &Phi,
                      int numBscc, double &outProb);

protected:
    ASMC		*asmc;	// Tested automata
    CslTaResult *phi;	// Subexpression result
    DSPN		 dspn;	// Generated DSPN
};




//-----------------------------------------------------------------------------
#endif  // __cplusplus
#endif  // __CSLTA_H__