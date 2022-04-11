#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <memory>
#include <map>
#include <set>
#include <stack>
#include <cstdint>

#include "rgmedd3.h"
#include "varorders.h"
#include "optimization_finder.h"
// #include "utils/thread_pool.h"
#include "utils/mt64.h"

// Options
size_t g_sim_ann_num_tentatives = 50000;

// Open a file with the default application/browser
int open_file(const char * filename);

//---------------------------------------------------------------------------------------
// Encoding of the Petri net transitions using IOH elements
//---------------------------------------------------------------------------------------

ostream& operator<< (ostream& os, const ioh_t& x) {
    if (x.has_input()) os << x.input;
    os << "/";
    if (x.has_output()) os << x.output;
    os << "/";
    if (x.has_inhib()) os << x.inhib;
    return os;
}

//---------------------------------------------------------------------------------------

ostream& operator<< (ostream& os, const var_ioh_t& x) {
    return os << ((const ioh_t&)x) << "@" << x.level;
}

//---------------------------------------------------------------------------------------

void trans_span_t::build_for_trn(int tr) {
    trn = tr;
    std::map<int, ioh_t> en;
    for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr))
        en[GET_PLACE_INDEX(in_ptr)].input = in_ptr->molt;
    for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr))
        en[GET_PLACE_INDEX(out_ptr)].output = out_ptr->molt;
    for (Node_p inh_ptr = GET_INHIBITOR_LIST(tr); inh_ptr; inh_ptr = NEXT_NODE(inh_ptr))
        en[GET_PLACE_INDEX(inh_ptr)].inhib = inh_ptr->molt;

    entries.reserve(en.size());
    sorted_entries.reserve(en.size());
    for (auto&& e : en) {
        // if (tr == 4420)
        //     cout << var_ioh_t{ e.second.input, e.second.output, e.second.inhib, e.first } << "  plc=" << e.first << endl;

        assert(0 <= e.first && e.first < npl);
        entries.push_back(var_ioh_t{ e.second.input, e.second.output, e.second.inhib, e.first });
    }
}

//---------------------------------------------------------------------------------------

// Rebuild the linked list of the IOH entries of a transition according to the variable order
void trans_span_t::build_ioh_list(const std::vector<int> &varorder, bool test_productiveness) {
    if (entries.empty()) 
        return;

    sorted_entries.resize(entries.size());
    for (size_t i=0; i<entries.size(); i++) { // Assign the level of each entry
        assert(0 <= entries[i].place && entries[i].place < npl);
        entries[i].level = varorder[ entries[i].place ];
        entries[i].is_productive = true;
        sorted_entries[i] = &entries[i];
    }
    std::sort(sorted_entries.begin(), sorted_entries.end(),
              [](const var_ioh_t* e1, const var_ioh_t* e2) -> bool {
                  assert(e1->level >= 0 && e1->level < npl && e2->level >= 0 && e2->level < npl);
                  return e1->level < e2->level;
              });

    // Initialize the max_unique_above field as the height of the zero IOH above
    // each IOH entry. Top entry has a max-above of zero.
    sorted_entries.back()->max_unique_above = 0; // top level
    for (ssize_t i=sorted_entries.size() - 2; i>=0; i--) {
        sorted_entries[i]->max_unique_above = sorted_entries[i+1]->level - sorted_entries[i]->level - 1;
    }

    // Evaluate non-productive arcs
    if (test_productiveness) {
        sorted_entries.front()->is_productive = sorted_entries.front()->is_productive_entry();

        // Mark as non-productive from the bottom up, until we find a productive arc.
        for (size_t i=1; i<sorted_entries.size(); i++) {
            if (sorted_entries[i - 1]->is_productive)
                break; // productive from here to the top
            sorted_entries[i]->is_productive = sorted_entries[i]->is_productive_entry();
        }
    }
}

//---------------------------------------------------------------------------------------

ostream& operator<< (ostream& os, const trans_span_t& x) {
    os << setw(12)<<tabt[x.trn].trans_name
       <<" (top="<<x.top()<<", bot="<<x.bot()<<")";
    for (auto&& e : x.entries)
        os << " " << e;
    return os;
}

//---------------------------------------------------------------------------------------

static const size_t s_primes_for_hash[] = {
    53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843, 
    50331653, 100663319, 201326611, 402653189, 805306457, 1610612741, 
};
static const size_t NUM_HASH_PRIMES = sizeof(s_primes_for_hash) / sizeof(s_primes_for_hash[0]);

//---------------------------------------------------------------------------------------

std::shared_ptr<trans_span_set_t> make_trans_span_set() {
    return std::make_shared<trans_span_set_t>();
}

//---------------------------------------------------------------------------------------

ostream& operator<< (ostream& os, const trans_span_set_t& x) {
    for (auto&& e : x.trns)
        os << e << endl;
    return os;
}

//---------------------------------------------------------------------------------------

void trans_span_set_t::initialize(bool verbose) {
    trns.resize(ntr);
    for (int tr=0; tr<ntr; tr++) {
        trns[tr].build_for_trn(tr);
        num_arcs += trns[tr].entries.size();

        for (auto&& ioh : trns[tr].entries) {
            if (ioh.has_inhib()) num_inhib++;
            if (ioh.has_input() && ioh.has_output()) num_test++;
            else if (ioh.has_input()) num_in++;
            else if (ioh.has_output()) num_out++;
            assert(ioh.place >= 0 && ioh.place < npl);
        }
    }
    if (!running_for_MCC() && verbose) {
        cout << "INPUT ARCS:  " << num_in << endl;
        cout << "OUTPUT ARCS: " << num_out << endl;
        cout << "INHIB ARCS:  " << num_inhib << endl;
        cout << "TEST ARCS:   " << num_test << endl;
        cout << "PLACES:      " << npl << endl;
        cout << "TRANSITIONS: " << ntr << endl;
    }
}

//---------------------------------------------------------------------------------------

void trans_span_set_t::build_all_ioh_lists(const std::vector<int> &varorder, bool test_productiveness) {
    if (finalized)
        throw rgmedd_exception("cannot rebuild a finalized transition set.");
    // Build the linked list of each transition entries
    for (int tr=0; tr<ntr; tr++)
        trns[tr].build_ioh_list(varorder, test_productiveness);

    size_t i = 0;
    while (i < NUM_HASH_PRIMES && s_primes_for_hash[i] < num_arcs/2)
        i++;
    hash_table.resize(s_primes_for_hash[i]);
}

//---------------------------------------------------------------------------------------

void trans_span_set_t::compute_unique_productive_nodes(bool sort_transitions, bool activation_lvls)
{
    if (finalized)
        throw rgmedd_exception("cannot rebuild a finalized transition set.");
    // build_all_ioh_lists(varorder);
    assert(!hash_table.empty());

    if (sort_transitions) {
        // Sort the transition, s.t. shared entries will appear on the right.
        // This is useful for the soups representation, but it is not
        // needed for intensive soups computations, where it can be skipped.
        sorted_trns.resize(ntr);
        for (int tr=0; tr<ntr; tr++)
            sorted_trns[tr] = &trns[tr];

        // Sort transitions according to their top levels
        std::sort(sorted_trns.begin(), sorted_trns.end(), 
                  [](const trans_span_t *t1, const trans_span_t *t2) -> bool { 
            return ( (t1->top() < t2->top()) || 
                     (t1->top() == t2->top() && t1->trn < t2->trn) ); 
        });
    }
    else sorted_trns.resize(0);

    std::fill(hash_table.begin(), hash_table.end(), nullptr);
    signatures = 1; // Uniquely assigned signatures

    // Assign a unique signature to each unique node.
    // Mark the remaining entries as shared
    for (size_t tr=0; tr<ntr; tr++) {
        const trans_span_t& trspan = (sort_transitions ? *sorted_trns[tr] : trns[tr]);
        size_t prev_signature = 0;

        for (var_ioh_t* p_elem : trspan.sorted_entries) {
            assert(0 <= p_elem->level && p_elem->level < npl);
            // Exclude non-productive entries from the node signature.
            // In this way, two spans that share the same enabling but terminate with 
            // distinct non-productive arcs will share the same DAG node.
            // if (!p_elem->is_productive) {
            //     p_elem->signature = 0;
            //     p_elem->is_unique = false;
            //     p_elem->hash_next = nullptr;
            //     continue;
            // }
            p_elem->compute_hash(prev_signature);
            p_elem->is_unique = true;
            // Search in the hash table if we have already assigned 
            // a unique signature for p_elem and its bottom entries
            const size_t hh = p_elem->hash % hash_table.size();
            var_ioh_t* h_chain = hash_table[hh];
            while (h_chain) {
                if (p_elem->is_same(*h_chain)) { // shared
                    p_elem->signature = h_chain->signature;
                    p_elem->is_unique = false;
                    p_elem->hash_next = nullptr;
                    // the node h_chain is shared among different transitions.
                    // Update its max_unique_above as the maximum number of IOH entries
                    // of all the merged entries.
                    h_chain->max_unique_above = std::max(h_chain->max_unique_above,
                                                         p_elem->max_unique_above);
                    break;
                }
                h_chain = h_chain->hash_next;
            }

            if (p_elem->is_unique) { // unique
                p_elem->signature = signatures++; // new signature
                p_elem->hash_next = hash_table[hh];
                hash_table[hh] = p_elem;
            }

            prev_signature = p_elem->signature;
        }
    }

    if (activation_lvls) {
        assert(sort_transitions);
        down_active.resize(npl);
        int min_down = npl;
        for (ssize_t tr=ntr-1; tr>=0; tr--) {
            const trans_span_t *trspan = sorted_trns[tr];
            for (var_ioh_t* p_elem : trspan->sorted_entries) {
                if (p_elem->is_unique) {
                    min_down = std::min(min_down, p_elem->level);
                    down_active[trspan->top()] = min_down;
                    break;
                }
            }
        }
    }
    else down_active.resize(0);
}

//---------------------------------------------------------------------------------------

void trans_span_set_t::compute_level_boundaries() {
    assert(!sorted_trns.empty());
    uf.initialize();
    for (size_t tr=0; tr<ntr; tr++) {
        const trans_span_t& trspan = *sorted_trns[tr];
        if (trspan.entries.empty() || !trspan.is_top_unique_productive())
            continue;

        size_t top = trspan.top();
        size_t up_bot = top;

        // Visit the transition entries top to bottom
        for (ssize_t i=trspan.sorted_entries.size() - 1; i>=0; i--) {
            const var_ioh_t* p_elem = trspan.sorted_entries[i];
            if (p_elem->is_unique && p_elem->is_productive)
                up_bot = p_elem->level;
            else break;
        }
        // Merge the sets in range [top, up_bot]
        if (uf.merge(up_bot, top)) {
            for (ssize_t i=up_bot; i<top; i++)
                uf.merge(i, top);
        }
            // if (uf.merge(i, top))
            //     break;
    }
    // for (size_t i=0; i<npl; i++) 
    //     cout << uf.find(i) << " ";
    // cout << endl;
}

//---------------------------------------------------------------------------------------

// void trans_span_set_t::compute_unique_productive_nodes2(bool sort_transitions, bool activation_lvls)
// {
//     if (finalized)
//         throw rgmedd_exception("cannot rebuild a finalized transition set.");
//     // build_all_ioh_lists(varorder);
//     assert(!hash_table.empty());

//     if (sort_transitions) {
//         // Sort the transition, s.t. shared entries will appear on the right.
//         // This is useful for the soups representation, but it is not
//         // needed for intensive soups computations, where it can be skipped.
//         sorted_trns.resize(ntr);
//         for (int tr=0; tr<ntr; tr++)
//             sorted_trns[tr] = &trns[tr];

//         // Sort transitions according to their top levels
//         std::sort(sorted_trns.begin(), sorted_trns.end(), 
//                   [](const trans_span_t *t1, const trans_span_t *t2) -> bool { 
//             return ( (t1->top() < t2->top()) || 
//                      (t1->top() == t2->top() && t1->trn < t2->trn) ); 
//         });
//     }
//     else sorted_trns.resize(0);

//     std::fill(hash_table.begin(), hash_table.end(), nullptr);
//     signatures = 1; // Uniquely assigned signatures

//     // Assign a unique signature to each unique node.
//     // Mark the remaining entries as shared
//     for (size_t tr=0; tr<ntr; tr++) {
//         const trans_span_t& trspan = (sort_transitions ? *sorted_trns[tr] : trns[tr]);
//         size_t prev_signature = 0;
//         bool prev_is_productive = false;
//         bool prev_is_unique = false;

//         for (var_ioh_t* p_elem : trspan.sorted_entries) {
//             assert(0 <= p_elem->level && p_elem->level < npl);
//             // Exclude non-productive entries from the node signature.
//             // In this way, two spans that share the same enabling but terminate with 
//             // distinct non-productive arcs will share the same DAG node.
//             bool is_productive = prev_is_unique || p_elem->is_productive_entry();
//             if (!is_productive) {
//                 p_elem->signature = 0;
//                 p_elem->is_unique = false;
//                 p_elem->hash_next = nullptr;
//                 p_elem->is_productive = is_productive;
//             }
//             else {
//                 p_elem->compute_hash(prev_signature);
//                 p_elem->is_unique = true;
//                 // Search in the hash table if we have already assigned 
//                 // a unique signature for p_elem and its bottom entries
//                 const size_t hh = p_elem->hash % hash_table.size();
//                 var_ioh_t* h_chain = hash_table[hh];
//                 while (h_chain) {
//                     if (p_elem->is_same(*h_chain)) {
//                         p_elem->signature = h_chain->signature;
//                         p_elem->is_unique = false;
//                         p_elem->hash_next = nullptr;
//                         p_elem->is_productive = is_productive;
//                         break;
//                     }
//                     h_chain = h_chain->hash_next;
//                 }

//                 if (p_elem->is_unique) {
//                     p_elem->signature = signatures++; // new signature
//                     p_elem->hash_next = hash_table[hh];
//                     hash_table[hh] = p_elem;
//                     p_elem->is_productive = is_productive;
//                 }
//                 prev_signature = p_elem->signature;
//             }

//             prev_is_unique = p_elem->is_unique;
//             prev_is_productive = (is_productive || prev_is_unique);
//         }
//     }

//     // if (activation_lvls) {
//     //     assert(sort_transitions);
//     //     down_active.resize(npl);
//     //     int min_down = npl;
//     //     for (ssize_t tr=ntr-1; tr>=0; tr--) {
//     //         const trans_span_t *trspan = sorted_trns[tr];
//     //         for (var_ioh_t* p_elem : trspan->sorted_entries) {
//     //             if (p_elem->is_unique) {
//     //                 min_down = std::min(min_down, p_elem->level);
//     //                 down_active[trspan->top()] = min_down;
//     //                 break;
//     //             }
//     //         }
//     //     }
//     // }
//     // else down_active.resize(0);
// }

//---------------------------------------------------------------------------------------

// Build the trns_by_level[] linked lists, where each list contains the transitions
// that start in a certain level
void trans_span_set_t::build_trn_list_by_level() {
    assert(!sorted_trns.empty() && !finalized);
    trns_by_level.resize(npl);
    std::fill(trns_by_level.begin(), trns_by_level.end(), nullptr);

    for (size_t tr=0; tr<ntr; tr++) {
        trans_span_t* trspan = &trns[tr];
        trspan->next_at_level = trns_by_level[ trspan->top() ];
        trns_by_level[ trspan->top() ] = trspan;
    }
}

//---------------------------------------------------------------------------------------

// Sum of Unique and Productive Spans
uint64_t measure_soups(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                       bool test_productiveness /* = true*/) 
{
    trn_set.build_all_ioh_lists(varorder, test_productiveness /* mark non-productive nodes */);
    trn_set.compute_unique_productive_nodes(false /* no need to sort transitions */, false); 
    // const std::vector<const trans_span_t*>& all_trns = trn_set.sorted_trns;
    const std::vector<trans_span_t>& all_trns = trn_set.trns;

    uint64_t SOUPS = 0;
    for (size_t tr=0; tr<ntr; tr++) {
        const trans_span_t& trspan = all_trns[tr];
        if (trspan.entries.empty())
            continue;
        uint64_t UPS = 0; // Weighted Unique Productive Span of transition tr

        // Visit the transition entries top to bottom
        for (ssize_t i=trspan.sorted_entries.size() - 1; i>=0; i--) {
            const var_ioh_t* p_elem = trspan.sorted_entries[i];
            if (p_elem->is_unique && p_elem->is_productive) {
                UPS += 1 + p_elem->max_unique_above;
            }
            else break;
        }
        SOUPS += UPS;
    }

    return SOUPS;
}

//---------------------------------------------------------------------------------------

// Sum of Unique and Productive Spans
uint64_t measure_soups2(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                        bool test_productiveness) 
{
    trn_set.build_all_ioh_lists(varorder, test_productiveness /* mark non-productive nodes */);
    trn_set.compute_unique_productive_nodes(true /*sort transitions */, false); 
    trn_set.compute_level_boundaries();
    const std::vector<const trans_span_t*>& all_trns = trn_set.sorted_trns;
    // const std::vector<trans_span_t>& all_trns = trn_set.trns;

    uint64_t SOUPS = 0;
    for (size_t tr=0; tr<ntr; tr++) {
        const trans_span_t& trspan = *all_trns[tr];
        if (trspan.entries.empty())
            continue;
        uint64_t UPS = 0; // Weighted Unique Productive Span of transition tr

        // Visit the transition entries top to bottom
        int top_set = trn_set.uf.find(trspan.top());
        for (ssize_t i=trspan.sorted_entries.size() - 1; i>=0; i--) {
            const var_ioh_t* p_elem = trspan.sorted_entries[i];
            if (p_elem->is_unique && p_elem->is_productive) {
                const size_t lvl = p_elem->level;
                for (ssize_t i=p_elem->max_unique_above; i > 0; i--) {
                    if (trn_set.uf.find(lvl + i) == top_set)
                        ++UPS;
                    else break;
                }
                if (trn_set.uf.find(lvl) == top_set)
                    ++UPS;
                else break;
            }
            else break;
        }
        SOUPS += UPS;
    }

    return SOUPS;
}

//---------------------------------------------------------------------------------------

// Soups Weighted by Invariant Ranks (SWIR)
template<typename SCORE, typename WEIGHT>
SCORE measure_swir_base(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                        const std::vector<WEIGHT>& lvl_weights, bool test_productiveness) 
{
    trn_set.build_all_ioh_lists(varorder, test_productiveness /* mark non-productive nodes */);
    trn_set.compute_unique_productive_nodes(false /* no need to sort transitions */, false); 
    // const std::vector<const trans_span_t*>& all_trns = trn_set.sorted_trns;
    const std::vector<trans_span_t>& all_trns = trn_set.trns;

    SCORE SWIR = 0;
    for (size_t tr=0; tr<ntr; tr++) {
        const trans_span_t& trspan = all_trns[tr];
        if (trspan.entries.empty())
            continue;
        SCORE wUPS = 0; // Weighted Unique Productive Span of transition tr

        // Visit the transition entries top to bottom
        for (ssize_t i=trspan.sorted_entries.size() - 1; i>=0; i--) {
            const var_ioh_t* p_elem = trspan.sorted_entries[i];
            if (p_elem->is_unique && p_elem->is_productive) {
                const size_t lvl = p_elem->level;
                for (size_t i=p_elem->max_unique_above; i>0; i--)
                    wUPS += lvl_weights[lvl + i];
                wUPS += lvl_weights[lvl];
            }
            else break;
        }
        // cout << "   " << tabt[trspan.trn].trans_name << "  swir=" << wUPS << endl;
        SWIR += wUPS;
    }

    return SWIR;
}

//---------------------------------------------------------------------------------------

uint64_t
measure_swir(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
             const std::vector<size_t>& lvl_weights, bool test_productiveness /* = true*/)
{ return measure_swir_base<uint64_t, size_t>(varorder, trn_set, lvl_weights, test_productiveness); }

double
measure_swir(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
             const std::vector<double>& lvl_weights, bool test_productiveness /* = true*/)
{ return measure_swir_base<double, double>(varorder, trn_set, lvl_weights, test_productiveness); }

cardinality_t
measure_swir(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
             const std::vector<cardinality_t>& lvl_weights, bool test_productiveness /* = true*/)
{ return measure_swir_base<cardinality_t, cardinality_t>(varorder, trn_set, lvl_weights, test_productiveness); }

//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------

// Soups Weighted by Invariant Ranks (SWIR)
template<typename SCORE, typename WEIGHT>
SCORE measure_swir2_base(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                         const std::vector<WEIGHT>& lvl_weights, bool test_productiveness) 
{
    trn_set.build_all_ioh_lists(varorder, test_productiveness /* mark non-productive nodes */);
    trn_set.compute_unique_productive_nodes(true /* sort transitions */, false); 
    trn_set.compute_level_boundaries();
    const std::vector<const trans_span_t*>& all_trns = trn_set.sorted_trns;
    // const std::vector<trans_span_t>& all_trns = trn_set.trns;

    SCORE SWIR = 0;
    for (size_t tr=0; tr<ntr; tr++) {
        const trans_span_t& trspan = *all_trns[tr];
        if (trspan.entries.empty())
            continue;
        SCORE wUPS = 0; // Weighted Unique Productive Span of transition tr

        // Visit the transition entries top to bottom
        int top_set = trn_set.uf.find(trspan.top());
        for (ssize_t i=trspan.sorted_entries.size() - 1; i>=0; i--) {
            const var_ioh_t* p_elem = trspan.sorted_entries[i];
            if (p_elem->is_unique && p_elem->is_productive) {
                const size_t lvl = p_elem->level;
                for (size_t i=p_elem->max_unique_above; i>0; i--) {
                    if (trn_set.uf.find(lvl + i) == top_set)
                        wUPS += lvl_weights[lvl + i];
                    else break;
                }
                if (trn_set.uf.find(lvl) == top_set)
                    wUPS += lvl_weights[lvl];
                else break;
            }
            else break;
        }
        // cout << "   " << tabt[trspan.trn].trans_name << "  swir=" << wUPS << endl;
        SWIR += wUPS;
    }

    return SWIR;
}

//---------------------------------------------------------------------------------------

uint64_t
measure_swir2(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
              const std::vector<size_t>& lvl_weights, bool test_productiveness /* = true*/)
{ return measure_swir2_base<uint64_t, size_t>(varorder, trn_set, lvl_weights, test_productiveness); }

double
measure_swir2(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
              const std::vector<double>& lvl_weights, bool test_productiveness /* = true*/)
{ return measure_swir2_base<double, double>(varorder, trn_set, lvl_weights, test_productiveness); }

cardinality_t
measure_swir2(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
              const std::vector<cardinality_t>& lvl_weights, bool test_productiveness /* = true*/)
{ return measure_swir2_base<cardinality_t, cardinality_t>(varorder, trn_set, lvl_weights, test_productiveness); }

//---------------------------------------------------------------------------------------

// // Sum of Unique and Productive Spans
// uint64_t measure_soups3(const std::vector<int> &varorder, trans_span_set_t& trn_set) 
// {
//     trn_set.build_all_ioh_lists(varorder, true /* mark non-productive nodes */);
//     trn_set.compute_unique_productive_nodes(false /* no need to sort transitions */, false); 
//     // const std::vector<const trans_span_t*>& all_trns = trn_set.sorted_trns;
//     const std::vector<trans_span_t>& all_trns = trn_set.trns;
//     uint64_t maxPerLevel[npl];
//     std::fill(maxPerLevel, maxPerLevel + npl, uint64_t(0));

//     uint64_t SOUPS = 0;
//     for (size_t tr=0; tr<ntr; tr++) {
//         const trans_span_t& trspan = all_trns[tr];
//         if (trspan.entries.empty())
//             continue;
//         uint64_t UPS = 0; // Weighted Unique Productive Span of transition tr

//         // Visit the transition entries top to bottom
//         for (ssize_t i=trspan.sorted_entries.size() - 1; i>=0; i--) {
//             const var_ioh_t* p_elem = trspan.sorted_entries[i];
//             if (p_elem->is_unique && p_elem->is_productive) {
//                 UPS += 1 + p_elem->max_unique_above;
//             }
//             else break;
//         }
//         maxPerLevel[trspan.top()] = std::max(maxPerLevel[trspan.top()], UPS);
//     }
//     SOUPS = std::accumulate(maxPerLevel, maxPerLevel + npl, uint64_t(0));

//     return SOUPS;
// }

//---------------------------------------------------------------------------------------

// Sum of Unique and Productive Spans
// uint64_t
// measure_soups_X(const std::vector<int> &varorder, trans_span_set_t& trn_set) 
// {
//     trn_set.build_all_ioh_lists(varorder, true /* mark non-productive nodes */);
//     trn_set.compute_unique_productive_nodes(true, true); 
//     const std::vector<const trans_span_t*>& all_trns = trn_set.sorted_trns;

//     uint64_t SOUPS = 0;
//     for (size_t tr=0; tr<ntr; tr++) {
//         const trans_span_t& trspan = *all_trns[tr];
//         if (trspan.entries.empty())
//             continue;
//         uint64_t UPS = 0; // Weighted Unique Productive Span of transition tr
//         int prev_lvl = trspan.sorted_entries[0]->level - 1;
//         bool prev_productive = true, prev_unique = true;
//         for (var_ioh_t* p_elem : trspan.sorted_entries) {
//             // cout << "(" << (prev_lvl+1) << "-" << p_elem->level << "):" << (prev_productive?"P":"") << (prev_unique?"U":"");
//             // Intermediate levels in range [prev_lvl + 1,  p_elem->level]
//             // Count these level if the current is unique and the previous is productive
//             if (p_elem->is_unique && prev_productive) {
//                 if (prev_unique) {
//                     UPS += (p_elem->level - prev_lvl - 1);
//                     // cout << "{"<<(p_elem->level - prev_lvl - 1)<<"}";
//                 }
//                 else {
//                     assert(trn_set.down_active[trspan.top()] <= p_elem->level);
//                     int activation_lvl = trn_set.down_active[trspan.top()] - 1;
//                     // cout << " [A="<<activation_lvl<<"]";
//                     UPS += (p_elem->level - std::max(activation_lvl, prev_lvl) - 1);
//                     // cout << "{"<<(p_elem->level - std::max(activation_lvl, prev_lvl) - 1)<<"}";
//                 }
//                 // cout << "*";
//             }

//             // Add this level if it is unique and productive
//             if (p_elem->is_unique) {
//                 // Weight the level using the # of active invariants
//                 UPS ++;
//                 // cout << "#";
//             }
//             // cout << " -> ";
//             // Next iteration
//             prev_lvl = p_elem->level;
//             prev_productive = p_elem->is_productive;
//             prev_unique = p_elem->is_unique;
//         }
//         // cout << setw(15) << tabt[trspan.trn].trans_name << "  " << UPS << endl;
//         SOUPS += UPS;
//     }

//     return SOUPS;
// }

//---------------------------------------------------------------------------------------

// // Soups Weighted by Invariant Ranks (SWIR) variant 3
// template<typename SCORE, typename WEIGHT>
// SCORE measure_swir3_base(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//                         const std::vector<WEIGHT>& lvl_weights) 
// {
//     trn_set.build_all_ioh_lists(varorder, true /* mark non-productive nodes */);
//     trn_set.compute_unique_productive_nodes(false /* no need to sort transitions */, false); 
//     // const std::vector<const trans_span_t*>& all_trns = trn_set.sorted_trns;
//     const std::vector<trans_span_t>& all_trns = trn_set.trns;
//     SCORE maxPerLevel[npl];
//     std::fill(maxPerLevel, maxPerLevel + npl, SCORE(0));

//     SCORE SWIR = 0;
//     for (size_t tr=0; tr<ntr; tr++) {
//         const trans_span_t& trspan = all_trns[tr];
//         if (trspan.entries.empty())
//             continue;
//         SCORE wUPS = 0; // Weighted Unique Productive Span of transition tr

//         // Visit the transition entries top to bottom
//         for (ssize_t i=trspan.sorted_entries.size() - 1; i>=0; i--) {
//             const var_ioh_t* p_elem = trspan.sorted_entries[i];
//             if (p_elem->is_unique && p_elem->is_productive) {
//                 const size_t lvl = p_elem->level;
//                 for (size_t i=p_elem->max_unique_above; i>0; i--)
//                     wUPS += lvl_weights[lvl + i];
//                 wUPS += lvl_weights[lvl];
//             }
//             else break;
//         }
//         // cout << "   " << tabt[trspan.trn].trans_name << "  swir=" << wUPS << endl;
//         // SWIR += wUPS;
//         maxPerLevel[trspan.top()] = std::max(maxPerLevel[trspan.top()], wUPS);
//     }
//     SWIR = std::accumulate(maxPerLevel, maxPerLevel + npl, SCORE(0));

//     return SWIR;
// }

//---------------------------------------------------------------------------------------

// uint64_t
// measure_swir3(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//               const std::vector<size_t>& lvl_weights)
// { return measure_swir3_base<uint64_t, size_t>(varorder, trn_set, lvl_weights); }

// double
// measure_swir3(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//               const std::vector<double>& lvl_weights)
// { return measure_swir3_base<double, double>(varorder, trn_set, lvl_weights); }

// cardinality_t
// measure_swir3(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//               const std::vector<cardinality_t>& lvl_weights)
// { return measure_swir3_base<cardinality_t, cardinality_t>(varorder, trn_set, lvl_weights); }

//---------------------------------------------------------------------------------------

double get_soups_discount(const trans_span_set_t &trns_set) {
    // Compute a WES-like discount factor for SOUPS-based metrics
    // Level that do not activate any transitions are discounted,
    // and the discount increases for transitions closer to the top.
    const double MIN_THRESHOLD = 0.55;
    bool level_has_trn[npl];
    std::fill(level_has_trn, level_has_trn+npl, false);

    for (const trans_span_t& t : trns_set.trns) {
        if (t.is_top_unique_productive() && !level_has_trn[t.top()]) {
            level_has_trn[t.top()] = true;
        }
    }
    double discount = 0;
    for (size_t lvl=0; lvl < npl; lvl++) {
        if (level_has_trn[lvl]) 
            discount += pow(lvl / (npl / 2.), 1.5);
    }
    discount /= npl; 

    // if (!running_for_MCC())
    //     cout << "  DISCOUNT = " << discount << endl;
    if (discount < MIN_THRESHOLD)
        return pow(discount, 1.5);// * discount;
    return 1.0;
}

//---------------------------------------------------------------------------------------
// Representation of SOUPS/SWIR in PDF form
//---------------------------------------------------------------------------------------

// Charachter width of the Postscript "Times" font, 72 points.
static const uint8_t s_ascii_char_widths_Times72pt[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    18, 24, 29, 36, 36, 60, 56, 13, 24, 24, 36, 41, 18, 24, 18, 20, 
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 20, 20, 41, 41, 41, 32, 
    66, 52, 48, 48, 52, 44, 40, 52, 52, 24, 28, 52, 44, 64, 52, 52, 
    40, 52, 48, 40, 44, 52, 52, 68, 52, 52, 44, 24, 20, 24, 34, 36, 
    24, 32, 36, 32, 36, 32, 24, 36, 36, 20, 20, 36, 20, 56, 36, 36, 
    36, 36, 24, 28, 20, 36, 36, 52, 36, 36, 32, 35, 14, 35, 39, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    18, 24, 36, 36, 36, 36, 14, 36, 24, 55, 20, 36, 41, 24, 55, 24, 
    29, 40, 22, 22, 24, 41, 33, 18, 24, 22, 22, 36, 54, 54, 54, 32, 
    52, 52, 52, 52, 52, 52, 64, 48, 44, 44, 44, 44, 24, 24, 24, 24, 
    52, 52, 52, 52, 52, 52, 52, 41, 52, 52, 52, 52, 52, 52, 40, 36, 
    32, 32, 32, 32, 32, 32, 48, 32, 32, 32, 32, 32, 20, 20, 20, 20, 
    36, 36, 36, 36, 36, 36, 36, 40, 36, 36, 36, 36, 36, 36, 36, 36, 
};

//---------------------------------------------------------------------------------------

// Make an estimate of an ASCII string width in Times Roman, given the font points
// Kerning is ignored, so the value is just a (very close) upper-bound of the width.
double EPS_string_width(const char* str, int pts) {
    double w = 0;
    while (*str != '\0') {
        w += s_ascii_char_widths_Times72pt[*str++] * (pts / 72.0);
    }
    return w;
}

//---------------------------------------------------------------------------------------

// Write the diagram of the transition spans in EPS format
void write_incidence_as_EPS(const char* filename, const trans_span_set_t &trn_set,
                            const std::vector<int> &net_to_level, const flow_basis_t& basis, 
                            int *out_width, int *out_height,
                            const std::vector<std::vector<std::string>>* rangeMat,
                            LevelInfoEPS* ppLvlInfo[], const size_t numLvlInfo,
                            const DDEPS* ddeps)
{
    assert(!trn_set.sorted_trns.empty());
    const int TIMES_FONT_SIZE = 8;
    const double PFLOW_FONT_SIZE = TIMES_FONT_SIZE * 0.8;
    // Get the maximum size of the place and transition names
    double max_plc_width = 0, max_trn_width = 0;
    for (int pl=0; pl<npl; pl++)
        max_plc_width = max(max_plc_width, EPS_string_width(tabp[pl].place_name, TIMES_FONT_SIZE));
    for (int tr=0; tr<ntr; tr++)
        max_trn_width = max(max_trn_width, EPS_string_width(tabt[tr].trans_name, TIMES_FONT_SIZE));
    
    int level_to_net[net_to_level.size()];
    for (int p=0; p<net_to_level.size(); p++)
        level_to_net[ net_to_level[p] ] = p;

    // Size of extended informations
    // const double EM = EPS_string_width("M", TIMES_FONT_SIZE);
    // const double ONE = EPS_string_width("1", TIMES_FONT_SIZE);
    double max_NxL_width = 0, max_SxL_width = 0, max_LvlW_width = 0;
    int node_count = 0;
    for (size_t l=0; l<numLvlInfo; l++) {
        ppLvlInfo[l]->width = EPS_string_width(ppLvlInfo[l]->header.c_str(), TIMES_FONT_SIZE);
        ppLvlInfo[l]->width = max(EPS_string_width(ppLvlInfo[l]->footer.c_str(), TIMES_FONT_SIZE), ppLvlInfo[l]->width);
        for (auto&& e : ppLvlInfo[l]->info)
            ppLvlInfo[l]->width = max(EPS_string_width(e.c_str(), TIMES_FONT_SIZE), ppLvlInfo[l]->width);
    }
    double iMatW = 10; // Invariant matrix cell width
    if (rangeMat != nullptr) {
        double rangeW = 0;
        for (auto&& r : *rangeMat)
            for (auto&& e : r)
                rangeW = max(rangeW, EPS_string_width(e.c_str(), PFLOW_FONT_SIZE));
        iMatW += rangeW - 1;
    }

    const double X0 = 5; // X start of the incidence matrix
    const double Y0 = 12; // Y start of the incidence matrix
    const double X_START_OF_M0    = X0 + 10*ntr + 6; // initial X position of the place names
    const double Y_START_OF_TRN   = Y0 + 10*npl+4; // initial Y position of the transition names
    const double X_START_OF_PLC   = X_START_OF_M0 + 15; // initial X position of the place names
    const double X_START_OF_BND   = X_START_OF_PLC + max_plc_width + 5; // X position of place bounds
    const double X_START_OF_BASIS = X_START_OF_BND + 15; // X position of invariants matrix
    double X_START_OF_LVL_INFO = X_START_OF_BASIS + basis.size() * iMatW + 8; // X position of LevelInfo[]s
    for (size_t l=0; l<numLvlInfo; l++) {
        ppLvlInfo[l]->startX = X_START_OF_LVL_INFO;
        X_START_OF_LVL_INFO += ppLvlInfo[l]->width + 5;
    }
    const double X_START_OF_DD = X_START_OF_LVL_INFO + 5;
    double DD_SCALE = 1.0, DD_WIDTH = 0.0;
    if (ddeps && ddeps->fname) {
        DD_SCALE = (10.0 * npl) / (ddeps->h - ddeps->y);
        DD_WIDTH = (ddeps->w - ddeps->x) * DD_SCALE;
    }
    const double X_WIDTH = X_START_OF_DD + DD_WIDTH + 10;

    *out_width = int(ceil(X_WIDTH + 5));
    *out_height = int(ceil(Y_START_OF_TRN + max_trn_width + 5));

    ofstream eps(filename);
    eps << "%!PS-Adobe-3.0 EPSF-3.0\n"
        << "%%BoundingBox: 0 0 " << *out_width << " " << *out_height << "\n"
        << "/Courier findfont 5 scalefont setfont\n";

    // draw the grid of the incidence matrix. Squares have size 10*10.
    eps << "\n0.8 setgray 0.5 setlinewidth 2 setlinecap\n";
    for (int t=0; t<=ntr; t++) {
        eps << "newpath "<<(X0+10*t)<<" "<<Y0<<" moveto "<<(X0+10*t)<<" "<<(10*npl+Y0)<<" lineto\n";
        eps << "closepath stroke\n";
    }
    for (int p=0; p<=npl; p++) {
        eps << "newpath "<<X0<<" "<<(10*p+Y0)<<" moveto "<<(X0+10*ntr)<<" "<<(10*p+Y0)<<" lineto\n";
        eps << "closepath stroke\n";
    }

    // draw the transition spans
    eps << "\n0.8 setlinewidth 2 setlinecap\n";
    eps << "/myblock {\nnewpath\n0 0 moveto\n0 10 lineto\n10 10 lineto\n10 0 lineto\n"
           "closepath\ngsave setrgbcolor fill\ngrestore 0 setgray stroke\n} def\n\n";
    std::set<pair<int, int>> sh2uniq;
    size_t soups = 0;
    for (int tr=0; tr<ntr; tr++) {
        const trans_span_t& trspan = *trn_set.sorted_trns[tr];
        if (trspan.entries.empty())
            continue;
        eps << "gsave\n"<< (X0+10*tr) << " " 
            << (10*trspan.sorted_entries[0]->level+Y0) << " translate\n";

        int activation_lvl = trn_set.down_active.empty() ? 0 : trn_set.down_active[trspan.top()];
        int prev_lvl = trspan.sorted_entries[0]->level - 1;
        int prev_signature = -1;
        bool prev_non_prod = true;
        bool prev_unique = true;
        for (var_ioh_t* p_elem : trspan.sorted_entries) { 
            size_t type = 2; // productive unique
            if (!p_elem->is_productive)
                type = 0; // non-productive
            else if (!p_elem->is_unique)
                type = 1; // productive shared

            // Fill the blocks below *p_elem
            for (size_t lvl = prev_lvl + 1; lvl < p_elem->level; lvl++) {
                static const char* s_clrs_nop[] = { "0.8 1 1", "1 1 0.8", "1 0.8 0.8" };
                int below_type = prev_non_prod ? 0 : type;
                if (p_elem->is_unique && !prev_non_prod && !prev_unique && lvl < activation_lvl)
                    below_type = 1; // shared
                if (p_elem->is_unique) {
                    pair<int, int> sig_lvl = make_pair(prev_signature, lvl);
                    if (!prev_unique && 0 != sh2uniq.count(sig_lvl)) // transition between shared and unique
                        below_type = 1; // shared
                    sh2uniq.insert(sig_lvl);
                }
                if (below_type == 2)
                    soups++;
                eps << s_clrs_nop[below_type] << " myblock\n";
                eps << "0 10 translate\n";
            }
            if (type == 2)
                soups++;
            // Fill the IOH block
            static const char* s_clrs_ioh[] = { "0 1 1", "1 1 0", "1 0 0" };
            eps << s_clrs_ioh[type] << " myblock\n";
            if (p_elem->input > 0)
                eps << "newpath 2 6 moveto (-"<<p_elem->input<<") show\n";
            if (p_elem->output > 0)
                eps << "newpath 2 2 moveto (+"<<p_elem->output<<") show\n";
            if (p_elem->inhib > 0)
                eps << "newpath 2 4 moveto (<"<<p_elem->inhib<<") show\n";
            eps << "0 10 translate\n";

            prev_non_prod = !p_elem->is_productive;
            prev_unique = p_elem->is_unique;
            prev_lvl = p_elem->level;
            prev_signature = p_elem->signature;
            // p_elem = p_elem->up;
        }
        eps << "grestore\n";

        // Draw the activation levels
        if (!trn_set.down_active.empty()) {
            int lvlY = Y0 + 10 * (trn_set.down_active[trspan.top()]);
            eps << "gsave\n1.5 setlinewidth\n 0 0 1 setrgbcolor\n";
            eps << "newpath "<<(X0+10*tr)<<" "<<lvlY<<" moveto "<<(X0+10*(tr+1))<<" "<<lvlY<<" lineto\n";
            eps << "closepath stroke\ngrestore\n";
        }        
    }
    cout << "Value of SOUPS is " << soups << endl;

    // Write transition names in span order
    eps << "\n/Times-Roman findfont "<<TIMES_FONT_SIZE<<" scalefont setfont\n0 setgray\n";
    for (int t=0; t<ntr; t++) {
        eps << "gsave "<<(X0 + 10*t+10/2)<<" "<<Y_START_OF_TRN<<" translate 67 rotate\n";
        eps <<"newpath 0 0 moveto\n";
        eps << "(" << tabt[trn_set.sorted_trns[t]->trn].trans_name << ") show\ngrestore\n";
    }

    // Write place names in the given variable order
    for (int p=0; p<npl; p++) {
        const int level_of_place = net_to_level[p];
        eps << "newpath "<<X_START_OF_PLC<<" "<<(10*level_of_place+10/4+Y0)<<" moveto\n";
        eps << "(" << tabp[p].place_name << ") show\n";
    }

    // Write initial marking
    eps << "\n/Times-Bold findfont "<<TIMES_FONT_SIZE<<" scalefont setfont\n0 0.4 0 setrgbcolor\n";
    for (int p=0; p<npl; p++) {
        const int level_of_place = net_to_level[p];
        eps << "newpath "<<X_START_OF_M0<<" "<<(10*level_of_place+10/4+Y0)<<" moveto\n";
        eps << "(" << net_mark[p].total << ") show\n";
    }
    // Write m_0 header
    eps << "newpath "<<(X_START_OF_M0-2)<<" "<<Y_START_OF_TRN<<" moveto\n";
    eps << "(m) show\n";
    eps << "\n/Times-Bold findfont "<<(TIMES_FONT_SIZE*0.75)<<" scalefont setfont\n";
    eps << "newpath "<<(X_START_OF_M0+5)<<" "<<(Y_START_OF_TRN-2)<<" moveto\n";
    eps << "(0) show\n";

    // Write place bounds
    const int* bounds =  load_bounds();
    eps << "\n/Times-Bold findfont "<<TIMES_FONT_SIZE<<" scalefont setfont\n0.4 0.2 0 setrgbcolor\n";
    eps << "newpath "<<(X_START_OF_BND-5)<<" "<<Y_START_OF_TRN<<" moveto\n";
    eps << "(bnd) show\n";
    // eps << "\n/Times-Roman findfont "<<TIMES_FONT_SIZE<<" scalefont setfont\n";
    for (int p=0; p<npl; p++) {
        const int level_of_place = net_to_level[p];
        eps << "newpath "<<X_START_OF_BND<<" "<<(10*level_of_place+10/4+Y0)<<" moveto\n";
        if (bounds != nullptr && bounds[p] >= 0)
            eps << "(" << bounds[p] << ") show\n";
        else
            eps << "(-) show\n";
    }

    // Write grid of p-flow basis
    eps << "\n0.8 setgray 0.5 setlinewidth 2 setlinecap\n";
    for (int r=0; r<=basis.size(); r++) {
        eps << "newpath "<<(X_START_OF_BASIS+iMatW*r)<<" "<<Y0<<"\n";
        eps << "moveto "<<(X_START_OF_BASIS+iMatW*r)<<" "<<(10*npl+Y0)<<" lineto\n";
        eps << "closepath stroke\n";
    }
    for (int p=0; p<=npl; p++) {
        eps << "newpath "<<X_START_OF_BASIS<<" "<<(10*p+Y0)<<"\n";
        eps << "moveto "<<(X_START_OF_BASIS+iMatW*basis.size())<<" "<<(10*p+Y0)<<" lineto\n";
        eps << "closepath stroke\n";
    }

    // Write p-flow basis
    eps << "/myblockInv {\nnewpath\n0 0 moveto\n0 10 lineto\n"<<iMatW<<" 10 lineto\n"<<iMatW<<" 0 lineto\n"
           "closepath\ngsave setrgbcolor fill\ngrestore 0 setgray stroke\n} def\n\n";
    eps << "\n0.8 setlinewidth 2 setlinecap\n";
    for (int r=0; r<basis.size(); r++) {
        bool is_int_flow = basis[r].end() != 
            std::find_if(basis[r].begin(), basis[r].end(), 
                         [](sparse_vector_t::index_value_pair iv){ return iv.value < 0; });
        eps << "/Times findfont 5 scalefont setfont\n0 setgray\n";
        const ssize_t leading = (basis[r].nonzeros() > 0 ? basis[r].leading() : 0);
        const ssize_t trailing = (basis[r].nonzeros() > 0 ? basis[r].trailing() : -1);
        eps << "gsave\n"<< (X_START_OF_BASIS+iMatW*r) << " " << (10*leading+Y0) << " translate\n";
        int f_m0 = 0; // p-flow * m_0
        for (int lvl=leading; lvl<=trailing; lvl++) {
            int value = basis[r][lvl];
            f_m0 += net_mark[ level_to_net[lvl] ].total * value;
            int type = (value == 0) ? 0 : (value > 0 ? 1 : 2); // inactive/positive/negative entry
            static const char* s_clrs_basis[2][2] = { 
                { "0.8 1 0.8",   "0.6 0.85 0.5" }, // semiflows
                { "1 0.78 0.62", "1 0.66 0.43"  }  // integer flows
            };
            eps << s_clrs_basis[is_int_flow ? 1 : 0][value != 0 ? 1 : 0] << " myblockInv\n";
            if (value != 0) {
                eps << "newpath "<<((abs(value)<10 ? 2 : 1) + (value<0 ? 0.5 : 0))<<" 3.5 moveto ("
                    << (value>0 ? "+" : "-") <<abs(value)
                    << (rangeMat!=nullptr ? " " : "")
                    << (rangeMat!=nullptr ? (*rangeMat)[r][lvl] : "") << ") show\n";
            }
            eps << "0 10 translate\n";
        }
        eps << "grestore\n";
        // Write the p-flow constant
        eps << "/Times findfont "<<(PFLOW_FONT_SIZE)<<" scalefont setfont\n0 setgray\n";
        eps << "newpath "<<(X_START_OF_BASIS+iMatW*r)<<" "<<(Y0-8)<<" moveto\n";
        eps << "("<<(abs(f_m0)<10?" ":"")<<(f_m0<0 ? "-" : " ")<<abs(f_m0)<< ") show\n";
    }

    // Write extra per-level info
    for (size_t l=0; l<numLvlInfo; l++) {
        // Header
        eps << "\n/Times-Bold findfont "<<TIMES_FONT_SIZE<<" scalefont setfont\n0"<<ppLvlInfo[l]->colors[0]<<" setrgbcolor\n";
        eps << "newpath "<<(ppLvlInfo[l]->startX-2)<<" "<<Y_START_OF_TRN<<" moveto\n";
        eps << "("<<ppLvlInfo[l]->header<<") show\n";
        // Footer
        eps << "newpath "<<(ppLvlInfo[l]->startX)<<" "<<(Y0-8)<<" moveto\n";
        eps << "("<<ppLvlInfo[l]->footer<<") show\n";
        // Entries
        for (int p=0; p<npl; p++) {
            const int level_of_place = net_to_level[p];
            if (!ppLvlInfo[l]->bold.empty())
                eps << "\n/Times"<<(ppLvlInfo[l]->bold[level_of_place]?"-Bold":"")
                    << " findfont "<<TIMES_FONT_SIZE<<" scalefont setfont\n";
            if (!ppLvlInfo[l]->clrIndex.empty())
                eps << ppLvlInfo[l]->colors[ ppLvlInfo[l]->clrIndex[level_of_place] ]<<" setrgbcolor\n";
            eps << "newpath "<<ppLvlInfo[l]->startX<<" "<<(10*level_of_place+10/4+Y0)<<" moveto\n";
            eps << "(" << ppLvlInfo[l]->info[level_of_place] << ") show\n";
        }
    }

    // Write the DD (loading the EPS file generated by dot)
    if (ddeps && ddeps->fname) {
        // Header
        double header_w = EPS_string_width("MDD", TIMES_FONT_SIZE);
        eps << "\n/Times-Bold findfont "<<TIMES_FONT_SIZE<<" scalefont setfont\n";
        eps << "0.64 0.16 0.16 setrgbcolor\n";
        eps << "newpath "<< (X_START_OF_DD + (DD_WIDTH-header_w) / 2) << " "
            << Y_START_OF_TRN << " moveto\n(MDD) show\n";
        // Lines
        eps << "\n0.8 setgray 0.5 setlinewidth 2 setlinecap\n";
        for (int p=0; p<npl; p++) {
            eps << "newpath "<<(X_START_OF_DD)<<" "<<(10*p+Y0+5)<<" moveto "
                << (X_START_OF_DD+DD_WIDTH)<<" "<<(10*p+Y0+5)<<" lineto\n";
            eps << "closepath stroke\n";
        }
        // run the EPS
        eps << "save\n" << X_START_OF_DD << " " << Y0 << " translate\n";
        eps << DD_SCALE << " " << DD_SCALE << " scale\n";
        eps << "-" << ddeps->x << " -" << ddeps->y << " translate\n";
        eps << "(" << ddeps->fname << ") run\nrestore\n";
    }

    eps << "\n%%Trailer\n";
}

//---------------------------------------------------------------------------------------

// Try improving a target score function over a given variable order
// using the simulated annealing technique.
void simulated_annealing(std::vector<int>& start_net_to_level, 
                         trans_span_set_t& trn_set, 
                         flow_basis_metric_t* opt_fbm,
                         VariableOrderMetric target_metric,
                         bool use_group_repos,
                         bool apply_group_force)
{
    optimization_finder::conf conf;
    // init_genrand64(0xA67BD90E);
    conf.max_tentatives = g_sim_ann_num_tentatives;
    std::vector<size_t> lvl_weights;

    // Get the SOUPS score from a variable order
    auto score_fn = [&](const std::vector<int>& net_to_level) -> double {
        metric_t m;
        metric_compute(net_to_level, trn_set ,opt_fbm, target_metric, &m);
        return m.cast_to_double();
    };

    // generate a new solution by relocating two variables
    auto generate_fn = [](const std::vector<int>& net_to_level, 
                          std::vector<int>& new_order) 
    {
        new_order = net_to_level;
        // Make a new varorder with a random place shift
        size_t pos0, pos1;
        pos0 = genrand64_int63() % npl;
        do { pos1 = genrand64_int63() % npl; } 
        while (pos1 == pos0);

        // Move a variable from pos0 to pos1, shifting the intermediate variables
        if (pos0 < pos1) {
            std::rotate(new_order.begin() + pos0, 
                        new_order.begin() + pos0 + 1, 
                        new_order.begin() + pos1);
        }
        else { // pos1 < pos0
            std::rotate(new_order.begin() + pos1, 
                        new_order.begin() + pos0 - 1, 
                        new_order.begin() + pos0);
        }
    };

    auto msg_fn = [&](optimization_finder::message m, size_t iter, 
                      double score, double new_score) 
    {
        if (running_for_MCC())
            return; // quiet
        if (m == optimization_finder::IMPROVEMENT) {
            cout << "   iter="<<setw(5)<<iter<<"  " 
                 << metric_name(target_metric) 
                 << "=" << new_score << endl;
        }
        else if (m == optimization_finder::TIME_EXCEEDED)
            cout << "   Time exceeded." << endl;
        else if (m == optimization_finder::REACHED_LOCAL_MINIMUM)
            cout << "   Local minimum found at iter " << iter << "." << endl;
    };

    relocation_fn_t gen_fn;
    if (use_group_repos)
        gen_fn = make_invariant_group_repositioner(opt_fbm, trn_set, apply_group_force);
    else
        gen_fn = generate_fn;

    optimization_finder::optimize(start_net_to_level, score_fn, gen_fn, msg_fn, conf);
}

//---------------------------------------------------------------------------------------
























