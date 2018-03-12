
#include <cmath>
#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>
using namespace std;

#include <boost/utility.hpp>   // for boost::noncopyable

#include "numeric.h"

using namespace numerical;

//-----------------------------------------------------------------------------

bool numerical::FG_Finder(const double lambda, const double epsilon,
						  const double tau, const double omega,
						  FoxGlynnWeights& fgw, double& W_M, VerboseLevel verboseLvl)
{
	assert(lambda > 0.0);
	const int M = int(floor(lambda)); // Mode of lambda
	
	//----------------------------------------------------------
	// STEP 1) Find the right truncation point R
	//----------------------------------------------------------
	
	// Compute the right truncation point R. If lambda<400, use lambda=400.
	const double R_lambda = max(400.0, lambda);
	
	// Find R with Corollary 1, using R_lambda.
	// The sum of the probabilities pi(R)..pi(inf) is Q_lambda(R)
	// If  1/(2*sqrt(2)*R_lambda) <= k <= sqrt(R_lambda)/(2*sqrt(2))
	// and lambda >= 2 then holds:
	//      Q_lambda(R) = a_lambda * d(k, R_lambda) * exp(-k*k/2) / (k*sqrt(2*PI))
	// with R = ceil( M + k*sqrt(2*R_lambda) + 3/2 )
	//      a_lambda = (1 + 1/R_lambda) * exp(1/16) * sqrt(2)
	//      d(k, lambda) = 1 / (1 - exp(-2/9 * (k*sqrt(2*lambda) + 3/2)))
	// We want to obtain the minimun R s.t. Q_lambda(R) <= epsilon/2
	int kR = 3;
	const int max_kR = int(ceil( sqrt(R_lambda) / (2.0*sqrt(2.0)) )); 
	const double a_lambda = (1. + 1./R_lambda) * exp(1./16.) * sqrt(2.);
	while (kR <= max_kR) {
		double d_k_lambda = 1 / (1 - exp(-2./9. * (kR * sqrt(2.*R_lambda) + 3./2.)));
		double Q_lambda = a_lambda * d_k_lambda * exp(-(kR*kR)/2.) / (kR*sqrt(2*M_PI));
		if (Q_lambda < epsilon/2)
			break;
		kR++;
	}
	fgw.R = int(ceil(M + kR*sqrt(2. * R_lambda) + 3./2.));
	
	//----------------------------------------------------------
	// STEP 2) Find the left truncation point L
	//----------------------------------------------------------
	
	int kL = -1;
	if (lambda < 25) {
		// For small lambda (0-25) don't truncate
		fgw.L = 0;
		if (exp(-lambda) < tau) {
			// Underflows will occur
			if (verboseLvl >= VL_VERBOSE)
				cout << "FOX-GLYNN Underflow: lambda="<<lambda<<" too small." << endl;
			return false;
		}
	}
	else { // lambda >= 25
		// Find L with Corollary 2, using the actual lambda.
		// The sum of the first probabilities pi(0)..pi(L) is T_lambda(L).
		// If lambda >= 2 and k >= 1/sqrt(2*lambda) then holds:
		//      T_lambda(L) <= b_lambda * exp(-k*k/2) / (k*sqrt(2*PI))
		// with L = floor(M - k*sqrt(lambda) - 3/2)
		//      b_lambda = (1 + 1/lambda) * exp(1/(8*lambda))
		// We want to obtain the maximum L s.t. T_lambda(L) <= epsilon/2
		
		kL = 3;
		const int max_kL = M; // FIXME: not sure of this
		const double b_lambda = (1. + 1./lambda) * exp(1. / (8.*lambda));
		while (kL <= max_kL) {
			double T_lambda = b_lambda * exp(-kL*kL / 2.) / (kL*sqrt(2*M_PI));
			if (T_lambda < epsilon/2)
				break;
			kL++;
		}
		fgw.L = int(floor(M - kL*sqrt(lambda) - 3./2.));
		fgw.L = max(0, fgw.L);
	}
	
	//----------------------------------------------------------
	// STEP 3) Derive the base weight w(M)
	//----------------------------------------------------------
	// Simply apply the formula in section 3.
	
	W_M = omega / (1.e10 * (fgw.R - fgw.L));
	
	
	//----------------------------------------------------------
	// STEP 4) Evaluate the lower bound L to avoid underflows
	//----------------------------------------------------------
	
	// Let cm = (1 / sqrt(2*PI*M)) * exp(M - lambda - 1/(12*m))
	// Holds that:  pi(m) >= cm
	// For lambda >= 25 holds:  cm >= 0.02935 / sqrt(M)
	const double min_cm = 0.02935 / sqrt(double(M));
	
	// Corollary 4 verifies if pi(L) will underflow.
	// Let top_kL = kL + 3/(2*sqrt(lambda))
	// For 0 < top_kL <= sqrt(lambda)/2 holds:
	//   pi(L) >= cm * exp(-top_kL^2 / 2 - top_kL^3 / (3*sqrt(lambda)))
	// Else for top_kL <= sqrt(M+1)/M holds:
	//   pi(L) >= max( cm * (1 - top_kL / sqrt(M+1)) ^ (top_kL*sqrt(M+1)),
	//                 exp(-lambda) )
	// The minimum Poisson value must be compared with w(m), to check 
	// possible underflows of pi(L). We use w(m) = omega / (1.0e10 * (R-L))
	if (lambda >= 25) {
		const double top_kL = kL + 3./(2. * sqrt(lambda));		
		double min_poisson = 1.0;
		
		if (top_kL <= sqrt(lambda) / 2.) {
			min_poisson = min_cm * exp(-top_kL*top_kL / 2.
									   -top_kL*top_kL*top_kL / (3.*sqrt(lambda)));
		}
		else if (top_kL <= sqrt(M+1.0)/M) {
			min_poisson = max(min_cm * pow(1 - top_kL / sqrt(M+1.0), 
										   top_kL*sqrt(M+1.0)), 
							  exp(-lambda));
		}
		else {
			// This happens only for small lambda (25-150 with epsilon 1e-7)
			// In this case, the left tail should never underflow.
		}
		
		if (min_poisson * W_M <= tau) {
			if (verboseLvl >= VL_VERBOSE)
				cout << "FOX-GLYNN: left Poisson tail will underflow." << endl;
			
			return false;
		}
	}
	
	
	//----------------------------------------------------------
	// STEP 5) Evaluate the upper bound R to avoid underflows
	//----------------------------------------------------------
	
	// Corollary 3 verifies if pi(R) will underflow.
	// Let top_kR = kR*sqrt(2) + 3/(2*sqrt(lambda)). Then for kR>0 holds:
	//   pi(R) >= cm * exp(-(top_kR + 1)^2 / 2)
	// The minimum Poisson value must be compared with w(m), to check 
	// possible underflows of pi(R). We use w(m) = omega / (1.0e10 * (R-L))
	if (lambda >= 400) {
		const double top_kR = kR*sqrt(2.) + 3./(2.*sqrt(lambda));
		double min_poisson = min_cm * exp(-pow(top_kR+1., 2.) / 2.);
		
		if (min_poisson * W_M <= tau) {
			if (verboseLvl >= VL_VERBOSE)
				cout << "FOX-GLYNN: right Poisson tail will underflow." << endl;
			return false;
		}
	}
	
	return true;
}



//-----------------------------------------------------------------------------

bool numerical::FG_Weighter(const double lambda, const double epsilon,
							const double tau, const double omega,
							FoxGlynnWeights& fgw, VerboseLevel verboseLvl)
{
	if (lambda <= 0.0) {
		if (verboseLvl >= VL_VERBOSE)
			cout << "FOX-GLYNN: Invalid lambda="<<lambda<<" value." << endl;
		return false;
	}
	
	const int M = int(floor(lambda)); // Mode of lambda
	
	// Find the left L and right R Poisson tails
	double W_M;
	if (!FG_Finder(lambda, epsilon, tau, omega, fgw, W_M, verboseLvl ))
		return false;
	int numWeights = fgw.R - fgw.L + 1;
	fgw.W.resize(numWeights);
	fgw.W[M - fgw.L] = W_M;
	
	// Starting from mode M, compute Poisson weights recursively.
	// Fill the left tail: w[M-1]...w[L]
	for (int j=M; j>fgw.L; j--)
		fgw.W[j - fgw.L - 1] = (j / lambda) * fgw.W[j - fgw.L];
	
	// Fill the right tail: w[M+1]...w[R]
	if (lambda < 400) {
		// Check for underflows
		if (fgw.R > 600) {
			if (verboseLvl >= VL_VERBOSE)
				cout << "FOX-GLYNN: Underflow is possible (R>600)." << endl;
			return false;
		}
		// Compute right weights
		for (int j=M; j<fgw.R; j++) {
			double q = lambda / (j+1.);
			if (fgw.W[j - fgw.L] > tau/q)
				fgw.W[j - fgw.L + 1] = q * fgw.W[j- fgw.L];
			else {
				// Ignore subsequent Right Poisson values
				fgw.R = j;
				numWeights = fgw.R - fgw.L + 1;
				fgw.W.resize(numWeights);
			}
		}
	}
	else { // lambda >= 400
		for (int j=M; j<fgw.R; j++)
			fgw.W[j - fgw.L + 1] = (lambda / (j+1.)) * fgw.W[j - fgw.L];
	}
	
	// Compute the sum of weights SumW.
	fgw.SumW = 0.0;
	int s = 0, t = fgw.R - fgw.L;
	while (s <= t) {
		// Add small terms first to reduce roundoffs.
		const int i = (fgw.W[s] <= fgw.W[t]) ? s++ : t--;
		fgw.SumW += fgw.W[i];
	}
	
	return true;
}


