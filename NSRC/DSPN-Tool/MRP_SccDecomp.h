/*
 *  MRP_SccDecomp.h
 *
 *  Steady state solution of MRP with SCC decomposition.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __MRP_SCC_DECOMPOSITION_H__
#define __MRP_SCC_DECOMPOSITION_H__
//=============================================================================

// enum ComponentProcessMethod {
// 	CPM_NO_COMPONENTS,  ///< Don't use the component process technique
// 	CPM_IMPLICIT,		///< Use the implicit non-ergodic MRP solution
// 	CPM_REPEAT_PRODUCTS	///< Do EMC products until the prob. mass is < epsilon
// };

 #if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct ComponentStructTag;
typedef private_integer<int, ComponentStructTag>  compId_t;
#else
typedef int  compId_t; // must be signed
#endif

//=============================================================================

// This class builds up the BaseSubsetList of an SCC decomposition
struct SccIndexTo0BasedIndex : boost::noncopyable {
    inline numerical::subsetindex_t
    externalSubsetTo0BasedIndex(const sccindex_t sccIndex) const {
        return numerical::subsetindex_t(int(sccIndex));
    }
};
typedef BaseSubsetList<SccIndexTo0BasedIndex, sccindex_t, marking_t> SCCSubsetList;

//-----------------------------------------------------------------------------

enum TQEntryElement {
    TQE_STATE_SUBSET = 0,
    TQ_HAS_PREV_COMPLEX_EDGE = 1,
    TQE_SCC_INDEX = 2
};
// tuple <StateSubset, hasEnteringComplexEdges, SccIndex>
typedef tuple<statesubset_t, bool, sccindex_t> TQEntry;
// An edge between two SCCs: pair<scc_from, scc_to>
typedef pair<int, int> EdgeType_t;
typedef set<EdgeType_t> SCCEdgeSet_t;


struct SCCInfo {
    bool	isGhost;
    bool	isBottomSCC;
    bool	isSimple;
    int		numPrevSCCs;
    statesubset_t subset; // IGNORED if the scc crosses multiple states subsets
};

struct TopologicalOrderQueue : boost::noncopyable {
    // Number of unique SCC indexes
    sccindex_t numSCC;
    // Information for each SCC
    vector<SCCInfo> sccInfo;
    // State partition of the MRP w.r.t the SCCs
    SCCSubsetList SCCStateSubsets;

    // Edges between SCCs. Note that Intersect[SimpleEdges,ComplexEdges] = 0
    SCCEdgeSet_t InSimpleEdges, InComplexEdges;
    SCCEdgeSet_t OutSimpleEdges, OutComplexEdges;

    // Priority queue of SCC s.t. all their predecessors have already been extracted
    set<TQEntry> FreeQueue;
};

const statesubset_t COMPLEX_COMPONENT_SUBSET(-500);

//=============================================================================

struct ComponentProcessor {
    // Called to process the next component in the list.
    virtual bool ProcessNextComponent(const TopologicalOrderQueue &tqo,
                                      const sccindex_t topScc,
                                      const statesubset_t topSubset,
                                      const vector<bool> &intCompStates,
                                      const vector<bool> &extCompStates,
                                      const size_t numAugmentedStates,
                                      VerboseLevel verboseLvl) = 0;

    // Finished processing components.
    virtual void EndProcessing() = 0;
};

//=============================================================================

// Descriptor of a component obtained from the component method
struct ComponentDesc {
    size_t       index;	        // 0-based component number
    vector<bool> intCompStates; // Internal tangible states
    vector<bool> extCompStates; // External(frontier) tangible states
    statesubset_t topSubset;    // Kind of component (simple/complex)

    inline bool isComplex() const { return topSubset == COMPLEX_COMPONENT_SUBSET; }
};

std::function<ostream& (ostream &)>
print_compdesc(const ComponentDesc &cd, const string &stateLabel, VerboseLevel verboseLvl);

//=============================================================================

// Simple component processor that just stores the list of processed components
class SimpleComponentProcessor : public ComponentProcessor {
public:
    SimpleComponentProcessor() {}

    std::vector<ComponentDesc>  comps;

    // Called to process the next component in the list.
    virtual bool ProcessNextComponent(const TopologicalOrderQueue &tqo,
                                      const sccindex_t topScc,
                                      const statesubset_t topSubset,
                                      const vector<bool> &intCompStates,
                                      const vector<bool> &extCompStates,
                                      const size_t numAugmentedStates,
                                      VerboseLevel verboseLvl) override;

    // Finished processing components.
    virtual void EndProcessing() override
    { }

private:
};

//=============================================================================

void ProcessComponentList(const MRP &mrp, const MRPStateKinds &stKinds,
                          ComponentProcessor &compProc, bool aggregateComplexComp,
                          ComponentMethod compMethod, bool randomizeSccSelection, 
                          size_t randomSeed, VerboseLevel verboseLvl);

void SteadyStateMrpBySccDecomp(const MRP &mrp, const MRPStateKinds &stKinds,
                               PetriNetSolution &sol,
                               const SolverParams &spar,
                               VerboseLevel verboseLvl);

void SteadyStateMrpBySccDecompBackward(const MRP &mrp, const MRPStateKinds &stKinds,
                                       const ublas::vector<double> &rho,
                                       ublas::vector<double> &xi,
                                       const SolverParams &spar,
                                       VerboseLevel verboseLvl);

bool SolveComponent_Forward(const MRP &compMrp,
                            const statesubset_t topSubset,
                            PetriNetSolution &outSol,
                            const SolverParams &spar,
                            VerboseLevel verboseLvl);

void SolveComponent_Backward(const MRP &compMrp,
                             const statesubset_t topSubset,
                             const ublas::vector<double> &rho,
                             PetriNetSolution &outSol,
                             const SolverParams &spar,
                             VerboseLevel verboseLvl);

//=============================================================================
#endif   // __MRP_SCC_DECOMPOSITION_H__
