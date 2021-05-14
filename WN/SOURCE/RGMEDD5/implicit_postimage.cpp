
#include <typeinfo> // for "bad_cast" exception
#include <set>
#include <map>

#ifdef HAS_GMP_LIB
#include <gmpxx.h>
#endif

// Include meddly only after having included <gmpxx.h>
#include <meddly.h>
#include <meddly_expert.h>

#undef MEDDLY_DCASSERT
#define MEDDLY_DCASSERT(x)  assert(x)

#include "implicit_postimage.h"

MEDDLY::postimage_impl_opname* MEDDLY::IMPLICIT_POSTIMAGE_OPNAME = nullptr;

void MEDDLY::initialize_implicit_postimage_opname() {
    IMPLICIT_POSTIMAGE_OPNAME = initImplPostImage();
}

void MEDDLY::cleanup_implicit_postimage_opname() {
    delete IMPLICIT_POSTIMAGE_OPNAME;
    IMPLICIT_POSTIMAGE_OPNAME = nullptr;
}

namespace MEDDLY {
    class postimage_impl_opname;
    class postimage_impl_unary_opname;

    class postimage_impl_outer_op; // outer recursion operation (all firings)
    class postimage_impl_inner_op; // inner recursion operation (single event firings)
}; // namespace MEDDLY

MEDDLY::postimage_impl_opname::postimage_impl_opname(const char *n)
    : specialized_opname(n)
{
}

MEDDLY::postimage_impl_opname::~postimage_impl_opname()
{
}

// ******************************************************************
// *                                                                *
// *               postimage_impl_unary_opname  class               *
// *                                                                *
// ******************************************************************

/** Simple class to keep compute table happy. */
class MEDDLY::postimage_impl_unary_opname : public unary_opname {
    static postimage_impl_unary_opname* instance;
public:
    postimage_impl_unary_opname();

    static const postimage_impl_unary_opname* getInstance();
};

MEDDLY::postimage_impl_unary_opname* MEDDLY::postimage_impl_unary_opname::instance = 0;

MEDDLY::postimage_impl_unary_opname::postimage_impl_unary_opname()
    : unary_opname("PostImage_impl_by_events")
{
}

const MEDDLY::postimage_impl_unary_opname* MEDDLY::postimage_impl_unary_opname::getInstance()
{
    if (0 == instance)
        instance = new postimage_impl_unary_opname;
    return instance;
}

// ******************************************************************
// *                                                                *
// *             postimage_impl_outer_op  class                     *
// *                                                                *
// ******************************************************************

// This class is the cache for the all-event-firings relation: mdd -> mdd'
// in the outer step of the recursion (compute_postimage)
class MEDDLY::postimage_impl_outer_op : public unary_operation {
    postimage_impl_inner_op* parent;
public:
    postimage_impl_outer_op(postimage_impl_inner_op* p,
        expert_forest* argF, expert_forest* resF);
    virtual ~postimage_impl_outer_op();

    node_handle compute_postimage(node_handle mdd);
    node_handle compute_postimage(node_handle mdd, int level);

protected:
    inline compute_table::entry_key*
    findPostImageResult(node_handle a, int level, node_handle& b)
    {
        compute_table::entry_key* CTsrch = CT0->useEntryKey(etype[0], 0);
        MEDDLY_DCASSERT(CTsrch);
        CTsrch->writeN(a);
        if (argF->isFullyReduced())
            CTsrch->writeI(level);
        CT0->find(CTsrch, CTresult[0]);
        if (!CTresult[0])
            return CTsrch;
        b = resF->linkNode(CTresult[0].readN());
        CT0->recycle(CTsrch);
        return 0;
    }

    inline void recycleCTKey(compute_table::entry_key* CTsrch)
    {
        CT0->recycle(CTsrch);
    }

    inline node_handle savePostImageResult(compute_table::entry_key* Key,
                                              node_handle a, node_handle b)
    {
        CTresult[0].reset();
        CTresult[0].writeN(b);
        CT0->addEntry(Key, CTresult[0]);
        return b;
    }
};

// ******************************************************************
// *                                                                *
// *            postimage_impl_inner_op  class                      *
// *                                                                *
// ******************************************************************

// This class is the cache for the single-event relation:
//     mdd * event_node -> mdd'
// in the inner step of the recursion (fireEvent)
class MEDDLY::postimage_impl_inner_op : public specialized_operation {
public:
    postimage_impl_inner_op(const postimage_impl_opname* opcode,
        satimpl_opname::implicit_relation* rel);
    virtual ~postimage_impl_inner_op();

    // operation starting point
    virtual void compute(const dd_edge& a, dd_edge& c);
    void fireTopEventsAtLevel(unpacked_node& mdd, unpacked_node& nbout);
    node_handle fireEvent(node_handle mdd, rel_node_handle mxd);

protected:
    inline compute_table::entry_key*
    findResult(node_handle a, rel_node_handle b, node_handle& c)
    {
        compute_table::entry_key* CTsrch = CT0->useEntryKey(etype[0], 0);
        MEDDLY_DCASSERT(CTsrch);
        CTsrch->writeN(a);
        CTsrch->writeL(b);
        CT0->find(CTsrch, CTresult[0]);
        if (!CTresult[0])
            return CTsrch;
        c = resF->linkNode(CTresult[0].readN());
        CT0->recycle(CTsrch);
        return 0;
    }

    inline void recycleCTKey(compute_table::entry_key* CTsrch)
    {
        CT0->recycle(CTsrch);
    }

    inline node_handle saveResult(compute_table::entry_key* Key,
        node_handle a, rel_node_handle b, node_handle c)
    {
        CTresult[0].reset();
        CTresult[0].writeN(c);
        CT0->addEntry(Key, CTresult[0]);
        return c;
    }

public:
    binary_operation* mddUnion;
    // binary_operation *mxdIntersection;
    // binary_operation *mxdDifference;

    satimpl_opname::implicit_relation* rel;

    postimage_impl_outer_op *outer_op;

    expert_forest* arg1F;
    // expert_forest *arg2F;
    expert_forest* resF;

    inline virtual bool checkForestCompatibility() const { return true; }
};

// ******************************************************************

void MEDDLY::postimage_impl_inner_op::fireTopEventsAtLevel(unpacked_node &nb, unpacked_node& nbout)
{
    const int level = nb.getLevel();
    node_handle *events = rel->arrayForLevel(level);
    int nEventsAtThisLevel = rel->lengthForLevel(nb.getLevel());

    // printf("  fireTopEventsAtLevel: level=%d  #ev:%d\n", level, nEventsAtThisLevel);

    if (0 == nEventsAtThisLevel)
        return;

    dd_edge nbdj(resF), newst(resF);
    expert_domain *dm = static_cast<expert_domain *>(resF->useDomain());

    // Fire all events, one by one, and accumulate the outputs in nbout
    // EA: which iteration order is better? first i and then events, or the opposite??
    for (int i = 0; i < nb.getSize(); i++) { // all unprimed values i
        for (int ei = 0; ei < nEventsAtThisLevel; ei++) { // all events rooted at @level
            relation_node* event = rel->nodeExists(events[ei]);
            if (nb.d(i) == 0)
                continue;
            int j = event->nextOf(i);
            if (j == -1)
                continue;

            node_handle rec = fireEvent(nb.d(i), event->getDown());
            // printf("    fireEvent@top  ei=%d  i:%d -> j:%d = rec:%d  \n", ei, i, j, rec);
            if (rec == 0)
                continue;

            //confirm local state
            rel->setConfirmedStates(level, j);
            if (j >= nbout.getSize()) {
                int new_var_bound = resF->isExtensibleLevel(level) ? -(j + 1) : (j + 1);
                dm->enlargeVariableBound(level, false, new_var_bound);
                int oldSize = nbout.getSize();
                //printf("[1] enlarge lvl:%d  nb:%d  %d->%d\n", level, nbout.getLevel(), oldSize, new_var_bound);
                nbout.resize(j + 1);
                while (oldSize < nbout.getSize()) {
                    nbout.d_ref(oldSize++) = 0;
                }
            }

            if (rec == nbout.d(j)) {
                resF->unlinkNode(rec);
            }
            else if (0 == nbout.d(j)) {
                nbout.d_ref(j) = rec;
            } 
            else if (rec == -1) { // terminal for true
                resF->unlinkNode(nb.d(j));
                nbout.d_ref(j) = -1;
            } 
            else {
                nbdj.set(nbout.d(j)); // clobber
                newst.set(rec); // clobber
                mddUnion->compute(nbdj, newst, nbdj);
                nbout.set_d(j, nbdj);
            }
            MEDDLY_DCASSERT(resF->getNodeLevel(nbout.d(j)) < nbout.getLevel());
        }
    }
}

// Firing of a single relational node down the @mdd
MEDDLY::node_handle MEDDLY::postimage_impl_inner_op::fireEvent(MEDDLY::node_handle mdd, rel_node_handle mxd)
{
    // termination conditions
    assert(mxd > 0);
    // if (mxd == 0 || mdd == 0)
    //     return 0;
    if (mdd == 0)
        return 0;

    if (mxd == 1) {
        // if (arg1F->isTerminalNode(mdd)) 
        //     return resF->handleForValue(true);
        // mxd is identity
        assert(arg1F == resF);
        return resF->linkNode(mdd);
    }

    relation_node* relNode = rel->nodeExists(mxd); // The relation node

    // check the cache
    node_handle result = 0;
    compute_table::entry_key* Key = findResult(mdd, mxd, result);
    if (0 == Key)
        return result;

// #ifdef TRACE_RECFIRE
//     printf("computing fireEvent(%d, %d)\n", mdd, mxd);
//     printf("  node %3d ", mdd);
//     arg1F->showNode(stdout, mdd, 1);
//     printf("\n  node %3d ", mxd);
//     arg2F->showNode(stdout, mxd, 1);
//     printf("\n");
// #endif

    // check if mxd and mdd are at the same level
    const int mddLevel = arg1F->getNodeLevel(mdd);
    const int mxdLevel = relNode->getLevel();
    const int rLevel = std::max(mxdLevel, mddLevel);
    int rSize = resF->getLevelSize(rLevel);
    unpacked_node *nb_out = unpacked_node::newFull(resF, rLevel, rSize);
    expert_domain *dm = static_cast<expert_domain *>(resF->useDomain());

    // printf("     fireEvent mdd:%d  mxd:%d  rLevel:%d  rSize:%d\n", mdd, mxd, rLevel, rSize);

    dd_edge nbdj(resF), newst(resF);

    // Initialize mdd reader
    unpacked_node *nb_in = unpacked_node::useUnpackedNode();
    if (mddLevel < rLevel)
        nb_in->initRedundant(arg1F, rLevel, mdd, true);
    else
        nb_in->initFromNode(arg1F, mdd, true);

    //Re-Think
    if (mddLevel > mxdLevel) {
        // Skipped levels in the MXD,
        // that's an important special case that we can handle quickly.
        for (int i = 0; i < rSize; i++) 
            nb_out->d_ref(i) = fireEvent(nb_in->d(i), mxd);
    } else {
        // Need to process this level in the MXD.
        MEDDLY_DCASSERT(mxdLevel >= mddLevel);

        // Initialize mxd readers, note we might skip the unprimed level
        // loop over mxd "rows"
        for (int iz = 0; iz < rSize; iz++) {
            int i = iz; // relation_node enabling condition
            if (0 == nb_in->d(i))
                continue;

            // loop over mxd "columns"
            int j = relNode->nextOf(i);
            if (j == -1)
                continue;

            node_handle newstates = fireEvent(nb_in->d(i), relNode->getDown());
            if (0 == newstates)
                continue;

            // confirm local state
            if (!rel->isConfirmedState(rLevel, j)) // if not yet confirmed
            {
                rel->setConfirmedStates(rLevel, j); // confirm and enlarge
                if (j >= nb_out->getSize()) {
                    int new_var_bound = resF->isExtensibleLevel(rLevel) ? -(j + 1) : (j + 1);
                    dm->enlargeVariableBound(rLevel, false, new_var_bound);
                    int oldSize = nb_out->getSize();
                    //printf("[2] enlarge lvl:%d  nb:%d  %d->%d\n", rLevel, nb_out->getLevel(), oldSize, new_var_bound);
                    nb_out->resize(j + 1);
                    while (oldSize < nb_out->getSize()) {
                        nb_out->d_ref(oldSize++) = 0;
                    }
                }
            }
            // add the i->j edge
            if (0 == nb_out->d(j)) {
                nb_out->d_ref(j) = newstates;
            }
            else { // there's new states and existing states; union them.
                nbdj.set(nb_out->d(j));
                newst.set(newstates);
                mddUnion->compute(nbdj, newst, nbdj);
                nb_out->set_d(j, nbdj);
            }
            MEDDLY_DCASSERT(resF->getNodeLevel(nb_out->d(j)) <= nb_out->getLevel());
        } // for i
    } // else

    // cleanup mdd reader
    unpacked_node::recycle(nb_in);

    result = resF->createReducedNode(-1, nb_out);

// #ifdef TRACE_ALL_OPS
//     printf("computed fireEvent(%d, %d) = %d\n", mdd, mxd, result);
// #endif
// #ifdef TRACE_RECFIRE
//     printf("computed fireEvent(%d, %d) = %d\n", mdd, mxd, result);
//     printf("  node %3d ", result);
//     resF->showNode(stdout, result, 1);
//     printf("\n");
// #endif

    return saveResult(Key, mdd, mxd, result);
}

// ******************************************************************
// *                                                                *
// *             postimage_impl_inner_op  methods                *
// *                                                                *
// ******************************************************************

MEDDLY::postimage_impl_inner_op::postimage_impl_inner_op(
    const postimage_impl_opname *opcode,
    satimpl_opname::implicit_relation *relation)
    : specialized_operation(opcode, 1)
{
    mddUnion = 0;
    // mxdIntersection = 0;
    // mxdDifference = 0;
    // freeqs = 0;
    // freebufs = 0;
    rel = relation;
    arg1F = static_cast<expert_forest *>(rel->getInForest());
    //arg2F = static_cast<expert_forest*>(rel->getInForest());
    resF = static_cast<expert_forest *>(rel->getOutForest());

    registerInForest(arg1F);
    //registerInForest(arg2F);
    registerInForest(resF);
    compute_table::entry_type *et = new compute_table::entry_type(opcode->getName(), "NL:N");
    et->setForestForSlot(0, arg1F);
    et->setForestForSlot(3, resF);
    registerEntryType(0, et);
    buildCTs();

    // Create the instance of the outer operator
    outer_op = new postimage_impl_outer_op(this, arg1F, resF);
}

MEDDLY::postimage_impl_inner_op::~postimage_impl_inner_op()
{
    if (rel->autoDestroy()) delete rel;
    unregisterInForest(arg1F);
    //unregisterInForest(arg2F);
    unregisterInForest(resF);

    delete outer_op;
}

void MEDDLY::postimage_impl_inner_op ::compute(const dd_edge &a, dd_edge &c)
{
    // Initialize operations
    mddUnion = getOperation(UNION, resF, resF, resF);
    MEDDLY_DCASSERT(mddUnion);

    /*mxdIntersection = getOperation(INTERSECTION, arg2F, arg2F, arg2F);
   MEDDLY_DCASSERT(mxdIntersection);
   
   mxdDifference = getOperation(DIFFERENCE, arg2F, arg2F, arg2F);
   MEDDLY_DCASSERT(mxdDifference);*/

#ifdef DEBUG_INITIAL
    printf("Calling saturate for states:\n");
    ostream_output s(std::cout);
    a.show(s, 2);
    std::cout.flush();
#endif
#ifdef DEBUG_NSF
    printf("Calling saturate for NSF:\n");
    // b.show(stdout, 2);
#endif

    // EA: is this efficient ???? Why is the op reallocated every time?
    // postimage_impl_outer_op *so = new postimage_impl_outer_op(this, arg1F, resF);
    node_handle cnode = outer_op->compute_postimage(a.getNode());
    c.set(cnode);

    // delete so;
}

// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::postimage_impl_opname *MEDDLY::initImplPostImage()
{
    return new postimage_impl_opname("ImplPostImage");
}

MEDDLY::specialized_operation *
MEDDLY::postimage_impl_opname::buildOperation(arguments *a) const
{

    satimpl_opname::implicit_relation *rel = dynamic_cast<satimpl_opname::implicit_relation *>(a);
    if (0 == rel)
        throw error(error::INVALID_ARGUMENT, __FILE__, __LINE__);

    MEDDLY::specialized_operation *op = 0;
    op = new postimage_impl_inner_op(this, rel);

    return op;
}

// ******************************************************************
// *                                                                *
// *               postimage_impl_outer_op  methods              *
// *                                                                *
// ******************************************************************

MEDDLY::postimage_impl_outer_op ::postimage_impl_outer_op(postimage_impl_inner_op *p,
                                                                expert_forest *argF, expert_forest *resF)
    : unary_operation(postimage_impl_unary_opname::getInstance(), 1, argF, resF)
{
    parent = p;

    const char *name = postimage_impl_unary_opname::getInstance()->getName();
    compute_table::entry_type *et;

    if (argF->isFullyReduced())
    {
        // CT entry includes level info
        et = new compute_table::entry_type(name, "NI:N");
        et->setForestForSlot(0, argF);
        et->setForestForSlot(3, resF);
    }
    else
    {
        et = new compute_table::entry_type(name, "N:N");
        et->setForestForSlot(0, argF);
        et->setForestForSlot(2, resF);
    }
    registerEntryType(0, et);
    buildCTs();
}

MEDDLY::postimage_impl_outer_op::~postimage_impl_outer_op()
{
    removeAllComputeTableEntries();
}

MEDDLY::node_handle MEDDLY::postimage_impl_outer_op::compute_postimage(MEDDLY::node_handle mdd)
{
#ifdef DEBUG_INITIAL
    printf("Calling compute_postimage for states:\n");
    ostream_output s(std::cout);
    argF->showNodeGraph(s, &mdd, 1);
    std::cout.flush();
#endif
    return compute_postimage(mdd, argF->getNumVariables());
}

MEDDLY::node_handle
MEDDLY::postimage_impl_outer_op::compute_postimage(node_handle mdd, int k)
{
#ifdef DEBUG_DFS
    printf("compute_postimage mdd: %d, level:%d\n", mdd, k);
#endif
    // terminal condition for recursion
    // if (argF->isTerminalNode(mdd))
    //     return 0;

    if (mdd==0)
        return 0;
    if (k==0) {
        assert(argF->isTerminalNode(mdd));
        return 0;
    }

    // search compute table
    node_handle n = 0;
    compute_table::entry_key *Key = findPostImageResult(mdd, k, n);
    // printf("findPostImageResult mdd:%d level:%d n:%d   %d\n", mdd, k, n, Key);
    if (0 == Key)
        return n;

    const int sz = argF->getLevelSize(k);          // size
    const int mdd_level = argF->getNodeLevel(mdd); // mdd level

#ifdef DEBUG_DFS
    printf("mdd: %d, level: %d, size: %d, mdd_level: %d\n",
           mdd, k, sz, mdd_level);
#endif

    // Initialize mdd reader
    unpacked_node *mddDptrs = unpacked_node::useUnpackedNode();
    if (mdd_level < k) 
        mddDptrs->initRedundant(argF, k, mdd, true);
    else 
        mddDptrs->initFromNode(argF, mdd, true);

    // Fire events below this level
    unpacked_node *nbdown = unpacked_node::newFull(resF, k, sz);
    for (int i = 0; i < sz; i++) {
        nbdown->d_ref(i) = mddDptrs->d(i) ? compute_postimage(mddDptrs->d(i), k - 1) : 0;
    }
    node_handle mdd2 = resF->createReducedNode(-1, nbdown);

    // Fire events rooted at this level
    unpacked_node* nbout = unpacked_node::newFull(resF, k, sz);
    parent->fireTopEventsAtLevel(*mddDptrs, *nbout); 
    node_handle mdd1 = resF->createReducedNode(-1, nbout);

    unpacked_node::recycle(mddDptrs); // Cleanup

    // get the union of all fired event
    dd_edge dd1(resF), dd2(resF), union_dd(resF);
    dd1.set(mdd1);
    dd2.set(mdd2);
    // FIX: the problem with the high-level interface of binary_operators, like mddUnion->compute,
    // is that the produced dd_edge is not at the required level, but at the top level.
    // This interferes when the forest is quasy-reduced instead of fully reduced, because the
    // final dd_edge has a long list of intermediate nodes up to the top level, that are not used.
    // Moreover, this breaks the DAG structure of MDDs. when the reduction rule is set to quasi-reduced.
    parent->mddUnion->compute(dd1, dd2, union_dd);

    n = resF->linkNode(union_dd.getNode());
    MEDDLY_DCASSERT(resF->getNodeLevel(n) <= k);
    // printf("mdd: %d, level: %d, size: %d, mdd_level: %d   n:%d\n", mdd, k, sz, mdd_level, resF->getNodeLevel(n));

    // save in compute table
    savePostImageResult(Key, mdd, n);

#ifdef DEBUG_DFS
    resF->showNodeGraph(stdout, n);
#endif

    return n;
}
