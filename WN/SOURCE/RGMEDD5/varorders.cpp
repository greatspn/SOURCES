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
#include <unistd.h>
#include <sys/wait.h>

#include "rgmedd5.h"
#include "varorders.h"
#include "utils/mt64.h"
#include "utils/reverse_heap.h"
#include "optimization_finder.h"
#include "parallel.h"
#include "irank.h"

//---------------------------------------------------------------------------------------


extern bool g_show_var_order;
extern bool g_exit_after_varorder;
extern bool g_print_varorder_metrics;
extern bool g_print_pbasis_metrics;
// extern bool g_save_incidence_as_image;
extern bool g_save_incidence_as_eps;
extern const char* g_incidence_file;
extern bool g_open_saved_incidence;
extern unsigned long long g_random_order_seeds[2];
// size_t g_sim_ann_num_tentatives = 50000;
extern const char* g_given_varorder;

//---------------------------------------------------------------------------------------

enum MetricFn { MF_EVSPANS, MF_UPEVSPANS, MF_UPEVSPANSx, MF_UNIQSPANS, MF_NONE }; // metric function
const char *mf_name[] = { "ES", "UPS", "UPSx", "US", "1" };
enum LevelWeight { LW_INV, LW_PSF, LW_RANGES, LW_RANGES_SHARE, LW_NONE }; // per-level weight
const char *lw_name[] = { "I", "PSF", "RNG", "RS", "1" };

//---------------------------------------------------------------------------------------

std::pair<MetricFn, LevelWeight>
decompose_SO_metric(int m) {
    MetricFn mf;
    LevelWeight lw;
    switch (m) {
        case METRIC_SO_ES_wI:      mf=MF_EVSPANS;     lw=LW_INV;           break;
        case METRIC_SO_ES_wPSF:    mf=MF_EVSPANS;     lw=LW_PSF;           break;
        case METRIC_SO_ES_wRNG:    mf=MF_EVSPANS;     lw=LW_RANGES;        break;
        case METRIC_SO_ES_wRS:     mf=MF_EVSPANS;     lw=LW_RANGES_SHARE;  break;
        case METRIC_SO_ES_w1:      mf=MF_EVSPANS;     lw=LW_NONE;          break;
        case METRIC_SO_UPS_wI:     mf=MF_UPEVSPANS;   lw=LW_INV;           break;
        case METRIC_SO_UPS_wPSF:   mf=MF_UPEVSPANS;   lw=LW_PSF;           break;
        case METRIC_SO_UPS_wRNG:   mf=MF_UPEVSPANS;   lw=LW_RANGES;        break;
        case METRIC_SO_UPS_wRS:    mf=MF_UPEVSPANS;   lw=LW_RANGES_SHARE;  break;
        case METRIC_SO_UPS_w1:     mf=MF_UPEVSPANS;   lw=LW_NONE;          break;
        case METRIC_SO_UPSx_wI:    mf=MF_UPEVSPANSx;  lw=LW_INV;           break;
        case METRIC_SO_UPSx_wPSF:  mf=MF_UPEVSPANSx;  lw=LW_PSF;           break;
        case METRIC_SO_UPSx_wRNG:  mf=MF_UPEVSPANSx;  lw=LW_RANGES;        break;
        case METRIC_SO_UPSx_wRS:   mf=MF_UPEVSPANSx;  lw=LW_RANGES_SHARE;  break;
        case METRIC_SO_UPSx_w1:    mf=MF_UPEVSPANSx;  lw=LW_NONE;          break;
        case METRIC_SO_US_wI:      mf=MF_UNIQSPANS;   lw=LW_INV;           break;
        case METRIC_SO_US_wPSF:    mf=MF_UNIQSPANS;   lw=LW_PSF;           break;
        case METRIC_SO_US_wRNG:    mf=MF_UNIQSPANS;   lw=LW_RANGES;        break;
        case METRIC_SO_US_wRS:     mf=MF_UNIQSPANS;   lw=LW_RANGES_SHARE;  break;
        case METRIC_SO_US_w1:      mf=MF_UNIQSPANS;   lw=LW_NONE;          break;
        case METRIC_SO_1_wI:       mf=MF_NONE;        lw=LW_INV;           break;
        case METRIC_SO_1_wPSF:     mf=MF_NONE;        lw=LW_PSF;           break;
        case METRIC_SO_1_wRNG:     mf=MF_NONE;        lw=LW_RANGES;        break;
        case METRIC_SO_1_wRS:      mf=MF_NONE;        lw=LW_RANGES_SHARE;  break;

        default:
            throw rgmedd_exception("Not an SO_x_y metric!");
    }
    return make_pair(mf, lw);
}

//---------------------------------------------------------------------------------------

uint64_t metric_SO_x_w1(int mf, const std::vector<int> &net_to_mddLevel,
                        trans_span_set_t& trns_set) 
{
    switch (mf) {
        case MF_UPEVSPANS:
            return measure_soups(net_to_mddLevel, trns_set);

        case MF_UNIQSPANS:
            return measure_soups(net_to_mddLevel, trns_set, false);

        case MF_UPEVSPANSx:
            return measure_soups2(net_to_mddLevel, trns_set);

        case MF_EVSPANS:
            return measure_SOS(net_to_mddLevel);
            
        case MF_NONE:
        default: 
            throw rgmedd_exception();
    }
}

//---------------------------------------------------------------------------------------

template<class WEIGHT>
WEIGHT metric_SO_x_W(int mf, const std::vector<int> &net_to_mddLevel,
                     trans_span_set_t& trns_set, const std::vector<WEIGHT>& W) 
{
    switch (mf) {
        case MF_EVSPANS:
            return measure_weighted_event_spans(net_to_mddLevel, &W);

        case MF_UPEVSPANS:
            return measure_swir(net_to_mddLevel, trns_set, W);

        case MF_UNIQSPANS:
            return measure_swir(net_to_mddLevel, trns_set, W, false);

        // case MF_UPEVSPANSx:
        //     if (pW)
        //         return to_double(measure_swir_X(net_to_mddLevel, trns_set, W));
        //     else
        //         return measure_soups_X(net_to_mddLevel, trns_set);
        //     break;
        case MF_UPEVSPANSx:
            return measure_swir2(net_to_mddLevel, trns_set, W);
            // m *= get_soups_discount(trns_set);

        case MF_NONE:
            return std::accumulate(W.begin(), W.end(), WEIGHT(0));

        default: throw rgmedd_exception();
    }
}

//---------------------------------------------------------------------------------------

template<typename T> double to_double(T x) { return double(x); }
template<> double to_double<cardinality_t>(cardinality_t x) { return get_double(x); }

template<class WEIGHT>
double compute_SO_x_W_metric(int mf, const std::vector<int> &net_to_mddLevel,
                             trans_span_set_t& trns_set, const std::vector<WEIGHT>* pW) 
{
    double m;
    switch (mf) {
        case MF_EVSPANS:
            m = to_double(measure_weighted_event_spans(net_to_mddLevel, pW));
            break;

        case MF_UPEVSPANS:
            if (pW)
                m = to_double(measure_swir(net_to_mddLevel, trns_set, *pW));
            else
                m = measure_soups(net_to_mddLevel, trns_set);
            break;

        case MF_UNIQSPANS:
            if (pW)
                m = to_double(measure_swir(net_to_mddLevel, trns_set, *pW, false));
            else
                m = measure_soups(net_to_mddLevel, trns_set, false);
            break;

        // case MF_UPEVSPANSx:
        //     if (pW)
        //         m = to_double(measure_swir_X(net_to_mddLevel, trns_set, *pW));
        //     else
        //         m = measure_soups_X(net_to_mddLevel, trns_set);
        //     break;
        case MF_UPEVSPANSx:
            if (pW)
                m = to_double(measure_swir2(net_to_mddLevel, trns_set, *pW));
            else
                m = measure_soups2(net_to_mddLevel, trns_set);
            // m *= get_soups_discount(trns_set);
            break;

        case MF_NONE:
            m = to_double(std::accumulate(pW->begin(), pW->end(), WEIGHT(0)));
            break;

        default: throw rgmedd_exception();
    }
    return m;
}

//---------------------------------------------------------------------------------------

// Handle a SIGUSR1 signal received from a child process
static volatile sig_atomic_t received_sigusr1 = false;
void handle_sigusr1_finished_statespace(int) {
    if (!running_for_MCC())
        cout << "RECEIVED SIGUSR1" << endl;
    received_sigusr1 = true;
}

//---------------------------------------------------------------------------------------

template<class RandomIt, class RandomFunc>
void my_random_shuffle(RandomIt first, RandomIt last, RandomFunc&& r)
{
    typename std::iterator_traits<RandomIt>::difference_type i, n;
    n = last - first;
    for (i = n-1; i > 0; --i) {
        using std::swap;
        swap(first[i], first[r(i+1)]);
    }
}

//---------------------------------------------------------------------------------------
// Entry point - apply a variable order heuristics, with optional refinements
//---------------------------------------------------------------------------------------

void determine_var_order(const var_order_selector& sel, 
	                     const std::map<const char*, int, cstr_less>& S2Ipl, 
                         std::vector<int> &net_to_mddLevel,
                         trans_span_set_t &trns_set,
                         flow_basis_metric_t& fbm) 
{
    const bool is_meta_heuristics = (sel.p_meta_score != nullptr);
    clock_t var_order_time = clock();
    if (sel.heuristics != VOC_INPUT_ORDER) {
    	net_to_mddLevel.resize(npl);
        std::fill(net_to_mddLevel.begin(), net_to_mddLevel.end(), -1);
    }
    const char *short_name, *long_name;
    std::tie(short_name, long_name) = var_order_name(sel.heuristics);
    const int_lin_constr_vec_t empty_ilcp;
    if (!running_for_MCC() && sel.verbose) {
    	cout << "Variable order method: " << long_name << endl;
    }

    bool invert_mapping = false;
    bool fill_missing_vars = false;
    bool is_meta = false;
    VariableOrderMetric ann_metric = sel.ann_metric;

    compute_connected_components();

    switch (sel.heuristics) {
	    case VOC_NO_REORDER:
        case VOC_TOPOLOGICAL: 
        case VOC_RANDOM:
            // test_metric_scores();
	        // Places appear int the MDD levels in the same order of the .net file
	        for (size_t i = 0; i < npl; i++)
	            net_to_mddLevel[i] = i;
            if (sel.heuristics == VOC_RANDOM) { // reshuffle
                my_random_shuffle(net_to_mddLevel.begin(), net_to_mddLevel.end(),
                                  [](int n){ return genrand64_int63() % n; });
                // for (size_t i = 0; i < npl; i++)
                //     std::swap(net_to_mddLevel[genrand64_int63() % npl],
                //               net_to_mddLevel[genrand64_int63() % npl]);
            }
	        break;

        case VOC_INPUT_ORDER:
            break; // net_to_mddLevel must be initialized

        case VOC_DFS:
        case VOC_BFS: {
            var_order_visit(sel.heuristics, net_to_mddLevel, 0);
            break;
        }

        case VOC_PCHAINING:
            varorder_P_chaining(get_int_constr_problem(), net_to_mddLevel);
            fill_missing_vars = true;
            invert_mapping = true;
            break;

	    case VOC_PINV: {
            if (get_int_constr_problem().empty())
	    	// if (read_PIN_file())
	    		throw rgmedd_exception("Cannot sort using P-euristics.");
	        int *pinv_order = sort_according_to_pinv();
	        free_PIN_file();
	        std::copy(pinv_order, pinv_order + npl, net_to_mddLevel.begin());
	        free(pinv_order);
	        fill_missing_vars = true;
	        invert_mapping = true;
	        break;
	    }
	    case VOC_FROM_FILE: 
	        var_order_from_file(net_to_mddLevel, S2Ipl);
	        fill_missing_vars = true;
	        break;

        case VOC_FORCE:
            var_order_visit(VOC_BFS, net_to_mddLevel, 0);
            var_order_FORCE(sel.heuristics, net_to_mddLevel, net_to_mddLevel, 
                            empty_ilcp, sel.verbose);
            break;

        case VOC_FORCE_PINV:
            var_order_visit(VOC_BFS, net_to_mddLevel, 0);
            var_order_FORCE(sel.heuristics, net_to_mddLevel, net_to_mddLevel, 
                            get_int_constr_problem(), sel.verbose);
            break;

        case VOC_FORCE_NU:
            var_order_visit(VOC_BFS, net_to_mddLevel, 0);
            var_order_FORCE(sel.heuristics, net_to_mddLevel, net_to_mddLevel, 
                            convert_nested_units_as_ilcp(), sel.verbose);
            break;

	    case VOC_CUTHILL_MCKEE:
	        var_order_cuthill_mckee(sel.heuristics, net_to_mddLevel);
	        break;

	    case VOC_KING:
	        var_order_king_ordering(sel.heuristics, net_to_mddLevel);
	        break;

        case VOC_SLOAN:
	    case VOC_SLOAN_1_16:
	        var_order_sloan(sel.heuristics, net_to_mddLevel);
	        break;

	    case VOC_NOACK:
        case VOC_TOVCHIGRECHKO:
	        var_order_noack_tovchigrechko(sel.heuristics, net_to_mddLevel);
	        std::reverse(net_to_mddLevel.begin(), net_to_mddLevel.end());
	        invert_mapping = true;
	        break;

        case VOC_NOACK2:
        case VOC_TOVCHIGRECHKO2:
            var_order_Noack_Tovchigrechko_fast(sel.heuristics, net_to_mddLevel);
            break;

	    case VOC_GRADIENT_P:
            var_order_gradient_P(sel.heuristics, get_int_constr_problem(), net_to_mddLevel);
            invert_mapping = true;
            break;

        case VOC_GRADIENT_NU:
            var_order_gradient_P(sel.heuristics, convert_nested_units_as_ilcp(), net_to_mddLevel);
            invert_mapping = true;
            break;

        // case VOC_MARKOV_CLUSTER:
	    // 	var_order_gradient_P(sel.heuristics, mcl_cluster_net(), net_to_mddLevel);
	    // 	invert_mapping = true;
	    // 	break;

        case VOC_VCL_CUTHILL_MCKEE:
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:
            var_order_vcl(sel.heuristics, net_to_mddLevel);
            invert_mapping = true;
            break;

        case VOC_META_FORCE:
            var_order_meta_force(net_to_mddLevel, trns_set, &fbm, sel.target_metric, 1000);
            break;

	    case VOC_META_BY_SCORE: 
            {
                std::vector<score_vo_t> varorders;
    	    	// metaheuristic_wscore(sel.heuristics, varorders, sel.num_tested_varorders, 
          //                            ann_metric, trns_set, fbm, S2Ipl);
                metaheuristic_context_t mhctx {
                    .metavoc = sel.heuristics,
                    .trns_set = trns_set,
                    .fbm = fbm,
                    .S2Ipl = S2Ipl
                };
                metaheuristic(mhctx, varorders, sel.num_tested_varorders);
                // if (varorders.size() == 1 && 
                //     ann_metric == VariableOrderMetric::METRIC_SWIR && 
                    // varorders[0].voc == VOC_MARKOV_CLUSTER)
                    // ann_metric = VariableOrderMetric::METRIC_SWIR_X;

                // cout << "sel.num_parallel_procs = " << sel.num_parallel_procs << endl;
                // cout << "sel.time_for_each_test = " << sel.time_for_each_test << endl;
                // cout << "sel.num_tested_varorders = " << sel.num_tested_varorders << endl;
                // cout << "g_max_seconds_statespace = " << g_max_seconds_statespace << endl;

                is_meta = true;

                size_t selected;
                if (varorders.size() == 1 && sel.num_parallel_procs < 0)
                    selected = 0;
                else {
                    // try all selected variable orders in parallel batches
                    int completing_varorder = -1;
                    std::vector<int> exitcodes;
                    int task_id = parallel_exec(sel.num_parallel_procs,
                                                varorders.size(), 
                                                &completing_varorder,
                                                exitcodes, 
                                                nullptr,
                                                false && !running_for_MCC());

                    if (task_id < 0) { // all parallel tasks have finished
                        if (completing_varorder >= 0) 
                            exit(WEXITSTATUS(exitcodes[completing_varorder]));

                        if (running_for_MCC())
                            printf("CANNOT_COMPUTE\n");
                        exit(EXIT_TIMEOUT_STATESPACE);
                    }
                    else { // child subprocess running for statespace
                        selected = task_id;
                        g_statespace_runs_as_subprocess = true;

                        // if (selected < varorders.size() - sel.num_parallel_procs) {
                        //     // last processes have no time limit
                        //     // all the other have the strict time bound
                            g_max_seconds_statespace = sel.time_for_each_test; 
                        // }
                    }


                    // // Try multiple variable orders
                    // for (size_t i=0; i<varorders.size(); i++) {
                    //     bool last_tentative = (i == varorders.size() - 1);
                    //     signal(SIGUSR1, handle_sigusr1_finished_statespace);
                    //     // Create the child process
                    //     pid_t pid = fork();
                    //     if (pid < 0) {
                    //         cerr << "Could not fork()" << endl;
                    //         exit(EXIT_FAILURE_RGMEDD);
                    //     }
                    //     if (pid == 0) { // Child process
                    //         signal(SIGUSR1, SIG_DFL);
                    //         // Select the i-th variable order and start the timer.
                    //         selected = i;
                    //         if (!last_tentative)
                    //             g_max_seconds_statespace = sel.time_for_each_test; // last process has no timer
                    //         break;
                    //     }
                    //     else { // Parent process
                    //         int exitcode;
                    //         pid_t term_pid;
                    //         do {
                    //             term_pid = waitpid(pid, &exitcode, 0);
                    //         } while (term_pid != pid);
                    //         if (!running_for_MCC())
                    //             cout << "\nCHILD EXIT STATUS " << WEXITSTATUS(exitcode) << endl;
                    //         if (WEXITSTATUS(exitcode) == 0)
                    //             exit(EXIT_SUCCESS); // everything has been done by the child process.

                    //         // Do not restart after the last tentative or if the child notified 
                    //         // that the state space was completed successfully (using SIGUSR1)
                    //         if (last_tentative || received_sigusr1)
                    //             exit(WEXITSTATUS(exitcode));
                    //     }
                    // }
                }
                // Get the selected variable order
                net_to_mddLevel = std::move(varorders[selected].order);
                if (!running_for_MCC()) {
                    cout << "Meta-heuristic: selecting method " 
                         << var_order_name(varorders[selected].voc).first 
                         << varorders[selected].variation()
                         << " with score: " << varorders[selected].score;
                    if (g_max_seconds_statespace > 0)
                        cout << "(max time: " << g_max_seconds_statespace << " sec)";
                    cout << endl;
                }
            }
	    	break;

	    default:
	        throw rgmedd_exception("Internal error: Unknown variable order heuristics.\n");
    }
    assert(net_to_mddLevel.size() == npl);
    // std::reverse(net_to_mddLevel.begin(), net_to_mddLevel.end());

    if (invert_mapping) {
        int inv[npl];
        for (size_t i = 0; i < npl; i++)
            inv[i] = -1;
        for (size_t i = 0; i < npl; i++)
            if (net_to_mddLevel[i] != -1)
                inv[net_to_mddLevel[i]] = i;
        for (size_t i = 0; i < npl; i++)
            net_to_mddLevel[i] = inv[i];
    }

    if (fill_missing_vars) {
        // PINV and file order could have left some places unassigned (-1 in net_to_mddLevel)
        // Assign them with the remaining indexes
        int num_assigned = 0;
        for (size_t i = 0; i < npl; i++)
            if (net_to_mddLevel[i] != -1)
                num_assigned++;

        for (size_t i = 0; i < npl; i++)
            if (net_to_mddLevel[i] == -1)
                net_to_mddLevel[i] = num_assigned++;

        if (num_assigned != npl) {
            cout << "Internal error: not all places have been assigned to MDD levels." << endl;
            exit(EXIT_FAILURE);
        }
    }

    //----------------------------------------------------
    // Improvement steps of the obtained variable order
    //----------------------------------------------------

    // If the net is non-ergodic, reorder from bottom SCC to top SCC (if the heuristic agrees)
    bool topo_sort = (sel.heuristics == VOC_TOPOLOGICAL);
    if (sel.reorder_SCC || topo_sort) {
        reorder_using_SCC(net_to_mddLevel);
    }

    // reverse top with bottom levels
    if (sel.reverse_order)
        for (size_t i = 0; i < npl; i++)
            net_to_mddLevel[i] = npl - 1 - net_to_mddLevel[i];


    // Apply again a FORCE method over the generated variable ordering
    ForceBasedRefinement refinement = sel.refinement;
    if (refinement == ForceBasedRefinement::BEST_AVAILABLE) {        
        const bool use_ForceTI = (get_num_invariants() > 1 && 
                                  get_num_invariants() < 100 && 
                                  get_max_invariant_coeff() < 100 &&
                                  ntr > 0);
        const bool use_ForceConstr = (get_int_constr_problem().size() > 1);
        if (use_ForceTI)
            refinement = ForceBasedRefinement::FORCE_TI;
        else if (use_ForceConstr)
            refinement = ForceBasedRefinement::FORCE_CONSTR;
        else
            refinement = ForceBasedRefinement::FORCE;
    }
    switch (refinement) {
        case ForceBasedRefinement::NO_REFINEMENT:
            break;

        case ForceBasedRefinement::FORCE:
            var_order_FORCE(VOC_FORCE, net_to_mddLevel, net_to_mddLevel, empty_ilcp, sel.verbose);
            break;

        case ForceBasedRefinement::FORCE_CONSTR:
            var_order_FORCE(VOC_FORCE_PINV, net_to_mddLevel, net_to_mddLevel, get_int_constr_problem(), sel.verbose);
            break;

        case ForceBasedRefinement::FORCE_TI:
            var_order_ti_force(fbm, trns_set, net_to_mddLevel, true, sel.verbose);
            break;

        case ForceBasedRefinement::FORCE_I:
            var_order_ti_force(fbm, trns_set, net_to_mddLevel, false, sel.verbose);
            break;

        default:
            throw rgmedd_exception("Unknown refinement");
    }
    
    // Keep the order that has the better SOUPS value between 
    // the computed order (bottom to top) and its reverse (top to bottom).
    if (sel.reverse_if_better_soups) {
        uint64_t SOUPS1 = measure_soups(net_to_mddLevel, trns_set);

        // reverse the order and recompute the metrics
        for (size_t i = 0; i < npl; i++)
            net_to_mddLevel[i] = npl - 1 - net_to_mddLevel[i];
        uint64_t SOUPS2 = measure_soups(net_to_mddLevel, trns_set);

        if (SOUPS1 < SOUPS2) { // reverse again (keep the original)
            cout << "Using direct order." << endl;
            for (size_t i = 0; i < npl; i++)
                net_to_mddLevel[i] = npl - 1 - net_to_mddLevel[i];
        }
        else
            cout << "Using reverse order." << endl;
    }

    // Try simulated annealing by groups to improve a target metric
    switch (sel.group_annealing) {
        case GroupAnnealing::NO_GROUP_ANN:
            break;

        case GroupAnnealing::GRPANN_INV:
            if (get_num_invariants() > 0)
                simulated_annealing(net_to_mddLevel, trns_set, &fbm, 
                                    VariableOrderMetric::METRIC_SWIR, true, false);
            break;

        case GroupAnnealing::GRPANN_INV_FORCE:
            if (get_num_invariants() > 0)
                simulated_annealing(net_to_mddLevel, trns_set, &fbm, 
                                    VariableOrderMetric::METRIC_SWIR, true, true);
            break;
    }

    // Try simulated annealing to improve a target metric
    if (sel.annealing != Annealing::NO_ANN) {
        if (!running_for_MCC())
            cout << "Simulated annealing using " << name_of_annealing(sel.annealing, ann_metric) << "..." << endl;
        switch (sel.annealing) {
            case Annealing::NO_ANN: break;

            case Annealing::ANN_PBASIS:
                annealing_compact(net_to_mddLevel, fbm);
                break;

            case Annealing::ANN_PBASIS_MIN:
                compact_basis_row_min(net_to_mddLevel, fbm);
                break;

            case Annealing::ANN_MINIMIZE_METRIC:
                try { // May break if P-basis footprint transformation overflows
                    simulated_annealing(net_to_mddLevel, trns_set, &fbm, ann_metric, false, false); 
                }
                catch (rgmedd_exception& e) {
                    cout << "Failed: " << e.what() << endl;
                }
                break;

            case Annealing::ANN_FORCE:
            case Annealing::ANN_FORCE_P:
            case Annealing::ANN_FORCE_TI:
                annealing_force(sel.annealing, net_to_mddLevel, trns_set, &fbm, ann_metric);
                break;
        }
    }


    // Perform a fixed number of random swaps of the variables, if requested
    for (int nn=0; nn<sel.num_random_swaps; nn++) {
        std::swap(net_to_mddLevel[genrand64_int63() % npl],
                  net_to_mddLevel[genrand64_int63() % npl]);
    }


    // Print the place order we use
    {
        var_order_time = clock() - var_order_time;
        if (!running_for_MCC() && sel.verbose)
            cout << "Time to compute variable order: " << (var_order_time / double(CLOCKS_PER_SEC)) << endl;
        // cout << "VARORD (index): ";
        // for (int p=0; p<npl; p++)
        //     cout << net_to_mddLevel[p] << " ";
        // cout << endl << endl;

        std::vector<int> mddLevel_to_net(npl, -1);
        for (int p = 0; p < npl; p++) {
        	// Sanity checks
        	if (net_to_mddLevel[p] < 0 || net_to_mddLevel[p] >= npl)
        		throw rgmedd_exception("Out-of-bound values in variable order.");
        	if (mddLevel_to_net[ net_to_mddLevel[p] ] != -1)
        		throw rgmedd_exception("Variable order is not a valid permutation.");
	        // Get the place corresponding to each MDD level
            mddLevel_to_net[ net_to_mddLevel[p] ] = p;
        }
        
        if (g_show_var_order && !is_meta_heuristics) { 
            cout << "\nVARORDER: ";
            for (int p = 0; p < npl; p++)
                cout << " " << tabp[ mddLevel_to_net[p] ].place_name;
            cout << endl;
        }
    }


    // Print the invariant matrix in footprint form on the console
    if (g_print_pbasis_metrics && !is_meta_heuristics) {
        print_PSI_diagram(net_to_mddLevel, fbm);
    }

    // The meta-heuristic needs a score for the generated variable order.
    if (is_meta_heuristics) {
        bool initialize_invariants = (get_num_invariants() > 0);
        if (get_max_invariant_coeff() > 100)
            initialize_invariants = false; // High risk of overflows using SWIR.
        std::vector<size_t> lw_invs;

        cardinality_t iRank = -2;
        if (initialize_invariants) {
            // Initialize the fbm data structure
            iRank = measure_PSI(net_to_mddLevel, 
                true /*only_ranks*/, false /*use_ilp*/, false /*use_enum*/, false /*print_enums*/, fbm);

            // Extract level weights for SWIR
            get_lvl_weights_invariants(fbm, lw_invs);
        }


        // *sel.p_meta_score = measure_soups(net_to_mddLevel, trns_set);
        // return;

        if (!initialize_invariants) { // No invariants available, use SOUPS
            *sel.p_meta_score = measure_soups(net_to_mddLevel, trns_set);
            if (sel.p_meta_score_ex2)
                *sel.p_meta_score_ex2 = -1; // swir with no discount
            if (sel.p_meta_score_ex3)
                *sel.p_meta_score_ex3 = *sel.p_meta_score; // soups
        }
        else if (initialize_invariants && ntr == 0) { // use iRank
            *sel.p_meta_score = get_double(iRank);
            if (sel.p_meta_score_ex2)
                *sel.p_meta_score_ex2 = *sel.p_meta_score; // same
            if (sel.p_meta_score_ex3)
                *sel.p_meta_score_ex3 = *sel.p_meta_score; // same
        }
        else { // use SWIR
            *sel.p_meta_score = measure_swir2(net_to_mddLevel, trns_set, lw_invs);
            if (sel.p_meta_score_ex2)
                *sel.p_meta_score_ex2 = *sel.p_meta_score; // swir with no discount
            if (sel.p_meta_score_ex3)
                *sel.p_meta_score_ex3 = measure_soups(net_to_mddLevel, trns_set); // soups
        }

        // Extra metrics
        // if (sel.p_meta_score_ex2) {
        //     *sel.p_meta_score_ex2 = measure_soups(net_to_mddLevel, trns_set);
        // }
        // if (sel.p_meta_score_ex3 && initialize_invariants) {
        //     *sel.p_meta_score_ex3 = measure_swir3(net_to_mddLevel, trns_set, lw_invs);
        // }
        // if (sel.p_meta_score_ex4) {
        //     *sel.p_meta_score_ex4 = measure_soups3(net_to_mddLevel, trns_set);
        // }

        if (sel.p_meta_score_ex4)
            *sel.p_meta_score_ex4 = 1;

        if (sel.discount_score) {
            double discount = get_soups_discount(trns_set);
            // if (!running_for_MCC() && discount != 1.0)
            //     cout << "  applying discount = " << discount << endl;
            discount *= discount;
            *sel.p_meta_score *= discount;
            if (sel.p_meta_score_ex4)
                *sel.p_meta_score_ex4 = discount;
        }
    }

    // Compute and show all the implemented metrics for this variable order
    if (g_print_varorder_metrics & !is_meta_heuristics) 
    {
        // // Initialize the fbm data structure for PSI (including ranges)
        // measure_PSI(net_to_mddLevel, 
        //             false /*only_ranks*/, false /*use_ilp*/, 
        //             false /*use_enum*/, false /*print_enums*/, fbm);

        // // Extract level weights for SWIR
        // std::vector<size_t> lw_invs;
        // get_lvl_weights_invariants(fbm, lw_invs);

        // // Extract level weights for PSI
        // std::vector<cardinality_t> lw_ranges;
        // get_lvl_weights_ranges(fbm, lw_ranges);
        // cardinality_t PSI = std::accumulate(lw_ranges.begin(), lw_ranges.end(), cardinality_t(0));

        // // Account for (approximate)  sharing in ranges
        // std::vector<double> lw_ranges_share(npl);
        // for (size_t i=0; i<npl; i++)
        //     lw_ranges_share[i] = get_double(lw_ranges[i]) / lw_invs[i];

        // // level weights using P-semiflows
        // const flow_basis_t& psf = load_Psemiflows();
        // const int* bounds = load_bounds();
        // std::vector<size_t> lw_psf(npl, 0);//, lw_psf_bnds;
        // for (const sparse_vector_t& p : psf) {
        //     int lvlbot, lvltop;
        //     lvlbot = lvltop = net_to_mddLevel[p.ith_nonzero(0).index];
        //     for (auto& e : p) {
        //         lvlbot = min(lvlbot, net_to_mddLevel[e.index]);
        //         lvltop = max(lvltop, net_to_mddLevel[e.index]);
        //     }
        //     // compute the p-semiflow span
        //     for (int lvl = lvlbot; lvl <= lvltop; lvl++) {
        //         lw_psf[lvl]++;
        //     }
        // }
        // // lw_psf_bnds = lw_psf;
        // // // Weight the invariants with the place bounds
        // // for (size_t plc=0; plc<npl; plc++) {
        // //     int bound = bounds ? bounds[plc] : 1;
        // //     if (bound >= 1) {
        // //         lw_psf_bnds[ net_to_mddLevel[plc] ] *= (bound + 1); // include the zero!;
        // //     }
        // // }


        // cardinality_t iRank = measure_PSI(net_to_mddLevel, true, false, false, false, fbm);
        // cout << "iRank: " << iRank << endl;

        // swir_score_t SWIR = measure_swir(net_to_mddLevel, trns_set, lw_invs);
        // clock_t time_SOUPS = clock();
        // uint64_t SOUPS = measure_soups(net_to_mddLevel, trns_set);
        // time_SOUPS = clock() - time_SOUPS;
        // double soups_discount = get_soups_discount(trns_set);

        // // swir_score_t SWIRorig = measure_swir_orig(net_to_mddLevel, trns_set, lw_invs);
        // // uint64_t SOUPSorig = measure_soups_orig(net_to_mddLevel, trns_set);

        // cout << "\nMETRICS:\n";
        // cout << "NES:         " << measure_NES(net_to_mddLevel) << endl;
        // cout << "WES(1):      " << measure_WESi(net_to_mddLevel, 1) << endl;
        // cout << "PSF:         " << measure_PSF(net_to_mddLevel) << endl;
        // cout << "PF:          " << measure_PF(net_to_mddLevel) << endl;
        // // cout << "NUS:         " << measure_NUS(net_to_mddLevel) << endl;
        // cout << "SOT:         " << measure_SOT(net_to_mddLevel) << endl;
        // cout << "SOS:         " << measure_SOS(net_to_mddLevel) << endl;
        // // cout << "SOUPS-OLD:   " << SOUPS << endl;
        // cout << "SOUPS:       " << SOUPS << endl;
        // cout << "SOUPS2:      " << measure_soups2(net_to_mddLevel, trns_set) << endl;
        // cout << "discount:    " << soups_discount << endl;
        // // cout << "SOUPSorig:   " << SOUPSorig << endl;
        // // cout << "TIME-SOUPS_OLD: " << double(time_SOUPS) / CLOCKS_PER_SEC << endl;
        // cout << "TIME-SOUPS:     " << double(time_SOUPS) / CLOCKS_PER_SEC << endl;
        // // cout << "SWIR-OLD     " << SWIR << endl;
        // cout << "SWIR:        " << SWIR << endl;
        // // cout << "SOUPS3:      " << measure_soups3(net_to_mddLevel, trns_set) << endl;
        // // cout << "SWIR3:        " << measure_swir3(net_to_mddLevel, trns_set, lw_invs) << endl;
        // // cout << "SWIRorig:    " << SWIRorig << endl;
        // // cout << "SOS:         " << SOS << endl;
        // cout << "B-RANK:      " << B_RANK << endl;
        // cout << "PSI:         " << PSI << endl;

        // double PTS = measure_FORCE_pts(VOC_FORCE, net_to_mddLevel, empty_psf), PTS_P = PTS;
        // if (get_num_Psemiflows() > 0)
        //     PTS_P = measure_FORCE_pts(VOC_FORCE_PINV, net_to_mddLevel, load_Psemiflows());
        // double PTS_INVw = metric_PTS_INV(fbm, trns_set, net_to_mddLevel, true);
        // double PTS_INV1 = metric_PTS_INV(fbm, trns_set, net_to_mddLevel, false);


        // cout << "FORCE-PTS:   " << PTS << endl;
        // cout << "FORCE-PTS-P: " << PTS_P << endl;
        // cout << "FORCE-PTS-INVw: " << PTS_INVw << endl;
        // cout << "FORCE-PTS-INV1: " << PTS_INV1 << endl;
        // // cout << "TIME-B-RANK: " << double(time_B_RANK) / CLOCKS_PER_SEC << endl;

        // for (int mf=MF_EVSPANS; mf <= MF_NONE; mf++) {
        //     for (int lw=LW_INV; lw <= LW_NONE; lw++) {
        //         if (mf == MF_NONE && lw == LW_NONE)  
        //             continue;

        //         const std::vector<size_t>* no_weights = nullptr;
        //         double m;
        //         switch (lw) {
        //             case LW_INV:
        //                 m = compute_SO_x_W_metric(mf, net_to_mddLevel, trns_set, &lw_invs);
        //                 break;

        //             case LW_PSF:
        //                 m = compute_SO_x_W_metric(mf, net_to_mddLevel, trns_set, &lw_psf);
        //                 break;

        //             case LW_RANGES:
        //                 m = compute_SO_x_W_metric(mf, net_to_mddLevel, trns_set, &lw_ranges);
        //                 break;

        //             case LW_RANGES_SHARE:
        //                 m = compute_SO_x_W_metric(mf, net_to_mddLevel, trns_set, &lw_ranges_share);
        //                 break;

        //             case LW_NONE:
        //                 m = compute_SO_x_W_metric(mf, net_to_mddLevel, trns_set, no_weights);
        //                 break;

        //         }
        //         cout << "SO_" << mf_name[mf]<<"_w"<<lw_name[lw]<<":   "<<m<<endl;
        //     }

#if 1
            cout << "\n--------------------------------------\n";
            // clock_t time_PSI_RANK = clock();
            cardinality_t PSI_RANK = measure_PSI(net_to_mddLevel, false, false, false, false, fbm);
            // print_PSI_diagram(net_to_mddLevel, fbm);
            cout << "score1:  " << PSI_RANK << "    (full product of ranges)" <<endl;

            cardinality_t score2 = measure_score_experimental(fbm, 0);
            cout << "score2:  " << score2 << "    (hyper-triangular product< / n!)" << endl;

            cardinality_t score3 = measure_score_experimental(fbm, 1);
            cout << "score3:  " << score3 << "    (hyper-triangular product> / n!)" << endl;

            cardinality_t score4 = measure_score_experimental(fbm, 2);
            cout << "score4:  " << score4 << "    (hypervolume? product of ranges / n)" << endl;

            cardinality_t score5 = measure_score_experimental(fbm, 10);
            cout << "score5:  " << score5 << "    ( new method - nodes )" << endl;

            cardinality_t score6 = measure_score_experimental(fbm, 11);
            cout << "score6:  " << score6 << "    ( new method - edges )" << endl;

            // time_PSI_RANK = clock() - time_PSI_RANK;
            // cout << "score4:  " << score4 << "    (...)" << endl;
            cout << "\n--------------------------------------\n";
            cout << endl;
            // cout << "TIME-PSI:  " << double(time_PSI_RANK) / CLOCKS_PER_SEC << endl;
            // if (g_print_pbasis_metrics)
            experiment_footprint_chaining(net_to_mddLevel);
            cout << "\n--------------------------------------\n";

#endif

        // }

        // // Print bandwitdh, profile and wavefront metrics
        // uint64_t BW, PROF;
        // double AVGWF, MAXWF, RMSWF;
        // metrics_bandwidth(net_to_mddLevel, &BW, &PROF, &AVGWF, &MAXWF, &RMSWF);
        // cout << "BANDWIDTH:        " << BW << endl;
        // cout << "PROFILE:          " << PROF << endl;
        // cout << "MAX-WAVEFRONT:    " << MAXWF << endl;
        // cout << "AVG-WAVEFRONT:    " << AVGWF << endl;
        // cout << "RMS-WAVEFRONT:    " << RMSWF << endl;



        metric_t all_metrics[NUM_METRICS];
        metric_t *ptrs[NUM_METRICS];
        for (int m=0; m<NUM_METRICS; m++) {
            ptrs[m] = &all_metrics[m];
            if (METRIC_BEGIN_SO_x_y <= m && m < METRIC_END_SO_x_y)
                ptrs[m] = nullptr;
        }

        metric_compute(net_to_mddLevel, trns_set, &fbm, ptrs);
        for (int m=0; m<NUM_METRICS; m++) {
            if (ptrs[m]) {
                const char* name = metric_name(VariableOrderMetric(m));
                cout << "  metric[" << name << "]: " << setw(13 - strlen(name)) << " ";
                cout << all_metrics[m] << endl;
            }
        }



        // clock_t time_PSI_LP = clock();
        // cardinality_t PSI_LP = measure_PSI(net_to_mddLevel, false, true, false, false, *fbm);
        // time_PSI_LP = clock() - time_PSI_LP;
        // cout << "PSI-LP:      " << PSI_LP << endl;
        // cout << "TIME-PSI-LP: " << double(time_PSI_LP) / CLOCKS_PER_SEC << endl;

        // clock_t time_SoIR = clock();
        // size_t SoIR = measure_SoIR(net_to_mddLevel);
        // time_SoIR = clock() - time_SoIR;
        // cout << "SoIR:        " << SoIR << endl;
        // cout << "TIME-SoIR:   " << double(time_SoIR) / CLOCKS_PER_SEC << endl;

        // clock_t time_PSI_EN = clock();
        // cardinality_t PSI_EN = measure_PSI(net_to_mddLevel, false, true, true, 
        //                                    g_print_pbasis_metrics, *static_cast<flow_basis_metric_t*>(&fbm));
        // time_PSI_EN = clock() - time_PSI_EN;
        // cout << "PSI-EN:      " << PSI_EN << endl;
        // cout << "TIME-PSI-EN: " << double(time_PSI_EN) / CLOCKS_PER_SEC << endl;

        // if (g_save_incidence_as_image) {
        //     std::string inc_name = net_name;
        //     inc_name += "inc.ppm";
        //     cout << "Writing incidence matrix to " << inc_name << " ..." << endl;
        //     write_incidence_as_NetPBM(inc_name.c_str(), trns_set);
        //     if (g_open_saved_incidence) 
        //         open_file(inc_name.c_str());
        // }
    }

    // Save a representation of the incidence + invariant matrices in eps/pdf formats
    if (g_save_incidence_as_eps && !is_meta_heuristics) {
        std::string inc_name = std::string(net_name) + "inc";
        // inc_name += "inc.eps";
        if (g_incidence_file)
            inc_name = g_incidence_file;

        // uint64_t newSOUPS = measure_soups(net_to_mddLevel, trns_set);
        cout << "Writing incidence matrix to " << inc_name << " ..." << endl;
        trns_set.build_all_ioh_lists(net_to_mddLevel, true/* mark non-productives */);
        trns_set.compute_unique_productive_nodes(true/* sort transitions */, 
                                                 false /* no activation levels */);
        write_incidence_as_EPS((inc_name + ".eps").c_str(), trns_set, 
                               net_to_mddLevel, get_basis(fbm));
        // By using the --gsopt -sFONTPATH=... command, Ghostscript encapsulates 
        // arbitrary fonts in the resulting PDF
        // epstopdf test.eps --gsopt -sFONTPATH=/Users/elvio/Desktop/MY-SVN/GreatSPN/
        // SOURCES/JavaGUI/jlatexmath-master/src/org/scilab/forge/jlatexmath/fonts/base/
        // std::string cmd = "ps2pdf -dEPSCrop \""+inc_name+".eps\"  \""+inc_name+".pdf\" > /dev/null 2>&1";
        std::string cmd = "epstopdf \""+inc_name+".eps\" > /dev/null 2>&1";
        if (system(cmd.c_str()))
            cout << "Cannot execute the command: " << cmd << endl;
        if (g_open_saved_incidence) {
            open_file((inc_name + ".pdf").c_str());
        }
        if (invoked_from_gui())
            cout << "#{GUI}# RESULT INC" << endl;
    }

    // if (save_image && !is_meta_heuristics) {
    //     std::string pbm = net_name;
    //     pbm += "pbm";
    //     cout << "WRITING VAR ORDER BITMAP TO " << pbm << " ..." << endl;
    //     write_var_order_as_NetPBM(pbm.c_str(), net_to_mddLevel);    	
    // }

    if (g_exit_after_varorder && !is_meta_heuristics) {
        cout << "\nEXIT." << endl;
        exit(0);
    }
}

//---------------------------------------------------------------------------------------

// Names of the variable orders
std::pair<const char*, const char*> var_order_name(VariableOrderCriteria voc) {
	switch (voc) {
        case VOC_NO_REORDER:                  return std::make_pair("",           "No Reorder");
        case VOC_INPUT_ORDER:                 return std::make_pair("",           "Input order");
		case VOC_RANDOM:		              return std::make_pair("RND",        "Random order");
    	case VOC_FROM_FILE:                   return std::make_pair("F",          "From File");
        case VOC_DFS:                         return std::make_pair("DFS",        "Depth-First Search visit");
        case VOC_BFS:                         return std::make_pair("BFS",        "Breadth-First Search visit");
        case VOC_PINV:                        return std::make_pair("P-old",      "Use P-invariants");
    	case VOC_PCHAINING:                   return std::make_pair("P",          "P-chaining algorithm");
        case VOC_FORCE:                       return std::make_pair("FR",         "FORCE method");
        case VOC_FORCE_PINV:                  return std::make_pair("FR-P",       "FORCE method with P-semiflows");
        case VOC_FORCE_NU:                    return std::make_pair("FR-NU",      "FORCE method with Nested Units");
        case VOC_TOPOLOGICAL:                 return std::make_pair("TS",         "Topological Sort");
    	case VOC_CUTHILL_MCKEE:               return std::make_pair("CM",         "Cuthill Mckee method");
    	case VOC_KING:                        return std::make_pair("KING",       "King ordering");
        case VOC_SLOAN:                       return std::make_pair("SLO",        "Sloan method");
    	case VOC_SLOAN_1_16:                  return std::make_pair("SLO-16",     "Sloan method with W1=1, W2=16");
    	case VOC_NOACK:                       return std::make_pair("NOACK",      "Noack method");
    	case VOC_TOVCHIGRECHKO:               return std::make_pair("TOV",        "Tovchigrechko method");
        case VOC_NOACK2:                      return std::make_pair("NOACK2",     "Noack method (new)");
        case VOC_TOVCHIGRECHKO2:              return std::make_pair("TOV2",       "Tovchigrechko method (new)");
        case VOC_GRADIENT_P:                  return std::make_pair("GP",         "Gradient P-invariants");
        // case VOC_MARKOV_CLUSTER:              return std::make_pair("MCL",        "Markov Cluster Algorithm");
    	case VOC_GRADIENT_NU:                 return std::make_pair("GNU",        "Gradient Nested Units");
        case VOC_VCL_CUTHILL_MCKEE:           return std::make_pair("CM2",        "VCL Cuthill-Mckee method");
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:  return std::make_pair("ACM",        "VCL Advanced Cuthill-Mckee method");
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:  return std::make_pair("GPS",        "VCL Gibbs-Poole-StockMeyer method");

        case VOC_META_FORCE:                  return std::make_pair("MFR",        "Meta-heuristic using metric-guided Random+Force.");
    	case VOC_META_BY_SCORE:               return std::make_pair("META",       "Meta-heuristic using weighted score.");
 
        case VOC_PRINT_METRICS_AND_QUIT:      return std::make_pair("--",         "--no reorder--");
    	case VOC_TOTAL_CRITERIAS:             break; // throw
	}
	throw rgmedd_exception("Missing algorithm name in var_order_name()");
}

//---------------------------------------------------------------------------------------

bool method_uses_lin_constraints(VariableOrderCriteria voc) {
	switch (voc) {
		case VOC_NO_REORDER:
        case VOC_RANDOM:
    	case VOC_FROM_FILE:
        case VOC_DFS:
        case VOC_BFS:
        case VOC_FORCE:
        case VOC_FORCE_NU:
        case VOC_TOPOLOGICAL:
    	case VOC_CUTHILL_MCKEE:
    	case VOC_KING:
    	case VOC_SLOAN:
        case VOC_SLOAN_1_16:
    	case VOC_NOACK:
        case VOC_TOVCHIGRECHKO:
        case VOC_NOACK2:
        case VOC_TOVCHIGRECHKO2:
        // case VOC_MARKOV_CLUSTER:
    	case VOC_GRADIENT_NU:
        case VOC_VCL_CUTHILL_MCKEE:
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:
        case VOC_PRINT_METRICS_AND_QUIT:
        case VOC_META_FORCE:
    		return false;

        case VOC_FORCE_PINV:
        case VOC_PINV:
    	case VOC_PCHAINING:
    	case VOC_GRADIENT_P:
    		return true;

    	case VOC_META_BY_SCORE:
        case VOC_INPUT_ORDER:
    	case VOC_TOTAL_CRITERIAS:
    		break; // throw
	}
	throw rgmedd_exception("Missing algorithm name in method_uses_pinvs()");	
}

//---------------------------------------------------------------------------------------

bool method_uses_nested_units(VariableOrderCriteria voc) {
    switch (voc) {
        case VOC_NO_REORDER:
        case VOC_RANDOM:
        case VOC_FROM_FILE:
        case VOC_DFS:
        case VOC_BFS:
        case VOC_FORCE:
        case VOC_FORCE_PINV:
        case VOC_TOPOLOGICAL:
        case VOC_CUTHILL_MCKEE:
        case VOC_KING:
        case VOC_SLOAN:
        case VOC_SLOAN_1_16:
        case VOC_NOACK:
        case VOC_TOVCHIGRECHKO:
        case VOC_NOACK2:
        case VOC_TOVCHIGRECHKO2:
        case VOC_PRINT_METRICS_AND_QUIT:
        case VOC_PINV:
        case VOC_PCHAINING:
        case VOC_GRADIENT_P:
        // case VOC_MARKOV_CLUSTER:
        case VOC_VCL_CUTHILL_MCKEE:
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:
        case VOC_META_FORCE:
            return false;

        case VOC_FORCE_NU:
        case VOC_GRADIENT_NU:
            return true;

        case VOC_META_BY_SCORE:
        case VOC_INPUT_ORDER:
        case VOC_TOTAL_CRITERIAS:
            break; // throw
    }
    throw rgmedd_exception("Missing algorithm name in method_uses_nested_units()");    
}

//---------------------------------------------------------------------------------------

const char *name_of_annealing(Annealing a, VariableOrderMetric m) {
    switch (a) {
        case Annealing::NO_ANN:             return "None";
        case Annealing::ANN_PBASIS:         return "iRank";
        case Annealing::ANN_PBASIS_MIN:     return "iRank reorder";
        case Annealing::ANN_MINIMIZE_METRIC:
            return metric_name(m);
        case Annealing::ANN_FORCE:          return "Force";
        case Annealing::ANN_FORCE_TI:       return "Force-TI";
        case Annealing::ANN_FORCE_P:        return "Force-P";
    }
    throw rgmedd_exception();
}

//---------------------------------------------------------------------------------------

const char* metric_name(VariableOrderMetric m) {
    switch (m) {
        case METRIC_SOUPS:    return "SOUPS";
        case METRIC_SWIR:     return "SWIR";
        case METRIC_SWIR_X:   return "SWIR_X";
        case METRIC_iRank:    return "iRank";
        case METRIC_NES:      return "NES";
        case METRIC_WES1:     return "WES1";
        case METRIC_PSF:      return "PSF";
        case METRIC_PF:       return "PF";
        case METRIC_SOT:      return "SOT";
        case METRIC_PTS:      return "PTS";
        case METRIC_PTS_P:    return "PTS_P";
        case METRIC_BW:       return "BW";
        case METRIC_PROF:     return "PROF";
        case METRIC_AVGWF:    return "AVGWF";
        case METRIC_MAXWF:    return "MAXWF";
        case METRIC_RMSWF:    return "RMSWF";
        case METRIC_SOS:      return "SOS";
        case METRIC_PSI:      return "PSI";

        case METRIC_SO_ES_wI:      return "SO_ES_wI";
        case METRIC_SO_ES_wPSF:    return "SO_ES_wPSF";
        case METRIC_SO_ES_wRNG:    return "SO_ES_wRNG";
        case METRIC_SO_ES_wRS:     return "SO_ES_wRS";
        case METRIC_SO_ES_w1:      return "SO_ES_w1";
        case METRIC_SO_UPS_wI:     return "SO_UPS_wI";
        case METRIC_SO_UPS_wPSF:   return "SO_UPS_wPSF";
        case METRIC_SO_UPS_wRNG:   return "SO_UPS_wRNG";
        case METRIC_SO_UPS_wRS:    return "SO_UPS_wRS";
        case METRIC_SO_UPS_w1:     return "SO_UPS_w1";
        case METRIC_SO_UPSx_wI:    return "SO_UPSx_wI";
        case METRIC_SO_UPSx_wPSF:  return "SO_UPSx_wPSF";
        case METRIC_SO_UPSx_wRNG:  return "SO_UPSx_wRNG";
        case METRIC_SO_UPSx_wRS:   return "SO_UPSx_wRS";
        case METRIC_SO_UPSx_w1:    return "SO_UPSx_w1";
        case METRIC_SO_US_wI:      return "SO_US_wI";
        case METRIC_SO_US_wPSF:    return "SO_US_wPSF";
        case METRIC_SO_US_wRNG:    return "SO_US_wRNG";
        case METRIC_SO_US_wRS:     return "SO_US_wRS";
        case METRIC_SO_US_w1:      return "SO_US_w1";
        case METRIC_SO_1_wI:       return "SO_1_wI";
        case METRIC_SO_1_wPSF:     return "SO_1_wPSF";
        case METRIC_SO_1_wRNG:     return "SO_1_wRNG";
        case METRIC_SO_1_wRS:      return "SO_1_wRS";

        case NO_METRIC:
        case NUM_METRICS:
        default:
            throw rgmedd_exception();
    }
}

//---------------------------------------------------------------------------------------

void metric_compute(const std::vector<int>& net_to_level,
                    trans_span_set_t& trn_set,
                    flow_basis_metric_t* opt_fbm,
                    metric_t* out[NUM_METRICS])
{
    bool bandwidth_computed = false;
    bool setup = false;


    auto require_initialize_fbm = [&]() {
        if (!setup) {
            assert(opt_fbm != nullptr);
            // Initialize the fbm data structure for PSI (including ranges)
            measure_PSI(net_to_level, 
                        false /*only_ranks*/, false /*use_ilp*/, 
                        false /*use_enum*/, false /*print_enums*/, *opt_fbm);
            setup = true;
        }
    };

    // Extract level weights for SWIR
    std::vector<size_t> lw_invs;
    auto require_lw_invs = [&]() {
        require_initialize_fbm();
        if (lw_invs.empty()) {
            // compute_lvl_weights(net_to_level, *opt_fbm, lw_invs);
            get_lvl_weights_invariants(*opt_fbm, lw_invs);
        }
    };

    // Extract level weights for PSI (ranges of possible domain values)
    std::vector<cardinality_t> lw_ranges;
    auto require_lw_ranges = [&]() {
        require_initialize_fbm();
        if (lw_ranges.empty()) {
            get_lvl_weights_ranges(*opt_fbm, lw_ranges);
        }
    };

    // Account for (approximate) sharing in ranges
    std::vector<double> lw_ranges_share;
    auto require_lw_ranges_share = [&]() {
        require_lw_invs();
        require_lw_ranges();
        if (lw_ranges_share.empty()) {
            lw_ranges_share.resize(npl, 0);
            for (size_t i=0; i<npl; i++)
                lw_ranges_share[i] = get_double(lw_ranges[i]) / lw_invs[i];
        }
    };

    // Level weights using P-semiflows
    std::vector<size_t> lw_psf;
    auto require_lw_psf = [&]() {
        if (lw_psf.empty()) {
            lw_psf.resize(npl, 0);
            const int_lin_constr_vec_t& psf = load_Psemiflows();
            const int* bounds = load_bounds();
            for (const int_lin_constr_t& p : psf) {
                int lvlbot, lvltop;
                lvlbot = lvltop = net_to_level[p.coeffs.ith_nonzero(0).index];
                for (auto& e : p.coeffs) {
                    lvlbot = min(lvlbot, net_to_level[e.index]);
                    lvltop = max(lvltop, net_to_level[e.index]);
                }
                // compute the p-semiflow span
                for (int lvl = lvlbot; lvl <= lvltop; lvl++) {
                    lw_psf[lvl]++;
                }
            }
        }
    };



    for (int m=0; m<NUM_METRICS; m++) {
        if (out[m] == nullptr)
            continue; // do not compute the value of metric m

        switch (m) {
            case METRIC_SOUPS:
                *out[m] = measure_soups(net_to_level, trn_set);
                break;

            case METRIC_SWIR:
                require_lw_invs();
                *out[m] = measure_swir(net_to_level, trn_set, lw_invs);
                break;

            case METRIC_SWIR_X:
                require_lw_invs();
                *out[m] = measure_swir2(net_to_level, trn_set, lw_invs);
                break;

            case METRIC_iRank:
                require_lw_invs();
                *out[m] = (uint64_t)std::accumulate(lw_invs.begin(), lw_invs.end(), size_t(0));
                // assert(opt_fbm != nullptr);
                // *out[m] = measure_PSI(net_to_level, true, false, false, false, *opt_fbm);
                break;

            case METRIC_NES:
                *out[m] = measure_NES(net_to_level);
                break;

            case METRIC_WES1:
                *out[m] = measure_WESi(net_to_level, 1);
                break;

            case METRIC_PSF:
                *out[m] = (uint64_t)measure_PSF(net_to_level);
                break;

            case METRIC_PF:
                *out[m] = (uint64_t)measure_PF(net_to_level);
                break;

            case METRIC_SOT:
                *out[m] = (uint64_t)measure_SOT(net_to_level);
                break;

            case METRIC_PTS: {
                const int_lin_constr_vec_t empty_ilcp;
                *out[m] = measure_FORCE_pts(VOC_FORCE, net_to_level, empty_ilcp);
                break;
            }

            case METRIC_PTS_P:
                *out[m] = measure_FORCE_pts(VOC_FORCE_PINV, net_to_level, load_Psemiflows());
                break;

            case METRIC_BW:                
            case METRIC_PROF:
            case METRIC_AVGWF:
            case METRIC_MAXWF:
            case METRIC_RMSWF:
                if (!bandwidth_computed) {
                    // Compute all bandwidth metrics in a single shot
                     metrics_bandwidth(net_to_level, 
                                       out[METRIC_BW]    ? &out[METRIC_BW]->as_uint64()    : nullptr,
                                       out[METRIC_PROF]  ? &out[METRIC_PROF]->as_uint64()  : nullptr,
                                       out[METRIC_AVGWF] ? &out[METRIC_AVGWF]->as_double() : nullptr,
                                       out[METRIC_MAXWF] ? &out[METRIC_MAXWF]->as_double() : nullptr,
                                       out[METRIC_RMSWF] ? &out[METRIC_RMSWF]->as_double() : nullptr);
                    bandwidth_computed = true;
                }
                break;

            case METRIC_SOS:
                *out[m] = measure_SOS(net_to_level);
                break;

            case METRIC_PSI:
                require_lw_ranges();
                *out[m] = std::accumulate(lw_ranges.begin(), lw_ranges.end(), cardinality_t(0));
                break;

            case METRIC_SO_ES_wI:
            case METRIC_SO_ES_wPSF:
            case METRIC_SO_ES_wRNG:
            case METRIC_SO_ES_wRS:
            case METRIC_SO_ES_w1:
            case METRIC_SO_UPS_wI:
            case METRIC_SO_UPS_wPSF:
            case METRIC_SO_UPS_wRNG:
            case METRIC_SO_UPS_wRS:
            case METRIC_SO_UPS_w1:
            case METRIC_SO_UPSx_wI:
            case METRIC_SO_UPSx_wPSF:
            case METRIC_SO_UPSx_wRNG:
            case METRIC_SO_UPSx_wRS:
            case METRIC_SO_UPSx_w1:
            case METRIC_SO_US_wI:
            case METRIC_SO_US_wPSF:
            case METRIC_SO_US_wRNG:
            case METRIC_SO_US_wRS:
            case METRIC_SO_US_w1:
            case METRIC_SO_1_wI:
            case METRIC_SO_1_wPSF:
            case METRIC_SO_1_wRNG:
            case METRIC_SO_1_wRS:
            {
                std::pair<MetricFn, LevelWeight> mw = decompose_SO_metric(m);

                const std::vector<size_t>* no_weights = nullptr;
                switch (mw.second) {
                    case LW_INV:
                        require_lw_invs();
                        *out[m] = (uint64_t)metric_SO_x_W(mw.first, net_to_level, trn_set, lw_invs);
                        break;

                    case LW_PSF:
                        require_lw_psf();
                        *out[m] = (uint64_t)metric_SO_x_W(mw.first, net_to_level, trn_set, lw_psf);
                        break;

                    case LW_RANGES:
                        require_lw_ranges();
                        *out[m] = metric_SO_x_W(mw.first, net_to_level, trn_set, lw_ranges);
                        break;

                    case LW_RANGES_SHARE:
                        require_lw_ranges_share();
                        *out[m] = metric_SO_x_W(mw.first, net_to_level, trn_set, lw_ranges_share);
                        break;

                    case LW_NONE:
                        *out[m] = metric_SO_x_w1(mw.first, net_to_level, trn_set);
                        break;
                }

                break;
            }

        }
    }
}

//---------------------------------------------------------------------------------------

void metric_compute(const std::vector<int>& net_to_level,
                    trans_span_set_t& trn_set,
                    flow_basis_metric_t* opt_fbm,
                    VariableOrderMetric m,
                    metric_t* out)
{
    metric_t* out_metrics[NUM_METRICS];
    std::fill(out_metrics, out_metrics + NUM_METRICS, nullptr);
    out_metrics[m] = out;

    metric_compute(net_to_level, trn_set, opt_fbm, out_metrics);
}

//---------------------------------------------------------------------------------------

// Load the variable order from a list of place names written on a file  <netname>.place
void var_order_from_file(std::vector<int> &out_order, const std::map<const char*, int, cstr_less>& S2Ipl) 
{
    istream *in;
    ifstream fin;
    istringstream sin;
    if (g_given_varorder != nullptr) { // varorder is passed on the command line
        sin = istringstream(g_given_varorder);
        in = &sin;
    }
    else { // read the varorder from the <netname>.place  file
        std::string buffer = net_name + std::string("place");
        fin.open(buffer.c_str());
        if (!fin) {
            ostringstream msg;
            msg << "Error opening input stream: " << buffer << " .";
            throw rgmedd_exception(msg.str());
        }
        in = &fin;
    }
    out_order.resize(npl);
    std::fill(out_order.begin(), out_order.end(), -1);
    std::string name;
    for (unsigned int i = 0; i < npl; i++) {
        (*in) >> name;
        auto iter = S2Ipl.find(name.c_str());
        if (iter != S2Ipl.end()) {
            out_order[ iter->second ] = i;
        }
        else {
            ostringstream msg;
            msg << "Error place " << name << " is not presented in the net.";
            throw rgmedd_exception(msg.str());
        }
    }
}

//---------------------------------------------------------------------------------------

static std::vector<int>
load_flow_consts_from_file(ifstream& pif, size_t num_flows) {
    std::vector<int> inv_consts;
    if (!pif) 
        return inv_consts; // empty

    int num_consts;
    pif >> num_consts;
    if (!pif || num_consts < 0 || num_consts != num_flows)
        return inv_consts;

    inv_consts.resize(num_consts);
    for (int i = 0; i < num_consts; i++) {
        int c;
        pif >> c;
        if (!pif || c < 0) {
            inv_consts.clear();
            return inv_consts; // bad file, or could not read the data
        }
        inv_consts[i] = c;
    }
    return inv_consts;
}

//---------------------------------------------------------------------------------------

static int_lin_constr_vec_t
load_flows_from_file(ifstream& pif, size_t max_index, bool allow_neg_card) {
    int_lin_constr_vec_t flows;
    if (!pif) 
        return flows; // empty

    int num_pinv;
    pif >> num_pinv;
    if (!pif || num_pinv < 0)
        return flows;

    flows.resize(num_pinv);
    for (int i = 0; i < num_pinv; i++) {
        int len;
        pif >> len;
        if (!pif || len < 0) {
            flows.clear();
            return flows; // bad file, or could not generate flows (len<0)
        }
        flows[i].coeffs.resize(max_index);
        flows[i].coeffs.reserve(len);
        flows[i].const_term = 0; // m0 * p-flow
        flows[i].op = CI_EQ;
        for (int j = 0; j < len; j++) {
            int card, pl;
            pif >> card >> pl;
            if (!pif || (card <= 0 && !allow_neg_card) || pl <= 0 || pl > max_index) {
                flows.clear();
                return flows; // bad file
            }
            flows[i].coeffs.insert_element(pl - 1, card);
            // flows[i].ith_nonzero(j).index = pl - 1;
            // flows[i].ith_nonzero(j).value = card;
            if (pl < npl) // real place index, not a supplementray variable
                flows[i].const_term += net_mark[pl - 1].total * card;
        }
    }
    return flows;
}

//---------------------------------------------------------------------------------------

static int_lin_constr_vec_t
load_int_lin_constr_problem_from_file(ifstream& pif, size_t max_index, bool allow_neg_card) {
    int_lin_constr_vec_t ilcp;
    if (!pif) 
        return ilcp; // empty

    int num_constr;
    pif >> num_constr;
    if (!pif || num_constr < 0)
        return ilcp;

    ilcp.reserve(num_constr);
    for (int i = 0; i < num_constr; i++) {
        int len;
        pif >> len;
        if (!pif || len < 0) {
            ilcp.clear();
            cerr << "Error reading constraints from file." << endl;
            return ilcp; // bad file, or could not generate constraints (len<0)
        }
        int_lin_constr_t constr;
        constr.coeffs.resize(max_index);
        constr.coeffs.reserve(len);
        for (int j = 0; j < len; j++) {
            int card, pl;
            pif >> card >> pl;
            if (!pif || (card <= 0 && !allow_neg_card) || pl <= 0 || pl > max_index) {
                ilcp.clear();
                cerr << "Error reading constraints coefficients from file." << endl;
                return ilcp; // bad file
            }
            constr.coeffs.insert_element(pl - 1, card);
        }
        std::string op;
        pif >> op >> constr.const_term;
        constr.op = constr_ineq_op_t(-1);
        for (size_t e=0; e<CI_TOTAL_OPERATORS; e++) {
            if (0==strcmp(s_constr_ineq_op_str[e], op.c_str())) {
                constr.op = constr_ineq_op_t(e);
                break;
            }
        }
        if (constr.op == constr_ineq_op_t(-1)) {
            ilcp.clear();
            cerr << "Error reading ineq. operator "<<op<<" from file." << endl;
            return ilcp; // bad file
        }
        ilcp.emplace_back(constr);
    }
    return ilcp;
}

//---------------------------------------------------------------------------------------

// Load and store the vector of P-semiflows (from the <netname>.pin file)
const int_lin_constr_vec_t&
load_Psemiflows() {
    static int_lin_constr_vec_t net_psf; // Permanently stored P-semiflows
    static bool psf_loaded = false;

    if (!psf_loaded) {
        psf_loaded = true;
        std::string pinv_name(net_name);
        pinv_name += "pin";
        ifstream pif(pinv_name.c_str());
        net_psf = load_flows_from_file(pif, npl, false);
    }
    return net_psf;
}

//---------------------------------------------------------------------------------------

// // Load and store p-semiflow constants (from the <netname>.pinc file)
// const std::vector<int>&
// load_Psemiflow_consts() {
//     static std::vector<int> net_ic; 
//     static bool ic_loaded = false;

//     if (!ic_loaded) {
//         ic_loaded = true;
//         std::string pinc_name(net_name);
//         pinc_name += "pinc";
//         ifstream pif(pinc_name.c_str());
//         net_ic = load_flow_consts_from_file(pif, get_num_Psemiflows());
//     }
//     return net_ic;
// }

//---------------------------------------------------------------------------------------

// Load and store the vector of P-semiflows (from the <netname>.pin+- file) with extra support variables
const int_lin_constr_vec_t&
load_Psemiflows_leq() {
    static int_lin_constr_vec_t net_psf_leq; // Permanently stored P-semiflows with extra variables
    static bool psf_leq_loaded = false;

    if (!psf_leq_loaded) {
        psf_leq_loaded = true;
        std::string pinv_name(net_name);
        pinv_name += "pin+-";
        ifstream pif(pinv_name.c_str());
        net_psf_leq = load_flows_from_file(pif, npl + 2*ntr, false);
    }
    return net_psf_leq;
}

//---------------------------------------------------------------------------------------

// // Load and store p-semiflow constants (from the <netname>.pin+-c file)
// const std::vector<int>&
// load_Psemiflow_leq_consts() {
//     static std::vector<int> net_ic_leq; 
//     static bool ic_leq_loaded = false;

//     if (!ic_leq_loaded) {
//         ic_leq_loaded = true;
//         std::string pinc_name(net_name);
//         pinc_name += "pin+-c";
//         ifstream pif(pinc_name.c_str());
//         net_ic_leq = load_flow_consts_from_file(pif, get_num_Psemiflows());
//     }
//     return net_ic_leq;
// }

//---------------------------------------------------------------------------------------

// Load and store the integer constraints problem (from the <netname>.icp file)
int_lin_constr_vec_t&
load_int_constr_problem_nonconst() {
    static int_lin_constr_vec_t net_icp; // Permanently stored 
    static bool icp_loaded = false;

    if (!icp_loaded) {
        icp_loaded = true;
        std::string icp_name(net_name);
        icp_name += "ilcp";
        ifstream pif(icp_name.c_str());
        net_icp = load_int_lin_constr_problem_from_file(pif, npl, true);
    }
    return net_icp;
}

const int_lin_constr_vec_t& load_int_constr_problem() { 
    return load_int_constr_problem_nonconst(); 
}

//---------------------------------------------------------------------------------------

// Manipulate the model to add slack variables for the constraints with < or >
void ilcp_add_slack_variables_to_model() {
    // Determine the missing slack variables, and standardize the constraints
    size_t num_slack_vars = 0, sv;
    int_lin_constr_vec_t& ilcp = load_int_constr_problem_nonconst();
    for (int_lin_constr_t& constr : ilcp) {
        switch (constr.op) {
            case CI_EQ:
                break; // already standardized

            case CI_LESS_EQ: 
                // x1 + 2*x2 <= 3  -->  x1 + 2*x2 + s1 = 3,  s1 >= 0
                // Ax <= b  -->  Ax + sv = b,  sv >= 0
                sv = npl + (num_slack_vars++);
                constr.coeffs.resize(sv+1);
                constr.coeffs.insert_element(sv, 1);
                constr.op = CI_EQ;
                break;

            case CI_GREAT_EQ: 
                // Ax >= b  -->  Ax - sv = b,  sv >= 0
                sv = npl + (num_slack_vars++);
                constr.coeffs.resize(sv+1);
                constr.coeffs.insert_element(sv, -1);
                constr.op = CI_EQ;
                break;

            case CI_LESS:
            case CI_GREAT:
            default:
                throw new rgmedd_exception("Unimplemented.");
        }
    }
    cout << "Adding " << num_slack_vars << " slack variables to the model." << endl;

    // resize the place table
    tabp = (struct PLACES *)realloc(tabp, sizeof(struct PLACES) * (npl + num_slack_vars));
    // Add the descriptors for the slack variables
    for (size_t ii=0; ii<num_slack_vars; ii++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "s%zu", ii);
        tabp[npl + ii].place_name = strdup(buffer);
        tabp[npl + ii].algebra_tags = nullptr;
        tabp[npl + ii].dominio = nullptr;
        tabp[npl + ii].comp_num = UNKNOWN;
        tabp[npl + ii].card = UNKNOWN;
        tabp[npl + ii].position = 0; // initial marking is zero
        tabp[npl + ii].tagged = FALSE;
        tabp[npl + ii].unit = NULL;
        tabp[npl + ii].is_slack_var = TRUE; // mark as a slack variable
    }

    // change the number of places
    npl = npl + num_slack_vars;

    // resize the ILCP constraints
    for (int_lin_constr_t& constr : ilcp)
        constr.coeffs.resize(npl);
}

//---------------------------------------------------------------------------------------

// Get the general ILCP problem, which could come either from the ILCP file, or from the PIN file
const int_lin_constr_vec_t&
get_int_constr_problem() {
    if (ilcp_model)
        return load_int_constr_problem();
    else
        return load_Psemiflows();
}

//---------------------------------------------------------------------------------------

ostream& operator<<(ostream& os, const int_lin_constr_vec_t& ilcp) {
    for (const int_lin_constr_t& row : ilcp) {
        size_t cnt = 0;
        for (auto el : row.coeffs) {
            if (0 != cnt++)
                os << " + ";
            if (el.value == -1)
                os << "-";
            else if (el.value != 1)
                os << el.value << "*";
            os << tabp[el.index].place_name;
        }
        os << " " << s_constr_ineq_op_str[row.op] << " " << row.const_term << endl;
    }
    return os;
}

//---------------------------------------------------------------------------------------

// Returns the number of P-invariants (0 if there are no P-invariants, or if the
// model has an empty .pin file)
int get_num_Psemiflows() {
    return load_Psemiflows().size();
}

//---------------------------------------------------------------------------------------

bool model_has_nested_units() {
    return (num_nested_units != 0);
}

//---------------------------------------------------------------------------------------

bool all_places_are_covered(const int_lin_constr_vec_t& ilcp) {
    // verify place coverage
    std::vector<bool> covered(npl, false);
    for (const auto &f : ilcp) {
        // size_t num_support_vars = 0;
        for (auto& elem : f.coeffs) {
            if (elem.index >= npl) { // support variable
                // num_support_vars++;
                // if (num_support_vars > 1)
                //     return false; // more than one support variable in this invariant
            }
            else
                covered[elem.index] = true;
        }
    }
    for (bool is_cov : covered)
        if (!is_cov)
            return false; // some places are not covered by invariants
    return true;
}

//---------------------------------------------------------------------------------------

// NuPN: convert nested units into fake linear constraints, to be injected as inputs to
// P-semiflows and/or constraint based variable orders
const int_lin_constr_vec_t&
convert_nested_units_as_ilcp() 
{
    static int_lin_constr_vec_t nupn_ilcp; // Permanently stored NuPN
    static bool nupn_conv = false;

    if (!nupn_conv) {
        nupn_conv = true;
        if (!model_has_nested_units())
            return nupn_ilcp;
        nupn_ilcp.resize(num_nested_units);
        for (int i=0; i<num_nested_units; i++) {
            nupn_ilcp[i].coeffs.resize(npl);
            nupn_ilcp[i].coeffs.reserve(nu_array[i]->num_places);
            for (int p=0; p<nu_array[i]->num_places; p++)
                nupn_ilcp[i].coeffs.add_element(nu_array[i]->places[p], 1);
            nupn_ilcp[i].const_term = 1;
            nupn_ilcp[i].op = CI_EQ;
            // nupn_ilcp[i].resize(nu_array[i]->num_places);
            // Read unit' places
            // for (int p=0; p<nupn_ilcp[i].size(); p++) {
            //     nupn_ilcp[i][p].place_no = nu_array[i]->places[p];
            //     nupn_ilcp[i][p].card = 1; 
            // }
        }
    }
    return nupn_ilcp;
}

//---------------------------------------------------------------------------------------

const pre_post_sets_t& get_pre_post_sets() {
    static pre_post_sets_t pps;
    if (pps.preP.empty())
        pps.build();
    return pps;
}

// //---------------------------------------------------------------------------------------
// // Load a P-flow file (pba, pfl, pin, ...) into a flow_basis_t object
// //---------------------------------------------------------------------------------------

// static bool load_flows_from_file(ifstream& pif, flow_basis_t& fb) 
// {
//     if (!pif) 
//         return false;
//     int num_pba;
//     pif >> num_pba;
//     if (!pif || num_pba < 0)
//         throw rgmedd_exception("Bad P-flow file. [1]");
//     fb.resize(num_pba);
//     for (int i = 0; i < num_pba; i++) {
//         int len;
//         pif >> len;
//         if (!pif || len < 0)
//             throw rgmedd_exception("Bad P-flow file. [2]");
//         fb[i] = sparse_vector_t(npl);
//         fb[i].reserve(len);
//         for (int j = 0; j < len; j++) {
//             int card, pl;
//             pif >> card >> pl;
//             if (!pif || pl <= 0 || pl > npl)
//                 throw rgmedd_exception("Bad P-flow file. [3]");
//             // if (pif && 0 <= pl && pl < npl)
//             fb[i].insert_element(pl - 1, card);
//         }
//     }
//     return true;
// }

//---------------------------------------------------------------------------------------

// Test if all places are covered by at least one p-flow in the given basis/set
static bool test_flows_coverage(int_lin_constr_vec_t& fb) {
    std::vector<bool> cov(npl);
    std::fill(cov.begin(), cov.end(), false);

    for (auto&& row : fb)
        for (auto&& el : row.coeffs)
            cov[el.index] = true;

    return std::find(cov.begin(), cov.end(), false) == cov.end();
}

//---------------------------------------------------------------------------------------
// Support for P-flow basis (netname.pba file)
//---------------------------------------------------------------------------------------

static bool s_basis_loaded = false; // Have we tried to load the basis?
static bool s_have_basis = false;   // Do we have the basis?
static int s_max_inv_coeff = 0;
static bool s_cov_by_flow_basis = false; // Are all places in the net covered by P-flows?
static int_lin_constr_vec_t s_fb;

bool load_flow_basis() {
    if (!s_basis_loaded) {
        s_basis_loaded = true;

        std::string pba_name(net_name);
        pba_name += "pba";
        ifstream pif(pba_name.c_str());
        s_fb = load_flows_from_file(pif, npl, true);
        s_have_basis = !s_fb.empty();
        if (s_have_basis) {
            for (auto&& row : s_fb)
                for (auto&& el : row.coeffs)
                    s_max_inv_coeff = max(s_max_inv_coeff, el.value);

            s_cov_by_flow_basis = test_flows_coverage(s_fb);
        }
    }
    return s_have_basis;
}

//---------------------------------------------------------------------------------------

const int_lin_constr_vec_t& get_flow_basis() {
    // for ILCP model, the ILCP matrix itself is the flow basis for iRank
    if (ilcp_model)
        return load_int_constr_problem();
    // for Petri nets instead, use the P-flow basis matrix
    if (!s_basis_loaded)
        load_flow_basis(); 
    return s_fb;
}

//---------------------------------------------------------------------------------------

// // Load and store p-basis constants (from the <netname>.pbac file)
// const std::vector<int>&
// load_flow_consts() {
//     static std::vector<int> net_basis_c; 
//     static bool pbac_loaded = false;

//     if (!pbac_loaded) {
//         pbac_loaded = true;
//         std::string pbac_name(net_name);
//         pbac_name += "pbac";
//         ifstream pif(pbac_name.c_str());
//         net_basis_c = load_flow_consts_from_file(pif, get_flow_basis().size());
//     }
//     return net_basis_c;
// }

//---------------------------------------------------------------------------------------

size_t get_num_invariants() {
    return get_flow_basis().size();
}

int get_max_invariant_coeff() {
    get_flow_basis();
    return s_max_inv_coeff;
}

bool is_net_covered_by_flow_basis() {
    get_flow_basis();
    return s_cov_by_flow_basis;
}

//---------------------------------------------------------------------------------------
// Support for P-flows (netname.pfl file)
//---------------------------------------------------------------------------------------

static bool s_pflows_loaded = false; // Have we tried to load the basis?
static bool s_have_pflows = false;   // Do we have the basis?
static int_lin_constr_vec_t s_pflows;

bool load_pflows() {
    if (!s_pflows_loaded) {
        s_pflows_loaded = true;

        std::string pfl_name(net_name);
        pfl_name += "pfl";
        ifstream pif(pfl_name.c_str());
        s_pflows = load_flows_from_file(pif, npl, true);
        s_have_pflows = !s_pflows.empty();
    }
    return s_have_pflows;
}

const int_lin_constr_vec_t& get_pflows() {
    if (!s_pflows_loaded)
        load_pflows();
    return s_pflows;
}

size_t get_num_pflows() {
    return get_pflows().size();
}

//---------------------------------------------------------------------------------------
// Force-based methods
//---------------------------------------------------------------------------------------

double estimate_points(const std::vector<int> &varorder) {
    double points = 0.0;
    for (int t = 0; t < ntr; t++) {
        double Dist = 0.0, DistSq = 0.0;
        int num_pl = 0;
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, ++num_pl) {
            int grade = varorder[ in_node->place_no ] + 1;
            Dist += grade;
            DistSq += grade * grade;
        }
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl) {
            int grade = varorder[ out_node->place_no ] + 1;
            Dist += grade;
            DistSq += grade * grade;
        }
        // cout << "TRANSITION " << t << " Dist " << Dist << " num_pl " << num_pl << endl;
        points += sqrt(fabs(DistSq - Dist * Dist)) / num_pl;
    }
    // cout << "POINTS = " << points << endl << endl;
    return points;
}

// Compute the sum of the standard deviations of the clustering
// of the places around each transition.
double estimate_points_place_spans(const std::vector<int> &varorder,
                                   const std::vector<double> &cog) {
    // return std::accumulate(cog.begin(), cog.end(), 0.0);
    double points = 0.0;
    for (int t = 0; t < ntr; t++) {
        double Dist = 0.0;
        double DistSq = 0.0;
        int num_pl = 0;
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, ++num_pl) {
            int grade = varorder[ in_node->place_no ] + 1;
            Dist += grade;
            DistSq += grade * grade;
        }
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl) {
            int grade = varorder[ out_node->place_no ] + 1;
            Dist += grade;
            DistSq += grade * grade;
        }
        // cout << "TRANSITION " << t << " Dist " << Dist << " num_pl " << num_pl << endl;
        double E_x2 = DistSq / num_pl, E_x = Dist / num_pl;
        points += sqrt(E_x2 - E_x * E_x);
        // points += sqrt(fabs(DistSq - Dist*Dist)) / num_pl;
    }
    // cout << "POINTS = " << points << endl << endl;
    return points;
}

// Clustering of each Place -> t -> Place relation
double estimate_points_transition_spans(const std::vector<int> &varorder) {
    double pl_points[npl];
    for (int p = 0; p < npl; p++)
        pl_points[p] = 0.0;
    for (int t = 0; t < ntr; t++) {
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next) {
            int p1 = in_node->place_no;
            int varpos1 = varorder[ p1 ] + 1;
            int num_pl = 0;
            double points = 0.0;
            for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl) {
                int p2 = out_node->place_no;
                int varpos2 = varorder[ p2 ] + 1;
                // p1 -> t -> p2
                points += std::abs(varpos1 - varpos2);
                num_pl++;
            }
            pl_points[p1] += points / num_pl;
        }
    }

    double tot = 0.0;
    for (int p = 0; p < npl; p++)
        tot += pl_points[p];
    return tot;
}

//---------------------------------------------------------------------------------------

inline std::pair<int, int> transition_top_bot(const std::vector<int> &varorder, int tr) {
	int top = 0, bot = npl - 1;
    for (Node_p in_node = tabt[tr].inptr; in_node != NULL; in_node = in_node->next) {
        // if (net_syphon_traps.is_trivial_syphon(in_node->place_no))
        //     continue;
        top = std::max(top, varorder[ in_node->place_no ]);
        bot = std::min(bot, varorder[ in_node->place_no ]);
    }
    for (Node_p out_node = tabt[tr].outptr; out_node != NULL; out_node = out_node->next) {
        // if (net_syphon_traps.is_trivial_trap(out_node->place_no))
        //     continue;
        top = std::max(top, varorder[ out_node->place_no ]);
        bot = std::min(bot, varorder[ out_node->place_no ]);
    }
    return make_pair(top, bot);
}

//---------------------------------------------------------------------------------------

// Top(e), Bot(e) = top and bottom levels of event e in the permutation varorder[]
// K = num variables, E = num events
//   NES = sum_{e in events} (Top(e) - Bot(e) + 1) / (K * E)
double measure_NES(const std::vector<int> &varorder) {
    double NES = 0.0;
    for (int t = 0; t < ntr; t++) {
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        NES += top - bot + 1;
    }
    return NES / (ntr * npl);
}

// WES(i) = sum_{e in events} (Top(e) / (K / 2))^i * (Top(e) - Bot(e) + 1) / (K * E)
double measure_WESi(const std::vector<int> &varorder, int i) {
    double WES = 0.0;
    for (int t = 0; t < ntr; t++) {
		int top, bot;
		std::tie(top, bot) = transition_top_bot(varorder, t);
        WES += pow(top / (npl / 2.), i) * (top - bot + 1);
    }
    return WES / (ntr * npl);
}

//---------------------------------------------------------------------------------------

// event spans weighted by independent per-level weights
template<typename SCORE, typename WEIGHT>
SCORE
measure_weighted_event_spans_base(const std::vector<int> &varorder, 
                                  const std::vector<WEIGHT> *lvl_weights) 
{
    SCORE LWES = 0;
    for (int t = 0; t < ntr; t++) {
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        if (lvl_weights != nullptr) {
            for (int lvl = bot; lvl <= top; lvl++)
                LWES += (*lvl_weights)[lvl];
        }
        else
            LWES += top - bot + 1; // sum of spans
    }
    return LWES;
}

//---------------------------------------------------------------------------------------

uint64_t
measure_weighted_event_spans(const std::vector<int> &varorder, const std::vector<size_t> *pW)
{ return measure_weighted_event_spans_base<uint64_t, size_t>(varorder, pW); }

double
measure_weighted_event_spans(const std::vector<int> &varorder, const std::vector<double> *pW)
{ return measure_weighted_event_spans_base<double, double>(varorder, pW); }

cardinality_t
measure_weighted_event_spans(const std::vector<int> &varorder, const std::vector<cardinality_t> *pW)
{ return measure_weighted_event_spans_base<cardinality_t, cardinality_t>(varorder, pW); }

//---------------------------------------------------------------------------------------

// double measure_CRS(const std::vector<int> &varorder) {
// 	// Compute how many events cross each variable
// 	std::vector<int> evXvar(varorder.size(), 0);
//     for (int t = 0; t < ntr; t++) {
// 		int top, bot;
// 		std::tie(top, bot) = transition_top_bot(varorder, t);
// 		for (int l=bot; l<top; l++)
// 			evXvar[l]++;
// 	}
// 	double CRS = 0.0;
// 	for (int t = 0; t < ntr; t++) {
// 		int top, bot;
// 		std::tie(top, bot) = transition_top_bot(varorder, t);
// 		for (int l=bot; l<top; l++)
// 			CRS += evXvar[l];
// 	}
// 	return CRS / (ntr * (npl/2.0) * npl);
// }

// double measure_CRSi(const std::vector<int> &varorder, int i) {
// 	// Compute how many events cross each variable
// 	std::vector<int> evXvar(varorder.size(), 0);
//     for (int t = 0; t < ntr; t++) {
// 		int top, bot;
// 		std::tie(top, bot) = transition_top_bot(varorder, t);
// 		for (int l=bot; l<top; l++)
// 			evXvar[l]++;
// 	}
// 	double CRSi = 0.0;
// 	for (int t = 0; t < ntr; t++) {
// 		int top, bot;
// 		std::tie(top, bot) = transition_top_bot(varorder, t);
// 		for (int l=bot; l<top; l++)
// 			CRSi += evXvar[l] * pow(top / (npl / 2.), i);
// 	}
// 	return CRSi / (ntr * (npl/2.0) * npl);
// }

//---------------------------------------------------------------------------------------

// // Maps [0..1] -> [0..1] with a (non) linear function
// inline double unit_filter(double x, UnitFilter uf) {
//     switch (uf) {
//         case UnitFilter::LINEAR:       return x;
//         case UnitFilter::CUBIC:        return x * x * x;
//         case UnitFilter::QUARTIC:      return x * x * x * x;
//         case UnitFilter::EXP:          return (exp(x) - 1) / 1.718281828459045235360287471352662497757;
//         case UnitFilter::INV_COSINE:   return 1 - cos(x * 1.570796326794896619231321691639751442099);
//     }
//     throw rgmedd_exception();
// }

// //---------------------------------------------------------------------------------------

// // Non-linear (normalized) weighted event span
// double measure_NLWES(const std::vector<int> &varorder, UnitFilter span_filter, int i) {
//     double NLWES = 0.0;
//     for (int t = 0; t < ntr; t++) {
//         // if (!is_transition_SCC_local[t]) 
//         //     continue;
//         int top, bot;
//         std::tie(top, bot) = transition_top_bot(varorder, t);
//         double norm_span = double(top - bot + 1) / npl;
//         double nls = unit_filter(norm_span, span_filter);
//         if (i != 0)
//             nls *= pow(top / (npl / 2.), i);
//         NLWES += nls;
//     }
//     NLWES /= ntr;

//     return NLWES;
// }

//---------------------------------------------------------------------------------------

// // Normalized and Weighted-normalized Event Spans
// std::pair<accumulator, accumulator> 
// measure_NWES(const std::vector<int> &varorder) {
//     accumulator NES, WES;
//     for (int t = 0; t < ntr; t++) {
//         int top, bot;
//         std::tie(top, bot) = transition_top_bot(varorder, t);
//         double norm_span = double(top - bot + 1) / npl;
//         // double nls = unit_filter(norm_span, span_filter);
//         NES += norm_span;
//         WES += norm_span * pow(top / (npl / 2.), 1);
//     }
//     return make_pair(NES, WES);
// }

//---------------------------------------------------------------------------------------

// Normalized Variable Profile and Model Bandwidth
std::tuple<accumulator, accumulator, accumulator> 
measure_profile_bandwidth(const std::vector<int> &varorder) {
    accumulator Profile, Bandwidth, Wavefront;
    // The profile of a variable (i.e. a row) is defined as:
    //   prof(i) = max{ K | event(i,k) } - min{ K | event(i,k) }
    // where the event relation is taken as symmetric (no read/write order)
    // The bandwidth of diagonal element d is defined as:
    //   band[d] = max{ abs(i-j) | d = (i+j)/2 }  forall event(i,j)
    // NOTE: this definition of bandwidth works only for symmetric matrices.
    std::vector<int> minK(npl, npl+1), maxK(npl, -1), band(npl, 0);
    for (int tr = 0; tr < ntr; tr++) {
        for (Node_p in_node = tabt[tr].inptr; in_node != NULL; in_node = in_node->next) {
            int i = varorder[ in_node->place_no ];
            for (Node_p out_node = tabt[tr].outptr; out_node != NULL; out_node = out_node->next) {
                int j = varorder[ out_node->place_no ];
                // Update profiles
                maxK[i] = std::max(maxK[i], j);
                minK[i] = std::min(minK[i], j);
                maxK[j] = std::max(maxK[j], i);
                minK[j] = std::min(minK[j], i);
                // Update bandwidth of diagonal line max(i,j):
                int d = (i + j) / 2;
                band[d] = max(band[d], abs(i - j));
            }
        }
    }
    for (int v = 0; v < npl; v++) {
        if (maxK[v] == -1) // Isolated place
            continue;
        double prof_v = maxK[v] - minK[v] + 1;
        double wfront_v = minK[v];

        Profile += prof_v / npl;
        Bandwidth += double(band[v]) / npl;
        Wavefront += wfront_v / npl;
    }
    return make_tuple(Profile, Bandwidth, Wavefront);
}

//---------------------------------------------------------------------------------------

// std::pair<accumulator, accumulator>
// measure_WCRS(const std::vector<int> &varorder) {
//     accumulator CRS, WCRS;
//     // Compute how many events cross each variable
//     std::vector<int> evXvar(varorder.size(), 0);
//     for (int t = 0; t < ntr; t++) {
//         int top, bot;
//         std::tie(top, bot) = transition_top_bot(varorder, t);
//         for (int l=bot; l<top; l++)
//             evXvar[l]++;
//     }
//     double PxT = npl * ntr;
//     for (int t = 0; t < ntr; t++) {
//         double cross = 0.0, wcross = 0.0;
//         int top, bot;
//         std::tie(top, bot) = transition_top_bot(varorder, t);
//         for (int l=bot; l<top; l++) {
//             cross += (evXvar[l] / PxT);
//             wcross += (evXvar[l] / PxT) * pow(top / (npl / 2.), 1);
//         }
//         CRS += cross;
//         WCRS += wcross;
//     }
//     return make_pair(CRS, WCRS);
// }

//---------------------------------------------------------------------------------------

// Sum of P-semiflows spans
int measure_PSF(const std::vector<int> &varorder) {
    const int_lin_constr_vec_t& psf = load_Psemiflows();
    int PSF = 0;
    for (int p=0; p<psf.size(); p++) {
        if (psf[p].coeffs.size() == 0)
            continue;
        int top = varorder[ psf[p].coeffs.ith_nonzero(0).index ];
        int bot = top;
        for (auto& elem : psf[p].coeffs) {
            top = std::max(top, varorder[ elem.index ]);
            bot = std::min(bot, varorder[ elem.index ]);
        }
        PSF += (top - bot + 1);
    }
    return PSF;
}

//---------------------------------------------------------------------------------------

// Sum of P-flows spans
int measure_PF(const std::vector<int> &varorder) {
    int_lin_constr_vec_t ilcp = get_int_constr_problem();
    reorder_basis(ilcp, varorder);
    int PF = 0;
    for (auto&& row : ilcp)
        PF += (row.coeffs.trailing() - row.coeffs.leading());
    return PF;
}

//---------------------------------------------------------------------------------------

// // Sum of Nested Units spans
// int measure_NUS(const std::vector<int> &varorder) {
//     if (!model_has_nested_units())
//         return 0;

//     int NUS = 0;
//     for (int i=0; i<num_nested_units; i++) {
//         if (nu_array[i]->num_places == 0)
//             continue;
//         int top = varorder[ nu_array[i]->places[0] ];
//         int bot = top;
//         for (int pl=0; pl<nu_array[i]->num_places; pl++) {
//             top = std::max(top, varorder[ nu_array[i]->places[pl] ]);
//             bot = std::min(bot, varorder[ nu_array[i]->places[pl] ]);
//         }
//         NUS += (top - bot + 1);
//     }
//     return NUS;
// }

//---------------------------------------------------------------------------------------

// Sum of Tops
int measure_SOT(const std::vector<int> &varorder) {
    int SOT = 0;
    for (int t = 0; t < ntr; t++) {
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        SOT += top;
    }
    return SOT;
}

//---------------------------------------------------------------------------------------

// Sum of spans
uint64_t measure_SOS(const std::vector<int> &varorder) {
    uint64_t SOS = 0;
    for (int t = 0; t < ntr; t++) {
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        SOS += top - bot + 1;
    }
    return SOS;
}

//---------------------------------------------------------------------------------------

template<typename T, typename D>
inline void safe_div(T &value, const D divisor) {
    if (divisor == D(0))
        value = 0;
    else
        value /= divisor;
}

//---------------------------------------------------------------------------------------

void pre_post_sets_t::build() {
    preP.resize(npl);
    postP.resize(npl);
    for (int t = 0; t < ntr; t++) {
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
            postP[in_node->place_no].push_back(t);
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
            preP[out_node->place_no].push_back(t);
    }
}

//---------------------------------------------------------------------------------------

// Use Depth/Breadth-First Visit to produce a variable order. Typically, this will be used
// as an initial order for the FORCE algorithm. The initial vertex is @start
void var_order_visit(const VariableOrderCriteria voc, std::vector<int> &order, int start) 
{
    const pre_post_sets_t& pps = get_pre_post_sets();
    order.resize(npl);
    std::fill(order.begin(), order.end(), -2);
    std::deque<int> Q; 
    int K = 0;

    // Make a visit in all disconnected components of the net
    for (int h=0; h<npl; h++) {
        int h0 = (h + start) % npl;
        if (order[h0] >= 0)
            continue;

        // Make a BFS visit starting from h0
        Q.push_front(h0);
        order[h0] = -1;
        while (!Q.empty()) {
            int v = Q.front();
            Q.pop_front();
            if (order[v] >= 0)
                continue; // already visited.
            order[v] = K++;

            // TODO: test without this part
            // Visit P <- T <- P
            for (auto p2t = pps.postP[v].begin(); p2t != pps.postP[v].end(); ++p2t) {
                for (Node_p t2p = tabt[*p2t].inptr; t2p != NULL; t2p = t2p->next) {
                    if (order[t2p->place_no] == -2) {
                        order[t2p->place_no] = -1;
                        if (voc == VOC_BFS)
                            Q.push_front(t2p->place_no);
                        else // DFS
                            Q.push_back(t2p->place_no);
                    }
                 }
            }
            // Visit P -> T -> P
            for (auto p2t = pps.preP[v].begin(); p2t != pps.preP[v].end(); ++p2t) {
                for (Node_p t2p = tabt[*p2t].outptr; t2p != NULL; t2p = t2p->next) {
                    if (order[t2p->place_no] == -2) {
                        order[t2p->place_no] = -1;
                        if (voc == VOC_BFS)
                            Q.push_front(t2p->place_no);
                        else // DFS
                            Q.push_back(t2p->place_no);
                    }
                }
            }
       }
    }
}

//---------------------------------------------------------------------------------------

// Maximum amount of time we are willing to spend computing FORCE iterations
static const int FORCE_MAX_SECONDS = 10;

//---------------------------------------------------------------------------------------

// Implements the exact FORCE heuristic for the ordering of variables in a Decision Diagram
// The initial order should be passed in the in_order vector.
// See: Aloul, Markov, Sakallah, "FORCE: a fast and easy-to-implement variable-ordering heuristic"
void var_order_FORCE(const VariableOrderCriteria voc, std::vector<int> &out_order, 
                     const std::vector<int> &in_order, const int_lin_constr_vec_t& ilcp, 
                     bool verbose) 
{
    std::vector<int> num_trns_of_place(npl);
    std::vector<double> cog(ntr), grade(npl);
    std::vector<std::pair<double, int>> sorter(npl);
    double last_pts = -1;

    // Load constraints (if available)
    bool has_constraints = (voc != VOC_FORCE) && (ilcp.size() > 0);
    std::vector<int> num_sf_per_place(npl);
    std::vector<double> psf_cog(ilcp.size());
    if (has_constraints) {
        // Count the number of constraints that cover each place
        for (int i = 0; i < ilcp.size(); i++)
            for (auto& p : ilcp[i].coeffs)
                num_sf_per_place[p.index]++;
    }


    // Count the number of transitions connected to each place
    std::fill(num_trns_of_place.begin(), num_trns_of_place.end(), 0);
    for (int t = 0; t < ntr; t++) {
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
            num_trns_of_place[ in_node->place_no ]++;
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
            num_trns_of_place[ out_node->place_no ]++;
    }

    clock_t time_start = clock();
    // Initialize variable positions using the initial order
    std::vector<int> var_position = in_order;
    assert(in_order.size() == npl);

    const int MAX_STEPS = 200;
    const int MIN_STEPS = int(ceil(log(npl)) + 1) * 2; // c * log(P), as suggested in the paper.
    for (int step = 0; step < MAX_STEPS; step++) {
        // Recompute the center-of-gravity (COG) of each transition
        //   COG(t) = Sum( grade(p) ) / num(p)      where variable p is connected with t
        for (int t = 0; t < ntr; t++) {
            cog[t] = 0.0;
            int num_pl = 0;
            for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, ++num_pl) 
                cog[t] += var_position[ in_node->place_no ];
            for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl) 
                cog[t] += var_position[ out_node->place_no ];
            safe_div(cog[t], num_pl);
        }
        // Recompute center-of-gravity of each P-semiflow (PSFCOG)
        //   PSFCOG(i) = Sum( grade(p) ) / num(p)      where variable p is connected with psf i
        if (has_constraints) {
            for (int i = 0; i < ilcp.size(); i++) {
                psf_cog[i] = 0.0;
                for (auto& p : ilcp[i].coeffs)
                    psf_cog[i] += var_position[p.index];
                safe_div(psf_cog[i], ilcp[i].coeffs.nonzeros());
                // cout << psf_cog[i] << " ";
            }
            // cout << endl;
        }

        // Compute the new grade of each variable, using the transition COGs
        //   grade(p) = Sum( COG[t] ) / num(t)      where transition t is connected with p
        std::fill(grade.begin(), grade.end(), 0.0);
        double pts = 0.0; // point-transition spans
        for (int t = 0; t < ntr; t++) {
            double trn_pts = 0.0;
            int num_pl = 0;
            for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, num_pl++) {
                grade[ in_node->place_no ] += cog[t];
                trn_pts += std::abs(cog[t] - var_position[ in_node->place_no ]);
            }
            for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, num_pl++){
                grade[ out_node->place_no ] += cog[t];
                trn_pts += std::abs(cog[t] - var_position[ out_node->place_no ]);
            }
            safe_div(trn_pts, num_pl);
            pts += trn_pts;
        }
        if (has_constraints) { // Add also the PSFCOG[i] to each place
            for (int i = 0; i < ilcp.size(); i++) {
                double psf_pts = 0.0; // point-PSF span
                for (auto& p : ilcp[i].coeffs) {
                    grade[p.index] += psf_cog[i];
                    psf_pts += std::abs(psf_cog[i] - var_position[p.index]);
                }
                safe_div(psf_pts, ilcp[i].coeffs.nonzeros());
                pts += psf_pts;
            }
        }
        for (int p = 0; p < npl; p++)
            safe_div(grade[p], num_trns_of_place[p] + num_sf_per_place[p]);

        // Check if we reached convergence. End as soon as the new order
        // does not improve the PTS measure over the previous order.
        if (verbose && !running_for_MCC())
            cout << step << "\t" << pts << endl;
        if (last_pts >= 0 && last_pts <= pts && step > MIN_STEPS)
            break; // Not converging any longer
        last_pts = pts;

        // Check if we consumed too much time doing FORCE iterations
        clock_t elapsed = clock() - time_start;
        if (elapsed > FORCE_MAX_SECONDS * CLOCKS_PER_SEC) {
            if (verbose && !running_for_MCC())
                cout << "Quit FORCE iteration loop." << endl;
            break;
        }

        // Reorder the variables according to their grades, and assign them a new
        // integer positioning from 1 to npl (which becomes the new variable
        // position in the next iteration)
        for (int p = 0; p < npl; p++)
            sorter[p] = std::make_pair(grade[p], p);
        std::sort(sorter.begin(), sorter.end());
        for (int p = 0; p < npl; p++)
            var_position[ sorter[p].second ] = p;
    }

    // Copy the final order
    assert(out_order.size() == npl);
    for (int p = 0; p < npl; p++) {
        out_order[p] = var_position[p];
    }
}

//---------------------------------------------------------------------------------------

// Computes the FORCE metric (point-transition spans) as defined for the FORCE algorithm
double measure_FORCE_pts(const VariableOrderCriteria voc, const std::vector<int> &in_order,
                         const int_lin_constr_vec_t& psf) 
{
    std::vector<int> num_trns_of_place(npl);
    std::vector<double> cog(ntr);//, grade(npl);
    // std::vector<std::pair<double, int>> sorter(npl);
    // double last_pts = -1;

    // Load P-semiflows (if available)
    bool has_semiflows = (voc != VOC_FORCE) && (psf.size() > 0);
    std::vector<int> num_sf_per_place(npl);
    std::vector<double> psf_cog(psf.size());
    if (has_semiflows) {
        // Count the number of P-semiflows that cover each place
        for (int i = 0; i < psf.size(); i++)
            for (auto& p : psf[i].coeffs)
                num_sf_per_place[p.index]++;
            // for (int j = 0; j < psf[i].size(); j++)
            //     num_sf_per_place[ psf[i][j].place_no ]++;
    }

    // Count the number of transitions connected to each place
    std::fill(num_trns_of_place.begin(), num_trns_of_place.end(), 0);
    for (int t = 0; t < ntr; t++) {
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
            num_trns_of_place[ in_node->place_no ]++;
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
            num_trns_of_place[ out_node->place_no ]++;
    }

    // Initialize variable positions using the initial order
    std::vector<int> var_position = in_order;
    assert(in_order.size() == npl);

    // Recompute the center-of-gravity (COG) of each transition
    //   COG(t) = Sum( grade(p) ) / num(p)      where variable p is connected with t
    for (int t = 0; t < ntr; t++) {
        cog[t] = 0.0;
        int num_pl = 0;
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, ++num_pl) 
            cog[t] += var_position[ in_node->place_no ];
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl) 
            cog[t] += var_position[ out_node->place_no ];
        safe_div(cog[t], num_pl);
    }
    // Recompute center-of-gravity of each P-semiflow (PSFCOG)
    //   PSFCOG(i) = Sum( grade(p) ) / num(p)      where variable p is connected with psf i
    if (has_semiflows) {
        for (int i = 0; i < psf.size(); i++) {
            psf_cog[i] = 0.0;
            for (auto& p : psf[i].coeffs)
                psf_cog[i] += var_position[p.index];
            // for (int j = 0; j < psf[i].size(); j++)
            //     psf_cog[i] += var_position[ psf[i][j].place_no ];
            safe_div(psf_cog[i], psf[i].coeffs.size());
        }
    }

    // Compute the new grade of each variable, using the transition COGs
    //   grade(p) = Sum( COG[t] ) / num(t)      where transition t is connected with p
    // std::fill(grade.begin(), grade.end(), 0.0);
    double pts = 0.0; // point-transition spans
    for (int t = 0; t < ntr; t++) {
        double trn_pts = 0.0;
        int num_pl = 0;
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, num_pl++) {
            // grade[ in_node->place_no ] += cog[t];
            trn_pts += std::abs(cog[t] - var_position[ in_node->place_no ]);
        }
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, num_pl++){
            // grade[ out_node->place_no ] += cog[t];
            trn_pts += std::abs(cog[t] - var_position[ out_node->place_no ]);
        }
        safe_div(trn_pts, num_pl);
        pts += trn_pts;
    }
    if (has_semiflows) { // Add also the PSFCOG[i] to each place
        for (int i = 0; i < psf.size(); i++) {
            double psf_pts = 0.0; // point-PSF span
            for (auto& p : psf[i].coeffs)
                psf_pts += std::abs(psf_cog[i] - var_position[p.index]);
            
            // for (int j = 0; j < psf[i].size(); j++) {
            //     // grade[ psf[i][j].place_no ] += psf_cog[i];
            //     psf_pts += std::abs(psf_cog[i] - var_position[ psf[i][j].place_no ]);
            // }
            safe_div(psf_pts, psf[i].coeffs.size());
            pts += psf_pts;
        }
    }
    // for (int p = 0; p < npl; p++)
    //     safe_div(grade[p], num_trns_of_place[p] + num_sf_per_place[p]);
    return pts;
}

//---------------------------------------------------------------------------------------

struct incremental_noack {
    std::vector<bool> S;    // already selected places
    // Size of the pre/post transitions sets
    std::vector<int> n_preT, n_postT, n_preT_S, n_postT_S;
    // Heap of the places, in weight order
    reverse_heap RH;
    // The Noack algorithm variation
    const VariableOrderCriteria voc;

    incremental_noack(VariableOrderCriteria _voc) : RH(npl), voc(_voc) { }

    void compute(std::vector<int> &out_order) {
        const pre_post_sets_t& PS = get_pre_post_sets(); // transitions connected to each place

        S.resize(npl, false);
        std::vector<int> update_list;
        update_list.reserve(npl);

        n_preT.resize(ntr, 0);
        n_postT.resize(ntr, 0);
        n_preT_S.resize(ntr, 0);
        n_postT_S.resize(ntr, 0);
        for (int p=0; p<npl; p++) {
            for (auto t : PS.preP[p]) // t in pre(p) <-> p in post(t)
                n_postT[t]++;
            for (auto t : PS.postP[p]) // t in post(p) <-> p in pre(t)
                n_preT[t]++;
        }

        // Initialize all weights and insert them into the heap
        for (int p=0; p<npl; p++)
            RH.push_heap(p, get_place_weight(p, PS));

        // Generate the variable order
        for (int pos=0; pos<npl; pos++) {
            double top_w = RH.top_weight();
            int p = RH.pop_heap(); // place with the highest weight
            S[p] = true;
            out_order[p] = pos;

            // Update the (pre/post T intersect S) counters
            for (auto t : PS.preP[p]) // t in pre(p) <-> p in post(t)
                n_postT_S[t]++;
            for (auto t : PS.postP[p]) // t in post(p) <-> p in pre(t)
                n_preT_S[t]++;

            // Get the list of places that require a weight update
            for (auto&& t : PS.preP[p]) { // t in pre(p)
                for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) // pre(t)
                    if (!S[GET_PLACE_INDEX(in)])
                        update_list.push_back(GET_PLACE_INDEX(in));
                for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) // post(t)
                    if (!S[GET_PLACE_INDEX(out)])
                        update_list.push_back(GET_PLACE_INDEX(out));
            }
            for (auto&& t : PS.postP[p]) { // t in post(p)
                for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) // pre(t)
                    if (!S[GET_PLACE_INDEX(in)])
                        update_list.push_back(GET_PLACE_INDEX(in));
                for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) // post(t)
                    if (!S[GET_PLACE_INDEX(out)])
                        update_list.push_back(GET_PLACE_INDEX(out));
            }

            // Remove duplicates in update_list
            std::sort(update_list.begin(), update_list.end());
            update_list.erase(std::unique(update_list.begin(), update_list.end()), update_list.end());

            // cout << "   select " << p <<  "   w=" <<top_w << "    "; // << "   update ";
            // for (int i=0; i<npl; i++)
            //     cout << (RH.get_weight(i) < 0 ? "-" : to_string(RH.get_weight(i))) << " ";
            // cout << endl;

            // Update the weights
            for (auto updP : update_list) {
                RH.update_weight(updP, get_place_weight(updP, PS));
            }
            update_list.resize(0);
        }
    }

    double get_place_weight(int p, const pre_post_sets_t& PS) const {
        double f = 0.0;
        switch (voc) {
            case VOC_TOVCHIGRECHKO2:
                // f(p) = sum_{t in pre(p)} (g1(t)/n_preT)           if n_preT>0
                //      + sum_{t in pre(p)} (g2(t)/n_postT)          if n_postT>0
                //      + sum_{t in post(p)} ((n_preT_S+1)/n_preT)   if n_preT>0
                //      + sum_{t in post(p)} (h(t)/n_postT)          if n_postT>0
                for (auto&& t : PS.preP[p]) { // t in pre(p)
                    if (n_preT[t] > 0) {
                        double g1_t = (n_preT_S[t] == 0) ? 0.1 : n_preT_S[t];
                        f += g1_t / n_preT[t];
                    }
                    if (n_postT[t] > 0) {
                        double g2_t = (n_postT_S[t] == 0) ? 0.1 : 2 * n_postT_S[t];
                        f += g2_t / n_postT[t];
                    }
                }
                for (auto&& t : PS.postP[p]) { // t in post(p)
                    if (n_preT[t] > 0) {
                        f += (n_preT_S[t] + 1.0) / n_preT[t];
                    }
                    if (n_postT[t] > 0) {
                        double h_t = (n_postT_S[t] == 0) ? 0.2 : 2 * n_postT_S[t];
                        f += h_t / n_postT[t];
                    }
                }
                break;

            case VOC_NOACK2:
                // f(p) = sum_{t in pre(p)} (g1(t)/n_preT + 2*n_postT_S/n_postT) +
                //        sum_{t in post(p)} (h(t)/n_postT + (1+n_preT_S)/n_preT)
                //        only if n_preT>0 and n_postT>0
                for (auto&& t : PS.preP[p]) { // t in pre(p)
                    if (n_preT[t] == 0 || n_postT[t] == 0)
                        continue;
                    double g_t = (n_preT_S[t] == 0) ? 0.1 : n_preT_S[t];
                    f += g_t / n_preT[t] + (2.0 * n_postT_S[t]) / n_postT[t];
                }
                for (auto&& t : PS.postP[p]) { // t in post(p)
                    if (n_preT[t] == 0 || n_postT[t] == 0)
                        continue;
                    double h_t = (n_postT_S[t] == 0) ? 0.2 : 2 * n_postT_S[t];
                    f += h_t / n_postT[t] + (n_preT_S[t] + 1) / n_preT[t];
                }
                break;

            default:
                throw rgmedd_exception();
        }
        size_t div_W = (PS.preP[p].size() + PS.postP[p].size());
        if (div_W > 0)
            f /= div_W;
        assert(!isinf(f) && !isnan(f));
        return f;
    }
};

//---------------------------------------------------------------------------------------
// Implements Noack and Tovchigrechko methods, as described in paper:
//   "MARCIE’s Secrets of Efficient Model Checking"
// with incremental weights update
void var_order_Noack_Tovchigrechko_fast(const VariableOrderCriteria voc, std::vector<int> &out_order) {
    if (ntr == 0)
        throw rgmedd_exception("var_order_Noack_Tovchigrechko_fast() cannot be used with 0 transitions.");
    incremental_noack inc_noack(voc);
    inc_noack.compute(out_order);
}

//---------------------------------------------------------------------------------------

// Implements Noack and Tovchigrechko methods, as described in paper:
//   "MARCIE’s Secrets of Efficient Model Checking"
// Old implementation: the cost of this method is at least O(P^2)
void var_order_noack_tovchigrechko(const VariableOrderCriteria voc, std::vector<int> &out_order) {
    if (ntr == 0)
        throw rgmedd_exception("var_order_noack_tovchigrechko() cannot be used with 0 transitions.");
    // Precompute divisor of W: div_W(p) = |pre(p) U post(p)|, and pre/post T sets sizes
    std::vector<int> div_W(npl, 0);
    for (int t=0; t<ntr; t++) {
        for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) 
            div_W[GET_PLACE_INDEX(in)]++;
        for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) 
            div_W[GET_PLACE_INDEX(out)]++;
    }

    std::vector<bool> S(npl, false); // already selected places
    std::vector<double> f(npl);      // intermediate weight of places
    std::vector<double> W(npl, 0);   // weight of places
    for (int pl=npl-1; pl>=0; pl--) {
        std::fill(f.begin(), f.end(), 0.0);
        for (int t=0; t<ntr; t++) {
            // Compute sizes of |pre(t) intersect S| and |post(t) intersect S|
            int n_preT_S = 0, n_postT_S = 0, n_preT = 0, n_postT = 0;
            for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in), n_preT++) {
                if (S[GET_PLACE_INDEX(in)])
                    n_preT_S++;
            }
            for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out), n_postT++) {
                if (S[GET_PLACE_INDEX(out)])
                    n_postT_S++;
            }
            double g1_t = (n_preT_S == 0) ? 0.1 : n_preT_S;
            double h_t = (n_postT_S == 0) ? 0.2 : 2 * n_postT_S;

            switch (voc) {
                case VOC_NOACK: {
                        // f(p) = sum_{t in pre(p)} (g1(t)/n_preT + 2*n_postT_S/n_postT) +
                        //        sum_{t in post(p)} (h(t)/n_postT + (1+n_preT_S)/n_preT)
                        //        only if n_preT>0 and n_postT>0
                        if (n_preT == 0 || n_postT == 0)
                            continue;
                        for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) {
                            if (!S[GET_PLACE_INDEX(in)])
                                f[GET_PLACE_INDEX(in)] += h_t / n_postT + (1.0 + n_preT_S) / n_preT;
                        }
                        for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) {
                            if (!S[GET_PLACE_INDEX(out)])
                                f[GET_PLACE_INDEX(out)] += g1_t / n_preT + (2.0 * n_postT_S) / n_postT;
                        }
                    }
                    break;

                case VOC_TOVCHIGRECHKO: {
                        double g2_t = (n_postT_S == 0) ? 0.1 : 2 * n_postT_S;
                        // f(p) = sum_{t in pre(p)} (g1(t)/n_preT)           if n_preT>0
                        //      + sum_{t in pre(p)} (g2(t)/n_postT)          if n_postT>0
                        //      + sum_{t in post(p)} ((n_preT_S+1)/n_preT)   if n_preT>0
                        //      + sum_{t in post(p)} (h(t)/n_postT)          if n_postT>0
                        for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) {
                            if (S[GET_PLACE_INDEX(in)])
                                continue;
                            if (n_preT > 0)
                                f[GET_PLACE_INDEX(in)] += (n_preT_S + 1.0) / n_preT;
                            if (n_postT > 0)
                                f[GET_PLACE_INDEX(in)] += h_t / n_postT;
                        }
                        for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) {
                            if (S[GET_PLACE_INDEX(out)])
                                continue;
                            if (n_preT > 0)
                                f[GET_PLACE_INDEX(out)] += g1_t / n_preT;
                            if (n_postT > 0)
                                f[GET_PLACE_INDEX(out)] += g2_t / n_postT;
                        }
                    }
                    break;
                default:
                    throw rgmedd_exception();
            }
        }
        // Compute weights:  W(p) = f(p) / div_W(p)
        for (int p=0; p<npl; p++)
            if (!S[p])
                W[p] = f[p] / (div_W[p] == 0 ? 1 : div_W[p]);

        // Select the place with the highest weight
        int selP = std::distance(W.begin(), std::max_element(W.begin(), W.end()));
        if (S[selP])
            throw rgmedd_exception("Bug in Noack/Tovchigrechko method implementation.");
        // cout << "   Noack: " << selP << "   w="<<W[selP] << "    ";
        S[selP] = true;
        W[selP] = -1000000.0;
        out_order[pl] = selP;

        // for (int i=0; i<npl; i++)
        //     cout << (W[i] < 0 ? "-" : to_string(W[i])) << " ";
        // cout << endl;
    }

    // cout << endl;
    // incremental_noack inc_noack(voc);
    // std::vector<int> out2(npl);
    // inc_noack.compute(out2);
}

//---------------------------------------------------------------------------------------

// // Write a bitmap that represents the variable interaction with a given variable order
// void write_var_order_as_NetPBM(const char* filename, const std::vector<int> &varorder)
// {
//     // bitmap vector
//     std::vector<bool> image(npl * npl);
//     std::fill(image.begin(), image.end(), false);
//     auto coord = [](int x, int y){ return x * npl + y; };

//     for (int tr=0; tr<ntr; tr++) {
//         for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
//             int plc_x = GET_PLACE_INDEX(in_ptr);

//             for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
//                 int plc_y = GET_PLACE_INDEX(out_ptr);

//                 image[coord(varorder[plc_x], varorder[plc_y])] = 1;
//                 image[coord(varorder[plc_y], varorder[plc_x])] = 1;
//             }
//         }
//     }
//     // Write bitmap file in NetPBM format.
//     ofstream pbm(filename);
//     pbm << "P1\n# heatmap\n" << npl << " " << npl << endl;
//     for (int y=0; y<npl; y++) {
//         for (int x=0; x<npl; x++) {
//             pbm << (image[coord(x,y)] ? 1 : 0) << (x == npl-1 ? "\n" : " ");
//         }
//     }
//     pbm.close();
// }

//---------------------------------------------------------------------------------------
















//---------------------------------------------------------------------------------------
// Faster reimplementation of the P-chaining heuristics using sparse data structures.
// Reengineering of the code originally written by Marieta, which was at least O(P^3) in cost
// This re-implkementation as almost linear in the number of places.
//---------------------------------------------------------------------------------------

static int find_min(const int_lin_constr_vec_t& PSF);
static int find_max(const int_lin_constr_vec_t& PSF, int keep, std::vector<int> &checked_pinv);
static bool common_places_are_not_touched(const std::vector<int>& not_touched, int position);

//---------------------------------------------------------------------------------------

// Efficient reimplementation of the P-chaining algorithm
void varorder_P_chaining(const int_lin_constr_vec_t& PSF, std::vector<int>& new_map_sort) {
    const size_t num_pinv = PSF.size();
    if (num_pinv == 0)
        throw rgmedd_exception("P-chaining requires a non-empty set of P-semiflows");
    bool move = true;

    new_map_sort.resize(npl);
    std::fill(new_map_sort.begin(), new_map_sort.end(), -1);
    std::vector<int> not_touched(npl, -1);
    std::vector<bool> selected_place(npl, false);  // stores if has already been inserted into new_map_sort[]
    std::vector<int> checked_pinv(num_pinv, -1); // stores if a p-invariant has already been selected

    int keep = find_min(PSF);
    // cout << "keep = " << keep << endl; 

    checked_pinv[keep] = keep;// keep the first p-inv i keep OLNY the pinvs that I have dealt with and are not needed any more
    int cnt = 0;

    while (move) {
        int keep_sec = find_max(PSF, keep, checked_pinv);
        // cout << "keep_sec = " << keep_sec << endl; 

        if (keep_sec == -1) { // if intersection is empty, put in the vector only the places of the first p-invariant
            for (auto& e : PSF[keep].coeffs) {
                if (!selected_place[e.index]) { // else i store it in the vector
                    new_map_sort[cnt++] = e.index;
                    selected_place[e.index] = true;
                }
            }
        }
        else { // if intersection is non empty
            // Store non-common places of keep
            for (auto& e : PSF[keep].coeffs) {
                bool is_common = false;
                for (auto& e2 : PSF[keep_sec].coeffs) {
                    if (e.index == e2.index) {
                        is_common = true;
                        continue; // common place
                    }
                }
                if (!is_common && !selected_place[e.index]) {
                    new_map_sort[cnt++] = e.index;
                    selected_place[e.index] = true;
                }
            }
            // Store common places between the two p-semiflows
            for (auto& e : PSF[keep].coeffs) {
                for (auto& e2 : PSF[keep_sec].coeffs) {
                    if (e.index == e2.index) {
                        if (!common_places_are_not_touched(not_touched, e.index)) { // if it isn't already in common places
                            if (!selected_place[e.index]) { // if it isn't on the new_map_sort
                                new_map_sort[cnt++] = e.index;
                                selected_place[e.index] = true;
                                not_touched[e.index] = e.index; // places which I am not allowed to touch
                            }
                            else { // if common places exists and I can touch it (isn't already in common )
                                int j;
                                for (j = 0; j < npl; j++) {
                                    if (new_map_sort[j] == e.index) // keep the position of new common place
                                        break;
                                }
                                // cout << "moving " << e.index << " from pos " << j << " to pos " << (cnt-1) << endl;
                                for (int tmp = j; tmp < cnt - 1; tmp++) {
                                    new_map_sort[tmp] = new_map_sort[tmp + 1]; // reorder the vector
                                }
                                new_map_sort[cnt - 1] = e.index;
                            }
                        }
                    }
                }
            }

            // Store non-common places of keep_sec
            for (auto& e2 : PSF[keep_sec].coeffs) {
                bool is_common = false;
                for (auto& e : PSF[keep].coeffs) {
                    if (e.index == e2.index) {
                        is_common = true;
                        continue; // common place
                    }
                }
                if (!is_common && !selected_place[e2.index]) {
                    new_map_sort[cnt++] = e2.index;
                    selected_place[e2.index] = true;
                }
            }
        }

        move = false;
        for (int i = 0; i < num_pinv; i++) {
            if (checked_pinv[i] != -1 || cnt == npl)
                continue;
            checked_pinv[i] = i;
            keep = i; // next p-invariant to deal with
            move = true;
            break;
        }
    }
}

//---------------------------------------------------------------------------------------
// Finds the P-invariant with the minimum intersection with the others
// and returns its position on the vector where P-invariants are stored
 
static int find_min(const int_lin_constr_vec_t& PSF) {
    const size_t num_pinv = PSF.size();
    int keep = 0;
    int min = 100;

    for (int i = 0; i < num_pinv; i++) {
        int inter_vec_min_i = 0;
        for (int j = i + 1; j < num_pinv; j++) {
            int k = 0;
            for (auto& e : PSF[j].coeffs) {
                // Search if e.place is in common with PSF[i]
                for (size_t h=0; h<PSF[i].coeffs.nonzeros(); h++) {
                    if (PSF[i].coeffs.ith_nonzero(h).index == e.index) {
                        k++;
                        break;
                    }
                }
            }
            if (k > 0) {
                inter_vec_min_i++;
            }

            if (inter_vec_min_i < min && k > 0) {
                min = inter_vec_min_i; // i shows the pinv with the minimum number of intersection with othe pinv
                keep = i; //BE AWARE it doesn't show the set of intersection only that there is intersection
            } // with another invariant
            if (k == 0) { // if there is no intersection with the other P-invariants
                min = 0;
                keep = i;
            }
        }
    }
    return keep;
}

//---------------------------------------------------------------------------------------
// Finds the P-invariant with the maximum intersection with the others
// and returns its position on the vector where P-invariants are stored
static int find_max(const int_lin_constr_vec_t& PSF, int keep, std::vector<int> &checked_pinv) {
    const size_t num_pinv = PSF.size();
    int max = -1;
    int keep_sec = 0;

    for (int i = 0; i < num_pinv; i++) {
        int num_inter_max_i = 0;
        if (i == checked_pinv[i]) { // if p-invariant is already checked_pinv
            continue;
        }
        else {
            int k = 0;
            for (auto& e : PSF[keep].coeffs) {
                // Search if e.place is in common with PSF[i]
                for (size_t h=0; h<PSF[i].coeffs.nonzeros(); h++) {
                    if (PSF[i].coeffs.ith_nonzero(h).index == e.index) {
                        k++;
                        num_inter_max_i++;
                        break;
                    }
                }
            }

            if (num_inter_max_i > max && k > 0) {
                max = num_inter_max_i;// i shows the pinv with the maximum number of intersection with othe pinv
                keep_sec = i; //BE AWARE it doesn't show the set of intersection only that there is intersection
                //k = 0;//printf("min intersection exei to %d \n",keep);
            }// with another invariant

            if (k == 0) {
                if (max == -1) {
                    keep_sec = -1; // if there is no intersection deal only with the first p-invariant (position = keep)
                }
            }
        }

    }
    return keep_sec;
}

//---------------------------------------------------------------------------------------
// Checks if the place already exists in the vector where common places are stored
// I know that this function with the above is the same. I wrote it twice to make it clear for me.
// Purpose of << reading >> the code. The final code will have only one function not both.
static bool common_places_are_not_touched(const std::vector<int>& not_touched, int position) {
    for (int i = 0; i < npl; i++) {
        if (not_touched[i] == position) { // to pithanotero na vriskontai stin idia thesi
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------









//---------------------------------------------------------------------------------------

void annealing_force(const Annealing ann,
                     std::vector<int>& out_order,
                     trans_span_set_t& trn_set,
                     flow_basis_metric_t* opt_fbm,
                     VariableOrderMetric target_metric)
{
    optimization_finder::conf conf;
    conf.temperature_delta = 0.0001;

    // compute a score from a variable order
    auto score_fn = [&](const std::vector<int>& net_to_level) -> double {
        metric_t m;
        metric_compute(net_to_level, trn_set ,opt_fbm, target_metric, &m);
        return m.cast_to_double();
    };

    // const int MAX_SWAPS = int(sqrt(npl)) + 2;
    const int MAX_SWAPS = int(npl/4) + 2;
    // generate a new solution from a previous one
    auto generate_fn = [&](const std::vector<int>& net_to_level, 
                           std::vector<int>& new_order) 
    {
        new_order = net_to_level;


        // size_t pos[3];
        // const size_t n_relocs = 1 + (genrand64_int63() % 5);
        // for (size_t rel=0; rel<n_relocs; rel++) {
        //     // Generate three different positions in the vector
        //     for (size_t i=0; i<3; i++) {
        //         while (true) {
        //             pos[i] = genrand64_int63() % npl;
        //             for (size_t j=0; j<i; j++)
        //                 if (pos[i] == pos[j])
        //                     continue; 
        //             break;
        //         }
        //     }
        //     std::sort(pos, pos+3);

        //     // Do the relocation
        //     std::rotate(new_order.begin() + pos[0], 
        //                 new_order.begin() + pos[1], 
        //                 new_order.begin() + pos[2]);
        // }

        // How many places do we want to swap?
        const size_t n_swaps = 2 + (genrand64_int63() % MAX_SWAPS);

        // Do the random swaps
        for (int i=0; i<n_swaps; i++)
            std::swap(new_order[genrand64_int63() % npl],
                      new_order[genrand64_int63() % npl]);

        // Apply FORCE and relax the variable order
        const int_lin_constr_vec_t empty_ilcp;
        switch (ann) {
            case Annealing::ANN_FORCE:
                var_order_FORCE(VOC_FORCE, new_order, new_order, empty_ilcp, false);
                break;

            case Annealing::ANN_FORCE_P:
                var_order_FORCE(VOC_FORCE, new_order, new_order, get_int_constr_problem(), false);
                break;

            case Annealing::ANN_FORCE_TI:
                assert(opt_fbm != nullptr);
                var_order_ti_force(*opt_fbm, trn_set, new_order, true, false);
                break;

            default:
                throw rgmedd_exception();
        }
    };

    // print a message when the procedure finds a better solution
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

    optimization_finder::optimize(out_order, score_fn, generate_fn, msg_fn, conf);
}

//---------------------------------------------------------------------------------------

void var_order_meta_force(std::vector<int>& best_net_to_level,
                          trans_span_set_t& trn_set,
                          flow_basis_metric_t* opt_fbm,
                          VariableOrderMetric target_metric,
                          const size_t num_attempts)
{
    std::vector<int> net_to_level(npl);
    for (size_t i = 0; i < npl; i++)
        net_to_level[i] = i;

    const int_lin_constr_vec_t empty_ilcp;
    metric_t best_metric;

    for (size_t i=0; i<num_attempts; i++) {
        // randomly reshuffle the vector
        for (size_t i = 0; i < npl; i++)
            std::swap(net_to_level[genrand64_int63() % npl],
                      net_to_level[genrand64_int63() % npl]);

        var_order_FORCE(VOC_FORCE, net_to_level, net_to_level, empty_ilcp, false);
        metric_t met;
        metric_compute(net_to_level, trn_set, opt_fbm, target_metric, &met);
        if (i==0 || met < best_metric) {
            best_metric = met;
            best_net_to_level = net_to_level;
            cout << right << setw(8) << i << " META-FORCE: " 
                 << metric_name(target_metric) << "=" << best_metric << endl;
        }
    }
}

//---------------------------------------------------------------------------------------
