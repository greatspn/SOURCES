#ifndef __COMPACT_TABLE_IMPL_H__
#define __COMPACT_TABLE_IMPL_H__
//=============================================================================
// Implementation of the compact tables
//=============================================================================



//=============================================================================
//  compact_column
//=============================================================================

inline bool compact_column::operator == (const compact_column &t) const {
    if (size() != t.size())
        return false;
    for (size_t i = 0; i < size(); i++)
        if (at(i) != t.at(i))
            return false;
    return true;
}

//=============================================================================

inline void compact_column::set(size_t pos, size_t value) {
    assert(pos < size());

    size_t n_bpv = storage_for(value);
    if (n_bpv > bpv)
        change_storage_type(n_bpv);

    // Look at the comment of the at() method to see how the memory layout works
    assert(value < (1ul << bpv));
    if (bpv == 0)
        return;

    const size_t h1ind = (pos * bpv) >> TBL_SHIFT;
    const size_t shift1 = (pos * bpv) & TBL_MASK;
    const size_t SZMASK1 = (1ul << bpv) - 1;

    // Set the first part of the stored value
    size_t elem1 = elems[h1ind];
    elem1 &= ~(SZMASK1 << shift1);
    elem1 |= value << shift1;
    elems[h1ind] = elem1;

    // Set the second part (if the value crosses two elems[] values)
    if (bpv + shift1 > TBL_BITS) {
        const size_t h2ind = ((pos + 1) * bpv - 1) >> TBL_SHIFT;
        const size_t shift2 = (bpv + shift1 - TBL_BITS);
        const size_t SZMASK2 = (1ul << shift2) - 1;
        size_t elem2 = elems[h2ind];
        elem2 &= ~SZMASK2;
        elem2 |= value >> (bpv - shift2);
        elems[h2ind] = elem2;
    }

    // Verify that the insertion has been completed correctly
    assert(value == at(pos));
}

//=============================================================================

inline compact_column::compact_column() {
    sz = 0;
    bpv = 0;
}

inline size_t compact_column::storage_for(size_t value) {
    size_t v_bpv = 0;
    if (value >= (1ul << 31))      { v_bpv |= 32; value >>= 32; }
    if (value >= (1ul << 15))      { v_bpv |= 16; value >>= 16; }
    if (value >= (1ul << 7))       { v_bpv |= 8;  value >>= 8;  }
    if (value >= (1ul << 3))       { v_bpv |= 4;  value >>= 4;  }
    if (value >= (1ul << 1))       { v_bpv |= 2;  value >>= 2;  }
    if (value >= (1ul << 0))       { v_bpv |= 1;  value >>= 1;  }
    return v_bpv;
}

inline void compact_column::change_storage_type(size_t n_bpv) {
    compact_column ntab;
    ntab.bpv = n_bpv;
    ntab.reserve(capacity());
    ntab.resize(size());

    for (size_t i = 0; i < size(); i++) {
        ntab.set(i, at(i));
    }
    assert(ntab == *this);
    swap(ntab);
}

inline void compact_column::resize(size_t new_sz, size_t value) {
    size_t old_sz = size();

    elems.resize((new_sz * bpv + TBL_BITS - 1) >> TBL_SHIFT);
    sz = new_sz;

    if (new_sz > old_sz) {
        for (size_t i = old_sz; i < size(); i++)
            set(i, value);
    }
}

inline void compact_column::push_back(size_t value) {
    resize(size() + 1, value);
}

inline size_t compact_column::num_bytes() const {
    return (size() * bpv + 7) / 8;
}

inline void compact_column::swap(compact_column &cs) {
    std::swap(sz, cs.sz);
    std::swap(bpv, cs.bpv);
    std::swap(elems, cs.elems);
}

inline void compact_column::clear() {
    elems.clear();
    sz = 0;
    bpv = 0;
}

inline void compact_column::reserve(size_t rsv_sz) {
    elems.reserve((rsv_sz * bpv + TBL_BITS - 1) >> TBL_SHIFT);
}

inline size_t compact_column::capacity() const {
    size_t cap = sz;
    if (bpv > 0)
        cap = std::max(cap, (elems.capacity() * TBL_BITS) / bpv);
    return cap;
}

//=============================================================================
// Implementation of the sgm_column class
//=============================================================================

template<size_t SZ>
sgm_column<SZ>::sgm_column() { }

template<size_t SZ>
bool sgm_column<SZ>::operator == (const sgm_column<SZ> &t) const {
    return size() == t.size() && vec == t.vec;
}

template<size_t SZ>
void sgm_column<SZ>::push_back(size_t value) {
    resize(size() + 1, value);
}

template<size_t SZ>
size_t sgm_column<SZ>::num_bytes() const {
    size_t nb = 0;
    for (size_t i = 0; i < vec.size(); i++)
        nb += vec[i].num_bytes();
    return nb;
}

template<size_t SZ>
void sgm_column<SZ>::swap(sgm_column &sc) {
    vec.swap(sc.vec);
}

template<size_t SZ>
void sgm_column<SZ>::clear() {
    vec.clear();
}

template<size_t SZ>
void sgm_column<SZ>::resize(size_t new_sz, size_t value) {
    size_t nblocks = (new_sz + SZ - 1) / SZ;
    size_t currblocks = vec.size();
#ifndef NDEBUG
    size_t currsize = size();
#endif
    // Resize block index
    vec.resize(nblocks);
    // Resize each block
    size_t startblock = (currblocks == 0 ? 0 : currblocks - 1);
    for (size_t b = startblock; b < nblocks; b++) {
        vec[b].reserve(SZ);
        vec[b].resize(std::min(SZ, new_sz - b * SZ), value);
    }

#ifndef NDEBUG
    size_t tsz = 0;
    for (size_t b = 0; b < vec.size(); b++) {
        assert(b == vec.size() - 1 || vec[b].size() == SZ);
        tsz += vec[b].size();
    }
    assert(tsz == size());
    for (size_t i = currsize; i < size(); i++) {
        assert(at(i) == value);
    }
#endif
}

template<size_t SZ>
void sgm_column<SZ>::reserve(size_t rsv_sz) {
    vec.reserve((rsv_sz + SZ - 1) / SZ);
}

template<size_t SZ>
size_t sgm_column<SZ>::capacity() const {
    return SZ * vec.capacity();
}

// Instantiation:
template class sgm_column<>;

//=============================================================================
// Printer helper functions
//=============================================================================

template<size_t N, typename Table>
struct print_tuple_aux {
    void print(ostream &os, const Table &t, size_t pos) {
        print_tuple_aux < N - 1, Table > ptaux;
        ptaux.print(os, t, pos);
        os << (std::tuple_size<typename Table::tuple_type>::value == (N - 1) ? ">" : ",");
        os << get_at<N>(t, pos);
    }
};
template<typename Table>
struct print_tuple_aux<0, Table> {
    void print(ostream &os, const Table &t, size_t pos) {
        os << "<" << get_at<0>(t, pos);
        os << (std::tuple_size<typename Table::tuple_type>::value == 1 ? ">" : "");
    }
};

//=============================================================================
// Helper functions for the computation of the hashes
//=============================================================================

template<typename Vec>
const sequence_id_impl<Vec>
sequence_id_impl<Vec>::INVALID_SEQ = sequence_id_impl<Vec>(-1);


static const size_t s_NumHashSizes = 26;
static const size_t s_HashTableSizes[s_NumHashSizes] = {
    53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189, 805306457, 1610612741
};

template<size_t N, typename Tuple>
struct mk_hash_helper {
    inline size_t mk_hash(const Tuple t) {
        typedef typename std::tuple_element<N, Tuple>::type type;
        mk_hash_helper < N - 1, Tuple > m;
        std::hash<type> h;
        return (h(std::get<N>(t)) << 3) ^ m.mk_hash(t);
    }
};
template<typename Tuple>
struct mk_hash_helper<0, Tuple> {
    inline size_t mk_hash(const Tuple t) {
        typedef typename std::tuple_element<0, Tuple>::type type;
        std::hash<type> h;
        return h(std::get<0>(t));
    }
};

template<typename Tuple>
static inline size_t mk_hash(size_t l_hash, size_t r_hash,
                             size_t index, const Tuple value) {
    mk_hash_helper < std::tuple_size<Tuple>::value - 1, Tuple > m;
    size_t m_hash = (index << 7) + m.mk_hash(value);
    return m_hash ^ (l_hash + (l_hash << 3)) ^ ((r_hash << 5) - r_hash);
}

template<typename SeqStore>
void print_memory_occupation(const SeqStore &s) {
    cout << "STORED SEQUENCES: " << s.tabs[INSERTED].max_id();
    cout << ", FRAGMENTS: " << s.tabs[FRAGMENTS].max_id() << endl;

    size_t seqdata = s.tabs[INSERTED].data_bytes();
    size_t fragsdata = s.tabs[FRAGMENTS].data_bytes();
    size_t seqhash = s.tabs[INSERTED].hash_bytes();
    size_t fragshash = s.tabs[FRAGMENTS].hash_bytes();

    cout << "  SEQUENCES MEMORY: " << right << setw(10) << (seqdata + seqhash) << " BYTES.\n";
    cout << "  FRAGMENTS MEMORY: " << right << setw(10) << (fragsdata + fragshash) << " BYTES.\n";
    cout << "      TOTAL MEMORY: " << right << setw(10) << s.num_bytes() << " BYTES." << endl;

    cout << "MEAN MEMORY PER SEQUENCE IS: " << double(s.num_bytes()) / size_t(s.tabs[INSERTED].max_id()) << " BYTES." << endl;
    cout << "  USED FOR SEQUENCE DATA: " << double(seqdata) / size_t(s.tabs[INSERTED].max_id()) << " BYTES." << endl;
    cout << "        USED FOR HASHING: " << double(seqhash + fragshash) / size_t(s.tabs[INSERTED].max_id()) << " BYTES." << endl;
    cout << "      USED FOR FRAGMENTS: " << double(fragsdata) / size_t(s.tabs[INSERTED].max_id()) << " BYTES." << endl;
}

//=============================================================================
// Implementation of stored_seq_const_iterator<seq_store>
//=============================================================================

template<class seq_store>
stored_seq_const_iterator<seq_store>::stored_seq_const_iterator(const seq_store *p)
/**/ : cur_sid(sequence_id::INVALID_SEQ), depth(0), pss(p), root_stk(FRAGMENTS) { }

template<class seq_store>
stored_seq_const_iterator<seq_store>::stored_seq_const_iterator(const stored_seq_const_iterator &c)
/**/ : cur_sid(c.cur_sid), depth(c.depth), pss(c.pss), root_stk(c.root_stk)
{ for (int i = 0; i < depth; i++)  sid_stack[i] = c.sid_stack[i]; }

template<class seq_store>
stored_seq_const_iterator<seq_store>::stored_seq_const_iterator(const seq_store *p, sequence_id sid,
        SeqTableKind stk)
/**/ : depth(0), pss(p), root_stk(stk) {
    assert(sid < pss->tabs[stk].max_id());
    if (stk == INSERTED && sid == pss->EmptyElemId) {
        cur_sid = sequence_id::INVALID_SEQ;
        root_stk = FRAGMENTS;
        return;
    }
    while (sid != sequence_id::INVALID_SEQ) {
        SeqTableKind this_stk = cur_stk();
        push_sid(sid);
        // cout << " PUSH " << top_sid() << endl;

        sid = pss->tabs[this_stk].s_left[size_t(sid)];
        // assert(stk == FRAGMENTS || this->pss->EmptyElemId == sequence_id::INVALID_SEQ ||
        // 	   sid != this->pss->EmptyElemId);
    }
    if (depth > 0) {
        cur_sid = top_sid();
        pop_sid();
    }
    else {
        cur_sid = sequence_id::INVALID_SEQ;
        root_stk = FRAGMENTS;
    }
}

template<class seq_store>
stored_seq_const_iterator<seq_store> &stored_seq_const_iterator<seq_store>::operator++() {
    // Are we leaving the top-level node?
    SeqTableKind this_stk = cur_stk();
    if (this_stk == INSERTED)
        root_stk = FRAGMENTS; // From here on we will only visit fragment nodes

    assert(cur_sid != sequence_id::INVALID_SEQ);
    assert(this_stk == FRAGMENTS || cur_sid != pss->EmptyElemId);
    sequence_id r_sid = pss->tabs[this_stk].s_right[ size_t(cur_sid) ];
    if (r_sid != sequence_id::INVALID_SEQ) {
        push_sid(r_sid);
        while (sequence_id::INVALID_SEQ != pss->tabs[cur_stk()].s_left[ size_t(top_sid()) ]) {
            assert(cur_stk() == FRAGMENTS);
            push_sid(pss->tabs[cur_stk()].s_left[ size_t(top_sid()) ]);
        }
    }
    if (depth == 0) {
        cur_sid = sequence_id::INVALID_SEQ;
        root_stk = FRAGMENTS;
        return *this;
    }
    cur_sid = top_sid();
    pop_sid();
    return *this;
}

template<class seq_store>
inline void stored_seq_const_iterator<seq_store>::push_sid(sequence_id sid) {
    assert(depth < MAX_TREE_DEPTH);
    sid_stack[depth++] = sid;
    // cout << "  push_sid("<<ts<<") : stack=" << print_vec((tab_sid*)&sid_stack, ((tab_sid*)&sid_stack) + depth, identity_functor())
    // 	 << ", cur_sid " << cur_sid << endl;
}

template<class seq_store>
inline void stored_seq_const_iterator<seq_store>::pop_sid() {
    assert(depth > 0);
    --depth;
    // cout << "  pop_sid() : stack=" << print_vec((tab_sid*)&sid_stack, ((tab_sid*)&sid_stack) + depth, identity_functor())
    // 	 << ", cur_sid " << cur_sid << endl;
}

template<class seq_store>
inline typename stored_seq_const_iterator<seq_store>::sequence_id
stored_seq_const_iterator<seq_store>::top_sid() const {
    assert(depth > 0);
    return sid_stack[depth - 1];
}


//=============================================================================
// Implementation of seq_store<Vec>
//=============================================================================

template<typename Vec, typename Traits>
seq_store<Vec, Traits>::seq_store() {
    // Initialize the hash tables
    tabs[INSERTED].hm_htab.resize(s_HashTableSizes[0], sequence_id::INVALID_SEQ);
    tabs[FRAGMENTS].hm_htab.resize(s_HashTableSizes[0], sequence_id::INVALID_SEQ);
}

//=============================================================================

template<typename Vec, typename Traits>
void seq_store<Vec, Traits>::freeze() {
    // Destroy the hash chains
    tabs[FRAGMENTS].hm_htab.clear();
    tabs[FRAGMENTS].s_next.clear();
    tabs[INSERTED].hm_htab.clear();
    tabs[INSERTED].s_next.clear();
}

//=============================================================================

template<typename T> static inline void prepare(std::vector<T> &sm, size_t len)
{	sm.resize(0);	sm.reserve(len);   }
template<typename T> static inline void prepare(std::set<T> &sm, size_t len)
{	sm.clear();   }
template<typename T> static inline void push_back(std::vector<T> &sm, const T arg)
{	sm.push_back(arg);   }
template<typename T> static inline void push_back(std::set<T> &sm, const T arg)
{	sm.insert(arg);   }

template<typename Vec, typename Traits>
void seq_store<Vec, Traits>::retrieve(const sequence_id id, Vec &sm) const {
    prepare(sm, get_length(id));
    _retrieve(INSERTED, id, sm);
}

//=============================================================================

template<typename Vec, typename Traits>
void seq_store<Vec, Traits>::_retrieve(SeqTableKind stk, const sequence_id id,
                                       Vec &sm) const {
    // Inserting the empty element or the leaf nodes
    if (stk == INSERTED && id == EmptyElemId)
        return;
    if (id == sequence_id::INVALID_SEQ)
        return;

#ifndef NDEBUG
    size_t i = sm.size();
#endif

    _retrieve(FRAGMENTS, tabs[stk].s_left[size_t(id)], sm);

    push_back(sm, conv.from_tuple(tabs[stk].s_values[size_t(id)]));

    _retrieve(FRAGMENTS, tabs[stk].s_right[size_t(id)], sm);

#ifndef NDEBUG
    assert((sm.size() - i) == tabs[stk].s_length[size_t(id)]);
    // Verify that iterators are consistent with the retrieved entries
    // const_iterator it = _begin_seq(stk, id), itE = end();
    // for (size_t j = 0; it != itE; ++it, ++j) {
    // 	assert(sm[i + j] == *it);
    // }
    // assert((i + j) == sm.size());
#endif
}

//=============================================================================

template<typename Vec, typename Traits>
typename seq_store<Vec, Traits>::value_type
seq_store<Vec, Traits>::_get_at(SeqTableKind stk, const sequence_id id,
                                size_t place) const {
    assert(stk == FRAGMENTS || id != EmptyElemId);
    assert(id != sequence_id::INVALID_SEQ);
    assert(id < tabs[stk].max_id());
    assert(place < tabs[stk].s_length[size_t(id)]);

    size_t left_length = _get_length(FRAGMENTS, tabs[stk].s_left[size_t(id)]);
    if (place == left_length)
        return conv.from_tuple(tabs[stk].s_values.tuple_at(size_t(id)));
    else if (place < left_length)
        return _get_at(FRAGMENTS, tabs[stk].s_left[size_t(id)], place);
    else
        return _get_at(FRAGMENTS, tabs[stk].s_right[size_t(id)], place - left_length - 1);
}

//=============================================================================

template<typename Vec, typename Traits>
size_t seq_store<Vec, Traits>::_get_length(SeqTableKind stk, const sequence_id id) const {
    if (id == sequence_id::INVALID_SEQ)
        return 0;
    assert(id < tabs[stk].max_id());
    return tabs[stk].s_length[size_t(id)];
}

//=============================================================================

template<typename Vec, typename Traits>
bool seq_store<Vec, Traits>::_check_equiv(SeqTableKind stk,
        const sequence_id sid,
        vec_const_iterator it,
        size_t len) const {
    vec_const_iterator vec_it = it;
    const_iterator seq_it = _begin_seq(stk, sid);
    size_t i = 0;
    for (; seq_it != end() && i < len; ++seq_it, ++vec_it, ++i) {
        if (*vec_it != *seq_it)
            return false;
    }
    return (seq_it == end() && i == len);

    // bool isEmptyElem = ((stk==INSERTED && sid == EmptyElemId) ||
    // 					sid == sequence_id::INVALID_SEQ);
    // bool isEmptySeq = (s2 == s1);
    // if (isEmptyElem || isEmptySeq)
    // 	return (isEmptyElem && isEmptySeq);

    // vec_const_iterator s_mid = s1 + (s2 - s1) / 2;
    // if (tabs[stk].s_index[size_t(sid)] != s_mid->index ||
    // 	conv.from_tuple(tabs[stk].s_values.tuple_at(size_t(sid))) != s_mid->value)
    // 	return false;

    // return (_check_equiv(FRAGMENTS, tabs[stk].s_left[size_t(sid)], s1, s_mid) &&
    // 		_check_equiv(FRAGMENTS, tabs[stk].s_right[size_t(sid)], s_mid + 1, s2));
}

//=============================================================================

template<typename Vec, typename Traits>
bool seq_store<Vec, Traits>::_exists(SeqTableKind stk, vec_const_iterator it,
                                     size_t len, size_t &hash,
                                     sequence_id &out_id) const {
    assert(tabs[stk].s_next.size() == size_t(tabs[stk].max_id()));
    if (len == 0) {
        hash = 0;
        if (stk == INSERTED) {
            // Check if the empty element already exists
            out_id = EmptyElemId;
            return (out_id != sequence_id::INVALID_SEQ);
        }
        else { // FRAGMENTS - never insert the empty fragment
            out_id = sequence_id::INVALID_SEQ;
            return true;
        }
    }

    vec_const_iterator it2 = it;
    hash = _make_hash(it2, len);
    sequence_id sid_chain = tabs[stk].hm_htab[ hash % tabs[stk].hm_htab.size() ];
    while (sid_chain != sequence_id::INVALID_SEQ) {
        bool equiv = _check_equiv(stk, sid_chain, it, len);
#ifndef NDEBUG
        Vec sm2, sm;
        _retrieve(stk, sid_chain, sm2);
        size_t l = 0;
        for (vec_const_iterator sit = it; l < len; ++sit, ++l)
            push_back(sm, *sit);
        assert(equiv == (sm == sm2));
#endif
        if (equiv) {// already stored
            out_id = sid_chain;
            return true;
        }

        // Next sid in hash chain
        sid_chain = tabs[stk].s_next[ size_t(sid_chain) ];
    }
    // Not found
    return false;
}

//=============================================================================

template<typename Vec, typename Traits>
typename seq_store<Vec, Traits>::sequence_id
seq_store<Vec, Traits>::_insert(SeqTableKind stk,
                                vec_const_iterator &it,
                                size_t len) {
    assert(tabs[stk].s_next.size() == size_t(tabs[stk].max_id()));

    if (stk == FRAGMENTS && len == 0) {
        return sequence_id::INVALID_SEQ;
    }

    // Test if the sequence [s1-s2) already exists in this table
    size_t hash;
    sequence_id new_id;
    if (_exists(stk, it, len, hash, new_id)) {
        // cout << " _insert " << print_vec(s1, s2, pp) << " exists with sid=" << new_id << endl;
        std::advance(it, len);
        return new_id;
    }

    if (len > 0) {
        // Split sm as:  sm1 + [half] + sm2
        size_t midlen = len / 2;
        // Insert left and right fragments
        sequence_id left_id = _insert(FRAGMENTS, it, midlen);
        value_type midval = *it;
        ++it;
        sequence_id right_id = _insert(FRAGMENTS, it, len - midlen - 1);
        assert(len == (_get_length(FRAGMENTS, left_id) + 1 + _get_length(FRAGMENTS, right_id)));

        new_id = tabs[stk].max_id();
        tabs[stk].s_values.push_back_tuple(conv.to_tuple(midval));
        tabs[stk].s_length.push_back(len);
        tabs[stk].s_left.push_back(left_id);
        tabs[stk].s_right.push_back(right_id);
        // cout << " _insert " << print_vec(s1, s2, pp) << " sid=" << new_id;
        // cout << " left=" << (left_id==sequence_id::INVALID_SEQ ? -1 : ssize_t(left_id));
        // cout << " right=" << (right_id==sequence_id::INVALID_SEQ ? -1 : ssize_t(right_id)) << endl;
    }
    else {
        // Special case: add the empty element in the INSERTED table.
        new_id = tabs[stk].max_id();
        assert(stk == INSERTED);
        assert(EmptyElemId == sequence_id::INVALID_SEQ);
        EmptyElemId = new_id;
        tabs[stk].s_values.push_back_tuple(conv.to_tuple(conv.empty_elem()));
        tabs[stk].s_length.push_back(0);
        tabs[stk].s_left.push_back(sequence_id::INVALID_SEQ);
        tabs[stk].s_right.push_back(sequence_id::INVALID_SEQ);
        // cout << " _insert empty elem, sid=" << new_id;
    }
    tabs[stk].s_next.push_back(sequence_id::INVALID_SEQ);

    // Do a rehashing if there are too many entries in the hm_htab table
    bool doRehashing = (tabs[stk].max_id() > tabs[stk].hm_htab.size() * 3);

    if (!doRehashing) {
        assert(tabs[stk].hm_htab.size() > 0);
        // Insert in the hash table
        size_t hashmap_pos = (hash % tabs[stk].hm_htab.size());
        tabs[stk].s_next.set(size_t(new_id), tabs[stk].hm_htab[hashmap_pos]);
        tabs[stk].hm_htab.set(hashmap_pos, new_id);
    }
    else { // Resize the hash table and re-hash
        size_t new_hash_sz = *std::find_if(s_HashTableSizes, s_HashTableSizes + s_NumHashSizes,
        [&](size_t i) { return i > tabs[stk].hm_htab.size(); });
        std::vector<bool> hins(tabs[stk].s_next.size());
        std::fill(hins.begin(), hins.end(), false);

        for (size_t i = 0; i < tabs[stk].s_next.size(); i++)
            tabs[stk].s_next.set(i, sequence_id::INVALID_SEQ);
        tabs[stk].hm_htab.resize(new_hash_sz);
        for (size_t i = 0; i < tabs[stk].hm_htab.size(); i++)
            tabs[stk].hm_htab.set(i, sequence_id::INVALID_SEQ);

        for (sequence_id sidit(0); sidit < tabs[stk].max_id(); sidit = sequence_id(size_t(sidit) + 1)) {
            if ((stk == INSERTED && sidit == EmptyElemId) ||
                    sidit == sequence_id::INVALID_SEQ)
                continue;
            _recursive_rehash(stk, sidit, hins);
        }

#ifndef NDEBUG
        // Verify that the insertion has been successful
        for (sequence_id sidit(0); sidit < tabs[stk].max_id(); sidit = sequence_id(size_t(sidit) + 1)) {
            Vec sm;
            _retrieve(stk, sidit, sm);
            size_t hash;
            sequence_id out_id;
            assert(_exists(stk, sm.begin(), sm.size(), hash, out_id));
            assert(out_id == sidit);
            // const_iterator elit = _begin_seq(stk, sidit);
            // for (const auto& ivp : sm) {
            // 	assert(elit.index() == ivp.index && *elit == ivp.value);
            // 	++elit;
            // }
            // assert(elit == end());
        }
#endif
    }

    assert(tabs[stk].max_id() == tabs[stk].s_values.size());
    assert(tabs[stk].max_id() == tabs[stk].s_length.size());
    assert(tabs[stk].max_id() == tabs[stk].s_left.size());
    assert(tabs[stk].max_id() == tabs[stk].s_right.size());
    assert(tabs[stk].max_id() == tabs[stk].s_next.size());

    return new_id;
}

//=============================================================================

template<typename Vec, typename Traits>
size_t seq_store<Vec, Traits>::num_bytes() const {
    size_t count = 0;
    for (SeqTableKind stk = SeqTableKind(0); stk < NUM_SEQUENCE_TABLES; next_stk(stk)) {
        count += tabs[stk].s_left.num_bytes() + tabs[stk].s_right.num_bytes();
        count += tabs[stk].s_length.num_bytes() + tabs[stk].s_values.num_bytes();
        count += tabs[stk].hm_htab.num_bytes() + tabs[stk].s_next.num_bytes();
    }
    return count;
}

//=============================================================================

template<typename Vec, typename Traits>
size_t seq_store<Vec, Traits>::make_hash(const Vec &sm) {
    vec_const_iterator it = sm.begin();
    size_t hash = _make_hash(it, sm.size());
    assert(it == sm.end());
    return hash;
}

template<typename Vec, typename Traits>
size_t seq_store<Vec, Traits>::_make_hash(vec_const_iterator &it, size_t len) const {
    if (len == 0)
        return 0;
    size_t mlen = len / 2;
    size_t l_hash = _make_hash(it, mlen);
    tuple_type mtup = conv.to_tuple(*it);
    ++it;
    size_t r_hash = _make_hash(it, len - mlen - 1);
    return mk_hash(l_hash, r_hash, len, mtup);
}

//=============================================================================

template<typename Vec, typename Traits>
size_t seq_store<Vec, Traits>::_recursive_rehash(SeqTableKind stk,
        const sequence_id sid,
        std::vector<bool> &hins) {
    if ((stk == INSERTED && sid == EmptyElemId) || sid == sequence_id::INVALID_SEQ)
        return 0;

    size_t l_hash, r_hash;
    if (stk == INSERTED) {
        // rehashing the first-level seq_table - need to recompute the
        // hash of the fragments
        l_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_left[ size_t(sid) ]);
        r_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_right[ size_t(sid) ]);
    }
    else {
        // rehashing of the fragment tables - can share the already computed hashes
        l_hash = _recursive_rehash(FRAGMENTS, tabs[stk].s_left[ size_t(sid) ], hins);
        r_hash = _recursive_rehash(FRAGMENTS, tabs[stk].s_right[ size_t(sid) ], hins);
    }

    size_t hash = mk_hash(l_hash, r_hash,
                          tabs[stk].s_length[ size_t(sid) ],
                          tabs[stk].s_values.tuple_at(size_t(sid)));
    if (!hins[ size_t(sid) ]) {
        // Insert in the hash table
        size_t hashmap_pos = (hash % tabs[stk].hm_htab.size());
        tabs[stk].s_next.set(size_t(sid), tabs[stk].hm_htab[ hashmap_pos ]);
        tabs[stk].hm_htab.set(hashmap_pos, sid);

        hins[ size_t(sid) ] = true;
    }
    return hash;
}

//=============================================================================

template<typename Vec, typename Traits>
size_t seq_store<Vec, Traits>::_recompute_hash(SeqTableKind stk,
        const sequence_id sid) const {
    if ((stk == INSERTED && sid == EmptyElemId) || sid == sequence_id::INVALID_SEQ)
        return 0;
    assert(sid < tabs[stk].max_id());
    size_t l_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_left[ size_t(sid) ]);
    size_t r_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_right[ size_t(sid) ]);

    size_t hash = mk_hash(l_hash, r_hash,
                          tabs[stk].s_length[ size_t(sid) ],
                          tabs[stk].s_values.tuple_at(size_t(sid)));
    return hash;
}

//=============================================================================

template<typename Vec, typename Traits>
bool seq_store<Vec, Traits>::exists(sequence_id &out_id, vec_const_iterator it,
                                    size_t len) const {
    size_t hash;
    return _exists(INSERTED, it, len, hash, out_id);
}

//=============================================================================

template<typename Vec, typename Traits>
void seq_store<Vec, Traits>::swap(seq_store &s) {
    for (size_t i = 0; i < NUM_SEQUENCE_TABLES; i++) {
        tabs[i].hm_htab.swap(s.tabs[i].hm_htab);
        tabs[i].s_left.swap(s.tabs[i].s_left);
        tabs[i].s_right.swap(s.tabs[i].s_right);
        tabs[i].s_length.swap(s.tabs[i].s_length);
        tabs[i].s_values.swap(s.tabs[i].s_values);
        tabs[i].s_next.swap(s.tabs[i].s_next);
    }
    std::swap(EmptyElemId, s.EmptyElemId);
    conv.swap(s.conv);
}

//=============================================================================

template<typename Vec, typename Traits>
void seq_store<Vec, Traits>::clear() {
    seq_store s;
    swap(s);
}

//=============================================================================

template<typename Vec, typename Traits>
void seq_store<Vec, Traits>::print_memory_occupation() const {
    ::print_memory_occupation(*this);
}

//=============================================================================
// Implementation of sparse_seq_store<SparseVec>
//=============================================================================

template<typename SparseVec, typename Traits>
sparse_seq_store<SparseVec, Traits>::sparse_seq_store(index_type _max_index)
/**/ : max_index(_max_index) {
    // Initialize the hash tables
    tabs[INSERTED].hm_htab.resize(s_HashTableSizes[0], sequence_id::INVALID_SEQ);
    tabs[FRAGMENTS].hm_htab.resize(s_HashTableSizes[0], sequence_id::INVALID_SEQ);
}

//=============================================================================

template<typename SparseVec, typename Traits>
void sparse_seq_store<SparseVec, Traits>::freeze() {
    // Destroy the hash chains
    tabs[FRAGMENTS].hm_htab.clear();
    tabs[FRAGMENTS].s_next.clear();
    tabs[INSERTED].hm_htab.clear();
    tabs[INSERTED].s_next.clear();
}

//=============================================================================

template<typename SparseVec, typename Traits>
void sparse_seq_store<SparseVec, Traits>::_retrieve(SeqTableKind stk, const sequence_id id,
        SparseVec &sm) const {
    // Inserting the empty element or the leaf nodes
    if (stk == INSERTED && id == EmptyElemId)
        return;
    if (id == sequence_id::INVALID_SEQ)
        return;

#ifndef NDEBUG
    size_t i = sm.nonzeros();
#endif

    _retrieve(FRAGMENTS, tabs[stk].s_left[size_t(id)], sm);

    sm.insert_element(tabs[stk].s_index[size_t(id)],
                      conv.from_tuple(tabs[stk].s_values[size_t(id)]));

    _retrieve(FRAGMENTS, tabs[stk].s_right[size_t(id)], sm);

#ifndef NDEBUG
    // Verify that iterators are consistent with the retrieved entries
    const_iterator it = _begin_seq(stk, id), itE = end();
    while (it != itE) {
        assert(sm.ith_nonzero(i).index == it.index());
        assert(sm.ith_nonzero(i).value == *it);
        ++i;
        ++it;
    }
    assert(i == sm.nonzeros());
#endif
}

//=============================================================================

template<typename SparseVec, typename Traits>
typename sparse_seq_store<SparseVec, Traits>::value_type
sparse_seq_store<SparseVec, Traits>::_get_at(SeqTableKind stk, const sequence_id id,
        index_type place) const {
    // Querying from the empty element
    if (stk == INSERTED && id == EmptyElemId)
        return conv.empty_elem();
    // Querying an empty index/value from a non-empty sequence
    if (id == sequence_id::INVALID_SEQ)
        return conv.empty_elem();

    assert(id < tabs[stk].max_id());
    if (place == tabs[stk].s_index[size_t(id)])
        return conv.from_tuple(tabs[stk].s_values.tuple_at(size_t(id)));
    else if (place < tabs[stk].s_index[size_t(id)])
        return _get_at(FRAGMENTS, tabs[stk].s_left[size_t(id)], place);
    else
        return _get_at(FRAGMENTS, tabs[stk].s_right[size_t(id)], place);
}

//=============================================================================

template<typename SparseVec, typename Traits>
bool sparse_seq_store<SparseVec, Traits>::_check_equiv(SeqTableKind stk,
        const sequence_id sid,
        sparsevec_const_iterator s1,
        sparsevec_const_iterator s2) const {
    bool isEmptyElem = ((stk == INSERTED && sid == EmptyElemId) ||
                        sid == sequence_id::INVALID_SEQ);
    bool isEmptySeq = (s2 == s1);
    if (isEmptyElem || isEmptySeq)
        return (isEmptyElem && isEmptySeq);

    sparsevec_const_iterator s_mid = s1 + (s2 - s1) / 2;
    if (tabs[stk].s_index[size_t(sid)] != s_mid->index ||
            conv.from_tuple(tabs[stk].s_values.tuple_at(size_t(sid))) != s_mid->value)
        return false;

    return (_check_equiv(FRAGMENTS, tabs[stk].s_left[size_t(sid)], s1, s_mid) &&
            _check_equiv(FRAGMENTS, tabs[stk].s_right[size_t(sid)], s_mid + 1, s2));
}

//=============================================================================

template<typename SparseVec, typename Traits>
bool sparse_seq_store<SparseVec, Traits>::_exists(SeqTableKind stk,
        sparsevec_const_iterator s1,
        sparsevec_const_iterator s2,
        size_t hash,
        sequence_id &out_id) const {
    assert(tabs[stk].s_next.size() == size_t(tabs[stk].max_id()));
    assert(hash == make_hash(s1, s2));
    if (s2 == s1) {
        if (stk == INSERTED) {
            // Check if the empty element already exists
            out_id = EmptyElemId;
            return (out_id != sequence_id::INVALID_SEQ);
        }
        else { // FRAGMENTS - never insert the empty fragment
            out_id = sequence_id::INVALID_SEQ;
            return true;
        }
    }

    sequence_id sid_chain = tabs[stk].hm_htab[ hash % tabs[stk].hm_htab.size() ];
    while (sid_chain != sequence_id::INVALID_SEQ) {
        bool equiv = _check_equiv(stk, sid_chain, s1, s2);
#ifndef NDEBUG
        SparseVec sm2(size_of_vectors()), sm(size_of_vectors());
        _retrieve(stk, sid_chain, sm2);
        for (sparsevec_const_iterator sit = s1; sit != s2; ++sit)
            sm.insert_element(sit->index, sit->value);
        assert(equiv == (sm == sm2));
#endif
        if (equiv) {// already stored
            out_id = sid_chain;
            return true;
        }

        // Next sid in hash chain
        sid_chain = tabs[stk].s_next[ size_t(sid_chain) ];
    }
    // Not found
    return false;
}

//=============================================================================

template<typename SparseVec, typename Traits>
typename sparse_seq_store<SparseVec, Traits>::sequence_id
sparse_seq_store<SparseVec, Traits>::_insert(SeqTableKind stk,
        sparsevec_const_iterator s1,
        sparsevec_const_iterator s2,
        size_t &out_hash) {
    assert(tabs[stk].s_next.size() == size_t(tabs[stk].max_id()));

    if (stk == FRAGMENTS && (s2 - s1) == 0) {
        out_hash = 0;
        return sequence_id::INVALID_SEQ;
    }

    sequence_id new_id;
    if ((s2 - s1) > 0) {
        // Split sm as:  sm1 + [half] + sm2
        sparsevec_const_iterator s_mid = s1 + (s2 - s1) / 2;
        assert(s_mid->value != conv.empty_elem());
        assert(s_mid->index < index_type(size_of_vectors()));

        // Insert left and right fragments
        size_t left_hash, right_hash;
        sequence_id left_id = _insert(FRAGMENTS, s1, s_mid, left_hash);
        sequence_id right_id = _insert(FRAGMENTS, s_mid + 1, s2, right_hash);

        // Test if the sequence [s1-s2) already exists in this table
        out_hash = mk_hash(left_hash, right_hash, s_mid->index, conv.to_tuple(s_mid->value));
        assert(out_hash == make_hash(s1, s2));
        if (_exists(stk, s1, s2, out_hash, new_id)) {
            // cout << " _insert " << print_vec(s1, s2, pp) << " exists with sid=" << new_id << endl;
            return new_id;
        }

        new_id = tabs[stk].max_id();
        tabs[stk].s_values.push_back_tuple(conv.to_tuple(s_mid->value));
        tabs[stk].s_index.push_back(s_mid->index);
        tabs[stk].s_left.push_back(left_id);
        tabs[stk].s_right.push_back(right_id);
        // cout << " _insert " << print_vec(s1, s2, pp) << " sid=" << new_id;
        // cout << " left=" << (left_id==sequence_id::INVALID_SEQ ? -1 : ssize_t(left_id));
        // cout << " right=" << (right_id==sequence_id::INVALID_SEQ ? -1 : ssize_t(right_id)) << endl;
    }
    else {
        // Special case: add the empty element in the INSERTED table.
        new_id = tabs[stk].max_id();
        out_hash = 0;
        assert(stk == INSERTED);
        assert(EmptyElemId == sequence_id::INVALID_SEQ);
        EmptyElemId = new_id;
        tabs[stk].s_values.push_back_tuple(conv.to_tuple(conv.empty_elem()));
        tabs[stk].s_index.push_back(index_type(0));
        tabs[stk].s_left.push_back(sequence_id::INVALID_SEQ);
        tabs[stk].s_right.push_back(sequence_id::INVALID_SEQ);
        // cout << " _insert empty elem, sid=" << new_id;
    }
    tabs[stk].s_next.push_back(sequence_id::INVALID_SEQ);

    // Do a rehashing if there are too many entries in the hm_htab table
    bool doRehashing = (tabs[stk].max_id() > tabs[stk].hm_htab.size() * 3);

    if (!doRehashing) {
        assert(tabs[stk].hm_htab.size() > 0);
        // Insert in the hash table
        size_t hashmap_pos = (out_hash % tabs[stk].hm_htab.size());
        tabs[stk].s_next.set(size_t(new_id), tabs[stk].hm_htab[hashmap_pos]);
        tabs[stk].hm_htab.set(hashmap_pos, new_id);
    }
    else { // Resize the hash table and re-hash
        size_t new_hash_sz = *std::find_if(s_HashTableSizes, s_HashTableSizes + s_NumHashSizes,
        [&](size_t i) { return i > tabs[stk].hm_htab.size(); });
        std::vector<bool> hins(tabs[stk].s_next.size());
        std::fill(hins.begin(), hins.end(), false);

        for (size_t i = 0; i < tabs[stk].s_next.size(); i++)
            tabs[stk].s_next.set(i, sequence_id::INVALID_SEQ);
        tabs[stk].hm_htab.resize(new_hash_sz);
        for (size_t i = 0; i < tabs[stk].hm_htab.size(); i++)
            tabs[stk].hm_htab.set(i, sequence_id::INVALID_SEQ);

        for (sequence_id sidit(0); sidit < tabs[stk].max_id(); sidit = sequence_id(size_t(sidit) + 1)) {
            if ((stk == INSERTED && sidit == EmptyElemId) ||
                    sidit == sequence_id::INVALID_SEQ)
                continue;
            _recursive_rehash(stk, sidit, hins);
        }

#ifndef NDEBUG
        // Verify that the insertion has been successful
        for (sequence_id sidit(0); sidit < tabs[stk].max_id(); sidit = sequence_id(size_t(sidit) + 1)) {
            SparseVec sm(size_of_vectors());
            _retrieve(stk, sidit, sm);
            size_t sid_hash = make_hash(sm.begin(), sm.end());
            sequence_id out_id;
            assert(_exists(stk, sm.begin(), sm.end(), sid_hash, out_id));
            assert(out_id == sidit);
            const_iterator elit = _begin_seq(stk, sidit);
            for (const auto &ivp : sm) {
                assert(elit.index() == ivp.index && *elit == ivp.value);
                ++elit;
            }
            assert(elit == end());
        }
#endif
    }

    assert(tabs[stk].max_id() == tabs[stk].s_values.size());
    assert(tabs[stk].max_id() == tabs[stk].s_index.size());
    assert(tabs[stk].max_id() == tabs[stk].s_left.size());
    assert(tabs[stk].max_id() == tabs[stk].s_right.size());
    assert(tabs[stk].max_id() == tabs[stk].s_next.size());

    return new_id;
}

//=============================================================================

template<typename SparseVec, typename Traits>
size_t sparse_seq_store<SparseVec, Traits>::num_bytes() const {
    size_t count = 0;
    for (SeqTableKind stk = SeqTableKind(0); stk < NUM_SEQUENCE_TABLES; next_stk(stk)) {
        count += tabs[stk].s_left.num_bytes() + tabs[stk].s_right.num_bytes();
        count += tabs[stk].s_index.num_bytes() + tabs[stk].s_values.num_bytes();
        count += tabs[stk].hm_htab.num_bytes() + tabs[stk].s_next.num_bytes();
    }
    return count;
}

//=============================================================================

template<typename SparseVec, typename Traits>
size_t sparse_seq_store<SparseVec, Traits>::make_hash(const SparseVec &sm) {
    return make_hash(sm.begin(), sm.end());
}

template<typename SparseVec, typename Traits>
size_t sparse_seq_store<SparseVec, Traits>::make_hash(sparsevec_const_iterator s1,
        sparsevec_const_iterator s2) const {
    size_t diff = (s2 - s1);
    if (diff == 0)
        return 0;

    sparsevec_const_iterator sm = s1 + (diff / 2);
    size_t l_hash = make_hash(s1, sm);
    size_t r_hash = make_hash(sm + 1, s2);
    return mk_hash(l_hash, r_hash, sm->index, conv.to_tuple(sm->value));
}

//=============================================================================

template<typename SparseVec, typename Traits>
size_t sparse_seq_store<SparseVec, Traits>::_recursive_rehash(SeqTableKind stk,
        const sequence_id sid,
        std::vector<bool> &hins) {
    if ((stk == INSERTED && sid == EmptyElemId) || sid == sequence_id::INVALID_SEQ)
        return 0;

    size_t l_hash, r_hash;
    if (stk == INSERTED) {
        // rehashing the first-level seq_table - need to recompute the
        // hash of the fragments
        l_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_left[ size_t(sid) ]);
        r_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_right[ size_t(sid) ]);
    }
    else {
        // rehashing of the fragment tables - can share the already computed hashes
        l_hash = _recursive_rehash(FRAGMENTS, tabs[stk].s_left[ size_t(sid) ], hins);
        r_hash = _recursive_rehash(FRAGMENTS, tabs[stk].s_right[ size_t(sid) ], hins);
    }

    size_t hash = mk_hash(l_hash, r_hash,
                          (index_type)tabs[stk].s_index[ size_t(sid) ],
                          tabs[stk].s_values.tuple_at(size_t(sid)));
    if (!hins[ size_t(sid) ]) {
        // Insert in the hash table
        size_t hashmap_pos = (hash % tabs[stk].hm_htab.size());
        tabs[stk].s_next.set(size_t(sid), tabs[stk].hm_htab[ hashmap_pos ]);
        tabs[stk].hm_htab.set(hashmap_pos, sid);

        hins[ size_t(sid) ] = true;
    }
    return hash;
}

//=============================================================================

template<typename SparseVec, typename Traits>
size_t sparse_seq_store<SparseVec, Traits>::_recompute_hash(SeqTableKind stk,
        const sequence_id sid) const {
    if ((stk == INSERTED && sid == EmptyElemId) || sid == sequence_id::INVALID_SEQ)
        return 0;
    assert(sid < tabs[stk].max_id());
    size_t l_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_left[ size_t(sid) ]);
    size_t r_hash = _recompute_hash(FRAGMENTS, tabs[stk].s_right[ size_t(sid) ]);

    size_t hash = mk_hash(l_hash, r_hash,
                          (index_type)tabs[stk].s_index[ size_t(sid) ],
                          tabs[stk].s_values.tuple_at(size_t(sid)));
    return hash;
}

//=============================================================================

template<typename SparseVec, typename Traits>
bool sparse_seq_store<SparseVec, Traits>::exists(sequence_id &out_id,
        sparsevec_const_iterator s1,
        sparsevec_const_iterator s2) const {
    size_t hash = make_hash(s1, s2);
    return _exists(INSERTED, s1, s2, hash, out_id);
}

//=============================================================================

template<typename SparseVec, typename Traits>
void sparse_seq_store<SparseVec, Traits>::swap(sparse_seq_store &s) {
    for (size_t i = 0; i < NUM_SEQUENCE_TABLES; i++) {
        tabs[i].hm_htab.swap(s.tabs[i].hm_htab);
        tabs[i].s_left.swap(s.tabs[i].s_left);
        tabs[i].s_right.swap(s.tabs[i].s_right);
        tabs[i].s_index.swap(s.tabs[i].s_index);
        tabs[i].s_values.swap(s.tabs[i].s_values);
        tabs[i].s_next.swap(s.tabs[i].s_next);
    }
    std::swap(EmptyElemId, s.EmptyElemId);
    std::swap(max_index, s.max_index);
    conv.swap(s.conv);
}

//=============================================================================

template<typename SparseVec, typename Traits>
void sparse_seq_store<SparseVec, Traits>::clear(index_type _max_index) {
    sparse_seq_store s(_max_index);
    swap(s);
}

//=============================================================================

template<typename SparseVec, typename Traits>
void sparse_seq_store<SparseVec, Traits>::print_memory_occupation() const {
    ::print_memory_occupation(*this);
}


//=============================================================================
#endif // __COMPACT_TABLE_IMPL_H__
