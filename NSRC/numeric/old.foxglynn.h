//-----------------------------------------------------------------------------
/// \file foxglynn.h
/// Fox-Glynn Algorithm for Poisson probability computation.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __NUMERIC_FOXGLYNN_H__
#define __NUMERIC_FOXGLYNN_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------

#ifndef M_PI
/// The \f$\pi\f$ constant, if not defined elsewhere.
# define M_PI  3.1415926535897932384626433832795028841971693993751
#endif

//-----------------------------------------------------------------------------
/// Auxiliary structures for the Fox-Glynn algorithm implementation
//-----------------------------------------------------------------------------
struct FoxGlynnWeights : boost::noncopyable {
    int				L;	     ///< Left truncation point
    int				R;       ///< Right truncation point
    vector<double>	W;       ///< (R-L) Poisson weights vector
    double			SumW;    ///< Sum of the weights in W
};
typedef struct FoxGlynnWeights  FoxGlynnWeights;


//-----------------------------------------------------------------------------
/// Fox-Glynn's Finder algorithm.
///   \param lambda		Poisson rate.
///   \param epsilon	Error tolerance (should be >= 1e-10).
///   \param tau		Underflow threshold.
///   \param omega		Overflow threshold.
///   \param fgw		Fox-Glynn weight vector storage & truncation points.
///   \param W_M		On return contains the w(M) weight.
///   \param verboseLvl Print informations to the stdout.
///   \return False if FG_Finder couldn't find a valid [L,R] interval.
///
/// \note
/// FG_Finder findes the left and right truncation points for the Poisson
/// distribution with coefficient lambda. It returns false if the underflow
/// conditions are not satisfied. On return, the (L, R, w_M) fields of fgw
/// are filled.
//-----------------------------------------------------------------------------
bool FG_Finder(const double lambda, const double epsilon,
               const double tau, const double omega,
               FoxGlynnWeights &fgw, double &W_M, VerboseLevel verboseLvl);


//-----------------------------------------------------------------------------
/// Fox-Glynn's Weighter algorithm.
///   \param lambda	Poisson rate
///   \param epsilon	Error tolerance. epsilon should be >= 1e-10
///   \param tau		Underflow threshold
///   \param omega		Overflow threshold
///   \param fgw		Fox-Glynn weight vector storage & truncation points
///   \param verboseLvl Print informations to the stdout
///   \return True if weights hav been computed correctly, false otherwise.
///
/// \note
/// FG_Weighter computes the Poisson probabilities with rate lambda, finding
/// the relevant left and right tails of the distribution. Returns true if
/// the computation succeeded, with the fields in fgw initialized.
//-----------------------------------------------------------------------------
bool FG_Weighter(const double lambda, const double epsilon,
                 const double tau, const double omega,
                 FoxGlynnWeights &fgw, VerboseLevel verboseLvl);


//-----------------------------------------------------------------------------
/* @}  Numerical documentiation group. */
#endif   // __NUMERIC_FOXGLYNN_H__
