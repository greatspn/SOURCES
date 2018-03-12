#ifndef MDP_minimize_incl
#define MDP_minimize_incl

/* MARKOV DECISION PROCESS MINIMIZER */
/*
  Copyright notice: This is a test version. Please do not distribute nor trust
  too much on this. I hope to deliver a public version (perhaps GNU license)
  in the future. Antti Valmari 17.9.2010
*/
/*
  This class features functionality for minimizing a Markov decision process
  (MDP) and querying about the result. Minimization runs in O((n+m) log n)
  time and rather small memory. It is essentially the bisimulation
  minimization algorithm of [1] augmented with a Markov splitting step copied
  from [2]. Because it is unlikely that two MDPs must be minimized
  simultaneously, and because member functions may be slower than static
  functions, the class is static.

  [1] Valmari, A: Simple Bisimilarity Minimization in O(m log n) Time.
      Fundamenta Informaticae, to appear (accepted in 2010).
  [2] Valmari, A. & Franceschinis, G.: Simple O(m log n) Time Markov Chain
      Lumping. TACAS 2010, LNCS 6015, 38--52.


  Markov Decision Processes in this class
  ---------------------------------------

  The MDPs are slightly generalized. First, while in a pure MDP a transition
  consists of a labelled transition leading from a "real" state to an
  auxiliary state from which there is a set of alternative weighed transitions
  (a probability distribution) to "real" states, in this class each state may
  freely have both labelled and weighed output transitions, and there is no
  distinction between "real" and "auxiliary" states. Second, weights of the
  output weighed transitions of a state need not be positive numbers and they
  need not add up to one.

  So, for this class an MDP is a tuple (S, Sigma, lDelta, wDelta, I), where S
  is the set of states, Sigma is the set of labels, lDelta is a subset of
  S times Sigma times S, wDelta is a partial function from S times S to some
  numeric type called MDP_weight_type, and I is the initial partition of S.
  (The class probably works correctly even if lDelta and wDelta are multisets,
  but this has neither been checked nor tested. In that case the complexity is
  O((n+m) log (n+m)).) To describe the output, let W(s1,s2) = wDelta(s1,s2)
  when the latter is defined and W(s1,s2) = 0 otherwise.

  The class computes the MDP (S', Sigma, lDelta', wDelta', I'), where S' is
  the coarsest refinement of I that is compatible with lDelta and W. Sets of
  S' are called blocks. They are the states of the result. (B1,a,B2) is in
  lDelta' if and only if any (equivalently, every) s1 in B1 has (s1,a,s2) in
  lDelta for some s2 in B2. W'(B1,B2) is the sum of W(s1,s2) for some s1 in B1
  and over every s2 in B2. wDelta' is the restriction of W' to those (B1,B2)
  that have W'(B1,B2) /= 0. I' is the partition of S' such that B is in block
  number b of I' if and only if B is a subset of block number b of I.

  In the class and its user interface, states, labels, and blocks are numbered
  starting from 1. In the class, labelled transitions and weighed transitions
  are numbered starting from 0, but the user does not see this.


  The type of the weights of weighed transitions
  ----------------------------------------------

  MDP_weight_type must have been defined elsewhere. It may be int, double, or
  something else. It must feature (at least) the usual operations =, +=, <,
  and ==. There must also be the following function and (constant) values:

    MDP_weight_type MDP_zero_weight

  This is just a funny name for the value zero. This convention was chosen
  instead of using the literal 0, because it avoids the need to define a type
  conversion function if the user defines an own MDP_weight_type.

    MDP_weight_type MDP_unused_weight

  This is a value that does not arise as a sum of transition weights. It is
  used to mark that an array slot is uninitialized.

    void MDP_round_weight( MDP_weight_type & )

  To avoid problems caused by imprecise arithmetic (like float and double),
  this function is called to convert the result of an addition of weights to a
  "nearby standard value". This approach is better than replacing == by an
  approximate comparison, because an approximate == is not transitive, which
  is quite worrying for an algorithm based on equivalence classes.

  Please see MDPmin.cc for how to define MDP_weight_type as double or int.


  Getting the MDP, minimizing, and delivering the result
  ------------------------------------------------------

  Please see MDPmin.cc for a discussion of the intended architecture where
  this class is used.

  For inputting the MDP, this class implements the following functions that
  are described in detail in MDP_read.cc under the names MDP_store_sizes, etc.
  store_sizes must be called first, but otherwise the ordering does not
  matter:

    store_sizes
    store_l_transition
    store_w_transition
    store_block

  Then the MDP is minimized by calling the following function:

    void minimize()

  For outputting the MDP, this class implements the following functions. The
  user may freely choose which of them and in which order are called, except
  that terminate must be called last or not at all:

  These functions are described in detail in MDP_write.cc:

    give_sizes
    give_l_transition
    give_w_transition
    give_block_first
    give_block_next

  The following function returns the number of the final block the state
  belongs to:

    unsigned which_block( unsigned state )

  By calling the following function, the user can indicate that will not
  request any more information on the result. This function releases the
  dynamic memory used by this class and returns the class to the initial
  state, so that the next MDP may be inputted and minimized.

    void terminate()

*/


#include "err.cc"


// #define no_debug     // better not switch this on


class MDPmin{


/* SIMPLE INDEX-CHECKING ARRAY CLASS, INDEXED 0...max */
/* Before use, the array must be made active with make_size( max ). Only
  those operations have been worried of that MDPmin needs. E.g., making and
  removing a copy (which happens with value parameters) causes a disaster by
  deleting the dynamic memory of the original. */
template< typename el_type > class Array{
  unsigned max1, free;
  el_type *elems;
  #ifndef no_debug
    inline void index_check( unsigned idx )const{
      if( idx >= max1 ){
        err::msg( "Illegal array index" ); err::msg( idx );
        err::msf( ", max index is" ); err::msg( max1 - 1 ); err::ready();
      }
    }
  #else
    inline void index_check( unsigned )const{}
  #endif
public:
  Array(): max1( 0 ), free( 0 ), elems( 0 ){}
  ~Array(){ del(); }
  inline void del(){ delete [] elems; elems = 0; max1 = 0; free = 0; }
  inline void make_size( unsigned ms ){
    if( free ){ err::msg( "Changing the size of an array" ); err::ready(); }
    max1 = ms + 1; elems = new el_type[ max1 ];
  }
  inline bool active()const{ return elems; }
  inline el_type &operator[]( unsigned idx ){
    index_check( idx ); return elems[ idx ];
  }
  inline const el_type &operator[]( unsigned idx )const{
    index_check( idx ); return elems[ idx ];
  }
  inline unsigned stack_size()const{ return free; }
  inline void add( el_type el ){
    #ifndef no_debug
      if( free >= max1 ){
        err::msg( "Adding to a full stack of size" ); err::msg( max1 - 1 );
        err::ready();
      }
    #endif
    elems[ free ] = el; ++free;
  }
  inline el_type remove(){
    #ifndef no_debug
      if( !free ){
        err::msg( "Removing from an empty stack" ); err::ready();
      }
    #endif
    --free; return elems[ free ];
  }
  inline void clear(){ free = 0; }
};
typedef Array< unsigned > uArray;
typedef Array< MDP_weight_type > wArray;


/* SORTING elems[ beg ... end-1 ] */
/* Uses key[i] as the key of element i. Elements must be (small) numbers. */

template< typename kArray > inline static void fix_heap(
  uArray & elems, unsigned beg, unsigned parent, unsigned end,
  const kArray & key
){
  unsigned child = 2*parent - beg + 1, tmp_el = elems[ parent ];
  while( child < end ){
    if(
      child + 1 < end && key[ elems[ child ] ] < key[ elems[ child + 1 ] ]
    ){ ++child; }
    if( !( key[ tmp_el ] < key[ elems[ child ] ] ) ){ break; }
    elems[ parent ] = elems[ child ]; parent = child;
    child = 2*parent - beg + 1;
  }
  elems[ parent ] = tmp_el;
}

template< typename kArray > static void heapsort(
  uArray & elems, unsigned beg, unsigned end, const kArray & key
){
  if( !end ){ return; }     // to avoid --i1 yielding a negative value

  /* Make the array a heap. */
  /* Runs from (end + beg) / 2 - 1 to beg. end - 1 is a child of the first. */
  for( unsigned i1 = (end + beg) / 2; i1 > beg; ){
    --i1; fix_heap( elems, beg, i1, end, key );
  }

  /* Sort the heap to a sorted array. Runs from end - 1 to beg + 1. */
  for( unsigned i1 = end; --i1 > beg; ){
    unsigned tmp_el = elems[ beg ];
    elems[ beg ] = elems[ i1 ]; elems[ i1 ] = tmp_el;
    fix_heap( elems, beg, beg, i1, key );
  }

}


/* REFINABLE PARTITIONS, THAT IS, BLOCKS AND CORDS */
/* This is a struct, because others frequently access its data. */

struct refinable_partition{

  /* Data structures, indexed from one */
  unsigned nr_sets;
  uArray elems, locs, sets, begins, ends, mids;

  /* Constructor */
  refinable_partition(): nr_sets( 0 ){}

  /* Mark a state in its set. */
  inline unsigned mark( unsigned el ){
    unsigned ss = sets[ el ], lc = locs[ el ], m_lc = mids[ ss ];
    mids[ ss ] = m_lc + 1;
    elems[ lc ] = elems[ m_lc ]; locs[ elems[ lc ] ] = lc;
    elems[ m_lc ] = el; locs[ el ] = m_lc;
    if( m_lc == begins[ ss ] ){ return ss; }else{ return 0; }
  }

  /* Splits a set to at most two parts. The (actually, a) bigger part inherits
    the original set number. Returns 0, if the unmarked part vanished.
    Otherwise returns the number of the unmarked part. */
  unsigned split( unsigned ss ){
    if( mids[ ss ] == ends[ ss ] ){ mids[ ss ] = begins[ ss ]; }
    if( mids[ ss ] == begins[ ss ] ){ return 0; }
    ++nr_sets;
    if( mids[ ss ] - begins[ ss ] <= ends[ ss ] - mids[ ss ] ){
      begins[ nr_sets ] = begins[ ss ];
      ends[ nr_sets ] = begins[ ss ] = mids[ ss ];
    }else{
      ends[ nr_sets ] = ends[ ss ];
      begins[ nr_sets ] = ends[ ss ] = mids[ ss ];
      mids[ ss ] = begins[ ss ]; ss = nr_sets;
    }
    mids[ nr_sets ] = begins[ nr_sets ];
    for( unsigned lc = begins[ nr_sets ]; lc < ends[ nr_sets ]; ++lc ){
      sets[ elems[ lc ] ] = nr_sets;
    }
    return ss;
  }

  /* Splits a set according to the contents of ww. */
  inline void multisplit( unsigned ss, const wArray & ww ){
    unsigned bg_s = begins[ ss ], en_s = ends[ ss ];
    heapsort( elems, bg_s, en_s, ww );
    for( unsigned lc = bg_s; lc < en_s; ++lc ){ locs[ elems[ lc ] ] = lc; }
    MDP_weight_type value = ww[ elems[ bg_s ] ];
    for( unsigned lc = bg_s + 1; lc < en_s; ++lc ){
      if( !( ww[ elems[ lc ] ] == value ) ){
        mids[ ss ] = lc; ss = split( ss ); value = ww[ elems[ lc ] ];
      }
    }
  }

};

static refinable_partition
  B,    // blocks; a block is a set in the current partition of states
  T;    // cords; they constitute a partition of labelled transitions


/* DATA STRUCTURES FOR THE MDP */

/* Sizes, all are initially 0 */
static unsigned nr_states, nr_labels, nr_l_trans, nr_w_trans, nr_init_blocks;

/* Maximum number of labelled transitions is reduced, see bisim_split(). */
static const unsigned nr_l_max1;    // = ~0u/2 + 1; maximum nr_l_trans + 1

/* Labelled transitions, indexed from one */
static uArray l_tails, l_labels, l_heads;

/* Weighed transitions, indexed from one */
static uArray w_tails, w_heads; static wArray w_weights;

/* Input and output transition data structures */
/* These store the labelled input transitions, weighed input transitions, and
  weighed output transitions of states. The latter two are not needed
  simultaneously, so they are made to share memory. The transitions of state
  st are _trans[ _end[ st - 1 ] ] ... _trans[ _end[ st ] - 1 ]. */
static uArray in_l_trans, in_l_ends, adj_w_trans, adj_w_ends;


/* INITIALIZATION SUBROUTINES */

/* This subroutine makes adj_end and adj_trans contain the adjacent (e.g.,
  input) transitions of states. adj_end[ st ] will contain (1 + the location
  of the last adjacent transition of state st in adj_trans), which is the
  same as the location of the first adjacent transition of state st+1.
  adj_end[ 0 ] will contain the location of the first adjacent transition of
  state 1. */
static void make_adjacent_tr_set(
  unsigned nr_trans, const uArray & tr_states, uArray & adj_trans,
  uArray & adj_ends
){

  /* Count the number of adjacent transitions of each state. */
  for( unsigned st = 0; st <= nr_states; ++st ){ adj_ends[ st ] = 0; }
  for( unsigned tr = 1; tr <= nr_trans; ++tr ){
    ++adj_ends[ tr_states[ tr ] ];
  }

  /* Transform the counts to the locations of first transitions. */
  unsigned sum = 0, tmp;
  for( unsigned st = 0; st <= nr_states; ++st ){
    tmp = adj_ends[ st ]; adj_ends[ st ] = sum; sum += tmp;
  }

  /* Put the transitions to their places. */
  for( unsigned tr = 1; tr <= nr_trans; ++tr ){
    unsigned st = tr_states[ tr ];
    adj_trans[ adj_ends[ st ] ] = tr; ++adj_ends[ st ];
  }

}


/* Sorts or, to be precise, groups labelled transitions primarily according to
  labels and secondarily according to tail states. Updates (only) T.nr_sets,
  T.elems and T.ends. */
static void group_l_transitions(){
  if( !nr_l_trans ){ return; }  // needed because of T.ends[ 1 ] below

  /* Choose memory-saving or time-saving algorithm. If memory needed by uArray
    index is bigger than an estimate of memory reserved after index is
    released, save memory. */
  if(
    // false    // always time-saving
    // true     // always memory-saving
    nr_labels > 11*nr_states + 7*nr_l_trans + nr_w_trans
  ){

    for( unsigned tr = 1; tr <= nr_l_trans; ++tr ){ T.elems[ tr ] = tr; }
    heapsort( T.elems, 1, nr_l_trans + 1, l_labels );
    T.nr_sets = 1; unsigned lb = l_labels[ T.elems[ 1 ] ];
    for( unsigned tr = 2; tr <= nr_l_trans; ++tr ){
      if( l_labels[ T.elems[ tr ] ] != lb ){
        T.ends[ T.nr_sets ] = tr; ++T.nr_sets; lb = l_labels[ T.elems[ tr ] ];
      }
    }
    T.ends[ T.nr_sets ] = nr_l_trans + 1;

  }else{

    /* Workset for mapping from labels to their new indices */
    uArray index; index.make_size( nr_labels );

    /* Count how many times each label occurs in labelled transitions. The
      index of the label is id. It is assigned when the label is first found.
      T.elems[ id ] is the counter and T.ends[ id ] maps back to the label. */
    T.nr_sets = 0;    // counts the number of different labels that occur
    for( unsigned tr = 1; tr <= nr_l_trans; ++tr ){
      unsigned lb = l_labels[ tr ], id = index[ lb ];
      if( id < 1 || id > T.nr_sets || T.ends[ id ] != lb ){
        id = ++T.nr_sets;
        T.ends[ id ] = lb; index[ lb ] = id; T.elems[ id ] = 1;
      }else{ ++T.elems[ id ]; }
    }

    /* Convert the counts to start locations. */
    T.ends[ 1 ] = 1;
    for( unsigned id = 1; id < T.nr_sets; ++id ){
      T.ends[ id + 1 ] = T.ends[ id ] + T.elems[ id ];
    }

    /* Put the transitions into their places. */
    for( unsigned tr = 1; tr <= nr_l_trans; ++tr ){
      unsigned id = index[ l_labels[ tr ] ];
      T.elems[ T.ends[ id ] ] = tr; ++T.ends[ id ];
    }

    /* The potentially big index array is not needed any more. */
    index.del();

  }

  /* Sort each transition set according to the tail states. */
  T.ends[ 0 ] = 1;
  for( unsigned id = 1; id <= T.nr_sets; ++id ){
    heapsort( T.elems, T.ends[ id - 1 ], T.ends[ id ], l_tails );
  }

}


/* Splitting according to the initial partition */
static void split_according_initial_partition(){
  for( unsigned st = 1; st <= nr_states; ++st ){ B.elems[ st ] = st; }
  heapsort( B.elems, 1, nr_states + 1, B.sets );
  for( unsigned lc = 1; lc <= nr_states; ++lc ){
    B.locs[ B.elems[ lc ] ] = lc;
  }
  B.begins[ 1 ] = B.mids[ 1 ] = B.nr_sets = 1;
  unsigned bl1 = B.sets[ B.elems[ 1 ] ];
  for( unsigned lc = 2; lc <= nr_states; ++lc ){
    unsigned bl2 = B.sets[ B.elems[ lc ] ];
    if( bl1 != bl2 ){
      B.ends[ B.nr_sets ] = lc;
      ++B.nr_sets;
      B.begins[ B.nr_sets ] = B.mids[ B.nr_sets ] = lc;
      bl1 = bl2;
    }
  }
  B.ends[ B.nr_sets ] = nr_states + 1;
}


/* MAIN SPLITTING LOOPS WITH THEIR IMMEDIATE HELPERS */

/* Worksets that keep track of touched entities */
static uArray touched_states, touched_sets;

/* Making the cords match blocks */
static unsigned cord_splitter;  // = 2
inline static void update_cords(){
  for( ; cord_splitter <= B.nr_sets; ++cord_splitter ){
    for(
      unsigned lc = B.begins[ cord_splitter ]; lc < B.ends[ cord_splitter ];
      ++lc
    ){
      unsigned st = B.elems[ lc ];
      for( unsigned pl = in_l_ends[ st - 1 ]; pl < in_l_ends[ st ]; ++pl ){
        unsigned cd = T.mark( in_l_trans[ pl ] );
        if( cd ){ touched_sets.add( cd ); }
      }
    }
    while( touched_sets.stack_size() ){ T.split( touched_sets.remove() ); }
  }
}


/* Bisimulation splitting */
static unsigned nr_counters;    // = 0
static uArray links, t_counts, s_counts;// counters, and links to tr counters
static unsigned bisim_splitter;     // = 1; number of next pending splitter
static void bisim_split(){

  /* Split as long as there are pending splitters. */
  for( ; bisim_splitter <= T.nr_sets; ++bisim_splitter ){

    /* Count transitions from states in current cord. */
    for(
      unsigned lc = T.begins[ bisim_splitter ]; lc < T.ends[ bisim_splitter ];
      ++lc
    ){ ++s_counts[ l_tails[ T.elems[ lc ] ] ]; }

    /* Mark states in the left block for splitting. */
    for(
      unsigned lc = T.begins[ bisim_splitter ]; lc < T.ends[ bisim_splitter ];
      ++lc
    ){
      unsigned tr = T.elems[ lc ], st = l_tails[ tr ], lntr = links[ tr ];
      if( s_counts[ st ] == t_counts[ lntr ] ){
        unsigned bl = B.mark( st );
        if( bl ){ touched_sets.add( bl ); }
        s_counts[ st ] = 0;
      }
    }

    /* Split encountered blocks. */
    while( touched_sets.stack_size() ){ B.split( touched_sets.remove() ); }

    /* Mark states in the middle block for splitting, creating new transition
      counters and updating links to them as necessary. When a new counter has
      been introduced, its index is stored in s_counts[ st ] so that it can be
      copied to all transitions that need it. The most significant bit of
      s_counts[ st ] tells which type of information it contains. To avoid
      confusion, the number of labelled transitions is restricted to maximum
      int instead of maximum unsigned. This restriction was considered a
      smaller problem than the introduction of one more array. */
    for(
      unsigned lc = T.begins[ bisim_splitter ]; lc < T.ends[ bisim_splitter ];
      ++lc
    ){
      unsigned tr = T.elems[ lc ], st = l_tails[ tr ], lntr = links[ tr ];
      if( s_counts[ st ] >= nr_l_max1 ){

        /* Big state counter contains the link to a new tr counter. */
        links[ tr ] = s_counts[ st ] - nr_l_max1;

      }else if( s_counts[ st ] > 0 ){

        /* Current state has transitions both in the current cord and
          elsewhere in the current rope. The transition counter has to be
          split to these parts. */
        unsigned bl = B.mark( st );
        if( bl ){ touched_sets.add( bl ); }
        ++nr_counters;
        t_counts[ nr_counters ] = s_counts[ st ];
        t_counts[ lntr ] -= s_counts[ st ];
        s_counts[ st ] = nr_counters + nr_l_max1; links[ tr ] = nr_counters;

      }
    }

    /* Reset state counters. */
    for(
      unsigned lc = T.begins[ bisim_splitter ]; lc < T.ends[ bisim_splitter ];
      ++lc
    ){ s_counts[ l_tails[ T.elems[ lc ] ] ] = 0; }

    /* Split encountered blocks. */
    while( touched_sets.stack_size() ){ B.split( touched_sets.remove() ); }
    update_cords();

  }
}


/* Markov chain splitting */
static wArray ww;       // state output weight sums, will be needed also later
static unsigned markov_splitter;    // = 1; number of next pending splitter
static void markov_split(){

  /* Split as long as there are pending splitters. */
  for( ; markov_splitter <= B.nr_sets; ++markov_splitter ){

    /* Scan the input transitions of a block and process their tail states. */
    for(
      unsigned lc = B.begins[ markov_splitter ];
      lc < B.ends[ markov_splitter ]; ++lc
    ){
      unsigned st2 = B.elems[ lc ];
      for(
        unsigned pl = adj_w_ends[ st2 - 1 ]; pl < adj_w_ends[ st2 ]; ++pl
      ){
        unsigned tr = adj_w_trans[ pl ], st1 = w_tails[ tr ];
        if( ww[ st1 ] == MDP_unused_weight ){
          touched_states.add( st1 ); ww[ st1 ] = w_weights[ tr ];
        }else{ ww[ st1 ] += w_weights[ tr ]; }
      }
    }

    /* Scan the previously met states and mark them for splitting. */
    for( unsigned i1 = 0; i1 < touched_states.stack_size(); ++i1 ){
      unsigned st = touched_states[ i1 ];
      MDP_round_weight( ww[ st ] );     // eliminate effect of imprecise sums
      if( ww[ st ] != MDP_zero_weight ){
        unsigned bl = B.mark( st );
        if( bl ){ touched_sets.add( bl ); }
      }
    }

    /* Scan the previously met blocks and split them. */
    while( touched_sets.stack_size() ){

      /* Split to touched and untouched states. */
      unsigned block1 = touched_sets.remove();
      if( block1 == B.split( block1 ) ){ block1 = B.nr_sets; }

      /* Find the possible majority candidate. */
      unsigned count = 0; MDP_weight_type pmc = MDP_zero_weight;
      for( unsigned lc = B.begins[ block1 ]; lc < B.ends[ block1 ]; ++lc ){
        unsigned st = B.elems[ lc ];
        if( !count ){ pmc = ww[ st ]; count = 1; }
        else if( pmc == ww[ st ] ){ ++count; }
        else{ --count; }
      }

      /* Split the block according to the possible majority candidate. */
      for( unsigned lc = B.begins[ block1 ]; lc < B.ends[ block1 ]; ++lc ){
        unsigned st = B.elems[ lc ];
        if( ww[ st ] == pmc ){ B.mark( st ); }
      }
      block1 = B.split( block1 );

      /* Split block1 as needed by ww. */
      if( block1 ){ B.multisplit( block1, ww ); }

    }

    /* Set the used ww slots to unused and make touched_states empty. */
    for( unsigned i1 = 0; i1 < touched_states.stack_size(); ++i1 ){
      ww[ touched_states[ i1 ] ] = MDP_unused_weight;
    }
    touched_states.clear();

  }
  update_cords();

}


/* THE MAIN FUNCTION */

/* These are to check that the services are not called in a wrong order. */
enum phase_type { initial, sizes_given, minimized };
static phase_type phase;    // = initial

static unsigned
  l_cnt, w_cnt,         // = 0; numbers of given transitions
  nr_l_out, nr_w_out;   // = 0; will be numbers of result transitions

/* Locations in B.elems where initial blocks change */
static uArray init_block_borders;


public:


static void minimize(){
  err err_context( "MDP minimizer" );
  if( phase != sizes_given ){ err::msg( "The MDP must be given first" ); }

  /* Check that all input data has been given. */
  if( l_cnt < nr_l_trans ){
    err::msg( "too few l-transitions given:" ); err::msg( l_cnt );
  }
  if( w_cnt < nr_w_trans ){
    err::msg( "too few w-transitions given:" ); err::msg( w_cnt );
  }
  err::ready();

  /* Terminate, if trivial MDP. Needed because the rest trusts nr_states > 0.
   Output variables have automatically the right values. */
  if( !nr_states ){ phase = minimized; return; }

  /* These data structures are normally created when the l-transitions are
    given. If there are no l-transitions, they are created separately here. */
  if( !nr_l_trans ){
    l_tails.make_size( nr_l_trans ); l_labels.make_size( nr_l_trans );
    l_heads.make_size( nr_l_trans );
    T.elems.make_size( nr_l_trans ); T.ends.make_size( nr_l_trans );
  }

  /* Create and initialize T.sets. */
  T.sets.make_size( nr_l_trans ); unsigned id = 1;
  for( unsigned lc = 1; lc <= nr_l_trans; ++lc ){
    if( lc >= T.ends[ id ] ){ ++id; }
    T.sets[ T.elems[ lc ] ] = id;
  }

  /* Create and initialize T.locs. */
  T.locs.make_size( nr_l_trans );
  for( unsigned lc = 1; lc <= nr_l_trans; ++lc ){
    T.locs[ T.elems[ lc ] ] = lc;
  }

  /* Create and initialize the remaining arrays of cords. */
  T.begins.make_size( nr_l_trans ); T.mids.make_size( nr_l_trans );
  if( T.nr_sets ){ T.begins[ 1 ] = T.mids[ 1 ] = 1; }
  for( unsigned id = 2; id <= T.nr_sets; ++id ){
    T.begins[ id ] = T.mids[ id ] = T.ends[ id - 1 ];
  }

  /* Create worksets. */
  touched_sets.make_size(
    ( nr_states > nr_l_trans ? nr_states : nr_l_trans ) - 1
  );
  touched_states.make_size( nr_states - 1 );

  /* Create and initialize counters, and links that point to them. */
  links.make_size( nr_l_trans );
  t_counts.make_size( nr_l_trans );
  s_counts.make_size( nr_states );
  unsigned prev_tail = 0, prev_label = 0;
  for( unsigned lc = 1; lc <= nr_l_trans; ++lc ){
    unsigned tr = T.elems[ lc ];
    if( l_tails[ tr ] != prev_tail || l_labels[ tr ] != prev_label ){
      prev_tail = l_tails[ tr ]; prev_label = l_labels[ tr ];
      ++nr_counters; t_counts[ nr_counters ] = 0;
    }
    links[ tr ] = nr_counters; ++t_counts[ nr_counters ];
  }
  for( unsigned st = 1; st <= nr_states; ++st ){ s_counts[ st ] = 0; }

  /* Create and initialize input labelled transition sets. */
  in_l_trans.make_size( nr_l_trans - 1 ); in_l_ends.make_size( nr_states );
  make_adjacent_tr_set( nr_l_trans, l_heads, in_l_trans, in_l_ends );

  /* This set may have been created when block information was given. */
  if( !B.sets.active() ){
    B.sets.make_size( nr_states );
    for( unsigned st = 1; st <= nr_states; ++st ){ B.sets[ st ] = 1; }
  }

  /* Create and initialize the remaining block arrays. */
  B.elems.make_size( nr_states ); B.locs.make_size( nr_states );
  B.begins.make_size( nr_states ); B.ends.make_size( nr_states );
  B.mids.make_size( nr_states );

  /* Do the initial split and check the sanity of initial partition. */
  split_according_initial_partition();
  if( B.nr_sets != nr_init_blocks ){
    err::msg( "Some initial block is empty" ); err::ready();
  }

  /* These data structures are normally created when the w-transitions are
    given. If there are no w-transitions, they are created separately here. */
  if( !nr_w_trans ){
    w_tails.make_size( nr_w_trans ); w_weights.make_size( nr_w_trans );
    w_heads.make_size( nr_w_trans );
  }

  /* Create input weighed transition sets of states. */
  adj_w_trans.make_size( nr_w_trans - 1 ); adj_w_ends.make_size( nr_states );
  make_adjacent_tr_set( nr_w_trans, w_heads, adj_w_trans, adj_w_ends );

  /* Create ww and initialize it to all unused. */
  ww.make_size( nr_states );
  for( unsigned st = 1; st <= nr_states; ++st ){
    ww[ st ] = MDP_unused_weight;
  }

  /* Store the borders of initial blocks. */
  init_block_borders.make_size( nr_init_blocks );
  init_block_borders[ 0 ] = 1;
  for( unsigned bl = 1; bl <= nr_init_blocks; ++bl ){
    init_block_borders[ bl ] = B.ends[ bl ];
  }

  /* The main loop */
  while( markov_splitter <= B.nr_sets || bisim_splitter <= T.nr_sets ){
    markov_split();
    bisim_split();
  }

  /* Count the number of labelled transitions in the result. Works by finding
    first transitions from the first state of each block, and uses their links
    to ignore other transitions from the same state with the same label to the
    same block. */
  for( unsigned tr = 1; tr <= nr_l_trans; ++tr ){
    unsigned st = l_tails[ tr ];
    if( st != B.elems[ B.begins[ B.sets[ st ] ] ] ){ continue; }
    if( !t_counts[ links[ tr ] ] ){ continue; }
    t_counts[ links[ tr ] ] = 0;
    ++nr_l_out;
  }

  /* Create output weighed transition sets of states. Reuse the memory for
    input weighed transitions. */
  make_adjacent_tr_set( nr_w_trans, w_tails, adj_w_trans, adj_w_ends );

  /* Reset B.mids for another use. */
  for( unsigned bl = 1; bl <= B.nr_sets; ++bl ){ B.mids[ bl ] = 0; }

  /* Compute the number of weighed transitions in the result. */
  for( unsigned bl1 = 1; bl1 <= B.nr_sets; ++bl1 ){
    unsigned st = B.elems[ B.begins[ bl1 ] ];
    for( unsigned pl = adj_w_ends[ st - 1 ]; pl < adj_w_ends[ st ]; ++pl ){
      unsigned tr = adj_w_trans[ pl ], bl2 = B.sets[ w_heads[ tr ] ];
      if( !B.mids[ bl2 ] ){
        touched_sets.add( bl2 ); B.mids[ bl2 ] = true;
        ww[ bl2 ] = w_weights[ tr ];
      }else{ ww[ bl2 ] += w_weights[ tr ]; }
    }
    while( touched_sets.stack_size() ){
      unsigned bl2 = touched_sets.remove(); B.mids[ bl2 ] = false;
      MDP_round_weight( ww[ bl2 ] );
      if( ww[ bl2 ] != MDP_zero_weight ){ ++nr_w_out; }
    }
  }

  /* Release memory that is not needed any more. */
  B.locs.del();
  T.elems.del(); T.locs.del(); T.sets.del(); T.begins.del(); T.ends.del();
  T.mids.del();
  w_tails.del();
  in_l_trans.del(); in_l_ends.del();
  touched_states.del();
  s_counts.del();
  phase = minimized;

}   // end of minimize


/* INPUT ROUTINES */
/* The user calls these to give the input MDP to the class. */

private:
  inline static bool bad_st( unsigned st ){ return st < 1 || st > nr_states; }
  inline static bool bad_lb( unsigned lb ){ return lb < 1 || lb > nr_labels; }
  inline static bool bad_bl( unsigned bl ){
    return bl < 1 || bl > nr_init_blocks;
  }
public:


/* Get the sizes from the user. */
static void store_sizes(
  unsigned nrst, unsigned nrlb, unsigned nrlt, unsigned nrwt, unsigned nrbl
){

  /* Check the arriving data. */
  if( phase != initial ){ err::msg( "Sizes are given a second time" ); }
  if( nrlt >= nr_l_max1 ){ err::msg( "Too many l-transitions" ); }
  err::ready();

  /* Store the data. */
  nr_states = nrst; nr_labels = nrlb; nr_l_trans = nrlt; nr_w_trans = nrwt;
  nr_init_blocks = nrbl; phase = sizes_given;

}


/* Get a labelled transition from the user. */
inline static void store_l_transition(
  unsigned tail, unsigned label, unsigned head
){

  /* Check the arriving data. */
  #ifndef no_debug
    if( phase != sizes_given ){ err::msg( "Sizes must be given first" ); }
    if( l_cnt >= nr_l_trans ){ err::msg( "Too many labelled transitions" ); }
    if( bad_st( tail ) ){
      err::msg( "Bad tail state number" ); err::msg( tail );
    }
    if( bad_lb( label ) ){
      err::msg( "Bad label number" ); err::msg( label );
    }
    if( bad_st( head ) ){
      err::msg( "Bad head state number" ); err::msg( head );
    }
    err::ready();
  #endif

  /* Reserve memory for data structures when the first transition arrives. */
  if( !l_cnt ){
    l_tails.make_size( nr_l_trans ); l_labels.make_size( nr_l_trans );
    l_heads.make_size( nr_l_trans );
  }

  /* Store the data. */
  ++l_cnt;
  l_tails[ l_cnt ] = tail; l_heads[ l_cnt ] = head; l_labels[ l_cnt ] = label;

  /* Group the transitions, when the last one arrives. */
  if( l_cnt == nr_l_trans ){
    T.elems.make_size( nr_l_trans ); T.ends.make_size( nr_l_trans );
    group_l_transitions();
  }

}


/* Get a weighed transition from the user. */
inline static void store_w_transition(
  unsigned tail, const MDP_weight_type & weight, unsigned head
){

  /* Check the arriving data. */
  #ifndef no_debug
    if( phase != sizes_given ){ err::msg( "Sizes must be given first" ); }
    if( w_cnt >= nr_w_trans ){ err::msg( "Too many weighed transitions" ); }
    if( bad_st( tail ) ){
      err::msg( "Bad tail state number" ); err::msg( tail );
    }
    if( bad_st( head ) ){
      err::msg( "Bad head state number" ); err::msg( head );
    }
    err::ready();
  #endif

  /* Reserve memory for data structures when the first transition arrives. */
  if( !w_cnt ){
    w_tails.make_size( nr_w_trans ); w_weights.make_size( nr_w_trans );
    w_heads.make_size( nr_w_trans );
  }

  /* Store the data. */
  ++w_cnt; w_tails[ w_cnt ] = tail; w_heads[ w_cnt ] = head;
  w_weights[ w_cnt ] = weight;

}


/* Get one state--in--block info from the user. */
inline static void store_block( unsigned state, unsigned block ){

  /* Check the arriving data for sanity. */
  #ifndef no_debug
    if( phase != sizes_given ){ err::msg( "Sizes must be given first" ); }
    if( bad_st( state ) ){
      err::msg( "Bad block state number" ); err::msg( state );
    }
    if( bad_bl( block ) ){
      err::msg( "Bad block number" ); err::msg( block );
    }
    err::ready();
  #endif

  /* Reserve and initialize memory for B.sets when the first info arrives. */
  if( !B.sets.active() ){
    B.sets.make_size( nr_states );
    for( unsigned st = 1; st <= nr_states; ++st ){ B.sets[ st ] = 1; }
  }

  /* More sanity checks. */
  #ifndef no_debug
    if( B.sets[ state ] != 1 ){
      err::msg( "State" ); err::msg( state ); err::msg( "is in two blocks");
      err::ready();
    }
  #endif

  /* Store the data. */
  B.sets[ state ] = block;

}


/* OUTPUT ROUTINES */
/* The user calls these to get the result MDP from the class. */

/* Get the sizes from this class. */
static void give_sizes(
  unsigned & nrst, unsigned & nrlb, unsigned & nrlt, unsigned & nrwt,
  unsigned & nrbl
){
  if( phase != minimized ){
    err::msg( "Minimization has not been called" ); err::ready();
  }
  nrst = B.nr_sets; nrlb = nr_labels; nrlt = nr_l_out; nrwt = nr_w_out;
  nrbl = nr_init_blocks;
}


/* Get a labelled transition from this class. Works by scanning until a new
  transition is found or possibilities exhausted. Only transitions of the
  first state in each block are used, and their counters are set to 1 to
  prevent finding other transitions from the same state with the same label to
  the same block. */
private: static unsigned given_tr;  // = 0
public:

static void give_l_transition(
  unsigned & tail, unsigned & label, unsigned & head
){
  #ifndef no_debug
    if( phase != minimized ){
      err::msg( "Minimization has not been called" ); err::ready();
    }
  #endif
  unsigned st;
  do{
    if( given_tr >= nr_l_trans ){ tail = label = head = 0; return; }
    ++given_tr;
    st = l_tails[ given_tr ]; tail = B.sets[ st ];
  }while(
    st != B.elems[ B.begins[ tail ] ] || t_counts[ links[ given_tr ] ]
  );
  t_counts[ links[ given_tr ] ] = 1;
  label = l_labels[ given_tr ]; head = B.sets[ l_heads[ given_tr ] ];
}


/* Get a weighed transition from this class. For each block, computes the
  weighed transitions from its first state to other blocks and stores the
  result in touched_sets and ww. Uses B.mids to keep track if the head block
  has been seen. When called, returns a previously computed transition if
  exists; otherwise scans the blocks until a new transition is found or
  possibilities exhausted. */
private: static unsigned given_bl;  // = 0
public:

static void give_w_transition(
  unsigned & tail, MDP_weight_type & weight, unsigned & head
){
  #ifndef no_debug
    if( phase != minimized ){
      err::msg( "Minimization has not been called" ); err::ready();
    }
  #endif
  while( true ){
    if( !touched_sets.stack_size() ){

      /* If transitions have been exhausted, return a fake one. */
      if( given_bl >= B.nr_sets ){
        tail = head = 0; weight = MDP_unused_weight; return;
      }

      /* Compute transitions from (the first state of) the next block. */
      ++given_bl;
      unsigned st = B.elems[ B.begins[ given_bl ] ];
      for( unsigned pl = adj_w_ends[ st - 1 ]; pl < adj_w_ends[ st ]; ++pl ){
        unsigned tr = adj_w_trans[ pl ], bl2 = B.sets[ w_heads[ tr ] ];
        if( !B.mids[ bl2 ] ){
          touched_sets.add( bl2 ); B.mids[ bl2 ] = true;
          ww[ bl2 ] = w_weights[ tr ];
        }else{ ww[ bl2 ] += w_weights[ tr ]; }
      }

    }

    /* Try to return an already computed transition. */
    while( touched_sets.stack_size() ){
      unsigned bl2 = touched_sets.remove(); B.mids[ bl2 ] = false;
      MDP_round_weight( ww[ bl2 ] );
      if( ww[ bl2 ] != MDP_zero_weight ){
        tail = given_bl; weight = ww[ bl2 ]; head = bl2; return;
      }
    }

  }
}


/* Numbers of final blocks that originated from initial block "out_block" are
  obtained by calling give_block_first( out_block ) and then give_block_next()
  until it starts yielding 0. */
private:
static unsigned
  gb_limit, // = 0; stop current scanning at this location of B.elems
  gb_now;   // = 0; most recently returned result block number
public:

inline static unsigned give_block_first( unsigned out_block ){
  #ifndef no_debug
    if( phase != minimized ){
      err::msg( "Minimization has not been called" );
    }
    if( bad_bl( out_block ) ){
      err::msg( "Bad result block number" ); err::msg( out_block );
    }
    err::ready();
  #endif
  gb_limit = init_block_borders[ out_block ];
  gb_now = B.sets[ B.elems[ init_block_borders[ out_block - 1 ] ] ];
  return gb_now;
}

inline static unsigned give_block_next(){
  #ifndef no_debug
    if( phase != minimized ){
      err::msg( "Minimization has not been called" ); err::ready();
    }
  #endif
  if( !gb_now ){ return 0; }
  unsigned lc = B.ends[ gb_now ];
  if( lc >= gb_limit ){ return 0; }
  gb_now = B.sets[ B.elems[ lc ] ]; return gb_now;
}


/* Returns the number of the final block the state st belongs to. */
inline static unsigned which_block( unsigned st ){
  #ifndef no_debug
    if( phase != minimized ){
      err::msg( "Minimization has not been called" );
    }
    if( bad_st( st ) ){ err::msg( "Bad state number" ); err::msg( st ); }
    err::ready();
  #endif
  return B.sets[ st ];
}



/* This can be called to release all memory reserved and not automatically
  released by this class. */
static void terminate(){
  B.nr_sets = 0; B.elems.del(); B.locs.del(); B.sets.del(); B.begins.del();
  B.ends.del(); B.mids.del();
  T.nr_sets = 0; T.elems.del(); T.locs.del(); T.sets.del(); T.begins.del();
  T.ends.del(); T.mids.del();
  phase = initial;
  l_tails.del(); l_labels.del(); l_heads.del();
  w_tails.del(); w_heads.del(); w_weights.del();
  in_l_trans.del(); in_l_ends.del(); adj_w_trans.del(); adj_w_ends.del();
  nr_states = nr_labels = nr_l_trans = nr_w_trans = nr_init_blocks = 0;
  l_cnt = w_cnt = nr_l_out = nr_w_out = 0;
  touched_states.del(); touched_sets.del(); init_block_borders.del();
  cord_splitter = 2; nr_counters = 0;
  links.del(); t_counts.del(); s_counts.del(); bisim_splitter = 1;
  ww.del(); markov_splitter = 1;
  given_tr = given_bl = gb_limit = gb_now = 0;
}


/* END OF THE CLASS AND DEFINITION OF STATIC VARIABLES */

};

MDPmin::refinable_partition MDPmin::B, MDPmin::T;
unsigned
  MDPmin::nr_states = 0, MDPmin::nr_labels = 0, MDPmin::nr_l_trans = 0,
  MDPmin::nr_w_trans = 0, MDPmin::nr_init_blocks = 0;
const unsigned MDPmin::nr_l_max1 = ~0u/2 + 1;
MDPmin::uArray MDPmin::l_tails, MDPmin::l_labels, MDPmin::l_heads;
MDPmin::uArray MDPmin::w_tails, MDPmin::w_heads;
MDPmin::wArray MDPmin::w_weights;
MDPmin::uArray
  MDPmin::in_l_trans, MDPmin::in_l_ends,
  MDPmin::adj_w_trans, MDPmin::adj_w_ends,
  MDPmin::touched_states, MDPmin::touched_sets;
unsigned MDPmin::cord_splitter = 2, MDPmin::nr_counters = 0;
MDPmin::uArray MDPmin::links, MDPmin::t_counts, MDPmin::s_counts;
unsigned MDPmin::bisim_splitter = 1;
MDPmin::wArray MDPmin::ww;
unsigned MDPmin::markov_splitter = 1;
MDPmin::phase_type MDPmin::phase = initial;
unsigned
  MDPmin::l_cnt = 0, MDPmin::w_cnt = 0,
  MDPmin::nr_l_out = 0, MDPmin::nr_w_out = 0;
MDPmin::uArray MDPmin::init_block_borders;
unsigned
  MDPmin::given_tr = 0, MDPmin::given_bl = 0,
  MDPmin::gb_limit = 0, MDPmin::gb_now = 0;

#endif
