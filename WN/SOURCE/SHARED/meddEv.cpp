

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include "medd.h"





extern ifstream fin;
extern ofstream fout;
extern int Max_Token_Bound;
extern int AState;
extern double _prec;
extern int _iter;
extern int out_mc;

extern dddRS::VariableOrderCriteria g_varOrderCriteria;

extern bool CTMC;

//-----------------------------------------------------------------------------

static int from_GUI = -1;

bool invoked_from_gui() {
    if (from_GUI == -1) { // Not yet determined
        const char *env = getenv("FROM_GUI");
        from_GUI = (env != NULL && 0 == strcmp(env, "1"));
    }
    return from_GUI != 0;
}

static bool print_stat_for_gui_flag = false;
extern "C" int print_stat_for_gui() {
    return (invoked_from_gui() && print_stat_for_gui_flag);
}
bool set_print_stat_for_gui() { print_stat_for_gui_flag = true; }

//-----------------------------------------------------------------------------

namespace dddRS {

void var_order_heuristic_FORCE(const VariableOrderCriteria, std::vector<int> &out_order);
void var_order_heuristic_FORCE_PINV(const VariableOrderCriteria voc, std::vector<int> &out_order);

RSRG::RSRG(const int &npl, std::string netname, const long meddly_cache_size) {

    this->cardinality = 0.0;
    this->npl = npl;
    this->netname = netname;
    this->propname = "";
    this->realBounds = NULL;
    this->maxSumTokens = -1;

    //Create mapping  between Place name and interger
    for (int i = 0; i < npl; i++) {
        S2Ipl[tabp[i].place_name] = i;
    }

    for (int i = 0; i < ntr; i++) {
        S2Itr[tabt[i].trans_name] = i;
    }

    net_to_mddLevel.resize(npl);
    std::fill(net_to_mddLevel.begin(), net_to_mddLevel.end(), -1);

    bool invert_mapping = false;
    bool fill_missing_vars = false;
    switch (g_varOrderCriteria) {
    case VOC_NO_REORDER:
        // Places appear int the MDD levels in the same order of the .net file
        for (size_t i = 0; i < npl; i++)
            net_to_mddLevel[i] = i;
        break;

    case VOC_PINV_HEURISTIC: {
        int *pinv_order = sort_according_to_pinv();
        std::copy(pinv_order, pinv_order + npl, net_to_mddLevel.begin());
        free(pinv_order);
        fill_missing_vars = true;
        invert_mapping = true;
        // Invert the order (the function sort_according_to_pinv)
        // constructs a mdd level -> net order, we need the opposite
        break;
    }
    case VOC_FROM_FILE: {
        load_var_order_from_file();
        fill_missing_vars = true;
        break;
    }
    case VOC_FORCE_HEURISTIC:
    case VOC_FORCE_NES_HEURISTIC:
    case VOC_FORCE_WES1_HEURISTIC:
        var_order_heuristic_FORCE(g_varOrderCriteria, net_to_mddLevel);
        // invert_mapping = true;
        break;

    case VOC_FORCE_PINV_HEURISTIC:
        var_order_heuristic_FORCE_PINV(g_varOrderCriteria, net_to_mddLevel);
        break;

    default:
        printf("Internal error: Unknown variable order criteria constant.\n");
        exit(-1);
    }
    assert(net_to_mddLevel.size() == npl);

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

    // Print the place order we use
    if (!running_for_MCC()) {
        // Get the place corresponding to each MDD level
        int mddLevel_to_net[npl];
        for (int p = 0; p < npl; p++)
            mddLevel_to_net[ net_to_mddLevel[p] ] = p;
        // for (int p=0; p<npl; p++)
        //     cout << net_to_mddLevel[p] << " ";
        // cout << endl << endl;
        cout << "VARORD: ";
        for (int p = 0; p < npl; p++)
            cout << tabp[ mddLevel_to_net[p] ].place_name << " ";
        cout << endl << endl;
    }


    MEDDLY::settings s;
    if (meddly_cache_size > 0) {
        s.computeTable.maxSize = meddly_cache_size;
        printf("Setting MEDDLY cache to %d entries.\n", s.computeTable.maxSize);
    }
    MEDDLY::initialize(s);


    //Create DD domain
    d = createDomain();
    bounds = (int *) malloc((npl) * sizeof(int));

    // Setup the estimated place bounds
    for (int i = 0; i < npl; i++)
        bounds[i] = (Max_Token_Bound == -1) ? 255 : Max_Token_Bound;
    // if (Max_Token_Bound == -1)
    // {
    try {
        read_bound(bounds);
    }
    catch (...) {
        cerr << "Error reading bounds from file." << endl;
        // for (int i = 0; i < npl; i++) {
        //    bounds[i] = 255;
    }
    // }

    // }
    // else
    //   for (int i = 0; i < npl; i++) {
    //     bounds[i] = Max_Token_Bound;
    //   }

    d->createVariablesBottomUp(bounds, npl);


    forest::policies fp(false); // false: not a relation
    fp.setFullyReduced();
    // fp.setQuasiReduced();
    //fp.setCompactStorage();
    fp.setOptimistic();
//Create DD forests
    fRS = d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, fp);


    forest::policies fp1(true); // false: not a relation
    fp1.setIdentityReduced();
    //fp1.setCompactStorage();
    fp1.setOptimistic();
// Create DD forests
//fRS = d->createForest(false, forest::BOOLEAN,forest::MULTI_TERMINAL,fp);
//fRS->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
//fRS->setNodeDeletion(forest::OPTIMISTIC_DELETION);

    fTranB = d->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL, fp1);
//fTranB->setReductionRule(forest::IDENTITY_REDUCED);
//fTranB->setReductionRule(forest::QUASI_REDUCED);
//fTranB->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
//fTranB->setNodeDeletion(forest::OPTIMISTIC_DELETION);

    fTranR = d->createForest(true, forest::REAL, forest::MULTI_TERMINAL, fp1);
//fTranR->setReductionRule(forest::IDENTITY_REDUCED);
//fTranR->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
//fTranR->setNodeDeletion(forest::OPTIMISTIC_DELETION);

//it is initialized only when it needs
    fEV = NULL;


//Create DD edge

    rs = new dd_edge(fRS);
    initMark = new dd_edge(fRS);
    indexrs = NULL;
    NSFt = new dd_edge(fTranB);
    //For priorities
    for (int i = 0; i < MAXPRIO; i++) {
        prioNSFt[i] = defNSFt[i] = NULL;
    }
    //For priorities

    NSFi = new dd_edge(fTranB);
    NSFtReal = new dd_edge(fTranR);
    DiagReal = new dd_edge(fTranR);
    VectNSFtReal = (dd_edge **) malloc((ntr) * sizeof(dd_edge *));
    for (int i = 0; i < ntr; i++) {
        VectNSFtReal[i] = new dd_edge(fTranR);
    }

    // to = (int **) malloc((1) * sizeof(int *));
    // to[0] = (int *) malloc((npl + 1) * sizeof(int));
    // from = (int **) malloc((1) * sizeof(int *));
    // from[0] = (int *) malloc((npl + 1) * sizeof(int));
    // ins = (int **) malloc((1) * sizeof(int *));
    // ins[0] = (int *) malloc((npl + 1) * sizeof(int));

    // pfrom = (int **) malloc((1) * sizeof(int *));
    // pfrom[0] = (int *) malloc((npl + 1) * sizeof(int));

    // for (int i = 0; i < npl + 1; i++) {
    //     to[0][i] = pfrom[0][i] = from[0][i] = ins[0][i] = -2;
    // }
}


RSRG::~RSRG() {
    if (realBounds != NULL)
        free(realBounds);
    // if (to != NULL) {
    //      free(to[0]);
    //      free(to);
    //  }
    //  if (from != NULL) {
    //      free(from[0]);
    //      free(from);
    //  }
    //  if (ins != NULL) {
    //      free(ins[0]);
    //      free(ins);
    //  }
    //  if (pfrom != NULL) {
    //      free(pfrom[0]);
    //      free(pfrom);
    //  }
    if (rs != NULL)
        delete rs;
    if (initMark != NULL)
        delete initMark;
    if (NSFt != NULL)
        delete NSFt;
    if (prioNSFt[0] != NULL) {
        for (int i = 0; i < MAXPRIO; i++) {
            delete prioNSFt[i];
            delete defNSFt[i];
        }
    }
    if (NSFi != NULL)
        delete NSFi;
    if (NSFtReal != NULL)
        delete NSFtReal;
    if (DiagReal != NULL)
        delete DiagReal;

    //Free memory
    free(bounds);

}

bool RSRG::init_RS(const Net_Mark_p &net_mark) {
    vector<int> ins(npl + 1);
    for (int pl = 0 ; pl < npl; pl++) {
        ins[net_to_mddLevel[pl] + 1] = net_mark[pl].total;
    }
    const int *ins_ptr = ins.data();
    fRS->createEdge(&ins_ptr, 1, *rs);
    fRS->createEdge(&ins_ptr, 1, *initMark);
#if DEBUG
    for (int i = 0; i < npl; i++) {
        cout << "[" << net_to_mddLevel[i] << "]" << ins[net_to_mddLevel[i] + 1] << " ";
        //cout<<"["<<i<<"]"<<ins[0][i]<<" ";
    }
    cout << endl << endl;
#endif

    // for (int pl = 0 ; pl < npl; pl++) {
    //     ins[pl + 1] = -1;
    // }

#if DEBUG1
    cout << "\nInitial Marking\n";
    (*rs).show(stdout, 2);
#endif
    return 0;
}




void RSRG::read_bound(int *bounds) {
    std::string bnd_file(netname + ".bnd");
    cout << "Opening file: " << bnd_file.c_str() << endl;
    ifstream in(bnd_file.c_str());
    if (!in) {
        cerr << "Cannot read input file " << bnd_file << endl;
        return;
        // throw std::string("Error opening .bnd");
    }
    int i = 0;
    while (in && i < npl) {
        in >> (bounds[net_to_mddLevel[i]]) >> (bounds[net_to_mddLevel[i]]);
        bounds[net_to_mddLevel[i]]++;
        ++i;
    }
    for (int i = 0; i < npl; i++) {
        if (bounds[i] < 0)
            bounds[i] = Max_Token_Bound;
        if (bounds[i] < 0) // NOTE: Max_Token_Bound could be -1 if undefined.
            bounds[i] = 255;
    }
#if DEBUG
    for (int i = 0; i < npl; i++)
        cout << "Place " << i << " has bound " << bounds[i] << "." << endl;
#endif
}

//-------------------------------------------------------------------------

// class MxMDD_inserter {
//     dd_edge*   MxMDD;       // Target MxMDD
//     const RSRG* rsrg;       // Defines the variable ordering
//     const int buf_size;     // Number of states added in each batch
//     int num_states;         // Current number of states in the buffer
//     // Buffer of MxM states waiting to be stored in the DD
//     std::vector<int*> src_vec;
//     std::vector<int*> dst_vec;
// public:
//     MxMDD_inserter(dd_edge *the_MxMDD, const RSRG *_rsrg, const int buf);
//     ~MxMDD_inserter();

//     void flush();
//     void insert(const int *src, const int *dst);
// };

// MxMDD_inserter::MxMDD_inserter(dd_edge *the_MxMDD, const RSRG* _rsrg, const int buf)
// : MxMDD(the_MxMDD), rsrg(_rsrg), buf_size(buf) {
//     num_states = 0;
//     src_vec.resize(buf);
//     dst_vec.resize(buf);
//     for (int i=0; i<buf_size; i++) {
//         src_vec[i] = new int[1 + npl];
//         dst_vec[i] = new int[1 + npl];
//     }
// }

// MxMDD_inserter::~MxMDD_inserter() {
//     flush();
//     for (int i=0; i<buf_size; i++) {
//         delete[] src_vec[i];
//         delete[] dst_vec[i];
//     }
// }

// void MxMDD_inserter::flush() {
//     if (num_states > 0) {
//         dd_edge edges(MxMDD->getForest());
//         int** p_src = src_vec.data();
//         int** p_dst = dst_vec.data();
//         MxMDD->getForest()->createEdge(p_src, p_dst, num_states, edges);
//         *MxMDD += edges;
//         num_states = 0;
//     }
// }

// void MxMDD_inserter::insert(const int *src, const int *dst) {
//     if (num_states >= buf_size)
//         flush();
//     std::copy(src, src + 1 + npl, src_vec[num_states]);
//     std::copy(dst, dst + 1 + npl, dst_vec[num_states]);
//     num_states++;
// }


//-------------------------------------------------------------------------

bool RSRG::MakeNextState(const int *F, const int *T, const int *H, const int tt) {

    dd_edge asf(fTranB);

    dd_edge Adif(fTranB);

    // MxMDDs of the CTMC (EV* or Multi-Terminal)
    dd_edge asfReal(fTranR);

    float rate = tabt[tt].mean_t;
    int prio = tabt[tt].pri;
    int server = tabt[tt].no_serv;
    if (server == 0)
        server = MAXSERV;
    float enable = 0.0;

    vector<int> to(npl + 1), from(npl + 1), pfrom(npl + 1), ins(npl + 1);
    const int *to_ptr = to.data(), *from_ptr = from.data();
    const int *pfrom_ptr = pfrom.data(), *ins_ptr = ins.data();

    for (int i = 0; i < npl; i++) { // DEBUG
        if ((F[i] == 0) && (T[i] == 0) && (H[i] == 0))
            to[net_to_mddLevel[i] + 1] = from[net_to_mddLevel[i] + 1] = -2;
        else
            to[net_to_mddLevel[i] + 1] = from[net_to_mddLevel[i] + 1] = -1;
    }
    std::fill(pfrom.begin(), pfrom.end(), -1);
    std::fill(ins.begin(), ins.end(), -1);


    fTranB->createEdge(&from_ptr, &to_ptr, 1, asf);
    fTranB->createEdge(&pfrom_ptr, &ins_ptr, 1, Adif);
    enable = rate * (float)server;
    fTranR->createEdge(&from_ptr, &to_ptr, &(enable), 1, asfReal);

    for (int pl = 0; pl < npl; pl++) {
        bool inserted = false;
        dd_edge nsf(fTranB);
        dd_edge nsfReal(fTranR);
        dd_edge tempAdif(fTranB);

        //  Determine the effective bound for this place
        int b;
        if (H[pl] != 0)
            b = H[pl] - 1;
        else
            b = getbound(net_to_mddLevel[pl]) - 1;

        if (b - F[pl] < 0)
            return 0; // transition is never enabled.

        //case input  and output  and inibitor
        while ((b - F[pl] >= 0) && (F[pl] > 0)) {
            pfrom[net_to_mddLevel[pl] + 1] = from[net_to_mddLevel[pl] + 1] = b;
            to[net_to_mddLevel[pl] + 1] = b - F[pl] + T[pl];
            enable = (b / F[pl]) > server ? server : (b / F[pl]);
            enable = rate * enable;
            if (to[net_to_mddLevel[pl] + 1] <= getbound(net_to_mddLevel[pl]) - 1) {
                // nsf_ins.insert(from_ptr, to_ptr);
                dd_edge temp(fTranB);
                fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
                nsf += temp;

                if (CTMC) {
                    dd_edge tempReal(fTranR);
                    fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
                    nsfReal += tempReal;
                }

                if (prio != 0) {
                    dd_edge temp2(fTranB);
                    fTranB->createEdge(&pfrom_ptr, &ins_ptr, 1, temp2);
                    tempAdif += temp2;
                }
                inserted = true;
            }
            b--;
        }

        //case only output
        enable = rate * server;
        while ((b >= 0) && (T[pl] > 0) && (F[pl] == 0)) {
            pfrom[net_to_mddLevel[pl] + 1] = from[net_to_mddLevel[pl] + 1] = b;
            to[net_to_mddLevel[pl] + 1] = b + T[pl];
            b--;

            if ((to[net_to_mddLevel[pl] + 1] <=  getbound(net_to_mddLevel[pl]) - 1)) {
                // nsf_ins.insert(from_ptr, to_ptr);
                dd_edge temp(fTranB);
                fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
                nsf += temp;


                if (CTMC) {
                    dd_edge tempReal(fTranR);
                    fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
                    nsfReal += tempReal;
                }

                if (prio != 0) {
                    dd_edge temp2(fTranB);
                    fTranB->createEdge(&pfrom_ptr, &ins_ptr, 1, temp2);
                    tempAdif += temp2;
                }
                inserted = true;
            }
        }

        //case only inhibitor
        while (((b >= 0) && (T[pl] == 0) && (F[pl] <= 0) && (H[pl] != 0)) ||
                ((b >= 0) && (T[pl] == 0) && (b - F[pl] < 0) && (H[pl] != 0))) {
            pfrom[net_to_mddLevel[pl] + 1] = to[net_to_mddLevel[pl] + 1] = from[net_to_mddLevel[pl] + 1] = b;
            b--;
            // nsf_ins.insert(from_ptr, to_ptr);
            dd_edge temp(fTranB);
            fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
            nsf += temp;

            if (CTMC) {
                dd_edge tempReal(fTranR);
                fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
                nsfReal += tempReal;
            }

            if (prio != 0) {
                dd_edge temp2(fTranB);
                fTranB->createEdge(&pfrom_ptr, &ins_ptr, 1, temp2);
                tempAdif += temp2;
            }
            inserted = true;
        }


        // reset vectors
        pfrom[net_to_mddLevel[pl] + 1] = from[net_to_mddLevel[pl] + 1] = to[net_to_mddLevel[pl] + 1] = -1;

        if (inserted) {
            asf *= nsf;
            if (CTMC) {
                apply(MINIMUM, asfReal, nsfReal, asfReal);
            }
            if (prio != 0) {
                Adif *= tempAdif;
            }
        }
    }

    if (prio != 0) {
        if (prioNSFt[prio] == NULL) {
            prioNSFt[prio] = new dd_edge(fTranB);
            defNSFt[prio] = new dd_edge(fTranB);
        }
        (*prioNSFt[prio]) += asf;
        (*defNSFt[prio]) += Adif;
        //(*NSFt)+=asf;
    }
    else {
        (*NSFt) += asf;
        if (CTMC)
            (*NSFtReal) += asfReal;
    }




#if DEBUG
    cout << "size :" << (*DiagReal).getCardinality() << endl;
#endif
    // NSFtReal->show(stdout, 2);
    return 0;
}


bool RSRG::genRSTimed() {

    clock_t startGlobal, endGlobal;
    startGlobal = clock();
//For prio
    for (int i = 1; i < MAXPRIO; i++) {
        if (prioNSFt[i] != NULL) {
            try {
                (*NSFt) -= (*defNSFt[i]);
            }
            catch (MEDDLY::error e) {
                cerr << "genRSTimed: MIN. MEDDLY Error: " << e.getName() << endl;
                return false;
            }
            //(*NSFt)+=(*prioNSFt[i]);
            apply(UNION, *NSFt, *prioNSFt[i], *NSFt);
        }
    }
//


//UPDATE 23-12-11
    try {
        // int num_steps = 0;
        // while (true) {
        //     dd_edge current(*rs);
        //     apply(POST_IMAGE, *rs, *NSFt, *rs);
        //     apply(UNION, current, *rs, *rs);
        //     num_steps++;
        //     if ((num_steps % 10) == 1)
        //         cout << num_steps << " " << flush;
        //     if (current == (*rs))
        //         break;
        // }
        // cout << "Convergence in " << num_steps << " steps." << endl;
        apply(REACHABLE_STATES_DFS, *rs, *NSFt, *rs);
        // apply(REVERSE_REACHABLE_DFS, *rs, *NSFt, *rs);
    }
    catch (MEDDLY::error e) {
        cerr << "MEDDLY Error: " << e.getName() << endl;
        if (running_for_MCC()) {
            cout << "CANNOT_COMPUTE" << endl;
            exit(-1);
        }
        return false;
    }
    endGlobal = clock();
    if (!running_for_MCC())
        cout << "\tTime RS: " << ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC << endl;
    // startGlobal = clock();
    // endGlobal = clock();
    // cout << "\tTime Card: " << ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC << endl;
    return false;
}



bool RSRG::genRSAll() {

    double first, second;

    dd_edge reached(fRS);
    dd_edge sourceXT(fRS);
    dd_edge source(fRS);
    dd_edge tmp(fRS);



    sourceXT += *rs;
    do {
        //UPDATE 23-12-11
        try {
            apply(POST_IMAGE, sourceXT, *NSFt, reached);
        }
        catch (MEDDLY::error e) {
            cerr << "genRSAll: MEDDLY Error: " << e.getName() << endl;
            return false;
        }
        sourceXT.clear();
        sourceXT += reached;
        second = rs->getCardinality();
        (*rs) += reached;
        do {
            //UPDATE 23-12-11
            try {
                apply(POST_IMAGE, reached, *NSFi, tmp);
            }
            catch (MEDDLY::error e) {
                cerr << "genRSAll: MEDDLY Error: " << e.getName() << endl;
                return false;
            }
            first = rs->getCardinality();
            sourceXT += tmp - (*rs);
            (*rs) += tmp;
            //UPDATE 23-12-11
            /*if (cm->apply(compute_manager::PRE_IMAGE,tmp, *NSFi,source)!=0)
                {
                cerr<<"Error PRE_IMAGE"<<endl;
                return -1;
                }*/
            //UPDATE 23-12-11
            try {
                apply(PRE_IMAGE, tmp, *NSFi, source);
            }
            catch (MEDDLY::error e) {
                cerr << "genRSAll: MEDDLY Error: " << e.getName() << endl;
                return false;
            }
            sourceXT -= source;
            source.clear();
            reached.clear();
            reached += tmp;
            tmp.clear();
        }
        while ((rs->getCardinality() - first) > _DIFFMIN(double));
        reached.clear();
    }
    while ((rs->getCardinality() - second) > _DIFFMIN(double));
    return 0;
}



bool RSRG::IndexRS() {
    fEV = d->createForest(false, forest::INTEGER, forest::EVPLUS);
    indexrs = new dd_edge(fEV);
    indexrs->clear();

//UPDATE 23-12-1
    try {
        apply(CONVERT_TO_INDEX_SET, *rs, *indexrs);
    }
    catch (MEDDLY::error e) {
        cerr << "IndexRS: MEDDLY Error: " << e.getName() << endl;
        return false;
    }
    return 0;
}


bool RSRG::JacobiSolver() {

    int jj = 0;

    double cardinality = this->getSizeRS();
    if (cardinality < 1) {
        return 0;
    }

    double *q1 = (double *) malloc((int)cardinality * sizeof(double));
    double *q2 = (double *) malloc((int)cardinality * sizeof(double));
    double *qold = (double *) malloc((int)cardinality * sizeof(double));
    double *h = (double *) malloc((int)cardinality * sizeof(double));
    for (jj = 0; jj < (int)cardinality; jj++) {
        q1[jj] = 1 / cardinality;
        q2[jj] = 0.0;
        qold[jj] = 1.0;
        h[jj] = 0.0;
    }



//UPDATE 23-12-1
    numerical_operation *VM = EXPLVECT_MATR_MULT->buildOperation(*indexrs, *DiagReal , *indexrs);
    try {
        VM->compute(h, qold);
    }
    catch (MEDDLY::error e) {
        cerr << "JacobiSolver: diagonal. MEDDLY Error: " << e.getName() << endl;
        return false;
    }
    numerical_operation *VM1 = EXPLVECT_MATR_MULT->buildOperation(*indexrs, *NSFtReal, *indexrs);

    int ss = 0;
    double sum = 0.0, diff = 0.0, norm = 0.0;
    bool precision = false;
    while ((ss < _iter) && (!precision)) {
        //UPDATE 23-12-1
        try {
            VM1->compute(q2, q1);
        }
        catch (MEDDLY::error e) {
            cerr << "JacobiSolver: vectorMatrixMultiply. MEDDLY Error: " << e.getName() << endl;
            return false;
        }

        if (ss % 1000 == 0)
            cout << "\tIteration: " << ss << " error: " << diff << "\n";
#if DEBUG
        cout << "Iteration: " << ss << " error: " << diff << "\n\t";
#endif
        sum = diff = norm = 0.0;
        for (int jj = 0; jj < (int)cardinality; jj++) {
            q2[jj] = (q2[jj]) / h[jj] + q1[jj];
            sum += q2[jj];
        }
        for (int jj = 0; jj < (int)cardinality; jj++) {
            q1[jj] = q2[jj] / sum;
            diff += pow(q1[jj], 2) - pow(qold[jj], 2);
            norm += pow(qold[jj], 2);
            qold[jj] = q1[jj];
            q2[jj] = 0.0;
        }

        diff = (sqrt(fabs(diff))) / sqrt(norm);
        if (diff < _prec)
            precision = true;
#if DEBUG1
        cout << "error: " << diff << endl;
#endif
        ss++;
    }
    if (diff > _prec) {
        cerr << "\nError: Precision  was not reached!!!" << endl;
        return -1;
    }
    else {
#if DEBUG1
        for (int jj = 0; jj < (int)cardinality; jj++) {
            cout << q1[jj] << " ";
        }
        cout << endl;
#endif
        std::string file = netname + std::string(".prob");
        ofstream out(file.c_str(), ofstream::out);
        if (!out) {
            cerr << "\n*****Error opening output stream .prob *****" << endl;
            return -1;
        }
        out << setprecision(IOPRECISION);
        for (int jj = 0; jj < (int)cardinality; jj++) {
            out << q1[jj] << endl;
        }
        out.close();
        file = netname + std::string(".sta");
        out.open(file.c_str(), ofstream::out);
        if (!out) {
            cerr << "\n*****Error opening output stream .sta *****" << endl;
            return -1;
        }
        out << setprecision(IOPRECISION);
        for (int tt = 0; tt < ntr; tt++) {
            //UPDATE 23-12-1
            /*if (cm->vectorMatrixMultiply(q2, *indexrs,  q1, *indexrs,*(VectNSFtReal[tt]))!=0)
                {
                cerr<<"Error  vectorMatrixMultiply"<<endl;
                return -1;
                }*/
            //UPDATE 23-12-1
            numerical_operation *VMT = EXPLVECT_MATR_MULT->buildOperation(*indexrs, *(VectNSFtReal[tt]), *indexrs);
            try {
                VMT->compute(q2, q1);
            }
            catch (MEDDLY::error e) {
                cerr << "JacobiSolver: vectorMatrixMultiply. MEDDLY Error: " << e.getName() << endl;
                return false;
            }
            sum = 0.0;
            for (int jj = 0; jj < (int)cardinality; jj++) {
                sum += q2[jj];
                q2[jj] = 0.0;
            }
            out << "Thru_" << tabt[tt].trans_name << " = " << sum << endl;
        }
        out.close();
    }
//end implementazione esplicita

//free memory
    free(q1);
    free(q2);
    free(qold);
    free(h);

    return 0;
}


bool RSRG::visitXBounds(int node, std::vector<bool> &visited,
                        std::vector<bool> &retValue,
                        std::vector<int> &nodeMaxSumTokens) {
    expert_forest *forest = static_cast<expert_forest *>(rs->getForest());
    if (node == getMeddlyTerminalNodeID(false))
        return false;
    if (node == getMeddlyTerminalNodeID(true))
        return true;
    if (node >= visited.size()) {
        cerr << node << " " << visited.size() << endl;
        cerr << "ERROR: the cache size is smaller than the forest peak size." << endl;
        exit(-1);
    }
    if (visited[node])
        return retValue[node];

    nodeMaxSumTokens[node] = 0;
    // cout << "Visit " << node << endl;
    node_reader rnode;
    forest->initNodeReader(rnode, node, true);
    bool rv = false;
    if (rnode.isFull()) {
        for (int i = rnode.getSize() - 1; i >= 0; i--) {
            if (visitXBounds(rnode.d(i), visited, retValue, nodeMaxSumTokens)) {
                realBounds[rnode.getLevel()] = std::max(realBounds[rnode.getLevel()], i);
                int subnode = rnode.d(i);
                if (subnode == getMeddlyTerminalNodeID(true))
                    nodeMaxSumTokens[node] = std::max(nodeMaxSumTokens[node], i);
                else if (subnode != getMeddlyTerminalNodeID(false)) {
                    nodeMaxSumTokens[node] = std::max(nodeMaxSumTokens[node],
                                                      nodeMaxSumTokens[subnode] + i);
                }
                // cout << "\t (FULL)TRUE-->" << rnode.getLevel() << "  i=" << realBounds[rnode.getLevel()] << endl;
                rv = true;
            }
        }
        // cout << "\t (FULL)FALSE\n";
        visited[node] = true;
        retValue[node] = rv;
        return rv;
    }
    else {
        for (int i = rnode.getNNZs() - 1; i >= 0; i--) {
            if (visitXBounds(rnode.d(i), visited, retValue, nodeMaxSumTokens)) {
                realBounds[rnode.getLevel()] = std::max(realBounds[rnode.getLevel()], rnode.i(i));
                int subnode = rnode.d(i);
                if (subnode == getMeddlyTerminalNodeID(true))
                    nodeMaxSumTokens[node] = std::max(nodeMaxSumTokens[node], i);
                else if (subnode != getMeddlyTerminalNodeID(false)) {
                    nodeMaxSumTokens[node] = std::max(nodeMaxSumTokens[node],
                                                      nodeMaxSumTokens[subnode] + rnode.i(i));
                }
                // cout << "\t (SPARSE)TRUE-->" << rnode.getLevel() << "  i=" << realBounds[rnode.getLevel()] << endl;
                rv = true;
            }
        }
        // cout << "\t (SPARSE)FALSE\n";
        visited[node] = true;
        retValue[node] = rv;
        return rv;
    }
}

// Compute the effective bounds of each variable by visiting all the nodes of the MDD
// and by taking the maximum bound of each variable in every level.
void RSRG::computeRealBounds(void) {
    long numNodes = rs->getForest()->getPeakNumNodes() + 1;
    if (!running_for_MCC())
        cout << "Computing effective variable bounds by visiting the MDD." << endl;
    std::vector<bool> visited(numNodes, 0), retValue(numNodes);
    std::vector<int> nodeMaxSumTokens(numNodes);
    realBounds = (int *)malloc(sizeof(int) * (npl + 1));
    assert(d->getNumVariables() == npl); // it must change if the number of levels is not equal to the places' number
    memset(realBounds + 1, 0, sizeof(int)*npl);
    realBounds[0] = -1;
    visitXBounds(rs->getNode(), visited, retValue, nodeMaxSumTokens);

    maxSumTokens = nodeMaxSumTokens[rs->getNode()];
    // rs->show(stdout, 2);
//#if DEBUG
    // for (int i = 0; i < npl + 1; i++)
    //     cout << "Variable " << i << " has bound " << realBounds[i] << endl;
//#endif
}





static const int VBP_NOT_YET_VISITED = -10;
static const int VBP_GOES_TO_ZERO_TERMINAL = -12;
// Compute the exact bound of the given set of places.
// The exact bound of a set of places is the maximum number of tokens that may
// appear in any tangible state for these places.
int RSRG::computeRealBoundOfPlaces(const std::vector<bool> &selected_places) const {
    long numNodes = rs->getForest()->getPeakNumNodes() + 1;
    std::vector<int> visit_cache(numNodes, VBP_NOT_YET_VISITED);
    assert(d->getNumVariables() == npl); // it must change if the number of levels is not equal to the places' number
    assert(selected_places.size() == npl); // it must change if the number of levels is not equal to the places' number

    return visitXBoundOfPlaces(rs->getNode(), selected_places, visit_cache);
}

int RSRG::visitXBoundOfPlaces(const int node,
                              const std::vector<bool> &selected_places,
                              std::vector<int> &cache) const {
    expert_forest *forest = static_cast<expert_forest *>(rs->getForest());
    if (node == getMeddlyTerminalNodeID(false))
        return VBP_GOES_TO_ZERO_TERMINAL;
    if (node == getMeddlyTerminalNodeID(true))
        return 0;
    if (node >= cache.size()) {
        cerr << node << " " << cache.size() << endl;
        cerr << "ERROR: the cache size is smaller than the forest peak size." << endl;
        exit(-1);
    }
    if (cache[node] != VBP_NOT_YET_VISITED)
        return cache[node];

    int node_level = forest->getNodeLevel(node);
    assert(node_level >= 0 && node_level < selected_places.size());
    bool is_selected = (selected_places[node_level]);

    node_reader rnode;
    forest->initNodeReader(rnode, node, true);
    int maximum = VBP_GOES_TO_ZERO_TERMINAL;

    if (rnode.isFull()) {
        for (int i = rnode.getSize() - 1; i >= 0; i--) {
            int val = visitXBoundOfPlaces(rnode.d(i), selected_places, cache);
            if (val != VBP_GOES_TO_ZERO_TERMINAL) {
                if (is_selected)
                    val += i;
                maximum = std::max(maximum, val);
            }
        }
        cache[node] = maximum;
        return cache[node];
    }
    else {
        for (int i = rnode.getNNZs() - 1; i >= 0; i--) {
            int val = visitXBoundOfPlaces(rnode.d(i), selected_places, cache);
            if (val != VBP_GOES_TO_ZERO_TERMINAL) {
                if (is_selected)
                    val += rnode.i(i);
                maximum = std::max(maximum, val);
            }
        }
        cache[node] = maximum;
        return cache[node];
    }
}






// RSRGAuto::RSRGAuto(const int &npl, std::string netname) {

//     this->cardinality = 0.0;
//     this->npl = npl;
//     this->netname = netname;

// //Create DD ComputeManager
// //cm = MEDDLY_getComputeManager();

// //Create DD domain
//     d = createDomain();
//     int *bounds = (int *) malloc((npl + 1) * sizeof(int));
//     for (int i = 0; i < npl + 1; i++) {
//         bounds[i] = Max_Token_Bound;
//     }
//     bounds[npl + 1] = AState;
//     d->createVariablesBottomUp(bounds, npl + 1);

//     forest::policies fp(false); // false: not a relation
//     fp.setFullyReduced();
//     //fp.setCompactStorage();
//     fp.setOptimistic();
// //Create DD forests
//     fRS = d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, fp);


//     forest::policies fp1(true); // false: not a relation
//     fp1.setIdentityReduced();
//     //fp1.setCompactStorage();
//     fp1.setOptimistic();
// // Create DD forests
// //fRS = d->createForest(false, forest::BOOLEAN,forest::MULTI_TERMINAL,fp);
// //fRS->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //fRS->setNodeDeletion(forest::OPTIMISTIC_DELETION);

//     fTranB = d->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL, fp1);
// //fTranB->setReductionRule(forest::IDENTITY_REDUCED);
// //fTranB->setReductionRule(forest::QUASI_REDUCED);
// //fTranB->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //fTranB->setNodeDeletion(forest::OPTIMISTIC_DELETION);

//     fTranR = d->createForest(true, forest::REAL, forest::MULTI_TERMINAL, fp1);
// //fTranR->setReductionRule(forest::IDENTITY_REDUCED);
// //fTranR->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //fTranR->setNodeDeletion(forest::OPTIMISTIC_DELETION);


// //it is initialized only when it needs
//     fEV = NULL;


// //Create DD edge

//     rs = new dd_edge(fRS);
//     initMark = new dd_edge(fRS);
//     indexrs = NULL;
//     NSFt = new dd_edge(fTranB);
//     NSFi = new dd_edge(fTranB);
//     NSFtReal = new dd_edge(fTranR);
//     DiagReal = new dd_edge(fTranR);
//     VectNSFtReal = (dd_edge **) malloc((ntr) * sizeof(dd_edge *));
//     for (int i = 0; i < ntr; i++) {
//         VectNSFtReal[i] = new dd_edge(fTranR);
//     }

// // //Create vector
// //     to = (int **) malloc((1) * sizeof(int *));
// //     to[0] = (int *) malloc((npl + 2) * sizeof(int));
// //     from = (int **) malloc((1) * sizeof(int *));
// //     from[0] = (int *) malloc((npl + 2) * sizeof(int));
// //     ins = (int **) malloc((1) * sizeof(int *));
// //     ins[0] = (int *) malloc((npl + 2) * sizeof(int));

// //     for (int i = 0; i < npl + 2; i++) {
// //         to[0][i] = from[0][i] = ins[0][i] = -2;
// //     }


// //Create mapping  between Place name and interger
//     for (int i = 0; i < npl; i++) {
//         S2Ipl[tabp[i].place_name] = i;
//     }

//     for (int i = 0; i < ntr; i++) {
//         S2Itr[tabt[i].trans_name] = i;
//     }

//     fRG = d->createForest(false, forest::REAL, forest::MULTI_TERMINAL);
// //fRG->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //fRG->setNodeDeletion(forest::OPTIMISTIC_DELETION);





// //Free memory
//     free(bounds);
// }

// bool RSRGAuto::MakeNextState(const int *F, const int *T, const int *H, const int tt,
//                              int AA, map <int, map <int, class ATRANSITION> > &automa) {

//     dd_edge nsf(fTranB);
//     dd_edge nsfminus(fTranB);//used to remove transition

//     dd_edge asf(fTranB);
//     dd_edge asfminus(fTranB);//used to remove transition

//     dd_edge nsfReal(fTranR);
//     dd_edge nsfRealminus(fTranR);//used to remove transition

//     dd_edge asfReal(fTranR);
//     dd_edge asfRealminus(fTranR);//used to remove transition



//     dd_edge temp(fTranB);

//     dd_edge tempReal(fTranR);



//     int aa = 0;

//     float rate = tabt[tt].mean_t;
//     int prio = tabt[tt].pri;

//     int server = tabt[tt].no_serv;
//     if (server == 0)
//         server = MAXSERV;
//     float enable = 0.0;

//     vector<int> to(npl + 1), from(npl + 1);
//     const int *to_ptr = to.data(), *from_ptr = from.data();

// //Automaton state
//     to[npl + 1] = automa[AA][tt].getD();
//     from[npl + 1] = AA;
// //Automaton state

//     for (int i = 0; i < npl + 1; i++) {
//         if ((i > 0) && (F[i - 1] == 0) && (T[i - 1] == 0) && (H[i - 1] == 0) && (!automa[AA][tt].findPre(i - 1)) && (!automa[AA][tt].findPost(i - 1)))
//             to[i] = from[i] = -2;
//         else
//             to[i] = from[i] = -1;
//     }

//     int Max, Min, MinTemp, initb, b;

//     for (int pl = 0; pl < npl; pl++) {
//         initb = Max_Token_Bound - 1;
//         Max = Min = MinTemp = -1;
// //archi inibitori
//         if (H[pl] != 0) {
//             initb = H[pl] - 1;
//         }
// //archi inibitori
// //Pre conditions
//         if (automa[AA][tt].getBoundPre(pl, Max, Min)) {
//             initb = initb > Max ? Max : initb;
//         }
// //Post conditions

//         if (automa[AA][tt].getBoundPost(pl, Max, MinTemp)) {
//             initb = initb > (Max - T[pl]) ? (Max - T[pl]) : initb;
//             Min = Min < (MinTemp - T[pl]) ? (MinTemp - T[pl]) : Min;
//         }

//         nsf.clear();
//         nsfminus.clear();
//         nsfReal.clear();
//         nsfRealminus.clear();
//         aa = 0;

//         b = initb;

//         if ((b - F[pl] < 0) || (b < Min)) return 0; //transition never  enable!!

//         while ((b - F[pl] >= 0) && (F[pl] > 0) && (b >= Min)) {
//             from[pl + 1] = b;
//             to[pl + 1] = b - F[pl] + T[pl];
//             enable = (int)b / F[pl] > server ? server : (int)b / F[pl];
//             enable = ((tt + 1) * INCTRAN) + (rate * enable);
//             //enable= rate * enable;
//             if (to[pl + 1] <= Max_Token_Bound - 1) {
//                 fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal); //rate solo la prima volta
//                 nsfReal += tempReal;
//                 tempReal.clear();
//                 //to remove
//                 to[npl + 1] = from[npl + 1] = AA;
//                 fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsfminus += temp;
//                 temp.clear();
//                 fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//                 nsfRealminus += tempReal;
//                 tempReal.clear();
//                 to[npl + 1] = automa[AA][tt].getD();
//                 from[npl + 1] = AA;
//                 //to remove
//                 aa = 1;
//             }
//             b--;
//             to[pl + 1] = from[pl + 1] = -1;
//         }
//         if (aa == 1) {
//             if (asf.getNode() != 0) {
//                 asf *= nsf;
//                 asfminus *= nsfminus;
//                 try {
//                     apply(MINIMUM, asfReal, nsfReal, asfReal);
//                     apply(MINIMUM, asfRealminus, nsfRealminus, asfRealminus);
//                 }
//                 catch (MEDDLY::error e) {
//                     cerr << "MakeNextState: MIN. MEDDLY Error: " << e.getName() << endl;
//                     return false;
//                 }
//             }
//             else {
//                 asf += nsf;
//                 asfminus += nsfminus;
//                 asfReal += nsfReal;
//                 asfRealminus += nsfRealminus;
//             }
//             nsf.clear();
//             nsfminus.clear();
//             nsfReal.clear();
//             nsfRealminus.clear();
//         }
//         aa = 0;
//         b = initb;
//         enable = ((tt + 1) * INCTRAN) + server * rate;
//         //enable=server*rate;
//         while ((b >= 0) && (T[pl] > 0) && (F[pl] == 0) && (b >= Min)) {
//             from[pl + 1] = b;
//             to[pl + 1] = b + T[pl];
//             b--;
//             if ((to[pl + 1] <= Max_Token_Bound - 1)) {
//                 fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//                 nsfReal += tempReal;
//                 tempReal.clear();

//                 //to remove
//                 to[npl + 1] = from[npl + 1] = AA;
//                 fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsfminus += temp;
//                 temp.clear();
//                 fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//                 nsfRealminus += tempReal;
//                 tempReal.clear();
//                 to[npl + 1] = automa[AA][tt].getD();
//                 from[npl + 1] = AA;
//                 //to remove
//                 aa = 1;
//             }
//             from[pl + 1] = to[pl + 1] = -1;
//         }
//         if (aa == 1) {
//             if (asf.getNode() != 0) {
//                 asf *= nsf;
//                 asfminus *= nsfminus;
//                 //UPDATE 23-12-11
//                 try {
//                     apply(MINIMUM, asfReal, nsfReal, asfReal);
//                     apply(MINIMUM, asfRealminus, nsfRealminus, asfRealminus);
//                 }
//                 catch (MEDDLY::error e) {
//                     cerr << "MakeNextState: MIN. MEDDLY Error: " << e.getName() << endl;
//                     return false;
//                 }
//                 //UPDATE 23-12-11
//             }
//             else {
//                 asf += nsf;
//                 asfminus += nsfminus;
//                 asfReal += nsfReal;
//                 asfRealminus += nsfRealminus;
//             }
//         }
//         aa = 0;
//         b = initb;

//         while (((b >= 0) && (T[pl] == 0) && (F[pl] <= 0) && (H[pl] != 0) && (b >= Min)) || ((b >= 0) && (T[pl] == 0) && (b - F[pl] < 0) && (H[pl] != 0) && (b >= Min))) {
//             to[pl + 1] = from[pl + 1] = b;
//             b--;
//             fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsf += temp;
//             temp.clear();
//             fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//             nsfReal += tempReal;
//             tempReal.clear();
//             aa = 1;

//             //to remove
//             to[npl + 1] = from[npl + 1] = AA;
//             fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsfminus += temp;
//             temp.clear();
//             fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//             nsfRealminus += tempReal;
//             tempReal.clear();
//             to[npl + 1] = automa[AA][tt].getD();
//             from[npl + 1] = AA;
//             //to remove
//             from[pl + 1] = to[pl + 1] = -1;
//         }

//         if (aa == 1) {
//             if (asf.getNode() != 0) {
//                 asf *= nsf;
//                 asfminus *= nsfminus;
//                 //UPDATE 23-12-11
//                 try {
//                     apply(MINIMUM, asfReal, nsfReal, asfReal);
//                     apply(MINIMUM, asfRealminus, nsfRealminus, asfRealminus);
//                 }
//                 catch (MEDDLY::error e) {
//                     cerr << "MakeNextState: MIN. MEDDLY Error: " << e.getName() << endl;
//                     return false;
//                 }

//             }
//             else {
//                 asf += nsf;
//                 asfminus += nsfminus;
//                 asfReal += nsfReal;
//                 asfRealminus += nsfRealminus;
//                 //aDiagReal+=tempDiagReal;
//             }
//         }

//         aa = 0;
//         b = initb;

//         while (((automa[AA][tt].findPre(pl)) || (automa[AA][tt].findPost(pl))) && (((b >= 0) && (T[pl] == 0) && (F[pl] <= 0) && (H[pl] == 0) && (b >= Min)) || ((b >= 0) && (T[pl] == 0) && (b - F[pl] < 0) && (H[pl] == 0) && (b >= Min)))) {
//             to[pl + 1] = from[pl + 1] = b;
//             b--;
//             fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsf += temp;
//             temp.clear();
//             fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//             nsfReal += tempReal;
//             tempReal.clear();
//             aa = 1;

//             //to remove
//             to[npl + 1] = from[npl + 1] = AA;
//             fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsfminus += temp;
//             temp.clear();
//             fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//             nsfRealminus += tempReal;
//             tempReal.clear();
//             to[npl + 1] = automa[AA][tt].getD();
//             from[npl + 1] = AA;
//             //to remove
//             from[pl + 1] = to[pl + 1] = -1;
//         }

//         if (aa == 1) {
//             if (asf.getNode() != 0) {
//                 asf *= nsf;
//                 asfminus *= nsfminus;
//                 //UPDATE 23-12-11
//                 try {
//                     apply(MINIMUM, asfReal, nsfReal, asfReal);
//                     apply(MINIMUM, asfRealminus, nsfRealminus, asfRealminus);
//                 }
//                 catch (MEDDLY::error e) {
//                     cerr << "MakeNextState: MIN. MEDDLY Error: " << e.getName() << endl;
//                     return false;
//                 }
//             }
//             else {
//                 asf += nsf;
//                 asfminus += nsfminus;
//                 asfReal += nsfReal;
//                 asfRealminus += nsfRealminus;
//             }
//         }

//     }

//     if (prio != 0) {
//         (*NSFi) += asf;
//         (*NSFi) -= asfminus;
//     }
//     else {
//         (*NSFt) += asf;
//         (*NSFt) -= asfminus;
//         (*NSFtReal) += asfReal;
//         (*NSFtReal) -= asfRealminus;
// //controllare--> lento se abilitato!!!!
//         //(*DiagReal)+=aDiagReal;
//         //(*VectNSFtReal[tt])+=aDiagReal;
//     }
//     asf.clear();
//     asfminus.clear();
//     asfReal.clear();
//     asfRealminus.clear();



// #if DEBUG
//     cout << "size :" << (*DiagReal).getCardinality() << endl;
// #endif
//     return 0;
// }



// bool RSRGAuto::MakeNextState(const int *F, const int *T, const int *H, const int tt, int AA) {

//     dd_edge nsf(fTranB);
//     dd_edge asf(fTranB);
//     dd_edge nsfReal(fTranR);
//     dd_edge asfReal(fTranR);

//     dd_edge temp(fTranB);
//     dd_edge tempReal(fTranR);



//     int aa = 0;

//     float rate = tabt[tt].mean_t;
//     int prio = tabt[tt].pri;

//     int server = tabt[tt].no_serv;
//     if (server == 0)
//         server = MAXSERV;
//     float enable = 0.0;

//     vector<int> to(npl + 1), from(npl + 1);
//     const int *to_ptr = to.data(), *from_ptr = from.data();
// //Automaton state
//     to[npl + 1] = from[npl + 1] = AA;
// //Automaton state


//     for (int i = 0; i < npl + 1; i++) {
//         if ((i > 0) && (F[i - 1] == 0) && (T[i - 1] == 0) && (H[i - 1] == 0))
//             to[i] = from[i] = -2;
//         else
//             to[i] = from[i] = -1;
//     }


//     for (int pl = 0; pl < npl; pl++) {
//         int b = Max_Token_Bound - 1;
// //archi inibitori
//         if (H[pl] != 0) {
//             b = H[pl] - 1;
//         }
// //archi inibitori
//         nsf.clear();
//         nsfReal.clear();

//         aa = 0;

//         if (b - F[pl] < 0) return 0; //transition never  enable!!

//         while ((b - F[pl] >= 0) && (F[pl] > 0)) {
//             from[pl + 1] = b;
//             to[pl + 1] = b - F[pl] + T[pl];
//             enable = (int)b / F[pl] > server ? server : (int)b / F[pl];
//             enable = ((tt + 1) * INCTRAN) + (rate * enable);
//             //enable=(rate * enable);
//             if (to[pl + 1] <= Max_Token_Bound - 1) {
//                 fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal); //rate solo la prima volta
//                 nsfReal += tempReal;
//                 tempReal.clear();
//                 aa = 1;
//             }
//             b--;
//             to[pl + 1] = from[pl + 1] = -1;
//         }
//         if (aa == 1) {
//             if (asf.getNode() != 0) {
//                 asf *= nsf;
//                 //UPDATE 23-12-11
//                 try {
//                     apply(MINIMUM, asfReal, nsfReal, asfReal);
//                 }
//                 catch (MEDDLY::error e) {
//                     cerr << "MakeNextState: MIN. MEDDLY Error: " << e.getName() << endl;
//                     return false;
//                 }
//             }
//             else {
//                 asf += nsf;
//                 asfReal += nsfReal;
//             }
//             nsf.clear();
//             nsfReal.clear();


//         }
//         aa = 0;
// //archi inibitori
//         if (H[pl] != 0) {
//             b = H[pl] - 1;
//         }
//         else
//             b = Max_Token_Bound - 1;
// //archi inibitori
//         enable = ((tt + 1) * INCTRAN) + rate * server;
//         //enable=enable= rate * server;
//         while ((b >= 0) && (T[pl] > 0) && (F[pl] == 0)) {
//             from[pl + 1] = b;
//             to[pl + 1] = b + T[pl];
//             b--;
//             if ((to[pl + 1] <= Max_Token_Bound - 1)) {
//                 fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//                 nsfReal += tempReal;
//                 tempReal.clear();
//                 aa = 1;
//             }
//             from[pl + 1] = to[pl + 1] = -1;
//         }
//         if (aa == 1) {
//             if (asf.getNode() != 0) {
//                 asf *= nsf;
//                 //UPDATE 23-12-11
//                 try {
//                     apply(MINIMUM, asfReal, nsfReal, asfReal);
//                 }
//                 catch (MEDDLY::error e) {
//                     cerr << "MakeNextState: MIN. MEDDLY Error: " << e.getName() << endl;
//                     return false;
//                 }
//             }
//             else {
//                 asf += nsf;
//                 asfReal += nsfReal;
//             };
//         }
//         aa = 0;
// //archi inibitori
//         if (H[pl] != 0) {
//             b = H[pl] - 1;
//         }
//         else
//             b = Max_Token_Bound - 1;
// //archi inibitori

//         //enable=enable= rate * server;
//         while (((b >= 0) && (T[pl] == 0) && (F[pl] <= 0) && (H[pl] != 0)) || ((b >= 0) && (T[pl] == 0) && (b - F[pl] < 0) && (H[pl] != 0))) {
//             to[pl + 1] = from[pl + 1] = b;
//             b--;
//             fTranB->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsf += temp;
//             temp.clear();
//             fTranR->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//             nsfReal += tempReal;
//             tempReal.clear();
//             aa = 1;
//             from[pl + 1] = to[pl + 1] = -1;
//         }
//         if (aa == 1) {
//             if (asf.getNode() != 0) {
//                 asf *= nsf;
//                 //UPDATE 23-12-11
//                 try {
//                     apply(MINIMUM, asfReal, nsfReal, asfReal);
//                 }
//                 catch (MEDDLY::error e) {
//                     cerr << "MakeNextState: MIN. MEDDLY Error: " << e.getName() << endl;
//                     return false;
//                 }
//             }
//             else {
//                 asf += nsf;
//                 asfReal += nsfReal;
//             }

//         }
//     }


//     if (prio != 0) {
//         (*NSFi) += asf;
//     }
//     else {
//         (*NSFt) += asf;
//         (*NSFtReal) += asfReal;
//     }
//     asf.clear();
//     asfReal.clear();

// #if DEBUG
//     cout << "size :" << (*DiagReal).getCardinality() << endl;
// #endif
//     return 0;
// }


// bool RSRGAuto::init_RS(const Net_Mark_p &net_mark) {

//     vector<int> ins(npl + 1);
//     const int *ins_ptr = ins.data();
//     ins[0] = 0;
//     for (int pl = 0 ; pl < npl; pl++) {
//         /* foreach place */
//         ins[pl + 1] = net_mark[pl].total;
//     }/* foreach place */
//     ins[npl + 1] = 0;
//     fRS->createEdge(&ins_ptr, 1, *rs);
//     fRS->createEdge(&ins_ptr, 1, *initMark);
// #if DEBUG
//     for (int i = 0; i < npl + 1; i++) {
//         cout << "[" << i << "]" << ins[i] << " ";
//     }
//     cout << endl << endl;
// #endif



// #if DEBUG1
//     cout << "\nInitial Marking\n";
//     (*rs).show(stdout, 3);
// #endif
//     return 0;
// }




// bool RSRGAuto::ComputingRG(const bool &output, FILE *wngr, FILE *f_outState, const int &Top, const int &Bot, set <int> &InitSet, bool outState) {

//     enumerator i(*(this->rs));
// //dd_edge::iterator i = this->rs->begin();

//     const int *psource;
//     const int *plmark;




//     dd_edge src(fRG);

//     float num = 1.0, val = 0.0;
//     int idm = 0;
// //char c;
// //int card;


//     if (output)
//         cout << "================= RG X A Output ================" << endl;


//     fprintf(wngr, "1\n");
// //fwrite(&idm,sizeof(int), 1,wngr);

//     while (i != 0) {
//         psource = i.getAssignments();
//         fRG->createEdge(&psource, &num, 1, src);
//         //get id marking
//         fEV->evaluate((*indexrs), psource, idm);
//         //get id marking
//         if (outState) {
//             if (psource[npl + 1] == Top) {
//                 //c='E';
//                 //fwrite(&c,sizeof(char), 1,f_outState);
//                 //fwrite(&idm,sizeof(int), 1,f_outState);
//                 fprintf(f_outState, "E:%d\n", idm);
//             }
//             if (psource[npl + 1] == Bot) {
//                 //c='F';
//                 fprintf(f_outState, "F:%d\n", idm);
//             }
//             if (InitSet.find(psource[npl + 1]) != InitSet.end()) {
//                 //c='S';
//                 fprintf(f_outState, "S:%d\n", idm);
//             }

//         }
//         if (output)
//             cout << "Source: " << idm << endl;
//         //UPDATE 23-12-11
//         try {
//             apply(POST_IMAGE, src, *NSFtReal, src);
//         }
//         catch (MEDDLY::error e) {
//             cerr << "ComputingRG: MIN. MEDDLY Error: " << e.getName() << endl;
//             return false;
//         }
//         fprintf(wngr, "%ld\n", (long int)src.getCardinality());
//         //dd_edge::iterator j = src.begin();//
//         enumerator j((src));
//         while (j != 0) {
//             plmark = j.getAssignments();
//             fEV->evaluate((*indexrs), plmark, idm);
//             fRG->evaluate(src, plmark, val);
//             checkRate(val, psource);
//             if (output)
//                 cout << "\tDest: " << idm << " Rate: " << val << endl;
//             fprintf(wngr, "\t%d %f\n", idm, val);
//             ++j;
//         }
//         ++i;
//     }
//     fprintf(wngr, "%ld\n", (long int)rs->getCardinality());
//     if (output)
//         cout << "================================================\n\n" << endl;
//     return true;
// }


// void RSRGAuto::checkRate(float &val, const int *plmark) {
// //cout<<"VAL:"<<val<<endl;
//     int idT = ((int)val / INCTRAN) - 1;
// #if DEBUG
//     cout << val << " " << this->getTR(idT) << " " << idT << endl;
// #endif
//     val = val - (float)((idT + 1) * INCTRAN);
// //For marking dependent
// #if DEBUG
//     for (int i = 0; i < npl + 1; i++) {
//         cout << plmark[i];
//     }
//     cout << endl;
// #endif
//     //cout<<idT<<" "<<this->getTR(idT)<<endl;
//     //T23t2 mu23*(#p3/(#p3+#P32))
//     if (idT == this->getTR("T23t2"))
//         {
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("p3")+1]/((float)plmark[this->getPL("p3")+1]+(float)plmark[this->getPL("P32")+1]));
//         //cout<<"\t"<<val<<" "<<plmark[this->getPL("P32")+1]<<" "<<plmark[this->getPL("p3")+1]<<endl;
//         }
//     else
//     //T23t28 mu23*(#P32/(#p3+#P32))
//     if (idT == this->getTR("T23t28"))
//         {
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("P32")+1]/((float)plmark[this->getPL("p3")+1]+(float)plmark[this->getPL("P32")+1]));
//         //cout<<val<<" "<<plmark[this->getPL("P32")+1]<<" "<<plmark[this->getPL("p3")+1]<<endl;
//         }
//     else
//     //T23t28t26 mu23*(#P32/(#p3+#P32))
//     if (idT == this->getTR("T23t28t26"))
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("P32")+1]/((float)plmark[this->getPL("p3")+1]+(float)plmark[this->getPL("P32")+1]));
//     else
//     //T23t2t26 mu23*(#p3/(#p3+#P32))
//     if (idT == this->getTR("T23t2t26"))
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("p3")+1]/((float)plmark[this->getPL("p3")+1]+(float)plmark[this->getPL("P32")+1]));
//     else
//     //T22t26 mu22*(#P29/(#p5+#P29))
//     if (idT == this->getTR("T22t26"))
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("P29")+1]/((float)plmark[this->getPL("p5")+1]+(float)plmark[this->getPL("P29")+1]));
//     else
//     //T22t3 mu22*(#p5/(#p5+#P29))
//     if (idT == this->getTR("T22t3"))
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("p5")+1]/((float)plmark[this->getPL("p5")+1]+(float)plmark[this->getPL("P29")+1]));
//     else
//     //T22t26t25  mu22*(#P29/(#p5+#P29))
//     if (idT == this->getTR("T22t26t25"))
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("P29")+1]/((float)plmark[this->getPL("p5")+1]+(float)plmark[this->getPL("P29")+1]));
//     else
//     //T22t3t25  mu22*(#p5/(#p5+#P29))
//     if (idT == this->getTR("T22t3t25"))
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("p5")+1]/((float)plmark[this->getPL("p5")+1]+(float)plmark[this->getPL("P29")+1]));
//     else
//     //T24t27 mu24*(#P30/(#p1+#P30))
//     if (idT == this->getTR("T24t27"))
//         val=tabt[idT].mean_t * ((float)plmark[this->getPL("P30")+1]/((float)plmark[this->getPL("p1")+1]+(float)plmark[this->getPL("P30")+1]));
//     else
//     //T24t1  mu24*(#p1/(#p1+#P30))
//     if (idT == this->getTR("T24t1"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("p1")+1]/((float)plmark[this->getPL("p1")+1]+(float)plmark[this->getPL("P30")+1]));
//     else
//     //T24t1t28  mu24*(#p1/(#p1+#P30))
//     if (idT == this->getTR("T24t1t28"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("p1")+1]/((float)plmark[this->getPL("p1")+1]+(float)plmark[this->getPL("P30")+1]));
//     else
//     //T24t27t28 mu24*(#P30/(#p1+#P30))
//     if (idT == this->getTR("T24t27t28"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("P30")+1]/((float)plmark[this->getPL("p1")+1]+(float)plmark[this->getPL("P30")+1]));
//     else
//     //T21t4  mu21*(#p7/(#p7+#P28))
//     if (idT == this->getTR("T21t4"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("p7")+1]/((float)plmark[this->getPL("p7")+1]+(float)plmark[this->getPL("P28")+1]));
//     else
//     //T21t25  mu21*(#P28/(#p7+#P28))
//     if (idT == this->getTR("T21t25"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("P28")+1]/((float)plmark[this->getPL("p7")+1]+(float)plmark[this->getPL("P28")+1]));
//     //T10t5  muPaperW*#p21
//     if (idT == this->getTR("T10t5"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("p21")+1]);
//     //T10t6 muPaperW*#p21
//     if (idT == this->getTR("T10t6"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("p21")+1]);
//     // T9t6  muTravel*#p19
//     if (idT == this->getTR("T9t6"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("p19")+1]);
//     //T9t5 muTravel*#p19
//     if (idT == this->getTR("T9t5"))
//         val=tabt[idT].mean_t* ((float)plmark[this->getPL("p19")+1]);
    
// }

void RSRG::load_var_order_from_file() {
    ifstream in;
    std::string buffer = net_name + std::string("place");
    in.open(buffer.c_str());
    if (!in) {
        cerr << "\n*****Error opening input stream " << buffer << "*****" << endl;
        throw (ExceptionIO());
    }
    net_to_mddLevel.resize(npl);
    std::fill(net_to_mddLevel.begin(), net_to_mddLevel.end(), -1);

    getline(in, buffer, '\0');
    class general::Parser par;
    char delim[] = "; \n\r\t";
    par.update(delim, buffer);
    for (unsigned int i = 0; i < par.size(); i++) {
        if (S2Ipl.find(par.get(i)) != S2Ipl.end()) {
            net_to_mddLevel[ S2Ipl[par.get(i)] ] = i;
        }
        else {
            cerr << "\n*****Error place " << par.get(i) << " is not presented in the net*****" << endl;
            throw (ExceptionIO());
        }
    }
}


//---------------------------------------------------------------------------------------

struct PSemiflowEntry {
    int place_no;
    int card;
};
typedef std::vector<PSemiflowEntry> PSemiflow;

bool load_Psemiflows(std::vector<PSemiflow> &psf) {
    std::string pinv_name(net_name);
    pinv_name += "pin";
    ifstream pif(pinv_name.c_str());
    if (!pif)
        return false;
    int num_pinv;
    pif >> num_pinv;
    if (!pif || num_pinv < 0) {
        cerr << "\nError: bad PIN file (1)." << endl;
        exit(-1);
    }
    psf.resize(num_pinv);
    for (int i = 0; i < num_pinv; i++) {
        int len;
        pif >> len;
        if (!pif || len < 0) {
            cerr << "\nError: bad PIN file (2)." << endl;
            exit(-1);
        }
        psf[i].resize(len);
        for (int j = 0; j < len; j++) {
            int card, pl;
            pif >> card >> pl;
            if (!pif || card <= 0 || pl <= 0 || pl > npl) {
                cerr << "\nError: bad PIN file (3)." << endl;
                exit(-1);
            }
            psf[i][j].place_no = pl - 1;
            psf[i][j].card = card;
        }
    }
    return true;
}

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

    // for (int N=0; N<1; N++) {
    //     double npts[npl];
    //     for (int p=0; p<npl; p++)
    //         npts[p] = 0.0;
    //     for (int t=0; t<ntr; t++) {
    //         for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next) {
    //             int p1 = in_node->place_no;
    //             int num_pl = 0;
    //             double points = 0.0;
    //             for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next, ++num_pl) {
    //                 int p2 = out_node->place_no;
    //                 // p1 -> t -> p2
    //                 points += pl_points[p2];
    //                 num_pl++;
    //             }
    //             npts[p1] += points / num_pl;
    //         }
    //     }
    //     for (int p=0; p<npl; p++)
    //         pl_points[p] = npts[p];
    // }

    double tot = 0.0;
    for (int p = 0; p < npl; p++)
        tot += pl_points[p];
    return tot;
}

double measure_NES(const std::vector<int> &varorder) {
    double NES = 0.0;
    for (int t = 0; t < ntr; t++) {
        int top = 0, bot = npl - 1;
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next) {
            top = std::max(top, varorder[ in_node->place_no ]);
            bot = std::min(bot, varorder[ in_node->place_no ]);
        }
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next) {
            top = std::max(top, varorder[ out_node->place_no ]);
            bot = std::min(bot, varorder[ out_node->place_no ]);
        }
        NES += top - bot + 1;
    }
    return NES / (ntr * npl);
}

double measure_WESi(const std::vector<int> &varorder, int i) {
    double WES = 0.0;
    double NES = measure_NES(varorder);
    for (int t = 0; t < ntr; t++) {
        int top = 0;
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
            top = std::max(top, varorder[ in_node->place_no ]);
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
            top = std::max(top, varorder[ out_node->place_no ]);
        WES += pow(top / (ntr / 2.), i);
    }
    return WES * NES;
}

template<typename T, typename D>
void safe_div(T &value, const D divisor) {
    if (divisor == D(0))
        value = 0;
    else
        value /= divisor;
}

// Implements the modified FORCE heuristic for the ordering of variables in a Decision Diagram
// See: Aloul, Markov, Sakallah, "FORCE: a fast and easy-to-implement variable-ordering heuristic"
void var_order_heuristic_FORCE(const VariableOrderCriteria voc, std::vector<int> &out_order) {
    cout << "Using FORCE Heuristic for the variable ordering." << endl;
    std::vector<int> var_order(npl), num_trns_of_place(npl);
    std::vector<double> cog(ntr), place_grades(npl);
    std::vector<std::pair<double, int> > sorter(npl);

    // The ordering that minimizes the measure function
    std::vector<int> min_varorder;
    double min_points = -1;

    // Create the initial ordering for the first iteration
    for (int i = 0; i < npl; i++) {
        var_order[i] = i;
        num_trns_of_place[i] = 0;
    }
    // Count the number of transitions connected to each place
    for (int t = 0; t < ntr; t++) {
        for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
            num_trns_of_place[ in_node->place_no ]++;
        for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
            num_trns_of_place[ out_node->place_no ]++;
    }


    const int NUM_STEPS = 200;
    const int MID_PHASE = 20;
    for (int step = 0; step <= NUM_STEPS; step++) {
        if (step == MID_PHASE) {
            // Modified heuristic: reset the ordering at the MID phase
            for (int i = 0; i < npl; i++)
                var_order[i] = i;
        }
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

            // Modified heuristic: after MID steps, relax the center-of-gravity reassignment
            const double MIX = 0.90;
            if (step < MID_PHASE)
                cog[t] = cog_t;
            else
                cog[t] = (1 - MIX) * cog[t] + MIX * cog_t;
        }

        // Compute the new grade of each variable, using the transition COGs
        //   grade(p) = Sum( COG[t] ) / num(t)      where transition t is connected with p
        for (int p = 0; p < npl; p++)
            place_grades[p] = 0.0;
        for (int t = 0; t < ntr; t++) {
            for (Node_p in_node = tabt[t].inptr; in_node != NULL; in_node = in_node->next)
                place_grades[ in_node->place_no ] += cog[t];
        }
        for (int t = 0; t < ntr; t++) {
            for (Node_p out_node = tabt[t].outptr; out_node != NULL; out_node = out_node->next)
                place_grades[ out_node->place_no ] += cog[t];
        }
        for (int p = 0; p < npl; p++)
            safe_div(place_grades[p], num_trns_of_place[p]);

        // Reorder the variables according to their grades, and assign them a new
        // integer positioning from 1 to npl (which becomes the new variable
        // position in the next iteration)
        for (int p = 0; p < npl; p++)
            sorter[p] = std::make_pair(place_grades[p], p);
        std::sort(sorter.begin(), sorter.end());
        for (int p = 0; p < npl; p++)
            var_order[p] = sorter[p].second;

        // if (step > MID_PHASE) {
        // cout << "STEP " << step << endl;
        // for (size_t i=0; i<npl; i++)
        //     cout << tabp[ var_order[i] ].place_name << " ";
        // cout << endl;

        // points = estimate_points(var_order);
        // points = estimate_points_place_spans(var_order, cog);

        // Compute the target measure on this new variable order
        // The objective is to take the ordering that minimizes this measure
        double points;
        switch (voc) {
        case VOC_FORCE_HEURISTIC:
            points = estimate_points_transition_spans(var_order);
            break;
        case VOC_FORCE_NES_HEURISTIC:
            points = measure_NES(var_order);
            break;
        case VOC_FORCE_WES1_HEURISTIC:
        case VOC_FORCE_PINV_HEURISTIC:
            points = measure_WESi(var_order, 1);
            break;
        default:
            cout << "Unexpected value!" << endl;
            exit(-1);
        }
        // double points = measure_WESi(var_order, 1);
        if (min_points < 0 || points < min_points) {
            min_points = points;
            min_varorder = var_order;
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

    // cout << "\n\nMIN POINTS = " << min_points << endl;
    // for (size_t i=0; i<npl; i++)
    //     cout << tabp[ min_varorder[i] ].place_name << " ";
    // cout << endl << endl << endl;


    assert(out_order.size() == npl);
    for (int p = 0; p < npl; p++) {
        assert(min_varorder[p] >= 0 && min_varorder[p] < npl);
        out_order[ min_varorder[p] ] = p;
    }
    // retval[ var_order[p] ] = p;

    // for (int p=0; p<npl; p++)
    //     cout << retval[p] << " ";
    // cout << endl << endl;

    // for (int p=0; p<npl; p++)
    //     cout << tabp[ retval[p] ].place_name << " ";
    // cout << endl << endl;

}

//---------------------------------------------------------------------------------------

void var_order_heuristic_FORCE_PINV(const VariableOrderCriteria voc, std::vector<int> &out_order) {
    cout << "Using FORCE-PINV Heuristic for the variable ordering." << endl;
    // Load P-semiflows (if available)
    std::vector<PSemiflow> psf;
    bool has_semiflows = load_Psemiflows(psf);
    // if (!has_semiflows) {
    //     cout << "No P-semiflows found. Switch to the FORCE heuristic." << endl;
    //     return var_order_heuristic_FORCE(voc, out_order);
    // }

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

        // Compute the target measure on this new variable order
        // The objective is to take the ordering that minimizes this measure
        double points;
        switch (voc) {
        case VOC_FORCE_HEURISTIC:
            points = estimate_points_transition_spans(var_order);
            break;
        case VOC_FORCE_NES_HEURISTIC:
            points = measure_NES(var_order);
            break;
        case VOC_FORCE_WES1_HEURISTIC:
        case VOC_FORCE_PINV_HEURISTIC:
            points = measure_WESi(var_order, 1);
            break;
        default:
            cout << "Unexpected value!" << endl;
            exit(-1);
        }
        if (min_points < 0 || points < min_points) {
            min_points = points;
            min_varorder = var_order;
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

} // namespace dddRS





















