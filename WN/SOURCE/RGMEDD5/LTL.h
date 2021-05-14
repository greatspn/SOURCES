#pragma once

#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <tuple>

// generation of MDDs from HOA representation
#include "rgmedd5.h"
#include <meddly.h>
#include <meddly_expert.h>


namespace ctlmdd {
class Context;
class Formula;

//-----------------------------------------------------------------------------
// Buchi Automata
//-----------------------------------------------------------------------------

typedef unsigned int ba_location_t;

// An edge of a Buchi Automaton
struct ba_edge_t {
    ba_location_t   src_loc;
    ba_location_t   dst_loc;
    Formula        *state_formula;
    bool            is_initial;

    ba_edge_t(ba_location_t _src_loc, ba_location_t _dst_loc, 
              Formula *_state_formula, bool _is_initial);

    ~ba_edge_t();

    ba_edge_t(const ba_edge_t&) = delete;
    ba_edge_t(ba_edge_t&&) = default;
    ba_edge_t& operator=(const ba_edge_t&) = delete;
    ba_edge_t& operator=(ba_edge_t&&) = default;
};

ostream& operator<<(ostream& os, const ba_edge_t& e);

//-----------------------------------------------------------------------------

// Buchi Automata categories
enum BAType { STRONG_BA, WEAK_BA, VERY_WEAK_BA, TERMINAL_BA };

extern const char* g_str_BAType[4];

//-----------------------------------------------------------------------------

struct BuchiAutomaton {
    ba_location_t                            num_locs;
    std::vector<ba_location_t>               init_locs;
    std::list<ba_edge_t>                     edges;
    std::vector<std::vector<ba_location_t>>  accept_loc_sets;
    BAType                                   type = STRONG_BA;


    BuchiAutomaton();
    virtual ~BuchiAutomaton();

    BuchiAutomaton(const BuchiAutomaton&) = delete;
    BuchiAutomaton(BuchiAutomaton&&) = default;
    BuchiAutomaton& operator=(const BuchiAutomaton&) = delete;
    BuchiAutomaton& operator=(BuchiAutomaton&&) = default;

    // Create all the MDDs labeling the Buchi Automata edges
    void pre_compute_subformula_MDDs(Context& ctx);
};

ostream& operator<<(ostream& os, const BuchiAutomaton& ba);

BuchiAutomaton
spot_LTL_to_Buchi_automaton(const char* ltl_formula,
                            const std::vector<Formula*>& atomic_propositions);

//-----------------------------------------------------------------------------

// // Transition system of the product of a Reachability Set with a buchi Automaton
// struct RS_times_BA {
//     dd_edge S0;		// initial states of the TS
//     dd_edge NSF;	// transition relation of the TS
//     std::list<dd_edge> accept_loc_sets; // list of acceptance set MDDs
//     dd_edge RS;     // set of reachable states in the TS

//     RS_times_BA(RSRG* rsrg);
// };

// //-----------------------------------------------------------------------------

// RS_times_BA 
// build_RS_times_BA(Context& ctx, RSRG* rsrg, BuchiAutomaton& ba, dd_edge deadlock);

//-----------------------------------------------------------------------------

// relabel the location level of a MDD, changing from loc to loc_primed
dd_edge mdd_relabel(const dd_edge& mdd, int loc, int loc_primed, forest* forestMxD);


//-----------------------------------------------------------------------------
// LTL implementation as RS times BA synchronized product
//-----------------------------------------------------------------------------

dd_edge
RSxBA_init_states(Context& ctx, BuchiAutomaton& ba);

std::list<dd_edge>
RSxBA_final_sets(Context& ctx, BuchiAutomaton& ba);

dd_edge 
RSxBA_makeNSF(Context& ctx, BuchiAutomaton& ba, dd_edge& deadlock);

dd_edge
RSxBA_postimage(Context& ctx, BuchiAutomaton& ba, const dd_edge& deadlock, const dd_edge& setMxA);

dd_edge
RSxBA_preimage(Context& ctx, BuchiAutomaton& ba, const dd_edge& deadlock, const dd_edge& setMxA);

//-----------------------------------------------------------------------------

class RSxBA_VirtualNSF : public VirtualNSF {
    Context *ctx;
    BuchiAutomaton* ba;
    dd_edge deadlock;
    bool implicit = true;
    dd_edge NSF; // only set when the vNSF is explicit.
public:
    RSxBA_VirtualNSF(Context *_ctx, BuchiAutomaton* _ba, dd_edge& d) 
    /**/: ctx(_ctx), ba(_ba), deadlock(d) { }

    inline void set_explicit(dd_edge& nsf) {
        implicit = false;
        NSF = nsf;
    }

    virtual forest* getForestMxD() const override;
    virtual dd_edge pre_image(const dd_edge& set) const override;
    virtual dd_edge post_image(const dd_edge& set) const override;
    virtual dd_edge forward_reachable(const dd_edge& s0) const override;
};

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
} // namespace ctlmdd

/** =================== debugging utils ========================= **/

/**
 * explanatory assertion msg
*/

#ifndef NDEBUG
#   define m_assert(Expr, Msg) \
    __m_assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define m_assert(Expr, Msg) ;
#endif

inline void __m_assert(const char* expr_str, bool expr, const char* file, int line, std::string msg)
{
    if (!expr)
    {
        std::cerr << "\n====\nAssertion failed:\t" << msg << "\n"
            << "\tExpression:\t" << expr_str << "\n"
            << "\tSource:\t\t" << file << ":" << line << "\n====\n";
        abort();
    }
}


inline void dumpDD(const std::string msg1, MEDDLY::dd_edge dd)
{
    cout << "\n" << msg1 << "\n" << std::endl;
    ostream_output meddout(cout);
    dd.show(meddout, 2);

    cout << "\n--- State-location pairs reachable ---" << std::endl;
    enumerator i(dd);
    int nvar = dd.getForest()->getDomain()->getNumVariables();
    while(i != 0) { // for each marking in the sat set
        int j;
        for(j=1; j <= nvar; j++) { // for each place
            int val = *(i.getAssignments() + j);
            const char* s = dd.getForest()->getDomain()->getVar(j)->getName();
            if(val>=0 && j < nvar)
                cout << s << "(" << val << ") ";
            else if(j == nvar)
                cout << "loc(" << val << ") ";
        }
        ++i;
        cout << endl;
    }
    cout << endl;
}

