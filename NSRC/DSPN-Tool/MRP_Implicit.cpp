/*
 *  MRP_Implicit.cpp
 *
 *  Implementation of the MRP implicit steady state solver.
 *
 *  Created by Elvio Amparore
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <ctime>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <memory>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "ReachabilityGraph.h"
#include "MRP.h"
#include "MRP_Implicit.h"

//=============================================================================

enum SteadyStateComputation {
    SSC_ONLY_UNNORMALIZED_EMC_PROB,
    SSC_COMPUTE_ALL
};
typedef enum SteadyStateComputation  SteadyStateComputation;

//-----------------------------------------------------------------------------

// Performs an implicit vector x matrix product: v' = (P-I).v
// This is needed for the solution of Krylov-subspace methods since the EMC
// is a DTMC matrix. This class is not used for Power Method.
template<class PxVproduct>
class PIxVproduct : boost::noncopyable {
    PxVproduct &emcProd;	// Implicit product with the DTMC P
public:
    inline PIxVproduct(PxVproduct &_emcProd)
        : emcProd(_emcProd) {}

    // Return the square matrix size
    inline size_t size() const				{   return emcProd.size();   }

    // Compute a vector * matrix product: v.(P-I) = v.P - v
    ublas::vector<double> product(const ublas::vector<double> &inVec) const	{
        ublas::vector<double> outVec(emcProd.product(inVec));
        outVec -= inVec;
        return outVec;
    }
};

//-----------------------------------------------------------------------------

template<class VectorX, class VectorB>
double RelativeError(const VectorX &x, const VectorX &next_x, const VectorB &b) {
    const size_t N = x.size();
    assert(next_x.size() == N && b.size() == N);
    double rel_err = 0.0;

    for (size_t i = 0; i < N; i++) {
        rel_err += abs(next_x(i) - x(i)/* - b(i)*/); // TODO: why does b(i) interfere?
    }
    rel_err /= vec_sum(next_x);
    assert(!std::isnan(rel_err));
    return rel_err;
}

//-----------------------------------------------------------------------------

template<class ResidualFunctor, class Vector, class VectorB, class PrintOut>
void PowerMethodIteration(ResidualFunctor &A, Vector &x, const VectorB &b,
                          const SolverParams &spar, double &rel_err,
                          size_t &iterCount, PrintOut &printOut,
                          VerboseLevel verboseLvl) {
    Vector next_x;

    // Power method iteration:  x(k+1) = A.x(k) - b
    next_x = A.product(x);
    next_x -= b;
    iterCount++;

    // Apply over-relaxation
    if (spar.omega != 1.0)
        noalias(next_x) = (spar.omega) * next_x + (1.0 - spar.omega) * x;

    rel_err = RelativeError(x, next_x, b);
    printOut.onIteration(AN_POWER_METHOD, iterCount, rel_err);
    x.swap(next_x);
}

//-----------------------------------------------------------------------------

inline double randAB(double A, double B) {
    return A + (B - A) * (rand() / double(RAND_MAX));
}

template < class ResidualFunctor, class Vector, class VectorB,
           class Real, class PrintOut >
int
IR2(const ResidualFunctor &A, Vector &x, const VectorB &b,
    const Preconditioner &L, const Preconditioner &R,
    size_t &max_iter, Real &tol, Real omega, PrintOut &printOut) {
    // A.x = b  =>  L.A.R.R^-1.x = L.b
    // (L.A.R - I).x + x = L.b
    // x(k+1) = x(k) + omega * L.(b - A.R.x(k))
    Real resid = 1;
    Vector z, y, r;

    Real normb = norm_2(b);
    if (less_than_machine_epsilon(normb * tol))
        normb = 1;
    //cout << "NORM[b] = " << normb << ",  omega = " << omega << "\n\n" << endl;

    for (size_t i = 1; i <= max_iter; i++) {
        y = R * x;
        r = A.product(y) - b;
        resid = norm_2(r) / normb;

        printOut.onIteration(AN_POWER_METHOD, i, resid);
        if (resid <= tol) {
            tol = resid;
            max_iter = i;
            printOut.onConvergence(AN_POWER_METHOD, max_iter, tol);
            x = R * x;
            return 0;
        }

        z = L * r;
        for (size_t j = 0; j < x.size(); j++) {
            if (x(j) < 0) {
                cout << "  x(" << j << ") = " << x(j) << endl;
                throw program_exception("negative x(j)!!");
            }
            double next_xj = x(j) + z(j) * omega;
            if (next_xj < 0) {
                cout << "  next_x(" << j << ") = " << next_xj << endl;
                throw program_exception("negative next_x(j)!!");
                //next_xj = randAB(x(j), x(j)*0.9);
            }
            x(j) = next_xj;
        }
        //noalias(x) += z * omega;

        // cout << "|x| = " << norm_2(x) << ", resid=" << resid << ", |r| = " << norm_2(r) << ", |y| = " << norm_2(y) << endl << endl;

        /*cout << "y("<<i<<") = " << print_vec(y) << "  " << norm_1(y) << endl;
         cout << "r("<<i<<") = " << print_vec(r) << "  " << norm_1(r) << endl;
         cout << "z("<<i<<") = " << print_vec(z) << "  " << norm_1(z) << endl;
         cout << "x("<<i<<") = " << print_vec(x) << "  " << norm_1(x) << endl;*/

        //TODO: with the following line uncommented, DSPN-tool does not
        //      resolve properly non-ergodic models, which have |b| != 0 and
        //      a nullspace vector with norm != 1.
        //TODO: x /= norm_inf(x);

        /*cout << "x("<<i<<") = " << print_vec(x) << "  " << norm_1(x) << endl;
         cout << "  resid="<<resid<<"   omega="<<omega<<endl<<endl;*/
    }

    //REMOVE
    //Vector x1 = R.solve(x), x2 = R.doProduct(x);
    //cout << "\nx1 = " << print_vec(x1/vec_sum(x1)) << "\nx2 = " << print_vec(x2/vec_sum(x2)) << endl << endl << endl;

    //x = R.solve(x); // useless since the method does not converge
    tol = resid;
    printOut.onFailure(AN_POWER_METHOD, max_iter, CFR_TOO_MANY_ITERATIONS);
    return 1;
}

// Random preconditioner matrix - just for test purposes
struct Random_Precond : public Preconditioner  {
    Random_Precond(size_t N, size_t samples, double _weight,
                   VerboseLevel verboseLvl) {
        weight = _weight;
        numerical::matrix_map<double> _RM(N, N);

        for (size_t i = 0; i < N; i++) {
            double sum = 1.0;
            for (size_t s = 0; s < samples; s++) {
                size_t j = rand() % N;
                double v_ij = (rand() % 1000) / (samples * 1000.0);
                _RM(i, j) += v_ij;
                sum -= v_ij;
            }
            _RM(i, i) += sum;
        }
        compress_matrix(RM, _RM);

        if (verboseLvl >= VL_VERBOSE && N <= 100) {
            cout << "RANDOM PRECONDITIONER:\n  ";
            print_matrix(cout, RM, "R");
            cout << endl << endl;
        }
    }

    bool isIdentity() const    {   return false;   }

protected:
    ublas::vector<double> solve(const ublas::vector<double> &inVec) const {
        size_t N = inVec.size();
        ublas::vector<double> outVec(N);
        outVec = prod(inVec, RM) * weight + inVec * (1.0 - weight);
        return outVec;
    }

private:
    ublas::compressed_matrix<double> RM;
    double weight;
};


template<class ResidualFunctor, class Vector, class VectorB,
         class Preconditioner, class PrintOut>
bool Krylov_Iteration(ResidualFunctor &A, Vector &x, const VectorB &b_,
                      const Preconditioner &M, const SolverParams &spar,
                      double &rel_err, size_t &iterCount,
                      PrintOut &printOut, VerboseLevel verboseLvl) {
    const ublas::vector<double> b(b_);
    Vector old_x = x;
    size_t N_MRP = A.size();
    NullPreconditioner nullPrec;

    size_t outIters = spar.maxIters;
    size_t KrylovM = min(spar.KrylovM, N_MRP - 1);
    PIxVproduct<ResidualFunctor> PIxV(A);
    rel_err = spar.epsilon;
    bool isOk;

    // TODO: remove this piece of code
    //cout << "BUILDING RANDOM PRECONDITIONER.... [[REMOVE THIS CODE]]" << endl;
    //Random_Precond rndPrec(N_MRP, 4, 0.1, spar, VL_VERBOSE);

    switch (spar.implicitAlgo) {
    case ILA_GMRES_M: {
        ublas::matrix<double> Hessenberg(KrylovM + 1, N_MRP);
        isOk = (0 == GMRES(PIxV, x, b, nullPrec, M, Hessenberg, KrylovM,
                           true, outIters, rel_err, printOut));
        break;
    }

    case ILA_BICGSTAB:
        isOk = (0 == BiCGSTAB(PIxV, x, b, M, outIters,
                              rel_err, printOut));
        break;

    case ILA_CGS:
        isOk = (0 == CGS(PIxV, x, b, M, outIters, rel_err, printOut));
        break;

    case ILA_POWER_METHOD:
        isOk = (0 == IR2(PIxV, x, b, nullPrec, M, outIters,
                         rel_err, spar.omega, printOut));
        break;

    default:
        throw program_exception("Wrong algorithm index.");
    }

    if (isOk) { // Successful convergence
        iterCount += outIters;
        return true;
    }
    else {
        x = old_x;
        rel_err = 1;
        return false;
    }
}

//-----------------------------------------------------------------------------

static inline double rand01() {
    return double(rand()) / RAND_MAX;
}

static inline double rand01int() {
    const double epsilon = 0.001;
    return epsilon + (double(rand()) / (RAND_MAX - 2 * epsilon));
}

static inline double expDuration(double rate, double randomValue) {
    return (-log(randomValue) / rate);
}

static inline
bool SelectNextState(const ublas::compressed_matrix<double> &M,
                     const marking_t i, marking_t &next_i,
                     double &acc, double randomValue) {
    matRow_t M_iRow(M, i);
    for (rowIter_t M_ij = M_iRow.begin(); M_ij != M_iRow.end(); ++M_ij)	{
        if (*M_ij <= 0)
            continue; // Skip the diagonal element of Q
        acc += *M_ij;
        next_i = M_ij.index();
        if (acc >= randomValue)
            return true;
    }
    return false;
}


static inline
marking_t SimulateSmc(const MRP &mrp, const MRPStateKinds &stKinds,
                      const marking_t initState, double simTime) {
    const double epsilon = 0.9999;
    double t = 0.0;
    marking_t i = initState;
    bool sel;

    //cout << "\n\nSTART SIM: " << int(i)+1 << "  t=" << t << " upto " << simTime << endl;

    while (true) {
        if (stKinds.kinds[i] & MSK_EMC_ABSORBING)
            break;

        // Choose the next state in Q + Qbar
        double mQ_ij = -mrp.Q(i, i);
        if (mQ_ij == 0.0)
            break; // No exponential transition, waiting for the Delta arc
        double rndNum = rand01() * mQ_ij * epsilon;
        double acc = 0.0;
        marking_t next_i(size_t(-1));

        // Time of the next exponential firing from state i
        t += expDuration(mQ_ij, rand01int());
        if (t > simTime)
            break; // Transition would fire after the deterministic

        // Determine which transition will be followed
        if (SelectNextState(mrp.Q, i, next_i, acc, rndNum)) {
            i = next_i;
            //cout << "       [Q]: " << int(next_i)+1 << "  t=" << t << endl;
            continue;  // Following a Q transition
        }

        sel = SelectNextState(mrp.Qbar, i, next_i, acc, rndNum);
        assert(sel);
        UNUSED_PARAM(sel);

        // After a preemptive transition fires, exit the SMC simulation
        //cout << "   [Qbar]: " << int(next_i)+1 << "  PREEMPT" << endl;
        return next_i;
    }

    // Choose the destination state with a Delta transition from state i
    double acc = 0.0;
    sel = SelectNextState(mrp.Delta, i, i, acc, rand01() * epsilon);
    assert(sel);
    //cout << "   [Delta]: " << int(i)+1 << endl;
    return i;
}

typedef map<marking_t, size_t>  frequency_map;

inline bool
comp_max_freq(const pair<marking_t, size_t> &it1, const pair<marking_t, size_t> &it2)
{	return it1.second < it2.second;   }



void BuildSimulatedEMC(const MRP &mrp, const SolverParams &spar,
                       const MRPStateKinds &stKinds,
                       ublas::compressed_matrix<double> &outM,
                       VerboseLevel verboseLvl) {
    const marking_t N_MRP(mrp.N());
    numerical::matrix_map<double> M(N_MRP, N_MRP);
    long_interval_timer timer;
    performance_timer perfTimer;
    srand(spar.rndSeed); // TODO: use MT19937

    for (marking_t i(0); i < N_MRP; ++i) {
        if (stKinds.kinds[i] & MSK_EMC_NOT_EMBEDDED)
            continue;
        if (verboseLvl >= VL_BASIC && timer) {
            timer.prepare_print();
            cout << "SIMULATIONG ROW " << i + marking_t(1) << "/" << N_MRP + marking_t(1) << "   ";
            if (timer.get_elapsed_time() > 2) {
                timer.print_estimate(size_t(i) + 1, size_t(N_MRP) + 1);
                cout << " remained...";
            }
            cout << "             " << endl;
        }
        statesubset_t rowSubset = mrp.Subsets.subsetOf(i);
        if (rowSubset == EXP_SUBSET) {
            if (stKinds.kinds[i] & MSK_EMC_ABSORBING) {
                /*if (!spar.ipaIsCtmc)
                	M(i, i) = 1.0;*/
            }
            else {
                // Copy the embedded DTMC i
                matRow_t Q_iRow(mrp.Q, i);
                double mQ_ii = - mrp.Q(i, i);
                for (rowIter_t Q_ij = Q_iRow.begin(); Q_ij != Q_iRow.end(); ++Q_ij) {
                    const marking_t j(Q_ij.index());
                    if (i != j)
                        M(i, j) = *Q_ij / mQ_ii;
                }
                M(i, i) += (spar.ipaPrecSum);
                // TODO: decomment this i to activate the "right" preconditioner,
                //  which however is worst. Still need some investigation....
                /*if (spar.ipaIsCtmc)
                 M(i, i) -= 1; //*/
            }
        }
        else { // General state: simulate the SMC
            verify(mrp.Gen[rowSubset].detDelay > 0.0); // Must be a deterministic, not a general event
            double detTime = mrp.Gen[rowSubset].detDelay;
            frequency_map freq;
            for (size_t k(0); k < spar.ipaNumSimulations; ++k) {
                // Generate a new sample with a simulation in mrp.Q + mrp.Qbar
                marking_t dst = SimulateSmc(mrp, stKinds, i, detTime);
                freq[dst]++;
            }

            // Now take the samplesPerRow most frequent destination markings
            size_t totFreq = 0;
            for (size_t sample(0); sample < spar.ipaSamplesPerRow; ++sample) {
                frequency_map::iterator it;
                it = max_element(freq.begin(), freq.end(), comp_max_freq);
                if (it == freq.end())
                    break;
                totFreq += it->second;
                M(i, it->first) += double(it->second) / spar.ipaNumSimulations;
                freq.erase(it);
            }

            // Add a self loop with the remaining frequencies
            if (spar.ipaAddSelfLoop)
                M(i, i) += double(spar.ipaNumSimulations - totFreq) / spar.ipaNumSimulations;

            M(i, i) += (spar.ipaPrecSum);

            // OLD but very good preconditioner:
            if (spar.ipaOldPrec)
                M(i, i) -= 1.0;
        }
    }

    compress_matrix(outM, M);

    /*for (marking_t row(0); row < N_MRP; ++row) {
     statesubset_t rowSubset = mrp.Subsets.subsetOf(row);
     cout << setw(4) << right << row << ": ";
     if (rowSubset == EXP_SUBSET)
     cout << "EXP ";
     else
     cout << "GEN ";
     double sum = 0.0;
     matRow_t M_iRow(outM, row);
     for (rowIter_t M_ij = M_iRow.begin(); M_ij != M_iRow.end(); ++M_ij)
     sum += *M_ij;
     cout << sum << endl;
     }//*/

    if (verboseLvl >= VL_BASIC) {
        timer.clear_any();
        cout << "SIMULATED DTMC BUILT." << endl;
        if (verboseLvl >= VL_VERBOSE && N_MRP < marking_t(50)) {
            print_matrix(cout, outM, "M");
            cout << endl << endl;
        }
        cout << "SIMULATED DTMC BUILD TIME: " << perfTimer.get_timer_repr() << endl;
    }
}


#if 0
struct DiagILU_Precond : public Preconditioner  {
    DiagILU_Precond(const MRP &mrp, const MRPStateKinds &stKinds,
                    const SolverParams &_spar, VerboseLevel verboseLvl) : spar(_spar) {
        ublas::compressed_matrix<double> M;
        BuildSimulatedEMC(mrp, spar, stKinds, M, verboseLvl);

        // Build up the D-ILU(0) diagonal preconditioner
        size_t N = M.size1();
        diag.resize(N);
        for (size_t i = 0; i < N; i++)
            diag(i) = M(i, i);
        for (size_t i = 0; i < N; i++) {
            if (diag(i) != 0)
                diag(i) = 1.0 / diag(i);
            for (size_t j = i + 1; j < N; j++) {
                if (M(i, j) != 0 && M(j, i) != 0)
                    diag(j) -= M(j, i) * diag(i) * M(i, j);
            }
        }

        if (verboseLvl >= VL_VERBOSE && N <= 100) {
            cout << "D-ILU0 DIAGONAL:\n  " << print_vec(diag) << endl << endl;
        }
    }

    bool isIdentity() const    {   return false;   }

protected:
    ublas::vector<double> solve(const ublas::vector<double> &inVec) const {
        size_t N = diag.size();
        ublas::vector<double> outVec(N);
        for (size_t i = 0; i < N; i++) {
            if (diag(i) != 0)
                outVec(i) = inVec(i) / diag(i);
            else
                outVec(i) = inVec(i);
        }
        return outVec;
    }

private:
    ublas::vector<double> diag;
    const SolverParams &spar;
};
#endif   // 0


void BuildEmbeddedMatrices(const MRP &mrp, const MRPStateKinds &stKinds,
                           const SolverParams &spar,
                           ublas::compressed_matrix<double> &outP,
                           ublas::compressed_matrix<double> &outC,
                           VerboseLevel verboseLvl);

//-----------------------------------------------------------------------------

struct embedded_state_pred : unary_function <MRPStateKind, bool> {
    inline bool operator()(const MRPStateKind k) const
    {	return (k & MSK_EMC_NOT_EMBEDDED) == 0;   }
};

//-----------------------------------------------------------------------------

void
BuildPreconditionerSourceMatrix(const MRP &mrp, const MRPStateKinds &stKinds,
                                ublas::compressed_matrix<double> &EMC,
                                const SolverParams &spar, VerboseLevel verboseLvl) {
    ublas::compressed_matrix<double> C;
    //ublas::compressed_matrix<double> EMC;
    switch (spar.implicitPrecStrategy) {
    case IPS_INNEROUTER_SIM:
    case IPS_PREC_SIM:
        // Build an approximate EMC with simulations
        BuildSimulatedEMC(mrp, spar, stKinds, EMC, verboseLvl);
        break;

    case IPS_INNEROUTER_EMC:
    case IPS_PREC_EMC:
        // Build the real EMC matrix (slow)
        BuildEmbeddedMatrices(mrp, stKinds, spar, EMC, C, verboseLvl);
        break;

    default:
        throw program_exception("wrong IPS index.");
    }
}

//-----------------------------------------------------------------------------

struct InnerOuter_Precond : public Preconditioner {
    InnerOuter_Precond(ublas::compressed_matrix<double> &_cM,
                       const shared_ptr<Preconditioner> _cM_prec,
                       const SolverParams &_spar, VerboseLevel verboseLvl) : spar(_spar) {
        cM.swap(_cM);
        cM_prec = _cM_prec;
        numSol = iterCnt = 0;
        assert(cM.size1() > 0 && cM.size2() > 0);
    }

    ublas::compressed_matrix<double> cM;
    shared_ptr<Preconditioner> cM_prec;
    mutable size_t numSol, iterCnt;
    const SolverParams &spar;


    class PrecResidualFunctor {
    public:
        inline PrecResidualFunctor(const InnerOuter_Precond *_p) : p(_p) { }

        inline size_t size() const		{  return p->cM.size1();  }

        template<class Vector>
        inline ublas::vector<double> product(const Vector &inVec) const {
            const size_t N = size();
            double alpha = p->spar.ipaAlpha;
            //double unifVal = (1.0 - alpha) / N;
            ublas::vector<double> outVec(N);
            full_element_selector elems(N);

            for (size_t i = 0; i < N; i++) {
                if (std::isnan(inVec[i]))
                    throw program_exception("Vector with NaNs sent in input to "
                                            "Inner/Outer preconditioner.");
            }
            prod_vecmat(outVec, inVec, p->cM, elems, elems);
            if (alpha != 0.0) {
                /*for (size_t i=0; i<N; i++)
                	outVec(i) = outVec(i) * alpha + unifVal;*/

                double v0 = outVec(0), vprev = outVec(N - 1);
                for (size_t i = 0; i < N; i++) {
                    double tmp = outVec(i);
                    outVec(i) = outVec(i) * alpha + (1 - alpha) * vprev;
                    vprev = (i == N - 1) ? v0 : tmp;
                }
            }
            outVec -= inVec;
            //cout << "INVEC NORM: " << norm_2(inVec) << "\t   ";
            //cout << "OUTVEC NORM: " << norm_2(inVec) << endl;
            return outVec;
        }

    private:
        const InnerOuter_Precond *p;
    };

    bool isIdentity() const    {   return false;   }

protected:
    ublas::vector<double> solve(const ublas::vector<double> &b) const {
        const size_t N = b.size();
        ublas::vector<double> x(N);

        fill(x.begin(), x.end(), sqrt(N) / N);
        PrecResidualFunctor Mat(this);
        NullPreconditioner nullPrec;
        LinearIterationPrinter printOut(VL_BASIC);
        //NullIterPrintOut printOut;
        size_t max_iter = spar.inoutMaxIters;
        double tol = spar.inoutEpsilon;
        size_t KrylovM = min(size_t(spar.inoutKrylovM), N - 1);
        ublas::matrix<double> Hessenberg(KrylovM + 1, KrylovM + 1);
        //cout << "(B) NORM: " << norm_2(b) << "\t   ";
        //cout << "(X) NORM: " << norm_2(x) << endl << endl;
        //CGS(Mat, x, b, *(cM_prec.get()), max_iter, tol, printOut);
        GMRES(Mat, x, b, *(cM_prec.get()), nullPrec, Hessenberg,
              KrylovM, true, max_iter, tol, printOut);//*/
        //throw program_exception("Internal GMRES does not converge.");
        //IR2(Mat, x, b, prec,  prec, max_iter, tol, spar.omega, printOut);
        //cout << "PRECONDITIONER SOLVE: " << max_iter << " iterations, resid = " << tol<< endl;
        iterCnt += max_iter;
        numSol++;
        if (printOut.getVerboseLevel() >= VL_BASIC)
            cout << "  E[iterCount]=" << double(iterCnt) / numSol << endl << endl;

        /*long_interval_timer timer;
         LinearIterationPrinter printOut(VL_BASIC, spar, LSM_EXPLICIT, &timer);
         ublas::scalar_vector<double> minusOneD(N, -1.0);
         full_element_selector all_elems(N);
         LinearSolveTransposed(x, cM, b, minusOneD, spar,
         printOut, all_elems, all_elems);
         cout << endl;
         //*/


        /*size_t max_iter = 0;
         NullIterPrintOut printOut;
         double alpha = 0.95, tol;
         ublas::scalar_vector<double> unif(N, (1.0 - alpha)/N);
         ublas::vector<double> v2;
         fill(x.begin(), x.end(), 1.0 / N);
         PrecResidualFunctor Mat(this);
         do {
         v2 = Mat.product(x) + x + b;  // TODO: or is it a -b ???
         tol = vec_sum(v2 - x) / vec_sum(v2);
         x.swap(v2);
         max_iter++;
         } while (tol >= 1.0e-10 && max_iter < 1000);
         cout << "PRECONDITIONER SOLVE: " << max_iter << " iterations, resid = " << tol<< endl;
         cout << endl;//*/

        return x;
    }
};


//-----------------------------------------------------------------------------

template<class ResidualFunctor, class Vector, class VectorB>
bool LinearSolveImplicit(ResidualFunctor &A, Vector &x, const VectorB &b,
                         const Preconditioner &M,
                         double &epsilon, const SolverParams &spar,
                         size_t &maxIters, VerboseLevel verboseLvl) {
    const size_t N_MRP = A.size();
    assert(N_MRP == b.size() && x.size() == N_MRP);
    full_element_selector AllMrpElems(N_MRP);
    size_t iterCount = 0;
    double rel_err;
    LinearIterationPrinter printOut(verboseLvl);
    ImplicitLinearSolverAlgo algo = spar.implicitAlgo;

    /*double norm_b = vec_sum(b);
     if (norm_b == 0)
     norm_b = 1.0;*/

    if (verboseLvl >= VL_BASIC)
        cout << ("ABSORB INITIAL PROBABILITY IN NON-RECURRENT "
                 "REGENERATIVE STATES...\n") << endl;

    // Absorb the initial probability in non-regenerative states
    PowerMethodIteration(A, x, b, spar, rel_err,
                         iterCount, printOut, VL_NONE);

    // Iterative steps
    while (iterCount < maxIters) {
        if (!Krylov_Iteration(A, x, b, M, spar, rel_err,
                              iterCount, printOut, verboseLvl)) {
            if (algo != ILA_POWER_METHOD) {
                printOut.onFailure(AN_UNSPECIFIED, iterCount, CFR_BREAKDOWN_SWITCH_TO_POWERMETHOD);
                algo = ILA_POWER_METHOD;
            }
            else return false;
        }

        // Test if the error is small enough
        if (rel_err < epsilon) {
            maxIters = iterCount;
            epsilon = rel_err;
            /*if (algo == ILA_POWER_METHOD) {
             printOut.onConvergence(iterCount, rel_err);
             }*/
            if (printOut.getVerboseLevel() >= VL_BASIC)
                cout << "TOTAL MATRIX-FREE PRODUCTS: " << A.getProductCount() << endl << endl;
            return true;
        }
    }
    /*if (algo == ILA_POWER_METHOD)
     printOut.onFailure(iterCount, CFR_TOO_MANY_ITERATIONS);*/
    return false;
}


//-----------------------------------------------------------------------------

// Prepare an initial guess vector x0 for PowerMethod. x0 starts with a
// uniform distribution across all its (selected) regenerative states.
// Non-regenerative states (or unselected states) are set to 0.
template<class RowSelector>
void InitialGuess(ublas::vector<double> &x0, const MRPStateKinds &sk,
                  const RowSelector &rows) {
    const marking_t N_MRP(sk.kinds.size());
    x0.resize(N_MRP);
    size_t cnt = 0;
    for (size_t r = 0; r < rows.count(); r++)
        if ((sk.kinds[rows[r]] & MSK_EMC_NOT_EMBEDDED) == 0)
            cnt++;
    const double initVal = 1.0 / cnt;
    for (marking_t m(0); m < N_MRP; m++) {
        if (!rows.isSelected(m) || sk.kinds[m] & MSK_EMC_NOT_EMBEDDED)
            x0(m) = 0.0;
        else {
            x0(m) = initVal;
            cnt--;
        }
    }
    assert(cnt == 0);
}

//-----------------------------------------------------------------------------

// Returns a new SubsetList where unselected markings are put in the
// ignored subset
template<class Selector>
void FilterSubsets(const MRPSubsetList &inSubset, MRPSubsetList &outSubset,
                   const Selector &elems) {
    const marking_t N_MRP(inSubset.numStates());
    vector<statesubset_t> stateToSubset(N_MRP);

    for (marking_t m(0); m < N_MRP; ++m) {
        if (elems.isSelected(m))
            stateToSubset[m] = inSubset.subsetOf(m);
        else
            stateToSubset[m] = IGNORED_SUBSET;
    }
    outSubset.ConstructSubsetList(inSubset.subsetCount(), stateToSubset);
}

//-----------------------------------------------------------------------------

shared_ptr<Preconditioner>
BuildEmcPreconditioner(const MRP &mrp, const MRPStateKinds &stKinds,
                       const SolverParams &spar, VerboseLevel verboseLvl) {
    if (verboseLvl >= VL_BASIC) {
        cout << "\nPRECONDITIONING STRATEGY: " << IPS_Names[spar.implicitPrecStrategy] << endl;
        cout << "USING " << PA_Names[spar.precAlgo] << " PRECONDITIONER METHOD." << endl;
    }

    ublas::compressed_matrix<double> A_tilde, srcEMC, projEMC;

    // Choose the preconditioner according to the implicit strategy
    if (spar.implicitPrecStrategy == IPS_NO_PRECONDITIONER)
        return make_shared<NullPreconditioner>();

    // Build up the source preconditioner matrix
    BuildPreconditionerSourceMatrix(mrp, stKinds, srcEMC, spar, verboseLvl);

    subset_selector<vector<MRPStateKind>, embedded_state_pred> emb_state_sel(stKinds.kinds);
    project_matrix(srcEMC, projEMC, emb_state_sel, emb_state_sel);

    // P is projEMC;  A_tilde = -(P - I)^T
    const size_t N_EMC(stKinds.numRegenStates), N_MRP(mrp.N());
    ublas::identity_matrix<double> I(N_EMC);
    FastTranspose(projEMC, A_tilde);
    A_tilde *= -1;
    A_tilde += I;

    /*/ TODO: remove this debug code
    typedef ublas::matrix_row<ublas::compressed_matrix<double> > matrix_row_t;
    typedef matrix_row_t::const_iterator matrix_row_iter_t;
    for (size_t r=0; r<projEMC.size1(); r++) {
    	matrix_row_t rth_col(projEMC, r);
    	double sum = 0;
    	matrix_row_iter_t it_r(rth_col.begin()), endIt_r(rth_col.end());
    	for (; it_r != endIt_r; ++it_r) {
    		sum += *it_r;
    	}
    	cout << "ROW " << r << " HAS SUM " << sum << endl;
    }*/


    // Create the preconditioner for the Inner system matrix
    shared_ptr<Preconditioner> pInnerPrec;
    double det = 0;
    if (verboseLvl >= VL_BASIC && spar.precAlgo != PA_NULL_PRECONDITIONER)
        cout << "INTERNAL PRECONDITIONER IS: " << PA_Names[spar.precAlgo] << endl;
    switch (spar.precAlgo) {
    case PA_NULL_PRECONDITIONER:
        pInnerPrec = make_shared<NullPreconditioner>();
        break;

    case PA_ILU0:
        det = ILU0(A_tilde, verboseLvl);
        if (verboseLvl >= VL_BASIC && abs(det) < 1.e-6)
            cout << "ILU0 DETERMINANT OF " << det << " VERY CLOSE TO ZERO." << endl;

        if (emb_state_sel.count() != N_MRP) {
            ublas::compressed_matrix<double> unproj_A_tilde;
            unproject_matrix(A_tilde, unproj_A_tilde, N_MRP, N_MRP,
                             emb_state_sel, emb_state_sel);
            unproj_A_tilde.swap(A_tilde);
        }
        pInnerPrec = make_shared<ILU_Precond>(A_tilde);
        break;

    case PA_ILUTK:
        ILUTK(A_tilde, A_tilde, spar.ilutkThreshold,
              spar.ilutkMaxNumEntries, verboseLvl);

        if (emb_state_sel.count() != N_MRP) {
            ublas::compressed_matrix<double> unproj_A_tilde;
            unproject_matrix(A_tilde, unproj_A_tilde, N_MRP, N_MRP,
                             emb_state_sel, emb_state_sel);
            unproj_A_tilde.swap(A_tilde);
        }
        /*{
        	print_matrix(cout, A_tilde, "A");
        	cout << "size1: " << A_tilde.size1() << endl;
        	cout << "size2: " << A_tilde.size2() << endl;
        	cout << "index1_data: " << print_vec(A_tilde.index1_data()) << endl;
        	cout << "index2_data: " << print_vec(A_tilde.index2_data()) << endl;
        	cout << "value_data: " << print_vec(A_tilde.value_data()) << endl;
        	cout << "index_base: " << A_tilde.index_base() << endl;
        	cout << "filled1: " << A_tilde.filled1() << endl;
        	cout << "filled2: " << A_tilde.filled2() << endl;
        	cout << "\n\n";

        	typedef typename ublas::matrix_row<const ublas::compressed_matrix<double> >  matrix_row_t;
        	typedef typename matrix_row_t::const_iterator  matrix_row_iterator_t;
        	for (size_t i = 0; i < A_tilde.size1(); i++) {
        		matrix_row_t ith_row(A_tilde, i);
        		matrix_row_iterator_t A_ij(ith_row.begin()), A_iEnd(ith_row.end());
        		cout << "ROW " << i << ": ";
        		for (; A_ij != A_iEnd; ++A_ij) {
        			cout << "v["<<A_ij.index()<<"]="<<*A_ij<<"  ";
        		}
        		cout << endl;
        	}
        	cout << endl;
        }*/
        pInnerPrec = make_shared<ILU_Precond>(A_tilde);
        break;

    case PA_DIAG:
        pInnerPrec = make_shared<Diag_Precond>(A_tilde);
        break;

    default:
        throw program_exception("wrong preconditioner index.");
    }
    /*unproject_matrix(projEMC, precMatrix, N_MRP, N_MRP,
    				 emb_state_sel, emb_state_sel);
    if (spar.precAlgo == PA_ILU0 || spar.precAlgo == PA_ILUTK)
    	pInnerPrec = make_shared<ILU_Precond>(projEMC);*/

    assert(pInnerPrec != nullptr);
    if (IPS_isInnerOuter[spar.implicitPrecStrategy]) {
        shared_ptr<Preconditioner> p;
        p.reset(new InnerOuter_Precond(srcEMC, pInnerPrec, spar, verboseLvl));
        return p;
    }
    else
        return pInnerPrec;
}

//-----------------------------------------------------------------------------

template<class VectorB, class RowSelector, class ColSelector>
bool Solve(const MRP &mrp, const MRPStateKinds &stKinds,
           const SolverParams &spar, KolmogorovEquationDirection ked, const VectorB &B,
           ublas::vector<double> &emcSol, ublas::vector<double> *optMrpSol,
           SteadyStateComputation whatToDo, VerboseLevel verboseLvl,
           const shared_ptr<Preconditioner> rightPrec,
           const RowSelector &rows, const ColSelector &cols) {
    const marking_t N_MRP(mrp.N()), N_ROWS(rows.count()), N_COLS(cols.count());
    UNUSED_PARAM(N_ROWS);
    UNUSED_PARAM(N_COLS);

    if (rows.count() == 1 &&
            count_matrix_row_nonzero_values(mrp.Q, rows[0]) == 0 &&
            count_matrix_row_nonzero_values(mrp.Qbar, rows[0]) == 0 &&
            count_matrix_row_nonzero_values(mrp.Delta, rows[0]) == 0) {
        emcSol(rows[0]) = 1.0 - B(rows[0]);
        if (optMrpSol)
            (*optMrpSol)(rows[0]) = 1.0 - B(rows[0]);
        return true;
    }

    // Rebuild the MRPSubsetList if there is a selection of rows.
    // In any case we will use the subsets pointed by *pSubsets
    const MRPSubsetList	*pSubsets;
    MRPSubsetList filteredSubsets;
    if (rows.count() == N_MRP)
        pSubsets = &mrp.Subsets;
    else {
        FilterSubsets(mrp.Subsets, filteredSubsets, rows);
        pSubsets = &filteredSubsets;
    }


    size_t outIters = spar.maxIters;
    double outTol = spar.epsilon;

    typedef EmcVectorMatrixProduct<RowSelector, ColSelector> VecMatProd_t;
    VecMatProd_t ImplicitEmcProd(mrp, spar, *pSubsets, ked, rows, cols);
    InitialGuess(emcSol, stKinds, rows);

    // STEP 1) Solve the linear equation system in implicit form
    if (verboseLvl >= VL_BASIC)
        cout << "COMPUTING STEADY STATE SOLUTION OF THE EMBEDDED DTMC...\n" << endl;
    if (!LinearSolveImplicit(ImplicitEmcProd, emcSol, B, *rightPrec,
                             outTol, spar, outIters, verboseLvl))
        throw program_exception("Didn't converge.");

    if (ked == KED_BACKWARD) {
        for (marking_t i(0); i < N_MRP; ++i)
            if (emcSol[i] < spar.epsilon * 0.001)
                emcSol[i] = 0.0;
    }

    if (whatToDo == SSC_ONLY_UNNORMALIZED_EMC_PROB)
        return true;

    assert(ked == KED_FORWARD);
    emcSol /= vec_sum(emcSol);

    // STEP 2) Convert the EMC solution into the MRP solution
    //         Compute a(M) and b(M) for the EMC solution vector u(M)
    if (verboseLvl >= VL_BASIC)
        cout << "CONVERSION OF THE EMC SOLUTION INTO THE MRP SOLUTION...\n" << endl;

    //full_element_selector AllMrpElems(N);
    NullIterPrintOut printOut;
    MRPSubsetList::selector ExpStates(pSubsets->selectorOf(EXP_SUBSET));
    vector<GenMarkingSubset>::const_iterator SgenIt;
    ublas::vector<double> aM(N_MRP), bM(N_MRP), ag, bg;
    std::fill(aM.begin(), aM.end(), 0.0);
    std::fill(bM.begin(), bM.end(), 0.0);
    for (SgenIt = mrp.Gen.begin(); SgenIt != mrp.Gen.end(); ++SgenIt) {
        if (pSubsets->numStatesIn(SgenIt->index) == marking_t(0))
            continue;
        MRPSubsetList::selector GenStates(pSubsets->selectorOf(SgenIt->index));
        // TODO: remove this
        /*if (GenStates.count() == 0)
         continue;*/
        UniformizationGen(mrp.Q, emcSol, ag, &bg, SgenIt->fg.c_str(), spar.epsilon,
                          KED_FORWARD, GenStates, cols, printOut);
        add_vec(aM, ag, GenStates);
        add_vec(bM, bg, GenStates);
    }

    // Normalization constant:  c = u(M).C.e = -u(M)^E.diag^-1(Q^E).e + b(M).e
    double c = 0.0;
    for (size_t r = 0; r < ExpStates.count(); r++) {
        const size_t i = ExpStates[r];
        if (mrp.Q(i, i) != 0.0)
            c -= emcSol(i) / mrp.Q(i, i);
    }
    c += vec_sum(bM);

    // Avoid zero conversion factors (this happens with absorbing states)
    if (c == 0.0)
        c = 1.0;

    // MRP state probabilities: pi = v.C = -(1/c)u(M)^E.diag^-1(Q^E) + (1/c)b(M)
    optMrpSol->resize(N_MRP);
    std::fill(optMrpSol->begin(), optMrpSol->end(), 0.0);
    for (marking_t m(0); m < N_MRP; ++m) {
        if (mrp.Subsets.subsetOf(m) == EXP_SUBSET) {
            if (mrp.Q(m, m) != 0.0)
                (*optMrpSol)(m) = -emcSol(m) / (c * mrp.Q(m, m));
            else
                (*optMrpSol)(m) = emcSol(m);  // Absorbing EXP state
        }
        else // Deterministic state
            (*optMrpSol)(m) = bM(m) / c;
    }

    // Firing frequencies:  Phi = v.Psi = (1/c)a(M)

    if (verboseLvl >= VL_VERBOSE) {
        cout << "CONVERSION FACTOR: " << c << endl;
        cout << "EMC SOLUTION: " << emcSol << endl;
        cout << "MRP SOLUTION: " << (*optMrpSol) << "   ";
        cout << vec_sum(*optMrpSol) << endl;
        cout << endl;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool SteadyStateMRPImplicit(const MRP &mrp, const MRPStateKinds &stKinds,
                            PetriNetSolution &sol, const SolverParams &spar,
                            VerboseLevel verboseLvl) {
    const marking_t N_MRP(mrp.N());
    BSCC bscc;
    full_element_selector FullMrpSelector(N_MRP);
    ublas::zero_vector<double> zeroB(N_MRP);

    // Generate the right preconditioner matrix for the EMC
    shared_ptr<Preconditioner> rightPrec(BuildEmcPreconditioner(mrp, stKinds, spar, verboseLvl));

    BottomStronglyConnectedComponents(mrp.Q + mrp.Qbar + mrp.Delta, bscc);
    if (bscc.numBscc == sccindex_t(0)) {
        // The MRP is ergodic, solve it directly
        if (verboseLvl >= VL_BASIC)
            cout << "STEADY STATE SOLUTION OF ERGODIC MRP." << endl;
        ublas::vector<double> emcSol(N_MRP, 0.0);
        return Solve(mrp, stKinds, spar, KED_FORWARD, zeroB, emcSol,
                     &sol.stateProbs, SSC_COMPUTE_ALL, verboseLvl,
                     rightPrec, FullMrpSelector, FullMrpSelector);
    }
    if (verboseLvl >= VL_BASIC) {
        cout << "STEADY STATE SOLUTION OF NON-ERGODIC MRP WITH ";
        cout << bscc.numBscc << " RECURRENCE CLASSES." << endl;
    }

    // sigma0 = mean sojourn time in the transient states until absorption
    ublas::vector<double> sigma0(N_MRP, 0.0), piAbs;
    subset_element_selector<sccindex_t> InitTrnElems(&bscc.indices, -1);
    if (InitTrnElems.count() > 0) {
        // Probability in the initial transient. This requires the solution
        // of the equation system:  -pi0(T) = sigma0 * ( P_{T,T} - I_T )
        double transient_prob = vec_sum(mrp.pi0, InitTrnElems);
        if (transient_prob > 0) {
            typedef subset_element_selector<sccindex_t, not_equal_to<sccindex_t> > inv_subset_sel;
            not_equal_to<sccindex_t> comparator;
            inv_subset_sel RecurrentElems(&bscc.indices, TRANSIENT_COMPONENT, comparator);

            // Solve the initial transient sub-matrix.
            // The solution of this sub-matrix is simply given by the solution of
            // the filtered generators Q, Qbar and Delta.
            if (!Solve(mrp, stKinds, spar, KED_FORWARD, -mrp.pi0, sigma0, nullptr,
                       SSC_ONLY_UNNORMALIZED_EMC_PROB, verboseLvl,
                       rightPrec, InitTrnElems, InitTrnElems))
                return false;
            // BugFix: zero the recurrent entries of sigma0 (could be non-zeros)
            set_vec(sigma0, 0.0, RecurrentElems);
            
            // Compute how the initial transient probability enters the
            // numBscc recurrent classes. This requires a single product
            // with the rectangular EMC sub-matrix that contains the
            // transitions from the initial transient to the recurrent classes.

            typedef	EmcVectorMatrixProduct<full_element_selector,
                    /**/				   full_element_selector> EmcProd;
            EmcProd sigma0xP0i(mrp, spar, mrp.Subsets, KED_FORWARD,
                               FullMrpSelector, FullMrpSelector);
            piAbs = sigma0xP0i.product(sigma0);
            set_vec(piAbs, 0.0, InitTrnElems);

            // Add to piAbs the probability of starting in a recurrent class
            add_vec(piAbs, mrp.pi0, RecurrentElems);
            //cout << "piAbs = " << piAbs << endl;
            piAbs /= vec_sum(piAbs);
        }
        else piAbs = mrp.pi0;
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "STEADY STATE SOLUTION OF " << bscc.numBscc;
        cout << " RECURRENCE CLASSES..." << endl;
    }

    // Get the steady-state probabilities of entering in each BSCC
    typedef BaseSubsetList<SccIndexTo0BasedIndexWithTransients,
            sccindex_t, size_t> SCCSubsetList2;
    SCCSubsetList2 sccSubsets;
    std::vector<sccindex_t> bsccIndices(bscc.indices);
    sccSubsets.ConstructSubsetList(size_t(bscc.numBscc) + 1, bsccIndices);
    sol.stateProbs.resize(N_MRP);
    std::fill(sol.stateProbs.begin(), sol.stateProbs.end(), 0.0);
    // long_interval_timer bsccStatusTimer;
    for (sccindex_t nc(0); nc < bscc.numBscc; nc++) {
        const SCCSubsetList2::selector RecurrentClassElems = sccSubsets.selectorOf(int(nc));
        //subset_element_selector<sccindex_t> RecurrentClassElems(&bscc.indices, nc);
        // Get the total probability of this recurrent class
        double probBscc = vec_sum(piAbs, RecurrentClassElems);
        if (verboseLvl >= VL_VERBOSE)
            cout << "  Pr[BSCC " << (int(nc) + 1) << "] = " << probBscc << endl;

        if (RecurrentClassElems.count() == 1) {
            // Special case for absorbing states.
            const size_t i = RecurrentClassElems[0];
            sol.stateProbs[i] += probBscc;
        }
        else {
            // Get the steady-state solution of the bscc sub-matrix
            ublas::vector<double> rcEmcSol(N_MRP, 0.0), rcMrpSol(N_MRP, 0.0);
            if (!Solve(mrp, stKinds, spar, KED_FORWARD, zeroB, rcEmcSol,
                       &rcMrpSol, SSC_COMPUTE_ALL, VL_NONE,
                       rightPrec, RecurrentClassElems, FullMrpSelector)) 
                return false;

            // Normalize with the total bscc probability
            double norm = vec_sum(rcMrpSol, RecurrentClassElems);
            norm = probBscc / norm;

            // Set the solution vector sol[]
            sol.stateProbs += rcMrpSol * norm;
        }
        // if (verboseLvl >= VL_BASIC && bsccStatusTimer) {
        //     bsccStatusTimer.prepare_print();
        //     cout << "COMPUTING SOLUTION OF RECURRENCE CLASS " << int(nc) + 1;
        //     cout << "/" << bscc.numBscc << "...      ";
        //     //cout << "WITH " << RecurrentClassElems.count() << " STATES.";
        //     cout << endl;
        // }
    }
    if (verboseLvl >= VL_BASIC) {
        // bsccStatusTimer.clear_any();
        cout << "ALL " << bscc.numBscc << " RECURRENCE CLASSES ";
        cout << "HAVE BEEN SOLVED." << endl;
    }

    sol.stateProbs /= vec_sum(sol.stateProbs);

    // TODO: add support to firing frequencies
    sol.firingFreq.resize(N_MRP);
    std::fill(sol.firingFreq.begin(), sol.firingFreq.end(), -1.0);

    if (verboseLvl >= VL_VERBOSE) {
        //cout << "EMC SOLUTION: " << emcSol << endl;
        cout << "MRP SOLUTION: " << sol.stateProbs << "   ";
        cout << vec_sum(sol.stateProbs) << endl;
        cout << endl;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool SteadyStateMRPImplicitBackward(const MRP &mrp, const MRPStateKinds &stKinds,
                                    const ublas::vector<double> &rho,
                                    ublas::vector<double> &xi,
                                    const SolverParams &spar, VerboseLevel verboseLvl) {
    const marking_t N_MRP(mrp.N());
    BSCC bscc;
    full_element_selector FullMrpSelector(N_MRP);
    ublas::zero_vector<double> zeroB(N_MRP);
    xi.resize(N_MRP);

    // Generate the right preconditioner matrix for the EMC
    shared_ptr<Preconditioner> rightPrec(BuildEmcPreconditioner(mrp, stKinds, spar, verboseLvl));

    BottomStronglyConnectedComponents(mrp.Q + mrp.Qbar + mrp.Delta, bscc);
    if (bscc.numBscc == sccindex_t(0)) {
        // The MRP is ergodic, solve it directly
        if (verboseLvl >= VL_BASIC)
            cout << "BACKWARD STEADY STATE SOLUTION OF ERGODIC MRP." << endl;
        ublas::vector<double> pi(N_MRP, 0.0);
        if (!Solve(mrp, stKinds, spar, KED_BACKWARD, zeroB, pi,
                   nullptr, SSC_ONLY_UNNORMALIZED_EMC_PROB, verboseLvl,
                   rightPrec, FullMrpSelector, FullMrpSelector))
            return false;
        double uniform_xi_value = vec_dot(pi, rho, FullMrpSelector);
        set_vec(xi, uniform_xi_value, FullMrpSelector);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "BACKWARD MRP SOLUTION: " << xi << "   ";
            cout << vec_sum(xi) << endl;
            cout << endl;
        }
        return true;
    }
    if (verboseLvl >= VL_BASIC) {
        cout << "BACKWARD STEADY STATE SOLUTION OF NON-ERGODIC MRP WITH ";
        cout << bscc.numBscc << " RECURRENCE CLASSES." << endl;
    }

    // Solve each recurrent class whose |rho| mass is nonzero
    if (verboseLvl >= VL_BASIC) {
        cout << "STEADY STATE SOLUTION OF " << bscc.numBscc;
        cout << " RECURRENCE CLASSES..." << endl;
    }

    // Get the steady-state probabilities of entering in each BSCC
    typedef BaseSubsetList<SccIndexTo0BasedIndexWithTransients,
            sccindex_t, size_t> SCCSubsetList2;
    SCCSubsetList2 sccSubsets;
    std::vector<sccindex_t> bsccIndices(bscc.indices);
    sccSubsets.ConstructSubsetList(size_t(bscc.numBscc) + 1, bsccIndices);
    long_interval_timer bsccStatusTimer;
    for (sccindex_t nc(0); nc < bscc.numBscc; nc++) {
        const SCCSubsetList2::selector RecurrentClassElems = sccSubsets.selectorOf(int(nc));

        // Get the total probability of this recurrent class
        double rhoBscc = vec_sum(rho, RecurrentClassElems);
        if (rhoBscc == 0.0) {
            set_vec(xi, 0.0, RecurrentClassElems);
            continue;
        }
        /*if (verboseLvl >= VL_VERBOSE)
        	cout << "  Pr[BSCC "<<(int(nc)+1)<<"] = " << probBscc << endl;*/

        if (RecurrentClassElems.count() == 1) {
            // Special case for absorbing states.
            const size_t i = RecurrentClassElems[0];
            xi[i] = rho[i];
            continue;
        }

        // Get the steady-state solution of the bscc sub-matrix
        ublas::vector<double> rcPiSol(N_MRP, 0.0);
        if (!Solve(mrp, stKinds, spar, KED_BACKWARD, zeroB, rcPiSol,
                   nullptr, SSC_ONLY_UNNORMALIZED_EMC_PROB, VL_NONE,
                   rightPrec, RecurrentClassElems, FullMrpSelector))
            return false;

        double uniform_xi_value = vec_dot(rcPiSol, rho, RecurrentClassElems);
        set_vec(xi, uniform_xi_value, RecurrentClassElems);

        if (verboseLvl >= VL_BASIC && bsccStatusTimer) {
            bsccStatusTimer.prepare_print();
            cout << "COMPUTING SOLUTION OF RECURRENCE CLASS " << int(nc) + 1;
            cout << "/" << bscc.numBscc << "...      ";
            //cout << "WITH " << RecurrentClassElems.count() << " STATES.";
            cout << endl;
        }
    }
    if (verboseLvl >= VL_BASIC) {
        bsccStatusTimer.clear_any();
        cout << "ALL " << bscc.numBscc << " RECURRENCE CLASSES ";
        cout << "HAVE BEEN SOLVED." << endl;
    }

    ublas::vector<double> xiTrn;
    subset_element_selector<sccindex_t> InitTrnElems(&bscc.indices, -1);
    if (InitTrnElems.count() > 0) {
        // Compute one backward product with the entering rectangular matrix
        typedef subset_element_selector<sccindex_t, not_equal_to<sccindex_t> > inv_subset_sel;
        not_equal_to<sccindex_t> comparator;
        inv_subset_sel RecurrentElems(&bscc.indices, TRANSIENT_COMPONENT, comparator);

        typedef	EmcVectorMatrixProduct<full_element_selector,
                /**/						   full_element_selector> EmcProd;
        EmcProd P0i_x_rcXi(mrp, spar, mrp.Subsets, KED_BACKWARD,
                           FullMrpSelector, FullMrpSelector);
        xiTrn = P0i_x_rcXi.product(xi);
        //copy_vec(xi, xiTrn, InitTrnElems);

        //cout << "xiTrn = " << xiTrn << endl;

        // Solve the initial transient system:
        //    x * (I - P_{0,0}) = xiTrn
        double transient_xi = vec_sum(xiTrn, InitTrnElems);
        if (transient_xi > 0) {
            ublas::vector<double> x;
            if (!Solve(mrp, stKinds, spar, KED_BACKWARD, -xiTrn, x, nullptr,
                       SSC_ONLY_UNNORMALIZED_EMC_PROB, verboseLvl,
                       rightPrec, InitTrnElems, InitTrnElems))
                return false;
            copy_vec(xi, x, InitTrnElems);
        }
        else
            set_vec(xi, 0, InitTrnElems);
    }

    // TODO: add support to firing frequencies

    if (verboseLvl >= VL_VERBOSE) {
        //cout << "EMC SOLUTION: " << emcSol << endl;
        cout << "BACKWARD MRP SOLUTION: " << xi << "   ";
        cout << vec_sum(xi) << endl;
        cout << endl;
    }

    return true;
}

//=============================================================================





