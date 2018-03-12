//-----------------------------------------------------------------------------
/// \file transient.h
/// Iterative Transient solution algorithm
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __NUMERIC_TRANSIENT_H__
#define __NUMERIC_TRANSIENT_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------

#if 0
//-----------------------------------------------------------------------------
/// Standard Uniformization (SU) method.
///   \param P		  Uniformized CTMC.
///   \param lambda   Uniformization rate of the CTMC \e P.
///   \param pi0	  Initial distribution vector.
///   \param pi		  On return contains the transient solution at time t.
///   \param pi_acc	  On return contains the cumulative probability.
///   \param t		  Transient solution time.
///   \param epsilon  Max. relative error in the iterative method.
///   \param rows     Selected rows.
///   \param cols     Selected columns.
///   \param verboseLvl Print informations on the console.
///
/// \note
/// This method implements the Standard uniformization method, with both
/// instantaneous and cumulative transient solution of a CTMC. \n
/// Instantaneous transient probabilities follows the formula:
///  \f[ \pi(t) = \sum^R_{n=0} \Phi(n) \cdot \beta(n, qt) \f]
/// with \f$ \Phi(n) = \Phi(n-1) \cdot P, ~~\Phi(0) = \pi_0 \f$ the probability
/// distribution after \e n steps of the uniformized process \f$ P=I+Q/q \f$,
/// for some \f$ q\geq max|-q_{ii}| \f$, and
/// \f$ \beta(n, qt)=\frac{e^{-qt}(qt)^n}{n!} \f$ the Poisson distribution. \n
/// Cumulative probabilities follows the formula:
///  \f[\pi_c(t)=\frac{1}{q}\sum^R_{n=0}\Phi(n)\cdot\sum^R_{k=n+1}\beta(k,qt)\f]
/// \par
/// For the instantaneous, see "Numerical solution of Markov Chains",
/// W.J. Stewart, page 417. The cumulative solution, instead, is based on the
/// pseudocode provided in "Transient Analysis of Cumulative Measures of
/// Markov Model Behavior", A. Reibman & K. Trivedi. Cumulative probability
/// refers to the *normalized* cumulative measure, therefore norm(pi_acc)=1.
/// Note that both solutions are computed with error tolerance epsilon, but if
/// the cumulative solution L(t) with norm(L(t))=t, then the solution must be
/// computed with \f$ \epsilon/t \f$ tolerance. See the  Reibman & Trivedi
/// paper, page 10, for details.
//-----------------------------------------------------------------------------
template<class Matrix, class Vector1, class Vector2,
         class RowSelector, class ColSelector>
void StandardUniformization(const Matrix &P, double lambda,
                            const Vector1 &pi0,	Vector2 *pi, Vector2 *pi_acc,
                            const double t, const double epsilon,
                            const RowSelector &rows, const ColSelector &cols,
                            const VerboseLevel verboseLvl) {
    const size_t N = P.size1();
    const bool compute_accum = (pi_acc != nullptr);  // Cumulative
    const bool compute_inst = (pi != nullptr);		  // Instantaneous
    assert(N > 1 && P.size2() == N && pi0.size() == N && t > 0.0);
    assert(compute_accum || compute_inst);

    // Number of uniformization steps M: solution of pi(0) => pi(t) is divided
    // into M steps:  pi(0) => pi(t/M) => pi(2*t/M) => ... => pi(t)
    double MAX_LAMBDA_T = 290.0;
    const size_t M = 1 + size_t(floor((lambda * t) / MAX_LAMBDA_T));
    const double tM = t / M;  // tm is the time step

    // Find the number R of iterations needed to reach the required accuracy
    // R is the right truncation point of the Taylor series.
    size_t R = 0;
    double sigma = 1;
    double xi = 1;
    double eta = (1 - epsilon / M) / exp(-lambda * tM);
    while (sigma < eta) {
        R++;
        xi *= (lambda * tM) / R;
        sigma += xi;
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "STANDARD UNIFORMIZATION(t=" << t << ", Lambda=" << lambda << "):";
        cout << " M=" << M << " steps, R=" << R << " right truncation point, ";
        cout << (M * R) << " vector*matrix products." << endl;
    }

    // The uniformization is done in M steps: each of these step advances
    // the probability vector pi of (t/M) time units.
    ublas::zero_vector<double> zero_vec(N);
    const double expLambdaT = exp(-lambda * tM);
    Vector2 y, yTmp(N), z, pi_t;

    if (!compute_inst) {
        // We need the pi vector in any case, because it's used to initialize
        // y at each new step iteration; however, we will skip the last m step
        pi = &pi_t;
    }
    (*pi) = pi0;
    if (compute_accum)
        (*pi_acc) = zero_vec;

    for (size_t m = 0; m < M; m++) {
        const bool last_m_iter = (m == M - 1);
        // To approximate pi(t/M), sum up the first R terms of:
        //      pi(tM) = SUM(k=1..R){ y_k * beta_k }
        // with y_k = pi0 * P^k   k-th exponentiation of pi0 * P
        //      beta_k = e^(-lambda*tM) * ((lambda*tM)^k)/k!  Poisson value.
        double beta = 1.0, gamma = (1 - expLambdaT);
        copy_vec(y, *pi, rows);							// y = (*pi)

        // Cumulative probabilities follows the formula:
        //      pi_acc(tM) = 1/lambda * SUM(k=0..R){ y_k * gamma_k }
        // with gamma_k = complement of the k-th cumulative Poisson value.
        if (compute_accum) {
            copy_vec(z, *pi, rows);
            mult_vec(z, 1 - expLambdaT, rows);	 // z = (*pi) * (1 - expLambdaT)
        }

        for (size_t k = 1; k <= R; k++) {
            // y is the probability vector after k steps of the DTMC P
            prod_vecmat(yTmp, y, P, rows, cols);		// y' = prod(y, P)
            copy_vec(y, yTmp, rows);					// y = y';
            // beta(k, lambda*tM) Poisson distribution function
            beta *= ((lambda * tM) / k);

            // Sum up the Taylor series terms
            if (compute_inst || !last_m_iter)
                plus_assign_mult(*pi, beta, y, rows);	// pi += beta * y
            if (compute_accum) {
                gamma -= beta * expLambdaT;
                plus_assign_mult(z, gamma, y, rows);	// z += gamma * y
            }
        }

        if (compute_inst || !last_m_iter)
            mult_vec(*pi, expLambdaT, rows);			// pi *= exp(-lambda*tM)
        if (compute_accum)
            plus_assign_mult(*pi_acc, 1 / (lambda * t), z, rows); // pi_acc += z / lambda
    }
}



//-----------------------------------------------------------------------------
/// Uniformization method with Fox-Glynn Poisson values.
///   \param P		  Uniformized CTMC.
///   \param lambda   Uniformization rate of the CTMC \e P.
///   \param pi0	  Initial distribution vector.
///   \param pi		  On return contains the transient solution at time t.
///   \param pi_acc	  On return contains the cumulative probability.
///   \param t		  Transient solution time.
///   \param epsilon  Max. relative error in the iterative method.
///   \param rows     Selected rows.
///   \param cols     Selected columns.
///   \param verboseLvl Print informations on the console.
///   \return False if something wrong happened (turn on verbose for details).
///
/// \note
/// The method implemented here is described by Lindemann (Performance Modeling
/// with Deterministic and Stochastic Petri Nets, page 106-113) and others, and
/// is essentialy a variation of the Standard Uniformization method.
/// \n At least one of the two vectors pi and pi_acc must be not-nullptr.
//-----------------------------------------------------------------------------
template<class Matrix, class Vector1, class Vector2,
         class RowSelector, class ColSelector>
bool FoxGlynnUniformization(const Matrix &P, double lambda,
                            const Vector1 &pi0, Vector2 *pi, Vector2 *pi_acc,
                            const double t, const double epsilon,
                            const RowSelector &rows, const ColSelector &cols,
                            const VerboseLevel verboseLvl) {
    const size_t N = P.size1();
    const bool compute_accum = (pi_acc != nullptr);  // Cumulative
    const bool compute_inst = (pi != nullptr);		  // Instantaneous
    assert(N > 1 && P.size2() == N && pi0.size() == N && t > 0.0);
    assert(compute_accum || compute_inst);

    // Compute Poisson weights W in the relevant interval [L, R]
    FoxGlynnWeights fgw;
    if (!FG_Weighter(lambda * t, epsilon, 1.0e-300, 1.0e300, fgw, verboseLvl))
        return false;

    if (verboseLvl >= VL_BASIC) {
        cout << "FOX-GLYNN UNIFORMIZATION(t=" << t << ", Lambda=" << lambda;
        cout << ", L=" << fgw.L << ", R=" << fgw.R << ", SumW=" << fgw.SumW << ")";
        cout << endl;
    }

    // Difference between 2 magnitude orders for multiorder_vector
    const int ORD_DIFF = 16;
    const double minVal = min(fgw.W[0], fgw.W[fgw.W.size() - 1]) / fgw.SumW * 1.0e-15;
    multiorder_vector  mo_pi(N, 1.0, minVal, ORD_DIFF);
    multiorder_vector  mo_pi_acc(N, 1.0, minVal, ORD_DIFF);

    // Fox-Glynn Uniformization uses a modified Taylor series with both left
    // and right truncation, in accordance with the formula:
    //        pi(t) = SUM{n=L..R}( Phi(n) * (W(n)/SumW) )
    // with:  Phi(n) = pi0 * (P^n)     n-th vector exponentiation
    ublas::vector<double> Phi;   // Prob. distribution after n transitions
    double Gamma = 1.0;          // Complementary cumulative probability
    Phi = pi0;

    // Compute the first L vector*matrix products
    ublas::vector<double> Phi_prime;
    for (int n = 0; n < fgw.L; n++) {
        if (compute_accum) {
            for (size_t i = 0; i < rows.count(); i++)
                mo_pi_acc.add(rows[i], Phi(rows[i]));
        }
        prod_vecmat(Phi_prime, Phi, P, rows, cols);
        Phi_prime.swap(Phi);
    }
    // Compute the [L..R] range
    for (int n = fgw.L; n < fgw.R; n++) {
        double beta = (fgw.W[n - fgw.L] / fgw.SumW);
        if (compute_accum) {
            Gamma -= beta;
            for (size_t i = 0; i < rows.count(); i++)
                mo_pi_acc.add(rows[i], Gamma * Phi(rows[i]));
        }
        if (compute_inst) {
            for (size_t i = 0; i < rows.count(); i++)
                mo_pi.add(rows[i], beta * Phi(rows[i]));
        }

        if (n != fgw.R - 1) {
            //Phi' = prod(Phi, P);
            prod_vecmat(Phi_prime, Phi, P, rows, cols);
            Phi_prime.swap(Phi);
        }
    }

    // Pack the multiorder solution vectors into *pi and *pi_acc
    if (compute_accum) {
        // Add the [0..L-1] probabilities for last, to avoid roundoffs
        mo_pi_acc.pack(*pi_acc);    //TODO: pack() sugli elementi in rows
        // Normalize the cumulative probabilities
        mult_vec(*pi_acc, 1 / (lambda * t), rows);
    }
    if (compute_inst) {
        mo_pi.pack(*pi);			//TODO: pack() sugli elementi in rows
    }

    return true;
}


//-----------------------------------------------------------------------------
/// Uniformization method.
///   \param Q		  Infinitesimal generator matrix (ergodic or non ergodic).
///   \param pi0	  Initial distribution vector.
///   \param pi		  On return contains the transient solution at time t.
///   \param pi_acc	  On return contains the cumulative probability.
///   \param t		  Transient solution time.
///   \param epsilon  Max. relative error in the iterative method.
///   \param rows     Selected rows.
///   \param cols     Selected columns.
///   \param verboseLvl Print informations on the console.
///   \return False if something wrong happened (turn on verbose for details).
///
/// \note
/// The Uniformization method computes the transient solution of the given
/// CTMC at time t, using the selected algorithm.
/// \n At least one of the two vectors pi and pi_acc must be not-nullptr.
//-----------------------------------------------------------------------------
template<class Matrix, class Vector1, class Vector2,
         class RowSelector, class ColSelector>
bool Uniformization(const Matrix &Q, const Vector1 &pi0,
                    Vector2 *pi, Vector2 *pi_acc,
                    const double t, const double epsilon,
                    const RowSelector &rows, const ColSelector &cols,
                    const VerboseLevel verboseLvl = VL_NONE,
                    TransientAlgorithm trAlgo = TA_STANDARD_UNIFORMIZATION) {
    const size_t N = Q.size1();
    assert(Q.size2() == N && pi0.size() == N);

    if (N == 0)
        return false; // Can this happen ?

    if (N == 1) {
        if (verboseLvl >= VL_BASIC)
            cout << "UNIFORMIZATION: trivial solution for 1x1 matrix." << endl;
        if (pi != nullptr) {
            pi->resize(1);
            (*pi)[0] = 1.0;
        }
        if (pi_acc != nullptr) {
            pi_acc->resize(1);
            (*pi_acc)[0] = 1.0;  // TODO: ma è giusto 1 invece che t????
        }
        return true;
    }
    if (t <= 0) {
        if (verboseLvl >= VL_BASIC)
            cout << "UNIFORMIZATION: trivial solution for t=0." << endl;
        if (pi != nullptr)
            *pi = pi0;
        if (pi_acc != nullptr)
            *pi_acc = pi0;  // TODO: ma il tempo cumulativo non è zero?
        return true;
    }

    // Find the uniformization factor lambda (the highest Poisson rate in Q)
    double lambda = abs(Q(0, 0));
    for (size_t n = 0; n < rows.count(); n++)
        lambda = max(lambda, abs(Q(rows[n], rows[n])));
    lambda *= 1.02; // Avoid periodicity [Wallace, Rosenberg, 1966]

    ublas::identity_matrix<double> I(N);
    ublas::compressed_matrix<double> P = I + (Q / lambda);

    switch (trAlgo)	{
    case TA_STANDARD_UNIFORMIZATION:
        StandardUniformization(P, lambda, pi0, pi, pi_acc,
                               t, epsilon, rows, cols, verboseLvl);
        return true;

    case TA_FOX_GLYNN_UNIFORMIZATION:
        return FoxGlynnUniformization(P, lambda, pi0, pi, pi_acc,
                                      t, epsilon, rows, cols, verboseLvl);

    default:
        return false;
    }
}
#endif


#if 0
//-----------------------------------------------------------------------------
/// Compute Uniformization parameters for a given input set
///   \param Q			Input CTMC.
///   \param t			Time elapsed.
///   \param epsilon    Required accuracy.
///   \param rows		Filtered rows of Q.
///   \param outLambda  On exit contains the highest exit rate of \a Q rows.
///   \param outM		On exit contains the number of independent unif. steps.
///   \param outR		On exit contains the right Taylor series truncation.
//-----------------------------------------------------------------------------
template<class Matrix, class RowSelector>
inline void _GetUniformizationParams(const Matrix &Q, const double t,
                                     const double epsilon,
                                     const RowSelector &rows,
                                     double &outLambda,
                                     size_t &outM, size_t &outR) {
    // Get the uniformization factor lambda (highest Poisson rate in Q rows)
    int r0 = rows[0];
    outLambda = abs(Q(r0, r0));
    for (size_t n = 1; n < rows.count(); n++)
        outLambda = max(outLambda, abs(Q(rows[n], rows[n])));

    // Avoid periodicity [Wallace, Rosenberg, 1966] and invalid lambdas
    outLambda = (outLambda != 0.0) ? (outLambda * 1.02) : 0.1;

    // Number of uniformization steps M: solution of pi(0) => pi(t) is divided
    // into M steps:  pi(0) => pi(t/M) => pi(2*t/M) => ... => pi(t)
    double MAX_LAMBDA_T = 290.0;
    outM = 1 + size_t(floor((outLambda * t) / MAX_LAMBDA_T));
    const double tM = t / outM;  // tm is the time step

    // Find the number R of iterations needed to reach the required accuracy
    // R is the right truncation point of the Taylor series.
    outR = 0;
    const double expLambdaT = exp(-outLambda * tM);
    double sigma = 1;
    double xi = 1;
    double eta = (1 - epsilon / outM) / expLambdaT;
    while (sigma < eta) {
        outR++;
        xi *= (outLambda * tM) / outR;
        sigma += xi;
    }
}



//-----------------------------------------------------------------------------
/// Standard Uniformization (SU) method.
///   \param Q		   CTMC.
///   \param pi0	   Initial distribution vector.
///   \param piExpQ	   On return contains the transient solution at time t.
///   \param piIntExpQ On return contains the cumulative probability.
///   \param t		   Transient solution time.
///   \param epsilon   Max. relative error in the iterative method.
///   \param ked	   Forward of backward uniformization
///   \param rows      Selected rows.
///   \param cols      Selected columns.
///   \param printOut  Print informations on the console.
///
/// \note
/// This method implements the Standard uniformization method, with both
/// instantaneous and cumulative transient solution of a CTMC. \n
/// Instantaneous transient probabilities follows the formula:
///  \f[ \pi(t) = \sum^R_{n=0} \Phi(n) \cdot \beta(n, qt) \f]
/// with \f$ \Phi(n) = \Phi(n-1) \cdot P, ~~\Phi(0) = \pi_0 \f$ the probability
/// distribution after \e n steps of the uniformized process \f$ P=I+Q/q \f$,
/// for some \f$ q\geq max|-q_{ii}| \f$, and
/// \f$ \beta(n, qt)=\frac{e^{-qt}(qt)^n}{n!} \f$ the Poisson distribution. \n
/// Cumulative probabilities follows the formula:
///  \f[\pi_c(t)=\frac{1}{q}\sum^R_{n=0}\Phi(n)\cdot\sum^R_{k=n+1}\beta(k,qt)\f]
/// \par
/// For the instantaneous, see "Numerical solution of Markov Chains",
/// W.J. Stewart, page 417. The cumulative solution, instead, is based on the
/// pseudocode provided in "Transient Analysis of Cumulative Measures of
/// Markov Model Behavior", A. Reibman & K. Trivedi. Cumulative probability
/// refers to the *normalized* cumulative measure, therefore norm(pi_acc)=1.
/// Note that both solutions are computed with error tolerance epsilon, but if
/// the cumulative solution L(t) with norm(L(t))=t, then the solution must be
/// computed with \f$ \epsilon/t \f$ tolerance. See the  Reibman & Trivedi
/// paper, page 10, for details.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorIn, class VectorOut,
         class RowSelector, class ColSelector>
void Uniformization2(const Matrix &Q, const VectorIn &pi0,
                     VectorOut &piExpQ, VectorOut *p_piIntExpQ,
                     const double t, const double epsilon,
                     const KolmogorovEquationDirection ked,
                     const RowSelector &rows, const ColSelector &cols,
                     NullIterPrintOut &printOut) {
    const size_t N = Q.size1();
    assert(pi0.size() == N && Q.size2() == N);
    piExpQ.resize(N);
    if (p_piIntExpQ != nullptr)
        p_piIntExpQ->resize(N);

    double lambda;	// Uniformization factor (highest rate in Q rows)
    size_t M;		// Number of uniformization steps M
    size_t R;		// Right Taylor series tail
    size_t vecMatProdCount = 0;
    _GetUniformizationParams(Q, t, epsilon, rows, lambda, M, R);
    const double tM = t / M;  // Time computed in every M step
    const double expLambdaT = exp(-lambda * tM);

    // Uniformization is carried out in M steps: each step advances
    // the probability distribution for (t/M) time units.
    VectorOut Phi, PhiTmp(N);
    set_vec(piExpQ, 0.0, cols);
    if (p_piIntExpQ != nullptr)
        set_vec(*p_piIntExpQ, 0.0, cols);

    for (size_t m = 0; m < M; m++) {
        // Phi(0) = pi0
        // beta(k,qt) = e^(-qt) * ((qt)^k)/k!    Poisson coefficient
        // gamma(k,qt) = complement of the k-th cumulative Poisson coefficient
        copy_vec(Phi, (m == 0 ? pi0 : piExpQ), rows);
        double beta = 1.0;
        double gamma = 1.0 - expLambdaT;

        assign_mult(piExpQ, beta * expLambdaT, Phi, cols);
        if (p_piIntExpQ != nullptr)
            plus_assign_mult(*p_piIntExpQ, gamma / lambda, Phi, cols);

        for (size_t k = 1; k <= R; k++) {
            // Phi(k) = Phi(k-1) P     Probability after k steps of the DTMC P
            switch (ked) {
            case KED_FORWARD:
                prod_vecmat_unif(PhiTmp, Phi, Q, lambda, rows, cols);
                break;
            case KED_BACKWARD:
                prod_matvec_unif(PhiTmp, Q, Phi, lambda, rows, cols);
                break;
            }
            Phi.swap(PhiTmp);

            // Next Poisson coefficient:  beta(k,qt) = beta(k-1,qt) * (qt)/k
            beta *= ((lambda * tM) / k);
            gamma -= beta * expLambdaT;

            plus_assign_mult(piExpQ, beta * expLambdaT, Phi, cols);
            if (p_piIntExpQ != nullptr)
                plus_assign_mult(*p_piIntExpQ, gamma / lambda, Phi, cols);
            printOut.onUnif(t, lambda, M, R, ++vecMatProdCount);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
/// General Uniformization method.
///   \param Q         CTMC.
///   \param pi0       Initial distribution vector.
///   \param piExpQ    On return contains the transient solution at time t.
///   \param piIntExpQ On return contains the cumulative probability.
///   \param fg        Probability density function of the general event.
///   \param epsilon   Max. relative error in the iterative method.
///   \param ked       Forward of backward uniformization
///   \param rows      Selected rows.
///   \param cols      Selected columns.
///   \param printOut  Print informations on the console.
//-----------------------------------------------------------------------------

template<class Matrix, class VectorIn, class VectorOut,
         class RowSelector, class ColSelector>
void UniformizationGen(const Matrix &Q, const VectorIn &pi0,
                       VectorOut &piExpQ, VectorOut *p_piIntExpQ,
                       const char *fg, const double epsilon,
                       const KolmogorovEquationDirection ked,
                       const RowSelector &rows, const ColSelector &cols,
                       NullIterPrintOut &printOut) 
{
    const size_t N = Q.size1();
    assert(pi0.size() == N && Q.size2() == N);
    piExpQ.resize(N);
    if (p_piIntExpQ != nullptr)
        p_piIntExpQ->resize(N);

    // Get the uniformization factor lambda (highest Poisson rate in Q rows)
    int r0 = rows[0];
    double lambda = abs(Q(r0, r0));
    for (size_t n = 1; n < rows.count(); n++)
        lambda = max(lambda, abs(Q(rows[n], rows[n])));

    // Avoid periodicity [Wallace, Rosenberg, 1966] and invalid lambdas
    lambda = (lambda != 0.0) ? (lambda * 1.02) : 0.1;

    // Get the alpha factors
    try {
        const std::vector<std::pair<double, double>> &alphaFactors 
            = ch_compute_alpha_factors_dbl(fg, lambda, epsilon);

        set_vec(piExpQ, 0.0, cols);
        if (p_piIntExpQ != nullptr)
            set_vec(*p_piIntExpQ, 0.0, cols);
        // Phi(0) = pi0
        VectorOut Phi, PhiTmp(N);
        copy_vec(Phi, pi0, rows); // copy Phi <- pi0

        for (size_t k = 0; k <= alphaFactors.size(); k++) {
            //    piExpQ += alpha-factor(fg, k) * Phi(k)
            // piIntExpQ += alpha-factor(1-Fg, k) * Phi(k)
            plus_assign_mult(piExpQ, alphaFactors[k].first, Phi, cols);
            if (p_piIntExpQ != nullptr)
                plus_assign_mult(*p_piIntExpQ, alphaFactors[k].second, Phi, cols);
            printOut.onUnifGen(fg, lambda, alphaFactors.size(), k);

            if (k == alphaFactors.size() - 1) // last interation
                break;

            // Phi(k) = Phi(k-1) * P     Probability after k steps of the DTMC P
            switch (ked) {
                case KED_FORWARD:
                    prod_vecmat_unif(PhiTmp, Phi, Q, lambda, rows, cols);
                    break;
                case KED_BACKWARD:
                    prod_matvec_unif(PhiTmp, Q, Phi, lambda, rows, cols);
                    break;
            }
            Phi.swap(PhiTmp);
        }
        printOut.onUnifGen(fg, lambda, alphaFactors.size(), alphaFactors.size());
    }
    catch (const char *e) {
        throw program_exception(e);
    }
}

//-----------------------------------------------------------------------------

template<class Matrix, class VectorIn, class VectorOut,
         class RowSelector, class ColSelector>
void UniformizationDet(const Matrix &Q, const VectorIn &pi0,
                       VectorOut &piExpQ, VectorOut *p_piIntExpQ,
                       double t, const double epsilon,
                       const KolmogorovEquationDirection ked,
                       const RowSelector &rows, const ColSelector &cols,
                       NullIterPrintOut &printOut)
{
    std::string fg("DiracDelta(");
    fg += std::to_string(t);
    fg += ")";

    UniformizationGen(Q, pi0, piExpQ, p_piIntExpQ, fg.c_str(),  
                      epsilon, ked, rows, cols, printOut);
}

//-----------------------------------------------------------------------------
/* @}  Numerical documentiation group. */
#endif  // __NUMERIC_TRANSIENT_H__
