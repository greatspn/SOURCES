//-----------------------------------------------------------------------------
/// \file methods.h
/// Implementation of various template numeric methods.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __NUMERIC_METHODS_H__
#define __NUMERIC_METHODS_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/// Fast matrix * vector product \f$ y = A \cdot x \f$.
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param A		Matrix factor.
///   \param x		Vector factor.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
/*template<class Matrix, class VectorIn, class VectorOut>
VectorOut& prod_matvec(VectorOut& y, const Matrix& A, const VectorIn& x)
{
	typedef typename Matrix::const_iterator1 const_row_iter_t;
	typedef typename Matrix::const_iterator2 const_elem_iter_t;
	const size_t M = A.size1();
	const size_t N = A.size2();
	assert(N >= 1 && M >= 1 && x.size() == N);
	y.resize(M);
	fill(y.begin(), y.end(), 0.0);

	const_row_iter_t row = A.begin1(), endRow = A.end1();
	while (row != endRow) {
		const size_t i = row.index1();
		const_elem_iter_t A_ij = row.begin(), endCol_j = row.end();
		double y_i = 0.0;
		while (A_ij != endCol_j) {
			const size_t j = A_ij.index2();
			y_i += (*A_ij) * x(j);
			++A_ij;
		}
		y(i) = y_i;
		++row;
	}

	return y;
}*/



//-----------------------------------------------------------------------------
/// Fast vector * matrix product \f$ y = x \cdot A \f$.
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param x		Vector factor.
///   \param A		Matrix factor.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
/*template<class Matrix, class VectorIn, class VectorOut>
VectorOut& prod_vecmat(VectorOut& y, const VectorIn& x, const Matrix& A)
{
	typedef typename Matrix::const_iterator1 const_row_iter_t;
	typedef typename Matrix::const_iterator2 const_elem_iter_t;
	const size_t N = A.size2();
	assert(N >= 1 && A.size1() >= 1 && x.size() == A.size1());
	y.resize(N);
	fill(y.begin(), y.end(), 0.0);

	const_row_iter_t row = A.begin1(), endRow = A.end1();
	while (row != endRow) {
		const size_t i = row.index1();
		const_elem_iter_t A_ij = row.begin(), endCol_j = row.end();
		const double x_i = x(i);
		while (A_ij != endCol_j) {
			const size_t j = A_ij.index2();
			y(j) += (*A_ij) * x_i;
			++A_ij;
		}
		++row;
	}

	return y;
}*/



//-----------------------------------------------------------------------------
/// Fast vector * matrix product \f$ y += x \cdot A \f$, added to \a y.
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param x		Vector factor.
///   \param A		Matrix factor.
///   \param rows	Rows of (y,A,x) selected for the computation.
///   \param cols	Columns of (y,A,x) selected for the computation.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &add_prod_vecmat(VectorOut &y, const VectorX &x, const Matrix &A,
                           const RowSelector &rows, const ColSelector &cols) {
    const typename Matrix::index_array_type &index1_data = A.index1_data();
    const typename Matrix::index_array_type &index2_data = A.index2_data();
    const typename Matrix::value_array_type &values_data = A.value_data();
    const size_t NR = A.size1(), NC = A.size2();
    assert(NR >= 1 && NC >= 1 && x.size() == NR && y.size() == NC);
    UNUSED_PARAM(NR); UNUSED_PARAM(NC);

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        if (i >= A.filled1() - 1)
            break;
        const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
        const double x_i = x(i);

        for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
            const size_t j = index2_data[pos2];
            if (cols.isSelected(j)) {
                const double A_ij = values_data[pos2];
                y(j) += x_i * A_ij;
            }
        }
    }
    return y;
}

//-----------------------------------------------------------------------------
/// Fast vector * matrix product \f$ y = x \cdot A \f$.
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param x		Vector factor.
///   \param A		Matrix factor.
///   \param rows	Rows of (y,A,x) selected for the computation.
///   \param cols	Columns of (y,A,x) selected for the computation.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &prod_vecmat(VectorOut &y, const VectorX &x, const Matrix &A,
                       const RowSelector &rows, const ColSelector &cols) {
    const size_t NR = A.size1(), NC = A.size2();
    assert(NR >= 1 && NC >= 1 && x.size() == NR);
    UNUSED_PARAM(NR);
    y.resize(NC);

    for (size_t c = 0; c < cols.count(); c++)
        y(cols[c]) = 0.0;

    return add_prod_vecmat(y, x, A, rows, cols);

    return y;
}


//-----------------------------------------------------------------------------
/// Fast matrix * vector product \f$ y += A \cdot x \f$, added to \a y.
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param A		Matrix factor.
///   \param x		Vector factor.
///   \param rows	Rows of (y,A,x) selected for the computation.
///   \param cols	Columns of (y,A,x) selected for the computation.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &add_prod_matvec(VectorOut &y, const Matrix &A, const VectorX &x,
                           const RowSelector &rows, const ColSelector &cols) {
    const typename Matrix::index_array_type &index1_data = A.index1_data();
    const typename Matrix::index_array_type &index2_data = A.index2_data();
    const typename Matrix::value_array_type &values_data = A.value_data();
    const size_t NR = A.size1(), NC = A.size2();
    assert(NR >= 1 && NC >= 1 && x.size() == NR && y.size() == NC);
    UNUSED_PARAM(NR); UNUSED_PARAM(NC);

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        if (i >= A.filled1() - 1)
            break;
        double y_i = 0.0;
        const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
        for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
            const size_t j = index2_data[pos2];
            if (cols.isSelected(j)) {
                const double A_ij = values_data[pos2];
                y_i += A_ij * x(j);
            }
        }
        y(i) += y_i;
    }
    return y;
}

//-----------------------------------------------------------------------------
/// Fast matrix * vector product \f$ y = A \cdot x \f$.
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param A		Matrix factor.
///   \param x		Vector factor.
///   \param rows	Rows of (y,A,x) selected for the computation.
///   \param cols	Columns of (y,A,x) selected for the computation.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &prod_matvec(VectorOut &y, const Matrix &A, const VectorX &x,
                       const RowSelector &rows, const ColSelector &cols) {
    const size_t NR = A.size1(), NC = A.size2();
    assert(NR >= 1 && NC >= 1 && x.size() == NC);
    UNUSED_PARAM(NC);
    y.resize(NR);

    for (size_t r = 0; r < rows.count(); r++)
        y(rows[r]) = 0.0;

    return add_prod_matvec(y, A, x, rows, cols);

    return y;
}


//-----------------------------------------------------------------------------
/// Fast matrix * vector product \f$ y = (A+Id) \cdot x \f$. \n
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param A		Matrix factor.
///   \param d      Diagonal vector added to A
///   \param x		Vector factor.
///   \param rows	Rows of (y,A,d,x) selected for the computation.
///   \param cols	Columns of (y,A,d,x) selected for the computation.
///   \return A reference to the destination vector \a y.
/// \note
/// \a A must be a N*N square matrix. Rectangular matrices are not supported
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorD, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &prod_matvec_diag(VectorOut &y, const Matrix &A,
                            const VectorD &d, const VectorX &x,
                            const RowSelector &rows, const ColSelector &cols) {
    const typename Matrix::index_array_type &index1_data = A.index1_data();
    const typename Matrix::index_array_type &index2_data = A.index2_data();
    const typename Matrix::value_array_type &values_data = A.value_data();
    const size_t NC = A.size2();
    assert(NC >= 1 && A.size2() == NC && x.size() == NC && d.size() == NC);
    y.resize(NC);
    fill(y.begin(), y.end(), 0.0);

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        double y_i = 0.0;
        if (i < A.filled1() - 1) {
            const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
            for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
                const size_t j = index2_data[pos2];
                if (cols.isSelected(j)) {
                    const double A_ij = values_data[pos2];
                    if (i != j)
                        y_i += A_ij * x(j);
                }
            }
        }
        y_i += (A(i, i) + d(i)) * x(i);
        y(i) = y_i;
    }

    return y;
    /*typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size2();
    assert(N >= 1 && A.size2() == N && x.size() == N && d.size() == N);
    y.resize(N);
    fill(y.begin(), y.end(), 0.0);

    for (size_t r=0; r<rows.count(); r++) {
    	const size_t i = rows[r];
    	matrix_row_t ith_row(A, i);
    	matrix_row_iterator_t A_ij = ith_row.begin();
    	double y_i = 0.0;

    	while (A_ij != ith_row.end()) {
    		const size_t j = A_ij.index();
    		if (cols.isSelected(j)) {
    			if (i != j)
    				y_i += (*A_ij) * x(j);
    		}
    		A_ij++;
    	}
    	y_i += (A(i,i) + d(i)) * x(i);
    	y(i) = y_i;
    }

    return y;*/
}



//-----------------------------------------------------------------------------
/// Fast vector * matrix product \f$ y = x \cdot (A+Id) \f$. \n
/// y must be a different vector than x
///   \param y		Destination vector.
///   \param x		Vector factor.
///   \param A		Matrix factor.
///   \param d      Diagonal vector added to A
///   \param rows	Rows of (y,A,d,x) selected for the computation.
///   \param cols	Columns of (y,A,d,x) selected for the computation.
///   \return A reference to the destination vector \a y.
/// \note
/// \a A must be a N*N square matrix. Rectangular matrices are not supported
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorD, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &prod_vecmat_diag(VectorOut &y, const VectorX &x,
                            const Matrix &A, const VectorD &d,
                            const RowSelector &rows, const ColSelector &cols) {
    const typename Matrix::index_array_type &index1_data = A.index1_data();
    const typename Matrix::index_array_type &index2_data = A.index2_data();
    const typename Matrix::value_array_type &values_data = A.value_data();
    const size_t NC = A.size2();
    assert(NC >= 1 && A.size2() == NC && x.size() == NC && d.size() == NC);
    y.resize(NC);
    fill(y.begin(), y.end(), 0.0);

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        const double x_i = x(i);
        if (i < A.filled1() - 1) {
            const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];

            for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
                const size_t j = index2_data[pos2];
                if (cols.isSelected(j)) {
                    const double A_ij = values_data[pos2];
                    if (i != j)
                        y(j) += x_i * A_ij;
                }
            }
        }
        y(i) += x(i) * (A(i, i) + d(i));
    }

    return y;
    /*typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t NC = A.size2();
    assert(NC >= 1 && A.size2() == NC && x.size() == NC && d.size() == NC);
    y.resize(NC);
    fill(y.begin(), y.end(), 0.0);

    for (size_t r=0; r<rows.count(); r++) {
    	const size_t i = rows[r];
    	matrix_row_t ith_row(A, i);
    	matrix_row_iterator_t A_ij = ith_row.begin();
    	const double x_i = x(i);

    	while (A_ij != ith_row.end()) {
    		const size_t j = A_ij.index();
    		if (cols.isSelected(j)) {
    			if (i != j)
    				y(j) += x_i * (*A_ij);
    			/ *else
    				y(j) += x_i * ((*A_ij) + d(i));* /
    		}
    		A_ij++;
    	}
    	y(i) += x(i) * (A(i,i) + d(i));
    }

    return y;*/
}


//-----------------------------------------------------------------------------
/// Fast vector product with assignment: \f$ y = y + (c \cdot x) \f$.
/// \a y and \a x may be the same vector.
///   \param y		Destination vector. Must be initialized.
///   \param c		Constant factor.
///   \param x		Source vector.
///   \param rows   Selected rows of \a x and \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorOut, class VectorIn, class Selector>
inline VectorOut &plus_assign_mult(VectorOut &y, const double c,
                                   const VectorIn &x, const Selector &rows) {
    assert(y.size() == x.size());

    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) += c * x(rows[n]);

    return y;
}

//-----------------------------------------------------------------------------
/// Fast vector x scalar product with: \f$ y = (c \cdot x) \f$.
/// \a y and \a x may be the same vector.
///   \param y		Destination vector.
///   \param c		Constant factor.
///   \param x		Source vector.
///   \param rows   Selected rows of \a x and \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorOut, class VectorIn, class Selector>
inline VectorOut &assign_mult(VectorOut &y, const double c,
                              const VectorIn &x, const Selector &rows) {
    y.resize(x.size());
    set_vec(y, 0.0, rows);

    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) = c * x(rows[n]);

    return y;
}



//-----------------------------------------------------------------------------
/// Fast vector product: \f$ y = y \cdot c \f$.
///   \param y		Destination vector. Must be initialized.
///   \param c		Constant factor.
///   \param rows   Selected elements of \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
/*template<class VectorOut, class Selector>
inline VectorOut& mult_assign(VectorOut& y, const double c,
							  const Selector & rows)
{
	for (size_t n=0; n<rows.count(); n++)
		y(rows[n]) *= c;

	return y;
}*/



//-----------------------------------------------------------------------------
/// Copy a vector: \f$ y = x \f$.
///   \param y		Destination vector.
///   \param x		Source vector
///   \param rows   Selected elements of \a x copied to \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorIn, class VectorOut, class Selector>
inline VectorOut &copy_vec(VectorOut &y, const VectorIn &x,
                           const Selector &rows) {
    if (y.size() == 0)
        y.resize(x.size());

    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) = x(rows[n]);
    return y;
}


//-----------------------------------------------------------------------------
/// Add a vector into another: \f$ y += x \f$.
///   \param y		Destination vector.
///   \param x		Source vector
///   \param rows   Selected elements of \a x added to \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorIn, class VectorOut, class Selector>
inline VectorOut &add_vec(VectorOut &y, const VectorIn &x,
                          const Selector &rows) {
    assert(x.size() == y.size());

    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) += x(rows[n]);
    return y;
}



//-----------------------------------------------------------------------------
/// Fast vector product: \f$ y = y \cdot c \f$.
///   \param y		Destination vector. Must be initialized.
///   \param c		Constant factor.
///   \param rows   Selected elements of \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorOut, class Selector>
inline VectorOut &mult_vec(VectorOut &y, const double factor,
                           const Selector &rows) {
    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) *= factor;
    return y;
}


//-----------------------------------------------------------------------------
/// Fast vector division: \f$ y = y / c \f$.
///   \param y		Destination vector. Must be initialized.
///   \param c		Constant factor.
///   \param rows   Selected elements of \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorOut, class Selector>
inline VectorOut &div_vec(VectorOut &y, const double factor, const Selector &rows) {
    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) /= factor;
    return y;
}



//-----------------------------------------------------------------------------
/// Add a constant factor to every vector elements: \f$ y += c \f$.
///   \param y		Destination vector. Must be initialized.
///   \param c		Constant factor.
///   \param rows   Selected elements of \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorOut, class Selector>
inline VectorOut &add_vec(VectorOut &y, const double factor,
                          const Selector &rows) {
    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) += factor;
    return y;
}


//-----------------------------------------------------------------------------
/// Assign a constant to every vector elements: \f$ y = c \f$.
///   \param y		Destination vector. Must be initialized.
///   \param c		Assigned constant.
///   \param rows   Selected elements of \a y.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorOut, class Selector>
inline VectorOut &set_vec(VectorOut &y, const double factor,
                          const Selector &rows) {
    for (size_t n = 0; n < rows.count(); n++)
        y(rows[n]) = factor;
    return y;
}


//-----------------------------------------------------------------------------
/// Assign a constant to every vector elements: \f$ y = c \f$.
///   \param y		Destination vector. Must be initialized.
///   \param c		Assigned constant.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorOut>
inline VectorOut &set_vec(VectorOut &y, const double factor) {
    for (size_t n = 0; n < y.size(); n++)
        y(n) = factor;
    return y;
}


//-----------------------------------------------------------------------------
/// 1-Norm: \f$ c = |x|_1 \f$.
///   \param x		Source vector. Must be initialized.
///   \param rows   Selected elements of \ax.
///   \return The 1-norm of \a x.
//-----------------------------------------------------------------------------
template<class VectorIn, class Selector>
inline double norm_1(VectorIn &x, const Selector &rows) {
    double c = 0.0;
    for (size_t n = 0; n < rows.count(); n++)
        c += std::abs(x(rows[n]));
    return c;
}


//-----------------------------------------------------------------------------
/// Component sum: \f$ c = \sum x_i \f$.
///   \param x		Source vector. Must be initialized.
///   \param rows   Selected elements of \ax.
///   \return The component sum of \a x.
//-----------------------------------------------------------------------------
template<class VectorIn, class Selector>
inline double vec_sum(const VectorIn &x, const Selector &rows) {
    double c = 0.0;
    for (size_t n = 0; n < rows.count(); n++)
        c += x(rows[n]);
    return c;
}



//-----------------------------------------------------------------------------
/// Component sum: \f$ c = \sum x_i \f$.
///   \param x		Source vector. Must be initialized.
///   \return The component sum of \a x.
//-----------------------------------------------------------------------------
template<class VectorIn>
inline double vec_sum(const VectorIn &x) {
    double c = 0.0;
    for (size_t i = 0; i < x.size(); i++)
        c += x(i);
    return c;
}


//-----------------------------------------------------------------------------
/// Component dot-product: \f$ c = x \cdot y \f$.
///   \param x		First vector. Must be initialized.
///   \param y		Second vector. Must be initialized.
///   \param rows   Selected elements of \ax.
///   \return The dot product of \a x.
//-----------------------------------------------------------------------------
template<class VectorIn1, class VectorIn2, class Selector>
inline double vec_dot(const VectorIn1 &x, const VectorIn2 &y, const Selector &rows) {
    assert(x.size() == y.size());
    double c = 0.0;
    for (size_t n = 0; n < rows.count(); n++)
        c += x(rows[n]) * y(rows[n]);
    return c;
}



//-----------------------------------------------------------------------------
/// Multiply \a x with the uniformized matrix, and assign the result
/// to \a y: \f$ y = x * (A/lambda + I) \f$.
///   \param y		Destination vector. Must be initialized.
///   \param x		Source vector multiplied with the uniformized matrix
///   \param A      Non uniformized matrix.
///   \param lambda Uniformization coefficient
///   \param rows   Selected rows of \a A and \a x.
///   \param cols   Selected columns of \a A and \a y.
///   \return A reference to the destination vector \a y.
/// \note The uniformized matrix is given by \f$ (A/lambda + I) \f$.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &prod_vecmat_unif(VectorOut &y, const VectorX &x,
                            const Matrix &A, const double lambda,
                            const RowSelector &rows, const ColSelector &cols) {
    const typename Matrix::index_array_type &index1_data = A.index1_data();
    const typename Matrix::index_array_type &index2_data = A.index2_data();
    const typename Matrix::value_array_type &values_data = A.value_data();
    const size_t NR = A.size1(), NC = A.size2();
    UNUSED_PARAM(NR);
    assert(NR >= 1 && NC >= 1 && x.size() == NR);
    y.resize(NC);
    set_vec(y, 0.0, cols);

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        const double x_i = x(i);
        if (i >= A.filled1() - 1)
            break;
        const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];

        for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
            const size_t j = index2_data[pos2];
            if (cols.isSelected(j)) {
                const double A_ij = values_data[pos2];
                if (i != j)
                    y(j) += x_i * A_ij;
            }
        }
    }
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        y(i) /= lambda;
        if (cols.isSelected(i))
            y(i) += x(i) * (A(i, i) / lambda + 1.0);
    }

    return y;
}


//-----------------------------------------------------------------------------
/// Multiply \a x with the uniformized matrix, and assign the result
/// to \a y: \f$ y = (A/lambda + I) * x \f$.
///   \param y		Destination vector. Must be initialized.
///   \param A      Non uniformized matrix.
///   \param x		Source vector multiplied with the uniformized matrix
///   \param lambda Uniformization coefficient
///   \param rows   Selected rows of \a A and \a x.
///   \param cols   Selected columns of \a A and \a y.
///   \return A reference to the destination vector \a y.
/// \note The uniformized matrix is given by \f$ (A/lambda + I) \f$.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorX, class VectorOut,
         class RowSelector, class ColSelector>
VectorOut &prod_matvec_unif(VectorOut &y, const Matrix &A,
                            const VectorX &x, const double lambda,
                            const RowSelector &rows, const ColSelector &cols) {
    // TODO: untested
    const typename Matrix::index_array_type &index1_data = A.index1_data();
    const typename Matrix::index_array_type &index2_data = A.index2_data();
    const typename Matrix::value_array_type &values_data = A.value_data();
    const size_t NR = A.size1(), NC = A.size2();
    UNUSED_PARAM(NC);
    assert(NR >= 1 && NC >= 1 && x.size() == NR);
    y.resize(NR);
    set_vec(y, 0.0, cols);

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        double y_i = y(i);
        if (i >= A.filled1() - 1)
            break;
        const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
        for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
            const size_t j = index2_data[pos2];
            if (cols.isSelected(j)) {
                const double A_ij = values_data[pos2];
                if (i != j)
                    y_i += A_ij * x(j);
            }
        }
        y_i /= lambda;
        if (cols.isSelected(i))
            y_i += x(i) * (A(i, i) / lambda + 1.0);
        y(i) = y_i;
    }

    return y;
}


//-----------------------------------------------------------------------------
/// Vector * matrix product with discretization of \a M:
/// \f$ out += -in \cdot (diag^{-1}(M)) \times M \f$.
/// y must be a different vector than x. The diag operation is safe and works
/// also with zero diagonals.
///   \param y		Destination vector.
///   \param x		Vector factor.
///   \param M		Matrix factor.
///   \param rows	Rows of (y,A,x) selected for the computation.
///   \param cols   Columns of (y,A,x) selected for the computation.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorIn, class VectorOut, class Matrix,
         class RowSelector, class ColSelector>
inline VectorOut &prod_vec_invDiagM_M(VectorOut &outVec, const VectorIn &inVec,
                                      const Matrix &M, const RowSelector &rows,
                                      const ColSelector &cols) {
    const typename Matrix::index_array_type &index1_data = M.index1_data();
    const typename Matrix::index_array_type &index2_data = M.index2_data();
    const typename Matrix::value_array_type &values_data = M.value_data();
    assert(outVec.size() == inVec.size() && outVec.size() == M.size1());

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        if (i >= M.filled1() - 1)
            break;
        const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
        const double invDiagMii = (M(i, i) == 0.0) ? 1.0 : -1.0 / M(i, i);
        double in_i = inVec(i);

        for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
            const size_t j = index2_data[pos2];
            if (cols.isSelected(j)) {
                const double M_ij = values_data[pos2];
                outVec(j) += in_i * (M_ij * invDiagMii);
            }
        }
    }
    return outVec;
}


//-----------------------------------------------------------------------------
/// Matrix * vector product with discretization of \a M:
/// \f$ out += - (diag^{-1}(M)) \times M \cdot in \f$.
/// y must be a different vector than x. The diag operation is safe and works
/// also with zero diagonals.
///   \param y		Destination vector.
///   \param x		Vector factor.
///   \param M		Matrix factor.
///   \param rows	Rows of (y,A,x) selected for the computation.
///   \param cols   Columns of (y,A,x) selected for the computation.
///   \return A reference to the destination vector \a y.
//-----------------------------------------------------------------------------
template<class VectorIn, class VectorOut, class Matrix,
         class RowSelector, class ColSelector>
inline VectorOut &prod_invDiagM_M_vec(VectorOut &outVec, const Matrix &M,
                                      const VectorIn &inVec,
                                      const RowSelector &rows,
                                      const ColSelector &cols) {
    const typename Matrix::index_array_type &index1_data = M.index1_data();
    const typename Matrix::index_array_type &index2_data = M.index2_data();
    const typename Matrix::value_array_type &values_data = M.value_data();
    assert(outVec.size() == inVec.size() && outVec.size() == M.size2());

    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        if (i >= M.filled1() - 1)
            break;
        const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
        const double invDiagMii = (M(i, i) == 0.0) ? 1.0 : -1.0 / M(i, i);
        //double in_i = inVec(i);
        double out_i = 0.0;

        for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
            const size_t j = index2_data[pos2];
            if (cols.isSelected(j)) {
                const double M_ij = values_data[pos2];
                //outVec(j) += in_i * (M_ij * invDiagMii);
                out_i += (M_ij * invDiagMii) * inVec(j);
            }
        }
        outVec(i) = out_i;
    }
    return outVec;
}


//-----------------------------------------------------------------------------
/// Constructs a new matrix outM which has only the entries in the selected
/// rows and columns, while preserving the dimensions of the input matrix
///  \param inM   Input matrix.
///  \param outM  Output filtered matrix.
///  \param rows  Filtered rows.
///  \param cols  Filtered columns.
//-----------------------------------------------------------------------------
template<class Matrix, class RowSelector, class ColSelector>
void filter_matrix(const Matrix &inM, Matrix &outM,
                   const RowSelector &rows, const ColSelector &cols) {
    outM.resize(inM.size1(), inM.size2(), false);

    typename Matrix::const_iterator1 row(inM.begin1()), endRows(inM.end1());
    for (; row != endRows; ++row) {
        const size_t i = row.index1();
        if (!rows.isSelected(i))
            continue;
        typename Matrix::const_iterator2 A_ij(row.begin()), endRow_i(row.end());
        for (; A_ij != endRow_i; ++A_ij) {
            const size_t j = A_ij.index2();
            if (!cols.isSelected(j))
                continue;
            outM.insert_element(i, j, *A_ij);
        }
    }
}


//-----------------------------------------------------------------------------
/// Constructs a new (smaller) matrix \a outM which is the row/column projection
/// of the input matrix. The size is rows.count() x cols.count()
///  \param inM   Input matrix.
///  \param outM  Output projected matrix.
///  \param rows  Projection rows.
///  \param cols  Projection columns.
//-----------------------------------------------------------------------------
template<class Matrix, class RowSelector, class ColSelector>
void project_matrix(const Matrix &inM, Matrix &outM,
                    const RowSelector &rows, const ColSelector &cols) {
    outM.resize(rows.count(), cols.count(), false);

    // inverse map of the row/column selectors
    std::vector<size_t> rowInv(inM.size1()), colInv(inM.size2());
    for (size_t k = 0, r = 0; k < inM.size1(); k++)
        rowInv[k] = rows.isSelected(k) ? r++ : size_t(-1);
    for (size_t k = 0, c = 0; k < inM.size2(); k++)
        colInv[k] = cols.isSelected(k) ? c++ : size_t(-1);

    typename Matrix::const_iterator1 row(inM.begin1()), endRows(inM.end1());
    for (; row != endRows; ++row) {
        const size_t i = row.index1();
        if (!rows.isSelected(i))
            continue;
        const size_t ii = rowInv[i];
        assert(ii != size_t(-1));
        typename Matrix::const_iterator2 A_ij(row.begin()), endRow_i(row.end());
        for (; A_ij != endRow_i; ++A_ij) {
            const size_t j = A_ij.index2();
            if (!cols.isSelected(j))
                continue;
            const size_t jj = colInv[j];
            assert(jj != size_t(-1));
            outM.insert_element(ii, jj, *A_ij);
        }
    }
}


//-----------------------------------------------------------------------------
/// Constructs a new (smaller) matrix \a outM which is the row/column projection
/// of the input matrix. The size is rows.count() x cols.count()
///  \param inM   Input matrix.
///  \param outM  Output projected matrix.
///  \param rows  Projection rows.
///  \param cols  Projection columns.
///  \param colsInv  Inverted projection columns.
//-----------------------------------------------------------------------------
template<class Matrix, class RowSelector, class ColSelector, class InvColSelector>
void project_matrix_inv(const Matrix &inM, Matrix &outM,
                        const RowSelector &rows,
                        const ColSelector &cols,
                        const InvColSelector &colsInv) {
    outM.resize(rows.count(), cols.count(), false);

    const typename Matrix::index_array_type &index1_data = inM.index1_data();
    const typename Matrix::index_array_type &index2_data = inM.index2_data();
    const typename Matrix::value_array_type &values_data = inM.value_data();

    for (size_t proj_i = 0; proj_i < rows.count(); proj_i++) {
        const size_t i = rows[proj_i];
        if (i >= inM.filled1() - 1)
            break;
        const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];

        for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
            const size_t j = index2_data[pos2];
            if (cols.isSelected(j)) {
                //const size_t proj_j = colsInv[j];
                const size_t proj_j = colsInv.positionInsideSubset(j);
                assert(cols[proj_j] == j);
                const double inM_ij = values_data[pos2];
                outM.insert_element(proj_i, proj_j, inM_ij);
            }
        }
    }


    // inverse map of the row/column selectors
    /*std::vector<int> rowInv(inM.size1()), colInv(inM.size2());
    for (size_t k=0, r=0; k<inM.size1(); k++)
    	rowInv[k] = rows.isSelected(k) ? r++ : size_t(-1);
    for (size_t k=0, c=0; k<inM.size2(); k++)
    	colInv[k] = cols.isSelected(k) ? c++ : size_t(-1);*/

    /*typename Matrix::const_iterator1 row(inM.begin1()), endRows(inM.end1());
    for (; row != endRows; ++row) {
    	const size_t i = row.index1();
    	if (!rows.isSelected(i))
    		continue;
    	const size_t ii = rowInv[i];
    	assert(ii != size_t(-1));
    	typename Matrix::const_iterator2 A_ij(row.begin()), endRow_i(row.end());
    	for (; A_ij != endRow_i; ++A_ij) {
    		const size_t j = A_ij.index2();
    		if (!cols.isSelected(j))
    			continue;
    		const size_t jj = colInv[j];
    		assert(jj != size_t(-1));
    		outM.insert_element(ii, jj, *A_ij);
    	}
    }*/
}


//-----------------------------------------------------------------------------
/// Invert the matrix projection, by filling with zeroes the new elements.
///  \param inM   Input matrix.
///  \param outM  Output matrix.
///  \param NR    Output matrix rows.
///  \param NC    Output matrix columns.
///  \param rows  Projection rows.
///  \param cols  Projection columns.
//-----------------------------------------------------------------------------
template<class Matrix, class RowSelector, class ColSelector>
void unproject_matrix(const Matrix &inM, Matrix &outM,
                      size_t NR, size_t NC,
                      const RowSelector &rows, const ColSelector &cols) {
    outM.resize(NR, NC, false);

    typename Matrix::const_iterator1 row(inM.begin1()), endRows(inM.end1());
    for (; row != endRows; ++row) {
        const size_t i = row.index1();
        const size_t ii = rows[i];
        assert(ii < NR);
        typename Matrix::const_iterator2 A_ij(row.begin()), endRow_i(row.end());
        for (; A_ij != endRow_i; ++A_ij) {
            const size_t j = A_ij.index2();
            const size_t jj = cols[j];
            assert(jj < NC);
            outM.insert_element(ii, jj, *A_ij);
        }
    }
}


//-----------------------------------------------------------------------------
/// Constructs a new (smaller) vector \a outV which is the projection
/// of the input vector. The new size is sel.count()
///  \param inV   Input vector.
///  \param outV  Output vector.
///  \param sel  Projection elements.
//-----------------------------------------------------------------------------
template<class VectorIn, class VectorOut, class Selector>
void project_vector(const VectorIn &inV, VectorOut &outV, const Selector &sel) {
    outV.resize(sel.count());
    for (size_t proj_i = 0; proj_i < outV.size(); proj_i++) {
        const size_t i = sel[proj_i];
        outV[proj_i] = inV[i];
    }
    /*for (size_t i=0, c=0; i<inV.size(); i++) {
    	if (sel.isSelected(i))
    		outV[c++] = inV[i];
    }*/
}


//-----------------------------------------------------------------------------
/// Invert the projection, by filling with zeroes the inserted elements.
///  \param inV   Input vector.
///  \param outV  Output vector.
///  \param N     Output vector dimension.
///  \param sel   Projection elements.
//-----------------------------------------------------------------------------
template<class VectorIn, class VectorOut, class Selector>
void unproject_vector(const VectorIn &inV, VectorOut &outV,
                      size_t N, const Selector &sel) {
    outV.resize(N);
    std::fill(outV.begin(), outV.end(), 0.0);
    for (size_t i = 0; i < sel.count(); i++)
        outV[ sel[i] ] = inV[i];
}



//-----------------------------------------------------------------------------
/// Infinity norm for matrices.
///  \param A   Input matrix
///  \return    The infinity norm of \a A.
//-----------------------------------------------------------------------------
template<class Matrix>
typename Matrix::value_type
NormInf(const Matrix &A) {
    typename Matrix::value_type norm = 0, norm_i;
    typename Matrix::const_iterator1 row(A.begin1()), endRows(A.end1());
    while (row != endRows) {
        typename Matrix::const_iterator2 A_ij(row.begin()), endRow_i(row.end());
        norm_i = 0;
        while (A_ij != endRow_i) {
            norm_i += std::abs(*A_ij);
            ++A_ij;
        }
        norm = std::max(norm, norm_i);
        ++row;
    }

    return norm;
}


//-----------------------------------------------------------------------------
/// Matrix inversion. Works for dense matrices with the LU factorization of ublas
///  \param input  Input matrix that will be inverted
///  \return       The inverse of \a input.
///  \note Thorws an exception if the \a input matrix is singular.
//-----------------------------------------------------------------------------
template<class E>
ublas::matrix<typename E::value_type>
Inverse(const ublas::matrix_expression<E> &input) {
    typedef typename E::value_type T;
    typedef ublas::permutation_matrix<std::size_t> pmatrix;

    // create a working copy of the input
    ublas::matrix<T> A(input), inverse;
    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());
    // perform LU-factorization
    int res = ublas::lu_factorize(A, pm);
    if (res != 0)
        throw std::exception(); // Inverse: input matrix is singular.

    // create identity matrix of "inverse"
    ublas::identity_matrix<T> I(A.size1());
    inverse = I;
    // back-substitute to get the inverse
    ublas::lu_substitute(A, pm, inverse);
    return inverse;
}


inline double safe_inv(double v) { return (v == 0) ? 1 : 1 / v; }

//-----------------------------------------------------------------------------
/// Do a back-substitution with the Upper Triangular A matrix
///  \param A  Upper part of the matrix in the back-substitution method.
///  \param Diag Diagonal used in he back-substitution method.
///  \param b  Input vector.
///  \param x  Output vector.
//-----------------------------------------------------------------------------
template<class MatA, class MatD, class VecB, class VecX>
void BackSubstitution(const MatA &A, const MatD &Diag, const VecB &b, VecX &x) {
    const typename MatA::index_array_type &index1_data = A.index1_data();
    const typename MatA::index_array_type &index2_data = A.index2_data();
    const typename MatA::value_array_type &values_data = A.value_data();
    const size_t NR = A.size1();
    assert(b.size() == x.size() && x.size() == NR);

    for (ssize_t i = ssize_t(NR - 1); i >= 0; i--) {
        double xi_next = b[i];

        // x[i] = ( b[i] - SUM{j=i+1 to N}(A[i,j] * x[j]) ) / D[i,i]
        if (i < ssize_t(A.filled1() - 1)) {
            const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
            for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
                const size_t j = index2_data[pos2];
                if (j > (size_t)i) {
                    const double A_ij = values_data[pos2];
                    xi_next -= A_ij * x[j];
                }
            }
        }
        double diag_i = Diag(i, i);
        if (diag_i != 0)
            xi_next /= diag_i;
        x[i] = xi_next;
    }
    /*typedef typename ublas::matrix_row<const MatA>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    assert(b.size() == x.size() && x.size() == A.size1());

    for (ssize_t i = ssize_t(N-1); i >= 0; i--) {
    	double xi_next = b[i];
    	matrix_row_t ith_row(A, i);
    	matrix_row_iterator_t A_ij(ith_row.begin()), A_iEnd(ith_row.end());

    	// x[i] = ( b[i] - SUM{j=i+1 to N}(A[i,j] * x[j]) ) / D[i,i]
    	for (; A_ij != A_iEnd; ++A_ij) {
    		const ssize_t j = ssize_t(A_ij.index());
    		if (j > i)
    			xi_next -= (*A_ij) * x[j];
    	}
    	double diag_i = Diag(i,i);
    	if (diag_i != 0)
    		xi_next /= diag_i;
    	x[i] = xi_next;

    	if (x[i] != x2[i]) {
    		cout << "BackSubstitution: x["<<i<<"] = " << x[i] << ", x2["<<i<<"] = "<<x2[i]<<endl;
    		exit(-1);
    	}
    }//*/
}

//-----------------------------------------------------------------------------
/// Do a fore-substitution with the Lower Triangular A matrix
///  \param A  Upper part of the matrix in the fore-substitution method.
///  \param Diag Diagonal used in he fore-substitution method.
///  \param b  Input vector.
///  \param x  Output vector.
//-----------------------------------------------------------------------------
template<class MatA, class MatD, class VecB, class VecX>
void ForeSubstitution(const MatA &A, const MatD &Diag, const VecB &b, VecX &x) {
    // typedef typename ublas::matrix_row<const MatA>  matrix_row_t;
    // typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const typename MatA::index_array_type &index1_data = A.index1_data();
    const typename MatA::index_array_type &index2_data = A.index2_data();
    const typename MatA::value_array_type &values_data = A.value_data();
    const size_t NR = A.size1();
    assert(b.size() == x.size() && x.size() == NR);

    for (size_t i = 0; i < NR; i++) {
        double xi_next = b[i];

        // x[i] = ( b[i] - SUM{j=0 to i}(A[i,j] * x[j]) ) / D[i,i]
        if (i < A.filled1() - 1) {
            const size_t i2_start = index1_data[i], i2_end = index1_data[i + 1];
            for (size_t pos2 = i2_start; pos2 < i2_end; ++pos2) {
                const size_t j = index2_data[pos2];
                if (j >= i)
                    break;
                const double A_ij = values_data[pos2];
                xi_next -= A_ij * x[j];
            }
        }
        double diag_i = Diag(i, i);
        if (diag_i != 0)
            xi_next /= diag_i;
        x[i] = xi_next;
    }
    /*typedef typename ublas::matrix_row<const MatA>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    assert(b.size() == x.size() && x.size() == A.size1());

    for (size_t i = 0; i < N; i++) {
    	double xi_next = b[i];
    	matrix_row_t ith_row(A, i);
    	matrix_row_iterator_t A_ij(ith_row.begin()), A_iEnd(ith_row.end());

    	// x[i] = ( b[i] - SUM{j=0 to i}(A[i,j] * x[j]) ) / D[i,i]
    	for (; A_ij != A_iEnd; ++A_ij) {
    		const size_t j = A_ij.index();
    		if (j >= i)
    			break;
    		xi_next -= (*A_ij) * x[j];
    	}
    	double diag_i = Diag(i,i);
    	if (diag_i != 0)
    		xi_next /= diag_i;
    	x[i] = xi_next;

    	if (x[i] != x2[i]) {
    		cout << "ForeSubstitution: x["<<i<<"] = " << x[i] << ", x2["<<i<<"] = "<<x2[i]<<endl;
    		exit(-1);
    	}
    }//*/
}





#if defined (__APPLE__)
# define USE_UNICODE_TTY
#endif

//-----------------------------------------------------------------------------
/// Print out a matrix onto the specified stream
///   \param os		Output stream.
///	  \param A		Printed matrix.
///   \param name   Name of the matrix.
//-----------------------------------------------------------------------------
template<class ostream_t, class Matrix>
inline void print_matrix(ostream_t &os, const Matrix &A, const char *name) {
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    char buf[32];

    if (A.size1() == 1) { // Only a single row, write A as an array
        os << name << " = [";
        for (size_t j = 0; j < A.size2(); j++) {
            sprintf(buf, "%.5lg", A(0, j));
            os << (j > 0 ? "  " : " ") << buf;
        }
        os << " ]" << endl;
        return;
    }

    size_t Rows = A.size1();
    size_t Cols = A.size2();
    vector<int> ColIntSz(Cols, 0);
    vector<int> ColFracSz(Cols, 0);

    // Determine column sizes
    for (size_t i = 0; i < Rows; i++) {
        matrix_row_t ith_row(A, i);
        matrix_row_iterator_t A_ij = ith_row.begin();
        while (A_ij != ith_row.end()) {
            const size_t j = A_ij.index();
            sprintf(buf, "%.5lg", *A_ij);
            int len = strlen(buf);
            int intPartSz = 0;
            for (int k = 0; k < len && buf[k] && buf[k] != '.' && buf[k] != ','; k++)
                intPartSz++;
            int fracSz = max(0, len - intPartSz - 1);
            /*cout << buf << ": len="<<len<<" intPartSz="<<intPartSz<<" ";
            cout << "fracSz="<<fracSz<<endl;//*/
            ColIntSz[j] = max(ColIntSz[j], intPartSz);
            ColFracSz[j] = max(ColFracSz[j], fracSz);
            ++A_ij;
        }
    }
    /*for (size_t j=0; j<Cols; j++) {
    	cout << "ColIntSz["<<j<<"] = " << ColIntSz[j] << "   ";
    	cout << "ColFracSz["<<j<<"] = " << ColFracSz[j] << endl;
    }//*/

    // Print the matrix
    size_t nameLen = strlen(name);
    for (size_t i = 0; i < Rows; i++) {
        if (i == Rows / 2)
            os << name << " = ";
        else {
            for (size_t nb = 0; nb < nameLen; nb++)
                os << " ";
            os << "   ";
        }
#ifndef USE_UNICODE_TTY
        os << "|  ";
#else
        os << (i == 0 ? "⎡" : (i == Rows - 1 ? "⎣" : "⎢")) << "  ";
#endif

        for (size_t j = 0; j < Cols; j++) {
            double val = A(i, j);
            sprintf(buf, "%.5lg", val);
            int len = strlen(buf);
            int intPartSz = 0;
            for (int k = 0; k < len && buf[k] && buf[k] != '.' && buf[k] != ','; k++)
                intPartSz++;
            int leftPad = ColIntSz[j] - intPartSz;
            for (int k = 0; k < leftPad; k++)
                os << " ";
            os << buf;
            int elemLen = ColIntSz[j] + ColFracSz[j] + (ColFracSz[j] ? 1 : 0);
            int rightPad = elemLen - len - leftPad;
            for (int k = 0; k < rightPad; k++)
                os << " ";
#ifndef USE_UNICODE_TTY
            os << "  ";
#else
            os << (j == Cols - 1 ? " " : "  ");
#endif
        }
#ifndef USE_UNICODE_TTY
        os << "|\n";
#else
        os << (i == 0 ? "⎤" : (i == Rows - 1 ? "⎦" : "⎥")) << "\n";
#endif
    }
    os << flush;
}



//-----------------------------------------------------------------------------
/// Intermediate class that implements a matrix-like interface for
/// filling a matrix in sparse order, and for compressing it at the end.
//-----------------------------------------------------------------------------
template<class T> class matrix_map {
public:
    typedef std::map<pair<size_t, size_t>, T>  map_type;
    typedef typename map_type::const_iterator  const_iterator;

    /// Creates an empty matrix_map
    ///   \param numRows  Number of rows in the matrix.
    ///   \param numCols  Number of columns in the matrix.
    matrix_map(size_t numRows, size_t numCols) : NI(numRows), NJ(numCols) { }

    /// Returns a reference to the (i,j)-th element, eventually allocating it.
    inline T &operator()(size_t i, size_t j) {
        assert(i < NI && j < NJ);
        return mat[make_pair(i, j)];
    }

    /// Returns the value of the (i,j)-th element.
    inline T operator()(size_t i, size_t j) const {
        assert(i < NI && j < NJ);
        typename map_type::const_iterator it = mat.find(make_pair(i, j));
        if (it != mat.end())
            return it->second;
        else return T(0);
    }

    /// Number of non-zeroes element in the mapped matrix.
    inline size_t nnz() const {  return mat.size();  }

    /// Number of matrix rows.
    inline size_t size1() const {  return NI;  }

    /// Number of matrix columns.
    inline size_t size2() const {  return NJ;  }

    /// Iterator to the first nonzero element of the ordered map.
    inline const_iterator begin() const {  return mat.begin();  }

    /// Iterator to the end nonzero element of the ordered map.
    inline const_iterator end() const {  return mat.end();  }

    /// Clears the content of this matrix_map
    inline void clear() {  mat.clear();  }

    /// Grows the dimensions of this matrix_map
    inline void enlarge(size_t numRows, size_t numCols)
    {  assert(numRows >= NI && numCols >= NJ); NI = numRows; NJ = numCols;  }

protected:
    std::map<pair<size_t, size_t>, T> mat;
    size_t NI, NJ;
};

//-----------------------------------------------------------------------------
/// Compress a matrix_map instance into a ublas::compressed_matrix.
///   \param dst   Destination matrix (will be cleared before the compression).
///   \param src   Source mapped matrix.
//-----------------------------------------------------------------------------
template<class T>
void compress_matrix(ublas::compressed_matrix<T> &dst, const matrix_map<T> &src) {
    ublas::compressed_matrix<T> M(src.size1(), src.size2(), src.nnz());
    typename matrix_map<T>::const_iterator it, it_end;
    it = src.begin();
    it_end = src.end();
    while (it != it_end) {
        M.insert_element(it->first.first, it->first.second, it->second);
        ++it;
    }
    dst.swap(M);
}


//-----------------------------------------------------------------------------
/// Fast transposition of a sparse matrix.
///  \param in  Input matrix.
///  \param out Matrix that will hold the transpose of \a in.
///  \return A reference of the \a out matrix.
//-----------------------------------------------------------------------------
template<class T>
ublas::compressed_matrix<T> &
FastTranspose(const ublas::compressed_matrix<T> &in,
              ublas::compressed_matrix<T> &out) {
    const size_t N = in.size1();
    assert(N == in.size2());
    numerical::matrix_map<double> M(N, N);

    ublas::compressed_matrix<double>::const_iterator1 it1(in.begin1()), it1end(in.end1());
    for (; it1 != it1end; ++it1) {
        const size_t i = it1.index1();
        ublas::compressed_matrix<double>::const_iterator2 it2(it1.begin()), it2end(it1.end());
        for (; it2 != it2end; ++it2) {
            const size_t j = it2.index2();
            M(j, i) = *it2;
        }
    }
    compress_matrix(out, M);
    return out;
}

//-----------------------------------------------------------------------------
/// Fast transposition of a sparse matrix.
///  \param in  Input matrix.
///  \param out Matrix that will hold the transpose of \a in.
///  \return A reference of the \a out matrix.
//-----------------------------------------------------------------------------
template<class T>
ublas::matrix<T> &
FastTranspose(const ublas::matrix<T> &in, ublas::matrix<T> &out) {
    out = trans(in);
    return out;
}




//-----------------------------------------------------------------------------
/// A vector of doubles that can hold data with different magnitude orders,
/// avoiding underflows.
//-----------------------------------------------------------------------------
class multiorder_vector {
public:
    /// Creates a new multiorder_vector
    ///  \param N        vector size
    ///  \param maxVal   maximum stored value
    ///  \param minVal   minimum stored value
    ///  \param _stepExp exponent delta between 2 separate vectors
    multiorder_vector(size_t N, double maxVal, double minVal, int _stepExp);

    /// Adds a value into the vector at the specified position
    ///   \param i    vector row
    ///   \param v    value added to row i
    inline void add(size_t i, double v);

    /// Packs the various values into a single vector
    ///   \param outVec  target vector that will be filled with the accumulated values
    void pack(ublas::vector<double> &outVec) const;

    /// Returns the number of differentiated vectors used
    inline int num_orders() const  {  return numOrders;  }

protected:
    int numOrders, baseExp, stepExp;
    ublas::matrix<double> movec;
};

//-----------------------------------------------------------------------------

inline void multiorder_vector::add(size_t i, double v) {
    int e, moind;
    frexp(v, &e);
    moind = min(numOrders - 1, max(0, (e - baseExp) / stepExp));
    double cv = movec(i, moind);
    cv += v;

    // Test if cv should be moved into another magnitude vector
    int cve, cmoind;
    frexp(cv, &cve);
    cmoind = min(numOrders - 1, max(0, (cve - baseExp) / stepExp));
    if (cmoind != moind) {
        movec(i, moind) = 0.0;
        add(i, cv);
    }
    else {
        movec(i, moind) = cv;
    }
}



//-----------------------------------------------------------------------------
/* @}  Numerical documentation group. */
#endif  // __NUMERIC_METHODS_H__
