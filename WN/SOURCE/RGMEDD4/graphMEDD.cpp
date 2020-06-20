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

#include "rgmedd4.h"
#include "parallel.h"

extern bool CTL_as_CTLstar;

extern "C" {


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

/*MDD*/
extern void CTLParser(RSRG *r);

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

// Verify that the found value corresponds to the expected value, with a
// margin of error due to approximations in the MCC format of the expected values
const char* 
regression_test_eps(double expected, cardinality_t& found, const char* what) {
    if (expected < 0 && expected != INFINITE_CARD)
        return ""; // No value to test
    double f = get_double(found);
    if (std::abs(expected - f) / expected > 0.0001) {
        cerr << "\n\nExpected "<<what<<" was " << expected <<", found "<<found<<endl;
        throw rgmedd_exception("Regression test failed.");
    }
    return "  [[OK]]";
}
const char*
regression_test(ssize_t expected, ssize_t found, const char* what) {
    if (expected < 0 && expected != INFINITE_CARD)
        return ""; // No value to test
    if (expected != found) {
        cerr << "\n\nExpected "<<what<<" was " << expected <<", found "<<found<<endl;
        throw rgmedd_exception("Regression test failed.");
    }
    return "  [[OK]]";
}

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
void build_graph(class RSRG &rs) {
    /* Init build_graph */    
    clock_t timeLRS, timeRS, timeStats, timeCTL, timeFirings;
    clock_t timeNSFAccum = 0, timeStateSpace = 0, timeStatsAccum = 0;

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

    // Build the LRS first, before we have the real bounds
    bool has_LRS = false;
    cardinality_t lrs_card = UNKNOWN_CARD;
    if (rs.shouldBuildLRS()) {
        print_banner(" LRS GENERATION ");
        timeLRS = clock();
        has_LRS = rs.buildLRS();
        timeLRS = clock() - timeLRS;

        // Print LRS statistics
        if (!running_for_MCC() && !CTL) {
            if (has_LRS) {
                signal(SIGABRT, handle_sigabrt_gmp_exit);
                apply(MEDDLY::CARDINALITY, rs.getLRS(), cardinality_ref(lrs_card));
                signal(SIGABRT, SIG_DFL);
            }
            else {
                lrs_card = INFINITE_CARD;
            }
        }
    }
    
    // Build the RS
    const bool has_RS = rs.shouldBuildRS();
    bool restart = has_RS;
    bool rs_is_inf = true;
    int restartCount = 1;
    timeRS = clock();
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
        rs_is_inf = rs.buildRS();
        timeStateSpace += clock() - startStateSpace;
        if (rs_is_inf)
            break;

        // Test if we need to restart the RS generation (guessed bounds are less than actual variable bounds)
        restart = rs.updateGuessedBounds(restartCount);
        if (restart) {
            restartCount++;
            if (!running_for_MCC())
                cout << "Restarting RS generation.\n" << endl;

            if (rs.useImplicitNSF() || rs.useOtfNSF())
                throw rgmedd_exception("Should not restart RS generation!");
        }
    } // RS build cycle
    if (!rs_is_inf && output_flag) {
        std::string net = std::string(net_name) + "rg";
        ofstream out(net.c_str(), ifstream::out);
        if (!out) {
            cerr << "\nError opening the output file\n\n";
            exit(EXIT_FAILURE);
        }
        out << rs;
        out.close();
    }
    timeRS = clock() - timeRS;
    const bool has_finite_RS = (!rs_is_inf && rs.shouldBuildRS());

    timeStats = clock();
    //=========================================================================
    // Compute RS cardinality.
    // GNU MP could run out of memory and call abort(). To avoid a core dump and
    // an abnormal termination, install the SIGABRT signal handler, that prints
    // a clean error message and terminate. Use the signal handler *only* for 
    // cardinality computation.
    cardinality_t rs_card = UNKNOWN_CARD;
    int max_place_bound = -1, token_sum_bound = -1, max_count_pl_levels = -1;
    if (has_RS) {
        if (has_finite_RS) {
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
        if (has_LRS && !CTL) {
            const char* rtest = regression_test_eps(rs.expected_lrs_card, lrs_card, "LRSCARD");
            cout << " Cardinality(LRS):        " << left << setw(15) << out_card(lrs_card) << rtest << endl;
            cout << " LRS nodes:               " << rs.getLRS().getNodeCount() << endl;
            cout << " LRS edges:               " << rs.getLRS().getEdgeCount() << endl;
        }
        if (has_RS && !CTL) {
            const char* rtest = regression_test_eps(rs.expected_rs_card, rs_card, "RSCARD");
            cout << " Cardinality(RS):         " << left << setw(15) << out_card(rs_card) << rtest << endl;
        }
    }
    
    if (print_stat_for_gui() && has_RS) {
        const double CLKDIV = double(CLOCKS_PER_SEC);
        cout << "#{GUI}# RESULT STAT build_time " << (timeRS/CLKDIV) << " seconds." << endl;
        cout << "#{GUI}# RESULT STAT num_tangible_markings " << out_card(rs_card) << endl;
        cout << "#{GUI}# RESULT STAT num_rs_nodes " << rs.getRS().getNodeCount() << endl;
    }

    // First set of MCC results
    if (running_for_MCC() && !CTL && has_RS) { // MCC StateSpace examination
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
    if (has_finite_RS && g_dot_RS && rs.getRS().getNodeCount() < 200) {
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
        write_dd_as_dot(&rs, rs.getRS(), dot_name.c_str(), true);
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

    //=========================================================================
    // Compute the total number of fired transitions needed to build the RS
    timeFirings = clock();
    cardinality_t rg_edges = UNKNOWN_CARD;
    bool has_firings = false;

    if (!CTL && g_count_firings && has_RS) {
        if (rs_is_inf) 
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
    if (running_for_MCC() && !CTL && has_RS) {
        cout << "STATE_SPACE TRANSITIONS " << out_card(rg_edges) << " " << MCC_TECHNIQUES << endl;
        cout << endl;
    }
    if (print_stat_for_gui() && has_RS) {
        cout << "#{GUI}# RESULT STAT total_firings " << out_card(rg_edges) << endl;
    }
    if (!running_for_MCC()) {
        if (!CTL) {
            if (rg_edges != UNKNOWN_CARD) {
                cout << " # fired transitions:     " << left << setw(15) << out_card(rg_edges)
                     << regression_test_eps(rs.expected_rg_edges, rg_edges, "RGEDGES") << endl;
            }
            const char *rtest_max_place_bound = "", *rtest_token_sum_bound = "";
            if (has_RS) {
                rtest_max_place_bound = regression_test(rs.expected_max_tokens_place, max_place_bound, "MAXPLCBOUND");
                rtest_token_sum_bound = regression_test(rs.expected_max_tokens_marking, token_sum_bound, "MAXTOKMARK");
                cout << " Max tokens x marking:    " << left << setw(15) << out_card(token_sum_bound) 
                     << rtest_token_sum_bound << endl;
                cout << " Max tokens in place:     " << left << setw(15) << out_card(max_place_bound)
                     << rtest_max_place_bound << endl;
                cout << " Max tokens in levels:    " << left << setw(15) << out_card(max_count_pl_levels)
                     << endl;
                cout << " RS nodes:                " << rs.getRS().getNodeCount() << endl;
                cout << " RS edges:                " << rs.getRS().getEdgeCount() << endl;
            }
            // Prepare forest statistics
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
    
    if (!running_for_MCC() && out_mc && has_RS && !rs_is_inf) {
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
        if (!has_RS) {
            cout << "RS is not built. Could not run CTL." << endl;
        }
        else if (!rs_is_inf) {
            clock_t nsf_ctl_start = clock();

            // prepare monolithic NSF (LTL/CTL*)
            if(rs.useMonolithicNSF() || CTL_as_CTLstar) rs.prepareNSFforCTL();
            // use event-based NSF (CTL)
            else rs.prepareEventMxDsForCTL();

            timeNSFAccum += clock() - nsf_ctl_start;

            if (!running_for_MCC()) {
                cout << endl;
                print_banner(" CTL EVALUATION ");
            }
            timeCTL = clock();
            CTLParser(&rs);
            timeCTL = clock() - timeCTL;
            if (print_stat_for_gui()) {
                const double CLKDIV = double(CLOCKS_PER_SEC);
                cout << "#{GUI}# RESULT STAT CTL_time " << (timeCTL/CLKDIV) << " seconds." << endl;
            }
        }
        else {
            if (!running_for_MCC())
                cout << "State space is infinite. Could not run CTL." << endl;
            else
                cout << "CANNOT_COMPUTE" << endl;
        }
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
        cout << " Variable order Time: " << setprecision(3) << setw(9) << fixed 
             << (rs.varorder_time/CLKDIV) << " sec." << endl;
        if (timeNSFAccum > 0) {
            cout << " NSF gen. Time:       " << setprecision(3) << setw(9) << fixed 
                 << (timeNSFAccum/CLKDIV) << " sec." << endl;
        }
        if (has_LRS) {
            cout << " LRS Generation Time: " << setprecision(3) << setw(9) << fixed 
                 << (timeLRS/CLKDIV) << " sec." << endl;
        }
        if (has_RS) {
            cout << " RS Generation Time:  " << setprecision(3) << setw(9) << fixed 
                 << (timeStateSpace/CLKDIV) << " sec." << endl;
            cout << " Total RS Time:       " << setprecision(3) << setw(9) << fixed 
                 << (timeRS/CLKDIV) << " sec." << endl;
        }
        if (!CTL && has_firings) {
            cout << " Firings Count Time:  " << setprecision(3) << setw(9) << fixed 
                 << (timeFirings/CLKDIV) << " sec." << endl;
        }
        cout << " Statistics Time:     " << setprecision(3) << setw(9) << fixed 
             << (timeStatsAccum/CLKDIV) << " sec." << endl;
        if (CTL)
            cout << " CTL Time:            " << setprecision(3) << setw(9) << fixed 
                 << (timeCTL/CLKDIV) << " sec." << endl;
        cout << " Total Time:          " << setprecision(3) << setw(9) << fixed 
             << (timeTotal/CLKDIV) << " sec." << endl;
        cout.unsetf(std::ios_base::floatfield);

        print_banner("");
    }


}/* End build_graph */




















