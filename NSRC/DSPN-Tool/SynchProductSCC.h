/*
 *  SynchProductSCC.h
 *
 *  Synchronized product between a CTMC and a ZDTA, using component-based method.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __SYNCHRONIZED_PRODUCT_SCC_H__
#define __SYNCHRONIZED_PRODUCT_SCC_H__
//=============================================================================

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct SZStructTag;
struct ReachMarkStructTag;
typedef private_integer<size_t, SZStructTag>			szstate_t;
typedef private_integer<size_t, ReachMarkStructTag>		marking_t;
#else
typedef size_t  szstate_t;
typedef size_t  marking_t;
#endif

//=============================================================================
// SYNCHRONIZED PRODUCT CONSTRUCTION
//=============================================================================

// State of the CTMC x ZDTA product MxZ
struct sz_tuple_t {
    sz_tuple_t(marking_t _s = marking_t(size_t(-1)),
               location_t _z = location_t(size_t(-1)))
        : s(_s), z(_z) { }

    marking_t    s; // The CTMC state
    location_t   z; // The ZDTA location (DTA location + clock zone)

    inline bool operator < (const sz_tuple_t &t) const {
        return (s < t.s || (s == t.s && z < t.z));
    }
    inline bool operator == (const sz_tuple_t &t) const {
        return (s == t.s && z == t.z);
    }
    inline bool isTop() const;
    inline bool isBot() const;

    static const sz_tuple_t TOP, BOT;
};

std::function<ostream& (ostream &)>
print_sz(const sz_tuple_t &sz, const RG2 &rg, const DTA &zdta);

inline bool sz_tuple_t::isTop() const { return *this == sz_tuple_t::TOP; }
inline bool sz_tuple_t::isBot() const { return *this == sz_tuple_t::BOT; }

//=============================================================================

typedef table < col::uint_col < marking_t, +2 >, col::uint_col < location_t, +2 >> sz_tuple_table_t;
extern template class table < col::uint_col < marking_t, +2 >, col::uint_col < location_t, +2 >>;

// Synchronized product MxZ bewteen a CTMC and a ZDTA
struct SynchProd : boost::noncopyable {
    MRP 					  mrp;	 ///< Synchronized MxA MRP
    sz_tuple_table_t          SZ;      ///< State space of a CTMC x ZDTA
    szstate_t   			  top;	   ///< Index of the top state in the state space (if any)
    szstate_t   			  bot;	   ///< Index of the failure state (if any)

    std::map<sz_tuple_t, szstate_t>  szToInd;    ///< State index from <s,z> tuple

    inline sz_tuple_t get_sz_tuple(size_t n) const
    { return sz_tuple_t(get_at<0>(SZ, n), get_at<1>(SZ, n)); }
};

//=============================================================================

struct BuildSynchProdHelper : boost::noncopyable {
    BuildSynchProdHelper(size_t N = 0) : Q(N, N), Qbar(N, N), Delta(N, N) { }

    std::queue<szstate_t>              state_queue; ///< Frontier of unexplored <s,l,c> states
    numerical::matrix_map<double>      Q;
    numerical::matrix_map<double>      Qbar;
    numerical::matrix_map<double>      Delta;
    size_t    numIntStates = 0;  // Number of tangible internal states with non-absorbing behavior.
    size_t    numExtStates = 0;  // Number of tangible frontier states.
    size_t    numInitSZs = 0; // Initial states count.
};

//=============================================================================

double
ModelCheck_CTMC_ZDTA_Forward_SCC(const RG2 &rg, const DTA &zdta,
                                 marking_t s0, const SolverParams &spar,
                                 VerboseLevel verboseLvl);

void
ModelCheck_CTMC_ZDTA_Backward_SCC(const RG2 &rg, const DTA &zdta,
                                  ublas::vector<double> &probs, const SolverParams &spar,
                                  VerboseLevel verboseLvl);

//=============================================================================
#endif   // __SYNCHRONIZED_PRODUCT_SCC_H__