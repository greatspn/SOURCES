/*
 *  SynchProduct.cpp
 *
 *  Implementation of the synchronized product between a DTA and a CTMC.
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
#include <unordered_map>
#include <functional>
using namespace std;

#include <boost/optional.hpp>

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "Language.h"
#include "DTA.h"
#include "ReachabilityGraph.h"
#include "PackedMemoryPool.h"
#include "NewReachabilityGraph.h"
#include "MRP.h"
#include "MRP_Explicit.h"
#include "MRP_Implicit.h"
#include "MRP_SccDecomp.h"
#include "SynchProduct.h"

#include "compact_table_impl.h"

//=============================================================================

template class table < col::uint_col < marking_t, +2 >,
                       col::uint_col < location_t, +2 >, col::uint_col<clockzone_t >>;

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
template class table<col::uint_col<slcstate_t>>;
#endif

//=============================================================================

static const marking_t   TOP_STATE = marking_t(size_t(-1));
static const location_t  TOP_LOCATION = location_t(size_t(-1));
static const marking_t   BOT_STATE = marking_t(size_t(-2));
static const location_t  BOT_LOCATION = location_t(size_t(-2));

//=============================================================================

std::function<ostream& (ostream &)>
print_slc(const slc_tuple_t &slc, const RG2 &rg, const DTA &dta) {
    return [&](ostream & os) -> ostream& {
        if (slc.l == BOT_LOCATION)
            os << "<Bot>";
        else if (slc.l == TOP_LOCATION)
            os << "<Top>";
        else {
            os << "<s" << 1 + size_t(slc.s) << "," << dta.locs[slc.l].label << ",";
            os << "[" << dta.zones[slc.c].v1 << "-";
            if (dta.zones[slc.c].v2 == numeric_limits<double>::max())
                os << "INF";
            else
                os << dta.zones[slc.c].v2;
            os << "]>";
        }
        return os;
    };
}

//=============================================================================
// SYNCHRONIZED PRODUCT CONSTRUCTION
//=============================================================================

// Add a new <s,l,c> state in the state space of MxA (if it is not already there),
// and return its index (of type slcstate_t).
static
slcstate_t GetOrInsertTangSLC(const slc_tuple_t &slc, SynchronizedProduct &MxA,
                              BuildMxAHelper &build) {
    std::map<slc_tuple_t, slcstate_t>::const_iterator x = build.slcToInd.find(slc);
    if (x != build.slcToInd.end())
        return x->second; // The tuple slc already exists

    // Add a new tuple to the state space of MxA
    slcstate_t newInd = MxA.SLC.size();
    MxA.SLC.push_back(slc.s, slc.l, slc.c);
    build.slcToInd[slc] = newInd;
    size_t N = MxA.SLC.size();

    build.state_queue.push(newInd);
    build.Q.enlarge(N, N);
    build.Qbar.enlarge(N, N);
    build.Delta.enlarge(N, N);

    return newInd;
}

//=============================================================================

// Closure of boundary edges
static inline
slcstate_t closure(slc_tuple_t slc, const rgstate_t stateIndex, const RG2 &rg,
                   const DTA &dta, SynchronizedProduct &MxA, BuildMxAHelper &build,
                   VerboseLevel verboseLvl, size_t recursiveDepth = 0) {
    const size_t MAX_CLOSURE_RECURSIVE_DEPTH = 1000;
    if (recursiveDepth >= MAX_CLOSURE_RECURSIVE_DEPTH)
        throw program_exception("Loops of Boundary edges are not allowed.");

    const Location &l = dta.locs[slc.l];
    const ClockZone &cz = dta.zones[slc.c];

    // Test for final location
    if (l.finalType == FLT_FINAL_ACCEPTING)
        return MxA.top;
    if (l.finalType == FLT_FINAL_REJECTING)
        return MxA.bot;

    // Test if there is a Boundary edge enabled in <s,l,c>
    for (const DtaEdge &boundEdge : l.boundEdges) {
        // Test clock zone
        double alpha = boundEdge.alphaValue(dta);
        assert(boundEdge.isBoundary() && alpha == boundEdge.betaValue(dta));
        if (alpha == cz.v1) {
            // Test the state proposition expression of the destination location
            const Location &newl = dta.locs[boundEdge.destLoc];
            if (newl.spExpr->EvaluateInRgState(stateIndex.index())) {
                // Recursively evaluate the closure in:  <s, newl, c[r:=0]>
                clockzone_t newc = (boundEdge.reset ? clockzone_t(0) : slc.c);
                if (verboseLvl >= VL_VERBOSE)
                    cout << " " << print_slc(slc, rg, dta) << "  ==[#]==>  ";
                return closure(slc_tuple_t(slc.s, newl.index, newc), stateIndex, rg,
                               dta, MxA, build, verboseLvl, recursiveDepth + 1);
            }
        }
    }

    // The <s,l,c> state is tangible. Add it to the MxA if necessary,
    // and return its index in the MxA state space.
    return GetOrInsertTangSLC(slc, MxA, build);
}

//=============================================================================

void BuildSynchronizedProduct(const RG2 &rg, const DTA &dta, SynchronizedProduct &MxA,
                              marking_t initMark, VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("The synchronized product can be built only with a CTMC.");
    verify(dta.isBound);

    const size_t NS = rg.NumStates();
    MxA.inits.resize(NS);
    clockzone_t cInf = clockzone_t(dta.zones.size() - 1), c0 = clockzone_t(0);
    long_interval_timer visitTimer;

    // Helper structure
    BuildMxAHelper build;

    // Insert the top and bottom states
    MxA.top = GetOrInsertTangSLC(slc_tuple_t(TOP_STATE, TOP_LOCATION, cInf), MxA, build);
    MxA.bot = GetOrInsertTangSLC(slc_tuple_t(BOT_STATE, BOT_LOCATION, cInf), MxA, build);
    build.state_queue.pop();
    build.state_queue.pop();
    for (size_t s0 = 0; s0 < NS; s0++)
        MxA.inits[s0] = MxA.bot;

    // Insert all the states of the CTMC as initial states of the MxA process
    size_t statNumOfNonTrivialInitStates = 0, statNumVisited = 0;
    // Function that inserts the initial state <s0,0,c> given s0, if a valid l0 exists.
    auto insert_init_slc = [&](size_t s0) {
        rgstate_t stateIndex(s0, TANGIBLE);
        // Find a initial location of the DTA tha accepts state s0
        location_t l0(size_t(-1));
        for (const Location &loc : dta.locs) {
            if (loc.isInitial && loc.spExpr->EvaluateInRgState(stateIndex.index())) {
                l0 = loc.index;
                break;
            }
        }
        if (l0 != location_t(size_t(-1))) {
            // There is a initial location that accepts state s0
            MxA.inits[s0] = closure(slc_tuple_t(s0, l0, c0), stateIndex, rg,
                                    dta, MxA, build, verboseLvl);
            if (MxA.inits[s0] != MxA.top && MxA.inits[s0] != MxA.bot)
                statNumOfNonTrivialInitStates++;
        }
        if (verboseLvl >= VL_VERBOSE) {
            cout << "  INITIAL STATE FOR s" << (1 + size_t(s0)) << " IS ";
            cout << print_slc(MxA.get_slc_tuple(MxA.inits.at(s0)), rg, dta) << endl;
        }
    };

    if (initMark != marking_t(-1)) {
        // Forward state space construction: consider a single initial state
        insert_init_slc(size_t(initMark));
    }
    else {
        // Backward state space construction: consider all possible initial states
        for (size_t s0 = 0; s0 < NS; s0++)
            insert_init_slc(s0);
    }

    if (verboseLvl >= VL_VERBOSE)
        cout << endl;

    // Explore the state space of MxA
    RgEdge ctmcEdge;
    while (!build.state_queue.empty()) {
        const slcstate_t srcInd = build.state_queue.front();
        const slc_tuple_t src_slc(MxA.get_slc_tuple(srcInd));
        build.state_queue.pop();
        const Location &src_loc = dta.locs[src_slc.l];
        const ClockZone &src_cz = dta.zones[src_slc.c];

        if (verboseLvl >= VL_VERBOSE)
            cout << 1 + size_t(srcInd) << ": FROM TUPLE "
                 << print_slc(MxA.get_slc_tuple(srcInd), rg, dta) << " DEPARTS:" << endl;

        // Try following every CTMC edge departing from s
        rgedge_t edgeList = rg.stateSets[TANGIBLE].get_first_edge(src_slc.s);
        while (edgeList != INVALID_RGEDGE_INDEX) {
            GetEdgeByIndex(rg, edgeList, ctmcEdge);
            assert(ctmcEdge.typeOfSrcState() == TANGIBLE && ctmcEdge.typeOfDstState() == TANGIBLE);
            size_t dstStateInd = ctmcEdge.dest.index();

            if (verboseLvl >= VL_VERBOSE) {
                cout << "   ==[T" << ctmcEdge.firedTrn << ", ";
                cout << ctmcEdge.delay;
                if (ctmcEdge.prob != 1.0)
                    cout << ", prob=" << ctmcEdge.prob;
                cout << "]==>";
            }

            // Check for an Inner edge that may follow that firing
            slcstate_t dstInd = MxA.bot;
            bool isPreemptive = (src_slc.c != cInf);
            for (const DtaEdge &innerEdge : src_loc.innerEdges) {
                // Test clock constraint
                double alpha = innerEdge.alphaValue(dta);
                double beta = innerEdge.betaValue(dta);
                assert(innerEdge.isInner() && alpha < beta);
                bool isCCok = (alpha <= src_cz.v1 && src_cz.v2 <= beta);
                if (!isCCok)
                    continue;

                // Test action set
                bool trn_in_actset = false;
                for (const auto &actInd : innerEdge.actset.actInds) {
                    if (dta.acts[actInd].boundedTrn == ctmcEdge.firedTrn) {
                        trn_in_actset = true;
                        break;
                    }
                }
                if (innerEdge.actset.type == AT_ACCEPTED_ACTIONS) {
                    if (!trn_in_actset)
                        continue;
                }
                else { // type == AT_REJECTED_ACTIONS
                    if (trn_in_actset)
                        continue;
                }
                // Test state proposition expression in the destination state
                rgstate_t dstStateIndex(dstStateInd, TANGIBLE);
                const Location &dst_loc = dta.locs[innerEdge.destLoc];
                marking_t dst_s(dstStateIndex.index());
                if (!dst_loc.spExpr->EvaluateInRgState(dstStateIndex.index()))
                    continue;

                // The CTMC transition has been accepted - test the finalType
                if (dst_loc.finalType == FLT_FINAL_ACCEPTING) { // M_OK transition
                    dstInd = MxA.top;
                }
                else if (dst_loc.finalType == FLT_FINAL_REJECTING) { // M_KO transition
                    dstInd = MxA.bot;
                }
                else if (innerEdge.reset) {
                    // M_RES transition
                    dstInd = closure(slc_tuple_t(dst_s, dst_loc.index, c0),
                                     dstStateIndex, rg, dta, MxA, build, verboseLvl);
                }
                else {
                    // M transition
                    dstInd = GetOrInsertTangSLC(slc_tuple_t(dst_s, dst_loc.index, src_slc.c),
                                                MxA, build);
                    isPreemptive = false;
                }
                break; // The CTMC transition has been accepted by a DTA edge
            }
            // If the loop ends without accepting, it is an M_KO edge

            // Add the transition from src to dst with Markovian delay
            double rate = ctmcEdge.delay * ctmcEdge.prob;
            if (isPreemptive)
                build.Qbar(srcInd, dstInd) += rate;
            else
                build.Q(srcInd, dstInd) += rate;
            build.Q(srcInd, srcInd) -= rate;

            if (verboseLvl >= VL_VERBOSE) {
                cout << "   " << print_slc(MxA.get_slc_tuple(dstInd), rg, dta) << endl;
            }

            edgeList = ctmcEdge.nextEdge;
        }

        // Evaluate time elapse arcs.
        if (src_slc.c != cInf) {
            rgstate_t stateIndex(src_slc.s, TANGIBLE);
            clockzone_t nextC = src_slc.c + clockzone_t(1);

            if (verboseLvl >= VL_VERBOSE)
                cout << "   ==[D]==>  ";
            slcstate_t dstInd = closure(slc_tuple_t(src_slc.s, src_slc.l, nextC),
                                        stateIndex, rg, dta, MxA, build, verboseLvl);

            // Add a Delta transition from src to dst
            build.Delta(srcInd, dstInd) = 1.0;

            if (verboseLvl >= VL_VERBOSE)
                cout << print_slc(MxA.get_slc_tuple(dstInd), rg, dta) << endl;
        }

        ++statNumVisited;
        if (verboseLvl >= VL_VERBOSE)
            cout << endl;
        if (verboseLvl == VL_BASIC && visitTimer) {
            visitTimer.prepare_print();
            cout << "  <S,L,C> STATES IN MxA: " << setw(7) << left << statNumVisited;
            cout << "       " << endl;
        }
    }

    if (verboseLvl == VL_BASIC) {
        visitTimer.clear_any();
    }
    if (verboseLvl >= VL_VERBOSE)
        cout << endl;


    // Extract an MRP from the intermediate matrix_maps
    std::vector<statesubset_t> slcToSubset(MxA.SLC.size());
    std::map<clockzone_t, statesubset_t> cz2sub;
    cz2sub[cInf] = EXP_SUBSET;
    const slcstate_t NSLC(MxA.SLC.size());
    for (slcstate_t i(0); i < NSLC; i++) {
        const slc_tuple_t &slc = MxA.get_slc_tuple(i);
        if (cz2sub.count(slc.c) == 0) {
            GenMarkingSubset gms;
            gms.index     = statesubset_t(MxA.mrp.Gen.size());
            gms.genTrnInd = transition_t(0);
            gms.fg        = DeterministicDurationFg(dta.zones[slc.c].duration());
            gms.detDelay  = dta.zones[slc.c].duration();
            MxA.mrp.Gen.push_back(gms);
            cz2sub[slc.c] = gms.index;
        }
        slcToSubset[i] = cz2sub[slc.c];
    }
    MxA.mrp.Subsets.ConstructSubsetList(MxA.mrp.Gen.size() + 2, slcToSubset);

    // Compress the 3 mapped matrices
    compress_matrix(MxA.mrp.Q, build.Q);
    compress_matrix(MxA.mrp.Qbar, build.Qbar);
    compress_matrix(MxA.mrp.Delta, build.Delta);

    if (verboseLvl >= VL_VERBOSE)
        PrintMRP(MxA.mrp);

    if (verboseLvl >= VL_BASIC) {
        cout << "\n";
        cout << "NUMBER OF TANGIBLE <S*L*C> STATES:     " << MxA.mrp.N() << " (INCLUDING TOP, BOT)\n";
        cout << "NUMBER OF CTMC STATES:                 " << NS << endl;
        cout << "NUMBER OF DTA LOCATIONS:               " << dta.locs.size() << endl;
        cout << "NUMBER OF TIMED CLOCK ZONES:           " << dta.zones.size() << endl;
        cout << "NUMBER OF DETERMINISTIC TRANSITIONS:   " << MxA.mrp.Gen.size() << endl;
        cout << "NONZERO ENTRIES IN Q:                  " << MxA.mrp.Q.nnz() << endl;
        cout << "NONZERO ENTRIES IN Qbar:               " << MxA.mrp.Qbar.nnz() << endl;
        cout << "NONZERO ENTRIES IN DELTA:              " << MxA.mrp.Delta.nnz() << endl;
        cout << "NON-TRIVIAL INITIAL STATES:            " << statNumOfNonTrivialInitStates << endl;
        cout << endl;
    }
}

//=============================================================================

double
ModelCheck_CTMC_DTA_Forward(const RG2 &rg, const DTA &dta, marking_t s0,
                            const SolverParams &spar, VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("The synchronized product can be built only with a CTMC.");
    if (!dta.isBound)
        throw program_exception("Component analysis requires the DTA to be bound.");
    verify(s0 < marking_t(rg.NumStates()));

    // Construct the synchronized process S*L*C
    SynchronizedProduct MxA;
    BuildSynchronizedProduct(rg, dta, MxA, s0, verboseLvl);
    MxA.mrp.pi0.resize(MxA.mrp.N());

    if (MxA.inits[s0] == MxA.bot) {
        if (verboseLvl >= VL_BASIC)
            cout << "INITIAL CTMC STATE s" << 1 + size_t(s0)
                 << " DOES NOT SATISFY ANY INITIAL DTA LOCATION.\n" << endl;
        return 0.0;
    }
    if (MxA.inits[s0] == MxA.top) {
        if (verboseLvl >= VL_BASIC)
            cout << "INITIAL CTMC STATE s" << 1 + size_t(s0)
                 << " STARTS IN A FINAL DTA LOCATION.\n" << endl;
        return 1.0;
    }

    // Compute the steady-state solution of the MxA MRP
    MxA.mrp.pi0.resize(MxA.mrp.N());
    MxA.mrp.pi0[MxA.inits.at(s0)] = 1.0;

    cout << console::beg_title() << "\nCOMPUTING STEADY STATE SOLUTION ";
    cout << "OF THE MARKOV RENEWAL PROCESS MxA..." << console::end_title() << endl;

    MRPStateKinds stKinds;
    PetriNetSolution sol;
    ClassifyMRPStates(MxA.mrp, stKinds, verboseLvl);
    if (spar.useSCCDecomp)
        SteadyStateMrpBySccDecomp(MxA.mrp, stKinds, sol, spar, verboseLvl);
    else if (spar.solMethod == LSS_IMPLICIT)
        SteadyStateMRPImplicit(MxA.mrp, stKinds, sol, spar, verboseLvl);
    else
        SteadyStateMRPExplicit(MxA.mrp, stKinds, sol, spar, verboseLvl);

    if (verboseLvl >= VL_BASIC)
        cout << "\nFORWARD STEADY STATE SOLUTION OF THE MxA PROCESS COMPUTED.\n" << endl;

    return sol.stateProbs[MxA.top];
}

//=============================================================================

void
ModelCheck_CTMC_DTA_Backward(const RG2 &rg, const DTA &dta, ublas::vector<double> &probs,
                             const SolverParams &spar, VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("The synchronized product can be built only with a CTMC.");
    if (!dta.isBound)
        throw program_exception("Component analysis requires the DTA to be bound.");

    // Construct the synchronized process S*L*C
    SynchronizedProduct MxA;
    BuildSynchronizedProduct(rg, dta, MxA, marking_t(-1), verboseLvl);
    MxA.mrp.pi0.resize(MxA.mrp.N());
    std::fill(MxA.mrp.pi0.begin(), MxA.mrp.pi0.end(), 0.0);
    for (const slcstate_t slc : MxA.inits)
        MxA.mrp.pi0[slc] = 1.0;

    // Construct the reward vector rho
    const slcstate_t NUMSLC = MxA.SLC.size();
    ublas::vector<double> rho(NUMSLC);
    std::fill(rho.begin(), rho.end(), 0.0);
    rho[ MxA.top ] = 1.0;

    MRPStateKinds stKinds;
    PetriNetSolution sol;
    ClassifyMRPStates(MxA.mrp, stKinds, verboseLvl);
    if (spar.useSCCDecomp)
        SteadyStateMrpBySccDecompBackward(MxA.mrp, stKinds, rho, sol.stateProbs, spar, verboseLvl);
    else if (spar.solMethod == LSS_IMPLICIT)
        SteadyStateMRPImplicitBackward(MxA.mrp, stKinds, rho, sol.stateProbs, spar, verboseLvl);
    else
        SteadyStateMRPExplicitBackward(MxA.mrp, stKinds, rho, sol.stateProbs, spar, verboseLvl);

    probs.resize(rg.NumStates());
    std::fill(probs.begin(), probs.end(), 0.0);
    for (size_t i = 0; i < rg.NumStates(); i++)
        probs[i] = sol.stateProbs[ MxA.inits.at(i) ];

    if (verboseLvl >= VL_BASIC)
        cout << "\nBACKWARD STEADY STATE SOLUTION OF THE MxA PROCESS COMPUTED.\n" << endl;
}

//=============================================================================


void PrepareStatePropositions(const RG2 &rg, DTA &dta, const SolverParams &spar,
                              VerboseLevel verboseLvl) {
    if (verboseLvl >= VL_VERBOSE)
        cout << "\n\nPREPARING DTA SUB-EXPRESSIONS...\n";

    // Optimize the DTA location expressions
    std::map<string, shared_ptr<AtomicPropExpr>> exprCache;
    for (Location &loc : dta.locs) {
        ostringstream repr;
        loc.spExpr->Print(repr, true);

        auto ec = exprCache.find(repr.str());
        if (ec != exprCache.end()) {
            loc.spExpr = ec->second;
            if (verboseLvl >= VL_VERBOSE) {
                cout << "OPTIMIZING STATE PROPOSITION EXPRESSION OF LOCATION " << loc.label << endl;
            }
        }
        else
            exprCache.insert(make_pair(repr.str(), loc.spExpr));
    }

    // Store the evaluation of the atomic propositions
    // Sub expressions are always evaluated for all the CTMC states, so no
    // initial state is needed.
    boost::optional<marking_t> noInitState;
    for (AtomicProp &ap : dta.aps) {
        ap.boundedExpr->get_expr()->StoreEvaluation(rg, spar, noInitState, verboseLvl);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "  SAT SET OF ATOMIC PROPOSITION " << ap.boundedExpr->get_name() << ": ";
            ap.boundedExpr->get_expr()->GetStoredSAT().Print(cout, true);
            cout << endl;
        }
    }

    // Prepare the DTA expressions on the locations.
    for (Location &loc : dta.locs) {
        loc.spExpr->StoreEvaluation(rg, spar, noInitState, verboseLvl);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "  SAT SET OF LOCATION " << loc.label << ": ";
            loc.spExpr->GetStoredSAT().Print(cout, true);
            cout << endl;
        }
    }
    if (verboseLvl >= VL_VERBOSE)
        cout << "\n\n";
}

//=============================================================================








