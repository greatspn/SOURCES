/*
 *  NewReachabilityGraph.h
 *
 *  Reachability Graph Structure Definitions and Functions.
 *
 *  Created by Elvio Amparore
 *
 */

//=============================================================================
#ifndef __NEW_REACHABILITY_GRAPH_H__
#define __NEW_REACHABILITY_GRAPH_H__
//=============================================================================

#include "compact_table.h"

namespace detail {
template<bool has_sign> struct col_type {};
template<> struct col_type<true>  { typedef table<col::int_col<ssize_t>> type; };
template<> struct col_type<false> { typedef table<col::uint_col<size_t>> type; };
};

template<typename T>
struct sequence_traits<std::set<T>> {
    typedef std::set<T>        				 vector_type;
    typedef T  		 						 value_type;
    typedef typename detail::col_type<std::is_signed<T>::value>::type  table_type;
    typedef typename table_type::value_type  stored_type;
    typedef std::tuple<stored_type>  	     tuple_type;

    inline value_type empty_elem() const { return T(0); }
    inline tuple_type to_tuple(const value_type arg) const {
        return make_tuple((stored_type)arg);
    }
    inline value_type from_tuple(const tuple_type tuple) const {
        return T(std::get<0>(tuple));
    }
#if defined __GNUC__ && !defined __clang__ && __GNUC__ < 6
    // required by GCC
    inline value_type from_tuple(const value_type arg) const { return arg; }
#endif
    inline void swap(sequence_traits<vector_type> &) {}
};

template<typename T>
struct sequence_traits<std::vector<T>> : sequence_traits<std::set<T>> {
    typedef std::vector<T>   vector_type;
    inline void swap(sequence_traits<vector_type> &) {}
};

template<>
struct sequence_traits<SparseMarking> : sequence_traits<std::vector<tokencount_t>> {
    typedef SparseMarking        				vector_type;
    inline void swap(sequence_traits<vector_type> &) {}
};

// Declare the specialized template versions of the compact storages
// extern template struct seq_store<std::set<transition_t>>;

// typedef seq_store<std::set<transition_t>>    transition_set_table;
// typedef transition_set_table::sequence_id    transition_set_id;
// #define EMPTY_TRANSITION_SET   				 transition_set_id::INVALID_SEQ



//=============================================================================

#if defined USE_PRIVATE_TYPES && !defined NDEBUG
//struct ReachMarkStructTag;
struct TransitionSetIdStructTag;
struct VanishPathIdStructTag;
struct VanishPathSetIdStructTag;
//typedef private_integer<int, ReachMarkStructTag>			rgstate_t;
typedef private_integer<size_t, TransitionSetIdStructTag>	transition_set_id;
typedef private_integer<size_t, VanishPathIdStructTag>		vanish_path_id;
typedef private_integer<size_t, VanishPathSetIdStructTag>	vanish_path_set_id;
#else
//typedef int  		rgstate_t;
typedef size_t  	transition_set_id;
typedef size_t		vanish_path_id;
typedef size_t		vanish_path_set_id;
#endif

//-----------------------------------------------------------------------------

enum RgStateType {
    VANISHING,		///< Vanishing state type
    TANGIBLE,		///< Tangible state type
    NUM_RG_STATE_TYPES
};

/// State type names
extern const char *s_RgStateTypeNames[NUM_RG_STATE_TYPES];
extern const char *s_RgStateTypeNamesShort[NUM_RG_STATE_TYPES];

typedef suffix_table<transition_t, transition_set_id, size_t>  transition_set_table;
#define EMPTY_TRANSITION_SET   transition_set_table::END_SET

// Special "Initial Vanishing Marking" transition ID
// #define SOLVE_INITIAL_VANISHING_TRN     transition_t(size_t(-6))

//=============================================================================
//     Reachability Graph Definitions
//=============================================================================

/// A (typed) index of an RgState, made by an index() and a type()
struct rgstate_t {
    inline rgstate_t() { }
    inline rgstate_t(size_t index, RgStateType type) { set(index, type); }
    inline rgstate_t(const rgstate_t &e) = default;
    inline rgstate_t(rgstate_t &&e) = default;
    rgstate_t &operator = (const rgstate_t &e) = default;
    rgstate_t &operator = (rgstate_t &&e) = default;

    // The 0-based index
    inline size_t index() const   {  assert(num != 0); return abs(num) - 1;  }
    // The type (either tangible or vanishing)
    inline RgStateType type() const
    {  assert(num != 0); return num > 0 ? TANGIBLE : VANISHING;  }

    inline void set(size_t index, RgStateType type)
    {  num = (type == TANGIBLE ? ssize_t(index) + 1 : -ssize_t(index) - 1);  }

    inline bool operator== (const rgstate_t e) const { return num == e.num; }
    inline bool operator!= (const rgstate_t e) const { return num != e.num; }
    inline bool operator< (const rgstate_t e) const { return num < e.num; }

    struct InvRgStateTag {};
    inline rgstate_t(InvRgStateTag) : num(0) { }
    inline ssize_t get_internal() const { return num; }

    static inline rgstate_t from_internal(size_t n) { rgstate_t st; st.num = n; return st; }
    static inline ssize_t max_index()
    { return std::numeric_limits<ssize_t>::max() - 1; }
protected:
    ssize_t  num;
};

// An invalid rgstate_t
const rgstate_t INVALID_RGSTATE_INDEX = rgstate_t(rgstate_t::InvRgStateTag());
// An rgstate_t of a tangible state with a non-valid index
const rgstate_t INVALID_TANGIBLE_RGSTATE_INDEX = rgstate_t(rgstate_t::max_index(), TANGIBLE);
// An rgstate_t of a vanishing state with a non-valid index
const rgstate_t INVALID_VANISHING_RGSTATE_INDEX	= rgstate_t(rgstate_t::max_index(), VANISHING);

// Hash function for rgstate_t's
namespace std {
template<> struct hash<rgstate_t> {
    size_t operator()(const rgstate_t &rg) const {
        return std::hash<ssize_t>()(rg.get_internal());
    }
};
};

//-----------------------------------------------------------------------------

enum RgEdgeType {
    TANGIBLE_TO_TANGIBLE,
    TANGIBLE_TO_VANISHING,
    VANISHING_TO_TANGIBLE,
    VANISHING_TO_VANISHING,
    TANGIBLE_TO_TANGIBLE_WITH_VANISHINGS,
    INVALID_EDGE_TYPE,
    //NUM_EDGE_TYPES = 6,
    NUM_VALID_EDGE_TYPES = 5
};

//-----------------------------------------------------------------------------

#define RGEDGE_NUM_LSHIFT   	3
#define RGEDGE_TYPE_MASK   	    ((size_t(1) << RGEDGE_NUM_LSHIFT) - 1)

// A (typed) index of an RG edge
struct rgedge_t {
    inline rgedge_t() { }
    inline rgedge_t(size_t _index, RgEdgeType _type) { set(_index, _type); }
    inline rgedge_t(const rgedge_t &e) = default;
    inline rgedge_t(rgedge_t &&e) = default;
    rgedge_t &operator = (const rgedge_t &e) = default;
    rgedge_t &operator = (rgedge_t &&e) = default;

    // The 0-based edge index
    inline size_t index() const   {  return num >> RGEDGE_NUM_LSHIFT;  }
    // The type (one of the 5 valid RgEdgeType types, or an invalid edge type).
    inline RgEdgeType type() const
    {  return RgEdgeType(num & RGEDGE_TYPE_MASK);  }

    inline void set(size_t _index, RgEdgeType _type)
    {  num = (size_t(_type) + (_index << RGEDGE_NUM_LSHIFT));  }

    bool operator== (const rgedge_t &e) const { return num == e.num; }
    bool operator!= (const rgedge_t &e) const { return num != e.num; }
    bool operator< (const rgedge_t &e) const { return num < e.num; }

    inline size_t get_internal() const { return num; }
    static inline rgedge_t from_internal(size_t n) { rgedge_t r; r.num = n; return r; }

protected:
    size_t  num;
};

// Index of an invalid edge
const rgedge_t INVALID_RGEDGE_INDEX = rgedge_t(0, INVALID_EDGE_TYPE);


//-----------------------------------------------------------------------------

// A reachable state in the state space. This is a temporary data structure,
// since the RG structure stores a packed version of it. The packed version
// does not contain the index and the type.
struct RgState {
    rgstate_t			  selfIndex;	// Index of this state
    SparseMarking		  marking;		// Token assignment
    set<transition_t>     enabledGen;   // Gen transitions with concession
    rgedge_t 			  firstEdge;    // First outgoing edge in the linked list
};

//-----------------------------------------------------------------------------

extern template struct sparse_seq_store<SparseMarking>;
extern template class table < col::uint_col < transition_set_id, +1 >>;
extern template class table < col::uint_col < size_t, +1 >>;

// Storage of the state markings
typedef sparse_seq_store<SparseMarking>::sequence_id  packed_marking_id;

// The state set (one for tangible states, another for vanishings)
struct StateSet {
    // Reachability set markings (in packed format)
    sparse_seq_store<SparseMarking>			     packedMarkings;
    // Set of transitions in the rg.trnSetTbl table
    table < col::uint_col < transition_set_id, +1 >>  enabledGenSets;
    //  First outgoing edge in the linked list of edges
    table < col::uint_col < size_t, +1 >>             firstEdge;

    inline size_t size() const { return firstEdge.size(); }

    inline rgedge_t get_first_edge(size_t stateInd) const
    { return rgedge_t::from_internal(firstEdge.at(stateInd)); }

    inline void set_first_edge(size_t stateInd, rgedge_t edge)
    { firstEdge.set(stateInd, edge.get_internal()); }

    inline transition_set_id get_enabled_gen_set(size_t stateInd) const
    { return enabledGenSets.at(stateInd); }

    void clear(size_t numPlaces);
};

//-----------------------------------------------------------------------------

// Element of a sequence of immediate transition firings
struct VanishPathElem {
    transition_t		immFiredTrn;	// Immediate transition index
    double				prob;			// Probability of firedImmTrn

    inline VanishPathElem() { }
    inline VanishPathElem(transition_t _immFiredTrn, double _prob)
        : immFiredTrn(_immFiredTrn), prob(_prob) {}

    bool operator==(const VanishPathElem &vpe) const
    { return immFiredTrn == vpe.immFiredTrn && prob == vpe.prob; }
    bool operator!=(const VanishPathElem &vpe) const { return !(*this == vpe); }
};

// Hash function for VanishPathElem's
namespace std {
template<> struct hash<VanishPathElem> {
    size_t operator()(const VanishPathElem &vpe) const {
        return std::hash<transition_t>()(vpe.immFiredTrn) +
               std::hash<double>()(vpe.prob);
    }
};
};

// template<>
// struct sequence_traits<std::vector<VanishPathElem>> {
// 	typedef std::vector<VanishPathElem>        				 vector_type;
// 	typedef VanishPathElem  		 						 value_type;
// 	typedef table<col::uint_col<size_t>, col::indexed_col<double>>  table_type;
// 	typedef std::tuple<size_t, double>  	     			  tuple_type;

// 	inline value_type empty_elem() const { return VanishPathElem(); }
// 	inline tuple_type to_tuple(const value_type arg) const {
// 		return make_tuple(size_t(arg.immFiredTrn), arg.prob);
// 	}
// 	inline value_type from_tuple(tuple_type tuple) const{
// 		return VanishPathElem(std::get<0>(tuple), std::get<1>(tuple));
// 	}
// };

// // Declare the specialized template versions of the compact storages
// extern template struct seq_store<std::vector<VanishPathElem>>;

// typedef seq_store<std::vector<VanishPathElem>>  vanish_path_table;
// typedef vanish_path_table::sequence_id    		vanish_path_id;
// #define EMPTY_VANISHING_PATH   				 	vanish_path_id::INVALID_SEQ

// extern template struct seq_store<std::set<vanish_path_id>>;

// typedef seq_store<std::set<vanish_path_id>>     vanish_path_set_table;
// typedef vanish_path_set_table::sequence_id      vanish_path_set_id;
// #define EMPTY_VANISHING_PATH_SET			    vanish_path_set_id::INVALID_SEQ


// Storage table for vector<VanishPathElem> sequences
typedef suffix_table<VanishPathElem, vanish_path_id>  	  vanish_path_table;
#define EMPTY_VANISHING_PATH    	vanish_path_table::END_SET

// Storage for set<vanish_path_id> elements
typedef suffix_table<vanish_path_id, vanish_path_set_id>  vanish_path_set_table;
#define EMPTY_VANISHING_PATH_SET    vanish_path_set_table::END_SET

//-----------------------------------------------------------------------------

extern template class table<col::int_col<ssize_t>>;
extern template class table<col::uint_col<transition_t>>;
extern template class table < col::uint_col < transition_set_id, +1 >>;
extern template class table < col::uint_col < size_t, +1 >>;
extern template class table<col::indexed_col<double>>;
extern template class table < col::uint_col < vanish_path_set_id, +1 >>;

// Base class for the set of edges
struct BaseEdgeSet {
    // Directed edge destination
    table<col::int_col<ssize_t>>    			destState;
    // Fired transition index
    table<col::uint_col<transition_t>> 			firedTrn;
    // Gen preemption set ID of this edge
    table < col::uint_col < transition_set_id, +1 >> preempted;
    // Next edge in the linked list
    table < col::uint_col < size_t, +1 >> 			nextEdge;

    inline size_t size() const { return destState.size(); }

    inline rgstate_t get_dest_state(size_t edgeInd) const
    { return rgstate_t::from_internal(destState.at(edgeInd)); }

    inline transition_t get_fired_trn(size_t edgeInd) const { return firedTrn.at(edgeInd); }
    inline transition_set_id get_preempted(size_t edgeInd) const { return preempted.at(edgeInd); }

    inline rgedge_t get_next_edge(size_t edgeInd) const
    { return rgedge_t::from_internal(nextEdge.at(edgeInd)); }
    inline void set_next_edge(size_t edgeInd, rgedge_t edge)
    { nextEdge.set(edgeInd, edge.get_internal()); }

    void clear();
    size_t num_bytes() const;
};



// Edges generated from the firing of timed transitions
struct TimedEdgeSet : BaseEdgeSet {
    // Delay of (timed) fired transition
    table<col::indexed_col<double>>    delay;

    inline double get_delay(size_t edgeInd) const { return delay.at(edgeInd); }
    void clear();
    size_t num_bytes() const;
};

// Edges generated from the firing of immediate transitions
struct ImmediateEdgeSet : BaseEdgeSet {
    // Edge branching probability
    table<col::indexed_col<double>>    prob;

    inline double get_prob(size_t edgeInd) const { return prob.at(edgeInd); }
    void clear();
    size_t num_bytes() const;
};

// Reduced edges that collapse to single timed edges plus a set
// of vanishing paths (which are sequences of immediate transitions)
struct TimedReduxEdgeSet : TimedEdgeSet {
    // Total probability of all the vanishing paths
    table<col::indexed_col<double>>    totProb;
    // Set of vanishing paths
    table < col::uint_col < vanish_path_set_id, +1 >>  pathSet;

    inline double get_total_prob(size_t edgeInd) const { return totProb.at(edgeInd); }
    inline vanish_path_set_id get_vanish_path_set_id(size_t edgeInd) const
    { return pathSet.at(edgeInd); }
    void clear();
    size_t num_bytes() const;
};

//-----------------------------------------------------------------------------

// An edge in the RG, in unpacked format
struct RgEdge {
    rgedge_t			selfIndex;
    RgEdgeType			type;
    rgstate_t			dest;		// Directed edge destination
    transition_t		firedTrn;	// Fired transition index
    transition_set_id	preempted;	// Gen preemption set ID of this edge
    rgedge_t 			nextEdge;   // Next edge in the linked list
    double				prob;		// Edge branching probability
    double				delay;		// Delay of (timed) fired transition
    vanish_path_set_id  pathSet;	// Set of vanishing paths


    inline bool isFromVanishingState() const
    {  return (type == VANISHING_TO_TANGIBLE || type == VANISHING_TO_VANISHING);  }

    inline bool goesToVanishingState() const
    {  return (type == TANGIBLE_TO_VANISHING || type == VANISHING_TO_VANISHING);  }

    inline bool hasDelay() const		{  return !isFromVanishingState();  }

    inline bool hasProb() const  		{  return isFromVanishingState();  }

    inline bool hasVanishPaths() const
    {  return (type == TANGIBLE_TO_TANGIBLE_WITH_VANISHINGS);  }

    inline RgStateType typeOfSrcState() const
    {  return isFromVanishingState() ? VANISHING : TANGIBLE;  }

    inline RgStateType typeOfDstState() const
    {  return goesToVanishingState() ? VANISHING : TANGIBLE;  }
};

RgEdge &GetEdgeByIndex(const RG2 &rg, rgedge_t edgeInd, RgEdge &edge);

//-----------------------------------------------------------------------------

struct InitialFrontierEntry {
    rgstate_t			state;		// Initial state
    double				initProb;	// Initial probability of entering in state
    vanish_path_set_id  pathSet;	// Set of vanishing paths

    inline InitialFrontierEntry(rgstate_t _state, double _initProb,
                                vanish_path_set_id _pathSet)
        : state(_state), initProb(_initProb), pathSet(_pathSet) { }
};

//-----------------------------------------------------------------------------

/// Reachability Graph
struct RG2 : boost::noncopyable {
    RG2();
    ~RG2();

    typedef vector<InitialFrontierEntry>  	 	initial_frontier_t;

    // Number of places(variables)
    size_t              	numPlaces;
    // Distributions of the transitions
    vector<TrnDistrib>		trnDistribs;
    // Initial state in unpacked format
    RgState          		initState;
    // Vector of the two State sets (Vanishing and Tangible, stored separately)
    StateSet 				stateSets[NUM_RG_STATE_TYPES];
    // Built keeping vanishing (Extended RG)
    bool					isReduced;
    // Vanishing path sets are stored in each reduced timed edge
    bool					hasVanishPaths;
    // Tangible frontier (initial tangible markings)
    initial_frontier_t		initStates;
    // Array of preprocessed Measures
    vector<PpMeasure>		ppMeasures;
    // Class of this reachability graph (CTMC, MRP, ...)
    RGClass					rgClass;
    // f_g(x) functions of general transitions
    map<transition_t, string> generalFn; 


    //------ Edge sets ------
    // Edge data are stored into these 5 arrays, and indexed
    // by the firstEdge/nextEdge linked list

    // Timed edge sets (T->T, T->V)
    TimedEdgeSet			timedEdgeSets[NUM_RG_STATE_TYPES];
    // Immediate edge sets (V->V, V->T)
    ImmediateEdgeSet		immedEdgeSets[NUM_RG_STATE_TYPES];
    // Reduced timed edge set (T->V*->T)
    TimedReduxEdgeSet		timedReduxEdgeSet;

    //------ Internal memory management of the RG structure ------

    // Storage for all the set<transition_t> used in the RG
    transition_set_table	trnSetTbl;
    // Storage for vanishing path sequences
    vanish_path_table		vanPathTbl;
    // Storage for sets of vanishing paths
    vanish_path_set_table	vanPathSetTbl;


    size_t NumEdges() const;

    size_t NumStates() const;
};

void UnpackState(const RG2 &rg, const rgstate_t stateInd, RgState &rs);

// Get, in sequence, all the tangible and vanishing states of the RG.
void GetIthState(const RG2 &rg, const size_t cnt, RgState &rs);

//-----------------------------------------------------------------------------
// Statistics of the RG building process
// TODO: some of these are not used
struct RG2BuildStat {
    size_t	numImmedFiredTrns;	// Number of immediate transitions fired
    size_t	numTimedFiredTrns;	// Number of timed transitions fired
    size_t	maxGenTrnsConEnabl; // Max gen. transitions Concurrently enabled

    size_t  numGenTrans;		// General transition firings
    size_t  numNonPreemtExp;	// Non-preemptive exponential transition firings
    size_t  numPreemtExp;		// Preemptive exponential transition firings

    size_t  numVanishingStates; // Number of vanishing states generated

    // Statistics only available for Tangible RG builds
    //size_t	maxVisitDepth;		// Max. Depth of the DFS visit procedure
    //size_t  maxArcVanishPaths;	// Max. number of vanishing path in an RG arcs
};

void BuildRG2(const PN &pn, const PETLT &petlt, RG2 &rg2, RG2BuildStat &stat,
              bool reduceImmediateTrns, bool storeVanishingPaths,
              bool eraseVanishingMarkings, VerboseLevel verboseLvl);

//=============================================================================

void LoadMRMC_Model(ifstream &ifsTra, ifstream &ifsLab,
                    PN &pn, RG2 &rg, VerboseLevel verboseLvl);

void LoadPrismModel(ifstream &ifsTra, ifstream &ifsLab, ifstream &ifsSta,
                    PN &pn, RG2 &rg, VerboseLevel verboseLvl);

//=============================================================================
//    Print clojures
//=============================================================================

std::function<ostream& (ostream &)>
print_rgstate(const rgstate_t s);

std::function<ostream& (ostream &)>
print_reachable_state(const PN &pn, const RgState &rs);

std::function<ostream& (ostream &)>
print_transition_set(const PN &pn, const RG2 &rg, transition_set_id tsid);

std::function<ostream& (ostream &)>
print_vanish_path_set(const PN &pn, const RG2 &rg, const vanish_path_set_id vpsid);

std::function<ostream& (ostream &)>
print_rgedge(const PN &pn, const RG2 &rg, const rgstate_t fromSt, const RgEdge &e);

std::function<ostream& (ostream &)>
print_outgoing_edges(const PN &pn, const RG2 &rg, const rgstate_t fromSt);

//=============================================================================
//    Inline method/function implementations
//=============================================================================

// // Returns the packed_rgstate* of a given rgstate_t index
// inline const StateSetEntry&
// GetStateSetEntryByIndex(const RG2& rg, const rgstate_t index) {
// 	assert(index != INVALID_RGSTATE_INDEX);
// 	return rg.stateSets[index.type()][index.index()];
// }

//=============================================================================

// inline StateSetEntry&
// GetStateSetEntryByIndex(RG2& rg, const rgstate_t index) {
// 	assert(index != INVALID_RGSTATE_INDEX);
// 	return rg.stateSets[index.type()][index.index()];
// }

//=============================================================================
#endif   // __NEW_REACHABILITY_GRAPH_H__
