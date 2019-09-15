#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <map>
#include <stack>

#include "rgmedd3.h"
#include "varorders.h"

//---------------------------------------------------------------------------------------

// static int num_pinvariants = -1;

// // Returns the number of P-invariants (0 if there are no P-invariants, or if the
// // model has an empty .pin file)
// int get_num_Psemiflows() {
//     if (num_pinvariants == -1) {
//         num_pinvariants = 0;
//         std::string pinv_name(net_name);
//         pinv_name += "pin";
//         ifstream pif(pinv_name.c_str());
//         if (pif) {
//             pif >> num_pinvariants;
//             pif.close();
//         }
//     }
//     return num_pinvariants;
// }

//---------------------------------------------------------------------------------------

// const size_t NUM_VARORDER_METRICS2 = 27;
// void gen_varorder_metrics2(std::vector<double>& out_metrics, const std::vector<int> *var_order) {
//     size_t curr_size = out_metrics.size();
//     if (var_order == nullptr) {
//         for (int k=0; k<NUM_VARORDER_METRICS2; k++)
//             out_metrics.push_back(0.0);
//     }
//     else {
//         // Generate the metrics for this variable order
//         accumulator NES, WES, CRS, WCRS, Prof, Band, Wave;

//         std::tie(NES, WES) = measure_NWES(*var_order);
//         std::tie(CRS, WCRS) = measure_WCRS(*var_order);
//         std::tie(Prof, Band, Wave) = measure_profile_bandwidth(*var_order);
        
//         out_metrics.push_back(NES.mean());
//         out_metrics.push_back(NES.stddev());        
//         out_metrics.push_back(NES.root_mean_sq());        
//         out_metrics.push_back(WES.mean());
//         out_metrics.push_back(WES.stddev());        
//         out_metrics.push_back(WES.root_mean_sq());    

//         out_metrics.push_back(CRS.mean());
//         out_metrics.push_back(CRS.stddev());        
//         out_metrics.push_back(CRS.root_mean_sq());        
//         out_metrics.push_back(WCRS.mean());
//         out_metrics.push_back(WCRS.stddev());        
//         out_metrics.push_back(WCRS.root_mean_sq());        

//         out_metrics.push_back(Prof.mean());
//         out_metrics.push_back(Prof.stddev());        
//         out_metrics.push_back(Prof.root_mean_sq());        
//         out_metrics.push_back(Band.mean());
//         out_metrics.push_back(Band.stddev());        
//         out_metrics.push_back(Band.root_mean_sq());     

//         out_metrics.push_back(Wave.mean());
//         out_metrics.push_back(Wave.stddev());        
//         out_metrics.push_back(Wave.root_mean_sq());     

//         // Normalization coefficients
//         out_metrics.push_back(double(npl)/ntr);
//         out_metrics.push_back(double(ntr)/npl);
//         out_metrics.push_back(1.0 / double(npl));
//         out_metrics.push_back(1.0 / double(ntr));
//         out_metrics.push_back(log(npl));
//         out_metrics.push_back(log(ntr));
//     }
//     assert(out_metrics.size() - curr_size == NUM_VARORDER_METRICS2);
// }

// //---------------------------------------------------------------------------------------

// static const VariableOrderCriteria tested_methods_bis[] = {
//     // VOC_MEAS_FORCE,
//     // VOC_MEAS_FORCE_NES,
//     // VOC_MEAS_FORCE_WES1,
//     VOC_GRADIENT_P,
//     VOC_CUTHILL_MCKEE,
//     VOC_KING,
//     VOC_SLOAN,
//     VOC_SLOAN_1_16,
//     VOC_NOACK,
//     VOC_TOVCHIGRECHKO,
//     // VOC_MEAS_FORCE_NU,
//     VOC_GRADIENT_NU,
//     VOC_MARKOV_CLUSTER,
//     VOC_PINV,
//     // VOC_MEAS_FORCE_PINV,
// };
// static const size_t NUM_TESTED_METHODS_BIS = sizeof(tested_methods_bis) / sizeof(tested_methods_bis[0]);

// void generate_varorder_metrics2(const std::map<const char*, int, cstr_less>& S2Ipl, 
//                                 trans_span_set_t &trns_set,
//                                 flow_basis_metric_t& fbm) 
// {
//     // collect statistics on input, test and output arcs
//     int num_input_arcs = 0, num_output_arcs = 0;

//     for (int tr=0; tr<ntr; tr++) {
//         for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr))
//             num_input_arcs++;
//         for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr))
//             num_output_arcs++;
//     }
//     cout << "npl="<<npl<<" ntr="<<ntr<<" in="<<num_input_arcs<<" out="<<num_output_arcs<<endl;
//     // exit(0);


//     // gen_net_metrics(out_metrics, pinv_metrics);
//     std::vector<double> metrics;

//     bool have_pinv = get_num_Psemiflows();
//     // Get the variable order metrics for each of the tested methods.
//     for (int m=0; m<NUM_TESTED_METHODS_BIS; m++) {
//         const VariableOrderCriteria voc = tested_methods_bis[m];
//         metrics.resize(0);
//         // if (voc == VOC_MARKOV_CLUSTER)
//         //     continue; // skip mcl
//         if (method_uses_pinvs(voc) && !have_pinv) {
//             // skip this method, produce a vector of 0 metrics
//             gen_varorder_metrics2(metrics, nullptr);
//         }
//         else if (method_uses_nested_units(voc) && !model_has_nested_units()) {
//             // skip this method, produce a vector of 0 metrics
//             gen_varorder_metrics2(metrics, nullptr);
//         }
//         else {
//             // Compute the order and measure it.
//             std::vector<int> var_order;
//             var_order_selector sel;
//             sel.heuristics = voc;
//             determine_var_order(sel, S2Ipl, var_order, trns_set, fbm);
//             gen_varorder_metrics2(metrics, &var_order);
//         }
//         assert(metrics.size() == NUM_VARORDER_METRICS2);

//         // Print the metric vector
//         cout << "METRIC " << net_name << "," << var_order_name(voc).first;
//         for (double v : metrics)
//             cout << "," << v;
//         cout << endl << flush;
//     }
//     cout << endl;
// }


//---------------------------------------------------------------------------------------

const size_t MCL_TIMEOUT_SECONDS = 5;

static bool is_clusterable() {   
    try { // Generate the place clustering using the command line mcl tool
        return !mcl_cluster_net(MCL_TIMEOUT_SECONDS).empty(); // use a timeout
    }
    catch (rgmedd_exception e) { // Timeout or mcl crash
        return false;
    }
}

//---------------------------------------------------------------------------------------

// A weight that counter-balances NES (2017 MCC, old)
// Give more wight to the algorithms that use invariants, since the NES
// score does not account for them.
struct ScoreWeights {
    VariableOrderCriteria voc;
    double base_weight;
    double force_weight;
};
static const ScoreWeights g_weights[] = { 
    // 481, 354.498
    { VOC_PCHAINING,                     1.049, 1.063 },
    { VOC_VCL_CUTHILL_MCKEE,             1.275, 1.099 },
    { VOC_VCL_ADVANCED_CUTHILL_MCKEE,    1.011, 1.278 },
    { VOC_SLOAN,                         1.122, 0.880 },
    { VOC_SLOAN_1_16,                    0.798, 0.911 },
    { VOC_NOACK2,                        1.131, 0.980 },
    { VOC_TOVCHIGRECHKO2,                0.909, 1.158 },
    { VOC_GRADIENT_P,                    1.163, 1.125 },
    { VOC_GRADIENT_NU,                   1.225, 0.885 },
    { VOC_MARKOV_CLUSTER,                0.615, 0.975 },
    { VOC_TOPOLOGICAL,                   0.684, 0.789 },
    // // End of list
    { VOC_NO_REORDER, 1.0, 1.0 } // No weights applied
};

ScoreWeights get_weights_for(VariableOrderCriteria voc) {
    const ScoreWeights* psw = g_weights;
    while (psw->voc != VOC_NO_REORDER) {
        if (psw->voc == voc)
            break;
        ++psw;
    }
    return *psw;
}

//---------------------------------------------------------------------------------------

// Select the variable order which has the best weighted score
void metaheuristic_wscore(VariableOrderCriteria metavoc,
                          std::vector<score_vo_t>& varorders,
                          size_t max_varorders,
                          VariableOrderMetric& ann_metric,
                          trans_span_set_t &trns_set,
                          flow_basis_metric_t& fbm,
                          const std::map<const char*, int, cstr_less>& S2Ipl) 
{
    std::vector<int> order(npl);
    // VariableOrderCriteria best_voc = VOC_NO_REORDER;
    // const char* best_variation = "";
    // double best_score = -1;
    clock_t start_time = 0;

    // Conditions for heuristics applicability
    bool has_psf = (get_num_Psemiflows() >= 1);
    bool has_nu = model_has_nested_units();
    bool has_scc = (get_num_components() > 1);

    struct { 
        VariableOrderCriteria voc;  // Tested algorithm in the meta-heuristic
        int refine_with_force;      // Test also the algo+FORCE variation
        int apply_compact_pbasis;   // Test also the algo+Annealing variation
        bool selectable;            // Is it selectable for the current instance?
    }
    algoList[] {
        { VOC_SLOAN,                       1, 0, true },
        { VOC_SLOAN_1_16,                  1, 0, (npl<5000 && ntr<10000) },
        { VOC_TOVCHIGRECHKO2,              1, 0, true }, 
        { VOC_NOACK2,                      1, 0, true }, 
        { VOC_VCL_ADVANCED_CUTHILL_MCKEE,  1, 0, (npl<1000) },
        { VOC_VCL_CUTHILL_MCKEE,           1, 0, (npl<1000) },
        { VOC_GRADIENT_NU,                 1, 0, has_nu },
        { VOC_MARKOV_CLUSTER,              0, 0, is_clusterable() },
        { VOC_PCHAINING,                   0, 0, has_psf && (npl < 1000) }, // P-INV does not scale well
        { VOC_GRADIENT_P,                  1, 0, has_psf },
        { VOC_TOPOLOGICAL,                 1, 0, has_scc },
        { VOC_NO_REORDER /* end of list */ }
    };

    // Evaluate and print the given score
    auto evaluate_score = [&](VariableOrderCriteria voc, const char* variation, double score, 
                              double weight, const var_order_selector& sel) 
    {
        score *= weight; // Apply weight **before** the announce
        if (!running_for_MCC()) { // announce computed score
            const char *vname = var_order_name(voc).first;
            ssize_t n = 25 - strlen(vname) - strlen(variation);
            cout << "  "<<vname<<variation<< setw(n<0?0:n) << score;
            if (sel.p_meta_score_ex2)
                cout << setw(12) << *sel.p_meta_score_ex2;
            if (sel.p_meta_score_ex3)
                cout << setw(12) << *sel.p_meta_score_ex3;
            if (sel.p_meta_score_ex4)
                cout << setw(9) << *sel.p_meta_score_ex4;
            cout << setw(8) << weight;
            cout << setw(11) << (double(clock() - start_time) / CLOCKS_PER_SEC) << " sec." << endl;
        }
        score_vo_t sv(npl);
        sv.order = order;
        sv.voc = voc;
        sv.score = score;
        sv.variation = variation;
        varorders.insert(std::upper_bound(varorders.begin(), varorders.end(), sv), sv);
        if (varorders.size() > max_varorders)
            varorders.resize(varorders.size() - 1);
        // if (best_score == -1 || score < best_score) {
        //     best_order = order;
        //     best_voc = voc;
        //     best_score = score;
        //     best_variation = variation;
        // }
    };

    if (!running_for_MCC()) {
        cout << "  METHOD              SCORE        SWIR       SOUPS DISCOUNT  WEIGHT       TIME " << endl;
    }

    // Test all the variable orderings in algoList[]
    for (int m=0; algoList[m].voc != VOC_NO_REORDER; m++) {
        const VariableOrderCriteria voc = algoList[m].voc;
        if (!algoList[m].selectable)
            continue;

        assert(!method_uses_pinvs(voc) || has_psf);
        assert(!method_uses_nested_units(voc) || model_has_nested_units());
        bool do_discount = (voc == VOC_VCL_ADVANCED_CUTHILL_MCKEE) || (voc == VOC_VCL_CUTHILL_MCKEE);
        do_discount = do_discount && (npl < ntr/4);
        ScoreWeights SW = get_weights_for(voc);

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
            start_time = clock();
            determine_var_order(sel, S2Ipl, order, trns_set, fbm);
            assert(score >= 0);
            evaluate_score(voc, "", score, SW.base_weight, sel);
        }
        catch (rgmedd_exception& e) {
            if (!running_for_MCC())
                cout << "Skipping " << var_order_name(voc).first << ", reason: " << e.what() << endl;
            continue;
        }

        // Run Force on that order and test the obtained variation
        if (algoList[m].refine_with_force) {
            score = -1;
            sel.heuristics = VOC_INPUT_ORDER;
            sel.refinement = ForceBasedRefinement::BEST_AVAILABLE;
            try {
                start_time = clock();
                determine_var_order(sel, S2Ipl, order, trns_set, fbm);
                assert(score >= 0);
                evaluate_score(voc, "+Force", score, SW.force_weight, sel);
            }
            catch (rgmedd_exception& e) {
                if (!running_for_MCC())
                    cout << "Skipping " << var_order_name(voc).first << "+Force, reason: " << e.what() << endl;
            }
        }

        // Run compact-pbasis heuristics on that order and test the obtained variation
        if (algoList[m].apply_compact_pbasis) {
            score = -1;
            sel.heuristics = VOC_INPUT_ORDER;
            sel.refinement = ForceBasedRefinement::NO_REFINEMENT;
            sel.annealing = Annealing::ANN_PBASIS_MIN;
            try {
                start_time = clock();
                determine_var_order(sel, S2Ipl, order, trns_set, fbm);
                assert(score >= 0);
                evaluate_score(voc, "+CP", score, 1.0/*SW.ann_weight*/, sel);
            }
            catch (rgmedd_exception& e) {
                if (!running_for_MCC())
                    cout << "Skipping " << var_order_name(voc).first << "+CP, reason: " << e.what() << endl;
            }
        }
    }

    // if (!running_for_MCC()) {
    //     cout << "Meta-heuristic: selecting method " << var_order_name(varorders[0].voc).first << varorders[0].variation
    //          << " with score: " << varorders[0].score << endl;
    // }

    // varorder = std::move(best_order);
    if (max_varorders == 1 && 
        ann_metric == VariableOrderMetric::METRIC_SWIR && 
        varorders[0].voc == VOC_MARKOV_CLUSTER)
        ann_metric = VariableOrderMetric::METRIC_SWIR_X;
}

//---------------------------------------------------------------------------------------







