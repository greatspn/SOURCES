//-----------------------------------------------------------------------------
/// \file numeric.h
/// Declarations of various Sparse Matrix algorithms.
///
/// This is the primary header for a small library for Markov chain
/// manipulation. Most of the algorithms implemented here are in template form.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __NUMERIC_H__
#define __NUMERIC_H__

//-----------------------------------------------------------------------------

#ifdef NDEBUG
# ifdef __GNUC__
#  define BOOST_UBLAS_INLINE  __attribute__((always_inline))
# endif
#endif

#ifdef WIN32
//# ifdef isnan
//#  undef isnan
//# endif
//# define isnan(x)  _isnan(x)
#define sleep(x)  Sleep(1000 * (x))
//inline long round(double x)  {  return (x>0) ? long(x+.5) : long(x-.5);  }
//typedef  long	   ssize_t;
#endif

#ifdef __MINGW32__
#include <windows.h>
# define sleep(x)  Sleep(1000 * (x))
#endif

// Include Boost-C++ uBlas headers
// #define BOOST_UBLAS_MOVE_SEMANTICS  1
#define BOOST_UBLAS_USE_ITERATING   1
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>

// #include "../alphaFactory/alphaFactory.h"
const std::vector<pair<double, double>>&
ch_compute_alpha_factors_dbl(const char* fg_expr, const double q, const double accuracy);

namespace ublas = boost::numeric::ublas;

//=============================================================================

// Runtime exception raised in case of unrecoverable errors
class program_exception : public std::exception {
    string expl;    // explanation
public:
    inline program_exception(const char *e) : expl(e) { }
    inline program_exception(const string &e) : expl(e) { }
    virtual ~program_exception() throw()   { }
    virtual const char *what() const throw()   { return expl.c_str(); }
};

//-----------------------------------------------------------------------------

#ifndef BEGIN_NAMESPACE
# define BEGIN_NAMESPACE(n)	 namespace n {
#endif

#ifndef END_NAMESPACE
# define END_NAMESPACE(n)  }
#endif

#ifndef UNUSED_PARAM
# define UNUSED_PARAM(p)   ((void)(p))
#endif

#ifdef NULL
# undef NULL
#endif

//-----------------------------------------------------------------------------
/// Namespace of the various numerical methods
BEGIN_NAMESPACE(numerical)
/** \defgroup Numeric  Numeric Functions for Sparse Matrices and Markov Chains
 *  \brief Various methods for Markov Chains manipulation, included Steady
 *         State and Transient solution and BSCC analysis.
 */
/*  @{ */
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Available Solution Algorithms
//-----------------------------------------------------------------------------

/// Steady State algorithms implemented
enum ExplicitLinearSolverAlgo {
    ELA_JOR,			///< The Jacobi Iterative Method (with over-relaxation)
    ELA_FORWARD_SOR,	///< SOR method with forward Gauss-Seidel iterations
    ELA_BACKWARD_SOR,	///< SOR method with backward Gauss-Seidel iterations
    ELA_SYMMETRIC_SOR,	///< Symmetric SOR method
    ELA_GMRES_M,		///< Generalized Minimum Residual
    ELA_BICGSTAB,		///< Bi-Conjugate Gradients
    ELA_CGS,			///< Conjugate Gradients Squared
    ELA_NUM_ALGORITHMS	///< Number of explicit steady-state algorithms
};

/// Names of the \ref ExplicitLinearSolverAlgo, without over-relaxation
extern const char *ELA_Names[ELA_NUM_ALGORITHMS];

/// Names of the \ref ExplicitLinearSolverAlgo, with over-relaxation
extern const char *ELA_Names_OR[ELA_NUM_ALGORITHMS];

/// Is a Krylov-subspace method
extern bool ELA_Is_Krylov[ELA_NUM_ALGORITHMS];


/// Transient algorithms implemented
enum TransientAlgorithm {
    TA_STANDARD_UNIFORMIZATION,		///< Standard uniformization method
    TA_FOX_GLYNN_UNIFORMIZATION		///< Fox-Glynn uniformization method
};


/// Implicit steady state algorithms
enum ImplicitLinearSolverAlgo {
    ILA_POWER_METHOD,	///< Power method
    ILA_GMRES_M,		///< Generalized Minimum Residual
    ILA_BICGSTAB,		///< Bi-Conjugate Gradients
    ILA_CGS,			///< Conjugate Gradients Squared
    ILA_NUM_ALGORITHMS	///< Number of implicit steady-state algorithms
};

/// Names of the \ref ImplicitLinearSolveAlgo methods.
extern const char *ILA_Names[ILA_NUM_ALGORITHMS];

/// Preconditioner algorithms
enum PreconditionerAlgo {
    PA_NULL_PRECONDITIONER,	///< No preconditioner is used
    PA_DIAG,				///< Diagonal (Jacobi) preconditioner
    PA_ILU0,				///< ILU(0) factorization without fill-ins
    PA_ILUTK,				///< ILUTK factorization with threshold
    PA_NUM_PRECONDITIONER_ALGO
};

/// Names of the \ref Preconditioner methods.
extern const char *PA_Names[PA_NUM_PRECONDITIONER_ALGO];

/// Preconditioning strategy for implicit MRP solution
enum ImplictPreconditionStrategy {
    IPS_NO_PRECONDITIONER,  ///< No preconditioner for implicit MRP solution
    IPS_INNEROUTER_EMC,     ///< Inner/outer precond. with A = explicit EMC
    IPS_INNEROUTER_SIM,     ///< Inner/outer precond. with A = simulated EMC
    IPS_PREC_EMC,           ///< Precond. is built from the *explicit* EMC
    IPS_PREC_SIM,           ///< Precond. is built from a randomly simulated EMC
    IPS_NUM_PRECONDITIONER_SOURCES
};

/// The IPS strategy builds an inner/outer preconditioner?
extern const bool IPS_isInnerOuter[IPS_NUM_PRECONDITIONER_SOURCES];

/// IPS strategy name
extern const char *IPS_Names[IPS_NUM_PRECONDITIONER_SOURCES];


/// Verbose levels of various methods
enum VerboseLevel {
    VL_NONE,		 ///< Do not print anything
    VL_BASIC,		 ///< Print only basic informations
    VL_VERBOSE,		 ///< Verbose output
    VL_VERY_VERBOSE  ///< Very verbose output
};

// Returns true if the current run of the tool is called by
// the GUI, which can be used to rearrange some outputs.
bool invoked_from_gui();

// Should we also print statistics in the format of the GUI?
bool print_stat_for_gui();
void enable_print_stat_for_gui();

/// Preferred solution strategy for MRPs
enum LinearSolutionStrategy {
    LSS_IMPLICIT,	///< Implicit matrix-free method
    LSS_EXPLICIT,	///< Explicit construction of the system matrix
    LSS_UNSPECIFIED
};

// Component method strategy
enum ComponentMethod {
    PCM_GREEDY,
    PCM_ILP,
    PCM_POLYNOMIAL_ILP
};

//-----------------------------------------------------------------------------

/// Numerical solvers parameters
struct SolverParams {
    double		omega;		///< Over-relaxation coefficient.
    double		epsilon;	///< Requested error tolerance.
    size_t		maxIters;	///< Max. number of iterations.
    size_t		KrylovM;	///< Number of Arnoldi iterations in GMRES(K).

    /// Implicit linear solver algorithm used
    ImplicitLinearSolverAlgo		implicitAlgo;
    /// Explicit linear solver algorithm
    ExplicitLinearSolverAlgo	explicitAlgo;

    /// Right preconditioner strategy
    PreconditionerAlgo			    precAlgo;
    double		ilutkThreshold;		///< Threshold for ILUTK preconditioner
    size_t		ilutkMaxNumEntries; ///< Max row entries in ILUTK preconditioner
    /// Preconditioner strategy used for implicit MRP steady-state solution
    ImplictPreconditionStrategy     implicitPrecStrategy;
    double      inoutEpsilon;       ///< Inner/outer preconditioner accuracy
    size_t      inoutMaxIters;      ///< Inner/outer preconditioner max. number of iterations
    size_t      inoutKrylovM;       ///< Inner/outer preconditioner Arnoldi base
    size_t		ipaNumSimulations;	///< Number of simulation steps
    size_t		ipaSamplesPerRow;	///< Num. of samples in simulated DTMC rows
    double		ipaAlpha;			///< Mixing coefficient (Google trick)
    double		ipaPrecSum;			///< Preconditioner total sum
    // TODO: remove this entry once the preconditioner strategy is more clear
    bool		ipaOldPrec;			///< Old prec (S^E rows have a boost of +1)
    bool		ipaAddSelfLoop;     ///< Add self-loop in approx. simulated EMC
    size_t		rndSeed;			///< Random seed used for simulations

    bool		aggregateComplexComp; ///< Complex component aggregation for SCC analysis
    ComponentMethod compMethod;     ///< What SCC method will be used.
    bool        randomizeSccSelection; ///< Greedy method randomizes scc selection.
    LinearSolutionStrategy solMethod; ///< Preferred linear solution strategy

    std::vector<std::string>  dtaSearchPaths; ///< Directory where DTAs are searched
    bool 		onTheFlyModelChecking; ///< on-the-fly state space construction
    bool 		useSCCDecomp;		///< Use SCC decomposition if possible
    bool        useZonedDTA;		///< Construct the zoned DTA whenever possible
    bool        onTheFlyCompRecursive; ///< Apply the component method on the on-the-fly components
};

//-----------------------------------------------------------------------------

enum ConvergenceFailureReason {
    CFR_TOO_MANY_ITERATIONS,
    CFR_BREAKDOWN,
    CFR_BREAKDOWN_SWITCH_TO_POWERMETHOD
};

//-----------------------------------------------------------------------------

enum KolmogorovEquationDirection {
    KED_FORWARD,
    KED_BACKWARD
};

/// Names of the \a KolmogorovEquationDirection constants.
extern const char *KED_Names[2];

//-----------------------------------------------------------------------------
// Prints a waiting line between iterations iterations
//-----------------------------------------------------------------------------

struct AlgoName {
    AlgoName(ImplicitLinearSolverAlgo i, size_t p = 0) : ilsa(i), elsa(ELA_NUM_ALGORITHMS), param(p) {}
    AlgoName(ExplicitLinearSolverAlgo e) : ilsa(ILA_NUM_ALGORITHMS), elsa(e), param(0) {}
    AlgoName() : ilsa(ILA_NUM_ALGORITHMS), elsa(ELA_NUM_ALGORITHMS), param(0) {}
    AlgoName(const AlgoName &an) : ilsa(an.ilsa), elsa(an.elsa), param(an.param) { }

    ImplicitLinearSolverAlgo ilsa;
    ExplicitLinearSolverAlgo elsa;
    size_t  param;
};
ostream &operator <<(ostream &os, const AlgoName an);

extern AlgoName AN_UNSPECIFIED;		// Unspecified algorithm
extern AlgoName AN_POWER_METHOD;
extern AlgoName AN_BICGSTAB;
extern AlgoName AN_CGS;

class NullIterPrintOut : boost::noncopyable {
public:
    virtual ~NullIterPrintOut() {}
    virtual void onIteration(AlgoName, size_t iter, double resid) {}
    virtual void onConvergence(AlgoName, size_t iter, double resid) {}
    virtual void onFailure(AlgoName, size_t iter, ConvergenceFailureReason cfr) {}
    virtual void onUnif(double t, double lambda, size_t M, size_t R, size_t vecMatProdCount) {}
    virtual void onUnifGen(const char* fg, double lambda, size_t R, size_t vecMatProdCount) {}
    virtual VerboseLevel getVerboseLevel() const { return VL_NONE; }
};

//-----------------------------------------------------------------------------

#ifndef M_PI
/// The \f$\pi\f$ constant, if not defined elsewhere.
# define M_PI  3.1415926535897932384626433832795028841971693993751
#endif

#ifndef M_E
# define M_E   2.7182818284590452353602874713526624977572470936999
#endif

//-----------------------------------------------------------------------------
// Private typedefs
#include "private_integer.h"

// Element selectors for the various numerical methods
#include "selectors.h"

// Additional numeric methods
#include "methods.h"

// Fox-Glynn algorithm
//#include "foxglynn.h"

// Preconditioning
#include "preconditioning.h"

// Iterative stationary methods (LinearSolve, LinearSolveTransposed)
#include "stationary.h"

// Krylov-subspace stationary methods
#include "krylov.h"

// Iterative transient methods
#include "transient.h"

// Tarjan algorithm (BSCC analysis of matrix graphs)
#include "bscc.h"

// Markov Chains analysis
#include "markov.h"
//-----------------------------------------------------------------------------

END_NAMESPACE(numerical)

//-----------------------------------------------------------------------------

namespace std {
/// Hash function for private_integer<T,U> instances.
template<typename T, typename U>
struct hash< numerical::private_integer<T, U> > {
    size_t operator()(const numerical::private_integer<T, U> &pi) const {
        return std::hash<T>()(pi.get());
    }
};
};

//-----------------------------------------------------------------------------
/* @}  Numerical documentiation group. */
#endif  // __NUMERIC_H__
