/*
 *  MRP.h
 *
 *  Definitions of the Markov Regenerative Process (MRP) data structures.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __MARKOV_REGENERATIVE_PROCESS_H__
#define __MARKOV_REGENERATIVE_PROCESS_H__
//=============================================================================

//=============================================================================
// Markov Regenerative Processes Steady State Solution Helper structures:
//=============================================================================

// List of Exponential states
typedef vector<marking_t>  MarkingSubset_t;

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
//struct SubsetIndexStructTag;
struct StateSubsetStructTag;
//struct EmcStateStructTag;
//struct SmcStateStructTag;
//struct SccIndexStructTag;
//struct TauBsccIndexStructTag;
//struct RegenCompStructTag;
//typedef private_integer<int, SubsetIndexStructTag>  subsetindex_t;
typedef private_integer<int, StateSubsetStructTag>  statesubset_t;
//typedef private_integer<int, EmcStateStructTag>     emcstate_t;
//typedef private_integer<int, SmcStateStructTag>     smcstate_t;
//typedef private_integer<int, SccIndexStructTag>		sccindex_t;
//typedef private_integer<int, TauBsccIndexStructTag> taubsccindex_t;
//typedef private_integer<int, RegenCompStructTag>	regencomp_t;
#else
//typedef int  subsetindex_t;
typedef int  statesubset_t;
//typedef int  emcstate_t;
//typedef int  smcstate_t;
//typedef int  sccindex_t;
//typedef int  taubsccindex_t;
//typedef int  regencomp_t;
#endif

const statesubset_t EXP_SUBSET = statesubset_t(-100);
const statesubset_t IGNORED_SUBSET = statesubset_t(-150);
// Used for SCC classification in MRP_SccDecomp
const statesubset_t MULTIPLE_SUBSETS = statesubset_t(-180);



//-----------------------------------------------------------------------------
// Convert a statesubset_t to a zero-based subset index for BaseSubsetList
struct MRPSubsetIndexFunctor {
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
struct GenMarkingSubset {
    statesubset_t	  index;
    transition_t	  genTrnInd;
    // The distribution function: it is written as an expression, as accepted 
    // by the alpha Factor engine. In the case of a deterministic duration,
    // also write the delay to simplify the job of the solvers.
    string            fg;
    double            detDelay;
    // double			  delay;	  // Transition delay (fixed in the entire subset)

    
    //MarkingSubset_t	  States;
};
typedef struct GenMarkingSubset  GenMarkingSubset;

//-----------------------------------------------------------------------------

// Returns the f_g(x) function of deterministic duration t, which is: "I[t]"
string DeterministicDurationFg(double time);

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
    vector<GenMarkingSubset>		 Gen;
    // State subset table
    MRPSubsetList					 Subsets;
    // Initial distribution vector
    ublas::vector<double>			 pi0;

    // Number of states in the MRP
    inline marking_t N() const { return Q.size1(); }
};
typedef struct MRP  MRP;

//-----------------------------------------------------------------------------
// MRP marking classification
typedef unsigned char MRPStateKind;

const MRPStateKind MSK_REGENSTATE			= 0x01;
const MRPStateKind MSK_INITSTATE			= 0x02;
const MRPStateKind MSK_EMC_ABSORBING		= 0x04;
const MRPStateKind MSK_EMC_NO_DET_ENABLED	= 0x08;
const MRPStateKind MSK_EMC_ONLY_DET_ENABLED = 0x10;
const MRPStateKind MSK_EMC_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY = 0x20;
const MRPStateKind MSK_EMC_SMC				= 0x40;
const MRPStateKind MSK_EMC_NOT_EMBEDDED		= 0x80;

/// Holder of the MRP state classification
struct MRPStateKinds {
    vector<MRPStateKind>	kinds;			///< State kinds
    size_t					numRegenStates; ///< Number of regenerative states
};
typedef struct MRPStateKinds  MRPStateKinds;

//=============================================================================
//    Function Definitions:
//=============================================================================

void ExtractMRP(const RG &rg, MRP &mrp, VerboseLevel verboseLvl);
void ExtractMRP(const RG2 &rg, MRP &mrp, VerboseLevel verboseLvl);

void PrintMRP(const MRP &mrp);

void ClassifyMRPStates(const MRP &mrp, MRPStateKinds &outSK,
                       VerboseLevel verboseLevel);

void ReorderMRP(const MRP& mrpIn, MRP& mrpOut, 
                const vector<marking_t>& order, VerboseLevel verboseLevel);

//=============================================================================
// Helper functions
//=============================================================================

typedef ublas::matrix_row< const ublas::compressed_matrix<double> > matRow_t;
typedef matRow_t::const_iterator rowIter_t;

static inline
bool matrix_row_has_nonzeroes(const ublas::compressed_matrix<double> &M,
                              const size_t row, bool &isAbsorbing) {
    matRow_t Mrow(M, row);
    rowIter_t M_ij(Mrow.begin()), M_rowEnd(Mrow.end());
    bool hasNonZeroDiag = false;
    while (M_ij != M_rowEnd) {
        const size_t col = M_ij.index();
        if (*M_ij != 0.0) {
            if (col != row) {
                isAbsorbing = false;
                return true;
            }
            hasNonZeroDiag = true;
        }
        ++M_ij;
    }
    return hasNonZeroDiag;
}

static inline
size_t count_matrix_row_nonzero_values(const ublas::compressed_matrix<double> &M,
                                       size_t row) {
    matRow_t Mrow(M, row);
    rowIter_t M_ij(Mrow.begin()), M_rowEnd(Mrow.end());
    size_t count = 0;
    for (; M_ij != M_rowEnd; ++M_ij)
        count++;
    return count;
}

//=============================================================================
#endif   // __MARKOV_REGENERATIVE_PROCESS_H__
