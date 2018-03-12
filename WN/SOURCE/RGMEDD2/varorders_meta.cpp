

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <map>
#include <stack>
#include <unistd.h> // access()

#include "rgmedd2.h"
#include "varorders.h"

//---------------------------------------------------------------------------------------

static int num_pinvariants = -1;

// Returns the number of P-invariants (0 if there are no P-invariants, or if the
// model has an empty .pin file)
static int test_have_pinvariants() {
    if (num_pinvariants == -1) {
        num_pinvariants = 0;
        std::string pinv_name(net_name);
        pinv_name += "pin";
        ifstream pif(pinv_name.c_str());
        if (pif) {
            pif >> num_pinvariants;
            pif.close();
        }
    }
    return num_pinvariants;
}

//---------------------------------------------------------------------------------------

const size_t NUM_VARORDER_METRICS2 = 27;
void gen_varorder_metrics2(std::vector<double>& out_metrics, const std::vector<int> *var_order) {
    size_t curr_size = out_metrics.size();
    if (var_order == nullptr) {
        for (int k=0; k<NUM_VARORDER_METRICS2; k++)
            out_metrics.push_back(0.0);
    }
    else {
        // Generate the metrics for this variable order
        accumulator NES, WES, CRS, WCRS, Prof, Band, Wave;

        std::tie(NES, WES) = measure_NWES(*var_order);
        std::tie(CRS, WCRS) = measure_WCRS(*var_order);
        std::tie(Prof, Band, Wave) = measure_profile_bandwidth(*var_order);
        
        out_metrics.push_back(NES.mean());
        out_metrics.push_back(NES.stddev());        
        out_metrics.push_back(NES.root_mean_sq());        
        out_metrics.push_back(WES.mean());
        out_metrics.push_back(WES.stddev());        
        out_metrics.push_back(WES.root_mean_sq());    

        out_metrics.push_back(CRS.mean());
        out_metrics.push_back(CRS.stddev());        
        out_metrics.push_back(CRS.root_mean_sq());        
        out_metrics.push_back(WCRS.mean());
        out_metrics.push_back(WCRS.stddev());        
        out_metrics.push_back(WCRS.root_mean_sq());        

        out_metrics.push_back(Prof.mean());
        out_metrics.push_back(Prof.stddev());        
        out_metrics.push_back(Prof.root_mean_sq());        
        out_metrics.push_back(Band.mean());
        out_metrics.push_back(Band.stddev());        
        out_metrics.push_back(Band.root_mean_sq());     

        out_metrics.push_back(Wave.mean());
        out_metrics.push_back(Wave.stddev());        
        out_metrics.push_back(Wave.root_mean_sq());     

        // Normalization coefficients
        out_metrics.push_back(double(npl)/ntr);
        out_metrics.push_back(double(ntr)/npl);
        out_metrics.push_back(1.0 / double(npl));
        out_metrics.push_back(1.0 / double(ntr));
        out_metrics.push_back(log(npl));
        out_metrics.push_back(log(ntr));
    }
    assert(out_metrics.size() - curr_size == NUM_VARORDER_METRICS2);
}

//---------------------------------------------------------------------------------------

static bool is_clusterable() {   
    return false;
}

//---------------------------------------------------------------------------------------

static const VariableOrderCriteria tested_methods_bis[] = {
    VOC_MEAS_FORCE,
    VOC_MEAS_FORCE_NES,
    VOC_MEAS_FORCE_WES1,
    VOC_GRADIENT_P,
    VOC_CUTHILL_MCKEE,
    VOC_KING,
    VOC_SLOAN,
    VOC_SLOAN_1_16,
    VOC_NOACK,
    VOC_TOVCHIGRECHKO,
    VOC_MEAS_FORCE_NU,
    VOC_GRADIENT_NU,
    VOC_MARKOV_CLUSTER,
    VOC_PINV,
    VOC_MEAS_FORCE_PINV,
};
static const size_t NUM_TESTED_METHODS_BIS = sizeof(tested_methods_bis) / sizeof(tested_methods_bis[0]);

void generate_varorder_metrics2(const std::map<std::string, int>& S2Ipl) 
{
    // collect statistics on input, test and output arcs
    int num_input_arcs = 0, num_output_arcs = 0;

    for (int tr=0; tr<ntr; tr++) {
        for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr))
            num_input_arcs++;
        for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr))
            num_output_arcs++;
    }
    cout << "npl="<<npl<<" ntr="<<ntr<<" in="<<num_input_arcs<<" out="<<num_output_arcs<<endl;
    // exit(0);


    // gen_net_metrics(out_metrics, pinv_metrics);
    std::vector<double> metrics;

    bool have_pinv = test_have_pinvariants();
    // Get the variable order metrics for each of the tested methods.
    for (int m=0; m<NUM_TESTED_METHODS_BIS; m++) {
        const VariableOrderCriteria voc = tested_methods_bis[m];
        metrics.resize(0);
        // if (voc == VOC_MARKOV_CLUSTER)
        //     continue; // skip mcl
        if (method_uses_pinvs(voc) && !have_pinv) {
            // skip this method, produce a vector of 0 metrics
            gen_varorder_metrics2(metrics, nullptr);
        }
        else if (method_uses_nested_units(voc) && !model_has_nested_units()) {
            // skip this method, produce a vector of 0 metrics
            gen_varorder_metrics2(metrics, nullptr);
        }
        else {
            // Compute the order and measure it.
            std::vector<int> var_order;
            determine_var_order(voc, false, false, S2Ipl, var_order, true);
            gen_varorder_metrics2(metrics, &var_order);
        }
        assert(metrics.size() == NUM_VARORDER_METRICS2);

        // Print the metric vector
        cout << "METRIC " << net_name << "," << var_order_name(voc).first;
        for (double v : metrics)
            cout << "," << v;
        cout << endl << flush;
    }
    cout << endl;
}


//---------------------------------------------------------------------------------------

// Select the variable order which has the best weighted score
void metaheuristic_wscore(VariableOrderCriteria metavoc,
                          std::vector<int>& varorder, bool reorder_SCC,
                          const std::map<std::string, int>& S2Ipl) 
{
    std::vector<int> best_order(npl), order(npl);
    VariableOrderCriteria best_voc = VOC_NO_REORDER;
    double best_score = -1, score;

    // Try computing P-invariants
    // bool has_pinv = try_compute_pinvariants();
    int num_pinvs = test_have_pinvariants();
    bool has_pinv = (num_pinvs >= 1);
    double P_Pinv_ratio = double(npl) / (num_pinvs + 1.0);
    bool has_nu = model_has_nested_units();

    struct { 
        VariableOrderCriteria voc; // Tested algorithm in the meta-heuristic
        double weight;             // A weight that counter-balances NES
        bool selectable;           // Is it selectable for the current instance?
    }
    algoList[] {
        { VOC_SLOAN,          1.00, true },
        { VOC_SLOAN_1_16,     1.15, true },
        { VOC_TOVCHIGRECHKO,  1.35, true },
        { VOC_NOACK,          1.35, (npl<500 && ntr<500) }, // Noack is expensive to compute.
        { VOC_CUTHILL_MCKEE,  1.01, true },
        { VOC_PINV,           1.0, has_pinv && (npl < 1000) }, // P-INV does not scale well
        { VOC_GRADIENT_P,     1.0, has_pinv },
        { VOC_MEAS_FORCE_PINV,0.50, has_pinv },
        { VOC_NO_REORDER /* end of list */ }
    };

    // Test all the variable orderings in algoList[]
    for (int m=0; algoList[m].voc != VOC_NO_REORDER; m++) {
        const VariableOrderCriteria voc = algoList[m].voc;
        if (!algoList[m].selectable)
            continue;

        assert(!method_uses_pinvs(voc) || has_pinv);
        assert(!method_uses_nested_units(voc) || model_has_nested_units());

        // Get the order & its (weighted) score
        determine_var_order(voc, false, reorder_SCC, S2Ipl, order, false);
        score = measure_NES(order) * algoList[m].weight;
        if (!running_for_MCC()) {
            cout << "Variable Order " << var_order_name(voc).first << " got score: " << score << endl;
        }
        if (best_score == -1 || score < best_score) {
            best_order.swap(order);
            best_voc = voc;
            best_score = score;
        }
    }

    if (!running_for_MCC()) {
        cout << "Meta-heuristic: selecting method " << var_order_name(best_voc).first 
             << " with score: " << best_score << endl;
    }

    varorder = std::move(best_order);
}

