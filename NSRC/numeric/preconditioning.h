//-----------------------------------------------------------------------------
/// \file preconditioning.h
/// Definition and implementation of basic preconditioning methods.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __PRECONDITIONING_H__
#define __PRECONDITIONING_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------

typedef struct NullPreconditioner   NullPreconditioner;

//-----------------------------------------------------------------------------
/// Base Preconditioner Interface
//-----------------------------------------------------------------------------
struct Preconditioner {
    virtual ~Preconditioner() { }

    inline ublas::vector<double> operator *(const ublas::vector<double> &v) const
    {   return solve(v);   }

    virtual bool isIdentity() const = 0;

protected:
    /// Apply the preconditioning to a given vector
    ///   \param v    Solution vector to precondition
    /// \return Preconditioned vector.
    virtual ublas::vector<double> solve(const ublas::vector<double> &v) const = 0;
};

//-----------------------------------------------------------------------------
/// Null (identity) Preconditioner Interface
//-----------------------------------------------------------------------------
struct NullPreconditioner : public Preconditioner {
    bool isIdentity() const    {   return true;   }
protected:
    ublas::vector<double> solve(const ublas::vector<double> &v) const
    {   return v;   }
    // TODO: rivedere se si può evitare la copia di v
};

//-----------------------------------------------------------------------------
/// Incomplete LU factorization with 0 level of fill-ins.
///  \param M  Input matrix. Overwritten by the algorithm
///  \param verboseLvl  Verbose level of the method.
///  \return The determinant of the ILU matrix.
//-----------------------------------------------------------------------------
template <class Matrix>
typename Matrix::value_type
ILU0(Matrix &M, VerboseLevel verboseLvl) {
    typedef typename Matrix::value_type value_type;
    typedef typename Matrix::size_type size_type;
    typedef typename ublas::matrix_row<Matrix>::iterator row_iterator, col_iterator;
    value_type det = 1;
    const size_type N = M.size1();

    if (verboseLvl >= VL_BASIC)
        cout << "BUILDING ILU(0) PRECONDITIONER... " << flush;

    Matrix trM; // transpose structure of M
    FastTranspose(M, trM);

    for (size_t r = 0; r < N - 1; r++) {
        value_type d = safe_inv(M(r, r));
        det *= M(r, r);

        ublas::matrix_row<Matrix> rth_col(trM, r);
        col_iterator it_r(rth_col.begin()), endIt_r(rth_col.end());
        for (; it_r != endIt_r; ++it_r) {
            const size_type i = it_r.index();
            if (i <= r)
                continue;
            if (M(i, r) == 0)
                continue;
            value_type e = M(i, r) * d;
            M(i, r) = e;

            ublas::matrix_row<Matrix> ith_row(M, i);
            row_iterator M_ij(ith_row.begin()), endM_ij(ith_row.end());
            for (; M_ij != endM_ij; ++M_ij) {
                const size_type j = M_ij.index();
                if (j <= r)
                    continue;
                if (*M_ij == 0 || M(r, j) == 0)
                    continue;
                *M_ij -= e * M(r, j);
            }
        }
    }
    if (verboseLvl >= VL_BASIC) {
        cout << "BUILT: det=" << det << ", nnz=" << M.nnz();
        double fill_in = int(double(M.nnz()) / (M.size1() * M.size2()) * 10000) * 0.01;
        cout << ", fill-in=" << fill_in << "%." << endl;
        if (verboseLvl >= VL_VERBOSE && M.size1() < 50) {
            print_matrix(cout, M, "M");
            cout << endl << endl;
        }
    }
    return det;
}

//-----------------------------------------------------------------------------

template <class T> struct qentry_comp : binary_function <T, T, bool> {
    bool operator()(const T &x, const T &y) const
    {return std::abs(x.first) < std::abs(y.first);}
};

//-----------------------------------------------------------------------------
/// Incomplete LU factorization with threshold and K maximum fill-ins.
///  \param A     Input matrix.
///  \param outP  Output preconditioner matrix.
///  \param threshold  Relative threshold of the dropping strategy.
///  \param K     Maximum number of entries kept in L and in U.
///  \param verboseLvl  Verbose level of the method.
//-----------------------------------------------------------------------------
template <class Matrix>
void ILUTK(const Matrix &A,
           ublas::compressed_matrix<typename Matrix::value_type> &outP,
           typename Matrix::value_type threshold, size_t K, VerboseLevel verboseLvl) {
    typedef typename Matrix::value_type value_type;
    typedef typename Matrix::size_type size_type;
    const size_type NR = A.size1(), NC = A.size2();

    typedef std::pair<value_type, size_t> qentry;
    std::priority_queue<qentry, std::vector<qentry>, qentry_comp<qentry> > qL, qU;
    ublas::vector<value_type> inv_diag(NR);
    ublas::mapped_vector<value_type> w(NC);
    ublas::compressed_vector<value_type> wL(NC), wU(NC);
    ublas::compressed_matrix<value_type> LU(NR, NC);
    value_type prec = numeric_limits<value_type>::epsilon();
    value_type max_pivot = std::abs(A(0, 0)) * prec;

    // typedef typename ublas::compressed_matrix<value_type>::const_iterator2 LU_iterator2_t;
    typedef typename ublas::compressed_vector<value_type>::const_iterator vec_iterator_t;
    // typedef typename Matrix::const_iterator2 mat_iterator2_t;
    typedef typename ublas::matrix_row<const Matrix>  matrix_row_t;
    typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;

    if (verboseLvl >= VL_BASIC)
        cout << "BUILDING ILUTK(T=" << threshold << ", K=" << K << ") PRECONDITIONER... " << flush;
    size_t nnz_in_L = 0, nnz_in_U = 0;

    // Take all the rows of A from 0 to N
    for (size_type i = 0; i < NR; ++i) {
        //w = row(A, i);
        w.resize(NR, false);
        matrix_row_t A_ith_row(A, i);
        matrix_row_iterator_t A_ij = A_ith_row.begin(), A_iend = A_ith_row.end();
        for (; A_ij != A_iend; ++A_ij)
            w(A_ij.index()) = *A_ij;

        value_type norm_row = norm_2(w);
        value_type row_threshold = norm_row * threshold;

        typedef typename ublas::mapped_vector<value_type>::iterator witerator_t;
        witerator_t wIt = w.begin();
        // Get the elements to reduce in the row w = A(i,*)
        while (wIt != w.end() && wIt.index() < i) {
            const size_t k = wIt.index();
            // Multiplier
            value_type mult = (*wIt) * inv_diag[k];

            if (std::abs(mult) < row_threshold) {
                w.erase_element(k);
            }
            else {
                *wIt += mult;

                matrix_row_t U_kth_row(LU, k);
                matrix_row_iterator_t U_kj = U_kth_row.find(k), U_kend = U_kth_row.end();
                while (U_kj != U_kend) {
                    const size_t j = U_kj.index();
                    w(j) -= *U_kj * mult;
                    ++U_kj;
                }
            }
            wIt = w.find(k + 1); // Due to erase wIt could be an invalid iterator...
        }
        value_type diag_i = w[i];

        if (std::abs(diag_i) <= max_pivot) {
            cout  << "  [ILUTK] PIVOT OF ROW " << i << " IS TOO SMALL. (IMPLEMENT ILUTP?)\n";
            w[i] = diag_i = value_type(1);
        }

        max_pivot = std::max(max_pivot, std::min(std::abs(diag_i) * prec, value_type(1)));
        if (std::abs(diag_i) > row_threshold)
            inv_diag[i] = value_type(1) / diag_i;
        else
            inv_diag[i] = value_type(1);

        // Prepare the new L and U rows
        while (!qL.empty())   qL.pop();
        while (!qU.empty())   qU.pop();
        wL.resize(NR, false);
        wU.resize(NR, false);
        wIt = w.begin();
        while (wIt != w.end()) {
            if (std::abs(*wIt) > row_threshold) {
                if (wIt.index() < i)
                    qL.push(make_pair(*wIt, wIt.index()));
                else if (wIt.index() > i)
                    qU.push(make_pair(*wIt, wIt.index()));
            }
            ++wIt;
        }
        // Push back at most the greater numL and numU entries in L and U
        size_t numL = 0, numU = 0;
        while (numL < K && !qL.empty()) {
            wL(qL.top().second) = qL.top().first;
            qL.pop();
            numL++;
        }
        while (numU < K && !qU.empty()) {
            wU(qU.top().second) = qU.top().first;
            qU.pop();
            numU++;
        }
        if (std::abs(w(i)) > row_threshold)
            wU(i) = w(i); // Unstable diagonal value?

        //row(LU, i) = wL + wU;
        vec_iterator_t wL_j = wL.begin(), wL_jend = wL.end();
        for (; wL_j != wL_jend; ++wL_j)
            LU.push_back(i, wL_j.index(), *wL_j);
        vec_iterator_t wU_j = wU.begin(), wU_jend = wU.end();
        for (; wU_j != wU_jend; ++wU_j)
            LU.push_back(i, wU_j.index(), *wU_j);
        nnz_in_L += numL;
        nnz_in_U += numU;
    }
    // Assemble the output preconditioner matrix outP
    outP.swap(LU);

    if (verboseLvl >= VL_BASIC) {
        cout << "BUILT: nnz(L)=" << nnz_in_L << ", nnz(U)=" << nnz_in_U;
        double fill_in = int(double(outP.nnz()) / (outP.size1() * outP.size2()) * 10000) * 0.01;
        cout << ", fill-in=" << fill_in << "%." << endl;
        if (verboseLvl >= VL_VERBOSE && outP.size1() < 50) {
            print_matrix(cout, outP, "M");
            cout << endl << endl;
        }
    }
}


//-----------------------------------------------------------------------------
/// Preconditioner class for all the ILU-like preconditioner matrices
//-----------------------------------------------------------------------------
struct ILU_Precond : public Preconditioner  {

    /// Initialize the preconditioner with the assigned ILU matrix
    ///  \param _M  The ILU matrix. The content of \a _M is swapped.
    ILU_Precond(ublas::compressed_matrix<double> &_M)
    {   M.swap(_M);   }

    template<class matrix_expression>
    ILU_Precond(const matrix_expression &_M)
    {   M = _M;   }

    bool isIdentity() const    {   return false;   }

protected:
    ublas::vector<double> solve(const ublas::vector<double> &inVec) const {
        size_t N = inVec.size();
        ublas::vector<double> outVec(N), midVec(N);
        ublas::identity_matrix<double> I(N);

        ForeSubstitution(M, I, inVec, midVec);
        BackSubstitution(M, M, midVec, outVec);

        return outVec;
    }

    ILU_Precond() { }

protected:
    ublas::compressed_matrix<double> M;   ///< ILU matrix.
};

//-----------------------------------------------------------------------------
/// Preconditioner class for a Diagonal/Jacobi preconditioner
//-----------------------------------------------------------------------------
struct Diag_Precond : public Preconditioner  {

    /// Initialize the preconditioner with the assigned diagonal matrix
    ///  \param M   Matrix from which the diagonal will be taken.
    ///  \param eps Inversion threshold.
    template<class matrix_expression>
    Diag_Precond(const matrix_expression &M,
                 double eps = 1.0e-1) : inv_eps(eps) {
        assert(M.size1() == M.size2());
        inv_diag.resize(M.size1());
        for (size_t i = 0; i < M.size1(); i++)
            inv_diag[i] = safe_inv(M(i, i));
    }

    /// Initialize the preconditioner with the assigned diagonal vector
    ///  \param M   Matrix from which the diagonal will be taken.
    ///  \param eps Inversion threshold.
    Diag_Precond(const ublas::vector<double> diag,
                 double eps = 1.0e-5) : inv_eps(eps) {
        inv_diag = diag;
        for (size_t i = 0; i < inv_diag.size(); i++)
            inv_diag[i] = safe_inv(inv_diag[i]);
    }

    bool isIdentity() const    {   return false;   }

protected:
    ublas::vector<double> solve(const ublas::vector<double> &inVec) const {
        const size_t N = inVec.size();
        assert(N == inv_diag.size());
        ublas::vector<double> outVec(N);
        for (size_t i = 0; i < N; i++)
            outVec[i] = inVec[i] * inv_diag[i];
        return outVec;
    }

    Diag_Precond() { }

    inline double safe_inv(double v)
    {   return (std::abs(v) < inv_eps) ? 1.0 : 1.0 / v;   }

protected:
    ublas::vector<double>   inv_diag;   ///< Inverted diagonal.
    double					inv_eps;    ///< Inversion epsilon
};

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/* @}  Numerical documentiation group. */
#endif  // __PRECONDITIONING_H__
