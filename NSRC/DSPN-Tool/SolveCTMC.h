/*
 *  SolveCTMC.h
 *
 *  Numerical solutions for CTMCs.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __SOLVE_CTMC_H__
#define __SOLVE_CTMC_H__
//=============================================================================

/// Continuous-Time Markov Chain
struct CTMC {
    typedef ublas::compressed_matrix<double>  matrix_t;
    typedef ublas::vector<double>  			  dense_vector_t;

    /// Infinitesimal generator of the CTMC
    matrix_t			Q;
    /// Initial probability distribution vector
    dense_vector_t		pi0;


    inline size_t N() const    {  return Q.size1();  }
};

//=============================================================================
//    Function Definitions:
//=============================================================================

//void GetInitialProbabilities(const RG &rg, ublas::vector<double>& pi0);

void ExtractCTMC(const RG &rg, CTMC &ctmc, VerboseLevel verboseLvl);
void ExtractCTMC(const RG2 &rg, CTMC &ctmc, VerboseLevel verboseLvl);

void SolveSteadyStateCTMC(const CTMC &ctmc, PetriNetSolution &sol,
                          KolmogorovEquationDirection ked,
                          ublas::vector<double> *pRho,
                          const SolverParams &spar, VerboseLevel verboseLvl);

void SolveTransientCTMC(const CTMC &ctmc, PetriNetSolution &sol, double t,
                        KolmogorovEquationDirection ked,
                        const SolverParams &spar, VerboseLevel verboseLvl);

//=============================================================================
#endif   // __SOLVE_CTMC_H__
