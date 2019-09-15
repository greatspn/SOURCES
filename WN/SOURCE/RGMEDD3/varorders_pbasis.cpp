#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <climits>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <memory>
#include <map>
#include <set>
#include <stack>
#include <cstdint>

#include "rgmedd3.h"
#include "varorders.h"
#include "bufferless_sort.h"
#include "optimization_finder.h"
// #include "utils/thread_pool.h"
#include "utils/mt64.h"
// #include "dlcrs_matrix.h"

#ifdef HAS_LP_SOLVE_LIB
#  undef NORMAL
#  include <lp_lib.h>
#else
#  warning "Missing lp_solve library. RGMEDD3 modules that use LP will not be compiled."
#endif // HAS_LP_SOLVE_LIB

extern size_t g_sim_ann_num_tentatives;

// Open a file with the default application/browser
int open_file(const char * filename);

//---------------------------------------------------------------------------------------

template<typename T, typename D>
inline void safe_div(T &value, const D divisor) {
    if (divisor == D(0))
        value = 0;
    else
        value /= divisor;
}

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
    inline int_range_t operator+(const int_range_t& i) { return int_range_t(l+i.l, r+i.r); }
    inline int_range_t operator-(const int_range_t& i) { return int_range_t(l-i.r, r-i.l); }
    inline int_range_t operator*(int c) { return int_range_t(c*l, c*r); }
    inline int_range_t operator/(int c) { return int_range_t(l/c, r/c); }

    inline bool operator==(const int_range_t& i) { return l==i.l && r==i.r; }
    inline bool operator!=(const int_range_t& i) { return l!=i.l && r!=i.r; }

    inline int size() const { return (r - l + 1); }
    inline bool inside(int v) const { return (l <= v) && (v <= r); }

    inline std::string str() const;

    int l, r; // [left, right] values of the range
};

inline int_range_t operator*(int c, const int_range_t& i) { 
    return int_range_t(c*i.l, c*i.r); 
}
ostream& operator<<(ostream& os, const int_range_t& i) {
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

//---------------------------------------------------------------------------------------
// Metric based on P-flow basis
//---------------------------------------------------------------------------------------

ostream& operator<<(ostream& os, const flow_basis_t& fb) {
    for (const sparse_vector_t& s : fb) {
        for (size_t i=0; i<s.size(); i++)
            os << setw(3) << s[i];
        os << endl;
    }
    return os;
}

//-----------------------------------------------------------------------------

inline int gcd(int a, int b) {
    assert(a >= 0 && b >= 0);
    if (a == 0)
        return b;

    while (b != 0) {
        if (a > b)
            a = a - b;
        else
            b = b - a;
    }
    return a;
}

//-----------------------------------------------------------------------------

inline int sign(int num) {
    if (num > 0)
        return +1;
    else if (num < 0)
        return -1;
    return 0;
}

//---------------------------------------------------------------------------------------

template<typename Container1, typename Container2>
inline ssize_t traverse_both(typename Container1::const_iterator& it1,
                             const Container1& cont1,
                             typename Container1::value_type& value1,
                             typename Container2::const_iterator& it2,
                             const Container2& cont2,
                             typename Container1::value_type& value2) 
{
    bool end1 = (it1 == cont1.end());
    bool end2 = (it2 == cont2.end());

    if (end1 && end2)
        return -1;

    if (end2 || (!end1 && it1->index < it2->index)) { // take from it1
        ssize_t index = ssize_t(it1->index);
        value1 = it1->value;
        value2 = 0;
        ++it1;
        return index;
    }
    if (end1 || it1->index > it2->index) { // take from it2
        ssize_t index = ssize_t(it2->index);
        value1 = 0;
        value2 = it2->value;
        ++it2;
        return index;
    }

    // take from both iterators
    assert(it1->index == it2->index);
    ssize_t index = ssize_t(it1->index);
    value1 = it1->value;
    value2 = it2->value;
    ++it1;
    ++it2;
    return index;
}

//---------------------------------------------------------------------------------------

sparse_vector_t linear_comb(int mult1, const sparse_vector_t& vec1, 
                            int mult2, const sparse_vector_t& vec2) 
{
    int val1, val2;
    ssize_t j;
    size_t reserved = 0;

    // Phase 1: compute allocation space and verify possible overflows
    auto it1 = vec1.begin(), it2 = vec2.begin();
    while(-1 != (j = traverse_both(it1, vec1, val1, it2, vec2, val2))) {
        long long value = (long long)(val1) * mult1 + (long long)(val2) * mult2;
        if (value > std::numeric_limits<int>::max() || 
            value < std::numeric_limits<int>::min()) 
            throw rgmedd_exception("Integer overflow when combining flows.");
        if (value != 0)
            reserved++;
    }

    // Phase 2: create the vector
    sparse_vector_t res(vec1.size());
    res.reserve(reserved);
    it1 = vec1.begin(), it2 = vec2.begin();
    while(-1 != (j = traverse_both(it1, vec1, val1, it2, vec2, val2))) {
        int value = val1 * mult1 + val2 * mult2;
        if (value != 0)
            res.insert_element(j, value);
    }

    return res;
}

// //---------------------------------------------------------------------------------------

// size_t count_neg(sparse_vector_t& vec) {
//     size_t num_neg = 0;
//     for (size_t i=0; i<vec.nonzeros(); i++)
//         num_neg += (vec.ith_nonzero(i).value < 0) ? 1 : 0;
//     return num_neg;
// }

//---------------------------------------------------------------------------------------

void canonicalize_sign(sparse_vector_t& vec) {
    // size_t num_neg = 0;
    // for (size_t i=0; i<vec.nonzeros(); i++)
    //     num_neg += (vec.ith_nonzero(i).value < 0) ? 1 : 0;

    // // Reduce the number of negative entries
    // if (num_neg > vec.nonzeros() / 2 ||
    //     (num_neg == vec.nonzeros() / 2 && vec.front_nonzero().value < 0)) 
    // {
    //     for (size_t i=0; i<vec.nonzeros(); i++)
    //         vec.set_nnz_value(i, -vec.ith_nonzero(i).value);
    // }
    // Always make the first entry positive
    if (vec.nonzeros() > 0 && vec.front_nonzero().value < 0) {
        for (size_t i=0; i<vec.nonzeros(); i++)
            vec.set_nnz_value(i, -vec.ith_nonzero(i).value);
    }
}

//---------------------------------------------------------------------------------------

void canonicalize(sparse_vector_t& vec) {
    // Always make the first entry positive
    canonicalize_sign(vec);
    // Get the g.c.d. of the vector entries
    int g = -1;
    for (size_t i=0; i<vec.nonzeros(); i++) {
        if (vec.ith_nonzero(i).value != 0) {
            if (g == -1) // first entry
                g = abs(vec.ith_nonzero(i).value);
            else
                g = gcd(g, abs(vec.ith_nonzero(i).value));
            if (g == 1)
                return;
        }
    }
    if (g > 1) {
        for (size_t i=0; i<vec.nonzeros(); i++)
            vec.set_nnz_value(i, vec.ith_nonzero(i).value / g);
    }
}

//---------------------------------------------------------------------------------------

// Annul column j of row B[i] by summing to it row B[k] with an appropriate multiplier
inline void annul_column_of_B_row(flow_basis_t& B, int i, int k, int j) {
    assert(B[i][j] != 0);
    // Find multipliers
    int mult_k = abs(B[i][j]);
    int mult_i = abs(B[k][j]);
    int gcd_ik = gcd(mult_k, mult_i);
    mult_k /= gcd_ik;
    mult_i /= gcd_ik;
    if (sign(B[i][j]) == sign(B[k][j]))
        mult_k *= -1;
    // Sum and make canonical
    B[i] = linear_comb(mult_k, B[k], mult_i, B[i]);
    canonicalize(B[i]);
    assert(B[i][j] == 0);
}

//---------------------------------------------------------------------------------------

// Get the basis B in row footprint form
void row_footprint_form(flow_basis_t& B) {
    for (int k=0; k<B.size(); k++) {
        // Find the k-th pivot row
        int i_max = k;
        for (int i=k; i<B.size(); i++)
            if (B[i].nonzeros() > 0 && B[i].leading() < B[i_max].leading())
                i_max = i;
        // Move the pivot in position k
        std::swap(B[i_max], B[k]);
        // Annull column j0 to all the rows below the pivot (row k)
        const int j0 = B[k].leading();
        for (int i=k+1; i<B.size(); i++) { // Get into a row-echelon form
            if (B[i][j0] != 0)
                annul_column_of_B_row(B, i, k, j0);
        }
    }
    // Step 2: Find row-trailing entries and annul all entries above each of them. 
    // This step is different from the one of the reduced row footprint form.
    // We need to check the trailings by columns, and subtract them from the highest
    // trailing to the lowest trailing, not in row order.
    size_t last_trailing = B[0].size();
    for (int hh=B.size()-1; hh>=0; hh--) {
        // From the bottom, find the row k with the higher trailing less than last_trailing
        int k = -1;
        for (int i=B.size()-1; i>=0; i--)
            if (B[i].trailing() < last_trailing)
                if (k == -1 || B[i].trailing() > B[k].trailing())
                    k = i;
        last_trailing = B[k].trailing();
        // Subtract row k to all the rows above with the same trailing
        const int jN = B[k].trailing();
        // cout << "k="<<k<<" jN="<<jN<<endl;
        for (int i=k-1; i>=0; i--) { 
            if (jN == B[i].trailing())
                annul_column_of_B_row(B, i, k, jN);
        }
    }
}

//---------------------------------------------------------------------------------------

// Get the basis B into reduced row footprint form
void reduced_row_footprint_form(flow_basis_t& B) {
    for (int k=0; k<B.size(); k++) {
        // Find the k-th pivot row
        int i_max = k;
        for (int i=k; i<B.size(); i++)
            if (B[i_max].nonzeros() == 0 || (B[i].nonzeros() > 0 && B[i].leading() < B[i_max].leading()))
                i_max = i;
        // Move the pivot in position k
        std::swap(B[i_max], B[k]);
        // Annull column j0 to all the rows below the pivot (row k)
        if (B[k].nonzeros() > 0) {
            const int j0 = B[k].leading();
            for (int i=k+1; i<B.size(); i++) { // Get into a row-echelon form
                if (B[i][j0] != 0)
                    annul_column_of_B_row(B, i, k, j0);
            }
        }
    }
    // Step 2: Find row-trailing entries and annul all entries above each of them. 
    for (int k=B.size()-1; k>=0; k--) {
        // Annul the last column of B[k] to all the rows above the pivot row k
        if (B[k].nonzeros() > 0) {
            const int jN = B[k].trailing();
            // const int j0 = B[k].leading();
            for (int i=k-1; i>=0; i--) { // Get into our modified row-echelon form
                if (B[i][jN] != 0) /*&& jN == B[i].trailing()*/
                    annul_column_of_B_row(B, i, k, jN);
            }
        }
    }
}

//---------------------------------------------------------------------------------------

// Get the basis B in row footprint form, operating exclusively
// on the rows in the inclusive range [start_row, end_row].
void reduced_row_footprint_form_range(flow_basis_t& B, ssize_t start_row, ssize_t end_row) {
    for (int k=start_row; k<=end_row; k++) {
        // Find the k-th pivot row
        int i_max = k;
        for (int i=k; i<B.size(); i++)
            if (B[i].nonzeros() > 0 && B[i].leading() < B[i_max].leading())
                i_max = i;
        // Move the pivot in position k
        std::swap(B[i_max], B[k]);
        // Annull column j0 to all the rows below the pivot (row k)
        const int j0 = B[k].leading();
        for (int i=k+1; i<B.size(); i++) { // Get into a row-echelon form
            if (B[i][j0] != 0)
                annul_column_of_B_row(B, i, k, j0);
        }
    }
    // Step 2: Find row-trailing entries and annul all entries above each of them. 
    for (int k=end_row; k>=start_row; k--) {
        // Annul the last column of B[k] to all the rows above the pivot row k
        const int jN = B[k].trailing();
        const int j0 = B[k].leading();
        for (int i=k-1; i>=0; i--) { 
            if (B[i][jN] != 0/* && jN == B[i].trailing()*/)
                annul_column_of_B_row(B, i, k, jN);
        }
    }
}

//---------------------------------------------------------------------------------------

void print_flow_basis(const flow_basis_t& B) {
    for (auto&& row : B) {
        for (size_t i=0; i<row.size(); i++) {
            if (row.leading() <= i && i <= row.trailing())
                cout << setw(3) << row[i];
            else
                cout << "  .";
        }
        cout << endl;
    }
    cout << endl;
}

//---------------------------------------------------------------------------------------

void reorder_basis(flow_basis_t& B, const std::vector<int>& net_to_level) {
    for (sparse_vector_t& s : B) {
        for (auto& el : s.data())
            el.index = net_to_level[el.index];
        std::sort(s.data().begin(), s.data().end());
        s.verify_invariants();
        canonicalize_sign(s);
    }
}

//---------------------------------------------------------------------------------------

// compute the support and the number of non-zeros of row i if we subtract from
// it row k s.t. entry B[i][j] = 0.

// inline bool
// better_support_after_col_annul(flow_basis_t& B, int i, int k, int j) {
//     assert(B[i][j] != 0);
//     size_t num_neg = 0;
//     for (size_t h=0; h<B[i].nonzeros(); h++)
//         num_neg += (B[i].ith_nonzero(h).value < 0) ? 1 : 0;
//     // Find multipliers
//     int mult_k = abs(B[i][j]);
//     int mult_i = abs(B[k][j]);
//     int gcd_ik = gcd(mult_k, mult_i);
//     mult_k /= gcd_ik;
//     mult_i /= gcd_ik;
//     if (sign(B[i][j]) == sign(B[k][j]))
//         mult_k *= -1;
//     // Compute the support after the sum
//     size_t new_nnz = 0, new_neg = 0;
//     int val_i, val_k;
//     ssize_t col;
//     size_t reserved = 0;

//     // Phase 1: compute allocation space and verify possible overflows
//     auto it_i = B[i].begin(), it_k = B[k].begin();
//     while(-1 != (col = traverse_both(it_i, B[i], val_i, it_k, B[k], val_k))) {
//         ssize_t value = ssize_t(val_i) * mult_i + ssize_t(val_k) * mult_k;
//         if (value > INT_MAX || value < INT_MIN) 
//             throw rgmedd_exception("Integer overflow in support_after_col_annul().");
//         if (value != 0)
//             new_nnz++;
//         if (value < 0)
//             new_neg++;
//     }
    
//     if (new_nnz < B[i].nonzeros())
//         return true; //  will reduce the support

//     // if (num_neg > B[i].nonzeros() / 2)
//     //     num_neg = B[i].nonzeros() - num_neg;
//     // if (new_neg > B[i].nonzeros() / 2)
//     //     new_neg = B[i].nonzeros() - new_neg;

//     // if (new_neg < num_neg)
//     //     return true; // same support, but will reduce the negative entries

//     return false; // no benefit
// }


//---------------------------------------------------------------------------------------
// Support structure for the computation of P-flow based metrics
//---------------------------------------------------------------------------------------

struct flow_basis_metric_t {
    // Place bound of the model (if available)
    const int* available_place_bounds;
    // Alternative bound value if bounds are not available
    int alternate_bound = -1;
    // The sorted and reduced flow basis
    flow_basis_t B;
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
    std::vector<int> inv_coeff;

    // Should we just compute ranks and not ranges?
    bool only_ranks = false;
    // Should we use ILP to tighten the ranges
    bool use_ilp = false;
    // Should we use explicit enumeration to tighten the ranges
    bool use_enum = false;
    bool print_enums = false;

    flow_basis_metric_t() {}
    ~flow_basis_metric_t();
    void initialize();

    // Compute the PSI score
    cardinality_t measure_PSI(const std::vector<int> &net_to_level);

    // Print the diagram with the flow matrix and the invariant ranges
    void print_PSI_diagram(const std::vector<int> &net_to_level);

    // Simulated annealing with row compact strategy
    void annealing_compact(std::vector<int> &net_to_level);

    // New strategy close to the annealing which finds the minimum in each row
    void compact_basis_row_min(std::vector<int> &net_to_level);

protected:

#ifdef HAS_LP_SOLVE_LIB
    // The ILP structure
    lprec *lp = nullptr;
    // Extra pre-allocated data structure
    std::vector<REAL> ilp_row;
    std::vector<int> ilp_col;

    void delete_LP();
    void initialize_LP(const std::vector<int> &net_to_level);
    void solve_LP();
#endif // HAS_LP_SOLVE_LIB

    // Compte the rank-only score
    size_t compute_rank_score() const;
    // Change rows of B (preserving the footprint form) if the range space of a row reduces
    void optimize_B();
    // Compute the allowed ranges for a given p-flow r of B[]
    void compute_ranges_of_pflow(size_t r);
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
        assert(B[inv].leading() <= lvl && lvl <= B[inv].trailing());
        // The array ranges[inv] does not have a non-zero for each level, but only
        // one non-zero for each non-zero in B[inv]. Get the closest level.
        int index = B[inv].lower_bound_nnz(lvl);
        return ranges[inv][index];
    };

};

//---------------------------------------------------------------------------------------

std::shared_ptr<flow_basis_metric_t> make_flow_basis_metric() {
    std::shared_ptr<flow_basis_metric_t> s = make_shared<flow_basis_metric_t>();
    s->initialize();
    return s;
}

//---------------------------------------------------------------------------------------

cardinality_t measure_PSI(const std::vector<int> &net_to_level, bool only_ranks, bool use_ilp, 
                          bool use_enum, bool print_enums, flow_basis_metric_t& fbm) 
{
    fbm.only_ranks   = only_ranks;
    fbm.use_ilp     = use_ilp;
    fbm.use_enum    = use_enum;
    fbm.print_enums = print_enums;
    return fbm.measure_PSI(net_to_level);
}

void print_PSI_diagram(const std::vector<int> &net_to_level, flow_basis_metric_t& fbm) {
    fbm.print_PSI_diagram(net_to_level);
}

void annealing_compact(std::vector<int> &net_to_level, flow_basis_metric_t& fbm) {
    fbm.annealing_compact(net_to_level);
}

void compact_basis_row_min(std::vector<int> &net_to_level, flow_basis_metric_t& fbm) {
    fbm.compact_basis_row_min(net_to_level);
}

const flow_basis_t& get_basis(const flow_basis_metric_t& fbm) {
    return fbm.B;
}

//---------------------------------------------------------------------------------------

static cardinality_t s_card_1 = 1;
const cardinality_t& 
estimate_nodes_at_level(const flow_basis_metric_t& fbm, int lvl) {
    assert(!fbm.combinations.empty());

    if (lvl == fbm.level_to_net.size() - 1)
        return s_card_1; // top level, return 1 node

    return fbm.combinations[lvl + 1];
}

//---------------------------------------------------------------------------------------

void get_lvl_weights_invariants(const flow_basis_metric_t& fbm, std::vector<size_t>& lvl_weights) {
    lvl_weights.resize(npl);
    std::fill(lvl_weights.begin(), lvl_weights.end(), 0);
    for (auto&& row : fbm.B) {
        // Include the leading, exclude the trailing (we don't have to remember anything
        // when the p-flow is concluded). Update the lvl_weights at lvl+1, because the effect
        // of the p-flow is observed at the next level.
        if (row.nonzeros() > 0)
            for (size_t lvl=row.leading(); lvl < row.trailing(); lvl++)
                lvl_weights[lvl]++;
       // for (size_t lvl=row.leading()+1; lvl < row.trailing(); lvl++)
       //      lvl_weights[lvl + 1]++;
    }
    // For sanity, we put at least 1 to all rank values, 
    // otherwise they could be zero for places not covered by any invariant.
    for (size_t lvl=0; lvl<npl; lvl++)
        lvl_weights[lvl] = std::max(size_t(1), lvl_weights[lvl]);
}

//---------------------------------------------------------------------------------------

void get_lvl_weights_invariants_bounds(const flow_basis_metric_t& fbm, 
                                       const std::vector<int> &net_to_level,
                                       std::vector<size_t>& lvl_weights)
{
    // Compute the active invariants per level
    get_lvl_weights_invariants(fbm, lvl_weights);

    // Weight the invariants with the place bounds
    for (size_t plc=0; plc<npl; plc++) {
        int bound = fbm.get_bound(plc);
        int lvl = net_to_level[plc];
        if (bound >= 1) {
            // cout << tabp[plc].place_name << " lvl="<<net_to_level[plc]<<" bound="<<bound
            //      <<" invariants="<<lvl_weights[net_to_level[plc]]<<endl;
            // lvl++;
            // if (0 < lvl && lvl < lvl_weights.size()) {
            assert(0 <= lvl && lvl < lvl_weights.size());
            lvl_weights[lvl] *= (bound + 1); // include the zero!
            // }
        }
    }
}

//---------------------------------------------------------------------------------------

void get_lvl_weights_ranges(const flow_basis_metric_t& fbm, std::vector<cardinality_t>& lvl_weights) {
    lvl_weights.resize(npl);
    std::fill(lvl_weights.begin(), lvl_weights.end(), cardinality_t(0));

    // Compute the product of the invariant ranges at each level
    for (int lvl = npl-1; lvl >=0; lvl--) {
        const int plc = fbm.level_to_net[lvl];
        lvl_weights[lvl] = 1;

        if (lvl == npl-1) // top level
            continue;
        // Consider all the invariants that are active at this level
        for (int r=0; r<fbm.B.size(); r++) {
            if (fbm.B[r].nonzeros() == 0)
                continue;
            if (fbm.B[r].leading() <= lvl && lvl <= fbm.B[r].trailing()) {
                int inv_combs;
                if (lvl == fbm.B[r].trailing())
                    inv_combs = 1;
                else
                    inv_combs = fbm.invariant_range_at_lvl(r, lvl + 1).size();
                lvl_weights[lvl] *= inv_combs;
            }
        }
    }

    // For sanity, we put at least 1 to all rank values, 
    // otherwise they could be zero for places not covered by any invariant.
    for (size_t lvl=0; lvl<npl; lvl++)
        lvl_weights[lvl] = std::max(cardinality_t(1), lvl_weights[lvl]);
}

//---------------------------------------------------------------------------------------

flow_basis_metric_t::~flow_basis_metric_t() {
#ifdef HAS_LP_SOLVE_LIB
    delete_LP();
#endif
}

//---------------------------------------------------------------------------------------

void flow_basis_metric_t::initialize() {
    load_flow_basis();
    // if (!load_flow_basis()) {
    //     cerr << "\nMISSING .pba FILE! CANNOT LOAD P-FLOW BASIS!\n" << endl;
    //     throw rgmedd_exception("Missing pba.");
    //     // return cardinality_t(0);
    // }

    available_place_bounds = load_bounds();
    // if (available_place_bounds == nullptr) {
    //     cerr << "\nMISSING BOUNDS! PSI score computation will be less reliable!" << endl;
    // }

    // Compute an alternative bound from the initial marking m0
    // when the bound is not available for some or all places.
    alternate_bound = net_mark[0].total;
    for (int pl=0; pl<npl; pl++)
        alternate_bound = max(alternate_bound, net_mark[pl].total);
}

//---------------------------------------------------------------------------------------

// Change rows of B (preserving the footprint form) if the range space of a row 
// may be reduced by adding another row with a smaller footprint.
void flow_basis_metric_t::optimize_B() {
    for (int k=B.size()-1; k>=0; k--) {
        if (B[k].nonzeros() == 0)
            continue;
        for (int i=k-1; i>=0; i--) { 
            if (B[i].nonzeros() == 0)
                continue;
            if (B[i].leading() < B[k].leading() && B[k].trailing() < B[i].trailing()) {
                // Find an annullable column for the sum of row k to i
                ssize_t j;
                if (B[i][ B[k].leading() ] != 0)
                    j = B[k].leading();
                else if (B[i][ B[k].trailing() ] != 0)
                    j = B[k].trailing();
                else { // search in the remaining columns of k
                    j = -1;
                    for (size_t h = 1; h < B[k].nonzeros() - 1; h++) {
                        if (B[i][ B[k].ith_nonzero(h).index ] != 0) {
                            j = B[k].ith_nonzero(h).index;
                            break; // found a pivot
                        }
                    }
                    if (j == -1)
                        continue; // no pivot for sum
                }

                // We can sum B[k] to B[i] to annul column j of B[i]. 
                // Check if the range space becomes smaller.
                cardinality_t S0 = estimate_range_space(i);
                sparse_vector_t Bi = B[i];
                std::vector<int_range_t> ranges_i = ranges[i];
                // for (int h=0; h<Bi.size(); h++)
                //     cout << setw(3) << Bi[h];
                // cout << "  space = " << S0 << endl;

                // Try subtracting B[k] to B[i]
                annul_column_of_B_row(B, i, k, j);
                compute_ranges_of_pflow(i);
                cardinality_t S1 = estimate_range_space(i);
                // for (int h=0; h<B[i].size(); h++)
                //     cout << setw(3) << B[i][h];
                // cout << "  space = " << S1 << endl;

                if (S1 > S0 || (S1 == S0 && B[i].nonzeros() > Bi.nonzeros())) {
                    B[i] = Bi; // keep the previous vector
                    ranges[i] = ranges_i;
                }
                // else if (S1 == S0 && B[i].nonzeros() == Bi.nonzeros() &&
                //          count_neg(B[i]) > count_neg(Bi))
                //     B[i] = Bi; // keep the previous vector
                // else {
                //     cout << "IMPROVED!!" << endl;
                // }
                // cout << endl;
            }
        }
    }
}

//---------------------------------------------------------------------------------------

void flow_basis_metric_t::print_ILP_Mathematica() {
    // std::vector<PSemiflow> psf;
    // load_Psemiflows(psf);
    // for (const PSemiflow& p : psf) {
    //     int sum = 0, cnt = 0;
    //     for (auto&& el : p) {
    //         sum += net_mark[ el.place_no ].total * el.card;
    //         cout << (cnt++==0 ? "" : " + ") << "x" << net_to_level[el.place_no];
    //     }
    //     cout << " == " << sum << ",\n";
    // }
    for (auto&& pflow : B) {
        int sum = 0, cnt = 0;
        for (auto&& el : pflow) {
            const int plc = level_to_net[el.index];
            sum += net_mark[plc].total * el.value;
            cout << std::showpos << el.value << std::noshowpos << "*x" << el.index;
        }
        cout << " == " << sum << ",\n";
    }
    for (int plc = 0; plc < level_to_net.size(); plc++) {
        cout << "x"<<plc<<" >= 0, ";
    }
    cout << endl;
    for (int plc = 0; plc < level_to_net.size(); plc++) {
        cout << "x"<<plc<<" <= "<<get_bound(plc)<<", ";
    }
    cout << endl;

    for (auto&& pflow : B) {
        cout << "{";
        for (int k = pflow.nonzeros() - 1; k >= 0; k--) {
            cout << "MinMax[";
            for (int i=pflow.nonzeros() - 1; i>=k; i--) {
                cout << std::showpos << pflow.ith_nonzero(i).value << std::noshowpos 
                     << "*x"<< pflow.ith_nonzero(i).index;
            }
            cout << "]" << (k==0 ? "" : ",\n ");
        }
        cout << "}\n";
    }
    cout << endl;
}

//---------------------------------------------------------------------------------------

// Compute the allowed ranges for a given p-flow r of B[]
void flow_basis_metric_t::compute_ranges_of_pflow(size_t r) {
    const sparse_vector_t& pflow = B[r];
    ranges[r].resize(pflow.nonzeros());
    // Get the constant of the product of the invariant r and M0
    // Since the set of invariants changes at every  re-ordering, 
    // these values must be recomputed at every change of variable order.
    inv_coeff[r] = 0;
    for (auto&& elem : pflow)
        inv_coeff[r] += elem.value * net_mark[ level_to_net[elem.index] ].total;

    // Compute the allowed ranges in forward, starting from the final value
    int_range_t R1 = int_range_t(inv_coeff[r], inv_coeff[r]);
    for (size_t k=0; k<pflow.nonzeros(); k++) {
        const int plc = level_to_net[pflow.ith_nonzero(k).index];
        ranges[r][k] = R1;
        R1 -= make_range(0, get_bound(plc) * pflow.ith_nonzero(k).value);
    }

    // Compute the allowed ranges backward, starting from 0
    int_range_t R2(0, 0);
    for (ssize_t k=pflow.nonzeros() - 1; k >= 0; k--) {
        const int plc = level_to_net[pflow.ith_nonzero(k).index];
        R2 += make_range(0, get_bound(plc) * pflow.ith_nonzero(k).value);
        // The actual range is the intersection of the two computed range series
        ranges[r][k] = intersect(ranges[r][k], R2);
    }
}

//---------------------------------------------------------------------------------------

// Compute how many combinations we have for p-flow r on all levels
// where the p-flow is active
cardinality_t flow_basis_metric_t::estimate_range_space(size_t r) {
    cardinality_t S = 0;
    const sparse_vector_t& pflow = B[r];

    for (int k=0; k<pflow.nonzeros(); k++) {
        int size = ranges[r][k].size();
        // For how many levels this range will be active?
        int n_levels;
        if (k < pflow.nonzeros() - 1)
            n_levels = pflow.ith_nonzero(k + 1).index - pflow.ith_nonzero(k).index;
        else
            n_levels = 1; // last level

        S += size * n_levels;
    }
    return S;
}

//---------------------------------------------------------------------------------------

cardinality_t flow_basis_metric_t::compute_score() 
{
    combinations.resize(npl);
    cardinality_t PSI = 0;

    // Compute the score at each level of the DD
    for (int lvl = level_to_net.size()-1; lvl >=0; lvl--) {
        const int plc = level_to_net[lvl];
        combinations[lvl] = 1;
        if (lvl == level_to_net.size() - 1)
            continue; // top level has always a single node

        // Consider all the invariants active at this level
        for (int r=0; r<B.size(); r++) {
            if (B[r].nonzeros() == 0)
                continue;
            if (B[r].leading() <= lvl && lvl <= B[r].trailing()) {
                // Compute the maximum sum combinations for invariant r 
                // we need to remember at this level. This assumes no interactions
                // between the various invariants of the model (synchronizations).

                // int weight = abs(B[r][lvl]), inv_combs;
                // if (weight <= 1) // could be 0 or 1 (0 = not active in this level)
                //     inv_combs = invariant_range_at_lvl(r, lvl).size();
                // else
                //     inv_combs = (invariant_range_at_lvl(r, lvl) / weight).size();

                int inv_combs = invariant_range_at_lvl(r, lvl).size();

                combinations[lvl] *= inv_combs;
            }
        }
        PSI += combinations[lvl];
    }

    return PSI;
}

//---------------------------------------------------------------------------------------
// ILP solution of the invariant ranges
//---------------------------------------------------------------------------------------

#ifdef HAS_LP_SOLVE_LIB

void flow_basis_metric_t::delete_LP() {
    if (lp) {
        delete_lp(lp);
        lp = nullptr;
    }    
}

//---------------------------------------------------------------------------------------

void flow_basis_metric_t::initialize_LP(const std::vector<int> &net_to_level) {
    assert(lp == nullptr);
    assert(!level_to_net.empty());
    const size_t N = level_to_net.size();
    // Initialize lp_solve. 
    // The ILP will have one column per DD level (i.e. one per place in DD order).
    lp = make_lp(0, N);
    set_add_rowmode(lp, true);

    // Initialize the integer variables
    for (size_t lvl=0; lvl<N; lvl++) {
        const int plc = level_to_net[lvl];
        set_int(lp, lvl+1, true);
        set_col_name(lp, lvl+1, tabp[plc].place_name);
        // Set the upper bound (if available)
        if (have_bound(plc))
            set_bounds(lp, lvl+1, 0.0, get_bound(plc));
    }

    std::vector<REAL>& row = ilp_row;
    std::vector<int>& col = ilp_col;
    row.resize(N);
    col.resize(N);
    std::fill(row.begin(), row.end(), 0.0);
    std::fill(col.begin(), col.end(), 0);

    // Add each p-flow as a constraint
    for (int r=0; r<B.size(); r++) {
        int j = 0;   // number of elements in the new row
        inv_coeff[r] = 0; // product of this p-flow with m0
        for (auto&& elem : B[r]) {
            row[j] = elem.value;
            col[j] = elem.index + 1;
            inv_coeff[r] += elem.value * net_mark[ level_to_net[elem.index] ].total;
            j++;
        } 
        add_constraintex(lp, j, row.data(), col.data(), ROWTYPE_EQ, inv_coeff[r]);
    }

    // Conclude LP initialization and prepare for the engine to run
    set_add_rowmode(lp, false);
    // write_LP(lp, stdout);
}

//---------------------------------------------------------------------------------------

const long MAX_TIME_FOR_ILP = 1; // in seconds
const long MAX_TIME_FOR_ALL_ILPs = 5; // in seconds

void flow_basis_metric_t::solve_LP() {
    std::vector<REAL>& row = ilp_row;
    std::vector<int>& col = ilp_col;
    int status;
    set_verbose(lp, IMPORTANT);
    clock_t start = clock();
    set_timeout(lp, MAX_TIME_FOR_ILP);

    // Solve the ILP problems
    for (size_t phase = 0; phase < 2; phase++) {
        if (phase == 0) 
            set_maxim(lp); // phase 0 solves maximization problems
        else
            set_minim(lp); // phase 1 solves minimization problems

        for (int r=0; r<B.size(); r++) {
            // The target function is the p-flow cut to the considered level
            // Prepare the p-flow in lp-solve format
            int j = 0;   // Number of elements in the objective function row
            for (ssize_t k = B[r].nonzeros() - 1; k >= 0; k--) {
                row[j] = B[r].ith_nonzero(k).value;
                col[j] = B[r].ith_nonzero(k).index + 1;
                j++;
            }

            // Solve two ILPs for each level in which the p-flow is active
            for (size_t k=0; k<B[r].nonzeros(); k++) {
                // Consider the first (k+1) entries of the p-flow
                set_obj_fnex(lp, B[r].nonzeros() - k, row.data(), col.data());

                status = solve(lp);
                if (status == OPTIMAL) {
                    if (phase == 0) // maximization
                        ranges[r][k].r = min(ranges[r][k].r, int(get_objective(lp)));
                    else // minimization
                        ranges[r][k].l = max(ranges[r][k].l, int(get_objective(lp)));

                    // Sanitize if the estimated range is worse than the ILP range
                    if (ranges[r][k].l > ranges[r][k].r)
                        ranges[r][k].l = ranges[r][k].r;
                }

                long elapsed = long((clock() - start) / CLOCKS_PER_SEC);
                if (elapsed > MAX_TIME_FOR_ALL_ILPs) {
                    cout << "TIMEOUT ILP" << endl;
                    return;
                }
            }
        }
    }
}

#endif // HAS_LP_SOLVE_LIB

//---------------------------------------------------------------------------------------

// set of invariants partial sum
struct uPSI_set {
    inline uPSI_set() {}
    inline uPSI_set(const uPSI_set&) = default;
    inline uPSI_set(uPSI_set&&) = default;
    inline uPSI_set& operator=(const uPSI_set&) = default;
    inline uPSI_set& operator=(uPSI_set&&) = default;

    std::vector<size_t> active_invs;

    // How many invariant sums are stored in each upsi?
    inline size_t psi_len() const { return active_invs.size(); }
    inline size_t size() const { return upsi_vec.size() / psi_len(); }

    void push_back(int* p) { upsi_vec.insert(upsi_vec.end(), p, p + psi_len()); }

    struct uPSI_ref {
        inline uPSI_ref() {}
        inline uPSI_ref(std::vector<int>::iterator _it, size_t _l) 
        : it(_it), len(_l) { }

        // Assign by value
        inline uPSI_ref& operator=(const uPSI_ref ref) {
            assert(ref.len == len);
            std::copy(ref.it, ref.it + ref.len, it);
            return *this;
        }
        inline bool operator==(const uPSI_ref ref) const {
            for (size_t i=0; i<len; i++)
                if (*(it + i) != *(ref.it + i))
                    return false;
            return true;
        }
        inline bool operator<(const uPSI_ref ref) const {
            for (size_t i=0; i<len; i++) {
                int v1 = *(it + i), v2 = *(ref.it + i);
                if (v1 < v2) 
                    return true;
                if (v1 > v2)
                    return false;
            }
            return false; // equal
        }
        inline bool operator<=(const uPSI_ref ref) const {
            for (size_t i=0; i<len; i++) {
                int v1 = *(it + i), v2 = *(ref.it + i);
                if (v1 < v2) 
                    return true;
                if (v1 > v2)
                    return false;
            }
            return true; // equal
        }
        inline void swap(const uPSI_ref& ref) {
            assert(ref.len == len);
            for (size_t i=0; i<len; i++)
                std::swap(*(it + i), *(ref.it + i));
        }

        // inline int get(size_t i) const { assert(i < len); return *(it + i); }
    private:
        std::vector<int>::iterator it;
        size_t len;
    };

    // An iterator to view the list of upsi
    struct iterator {
        typedef uPSI_ref value_type;
        typedef std::vector<int>::iterator::difference_type difference_type;
        typedef void* pointer;   // should not be used.
        typedef uPSI_ref& reference;
        typedef std::random_access_iterator_tag iterator_category;

        inline iterator() {}
        inline iterator(std::vector<int>::iterator _it, size_t _l) 
        : it(_it), len(_l) { }
        inline iterator(const iterator&) = default;
        inline iterator(iterator&&) = default;
        inline iterator& operator=(const iterator&) = default;
        inline iterator& operator=(iterator&&) = default;

        inline iterator& operator++ () { it+=len; return *this; }
        inline iterator operator++ (int) { iterator i(*this); ++(*this); return i; }
        inline iterator& operator-- () { it-=len; return *this; }
        inline iterator operator-- (int) { iterator i(*this); --(*this); return i; }

        inline difference_type operator- (const iterator& i) 
        { assert((it - i.it) == len * ((it - i.it)/len)); return (it - i.it) / len; }

        inline void operator+=(int k) { it+=len * k; }
        inline void operator-=(int k) { it-=len * k; }

        inline iterator operator+(int k) { iterator i(*this); i.it+=k*len; return i; }
        inline iterator operator-(int k) { iterator i(*this); i.it-=k*len; return i; }

        inline bool operator == (const iterator &i) const { return it == i.it; }
        inline bool operator != (const iterator &i) const { return it != i.it; }
        inline bool operator >= (const iterator &i) const { return it >= i.it; }
        inline bool operator <= (const iterator &i) const { return it <= i.it; }
        inline bool operator >  (const iterator &i) const { return it >  i.it; }
        inline bool operator <  (const iterator &i) const { return it <  i.it; }

        // inline const int** operator -> () const { return it.operator->(); }
        inline uPSI_ref operator * () { return uPSI_ref(it, len); }
        inline int get(size_t i) const { assert(i < len); return *(it + i); }

    private:
        std::vector<int>::iterator it;
        size_t len; // IPS tuple length
        friend class uPSI_set;
    };

    inline iterator begin() { return iterator(upsi_vec.begin(), psi_len()); }
    inline iterator end()   { return iterator(upsi_vec.end(), psi_len()); }
    void truncate(iterator i) { upsi_vec.erase(i.it, upsi_vec.end()); }

protected:
    std::vector<int> upsi_vec;
};

inline void swap(uPSI_set::uPSI_ref i1, uPSI_set::uPSI_ref i2) { i1.swap(i2); }

//---------------------------------------------------------------------------------------

const size_t MAX_ENUM_SECONDS = 10;
const size_t MAX_ENTRIES_PER_ROW = 1000000;

// Enumerate the uPSI at each level, to get the exact # of them
bool flow_basis_metric_t::bound_by_enumeration() {
    // All bounds must be available to do enumeration
    for (int plc=0; plc < level_to_net.size(); plc++)
        if (!have_bound(plc))
            return false;

    combinations.resize(npl);
    clock_t start_of_enum = clock();
    count_uPSIs.resize(level_to_net.size());
    const int N_INV = B.size();
    uPSI_set set, prev_set;
    std::vector<int> upsi(N_INV), new_upsi(N_INV), weights(N_INV), temp(N_INV);
    std::vector<int_range_t> active_ranges(N_INV);

    // Initialize: insert the initial PSI (sum 0 of the first invariant)
    set.active_invs.push_back(0);
    upsi[0] = 0;
    set.push_back(upsi.data());

    // Generate the set of IPS for each level, looking at the IPS of the previous level
    for (int lvl = level_to_net.size() - 1; lvl >=0; lvl--) {        
        const int plc = level_to_net[lvl];
        const int bound = get_bound(plc);
        std::swap(set, prev_set);

        if ((clock() - start_of_enum) / CLOCKS_PER_SEC > MAX_ENUM_SECONDS) {
            cerr << "TIMEOUT uPSI ENUMERATION!" <<endl;
            return false;
        }

        // Count which invariants are active at this level
        set.truncate(set.begin());
        set.active_invs.resize(0);
        weights.resize(0);
        active_ranges.resize(0);
        for (int r=0; r<B.size(); r++) {
            if (B[r].leading() <= lvl && lvl <= B[r].trailing()) {
                set.active_invs.push_back(r);
                weights.push_back(B[r][lvl]);
                active_ranges.push_back(invariant_range_at_lvl(r, lvl));
            }
        }
        if (set.active_invs.empty()) { // If we have bounds, this should not happen
            cerr << "Cannot have an empty set of active invariants at level." << endl;
            return false;
        }

        // Generate the new tuples from the previous ones.
        std::fill(temp.begin(), temp.end(), 0);
        for (uPSI_set::iterator prev_upsi = prev_set.begin(); 
            prev_upsi != prev_set.end(); ++prev_upsi) 
        {
            // Unpack to temp[]
            int j = 0;
            for (int i : prev_set.active_invs)
                temp[i] = prev_upsi.get(j++);
            // Pack in upsi[] using the set of active invariants of this level.
            j = 0;
            for (int i : set.active_invs)
                upsi[j++] = temp[i];
            // cout << "[";
            // for (int i=0; i<set.active_invs.size(); i++)
            //     cout << (i==0 ? "" : ",") << upsi[i];
            // cout << "] -> ";

            // Generate all the reachable upsi
            for (int m = 0; m <= bound; m++) {
                for (int i=0; i<set.active_invs.size(); i++) {
                    new_upsi[i] = upsi[i] + m * weights[i];
                    if (!active_ranges[i].inside(new_upsi[i]))
                        goto end_bound_loop;
                }
                // Insert the new upsi
                set.push_back(new_upsi.data());

                if (set.size() > MAX_ENTRIES_PER_ROW) {
                    cerr << "Exceeded max uPSI count per level." << endl;
                    return false;
                }

                // cout << "[";
                // for (int i=0; i<set.active_invs.size(); i++)
                //     cout << (i==0 ? "" : ",") << new_upsi[i];
                // cout << "] ";

                end_bound_loop: ;
            }
            // cout << "\n  ";
        }
        // cout << endl;

        // Remove the duplicate psi, to make them unique
        // Must use bufferless sort and not the standard sort, because
        // the uPSI_ref class does not make a copy and works only by swaps().
        bufferless_mergesort(set.begin(), set.size());
        set.truncate(std::unique(set.begin(), set.end()));

        count_uPSIs[lvl] = std::min(get_ulong(combinations[lvl]), set.size());

        // print the upsi
        if (print_enums) {
            cout << "level " << setw(3) << lvl << " bound="<<setw(2)<<bound
                 << "  unique_PSI=" << count_uPSIs[lvl];

            int j = 0;
            char buffer[1024];
            for (uPSI_set::iterator the_upsi = set.begin(); the_upsi != set.end(); ++the_upsi) {
                int num_wr, start_buf = 1;
                buffer[0] = '[';
                for (int i=0; i<set.active_invs.size(); i++) {
                    sprintf(buffer + start_buf, "%s%d%n", 
                            (i==0 ? "" : ","), the_upsi.get(i), &num_wr);
                    start_buf += num_wr;
                }
                cout << ((j++ % 10) == 0 ? "\n   " : "");
                cout << setw(8) << (char*)(buffer) << "] ";
            }
            cout << endl << endl;
        }
    }

    if (print_enums)
        cout << endl;
    return true;
}

//---------------------------------------------------------------------------------------

size_t flow_basis_metric_t::compute_rank_score() const {
    size_t PSI = 0;
    // Sum all the p-flow spans
    for (auto&& row : B) {
        // Note: unlike transition spans, we do not add + 1, since the invariant
        // does not have an impact on the trailing level.
        if (row.nonzeros() > 0)
            PSI += (row.trailing() - row.leading()); 
    }
    return PSI;
}

//---------------------------------------------------------------------------------------

void compute_lvl_weights(const std::vector<int> &net_to_level,
                         flow_basis_metric_t& fbm, 
                         std::vector<size_t>& lvl_weights) 
{
    // Build a copy of the basis where columns are ordered according to @net_to_level
    fbm.B = get_flow_basis();
    reorder_basis(fbm.B, net_to_level);

    // Gaussian elimination: move B in reduced footprint row form
    reduced_row_footprint_form(fbm.B);

    get_lvl_weights_invariants(fbm, lvl_weights);    
}

//---------------------------------------------------------------------------------------

std::vector<std::vector<std::string>>
range_matrix_for_representation(flow_basis_metric_t& fbm) {
    assert(fbm.ranges.size() == fbm.B.size());
    std::vector<std::vector<std::string>> RM(fbm.B.size());

    for (int r=0; r<fbm.B.size(); r++) {
        const sparse_vector_t& pflow = fbm.B[r];
        RM[r].resize(pflow.size());

        for (size_t k=0; k<pflow.nonzeros(); k++) {
            ostringstream str;
            str << fbm.ranges[r][k];
            RM[r][pflow.ith_nonzero(k).index] = str.str();
        }
    }
    return RM;
}

//---------------------------------------------------------------------------------------

size_t
range_prod_for_representation(flow_basis_metric_t& fbm, std::vector<std::string>& RP) 
{
    RP.resize(npl);
    size_t PSI = 1; // top level

    // Compute the score at each level of the DD
    for (int lvl = npl-1; lvl >=0; lvl--) {
        const int plc = fbm.level_to_net[lvl];

        if (lvl == npl-1) // top level
            RP[lvl] = "1 [top]";
        else {
            ostringstream oss;
            size_t cnt = 0;
            size_t prod = 1;
            // Consider all the invariants active at this level
            for (int r=0; r<fbm.B.size(); r++) {
                if (fbm.B[r].nonzeros() == 0)
                    continue;
                if (fbm.B[r].leading() <= lvl && lvl <= fbm.B[r].trailing()) {
                    int inv_combs;
                    if (lvl == fbm.B[r].trailing())
                        inv_combs = 1;
                    else
                        inv_combs = fbm.invariant_range_at_lvl(r, lvl + 1).size();
                    prod *= inv_combs;
                    oss << (cnt++==0 ? "" : "*") << inv_combs;
                }
            }
            RP[lvl] = std::to_string(prod) + "  [" + oss.str() + "]";
            PSI += prod;
        }
    }
    return PSI;
}

//---------------------------------------------------------------------------------------

cardinality_t 
flow_basis_metric_t::measure_PSI(const std::vector<int> &net_to_level)
{
    const flow_basis_t& pbasis = get_flow_basis();
    cardinality_t PSI;

    // Build a copy of the basis where columns are ordered according to @net_to_level
    B = pbasis;
    reorder_basis(B, net_to_level);

    // Gaussian elimination: move B in reduced footprint row form
    reduced_row_footprint_form(B);

    // If we just want rank spans, the footprint row form is enough
    if (only_ranks) {
        ranges.resize(0);
        inv_coeff.resize(0);
        combinations.resize(0);
        return cardinality_t(compute_rank_score());
    }

    // Level -> place mapping
    level_to_net.resize(net_to_level.size());
    for (int i=0; i<net_to_level.size(); i++)
        level_to_net[ net_to_level[i] ] = i;

    // Assign to each non-zero in B its token range
    ranges.resize(B.size());
    inv_coeff.resize(B.size());
    for (int r=0; r<B.size(); r++)
        compute_ranges_of_pflow(r);

    // Optimize B[] to reduce the ranges by looking at the 
    // possible row sums in the footprint row form of B[]
    optimize_B();

#ifdef HAS_LP_SOLVE_LIB
    // Try again to reduce the ranges using ILP
    if (use_ilp) {
        initialize_LP(net_to_level);
        solve_LP(); // Try to improve the ranges using the ILP
    }
#endif // HAS_LP_SOLVE_LIB

    if (use_enum && bound_by_enumeration()) {
        PSI = 0;
        for (const size_t count : count_uPSIs)
            PSI += count;
    }
    else {
        count_uPSIs.resize(0); // will not be printed
        PSI = compute_score();
    }
    // print_ILP_Mathematica();

#ifdef HAS_LP_SOLVE_LIB
    delete_LP();
#endif // HAS_LP_SOLVE_LIB

    return PSI;    
}

//---------------------------------------------------------------------------------------

void flow_basis_metric_t::print_PSI_diagram(const std::vector<int> &net_to_level) {
    size_t max_plc_len = 5;
    for (int p=0; p<npl; p++)
        max_plc_len = max(max_plc_len, strlen(tabp[p].place_name));
    max_plc_len += 2;

    // rebuild Level -> place mapping (could be missing for B-rank only analysis)
    level_to_net.resize(net_to_level.size());
    for (int i=0; i<net_to_level.size(); i++)
        level_to_net[ net_to_level[i] ] = i;

    // Print header
    cout << endl;
    cout << "Lv " << setw(max_plc_len) << left << "Place";
    for (int b=0; b<B.size(); b++)
        cout << "   ";
    cout << " | bnd ";
    if (!ranges.empty()) {
        assert(!combinations.empty());
        for (int b=0; b<B.size(); b++)
            cout << "   inv" << setw(2) << b ;
        cout << " #cmb" << (count_uPSIs.empty() ? "" : " #uPSI");
    }
    cout << endl;

    // Compute the score at each level of the DD
    for (int lvl = level_to_net.size()-1; lvl >=0; lvl--) {
        const int plc = level_to_net[lvl];

        // print p-flow matrix
        cout << setw(3) << left << lvl << setw(max_plc_len) << left << tabp[plc].place_name;
        int lvl_rank = 0; // # of active invariants at this level
        for (int b=0; b<B.size(); b++) {
            if (B[b].leading() <= lvl && lvl <= B[b].trailing()) {
                cout << setw(3) << right << B[b][lvl];
                lvl_rank++;
            }
            else
                cout << right << "  .";
        }
        cout << " |" ;//<< setw(2) << lvl_rank << " ";
        cout << setw(3) << get_bound(plc) << " ";

        if (!ranges.empty()) {
            for (int r=0; r<B.size(); r++) {
                if (B[r].leading() <= lvl && lvl <= B[r].trailing())
                    cout << setw(8) << invariant_range_at_lvl(r, lvl).str();
                else 
                    cout << setw(8) << " - ";

            }

            cout << setw(5) << combinations[lvl];
            if (!count_uPSIs.empty())
                cout << setw(6) << count_uPSIs[lvl];
        }
        cout << endl;
    }

    if (!inv_coeff.empty()) {
        cout << setw(max_plc_len+3) << " ";
        for (int b=0; b<B.size(); b++)
            cout << "---";
        cout << endl << setw(max_plc_len+3) << " ";
        for (int b=0; b<B.size(); b++)
            cout << setw(3) << inv_coeff[b];
    }
    cout << endl << endl;
}

//---------------------------------------------------------------------------------------

size_t sum_of_footprints(const flow_basis_t& B) {
    size_t score = 0;
    for (auto&& row : B)
        score += row.trailing() - row.leading() + 1;
    return score;
}

//---------------------------------------------------------------------------------------

void flow_basis_metric_t::annealing_compact(std::vector<int> &net_to_level) 
{
    assert(net_to_level.size() == npl); // expect to be called with an initial guess
    std::vector<bool> selected(npl);
    std::vector<int> rev_order(npl);
    // std::vector<pair<size_t, size_t>> sharing;
    level_to_net.resize(net_to_level.size());
    const flow_basis_t& pbasis = get_flow_basis();
    init_genrand64(0xA67BD90E);

    optimization_finder::conf conf;
    conf.max_tentatives = g_sim_ann_num_tentatives;

    auto score_fn = [&](const std::vector<int>& net_to_level) -> size_t {
        // Build a copy of the basis where columns are ordered according to @net_to_level
        B = pbasis;
        for (sparse_vector_t& s : B) {
            for (auto& el : s.data())
                el.index = net_to_level[el.index];
            std::sort(s.data().begin(), s.data().end());
            s.verify_invariants();
            canonicalize_sign(s);
        }
        // Gaussian elimination: move B in reduced footprint row form
        row_footprint_form(B);

        // for (size_t k=0; k<B.size(); k++) {
        //     extract_reinsert_row(k, net_to_level);
        //     cout << endl;
        // }
        // exit(-1);


        return compute_rank_score();
    };

    auto generate_fn = [&](const std::vector<int>& net_to_level, 
                           std::vector<int>& new_order) 
    {
        for (int i=0; i<net_to_level.size(); i++)
            level_to_net[ net_to_level[i] ] = i;
        // Select a row at random and compact all its places
        size_t row = genrand64_int63() % B.size();
        std::fill(selected.begin(), selected.end(), false);
        // sharing.resize(0);
        for (auto&& el : B[row]) {
            selected[el.index] = true;
            // Count how many invariants we have above or below
            // pair<size_t, size_t> sh(row, el.index);
            // for (int r=0; r<row; r++) {
            //     if (r == row)
            //         continue;
            //     if (B[r].leading() <= el.index && el.index <= B[r].trailing()) {
            //         sh.first = r;
            //         break;
            //     }
            // }
            // sharing.push_back(sh);
        }
        // std::sort(sharing.begin(), sharing.end());

        // Take all places before the new leading
        size_t new_leading = genrand64_int63() % B.size();
        // if (0 == (genrand64_int63() % 2)) 
        //     new_leading = B[row].leading();
        // else
        //     new_leading = B[row].leading() + B[row].nonzeros();

        size_t j = 0;
        for (size_t p=0; p<new_leading; p++)
            if (!selected[p])
                rev_order[j++] = level_to_net[p];
        // Take all selected places
        // for (auto&& el : sharing)
        //     rev_order[j++] = level_to_net[el.second];
        for (size_t p=0; p<npl; p++)
            if (selected[p])
                rev_order[j++] = level_to_net[p];
        // Take the remaining unselected places
        for (size_t p=new_leading; p<npl; p++)
            if (!selected[p])
                rev_order[j++] = level_to_net[p];

        for (size_t p=0; p<npl; p++)
            new_order[ rev_order[p] ] = p;
    };

    auto msg_fn = [&](optimization_finder::message m, size_t iter, 
                      size_t score, size_t new_score) 
    {
        if (running_for_MCC())
            return; // quiet 
        if (m == optimization_finder::IMPROVEMENT)
            cout << "   iter="<<iter<<"  score="<<new_score<<endl;
        else if (m == optimization_finder::TIME_EXCEEDED)
            cout << "   Time exceeded." << endl;
        else if (m == optimization_finder::REACHED_LOCAL_MINIMUM)
            cout << "   Local minimum found at iter " << iter << "." << endl;
    };

    optimization_finder::optimize(net_to_level, score_fn, generate_fn, msg_fn, conf);
}

//---------------------------------------------------------------------------------------

// This method implements an heuristic that compact the dominant p-flows of a variable 
// order. At each iteration, a p-flow is selected, its places are made consecutive
// and a new leading position is searched using a sliding window, to reduce the score.
// If a good position is found, the variable order is updated and the loop restarts.
void flow_basis_metric_t::compact_basis_row_min(std::vector<int> &net_to_level) 
{
    assert(net_to_level.size() == npl); // expect to be called with an initial guess
    std::vector<bool> selected_cols(npl);
    std::vector<int> lvl_to_lvl_map(npl);
    std::vector<int> tmp_order(npl);

    level_to_net.resize(net_to_level.size());
    for (int i=0; i<net_to_level.size(); i++)
        level_to_net[ net_to_level[i] ] = i;

    const flow_basis_t& pbasis = get_flow_basis();
    // Reorder B (the method extract_reinsert_row() expects it to be ordered).
    B = pbasis;
    reorder_basis(B, net_to_level);
    reduced_row_footprint_form(B);
    size_t best_score = sum_of_footprints(B), new_leading, iter = 0;
    cout << "   starting score="<<best_score<<endl;

    // Iterate until a fixed point is reached
    bool changed = true;
    do {
        changed = false;

        for (size_t r = 0; r<B.size(); r++) {
            if (B[r].nonzeros() == 1) // constant place
                continue; // Any position is ok.
            // Search the leading column that gets the minimum score
            // if the p-flow of row r is made consecutive, starting from such column.
            size_t new_score = extract_reinsert_row(r, net_to_level,
                                                    &new_leading, selected_cols);
            if (new_score < best_score) {
                changed = true;
                cout << "   iter="<<iter<<"  score="<<new_score<<endl;
                best_score = new_score;

                // Update the reverse level -> net order
                for (int i=0; i<net_to_level.size(); i++)
                    level_to_net[ net_to_level[i] ] = i;

                // cout << "new_leading="<<new_leading<<endl;
                // for (int k : net_to_level)
                //     cout << setw(3) << k;
                // cout << endl;

                // for (int k : selected_cols)
                //     cout << setw(3) << k;
                // cout << endl;

                // Build the "old level" -> "new level" transformation
                for (size_t j=0, jj_sel=0, jj_nsel=0; j<npl; j++) {
                    if (!selected_cols[j]) {
                        if (jj_nsel == new_leading)
                            jj_nsel += B[r].nonzeros();
                        lvl_to_lvl_map[j] = jj_nsel++;
                    }
                    else { // part of the moved p-flow, make its levels consecutives
                        lvl_to_lvl_map[j] = new_leading + jj_sel++;
                    }
                }

                // Update the net->level order
                for (size_t pl=0; pl<npl; pl++)
                    tmp_order[pl] = lvl_to_lvl_map[ net_to_level[pl] ];
                std::swap(net_to_level, tmp_order);

                // for (int k : net_to_level)
                //     cout << setw(3) << k;
                // cout << endl;

                // for (int k : lvl_to_lvl_map)
                //     cout << setw(3) << k;
                // cout << endl;

                // Reorder B (the method extract_reinsert_row() expects it to be ordered).
                reorder_basis(B, lvl_to_lvl_map);
                reduced_row_footprint_form(B);
                assert(sum_of_footprints(B) == new_score);
            }
            iter++; // Another row tested.
        }
    }
    while (changed);
}

//---------------------------------------------------------------------------------------

// Find the minimum score that we can obtain if we compact all places of @row.
// All leading positions are considered. On exit, @new_leading contains the leading
// position that generates the minimum (returned) score. 
// The array @selected_cols is initialized to mark the non-zero columns of @row.
size_t flow_basis_metric_t::extract_reinsert_row(size_t row, const std::vector<int>& net_to_level,
                                                 size_t* new_leading, std::vector<bool>& selected_cols) 
{
    assert(net_to_level.size() == npl); // expect to be called with an initial guess
    assert(selected_cols.size() == npl);
    std::vector<int> lvl_to_lvl_map(npl);


    const sparse_vector_t& the_row = B[row];
    const size_t ROWNNZ = the_row.nonzeros();
    const size_t NPL = the_row.size();
    std::fill(selected_cols.begin(), selected_cols.end(), false);
    size_t jj = 0;
    for (auto&& el : the_row) 
        selected_cols[el.index] = true;

    // Move all selected rows to the left by remapping the levels
    jj = 0;
    for (size_t j=0; j<NPL; j++)
        if (selected_cols[j])
            lvl_to_lvl_map[j] = jj++;
    for (size_t j=0; j<NPL; j++)
        if (!selected_cols[j])
            lvl_to_lvl_map[j] = jj++;

    flow_basis_t B2 = B; // B *must* be ordered in net_to_level order
    reorder_basis(B2, lvl_to_lvl_map);
    // Beware: it only works in reduced row footprint form, because
    // the method reduced_row_footprint_form_range() used below
    // assumes that all rows above each trailing is zeroed.
    reduced_row_footprint_form(B2);

    size_t best_score = sum_of_footprints(B2);
    size_t best_k = 0;
    // cout << setw(4) << best_score;

    for (size_t hh=0; hh < (NPL - ROWNNZ - 1); hh++) {
        // Advance all columns in [k, k+ROWNNZ) of one position
        size_t start_row = B2.size(), end_row = 0;
        for (size_t i=0; i<B2.size(); i++) {
            if (B2[i].trailing() < hh || hh+ROWNNZ < B2[i].leading())
                continue; // No need to sort
            bool changed = false;
            for (auto& el : B2[i].data()) {
                if (el.index == hh + ROWNNZ) {
                    el.index = hh;
                    changed = true;
                }
                else if (hh <= el.index && el.index < hh + ROWNNZ) {
                    el.index++;
                    changed = true;
                }
            }
            if (changed) {
                std::sort(B2[i].data().begin(), B2[i].data().end());
                B2[i].verify_invariants();
                canonicalize_sign(B2[i]);
                start_row = min(start_row, i);
                end_row = max(end_row, i);
            }
        }
        // Rebuild the row footprint form
        reduced_row_footprint_form_range(B2, start_row, end_row);

        // Get the new score after the column repositioning
        size_t score = sum_of_footprints(B2);
        if (score < best_score) {
            best_score = score;
            best_k = hh + 1;
        }
        // cout << setw(4) << score;

#if 0
        // DEBUG CODE: Get the same score using the standard reorder method
        for (size_t j=0, jj_sel=0, jj_nsel=0; j<NPL; j++) {
            if (!selected_cols[j]) {
                if (jj_nsel == (hh + 1))
                    jj_nsel += ROWNNZ;
                lvl_to_lvl_map[j] = jj_nsel++;
            }
            else {
                lvl_to_lvl_map[j] = (hh + 1) + jj_sel++;
            }
        }

        flow_basis_t B3 = B;
        reorder_basis(B3, lvl_to_lvl_map);
        row_footprint_form(B3);
        size_t sc = sum_of_footprints(B3);

        if (sc != score) {
            cout << endl << endl;
            cout << "hh="<<hh<<endl;
            cout << "start_row="<<start_row<<endl;
            cout << "end_row="<<end_row<<endl;
            print_flow_basis(B2);
            cout << endl << endl;
            print_flow_basis(B3);
            cout << endl << endl;
        }
        assert(sc == score);


        std::vector<int> tmp_order(npl);
        for (size_t pl=0; pl<npl; pl++)
            tmp_order[pl] = lvl_to_lvl_map[ net_to_level[pl] ];

        B3 = get_flow_basis();
        reorder_basis(B3, tmp_order);
        row_footprint_form(B3);
        sc = sum_of_footprints(B3);
        assert(sc == score);
#endif
    }
    // cout << endl;

    *new_leading = best_k;
    return best_score;
}

//---------------------------------------------------------------------------------------

#if 0
typedef dlcrs_mat::doubly_linked_crs_matrix<int> flow_matrix_t;
typedef flow_matrix_t::row_vector_type  flow_vector_t;

//---------------------------------------------------------------------------------------

void print_flow_matrix(const flow_matrix_t& B) {
    for (size_t i=0; i<B.num_rows(); i++) {
        for (size_t j=0; j<B.num_cols(); j++) {
            if (B[i].leading() <= j && j <= B[i].trailing())
                cout << setw(3) << B[i][j];
            else
                cout << "  .";
        }
        cout << endl;
    }
    cout << endl;
}

//---------------------------------------------------------------------------------------

flow_vector_t linear_comb(int mult1, const flow_vector_t& vec1, 
                          int mult2, const flow_vector_t& vec2) 
{
    int val1, val2;
    ssize_t j;
    size_t reserved = 0;

    // Phase 1: compute allocation space and verify possible overflows
    auto it1 = vec1.begin(), it2 = vec2.begin();
    while(-1 != (j = traverse_both(it1, vec1, val1, it2, vec2, val2))) {
        ssize_t value = ssize_t(val1) * mult1 + ssize_t(val2) * mult2;
        if (value > INT_MAX || value < INT_MIN) 
            throw rgmedd_exception("Integer overflow when combining flows.");
        if (value != 0)
            reserved++;
    }

    // Phase 2: create the vector
    flow_vector_t res(vec1.size());
    res.reserve(reserved);
    it1 = vec1.begin(), it2 = vec2.begin();
    while(-1 != (j = traverse_both(it1, vec1, val1, it2, vec2, val2))) {
        ssize_t value = ssize_t(val1) * mult1 + ssize_t(val2) * mult2;
        if (value != 0)
            res.insert_element(j, value);
    }

    return res;
}

//---------------------------------------------------------------------------------------

void canonicalize_sign(flow_vector_t& vec) {
    // Always make the first entry positive
    if (vec.nonzeros() > 0 && vec.front_nonzero().value < 0) {
        for (size_t i=0; i<vec.nonzeros(); i++)
            vec.set_nnz_value(i, -vec.ith_nonzero(i).value);
    }
}

//---------------------------------------------------------------------------------------

void canonicalize(flow_vector_t& vec) {
    // Always make the first entry positive
    canonicalize_sign(vec);
    // Get the g.c.d. of the vector entries
    int g = -1;
    for (size_t i=0; i<vec.nonzeros(); i++) {
        if (vec.ith_nonzero(i).value != 0) {
            if (g == -1) // first entry
                g = abs(vec.ith_nonzero(i).value);
            else
                g = gcd(g, abs(vec.ith_nonzero(i).value));
            if (g == 1)
                return;
        }
    }
    if (g > 1) {
        for (size_t i=0; i<vec.nonzeros(); i++)
            vec.set_nnz_value(i, vec.ith_nonzero(i).value / g);
    }
}

//---------------------------------------------------------------------------------------

// Annul column j of row B[i] by summing to it row B[k] with an appropriate multiplier
inline void annul_column_of_B_row(flow_matrix_t& B, int i, int k, int j) {
    assert(B[i][j] != 0);
    // Find multipliers
    int mult_k = abs(B[i][j]);
    int mult_i = abs(B[k][j]);
    int gcd_ik = gcd(mult_k, mult_i);
    mult_k /= gcd_ik;
    mult_i /= gcd_ik;
    if (sign(B[i][j]) == sign(B[k][j]))
        mult_k *= -1;
    // Sum and make canonical
    flow_vector_t new_row_i = linear_comb(mult_k, B[k], mult_i, B[i]);
    B.replace_row(i, new_row_i);
    canonicalize(B[i]);
    assert(B[i][j] == 0);
}

//---------------------------------------------------------------------------------------

// // Get the basis B into reduced row footprint form
// void reduced_row_footprint_form(flow_basis_t& B) {
//     for (int k=0; k<B.size(); k++) {
//         // Find the k-th pivot row
//         int i_max = k;
//         for (int i=k; i<B.size(); i++)
//             if (B[i].nonzeros() > 0 && B[i].leading() < B[i_max].leading())
//                 i_max = i;
//         // Move the pivot in position k
//         std::swap(B[i_max], B[k]);
//         // Annull column j0 to all the rows below the pivot (row k)
//         const int j0 = B[k].leading();
//         for (int i=k+1; i<B.size(); i++) { // Get into a row-echelon form
//             if (B[i][j0] != 0)
//                 annul_column_of_B_row(B, i, k, j0);
//         }
//     }
//     // Step 2: Find row-trailing entries and annul all entries above each of them. 
//     for (int k=B.size()-1; k>=0; k--) {
//         // Annul the last column of B[k] to all the rows above the pivot row k
//         const int jN = B[k].trailing();
//         // const int j0 = B[k].leading();
//         for (int i=k-1; i>=0; i--) { // Get into our modified row-echelon form
//             if (B[i][jN] != 0) /*&& jN == B[i].trailing()*/
//                 annul_column_of_B_row(B, i, k, jN);
//         }
//     }
// }
// Get the basis B in row footprint form
void reduced_row_footprint_form(flow_matrix_t& B, ssize_t start_row, ssize_t end_row) {
    for (int k=start_row; k<=end_row; k++) {
        // Find the k-th pivot row
        int i_max = k;
        for (int i=k; i<B.num_rows(); i++)
            if (B[i].nonzeros() > 0 && B[i].leading() < B[i_max].leading())
                i_max = i;
        // Move the pivot in position k
        B.swap_rows(i_max, k);
        // B.verify_integrity(); // TODO: remove
        // Annull column j0 to all the rows below the pivot (row k)
        const int j0 = B[k].leading();
        flow_matrix_t::column_iterator cit = B.begin_col(k, B[k].begin());
        cit++;
        while (cit != B.end_col(j0)) {
            size_t i = cit->row;
            cit++;
            assert(B[i][j0] != 0);
            annul_column_of_B_row(B, i, k, j0);
        }
    }
    // Step 2: Find row-trailing entries and annul all entries above each of them. 
    // This step is different from the one of the reduced row footprint form.
    // We need to check the trailings by columns, and subtract them from the highest
    // trailing to the lowest trailing, not in row order.
    for (int k=B.num_rows() - 1; k>=0; k--) {
        // Annul the last column of B[k] to all the rows above the pivot row k
        const int jN = B[k].trailing();
        flow_matrix_t::column_iterator cit = B.begin_col(k, B[k].begin() + (B[k].nonzeros() - 1));
        cit--;
        while (cit != B.end_col(jN)) {
            int i = cit->row;
            cit--; // do before annull, otherwise the entry could disappear
            assert(jN <= B[i].trailing());
            annul_column_of_B_row(B, i, k, jN);
        }
    }

    // ssize_t jN = B.num_cols() - 1;
    // for (ssize_t hh=end_row; hh>=start_row; hh--) {
    //     // Find the first non-empty column less than jN, and update jN
    //     flow_matrix_t::column_iterator cit;
    //     while (jN >= 0) {
    //         cit = B.last_col(jN);

    //         // Find the row that will annul the others
    //         while (cit != B.end_col(jN) && B[cit->row].trailing() != jN)
    //             --cit;
    //         if (cit != B.end_col(jN))
    //             break;
    //         jN--;
    //     }
    //     assert(jN >= 0);

    //     int k = cit->row; // row k will annul this column to all rows i, i<k
    //     cit--;
    //     while (cit != B.end_col(jN)) {
    //         int i = cit->row;
    //         cit--; // do before annull, otherwise the entry could disappear
    //         assert(jN <= B[i].trailing());
    //         annul_column_of_B_row(B, i, k, jN);
    //     }
    //     jN--;
    // }
}

//---------------------------------------------------------------------------------------

void reduced_row_footprint_form(flow_matrix_t& B) {
    return reduced_row_footprint_form(B, 0, B.num_rows() - 1);
}

//---------------------------------------------------------------------------------------

void initialize_flow_matrix(flow_matrix_t& B) {
    const flow_basis_t& pbasis = get_flow_basis();
    B = flow_matrix_t(pbasis.size(), npl);
    for (size_t i=0; i<pbasis.size(); i++) {
        flow_vector_t row(npl);
        row.reserve(pbasis[i].nonzeros());
        for (auto&& el : pbasis[i])
            row.insert_element(el.index, el.value);
        B.replace_row(i, row);
    }
}

//---------------------------------------------------------------------------------------

size_t sum_of_invariant_ranks(const flow_matrix_t& B) {
    size_t SoIR = 0;
    for (size_t i=0; i<B.num_rows(); i++)
        SoIR += B[i].trailing() - B[i].leading() + 1;
    return SoIR;
}

//---------------------------------------------------------------------------------------

size_t measure_SoIR(const std::vector<int> &net_to_level)
{
    flow_matrix_t B;
    initialize_flow_matrix(B);

    // Sort columns according to @net_to_level
    B.sort_columns(net_to_level);

    // Gaussian elimination: move B in reduced footprint row form
    reduced_row_footprint_form(B);

    return sum_of_invariant_ranks(B);
}

//---------------------------------------------------------------------------------------

// Find the minimum score that we can obtain if we compact all places of @row.
// All leading positions are considered. On exit, @new_leading contains the leading
// position that generates the minimum (returned) score. 
// The array @selected_cols is initialized to mark the non-zero columns of @row.
size_t extract_reinsert_row(size_t row, const std::vector<int>& net_to_level,
                            size_t* new_leading, std::vector<bool>& selected_cols,
                            flow_matrix_t& B) 
{
    assert(net_to_level.size() == npl); // expect to be called with an initial guess
    assert(selected_cols.size() == npl);
    std::vector<int> lvl_to_lvl_map(npl);

    const flow_vector_t& the_row = B[row];
    const size_t ROWNNZ = the_row.nonzeros();
    std::fill(selected_cols.begin(), selected_cols.end(), false);
    size_t jj = 0;
    for (auto&& el : the_row) 
        selected_cols[el.index] = true;

    // Move all selected rows to the left by remapping the levels
    jj = 0;
    for (size_t j=0; j<B.num_cols(); j++)
        if (selected_cols[j])
            lvl_to_lvl_map[j] = jj++;
    for (size_t j=0; j<B.num_cols(); j++)
        if (!selected_cols[j])
            lvl_to_lvl_map[j] = jj++;

    flow_matrix_t B2 = B; // B *must* be ordered in net_to_level order
    B2.sort_columns(lvl_to_lvl_map);
    // Beware: it only works in reduced row footprint form, because
    // the method reduced_row_footprint_form_range() used below
    // assumes that all rows above each trailing is zeroed.
    reduced_row_footprint_form(B2);

    size_t best_score = sum_of_invariant_ranks(B2);
    size_t best_k = 0;
    // cout << setw(4) << best_score;

    for (size_t hh=0; hh < (B2.num_cols() - ROWNNZ - 1); hh++) {
        // Advance all columns in [k, k+ROWNNZ) of one position
        size_t start_row, end_row;
        B2.slide_backward(hh, ROWNNZ, start_row, end_row);

        // Rebuild the row footprint form
        reduced_row_footprint_form(B2, start_row, end_row);

        // Get the new score after the column repositioning
        size_t score = sum_of_invariant_ranks(B2);
        if (score < best_score) {
            best_score = score;
            best_k = hh + 1;
        }
    }
    // cout << endl;

    *new_leading = best_k;
    return best_score;
}

//---------------------------------------------------------------------------------------

// This method implements an heuristic that compact the dominant p-flows of a variable 
// order. At each iteration, a p-flow is selected, its places are made consecutive
// and a new leading position is searched using a sliding window, to reduce the score.
// If a good position is found, the variable order is updated and the loop restarts.
void compact_basis_row_min(std::vector<int> &net_to_level)
{
    assert(net_to_level.size() == npl); // expect to be called with an initial guess
    std::vector<bool> selected_cols(npl);
    std::vector<int> lvl_to_lvl_map(npl);
    std::vector<int> tmp_order(npl);
    std::vector<int> level_to_net(npl);

    for (int i=0; i<net_to_level.size(); i++)
        level_to_net[ net_to_level[i] ] = i;

    flow_matrix_t B;
    initialize_flow_matrix(B);
    B.sort_columns(net_to_level);
    reduced_row_footprint_form(B);
    size_t best_score = sum_of_invariant_ranks(B), new_leading, iter = 0;
    cout << "   starting score="<<best_score<<endl;

    // Iterate until a fixed point is reached
    bool changed = true;
    do {
        changed = false;

        for (size_t r = 0; r<B.num_rows(); r++) {
            if (B[r].nonzeros() == 1) // constant place
                continue; // Any position is ok.
            // Search the leading column that gets the minimum score
            // if the p-flow of row r is made consecutive, starting from such column.
            size_t new_score = extract_reinsert_row(r, net_to_level,
                                                    &new_leading, selected_cols, B);
            if (new_score < best_score) {
                changed = true;
                cout << "   iter="<<iter<<"  score="<<new_score<<endl;
                best_score = new_score;

                // Update the reverse level -> net order
                for (int i=0; i<net_to_level.size(); i++)
                    level_to_net[ net_to_level[i] ] = i;

                // cout << "new_leading="<<new_leading<<endl;
                // for (int k : net_to_level)
                //     cout << setw(3) << k;
                // cout << endl;

                // for (int k : selected_cols)
                //     cout << setw(3) << k;
                // cout << endl;

                // Build the "old level" -> "new level" transformation
                for (size_t j=0, jj_sel=0, jj_nsel=0; j<npl; j++) {
                    if (!selected_cols[j]) {
                        if (jj_nsel == new_leading)
                            jj_nsel += B[r].nonzeros();
                        lvl_to_lvl_map[j] = jj_nsel++;
                    }
                    else { // part of the moved p-flow, make its levels consecutives
                        lvl_to_lvl_map[j] = new_leading + jj_sel++;
                    }
                }

                // Update the net->level order
                for (size_t pl=0; pl<npl; pl++)
                    tmp_order[pl] = lvl_to_lvl_map[ net_to_level[pl] ];
                std::swap(net_to_level, tmp_order);

                // for (int k : net_to_level)
                //     cout << setw(3) << k;
                // cout << endl;

                // for (int k : lvl_to_lvl_map)
                //     cout << setw(3) << k;
                // cout << endl;

                // Reorder B (the method extract_reinsert_row() expects it to be ordered).
                B.sort_columns(lvl_to_lvl_map);
                reduced_row_footprint_form(B);

                // if (sum_of_invariant_ranks(B) != new_score) {
                //     cout << "sum_of_invariant_ranks(B) = " << sum_of_invariant_ranks(B) << endl;
                //     cout << "new_score = " << new_score << endl;
                //     print_flow_matrix(B);
                // }
                assert(sum_of_invariant_ranks(B) == new_score);
            }
            iter++; // Another row tested.
        }
    }
    while (changed);
}

#endif


//---------------------------------------------------------------------------------------

// Support structure that implements the iteration step of
// the FORCE with Transitions and Invariants algorithm
struct ti_force {
    flow_basis_metric_t& fbm;
    const trans_span_set_t& trns_set;
    bool use_trn_cogs;
    // P-invariant basis
    const flow_basis_t& pbasis;
    const size_t ninv;
    // Support vectors
    std::vector<int> level_to_net;
    std::vector<int> num_trns_of_place, num_inv_of_place;
    std::vector<double> t_cog, i_cog, hy_pos;
    std::vector<std::pair<double, int>> sorter;
    std::vector<int> num_inv_per_place;

    double TW, IW; // transition and invariant weights

    ti_force(flow_basis_metric_t& _fbm, const trans_span_set_t& _ts, bool _ut) 
    : fbm(_fbm), trns_set(_ts), use_trn_cogs(_ut), 
      pbasis(get_flow_basis()), ninv(get_num_invariants())
    { }

    void initialize() {
        // Initialize the data structures used in the force loop
        level_to_net.resize(npl);
        num_trns_of_place.resize(npl);
        num_inv_of_place.resize(npl);
        t_cog.resize(ntr);
        i_cog.resize(ninv);
        hy_pos.resize(npl);
        sorter.resize(npl);
        num_inv_per_place.resize(npl);
        TW = 1.0 / ntr;
        IW = 1.0 / ninv;

        // Count the number of invariants that span each place
        for (auto&& row : fbm.B) {
            for (auto&& el : row) {
                num_inv_per_place[ level_to_net[el.index] ]++;
            }
        }

        // Count the number of transitions connected to each place
        if (use_trn_cogs) {
            std::fill(num_trns_of_place.begin(), num_trns_of_place.end(), 0);
            for (auto&& trn : trns_set.trns) {
                for (auto&& ioh : trn.entries)
                    num_trns_of_place[ ioh.place ]++;
            }
        }
    }

    double
    force_iter_start(const std::vector<int>& var_position) 
    {
        // Initialize variable positions using the initial order
        // var_position = net_to_level;

        // Move the P-invariant basis in footprint form according to the current variable order
        flow_basis_t& B = fbm.B; // Use the fbm's B, do not reallocate
        B = pbasis;
        reorder_basis(B, var_position);
        reduced_row_footprint_form(B);

        // Level -> place mapping
        for (int i=0; i<var_position.size(); i++)
            level_to_net[ var_position[i] ] = i;

        // Count the number of invariants that span each place
        std::fill(num_inv_per_place.begin(), num_inv_per_place.end(), 0);
        for (auto&& row : fbm.B) {
            for (auto&& el : row)
                num_inv_per_place[ level_to_net[el.index] ]++;
        }

        // Recompute center-of-gravity of each P-semiflow (I-COG)
        //   I-COG(i) = Sum( grade(p) ) / num(p)      where variable p is connected with i
        for (size_t i=0; i<B.size(); i++) {
            i_cog[i] = 0.0;
            size_t num_pl = 0;
            for (auto&& el : B[i]) {
                i_cog[i] += var_position[ level_to_net[el.index] ];
                num_pl++;
            }
            safe_div(i_cog[i], num_pl);
        }
        // Recompute the center-of-gravity (T-COG) of each transition
        //   T-COG(t) = Sum( grade(p) ) / num(p)      where variable p is connected with t
        if (use_trn_cogs) {
            for (auto&& trn : trns_set.trns) {
                t_cog[trn.trn] = 0.0;
                size_t num_pl = 0;
                for (auto&& ioh : trn.entries) {
                    t_cog[trn.trn] += var_position[ ioh.place ];
                    num_pl++;
                }
                safe_div(t_cog[trn.trn], num_pl);
            }
        }

        // Compute the new hyper-position of each variable, using the  T/I-COGs
        //   hy_pos(p) = Sum( T-COG[t] ) / num(t) +     where transition t is connected with p
        //              + Sum( I-COG[i] ) / num(i)      where invariant i is connected with p
        std::fill(hy_pos.begin(), hy_pos.end(), 0.0);

        for (size_t i=0; i<B.size(); i++) {
            for (auto&& el : B[i])
                hy_pos[ level_to_net[el.index] ] += i_cog[i] * IW;
        }
        if (use_trn_cogs) {
            for (auto&& trn : trns_set.trns) {
                for (auto&& ioh : trn.entries)
                    hy_pos[ ioh.place ] += t_cog[trn.trn] * TW;
            }
        }

        for (int p = 0; p < npl; p++)
            safe_div(hy_pos[p], num_inv_per_place[p] * IW + (use_trn_cogs ? num_trns_of_place[p] * TW : 0));

        // Compute a PTS-like score from the new hyper_positions
        double score = 0.0;
        for (size_t i=0; i<B.size(); i++) {
            for (auto&& el : B[i]) {
                score += std::abs( hy_pos[ level_to_net[el.index] ] - i_cog[i] ) * IW;
            }
        }
        if (use_trn_cogs) {
            for (auto&& trn : trns_set.trns) {
                for (auto&& ioh : trn.entries)
                    score += std::abs( hy_pos[ ioh.place ] - t_cog[trn.trn] ) * TW;
            }
        }
        return score;
    }


    void force_iter_finalize(std::vector<int>& var_position) {
        // Reorder the variables according to their hyper_positions, and assign them a new
        // integer positioning from 0 to npl (which becomes the new variable
        // position in the next iteration)
        for (int p = 0; p < npl; p++)
            sorter[p] = std::make_pair(hy_pos[p], p);
        std::sort(sorter.begin(), sorter.end());
        for (int p = 0; p < npl; p++)
            // var_position[p] = sorter[p].second;
            var_position[ sorter[p].second ] = p;        
    }
};

//---------------------------------------------------------------------------------------


static const int TI_FORCE_MAX_SECONDS = 20;

// transitions/invariants force
void var_order_ti_force(flow_basis_metric_t& fbm, 
                        const trans_span_set_t& trns_set,
                        std::vector<int>& net_to_level,
                        bool use_trn_cogs,
                        bool verbose) 
{
    assert(net_to_level.size() == npl);

    ti_force F(fbm, trns_set, use_trn_cogs);
    F.initialize();
    double last_score = -1;

    clock_t time_start = clock();
    // Initialize variable positions using the initial order
    std::vector<int> var_position = net_to_level;

    const int MAX_STEPS = 50;
    const int MIN_STEPS = 4; //int(ceil(log(npl)) + 1) * 2; // c * log(P), as suggested in the paper.
    for (int step = 0; step < MAX_STEPS; step++) {
        // Compute hyper-positions of places and get a score
        double score = F.force_iter_start(var_position);

        // Check if we reached convergence. End as soon as the new order
        // does not improve the score over the previous order.
        if (verbose && !running_for_MCC())
            cout << step << "\t" << score << endl;
        if (last_score >= 0 && last_score <= score && step > MIN_STEPS)
            break; // Not converging any longer
        last_score = score;

        // Check if we consumed too much time doing FORCE iterations
        clock_t elapsed = clock() - time_start;
        if (elapsed > TI_FORCE_MAX_SECONDS * CLOCKS_PER_SEC) {
            if (verbose && !running_for_MCC())
                cout << "Quit FORCE iteration loop." << endl;
            break;
        }

        // Reorder the variables according to their hyper_positions
        F.force_iter_finalize(var_position);
    }

    // Copy the final order
    net_to_level = var_position;
}

//---------------------------------------------------------------------------------------

// Just return the PTS-INV metric value of a given variable order
double
metric_PTS_INV(flow_basis_metric_t& fbm, 
               const trans_span_set_t& trns_set,
               const std::vector<int>& net_to_level,
               bool use_weights)
{
    ti_force F(fbm, trns_set, true);
    F.initialize();
    if (!use_weights)
        F.TW = F.IW = 1.0; // No weights
    return F.force_iter_start(net_to_level);
}


//---------------------------------------------------------------------------------------

// A repositioner of place groups for annealing.
// It takes a random invariant and moves all places in that invariant into a new location
struct inv_group_repositioner {
    flow_basis_metric_t* fbm;
    const trans_span_set_t& trns_set;
    bool force_step;
    ti_force F;
    std::vector<int> level_to_net;
    std::vector<int> rev_order;
    std::vector<bool> selected;

    inv_group_repositioner(flow_basis_metric_t *_fbm, const trans_span_set_t& _trns_set, bool _force_step)
    : fbm(_fbm), trns_set(_trns_set), force_step(_force_step), F(*fbm, trns_set, true)
    { 
        if (force_step)
            F.initialize();
    }

    void operator() (const std::vector<int>& net_to_level, 
                     std::vector<int>& new_order)
    {
        level_to_net.resize(npl);
        rev_order.resize(npl);
        selected.resize(npl);

        for (int i=0; i<net_to_level.size(); i++)
            level_to_net[ net_to_level[i] ] = i;
        // Select a row at random and compact all its places
        size_t row = genrand64_int63() % fbm->B.size();
        std::fill(selected.begin(), selected.end(), false);
        for (auto&& el : fbm->B[row]) {
            selected[el.index] = true;
        }
        // Randomly select the new position of the place group
        size_t new_leading = genrand64_int63() % (npl - fbm->B[row].nonzeros());

        size_t j = 0;
        for (size_t p=0; p<new_leading; p++)
            if (!selected[p])
                rev_order[j++] = level_to_net[p];
        // Take all selected places
        for (size_t p=0; p<npl; p++)
            if (selected[p])
                rev_order[j++] = level_to_net[p];
        // Take the remaining unselected places
        for (size_t p=new_leading; p<npl; p++)
            if (!selected[p])
                rev_order[j++] = level_to_net[p];

        for (size_t p=0; p<npl; p++)
            new_order[ rev_order[p] ] = p;

        // Reorder all the places using TI-Force
        if (force_step) {
            const size_t MAX_ITERS = 5;
            double last_score = -1;
            for (size_t i=0; i<MAX_ITERS; i++) {
                double score = F.force_iter_start(new_order);
                if (last_score > 0 && last_score < score)
                    break;

                last_score = score;
                F.force_iter_finalize(new_order);
            }
        }
    }
};

//---------------------------------------------------------------------------------------

relocation_fn_t
make_invariant_group_repositioner(flow_basis_metric_t *fbm, 
                                  const trans_span_set_t& trns_set,
                                  bool force_step) 
{
    return relocation_fn_t(inv_group_repositioner(fbm, trns_set, force_step));
}

//---------------------------------------------------------------------------------------
















