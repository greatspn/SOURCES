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

dd_edge Context::PREIMG(dd_edge f1) const {
    // Note: for non-ergodic RS, the E X true is only valid
    // for non-dead states. Therefore, the E X true case
    // is not different from the general case.
    if (is_false(f1)) // E X False -> false
        return empty_set();

    dd_edge result = vNSF->pre_image(f1);
    return SELECT_REAL(result);
}

//----------------------------------------------------------------------------

dd_edge Context::NON_DEAD() const {
    // dd_edge non_deadlock_states = ctx.vNSF->pre_image(ctx.RS);
    dd_edge non_deadlock_states = PREIMG(RS);
	return non_deadlock_states;
}

dd_edge Context::DEAD() const {
    return NOT(NON_DEAD());
}

//----------------------------------------------------------------------------

// dd_edge EF(dd_edge f1, Context& ctx) {
//     if (ctx.is_true(f1)) // E F true  ->  true
//         return ctx.RS;
//     if (ctx.is_false(f1)) // E F false  ->  false
//         return ctx.empty_set();

//     dd_edge result(ctx.get_MDD_forest());

//     // Number of steps of pre-image before switching to saturation
//     const int NUM_PRESTEPS = 0;
//     result = ctx.SELECT_REAL(f1);
//     for (int i = 0; /*i < NUM_PRESTEPS*/true; i++) {
//         dd_edge prev_r(result);
//         // Y' = Y union (Y * N^-1)
//         result = OR(result, PREIMG(result, ctx), ctx);
//         if (print_intermediate_expr()) {
//             cout << "EF: step=" << i << ",  SAT size=" 
//                  << fixed << result.getCardinality() << endl;
//             cout.unsetf(ios_base::floatfield);
//         }
//         if (result == prev_r)
//             return result; // fixed point reached
//     }
//     // // Solve using backward reachability with saturation
//     // // (slightly more costly than PRE IMAGE + UNION)
//     // apply(REVERSE_REACHABLE_DFS, result, NSF, result);
//     // if (print_intermediate_expr()) {
//     //     cout << "      saturation: ";
//     // }
//     return result;
// }

//----------------------------------------------------------------------------

dd_edge Context::EG(dd_edge f1) const {
    if (is_false(f1)) // E G false -> false
        return empty_set();

    // NOTE: the case E G true is not trivial for non-ergodic RS.
    // Therefore, we leave it with the general case and do not
    // treat it separately.
    dd_edge result(get_MDD_forest());
    dd_edge prev_r(get_MDD_forest());
    f1 = SELECT_REAL(f1);
    result = f1;

    dd_edge deadlock_f1;
    if(stutter_EG) 
        deadlock_f1 = f1 - NON_DEAD();

    size_t n_iters = 0;
    do {
        prev_r = result; // previous iteration result
        result = AND(result, PREIMG(prev_r));
        if(stutter_EG) 
            result = OR(result, deadlock_f1);

        n_iters++;
        if (print_intermediate_expr()) {
            cout << "EG: step=" << n_iters << ",  SAT size=" 
                 << fixed << result.getCardinality() << endl;
            cout.unsetf(ios_base::floatfield);
        }
    }
    while (result != prev_r);

    if (print_intermediate_expr()) {
        cout << "R2 = " << result.getNode() << " in " 
             << n_iters << " iterations." << endl;
    }
    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::EU(dd_edge f1, dd_edge f2) const {
    if (is_false(f2)) // E f1 U false  ->  false
        return empty_set();
    else if (is_true(f2)) // E f1 U true  ->  true
        return RS;

    // if (is_true(f1)) // E true U f2  ->  E F f2
    //     return EF(f2);
    // else 
    if (is_false(f1)) // E false U f2  ->  f2
        return f2;
    
    dd_edge result(get_MDD_forest());
    dd_edge prev_r(get_MDD_forest());
    result = SELECT_REAL(f2);

    size_t n_iters = 0;
    do {
        prev_r = result;
        // R' = R union (F1 intersect (R * N^-1))
        result = OR(result, AND(f1, PREIMG(result)));

        n_iters++;
        if (print_intermediate_expr()) {
            cout << "EU: step=" << n_iters << ",  SAT size=" 
                 << fixed << result.getCardinality() << endl;
            cout.unsetf(ios_base::floatfield);
        }
    }
    while (result != prev_r);

    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::EX(dd_edge f1) const {
    dd_edge result = PREIMG(f1);

    if (stutter_EX) {
        // cout << "## !stuttered EX" << endl;
        dd_edge deadlock_f1 = f1 - NON_DEAD();
        // add self-loops on f1-dead states
        result = OR(result, deadlock_f1);
    }

    return result;
}

//----------------------------------------------------------------------------

dd_edge Context::EF(dd_edge f1) const {
    return EU(RS, f1);
}

//----------------------------------------------------------------------------
// Fair ECTL (explicit)
//----------------------------------------------------------------------------

dd_edge Context::EGfair(dd_edge f1) const
{
    if (!has_fairness_constraints())
        return EG(f1); // use non-fair EG
    if (is_false(f1)) // E G false -> false
        return empty_set();

    // NOTE: the case E G true is not trivial for non-ergodic RS.
    // Therefore, we leave it with the general case and do not
    // treat it separately.
    dd_edge result(get_MDD_forest());
    dd_edge prev_r(get_MDD_forest());
    f1 = SELECT_REAL(f1);
    result = f1;

    dd_edge deadlock_f1;
    if(stutter_EG) {
        deadlock_f1 = f1 - NON_DEAD();
        // select dead fair f1 states
        if (has_fairness_constraints()) 
            for (auto&& F : fair_sets)
                apply(INTERSECTION, deadlock_f1, F, deadlock_f1);
    }

    size_t n_iters = 0;
    do {
        prev_r = result; // previous iteration result
        if (has_fairness_constraints()) {
            for (auto&& F : fair_sets) {
                dd_edge Y = EU(result, AND(F, result));
                result = AND(result, PREIMG(Y));
            }
        }
        else {
            result = AND(result, PREIMG(prev_r));
        }
        if(stutter_EG) 
            result = OR(result, deadlock_f1);

        n_iters++;
        if (print_intermediate_expr()) {
            cout << (has_fairness_constraints() ? "Fair" : "")
                 << "EG: step=" << n_iters << ",  SAT size=" 
                 << fixed << result.getCardinality() << endl;
            cout.unsetf(ios_base::floatfield);
        }
    }
    while (result != prev_r);

    if (print_intermediate_expr()) {
        cout << "R2 = " << result.getNode() << " in " 
             << n_iters << " iterations." << endl;
    }
    return result;
}
// {
//     if (!has_fairness_constraints())
//         return EG(f1); // use non-fair EG

//     dd_edge result(get_MDD_forest());
//     dd_edge prev_r(get_MDD_forest());
//     // dd_edge deadlock_f1 = f1 - ctx.NON_DEAD();

//     result = f1;
//     do {
//         prev_r = result; // previous iteration result
//         for (auto&& F : fair_sets) {
//             dd_edge Y = EU(result, AND(F, result));
//             result = AND(result, EX(Y));
//         }
//         // result = OR(result, deadlock_f1);
//     }
//     while (prev_r.getNode() != result.getNode());

//     return result;
// }

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
    //not (E not f2 U (not f1 and not f2) ) and not EG not f2
    dd_edge not_f1 = NOT(f1);
    dd_edge not_f2 = NOT(f2);

    return AND(NOT(EUfair(not_f2, AND(not_f1, not_f2))),
               NOT(EGfair(not_f2)));
}

//----------------------------------------------------------------------------





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
} // end namespace ctlmdd
