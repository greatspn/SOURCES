

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <algorithm>
#include <numeric>
#include <memory>
#include <map>

#include "rgmedd2.h"
#include "varorders.h"
#include "nsf_subtree.h"

extern ifstream fin;
extern ofstream fout;
extern int AState;
extern double _prec;
extern int _iter;
extern int out_mc;

extern bool CTMC;

static const cardinality_t CARD0 = cardinality_t(0);
static const cardinality_t CARD1 = cardinality_t(1);

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


const char* rgmedd_exception::what() const noexcept { 
    return msg.c_str(); 
}

//-----------------------------------------------------------------------------

// remove any tags (used by algebra) from a place/transition name
static const char* filter_tags(const char* name, std::string& cache) {
    const char* tag_sep = strchr(name, '|');
    if (tag_sep == nullptr)
        return name; // no tags in the object name
    size_t n_chars = tag_sep - name;
    cache.resize(n_chars + 1);
    std::copy(name, tag_sep, cache.begin());
    cache.back() = '\0';
    // cout << "<"<<name<<"> -> <"<<cache<<">" << endl;
    return cache.c_str();
}

//-----------------------------------------------------------------------------

// Main initialization of Meddly: variable ordering, make domain & forests, ...
void RSRG::initialize(bool _use_monolithic_nsf,
                      const VariableOrderCriteria voc,
                      bool save_ordering_image,
                      bool reorder_SCC,
                      const long meddly_cache_size, 
                      BoundGuessPolicy _bound_policy) 
{
    this->use_monolithic_nsf = _use_monolithic_nsf;
    this->bound_policy = _bound_policy;
    this->netname = net_name;

    // Map place & transition names to their indexes
    // Note: algebra tags needs to be removed here.
    std::string cache_str;
    for (int i = 0; i < npl; i++) {
        S2Ipl[filter_tags(tabp[i].place_name, cache_str)] = i;
    }
    for (int i = 0; i < ntr; i++) {
        S2Itr[filter_tags(tabt[i].trans_name, cache_str)] = i;
    }
    
    // Determine the variable ordering that will be used in the MDD/MxD domain
    cout << endl;
    print_banner(" VARIABLE ORDER ");
    determine_var_order(voc, save_ordering_image, reorder_SCC, S2Ipl, net_to_mddLevel, true);

    // Inverse variable permutation
    mddLevel_to_net.resize(net_to_mddLevel.size());
    for (int i=0; i<net_to_mddLevel.size(); i++)
        mddLevel_to_net[ net_to_mddLevel[i] ] = i;
    
    // Determine max variable increment as the maximum output arc 
    // entering the corresponding place in the net
    maxVarIncr.resize(npl);
    fill(maxVarIncr.begin(), maxVarIncr.end(), 0);
    for (int tt = 0; tt < ntr; tt++) {
        Node_p l_ptr = GET_OUTPUT_LIST(tt);
        while (l_ptr != NULL) {
            int pp = GET_PLACE_INDEX(l_ptr);
            maxVarIncr[net_to_mddLevel[pp]] = max(maxVarIncr[net_to_mddLevel[pp]], l_ptr->molt);
            l_ptr = NEXT_NODE(l_ptr);
        }
    }

    // cout << "MAX VAR INCR: ";
    // for (int i = 0; i < npl; i++)
    //      cout << maxVarIncr[net_to_mddLevel[i]] << " ";
    // cout << endl << endl;
    
    // initialize Meddly
    MEDDLY::initializer_list* L = defaultInitializerList(0);
    // ct_initializer::setBuiltinStyle(ct_initializer::MonolithicUnchainedHash);
    if (meddly_cache_size > 0) {
        ct_initializer::setMaxSize(meddly_cache_size);
        printf("Setting MEDDLY cache to %ld entries.\n", meddly_cache_size);
    }
    MEDDLY::initialize(L);
    
    initializeGuessedBounds();

    // cout << "DOMAIN BND: ";
    // for (int i = 0; i < npl; i++)
    //      cout << domainBounds(net_to_mddLevel[i]) << " ";
    // cout << endl << endl;

    //Create DD domain
    vector<int> domainBnd(npl);
    for(int i=0; i<npl; i++)
        domainBnd[i] = domainBounds(i);
    dom = createDomainBottomUp(domainBnd.data(), npl);

#warning "Ricontrollare policy garbage collection."
    forest::policies fp(false); // false: not a relation
    fp.setFullyReduced();
    // fp.setQuasiReduced();
    //fp.setCompactStorage();
    fp.setOptimistic();
    forestMDD = dom->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, fp);


    forest::policies fp1(true); // false: not a relation
    fp1.setIdentityReduced();
    //fp1.setCompactStorage();
    fp1.setOptimistic();
    forestMxD = dom->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL, fp1);
    //forestMxD->setReductionRule(forest::IDENTITY_REDUCED);
    //forestMxD->setReductionRule(forest::QUASI_REDUCED);
    //forestMxD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
    //forestMxD->setNodeDeletion(forest::OPTIMISTIC_DELETION);

    forestMTMxD = dom->createForest(true, forest::REAL, forest::MULTI_TERMINAL, fp1);
    //forestMTMxD->setReductionRule(forest::IDENTITY_REDUCED);
    //forestMTMxD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
    //forestMTMxD->setNodeDeletion(forest::OPTIMISTIC_DELETION);
    
    if (useMonolithicNSF()) {
        // Initialize dd_edges for the monolithic NSF
        NSF = dd_edge(forestMxD);
        // NSFi = new dd_edge(forestMxD);
        NSFReal = dd_edge(forestMTMxD);
        DiagReal = dd_edge(forestMTMxD);
        VectNSFReal.resize(ntr, dd_edge(forestMTMxD));
    } else {
        // by events.
    }
}

//-----------------------------------------------------------------------------

void RSRG::initializeGuessedBounds(){
    int max_m0 = 1;
    for(int pl=0; pl < npl; pl++) {
        max_m0 = std::max(net_mark[pl].total, max_m0);
    }

    // Make an initial guess of the variable bounds
    guessedBounds.resize(npl);
    switch(bound_policy){
        case BoundGuessPolicy::M0_LINEAR:
        case BoundGuessPolicy::M0_EXP:
        case BoundGuessPolicy::M0_MAX0_EXP:
            for(int pl=0; pl < npl; pl++) {
                guessedBounds[net_to_mddLevel[pl]] = std::max(net_mark[pl].total, 1);
            }
            break;
            
        case BoundGuessPolicy::MAX0_LINEAR:
        case BoundGuessPolicy::MAX0_EXP:
        {
            std::fill(guessedBounds.begin(), guessedBounds.end(), max_m0);
            break;
        }
        case BoundGuessPolicy::LOAD_FROM_FILE:
        {
            std::string bnd_file(netname + "bnd");
            cout << "Opening file: " << bnd_file.c_str() << endl;
            ifstream in(bnd_file.c_str());
            if (!in) {
                cerr << "Cannot read input file " << bnd_file << endl;
                exit(-1);
                return;
            }
            int i = 0;
            while (in && i < npl) {
                int lowerBoundP, upperBoundP;
                in >> lowerBoundP >> upperBoundP;
                // The bnd file may have both 0 and 2147483647 as "unknown bounds".
                if (upperBoundP >= 2147483647 || upperBoundP <= 0)
                    guessedBounds[net_to_mddLevel[i]] = max_m0;
                else
                    // upperBoundP = -1;
                    guessedBounds[net_to_mddLevel[i]] = 
                        (upperBoundP > 0) ? upperBoundP : std::max(net_mark[i].total, 1);
                i++;
            }
            break;
        }
            
        default:
            cerr << "Bound Guess Policy not implemented!!!" << endl;
            exit(1);
    }

    // cout << "GUESSED BOUNDS: ";
    // for (int i = 0; i < npl; i++)
    //     cout << guessedBounds[net_to_mddLevel[i]] << " ";
    // cout << endl << endl;
}

//-----------------------------------------------------------------------------

RSRG::RSRG() {}

RSRG::~RSRG() {}

//-----------------------------------------------------------------------------

bool RSRG::init_RS(const Net_Mark_p &net_mark) {
    // Create the initial marking as a dd_edge in the BDD forest
    vector<int> ins(npl + 1);
    for (int pl = 0 ; pl < npl; pl++) {
        ins[net_to_mddLevel[pl] + 1] = net_mark[pl].total;
    }
    const int *ins_ptr = ins.data();
    rs = dd_edge(forestMDD);
    initMark = dd_edge(forestMDD); 
    forestMDD->createEdge(&ins_ptr, 1, rs);
    initMark = rs;

// #if DEBUG
//     for (int i = 0; i < npl; i++) {
//         cout << "[" << net_to_mddLevel[i] << "]" << ins[net_to_mddLevel[i] + 1] << " ";
//         //cout<<"["<<i<<"]"<<ins[0][i]<<" ";
//     }
//     cout << endl << endl;
// #endif

// #if DEBUG1
//     cout << "\nInitial Marking\n";
//     rs.show(stdout, 2);
// #endif
    return 0;
}

//-------------------------------------------------------------------------

// the MEDDLY domain bound for variable i (exclusive)
int RSRG::domainBounds(int i) const {
    if (i < 0 || i >= npl)
        throw rgmedd_exception("Bad variable index!");

    if (guessing_bounds)
        return guessedBounds[i] + maxVarIncr[i] + 1;
    else
        return realBounds[i] + 1;
}

//-------------------------------------------------------------------------

// bool RSRG::MakeNextState(const std::vector<int>& F, 
//                          const std::vector<int>& T, 
//                          const std::vector<int>& H, 
//                          const int tt)
// {
//     dd_edge nsf_event(forestMxD);
//     dd_edge enabling(forestMxD);
//     vector<int> minterm(npl+1);
//     int* pMinterm = minterm.data();
//     vector<int> mtprime(npl+1); 
//     int* pMtprime = mtprime.data();
//     int maxBound = 0;
//     for(int pl = 0; pl < npl; pl++){
//         maxBound = max(maxBound, domainBounds(net_to_mddLevel[pl]));
//         if (H[pl]==0 && F[pl]==0 && T[pl]==0) {
//             minterm[net_to_mddLevel[pl]+1] = DONT_CARE;
//             mtprime[net_to_mddLevel[pl]+1] = DONT_CHANGE;
//         } else {
//             minterm[net_to_mddLevel[pl]+1] = DONT_CARE;
//             mtprime[net_to_mddLevel[pl]+1] = DONT_CARE;
//         }
//     }
//     forestMxD->createEdge(&pMinterm, &pMtprime, 1, nsf_event); 
//     forestMxD->createEdge(&pMinterm, &pMinterm, 1, enabling); 
    
//     vector<int> placeFunction(maxBound + 2);
//     placeFunction[0] = 0;
//     int* pPlaceFunction = placeFunction.data() + 1;
//     bool* isEnabledFor = new bool[(maxBound + 1)];
//     bool* pIsEnabledFor = isEnabledFor + 1;
//     //clock_t startOfNsfEv = clock();
    
//     for(int pl = 0; pl < npl; pl++){
//         const int mddPl = net_to_mddLevel[pl];
//         if(H[pl]!=0 || F[pl]!=0 || T[pl]!=0) {
//             // Setup the dd_edge for the enabling conditions of this event
//             dd_edge enablingVar(forestMxD);
//             for(int i = 0; i < domainBounds(mddPl); i++) { 
//                 // if (i <= guessedBounds[mddPl])
//                     pIsEnabledFor[i] = (i>=F[pl]) && ((H[pl]==0) || (i < H[pl]));
//                 // else
//                     // pIsEnabledFor[i] = false;
//             }

//             forestMxD->createEdgeForVar(mddPl+1, false, pIsEnabledFor, enablingVar);
//             enabling *= enablingVar;
//         // }
//         // if(/*H[pl]!=0 || */F[pl]!=0 || T[pl]!=0) {
//             // Setup the dd_edge of the firing of tt at place pl (no enabling conditions test)
//             dd_edge event_firing_pl(forestMxD);
//             int delta = T[pl] - F[pl];
//             for(int i = 0; i < domainBounds(mddPl); i++) {
//                 if (pIsEnabledFor[i] /*&& i <= guessedBounds[mddPl]*/)
//                     pPlaceFunction[i] = (i>=F[pl]) ? (i + delta) : -1;
//                 else
//                     pPlaceFunction[i] = -1;
//             }
//             // for(int i = 0; i < domainBounds(mddPl); i++) {
//             //     pPlaceFunction[i] = (i>=F[pl]) ? (i + delta) : -1;
//             // }

//             createEdgeForRelVar(mddPl, pPlaceFunction, event_firing_pl);

//             //MEDDLY::ostream_output stdout_wrap(cout);
//             //event_firing_pl.show(stdout_wrap, 2);
//             nsf_event *= event_firing_pl;
//         }
//         // if(H[pl]!=0 || F[pl]!=0 || T[pl]!=0) {
//         //     // Setup the dd_edge for the enabling conditions of this event
//         //     dd_edge enablingVar(forestMxD);
//         //     for(int i = 0; i < domainBounds(mddPl); i++){
//         //         pIsEnabledFor[i] = (i>=F[pl]) && ((H[pl]==0) || (i < H[pl]));
//         //     }
//         //     forestMxD->createEdgeForVar(mddPl+1, false, pIsEnabledFor, enablingVar);
//         //     enabling *= enablingVar;

//         //     // Setup the dd_edge conditions and effects of the firing of tt at place pl
//         //     dd_edge event_firing_pl(forestMxD);
//         //     const int delta = T[pl] - F[pl];
//         //     for(int i = 0; i <= guessedBounds[mddPl]; i++){
//         //         pPlaceFunction[i] = i>=F[pl] ? i + delta : -1;
//         //     }
//         //     for(int i = guessedBounds[mddPl]+1; i < domainBounds(mddPl); i++)
//         //         pPlaceFunction[i] = -1;
//         //     createEdgeForRelVar(mddPl, pPlaceFunction, event_firing_pl);
//         //     //MEDDLY::ostream_output stdout_wrap(cout);
//         //     //event_firing_pl.show(stdout_wrap, 2);
//         //     nsf_event *= event_firing_pl;
//         // }
//     }
//     delete[] isEnabledFor;
//     isEnabledFor = pIsEnabledFor = nullptr;

//     if(tabt[tt].pri < ngr) {
//         // There are higher priority transitions. Subtract their enabling
//         // from the enabling of transition tt. Hence, enabling considers the priority.
//         for(int tr=0; tr<ntr; tr++){
//             if(tabt[tt].pri < tabt[tr].pri){
//                 enabling -= all_enabling_events[tr];
//             }
//         }
//     }
//     all_enabling_events[tt] = enabling;
    
//     // Remove concurrently enabled higher-priority transitions from the NSF of this event
//     nsf_event *= enabling;

//     //cout << "Time to build nsf (sec): " << ((clock() - startOfNsfEv) / double(CLOCKS_PER_SEC)) << endl;
//     if(useMonolithicNSF()){
//         NSF += nsf_event;
//     } else { // saturation by events
//         //collection of nsf
//         nsf_collection->addToRelation(nsf_event);
//     }
//     // Keep the NSF of this event, for later use
//     events[tt] = nsf_event;

//     return 0;
// }

//-----------------------------------------------------------------------------

struct RSRG::MakeNsfHelper {
    vector<int> minterm;
    vector<int> mtprime; 
    vector<int> placeFunction;
    bool* isEnabledFor = nullptr;

    int* pMinterm = nullptr;
    int* pMtprime = nullptr;
    int* pPlaceFunction = nullptr;
    bool* pIsEnabledFor = nullptr;
    int maxBound = 0;

    // clock_t clkEnab=0, clkFiring=0, clkComb=0, clkAddRel=0, clkCrEdg1=0, clkCrEdg2=0, clkCrEdg3=0;

    MakeNsfHelper(const RSRG& rsrg) {
        for(int pl = 0; pl < npl; pl++) {
            maxBound = std::max(maxBound, rsrg.domainBounds(rsrg.convertPlaceToMDDLevel(pl)));
        }
        minterm.resize(npl+1, 0);
        pMinterm = minterm.data();
        mtprime.resize(npl+1, 0); 
        pMtprime = mtprime.data();

        placeFunction.resize(maxBound + 2, 0);
        pPlaceFunction = placeFunction.data() + 1;
        placeFunction[0] = 0;

        isEnabledFor = new bool[maxBound + 1];
        pIsEnabledFor = isEnabledFor + 1;
    }

    ~MakeNsfHelper() {
        if (isEnabledFor)
            delete[] isEnabledFor;
    }
};

//-----------------------------------------------------------------------------

// Advance three iterators on a std::map<int, int> that maps place indexes to
// arc multiplicities. Return the lowest place index, and the three values.
template<typename IT> 
bool advance3(IT& i1, const IT end1, IT& i2, const IT end2, IT& i3, const IT end3, 
              int& place, int& val1, int& val2, int& val3) 
{
    if (i1 == end1 && i2 == end2 && i3 == end3)
        return false;

    place = INT_MAX;
    if (i1 != end1)   place = std::min(place, i1->first);
    if (i2 != end2)   place = std::min(place, i2->first);
    if (i3 != end3)   place = std::min(place, i3->first);
    assert(place != INT_MAX);

    if (i1 != end1 && i1->first == place)   { val1 = i1->second; i1++; } 
    else val1 = 0;

    if (i2 != end2 && i2->first == place)   { val2 = i2->second; i2++; } 
    else val2 = 0;

    if (i3 != end3 && i3->first == place)   { val3 = i3->second; i3++; } 
    else val3 = 0;

    return true;
} 

//-----------------------------------------------------------------------------

void RSRG::MakeAllNsfs() {
    unique_ptr<RSRG::MakeNsfHelper> mh(new RSRG::MakeNsfHelper(*this));
    // clock_t timepoint;

    // Cycle from the higher to the lower priority 
    for (int curPrio = ngr, encodedCount = 0; curPrio >= 0; curPrio--) {
        for (int tt = 0; tt < ntr; tt++) {
            if(tabt[tt].pri != curPrio)
                continue;
            if (!running_for_MCC() && useMonolithicNSF()) {
                cout << "\tEncoding transition " << tabt[tt].trans_name
                     << " (" << (++encodedCount) << "/" << ntr << ")";
                if (tabt[tt].pri > 0)
                    cout << ", priority group = " << tabt[tt].pri;
                cout << endl;
            }

            std::map<int, int> F, T, H;
            for (Node_p in_node = GET_INPUT_LIST(tt); in_node != NULL; in_node = NEXT_NODE(in_node))
                F[GET_PLACE_INDEX(in_node)] = in_node->molt;
            for (Node_p out_node = GET_OUTPUT_LIST(tt); out_node != NULL; out_node = NEXT_NODE(out_node))
                T[GET_PLACE_INDEX(out_node)] = out_node->molt;
            for (Node_p ih_node = GET_INHIBITOR_LIST(tt); ih_node != NULL; ih_node = NEXT_NODE(ih_node))
                H[GET_PLACE_INDEX(ih_node)] = ih_node->molt;

            // Initialize empty terms for the enabling MxD and the NSF MxD
            std::fill(mh->minterm.begin(), mh->minterm.end(), DONT_CARE);
            std::fill(mh->mtprime.begin(), mh->mtprime.end(), DONT_CHANGE);
            mh->minterm[0] = mh->mtprime[0] = 0;
            for (auto&& in : F)
                mh->mtprime[ net_to_mddLevel[in.first] + 1 ] = DONT_CARE;
            for (auto&& in : T)
                mh->mtprime[ net_to_mddLevel[in.first] + 1 ] = DONT_CARE;
            for (auto&& in : H)
                mh->mtprime[ net_to_mddLevel[in.first] + 1 ] = DONT_CARE;

            dd_edge nsf_event(forestMxD); // Encodes the Next State Function of transition tt
            dd_edge enabling(forestMxD);  // Encodes the enabling conditions of transition tt
            // timepoint = clock();
            forestMxD->createEdge(&mh->pMinterm, &mh->pMtprime, 1, nsf_event); 
            forestMxD->createEdge(&mh->pMinterm, &mh->pMinterm, 1, enabling); 
            // mh->clkCrEdg1 += clock() - timepoint;
            
            auto itF = F.begin(), itT = T.begin(), itH = H.begin();
            int pl, Fpl, Tpl, Hpl;
            while (advance3(itF, F.end(), itT, T.end(), itH, H.end(), pl, Fpl, Tpl, Hpl)) {
                const int mddPl = net_to_mddLevel[pl];
                assert(Hpl != 0 || Fpl != 0 || Tpl != 0);

                // Enabling conditions of tt for place pl 
                dd_edge enablingAtVar(forestMxD);
                for(int i = 0; i < domainBounds(mddPl); i++) { 
                    // if (i <= guessedBounds[mddPl])
                        mh->pIsEnabledFor[i] = (i>=Fpl) && ((Hpl==0) || (i < Hpl));
                    // else
                        // mh->pIsEnabledFor[i] = false;
                }
                // timepoint = clock();
                forestMxD->createEdgeForVar(mddPl+1, false, mh->pIsEnabledFor, enablingAtVar);
                // mh->clkCrEdg2 += clock() - timepoint;

                // timepoint = clock();
                enabling *= enablingAtVar;
                // mh->clkEnab += clock() - timepoint;

                // Firing outcome of tt for place pl (enabling conditions are ignored)
                dd_edge firingAtVar(forestMxD);
                int delta = Tpl - Fpl;
                for(int i = 0; i < domainBounds(mddPl); i++) {
                    if (mh->pIsEnabledFor[i] /*&& i <= guessedBounds[mddPl]*/)
                        mh->pPlaceFunction[i] = (i>=Fpl) ? (i + delta) : -1;
                    else
                        mh->pPlaceFunction[i] = -1;
                }
                // timepoint = clock();
                createEdgeForRelVar(mddPl, mh->pPlaceFunction, firingAtVar);
                // mh->clkCrEdg3 += clock() - timepoint;

                // timepoint = clock();
                nsf_event *= firingAtVar;
                // mh->clkFiring += clock() - timepoint;

                //MEDDLY::ostream_output stdout_wrap(cout);
                //firingAtVar.show(stdout_wrap, 2);
            }

            // Exclude the states in which higher-priority transitions are enabled
            if(tabt[tt].pri < ngr) {
                for(int hptr=0; hptr<ntr; hptr++) {
                    if(tabt[tt].pri < tabt[hptr].pri) {
                        // Remove the states where hptr is enabled from the enabling of tt
                        enabling -= all_enabling_events[hptr];
                    }
                }
            }
            
            // Combine enabling conditions and firing consequences into the NSF of tt
            // timepoint = clock();
            nsf_event *= enabling;
            // mh->clkComb += clock() - timepoint;

            if(useMonolithicNSF()) {
                NSF += nsf_event;
            } else { // saturation by events
                // timepoint = clock();
                nsf_collection->addToRelation(nsf_event);
                // mh->clkAddRel += clock() - timepoint;
            }

            // Keep the NSF of this event, for later use
            events[tt] = nsf_event;
            all_enabling_events[tt] = enabling;
        }
    }

    // cout << "clkEnab=" << double(mh->clkEnab) / CLOCKS_PER_SEC << endl;
    // cout << "clkFiring=" << double(mh->clkFiring) / CLOCKS_PER_SEC << endl;
    // cout << "clkComb=" << double(mh->clkComb) / CLOCKS_PER_SEC << endl;
    // cout << "clkAddRel=" << double(mh->clkAddRel) / CLOCKS_PER_SEC << endl;
    // cout << "clkCrEdg1=" << double(mh->clkCrEdg1) / CLOCKS_PER_SEC << endl;
    // cout << "clkCrEdg2=" << double(mh->clkCrEdg2) / CLOCKS_PER_SEC << endl;
    // cout << "clkCrEdg3=" << double(mh->clkCrEdg3) / CLOCKS_PER_SEC << endl;
}

//-----------------------------------------------------------------------------

void RSRG::resetNSFCollection() {
    all_enabling_events.resize(ntr);
    events.resize(ntr);

    if (useMonolithicNSF()) {
        NSF = dd_edge(forestMxD);
    }
    else { // by events
        if (nsf_collection != NULL){
            delete nsf_collection;
        }
        nsf_collection = new satpregen_opname::pregen_relation(forestMDD, forestMxD, forestMDD);
    }
}

//-----------------------------------------------------------------------------

void RSRG::genRSTimed() {

    clock_t startGlobal, endGlobal;
    startGlobal = clock();
    if(useMonolithicNSF()){
        // Consider priority groups
        // TODO: this code has been disabled because we don't have
        // the prioNSF array any more. 
        // for (int i = 1; i < MAXPRIO; i++) {
        //     if (prioNSF[i] != NULL) {
        //         // Subtract enabling of higher priority transitions
        //         NSF -= (*defNSF[i]);
        //         //NSF+=(*prioNSF[i]);
        //         apply(UNION, NSF, *prioNSF[i], NSF);
        //     }
        // }



        //UPDATE 23-12-11
        // int num_steps = 0;
        // while (true) {
        //     dd_edge current(rs);
        //     apply(POST_IMAGE, rs, NSF, rs);
        //     apply(UNION, current, rs, rs);
        //     num_steps++;
        //     if ((num_steps % 10) == 1)
        //         cout << num_steps << " " << flush;
        //     if (current == rs)
        //         break;
        // }
        // cout << "Convergence in " << num_steps << " steps." << endl;
        apply(REACHABLE_STATES_DFS, rs, NSF, rs);
        // apply(REVERSE_REACHABLE_DFS, rs, NSF, rs);
    } 
    else {
        specialized_operation* sat = SATURATION_FORWARD->buildOperation(nsf_collection);
        sat->compute(rs, rs);
        // TODO: not sure how to release the memory of this specialized operation
        //delete sat;
    }
    endGlobal = clock();
    if (!running_for_MCC())
        cout << "RS generation time: " << ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC << endl;
    // startGlobal = clock();
    // endGlobal = clock();
    // cout << "\tTime Card: " << ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC << endl;
}

//-----------------------------------------------------------------------------

bool RSRG::genRSAll() {
    throw rgmedd_exception("genRSAll needs to be revised.");

    // double first, second;

    // dd_edge reached(forestMDD);
    // dd_edge sourceXT(forestMDD);
    // dd_edge source(forestMDD);
    // dd_edge tmp(forestMDD);



    // sourceXT += rs;
    // do {
    //     apply(POST_IMAGE, sourceXT, NSF, reached);
    //     sourceXT.clear();
    //     sourceXT += reached;
    //     second = rs.getCardinality();
    //     rs += reached;
    //     do {
    //         //UPDATE 23-12-11
    //         apply(POST_IMAGE, reached, *NSFi, tmp);
    //         first = rs.getCardinality();
    //         sourceXT += tmp - rs;
    //         rs += tmp;
    //         //UPDATE 23-12-11
    //         /*if (cm->apply(compute_manager::PRE_IMAGE,tmp, *NSFi,source)!=0)
    //             {
    //             cerr<<"Error PRE_IMAGE"<<endl;
    //             return -1;
    //             }*/
    //         //UPDATE 23-12-11
    //         apply(PRE_IMAGE, tmp, *NSFi, source);
    //         sourceXT -= source;
    //         source.clear();
    //         reached.clear();
    //         reached += tmp;
    //         tmp.clear();
    //     }
    //     while ((rs.getCardinality() - first) > _DIFFMIN(double));
    //     reached.clear();
    // }
    // while ((rs.getCardinality() - second) > _DIFFMIN(double));
    return 0;
}

//-----------------------------------------------------------------------------

void RSRG::IndexRS() {
    forestEVplMDD = dom->createForest(false, forest::INTEGER, forest::EVPLUS);
    indexrs = dd_edge(forestEVplMDD);
    indexrs.clear();

    apply(CONVERT_TO_INDEX_SET, rs, indexrs);
}

//-----------------------------------------------------------------------------

bool RSRG::JacobiSolver() {

    int jj = 0;

    double cardinality = rs.getCardinality();
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
    specialized_operation *VM = EXPLVECT_MATR_MULT->buildOperation(indexrs, DiagReal , indexrs);
    VM->compute(h, qold);
    specialized_operation *VM1 = EXPLVECT_MATR_MULT->buildOperation(indexrs, NSFReal, indexrs);

    int ss = 0;
    double sum = 0.0, diff = 0.0, norm = 0.0;
    bool precision = false;
    while ((ss < _iter) && (!precision)) {
        //UPDATE 23-12-1
        VM1->compute(q2, q1);

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
            /*if (cm->vectorMatrixMultiply(q2, indexrs,  q1, indexrs,*(VectNSFReal[tt]))!=0)
                {
                cerr<<"Error  vectorMatrixMultiply"<<endl;
                return -1;
                }*/
            //UPDATE 23-12-1
            specialized_operation *VMT = EXPLVECT_MATR_MULT->buildOperation(indexrs, VectNSFReal[tt], indexrs);
            VMT->compute(q2, q1);
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

//-----------------------------------------------------------------------------

// Updates the guess of the variable bounds also in the domain.
// Returns true iff any bound changed.
bool RSRG::updateGuessedBounds(int restart_count){
    // Compute effective/real bounds directly on the RS
    computeRealBounds();
    expert_domain *exp_dom = static_cast<expert_domain *>(dom);
    bool anyBoundChanged = false;
    int max_m0 = -1;
    
    // cout << "REAL BOUNDS: \n";
    // for (int i = 0; i < npl; i++)
    //       cout << tabp[i].place_name << " " << realBounds[net_to_mddLevel[i]] << "\n";
    // cout << endl << endl;

    for(int var = 0; var<npl; var++){
        if(realBounds[var] > guessedBounds[var]) {
            // The guessed bound was wrong. Increase it, enlarge the domain variable
            // and prepare to restart another iteration of saturation.
            anyBoundChanged = true;
            switch(bound_policy){ 
                case BoundGuessPolicy::M0_LINEAR:
                case BoundGuessPolicy::MAX0_LINEAR:
                    guessedBounds[var] = realBounds[var];
                    break;
                    
                case BoundGuessPolicy::M0_EXP:
                case BoundGuessPolicy::MAX0_EXP:
                case BoundGuessPolicy::LOAD_FROM_FILE:
                    guessedBounds[var] = realBounds[var] + maxVarIncr[var] * (1 << (restart_count));
                    // guessedBounds[var] += maxVarIncr[var] * (1 << (restart_count));
                    // guessedBounds[var] = std::max(guessedBounds[var], realBounds[var]);
                    break;
                    
                case BoundGuessPolicy::M0_MAX0_EXP:
                    if(restart_count == 1){
                        if(max_m0 == -1){ 
                            max_m0 = 1;
                            for(int pl=0; pl < npl; pl++) {
                                max_m0 = std::max(net_mark[pl].total, max_m0);
                            }
                        }
                        guessedBounds[var] = std::max(realBounds[var], max_m0);
                    } else {
                        guessedBounds[var] += maxVarIncr[var] * (1 << (restart_count-1));
                    }
                    break;
                default:
                    throw;
            }
            exp_dom->enlargeVariableBound(var+1, false, domainBounds(var));
        }
        if (guessedBounds[var] < realBounds[var])
            throw rgmedd_exception("Bug in updateGuessedBounds.");
    }
    // Store if we are still guessing bounds, or if we finally have the right bounds for RS
    guessing_bounds = anyBoundChanged;

    // if (anyBoundChanged) {
    //     cout << "UPDATED GUESSED BOUNDS: ";
    //     for (int i = 0; i < npl; i++)
    //          cout << guessedBounds[net_to_mddLevel[i]] << " ";
    //     cout << endl << endl;
    //     cout << "MAX VAR INCR: ";
    //     for (int i = 0; i < npl; i++)
    //          cout << maxVarIncr[net_to_mddLevel[i]] << " ";
    //     cout << endl << endl;
    //     cout << "DOMAIN BND: ";
    //     for (int i = 0; i < npl; i++)
    //          cout << domainBounds(net_to_mddLevel[i]) << " ";
    //     cout << endl << endl;
    // }
    return anyBoundChanged;
}

//-----------------------------------------------------------------------------

ostream& operator<<(ostream &out, class RSRG &rs) {
    //dd_edge::iterator i = rs.rs->begin();
    enumerator i(rs.getRS());
    int val, z = 0;
    while (i != 0) {
        out << "M" << z << "\n\t";
        for (int j = 1; j <= npl; j++) {
            val = *(i.getAssignments() + j);
            if ((val != 0))
                out << rs.placeNameOfMDDVar(j - 1) << "(" << val << ")";
        }//per ogni posto
        out << endl;
        ++i;
        ++z;
    }//per ogni marcatura
    return out;
}

//-----------------------------------------------------------------------------

void RSRG::show_markings(ostream& out, const dd_edge& e, int max_markings) {
    enumerator it(e);
    int count = 0;
    const int nvar = e.getForest()->getDomain()->getNumVariables();
    while (it) {
        // out << "  M" << left << setw(4) << count << "  ";
        out << "     ";
        const int *mark = it.getAssignments();
        for (int i = 1, num=0; i <= nvar; i++) {
            int val = *(it.getAssignments() + i);
            if(val!=0) {
                if (num++ != 0)
                    out << " + ";
                if (val != 1)
                    out << val << "*";
                out << placeNameOfMDDVar(i - 1);
            }
        }
        out << "\n";
        ++it;
        if (count++ > max_markings) {
            out << "     ..." << endl;
            return;
        }
    }
    out << flush;
}

//-----------------------------------------------------------------------------

static const int VBP_NOT_YET_VISITED = -10;
static const int VBP_GOES_TO_ZERO_TERMINAL = -12;

int RSRG::visitXBounds(const node_handle node, int visit_level, 
                        std::vector<bool> &visited,
                        std::vector<int> &nodeMaxSumTokens) 
{
    expert_forest *forest = static_cast<expert_forest *>(rs.getForest());
    if (node == forest->handleForValue(false))
        return VBP_GOES_TO_ZERO_TERMINAL;

    if (visit_level > forest->getNodeLevel(node)) { 
        // visiting intermediate levels (reduced node)
        int maxSumToks = visitXBounds(node, visit_level - 1, visited, nodeMaxSumTokens);
        if(maxSumToks == VBP_GOES_TO_ZERO_TERMINAL)
            return VBP_GOES_TO_ZERO_TERMINAL;
        int level_max = forest->getDomain()->getVariableBound(visit_level) - 1;
        realBounds.at(visit_level - 1) = std::max(realBounds.at(visit_level - 1), level_max);
        return level_max + maxSumToks;
    }

    if (node == forest->handleForValue(true))
        return 0;

    if (node >= visited.size()) {
        cerr << node << " " << visited.size() << endl;
        throw rgmedd_exception("ERROR: the cache size is smaller than the forest peak size.");
    }
    if (visited.at(node))
        return nodeMaxSumTokens.at(node);

    nodeMaxSumTokens.at(node) = VBP_GOES_TO_ZERO_TERMINAL;
    // cout << "Visit " << node << endl;
    // node_reader rnode;
    // forest->initNodeReader(rnode, node, true);
    unpacked_node *rnode = unpacked_node::newFromNode(forest, node, unpacked_node::storage_style::AS_STORED);
    assert(rnode->getLevel() >= 1 && rnode->getLevel() <= npl);

    if (rnode->isFull()) {
        for (int i = rnode->getSize() - 1; i >= 0; i--) {
            int maxSumToks_i = visitXBounds(rnode->d(i), visit_level - 1, visited, nodeMaxSumTokens);
            if (maxSumToks_i != VBP_GOES_TO_ZERO_TERMINAL) {
                realBounds.at(rnode->getLevel()-1) = std::max(realBounds.at(rnode->getLevel()-1), i);
                nodeMaxSumTokens.at(node) = std::max(nodeMaxSumTokens.at(node), i + maxSumToks_i);
            }
        }
    }
    else { // sparse node
        for (int i = rnode->getNNZs() - 1; i >= 0; i--) {
            int maxSumToks_di = visitXBounds(rnode->d(i), visit_level - 1, visited, nodeMaxSumTokens);
            if (maxSumToks_di != VBP_GOES_TO_ZERO_TERMINAL) {
                realBounds.at(rnode->getLevel()-1) = std::max(realBounds.at(rnode->getLevel()-1), rnode->i(i));
                nodeMaxSumTokens.at(node) = std::max(nodeMaxSumTokens.at(node), rnode->i(i) + maxSumToks_di);
            }
        }
    }

    unpacked_node::recycle(rnode);
    visited.at(node) = true;
    return nodeMaxSumTokens.at(node);
}

//-----------------------------------------------------------------------------

// Compute the effective bounds of each variable by visiting all the nodes of the MDD
// and by taking the maximum bound of each variable in every level.
void RSRG::computeRealBounds() {
    long numNodes = rs.getForest()->getPeakNumNodes() + 1;
    if (!running_for_MCC())
        cout << "Computing variable bounds..." << endl;
    std::vector<bool> visited(numNodes, false);
    std::vector<int> nodeMaxSumTokens(numNodes);
    int num_vars = rs.getForest()->getDomain()->getNumVariables();
    assert(dom->getNumVariables() == npl); // it must change if the number of levels is not equal to the places' number

    // Compute real bounds
    realBounds.resize(npl);
    std::fill(realBounds.begin(), realBounds.end(), 0);

    // Compute maximum # of tokens in every marking    
    maxSumTokens = visitXBounds(rs.getNode(), num_vars, visited, nodeMaxSumTokens);
}

//-----------------------------------------------------------------------------

// Compute the exact bound of the given set of places.
// The exact bound of a set of places is the maximum number of tokens that may
// appear in any tangible state for these places.
int RSRG::computeRealBoundOfVariables(const std::vector<bool> &selected_vars) const {
    long numNodes = rs.getForest()->getPeakNumNodes() + 1;
    std::vector<int> visit_cache(numNodes, VBP_NOT_YET_VISITED);
    assert(dom->getNumVariables() == npl); // it must change if the number of levels is not equal to the places' number
    assert(selected_vars.size() == npl); // it must change if the number of levels is not equal to the places' number

    return visitXBoundOfVariables(rs.getNode(), rs.getForest()->getDomain()->getNumVariables(), 
                               selected_vars, visit_cache);
}

//-----------------------------------------------------------------------------

int RSRG::visitXBoundOfVariables(const node_handle node, int visit_level,
                                 const std::vector<bool> &selected_vars,
                                 std::vector<int> &cache) const 
{
    expert_forest *forest = static_cast<expert_forest *>(rs.getForest());
    if (node == forest->handleForValue(false))
        return VBP_GOES_TO_ZERO_TERMINAL;

    if (visit_level > forest->getNodeLevel(node)) { 
        // visiting intermediate levels (reduced node)
        int vbpDown =  visitXBoundOfVariables(node, visit_level - 1, selected_vars, cache);
        if(vbpDown == VBP_GOES_TO_ZERO_TERMINAL)
            return VBP_GOES_TO_ZERO_TERMINAL;
        else
            return vbpDown + selected_vars.at(visit_level-1)?(forest->getDomain()->getVariableBound(visit_level) - 1):0;
    }

    if (node == forest->handleForValue(true))
        return 0;

    if (node >= cache.size()) {
        cerr << node << " " << cache.size() << endl;
        throw rgmedd_exception("ERROR: the cache size is smaller than the forest peak size.");
    }
    if (cache[node] != VBP_NOT_YET_VISITED)
        return cache[node];

    int node_level = forest->getNodeLevel(node);
    assert(node_level >= 1 && node_level < selected_vars.size()+1);
    bool is_selected = (selected_vars.at(node_level-1));

    unpacked_node *rnode = unpacked_node::newFromNode(forest, node, unpacked_node::storage_style::AS_STORED);
    int maximum = VBP_GOES_TO_ZERO_TERMINAL;

    if (rnode->isFull()) {
        for (int i = rnode->getSize() - 1; i >= 0; i--) {
            int val = visitXBoundOfVariables(rnode->d(i), visit_level - 1, selected_vars, cache);
            if (val != VBP_GOES_TO_ZERO_TERMINAL) {
                if (is_selected)
                    val += i;
                maximum = std::max(maximum, val);
            }
        }
    }
    else {
        for (int i = rnode->getNNZs() - 1; i >= 0; i--) {
            int val = visitXBoundOfVariables(rnode->d(i), visit_level - 1, selected_vars, cache);
            if (val != VBP_GOES_TO_ZERO_TERMINAL) {
                if (is_selected)
                    val += rnode->i(i);
                maximum = std::max(maximum, val);
            }
        }
    }

    unpacked_node::recycle(rnode);
    cache[node] = maximum;
    return cache[node];
}

//-----------------------------------------------------------------------------

// cardinality_t RSRG::count_num_fired_transitions() const {
//     std::vector<pair<int, int>> enabling; // Allocate once and reuse
//     std::vector<cardinality_t> cache; // Allocate once and reuse
//     std::vector<cardinality_t> cache2; // Allocate once and reuse

//     cardinality_t num_firings = 0;
//     for(int tr=0; tr<ntr; tr++) {
//         // Pass the pre-allocated caches to count_markings_enabling().
//         // Note: once a mpz_t objects s allocated, it never releases its storage
//         // until it is dropped. Therefore, sharing the cache between multiple
//         // calls to count_markings_enabling() significatly reduces the
//         // heap allocation stress.
//         num_firings += count_markings_enabling(tr, enabling, cache, cache2);
//     }

//     return num_firings;
// }

// //-----------------------------------------------------------------------------

// const cardinality_t& 
// RSRG::count_markings_enabling(int tr, std::vector<pair<int, int>>& enabling,
//                               std::vector<cardinality_t>& cache,
//                               std::vector<cardinality_t>& cache2) const 
// {
//     // Enabling conditions vector: resize and initialize
//     enabling.resize(npl);
//     for (int p=0; p<npl; p++)
//         enabling[p] = make_pair(0, realBounds[p] + 1);

//     // Get the enabling conditions of transition tr
//     for (Node_p inptr = GET_INPUT_LIST(tr); inptr != nullptr; inptr = NEXT_NODE(inptr)) {
//         int var = net_to_mddLevel[inptr->place_no];
//         enabling[var].first = std::max(enabling[var].first, inptr->molt);
//     }
//     for (Node_p hbptr = GET_INHIBITOR_LIST(tr); hbptr != nullptr; hbptr = NEXT_NODE(hbptr)) {
//         int var = net_to_mddLevel[hbptr->place_no];
//         enabling[var].second = std::min(enabling[var].second, hbptr->molt);
//     }

//     // for (int p=0; p<npl; p++)
//     //     cout << "  Place " << p<<": [" << enabling[p].first <<", " << enabling[p].second << "]\n";

//     // Visit cache is the # of markings at each node that fulfill the enabling conditions of tr
//     long numNodes = rs.getForest()->getPeakNumNodes() + 1;
//     assert(dom->getNumVariables() == npl);
//     cache.resize(numNodes);
//     for (int i=0; i<numNodes; i++)
//         cache[i] = -1;

//     // Secondary cache needed for reduced nodes. It avoids re-allocating the mpz_t
//     // objects when returning a mpz_t& object that does not exists in the primary cache.
//     cache2.resize(dom->getNumVariables() + 1);
    
//     return visitXTrnEnabling(rs.getNode(), rs.getForest()->getDomain()->getNumVariables(),
//                              enabling, cache, cache2);
// }

// //-----------------------------------------------------------------------------

// const cardinality_t&
// RSRG::visitXTrnEnabling(const node_handle node, int visit_level,
//                         const std::vector<pair<int, int>> &enabling,
//                         std::vector<cardinality_t> &cache,
//                         std::vector<cardinality_t> &cache2) const 
// {
//     expert_forest *forest = static_cast<expert_forest *>(rs.getForest());
//     if (node == forest->handleForValue(false))
//         return CARD0;
//     if (node == forest->handleForValue(true))
//         return CARD1;
//     if (node >= cache.size()) {
//         cerr << node << " " << cache.size() << endl;
//         throw rgmedd_exception("ERROR: the cache size is smaller than the forest peak size.");
//     }
//     const std::pair<int, int>& lvl_enab = enabling.at(visit_level - 1);

//     if (visit_level > forest->getNodeLevel(node)) { 
//         // visiting intermediate levels (reduced node)
//         // We have all values in this level.
//         int var_bound = forest->getDomain()->getVariableBound(visit_level);
//         if (var_bound != lvl_enab.second)
//             throw rgmedd_exception("should not happen");
//         int num_enabling = lvl_enab.second - lvl_enab.first;

//         // Use a pre-allocated mpz_t object from the secondary cache, avoiding reallocations
//         cardinality_t& markings_count = cache2.at(visit_level);
//         markings_count = visitXTrnEnabling(node, visit_level - 1, enabling, cache, cache2);
//         markings_count *= num_enabling;
//         return markings_count;
//         // return visitXTrnEnabling(node, visit_level - 1, enabling, cache, cache2) * num_enabling;
//     }

//     if (cache[node] >= 0)
//         return cache[node];

//     int node_level = forest->getNodeLevel(node);

//     unpacked_node *rnode = unpacked_node::newFromNode(forest, node, unpacked_node::storage_style::AS_STORED);
//     cardinality_t& markings_count = cache[node];
//     markings_count = 0;

//     if (rnode->isFull()) {
//         for (int i = lvl_enab.first; i < std::min(rnode->getSize(), lvl_enab.second); i++) {
//             markings_count += visitXTrnEnabling(rnode->d(i), visit_level - 1, enabling, cache, cache2);
//         }
//     }
//     else {
//         for (int i = 0; i < rnode->getNNZs(); i++) {
//             if (lvl_enab.first <= rnode->i(i) && rnode->i(i) < lvl_enab.second) {
//                 markings_count += visitXTrnEnabling(rnode->d(i), visit_level - 1, enabling, cache, cache2);
//             }
//         }
//     }

//     unpacked_node::recycle(rnode);  
//     // cache[node] = markings_count;
//     return markings_count;
// }

//-----------------------------------------------------------------------------

typedef std::pair<int, int>  enabling_range_t;
typedef std::map<int, enabling_range_t>  enabling_at_level_t;

// Enabling conditions of an event. It stores the ranges of the 
// variables that conditions the enabling of this event.
struct DdEvent {
    int top, bot;             // Higher & lower level in the enabling conditions
    int trn_index;            // Transition index in the Petri net
    enabling_at_level_t enab; // Enabling range at each levels

    DdEvent(int tr) : top(-1), bot(-1), trn_index(tr) { }

    // retrieve the enabling range at the specified level, eventually allocating it
    enabling_at_level_t::iterator get_range_at(int level) {
        top = (top==-1) ? level : std::max(top, level);
        bot = (bot==-1) ? level : std::min(bot, level);
        auto e = enab.find(level);
        if (e == enab.end())
            e = enab.insert(make_pair(level, make_pair(0, INT_MAX))).first;
        return e;
    }

    // get in read-only mode the enabling range at the specified level
    enabling_range_t range_at_level(int level) const {
        assert(level >= 0 && level < npl);
        auto e = enab.find(level);
        if (e != enab.end())
            return e->second;
        return make_pair(0, INT_MAX);
    }
};

ostream& operator << (ostream& os, const DdEvent& ev) {
    os << "    Event "<<tabt[ev.trn_index].trans_name << "  top: "<<ev.top<<", bot:"<<ev.bot<<endl;
    for (auto e : ev.enab)
        os << "\t\tlevel: " << e.first << "  ["<<e.second.first<<", "<<e.second.second<<")"<<endl;
    return os;
}

//-----------------------------------------------------------------------------

typedef std::list<DdEvent>  event_list_t;

struct EventsPerLevels {
    // List of events per level. Each level has a list of events, such that
    // each event e in the list has: e->top == level.
    std::vector<event_list_t> events_at_level;

    void initialize(const std::vector<int>& net_to_mddLevel) {
        events_at_level.resize(npl);
        for (int tr=0; tr<ntr; tr++) {
            DdEvent ev(tr);

            // Get the enabling conditions of transition tr
            for (Node_p inptr = GET_INPUT_LIST(tr); inptr != nullptr; inptr = NEXT_NODE(inptr)) {
                int level = net_to_mddLevel[inptr->place_no];
                auto enab_range = ev.get_range_at(level);
                enab_range->second.first = std::max(enab_range->second.first, inptr->molt);
            }
            for (Node_p hbptr = GET_INHIBITOR_LIST(tr); hbptr != nullptr; hbptr = NEXT_NODE(hbptr)) {
                int level = net_to_mddLevel[hbptr->place_no];
                auto enab_range = ev.get_range_at(level);
                enab_range->second.first = std::min(enab_range->second.first, hbptr->molt);
            }

            // Add the event at its top level
            events_at_level[ev.top].emplace_back(std::move(ev));
        }
    }
};

ostream& operator << (ostream& os, const EventsPerLevels& epl) {
    for (int lvl=npl-1; lvl>=0; lvl--) {
        if (!epl.events_at_level[lvl].empty()) {
            os << "Level "<<lvl<<endl;
            for (const DdEvent& ev : epl.events_at_level[lvl])
                os << ev;
        }
    }
    return os;
}

//-----------------------------------------------------------------------------

static const int KEY_NO_EVENTS = -2;
static const int KEY_SUM_FIRINGS = -3;

// A compact small cache that stores <key, node> -> cardinality  entries
struct compact_cardinality_cache {
    struct CacheEntry {
        int index_key = -1;         // Index key (transition index or special code)
        node_handle node_key = -1;  // Meddly node key
        cardinality_t value = 0;    // Stored value
    };
    std::vector<CacheEntry> cache;

    // Hash function that determines the cache entry for a given key pair
    inline size_t hash_fn(int index_key, node_handle node_key) const {
        size_t hash = size_t(index_key * 8960453) + size_t(node_key * 1459);// + size_t(node_key);
        return hash % cache.size();
    }

    void init_cache(size_t num_entries) {
        cache.resize(num_entries); // may throw std::bad_alloc
    }

    void store(int index_key, node_handle node_key, const cardinality_t& value) {
        size_t pos = hash_fn(index_key, node_key);
        cache[pos].index_key = index_key;
        cache[pos].node_key = node_key;
        cache[pos].value = value;
    }

    const cardinality_t* 
    find_key(int index_key, node_handle node_key) {
        size_t pos = hash_fn(index_key, node_key);
        if (cache[pos].index_key == index_key && cache[pos].node_key == node_key) {
            return &cache[pos].value;
        }
        return nullptr; // not found
    }

};

//-----------------------------------------------------------------------------

const static enabling_range_t ALWAYS_ENAB = make_pair(0, INT_MAX);

struct EventFiringsCounter {
    expert_forest *forestMDD;
    // Events organized per levels
    const EventsPerLevels *ev_lvl;

    struct PrimaryCacheEntry {
        int trn_index = -1;
        cardinality_t card = -1;
    };

    // Shared cache
    compact_cardinality_cache  cache;
    // Secondary cache (one entry per level + 1). This cache provides a cardinality_t entry 
    // for every level, to avoid allocating/freeing lots of mpz_t objects.
    std::vector<cardinality_t> working_set;

    EventFiringsCounter(const dd_edge &RS, const EventsPerLevels *e) : ev_lvl(e) {
        // card_op = MEDDLY::getOperation(CARDINALITY, f, cardinality_operant_type);
        forestMDD = static_cast<expert_forest*>(RS.getForest());
        long numNodes = RS.getNodeCount(); //->getPeakNumNodes() + 1;
        assert(forestMDD->getDomain()->getNumVariables() == npl);
        working_set.resize(forestMDD->getDomain()->getNumVariables() + 2); // Must be 2!

        // Compute the average transition span
        double sum_spans = 0.0, sum_spans2 = 0.0;
        for (auto& evt_list : ev_lvl->events_at_level) {
            for (const DdEvent& evt : evt_list) {
                size_t span = evt.top - evt.bot + 1;
                sum_spans += span;
                sum_spans2 += span * span;
            }
        }
        double avg_spans = sum_spans / ntr;
        double avg_spans2 = sum_spans2 / ntr;
        double devstd_spans = sqrt(avg_spans2 - avg_spans * avg_spans);
        size_t mult = size_t(avg_spans + devstd_spans) / 2;
        mult = std::max(mult, size_t(16));
        mult = std::min(mult, size_t(150));
        // cout << "Cache multiplier = " << mult << endl;

        cache.init_cache(numNodes * mult);
    }

    // Count the # of firings of an event *evt at the specified node level
    // This method operates in two modes: when evt is nullptr, it counts all the
    // markings below the node, much like the CARDINALITY operator. I cannot use 
    // directly the Meddly operator for cardinality because it misses the low level
    // interface.
    // In the secondary mode, when evt != nullptr, only the markings that satisfy the
    // enabling conditions of *evt are counted.
    const cardinality_t&
    count_firings_of_event(const node_handle node, const DdEvent *evt, int visit_level) 
    {
        if (visit_level == 0) {
            if (node == forestMDD->handleForValue(false))
                return CARD0;
            if (node == forestMDD->handleForValue(true))
                return CARD1;
            throw rgmedd_exception();
        }
        const enabling_range_t& lvl_enab = 
            (evt==nullptr) ? ALWAYS_ENAB : evt->range_at_level(visit_level - 1);

        const int node_level = forestMDD->getNodeLevel(node);
        cardinality_t& markings_count = working_set.at(visit_level);
        if (visit_level > node_level) { 
            // visiting intermediate levels (reduced node)
            // We have all values in this level, up to the variable bound.
            int var_bound = forestMDD->getDomain()->getVariableBound(visit_level);
            int num_enabling = std::min(var_bound, lvl_enab.second) - lvl_enab.first;

            // Use a pre-allocated mpz_t object from the secondary cache, avoiding reallocations
            markings_count = count_firings_of_event(node, evt, visit_level - 1);
            markings_count *= num_enabling;
            return markings_count;
            // return count_firings_of_event(node, evt, visit_level - 1) * num_enabling;
        }

        const int trn_key = (evt==nullptr) ? KEY_NO_EVENTS : evt->trn_index;
        const cardinality_t *p_cached_count = cache.find_key(trn_key, node);
        if (p_cached_count != nullptr)
            return *p_cached_count;

        if (evt!=nullptr) {
            if (node_level-1 < evt->bot) {
                // There are no more enabling conditions for this event. Just count
                // all states at this node.
                markings_count = count_firings_of_event(node, nullptr, visit_level);
                //cache.store(trn_key, node, markings_count);
                return markings_count;

                // MEDDLY::ct_object& result = get_mpz_wrapper();
                // dd_edge dd_node(forestMDD);
                // dd_node.set(node);
                // card_op->compute(dd_node, result);
                // MEDDLY::unwrap(result, cardinality_ref(markings_count));
                // return markings_count;
            }
        }

        markings_count = 0;
        unpacked_node *rnode = unpacked_node::newFromNode(forestMDD, node, unpacked_node::storage_style::AS_STORED);

        if (rnode->isFull()) {
            for (int i = lvl_enab.first; i < std::min(rnode->getSize(), lvl_enab.second); i++) {
                markings_count += count_firings_of_event(rnode->d(i), evt, visit_level - 1);
            }
        }
        else {
            for (int i = 0; i < rnode->getNNZs(); i++) {
                if (lvl_enab.first <= rnode->i(i) && rnode->i(i) < lvl_enab.second) {
                    markings_count += count_firings_of_event(rnode->d(i), evt, visit_level - 1);
                }
            }
        }

        unpacked_node::recycle(rnode);

        cache.store(trn_key, node, markings_count);
        return markings_count;
    }

    // Count the total number of firings of all events in a single visit.
    // individual events are visited recursively only once at their root level.
    const cardinality_t&
    count_firings_of_all_events(const node_handle node, int visit_level) {
        if (visit_level == 0)
            return CARD0;

        const int node_level = forestMDD->getNodeLevel(node);
        const event_list_t& events_at_level = ev_lvl->events_at_level.at(visit_level - 1);
        // Use visit_level+1 to avoid colliding with count_firings_of_event() in the working set
        cardinality_t& sum_firings = working_set.at(visit_level + 1);

        if (visit_level > node_level) { // visiting reduced node
            // We have all values in this level, up to the variable bound.
            int var_bound = forestMDD->getDomain()->getVariableBound(visit_level);
            // Visit recursively            
            sum_firings = count_firings_of_all_events(node, visit_level - 1);
            sum_firings *= var_bound;
            // Add firings of events rooted at this visit level
            for (const DdEvent& event : events_at_level) {
                sum_firings += count_firings_of_event(node, &event, visit_level);
            }
            return sum_firings;
        }

        const cardinality_t *p_cached_sum = cache.find_key(KEY_SUM_FIRINGS, node);
        if (p_cached_sum != nullptr)
            return *p_cached_sum;

        sum_firings = 0;

        // Count how many markings rooted at node enable the set of transitions
        // that have the enabling conditions' top level set at this level
        for (const DdEvent& event : events_at_level) {
            sum_firings += count_firings_of_event(node, &event, visit_level);
        }

        // Visit sub-levels recursively
        unpacked_node *rnode = unpacked_node::newFromNode(forestMDD, node, false /* SPARSE */);
        assert(!rnode->isFull());     

        for (int i = 0; i < rnode->getNNZs(); i++) {
            sum_firings += count_firings_of_all_events(rnode->d(i), visit_level - 1);
        }
        unpacked_node::recycle(rnode);

        cache.store(KEY_SUM_FIRINGS, node, sum_firings);
        return sum_firings;
    }
};

//-----------------------------------------------------------------------------

cardinality_t RSRG::count_num_fired_transitions_by_events() const {
    // Split transition enabling rules by events
    EventsPerLevels ev_lvl;
    ev_lvl.initialize(net_to_mddLevel);
    // cout << ev_lvl << endl << endl;

    // Initialize visit cache
    expert_forest *forestMDD = static_cast<expert_forest *>(rs.getForest());
    const int start_lvl = forestMDD->getDomain()->getNumVariables();
    EventFiringsCounter efc(getRS(), &ev_lvl);


    // cardinality_t sum = 0;
    // for (auto& list_events : ev_lvl.events_at_level) {
    //     for (DdEvent& evt : list_events) {
    //         cardinality_t enab_tt;
    //         dd_edge rs_tr(getRS().getForest());
    //         apply(POST_IMAGE, getRS(), getEventMxD(evt.trn_index), rs_tr);
    //         apply(CARDINALITY, rs_tr, cardinality_ref(enab_tt));

    //         cardinality_t enab2_tt = efc.count_firings_of_event(getRS().getNode(), &evt, start_lvl);

    //         cout << "trn: " << evt.trn_index << " " << tabt[evt.trn_index].trans_name << "  lvl:"<<evt.top<<"   " << enab_tt << " " << enab2_tt << endl;
    //         if (enab2_tt != enab_tt) {
    //             throw rgmedd_exception();
    //         }
    //         sum += enab_tt;
    //     }
    // }
    // cout << "\n sum = " << sum << endl;
    // for (int lvl=npl-1; lvl>=0; lvl--) {
    //     if (!ev_lvl.events_at_level[lvl].empty()) {
    //         for (const DdEvent& ev : ev_lvl.events_at_level[lvl]) {
    //             cout << tabt[ev.trn_index].trans_name << endl;

    //             rg_edges = efc.count_firings_of_event(rs.getNode(), &ev, start_lvl);
    //             // cout << rg_edges << endl;
    //         }
    //     }
    // }

    // Count firings
    cardinality_t total = efc.count_firings_of_all_events(rs.getNode(), start_lvl);
    return total;
}
//-----------------------------------------------------------------------------




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
//     dom->createVariablesBottomUp(bounds, npl + 1);

//     forest::policies fp(false); // false: not a relation
//     fp.setFullyReduced();
//     //fp.setCompactStorage();
//     fp.setOptimistic();
// //Create DD forests
//     forestMDD = dom->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, fp);


//     forest::policies fp1(true); // false: not a relation
//     fp1.setIdentityReduced();
//     //fp1.setCompactStorage();
//     fp1.setOptimistic();
// // Create DD forests
// //forestMDD = dom->createForest(false, forest::BOOLEAN,forest::MULTI_TERMINAL,fp);
// //forestMDD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //forestMDD->setNodeDeletion(forest::OPTIMISTIC_DELETION);

//     forestMxD = dom->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL, fp1);
// //forestMxD->setReductionRule(forest::IDENTITY_REDUCED);
// //forestMxD->setReductionRule(forest::QUASI_REDUCED);
// //forestMxD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //forestMxD->setNodeDeletion(forest::OPTIMISTIC_DELETION);

//     forestMTMxD = dom->createForest(true, forest::REAL, forest::MULTI_TERMINAL, fp1);
// //forestMTMxD->setReductionRule(forest::IDENTITY_REDUCED);
// //forestMTMxD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //forestMTMxD->setNodeDeletion(forest::OPTIMISTIC_DELETION);


// //it is initialized only when it needs
//     forestEVplMDD = NULL;


// //Create DD edge

//     rs = new dd_edge(forestMDD);
//     initMark = new dd_edge(forestMDD);
//     indexrs = NULL;
//     NSF = new dd_edge(forestMxD);
//     NSFi = new dd_edge(forestMxD);
//     NSFReal = new dd_edge(forestMTMxD);
//     DiagReal = new dd_edge(forestMTMxD);
//     VectNSFReal = (dd_edge **) malloc((ntr) * sizeof(dd_edge *));
//     for (int i = 0; i < ntr; i++) {
//         VectNSFReal[i] = new dd_edge(forestMTMxD);
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

//     fRG = dom->createForest(false, forest::REAL, forest::MULTI_TERMINAL);
// //fRG->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
// //fRG->setNodeDeletion(forest::OPTIMISTIC_DELETION);





// //Free memory
//     free(bounds);
// }

// bool RSRGAuto::MakeNextState(const int *F, const int *T, const int *H, const int tt,
//                              int AA, map <int, map <int, class ATRANSITION> > &automa) {

//     dd_edge nsf(forestMxD);
//     dd_edge nsfminus(forestMxD);//used to remove transition

//     dd_edge asf(forestMxD);
//     dd_edge asfminus(forestMxD);//used to remove transition

//     dd_edge nsfReal(forestMTMxD);
//     dd_edge nsfRealminus(forestMTMxD);//used to remove transition

//     dd_edge asfReal(forestMTMxD);
//     dd_edge asfRealminus(forestMTMxD);//used to remove transition



//     dd_edge temp(forestMxD);

//     dd_edge tempReal(forestMTMxD);



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
//                 forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal); //rate solo la prima volta
//                 nsfReal += tempReal;
//                 tempReal.clear();
//                 //to remove
//                 to[npl + 1] = from[npl + 1] = AA;
//                 forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsfminus += temp;
//                 temp.clear();
//                 forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
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
//                 forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//                 nsfReal += tempReal;
//                 tempReal.clear();

//                 //to remove
//                 to[npl + 1] = from[npl + 1] = AA;
//                 forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsfminus += temp;
//                 temp.clear();
//                 forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
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
//             forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsf += temp;
//             temp.clear();
//             forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//             nsfReal += tempReal;
//             tempReal.clear();
//             aa = 1;

//             //to remove
//             to[npl + 1] = from[npl + 1] = AA;
//             forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsfminus += temp;
//             temp.clear();
//             forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
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
//             forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsf += temp;
//             temp.clear();
//             forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
//             nsfReal += tempReal;
//             tempReal.clear();
//             aa = 1;

//             //to remove
//             to[npl + 1] = from[npl + 1] = AA;
//             forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsfminus += temp;
//             temp.clear();
//             forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
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
//         NSF += asf;
//         NSF -= asfminus;
//         (*NSFReal) += asfReal;
//         (*NSFReal) -= asfRealminus;
// //controllare--> lento se abilitato!!!!
//         //(*DiagReal)+=aDiagReal;
//         //(*VectNSFReal[tt])+=aDiagReal;
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

//     dd_edge nsf(forestMxD);
//     dd_edge asf(forestMxD);
//     dd_edge nsfReal(forestMTMxD);
//     dd_edge asfReal(forestMTMxD);

//     dd_edge temp(forestMxD);
//     dd_edge tempReal(forestMTMxD);



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
//                 forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal); //rate solo la prima volta
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
//                 forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//                 nsf += temp;
//                 temp.clear();
//                 forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
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
//             forestMxD->createEdge(&from_ptr, &to_ptr, 1, temp);
//             nsf += temp;
//             temp.clear();
//             forestMTMxD->createEdge(&from_ptr, &to_ptr, &enable, 1, tempReal);
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
//         NSF += asf;
//         (*NSFReal) += asfReal;
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
//     forestMDD->createEdge(&ins_ptr, 1, rs);
//     forestMDD->createEdge(&ins_ptr, 1, *initMark);
// #if DEBUG
//     for (int i = 0; i < npl + 1; i++) {
//         cout << "[" << i << "]" << ins[i] << " ";
//     }
//     cout << endl << endl;
// #endif



// #if DEBUG1
//     cout << "\nInitial Marking\n";
//     rs.show(stdout, 3);
// #endif
//     return 0;
// }




// bool RSRGAuto::ComputingRG(const bool &output, FILE *wngr, FILE *f_outState, const int &Top, const int &Bot, set <int> &InitSet, bool outState) {

//     enumerator i(*(this->rs));
// //dd_edge::iterator i = this->rs.begin();

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
//         forestEVplMDD->evaluate(indexrs, psource, idm);
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
//             apply(POST_IMAGE, src, *NSFReal, src);
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
//             forestEVplMDD->evaluate(indexrs, plmark, idm);
//             fRG->evaluate(src, plmark, val);
//             checkRate(val, psource);
//             if (output)
//                 cout << "\tDest: " << idm << " Rate: " << val << endl;
//             fprintf(wngr, "\t%d %f\n", idm, val);
//             ++j;
//         }
//         ++i;
//     }
//     fprintf(wngr, "%ld\n", (long int)rs.getCardinality());
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



//---------------------------------------------------------------------------------------

