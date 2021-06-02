//-----------------------------------------------------------------------------
// Model Checking core algorithms
//-----------------------------------------------------------------------------
#include "CTL.h"
#include "LTL.h"

#undef GREATER_THAN
#undef DEAD

using namespace std;

//-----------------------------------------------------------------------------
namespace ctlmdd {

//----------------------------------------------------------------------------
// --- Basic Logic algorithms
//----------------------------------------------------------------------------

// potential state on each model variable, and fixed 0 values for the extra levels
dd_edge mdd_potential_state_set(RSRG* rsrg, forest* forestMDD, bool zero_extra_lvls)
{
    expert_forest *forest = static_cast<expert_forest*>(forestMDD);
    dd_edge d(forestMDD);
    // take any value for all levels
    d.set(forest->handleForValue(true));

    if (zero_extra_lvls) {
        // impose a fixed 0 value for all extra levels in the domain
        for (int i=0; i<rsrg->getExtraLvls(); i++) {
            int lvl = rsrg->indexOfExtraLvl(i);
            assert(d.getLevel() < lvl);

            unpacked_node *nb_out = unpacked_node::newFull(forest, lvl, 1);
            nb_out->set_d(0, d);
            d.set(forest->createReducedNode(-1, nb_out));
        }
    }

    // MEDDLY::ostream_output stdout_wrap(cout);
    // d.show(stdout_wrap, 2);
    // assert(0);
    return d;
}

//-----------------------------------------------------------------------------

dd_edge MDD_INTERSECT(dd_edge f1, dd_edge f2) {
    dd_edge result(f1.getForest());
    apply(INTERSECTION, f1, f2, result);
    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::NOT(dd_edge f1) const {
    if (is_true(f1)) // NOT TRUE -> FALSE
        return empty_set();
    else if (is_false(f1)) // NOT FALSE -> TRUE
        return RS;

    // NOT f1
    dd_edge result(get_MDD_forest());
    apply(DIFFERENCE, RS, f1, result);
    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::AND(dd_edge f1, dd_edge f2) const {
    if (is_true(f1)) // TRUE and f2 -> f2
        return f2;
    else if (is_false(f1)) // FALSE AND f2 -> FALSE
        return empty_set();
    if (is_true(f2)) // f1 and TRUE -> f1
        return f1;
    else if (is_false(f2)) // f1 AND FALSE -> FALSE
        return empty_set();
    
    // f1 AND f2
    dd_edge result(get_MDD_forest());
    apply(INTERSECTION, f1, f2, result);
    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::AND_NOT(dd_edge f1, dd_edge f2) const {
    if (is_false(f1)) // FALSE AND (NOT f2) -> FALSE
        return empty_set();
    
    // f1 AND (NOT f2)  computed as a SETDIFF: f1 \ f2
    dd_edge result(get_MDD_forest());
    apply(DIFFERENCE, f1, f2, result);
    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::OR(dd_edge f1, dd_edge f2) const {
    if (is_true(f1)) // TRUE OR f2 -> TRUE
        return RS;
    else if (is_false(f1)) // FALSE OR f2 -> f2
        return f2;

    if (is_true(f2)) // f1 OR TRUE -> TRUE
        return RS;
    else if (is_false(f2)) // f1 OR FALSE -> f1
        return f1;

    // f1 OR f2
    dd_edge result(get_MDD_forest());
    apply(UNION, f1, f2, result);
    return result;
}

//----------------------------------------------------------------------------
// --- CTL Core Algorithms ---
//----------------------------------------------------------------------------

dd_edge Context::SELECT_REAL(dd_edge f1) const {
    if (CTL_preimg_stays_in_RS) {
        apply(INTERSECTION, RS, f1, f1);
    }
    return f1;
}

dd_edge Context::PREIMG(dd_edge f1, bool stuttered) const {
    if (is_false(f1)) // E X False -> false
        return empty_set();
    // Note: for non-ergodic RS, the E X true is only valid
    // for non-dead states. Therefore, the E X true case
    // is not different from the general case.
    if (is_true(f1) && stuttered)
        return RS;

    dd_edge result(get_MDD_forest());
    if (stuttered) {
        result = get_stuttered_nsf()->pre_image(f1);
    }
    else {
        result = vNSF->pre_image(f1);
    }
    return SELECT_REAL(result);
}

//----------------------------------------------------------------------------

dd_edge Context::NON_DEAD() const {
    // dd_edge non_deadlock_states = ctx.vNSF->pre_image(ctx.RS);
    dd_edge non_deadlock_states = PREIMG(RS, false);
	return non_deadlock_states;
}

dd_edge Context::DEAD() const {
    return NOT(NON_DEAD());
}

//----------------------------------------------------------------------------

dd_edge Context::EG(dd_edge f1) const {
    return EGfair(f1);
}

//----------------------------------------------------------------------------

dd_edge Context::EU(dd_edge f1, dd_edge f2) const {
    if (is_false(f2)) // E f1 U false  ->  false
        return empty_set();
    else if (is_true(f2)) // E f1 U true  ->  true
        return RS;

    // E true U f2  ->  E F f2  (not reduced)
    if (is_false(f1)) // E false U f2  ->  f2
        return f2;

    // E[f1 U f2]
    // M,s |= E[f1 U f2] iff there exists a path s_0, s_1, s_2,... where s_0 = s 
    // and there exists k>=0 such that M,sk |= f2 and M,s_i |= f1 for all 0<=i<k.
    // Recursive definition:
    //   E[f1 U f2] = f2 OR (f1 AND EX E[f1 U f2])
    // expands as:
    //   res(0)   = f2
    //   res(i+1) = res(i) OR (f1 AND EX res(i))

    dd_edge result(get_MDD_forest());
    dd_edge curr_result(get_MDD_forest());
    result = SELECT_REAL(f2);

    size_t n_iters = 0;
    do {
        curr_result = result;
        result = OR(result, AND(f1, PREIMG(result, false)));

        n_iters++;
        if (print_intermediate_expr()) {
            cout << (is_true(f1) ? "EF" : "EU") 
                 << ": step=" << n_iters << ",  SAT size=" 
                 << fixed << result.getCardinality() << endl;
            cout.unsetf(ios_base::floatfield);
        }
    }
    while (result != curr_result);

    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::EX(dd_edge f1) const {
    /*dd_edge result = PREIMG(f1);

    if (stutter_EX) {
        // cout << "## !stuttered EX" << endl;
        dd_edge deadlock_f1 = f1 - NON_DEAD();
        // add self-loops on f1-dead states
        result = OR(result, deadlock_f1);
    }*/
    dd_edge result = PREIMG(f1, stutter_EX);

    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::EF(dd_edge f1) const {
    return EU(RS, f1);
}

//----------------------------------------------------------------------------
// Fair ECTL (explicit)
//----------------------------------------------------------------------------

dd_edge Context::EGfair(dd_edge f1) const {
    //   EG f1 = E[false R f1]
    return ERfair(empty_set(), f1);
    /*// if (!has_fairness_constraints())
    //     return EG(f1); // use non-fair EG
    if (is_false(f1)) // E G false -> false
        return empty_set();
    // NOTE: the case E G true is not trivial for non-ergodic RS.

    dd_edge result(get_MDD_forest());
    dd_edge curr_result(get_MDD_forest());
    f1 = SELECT_REAL(f1);
    result = f1;

    size_t n_iters = 0;
    do {
        curr_result = result; // previous iteration result
        if (has_fairness_constraints()) {
            for (auto&& F : fair_sets) {
                dd_edge Y = EU(result, AND(F, result));
                result = AND(result, PREIMG(Y, stutter_EG));
            }
        }
        else {
            result = AND(result, PREIMG(curr_result, stutter_EG));
        }

        n_iters++;
        if (print_intermediate_expr()) {
            cout << (has_fairness_constraints() ? "Fair" : "")
                 << "EG: step=" << n_iters << ",  SAT size=" 
                 << fixed << result.getCardinality() << endl;
            cout.unsetf(ios_base::floatfield);
        }
    }
    while (result != curr_result);

    if (print_intermediate_expr()) {
        cout << "R2 = " << result.getNode() << " in " 
             << n_iters << " iterations." << endl;
    }

    return result;*/
}

//----------------------------------------------------------------------------

dd_edge Context::EXfair(dd_edge f1) const
{
    if (!has_fairness_constraints())
        return EX(f1);

    // fair = EGfair(true)
    dd_edge fair = get_fair_states(); //EGfair(RS);
    // EXfair f = EX(f AND fair)
    return EX(AND(f1, fair));
}

//----------------------------------------------------------------------------

dd_edge Context::EUfair(dd_edge f1, dd_edge f2) const
{
    if (!has_fairness_constraints())
        return EU(f1, f2);

    // fair = EGfair(true)
    dd_edge fair = get_fair_states(); //EGfair(RS);
    // EUfair(f1, f2) = EU(f1, f2 AND fair)
    return EU(f1, AND(f2, fair));
}

//----------------------------------------------------------------------------

dd_edge Context::ERfair(dd_edge f1, dd_edge f2) const {
    if (is_false(f2)) // E f1 R false -> false
        return empty_set();
    // NOTE: the case f2==true is not trivial for non-ergodic RS.

    // E[f1 R f2]
    // M,s |= E[f1 R f2] iff there exists a path s_0, s_1, s_2,... where s_0 = s, 
    // such that for all k > 0, if not(M,s_i |= f1) for all 0<=i<k, then M,s_k |= f2
    // Recursive definition:
    //   E[f1 R f2] = f2 AND (f1 OR EX E[f1 R f2])
    // expands as BDD operations:
    //   res(0)   = f2
    //   res(i+1) = res(i) AND (f1 OR EX res(i))
    //
    // Emerson-Let E_fair G expansion:
    // Efair G f2 = nu Z . ( f2 OR_{Fi \in Fair}(EX(E[Z U (Z and Fi)]))) )
    //   res(0)   = f2
    //   Y(i)     =  AND_{Fi \in Fair} E[res(i) U (res(i) and Fi)]
    //   res(i+1) = res(i) AND EX Y(i+1)
    // Current espanzion for E_fair R
    //   res(0)   = f2
    //   Y(i)     = AND_{Fi \in Fair} E[res(i) U (res(i) and Fi)]
    //   res(i+1) = res(i) AND ((f1 AND FairStates) OR EX Y(i+1))

    dd_edge result(get_MDD_forest());
    dd_edge curr_result(get_MDD_forest());
    f2 = SELECT_REAL(f2);
    result = f2;

    // Since evaluation stops in f1 states, we have to ensure that from every
    // Sat(f1) state a fair path may originate.
    dd_edge fair_f1 = f1;
    if (!is_false(f1) && has_fairness_constraints()) {
        dd_edge fair = get_fair_states(); //EGfair(RS);
        for (auto&& Fi : fair_sets)
            fair_f1 = AND(fair_f1, fair);
    }

    size_t n_iters = 0;
    do {
        curr_result = result; // previous iteration result
        if (has_fairness_constraints()) {
            for (auto&& Fi : fair_sets) {
                dd_edge Y = EU(result, AND(Fi, result));
                result = AND(result, OR(fair_f1, PREIMG(Y, stutter_EG)));
            }
        }
        else {
            result = AND(result, OR(fair_f1, PREIMG(curr_result, stutter_EG)));
        }

        n_iters++;
        if (print_intermediate_expr()) {
            cout << (has_fairness_constraints() ? "Fair" : "")
                 << (is_false(f1) ? "EG" : "ER") 
                 << ": step=" << n_iters << ",  SAT size=" 
                 << fixed << result.getCardinality() << endl;
            cout.unsetf(ios_base::floatfield);
        }
    }
    while (result != curr_result);

    // if (print_intermediate_expr()) {
    //     cout << "R2 = " << result.getNode() << " in " 
    //          << n_iters << " iterations." << endl;
    // }

    return result;
}


//----------------------------------------------------------------------------

dd_edge Context::EFfair(dd_edge f1) const
{
    if (!has_fairness_constraints())
        return EF(f1);

    return EUfair(RS, f1); // true Ufair f1
}

//----------------------------------------------------------------------------
// ForAll-quantified CTL (fair-version only)
//----------------------------------------------------------------------------

dd_edge Context::AXfair(dd_edge f1) const {
    // not EX not f1 
    return NOT(EXfair(NOT(f1)));
}

//----------------------------------------------------------------------------

dd_edge Context::AFfair(dd_edge f1) const {
    // not EG not f1
    return NOT(EGfair(NOT(f1)));
}

//----------------------------------------------------------------------------

dd_edge Context::AGfair(dd_edge f1) const {
    // not (E true U not f1 )  =  not EF not f1
    return NOT(EFfair(NOT(f1)));
}

//----------------------------------------------------------------------------

dd_edge Context::AUfair(dd_edge f1, dd_edge f2) const {
    // //not (E not f2 U (not f1 and not f2) ) and not EG not f2
    // dd_edge not_f1 = NOT(f1);
    // dd_edge not_f2 = NOT(f2);

    // dd_edge not_E_not_f2_U_not_f1_and_not_f2 = NOT(EUfair(not_f2, AND(not_f1, not_f2)));
    // if (is_false(not_E_not_f2_U_not_f1_and_not_f2))
    //     return not_E_not_f2_U_not_f1_and_not_f2;

    // return AND(not_E_not_f2_U_not_f1_and_not_f2,
    //            NOT(EGfair(not_f2)));
    // // return AND(NOT(EUfair(not_f2, AND(not_f1, not_f2))),
    // //            NOT(EGfair(not_f2)));

    // A[f1 U f2] = not E [(not f1) R (not f2)]
    return NOT(ERfair(NOT(f1), NOT(f2)));
}

//----------------------------------------------------------------------------

dd_edge Context::ARfair(dd_edge f1, dd_edge f2) const {
    // A[f1 R f2] = not E [(not f1) U (not f2)]
    return NOT(EUfair(NOT(f1), NOT(f2)));
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
// Fairness constraints management (for fair CTL model checking)
//----------------------------------------------------------------------------

// add a new fairness constraint and update fair_states
void Context::add_fairness_constraint(dd_edge fair_set) {
    fair_sets.push_back(fair_set);
    fair_states = empty_set();
}

//----------------------------------------------------------------------------

// get the set of fair states
dd_edge Context::get_fair_states() const {
    if (is_false(fair_states)) {
        fair_states = EGfair(RS);
    }
    return fair_states;
}

//-----------------------------------------------------------------------------







//-----------------------------------------------------------------------------

const VirtualNSF* Context::get_stuttered_nsf() const {
    if (stuttered_NSF.get() == nullptr) {
        stuttered_NSF = make_unique<StutteredNSF>(vNSF, DEAD());
    }
    return stuttered_NSF.get();
}

//-----------------------------------------------------------------------------









//-----------------------------------------------------------------------------

forest* StutteredNSF::getForestMxD() const {
    return petri_net_NSF->getForestMxD();
}

//-----------------------------------------------------------------------------

dd_edge StutteredNSF::pre_image(const dd_edge& set) const {
    // return petri_net_NSF->pre_image(set) + (set - non_dead_markings);
    return petri_net_NSF->pre_image(set) + (set * dead_markings);
}

//-----------------------------------------------------------------------------

dd_edge StutteredNSF::post_image(const dd_edge& set) const {
    return petri_net_NSF->post_image(set) + (set * dead_markings);
}

//-----------------------------------------------------------------------------

dd_edge StutteredNSF::forward_reachable(const dd_edge& s0) const {
    return petri_net_NSF->forward_reachable(s0);
}

//-----------------------------------------------------------------------------







//-----------------------------------------------------------------------------
// Trace generation algorithms
//-----------------------------------------------------------------------------

// check if a DD contains a specific marking
bool sat_set_contains(const dd_edge &dd, const std::vector<int> &marking) {
    bool isContained;
    dd.getForest()->evaluate(dd, marking.data(), isContained);
    return isContained;
}

//-----------------------------------------------------------------------------

bool sat_set_is_empty(const dd_edge &e) {
    expert_forest *forest = static_cast<expert_forest *>(e.getForest());
    return e.getNode() == forest->handleForValue(false);
}

//-----------------------------------------------------------------------------

// create the DD corresponding to a single marking
dd_edge dd_from_marking(const std::vector<int> &marking, MEDDLY::forest *mdd_forest) {
    dd_edge dd_of_state(mdd_forest);
    const int *vlist = marking.data();
    mdd_forest->createEdge(&vlist, 1, dd_of_state);
    return dd_of_state;
}

//-----------------------------------------------------------------------------

// extract an arbitrary state from the dd
void get_marking_from_dd(const dd_edge &dd, std::vector<int> &marking) {
    MEDDLY::enumerator it(dd);
    const int *tmp = it.getAssignments();
    const size_t n_vars = dd.getForest()->getDomain()->getNumVariables();
    marking.resize(n_vars + 1);
    std::copy(tmp, tmp + n_vars + 1, marking.begin());
}

//-----------------------------------------------------------------------------

// generate a trace in the form:
//  s0 -> s1 -> s2 -> ... -> sN
// from a vector of M>=N intermediate DD. 
// sN \subsetof steps[M-1]
// the first step is the first DD that contains s0.
void generate_sequential_trace(const std::vector<int> &s0, 
                               const std::vector<dd_edge> &steps, 
                               const VirtualNSF& NSF,
                               std::list<std::vector<int>> &trace_states) 
{
    MEDDLY::forest* forestMDD = steps.front().getForest();
    // follow back the intermediate DD steps, and find the one that contains s0
    int start_i;
    for (start_i=steps.size()-1; start_i>=0; start_i--) {
        if (sat_set_contains(steps[start_i], s0))
            break;
    }
    if (start_i < 0)
        throw rgmedd_exception(" Could not find s0 in the trace");

    // Generate the trace from steps[i] to the end, starting from state s0 and picking
    // arbitrary successor states
    std::vector<int> state = s0;
    trace_states.push_back(state);
    for (int i = start_i+1; i<steps.size(); i++) {
        dd_edge dd_state = dd_from_marking(state, forestMDD);
        // generate the successors of @state
        dd_edge dd_succ = NSF.post_image(dd_state);
        // take only those that are also in step[i]
        MEDDLY::apply(INTERSECTION, dd_succ, steps[i], dd_succ);
        if (sat_set_is_empty(dd_succ))
            throw rgmedd_exception("Problem generating the trace successors");
        // get one successor state for the trace
        get_marking_from_dd(dd_succ, state);
        trace_states.push_back(state);
    }
}

//-----------------------------------------------------------------------------

// generate a rho trace in the form:
//  s0 -> s1 -> s2 -> ... -> sk -> ... -> sN --\   <- goes back
//                            ^----------------/
// that contains a self loop going back to some intermediate state sk.
// States in the trace belong to the M steps DDs.
void generate_rho_trace(const std::vector<int> &s0, 
                        const std::vector<dd_edge> &steps, 
                        const VirtualNSF& NSF,
                        std::list<std::vector<int>> &trace_states,
                        size_t& start_of_loop) 
{
    MEDDLY::forest* forestMDD = steps.front().getForest();
    // first generate a sequence from s0 to sk in the last steps[] DD,
    // which is assumed to be a SCC.
    generate_sequential_trace(s0, steps, NSF, trace_states);
    start_of_loop = trace_states.size();

    std::vector<int> sk = trace_states.back();
    dd_edge dd_sk = dd_from_marking(sk, forestMDD);

    // find a loop that goes back to sk, while remaining in the SCC. 
    // Generate all intermediate DDs
    std::vector<dd_edge> loop_steps;
    loop_steps.push_back(dd_sk);

    while (true) {
        dd_edge succs = NSF.post_image(loop_steps.back());
        MEDDLY::apply(INTERSECTION, succs, steps.back(), succs);
        loop_steps.push_back(succs);
        if (sat_set_contains(succs, sk))
            break;
    }
    auto iter = loop_steps.rbegin();
    while (true) {
        dd_edge preds = *iter;
        iter++;
        if (iter == loop_steps.rend())
            break;

        preds = NSF.pre_image(preds);
        MEDDLY::apply(INTERSECTION, *iter, preds, *iter);
    }

    trace_states.resize(trace_states.size() - 1); // remove the last state
    // Now generate a self loop visiting the succs_lst DDs.
    generate_sequential_trace(sk, loop_steps, NSF, trace_states);
}

//-----------------------------------------------------------------------------






//-----------------------------------------------------------------------------
} // end namespace ctlmdd
