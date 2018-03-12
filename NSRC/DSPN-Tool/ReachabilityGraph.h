/*
 *  ReachabilityGraph.h
 *
 *  Reachability Graph Structure Definitions and Functions.
 *
 *  Created by Elvio Amparore
 *
 */

//=============================================================================
#ifndef __REACHABILITY_GRAPH_NET_H__
#define __REACHABILITY_GRAPH_NET_H__
//=============================================================================

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct ReachMarkStructTag;
typedef private_integer<size_t, ReachMarkStructTag>		marking_t;
#else
typedef size_t  marking_t;
#endif

//=============================================================================
//   Forward Declarations
//=============================================================================
struct VanishingMarking;
struct VanishingPath;
struct RGArc;
struct ReachableMarking;
struct RG;
struct RGBuildStat;

typedef struct VanishingMarking		VanishingMarking;
typedef struct VanishingPath		VanishingPath;
typedef struct RGArc				RGArc;
typedef struct ReachableMarking		ReachableMarking;
typedef struct RG					RG;
typedef struct RGBuildStat			RGBuildStat;

//=============================================================================
//     Reachability Graph Definitions
//=============================================================================

// Element in a vanishing path between two tangible markings
struct VanishingMarking {
    transition_t			immTrnInd;		// Immediate transition
    set<transition_t>		conEnabImmTrn;	// Other concurrently enabled immed.
    double					prob;			// Probability of this firing
};

//-----------------------------------------------------------------------------
// Vanishing path
struct VanishingPath {
    double					pathProb; // Total probability of this vanishing path
    list<VanishingMarking>  path;	  // Sequence of immediate transitions
};

//-----------------------------------------------------------------------------
// Reachability Graph Arc
struct RGArc {
    transition_t		  timedTrnInd;	  // Timed transition that starts this arc
    marking_t			  destMarking;	  // Destination marking reached
    set<transition_t>  	  preempted;	  // Det. trns. preempted along this arc
    double				  prob;			  // Total arc probability
    double				  delay;		  // Transition delay in this marking
    TrnDistrib			  distrib;		  // Distribution function
    list<VanishingPath>   vanishingPaths; // Vanishing subpaths
};

//-----------------------------------------------------------------------------
// Reachability Graph Marking
struct ReachableMarking {
    marking_t			  index;			// Index of this marking
    SparseMarking		  marking;		    // Token assignment
    set<transition_t>     enabledDetInds;   // Det. trn. enabled in this marking
    vector<RGArc>		  timedPaths;	    // Arcs that fires from this marking
    bool				  isVanishing;	    // Type of RG marking
    set<transition_t>     vanishPreempted;  // Only in vanishing markings
    set<transition_t>     vanishPreemptables; // Only in vanishing markings
};

//-----------------------------------------------------------------------------
// RG Classes
enum RGClass {
    RGC_CTMC,
    RGC_MarkovRenewalProcess,
    RGC_GeneralizedSemiMarkov,
    RGC_WithVanishingMarkings
};
typedef enum RGClass  RGClass;

// RG Classes long names
extern const char *RGClass_Names[];

// RG Classes short names
extern const char *RGClass_ShortNames[];

//-----------------------------------------------------------------------------
// (Tangible) Reachability Graph
struct RG {
    vector<ReachableMarking> markSet;	 // Marking Set
    RGClass					 rgClass;	 // RG Class
    ReachableMarking		 initMark;	 // Initial Vanishing marking (if any)
    vector<PpMeasure>		 ppMeasures; // Preprocessed measures
    map<transition_t, string> generalFn; // f_g(x) functions of general transitions

    inline bool hasMultipleInitMarks() const;
    inline size_t countNumOfTimedPaths(bool expNoPreem, bool expPreem,
                                       bool det) const;
    inline size_t NumStates() const;
};

//-----------------------------------------------------------------------------
// Statistics of the RG building process
struct RGBuildStat {
    size_t	numPathsFollowed;	// Number of transition paths followed
    size_t	maxDetTrnsConEnabl; // Max. Determ. Transitions Concurrently enabled
    size_t  maxExitArcs;		// Max. number of arcs in a RG marking

    // Statistics only available for Tangible RG builds
    size_t	maxVisitDepth;		// Max. Depth of the DFS visit procedure
    size_t  maxArcVanishPaths;	// Max. number of vanishing path in an RG arcs
};

//=============================================================================
//     Function Prototypes
//=============================================================================

void BuildRG(const PN &pn, const PETLT &petlt, RG &rg, bool includeVanishings,
             bool saveVanishPaths, RGBuildStat &stat, VerboseLevel verboseLvl);

//=============================================================================
//     Inline Implementations
//=============================================================================

inline bool RG::hasMultipleInitMarks() const {
    return initMark.timedPaths.size() > 1;
}

inline size_t RG::countNumOfTimedPaths(bool expNoPreem, bool expPreem,
                                       bool det) const {
    vector<ReachableMarking>::const_iterator rmIt;
    size_t cnt = 0;
    for (rmIt = markSet.begin(); rmIt != markSet.end(); ++rmIt) {
        bool hasDetEnab = (rmIt->enabledDetInds.size() > 0);
        if (!hasDetEnab) {
            // Only exponential arcs, no deterministic transitions
            if (expNoPreem)
                cnt += rmIt->timedPaths.size();
            continue;
        }
        vector<RGArc>::const_iterator arcIt = rmIt->timedPaths.begin();
        for (; arcIt != rmIt->timedPaths.end(); ++arcIt) {
            if (arcIt->distrib == DET) {
                if (det)
                    cnt++;
            }
            else if (arcIt->distrib == EXP) {
                if (arcIt->preempted.size() == 0 && expNoPreem)
                    cnt++;
                else if (arcIt->preempted.size() > 0 && expPreem)
                    cnt++;
            }
        }
    }
    return cnt;
}

inline size_t RG::NumStates() const {
    return markSet.size();
}

//=============================================================================
#endif   // __REACHABILITY_GRAPH_NET_H__
