#include <typeinfo> // for "bad_cast" exception
#include <set>
#include <map>
#include <cassert>

#ifdef HAS_GMP_LIB
#include <gmpxx.h>
#endif

// Include meddly only after having included <gmpxx.h>
#include <meddly/meddly.h>
#include <meddly/ct_entry_key.h>
#include <meddly/ct_entry_result.h>
#include <meddly/compute_table.h>
#include <meddly/oper_satur.h>

#undef MEDDLY_DCASSERT
#define MEDDLY_DCASSERT(x)  assert(x)

#include "implicit_postimage.h"

namespace MEDDLY {
    class postimage_impl_outer_op; // outer recursion operation (all firings)
    class postimage_impl_inner_op; // inner recursion operation (single event firings)
}; // namespace MEDDLY

// ******************************************************************
// *                                                                *
// *             postimage_impl_outer_op  class                     *
// *                                                                *
// ******************************************************************

// This class is the cache for the all-event-firings relation: mdd -> mdd'
// in the outer step of the recursion (compute_postimage)
class MEDDLY::postimage_impl_outer_op : public saturation_operation {
    postimage_impl_inner_op* parent;
public:
    postimage_impl_outer_op(postimage_impl_inner_op* p,
        forest* argF, forest* resF);
    virtual ~postimage_impl_outer_op();

    virtual void compute(const dd_edge& a, dd_edge& c) override
    {
        c.set(compute_postimage(a.getNode()));
    }

    node_handle compute_postimage(node_handle mdd);
    node_handle compute_postimage(node_handle mdd, int level);

protected:
    inline ct_entry_key*
    findPostImageResult(node_handle a, int level, node_handle& b)
    {
        ct_entry_key* CTsrch = CT0->useEntryKey(etype[0], 0);
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

    inline void recycleCTKey(ct_entry_key* CTsrch)
    {
        CT0->recycle(CTsrch);
    }

    inline node_handle savePostImageResult(ct_entry_key* Key,
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
class MEDDLY::postimage_impl_inner_op : public saturation_operation {
public:
    postimage_impl_inner_op(implicit_relation* rel);
    virtual ~postimage_impl_inner_op();

    // operation starting point
    virtual void compute(const dd_edge& a, dd_edge& c) override;
    void fireTopEventsAtLevel(unpacked_node& mdd, unpacked_node& nbout);
    node_handle fireEvent(node_handle mdd, rel_node_handle mxd);

protected:
    inline ct_entry_key*
    findResult(node_handle a, rel_node_handle b, node_handle& c)
    {
        ct_entry_key* CTsrch = CT0->useEntryKey(etype[0], 0);
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

    inline void recycleCTKey(ct_entry_key* CTsrch)
    {
        CT0->recycle(CTsrch);
    }

    inline node_handle saveResult(ct_entry_key* Key,
        node_handle a, rel_node_handle b, node_handle c)
    {
        CTresult[0].reset();
        CTresult[0].writeN(c);
        CT0->addEntry(Key, CTresult[0]);
        return c;
    }

public:
    binary_operation* mddUnion;

    implicit_relation* rel;

    postimage_impl_outer_op *outer_op;

    forest* arg1F;
    forest* resF;
};

// ******************************************************************
// *                                                                *
// *             postimage_impl_inner_op  methods                   *
// *                                                                *
// ******************************************************************

// Firing of all events rooted at level of @nb
void MEDDLY::postimage_impl_inner_op::fireTopEventsAtLevel(unpacked_node &nb, unpacked_node &nbout)
{
    const int level = nb.getLevel();
    rel_node_handle *events = rel->arrayForLevel(level);
    int nEventsAtThisLevel = rel->lengthForLevel(nb.getLevel());

    if (0 == nEventsAtThisLevel)
        return;

    dd_edge nbdj(resF), newst(resF);
    domain *dm = resF->getDomain();

    // Fire all events, one by one, and accumulate the outputs in nbout
    // EA: which iteration order is better? first i and then events, or the opposite??
    for (int i = 0; i < (int)nb.getSize(); i++) { // all unprimed values i
        for (int ei = 0; ei < nEventsAtThisLevel; ei++) { // all events rooted at @level
            relation_node* event = rel->nodeExists(events[ei]);
            if (nb.down(i) == 0)
                continue;
            int j = event->nextOf(i);
            if (j == -1)
                continue;

            node_handle rec = fireEvent(nb.down(i), event->getDown());
            if (rec == 0)
                continue;

            // confirm local state
            rel->confirm(level, j);
            if (j >= (int)nbout.getSize()) {
                int new_var_bound = dm->getVar(level)->isExtensible() ? -(j + 1) : (j + 1);
                dm->enlargeVariableBound(level, false, new_var_bound);
                int oldSize = nbout.getSize();
                nbout.resize(j + 1);
                while (oldSize < (int)nbout.getSize()) {
                    nbout.down(oldSize++) = 0;
                }
            }

            if (rec == nbout.down(j)) {
                resF->unlinkNode(rec);
            }
            else if (0 == nbout.down(j)) {
                nbout.down(j) = rec;
            } 
            else if (rec == -1) { // terminal for true
                resF->unlinkNode(nbout.down(j));
                nbout.down(j) = -1;
            } 
            else {
                nbdj.set(nbout.down(j)); // clobber
                newst.set(rec); // clobber
                mddUnion->computeTemp(nbdj, newst, nbdj);
                nbout.setFull(j, nbdj);
            }
            MEDDLY_DCASSERT(resF->getNodeLevel(nbout.down(j)) < nbout.getLevel());
        }
    }
}

// Firing of a single relational node down the @mdd
MEDDLY::node_handle MEDDLY::postimage_impl_inner_op::fireEvent(MEDDLY::node_handle mdd, rel_node_handle mxd)
{
    // termination conditions
    assert(mxd > 0);
    if (mdd == 0)
        return 0;

    if (mxd == 1) {
        assert(arg1F == resF);
        return resF->linkNode(mdd);
    }

    relation_node* relNode = rel->nodeExists(mxd); // The relation node

    // check the cache
    node_handle result = 0;
    ct_entry_key* Key = findResult(mdd, mxd, result);
    if (0 == Key)
        return result;

    // check if mxd and mdd are at the same level
    const int mddLevel = arg1F->getNodeLevel(mdd);
    const int mxdLevel = relNode->getLevel();
    const int rLevel = std::max(mxdLevel, mddLevel);
    int rSize = resF->getLevelSize(rLevel);
    unpacked_node *nb_out = unpacked_node::newWritable(resF, rLevel, rSize, FULL_ONLY);
    domain *dm = resF->getDomain();

    dd_edge nbdj(resF), newst(resF);

    // Initialize mdd reader
    unpacked_node *nb_in;
    if (mddLevel < rLevel)
        nb_in = unpacked_node::newRedundant(arg1F, rLevel, mdd, FULL_ONLY);
    else
        nb_in = unpacked_node::newFromNode(arg1F, mdd, FULL_ONLY);

    if (mddLevel > mxdLevel) {
        // Skipped levels in the MXD,
        // that's an important special case that we can handle quickly.
        for (int i = 0; i < rSize; i++) 
            nb_out->down(i) = fireEvent(nb_in->down(i), mxd);
    } else {
        // Need to process this level in the MXD.
        MEDDLY_DCASSERT(mxdLevel >= mddLevel);

        // Initialize mxd readers, note we might skip the unprimed level
        // loop over mxd "rows"
        for (int iz = 0; iz < rSize; iz++) {
            int i = iz; // relation_node enabling condition
            if (0 == nb_in->down(i))
                continue;

            // loop over mxd "columns"
            int j = relNode->nextOf(i);
            if (j == -1)
                continue;

            node_handle newstates = fireEvent(nb_in->down(i), relNode->getDown());
            if (0 == newstates)
                continue;

            // confirm local state
            if (!rel->isConfirmedState(rLevel, j))
            {
                rel->confirm(rLevel, j);
                if (j >= (int)nb_out->getSize()) {
                    int new_var_bound = dm->getVar(rLevel)->isExtensible() ? -(j + 1) : (j + 1);
                    dm->enlargeVariableBound(rLevel, false, new_var_bound);
                    int oldSize = nb_out->getSize();
                    nb_out->resize(j + 1);
                    while (oldSize < (int)nb_out->getSize()) {
                        nb_out->down(oldSize++) = 0;
                    }
                }
            }
            // add the i->j edge
            if (0 == nb_out->down(j)) {
                nb_out->down(j) = newstates;
            }
            else { // there's new states and existing states; union them.
                nbdj.set(nb_out->down(j));
                newst.set(newstates);
                mddUnion->computeTemp(nbdj, newst, nbdj);
                nb_out->setFull(j, nbdj);
            }
            MEDDLY_DCASSERT(resF->getNodeLevel(nb_out->down(j)) <= nb_out->getLevel());
        } // for i
    } // else

    // cleanup mdd reader
    unpacked_node::Recycle(nb_in);

    result = resF->createReducedNode(-1, nb_out);

    return saveResult(Key, mdd, mxd, result);
}

// ******************************************************************
// *                                                                *
// *             postimage_impl_inner_op  methods                   *
// *                                                                *
// ******************************************************************

MEDDLY::postimage_impl_inner_op::postimage_impl_inner_op(
    implicit_relation *relation)
    : saturation_operation("ImplPostImage", 1, relation->getInForest(), relation->getOutForest())
{
    mddUnion = 0;
    rel = relation;
    arg1F = rel->getInForest();
    resF = rel->getOutForest();

    ct_entry_type *et = new ct_entry_type(getName(), "NL:N");
    et->setForestForSlot(0, arg1F);
    et->setForestForSlot(3, resF);
    registerEntryType(0, et);
    buildCTs();

    // Create the instance of the outer operator
    outer_op = new postimage_impl_outer_op(this, arg1F, resF);
}

MEDDLY::postimage_impl_inner_op::~postimage_impl_inner_op()
{
    delete outer_op;
}

void MEDDLY::postimage_impl_inner_op::compute(const dd_edge &a, dd_edge &c)
{
    // Initialize operations
    if (0 == mddUnion) {
        mddUnion = build(UNION, resF, resF, resF);
        MEDDLY_DCASSERT(mddUnion);
    }

    node_handle cnode = outer_op->compute_postimage(a.getNode());
    c.set(cnode);
}

// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::saturation_operation*
MEDDLY::createImplicitPostImage(implicit_relation* rel)
{
    if (0 == rel)
        throw error(error::INVALID_ARGUMENT, __FILE__, __LINE__);
    return new postimage_impl_inner_op(rel);
}

// ******************************************************************
// *                                                                *
// *               postimage_impl_outer_op  methods                 *
// *                                                                *
// ******************************************************************

MEDDLY::postimage_impl_outer_op::postimage_impl_outer_op(postimage_impl_inner_op *p,
                                                         forest *argF, forest *resF)
    : saturation_operation("PostImage_impl_by_events", 1, argF, resF)
{
    parent = p;

    const char *name = "PostImage_impl_by_events";
    ct_entry_type *et;

    if (argF->isFullyReduced())
    {
        // CT entry includes level info
        et = new ct_entry_type(name, "NI:N");
        et->setForestForSlot(0, argF);
        et->setForestForSlot(3, resF);
    }
    else
    {
        et = new ct_entry_type(name, "N:N");
        et->setForestForSlot(0, argF);
        et->setForestForSlot(2, resF);
    }
    registerEntryType(0, et);
    buildCTs();
}

MEDDLY::postimage_impl_outer_op::~postimage_impl_outer_op()
{
}

MEDDLY::node_handle MEDDLY::postimage_impl_outer_op::compute_postimage(MEDDLY::node_handle mdd)
{
    return compute_postimage(mdd, argF->getNumVariables());
}

MEDDLY::node_handle
MEDDLY::postimage_impl_outer_op::compute_postimage(node_handle mdd, int k)
{
    // terminal condition for recursion
    if (mdd == 0)
        return 0;
    if (k == 0) {
        assert(argF->isTerminalNode(mdd));
        return 0;
    }

    // search compute table
    node_handle n = 0;
    ct_entry_key *Key = findPostImageResult(mdd, k, n);
    if (0 == Key)
        return n;

    const int sz = argF->getLevelSize(k);          // size
    const int mdd_level = argF->getNodeLevel(mdd); // mdd level

    // Initialize mdd reader
    unpacked_node *mddDptrs;
    if (mdd_level < k) 
        mddDptrs = unpacked_node::newRedundant(argF, k, mdd, FULL_ONLY);
    else 
        mddDptrs = unpacked_node::newFromNode(argF, mdd, FULL_ONLY);

    // Fire events below this level
    unpacked_node *nbdown = unpacked_node::newWritable(resF, k, sz, FULL_ONLY);
    for (int i = 0; i < sz; i++) {
        nbdown->down(i) = mddDptrs->down(i) ? compute_postimage(mddDptrs->down(i), k - 1) : 0;
    }
    node_handle mdd2 = resF->createReducedNode(-1, nbdown);

    // Fire events rooted at this level
    unpacked_node* nbout = unpacked_node::newWritable(resF, k, sz, FULL_ONLY);
    parent->fireTopEventsAtLevel(*mddDptrs, *nbout); 
    node_handle mdd1 = resF->createReducedNode(-1, nbout);

    unpacked_node::Recycle(mddDptrs); // Cleanup

    // get the union of all fired event
    dd_edge dd1(resF), dd2(resF), union_dd(resF);
    dd1.set(mdd1);
    dd2.set(mdd2);
    // FIX: the problem with the high-level interface of binary_operators, like mddUnion->compute,
    // is that the produced dd_edge is not at the required level, but at the top level.
    // This interferes when the forest is quasy-reduced instead of fully reduced, because the
    // final dd_edge has a long list of intermediate nodes up to the top level, that are not used.
    // Moreover, this breaks the DAG structure of MDDs. when the reduction rule is set to quasi-reduced.
    parent->mddUnion->computeTemp(dd1, dd2, union_dd);

    n = resF->linkNode(union_dd.getNode());
    MEDDLY_DCASSERT(resF->getNodeLevel(n) <= k);

    // save in compute table
    savePostImageResult(Key, mdd, n);

    return n;
}
