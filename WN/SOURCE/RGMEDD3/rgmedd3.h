#ifndef MEDD_H_
#define MEDD_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef OVERFLOW
#undef OVERFLOW
#endif

#include <assert.h>
#include <exception>
#include <sstream>
#include <fstream>
#include <math.h>
#include <limits>
#include <iomanip>
#include <map>
#include <list>
#include <time.h>
#include <functional>
#include <iostream>
#include <sys/resource.h>
#include "utils/union_find.h"
#include "utils/mt64.h"

// Optionally include <gmpxx.h> (if available on the platform)
#ifdef HAS_GMP_LIB
#include <gmpxx.h>
// The cardinality type is a multi-precision integer
typedef mpz_class cardinality_t;
// Get the mpz_t& needed by Meddly from a mpz_class& object
inline mpz_t& cardinality_ref(cardinality_t& c) { return *reinterpret_cast<mpz_t*>(&c); }
inline unsigned long int get_ulong(cardinality_t& c) { return mpz_get_ui(cardinality_ref(c)); }
inline long int get_long(cardinality_t& c) { return mpz_get_si(cardinality_ref(c)); }
inline double get_double(cardinality_t& c) { return mpz_get_d(cardinality_ref(c)); }
// The Meddly operand type
#define cardinality_operant_type  MEDDLY::HUGEINT
#else
// The cardinality type is just a double
typedef double cardinality_t;
inline double& cardinality_ref(double& c) { return c; }
inline unsigned long int get_ulong(cardinality_t c) { return (unsigned long int)(c); }
inline long int get_long(cardinality_t c) { return (long int)(c); }
inline double get_double(cardinality_t c) { return c; }
#define cardinality_operant_type  MEDDLY::INTEGER
#endif

// Include meddly only after having included <gmpxx.h>
#include <meddly.h>
#include <meddly_expert.h>

#include "general.h"

// #ifndef __PRS_H__
// #define __PRS_H__
// #include "../AUTOMA/parser.h"
// #endif

extern "C" {
#include "WN/INCLUDE/const.h"
#include "WN/INCLUDE/struct.h"
#include "WN/INCLUDE/var_ext.h"
#include "WN/INCLUDE/fun_ext.h"
#include "WN/INCLUDE/macros.h"
}

#define EXIT_FAILURE_MEDDLY    80
#define EXIT_FAILURE_RGMEDD    81
#define EXIT_FAILURE_GENERIC   81
#define EXIT_FAILURE_GMP       82
#define EXIT_FAILURE_SIGABRT   83
#define EXIT_TIMEOUT_VARORDER  84


// #ifndef __PRS_H__
// #define __PRS_H__
// #include "../AUTOMA/parser.h"
// #endif



#ifdef MCC
# error "Do not compile with -DMCC. Use the environment variable MCC=1 to turn on MCC mode."
#endif

#define  DEBUG 0
#define  DEBUG1 0
#define  DEBUG2 0
#define  DEBUG3 0
#define  DEBUG4 0
#define  DEBUGPARSER 0
#define  MAX_ID 32
#define  IOPRECISION 12
#define  MAXSERV 255
#define  MAXPRIO 127
//Use to endoce the id transition in the MTMDD for the AUTOMATON
#define  INCTRAN 1000
//predefined "machine epsilon", a numeric value representing the difference between 1 and the least value greater than 1 that is representable [1]
#define _DIFFMIN(T) numeric_limits<T>::epsilon()

extern "C" { 
    int read_PIN_file();
    void free_PIN_file();
    int* sort_according_to_pinv(); 
};
extern void print_banner(const char* title);
// Open a file with the default application/browser
int open_file(const char * filename);

// A dummy value used to mark an infinite RS cardinality (just an arbitrary negative value)
const int INFINITE_CARD = -570;
const int UNKNOWN_CARD = -571;

using namespace std;

// multi-test support: rgmedd3 may be invoked as a child process with a timeout
extern int g_max_seconds_statespace;
inline bool is_child_subprocess() { return (g_max_seconds_statespace != 0); }


// C string comparator
struct cstr_less {
    inline bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) < 0;
    }
};

//-----------------------------------------------------------------------------
// Exception class used by RGMEDD
//-----------------------------------------------------------------------------

class rgmedd_exception : public std::exception {
    std::string msg; // message carried in this exception
public:
    rgmedd_exception() {}
    rgmedd_exception(const std::string& m) : msg(m) {}
    rgmedd_exception(std::string&& m) : msg(m) {}
    rgmedd_exception(const char* m) : msg(m) {}

    virtual const char* what() const throw() override;
};

//-----------------------------------------------------------------------------

// Is the application called from the new Java GUI?
bool invoked_from_gui();
extern "C" int print_stat_for_gui();
bool set_print_stat_for_gui();

// Is running for the model checking contests?
// defined in rg_files.c for compatibility
extern "C" int running_for_MCC();

//-----------------------------------------------------------------------------

using namespace std;
using namespace MEDDLY;

struct flow_basis_metric_t;

//---------------------------------------------------------------------------------------
// Input + output + inhibitor effect of a transition on a single place
//---------------------------------------------------------------------------------------
struct ioh_t {
    ioh_t(int i=-1, int o=-1, int h=-1)
    : input(i), output(o), inhib(h) {}
    int input;
    int output;
    int inhib;

    inline bool compare(const ioh_t& e) const {
        if (input != e.input) return input < e.input;
        if (output != e.output) return output < e.output;
        if (inhib != e.inhib) return inhib < e.inhib;
        return false;
    }

    // productive arc = either some input or some output.
    // Test arcs and inhibitor arcs are non-productive
    inline bool is_productive_entry() const {
        return input != output;
    }

    inline bool has_input() const { return input > 0; }
    inline bool has_output() const { return output > 0; }
    inline bool has_inhib() const { return inhib > 0; }

    // Test for enabling
    inline bool enabled_for(int i) const {
        if (has_input() && i < input) return false;
        if (has_inhib() && i >= inhib) return false;
        return true;
    }
    // Effect of firing (assuming enabling)
    inline int firing_effect(int i) const {
        if (has_input())   i -= input;
        if (has_output())  i += output;
        assert(i >= 0);
        return i;
    }
};
//static const ioh_t DO_NOTHING_IOH = ioh_t();

ostream& operator<< (ostream& os, const ioh_t& x);

//---------------------------------------------------------------------------------------

#define COMBINE_HASH(h1, h2)   (((h1) << 4) + (h1) + (h2))

//---------------------------------------------------------------------------------------
//  IOH effect on a specified place/level
//---------------------------------------------------------------------------------------
struct var_ioh_t : ioh_t {
    var_ioh_t(int i=-1, int o=-1, int h=-1, int p=-1)
    : ioh_t(i, o, h), place(p) {}
    int place;      // Place connected to the I/O/H arc
    int level = -1; // Level of that place in a given variable order

    // Fields filled by the soups metric function
    bool is_unique;          // unique entry, including entries below
    bool is_productive;      // productive entry (considers also entries below)
    size_t max_unique_above; // max # of unique level above before the next IOH 
    size_t signature;        // Unique signature of this entry
    size_t signature_below;  // Signature of the entry below

    // Fields used by the soups metric function
    size_t hash;             // hash value, for the hash table and fast comparisons
    var_ioh_t* hash_next;    // hash table chain

    // recompute the hash, using the IOH and the signature of the node below
    inline void compute_hash(size_t sig_below) {
        hash = signature_below = sig_below;
        hash = COMBINE_HASH(hash, level);
        hash = COMBINE_HASH(hash, input);
        hash = COMBINE_HASH(hash, output);
        hash = COMBINE_HASH(hash, inhib);
    }

    // to be the same node, it must have the same IOH and the same sub-nodes
    inline bool is_same(const var_ioh_t& v) const {
        if (v.hash != hash)
            return false;
        if (v.signature_below != signature_below)
            return false;

        return (input==v.input) && (output==v.output) && (inhib==v.inhib) && (place==v.place);
    }
};

ostream& operator<< (ostream& os, const var_ioh_t& x);

//---------------------------------------------------------------------------------------
// Incidence of a transition (all the arcs of the transition) using IOH structures
//---------------------------------------------------------------------------------------
struct trans_span_t {
    int trn;      // Transition index
    std::vector<var_ioh_t> entries; // The IOH arcs of this transition

    // Pointers to the entries[] elements, sorted using the variable order
    std::vector<var_ioh_t*> sorted_entries;

    // Next transition in the same level
    trans_span_t* next_at_level = nullptr;

    // Build the list of transition entries from the GreatSPN data structures
    void build_for_trn(int tr);

    // Rebuild the linked list of the IOH entries according to the variable order
    void build_ioh_list(const std::vector<int> &varorder, bool test_non_productiveness);

    // Get the top level. Can only be called after build_ioh_list()
    inline size_t top() const { 
        return sorted_entries.back()->level;
    }

    // Get the bottom level. Can only be called after build_ioh_list()
    inline size_t bot() const { 
        return sorted_entries.front()->level;
    }

    inline bool is_top_unique_productive() const { 
        return sorted_entries.back()->is_unique && sorted_entries.back()->is_productive;
    }

};

ostream& operator<< (ostream& os, const trans_span_t& x);

//---------------------------------------------------------------------------------------
// The incidence matrix of the Petri net, encoded using IOH elements
//---------------------------------------------------------------------------------------
struct trans_span_set_t {
    trans_span_set_t() : uf(npl) {}

    // The PN transitions, re-encoded as IOH entries
    std::vector<trans_span_t> trns;
    // Statistics on the arcs
    size_t num_in=0, num_out=0, num_inhib=0, num_test=0, num_arcs=0;

    // Support data structure for SOUPS computation
    // Hash table to recognize the shared entries
    std::vector<var_ioh_t*>     hash_table;
    // Sorted list of transitions, for EPS visualization
    std::vector<const trans_span_t*>  sorted_trns;
    // Total number of assigned signatures (i.e., unique DAG nodes)
    size_t signatures = 0;
    // Down activation levels of each MDD level
    std::vector<int> down_active;
    // List all transitions per level
    std::vector<trans_span_t*> trns_by_level;

    union_find uf;

    // Initialize the transition set from the GreatSPN global net structures
    void initialize(bool verbose = true);

    // Build the linked list of each transition entries
    //   test_non_productiveness = mark non-productive nodes
    void build_all_ioh_lists(const std::vector<int> &varorder, bool test_non_productiveness);

    // Assign a unique signature to all unique & productive IOH nodes
    // sort_transitions = used by soups representation, it is not needed for intensive 
    //    soups computations, where it can be skipped.
    void compute_unique_productive_nodes(bool sort_transitions, bool activation_lvls);
    // void compute_unique_productive_nodes2(bool sort_transitions, bool activation_lvls);
    void compute_level_boundaries();

    // Initialize the list of transitions by level
    void build_trn_list_by_level();

    // Finalize the transition set (no more rebuilds)
    void finalize() { finalized = true; }

private:
    bool finalized = false;
};

std::shared_ptr<trans_span_set_t> make_trans_span_set();

ostream& operator<< (ostream& os, const trans_span_set_t& x);

//---------------------------------------------------------------------------------------

class CTLResult {
    bool _is_int = false;
    bool _is_bool = false;

    bool b = false;
    int i = 0;
public:
    CTLResult() {}
    CTLResult(bool x) { set_bool(x); }
    CTLResult(int x) { set_int(x); }

    inline void set_bool(bool x) { b=x; _is_bool=true; }
    inline void set_int(int x) { i=x; _is_int=true; }
    inline bool is_int() const { return _is_int; }
    inline bool is_bool() const { return _is_bool; }
    inline bool is_undef() const { return !is_int() && !is_bool(); }

    inline bool get_bool() const { return b; }
    inline int get_int() const { return i; }

    inline bool operator==(const CTLResult& r) const {
        return _is_int == r._is_int && _is_bool == r._is_bool && b == r.b && i == r.i;
    }
};

inline ostream& operator << (ostream& os, const CTLResult& r) {
    if (r.is_bool())  return os << (r.get_bool() ? "TRUE" : "FALSE");
    if (r.is_int())  return os << r.get_int();
    return os << "?";
}

//-----------------------------------------------------------------------------

enum RsMethod {
    RSM_NONE,               // Do not generate the potential state space from the net constraints
    RSM_SAT_MONOLITHIC,     // Single monolithic next-state-function
    RSM_BFS_MONOLITHIC,     // Monolithic NSF, no saturation
    RSM_BFS_MONOLITHIC_COV, // Monolithic NSF, no saturation, coverability test
    RSM_SAT_PREGEN,         // Separate pre-generated next-state-functions, one per transition
    RSM_SAT_IMPLICIT,       // NSF encoded using an Implicit transition relation.
    RSM_SAT_OTF,            // Generate NSFs of transitions on-the-fly
};

//-----------------------------------------------------------------------------

enum LrsMethod {
    LRSM_NONE,
    LRSM_PBASIS_CONSTRAINTS, // LRS from the P-flow basis constraints
};

//-----------------------------------------------------------------------------

enum VariableOrderCriteria {
    VOC_NO_REORDER = 1,        // Take the same order of the .net/.def places
    VOC_INPUT_ORDER,           // Take the same order given in input
    VOC_RANDOM,                // Generate a random order (fixed or specified seed)
    VOC_FROM_FILE,             // Read the order from a separate .place file
    VOC_DFS,                   // Depth-first Visit order
    VOC_BFS,                   // Breadth-first Visit order
    VOC_PINV,                  // Heuristic that uses P-invariants (old version)
    VOC_PCHAINING,             // P-chaining heuristics
    VOC_FORCE,                 // Use FORCE implementation
    VOC_FORCE_PINV,            // Use FORCE implementation with P-semiflows as clusters
    VOC_FORCE_NU,              // Use FORCE implementation with Nested Units as clusters
    VOC_TOPOLOGICAL,           // A topological sort of the places
    VOC_CUTHILL_MCKEE,         // Cuthill-Mckee algorithm
    VOC_KING,                  // Use King ordering (less efficient than Cuthill-Mckee)
    VOC_SLOAN,                 // Use sloan ordering with W1=1, W2=2
    VOC_SLOAN_1_16,            // Use sloan ordering with W1=1, W2=16
    VOC_NOACK,                 // Use Noack order ("A ZBDD package for efficient model checking of Petri nets")
    VOC_TOVCHIGRECHKO,         // Use Tovchigrechko method.
    VOC_NOACK2,                // Use Noack method. (new implementation)
    VOC_TOVCHIGRECHKO2,        // Use Tovchigrechko method. (new implementation)
    VOC_GRADIENT_P,
    VOC_MARKOV_CLUSTER,
    VOC_GRADIENT_NU,
    VOC_VCL_CUTHILL_MCKEE,
    VOC_VCL_ADVANCED_CUTHILL_MCKEE,
    VOC_VCL_GIBBS_POOLE_STOCKMEYER,
    VOC_PRINT_METRICS_AND_QUIT,  // Do not order anything, just print graph metrics and quit
    // Meta-heuristics
    VOC_META_FORCE,            // Select a random+Force that minimizes a target metric
    VOC_META_BY_SCORE,         // Select method with best score
    //
    VOC_TOTAL_CRITERIAS,       // Must be the last entry of this enum
};

//-----------------------------------------------------------------------------

// Which variable order metrics are available
enum VariableOrderMetric {
    METRIC_SOUPS,
    METRIC_SWIR,
    METRIC_SWIR_X,
    METRIC_B_RANK,
    METRIC_NES,
    METRIC_WES1,
    METRIC_PSF,
    METRIC_PF,
    METRIC_SOT,
    METRIC_PTS,
    METRIC_PTS_P,
    METRIC_BW,
    METRIC_PROF,
    METRIC_AVGWF,
    METRIC_MAXWF,
    METRIC_RMSWF,
    METRIC_SOS,
    METRIC_PSI,
    // SO_x_y metrics
    METRIC_SO_ES_wI,
    METRIC_SO_ES_wPSF,
    METRIC_SO_ES_wRNG,
    METRIC_SO_ES_wRS,
    METRIC_SO_ES_w1,
    METRIC_SO_UPS_wI,
    METRIC_SO_UPS_wPSF,
    METRIC_SO_UPS_wRNG,
    METRIC_SO_UPS_wRS,
    METRIC_SO_UPS_w1,
    METRIC_SO_UPSx_wI,
    METRIC_SO_UPSx_wPSF,
    METRIC_SO_UPSx_wRNG,
    METRIC_SO_UPSx_wRS,
    METRIC_SO_UPSx_w1,
    METRIC_SO_US_wI,
    METRIC_SO_US_wPSF,
    METRIC_SO_US_wRNG,
    METRIC_SO_US_wRS,
    METRIC_SO_US_w1,
    METRIC_SO_1_wI,
    METRIC_SO_1_wPSF,
    METRIC_SO_1_wRNG,
    METRIC_SO_1_wRS,

    NUM_METRICS,
    NO_METRIC // invalid value
};

// Get the name of a metric
const char* metric_name(VariableOrderMetric m);

//---------------------------------------------------------------------------------------
// Variable order improvements using simulated annealing
enum class Annealing {
    NO_ANN,
    ANN_PBASIS,
    ANN_PBASIS_MIN,
    ANN_MINIMIZE_METRIC,
    ANN_FORCE,
    ANN_FORCE_P,
    ANN_FORCE_TI,
};
const char *name_of_annealing(Annealing, VariableOrderMetric);

//-----------------------------------------------------------------------------
// Variable order improvements using simulated annealing by groups
enum class GroupAnnealing {
    NO_GROUP_ANN,
    GRPANN_INV,
    GRPANN_INV_FORCE
};

//-----------------------------------------------------------------------------
// Variable order refinement with force
enum class ForceBasedRefinement {
    NO_REFINEMENT,
    FORCE,
    FORCE_PSF,
    FORCE_TI,
    FORCE_I,
    BEST_AVAILABLE
};


//-----------------------------------------------------------------------------
// Criteria to select a variable order
struct var_order_selector {
    VariableOrderCriteria heuristics = VOC_NO_REORDER;
    VariableOrderMetric target_metric = VariableOrderMetric::NO_METRIC;
    bool verbose = true;

    // Simple transformations
    bool reverse_order = false;
    bool reverse_if_better_soups = false;
    bool reorder_SCC = false;

    // Meta-heuristic
    double *p_meta_score = nullptr;
    double *p_meta_score_ex2 = nullptr;
    double *p_meta_score_ex3 = nullptr;
    double *p_meta_score_ex4 = nullptr;
    bool discount_score = false;
    int num_tested_varorders = 1;
    int time_for_each_test = 10;

    // Type of force-based refinement
    ForceBasedRefinement refinement = ForceBasedRefinement::NO_REFINEMENT;

    // Group-based simulated annealing
    GroupAnnealing group_annealing = GroupAnnealing::NO_GROUP_ANN;

    // Improvement with simulated annealing
    Annealing annealing = Annealing::NO_ANN;
    VariableOrderMetric ann_metric = VariableOrderMetric::NO_METRIC;
};

//-----------------------------------------------------------------------------

enum class BoundGuessPolicy {
    M0_LINEAR, // each variable starts at its initial marking value or 1, then underestimated variables increase by their max_var_incr times restarts
    M0_EXP, // each variable starts at its initial marking value or 1, then underestimated variables increase by their max_var_incr times 2^restarts
    MAX0_LINEAR, // all variables start with the max token count in m0, then underestimated variables increase by their max_var_incr times restarts
    MAX0_EXP, // all variables start with the max token count in m0, then underestimated variables increase by their max_var_incr times 2^restarts
    M0_MAX0_EXP, // each variable starts at its initial marking value or 1, underestimated variables start with the max token count in m0, 
                 // then underestimated variables increase by their max_var_incr times 2^(restarts-1)
    LOAD_FROM_FILE, // loads the bounds from the bnd file; if a bound of a place is not available m0 or 1 is used. 
                    // The underestimated variables increase by their max_var_incr times 2^restarts
    SAFE,        // model is safe (every place is 1-bounded).
};

// Load the .bnd file, if it exists. If it does not exists, return nullptr.
int* load_bounds();

// Do we have a list of potentially unbounded places? (determined using ILP)
bool have_unbound_list();
// Get the list of potentially unbounded places
const std::vector<int>& get_unbound_list();

// Check if there is a maximum token count bound of infinite
bool have_unbounded_token_count();

//-----------------------------------------------------------------------------

enum class PlaceBoundState {
    GUESSED,  // The place bound is guessed, i.e. it is not known for sure
    CERTAIN,  // The place bound is known with certainty before RS construction
};

//-----------------------------------------------------------------------------
// Main class of RGMEDD. Holds the Meddly objects, and builds the RS.
//-----------------------------------------------------------------------------

class RSRG {
protected:
    domain *dom = nullptr;

    forest *forestMDD = nullptr; //RS forest (Boolean MDD)
    forest *forestMxD = nullptr; // transitions forest MxD (Boolean matrix diagram)
    forest *forestMTMxD = nullptr; // transitions + rates forest MxD (Float matrix diagram)
    forest *forestEVplMDD = nullptr; // RS + id states forest (EV+MDD)
    forest *forestMTMDDint = nullptr; // Integer terminals
    
    dd_edge rs;         // Reachability set
    bool rs_is_infinite = false; // Store if the RS is infinite (fails coverability test)

    dd_edge lrs;        // Linearized RS (constraint satisfaction problem)
    dd_edge indexrs;
    dd_edge NSF;        // Monolithic Next State Function 
    dd_edge NSFReal;    // timed transitions with rates
    dd_edge DiagReal;   // Matrix diagonal
    // dd_edge *NSFi = nullptr;//immediate transitions
    std::vector<dd_edge> VectNSFReal;//vector of timed transitions with rates X throughput

    std::string netname;
    std::string propname;
    dd_edge initMark;   // Initial state encoded as MDD edge

    // Enabling conditions of each event as MxDs (only input/inhibitor arcs are encoded)
    vector<dd_edge> all_enabling_events;
    // Firing of each event (enabling conditions + output arcs)
    vector<dd_edge> events;

    // ---- Dynamic bound management ----------------------

    // Stores if the stored bounds are exact (i.e. are the bounds of the full RS)
    // or if they are a guess made during RS generation. This variable starts at true,
    // and becomes false after RS construction.
    bool guessing_bounds = true;

    // Contains, for each variable, its effective max value in the RG
    // This vector is computed *after* the RG construction, and reflects
    // the real maximum value of each variable.
    std::vector<int> realBounds;

    // maximum increment a variable could receive from the firing of any event
    std::vector<int> maxVarIncr;
    
    // our current estimate of each variable max value (inclusive)
    std::vector<int> guessedBounds;
    std::vector<PlaceBoundState> guessState;
    
    // The domain bound for variable i (exclusive) that will be used for Meddly variable domains.
    int domainBounds(int i) const;

    // The guessing policy for unknown/uncertain bounds
    BoundGuessPolicy bound_policy = BoundGuessPolicy::MAX0_EXP;
    bool print_guessed = false;


    // Variable order: from place order in .net file to variables levels in the MDD
    std::vector<int> net_to_mddLevel;
    // Inverse relation
    std::vector<int> mddLevel_to_net;
    
    // The maximum number of tokens in any marking of the RG
    int maxSumTokens = -1;

    // The method used to build the RS and the LRS
    RsMethod rsMethod;
    LrsMethod lrsMethod;

    // Encoding of the Petri net transitions using IOH elements.
    trans_span_set_t trns_set;
    std::shared_ptr<flow_basis_metric_t> p_fbm;

    std::map<const char*, int, cstr_less> S2Ipl; // from place name to place index
    std::map<const char*, int, cstr_less> S2Itr; // from transition name to its index

    // Use fast NSF generation.
    bool fast_NSF_gen = true;

    // Criteria for variable order generation
    var_order_selector var_order_sel;

public:
    clock_t init_time;     // Clock time when RSRG was initialized
    clock_t varorder_time; // Time to generate the variable order

    // Expected values for regression control
    double expected_rs_card = -1.0;
    double expected_rg_edges = -1.0;
    double expected_lrs_card = -1.0;
    ssize_t expected_max_tokens_place = -1;
    ssize_t expected_max_tokens_marking = -1;

    std::vector<CTLResult> expected_ctl;

protected:
    //--------------------------------------------------
    // Saturation with pregenerated relation support:
    satpregen_opname::pregen_relation* pregen_rel = nullptr;
    specialized_operation* pregen_sat = nullptr;

    //--------------------------------------------------
    // Implicit relation support:
    satimpl_opname::implicit_relation* impl_rel = nullptr;
    std::vector<rel_node_handle> impl_trn_handle; // The top rel_node_handle of each encoded transition.
    specialized_operation* impl_sat = nullptr;
    void buildImplicitRelation();

    //--------------------------------------------------
    // On-the-fly saturation support:
    int *p_otf_unprimed_minterms = nullptr;
    int *p_otf_primed_minterms = nullptr;
    std::vector<satotf_opname::event*> otf_events;
    satotf_opname::otf_relation *otf_rel = nullptr;
    specialized_operation* otf_sat = nullptr;
    void buildOtfRelation();

    class otf_subevent;
    friend class otf_subevent;

    //--------------------------------------------------
    // coverability analysis support:
    dd_edge testerForCoverabilityOfPlace(int pl);

    //--------------------------------------------------
    // constraint satisfaction (LRS generation)
    bool buildLRSbyPBasisConstraints();

private:
    int visitXBounds(const node_handle node, int visit_level, std::vector<bool> &visited, std::vector<int> &nodeMaxSumTokens);

    /*inline int getbound(const int i) {if (bounds != NULL) return bounds[i]; else return -1;};*/

    int visitXBoundOfVariables(const node_handle node, int visit_level, const std::vector<bool> &selected_vars,
                               std::vector<int> &cache) const;
    // Initializes guessedBound according to the policy. Called before domain initialisaztion
    void initializeGuessedBounds();
    //!compute the effective variable bounds associated with the current RS.
    void computeRealBounds();

    // const cardinality_t& 
    // visitXTrnEnabling(const node_handle node, int visit_level, const std::vector<pair<int, int>> &enabling,
    //                   std::vector<cardinality_t> &cache, std::vector<cardinality_t> &cache2) const;

    struct MakeNsfHelper;
    friend struct MakeNsfHelper;
    
public:
    // Make a new estimate of the guessed bounds, using the realBounds[] values
    bool updateGuessedBounds(int restart_count);
    // Compute the bound of a set of places
    int computeRealBoundOfVariables(const std::vector<bool> &selected_vars) const;

    // Notify rgmedd that we have observed a new variable bound during on-the-fly RS generation
    inline void observeVariableValue(int var, int tokens) {
        // if (guessedBounds[var] < tokens) {
        //     cout << "Observe: (" << tabp[mddLevel_to_net[var]].place_name << ", " << tokens << ")" << endl;
        // }
        guessedBounds[var] = std::max(guessedBounds[var], tokens);
    }


    struct NodesPerLevelCounter {
        std::vector<bool> visited;
        std::vector<int>  nodesPerLvl;
        std::vector<int>  singletoneNodesPerLvl;
    };
    // Count the # of nodes for each level in the DD (for visualization purposes)
    void countNodesPerLevel(NodesPerLevelCounter& nplc, const dd_edge& dd);
    void showExtendedIncidenceMatrix(bool show_saved_file);
private:
    bool visitCountNodesPerLevel(const node_handle node, NodesPerLevelCounter& nplc);

public:
    // Count # of fired transitions in RS
    // const cardinality_t& 
    // count_markings_enabling(int tr, std::vector<pair<int, int>>& enabling,
    //                         std::vector<cardinality_t>& cache,
    //                         std::vector<cardinality_t> &cache2) const;
    // cardinality_t count_num_fired_transitions() const;

    // cardinality_t count_num_fired_transitions_by_events() const;

    // Count the number of fired transition in the RS using the trns_set IOH entries
    cardinality_t count_num_fired_transitions_by_events_shared() const;

    // Get the MxD of a single event
    const dd_edge& getEventMxD(int tr) const { return events.at(tr); }

    inline bool useMonolithicNSF() const { 
        return (rsMethod == RSM_SAT_MONOLITHIC) || 
               (rsMethod == RSM_BFS_MONOLITHIC) || 
               (rsMethod == RSM_BFS_MONOLITHIC_COV); 
    }
    inline bool useByEventsNSF() const { return  (rsMethod == RSM_SAT_PREGEN); }
    inline bool useImplicitNSF() const { return  (rsMethod == RSM_SAT_IMPLICIT); }
    inline bool useOtfNSF() const { return  (rsMethod == RSM_SAT_IMPLICIT); }
    
    inline bool shouldBuildRS() const { return (rsMethod != RSM_NONE); }
    inline bool shouldBuildLRS() const { return (lrsMethod != LRSM_NONE); }

    // Should we build the NSF before generating the RS with Saturation?
    inline bool buildNSFsBeforeRS() const { return useMonolithicNSF() || useByEventsNSF(); }

    RSRG();
    ~RSRG();

    // Initialize Meddly, computes the variable order, initializes the guessed bounds
    void initialize(RsMethod _rsMethod, LrsMethod _lrsMethod,
                    const var_order_selector& initSel,
                    const long meddly_cache_size, 
                    BoundGuessPolicy bound_policy);

    // Initialize the dd_edges of the initial marking
    bool init_RS(const Net_Mark_p &);
    //! \name Methods use to build the NEXT STATE FUNCTIONS
    //@{
    // //! it takes in input matrices I,O,H for a transition and its type and computes its next state function.
    // bool MakeNextState(const std::vector<int>& F, const std::vector<int>& T, 
    //                    const std::vector<int>& H, const int tt);
    // //! it takes in input matrices I,O,H for a transition and its type and computes recursively its next state function. It returns -1 in case of error 0 otherwise.
    // void MakeNextState(int *f, int *t, int *h, const int &tt, int pl);
    // //@}

    // Build the NSF for the guessed bounds for those RS methods that require the NSFs
    // before the RS generation. On-the-fly and implicit RS generation do not require this method.
    void initializeNSFsForKnownBounds();

    // Build the RS using the selected method.
    // Returns true if the RS is infinite, false otherwise
    bool buildRS();

    bool isRS_infinite() const { return rs_is_infinite; }

    // Build the Linearized Reachability Set.
    // Return true if it is possible, false if it is not possible.
    bool buildLRS();

    //! it generates the RS using the next state function for models with timed and immediate transitions.  It returns -1 in case of error 0 otherwise.
    bool genRSAll();
    //@}

    void prepareNSFforCTL();

    //! \name Methods use to index the RS
    //@{
    //! it creates an RS EV+MDD from the RS MDD. 
    void IndexRS();
    //@}

    //! \name Methods use to solve the underlying CTMC
    //@{
    //!It computes the probability vector and the transition throughput using Jacobi's method. The results are saved in <netname>.prob and <netname>.sta. It returns -1 in case of error 0 otherwise.
    bool JacobiSolver();
    //@}

    // Prints any dd_edge in readable format
    void show_markings(ostream& out, const dd_edge& e, int max_markings=100);


    //! \name Methods use to produce output
    //@{
    //!It returns the RS cardinality
    // void getSizeRS(cardinality_t& card) {
    //     MEDDLY::apply(CARDINALITY, rs, cardinality_ref(card));
    // }
        
    //!It prints the RS and the domain info.
    void statistic() {
        MEDDLY::ostream_output stdout_wrap(cout);
        cout << "\n-----------RS-----------\n";
        rs.show(stdout_wrap, 2);

        if(useMonolithicNSF()){
            cout << "\n---------RG---------\n";
            NSF.show(stdout_wrap, 2);
        }
        //d->showInfo(stdout);
    }
    //@}
    //! \name Methods use to get from the transition/place name its corresponding integer.
    //@{
    //!It takes in input a place name and returns its corresponding interger.
    int findPlaceIdByName(const char* name) {
        auto it = S2Ipl.find(name);
        if (it == S2Ipl.end())
            return -1;
        return it->second;
    }
    // //!It takes in input a integer and returns its corresponding place name.
    // const char* getPL(const int id) {
    //     return tabp[id].place_name;
    // }

    int getMaxValueOfPlaceRS(const int place) {
        if (realBounds.empty())
            throw rgmedd_exception("Compute the RS first");
        assert(place >= 0 && place < npl);
        return realBounds[net_to_mddLevel[place]];

        // if ((var <= 0) || (var > npl)) { // variables are 1-based
        //     cerr << "var = " << var << ", npl = " << npl << endl;
        //     throw rgmedd_exception("variable index out-of-bound.");
        // }
        // return realBounds[id - 1];
    }

    // return the maximum number of tokens that can be found in any TRG marking
    int get_token_sum_bound() const { return maxSumTokens; }

    //!It takes in input a transition name and returns its corresponding interger.
    int findTransitionIdByName(const char* name) {
        auto it = S2Itr.find(name);
        if (it == S2Itr.end())
            return -1;
        return it->second;
    }
    //!It takes in input a integer and returns its corresponding transition name.
    // const char* getTR(const int id) {
    //     return tabt[id].trans_name;
    // }
    //@}
    //! \name Methods use to get/set file name.
    //@{
    //!It returns the net name
    const std::string& getNetName() { return netname;}
    //!It returns the property file name
    const std::string& getPropName() { return propname;}
    //!It sets the property file name
    void setPropName(std::string propname) {
        this->propname = propname;
    }

    void setFastNSFGen(bool f) { fast_NSF_gen = f; }
    void printGuessedBounds(bool f) { print_guessed = f; }
    

    const dd_edge& getRS() const  { return rs;}
    const dd_edge& getLRS() const  { return lrs;}
    // It returns the MDD encoding the initial marking
    const dd_edge& getInitMark() const  { return initMark;}
    // It returns the MDD encoding the RS + its enumeration
    const dd_edge& getIndexRS() const { return indexrs;}
    // It returns the MXD encoding the Next State Function
    const dd_edge& getNSF() const  { return NSF;}
 

    // It returns the DD domain
    domain *getDomain() const  { return dom;}

    forest* getForestMDD() const { return forestMDD; }
    forest* getForestMxD() const { return forestMxD; }

    // It returns the vector encoding the place ordering
    // const int *getPlaceOrd() const { return net_to_mddLevel.data(); }
    //@}

    // Converts a place id into its corresponding MDD variable (0-based)
    int convertPlaceToMDDLevel(int place) const { return net_to_mddLevel.at(place); }
    
    // Converts a (0-based) MDD variable level into the place name it is encoding
    const char* placeNameOfMDDVar(int var) const { return tabp[mddLevel_to_net.at(var)].place_name; }
};

// Prints the RS for the model.rg file
ostream &operator<<(ostream &out, class RSRG &rs);

// Print a DD in dot format
void write_dd_as_dot(const RSRG* rs, const dd_edge& e, 
                     const char* dot_name, bool level_labels);

//!Class RSRGAuto
/*!
This class is used to manage the RS augmented with the Automaton
*/
// class RSRGAuto : public RSRG {
//     int Top;
//     int Bot;
// //op_info* OpPOST;

// //expert_compute_manager* expcm;
//     forest *fRG;
// public:
//     //! Empty constructor
//     RSRGAuto() { }
//     //!Constructor: it takes in input the  place number and the net name; and initializes the domain and the forest. Observe that the last level (|P|+2) is used to encode the automaton state.
//     RSRGAuto(const int &, std::string);
//     bool init_RS(const Net_Mark_p &net_mark);
//     //! it takes in input matrices I,O,H for a transition and its type and computes its next state function.
//     bool MakeNextState(const int *F, const int *T, const int *H, const int tt, int AA, map <int, map <int, class ATRANSITION> > &automa);
//     //! it takes in input matrices I,O,H for a transition and its type and computes its next state function  considering the automanton state.
//     bool MakeNextState(const int *F, const int *T, const int *H, const int tt, int AA);
//     //! it computes the RG and saves it in the wngr file according to GreatSPN format. Variable outState  enables the saving in the file *.outT
//     bool ComputingRG(const bool &output, FILE *, FILE *, const int &Top, const int &Bot, set <int> &InitSet, bool outState);
//     //! it prints the RS
//     friend ostream &operator<<(ostream &out, class RSRGAuto &rs) {
//         //dd_edge::iterator i = rs.rs->begin();
//         enumerator i(*(rs.rs));
//         int nvar = rs.fRS->getDomain()->getNumVariables();
//         int val;
//         while (i != 0) {
//             for (int j = 1; j <= nvar; j++) {
//                 val = *(i.getAssignments() + j);
//                 if ((val != 0) && (j != nvar))
//                     out << rs.getPL(j - 1) << "(" << val << ")";
//                 if (j == nvar)
//                     out << " - L" << "(" << val << ")\n";
//             }//per ogni posto
//             ++i;
//         }//per ogni marcatura
//         return out;
//     }//fine stampa
//     //!it clears the next state functions
//     void clearNSF() {
//         NSF.clear();//timed transitions
//         NSFReal->clear();//timed transitions with rates
//         DiagReal->clear();//Matrix diagonal
//         NSFi->clear();//immediate transitions
//         for (int i = 0; i < ntr; i++) {
//             VectNSFReal[i]->clear();
//         }
//     }
//     //!It takes in input the transition id and its rate encoded in a double and compute the corresponding rate for marking dependent.
//     void  checkRate(float &val, const int *plmark);
// };

class QUASIPRODFOR {
private:
    domain *d;
    forest *fRS;
    forest *fEV;
    dd_edge *rs;
    dd_edge *indexrs;
    dd_edge *Mark;
    int **to;
    enumerator *it;
    int nvar;

public:
    QUASIPRODFOR(dd_edge *rs, dd_edge *indexrs, domain *d) {
        fRS = rs->getForest();
        fEV = indexrs->getForest();
        this->d = d;
        this->rs = rs;
        this->indexrs = indexrs;
        nvar = d->getNumVariables();
        //to = (int **) malloc(sizeof(int *));
        //to[0] = (int*) malloc(sizeof(int) * (nvar + 1));
        it = NULL;
        Mark = new dd_edge(fRS);
    }
    ~QUASIPRODFOR() {
        // free(to[0]);
        // free(to);
        if (it != NULL)
            delete it;
        delete Mark;
    }

    //! \name Methods use to quasi product form
    //@{
    //!It computes the intersection between input MDD and the MDD encoding the RS. The ouput is encoded on the input MDD.
    inline bool getState(int **ins) {

        fRS->createEdge(ins, 1, *Mark);
        apply(INTERSECTION, *Mark, *rs, *Mark);
        return true;
    };


    inline int initIterator() {
        it = new enumerator(*Mark);
        return Mark->getCardinality();
    };

    inline bool getMarking(int *vmark, long &idm) {
        const int *plmark;
        if (it == NULL || *it == 0)
            return false;
        else {
            plmark = it->getAssignments();
            fEV->evaluate((*indexrs), plmark, idm);
            memcpy(vmark, plmark, nvar * sizeof(int));
            ++(*it);
            return true;
        }
    }

    //@}
};


#endif /* MEDD_H_ */

