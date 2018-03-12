//-----------------------------------------------------------------------------
/// \file markov.h
/// Analysis of Markov Chains.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __NUMERIC_MARKOV_H__
#define __NUMERIC_MARKOV_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Finds the Steady State solution of an ergodic Markov Chain.
///   \param A			Markov chain matrix (must be ergodic).
///   \param d			Diagonal vector added to \a A.
///   \param sol		On return contains the solution vector.
///   \param pi0		Initial probability vector.
///   \param rho        Backward boundary condition.
///   \param ked        Forward or backward solution.
///   \param spar		Method parameters.
///   \param printOut   Print informations on the console.
///   \param mcClass    Name of the Markov Chain class of \a A.
///   \param init_x0    Initial distribution of x(0), (nullptr = uniform).
///   \return  Returns true if the solution has been found, false if the
///            iterative method does not converge in maxIters iterations.
/// \note
/// Steady state solutions are computed with the \f$(A+Id)\f$ matrix.
/// Therefore, set d=0 for CTMCs, and d=-1 for DTMCs.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorS, class VectorD, class VectorPi, class VectorRho>
bool Solve_SteadyState_ErgodicMC(const Matrix &A,
                                 VectorS &sol,
                                 const VectorD &d,
                                 const VectorPi &pi0,
                                 const VectorRho &rho,
                                 KolmogorovEquationDirection ked,
                                 const SolverParams &spar,
                                 NullIterPrintOut &printOut,
                                 const char *mcClass,
                                 const VectorS *init_x0 = nullptr) {
    const size_t N = A.size1();
    assert(A.size2() == N && d.size() == N);
    sol.resize(N);

    if (printOut.getVerboseLevel() >= VL_BASIC) {
        cout << KED_Names[ked] << " STEADY STATE SOLUTION ";
        cout << "OF ERGODIC " << mcClass << "." << endl;
    }

    // Solve the Markov Chain with an iterative method
    full_element_selector all_elems(N);
    ublas::zero_vector<double> zeroB(N);
    bool solFound = LinearSolve(sol, A, zeroB, d, KED_FORWARD, spar,
                                printOut, all_elems, all_elems, init_x0);

    // Normalize the solution vector
    sol /= vec_sum(sol) * vec_sum(pi0);

    if (ked == KED_BACKWARD) {
        // Compute the backward solution from the forward one
        double unif_value = vec_dot(sol, rho, all_elems);
        set_vec(sol, unif_value, all_elems);
    }

    return solFound;
}



//-----------------------------------------------------------------------------
/// Finds the Steady State solution of any Markov Chain.
///   \param A			Markov chain matrix (may be reducible).
///   \param d			Diagonal vector added to \a A.
///   \param sol		On return contains the solution vector.
///   \param pi0		Initial probability vector at iteration t=0.
///   \param rho        Backward boundary condition.
///   \param ked        Forward or backward solution.
///   \param spar		Method parameters.
///   \param bscc       BSCC of the \a A Markov chain.
///   \param printOut   Print informations on the console.
///   \param mcClass    Name of the Markov Chain class of \a A.
///   \param init_x0    Initial distribution of x(0), (nullptr = uniform).
///   \return  Returns true if the solution has been found, false if the
///            iterative method does not converge in maxIters iterations.
/// Steady state solutions are computed with the \f$(A+Id)\f$ matrix.
/// Therefore, set d=0 for CTMCs, and d=-1 for DTMCs.
//-----------------------------------------------------------------------------
template<class Matrix, class VectorS, class VectorPi, class VectorRho,
         class VectorD>
bool Solve_SteadyState_MC(const Matrix &A, VectorS &sol, const VectorD &d,
                          const VectorPi &pi0, const VectorRho &rho,
                          KolmogorovEquationDirection ked,
                          const SolverParams &spar,
                          const BSCC &bscc, NullIterPrintOut &printOut,
                          const char *mcClass, const VectorS *init_x0 = nullptr) {
    const size_t N = A.size1();
    assert(A.size2() == N && pi0.size() == N && d.size() == N);
    sol.resize(N);
    std::fill(sol.begin(), sol.end(), 0);

    NullIterPrintOut *pInnerPrint = &printOut, nullPrinter;
    if (printOut.getVerboseLevel() >= VL_BASIC)
        pInnerPrint = &nullPrinter;

    typedef BaseSubsetList<SccIndexTo0BasedIndexWithTransients,
                           sccindex_t, size_t> SCCSubsetList2;
    full_element_selector all_elems(N);

    if (ked == KED_FORWARD) {
        assert(bscc.indices.size() == N);
        if (bscc.numBscc == sccindex_t(0)) {
            // A is ergodic, solve it with a simpler method
            return Solve_SteadyState_ErgodicMC(A, sol, d, pi0, rho, ked, spar,
                                               printOut, mcClass, init_x0);
        }
        if (printOut.getVerboseLevel() >= VL_BASIC) {
            cout << KED_Names[ked] << " STEADY STATE SOLUTION OF NON-ERGODIC ";
            cout << mcClass << " WITH " << bscc.numBscc << " RECURRENCE CLASSES." << endl;
        }

        // Transient selector and recurrent selectors
        assert(bscc.indices.size() == N);
        subset_element_selector<sccindex_t> transient_selector(&bscc.indices, -1);
        subset_element_selector<sccindex_t, not_equal_to<sccindex_t> > recurrent_selector(&bscc.indices, -1);
        full_element_selector all_elems(N);
        SCCSubsetList2 sccSubsets;
        std::vector<sccindex_t> bsccIndices(bscc.indices);
        sccSubsets.ConstructSubsetList(size_t(bscc.numBscc) + 1, bsccIndices);
        ublas::zero_vector<double> zeroB(N);

        // ================ Forward non-ergodic solution ====================
        // sigma0 = mean sojourn time in the transient states until absorption
        ublas::vector<double> sigma0(N, 0.0);
        double transient_prob = vec_sum(pi0, transient_selector);
        if (transient_prob > 0) {
            if (!LinearSolve(sigma0, A, -pi0, d, KED_FORWARD, spar, printOut,
                             transient_selector, transient_selector, init_x0))
                return false; // Does not converge...
        }

        // Compute how the transient probability enters each recurrent class
        ublas::vector<double> pi_rec(N);
        set_vec(pi_rec, 0.0, all_elems);
        prod_vecmat(pi_rec, sigma0, A, all_elems, all_elems);
        add_vec(pi_rec, pi0, recurrent_selector);
        double norm_coeff = vec_sum(pi_rec, recurrent_selector);
        div_vec(pi_rec, norm_coeff, recurrent_selector);


        // Compute the (weighted) steady-state solution of each BSCC
        for (sccindex_t nc(0); nc < bscc.numBscc; nc++) {
            const SCCSubsetList2::selector bscc_selector = sccSubsets.selectorOf(int(nc));
            double prob_bscc = vec_sum(pi_rec, bscc_selector);

            if (printOut.getVerboseLevel() >= VL_VERBOSE)
                cout << "  ProbBscc[" << nc << "] = " << prob_bscc << endl;
            if (bscc_selector.count() == 1) { // Absorbing states
                const size_t i = bscc_selector[0];
                sol(i) = prob_bscc;
                continue;
            }
            if (!LinearSolve(sol, A, zeroB, d, KED_FORWARD, spar, *pInnerPrint,
                             bscc_selector, bscc_selector, init_x0))
                return false; // Does not converge

            // Normalize
            norm_coeff = vec_sum(sol, bscc_selector) / prob_bscc;
            div_vec(sol, norm_coeff, bscc_selector);
        }
    }
    else { // ked == KED_BACKWARD
        // ================ Backward non-ergodic solution ====================

        // if (printOut.getVerboseLevel() >= VL_BASIC)
        //     cout << KED_Names[ked] << " STEADY STATE SOLUTION." << endl;

        // full_element_selector all_elems(N);
        // // Solve the transient backward system:   (A+Id) x = -rho_T
        // if (!LinearSolve(sol, A, rho, d, KED_BACKWARD, spar, printOut,
        //                  all_elems, all_elems, init_x0))
        //     return false; // Does not converge


        assert(bscc.indices.size() == N);
        if (bscc.numBscc == sccindex_t(0)) {
            if (printOut.getVerboseLevel() >= VL_BASIC)
                cout << "BACKWARD ERGODIC STEADY STATE SOLUTION OF " << mcClass << "." << endl;

            // Solve the transient backward system:   (A+Id) x = -rho_T
            return LinearSolve(sol, A, rho, d, KED_BACKWARD, spar, printOut,
                               all_elems, all_elems, init_x0);
        }
        else {
            if (printOut.getVerboseLevel() >= VL_BASIC) {
                cout << "BACKWARD NON-ERGODIC STEADY STATE SOLUTION OF ";
                cout << mcClass << " WITH " << bscc.numBscc << " RECURRENCE CLASSES." << endl;
            }

            ublas::vector<double> sigma0(N, 0.0);

            subset_element_selector<sccindex_t> transient_selector(&bscc.indices, -1);
            subset_element_selector<sccindex_t, not_equal_to<sccindex_t> > recurrent_selector(&bscc.indices, -1);
            SCCSubsetList2 sccSubsets;
            std::vector<sccindex_t> bsccIndices(bscc.indices);
            sccSubsets.ConstructSubsetList(size_t(bscc.numBscc) + 1, bsccIndices);

            // Compute the (weighted) steady-state solution of each BSCC
            for (sccindex_t nc(0); nc < bscc.numBscc; nc++) {
                const SCCSubsetList2::selector bscc_selector = sccSubsets.selectorOf(int(nc));

                if (bscc_selector.count() == 1) { // Absorbing states
                    const size_t i = bscc_selector[0];
                    sigma0(i) = rho(i);
                    sol(i) = rho(i);
                    continue;
                }
                if (!LinearSolve(sigma0, A, rho, d, KED_BACKWARD, spar, *pInnerPrint,
                                 bscc_selector, bscc_selector, init_x0))
                    return false; // Does not converge

                add_vec(sol, sigma0, bscc_selector);
            }
            set_vec(sigma0, 0.0, transient_selector);

            // Compute how the recurrent rewards propagate beckward to the transient states
            ublas::vector<double> rho_transient(N, 0.0);
            prod_matvec(rho_transient, A, sigma0, transient_selector, all_elems);
            add_vec(rho_transient, rho, recurrent_selector);
            // double norm_coeff = vec_sum(pi_rec, recurrent_selector);
            // div_vec(pi_rec, norm_coeff, recurrent_selector);

            // cout << "sigma0        = " << sigma0 << endl;
            // cout << "rho           = " << rho << endl;
            // cout << "rho_transient = " << rho_transient << endl;

            double transient_reward = vec_sum(rho_transient, transient_selector);
            if (transient_reward > 0) {
                return LinearSolve(sol, A, -rho_transient, d, KED_BACKWARD, spar, printOut,
                                   transient_selector, transient_selector, init_x0);
            }
        }


        /*/ Compute the uniform solution vector of each terminal bscc
        assert(bscc.indices.size() == N);
        for (sccindex_t nc(0); nc<bscc.numBscc; nc++) {
        	const SCCSubsetList2::selector bscc_selector = sccSubsets.selectorOf(int(nc));
        	if (bscc_selector.count() == 1) {
        		// Special case for absorbing states
        		const size_t i = bscc_selector[0];
        		sol(i) = rho(i);
        		continue;
        	}
        	// Compute the stationary backward solution from the forward solution,
        	// using the relation:  x = sol(bscc) * rho
        	double norm_of_rho = vec_sum(rho, bscc_selector), unif_value;
        	if (norm_of_rho == 0.0)
        		unif_value = 0.0;
        	else {
        		if (!LinearSolve(sol, A, zeroB, d, KED_FORWARD, spar, *pInnerPrint,
        						 bscc_selector, bscc_selector, init_x0))
        			return false; // Does not converge
        		double norm_coeff = vec_sum(sol, bscc_selector);
        		div_vec(sol, norm_coeff, bscc_selector);
        		unif_value = vec_dot(rho, sol, bscc_selector);
        	}
        	set_vec(sol, unif_value, bscc_selector);
        }

        // Compute how the backward probability goes back into transient states.
        ublas::vector<double> rho_T(N);
        prod_matvec(rho_T, A, sol, transient_selector, all_elems);

        // Solve the transient backward system:   (A+Id) x = -rho_T
        if (!LinearSolve(sol, A, -rho_T, d, KED_BACKWARD, spar, printOut,
        				 transient_selector, transient_selector, init_x0))
        	return false; // Does not converge
        */
    }

    return true;
}


//-----------------------------------------------------------------------------
/// Finds the Steady State solution of an ergodic CTMC.
///   \param Q			Infinitesimal generator matrix (must be ergodic).
///   \param sol		On return contains the solution vector.
///   \param ked        Forward or backward solution.
///   \param spar		Method parameters.
///   \param printOut   Print informations on the console.
///   \param init_x0    Initial distribution of x(0), (nullptr = uniform).
///   \return  Returns true if the solution has been found, false if the
///            iterative method does not converge in maxIters iterations.
//-----------------------------------------------------------------------------
template<class Matrix, class Vector>
bool Solve_SteadyState_ErgodicCTMC(const Matrix &Q, Vector &sol,
                                   KolmogorovEquationDirection ked,
                                   const SolverParams &spar,
                                   NullIterPrintOut &printOut,
                                   const Vector *init_x0 = nullptr) {
    const size_t N = Q.size1();
    ublas::zero_vector<double> zeroD(N);

    return Solve_SteadyState_ErgodicMC(Q, sol, zeroD, ked, spar,
                                       printOut, "CTMC", init_x0);
}


//-----------------------------------------------------------------------------
/// Finds the Steady State solution of any CTMC.
///   \param Q			Infinitesimal generator matrix (ergodic or not).
///   \param sol		On return contains the solution vector.
///   \param pi0		Initial probability vector at time t=0.
///   \param rho        Backward boundary condition.
///   \param ked        Forward or backward solution.
///   \param spar		Method parameters.
///   \param bscc       BSCC of the \a A Markov chain.
///   \param printOut   Print informations on the console.
///   \param init_x0    Initial distribution of x(0), (nullptr = uniform).
///   \return  Returns true if the solution has been found, false if the
///            iterative method does not converge in maxIters iterations.
//-----------------------------------------------------------------------------
template<class Matrix, class Vector1, class Vector2, class Vector3>
bool Solve_SteadyState_CTMC(const Matrix &Q, Vector1 &sol, const Vector2 &pi0,
                            const Vector3 &rho, KolmogorovEquationDirection ked,
                            const SolverParams &spar, const BSCC &bscc,
                            NullIterPrintOut &printOut, const Vector1 *init_x0 = nullptr) {
    const size_t N = Q.size1();
    ublas::zero_vector<double> zeroD(N);

    return Solve_SteadyState_MC(Q, sol, zeroD, pi0, rho, ked, spar, bscc,
                                printOut , "CTMC", init_x0);
}


//-----------------------------------------------------------------------------
/// Finds the Steady State solution of an ergodic DTMC.
///   \param P			Probabilistic matrix (must be ergodic).
///   \param sol		On return contains the solution vector.
///   \param ked        Forward or backward solution.
///   \param spar		Method parameters.
///   \param printOut   Print informations on the console.
///   \param init_x0    Initial distribution of x(0), (nullptr = uniform).
///   \return  Returns true if the solution has been found, false if the
///            iterative method does not converge in maxIters iterations.
//-----------------------------------------------------------------------------
template<class Matrix, class Vector>
bool Solve_SteadyState_ErgodicDTMC(const Matrix &P, Vector &sol,
                                   KolmogorovEquationDirection ked,
                                   const SolverParams &spar,
                                   NullIterPrintOut &printOut,
                                   const Vector *init_x0 = nullptr) {
    const size_t N = P.size1();
    ublas::scalar_vector<double> minusOneD(N, -1.0);

    return Solve_SteadyState_ErgodicMC(P, sol, minusOneD, ked, spar,
                                       printOut, "DTMC", init_x0);
}


//-----------------------------------------------------------------------------
/// Finds the Steady State solution of any DTMC.
///   \param P			Probabilistic matrix (ergodic or not).
///   \param sol		On return contains the solution vector.
///   \param pi0		Initial probability vector at iteration t=0.
///   \param rho        Backward boundary condition.
///   \param ked        Forward or backward solution.
///   \param spar		Method parameters.
///   \param bscc       BSCC of the \a A Markov chain.
///   \param printOut   Print informations on the console.
///   \param init_x0    Initial distribution of x(0), (nullptr = uniform).
///   \return  Returns true if the solution has been found, false if the
///            iterative method does not converge in maxIters iterations.
//-----------------------------------------------------------------------------
template<class Matrix, class Vector1, class Vector2, class Vector3>
bool Solve_SteadyState_DTMC(const Matrix &P, Vector1 &sol, const Vector2 &pi0,
                            const Vector3 &rho, KolmogorovEquationDirection ked,
                            const SolverParams &spar, const BSCC &bscc,
                            NullIterPrintOut &printOut, const Vector1 *init_x0 = nullptr) {
    const size_t N = P.size1();
    ublas::scalar_vector<double> minusOneD(N, -1.0);

    return Solve_SteadyState_MC(P, sol, minusOneD, pi0, rho, ked, spar,
                                bscc, printOut, "DTMC", init_x0);
}


//-----------------------------------------------------------------------------
/* @}  Numerical documentiation group. */
#endif  // __NUMERIC_MARKOV_H__
