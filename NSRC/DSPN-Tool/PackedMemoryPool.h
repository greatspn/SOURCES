/*
 *  PackedMemoryPool.h
 *
 *  Auxiliary (internal) data structures and methods for packed memory pools.
 *
 *  Created by Elvio Amparore
 *
 */

//=============================================================================
#ifndef __PACKED_MEMORY_POOL_H__
#define __PACKED_MEMORY_POOL_H__
//=============================================================================


//=============================================================================
//  Memory Pool interface
//=============================================================================

class raw_pool_alloc : boost::noncopyable {
public:
    virtual ~raw_pool_alloc() { }
    virtual uint8_t *allocate(size_t sz) = 0;
    virtual void deallocate(uint8_t *ptr, size_t sz) = 0;
    virtual size_t max_size() const = 0;
    virtual size_t get_count() const = 0;
    virtual size_t get_allocated_bytes() const = 0;
};

// Create a general purpose memory allocator from ::new and ::delete
raw_pool_alloc *CreateStdAllocator();

// Create a general purpose memory pool with our pool allocator interface
raw_pool_alloc *CreateRawPoolAllocator();

//=============================================================================
//  Equality test with forward iterators, testing for same-length sequences
//=============================================================================

// Equality test on the entire range of [first1, last1) and [first2, last2)
// If one of the two ranges is smaller, equal_safe() returns false.
template <class InputIterator1, class InputIterator2>
bool equal_safe(InputIterator1 first1, InputIterator1 last1,
                InputIterator2 first2, InputIterator2 last2) {
    while (first1 != last1 && first2 != last2) {
        if (*first1 != *first2)
            return false;
        ++first1; ++first2;
    }
    return (first1 == last1 && first2 == last2);
}

//=============================================================================
//  Suffix Table
//=============================================================================

// Forward declarations
template<class T, class I, class H> class suffix_table;
template<class T, class I, class H>
ostream &operator<< (ostream &os, const suffix_table<T, I, H> &st);

// A suffix table is an associative data structure that stores a sequence of T
// with a single key index of type I. Internally, a sequence of entry records
// are stored, one for each T value, to form a chain of entries.
template<class T, class I = size_t, class H = size_t>
class suffix_table : boost::noncopyable {
public:
    typedef T  value_type;
    typedef I  index_type;
    typedef H  hash_type;

    //-------------------------------------------------------------------------
    // Forward iterator of a stored sequence of T into the suffix_table
    //-------------------------------------------------------------------------
    class const_seq_iterator : public std::iterator<forward_iterator_tag, T> {
    public:
        typedef const_seq_iterator  cst_iter;
        inline const_seq_iterator() : pst(nullptr), i(END_SET) {}
        inline const_seq_iterator(const suffix_table<T, I, H> *_pst, index_type _i)
            : pst(_pst), i(_i) { }
        inline const_seq_iterator(const cst_iter &c)
            : pst(c.pst), i(c.i) { }

        T operator*() const  	 	{  no_end(); return pst->set_tbl[i].value;  }
        const T *operator->() const {  no_end(); return &(pst->set_tbl[i].value);  }
        I index() const		 	 	{  return i;  }

        cst_iter &operator++()   {
            no_end();  i = pst->set_tbl[i].next;  return *this;
        }
        cst_iter operator++(int) {
            no_end();  cst_iter ci = *this;  i = pst->set_tbl[i].next;  return ci;
        }
        bool operator==(const cst_iter &c) const  {  return i == c.i && pst == c.pst;  }
        bool operator!=(const cst_iter &c) const  {  return i != c.i || pst != c.pst;  }

    protected:
        inline void no_end() const  {  assert(i != END_SET);  }

        const suffix_table<T, I, H> *pst;	///< The suffix table pointer
        index_type i;					///< Current entry position
    };

    // The empty sequence has the END_SET index number
    static const I END_SET;

    // Get the index_type for the specified sequence [beg, end), by inserting
    // all the entries needed.
    template<class It>
    index_type insert(It first, It last) {
        index_type i = END_SET;
        hash_type h = 0;
        It it = last;
        while (it != first) {
            --it;
            h = combine_hash(*it, h);
            index_type suffix = i;
            i = lookup_head_suffix(*it, suffix, h);
            if (i == END_SET)
                i = insert_head_suffix(*it, suffix, h);
        }
        assert(equal_safe(first, last, begin_seq(i), end()));
        return i;
    }

    template<class Vec>
    inline index_type insert(const Vec &vec) {
        return insert(vec.begin(), vec.end());
    }

    // Get the index_type of a new sequence made by an head entry
    // prepended to an already inserted suffix
    index_type prepend(value_type head, index_type suffix) {
        assert(suffix == END_SET || suffix < index_type(set_tbl.size()));
        hash_type suffix_hash = get_hash(suffix);
        hash_type h = combine_hash(head, suffix_hash);
        index_type i = lookup_head_suffix(head, suffix, h);
        if (i == END_SET) {
            i = insert_head_suffix(head, suffix, h);
        }
        assert(i == lookup_head_suffix(head, suffix, get_hash(i)));
        return i;
    }

    // Get back the sequence of data indexed by i, into the back_inserter bi
    template<class BackInserter>
    inline void get(index_type i, BackInserter bi) const {
        std::copy(begin_seq(i), end(), bi);
    }

    // Get the sequence length of the entry chain i
    inline size_t get_length(index_type i) const {
        return std::distance(begin_seq(i), end());
    }

    // Get the number of stored entries, which is the number of unique inserted
    // sequences PLUS all their suffixes. Note that this is not neither the
    // number of insert() calls, nor the number of unique sequences insert()ed.
    inline size_t size() const {
        return set_tbl.size();
    }

    // End iterator of any stored sequence
    const_seq_iterator end() const {
        return const_seq_iterator(this, END_SET);
    }

    // Forward iterator for a stored sequence
    const_seq_iterator begin_seq(index_type i) const {
        return const_seq_iterator(this, i);
    }

protected:
    struct entry {
        inline entry(T v, I n) : value(v), next(n) { }
        T	value;	///< Stored value
        I	next;	///< Next entry in the chain, or END_SET
    };

    typedef std::unordered_multimap<H, I>  hash_table_t;
    typedef typename hash_table_t::const_iterator const_hashtbl_iterator;
    typedef std::pair<const_hashtbl_iterator, const_hashtbl_iterator> const_hashrange_t;

    vector<entry> 	 set_tbl;	///< Contains all the stored entries
    hash_table_t	 hash_tbl;	///< Fast lookup hash
    std::hash<T> 	 hasher;	///< Compute hash values out of T values

    // Search for an exact suffix in the set_tbl. The suffix has the specified
    // length, hash value, and [first, last) value sequence.
    template<class It>
    index_type find_exact_suffix(It first, It last, hash_type h, size_t len) const {
        assert(len > 0);
        const_hashrange_t range = hash_tbl.equal_range(h);
        while (range.first != range.second) {
            // Check if the sequence in *range.first is the same of [first, last)
            if (equal_safe(first, last, begin_seq(*range.first), end()))
                return *range.first;
            ++range.first;
        }
        return END_SET;
    }

    // Compute the hash value of an entry chain
    inline hash_type get_hash(index_type i) const {
        if (i == END_SET)
            return 0;
        return combine_hash(set_tbl[i].value, get_hash(set_tbl[i].next));
    }

    // Hash value combination function
    inline hash_type combine_hash(T head, hash_type tail) const {
        hash_type h = tail;
        // rotate hash by ROL_N bits
        const size_t ROL_N = 7;
        h = (h << ROL_N) | (h >> (sizeof(hash_type) * CHAR_BIT - ROL_N));
        // xor with value
        return (h ^ hash_type(hasher(head)));
    }

    // Search for an entry chain that starts with the specified head value, and
    // continues with a tail chain. h is the hash value of [head; tail] chain
    inline index_type lookup_head_suffix(T head, index_type suffix, hash_type h) const {
        const_hashrange_t range = hash_tbl.equal_range(h);
        for (; range.first != range.second; ++range.first) {
            index_type i = range.first->second;
            if (set_tbl[i].value == head && set_tbl[i].next == suffix)
                return i;
        }
        return END_SET;  // Not found
    }

    // Insert a new entry made of an head value and an already stored tail entry
    inline index_type insert_head_suffix(T head, index_type suffix, hash_type h) {
        assert(suffix == END_SET || suffix < index_type(set_tbl.size()));
        assert(END_SET == lookup_head_suffix(head, suffix, h));
        index_type i = index_type(set_tbl.size());
        set_tbl.push_back(entry(head, suffix));
        hash_tbl.insert(make_pair(h, i));
        assert(h == get_hash(i));
        return i;
    }

    friend ostream &operator<< <T, I, H> (ostream &os, const suffix_table<T, I, H> &st);
};

template<class T, class I, class H> const I suffix_table<T, I, H>::END_SET = I(-1);


template<class T, class I, class H>
ostream &operator<< (ostream &os, const suffix_table<T, I, H> &st) {
    os << "set_tbl[" << st.set_tbl.size() << "]:\n";
    for (size_t i = 0; i < st.set_tbl.size(); i++) {
        os << "  " << setw(2) << i << ": v=" << left << setw(3) << st.set_tbl[i].value << " next=";
        if (st.set_tbl[i].next == suffix_table<T, I, H>::END_SET)
            os << "END\n";
        else os << st.set_tbl[i].next << "\n";
    }
    os << "hash_tbl[" << st.hash_tbl.size() << "]\n";
    typename suffix_table<T, I, H>::const_hashtbl_iterator it;
    for (it = st.hash_tbl.begin(); it != st.hash_tbl.end(); ++it) {
        os << "  " << setw(2) << it->second << "  hash=" << it->first << "\n";
    }
    return os;
}


//=============================================================================
//  Segmented vector<T>
//=============================================================================

// A segmented vector is just a two-level vector: the first level is a
// variable-size vector, and the second level is made of vectors of the
// specified fixed size SZ. The sgm_vector class is designed to mimic
// the interface of the std::vector class.
template<class T, size_t SZ, class Alloc = std::allocator<T> >
class sgm_vector {
public:
    typedef T					 				value_type;
    typedef typename Alloc::pointer             pointer;
    typedef typename Alloc::const_pointer       const_pointer;
    typedef typename Alloc::reference           reference;
    typedef typename Alloc::const_reference     const_reference;
    typedef size_t								size_type;
    typedef ptrdiff_t							difference_type;
    typedef Alloc								allocator_type;

    class iterator : public std::iterator<random_access_iterator_tag, T> {
    public:
        typedef iterator  iter;
        inline iterator() : k(0), p(nullptr) { }
        inline iterator(size_t _k, sgm_vector *_p) : k(_k), p(_p) { }
        inline iterator(const iter &i) : k(i.k), p(i.p) { }

        reference operator*() const { is_ok(); return p->at(k); }
        pointer operator->() const  { is_ok(); return &(p->at(k)); }
        reference operator[](difference_type d) const
        {   is_ok(); return p->at(k);   }

        iter &operator++()   { ++k; return *this;}
        iter operator++(int) { iter i(*this); ++k; return i; }
        iter &operator--()   { --k; return *this;}
        iter operator--(int) { iter i(*this); --k; return i; }

        iter operator+(difference_type d) const { return iterator(k + d, p); }
        iter &operator+=(difference_type d)     { k += d; return *this; }
        iter operator-(difference_type d) const { return iterator(k - d, p); }
        iter &operator-=(difference_type d)     { k -= d; return *this; }

        bool operator==(const iter &i) const { assert(i.p == p); return k == i.k; }
        bool operator!=(const iter &i) const { assert(i.p == p); return k != i.k; }
        bool operator<=(const iter &i) const { assert(i.p == p); return k <= i.k; }
        bool operator>=(const iter &i) const { assert(i.p == p); return k >= i.k; }
        bool operator<(const iter &i) const  { assert(i.p == p); return k < i.k; }
        bool operator>(const iter &i) const  { assert(i.p == p); return k > i.k; }

        template<class It> difference_type operator-(const It &i) const
        {  return difference_type(k) - difference_type(i.index());  }

        inline size_t index() const   { return k; }
    protected:
        inline void is_ok() const 	  { assert(p != nullptr); }
        size_t  		k;
        sgm_vector 	*p;
    };

    class const_iterator : public std::iterator<random_access_iterator_tag, const T> {
    public:
        typedef const_iterator  iter;
        inline const_iterator() : k(0), p(nullptr) { }
        inline const_iterator(size_t _k, sgm_vector const *_p) : k(_k), p(_p) { }
        inline const_iterator(const const_iterator &i) : k(i.k), p(i.p) { }
        inline const_iterator(const iterator &i) : k(i.k), p(i.p) { }

        const_reference operator*() const { is_ok(); return p->at(k); }
        const_pointer operator->() const  { is_ok(); return &(p->at(k)); }
        const_reference operator[](difference_type d) const
        {   is_ok(); return p->at(k);   }

        iter &operator++()   { ++k; return *this;}
        iter operator++(int) { iter i(*this); ++k; return i; }
        iter &operator--()   { --k; return *this;}
        iter operator--(int) { iter i(*this); --k; return i; }

        iter operator+(difference_type d) const { return iterator(k + d, p); }
        iter &operator+=(difference_type d)     { k += d; return *this; }
        iter operator-(difference_type d) const { return iterator(k - d, p); }
        iter &operator-=(difference_type d)     { k -= d; return *this; }

        bool operator==(const iter &i) const { assert(i.p == p); return k == i.k; }
        bool operator!=(const iter &i) const { assert(i.p == p); return k != i.k; }
        bool operator<=(const iter &i) const { assert(i.p == p); return k <= i.k; }
        bool operator>=(const iter &i) const { assert(i.p == p); return k >= i.k; }
        bool operator<(const iter &i) const  { assert(i.p == p); return k < i.k; }
        bool operator>(const iter &i) const  { assert(i.p == p); return k > i.k; }

        template<class It> difference_type operator-(const It &i) const
        {  return difference_type(k) - difference_type(i.index());  }

        inline size_t index() const   { return k; }
    protected:
        inline void is_ok() const 	{ assert(p != nullptr); }
        size_t  			k;
        sgm_vector const 	*p;
    };

    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    typedef std::reverse_iterator<iterator>		    reverse_iterator;

    sgm_vector() : sz(0) { }
    sgm_vector(size_type sz, const value_type &x) : sz(0)  { resize(sz, x); }
    sgm_vector(const sgm_vector &s) : sz(s.sz), vec(s.vec) { }

    inline size_type size() const 	{  return sz;  }

    inline iterator begin() 	{  return iterator(0, this); }
    inline iterator end()		{  return iterator(size(), this); }
    inline const_iterator begin() const {  return const_iterator(0, this); }
    inline const_iterator end()	const	{  return const_iterator(size(), this); }

    inline reverse_iterator rbegin()    { return reverse_iterator(end()); }
    inline reverse_iterator rend()      { return reverse_iterator(begin()); }
    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    inline const_reverse_iterator rend() const   { return const_reverse_iterator(begin()); }

    sgm_vector &operator=(const sgm_vector &s) {  vec = s.vec;  sz = s.sz; 	}

    void resize(size_type nsz, const value_type &filler) {
        if (nsz < size())
            erase(begin() + nsz, end());
        else insert(end(), nsz - size(), filler);
    }

    void resize(size_type nsz) {  resize(nsz, value_type());  }

    bool empty() const    {  return sz == 0;  }

    reference at(size_type k)
    {  assert(k < sz);  return vec.at(k / SZ).at(k % SZ);  }

    const_reference at(size_type k) const
    {  assert(k < sz);  return vec.at(k / SZ).at(k % SZ);  }

    reference operator[](size_type k)			   {  return at(k);  }
    const_reference operator[](size_type k) const  {  return at(k);  }

    reference front()				{  return *begin();  }
    const_reference front() const	{  return *begin();  }
    reference back()				{  return *(end() - 1);  }
    const_reference back() const	{  return *(end() - 1);  }

    void assign(size_type num, const value_type &filler) {
        clear();  resize(num, filler);
    }

    /*template<typename InputIterator>
    void assign(InputIterator first, InputIterator last);*/

    size_type capacity() const {
        return SZ * vec.capacity();
    }

    void reserve(size_type cap) {
        vec.reserve((cap + SZ - 1) / SZ);
    }

    void push_back(const value_type &x) {
        insert(end(), x);
    }

    void pop_back() {
        erase(end() - difference_type(1));
    }

    iterator insert(iterator start_pos, const value_type &x) {
        insert(start_pos, 1, x);
        return start_pos;
    }

    void insert(iterator start_pos, size_type num, const value_type &x) {
        set_size(size() + num);
        copy_backward(start_pos, end() - difference_type(num), end());
        fill(start_pos, start_pos + num, x);
    }

    /*template<typename InputIterator>
    void insert(iterator start_pos, InputIterator first, InputIterator last);*/

    iterator erase(iterator start_pos) {
        erase(start_pos, start_pos + 1);
        return start_pos;
    }

    iterator erase(iterator start_pos, iterator end_pos) {
        assert(start_pos <= end_pos && end_pos <= end());
        copy(end_pos, end(), start_pos);
        set_size(size() - (end_pos - start_pos));
        return start_pos;
    }

    void swap(sgm_vector &s) {
        vec.swap(s.vec);
        swap(sz, s.sz);
    }

    void clear()		{  erase(begin(), end());  }

    size_type max_size() const {  return size_type(-1) / sizeof(T);  }

    inline bool operator==(const sgm_vector &s) const
    {  return sz == s.sz && vec == s.vec;  }
    inline bool operator<(const sgm_vector &s) const
    {  return std::lexicographical_compare(begin(), end(), s.begin(), s.end());  }

    inline bool operator!=(const sgm_vector &s) const   {  return !(s == *this);  }
    inline bool operator>(const sgm_vector &s) const	{  return (s < *this);  }
    inline bool operator<=(const sgm_vector &s) const	{  return !(s < *this);  }
    inline bool operator>=(const sgm_vector &s) const	{  return !(*this < s);  }

protected:
    vector< vector<T, Alloc> > 	vec;
    size_t  					sz;

    void set_size(size_t nsz) {
        assert(nsz < max_size());
        size_t nblocks = (nsz + SZ - 1) / SZ, currblocks = vec.size();
        if (nblocks != vec.size() && nblocks > 0) {
            if (nblocks > vec.capacity()) {
                vector< vector<T, Alloc> > nvec;
                nvec.reserve(nblocks * 4);
                nvec.resize(nblocks);
                for (size_t b = 0; b < currblocks; b++)
                    nvec[b].swap(vec[b]);
                vec.swap(nvec);
            }
            else {
                vec.resize(nblocks);
            }
            for (size_t b = currblocks; b < nblocks - 1; b++) {
                vec[b].reserve(SZ);
                vec[b].resize(min(SZ, nsz - b * SZ));
            }
            // reserve last block size
            vec[nblocks - 1].reserve(SZ);
            vec[nblocks - 1].resize(min(SZ, nsz - (nblocks - 1)*SZ));
        }
        // resize the last block
        if (nsz > 0)
            vec[nblocks - 1].resize(min(SZ, nsz - (nblocks - 1)*SZ));
        else
            vec.resize(0);
        sz = nsz;

#ifndef NDEBUG
        size_t tsz = 0;
        for (size_t b = 0; b < vec.size(); b++) {
            assert(b == vec.size() - 1 || vec[b].size() == SZ);
            tsz += vec[b].size();
        }
        assert(tsz == sz);
#endif
    }
};

//=============================================================================
//   Packing/unpacking procedures
//=============================================================================

//#define PACK_DEBUG

#ifdef PACK_DEBUG
template<class T>
inline void DBG_PACK(const vector<uint8_t> &cbuf, T val, const char *type) {
    cout << "  #PACK " << type << " " << setw(6) << left << val << "   buf=<";
    for (size_t i = 0; i < cbuf.size(); i++)
        cout << (i == 0 ? "" : " ") << hex << int(cbuf[i]) << dec;
    cout << ">" << endl;
}

template<class const_iterator, class T>
inline void DBG_UNPACK(const_iterator p, T val, const char *type) {
    cout << "  #UNPACK " << type << " " << setw(4) << left << val << "   buf=<";
    size_t m_cnt = 0;
    while (*p != 0)
        cout << (m_cnt++ == 0 ? "" : " ") << hex << int(*p++) << dec;
    cout << ">" << endl;
}
#else
#define DBG_PACK(cbuf, val, type)
#define DBG_UNPACK(p, val, type)
#endif

//=============================================================================

static const size_t PACKENC_BASE = 1;

// appends the value v to the compression buffer with a simple byte encoding
inline void pack_into_buffer(vector<uint8_t> &cbuf, unsigned long val) {
    // Format: < 2^7     0xxxxxxx
    //         < 2^14    1xxxxxxx 0xxxxxxx
    //         < 2^21    1xxxxxxx 1xxxxxxx 0xxxxxxx
    // and so on. No byte in the packed sequence is a zero.
    size_t v = val;
    v += PACKENC_BASE;
    assert(v != 0);
    while (v > 0x7f) {
        cbuf.push_back(0x80 | uint8_t(v & 0x7f));
        v >>= 7;
    }
    assert(v != 0);
    cbuf.push_back(uint8_t(v));
    DBG_PACK(cbuf, val, "size_t ");
}

// appends the value v to the compression buffer with a simple byte encoding
inline void pack_into_buffer(vector<uint8_t> &cbuf, long val) {
    // Similar to the size_t version, but the first byte has 6 significant bit,
    // a continuation bit (0x80) and a sign bit (0x40 = positive)
    // Again, no byte in the packed sequence is a zero.
    ssize_t v = val;
    if (v >= 0) {
        uint8_t mask0 = (v > 0x3f ? 0x80 : 0);
        cbuf.push_back(0x40 | mask0 | uint8_t(v & 0x3f));
    }
    else {
        v = -v;
        uint8_t mask0 = (v > 0x3f ? 0x80 : 0);
        cbuf.push_back(mask0 | uint8_t(v & 0x3f));
    }
    v >>= 6;
    while (v > 0) {
        uint8_t mask0 = (v > 0x7f ? 0x80 : 0);
        cbuf.push_back(mask0 | uint8_t(v & 0x7f));
        v >>= 7;
    }
    DBG_PACK(cbuf, val, "ssize_t");
}

// pack an int value
inline void pack_into_buffer(vector<uint8_t> &cbuf, int v) {
    pack_into_buffer(cbuf, (long)v);
}

// pack an unsigned value
inline void pack_into_buffer(vector<uint8_t> &cbuf, unsigned int v) {
    pack_into_buffer(cbuf, (unsigned long)v);
}

// pack a generic sparsevector<Tag> object
template<class Tag>
static inline void
pack_into_buffer(vector<uint8_t> &cbuf, const sparsevector<Tag> &svec) {
    pack_into_buffer(cbuf, svec.nonzeros());
    typename sparsevector<Tag>::const_iterator it;
    for (it = svec.begin(); it != svec.end(); ++it) {
        pack_into_buffer(cbuf, it->index);
        pack_into_buffer(cbuf, it->value);
    }
}

// pack an std::vector<T,A> object
template<class T, class A>
static inline void
pack_into_buffer(vector<uint8_t> &cbuf, const std::vector<T, A> &vec) {
    pack_into_buffer(cbuf, vec.size());
    typename std::vector<T, A>::const_iterator it;
    for (it = vec.begin(); it != vec.end(); ++it) {
        pack_into_buffer(cbuf, *it);
    }
}

// pack an std::set<K,Cmp,A> object
template<class K, class Cmp, class A>
static inline void
pack_into_buffer(vector<uint8_t> &cbuf, const std::set<K, Cmp, A> &s) {
    pack_into_buffer(cbuf, s.size());
    typename std::set<K, Cmp, A>::const_iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        pack_into_buffer(cbuf, *it);
    }
}

// pack a private_integer<T,U> type
template<class T, class U>
static inline void
pack_into_buffer(vector<uint8_t> &cbuf, const private_integer<T, U> pi) {
    pack_into_buffer(cbuf, pi.get());
}


//=============================================================================

// reads back a packed value (assuming a 0-terminated unit8_t sequence)
// NOTE: the iterator is modified and returned back incremented.
template<class const_iterator>
inline void unpack_from_buffer(const_iterator &p, unsigned long &v) {
    size_t offset = 0;
    v = 0;
    while (*p & 0x80) {
        v |= (*p++ & 0x7f) << offset;
        offset += 7;
    }
    assert(*p != 0);
    v |= (*p++) << offset;
    assert(v != 0);
    v -= PACKENC_BASE;
    DBG_UNPACK(p, v, "size_t ");
}

template<class const_iterator>
inline void unpack_from_buffer(const_iterator &p, long &v) {
    uint8_t b1 = *p++;
    bool negative = (b1 & 0x40) == 0;
    v = (b1 & 0x3f);
    size_t tail = 6;
    while (b1 & 0x80) {
        b1 = *p++;
        v |= (b1 & 0x7f) << tail;
        tail += 7;
    }
    if (negative)
        v = -v;
    DBG_UNPACK(p, v, "ssize_t");
}

// unpack an int
template<class const_iterator>
inline void unpack_from_buffer(const_iterator &p, int &v) {
    long val;
    unpack_from_buffer(p, val);
    v = val;
}

// unpack an unsigned
template<class const_iterator>
inline void unpack_from_buffer(const_iterator &p, unsigned int &v) {
    unsigned long val;
    unpack_from_buffer(p, val);
    v = val;
}

// unpack a sparsevector<> object from packed sequence, and modifies the
// buffer pointer. The size of the sparsevector has to be given explicitly.
template<class const_iterator, class Tag>
static inline void
unpack_from_buffer(const_iterator &p, sparsevector<Tag> &svec, size_t sz) {
    size_t nonzeroes;
    unpack_from_buffer(p, nonzeroes);
    if (svec.capacity() < nonzeroes)
        svec.reserve(nonzeroes);
    svec.resize(sz);
    for (size_t i = 0; i < nonzeroes; i++) {
        typename Tag::index_type index;
        typename Tag::value_type value;
        unpack_from_buffer(p, index);
        unpack_from_buffer(p, value);
        svec.insert_element(index, value);
    }
    svec.verify_invariant();
}

// unpack an std::vector<> object from a packed sequence
template<class const_iterator, class T, class A>
static inline void
unpack_from_buffer(const_iterator &p, std::vector<T, A> &vec) {
    size_t sz, val;
    unpack_from_buffer(p, sz);
    vec.resize(sz);
    for (size_t i = 0; i < sz; i++) {
        unpack_from_buffer(p, val);
        vec[i] = val;
    }
}

// unpack an std::set<> object from a packed sequence
template<class const_iterator, class K, class Cmp, class A>
static inline void
unpack_from_buffer(const_iterator &p, std::set<K, Cmp, A> &s) {
    size_t sz, val;
    unpack_from_buffer(p, sz);
    for (size_t i = 0; i < sz; i++) {
        unpack_from_buffer(p, val);
        s.insert(val);
    }
}

// unpack a private_integer<T,U> type
template<class const_iterator, class T, class U>
static inline void
unpack_from_buffer(const_iterator &p, private_integer<T, U> &pi) {
    T val;
    unpack_from_buffer(p, val);
    pi.set(val);
}

//=============================================================================











//=============================================================================
#endif   // __PACKED_MEMORY_POOL_H__
