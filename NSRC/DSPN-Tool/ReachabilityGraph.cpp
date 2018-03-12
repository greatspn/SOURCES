/*
 *  ReachabilityGraph.cpp
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
#include <functional>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "ReachabilityGraph.h"

//=============================================================================

const char *RGClass_Names[] = {
    "CONTINUOS TIME MARKOV CHAIN",
    "MARKOV REGENERATIVE PROCESS",
    "GENERALIZED SEMI-MARKOV PROCESS",
    "REACHABILITY GRAPH WITH TANGIBLE/VANISHING MARKINGS"
};
const char *RGClass_ShortNames[] = {
    "CTMC", "MRP", "GSMP", "UNCLASSIFIED"
};

struct VisitedMarking;

//=============================================================================
//    Internal Definitions needed by the RG building process only
//=============================================================================

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct MarkingHashStructTag;
typedef private_integer<size_t, MarkingHashStructTag>		markinghash_t;
#else
typedef size_t  markinghash_t;
#endif

//=============================================================================

// Construct an hash value from a sequence of added values (of size_t type)
struct hash_builder {
    inline hash_builder() { h = markinghash_t(0); }
    inline void add(size_t v);
    inline markinghash_t get() const { return h; }
    markinghash_t h;  			// The hash value being built
};

//-----------------------------------------------------------------------------
typedef std::unordered_multimap<markinghash_t, marking_t, std::hash<markinghash_t>,
        std::equal_to<markinghash_t>> MarkingHashTable_t;

// Helper structure used during the RG building process
struct RG_Build {
    ~RG_Build();
    queue<marking_t>    grayMarks;		 // Known marks not yet visited
    set<SparseMarking>  vanishingMarks;  // Avoids vanishing loops
    // Lookup for hash -> markings. Note that it is a multimap, since we are
    // storing only the hash, which is not guaranteed of being unique.
    MarkingHashTable_t  knownMarkHashes;
    bool			    includeVanishings; // Build a RG including vanishings
    bool			    saveVanishPaths; // Save the VanishingPath in the RGArcs
    RGBuildStat		   *pStat;			 // Statistics
    stack<const VisitedMarking *> vanishPathStack; // Used for buildings VPaths
    stack<VisitedMarking *> vmark_cache; // Cache buffer of unused VisitedMarking object
};
typedef struct RG_Build  RG_Build;

//-----------------------------------------------------------------------------
// Stack of currently visited vanishing markings (in the Depth-First visit)
struct VisitedMarking {
    const VisitedMarking *prev;
    size_t				  level;
    transition_t		  firedTrn;
    SparseMarking		  marking;
    set<transition_t>     preempted, preemptables;
    set<transition_t>     enabledDet, conEnabImmTrn;
    double				  prob;
    double				  trnProb;
    bool				  isVanishing;
};
typedef struct VisitedMarking  VisitedMarking;
const size_t MAX_VISIT_LEVELS = 10000;

// Special "Initial Vanishing Marking" ID
const marking_t INITIAL_VANISHING_MARKING_ID = marking_t(size_t(-5));

// Special "Initial Vanishing Marking" transition ID
const transition_t SOLVE_INITIAL_VANISHING_TRN = transition_t(size_t(-6));

// Special "Null" transition ID
const transition_t NULL_TRANSITION_ID = transition_t(size_t(-7));

//=============================================================================
//    Function implementations
//=============================================================================

// Internal function
// Print a ReachableMarking object, with its vanishing sub-paths
static void PrintReachableMarking(const PN &pn, const ReachableMarking &rm) {
    cout << (rm.isVanishing ? "VANISHING " : "TANGIBLE ");
    if (rm.index == INITIAL_VANISHING_MARKING_ID) // Initial vanishing
        cout << "INITIAL MARKING ";
    else
        cout << "MARKING M" << size_t(rm.index) + 1;
    cout << ": " << print_marking(pn, rm.marking);
    if (rm.isVanishing) {
        cout << ", preempted=";
        cout << print_indexed_names(rm.vanishPreempted, pn.trns);
        cout << ", preemptables=";
        cout << print_indexed_names(rm.vanishPreemptables, pn.trns);
    }
    assert(rm.vanishPreempted.size() == 0 || rm.isVanishing);
    cout << ", det=";
    cout << print_indexed_names(rm.enabledDetInds, pn.trns) << endl;

    vector<RGArc>::const_iterator rgArcIt = rm.timedPaths.begin();
    for (; rgArcIt != rm.timedPaths.end(); ++rgArcIt) {
        if (rgArcIt->timedTrnInd == SOLVE_INITIAL_VANISHING_TRN) {
            cout << "  -START_RG->";  // Special initial vanishing
        }
        else {
            const Transition &tt = pn.trns[rgArcIt->timedTrnInd];
            cout << "  -" << TrnDistrib_names[tt.distrib];
            cout << "(";
            if (tt.isGeneral())
                tt.delayFn->Print(cout, true);
            else
                cout << rgArcIt->delay;
            cout << ")-> " << tt.name << flush;
            assert(tt.delay(pn, rm.marking) == rgArcIt->delay);
            assert(rgArcIt->timedTrnInd == SOLVE_INITIAL_VANISHING_TRN ||
                   rgArcIt->distrib == pn.trns[rgArcIt->timedTrnInd].distrib);
        }
        cout << " to M" << size_t(rgArcIt->destMarking) + 1;
        cout << "  with prob=" << rgArcIt->prob << ", preempted=";
        cout << print_indexed_names(rgArcIt->preempted, pn.trns) << endl;

        list<VanishingPath>::const_iterator vpit = rgArcIt->vanishingPaths.begin();
        for (; vpit != rgArcIt->vanishingPaths.end(); ++vpit) {
            if (vpit->path.size() > 0) {
                cout << "      -VP-> ";
                list<VanishingMarking>::const_iterator vpelit = vpit->path.begin();
                size_t vpelCnt = 0;
                for (; vpelit != vpit->path.end(); ++vpelit) {
                    cout << (vpelCnt++ > 0 ? " + " : "");
                    cout << print_transition(pn, vpelit->immTrnInd);
                    if (vpelit->prob != 1.0)
                        cout << "(" << vpelit->prob << ")";
                }
                cout << "  with path prob=" << vpit->pathProb << endl;
            }
        }
    }
    cout << endl;
}

//=============================================================================

inline void hash_builder::add(size_t v) {
    // rotate h by ROL_N bits
    const size_t ROL_N = 7;
    h = (h << ROL_N) | (h >> (sizeof(markinghash_t) * CHAR_BIT - ROL_N));
    // xor with v
    h = h ^ v;
}

//=============================================================================

// Internal function
// Return an hash value for a given marking/preemption tuple, constructed from
// a source set of informations which are:
//    - a SparseMarking (place id and token counts)
// for tangible markings, or :
//    - a triplet of SparseMarking and the two deterministic preemption sets
// for vanishing markings.
inline markinghash_t
MakeMarkingHash(const SparseMarking &mark,
                const set<transition_t> &preempted,
                const set<transition_t> &preemptables) {
    const size_t MH_ENCODING_BASE = 1;
    const size_t MH_ENCODING_PREEMPT_LIST_SEP = 2;
    hash_builder hb;
    SparseMarking::const_iterator markIt;
    for (markIt = mark.begin(); markIt != mark.end(); ++markIt) {
        hb.add(MH_ENCODING_BASE + markIt->value);
        hb.add(MH_ENCODING_BASE + markIt->index);
    }
    if (preempted.size() > 0 || preemptables.size() > 0) {
        set<transition_t>::const_iterator tvIt;
        hb.add(MH_ENCODING_PREEMPT_LIST_SEP);
        for (tvIt = preempted.begin(); tvIt != preempted.end(); ++tvIt) {
            hb.add(MH_ENCODING_BASE + *tvIt);
        }
        hb.add(MH_ENCODING_PREEMPT_LIST_SEP);
        for (tvIt = preemptables.begin(); tvIt != preemptables.end(); ++tvIt) {
            hb.add(MH_ENCODING_BASE + *tvIt);
        }
    }
    return hb.get();
}

//=============================================================================

// Returns true if the specified marking is already stored inside the
// knownMarkHashes table. In that case, outIndexFound is set to the marking index
inline bool
IsMarkingKnown(const RG &rg, const RG_Build &build, const markinghash_t hash,
               const SparseMarking &mark,
               const set<transition_t> &preempted,
               const set<transition_t> &preemptables,
               marking_t &outIndexFound) {
    typedef MarkingHashTable_t::const_iterator const_iterator;
    std::pair<const_iterator, const_iterator> range;

    range = build.knownMarkHashes.equal_range(hash);
    while (range.first != range.second) {
        assert(hash == range.first->first);
        marking_t markIndex = range.first->second;
        const ReachableMarking &rm = rg.markSet[markIndex];
        // Check if the marking found has the same signature.
        if (rm.marking == mark &&
                rm.vanishPreempted == preempted &&
                rm.vanishPreemptables == preemptables) {
            outIndexFound = markIndex;
            return true;
        }
        ++range.first;
    }
    return false;
}

//=============================================================================

// Debug function used to test knownMarkHashes table correctness
inline void AssertMarkingIsNew(const RG &rg, const RG_Build &build,
                               const markinghash_t hash,
                               const SparseMarking &mark,
                               const set<transition_t> &preempted,
                               const set<transition_t> &preemptables) {
#ifndef NDEBUG
    marking_t index;
    if (IsMarkingKnown(rg, build, hash, mark, preempted, preemptables, index)) {
        throw program_exception("Wrong knownMarkHashes construction.");
    }
#endif
}

//=============================================================================

RG_Build::~RG_Build() {
    // We need to clear all the vmark_cache objects
    while (!vmark_cache.empty()) {
        delete vmark_cache.top();
        vmark_cache.pop();
    }
}

//=============================================================================

// Internal function
static void
FollowTransitionPath(const PN &pn, const PETLT &petlt, RG &rg,
                     RG_Build &build, const VisitedMarking *prevMark,
                     const transition_t firedTrn, double trnProb,
                     marking_t pathRootId, VerboseLevel verboseLvl) {
    if (prevMark->level > MAX_VISIT_LEVELS)
        throw program_exception("Vanishing paths are too long.");
    build.pStat->numPathsFollowed++;
    max_assign(build.pStat->maxVisitDepth, prevMark->level);

    // Allocate the VisitedMarking object using the vmark_cache cache
    VisitedMarking *pMarkObj;
    if (build.vmark_cache.empty()) {
        build.vmark_cache.push(new VisitedMarking());
    }
    pMarkObj = build.vmark_cache.top();
    build.vmark_cache.pop();
    assert(pMarkObj != nullptr);

    VisitedMarking &mark = *pMarkObj;
    mark.prev         = prevMark;
    mark.level        = prevMark->level + 1;
    mark.firedTrn     = firedTrn;
    mark.preempted    = prevMark->preempted;	// Inherit preempted set
    mark.preemptables = prevMark->preemptables; // Inherit preemptable set
    mark.prob         = prevMark->prob * trnProb;
    mark.trnProb      = trnProb;

    // Apply the transition firing to the marking
    FireTransition(pn, mark.firedTrn, prevMark->marking, mark.marking);


    // Test if some deterministic transitions have been preempted by the
    // firing of firedTrn, with regards to the enabled deterministic
    // transitions in the pathRootId marking.
    ListEnabledTransitionsInMarking(pn, petlt, mark.marking,
                                    DET, mark.enabledDet, nullptr);
    set<transition_t>::const_iterator trnIt;
    for (trnIt = mark.preemptables.begin(); trnIt != mark.preemptables.end(); ++trnIt) {
        transition_t detTrnInd = *trnIt;
        // The firing of a Deterministic transition is not a preemption
        if (detTrnInd == firedTrn)
            continue;
        // Check if the previously enabled detTrnInd has become disabled in mark
        if (mark.enabledDet.count(detTrnInd) > 0)
            continue;

        // detTrnInd is a deterministic transition newly disabled by firedTrn
        mark.preempted.insert(detTrnInd);
    }

    // Check if this marking is vanishing or tangible
    double totalConEnabImmWeight = 0.0;
    ListEnabledTransitionsInMarking(pn, petlt, mark.marking, IMM,
                                    mark.conEnabImmTrn, &totalConEnabImmWeight);
    mark.isVanishing = (mark.conEnabImmTrn.size() > 0);

    /*if (verboseLvl >= VL_VERBOSE) {
    	for (int l=mark.level; l>0; l--)
    		cout << "|  ";
    	cout << "#=="<<pn.trns[mark.firedTrn].name<<"==> ";
    	cout << (mark.isVanishing ? "VANISHING" : "TANGIBLE");
    	cout << " MARKING "<<print_marking(pn, mark.marking);
    	cout << " preempted={"; // TODO usare print_indexed_names
    	for (size_t i=0; i<mark.preempted.size(); i++)
    		cout << (i>0 ? "," : "") << pn.trns[mark.preempted[i]].name;
    	cout << "} preemptables={";
    	for (size_t i=0; i<mark.preemptables.size(); i++)
    		cout << (i>0 ? "," : "") << pn.trns[mark.preemptables[i]].name;
    	cout << "}";
    	cout << endl;

     }//*/

    if (mark.isVanishing && !build.includeVanishings) {
        //-------------------------------------------------------------
        // We are visiting a vanishing marking
        //-------------------------------------------------------------
        if (build.vanishingMarks.count(mark.marking) != 0)
            throw program_exception("Vanishing loop.");

        build.vanishingMarks.insert(mark.marking);

        // Visit recursively every connected marking
        trnIt = mark.conEnabImmTrn.begin();
        while (trnIt != mark.conEnabImmTrn.end()) {
            transition_t nextVisitedImmTrn = *trnIt++;
            // Follow recursively...
            FollowTransitionPath(pn, petlt, rg, build, &mark,
                                 nextVisitedImmTrn,
                                 pn.trns[nextVisitedImmTrn].weight(pn, mark.marking)
                                 / totalConEnabImmWeight,
                                 pathRootId, verboseLvl);
        }

        assert(build.vanishingMarks.count(mark.marking) == 1);
        build.vanishingMarks.erase(mark.marking);
        assert(build.vanishingMarks.count(mark.marking) == 0);
    }
    else {
        //-------------------------------------------------------------
        // We reached a tangible marking
        //-------------------------------------------------------------
        assert(build.includeVanishings || !mark.isVanishing);
        marking_t reachedRM;

        // Build up the markingHash value
        markinghash_t markHash;
        set<transition_t> emptySet;
        bool isMarkingKnown;
        marking_t foundMarkIndex(size_t(-1));
        if (mark.isVanishing) {
            markHash = MakeMarkingHash(mark.marking, mark.preempted,
                                       mark.preemptables);
            isMarkingKnown = IsMarkingKnown(rg, build, markHash, mark.marking,
                                            mark.preempted, mark.preemptables,
                                            foundMarkIndex);
        }
        else {
            markHash = MakeMarkingHash(mark.marking, emptySet, emptySet);
            isMarkingKnown = IsMarkingKnown(rg, build, markHash, mark.marking,
                                            emptySet, emptySet, foundMarkIndex);
        }

        if (isMarkingKnown) {
            // We already know this RG marking
            reachedRM = foundMarkIndex;
        }
        else { // Reached a new unexplored tangible marking
            if (rg.markSet.capacity() == rg.markSet.size())
                rg.markSet.reserve(rg.markSet.size() * 8);
            reachedRM = marking_t(rg.markSet.size());
            rg.markSet.resize(size_t(reachedRM) + 1);

            ReachableMarking &newRM = rg.markSet[reachedRM];
            newRM.index       = reachedRM;
            newRM.marking     = mark.marking;
            newRM.isVanishing = mark.isVanishing;
            if (newRM.isVanishing) {
                newRM.vanishPreempted    = mark.preempted;
                newRM.vanishPreemptables = mark.preemptables;
            }

            build.grayMarks.push(newRM.index);
            AssertMarkingIsNew(rg, build, markHash, newRM.marking,
                               newRM.vanishPreempted, newRM.vanishPreemptables);
            build.knownMarkHashes.insert(make_pair(markHash, newRM.index));
        }

        // Follow the vanishing path backward until the root tangible marking
        // is reached and is put at the stack top.
        stack<const VisitedMarking *> &vanishPath = build.vanishPathStack;
        const VisitedMarking *pRootMark = &mark;
        while (true) {
            if (pRootMark->level == 1)
                break;
            else {
                if (build.saveVanishPaths)
                    vanishPath.push(pRootMark);
                pRootMark = pRootMark->prev;
            }
        }
        assert(pRootMark != nullptr);
        assert(build.includeVanishings ||
               (pathRootId == INITIAL_VANISHING_MARKING_ID &&
                pRootMark->firedTrn == SOLVE_INITIAL_VANISHING_TRN) ||
               pn.trns[pRootMark->firedTrn].isTimed());

        // If this path follows a deterministic transition D, D shouldn't be in
        // the preemption set (Note that if isInitialVanishingMarking is true,
        // than pRootMark->firedTrn is invalid).
        assert(pRootMark->firedTrn == SOLVE_INITIAL_VANISHING_TRN ||
               !pn.trns[pRootMark->firedTrn].isGeneral() ||
               mark.preempted.count(pRootMark->firedTrn) == 0);

        // Add a new vanishing path between the starting and the reached TMs
        ReachableMarking &pathRoot = (pathRootId == INITIAL_VANISHING_MARKING_ID ?
                                      rg.initMark : rg.markSet[pathRootId]);
        vector<RGArc>::iterator rgArcIt = pathRoot.timedPaths.begin();
        for (; rgArcIt != pathRoot.timedPaths.end(); ++rgArcIt) {
            if (rgArcIt->timedTrnInd == pRootMark->firedTrn &&
                    rgArcIt->destMarking == reachedRM &&
                    rgArcIt->preempted == mark.preempted) {
                // Add this new vanishing path to an existing RG Arc
                break;
            }
        }
        if (rgArcIt == pathRoot.timedPaths.end()) {
            // Create a new RG arc
            pathRoot.timedPaths.resize(pathRoot.timedPaths.size() + 1);
            rgArcIt = --pathRoot.timedPaths.end();
            rgArcIt->timedTrnInd = pRootMark->firedTrn;
            rgArcIt->destMarking = reachedRM;
            rgArcIt->preempted   = mark.preempted;
            rgArcIt->prob        = 0.0;
            if (pRootMark->firedTrn == SOLVE_INITIAL_VANISHING_TRN)
                rgArcIt->distrib = IMM;
            else {
                rgArcIt->delay   = (pn.trns[rgArcIt->timedTrnInd]
                                    .delay(pn, pRootMark->prev->marking));
                rgArcIt->distrib = pn.trns[rgArcIt->timedTrnInd].distrib;
            }

            max_assign(build.pStat->maxExitArcs, pathRoot.timedPaths.size());
        }

        // Add the vanishing path (if required)
        if (build.saveVanishPaths) {
            VanishingPath vpath;
            while (!vanishPath.empty()) {
                const VisitedMarking *pPathElem = vanishPath.top();
                vanishPath.pop();

                VanishingMarking pathElem;
                pathElem.immTrnInd     = pPathElem->firedTrn;
                pathElem.conEnabImmTrn = pPathElem->conEnabImmTrn;
                pathElem.prob          = pPathElem->trnProb;
                assert(pn.trns[pathElem.immTrnInd].isImmediate());

                vpath.path.push_back(pathElem);
            }
            vpath.pathProb = mark.prob;
            rgArcIt->vanishingPaths.push_back(vpath);
            max_assign(build.pStat->maxArcVanishPaths, vpath.path.size());
        }
        assert(vanishPath.empty());

        // Add up the path probability to the RG arc
        rgArcIt->prob += mark.prob;
    }

    // Push back in the cache the VisitedMarking object
    build.vmark_cache.push(pMarkObj);
}

//=============================================================================

void BuildRG(const PN &pn, const PETLT &petlt, RG &rg, bool includeVanishings,
             bool saveVanishPaths, RGBuildStat &stat, VerboseLevel verboseLvl) {
    rg.rgClass = RGC_CTMC;

    // General events table
    for (const Transition& trn : pn.trns)
        if (trn.isGeneral())
            rg.generalFn.insert(make_pair(trn.index, trn.generalPdf()));

    // Initial marking setup
    vector<Place>::const_iterator plcit;
    rg.initMark.marking.resize(pn.plcs.size());
    for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit) {
        tokencount_t tc = plcit->getInitTokenCount();
        if (tc != tokencount_t(0))
            rg.initMark.marking.insert_element(plcit->index, tc);
    }
    if (verboseLvl >= VL_VERBOSE)
        cout << "INITIAL MARKING: " << print_marking(pn, rg.initMark.marking) << "\n\n";

    // Verify that the initial marking is tangible (when building a TRG)
    set<transition_t> enabledImm, enabledTimed;
    set<transition_t>::const_iterator trnIt;
    double totalConEnabImmWeight = 0.0;
    ListEnabledTransitionsInMarking(pn, petlt, rg.initMark.marking, IMM,
                                    enabledImm, &totalConEnabImmWeight);

    rg.initMark.index = marking_t(0);
    rg.initMark.isVanishing = enabledImm.size() > 0;

    RG_Build build;
    build.pStat	             = &stat;
    build.includeVanishings  = includeVanishings;
    build.saveVanishPaths    = saveVanishPaths;
    memset(build.pStat, 0, sizeof(RGBuildStat));

    if (!includeVanishings && enabledImm.size() > 0) {
        // Solve the initial vanishing marking for a Tangible RG
        rg.initMark.index = INITIAL_VANISHING_MARKING_ID;
        for (trnIt = enabledImm.begin(); trnIt != enabledImm.end(); ++trnIt) {
            VisitedMarking visit;
            visit.prev        = nullptr;
            visit.level       = 1; // It's like an "external" timed transition firing
            visit.firedTrn    = SOLVE_INITIAL_VANISHING_TRN;
            visit.marking     = rg.initMark.marking;
            visit.prob        = 1.0;
            visit.trnProb     = visit.prob;
            visit.isVanishing = true;
            visit.preemptables.clear();
            visit.enabledDet.clear();

            double trnProb = (pn.trns[*trnIt].weight(pn, visit.marking)
                              / totalConEnabImmWeight);
            FollowTransitionPath(pn, petlt, rg, build, &visit, *trnIt,
                                 trnProb, INITIAL_VANISHING_MARKING_ID, verboseLvl);
            assert(build.vanishingMarks.empty());
        }
        enabledImm.clear();
        assert(rg.markSet.size() > 0);
        assert(rg.initMark.timedPaths.size() > 0);
        if (verboseLvl >= VL_VERBOSE)
            PrintReachableMarking(pn, rg.initMark);
    }
    else {
        // Add the initial tangible marking in the RG
        rg.markSet.push_back(rg.initMark);
        build.grayMarks.push(rg.initMark.index);
        RGArc initArc;
        initArc.timedTrnInd = SOLVE_INITIAL_VANISHING_TRN;
        initArc.destMarking = rg.initMark.index;
        initArc.prob        = 1.0;
        initArc.delay       = 0.0;
        initArc.distrib     = IMM;
        rg.initMark.timedPaths.push_back(initArc);

        markinghash_t markHash = MakeMarkingHash(rg.initMark.marking,
                                 rg.initMark.vanishPreempted,
                                 rg.initMark.vanishPreemptables);
        AssertMarkingIsNew(rg, build, markHash, rg.initMark.marking,
                           rg.initMark.vanishPreempted, rg.initMark.vanishPreemptables);
        build.knownMarkHashes.insert(make_pair(markHash, rg.initMark.index));
    }

    // Visit every reachable marking, effectively building the RG
    long_interval_timer visitTimer;
    while (!build.grayMarks.empty()) {
        marking_t markId = build.grayMarks.front();
        build.grayMarks.pop();
        ReachableMarking *pRM = &rg.markSet[markId];

        /*if (verboseLvl >= VL_VERBOSE)
        	cout << "|\nVISITING: "<<print_marking(pn, pRM->marking)<<endl;//*/

        size_t numEnabled = 0;
        totalConEnabImmWeight = 0.0;
        ListEnabledTransitionsInMarking(pn, petlt, pRM->marking, DET,
                                        pRM->enabledDetInds, nullptr);
        max_assign(build.pStat->maxDetTrnsConEnabl, pRM->enabledDetInds.size());
        if (includeVanishings) {
            numEnabled = ListEnabledTransitionsInMarking(pn, petlt, pRM->marking,
                         IMM, enabledImm,
                         &totalConEnabImmWeight);
            trnIt = enabledImm.begin();
        }
        if (numEnabled == 0) {
            numEnabled = pRM->enabledDetInds.size();
            numEnabled += ListEnabledTransitionsInMarking(pn, petlt, pRM->marking,
                          EXP, enabledTimed, nullptr);
            enabledTimed.insert(pRM->enabledDetInds.begin(),
                                pRM->enabledDetInds.end());
            trnIt = enabledTimed.begin();
        }

        // Reclassify the RG if necessary
        if (rg.rgClass == RGC_CTMC && pRM->enabledDetInds.size() == 1)
            rg.rgClass = RGC_MarkovRenewalProcess;
        else if (!includeVanishings && pRM->enabledDetInds.size() > 1)
            rg.rgClass = RGC_GeneralizedSemiMarkov;
        else if (includeVanishings && enabledImm.size() > 0)
            rg.rgClass = RGC_WithVanishingMarkings;

        // Extend the RG following (timed) transition paths enabled in *pRM
        for (size_t e = 0; e < numEnabled; e++) {
            pRM = &rg.markSet[markId]; // rg.markSet may have been relocated

            VisitedMarking visit;
            visit.prev       = nullptr;
            visit.level      = 0;
            visit.firedTrn   = NULL_TRANSITION_ID;
            visit.marking    = pRM->marking;
            visit.enabledDet = pRM->enabledDetInds;

            transition_t firedTrn = *trnIt++;
            if (enabledImm.size() > 0) {
                visit.prob         = (pn.trns[firedTrn].weight(pn, pRM->marking)
                                      / totalConEnabImmWeight);
                visit.trnProb      = visit.prob;
                visit.isVanishing  = true;
                visit.preempted    = pRM->vanishPreempted;
                visit.preemptables = pRM->vanishPreemptables;
            }
            else {
                visit.prob         = 1.0;
                visit.trnProb      = 1.0;
                visit.isVanishing  = false;
                visit.preemptables = pRM->enabledDetInds;
            }

            // Deterministic transition firings doesn't count as "preemptions"
            if (pn.trns[firedTrn].isGeneral())
                visit.preemptables.erase(firedTrn);

            FollowTransitionPath(pn, petlt, rg, build, &visit, firedTrn,
                                 1.0, markId, verboseLvl);
            assert(build.vanishingMarks.empty());
        }
        assert(rg.markSet[markId].isVanishing ||
               (rg.markSet[markId].vanishPreempted.empty() &&
                rg.markSet[markId].vanishPreemptables.empty()));

        if (verboseLvl >= VL_VERBOSE)
            PrintReachableMarking(pn, rg.markSet[markId]);

        if (verboseLvl == VL_BASIC && visitTimer) {
            visitTimer.prepare_print();
            cout << "MARKING COUNT: " << rg.markSet.size();
            cout << "           " << endl;
        }
    }

    if (verboseLvl == VL_BASIC) {
        visitTimer.clear_any();
    }

    // Finally, preprocess PN measures vector
    PreprocessMeasures(pn, rg, verboseLvl);
}

//=============================================================================










