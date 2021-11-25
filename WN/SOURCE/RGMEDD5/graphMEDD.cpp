#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <set>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>


// #ifndef __PRS_H__
// #define __PRS_H__
// #include "../AUTOMA/parser.h"
// #endif

#include "rgmedd5.h"
#include "parallel.h"

extern "C" {

extern bool eval_CTL_using_SatELTL;
extern bool test_LRS_RS_equiv;

// #include "../../INCLUDE/const.h"
// #include "../../INCLUDE/struct.h"
// #include "../../INCLUDE/var_ext.h"
// #include "../../INCLUDE/fun_ext.h"
// #include "../../INCLUDE/macros.h"
// #include "../../INCLUDE/ealloc.h"


#define PERFORMANCE 1
#define QUASIPRODFORM 0



    Tree_p reached_marking = NULL;
    Tree_p initial_marking = NULL;
    Tree_p current_marking = NULL;

    Result_p enabled_head = NULL;	/* Puntatori alla lista delle tr. abil. */

    int tro;			/* 1 marc. gia' raggiunta, 0 altrimenti */
    int marcatura = 0;	/* contatore per le marcature		*/
    int h = 0;			/* per bilanciamento nella insert_tree	*/
    unsigned long tang = 0;		/* contatori tipi di marc. raggiunte	*/
    unsigned long evan = 0;
    unsigned long dead = 0;
    int home = 0;
    unsigned long cont_tang;
    int count_arc = 0;


    int cur_priority;

    int *code_place = NULL;
    int *min_place = NULL;
    int *max_place = NULL;
    int *init_place = NULL;
    char IstanceName[MAX_TAG_SIZE];
    int max_priority = 0;
    time_t old_time, new_time;




    unsigned long d_ptr;
    unsigned long length;
    unsigned long f_mark;
    int f_bot;
    int f_throu;
    int f_tang;

    extern int out_mc;
    // extern int exp_set;
    // extern int fast_solve;
    // extern int dot_flag;
    extern int output_flag;
    /*CTL*/
    extern bool CTL;
    /*CTL*/

    /*AUTOMA*/
    // extern bool AUTOMA;
    // extern FILE *f_outState;
    // extern FILE *wngrR;
    /*AUTOMA*/

    /*MDD*/

    // extern char cache_string[MAX_CACHE];
    extern bool g_dot_RS;
    extern const char* g_dot_file;
    extern bool g_dot_open_RS;
    extern bool g_count_firings;
    extern bool g_save_ext_incidence;
    extern bool g_open_saved_ext_incidence;
    /*MDD*/

    int exceeded_markings_bound() { return FALSE; }
}
/*MDD*/

int g_max_seconds_statespace = 0;
int g_max_MB_statespace = -1;
bool g_statespace_runs_as_subprocess = false;

// Used here and in CTL, for Model Checking Context list of techniques.
const char* MCC_TECHNIQUES = "TECHNIQUES DECISION_DIAGRAMS PARALLEL_PROCESSING UNFOLDING_TO_PT USE_NUPN TOPOLOGICAL";

void handle_sigabrt_gmp_exit(int) {
    printf("Terminating the application...\n");
    if (running_for_MCC() && !is_child_subprocess())
        printf("CANNOT_COMPUTE\n");
    exit(EXIT_FAILURE_GMP);
}

// Handle a possible SIGABRT raised by GMP when computing the number of
// fired transitions during MCC.
void handle_sigabrt_gmp_when_counting_firings_exit(int) {
    printf("Not enough memory to compute transition firings...\n");
    if (running_for_MCC())
        printf("STATE_SPACE TRANSITIONS -1 %s\n", MCC_TECHNIQUES);
    exit(running_for_MCC() ? 0 : EXIT_FAILURE_GMP);
}

// Handle a timeout induced by an ALARM signal
void handle_sigalarm_try_next_varorder(int) {
    if (!running_for_MCC())
        printf("\n\n\nTimed out. Trying another variable order...\n");
    exit(EXIT_TIMEOUT_VARORDER);
}

extern ofstream fout;
int steps = 0, maxsteps = 0;

extern bool do_model_checking(RSRG *r);

// Open a file with the default application/browser
int open_file(const char * filename) {
#ifdef __APPLE__
    ostringstream cmd;
    cmd << "open \"" << filename << "\" > /dev/null 2>&1";
    return system(cmd.str().c_str());
#elif defined __linux__
    ostringstream cmd;
    cmd << "xdg-open \"" << filename << "\" > /dev/null 2>&1";
    return system(cmd.str().c_str());
#else
    // Windows should use the START command
    #warning "open_file() is not implemented.!"
    return 0;
#endif
}

//-----------------------------------------------------------------------------

// Verify that the found value corresponds to the expected value, with a
// margin of error due to approximations in the MCC format of the expected values
const char* 
regression_test_eps(const RSRG& rs, cardinality_t& found, const char* what, bool *set_failed) {
    std::string key = std::string("StateSpace_") + what;
    auto kv = rs.expected_results.find(key);
    if (kv == rs.expected_results.end()) {
        return ""; // No value to test
    }
    else if (std::holds_alternative<double>(kv->second)) {
        double expected = boost::get<double>(kv->second);
        cardinality_t diff = expected - found;
        if (std::abs(get_double(diff)) / expected > 0.0001) {
            cerr << "\n\nExpected "<<what<<" was " << expected <<", found "<<found<<endl;
            // throw rgmedd_exception("Regression test failed.");
            *set_failed = true;
            return "  [[FAILED]]";
        }
        return "  [[OK]]";
    }
    else return "  [[??]]";
}

//-----------------------------------------------------------------------------

const char*
regression_test(const RSRG& rs, ssize_t found, const char* what, bool *set_failed) {
    std::string key = std::string("StateSpace_") + what;
    auto kv = rs.expected_results.find(key);
    if (kv == rs.expected_results.end()) {
        return ""; // No value to test
    }
    else if (std::holds_alternative<ssize_t>(kv->second)) {
        ssize_t expected = boost::get<ssize_t>(kv->second);
        if (expected != found) {
            cerr << "\n\nExpected "<<what<<" was " << expected <<", found "<<found<<endl;
            // throw rgmedd_exception("Regression test failed.");
            *set_failed = true;
            return "  [[FAILED]]";
        }
        return "  [[OK]]";
    }
    else return "  [[??]]";
}

//-----------------------------------------------------------------------------

template<typename T>
std::string out_card(T& card) {
    if (card == INFINITE_CARD)
        return std::string("inf");
    if (card == UNKNOWN_CARD)
        return std::string("unknown");
    ostringstream oss;
    oss << card;
    return oss.str();   
}

/*AUTOMATON*/
// extern int initAutoma(RSRGAuto *rs);
// extern map <int, map <int, class ATRANSITION> > automa;
// extern int Top, Bot;
// extern set <int> InitSet;

/*AUTOMATON*/
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
bool build_graph(class RSRG &rs) {
    /* Init build_graph */    
    clock_t timeLRS, timeRS, timeStats, timeCTL, timeFirings;
    clock_t timeNSFAccum = 0, timeStateSpace = 0, timeStatsAccum = 0;
    bool failed_regression = false;

    if (g_max_seconds_statespace) {
        // Start the timer
        signal(SIGALRM, handle_sigalarm_try_next_varorder);
        alarm(g_max_seconds_statespace);
    }
    if (g_max_MB_statespace > 0)
        constraint_address_space(g_max_MB_statespace);

    // Intialize RS/LRS generation
    if (rs.init_RS(net_mark) != 0)
        throw rgmedd_exception("Error in method init_RS\n");


    // Check LRS/RS equivalence 
    bool equiv_LRS_RS = false;
    if (test_LRS_RS_equiv) {
        print_banner(" VERIFYING LRS/RS EQUIVALENCE ");
        if (rs.verify_LRS_RS_equiv()) {
            cout << "Assuming: LRS == RS" <<endl;
            if (rs.shouldBuildRS()) { // build LRS instead
                equiv_LRS_RS = true;
                rs.setBuildLRS();
            }
        }
        else {
            cout << "Assuming: LRS != RS" << endl;
        }
    }

    //=========================================================================
    // Build the LRS 
    // LRS is built first, before we have the real bounds
    cardinality_t lrs_card = UNKNOWN_CARD;
    if (rs.shouldBuildLRS()) {
        print_banner(" LRS GENERATION ");
        timeLRS = clock();
        rs.buildLRS();
        timeLRS = clock() - timeLRS;

        // Print LRS statistics
        // if (!running_for_MCC() && !CTL) {
        if (rs.has_LRS()) {
            signal(SIGABRT, handle_sigabrt_gmp_exit);
            apply(MEDDLY::CARDINALITY, rs.getLRS(), cardinality_ref(lrs_card));
            signal(SIGABRT, SIG_DFL);
        }
        else {
            lrs_card = INFINITE_CARD;
        }
        // }
    }

    // Control variables for the RS generation loop
    // bool has_RS = false;
    // bool rs_is_inf = true;
    int restartCount = 1;

    // If RS==LRS, then use the latter for the former
    if (equiv_LRS_RS && rs.has_LRS() && rs.shouldBuildRS()) {
        rs.setRS_asLRS();
        rs.updateGuessedBounds(0);
        // has_RS = true;
        // rs_is_inf = false;
    }
    

    //=========================================================================
    // Build the RS
    timeRS = clock();
    bool restart = (rs.shouldBuildRS() && !rs.has_RS());
    while (restart) {
        if (rs.buildNSFsBeforeRS()) {
            // NSF generation for the currently estimated variable bounds.
            if (!running_for_MCC())
                cout << endl;
            print_banner(" START FIRING RULES ENCODING ");

            clock_t nsfs_start = clock();
            // Generate all transition NSF, ordered by priority groups
            rs.initializeNSFsForKnownBounds();
            clock_t nsfsTime = clock() - nsfs_start;
            timeNSFAccum += nsfsTime;

            if (!running_for_MCC()) {
                cout << "Encoded " << ntr << " transition in " << (ngr+1) << " priority groups." << endl;
                cout << "Time to build all NSFs: " << (nsfsTime / double(CLOCKS_PER_SEC)) << endl;
                cout << endl;
            }
        }

        // int who1 = RUSAGE_SELF;
        // struct rusage usage1;
        // getrusage(who1, &usage1);
        // cout << "Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;

        print_banner(" REACHABILITY SET GENERATION ");
        clock_t startStateSpace = clock();
        // Generate the Reachability Set using the RS method
        rs.buildRS();
        // has_RS = true;
        timeStateSpace += clock() - startStateSpace;
        if (rs.is_RS_infinite())
            break;

        // Test if we need to restart the RS generation (guessed bounds are less than actual variable bounds)
        restart = rs.updateGuessedBounds(restartCount);
        if (restart) {
            restartCount++;
            if (!running_for_MCC())
                cout << "Restarting RS generation.\n" << endl;

            if (rs.useImplicitNSF() || rs.useOtfNSF())
                throw rgmedd_exception("Should not restart RS generation!");
            rs.clearNSFs();
        }
    } // RS build cycle
    if (!rs.is_RS_infinite() && output_flag) {
        std::string net = std::string(net_name) + "rg";
        ofstream out(net.c_str(), ifstream::out);
        if (!out) {
            cerr << "\nError opening the output file\n\n";
            exit(EXIT_FAILURE);
        }
        out << rs;
        out.close();
    }
    if (rs.has_RS() && !rs.is_RS_infinite() && !CTL) {
        cout << "verifying initial bounds..." << endl;
        rs.verifyGuessedBounds(); // extra checks on bounds
    }
    timeRS = clock() - timeRS;
    //const bool has_finite_RS = (!rs.is_RS_infinite() && rs.shouldBuildRS());
    // cout << "RS flags: " << rs.has_RS() << " " << rs.has_finite_RS() << " " << rs.is_RS_infinite() << endl;

    timeStats = clock();
    //=========================================================================
    // Compute RS cardinality.
    // GNU MP could run out of memory and call abort(). To avoid a core dump and
    // an abnormal termination, install the SIGABRT signal handler, that prints
    // a clean error message and terminate. Use the signal handler *only* for 
    // cardinality computation.
    cardinality_t rs_card = UNKNOWN_CARD;
    int max_place_bound = -1, token_sum_bound = -1, max_count_pl_levels = -1;
    if (rs.has_RS()) {
        if (rs.has_finite_RS()) {
            signal(SIGABRT, handle_sigabrt_gmp_exit);
            if (!running_for_MCC() || !CTL) { // do not waste time in MCC/CTL mode by computing the RS size
                apply(MEDDLY::CARDINALITY, rs.getRS(), cardinality_ref(rs_card));
            }
            signal(SIGABRT, SIG_DFL);

            // Compute max place bound
            max_place_bound = rs.computeTokenBoundForAnyPlace().second;

            // Compute max sum of tokens in every marking
            token_sum_bound = rs.get_token_sum_bound();

            // Compute maximum number of tokens in place levels (i.,e. unfolded places for COL models)
            // max_count_pl_levels will be different from max_place_bound for COL models.
            max_count_pl_levels = 0;
            for (int p = 0; p < npl; p++)
                max_count_pl_levels = std::max(max_count_pl_levels, rs.getMaxValueOfPlaceRS(p));
        }
        else {
            rs_card = INFINITE_CARD;
            max_place_bound = INFINITE_CARD;
            token_sum_bound = INFINITE_CARD;
            max_count_pl_levels = INFINITE_CARD;
        }
    }
    rs.buildRS_phaseEnd();

    // // Show the markings in LRS but not in RS
    // dd_edge LRS_diff_RS(rs.getForestMDD());
    // apply(DIFFERENCE, rs.getLRS(), rs.getRS(), LRS_diff_RS);
    // cout << "\n\nLRS \\ RS:" << endl;
    // rs.show_markings(cout, LRS_diff_RS, 1000);

    // // Show all dead markings in RS (requires the NSF with -sat-mono)
    // dd_edge dead_marks(rs.getForestMDD());
    // apply(PRE_IMAGE, rs.getRS(), rs.getNSF(), dead_marks);
    // apply(DIFFERENCE, rs.getRS(), dead_marks, dead_marks);
    // cout << "\n\nDEAD MARKINGS:" << endl;
    // rs.show_markings(cout, dead_marks, 1000);

    if (rs.has_LRS() && rs.has_RS()) {
        if (rs.getRS().getNode() == rs.getLRS().getNode())
            cout << "LREP Test:  RS==LRS" << endl;
        else
            cout << "LREP Test:  RS!=LRS" << endl;
    }


    // At this point, we can disable the timeout alarm and notify the parent process
    if (g_max_seconds_statespace) {
        alarm(0);
        signal(SIGALRM, SIG_DFL);
    }
    // notify parent that this process has finished and it is the
    // process that will continue running. This method ends all
    // other concurrent processes, and it avoids race conditions.
    if (g_statespace_runs_as_subprocess)
        notify_parent_and_continue();

    // restore previous memory limit
    if (g_max_MB_statespace > 0)
        constraint_address_space(-1);



    //=========================================================================
    // Print first set of statistics
    //=========================================================================
    if (!running_for_MCC()) {
        cout << endl;
        print_banner(" MEMORY ");
        if (rs.has_LRS()) {
            const char* rtest = regression_test_eps(rs, lrs_card, "LRS_STATES", &failed_regression);
            cout << " Cardinality(LRS):        " << left << setw(15) << out_card(lrs_card) << rtest << endl;
            cout << " LRS nodes:               " << rs.getLRS().getNodeCount() << endl;
            cout << " LRS edges:               " << rs.getLRS().getEdgeCount() << endl;
        }
        if (rs.has_RS()) {
            const char* rtest = regression_test_eps(rs, rs_card, "STATES", &failed_regression);
            cout << " Cardinality(RS):         " << left << setw(15) << out_card(rs_card) << rtest << endl;
            // cout << " Cardinality(RS):         " << left << setw(15) << get_double(rs_card) << rtest << endl;
        }
    }
    
    if (print_stat_for_gui() && rs.has_RS()) {
        const double CLKDIV = double(CLOCKS_PER_SEC);
        cout << "#{GUI}# RESULT STAT build_time " << (timeRS/CLKDIV) << " seconds." << endl;
        cout << "#{GUI}# RESULT STAT num_tangible_markings " << out_card(rs_card) << endl;
        cout << "#{GUI}# RESULT STAT num_rs_nodes " << rs.getRS().getNodeCount() << endl;
    }

    // First set of MCC results
    if (running_for_MCC() && !CTL && rs.has_RS()) { // MCC StateSpace examination
        cout << "STATE_SPACE STATES " << out_card(rs_card) << " " << MCC_TECHNIQUES << endl;
        cout << "STATE_SPACE MAX_TOKEN_PER_MARKING " << out_card(token_sum_bound) 
             << " " << MCC_TECHNIQUES << endl; 
             
        // cout << "STATE_SPACE MAX_TOKEN_IN_PLACE " 
        //      << out_card(max_place_bound) << " " << MCC_TECHNIQUES << endl;
        cout << "STATE_SPACE MAX_TOKEN_IN_PLACE " 
             << out_card(max_count_pl_levels) << " " << MCC_TECHNIQUES << endl;
    }
    timeStatsAccum += clock() - timeStats;


    //=========================================================================
    // Write the MDD of the RS in dot/pdf format using Graphviz
    if (rs.has_finite_RS() && g_dot_RS && rs.getRS().getNodeCount() < 300) {
        // std::string rsdd_name = net_name;
        // rsdd_name += "rsdd";
        // if (g_dot_file != nullptr)
        //     rsdd_name = g_dot_file;        
        // cout << "Writing PDF of the RS MDD "<<rsdd_name<<" ..." << endl;

        // try {
        //     rs.getRS().writePicture(rsdd_name.c_str(), "pdf"); // throws if dot is missing.
        //     if (g_dot_open_RS) 
        //         open_file((rsdd_name + ".pdf").c_str());
        //     if (invoked_from_gui())
        //         cout << "#{GUI}# RESULT DD" << endl;
        // }
        // catch (MEDDLY::error e) {
        //     cerr << "Could not generate the PDF of the MDD graph.\n"
        //             "Could not call Graphviz' dot command." << endl;
        // }

        std::string rsdd_name = net_name;
        if (g_dot_file != nullptr) {
            rsdd_name = g_dot_file;
            rsdd_name += ".";
        }
        std::string dot_name = rsdd_name, pdf_name = rsdd_name;
        dot_name += "dot";
        pdf_name += "pdf";
        cout << "Writing dot file "<<dot_name<<" ..." << endl;
        write_dd_as_dot(&rs, rs.getRS(), dot_name.c_str(), true, false, true, nullptr);
        ostringstream cmd1, cmd2;
        cout << "Generating PDF with Graphviz ..." << endl;
        cmd1 << "dot -Tpdf \""<<dot_name<<"\" > \""<<pdf_name<<"\"";
        if (0 == system(cmd1.str().c_str())) {
            if (g_dot_open_RS) 
                open_file(pdf_name.c_str());
            if (invoked_from_gui())
                cout << "#{GUI}# RESULT DD" << endl;
        }
    }

    // Write the MDD of the LRS in dot/pdf format using Graphviz
    if (rs.has_LRS() && g_dot_RS && rs.getLRS().getNodeCount() < 300) {
        std::string lrsdd_name = net_name;
        if (g_dot_file != nullptr) {
            lrsdd_name = g_dot_file;
            lrsdd_name += ".";
        }
        std::string dot_name = lrsdd_name, pdf_name = lrsdd_name;
        dot_name += "lrs.dot";
        pdf_name += "lrs.pdf";
        cout << "Writing dot file "<<dot_name<<" ..." << endl;
        write_dd_as_dot(&rs, rs.getLRS(), dot_name.c_str(), true, false, true, nullptr);
        ostringstream cmd1, cmd2;
        cout << "Generating PDF with Graphviz ..." << endl;
        cmd1 << "dot -Tpdf \""<<dot_name<<"\" > \""<<pdf_name<<"\"";
        if (0 == system(cmd1.str().c_str())) {
            if (g_dot_open_RS) 
                open_file(pdf_name.c_str());
            // if (invoked_from_gui())
            //     cout << "#{GUI}# RESULT DD" << endl;
        }
    }

    //=========================================================================
    // Compute firings (total number of fired transitions in the RG)
    timeFirings = clock();
    cardinality_t rg_edges = UNKNOWN_CARD;
    bool has_firings = false;

    if (!CTL && g_count_firings && rs.has_RS()) {
        if (rs.is_RS_infinite()) 
            rg_edges = INFINITE_CARD;
        else { // RS is finite
            clock_t firingCountTime = clock();
            signal(SIGABRT, handle_sigabrt_gmp_when_counting_firings_exit);
            try {
                rg_edges = rs.count_num_fired_transitions_by_events_shared();
                has_firings = true;
            }
            catch (std::bad_alloc e) {
                cout << "Not enough memory to count transition firings..." << endl;
            }
            signal(SIGABRT, SIG_DFL);
            firingCountTime = clock() - firingCountTime;
        }
    }
    timeFirings = clock() - timeFirings;
    //=========================================================================


    //=========================================================================
    // Print the second set of statistics
    //=========================================================================
    timeStats = clock();
    if (running_for_MCC() && !CTL && rs.has_RS()) {
        cout << "STATE_SPACE TRANSITIONS " << out_card(rg_edges) << " " << MCC_TECHNIQUES << endl;
        cout << endl;
    }
    if (print_stat_for_gui() && rs.has_RS()) {
        cout << "#{GUI}# RESULT STAT total_firings " << out_card(rg_edges) << endl;
    }
    if (!running_for_MCC()) {
        // Basic statistics (both statespace/model-checking modes)
        if (rg_edges != UNKNOWN_CARD) {
            cout << " # fired transitions:     " << left << setw(15) << out_card(rg_edges)
                    << regression_test_eps(rs, rg_edges, "TRANSITIONS", &failed_regression) << endl;
            // cout << " # fired transitions:     " << left << setw(15) << get_double(rg_edges) << endl;
        }
        const char *rtest_max_place_bound = "", *rtest_token_sum_bound = "";
        if (rs.has_RS()) {
            rtest_max_place_bound = regression_test(rs, max_count_pl_levels, "MAX_TOKEN_IN_PLACE", &failed_regression);
            rtest_token_sum_bound = regression_test(rs, token_sum_bound, "MAX_TOKEN_PER_MARKING", &failed_regression);
            cout << " Max tokens x marking:    " << left << setw(15) << out_card(token_sum_bound) 
                    << rtest_token_sum_bound << endl;
            cout << " Max tokens in place:     " << left << setw(15) << out_card(max_place_bound)
                    << rtest_max_place_bound << endl;
            cout << " Max tokens in levels:    " << left << setw(15) << out_card(max_count_pl_levels)
                    << endl;
        }
        if (!CTL) {
            // Prepare MDD/MxD/forests statistics (only in statespace mode)
            cout << " RS nodes:                " << rs.getRS().getNodeCount() << endl;
            cout << " RS edges:                " << rs.getRS().getEdgeCount() << endl;
            // rs.getRS().getForest()->garbageCollect();
            forest* forestMxD = rs.getForestMxD();
            // forestMxD->garbageCollect();
            // Print forest statistics            
            cout << " Forest(RS) nodes:        " << rs.getRS().getForest()->getCurrentNumNodes() << " actives, "
                 << rs.getRS().getForest()->getPeakNumNodes() << " peak, "
                 << rs.getRS().getForest()->getStats().num_compactions << " compactions." << endl;
            cout << " Forest(RS) size:         " << rs.getRS().getForest()->getCurrentMemoryUsed() << " Bytes now, "
                 << rs.getRS().getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

            if(rs.useMonolithicNSF()){ // TODO anche per non monolitico
                cout << " Monolithic NSF nodes:    " << rs.getNSF().getNodeCount() << endl;
                // cardinality_t mxd_card;
                // apply(CARDINALITY, rs.getNSF(), cardinality_ref(mxd_card));
                // cout << " Monolithic NSF card:     " << mxd_card << endl;
            }
            cout << " MxD nodes:               " << forestMxD->getCurrentNumNodes() << " actives, "
                 << forestMxD->getPeakNumNodes() << " peak, "
                 << forestMxD->getStats().num_compactions << " compactions." << endl;
            cout << " MxD size:                " << forestMxD->getCurrentMemoryUsed() << " Bytes now, "
                 << forestMxD->getPeakMemoryUsed() << " Bytes peak." << endl;                 
        }
        int who = RUSAGE_SELF;
        struct rusage usage;
        getrusage(who, &usage);
        cout << " Total Memory Used:       " << usage.ru_maxrss << " KBytes." << endl;
        // print_banner("");        
    }
    
    if (!running_for_MCC() && out_mc && rs.has_RS() && !rs.is_RS_infinite()) {
        cout << endl;
        print_banner(" START CTMC SOLUTION ");
        cout << " Start Index RS  generation" << endl;
        rs.IndexRS();
        cout << " End Index RS  generation" << endl;
        if (rs.JacobiSolver() != 0)
            throw rgmedd_exception("Error in method JacobiSolver\n");
    }
    timeStatsAccum += clock() - timeStats;


    // MEDDLY::ostream_output stdout_wrap(cout);
    // rs.getRS().show(stdout_wrap, 2);

    if (g_save_ext_incidence) {
        rs.showExtendedIncidenceMatrix(g_open_saved_ext_incidence);
    }


    //=========================================================================
    // Evaluate CTL formulas
    //=========================================================================
    if (CTL) {
        // if (!rs.has_RS()) {
        //     cout << "RS is not built. Could not evaluate formulas." << endl;
        // }
        // else if (!rs.is_RS_infinite()) {
        if (!running_for_MCC()) {
            cout << endl;
            print_banner(" MODEL CHECKING ");
        }
        timeCTL = clock();
        if (do_model_checking(&rs))
            failed_regression = true;
        timeCTL = clock() - timeCTL;
        if (print_stat_for_gui()) {
            const double CLKDIV = double(CLOCKS_PER_SEC);
            cout << "#{GUI}# RESULT STAT CTL_time " << (timeCTL/CLKDIV) << " seconds." << endl;
        }
        // }
        // else {
        //     if (!running_for_MCC())
        //         cout << "State space is infinite. Could not evaluate formulas." << endl;
        //     else
        //         cout << "CANNOT_COMPUTE" << endl;
        // }
    }


#if QUASIPRODFORM
    int **to = (int **) malloc((1) * sizeof(int *));
    to[0] = (int *) malloc((npl + 1) * sizeof(int));

    memset(to[0], -1, (npl + 1)*sizeof(int));

    to[0][1] = 4;

//create the state index
    rs.IndexRS();

    class QUASIPRODFOR qpf(rs.getRS(), rs.getIndexrs(), rs.getDomain());
//perform the intersection
    qpf.getState(to);
    memset(to[0], -1, (npl + 1)*sizeof(int));

//initialize the iterator
    cout << "Number of markings: " << qpf.initIterator() << endl;
    int id = -1;
//return true  if the value of to is valid otherwise false.
    while (qpf.getMarking(to[0], id)) {
        cout << "\tIndex of Marking < ";
        for (int i = 1; i < rs.getDomain()->getNumVariables(); i++)
            cout << to[0][i] << " ";
        cout << "> : " << id << endl;

    }

#endif


    //=========================================================================
    // Final time statistics
    //=========================================================================
    clock_t timeTotal = clock() - rs.init_time;
    if (!running_for_MCC()) {
        const double CLKDIV = double(CLOCKS_PER_SEC);
        cout << endl;
        print_banner(" TIME ");
        cout << " Variable order Time:   " << setprecision(3) << setw(7) << fixed 
             << (rs.varorder_time/CLKDIV) << " sec." << endl;
        if (timeNSFAccum > 0) {
            cout << " NSF gen. Time:         " << setprecision(3) << setw(7) << fixed 
                 << (timeNSFAccum/CLKDIV) << " sec." << endl;
        }
        if (rs.has_LRS()) {
            cout << " LRS Generation Time:   " << setprecision(3) << setw(7) << fixed 
                 << (timeLRS/CLKDIV) << " sec." << endl;
        }
        if (rs.has_RS()) {
            cout << " RS Generation Time:    " << setprecision(3) << setw(7) << fixed 
                 << (timeStateSpace/CLKDIV) << " sec." << endl;
            cout << " Total RS Time:         " << setprecision(3) << setw(7) << fixed 
                 << (timeRS/CLKDIV) << " sec." << endl;
        }
        if (!CTL && has_firings) {
            cout << " Firings Count Time:    " << setprecision(3) << setw(7) << fixed 
                 << (timeFirings/CLKDIV) << " sec." << endl;
        }
        cout << " Statistics Time:       " << setprecision(3) << setw(7) << fixed 
             << (timeStatsAccum/CLKDIV) << " sec." << endl;
        if (CTL)
            cout << " Model Checking Time:   " << setprecision(3) << setw(7) << fixed 
                 << (timeCTL/CLKDIV) << " sec." << endl;
        cout << " Total Time:            " << setprecision(3) << setw(7) << fixed 
             << (timeTotal/CLKDIV) << " sec." << endl;
        cout.unsetf(std::ios_base::floatfield);

        print_banner("");
    }

    return failed_regression;
}/* End build_graph */




















