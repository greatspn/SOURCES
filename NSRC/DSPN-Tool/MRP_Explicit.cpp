/*
 *  MRP_Explicit.cpp
 *
 *  Implementation of the MRP explicit steady state solver.
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
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <memory>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "ReachabilityGraph.h"
#include "MRP.h"
#include "MRP_Explicit.h"

//=============================================================================

void BuildEmbeddedMatrices(const MRP &mrp, const MRPStateKinds &stKinds,
                           const SolverParams &spar,
                           ublas::compressed_matrix<double> &outP,
                           ublas::compressed_matrix<double> &outC,
                           VerboseLevel verboseLvl) {
    const marking_t N_MRP(mrp.Subsets.numStates());

    // Construct the two main matrices of the Markov Regenerative process:
    //   P = probabilities of the EMC states
    //   C = conversion factors (EMC -> MRP)
    numerical::matrix_map<double> P(N_MRP, N_MRP);
    numerical::matrix_map<double> C(N_MRP, N_MRP);
    long_interval_timer timer;
    performance_timer perfTimer;
    size_t numSmc = 0, numSmcComputed = 0;
    for (marking_t i = 0; i < N_MRP; ++i)
        if (0 != (stKinds.kinds[i] & MSK_EMC_SMC))
            numSmc++;
    if (verboseLvl >= VL_BASIC)
        cout << numSmc << " SUBORDINATED MARKOV CHAINS WILL BE COMPUTED." << endl;

    // Fill in P and C accordingly to the EMC state kinds
    for (marking_t i(0); i < N_MRP; ++i) {
        MRPStateKind kind = stKinds.kinds[i];

        // Fill the i-th row of P and C according to the state kind of i
        if (0 != (kind & MSK_EMC_ABSORBING)) {
            // P(i) = 0       C(i,i) = 1     It won't be normalized
            C(i, i) = 1;
        }
        else if (0 != (kind & MSK_EMC_NO_DET_ENABLED)) {
            // P(i) = (Q(i) / -q(i,i)) - I(i)
            // C(i,i) = 1 / -q(i,i)
            matRow_t Q_iRow(mrp.Q, i);
            double mQ_ii = - mrp.Q(i, i);
            for (rowIter_t Q_ij = Q_iRow.begin(); Q_ij != Q_iRow.end(); ++Q_ij) {
                const marking_t j(Q_ij.index());
                if (i != j)
                    P(i, j) = *Q_ij / mQ_ii;
            }
            if (mQ_ii != 0)
                C(i, i) = 1 / mQ_ii;
            else { // Avoid problems with absorbing states
                throw program_exception("Should be a EMCSK_ABSORBING.");
            }
        }
        else if (0 != (kind & MSK_EMC_ONLY_DET_ENABLED)) {
            // P(i) = Delta(i)
            // C(i,i) = delay
            double delay = mrp.Gen[mrp.Subsets.subsetOf(i)].detDelay;
            if (delay == GENERAL_DELAY) {
                throw program_exception("Explicit MRP solution is only supported for DSPN with "
                    "deterministic transitions. General transitions are not supported. Use the "
                    "implicit solution instead.");
            }
            verify(delay > 0.0); // Only deterministic, no general
            matRow_t Delta_iRow(mrp.Delta, i);
            for (rowIter_t Delta_ij = Delta_iRow.begin();
                    Delta_ij != Delta_iRow.end(); ++Delta_ij) {
                const marking_t j(Delta_ij.index());
                P(i, j) = *Delta_ij;
            }
            C(i, i) = delay;
        }
        else if (0 != (kind & MSK_EMC_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY)) {
            // P(i,j) = exp(-delta * |Qbar(i)|) * Delta(i)
            // P(i,j) = (1-exp(-delta * |Qbar(i)|)) * Qbar(i,j)/(|Qbar(i)|)
            // C(i,i) = 1 - exp(-delta * |Qbar(i)|) / (|Qbar(i)|)
            // C(i,j) = 0
            double delay = mrp.Gen[mrp.Subsets.subsetOf(i)].detDelay;
            verify(delay > 0.0); // Only deterministic, no general
            double SumQ = -mrp.Q(i, i);

            const double MAX_EXP_VAL = 200.0;
            double expVal = 0.0;
            if (SumQ * delay < MAX_EXP_VAL) // Avoid underflows
                expVal = exp(-SumQ * delay);

            matRow_t Delta_iRow(mrp.Delta, i);
            for (rowIter_t Delta_ij = Delta_iRow.begin();
                    Delta_ij != Delta_iRow.end(); ++Delta_ij) {
                const marking_t j(Delta_ij.index());
                P(i, j) += *Delta_ij * expVal;
            }

            matRow_t Qbar_iRow(mrp.Qbar, i);
            for (rowIter_t Qbar_ij = Qbar_iRow.begin();
                    Qbar_ij != Qbar_iRow.end(); ++Qbar_ij) {
                const marking_t j(Qbar_ij.index());
                P(i, j) += (1 - expVal) * (*Qbar_ij) / SumQ;
            }

            C(i, i) = (1 - expVal) / SumQ;
        }
        else if (0 != (kind & MSK_EMC_SMC)) {
            numSmcComputed++;
            if (verboseLvl == VL_BASIC && timer) {
                timer.prepare_print();
                cout << "COMPUTING SMC " << numSmcComputed << "/" << numSmc << "... ";
                if (timer.get_elapsed_time() > 1.5) {
                    timer.print_estimate(numSmcComputed, numSmc);
                    cout << " remained.";
                }
                cout << "           " << endl;
            }

            // P(i) = Prob(i @ t) * Delta(i)    with transient probabilities
            // C(i) = AccumProb(i @ t)          with cumulative transient probs.
            // These lines are obtained with an intermediate SMC.
            const GenMarkingSubset &Sgen = mrp.Gen[mrp.Subsets.subsetOf(i)];
            full_element_selector allMrp(N_MRP);
            ublas::vector<double> inVec(N_MRP), ag, bg, OmegaDelta_iRow, PsiQbar_iRow;
            set_vec(inVec, 0.0, allMrp);
            inVec(i) = 1.0;

            MRPSubsetList::selector GenStates(mrp.Subsets.selectorOf(Sgen.index));
            NullIterPrintOut printOut;
            UniformizationGen(mrp.Q, inVec, ag, &bg, Sgen.fg.c_str(), spar.epsilon,
                              KED_FORWARD, GenStates, allMrp, printOut);
            prod_vecmat(OmegaDelta_iRow, ag, mrp.Delta, GenStates, allMrp);
            prod_vecmat(PsiQbar_iRow, bg, mrp.Qbar, GenStates, allMrp);
            for (marking_t j(0); j < N_MRP; ++j) {
                double P_ij = OmegaDelta_iRow[j] + PsiQbar_iRow[j];
                double C_ij = bg[j];
                if (P_ij != 0.0)
                    P(i, j) = P_ij;
                if (C_ij != 0.0)
                    C(i, j) = C_ij;
            }
        }
        else {
            // State i is not a regenerative state
            // P(i,i) = 1     C(i,i) = 1   (any value, since the i-th line has no prob)
            assert(0 == (kind & (MSK_INITSTATE | MSK_REGENSTATE)));
            P(i, i) = 1.0;
            C(i, i) = 1.0;
        }
    }

    compress_matrix(outP, P);
    compress_matrix(outC, C);
    if (verboseLvl >= VL_BASIC) {
        cout << "P MATRIX (" << P.size1() << " x " << P.size2() << ") ";
        cout << "HAS " << P.nnz() << " NONZERO ENTRIES." << endl;
        cout << "C MATRIX (" << C.size1() << " x " << C.size2() << ") ";
        cout << "HAS " << C.nnz() << " NONZERO ENTRIES." << endl;
        cout << "EMC BUILD TIME: " << perfTimer.get_timer_repr() << endl << endl;
    }
    P.clear();
    C.clear();

    if (verboseLvl >= VL_VERBOSE && N_MRP < marking_t(50)) {
        cout << "\n\n\nEMBEDDED DTMC P AND CONVERSION FACTORS C MATRICES:\n\n";
        print_matrix(cout, outP, "P");
        cout << endl;
        print_matrix(cout, outC, "C");
        cout << endl;
    }
}

//=============================================================================

void SteadyStateMRPExplicit(const MRP &mrp, const MRPStateKinds &stKinds,
                            PetriNetSolution &sol, const SolverParams &spar,
                            VerboseLevel verboseLvl) {
    const marking_t N_MRP(mrp.Subsets.numStates());
    ublas::compressed_matrix<double> P, C;
    BuildEmbeddedMatrices(mrp, stKinds, spar, P, C, verboseLvl);

    // The recurrent structure of an MRP is given by the "sum" of all
    // the transitions, which is (Q + Qbar + Delta).
    BSCC bscc;
    BottomStronglyConnectedComponents(mrp.Q + mrp.Qbar + mrp.Delta, bscc);

    // Prepare the initial distribution: only initial/regenerative states
    // starts with a nonzero values.
    ublas::vector<double> init_x0(N_MRP);
    size_t numRegenOrInitSt = 0;
    for (marking_t m(0); m < N_MRP; ++m)
        if (stKinds.kinds[m] & (MSK_REGENSTATE | MSK_INITSTATE))
            numRegenOrInitSt++;
    for (marking_t m(0); m < N_MRP; ++m) {
        if (stKinds.kinds[m] & (MSK_REGENSTATE | MSK_INITSTATE))
            init_x0(m) = 1.0 / numRegenOrInitSt;
        else
            init_x0(m) = 0.0;  // Non regenerative state
    }

    // Solve the Embedded EMC matrix P, using the MRP recurrence structure.
    ublas::vector<double> emcSol;
    LinearIterationPrinter printOut(verboseLvl);
    ublas::zero_vector<double> zeroRho(N_MRP);
    if (!Solve_SteadyState_DTMC(P, emcSol, mrp.pi0, zeroRho, KED_FORWARD,
                                spar, bscc, printOut, &init_x0))
        throw program_exception("Solution method does not converge.");

    // Convert the EMC solution to the MRP solution with the C factors
    full_element_selector all_elems(N_MRP);
    prod_vecmat(sol.stateProbs, emcSol, C, all_elems, all_elems);

    // At this point sol.stateProbs contains the mean times in the MRP states,
    // before state transitions: normalize these mean times into probabilities.
    // Note that the normalization process must be done separately for each BSCC,
    // because each BSCC may have a different normalization coefficient. Also,
    // absorbing states requires a special treatment (because they have, at
    // least in theory, an infinite mean time).
    for (sccindex_t nc(-1); nc < bscc.numBscc; nc++) {
        double total_bscc_prob = 0.0;
        for (marking_t m = 0; m < N_MRP; m++) {
            if (bscc.indices[m] == nc) {
                total_bscc_prob += emcSol(m);
            }
        }
        // Normalize this BSCC.
        // Note that absorbing states of the EMC should be excluded
        // from the normalization process (i.e. simply use the absorption probability)
        double sumNonAbs = 0.0, sumAbs = 0.0;
        for (marking_t m = 0; m < N_MRP; m++) {
            if (bscc.indices[m] != nc)
                continue;
            bool isAbsorbingState = (0 != (stKinds.kinds[m] & MSK_EMC_ABSORBING));
            if (isAbsorbingState)
                sumAbs += sol.stateProbs[m];
            else sumNonAbs += sol.stateProbs[m];
        }
        if (sumNonAbs > 0.0) {
            // Normalize non-absorbing states
            double NormCoeff = (1.0 - sumAbs) / sumNonAbs;
            NormCoeff *= total_bscc_prob;
            for (marking_t m = 0; m < N_MRP; m++) {
                if (bscc.indices[m] != nc)
                    continue;
                bool isAbsorbingState = (0 != (stKinds.kinds[m] & MSK_EMC_ABSORBING));
                if (!isAbsorbingState)
                    sol.stateProbs[m] *= NormCoeff;
            }
        }
    }

    sol.stateProbs /= vec_sum(sol.stateProbs);

    // TODO: add support to firing frequencies
    sol.firingFreq.resize(N_MRP);
    std::fill(sol.firingFreq.begin(), sol.firingFreq.end(), -1.0);

    if (verboseLvl >= VL_VERBOSE) {
        cout << "\nEMC SOLUTION: " << emcSol << endl;
        cout << "MRP SOLUTION: " << sol.stateProbs << endl;
        cout << endl;
    }
}

//=============================================================================

void SteadyStateMRPExplicitBackward(const MRP &mrp, const MRPStateKinds &stKinds,
                                    const ublas::vector<double> &rho,
                                    ublas::vector<double> &xi,
                                    const SolverParams &spar,
                                    VerboseLevel verboseLvl) {
    const marking_t N_MRP(mrp.Subsets.numStates());
    ublas::compressed_matrix<double> P, C;
    BuildEmbeddedMatrices(mrp, stKinds, spar, P, C, verboseLvl);
    full_element_selector all_elems(N_MRP);

    // Compute the vector: C * rho
    ublas::vector<double> C_rho;
    ublas::scalar_vector<double> one_vec(N_MRP, 1.0);
    prod_matvec(C_rho, C, -rho, all_elems, all_elems);

    BSCC bscc;
    //BottomStronglyConnectedComponents(mrp.Q + mrp.Qbar + mrp.Delta, bscc);

    // Compute the backward solution of the system in P
    ublas::vector<double> emcSol_C_rho;
    LinearIterationPrinter printOut(verboseLvl);
    ublas::zero_vector<double> zeroPi(N_MRP);
    if (!Solve_SteadyState_DTMC(P, emcSol_C_rho, zeroPi, C_rho, KED_BACKWARD,
                                spar, bscc, printOut, &rho))
        throw program_exception("Solution method does not converge.");

    // Compute the backward vector xi
    xi = std::move(emcSol_C_rho);

    if (verboseLvl >= VL_VERBOSE) {
        cout << "\nBACKWARD EMC SOLUTION: " << xi << endl;
        cout << endl;
    }
}

//=============================================================================









