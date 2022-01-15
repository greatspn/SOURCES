//---------------------------------------------------------------------------------------
#ifndef __I_RANK_H__
#define __I_RANK_H__
//---------------------------------------------------------------------------------------

// Forward declaration
typedef struct _lprec lprec; // from <lp_lib.h>

//---------------------------------------------------------------------------------------
// Integer range - used to approximate token ranges in p-basis metric
//---------------------------------------------------------------------------------------

struct int_range_t {
    inline int_range_t() {}
    inline int_range_t(int _l, int _r) : l(_l), r(_r) {}
    inline ~int_range_t() {}
    inline int_range_t(const int_range_t&) = default;
    inline int_range_t(int_range_t&&) = default;
    inline int_range_t& operator=(const int_range_t&) = default;
    inline int_range_t& operator=(int_range_t&&) = default;

    inline int_range_t& operator+=(const int_range_t& i) { l+=i.l; r+=i.r; return *this; }
    inline int_range_t& operator-=(const int_range_t& i) { l-=i.r; r-=i.l; return *this; }
    inline int_range_t& operator*=(int c) { l*=c; r*=c; return *this; }
    inline int_range_t operator+(const int_range_t& i) const { return int_range_t(l+i.l, r+i.r); }
    inline int_range_t operator-(const int_range_t& i) const { return int_range_t(l-i.r, r-i.l); }
    inline int_range_t operator*(int c) const { return int_range_t(c*l, c*r); }
    inline int_range_t operator/(int c) const { return int_range_t(l/c, r/c); }

    inline bool operator==(const int_range_t& i) const { return l==i.l && r==i.r; }
    inline bool operator!=(const int_range_t& i) const { return l!=i.l && r!=i.r; }

    inline int size() const { return (r - l + 1); }
    inline int size_min1() const { return /*max(1, size())*/ size(); }
    inline bool inside(int v) const { return (l <= v) && (v <= r); }

    inline std::string str() const;

    int l, r; // [left, right] values of the range
};

inline int_range_t operator*(int c, const int_range_t& i) { 
    return int_range_t(c*i.l, c*i.r); 
}
inline ostream& operator<<(ostream& os, const int_range_t& i) {
    return os << "[" << i.l << "," << i.r << "]";
}
inline std::string int_range_t::str() const { 
    ostringstream o; o << *this; return o.str(); 
}

inline int_range_t make_range(int i0, int i1) {
    return int_range_t(std::min(i0, i1), std::max(i0, i1));
}
inline int_range_t intersect(const int_range_t& i1, const int_range_t& i2) {
    return int_range_t(std::max(i1.l, i2.l), std::min(i1.r, i2.r));
}
inline int_range_t merge(const int_range_t& i1, const int_range_t& i2) {
    return int_range_t(std::min(i1.l, i2.l), std::max(i1.r, i2.r));
}

//---------------------------------------------------------------------------------------
// Footprint form
//---------------------------------------------------------------------------------------

// Get the basis B in row footprint form
void row_footprint_form(int_lin_constr_vec_t& B);

// Get the basis B into reduced row footprint form
void reduced_row_footprint_form(int_lin_constr_vec_t& B);

void print_flow_basis(const int_lin_constr_vec_t& B);
void print_flow_basis(const std::vector<sparse_vector_t>& B);

// Reorder p-flows' variables according to a variable order
void reorder_basis(int_lin_constr_vec_t& B, const std::vector<int>& net_to_level);
// also canonicalize the vector of invariant constants
// void reorder_basis(int_lin_constr_vec_t& B, std::vector<int>& consts, const std::vector<int>& net_to_level);

//---------------------------------------------------------------------------------------
// Support structure for the computation of P-flow based metrics
//---------------------------------------------------------------------------------------
class iRank2Support;

struct flow_basis_metric_t {
    // Place bound of the model (if available)
    const int* available_place_bounds;
    // Alternative bound value if bounds are not available
    int alternate_bound = -1;
    // The sorted and reduced flow basis
    int_lin_constr_vec_t B;
    // The flow invariant' constants (flow * m0)
    // std::vector<int> inv_coeffs;
    // level to place mapping
    std::vector<int> level_to_net;
    // The token range we have to remember for an invariant at each level.
    // The vector has one non-zero for each non-zero in B.
    std::vector<std::vector<int_range_t>> ranges;
    // The expected combinations of nodes at the given level
    std::vector<cardinality_t> combinations;
    // The number of enumerated nodes at each level
    std::vector<size_t> count_uPSIs;
    // Product of each invariants with m0
    // std::vector<int> inv_coeff;

    // Should we just compute ranks and not ranges?
    bool only_ranks = false;
    // Should we use ILP to tighten the ranges
    bool use_ilp = false;
    // Should we use explicit enumeration to tighten the ranges
    bool use_enum = false;
    bool print_enums = false;
    // invariant coefficients are computed from m0 or loaded from file?
    // bool using_inv_coeffs_from_file = false;

    flow_basis_metric_t() {}
    ~flow_basis_metric_t();
    void initialize();

    // reset the basis B and the invariant coefficients
    void reset_B_and_inv_coeffs(const std::vector<int> &net_to_level);

    // Compute the PSI score
    cardinality_t measure_PSI(const std::vector<int> &net_to_level);

    // Print the diagram with the flow matrix and the invariant ranges
    void print_PSI_diagram(const std::vector<int> &net_to_level);

    // Simulated annealing with row compact strategy
    void annealing_compact(std::vector<int> &net_to_level);

    // New strategy close to the annealing which finds the minimum in each row
    void compact_basis_row_min(std::vector<int> &net_to_level);


    double compute_score_experimental_A(int var);

    // iRank2 experimental code
    unique_ptr<iRank2Support> p_irank2supp;
    void initialize_irank2();
    cardinality_t compute_iRank2_score(bool edge_based);

protected:

// #ifdef HAS_LP_SOLVE_LIB
    // The ILP structure
    lprec *lp = nullptr;
    // Extra pre-allocated data structure
    std::vector<double> ilp_row;
    std::vector<int> ilp_col;

    void delete_LP();
    void initialize_LP(const std::vector<int> &net_to_level);
    void solve_LP();
// #endif // HAS_LP_SOLVE_LIB

    // verify that the inv_coeffs[] vector is coherent with the flow*m0 products
    void verify_inv_coeffs() const;
    // Compte the rank-only score
    size_t compute_rank_score() const;
    // Change rows of B (preserving the footprint form) if the range space of a row reduces
    void optimize_B();
    // Compute the allowed ranges for a given p-flow r of B[]
    void compute_ranges_of_pflow(size_t r, bool verbose);
    // Compute the score from the ranges
    cardinality_t compute_score();
    // Compute how many combinations we have for p-flow r on all levels
    // where the p-flow is active
    cardinality_t estimate_range_space(size_t r);
    // Print the ILP in Mathematica format
    void print_ILP_Mathematica();
    // Enumerate the uPSI at each level, to get the exact # of them
    bool bound_by_enumeration();

    size_t extract_reinsert_row(size_t row, const std::vector<int>& net_to_level,
                                size_t* new_leading, std::vector<bool>& selected_cols);

public:
    // Do we have a basis of P-invariants?
    inline bool have_basis() const {
        return !B.empty();
    }

    // Check if we have the bound for a given place
    inline bool have_bound(int plc) const {
        return (available_place_bounds != nullptr &&
                available_place_bounds[plc] >= 0);
    }

    // Return the bounds of a place (if available)
    inline int get_bound(int plc) const {
        if (have_bound(plc))
            return available_place_bounds[plc];
        return alternate_bound;
    }

    // Return the range of tokens to remember for the specified invariant at the given level
    inline const int_range_t& invariant_range_at_lvl(int inv, int lvl) const {
        assert(B[inv].coeffs.leading() <= lvl && lvl <= B[inv].coeffs.trailing());
        // The array ranges[inv] does not have a non-zero for each level, but only
        // one non-zero for each non-zero in B[inv]. Get the closest level.
        int index = B[inv].coeffs.lower_bound_nnz(lvl);
        return ranges[inv][index];
    };

    std::vector<std::vector<double>> range_scores;
    inline const double range_scores_at_lvl(int inv, int lvl) const {
        assert(B[inv].coeffs.leading() <= lvl && lvl <= B[inv].coeffs.trailing());
        // The array ranges[inv] does not have a non-zero for each level, but only
        // one non-zero for each non-zero in B[inv]. Get the closest level.
        int index = B[inv].coeffs.lower_bound_nnz(lvl);
        return range_scores[inv][index];
    };
};

//---------------------------------------------------------------------------------------

std::shared_ptr<flow_basis_metric_t> make_flow_basis_metric();

//---------------------------------------------------------------------------------------
// New iRank2 support structures
//---------------------------------------------------------------------------------------

class iRank2Support {
    const flow_basis_metric_t& fbm;
    const int_lin_constr_vec_t& B;
public:
    iRank2Support(const flow_basis_metric_t& _fbm) : fbm(_fbm), B(_fbm.B) { }

    // vector (one entry per constraint level) containing
    // a map of distinct partial sums -> allowed variable assignments
    typedef std::vector<std::map<int, std::vector<int>>> psums_at_level_t; 
    // constraint -> vector of partial sums
    typedef std::vector<psums_at_level_t> constr_psums_t; 

    // Enumerate the constraint values at level of each constraint
    constr_psums_t irank2_constr_psums;

    bool verbose = false;
    // stored edge counts
    std::vector<size_t> edge_counts;

    // Return the new iRank2 range of values for constraint cc at level lvl
    inline const size_t irank2_constr_combinations_at_level(int cc, int lvl) const {
        assert(irank2_constr_psums.size() == B.size());
        assert(B[cc].coeffs.leading() <= lvl && lvl <= B[cc].coeffs.trailing());
        // The array ranges[cc] does not have a non-zero for each level, but only
        // one non-zero for each non-zero in B[cc]. Get the closest level.
        int index = B[cc].coeffs.lower_bound_nnz(lvl);
        assert(0 <= index && index < irank2_constr_psums[cc].size());
        if (B[cc].coeffs.ith_nonzero(index).index == lvl) // at level
            return irank2_constr_psums[cc][index].size();
        else // memory between levels
            return irank2_constr_psums[cc][index - 1].size();
    };

    void initialize();
    bool remove_unused_var_values();
    void print_constraints();
    cardinality_t compute_score_for_nodes();
    cardinality_t compute_score_for_edges();
    cardinality_t get_level_repr_for_nodes(std::vector<std::string>& RP) const;
    cardinality_t get_level_repr_for_edges(std::vector<std::string>& RP);
};

//---------------------------------------------------------------------------------------

cardinality_t irank2_repr_for_nodes(flow_basis_metric_t& fbm, std::vector<std::string>& RP);
cardinality_t irank2_repr_for_edges(flow_basis_metric_t& fbm, std::vector<std::string>& RP);

void experiment_cdd(const flow_basis_metric_t& fbm);
void experiment_footprint_chaining(const std::vector<int>& net_to_mddLevel);

//---------------------------------------------------------------------------------------
#endif // __I_RANK_H__
//---------------------------------------------------------------------------------------
