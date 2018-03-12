/*
 *  MRP.cpp
 *
 *  Implementation of the Markov Regenerative Process basic functions.
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
#include <cmath>
#include <cfloat>
#include <ctime>
#include <climits>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <exception>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <functional>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "ReachabilityGraph.h"
#include "PackedMemoryPool.h"
#include "NewReachabilityGraph.h"
#include "MRP.h"

//=============================================================================

double DeriveSimpleDeterministicDelay(const char* fgx);

//=============================================================================

void ExtractMRP(const RG &rg, MRP &mrp, VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_MarkovRenewalProcess)
        throw program_exception("Call ExtractMRP only for MRPs");

    const size_t N = rg.markSet.size();

    numerical::matrix_map<double> Q(N, N);
    numerical::matrix_map<double> Qbar(N, N);
    numerical::matrix_map<double> Delta(N, N);

    // A general state partition has a single fixed delay in every state
    map< transition_t, double > numGenSubsets;
    vector<double> allDetDelays(N, -1.0);

    // Fill in the generator matrices Q, Qbar and Delta
    vector<ReachableMarking>::const_iterator rmIt;
    for (rmIt = rg.markSet.begin(); rmIt != rg.markSet.end(); ++rmIt) {
        size_t i = size_t(rmIt->index);
        double Qii = 0.0, detDelay = -1;
        vector<RGArc>::const_iterator arcIt = rmIt->timedPaths.begin();
        for (; arcIt != rmIt->timedPaths.end(); ++arcIt) {
            size_t j = size_t(arcIt->destMarking);
            if (arcIt->distrib == DET) {
                // Store separately the branching probability and the delay
                Delta(i, j) += arcIt->prob;
                detDelay = arcIt->delay;

                assert(allDetDelays[rmIt->index] == -1.0 ||
                       allDetDelays[rmIt->index] == detDelay);
                allDetDelays[rmIt->index] = detDelay;
            }
            else {
                double rate = arcIt->delay * arcIt->prob;
                if (arcIt->preempted.size() != 0)
                    Qbar(i, j) += rate;
                else
                    Q(i, j) += rate;
                Qii += rate;
            }
        }
        Q(i, i) += -Qii;

        // Count the dimensions of the state space subset
        if (rmIt->enabledDetInds.size() != 0) {
            assert(rmIt->enabledDetInds.size() == 1);
            assert(detDelay != -1.0);
            transition_t enabledDetInd = *rmIt->enabledDetInds.begin();
            if (numGenSubsets.count(enabledDetInd) == 0)
                numGenSubsets[enabledDetInd] = detDelay;
            else {
                if (numGenSubsets[enabledDetInd] != detDelay)
                    throw program_exception
                    ("Inside a S^Det subsets the deterministic transition"
                     "delay should be fixed.");
            }
        }
    }
    assert(numGenSubsets.size() > 0);

    // Compress the 3 mapped matrices
    compress_matrix(mrp.Q, Q);
    compress_matrix(mrp.Qbar, Qbar);
    compress_matrix(mrp.Delta, Delta);

    // Set up the initial vector pi0
    mrp.pi0.resize(N);
    fill(mrp.pi0.begin(), mrp.pi0.end(), 0.0);
    vector<RGArc>::const_iterator arcIt = rg.initMark.timedPaths.begin();
    for (; arcIt != rg.initMark.timedPaths.end(); ++arcIt)
        mrp.pi0[arcIt->destMarking] += arcIt->prob;

    // Prepare the state space partitions
    vector<statesubset_t> markToSubset;
    markToSubset.resize(N);

    mrp.Gen.resize(numGenSubsets.size());

    // Iterate over each combination of <genTrnInd, delay> pairs
    map< transition_t, double >::const_iterator udt;
    statesubset_t detSubsetCnt = 0;
    map< pair<transition_t, double>, statesubset_t> remapDetInds;
    for (udt = numGenSubsets.begin(); udt != numGenSubsets.end(); ++udt) {
        GenMarkingSubset &gms = mrp.Gen[detSubsetCnt];
        gms.index     = detSubsetCnt;
        gms.genTrnInd = udt->first;
        gms.fg        = rg.generalFn.at(gms.genTrnInd);
        gms.detDelay  = DeriveSimpleDeterministicDelay(gms.fg.c_str());
        remapDetInds[ *udt ] = detSubsetCnt;
        detSubsetCnt++;
    }
    for (rmIt = rg.markSet.begin(); rmIt != rg.markSet.end(); ++rmIt) {
        if (rmIt->enabledDetInds.size() == 0) {
            // Add in the exponential states set
            markToSubset[rmIt->index] = EXP_SUBSET;
        }
        else {
            // Add *rmIt in the proper Deterministic states subset
            transition_t detTrn = *rmIt->enabledDetInds.begin();
            double detDelay = allDetDelays[rmIt->index];
            assert(detDelay >= 0 || detDelay == GENERAL_DELAY);
            statesubset_t remappedInd = remapDetInds[make_pair(detTrn, detDelay)];

            // Verify that this state is not connected with other Det states
            // with different delays (i.e. the delay of the deterministic
            // transition must be fixed inside a single S^Det subset)
            vector<RGArc>::const_iterator arcIt = rmIt->timedPaths.begin();
            for (; arcIt != rmIt->timedPaths.end(); ++arcIt) {
                // We are interested only in exponential non-preempting arcs
                if (arcIt->distrib == EXP && arcIt->preempted.size() == 0) {
                    marking_t destMark = arcIt->destMarking;
                    assert(allDetDelays[destMark] > 0 || allDetDelays[destMark]==GENERAL_DELAY);
                    assert(rg.markSet[destMark].enabledDetInds.size() == 1);
                    assert(*(rg.markSet[destMark].enabledDetInds.begin()) == detTrn);

                    transition_t destDetTrn = *(rg.markSet[destMark].enabledDetInds.begin());
                    double destDelay = allDetDelays[arcIt->destMarking];
                    assert(destDetTrn == detTrn);
                    assert(remapDetInds.count(make_pair(destDetTrn, destDelay)) > 0);
                    if (remappedInd != remapDetInds[make_pair(destDetTrn, destDelay)])
                        throw program_exception
                        ("Inside a S^Det subsets the deterministic transition"
                         "delay should be fixed.");
                }
            }
            markToSubset[rmIt->index] = remappedInd;
        }
    }
    mrp.Subsets.ConstructSubsetList(mrp.Gen.size() + 2, markToSubset);

    if (verboseLvl >= VL_VERBOSE)
        PrintMRP(mrp);
}

//=============================================================================

inline static void skip_spaces(istream& is) {
    while (is && isspace(is.peek()))
        is.get();
}

// Derives the deterministic delay of a f_g(X)=I[delay]
// For non-determnistic general functions, returns a negative value
double DeriveSimpleDeterministicDelay(const char* fgx) {
    istringstream iss(fgx);
    skip_spaces(iss);
    if (iss.get() != 'I')
        return GENERAL_DELAY;
    skip_spaces(iss);
    if (iss.get() != '[')
        return GENERAL_DELAY;
    double delay = GENERAL_DELAY;
    iss >> delay;
    if (delay < 0)
        return GENERAL_DELAY;
    skip_spaces(iss);
    if (iss.get() != ']')
        return GENERAL_DELAY;
    skip_spaces(iss);
    if (!iss.eof())
        return GENERAL_DELAY;
    return delay;
}

//=============================================================================

string DeterministicDurationFg(double delay) {
    ostringstream oss;
    oss << "I[" << setprecision(numeric_limits<double>::digits10) << delay << "]";
    return oss.str();
}

//=============================================================================

void ExtractMRP(const RG2 &rg, MRP &mrp, VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_MarkovRenewalProcess)
        throw program_exception("Call ExtractMRP only for MRPs");

    const size_t N = rg.NumStates();

    numerical::matrix_map<double> Q(N, N);
    numerical::matrix_map<double> Qbar(N, N);
    numerical::matrix_map<double> Delta(N, N);

    // A Deterministic state partition has a single fixed delay in every state
    map< transition_t, double > numGenSubsets;
    vector<double> allGenDelays(N, -1.0);

    // Fill in the generator matrices Q, Qbar and Delta
    RgEdge edge;
    for (size_t i = 0; i < N; i++) {
        // rgstate_t stateIndex(i, TANGIBLE);
        transition_set_id enabledGenSet = rg.stateSets[TANGIBLE].get_enabled_gen_set(i);
        // const StateSetEntry& sse = GetStateSetEntryByIndex(rg, stateIndex);
        bool isGenState = (enabledGenSet != EMPTY_TRANSITION_SET);

        // Take each outgoing edge and read its delay/probability
        double Q_ii = 0.0, genDelay = -1;
        rgedge_t edgeList = rg.stateSets[TANGIBLE].get_first_edge(i);
        while (edgeList != INVALID_RGEDGE_INDEX) {
            GetEdgeByIndex(rg, edgeList, edge);
            assert(edge.typeOfSrcState() == TANGIBLE && edge.typeOfDstState() == TANGIBLE);
            size_t j = edge.dest.index();
            bool isGenTrn = rg.trnDistribs[edge.firedTrn] == DET;

            if (isGenTrn) {
                // Store the branching probability and the delay separately
                Delta(i, j) += edge.prob;
                genDelay = edge.delay;

                assert(allGenDelays[i] == -1.0 || allGenDelays[i] == genDelay);
                allGenDelays[i] = genDelay;
            }
            else {
                double rate = edge.delay * edge.prob;
                bool isPreempt = edge.preempted != EMPTY_TRANSITION_SET;
                if (isPreempt)
                    Qbar(i, j) += rate;
                else
                    Q(i, j) += rate;
                Q_ii += rate;
            }
            edgeList = edge.nextEdge;
        }
        Q(i, i) -= Q_ii;

        // Verify that the general transition delay remains constant.
        if (isGenState) {
            assert(rg.trnSetTbl.get_length(enabledGenSet) == 1);
            assert(genDelay != -1);
            transition_t enabledGenInd = *rg.trnSetTbl.begin_seq(enabledGenSet);
            if (numGenSubsets.count(enabledGenInd) == 0)
                numGenSubsets[enabledGenInd] = genDelay;
            else {
                if (numGenSubsets[enabledGenInd] != genDelay)
                    throw program_exception
                    ("Inside a S^Gen subsets the general transition"
                     "delay should be fixed.");
            }
        }
    }
    assert(numGenSubsets.size() > 0);

    // Compress the 3 mapped matrices
    compress_matrix(mrp.Q, Q);
    compress_matrix(mrp.Qbar, Qbar);
    compress_matrix(mrp.Delta, Delta);

    // Set up the initial vector pi0
    mrp.pi0.resize(N);
    fill(mrp.pi0.begin(), mrp.pi0.end(), 0.0);
    for (size_t i = 0; i < rg.initStates.size(); i++)
        mrp.pi0(rg.initStates[i].state.index()) = rg.initStates[i].initProb;

    // Prepare the state space partitions
    vector<statesubset_t> markToSubset;
    markToSubset.resize(N);

    mrp.Gen.resize(numGenSubsets.size());

    // Iterate over each combination of <genTrnInd, delay> pairs
    map< transition_t, double >::const_iterator udt;
    statesubset_t genSubsetCnt = 0;
    map< pair<transition_t, double>, statesubset_t> remapGenInds;
    for (udt = numGenSubsets.begin(); udt != numGenSubsets.end(); ++udt) {
        GenMarkingSubset &gms = mrp.Gen[genSubsetCnt];
        gms.index     = genSubsetCnt;
        gms.genTrnInd = udt->first;
        gms.fg        = rg.generalFn.at(gms.genTrnInd);
        gms.detDelay  = DeriveSimpleDeterministicDelay(gms.fg.c_str());
        remapGenInds[ *udt ] = genSubsetCnt;
        genSubsetCnt++;
    }

    for (size_t i = 0; i < N; i++) {
        // rgstate_t stateIndex(i, TANGIBLE);
        // const StateSetEntry& sse = GetStateSetEntryByIndex(rg, stateIndex);
        transition_set_id enabledGenSet = rg.stateSets[TANGIBLE].get_enabled_gen_set(i);
        bool isGenState = (enabledGenSet != EMPTY_TRANSITION_SET);

        if (!isGenState) {
            // Add in the exponential states set
            markToSubset[i] = EXP_SUBSET;
        }
        else {
            // Add state i in the proper Deterministic states subset
            transition_t genTrn = *rg.trnSetTbl.begin_seq(enabledGenSet);
            double genDelay = allGenDelays[i];
            assert(genDelay >= 0 || genDelay == GENERAL_DELAY);
            statesubset_t remappedInd = remapGenInds[make_pair(genTrn, genDelay)];

            // Verify that this state is not connected with other Gen states
            // with different delays (i.e. the delay of the deterministic
            // transition must be fixed inside a single S^Gen subset)
            rgedge_t edgeList = rg.stateSets[TANGIBLE].get_first_edge(i);
            while (edgeList != INVALID_RGEDGE_INDEX) {
                GetEdgeByIndex(rg, edgeList, edge);
                assert(edge.typeOfSrcState() == TANGIBLE && edge.typeOfDstState() == TANGIBLE);
                size_t j = edge.dest.index();
                bool isGenTrn = rg.trnDistribs[edge.firedTrn] == DET;
                bool isPreempt = edge.preempted != EMPTY_TRANSITION_SET;

                // We are interested only in exponential non-preempting arcs
                if (!isGenTrn && !isPreempt) {
                    // const StateSetEntry& dest_sse = GetStateSetEntryByIndex(rg, edge.dest);
                    transition_set_id dest_enabledGenSet = rg.stateSets[TANGIBLE].get_enabled_gen_set(edge.dest.index());
                    assert(allGenDelays[j] > 0 || allGenDelays[j]==GENERAL_DELAY);
                    assert(dest_enabledGenSet != EMPTY_TRANSITION_SET);

                    transition_t destGenTrn = *rg.trnSetTbl.begin_seq(dest_enabledGenSet);
                    assert(destGenTrn == genTrn);

                    double destDelay = allGenDelays[j];
                    assert(remapGenInds.count(make_pair(destGenTrn, destDelay)) > 0);
                    if (remappedInd != remapGenInds[make_pair(destGenTrn, destDelay)])
                        throw program_exception
                        ("Inside a S^Gen subsets the deterministic transition"
                         "delay should be fixed.");
                }
                edgeList = edge.nextEdge;
            }
            markToSubset[i] = remappedInd;
        }
    }
    mrp.Subsets.ConstructSubsetList(mrp.Gen.size() + 2, markToSubset);

    if (verboseLvl >= VL_VERBOSE)
        PrintMRP(mrp);
}

//=============================================================================

void PrintMRP(const MRP &mrp) {
    if (mrp.N() < marking_t(75)) {
        cout << "MARKOV REGENERATIVE PROCESS MATRICES:\n\n";
        print_matrix(cout, mrp.Q, "Q");
        cout << endl;
        print_matrix(cout, mrp.Qbar, "Qbar");
        cout << endl;
        print_matrix(cout, mrp.Delta, "Delta");
        cout << endl;
        cout << "pi0 = " << print_vec(mrp.pi0) << endl;
    }
    cout << "EXPONENTIAL STATES SET:\n   S^Exp = ";
    cout << print_vec_1based(mrp.Subsets.beginSubset(EXP_SUBSET),
                             mrp.Subsets.endSubset(EXP_SUBSET)) << endl;
    cout << "DETERMINISTIC STATES SETS:\n";
    for (size_t d = 0; d < mrp.Gen.size(); d++) {
        cout << "   S^Gen[" << (d + 1) << "] = ";
        cout << print_vec_1based(mrp.Subsets.beginSubset(d),
                                 mrp.Subsets.endSubset(d));
        cout << "   with transition_index=";
        cout << size_t(mrp.Gen[d].genTrnInd) + 1;
        cout << ", f_g(x)=" << mrp.Gen[d].fg;
        if (mrp.Gen[d].detDelay >= 0)
            cout << ", detDelay=" << mrp.Gen[d].detDelay;
        cout << "\n";
    }
    cout << "\nSTATE SPACE PARTITIONING:\n   ";
    for (marking_t m(0); m < mrp.Subsets.numStates(); ++m) {
        if (mrp.Subsets.subsetOf(m) == EXP_SUBSET)
            cout << "X ";
        else
            cout << mrp.Subsets.subsetOf(m) << " ";
    }
    cout << "\n" << endl;
}

//=============================================================================

void ClassifyMRPStates(const MRP &mrp, MRPStateKinds &outSK,
                       VerboseLevel verboseLvl) {
    const marking_t N_MRP = mrp.N();
    outSK.kinds.resize(N_MRP);
    std::fill(outSK.kinds.begin(), outSK.kinds.end(), 0);
    outSK.numRegenStates = 0;

    // Find MRP initial states, which are regenerative points
    for (marking_t m(0); m < N_MRP; m++)
        if (!mrp.pi0.empty() && mrp.pi0(m) != 0.0)
            // TODO: gli init states devono essere anche MSK_REGENSTATE ??
            outSK.kinds[m] |= MSK_INITSTATE;

    // Find the regeneration states of the Markov Process
    for (marking_t m(0); m < N_MRP; m++) {
        if (mrp.Subsets.subsetOf(m) == EXP_SUBSET) {
            // m is an exp state, therefore it is a regeneration state.
            // Also, every other state reachable from m is a regen. state.
            // Note that only Q contains rates for EXP_SUBSET states
            outSK.kinds[m] |= MSK_REGENSTATE;

            matRow_t Q_row(mrp.Q, m);
            rowIter_t Q_ij(Q_row.begin()), Q_end(Q_row.end());
            for (; Q_ij != Q_end; ++Q_ij) {
                const marking_t j(Q_ij.index());
                outSK.kinds[j] |= MSK_REGENSTATE;
            }
            continue;
        }

        // m belongs to a deterministic subset. States reachable from m with
        // a preemptive transition are regenerative states.
        matRow_t Qbar_row(mrp.Qbar, m);
        rowIter_t Qbar_ij(Qbar_row.begin()), Qbar_end(Qbar_row.end());
        for (; Qbar_ij != Qbar_end; ++Qbar_ij) {
            const marking_t j(Qbar_ij.index());
            outSK.kinds[j] |= MSK_REGENSTATE;
        }

        // State reached with the firing of a deterministic transition
        // are regenerative states.
        matRow_t Delta_row(mrp.Delta, m);
        rowIter_t Delta_ij(Delta_row.begin()), Delta_end(Delta_row.end());
        for (; Delta_ij != Delta_end; ++Delta_ij) {
            const marking_t j(Delta_ij.index());
            outSK.kinds[j] |= MSK_REGENSTATE;
        }
    }

    // Now classify states according to their Embedded kinds
    for (marking_t m(0); m < N_MRP; m++) {
        if ((outSK.kinds[m] & MSK_REGENSTATE) == 0) {
            // A state which never becomes a regeneration point is not embedded
            outSK.kinds[m] |= MSK_EMC_NOT_EMBEDDED;
        }
        else outSK.numRegenStates++;

        // Classify all the regenerative (and initial) states
        if ((outSK.kinds[m] & (MSK_REGENSTATE | MSK_INITSTATE)) == 0)
            continue;

        bool isAbsorbing   = true;
        bool hasExpNoPreem = matrix_row_has_nonzeroes(mrp.Q, m, isAbsorbing);
        bool hasExpPreem   = matrix_row_has_nonzeroes(mrp.Qbar, m, isAbsorbing);
        bool hasDet        = matrix_row_has_nonzeroes(mrp.Delta, m, isAbsorbing);

        // Classify this EMC state according to its out transitions
        if (isAbsorbing)
            outSK.kinds[m] |= MSK_EMC_ABSORBING;
        else if (!hasDet)
            outSK.kinds[m] |= MSK_EMC_NO_DET_ENABLED;
        else if (hasDet && !hasExpPreem && !hasExpNoPreem)
            outSK.kinds[m] |= MSK_EMC_ONLY_DET_ENABLED;
        else if (hasDet && hasExpPreem && !hasExpNoPreem)
            outSK.kinds[m] |= MSK_EMC_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY;
        else
            outSK.kinds[m] |= MSK_EMC_SMC;
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "THERE ARE " << outSK.numRegenStates;
        cout << " REGENERATIVE STATES.\n" << endl;
    }
    if (verboseLvl >= VL_VERBOSE) {
        cout << "MRP STATE CLASSIFICATION:\n";
        for (marking_t m(0); m < N_MRP; m++) {
            cout << "  MRP STATE " << left << setw(3) << size_t(m) + 1 << ": ";
            cout << ((outSK.kinds[m] & MSK_REGENSTATE) != 0 ? "EMC " : "-   ");
            cout << ((outSK.kinds[m] & MSK_INITSTATE) != 0 ? "INIT " : "-    ");
            if (outSK.kinds[m] & MSK_EMC_ABSORBING)
                cout << "Absorbing State ";
            if (outSK.kinds[m] & MSK_EMC_NO_DET_ENABLED)
                cout << "No Deterministic enabled ";
            if (outSK.kinds[m] & MSK_EMC_ONLY_DET_ENABLED)
                cout << "Only Deterministic enabled ";
            if (outSK.kinds[m] & MSK_EMC_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY)
                cout << "Deterministic concurrently but not competitively enabled ";
            if (outSK.kinds[m] & MSK_EMC_SMC)
                cout << "SMC Initial State ";
            cout << endl;
        }
        cout << endl;
    }
}

//=============================================================================

void ReorderMRP(const MRP& mrpIn, MRP& mrpOut, 
                const vector<marking_t>& order, VerboseLevel verboseLvl) 
{
    const marking_t N = mrpIn.N();

    for (int k=0; k<3; k++) {
        numerical::matrix_map<double> Matrix(N, N);
        const ublas::compressed_matrix<double> *matIn;
        ublas::compressed_matrix<double> *matOut;
        switch (k) {
            case 0:     matIn = &mrpIn.Q;     matOut = & mrpOut.Q;     break;
            case 1:     matIn = &mrpIn.Qbar;  matOut = & mrpOut.Qbar;  break;
            case 2:     matIn = &mrpIn.Delta; matOut = & mrpOut.Delta; break;
        }

        for (marking_t i(0); i < N; i++) {
            matRow_t mat_row(*matIn, i);
            rowIter_t mat_ij(mat_row.begin()), row_end(mat_row.end());
            for (; mat_ij != row_end; ++mat_ij) {
                const marking_t j(mat_ij.index());
                Matrix(order[i], order[j]) = *mat_ij;
            }
        }

        compress_matrix(*matOut, Matrix);
    }

    mrpOut.Gen = mrpIn.Gen;
    mrpOut.pi0.resize(N);
    for (marking_t m(0); m < N; m++)
        mrpOut.pi0[ order[m] ] = mrpIn.pi0[m];

    vector<statesubset_t> markToSubset(N);
    for (marking_t m(0); m < N; m++)
        markToSubset[ order[m] ] = mrpIn.Subsets.subsetOf(m);
    mrpOut.Subsets.ConstructSubsetList(mrpOut.Gen.size() + 2, markToSubset);

    if (verboseLvl >= VL_VERBOSE) {
        cout << "\n\nREORDERED MRP:" << endl;
        PrintMRP(mrpOut);
    }
}

//=============================================================================

