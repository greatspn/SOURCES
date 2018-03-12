/*
 *  SolveMRP.h
 *
 *  Numerical solutions for MRPs.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __SOLVE_MRP_H__
#define __SOLVE_MRP_H__
//=============================================================================

//=============================================================================
// Markov Regenerative Processes Steady State Solution Helper structures:
//=============================================================================

// List of Exponential states
typedef vector<marking_t>  MarkingSubset_t;

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
//struct SubsetIndexStructTag;
//struct StateSubsetStructTag;
struct EmcStateStructTag;
struct SmcStateStructTag;
struct SccIndexStructTag;
struct TauBsccIndexStructTag;
struct RegenCompStructTag;
//typedef private_integer<int, SubsetIndexStructTag>  subsetindex_t;
//typedef private_integer<int, StateSubsetStructTag>  statesubset_t;
typedef private_integer<int, EmcStateStructTag>     emcstate_t;
typedef private_integer<int, SmcStateStructTag>     smcstate_t;
typedef private_integer<int, SccIndexStructTag>		sccindex_t;
typedef private_integer<int, TauBsccIndexStructTag> taubsccindex_t;
typedef private_integer<int, RegenCompStructTag>	regencomp_t;
#else
//typedef int  subsetindex_t;
//typedef int  statesubset_t;
typedef int  emcstate_t;
typedef int  smcstate_t;
typedef int  sccindex_t;
typedef int  taubsccindex_t;
typedef int  regencomp_t;
#endif

//const statesubset_t EXP_SUBSET = statesubset_t(-100);
//const statesubset_t IGNORED_SUBSET = statesubset_t(-150);



//-----------------------------------------------------------------------------
// Convert a statesubset_t to a zero-based subset index for BaseSubsetList
/*struct MRPSubsetIndexFunctor {
	inline numerical::subsetindex_t
	externalSubsetTo0BasedIndex(const statesubset_t subs) const {
		if (subs == EXP_SUBSET)
			return numerical::subsetindex_t(0);
		else if (subs == IGNORED_SUBSET)
			return numerical::subsetindex_t(1);
		else
			return numerical::subsetindex_t(size_t(subs) + 2);
	}
};
typedef BaseSubsetList<MRPSubsetIndexFunctor, statesubset_t, marking_t> MRPSubsetList;

//-----------------------------------------------------------------------------
// List of states where a deterministic transition is enabled with fixed delay
struct DetMarkingSubset {
	statesubset_t	  index;
	transition_t	  detTrnInd;
	double			  delay;	  // Transition delay (fixed in the entire subset)
	//MarkingSubset_t	  States;
};
typedef struct DetMarkingSubset  DetMarkingSubset;

//-----------------------------------------------------------------------------
// MRP representation
struct MRP {
	// (Defective) non-preemptive exponential transition rates matrix
	ublas::compressed_matrix<double> Q;
	// Preemptive exponential transition rates matrix
	ublas::compressed_matrix<double> Qbar;
	// Branching probabilities of deterministic transitions
	ublas::compressed_matrix<double> Delta;
	// Deterministic transitions (with fixed delay) set
	vector<DetMarkingSubset>		 Det;
	// State subset table
	MRPSubsetList					 Subsets;
	// Initial distribution vector
	ublas::vector<double>			 pi0;

	// Number of states in the MRP
	inline marking_t N() const { return Q.size1(); }
};
typedef struct MRP  MRP;*/

//-----------------------------------------------------------------------------
// RG State Classification
enum EmcStateKind {
    EMCSK_ABSORBING,
    EMCSK_NO_DET_ENABLED,
    EMCSK_ONLY_DET_ENABLED,
    EMCSK_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY,
    EMCSK_SMC
};
typedef enum EmcStateKind  EmcStateKind;
extern const char *EmcStateKind_names[];

//-----------------------------------------------------------------------------
// Embedded Markov Chain state descriptor
struct EMCState {
    emcstate_t		index;		// Index of the EMC
    marking_t		mrpIndex;	// Corresponding enter state of the MRP
    EmcStateKind	kind;		// Classification
    Signature128	isoClass;	// Isomorphism class (128-bit hash)
};
typedef struct EMCState  EMCState;

extern const emcstate_t  NOT_EMC_STATE;

//-----------------------------------------------------------------------------
// Embedded Markov Chain
struct EMC {
    vector<EMCState>    states;
    vector<emcstate_t>  mrpToEmcInds;  /// Maps MRP states to EMC state indexes
};
typedef EMC  EMC;

//-----------------------------------------------------------------------------
/// Subordinated Markov Chain
struct SMC {
    ublas::compressed_matrix<double>  S; /// The SMC matrix
    statesubset_t			subset;
    vector<smcstate_t>		mrpToInternalSmcInd;
    vector<smcstate_t>		mrpToPreemptSmcInd;
    vector<marking_t>		smcToMrpInds;
    smcstate_t				numInternalStates;
};
typedef struct SMC  SMC;

extern const smcstate_t NOT_SMC_STATE;

//-----------------------------------------------------------------------------
// Regenerative component (a component that starts in a set of regen. points)
struct RegenerativeComponent {
    regencomp_t	    index;	 // Index of this Regenerative Component
    statesubset_t	subset;	 // State partition (Exp, Gen[i]) it belongs
    taubsccindex_t	tauBscc; // Tau component class (BSCC Union {-1})
    MarkingSubset_t States;  // Initial SCC states pf this MP Component. Note
    // that the SMC of this may containt additional states
    bool          enqueued;	 // True if in the processing queue
    double        inputProb; // Total input probability before processing

    inline bool belongsToInitTransient() const
    {   return (tauBscc == taubsccindex_t(-1));   }
};
typedef struct RegenerativeComponent  RegenerativeComponent;




//-----------------------------------------------------------------------------
/*/ MRP marking classification
typedef unsigned char MRPStateKind;

const MRPStateKind MSK_REGENSTATE			= 0x01;
const MRPStateKind MSK_INITSTATE			= 0x02;
const MRPStateKind MSK_EMC_ABSORBING		= 0x04;
const MRPStateKind MSK_EMC_NO_DET_ENABLED	= 0x08;
const MRPStateKind MSK_EMC_ONLY_DET_ENABLED = 0x10;
const MRPStateKind MSK_EMC_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY = 0x20;
const MRPStateKind MSK_EMC_SMC				= 0x40;
const MRPStateKind MSK_EMC_NOT_EMBEDDED		= 0x80;

//TODO: commentare
struct MRPStateKinds {
	vector<MRPStateKind>	kinds;
	size_t					numRegenStates;
};
typedef struct MRPStateKinds  MRPStateKinds;
*/
/*enum ComponentProcessMethod {
	CPM_NO_COMPONENTS,  ///< Don't use the component process technique
	CPM_IMPLICIT,		///< Use the implicit non-ergodic MRP solution
	CPM_REPEAT_PRODUCTS	///< Do EMC products until the prob. mass is < epsilon
};
typedef enum ComponentProcessMethod  ComponentProcessMethod ;*/

//=============================================================================
//    Function Definitions:
//=============================================================================

//void ExtractMRP(const RG& rg, MRP& mrp, VerboseLevel verboseLvl);

//void PrintMRP(const MRP& mrp);

void ConstructSMC(const MRP &mrp, const MarkingSubset_t &initStates,
                  SMC &smc, VerboseLevel verboseLvl);

void SolveReducedMRP(const MRP &mrp, PetriNetSolution &sol, bool detectIsomorphism,
                     const SolverParams &spar, VerboseLevel verboseLvl);

void SolveNonErgodicMRP(const MRP &mrp, PetriNetSolution &sol, bool detectIsomorphism,
                        const SolverParams &spar, VerboseLevel verboseLvl);

void SolveSteadyStateMRP(RG &rg, PetriNetSolution &sol,
                         bool detectIsomorphism, bool reduceInitialTransient,
                         const SolverParams &spar, VerboseLevel verboseLvl);


/*bool SolveSteadyStateReducibleMRP(const MRP& mrp, const MRPStateKinds& stKinds,
								  PetriNetSolution& sol, const SolverParams& spar,
								  VerboseLevel verboseLvl);

void SolveSteadyStateMRPImplicit(RG& rg, PetriNetSolution& sol,
								 const SolverParams& spar,
								 VerboseLevel verboseLevel);*/

/*void ClassifyMRPStates(const MRP& mrp, MRPStateKinds& outSK,
					   VerboseLevel verboseLevel);*/

/*void SolveReducibleMrpBySccDecomp(RG& rg, PetriNetSolution& sol,
								  const SolverParams& spar,
								  ComponentProcessMethod cpm,
								  VerboseLevel verboseLvl);*/

//=============================================================================
#endif   // __SOLVE_MRP_H__
