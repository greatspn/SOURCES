/*
 *  SynchProductSCC.cpp
 *
 *  Implementation of the synchronized product between a CTMC and a ZDTA.
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
#include <iterator>
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
#include "MRP_Implicit.h"
#include "MRP_SccDecomp.h"
#include "SynchProductSCC.h"

#include "compact_table_impl.h"

template class table < col::uint_col < marking_t, +2 >, col::uint_col < location_t, +2 >>;

//=============================================================================

const sz_tuple_t sz_tuple_t::TOP(marking_t(size_t(-1)), location_t(size_t(-1))) ;
const sz_tuple_t sz_tuple_t::BOT(marking_t(size_t(-2)), location_t(size_t(-2))) ;

//=============================================================================

const marking_t TOP_IND = 0, BOT_IND = 1, VANISHING_IND = -100;

void PrepareMRPStructureFromZonedDTA(const DTA &zdta, MRP &zmrp,
                                     const vector<bool> &initLocsUsed,
                                     std::map<marking_t, location_t> &tangLocToZonedLoc,
                                     VerboseLevel verboseLvl) {
    if (!zdta.isZoned)
        throw program_exception("Component analysis requires a zoned DTA.");

    // Build a fake MRP from the tangible locations of this zoned DTA.
    // States represent locations, and edges represent sets of transitions of the same kind.
    zmrp.Gen.resize(zdta.zones.size() - 1);
    for (size_t i = 0; i < zmrp.Gen.size(); i++) {
        zmrp.Gen[i].index     = statesubset_t(i);
        zmrp.Gen[i].genTrnInd = transition_t(0);
        zmrp.Gen[i].detDelay  = zdta.zones[i].duration();
        zmrp.Gen[i].fg        = DeterministicDurationFg(zdta.zones[i].duration());
    }
    if (verboseLvl >= VL_VERBOSE) {
        cout << "\nPREPARING THE ZONED DTA FOR THE COMPONENT METHOD.\n\n";
        cout << "  LOCATION \'TOP\'            IS [m1]\n";
        cout << "  LOCATION \'BOT\'            IS [m2]" << endl;
    }

    std::map<location_t, marking_t> locToTang;
    std::map<marking_t, location_t> tangToLoc;
    // Helper function to print tangible zoned markings, and their corresponding location in the ZDTA
    auto print_zmark = [&](const marking_t mk) -> std::function<ostream& (ostream &)> {
        return [ &, mk](ostream & os) -> ostream &{
            assert(mk != VANISHING_IND);
            os << "[m" << 1 + size_t(mk) << (mk == TOP_IND ? ",(TOP)" : (mk == BOT_IND ? ",(BOT)" : ""));
            if (tangToLoc.count(mk))
                os << ",\"" << zdta.locs[tangToLoc[mk]].label << "\"";
            return os << "]";
        };
    };

    // Prepare the state space of the decomposable process from the tangible/final locations
    std::vector<statesubset_t> slcToSubset;
    size_t numTangNonFinalLocs = 0;
    slcToSubset.push_back(EXP_SUBSET); // Top state has index 0 and is in EXP
    slcToSubset.push_back(EXP_SUBSET); // Bottom state has index 1 and is in EXP
    for (const Location &zloc : zdta.locs) {
        marking_t mk = VANISHING_IND;
        if (zloc.isFinal())
            mk = (zloc.finalType == FLT_FINAL_ACCEPTING ? TOP_IND : BOT_IND);
        else if (zloc.isTimed()) {
            mk = marking_t(2 + numTangNonFinalLocs++);
            slcToSubset.push_back((zloc.zone != zdta.cInf()) ? statesubset_t(zloc.zone) : EXP_SUBSET);
            tangToLoc[mk] = zloc.index;
        }
        locToTang[zloc.index] = mk;

        if (verboseLvl >= VL_VERBOSE) {
            cout << "  LOCATION " << setw(16) << left << zloc.label << " IS ";
            if (mk == VANISHING_IND) cout << "VANISHING"; else cout << print_zmark(mk);
            cout << endl;
        }
    }
    if (verboseLvl >= VL_VERBOSE)
        cout << endl;

    const size_t N_MRP = numTangNonFinalLocs + 2;
    numerical::matrix_map<double> Q(N_MRP, N_MRP), Qbar(N_MRP, N_MRP), Delta(N_MRP, N_MRP);
    zmrp.pi0.resize(N_MRP);
    zmrp.Subsets.ConstructSubsetList(zmrp.Gen.size() + 2, slcToSubset);
    if (verboseLvl >= VL_VERBOSE)
        cout << "LIST OF EDGES IN THE TANGIBLE ZONED DTA:\n" << endl;

    auto AppendPath = [](const vector<location_t> &v, location_t loc) -> vector<location_t> {
        vector<location_t> v2(v);  v2.push_back(loc);  return v2;
    };
    auto CheckPath = [](const vector<location_t> &v) {
        if (v.size() > 1000)
            throw program_exception("Loops of boundary edges are not allowed.");
    };
    // Helper function to print tangible zoned markings, and their corresponding location in the ZDTA
    auto print_vpath = [&](const vector<location_t> &vpath, const marking_t mk) -> std::function<ostream& (ostream &)> {
        return [&](ostream & os) -> ostream &{
            assert(vpath.size() > 0);
            for (size_t i = 0; i < vpath.size(); i++) {
                if (i == vpath.size() - 1) {
                    assert(locToTang.count(vpath[i]) > 0);
                    marking_t mk = locToTang[vpath[i]];
                    os << print_zmark(mk);
                }
                else
                    os << "\"" << zdta.locs[vpath[i]].label << "\"  ->  ";
            }
            return os;
        };
    };
    auto PrintTvtEdge = [&](char edgeType, const marking_t mkSrc,
    const vector<location_t> &vpath, bool isPreempt) {
        assert(vpath.size() > 0 && locToTang.count(vpath.back()) > 0);
        marking_t mkDst = locToTang[vpath.back()];
        cout << "  " << edgeType << (mkDst == TOP_IND ? "_OK:  " :
                                     (mkDst == BOT_IND ? "_KO:  " : (isPreempt ? "_RES: " : ":     ")));
        cout << print_zmark(mkSrc) << "  ->  " << print_vpath(vpath, mkDst) << endl;
    };



    // Initialize pi0 with the boundary closure of the initial locations
    for (const Location &initLoc : zdta.locs) {
        if (!initLoc.isInitial || !initLocsUsed[initLoc.index])
            continue;

        // Explore locations reached by boundary edges from initLoc in c0
        queue<vector<location_t>> loc_queue;
        loc_queue.push({initLoc.index});
        while (!loc_queue.empty()) {
            const vector<location_t> vpath = loc_queue.front();
            const Location &lastLoc = zdta.locs[vpath.back()];
            loc_queue.pop();
            CheckPath(vpath);
            if (lastLoc.isTimed() || lastLoc.isFinal()) {
                zmrp.pi0[ locToTang[lastLoc.index] ] = 1.0;
                if (verboseLvl >= VL_VERBOSE)
                    cout << "  INIT:  " << print_vpath(vpath, locToTang[lastLoc.index]) << endl;
            }
            else {
                for (const DtaEdge &boundEdge : lastLoc.boundEdges) {
                    assert(boundEdge.isBoundary() && boundEdge.alpha == zdta.zones[initLoc.zone].cv1);
                    loc_queue.push(AppendPath(vpath, boundEdge.destLoc));
                }
            }
        }
    }

    // Build the MRP transition sets
    for (const Location &zloc : zdta.locs) {
        if (!zloc.isTimed() || zloc.isFinal())
            continue;

        queue<vector<location_t>> loc_queue;
        marking_t mkSrc = locToTang[zloc.index];

        // Always add the M_KO edge (rejected action/path)
        ((zloc.zone != zdta.cInf()) ? Qbar : Q)(mkSrc, BOT_IND) = 1.0;
        if (verboseLvl >= VL_VERBOSE)
            cout << "  M_KO:  " << print_zmark(mkSrc) << "  ->  [m2,(BOT)]" << endl;

        // Add M/M_RES/M_OK edges
        for (const DtaEdge &innerEdge : zloc.innerEdges) {
            if (!innerEdge.reset) {
                bool isPreempt = (zloc.zone != zdta.cInf() && zdta.locs[ innerEdge.destLoc ].isFinal());
                marking_t mkDst = locToTang[ innerEdge.destLoc ];
                (isPreempt ? Qbar : Q)(mkSrc, mkDst) = 1.0;
                if (verboseLvl >= VL_VERBOSE)
                    PrintTvtEdge('M', mkSrc, {innerEdge.destLoc}, isPreempt);
                continue;
            }
            loc_queue.push({innerEdge.destLoc});
            while (!loc_queue.empty()) {
                const vector<location_t> vpath = loc_queue.front();
                const Location &lastLoc = zdta.locs[vpath.back()];
                loc_queue.pop();
                CheckPath(vpath);
                if (lastLoc.isTimed() || lastLoc.isFinal()) {
                    bool isPreempt = (zloc.zone != zdta.cInf());
                    marking_t mkDst = locToTang[ lastLoc.index ];
                    (isPreempt ? Qbar : Q)(mkSrc, mkDst) = 1.0;
                    if (verboseLvl >= VL_VERBOSE)
                        PrintTvtEdge('M', mkSrc, vpath, isPreempt);
                }
                else {
                    for (const DtaEdge &boundEdge : lastLoc.boundEdges) {
                        assert(boundEdge.isBoundary());
                        assert(boundEdge.reset || boundEdge.alpha == zdta.zones[lastLoc.zone].cv1);
                        loc_queue.push(AppendPath(vpath, boundEdge.destLoc));
                    }
                }
            }
        }

        // Add Delta edges
        if (zloc.zone == zdta.cInf())
            continue;
        for (const DtaEdge &boundEdge : zloc.boundEdges) {
            queue<vector<location_t>> loc_queue;
            assert(boundEdge.isBoundary() && boundEdge.alpha == zdta.zones[zloc.zone].cv2);
            loc_queue.push({boundEdge.destLoc});
            while (!loc_queue.empty()) {
                const vector<location_t> vpath = loc_queue.front();
                const Location &lastLoc = zdta.locs[vpath.back()];
                loc_queue.pop();
                CheckPath(vpath);
                if (lastLoc.isTimed() || lastLoc.isFinal()) {
                    marking_t mkDst = locToTang[lastLoc.index];
                    Delta(mkSrc, mkDst) = 1.0;
                    if (verboseLvl >= VL_VERBOSE)
                        PrintTvtEdge('D', mkSrc, vpath, (lastLoc.zone == zdta.c0()));
                }
                else {
                    for (const DtaEdge &boundEdge : lastLoc.boundEdges) {
                        assert(boundEdge.isBoundary());
                        assert(boundEdge.reset || boundEdge.alpha == zdta.zones[lastLoc.zone].cv1);
                        loc_queue.push(AppendPath(vpath, boundEdge.destLoc));
                    }
                }
            }
        }
    }

    // Compress the 3 mapped matrices
    compress_matrix(zmrp.Q, Q);
    compress_matrix(zmrp.Qbar, Qbar);
    compress_matrix(zmrp.Delta, Delta);

    // Save back the map for: tangible location index -> ZDTA location index
    tangLocToZonedLoc = std::move(tangToLoc);

    if (verboseLvl >= VL_VERBOSE) {
        cout << "\n";
        PrintMRP(zmrp);
    }
}

//=============================================================================

std::function<ostream& (ostream &)>
print_sz(const sz_tuple_t &sz, const RG2 &rg, const DTA &zdta) {
    return [&](ostream & os) -> ostream& {
        if (sz.isBot())
            os << "<Bot>";
        else if (sz.isTop())
            os << "<Top>";
        else {
            const Location &loc = zdta.locs[sz.z];
            os << "<s" << 1 + size_t(sz.s) << "," << loc.label << ",";
            os << "[" << zdta.zones[loc.zone].v1 << "-";
            if (zdta.zones[loc.zone].v2 == numeric_limits<double>::max())
                os << "INF";
            else
                os << zdta.zones[loc.zone].v2;
            os << "]>";
        }
        return os;
    };
}

//=============================================================================

// Add a new <s,z> state in the state space of MxZ (if it is not already there),
// and return its index (of type szstate_t).
static
szstate_t GetOrInsertTangSZ(const sz_tuple_t &sz, SynchProd &MxZ,
                            BuildSynchProdHelper &build) {
    std::map<sz_tuple_t, szstate_t>::const_iterator x = MxZ.szToInd.find(sz);
    if (x != MxZ.szToInd.end())
        return x->second; // The tuple <s,z> already exists

    // Add a new tuple to the state space of MxZ
    szstate_t newInd = MxZ.SZ.size();
    MxZ.SZ.push_back(sz.s, sz.z);
    MxZ.szToInd[sz] = newInd;

    if (!sz.isBot() && !sz.isTop())
        build.state_queue.push(newInd);

    size_t N = MxZ.SZ.size();
    build.Q.enlarge(N, N);
    build.Qbar.enlarge(N, N);
    build.Delta.enlarge(N, N);

    return newInd;
}

//=============================================================================

// Closure of boundary edges for the MxZ product
static inline
sz_tuple_t closure_sz(sz_tuple_t sz, const rgstate_t stateIndex, const RG2 &rg,
                      const DTA &zdta, VerboseLevel verboseLvl,
                      size_t recursiveDepth = 0) {
    const size_t MAX_CLOSURE_RECURSIVE_DEPTH = 1000;
    if (recursiveDepth >= MAX_CLOSURE_RECURSIVE_DEPTH)
        throw program_exception("Loops of Boundary edges are not allowed.");

    const Location &z = zdta.locs[sz.z];
    const ClockZone &cz = zdta.zones[z.zone];

    // Test for final location
    if (z.finalType == FLT_FINAL_ACCEPTING)
        return sz_tuple_t::TOP;
    if (z.finalType == FLT_FINAL_REJECTING)
        return sz_tuple_t::BOT;

    // Test if there is a Boundary edge enabled in <s,z>
    for (const DtaEdge &boundEdge : z.boundEdges) {
        // Test clock zone c of z=<l,c>
        double alpha = boundEdge.alphaValue(zdta);
        assert(boundEdge.isBoundary() && alpha == boundEdge.betaValue(zdta));
        if (alpha == cz.v1) {
            // Test the state proposition expression of the destination location
            const Location &new_z = zdta.locs[boundEdge.destLoc];
            if (new_z.spExpr->EvaluateInRgState(stateIndex.index())) {
                // Recursively evaluate the closure in:  <s, newl, c[r:=0]>
                if (verboseLvl >= VL_VERBOSE)
                    cout << " " << print_sz(sz, rg, zdta) << "  ==[#]==>  ";
                return closure_sz(sz_tuple_t(sz.s, new_z.index), stateIndex, rg,
                                  zdta, verboseLvl, recursiveDepth + 1);
            }
        }
    }
    // The tuple <s,z> state is tangible.
    return sz;
}

// Closure of boundary edges for the MxZ product
static inline
szstate_t closure(sz_tuple_t sz, const rgstate_t stateIndex, const RG2 &rg,
                  const DTA &zdta, SynchProd &MxZ, BuildSynchProdHelper &build,
                  VerboseLevel verboseLvl) {
    // Add it to the state space of MxZ if necessary, and return its index.
    return GetOrInsertTangSZ(closure_sz(sz, stateIndex, rg, zdta, verboseLvl), MxZ, build);
}

//=============================================================================

bool ExtendSynchProduct(const RG2 &rg, const DTA &zdta, SynchProd &MxZ,
                        const std::vector<bool> &intLocs,
                        const std::vector<bool> &extLocs,
                        const std::map<sz_tuple_t, double> *pInitSZs,
                        VerboseLevel verboseLvl) {
    verify(rg.rgClass == RGC_CTMC);
    verify(zdta.isBound && zdta.isZoned);

    // Helper structure
    const marking_t NRG = rg.NumStates();
    const location_t NLOCS = zdta.locs.size();
    BuildSynchProdHelper build;
    long_interval_timer visitTimer;

    // Add the initial states to MxZ that are internal locations
    if (pInitSZs != nullptr) {
        // Initial states are specified directly.
        for (const pair<sz_tuple_t, double> &szp : *pInitSZs) {
            if (!intLocs[szp.first.z] || szp.second <= 0.0)
                continue;

            build.numInitSZs++;
            assert(zdta.locs[szp.first.z].isTimed());
            szstate_t szInit = GetOrInsertTangSZ(szp.first, MxZ, build);

            if (verboseLvl >= VL_VERBOSE) {
                cout << "INITIAL SxZ STATE s" << (1 + size_t(szInit)) << " IS ";
                cout << print_sz(MxZ.get_sz_tuple(szInit), rg, zdta);
                cout << " WITH PROB. " << szp.second << endl;
            }
        }
    }
    else {
        // Add all the possible CTMC states that satisfies all the internal location spExprs
        for (marking_t m(0); m < NRG; ++m) {
            rgstate_t stateIndex(m, TANGIBLE);
            for (location_t loc(0); loc < NLOCS; ++loc) {
                if (!intLocs[loc])
                    continue;
                assert(zdta.locs[loc].isTimed());
                if (!zdta.locs[loc].spExpr->EvaluateInRgState(stateIndex.index()))
                    continue;

                build.numInitSZs++;
                szstate_t szInit = GetOrInsertTangSZ(sz_tuple_t(m, loc), MxZ, build);
                if (verboseLvl >= VL_VERBOSE) {
                    cout << "INITIAL SxZ STATE s" << (1 + size_t(szInit)) << " IS ";
                    cout << print_sz(MxZ.get_sz_tuple(szInit), rg, zdta) << endl;
                }
            }
        }
    }

    if (build.numInitSZs == 0)
        return false;

    // Insert the top and bottom states
    MxZ.top = GetOrInsertTangSZ(sz_tuple_t::TOP, MxZ, build);
    MxZ.bot = GetOrInsertTangSZ(sz_tuple_t::BOT, MxZ, build);
    if (verboseLvl >= VL_VERBOSE)
        cout << "ADDING TOP STATE s" << size_t(MxZ.top) + 1 << " AND BOTTOM STATE s" << size_t(MxZ.bot) + 1 << "." << endl;

    if (verboseLvl >= VL_VERBOSE)
        cout << endl;

    // Explore the state space of MxZ from the initial states
    // up to the external frontier.
    RgEdge ctmcEdge;
    while (!build.state_queue.empty()) {
        const szstate_t srcInd = build.state_queue.front();
        assert(srcInd != MxZ.top && srcInd != MxZ.bot);
        const sz_tuple_t src_sz = MxZ.get_sz_tuple(srcInd);
        build.state_queue.pop();
        const Location &src_loc = zdta.locs[src_sz.z];
        const ClockZone &src_cz = zdta.zones[src_loc.zone];
        UNUSED_PARAM(src_cz);

        assert(intLocs[src_sz.z] || extLocs[src_sz.z]);
        if (extLocs[src_sz.z]) {
            // This <s,z> state is on the absorbing frontier of the state space of this component.
            if (verboseLvl >= VL_VERBOSE)
                cout <<  1 + size_t(srcInd) << ": TUPLE "
                     << print_sz(MxZ.get_sz_tuple(srcInd), rg, zdta) << " IS ON THE FRONTIER.\n\n";
            build.numExtStates++;
            continue;
        }
        build.numIntStates++;

        if (verboseLvl >= VL_VERBOSE)
            cout << 1 + size_t(srcInd) << ": FROM TUPLE "
                 << print_sz(MxZ.get_sz_tuple(srcInd), rg, zdta) << " DEPARTS:" << endl;

        // Try following every CTMC edge departing from s
        rgedge_t edgeList = rg.stateSets[TANGIBLE].get_first_edge(src_sz.s);
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
            szstate_t dstInd = MxZ.bot;
            bool isPreemptive = (src_loc.zone != zdta.cInf());
            for (const DtaEdge &innerEdge : src_loc.innerEdges) {
                assert(innerEdge.alphaValue(zdta) <= src_cz.v1 &&
                       src_cz.v2 <= innerEdge.betaValue(zdta));

                // Test action set
                bool trn_in_actset = false;
                for (const auto &actInd : innerEdge.actset.actInds) {
                    if (zdta.acts[actInd].boundedTrn == ctmcEdge.firedTrn) {
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
                const Location &dst_loc = zdta.locs[innerEdge.destLoc];
                marking_t dst_s(dstStateIndex.index());
                if (!dst_loc.spExpr->EvaluateInRgState(dstStateIndex.index()))
                    continue;

                // The CTMC transition has been accepted - test the finalType
                if (dst_loc.finalType == FLT_FINAL_ACCEPTING) { // M_OK transition
                    dstInd = MxZ.top;
                }
                else if (dst_loc.finalType == FLT_FINAL_REJECTING) { // M_KO transition
                    dstInd = MxZ.bot;
                }
                else if (innerEdge.reset) {
                    // M_RES transition
                    assert(dst_loc.zone == zdta.c0());
                    dstInd = closure(sz_tuple_t(dst_s, dst_loc.index),
                                     dstStateIndex, rg, zdta, MxZ, build, verboseLvl);
                }
                else {
                    // M transition
                    assert(dst_loc.zone == src_loc.zone);
                    dstInd = GetOrInsertTangSZ(sz_tuple_t(dst_s, dst_loc.index),
                                               MxZ, build);
                    isPreemptive = false;
                }
                break; // The CTMC transition has been accepted by a DTA edge
            }
            // If the loop ends without accepting, it is an M_KO edge

            // Add the transition from src to dst with Markovian delay
            double rate = ctmcEdge.delay * ctmcEdge.prob;
            (isPreemptive ? build.Qbar : build.Q)(srcInd, dstInd) += rate;
            build.Q(srcInd, srcInd) -= rate;

            if (verboseLvl >= VL_VERBOSE) {
                cout << "   " << print_sz(MxZ.get_sz_tuple(dstInd), rg, zdta) << endl;
            }

            edgeList = ctmcEdge.nextEdge;
        }

        // Evaluate time elapse arcs.
        if (src_loc.zone != zdta.cInf()) {
            assert(src_loc.boundEdges.size() == 1);
            const DtaEdge &boundEdge = src_loc.boundEdges[0];
            assert(boundEdge.alpha == src_cz.cv2);
            rgstate_t stateIndex(src_sz.s, TANGIBLE);

            if (verboseLvl >= VL_VERBOSE)
                cout << "   ==[D]==>  ";
            szstate_t dstInd = closure(sz_tuple_t(src_sz.s, boundEdge.destLoc),
                                       stateIndex, rg, zdta, MxZ, build, verboseLvl);

            // Add a Delta transition from src to dst
            build.Delta(srcInd, dstInd) = 1.0;

            if (verboseLvl >= VL_VERBOSE)
                cout << print_sz(MxZ.get_sz_tuple(dstInd), rg, zdta) << endl;
        }

        if (verboseLvl >= VL_VERBOSE)
            cout << endl;
        if (verboseLvl == VL_BASIC && visitTimer) {
            visitTimer.prepare_print();
            cout << "  <S,Z> STATES IN MxZ: " << setw(7) << left << MxZ.SZ.size();
            cout << "       " << endl;
        }
    }

    if (verboseLvl == VL_BASIC) {
        visitTimer.clear_any();
    }
    if (verboseLvl >= VL_VERBOSE)
        cout << endl;

    // Extract an MRP from the intermediate matrix_maps
    std::vector<statesubset_t> szToSubset(MxZ.SZ.size());
    std::map<clockzone_t, statesubset_t> cz2sub;
    cz2sub[zdta.cInf()] = EXP_SUBSET;
    for (szstate_t i(0); i < szstate_t(MxZ.SZ.size()); i++) {
        const sz_tuple_t &sz = MxZ.get_sz_tuple(i);
        // Top, bot and states on the frontier are considered to be in the EXP subset
        clockzone_t czone;
        if (sz.isBot() || sz.isTop() || extLocs[sz.z])
            czone = zdta.cInf();
        else
            czone = zdta.locs[sz.z].zone;
        if (cz2sub.count(czone) == 0) {
            GenMarkingSubset gms;
            gms.index     = statesubset_t(MxZ.mrp.Gen.size());
            gms.genTrnInd = transition_t(0);
            gms.detDelay  = zdta.zones[czone].duration();
            gms.fg        = DeterministicDurationFg(zdta.zones[czone].duration());
            MxZ.mrp.Gen.push_back(gms);
            cz2sub[czone] = gms.index;
        }
        szToSubset[i] = cz2sub[czone];
    }
    MxZ.mrp.Subsets.ConstructSubsetList(MxZ.mrp.Gen.size() + 2, szToSubset);

    // Compress the 3 mapped matrices
    compress_matrix(MxZ.mrp.Q, build.Q);
    compress_matrix(MxZ.mrp.Qbar, build.Qbar);
    compress_matrix(MxZ.mrp.Delta, build.Delta);

    if (verboseLvl >= VL_VERBOSE)
        PrintMRP(MxZ.mrp);

    if (verboseLvl >= VL_BASIC) {
        size_t numTimedZdtaLocs = std::count_if(zdta.locs.begin(), zdta.locs.begin(), 
                                                [&](const Location& l){ return l.isTimed(); });
        cout << "\n";
        cout << "NUMBER OF TANGIBLE <S*Z> STATES:     " << MxZ.mrp.N() << "\n";
        cout << "  INTERNAL <S*Z> STATES:             " << build.numIntStates << "\n";
        cout << "  EXTERNAL <S*Z> STATES:             " << build.numExtStates << "\n";
        cout << "NUMBER OF CTMC STATES:               " << rg.NumStates() << endl;
        cout << "NUMBER OF ZDTA LOCATIONS:            " << zdta.locs.size() << endl;
        cout << "  OF WHICH TIMED ZDTA LOCATIONS:     " << numTimedZdtaLocs << endl;
        cout << "NUMBER OF TIMED CLOCK ZONES:         " << zdta.zones.size() << endl;
        cout << "NUMBER OF DETERMINISTIC TRANSITIONS: " << MxZ.mrp.Gen.size() << endl;
        cout << "NONZERO ENTRIES IN Q:                " << MxZ.mrp.Q.nnz() << endl;
        cout << "NONZERO ENTRIES IN Qbar:             " << MxZ.mrp.Qbar.nnz() << endl;
        cout << "NONZERO ENTRIES IN DELTA:            " << MxZ.mrp.Delta.nnz() << endl;
        cout << "INITIAL <S*Z> STATES:                " << build.numInitSZs << endl;
        cout << endl;
    }


    return true;
}

//=============================================================================

// Location index processed by component method are for tangible locations only
// This method remaps tangible index in cd.(int/ext)CompStates into DTA
// location indexes in the (int/ext)Locs vectors.
bool TangibleLocIndexesToZDTALocIndexes(const DTA &zdta, const ComponentDesc &cd,
                                        const std::map<marking_t, location_t> &tangLocToZonedLoc,
                                        std::vector<bool> &intLocs, std::vector<bool> &extLocs) {
    // Convert from tangible loc index to ZDTA loc indexes
    intLocs.resize(zdta.locs.size());
    extLocs.resize(zdta.locs.size());
    std::fill(intLocs.begin(), intLocs.end(), false);
    std::fill(extLocs.begin(), extLocs.end(), false);
    bool hasOnlyFinalLoc = true;

    for (size_t i = 0; i < cd.intCompStates.size(); i++) {
        if (i == TOP_IND || i == BOT_IND)
            continue;
        auto tli = tangLocToZonedLoc.find(marking_t(i));
        verify(tli != tangLocToZonedLoc.end());
        intLocs[ tli->second ] = cd.intCompStates[i];
        extLocs[ tli->second ] = cd.extCompStates[i];
        if (cd.intCompStates[i] && !zdta.locs[tli->second].isFinal())
            hasOnlyFinalLoc = false;
    }
    return hasOnlyFinalLoc;
}

//=============================================================================

double
ModelCheck_CTMC_ZDTA_Forward_SCC(const RG2 &rg, const DTA &zdta,
                                 marking_t s0, const SolverParams &spar,
                                 VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("The synchronized product can be built only with a CTMC.");
    if (!zdta.isZoned)
        throw program_exception("Component analysis requires a zoned DTA.");


    // Find the initial zoned location of s0
    rgstate_t s0Index(s0, TANGIBLE);
    location_t z0(size_t(-1));
    vector<bool> initLocsUsed(zdta.locs.size(), false);
    for (const Location &loc : zdta.locs) {
        assert(!loc.isInitial || loc.zone == zdta.c0());
        if (loc.isInitial &&
                loc.spExpr->EvaluateInRgState(s0Index.index())) {
            z0 = loc.index;
            initLocsUsed[z0] = true;
            if (loc.isFinal()) {
                // The ZDTA starts in a final location
                if (verboseLvl >= VL_BASIC)
                    cout << "INITIAL CTMC STATE s" << 1 + size_t(s0)
                         << " STARTS IN A FINAL DTA LOCATION.\n" << endl;
                return (loc.finalType == FLT_FINAL_ACCEPTING) ? 1.0 : 0.0;
            }
            break;
        }
    }
    if (z0 == location_t(-1)) {
        // No initial location for s0
        if (verboseLvl >= VL_BASIC)
            cout << "INITIAL CTMC STATE s" << 1 + size_t(s0)
                 << " DOES NOT SATISFY ANY INITIAL DTA LOCATION.\n" << endl;
        return 0.0;
    }
    if (verboseLvl >= VL_VERBOSE)
        cout << "\nINITIAL TUPLE <s0,z0> IS: " << print_sz(sz_tuple_t(s0, z0), rg, zdta) << ".\n\n";

    // Prepare the component method
    MRP zmrp;
    std::map<marking_t, location_t> tangLocToZonedLoc;
    PrepareMRPStructureFromZonedDTA(zdta, zmrp, initLocsUsed, tangLocToZonedLoc, verboseLvl);
    MRPStateKinds stKinds;
    ClassifyMRPStates(zmrp, stKinds, verboseLvl);
    SimpleComponentProcessor compProc;
    ProcessComponentList(zmrp, stKinds, compProc, spar.aggregateComplexComp, 
                         spar.compMethod, spar.randomizeSccSelection, spar.rndSeed, VL_NONE);

    // Current probability distribution - starts in closure(<s0,z0>)
    std::map<sz_tuple_t, double> pi;
    if (verboseLvl >= VL_BASIC)
        cout << "INITIAL STATE IS: ";
    sz_tuple_t sz0tang = closure_sz(sz_tuple_t(s0, z0), s0Index, rg, zdta, verboseLvl);
    pi[sz0tang] = 1.0;
    if (verboseLvl >= VL_BASIC) {
        cout << print_sz(sz0tang, rg, zdta) << endl;
        cout << "\n\nFORWARD ON-THE-FLY STATE SPACE CONSTRUCTION FOLLOWING THE COMPONENT ORDER.\n" << endl;
    }

    // Process in forward sequence all the SCCs of the ZDTA
    double successProb = 0.0;
    std::vector<bool> intLocs, extLocs;
    for (const ComponentDesc &cd : compProc.comps) {
        // Convert from tangible loc index to ZDTA loc indexes
        if (TangibleLocIndexesToZDTALocIndexes(zdta, cd, tangLocToZonedLoc, intLocs, extLocs)) {
            if (verboseLvl >= VL_BASIC)
                cout << "\nSKIPPING COMPONENT NUM=" << cd.index + 1 << " MADE OF FINAL LOCATIONS." << endl;
            continue; // Nothing to process, only final locations
        }
        if (verboseLvl >= VL_BASIC)
            cout << print_compdesc(cd, "LOCATION", verboseLvl) << endl;

        // Construct the component state space
        SynchProd MxZ;
        if (!ExtendSynchProduct(rg, zdta, MxZ, intLocs, extLocs, &pi, verboseLvl)) {
            if (verboseLvl >= VL_BASIC)
                cout << "COMPONENT NUM=" << cd.index + 1 << " HAS NO INGOING "
                     "PROBABILITY (\'GHOST\' COMPONENT)." << endl;
            continue; // No initial states with nonzero probability.
        }

        // Initialize mrp.pi0
        MxZ.mrp.pi0.resize(MxZ.mrp.N());
        for (szstate_t sz(0); sz < szstate_t(MxZ.mrp.N()); sz++)
            if (pi.count(MxZ.get_sz_tuple(sz)) && intLocs[MxZ.get_sz_tuple(sz).z])
                MxZ.mrp.pi0[sz] = pi[ MxZ.get_sz_tuple(sz) ];

        // Process the component MRP
        performance_timer timer;
        PetriNetSolution outSol; // outgoing probability

        if (!spar.onTheFlyCompRecursive) {
            SolveComponent_Forward(MxZ.mrp, cd.topSubset, outSol, spar, verboseLvl);
        }
        else {
            // Re-apply the component method on the on-the-fly component
            MRPStateKinds stKinds;
            ClassifyMRPStates(MxZ.mrp, stKinds, verboseLvl);
            SteadyStateMrpBySccDecomp(MxZ.mrp, stKinds, outSol, spar, verboseLvl);
            // Rescale solution vector (steady-state normalizes it)
            outSol.stateProbs *= vec_sum(MxZ.mrp.pi0);
        }

        // Update the probability vector pi
        std::map<sz_tuple_t, double>::iterator it = pi.begin();
        while (it != pi.end()) {
            // Remove the probability that entered in this component.
            if (intLocs[it->first.z])
                pi.erase(it++);
            else
                ++it;
        }
        // Add the probability that reached the absorbing external frontier.
        for (size_t i = 0; i < outSol.stateProbs.size(); i++) {
            const sz_tuple_t sz_i = MxZ.get_sz_tuple(i);
            if (sz_i.isBot())
                continue;
            if (sz_i.isTop()) {
                successProb += outSol.stateProbs[i]; // Collect the success probability
                continue;
            }
            if (extLocs[sz_i.z] && outSol.stateProbs[i] >= 0.0)
                pi[ sz_i ] += outSol.stateProbs[i]; // Probability that exits this component
        }
        if (verboseLvl >= VL_BASIC)
            cout << "COMPONENT SOLUTION TIME: " << timer << endl;
    }

    if (verboseLvl >= VL_BASIC)
        cout << endl;
    verify(pi.size() == 0); // All states should have been processed.
    return successProb;
}

//=============================================================================

void
ModelCheck_CTMC_ZDTA_Backward_SCC(const RG2 &rg, const DTA &zdta,
                                  ublas::vector<double> &probs, const SolverParams &spar,
                                  VerboseLevel verboseLvl) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("The synchronized product can be built only with a CTMC.");
    if (!zdta.isZoned)
        throw program_exception("Component analysis requires a zoned DTA.");

    const marking_t NRG = rg.NumStates();
    probs.resize(NRG);
    std::fill(probs.begin(), probs.end(), 0.0);

    // Find all the initial states of the RG * ZDTA process
    std::vector<sz_tuple_t> initSZs(NRG, sz_tuple_t::BOT);
    vector<bool> initLocsUsed(zdta.locs.size(), false);
    for (marking_t m(0); m < NRG; ++m) {
        rgstate_t stateIndex(m, TANGIBLE);
        // Find the initial location that accepts m
        for (const Location &z : zdta.locs) {
            assert(!z.isInitial || z.zone == zdta.c0());
            if (z.isInitial && z.spExpr->EvaluateInRgState(stateIndex.index())) {
                if (verboseLvl >= VL_VERBOSE)
                    cout << "INITIAL STATE FOR m" << size_t(m) + 1 << " IS: ";
                sz_tuple_t sz_tang = closure_sz(sz_tuple_t(m, z.index), stateIndex, rg, zdta, verboseLvl);
                if (verboseLvl >= VL_VERBOSE)
                    cout << print_sz(sz_tang, rg, zdta) << endl;
                if (sz_tang == sz_tuple_t::TOP)
                    probs[m] = 1.0;  // The marking m is already accepted before starting.
                else if (!(sz_tang == sz_tuple_t::BOT)) {
                    initLocsUsed[z.index] = true;
                    initSZs[m] = sz_tang;
                }
                break;
            }
        }
    }
    if (initLocsUsed.empty()) {
        if (verboseLvl >= VL_BASIC)
            cout << "\nALL THE CTMC STATES START IN A FINAL ZDTA LOCATION.\n" << endl;
        return;
    }

    // Prepare the component method
    MRP zmrp;
    std::map<marking_t, location_t> tangLocToZonedLoc;
    PrepareMRPStructureFromZonedDTA(zdta, zmrp, initLocsUsed, tangLocToZonedLoc, verboseLvl);
    MRPStateKinds stKinds;
    ClassifyMRPStates(zmrp, stKinds, verboseLvl);
    SimpleComponentProcessor compProc;
    ProcessComponentList(zmrp, stKinds, compProc, spar.aggregateComplexComp, 
                         spar.compMethod, spar.randomizeSccSelection, spar.rndSeed, VL_NONE);

    // Current backward probability distributions - starts in TOP = 1
    std::map<sz_tuple_t, double> rho;
    rho[sz_tuple_t::TOP] = 1.0;
    if (verboseLvl >= VL_BASIC) {
        cout << "\n\nBACKWARD ON-THE-FLY STATE SPACE CONSTRUCTION FOLLOWING THE COMPONENT ORDER.\n" << endl;
    }

    // Process in backward sequence all the SCCs of the ZDTA
    std::vector<bool> intLocs, extLocs;
    std::vector<ComponentDesc>::const_reverse_iterator cdit;
    for (cdit = compProc.comps.rbegin(); cdit != compProc.comps.rend(); ++cdit) {
        const ComponentDesc &cd = *cdit;
        // Convert from tangible loc index to ZDTA loc indexes
        if (TangibleLocIndexesToZDTALocIndexes(zdta, cd, tangLocToZonedLoc, intLocs, extLocs)) {
            if (verboseLvl >= VL_BASIC)
                cout << "\nSKIPPING COMPONENT NUM=" << cd.index + 1 << " MADE OF FINAL LOCATIONS." << endl;
            continue; // Nothing to process, only final locations
        }
        if (verboseLvl >= VL_BASIC)
            cout << print_compdesc(cd, "LOCATION", verboseLvl) << endl;

        // Construct the component state space
        SynchProd MxZ;
        if (!ExtendSynchProduct(rg, zdta, MxZ, intLocs, extLocs, nullptr, verboseLvl)) {
            if (verboseLvl >= VL_BASIC)
                cout << "COMPONENT NUM=" << cd.index + 1 << " HAS NO STATES." << endl;
            continue;
        }

        // Initialize the reward vector
        ublas::vector<double> compRho(MxZ.mrp.N());
        std::fill(compRho.begin(), compRho.end(), 0.0);
        for (szstate_t sz_i(0); sz_i < szstate_t(MxZ.mrp.N()); sz_i++) {
            const sz_tuple_t sz = MxZ.get_sz_tuple(sz_i);
            if (sz.isBot() || sz.isTop() || (extLocs[sz.z] && rho.count(sz)))
                compRho[sz_i] = rho[sz];
        }

        // Process the component MRP
        performance_timer timer;
        PetriNetSolution outSol; // ingoing backward probability vector

        if (!spar.onTheFlyCompRecursive) {
            SolveComponent_Backward(MxZ.mrp, cd.topSubset, compRho, outSol, spar, verboseLvl);
        }
        else {
            // Re-apply the component method on the on-the-fly component
            MRPStateKinds stKinds;
            ClassifyMRPStates(MxZ.mrp, stKinds, verboseLvl);
            SteadyStateMrpBySccDecompBackward(MxZ.mrp, stKinds, compRho, outSol.stateProbs, spar, verboseLvl);
        }

        // Add the backward reward that arrives to internal initial states
        for (size_t i = 0; i < outSol.stateProbs.size(); i++) {
            const sz_tuple_t sz = MxZ.get_sz_tuple(i);
            if (sz.isBot() || sz.isTop())
                continue;
            if (intLocs[sz.z]) // && initLocInds.count(sz.s) == 1 && initLocInds[sz.s] == sz.z)
                rho[ sz ] += outSol.stateProbs[i]; // Probability that arrives backward to this state
        }
        if (verboseLvl >= VL_BASIC)
            cout << "COMPONENT SOLUTION TIME: " << timer << endl;
    }

    for (size_t m = 0; m < NRG; m++) {
        if (rho.count(initSZs[m]) > 0)
            probs[m] += rho[ initSZs[m] ];
    }

    if (verboseLvl >= VL_BASIC)
        cout << endl;
}

//=============================================================================











