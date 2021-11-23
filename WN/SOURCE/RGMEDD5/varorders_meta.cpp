#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <map>
#include <stack>

#include <unistd.h>

#include "rgmedd5.h"
#include "varorders.h"
#include "parallel.h"

int g_par_vo_num_parallel_procs = -1;
extern bool g_print_varorder_metaheuristic_scores;

//---------------------------------------------------------------------------------------

// const size_t MCL_TIMEOUT_SECONDS = 5;

// static bool is_clusterable() {   
//     try { // Generate the place clustering using the command line mcl tool
//         return !mcl_cluster_net(MCL_TIMEOUT_SECONDS).empty(); // use a timeout
//     }
//     catch (rgmedd_exception e) { // Timeout or mcl crash
//         return false;
//     }
// }

//---------------------------------------------------------------------------------------

// // A weight that counter-balances NES (2017 MCC, old)
// // Give more wight to the algorithms that use invariants, since the NES
// // score does not account for them.
struct ScoreWeights {
    VariableOrderCriteria voc;
    double base_weight;
    double force_weight;
};
static const ScoreWeights g_weights[] = { 
    // // End of list
    { VOC_NO_REORDER, 1.0, 1.0 } // No weights applied
};

ScoreWeights get_weights_for(VariableOrderCriteria voc, bool is_multi) {
    if (is_multi) // Model Checking Context : do not use weights
        return { voc, 1, 1 };

    const ScoreWeights* psw = g_weights;
    while (psw->voc != VOC_NO_REORDER) {
        if (psw->voc == voc)
            break;
        ++psw;
    }
    return *psw;
}

//---------------------------------------------------------------------------------------






//---------------------------------------------------------------------------------------

const char *g_variable_order_variation_type[] = {
    "", "+Force", "+CP"
};

//---------------------------------------------------------------------------------------

ssize_t write_retry (int fd, const void* buf, size_t size) {
    ssize_t ret;
    while (size > 0) {
        do {
            ret = write(fd, buf, size);
        } while ((ret < 0) && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK));
        if (ret < 0)
            return ret;
        size -= ret;
        buf = (char*)(buf) + ret;
    }
    return 0;
}

//---------------------------------------------------------------------------------------

// serialize a score_vo_t object into a file
bool write_score_vo(const score_vo_t& sv, int fd) {
    if (sv.valid()) {
        const score_vo_base_t* header = (const score_vo_base_t*)&sv;
        size_t nbytes = sv.order.size() * sizeof(sv.order[0]);

        if (0 == write_retry(fd, header, sizeof(*header))) {
            if (0 == write_retry(fd, sv.order.data(), nbytes)) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------

// read back a serialized score_vo_t object form a memory buffer
score_vo_t read_score_vo(const void* buffer, size_t buflen) {
    score_vo_t sv;
    size_t nbytes_tot = sizeof(score_vo_base_t) + npl * sizeof(int);

    if (buflen == nbytes_tot) {
        const score_vo_base_t* header = (const score_vo_base_t*)buffer;
        buffer = (char*)(buffer) + sizeof(score_vo_base_t);

        *(score_vo_base_t*)&sv = *header;
        sv.order.resize(npl);
        std::copy((int*)buffer, (int*)buffer + npl, sv.order.begin());
    }
    return sv;
}

//---------------------------------------------------------------------------------------

void print_score(const score_vo_t& sv, bool print_table) {
    if (print_table) { 
        const char *vname = var_order_name(sv.voc).first;
        ssize_t n = 25 - strlen(vname) - strlen(sv.variation());
        cout << "  "<<vname<<sv.variation()<< setw(n<0?0:n) << sv.score;
        cout << setw(12) << sv.score_ex2;
        cout << setw(12) << sv.score_ex3;
        cout << setw(9) << sv.score_ex4;
        cout << setw(8) << sv.weight;
        cout << setw(11) << sv.seconds_to_build
             << " sec." << endl;
    }    
}

//---------------------------------------------------------------------------------------

// store all order data and scores into a score_vo_t structure
score_vo_t
build_score(VariableOrderCriteria voc, 
            variable_order_variation_type vt, 
            std::vector<int> &&order,
            double score, double weight, 
            const var_order_selector& sel, 
            clock_t start_time) 
{
    score *= weight; // Apply weight **before** the announce

    score_vo_t sv(npl);
    sv.order = std::move(order);
    sv.voc = voc;
    sv.score = score;
    sv.score_ex2 = sel.p_meta_score_ex2 ? *sel.p_meta_score_ex2 : -1;
    sv.score_ex3 = sel.p_meta_score_ex3 ? *sel.p_meta_score_ex3 : -1;
    sv.score_ex4 = sel.p_meta_score_ex4 ? *sel.p_meta_score_ex4 : -1;
    sv.weight = weight;
    sv.variation_type = vt;
    sv.seconds_to_build = (double(clock() - start_time) / CLOCKS_PER_SEC);

    return sv;
};

//---------------------------------------------------------------------------------------

score_vo_t
generate_variable_order(VariableOrderCriteria voc,
                        metaheuristic_context_t& mhctx)
{
    assert(!method_uses_lin_constraints(voc) || mhctx.has_psf);
    assert(!method_uses_nested_units(voc) || model_has_nested_units());
    bool do_discount = (voc == VOC_VCL_ADVANCED_CUTHILL_MCKEE) || 
                       (voc == VOC_VCL_CUTHILL_MCKEE);
    do_discount = do_discount && (npl < ntr/4);
    ScoreWeights SW = get_weights_for(voc, mhctx.is_multi);

    double score = -1, score_ex2 = -1, score_ex3 = -1, score_ex4 = -1;
    var_order_selector sel;
    sel.heuristics = voc;
    sel.p_meta_score = &score;
    sel.p_meta_score_ex2 = &score_ex2; // TODO: remove
    sel.p_meta_score_ex3 = &score_ex3; // TODO: remove
    sel.p_meta_score_ex4 = &score_ex4; // TODO: remove
    sel.discount_score = do_discount;
    sel.verbose = false;

    try {
        // Get the order & its (weighted) score
        clock_t start_time = clock();
        std::vector<int> order(npl);
        determine_var_order(sel, mhctx.S2Ipl, order, mhctx.trns_set, mhctx.fbm);
        assert(score >= 0);
        return build_score(voc, VT_NONE, std::move(order), score, 
                           SW.base_weight, sel, start_time);
    }
    catch (rgmedd_exception& e) {
        if (!running_for_MCC())
            cout << "Skipping " << var_order_name(voc).first 
                 << ", reason: " << e.what() << endl;
        return score_vo_t(0);
    }
}

//---------------------------------------------------------------------------------------

score_vo_t
refine_variable_order(const score_vo_t& base_order,
                      metaheuristic_context_t& mhctx) 
{
    ScoreWeights SW = get_weights_for(base_order.voc, mhctx.is_multi);

    double score = -1, score_ex2 = -1, score_ex3 = -1, score_ex4 = -1;
    var_order_selector sel;
    // sel.heuristics = voc;
    sel.p_meta_score = &score;
    sel.p_meta_score_ex2 = &score_ex2; // TODO: remove
    sel.p_meta_score_ex3 = &score_ex3; // TODO: remove
    sel.p_meta_score_ex4 = &score_ex4; // TODO: remove
    sel.discount_score = false;
    sel.verbose = false;

    // Run Force on that order and test the obtained variation
    score = -1;
    sel.heuristics = VOC_INPUT_ORDER;
    sel.refinement = ForceBasedRefinement::BEST_AVAILABLE;
    try {
        clock_t start_time = clock();
        std::vector<int> order = base_order.order; // copy and modify
        determine_var_order(sel, mhctx.S2Ipl, order, mhctx.trns_set, mhctx.fbm);
        assert(score >= 0);
        return build_score(base_order.voc, VT_FORCE, std::move(order), score, 
                           SW.force_weight, sel, start_time);
    }
    catch (rgmedd_exception& e) {
        if (!running_for_MCC())
            cout << "Skipping " << var_order_name(base_order.voc).first 
                 << "+Force, reason: " << e.what() << endl;
        return score_vo_t(0);
    }
}

//---------------------------------------------------------------------------------------

bool insert_order_priority(const score_vo_t& sv,
                           std::vector<score_vo_t>& varorders, 
                           size_t max_varorders)
{
    if (!sv.valid())
        return false;
    varorders.insert(std::upper_bound(varorders.begin(), varorders.end(), sv), sv);
    if (varorders.size() > max_varorders)
        varorders.resize(varorders.size() - 1);

    return true;
}

//---------------------------------------------------------------------------------------

// Select the variable order which has the best weighted score
void metaheuristic(metaheuristic_context_t& mhctx,
                   std::vector<score_vo_t>& varorders,
                   size_t max_varorders)
{
    clock_t start_time = 0;

    // Conditions for heuristics applicability
    mhctx.has_psf = (get_int_constr_problem().size() >= 1);
    mhctx.has_nu = model_has_nested_units();
    mhctx.has_scc = (get_num_components() > 1);
    mhctx.is_multi = (max_varorders > 1);

    struct { 
        VariableOrderCriteria voc;  // Tested algorithm in the meta-heuristic
        int refine_with_force;      // Test also the algo+FORCE variation
        int apply_compact_pbasis;   // Test also the algo+Annealing variation
        bool selectable;            // Is it selectable for the current instance?
    } 
    algoList[] = {
        { VOC_SLOAN,                       1, 0, true },
        { VOC_SLOAN_1_16,                  1, 0, (npl<5000 && ntr<10000) },
        { VOC_TOVCHIGRECHKO2,              1, 0, true }, 
        { VOC_NOACK2,                      1, 0, true }, 
        { VOC_VCL_ADVANCED_CUTHILL_MCKEE,  1, 0, (npl<1000) },
        { VOC_VCL_CUTHILL_MCKEE,           1, 0, (npl<1000) },
        { VOC_VCL_GIBBS_POOLE_STOCKMEYER,  1, 0, (npl<5000) && mhctx.is_multi }, // NEW ENTRY 2020
        { VOC_CUTHILL_MCKEE,               1, 0, (npl<5000) && mhctx.is_multi }, // NEW ENTRY 2020
        { VOC_GRADIENT_NU,                 1, 0, mhctx.has_nu },
        // { VOC_MARKOV_CLUSTER,              0, 0, is_clusterable() },
        { VOC_PCHAINING,                   0, 0, mhctx.has_psf && (npl < 1000) }, // P-INV does not scale well
        { VOC_GRADIENT_P,                  1, 0, mhctx.has_psf },
        { VOC_TOPOLOGICAL,                 1, 0, mhctx.has_scc },
        { VOC_FORCE_NU,                    1, 0, mhctx.has_nu && mhctx.is_multi }, // NEW ENTRY 2020
        { VOC_NO_REORDER /* end of list */ }
    };
    const size_t NUM_ALGOS = sizeof(algoList) / sizeof(algoList[0]);

    bool print_table = g_print_varorder_metaheuristic_scores;//!running_for_MCC();

    if (print_table) {
        cout << "  METHOD              SCORE        SWIR       SOUPS DISCOUNT  WEIGHT       TIME " << endl;
    }

    if (g_par_vo_num_parallel_procs <= 0) {
        //---------------------------
        // Sequential
        //---------------------------
        // Test all the variable orderings in algoList[]
        for (int m=0; algoList[m].voc != VOC_NO_REORDER; m++) {
            const VariableOrderCriteria voc = algoList[m].voc;
            if (!algoList[m].selectable)
                continue;

            score_vo_t sv = generate_variable_order(voc, mhctx);
            print_score(sv, print_table);
            if (insert_order_priority(sv, varorders, max_varorders)) {

                if (!algoList[m].refine_with_force)
                    continue;
                score_vo_t sv2 = refine_variable_order(sv, mhctx);
                print_score(sv2, print_table);
                insert_order_priority(sv2, varorders, max_varorders);
            }
        }
    }
    else {
        //---------------------------
        // Parallel
        //---------------------------
        const size_t MAXBUF = 4096;
        const size_t NUM_PHASES = 2;
        proc_wback_t wb;

        std::vector<score_vo_t> gen_orders[NUM_PHASES];

        // PHASE 1: base heuristics
        for (size_t ph=0; ph<NUM_PHASES; ph++) {
            int result = EXIT_FAILURE;
            std::vector<int> exitcodes;
            std::vector<size_t> lengths;
            std::vector<void*> memblocks;
            wb = parallel_exec_wback(g_par_vo_num_parallel_procs, NUM_ALGOS,
                                     exitcodes, lengths, memblocks, 
                                     MAXBUF, false);

            if (wb.task_id >= 0) { // subprocess computing the variable order
                const VariableOrderCriteria voc = algoList[wb.task_id].voc;
                if (algoList[wb.task_id].selectable) {
                    switch (ph) {
                    case 0: {
                            score_vo_t sv = generate_variable_order(voc, mhctx);
                            write_score_vo(sv, wb.write_fd);
                            result = EXIT_SUCCESS;
                        }
                        break;

                    case 1: {
                            const score_vo_t& base_vo = gen_orders[0][wb.task_id];
                            if (base_vo.valid()) {
                                score_vo_t sv = refine_variable_order(base_vo, mhctx);
                                write_score_vo(sv, wb.write_fd);
                                result = EXIT_SUCCESS;
                            }
                        }
                        break;
                    }
                }
                close(wb.write_fd);
                exit(result);
            }

            // server process
            gen_orders[ph].resize(NUM_ALGOS);
            for (size_t m=0; m<NUM_ALGOS; m++) {
                // recover all generated orders
                if (WEXITSTATUS(exitcodes[m]) == EXIT_SUCCESS) {
                    gen_orders[ph][m] = read_score_vo(memblocks[m], lengths[m]);
                    print_score(gen_orders[ph][m], print_table);
                }
                if (memblocks[m] != nullptr)
                    free(memblocks[m]);
            }
        }

        // PHASE 3: collect and compare
        for (int m=0; algoList[m].voc != VOC_NO_REORDER; m++) {
            for (size_t ph=0; ph<NUM_PHASES; ph++) {
                const score_vo_t& sv = gen_orders[ph][m];
                if (sv.valid())
                    insert_order_priority(sv, varorders, max_varorders);
            }
        } 

    } // end sequential/parallel
}

//---------------------------------------------------------------------------------------



















