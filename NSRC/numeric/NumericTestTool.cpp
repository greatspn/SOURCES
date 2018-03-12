
//  g++ ublas-test.cpp -o ublas-test -I /usr/local/boost-1.39/include/boost-1_39/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <vector>
using namespace std;

#include "numeric.h"
using namespace numerical;



// TODO: commentare
/*template<class Vector, class Selector>
void copy_vec(const Vector& in, Vector& out, const Selector& sel) 
{
	for (size_t n=0; n<sel.count(); n++)
		out(sel[n]) = in(sel[n]);
}*/


/*
//-----------------------------------------------------------------------------
/// Solves a system of linear equations in the form \f$(A+Id) \cdot x=b\f$
/// with the GMRES(m) algorithm, as written in "Templates for the Solution of 
/// Linear Systems: Building Blocks for Iterative Methods".
///   \param BmAX	  Functor that computes the residual vector $b-Ax$.
///   \param sol	  On output contains the solution vector.
///   \param m0       Number of vectors in the projected Krylov space.
///   \param epsilon  Max. relative error.
///   \param omega	  Over-relaxation parameter (SOR/JOR method), 0<\a omega<2.
///   \param maxIters Maximum number of iterations (before returning false).
///   \param verboseLvl Print informations on the console.
///   \param rows	  Rows of (A,x,b) selected for the computation.
///   \param cols	  Columns of (A,x,b) selected for the computation.
///   \param init_x0  Initial distribution of x(0), (nullptr for a uniform vector).
///   \return True if the method converges in less than \a maxIters iterations.
///
/// \note
/// On return, the sol[] elements not selected by (rows) are unchanged.
//-----------------------------------------------------------------------------
template<class ResidualFunctor, class Vector, class RowSelector, class ColSelector>
bool GMRES(const ResidualFunctor& BmAX, Vector& sol, const size_t m0,
		   const double epsilon, const double omega, const size_t maxIters, 
		   const VerboseLevel verboseLvl, const RowSelector& rows, const ColSelector& cols,
		   const VectorIn *init_x0 = nullptr)
{
	const size_t N = A.size1();
	assert(A.size2() == N && b.size() == N && d.size() == N);
	assert(omega > 0.0 && omega < 2.0);
	assert(init_x0 == nullptr || init_x0->size() == N);
	
	x.resize(N);
	
	// Special 1x1 matrix case:  x(0) = b(0) / (A(0,0) * init_x0)
	if (rows.count() == 1) {
		if (verboseLvl >= VL_VERBOSE)
			cout << "GMRES: trivial solution for 1x1 matrix." << endl;
		BmAX(x, *init_x0, row, col);
		return true;
	}
	
	// Initial guess for x(0)
	size_t M = std::min(m0, N);
	copy_vec(*init_x0, x, rows);
	Vector r;
	
	ublas::matrix<double> v(N, M+1), w(N, M);
	
	// Main GMRES(m) iteration
	for (size_t iter=0; iter<maxIters; iter++) {
		// Compute the residual vector r(i)
		BmAX(r, x, rows, cols);
		double beta = norm_1(r);
	}
}*/






template<class M>
M& subtract_diag_sum(M& A) {
	for (typename M::iterator1 i= A.begin1(); i != A.end1(); ++i) {
		double sum = 0.0;
		for (typename M::iterator2 j= i.begin(); j != i.end(); ++j)
			if (i.index1() != j.index2())
				sum += *j;
		A(i.index1(), i.index1()) -= sum;
	}
	return A;
}


//typedef ublas::compressed_matrix<double>	sparse_matrix_t;
//typedef ublas::mapped_matrix<double>		mapped_matrix_t;


// TODO: rendere la non-convergenza un'eccezione c++???

// TODO: solve steady state DTMC


//#include "mrmcFoxGlynn-141.h"








bool LoadPrismCTMC(const char* filename, 
				   ublas::compressed_matrix<double>& Q,
				   VerboseLevel verboseLvl)
{
	ifstream ifs(filename);
	if (!ifs) {
		cout << "Cannot open file "<<filename<<endl;
		Q.clear();
		return false;
	}
	
	double rate;
	size_t N, T, from, to;
	ifs >> N >> T;
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << "LOADING \""<<filename<<"\" ("<<flush;
		cout << N << " States, " << T << " Transitions)... " << flush;
	}
	
	//cout << "\nQ = Normal[SparseArray[{";
	
	ublas::mapped_matrix<double> mQ(N, N, T+N);
	for (size_t i=0; i<T; i++) {
		ifs >> from >> to >> rate;
		mQ.insert_element(from, to, rate);
		//cout << "{"<<(from+1)<<","<<(to+1)<<"}->"<<rate<<", ";
	}
	//cout << "}]];\nQ = Q - Total[Transpose[Q]]*IdentityMatrix[Length[Q]];\n";
	//cout << "pi0 = Table[If[i == 1, 1, 0], {i, Length[Q]}];\n";
	//cout << "PiT = CTMCtransient[Q, pi0, 1.0, 7]\n";
	//cout << "Total[PiT]\n(If[# > 0.00001, #, 0]) & /@ PiT\n" << endl;
	subtract_diag_sum(mQ);
	Q = mQ;
	
	if (verboseLvl >= VL_VERBOSE)
		cout << "DONE!" << endl;
	
	return true;
}


bool LoadBoostCTMC(const char* filename, 
				   ublas::compressed_matrix<double>& Q,
				   bool subtractDiag, VerboseLevel verboseLvl)
{
	ifstream ifs(filename);
	if (!ifs) {
		cout << "Cannot open file "<<filename<<endl;
		Q.clear();
		return false;
	}
	
	ublas::matrix<double> mQ;
	ifs >> mQ;
	
	if (subtractDiag) {
		subtract_diag_sum(mQ);
		/*for (size_t n=0; n<mQ.size1(); n++)
			mQ(n,n) -= 1.0;	
		for (size_t n=0; n<mQ.size1(); n++) {
			double sum = 0;
			for (size_t m=0; m<mQ.size2(); m++)
				sum += mQ(n,m);
			mQ(n,n) -= sum;
		}
		for (size_t n=0; n<mQ.size1(); n++) {
			double sum = 0;
			for (size_t m=0; m<mQ.size2(); m++)
				sum += mQ(n,m);
			cout << "Sum["<<n<<"] = "<<sum<<endl;
		}*/
	}
	
	
	Q = mQ;
	if (verboseLvl >= VL_VERBOSE)
		cout << "DONE!" << endl;
	return true;
}


void PrintInMathematicaFormat(const ublas::compressed_matrix<double>& Q)
{
	cout.precision(11);
	cout << "MAT={\n  {";
	for (size_t r=0; r<Q.size1(); r++) {
		cout << (r==0 ? "" : "},\n  {");
		for (size_t c=0; c<Q.size2(); c++) {
			cout << (c==0 ? "" : ", ");
			cout << Q(r,c);
		}
	}
	cout << "}\n};" << endl;
	cout.precision(0);
}




int main(int argc, char *argv[]) 
{
	int argNum = 1;
	bool loadedQ = false;
	double omega = 1.0;
	double epsilon = 1.0e-7;
	size_t maxIters = 10000;
	ublas::compressed_matrix<double> Q;
	ublas::vector<double> pi0;
	SteadyStateAlgorithm algo = SSA_FORWARD_SOR;
	
	while (argNum < argc) 
	{
		const char *cmdArg = argv[argNum++];
		size_t remainedArgs = argc - argNum;
		
		if (0==strcmp(cmdArg, "-s")) {
			if (!loadedQ) {
				cout << "Load a CTMC before using the -s option." << endl;
				continue;
			}
			
			ublas::vector<double> x;
			Solve_SteadyState_CTMC(Q, x, pi0, epsilon, omega, maxIters, true, algo);
			cout << "π = " << x << endl << endl;			
		}
		else if (0==strcmp(cmdArg, "-t") && remainedArgs >= 1) {
			if (!loadedQ) {
				cout << "Load a CTMC before using the -t option." << endl;
				continue;
			}
			const double t = atof(argv[argNum++]);
			ublas::vector<double> x, x2, x3, xc, xc2, xc3;
			
			do {
				ublas::unit_vector<double> pi0(Q.size1(), 0);
				full_element_selector all_elems(Q.size1());
				Uniformization(Q, pi0, &x, &xc, t, epsilon, all_elems, all_elems, 
							   true, TA_STANDARD_UNIFORMIZATION);
				//cout << "π("<<t<<") = " << x << endl;
				//cout << "|π("<<t<<")| = " << norm_1(x) << endl;
				for (size_t n=0; n<x.size(); n++) {
					if (x(n) < epsilon/10)
						x(n) = 0;
					if (xc(n) < epsilon/10)
						xc(n) = 0;
				}
				cout << " π("<<t<<")\' = " << x << endl;
				cout << "πc("<<t<<")\' = " << xc << endl << endl;
				
				Uniformization(Q, pi0, &x2, &xc2, t, epsilon, all_elems, all_elems,
							   true, TA_FOX_GLYNN_UNIFORMIZATION);
				//cout << "π("<<t<<") = " << x2 << endl;
				//cout << "|π("<<t<<")| = " << norm_1(x2) << endl;
				for (size_t n=0; n<x2.size(); n++) {
					if (x2(n) < epsilon/10)
						x2(n) = 0;
					if (xc2(n) < epsilon/10)
						xc2(n) = 0;
				}
				cout << " π("<<t<<")\' = " << x2 << endl;
				cout << "πc("<<t<<")\' = " << xc2 << endl << endl;
				
				Uniformization2(Q, pi0, x3, xc3, t, epsilon, 
								all_elems, all_elems, true);
				//cout << "π("<<t<<") = " << x2 << endl;
				//cout << "|π("<<t<<")| = " << norm_1(x2) << endl;
				for (size_t n=0; n<x3.size(); n++) {
					if (x3(n) < epsilon/10)
						x3(n) = 0;
					if (xc3(n) < epsilon/10)
						xc3(n) = 0;
				}
				cout << " π("<<t<<")\' = " << x3 << endl;
				cout << "πc("<<t<<")\' = " << xc3 << endl << endl;				
			} while(false);
			
			bool failedCmp = false;
			cout << "COMPARING INSTANTANEOUS AND CUMULATIVE PROBABILITY VECTORS..."<<endl;
			for (size_t n=0; n<x.size(); n++) {
				if (abs(x(n) - x2(n)) > epsilon || abs(x(n) - x3(n)) > epsilon) {
					cout << "  !!! DIFFERENT PROBABILITIES !!!" << flush;
					printf(" %3d  %1.9lf - %1.9lf = %1.9lf\n", 
						   n, x(n), x2(n), abs(x(n) - x2(n)));
					fflush(stdout);
					failedCmp = true;
				}
				if (abs(xc(n) - xc2(n)) > epsilon || abs(xc(n) - xc3(n)) > epsilon) {
					cout << "  !!! DIFFERENT CUMULATIVE PROBABILITIES !!!" << flush;
					printf(" %3d  %1.9lf - %1.9lf = %1.9lf\n", 
						   n, xc(n), xc2(n), abs(xc(n) - xc2(n)));
					fflush(stdout);
					failedCmp = true;
				}
			}
			if (!failedCmp)
				cout << "OK. Vectors differences are less than epsilon.\n"<<endl;
		}
		else if (0==strcmp(cmdArg, "-steady-bench")) {
			if (!loadedQ) {
				cout << "Load a CTMC before using the -steady-bench option." << endl;
				continue;
			}
			const size_t N = Q.size1(), N_SOLS=50;
			char const* names[N_SOLS];
			vector< ublas::vector<double> > sols(N_SOLS);
			ublas::compressed_matrix<double> QT = trans(Q);
			ublas::zero_vector<double> b(N), d(N);
			full_element_selector full_matrix(N);
			
			for (size_t k=0; k<N_SOLS; k++) {
				(sols[k]).resize(N);
				fill((sols[k]).begin(), (sols[k]).end(), 1.0/N);
			}
			int i=0;

			/*Jacobi(QT, sols[i], b, epsilon, omega, 10000, true, 
				   full_matrix, full_matrix);
			names[i++] = "Jacobi";
			GaussSeidel(QT, sols[i], b, epsilon, omega, 10000, true, 
						full_matrix, full_matrix);
			names[i++] = "GaussSeidel";
			GaussSeidel2(QT, sols[i], b, epsilon, omega, 10000, true, 
						 full_matrix, full_matrix);
			names[i++] = "GaussSeidel2";
			JacobiTransposed(sols[i], Q, b, epsilon, omega, 10000, true, 
							 full_matrix, full_matrix);
			names[i++] = "JacobiTransposed";
			GaussSeidelTransposed(sols[i], Q, b, epsilon, omega, 10000, true, 
								  full_matrix, full_matrix);
			names[i++] = "GaussSeidelTransposed";
			GaussSeidelTransposed2(sols[i], Q, b, epsilon, omega, 10000, true, 
								   full_matrix, full_matrix);
			names[i++] = "GaussSeidelTransposed2";*/
			LinearSolve(QT, sols[i], b, d, epsilon, omega, maxIters, true,
						SSA_JOR, full_matrix, full_matrix);
			names[i++] = "LinearSolve2[SSA_JOR]";
			LinearSolve(QT, sols[i], b, d, epsilon, omega, maxIters, true,
						SSA_FORWARD_SOR, full_matrix, full_matrix);
			names[i++] = "LinearSolve2[SSA_FORWARD_SOR]";
			LinearSolve(QT, sols[i], b, d, epsilon, omega, maxIters, true,
						SSA_BACKWARD_SOR, full_matrix, full_matrix);
			names[i++] = "LinearSolve2[SSA_BACKWARD_SOR]";
			LinearSolve(QT, sols[i], b, d, epsilon, omega, maxIters, true,
						SSA_SYMMETRIC_SOR, full_matrix, full_matrix);
			names[i++] = "LinearSolve2[SSA_SYMMETRIC_SOR]";
			LinearSolveTransposed(sols[i], Q, b, d, epsilon, omega, maxIters, true,
								  SSA_JOR, full_matrix, full_matrix);
			names[i++] = "LinearSolveTransposed2[SSA_JOR]";
			LinearSolveTransposed(sols[i], Q, b, d, epsilon, omega, maxIters, true,
								  SSA_FORWARD_SOR, full_matrix, full_matrix);
			names[i++] = "LinearSolveTransposed2[SSA_FORWARD_SOR]";
			LinearSolveTransposed(sols[i], Q, b, d, epsilon, omega, maxIters, true,
								  SSA_BACKWARD_SOR, full_matrix, full_matrix);
			names[i++] = "LinearSolveTransposed2[SSA_BACKWARD_SOR]";
			LinearSolveTransposed(sols[i], Q, b, d, epsilon, omega, maxIters, true,
								  SSA_SYMMETRIC_SOR, full_matrix, full_matrix);
			names[i++] = "LinearSolveTransposed2[SSA_SYMMETRIC_SOR]";
			
			NullPreconditioner precond;
			size_t KrylovM = 20, outIters = maxIters;
			double outTol = epsilon;
			ublas::matrix<double> Hessenberg(KrylovM+1, N);
			SimpleResidualFunctor< ublas::compressed_matrix<double> > AXprod(&QT);
			GMRES(AXprod, sols[i], b, precond, Hessenberg, KrylovM, outIters, outTol);
			cout << "GMRES("<<KrylovM<<"): outIters="<<outIters<<" err="<<outTol<<endl;
			names[i++] = "GMRES(20)";
			
			outIters = maxIters; outTol = epsilon;
			BiCGSTAB(AXprod, sols[i], b, precond, outIters, outTol);
			cout << "BiCGSTAB: outIters="<<outIters<<" err="<<outTol<<endl;
			names[i++] = "BiCGSTAB";
			
			outIters = maxIters; outTol = epsilon;
			CGS(AXprod, sols[i], b, precond, outIters, outTol);
			cout << "CGS: outIters="<<outIters<<" err="<<outTol<<endl;
			names[i++] = "CGS";
			
			cout << endl;
			for (int k=0; k<i; k++) {
				ublas::vector<double> p;
				full_element_selector all_elems(Q.size1());
				prod_vecmat(p, sols[k], Q, all_elems, all_elems);
				cout <<names[k]<<": norm_err="<<norm_1(p)<<"\n";
				cout <<(sols[k]/norm_1(sols[k]))<<"\n\n";
			}
			cout << endl;
		}
		else if (0==strcmp(cmdArg, "-jor")) {
			cout << "Using JOR Algorithm for Steady State solutions.\n"<<endl;
			algo = SSA_JOR;
		}
		else if (0==strcmp(cmdArg, "-forward-sor")) {
			cout << "Using Forward SOR Algorithm for Steady State solutions.\n"<<endl;
			algo = SSA_FORWARD_SOR;
		}
		else if (0==strcmp(cmdArg, "-backward-sor")) {
			cout << "Using Backward SOR Algorithm for Steady State solutions.\n"<<endl;
			algo = SSA_BACKWARD_SOR;
		}
		else if (0==strcmp(cmdArg, "-ssor")) {
			cout << "Using Symmetric SOR Algorithm for Steady State solutions.\n"<<endl;
			algo = SSA_SYMMETRIC_SOR;
		}
		else if (0==strcmp(cmdArg, "-omega") && remainedArgs >= 1) {
			omega = atof(argv[argNum++]);
			cout << "Using relaxation coefficient "<<omega<<" for JOR/SOR methods.\n"<<endl;
		}
		else if (0==strcmp(cmdArg, "-maxIters") && remainedArgs >= 1) {
			maxIters = (size_t)atoi(argv[argNum++]);
			cout << "Maximum number of iteration: "<<maxIters<<"\n"<<endl;
		}
		else if (0==strcmp(cmdArg, "-epsilon") && remainedArgs >= 1) {
			epsilon = atof(argv[argNum++]);
			cout << "Using error tolerance "<<epsilon<<".\n"<<endl;
		}
		else if (0==strcmp(cmdArg, "-foxglynn") && remainedArgs >= 1) {
			double lambda = atof(argv[argNum++]);
			FoxGlynnWeights fgw;
			FG_Weighter(lambda, epsilon, 1.0e-300, 1.0e300, fgw, true);
			cout << endl;
			
			cout << "Left Poisson Tail = " << fgw.L << endl;
			cout << "Right Poisson Tail = " << fgw.R << endl;
			cout << "Weight Sum = " << fgw.SumW << endl;
			for (size_t i=0; i<fgw.W.size(); i++)
				cout << "W["<<(fgw.L+i)<<"] = "<<fgw.W[i]<<endl;
			
			/*FoxGlynn *fg;
			bool ret = fox_glynn(lambda, 1.0e-300, 1.0e300, epsilon, &fg);
			cout << "MRMC FOX-GLYNN("<<ret<<"): L="<<fg->left;
			cout <<" R="<<fg->right<<" W(M)="<<fg->weights[int(lambda)-fg->left]<<endl;
			cout << "\n";
			if (fg->left == fgw.L && fg->right == fgw.R) {
				for (int i=0; i<fgw.W.size(); i++) {
					if ((fgw.W[i] - fg->weights[i]) / fgw.W[i] > 0.000001) {
						cout << "  DIFFERENT W["<<(i+fgw.L)<<"] = ";
						cout << setw(15)<<right<<fgw.W[i]<<"  ";
						cout << fg->weights[i]<<endl;
					}
				}
			}
			else {
				cout << "CANNOT COMPARE THE RETURNED WEIGHTS." << endl;
			}
			freeFG(fg);///*/
		}
		/*else if (0==strcmp(cmdArg, "-foxglynn-bench") && remainedArgs >= 3) {
			double start = atof(argv[argNum++]);
			double step = atof(argv[argNum++]);
			size_t count = atoi(argv[argNum++]);
			if (count <= 0) {
				cout << "Specify <start> <step> <count>" << endl;
			}
			
			vector<FoxGlynnWeights> fgws(count);
			vector<FoxGlynn*> fgs(count, nullptr);
			for (size_t n=0; n<count; n++) {
				double lambda = start + n*step;				
				FG_Weighter(lambda, epsilon, 1.0e-300, 1.0e300, fgws[n], true);
				fox_glynn(lambda, 1.0e-300, 1.0e300, epsilon, &fgs[n]);
			}
			cout << endl;
			for (size_t n=0; n<count; n++) {
				double lambda = start + n*step;				
				cout <<"  "<<setw(9)<<right<<lambda<<", ";
				cout <<setw(9)<<right<<fgws[n].L<<", ";
				cout <<setw(5)<<right<<fgws[n].R<<", ";
				cout <<setw(9)<<right<<fgs[n]->left<<", ";
				cout <<setw(5)<<right<<fgs[n]->right<<endl;
				
				freeFG(fgs[n]);
			}
			cout << endl;
		}//*/
		else if (0==strcmp(cmdArg, "-ctmc") && remainedArgs >= 1) {
			const char *ctmcFilename = argv[argNum++];
			loadedQ = LoadPrismCTMC(ctmcFilename, Q, true);
			if (loadedQ) {
				ublas::unit_vector<double> defaultPi0(Q.size1(), 0);
				pi0 = defaultPi0;
			}
		}
		else if ((0==strcmp(cmdArg, "-boost-ctmc") || 
				  0==strcmp(cmdArg, "-boost-dtmc")) && remainedArgs >= 1) 
		{
			const char *ctmcFilename = argv[argNum++];
			bool subtractDiag = (0==strcmp(cmdArg, "-boost-dtmc"));
			loadedQ = LoadBoostCTMC(ctmcFilename, Q, subtractDiag, true);
			if (loadedQ) {
				ublas::unit_vector<double> defaultPi0(Q.size1(), 0);
				pi0 = defaultPi0;
				//print_matrix(cout, Q, "Q");
				//cout << endl << endl;
			}
		}
		else if (0==strcmp(cmdArg, "-print")) {
			if (!loadedQ) {
				cout << "Load a CTMC before using the -pi0 option." << endl;
				continue;
			}
			print_matrix(cout, Q, "Q");
			cout << endl;
		}
		else if (0==strcmp(cmdArg, "-transpose")) {
			if (!loadedQ) {
				cout << "Load a CTMC before using the -pi0 option." << endl;
				continue;
			}
			Q = trans(Q);
			cout << endl;
		}
		else if (0==strcmp(cmdArg, "-mathprint")) {
			if (!loadedQ) {
				cout << "Load a CTMC before using the -pi0 option." << endl;
				continue;
			}
			PrintInMathematicaFormat(Q);
			cout << endl;
		}
		else if (0==strcmp(cmdArg, "-pi0") && remainedArgs >= 1) {
			if (!loadedQ) {
				cout << "Load a CTMC before using the -pi0 option." << endl;
				continue;
			}
			const char *pi0Filename = argv[argNum++];
			ublas::vector<double> npi0;
			ifstream pi0file(pi0Filename);
			pi0file >> npi0;
			double norm = norm_1(npi0);
			if (pi0file && npi0.size()==Q.size1() && norm > 0) {
				pi0 = npi0 / norm;
				cout << "Initial vector pi0:\n"<<pi0<<"\n"<<endl;
			}
		}
		else {
			cout << "Unrecognized argument: "<<cmdArg<<endl;
			return -1;
		}
		
	}
	
	return 0;
}







