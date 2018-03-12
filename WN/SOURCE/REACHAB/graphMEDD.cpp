#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meddly.h>

#ifndef __PRS_H__
#define __PRS_H__
#include "../AUTOMA/parser.h"
#endif

#ifndef __MDD_H__
#define __MDD_H__
#include "../SHARED/medd.h"
#endif

#ifndef __SET_H__
#define __SET_H__
#include <set>
#endif



extern "C" {


#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/ealloc.h"


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

}
/*MDD*/

using namespace dddRS;

extern ofstream fout;
int steps = 0, maxsteps = 0;

/*MDD*/
extern void CTLParser(dddRS::RSRG *r);

/*AUTOMATON*/
// extern int initAutoma(dddRS::RSRGAuto *rs);
extern map <int, map <int, class ATRANSITION> > automa;
extern int Top, Bot;
extern set <int> InitSet;

/*AUTOMATON*/
//using namespace dddRS;

//!It takes in input a transition ID  and returns its corresponding preset.
void getPreTR(const int tID, int *preset) {
    Node_p l_ptr = NULL;
    int pp;
    for (int ii = 0; ii < npl; ii++) {
        preset[ii] = 0;
    }
    l_ptr = GET_INPUT_LIST(tID);
    while (l_ptr != NULL) {
        pp = GET_PLACE_INDEX(l_ptr);
        preset[pp] = l_ptr->molt;
        l_ptr = NEXT_NODE(l_ptr);
    }
}
//!It takes in input a transition ID  and returns its corresponding inhibitor vector.
void getInhTR(const int tID, int *set) {
    Node_p l_ptr = NULL;
    int pp;
    l_ptr = GET_INHIBITOR_LIST(tID);
    while (l_ptr != NULL) {
        pp = GET_PLACE_INDEX(l_ptr);
        set[pp] = l_ptr->molt;
        l_ptr = NEXT_NODE(l_ptr);
    }
}

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
    int *F = (int *) malloc((npl) * sizeof(int));
    int *T = (int *) malloc((npl) * sizeof(int));
    int *H = (int *) malloc((npl) * sizeof(int));

    clock_t startGlobal, endGlobal, startRS, endRS, startCTL = 0.0, endCTL = 0.0;
    double timeGlobal, timeRS, timeCTL;
    startGlobal = clock();

    if (rs.init_RS(net_mark) != 0) {
        ExceptionIO obj("Error in method init_RS\n");
        throw (obj);
    }
    cout << "----------------------------------------" << endl;
    cout << "Start firing rule encoding" << endl;
    cout << "----------------------------------------" << endl;

    for (int tt = 0; tt < ntr; tt++) {
        if (!running_for_MCC()) {
            cout << "\tEncoding transition " << tabt[tt].trans_name
                 << " (" << tt << "/" << ntr << ")." << endl;
        }
        for (int ii = 0; ii < npl; ii++) {
            H[ii] = T[ii] = F[ii] = 0;
        }
        l_ptr = GET_INPUT_LIST(tt);
        while (l_ptr != NULL) {
            pp = GET_PLACE_INDEX(l_ptr);
            F[pp] = l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_INHIBITOR_LIST(tt);
        while (l_ptr != NULL) {
            pp = GET_PLACE_INDEX(l_ptr);
            H[pp] = l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_OUTPUT_LIST(tt);
        while (l_ptr != NULL) {
            pp = GET_PLACE_INDEX(l_ptr);
            T[pp] = l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
// 	for (int i=0;i<npl;i++)
// 		cout<<F[i]<<" ";
// 	cout<<endl;

        if (rs.MakeNextState(F, T, H, tt) != 0) {
            ExceptionIO obj("Error in method MakeNextState\n");
            throw (obj);
        }
        // cout << "\tDone." << endl;
    }


    free(F);
    free(T);
    free(H);


    cout << "----------------------------------------" << endl;
    cout << "End firing rule encoding\n" << endl;
    cout << "----------------------------------------" << endl;
    cout << "----------------------------------------" << endl;
    cout << "Start RS generation" << endl;
    cout << "----------------------------------------" << endl;


    startRS = clock();

    int who1 = RUSAGE_SELF;
    struct rusage usage1;
    getrusage(who1, &usage1);
    cout << "\n Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;

#if DEBUG
    cout << "\n*********************************************" << endl;
    cout << "Initial marking\n" << rs;
    cout << "*********************************************" << endl;
#endif

    if (rs.genRSTimed() != 0) {
        ExceptionIO obj("Error during the state space generation\n");
        throw (obj);
    }

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
    endRS = clock();

    cout << "\tRS size: " << (long long)rs.getSizeRS() << " (" << rs.getSizeRS() << ")" << endl;
    rs.getRealBound(1);

    cout << "----------------------------------------" << endl;
    cout << "End RS generation" << endl;
    cout << "----------------------------------------" << endl << endl;

    if (running_for_MCC() && !CTL) {
        // Print STATE_SPACE results
        int max_place_bound = 0;
        for (int i = 1; i <= npl; i++)
            max_place_bound = std::max(max_place_bound, rs.getRealBound(i));
        cout << "STATE_SPACE STATES " << rs.getSizeRS() << " TECHNIQUES DECISION_DIAGRAMS SEQUENTIAL_PROCESSING\n";
        cout << "STATE_SPACE TRANSITIONS -1 TECHNIQUES DECISION_DIAGRAMS SEQUENTIAL_PROCESSING\n";
        cout << "STATE_SPACE MAX_TOKEN_PER_MARKING -1 TECHNIQUES DECISION_DIAGRAMS SEQUENTIAL_PROCESSING\n";
        cout << "STATE_SPACE MAX_TOKEN_IN_PLACE -1 TECHNIQUES DECISION_DIAGRAMS SEQUENTIAL_PROCESSING\n";
        // cout << "STATE_SPACE MAX_TOKEN_PER_MARKING " << rs.get_token_sum_bound() << " TECHNIQUES DECISION_DIAGRAMS\n";
        // cout << "STATE_SPACE MAX_TOKEN_IN_PLACE " << max_place_bound << " TECHNIQUES DECISION_DIAGRAMS\n";
        cout << endl;
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


#if DEBUG
    cout << "\n*********************************************\n";
    cout << "Rechability Set: \n" << rs;
    cout << "*********************************************\n";
#endif

    if (!running_for_MCC() && out_mc) {
        cout << "----------------------------------------" << endl;
        cout << "Start CTMC solution" << endl;
        cout << "----------------------------------------" << endl;
        cout << "\tStart Index RS  generation" << endl;
        if (rs.IndexRS() != 0) {
            ExceptionIO obj("Error in method IndexRS\n");
            throw (obj);
        }
        cout << "\tEnd Index RS  generation" << endl;
        if (rs.JacobiSolver() != 0) {
            ExceptionIO obj("Error in method JacobiSolver\n");
            throw (obj);
        }
        cout << "----------------------------------------" << endl;
        cout << "End CTMC solution" << endl;
        cout << "----------------------------------------" << endl << endl;
    }

    if (CTL) {
        if (!running_for_MCC()) {
            cout << "----------------------------------------" << endl;
            cout << "Start CTL" << endl << endl;
            cout << "----------------------------------------" << endl;
        }
        startCTL = clock();
        CTLParser(&rs);
        endCTL = clock();
        if (!running_for_MCC()) {
            cout << "----------------------------------------" << endl;
            cout << "End CTL" << endl;
            cout << "----------------------------------------" << endl << endl;
        }
    }


#if DEBUG1
    for (int i = 0; i < npl; i++) {
        cout << "Place Name: " << tabp[i].place_name << " id:" << rs.getPL(std::string(tabp[i].place_name)) << endl;
    }
    for (int i = 0; i < ntr; i++) {
        cout << "Transition Name: " << tabt[i].trans_name << " id:" << rs.getTR(std::string(tabt[i].trans_name)) << endl;;
    }

#endif

    endGlobal = clock();
    timeGlobal = ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC;
    timeRS = ((double)(endRS - startRS)) / CLOCKS_PER_SEC;
    if (CTL)
        timeCTL = ((double)(endCTL - startCTL)) / CLOCKS_PER_SEC;


#if PERFORMANCE
    if (!running_for_MCC()) {
        cout << "============================= TIME =============================" << endl;
        cout << " Total Time:            " << setprecision(7) << timeGlobal << " sec." << endl;
        cout << " RS Time:               " << setprecision(7) << timeRS << " sec." << endl;
        if (CTL)
            cout << " CTL Time:              " << setprecision(7) << timeCTL << " sec." << endl;
        // cout << "==============================================================\n" << endl;


        cout << "============================ MEMORY ============================" << endl;
        if (!CTL) {
            rs.getRS()->getForest()->garbageCollect();
            rs.getNSFt()->getForest()->garbageCollect();
            cout << " RS nodes:              " << rs.getRS()->getNodeCount() << endl;
            cout << " Cardinality(RS):       " << rs.getRS()->getCardinality() << endl;
            cout << " Forest(RS) nodes:      " << rs.getRS()->getForest()->getCurrentNumNodes() << " actives, "
                 << rs.getRS()->getForest()->getPeakNumNodes() << " peak, "
                 << rs.getRS()->getForest()->getStats().num_compactions << " compactions." << endl;
            cout << " Forest(RS) size:       " << rs.getRS()->getForest()->getCurrentMemoryUsed() << " Bytes now, "
                 << rs.getRS()->getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

            cout << " Potential RG nodes:    " << rs.getNSFt()->getNodeCount() << endl;
            cout << " Cardinality(RG):       " << rs.getNSFt()->getCardinality() << endl;
            cout << " Forest(RG) nodes:      " << rs.getNSFt()->getForest()->getCurrentNumNodes() << " actives, "
                 << rs.getNSFt()->getForest()->getPeakNumNodes() << " peak, "
                 << rs.getNSFt()->getForest()->getStats().num_compactions << " compactions." << endl;
            cout << " Forest(RG) size:       " << rs.getNSFt()->getForest()->getCurrentMemoryUsed() << " Bytes now, "
                 << rs.getNSFt()->getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;
        }
        int who = RUSAGE_SELF;
        struct rusage usage;
        getrusage(who, &usage);
        cout << "\n Total Used Memory:     " << usage.ru_maxrss << " KBytes." << endl;
        cout << "================================================================\n" << endl;
    }
    if (print_stat_for_gui()) {
        cout << "#{GUI}# RESULT STAT build_time " << timeRS << " seconds." << endl;
        cout << "#{GUI}# RESULT STAT CTL_time " << timeCTL << " seconds." << endl;

        cout << "#{GUI}# RESULT STAT num_tangible_markings " << rs.getRS()->getCardinality() << endl;
        cout << "#{GUI}# RESULT STAT num_rs_nodes " << rs.getRS()->getNodeCount() << endl;
    }
#endif


}/* End build_graph */


void computeFTH(int *F, int *T, int *H, int tt) {
    Node_p l_ptr = NULL;
    int pp;

    for (int ii = 0; ii < npl; ii++) {
        H[ii] = T[ii] = F[ii] = 0;
    }
    l_ptr = GET_INPUT_LIST(tt);
    while (l_ptr != NULL) {
        pp = GET_PLACE_INDEX(l_ptr);
        F[pp] = l_ptr->molt;
        l_ptr = NEXT_NODE(l_ptr);
    }
    l_ptr = GET_INHIBITOR_LIST(tt);
    while (l_ptr != NULL) {
        pp = GET_PLACE_INDEX(l_ptr);
        H[pp] = l_ptr->molt;
        l_ptr = NEXT_NODE(l_ptr);
    }
    l_ptr = GET_OUTPUT_LIST(tt);
    while (l_ptr != NULL) {
        pp = GET_PLACE_INDEX(l_ptr);
        T[pp] = l_ptr->molt;
        l_ptr = NEXT_NODE(l_ptr);
    }
}

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
//         ExceptionIO obj("Error in method init_RS\n");
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
//             ExceptionIO obj("Error in method MakeNextState\n");
//             throw (obj);
//         }

//     }

//     cout << "\n====================== RS ======================" << endl;
//     startRS = clock();
//     if (priority) {
//         if (rs.genRSAll() != 0) {
//             ExceptionIO obj("Error in method genRSALL\n");
//             throw (obj);
//         }
//     }
//     else {
//         if (rs.genRSTimed() != 0) {
//             ExceptionIO obj("Error in method genRSTimed\n");
//             throw (obj);
//         }
//     }
//     endRS = clock();

//     long sizeRS = rs.getSizeRS();
//     cout << "RS size: " << sizeRS << endl;
//     fprintf(f_outState, "RS size: %ld\n", sizeRS);

//     cout << "================================================\n\n" << endl;


//     if (rs.IndexRS() != 0) {
//         ExceptionIO obj("Error in method IndexRS\n");
//         throw (obj);
//     }
//     rs.ComputingRG(output_flag, wngrR, f_outState, Top, Bot, InitSet, false);


//     rs.clearNSF();

//     for (int AA = 0; AA < (signed)automa.size(); AA++) {
//         for (int tt = 0; tt < ntr; tt++) {
//             computeFTH(F, T, H, tt);
//             if (rs.MakeNextState(F, T, H, tt, AA) != 0) {
//                 //build as loop in the automaton
//                 ExceptionIO obj("Error in method MakeNextState\n");
//                 throw (obj);
//             }//build as loop in the automaton
//             if (automa[AA].find(tt) != automa[AA].end()) {
//                 if (rs.MakeNextState(F, T, H, tt, AA, automa) != 0) {
//                     ExceptionIO obj("Error in method MakeNextState\n");
//                     throw (obj);
//                 }
//             }
//         }
//     }



//     cout << "==================== RS X A ====================" << endl;
//     startRSA = clock();
//     if (priority) {
//         if (rs.genRSAll() != 0) {
//             ExceptionIO obj("Error in method genRSALL\n");
//             throw (obj);
//         }
//     }
//     else {
//         if (rs.genRSTimed() != 0) {
//             ExceptionIO obj("Error in method genRSTimed\n");
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
//         ExceptionIO obj("Error in method IndexRS\n");
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
//     rs.getRS()->getForest()->showInfo(stdout, 0);
//     cout << "================================================" << endl;

// #endif


// }/* End build_graphAutoma */
// /*AUTOMATON*/
