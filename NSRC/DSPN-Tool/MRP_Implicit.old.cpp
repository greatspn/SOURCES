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
using namespace std;

#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>
using namespace boost;

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
	PxVproduct& emcProd;	// Implicit product with the DTMC P
public:
	inline PIxVproduct(PxVproduct& _emcProd)
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
double RelativeError(const VectorX &x, const VectorX &next_x, const VectorB& b)
{
	const size_t N = x.size();
	assert(next_x.size() == N && b.size() == N);
	double rel_err = 0.0;
	
	for (size_t i=0; i<N; i++) {
		rel_err += abs(next_x(i) - x(i)/* - b(i)*/); // TODO: why does b(i) interfere?
	}
	rel_err /= vec_sum(next_x);
	assert(!isnan(rel_err));
	return rel_err;
}

//-----------------------------------------------------------------------------

template<class ResidualFunctor, class Vector, class VectorB, class PrintOut>
void PowerMethodIteration(ResidualFunctor &A, Vector &x, const VectorB &b, 
						  const SolverParams& spar, double &rel_err,
						  size_t &iterCount, PrintOut& printOut,
						  VerboseLevel verboseLvl)
{
	Vector next_x;
	
	// Power method iteration:  x(k+1) = A.x(k) - b
	next_x = A.product(x);
	next_x -= b;
	iterCount++;
	
	// Apply over-relaxation
	if (spar.omega != 1.0)
		noalias(next_x) = (spar.omega)*next_x + (1.0 - spar.omega)*x;
	
	rel_err = RelativeError(x, next_x, b);
	printOut.onIteration(iterCount, rel_err);
	x.swap(next_x);
}

//-----------------------------------------------------------------------------

inline double randAB(double A, double B) {
	return A + (B-A) * (rand() / double(RAND_MAX));
}

template < class ResidualFunctor, class Vector, class VectorB,
class Real, class PrintOut >
int
IR2(const ResidualFunctor &A, Vector &x, const VectorB &b,
	const Preconditioner &L, const Preconditioner& R,
	size_t &max_iter, Real &tol, Real omega, PrintOut& printOut)
{
	// A.x = b  =>  L.A.R.R^-1.x = L.b
	// (L.A.R - I).x + x = L.b
	// x(k+1) = x(k) + omega * L.(b - A.R.x(k))
	Real resid = 1;
	Vector z, y, r;
	
	Real normb = norm_2(b);
	if (less_than_machine_epsilon(normb * tol))
		normb = 1;
	//cout << "NORM[b] = " << normb << ",  omega = " << omega << "\n\n" << endl;
	
	for (size_t i = 1; i <= max_iter; i++)
	{
		y = R * x;
		r = A.product(y) - b;
		resid = norm_2(r) / normb;
		
		printOut.onIteration(i, resid);
		if (resid <= tol) {
			tol = resid;
			max_iter = i;
			printOut.onConvergence(max_iter, tol);
			x = R * x;
			return 0;
		}
		
		z = L * r;
		for (size_t j=0; j<x.size(); j++) {
			if (x(j) < 0) {
				cout << "  x("<<j<<") = "<<x(j)<<endl;
				throw program_exception("negative x(j)!!");
			}
			double next_xj = x(j) + z(j) * omega;
			if (next_xj < 0) {
				cout << "  next_x("<<j<<") = "<<next_xj<<endl;
				throw program_exception("negative next_x(j)!!");
				//next_xj = randAB(x(j), x(j)*0.9);
			}
			x(j) = next_xj;
		}
		//noalias(x) += z * omega;
		
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
	printOut.onFailure(max_iter, CFR_TOO_MANY_ITERATIONS);
	return 1;
}

// Random preconditioner matrix - just for test purposes
struct Random_Precond : public Preconditioner  {
	Random_Precond(size_t N, size_t samples, double _weight,
			       const SolverParams& _spar, VerboseLevel verboseLvl) : spar(_spar)
	{
		weight = _weight;
		numerical::matrix_map<double> _RM(N, N);

		for (size_t i=0; i<N; i++) {
			double sum = 1.0;
			for (size_t s=0; s<samples; s++) {
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
	ublas::vector<double> solve(const ublas::vector<double>& inVec) const
	{
		size_t N = inVec.size();
		ublas::vector<double> outVec(N);
		outVec = prod(inVec, RM) * weight + inVec * (1.0 - weight);
		return outVec;
	}

private:
	ublas::compressed_matrix<double> RM;
	const SolverParams& spar;
	double weight;
};


template<class ResidualFunctor, class Vector, class VectorB,
class Preconditioner, class PrintOut>
bool Krylov_Iteration(ResidualFunctor &A, Vector &x, const VectorB &b_, 
					  const Preconditioner &M, const SolverParams& spar,
					  double &rel_err, size_t &iterCount,
					  PrintOut& printOut, VerboseLevel verboseLvl)
{
	const ublas::vector<double> b(b_);
	Vector old_x = x;
	size_t N_MRP = A.size();
	NullPreconditioner nullPrec;
	
	size_t outIters = spar.maxIters;
	size_t KrylovM = min(spar.KrylovM, N_MRP-1);
	PIxVproduct<ResidualFunctor> PIxV(A);
	rel_err = spar.epsilon;
	bool isOk;
	
	// TODO: remove this piece of code
	//cout << "BUILDING RANDOM PRECONDITIONER.... [[REMOVE THIS CODE]]" << endl;
	//Random_Precond rndPrec(N_MRP, 4, 0.1, spar, VL_VERBOSE);

	switch (spar.implicitAlgo) {
		case ILA_GMRES_M: {
			ublas::matrix<double> Hessenberg(KrylovM+1, N_MRP);
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
// TODO: EXPERIMENT

/*void BuildEmbeddedMatrices(const MRP& mrp, const MRPStateKinds& stKinds,
 const SolverParams& spar,
 ublas::compressed_matrix<double>& outP,
 ublas::compressed_matrix<double>& outC,
 VerboseLevel verboseLvl);*/

/*/ Union-Find data structure
 class disjoint_set {
 struct set_info {
 size_t parent;
 size_t rank;
 };
 vector<set_info> ds;	// Disjoint sets
 size_t	set_count;		// Number of sets
 public:
 disjoint_set(size_t sz) {
 set_count = 0;
 append_sets(sz);
 }
 
 inline void append_sets(size_t num) {
 size_t curr_num = ds.size();
 ds.resize(curr_num + num);
 for (size_t i = curr_num; i < ds.size(); i++) {
 ds[i].parent = i;
 ds[i].rank = 0;
 }
 set_count += num;
 }
 
 inline size_t find(const size_t elem) {
 // Find the root of elem
 size_t root = elem;
 while (root != ds[root].parent)
 root = ds[root].parent;
 // Go back in the tree and set root as the parent node of each element
 size_t curr = elem;
 while (curr != ds[curr].parent) {
 size_t next = ds[curr].parent;
 ds[curr].parent = root;
 curr = next;
 }
 return root;
 }
 
 inline void union_sets(const size_t set1, const size_t set2) {
 assert(ds[set1].parent == set1);
 assert(ds[set2].parent == set2);
 
 if (set1 == set2)
 return;	// Already joined
 
 if (ds[set1].rank > ds[set2].rank)
 ds[set2].parent = set1;
 else if (ds[set1].rank < ds[set2].rank)
 ds[set1].parent = set2;
 else { // same rank
 ds[set2].parent = set1;
 ++ds[set1].rank;
 }
 --set_count;
 }
 
 inline size_t num_sets() const { return set_count; }
 };*/

/*struct SccIndexFunctor {
 inline numerical::subsetindex_t
 externalSubsetTo0BasedIndex(const sccindex_t scc) const {
 return numerical::subsetindex_t(int(scc));
 }
 };
 typedef BaseSubsetList<SccIndexFunctor, sccindex_t, marking_t> SccSubsetList;*/



static inline double rand01() {
	return double(rand()) / RAND_MAX;
}

static inline double rand01int() {
	const double epsilon = 0.001;
	return epsilon + ( double(rand()) / (RAND_MAX - 2*epsilon) );
}

static inline double expDuration(double rate, double randomValue) {
	return ( -log(randomValue) / rate );
}

static inline
bool SelectNextState(const ublas::compressed_matrix<double>& M,
					 const marking_t i, marking_t& next_i,
					 double& acc, double randomValue)
{
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
marking_t SimulateSmc(const MRP& mrp, const MRPStateKinds& stKinds,
					  const marking_t initState, double simTime)
{
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
comp_max_freq(const pair<marking_t, size_t>& it1, const pair<marking_t, size_t>& it2)
{	return it1.second < it2.second;   }



void BuildSimulatedEMC(const MRP& mrp, const SolverParams& spar,
							 const MRPStateKinds& stKinds,
							 ublas::compressed_matrix<double>& outM,
							 VerboseLevel verboseLvl)
{
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
			cout << "SIMULATIONG ROW " << i+marking_t(1) << "/" << N_MRP+marking_t(1) << "   ";
			if (timer.get_elapsed_time() > 2) {
				timer.print_estimate(size_t(i)+1, size_t(N_MRP)+1);
				cout << " remained...";
			}
			cout << "             " << endl;
		}
		statesubset_t rowSubset = mrp.Subsets.subsetOf(i);
		if (rowSubset == EXP_SUBSET)
		{
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
			double detTime = mrp.Det[rowSubset].delay;
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


/*struct Jacobi_Precond : public Preconditioner {
 Jacobi_Precond(const MRP& mrp, const SolverParams& spar) {
 const marking_t N_MRP(mrp.N());
 / *MRPStateKinds stKinds;
 ClassifyMRPStates(mrp, stKinds, VL_NONE);
 ublas::compressed_matrix<double> P, C;
 cout << "BUILDING THE PRECONDITIONER MATRIX....\n\n";
 BuildEmbeddedMatrices(mrp, stKinds, spar, P, C, VL_BASIC);* /
 
 invDiag.resize(N_MRP);
 for (marking_t i(0); i < N_MRP; ++i) {
 / *if (P(i, i) != 0.0)
 invDiag(i) = 1.0 / P(i, i);
 else* /
 invDiag(i) = 0.5 + rand01();
 }
 }
 
 ublas::vector<double> doProduct(const ublas::vector<double>& inVec) const {
 const size_t N = inVec.size();
 ublas::vector<double> outVec(N);
 for (size_t i = 0; i < N; i++)
 outVec(i) = inVec(i) / invDiag(i);
 return outVec;
 }
 
 ublas::vector<double> solve(const ublas::vector<double>& inVec) const
 {
 const size_t N = inVec.size();
 ublas::vector<double> outVec(N);
 for (size_t i = 0; i < N; i++)
 outVec(i) = inVec(i) * invDiag(i);
 return outVec;
 }
 
 ublas::vector<double> invDiag;
 };*/


struct DiagILU_Precond : public Preconditioner  {
	DiagILU_Precond(const MRP& mrp, const MRPStateKinds& stKinds,
			        const SolverParams& _spar, VerboseLevel verboseLvl) : spar(_spar)
	{
		ublas::compressed_matrix<double> M;
		BuildSimulatedEMC(mrp, spar, stKinds, M, verboseLvl);
		
		// Build up the D-ILU(0) diagonal preconditioner
		size_t N = M.size1();
		diag.resize(N);
		for (size_t i=0; i<N; i++)
			diag(i) = M(i, i);
		for (size_t i=0; i<N; i++) {
			if (diag(i) != 0)
				diag(i) = 1.0 / diag(i);
			for (size_t j=i+1; j<N; j++) {
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
	ublas::vector<double> solve(const ublas::vector<double>& inVec) const
	{
		size_t N = diag.size();
		ublas::vector<double> outVec(N);
		for (size_t i=0; i<N; i++) {
			if (diag(i) != 0)
				outVec(i) = inVec(i) / diag(i);
			else
				outVec(i) = inVec(i);
		}
		return outVec;
	}
	
private:
	ublas::vector<double> diag;
	const SolverParams& spar;
};



void BuildEmbeddedMatrices(const MRP& mrp, const MRPStateKinds& stKinds,
						   const SolverParams& spar,
						   ublas::compressed_matrix<double>& outP,
						   ublas::compressed_matrix<double>& outC,
						   VerboseLevel verboseLvl);

//-----------------------------------------------------------------------------

/*struct ILU0_Precond : public Preconditioner
{
	// Note: On return _M is cleared
	ILU0_Precond(ublas::compressed_matrix<double>& _M) { M.swap(_M); }

	bool isIdentity() const    {   return false;   }

protected:
	ublas::vector<double> solve(const ublas::vector<double>& inVec) const
	{
		size_t N = inVec.size();
		ublas::vector<double> outVec(N), midVec(N);
		ublas::identity_matrix<double> I(N);

		for (size_t i=0; i<N; i++)
			outVec[i] = 0.0;

		ForeSubstitution(M, I, inVec, midVec);
		BackSubstitution(M, M, midVec, outVec);

		return outVec;
	}

protected:
	ILU0_Precond() { }

	ublas::compressed_matrix<double>   M;
};*/

//-----------------------------------------------------------------------------

template<class Vector, class Predicate>
class subset_selector : boost::noncopyable {
	const Vector &vec;
	std::vector<int> revInd;
	Predicate pred;
public:
	/// Initialize a subset_selector instance.
	inline subset_selector(const Vector &_vec, Predicate _pred = Predicate()) : vec(_vec), pred(_pred) {
		size_t cnt = count_if(vec.begin(), vec.end(), pred), k = 0;
		revInd.resize(cnt);
		for (size_t i=0; i<vec.size(); i++)
			if (pred(vec[i]))
				revInd[k++] = i;
		assert(k == cnt);
	}

	/// Return if the k-th element is in the selected subset.
	inline bool isSelected(size_t k) const
	{	return pred(vec[k]);   }

	/// Returns the selected elements count.
	inline size_t count() const
	{   return revInd.size();   }

	/// Return the n-th selected element.
	inline size_t operator[] (size_t n) const
	{   return revInd[n];   }
};

//-----------------------------------------------------------------------------


struct embedded_state_pred : unary_function <MRPStateKind,bool> {
	inline bool operator() (const MRPStateKind k) const
	{	return (k & MSK_EMC_NOT_EMBEDDED) == 0;   }
};

struct ILU0ofEMC_Precond : public ILU0_Precond  {
	
	inline double safe_inv(double v) { return (v == 0) ? 1 : 1/v; }
	inline double test_threshold(double v) { return (abs(v) < 0.001) ? 0 : v; }
	
	ILU0ofEMC_Precond(const MRP& mrp, const MRPStateKinds& stKinds, bool simulated,
					  const SolverParams& _spar, VerboseLevel verboseLvl) : spar(_spar)
	{
		ublas::compressed_matrix<double> C;
		if (simulated)
			BuildSimulatedEMC(mrp, spar, stKinds, M, verboseLvl);
		else
			BuildEmbeddedMatrices(mrp, stKinds, spar, M, C, verboseLvl);
		
		size_t N = M.size1();
		ublas::identity_matrix<double> I(N);
		FastTranspose(M, M);
		//M = trans(M);
		M *= -1;
		M += I;
		
		subset_selector<vector<MRPStateKind>,embedded_state_pred> emb_state_sel(stKinds.kinds);
		ublas::compressed_matrix<double> projM;
		project_matrix(M, projM, emb_state_sel, emb_state_sel);
		// TODO: this switch between ILU0 and ILUTK should be controllable from the command line...
		//ILU0(projM);
		ILUTK_d(projM, projM, 1.0e-5, 8);
		unproject_matrix(projM, M, N, N, emb_state_sel, emb_state_sel);

		/*/ Build up the ILU(0) preconditioner
		for (size_t r=0; r<N-1; r++) {
			if (stKinds.kinds[r] & MSK_EMC_NOT_EMBEDDED)
				continue;
			double d = safe_inv(M(r,r));
			for (size_t i=r+1; i<N; i++) {
				if (stKinds.kinds[i] & MSK_EMC_NOT_EMBEDDED)
					continue;
				if (M(i,r) == 0)
					continue;
				double e = M(i,r) * d;
				M(i,r) = e;
				for (size_t j=r+1; j<N; j++) {
					if (stKinds.kinds[j] & MSK_EMC_NOT_EMBEDDED)
						continue;
					if (M(i,j) == 0 || M(r,j) == 0)
						continue;
					M(i,j) -= e * M(r,j);
					if (i != j)
						M(i,j) = test_threshold(M(i,j));
				}
			}
		}//*/

		double det = 1;
		for (size_t i=0; i<N; i++)
			det *= M(i,i);
		cout << "ILU(0) DETERMINANT: " << det << endl;

		if (verboseLvl >= VL_VERBOSE && N <= 50) {
			cout << "ILU(0) MATRIX:\n";
			print_matrix(cout, M, "M");
			cout << endl << endl;

			ublas::vector<double> b(N), x(N), y(N);
			for (size_t i=0; i<N; i++) {
				if (stKinds.kinds[i] & MSK_EMC_NOT_EMBEDDED)
					b(i) = 0;
				else
					b(i) = 1.0/N;
				x(i) = y(i) = 0;
			}
			/*ublas::identity_matrix<double> I(N);
			cout << "STRT " << b << endl;
			ForeSubstitution(M, I, b, x);
			cout << "FORE " << x << endl;
			BackSubstitution(M, M, x, y);
			cout << "BACK " << y << endl;
			cout << endl;*/
		}
	}
	
	bool isIdentity() const    {   return false;   }

	/*ublas::vector<double> solve(const ublas::vector<double>& inVec) const
	{
		size_t N = inVec.size();
		ublas::vector<double> outVec(N), midVec(N);
		ublas::identity_matrix<double> I(N);

		for (size_t i=0; i<N; i++)
			outVec[i] = 0.0;

		ForeSubstitution(M, I, inVec, midVec);
		BackSubstitution(M, M, midVec, outVec);

		//outVec = 0.9 * outVec + 0.1 * inVec;

		return outVec;
	}*/
	
private:
	const SolverParams& spar;
};


// TODO: should be called InnerOuterPreconditioner, and EMC_Precond should be a derived class
struct EMC_Precond : public Preconditioner
{
	EMC_Precond(const MRP& mrp, const MRPStateKinds& stKinds, bool simulated,
			              const SolverParams& _spar, VerboseLevel verboseLvl) : spar(_spar)
	{
		ublas::compressed_matrix<double> C;
		if (simulated)
			BuildSimulatedEMC(mrp, spar, stKinds, cM, verboseLvl);
		else
			BuildEmbeddedMatrices(mrp, stKinds, spar, cM, C, verboseLvl);
		ublas::identity_matrix<double> I(cM.size1());
		/*cM *= -1;
		cM += I;*/

		hasInternalILU0 = true;
		if (hasInternalILU0) {
			ublas::compressed_matrix<double> ilu0;
			FastTranspose(cM, ilu0);
			ilu0 *= -1;
			ilu0 += I;
			if (verboseLvl >= VL_BASIC)
				cout << "BUILDING INTERNAL ILU0 PRECONDITIONER..." << endl;
			ILU0(ilu0);
			ilu0prec.reset(new ILU0_Precond(ilu0));
		}

		numSol = iterCnt = 0;
	}
	
	ublas::compressed_matrix<double> cM;
	mutable size_t numSol, iterCnt;
	const SolverParams& spar;
	
	bool hasInternalILU0;
	scoped_ptr<ILU0_Precond> ilu0prec;

	
	class PrecResidualFunctor {
	public:
		inline PrecResidualFunctor(const EMC_Precond* _p) : p(_p) { }
		
		inline size_t size() const		{  return p->cM.size1();  }
		
	 	template<class Vector>
		inline ublas::vector<double> product(const Vector &inVec) const
		{
			const size_t N = size();
			double alpha = p->spar.ipaAlpha;
			//double unifVal = (1.0 - alpha) / N;
			ublas::vector<double> outVec(N);
			full_element_selector elems(N);
			
			prod_vecmat(outVec, inVec, p->cM, elems, elems);
			if (alpha != 0.0) {
				/*for (size_t i=0; i<N; i++)
					outVec(i) = outVec(i) * alpha + unifVal;*/

				double v0 = outVec(0), vprev = outVec(N-1);
				for (size_t i=0; i<N; i++) {
					double tmp = outVec(i);
					outVec(i) = outVec(i) * alpha + (1-alpha)*vprev;
					vprev = (i==N-1) ? v0 : tmp;
				}
			}
			outVec -= inVec;
			return outVec;
		}
		
	private:
		const EMC_Precond* p;
	};
	
	bool isIdentity() const    {   return false;   }

	/*ublas::vector<double> doProduct(const ublas::vector<double>& inVec) const {
	 PrecResidualFunctor Mat(this);
	 return Mat.product(inVec);// + inVec;
	 }*/
	
	
protected:
	ublas::vector<double> solve(const ublas::vector<double>& b) const
	{
		const size_t N = b.size();
		ublas::vector<double> x(N);
		
		// TODO: the preconditioner solve method should be changeable by the user
		
		fill(x.begin(), x.end(), 1.0/N);
		PrecResidualFunctor Mat(this);
		NullPreconditioner nullPrec;
		Preconditioner *p = &nullPrec;
		if (hasInternalILU0)
			p = ilu0prec.get();
		long_interval_timer timer;
		LinearIterationPrinter printOut(VL_BASIC, spar, LSM_IMPLICIT, &timer);
		size_t max_iter = spar.KrylovM; //spar.maxIters;
		double tol = sqrt(spar.epsilon);// /*1.0e-10;*/ spar.epsilon /10;
		size_t KrylovM = min(size_t(spar.KrylovM), N-1);
		ublas::matrix<double> Hessenberg(KrylovM+1, N);
		GMRES(Mat, x, b, *p, nullPrec, Hessenberg, KrylovM, true, max_iter, tol, printOut);
		//throw program_exception("Internal GMRES does not converge.");
		//IR2(Mat, x, b, prec,  prec, max_iter, tol, spar.omega, printOut);
		//cout << "PRECONDITIONER SOLVE: " << max_iter << " iterations, resid = " << tol<< endl;
		iterCnt += max_iter;
		numSol++;
		cout << "  E[iterCount]=" << double(iterCnt)/numSol << endl;
		cout << endl;//*/
		
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
						 const Preconditioner& M,
						 double& epsilon, const SolverParams& spar,
						 size_t& maxIters, VerboseLevel verboseLvl)
{
	const size_t N_MRP = A.size();
	assert(N_MRP == b.size() && x.size() == N_MRP);
	full_element_selector AllMrpElems(N_MRP);
	size_t iterCount = 0;
	double rel_err;
	long_interval_timer timer;
	LinearIterationPrinter printOut(verboseLvl, spar, LSM_IMPLICIT, &timer);
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
	while (iterCount < maxIters)
	{
		/*switch (algo) {
		 case ILA_POWER_METHOD:
		 PowerMethodIteration(A, x, b, spar, rel_err,
		 iterCount, printOut, verboseLvl);
		 break;
		 
		 default:*/
		if (!Krylov_Iteration(A, x, b, M, spar, rel_err,
							  iterCount, printOut, verboseLvl))
		{
			if (algo != ILA_POWER_METHOD) {
				printOut.onFailure(iterCount, CFR_BREAKDOWN_SWITCH_TO_POWERMETHOD);
				algo = ILA_POWER_METHOD;
			}
			else return false;
		}
		//		break;
		//}
		
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
void InitialGuess(ublas::vector<double> &x0, const MRPStateKinds& sk,
				  const RowSelector& rows)
{
	const marking_t N_MRP(sk.kinds.size());
	x0.resize(N_MRP);
	size_t cnt = 0;
	for (size_t r=0; r<rows.count(); r++)
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
void FilterSubsets(const MRPSubsetList& inSubset, MRPSubsetList& outSubset,
				   const Selector& elems)
{
	const marking_t N_MRP(inSubset.numStates());
	vector<statesubset_t> stateToSubset(N_MRP);
	
	for (marking_t m(0); m<N_MRP; ++m) {
		if (elems.isSelected(m))
			stateToSubset[m] = inSubset.subsetOf(m);
		else
			stateToSubset[m] = IGNORED_SUBSET;
	}
	outSubset.ConstructSubsetList(inSubset.subsetCount(), stateToSubset);
}

//-----------------------------------------------------------------------------

template<class VectorB, class RowSelector, class ColSelector>
bool Solve(const MRP& mrp, const MRPStateKinds& stKinds,
		   const SolverParams& spar, const VectorB& B,
		   ublas::vector<double>& emcSol, ublas::vector<double> *optMrpSol,
		   SteadyStateComputation whatToDo, VerboseLevel verboseLvl,
		   const RowSelector& rows, const ColSelector& cols)
{ 
	const marking_t N_MRP(mrp.N()), N_ROWS(rows.count()), N_COLS(cols.count());
	UNUSED_PARAM(N_ROWS);
	UNUSED_PARAM(N_COLS);
	
	if (rows.count() == 1 && 
		count_matrix_row_nonzero_values(mrp.Q, rows[0]) == 0 &&
		count_matrix_row_nonzero_values(mrp.Qbar, rows[0]) == 0 &&
		count_matrix_row_nonzero_values(mrp.Delta, rows[0]) == 0)
	{
		emcSol(rows[0]) = 1.0 -B(rows[0]);
		if (optMrpSol)
			(*optMrpSol)(rows[0]) = 1.0 -B(rows[0]);
		return true;
	}
	
	// Rebuild the MRPSubsetList if there is a selection of rows.
	// In any case we will use the subsets pointed by *pSubsets
	const MRPSubsetList	*pSubsets;
	MRPSubsetList filteredSubsets;	
	if (rows.count() == mrp.N())
		pSubsets = &mrp.Subsets;
	else {
		FilterSubsets(mrp.Subsets, filteredSubsets, rows);
		pSubsets = &filteredSubsets;
	}
	
	// Build up the preconditioner matrix
	if (verboseLvl >= VL_BASIC)
		cout << "\nUSING " << IPA_Names[spar.implPrecond] << " PRECONDITIONER." << endl;
	scoped_ptr<Preconditioner> rightPrec;
	switch (spar.implPrecond) {
		case IPA_NO_PRECONDITIONER:
			rightPrec.reset(new NullPreconditioner());
			break;
			
		case IPA_EMC:
			rightPrec.reset(new EMC_Precond(mrp, stKinds, false, spar, verboseLvl));
			break;

		case IPA_SIMULATED_EMC:
			rightPrec.reset(new EMC_Precond(mrp, stKinds, true, spar, verboseLvl));
			break;
			
		/*case IPA_DIAG_ILU:
			rightPrec.reset(new DiagILU_Precond(mrp, stKinds, spar, verboseLvl));
			break;*/
			
		case IPA_ILU0:
			rightPrec.reset(new ILU0ofEMC_Precond(mrp, stKinds, false, spar, verboseLvl));
			break;

		case IPA_SIMULATED_ILU0:
			rightPrec.reset(new ILU0ofEMC_Precond(mrp, stKinds, true, spar, verboseLvl));
			break;

		default:
			throw program_exception("Wrong preconditioner index.");
	}
	
	//size_t KrylovM = min(size_t(N_ROWS), spar.KrylovM);
	size_t outIters = spar.maxIters;
	double outTol = spar.epsilon;
	
	typedef EmcVectorMatrixProduct<RowSelector, ColSelector> VecMatProd_t; 
	VecMatProd_t ImplicitEmcProd(mrp, spar, *pSubsets, rows, cols);
	InitialGuess(emcSol, stKinds, rows);
	
	// STEP 1) Solve the linear equation system in implicit form
	if (verboseLvl >= VL_BASIC)
		cout << "COMPUTING STEADY STATE SOLUTION OF THE EMBEDDED DTMC...\n" << endl;
	if (!LinearSolveImplicit(ImplicitEmcProd, emcSol, B, *rightPrec,
							 outTol, spar, outIters, verboseLvl))
		throw program_exception("Didn't converge.");	
	
	if (whatToDo == SSC_ONLY_UNNORMALIZED_EMC_PROB)
		return true;
	
	emcSol /= vec_sum(emcSol);
	
	// STEP 2) Convert the EMC solution into the MRP solution
	//         Compute a(M) and b(M) for the EMC solution vector u(M)
	if (verboseLvl >= VL_BASIC)
		cout << "CONVERSION OF THE EMC SOLUTION INTO THE MRP SOLUTION...\n" << endl;
	
	//full_element_selector AllMrpElems(N);
	NullIterPrintOut printOut;
	MRPSubsetList::selector ExpStates(pSubsets->selectorOf(EXP_SUBSET));
	vector<DetMarkingSubset>::const_iterator SgenIt;
	ublas::vector<double> aM(N_MRP), bM(N_MRP), ag, bg;
	std::fill(aM.begin(), aM.end(), 0.0);
	std::fill(bM.begin(), bM.end(), 0.0);
	for (SgenIt = mrp.Det.begin(); SgenIt != mrp.Det.end(); ++SgenIt) {
		if (pSubsets->numStatesIn(SgenIt->index) == marking_t(0))
			continue;
		MRPSubsetList::selector DetStates(pSubsets->selectorOf(SgenIt->index));
		// TODO: remove this
		/*if (DetStates.count() == 0)
		 continue;*/
		Uniformization2(mrp.Q, emcSol, ag, bg, SgenIt->delay,
						spar.epsilon, DetStates, cols, printOut);
		add_vec(aM, ag, DetStates);
		add_vec(bM, bg, DetStates);
	}
	
	// Normalization constant:  c = u(M).C.e = -u(M)^E.diag^-1(Q^E).e + b(M).e
	double c = 0.0;
	for (size_t r=0; r<ExpStates.count(); r++) {
		const size_t i = ExpStates[r];
		if (mrp.Q(i,i) != 0.0)
			c -= emcSol(i) / mrp.Q(i,i);
	}
	c += vec_sum(bM);
	
	// Avoid zero conversion factors (this happens with absorbing states)
	if (c == 0.0)
		c = 1.0; 
	
	// MRP state probabilities: pi = v.C = -(1/c)u(M)^E.diag^-1(Q^E) + (1/c)b(M)
	optMrpSol->resize(N_MRP);
	std::fill(optMrpSol->begin(), optMrpSol->end(), 0.0);
	for (marking_t m(0); m<N_MRP; ++m) {
		if (mrp.Subsets.subsetOf(m) == EXP_SUBSET) {
			if (mrp.Q(m,m) != 0.0)
				(*optMrpSol)(m) = -emcSol(m) / (c * mrp.Q(m,m));
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

/*template<class TSelector, class ASelector>
 void ImplicitProduct_TA(const MRP& mrp, const SolverParams& spar, 
 const ublas::vector<double>& inVec,
 ublas::vector<double>& out, 
 const TSelector& sel_T, const ASelector& sel_A)
 {
 const marking_t N_MRP(mrp.N());
 fill(out.begin(), out.end(), 0.0);
 
 / *MRPSubsetList Subsets_T, Subsets_A;
 FilterSubsets(mrp.Subsets, Subsets_T, sel_T);
 FilterSubsets(mrp.Subsets, Subsets_A, sel_A);* /
 
 // The implicit EMC x Vector product is done in two phases.
 // 1) Exponential states:
 //    input vector is multiplied with P^E = I^E - diag^{-1}(Q^E)Q^E
 
 MRPSubsetList::selector ExpStates(mrp.Subsets.selectorOf(EXP_SUBSET));
 add_invDiagM_M(out, inVec, mrp.Q, ExpStates, sel_A); // TODO: al contrario??
 add_vec(out, inVec, sel_A);
 
 // 2) Deterministic states:
 //    input vector is multiplied with Omega.Delta + Psi.Qbar
 vector<DetMarkingSubset>::const_iterator SgenIt;
 ublas::vector<double> ag(N_MRP), bg(N_MRP);
 for (SgenIt = mrp.Det.begin(); SgenIt != mrp.Det.end(); ++SgenIt) {
 MRPSubsetList::selector DetStates(mrp.Subsets.selectorOf(SgenIt->index));
 full_element_selector FullMrpSelector(N_MRP);
 
 // Compute uniformization over the Sgen subset *SgenIt
 Uniformization2(mrp.Q, inVec, ag, bg, SgenIt->delay,
 spar.epsilon, DetStates, FullMrpSelector, VL_NONE);
 add_prod_vecmat(out, ag, mrp.Delta, DetStates, sel_A);
 add_prod_vecmat(out, bg, mrp.Qbar, DetStates, sel_A);
 }
 set_vec(out, 0.0, sel_T);
 }*/


bool SteadyStateMRPImplicit(const MRP& mrp, const MRPStateKinds& stKinds,
							PetriNetSolution& sol, const SolverParams& spar,
							VerboseLevel verboseLvl)
{
	const marking_t N_MRP(mrp.N());
	BSCC bscc;
	full_element_selector FullMrpSelector(N_MRP);
	ublas::zero_vector<double> zeroB(N_MRP);
	
	BottomStronglyConnectedComponents(mrp.Q + mrp.Qbar + mrp.Delta, bscc);
	if (bscc.numBscc == sccindex_t(0)) {
		// The MRP is ergodic, solve it directly
		if (verboseLvl >= VL_BASIC)
			cout << "STEADY STATE SOLUTION OF ERGODIC MRP." << endl;
		ublas::vector<double> emcSol(N_MRP, 0.0);
		return Solve(mrp, stKinds, spar, zeroB, emcSol, 
					 &sol.stateProbs, SSC_COMPUTE_ALL, verboseLvl, 
					 FullMrpSelector, FullMrpSelector);
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
			// Solve the initial transient sub-matrix.
			// The solution of this sub-matix is simply given by the solution of
			// the filtered generators Q, Qbar and Delta.
			if (!Solve(mrp, stKinds, spar, -mrp.pi0, sigma0, nullptr, 
					   SSC_ONLY_UNNORMALIZED_EMC_PROB,
					   verboseLvl, InitTrnElems, InitTrnElems))
				return false;
			
			// Compute how the initial transient probability enters the 
			// numBscc recurrent classes. This requires a single product
			// with the rectangular EMC sub-matrix that contains the 
			// transitions from the initial transient to the recurrent classes.
			typedef subset_element_selector<sccindex_t, not_equal_to<sccindex_t> > inv_subset_sel;
			not_equal_to<sccindex_t> comparator;
			inv_subset_sel RecurrentElems(&bscc.indices, TRANSIENT_COMPONENT, comparator);
			
			typedef	EmcVectorMatrixProduct<full_element_selector, 
			full_element_selector> EmcProd;
			EmcProd sigma0xP0i(mrp, spar, mrp.Subsets, FullMrpSelector, 
							   FullMrpSelector);
			piAbs = sigma0xP0i.product(sigma0);
			set_vec(piAbs, 0.0, InitTrnElems);
			
			// Add to piAbs the probability of starting in a recurrent class
			add_vec(piAbs, mrp.pi0, RecurrentElems);
			//cout << "piAbs = " << piAbs << endl;
			piAbs /= vec_sum(piAbs);
		}
		else piAbs = mrp.pi0;
	}
	
	// Get the steady-state probabilities of entering in each BSCC
	sol.stateProbs.resize(N_MRP);
	std::fill(sol.stateProbs.begin(), sol.stateProbs.end(), 0.0);
	for (sccindex_t nc(0); nc < bscc.numBscc; nc++) {
		subset_element_selector<sccindex_t> RecurrentClassElems(&bscc.indices, nc);
		// Get the total probability of this recurrent class
		double probBscc = vec_sum(piAbs, RecurrentClassElems);
		if (verboseLvl >= VL_VERBOSE)
			cout << "  Pr[BSCC "<<(int(nc)+1)<<"] = " << probBscc << endl;
		
		// Get the steady-state solution of the bscc sub-matrix
		ublas::vector<double> rcEmcSol(N_MRP, 0.0), rcMrpSol(N_MRP, 0.0);
		if (!Solve(mrp, stKinds, spar, zeroB, rcEmcSol, 
				   &rcMrpSol, SSC_COMPUTE_ALL, VL_NONE, 
				   RecurrentClassElems, FullMrpSelector))
			return false;
		
		// Normalize with the total bscc probability
		double norm = vec_sum(rcMrpSol, RecurrentClassElems);
		norm = probBscc / norm;
		
		// Set the solution vector sol[]
		sol.stateProbs += rcMrpSol * norm;
	}
	
	sol.stateProbs /= vec_sum(sol.stateProbs);
	
	if (verboseLvl >= VL_VERBOSE) {
		//cout << "EMC SOLUTION: " << emcSol << endl;
		cout << "MRP SOLUTION: " << sol.stateProbs << "   ";
		cout << vec_sum(sol.stateProbs) << endl;
		cout << endl;
	}
	
	return true;
}

//=============================================================================

void SteadyStateMRPImplicit(RG& rg, PetriNetSolution& sol,
							const SolverParams& spar, VerboseLevel verboseLvl)
{
	MRP mrp;
	ExtractMRP(rg, mrp, verboseLvl);
	
	MRPStateKinds stKinds;
	ClassifyMRPStates(mrp, stKinds, verboseLvl);
	
	SteadyStateMRPImplicit(mrp, stKinds, sol, spar, verboseLvl);
	ComputeMeasures(rg, sol, verboseLvl);
}

//=============================================================================





