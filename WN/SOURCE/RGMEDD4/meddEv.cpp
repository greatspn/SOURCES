#define _LIBCPP_DISABLE_AVAILABILITY
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
#include <set>
#include <unistd.h> // mkstemp() & close()

#include "rgmedd4.h"
#include "varorders.h"

extern ifstream fin;
extern ofstream fout;
extern int AState;
extern double _prec;
extern int _iter;
extern int out_mc;

extern bool CTMC;
MEDDLY::forest::policies::node_deletion mdd_node_del_policy = 
/**/ MEDDLY::forest::policies::node_deletion::OPTIMISTIC_DELETION;
MEDDLY::forest::policies::node_deletion mxd_node_del_policy = 
/**/ MEDDLY::forest::policies::node_deletion::OPTIMISTIC_DELETION;

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
void set_print_stat_for_gui() { print_stat_for_gui_flag = true; }


const char* rgmedd_exception::what() const noexcept { 
    return msg.c_str(); 
}

template<typename T>
void safe_delete_array(T& a) {
    if (a != nullptr) {
        delete[] a;
        a = nullptr;
    }
}

//-----------------------------------------------------------------------------

// // remove any tags (used by algebra) from a place/transition name
// static const char* filter_tags(const char* name, std::string& cache) {
//     const char* tag_sep = strchr(name, '|');
//     if (tag_sep == nullptr)
//         return name; // no tags in the object name
//     size_t n_chars = tag_sep - name;
//     cache.resize(n_chars + 1);
//     std::copy(name, tag_sep, cache.begin());
//     cache.back() = '\0';
//     // cout << "<"<<name<<"> -> <"<<cache<<">" << endl;
//     return cache.c_str();
// }


//-----------------------------------------------------------------------------

// Main initialization of Meddly: variable ordering, make domain & forests, ...
void RSRG::initialize(RsMethod _rsMethod, LrsMethod _lrsMethod,
                      const var_order_selector& initSel,
                      const long meddly_cache_size, 
                      BoundGuessPolicy _bound_policy) 
{
    this->rsMethod = _rsMethod;
    this->lrsMethod = _lrsMethod;
    this->var_order_sel = initSel;
    this->bound_policy = _bound_policy;
    this->netname = net_name;
    this->extraLvls = 1;

    init_time = clock();
    load_bounds();

    if (shouldBuildRS()) {
        if (!get_unbound_list().empty() || have_unbounded_token_count()) {
            // There are potentially unbounded places - enable coverability test
            cout << "Enabling coverability test." << endl;
            rsMethod = RSM_BFS_MONOLITHIC_COV;
        }
    }

    // Build th transition sets for SOUPS/SWIR
    trns_set.initialize();
    p_fbm = make_flow_basis_metric();

    // Map place & transition names to their indexes
    for (int i = 0; i < npl; i++)
        S2Ipl[tabp[i].place_name] = i;
    for (int i = 0; i < ntr; i++)
        S2Itr[tabt[i].trans_name] = i;

    if (!running_for_MCC()) {
        cout << "INVARIANTS:  " << get_num_invariants() << endl;
        cout << "P-SEMIFLOWS: " << get_num_Psemiflows() << endl;
        cout << "MAX INVARIANT CARD: " << get_max_invariant_coeff() << endl;  
        cout << "IS UNFOLDING: " << has_unfolding_map() << endl;
    }

    // Determine the variable ordering that will be used in the MDD/MxD domain
    cout << endl;
    print_banner(" VARIABLE ORDER ");
    determine_var_order(var_order_sel, S2Ipl, net_to_mddLevel, trns_set, *p_fbm);
    varorder_time = clock() - init_time;

    // Inverse variable permutation
    mddLevel_to_net.resize(net_to_mddLevel.size() + extraLvls);
    std::fill(mddLevel_to_net.begin(), mddLevel_to_net.end(), -1000); // not a Petri net level
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
        if(!running_for_MCC()) {
            printf("Setting MEDDLY cache to %ld entries.\n", meddly_cache_size);
        }
    }
    MEDDLY::initialize(L);
    initializeGuessedBounds();

    // cout << "DOMAIN BND: ";
    // for (int i = 0; i < npl; i++)
    //      cout << domainBounds(net_to_mddLevel[i]) << " ";
    // cout << endl << endl;

    //Create DD domain
    vector<int> domainBnd(npl + extraLvls);
    for (int i=0; i<npl; i++)
        domainBnd[i] = domainBounds(i);
    for (int i=npl; i<npl + extraLvls; i++)
        domainBnd[i] = 10; // arbitrary bound for the extra levels, to be extended
    dom = createDomainBottomUp(domainBnd.data(), npl + extraLvls);

    // cout << "DOMAIN BND: ";
    // for (int i = 0; i < npl; i++) {
    //     cout << domainBounds(net_to_mddLevel[i]) << "/" << dom->getVariableBound(net_to_mddLevel[i]+1) << " ";
    // }
    // cout << endl << endl;


    // Assign place names to each Meddly variable.
    {
        const char* lvl_name;
        std::string lvl_name_s;
        for (int lvl=0; lvl<npl + extraLvls; lvl++) {
            if (lvl < npl)
                lvl_name = tabp[net_to_mddLevel[lvl]].place_name;
            else {
                lvl_name_s = "extra_level_" + std::to_string(lvl - npl);
                lvl_name = lvl_name_s.c_str();
            }

            // Meddly pretends to have a newly allocated string with new[]
            char *pstr = new char[strlen(lvl_name) + 1];
            strcpy(pstr, lvl_name);
            dom->useVar(lvl + 1)->setName(pstr);
        }
    }

    auto set_forest_node_del_policy = [](forest::policies& _fp, forest::policies::node_deletion nd) {
        switch (nd) {
            case MEDDLY::forest::policies::node_deletion::OPTIMISTIC_DELETION:
                _fp.setOptimistic(); break;
            case MEDDLY::forest::policies::node_deletion::PESSIMISTIC_DELETION:
                _fp.setPessimistic(); break;
            case MEDDLY::forest::policies::node_deletion::NEVER_DELETE:
                _fp.setNeverDelete(); break;
        }
    };

    forest::policies mdd_fp(false); // false: not a relation
    mdd_fp.setFullyReduced();
    // mdd_fp.setQuasiReduced();
    //mdd_fp.setCompactStorage();
    set_forest_node_del_policy(mdd_fp, mdd_node_del_policy);
    forestMDD = dom->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, mdd_fp);


    forest::policies mxd_fp(true); // false: not a relation
    mxd_fp.setIdentityReduced();
    //mxd_fp.setCompactStorage();
    set_forest_node_del_policy(mxd_fp, mxd_node_del_policy);
    forestMxD = dom->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL, mxd_fp);
    //forestMxD->setReductionRule(forest::IDENTITY_REDUCED);
    //forestMxD->setReductionRule(forest::QUASI_REDUCED);
    //forestMxD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
    //forestMxD->setNodeDeletion(forest::OPTIMISTIC_DELETION);

    forestMTMxD = dom->createForest(true, forest::INTEGER, forest::MULTI_TERMINAL, mxd_fp);
    //forestMTMxD->setReductionRule(forest::IDENTITY_REDUCED);
    //forestMTMxD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
    //forestMTMxD->setNodeDeletion(forest::OPTIMISTIC_DELETION);

    forestMTMDDint = dom->createForest(false, forest::INTEGER, forest::MULTI_TERMINAL, mdd_fp);
    
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

static std::vector<int> s_bounds;
static std::vector<int> s_places_inf_bnd;
static bool s_bounds_loaded = false;
static bool s_have_unbound_list = false;

int* load_bounds() {
    if (!s_bounds_loaded) {
        s_bounds_loaded = true;

        // Load the <net_name>.bnd file
        {
            std::string bnd_file(net_name);
            bnd_file += "bnd";
            ifstream in(bnd_file.c_str());
            if (!running_for_MCC()) 
                cout << "Opening file: " << bnd_file.c_str() << (in ? " OK." : " NOT_FOUND.") << endl;
            if (in) {
                s_bounds.resize(npl, -1);
                int i = 0;
                while (in && i < npl) {
                    int lowerBoundP, upperBoundP;
                    in >> lowerBoundP >> upperBoundP;
                    // The bnd file may have both <=0 or 2147483647 as "unknown bounds".
                    if (upperBoundP >= 2147483647 || upperBoundP <= 0)
                        s_bounds[i] = -1;
                    else
                        s_bounds[i] = upperBoundP;
                    i++;
                }
                in.close();
            }
        }

        // Load the <net_name>.ilpbnd file
        {
            std::string ilpbnd_file(net_name);
            ilpbnd_file += "ilpbnd";
            ifstream in2(ilpbnd_file.c_str());
            if (!running_for_MCC()) 
                cout << "Opening file: " << ilpbnd_file.c_str() << (in2 ? " OK." : " NOT_FOUND.") << endl;
            if (in2) {
                s_have_unbound_list = true;
                if (s_bounds.empty())
                    s_bounds.resize(npl, -1);
                int num, pl;
                char buffer[64];
                in2 >> num;
                while (in2 && num--) {
                    in2 >> pl >> buffer;
                    // The bound can be an integer, "inf" for unbounded places, or "?" for unknown
                    if (isdigit(buffer[0])) {
                        int b = atoi(buffer);
                        if (s_bounds[pl] < 0 || s_bounds[pl] > b)
                            s_bounds[pl] = b;
                    }
                    else if (0 == strcmp(buffer, "inf")) {
                        s_bounds[pl] = -1;
                        s_places_inf_bnd.push_back(pl);
                        // cout << "Place " << pl << " is marked as potentially unbounded." << endl;
                    }
                    else { /* do nothing */ }
                }
            }
        }
    }
    return (s_bounds.empty() ? nullptr : s_bounds.data());
}

// Return the list of poentially unbounded places (determined using ilp bounds)
const std::vector<int>& get_unbound_list() {
    if (!s_bounds_loaded)
        throw rgmedd_exception("Load bounds before!");
    return s_places_inf_bnd;
}

bool have_unbound_list() {
    return s_have_unbound_list;
}

//-----------------------------------------------------------------------------

static bool s_have_maxtokencnt_bound = false;
static bool s_is_maxtokencnt_inf = false;

bool have_unbounded_token_count() {
    if (!s_have_maxtokencnt_bound) {
        s_have_maxtokencnt_bound = true;

        std::string maxbnd_file(net_name), bnd;
        maxbnd_file += "maxbnd";
        ifstream in(maxbnd_file.c_str());
        if (in) {
            in >> bnd;
            if (bnd == "inf")
                s_is_maxtokencnt_inf = true;
        }
    }
    return s_is_maxtokencnt_inf;
}

//-----------------------------------------------------------------------------

void RSRG::initializeGuessedBounds(){
    int max_m0 = 1;
    for (int pl=0; pl < npl; pl++) {
        max_m0 = std::max(net_mark[pl].total, max_m0);
    }

    guessState.resize(npl);
    std::fill(guessState.begin(), guessState.end(), PlaceBoundState::GUESSED);

    guessedBounds.resize(npl);

    // Make an initial guess of the variable bounds
    switch (bound_policy) {
        case BoundGuessPolicy::M0_LINEAR:
        case BoundGuessPolicy::M0_EXP:
        case BoundGuessPolicy::M0_MAX0_EXP:
            for (int pl=0; pl < npl; pl++) {
                const int lvl = net_to_mddLevel[pl];
                guessedBounds[lvl] = std::max(net_mark[pl].total, 1);
            }
            break;
            
        case BoundGuessPolicy::LOAD_FROM_FILE:
        {
            int *bounds = load_bounds();
            if (bounds == nullptr) {
                cout << "No place bound informations found." << endl;
            }
            for (int i=0; i<npl; i++) {
                int upperBoundP = (bounds ? bounds[i] : -1); // bounds[] could be null
                const int lvl = net_to_mddLevel[i];
                if (upperBoundP <= 0) { // Unknown bound
                    guessedBounds[net_to_mddLevel[i]] = max_m0;
                }
                else {
                    guessedBounds[net_to_mddLevel[i]] = 
                        (upperBoundP > 0) ? upperBoundP : std::max(net_mark[i].total, 1);
                    // This place bound is known with certainty
                    guessState[lvl] = PlaceBoundState::CERTAIN; 
                }
            }
            break;
        }

        case BoundGuessPolicy::MAX0_LINEAR:
        case BoundGuessPolicy::MAX0_EXP: 
        {
            std::fill(guessedBounds.begin(), guessedBounds.end(), max_m0);
            break;
        }

        case BoundGuessPolicy::SAFE:
        {
            // By telling that the model is safe, we assume that places are 1-bounded with certainty.
            std::fill(guessedBounds.begin(), guessedBounds.end(), 1);
            std::fill(guessState.begin(), guessState.end(), PlaceBoundState::CERTAIN);
            break;
        }
            
        default:
            cerr << "Bound Guess Policy not implemented!!!" << endl;
            exit(1);
    }

    if (print_guessed) {
        cout << "GUESSED BOUNDS: ";
        for (int i = 0; i < npl; i++) {
            const int lvl = net_to_mddLevel[i];
            cout << guessedBounds[lvl] << (guessState[lvl]==PlaceBoundState::GUESSED ? " " : "! ");
        }
        cout << endl << endl;
        // cout << "DOMAIN BND: ";
        // for (int i = 0; i < npl; i++)
        //      cout << domainBounds(net_to_mddLevel[i]) << " ";
        // cout << endl << endl;
    }
}

//-----------------------------------------------------------------------------

// Updates the guess of the variable bounds also in the domain.
// Returns true iff any bound changed.
bool RSRG::updateGuessedBounds(int restart_count){
    // Compute effective/real bounds directly on the RS
    computeRealBounds();
    expert_domain *exp_dom = static_cast<expert_domain *>(dom);
    bool changed = false;
    int max_m0 = -1;
    
    // cout << "REAL BOUNDS: \n";
    // for (int i = 0; i < npl; i++)
    //       cout << setw(15) << tabp[i].place_name << " " 
    //            << setw(5) << realBounds[net_to_mddLevel[i]] 
    //            << "    guessed=" << guessedBounds[net_to_mddLevel[i]] << "\n";
    // cout << endl << endl;

    for (int var = 0; var<npl; var++) {
        // cout << "    " << var << "   " <<  realBounds[var] << " > " << guessedBounds[var] << endl;
        if (realBounds[var] > guessedBounds[var]) {
            // Verify sanity of definite bounds
            if (guessState[var] == PlaceBoundState::CERTAIN) {
                cerr << "Wrong CERTAIN place bound! var="<<var << endl;
                throw rgmedd_exception("Wrong input place bounds!");
            }
            // The guessed bound was wrong. Increase it, enlarge the domain variable
            // and prepare to restart another iteration of saturation.
            changed = true;
            switch(bound_policy){ 
                case BoundGuessPolicy::M0_LINEAR:
                case BoundGuessPolicy::MAX0_LINEAR:
                case BoundGuessPolicy::SAFE:
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
                    if (restart_count == 1){
                        if (max_m0 == -1){ 
                            max_m0 = 1;
                            for (int pl=0; pl < npl; pl++) {
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
    guessing_bounds = changed;

    if (print_guessed) {
        if (changed) {
            cout << "GUESSED BOUNDS: ";
            for (int i = 0; i < npl; i++) {
                const int lvl = net_to_mddLevel[i];
                cout << guessedBounds[lvl] << (guessState[lvl]==PlaceBoundState::GUESSED ? " " : "! ");
            }
            cout << endl << endl;
            // cout << "MAX VAR INCR: ";
            // for (int i = 0; i < npl; i++)
            //      cout << maxVarIncr[net_to_mddLevel[i]] << " ";
            // cout << endl << endl;
            // cout << "DOMAIN BND: ";
            // for (int i = 0; i < npl; i++)
            //      cout << domainBounds(net_to_mddLevel[i]) << " ";
            // cout << endl << endl;
        }
        else {
            cout << "REAL BOUNDS: ";
            for (int i = 0; i < npl; i++)
                 cout << realBounds[net_to_mddLevel[i]] << " ";
            cout << endl << endl;
        }
    }

    return changed;
}

//-------------------------------------------------------------------------

// The domain bound for variable i (inclusive)
int RSRG::domainBounds(int i) const {
    if (i < 0 || i >= npl)
        throw rgmedd_exception("Bad variable index!");

    int var_bound;
    if (guessing_bounds) { // bounds are still guessed (not yet sure)
        if (guessState[i] == PlaceBoundState::CERTAIN)
            var_bound = guessedBounds[i]; // the place bound is known with certainty
        else
            var_bound = guessedBounds[i] + maxVarIncr[i];
    }
    else { // RS has been generated, bounds are known from the RS
        var_bound = realBounds[i];
    }

    // Return the possible number of values, including the zero, i.e. return the bound + 1
    // cout << "bound of place " << i << " is " << var_bound << endl;
    return var_bound + 1;
}

//-----------------------------------------------------------------------------

RSRG::RSRG() {
}

RSRG::~RSRG() {
    safe_delete_array(p_otf_primed_minterms);
    safe_delete_array(p_otf_unprimed_minterms);
}

//-----------------------------------------------------------------------------

bool RSRG::init_RS(const Net_Mark_p &net_mark) {
    // Create the initial marking as a dd_edge in the BDD forest
    vector<int> ins(npl + extraLvls + 1);
    for (int pl = 0 ; pl < npl; pl++) {
        ins[net_to_mddLevel[pl] + 1] = net_mark[pl].total;
    }
    for (int el = 0; el < extraLvls; el++)
        ins[npl + el + 1] = 0; 
    const int *ins_ptr = ins.data();
    rs = dd_edge(forestMDD);
    initMark = dd_edge(forestMDD); 
    forestMDD->createEdge(&ins_ptr, 1, rs);
    initMark = rs;

    // Setup the transition set (IOH) and finalize it
    trns_set.build_all_ioh_lists(net_to_mddLevel, false/* keep non-productives */);
    trns_set.compute_unique_productive_nodes(true/* keep sorted trns */, false /* no activation levels */);
    trns_set.build_trn_list_by_level();
    trns_set.finalize(); // no more rebuilds

    // Initialze also the LRS
    lrs = dd_edge(forestMDD);

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

//-----------------------------------------------------------------------------

// This helper method generates a relational tree with values at level vh and vh'
// The generated MxD has this shape:
//     1 1' ... vh     vh'         ...
//     * *   *   0 primed_vars[0]   *  T
//     * *   *   1 primed_vars[1]   *  T
//     * *   *   2 primed_vars[2]   *  T
// vh is the involved level.
// primed_vars[] contains the post-image values.
void createEdgeForRelVar(int vh, int const* primed_vars, dd_edge& out) {
    forest* mxd = out.getForest();
    const int sz = mxd->getDomain()->getVariableBound(vh + 1, false);
    const int nvars = mxd->getDomain()->getNumVariables();
    assert(vh >= 0 && vh < nvars);
    // cout << "    vh="<<vh<<" bound="<<sz<<endl;

    int *vlist[sz], *vplist[sz];
    int N = 0;
    // Create several edges with shape:
    //    1  1' 2  2' ... vh       vh'      ...
    //    *  *  *  *   *  i  primed_vars[i]  *    T 
    for (int i=0; i<sz; i++) {
        if (primed_vars[i] < 0 || primed_vars[i] >= sz)
            continue; // out of bound, do not create this edge

        vlist[N] = new int[nvars + 1];
        vplist[N] = new int[nvars + 1];
        std::fill(vlist[N], vlist[N] + nvars + 1, DONT_CARE);
        std::fill(vplist[N], vplist[N] + nvars + 1, DONT_CARE);

        vlist[N][vh + 1] = i;
        vplist[N][vh + 1] = primed_vars[i];
        // for (int n=0; n<nvars; n++)
        //     cout << " " << vlist[N][n+1]<<":"<<vplist[N][n+1];
        // cout << endl;

        N++;
    }

    mxd->createEdge(vlist, vplist, N, out);

    for (int i=0; i<N; i++) {
        delete[] vlist[i];
        delete[] vplist[i];
    }
}

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
        for (int pl = 0; pl < npl; pl++) {
            maxBound = std::max(maxBound, rsrg.domainBounds(rsrg.convertPlaceToMDDLevel(pl)));
        }
        minterm.resize(npl + rsrg.extraLvls + 1, 0);
        pMinterm = minterm.data();
        mtprime.resize(npl + rsrg.extraLvls + 1, 0); 
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

void RSRG::initializeNSFsForKnownBounds() {
    unique_ptr<RSRG::MakeNsfHelper> mh(new RSRG::MakeNsfHelper(*this));
    // clock_t timepoint;

    // Initialize the transition relations
    all_enabling_events.resize(ntr);
    events.resize(ntr);

    switch (rsMethod) {
        case RSM_SAT_MONOLITHIC:
        case RSM_BFS_MONOLITHIC:
        case RSM_BFS_MONOLITHIC_COV:
            NSF = dd_edge(forestMxD);
            break;

        case RSM_SAT_PREGEN:
            // Allocate a new pregen relation
            if (pregen_rel != NULL) {
                delete pregen_rel;
            }
            pregen_rel = new satpregen_opname::pregen_relation(forestMDD, forestMxD, forestMDD);
            break;

        case RSM_SAT_IMPLICIT:
        case RSM_SAT_OTF:
        case RSM_NONE:
            throw rgmedd_exception();
    }


    bool fast = fast_NSF_gen;
    if (ngr > 1)
        fast = false; // priority groups are not supported with fast NSF generation
    if (fast) {
        if (otf_rel == nullptr)
            buildOtfRelation();

        // Confirm all variable bounds up to the domain bounds
        for (int lvl=0; lvl<npl; lvl++) {
            int bound = domainBounds(lvl);
            // cout << "confirming bound of " << bound << " for level " << lvl << "/" << npl << endl;
            for (int tokens=0; tokens < bound; tokens++) {
                // cout << "  confirm " << (lvl+1) << ", tokens=" << tokens << endl;
                otf_rel->confirm(lvl + 1, tokens);
            }
        }
        // cout << "all bounds confirmed." << endl;

        for (int tt=0; tt<ntr; tt++) {
            // Generate the MxD from the described event
            otf_events[tt]->rebuild(); // rebuild if the new var bounds affect this event
            events[tt] = otf_events[tt]->getRoot(); // MxD of the event

            if (useMonolithicNSF())
                NSF += events[tt];
            else if (useByEventsNSF())
                pregen_rel->addToRelation(events[tt]);
        }

        return;
    }


    // Cycle from the higher to the lower priority 
    for (int curPrio = ngr, encodedCount = 0; curPrio >= 0; curPrio--) {
        for (int tt = 0; tt < ntr; tt++) {
            if (tabt[tt].pri != curPrio)
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
                for (int i = 0; i < domainBounds(mddPl); i++) { 
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
                for (int i = 0; i < domainBounds(mddPl); i++) {
                    if (mh->pIsEnabledFor[i] /*&& i <= guessedBounds[mddPl]*/)
                        mh->pPlaceFunction[i] = (i>=Fpl) ? (i + delta) : -1;
                    else
                        mh->pPlaceFunction[i] = -1;
                }
                // timepoint = clock();
                createEdgeForRelVar(mddPl, mh->pPlaceFunction, firingAtVar);
                // createEdgeForRelVar_vanilla(mddPl, mh->pPlaceFunction, firingAtVar, forestMTMxD);
                // mh->clkCrEdg3 += clock() - timepoint;

                // timepoint = clock();
                nsf_event *= firingAtVar;
                // mh->clkFiring += clock() - timepoint;

                // cout << "   var="<<mddPl<<endl;
                // MEDDLY::ostream_output stdout_wrap(cout);
                // firingAtVar.show(stdout_wrap, 2);
                // cout << endl;
            }

            // Exclude the states in which higher-priority transitions are enabled
            if (tabt[tt].pri < ngr) {
                for (int hptr=0; hptr<ntr; hptr++) {
                    if (tabt[tt].pri < tabt[hptr].pri) {
                        // Remove the states where hptr is enabled from the enabling of tt
                        enabling -= all_enabling_events[hptr];
                    }
                }
            }
            
            // Combine enabling conditions and firing consequences into the NSF of tt
            // timepoint = clock();
            nsf_event *= enabling;
            // mh->clkComb += clock() - timepoint;

            if (useMonolithicNSF()) {
                NSF += nsf_event;
            } 
            else if (useByEventsNSF()) { // saturation by events
                // timepoint = clock();
                pregen_rel->addToRelation(nsf_event);
                // mh->clkAddRel += clock() - timepoint;
            }

            // Keep the NSF of this event, for later use
            events[tt] = nsf_event;
            all_enabling_events[tt] = enabling;
        }
    }

    // cout << "clkEnab=  " << double(mh->clkEnab) / CLOCKS_PER_SEC << endl;
    // cout << "clkFiring=" << double(mh->clkFiring) / CLOCKS_PER_SEC << endl;
    // cout << "clkComb=  " << double(mh->clkComb) / CLOCKS_PER_SEC << endl;
    // cout << "clkAddRel=" << double(mh->clkAddRel) / CLOCKS_PER_SEC << endl;
    // cout << "clkCrEdg1=" << double(mh->clkCrEdg1) / CLOCKS_PER_SEC << endl;
    // cout << "clkCrEdg2=" << double(mh->clkCrEdg2) / CLOCKS_PER_SEC << endl;
    // cout << "clkCrEdg3=" << double(mh->clkCrEdg3) / CLOCKS_PER_SEC << endl;
}

//-----------------------------------------------------------------------------
// Encoding of the Petri net transitions for sat implicit
//-----------------------------------------------------------------------------

#define NOT_KNOWN     -2
#define OUT_OF_BOUNDS -1

// changed class declaration because of https://github.com/asminer/meddly/commit/ca1cfb14668a2414eed8f7ee98c8d8d257bf14a4
class impl_firing_subevent : public MEDDLY::relation_node { 
    RSRG            *rs;    // Update guessed bounds on the fly
    int              trn;   // Transition this sub-event is part of
    const var_ioh_t *ioh;   // Enabling/firing conditions of a single variable(level)
public:
    impl_firing_subevent(unsigned long signature,   // Unique signature of this instance in the DAG
                         int level,                 // MDD level
                         rel_node_handle down,      // handle of the DAG node below
                         RSRG *_rs,                 // RGMEDD object
                         int _trn,                  // GreatSPN transition index
                         const var_ioh_t *_ioh)     // Encoded IOH
    : relation_node(signature, level + 1 /* meddly levels are 1-based */, down), rs(_rs), trn(_trn), ioh(_ioh)
    { assert(level == ioh->level); }

    // If the variable at this level has value i, what should the new value be?
    // This method works with a pre-computed array of "token updates".
    // We need to: resize the array if it is too small (new token bound observed) and
    // set the produced value, if any.
    long nextOf(long i) override {
        if (i >= getPieceSize()) // Increase the variable bound & the pre-computed update array
            expandTokenUpdate(i);

        if (getTokenUpdate()[i] == NOT_KNOWN) { // Pre-computed value is missing
            long result = i;
            if (ioh->enabled_for (i)) 
                result = ioh->firing_effect(i);
            else 
                result = OUT_OF_BOUNDS; // does not fire
            // cout << "nextOf("<<i<<") trn="<<trn<<" lvl="<<ioh->level<<" ioh="<<*ioh<<"  result="<<result<<endl;
            setTokenUpdateAtIndex(i, result);
            rs->observeVariableValue(ioh->level, i);
        }

        return getTokenUpdate()[i];
    }

    bool equals(const relation_node* n) const override { return false; }
};

//---------------------------------------------------------------------------------------

void RSRG::buildImplicitRelation() {
    impl_rel = new satimpl_opname::implicit_relation(forestMDD, forestMDD, forestMDD);
    impl_rel->setAutoDestroy(false);
    // Associate all unique IOH nodes to the registered rel_node_handles
    std::vector<rel_node_handle> registered_handles(trns_set.signatures, -1);
    impl_trn_handle.resize(ntr);

    // Register all transitions
    for (const trans_span_t* p_trn : trns_set.sorted_trns) {
        rel_node_handle previous_handle = 1; // end of DAG
        // cout << "\nTransition " << p_trn->trn << endl;
        // Register all IOH of this transition
        for (const var_ioh_t* p_ioh : p_trn->sorted_entries) {
            // Is this the top node in the IOH chain?
            bool top = (p_ioh == p_trn->sorted_entries.back());
            assert(0 <= p_ioh->level && p_ioh->level < npl);

            if (p_ioh->is_unique) {
                // Allocate a new sub-event
                // cout << "Unique node: sig="<<p_ioh->signature<<" lvl="<<p_ioh->level<<" prev_handle="<<previous_handle<<"  ioh="<<*p_ioh<<"  top="<<top;
                // TODO: memory leak!
                impl_firing_subevent *p_fse = new impl_firing_subevent(p_ioh->signature+10, p_ioh->level, previous_handle, this, p_trn->trn, p_ioh);
                previous_handle = impl_rel->registerNode(top, p_fse);
                assert(registered_handles[p_ioh->signature] < 0);
                registered_handles[p_ioh->signature] = previous_handle;
                // cout << "   registered as "<<previous_handle<<endl;
            }
            else {
                // A sub-chain with the same IOH conditions already exists for some other transition. Share it.
                previous_handle = registered_handles[p_ioh->signature];
                // cout << "Shared node: sig="<<p_ioh->signature<<"  handle="<<previous_handle<<endl;
                assert(previous_handle > 0);
            }
            // Save the top handle for later use
            if (top)
                impl_trn_handle[p_trn->trn] = previous_handle;
        }
    }
    impl_sat = SATURATION_IMPL_FORWARD->buildOperation(impl_rel);
}

//-----------------------------------------------------------------------------
// Encoding of the firing rules for the on-the-fly saturation
//-----------------------------------------------------------------------------

class RSRG::otf_subevent : public satotf_opname::subevent {
  public:
    otf_subevent(RSRG* _rs, const var_ioh_t* _p_ioh, int _trn,
              int* subevent_vars, int n_subevent_vars, bool is_firing)
    : satotf_opname::subevent(_rs->getForestMxD(), subevent_vars, n_subevent_vars, is_firing), 
      rs(_rs), p_ioh(_p_ioh), trn(_trn)
    { }

    ~otf_subevent() { }

    // The method we have to overwrite to confirm a new value for variable @v
    virtual void confirm(satotf_opname::otf_relation &rel, int v, int tokens) {
        assert(v == p_ioh->level + 1);
        rs->observeVariableValue(p_ioh->level, tokens);
        if (isFiring()) {
            // Determine firing effect
            // cout << "Confirming (firing): (" << tabp[p_ioh->place].place_name << ", " << tokens << ")" << endl;

            if (!p_ioh->enabled_for (tokens)) 
                return; // does not fire
            int next_tokens = p_ioh->firing_effect(tokens);

            // Add the new firing effect minterm to the relation
            initializeSharedMinterms();
            rs->p_otf_unprimed_minterms[v] = tokens;
            rs->p_otf_primed_minterms[v] = next_tokens;
            addMinterm(rs->p_otf_unprimed_minterms, 
                       rs->p_otf_primed_minterms);
            deinitializeSharedMinterms();
        }
        else { 
            // Test enabling condition
            // cout << "Confirming (enabling): (" << tabp[p_ioh->place].place_name << ", " << tokens << ")" << endl;

            if (!p_ioh->enabled_for (tokens))
                return; // Not enabled

            // The token value is an enabling minterm. Add it to the relation.
            initializeSharedMinterms();
            rs->p_otf_unprimed_minterms[v] = tokens;
            rs->p_otf_primed_minterms[v] = DONT_CARE;
            addMinterm(rs->p_otf_unprimed_minterms, 
                       rs->p_otf_primed_minterms);
            deinitializeSharedMinterms();
        }
    }

    void initializeSharedMinterms() {
        for (const var_ioh_t& elem : rs->trns_set.trns[trn].entries) {
            rs->p_otf_primed_minterms[elem.level + 1] = DONT_CARE;
        }
    }

    void deinitializeSharedMinterms() {
        for (const var_ioh_t& elem : rs->trns_set.trns[trn].entries) {
            rs->p_otf_primed_minterms[elem.level + 1] = DONT_CHANGE;
        }
        rs->p_otf_unprimed_minterms[p_ioh->level + 1] = DONT_CARE;
        rs->p_otf_primed_minterms[p_ioh->level + 1] = DONT_CHANGE;
    }

protected:
    RSRG* rs;
    const var_ioh_t* p_ioh; // The enabling condition for a place
    int trn;                // Transition of this subevent
};

//-----------------------------------------------------------------------------

void RSRG::buildOtfRelation() {
    // Initialize the shared minterms
    p_otf_unprimed_minterms = new int[npl + extraLvls + 1];
    p_otf_primed_minterms = new int[npl + extraLvls + 1];

    p_otf_unprimed_minterms[0] = p_otf_primed_minterms[0] = 0;
    for (int i = 0; i < npl + extraLvls; i++) {
        p_otf_unprimed_minterms[i+1] = DONT_CARE;
        p_otf_primed_minterms[i+1] = DONT_CHANGE;
    }

    // Build all the OTF events
    otf_events.resize(ntr);
    // for (ssize_t st=trns_set.sorted_trns.size()-1; st>=0; st--) {
    for (size_t st=0; st<trns_set.sorted_trns.size(); st++) {
        const trans_span_t& tr = *trns_set.sorted_trns[st];
        const size_t N = tr.sorted_entries.size();
        size_t num_subevents = 0;
        otf_subevent *subevents[2 * N];

        // Allocate all the enabling & firing sub-events
        for (size_t i=0; i<tr.sorted_entries.size(); i++) {
            const var_ioh_t *p_ioh = tr.sorted_entries[i];

            if (p_ioh->has_input() || p_ioh->has_inhib()) {
                // Add a new enabling condition at a given MDD level
                int se_var[1];
                se_var[0] = p_ioh->level + 1; // MDD level for this variable
                otf_subevent* se = new otf_subevent(this, p_ioh, tr.trn, &se_var[0], 1, false /* enabling */);

                subevents[ num_subevents++ ] = se;
            }
            if (p_ioh->has_input() || p_ioh->has_output()) {
                // Add a new firing condition at a given MDD level
                int se_var[1];
                se_var[0] = p_ioh->level + 1; // MDD level for this variable
                otf_subevent* se = new otf_subevent(this, p_ioh, tr.trn, &se_var[0], 1, true /* firing */);

                subevents[ num_subevents++ ] = se;
            }
        }

        // Allocate the Meddly OTF event
        satotf_opname::event* ev;
        ev = new satotf_opname::event((satotf_opname::subevent**)(&subevents[0]), num_subevents);
        otf_events[tr.trn] = ev;
    }

    // Create the OTF transition relation operator
    otf_rel = new satotf_opname::otf_relation(forestMDD, forestMxD, forestMDD, otf_events.data(), otf_events.size());
    otf_sat = SATURATION_OTF_FORWARD->buildOperation(otf_rel);
}

//-----------------------------------------------------------------------------

dd_edge RSRG::testerForCoverabilityOfPlace(int pl) {
    std::vector<int> unprimed(npl+extraLvls+1, DONT_CARE), primed(npl+extraLvls+1, DONT_CHANGE);
    primed[ net_to_mddLevel[pl] + 1 ] = 0;
    dd_edge tester(forestMxD);
    const int *p_unprimed = unprimed.data(), *p_primed = primed.data();
    forestMxD->createEdge(&p_unprimed, &p_primed, 1, tester);
    return tester;
}

//-----------------------------------------------------------------------------

bool RSRG::buildRS() {
    assert(!rs_is_infinite);
    clock_t startGlobal, endGlobal;
    startGlobal = clock();

    switch (rsMethod) {
        case RSM_BFS_MONOLITHIC:
        case RSM_BFS_MONOLITHIC_COV: {
            bool coverability_test = (rsMethod == RSM_BFS_MONOLITHIC_COV);
            std::vector<std::pair<dd_edge, int>> covT;
            if (coverability_test) {
                if (have_unbound_list()) { // Test only the places in the list for coverability 
                    auto&& unb_plcs = get_unbound_list();
                    for (int pl : unb_plcs)
                        covT.push_back(make_pair(testerForCoverabilityOfPlace(pl), pl));
                }
                else { // Test all places for coverability
                    for (int pl=0; pl<npl; pl++)
                        covT.push_back(make_pair(testerForCoverabilityOfPlace(pl), pl));
                }
            }
            int num_iters = 0;
            while (!rs_is_infinite) {
                dd_edge current(rs);
                apply(POST_IMAGE, rs, NSF, rs);
                apply(UNION, current, rs, rs);
                num_iters++;
                // if ((num_iters % 10) == 0)
                if (!running_for_MCC())
                    cout << num_iters << " " << flush;
                if (current == rs)
                    break;
                if (coverability_test && (1 == (num_iters % 2))) {
                    if (!running_for_MCC())
                        cout << "C " << flush;
                    cardinality_t rs_card, rs_omega_card;
                    apply(CARDINALITY, rs, cardinality_ref(rs_card));
                    for (auto&& T : covT) {
                        dd_edge rs_omega(forestMDD);
                        apply(POST_IMAGE, rs, T.first, rs_omega);
                        apply(CARDINALITY, rs_omega, cardinality_ref(rs_omega_card));
                        if (rs_card != rs_omega_card) {
                            cout << "coverability test failed!!"<<endl; // for place "<<tabp[T.second].place_name<<endl;
                            rs_is_infinite = true;
                            break;
                        }
                    }
                }
            }
            cout << "Convergence in " << num_iters << " steps." << endl;
            break;
        }

        case RSM_SAT_MONOLITHIC: {
            apply(REACHABLE_STATES_DFS, rs, NSF, rs);
            // apply(REVERSE_REACHABLE_DFS, rs, NSF, rs);
            break;
        }

        case RSM_SAT_PREGEN: {
            assert(pregen_rel);
            // cout << "Finalizing SAT-pregen operator..." << endl;
            pregen_rel->finalize();

            pregen_sat = SATURATION_FORWARD->buildOperation(pregen_rel);
            pregen_sat->compute(rs, rs);
            break;
        }

        case RSM_SAT_IMPLICIT: {
            buildImplicitRelation();
            impl_sat->compute(initMark, rs);

            // dd_edge mxd = impl_rel->buildMxdForest();
            // MEDDLY::ostream_output stdout_wrap(cout);
            // mxd.show(stdout_wrap, 2);

            break;
        }

        case RSM_SAT_OTF: {
            buildOtfRelation();
            assert(otf_sat);

            // Initialize transition relation to be able to fire in the initial marking
            otf_rel->confirm(initMark);
            // cout << "\n\n\nINIT MARK CONFIRM ENDS.\n\n\n";

            // for (int lvl=0; lvl<npl; lvl++) {
            //     for (int tokens=0; tokens<=guessedBounds[lvl]; tokens++)
            //         otf_rel->confirm(lvl + 1, tokens);
            // }

            // cout << "\n\n\nCONFIRM ENDS.\n\n\n";

            // Build the reachability set with Saturation
            otf_sat->compute(initMark, rs);

            break;
        }

        case RSM_NONE:
            break;
    }

    if (rs_is_infinite) {
        cout << " The state space is infinite." << endl;
        rs = dd_edge(forestMDD);  // clear rs and make it unusable.
    }

    endGlobal = clock();
    if (!running_for_MCC())
        cout << "RS generation time: " << ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC << endl;

    return rs_is_infinite;
}

//-----------------------------------------------------------------------------

bool RSRG::buildLRSbyPBasisConstraints() {
    // Verify that all places have a known bound
    size_t nguess = 0;
    for (int i=0; i<npl; i++)
        if (guessState[i] == PlaceBoundState::GUESSED)
            nguess++;
    if (nguess > 0) {
        cout << nguess << " places have an unknown bound. Cannot do constraint satisfaction." << endl;
        return false;
    }
    if (!load_flow_basis()) {
        cout << "Missing P-flow basis. Cannot do constraint satisfaction." << endl;
        return false;
    }
    // if (!is_net_covered_by_flow_basis()) {
    //     cout << "Some places are not covered by the P-flow basis. Cannot do constraint satisfaction." << endl;
    //     return false;
    // }

    expert_forest *forest = static_cast<expert_forest *>(lrs.getForest());
    lrs.set(forest->handleForValue(true));

    // Reduce the potential state space accoding to each variable bound
    // This is needed because the next step works with the Meddly domain bounds,
    // which could be different from the variable bound due to bound guessing.    
    for (int lvl=0; lvl<npl; lvl++) {
        int var_bound = dom->getVariableBound(lvl + 1);
        int real_bound = domainBounds(lvl);
        // cout << real_bound << " / " << var_bound << endl;

        // Encode the constraint:   var < bound
        bool terms[var_bound];
        for (int i=0; i<var_bound; i++)
            terms[i] = (i < real_bound); // exclude every value above the real bound
        dd_edge boundDD(forestMDD);
        forestMDD->createEdgeForVar(lvl + 1, false, terms, boundDD);

        // Remove the states above the bound
        lrs *= boundDD;
    }

    // Empty terms for edges of constant value
    std::vector<int> dcterms(npl + extraLvls + 1);
    std::fill(dcterms.begin(), dcterms.end(), DONT_CARE);
    const int* p_dcterms = dcterms.data();

    // Evaluate one constraint at a time from the P-flow basis
    const flow_basis_t& B = get_flow_basis();
    for (const sparse_vector_t& pflow : B) {
        dd_edge constrDD(forestMTMDDint);
		// changed from int to long due to line 1324:createEdge prototype changed
        long m0_pflow = 0;

        // Initialize all the weighted place terms
        for (size_t i=0; i<pflow.nonzeros(); i++) {
            int plc = pflow.ith_nonzero(i).index;
            int lvl = convertPlaceToMDDLevel(plc);
            int card = pflow.ith_nonzero(i).value;
            int bound = dom->getVariableBound(lvl + 1);
            // int real_bound = domainBounds(lvl);
            // cout << "plc="<<plc<<" lvl="<<lvl<<" card="<<card<<" bound="<<bound<<endl;

            m0_pflow += card * net_mark[plc].total; // m0 * pflow

            // Create the edge that adds the weighted token count of the variable
            int term_vals[bound];
            for (int i=0; i<bound; i++)
                term_vals[i] = i * card;
            dd_edge tokens_in_var(forestMTMDDint);

			// cast to long* since prototypes accept bool, long, float (TODO TEST)
            forestMTMDDint->createEdgeForVar(lvl + 1, false, (long*)term_vals, tokens_in_var);

            // Add the value to the constraint expression
            constrDD += tokens_in_var;
        }
        // cout << endl;

        // Create the terminal that ends in m0_pflow
        dd_edge const_val(forestMTMDDint);
        forestMTMDDint->createEdge(&p_dcterms, &m0_pflow, 1, const_val);

        // The weighted sum of the places must be equal to (m0 * pflow)
        apply(MEDDLY::EQUAL, constrDD, const_val, constrDD);

        // Move the p-flow constraint in the state space MDD
        dd_edge stateConstrDD(forestMDD);
        apply(MEDDLY::COPY, constrDD, stateConstrDD);

        // Remove from the state space all the states that violate the invariant
        apply(MEDDLY::INTERSECTION, lrs, stateConstrDD, lrs);
    }

    return true;
}

//-----------------------------------------------------------------------------

bool RSRG::buildLRS() {
    switch (lrsMethod) {
        case LRSM_NONE:
            lrs.set(static_cast<expert_forest *>(lrs.getForest())->handleForValue(false));
            return false;

        case LRSM_PBASIS_CONSTRAINTS:
            return buildLRSbyPBasisConstraints();
    }
    return false;
}

//-----------------------------------------------------------------------------

// require that RSRG::events vector has been initialized 
void RSRG::prepareEventMxDsForCTL() {
    assert(!RSRG::useMonolithicNSF());
    assert(events.size() == ntr
           || RSRG::rsMethod == RSM_SAT_OTF
           || RSRG::rsMethod == RSM_SAT_IMPLICIT);
}

//-----------------------------------------------------------------------------

void RSRG::prepareNSFforCTL() {
    //assert(RSRG::useMonolithicNSF());
    buildMonolithicNSF();
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

ostream& operator<<(ostream &out, class RSRG &rs) {
    //dd_edge::iterator i = rs.rs->begin();
    enumerator i(rs.getRS());
    int mddVar, z = 0;
    while (i != 0) {
        out << "M" << z << "\n\t";
        for (int j = 1; j <= rs.getDomain()->getNumVariables(); j++) {
            mddVar = *(i.getAssignments() + j);
            if ((mddVar != 0)) {
                out << rs.nameOfMddVar(j - 1) << "(" << mddVar << ")";

            }
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
            if (val!=0) {
                if (num++ != 0)
                    out << " + ";
                if (val != 1)
                    out << val << "*";
                out << nameOfMddVar(i - 1);
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
                       std::vector<bool> &visited, std::vector<int> &nodeMaxSumTokens) 
{
    expert_forest *forest = static_cast<expert_forest *>(rs.getForest());
    if (node == forest->handleForValue(false))
        return VBP_GOES_TO_ZERO_TERMINAL;

    const int node_level = forest->getNodeLevel(node);

    if (visit_level > node_level) { 
        // if (strcmp(tabp[mddLevel_to_net[visit_level - 1]].place_name, "NADPH") == 0) {
        //     cout << "skipping level " << visit_level << endl;
        // }
        // visiting intermediate levels (reduced node)
        int maxSumToks = visitXBounds(node, visit_level - 1, visited, nodeMaxSumTokens);
        if (maxSumToks == VBP_GOES_TO_ZERO_TERMINAL)
            return VBP_GOES_TO_ZERO_TERMINAL;
        int level_max = forest->getDomain()->getVariableBound(visit_level) - 1;
        // cout << "   " << level_max << endl;
        realBounds.at(visit_level - 1) = std::max(realBounds.at(visit_level - 1), level_max);
        realLowerBounds.at(visit_level - 1) = 0;

        int level_contrib = (visit_level - 1 < npl) ? level_max : 0;
        return level_contrib + maxSumToks;
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
    //  cout << "Visit " << node << " at level " << forest->getNodeLevel(node) << endl;
    unpacked_node *rnode = unpacked_node::newFromNode(forest, node, unpacked_node::storage_style::AS_STORED);
    assert(rnode->getLevel() >= 1 && rnode->getLevel() <= realBounds.size());

    if (rnode->isFull()) {
        for (int i = rnode->getSize() - 1; i >= 0; i--) {
            int maxSumToks_i = visitXBounds(rnode->d(i), visit_level - 1, visited, nodeMaxSumTokens);
            if (maxSumToks_i != VBP_GOES_TO_ZERO_TERMINAL) {
                realBounds.at(node_level - 1) = std::max(realBounds.at(node_level - 1), i);
                realLowerBounds.at(node_level - 1) = std::min(realLowerBounds.at(node_level - 1), i);
                // Nodes contribute to max computation only if they are levels of places
                // Extra levels do not contribute to the max token sum.
                int node_contrib = (node_level - 1 < npl) ? i : 0;
                nodeMaxSumTokens.at(node) = std::max(nodeMaxSumTokens.at(node), 
                                                     node_contrib + maxSumToks_i);
            }
        }
    }
    else { // sparse node
        for (int i = rnode->getNNZs() - 1; i >= 0; i--) {
            int maxSumToks_di = visitXBounds(rnode->d(i), visit_level - 1, visited, nodeMaxSumTokens);
            if (maxSumToks_di != VBP_GOES_TO_ZERO_TERMINAL) {
                realBounds.at(node_level - 1) = std::max(realBounds.at(node_level - 1), (int)rnode->i(i));
                realLowerBounds.at(node_level - 1) = std::min(realLowerBounds.at(node_level - 1), (int)rnode->i(i));
                int node_contrib = (node_level - 1 < npl) ? (int)rnode->i(i) : 0;
                nodeMaxSumTokens.at(node) = std::max(nodeMaxSumTokens.at(node), 
                                                     node_contrib + maxSumToks_di);
            }
        }
    }

    // if (strcmp(tabp[mddLevel_to_net[node_level - 1]].place_name, "NADPH") == 0) {
    //     cout << "visiting " << node << " at level: " << visit_level << "\n   ";
    //     if (rnode->isFull()) {
    //         for (int i = rnode->getSize() - 1; i >= 0; i--) {
    //             cout << i << ":" << rnode->d(i) << " ";
    //         }
    //     }
    //     else { // sparse node
    //         for (int i = rnode->getNNZs() - 1; i >= 0; i--) {
    //             cout << rnode->i(i) << ":" << rnode->d(i) << " ";
    //         }
    //     }
    //     cout << endl;
    // }

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
    assert(dom->getNumVariables() == npl + extraLvls); // it must change if the number of levels is not equal to the places' number

    // Compute real bounds
    realBounds.resize(npl + extraLvls);
    std::fill(realBounds.begin(), realBounds.end(), 0);

    realLowerBounds.resize(npl + extraLvls);
    std::fill(realLowerBounds.begin(), realLowerBounds.end(), INT_MAX);

    // Compute maximum # of tokens in every marking    
    maxSumTokens = visitXBounds(rs.getNode(), num_vars, visited, nodeMaxSumTokens);
}

//-----------------------------------------------------------------------------

static const pair<int, int> VBPV_NOT_YET_VISITED = make_pair(INT_MAX, -10);
static const pair<int, int> VBPV_GOES_TO_ZERO_TERMINAL = make_pair(INT_MAX, -12);

// Compute the exact bound of the given set of places.
// The exact bound of a set of places is the maximum number of tokens that may
// appear in any tangible state for these places.
pair<int, int>
RSRG::computeRealBoundOfVariables(const std::vector<bool> &selected_vars) const {
    long numNodes = rs.getForest()->getPeakNumNodes() + 1;
    std::vector<pair<int, int>> visit_cache(numNodes, VBPV_NOT_YET_VISITED);
    assert(dom->getNumVariables() == npl + extraLvls); // it must change if the number of levels is not equal to the places' number
    assert(selected_vars.size() == dom->getNumVariables());

    size_t num_levels = rs.getForest()->getDomain()->getNumVariables();
    return visitXBoundOfVariables(rs.getNode(), num_levels, 
                                  selected_vars, visit_cache);
}

//-----------------------------------------------------------------------------

pair<int, int>
RSRG::visitXBoundOfVariables(const node_handle node, int visit_level,
                             const std::vector<bool> &selected_vars,
                             std::vector<pair<int, int>> &cache) const 
{
    expert_forest *forest = static_cast<expert_forest *>(rs.getForest());
    if (node == forest->handleForValue(false))
        return VBPV_GOES_TO_ZERO_TERMINAL;

    if (visit_level > forest->getNodeLevel(node)) { 
        // visiting intermediate levels (reduced node)
        pair<int, int> vbpDown = visitXBoundOfVariables(node, visit_level - 1, 
                                                        selected_vars, cache);
        if (vbpDown == VBPV_GOES_TO_ZERO_TERMINAL)
            return VBPV_GOES_TO_ZERO_TERMINAL;
        else {
            if (selected_vars.at(visit_level-1)) // selected level is full
                vbpDown.second += forest->getDomain()->getVariableBound(visit_level) - 1;
            return vbpDown;
        }
    }

    if (node == forest->handleForValue(true))
        return make_pair(0, 0);

    if (node >= cache.size()) {
        cerr << node << " " << cache.size() << endl;
        throw rgmedd_exception("ERROR: the cache size is smaller than the forest peak size.");
    }
    if (cache[node] != VBPV_NOT_YET_VISITED)
        return cache[node];

    int node_level = forest->getNodeLevel(node);
    assert(node_level >= 1 && node_level < selected_vars.size()+1);
    bool is_selected = (selected_vars.at(node_level-1));

    unpacked_node *rnode = unpacked_node::newFromNode(forest, node, unpacked_node::storage_style::AS_STORED);
    pair<int, int> bound = VBPV_GOES_TO_ZERO_TERMINAL;

    if (rnode->isFull()) {
        for (int i = rnode->getSize() - 1; i >= 0; i--) {
            pair<int, int> val = visitXBoundOfVariables(rnode->d(i), visit_level - 1, selected_vars, cache);
            if (val != VBPV_GOES_TO_ZERO_TERMINAL) {
                int incr = is_selected ? i : 0;
                bound.first = std::min(bound.first, val.first + incr);
                bound.second = std::max(bound.second, val.second + incr);
                // bound = std::max(bound, val);
            }
        }
    }
    else {
        for (int i = rnode->getNNZs() - 1; i >= 0; i--) {
            pair<int, int> val = visitXBoundOfVariables(rnode->d(i), visit_level - 1, selected_vars, cache);
            if (val != VBPV_GOES_TO_ZERO_TERMINAL) {
                int incr = is_selected ? rnode->i(i) : 0;
                bound.first = std::min(bound.first, val.first + incr);
                bound.second = std::max(bound.second, val.second + incr);
            }
        }
    }

    unpacked_node::recycle(rnode);
    cache[node] = bound;
    return cache[node];
}

//-----------------------------------------------------------------------------

// computes the ma/min bound on any marking of the RS 
// on a place of the original net (i.e. unrolls the unfolding)
pair<int, int>
RSRG::computeTokenBoundForAnyPlace() {
    if (realLowerBounds.empty())
        throw rgmedd_exception("Compute the RS first");

    const bool is_unfolded = has_unfolding_map();
    std::vector<int> sel_vars;
    std::vector<bool> selected_vars_mask;
    pair<int, int> out_bnd = make_pair(INT_MAX, -1);
    size_t num_places = (is_unfolded ? get_unfolding_map().pl_unf.size() : npl);

    for (size_t pl = 0; pl<num_places; pl++) {
        // get the set of places where we need to compute the bound
        if (is_unfolded) {
            sel_vars = get_unfolding_map().pl_unf[pl].second;
        }
        else {
            sel_vars.resize(1);
            sel_vars[0] = pl;
        }
        // convert places to levels
        for (size_t i=0; i<sel_vars.size(); i++) {
            assert(sel_vars[i] >= 0 && sel_vars[i] < npl);
            sel_vars[i] = net_to_mddLevel[sel_vars[i]];
        }

        // compute bounds
        pair<int, int> pl_bnd;
        if (sel_vars.size() == 1) {
            pl_bnd = make_pair(realLowerBounds[sel_vars[0]],
                               realBounds[sel_vars[0]]);
        }
        else {
            selected_vars_mask.resize(dom->getNumVariables());
            std::fill(selected_vars_mask.begin(), selected_vars_mask.end(), false);
            for (int i=0; i<sel_vars.size(); i++) {
                assert(selected_vars_mask[ sel_vars[i] ] == false);
                selected_vars_mask[ sel_vars[i] ] = true;
            }

            pl_bnd = computeRealBoundOfVariables(selected_vars_mask);
        }

        // update maximum/minimum bounds
        out_bnd = make_pair(min(out_bnd.first, pl_bnd.first),
                            max(out_bnd.second, pl_bnd.second));
    }
    return out_bnd;
}

//-----------------------------------------------------------------------------

const dd_edge& RSRG::getNSF() const {
    return NSF;
}

// build NSF if not already built (required by CTL context, printStatistics)
void RSRG::buildMonolithicNSF() {
    cout << "Building monolithic NSF..." << endl;
    // assert(useMonolithicNSF());
    // cout << " Using monolithic NSF." << endl;

    // check if empty, if not return
    expert_forest *forest = static_cast<expert_forest *>(getForestMxD());
    if(NSF.getNode() != forest->handleForValue(false)) return;

    switch (rsMethod) {
        case RSM_NONE:
        case RSM_SAT_MONOLITHIC:
        case RSM_BFS_MONOLITHIC:
        case RSM_BFS_MONOLITHIC_COV:
            break;
        case RSM_SAT_PREGEN: {
            // Build the monolithic NSF from the event NSFs
            NSF = dd_edge(forestMxD);
            for (int tr=0; tr<ntr; tr++) {
                NSF += events[tr]; // already initialized for saturation.
            }
            break;
        }

        case RSM_SAT_IMPLICIT: {
            // Build the monolithic NSF from the implicit relation
            // Since this method is called after saturation, the implicit relation
            // is fully generated and all variable bounds are known
            NSF = dd_edge(forestMxD);
            events.resize(ntr);
            for (int tr=0; tr<ntr; tr++) {
                events[tr] = impl_rel->buildEventMxd(impl_trn_handle[tr], forestMxD);

                NSF += events[tr];
            }
            break;
        }

        case RSM_SAT_OTF: {
            // Build the monolithic NSF from the OTF events, using the confirmed bounds.
            NSF = dd_edge(forestMxD);
            events.resize(ntr);

            for (int tt=0; tt<ntr; tt++) {
                events[tt] = otf_events[tt]->getRoot(); // MxD of the event

                NSF += events[tt];
            }
            break;
        }
        default:
            assert(false);
    }
}

//-----------------------------------------------------------------------------

PreImage::~PreImage() { }

dd_edge MonoPreImage::apply(const dd_edge& set) const {
    dd_edge pre_img(set.getForest());
    MEDDLY::apply(PRE_IMAGE, set, NSF, pre_img);
    return pre_img;
}

dd_edge ByEventPreImage::apply(const dd_edge& set) const {
    dd_edge pre_img(set.getForest());
    // pre-image as union of all pre-images of MxD events
    for (const dd_edge e: rsrg->events) {
        dd_edge pre_e(set.getForest());
        MEDDLY::apply(PRE_IMAGE, set, e, pre_e);
        pre_img += pre_e;
    }
    return pre_img;
}

//-----------------------------------------------------------------------------
// Support for saving DD in dot format
//-----------------------------------------------------------------------------

// Draw a dd_edge in Graphviz format
struct dot_of_DD {
    ofstream dot;
    std::set<node_handle> visited;
    expert_forest *forest;
    std::vector<std::vector<std::string>> nodes_per_lvl; // unprimed and primed
    const RSRG *rs;
    bool write_level_labels = false;

    void visit(const node_handle node) {
        if (visited.count(node) > 0)
            return;
        if (node == forest->handleForValue(false) || node == forest->handleForValue(true))
            return;
        visited.insert(node);

        const int node_level = forest->getNodeLevel(node);
        int series = node_level<0 ? 1 : 0;
        // Specify the rank of the node
        nodes_per_lvl[series][abs(node_level)] += " n";
        nodes_per_lvl[series][abs(node_level)] += std::to_string(node);

        // draw the node
        ostringstream edges;
        dot << "  n"<<node<<" [label=\"";
        unpacked_node *rnode = unpacked_node::newFromNode(forest, node, unpacked_node::storage_style::FULL_NODE);
        assert(rnode->isFull());
        int end = rnode->getSize() - 1;
        while (rnode->d(end) == forest->handleForValue(false))
            end--;

        for (int i = 0, cnt=0; i <= end; i++) {
            if (rnode->d(i) == forest->handleForValue(false))
                continue;
            dot << (cnt++==0 ? "" : "|") << "<i"<<i<<">";
            if (rnode->d(i) == forest->handleForValue(true)) {
                dot << i;// << ":T";
                //edges << "  n"<<node<<":i"<<i<<" -> \"T\";\n";
            }
            else {
                dot << i;
                edges << "  n"<<node<<":i"<<i<<" -> n"<<rnode->d(i)<<":n;\n";
            }
        }
        dot << "\"];\n";
        dot << edges.str();
        // Visit recursively
        for (int i = 0; i <= end; i++)
            visit(rnode->d(i));

        unpacked_node::recycle(rnode);
    }

    void start_visit(const dd_edge& e) {
        forest = static_cast<expert_forest *>(e.getForest());
        bool isForRel = forest->isForRelations();
        int num_series = isForRel ? 2 : 1; // number of sets in nodes_per_lvl[]
        dot << "digraph structs {\n  newrank=true;\n  size=\"5,5\";\n";
        dot << "  subgraph cluster1 { style=invis;\n";
        // dot << "  node [shape=record, width=0.2];\n";
        // dot << "  edge [arrowhead=vee, samehead=true];\n";
        dot << "  node [shape=record, height=0.8, width=0.5, fontsize=50, penwidth=4, fillcolor=white, style=filled];\n";
        dot << "  edge [arrowhead=vee, minlen=1, penwidth=4, color=blue];\n";
        // const int max_levels = forest->getDomain()->getNumVariables();//e.getLevel();
        const int max_levels = abs(e.getLevel());
        nodes_per_lvl.resize(num_series);
        for (size_t i=0; i<num_series; i++)
            nodes_per_lvl[i].resize(max_levels + 1);

        visit(e.getNode());
        dot << "}\n";

        if (write_level_labels) {
            // write places/levels in a separate cluster
            dot << "  subgraph cluster2 { style=invis;\n  node [shape=none, fontsize=60, margin=\"0.5,0.1\"];\n";
            for (int lvl=1; lvl<=max_levels; lvl++) {
                const char *level_name;
                if (rs != nullptr) { // use GreatSPN names
                    size_t p = rs->convertPlaceToMDDLevel(lvl) + 1;
                    level_name = tabp[p].place_name;
                }
                else { // use Meddly domain names
                    level_name = forest->getDomain()->getVar(lvl)->getName();
                }
                for (size_t i=0; i<num_series; i++) {
                    dot << "  LVL"<<lvl<<"_"<<i<<" [label=\""
                        << level_name<<(i==1?"\\\'":"")<<"\"];\n";
                    nodes_per_lvl[i][lvl] += " LVL";
                    nodes_per_lvl[i][lvl] += std::to_string(lvl);
                    nodes_per_lvl[i][lvl] += "_";
                    nodes_per_lvl[i][lvl] += std::to_string(i);
                }
            }
            dot << "}\n";
        }

        // write node rankings
        for (int lvl = max_levels; lvl > 0; lvl--) {
            for (size_t i=0; i<num_series; i++) {
                dot << (lvl==max_levels && i==0 ? "" : " -> ") 
                    << "{rank=same " << nodes_per_lvl[i][lvl] << "}";
            }
        }
        dot << " [style=invis]\n";
        dot << "}\n";
    }
};

// //-----------------------------------------------------------------------------

void write_dd_as_dot(const RSRG* rs, const dd_edge& e, 
                     const char* dot_name, bool level_labels)
{
    dot_of_DD d;
    d.rs = rs;
    d.write_level_labels = level_labels;
    d.dot.open(dot_name);
    // throw rgmedd_exception("Cannot open .dot file for writing!");

    d.start_visit(e);
}

//-----------------------------------------------------------------------------

void write_dd_as_pdf(const RSRG* rs, const dd_edge& e, 
                     const char* base_name, bool level_labels)
{
    std::string dot_name(base_name), pdf_name(base_name);
    dot_name += ".dot";
    pdf_name += ".pdf";

    write_dd_as_dot(rs, e, dot_name.c_str(), level_labels);

    std::string cmd = "dot -Tpdf \""+std::string(dot_name)+"\" -o \""+
                      pdf_name+"\" > /dev/null 2>&1";
    system(cmd.c_str());
    //remove(dot_name.c_str());
}

//-----------------------------------------------------------------------------

#if 0
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
#endif

//-----------------------------------------------------------------------------

static const int KEY_NO_EVENTS = -2;
static const int KEY_SUM_FIRINGS = -3;

// A compact small cache that stores <key, node> -> cardinality  entries
struct compact_cardinality_cache {
    struct CacheEntry {
        int index_key = -1;         // Index key (transition index or special code)
        node_handle node_key = -1;  // Meddly node key
        cardinality_t value = 0;    // Stored value

        bool is_valid() const { return index_key >= 0; }
    };
    std::vector<CacheEntry> cache;

    uint64_t n_valid = 0;
    uint64_t n_replaced = 0;
    mutable uint64_t n_searched = 0;
    mutable uint64_t n_found = 0;


    // Hash function that determines the cache entry for a given key pair
    inline size_t hash_fn(int index_key, node_handle node_key) const {
        size_t hash = size_t(index_key * 8960453) + size_t(node_key * 1459);
        return hash % cache.size();
    }

    void init_cache(size_t num_entries) {
        cache.resize(num_entries); // may throw std::bad_alloc
    }

    void store(int index_key, node_handle node_key, const cardinality_t& value) {
        // auto_rehash();
        size_t pos = hash_fn(index_key, node_key);
        if (cache[pos].is_valid())
            n_replaced++;
        else n_valid++;

        cache[pos].index_key = index_key;
        cache[pos].node_key = node_key;
        cache[pos].value = value;
    }

    const cardinality_t* 
    find_key(int index_key, node_handle node_key) const {
        n_searched++;
        size_t pos = hash_fn(index_key, node_key);
        if (cache[pos].index_key == index_key && cache[pos].node_key == node_key) {
            n_found++;
            return &cache[pos].value;
        }
        return nullptr; // not found
    }

    void rehash(size_t num) {
        std::vector<CacheEntry> old_cache;
        old_cache.resize(num);
        old_cache.swap(cache);

        n_valid = 0;
        for (size_t i=0; i<old_cache.size(); i++) {
            if (old_cache[i].is_valid()) {
                size_t pos = hash_fn(old_cache[i].index_key, old_cache[i].node_key);
                cache[pos] = std::move(old_cache[i]);
            }
        }
        n_searched = n_found = n_replaced = 0;
    }

    void auto_rehash() {
        // uint64_t total_inserted = st.n_replaced + st.n_valid;
        if (n_searched >= cache.size()) {
            uint64_t n_missed = n_searched - n_found;
            float miss_ratio = float(n_missed) / float(n_searched);
            cout << miss_ratio << endl;
            // Reset search stats
            n_searched = n_found = 0;
        }
    }
};

//-----------------------------------------------------------------------------

// Transition firings counter based on the trans_span_set_t encoding.
struct EventFiringsCounterShared {
    const RSRG& rsrg;
    expert_forest *forestMDD;
    const trans_span_set_t *trns_set;
    // Shared cache
    compact_cardinality_cache  cache;
    // Secondary cache (one entry per level + 1). This cache provides a cardinality_t entry 
    // for every level, to avoid allocating/freeing lots of mpz_t objects.
    std::vector<cardinality_t> working_set;

    EventFiringsCounterShared(const RSRG& _rsrg, const dd_edge &RS, 
                              const trans_span_set_t* ts) 
    : rsrg(_rsrg), trns_set(ts) 
    {
        // card_op = MEDDLY::getOperation(CARDINALITY, f, cardinality_operant_type);
        forestMDD = static_cast<expert_forest*>(RS.getForest());
        long numNodes = RS.getNodeCount(); //->getPeakNumNodes() + 1;
        assert(forestMDD->getDomain()->getNumVariables() == npl + rsrg.getExtraLvls());
        working_set.resize(forestMDD->getDomain()->getNumVariables() + 2); // Must be 2!

        // Compute the average transition span
        double sum_spans = 0.0, sum_spans2 = 0.0;
        for (auto& trn : trns_set->trns) {
            size_t span = trn.top() - trn.bot() + 1;
            sum_spans += span;
            sum_spans2 += span * span;
        }
        double avg_spans = sum_spans / ntr;
        double avg_spans2 = sum_spans2 / ntr;
        double devstd_spans = sqrt(avg_spans2 - avg_spans * avg_spans);
        size_t mult = size_t(avg_spans + devstd_spans) / 2;
        mult = std::max(mult, size_t(16));
        mult = std::min(mult, size_t(150));
        // cout << "\n\nCache size = " << (numNodes * mult) << "   mult = " << mult << "\n\n" <<endl;

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
    count_firings_of_event(const node_handle node, const trans_span_t *evt, 
                           ssize_t ioh_pos, int visit_level) 
    {
        if (visit_level == 0) {
            if (node == forestMDD->handleForValue(false))
                return CARD0;
            if (node == forestMDD->handleForValue(true))
                return CARD1;
            throw rgmedd_exception();
        }

        // Determine the enabling conditions of *evt at this level (if any)
        std::pair<int, int> enab_range = make_pair(0, INT_MAX);
        int next_ioh_pos = ioh_pos;
        if (evt != nullptr && ioh_pos >= 0 && evt->sorted_entries[ioh_pos]->level == visit_level - 1) {
            const ioh_t* ioh = evt->sorted_entries[ioh_pos];
            next_ioh_pos = ioh_pos - 1;

            if (ioh->has_input())
                enab_range.first = ioh->input;
            if (ioh->has_inhib())
                enab_range.second = ioh->inhib;
        }

        const int node_level = forestMDD->getNodeLevel(node);
        cardinality_t& markings_count = working_set.at(visit_level);
        if (visit_level > node_level) { 
            // visiting intermediate levels (reduced node)
            // We have all values in this level, up to the variable bound.
            int var_bound = forestMDD->getDomain()->getVariableBound(visit_level);
            // int num_enabling = 0;
            // for (int i=0; i<var_bound; i++)
            //     if (ioh->enabled_for (i))
            //         num_enabling++;
            int num_enabling = std::min(var_bound, enab_range.second) - enab_range.first;

            // Use a pre-allocated mpz_t object from the secondary cache, avoiding reallocations
            markings_count = count_firings_of_event(node, evt, next_ioh_pos, visit_level - 1);
            markings_count *= num_enabling;
            return markings_count;
        }
        assert(node_level > 0 && node_level-1 < npl);

        if (evt != nullptr) {
            if (node_level - 1 < evt->bot()) {
                assert(ioh_pos < 0);
                // There are no more enabling conditions for this event. Just count
                // all states at this node.
                markings_count = count_firings_of_event(node, nullptr, 0, visit_level);
                return markings_count;

                // MEDDLY::ct_object& result = get_mpz_wrapper();
                // dd_edge dd_node(forestMDD);
                // dd_node.set(node);
                // card_op->compute(dd_node, result);
                // MEDDLY::unwrap(result, cardinality_ref(markings_count));
                // return markings_count;
            }
        }
        assert(ioh_pos >= 0);        
        // Use the IOH signature as cache key, so that it will be shared among different transitions.
        const int trn_key = (evt==nullptr) ? KEY_NO_EVENTS : evt->sorted_entries[ioh_pos]->signature;
        const cardinality_t *p_cached_count = cache.find_key(trn_key, node);
        if (p_cached_count != nullptr)
            return *p_cached_count;


        markings_count = 0;
        unpacked_node *rnode = unpacked_node::newFromNode(forestMDD, node, unpacked_node::storage_style::AS_STORED);

        if (rnode->isFull()) {
            for (int i = enab_range.first; i < std::min((int)rnode->getSize(), enab_range.second); i++) {
                markings_count += count_firings_of_event(rnode->d(i), evt, next_ioh_pos, visit_level - 1);
            }
            // for (int i=0; i<rnode->getSize(); i++)
            //     if (ioh->enabled_for (i))
            //         markings_count += count_firings_of_event(rnode->d(i), evt, next_ioh_pos, visit_level - 1);
        }
        else {
            for (int i = 0; i < rnode->getNNZs(); i++) {
                if (rnode->i(i) >= enab_range.second)
                    break; // out of enabling range
                if (rnode->i(i) >= enab_range.first) {
                    markings_count += count_firings_of_event(rnode->d(i), evt, next_ioh_pos, visit_level - 1);
                }
            }
            // for (int i = 0; i < rnode->getNNZs(); i++)
            //     if (ioh->enabled_for (rnode->i(i)))
            //         markings_count += count_firings_of_event(rnode->d(i), evt, next_ioh_pos, visit_level - 1);
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

        cardinality_t& sum_firings = working_set.at(visit_level + 1);
        sum_firings = 0;
        const int node_level = forestMDD->getNodeLevel(node);

        if (visit_level - 1 >= npl) {
            // we are visiting an extra level above the place levels
            if (visit_level > node_level) { // visiting reduced node
                return count_firings_of_all_events(node, visit_level - 1);
            }
        }
        else { // This is the level of a Petri net place
            const trans_span_t* events_at_level = trns_set->trns_by_level[visit_level - 1];
            // Use visit_level+1 to avoid colliding with count_firings_of_event() in the working set

            if (visit_level > node_level) { // visiting reduced node
                // We have all values in this level, up to the variable bound.
                int var_bound = forestMDD->getDomain()->getVariableBound(visit_level);
                // Visit recursively            
                sum_firings = count_firings_of_all_events(node, visit_level - 1);
                sum_firings *= var_bound;
                // Add firings of events rooted at this visit level
                while (events_at_level) {
                    sum_firings += count_firings_of_event(node, events_at_level, 
                                                          events_at_level->sorted_entries.size()-1, 
                                                          visit_level);
                    events_at_level = events_at_level->next_at_level;
                }
                return sum_firings;
            }

            const cardinality_t *p_cached_sum = cache.find_key(KEY_SUM_FIRINGS, node);
            if (p_cached_sum != nullptr)
                return *p_cached_sum;

            // Count how many markings rooted at node enable the set of transitions
            // that have the enabling conditions' top level set at this level
            while (events_at_level) {
                sum_firings += count_firings_of_event(node, events_at_level, 
                                                      events_at_level->sorted_entries.size()-1,
                                                      visit_level);
                events_at_level = events_at_level->next_at_level;
            }
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

cardinality_t RSRG::count_num_fired_transitions_by_events_shared() const {
    expert_forest *forestMDD = static_cast<expert_forest *>(rs.getForest());
    const int start_lvl = forestMDD->getDomain()->getNumVariables();
    EventFiringsCounterShared efc(*this, getRS(), &trns_set);


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






















//-----------------------------------------------------------------------------

bool RSRG::visitCountNodesPerLevel(const node_handle node, RSRG::NodesPerLevelCounter& nplc) 
{
    expert_forest *forest = static_cast<expert_forest *>(rs.getForest());
    if (node == forest->handleForValue(false))
        return false;
    if (node == forest->handleForValue(true))
        return true;

    if (node >= nplc.visited.size()) {
        cerr << node << " " << nplc.visited.size() << endl;
        throw rgmedd_exception("ERROR: the cache size is smaller than the forest peak size.");
    }
    if (nplc.visited.at(node))
        return true; // Already visited

    const int node_level = forest->getNodeLevel(node);
    unpacked_node *rnode = unpacked_node::newFromNode(forest, node, unpacked_node::storage_style::AS_STORED);
    // assert(rnode->getLevel() >= 1 && rnode->getLevel() <= npl);
    nplc.nodesPerLvl[node_level-1]++;

    size_t num_nnz = 0;
    if (rnode->isFull()) {
        for (int i = rnode->getSize() - 1; i >= 0; i--) {
            if (visitCountNodesPerLevel(rnode->d(i), nplc))
                num_nnz++;
        }
    }
    else { // sparse node
        for (int i = rnode->getNNZs() - 1; i >= 0; i--) {
            if (visitCountNodesPerLevel(rnode->d(i), nplc))
                num_nnz++;
        }
    }
    if (num_nnz == 1)
        nplc.singletoneNodesPerLvl[node_level-1]++;

    unpacked_node::recycle(rnode);
    nplc.visited.at(node) = true;
    return true;
}

//-----------------------------------------------------------------------------

// Count the number of nodes in each level of the DD
void RSRG::countNodesPerLevel(NodesPerLevelCounter& nplc, const dd_edge& dd) {
    long numNodes = dd.getForest()->getPeakNumNodes() + 1;
    nplc.visited.resize(numNodes, false);
    int num_vars = dd.getForest()->getDomain()->getNumVariables();
    nplc.nodesPerLvl.resize(num_vars, 0);
    nplc.singletoneNodesPerLvl.resize(num_vars, 0);
    assert(dom->getNumVariables() == npl + extraLvls); 

    visitCountNodesPerLevel(dd.getNode(), nplc);
}

//-----------------------------------------------------------------------------

void RSRG::showExtendedIncidenceMatrix(bool show_saved_file) {
    std::vector<LevelInfoEPS*> allInfo;

    // Setup a new transition set (IOH) 
    trans_span_set_t new_trn_set;
    new_trn_set.initialize(false);
    new_trn_set.build_all_ioh_lists(net_to_mddLevel, true/* mark non-productives */);
    new_trn_set.compute_unique_productive_nodes(true/* keep sorted trns */, false /* no activation levels */);


    std::string inc_name = std::string(net_name) + "xinc";


    measure_PSI(net_to_mddLevel, false, false /*use_ilp*/, false /*ese_enum*/, false /*print_enums*/, *p_fbm);
    // if (g_incidence_file)
    //     inc_name = g_incidence_file;
    // uint64_t newSOUPS = measure_soups(net_to_mddLevel, new_trn_set);
    std::vector<size_t> lw_invs;
    get_lvl_weights_invariants(*p_fbm, lw_invs);
    // for (int lvl=npl-1; lvl>=0; lvl--)
    //     cout << setw(15)<<tabp[mddLevel_to_net[lvl]].place_name<<"  w="<<lw_invs[lvl]<<endl;

    std::vector<std::vector<std::string>> rangeMat;
    rangeMat = range_matrix_for_representation(*p_fbm);

    std::vector<std::string> RP;
    size_t totalPSI = range_prod_for_representation(*p_fbm, RP);

    LevelInfoEPS DD[2], Sing[2];
    for (int i=0; i<2; i++) {
        NodesPerLevelCounter nplc;
        const dd_edge* p_edge;
        switch (i) {
            case 0: // RS info
                if (!shouldBuildRS())
                    continue;
                p_edge = &rs;
                DD[i] = LevelInfoEPS{ .header="RS" };
                Sing[i] = LevelInfoEPS{ .header="Sing" };
                break;

            case 1: // LRS info
                if (!shouldBuildLRS())
                    continue;
                p_edge = &lrs;
                DD[i] = LevelInfoEPS{ .header="LRS" };
                Sing[i] = LevelInfoEPS{ .header="Sing" };
                break;
        }
        countNodesPerLevel(nplc, *p_edge);

        // Nodes per level in the RS DD
        static const char *colorsDD[] = { "0 0.4 0.7" };
        // LevelInfoEPS DD { .header="DD" };
        DD[i].colors = (const char **)colorsDD;
        DD[i].info.resize(npl);
        for (size_t lvl=0; lvl<npl; lvl++)
            DD[i].info[lvl] = std::to_string(nplc.nodesPerLvl[lvl]);
        DD[i].footer = std::to_string(std::accumulate(nplc.nodesPerLvl.begin(), 
                                                      nplc.nodesPerLvl.begin() + npl, 0));
        allInfo.push_back(&DD[i]);

        // Singleton nodes
        static const char *colorsSing[] = { "0.18 0.31 0.31", "0 0.4 0.7" };
        // LevelInfoEPS Sing { .header="Sing" };
        Sing[i].colors = (const char **)colorsSing;
        Sing[i].info.resize(npl);
        Sing[i].bold.resize(npl, false);
        Sing[i].clrIndex.resize(npl, 0);
        for (size_t lvl=0; lvl<npl; lvl++) {
            Sing[i].info[lvl] = std::to_string(nplc.singletoneNodesPerLvl[lvl]);
            if (nplc.nodesPerLvl[lvl] == nplc.singletoneNodesPerLvl[lvl]) {
                Sing[i].bold[lvl] = true;
                Sing[i].clrIndex[lvl] = 1;
            }
        }
        // allInfo.push_back(&Sing[i]);
    }

    // SWIR weights
    const char* colorsSwirW[] = { "0.3 0 0.5" };
    LevelInfoEPS SwirW { .header="W" };
    SwirW.colors = (const char **)colorsSwirW;
    SwirW.info.resize(npl);
    for (size_t lvl=0; lvl<npl; lvl++)
        SwirW.info[lvl] = std::to_string(lw_invs[lvl]);
    SwirW.footer = std::to_string(std::accumulate(lw_invs.begin(), lw_invs.end(), 0));
    // allInfo.push_back(&SwirW);


    // PSI weights (ranges)
    const char* colorsPSI[] = { "0.64 0.16 0.16" };
    LevelInfoEPS PSI { .header="Ranges", .footer=std::to_string(totalPSI) };
    PSI.colors = (const char **)colorsPSI;
    PSI.info.resize(npl);
    PSI.info.swap(RP);
    // allInfo.push_back(&PSI);


    // Array of extra per-level infos
    // LevelInfoEPS *allInfo[] = { &DD, &Sing, &SwirW, &PSI };

    // P-flow matrix
    flow_basis_t pfb = get_pflows();
    reorder_basis(pfb, net_to_mddLevel);
    // P-flow basis in footprint form
    // auto && pfb = get_basis(*p_fbm);

    // Generate the DD as an EPS and incorporate it inside the diagram
    DDEPS ddeps;
    size_t nodeCount = getRS().getNodeCount();
    char dot_name[] = "tempDD.XXXXXXXX";
    char ddEPS_name[128];
    bool has_dot_file = false, has_ddEPS_file = false;
    if (nodeCount < 200) { // Maximum # of RS nodes
        int fd = mkstemp(dot_name); // create the temp file
        cout << dot_name << endl;
        close(fd);

        write_dd_as_dot(this, getRS(), dot_name, false);
        has_dot_file = true;

        snprintf(ddEPS_name, sizeof(ddEPS_name), "%s.eps", dot_name);
        std::string cmd = "dot -Teps \""+std::string(dot_name)+"\" -o \""+
                          ddEPS_name+"\" > /dev/null 2>&1";
        if (0 == system(cmd.c_str())) {
            has_ddEPS_file = true;

            // Read back the bounding box of the DD
            ifstream ifeps(ddEPS_name);
            std::string line;
            while (getline(ifeps, line)) {
                if (4 == sscanf(line.c_str(), "%%%%BoundingBox: %lf %lf %lf %lf",
                                &ddeps.x, &ddeps.y, &ddeps.w, &ddeps.h)) 
                {
                    ddeps.fname = ddEPS_name;
                    break;
                }
            }
        }
        else cout << "Could not generate the DD plot with dot." << endl;
    }

    cout << "Writing extended incidence matrix to " << inc_name << " ..." << endl;
    write_incidence_as_EPS((inc_name + ".eps").c_str(), new_trn_set, 
                           net_to_mddLevel, pfb,
                           /*&rangeMat,*/ nullptr,
                           allInfo.data(), allInfo.size(),
                           &ddeps);
    // By using the --gsopt -sFONTPATH=... command, Ghostscript encapsulates 
    // arbitrary fonts in the resulting PDF
    // epstopdf test.eps --gsopt -sFONTPATH=/Users/elvio/Desktop/MY-SVN/GreatSPN/
    // SOURCES/JavaGUI/jlatexmath-master/src/org/scilab/forge/jlatexmath/fonts/base/
    // std::string cmd = "eps2eps -dNOSAFER \""+inc_name+".eps\" \""+inc_name+"-merged.eps\"  > /dev/null 2>&1";
    std::string cmd = "epstopdf --nosafer \""+inc_name+".eps\" > /dev/null 2>&1";
    if (system(cmd.c_str()))
        cout << "Cannot execute the command: " << cmd << endl;

    // cmd = "ps2pdf -dEPSCrop \""+inc_name+"-merged.eps\"  \""+inc_name+".pdf\" > /dev/null 2>&1";
    // system(cmd.c_str());

    // cmd = inc_name+"-merged.eps";
    // remove(cmd.c_str());

    if (show_saved_file) {
        open_file((inc_name + ".pdf").c_str());
    }
    // if (invoked_from_gui())
    //     cout << "#{GUI}# RESULT INC" << endl;
    if (has_dot_file) 
        remove(dot_name);
    if (has_ddEPS_file) 
        remove(ddEPS_name);
}

//-----------------------------------------------------------------------------






//-----------------------------------------------------------------------------

// Load the <netname>.unfmap file generated by the unfolding process
bool load_unfolding_map(const char* filename, const RSRG& rsrg, 
                        unfolding_map_t& out)
{
    unfolding_map_t umap;
    ifstream ifs(filename);
    if (!ifs)
        return false;

    size_t num_pl, num_tr, num_elems;
    int j;
    std::string base, id;

    // load place relation
    ifs >> num_pl;
    umap.pl_unf.resize(num_pl);
    for (size_t n=0; n<num_pl; n++) {
        ifs >> umap.pl_unf[n].first >> num_elems;
        umap.pl_unf[n].second.resize(num_elems);
        for (size_t el=0; el<num_elems; el++) {
            ifs >> id;
            if ((j = rsrg.findPlaceIdByName(id.c_str())) < 0)
                return false; 
            umap.pl_unf[n].second[el] = j;
        }
    }

    // load transition relation
    ifs >> num_tr;
    umap.tr_unf.resize(num_tr);
    for (size_t n=0; n<num_tr; n++) {
        ifs >> umap.tr_unf[n].first >> num_elems;
        umap.tr_unf[n].second.resize(num_elems);
        for (size_t el=0; el<num_elems; el++) {
            ifs >> id;
            if ((j = rsrg.findTransitionIdByName(id.c_str())) < 0)
                return false; 
            umap.tr_unf[n].second[el] = j;
        }
    }

    // cout << "PLACES:\n";
    // for (auto&& list : umap.pl_unf) {
    //     cout << list.first << ": ";
    //     for (int ii : list.second)
    //         cout << ii << " ";
    //     cout << endl;
    // }
    // cout << "TRANSITIONS:\n";
    // for (auto&& list : umap.tr_unf) {
    //     cout << list.first << ": ";
    //     for (int ii : list.second)
    //         cout << ii << " ";
    //     cout << endl;
    // }

    out = std::move(umap);
    return true;
}

//-----------------------------------------------------------------------------

bool RSRG::has_unfolding_map() const {
    if (!m_unfmap_loaded) {
        // try loading the map for the first time
        std::string filename = netname + std::string("unfmap");
        m_has_unfmap = load_unfolding_map(filename.c_str(), *this, m_unfmap);
        m_unfmap_loaded = true;
    }
    return m_has_unfmap;
}

const unfolding_map_t& RSRG::get_unfolding_map() const {
    assert(m_unfmap_loaded && m_has_unfmap);
    return m_unfmap;
}

//-----------------------------------------------------------------------------













//-----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const result_t& r) {
    boost::apply_visitor(overload{
        [&os](ssize_t i)  { if (i==INFINITE_CARD) os << "inf"; else os << i; },
        [&os](bool b)     { os << (b ? "TRUE" : "FALSE"); },
        [&os](double d)   { if (d==INFINITE_CARD) os << "inf"; else os << d; },
    }, r);
    return os;
}

//-----------------------------------------------------------------------------













