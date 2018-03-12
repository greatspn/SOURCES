/*
 *  SolveCTMC.cpp
 *
 *  Implementation of the CTMC numerical solvers.
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
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
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
#include "SolveCTMC.h"


//=============================================================================

void ExtractCTMC(const RG &rg, CTMC &ctmc, VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("Call ExtractCTMC only for CTMCs");

    const size_t N = rg.markSet.size();
    numerical::matrix_map<double> Q(N, N);

    // Fill in the generator matrix Q
    vector<ReachableMarking>::const_iterator rmIt;
    for (rmIt = rg.markSet.begin(); rmIt != rg.markSet.end(); ++rmIt) {
        size_t i = size_t(rmIt->index);
        double Qii = 0.0;
        vector<RGArc>::const_iterator arcIt;
        for (arcIt = rmIt->timedPaths.begin(); arcIt != rmIt->timedPaths.end(); ++arcIt) {
            double rate = arcIt->delay * arcIt->prob;
            size_t j = size_t(arcIt->destMarking);
            Q(i, j) += rate;
            Qii += rate;
        }
        Q(i, i) += -Qii;
    }
    compress_matrix(ctmc.Q, Q);

    // Get the initial probability vector
    ctmc.pi0.resize(N);
    fill(ctmc.pi0.begin(), ctmc.pi0.end(), 0.0);
    for (size_t n = 0; n < rg.initMark.timedPaths.size(); n++) {
        const RGArc &arc = rg.initMark.timedPaths[n];
        ctmc.pi0(arc.destMarking) = arc.prob;
    }

    if (verboseLvl >= VL_VERBOSE && N < 50) {
        cout << "CTMC MATRIX:\n";
        print_matrix(cout, ctmc.Q, "Q");
        cout << endl;
    }
}

//=============================================================================

void ExtractCTMC(const RG2 &rg, CTMC &ctmc, VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("Call ExtractCTMC only for CTMCs");

    const size_t N = rg.NumStates();
    numerical::matrix_map<double> Q(N, N);

    // Fill in the generator matrix Q
    RgEdge edge;
    for (size_t s = 0; s < rg.stateSets[TANGIBLE].size(); s++) {
        rgedge_t edgeList = rg.stateSets[TANGIBLE].get_first_edge(s);
        while (edgeList != INVALID_RGEDGE_INDEX) {
            GetEdgeByIndex(rg, edgeList, edge);
            assert(edge.typeOfSrcState() == TANGIBLE && edge.typeOfDstState() == TANGIBLE);
            size_t i = s;
            size_t j = edge.dest.index();
            double rate = edge.delay * edge.prob;
            Q(i, j) += rate;
            Q(i, i) -= rate;
            edgeList = edge.nextEdge;
        }

    }
    compress_matrix(ctmc.Q, Q);

    // Get the initial probability vector
    ctmc.pi0.resize(N);
    fill(ctmc.pi0.begin(), ctmc.pi0.end(), 0.0);
    for (size_t n = 0; n < rg.initStates.size(); n++)
        ctmc.pi0(rg.initStates[n].state.index()) = rg.initStates[n].initProb;

    if (verboseLvl >= VL_VERBOSE && N < 50) {
        cout << "CTMC MATRIX:\n";
        print_matrix(cout, ctmc.Q, "Q");
        cout << endl;
    }
}


//=============================================================================

void SolveSteadyStateCTMC(const CTMC &ctmc, PetriNetSolution &sol,
                          KolmogorovEquationDirection ked,
                          ublas::vector<double> *pRho,
                          const SolverParams &spar, VerboseLevel verboseLvl) {
    BSCC bscc;
    BottomStronglyConnectedComponents(ctmc.Q, bscc);

    LinearIterationPrinter printOut(verboseLvl);
    assert(ked == KED_FORWARD || pRho != nullptr);
    if (pRho != nullptr) {
        if (!Solve_SteadyState_CTMC(ctmc.Q, sol.stateProbs, ctmc.pi0, *pRho,
                                    ked, spar, bscc, printOut))
            throw program_exception("Solution method does not converge.");
    }
    else {
        ublas::zero_vector<double> zeroRho(ctmc.N());
        if (!Solve_SteadyState_CTMC(ctmc.Q, sol.stateProbs, ctmc.pi0, zeroRho,
                                    ked, spar, bscc, printOut))
            throw program_exception("Solution method does not converge.");
    }

    if (verboseLvl >= VL_VERBOSE) {
        cout << "PROBABILITIES = " << sol.stateProbs << endl << endl;
    }
}

//=============================================================================

void SolveTransientCTMC(const CTMC &ctmc, PetriNetSolution &sol, double t,
                        KolmogorovEquationDirection ked,
                        const SolverParams &spar, VerboseLevel verboseLvl) {
    LinearIterationPrinter printOut(verboseLvl);
    ublas::vector<double> *no_vec = nullptr;
    full_element_selector all_elems(ctmc.Q.size1());
    // Uniformization2(ctmc.Q, ctmc.pi0, sol.stateProbs, no_vec, t,
    //                 spar.epsilon, ked, all_elems, all_elems, printOut);
    UniformizationDet(ctmc.Q, ctmc.pi0, sol.stateProbs, no_vec, t,
                      spar.epsilon, ked, all_elems, all_elems, printOut);


    if (verboseLvl >= VL_VERBOSE) {
        cout << "PROBABILITIES = " << sol.stateProbs << endl << endl;
    }
}

//=============================================================================










