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
    forest *forest_ptr = forestMDD;
    dd_edge d(forestMDD);
    // take any value for all levels
    d.set(forest_ptr->handleForValue(true));

    if (zero_extra_lvls) {
        // impose a fixed 0 value for all extra levels in the domain
        for (int i=0; i<rsrg->getExtraLvls(); i++) {
            int lvl = rsrg->indexOfExtraLvl(i);
            assert(d.getLevel() < lvl);

            unpacked_node *nb_out = unpacked_node::newWritable(forest_ptr, lvl, 1, FULL_ONLY);
            nb_out->down(0) = forest_ptr->linkNode(d.getNode());
            d.set(forest_ptr->createReducedNode(-1, nb_out));
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
dd_edge MDD_DIFFERENCE(dd_edge f1, dd_edge f2) {
    dd_edge result(f1.getForest());
    apply(DIFFERENCE, f1, f2, result);
    return result;
}
dd_edge MDD_UNION(dd_edge f1, dd_edge f2) {
    dd_edge result(f1.getForest());
    apply(UNION, f1, f2, result);
    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::NOT(dd_edge f1) const {
    if (is_true(f1)) // NOT TRUE -> FALSE
        return empty_set();
    else if (is_false(f1)) // NOT FALSE -> TRUE
        return RS;

    // NOT f1
    // dd_edge result(get_MDD_forest());
    // apply(DIFFERENCE, RS, f1, result);
    return MDD_DIFFERENCE(RS, f1);
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
    // dd_edge result(get_MDD_forest());
    // apply(INTERSECTION, f1, f2, result);
    return MDD_INTERSECT(f1, f2);
}

//----------------------------------------------------------------------------

dd_edge Context::AND_NOT(dd_edge f1, dd_edge f2) const {
    if (is_false(f1)) // FALSE AND (NOT f2) -> FALSE
        return empty_set();
    
    // f1 AND (NOT f2)  computed as a SETDIFF: f1 \ f2
    // dd_edge result(get_MDD_forest());
    // apply(DIFFERENCE, f1, f2, result);
    return MDD_DIFFERENCE(f1, f2);;
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
    // dd_edge result(get_MDD_forest());
    // apply(UNION, f1, f2, result);
    return MDD_UNION(f1, f2);
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
            cardinality_t c;
            apply(CARDINALITY, result, cardinality_ref(c));
            cout << (is_true(f1) ? "EF" : "EU") 
                 << ": step=" << n_iters << ",  SAT size=" 
                 << fixed << c << endl;
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
            cardinality_t c;
            apply(CARDINALITY, result, cardinality_ref(c));
            cout << (has_fairness_constraints() ? "Fair" : "")
                 << (is_false(f1) ? "EG" : "ER") 
                 << ": step=" << n_iters << ",  SAT size=" 
                 << fixed << c << endl;
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
    bool isContained = false;
    minterm m(dd.getForest());
    int num_vars = dd.getForest()->getDomain()->getNumVariables();
    for (int i = 1; i <= num_vars; ++i) {
        int val = (i < (int)marking.size()) ? marking[i] : 0;
        m.setVar(i, val);
    }
    dd.evaluate(m, isContained);
    return isContained;
}

//-----------------------------------------------------------------------------

bool sat_set_is_empty(const dd_edge &e) {
    const forest *forest = e.getForest();
    return e.getNode() == forest->handleForValue(false);
}

//-----------------------------------------------------------------------------

// create the DD corresponding to a single marking
dd_edge dd_from_marking(const std::vector<int> &marking, MEDDLY::forest *mdd_forest) {
    dd_edge dd_of_state(mdd_forest);
    minterm m(mdd_forest);
    int num_vars = mdd_forest->getDomain()->getNumVariables();
    for (int i = 1; i <= num_vars; ++i) {
        int val = (i < (int)marking.size()) ? marking[i] : 0;
        m.setVar(i, val);
    }
    m.buildFunction(false, dd_of_state);
    return dd_of_state;
}

//-----------------------------------------------------------------------------

// extract an arbitrary state from the dd
void get_marking_from_dd(const dd_edge &dd, std::vector<int> &marking) {
    const size_t n_vars = dd.getForest()->getDomain()->getNumVariables();
    marking.assign(n_vars + 1, 0);
    auto it = dd.begin();
    if (it) {
        const minterm& m = *it;
        for (size_t i = 1; i <= n_vars; ++i) {
            marking[i] = m.getVar(i);
        }
    }
}

//-----------------------------------------------------------------------------

// Resolution of firing transition between two states
int Context::find_firing_transition(const std::vector<int>& s_prev,
                                    const std::vector<int>& s_curr) const
{
    dd_edge dd_prev = dd_from_marking(s_prev, get_MDD_forest());
    for (int tr = 0; tr < ntr; ++tr) {
        dd_edge img_tr(get_MDD_forest());
        apply(POST_IMAGE, dd_prev, rsrg->getEventMxD(tr), img_tr);
        if (sat_set_contains(img_tr, s_curr)) {
            return tr;
        }
    }
    return -1;
}

//-----------------------------------------------------------------------------

// generate a trace in the form:
//  s0 -> s1 -> s2 -> ... -> sN
// from a vector of intermediate DDs.
void generate_sequential_trace(const Context& ctx,
                               const std::vector<int> &s0, 
                               const std::vector<dd_edge> &steps, 
                               const VirtualNSF& NSF,
                               std::list<std::vector<int>> &trace_states,
                               std::vector<int> &trace_transitions) 
{
    MEDDLY::forest* forestMDD = steps.front().getForest();
    int start_i = -1;
    for (int i = 0; i < (int)steps.size(); i++) {
        if (sat_set_contains(steps[i], s0)) {
            start_i = i;
            break;
        }
    }
    if (start_i < 0) {
        start_i = (int)steps.size() - 1;
    }

    std::vector<int> state = s0;
    trace_states.push_back(state);
    trace_transitions.push_back(-1);

    for (int i = start_i - 1; i >= 0; i--) {
        dd_edge dd_state = dd_from_marking(state, forestMDD);
        dd_edge dd_succ = NSF.post_image(dd_state);
        MEDDLY::apply(INTERSECTION, dd_succ, steps[i], dd_succ);
        if (sat_set_is_empty(dd_succ))
            break;
        std::vector<int> next_state;
        get_marking_from_dd(dd_succ, next_state);
        int tr = ctx.find_firing_transition(state, next_state);
        trace_states.push_back(next_state);
        trace_transitions.push_back(tr);
        state = next_state;
    }
}

void generate_sequential_trace(const std::vector<int> &s0, 
                               const std::vector<dd_edge> &steps, 
                               const VirtualNSF& NSF,
                               std::list<std::vector<int>> &trace_states) 
{
    MEDDLY::forest* forestMDD = steps.front().getForest();
    int start_i = -1;
    for (int i = 0; i < (int)steps.size(); i++) {
        if (sat_set_contains(steps[i], s0)) {
            start_i = i;
            break;
        }
    }
    if (start_i < 0) start_i = (int)steps.size() - 1;

    std::vector<int> state = s0;
    trace_states.push_back(state);
    for (int i = start_i - 1; i >= 0; i--) {
        dd_edge dd_state = dd_from_marking(state, forestMDD);
        dd_edge dd_succ = NSF.post_image(dd_state);
        MEDDLY::apply(INTERSECTION, dd_succ, steps[i], dd_succ);
        if (sat_set_is_empty(dd_succ))
            break;
        get_marking_from_dd(dd_succ, state);
        trace_states.push_back(state);
    }
}

//-----------------------------------------------------------------------------

// generate a rho trace in the form:
//  s0 -> s1 -> s2 -> ... -> sk -> ... -> sN --\   <- goes back
//                            ^----------------/
void generate_rho_trace(const Context& ctx,
                        const std::vector<int> &s0, 
                        const std::vector<dd_edge> &steps, 
                        const VirtualNSF& NSF,
                        std::list<std::vector<int>> &trace_states,
                        std::vector<int> &trace_transitions,
                        size_t& start_of_loop) 
{
    MEDDLY::forest* forestMDD = steps.front().getForest();
    generate_sequential_trace(ctx, s0, steps, NSF, trace_states, trace_transitions);
    start_of_loop = trace_states.size();

    std::vector<int> sk = trace_states.back();
    dd_edge dd_sk = dd_from_marking(sk, forestMDD);

    std::vector<dd_edge> loop_steps;
    loop_steps.push_back(dd_sk);

    while (true) {
        dd_edge succs = NSF.post_image(loop_steps.back());
        MEDDLY::apply(INTERSECTION, succs, steps.back(), succs);
        if (sat_set_is_empty(succs)) break;
        loop_steps.push_back(succs);
        if (sat_set_contains(succs, sk))
            break;
        if (loop_steps.size() > 1000) break;
    }
    if (loop_steps.size() > 1) {
        auto iter = loop_steps.rbegin();
        while (true) {
            dd_edge preds = *iter;
            iter++;
            if (iter == loop_steps.rend())
                break;
            preds = NSF.pre_image(preds);
            MEDDLY::apply(INTERSECTION, *iter, preds, *iter);
        }
        std::list<std::vector<int>> loop_states;
        std::vector<int> loop_transitions;
        generate_sequential_trace(ctx, sk, loop_steps, NSF, loop_states, loop_transitions);
        if (loop_states.size() > 1) {
            auto it_s = ++loop_states.begin();
            auto it_t = ++loop_transitions.begin();
            for (; it_s != loop_states.end(); ++it_s, ++it_t) {
                trace_states.push_back(*it_s);
                trace_transitions.push_back(*it_t);
            }
        }
    }
}

void generate_rho_trace(const std::vector<int> &s0, 
                        const std::vector<dd_edge> &steps, 
                        const VirtualNSF& NSF,
                        std::list<std::vector<int>> &trace_states,
                        size_t& start_of_loop) 
{
    MEDDLY::forest* forestMDD = steps.front().getForest();
    generate_sequential_trace(s0, steps, NSF, trace_states);
    start_of_loop = trace_states.size();

    std::vector<int> sk = trace_states.back();
    dd_edge dd_sk = dd_from_marking(sk, forestMDD);

    std::vector<dd_edge> loop_steps;
    loop_steps.push_back(dd_sk);

    while (true) {
        dd_edge succs = NSF.post_image(loop_steps.back());
        MEDDLY::apply(INTERSECTION, succs, steps.back(), succs);
        if (sat_set_is_empty(succs)) break;
        loop_steps.push_back(succs);
        if (sat_set_contains(succs, sk))
            break;
        if (loop_steps.size() > 1000) break;
    }
    if (loop_steps.size() > 1) {
        auto iter = loop_steps.rbegin();
        while (true) {
            dd_edge preds = *iter;
            iter++;
            if (iter == loop_steps.rend())
                break;
            preds = NSF.pre_image(preds);
            MEDDLY::apply(INTERSECTION, *iter, preds, *iter);
        }
        std::list<std::vector<int>> loop_states;
        generate_sequential_trace(sk, loop_steps, NSF, loop_states);
        if (loop_states.size() > 1) {
            auto it_s = ++loop_states.begin();
            for (; it_s != loop_states.end(); ++it_s) {
                trace_states.push_back(*it_s);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// CTL Core Trace Algorithms
//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_EX(Formula* top_formula, Formula* phi, const std::vector<int>& state0, TraceType traceTy) {
    TreeTraceNode* ttn = new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
    if (traceTy == TT_COUNTEREXAMPLE) {
        ttn->set_ECTL_cntexample();
        return ttn;
    }

    dd_edge phi_mdd = phi->getMDD(*this);
    if (has_fairness_constraints()) {
        phi_mdd = AND(phi_mdd, get_fair_states());
    }

    dd_edge dd_s0 = dd_from_marking(state0, get_MDD_forest());
    dd_edge succs(get_MDD_forest());
    if (stutter_EX) {
        succs = get_stuttered_nsf()->post_image(dd_s0);
    } else {
        succs = vNSF->post_image(dd_s0);
    }
    succs = MDD_INTERSECT(succs, phi_mdd);

    if (sat_set_is_empty(succs)) {
        return ttn;
    }

    std::vector<int> state1;
    get_marking_from_dd(succs, state1);
    int tr = find_firing_transition(state0, state1);

    TreeTraceNode* next_ttn = new TreeTraceNode(state1, traceTy, tr, (tr >= 0 && tr < ntr) ? tabt[tr].trans_name : "");
    ttn->set_next(next_ttn);

    next_ttn->set_sub_trace1(phi->generateTrace(*this, state1, traceTy));
    return ttn;
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_EF(Formula* top_formula, Formula* phi, const std::vector<int>& state0, TraceType traceTy) {
    TreeTraceNode* ttn = new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
    if (traceTy == TT_COUNTEREXAMPLE) {
        ttn->set_ECTL_cntexample();
        return ttn;
    }

    dd_edge phi_mdd = phi->getMDD(*this);
    if (has_fairness_constraints()) {
        phi_mdd = AND(phi_mdd, get_fair_states());
    }

    if (sat_set_contains(phi_mdd, state0)) {
        ttn->set_sub_trace1(phi->generateTrace(*this, state0, traceTy));
        return ttn;
    }

    std::vector<dd_edge> steps;
    steps.push_back(phi_mdd);
    while (!sat_set_contains(steps.back(), state0)) {
        dd_edge pre = PREIMG(steps.back(), stutter_EX);
        if (sat_set_is_empty(pre) || pre == steps.back()) {
            break;
        }
        steps.push_back(pre);
    }

    if (!sat_set_contains(steps.back(), state0)) {
        return ttn;
    }

    TraceType tt0 = (steps.size() > 1) ? oppositeTraceType(traceTy) : traceTy;
    ttn->set_sub_trace1(phi->generateTrace(*this, state0, tt0));

    TreeTraceNode* current_ttn = ttn;
    std::vector<int> curr_state = state0;

    for (ssize_t step = (ssize_t)steps.size() - 2; step >= 0; step--) {
        dd_edge dd_curr = dd_from_marking(curr_state, get_MDD_forest());
        dd_edge succs(get_MDD_forest());
        if (stutter_EX) {
            succs = get_stuttered_nsf()->post_image(dd_curr);
        } else {
            succs = vNSF->post_image(dd_curr);
        }
        succs = MDD_INTERSECT(succs, steps[step]);
        if (sat_set_is_empty(succs)) {
            break;
        }
        std::vector<int> next_state;
        get_marking_from_dd(succs, next_state);
        int tr = find_firing_transition(curr_state, next_state);

        TreeTraceNode* next_ttn = new TreeTraceNode(next_state, traceTy, tr, (tr >= 0 && tr < ntr) ? tabt[tr].trans_name : "");
        TraceType tt = (step > 0) ? oppositeTraceType(traceTy) : traceTy;
        next_ttn->set_sub_trace1(phi->generateTrace(*this, next_state, tt));

        current_ttn->set_next(next_ttn);
        current_ttn = next_ttn;
        curr_state = next_state;
    }
    return ttn;
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_EG(Formula* top_formula, Formula* phi, const std::vector<int>& state0, TraceType traceTy) {
    TreeTraceNode* ttn = new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
    if (traceTy == TT_COUNTEREXAMPLE) {
        ttn->set_ECTL_cntexample();
        return ttn;
    }

    dd_edge phi_mdd = phi->getMDD(*this);
    dd_edge S_EG = EGfair(phi_mdd);
    if (!sat_set_contains(S_EG, state0)) {
        return ttn;
    }

    const VirtualNSF* nsf = stutter_EG ? get_stuttered_nsf() : vNSF.get();
    dd_edge dd_s0 = dd_from_marking(state0, get_MDD_forest());

    // =========================================================================
    // Phase 1: Forward BFS layers from s0 (Check Case A: cycle through s0)
    // =========================================================================
    std::vector<dd_edge> L;
    L.push_back(dd_s0);
    dd_edge visited = dd_s0;
    int cycle_len = -1;

    for (int step = 1; step <= 1000; ++step) {
        dd_edge nxt = nsf->post_image(L.back());
        nxt = MDD_INTERSECT(nxt, S_EG);
        if (sat_set_is_empty(nxt)) {
            break;
        }

        L.push_back(nxt);
        if (sat_set_contains(nxt, state0)) {
            cycle_len = step;
            break;
        }

        dd_edge new_visited = visited + nxt;
        if (new_visited == visited) {
            // Fixpoint reached: s0 cannot reach s0 within S_EG -> proceed to Case B
            break;
        }
        visited = new_visited;
    }

    std::vector<std::vector<int>> stem_path;
    std::vector<std::vector<int>> loop_path;
    int loop_len = -1;
    size_t k = 0;

    if (cycle_len > 0) {
        // Case A: minimal cycle from s0 back to s0
        stem_path.push_back(state0);
        k = 0;
        loop_len = cycle_len;
        loop_path.resize(cycle_len + 1);
        loop_path[cycle_len] = state0;
        for (int i = cycle_len; i >= 1; --i) {
            dd_edge dd_curr = dd_from_marking(loop_path[i], get_MDD_forest());
            dd_edge preds = nsf->pre_image(dd_curr);
            preds = MDD_INTERSECT(preds, L[i - 1]);
            get_marking_from_dd(preds, loop_path[i - 1]);
        }
    } else {
        // =====================================================================
        // Phase 2: Case B (s0 is transient)
        // =====================================================================
        // 1. Identify recurrent cycle states C within reachable states 'visited'
        dd_edge C = visited;
        for (int it = 0; it < 500; ++it) {
            dd_edge prev_C = C;
            dd_edge pre_C = nsf->pre_image(C);
            C = MDD_INTERSECT(C, pre_C);
            dd_edge post_C = nsf->post_image(C);
            C = MDD_INTERSECT(C, post_C);
            if (C == prev_C || sat_set_is_empty(C)) {
                break;
            }
        }

        if (!sat_set_is_empty(C)) {
            // 2. Shortest stem from s0 to C using backward BFS
            std::vector<dd_edge> stem_layers;
            stem_layers.push_back(C);
            while (!sat_set_contains(stem_layers.back(), state0) && stem_layers.size() <= 1000) {
                dd_edge pre = nsf->pre_image(stem_layers.back());
                pre = MDD_INTERSECT(pre, S_EG);
                if (sat_set_is_empty(pre) || pre == stem_layers.back()) {
                    break;
                }
                stem_layers.push_back(pre);
            }

            if (sat_set_contains(stem_layers.back(), state0)) {
                size_t stem_len = stem_layers.size() - 1;
                stem_path.resize(stem_len + 1);
                stem_path[0] = state0;
                for (size_t i = 1; i <= stem_len; ++i) {
                    dd_edge dd_curr = dd_from_marking(stem_path[i - 1], get_MDD_forest());
                    dd_edge succs = nsf->post_image(dd_curr);
                    succs = MDD_INTERSECT(succs, stem_layers[stem_len - i]);
                    get_marking_from_dd(succs, stem_path[i]);
                }
                k = stem_len;
                const std::vector<int>& sk = stem_path.back();

                // 3. Shortest loop from sk back to sk using forward BFS
                dd_edge dd_sk = dd_from_marking(sk, get_MDD_forest());
                std::vector<dd_edge> loop_L;
                loop_L.push_back(dd_sk);

                for (int step = 1; step <= 1000; ++step) {
                    dd_edge nxt = nsf->post_image(loop_L.back());
                    nxt = MDD_INTERSECT(nxt, S_EG);
                    if (sat_set_is_empty(nxt)) {
                        break;
                    }
                    loop_L.push_back(nxt);
                    if (sat_set_contains(nxt, sk)) {
                        loop_len = step;
                        break;
                    }
                }

                if (loop_len > 0) {
                    loop_path.resize(loop_len + 1);
                    loop_path[loop_len] = sk;
                    for (int i = loop_len; i >= 1; --i) {
                        dd_edge dd_curr = dd_from_marking(loop_path[i], get_MDD_forest());
                        dd_edge preds = nsf->pre_image(dd_curr);
                        preds = MDD_INTERSECT(preds, loop_L[i - 1]);
                        get_marking_from_dd(preds, loop_path[i - 1]);
                    }
                }
            }
        }
    }

    // Verify fairness constraints on loop_path if any
    if (loop_len > 0 && has_fairness_constraints()) {
        for (const auto& F : fair_sets) {
            bool visited_F = false;
            for (const auto& st : loop_path) {
                if (sat_set_contains(F, st)) {
                    visited_F = true;
                    break;
                }
            }
            if (!visited_F) {
                loop_len = -1; // Fairness condition violated, fall back to simulation walk
                break;
            }
        }
    }

    // =========================================================================
    // Trace Assembly
    // =========================================================================
    if (loop_len > 0) {
        ttn->set_sub_trace1(phi->generateTrace(*this, state0, traceTy));
        TreeTraceNode* curr_ttn = ttn;

        // 1. Stem transitions: stem_path[0] -> stem_path[1] -> ... -> stem_path[k]
        for (size_t i = 1; i <= k; ++i) {
            int tr = find_firing_transition(stem_path[i - 1], stem_path[i]);
            TreeTraceNode* next_ttn = new TreeTraceNode(stem_path[i], traceTy, tr,
                (tr >= 0 && tr < ntr) ? tabt[tr].trans_name : "");
            next_ttn->set_sub_trace1(phi->generateTrace(*this, stem_path[i], traceTy));
            curr_ttn->set_next(next_ttn);
            curr_ttn = next_ttn;
        }

        // 2. Loop transitions: loop_path[0] -> loop_path[1] -> ... -> loop_path[loop_len - 1]
        for (int j = 1; j < loop_len; ++j) {
            int tr = find_firing_transition(loop_path[j - 1], loop_path[j]);
            TreeTraceNode* next_ttn = new TreeTraceNode(loop_path[j], traceTy, tr,
                (tr >= 0 && tr < ntr) ? tabt[tr].trans_name : "");
            next_ttn->set_sub_trace1(phi->generateTrace(*this, loop_path[j], traceTy));
            curr_ttn->set_next(next_ttn);
            curr_ttn = next_ttn;
        }

        // 3. Loop back to state (k + 1)
        int last_tr = find_firing_transition(loop_path[loop_len - 1], stem_path[k]);
        TreeTraceNode* loop_node = new TreeTraceNode(stem_path[k], traceTy, last_tr,
            (last_tr >= 0 && last_tr < ntr) ? tabt[last_tr].trans_name : "");
        loop_node->set_loop_back_step((int)k);
        loop_node->set_sub_trace1(phi->generateTrace(*this, stem_path[k], traceTy));
        curr_ttn->set_next(loop_node);
        ttn->set_circular();
        ttn->set_loop_back_step((int)k);
        return ttn;
    }

    // =========================================================================
    // Fallback: Greedy Simulation Walk
    // =========================================================================
    TreeTraceNode* current_ttn = ttn;
    std::vector<std::vector<int>> visited_states;
    visited_states.push_back(state0);
    std::vector<int> curr_state = state0;

    ttn->set_sub_trace1(phi->generateTrace(*this, state0, traceTy));

    while (visited_states.size() < 10000) {
        dd_edge dd_curr = dd_from_marking(curr_state, get_MDD_forest());
        dd_edge succs(get_MDD_forest());
        if (stutter_EG) {
            succs = get_stuttered_nsf()->post_image(dd_curr);
        } else {
            succs = vNSF->post_image(dd_curr);
        }
        succs = MDD_INTERSECT(succs, S_EG);
        if (sat_set_is_empty(succs)) {
            break;
        }

        std::vector<int> next_state;
        get_marking_from_dd(succs, next_state);
        int tr = find_firing_transition(curr_state, next_state);

        int loop_idx = -1;
        for (size_t i = 0; i < visited_states.size(); ++i) {
            if (visited_states[i] == next_state) {
                loop_idx = (int)i;
                break;
            }
        }

        if (loop_idx >= 0) {
            TreeTraceNode* loop_node = new TreeTraceNode(next_state, traceTy, tr, (tr >= 0 && tr < ntr) ? tabt[tr].trans_name : "");
            loop_node->set_loop_back_step(loop_idx);
            loop_node->set_sub_trace1(phi->generateTrace(*this, next_state, traceTy));
            current_ttn->set_next(loop_node);
            ttn->set_circular();
            ttn->set_loop_back_step(loop_idx);
            break;
        }

        TreeTraceNode* next_ttn = new TreeTraceNode(next_state, traceTy, tr, (tr >= 0 && tr < ntr) ? tabt[tr].trans_name : "");
        next_ttn->set_sub_trace1(phi->generateTrace(*this, next_state, traceTy));
        current_ttn->set_next(next_ttn);
        current_ttn = next_ttn;

        visited_states.push_back(next_state);
        curr_state = next_state;
    }
    return ttn;
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_EU(Formula* top_formula, Formula* phi1, Formula* phi2, const std::vector<int>& state0, TraceType traceTy) {
    TreeTraceNode* ttn = new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
    if (traceTy == TT_COUNTEREXAMPLE) {
        ttn->set_ECTL_cntexample();
        return ttn;
    }

    dd_edge phi1_mdd = phi1->getMDD(*this);
    dd_edge phi2_mdd = phi2->getMDD(*this);
    if (has_fairness_constraints()) {
        phi2_mdd = AND(phi2_mdd, get_fair_states());
    }

    if (sat_set_contains(phi2_mdd, state0)) {
        ttn->set_sub_trace1(phi2->generateTrace(*this, state0, traceTy));
        return ttn;
    }

    std::vector<dd_edge> steps;
    steps.push_back(phi2_mdd);
    while (!sat_set_contains(steps.back(), state0)) {
        dd_edge pre = PREIMG(steps.back(), false);
        pre = MDD_INTERSECT(pre, phi1_mdd);
        if (sat_set_is_empty(pre) || pre == steps.back()) {
            break;
        }
        steps.push_back(pre);
    }

    if (!sat_set_contains(steps.back(), state0)) {
        return ttn;
    }

    TreeTraceNode* current_ttn = ttn;
    std::vector<int> curr_state = state0;
    ttn->set_sub_trace1(phi1->generateTrace(*this, state0, traceTy));

    for (ssize_t step = (ssize_t)steps.size() - 2; step >= 0; step--) {
        dd_edge dd_curr = dd_from_marking(curr_state, get_MDD_forest());
        dd_edge succs = vNSF->post_image(dd_curr);
        succs = MDD_INTERSECT(succs, steps[step]);
        if (sat_set_is_empty(succs)) {
            break;
        }
        std::vector<int> next_state;
        get_marking_from_dd(succs, next_state);
        int tr = find_firing_transition(curr_state, next_state);

        TreeTraceNode* next_ttn = new TreeTraceNode(next_state, traceTy, tr, (tr >= 0 && tr < ntr) ? tabt[tr].trans_name : "");
        if (step > 0) {
            next_ttn->set_sub_trace1(phi1->generateTrace(*this, next_state, traceTy));
        } else {
            next_ttn->set_sub_trace1(phi2->generateTrace(*this, next_state, traceTy));
        }

        current_ttn->set_next(next_ttn);
        current_ttn = next_ttn;
        curr_state = next_state;
    }
    return ttn;
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_ERfair(Formula* top_formula, Formula* phi1, Formula* phi2, const std::vector<int>& state0, TraceType traceTy) {
    dd_edge phi1_mdd = phi1->getMDD(*this);
    dd_edge phi2_mdd = phi2->getMDD(*this);
    dd_edge phi1_and_phi2 = AND(phi1_mdd, phi2_mdd);
    dd_edge eu_part = EUfair(phi2_mdd, phi1_and_phi2);

    if (sat_set_contains(eu_part, state0)) {
        ref_ptr<Formula> phi1_and_phi2_f = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi1), make_ref_ptr<Formula>(phi2), LogicalFormula::CBF_AND);
        return trace_EU(top_formula, phi2, phi1_and_phi2_f.get(), state0, traceTy);
    }
    return trace_EG(top_formula, phi2, state0, traceTy);
}

//-----------------------------------------------------------------------------
// Universal Duals
//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_AX(Formula* top_formula, Formula* phi, const std::vector<int>& state0, TraceType traceTy) {
    if (traceTy == TT_COUNTEREXAMPLE) {
        ref_ptr<Formula> not_phi = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi));
        TreeTraceNode* ttn = trace_EX(top_formula, not_phi.get(), state0, TT_WITNESS);
        ttn->set_trace_type(TT_COUNTEREXAMPLE);
        return ttn;
    }
    return new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_AF(Formula* top_formula, Formula* phi, const std::vector<int>& state0, TraceType traceTy) {
    if (traceTy == TT_COUNTEREXAMPLE) {
        ref_ptr<Formula> not_phi = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi));
        TreeTraceNode* ttn = trace_EG(top_formula, not_phi.get(), state0, TT_WITNESS);
        ttn->set_trace_type(TT_COUNTEREXAMPLE);
        return ttn;
    }
    return new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_AG(Formula* top_formula, Formula* phi, const std::vector<int>& state0, TraceType traceTy) {
    if (traceTy == TT_COUNTEREXAMPLE) {
        ref_ptr<Formula> not_phi = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi));
        TreeTraceNode* ttn = trace_EF(top_formula, not_phi.get(), state0, TT_WITNESS);
        ttn->set_trace_type(TT_COUNTEREXAMPLE);
        return ttn;
    }
    return new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_AU(Formula* top_formula, Formula* phi1, Formula* phi2, const std::vector<int>& state0, TraceType traceTy) {
    if (traceTy == TT_COUNTEREXAMPLE) {
        ref_ptr<Formula> not_phi1 = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi1));
        ref_ptr<Formula> not_phi2 = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi2));
        ref_ptr<Formula> not_both = ctlnew<LogicalFormula>(not_phi1, not_phi2, LogicalFormula::CBF_AND);

        dd_edge not_phi2_mdd = not_phi2->getMDD(*this);
        dd_edge eg_set = EGfair(not_phi2_mdd);

        TreeTraceNode* ttn = nullptr;
        if (sat_set_contains(eg_set, state0)) {
            ttn = trace_EG(top_formula, not_phi2.get(), state0, TT_WITNESS);
        } else {
            ttn = trace_EU(top_formula, not_phi2.get(), not_both.get(), state0, TT_WITNESS);
        }
        ttn->set_trace_type(TT_COUNTEREXAMPLE);
        return ttn;
    }
    return new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_ARfair(Formula* top_formula, Formula* phi1, Formula* phi2, const std::vector<int>& state0, TraceType traceTy) {
    if (traceTy == TT_COUNTEREXAMPLE) {
        ref_ptr<Formula> not_phi1 = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi1));
        ref_ptr<Formula> not_phi2 = ctlnew<LogicalFormula>(make_ref_ptr<Formula>(phi2));
        TreeTraceNode* ttn = trace_EU(top_formula, not_phi1.get(), not_phi2.get(), state0, TT_WITNESS);
        ttn->set_trace_type(TT_COUNTEREXAMPLE);
        return ttn;
    }
    return new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
}

//-----------------------------------------------------------------------------
// Logic Trace Algorithms
//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_NOT(Formula* top_formula, Formula* phi, const std::vector<int>& state0, TraceType traceTy) {
    TreeTraceNode* sub = phi->generateTrace(*this, state0, oppositeTraceType(traceTy));
    TreeTraceNode* ttn = new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
    ttn->set_sub_trace1(sub);
    return ttn;
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_AND(Formula* top_formula, Formula* phi1, Formula* phi2, const std::vector<int>& state0, TraceType traceTy) {
    TreeTraceNode* ttn = new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
    if (traceTy == TT_WITNESS) {
        ttn->set_sub_trace1(phi1->generateTrace(*this, state0, traceTy));
        ttn->set_sub_trace2(phi2->generateTrace(*this, state0, traceTy));
    } else { // TT_COUNTEREXAMPLE
        dd_edge dd1 = phi1->getMDD(*this);
        bool sat1 = sat_set_contains(dd1, state0);
        if (!sat1) {
            ttn->set_sub_trace1(phi1->generateTrace(*this, state0, traceTy));
        } else {
            ttn->set_sub_trace1(phi2->generateTrace(*this, state0, traceTy));
        }
    }
    return ttn;
}

//-----------------------------------------------------------------------------

TreeTraceNode* Context::trace_OR(Formula* top_formula, Formula* phi1, Formula* phi2, const std::vector<int>& state0, TraceType traceTy) {
    TreeTraceNode* ttn = new TreeTraceNode(state0, make_ref_ptr<Formula>(top_formula), traceTy);
    if (traceTy == TT_WITNESS) {
        dd_edge dd1 = phi1->getMDD(*this);
        bool sat1 = sat_set_contains(dd1, state0);
        if (sat1) {
            ttn->set_sub_trace1(phi1->generateTrace(*this, state0, traceTy));
        } else {
            ttn->set_sub_trace1(phi2->generateTrace(*this, state0, traceTy));
        }
    } else { // TT_COUNTEREXAMPLE
        ttn->set_sub_trace1(phi1->generateTrace(*this, state0, traceTy));
        ttn->set_sub_trace2(phi2->generateTrace(*this, state0, traceTy));
    }
    return ttn;
}

//-----------------------------------------------------------------------------






//-----------------------------------------------------------------------------
} // end namespace ctlmdd
