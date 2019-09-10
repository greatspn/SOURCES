#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <set>
#include <signal.h>

// #ifndef __PRS_H__
// #define __PRS_H__
// #include "../AUTOMA/parser.h"
// #endif

#include "rgmedd2.h"

extern "C" {

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
    extern int exp_set;
    extern int fast_solve;
    extern int dot_flag;
    extern int output_flag;
    /*CTL*/
    extern bool CTL;
    /*CTL*/

    /*AUTOMA*/
    extern bool AUTOMA;
    extern FILE *f_outState;
    extern FILE *wngrR;
    /*AUTOMA*/

    /*MDD*/

    extern char cache_string[MAX_CACHE];
    /*MDD*/

    int exceeded_markings_bound() { return FALSE; }
}
/*MDD*/

// Used here and in CTL, for Model Checking Context list of techniques.
const char* MCC_TECHNIQUES = "TECHNIQUES DECISION_DIAGRAMS SEQUENTIAL_PROCESSING UNFOLDING_TO_PT USE_NUPN TOPOLOGICAL";

void handle_sigabrt_gmp_exit(int) {
    printf("Terminating the application...\n");
    if (running_for_MCC())
        printf("CANNOT_COMPUTE\n");
    exit(EXIT_FAILURE_GMP);
}

// Handle a possible SIGABRT raised bu GMP when computing the number of
// fired transitions during MCC.
void handle_sigabrt_gmp_when_counting_firings_exit(int) {
    printf("Not enough memory to compute transition firings...\n");
    if (running_for_MCC())
        printf("STATE_SPACE TRANSITIONS -1 %s\n", MCC_TECHNIQUES);
    exit(running_for_MCC() ? 0 : EXIT_FAILURE_GMP);
}



extern ofstream fout;
int steps = 0, maxsteps = 0;

/*MDD*/
extern void CTLParser(RSRG *r);

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


    Node_p l_ptr = NULL;
    int pp;
    
    clock_t timeTotal, timeRS, timeStats, timeCTL, timeFirings;
    clock_t timeNSFAccum = 0, timeSaturation = 0, timeStatsAccum = 0;
    timeTotal = timeRS = clock();
    
    if (rs.init_RS(net_mark) != 0) {
        throw rgmedd_exception("Error in method init_RS\n");
    }

    // Temporary memory for the incidence matrix row of events
    // std::vector<int> F(npl); // from
    // std::vector<int> T(npl); // to
    // std::vector<int> H(npl); // inhibitor
    
    bool incompleteRSGeneration;
    int restartCount = 1;
    do {
        rs.resetNSFCollection();

        if (!running_for_MCC())
            cout << endl;
        print_banner(" START FIRING RULES ENCODING ");

        // Generate all transition NSF, ordered by priority groups
        clock_t nsfs_start = clock();

        // for (int curPrio = ngr, encodedCount = 0; curPrio>=0; curPrio--){
        //     for (int tt = 0; tt < ntr; tt++) {
        //         if(tabt[tt].pri != curPrio)
        //             continue;
        //         if (!running_for_MCC() && rs.useMonolithicNSF()) {
        //             cout << "\tEncoding transition " << tabt[tt].trans_name
        //                  << " (" << (++encodedCount) << "/" << ntr << ")";
        //             if (tabt[tt].pri > 0)
        //                 cout << ", priority group = " << tabt[tt].pri;
        //             cout << endl;
        //         }
        //         for (int ii = 0; ii < npl; ii++) {
        //             H[ii] = T[ii] = F[ii] = 0;
        //         }
        //         l_ptr = GET_INPUT_LIST(tt);
        //         while (l_ptr != NULL) {
        //             pp = GET_PLACE_INDEX(l_ptr);
        //             F[pp] = l_ptr->molt;
        //             l_ptr = NEXT_NODE(l_ptr);
        //         }
        //         l_ptr = GET_INHIBITOR_LIST(tt);
        //         while (l_ptr != NULL) {
        //             pp = GET_PLACE_INDEX(l_ptr);
        //             H[pp] = l_ptr->molt;
        //             l_ptr = NEXT_NODE(l_ptr);
        //         }
        //         l_ptr = GET_OUTPUT_LIST(tt);
        //         while (l_ptr != NULL) {
        //             pp = GET_PLACE_INDEX(l_ptr);
        //             T[pp] = l_ptr->molt;
        //             l_ptr = NEXT_NODE(l_ptr);
        //         }   

        //         if (rs.MakeNextState(F, T, H, tt) != 0) {
        //             throw rgmedd_exception("Error in method MakeNextState\n");
        //         }
        //     }
        // }
        rs.MakeAllNsfs();
        clock_t nsfsTime = clock() - nsfs_start;
        timeNSFAccum += nsfsTime;

        if (!running_for_MCC()) {
            cout << "Encoded " << ntr << " transition in " << (ngr+1) << " priority groups." << endl;
            cout << "Time to build all NSFs: " << (nsfsTime / double(CLOCKS_PER_SEC)) << endl;
            if(rs.useByEventsNSF()) {
                cout << "Finalizing SAT-pregen operator..." << endl;
                rs.getNSFCollection()->finalize();
            }
            cout << endl;
        }

        // int who1 = RUSAGE_SELF;
        // struct rusage usage1;
        // getrusage(who1, &usage1);
        // cout << "Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;

        print_banner(" REACHABILITY SET GENERATION ");
        clock_t startSaturation = clock();
        rs.genRSTimed();
        timeSaturation += clock() - startSaturation;
        
        // Test if we need a restart of the RS generation (guessed bounds are less than real bounds)
        incompleteRSGeneration = rs.updateGuessedBounds(restartCount);
        if(incompleteRSGeneration) {
            restartCount++;
            if (!running_for_MCC())
                cerr << "Restarting RS generation.\n" << endl;
        }
    } 
    while(incompleteRSGeneration);
    
    if (output_flag) {
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


    timeStats = clock();
    //=========================================================================
    // Compute RS cardinality.
    // GNU MP could run out of memory and call abort(). To avoid a core dump and
    // an abnormal termination, install the SIGABRT signal handler, that prints
    // a clean error message and terminate. Use the signal handler *only* for 
    // cardinality computation.
    cardinality_t rs_card = 0;
    signal(SIGABRT, handle_sigabrt_gmp_exit);
    if (!running_for_MCC() || !CTL) // do not waste time in MCC/CTL mode by computing the RS size
        rs.getSizeRS(rs_card);
    signal(SIGABRT, SIG_DFL);

    // Compute max place bound & max sum of tokens in every marking
    int max_place_bound = 0;
    for (int p = 0; p < npl; p++)
        max_place_bound = std::max(max_place_bound, rs.getMaxValueOfPlaceRS(p));
    //=========================================================================


    //=========================================================================
    // Print first set of statistics
    //=========================================================================

    if (!running_for_MCC()) {
        cout << endl;
        print_banner(" MEMORY ");
        if (!CTL) {
            cout << " Cardinality(RS):         " << rs_card << endl;
        }
    }
    
    if (print_stat_for_gui()) {
        const double CLKDIV = double(CLOCKS_PER_SEC);
        cout << "#{GUI}# RESULT STAT build_time " << (timeRS/CLKDIV) << " seconds." << endl;
        cout << "#{GUI}# RESULT STAT num_tangible_markings " << rs_card << endl;
        cout << "#{GUI}# RESULT STAT num_rs_nodes " << rs.getRS().getNodeCount() << endl;
    }

    // First set of MCC results
    if (running_for_MCC() && !CTL) { // MCC StateSpace examination
        cout << "STATE_SPACE STATES " << rs_card << " " << MCC_TECHNIQUES << endl;
        cout << "STATE_SPACE MAX_TOKEN_PER_MARKING " << rs.get_token_sum_bound() 
             << " " << MCC_TECHNIQUES << endl;
        cout << "STATE_SPACE MAX_TOKEN_IN_PLACE " 
             << max_place_bound << " " << MCC_TECHNIQUES << endl;
    }
    timeStatsAccum += clock() - timeStats;

    // cout << "----------------------------------------" << endl;
    // cout << "End RS generation" << endl;
    // cout << "----------------------------------------" << endl << endl;


    //=========================================================================
    // Compute the total number of fired transitions needed to build the RS
    timeFirings = clock();
    cardinality_t rg_edges = 0;
    // {
    //     cardinality_t enab_tt;
    //     clock_t firingCountTime = clock();
    //     for(int tr=0; tr<ntr; tr++){
    //         dd_edge rs_tr(rs.getRS().getForest());
    //         apply(POST_IMAGE, *rs.getRS(), rs.getEventMxD(tr), rs_tr);
    //         apply(CARDINALITY, rs_tr, cardinality_ref(enab_tt));
    //         rg_edges += enab_tt;
    //     }
    //     firingCountTime = clock() - firingCountTime;
    //     if (!running_for_MCC())
    //         cout << "Time to compute # of fired transitions: " << firingCountTime / double(CLOCKS_PER_SEC) << endl;
    // }
    // cout << rg_edges << endl;

    // Count transition firings (by events)
    if (!CTL) {
        rg_edges = -1;
        clock_t firingCountTime = clock();
        signal(SIGABRT, handle_sigabrt_gmp_when_counting_firings_exit);
        try {
            rg_edges = rs.count_num_fired_transitions_by_events();
        }
        catch (std::bad_alloc e) {
            cout << "Not enough memory to count transition firings..." << endl;
        }
        signal(SIGABRT, SIG_DFL);
        // cout << rg_edges << endl;
        //rg_edges = rs.count_num_fired_transitions();
        firingCountTime = clock() - firingCountTime;
    }
    timeFirings = clock() - timeFirings;
    // cout << rg_edges << endl;
    //=========================================================================


    //=========================================================================
    // Print the second set of statistics
    //=========================================================================
    timeStats = clock();
    if (running_for_MCC() && !CTL) {
        cout << "STATE_SPACE TRANSITIONS " << rg_edges << " " << MCC_TECHNIQUES << endl;
        cout << endl;
    }
    if (print_stat_for_gui()) {
        cout << "#{GUI}# RESULT STAT total_firings " << rg_edges << endl;
    }
    if (!running_for_MCC()) {
        if (!CTL) {
            // rs.getRS().getForest()->garbageCollect();
            forest* forestMxD = rs.getForestMxD();
            // if(rs.useMonolithicNSF())
            //     outputForest = rs.getNSF().getForest();
            // else
            //     outputForest = rs.getNSFCollection()->getOutForest();
                
            // forestMxD->garbageCollect();

            if (rg_edges >= 0)
                cout << " # fired transitions:     " << rg_edges << endl;
            cout << " Max tokens x marking:    " << rs.get_token_sum_bound() << endl;
            cout << " Max tokens in place:     " << max_place_bound << endl;
            cout << " RS nodes:                " << rs.getRS().getNodeCount() << endl;
            cout << " Forest(RS) nodes:        " << rs.getRS().getForest()->getCurrentNumNodes() << " actives, "
                 << rs.getRS().getForest()->getPeakNumNodes() << " peak, "
                 << rs.getRS().getForest()->getStats().num_compactions << " compactions." << endl;
            cout << " Forest(RS) size:         " << rs.getRS().getForest()->getCurrentMemoryUsed() << " Bytes now, "
                 << rs.getRS().getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

            if(rs.useMonolithicNSF()){ // TODO anche per non monolitico
                cout << " Monolithic NSF nodes:    " << rs.getNSF().getNodeCount() << endl;
                cardinality_t mxd_card;
                apply(CARDINALITY, rs.getNSF(), cardinality_ref(mxd_card));
                cout << " Monolithic NSF card:     " << mxd_card << endl;
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
    
    if (!running_for_MCC() && out_mc) {
        cout << endl;
        print_banner(" START CTMC SOLUTION ");
        cout << " Start Index RS  generation" << endl;
        rs.IndexRS();
        cout << " End Index RS  generation" << endl;
        if (rs.JacobiSolver() != 0)
            throw rgmedd_exception("Error in method JacobiSolver\n");
    }
    timeStatsAccum += clock() - timeStats;


    //=========================================================================
    // Evaluate CTL formulas
    //=========================================================================
    if (CTL) {
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
    timeTotal = clock() - timeTotal;
    if (!running_for_MCC()) {
        const double CLKDIV = double(CLOCKS_PER_SEC);
        cout << endl;
        print_banner(" TIME ");
        cout << " NSF gen. Time:       " << setprecision(3) << setw(9) << fixed 
             << (timeNSFAccum/CLKDIV) << " sec." << endl;
        cout << " Saturation Time:     " << setprecision(3) << setw(9) << fixed 
             << (timeSaturation/CLKDIV) << " sec." << endl;
        cout << " Total RS Time:       " << setprecision(3) << setw(9) << fixed 
             << (timeRS/CLKDIV) << " sec." << endl;
        if (!CTL)
            cout << " Firings Count Time:  " << setprecision(3) << setw(9) << fixed 
                 << (timeFirings/CLKDIV) << " sec." << endl;
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

// #if DEBUG
//     cout << "\n*********************************************\n";
//     cout << "Rechability Set: \n" << rs;
//     cout << "*********************************************\n";
// #endif

 

}/* End build_graph */


// void computeFTH(int *F, int *T, int *H, int tt) {
//     Node_p l_ptr = NULL;
//     int pp;

//     for (int ii = 0; ii < npl; ii++) {
//         H[ii] = T[ii] = F[ii] = 0;
//     }
//     l_ptr = GET_INPUT_LIST(tt);
//     while (l_ptr != NULL) {
//         pp = GET_PLACE_INDEX(l_ptr);
//         F[pp] = l_ptr->molt;
//         l_ptr = NEXT_NODE(l_ptr);
//     }
//     l_ptr = GET_INHIBITOR_LIST(tt);
//     while (l_ptr != NULL) {
//         pp = GET_PLACE_INDEX(l_ptr);
//         H[pp] = l_ptr->molt;
//         l_ptr = NEXT_NODE(l_ptr);
//     }
//     l_ptr = GET_OUTPUT_LIST(tt);
//     while (l_ptr != NULL) {
//         pp = GET_PLACE_INDEX(l_ptr);
//         T[pp] = l_ptr->molt;
//         l_ptr = NEXT_NODE(l_ptr);
//     }
// }

/*AUTOMATON*/
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
// void build_graphAutoma(class RSRGAuto &rs) {
//     /* Init build_graph */


//     int *F = (int *) malloc((npl) * sizeof(int));
//     int *T = (int *) malloc((npl) * sizeof(int));
//     int *H = (int *) malloc((npl) * sizeof(int));
//     clock_t startGlobal, endGlobal, startRS, endRS, startRSA, endRSA, startInd, endInd;
//     double timeGlobal, timeRS, timeRSA, timeInd;
//     startGlobal = clock();



//     if (rs.init_RS(net_mark) != 0) {
//         rgmedd_exception obj("Error in method init_RS\n");
//         throw (obj);
//     }
//     initAutoma(&rs);





//     int kk = 0;
//     bool priority = false;

//     while ((kk < ntr) && (!priority)) {
//         if (tabt[kk].pri != 0) {
//             priority = true;
//         }
//         kk++;
//     }



//     for (int tt = 0; tt < ntr; tt++) {
//         computeFTH(F, T, H, tt);
//         if (rs.MakeNextState(F, T, H, tt, 0) != 0) {
//             rgmedd_exception obj("Error in method MakeNextState\n");
//             throw (obj);
//         }

//     }

//     cout << "\n====================== RS ======================" << endl;
//     startRS = clock();
//     if (priority) {
//         if (rs.genRSAll() != 0) {
//             rgmedd_exception obj("Error in method genRSALL\n");
//             throw (obj);
//         }
//     }
//     else {
//         if (rs.genRSTimed() != 0) {
//             rgmedd_exception obj("Error in method genRSTimed\n");
//             throw (obj);
//         }
//     }
//     endRS = clock();

//     long sizeRS = rs.getSizeRS();
//     cout << "RS size: " << sizeRS << endl;
//     fprintf(f_outState, "RS size: %ld\n", sizeRS);

//     cout << "================================================\n\n" << endl;


//     if (rs.IndexRS() != 0) {
//         rgmedd_exception obj("Error in method IndexRS\n");
//         throw (obj);
//     }
//     rs.ComputingRG(output_flag, wngrR, f_outState, Top, Bot, InitSet, false);


//     rs.clearNSF();

//     for (int AA = 0; AA < (signed)automa.size(); AA++) {
//         for (int tt = 0; tt < ntr; tt++) {
//             computeFTH(F, T, H, tt);
//             if (rs.MakeNextState(F, T, H, tt, AA) != 0) {
//                 //build as loop in the automaton
//                 rgmedd_exception obj("Error in method MakeNextState\n");
//                 throw (obj);
//             }//build as loop in the automaton
//             if (automa[AA].find(tt) != automa[AA].end()) {
//                 if (rs.MakeNextState(F, T, H, tt, AA, automa) != 0) {
//                     rgmedd_exception obj("Error in method MakeNextState\n");
//                     throw (obj);
//                 }
//             }
//         }
//     }



//     cout << "==================== RS X A ====================" << endl;
//     startRSA = clock();
//     if (priority) {
//         if (rs.genRSAll() != 0) {
//             rgmedd_exception obj("Error in method genRSALL\n");
//             throw (obj);
//         }
//     }
//     else {
//         if (rs.genRSTimed() != 0) {
//             rgmedd_exception obj("Error in method genRSTimed\n");
//             throw (obj);
//         }
//     }
//     endRSA = clock();


//     sizeRS = rs.getSizeRS();
//     fprintf(f_outState, "RS X A size: %ld\n", sizeRS);
//     cout << "RS X A size: " << sizeRS << endl;
//     cout << "================================================\n\n" << endl;

// #if DEBUGPARSER
//     cout << "\n*******************************************************";
//     rs.statistic();
//     cout << "*******************************************************\n\n";
// #endif

//     endGlobal = clock();
//     timeGlobal = ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC;
//     timeRS = ((double)(endRS - startRS)) / CLOCKS_PER_SEC;
//     timeRSA = ((double)(endRSA - startRSA)) / CLOCKS_PER_SEC;

//     free(F);
//     free(T);
//     free(H);

//     if (output_flag) {
//         cout << "================= RS X A Output ================" << endl;
//         cout << rs;
//         cout << "================================================\n\n" << endl;
//     }


//     startInd = clock();

//     if (rs.IndexRS() != 0) {
//         rgmedd_exception obj("Error in method IndexRS\n");
//         throw (obj);
//     }
//     endInd = clock();
//     timeInd = ((double)(endInd - startInd)) / CLOCKS_PER_SEC;


//     rs.ComputingRG(output_flag, wngr, f_outState, Top, Bot, InitSet, true);

//     fprintf(rgr_aux, "toptan= %ld\n", (long)rs.getSizeRS());
//     fprintf(rgr_aux, "topvan= %ld\n", 0L);

// #if PERFORMANCE
//     cout << "===================== TIME =====================" << endl;
//     cout << "Total Time: " << setprecision(7) << timeGlobal << " sec" << endl;
//     cout << "RS Time:  " << setprecision(7) << timeRS << " sec" << endl;
//     cout << "RS X A Time: " << setprecision(7) << timeRSA << " sec" << endl;
//     cout << "RS indexing Time: " << setprecision(7) << timeInd << " sec" << endl;
//     cout << "================================================\n\n" << endl;

//     cout << "==================== MEMORY ====================" << endl;
//     rs.getRS().getForest()->showInfo(stdout, 0);
//     cout << "================================================" << endl;

// #endif


// }/* End build_graphAutoma */
// /*AUTOMATON*/
