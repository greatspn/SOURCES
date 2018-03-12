//-----------------------------------------------------------------------------
/// \file stationary.h
/// Iterative Steady-State template methods.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __STATIONARY_METHODS_H__
#define __STATIONARY_METHODS_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/// Jacobi iteration for a linear equation system \f$(A+Id) \cdot x=b\f$.
///   \param A		 Matrix with the linear equation system.
///   \param x		 Solution vector.
///   \param b		 Column vector b.
///   \param d		 Diagonal vector added to A.
///   \param omega	 Over-relaxation parameter (JOR method), 0<\a omega<2.
///   \param x_next  Auxiliary vector needed in the Jacobi method.
///   \param max_err On return contains the maximum difference between elements
///                  of \a x and \a x'.
///   \param x_norm  Norm-1 of \a x after this Jacobi iteration.
///   \param rows	 Rows of (A,x,b) selected for the computation.
///   \param cols	 Columns of (A,x,b) selected for the computation.
///
/// \note
/// This method performs a single iteration of the Jacobi method over \a x
/// for linear system \f$(A+Id) \cdot x=b\f$ with the formula:
///    \f[ x' = D^{-1} \cdot (b - (L+U)x) \f]
/// with \f$ (A+Id)=D+L+U \f$ the <em>(D,L,U)</em> decomposition of the linear
/// equation system matrix.
/// \n Values of \a x not selected by \a rows remains unchanged.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
void JacobiIteration(const Matrix &A, VectorIn &x, const VectorB &b,
                     const VectorD &d, const double omega, VectorIn &x_next,
                     double &max_err, double &x_norm,
                     const RowSelector &rows, const ColSelector &cols) {
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    assert(N > 1 && A.size2() == N && b.size() == N && x.size() == N);

    x_next.resize(N);
    max_err = 0.0;
    x_norm = 0.0;

    // Iterate for each selected row
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        double sigma = 0.0;
        matrix_row_t ith_row(A, i);
        matrix_row_iterator_t A_ij = ith_row.begin();

        // sigma = SUM(j!=i) { A[i,j] * x[j] }
        while (A_ij != ith_row.end()) {
            const size_t j = A_ij.index();
            if (i != j && cols.isSelected(j))
                sigma += (*A_ij) * x(j);
            A_ij++;
        }

        // x'[i] = (b[i] - sigma) / (A[i,i] + d[i])
        sigma = (b(i) - sigma);
        if (sigma != 0.0) {
            if (A(i, i) + d(i) != 0.0)
                sigma /= (A(i, i) + d(i));
            max_err = max(max_err, abs(sigma - x(i)));
            x_next(i) = x(i) + omega * (sigma - x(i));
            x_norm += abs(x_next(i));
        }
        else x_next(i) = (1 - omega) * x(i);
    }
    x.swap(x_next);
}



//-----------------------------------------------------------------------------
/// Forward Gauss-Seidel iteration for a linear equation system
/// \f$(A+Id) \cdot x=b\f$.
///   \param A		 Matrix with the linear equation system.
///   \param x		 Solution vector.
///   \param b		 Column vector b.
///   \param d		 Diagonal vector added to A.
///   \param omega	 Over-relaxation parameter (SOR method), 0<\a omega<2.
///   \param max_err On return contains the maximum difference between elements
///                  of \a x and \a x'.
///   \param x_norm  Norm-1 of \a x after this Jacobi iteration.
///   \param rows	 Rows of (A,x,b) selected for the computation.
///   \param cols	 Columns of (A,x,b) selected for the computation.
///
/// \note
/// This method performs a single iteration of the forward Gauss-Seidel method
/// over \a x for linear system \f$(A+Id) \cdot x=b\f$ with the formula:
///   \f[ x' = (D+L)^{-1} \cdot (b - Ux) \f]
///   \f[ x' = D^{-1} \cdot (b - (Ux + Lx')) \f]
/// with \f$ (A+Id)=D+L+U \f$ the <em>(D,L,U)</em> decomposition of the linear
/// equation system matrix.
/// \n Values of \a x not selected by \a rows remains unchanged.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
void ForwardGaussSeidelIteration(const Matrix &A, VectorIn &x,
                                 const VectorB &b, const VectorD &d,
                                 const double omega,
                                 double &max_err, double &x_norm,
                                 const RowSelector &rows,
                                 const ColSelector &cols) {
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    UNUSED_PARAM(N);
    assert(N > 1 && A.size2() == N && b.size() == N && x.size() == N);

    max_err = 0.0;
    x_norm = 0.0;

    // Iterate for each selected row
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        double sigma = 0.0;
        matrix_row_t ith_row(A, i);
        matrix_row_iterator_t A_ij = ith_row.begin();

        // j in {[0, i) U [i+1, N)}, using new x values for j in [0, i)
        // sigma = SUM(j<i) { L[i,j] * x'[j] } + SUM(j>i) { U[i,j] * x[j] }
        while (A_ij != ith_row.end()) {
            const size_t j = A_ij.index();
            if (i != j && cols.isSelected(j))
                sigma += (*A_ij) * x(j);
            A_ij++;
        }

        // x'[i] = (b[i] - sigma) / (A[i,i] + d[i])
        sigma = (b(i) - sigma);
        if (sigma != 0.0) {
            if (A(i, i) + d(i) != 0.0)
                sigma /= (A(i, i) + d(i));
            max_err = max(max_err, abs(sigma - x(i)));
            x(i) = x(i) + omega * (sigma - x(i));
            x_norm += abs(x(i));
        }
        else x(i) = (1 - omega) * x(i);
    }
}



//-----------------------------------------------------------------------------
/// Backward Gauss-Seidel iteration for a linear equation system
/// \f$(A+Id) \cdot x=b\f$.
///   \param A		 Matrix with the linear equation system.
///   \param x		 Solution vector.
///   \param b		 Column vector b.
///   \param d		 Diagonal vector added to A.
///   \param omega	 Over-relaxation parameter (SOR method), 0<\a omega<2.
///   \param max_err On return contains the maximum difference between elements
///                  of \a x and \a x'.
///   \param x_norm  Norm-1 of \a x after this Jacobi iteration.
///   \param rows	 Rows of (A,x,b) selected for the computation.
///   \param cols	 Columns of (A,x,b) selected for the computation.
///
/// \note
/// This method performs a single iteration of the backward Gauss-Seidel method
/// over \a x for linear system \f$(A+Id) \cdot x=b\f$ with the formula:
///   \f[ x' = (D+U)^{-1} \cdot (b - Lx) \f]
///   \f[ x' = D^{-1} \cdot (b - (Ux' + Lx)) \f]
/// with \f$ (A+Id)=D+L+U \f$ the <em>(D,L,U)</em> decomposition of the linear
/// equation system matrix.
/// \n Values of \a x not selected by \a rows remains unchanged.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
void BackwardGaussSeidelIteration(const Matrix &A, VectorIn &x,
                                  const VectorB &b, const VectorD &d,
                                  const double omega,
                                  double &max_err, double &x_norm,
                                  const RowSelector &rows,
                                  const ColSelector &cols) {
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    UNUSED_PARAM(N);
    assert(N > 1 && A.size2() == N && b.size() == N && x.size() == N);

    max_err = 0.0;
    x_norm = 0.0;

    // Iterate for each selected row
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[rows.count() - 1 - r];
        double sigma = 0.0;
        matrix_row_t ith_row(A, i);
        matrix_row_iterator_t A_ij = ith_row.begin();

        // j in {[0, i) U [i+1, N)}, using new x values for j in [0, i)
        // sigma = SUM(j<i) { L[i,j] * x'[j] } + SUM(j>i) { U[i,j] * x[j] }
        while (A_ij != ith_row.end()) {
            const size_t j = A_ij.index();
            if (i != j && cols.isSelected(j))
                sigma += (*A_ij) * x(j);
            A_ij++;
        }

        // x'[i] = (b[i] - sigma) / (A[i,i] + d[i])
        sigma = (b(i) - sigma);
        if (sigma != 0.0) {
            if (A(i, i) + d(i) != 0.0)
                sigma /= (A(i, i) + d(i));
            max_err = max(max_err, abs(sigma - x(i)));
            x(i) = x(i) + omega * (sigma - x(i));
            x_norm += abs(x(i));
        }
        else x(i) = (1 - omega) * x(i);
    }
}



//-----------------------------------------------------------------------------
/// Jacobi iteration for a linear equation system \f$x^T \cdot (A+Id)^T=b^T\f$.
///   \param x		 Solution vector.
///   \param A		 Matrix with the linear equation system (transposed).
///   \param b		 Column vector b.
///   \param d		 Diagonal vector added to A.
///   \param omega	 Over-relaxation parameter (JOR method), 0<\a omega<2.
///   \param sigma   Auxiliary vector needed in the Jacobi method.
///   \param max_err On return contains the maximum difference between elements.
///                  of \a x and \a x'.
///   \param x_norm  Norm-1 of \a x after this Jacobi iteration.
///   \param rows	 Rows of (x,A,b) selected for the computation.
///   \param cols	 Columns of (x,A,b) selected for the computation.
///
/// \note
/// This method performs a single iteration of the Jacobi method over \a x
/// for linear system \f$x^T \cdot (A+Id)^T=b^T\f$ with the formula:
///    \f[ x' = (b - x(L+U)) \cdot D^{-1} \f]
/// with \f$ (A+Id)^T=D+L+U \f$ the <em>(D,L,U)</em> decomposition of the
/// transposed linear equation system matrix.
/// \n Values of \a x not selected by \a rows remains unchanged.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
void JacobiTransIteration(VectorIn &x, const Matrix &A, const VectorB &b,
                          const VectorD &d, const double omega, VectorIn &sigma,
                          double &max_err, double &x_norm,
                          const RowSelector &rows, const ColSelector &cols) {
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    assert(N > 1 && A.size2() == N && b.size() == N && x.size() == N);

    sigma.resize(N);
    max_err = 0.0;
    x_norm = 0.0;

    // Compute:  x'[j] = (b[j] - SUM(i){ x[i] * A[i,j] }) / (A[j,j] + d[i])
    for (size_t r = 0; r < rows.count(); r++)
        sigma(rows[r]) = 0.0;

    // sigma[j] = SUM(i!=j) { x[i] * A[i,j] }
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];

        matrix_row_t ith_row(A, i);
        matrix_row_iterator_t A_ij = ith_row.begin();

        while (A_ij != ith_row.end()) {
            const size_t j = A_ij.index();
            if (i != j && cols.isSelected(j)) {
                sigma(j) += x(i) * (*A_ij);
            }
            ++A_ij;
        }
    }

    // x'[i] = (b[i] - sigma[i]) / A[i,i]
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        double xi_next = (b(i) - sigma(i));
        if (A(i, i) + d(i) != 0.0)
            xi_next /= (A(i, i) + d(i));
        max_err = max(max_err, abs(xi_next - x(i)));
        x(i) = x(i) + omega * (xi_next - x(i));
        x_norm += abs(x(i));
    }
}



//-----------------------------------------------------------------------------
/// Forward Gauss-Seidel iteration for a linear equation system
/// \f$x^T \cdot (A+Id)^T=b^T\f$.
///   \param x		 Solution vector.
///   \param A		 Matrix with the linear equation system (transposed).
///   \param b		 Column vector b.
///   \param d		  Diagonal vector added to A.
///   \param omega	 Over-relaxation parameter (SOR method), 0<\a omega<2.
///   \param xLU     Auxiliary vector needed in the iterative method.
///   \param startOfU Starting index of the U rows in A.
///   \param max_err On return contains the maximum difference between elements.
///                  of \a x and \a x'.
///   \param x_norm  Norm-1 of \a x after this Jacobi iteration.
///   \param rows	 Rows of (x,A,b) selected for the computation.
///   \param cols	 Columns of (x,A,b) selected for the computation.
///
/// \note
/// This method performs a single iteration of the Jacobi method over \a x
/// for linear system \f$x^T \cdot (A+Id)^T=b^T\f$ with the formula:
///   \f[ x' = (b - xL) \cdot (D+U)^{-1} \f]
///   \f[ x' = (b - (xL + x'U)) \cdot D^{-1} \f]
/// with \f$ (A+Id)^T=D+L+U \f$ the <em>(D,L,U)</em> decomposition of the
/// transposed linear equation system matrix.
/// \n Values of \a x not selected by \a rows remains unchanged.
/// \n A Forward Gauss-Seidel transposed iteration is equivalent to a formard
/// Gauss-Seidel iteration on a transposed matrix. Note that \c U and \c L in
/// the formula above are refersed compared to the forward Gauss-Seidel
/// definition, because of the transpose of \a A.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
void ForwardGaussSeidelTransIteration(VectorIn &x, const Matrix &A,
                                      const VectorB &b, const VectorD &d,
                                      const double omega,
                                      VectorIn &xLU, vector<size_t> startOfU,
                                      double &max_err, double &x_norm,
                                      const RowSelector &rows,
                                      const ColSelector &cols) {
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    assert(N > 1 && A.size2() == N && b.size() == N && x.size() == N);

    xLU.resize(N);
    startOfU.resize(N, 0);

    // xLU = prod(x, L+U) = prod(x, L) + prod(x', U))
    for (size_t r = 0; r < rows.count(); r++)
        xLU(rows[r]) = 0.0;

    // Compute: xLU = prod(x, L)  => the xU part will be added later.
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        size_t start_of_Ui = 0;
        matrix_row_t ith_row(A, i);
        matrix_row_iterator_t A_ij = ith_row.begin();
        while (A_ij != ith_row.end()) {
            const size_t j = A_ij.index();
            if (j >= i)
                break;
            if (cols.isSelected(j))
                xLU(j) += x(i) * (*A_ij);
            ++A_ij;
            start_of_Ui++;
        }
        assert(startOfU[i] == 0 || startOfU[i] == start_of_Ui);
        startOfU[i] = start_of_Ui;
    }

    // First iteration: x'[0] = (b[0] - prod(x, L+U)[0]) / (A[0,0] + d[0])
    const size_t i0 = rows[0];
    double next_x0 = (b(i0) - xLU(i0)) / (A(i0, i0) + d(i0));
    max_err = abs(x(i0) - next_x0);
    x(i0) = x(i0) + omega * (next_x0 - x(i0));
    x_norm = abs(x(i0));

    for (size_t r = 1; r < rows.count(); r++) {
        const size_t i = rows[r], im1 = rows[r - 1];

        // Add to xLU the (i-1)th row product of x*U
        matrix_row_t ith_row(A, im1);
        matrix_row_iterator_t A_im1_j = ith_row.find(startOfU[im1]);
        while (A_im1_j != ith_row.end()) {
            const size_t j = A_im1_j.index();
            // xLU(j) += x(i-1) * A(i-1, j)
            if (j >= i && cols.isSelected(j))
                xLU(j) += x(im1) * (*A_im1_j);
            ++A_im1_j;
        }

        // x'[i] = (b[i] - prod(x, L+U)[i]) / (A[i,i] + d[i])
        double next_xi = (b(i) - xLU(i));
        if (A(i, i) + d(i) != 0.0)
            next_xi /= (A(i, i) + d(i));
        max_err = max(max_err, abs(x(i) - next_xi));
        x(i) = x(i) + omega * (next_xi - x(i));
        x_norm += abs(next_xi);
    }
}



//-----------------------------------------------------------------------------
/// Backward Gauss-Seidel iteration for a linear equation system
/// \f$x^T \cdot (A+Id)^T=b^T\f$.
///   \param x		 Solution vector.
///   \param A		 Matrix with the linear equation system (transposed).
///   \param b		 Column vector b.
///   \param d		  Diagonal vector added to A.
///   \param omega	 Over-relaxation parameter (SOR method), 0<\a omega<2.
///   \param xLU     Auxiliary vector needed in the iterative method.
///   \param startOfU Starting index of the U rows in A.
///   \param max_err On return contains the maximum difference between elements.
///                  of \a x and \a x'.
///   \param x_norm  Norm-1 of \a x after this Jacobi iteration.
///   \param rows	 Rows of (x,A,b) selected for the computation.
///   \param cols	 Columns of (x,A,b) selected for the computation.
///
/// \note
/// This method performs a single iteration of the Jacobi method over \a x
/// for linear system \f$x^T \cdot (A+Id)^T=b^T\f$ with the formula:
///   \f[ x' = (b - xU) \cdot (D+L)^{-1} \f]
///   \f[ x' = (b - (x'L + xU)) \cdot D^{-1} \f]
/// with \f$ (A+Id)^T=D+L+U \f$ the <em>(D,L,U)</em> decomposition of the
/// transposed linear equation system matrix.
/// \n Values of \a x not selected by \a rows remains unchanged.
/// \n A Backward Gauss-Seidel transposed iteration is equivalent to a backward
/// Gauss-Seidel iteration on a transposed matrix. Note that \c U and \c L in
/// the formula above are refersed compared to the backward Gauss-Seidel
/// definition, because of the transpose of \a A.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
void BackwardGaussSeidelTransIteration(VectorIn &x, const Matrix &A,
                                       const VectorB &b, const VectorD &d,
                                       const double omega,
                                       VectorIn &xLU, vector<size_t> startOfU,
                                       double &max_err, double &x_norm,
                                       const RowSelector &rows,
                                       const ColSelector &cols) {
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
    const size_t N = A.size1();
    assert(N > 1 && A.size2() == N && b.size() == N && x.size() == N);

    xLU.resize(N);
    startOfU.resize(N, 0);

    // xLU = prod(x, L+U) = prod(x', L) + prod(x, U))
    for (size_t r = 0; r < rows.count(); r++)
        xLU(rows[r]) = 0.0;

    // Compute: xLU = prod(x, U)  => the xL part will be added later.
    for (size_t r = 0; r < rows.count(); r++) {
        const size_t i = rows[r];
        size_t start_of_Ui = 0;
        matrix_row_t ith_row(A, i);
        matrix_row_iterator_t A_ij = ith_row.find(startOfU[i]);
        while (A_ij != ith_row.end()) {
            const size_t j = A_ij.index();
            if (j <= i)
                start_of_Ui++;
            else {
                if (cols.isSelected(j))
                    xLU(j) += x(i) * (*A_ij);
            }
            ++A_ij;
        }
        assert(startOfU[i] == 0 || startOfU[i] == start_of_Ui);
        startOfU[i] = start_of_Ui;
    }

    // Proceed by backward iterations, starting from the last row R
    // First iteration: x'[R] = (b[R] - prod(x, L+U)[R]) / (A[R,R] + d[R])
    const size_t iR = rows[rows.count() - 1];
    double next_xR = (b(iR) - xLU(iR)) / (A(iR, iR) + d(iR));
    max_err = abs(x(iR) - next_xR);
    x(iR) = x(iR) + omega * (next_xR - x(iR));
    x_norm = abs(x(iR));

    for (size_t r = 1; r < rows.count(); r++) {
        const size_t i = rows[rows.count() - 1 - r];
        const size_t ip1 = rows[rows.count() - r];

        // Add to xLU the (R-i-1)th row product of x*L
        matrix_row_t ith_row(A, ip1);
        matrix_row_iterator_t A_ip1_j = ith_row.begin();
        while (A_ip1_j != ith_row.end()) {
            const size_t j = A_ip1_j.index();
            if (j >= ip1)
                break;
            // xLU(j) += x(i+1) * A(i+1, j)
            if (cols.isSelected(j))
                xLU(j) += x(ip1) * (*A_ip1_j);
            ++A_ip1_j;
        }

        // x'[i] = (b[i] - prod(x, L+U)[i]) / (A[i,i] + d[i])
        double next_xi = (b(i) - xLU(i));
        if (A(i, i) + d(i) != 0.0)
            next_xi /= (A(i, i) + d(i));
        max_err = max(max_err, abs(x(i) - next_xi));
        x(i) = x(i) + omega * (next_xi - x(i));
        x_norm += abs(next_xi);
    }
}



//-----------------------------------------------------------------------------
/// Solves a system of linear equations in the form
/// \f$x^T \cdot (A+Id)^T=b^T\f$. \n Note that this is the transposed
/// of the standard system \f$(A+Id) \cdot x=b\f$.
///   \param x		  Solution vector.
///   \param A		  Matrix with the linear equation system (transposed).
///   \param b		  Column vector b.
///   \param d		  Diagonal vector added to A.
///   \param ked      Direction of the equation system.
///   \param spar	  Algorithm parameters.
///   \param printOut Print informations on the console.
///   \param rows	  Rows of (x,A,b) selected for the computation.
///   \param cols	  Columns of (x,A,b) selected for the computation.
///   \param init_x0  Initial distribution of x(0), (nullptr for a uniform vector).
///   \return True if the method converges in less than \a maxIters iterations.
///
/// \note
/// On return, the x[] elements not selected by (rows) are unchanged.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
bool LinearSolve(VectorIn &x, const Matrix &A, const VectorB &b, const VectorD &d,
                 KolmogorovEquationDirection ked, const SolverParams &spar,
                 NullIterPrintOut &printOut, const RowSelector &rows, const ColSelector &cols,
                 const VectorIn *init_x0 = nullptr) {
    const size_t N = A.size1();
    assert(A.size2() == N && b.size() == N);
    assert(spar.omega > 0.0 && spar.omega < 2.0);
    assert(init_x0 == nullptr || init_x0->size() == N);

    x.resize(N);

    if (rows.count() == 0)
        return true;
    // Special 1x1 matrix case:  x(0) = b(0) / (A(0,0) + d(0))
    if (rows.count() == 1) {
        printOut.onConvergence(AN_UNSPECIFIED, 0, 0);
        size_t r0 = rows[0];
        if (A(r0, r0) + d(r0) != 0.0 && b(r0) != 0.0)
            x(r0) = b(r0) / (A(r0, r0) + d(r0));
        else
            x(r0) = 1.0;
        return true;
    }

    if (ELA_Is_Krylov[spar.explicitAlgo]) {
        return KrylovSolve(x, A, b, d, ked, spar, printOut, rows, cols, init_x0);
    }

    double real_norm_b = norm_1(b), norm_b = real_norm_b;
    // Avoid values of |b| that are zero or below the machine precision
    if (norm_b * spar.epsilon < 100 * DBL_EPSILON)
        norm_b = 1.0;

    // Initial distribution of x(0)
    if (init_x0 != nullptr) { // Copy init_x0
        for (size_t n = 0; n < rows.count(); n++)
            x(rows[n]) = (*init_x0)(rows[n]);
    }
    else { // Uniform distribution
        for (size_t n = 0; n < rows.count(); n++)
            x(rows[n]) = 1.0 / rows.count();
    }

    double max_err, x_norm;
    VectorIn auxVec;
    vector<size_t> strtU;
    const size_t minIters = 2;
    size_t iterCount = 0, skipIters = 0, penalty = 2;
    AlgoName algoName(spar.explicitAlgo);

    // Iterative steps
    while (++iterCount < spar.maxIters) {
        // Choose the appropriate algorithm for this step
        ExplicitLinearSolverAlgo algo = spar.explicitAlgo;
        if (algo == ELA_SYMMETRIC_SOR)
            algo = (1 == (iterCount & 1)) ? ELA_FORWARD_SOR : ELA_BACKWARD_SOR;

        switch (algo) {
        case ELA_JOR:
            if (ked == KED_BACKWARD)
                JacobiIteration(A, x, b, d, spar.omega, auxVec,
                                max_err, x_norm, rows, cols);
            else
                JacobiTransIteration(x, A, b, d, spar.omega, auxVec,
                                     max_err, x_norm, rows, cols);
            break;

        case ELA_FORWARD_SOR:
            if (ked == KED_BACKWARD)
                ForwardGaussSeidelIteration(A, x, b, d, spar.omega,
                                            max_err, x_norm, rows, cols);
            else
                ForwardGaussSeidelTransIteration(x, A, b, d, spar.omega,
                                                 auxVec, strtU, max_err,
                                                 x_norm, rows, cols);
            break;

        case ELA_BACKWARD_SOR:
            if (ked == KED_BACKWARD)
                BackwardGaussSeidelIteration(A, x, b, d, spar.omega,
                                             max_err, x_norm, rows, cols);
            else
                BackwardGaussSeidelTransIteration(x, A, b, d, spar.omega,
                                                  auxVec, strtU, max_err,
                                                  x_norm, rows, cols);
            break;

        default:
            assert(0);
        }

        double max_rel_err;
        if (abs(x_norm) >= spar.epsilon)
            max_rel_err = (max_err / x_norm);
        else
            max_rel_err = max_err;
        printOut.onIteration(algoName, iterCount, max_rel_err);

        // Test convergence
        if (skipIters > 0)
            skipIters--;
        else if (iterCount > minIters) {
            /*if ((iterCount % 10) == 0) {
            	cout << "  iterCount="<<iterCount<<"  max_rel_err="<<max_rel_err;
            	cout << "  max_err="<<max_err<<"  x_norm="<<x_norm<<endl;
            }*/
            if (max_rel_err < spar.epsilon) {
                printOut.onConvergence(algoName, iterCount, max_rel_err);
                return true;

                // TODO: rivedere - da problemi con sistemi malcondizionati....
                // Verify the real error: err = norm(x*(A+Id) - b)
                // Note that max_rel_err is only the difference between
                // successive iteration vectors. If the solution is
                // converging very slowly, this test is needed.
                if (ked == KED_BACKWARD)
                    prod_matvec_diag(auxVec, A, d, x, rows, cols);
                else
                    prod_vecmat_diag(auxVec, x, A, d, rows, cols);

                double real_err = 0; // Effective relative difference
                for (size_t n = 0; n < rows.count(); n++) {
                    const size_t r = rows[n];
                    // TODO: rivedere bene: qui la condizione di test
                    // con divisione per max(abs(auxVec(r)), abs(b(r)))
                    // è sbagliata. Rivedere perchè.
                    if (b(r) != 0.0 && auxVec(r) != b(r))
                        real_err += abs(auxVec(r) - b(r)) / norm_b;
                    else
                        real_err += abs(auxVec(r) - b(r));
                    assert(!std::isnan(real_err));
                }

                /*cout << "  iterCount="<<iterCount<<"  max_err="<<max_err;
                cout << "  x_norm="<<x_norm<<"  real_err="<<real_err;
                cout << "  auxVec="<<auxVec<<"  x="<<x<<endl;*/

                if (real_err < spar.epsilon) {
                    printOut.onConvergence(algoName, iterCount, real_err);
                    return true;
                }
                else {
                    // Don't repeat the test for some iterations, to avoid
                    // doing too many vec*mat products.
                    skipIters = penalty;
                    penalty = min(2 * penalty, size_t(30));
                }
            }
        }
    }

    // The iterative method didn't converge in maxIters iterations.
    printOut.onFailure(algoName, iterCount, CFR_TOO_MANY_ITERATIONS);
    return false;
}



//-----------------------------------------------------------------------------
/* @}  Numerical documentation group. */
#endif  // __STATIONARY_METHODS_H__
