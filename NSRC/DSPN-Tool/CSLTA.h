/*
 *  MRP.h
 *
 *  Definitions of the CSL^TA model checking logic operators.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __CSLTA_MODEL_CHECKING_LOGIC_H__
#define __CSLTA_MODEL_CHECKING_LOGIC_H__
//=============================================================================

void ModelCheckCslTA(const RG2 &rg, shared_ptr<AtomicPropExpr> cslTaExpr,
                     boost::optional<marking_t> state0,
                     const SolverParams &spar, VerboseLevel verboseLvl);

// shared_ptr<AtomicPropExpr>
// SimpleProbExpr(const PN& pn, const char* dtaName,
// 			   const char* bindingRules, VerboseLevel verboseLvl);

shared_ptr<AtomicPropExpr>
ParseCslTaExpr(const PN &pn, const char *exprText, VerboseLevel verboseLvl);

//=============================================================================

// Probability constraint
struct ProbabilityConstr {
    ProbabilityConstr(CompareOp _cop, shared_ptr<Expr<double>> v) : cop(_cop), value(v) { }
    CompareOp				  cop;	 // operator used in comparison
    shared_ptr<Expr<double>>  value; // compared value expression
};
ostream &operator<<(ostream &os, const ProbabilityConstr &pc);

void EvaluateProbConstr(SAT &s, const ProbabilityConstr pc);

//=============================================================================
// Special CSL^TA logic operators: P() and S()
//=============================================================================

// // Base class of atomic propositions with a precomputed SAT object
// struct AtomicPropExprWithSatSet : public AtomicPropExpr {
// 	AtomicPropExprWithSatSet(ProbabilityConstr _pc) : pc(_pc) { }

// 	virtual bool EvaluateInRgState(const RG2& rg, const rgstate_t stateIndex) const override;

// protected:
// 	std::vector<bool>		sat;   // Satisfiability set
// 	ublas::vector<double>	probs; // Associated probabilities
// 	ProbabilityConstr 		pc;    // Probability constraint

// 	void EvaluateProbConstr();
// };

//=============================================================================

// CSLTA Operator P
struct CsltaProbExpr : public AtomicPropExpr {
    CsltaProbExpr(ProbabilityConstr _pc, const char *_dtaName, shared_ptr<DtaBinding> db)
        : pc(_pc), dtaName(_dtaName), dtaBinding(db) {}

    virtual void Print(ostream &os, bool expandParamNames) const override;

protected:
    virtual SAT Evaluate(const RG2 &rg, const SolverParams &spar,
                         boost::optional<marking_t>, VerboseLevel verboseLvl) override;

    ProbabilityConstr 		pc;    		// Probability constraint
    string 					dtaName;	// Name of the DTA for look-up
    shared_ptr<DtaBinding> 	dtaBinding;	// Binding rules
};

//=============================================================================
#endif   // __CSLTA_MODEL_CHECKING_LOGIC_H__

