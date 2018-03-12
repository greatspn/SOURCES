#ifndef __COMPACT_TABLE_H__
#define __COMPACT_TABLE_H__

//=============================================================================
// Forward declarations:
//=============================================================================

template<typename T, typename ...Tail>
class table;

template<typename Table>
class const_table_iterator;

// template<typename T, typename U>
// class private_integer;

//=============================================================================

// vector of unsigned integers which chooses dinamically
// its internal storage (1 to 63 bits on 64-bit machines)

class compact_column {
public:
    compact_column();
    compact_column(const compact_column &) = default;
    compact_column(compact_column &&) = default;
    inline compact_column &operator = (const compact_column &) = default;
    inline compact_column &operator = (compact_column &&) = default;
    bool operator == (const compact_column &t) const;
    inline bool operator != (const compact_column &t) const  { return !operator==(t); }

    inline size_t size() const { return sz; }

    inline size_t at(size_t pos) const {
        assert(pos < size());
        if (bpv == 0)
            return 0;
        // Layout organization of the stored elements inside the elems[] vector
        //
        // CASE 1: Single word storage:
        //   Example: shift1 = 3, sz = 20, assuming 32-bit size_t's:
        //                                <-- shift1
        //            ********************
        //   10987654321098765432109876543210
        //   \--elems[h1ind] = elems[h2ind]-/
        //
        // CASE 2: Double word storage, assuming 32-bit size_t's:
        //   Example: shift1 = 18, sz = 20:
        //                                                <----------------- shift1
        //                           ****** **************
        // 32109876543210987654321098765432 10987654321098765432109876543210
        // \--------elems[h2ind]----------/ \--------elems[h1ind]----------/
        //

        // Index in elems[] of the first and second parts of the stored value
        const size_t h1ind = (pos * bpv) >> TBL_SHIFT;
        const size_t h2ind = ((pos + 1) * bpv - 1) >> TBL_SHIFT;
        // Bit shift to retrieve the first part
        const size_t shift1 = (pos * bpv) & TBL_MASK;

        // Load the first part (1 & 2 word cases)
        size_t value = elems[h1ind] >> shift1;
        // Load the second part (in the double word case)
        if (h2ind > h1ind)
            value |= elems[h2ind] << (TBL_BITS - shift1);
        // Mask away higher bits (keep the bpv's less significant bits)
        value &= (1ul << bpv) - 1;
        return value;
    }

    // append at the end a new element
    void push_back(size_t value);

    // number of bytes used.
    size_t num_bytes() const;

    // set a new value at position i
    void set(size_t pos, size_t value);

    void swap(compact_column &cs);
    void clear();
    void resize(size_t new_sz, size_t value = 0ul);
    void reserve(size_t rsv_sz);
    size_t capacity() const;

protected:
    size_t       		sz;		// Size of the vector
    size_t 				bpv;    // Bits per stored value
    std::vector<size_t> elems;  // Compacted storage

    // minimum storage type needed to store value
    static size_t storage_for(size_t value);

    // change the storage type of this compact_column
    void change_storage_type(size_t n_bpv);

    // Example: TBL_BITS=32, TBL_SHIFT=5, TBL_MASK=31    (32-bit machines)
    //          TBL_BITS=64, TBL_SHIFT=6, TBL_MASK=63    (64-bit machines)
    static constexpr size_t TBL_BITS  = sizeof(size_t) *CHAR_BIT;
    static constexpr size_t TBL_SHIFT = (TBL_BITS == 32 ? 5 : (TBL_BITS == 64 ? 6 : 0));
    static constexpr size_t TBL_MASK  = (1ul << TBL_SHIFT) - 1;
    static_assert((TBL_BITS == 32 && TBL_SHIFT == 5) ||
                  (TBL_BITS == 64 && TBL_SHIFT == 6),
                  "Incorrect TBL_BITS/TBL_SHIFT assignments. "
                  "Review the compact_column class.");
};

//=============================================================================

// A segmented column is just a two-level vector of size_t's:
// the first level is a variable-size vector, and the second level is made of
// tables of the specified fixed size SZ.
template<size_t SZ = 4096>
class sgm_column {
public:
    sgm_column();
    sgm_column(const sgm_column &) = default;
    sgm_column(sgm_column &&) = default;
    inline sgm_column &operator = (const sgm_column &) = default;
    inline sgm_column &operator = (sgm_column &&) = default;
    bool operator == (const sgm_column &t) const;
    inline bool operator != (const sgm_column &t) const  { return !operator==(t); }

    inline size_t size() const {
        return vec.size() == 0 ? 0 : (vec.size() - 1) * SZ + vec.back().size();
    }
    inline size_t at(size_t k) const {
        assert(k < size()); return vec[k / SZ].at(k % SZ);
    }

    // append at the end a new element
    void push_back(size_t value);

    // number of bytes used.
    size_t num_bytes() const;

    // set a new value at position i
    inline void set(size_t k, size_t value) {
        assert(k < size()); vec[k / SZ].set(k % SZ, value);
    }
    void swap(sgm_column &sc);
    void clear();
    void resize(size_t new_sz, size_t value = 0ul);
    void reserve(size_t rsv_sz);
    size_t capacity() const;

protected:
    /// Internal column storage, divided into multiple segments of SZ entries each
    std::vector<compact_column> 	vec;
};

extern template class sgm_column<>;

//=============================================================================
// Access to single entres in a table<...> instance
//=============================================================================

namespace detail {
//
// Helpers for the get_at() method
//
template<size_t N, typename T, typename ...Tail>
struct get_at_helper {
    typedef typename get_at_helper < N - 1, Tail... >::return_type return_type;
    inline return_type get_at(const table<T, Tail...> &table, size_t pos)
    {   get_at_helper < N - 1, Tail... > h; return h.get_at(table.nxt, pos); }
};
template<typename T, typename ...Tail>
struct get_at_helper<0, T, Tail...> {
    typedef typename T::value_type return_type;
    inline return_type get_at(const table<T, Tail...> &table, size_t pos)
    {   return table.at(pos); }
};
//
// Helpers for the set_at() method
//
template<size_t N, typename T, typename ...Tail>
struct set_at_helper {
    typedef typename set_at_helper < N - 1, Tail... >::value_type value_type;
    inline void set_at(table<T, Tail...> &table, size_t pos, value_type value)
    {   set_at_helper < N - 1, Tail... > h; h.set_at(table.nxt, pos, value); }
};
template<typename T, typename ...Tail>
struct set_at_helper<0, T, Tail...> {
    typedef typename T::value_type value_type;
    inline void set_at(table<T, Tail...> &table, size_t pos, value_type value)
    {   table[pos] = value; }
};
};

template<size_t N, typename T, typename ...Tail>
inline typename detail::get_at_helper<N, T, Tail...>::return_type
get_at(const table<T, Tail...> &table, size_t pos) {
    detail::get_at_helper<N, T, Tail...> h;
    return h.get_at(table, pos);
}

template<size_t N, typename T, typename ...Tail>
void set_at(table<T, Tail...> &table, size_t pos,
            typename detail::set_at_helper<N, T, Tail...>::value_type value) {
    detail::set_at_helper<N, T, Tail...> h;
    h.set_at(table, pos, value);
}

//=============================================================================
// Access to table<> entries
//=============================================================================

template<class Table>
class table_entry {
public:
    typedef typename Table::index_type  index_type;
    typedef typename Table::value_type  value_type;

    table_entry(Table *_ptab, index_type _i)
        : ptab(_ptab), i(_i) { }
    inline table_entry(const table_entry &) = default;
    inline table_entry(table_entry &&) = default;

    table_entry &operator = (value_type v) { ptab->set(i, v); return *this; }
    table_entry &operator = (const table_entry &te) { return *this = value_type(te); }
    inline operator value_type() const { return ptab->at(i); }

    template<size_t N> inline auto get() -> decltype(get_at<N>(Table(), 0))
    { return get_at<N>(*ptab, i); }

protected:
    Table  	*ptab;	///< Pointed table<T>
    index_type  i;		///< Current position
};

template<class Table>
inline ostream &operator << (ostream &os, const table_entry<Table> &te)
{ return os << typename table_entry<Table>::value_type(te); }

template<class Table>
inline bool operator == (const table_entry<Table> &v1, const typename Table::value_type &v2)
{ return typename Table::value_type(v1) == v2; }

template<class Table>
inline bool operator == (const typename Table::value_type &v1, const table_entry<Table> &v2)
{ return v1 == typename Table::value_type(v2); }

template<class Table>
inline bool operator != (const table_entry<Table> &v1, const typename Table::value_type &v2)
{ return typename Table::value_type(v1) != v2; }

template<class Table>
inline bool operator != (const typename Table::value_type &v1, const table_entry<Table> &v2)
{ return v1 != typename Table::value_type(v2); }

//=============================================================================
// Table column descriptors
//=============================================================================

// Column descriptors
namespace col {

//--------------------------------------
// Unsigned integer column
//--------------------------------------
template<typename T, size_t Offset = 0>
struct uint_col {
    typedef T value_type;
    uint_col() {}
    uint_col(uint_col &&) = default;
    uint_col(const uint_col &) = default;
    inline uint_col &operator = (const uint_col &) = default;
    inline uint_col &operator = (uint_col &&) = default;
    inline bool operator == (const uint_col &) const { return true; }
    inline bool operator != (const uint_col &) const { return false; }

    inline size_t encode(value_type v) const { return size_t(v) + Offset; }
    inline value_type decode(size_t v) const { return value_type(v - Offset); }

    inline void swap(uint_col &uc) {}
    inline void clear() {}
    inline size_t num_bytes() const { return 0; }
};

//--------------------------------------
// Signed integer column
//--------------------------------------
template<typename T, size_t Offset = 0>
struct int_col {
    typedef T value_type;
    int_col() {}
    int_col(int_col &&) = default;
    int_col(const int_col &) = default;
    inline int_col &operator = (const int_col &) = default;
    inline int_col &operator = (int_col &&) = default;
    inline bool operator == (const int_col &) const { return true; }
    inline bool operator != (const int_col &) const { return false; }

    inline size_t encode(value_type _v) const
    { ssize_t v = ssize_t(_v) + Offset; return (v < 0) ? ((-v) << 1) + 1 : (v << 1); }
    inline value_type decode(size_t v) const
    { ssize_t _v = (v & 1) ? -(v >> 1) : (v >> 1);  return value_type(_v - Offset); }

    inline void swap(int_col &uc) {}
    inline void clear() {}
    inline size_t num_bytes() const { return 0; }
};

//--------------------------------------
// Column with indirect hash table
//--------------------------------------
template<typename T>
struct indexed_col {
    typedef T value_type;
    indexed_col() {}
    indexed_col(indexed_col &&) = default;
    indexed_col(const indexed_col &) = default;
    inline indexed_col &operator = (const indexed_col &) = default;
    inline indexed_col &operator = (indexed_col &&) = default;
    inline bool operator == (const indexed_col &ic) const { return value2index == ic.value2index; }
    inline bool operator != (const indexed_col &ic) const { return !(*this == ic); }

    inline size_t encode(value_type v) const {
        typename std::unordered_map<T, size_t>::const_iterator it = value2index.find(v);
        if (it != value2index.end()) {
            // Value already exists in the index
            assert(decode(it->second) == v);
            return it->second;
        }
        // Add a new entry
        size_t index = value2index.size();
        value2index.insert(make_pair(v, index));
        values.push_back(v);
        assert(decode(index) == v);
        assert(value2index.size() == values.size());
        return index;
    }
    inline value_type decode(size_t v) const {
        assert(v < values.size());
        return values[v];
    }

    inline void swap(indexed_col &uc) {
        value2index.swap(uc.value2index);
        values.swap(uc.values);
    }
    inline void clear() {
        value2index.clear();
        values.clear();
    }
    inline size_t num_bytes() const {
        return (value2index.size() * sizeof(std::tuple<T, size_t, void *, void *>) +
                values.size() * sizeof(T));
    }

protected:
    mutable std::unordered_map<T, size_t>  value2index;
    mutable std::vector<T>    			   values;
};


}; // namespace col

//=============================================================================

namespace detail {
template <typename T, typename Tuple> struct cat_types;
template <typename T, typename ...Args>
struct cat_types<T, std::tuple<Args...>> {
    typedef typename std::tuple<T, Args...> tuple_type;
};
}; //namespace detail

//=============================================================================

template<typename T, typename ...Tail>
class table {
public:
    typedef typename table<T>::index_type      			  index_type;
    typedef typename table<T>::value_type      			  value_type;
    typedef const_table_iterator<table<T, Tail...>>		  const_iterator;
    typedef typename table<T>::table_entry_t			  table_entry_t;
    typedef typename detail::cat_types<value_type,
            typename table<Tail...>::tuple_type>::tuple_type  tuple_type;

    table() {}
    table(table &&) = default;
    table(const table &) = default;
    inline table &operator = (const table &) = default;
    inline table &operator = (table &&) = default;
    inline bool operator == (const table &t) const  { return col == t.col && (nxt == t.nxt); }
    inline bool operator != (const table &t) const  { return col != t.col && (nxt != t.nxt); }

    inline size_t size() const 				        { assert(col.size() == nxt.size()); return col.size(); }
    inline value_type at(size_t pos) const 			{ return col.at(pos); }
    inline value_type operator[](size_t pos) const 	{ return col.at(pos); }
    inline table_entry_t operator[](size_t pos)     { return col[pos]; }

    // append at the end a new element
    template<typename... Args>
    inline void push_back(value_type value, Args... args) {
        col.push_back(value); nxt.push_back(args...);
    }

    // Tuple operations
    template<typename Tuple, size_t N = 0>
    inline void push_back_tuple(Tuple tuple) {
        col.push_back(std::get<N>(tuple));
        nxt.template push_back_tuple < Tuple, N + 1 > (tuple);
    }
    inline tuple_type tuple_at(size_t pos) const {
        return std::tuple_cat(std::tuple<value_type>(at(pos)), nxt.tuple_at(pos));
    }
    template<typename Tuple, size_t N = 0>
    inline void set_tuple(size_t pos, const Tuple tuple) {
        col.set(pos, std::get<N>(tuple));
        nxt.template set_tuple < Tuple, N + 1 > (pos, tuple);
    }

    // number of bytes used.
    inline size_t num_bytes() const 		        { return col.num_bytes() + nxt.num_bytes(); }

    inline const_iterator begin() const 	        { return const_iterator(this, 0); }
    inline const_iterator end() const  		        { return const_iterator(this, size()); }

    template<typename... Args>
    inline void set(size_t pos, value_type value, Args... args) {
        col.set(pos, value); nxt.set(pos, args...);
    }
    inline void swap(table &tbl)    		        { col.swap(tbl.col);  nxt.swap(tbl.nxt); }
    inline void clear() 					        { col.clear();        nxt.clear(); }
    inline void resize(size_t new_sz) 		        { col.resize(new_sz); nxt.resize(new_sz); }
    template<typename... Args>
    inline void resize(size_t new_sz, value_type value, Args... args) {
        col.resize(new_sz, value); nxt.resize(new_sz, args...);
    }
    inline void reserve(size_t rsv_sz) 		        { col.reserve(rsv_sz); nxt.reserve(rsv_sz); }
    inline size_t capacity() const 			        { return std::max(col.capacity(), nxt.capacity()); }

public:
    table<T>        col;   // Columnar storage for T
    table<Tail...>  nxt;	// Recursive storage for the remained args...
};

//=============================================================================
// Base case of the recursive type
//=============================================================================

template<typename T>
class table<T> {
public:
    typedef size_t      			         index_type;
    typedef typename T::value_type	         value_type;
    typedef T 						         column_t;
    typedef const_table_iterator<table<T>>	 const_iterator;
    typedef table_entry<table<T>>            table_entry_t;
    typedef std::tuple<value_type>	         tuple_type;

    table() {}
    table(table &&) = default;
    table(const table &) = default;
    inline table &operator = (const table &) = default;
    inline table &operator = (table &&) = default;
    inline bool operator == (const table &t) const    { return col == t.col && impl == t.impl; }
    inline bool operator != (const table &t) const    { return col != t.col || impl != t.impl; }

    inline size_t size() const 				          { return impl.size(); }
    inline value_type at(size_t pos) const 		      { return col.decode(impl.at(pos)); }
    inline value_type operator[](size_t pos) const    { return at(pos); }
    inline table_entry_t operator[](size_t pos)       { return table_entry_t(this, pos); }

    // append at the end a new element
    inline void push_back(value_type value)           { impl.push_back(col.encode(value)); }

    // Tuple operations
    template<typename Tuple, size_t N = 0>
    inline void push_back_tuple(Tuple tuple)          { push_back(std::get<N>(tuple)); }
    inline tuple_type tuple_at(size_t pos) const      { return std::tuple<value_type>(at(pos)); }
    template<typename Tuple, size_t N = 0>
    inline void set_tuple(size_t pos, const Tuple tuple) { col.set(pos, std::get<N>(tuple)); }

    // number of bytes used.
    inline size_t num_bytes() const                   { return impl.num_bytes() + col.num_bytes(); }

    inline const_iterator begin() const               { return const_iterator(this, 0); }
    inline const_iterator end() const                 { return const_iterator(this, size()); }

    inline void set(size_t pos, value_type value)     { impl.set(pos, col.encode(value)); }
    inline void swap(table &tbl)    		          { impl.swap(tbl.impl); col.swap(tbl.col); }
    inline void clear() 					          { impl.clear(); col.clear(); }
    inline void resize(size_t new_sz) 		          { impl.resize(new_sz); }
    inline void resize(size_t new_sz, value_type val) { impl.resize(new_sz, col.encode(val)); }
    inline void reserve(size_t rsv_sz) 		          { impl.reserve(rsv_sz); }
    inline size_t capacity() const 			          { return impl.capacity(); }

protected:
    sgm_column<4096>   impl; // Real storage
    column_t  		   col;	 // Column descriptor
};

//=============================================================================


//=============================================================================
// Iterator of the table<> storage class
//=============================================================================

template<typename Table>
class const_table_iterator {
public:
    typedef Table 					table_type;
    typedef size_t    				index_type;
    typedef const_table_iterator 	this_type;

    inline const_table_iterator() : ptable(nullptr), i(0) {}
    inline const_table_iterator(const table_type *_ptable, index_type _i)
    /**/ : ptable(_ptable), i(_i) { }
    inline const_table_iterator(const this_type &) = default;
    inline const_table_iterator(this_type &&) = default;
    inline const_table_iterator &operator = (const const_table_iterator &) = default;
    inline const_table_iterator &operator = (const_table_iterator &&) = default;

    index_type index() const		 	       {  no_end(); return i;  }
    template<size_t N>
    inline decltype(get_at<N>(table_type(), 0u)) get() { no_end(); return get_at<N>(*ptable, i); }

    inline decltype(get_at<0>(table_type(), 0u)) operator*() const {
        static_assert(std::tuple_size<typename table_type::tuple_type>::value == 1,
                      "operator*() can be called only on single-typed tables.");
        no_end(); return get_at<0>(*ptable, i);
    }

    this_type &operator++()   { no_end();  ++i;  return *this; }
    this_type operator++(int) { no_end();  this_type ci = *this;  ++i;  return ci; }
    bool operator==(const this_type &c) const  {  return i == c.i && ptable == c.ptable;  }
    bool operator!=(const this_type &c) const  {  return i != c.i || ptable != c.ptable;  }

protected:
    inline void no_end() const  {  assert(ptable && i < ptable->size());  }

    const table_type   *ptable;	///< Pointed vector
    size_t  			i;		///< Current position
};

//=============================================================================
// Trait class instantiated by seq_store'd objects
//=============================================================================

template<typename Vec>
struct sequence_traits {
    // typedef Vec                               vector_type;
    // typedef typename Vec::value_type          value_type;
    // typedef table<col::uint_col<size_t>>      table_type;
    // typedef typename table_type::tuple_type   tuple_type;

    // inline value_type empty_elem() const;
    // inline tuple_type to_tuple(const value_type& arg) const;
    // inline value_type from_tuple(tuple_type tuple) const;
};

//=============================================================================
// Sequence identifiers used by (sparse_)seq_store<>
//=============================================================================

template<typename Vec>
struct sequence_id_impl {
    inline sequence_id_impl(size_t _v = 0) : v(_v) { }
    sequence_id_impl(sequence_id_impl &&) = default;
    sequence_id_impl(const sequence_id_impl &) = default;
    inline sequence_id_impl &operator = (const sequence_id_impl &) = default;
    inline sequence_id_impl &operator = (sequence_id_impl &&) = default;

    inline bool operator == (const sequence_id_impl &s) const { return v == s.v; }
    inline bool operator != (const sequence_id_impl &s) const { return v != s.v; }
    inline bool operator < (const sequence_id_impl &s) const { return v <  s.v; }
    inline bool operator <= (const sequence_id_impl &s) const { return v <= s.v; }
    inline bool operator > (const sequence_id_impl &s) const { return v >  s.v; }
    inline bool operator >= (const sequence_id_impl &s) const { return v >= s.v; }

    explicit operator size_t() const { return v; }

    static const sequence_id_impl INVALID_SEQ;

protected:
    size_t v;
};

template<class Vec>
inline ostream &operator << (ostream &os, const sequence_id_impl<Vec> &sid) {
    if (sid == sequence_id_impl<Vec>::INVALID_SEQ)
        return os << "X";
    return os << size_t(sid);
}

//=============================================================================

/// Structure of the seq_store's storages
enum SeqTableKind {
    INSERTED = 0,	/// First-level table (where insert()ed nodes go)
    FRAGMENTS = 1,	/// Fragments table.
    NUM_SEQUENCE_TABLES = 2
};
inline void next_stk(SeqTableKind &stk) { stk = SeqTableKind(int(stk) + 1); }

//=============================================================================
// const_iterator for (sparse_)seq_store's objects
//=============================================================================

template<class seq_store>
class stored_seq_const_iterator {
public:
    typedef typename seq_store::index_type        index_type;
    typedef typename seq_store::value_type        value_type;
    typedef typename seq_store::sequence_id       sequence_id;
    typedef stored_seq_const_iterator             this_class;

    stored_seq_const_iterator(const seq_store *p = nullptr);
    stored_seq_const_iterator(const this_class &c);
    stored_seq_const_iterator(const seq_store *p, sequence_id sid, SeqTableKind stk);
    inline this_class &operator = (const this_class &) = default;
    inline this_class &operator = (this_class &&) = default;

    // get back the value
    inline value_type operator*() const {
        assert(cur_sid != sequence_id::INVALID_SEQ);
        assert(cur_stk() == FRAGMENTS || cur_sid != pss->EmptyElemId);
        return pss->conv.from_tuple(pss->tabs[cur_stk()].s_values.tuple_at(size_t(cur_sid)));
    }

    // // get back the index
    // inline index_type index() const {
    // 	assert(cur_sid.second != sequence_id::INVALID_SEQ);
    // 	return pss->tabs[cur_sid.first].s_index[size_t(cur_sid.second)];
    // }

    this_class &operator++();
    this_class operator++(int) 							{ this_class c(*this); return ++c; }
    inline bool operator == (const this_class &c) const { return cur_sid == c.cur_sid;	}
    inline bool operator != (const this_class &c) const { return !operator == (c); }

protected:
    inline void push_sid(sequence_id sid);
    inline void pop_sid();
    inline sequence_id top_sid() const;

    inline SeqTableKind cur_stk() const { return (depth > 0 ? FRAGMENTS : root_stk); }

    // Maximum iterator stack depth (non-recursive in-depth traversal)
    static const int MAX_TREE_DEPTH = 32;

    sequence_id	  	  sid_stack[MAX_TREE_DEPTH]; // Visiting stack
    sequence_id       cur_sid;  // Currently visited entry in the sequence
    int				  depth;    // Depth of sid_stack[]
    const seq_store  *pss;	    // The sequence storage itself
    SeqTableKind      root_stk; // The table of the initial node
};

//=============================================================================
// Associative storage of vector elements with sharing
// The seq_store<Vec> class stores vectors of elements and gets back an id
// Stored vector entries can be accessed directly with the get_at method,
// or with the const_iterator. Internally, sequences are stored as balanced
// trees with sharing of common subtrees. It is possible to test the existance
// of an vector with the exist() method.
// The number of vectors inserted is given by the num_inserted_seq() method.
// Note that internally the seq_store class contains more vectors, due to its
// memory structure. The total number of contained vectors (inserted explicitly
// or not) is given by the max_id() method.
// The typename Vec is the vector of objects (like std::vector<Obj>), and the
// Args... are the column descriptor of the stored object fields.
// A trait tuple_conv<Obj, std::tuple<...>> implements the conversion from
// Obj instances to the stored columns, and viceversa.
//=============================================================================

template<typename Vec, typename Traits = sequence_traits<Vec> >
struct seq_store {
public:
    typedef typename Traits::table_type		table_type;
    typedef size_t     						index_type;
    typedef typename Traits::tuple_type    	tuple_type;
    typedef typename Vec::value_type     	value_type;
    typedef typename Vec::const_iterator 	vec_const_iterator;
    typedef sequence_id_impl<Vec>        	sequence_id;

protected:
    typedef table < col::uint_col < sequence_id, +1 >>  table_of_sequence_id;
    typedef table<col::uint_col<size_t>>   	       table_of_lengths;

    // Table of nodes
    struct seq_tables {
        // Hash table
        table_of_sequence_id   hm_htab;	   // Heads of the hash table linked lists

        // Columnar representation of the stored data:
        table_of_sequence_id   s_left;	   // Left branches
        table_of_sequence_id   s_right;	   // Right branches
        table_of_lengths       s_length;   // Length of the sequences
        table_type        	   s_values;   // Stored values
        table_of_sequence_id   s_next;	   // Hash chain (next sid in hash table chain)

        inline sequence_id max_id() const { return s_values.size(); }
        inline size_t data_bytes() const {
            return (s_values.num_bytes() + s_length.num_bytes() +
                    s_left.num_bytes() + s_right.num_bytes());
        }
        inline size_t hash_bytes() const { return s_next.num_bytes() + hm_htab.num_bytes(); }
    };
    // Two level tables.
    // The first table contains the sequences insert()ed by the user of the class
    // The second table contains all the fragments generated by the first-level
    //   insertions.
    seq_tables tabs[NUM_SEQUENCE_TABLES];

    // The insert()ed empty element, if any. This exists only when an empty vector is
    // explicitly inserted in the first-level table. The fragment table does not have
    // any empty element.
    sequence_id EmptyElemId = sequence_id::INVALID_SEQ;

    // Conversion of objects from/to tuples
    Traits conv;

    template<typename SeqStore> friend void print_memory_occupation(const SeqStore &s);

public:
    typedef stored_seq_const_iterator<seq_store>  const_iterator;
    friend const_iterator;

    seq_store();
    seq_store(seq_store &&) = default;
    seq_store(const seq_store &) = default;
    inline seq_store &operator = (const seq_store &) = default;
    inline seq_store &operator = (seq_store &&) = default;

    // Total number of sequences stored with insert()
    inline size_t num_inserted_seq() const { return tabs[INSERTED].s_values.size();; }

    // Destroy the hash table and make this object immutable
    void freeze();

    // Print memory occupation statistics
    void print_memory_occupation() const;

    // retrieve back a sequence and store it in the given sparsevector
    void retrieve(const sequence_id id, Vec &sm) const;

    // access to a value by its index without unpacking the sparse vector
    inline value_type get_at(const sequence_id id, size_t place) const
    { return _get_at(INSERTED, id, place); }

    // get the length of a stored sequence
    inline size_t get_length(const sequence_id id) const
    { return _get_length(INSERTED, id); }

    // insert a new sparsevector, or return its sid if it does already exists
    inline sequence_id insert(vec_const_iterator it, size_t len)
    { return _insert(INSERTED, it, len); }

    inline sequence_id insert(const Vec &sm) { return insert(sm.begin(), sm.size()); }

    // Number of bytes occupied
    size_t num_bytes() const;

    // start reading a sequence
    inline const_iterator begin_seq(sequence_id sid) const
    { return _begin_seq(INSERTED, sid); }

    inline const_iterator end() const { return const_iterator(this); }

    size_t make_hash(const Vec &sm);

    bool exists(sequence_id &out_id, vec_const_iterator it, size_t len) const;

    void clear();

    void swap(seq_store &s);

protected:
    size_t _make_hash(vec_const_iterator &it, size_t len) const;

    // Test existance. returns the found id in out_it (if any)
    bool _exists(SeqTableKind stk, vec_const_iterator it, size_t len,
                 size_t &hash, sequence_id &out_id) const;

    value_type _get_at(SeqTableKind stk, const sequence_id id, size_t place) const;

    size_t _get_length(SeqTableKind stk, const sequence_id id) const;

    sequence_id _insert(SeqTableKind stk, vec_const_iterator &it, size_t len);

    void _retrieve(SeqTableKind stk, const sequence_id id, Vec &sm) const;

    size_t _recursive_rehash(SeqTableKind stk, const sequence_id sid, std::vector<bool> &hins);

    size_t _recompute_hash(SeqTableKind stk, const sequence_id sid) const;

    // Verify that [s1,s2) is equivalent to the sequence stored in id
    // in the index range [start, end)
    bool _check_equiv(SeqTableKind stk, const sequence_id id,
                      vec_const_iterator it, size_t len) const;

    inline const_iterator _begin_seq(SeqTableKind stk, sequence_id sid) const
    { return const_iterator(this, sid, stk); }
};

//=============================================================================
// Associative storage of index => sparse vectors with sharing
// Similar to a seq_store<> for sparse vectors.
//=============================================================================

template<typename SparseVec, typename Traits = sequence_traits<SparseVec> >
struct sparse_seq_store {
public:
    typedef typename Traits::table_type			table_type;
    typedef typename Traits::tuple_type    		tuple_type;
    typedef typename SparseVec::value_type     	value_type;
    typedef typename SparseVec::index_type      index_type;
    typedef typename SparseVec::const_iterator  sparsevec_const_iterator;
    typedef sequence_id_impl<SparseVec>         sequence_id;

protected:
    typedef table < col::uint_col < sequence_id, +1 >>  table_of_sequence_id;
    typedef table<col::uint_col<index_type>>   	   table_of_indexes;

    // seq_tables represents the column where the sequence datas are stored.
    // Each sequence is stored as a binary tree of records. There are two seq_tables.
    // The first record of each stored sequence, which is the one directly inserted
    // with insert(), is stored in the first table. All the other parts (the 'fragments')
    // are stored in the second table.
    //  For instance, the sequence:  0*P0, 10*P1, 20*P2, 30*P3, 40*P4, 50*P5, 60*P6
    // is stored as:
    //        tabs[INSERTED]      tabs[FRAGMENTS]
    //  index    3                   1,  0,  2,  5,  4,  6
    // values   30                  10,  0, 20, 50, 40, 60
    //   left    0                   1,  X,  X,  4,  X,  X
    //  right    3                   2,  X,  X,  5,  X,  X
    //
    // plus the hash chain informations. The left/right indices in the first table
    // refers to the fragment table, while left/right indices in the fragment table
    // refers to the same table.
    struct seq_tables {
        // Hash table
        table_of_sequence_id   hm_htab;	   // Heads of the hash table linked lists

        // Columnar representation of the stored data:
        table_of_sequence_id   s_left;	   // Left branches
        table_of_sequence_id   s_right;	   // Right branches
        table_of_indexes       s_index;	   // Stored indexes
        table_type             s_values;   // Stored values
        table_of_sequence_id   s_next;	   // Hash chain (next sid in hash table chain)

        inline sequence_id max_id() const { return s_index.size(); }
        inline size_t data_bytes() const {
            return (s_values.num_bytes() + s_index.num_bytes() +
                    s_left.num_bytes() + s_right.num_bytes());
        }
        inline size_t hash_bytes() const { return s_next.num_bytes() + hm_htab.num_bytes(); }
    };
    // Two level tables.
    // The first table contains the sequences insert()ed by the user of the class
    // The second table contains all the fragments generated by the first-level
    //   insertions.
    seq_tables tabs[NUM_SEQUENCE_TABLES];

    // The insert()ed empty element, if any. This exists only when an empty vector is
    // explicitly inserted in the first-level table. The fragment table does not have
    // any empty element.
    sequence_id EmptyElemId = sequence_id::INVALID_SEQ;

    // Maximum index size (size of sparse vector)
    index_type max_index;

    // Conversion of objects from/to tuples
    Traits conv;

    template<typename SeqStore> friend void print_memory_occupation(const SeqStore &s);

public:
    friend class stored_seq_const_iterator<sparse_seq_store>;
    class const_iterator : public stored_seq_const_iterator<sparse_seq_store> {
    public:
        typedef stored_seq_const_iterator<sparse_seq_store>  base;

        const_iterator(const sparse_seq_store *p) : base(p) {}
        const_iterator(const const_iterator &c)  : base(c) {}
        const_iterator(const sparse_seq_store *p, sequence_id sid, SeqTableKind stk)
        /**/ : base(p, sid, stk) {}
        inline const_iterator &operator = (const const_iterator &) = default;
        inline const_iterator &operator = (const_iterator &&) = default;

        inline index_type index() const {
            assert(this->cur_stk() == FRAGMENTS || this->cur_sid != this->pss->EmptyElemId);
            return this->pss->tabs[this->cur_stk()].s_index[size_t(this->cur_sid)];
        }
    };

    sparse_seq_store(index_type _max_index = index_type(0));
    sparse_seq_store(sparse_seq_store &&) = default;
    sparse_seq_store(const sparse_seq_store &) = default;
    inline sparse_seq_store &operator = (const sparse_seq_store &) = default;
    inline sparse_seq_store &operator = (sparse_seq_store &&) = default;

    // Total number of sequences stored with insert()
    inline size_t num_inserted_seq() const { return tabs[INSERTED].s_index.size(); }

    // size of the stored sparsevectors
    inline index_type size_of_vectors() const { return max_index; }

    // Destroy the hash table and make this object immutable
    void freeze();

    // print a simple representation of a stored sequence
    //void print(const sequence_id id) const;

    // Print memory occupation statistics
    void print_memory_occupation() const;

    // retrieve back a sequence and store it in the given sparsevector
    inline void retrieve(const sequence_id id, SparseVec &sm) const
    { sm.resize(size_of_vectors(), false); _retrieve(INSERTED, id, sm); }

    // access to a value by its index without unpacking the sparse vector
    inline value_type get_at(const sequence_id id, index_type place) const
    { return _get_at(INSERTED, id, place); }

    // insert a new sparsevector, or return its sid if it does already exists
    inline sequence_id insert(sparsevec_const_iterator s1, sparsevec_const_iterator s2)
    { size_t hash; return _insert(INSERTED, s1, s2, hash); }
    inline sequence_id insert(const SparseVec &sm) { return insert(sm.begin(), sm.end()); }

    // Number of bytes occupied
    size_t num_bytes() const;

    // start reading a sequence
    inline const_iterator begin_seq(sequence_id sid) const
    { return _begin_seq(INSERTED, sid); }

    inline const_iterator end() const { return const_iterator(this); }

    size_t make_hash(const SparseVec &sm);

    bool exists(sequence_id &out_id, sparsevec_const_iterator s1,
                sparsevec_const_iterator s2) const;

    void clear(index_type _max_index = index_type(0));

    void swap(sparse_seq_store &s);

protected:
    size_t make_hash(sparsevec_const_iterator s1,
                     sparsevec_const_iterator s2) const;
    // Test existance. returns the found id in out_it (if any)
    bool _exists(SeqTableKind stk, sparsevec_const_iterator s1,
                 sparsevec_const_iterator s2, size_t hash, sequence_id &out_id) const;

    value_type _get_at(SeqTableKind stk, const sequence_id id, index_type place) const;

    sequence_id _insert(SeqTableKind stk, sparsevec_const_iterator s1,
                        sparsevec_const_iterator s2, size_t &out_hash);

    void _retrieve(SeqTableKind stk, const sequence_id id, SparseVec &sm) const;

    size_t _recursive_rehash(SeqTableKind stk, const sequence_id sid, std::vector<bool> &hins);

    size_t _recompute_hash(SeqTableKind stk, const sequence_id sid) const;

    // Verify that [s1,s2) is equivalent to the sequence stored in id
    // in the index range [start, end)
    bool _check_equiv(SeqTableKind stk, const sequence_id id, sparsevec_const_iterator s1,
                      sparsevec_const_iterator s2) const;

    inline const_iterator _begin_seq(SeqTableKind stk, sequence_id sid) const
    { return const_iterator(this, sid, stk); }
};

//=============================================================================
#endif // __COMPACT_TABLE_H__
