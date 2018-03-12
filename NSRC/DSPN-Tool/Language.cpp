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
#include <cfloat>
#include <cstring>
#include <memory>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "Language.h"

//=============================================================================

const char *CompareOp_Names[] = {
    "=", "/=", "<", "<=", ">", ">="
};
const char *CompareOp_SPNicaNames[] = {
    "==", "!=", "<", "<=", ">", ">="
};
const char *CompareOp_CosmosNames[] = {
    "=", "!=", "<", "<=", ">", ">="
};
const char *UnaryFunct_Names[UF_NumFuncts] = {
    "-", "Abs", "Sin", "Cos", "Tan",
    "ArcSin", "ArcCos", "ArcTan", "Exp",
    "Log", "Sqrt", "Ceil", "Floor", "Factorial",
    "I"
};
const bool  UnaryFunct_isPrefix[UF_NumFuncts] = {
    true, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false
};
const char *UnaryFunct_SPNicaNames[UF_NumFuncts] = {
    "-", "Abs", "Sin", "Cos", "Tan",
    "ArcSin", "ArcCos", "ArcTan", "Exp",
    "Log", "Sqrt", "Ceil", "Floor", "Factorial", "DiracDelta"
};
const char *UnaryFunct_CosmosNames[UF_NumFuncts] = {
    "-", "abs", "sin", "cos", "tan",
    "arcsin", "arccos", "arctan", "exp",
    "log", "sqrt", "ceil", "floor", "factorial", "DiracDelta"
};
const char *BinaryFunct_Names[BF_NumFuncts] = {
    "+", "-", "*", "/", "Min", "Max", 
    "Mod", "Pow", "Log", "Binomial", "Fract",
    "R", "Uniform", "Triangular", "Erlang", "TruncatedExp", "Pareto"
};
const bool  BinaryFunct_isPrefix[BF_NumFuncts] = {
    false, false, false, false, true, true,
    true, true, true, true, true,
    true, true, true, true, true, true, 
};
const char *BinaryFunct_SPNicaNames[BF_NumFuncts] = {
    "+", "-", "*", "/", "Min", "Max", "Mod", "Power", "Log", "Binomial", "Divide",
    "R", "uniform", "triangular", "erlang", "truncatedexp", "pareto"
};
const char *BinaryFunct_CosmosNames[BF_NumFuncts] = {
    "+", "-", "*", "/", "min", "max", "mod", "^", "log", "binomial", "divide",
    "R", "uniform", "triangular", "erlang", "truncatedexp", "pareto"
};


const char *BinaryBoolFunct_Names[] = { "&", "|" };
const char *BinaryBoolFunct_SPNicaNames[] = { "&&", "||" };
const char *BinaryBoolFunct_CosmosNames[] = { "&", "|" };

const char *UnaryBoolFunct_Names[] = { "~" };
const char *UnaryBoolFunct_SPNicaNames[] = { "!" };
const char *UnaryBoolFunct_CosmosNames[] = { "!" };



//=============================================================================

template<typename T>
struct ExprEvalInterface : boost::noncopyable {
    virtual ~ExprEvalInterface() { }
    virtual void evalConstValExpr(const T &value) const = 0;
    virtual void evalParamValExpr(const string &name, const shared_ptr<Expr<T> > &ex) const = 0;
    // CastExpr
};

template<typename T>
struct MdepExprEvalInterface : public ExprEvalInterface<T> {
    // CastMdepExpr
    virtual void evalExpr(const shared_ptr<Expr<T> > &ex) const = 0;
    virtual void evalPlaceMarkingMdepExpr(const string &plcName, const place_t plcInd) const = 0;
};


//=============================================================================

string getEnablingDegreeInSpnicaFormat(const PN &pn, const Transition &trn) {
    ostringstream enabFn;
    enabFn << "Min[";
    vector<Arc>::const_iterator iaIt = trn.arcs[IA].begin();
    for (; iaIt != trn.arcs[IA].end(); ++iaIt) {
        if (iaIt->isMultMarkingDep()) {
            enabFn << "If[0!=";
            iaIt->multFn->PrintInSPNicaFormat(enabFn, pn);
            enabFn << ", rr[" << pn.plcs[iaIt->plc].name << "] / ";
            iaIt->multFn->PrintInSPNicaFormat(enabFn, pn);
            enabFn << ", DirectedInfinity[1]]";
        }
        else { // constant
            enabFn << "rr[" << pn.plcs[iaIt->plc].name << "]";
            if (iaIt->getConstantMult() != tokencount_t(1)) {
                enabFn << "/";
                iaIt->multFn->PrintInSPNicaFormat(enabFn, pn);
            }
        }
        enabFn << ",";
    }
    enabFn << "DirectedInfinity[1]]";
    return enabFn.str();
}


//=============================================================================






//=============================================================================

string getEnablingDegreeInCosmosFormat(const PN &pn, const Transition &trn) {
    ostringstream enabFn;
    enabFn << "min(";
    vector<Arc>::const_iterator iaIt = trn.arcs[IA].begin();
    for (; iaIt != trn.arcs[IA].end(); ++iaIt) {
        if (iaIt->isMultMarkingDep()) {
            enabFn << "( ";
            iaIt->multFn->PrintInCosmosFormat(enabFn, pn);
            enabFn << " > 0)* " << pn.plcs[iaIt->plc].name << ") / ";
            iaIt->multFn->PrintInCosmosFormat(enabFn, pn);
            enabFn << ", INT_MAX)";
        }
        else { // arc condition is constant
            enabFn << " " << pn.plcs[iaIt->plc].name << " ";
            if (iaIt->getConstantMult() != tokencount_t(1)) {
                enabFn << "/";
                iaIt->multFn->PrintInCosmosFormat(enabFn, pn);
            }
        }
        enabFn << ",";
    }
    enabFn << " INT_MAX)";
    return enabFn.str();
}




//=============================================================================
//      MultipleServerDelayFn transition delay
//=============================================================================

bool MultipleServerDelayFn::IsMarkingDep() const {
    return (numSrv != 1); // || delay->IsMarkingDep
}
double MultipleServerDelayFn::EvaluateConstantDelay() const {
    assert(!IsMarkingDep());
    return delay->Evaluate();
}

double MultipleServerDelayFn::EvaluateDelay(const PN &pn, const SparseMarking &mark,
        const Transition &trn) const {
    double d = delay->Evaluate();
    size_t enabDeg = 1;
    if (numSrv > 1)
        enabDeg = TransitionEnablingDegree(pn, trn, mark);
    return d * min(numSrv, enabDeg);
}

void MultipleServerDelayFn::Print(ostream &os, bool expandParamNames) const {
    delay->Print(os, expandParamNames);
    if (numSrv == numeric_limits<size_t>::max())
        os << " (INFINITE-SERVERS)";
    else if (numSrv > 1)
        os << " (" << numSrv << "-SERVERS)";
}


void MultipleServerDelayFn::PrintInMarking(ostream &os, const PN &pn,
        const SparseMarking &mark,
        const Transition &trn) const {
    delay->Print(os, false);
    if (numSrv > 1) {
        size_t enabDeg = min(numSrv, TransitionEnablingDegree(pn, trn, mark));
        os << " * " << enabDeg;
    }
}

bool MultipleServerDelayFn::IsPDF() const {
    return false;
}

void MultipleServerDelayFn::PrintInSPNicaFormat(ostream &os, const PN &pn,
        const Transition &trn) const {
    delay->PrintInSPNicaFormat(os, pn);

    if (numSrv == 1)
        return;

    string enabDegFn = getEnablingDegreeInSpnicaFormat(pn, trn);
    os << "*";
    if (numSrv == numeric_limits<size_t>::max())
        os << enabDegFn;
    else
        os << "Min[" << numSrv << "," << enabDegFn << "]";
}



void MultipleServerDelayFn::PrintInCosmosFormat(ostream &os, const PN &pn,
        const Transition &trn) const {
    switch (trn.distrib) {
    case IMM: //(IMMEDIATE, priority, weight)
        os << "IMMEDIATE, " << trn.prio << ", ";
        delay->PrintInCosmosFormat(os, pn);
        break;

    case EXP: //(EXPONENTIAL(Rate), priority, weight, ServicePolicy)
        //In GreatSpn priority and weight are only defined for immediates transitions
        //this is why  here priority = weight = 1
        os << "EXPONENTIAL(";
        delay->PrintInCosmosFormat(os, pn);
        os << "), 1, 1, ";
        if (numSrv == numeric_limits<size_t>::max())
            os << " INFINITE";
        else {
            if (numSrv > 1)
                os << " MULTIPLE(" << numSrv << ")";
            else
                os << " SINGLE";
        }
        break;

    case DET: //(DETERMINISTIC(Rate), priority, weight)
        //In GreatSpn priority and weight are only defined for immediates transitions
        //this is why  here priority = weight = 1
        os << "DETERMINISTIC(";
        delay->PrintInCosmosFormat(os, pn);
        os << "), 1, 1";
        break;

    default:
        throw program_exception("MultipleServerDelayFn::PrintInCosmosFormat");
    }
}

//=============================================================================
//    LoadDependentDelayFn transition delay
//=============================================================================

bool LoadDependentDelayFn::IsMarkingDep() const {
    return (delays.size() != 1);
}

double LoadDependentDelayFn::EvaluateConstantDelay() const {
    assert(!IsMarkingDep());
    return delays[0];
}

double LoadDependentDelayFn::EvaluateDelay(const PN &pn, const SparseMarking &mark,
        const Transition &trn) const {
    size_t enabDeg = TransitionEnablingDegree(pn, trn, mark);
    return delays[ min(enabDeg, delays.size()) - 1 ];
}

void LoadDependentDelayFn::Print(ostream &os, bool expandParamNames) const {
    os << print_vec(delays) << " (LOAD-DEPENDENT)";
}

void LoadDependentDelayFn::PrintInMarking(ostream &os, const PN &pn,
        const SparseMarking &mark,
        const Transition &trn) const {
    size_t enabDeg = TransitionEnablingDegree(pn, trn, mark);
    double d = delays[ min(delays.size(), enabDeg) - 1];
    os << d;
}

bool LoadDependentDelayFn::IsPDF() const {
    return false;
}

void LoadDependentDelayFn::PrintInSPNicaFormat(ostream &os, const PN &pn,
        const Transition &trn) const {
    string enabDegFn = getEnablingDegreeInSpnicaFormat(pn, trn);

    os << "Switch[Min[" << delays.size() << "," << enabDegFn << "]";
    for (size_t i = 0; i < delays.size(); i++)
        os << "," << i + 1 << "," << delays[i];
    os << "]";
}

void LoadDependentDelayFn::PrintInCosmosFormat(ostream &os, const PN &pn,
        const Transition &trn) const {
    os << "Load Dependent not implemented in cosmos";
}

//=============================================================================
//    MarkingDependentDelayFn transition delay
//=============================================================================

bool MarkingDependentDelayFn::IsMarkingDep() const {
    return mdepFn->IsMarkingDep();
}

double MarkingDependentDelayFn::EvaluateConstantDelay() const {
    assert(!IsMarkingDep());
    return mdepFn->Evaluate();
}

double MarkingDependentDelayFn::EvaluateDelay(const PN &pn, const SparseMarking &mark,
        const Transition &trn) const {
    return mdepFn->EvaluateInMarking(mark);
}

void MarkingDependentDelayFn::Print(ostream &os, bool expandParamNames) const {
    mdepFn->Print(os, expandParamNames);
    os << " (MARK-DEP)";
}

void MarkingDependentDelayFn::PrintInMarking(ostream &os, const PN &pn,
        const SparseMarking &mark,
        const Transition &trn) const {
    os << EvaluateDelay(pn, mark, trn);
}

bool MarkingDependentDelayFn::IsPDF() const {
    return false;
}

void MarkingDependentDelayFn::PrintInSPNicaFormat(ostream &os, const PN &pn,
        const Transition &trn) const {
    mdepFn->PrintInSPNicaFormat(os, pn);
}

void MarkingDependentDelayFn::PrintInCosmosFormat(ostream &os, const PN &pn,
        const Transition &trn) const {
    os << "EXPONENTIAL( ";
    mdepFn->PrintInCosmosFormat(os, pn);
    os << " ),1, 1, SINGLE";
}

//=============================================================================
//    GeneralPdfFn transition delay
//=============================================================================

bool GeneralPdfFn::IsMarkingDep() const {
    return false;
}

double GeneralPdfFn::EvaluateConstantDelay() const {
    throw program_exception("EvaluateConstantDelay() not supported for general PDF functions.");
}

double GeneralPdfFn::EvaluateDelay(const PN &pn, const SparseMarking &mark,
                                   const Transition &trn) const {
    throw program_exception("EvaluateDelay() not supported for general PDF functions.");
}

void GeneralPdfFn::Print(ostream &os, bool expandParamNames) const {
    genFn->Print(os, expandParamNames);
    // os << " (GENERAL)";
}

void GeneralPdfFn::PrintInMarking(ostream &os, const PN &pn,
                                  const SparseMarking &mark,
                                  const Transition &trn) const {
    os << "(gen: ";
    genFn->Print(os, true);
    os << ")";
}

bool GeneralPdfFn::IsPDF() const {
    return true;
}

void GeneralPdfFn::PrintInSPNicaFormat(ostream &os, const PN &pn,
                                       const Transition &trn) const {
    genFn->PrintInSPNicaFormat(os, pn);
}

void GeneralPdfFn::PrintInCosmosFormat(ostream &os, const PN &pn,
                                       const Transition &trn) const {
    throw program_exception("Not supported");
}

//=============================================================================

shared_ptr<MarkDepDelayFn>
NewMultipleServerDelayFn(size_t numSrv, const shared_ptr<Expr<double> > &delay) {
    return make_shared<MultipleServerDelayFn>(numSrv, delay);
}

shared_ptr<MarkDepDelayFn>
NewLoadDependentDelayFn(const vector<double> &delays) {
    return make_shared<LoadDependentDelayFn>(delays);
}

shared_ptr<MarkDepDelayFn>
NewMarkingDependentDelayFn(const shared_ptr<MdepExpr<double> > &mdepFn) {
    return make_shared<MarkingDependentDelayFn>(mdepFn);
}

shared_ptr<MarkDepDelayFn> 
NewGeneralPdfFn(const shared_ptr<Expr<double>> &fg, const char* expr) {
    return make_shared<GeneralPdfFn>(fg, expr);
}

//=============================================================================










//=============================================================================
// External Parser functions
//=============================================================================

shared_ptr<Expr<int> > FindMarkPar(const char *mparName, const PN *pn) {
    map<string, markpar_t>::const_iterator mparIt;
    mparIt = pn->mparInds.find(mparName);
    if (mparIt == pn->mparInds.end()) {
        ostringstream msg;
        msg << "Marking Parameter " << mparName << " is not defined.";
        throw program_exception(msg.str());
    }
    return pn->mpars[mparIt->second].param;
}

shared_ptr<Expr<double> > FindRatePar(const char *rparName, const PN *pn) {
    map<string, ratepar_t>::const_iterator rparIt;
    rparIt = pn->rparInds.find(rparName);
    if (rparIt == pn->rparInds.end()) {
        ostringstream msg;
        msg << "Rate Parameter " << rparName << " is not defined.";
        throw program_exception(msg.str());
    }
    return pn->rpars[rparIt->second].param;
}

place_t FindPlace(const char *placeName, const PN *pn) {
    map<string, place_t>::const_iterator plcIt;
    plcIt = pn->plcInds.find(placeName);
    if (plcIt == pn->plcInds.end()) {
        ostringstream msg;
        msg << "Place \"" << placeName << "\" is not defined.";
        throw program_exception(msg.str());
    }
    return plcIt->second;
}

transition_t FindTransition(const char *trnName, const PN *pn) {
    map<string, transition_t>::const_iterator trnIt;
    trnIt = pn->trnInds.find(trnName);
    if (trnIt == pn->trnInds.end()) {
        ostringstream msg;
        msg << "Transition \"" << trnName << "\" is not defined.";
        throw program_exception(msg.str());
    }
    return trnIt->second;
}



//=============================================================================
//       Lexer helper functions
//=============================================================================

bool IsGMarkPar(const char *name, const PN *pPetriNet) {
    assert(pPetriNet != nullptr);
    return pPetriNet->mparInds.count(name) > 0;
}

bool IsGRatePar(const char *name, const PN *pPetriNet) {
    assert(pPetriNet != nullptr);
    return pPetriNet->rparInds.count(name) > 0;
}

bool IsGPlaceId(const char *name, const PN *pPetriNet) {
    assert(pPetriNet != nullptr);
    return pPetriNet->plcInds.count(name) > 0;
}

bool IsGTransitionId(const char *name, const struct PN *pPetriNet) {
    assert(pPetriNet != nullptr);
    return pPetriNet->trnInds.count(name) > 0;
}


//=============================================================================

















