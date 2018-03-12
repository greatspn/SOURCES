

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

extern bool g_show_var_order;
extern bool g_exit_after_varorder;
extern bool g_print_varorder_metrics;
extern bool g_refine_force;

//---------------------------------------------------------------------------------------
// Entry point - selects the appropriate method according to the criteria
//---------------------------------------------------------------------------------------

void determine_var_order(const VariableOrderCriteria criteria, 
                         bool save_image, bool reorder_SCC,
	                     const std::map<std::string, int>& S2Ipl, 
						 std::vector<int> &net_to_mddLevel,
                         bool verbose) 
{
    clock_t var_order_time = clock();
	net_to_mddLevel.resize(npl);
    std::fill(net_to_mddLevel.begin(), net_to_mddLevel.end(), -1);
    const char *short_name, *long_name;
    std::tie(short_name, long_name) = var_order_name(criteria);
    if (!running_for_MCC() && verbose) {
    	cout << "Variable order method: " << long_name << endl;
    }

    bool invert_mapping = false;
    bool fill_missing_vars = false;
    bool is_meta = false;

    compute_connected_components();

    switch (criteria) {
	    case VOC_NO_REORDER:
            // test_metric_scores();
	        // Places appear int the MDD levels in the same order of the .net file
	        for (size_t i = 0; i < npl; i++)
	            net_to_mddLevel[i] = i;
	        break;

        case VOC_DFS:
        case VOC_BFS: {
            pre_post_sets_t pps;
            pps.build();
            var_order_visit(criteria, net_to_mddLevel, pps, 0);
            break;
        }

	    case VOC_PINV: {
	    	if (read_PIN_file())
	    		throw rgmedd_exception("Cannot load P-invariants (.pin) file.");
	        int *pinv_order = sort_according_to_pinv();
	        free_PIN_file();
	        std::copy(pinv_order, pinv_order + npl, net_to_mddLevel.begin());
	        free(pinv_order);
	        fill_missing_vars = true;
	        invert_mapping = true;
	        // Invert the order (the function sort_according_to_pinv)
	        // constructs a mdd level -> net order, we need the opposite
	        break;
	    }
	    case VOC_FROM_FILE: 
	        var_order_from_file(net_to_mddLevel, S2Ipl);
	        fill_missing_vars = true;
	        break;

        case VOC_FORCE:
        case VOC_FORCE_PINV:
        case VOC_FORCE_NU: {
            pre_post_sets_t pps;
            pps.build();
            var_order_visit(VOC_BFS, net_to_mddLevel, pps, 0);
            var_order_FORCE(criteria, net_to_mddLevel, net_to_mddLevel);
            break;
        }

	    case VOC_MEAS_FORCE:
	    case VOC_MEAS_FORCE_NES:
	    case VOC_MEAS_FORCE_WES1:
            var_order_MeasFORCE(criteria, net_to_mddLevel);
	        break;

        case VOC_MEAS_FORCE_PINV:
	    case VOC_MEAS_FORCE_NU:
	        var_order_MeasFORCE_PINV(criteria, net_to_mddLevel);
	        break;

	    case VOC_CUTHILL_MCKEE:
	        var_order_cuthill_mckee(criteria, net_to_mddLevel);
	        break;

	    // case VOC_MINIMUM_DEGREE:
	    //     var_order_minimum_degree_ordering(criteria, net_to_mddLevel);
	    //     break;

	    case VOC_KING:
	        var_order_king_ordering(criteria, net_to_mddLevel);
	        break;

        case VOC_SLOAN:
	    case VOC_SLOAN_1_16:
	        var_order_sloan(criteria, net_to_mddLevel);
	        break;

	    case VOC_NOACK:
        // case VOC_NOACK_NU:
        case VOC_TOVCHIGRECHKO:
	    // case VOC_TOVCHIGRECHKO_NU:
	        var_order_noack_tovchigrechko(criteria, net_to_mddLevel);
	        std::reverse(net_to_mddLevel.begin(), net_to_mddLevel.end());
	        invert_mapping = true;
	        break;

	    case VOC_GRADIENT_P:
        case VOC_GRADIENT_NU:
        case VOC_MARKOV_CLUSTER:
	    	var_order_gradient_P(criteria, net_to_mddLevel);
	    	invert_mapping = true;
	    	break;

        case VOC_VCL_CUTHILL_MCKEE:
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:
            var_order_vcl(criteria, net_to_mddLevel);
            invert_mapping = true;
            break;

	    case VOC_META_BY_SCORE:
	    	metaheuristic_wscore(criteria, net_to_mddLevel, reorder_SCC, S2Ipl);
            is_meta = true;
	    	break;

        // case VOC_META_USE_ANN:
        //     metaheuristic_table(net_to_mddLevel, reorder_SCC, S2Ipl);
        //     is_meta = true;
        //     break;

        case VOC_PRINT_METRICS_AND_QUIT: {
            generate_varorder_metrics2(S2Ipl);
            // std::vector<double> metrics;
            // generate_varorder_metrics(metrics, true, reorder_SCC, S2Ipl);
            // cout << "\nMETRICS: ";
            // for (int i=0; i<metrics.size(); i++)
            //     cout << (i>0 ? "," : "") << setprecision(7) << metrics[i];
            // cout << endl;
            exit(15);
            break;
        }

	    default:
	        throw rgmedd_exception("Internal error: Unknown variable order criteria constant.\n");
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

    // Apply again a FORCE method over the generated variable ordering,
    // to try to improve it
    if (g_refine_force) {
        var_order_FORCE(VOC_FORCE, net_to_mddLevel, net_to_mddLevel);
    }

    // If the net is non-ergodic, reorder from bottom SCC to top SCC (if the heuristic agrees)
    reorder_using_SCC(net_to_mddLevel, reorder_SCC);

    // Print the place order we use
    if (!running_for_MCC()) {
        var_order_time = clock() - var_order_time;
        if (!running_for_MCC() && verbose)
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
        
        if (g_show_var_order) { 
            cout << "\nVARORDER: ";
            for (int p = 0; p < npl; p++)
                cout << " " << tabp[ mddLevel_to_net[p] ].place_name;
            cout << endl;
        }

        if (g_print_varorder_metrics) {
            cout << "\nMETRICS:\n";
            cout << "NES:    " << measure_NES(net_to_mddLevel) << endl;
            cout << "WES(1): " << measure_WESi(net_to_mddLevel, 1) << endl;
        }
    }

    if (save_image) {
        std::string pbm = net_name;
        pbm += "pbm";
        cout << "WRITING VAR ORDER BITMAP TO " << pbm << " ..." << endl;
        write_var_order_as_NetPBM(pbm.c_str(), net_to_mddLevel);    	
    }

    if (g_exit_after_varorder) {
        cout << "\nEXIT." << endl;
        exit(0);
    }
}

//---------------------------------------------------------------------------------------

// Names of the variable orders
std::pair<const char*, const char*> var_order_name(VariableOrderCriteria voc) {
	switch (voc) {
		case VOC_NO_REORDER:		          return std::make_pair("",           "No Reorder");
    	case VOC_FROM_FILE:                   return std::make_pair("F",          "From File");
        case VOC_DFS:                         return std::make_pair("DFS",        "Depth-First Search visit");
        case VOC_BFS:                         return std::make_pair("BFS",        "Breadth-First Search visit");
    	case VOC_PINV:                        return std::make_pair("P",          "Use P-invariants");
        case VOC_FORCE:                       return std::make_pair("FR",         "FORCE method");
        case VOC_FORCE_PINV:                  return std::make_pair("FR-P",       "FORCE method with P-semiflows");
        case VOC_FORCE_NU:                    return std::make_pair("FR-NU",      "FORCE method with Nested Units");
    	case VOC_MEAS_FORCE:                  return std::make_pair("FORCE",      "FORCE with place span metric");
    	case VOC_MEAS_FORCE_NES:              return std::make_pair("FORCE-NES",  "FORCE with NES metric");
    	case VOC_MEAS_FORCE_WES1:             return std::make_pair("FORCE-WES1", "FORCE with WES(1) metric");
    	case VOC_MEAS_FORCE_PINV:             return std::make_pair("FORCE-P",    "FORCE with P-invariants");
        case VOC_MEAS_FORCE_NU:               return std::make_pair("FORCE-NU",   "FORCE with Nested Units");
    	case VOC_CUTHILL_MCKEE:               return std::make_pair("CM",         "Cuthill Mckee method");
    	// case VOC_MINIMUM_DEGREE:              return std::make_pair("MDO",        "Minimum Degree ordering");
    	case VOC_KING:                        return std::make_pair("KING",       "King ordering");
        case VOC_SLOAN:                       return std::make_pair("SLO",        "Sloan method");
    	case VOC_SLOAN_1_16:                  return std::make_pair("SLO-16",     "Sloan method with W1=1, W2=16");
    	case VOC_NOACK:                       return std::make_pair("NOACK",      "Noack method");
        // case VOC_NOACK_NU:                    return std::make_pair("NOACK-NU",   "NU-aware Noack method");
    	case VOC_TOVCHIGRECHKO:               return std::make_pair("TOV",        "Tovchigrechko method");
        // case VOC_TOVCHIGRECHKO_NU:            return std::make_pair("TOV-NU",     "NU-aware Tovchigrechko method");
        case VOC_GRADIENT_P:                  return std::make_pair("GP",         "Gradient P-invariants");
        case VOC_MARKOV_CLUSTER:              return std::make_pair("MCL",        "Markov Cluster Algorithm");
    	case VOC_GRADIENT_NU:                 return std::make_pair("GNU",        "Gradient Nested Units");
        case VOC_VCL_CUTHILL_MCKEE:           return std::make_pair("CM2",        "VCL Cuthill-Mckee method");
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:  return std::make_pair("ACM",        "VCL Advanced Cuthill-Mckee method");
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:  return std::make_pair("GPS",        "VCL Gibbs-Poole-StockMeyer method");

    	case VOC_META_BY_SCORE:               return std::make_pair("META",       "Meta-heuristic using weighted score.");
 
        case VOC_PRINT_METRICS_AND_QUIT:      return std::make_pair("--",         "--no reorder--");
    	case VOC_TOTAL_CRITERIAS:             break; // throw
	}
	throw rgmedd_exception("Missing algorithm name in var_order_name()");
}

//---------------------------------------------------------------------------------------

bool method_uses_pinvs(VariableOrderCriteria voc) {
	switch (voc) {
		case VOC_NO_REORDER:
    	case VOC_FROM_FILE:
        case VOC_DFS:
        case VOC_BFS:
        case VOC_FORCE:
        case VOC_FORCE_NU:
    	case VOC_MEAS_FORCE:
    	case VOC_MEAS_FORCE_NES:
        case VOC_MEAS_FORCE_WES1:
    	case VOC_MEAS_FORCE_NU:
    	case VOC_CUTHILL_MCKEE:
    	// case VOC_MINIMUM_DEGREE:
    	case VOC_KING:
    	case VOC_SLOAN:
        case VOC_SLOAN_1_16:
    	case VOC_NOACK:
        // case VOC_NOACK_NU:
        case VOC_TOVCHIGRECHKO:
        case VOC_MARKOV_CLUSTER:
    	// case VOC_TOVCHIGRECHKO_NU:
    	case VOC_GRADIENT_NU:
        case VOC_VCL_CUTHILL_MCKEE:
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:
        case VOC_PRINT_METRICS_AND_QUIT:
    		return false;

        case VOC_FORCE_PINV:
    	case VOC_MEAS_FORCE_PINV:
    	case VOC_PINV:
    	case VOC_GRADIENT_P:
    		return true;

    	case VOC_META_BY_SCORE:
    	case VOC_TOTAL_CRITERIAS:
    		break; // throw
	}
	throw rgmedd_exception("Missing algorithm name in method_uses_pinvs()");	
}

//---------------------------------------------------------------------------------------

bool method_uses_nested_units(VariableOrderCriteria voc) {
    switch (voc) {
        case VOC_NO_REORDER:
        case VOC_FROM_FILE:
        case VOC_DFS:
        case VOC_BFS:
        case VOC_FORCE:
        case VOC_FORCE_PINV:
        case VOC_MEAS_FORCE:
        case VOC_MEAS_FORCE_NES:
        case VOC_MEAS_FORCE_WES1:
        case VOC_MEAS_FORCE_PINV:
        case VOC_CUTHILL_MCKEE:
        // case VOC_MINIMUM_DEGREE:
        case VOC_KING:
        case VOC_SLOAN:
        case VOC_SLOAN_1_16:
        case VOC_NOACK:
        case VOC_TOVCHIGRECHKO:
        case VOC_PRINT_METRICS_AND_QUIT:
        case VOC_PINV:
        case VOC_GRADIENT_P:
        case VOC_MARKOV_CLUSTER:
        case VOC_VCL_CUTHILL_MCKEE:
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:
            return false;

        case VOC_FORCE_NU:
        case VOC_MEAS_FORCE_NU:
        case VOC_GRADIENT_NU:
        // case VOC_NOACK_NU:
        // case VOC_TOVCHIGRECHKO_NU:
            return true;

        case VOC_META_BY_SCORE:
        case VOC_TOTAL_CRITERIAS:
            break; // throw
    }
    throw rgmedd_exception("Missing algorithm name in method_uses_nested_units()");    
}

//---------------------------------------------------------------------------------------

// Load the variable order from a list of place names written on a file  <netname>.place
void var_order_from_file(std::vector<int> &out_order, const std::map<std::string, int>& S2Ipl) 
{
    ifstream in;
    std::string buffer = net_name + std::string("place");
    in.open(buffer.c_str());
    if (!in) {
        ostringstream msg;
        msg << "Error opening input stream: " << buffer << " .";
        throw rgmedd_exception(msg.str());
    }
    out_order.resize(npl);
    std::fill(out_order.begin(), out_order.end(), -1);
    std::string name;
    for (unsigned int i = 0; i < npl; i++) {
        in >> name;
        auto iter = S2Ipl.find(name);
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

bool load_Psemiflows(std::vector<PSemiflow> &psf) {
    std::string pinv_name(net_name);
    pinv_name += "pin";
    ifstream pif(pinv_name.c_str());
    if (!pif)
        return false;
    int num_pinv;
    pif >> num_pinv;
    if (!pif || num_pinv < 0)
		throw rgmedd_exception("Bad P-invariants file. [1]");
    psf.resize(num_pinv);
    for (int i = 0; i < num_pinv; i++) {
        int len;
        pif >> len;
        if (!pif || len < 0)
			throw rgmedd_exception("Bad P-invariants file. [2]");
        psf[i].resize(len);
        for (int j = 0; j < len; j++) {
            int card, pl;
            pif >> card >> pl;
            if (!pif || card <= 0 || pl <= 0 || pl > npl)
				throw rgmedd_exception("Bad P-invariants file. [3]");
            psf[i][j].place_no = pl - 1;
            psf[i][j].card = card;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------

bool model_has_nested_units() {
    return (num_nested_units != 0);
}

//---------------------------------------------------------------------------------------

// NuPN: convert nested units into fake p-semiflows, to be injected as inputs to
// P-semiflows based variable orders
bool convert_nested_units_as_semiflows(std::vector<PSemiflow> &psf) 
{
    if (!model_has_nested_units())
        return false;
    psf.resize(num_nested_units);
    for (int i=0; i<num_nested_units; i++) {
        psf[i].resize(nu_array[i]->num_places);
        // Read unit' places
        for (int p=0; p<psf[i].size(); p++) {
            psf[i][p].place_no = nu_array[i]->places[p];
            psf[i][p].card = 1; 
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------

bool load_flow_data(std::vector<PSemiflow> &psf, VariableOrderCriteria criteria) 
{
    switch (criteria) {
        case VOC_GRADIENT_P:
        case VOC_FORCE_PINV:
        case VOC_MEAS_FORCE_PINV:
            // Use P-invariants
            if (!load_Psemiflows(psf))
                throw rgmedd_exception("No P-semiflows file found. Select another ordering method.");
            break;

        case VOC_GRADIENT_NU:
        case VOC_FORCE_NU:
        case VOC_MEAS_FORCE_NU:
            // Use nested units
            if (!convert_nested_units_as_semiflows(psf))
                throw rgmedd_exception("No Nested Units file found. Select another ordering method.");
            break;

        case VOC_MARKOV_CLUSTER: 
            // Use place clusters computed using MCL
            mcl_cluster_net(psf);
            break;

        default:
            throw rgmedd_exception("Missing method in load_flow_data().");
    }
    return true;
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

double measure_CRS(const std::vector<int> &varorder) {
	// Compute how many events cross each variable
	std::vector<int> evXvar(varorder.size(), 0);
    for (int t = 0; t < ntr; t++) {
		int top, bot;
		std::tie(top, bot) = transition_top_bot(varorder, t);
		for (int l=bot; l<top; l++)
			evXvar[l]++;
	}
	double CRS = 0.0;
	for (int t = 0; t < ntr; t++) {
		int top, bot;
		std::tie(top, bot) = transition_top_bot(varorder, t);
		for (int l=bot; l<top; l++)
			CRS += evXvar[l];
	}
	return CRS / (ntr * (npl/2.0) * npl);
}

double measure_CRSi(const std::vector<int> &varorder, int i) {
	// Compute how many events cross each variable
	std::vector<int> evXvar(varorder.size(), 0);
    for (int t = 0; t < ntr; t++) {
		int top, bot;
		std::tie(top, bot) = transition_top_bot(varorder, t);
		for (int l=bot; l<top; l++)
			evXvar[l]++;
	}
	double CRSi = 0.0;
	for (int t = 0; t < ntr; t++) {
		int top, bot;
		std::tie(top, bot) = transition_top_bot(varorder, t);
		for (int l=bot; l<top; l++)
			CRSi += evXvar[l] * pow(top / (npl / 2.), i);
	}
	return CRSi / (ntr * (npl/2.0) * npl);
}

//---------------------------------------------------------------------------------------

// Maps [0..1] -> [0..1] with a (non) linear function
inline double unit_filter(double x, UnitFilter uf) {
    switch (uf) {
        case UnitFilter::LINEAR:       return x;
        case UnitFilter::CUBIC:        return x * x * x;
        case UnitFilter::QUARTIC:      return x * x * x * x;
        case UnitFilter::EXP:          return (exp(x) - 1) / 1.718281828459045235360287471352662497757;
        case UnitFilter::INV_COSINE:   return 1 - cos(x * 1.570796326794896619231321691639751442099);
    }
    throw rgmedd_exception();
}

//---------------------------------------------------------------------------------------

// Non-linear (normalized) weighted event span
double measure_NLWES(const std::vector<int> &varorder, UnitFilter span_filter, int i) {
    double NLWES = 0.0;
    for (int t = 0; t < ntr; t++) {
        // if (!is_transition_SCC_local[t]) 
        //     continue;
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        double norm_span = double(top - bot + 1) / npl;
        double nls = unit_filter(norm_span, span_filter);
        if (i != 0)
            nls *= pow(top / (npl / 2.), i);
        NLWES += nls;
    }
    NLWES /= ntr;

    return NLWES;
}

//---------------------------------------------------------------------------------------

// Normalized and Weighted-normalized Event Spans
std::pair<accumulator, accumulator> 
measure_NWES(const std::vector<int> &varorder) {
    accumulator NES, WES;
    for (int t = 0; t < ntr; t++) {
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        double norm_span = double(top - bot + 1) / npl;
        // double nls = unit_filter(norm_span, span_filter);
        NES += norm_span;
        WES += norm_span * pow(top / (npl / 2.), 1);
    }
    return make_pair(NES, WES);
}

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

std::pair<accumulator, accumulator>
measure_WCRS(const std::vector<int> &varorder) {
    accumulator CRS, WCRS;
    // Compute how many events cross each variable
    std::vector<int> evXvar(varorder.size(), 0);
    for (int t = 0; t < ntr; t++) {
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        for (int l=bot; l<top; l++)
            evXvar[l]++;
    }
    double PxT = npl * ntr;
    for (int t = 0; t < ntr; t++) {
        double cross = 0.0, wcross = 0.0;
        int top, bot;
        std::tie(top, bot) = transition_top_bot(varorder, t);
        for (int l=bot; l<top; l++) {
            cross += (evXvar[l] / PxT);
            wcross += (evXvar[l] / PxT) * pow(top / (npl / 2.), 1);
        }
        CRS += cross;
        WCRS += wcross;
    }
    return make_pair(CRS, WCRS);
}

//---------------------------------------------------------------------------------------

template<typename T, typename D>
void safe_div(T &value, const D divisor) {
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
void var_order_visit(const VariableOrderCriteria voc, std::vector<int> &order,
                     const pre_post_sets_t& pps, int start) 
{
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
                     const std::vector<int> &in_order) 
{
    std::vector<int> num_trns_of_place(npl);
    std::vector<double> cog(ntr), grade(npl);
    std::vector<std::pair<double, int>> sorter(npl);
    double last_pts = -1;

    // Load P-semiflows (if available)
    std::vector<PSemiflow> psf;
    bool has_semiflows = (voc != VOC_FORCE) && load_flow_data(psf, voc);
    std::vector<int> num_sf_per_place(npl);
    std::vector<double> psf_cog(psf.size());
    if (has_semiflows) {
        // Count the number of P-semiflows that cover each place
        for (int i = 0; i < psf.size(); i++)
            for (int j = 0; j < psf[i].size(); j++)
                num_sf_per_place[ psf[i][j].place_no ]++;
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
        if (has_semiflows) {
            for (int i = 0; i < psf.size(); i++) {
                psf_cog[i] = 0.0;
                for (int j = 0; j < psf[i].size(); j++)
                    psf_cog[i] += var_position[ psf[i][j].place_no ];
                safe_div(psf_cog[i], psf[i].size());
            }
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
        if (has_semiflows) { // Add also the PSFCOG[i] to each place
            for (int i = 0; i < psf.size(); i++) {
                double psf_pts = 0.0; // point-PSF span
                for (int j = 0; j < psf[i].size(); j++) {
                    grade[ psf[i][j].place_no ] += psf_cog[i];
                    psf_pts += std::abs(psf_cog[i] - var_position[ psf[i][j].place_no ]);
                }
                safe_div(psf_pts, psf[i].size());
                pts += psf_pts;
            }
        }
        for (int p = 0; p < npl; p++)
            safe_div(grade[p], num_trns_of_place[p] + num_sf_per_place[p]);

        // Check if we reached convergence. End as soon as the new order
        // does not improve the PTS measure over the previous order.
        cout << step << "\t" << pts << endl;
        if (last_pts >= 0 && last_pts <= pts && step > MIN_STEPS)
            break; // Not converging any longer
        last_pts = pts;

        // Check if we consumed too much time doing FORCE iterations
        clock_t elapsed = clock() - time_start;
        if (elapsed > FORCE_MAX_SECONDS * CLOCKS_PER_SEC) {
            if (!running_for_MCC())
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
            // var_position[p] = sorter[p].second;
            var_position[ sorter[p].second ] = p;
    }

    // Copy the final order
    assert(out_order.size() == npl);
    for (int p = 0; p < npl; p++) {
        out_order[p] = var_position[p];
        // out_order[ var_position[p] ] = p;
    }
}


//---------------------------------------------------------------------------------------

// Implements a modified FORCE heuristic for the ordering of variables in a Decision Diagram
// Force iterations sample the order that minimizes a target measure among the many orders generated.
// See: Aloul, Markov, Sakallah, "FORCE: a fast and easy-to-implement variable-ordering heuristic"
void var_order_MeasFORCE(const VariableOrderCriteria voc, std::vector<int> &out_order) {
    std::vector<int> var_order(npl), num_trns_of_place(npl);
    std::vector<double> cog(ntr), grade(npl);
    std::vector<std::pair<double, int>> sorter(npl);

    // The ordering that minimizes the measure function
    std::vector<int> min_varorder;
    double min_points = -1;

    // Build the support structures
    pre_post_sets_t pps;
    pps.build();
    // Count the number of transitions connected to each place
    std::fill(num_trns_of_place.begin(), num_trns_of_place.end(), 0);
    for (int t = 0; t < ntr; t++) {
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
            num_trns_of_place[ in_node->place_no ]++;
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
            num_trns_of_place[ out_node->place_no ]++;
    }

    clock_t time_start = clock();

    const int NUM_STEPS = 200, RESTART_FREQ = 20;
    for (int step = 0; step < NUM_STEPS; step++) {
        if ((step % RESTART_FREQ) == 0) {
            // Guess an initial ordering
            var_order_visit(VOC_BFS, var_order, pps, (npl * step) / NUM_STEPS);
        }

        // Recompute center-of-gravity (COG) of each transition
        //   COG(t) = Sum( grade(p) ) / num(p)      where variable p is connected with t
        for (int t = 0; t < ntr; t++) {
            cog[t] = 0.0;
            int num_pl = 0;
            for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, ++num_pl)
                cog[t] += var_order[ in_node->place_no ] + 1;
            for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl)
                cog[t] += var_order[ out_node->place_no ] + 1;
            safe_div(cog[t], num_pl);
        }
        // Compute the new grade of each variable, using the transition COGs
        //   grade(p) = Sum( COG[t] ) / num(t)      where transition t is connected with p
        std::fill(grade.begin(), grade.end(), 0.0);
        for (int t = 0; t < ntr; t++) {
            for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
                grade[ in_node->place_no ] += cog[t];
            for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
                grade[ out_node->place_no ] += cog[t];
        }
        for (int p = 0; p < npl; p++)
            safe_div(grade[p], num_trns_of_place[p]);

        // Reorder the variables according to their grades, and assign them a new
        // integer positioning from 1 to npl (which becomes the new variable
        // position in the next iteration)
        for (int p = 0; p < npl; p++)
            sorter[p] = std::make_pair(grade[p], p);
        std::sort(sorter.begin(), sorter.end());
        for (int p = 0; p < npl; p++)
            var_order[p] = sorter[p].second;

        // ostringstream filename;
        // filename << "force-" << setw(2) << setfill('0') << step << ".pbm";
        // for (int p = 0; p < npl; p++)
        //     out_order[ var_order[p] ] = p;
        // write_var_order_as_NetPBM(filename.str().c_str(), out_order);

        // We need to invert the order before computing the score function
        // Recycle out_order as our support vector for this purpose.
        for (int p = 0; p < npl; p++) {
            out_order[ var_order[p] ] = p;
        }

        // Compute the target measure on this new variable order
        // The objective is to take the ordering that minimizes this measure
        double points;
        switch (voc) {
            case VOC_MEAS_FORCE:
                points = estimate_points_transition_spans(out_order);
                break;
            case VOC_MEAS_FORCE_NES:
                points = measure_NES(out_order);
                break;
            case VOC_MEAS_FORCE_WES1:
            case VOC_MEAS_FORCE_PINV:
                points = measure_WESi(out_order, 1);
                break;
            default:
                throw rgmedd_exception("Unexpected value!");
        }
        // double points = measure_WESi(var_order, 1);
        if (min_points < 0 || points < min_points) {
            min_points = points;
            min_varorder = var_order;
        }

        clock_t elapsed = clock() - time_start;
        if (elapsed > FORCE_MAX_SECONDS * CLOCKS_PER_SEC) {
            if (!running_for_MCC())
                cout << "Quit FORCE iteration loop." << endl;
            break;
        }
    }

    assert(out_order.size() == npl);
    for (int p = 0; p < npl; p++) {
        // out_order[p] = min_varorder[p];
        out_order[ min_varorder[p] ] = p;
    }
}

//---------------------------------------------------------------------------------------

void var_order_MeasFORCE_PINV(const VariableOrderCriteria voc, std::vector<int> &out_order)
{
    // Load P-semiflows (if available)
    std::vector<PSemiflow> psf;
    bool has_semiflows = load_flow_data(psf, voc);

    std::vector<int> var_order(npl), num_sf_per_place(npl), num_trns_of_place(npl);;
    std::vector<double> psf_cog(psf.size()), trn_cog(ntr), place_grades(npl);
    std::vector<std::pair<double, int> > sorter(npl);

    // The ordering that minimizes the measure function
    std::vector<int> min_varorder;
    double min_points = -1;

    // Create the initial ordering for the first iteration
    for (int i = 0; i < npl; i++) {
        var_order[i] = i;
        num_sf_per_place[i] = 0;
    }
    // Count the number of transitions connected to each place
    for (int t = 0; t < ntr; t++) {
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
            num_trns_of_place[ in_node->place_no ]++;
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
            num_trns_of_place[ out_node->place_no ]++;
    }
    // Count the number of P-semiflows that cover each place
    if (has_semiflows)
        for (int i = 0; i < psf.size(); i++)
            for (int j = 0; j < psf[i].size(); j++)
                num_sf_per_place[ psf[i][j].place_no ]++;

    clock_t time_start = clock();
    const int NUM_RESETS = 30;
    const int NN = (npl < 100 ? 10 : int(sqrt(npl)));
    const int NUM_PSF_STEPS = 3 * NN;
    const int NUM_TRN_STEPS = 6 * NN;
    int phase = 0, reset = 0;
    while (reset < NUM_RESETS) {
        bool do_Pstep;
        double mix_factor;
        if (phase < NUM_PSF_STEPS) {
            do_Pstep = true;
            mix_factor = 1.0 - double(phase) / (2 * NUM_PSF_STEPS);
            phase++;
        }
        else if (phase - NUM_PSF_STEPS < NUM_TRN_STEPS) {
            do_Pstep = false;
            mix_factor = 1.0 - double(phase - NUM_PSF_STEPS) / NUM_TRN_STEPS;
            phase++;
        }
        else {
            reset++;
            phase = 0;
            std::random_shuffle(var_order.begin(), var_order.end());
            continue;
        }

        if (has_semiflows && do_Pstep) { // Weight the P-semiflows
            // Recompute center-of-gravity (COG) of each P-semiflow
            //   COG(i) = Sum( grade(p) ) / num(p)      where variable p is connected with i
            for (int i = 0; i < psf.size(); i++) {
                double cog_semiflow = 0.0;
                for (int j = 0; j < psf[i].size(); j++)
                    cog_semiflow += var_order[ psf[i][j].place_no ];
                safe_div(cog_semiflow, psf[i].size());
                psf_cog[i] = cog_semiflow;
            }

            // Compute the new grade of each variable, using the semiflow COGs
            //   grade(p) = Sum( COG[sf] ) / num(sf)      where semiflow sf covers place p
            for (int p = 0; p < npl; p++)
                place_grades[p] = 0.0;
            for (int i = 0; i < psf.size(); i++)
                for (int j = 0; j < psf[i].size(); j++)
                    place_grades[ psf[i][j].place_no ] += psf_cog[i];
            for (int p = 0; p < npl; p++)
                safe_div(place_grades[p], num_sf_per_place[p]);
        }
        else { // Weight the transitions
            // Recompute center-of-gravity (COG) of each transition
            //   COG(t) = Sum( grade(p) ) / num(p)      where variable p is connected with t
            for (int t = 0; t < ntr; t++) {
                double cog_t = 0.0;
                int num_pl = 0;
                for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next, ++num_pl)
                    cog_t += var_order[ in_node->place_no ] + 1;
                for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl)
                    cog_t += var_order[ out_node->place_no ] + 1;
                safe_div(cog_t, num_pl);
                trn_cog[t] = cog_t;
            }
            // Compute the new grade of each variable, using the transition COGs
            //   grade(p) = Sum( COG[t] ) / num(t)      where transition t is connected with p
            for (int p = 0; p < npl; p++)
                place_grades[p] = 0.0;
            for (int t = 0; t < ntr; t++) {
                for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
                    place_grades[ in_node->place_no ] += trn_cog[t];
            }
            for (int t = 0; t < ntr; t++) {
                for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
                    place_grades[ out_node->place_no ] += trn_cog[t];
            }
            for (int p = 0; p < npl; p++)
                safe_div(place_grades[p], num_trns_of_place[p]);
        }

        // Under-relax the repositionings
        for (int p = 0; p < npl; p++) {
            place_grades[p] += (1.0 - mix_factor) * (place_grades[p] - var_order[p]);
        }

        // Reorder the variables according to their grades, and assign them a new
        // integer position from 1 to npl (which becomes the new variable
        // position in the next iteration)
        for (int p = 0; p < npl; p++)
            sorter[p] = std::make_pair(place_grades[p], p);
        std::sort(sorter.begin(), sorter.end());
        for (int p = 0; p < npl; p++)
            var_order[p] = sorter[p].second;

        // We need to invert the order before computing the measure function
        // Recycle out_order as our support vector for this purpose.
        for (int p = 0; p < npl; p++) {
            out_order[ var_order[p] ] = p;
        }

        // Compute the target measure on this new variable order
        // The objective is to take the ordering that minimizes this measure
        double points;
        switch (voc) {
            case VOC_MEAS_FORCE:
                points = estimate_points_transition_spans(out_order);
                break;
            case VOC_MEAS_FORCE_NES:
                points = measure_NES(out_order);
                break;
            case VOC_MEAS_FORCE_WES1:
            case VOC_MEAS_FORCE_PINV:
            case VOC_MEAS_FORCE_NU:
                points = measure_WESi(out_order, 1);
                break;
            default:
                throw rgmedd_exception("Unexpected value!");
        }
        if (min_points < 0 || points < min_points) {
            min_points = points;
            min_varorder = var_order;
        }

        clock_t elapsed = clock() - time_start;
        if (elapsed > FORCE_MAX_SECONDS * CLOCKS_PER_SEC) {
            if (!running_for_MCC())
                cout << "Quit FORCE iteration loop." << endl;
            break;
        }

        // cout << "STEP " << step << endl;
        // ofstream plofs("model.place");
        // if (!plofs)
        //     exit(-5);
        // for (size_t i=0; i<npl; i++)
        //     plofs << tabp[ var_order[i] ].place_name << " ";
        // plofs.close();

        // cout << "XYZ\t" << step << "\t" << (int)points << "  " << flush;
        // system("~/BenchKit/bin/GreatSPN/SOURCES/bin/RGMEDD model -F | grep 'RS nodes' | cut -d : -f 2");
        // cout << endl;
        // }
    }

    assert(out_order.size() == npl);
    for (int p = 0; p < npl; p++) {
        assert(min_varorder[p] >= 0 && min_varorder[p] < npl);
        out_order[ min_varorder[p] ] = p;
    }
}

//---------------------------------------------------------------------------------------

// Write a bitmap that represents the variable interaction with a given variable order
void write_var_order_as_NetPBM(const char* filename, const std::vector<int> &varorder)
{
    // bitmap vector
    std::vector<bool> image(npl * npl);
    std::fill(image.begin(), image.end(), false);
    auto coord = [](int x, int y){ return x * npl + y; };

    for (int tr=0; tr<ntr; tr++) {
        for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
            int plc_x = GET_PLACE_INDEX(in_ptr);

            for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
                int plc_y = GET_PLACE_INDEX(out_ptr);

                image[coord(varorder[plc_x], varorder[plc_y])] = 1;
                image[coord(varorder[plc_y], varorder[plc_x])] = 1;
            }
        }
    }
    // Write bitmap file in NetPBM format.
    ofstream pbm(filename);
    pbm << "P1\n# heatmap\n" << npl << " " << npl << endl;
    for (int y=0; y<npl; y++) {
        for (int x=0; x<npl; x++) {
            pbm << (image[coord(x,y)] ? 1 : 0) << (x == npl-1 ? "\n" : " ");
        }
    }
    pbm.close();
}

//---------------------------------------------------------------------------------------

