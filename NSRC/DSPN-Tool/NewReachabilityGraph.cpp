/*
 *  NewReachabilityGraph.cpp
 *
 *  Implementation of the Reachability Graph building process.
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
#include <cfloat>
#include <cstring>
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
#include "PackedMemoryPool.h"
#include "compact_table.h"
#include "ReachabilityGraph.h"
#include "NewReachabilityGraph.h"

//=============================================================================

#include "compact_table_impl.h"

// template struct seq_store<std::set<transition_t>>;
template struct sparse_seq_store<SparseMarking>;
template class table < col::uint_col < size_t, +1 >>;
template class table<col::int_col<ssize_t>>;
template class table<col::indexed_col<double>>;
template class table<col::uint_col<size_t>>;

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
template class table < col::uint_col < transition_set_id, +1 >>;
template class table<col::uint_col<transition_t>>;
template class table < col::uint_col < vanish_path_set_id, +1 >>;
#endif

//=============================================================================
//    Internal Definitions needed by the RG building process only
//=============================================================================

struct VanishReductionEntry {
    VanishReductionEntry(rgstate_t _srcTangSt, rgedge_t _tv_edgeInd,
                         rgstate_t _firstVanish)
        : srcTangSt(_srcTangSt), tv_edgeInd(_tv_edgeInd), firstVanish(_firstVanish) { }
    rgstate_t		srcTangSt;
    rgedge_t		tv_edgeInd;
    rgstate_t		firstVanish;
};


// RG build helper structure
struct RgBuild : boost::noncopyable {
    // Erase vanishing markings after immediate reductions
    bool 				 eraseVanishingMarkings;

    // Shared buffer for edge preemption sets
    set<transition_t> 	 edgePreemptSet_buf;

    // Shared buffer for enabled immediate transitions
    set<transition_t> 	 enabledImm_buf;

    // Frontier in the RG build DFS visit
    queue<rgstate_t>	 		grayStates[NUM_RG_STATE_TYPES];
    queue<VanishReductionEntry> vanishReductionQueue;

    // Visiting set - avoids vanishing loops
    unordered_set<rgstate_t>    visitingVanish;
};

static void PrintMemoryUsage(const RG2 &rg);

//=============================================================================

const char *s_RgStateTypeNames[NUM_RG_STATE_TYPES] = {
    "VANISHING", "TANGIBLE"
};
const char *s_RgStateTypeNamesShort[NUM_RG_STATE_TYPES] = {
    "VANISH", "TANGIB"
};

//=============================================================================

void StateSet::clear(size_t numPlaces) {
    packedMarkings.clear(numPlaces);
    enabledGenSets.clear();
    firstEdge.clear();
}
void BaseEdgeSet::clear() {
    destState.clear();  firedTrn.clear();  preempted.clear();  nextEdge.clear();
}
size_t BaseEdgeSet::num_bytes() const {
    return destState.num_bytes() + firedTrn.num_bytes() +
           preempted.num_bytes() + nextEdge.num_bytes();
}
void TimedEdgeSet::clear() {
    BaseEdgeSet::clear();  delay.clear();
}
size_t TimedEdgeSet::num_bytes() const {
    return BaseEdgeSet::num_bytes() + delay.num_bytes();
}
void ImmediateEdgeSet::clear() {
    BaseEdgeSet::clear();  prob.clear();
}
size_t ImmediateEdgeSet::num_bytes() const {
    return BaseEdgeSet::num_bytes() + prob.num_bytes();
}
void TimedReduxEdgeSet::clear() {
    TimedEdgeSet::clear();  totProb.clear();  pathSet.clear();
}
size_t TimedReduxEdgeSet::num_bytes() const {
    return BaseEdgeSet::num_bytes() + totProb.num_bytes() + pathSet.num_bytes();
}

//=============================================================================

size_t RG2::NumEdges() const {
    return (timedEdgeSets[TANGIBLE].size() +
            timedEdgeSets[VANISHING].size() +
            immedEdgeSets[TANGIBLE].size() +
            timedEdgeSets[VANISHING].size() +
            timedReduxEdgeSet.size());
}

size_t RG2::NumStates() const {
    return stateSets[TANGIBLE].size() + stateSets[VANISHING].size();
}

//=============================================================================

RgEdge &GetEdgeByIndex(const RG2 &rg, rgedge_t edgeInd, RgEdge &edge) {
    edge.type = edgeInd.type();
    edge.selfIndex = edgeInd;
    size_t ind = edgeInd.index();
    switch (edge.type) {
    case TANGIBLE_TO_TANGIBLE:
    case TANGIBLE_TO_VANISHING: {
        RgStateType dstStType = (edge.type == TANGIBLE_TO_TANGIBLE
                                 ? TANGIBLE : VANISHING);
        const TimedEdgeSet &tes = rg.timedEdgeSets[dstStType];
        edge.dest      = tes.get_dest_state(ind);
        edge.firedTrn  = tes.get_fired_trn(ind);
        edge.preempted = tes.get_preempted(ind);
        edge.nextEdge  = tes.get_next_edge(ind);
        edge.delay     = tes.get_delay(ind);
        edge.prob      = 1.0;
        edge.pathSet   = EMPTY_VANISHING_PATH_SET;
        break;
    }
    case VANISHING_TO_TANGIBLE:
    case VANISHING_TO_VANISHING: {
        RgStateType dstStType = (edge.type == VANISHING_TO_TANGIBLE
                                 ? TANGIBLE : VANISHING);
        const ImmediateEdgeSet &ies = rg.immedEdgeSets[dstStType];
        edge.dest      = ies.get_dest_state(ind);
        edge.firedTrn  = ies.get_fired_trn(ind);
        edge.preempted = ies.get_preempted(ind);
        edge.nextEdge  = ies.get_next_edge(ind);
        edge.delay     = -1.0;
        edge.prob      = ies.get_prob(ind);
        edge.pathSet   = EMPTY_VANISHING_PATH_SET;
        break;
    }
    case TANGIBLE_TO_TANGIBLE_WITH_VANISHINGS: {
        const TimedReduxEdgeSet &tres = rg.timedReduxEdgeSet;
        edge.dest      = tres.get_dest_state(ind);
        edge.firedTrn  = tres.get_fired_trn(ind);
        edge.preempted = tres.get_preempted(ind);
        edge.nextEdge  = tres.get_next_edge(ind);
        edge.delay     = tres.get_delay(ind);
        edge.prob      = tres.get_total_prob(ind);
        edge.pathSet   = tres.get_vanish_path_set_id(ind);
        break;
    }
    default:
        assert(0);
        throw program_exception("Internal error: invalid edge type.");
    }
    return edge;
}

//=============================================================================

void PushBackEdge(RG2 &rg, rgstate_t fromSt, RgEdge &edge) {
    assert(edge.selfIndex == INVALID_RGEDGE_INDEX);
    assert(edge.nextEdge == INVALID_RGEDGE_INDEX);
    assert(edge.typeOfSrcState() == fromSt.type());
    assert(edge.typeOfDstState() == edge.dest.type());
    // assert(edge.preempted != EMPTY_TRANSITION_SET);

    edge.nextEdge = rg.stateSets[fromSt.type()].get_first_edge(fromSt.index());

    size_t index;
    switch (edge.type) {
    case TANGIBLE_TO_TANGIBLE:
    case TANGIBLE_TO_VANISHING: {
        RgStateType dstStType = (edge.type == TANGIBLE_TO_TANGIBLE
                                 ? TANGIBLE : VANISHING);
        TimedEdgeSet &tes = rg.timedEdgeSets[dstStType];
        index = tes.size();
        tes.destState.push_back(edge.dest.get_internal());
        tes.firedTrn.push_back(edge.firedTrn);
        tes.preempted.push_back(edge.preempted);
        tes.nextEdge.push_back(edge.nextEdge.get_internal());
        tes.delay.push_back(edge.delay);
        assert(edge.prob == 1.0);
        assert(edge.pathSet == EMPTY_VANISHING_PATH_SET);
        break;
    }
    case VANISHING_TO_TANGIBLE:
    case VANISHING_TO_VANISHING: {
        RgStateType dstStType = (edge.type == VANISHING_TO_TANGIBLE
                                 ? TANGIBLE : VANISHING);
        ImmediateEdgeSet &ies = rg.immedEdgeSets[dstStType];
        index = ies.size();
        ies.destState.push_back(edge.dest.get_internal());
        ies.firedTrn.push_back(edge.firedTrn);
        ies.preempted.push_back(edge.preempted);
        ies.nextEdge.push_back(edge.nextEdge.get_internal());
        ies.prob.push_back(edge.prob);
        assert(edge.delay <= 0.0);
        assert(edge.pathSet == EMPTY_VANISHING_PATH_SET);
        break;
    }
    case TANGIBLE_TO_TANGIBLE_WITH_VANISHINGS: {
        TimedReduxEdgeSet &tres = rg.timedReduxEdgeSet;
        index = tres.size();
        tres.destState.push_back(edge.dest.get_internal());
        tres.firedTrn.push_back(edge.firedTrn);
        tres.preempted.push_back(edge.preempted);
        tres.nextEdge.push_back(edge.nextEdge.get_internal());
        tres.delay.push_back(edge.delay);
        tres.totProb.push_back(edge.prob);
        tres.pathSet.push_back(edge.pathSet);
        break;
    }
    default:
        assert(0);
        throw program_exception("Internal error: invalid edge type.");
    }

    edge.selfIndex.set(index, edge.type);
    // Update the head of the linked list of edges departing from fromSt
    rg.stateSets[fromSt.type()].set_first_edge(fromSt.index(), edge.selfIndex);
}

//=============================================================================

RG2::RG2()
{ }

//=============================================================================

RG2::~RG2()
{ }

//=============================================================================

// Inverse of PackState (see below)
void
UnpackState(const RG2 &rg, const rgstate_t stateInd, RgState &rs) {
    // Unpack the sparse marking and the transition vectors
    rg.stateSets[stateInd.type()].packedMarkings
    .retrieve(packed_marking_id(stateInd.index()), rs.marking);
    rs.enabledGen.clear();
    rs.selfIndex = stateInd;
    rs.firstEdge = rg.stateSets[stateInd.type()].get_first_edge(stateInd.index());

    // Unpack the enabledGen set
    transition_set_id enabledGenSet = rg.stateSets[stateInd.type()].get_enabled_gen_set(stateInd.index());
    transition_set_table::const_seq_iterator it;
    for (it = rg.trnSetTbl.begin_seq(enabledGenSet); it != rg.trnSetTbl.end(); ++it)
        rs.enabledGen.insert(*it);
}

//=============================================================================

// Pack a RgState into a byte sequence allocated onto the memory_pool
static void
AppendState(RG2 &rg, const RgState &rs) {
    StateSet &stSet = rg.stateSets[rs.selfIndex.type()];
    assert(rs.selfIndex.index() == stSet.size());
    assert(rs.selfIndex.index() == stSet.packedMarkings.num_inserted_seq());
    assert(rs.selfIndex.index() == stSet.enabledGenSets.size());
    assert(rs.selfIndex.index() == stSet.firstEdge.size());

    // Insert the marking and the linked list head to the first edge
    stSet.packedMarkings.insert(rs.marking);
    stSet.firstEdge.push_back(rs.firstEdge.get_internal());

    // Insert the enabledGen set index
    transition_set_id enabledGenSet = rg.trnSetTbl.insert(rs.enabledGen);
    stSet.enabledGenSets.push_back(enabledGenSet);

#ifndef NDEBUG
    // Test correctness and invertibility of the Packing procedure
    RgState newRS;
    UnpackState(rg, rs.selfIndex, newRS);
    assert(rs.selfIndex == newRS.selfIndex);
    assert(rs.marking == newRS.marking);
    assert(rs.enabledGen == newRS.enabledGen);
    assert(rs.firstEdge == newRS.firstEdge);
#endif
}

//=============================================================================

void GetIthState(const RG2 &rg, size_t cnt, RgState &out_rs) {
    size_t numTang = rg.stateSets[TANGIBLE].size();
    RgStateType stype;
    if (cnt < numTang) {
        stype = TANGIBLE;
    }
    else {
        cnt -= numTang;
        assert(cnt < rg.stateSets[VANISHING].size());
        stype = VANISHING;
    }
    rgstate_t index(cnt, stype);
    UnpackState(rg, index, out_rs);
}


//=============================================================================

// Returns true if the specified state is already stored inside the
// knownStateHashes table. In that case, outIndexFound is set to the rgstate index
static inline bool
IsStateKnown(const RG2 &rg, const SparseMarking &sm, rgstate_t &outIndexFound) {
    for (size_t rst = 0; rst < NUM_RG_STATE_TYPES; rst++) {
        const RgStateType stType = RgStateType(rst);
        packed_marking_id out_id;
        bool ex = rg.stateSets[stType].packedMarkings.exists(out_id, sm.begin(), sm.end());
        if (ex) {
            outIndexFound.set(size_t(out_id), stType);
            return true;
        }
    }
    return false;
}

//=============================================================================

// #ifndef NDEBUG
// // Debug function used to test knownStateHashes table correctness
// static inline void
// AssertStateIsNew(const RG2& rg, const SparseMarking& sm)
// {
// 	rgstate_t index;
// 	if (IsStateKnown(rg, sm, index)) {
// 		throw program_exception("Internal error: wring marking hash function.");
// 	}
// }
// #else
// # define AssertStateIsNew(rg,sm)
// #endif

//=============================================================================

std::function<ostream& (ostream &)>
print_rgstate(const rgstate_t stInd) {
    return [ = ](ostream & os) -> ostream& {
        assert(stInd != INVALID_RGSTATE_INDEX);
        return os << (stInd.type() == TANGIBLE ? "M" : "V") << stInd.index() + 1;
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_reachable_state(const PN &pn, const RgState &rs) {
    return [&](ostream & os) -> ostream& {
        os << "STATE " << print_rgstate(rs.selfIndex);
        os << ": " << print_marking(pn, rs.marking);

        if (rs.enabledGen.size() > 0) {
            os << ", gen=";
            os << print_indexed_names(rs.enabledGen, pn.trns);
        }
        return os << endl;
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_transition_set(const PN &pn, const RG2 &rg, transition_set_id tsid) {
    return print_indexed_names(rg.trnSetTbl.begin_seq(tsid),
                               rg.trnSetTbl.end(), pn.trns);
}

//=============================================================================

std::function<ostream& (ostream &)>
print_vanish_path_set(const PN &pn, const RG2 &rg, const vanish_path_set_id vpsid) {
    return [ &, vpsid](ostream & os) -> ostream& {
        vanish_path_set_table::const_seq_iterator vps, vpsE(rg.vanPathSetTbl.end());
        for (vps = rg.vanPathSetTbl.begin_seq(vpsid); vps != vpsE; ++vps) {
            vanish_path_id vpid = *vps;
            double pathProb = 1.0;
            vanish_path_table::const_seq_iterator vp, vpE(rg.vanPathTbl.end());
            size_t cnt;
            for (cnt = 0, vp = rg.vanPathTbl.begin_seq(vpid); vp != vpE; ++vp, ++cnt) {
                cout << (cnt == 0 ? "      -VP-> " : " + ");
                cout << print_transition(pn, (*vp).immFiredTrn);
                pathProb *= (*vp).prob;
            }
            cout << "  vanish path prob=" << pathProb << endl;
        }
        return os;
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_rgedge(const PN &pn, const RG2 &rg, const rgstate_t fromSt, const RgEdge &e) {
    return [ &, fromSt](ostream & os) -> ostream& {
        const Transition &trn = pn.trns[e.firedTrn];
        os << "   " << print_rgstate(fromSt);
        os << " -[ " << trn.name << ", " << TrnDistrib_names[trn.distrib];
        if (e.hasDelay()) {
            cout << "(";
            if (trn.isGeneral())
                trn.delayFn->Print(cout, true);
            else
                cout << e.delay;
            cout << ")";
        }
        if (e.hasProb())
            os << ", prob=" << e.prob;
        if (e.preempted != EMPTY_TRANSITION_SET) {
            os << ", preempted=";
            os << print_transition_set(pn, rg, e.preempted);
        }
        os << " ]-> " << print_rgstate(e.dest);
        os << endl;
        if (e.hasVanishPaths())
            os << print_vanish_path_set(pn, rg, e.pathSet);
        return os;
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_outgoing_edges(const PN &pn, const RG2 &rg, const rgstate_t fromSt) {
    return [ &, fromSt](ostream & os) -> ostream& {
        RgEdge edge;
        rgedge_t listEdge = rg.stateSets[fromSt.type()].get_first_edge(fromSt.index());
        while (listEdge != INVALID_RGEDGE_INDEX) {
            GetEdgeByIndex(rg, listEdge, edge);
            cout << print_rgedge(pn, rg, fromSt, edge);
            listEdge = edge.nextEdge;
        }
        return os;
    };
}

//=============================================================================

void
PrintRG(const PN &pn, const RG2 &rg) {
    RgState rs;

    cout << "INITIAL " << s_RgStateTypeNames[rg.initState.selfIndex.type()];
    cout << " MARKING " << print_marking(pn, rg.initState.marking) << "\n\n";

//	if (IsIndexOfVanishingState(rg.initState.selfIndex)) {
    cout << "INITIAL TANGIBLE FRONTIER:\n";
    for (size_t i = 0; i < rg.initStates.size(); i++) {
        const InitialFrontierEntry &ife = rg.initStates[i];
        cout << "   " << print_rgstate(ife.state);
        cout << " with initial probability " << ife.initProb << endl;
        cout << print_vanish_path_set(pn, rg, ife.pathSet);
    }
    cout << endl;

    for (size_t type = 0; type < NUM_RG_STATE_TYPES; type++) {
        RgStateType rst = RgStateType(type);
        for (size_t s = 0; s < rg.stateSets[rst].size(); s++) {
            rgstate_t ind(s, rst);
            UnpackState(rg, ind, rs);
            cout << print_reachable_state(pn, rs);
            cout << print_outgoing_edges(pn, rg, rs.selfIndex) << endl;
        }
    }
}

//=============================================================================

// TODO: remove this
void CompareRGs(const PN &pn, const PETLT &petlt, RG2 &rg2, RgBuild &build2);

//=============================================================================

void
InsertInitialMarking(const PN &pn, const PETLT &petlt, RG2 &rg, RgBuild &build) {
    // Initial state setup
    vector<Place>::const_iterator plcit;
    rg.initState.marking.resize(pn.plcs.size());
    rg.initState.firstEdge = INVALID_RGEDGE_INDEX;
    for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit) {
        // Take the initial marking M0 of the Petri Net
        tokencount_t tc = plcit->getInitTokenCount();
        if (tc != tokencount_t(0))
            rg.initState.marking.insert_element(plcit->index, tc);
    }

    // Check if the initial marking is tangible or vanishing
    set<transition_t> enabledImm;
    double totalConEnabImmWeight = 0.0;
    ListEnabledTransitionsInMarking(pn, petlt, rg.initState.marking, IMM,
                                    enabledImm, &totalConEnabImmWeight);

    rg.initState.selfIndex.set(0, (enabledImm.size() > 0) ? VANISHING : TANGIBLE);
    ListEnabledTransitionsInMarking(pn, petlt, rg.initState.marking, DET,
                                    rg.initState.enabledGen, nullptr);
    AppendState(rg, rg.initState);

    build.grayStates[rg.initState.selfIndex.type()].push(rg.initState.selfIndex);

    if (rg.initState.selfIndex.type() == VANISHING && rg.isReduced) {
        build.vanishReductionQueue.push(VanishReductionEntry(INVALID_RGSTATE_INDEX,
                                        INVALID_RGEDGE_INDEX, rg.initState.selfIndex));
    }
    else {
        rg.initStates.push_back(InitialFrontierEntry(rg.initState.selfIndex, 1.0,
                                EMPTY_VANISHING_PATH_SET));
    }
}

//=============================================================================

static void
VisitVanishingPaths(const PN &pn, RG2 &rg, RgBuild &build,
                    const VanishReductionEntry &vre, double pathProb,
                    vector<VanishPathElem> &vanishPath,
                    vector<RgEdge> &trEdgeVec,
                    const set<transition_t> &preemptablesGen,
                    const set<transition_t> &preemptedGen,
                    rgstate_t vanishSt) {
    assert(vanishSt.type() == VANISHING);
    if (build.visitingVanish.count(vanishSt) > 0)
        throw program_exception("Vanishing loops are not supported.");
    if (build.visitingVanish.size() > 1000) { // TODO: should be a build parameter
        RgState rs;
        for (rgstate_t index : build.visitingVanish) {
            UnpackState(rg, index, rs);
            cout << print_reachable_state(pn, rs);
        }
        throw program_exception("Vanishing paths are too long.");
    }

    build.visitingVanish.insert(vanishSt);

    // Follow all the states reached by vanishSt outgoing V->V and V->T edges
    rgedge_t edgeList = rg.stateSets[vanishSt.type()].get_first_edge(vanishSt.index());
    while (edgeList != INVALID_RGEDGE_INDEX) {
        RgEdge edge;
        GetEdgeByIndex(rg, edgeList, edge);
        vanishPath.push_back(VanishPathElem(edge.firedTrn, edge.prob));
        double outgoingProb = pathProb * edge.prob;

        // Check if a new preemptable gen has been preempted by this edge
        // TODO: optimze??
        set<transition_t> newPreemptedGen = preemptedGen;
        transition_set_table::const_seq_iterator epIt, epItE = rg.trnSetTbl.end();
        for (epIt = rg.trnSetTbl.begin_seq(edge.preempted); epIt != epItE; ++epIt) {
            if (preemptablesGen.count(*epIt))
                newPreemptedGen.insert(*epIt);
        }

        if (edge.type == VANISHING_TO_VANISHING) {
            // Continue the top-down visit of the vanishing paths
            VisitVanishingPaths(pn, rg, build, vre, outgoingProb, vanishPath,
                                trEdgeVec, preemptablesGen, newPreemptedGen, edge.dest);
        }
        else {  // edge.type == VANISHING_TO_TANGIBLE
            // A tangible marking has been reached, ending the vanishing path
            // Allocate the vanishing path in the RG storage table
            // TODO: testare l'inserimento al contrario: end()->begin()
            vanish_path_id vpid = rg.vanPathTbl.insert(vanishPath);

            if (vre.tv_edgeInd == INVALID_RGEDGE_INDEX) { // Initial Vanishing
                // Search the edge in the initial tangible frontier vector
                RG2::initial_frontier_t::iterator it, itE(rg.initStates.end());
                for (it = rg.initStates.begin(); it != itE; ++it) {
                    if (it->state == edge.dest)
                        break;   // found.
                }
                if (it == itE) {
                    // Insert a new initial edge in the tangible frontier
                    rg.initStates.push_back(InitialFrontierEntry(edge.dest, 0.0,
                                            EMPTY_VANISHING_PATH_SET));
                    it = rg.initStates.end() - 1;
                }

                // Add the vanishing path (and prob) to the (new) reduced edge
                it->pathSet   = rg.vanPathSetTbl.prepend(vpid, it->pathSet);
                it->initProb += outgoingProb;
            }
            else {
                const rgedge_t startTVedgeInd = vre.tv_edgeInd;
                assert(startTVedgeInd.type() == TANGIBLE_TO_VANISHING);
                const TimedEdgeSet &start_tes = rg.timedEdgeSets[VANISHING];
                transition_t start_trn = start_tes.get_fired_trn(startTVedgeInd.index());
                double start_delay = start_tes.get_delay(startTVedgeInd.index());

                transition_set_id preempted;
                preempted = rg.trnSetTbl.insert(newPreemptedGen);

                // Find the reduced edge where this vanish path will be appended
                vector<RgEdge>::iterator trEdgeIt, trEdgeItE(trEdgeVec.end());
                for (trEdgeIt = trEdgeVec.begin(); trEdgeIt != trEdgeItE; ++trEdgeIt) {
                    RgEdge &p = *trEdgeIt;
                    if (p.firedTrn == start_trn &&
                            p.dest == edge.dest &&
                            p.preempted == preempted) {
                        // The edge already exist: add the probability
                        // and the new vanishing path
                        p.pathSet = rg.vanPathSetTbl.prepend(vpid, p.pathSet);
                        p.prob   += outgoingProb;
                        break;
                    }
                }
                if (trEdgeIt == trEdgeItE) {
                    // Insert a new reduced edge in the edge set, with
                    // outgoingProb probability and the explored vanishing path
                    RgEdge tre;
                    tre.selfIndex = INVALID_RGEDGE_INDEX;
                    tre.type      = TANGIBLE_TO_TANGIBLE_WITH_VANISHINGS;
                    tre.dest      = edge.dest;
                    tre.firedTrn  = start_trn;
                    tre.preempted = preempted;
                    tre.nextEdge  = INVALID_RGEDGE_INDEX;
                    tre.prob      = outgoingProb;
                    tre.delay     = start_delay;
                    tre.pathSet   = rg.vanPathSetTbl.prepend(vpid, EMPTY_VANISHING_PATH_SET);

                    trEdgeVec.push_back(tre);
                }
            }
        }

        vanishPath.pop_back();
        edgeList = edge.nextEdge;
    }

    build.visitingVanish.erase(vanishSt);
}

//=============================================================================

static
void ExtendRG(const PN &pn, const PETLT &petlt, RG2 &rg, RgBuild &build,
              const RgState &src_rs, RgEdge &edge, RgState &dst_rs) {
    assert(edge.dest == INVALID_RGSTATE_INDEX);
    assert(edge.selfIndex == INVALID_RGEDGE_INDEX);
    assert(src_rs.selfIndex != INVALID_RGSTATE_INDEX);
    assert((src_rs.selfIndex.type() == VANISHING && src_rs.selfIndex.type() == VANISHING) ||
           (src_rs.selfIndex.type() == TANGIBLE && src_rs.selfIndex.type() == TANGIBLE));

    dst_rs.selfIndex = INVALID_RGSTATE_INDEX;
    dst_rs.firstEdge = INVALID_RGEDGE_INDEX;

    // Fire the edge.firedTrn transition and compute the reached marking
    FireTransition(pn, edge.firedTrn, src_rs.marking, dst_rs.marking);

    // Get the list of general transitions with concession in dst_rs
    ListEnabledTransitionsInMarking(pn, petlt, dst_rs.marking, DET,
                                    dst_rs.enabledGen, nullptr);

    // Now determine if the reached state dst_rs is vanishing or tangible
    ListEnabledTransitionsInMarking(pn, petlt, dst_rs.marking, IMM,
                                    build.enabledImm_buf, nullptr);

    RgStateType dst_rsType = (build.enabledImm_buf.size() > 0) ? VANISHING : TANGIBLE;
    dst_rs.selfIndex.set(rg.stateSets[dst_rsType].size(), dst_rsType);

    // Determine if this edge does a preemption of a general transition
    // enabled in the source state.
    const set<transition_t> &previouslyEnabled = src_rs.enabledGen;
    if (previouslyEnabled.size() > 0) {
        build.edgePreemptSet_buf.clear();
        // Test if some general transitions have been preempted by the
        // firing of firedTrn, with regards to the enabledGen set
        set<transition_t>::const_iterator trnIt, trnItE = previouslyEnabled.end();
        for (trnIt = previouslyEnabled.begin(); trnIt != trnItE; ++trnIt) {
            transition_t genTrnInd = *trnIt;
            // The firing of a general transition is not a preemption
            if (genTrnInd == edge.firedTrn)
                continue;
            // Check if the previously enabled genTrnInd has become disabled
            if (dst_rs.enabledGen.count(genTrnInd) > 0)
                continue;   // still enabled...

            // genTrnInd is a newly disabled general transition
            build.edgePreemptSet_buf.insert(genTrnInd);
        }
        edge.preempted = rg.trnSetTbl.insert(build.edgePreemptSet_buf);
    }
    else
        edge.preempted = EMPTY_TRANSITION_SET;

    // Now determine if the reached state dst_rs already exists in the state set
    if (!IsStateKnown(rg, dst_rs.marking, dst_rs.selfIndex)) {
        // Insert the new reachable state entry
        AppendState(rg, dst_rs);
        // Add to the visit queue the new unexplored state
        build.grayStates[dst_rs.selfIndex.type()].push(dst_rs.selfIndex);
    }
    assert(dst_rs.selfIndex != INVALID_RGSTATE_INDEX &&
           dst_rs.selfIndex.index() < rg.stateSets[dst_rs.selfIndex.type()].size());

    edge.dest = dst_rs.selfIndex;
}

//=============================================================================

#ifndef NDEBUG

// Assert that the enabledGen set is consistent after an UnpackState
# define ASSERT_ENABLED_GEN_IS_CONSISTENT(rs)  do { \
		set<transition_t> testEnabledTrns; \
		ListEnabledTransitionsInMarking(pn, petlt, rs.marking, \
										DET, testEnabledTrns, nullptr); \
		assert(testEnabledTrns == rs.enabledGen); \
	} while(0)

// Assert that in state rs no immediate transition is enabled (rs is tangible)
# define ASSERT_NO_ENABLED_IMMEDIATE(rs)  do { \
		set<transition_t> testEnabledTrns; \
		ListEnabledTransitionsInMarking(pn, petlt, rs.marking, \
										IMM, testEnabledTrns, nullptr); \
		assert(testEnabledTrns.size() == 0); \
	} while(0)

#else
# define ASSERT_ENABLED_GEN_IS_CONSISTENT(rs)
# define ASSERT_NO_ENABLED_IMMEDIATE(rs)
#endif

void BuildRG2(const PN &pn, const PETLT &petlt, RG2 &rg, RG2BuildStat &stat,
              bool reduceImmediateTrns, bool storeVanishingPaths,
              bool eraseVanishingMarkings, VerboseLevel verboseLvl) {
    RgBuild build;

    rg.numPlaces      = pn.plcs.size();
    rg.isReduced      = reduceImmediateTrns;
    rg.hasVanishPaths = storeVanishingPaths;
    rg.rgClass        = RGC_CTMC;

    rg.stateSets[TANGIBLE].packedMarkings.clear(rg.numPlaces);
    rg.stateSets[VANISHING].packedMarkings.clear(rg.numPlaces);

    build.eraseVanishingMarkings = eraseVanishingMarkings;

    stat.numImmedFiredTrns  = 0;
    stat.numTimedFiredTrns  = 0;
    stat.maxGenTrnsConEnabl = 0;
    stat.numGenTrans        = 0;
    stat.numNonPreemtExp    = 0;
    stat.numPreemtExp       = 0;
    stat.numVanishingStates = 0;

    rg.trnDistribs.resize(pn.trns.size());
    for (size_t i = 0; i < pn.trns.size(); i++)
        rg.trnDistribs[i] = pn.trns[i].distrib;

    InsertInitialMarking(pn, petlt, rg, build);

    set<transition_t> enabledImm, enabledTimed;
    set<transition_t> preemptablesGen, preemptedGen;
    long_interval_timer visitTimer;
    RgState src_rs, dst_rs;  // Source and destination state
    RgEdge edge;
    vector<RgEdge> trEdgeVec;
    vector<VanishPathElem> vanishPath;

    // General events table
    for (const Transition& trn : pn.trns)
        if (trn.isGeneral())
            rg.generalFn.insert(make_pair(trn.index, trn.generalPdf()));

    for (;;) {
        //---------------------------------------------------------------------
        //  PROCESS IMMEDIATE TRANSITION FIRINGS FROM VANISHING STATES
        //---------------------------------------------------------------------
        if (build.grayStates[VANISHING].size() > 0) {
            // Unpack and process the next unvisited vanishing marking
            src_rs.selfIndex = build.grayStates[VANISHING].front();
            build.grayStates[VANISHING].pop();

            UnpackState(rg, src_rs.selfIndex, src_rs);
            ASSERT_ENABLED_GEN_IS_CONSISTENT(src_rs);

            // Get the list of currently enabled immediate transitions
            double totalImmWeight = 0.0;
            ListEnabledTransitionsInMarking(pn, petlt, src_rs.marking, IMM,
                                            enabledImm, &totalImmWeight);
            assert(enabledImm.size() > 0 && totalImmWeight > 0.0);

            set<transition_t>::const_iterator trnIt;
            for (trnIt = enabledImm.begin(); trnIt != enabledImm.end(); ++trnIt) {
                double weight  = pn.trns[*trnIt].weight(pn, src_rs.marking);
                edge.type      = INVALID_EDGE_TYPE;
                edge.selfIndex = INVALID_RGEDGE_INDEX;
                edge.dest      = INVALID_RGSTATE_INDEX;
                edge.firedTrn  = *trnIt;
                edge.prob      = weight / totalImmWeight;
                edge.preempted = EMPTY_TRANSITION_SET;
                edge.nextEdge  = INVALID_RGEDGE_INDEX;
                edge.delay     = -1.0;
                edge.pathSet   = EMPTY_VANISHING_PATH_SET;

                ExtendRG(pn, petlt, rg, build, src_rs, edge, dst_rs);
                edge.type = (dst_rs.selfIndex.type() == TANGIBLE ? VANISHING_TO_TANGIBLE :
                             VANISHING_TO_VANISHING);
                PushBackEdge(rg, src_rs.selfIndex, edge);
            }
            stat.numImmedFiredTrns += enabledImm.size();
            stat.numVanishingStates++;
        }
        //---------------------------------------------------------------------
        // REDUCE VANISHING PATHS
        //---------------------------------------------------------------------
        else if (build.vanishReductionQueue.size() > 0) {
            assert(rg.isReduced);
            assert(build.visitingVanish.size() == 0);
            const VanishReductionEntry &vre = build.vanishReductionQueue.front();

            // Determine the set of preemptables General transitions
            preemptablesGen.clear();
            preemptedGen.clear();
            if (vre.tv_edgeInd != INVALID_RGEDGE_INDEX) {  // Initial vanishing
                // TODO: this code could be optimized
                transition_set_id src_enabledGenSet;
                src_enabledGenSet = rg.stateSets[vre.srcTangSt.type()].get_enabled_gen_set(vre.srcTangSt.index());
                preemptablesGen.insert(rg.trnSetTbl.begin_seq(src_enabledGenSet),
                                       rg.trnSetTbl.end());

                // Add all the General transitions preempted by the firing of
                // the timed transition vre.timedEdge
                assert(vre.tv_edgeInd.type() == TANGIBLE_TO_VANISHING);
                const TimedEdgeSet &src_tes = rg.timedEdgeSets[VANISHING];
                transition_t src_trn = src_tes.get_fired_trn(vre.tv_edgeInd.index());
                transition_set_id src_preempted = src_tes.get_preempted(vre.tv_edgeInd.index());

                // Fired transition is not a preemption (if firedTrn is Gen)
                preemptablesGen.erase(src_trn);

                transition_set_table::const_seq_iterator ptIt, ptItE;
                ptItE = rg.trnSetTbl.end();
                for (ptIt = rg.trnSetTbl.begin_seq(src_preempted); ptIt != ptItE; ++ptIt) {
                    transition_t genTrnInd = *ptIt;
                    if (preemptablesGen.count(genTrnInd) == 0)
                        continue; // genTrnInd was not enabled in srcTangSt
                    preemptedGen.insert(genTrnInd);
                }
            }

            // Visit all the vanishing paths that starts from vre
            trEdgeVec.clear();
            VisitVanishingPaths(pn, rg, build, vre, 1.0, vanishPath, trEdgeVec,
                                preemptablesGen, preemptedGen, vre.firstVanish);

            // Now insert all the new T->V*->T edges into the RG edgeSet
            for (size_t i = 0; i < trEdgeVec.size(); i++) {
                PushBackEdge(rg, vre.srcTangSt, trEdgeVec[i]);
            }

            build.vanishReductionQueue.pop();
        }
        //---------------------------------------------------------------------
        // PROCESS TANGIBLE STATES
        //---------------------------------------------------------------------
        else if (build.grayStates[TANGIBLE].size() > 0) {
            // Process next (unvisited) tangible state
            src_rs.selfIndex = build.grayStates[TANGIBLE].front();
            build.grayStates[TANGIBLE].pop();

            UnpackState(rg, src_rs.selfIndex, src_rs);
            ASSERT_ENABLED_GEN_IS_CONSISTENT(src_rs);
            ASSERT_NO_ENABLED_IMMEDIATE(src_rs);

            size_t numGenEnabled = src_rs.enabledGen.size();
            if (numGenEnabled > 1)
                rg.rgClass = RGC_GeneralizedSemiMarkov;
            else if (rg.rgClass == RGC_CTMC && numGenEnabled == 1)
                rg.rgClass = RGC_MarkovRenewalProcess;

            // Get the list of currently enabled exponential transitions
            ListEnabledTransitionsInMarking(pn, petlt, src_rs.marking, EXP,
                                            enabledTimed, nullptr);
            enabledTimed.insert(src_rs.enabledGen.begin(),
                                src_rs.enabledGen.end());

            set<transition_t>::const_iterator trnIt;
            for (trnIt = enabledTimed.begin(); trnIt != enabledTimed.end(); ++trnIt) {
                edge.type      = INVALID_EDGE_TYPE;
                edge.selfIndex = INVALID_RGEDGE_INDEX;
                edge.dest      = INVALID_RGSTATE_INDEX;
                edge.firedTrn  = *trnIt;
                edge.delay     = pn.trns[edge.firedTrn].delay(pn, src_rs.marking);
                edge.prob      = 1.0;
                edge.preempted = EMPTY_TRANSITION_SET;
                edge.nextEdge  = INVALID_RGEDGE_INDEX;
                edge.pathSet   = EMPTY_VANISHING_PATH_SET;

                ExtendRG(pn, petlt, rg, build, src_rs, edge, dst_rs);
                edge.type = (dst_rs.selfIndex.type() == TANGIBLE ? TANGIBLE_TO_TANGIBLE :
                             TANGIBLE_TO_VANISHING);
                PushBackEdge(rg, src_rs.selfIndex, edge);

                // Add the vanishing reduction rule if this is a T->V edge
                if (rg.isReduced && src_rs.selfIndex.type() == TANGIBLE &&
                        dst_rs.selfIndex.type() == VANISHING) {
                    build.vanishReductionQueue.push(VanishReductionEntry(src_rs.selfIndex,
                                                    edge.selfIndex, dst_rs.selfIndex));
                }
            }
            stat.numTimedFiredTrns += enabledTimed.size();
        }
        //---------------------------------------------------------------------
        // NO MORE MARKINGS TO PROCESS IN THE VISIT QUEUE
        //---------------------------------------------------------------------
        else {
            // TODO: try to understand why this piece of code cannot be reliably
            //       moved just after the VRE queue reduction step. See test08,
            //       which fails when this optimization is done.
            if (reduceImmediateTrns && eraseVanishingMarkings) {
                // We can safely remove all the vanishing markings and immed. edges
                rg.stateSets[VANISHING].clear(rg.numPlaces);
                std::function<rgedge_t(const rgedge_t)> removeImmedEdgesInLinkedList;
                removeImmedEdgesInLinkedList = [&](const rgedge_t edgeList) -> rgedge_t {
                    if (edgeList == INVALID_RGEDGE_INDEX)
                        return INVALID_RGEDGE_INDEX;
                    switch (edgeList.type()) {
                    case TANGIBLE_TO_TANGIBLE_WITH_VANISHINGS: // ok
                        rg.timedReduxEdgeSet.set_next_edge(edgeList.index(),
                        removeImmedEdgesInLinkedList(rg.timedReduxEdgeSet.get_next_edge(edgeList.index())));
                        return edgeList;

                    case TANGIBLE_TO_TANGIBLE: // ok
                        rg.timedEdgeSets[TANGIBLE].set_next_edge(edgeList.index(),
                        removeImmedEdgesInLinkedList(rg.timedEdgeSets[TANGIBLE].get_next_edge(edgeList.index())));
                        return edgeList;

                    case TANGIBLE_TO_VANISHING: // drop
                        return removeImmedEdgesInLinkedList(rg.timedEdgeSets[VANISHING].get_next_edge(edgeList.index()));

                    case VANISHING_TO_TANGIBLE: // drop
                        return removeImmedEdgesInLinkedList(rg.immedEdgeSets[TANGIBLE].get_next_edge(edgeList.index()));

                    case VANISHING_TO_VANISHING: // drop
                        return removeImmedEdgesInLinkedList(rg.immedEdgeSets[VANISHING].get_next_edge(edgeList.index()));

                    default:
                        throw program_exception("removeImmedEdgesInLinkedList(): internal error.");
                    }
                };
                // Unlink V->V, T->V and V->T edges from the linked lists of tangible states
                for (size_t s = 0; s < rg.stateSets[TANGIBLE].size(); s++) {
                    rg.stateSets[TANGIBLE].set_first_edge(s,
                                                          removeImmedEdgesInLinkedList(rg.stateSets[TANGIBLE].get_first_edge(s)));
                }

                rg.timedEdgeSets[VANISHING].clear();
                rg.immedEdgeSets[TANGIBLE].clear();
                rg.immedEdgeSets[VANISHING].clear();
            }
            if (!rg.isReduced && rg.stateSets[VANISHING].size() > 0) {
                rg.rgClass = RGC_WithVanishingMarkings;
            }
            if (verboseLvl == VL_BASIC) {
                visitTimer.clear_any();
            }
            break;
        }

        if (verboseLvl == VL_BASIC && visitTimer) {
            visitTimer.prepare_print();
            cout << "TANGIBLE: " << setw(7) << left << rg.stateSets[TANGIBLE].size();
            cout << " VANISHING: " << setw(7) << left << rg.stateSets[VANISHING].size();
            cout << " IMMEDIATE: " << setw(7) << left << stat.numImmedFiredTrns;
            cout << " TIMED: " << setw(7) << left << stat.numTimedFiredTrns;
            cout << "    " << endl;
        }
    }

    // Preprocess measures
    PreprocessMeasures(pn, rg, verboseLvl);

    // Print the RG
    if (verboseLvl >= VL_VERBOSE) {
        PrintRG(pn, rg);
    }

#ifndef NDEBUG
    if (rg.isReduced) {
        cout << endl << endl;
        cout << "COMPARING THE TWO RG CONSTRUCTION METHODS..." << endl;
        CompareRGs(pn, petlt, rg, build);
        cout << "COMPARISON IS OK." << endl;
    }
#endif

    rg.stateSets[TANGIBLE].packedMarkings.freeze();
    rg.stateSets[VANISHING].packedMarkings.freeze();

    // Print statistics
    if (verboseLvl >= VL_BASIC) {
        cout << "\n\n";
        cout << "TANGIBLE STATES:             " << setw(9) << right << rg.stateSets[TANGIBLE].size() << endl;
        cout << "VANISHING STATES:            " << setw(9) << right << rg.stateSets[VANISHING].size() << endl;
        size_t numTT = rg.timedEdgeSets[TANGIBLE].size();
        size_t numTV = rg.timedEdgeSets[VANISHING].size();
        size_t numVT = rg.immedEdgeSets[TANGIBLE].size();
        size_t numVV = rg.immedEdgeSets[VANISHING].size();
        size_t numTVxT = rg.timedReduxEdgeSet.size();
        cout << "NUMBER OF TIMED EDGES:       " << setw(9) << right << numTT + numTV + numTVxT;
        cout << "  (" << numTT << " T->T, " << numTV << " T->V, ";
        cout << numTVxT << " T->V*->T)." << endl;
        cout << "NUMBER OF IMMEDIATE EDGES:   " << setw(9) << right << numVT + numVV;
        cout << "  (" << numVT << " V->T, " << numVV << " V->V)." << endl;
        cout << "TOTAL NUMBER OF EDGES:       " << setw(9) << right << numTT + numTV + numVV + numVT + numTVxT << endl;
        cout << "IMMEDIATE TRNS. FIRINGS:     " << setw(9) << right << stat.numImmedFiredTrns << endl;
        cout << "TIMED TRNS. FIRINGS:         " << setw(9) << right << stat.numTimedFiredTrns << endl;
        cout << "VISITED VANISHING STATES:    " << setw(9) << right << stat.numVanishingStates << endl;
        for (size_t s = 0; s < rg.stateSets[TANGIBLE].size(); ++s) {
            rgedge_t edgeList = rg.stateSets[TANGIBLE].get_first_edge(s);
            while (edgeList != INVALID_RGEDGE_INDEX) {
                GetEdgeByIndex(rg, edgeList, edge);
                switch (pn.trns[edge.firedTrn].distrib) {
                case IMM:  break;
                case DET:  stat.numGenTrans++;
                case EXP:
                    if (rg.trnSetTbl.get_length(edge.preempted) > 0)
                        stat.numPreemtExp++;
                    else stat.numNonPreemtExp++;
                    break;
                default: assert(0);
                }
                edgeList = edge.nextEdge;
            }
        }
        cout << "GENERAL TRANSITIONS:         " << setw(9) << right << stat.numGenTrans << endl;
        cout << "NON-PREEMPTIVE EXP. TRNS.:   " << setw(9) << right << stat.numNonPreemtExp << endl;
        cout << "PREEMPTIVE EXP. TRANSITIONS: " << setw(9) << right << stat.numPreemtExp << endl;
        PrintMemoryUsage(rg);
    }
}

//=============================================================================

static void PrintMemoryUsage(const RG2 &rg) {
    // Memory usage statistics:
    cout << "MEMORY USAGE:" << endl;
    cout << "TRANSITION-SET ENTRIES:      " << setw(9) << right << rg.trnSetTbl.size() << endl;
    cout << "VANISH-PATH ENTRIES:         " << setw(9) << right << rg.vanPathTbl.size() << endl;
    cout << "VANISH-PATH-SET ENTRIES:     " << setw(9) << right << rg.vanPathSetTbl.size() << endl;
    size_t edgeBytes = 0, pMarkBytes = 0, stSetBytes = 0;
    pMarkBytes = rg.stateSets[TANGIBLE].packedMarkings.num_bytes() +
                 rg.stateSets[VANISHING].packedMarkings.num_bytes();
    cout << "PACKED MARKINGS:             " << setw(9) << right << pMarkBytes << " BYTES" << endl;
    stSetBytes += rg.stateSets[TANGIBLE].enabledGenSets.num_bytes() +
                  rg.stateSets[VANISHING].enabledGenSets.num_bytes() +
                  rg.stateSets[TANGIBLE].firstEdge.num_bytes() +
                  rg.stateSets[VANISHING].firstEdge.num_bytes();
    cout << "STATE SET DATA:              " << setw(9) << right << stSetBytes << " BYTES" << endl;
    edgeBytes += rg.timedEdgeSets[TANGIBLE].num_bytes();
    edgeBytes += rg.timedEdgeSets[VANISHING].num_bytes();
    edgeBytes += rg.immedEdgeSets[TANGIBLE].num_bytes();
    edgeBytes += rg.immedEdgeSets[VANISHING].num_bytes();
    edgeBytes += rg.timedReduxEdgeSet.num_bytes();
    cout << "EDGES DATA:                  " << setw(9) << right << edgeBytes << " BYTES" << endl;

    cout << "\nPACKED TANGIBLE MARKINGS TABLE:" << endl;
    rg.stateSets[TANGIBLE].packedMarkings.print_memory_occupation();
    if (rg.stateSets[VANISHING].packedMarkings.num_inserted_seq() > 0) {
        cout << "\nPACKED VANISHING MARKINGS TABLE:" << endl;
        rg.stateSets[VANISHING].packedMarkings.print_memory_occupation();
    }
}

//=============================================================================

void
CompareRGs(const PN &pn, const PETLT &petlt, RG2 &rg2, RgBuild &build2) {
    RG rg1;
    RGBuildStat stat1;
    BuildRG(pn, petlt, rg1, !rg2.isReduced, true, stat1, VL_NONE);

    if (rg1.markSet.size() != rg2.stateSets[TANGIBLE].size() + rg2.stateSets[VANISHING].size())
        throw program_exception("Wrong RG sizes.");

    if (rg1.rgClass != rg2.rgClass)
        throw program_exception("Different RG classes.");

    // From RG marking_t indexes to RG2 rgstate_t indices
    map<marking_t, rgstate_t> conv1to2;

    for (size_t m = 0; m < rg1.markSet.size(); m++) {
        const ReachableMarking &rm1 = rg1.markSet[m];

        RgState rs2;
        rs2.selfIndex       = rm1.isVanishing ? INVALID_VANISHING_RGSTATE_INDEX
                              : INVALID_TANGIBLE_RGSTATE_INDEX;
        rs2.marking         = rm1.marking;
        rs2.enabledGen.insert(rm1.enabledDetInds.begin(), rm1.enabledDetInds.end());
        rs2.firstEdge       = INVALID_RGEDGE_INDEX;
        rgstate_t index2 = INVALID_RGSTATE_INDEX;
        bool isKnown = IsStateKnown(rg2, rs2.marking, index2);
        if (!isKnown) {
            cout << "  NO MATCH FOR M" << 1 + size_t(rm1.index) << endl;
            throw program_exception("states do not match.");
        }
        conv1to2[m] = index2;
    }
    cout << " [*] ALL " << rg1.markSet.size() << " STATES MATCH." << endl;


    for (size_t e1 = 0, e2; e1 < rg1.initMark.timedPaths.size(); e1++) {
        const RGArc &arc1 = rg1.initMark.timedPaths[e1];
        for (e2 = 0; e2 < rg2.initStates.size(); e2++) {
            const InitialFrontierEntry &ife = rg2.initStates[e2];
            // TODO: we are not comparing the initial vanishing paths
            if (ife.initProb == arc1.prob && ife.state == conv1to2[arc1.destMarking])
                break;
        }
        if (e2 == rg2.initStates.size()) {
            cout << "INITIAL EDGE:  M" << 1 + size_t(arc1.destMarking) << " [";
            cout << print_rgstate(conv1to2[arc1.destMarking]);
            cout << "], prob = " << arc1.prob << " DOES NOT MATCH:" << endl;
            for (e2 = 0; e2 < rg2.initStates.size(); e2++) {
                const InitialFrontierEntry &ife = rg2.initStates[e2];
                cout << "    " << print_rgstate(ife.state) << ", prob=" << ife.initProb << endl;
            }
            throw program_exception("initial distributions do not match.");
        }
    }
    cout << " [*] ALL " << rg1.initMark.timedPaths.size() << " INITIAL MARKINGS MATCH." << endl;


    size_t matchedVPs = 0;
    for (size_t m = 0; m < rg1.markSet.size(); m++) {
        const ReachableMarking &rm1 = rg1.markSet[m];
        rgstate_t index2_from = conv1to2[m];

        // Search for corresponding edges
        bool found = false;
        for (size_t e = 0; e < rm1.timedPaths.size(); e++) {
            const RGArc &arc1 = rm1.timedPaths[e];
            set<transition_t> arc1_preempted = arc1.preempted;

            // Search for a matching edge in the RG2 edgeSet
            rgedge_t edgeList2 = rg2.stateSets[index2_from.type()].get_first_edge(index2_from.index());
            while (!found && edgeList2 != INVALID_RGEDGE_INDEX) {
                RgEdge edge2;
                GetEdgeByIndex(rg2, edgeList2, edge2);
                edgeList2 = edge2.nextEdge;

                if (edge2.isFromVanishingState() && arc1.distrib != IMM)
                    continue;
                if (!edge2.isFromVanishingState() && arc1.distrib == IMM)
                    continue;
                if (arc1.timedTrnInd != edge2.firedTrn ||
                        conv1to2[arc1.destMarking] != edge2.dest ||
                        !equal_safe(arc1.preempted.begin(), arc1.preempted.end(),
                                    rg2.trnSetTbl.begin_seq(edge2.preempted),
                                    rg2.trnSetTbl.end()))
                    continue;
                if (edge2.hasDelay() && edge2.delay != arc1.delay)
                    continue;
                if (edge2.hasProb() && edge2.prob != arc1.prob)
                    continue;
                if (edge2.hasVanishPaths()) {
                    // Compare each vanishing path
                    list<VanishingPath>::const_iterator vp1, vp1E(arc1.vanishingPaths.end());
                    for (vp1 = arc1.vanishingPaths.begin(); vp1 != vp1E; ++vp1) {
                        // Find a matching VP in RG2 path set
                        bool vp_matches = false;
                        vanish_path_set_table::const_seq_iterator vp2, vp2E(rg2.vanPathSetTbl.end());
                        for (vp2 = rg2.vanPathSetTbl.begin_seq(edge2.pathSet); vp2 != vp2E; ++vp2) {
                            list<VanishingMarking>::const_iterator vm1(vp1->path.begin());
                            list<VanishingMarking>::const_iterator vm1E(vp1->path.end());
                            vanish_path_table::const_seq_iterator vm2(rg2.vanPathTbl.begin_seq(*vp2));
                            vanish_path_table::const_seq_iterator vm2E(rg2.vanPathTbl.end());
                            double prob2 = 1.0;

                            while (vm1 != vm1E && vm2 != vm2E) {
                                if (vm1->immTrnInd != vm2->immFiredTrn)
                                    break; // different VP
                                prob2 *= vm2->prob;
                                ++vm1; ++vm2;
                            }
                            if (prob2 == vp1->pathProb && vm1 == vm1E && vm2 == vm2E) {
                                vp_matches = true;
                                matchedVPs++;
                                break;
                            }
                        }
                        if (!vp_matches) {
                            throw program_exception("VP do not match!");
                        }
                    }
                }

                found = true; // ok, these two edges match
            }

            if (!found) {
                cout << "RG1 ARC: src=M" << marking_t(1) + (rm1.index) << " [";
                cout << print_rgstate(conv1to2[rm1.index]);
                cout << "] trn=" << print_transition(pn, arc1.timedTrnInd);
                cout << " preempt=" << print_indexed_names(arc1.preempted, pn.trns);
                cout << " delay=" << arc1.delay;
                cout << " prob=" << arc1.prob;
                cout << " dest=M" << marking_t(1) + (arc1.destMarking) << " [";
                cout << print_rgstate(conv1to2[arc1.destMarking]);
                cout << "]" << endl;
                cout << "HAS NO MATCH WITH THESE RG2 EDGEs:" << endl;
                cout << print_outgoing_edges(pn, rg2, index2_from) << endl;
                throw program_exception("No match between edges.");
            }
        }
    }
    cout << " [*] ALL " << rg2.NumEdges() << " EDGES MATCH." << endl;
    cout << " [*] ALL " << matchedVPs << " VANISHING PATHS MATCH." << endl;
}

//=============================================================================

void LoadMRMC_Model(ifstream &ifsTra, ifstream &ifsLab,
                    PN &pn, RG2 &rg, VerboseLevel verboseLvl) {
    simple_tokenizer tra(&ifsTra);
    simple_tokenizer lab(&ifsLab);
    string label;
    size_t numStates, numEdges;
    tra.readtok("STATES").read(numStates).readtok("TRANSITIONS").read(numEdges);

    // Load the MRMC definitions as places and transitions
    transition_t expTrn = LoadMRMC_ModelAsPN(tra, lab, pn, verboseLvl);

    // Prepare the RG
    RgBuild build;
    rg.numPlaces      = pn.plcs.size();
    rg.isReduced      = true;
    rg.hasVanishPaths = false;
    rg.rgClass        = RGC_CTMC;

    rg.stateSets[TANGIBLE].packedMarkings.clear(rg.numPlaces);
    rg.stateSets[VANISHING].packedMarkings.clear(rg.numPlaces);

    // Load markings into the RG
    string line;
    lab.skiplines(1);
    bool hasInit = false;
    for (size_t i = 0; i < numStates; i++) {
        lab.getline(line);
        istringstream iss(line);
        size_t pos;
        iss >> pos;
        pos--;
        if (!iss)
            pos = numStates;
        else if (pos < i || pos >= numStates)
            throw program_exception(lab.pos() + ": Unexpected state index in the .lab file.");
        while (i < pos) {
            // Insert empty states until pos
            RgState state;
            state.selfIndex.set(i, TANGIBLE);
            state.marking.resize(pn.plcs.size());
            state.firstEdge = INVALID_RGEDGE_INDEX;
            AppendState(rg, state);

            i++;
        }

        RgState state;
        state.selfIndex.set(i, TANGIBLE);
        state.marking.resize(pn.plcs.size());
        state.firstEdge = INVALID_RGEDGE_INDEX;
        bool isInit = false;
        iss >> label;
        while (iss) {
            auto plcInd = pn.plcInds.find(label);
            if (plcInd == pn.plcInds.end())
                throw program_exception(lab.pos() + ": CTMC variable " + plcInd->first + " does not exist.");
            if (label == "init")
                isInit = true;
            state.marking.add_element(plcInd->second, 1);
            iss >> label;
        }
        // Pack the RG state
        AppendState(rg, state);

        if (isInit) {
            InitialFrontierEntry ife(state.selfIndex, 1.0, EMPTY_VANISHING_PATH_SET);
            rg.initState = state;
            rg.initStates.push_back(ife);
            hasInit = true;
        }
    }
    if (!hasInit)
        throw program_exception(".lab file does not have an initial state.");

    // Read RG T->T edges
    RgEdge edge;
    for (size_t i = 0; i < numEdges; i++) {
        size_t src, dst;
        double rate;
        tra.read(src).read(dst).read(rate);
        src--;
        dst--;
        if (src >= numStates || dst >= numStates || rate <= 0.0)
            throw program_exception(tra.pos() + ": Invalid transition in the .tra file.");

        edge.dest.set(dst, TANGIBLE);
        edge.selfIndex = INVALID_RGEDGE_INDEX;
        edge.type      = TANGIBLE_TO_TANGIBLE;
        edge.firedTrn  = expTrn;
        edge.preempted = EMPTY_TRANSITION_SET;
        edge.nextEdge  = INVALID_RGEDGE_INDEX;
        edge.prob      = 1.0;
        edge.delay     = rate;
        edge.pathSet   = EMPTY_VANISHING_PATH_SET;

        PushBackEdge(rg, rgstate_t(src, TANGIBLE), edge);
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "STATES:          " << rg.NumStates() << endl;
        cout << "TRANSITIONS:     " << rg.NumEdges() << endl;
        cout << "STATE LABELS:    " << pn.plcs.size() << endl;
        cout << "INIT STATE IS:   " << rg.initState.selfIndex.index() << endl;
        PrintMemoryUsage(rg);
    }
}

//=============================================================================

int isnotquote(int ch) { return (ch != '\"'); }

void LoadPrismModel(ifstream &ifsTra, ifstream &ifsLab, ifstream &ifsSta,
                    PN &pn, RG2 &rg, VerboseLevel verboseLvl) {
    simple_tokenizer tra(&ifsTra);
    simple_tokenizer lab(&ifsLab);
    simple_tokenizer sta(&ifsSta);
    string label;
    size_t numStates, numEdges;
    tra.read(numStates).read(numEdges);

    // Load the MRMC definitions as places and transitions
    transition_t expTrn = LoadPrismModelAsPN(tra, lab, sta, pn, verboseLvl);

    // Find the initial state label in the lab file
    size_t initLabel = size_t(-1);
    string line;
    lab.getline(line);
    istringstream iss(line);
    simple_tokenizer stline(&iss);
    while (stline) {
        size_t index;
        stline.read(index);
        if (!stline)
            break;
        stline.readtok("=\"").gettok(label, isnotquote).readtok("\"");
        if (label == "init")
            initLabel = index;
    }
    if (initLabel == size_t(-1))
        throw program_exception(lab.pos() + ": Could not find the \"init\" label in the .label file.");
    size_t initStateInd = size_t(-1);

    // Find the initial state index
    lab.getline(line);
    while (lab && !line.empty() && initStateInd == size_t(-1)) {
        istringstream iss2(line);
        simple_tokenizer stline2(&iss2);
        size_t stInd, val;
        stline2.read(stInd).readtok(":");
        while (stline2) {
            stline2.read(val);
            if (val == initLabel) {
                initStateInd = stInd;
                break;
            }
        }
        lab.getline(line);
    }

    // Prepare the RG
    RgBuild build;
    rg.numPlaces      = pn.plcs.size();
    rg.isReduced      = true;
    rg.hasVanishPaths = false;
    rg.rgClass        = RGC_CTMC;

    rg.stateSets[TANGIBLE].packedMarkings.clear(rg.numPlaces);
    rg.stateSets[VANISHING].packedMarkings.clear(rg.numPlaces);

    // Load RG markings
    for (size_t i = 0; i < numStates; i++) {
        RgState state;
        state.selfIndex.set(i, TANGIBLE);
        state.marking.resize(pn.plcs.size());
        state.firstEdge = INVALID_RGEDGE_INDEX;
        // Read the state marking
        size_t pos, value;
        sta.read(pos).readtok(":").readtok("(");
        for (size_t p = 0; p < pn.plcs.size(); p++) {
            sta.read(value).readtok(p == pn.plcs.size() - 1 ? ")" : ",");
            if (value != 0)
                state.marking.add_element(p, value);
        }
        // Pack the RG state
        AppendState(rg, state);

        if (initStateInd == i) {
            InitialFrontierEntry ife(state.selfIndex, 1.0, EMPTY_VANISHING_PATH_SET);
            rg.initState = state;
            rg.initStates.push_back(ife);
        }
    }

    // Read RG T->T edges
    RgEdge edge;
    for (size_t i = 0; i < numEdges; i++) {
        size_t src, dst;
        double rate;
        tra.read(src).read(dst).read(rate);
        if (src >= numStates || dst >= numStates || rate <= 0.0)
            throw program_exception(tra.pos() + ": Invalid transition in the .tra file.");

        edge.dest.set(dst, TANGIBLE);
        edge.selfIndex = INVALID_RGEDGE_INDEX;
        edge.type      = TANGIBLE_TO_TANGIBLE;
        edge.firedTrn  = expTrn;
        edge.preempted = EMPTY_TRANSITION_SET;
        edge.nextEdge  = INVALID_RGEDGE_INDEX;
        edge.prob      = 1.0;
        edge.delay     = rate;
        edge.pathSet   = EMPTY_VANISHING_PATH_SET;

        PushBackEdge(rg, rgstate_t(src, TANGIBLE), edge);
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "STATES:          " << rg.NumStates() << endl;
        cout << "TRANSITIONS:     " << rg.NumEdges() << endl;
        cout << "STATE LABELS:    " << pn.plcs.size() << endl;
        cout << "INIT STATE IS:   " << rg.initState.selfIndex.index() << endl;
        PrintMemoryUsage(rg);
    }
}

//=============================================================================




































