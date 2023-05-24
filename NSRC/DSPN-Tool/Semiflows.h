/*
 *  Semiflows.h
 *
 *  Computation of minimal P/T-semiflows, flows, basis and place bounds.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __SEMIFLOWS_H__
#define __SEMIFLOWS_H__
//=============================================================================

enum class InvariantKind {
    PLACE,
    TRANSITION
};

//-----------------------------------------------------------------------------

enum class FlowMatrixKind {
    EMPTY, 
    INCIDENCE, 
    SEMIFLOWS, 
    BASIS,
    INTEGER_FLOWS,
    NONE,
    // NESTED_FLOW_SPAN
};

//-----------------------------------------------------------------------------

enum class SystemMatrixType {
    REGULAR, TRAPS, SIPHONS
};

//-----------------------------------------------------------------------------

struct invariants_spec_t {
    InvariantKind invknd;
    SystemMatrixType system_kind;
    FlowMatrixKind matk;
    size_t suppl_flags;
};

//-----------------------------------------------------------------------------

// Returns the GreatSPN file extension for the given type of flow (like .pin, .tin, etc...)
std::string GetGreatSPN_FileExt(invariants_spec_t is);
// Return the name of the type of flows (like "PLACE SEMIFLOWS" or "TRANSITIONS FLOW BASIS")
const char* GetFlowName(InvariantKind fk, FlowMatrixKind matk, SystemMatrixType smt);

//-----------------------------------------------------------------------------

// An entry (flow) in the incidence matrix
struct flow_entry_t {
    size_t i, j;
    int card;
    inline flow_entry_t(size_t _i, size_t _j, int _card) : i(_i), j(_j), card(_card) {};
    inline flow_entry_t(const flow_entry_t&) = default;
    inline flow_entry_t(flow_entry_t&&) = default;
    inline flow_entry_t& operator=(const flow_entry_t&) = default;
    inline flow_entry_t& operator=(flow_entry_t&&) = default;

    bool operator < (const flow_entry_t& f) const;
};

ostream& operator<<(ostream& os, const flow_entry_t& mrow);

//-----------------------------------------------------------------------------

// Callback that prints the state of the algorithm during the computation.
class flow_algorithm_printer_t {
public:
    virtual void advance(const char* algo, size_t step, size_t totalSteps, 
                         size_t size_K, ssize_t num_prods) = 0;
};

//-----------------------------------------------------------------------------

const size_t FM_POSITIVE_SUPPLEMENTARY = 1;
const size_t FM_NEGATIVE_SUPPLEMENTARY = 2;
const size_t FM_ON_THE_FLY_SUPPL_VARS = 4;
const size_t FM_REDUCE_SUPPLEMENTARY_VARS = 8;

//-----------------------------------------------------------------------------

// Matrix of flows in a P/T net. 
// Can be used to represent both P/T-semiflows and a P/T-flow basis.
// The flow matrix is made by the combination of two separate matrices [D|A].
class flow_matrix_t {
public:
    struct spvec_int_tag {
        typedef size_t index_type;
        typedef int value_type;
        typedef base_index_value_pair<index_type, value_type>  index_value_pair;
        static inline bool allow_zeros() { return true; }
    };
    typedef sparsevector<spvec_int_tag>    spintvector;
    // typedef sparsevector<size_t, bool>   spboolvector;

    flow_matrix_t(size_t N, size_t N0, size_t M, InvariantKind k, SystemMatrixType smt, int suppl_flags, 
                  bool add_extra_vars, bool use_Colom_pivoting, bool extra_vars_in_support);
    flow_matrix_t(const flow_matrix_t&) = delete;
    flow_matrix_t(flow_matrix_t&&) = default;
    flow_matrix_t& operator=(const flow_matrix_t&) = delete;
    flow_matrix_t& operator=(flow_matrix_t&&) = default;

    struct const_iterator;

    inline size_t num_flows() const  { return mK.size(); }
    inline const_iterator begin() const { return const_iterator(mK.begin()); }
    inline const_iterator end() const   { return const_iterator(mK.end()); }

    // A row of the flow matrix [D|A]
    struct row_t {
        inline row_t(flow_matrix_t& f) : D(f.N), A(f.M) { }
        inline row_t(const row_t&) = delete;
        inline row_t(row_t&&) = default;
        inline row_t& operator=(const row_t&) = delete;
        inline row_t& operator=(row_t&&) = default;

        // Row of matrix D, size N. 
        // Usually D starts as the identity and ends as the flow matrix.
        spintvector  D;
        // Row of matrix A, size M.
        // Initialized as the incidence matrix, is zeroed/triangularized by the computation.
        // After the semiflow/basis generation, the A matrix can be dropped.
        spintvector  A;
        // Does D have negative entries? (only used by "span of flows")
        size_t neg_D = 0;
        // Step when this row entry has been generated (only used by "span of flows")
        size_t gen_step = 0;

        inline bool is_negative() const { return neg_D > 0; }

        // Compute the GCD of the non-zero elements in D and A
        int gcd_nnz_DA() const;

        // Check if the support of D2 is included in D, i.e.
        // support(D2) subseteq support(D), where support(.) is the set of non-zero columns.
        bool test_minimal_support_D(const spintvector& D2, const size_t N0) const;

        // Test if the support of the columns with positive entries of D2 is included in D
        // bool test_minimal_positive_support_D(const spintvector& D2, int D_mult) const;

        // Check support(D2) subseteq support(D +/- R)
        // bool test_minimal_support_linear_comb_D(const spintvector& D2, const spintvector& R) const;

        // Test if the two vectors share at least a common non-zero
        // bool test_common_nonzeros(const spintvector& D2) const;

        // Test if the two vectors have a disjoint support for their negative entries
        // bool test_disjoint_supports(const row_t& row2, int mult) const;

        // Test if we could apply the substitution rule of the given vector R
        // bool test_subst(const spintvector& R, int& multR, int& multD) const;

        // Make the vector canonical, i.e. the gcd of its nonzero entries must be 1, and
        // if it has negative entries it must start with a positive one.
        void canonicalize();

        // Create a new row from the linear combination of:  r1*mult1 + r2*mult2
        // Returns false without completing if the sum of D is annulled.
        bool linear_comb_nnD(const row_t& row1, int mult1, const row_t& row2, int mult2);

        // count the # of negative entries in D
        size_t count_negatives_D() const;

        ostream& print(ostream& os, const ssize_t M, const ssize_t N0, bool highlight_annulled) const;
    };

    // For P-semiflows: N=|P|, M=|T| (for T-semiflows: N=|T|, M=|P|)
    // N0 is the same as N when no slack variables are considered, otherwise 
    // N has the additional slack variables, and N0 is the count of proper (non-slack) vars only.
    const size_t N, N0, M;

    // Stores P or T flows?
    const InvariantKind inv_kind;

    // How the system was built
    SystemMatrixType system_kind;

    // Columns for the supplementary variables
    const size_t suppl_flags; // 0 means normal P/T flows, otherwise have supplementary vars.
    // Extra variables should be added dynamically during flow generation?
    const bool add_extra_vars;
    // Use Colom pivoting strategy
    const bool use_Colom_pivoting;
    // include extra variables from the support
    const bool extra_vars_in_support;

    // The content of this flow matrix
    FlowMatrixKind mat_kind;

    // Flow matrix mK=[D|A], kept as a list for ease of adding/removing rows
    std::list<row_t> mK;

    // return the column of a supplementary variable
    // size_t column_of_suppl_var(size_t var, int sign);

    // Drop all the vectors in the A part of the [D|A] matrix.
    void clear_A_vectors();

public:
    // An iterator to view the list of (semi)flows
    struct const_iterator {
        inline const_iterator();
        inline const_iterator(std::list<row_t>::const_iterator _it) : it(_it) { }
        inline const_iterator(const const_iterator&) = default;
        inline const_iterator(const_iterator&&) = default;
        inline const_iterator& operator=(const const_iterator&) = default;
        inline const_iterator& operator=(const_iterator&&) = default;

        inline const_iterator& operator++ () { ++it; return *this; }
        inline const_iterator operator++ (int) { const_iterator i(*this); ++it; return i; }

        inline bool operator == (const const_iterator &i) const { return it == i.it; }
        inline bool operator != (const const_iterator &i) const { return it != i.it; }

        inline const spintvector* operator -> () const { return &it->D; }
        inline const spintvector& operator * () const { return it->D; }

    private:
        std::list<row_t>::const_iterator it;
    };

    friend ostream& operator<<(ostream& os, const flow_matrix_t& msa);
    // friend ostream& operator<<(ostream& os, const flow_matrix_t::row_t& mrow);

    ostream& print(ostream& os, bool highlight_annulled = false) const;

    void save_matrix_A(ostream& os) const;
};

ostream& operator<<(ostream& os, const flow_matrix_t& msa);
// ostream& operator<<(ostream& os, const flow_matrix_t::row_t& mrow);

//-----------------------------------------------------------------------------

// Flow matrix initializer
class incidence_matrix_generator_t {
    flow_matrix_t& f;
    // Set of initial entries (incidence matrix) to be inserted in f
    std::set<flow_entry_t> initEntries;
public:
    incidence_matrix_generator_t(flow_matrix_t& _f) : f(_f) { }
    incidence_matrix_generator_t(const incidence_matrix_generator_t&) = delete;
    incidence_matrix_generator_t(incidence_matrix_generator_t&&) = default;

    // Add an entry of the incidence matrix from i to j with the given cardinality
    // void add_flow_entry(size_t i, size_t j, int cardinality);

    // Insert flows from Petri net arcs
    // void add_flows_from(const PN& pn, bool print_warns = true);

    // Initialize the flow matrix from the inserted entries
    // void generate_matrix();
    void generate_matrix2(const PN& pn, bool print_warns);

    // Add increasing/decreasing flows (for I/D invariants)
    // void add_increase_decrease_flows();
};

//-----------------------------------------------------------------------------

// Semiflows/flow basis generator
class flows_generator_t {
public:
    typedef flow_matrix_t::spintvector   spintvector;

    flows_generator_t(flow_matrix_t& _f, flow_algorithm_printer_t& _p, VerboseLevel _verboseLvl);
    flows_generator_t(const flows_generator_t&) = delete;
    flows_generator_t(flows_generator_t&&) = default;

    // Compute semiflows starting from the incidence matrix
    void compute_semiflows();
    // Compute a flow basis starting from the incidence matrix
    void compute_basis();
    // Compute the minimal general flows (both positive and negative)
    void compute_integer_flows();
    // Reduce non-minimal flows (excluding slack variables for the support check)
    void reduce_non_minimal();
    // drop all entries for the slack variables in D
    void drop_slack_vars_in_D();
    // Compute a span of flows which maximizes the nested property
    // Nesting: if we have A+B, A+C and B=C, then drop the 1st (or the 2nd), because it is
    // easily derived from a single sum (A+B)-(B+C), and B subset {A,B}  (nesting property).
    // void compute_nested_flow_span();

protected:
    // What to report
    const VerboseLevel verboseLvl;

    // Operations printer
    flow_algorithm_printer_t& printer;

    // Target flow matrix that will be filled with P/T semiflows
    flow_matrix_t& f;

    // The count of the positive and negative entries in the columns of A
    std::vector<pair<int, int>> A_cols_count;

    // The step of the method
    size_t step;

    // Initialize pre-computed column sums of matrix A
    void init_A_columns_sum();
    // Update pre-computed column sums of A
    void update_A_columns_sum(const spintvector& A, int mult);

public:
    // Maximum peak number of candidate rows, before quitting
    size_t max_peak_rows = std::numeric_limits<size_t>::max();

    friend ostream& operator<<(ostream&, const flows_generator_t&);
};

ostream& operator<<(ostream& os, const flows_generator_t& fg);

//-----------------------------------------------------------------------------

// Estimated token bounds for a given place in the net.
struct PlaceBounds {
    int lower;   // Lower bound, in tokens.
    int upper;   // Upper bound, in tokens.

    inline bool is_upper_bounded() const { return upper != numeric_limits<int>::max(); }
};
// A vector of place bounds, one for each place.
typedef std::vector<PlaceBounds> place_bounds_t;

//-----------------------------------------------------------------------------

shared_ptr<flow_matrix_t>
ComputeFlows(const PN& pn, InvariantKind kind, FlowMatrixKind mat_kind, 
             SystemMatrixType system_kind, bool detect_exp_growth, 
             int suppl_flags, bool use_Colom_pivoting, 
             bool extra_vars_in_support, VerboseLevel verboseLvl);

void SaveFlows(const flow_matrix_t& msa, ofstream& file);

void PrintFlows(const PN& pn, const flow_matrix_t& msa, const char* cmd, VerboseLevel verboseLvl);

// are all places of a net covered by at least one flow?
bool IsNetCoveredByFlows(const PN& pn, const flow_matrix_t& flows);

//-----------------------------------------------------------------------------

// Compute place bounds using the P-semiflows
void ComputeBoundsFromSemiflows(const PN& pn, const flow_matrix_t& semiflows, 
                                place_bounds_t& bounds);

// Save place bounds in GreatSPN format
void SaveBounds(const place_bounds_t& bounds, ofstream& file);

void LoadBounds(const PN& pn, place_bounds_t& bounds, ifstream& file);

void PrintBounds(const PN& pn, const place_bounds_t& bounds, VerboseLevel verboseLvl);

//-----------------------------------------------------------------------------

// Compute the minimal number of tokens to satisfy each (semi)flow
void ComputeMinimalTokensFromFlows(const PN& pn, 
                                   const flow_matrix_t& semiflows, 
                                   std::vector<int>& m0min);

void SaveMinimalTokens(const std::vector<int>& m0min, ofstream& file);

void PrintMinimalTokens(const PN& pn, const std::vector<int>& m0min, VerboseLevel verboseLvl);

//=============================================================================
#endif // __SEMIFLOWS_H__

