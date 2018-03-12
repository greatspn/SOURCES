//---------------------------------------------------------------------------------------
#ifndef __VARORDERS_H__
#define __VARORDERS_H__
//---------------------------------------------------------------------------------------
// Forward declarations
//---------------------------------------------------------------------------------------

class accumulator;

// Unit filter: maps [0..1] to [0..1] with a curve function
enum class UnitFilter {
    LINEAR,      // f(x) = x
    CUBIC,       // f(x) = x^3
    QUARTIC,     // f(x) = x^4
    EXP,         // f(x) = (e^x - 1) / (e - 1)
    INV_COSINE   // f(x) = 1 - cos(x * pi/2)
};

// Metrics for variable orders (Ciardo, Siminiceanu, 
//  in: New Metrics for Static Variable Ordering in Decision Diagrams)
double measure_NES(const std::vector<int> &varorder);
double measure_WESi(const std::vector<int> &varorder, int i);
// Experimental measures for event crossing
double measure_CRS(const std::vector<int> &varorder);
double measure_CRSi(const std::vector<int> &varorder, int i);
// Non-linear normalized event span
double measure_NLWES(const std::vector<int> &varorder, UnitFilter span_filter, int i);

// Normalized Variable Profile and Model Bandwidth
std::tuple<accumulator, accumulator, accumulator> 
measure_profile_bandwidth(const std::vector<int> &varorder);
// Normalized and Weighted-normalized Event Spans
std::pair<accumulator, accumulator> 
measure_NWES(const std::vector<int> &varorder);
// Event crossing metric
std::pair<accumulator, accumulator>
measure_WCRS(const std::vector<int> &varorder);

//---------------------------------------------------------------------------------------

// Connected components of the incidence relation, in topological order.
int compute_connected_components();

// Reorder out_order[] according to the SCC of the net.
// Move BSCC to the top variables, and move root nodes to the bottom.
void reorder_using_SCC(std::vector<int> &out_order, bool always_reorder);

// Get the vector of place component indices
const int* get_place_components();

//---------------------------------------------------------------------------------------

// Support structure that stores the Pre and post sets of the set of places
struct pre_post_sets_t {
    // Pre and Post sets of each place (transition indexes)
    std::vector<std::list<int>> preP, postP;

    void build();
};

//---------------------------------------------------------------------------------------

// Ordering algorithms
void var_order_from_file(std::vector<int> &out_order, const std::map<std::string, int>& S2Ipl);
void var_order_visit(const VariableOrderCriteria voc, std::vector<int> &order, 
                     const pre_post_sets_t& pps, int start=0);
void var_order_FORCE(const VariableOrderCriteria voc, std::vector<int> &out_order, 
                     const std::vector<int> &in_order);
void var_order_MeasFORCE(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_MeasFORCE_PINV(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_cuthill_mckee(const VariableOrderCriteria voc, std::vector<int> &out_order);
// void var_order_minimum_degree_ordering(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_king_ordering(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_sloan(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_noack_tovchigrechko(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_gradient_P(const VariableOrderCriteria voc, std::vector<int> &out_order);
void var_order_vcl(const VariableOrderCriteria voc, std::vector<int> &out_order);

// Order algorithm names
std::pair<const char*, const char*> var_order_name(VariableOrderCriteria voc);
// Returns true if the order methods needs P-invariants
bool method_uses_pinvs(VariableOrderCriteria voc);

// Write the ordered event matrix as a sparse matrix in NetPBM format
void write_var_order_as_NetPBM(const char* filename, const std::vector<int> &varorder);


void gen_varorder_metrics2(std::vector<double>& out_metrics, const std::vector<int> *var_order);
void generate_varorder_metrics2(const std::map<std::string, int>& S2Ipl); 

// Select the variable order which has the best weighted score
void metaheuristic_wscore(VariableOrderCriteria metavoc,
                          std::vector<int>& varorder, bool reorder_SCC,
                          const std::map<std::string, int>& S2Ipl);

//---------------------------------------------------------------------------------------

struct PSemiflowEntry {
    int place_no;
    int card;
};
typedef std::vector<PSemiflowEntry> PSemiflow;

bool load_Psemiflows(std::vector<PSemiflow> &psf);

bool load_flow_data(std::vector<PSemiflow> &psf, VariableOrderCriteria criteria);

//---------------------------------------------------------------------------------------

// NuPN support:
bool model_has_nested_units();
bool convert_nested_units_as_semiflows(std::vector<PSemiflow> &psf);
bool method_uses_nested_units(VariableOrderCriteria voc);

// On-the-fly clustering of places using MCL
void mcl_cluster_net(std::vector<PSemiflow>& out_psf, double inflation = 6.0);

//---------------------------------------------------------------------------------------
// Entry point - selects the appropriate method according to the criteria
//---------------------------------------------------------------------------------------

void determine_var_order(const VariableOrderCriteria criteria, 
                         bool save_image, bool reorder_SCC,
	                     const std::map<std::string, int>& S2Ipl, 
						 std::vector<int> &net_to_mddLevel,
                         bool verbose) ;

//---------------------------------------------------------------------------------------

// Basic accumulator of statistics measures
class accumulator {
    double m_sum = 0, m_sum2 = 2;
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
        return m == 0 ? 0 : mean2() / (m * m); 
    }
    inline double stddev() const { return sqrt(variance()); }
    inline double root_mean_sq() const { return m_count ? sqrt(m_sum2 / m_count) : 0; }
};

//---------------------------------------------------------------------------------------
#endif // __VARORDERS_H__
//---------------------------------------------------------------------------------------
