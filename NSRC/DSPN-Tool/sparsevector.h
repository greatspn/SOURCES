/*
 *  sparsevector.h
 *  Sparse Vector ordered by an integer index element
 *
 *  Created by Elvio Amparore
 *
 */

//=============================================================================
#ifndef __SPARSE_VECTOR_H__
#define __SPARSE_VECTOR_H__
//=============================================================================

template <class Iterator, class T, class Compare>
Iterator my_lower_bound(Iterator first, Iterator last,
                        const T &value, Compare comp) {
    ptrdiff_t len = last - first;

    while (len > 0) {
        ptrdiff_t half = len >> 1;
        Iterator middle = first + half;
        if (comp(*middle, value)) {
            first = middle + 1;
            len = len - half - 1;
        }
        else
            len = half;
    }
    return first;
}

template <class Iterator, class T, class Compare>
Iterator my_upper_bound(Iterator first, Iterator last,
                        const T &value, Compare comp) {
    ptrdiff_t len = last - first;

    while (len > 0) {
        ptrdiff_t half = len >> 1;
        Iterator middle = first + half;
        if (comp(value, *middle))
            len = half;
        else {
            first = middle + 1;
            len = len - half - 1;
        }
    }
    return first;
}

//=============================================================================

/*
struct spvec_tag {
    typedef  size_t    index_type;
    typedef  double    value_type;
    typedef  base_index_value_pair<index_type, value_type>  index_value_pair;

    static inline bool allow_zeros() const { return true or false; }
};
*/

//=============================================================================

template<class IND, class VAL>
struct base_index_value_pair {
    typedef IND  index_type;
    typedef VAL  value_type;   

    inline base_index_value_pair() { }
    inline base_index_value_pair(const index_type i, const value_type v)
    /**/ : index(i), value(v) {}
    inline base_index_value_pair(const base_index_value_pair&) = default;
    inline base_index_value_pair(base_index_value_pair&&) = default;
    inline base_index_value_pair& operator=(const base_index_value_pair&) = default;
    inline base_index_value_pair& operator=(base_index_value_pair&&) = default;


    inline bool operator < (const base_index_value_pair &ivp) const
    { return (index < ivp.index) || (index == ivp.index && value < ivp.value); }

    inline bool operator == (const base_index_value_pair &ivp) const
    { return (index == ivp.index && value == ivp.value); }

    inline bool operator != (const base_index_value_pair &ivp) const
    { return (index != ivp.index || value != ivp.value); }

    index_type index;
    value_type value;
};

//=============================================================================

template<class Tag>
class sparsevector {
public:    
    typedef typename Tag::index_type  index_type;
    typedef typename Tag::value_type  value_type;
    typedef typename Tag::index_value_pair  index_value_pair;

    typedef typename std::vector<index_value_pair>  storage_type;
    typedef typename storage_type::iterator		  iterator;
    typedef typename storage_type::const_iterator const_iterator;

    // comparator for  index_value_pair <> index_type
    struct less_by_index : std::binary_function <index_value_pair, index_type, bool> {
        inline bool operator()(const index_value_pair &x, const index_type &y) const
        { return x.index < y; }
        inline bool operator()(const index_type &x, const index_value_pair &y) const
        { return x < y.index; }
    };

    // Internal invariants check
    inline void dbg_check() const {
#ifndef NDEBUG
#ifndef NO_SPARSE_VECTOR_VERIFY_INVARIANT
        for (size_t i = 0; i < spvec.size(); i++) {
            assert(spvec[i].index >= index_type(0) && spvec[i].index < size());
            assert(Tag::allow_zeros() || spvec[i].value != value_type(0));
        }
        for (size_t i = 1; i < spvec.size(); i++) {
            assert(spvec[i - 1].index < spvec[i].index);
        }
#endif
#endif
    }

    // Manually verify invariants
    inline void verify_invariants() const {
#ifndef NDEBUG
        for (size_t i = 0; i < spvec.size(); i++) {
            assert(spvec[i].index >= index_type(0) && spvec[i].index < size());
            assert(Tag::allow_zeros() || spvec[i].value != value_type(0));
        }
        for (size_t i = 1; i < spvec.size(); i++) {
            assert(spvec[i - 1].index < spvec[i].index);
        }
#endif
    }

    inline sparsevector() : maxIndex(0) {}
    inline sparsevector(size_t mi) : maxIndex(mi) {}
    inline sparsevector(size_t mi, storage_type&& s) : maxIndex(mi), spvec(s) {}
    inline ~sparsevector() {}
    inline sparsevector(const sparsevector&) = default;
    inline sparsevector(sparsevector&&) = default;
    inline sparsevector& operator=(const sparsevector&) = default;
    inline sparsevector& operator=(sparsevector&&) = default;

    inline void set_element(const index_type ind, const value_type val) {
        assert(ind >= index_type(0) && ind < size());
        less_by_index comp;
        iterator it = my_lower_bound(spvec.begin(), spvec.end(), ind, comp);
        assert(it != spvec.end() || spvec.empty() || ind > spvec.back().index);
        if (it == spvec.end()) // does not exists yet, insert it
            spvec.insert(it, index_value_pair(ind, val));
        else // replace the old value with the new one
            it->value = val;
        dbg_check();
    }

    inline void insert_element(const index_type ind, const value_type val) {
        assert(ind >= index_type(0) && ind < size());
        less_by_index comp;
        iterator it = my_lower_bound(spvec.begin(), spvec.end(), ind, comp);
        assert(it == spvec.end() || it->index > ind);
        spvec.insert(it, index_value_pair(ind, val));
        dbg_check();
    }

    inline value_type add_element(const index_type ind, value_type val) {
        assert(ind >= index_type(0) && ind < size());
        less_by_index comp;
        iterator it = my_lower_bound(spvec.begin(), spvec.end(), ind, comp);
        assert(it != spvec.end() || spvec.empty() || ind > spvec.back().index);
        if (it == spvec.end() || it->index != ind)
            spvec.insert(it, index_value_pair(ind, val));
        else {
            val += it->value;
            if (val != value_type(0))
                it->value = val;
            else
                spvec.erase(it);
        }
        dbg_check();
        return val;
    }

    inline void set_element_0(const index_type ind, const value_type val) 
    { if (val != value_type(0))  set_element(ind, val); }
    inline void insert_element_0(const index_type ind, const value_type val) 
    { if (val != value_type(0))  insert_element(ind, val); }
    // inline void add_element_0(const index_type ind, const value_type val) 
    // { if (val != value_type(0))  add_element(ind, val); }


    inline void add_vector(const sparsevector<Tag> &v2, bool sign = 1) {
        assert(size() == v2.size());
        for (const_iterator it2 = v2.begin(); it2 != v2.end(); ++it2) {
            add_element(it2->index, sign == 1 ? it2->value : -(it2->value));
        }
        dbg_check();
    }

    inline size_t nonzeros() const { return spvec.size(); }

    inline const index_value_pair& ith_nonzero(size_t ith) const { return spvec.at(ith); }
    inline index_value_pair& ith_nonzero(size_t ith) { return spvec.at(ith); }
    inline index_value_pair front_nonzero() const { assert(nonzeros()>0); return spvec.front(); }
    inline index_value_pair back_nonzero() const { assert(nonzeros()>0); return spvec.back(); }
    // Index of the first/last entries
    inline index_type leading() const { assert(nonzeros()>0); return spvec.front().index; }
    inline index_type trailing() const { assert(nonzeros()>0); return spvec.back().index; }

    inline index_type size() const { return maxIndex; }

    inline void set_nnz_value(size_t ith, value_type v) {
        spvec[ith].value = v;
        dbg_check();
    }

    inline void set_nnz_index(size_t ith, index_type ind) {
        assert(ind >= index_type(0) && ind < size());
        spvec[ith].index = ind;
        dbg_check();
    }


    // FIXME: does not truncate values with indexes >= ns...
    inline void resize(size_t ns, bool preserve = true) {
        maxIndex = ns;
        if (!preserve)
            spvec.clear();
        dbg_check();
    }

    inline void clear() { spvec.clear(); }

    inline void reserve(size_t sz) { spvec.reserve(sz); }
    inline size_t capacity() const { return spvec.capacity(); }
    inline bool empty() const { return spvec.empty(); }

    inline const_iterator begin() const { return spvec.begin(); }
    inline const_iterator end() const { return spvec.end(); }

    inline iterator begin() { return spvec.begin(); }
    inline iterator end() { return spvec.end(); }

    inline value_type operator [](const index_type ind) const {
        assert(ind >= index_type(0) && ind < size());
        less_by_index comp;
        const_iterator it = my_lower_bound(spvec.begin(), spvec.end(),
                                           ind, comp);
        if (it == spvec.end() || it->index != ind)
            return value_type(0);
        else return it->value;
    }

    // Closest non-zero to the specified index
    inline index_type lower_bound_nnz(const index_type ind) const {
        assert(ind >= index_type(0) && ind < size());
        less_by_index comp;
        const_iterator it = my_lower_bound(spvec.begin(), spvec.end(), ind, comp);
        if (it == spvec.end())
            return -1;
        return it - spvec.begin();
    }
    inline index_type upper_bound_nnz(const index_type ind) const {
        assert(ind >= index_type(0) && ind < size());
        less_by_index comp;
        const_iterator it = my_upper_bound(spvec.begin(), spvec.end(), ind, comp);
        if (it == spvec.end())
            return -1;
        return it - spvec.begin();
    }

    // inline value_type& operator [] (const index_type ind) {
    // 	assert(ind >= index_type(0) && ind < size());
    // 	less_by_index comp;
    // 	iterator it = my_lower_bound(spvec.begin(), spvec.end(),
    // 								 ind, comp);
    // 	if (it == spvec.end() || it->index != ind) {
    // 		iterator it2 = spvec.insert(it, index_value_pair(ind, value_type(0)));
    // 		dbg_check(); // BUG: cannot verify() since we have just added a 0 value.
    // 		return it2->value;
    // 	}
    // 	else return it->value;
    // }

    inline void erase_zeros() {
        index_type j(0), i(0);
        while (i < spvec.size()) {
            if (spvec[i].value != value_type(0))
                spvec[j++].value = spvec[i++].value;
            else
                i++;
        }
        spvec.resize(j);
    }

    // Low-level access to the storage
    storage_type& data() { return spvec; }
    const storage_type& data() const { return spvec; }

    inline bool operator ==
    (const sparsevector<Tag> &v2) const
    {	return (maxIndex == v2.maxIndex) && (spvec == v2.spvec);   }

    inline bool operator !=
    (const sparsevector<Tag> &v2) const
    {	return (maxIndex != v2.maxIndex) || (spvec != v2.spvec);   }


    inline bool operator <
    (const sparsevector<Tag> &v2) const {
        assert(maxIndex == v2.maxIndex);
        const_iterator it1 = begin(), it1end = end();
        const_iterator it2 = v2.begin(), it2end = v2.end();
        return lexicographical_compare(it1, it1end, it2, it2end);
    }

    inline sparsevector<Tag> &operator +=
    (const sparsevector<Tag> &v2) {
        add_vector(v2, 1);
        return *this;
    }

    inline sparsevector<Tag> &operator -=
    (const sparsevector<Tag> &v2) {
        add_vector(v2, -1);
        return *this;
    }

    inline void swap(sparsevector<Tag>& s) {
        spvec.swap(s.spvec);
        std::swap(maxIndex, s.maxIndex);
    }

private:
    storage_type    spvec;
    index_type		maxIndex;
};



/*template<class ostream_t, class IND, class VAL>
inline ostream_t& operator << (ostream_t& os, const sparsevector<IND,VAL>& sv) {
	typename sparsevector<IND,VAL>::const_iterator it;
	os << "[";
	size_t cnt = 0;
	for (it = sv.begin(); it != sv.end(); ++it) {
		os << (cnt++ > 0 ? ", " : " ");
		if (it->value != 1)
			os << it->value << " @ ";
		os << "I"<<it->index;
	}
	os << " ]";
	return os;
}//*/


/// Copy the sparsevector into the specified matrix row
///   \param M    Destination matrix.
///   \param sv   Source sparse vector.
///   \param row  Index of the destination row.
template<class Matrix, class Tag> inline
void copy_sparse_row(Matrix &M, const sparsevector<Tag> &sv, size_t row) {
    typename sparsevector<Tag>::const_iterator it;
    for (it = sv.begin(); it != sv.end(); ++it) {
        M(row, it->index) = it->value;
    }
}





//=============================================================================
#endif   // __SPARSE_VECTOR_H__


