/*
 *  MRP_SccDecomp.cpp
 *
 *  Steady state MRP solution with SCC decomposition.
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
#include <cstdio>
#include <cassert>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <memory>
#include <tuple>
#include <random>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "ReachabilityGraph.h"
#include "MRP.h"
#include "MRP_Implicit.h"
#include "MRP_SccDecomp.h"


//=============================================================================

std::function<ostream& (ostream &)>
print_compdesc(const ComponentDesc &cd, const string &stateLabel, VerboseLevel verboseLvl) {
    return [ =, &cd](ostream & os) -> ostream& {
        if (verboseLvl >= VL_BASIC) {
            size_t numIntLocs = std::count(cd.intCompStates.begin(), cd.intCompStates.end(), true);
            size_t numExtLocs = std::count(cd.extCompStates.begin(), cd.extCompStates.end(), true);
            cout << "\n\nPROCESSING " << console::cyan_fgnd() ;
            if (cd.topSubset == EXP_SUBSET)
                cout << "(EXP)";
            else if (cd.topSubset == COMPLEX_COMPONENT_SUBSET)
                cout << "(MIXED)";
            else
                cout << "(GEN: " << cd.topSubset << ")";
            // cout << (cd.topSubset == COMPLEX_COMPONENT_SUBSET ? "COMPLEX" : "SIMPLE");
            cout << console::default_disp() << " COMPONENT NUM=" << cd.index + 1;
            cout << " WITH " << numIntLocs << " INTERNAL " << stateLabel << (numIntLocs == 1 ? "" : "s");
            cout << " AND " << numExtLocs << " FRONTIER " << stateLabel << (numExtLocs == 1 ? "" : "s");
            cout << "." << endl;
        }
        if (verboseLvl >= VL_VERBOSE) {
            cout << "   INT = " << print_vec(cd.intCompStates) << endl;
            cout << "   EXT = " << print_vec(cd.extCompStates) << endl;
        }
        return os;
    };
}

//=============================================================================

bool SimpleComponentProcessor::ProcessNextComponent(const TopologicalOrderQueue &tqo,
        const sccindex_t topScc,
        const statesubset_t topSubset,
        const vector<bool> &intCompStates,
        const vector<bool> &extCompStates,
        const size_t numAugmentedStates,
        VerboseLevel verboseLvl) {
    /*if (verboseLvl >= VL_VERBOSE) {
    	cout << "  INT = " << print_vec(intCompStates) << endl;
    	cout << "  EXT = " << print_vec(extCompStates) << endl;
    	cout << endl;
    }*/
    // Store the component descriptor for later use
    ComponentDesc cd;
    cd.intCompStates = intCompStates;
    cd.extCompStates = extCompStates;
    cd.topSubset     = topSubset;
    cd.index         = comps.size();
    comps.emplace_back(cd);

    return true;
}

//=============================================================================

// Returns true if M contains arcs from M[row] to other states of the same SCC
static inline
bool HasInternalArcs(const ublas::compressed_matrix<double> &M,
                     const SCCSubsetList &SCCSubsets, const marking_t row) {
    matRow_t Mrow(M, row);
    rowIter_t M_ij(Mrow.begin()), M_rowEnd(Mrow.end());
    while (M_ij != M_rowEnd) {
        const marking_t col(M_ij.index());
        if (*M_ij != 0.0) {
            if (SCCSubsets.subsetOf(col) == SCCSubsets.subsetOf(row))
                return true;
        }
        ++M_ij;
    }
    return false;
}

//=============================================================================

// Select the MRP states of the specified SCC and add the corresponding
// internal/external states in the two bitvectors. Note that an "external" state
// is a state that can be reached by a sequence of Q arcs followed by one Qbar
// or Delta arc. Internal states are MRP states reachable from Q arcs.
// If the SCC spans multiple StateSubsets, external states are the "frontier"
// of the SCC. Note that Intersect[intStates, extStates] = 0.
void AddAugmentedSCCSubset(const MRP &mrp, const SCCSubsetList &SCCSubsets,
                           sccindex_t sccIndex, vector<bool> &intStates,
                           vector<bool> &extStates) {
    assert(intStates.size() == mrp.N() && extStates.size() == mrp.N());

    queue<marking_t> dfsVisitStack;
    SCCSubsetList::selector SCCSel = SCCSubsets.selectorOf(sccIndex);
    for (size_t sccStateInd = 0; sccStateInd < SCCSel.count(); sccStateInd++) {
        marking_t initVisitState(SCCSel[sccStateInd]);

        // Visit the states in the same subset but not in the same SCC that can
        // be reached by non-preemptive Markovian transitions from initVisitState
        if (intStates[initVisitState])
            continue; //  Already visited.
        intStates[initVisitState] = true;
        extStates[initVisitState] = false;
        dfsVisitStack.push(initVisitState);

        while (!dfsVisitStack.empty()) {
            const marking_t mi(dfsVisitStack.front());
            const statesubset_t subset_i = mrp.Subsets.subsetOf(mi);
            dfsVisitStack.pop();

            // Visit states reached by Q arcs
            matRow_t Q_row(mrp.Q, mi);
            rowIter_t Q_ij(Q_row.begin()), Q_end(Q_row.end());
            for (; Q_ij != Q_end; ++Q_ij) {
                const marking_t mj(Q_ij.index());
                // const statesubset_t subset_j = mrp.Subsets.subsetOf(mj);
                if (subset_i == EXP_SUBSET/* && subset_j != EXP_SUBSET*/) {
                    // A Q arc that exits the Exponential Subset
                    // if (SCCSubsets.subsetOf(mj) != sccIndex) {
                        // mj is an external state; note, however, that if mj
                        // is in a different SCC reachable from another
                        // intStates, then mj should be classified as intState.
                        if (!intStates[mj])
                            extStates[mj] = true;
                    // }
                }
                else {
                    if (!intStates[mj]) {
                        intStates[mj] = true;
                        extStates[mj] = false;
                        dfsVisitStack.push(mj); // Continue DFS visit
                    }
                }
            }

            // List the external states reached by Qbar and Delta arcs
            if (subset_i != EXP_SUBSET) {
                matRow_t Qbar_row(mrp.Qbar, mi);
                rowIter_t Qbar_ij(Qbar_row.begin()), Qbar_end(Qbar_row.end());
                for (; Qbar_ij != Qbar_end; ++Qbar_ij) {
                    const marking_t mj(Qbar_ij.index());
                    // if (SCCSubsets.subsetOf(mj) != sccIndex) {
                        if (!intStates[mj])
                            extStates[mj] = true;
                    // }
                }
                matRow_t Delta_row(mrp.Delta, mi);
                rowIter_t Delta_ij(Delta_row.begin()), Delta_end(Delta_row.end());
                for (; Delta_ij != Delta_end; ++Delta_ij) {
                    const marking_t mj(Delta_ij.index());
                    // if (SCCSubsets.subsetOf(mj) != sccIndex) {
                        if (!intStates[mj])
                            extStates[mj] = true;
                    // }
                }
            }
        }
    }
}

//=============================================================================

// Special value that indicates which markings are to be excluded when
// extracting sub-mrps
const marking_t NOT_SUBMRP_MARKING(size_t(-54));

//=============================================================================

// NOTE: extStates became absorbing EXP sub-states
void ExtractSubMRP2(const MRP &mrp, const vector<bool> &intStates,
                    const vector<bool> &extStates, MarkingSubset_t &mark2submark,
                    const ublas::vector<double> &mrpPi, MRP &subMrp,
                    VerboseLevel verboseLvl) {
    const marking_t N_MRP(mrp.N());
    marking_t N_SUBMRP(0);
    mark2submark.resize(N_MRP);

    // Count sub-MRP states
    for (marking_t m = 0; m < N_MRP; m++)
        mark2submark[m] = (intStates[m] ? N_SUBMRP++ : NOT_SUBMRP_MARKING);
    for (marking_t m = 0; m < N_MRP; m++) {
        verify(extStates[m] == false || intStates[m] == false);
        mark2submark[m] = (extStates[m] ? N_SUBMRP++ : mark2submark[m]);
    }
    assert(N_SUBMRP > marking_t(0));
    subMrp.pi0.resize(N_SUBMRP);

    // Conversion map for state subset indices
    const statesubset_t N_DET(mrp.Gen.size()), NOT_SUBDET(-31);
    statesubset_t N_SUBDET(0);
    vector<statesubset_t> det2subdet(N_DET);
    fill(det2subdet.begin(), det2subdet.end(), NOT_SUBDET);

    vector<statesubset_t> markToSubset;
    markToSubset.resize(N_SUBMRP);
    fill(markToSubset.begin(), markToSubset.end(), EXP_SUBSET);

    for (marking_t m(0); m < N_MRP; m++) {
        const marking_t sub_m = mark2submark[m];
        if (sub_m == NOT_SUBMRP_MARKING)
            continue;
        // Prepare State subsets
        statesubset_t ssm = mrp.Subsets.subsetOf(m);
        if (ssm == EXP_SUBSET || extStates[m])
            markToSubset[sub_m] = EXP_SUBSET;
        else {
            if (det2subdet[ssm] == NOT_SUBDET) {
                GenMarkingSubset gms;
                gms.index     = N_SUBDET++;
                gms.fg        = mrp.Gen[ssm].fg;
                gms.detDelay  = mrp.Gen[ssm].detDelay;
                gms.genTrnInd = mrp.Gen[ssm].genTrnInd;
                subMrp.Gen.push_back(gms);
                det2subdet[ssm] = gms.index;
            }
            markToSubset[sub_m] = det2subdet[ssm];
        }
        // Initial vector pi0
        subMrp.pi0(sub_m) = mrpPi(m);
    }
    subMrp.Subsets.ConstructSubsetList(det2subdet.size() + 2, markToSubset);

    // Filter out and copy the 3 MRP matrices
    typedef ublas::compressed_matrix<double>::const_iterator1 row_iterator_t;
    typedef ublas::compressed_matrix<double>::const_iterator2 elem_iterator_t;
    for (size_t mat = 0; mat < 3; mat++) {
        const ublas::compressed_matrix<double> *pSrc = nullptr;
        ublas::compressed_matrix<double> *pDst = nullptr;
        switch (mat) {
        case 0:	pSrc = &mrp.Q;     pDst = &subMrp.Q;	 break;
        case 1:	pSrc = &mrp.Qbar;  pDst = &subMrp.Qbar;  break;
        case 2:	pSrc = &mrp.Delta; pDst = &subMrp.Delta; break;
        }

        pDst->resize(size_t(N_SUBMRP), size_t(N_SUBMRP), false);
        row_iterator_t M_rowIt(pSrc->begin1());
        row_iterator_t M_rowItEnd(pSrc->end1());
        while (M_rowIt != M_rowItEnd) {
            const marking_t i = marking_t(M_rowIt.index1());
            const marking_t sub_i = mark2submark[i];
            if (sub_i != NOT_SUBMRP_MARKING && intStates[i]) {
                // Copy row i elements
                elem_iterator_t M_ij(M_rowIt.begin());
                elem_iterator_t iRowEnd(M_rowIt.end());
                while (M_ij != iRowEnd) {
                    const marking_t j = marking_t(M_ij.index2());
                    const marking_t sub_j = mark2submark[j];

                    // If this is really an irreducible terminal sub-MRP, than
                    // any transition should be contained inside the sub-MRP
                    verify(sub_j != NOT_SUBMRP_MARKING);
                    pDst->insert_element(size_t(sub_i), size_t(sub_j), *M_ij);

                    ++M_ij;
                }
            }
            ++M_rowIt;
        }
    }

    if (verboseLvl >= VL_VERBOSE) {
        cout << "===============================================\n";
        PrintMRP(subMrp);
        cout << "===============================================\n" << flush;
    }
}

//=============================================================================

static std::ostream &operator<<(std::ostream &o, const TQEntry &tqe) {
    o << "(" << std::get<TQE_STATE_SUBSET>(tqe) << ", ";
    o << std::get<TQ_HAS_PREV_COMPLEX_EDGE>(tqe) << ", ";
    o << std::get<TQE_SCC_INDEX>(tqe) << ")";
    return o;
}

string tqo_printer(const TQEntry &tqe) {
    ostringstream o;
    o << tqe;
    return o.str();
}

//=============================================================================

static void PushFreeSCC(const MRP &mrp, TopologicalOrderQueue &tqo, int scc) {
    assert(scc >= 0 && scc < tqo.numSCC && tqo.sccInfo[scc].numPrevSCCs == 0);
    SCCSubsetList::selector sel = tqo.SCCStateSubsets.selectorOf(scc);
    bool hasPrevComplexEdges = false;
    SCCEdgeSet_t::const_iterator emCIt1(tqo.InComplexEdges.lower_bound(make_pair(scc, -1)));
    SCCEdgeSet_t::const_iterator emCIt2(tqo.InComplexEdges.upper_bound(make_pair(scc, int(tqo.numSCC) + 1)));
    if (emCIt1 != emCIt2)
        hasPrevComplexEdges = true;
    if (tqo.sccInfo[scc].isSimple)
        tqo.FreeQueue.insert(make_tuple(mrp.Subsets.subsetOf(sel[0]), hasPrevComplexEdges, scc));
    else
        tqo.FreeQueue.insert(make_tuple(COMPLEX_COMPONENT_SUBSET, hasPrevComplexEdges, scc));
}

//=============================================================================

static void PopFreeSCC(const MRP &mrp, TopologicalOrderQueue &tqo,
                       set<TQEntry>::iterator it) {
    assert(tqo.FreeQueue.size() != 0 && it != tqo.FreeQueue.end());
    int top = std::get<TQE_SCC_INDEX>(*it);
    tqo.FreeQueue.erase(it);
    SCCEdgeSet_t::const_iterator emSIt1(tqo.OutSimpleEdges.lower_bound(make_pair(top, -1)));
    SCCEdgeSet_t::const_iterator emSIt2(tqo.OutSimpleEdges.upper_bound(make_pair(top, int(tqo.numSCC) + 1)));
    for (; emSIt1 != emSIt2; emSIt1++) {
        if (--tqo.sccInfo[emSIt1->second].numPrevSCCs == 0) {
            PushFreeSCC(mrp, tqo, emSIt1->second);
        }
    }
    SCCEdgeSet_t::const_iterator emCIt1(tqo.OutComplexEdges.lower_bound(make_pair(top, -1)));
    SCCEdgeSet_t::const_iterator emCIt2(tqo.OutComplexEdges.upper_bound(make_pair(top, int(tqo.numSCC) + 1)));
    for (; emCIt1 != emCIt2; emCIt1++) {
        if (--tqo.sccInfo[emCIt1->second].numPrevSCCs == 0) {
            PushFreeSCC(mrp, tqo, emCIt1->second);
        }
    }
}

//=============================================================================

void FindBottomSCCs(const MRP &mrp, const vector<sccindex_t> &SCC,
                    sccindex_t SccCount, vector<SCCInfo> &sccInfo) {
    // Find bottom SCCs
    assert(sccInfo.size() == (size_t)SccCount);
    for (sccindex_t s(0); s < SccCount; s++)
        sccInfo[s].isBottomSCC = true;
    for (size_t k = 0; k < 3; k++) {
        const ublas::compressed_matrix<double> *pMat = nullptr;
        switch (k) {
        case 0:	 pMat = &mrp.Q;		 break;
        case 1:  pMat = &mrp.Qbar;	 break;
        case 2:  pMat = &mrp.Delta;  break;
        }
        ublas::compressed_matrix<double>::const_iterator1 rowIt;
        ublas::compressed_matrix<double>::const_iterator2 colIt;
        for (rowIt = pMat->begin1(); rowIt != pMat->end1(); ++rowIt) {
            size_t i = rowIt.index1();
            for (colIt = rowIt.begin(); colIt != rowIt.end(); ++colIt) {
                size_t j = colIt.index2();
                if (SCC[i] != SCC[j])
                    sccInfo[SCC[i]].isBottomSCC = false;
            }
        }
    }
}

//=============================================================================

// A compressed_matrix<> iterator that seeks every nonzero element, in every
// row of the matrix, as if the matrix was a vector in row/column order.
class linear_compressed_matrix_iterator {
    typedef ublas::compressed_matrix<double> mat_t;
    const mat_t *pM;
    size_t i, i2_it, i2_end;

    inline void next_row() {
        do {
            ++i;
            if (! bool(*this))
                return;
            i2_it = pM->index1_data()[i];
            i2_end = pM->index1_data()[i + 1];
        }
        while (i2_it >= i2_end);
    }
public:
    inline linear_compressed_matrix_iterator(const mat_t *_pM)
        : pM(_pM), i(-1), i2_it(0), i2_end(0) { next_row(); }

    inline operator bool() const {  return (i < pM->filled1() - 1);  }

    inline linear_compressed_matrix_iterator &operator++() {
        assert(*this);
        if (++i2_it >= i2_end)
            next_row();
        return *this;
    }

    inline size_t index1() const {  assert(*this); return i;  }
    inline size_t index2() const {  assert(*this); return pM->index2_data()[i2_it];  }
    inline size_t value() const  {  assert(*this); return pM->value_data()[i2_it];  }

    inline std::pair<int, int> index12() const {  return make_pair(int(index1()), int(index2()));  }
};

//=============================================================================

static void AssembleTopologicalSparseMatrix(const MRP &mrp,
        ublas::compressed_matrix<double> &outM) {
    outM.resize(mrp.N(), mrp.N(), false);
    size_t nnz = 0;

    for (size_t phase = 0; phase < 2; phase++) {
        pair<int, int> last12(-1, -1);
        linear_compressed_matrix_iterator lcmQ(&mrp.Q), lcmQbar(&mrp.Qbar);
        linear_compressed_matrix_iterator lcmDelta(&mrp.Delta), *pMinLcm;
        if (phase == 1)
            outM.reserve(nnz, false);

        for (;;) {
            pMinLcm = nullptr;
            if (lcmQ)
                pMinLcm = &lcmQ;
            if (lcmQbar && (pMinLcm == nullptr || pMinLcm->index12() > lcmQbar.index12()))
                pMinLcm = &lcmQbar;
            if (lcmDelta && (pMinLcm == nullptr || pMinLcm->index12() > lcmDelta.index12()))
                pMinLcm = &lcmDelta;

            if (pMinLcm == nullptr)
                break;  // End of iteration...
            if (pMinLcm->index12() == last12) {
                ++(*pMinLcm);
                continue;  // Same <i,j> as previous iteration
            }
            last12 = pMinLcm->index12();
            ++(*pMinLcm);

            if (phase == 0)
                ++nnz;
            else {
                outM.insert_element(last12.first, last12.second, 1.0);
            }
        }
    }
}

//=============================================================================

static void PrepareTopologicalOrderQueue(const MRP &mrp, const MRPStateKinds &stKinds,
                                         TopologicalOrderQueue &tqo,
                                         VerboseLevel verboseLvl) 
{
    // SCC indexes of each MRP state
    vector<sccindex_t> SCC;
    do {
        ublas::compressed_matrix<double> outM;
        AssembleTopologicalSparseMatrix(mrp, outM);
        tqo.numSCC = TarjanIterative(outM, SCC);
        tqo.sccInfo.resize(tqo.numSCC);
        /*print_matrix(cout, mrp.Q, "Q");
        print_matrix(cout, mrp.Qbar, "Qbar");
        print_matrix(cout, mrp.Delta, "Delta");
        print_matrix(cout, mrp.Q+mrp.Qbar+mrp.Delta, "Sum");
        print_matrix(cout, outM, "outM");//*/
    }
    while (0);
    FindBottomSCCs(mrp, SCC, tqo.numSCC, tqo.sccInfo);
    int numBSCC = 0;
    for (int s = 0; s < tqo.numSCC; s++)
        if (tqo.sccInfo[s].isBottomSCC)
            numBSCC++;

    if (verboseLvl >= VL_BASIC) {
        cout << "THE MRP HAS " << tqo.numSCC << " DISTINCT SCC(s) ";
        cout << "AND " << numBSCC << " BSCC(s)." << endl;
        if (verboseLvl >= VL_VERBOSE) {
            cout << "   SCC: " << print_vec(SCC) << endl;
            cout << endl;
        }
    }

    // Classify the SCC list
    tqo.SCCStateSubsets.ConstructSubsetList(tqo.numSCC, SCC);
    //vector<bool> isSimpleSCC(numSCC), isGhostSCC(numSCC);

    for (int sccIndex = 0; sccIndex < tqo.numSCC; sccIndex++) {
        SCCSubsetList::selector SCCSel = tqo.SCCStateSubsets.selectorOf(sccIndex);
        tqo.sccInfo[sccIndex].isGhost = !tqo.sccInfo[sccIndex].isBottomSCC;
        tqo.sccInfo[sccIndex].isSimple = true;
        tqo.sccInfo[sccIndex].numPrevSCCs = 0;

        tqo.sccInfo[sccIndex].subset = mrp.Subsets.subsetOf(SCCSel[0]);
        for (size_t i = 0; i < SCCSel.count(); i++) {
            marking_t mi(SCCSel[i]);
            // A complex component has internal Delta or Qbar arc
            if (tqo.sccInfo[sccIndex].isSimple)
                tqo.sccInfo[sccIndex].isSimple = !HasInternalArcs(mrp.Qbar, tqo.SCCStateSubsets, mi);
            if (tqo.sccInfo[sccIndex].isSimple)
                tqo.sccInfo[sccIndex].isSimple = !HasInternalArcs(mrp.Delta, tqo.SCCStateSubsets, mi);

            // A ghost component has no regenerative/initial state
            if (0 != (stKinds.kinds[mi] & (MSK_REGENSTATE | MSK_INITSTATE)))
                tqo.sccInfo[sccIndex].isGhost = false;

            // Determine if there is a single or multiple state subsets
            statesubset_t sub_i = mrp.Subsets.subsetOf(mi);
            if (sub_i != tqo.sccInfo[sccIndex].subset)
                tqo.sccInfo[sccIndex].subset = MULTIPLE_SUBSETS;
        }
    }

    // Build up the edge list of the SCCs
    for (int sccIndex = 0; sccIndex < tqo.numSCC; sccIndex++) {
        SCCSubsetList::selector SCCSel = tqo.SCCStateSubsets.selectorOf(sccIndex);
        for (size_t k = 0; k < 3; k++) {
            const ublas::compressed_matrix<double> *pMat = nullptr;
            switch (k) {
            case 0:  pMat = &mrp.Q;      break;
            case 1:  pMat = &mrp.Qbar;   break;
            case 2:  pMat = &mrp.Delta;  break;
            }
            for (size_t i = 0; i < SCCSel.count(); i++) {
                marking_t mi(SCCSel[i]);
                matRow_t ithRow(*pMat, mi);
                rowIter_t M_ij(ithRow.begin()), M_rowEnd(ithRow.end());
                while (M_ij != M_rowEnd) {
                    const marking_t mj(M_ij.index());
                    int mj_scc = tqo.SCCStateSubsets.subsetOf(mj);
                    if (*M_ij != 0.0 && sccIndex != mj_scc) {
                        // Add an SCC edge between SCC[mi] and SCC[mj]
                        EdgeType_t e(make_pair(sccIndex, mj_scc));
                        if (k == 0) {
                            if (tqo.OutComplexEdges.count(e) == 0 &&
                                    tqo.OutSimpleEdges.count(e) == 0) {
                                // Add a simple edge
                                tqo.OutSimpleEdges.insert(e);
                                tqo.sccInfo[mj_scc].numPrevSCCs++;
                            }
                        }
                        else {
                            if (tqo.OutComplexEdges.count(e) == 0) {
                                if (tqo.OutSimpleEdges.count(e) == 0) {
                                    // Add as complex edge
                                    tqo.OutComplexEdges.insert(e);
                                    tqo.sccInfo[mj_scc].numPrevSCCs++;
                                }
                                else {
                                    // Upgrade e from simple to complex edge
                                    tqo.OutSimpleEdges.erase(e);
                                    tqo.OutComplexEdges.insert(e);
                                }
                            }
                        }
                    }
                    ++M_ij;
                }
            }
        }
    }

    // Build up the in-going edge list
    SCCEdgeSet_t::const_iterator it;
    for (it = tqo.OutSimpleEdges.begin(); it != tqo.OutSimpleEdges.end(); ++it)
        tqo.InSimpleEdges.insert(make_pair(it->second, it->first));
    for (it = tqo.OutComplexEdges.begin(); it != tqo.OutComplexEdges.end(); ++it)
        tqo.InComplexEdges.insert(make_pair(it->second, it->first));

    // Print the digraph informations
    if (verboseLvl >= VL_VERBOSE) {
        cout << "ADJACENCY DIGRAPH CHI:" << endl;
        for (int sccIndex = 0; sccIndex < tqo.numSCC; sccIndex++) {
            cout << (tqo.sccInfo[sccIndex].isBottomSCC ? "  BSCC" : "   SCC");
            cout << " z" << left << setw(3) << sccIndex << " : ";
            cout << (tqo.sccInfo[sccIndex].isSimple ? "SIMPLE " : "COMPLX ");
            cout << (tqo.sccInfo[sccIndex].isGhost ? "GHOST " : "      ");
            cout << "  NumPrevSCC=" << tqo.sccInfo[sccIndex].numPrevSCCs;
            cout << "  Next={";
            SCCEdgeSet_t::const_iterator emSIt1(tqo.OutSimpleEdges.lower_bound(make_pair(sccIndex, -1)));
            SCCEdgeSet_t::const_iterator emSIt2(tqo.OutSimpleEdges.upper_bound(make_pair(sccIndex, int(tqo.numSCC) + 1)));
            size_t i = 0;
            for (; emSIt1 != emSIt2; emSIt1++, i++)
                cout << (i == 0 ? " " : ", ") << "S:z" << emSIt1->second;
            SCCEdgeSet_t::const_iterator emCIt1(tqo.OutComplexEdges.lower_bound(make_pair(sccIndex, -1)));
            SCCEdgeSet_t::const_iterator emCIt2(tqo.OutComplexEdges.upper_bound(make_pair(sccIndex, int(tqo.numSCC) + 1)));
            for (; emCIt1 != emCIt2; emCIt1++, i++)
                cout << (i == 0 ? " " : ", ") << "C:z" << emCIt1->second;
            cout << " }\n\t\t\t\t\t   States={";
            SCCSubsetList::selector SCCSel = tqo.SCCStateSubsets.selectorOf(sccIndex);
            for (size_t i = 0; i < SCCSel.count(); i++)
                cout << (i == 0 ? " " : ", ") << int(SCCSel[i]) + 1;
            cout << " }" << endl;
        }
        cout << endl;
    }

    // Prepare the topological sort
    for (int sccIndex = 0; sccIndex < tqo.numSCC; sccIndex++) {
        if (tqo.sccInfo[sccIndex].numPrevSCCs == 0) {
            PushFreeSCC(mrp, tqo, sccIndex);
        }
    }
    assert(!tqo.FreeQueue.empty());

    if (verboseLvl >= VL_VERBOSE) {
        cout << "FREE QUEUE: " << print_vec(tqo.FreeQueue, tqo_printer) << endl << endl;
    }
}

//=============================================================================

bool SolveComponent_Forward(const MRP &compMrp,
                            const statesubset_t topSubset,
                            PetriNetSolution &outSol,
                            const SolverParams &spar,
                            VerboseLevel verboseLvl) 
{
    const double totalInputProb = vec_sum(compMrp.pi0);
    const marking_t N_MRP = compMrp.N();
    outSol.stateProbs.resize(N_MRP);
    std::fill(outSol.stateProbs.begin(), outSol.stateProbs.end(), 0.0);

    // Zero input probability, means zero output. Do nothing.
    if (totalInputProb == 0.0) {
        if (verboseLvl >= VL_BASIC)
            cout << "\nSKIPPING COMPONENT WITH ZERO INPUT PROBABILITY." << endl;
        return false;
    }

    performance_timer timer;
    if (topSubset == COMPLEX_COMPONENT_SUBSET) {
        // SOLUTION OF A COMPLEX COMPONENT
        MRPStateKinds subStKinds;
        ClassifyMRPStates(compMrp, subStKinds, VL_NONE);
        // PrintMRP(compMrp);
        // PetriNetSolution explSol;
        // SteadyStateMRPExplicit(compMrp, subStKinds, explSol, spar, verboseLvl);
        if (!SteadyStateMRPImplicit(compMrp, subStKinds, outSol,
                                    spar, verboseLvl))
            throw program_exception("Solution method does not converge.");

        // switch (numAugmentedStates>0 ? compProcMethod : CPM_IMPLICIT) {
        // 	case CPM_REPEAT_PRODUCTS: {
        // 		vector<bool> rowSelB(N_MRP); // TODO: fare un range_selector
        // 		size_t numIntStates = count(intCompStates.begin(), intCompStates.end(), true);
        // 		fill(rowSelB.begin(), rowSelB.begin()+ numIntStates, true);
        // 		fill(rowSelB.begin() + numIntStates, rowSelB.end(), false);
        // 		typedef subset_element_selector<bool> rowsel_t;
        // 		typedef EmcVectorMatrixProduct<full_element_selector, full_element_selector> PxV_t;
        // 		full_element_selector all_submrp_elems(N_MRP);
        // 		rowsel_t int_submrp_rows(&rowSelB, true);
        // 		//MRPSubsetList intSubset;
        // 		//FilterSubsets(compMrp.Subsets, intSubset, int_submrp_rows);
        // 		PxV_t PxV(compMrp, spar, compMrp.Subsets, KED_FORWARD,
        // 				  all_submrp_elems, all_submrp_elems);
        // 		double residNorm; // Residual probability inside internal states
        // 		outSol.stateProbs = compMrp.pi0;
        // 		do {
        // 			outSol.stateProbs = PxV.product(outSol.stateProbs);
        // 			//residNorm = vec_sum(outSol.stateProbs, int_submrp_rows);
        // 			residNorm = 0.0;
        // 			for (marking_t m(0); m<N_MRP; ++m) {
        // 				if (intCompStates[m]) {
        // 					marking_t sm(mark2submark[m]);
        // 					if (tqo.SCCStateSubsets.subsetOf(sm) == topScc ||
        // 						!tqo.sccInfo[tqo.SCCStateSubsets.subsetOf(sm)].isBottomSCC)
        // 						residNorm += outSol.stateProbs[sm];
        // 				}
        // 			}
        // 			//cout << "   " << outSol.stateProbs << "  ";
        // 			//cout << "   residNorm="<<residNorm<<endl;
        // 		} while (residNorm >= spar.epsilon);
        // 		//set_vec(outSol.stateProbs, 0.0, int_submrp_rows);
        // 		for (marking_t m(0); m<N_MRP; ++m) {
        // 			if (intCompStates[m]) {
        // 				marking_t sm(mark2submark[m]);
        // 				if (tqo.SCCStateSubsets.subsetOf(sm) == topScc ||
        // 					!tqo.sccInfo[tqo.SCCStateSubsets.subsetOf(sm)].isBottomSCC)
        // 					outSol.stateProbs[sm] = 0.0;
        // 			}
        // 		}
        // 		break;
        // 	}

        // 	case CPM_IMPLICIT: {
        // 		MRPStateKinds subStKinds;
        // 		ClassifyMRPStates(compMrp, subStKinds, VL_NONE);
        // 		if (!SteadyStateMRPImplicit(compMrp, subStKinds, outSol,
        // 									spar, VL_BASIC))
        // 			throw program_exception("Solution method does not converge.");
        // 		break;

        // 	}

        // 	default:
        // 		throw program_exception("");
        // }
    }
    else {
        // SOLUTION OF A SIMPLE COMPONENT

        if (compMrp.Gen.size() == 0) {
            // Solve the simple component in steady state
            LinearIterationPrinter printOut(verboseLvl);
            BSCC bscc;
            BottomStronglyConnectedComponents(compMrp.Q, bscc);
            ublas::zero_vector<double> zeroRho(N_MRP);
            if (!Solve_SteadyState_CTMC(compMrp.Q, outSol.stateProbs, compMrp.pi0,
                                        zeroRho, KED_FORWARD, spar, bscc, printOut))
                throw program_exception("Solution method does not converge.");
        }
        else {
            // Solve the simple component in transient
            verify(compMrp.Gen.size() == 1);
            full_element_selector all_elems(N_MRP);
            LinearIterationPrinter printOut(verboseLvl);
            const GenMarkingSubset &SgenMS = compMrp.Gen[0];
            ublas::vector<double> ag(N_MRP), bg(N_MRP);
            MRPSubsetList::selector GenStates(compMrp.Subsets.selectorOf(SgenMS.index));
            verify(GenStates.count() != 0);
            if (vec_sum(compMrp.pi0, GenStates) == 0.0) {
                if (verboseLvl >= VL_BASIC)
                    cout << "\nSKIPPING COMPONENT WITH ZERO INTERNAL INPUT PROBABILITY." << endl;
                return false;
            }

            // Compute uniformization over the Sgen subset *SgenIt
            UniformizationGen(compMrp.Q, compMrp.pi0, ag, &bg, SgenMS.fg.c_str(),
                              spar.epsilon, KED_FORWARD, GenStates, all_elems, printOut);
            add_prod_vecmat(outSol.stateProbs, ag, compMrp.Delta, GenStates, all_elems);
            add_prod_vecmat(outSol.stateProbs, bg, compMrp.Qbar, GenStates, all_elems);
            add_vec(outSol.stateProbs, compMrp.pi0, compMrp.Subsets.selectorOf(EXP_SUBSET));
        }
    }

    // Third step: re-normalize to avoid rounding error propagations

    double totalOutputProb = vec_sum(outSol.stateProbs);
    outSol.stateProbs /= totalOutputProb;
    outSol.stateProbs *= totalInputProb;

    if (verboseLvl >= VL_BASIC)
        cout << "COMPONENT SOLUTION TIME: " << timer << endl;
    if (verboseLvl >= VL_VERBOSE) {
        cout << "\nINPUT COMPONENT PROBABILITY:\n  " << compMrp.pi0;
        cout << " norm=" << vec_sum(compMrp.pi0) << endl;
        cout << "OUTPUT COMPONENT PROBABILITY:\n  " << outSol.stateProbs;
        cout << " norm=" << vec_sum(outSol.stateProbs) << endl;
    }

    return true;
}

//=============================================================================

void SolveComponent_Backward(const MRP &compMrp,
                             const statesubset_t topSubset,
                             const ublas::vector<double> &rho,
                             PetriNetSolution &outSol,
                             const SolverParams &spar,
                             VerboseLevel verboseLvl) {
    const marking_t N_MRP = compMrp.N();
    assert(rho.size() == N_MRP);

    // Initialize the ingoing backward probability vector
    outSol.stateProbs.resize(N_MRP);
    std::fill(outSol.stateProbs.begin(), outSol.stateProbs.end(), 0.0);
    if (topSubset == COMPLEX_COMPONENT_SUBSET) {
        // SOLVE A COMPLEX COMPONENT
        MRPStateKinds compStKinds;
        ClassifyMRPStates(compMrp, compStKinds, VL_NONE);
        if (!SteadyStateMRPImplicitBackward(compMrp, compStKinds, rho,
                                            outSol.stateProbs, spar, verboseLvl))
            throw program_exception("Solution method does not converge.");
    }
    else {
        // SOLVE A SIMPLE COMPONENT
        if (compMrp.Gen.size() == 0) {
            // Solve the simple component in steady state
            BSCC bscc;
            LinearIterationPrinter printOut(verboseLvl);
            BottomStronglyConnectedComponents(compMrp.Q, bscc);
            // cout << rho <<endl;
            // cout << "sum(rho) = " << vec_sum(rho) << endl;
            // cout << "sum(compMrp.pi0) = " << vec_sum(compMrp.pi0) << endl;
            ublas::zero_vector<double> zeroPi(N_MRP);
            if (!Solve_SteadyState_CTMC(compMrp.Q, outSol.stateProbs, zeroPi,
                                        rho, KED_BACKWARD, spar, bscc, printOut, &rho))
                throw program_exception("Solution method does not converge.");
        }
        else {
            // Solve the simple component in transient
            // P^g y  =  Omega (Delta y) + Psi (Qbar y)
            assert(compMrp.Gen.size() == 1);
            full_element_selector all_elems(N_MRP);
            LinearIterationPrinter printOut(verboseLvl);
            const GenMarkingSubset &SgenMS = compMrp.Gen[0];
            ublas::vector<double> DeltaX(N_MRP), QbarX(N_MRP), outExp, outExpAcc;
            ublas::vector<double> *no_vec = nullptr;
            MRPSubsetList::selector GenStates(compMrp.Subsets.selectorOf(SgenMS.index));
            assert(GenStates.count() != 0);

            prod_matvec(DeltaX, compMrp.Delta, rho, GenStates, all_elems);
            UniformizationGen(compMrp.Q, DeltaX, outExp, no_vec, SgenMS.fg.c_str(),
                              spar.epsilon * 1.0e-1, KED_BACKWARD,
                              GenStates, all_elems, printOut);
            add_vec(outSol.stateProbs, outExp, all_elems);

            prod_matvec(QbarX, compMrp.Qbar, rho, GenStates, all_elems);
            if (vec_sum(QbarX) != 0.0) {
                UniformizationGen(compMrp.Q, QbarX, outExp, &outExpAcc, SgenMS.fg.c_str(),
                                  spar.epsilon * 1.0e-1, KED_BACKWARD,
                                  GenStates, all_elems, printOut);
                add_vec(outSol.stateProbs, outExpAcc, all_elems);
            }
        }
    }

    if (verboseLvl >= VL_VERBOSE) {
        cout << "\nINPUT COMPONENT PROBABILITY:\n  " << rho;
        cout << " norm=" << vec_sum(rho) << endl;
        cout << "OUTPUT COMPONENT PROBABILITY:\n  " << outSol.stateProbs;
        cout << " norm=" << vec_sum(outSol.stateProbs) << endl;
        cout << "\n\n\n";
    }
}

//=============================================================================

class SteadyStateMRPComponentProcessor : public ComponentProcessor {
public:
    SteadyStateMRPComponentProcessor(const MRP &_mrp, const SolverParams &_spar,
                                     PetriNetSolution &_sol)
        : mrp(_mrp), spar(_spar), sol(_sol) { pi = mrp.pi0; }

    virtual bool ProcessNextComponent(const TopologicalOrderQueue &tqo,
                                      const sccindex_t topScc,
                                      const statesubset_t topSubset,
                                      const vector<bool> &intCompStates,
                                      const vector<bool> &extCompStates,
                                      const size_t numAugmentedStates,
                                      VerboseLevel verboseLvl) override {
        const marking_t N_MRP = mrp.N();
        MarkingSubset_t mark2submark;
        PetriNetSolution outSol;
        MRP subMrp;

        // First step: build up the sub-MRP
        ExtractSubMRP2(mrp, intCompStates, extCompStates,
                       mark2submark, pi, subMrp, VL_NONE);

        // PrintMRP(subMrp);

        // Second step: solve the subordinated system
        if (!SolveComponent_Forward(subMrp, topSubset, outSol, spar, verboseLvl))
            return false; // No input probability

        // Distribute the output probability back into pi
        for (marking_t m(0); m < N_MRP; ++m) {
            if (mark2submark[m] != NOT_SUBMRP_MARKING) {
                // if (outSol.stateProbs[ mark2submark[m] ] > 0.0)
                pi[m] = outSol.stateProbs[ mark2submark[m] ];
            }
        }

        if (verboseLvl >= VL_VERBOSE) {
            cout << "MRP PROBABILITY:\n  " << pi << endl;
            cout << "\n\n\n";
        }

        return true;
    }

    virtual void EndProcessing() override {
        pi /= vec_sum(pi);
        sol.stateProbs.swap(pi);

        // TODO: add support to firing frequencies
        sol.firingFreq.resize(mrp.N());
        std::fill(sol.firingFreq.begin(), sol.firingFreq.end(), -1.0);
    }

private:
    const MRP &mrp;
    ublas::vector<double> pi;	// Intermediate vector that holds the flowing probability
    const SolverParams   &spar;	// Solver parameters
    PetriNetSolution     &sol;	// Output solution
};

//=============================================================================

int statesubset_to_section(statesubset_t ssub) {
    if (ssub == EXP_SUBSET)
        return 0;
    else if (ssub == MULTIPLE_SUBSETS)
        return 1;
    else 
        return (int(ssub) + 2);
};

//=============================================================================

void SolutionFromXYvars(const std::vector<std::vector<int>>& mvars,
                        const TopologicalOrderQueue& tqo,
                        std::vector<int>& x_vars /* out */)
{
    const int N_SCC = tqo.sccInfo.size();

    auto section_of = [&](int scc_v) -> int /* section */ {
        assert(scc_v >= 0 && scc_v < int(tqo.sccInfo.size()));
        return statesubset_to_section(tqo.sccInfo[scc_v].subset);
    };

    x_vars.resize(N_SCC);
    for (int scc=0; scc<N_SCC; scc++) {
        x_vars[scc] = mvars[section_of(scc)][scc];
    }
}

//=============================================================================

bool DeriveAcyclicPartitionsFromXvars(const MRP &mrp, 
                                      const std::vector<int>& x_vars,
                                      const TopologicalOrderQueue& tqo,
                                      const std::set<std::pair<int, int>>& EN,
                                      const std::set<std::pair<int, int>>& EJ,
                                      std::set<std::pair<compId_t, compId_t>>& EC /* out */,
                                      compId_t& numComps/* out */,
                                      std::vector<compId_t>& compIds/* out */,
                                      std::vector<int>* compSection/* out, optional */,
                                      std::vector<compId_t>& topoOrdComp/* out */,
                                      VerboseLevel verboseLvl)
{
    const int N_SECT = mrp.Gen.size() + 2;
    const int N_SCC = tqo.sccInfo.size();
    // Component numbers and ids
    numComps = compId_t(0);
    compIds.resize(N_SCC);

    auto section_of = [&](int scc_v) -> int /* section */ {
        assert(scc_v >= 0 && scc_v < int(tqo.sccInfo.size()));
        return statesubset_to_section(tqo.sccInfo[scc_v].subset);
    };

    // Making new components
    if (verboseLvl >= VL_BASIC)
        cout << "DERIVING COMPONENTS..." << endl;
    numComps = 0;
    std::fill(compIds.begin(), compIds.end(), compId_t(-1));
    for (int D=0; D<N_SECT; D++) {
        compId_t numSectComps(0);
        for (int scc=0; scc<N_SCC; scc++) {
            if (section_of(scc) == D) { // Internal
                verify(x_vars[scc] >= 1);
                numSectComps = std::max(numSectComps, compId_t(x_vars[scc]));
                compIds[scc] = numComps + compId_t(x_vars[scc] - 1);
            }
        }
        if (verboseLvl >= VL_BASIC) {
            cout << "SECTION " << D << " HAS " << numSectComps << " COMPONENTS." << endl;
        }
        numComps += compId_t(numSectComps);
        if (compSection != nullptr)
            for (int j=0; j<numSectComps; j++)
                compSection->push_back(D);
    }
    if (verboseLvl >= VL_VERBOSE) {
        cout << "COMPONENT IDS: " << print_vec(compIds) << endl;
    }
    if (verboseLvl >= VL_BASIC) {
        cout << "THERE ARE " << numComps << " COMPONENTS.\n\n" << endl;
    }



    // Get the edges in the component graph
    EC.clear();
    for (auto e : EJ)
        if (compIds[e.second] != compIds[e.first])
            EC.insert(make_pair(compIds[e.second], compIds[e.first]));
    for (auto e : EN)
        if (compIds[e.second] != compIds[e.first])
            EC.insert(make_pair(compIds[e.second], compIds[e.first]));

    // Get the component order
    std::vector<size_t> compInDegrees(numComps);
    std::fill(compInDegrees.begin(), compInDegrees.end(), 0);
    for (auto e : EC)
        compInDegrees[e.second]++;
    std::set<compId_t> compFront;
    for (compId_t i(0); i<numComps; i++)
        if (compInDegrees[i] == 0)
            compFront.insert(i);

    // Visit components in topological order
    topoOrdComp.resize(0);
    topoOrdComp.reserve(numComps);
    while (!compFront.empty()) {
        compId_t topComp = *compFront.begin();
        topoOrdComp.push_back(topComp);
        compFront.erase(topComp);
        // cout << "visit " << topComp << endl;

        auto ecIt = EC.lower_bound(make_pair(topComp, compId_t(-1)));
        auto lastEcIt = EC.upper_bound(make_pair(topComp, compId_t(1 + numComps)));
        for (; ecIt != lastEcIt; ++ecIt) {
            if (--compInDegrees[ecIt->second] == 0)
                compFront.insert(ecIt->second);
        }
    }
    if (topoOrdComp.size() != size_t(numComps)) {
        // Not all components have been visited! The structure is not acyclic!
        return false;
    }

#ifndef NDEBUG
    // Verify component order
    for (auto e : EC) {
        verify(std::find(topoOrdComp.begin(), topoOrdComp.end(), e.first) < 
               std::find(topoOrdComp.begin(), topoOrdComp.end(), e.second));
    }
    // Verify that components do not cointain EN edges
    for (auto e : EN) {
        verify (compIds[e.second] != compIds[e.first]);
    }
#endif

    return true;
}


//=============================================================================

void ProcessComponentListFromXvars(const MRP &mrp, const MRPStateKinds &stKinds,
                                   ComponentProcessor &compProc,
                                   const std::vector<int>& x_vars,
                                   const TopologicalOrderQueue& tqo,
                                   const std::set<std::pair<int, int>>& EN,
                                   const std::set<std::pair<int, int>>& EJ,
                                   const bool print_in_dot,
                                   VerboseLevel verboseLvl) 
{
    const marking_t N_MRP = mrp.N();
    const int N_SCC = tqo.sccInfo.size();

    // section of a vertex
    auto section_of = [&](int scc_v) -> int /* section */ {
        assert(scc_v >= 0 && scc_v < int(tqo.sccInfo.size()));
        return statesubset_to_section(tqo.sccInfo[scc_v].subset);
    };
    auto printSectionName = [&mrp](int D, ostream& os) -> ostream& {
        assert(D>=0 && D<int(2+mrp.Gen.size()));
        if (D == 1)
            return os << "(MIXED)";
        else if (D == 0)
            return os << "(EXP)";
        else
            return os << "(GEN: " << (D-2) << ")";
    };

    // Component numbers and ids
    compId_t numComps(0);
    std::vector<compId_t> compIds(N_SCC);
    std::vector<int> compSection;
    std::set<std::pair<compId_t, compId_t>> EC;
    std::vector<compId_t> topoOrdComp;

    bool is_acyclic = DeriveAcyclicPartitionsFromXvars(mrp, x_vars, tqo, EN, EJ, EC,
                                                       numComps, compIds, &compSection, 
                                                       topoOrdComp, verboseLvl);


    // // Making new components
    // cout << "DERIVING COMPONENTS..." << endl;
    // numComps = 0;
    // std::fill(compIds.begin(), compIds.end(), compId_t(-1));
    // std::vector<int> compSection;
    // for (int D=0; D<N_SECT; D++) {
    //     compId_t numSectComps(0);
    //     for (int scc=0; scc<N_SCC; scc++) {
    //         if (section_of(scc) == D) { // Internal
    //             verify(mvars[D][scc] >= 1);
    //             numSectComps = std::max(numSectComps, compId_t(mvars[D][scc]));
    //             compIds[scc] = numComps + compId_t(mvars[D][scc] - 1);
    //         }
    //     }
    //     if (verboseLvl >= VL_BASIC) {
    //         cout << "SECTION " << D << " HAS " << numSectComps << " COMPONENTS." << endl;
    //     }
    //     numComps += compId_t(numSectComps);
    //     for (int j=0; j<numSectComps; j++)
    //         compSection.push_back(D);
    // }
    // if (verboseLvl >= VL_VERBOSE) {
    //     cout << "COMPONENT IDS: " << print_vec(compIds) << endl;
    // }
    // if (verboseLvl >= VL_BASIC) {
    //     cout << "THERE ARE " << numComps << " COMPONENTS.\n\n" << endl;
    // }



    // // Get the edges in the component graph
    // std::set<std::pair<compId_t, compId_t>> EC;
    // for (auto e : EJ)
    //     if (compIds[e.second] != compIds[e.first])
    //         EC.insert(make_pair(compIds[e.second], compIds[e.first]));
    // for (auto e : EN)
    //     if (compIds[e.second] != compIds[e.first])
    //         EC.insert(make_pair(compIds[e.second], compIds[e.first]));

    // cout << "\nCOMPONENT EDGES:" << endl;
    // for (auto e : EC)
    //     cout << "comp(" << e.first << ") -> comp(" << e.second << ")" << endl;
    // cout << "\n" << endl;




    // Print the SCC graph in Graphviz format
    if (print_in_dot) {
        cout << "\nWRITING DOT FILE..." <<endl;
        ofstream dot("/Users/elvio/Desktop/dag.dot");
        dot << "\n\n\ndigraph G{\n  rankdir=LR;\n";
        bool components_as_clusters=false;
        for (compId_t c(0); c<numComps; c++) {
            dot << "  subgraph " << (components_as_clusters?"cluster_":"") << "c" << c << " {\n";
            // Write all SCCs in component s
            int compD = 0;
            for (int scc=0; scc<N_SCC; scc++) {
                if (compIds[scc] == c) {
                    compD = section_of(scc);
                    dot << "    scc" << scc << " [label=\"scc "<<scc<<"\\n"
                         // << minSccVal[scc]<<","
                         // <<(maxSccVal[scc]==numeric_limits<int>::max()?"inf":to_string(maxSccVal[scc]))
                         // <<"] : " << sectionVal[scc] 
                         <<"c"<<c
                         <<" : D"<<section_of(scc)
                         <<" : " << x_vars[scc]
                         << "\", "
                        "style=filled, colorscheme=pastel19, color="
                         <<(compD+1)<< "];\n";
                }
            }

            dot << "    label = \"c" << c << " : ";
            printSectionName(compD, dot) << "\";\n";
            dot << "  }\n";
        }
        // Joinable edges
        dot << "  edge [style=dotted];\n  ";
        for (auto e : EJ)
            dot << "scc" << int(e.second) << " -> scc" << int(e.first) << "; ";
        dot << "\n\n";
        // Non-joinable edges
        dot << "  edge [style=solid];\n  ";
        for (auto e : EN)
            dot << "scc" << int(e.second) << " -> scc" << int(e.first) << "; ";
        dot << "\n\n";

        // Write the graph of components
        for (int c=0; c<numComps; c++) {
            dot << "  comp"<<c<<" [shape=rectangle label=\"Component "<<c
            <<"\nSection: D"<<compSection[c]<<" ";
            printSectionName(compSection[c], dot);
            dot << "\" " << "style=filled, colorscheme=pastel19, color="<<(compSection[c]+1);
            dot << "];\n";
        }
        dot << "  edge [style=solid];\n  ";
        for (auto e : EC) {
            dot << "  comp"<<e.first<<" -> comp"<<e.second<<";\n";
        }

        dot << "\n\n}\n\n\n"; 
        dot.close();
    }

    if (!is_acyclic)
        throw program_exception("Component structure is not acyclic.");




    // // Get the component order
    // std::vector<size_t> compInDegrees(numComps);
    // std::fill(compInDegrees.begin(), compInDegrees.end(), 0);
    // for (auto e : EC)
    //     compInDegrees[e.second]++;
    // std::set<compId_t> compFront;
    // for (compId_t i(0); i<numComps; i++)
    //     if (compInDegrees[i] == 0)
    //         compFront.insert(i);

    // // Visit components in topological order
    // std::vector<compId_t> topoOrdComp;
    // topoOrdComp.reserve(numComps);
    // while (!compFront.empty()) {
    //     compId_t topComp = *compFront.begin();
    //     topoOrdComp.push_back(topComp);
    //     compFront.erase(topComp);
    //     // cout << "visit " << topComp << endl;

    //     auto ecIt = EC.lower_bound(make_pair(topComp, compId_t(-1)));
    //     auto lastEcIt = EC.upper_bound(make_pair(topComp, compId_t(1 + numComps)));
    //     for (; ecIt != lastEcIt; ++ecIt) {
    //         if (--compInDegrees[ecIt->second] == 0)
    //             compFront.insert(ecIt->second);
    //     }
    // }
    // if (topoOrdComp.size() != size_t(numComps)) {
    //     cout << "VISITED COMPONENTS: " << topoOrdComp.size() << "/" << size_t(numComps) << endl;
    //     throw program_exception("Component structure is not acyclic.");
    // }

// #ifndef NDEBUG
//     // Verify component order
//     for (auto e : EC) {
//         verify(std::find(topoOrdComp.begin(), topoOrdComp.end(), e.first) < 
//                std::find(topoOrdComp.begin(), topoOrdComp.end(), e.second));
//     }
//     // Verify that components do not cointain EN edges
//     for (auto e : EN) {
//         verify (compIds[e.second] != compIds[e.first]);
//     }
// #endif

    // Process components
    vector<bool> intCompStates(N_MRP), extCompStates(N_MRP);
    vector<int> ProcessedSCCs;
    size_t numProcExpComp = 0, numProcGenComp = 0, numProcComplexComp = 0;
    for (compId_t count=0; count<numComps; count++) {
        const compId_t comp = topoOrdComp[count];

        // Determine how many SCCs are in component *comp
        ProcessedSCCs.resize(0);
        for (int scc = 0; scc<N_SCC; scc++)
            if (compIds[scc] == comp)
                ProcessedSCCs.push_back(scc);
        if (ProcessedSCCs.empty())
            continue;

        sccindex_t topScc = ProcessedSCCs.front();
        statesubset_t topSubset = tqo.sccInfo[topScc].subset;

#ifndef NDEBUG
        for (int otherScc : ProcessedSCCs) {
            verify(tqo.sccInfo[otherScc].subset == topSubset);
        }
#endif

        // Extend the selected states with the transitive closure
        //  w.r.t. exponential transitions and frontier absorbing states.
        fill(intCompStates.begin(), intCompStates.end(), false);
        fill(extCompStates.begin(), extCompStates.end(), false);
        for (size_t i = 0; i < ProcessedSCCs.size(); i++)
            AddAugmentedSCCSubset(mrp, tqo.SCCStateSubsets, ProcessedSCCs[i],
                                  intCompStates, extCompStates);
        const size_t numStates = std::count(intCompStates.begin(),
                                            intCompStates.end(), true);
        const size_t numAugmentedStates = std::count(extCompStates.begin(),
                                          extCompStates.end(), true);

        if (verboseLvl >= VL_BASIC) {
            cout << "PROCESSING COMPONENT IN " << console::cyan_fgnd() ;
            printSectionName(section_of(topScc), cout);
            cout << console::default_disp();
            cout << " SECTION WITH " << ProcessedSCCs.size() << " SCC";
            cout << (ProcessedSCCs.size() == 1 ? "" : "s");
            cout << " (States=" << numStates;
            if (numAugmentedStates > 0)
                cout << ", augmented to " << (numStates + numAugmentedStates);
            cout << ")." << endl;
        }

        // Process the component
        if (topSubset == MULTIPLE_SUBSETS)
            topSubset = COMPLEX_COMPONENT_SUBSET;
        if (compProc.ProcessNextComponent(tqo, topScc, topSubset, intCompStates, extCompStates,
                                          numAugmentedStates, verboseLvl)) 
        {
            if (topSubset == EXP_SUBSET)
                numProcExpComp++;
            else if (topSubset == COMPLEX_COMPONENT_SUBSET)
                numProcComplexComp++;
            else
                numProcGenComp++;
        }
    }

    compProc.EndProcessing();

    if (verboseLvl >= VL_BASIC) {
        cout << "\nPROCESSED " << numProcExpComp << " EXP, " << numProcGenComp << " GEN AND ";
        cout << numProcComplexComp << " MIXED COMPONENTS." << endl;
    }
}

//=============================================================================

void DeriveEJ_EN_sets(const MRP& mrp,
                      const TopologicalOrderQueue& tqo,
                      bool aggregateComplexComp,
                      std::set<EdgeType_t>& EJ /* out */, 
                      std::set<EdgeType_t>& EN /* out */,
                      VerboseLevel verboseLvl) 
{
    // Joinable and non-joinable edges (in the reverse form: dst <- src)
    EJ.clear();
    EN.clear();
    for (int sccIndex = 0; sccIndex < tqo.numSCC; sccIndex++) {
        const statesubset_t subScc_src = tqo.sccInfo[sccIndex].subset;
        SCCSubsetList::selector SCCSel = tqo.SCCStateSubsets.selectorOf(sccIndex);
        for (size_t k = 0; k < 3; k++) {
            const ublas::compressed_matrix<double> *pMat = nullptr;
            switch (k) {
            case 0:  pMat = &mrp.Q;      break;
            case 1:  pMat = &mrp.Qbar;   break;
            case 2:  pMat = &mrp.Delta;  break;
            }
            for (size_t i = 0; i < SCCSel.count(); i++) {
                marking_t mi(SCCSel[i]);
                matRow_t ithRow(*pMat, mi);
                rowIter_t M_ij(ithRow.begin()), M_rowEnd(ithRow.end());
                while (M_ij != M_rowEnd) {
                    const marking_t mj(M_ij.index());
                    int mj_scc = tqo.SCCStateSubsets.subsetOf(mj);
                    if (*M_ij != 0.0 && sccIndex != mj_scc) {
                        const statesubset_t subScc_dst = tqo.sccInfo[mj_scc].subset;
                        bool joinable;
                        if (subScc_src != subScc_dst)
                            joinable = false; // different sections
                        else if (subScc_src == MULTIPLE_SUBSETS) 
                            joinable = aggregateComplexComp; // any edge
                        else 
                            joinable = (k==0); // only Q edges are joinable

                        EdgeType_t e(make_pair(mj_scc, sccIndex)); // dst <- src
                        if (joinable) {
                            if (!EN.count(e))
                                EJ.insert(e);
                        }
                        else {
                            if (EJ.count(e))
                                EJ.erase(e);                                
                            EN.insert(e);
                        }

                    }
                    ++M_ij;
                }
            }
        }
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "\nTHERE ARE " << EJ.size() << " JOINABLE AND " 
             << EN.size() << " NON-JOINABLE EDGES." << endl;
    }
}

//=============================================================================

void ProcessComponentList_ILP(const MRP &mrp, const MRPStateKinds &stKinds,
                              ComponentProcessor &compProc,
                              bool aggregateComplexComp, VerboseLevel verboseLvl) 
{
    const int N_SECT = mrp.Gen.size() + 2;

    TopologicalOrderQueue tqo;
    PrepareTopologicalOrderQueue(mrp, stKinds, tqo, verboseLvl);
    const int N_SCC = tqo.sccInfo.size();

    auto printSectionLambda = [](statesubset_t sub, ostream& os) -> ostream& {
        if (sub == MULTIPLE_SUBSETS)
            return os << "(MIXED)";
        else if (sub == EXP_SUBSET)
            return os << "(EXP)";
        else
            return os << "(GEN: " << sub << ")";
    };

    if (verboseLvl >= VL_VERBOSE) {
        cout << "TOPOLOGICAL ORDER:" << endl;
        for (int scc=N_SCC-1; scc>=0; scc--) { //  visit SCCs in topological order
            cout << " " << scc;
            printSectionLambda(tqo.sccInfo[scc].subset, cout);
        }
        cout << endl;
    }

    // Joinable and non-joinable edges (in the reverse form: dst <- src)
    std::set<EdgeType_t> EJ, EN;
    DeriveEJ_EN_sets(mrp, tqo, aggregateComplexComp, EJ, EN, verboseLvl);

    // Table of mvars.
    // std::vector<std::vector<int>> mvars(N_SECT);
    // for (int d=0; d<N_SECT; d++) {
    //     mvars[d].resize(N_SCC);
    //     std::fill(mvars[d].begin(), mvars[d].end(), -1);
    // }
    std::vector<int> x_vars(N_SCC);
    std::fill(x_vars.begin(), x_vars.end(), -1);

    // Print the ILP in lp_solve format
    stringstream var_decl, binL_decl, binG_decl, constr, targetfn;
    // ND variables
    for (int D=0; D<N_SECT; D++) {
        var_decl << (D==0 ? "" : ", ") << "ND"<<D;
        targetfn << (D==0 ? "" : "+") << "ND"<<D;
        constr << "ND"<<D<<" >= 0;\n";
    }
    // section of a vertex
    auto section_of = [&](int scc_v) -> int /* section */ {
        return statesubset_to_section(tqo.sccInfo[scc_v].subset);
    };
    // x^D_v variables
    auto name_var = [&](int D, int scc_v) -> string {
        stringstream s;
        if (section_of(scc_v) == D)
            s << "x" << scc_v;
        else
            s << "y" << scc_v << "D" << D;
        return s.str();
    };
    std::set<EdgeType_t> Estar; // extended reachability,  src,first --> dst,decond
    std::set<EdgeType_t> EJ_fwd, EN_fwd; // src,first --> dst,decond
    for (auto edge : EJ)
        EJ_fwd.insert(make_pair(edge.second, edge.first));
    for (auto edge : EN)
        EN_fwd.insert(make_pair(edge.second, edge.first));
    // vertex v1 reaches vertex v2 ?
    auto may_reach = [&EJ_fwd, &EN_fwd, &Estar, &N_SCC](int v1, int v2) -> bool {
        std::vector<int> flags(N_SCC);
        std::fill(flags.begin(), flags.end(), 0);
        std::queue<int> visit_q;
        visit_q.push(v1);
        while (!visit_q.empty()) {
            int scc = visit_q.front();
            visit_q.pop();
            if (scc == v2)
                return true;  // v1 may reach v2

            auto lastEJ = EJ_fwd.upper_bound(make_pair(scc, sccindex_t(1 + N_SCC)));
            for (auto EJIt = EJ_fwd.lower_bound(make_pair(scc, -1)); EJIt != lastEJ; ++EJIt) {
                assert(EJIt->first == scc);
                if (flags[EJIt->second] == 0) {
                    flags[EJIt->second] = 1;
                    visit_q.push(EJIt->second);
                }
            }
            auto lastEN = EN_fwd.upper_bound(make_pair(scc, sccindex_t(1 + N_SCC)));
            for (auto ENIt = EN_fwd.lower_bound(make_pair(scc, -1)); ENIt != lastEN; ++ENIt) {
                assert(ENIt->first == scc);
                if (flags[ENIt->second] == 0) {
                    flags[ENIt->second] = 1;
                    visit_q.push(ENIt->second);
                }
            }
            auto lastEstar = Estar.upper_bound(make_pair(scc, sccindex_t(1 + N_SCC)));
            for (auto EstarIt = Estar.lower_bound(make_pair(scc, -1)); EstarIt != lastEstar; ++EstarIt) {
                assert(EstarIt->first == scc);
                if (flags[EstarIt->second] == 0) {
                    flags[EstarIt->second] = 1;
                    visit_q.push(EstarIt->second);
                }
            }
        }
        return false;
    };

    // Variables
    for (int k=0; k<2; k++) {
        for (int scc=0; scc<N_SCC; scc++) {
            for (int D=0; D<N_SECT; D++) {
                bool is_internal = (section_of(scc) == D);
                if (is_internal == (k==0)) {
                    var_decl << ", " << name_var(D, scc);
                    constr << name_var(D, scc) << " <= ND"<<D<<";\n"
                           << name_var(D, scc) << (is_internal ? " >= 1;\n" : ">= 0;\n");
               }
            }
        }
    }
    // Joinable edges
    for (int D=0; D<N_SECT; D++) {
        for (const auto& e : EJ) // v:second ---J-> v':first   => x(v) <= x(v')
            constr << name_var(D, e.second) << " <= " << name_var(D, e.first) << ";\n";
    }
    // Non-joinable edges
    for (int D=0; D<N_SECT; D++) {
        for (const auto& e : EN) { // v:second ---N-> v':first   => x(v) (op) x(v'), op= < or <=
            bool is_internal = (section_of(e.first) == D);
            constr << name_var(D, e.second) << (is_internal ? " + 1 <= " :  " <= ") 
                   << name_var(D, e.first) << ";\n";
        }
    }
    // Constraints for non-directly connected vertices
    constr << "\n";
    const int U = N_SCC+1;
    size_t kvars = 0;
    for (int D=0; D<N_SECT; D++) {
        for (int v1=N_SCC-1; v1 >= 0; v1--) {
            if (section_of(v1) != D)
                continue;
            for (int v2=N_SCC-1; v2 >=0; v2--) {
                if (section_of(v2) != D || v1 == v2)
                    continue;
                if (may_reach(v1, v2) || may_reach(v2, v1)) 
                    continue;
                binG_decl << (kvars==0 ? "" : ", ") << " bK"<<kvars;
                constr << "// v1="<<v1<<", v2="<<v2<<"\n";
                constr << "bK"<<kvars<<" >= 0;\n";
                constr << "bK"<<kvars<<" <= 1;\n";
                constr << U<<" * bK"<<kvars<<" - "<<U<<" + 1 <= x"<<v1<<" - x"<<v2<<";\n";
                constr << "x"<<v1<<" - x"<<v2<<" <= "<<U<<" * bK"<<kvars<<";\n";
                for (int Dprime=0; Dprime<N_SECT; Dprime++) {
                    if (Dprime != section_of(v1)) {
                        constr << name_var(Dprime, v1) << " <= " 
                               << name_var(Dprime, v2) << " + "<<U<<" * bK"<<kvars<<";\n";
                    }
                }
                // Estar.insert(make_tuple(v1, v2));
                kvars++;
            }
        }
        Estar.clear();
    }

    cout << "THERE ARE " << kvars << " K BOOLEAN VARIABLES IN THE ILP." << endl;
    cout << "ILP HAS " << (kvars + N_SCC*N_SCC) << " VARIABLES." <<endl;
    cout << "\nWRITING ILP..." << endl;
    char ilp_fname[L_tmpnam], sol_fname[L_tmpnam];
    strcpy(ilp_fname, "ilp.txt");
    strcpy(sol_fname, "ilp_solution.txt");
    // std::tmpnam(ilp_fname);
    // std::tmpnam(sol_fname);
    cout << "ILP FILENAME: " << ilp_fname << endl;
    cout << "ILP SOLUTION FILENAME: " << sol_fname << endl;
    ofstream ofs(ilp_fname);
    ofs << "min: " << targetfn.str() << ";\n";
    ofs << constr.str() << "\nint " << var_decl.str() 
        // << ";\nbin " << binL_decl.str()
        << ";\nbin " << binG_decl.str() << ";\n\n";
    ofs.close();

    cout << "\nSOLVING ILP..." << endl;
    ostringstream cmd;
    cmd << "lp_solve " << ilp_fname << " > " << sol_fname;
    cout << cmd.str() << endl;
    system(cmd.str().c_str());

    cout << "\nREADING ILP SOLUTION..." << endl;
    ifstream ifs(sol_fname);
    string line;
    int num_x_read = 0;
    while (getline(ifs, line)) {
        int scc_v, value;
        if (2 == sscanf(line.c_str(), "x%d %d", &scc_v, &value)) {
            x_vars[scc_v] = value;
            num_x_read++;
        }
    }
    ifs.close();
    if (num_x_read != N_SCC) {
        throw program_exception("Could not read back the ILP solution.");
    }


    // Now that we have the mvars[] matrix complete the processing.
    ProcessComponentListFromXvars(mrp, stKinds, compProc, x_vars, tqo, 
                                  EN, EJ, true, verboseLvl);
}

//=============================================================================

void ProcessComponentListOptimal(const MRP &mrp, const MRPStateKinds &stKinds,
                                 ComponentProcessor &compProc,
                                 bool aggregateComplexComp, VerboseLevel verboseLvl) 
{
    const int N_SECT = mrp.Gen.size() + 2;

    TopologicalOrderQueue tqo;
    PrepareTopologicalOrderQueue(mrp, stKinds, tqo, verboseLvl);
    const int N_SCC = tqo.sccInfo.size();

    auto printSectionName = [&mrp](int D, ostream& os) -> ostream& {
        assert(D>=0 && D<int(2+mrp.Gen.size()));
        if (D == 1)
            return os << "(MIXED)";
        else if (D == 0)
            return os << "(EXP)";
        else
            return os << "(GEN: " << (D-2) << ")";
    };
    // section of a vertex
    auto section_of = [&](int scc_v) -> int /* section */ {
        assert(scc_v >= 0 && scc_v < int(tqo.sccInfo.size()));
        return statesubset_to_section(tqo.sccInfo[scc_v].subset);
    };

    if (verboseLvl >= VL_VERBOSE) {
        cout << "TOPOLOGICAL ORDER:" << endl;
        for (int scc=N_SCC-1; scc>=0; scc--) { //  visit SCCs in topological order
            cout << " " << scc;
            printSectionName(section_of(scc), cout);
        }
        cout << endl;
    }

    // Joinable and non-joinable edges (in the reverse form: dst <- src)
    std::set<EdgeType_t> EJ, EN;
    DeriveEJ_EN_sets(mrp, tqo, aggregateComplexComp, EJ, EN, verboseLvl);



    std::set<EdgeType_t> Estar; // extended reachability,  src,first --> dst,decond
    std::set<EdgeType_t> EJ_fwd, EN_fwd; // src,first --> dst,decond
    for (auto edge : EJ)
        EJ_fwd.insert(make_pair(edge.second, edge.first));
    for (auto edge : EN)
        EN_fwd.insert(make_pair(edge.second, edge.first));
    // vertex v1 reaches vertex v2 ?
    auto may_reach = [&EJ_fwd, &EN_fwd, &Estar, &N_SCC](int v1, int v2) -> bool {
        std::vector<int> flags(N_SCC);
        std::fill(flags.begin(), flags.end(), 0);
        std::queue<int> visit_q;
        visit_q.push(v1);
        while (!visit_q.empty()) {
            int scc = visit_q.front();
            visit_q.pop();
            if (scc == v2)
                return true;  // v1 may reach v2

            auto lastEJ = EJ_fwd.upper_bound(make_pair(scc, sccindex_t(1 + N_SCC)));
            for (auto EJIt = EJ_fwd.lower_bound(make_pair(scc, -1)); EJIt != lastEJ; ++EJIt) {
                assert(EJIt->first == scc);
                if (flags[EJIt->second] == 0) {
                    flags[EJIt->second] = 1;
                    visit_q.push(EJIt->second);
                }
            }
            auto lastEN = EN_fwd.upper_bound(make_pair(scc, sccindex_t(1 + N_SCC)));
            for (auto ENIt = EN_fwd.lower_bound(make_pair(scc, -1)); ENIt != lastEN; ++ENIt) {
                assert(ENIt->first == scc);
                if (flags[ENIt->second] == 0) {
                    flags[ENIt->second] = 1;
                    visit_q.push(ENIt->second);
                }
            }
            auto lastEstar = Estar.upper_bound(make_pair(scc, sccindex_t(1 + N_SCC)));
            for (auto EstarIt = Estar.lower_bound(make_pair(scc, -1)); EstarIt != lastEstar; ++EstarIt) {
                assert(EstarIt->first == scc);
                if (flags[EstarIt->second] == 0) {
                    flags[EstarIt->second] = 1;
                    visit_q.push(EstarIt->second);
                }
            }
        }
        return false;
    };

    

    // Initialize table of x and y variables
    std::vector<std::vector<int>> mvars(N_SECT);
    for (int D=0; D<N_SECT; D++) {
        mvars[D].resize(N_SCC);
        std::fill(mvars[D].begin(), mvars[D].end(), 0);
    }
    for (int scc=0; scc<N_SCC; scc++)
        mvars[section_of(scc)][scc] = 1;

    auto apply_direct_constraints = [&]() -> bool {
        bool changed = false;

        for (int scc=N_SCC-1; scc>=0; scc--) { 
            int D_scc = section_of(scc);
            // cout << endl;

            for (int SN=-1; SN<N_SECT; SN++) {
                // Visit the section of scc first, then all the other sections
                int D;
                bool is_internal;
                if (SN == -1) { // Assign the x_scc value
                    D = D_scc;
                    is_internal = true;
                }
                else { // Assign the y^D_scc value
                    D = SN;
                    is_internal = false;
                    if (D == D_scc)
                        continue; // already visited.
                }
                int var = mvars[D][scc];

                // visit all the EJ and EN edges.
                auto lastEJ = EJ.upper_bound(make_pair(scc, sccindex_t(1 + tqo.numSCC)));
                for (auto EJIt = EJ.lower_bound(make_pair(scc, -1)); EJIt != lastEJ; ++EJIt) {
                    // v:second ---J-> v':first, first==scc
                    int prev_scc = EJIt->second;
                    verify(D_scc == section_of(EJIt->second));
                    verify(mvars[D][prev_scc] != -1 && EJIt->first == scc);
                    var = std::max(var, mvars[D][prev_scc]);
                }

                auto lastEN = EN.upper_bound(make_pair(scc, sccindex_t(1 + tqo.numSCC)));
                for (auto ENIt = EN.lower_bound(make_pair(scc, -1)); ENIt != lastEN; ++ENIt) {
                    // v:second ---N-> v':first, first==scc
                    int prev_scc = ENIt->second;
                    verify(mvars[D][prev_scc] != -1 && ENIt->first == scc);
                    var = std::max(var, mvars[D][prev_scc] + (is_internal ? 1 : 0));
                }

                if (var != mvars[D][scc])
                    changed = true;
                mvars[D][scc] = var;
            }
        }
        return changed;
    };


    apply_direct_constraints();
    while (true) {
        for (int D=0; D<N_SECT; D++) {
            for (int v1=N_SCC-1; v1>=0; v1--) {
                if (D != section_of(v1))
                    continue;
                // int D = section_of(v1);
                for (int v2=N_SCC-1; v2>=0; v2--) {
                    if (D != section_of(v2))
                        continue;

                    if (mvars[D][v1] <= mvars[D][v2]) {
                        if (may_reach(v1, v2) || may_reach(v2, v1))
                            continue;

                        cout << "APPLY RULE 8 FOR v1="<<v1<<", v2="<<v2<<endl;
                        bool changed = false;
                        for (int DD=0; DD<N_SECT; DD++) {
                            if (D == DD)
                                continue;

                            int y_DD_v1 = mvars[DD][v1];
                            int y_DD_v2 = mvars[DD][v2];
                            mvars[DD][v1] = std::max(mvars[DD][v1], mvars[DD][v2]);
                            // mvars[DD][v2] = std::max(mvars[DD][v1], mvars[DD][v2]);
                            changed = changed || (y_DD_v1 < mvars[DD][v1]) || (y_DD_v2 < mvars[DD][v2]);
                        }
                        while (changed && apply_direct_constraints()) { }
                    }
                }
            }
        }
        // Verify that the solution is acyclic
        compId_t numComps(0);
        std::vector<compId_t> compIds(N_SCC);
        std::vector<int> compSection;
        std::set<std::pair<compId_t, compId_t>> EC;
        std::vector<compId_t> topoOrdComp;

        std::vector<int> x_vars;
        SolutionFromXYvars(mvars, tqo, x_vars);

        bool is_acyclic = DeriveAcyclicPartitionsFromXvars(mrp, x_vars, tqo, EN, EJ, EC,
                                                           numComps, compIds, &compSection, 
                                                           topoOrdComp, VL_NONE);
        if (is_acyclic)
            break;
    }


    // //  visit SCCs in topological order, from initial SCCs to bottom SCCs
    // bool changed = true;
    // while (changed) {
    //     cout << "NEW ITERATION OF ILP COMPONENT METHOD."<<endl;
    //     changed = false;
    //     for (int scc=N_SCC-1; scc>=0; scc--) { 
    //         int D_scc = section_of(scc);
    //         // cout << endl;

    //         for (int SN=-1; SN<N_SECT; SN++) {
    //             // Visit the section of scc first, then all the other sections
    //             int D;
    //             bool is_internal;
    //             if (SN == -1) { // Assign the x_scc value
    //                 D = D_scc;
    //                 is_internal = true;
    //             }
    //             else { // Assign the y^D_scc value
    //                 D = SN;
    //                 is_internal = false;
    //                 if (D == D_scc)
    //                     continue; // already visited.
    //             }
    //             int var = mvars[D][scc];

    //             if (!is_internal) { // var is a y^D variable
    //                 // Visit all the previous SCCs
    //                 for (int prev_scc=N_SCC-1; prev_scc>=0; prev_scc--) {
    //                     if (section_of(prev_scc) == D_scc) {
    //                         // x[prev_scc] <= x[scc]  imply  y[D][prev_scc] <= y[D][scc]
    //                         verify(mvars[D_scc][prev_scc] != -1);
    //                         if (mvars[D_scc][prev_scc] == mvars[D_scc][scc]) {
    //                             // if (may_reach(prev_scc, scc) || may_reach(scc, prev_scc))
    //                             //     continue;

    //                             if (var < mvars[D][prev_scc]) {
    //                                 cout << "x[scc"<<prev_scc<<"] <= x[scc"<<scc<<"]  "
    //                                         "->  y[D"<<D<<"][scc"<<prev_scc<<"] <= y[D"<<D<<"][scc"<<scc<<"]  :  "
    //                                      << var << " <= " << mvars[D_scc][scc] << "  ->  "
    //                                      << var << " <= " << mvars[D][scc] << endl;
    //                             }
    //                             var = std::max(var, mvars[D][prev_scc]);
    //                         }
    //                     }
    //                 }
    //             }

    //             // visit all the EJ and EN edges.
    //             auto lastEJ = EJ.upper_bound(make_pair(scc, sccindex_t(1 + tqo.numSCC)));
    //             for (auto EJIt = EJ.lower_bound(make_pair(scc, -1)); EJIt != lastEJ; ++EJIt) {
    //                 // v:second ---J-> v':first, first==scc
    //                 int prev_scc = EJIt->second;
    //                 verify(D_scc == section_of(EJIt->second));
    //                 verify(mvars[D][prev_scc] != -1 && EJIt->first == scc);
    //                 var = std::max(var, mvars[D][prev_scc]);
    //             }

    //             auto lastEN = EN.upper_bound(make_pair(scc, sccindex_t(1 + tqo.numSCC)));
    //             for (auto ENIt = EN.lower_bound(make_pair(scc, -1)); ENIt != lastEN; ++ENIt) {
    //                 // v:second ---N-> v':first, first==scc
    //                 int prev_scc = ENIt->second;
    //                 verify(mvars[D][prev_scc] != -1 && ENIt->first == scc);
    //                 var = std::max(var, mvars[D][prev_scc] + (is_internal ? 1 : 0));
    //             }

    //             if (var != mvars[D][scc])
    //                 changed = true;
    //             mvars[D][scc] = var;

    //             // if (is_internal)
    //             //     cout << "x[scc" << scc << "] = " << var << endl;
    //             // else
    //             //     cout << "y[D" << D << "][scc" << scc << "] = " << var << endl;

    //         }
    //     }

    //     // // Verify that the solution is acyclic
    //     // compId_t numComps(0);
    //     // std::vector<compId_t> compIds(N_SCC);
    //     // std::vector<int> compSection;
    //     // std::set<std::pair<compId_t, compId_t>> EC;
    //     // std::vector<compId_t> topoOrdComp;

    //     // bool is_acyclic = DeriveAcyclicPartitionsFromXYvars(mrp, mvars, tqo, EN, EJ, EC,
    //     //                                                     numComps, compIds, &compSection, 
    //     //                                                     topoOrdComp, VL_NONE);
    //     // if (is_acyclic)
    //     //     break;
    // }

    // Now that we have the mvars[] matrix complete the processing.
    std::vector<int> x_vars;
    SolutionFromXYvars(mvars, tqo, x_vars);
    ProcessComponentListFromXvars(mrp, stKinds, compProc, x_vars, tqo, 
                                  EN, EJ, true, verboseLvl);
}

//=============================================================================

void ProcessComponentListGreedy(const MRP &mrp, const MRPStateKinds &stKinds,
                                ComponentProcessor &compProc,
                                bool aggregateComplexComp, 
                                bool randomizeSccSelection,
                                size_t randomSeed,
                                VerboseLevel verboseLvl) 
{
    std::default_random_engine rand_gen(randomSeed);

    TopologicalOrderQueue tqo;
    PrepareTopologicalOrderQueue(mrp, stKinds, tqo, verboseLvl);
    const int N_SCC = tqo.sccInfo.size();

    const int N_SECT = mrp.Gen.size() + 2;
    // section of a vertex
    auto section_of = [&](int scc_v) -> int /* section */ {
        assert(scc_v >= 0 && scc_v < int(tqo.sccInfo.size()));
        return statesubset_to_section(tqo.sccInfo[scc_v].subset);
    };

    std::vector<int> comps_per_section(N_SECT);
    std::fill(comps_per_section.begin(), comps_per_section.end(), 1);

    std::vector<int> x_vars(N_SCC);
    std::fill(x_vars.begin(), x_vars.end(), -1);

    // Process the SCC list in topological order
    while (!tqo.FreeQueue.empty()) {
        set<TQEntry>::iterator topSccIt = tqo.FreeQueue.begin();

        // Choose randomly the next SCC, ignoring the preference over the general partition
        if (randomizeSccSelection) {
            int z = rand_gen() % (tqo.FreeQueue.size());
            // cout << z << "/" << tqo.FreeQueue.size() << endl;
            // z = tqo.FreeQueue.size()-1;
            while (--z > 0) {
                ++topSccIt;
                verify(topSccIt != tqo.FreeQueue.end());
            }
        }

        statesubset_t topSubset = std::get<TQE_STATE_SUBSET>(*topSccIt);
        sccindex_t topScc = std::get<TQE_SCC_INDEX>(*topSccIt);
        int topD = section_of(topScc);
        PopFreeSCC(mrp, tqo, topSccIt);

        // First step: identify the next component. A component is a group of
        // SCCs taken from the FreeQueue, such that it is convenient to treat
        // them in a single step. The greedy logic of how components are made
        // depends on whether the resulting component will be simple or complex

        // vector<int> ProcessedSCCs;
        set<int> SetOfProcessedSCCs;
        // ProcessedSCCs.push_back(topScc);
        // A fast look-up of the processed SCCs is needed
        SetOfProcessedSCCs.insert(topScc);
        bool inserted = true;

        while (!tqo.FreeQueue.empty() && inserted) {
            set<TQEntry>::iterator nextSccIt, lastSccIt;
            nextSccIt = tqo.FreeQueue.lower_bound(make_tuple(topSubset, false, -1));
            lastSccIt = tqo.FreeQueue.upper_bound(make_tuple(topSubset, true, sccindex_t(1 + tqo.numSCC)));
            inserted = false;

            for (; nextSccIt != lastSccIt; ++nextSccIt) {
                if (topSubset == COMPLEX_COMPONENT_SUBSET) {
                    // This component will be a COMPLEX component
                    if (!aggregateComplexComp)
                        break; // Complex component aggregation is disabled

                    // Only join complex components together
                    if (tqo.sccInfo[std::get<TQE_SCC_INDEX>(*nextSccIt)].isSimple)
                        continue;
                }
                else {
                    // This component will be a SIMPLE component

                    bool noOtherSCCInQueue = (nextSccIt == tqo.FreeQueue.end());
                    bool hasPrevComplexEdge = true, notInTheSameSubset = true;
                    if (!noOtherSCCInQueue) {
                        hasPrevComplexEdge = (std::get<TQ_HAS_PREV_COMPLEX_EDGE>(*nextSccIt));
                        if (std::get<TQE_STATE_SUBSET>(*nextSccIt) == EXP_SUBSET)
                            hasPrevComplexEdge = false;
                        notInTheSameSubset = (std::get<TQE_STATE_SUBSET>(*nextSccIt) != topSubset);
                    }
                    if (std::get<TQE_STATE_SUBSET>(*nextSccIt) != EXP_SUBSET) {
                        // Verify that complex edges came from the same ProcessedSCCs group
                        sccindex_t thisScc = std::get<TQE_SCC_INDEX>(*nextSccIt);
                        SCCEdgeSet_t::const_iterator nextIngoingSccIt, lastIngoingSccIt;
                        nextIngoingSccIt = tqo.InComplexEdges.lower_bound(make_pair(thisScc, -1));
                        lastIngoingSccIt = tqo.InComplexEdges.upper_bound(make_pair(thisScc, sccindex_t(1 + tqo.numSCC)));
                        bool hasComplexEdgeFromThisComponentGroup = false;
                        for (; nextIngoingSccIt != lastIngoingSccIt; ++nextIngoingSccIt) {
                            sccindex_t cpEdgFrom = nextIngoingSccIt->second;
                            assert(nextIngoingSccIt->first == thisScc);
                            if (cpEdgFrom == thisScc /* complex self-loop */ ||
                                    // exists(ProcessedSCCs.begin(), ProcessedSCCs.end(), cpEdgFrom)) 
                                SetOfProcessedSCCs.count(cpEdgFrom) > 0)
                            {
                                hasComplexEdgeFromThisComponentGroup = true;
                                break;
                            }
                        }
                        if (!hasComplexEdgeFromThisComponentGroup)
                            hasPrevComplexEdge = false;
                    }
                    if (noOtherSCCInQueue || hasPrevComplexEdge || notInTheSameSubset)
                        continue;
                }

                // Add std::get<TQE_SCC_INDEX>(*nextSccIt) into the component group
                // ProcessedSCCs.push_back(std::get<TQE_SCC_INDEX>(*nextSccIt));
                SetOfProcessedSCCs.insert(std::get<TQE_SCC_INDEX>(*nextSccIt));
                PopFreeSCC(mrp, tqo, nextSccIt);
                inserted = true;
                break;
            }
        }

        int x_num = comps_per_section[topD]++;
        for (int scc : SetOfProcessedSCCs)
            x_vars[scc] = x_num;
    }  

    std::set<EdgeType_t> EJ, EN;
    DeriveEJ_EN_sets(mrp, tqo, aggregateComplexComp, EJ, EN, verboseLvl);

    ProcessComponentListFromXvars(mrp, stKinds, compProc, x_vars, tqo, 
                                  EN, EJ, false, verboseLvl);      
}

//=============================================================================

void ProcessComponentListGreedy_old(const MRP &mrp, const MRPStateKinds &stKinds,
                                ComponentProcessor &compProc,
                                bool aggregateComplexComp, 
                                bool randomizeSccSelection,
                                size_t randomSeed,
                                VerboseLevel verboseLvl) 
{
    const marking_t N_MRP = mrp.N();
    std::default_random_engine rand_gen(randomSeed);

    TopologicalOrderQueue tqo;
    PrepareTopologicalOrderQueue(mrp, stKinds, tqo, verboseLvl);

    // Process the SCC list in topological order
    vector<bool> intCompStates(N_MRP), extCompStates(N_MRP);
    size_t numProcExpComp = 0, numProcGenComp = 0, numProcComplexComp = 0;
    while (!tqo.FreeQueue.empty()) {
        set<TQEntry>::iterator topSccIt = tqo.FreeQueue.begin();

        // Choose randomly the next SCC, ignoring the preference over the general partition
        if (randomizeSccSelection) {
            int z = rand_gen() % (tqo.FreeQueue.size());
            // cout << z << "/" << tqo.FreeQueue.size() << endl;
            // z = tqo.FreeQueue.size()-1;
            while (--z > 0) {
                ++topSccIt;
                verify(topSccIt != tqo.FreeQueue.end());
            }
        }

        statesubset_t topSubset = std::get<TQE_STATE_SUBSET>(*topSccIt);
        sccindex_t topScc = std::get<TQE_SCC_INDEX>(*topSccIt);
        PopFreeSCC(mrp, tqo, topSccIt);

        // First step: identify the next component. A component is a group of
        // SCCs taken from the FreeQueue, such that it is convenient to treat
        // them in a single step. The greedy logic of how components are made
        // depends on whether the resulting component will be simple or complex

        vector<int> ProcessedSCCs;
        ProcessedSCCs.push_back(topScc);
        bool inserted = true;

        while (!tqo.FreeQueue.empty() && inserted) {
            set<TQEntry>::iterator nextSccIt, lastSccIt;
            nextSccIt = tqo.FreeQueue.lower_bound(make_tuple(topSubset, false, -1));
            lastSccIt = tqo.FreeQueue.upper_bound(make_tuple(topSubset, true, sccindex_t(1 + tqo.numSCC)));
            inserted = false;

            for (; nextSccIt != lastSccIt; ++nextSccIt) {
                if (topSubset == COMPLEX_COMPONENT_SUBSET) {
                    // This component will be a COMPLEX component
                    if (!aggregateComplexComp)
                        break; // Complex component aggregation is disabled

                    // Only join complex components together
                    if (tqo.sccInfo[std::get<TQE_SCC_INDEX>(*nextSccIt)].isSimple)
                        continue;
                }
                else {
                    // This component will be a SIMPLE component

                    bool noOtherSCCInQueue = (nextSccIt == tqo.FreeQueue.end());
                    bool hasPrevComplexEdge = true, notInTheSameSubset = true;
                    if (!noOtherSCCInQueue) {
                        hasPrevComplexEdge = (std::get<TQ_HAS_PREV_COMPLEX_EDGE>(*nextSccIt));
                        if (std::get<TQE_STATE_SUBSET>(*nextSccIt) == EXP_SUBSET)
                            hasPrevComplexEdge = false;
                        notInTheSameSubset = (std::get<TQE_STATE_SUBSET>(*nextSccIt) != topSubset);
                    }
                    if (std::get<TQE_STATE_SUBSET>(*nextSccIt) != EXP_SUBSET) {
                        // Verify that complex edges came from the same ProcessedSCCs group
                        sccindex_t thisScc = std::get<TQE_SCC_INDEX>(*nextSccIt);
                        SCCEdgeSet_t::const_iterator nextIngoingSccIt, lastIngoingSccIt;
                        nextIngoingSccIt = tqo.InComplexEdges.lower_bound(make_pair(thisScc, -1));
                        lastIngoingSccIt = tqo.InComplexEdges.upper_bound(make_pair(thisScc, sccindex_t(1 + tqo.numSCC)));
                        bool hasComplexEdgeFromThisComponentGroup = false;
                        for (; nextIngoingSccIt != lastIngoingSccIt; ++nextIngoingSccIt) {
                            sccindex_t cpEdgFrom = nextIngoingSccIt->second;
                            assert(nextIngoingSccIt->first == thisScc);
                            if (cpEdgFrom == thisScc /* complex self-loop */ ||
                                    exists(ProcessedSCCs.begin(), ProcessedSCCs.end(), cpEdgFrom)) {
                                hasComplexEdgeFromThisComponentGroup = true;
                                break;
                            }
                        }
                        if (!hasComplexEdgeFromThisComponentGroup)
                            hasPrevComplexEdge = false;
                    }
                    if (noOtherSCCInQueue || hasPrevComplexEdge || notInTheSameSubset)
                        continue;
                }

                // Add std::get<TQE_SCC_INDEX>(*nextSccIt) into the component group
                ProcessedSCCs.push_back(std::get<TQE_SCC_INDEX>(*nextSccIt));
                PopFreeSCC(mrp, tqo, nextSccIt);
                inserted = true;
                break;
            }
        }

        // Second step:  extend the selected states with the transitive closure
        //  w.r.t. exponential transitions and frontier absorbing states.

        fill(intCompStates.begin(), intCompStates.end(), false);
        fill(extCompStates.begin(), extCompStates.end(), false);
        for (size_t i = 0; i < ProcessedSCCs.size(); i++)
            AddAugmentedSCCSubset(mrp, tqo.SCCStateSubsets, ProcessedSCCs[i],
                                  intCompStates, extCompStates);
        const size_t numStates = std::count(intCompStates.begin(),
                                            intCompStates.end(), true);
        const size_t numAugmentedStates = std::count(extCompStates.begin(),
                                          extCompStates.end(), true);

        if (verboseLvl >= VL_BASIC) {
            cout << "PROCESSING " << console::cyan_fgnd() ;
            if (topSubset == EXP_SUBSET)
                cout << "(EXP)";
            else if (topSubset == COMPLEX_COMPONENT_SUBSET)
                cout << "(MIXED)";
            else
                cout << "(GEN: " << topSubset << ")";
            // cout << (topSubset == COMPLEX_COMPONENT_SUBSET ? "COMPLEX" : "SIMPLE");
            cout << console::default_disp();
            cout << " COMPONENT WITH " << ProcessedSCCs.size() << " SCC";
            cout << (ProcessedSCCs.size() == 1 ? "" : "s");
            cout << " (States=" << numStates;
            if (numAugmentedStates > 0)
                cout << ", augmented to " << (numStates + numAugmentedStates);
            cout << ")." << endl;
        }
        if (verboseLvl >= VL_VERBOSE) {
            cout << "    " << ProcessedSCCs.size() << " SCCs = {";
            for (size_t i = 0; i < ProcessedSCCs.size(); i++) {
                cout << (i == 0 ? " " : ", ") << "z" << ProcessedSCCs[i];
            }
            cout << " }" << endl;
            //cout << " FreeQueue=" << print_vec(tqo.FreeQueue) << endl;
        }

        // Third step: process the component
        if (compProc.ProcessNextComponent(tqo, topScc, topSubset, intCompStates, extCompStates,
                                          numAugmentedStates, verboseLvl)) {
            if (topSubset == EXP_SUBSET)
                numProcExpComp++;
            else if (topSubset == COMPLEX_COMPONENT_SUBSET)
                numProcComplexComp++;
            else
                numProcGenComp++;
        }
    }

    compProc.EndProcessing();

    if (verboseLvl >= VL_BASIC) {
        cout << "\nPROCESSED " << numProcExpComp << " EXP, " << numProcGenComp << " GEN AND ";
        cout << numProcComplexComp << " MIXED COMPONENTS." << endl;
    }
}

//=============================================================================

void ProcessComponentList(const MRP &mrp, const MRPStateKinds &stKinds,
                          ComponentProcessor &compProc,
                          bool aggregateComplexComp, 
                          ComponentMethod compMethod,
                          bool randomizeSccSelection,
                          size_t randomSeed,
                          VerboseLevel verboseLvl) 
{
    switch (compMethod) {
        case PCM_GREEDY:
            ProcessComponentListGreedy(mrp, stKinds, compProc, 
                                       aggregateComplexComp, randomizeSccSelection, 
                                       randomSeed, verboseLvl);
            break;

        case PCM_ILP:
            ProcessComponentList_ILP(mrp, stKinds, compProc, 
                                     aggregateComplexComp, verboseLvl);
            break;

        case PCM_POLYNOMIAL_ILP:
            ProcessComponentListOptimal(mrp, stKinds, compProc, 
                                        aggregateComplexComp, verboseLvl);
            break;
    }
}

//=============================================================================

void SteadyStateMrpBySccDecomp(const MRP &mrp, const MRPStateKinds &stKinds,
                               PetriNetSolution &sol,
                               const SolverParams &spar,
                               VerboseLevel verboseLvl) {
    SteadyStateMRPComponentProcessor ssmrpcp(mrp, spar, sol);
    ProcessComponentList(mrp, stKinds, ssmrpcp, spar.aggregateComplexComp, 
                         spar.compMethod, spar.randomizeSccSelection, 
                         spar.rndSeed, verboseLvl);
}

//=============================================================================

void SteadyStateMrpBySccDecompBackward(const MRP &mrp, const MRPStateKinds &stKinds,
                                       const ublas::vector<double> &rho,
                                       ublas::vector<double> &xi,
                                       const SolverParams &spar,
                                       VerboseLevel verboseLvl) {
    const marking_t N_MRP = mrp.N();

    SimpleComponentProcessor compProc;
    ProcessComponentList(mrp, stKinds, compProc, spar.aggregateComplexComp, 
                         spar.compMethod, spar.randomizeSccSelection, 
                         spar.rndSeed, VL_NONE);
    xi = rho;

    std::vector<ComponentDesc>::const_reverse_iterator cdit;
    size_t numProcExpComp = 0, numProcGenComp = 0, numProcComplexComp = 0;
    for (cdit = compProc.comps.rbegin(); cdit != compProc.comps.rend(); ++cdit) {
        const ComponentDesc &cd = *cdit;
        if (verboseLvl >= VL_BASIC)
            cout << print_compdesc(cd, "STATE", verboseLvl) << endl;

        // First step: build up the sub-MRP
        MRP subMrp;
        MarkingSubset_t mark2submark;
        ExtractSubMRP2(mrp, cd.intCompStates, cd.extCompStates,
                       mark2submark, xi, subMrp, VL_NONE);

        // cout << "xi = " << print_vec(xi) << endl;
        // cout << "subMrp.pi0 = " << print_vec(subMrp.pi0) << endl;
        if (verboseLvl >= VL_VERBOSE) {
            PrintMRP(subMrp);
            cout << endl;
        }

        // Second step: solve the subordinated system
        performance_timer timer;
        PetriNetSolution outSol;
        SolveComponent_Backward(subMrp, cd.topSubset, subMrp.pi0, outSol, spar, verboseLvl);
        if (cd.topSubset == EXP_SUBSET)
            numProcExpComp++;
        else if (cd.topSubset == COMPLEX_COMPONENT_SUBSET)
            numProcComplexComp++;
        else
            numProcGenComp++;

        // Distribute the output probability back into pi
        for (marking_t m(0); m < N_MRP; ++m) {
            if (mark2submark[m] != NOT_SUBMRP_MARKING && cd.intCompStates[m])
                xi[m] = outSol.stateProbs[ mark2submark[m] ];
        }
        if (verboseLvl >= VL_BASIC)
            cout << "COMPONENT SOLUTION TIME: " << timer << endl;


        if (verboseLvl >= VL_VERBOSE) {
            cout << "BACKWARD MRP PROBABILITY:\n  " << xi << endl;
            cout << "\n\n\n";
        }
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "\nPROCESSED " << numProcExpComp << " EXP, " << numProcGenComp << " GEN AND ";
        cout << numProcComplexComp << " MIXED COMPONENTS." << endl;
    }
}

//=============================================================================







