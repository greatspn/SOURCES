//-----------------------------------------------------------------------------
/// \file krylov.h
/// Krylov-subspace methods for the solution of linear equation systems.
/// These methods are taken from IML++ and modified to compile with boost c++
/// IML++ is a public domain library developed by NIST. It is a C++ templated
/// library of modern iterative methods for solving both symmetric and
/// nonsymmetric linear systems of equations.
///
/// Nonsymmetric matrix:
///  * Is transpose available?   -> Try QMR.
///  * Otherwise, is storage at a premium?   -> Try CGS or Bi-CGSTAB.
///  * Otherwise try GMRES with a long restart.
///
/// \author IML++ (minor changes here to make it compile against boost c++)
///
//-----------------------------------------------------------------------------

#ifndef __NUMERIC_KRYLOV_H__
#define __NUMERIC_KRYLOV_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------
// Computes the A*x (and x*A) matrix by vector product
//-----------------------------------------------------------------------------

template<class Matrix> struct Ax_Residual {
    inline Ax_Residual(const Matrix &_A) : A(_A) { }

    inline size_t size() const		{  return A.size1();  }

    template<class Vector>
    inline ublas::vector<typename Vector::value_type>
    product(const Vector &x) const
    {  return sparse_prod(A, x);  }

private:
    const Matrix &A;
};

template<class Matrix> struct xA_Residual {
    inline xA_Residual(const Matrix &_A) : A(_A) { }

    inline size_t size() const		{  return A.size1();  }

    template<class Vector>
    inline ublas::vector<typename Vector::value_type>
    product(const Vector &x) const {
        ublas::vector<typename Vector::value_type> out;
        full_element_selector rowSel(A.size1()), colSel(A.size2());
        return prod_vecmat(out, x, A, rowSel, colSel);
    }

private:
    const Matrix &A;
};


//-----------------------------------------------------------------------------
// Computes the (A+Id)*x and x*(A+Id) matrix by vector product
//-----------------------------------------------------------------------------

/*template<class Matrix, class VectorD> struct ApId_x_Residual {
	inline ApId_x_Residual(const Matrix& _A, const VectorD& _d) : A(_A), d(_d) { }

	inline size_t size() const		{  return A.size1();  }

	template<class Vector>
	inline ublas::vector<typename Vector::value_type>
	product(const Vector &x) const
	{  assert(0); return x;  }

private:
	const Matrix& A;
	const VectorD& d;
};*/

template<class Matrix, class VectorD> struct ApId_Residual {
    inline ApId_Residual(const Matrix &_A, const VectorD &_d,
                         KolmogorovEquationDirection _ked) : A(_A), d(_d), ked(_ked)
    {	assert(A.size1() == d.size() && A.size1() == A.size2());   }

    inline size_t size() const		{  return A.size1();  }

    template<class Vector>
    inline ublas::vector<typename Vector::value_type>
    product(const Vector &x) const {
        ublas::vector<typename Vector::value_type> out;
        full_element_selector rowSel(A.size1()), colSel(A.size2());
        switch (ked) {
        case KED_FORWARD:
            prod_vecmat(out, x, A, rowSel, colSel);
            break;
        case KED_BACKWARD:
            prod_matvec(out, A, x, rowSel, colSel);
            break;
        }
        for (size_t i = 0; i < size(); i++)
            out(i) += x(i) * d(i);
        return out;
    }

private:
    const Matrix &A;
    const VectorD &d;
    KolmogorovEquationDirection ked;
};

//-----------------------------------------------------------------------------
// Machine precision epsilon
//-----------------------------------------------------------------------------

inline void get_machine_epsilon(float &v)			{  v = FLT_EPSILON;  }
inline void get_machine_epsilon(double &v)			{  v = DBL_EPSILON;  }
inline void get_machine_epsilon(long double &v)		{  v = LDBL_EPSILON;  }

template<class Real>
inline Real less_than_machine_epsilon(Real v) {
    Real epsilon;
    get_machine_epsilon(epsilon);
    return v < epsilon * 100;
}


//-----------------------------------------------------------------------------


//*****************************************************************
// Iterative template routine -- GMRES
//
// GMRES solves the unsymmetric linear system Ax = b using the
// Generalized Minimum Residual method
//
// GMRES follows the algorithm described on p. 20 of the
// SIAM Templates book.
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//
//*****************************************************************


template<class Real>
void GeneratePlaneRotation(const Real dx, const Real dy, Real &cs, Real &sn) {
    if (dy == 0.0) {
        cs = 1.0;
        sn = 0.0;
    }
    else if (std::abs(dy) > std::abs(dx)) {
        Real temp = dx / dy;
        sn = 1.0 / sqrt(1.0 + temp * temp);
        cs = temp * sn;
    }
    else {
        Real temp = dy / dx;
        cs = 1.0 / sqrt(1.0 + temp * temp);
        sn = temp * cs;
    }
}

template<class Real>
void ApplyPlaneRotation(Real &dx, Real &dy, const Real cs, const Real sn) {
    Real temp  =  cs * dx + sn * dy;
    dy = -sn * dx + cs * dy;
    dx = temp;
}

template < class Matrix, class Vector >
void Update(Vector &x, int k, const Matrix &h, const Vector &s,
            const std::vector<Vector> &v, const std::vector<Vector> &z,
            const Preconditioner &R, bool is_fgmres) {
    Vector y(s);

    // Backsolve: y(m) = argmin || beta*e(1) - H(m)*y ||
    for (int i = k; i >= 0; i--) {
        y(i) = (h(i, i) != 0.0) ? (y(i) / h(i, i)) : 0.0;
        for (int j = i - 1; j >= 0; j--)
            y(j) -= h(j, i) * y(i);
    }

    // Update the solution vector;
    if (R.isIdentity()) {
        // x(m) = x(0) + V(m)*y(m)
        for (int j = 0; j <= k; j++)
            x += v[j] * y(j);
    }
    else if (is_fgmres) {
        // x(m) = x(0) + Z(m)*y(m)    since Z = R*V
        for (int j = 0; j <= k; j++)
            x += z[j] * y(j);
    }
    else {
        // x(m) = x(0) + R * (V(m)*y(m))
        Vector xx(x.size());
        std::fill(xx.begin(), xx.end(), 0.0);
        for (int j = 0; j <= k; j++)
            xx += v[j] * y(j);
        x += R * xx;
    }
}


// Works with left, right, and split preconditioners. In the right/split cases,
// the bases can be stored either directly or in the "flexible" GMRES approach,
// which allows for a right preconditioner that can change at every iteration.
//   Left:     L*A*x = L*b
//   Right:    A*R*u = b,     x = R*u
//   Split:  L*A*R*u = L*b,   x = R*u
template < class ResidualFunctor, class Vector, class VectorB,
           class Matrix, class Real >
int
GMRES(const ResidualFunctor &A, Vector &x, const VectorB &b,
      const Preconditioner &L, const Preconditioner &R, Matrix &H,
      size_t &m, bool is_fgmres, size_t &max_iter, Real &tol,
      NullIterPrintOut &printOut) {
    Real resid;
    size_t i, j = 1, k;
    Vector s(m + 1), cs(m + 1), sn(m + 1), w;
    AlgoName AN_GMRES(ILA_GMRES_M, m);

    is_fgmres = (is_fgmres && !R.isIdentity());
    Vector L_b = L * b;
    Real normb = norm_2(L_b);
    Vector r = L_b - L * (A.product(x));
    Real beta = norm_2(r);

    if (less_than_machine_epsilon(normb * tol))
        normb = 1;

    if ((resid = norm_2(r) / normb) <= tol) {
        tol = resid;
        max_iter = 0;
        printOut.onConvergence(AN_GMRES, max_iter, tol);
        return 0;
    }

    std::vector<Vector> v(m + 1), z(m + 1);
    for (size_t n = 0; n < m + 1; n++) {
        v[n].resize(x.size());
        std::fill(v[n].begin(), v[n].end(), 0.0);
    }

    while (j <= max_iter) {
        // Arnoldi method (modified Gram-Schmidt over Krylov subspace)
        v[0] = r / beta;
        fill(s.begin(), s.end(), 0.0);
        s(0) = beta;

        for (i = 0; i < m && j <= max_iter; i++, j++) {
            z[i] = R * v[i];
            w = L * (A.product(z[i]));
            if (!is_fgmres)
                z[i].clear();
            for (k = 0; k <= i; k++) {
                H(k, i) = inner_prod(w, v[k]);
                w -= H(k, i) * v[k];
            }
            H(i + 1, i) = norm_2(w);
            if (H(i + 1, i) != 0.0)
                v[i + 1] = w / H(i + 1, i);
            else
                std::fill(v[i + 1].begin(), v[i + 1].end(), 0.0);

            // Factor H into a QR decomposition using plane rotations
            for (k = 0; k < i; k++)
                ApplyPlaneRotation(H(k, i), H(k + 1, i), cs(k), sn(k));

            GeneratePlaneRotation(H(i, i), H(i + 1, i), cs(i), sn(i));
            ApplyPlaneRotation(H(i, i), H(i + 1, i), cs(i), sn(i));
            ApplyPlaneRotation(s(i), s(i + 1), cs(i), sn(i));

            resid = std::abs(s(i + 1)) / normb;
            printOut.onIteration(AN_GMRES, j, resid);
            if (resid < tol) {
                //Update(x, i, H, s, (isFGMRES ? z : v));
                Update(x, i, H, s, v, z, R, is_fgmres);
                tol = resid;
                max_iter = j;
                printOut.onConvergence(AN_GMRES, max_iter, tol);
                return 0;
            }
        }
        //Update(x, m - 1, H, s, (isFGMRES ? z : v));
        Update(x, m - 1, H, s, v, z, R, is_fgmres);
        r = L_b - L * (A.product(x));
        beta = norm_2(r);
        resid = beta / normb;
        printOut.onIteration(AN_GMRES, j, resid);
        if (resid < tol) {
            tol = resid;
            max_iter = j;
            printOut.onConvergence(AN_GMRES, max_iter, tol);
            return 0;
        }
    }

    tol = resid;
    printOut.onFailure(AN_GMRES, max_iter, CFR_TOO_MANY_ITERATIONS);
    return 1;
}



//*****************************************************************
// Iterative template routine -- BiCGSTAB
//
// BiCGSTAB solves the unsymmetric linear system Ax = b
// using the Preconditioned BiConjugate Gradient Stabilized method
//
// BiCGSTAB follows the algorithm described on p. 27 of the
// SIAM Templates book.
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//
//*****************************************************************


template < class ResidualFunctor, class Vector, class VectorB, class Real >
int
BiCGSTAB(const ResidualFunctor &A, Vector &x, const VectorB &b,
         const Preconditioner &M, size_t &max_iter, Real &tol,
         NullIterPrintOut &printOut) {
    Real resid;
    Real rho_1, rho_2 = 0, alpha = 0, beta = 0, omega = 0;
    Vector p, phat, s, shat, t, v;

    Real normb = norm_2(b);
    Vector r = b - A.product(x);
    Vector rtilde = r;

    if (less_than_machine_epsilon(normb * tol))
        normb = 1;

    if ((resid = norm_2(r) / normb) <= tol) {
        tol = resid;
        max_iter = 0;
        printOut.onConvergence(AN_BICGSTAB, max_iter, tol);
        return 0;
    }

    for (size_t i = 1; i <= max_iter; i++) {
        rho_1 = inner_prod(rtilde, r);
        if (rho_1 == 0) {
            tol = norm_2(r) / normb;
            printOut.onFailure(AN_BICGSTAB, i, CFR_BREAKDOWN);
            return 2;
        }
        if (i == 1)
            p = r;
        else {
            beta = (rho_1 / rho_2) * (alpha / omega);
            p = r + beta * (p - omega * v);
        }
        phat = M * p;
        v = A.product(phat);
        alpha = rho_1 / inner_prod(rtilde, v);
        s = r - alpha * v;
        resid = norm_2(s) / normb;
        printOut.onIteration(AN_BICGSTAB, i, resid);
        if (resid < tol) {
            x += alpha * phat;
            max_iter = i;
            tol = resid;
            printOut.onConvergence(AN_BICGSTAB, max_iter, tol);
            return 0;
        }
        shat = M * s;
        t = A.product(shat);
        omega = inner_prod(t, s) / inner_prod(t, t);
        x += alpha * phat + omega * shat;
        r = s - omega * t;

        rho_2 = rho_1;
        resid = norm_2(r) / normb;
        printOut.onIteration(AN_BICGSTAB, i, resid);
        if (resid < tol) {
            tol = resid;
            max_iter = i;
            printOut.onConvergence(AN_BICGSTAB, max_iter, tol);
            return 0;
        }
        if (omega == 0 || std::isnan((Real)resid)) {
            tol = norm_2(r) / normb;
            printOut.onFailure(AN_BICGSTAB, i, CFR_BREAKDOWN);
            return 3;
        }
    }

    tol = resid;
    printOut.onFailure(AN_BICGSTAB, max_iter, CFR_TOO_MANY_ITERATIONS);
    return 1;
}



//*****************************************************************
// Iterative template routine -- CGS
//
// CGS solves the unsymmetric linear system Ax = b
// using the Conjugate Gradient Squared method
//
// CGS follows the algorithm described on p. 26 of the
// SIAM Templates book.
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//
//*****************************************************************

template < class ResidualFunctor, class Vector, class VectorB, class Real >
int
CGS(const ResidualFunctor &A, Vector &x, const VectorB &b,
    const Preconditioner &M, size_t &max_iter, Real &tol,
    NullIterPrintOut &printOut) {
    Real resid;
    Real rho_1, rho_2 = 0, alpha = 0, beta = 0;
    Vector p, phat, q, qhat, vhat, u, uhat;

    Real normb = norm_2(b);
    Vector r = b - A.product(x);
    Vector rtilde = r;

    if (less_than_machine_epsilon(normb * tol))
        normb = 1;

    if ((resid = norm_2(r) / normb) <= tol) {
        tol = resid;
        max_iter = 0;
        printOut.onConvergence(AN_CGS, max_iter, tol);
        return 0;
    }

    for (size_t i = 1; i <= max_iter; i++) {
        rho_1 = inner_prod(rtilde, r);
        if (rho_1 == 0) {
            tol = norm_2(r) / normb;
            printOut.onFailure(AN_CGS, i, CFR_BREAKDOWN);
            return 2;
        }
        if (i == 1) {
            u = r;
            p = u;
        }
        else {
            beta = rho_1 / rho_2;
            u = r + beta * q;
            p = u + beta * (q + beta * p);
        }
        phat = M * p;
        vhat = A.product(phat);
        alpha = rho_1 / inner_prod(rtilde, vhat);
        q = u - alpha * vhat;
        Vector u_plus_q = (u + q);
        uhat = M * u_plus_q;
        x += alpha * uhat;
        qhat = A.product(uhat);
        r -= alpha * qhat;
        rho_2 = rho_1;
        resid = norm_2(r) / normb;
        printOut.onIteration(AN_CGS, i, resid);
        if (resid < tol) {
            tol = resid;
            max_iter = i;
            printOut.onConvergence(AN_CGS, max_iter, tol);
            return 0;
        }
    }

    tol = resid;
    printOut.onFailure(AN_CGS, max_iter, CFR_TOO_MANY_ITERATIONS);
    return 1;
}



//*****************************************************************
// Iterative template routine -- Preconditioned Richardson
//
// IR solves the unsymmetric linear system Ax = b using
// Iterative Refinement (preconditioned Richardson iteration).
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//
//*****************************************************************

template < class ResidualFunctor, class Vector, class VectorB, class Real >
int
IR(const ResidualFunctor &A, Vector &x, const VectorB &b,
   const Preconditioner &M, size_t &max_iter, Real &tol,
   Real omega, NullIterPrintOut &printOut) {
    Real resid;
    Vector z;

    Real normb = norm_1(b);
    if (less_than_machine_epsilon(normb * tol))
        normb = 1;

    Vector r = b - A.product(x);
    resid = norm_1(r) / normb;
    printOut.onIteration(AN_POWER_METHOD, 1, resid);
    if (resid <= tol) {
        tol = resid;
        max_iter = 0;
        printOut.onConvergence(AN_POWER_METHOD, max_iter, tol);
        return 0;
    }

    for (size_t i = 1; i <= max_iter; i++) {
        z = M * r;
        noalias(x) -= z * omega;
        r = b - A.product(x);

        resid = norm_1(r) / normb;
        printOut.onIteration(AN_POWER_METHOD, i, resid);
        if (resid <= tol) {
            tol = resid;
            max_iter = i;
            printOut.onConvergence(AN_POWER_METHOD, max_iter, tol);
            return 0;
        }
    }

    tol = resid;
    printOut.onFailure(AN_POWER_METHOD, max_iter, CFR_TOO_MANY_ITERATIONS);
    return 1;
}


//*****************************************************************
//*****************************************************************
template<class Matrix, class VectorIn, class VectorB, class VectorD,
         class RowSelector, class ColSelector>
bool KrylovSolve(VectorIn &x_, const Matrix &A_,
                 const VectorB &b_, const VectorD &d_,
                 KolmogorovEquationDirection ked,
                 const SolverParams &spar,
                 NullIterPrintOut &printOut,
                 const RowSelector &rows,
                 const ColSelector &cols,
                 const VectorIn *init_x0_ = nullptr) {
    const Matrix *pA = &A_;
    Matrix projA;
    VectorIn projX, *pX = &x_;
    VectorIn projInitX0;
    const VectorIn *init_x0 = init_x0_;
    typedef ublas::vector<typename VectorD::value_type> dense_diag_vec;
    dense_diag_vec d, b;
    bool projected = false;
    if (rows.count() != A_.size1() || cols.count() != A_.size2()) {
        project_matrix(A_, projA, rows, cols);
        pA = &projA;
        project_vector(d_, d, rows);
        project_vector(b_, b, rows);
        pX = &projX;
        if (init_x0_ != nullptr) {
            project_vector(*init_x0_, projInitX0, rows);
            init_x0 = &projInitX0;
        }
        projected = true;
    }
    else { d = d_; b = b_; }

    const Matrix &A = *pA;
    VectorIn &x = *pX;
    const size_t N = A.size1();
    assert(A.size2() == N && b.size() == N);
    assert(init_x0 == nullptr || init_x0->size() == N);
    assert(N >= 2);

    x.resize(N);

    /*cout << "N = " << N << endl;
    if (printOut.getVerboseLevel() >= VL_VERBOSE && N < 50) {
    	cout << endl;
    	print_matrix(cout, A, "A");
    	cout << "\nD = " << print_vec(d);
    	cout << "\nB = " << print_vec(b);
    	//cout << "\nX = " << print_vec(x);
    	cout << endl << endl;
    }*/

    // Special 1x1 matrix case:  x(0) = b(0) / (A(0,0) + d(0))
    if (N == 1) {
        printOut.onConvergence(AN_UNSPECIFIED, 0, 0);
        if (A(0, 0) + d(0) != 0.0 && b(0) != 0.0)
            x(0) = b(0) / (A(0, 0) + d(0));
        else
            x(0) = 1.0;
        /*if (printOut.getVerboseLevel() >= VL_VERBOSE && N < 50) {
        	cout << "\nX = " << print_vec(x) << endl << endl;
        }*/
        if (projected) {
            for (size_t i = 0; i < rows.count(); i++)
                x_[ rows[i] ] = x[i];
        }
        return true;
    }

    // Initial distribution of x(0)
    if (init_x0 != nullptr) // Copy init_x0
        std::copy(init_x0->begin(), init_x0->end(), x.begin());
    else // Uniform distribution
        std::fill(x.begin(), x.end(), sqrt(N) / N);

    // Build the preconditioner matrix
    std::unique_ptr<Preconditioner> prec;
    ublas::compressed_matrix<double> M_prec;
    VerboseLevel verboseLvl = printOut.getVerboseLevel();
    switch (spar.precAlgo) {
    case PA_NULL_PRECONDITIONER:
        prec.reset(new NullPreconditioner());
        break;

    case PA_ILU0: {
        FastTranspose(A, M_prec);
        double det = ILU0(M_prec, verboseLvl);
        if (det < 1000) {
            prec.reset(new ILU_Precond(M_prec));
        }
        else if (verboseLvl >= VL_BASIC)
            cout << "INSTABLE ILU(0) PRECONDITIONER (det=" << det << ")." << endl;
        break;
    }

    case PA_DIAG:
        prec.reset(new Diag_Precond(A));
        break;

    case PA_ILUTK: {
        FastTranspose(A, M_prec);
        ILUTK(M_prec, M_prec, spar.ilutkThreshold,
              spar.ilutkMaxNumEntries, verboseLvl);
        prec.reset(new ILU_Precond(M_prec));
        break;
    }

    default:
        assert(0);
    }
    assert(prec != nullptr);

    /*if (spar.implPrecond == IPA_ILU0) {
    	/ *FastTranspose(A, M_prec);
    	cout << "BUILDING ILU(0) PRECONDITIONER..." << endl;
    	double det = ILU0(M_prec);
    	bool hasPrec = (det < 100);// * /

    	cout << "BUILDING ILUTK PRECONDITIONER... " << endl;
    	FastTranspose(A, M_prec);
    	//gmm_ILUT_d(M_prec, 1.0e-4, 15);
    	ILUTK_d(M_prec, M_prec, 1.0e-5, 12);
    	bool hasPrec = true;
    	cout << "ILUTK PRECONDITIONER BUILT." << endl;// * /
    	if (printOut.getVerboseLevel() >= VL_VERBOSE && N < 50) {
    		cout << endl;
    		print_matrix(cout, M_prec, "M");
    		cout << endl;
    	}

    	// TODO: hack!! check the stability of the preconditioner instead, or switch to ILUT
    	if (!hasPrec)
    		prec.reset(new NullPreconditioner());
    	else
    		prec.reset(new ILU_Precond(M_prec));
    	/ *if (spar.verboseLvl >= VL_VERBOSE && N < 50) {
    		cout << endl;
    		print_matrix(cout, M_prec, "M");
    		cout << endl;
    	}* /
    }
    else
    	prec.reset(new NullPreconditioner());*/

    NullPreconditioner nullPrec;
    ApId_Residual<const Matrix, const dense_diag_vec> ApId_resid(A, d, ked);
    double tol = spar.epsilon;
    size_t maxIter = spar.maxIters;
    bool success = false;

    switch (spar.explicitAlgo) {
    case ELA_GMRES_M: {
        size_t KrylovM = std::min(spar.KrylovM, N - 1);
        ublas::matrix<double> Hessenberg(KrylovM + 1, KrylovM + 1);
        success = (0 == GMRES(ApId_resid, x, b, nullPrec, *prec, Hessenberg,
                              KrylovM, true, maxIter, tol, printOut));
        break;
    }

    case ELA_BICGSTAB:
        success = (0 == BiCGSTAB(ApId_resid, x, b, *prec, maxIter, tol, printOut));
        break;

    case ELA_CGS:
        success = (0 == CGS(ApId_resid, x, b, *prec, maxIter, tol, printOut));
        break;

    default:
        assert(0);
    }

    if (projected) {
        for (size_t i = 0; i < rows.count(); i++)
            x_[ rows[i] ] = x[i];
    }

    /*if (printOut.getVerboseLevel() >= VL_VERBOSE && N < 50) {
    	cout << "\nX = " << print_vec(x) << endl << endl;
    }*/
    return success;
}



//-----------------------------------------------------------------------------
/* @}  Numerical documentation group. */
#endif  // __NUMERIC_KRYLOV_H__
