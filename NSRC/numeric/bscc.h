//-----------------------------------------------------------------------------
/// \file bscc.h
/// Implementation of the Tarjan algorithm for (B)SCC analysis.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __NUMERIC_BSCC_H__
#define __NUMERIC_BSCC_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct SccIndex;
typedef private_integer<int, SccIndex>  sccindex_t;
#else
typedef int  sccindex_t;
#endif

const sccindex_t TRANSIENT_COMPONENT = sccindex_t(-1);

//-----------------------------------------------------------------------------
// Auxiliary structures needed by the TarjanIterative method.
//-----------------------------------------------------------------------------

/// \internal
struct scc_node {
    inline scc_node() { index = lowlink = -1; on_stack = false; }
    int index, lowlink;	// Used by Tarjan algorithm to track SCCs
    bool on_stack;
};
typedef struct scc_node  scc_node;

/// \internal
enum DfsVisitState { DFS_ENTER, DFS_CONTINUE };

/// Depth-first search stack structure
/// \internal
struct dfs_visit_node {
    inline dfs_visit_node(size_t _v, DfsVisitState vs);

    size_t v;  ///< Visited node in the DFS stack
    size_t j;  ///< Number of successors of v already visited
    size_t w;  ///< Last successor visited
    DfsVisitState visitState;  ///< Start visiting or resuming?
};
typedef struct dfs_visit_node  dfs_visit_node;
inline dfs_visit_node::dfs_visit_node(size_t _v, DfsVisitState vs)
/**/ : v(_v), j(0), w(size_t(-1)), visitState(vs)  { }


//-----------------------------------------------------------------------------
/// Iterative version of Tarjan algorithm for finding the SCC in a sparse
/// matrix, analyzed as an adjacency list.
///	  \param mat	The matrix graph.
///   \param SCC	On return contains the scc numbers for every node in mat
///   \return  Returns the number of SCCs in the matrix graph.
//-----------------------------------------------------------------------------
template<class Matrix>
sccindex_t TarjanIterative(const Matrix &mat, std::vector<sccindex_t> &SCC) {
    const size_t N = mat.size1();
    assert(mat.size2() == N);
    int IndexCount = 0;
    sccindex_t SccCount(0);
    std::vector<scc_node> Nodes(N);
    std::vector<int> NodeStack;
    std::vector<dfs_visit_node> DepthFirstStack;
    NodeStack.reserve(N);
    DepthFirstStack.reserve(N);
    SCC.resize(N);
    std::fill(SCC.begin(), SCC.end(), -1);

    for (size_t startNode = 0; startNode < N; startNode++) {
        if (Nodes[startNode].index >= 0)
            continue; // already visited

        dfs_visit_node start(startNode, DFS_ENTER);
        DepthFirstStack.push_back(start);

        // Start the Depth-First visit from the top-of-stack node
        while (!DepthFirstStack.empty()) {
            dfs_visit_node &cn = DepthFirstStack.back();

            switch (cn.visitState) {
            case DFS_ENTER:
                // Start visiting node cn.v, pushing it onto the node stack
                // Set depth index of node v and push it on the stack
                Nodes[cn.v].index = Nodes[cn.v].lowlink = IndexCount;
                IndexCount += 1;
                NodeStack.push_back(cn.v);
                Nodes[cn.v].on_stack = true;
                cn.visitState = DFS_CONTINUE;
                break;

            case DFS_CONTINUE:
                // Resume the previously stopped visit at node cn.w
                Nodes[cn.v].lowlink = std::min(Nodes[cn.v].lowlink,
                                               Nodes[cn.w].lowlink);
                break;
            }

            // Visit all the successors of node cn.v , from where we
            // stopped (after the first cn.j successors).
            typedef typename ublas::matrix_row<const Matrix> row_t;
            typedef typename row_t::const_iterator row_iter_t;

            row_t row(mat, cn.v);
            row_iter_t iter = row.find(cn.j); // Resume at the j-th non-zero edge

            // Visit the successors of v
            bool visit_other_nodes_before = false;
            while (iter != row.end()) {
                cn.w = iter.index();
                double edge_value = *iter;
                iter++;
                cn.j++;
                if (cn.v == cn.w)
                    continue;
                // Ignore edges mat(cn.v, cn.w) == 0
                if (edge_value == 0.0) // FIXME: abs(edge_value) <= threshold
                    continue;

                // Visit w if it has not been visited yet
                if (Nodes[cn.w].index < 0) {
                    dfs_visit_node visit_w(cn.w, DFS_ENTER);
                    DepthFirstStack.push_back(visit_w);
                    visit_other_nodes_before = true;
                    break;
                }
                else if (Nodes[cn.w].on_stack) { // Is w onto the stack?
                    Nodes[cn.v].lowlink = std::min(Nodes[cn.v].lowlink,
                                                   Nodes[cn.w].index);
                }
            }
            // Check if we must visit other nodes before v successors
            if (visit_other_nodes_before)
                continue;

            // Test if v is an SCC root
            if (Nodes[cn.v].index == Nodes[cn.v].lowlink) {
                //cout << "SCC " << SccCount << ":  ";
                size_t w;
                do {
                    w = NodeStack.back();
                    NodeStack.pop_back();
                    Nodes[w].on_stack = false;
                    SCC[w] = SccCount;
                    //cout << w << " ";
                }
                while (w != cn.v);
                //cout << endl;
                SccCount++;
            }

            // v has been visited completely
            DepthFirstStack.pop_back();
        }

        assert(DepthFirstStack.empty() && NodeStack.empty());
    }

    return SccCount;
}


//-----------------------------------------------------------------------------

/// BSCC analysis of a Markov chain
class BSCC : boost::noncopyable {
public:
    /// Bscc index of every state. The set with value TRANSIENT_COMPONENT (-1)
    /// is the transient set.
    std::vector<sccindex_t> indices;
    /// Total number of Bottom SCC, or 0 if the graph is a single strongly
    /// connected component.
    sccindex_t numBscc = (sccindex_t) - 1;
};

//-----------------------------------------------------------------------------
/// Find the Bottom Strongly Connected Components of a sparse matrix graph.
///   \param mat	The matrix graph (as an adjacency list)
///   \param bscc   On return contains the BSCC structure initialized.
//-----------------------------------------------------------------------------
template<class Matrix>
void BottomStronglyConnectedComponents(const Matrix &mat, BSCC &bscc) {
    const size_t N = mat.size1();
    assert(mat.size2() == N);

    // Get the SCCs
    sccindex_t SccCount = TarjanIterative(mat, bscc.indices);

    if (SccCount == sccindex_t(1)) {
        bscc.numBscc = 0;  // Only one large BSCC, with no initial transient.
        return;
    }

    // Find bottom SCCs
    vector<bool> IsBSCC(SccCount, true);
    typename Matrix::const_iterator1 rowIt;
    typename Matrix::const_iterator2 colIt;
    for (rowIt = mat.begin1(); rowIt != mat.end1(); ++rowIt) {
        size_t i = rowIt.index1();
        for (colIt = rowIt.begin(); colIt != rowIt.end(); ++colIt) {
            size_t j = colIt.index2();
            if (bscc.indices[i] != bscc.indices[j])
                IsBSCC[ bscc.indices[i] ] = false;
        }
    }

    // Remap SCC indexes into BSCC indexes
    sccindex_t BsccCount = 0;
    vector<sccindex_t> BsccRemap(SccCount, -1);
    for (int i = 0; i < SccCount; i++)
        if (IsBSCC[i])
            BsccRemap[i] = BsccCount++;
    for (size_t i = 0; i < N; i++)
        bscc.indices[i] = BsccRemap[ bscc.indices[i] ];

    bscc.numBscc = BsccCount;
}

//-----------------------------------------------------------------------------
// This class builds up the BaseSubsetList of an SCC decomposition
struct SccIndexTo0BasedIndexWithTransients : boost::noncopyable {
    inline numerical::subsetindex_t
    externalSubsetTo0BasedIndex(const sccindex_t sccIndex) const {
        return numerical::subsetindex_t(int(sccIndex) + 1);
    }
};

//-----------------------------------------------------------------------------
/* @}  Numerical documentiation group. */
#endif  // __NUMERIC_BSCC_H__
