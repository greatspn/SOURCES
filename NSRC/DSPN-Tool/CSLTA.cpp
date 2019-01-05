/*
 *  CSLTA.cpp
 *
 *  Implementation of the CSLTA interface.
 *
 *  Created by Elvio Amparore
 *
 */

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
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <functional>
using namespace std;

#include <boost/optional.hpp>

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
//#include "ParserDefs.h"
#include "PetriNet.h"
#include "Measure.h"
#include "Language.h"
#include "ReachabilityGraph.h"
#include "PackedMemoryPool.h"
#include "NewReachabilityGraph.h"
#include "newparser.h"
#include "DTA.h"
#include "MRP.h"
#include "MRP_Explicit.h"
#include "MRP_Implicit.h"
#include "MRP_SccDecomp.h"
#include "SynchProduct.h"
#include "SynchProductSCC.h"
#include "CSLTA.h"



//=============================================================================

ostream &operator<<(ostream &os, const ProbabilityConstr &pc) {
    os << CompareOp_Names[pc.cop] << " ";
    pc.value->Print(os, true);
    return os;
}

//=============================================================================

void ModelCheckCslTA(const RG2 &rg, shared_ptr<AtomicPropExpr> cslTaExpr,
                     boost::optional<marking_t> state0,
                     const SolverParams &spar, VerboseLevel verboseLvl) {
    // Model check P() and S() operators
    cslTaExpr->StoreEvaluation(rg, spar, state0, verboseLvl);
}

//=============================================================================

// shared_ptr<AtomicPropExpr>
// SimpleProbExpr(const PN& pn, const char* dtaName,
// 			   const char* bindingRules, VerboseLevel verboseLvl)
// {
// 	ostringstream text;
// 	text << "PROB_TA > 0.0 " << dtaName << "(" << bindingRules << ")";
// 	return ParseCslTaExpr(pn, text.str(), verboseLvl);
// }

//=============================================================================

shared_ptr<AtomicPropExpr>
ParseCslTaExpr(const PN &pn, const char *exprText, VerboseLevel verboseLvl) {
    shared_ptr<AtomicPropExpr> expr;

    // Parse the CSLTA model checking logic expression
    do {
        shared_ptr<ParserObj> pObj;
        pObj = ParseObjectExpr(&pn, nullptr, exprText, TOKEN_CSLTA_EXPR,
                               "CSLTA expression: ", verboseLvl);
        expr = shared_polymorphic_downcast<AtomicPropExpr>(pObj);
    }
    while (false);

    return expr;
}

//=============================================================================

SAT AtomicPropExpr_MdepExpr::Evaluate(const RG2 &rg, const SolverParams &spar,
                                      boost::optional<marking_t> state0,
                                      VerboseLevel verboseLvl) {
    if (IsStored()) {
        assert(GetStoredSAT().state == state0);
        return GetStoredSAT();
    }
    SAT sat;

    // Extract the state marking and evaluate
    RgState stateEntry;
    if (state0) {
        rgstate_t state0Index(*state0, TANGIBLE);
        UnpackState(rg, state0Index, stateEntry);

        sat.sat.push_back(expr->EvaluateInMarking(stateEntry.marking));
        sat.state = state0;
    }
    else { // All states
        sat.sat.resize(rg.NumStates());
        for (size_t i = 0; i < rg.NumStates(); i++) {
            rgstate_t stateIndex(i, TANGIBLE);
            UnpackState(rg, stateIndex, stateEntry);

            sat.sat[i] = expr->EvaluateInMarking(stateEntry.marking);
        }
    }
    return sat;
}

void AtomicPropExpr_MdepExpr::Print(ostream &os, bool expandParamNames) const {
    expr->Print(os, expandParamNames);
}

//=============================================================================

SAT PtrToAtomicPropExpr::Evaluate(const RG2 &rg, const SolverParams &spar,
                                  boost::optional<marking_t> state0,
                                  VerboseLevel verboseLvl) {
    return apExpr->Evaluate(rg, spar, state0, verboseLvl);
}

void PtrToAtomicPropExpr::Print(ostream &os, bool expandParamNames) const {
    if (isBound())
        apExpr->Print(os, expandParamNames);
    else
        os << "<<" << name << ">>";
}

//=============================================================================

SAT AtomicPropExpr_Const::Evaluate(const RG2 &rg, const SolverParams &spar,
                                   boost::optional<marking_t> state0,
                                   VerboseLevel verboseLvl) {
    SAT sat;
    if (state0) {
        sat.sat.resize(1);
        sat.state = state0;
    }
    else
        sat.sat.resize(rg.NumStates());

    std::fill(sat.sat.begin(), sat.sat.end(), value);
    return sat;
}

void AtomicPropExpr_Const::Print(ostream &os, bool expandParamNames) const {
    ExprTraits<bool>::Print(os, value);
}

//=============================================================================

SAT AtomicPropExpr_NOT::Evaluate(const RG2 &rg, const SolverParams &spar,
                                 boost::optional<marking_t> state0,
                                 VerboseLevel verboseLvl) {
    return ! expr->Evaluate(rg, spar, state0, verboseLvl);
}

void AtomicPropExpr_NOT::Print(ostream &os, bool expandParamNames) const {
    os << "~(";
    expr->Print(os, expandParamNames);
    os << ")";
}

//=============================================================================

SAT AtomicPropExpr_BOP::Evaluate(const RG2 &rg, const SolverParams &spar,
                                 boost::optional<marking_t> state0,
                                 VerboseLevel verboseLvl) {
    switch (op) {
    case BBF_AND:
        return expr1->Evaluate(rg, spar, state0, verboseLvl) &&
               expr2->Evaluate(rg, spar, state0, verboseLvl);
    case BBF_OR:
        return expr1->Evaluate(rg, spar, state0, verboseLvl) ||
               expr2->Evaluate(rg, spar, state0, verboseLvl);
    }
    throw program_exception("AtomicPropExpr_BOP::EvaluateInRgState: Internal error.");
}

void AtomicPropExpr_BOP::Print(ostream &os, bool expandParamNames) const {
    os << "(";
    expr1->Print(os, expandParamNames);
    os << " " << BinaryBoolFunct_Names[op] << " ";
    expr2->Print(os, expandParamNames);
    os << ")";
}

//=============================================================================




//=============================================================================

void EvaluateProbConstr(SAT &s, const ProbabilityConstr pc) {
    s.sat.resize(s.probs.size());

    const double cmpVal = pc.value->Evaluate();
    for (size_t i = 0; i < s.probs.size(); i++) {
        bool res;
        switch (pc.cop) {
        case CMPOP_EQ:			res = s.probs[i] == cmpVal;   break;
        case CMPOP_NOT_EQ:		res = s.probs[i] != cmpVal;   break;
        case CMPOP_LESS:		res = s.probs[i] <  cmpVal;   break;
        case CMPOP_LESS_EQ:		res = s.probs[i] <= cmpVal;   break;
        case CMPOP_GREATER:		res = s.probs[i] >  cmpVal;   break;
        case CMPOP_GREATER_EQ:	res = s.probs[i] >= cmpVal;   break;
        default:
            throw program_exception("EvaluateProbConstr(): internal error.");
        }
        s.sat[i] = res;
    }
}

//=============================================================================

int ispathsep(int ch)    { return ch == PATH_SEP; }
int isnotpathsep(int ch) { return ch != PATH_SEP; }

SAT CsltaProbExpr::Evaluate(const RG2 &rg, const SolverParams &spar,
                            boost::optional<marking_t> state0,
                            VerboseLevel verboseLvl) {
    if (IsStored()) {
        assert(GetStoredSAT().state == state0);
        return GetStoredSAT();
    }

    // Procedure that tries to open the dta file from the specified directory
    string dtaFilename;
    auto try_open = [&](ifstream & ifs, const char *pathDir) -> bool {
        dtaFilename = pathDir;
        dtaFilename += DIR_SEP;
        dtaFilename += dtaName;
        ifs.open(dtaFilename.c_str());
        if (ifs.is_open())
            return true;
        dtaFilename += ".dta";
        ifs.open(dtaFilename.c_str());
        if (ifs.is_open())
            return true;
        return false;
    };

    // First, locate the DTA file in the filesystem and load it
    ifstream ifsDta;
    for (const string &pathDir : spar.dtaSearchPaths) {
        if (try_open(ifsDta, pathDir.c_str()))
            break;
    }
    const char *envVar = getenv("DTA_PATH");
    if (!ifsDta.is_open()) {
        if (envVar != nullptr && strlen(envVar) > 0) {
            istringstream envStr(envVar);
            simple_tokenizer st(&envStr);
            string dir;
            do {
                st.gettok(dir, ispathsep);
                st.gettok(dir, isnotpathsep);
                if (dir.empty())
                    break;
                if (try_open(ifsDta, dir.c_str()))
                    break;
            }
            while (true);
        }
    }
    if (!ifsDta.is_open()) {
        ostringstream reason;
        reason << "Could not open a DTA file \"" << dtaName << "\" in the DTA search path.\n";
        reason << "       DTAs must be located in the same directory, in the DTA/ subdirectory,\n";
        reason << "       in any directory specified by the option -dta-path <dirname>,\n";
        reason << "       or in the directories specified in the DTA_PATH environment variable.\n";
        reason << "       Current DTA search path: " << print_vec(spar.dtaSearchPaths, s_pvfmtPathSep) << "\n";
        reason << "       DTA_PATH variable: " << (envVar != nullptr ? envVar : "Not defined.") << endl;
        throw program_exception(reason.str());
    }
    if (verboseLvl >= VL_BASIC)
        cout << "LOADING DTA FILE \"" << dtaFilename << "\" ..." << endl;

    DTA dta;
    LoadDTAFromFile(ifsDta, dta, verboseLvl);

    // Bind the DTA using the binding rules parsed from the CSL^TA expression
    bool isComplete = BindDTA(dta, dtaBinding, verboseLvl);
    if (!isComplete) {
        throw program_exception("Evaluation of the model checking operator P() "
                                "requires a fully bound DTA.");
    }

    bool isForward = (bool)(state0);
    if (verboseLvl >= VL_VERBOSE) {
        cout << "SOLVING " << (isForward ? "FORWARD" : "BACKWARD") << " CSLTA EXPRESSION: ";
        Print(cout, true);
        cout << "..." << endl;
    }

    SAT sat;
    if (isForward) {
        sat.state = state0;
        sat.probs.resize(1);
    }

    bool useZonedDTA = spar.useZonedDTA;
    if (spar.onTheFlyModelChecking)
        useZonedDTA = true; // ZDTA is required by the on-the-fly method
    DTA zdta, *pdta = &dta;
    if (useZonedDTA) {
        MakeZonedDTA(dta, zdta, verboseLvl);
        pdta = &zdta;
    }
    PrepareStatePropositions(rg, *pdta, spar, verboseLvl);

    // Compute the P() operator probabilities
    if (spar.onTheFlyModelChecking) {
        // Component-based with on-the-fly state space construction
        if (isForward) // Only for the initial state
            sat.probs[0] = ModelCheck_CTMC_ZDTA_Forward_SCC(rg, *pdta, *state0, spar, verboseLvl);
        else // Compute for all the states
            ModelCheck_CTMC_ZDTA_Backward_SCC(rg, *pdta, sat.probs, spar, verboseLvl);
    }
    else {
        if (isForward)
            sat.probs[0] = ModelCheck_CTMC_DTA_Forward(rg, *pdta, *state0, spar, verboseLvl);
        else // Backward whole-state-space model checking
            ModelCheck_CTMC_DTA_Backward(rg, *pdta, sat.probs, spar, verboseLvl);
    }

    // Complete the evaluation of the SAT set.
    EvaluateProbConstr(sat, pc);

    return sat;
}

//=============================================================================

void CsltaProbExpr::Print(ostream &os, bool expandParamNames) const {
    os << "PROB_TA " << pc << " \"" << dtaName << "\" (";
    dtaBinding->Print(os, expandParamNames);
    os << ")";
}

//=============================================================================





















