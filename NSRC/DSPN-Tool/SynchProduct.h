/*
 *  SynchProduct.h
 *
 *  Synchronized product between a DTA and a CTMC.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __SYNCHRONIZED_PRODUCT_H__
#define __SYNCHRONIZED_PRODUCT_H__
//=============================================================================

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct SLCStructTag;
struct ReachMarkStructTag;
typedef private_integer<size_t, SLCStructTag>			slcstate_t;
typedef private_integer<size_t, ReachMarkStructTag>		marking_t;
#else
typedef size_t  slcstate_t;
typedef size_t  marking_t;
#endif

//=============================================================================
// SYNCHRONIZED PRODUCT CONSTRUCTION
//=============================================================================

// State of the synchronized product
struct slc_tuple_t {
    slc_tuple_t(marking_t _s = marking_t(size_t(-1)),
                location_t _l = location_t(size_t(-1)),
                clockzone_t _c = clockzone_t(size_t(-1)))
        : s(_s), l(_l), c(_c) { }

    marking_t    s; // The CTMC state
    location_t   l; // The DTA location
    clockzone_t  c; // The clock zone

    inline bool operator < (const slc_tuple_t &t) const {
        return (s < t.s ||
                (s == t.s && l < t.l) ||
                (s == t.s && l == t.l && c < t.c));
    }
};

typedef table < col::uint_col < marking_t, +2 >,
        col::uint_col < location_t, +2 >, col::uint_col<clockzone_t >> slc_tuple_table_t;
extern template class table < col::uint_col < marking_t, +2 >,
                              col::uint_col < location_t, +2 >, col::uint_col<clockzone_t >>;

typedef table<col::uint_col<slcstate_t>> slcstate_table_t;
extern template class table<col::uint_col<slcstate_t>>;

// Synchronized product bewteen a DTA and a CTMC
struct SynchronizedProduct : boost::noncopyable {
    MRP 					  mrp;	 ///< Synchronized MxA MRP
    slcstate_table_t	      inits; ///< Corresponding initial state of M in MxA
    slcstate_t   			  top;	 ///< Index of the top state in MxA mrp
    slcstate_t   			  bot;	 ///< Index of the failure state
    slc_tuple_table_t         SLC;   ///< State space of MxA

    inline slc_tuple_t get_slc_tuple(size_t n) const
    { return slc_tuple_t(get_at<0>(SLC, n), get_at<1>(SLC, n), get_at<2>(SLC, n)); }
};

struct BuildMxAHelper : boost::noncopyable {
    BuildMxAHelper(size_t N = 0) : Q(N, N), Qbar(N, N), Delta(N, N) { }

    std::queue<slcstate_t>             state_queue; ///< Frontier of unexplored <s,l,c> states
    std::map<slc_tuple_t, slcstate_t>  slcToInd;    ///< State index from <s,l,c> tuple
    numerical::matrix_map<double>      Q;
    numerical::matrix_map<double>      Qbar;
    numerical::matrix_map<double>      Delta;
};

void BuildSynchronizedProduct(const RG2 &rg, const DTA &dta, SynchronizedProduct &MxA,
                              marking_t initMark/* -1 considers all initial states */,
                              VerboseLevel verboseLvl);

void PrepareStatePropositions(const RG2 &rg, DTA &dta, const SolverParams &spar,
                              VerboseLevel verboseLvl);

//=============================================================================

double
ModelCheck_CTMC_DTA_Forward(const RG2 &rg, const DTA &dta, marking_t s0,
                            const SolverParams &spar, VerboseLevel verboseLvl);

void
ModelCheck_CTMC_DTA_Backward(const RG2 &rg, const DTA &dta, ublas::vector<double> &probs,
                             const SolverParams &spar, VerboseLevel verboseLvl);

//=============================================================================
#endif   // __SYNCHRONIZED_PRODUCT_H__