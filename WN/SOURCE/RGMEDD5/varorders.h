//---------------------------------------------------------------------------------------
#ifndef __VARORDERS_H__
#define __VARORDERS_H__
//---------------------------------------------------------------------------------------
// Forward declarations
//---------------------------------------------------------------------------------------

class accumulator;

//---------------------------------------------------------------------------------------

// struct PSemiflowEntry {
//     int place_no;
//     int card;

//     // // does this entry refer to a support variable (which is not a place of the net, but a virtual extra place)
//     // inline bool is_support_var(int num_pl) const { return place_no >= num_pl; }
// };
// typedef std::vector<PSemiflowEntry> PSemiflow;

//---------------------------------------------------------------------------------------

// P-semiflows with strict equivalence, i.e.:   m*Y == m0*Y, forall m in RS(m0)
const flow_basis_t& load_Psemiflows();

// P-semiflows with <= relation, i.e.:   m*Y <= m0*Y, forall m in RS(m0)
const flow_basis_t& load_Psemiflows_leq();

const std::vector<int>& load_Psemiflow_consts();
const std::vector<int>& load_Psemiflow_leq_consts();


// Returns the number of P-semiflows (0 if there are no P-invariants, or if the
// model has an empty .pin file)
int get_num_Psemiflows();

//---------------------------------------------------------------------------------------

// Get the basis of P-invariants from file (netname.pba)
const flow_basis_t& get_flow_basis();
bool load_flow_basis();
const std::vector<int>& load_flow_consts();
size_t get_num_invariants();
int get_max_invariant_coeff();
bool is_net_covered_by_flow_basis();

bool all_places_are_covered(const flow_basis_t&);

//---------------------------------------------------------------------------------------

// Get the P-flows from file (netname.pfl)
const flow_basis_t& get_pflows();
bool load_pflows();
size_t get_num_pflows();

//---------------------------------------------------------------------------------------

#undef NONE
// Variant type of a metric value
enum class metric_value_type { CARD, DOUBLE, UINT64, NONE };

//---------------------------------------------------------------------------------------

// Store a metric value. Different metrics could have different types, hence the
// class is a variant type.
class metric_t {
    cardinality_t value_c;
    double        value_d;
    uint64_t        value_s;
    metric_value_type type;
public:
    metric_t() : type(metric_value_type::NONE) {}
    metric_t(cardinality_t&& c) : value_c(c), type(metric_value_type::CARD) {}
    metric_t(cardinality_t c) : value_c(c), type(metric_value_type::CARD) {}
    metric_t(double d) : value_d(d), type(metric_value_type::DOUBLE) {}
    metric_t(uint64_t s) : value_s(s), type(metric_value_type::UINT64) {}

    metric_t(const metric_t&) = default;
    metric_t(metric_t&&) = default;
    metric_t& operator=(const metric_t&) = default;
    metric_t& operator=(metric_t&&) = default;

    // metric_t& operator=(cardinality_t&& c) { *this = metric_t(c); return *this; }
    metric_t& operator=(cardinality_t c) { *this = metric_t(c); return *this; }
    metric_t& operator=(double d) { *this = metric_t(d); return *this; }
    metric_t& operator=(uint64_t s) { *this = metric_t(s); return *this; }

    double& as_double() { type=metric_value_type::DOUBLE; return value_d; }
    cardinality_t& as_card() { type=metric_value_type::CARD; return value_c; }
    uint64_t& as_uint64() { type=metric_value_type::UINT64; return value_s; }

    inline double cast_to_double();

    friend inline bool operator<(const metric_t&m1, const metric_t& m2);
    friend inline bool operator==(const metric_t&m1, const metric_t& m2);
    friend inline bool operator!=(const metric_t&m1, const metric_t& m2);
    friend inline ostream& operator<<(ostream& os, const metric_t& m);
};

inline double metric_t::cast_to_double() {
    switch (type) {
        case metric_value_type::CARD:    return get_double(value_c);
        case metric_value_type::DOUBLE:  return value_d;
        case metric_value_type::UINT64:  return value_s;
        default: throw rgmedd_exception();
    }
}

inline bool operator<(const metric_t&m1, const metric_t& m2) {
    assert(m1.type == m2.type);
    switch (m1.type) {
        case metric_value_type::CARD:    return m1.value_c < m2.value_c;
        case metric_value_type::DOUBLE:  return m1.value_d < m2.value_d;
        case metric_value_type::UINT64:  return m1.value_s < m2.value_s;
        default: throw rgmedd_exception();
    }
}
inline bool operator==(const metric_t&m1, const metric_t& m2) {
    assert(m1.type == m2.type);
    switch (m1.type) {
        case metric_value_type::CARD:    return m1.value_c == m2.value_c;
        case metric_value_type::DOUBLE:  return m1.value_d == m2.value_d;
        case metric_value_type::UINT64:  return m1.value_s == m2.value_s;
        default: throw rgmedd_exception();
    }
}
inline bool operator!=(const metric_t&m1, const metric_t& m2) {
    assert(m1.type == m2.type);
    switch (m1.type) {
        case metric_value_type::CARD:    return m1.value_c != m2.value_c;
        case metric_value_type::DOUBLE:  return m1.value_d != m2.value_d;
        case metric_value_type::UINT64:  return m1.value_s != m2.value_s;
        default: throw rgmedd_exception();
    }
}
inline ostream& operator<<(ostream& os, const metric_t& m) {
    switch (m.type) {
        case metric_value_type::CARD:    return os << m.value_c;
        case metric_value_type::DOUBLE:  return os << m.value_d;
        case metric_value_type::UINT64:  return os << m.value_s;
        default: throw rgmedd_exception();
    }
}

//---------------------------------------------------------------------------------------

// Compute several metric values in a single step
void metric_compute(const std::vector<int>& net_to_level,
                    trans_span_set_t& trn_set,
                    flow_basis_metric_t* opt_fbm,
                    metric_t* out_metrics[NUM_METRICS]);

// Compute a single metric value
void metric_compute(const std::vector<int>& net_to_level,
                    trans_span_set_t& trn_set,
                    flow_basis_metric_t* opt_fbm,
                    VariableOrderMetric m,
                    metric_t* out);

//---------------------------------------------------------------------------------------
// // Unit filter: maps [0..1] to [0..1] with a curve function
// enum class UnitFilter {
//     LINEAR,      // f(x) = x
//     CUBIC,       // f(x) = x^3
//     QUARTIC,     // f(x) = x^4
//     EXP,         // f(x) = (e^x - 1) / (e - 1)
//     INV_COSINE   // f(x) = 1 - cos(x * pi/2)
// };

// Metrics for variable orders (Ciardo, Siminiceanu, 
//  in: New Metrics for Static Variable Ordering in Decision Diagrams)
double measure_NES(const std::vector<int> &varorder);
double measure_WESi(const std::vector<int> &varorder, int i);
// Experimental measures for event crossing
// double measure_CRS(const std::vector<int> &varorder);
// double measure_CRSi(const std::vector<int> &varorder, int i);
// Non-linear normalized event span
// double measure_NLWES(const std::vector<int> &varorder, UnitFilter span_filter, int i);

// event spans weighted by independent per-level weights
uint64_t      measure_weighted_event_spans(const std::vector<int> &varorder, const std::vector<size_t> *pW);
double        measure_weighted_event_spans(const std::vector<int> &varorder, const std::vector<double> *pW);
cardinality_t measure_weighted_event_spans(const std::vector<int> &varorder, const std::vector<cardinality_t> *pW);

// Normalized Variable Profile and Model Bandwidth
std::tuple<accumulator, accumulator, accumulator> 
measure_profile_bandwidth(const std::vector<int> &varorder);
// Normalized and Weighted-normalized Event Spans
// std::pair<accumulator, accumulator> 
// measure_NWES(const std::vector<int> &varorder);
// Event crossing metric
// std::pair<accumulator, accumulator>
// measure_WCRS(const std::vector<int> &varorder);

// Sum of P-semiflows spans
int measure_PSF(const std::vector<int> &varorder);
// Sum of P-flows spans
int measure_PF(const std::vector<int> &varorder);
// // Sum of Nested Units spans
// int measure_NUS(const std::vector<int> &varorder);
// Sum of tops
int measure_SOT(const std::vector<int> &varorder);
// Sum of spans
uint64_t measure_SOS(const std::vector<int> &varorder);

// Computes the FORCE metric (point-transition spans) as defined for the FORCE algorithm
double measure_FORCE_pts(const VariableOrderCriteria voc, const std::vector<int> &in_order, 
                         const flow_basis_t& psf);

// Bandwidth-reduction metrics (bandwidth, profile, wavefront)
void metrics_bandwidth(const std::vector<int>& net_to_level, uint64_t *BW, uint64_t *PROF,
                       double *AVGWF, double *MAXWF, double *RMSWF);

struct trans_span_set_t;
struct flow_basis_metric_t;

std::shared_ptr<trans_span_set_t> make_trans_span_set();
std::shared_ptr<flow_basis_metric_t> make_flow_basis_metric();

typedef uint64_t swir_score_t;

// --- SOUPS Metric ----------------------------------------------------------------------

uint64_t measure_soups(const std::vector<int> &varorder, trans_span_set_t& trn_set,
                       bool test_productiveness = true);

// --- SWIR Metric ----------------------------------------------------------------------

uint64_t      measure_swir(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                           const std::vector<size_t>& lvl_weights, bool test_productiveness = true);
double        measure_swir(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                           const std::vector<double>& lvl_weights, bool test_productiveness = true);
cardinality_t measure_swir(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                           const std::vector<cardinality_t>& lvl_weights, bool test_productiveness = true);

// --- SOUPS Metric ----------------------------------------------------------------------

// uint64_t measure_soups_X(const std::vector<int> &varorder, trans_span_set_t& trn_set);

// SOUPS with correction for isolated level boundaries
uint64_t measure_soups2(const std::vector<int> &varorder, trans_span_set_t& trn_set,
                        bool test_productiveness = true);

// uint64_t measure_soups3(const std::vector<int> &varorder, trans_span_set_t& trn_set);

// --- SWIR Metric ----------------------------------------------------------------------

// uint64_t      measure_swir_X(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//                              const std::vector<size_t>& lvl_weights);
// double        measure_swir_X(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//                              const std::vector<double>& lvl_weights);
// cardinality_t measure_swir_X(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//                              const std::vector<cardinality_t>& lvl_weights);

// SWIR with correction for isolated level boundaries
uint64_t      measure_swir2(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                            const std::vector<size_t>& lvl_weights, bool test_productiveness = true);
double        measure_swir2(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                            const std::vector<double>& lvl_weights, bool test_productiveness = true);
cardinality_t measure_swir2(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
                            const std::vector<cardinality_t>& lvl_weights, bool test_productiveness = true);

// uint64_t      measure_swir3(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//                             const std::vector<size_t>& lvl_weights);
// double        measure_swir3(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//                             const std::vector<double>& lvl_weights);
// cardinality_t measure_swir3(const std::vector<int> &varorder, trans_span_set_t& trn_set, 
//                             const std::vector<cardinality_t>& lvl_weights);

//---------------------------------------------------------------------------------------

// Extra per-level informations shown in the EPS
struct LevelInfoEPS {
    std::string header, footer;
    const char** colors;
    std::vector<std::string> info;
    std::vector<bool> bold;
    std::vector<int> clrIndex;
    double width = 0;
    double startX = 0;
};

struct DDEPS {
    const char* fname = nullptr; // filename of the DD
    double x, y, w, h; // bounding box
};

// Write the incidence matrix and the invariants matrix in EPS format
void write_incidence_as_EPS(const char* filename, const trans_span_set_t &trn_set,
                            const std::vector<int> &varorder, const flow_basis_t& basis, 
                            const std::vector<std::vector<std::string>>* rangeMat = nullptr,
                            LevelInfoEPS* ppLvlInfo[] = nullptr, const size_t numLvlInfo = 0,
                            const DDEPS* ddeps = nullptr, bool write_trn_matrix = true);

//---------------------------------------------------------------------------------------

// only_ranks:  just compute the ranks of the footprint form of the basis, 
//              do not compute ranges.
// use_ilp:  range computation is improved by solving two ILPs for each range
// use_enum: range compuation is improved by actually enumerating the PSI tuples
// print_enums:  print all the tuples generated during the process of enumeration
cardinality_t 
measure_PSI(const std::vector<int> &varorder, bool only_ranks,
            bool use_ilp, bool use_enum, bool print_enums,
            flow_basis_metric_t& fbm);

cardinality_t measure_score_experimental(flow_basis_metric_t& fbm, int var);

void print_PSI_diagram(const std::vector<int> &varorder, flow_basis_metric_t& fbm);

void annealing_compact(std::vector<int> &net_to_level, flow_basis_metric_t& fbm);
void compact_basis_row_min(std::vector<int> &net_to_level, flow_basis_metric_t& fbm);

const cardinality_t& 
estimate_nodes_at_level(const flow_basis_metric_t& fbm, int lvl);

// get the count of the active invariants for each level level.
// The basis must already be ordered (like after calling measure_PSI)
void get_lvl_weights_invariants(const flow_basis_metric_t& fbm, std::vector<size_t>& lvl_weights);

// get the level weights made by the active invariants and the place bounds
void get_lvl_weights_invariants_bounds(const flow_basis_metric_t& fbm, const std::vector<int> &net_to_level,
                                       std::vector<size_t>& lvl_weights);

// get the level weight estimated using invariant ranges
void get_lvl_weights_ranges(const flow_basis_metric_t& fbm, std::vector<cardinality_t>& lvl_weights);

std::vector<std::vector<std::string>>
range_matrix_for_representation(flow_basis_metric_t& fbm);

size_t
range_prod_for_representation(flow_basis_metric_t& fbm, std::vector<std::string>& RP);

// cardinality_t
// lvl_combinations_for_representation(flow_basis_metric_t& fbm, std::vector<std::string>& RP);

// compute the basis and the level ranks
void compute_lvl_weights(const std::vector<int> &net_to_level,
                         flow_basis_metric_t& fbm, std::vector<size_t>& lvl_weights);


const flow_basis_t& get_basis(const flow_basis_metric_t& fbm);

// Reorder p-flows' variables according to a variable order
void reorder_basis(flow_basis_t& B, const std::vector<int>& net_to_level);
// also canonicalize the vector of invariant constants
void reorder_basis(flow_basis_t& B, std::vector<int>& consts, const std::vector<int>& net_to_level);

// transitions/invariants force
void var_order_ti_force(flow_basis_metric_t& fbm, 
                        const trans_span_set_t& trns_set,
                        std::vector<int>& net_to_level,
                        bool use_trn_cogs,
                        bool verbose);

// The metric score computed by TI-Force
double
metric_PTS_INV(flow_basis_metric_t& fbm, 
               const trans_span_set_t& trns_set,
               const std::vector<int>& net_to_level,
               bool use_weights);

// Get a discount factor for SOUPS-based metrics
double get_soups_discount(const trans_span_set_t &trns_set);

//---------------------------------------------------------------------------------------

typedef std::function<void(const std::vector<int>& net_to_level, std::vector<int>& new_order)> relocation_fn_t;

relocation_fn_t
make_invariant_group_repositioner(flow_basis_metric_t *fbm, const trans_span_set_t& trns_set, bool force_step);


//-------------------

// size_t measure_SoIR(const std::vector<int> &net_to_level);
// void compact_basis_row_min(std::vector<int> &net_to_level);

//---------------------------------------------------------------------------------------

// // Trivial syphons/traps (trivial = non minimal)
// // The integer value is the nesting level of the syphon/trap place. 
// // 0 means not a trivial syphon/trap.
// struct SyphonsTraps {
//     std::vector<int> syph_trap;   // >0 means trivial syphon. <0 means trivial trap.
//     int num_syphons = 0;
//     int num_traps = 0;

//     inline bool is_trivial_syphon(int pl) const { return syph_trap[pl] > 0; }
//     inline bool is_trivial_trap(int pl) const { return syph_trap[pl] < 0; }
// };
// void classify_trivial_syphons_traps(SyphonsTraps& st);

// // Reorder out_order[] moving trap places to the top, anc syphon places to the bottom
// void reorder_non_ergodic(std::vector<int> &out_order, const SyphonsTraps& st);

// Connected components of the incidence relation, in topological order.
int compute_connected_components();

// Reorder out_order[] according to the SCC of the net.
// Move BSCC to the top variables, and move root nodes to the bottom.
void reorder_using_SCC(std::vector<int> &out_order);

// Get the vector of place component indices
const int* get_place_components();

// Number of SCCs in the net
int get_num_components();

// Reorder using metric minimization and simulated annealing
void simulated_annealing(std::vector<int>& varorder, 
                         trans_span_set_t& trn_set, 
                         flow_basis_metric_t* opt_fbm,
                         VariableOrderMetric target_metric,
                         bool use_group_repos,
                         bool apply_group_force);

void annealing_force(const Annealing ann,
                     std::vector<int>& out_order,
                     trans_span_set_t& trn_set,
                     flow_basis_metric_t* opt_fbm,
                     VariableOrderMetric target_metric);

// Tell if a transition is local to a component, or if it connectes
// places in multiple components
// bool is_transition_SCC_local(int tr);

// static SyphonsTraps net_syphon_traps;
// static bool syphon_traps_init = false;
//---------------------------------------------------------------------------------------

// Support structure that stores the Pre and post sets of the set of places
struct pre_post_sets_t {
    // Pre and Post sets of each place (transition indexes)
    std::vector<std::list<int>> preP, postP;

    void build();
};

const pre_post_sets_t& get_pre_post_sets();

//---------------------------------------------------------------------------------------

// Ordering algorithms
void var_order_from_file(std::vector<int> &out_order, const std::map<const char*, int, cstr_less>& S2Ipl);
void var_order_visit(const VariableOrderCriteria voc, std::vector<int> &order, int start=0);
void var_order_FORCE(const VariableOrderCriteria voc, std::vector<int> &out_order, 
                     const std::vector<int> &in_order,  const flow_basis_t& psf, bool verbose);
// void var_order_MeasFORCE(const VariableOrderCriteria voc, std::vector<int> &out_order);
// void var_order_MeasFORCE_PINV(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_cuthill_mckee(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_king_ordering(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_sloan(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_noack_tovchigrechko(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_Noack_Tovchigrechko_fast(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_gradient_P(const VariableOrderCriteria voc, const flow_basis_t& psf, std::vector<int> &out_order);
void var_order_vcl(const VariableOrderCriteria voc, std::vector<int> &out_order);
void varorder_P_chaining(const flow_basis_t& PSF, std::vector<int>& new_map_sort);

// Order algorithm names
std::pair<const char*, const char*> var_order_name(VariableOrderCriteria voc);
// Returns true if the order methods needs P-invariants
bool method_uses_pinvs(VariableOrderCriteria voc);

// // Generate the set of a-priori metrics from the net graph structure
// void gen_net_metrics(std::vector<double>& out_metrics, bool pinv_metrics);
// // Generate the set of a-posteriori metrics from a given variable order
// void gen_varorder_metrics(std::vector<double>& out_metrics, const std::vector<int> *var_order);

// // Generate a comma-separated list of graph metrics
// // void generate_netgraph_metrics(std::vector<double>& out_metrics, bool pinv_metrics, bool bandwidth_metrics);
// void generate_varorder_metrics(std::vector<double>& out_metrics, bool pinv_metrics, 
//                                bool reorder_SCC, const std::map<const char*, int, cstr_less>& S2Ipl);
// Write the ordered event matrix as a sparse matrix in NetPBM format
// void write_var_order_as_NetPBM(const char* filename, const std::vector<int> &varorder);

// // Internal metric test
// void test_metric_scores();

// // Meta-heuristics:
// void metaheuristic_by_score(VariableOrderCriteria metavoc, std::vector<int>& varorder, 
// 	                        bool reorder_SCC, const std::map<const char*, int, cstr_less>& S2Ipl);
// VariableOrderCriteria
// metaheuristic_table(std::vector<int>& varorder, bool reorder_SCC, const std::map<const char*, int, cstr_less>& S2Ipl);


// void gen_varorder_metrics2(std::vector<double>& out_metrics, const std::vector<int> *var_order);
// void generate_varorder_metrics2(const std::map<const char*, int, cstr_less>& S2Ipl, 
//                                 trans_span_set_t &trns_set, 
//                                 flow_basis_metric_t& fbm);

enum variable_order_variation_type {
    VT_NONE, VT_FORCE, VT_COMPACT
};
extern const char *g_variable_order_variation_type[];

void var_order_meta_force(std::vector<int>& best_net_to_level,
                          trans_span_set_t& trn_set,
                          flow_basis_metric_t* opt_fbm,
                          VariableOrderMetric m,
                          const size_t num_attempts);
// fields that can be serialized by direct memcpy
struct score_vo_base_t {
    VariableOrderCriteria voc = VOC_NO_REORDER;
    variable_order_variation_type variation_type = VT_NONE;
    double score = -1;
    double score_ex2 = -1, score_ex3 = -1, score_ex4 = -1, weight = -1;
    double seconds_to_build = 0;
};

struct score_vo_t : public score_vo_base_t {
    std::vector<int> order;

    score_vo_t() {}
    score_vo_t(int n) : order(n) {}
    score_vo_t(const score_vo_t&) = default;
    score_vo_t(score_vo_t&&) = default;
    score_vo_t& operator=(const score_vo_t&) = default;
    score_vo_t& operator=(score_vo_t&&) = default;

    inline bool valid() const { return !order.empty(); }
    inline const char* variation() const 
    { return g_variable_order_variation_type[variation_type]; }

    inline bool operator<(const score_vo_t& sv) const { return score < sv.score; }
};

// // Select the variable order which has the best weighted score
// void metaheuristic_wscore(VariableOrderCriteria metavoc,
//                           std::vector<score_vo_t>& varorders,
//                           size_t max_varorders,
//                           VariableOrderMetric& ann_metric,
//                           trans_span_set_t &trns_set,
//                           flow_basis_metric_t& fbm,
//                           const std::map<const char*, int, cstr_less>& S2Ipl);

//---------------------------------------------------------------------------------------

struct metaheuristic_context_t {
    VariableOrderCriteria metavoc;
    trans_span_set_t &trns_set;
    flow_basis_metric_t& fbm;
    const std::map<const char*, int, cstr_less>& S2Ipl;
    // output section
    bool has_psf;
    bool has_nu;
    bool has_scc;
    bool is_multi;
    VariableOrderMetric ann_metric;
};

void metaheuristic(metaheuristic_context_t& mhctx,
                   std::vector<score_vo_t>& varorders,
                   size_t max_varorders);

//---------------------------------------------------------------------------------------

// NuPN support:
bool model_has_nested_units();
const flow_basis_t& convert_nested_units_as_semiflows();
bool method_uses_nested_units(VariableOrderCriteria voc);

// On-the-fly clustering of places using MCL
// const flow_basis_t&
// mcl_cluster_net(size_t timeout = 1000, double inflation = 6.0);

// Get a measure fo the clustering level obtained by MCL
// double get_cluster_isolation_coeff(const flow_basis_t& mcl_psf);

//---------------------------------------------------------------------------------------
// Entry point - selects the appropriate method according to the criteria
//---------------------------------------------------------------------------------------

void determine_var_order(const var_order_selector& sel, 
                         const std::map<const char*, int, cstr_less>& S2Ipl, 
                         std::vector<int> &net_to_mddLevel,
                         trans_span_set_t &trns_set,
                         flow_basis_metric_t& fbm);

//---------------------------------------------------------------------------------------

// Basic accumulator of statistics measures
class accumulator {
    double m_sum = 0, m_sum2 = 2; // TODO: why m_sum2=2 ???
    double m_min = 0, m_max = 0;
    size_t m_count = 0;
public:
    accumulator() {}
    accumulator(const accumulator&) = default;
    accumulator(accumulator&&) = default;

    accumulator& operator = (const accumulator&) = default;
    accumulator& operator = (accumulator&&) = default;

    inline void operator += (double value) {
        m_sum += value;
        m_sum2 += (value * value);
        if (count() == 0)
            m_min = m_max = value;
        m_min = std::min(m_min, value);
        m_max = std::max(m_max, value);
        m_count++;
    }

    inline double count() const { return m_count; }
    inline double min() const { return m_min; }
    inline double max() const { return m_max; }
    inline double sum() const { return m_sum; }

    inline double mean() const { return m_count ? (m_sum / m_count) : 0; }
    inline double mean2() const { return m_count ? (m_sum2 / m_count) : 0; }
    inline double variance() const { 
        double m = mean();
        return (m == 0) ? 0 : (mean2() - (m * m)); 
    }
    inline double stddev() const { return sqrt(variance()); }
    inline double root_mean_sq() const { return m_count ? sqrt(m_sum2 / m_count) : 0; }
};

//---------------------------------------------------------------------------------------
#endif // __VARORDERS_H__
//---------------------------------------------------------------------------------------
