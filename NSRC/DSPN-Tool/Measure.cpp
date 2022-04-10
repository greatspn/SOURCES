/*
 *  Measure.cpp
 *
 *  Implementation of the Measure processing.
 *
 *  Created by Elvio Amparore
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cfloat>
#include <climits>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

# include "../platform/platform_utils.h"

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "Language.h"
#include "ReachabilityGraph.h"
#include "PackedMemoryPool.h"
#include "NewReachabilityGraph.h"



//=============================================================================
// Preprocessed measure: Real value
//=============================================================================

void PpMeasureExpr_Real::Print(ostream &os, bool expandParamNames) const {
    os << value;
}
double PpMeasureExpr_Real::EvaluateMeasure(const PetriNetSolution &pns) {
    return value;
}
void PpMeasureExpr_Real::SetupRewardVectors(size_t numStates, RewardVectors &rv) {
    rv.rateRewards.resize(numStates);
    set_vec(rv.rateRewards, value);
    rv.impulseRewards.resize(numStates);
    set_vec(rv.impulseRewards, value);
}

//=============================================================================
// Preprocessed measure: reward vectors
//=============================================================================

void PpMeasureExpr_Reward::Print(ostream &os, bool expandParamNames) const {
    os << "{ RR" << print_vec_ublas(rateReward) << ", ";
    os << "IR" << print_vec_ublas(impulseReward) << " }";
}
double PpMeasureExpr_Reward::EvaluateMeasure(const PetriNetSolution &pns) {
    if (pns.stateProbs.size() != rateReward.size() ||
            (pns.firingFreq.size() != 0 && pns.firingFreq.size() != impulseReward.size()))
        throw program_exception("Internal error in PpMeasureExpr_Reward::EvaluateMeasure.");
    double reward = 0.0;

    // Multiply the state probability vector with the rate rewards, and
    // the firing frequencies vector with the impulse reward vector.
    ublas::compressed_vector<double>::const_iterator it, itE;
    for (it = rateReward.begin(), itE = rateReward.end(); it != itE; ++it)
        reward += (*it) * pns.stateProbs(it.index());
    if (impulseReward.size() != 0 && pns.firingFreq.size() > 0)
        for (it = impulseReward.begin(), itE = impulseReward.end(); it != itE; ++it)
            reward += (*it) * pns.firingFreq(it.index());

    return reward;
}
void PpMeasureExpr_Reward::SetupRewardVectors(size_t numStates, RewardVectors &rv) {
    rv.rateRewards = rateReward;
    rv.impulseRewards = impulseReward;
}

//=============================================================================
// Preprocessed measure: Binary operator
//=============================================================================

void PpMeasureExpr_Binary::Print(ostream &os, bool expandParamNames) const {
    expr1->Print(os, expandParamNames);
    os << " " << char(op) << " ";
    expr2->Print(os, expandParamNames);
}
double PpMeasureExpr_Binary::EvaluateMeasure(const PetriNetSolution &pns) {
    double r1 = expr1->EvaluateMeasure(pns);
    double r2 = expr2->EvaluateMeasure(pns);
    switch (op) {
    case '+':	return r1 + r2;
    case '-':	return r1 - r2;
    case '*':	return r1 * r2;
    case '/':	return r1 / r2;
    default:
        throw program_exception("Internal error in PpMeasureExpr_Binary::EvaluateMeasure.");
    }
}
void PpMeasureExpr_Binary::SetupRewardVectors(size_t numStates, RewardVectors &rv) {
    RewardVectors rv1, rv2;
    expr1->SetupRewardVectors(numStates, rv1);
    expr2->SetupRewardVectors(numStates, rv2);
    rv.rateRewards.resize(numStates);
    rv.impulseRewards.resize(numStates);
    for (size_t i = 0; i < numStates; i++) {
        double rr1 = rv1.rateRewards[i], rr2 = rv2.rateRewards[i], rrf;
        double ir1 = rv1.impulseRewards[i], ir2 = rv2.impulseRewards[i], irf;
        switch (op) {
        case '+':	rrf = rr1 + rr2;   irf = ir1 + ir2;   break;
        case '-':	rrf = rr1 - rr2;   irf = ir1 - ir2;   break;
        case '*':	rrf = rr1 * rr2;   irf = ir1 * ir2;   break;
        case '/':	rrf = rr1 / rr2;   irf = ir1 / ir2;   break;
        default:
            throw program_exception("Internal error in PpMeasureExpr_Binary::EvaluateMeasure.");
        }
        rv.rateRewards[i] = rrf;
        rv.impulseRewards[i] = irf;
    }
}

//=============================================================================




//=============================================================================
// Measure expression: real constant value
//=============================================================================

shared_ptr<PpMeasureExpr> MeasureExpr_Real::Preprocess(const PN &pn, const RG &rg) {
    return make_shared<PpMeasureExpr_Real>(value->Evaluate());
}
shared_ptr<PpMeasureExpr> MeasureExpr_Real::Preprocess(const PN &pn, const RG2 &rg) {
    return make_shared<PpMeasureExpr_Real>(value->Evaluate());
}
void MeasureExpr_Real::Print(ostream &os, bool expandParamNames) const {
    value->Print(os, expandParamNames);
}
void MeasureExpr_Real::PrintInSPNicaFormat(ostream &os, const PN &pn) const {
    value->PrintInSPNicaFormat(os, pn);
}

//=============================================================================
// Measure expression: P{cond}  => reward is the probability that *cond is true
//=============================================================================

shared_ptr<PpMeasureExpr> MeasureExpr_P::Preprocess(const PN &pn, const RG &rg) {
    const size_t N = rg.markSet.size();
    ublas::compressed_vector<double> rateRew(N), impulseRew(N);
    // Prepare the rate/impulse reward vectors
    for (size_t i = 0; i < N; i++) {
        if (cond->EvaluateInMarking(rg.markSet[i].marking))
            rateRew[i] = 1.0;
    }
    return shared_ptr<PpMeasureExpr>(new PpMeasureExpr_Reward(rateRew, impulseRew));
}

shared_ptr<PpMeasureExpr> MeasureExpr_P::Preprocess(const PN &pn, const RG2 &rg) {
    const size_t N = rg.NumStates();
    ublas::compressed_vector<double> rateRew(N), impulseRew(N);
    // Prepare the rate/impulse reward vectors
    RgState rs;
    for (size_t i = 0; i < N; i++) {
        GetIthState(rg, i, rs);
        if (cond->EvaluateInMarking(rs.marking))
            rateRew[i] = 1.0;
    }
    return shared_ptr<PpMeasureExpr>(new PpMeasureExpr_Reward(rateRew, impulseRew));
}

void MeasureExpr_P::Print(ostream &os, bool expandParamNames) const {
    os << "P{ ";
    cond->Print(os, expandParamNames);
    os << " }";
}

void MeasureExpr_P::PrintInSPNicaFormat(ostream &os, const PN &pn) const {
    os << "If[";
    cond->PrintInSPNicaFormat(os, pn);
    os << " > 0, 1, 0]";
}

//=============================================================================
// Measure expression: E{place/cond}  => reward is a token count with cond
//=============================================================================

shared_ptr<PpMeasureExpr> MeasureExpr_E::Preprocess(const PN &pn, const RG &rg) {
    const size_t N = rg.markSet.size();
    ublas::compressed_vector<double> rateRew(N), impulseRew(N);
    // Prepare the rate/impulse reward vectors
    for (size_t i = 0; i < N; i++) {
        if (cond == nullptr || cond->EvaluateInMarking(rg.markSet[i].marking))
            rateRew[i] = place->EvaluateInMarking(rg.markSet[i].marking);
    }
    return shared_ptr<PpMeasureExpr>(new PpMeasureExpr_Reward(rateRew, impulseRew));
}

shared_ptr<PpMeasureExpr> MeasureExpr_E::Preprocess(const PN &pn, const RG2 &rg) {
    const size_t N = rg.NumStates();
    ublas::compressed_vector<double> rateRew(N), impulseRew(N);
    // Prepare the rate/impulse reward vectors
    RgState rs;
    for (size_t i = 0; i < N; i++) {
        GetIthState(rg, i, rs);
        if (cond == nullptr || cond->EvaluateInMarking(rs.marking))
            rateRew[i] = place->EvaluateInMarking(rs.marking);
    }
    return shared_ptr<PpMeasureExpr>(new PpMeasureExpr_Reward(rateRew, impulseRew));
}

void MeasureExpr_E::Print(ostream &os, bool expandParamNames) const {
    os << "E{ ";
    place->Print(os, expandParamNames);
    if (cond != nullptr) {
        os << " / ";
        cond->Print(os, expandParamNames);
    }
    os << " }";
}

void MeasureExpr_E::PrintInSPNicaFormat(ostream &os, const PN &pn) const {
    if (cond == nullptr) {
        place->PrintInSPNicaFormat(os, pn);
    }
    else {
        os << "If[";
        cond->PrintInSPNicaFormat(os, pn);
        os << " != 0, ";
        place->PrintInSPNicaFormat(os, pn);
        os << ", 0]";
    }
}

//=============================================================================
// Throughput expression: mult * X{...}  => Impulse reward of (timed) transition
//=============================================================================

shared_ptr<PpMeasureExpr> MeasureExpr_X::Preprocess(const PN &pn, const RG &rg) {
    const size_t N = rg.markSet.size();
    ublas::compressed_vector<double> rateRew(N), impulseRew(N);
    if (pn.trnInds.count(trnName) == 0)
        throw program_exception("MeasureExpr_IR::Preprocess: transition "
                                "name does not exist.");
    transition_t trnInd = pn.trnInds.find(trnName)->second;
    const Transition &trn = pn.trns[trnInd];
    static bool g_onetime_warning_immed = true;
    if (trn.isImmediate() && g_onetime_warning_immed) {
        cout << console::beg_emph() << "Throughput of immediate transitions ";
        cout << "is still experimental. Check the results..." << console::end_emph() << endl;
        g_onetime_warning_immed = false;
    }
    // Prepare the rate/impulse reward vectors
    for (size_t i = 0; i < N; i++) {
        if (rg.markSet[i].isVanishing)
            continue;
        if (cond != nullptr && !cond->EvaluateInMarking(rg.markSet[i].marking))
            continue;

        const ReachableMarking &rm = rg.markSet[i];
        if (trn.isImmediate()) {
            vector<RGArc>::const_iterator arcIt = rm.timedPaths.begin();
            vector<RGArc>::const_iterator arcItE = rm.timedPaths.end();
            for (; arcIt != arcItE; ++arcIt) {
                double immFiringInArc = 0.0;
                list<VanishingPath>::const_iterator vpIt = arcIt->vanishingPaths.begin();
                list<VanishingPath>::const_iterator vpItE = arcIt->vanishingPaths.end();
                for (; vpIt != vpItE; ++vpIt) {
                    double pathProb = vpIt->pathProb; // Firing sequence probability
                    list<VanishingMarking>::const_iterator vmIt = vpIt->path.begin();
                    list<VanishingMarking>::const_iterator vmItE = vpIt->path.end();
                    for (; vmIt != vmItE; ++vmIt) {
                        // Count each repetition of trnInd in the firing sequence
                        if (vmIt->immTrnInd == trnInd)
                            immFiringInArc += pathProb;
                    }
                }
                if (immFiringInArc > 0) {
                    transition_t firedTimedTrnInd = arcIt->timedTrnInd;
                    const Transition &timedTrn = pn.trns[firedTimedTrnInd];
                    if (timedTrn.isGeneral())
                        impulseRew[i] += immFiringInArc;
                    else { // exponential
                        assert(timedTrn.distrib == EXP);
                        rateRew[i] += arcIt->delay * immFiringInArc;
                    }
                }
            }
        }
        else {
            vector<RGArc>::const_iterator arcIt = rm.timedPaths.begin();
            vector<RGArc>::const_iterator arcItE = rm.timedPaths.end();
            for (; arcIt != arcItE; ++arcIt) {
                if (arcIt->timedTrnInd == trnInd) {
                    if (trn.isGeneral())
                        impulseRew[i] = 1;
                    else
                        rateRew[i] = arcIt->delay;
                }
            }
        }
    }
    return shared_ptr<PpMeasureExpr>(new PpMeasureExpr_Reward(rateRew, impulseRew));
}

shared_ptr<PpMeasureExpr> MeasureExpr_X::Preprocess(const PN &pn, const RG2 &rg) {
    const size_t N = rg.NumStates();
    ublas::compressed_vector<double> rateRew(N), impulseRew(N);
    if (pn.trnInds.count(trnName) == 0)
        throw program_exception("MeasureExpr_IR::Preprocess: transition "
                                "name does not exist.");
    transition_t trnInd = pn.trnInds.find(trnName)->second;
    const Transition &trn = pn.trns[trnInd];
    static bool g_onetime_warning_immed2 = true;
    if (trn.isImmediate() && g_onetime_warning_immed2) {
        cout << console::beg_emph() << "Throughput of immediate transitions ";
        cout << "is still experimental. Check the results..." << console::end_emph() << endl;
        g_onetime_warning_immed2 = false;
    }
    // Prepare the rate/impulse reward vectors
    RgState rs;
    RgEdge edge;
    for (size_t i = 0; i < N; i++) {
        GetIthState(rg, i, rs);
        if (rs.selfIndex.type() == VANISHING)
            continue;
        if (cond != nullptr && !cond->EvaluateInMarking(rs.marking))
            continue;

        rgedge_t edgeList = rg.stateSets[rs.selfIndex.type()].get_first_edge(rs.selfIndex.index());
        while (edgeList != INVALID_RGEDGE_INDEX) {
            GetEdgeByIndex(rg, edgeList, edge);

            if (trn.isImmediate()) {
                double immFiringInArc = 0.0;
                vanish_path_set_table::const_seq_iterator vpsIt, vpsItE;
                vpsIt = rg.vanPathSetTbl.begin_seq(edge.pathSet);
                vpsItE = rg.vanPathSetTbl.end();
                for (; vpsIt != vpsItE; ++vpsIt) {
                    vanish_path_id vpID = *vpsIt;
                    double pathProb = 1.0; // Firing sequence probability
                    size_t repCnt = 0;
                    vanish_path_table::const_seq_iterator vpIt, vpItE;
                    vpIt = rg.vanPathTbl.begin_seq(vpID);
                    vpItE = rg.vanPathTbl.end();
                    for (; vpIt != vpItE; ++vpIt) {
                        // Count each repetition of trnInd in the firing sequence
                        if (vpIt->immFiredTrn == trnInd)
                            repCnt++;
                        pathProb *= vpIt->prob;
                    }
                    immFiringInArc += pathProb * repCnt;
                }
                if (immFiringInArc > 0) {
                    transition_t firedTimedTrnInd = edge.firedTrn;
                    const Transition &timedTrn = pn.trns[firedTimedTrnInd];
                    if (timedTrn.isGeneral())
                        impulseRew[i] += immFiringInArc;
                    else { // exponential
                        assert(timedTrn.distrib == EXP);
                        rateRew[i] += edge.delay * immFiringInArc;
                    }
                }
            }
            else {
                if (edge.firedTrn == trnInd) {
                    if (trn.isGeneral())
                        impulseRew[i] = 1;
                    else
                        rateRew[i] = edge.delay;
                }
            }
            edgeList = edge.nextEdge;
        }
    }
    return shared_ptr<PpMeasureExpr>(new PpMeasureExpr_Reward(rateRew, impulseRew));
}

void MeasureExpr_X::Print(ostream &os, bool expandParamNames) const {
    os << "X{ " << trnName;
    if (cond != nullptr) {
        os << " / ";
        cond->Print(os, expandParamNames);
    }
    os << " }";
}

void MeasureExpr_X::PrintInSPNicaFormat(ostream &os, const PN &pn) const {
    if (cond == nullptr) {
        os << "ir[" << trnName << "]";
    }
    else {
        os << "If[";
        cond->PrintInSPNicaFormat(os, pn);
        os << " != 0, ir[" << trnName << "], 0]";
    }
}

//=============================================================================
// Measure binary operator: measure +/- measure
//=============================================================================

shared_ptr<PpMeasureExpr> MeasureExpr_Binary::Preprocess(const PN &pn, const RG &rg) {
    return make_shared<PpMeasureExpr_Binary>(expr1->Preprocess(pn, rg), op,
            expr2->Preprocess(pn, rg));
}

shared_ptr<PpMeasureExpr> MeasureExpr_Binary::Preprocess(const PN &pn, const RG2 &rg) {
    return make_shared<PpMeasureExpr_Binary>(expr1->Preprocess(pn, rg), op,
            expr2->Preprocess(pn, rg));
}

void MeasureExpr_Binary::Print(ostream &os, bool expandParamNames) const {
    expr1->Print(os, expandParamNames);
    os << " " << char(op) << " ";
    expr2->Print(os, expandParamNames);
}

void MeasureExpr_Binary::PrintInSPNicaFormat(ostream &os, const PN &pn) const {
    expr1->PrintInSPNicaFormat(os, pn);
    os << " " << char(op) << " ";
    expr2->PrintInSPNicaFormat(os, pn);
}

//=============================================================================





//=============================================================================

template<typename RG>
void PreprocessMeasuresTmpl(const PN &pn, RG &rg, VerboseLevel verboseLvl) {
    if (verboseLvl >= VL_VERBOSE)
        cout << "MEASURE PROCESSING:\n";

    const size_t N = pn.measures.size();
    rg.ppMeasures.resize(N);
    for (size_t m = 0; m < N; m++) {
        const Measure &meas = pn.measures[m];
        PpMeasure &ppMeas = rg.ppMeasures[m];

        ostringstream oss;
        meas.expr->Print(oss, false);

        ppMeas.name 	   = meas.name;
        ppMeas.printedExpr = oss.str();
        ppMeas.result      = 0.0;
        ppMeas.expr        = meas.expr->Preprocess(pn, rg);

        if (verboseLvl >= VL_VERBOSE) {
            cout << "  MEASURE " << meas.name << ": " << ppMeas.printedExpr;
            cout << endl;
            cout << "    PROCESSED AS: ";
            ppMeas.expr->Print(cout, false);
            cout << endl << endl;;
        }
    }

    if (verboseLvl >= VL_VERBOSE)
        cout << endl;
}

void PreprocessMeasures(const PN &pn, RG &rg, VerboseLevel verboseLvl) {
    PreprocessMeasuresTmpl(pn, rg, verboseLvl);
}

void PreprocessMeasures(const PN &pn, RG2 &rg, VerboseLevel verboseLvl) {
    PreprocessMeasuresTmpl(pn, rg, verboseLvl);
}

//=============================================================================

template<typename RG>
void ComputeMeasuresTmpl(RG &rg, const PetriNetSolution &sol, VerboseLevel verboseLvl) {
    if (verboseLvl >= VL_BASIC)
        cout << "\n";

    for (size_t m = 0; m < rg.ppMeasures.size(); m++) {
        PpMeasure &meas = rg.ppMeasures[m];

        meas.result = meas.expr->EvaluateMeasure(sol);

        if (verboseLvl >= VL_BASIC) {
            cout << "MEASURE: " << meas.name << ": " << meas.printedExpr;
            cout << " = " << fmtdbl(meas.result, "%.10lf") << endl;
        }
        if (invoked_from_gui()) {
            cout << "#{GUI}# RESULT " << meas.name << " = "
                 << fmtdbl(meas.result, "%.10lf") << endl;
        }
    }

    if (verboseLvl >= VL_BASIC)
        cout << "\n";
}

void ComputeMeasures(RG &rg, const PetriNetSolution &sol, VerboseLevel verboseLvl) {
    ComputeMeasuresTmpl(rg, sol, verboseLvl);
}

void ComputeMeasures(RG2 &rg, const PetriNetSolution &sol, VerboseLevel verboseLvl) {
    ComputeMeasuresTmpl(rg, sol, verboseLvl);
}

//=============================================================================

template<typename RG>
void GetMeasureRewardVectorsTmpl(RG &rg, const char *measureName, RewardVectors &rv) {
    for (size_t m = 0; m < rg.ppMeasures.size(); m++) {
        PpMeasure &meas = rg.ppMeasures[m];
        if (meas.name == measureName) {
            meas.expr->SetupRewardVectors(rg.NumStates(), rv);
            return;
        }
    }
    throw program_exception("No measure exists with the specified name.");
}

void GetMeasureRewardVectors(RG &rg, const char *measureName, RewardVectors &rv) {
    GetMeasureRewardVectorsTmpl(rg, measureName, rv);
}
void GetMeasureRewardVectors(RG2 &rg, const char *measureName, RewardVectors &rv) {
    GetMeasureRewardVectorsTmpl(rg, measureName, rv);
}

//=============================================================================

template<typename RG>
void SaveMeasureValuesOnFileTmpl(RG &rg, ofstream &res, VerboseLevel verboseLvl) {
    for (size_t m = 0; m < rg.ppMeasures.size(); m++) {
        res << fmtdbl(rg.ppMeasures[m].result, "%.10lf") << "\n";
    }
    res << flush;
    if (!res)
        throw program_exception("Couldn't write measures to file.");
}

void SaveMeasureValuesOnFile(RG &rg, ofstream &res, VerboseLevel verboseLvl) {
    SaveMeasureValuesOnFileTmpl(rg, res, verboseLvl);
}

void SaveMeasureValuesOnFile(RG2 &rg, ofstream &res, VerboseLevel verboseLvl) {
    SaveMeasureValuesOnFileTmpl(rg, res, verboseLvl);
}

//=============================================================================

void SaveStateProbsOnFile(const PetriNetSolution &sol, ofstream &res, VerboseLevel verboseLvl) {
    PrintUblasVector(res, sol.stateProbs);
    // res << '['<< sol.stateProbs.size() << "](";
    // for (size_t m=0; m<sol.stateProbs.size(); m++) {
    // 	res << fmtdbl(sol.stateProbs[m], "%.10lf") << (m==sol.stateProbs.size()-1 ? ")" : ",");
    // }
    // res << "\n" << flush;
    // if (!res)
    // 	throw program_exception("Couldn't write state probabilities to file.");
}

//=============================================================================

void CompareMeasuresAgainstFile(const PetriNetSolution &sol, ifstream &wrf,
                                double epsilon) {
    ublas::vector<double> cmpRateReward;
    wrf >> cmpRateReward;
    if (!wrf)
        throw program_exception("The comparison file is damaged");
    if (cmpRateReward.size() != sol.stateProbs.size())
        throw program_exception("Comparison rate reward vector has a different "
                                "number of states.");

    // Take into account a possible permutation of the input probabilities
    const size_t N = sol.stateProbs.size();
    vector<bool> usedVal(N, false);
    for (size_t i = 0; i < N; i++) {
        // Find a possible match in cmpRateReward
        bool matchFound = false;
        for (size_t m = 0; m < N; m++) {
            if (!usedVal[m] &&
                    fabs(sol.stateProbs[i] - cmpRateReward[m]) < epsilon) {
                cout << "  VERIFYING: ";
                cout << right << setw(12) << sol.stateProbs[i] << " <=> ";
                cout << cmpRateReward[m] << endl;
                usedVal[m] = true;
                matchFound = true;
                break;
            }
        }
        if (!matchFound) {
            cout << "Probability " << i + 1 << " failed comparison ";
            cout << "test.\n Computed=" << sol.stateProbs[i];
            cout << ", stored=" << cmpRateReward << ".";
            throw program_exception("Probability comparison test failed.");
        }
    }
}

//=============================================================================

// Compute all the basic Petri net measures for places and transitions
void ComputeAllBasicMeasures(const PN &pn, const RG2 &rg, const PetriNetSolution &pns) {

    cout << "COMPUTING BASIC PETRI NET MEASURES." << endl;

    // Average number of tokens in a place
    for (const Place &pl : pn.plcs) {
        shared_ptr<MdepExpr<int>> markP; // #Place
        markP = make_shared<PlaceMarkingMdepExpr<int>>(pl.name, pl.index);

        shared_ptr<MeasureExpr> meanP;   // E{ #Place }
        meanP = make_shared<MeasureExpr_E>(markP, nullptr);

        shared_ptr<PpMeasureExpr> pp = meanP->Preprocess(pn, rg);
        double value = pp->EvaluateMeasure(pns);

        if (invoked_from_gui())
            cout << "#{GUI}# RESULT ALL MEAN " << pl.name << " = " << value << endl;
        else
            cout << "E{" << pl.name << "} = " << value << endl;
    }

    // Token distribution of each place
    std::vector<double> distrib;
    for (const Place &pl : pn.plcs) {
        const size_t N = rg.NumStates();
        RgState rs;
        for (size_t i = 0; i < N; i++) {
            GetIthState(rg, i, rs);
            tokencount_t n_tokens = rs.marking[pl.index];
            if (distrib.size() <= size_t(n_tokens))
                distrib.resize(size_t(n_tokens) + 1, 0.0);
            distrib[n_tokens] += pns.stateProbs[i];
        }
        if (invoked_from_gui()) {
            cout << "#{GUI}# RESULT ALL DISTRIB " << pl.name << " " << distrib.size() << " = ";
            for (double v : distrib)
                cout << " " << v;
            cout << endl;
        }
        else
            cout << "Distrib{" << pl.name << "} = " << print_vec(distrib) << endl;
        distrib.resize(0);
    }

    // Average transition throughputs
    for (const Transition &trn : pn.trns) {
        shared_ptr<MeasureExpr> tput; // X{ trn }
        tput = make_shared<MeasureExpr_X>(trn.name.c_str(), nullptr);

        shared_ptr<PpMeasureExpr> pp = tput->Preprocess(pn, rg);
        double value = pp->EvaluateMeasure(pns);

        if (invoked_from_gui())
            cout << "#{GUI}# RESULT ALL THROUGHPUT " << trn.name << " = " << value << endl;
        else
            cout << "X{" << trn.name << "} = " << value << endl;
    }

    cout << endl;
}

//=============================================================================

// Descriptor of the basic Petri net properties, for a given solution vector
struct BasicMeasures {
    // Place bounds
    vector<int> p_bounds;
    // Place means (expected token count)
    vector<double>  p_means;
    // Place distributions
    vector<vector<double>> p_distr;
    // Transition throughputs
    vector<double>  throughputs;
};

//=============================================================================

// Compute all the basic Petri net measures for places and transitions
void ComputeBasicMeasures(const PN &pn, const RG2 &rg, const PetriNetSolution &pns,
                          BasicMeasures &bm, VerboseLevel verboseLvl) {
    if (verboseLvl >= VL_BASIC)
        cout << "COMPUTING BASIC PETRI NET MEASURES." << endl;

    // Compute place bounds
    bm.p_bounds.resize(pn.plcs.size(), 0);
    const size_t N = rg.NumStates();
    RgState rs;
    for (size_t i = 0; i < N; i++) {
        GetIthState(rg, i, rs);

        for (auto mit = rs.marking.begin(); mit != rs.marking.end(); ++mit) {
            bm.p_bounds[mit->index] = std::max(bm.p_bounds[mit->index], (int)mit->value);
        }
    }

    // Allocate memory for place/transitions measures
    bm.p_means.resize(pn.plcs.size(), 0.0);
    bm.p_distr.resize(pn.plcs.size());
    for (size_t i = 0; i < pn.plcs.size(); i++)
        bm.p_distr[i].resize(bm.p_bounds[i] + 1);
    bm.throughputs.resize(pn.trns.size());

    RgEdge edge;
    for (size_t i = 0; i < N; i++) {
        GetIthState(rg, i, rs);
        for (auto mit = rs.marking.begin(); mit != rs.marking.end(); ++mit) {
            // Compute place measures
            bm.p_means[mit->index] += pns.stateProbs[i] * mit->value;
            bm.p_distr[mit->index][mit->value] += pns.stateProbs[i];

            // Compute transition throughputs
            if (rs.selfIndex.type() == VANISHING)
                continue;
            rgedge_t edgeList = rg.stateSets[rs.selfIndex.type()].get_first_edge(rs.selfIndex.index());
            while (edgeList != INVALID_RGEDGE_INDEX) {
                GetEdgeByIndex(rg, edgeList, edge);
                const Transition &trn = pn.trns[edge.firedTrn];
                assert(trn.isTimed());

                // Get the timed edge throughput
                double reward;
                if (trn.isGeneral()) {
                    if (pns.firingFreq.size() > 0)
                        reward = pns.firingFreq[i]/* * edge.delay*/;
                    else
                        reward = 0.0; // Missing firing frequencies in the solution vector.
                }
                else { // exponential
                    assert(trn.distrib == EXP);
                    reward = pns.stateProbs[i] * edge.delay;
                }
                bm.throughputs[trn.index] += reward;

                // Get the vanishing paths
                vanish_path_set_table::const_seq_iterator vpsIt, vpsItE;
                vpsIt = rg.vanPathSetTbl.begin_seq(edge.pathSet);
                vpsItE = rg.vanPathSetTbl.end();
                for (; vpsIt != vpsItE; ++vpsIt) {
                    vanish_path_id vpID = *vpsIt;
                    double pathProb = 1.0; // Firing sequence probability

                    vanish_path_table::const_seq_iterator vpIt, vpItE;
                    vpIt = rg.vanPathTbl.begin_seq(vpID);
                    vpItE = rg.vanPathTbl.end();
                    for (; vpIt != vpItE; ++vpIt) {
                        pathProb *= vpIt->prob;
                        // Immediate edge throughput
                        bm.throughputs[vpIt->immFiredTrn] += reward * pathProb;
                    }
                }

                // if (trn.isImmediate()) {
                // 	double immFiringInArc = 0.0;
                // 	vanish_path_set_table::const_seq_iterator vpsIt, vpsItE;
                // 	vpsIt = rg.vanPathSetTbl.begin_seq(edge.pathSet);
                // 	vpsItE = rg.vanPathSetTbl.end();
                // 	for (; vpsIt != vpsItE; ++vpsIt) {
                // 		vanish_path_id vpID = *vpsIt;
                // 		double pathProb = 1.0; // Firing sequence probability
                // 		size_t repCnt = 0;
                // 		vanish_path_table::const_seq_iterator vpIt, vpItE;
                // 		vpIt = rg.vanPathTbl.begin_seq(vpID);
                // 		vpItE = rg.vanPathTbl.end();
                // 		for (; vpIt != vpItE; ++vpIt) {
                // 			// Count each repetition of trnInd in the firing sequence
                // 			if (vpIt->immFiredTrn == trnInd)
                // 				repCnt++;
                // 			pathProb *= vpIt->prob;
                // 		}
                // 		immFiringInArc += pathProb * repCnt;
                // 	}
                // 	if (immFiringInArc > 0) {
                // 		transition_t firedTimedTrnInd = edge.firedTrn;
                // 		const Transition& timedTrn = pn.trns[firedTimedTrnInd];
                // 		if (timedTrn.isGeneral())
                // 			impulseRew[i] += immFiringInArc;
                // 		else { // exponential
                // 			assert(timedTrn.distrib == EXP);
                // 			rateRew[i] += edge.delay * immFiringInArc;
                // 		}
                // 	}
                // }
                // else {
                // 	if (edge.firedTrn == trnInd) {
                // 		if (trn.isGeneral())
                // 			impulseRew[i] = 1;
                // 		else
                // 			rateRew[i] = edge.delay;
                // 	}
                // }


                edgeList = edge.nextEdge;
            }
        }
    }
}

//=============================================================================







