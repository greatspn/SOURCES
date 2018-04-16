/*
 *  LinearProg.cpp
 *
 *  Problems that can be solved using linear programming
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
#include <cfloat>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
// #include "Measure.h"
// #include "ReachabilityGraph.h"
// #include "PackedMemoryPool.h"
// #include "NewReachabilityGraph.h"
#include "Semiflows.h"
#include "LinearProg.h"

//-----------------------------------------------------------------------------
#ifndef HAS_LP_SOLVE_LIB
//-----------------------------------------------------------------------------

#warning "Missing lp_solve library. Modules that need linear programming will not be compiled."

void SilvaColom88(const PN& pn) {
	cout << "Missing lpsolve library. SilvaColom88 will not work." << endl;
}

//-----------------------------------------------------------------------------
#else  // HAS_LP_SOLVE_LIB is defined 
//-----------------------------------------------------------------------------

#include <lp_lib.h>

//-----------------------------------------------------------------------------

struct SilvaColom88 {
	const PN& pn;
	const size_t N, M;
	VerboseLevel verboseLvl;

   	std::vector<REAL> row;
   	std::vector<int> col;
	lprec *lp = nullptr;

	SilvaColom88(const PN& _pn, VerboseLevel _verboseLvl)
	: pn(_pn), N(pn.plcs.size()), M(pn.trns.size()), verboseLvl(_verboseLvl)
	{ }

	~SilvaColom88() {
		if (lp) delete_lp(lp);
	}

	void initialize();

	ssize_t solve_for(const size_t *places, size_t num_pl);
};

//-----------------------------------------------------------------------------

void SilvaColom88::initialize() {
	bool warnForMarkDepArcMult = false;
	bool warnForInhibitorArcs = false;

	// Derive the incidence matrix C: N * M
	map<pair<place_t, transition_t>, int> mC;
	for (const Transition& trn : pn.trns) {
		for (int kind = 0; kind < 3; kind++) {
			if (kind == HA) {
				if (!trn.arcs[kind].empty())
					warnForInhibitorArcs = true;
				continue; // Ignore inhibitor arcs
			}
			int sign { (kind == IA) ? -1 : +1 };
			for (const Arc& arc : trn.arcs[kind]) {
				int mult(1);
				if (arc.isMultMarkingDep())
					warnForMarkDepArcMult = true;
				else
					mult = int(arc.getConstantMult());

				// cout << pn.plcs[arc.plc].name << " | " << trn.name << " = " << sign * mult << endl;
				mC[make_pair(arc.plc, trn.index)] += sign * mult;
			}
		}
	}

	// Initialize lp_solve
  	lp = make_lp(0, N + M);
  	set_add_rowmode(lp, true);

  	// Initialize the integer variables
  	for (size_t n=0; n<M+N; n++) {
  		set_int(lp, n+1, true);
  		char* name;
  		if (n < N)
  			name = const_cast<char*>(pn.plcs[n].name.c_str());
  		else
  			name = const_cast<char*>(pn.trns[n-N].name.c_str());
		set_col_name(lp, n+1, name);
   	}

   	// Add the m = m0 + C * sigma rows  ==>  m - C * sigma = m0
   	row.resize(M + N);
   	col.resize(M + N);
	std::fill(row.begin(), row.end(), 0.0);
	std::fill(col.begin(), col.end(), 0);
   	for (const Place& plc : pn.plcs) {
   		int m0 = plc.initMarkFn->Evaluate();
   		int j = 0;

   		// Add:  m
   		col[j] = int(plc.index) + 1;
   		row[j] = 1;
   		j++;

   		// Add:  -C * sigma
   		auto it = mC.lower_bound(make_pair(plc.index, transition_t(0)));
   		for (; it != mC.end(); ++it) {
   			if (it->first.first != plc.index)
   				break;
   			transition_t t = it->first.second;
   			int mult { it->second };

   			col[j] = int(t) + N + 1;
   			row[j] = -mult;
   			j++;
   		}

   		add_constraintex(lp, j, row.data(), col.data(), ROWTYPE_EQ, m0);
		std::fill(row.begin(), row.begin() + j, 0.0);
		std::fill(col.begin(), col.begin() + j, 0);
   	}

   	if (warnForInhibitorArcs)
   		cout << "Inhibitor arcs have been ignored." << endl;
   	if (warnForMarkDepArcMult)
   		cout << "Marking-dependent arcs cannot be evaluated. Using multiplicity of 1." << endl;

   	// Set objective function
   	set_add_rowmode(lp, false);
   	set_maxim(lp);
   	// write_LP(lp, stdout);
}

//-----------------------------------------------------------------------------

ssize_t SilvaColom88::solve_for(const size_t *places, size_t num_pl)
{	
	for (size_t p=0; p<num_pl; p++) {
		col[0] = places[p] + 1;
	   	row[0] = 1;
	}
   	set_obj_fnex(lp, num_pl, row.data(), col.data());

   	// Reset the basis. This step is very important to improve the numerical stability.
   	default_basis(lp); 
   	// Solve the ILP
   	set_verbose(lp, IMPORTANT);
   	int result = solve(lp);

	// cout << "exitcode=" << result << "  solution=" << get_objective(lp) << " - " << ((ssize_t)get_objective(lp)) << endl;
   	if (result == OPTIMAL || result == SUBOPTIMAL) {
#ifdef DEBUG
   		ssize_t sum = 0;
   		REAL* pVars;
	   	get_ptr_variables(lp, &pVars);
	   	for (size_t p=0; p<num_pl; p++) 
	   		sum += pVars[places[p]];
	   	ssize_t object = (ssize_t)get_objective(lp);
	   	assert((ssize_t)get_objective(lp) == sum);
#endif
	   	// return sum;
   		return (ssize_t)ceil(get_objective(lp)); // Use ceil, otherwise sometimes it will truncate the values!!
   	}
   	else if (result == UNBOUNDED) 
   		return ILPBND_UNBOUNDED;
   	else 
   		return ILPBND_UNKNOWN;
}

//-----------------------------------------------------------------------------

void ComputeILPBounds(const PN& pn, 
				      const place_bounds_t* opt_bnds,
				      place_ilp_bounds_t& bounds,
				      VerboseLevel verboseLvl)
{
	const size_t N = pn.plcs.size();
	// const size_t M = pn.trns.size();
	bounds.resize(N);
	assert(opt_bnds == nullptr || opt_bnds->size() == N);

	SilvaColom88 sc88(pn, verboseLvl);
	sc88.initialize();
	
   	size_t num_unbounded = 0;
   	size_t num_unknown = 0;
   	// write_LP(lp, stdout);

	std::vector<size_t> targetPlc(1);
   	for (size_t targetP=0; targetP<N; targetP++) {
   		if (opt_bnds != nullptr && (*opt_bnds)[targetP].is_upper_bounded()) {
   			bounds[targetP] = ILPBND_SKIPPED;
   			continue;
   		}

   		targetPlc[0] = targetP;
   		bounds[targetP] = sc88.solve_for(targetPlc.data(), 1);

   		switch (bounds[targetP]) {
   			case ILPBND_UNBOUNDED:
   				if (verboseLvl > VL_BASIC)
		   			cout << "Place "<<pn.plcs[targetP].name << " is unbounded." << endl;
		   		num_unbounded++;;
   				break;

			case ILPBND_UNKNOWN:
				if (verboseLvl > VL_BASIC)
			   		cout << "Could not compute bound of place "<<pn.plcs[targetP].name << endl; //". error code=" << result << endl;   		   		
			   	num_unknown++;
				break;

			default:
				if (verboseLvl > VL_BASIC)
	   				cout << "Bound of "<<pn.plcs[targetP].name << " is " << bounds[targetP] << endl;
   		}
	}

	if (verboseLvl >= VL_BASIC) {
		if (num_unbounded > 0)
			cout << "There are " << num_unbounded << " unbounded places." << endl;
		if (num_unknown > 0)
			cout << "There are " << num_unknown << " places with an unknown bound." << endl;
		if (num_unknown==0 && num_unbounded==0)
			cout << "All places are bounded." << endl;
	}
}

//-----------------------------------------------------------------------------

int isnotdblquotes(int ch) { return ch != '\"'; }

void UpperBoundsMCC(const PN& pn, VerboseLevel verboseLvl, ifstream& UB) {
	simple_tokenizer st(&UB);
	std::vector<size_t> targetPlc;

	SilvaColom88 sc88(pn, verboseLvl);
	sc88.initialize();
	
	while (UB && st.peek()=='%') {
		targetPlc.resize(0);
		string tag, buf;
		int ch;
		st.readtok("%");
		while (st.peek() == ' ') 
			st.get(ch);
		st.readtok("\"");
		st.gettok(tag, isnotdblquotes);
		st.readtok("\"");
		st.readtok("bounds(");
		do {
			st.readtok("\"");
			st.gettok(buf, isnotdblquotes);
			// cout << "<" << buf << ">" << endl;
			auto plcIt = pn.plcInds.find(buf);
			if (plcIt == pn.plcInds.end())
				throw program_exception("Unknown place.");
			targetPlc.push_back(plcIt->second);
			st.readtok("\"");
			while (st && isspace(st.peek()))
				st.get(ch);
		}
		while (st.get(ch) && ch == ',');
		st.skiplines();
		// cout << endl;

		// Compute the upper bound
		ssize_t result = sc88.solve_for(targetPlc.data(), targetPlc.size());

		cout << "FORMULA " << tag << " ";
		switch (result) {
			case ILPBND_UNKNOWN:
				cout << "?";
				break;

			case ILPBND_UNBOUNDED:
				cout << "inf";
				break;

			default:
				cout << result;
		}
		cout << " TECHNIQUES SAT_SMT SEQUENTIAL_PROCESSING UNFOLDING_TO_PT" << endl;
	}
	return;
}

//-----------------------------------------------------------------------------

void SaveILPBounds(const place_ilp_bounds_t& bounds, ofstream& file) {
	size_t num_bnds = 0;
	for (size_t i=0; i<bounds.size(); i++)
		if (bounds[i] != ILPBND_SKIPPED)
			num_bnds++;

	file << num_bnds << "\n";
	for (size_t i=0; i<bounds.size(); i++) {
		if (bounds[i] == ILPBND_SKIPPED)
			continue; // skip this bound

		file << i << " ";
		if (bounds[i] == ILPBND_UNKNOWN)
			file << "?\n";
		else if (bounds[i] == ILPBND_UNBOUNDED)
			file << "inf\n";
		else
			file << bounds[i] << "\n";
	}
	file << "0\n" << flush;
}

//-----------------------------------------------------------------------------
#endif // HAS_LP_SOLVE_LIB
//-----------------------------------------------------------------------------

