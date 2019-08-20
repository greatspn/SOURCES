//---------------------------------------------------------------------------------------
#ifndef __DOUBLY_LINKED_CRS_MATRIX__
#define __DOUBLY_LINKED_CRS_MATRIX__
//---------------------------------------------------------------------------------------

namespace dlcrs_mat {

template<typename Value> class doubly_linked_crs_matrix;

//---------------------------------------------------------------------------------------

// Index of the header row (actually row 0 in the matrix)
const size_t HEADER_ROW = size_t(-1);

//---------------------------------------------------------------------------------------

struct row_colnnz_pair {
	size_t  row;     // Row index
	size_t  colnnz;  // Index of the i-th nonzero entry

    inline row_colnnz_pair() : row(HEADER_ROW), colnnz(HEADER_ROW) {}
    inline row_colnnz_pair(size_t i, size_t j) : row(i), colnnz(j) {}
    inline ~row_colnnz_pair() {}
    inline row_colnnz_pair(const row_colnnz_pair&) = default;
    inline row_colnnz_pair(row_colnnz_pair&&) = default;
    inline row_colnnz_pair& operator=(const row_colnnz_pair&) = default;
    inline row_colnnz_pair& operator=(row_colnnz_pair&&) = default;

    inline bool operator == (const row_colnnz_pair &ij) const
    { return (row == ij.row && colnnz == ij.colnnz); }
    inline bool operator != (const row_colnnz_pair &ij) const
    { return (row != ij.row || colnnz != ij.colnnz); }

    inline bool operator < (const row_colnnz_pair &ij) const
    { return (row < ij.row || (row == ij.row && colnnz < ij.colnnz)); }
    inline bool operator > (const row_colnnz_pair &ij) const
    { return (row > ij.row || (row == ij.row && colnnz > ij.colnnz)); }
    inline bool operator <= (const row_colnnz_pair &ij) const
    { return (row <= ij.row || (row == ij.row && colnnz <= ij.colnnz)); }
    inline bool operator >= (const row_colnnz_pair &ij) const
    { return (row >= ij.row || (row == ij.row && colnnz >= ij.colnnz)); }
};

//---------------------------------------------------------------------------------------

template<typename Value>
struct entry_t {
	size_t index;  // Column index of this entry
	Value  value;
protected:
	row_colnnz_pair  above;
	row_colnnz_pair  below;
public:
    inline entry_t() : index(-1), value(-1), above(), below() {}
    inline entry_t(size_t j, Value v, row_colnnz_pair 
    			 a=row_colnnz_pair(), row_colnnz_pair b=row_colnnz_pair()) 
    : index(j), value(v), above(a), below(b) {}
    inline entry_t(base_index_value_pair<size_t, Value> ivp)
    : index(ivp.index), value(ivp.value) { }
    inline ~entry_t() {}
    inline entry_t(const entry_t&) = default;
    inline entry_t(entry_t&&) = default;
    inline entry_t& operator=(const entry_t&) = default;
    inline entry_t& operator=(entry_t&&) = default;

    // Comparison does not consider above/below pairs, but only the index/value
    inline bool operator == (const entry_t &en) const
    { return (index == en.index && value == en.value); }

    inline bool operator != (const entry_t &en) const
    { return (index != en.index || value != en.value); }

    inline bool operator < (const entry_t &en) const { return (index < en.index); }
    inline bool operator > (const entry_t &en) const { return (index > en.index); }
    inline bool operator <= (const entry_t &en) const { return (index <= en.index); }
    inline bool operator >= (const entry_t &en) const { return (index >= en.index); }

    friend class doubly_linked_crs_matrix<Value>;
};

//---------------------------------------------------------------------------------------

template<class Value>
struct srv_tag { // sparse row vector tag
    typedef size_t              index_type;
    typedef Value               value_type;
    typedef entry_t<value_type> index_value_pair;
    static inline bool allow_zeros() { return true; }
};

//---------------------------------------------------------------------------------------

template<typename Value>
class doubly_linked_crs_matrix {
public:
	typedef size_t              index_type;
	typedef Value               value_type;
	typedef entry_t<value_type> entry_type;

	typedef sparsevector<srv_tag<Value>> row_vector_type;
	typedef std::vector<row_vector_type> matrix_type;

    inline doubly_linked_crs_matrix(size_t _r=0, size_t _c=0) 
    : n_rows(_r), n_cols(_c), __rows(_r + 1) {
    	for (size_t i=0; i<=n_rows; i++)
    		__rows[i].resize(n_cols);
    	for (size_t j=0; j<n_cols; j++) 
    		__rows[0].data().push_back(entry_type(HEADER_ROW, value_type(), 
    								      row_colnnz_pair(HEADER_ROW, j), 
    								      row_colnnz_pair(HEADER_ROW, j)));
    }
    inline ~doubly_linked_crs_matrix() {}
    inline doubly_linked_crs_matrix(const doubly_linked_crs_matrix&) = default;
    inline doubly_linked_crs_matrix(doubly_linked_crs_matrix&&) = default;
    inline doubly_linked_crs_matrix& operator=(const doubly_linked_crs_matrix&) = default;
    inline doubly_linked_crs_matrix& operator=(doubly_linked_crs_matrix&&) = default;

    // Basic methods
    inline size_t num_rows() const { return n_rows; }
    inline size_t num_cols() const { return n_cols; }

protected:
	inline row_vector_type& rows(size_t r) 
	{ assert(r>=0 && r<n_rows); return __rows[r+1]; }
	inline const row_vector_type& rows(size_t r) const 
	{ assert(r>=0 && r<n_rows); return __rows[r+1]; }

	inline entry_type& header(size_t c) { return __rows[0].ith_nonzero(c); }
	inline const entry_type& header(size_t c) const { return __rows[0].ith_nonzero(c); }

	// Row reference or header reference (when passing HEADER_ROW index)
	inline row_vector_type& rows_or_header(size_t r) 
	{ r++; assert(r>=0 && r<=n_rows); return __rows[r]; }
	inline const row_vector_type& rows_or_header(size_t r) const 
	{ r++; assert(r>=0 && r<=n_rows); return __rows[r]; }

	inline entry_type& entry(const row_colnnz_pair& rcp) 
	{ return rows(rcp.row).ith_nonzero(rcp.colnnz); }
	inline const entry_type& entry(const row_colnnz_pair& rcp) const
	{ return rows(rcp.row).ith_nonzero(rcp.colnnz); }

	inline entry_type& entry_or_header(const row_colnnz_pair& rcp) 
	{ return rows_or_header(rcp.row).ith_nonzero(rcp.colnnz); }
	inline const entry_type& entry_or_header(const row_colnnz_pair& rcp) const
	{ return rows_or_header(rcp.row).ith_nonzero(rcp.colnnz); }

	// inline iterator sentry() { return header().end(); }
	// inline const_iterator sentry() const { return header().end(); }
public:
    // Iterators
    typedef typename row_vector_type::iterator iterator;
    typedef typename row_vector_type::const_iterator const_iterator;

    // Interface for row iterators (non-standard)
    inline iterator begin_row(size_t r) { return iterator(rows(r).begin()); }
    inline iterator end_row(size_t r) { return iterator(rows(r).end()); }
    inline const_iterator begin_row(size_t r) const { return const_iterator(rows(r).begin()); }
    inline const_iterator end_row(size_t r) const { return const_iterator(rows(r).end()); }


	struct column_iterator {
        typedef row_colnnz_pair value_type;
        typedef typename iterator::difference_type difference_type;
        typedef row_colnnz_pair* pointer;   // should not be used.
        typedef row_colnnz_pair& reference;
        typedef std::bidirectional_iterator_tag iterator_category;

        inline column_iterator() : pos(), mat(nullptr) {}
        inline column_iterator(row_colnnz_pair _p, const doubly_linked_crs_matrix* _m) 
        : pos(_p), mat(_m) { }
        inline column_iterator(const column_iterator&) = default;
        inline column_iterator(column_iterator&&) = default;
        inline column_iterator& operator=(const column_iterator&) = default;
        inline column_iterator& operator=(column_iterator&&) = default;

        inline column_iterator& operator++ () { 
        	assert(pos.row != HEADER_ROW);
        	pos = mat->entry(pos).below;
        	return *this; 
        }
        inline column_iterator operator++ (int) { column_iterator i(*this); ++(*this); return i; }

        inline column_iterator& operator-- ()  { 
        	assert(pos.row != HEADER_ROW);
        	pos = mat->entry(pos).above;
        	return *this; 
        }
        inline column_iterator operator-- (int) { column_iterator i(*this); --(*this); return i; }

        inline bool operator == (const column_iterator &i) const { return pos == i.pos; }
        inline bool operator != (const column_iterator &i) const { return pos != i.pos; }
        inline bool operator >= (const column_iterator &i) const { return pos >= i.pos; }
        inline bool operator <= (const column_iterator &i) const { return pos <= i.pos; }
        inline bool operator >  (const column_iterator &i) const { return pos >  i.pos; }
        inline bool operator <  (const column_iterator &i) const { return pos <  i.pos; }

        inline const row_colnnz_pair* operator -> () const { return &pos; }
        inline const row_colnnz_pair& operator * () const { return pos; }

    private:
    	row_colnnz_pair                  pos;  // Position 
    	const doubly_linked_crs_matrix  *mat;  // Matrix
    	friend class doubly_linked_crs_matrix;
    };

    // Column iterators
    inline column_iterator end_col(size_t c) const 
    { return column_iterator(row_colnnz_pair(HEADER_ROW, c), this); }
    inline column_iterator begin_col(size_t c) const 
    { return column_iterator(header(c).below, this); }
    inline column_iterator last_col(size_t c) const 
    { return column_iterator(header(c).above, this); }

    // Row iterator from a column iterator
    inline iterator begin_row(const column_iterator& it) 
    { return iterator(rows(it->row).begin() + it->colnnz); }
    inline const_iterator begin_row(const column_iterator& it) const 
    { return const_iterator(rows(it->row).begin() + it->colnnz); }

    // Column iterator from a row iterator
    inline column_iterator begin_col(size_t r, const iterator& it) const { 
    	assert(rows(r).begin() <= it && it < rows(r).end());  
      	return column_iterator(row_colnnz_pair(r, it - rows(r).begin()), this);
    }
    inline column_iterator begin_col(size_t r, const const_iterator& it) const { 
    	assert(rows(r).begin() <= it && it < rows(r).end());  
      	return column_iterator(row_colnnz_pair(r, it - rows(r).begin()), this);
    }

    // Entry operations - use at your own risk.
    // Only allowed operations are changes to non-zero entries, s.t. they remain non-zero.
    inline const row_vector_type& operator[] (size_t i) const { return rows(i); }
    inline row_vector_type& operator[] (size_t i) { return rows(i); }

protected:
	// unlink an existing entry from the column chains
	inline void unlink(size_t row, const entry_type& en) {
		if (en.above.row == en.below.row) { // last entry in that column
			assert(en.above.row == HEADER_ROW);
			header(en.index).above = row_colnnz_pair(HEADER_ROW, en.index);
			header(en.index).below = row_colnnz_pair(HEADER_ROW, en.index);
			return;
		}
		entry_or_header(en.above).below = en.below;
		entry_or_header(en.below).above = en.above;
	}

	// change an entry from the column chains
	inline void relink(size_t row, size_t colnnz, const entry_type& replaced_en, entry_type& new_en) {
		new_en.below = replaced_en.below;
		new_en.above = replaced_en.above;
		entry_or_header(replaced_en.above).below = row_colnnz_pair(row, colnnz);
		entry_or_header(replaced_en.below).above = row_colnnz_pair(row, colnnz);
	}

	// Add a new entry to the column chains
	inline void link(size_t row, size_t colnnz, entry_type& en) {
		if (header(en.index).below.row == HEADER_ROW) { // no entries in this column
			header(en.index).below = header(en.index).above = row_colnnz_pair(row, colnnz); 
			en.above = en.below = row_colnnz_pair(HEADER_ROW, en.index);
			return;
		}
		column_iterator it_above, it_below;
		if (row < n_rows / 2) { // start from top
			it_below = begin_col(en.index), it_above = end_col(en.index);
			while (it_below != end_col(en.index) && it_below->row < row)
				it_above = it_below++;
		}
		else { // start from bottom
			it_above = last_col(en.index), it_below = end_col(en.index);
			while (it_above != end_col(en.index) && it_above->row > row)
				it_below = it_above--;			
		}

		en.above = *it_above;
		en.below = *it_below;
		entry_or_header(*it_above).below = row_colnnz_pair(row, colnnz);
		entry_or_header(*it_below).above = row_colnnz_pair(row, colnnz);
	}

public:
    // replace an entire row at position r
    inline void replace_row(size_t r, row_vector_type& nrow) {
    	// Relink the column indices
    	iterator oit = rows(r).begin(), nit=nrow.begin();
    	size_t colnnz=0;
    	while (nit != nrow.end()) {
    		while (oit != rows(r).end() && oit->index < nit->index) {
    			// Unlink the current *oit entry, since *nit has a zero in that position
    			// cout << "unlink r="<<r<<endl;
    			unlink(r, *oit);
    			++oit;
    		}
    		if (oit != rows(r).end() && oit->index == nit->index) { // replace the entry
    			// cout << "relink r="<<r<<"  colnnz="<<colnnz<<endl;
    			relink(r, colnnz++, *oit, *nit);
    			++oit;
    		}
    		else { // Insert the new entry
    			// cout << "link r="<<r<<"  colnnz="<<colnnz<<endl;
    			link(r, colnnz++, *nit);
    		}
    		++nit;
    	}
    	while (oit != rows(r).end()) { // Unlink remaining *oit entries
			// cout << "unlink r="<<r<<endl;
    		unlink(r, *oit);
    		++oit;
    	}
    	rows(r).swap(nrow); // replace row
    }

    // sort columns with a given permutation
    template<class T>
    inline void sort_columns(const std::vector<T>& perm) {
    	for (size_t i=0; i<n_rows + 1; i++) {
    		row_vector_type& row = __rows[i];
    		if (i==0) { // header
    			for (size_t j=0; j<n_cols; j++)
    				row.ith_nonzero(j).index = j;
    		}
    		for (auto& el : row)
	            el.index = index_type(perm[el.index]);
	        std::sort(row.data().begin(), row.data().end());
	        if (i==0) { // header
    			for (size_t j=0; j<n_cols; j++)
    				row.ith_nonzero(j).index = HEADER_ROW;
    		}
    		for (size_t cnz=0; cnz < row.nonzeros(); cnz++) {
    			if (i==0 && row.ith_nonzero(cnz).above.row == HEADER_ROW) { // empty column
					row.ith_nonzero(cnz).above.colnnz = cnz;
					row.ith_nonzero(cnz).below.colnnz = cnz;
    			}
    			else { // permute also the colnnz entries
	    			entry_or_header(row.ith_nonzero(cnz).above).below.colnnz = cnz;
	    			entry_or_header(row.ith_nonzero(cnz).below).above.colnnz = cnz;
	    		}
    		}
    		if (i > 0)
    			row.verify_invariants();
    	}
    }

    // move a column in a new position, sliding @num_pos intermediate columns backward
    inline void slide_backward(size_t col, size_t num_pos, size_t& start_row, size_t& end_row) {
        start_row = num_rows();
        end_row = 0;
        for (size_t i=0; i<num_rows() + 1; i++) {
            row_vector_type& row = __rows[i];
            if (i==0) { // header
                for (size_t j=0; j<num_cols(); j++)
                    row.ith_nonzero(j).index = j;
            }
            if (i > 0)
                row.verify_invariants();
            // remap indices
            bool changed = false;
            for (auto& el : row) {
                assert(el.index < num_cols());
                if (el.index == col + num_pos) {
                    el.index = col;
                    changed = true;
                }
                else if (col <= el.index && el.index < col + num_pos) {
                    el.index++;
                    changed = true;
                }
                assert(el.index < num_cols());
            }
            if (changed)
                std::sort(row.data().begin(), row.data().end());
            if (i==0) { // header
                for (size_t j=0; j<num_cols(); j++)
                    row.ith_nonzero(j).index = HEADER_ROW;
            }
            if (changed && i != 0) {
                start_row = std::min(start_row, i - 1);
                end_row = std::max(end_row, i - 1);
            }
            if (changed) {
                for (size_t cnz=0; cnz < row.nonzeros(); cnz++) {
                    if (i==0 && row.ith_nonzero(cnz).above.row == HEADER_ROW) { // empty column
                        row.ith_nonzero(cnz).above.colnnz = cnz;
                        row.ith_nonzero(cnz).below.colnnz = cnz;
                    }
                    else { // permute also the colnnz entries
                        entry_or_header(row.ith_nonzero(cnz).above).below.colnnz = cnz;
                        entry_or_header(row.ith_nonzero(cnz).below).above.colnnz = cnz;
                    }
                }
                if (i > 0)
                    row.verify_invariants();
            }
        }
    }

    inline void swap_rows(size_t i1, size_t i2) {
        if (i1 == i2)
            return;
        row_vector_type tmp_row = rows(i1);
        replace_row(i2, tmp_row);
        replace_row(i1, tmp_row);
    }

    inline void verify_integrity() const {
    	std::vector<row_colnnz_pair> expected(n_cols);
    	std::vector<row_colnnz_pair> previous(n_cols);
    	// Start from the header->below entries
    	for (size_t j=0; j<n_cols; j++) {
    		expected[j] = header(j).below;
    		previous[j] = row_colnnz_pair(HEADER_ROW, j);
    	}
    	// Verify that the expected entries appear in their positions
    	for (size_t i=0; i<n_rows; i++) {
    		for (size_t j=0; j<n_cols; j++) {
    			size_t lbj = rows(i).lower_bound_nnz(j);
    			if (lbj < rows(i).nonzeros() && rows(i).ith_nonzero(lbj).index == j) { 
    				// row has a nnz entry in j
    				if (expected[j] != row_colnnz_pair(i, lbj)) {
    					cerr << "Wrong below link. row="<<i<<" col="<<j<<" colnnz="<<lbj<<endl;
    					exit(-1);
    				}
    				if (previous[j] != rows(i).ith_nonzero(lbj).above) {
    					cerr << "Wrong below link. row="<<i<<" col="<<j<<" colnnz="<<lbj<<endl;
    					exit(-1);
    				}
    				previous[j] = expected[j];
    				expected[j] = rows(i).ith_nonzero(lbj).below;
    			}
    			else { // no non-zero
    				if (expected[j].row == i) {
    					cerr << "Expected entry. row="<<i<<" col="<<j<<endl;
    					exit(-1);
    				}
    			}
    		}
    	}
    	// Verify footer integrity
    	for (size_t j=0; j<n_cols; j++) {
    		if (header(j).above != previous[j]) {
    			cerr << "Wrong footer above link. col="<<j<<endl;
    			exit(-1);
    		}
    		if (expected[j] != row_colnnz_pair(HEADER_ROW, j)) {
    			cerr << "Wrong link to footer. col="<<j<<endl;
    			exit(-1);
    		}
    	}
    }

#ifdef DLS_TEST
    inline void print_entry(const entry_type& e) {
    	if (e.index == HEADER_ROW)
    		cout << "|HHH";
    	else cout << "|" << setw(3) << e.value;

    	if (e.above.row == HEADER_ROW)
    		cout << "(--@" << setw(2) << e.above.colnnz;
    	else cout << "(" << setw(2) << e.above.row << "@" << setw(2) << e.above.colnnz;

    	if (e.below.row == HEADER_ROW)
    		cout << "/--@" << setw(2) << e.below.colnnz << ")";
    	else cout << "/" << setw(2) << e.below.row << "@" << setw(2) << e.below.colnnz << ")";
    }

    inline void print() {
    	for (size_t j=0; j<n_cols; j++)
    		print_entry(header(j));
    	cout << "|" << endl;
    	for (size_t i=0; i<n_rows; i++) {
    		size_t jnz = 0;
    		for (size_t j=0; j<n_cols; j++) {
    			if (jnz < rows(i).nonzeros() && rows(i).ith_nonzero(jnz).index == j) {
	    			print_entry(rows(i).ith_nonzero(jnz++));
    			}
	    		else
	    			cout << "|  .             ";
    		}
	    	cout << "|" << endl;
    	}
    }
#endif // DLS_TEST

protected:
	// n_rows+1 rows of the matrix. Row 0 is the header.
	matrix_type   __rows;
	size_t        n_rows, n_cols;
};

//---------------------------------------------------------------------------------------
}; // namespace


//---------------------------------------------------------------------------------------
#endif // __DOUBLY_LINKED_CRS_MATRIX__









