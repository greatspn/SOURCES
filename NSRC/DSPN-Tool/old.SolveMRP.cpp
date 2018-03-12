/*
 *  SolveMRP.cpp
 *  
 *  Implementation of the Markov Regenerative Process numerical solvers.
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

#include "../numeric/numeric.h"
using namespace numerical;

#include <boost/shared_ptr.hpp>
using namespace boost;

#include "utils.h"
#include "sparsevector.h"
#include "ParserDefs.h"
#include "PetriNet.h"
#include "Measure.h"
#include "ReachabilityGraph.h"
#include "Isomorphism.h"
#include "MRP.h"
#include "SolveMRP.h"

//=============================================================================

const char *EmcStateKind_names[] = {
	"Absorbing State",
	"No Deterministics enabled",
	"Only Deterministics enabled",
	"Deterministic concurrently but not competitively enabled",
	"SMC Initial State"
};

//=============================================================================

const emcstate_t NOT_EMC_STATE = emcstate_t(-3);

const smcstate_t NOT_SMC_STATE = smcstate_t(size_t(-9));

//=============================================================================

/*
void ExtractMRP(const RG& rg, MRP& mrp, VerboseLevel verboseLvl)
{
	if (rg.rgClass != RGC_MarkovRenewalProcess)
		throw program_exception("Call ExtractMRP only for MRPs");
	
	const size_t N = rg.markSet.size();
	
	numerical::matrix_map<double> Q(N, N);
	numerical::matrix_map<double> Qbar(N, N);
	numerical::matrix_map<double> Delta(N, N);
	
	// A Deterministic state partition has a single fixed delay in every state
	map< transition_t, double > numDetSubsets;
	vector<double> allDetDelays(N, -1.0);
	
	// Fill in the generator matrices Q, Qbar and Delta
	vector<ReachableMarking>::const_iterator rmIt;
	for (rmIt = rg.markSet.begin(); rmIt != rg.markSet.end(); ++rmIt) {
		size_t i = size_t(rmIt->index);
		double Qii = 0.0, detDelay = -1;
		vector<RGArc>::const_iterator arcIt = rmIt->timedPaths.begin();
		for (; arcIt != rmIt->timedPaths.end(); ++arcIt) {
			size_t j = size_t(arcIt->destMarking);
			if (arcIt->distrib == DET) {
				// Store separately the branching probability and the delay
				Delta(i, j) += arcIt->prob;
				detDelay = arcIt->delay;
				
				assert(allDetDelays[rmIt->index] < 0 || 
					   allDetDelays[rmIt->index] == detDelay);
				allDetDelays[rmIt->index] = detDelay;
			}
			else {
				double rate = arcIt->delay * arcIt->prob;
				if (arcIt->preempted.size() != 0)
					Qbar(i, j) += rate;
				else 
					Q(i, j) += rate;
				Qii += rate;
			}
		}
		Q(i, i) += -Qii;
		
		// Count the dimensions of the state space subset
		if (rmIt->enabledDetInds.size() != 0) 
		{
			assert(rmIt->enabledDetInds.size() == 1);
			assert(detDelay != -1.0);
			if (numDetSubsets.count(rmIt->enabledDetInds[0]) == 0)
				numDetSubsets[rmIt->enabledDetInds[0]] = detDelay;
			else {
				if (numDetSubsets[rmIt->enabledDetInds[0]] != detDelay)
					throw program_exception
					("Inside a S^Det subsets the deterministic transition"
					 "delay should be fixed.");				
			}
		}
	}
	assert(numDetSubsets.size() > 0);
	
	// Compress the 3 mapped matrices
	compress_matrix(mrp.Q, Q);
	compress_matrix(mrp.Qbar, Qbar);
	compress_matrix(mrp.Delta, Delta);
	
	// Set up the initial vector pi0
	mrp.pi0.resize(N);
	fill(mrp.pi0.begin(), mrp.pi0.end(), 0.0);
	vector<RGArc>::const_iterator arcIt = rg.initMark.timedPaths.begin();
	for (; arcIt != rg.initMark.timedPaths.end(); ++arcIt)
		mrp.pi0[arcIt->destMarking] += arcIt->prob;
	
	// Prepare the state space partitions
	vector<statesubset_t> markToSubset;
	markToSubset.resize(N);
	
	mrp.Det.resize(numDetSubsets.size());
	
	// Iterate over each combination of <detTrnInd, delay> pairs
	map< transition_t, double >::const_iterator udt;
	statesubset_t detSubsetCnt = 0;
	map< pair<transition_t, double>, statesubset_t> remapDetInds;
	for (udt = numDetSubsets.begin(); udt != numDetSubsets.end(); ++udt) {
		DetMarkingSubset& dms = mrp.Det[detSubsetCnt];
		dms.index     = detSubsetCnt;
		dms.detTrnInd = udt->first;
		dms.delay     = udt->second;	
		remapDetInds[ *udt ] = detSubsetCnt;
		detSubsetCnt++;
	}
	for (rmIt = rg.markSet.begin(); rmIt != rg.markSet.end(); ++rmIt) {
		if (rmIt->enabledDetInds.size() == 0) {
			// Add in the exponential states set
			markToSubset[rmIt->index] = EXP_SUBSET;
		}
		else {
			// Add *rmIt in the proper Deterministic states subset
			transition_t detTrn = rmIt->enabledDetInds[0];
			double detDelay = allDetDelays[rmIt->index];
			assert(detDelay >= 0);
			statesubset_t remappedInd = remapDetInds[make_pair(detTrn, detDelay)];
			
			// Verify that this state is not connected with other Det states
			// with different delays (i.e. the delay of the deterministic 
			// transition must be fixed inside a single S^Det subset)
			vector<RGArc>::const_iterator arcIt = rmIt->timedPaths.begin();
			for (; arcIt != rmIt->timedPaths.end(); ++arcIt) {
				// We are interested only in exponential non-preempting arcs
				if (arcIt->distrib == EXP && arcIt->preempted.size() == 0) {
					marking_t destMark = arcIt->destMarking;
					assert(allDetDelays[destMark] > 0);
					assert(rg.markSet[destMark].enabledDetInds.size() == 1);
					assert(rg.markSet[destMark].enabledDetInds[0] == detTrn);
					
					transition_t destDetTrn = rg.markSet[destMark].enabledDetInds[0];
					double destDelay = allDetDelays[arcIt->destMarking];
					assert(destDetTrn == detTrn);
					assert(remapDetInds.count(make_pair(destDetTrn, destDelay)) > 0);
					if (remappedInd != remapDetInds[make_pair(destDetTrn, destDelay)])
						throw program_exception
						("Inside a S^Det subsets the deterministic transition"
						 "delay should be fixed.");
				}
			}
			markToSubset[rmIt->index] = remappedInd;
		}
	}
	mrp.Subsets.ConstructSubsetList(mrp.Det.size()+2, markToSubset);
	
	if (verboseLvl >= VL_VERBOSE) 
		PrintMRP(mrp);
}

//=============================================================================

void PrintMRP(const MRP& mrp) 
{
	if (mrp.N() < marking_t(50)) {
		cout << "MARKOV REGENERATIVE PROCESS MATRICES:\n\n";
		print_matrix(cout, mrp.Q, "Q");
		cout << endl;
		print_matrix(cout, mrp.Qbar, "Qbar");
		cout << endl;
		print_matrix(cout, mrp.Delta, "Delta");
		cout << endl;
		cout << "pi0 = " << print_vec(mrp.pi0) << endl;
	}
	cout << "EXPONENTIAL STATES SET:\n   S^Exp = ";
	cout << print_iter_1based(mrp.Subsets.beginSubset(EXP_SUBSET),
							  mrp.Subsets.endSubset(EXP_SUBSET)) << endl;
	cout << "DETERMINISTIC STATES SETS:\n";
	for (size_t d=0; d<mrp.Det.size(); d++) {
		cout << "   S^Det["<<(d+1)<<"] = ";
		cout << print_iter_1based(mrp.Subsets.beginSubset(d),
								  mrp.Subsets.endSubset(d));
		cout << "   with transition_index=";
		cout << size_t(mrp.Det[d].detTrnInd) + 1;
		cout << ", delay=" << mrp.Det[d].delay << "\n";
	}
	cout << "\nSTATE SPACE PARTITIONING:\n   ";
	for (marking_t m(0); m<mrp.Subsets.numStates(); ++m) {
		if (mrp.Subsets.subsetOf(m) == EXP_SUBSET)
			cout << "X ";
		else
			cout << mrp.Subsets.subsetOf(m) << " ";
	}
	cout << "\n" << endl;
}
*/

//=============================================================================

static
void ClassifyMRPStates(const MRP& mrp, EMC& emc, VerboseLevel verboseLvl)
{	
	const marking_t N_MRP = mrp.N();	
	emc.mrpToEmcInds.resize(N_MRP, NOT_EMC_STATE);
	emcstate_t emcStateCnt(0);
	
	// All the initial states belongs to the EMC
	for (marking_t i(0); i < N_MRP; i++)
		if (mrp.pi0(i) != 0.0)
			emc.mrpToEmcInds[i] = emcStateCnt++;

	// Analyze each state of the marking process
	for (marking_t i(0); i < N_MRP; i++) {
		if (mrp.Subsets.subsetOf(i) == EXP_SUBSET) {
			// i belongs to the EMC, and every state reached from i too
			// Note that only Q contains vaules for EXP_SUBSET states
			if (NOT_EMC_STATE == emc.mrpToEmcInds[i])
				emc.mrpToEmcInds[i] = emcStateCnt++;
			matRow_t Q_row(mrp.Q, i);
			rowIter_t Q_ij(Q_row.begin()), Q_end(Q_row.end());
			for (; Q_ij != Q_end; ++Q_ij) {
				const marking_t j(Q_ij.index());
				if (NOT_EMC_STATE == emc.mrpToEmcInds[j])
					emc.mrpToEmcInds[j] = emcStateCnt++;
			}			
			continue;
		}
		
		// Add to the EMC states reached from a Markov transition with preemption
		matRow_t Qbar_row(mrp.Qbar, i);
		rowIter_t Qbar_ij(Qbar_row.begin()), Qbar_end(Qbar_row.end());
		for (; Qbar_ij != Qbar_end; ++Qbar_ij) {
			const marking_t j(Qbar_ij.index());
			if (NOT_EMC_STATE == emc.mrpToEmcInds[j])
				emc.mrpToEmcInds[j] = emcStateCnt++;
		}
		
		// Add to the EMC states reached from a Deterministic transition firing
		matRow_t Delta_row(mrp.Delta, i);
		rowIter_t Delta_ij(Delta_row.begin()), Delta_end(Delta_row.end());
		for (; Delta_ij != Delta_end; ++Delta_ij) {
			const marking_t j(Delta_ij.index());
			if (NOT_EMC_STATE == emc.mrpToEmcInds[j])
				emc.mrpToEmcInds[j] = emcStateCnt++;
		}
	}
	
	// Allocate EMC vector descriptor and classify each state
	emc.states.resize(emcStateCnt);
	emcstate_t newEmcStateId(0);
	for (marking_t i(0); i < N_MRP; i++) {
		if (NOT_EMC_STATE != emc.mrpToEmcInds[i]) 
		{
			EMCState& emcs = emc.states[newEmcStateId];
			emcs.index    = newEmcStateId;
			emcs.mrpIndex = i;
			emc.mrpToEmcInds[i] = newEmcStateId;
			++newEmcStateId;
			
			bool isAbsorbing   = true;
			bool hasExpNoPreem = matrix_row_has_nonzeroes(mrp.Q, i, isAbsorbing);
			bool hasExpPreem   = matrix_row_has_nonzeroes(mrp.Qbar, i, isAbsorbing);
			bool hasDet        = matrix_row_has_nonzeroes(mrp.Delta, i, isAbsorbing);

			// Classify this EMC state according to its out transitions
			if (isAbsorbing)
				emcs.kind = EMCSK_ABSORBING;
			else if (!hasDet)
				emcs.kind = EMCSK_NO_DET_ENABLED;
			else if (hasDet && !hasExpPreem && !hasExpNoPreem)
				emcs.kind = EMCSK_ONLY_DET_ENABLED;
			else if (hasDet && hasExpPreem && !hasExpNoPreem)
				emcs.kind = EMCSK_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY;
			else
				emcs.kind = EMCSK_SMC;
		}
	}
	assert(newEmcStateId == emcStateCnt);
		
	if (verboseLvl >= VL_VERBOSE) {
		cout << "EMBEDDED MARKOV CHAIN STRUCTURE:\n";
		for (emcstate_t s=0; s<emcStateCnt; s++) {
			EMCState& emcs = emc.states[s];
			cout << "  EMC STATE " << left << setw(3) << size_t(s)+1 << " = M";
			cout << setw(3) << size_t(emcs.mrpIndex) + 1 << "  kind: ";
			cout << EmcStateKind_names[emcs.kind] << endl;
		}
		cout << endl;
	}
}

//=============================================================================

// Extract the SMC reachable from initStates; all the initStates must belong
// to the same state subset
void ConstructSMC(const MRP& mrp, const MarkingSubset_t& initStates,
				  SMC& smc, VerboseLevel verboseLvl)
{
	assert(initStates.size() > 0);
	const marking_t N_MRP = mrp.N();
	
	// Explore the state space that belongs to the SMC
	queue<marking_t> dfsVisitStack;
	smcstate_t internalSmcStatesCnt(0);
	smcstate_t preemptSmcStatesCnt(0);
	smc.mrpToInternalSmcInd.resize(N_MRP, NOT_SMC_STATE);
	smc.mrpToPreemptSmcInd.resize(N_MRP, NOT_SMC_STATE);
	smc.subset = mrp.Subsets.subsetOf(initStates[0]);
	
	// Start the visit in the initial state
	for (size_t i=0; i<initStates.size(); i++) {
		const marking_t visitInitState = initStates[i];
		if (smc.mrpToInternalSmcInd[visitInitState] != NOT_SMC_STATE)
			continue; // Already visited
		
		assert(smc.subset == mrp.Subsets.subsetOf(visitInitState));
		dfsVisitStack.push(visitInitState);
		smc.mrpToInternalSmcInd[visitInitState] = internalSmcStatesCnt++;
		
		while(!dfsVisitStack.empty()) {
			marking_t currMark = dfsVisitStack.front();
			dfsVisitStack.pop();
			
			// Classify neighbourhoods of currMark: we are interested only in exp arcs
			// States reached by non-preempting exponential are internal states 
			// Note that, when building an SMC for the EXP state subset, Q arcs
			// contains both internal and external arcs.
			matRow_t Q_row(mrp.Q, currMark);
			rowIter_t Q_ij(Q_row.begin()), Q_end(Q_row.end());
			for (; Q_ij != Q_end; ++Q_ij) {
				const marking_t j(Q_ij.index());
				if (smc.subset == EXP_SUBSET && mrp.Subsets.subsetOf(j) != smc.subset) {
					// A Q arc that exits the EXP SMC
					if (smc.mrpToPreemptSmcInd[j] == NOT_SMC_STATE) {
						smc.mrpToPreemptSmcInd[j] = preemptSmcStatesCnt++;
					}
				}
				else {
					assert(mrp.Subsets.subsetOf(j) == smc.subset);
					if (smc.mrpToInternalSmcInd[j] == NOT_SMC_STATE) {
						smc.mrpToInternalSmcInd[j] = internalSmcStatesCnt++;
						dfsVisitStack.push(j); // Continue DFS visit
					}
				}
			}		
		
			// States reached by preempting exponential are preemptSmc states 
			matRow_t Qbar_row(mrp.Qbar, currMark);
			rowIter_t Qbar_ij(Qbar_row.begin()), Qbar_end(Qbar_row.end());
			for (; Qbar_ij != Qbar_end; ++Qbar_ij) {
				const marking_t j(Qbar_ij.index());
				if (smc.mrpToPreemptSmcInd[j] == NOT_SMC_STATE) {
					smc.mrpToPreemptSmcInd[j] = preemptSmcStatesCnt++;
				}
			}
		}
	}
	
	// Number of SMC states (internal + external preempted)
	const smcstate_t N_SMC = (internalSmcStatesCnt + preemptSmcStatesCnt);
	smc.numInternalStates = internalSmcStatesCnt;
	const smcstate_t QbarOffset_j = smc.numInternalStates;
	
	// Now enumerate the MRP states that are used inside the SMC
	//smcstate_t smcStateCnt = 0;
	smc.smcToMrpInds.resize(size_t(N_SMC), marking_t(size_t(-2)));
	for (marking_t n=0; n<N_MRP; n++)
		if (smc.mrpToInternalSmcInd[n] != NOT_SMC_STATE)
			smc.smcToMrpInds[ smc.mrpToInternalSmcInd[n] ] = n;
	for (marking_t n=0; n<N_MRP; n++)
		if (smc.mrpToPreemptSmcInd[n] != NOT_SMC_STATE)
			smc.smcToMrpInds[ QbarOffset_j + smc.mrpToPreemptSmcInd[n] ] = n;
	//assert(smcStateCnt == K);
	
	if (verboseLvl >= VL_VERBOSE) {
		double delay = -1;
		if (smc.subset != EXP_SUBSET)
			delay = mrp.Det[smc.subset].delay;
		cout << "\n\nSUBORDINATED MARKOV CHAIN OF THE MARKING PROCESS ";
		if (initStates.size() == 1)
			cout << "STATE " << size_t(initStates[0])+1;
		else cout << "STATES " << print_vec_1based(initStates);
		if (delay >= 0)
			cout << " (at time t=" << delay << "):\n";
		else cout << " (at time t=INF):\n";
		cout << "  INTERNAL SMC STATES:  " << internalSmcStatesCnt << endl;
		cout << "  SMC PREEMPTED STATES: " << preemptSmcStatesCnt << endl;
		cout << "  MRP -> INTERNAL SMC:  ";
		cout << print_vec_1based_ifnoteq(smc.mrpToInternalSmcInd, 
										 NOT_SMC_STATE, "X") << endl;
		cout << "  MRP -> PREEMPTED SMC: ";
		cout << print_vec_1based_ifnoteq(smc.mrpToPreemptSmcInd, 
										 NOT_SMC_STATE, "X") << endl;
		cout << "  SMC -> MRP STATES:    ";
		cout << print_vec_1based(smc.smcToMrpInds) << endl;
		cout << endl;
	}	
	
	// At this point the SMC matrix and the branching probabilities can be extracted
	typedef ublas::compressed_matrix<double>::const_iterator1 row_iterator_t;
	typedef ublas::compressed_matrix<double>::const_iterator2 elem_iterator_t;
	numerical::matrix_map<double> S(N_SMC, N_SMC); 
	
	// The SMC matrix is made up by two sub-matrices:
	//           [ Q^g  | Qbar^g ]
	//      S =  [------+--------]
	//           [  0   |    0   ]
	// with Q^g and Qbar^g the subset of Q and Qbar related to the 
	// deterministic transition g of the considered SMC.
	
	// Add Q^g to te left of S
	row_iterator_t Q_rowIt(mrp.Q.begin1());
	row_iterator_t Q_rowItEnd(mrp.Q.end1());
	while (Q_rowIt != Q_rowItEnd) {
		const marking_t i = marking_t(Q_rowIt.index1());
		const smcstate_t smc_i = smc.mrpToInternalSmcInd[i];
		if (smc_i != NOT_SMC_STATE) {
			elem_iterator_t Q_ij(Q_rowIt.begin());
			elem_iterator_t iRowEnd(Q_rowIt.end());
			while (Q_ij != iRowEnd) {
				const marking_t j = marking_t(Q_ij.index2());
				if (smc.subset == EXP_SUBSET && mrp.Subsets.subsetOf(j) != smc.subset) {
					const smcstate_t smc_j = smc.mrpToPreemptSmcInd[j];
					if (smc_j != NOT_SMC_STATE) {
						S(smc_i, smc_j + smc.numInternalStates) += *Q_ij;
					}
				}
				else {
					const smcstate_t smc_j = smc.mrpToInternalSmcInd[j];
					if (smc_j != NOT_SMC_STATE) {
						S(smc_i, smc_j) += *Q_ij;
					}
				}
				++Q_ij;
			}
		}
		++Q_rowIt;
	}
	
	// Add Qbar^g to the right of S
	row_iterator_t Qbar_rowIt(mrp.Qbar.begin1());
	row_iterator_t Qbar_rowItEnd(mrp.Qbar.end1());
	while (Qbar_rowIt != Qbar_rowItEnd) {
		const marking_t i = marking_t(Qbar_rowIt.index1());
		const smcstate_t smc_i = smc.mrpToInternalSmcInd[i];
		if (smc_i != NOT_SMC_STATE) {
			elem_iterator_t Qbar_ij(Qbar_rowIt.begin());
			elem_iterator_t iRowEnd(Qbar_rowIt.end());
			while (Qbar_ij != iRowEnd) {
				const marking_t j = marking_t(Qbar_ij.index2());
				const smcstate_t smc_j = smc.mrpToPreemptSmcInd[j];
				if (smc_j != NOT_SMC_STATE) {
					S(smc_i, smc_j + smc.numInternalStates) += *Qbar_ij;
				}
				++Qbar_ij;
			}
		}
		++Qbar_rowIt;
	}
	compress_matrix(smc.S, S);
	
	if (verboseLvl >= VL_VERBOSE && size_t(N_SMC) < 50) {
		print_matrix(cout, smc.S, "   S");
		cout << endl;
	}
}	

//=============================================================================

// Compute an hash for each SMC: if two SMCs have the same hash, they may be
// isomorphic, otherwise they are surely non isomorphic.
static
void ComputeIsoClasses(const MRP& mrp, EMC& emc, 
					   vector<Signature64>& vertInvs,
					   vector<emcstate_t>& evalOrder, 
					   VerboseLevel verboseLvl)
{
	if (verboseLvl == VL_BASIC)
		cout << endl;
	
	// Compute global vertex hashes (shared by all the isoClass computations)
	ComputeMRP_VertexHashes(mrp.Q, mrp.Qbar, vertInvs);
	
	// Compute SMC subgraph invariants (the "isoClass")
	const emcstate_t N_EMC = emc.states.size();
	for (emcstate_t es=0; es<N_EMC; es++) {
		EMCState& state = emc.states[es];
		if (state.kind == EMCSK_SMC) {			
			double delay = mrp.Det[mrp.Subsets.subsetOf(state.mrpIndex)].delay;
			// Get the graph invariant for this SMC
			ComputeSMC_IsoHash(state.mrpIndex, mrp.Q, mrp.Qbar, 
							   vertInvs, delay, state.isoClass);			
			if (verboseLvl >= VL_VERBOSE) {
				cout << "  SMC "<<setw(2)<<size_t(es)+1;
				cout <<" HAS HASH: "<<state.isoClass<<endl;
			}
		}
		else state.isoClass.reset(0);
	}

	if (verboseLvl == VL_BASIC)
		cout << one_line_up() << "ISOCLASSES COMPUTED.      \n" << endl;

	// Determine the evaluation order for the EMC states
	typedef priority_queue< pair<Signature128, emcstate_t> > emcstate_prioqueue;
	emcstate_prioqueue sortedList;
	for (emcstate_t es=0; es<N_EMC; es++)
		sortedList.push(make_pair(emc.states[es].isoClass, es));
	
	evalOrder.resize(0);
	evalOrder.reserve(N_EMC);
	while (!sortedList.empty()) {
		evalOrder.push_back(sortedList.top().second);
		sortedList.pop();
	}
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << "EMC STATES WILL BE EVALUATED IN THIS ORDER:\n      ";
		for (emcstate_t es=0; es<N_EMC; es++)
			cout << " " << evalOrder[es] + emcstate_t(1);
		cout << endl << endl;
	}
}

//=============================================================================

struct IsoSMCSolution;
typedef struct IsoSMCSolution IsoSMCSolution;

struct IsoSMCSolution {
	ublas::compressed_matrix<double> S;	// previously computed SMC matrix
	double				  delay;	// delay at which the solution was computed
	Signature128		  isoClass; // SMC hash
	ublas::vector<double> pi, pi_C; // solution vectors
};


bool SolveSMC(const MRP& mrp, const EMC& emc, const EMCState& state,
			  SMC& smc, bool detectIsomorphism, IsoSMCSolution &prevSmcSol,
			  sparsevector<emcstate_t, double>& P_row,
			  sparsevector<marking_t, double>& C_row, 
			  const SolverParams& spar, VerboseLevel verboseLvl) 
{
	const smcstate_t N_SMC = smc.smcToMrpInds.size();
	
	ublas::vector<double> pi, pi_C;
	double delay = mrp.Det[smc.subset].delay;
	
	// Choose minimum between the transient and the cumulative epsilon
	double effective_epsilon = min(spar.epsilon, spar.epsilon / delay);

	// Test if we can use the solution of another isomorphic SMC
	bool isomorphicSolFound = false;
	if (detectIsomorphism &&
		(prevSmcSol.delay==delay && prevSmcSol.isoClass==state.isoClass))
	{
		// Test isomorphism of this SMC with the SMC in prevSmcSol
		vector<int> tab1to2;
		if (FindRootedDigraphIsomorphism(prevSmcSol.S, smc.S, tab1to2, verboseLvl )) 
		{
			// Reuse the previously computed pi and pi_C vectors
			pi.resize(N_SMC, 0.0);
			pi_C.resize(N_SMC, 0.0);
			for (size_t n=0; n<tab1to2.size(); n++) {
				pi[tab1to2[n]] = prevSmcSol.pi[n];
				pi_C[tab1to2[n]] = prevSmcSol.pi_C[n];
			}

#ifndef NDEBUG
			// Verify correctness of the overall isomorphism detection
			ublas::vector<double> pi2, pi2_C;
			ublas::unit_vector<double> pi0(size_t(N_SMC), 0);
			full_element_selector all_elems(N_SMC);
			if(!Uniformization(smc.S, pi0, &pi2, &pi2_C, delay, 
							   effective_epsilon, all_elems, all_elems, 
							   VL_NONE, spar.trAlgo))
				throw program_exception("Couldn't compute transient "
										"SMC probabilities.");
			/*cout << "pi="<<pi<<"\npi2="<<pi2<<endl;
			 cout << "pi_C="<<pi_C<<"\npi_C2="<<pi2_C<<endl;
			 cout << "pi-pi2 = "<<(pi-pi2)<<endl;
			 cout << "pi_C-pi2_C = "<<(pi_C-pi2_C)<<endl;*/
			for (size_t n=0; n<pi.size(); n++) { 
				if (fabs(pi[n] - pi2[n]) > spar.epsilon*10)
					throw program_exception("ISOMORPHISM TEST BUG: pi != pi2");
				if (fabs(pi_C[n] - pi2_C[n]) > spar.epsilon*10)
					throw program_exception("ISOMORPHISM TEST BUG: pi_C != pi2_C");
			}
#endif
			isomorphicSolFound = true;
		}
	}
	
	// Otherwise, compute the solution with Uniformization
	if (!isomorphicSolFound) {		
		ublas::unit_vector<double> pi0(size_t(N_SMC), 0);
		full_element_selector all_elems(N_SMC);
		if(!Uniformization(smc.S, pi0, &pi, &pi_C, delay, effective_epsilon,
						   all_elems, all_elems, VL_NONE, spar.trAlgo))
			throw program_exception("Couldn't compute transient SMC probabilities.");
	}
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << endl;
		cout << "SMC SOLUTION:  pi("<<delay<<") = "<<pi<<endl;
		cout << "    Cumulative_pi("<<delay<<") = "<<pi_C<<endl;
	}
	
	// Set up the EMC probabilities for this SMC
	for (smcstate_t s=0; s<N_SMC; s++) {
		if (s < smc.numInternalStates) {
			typedef ublas::matrix_row<const ublas::compressed_matrix<double> > MatRow;
			// Probability of being in m at time t * probability that the
			// firing of the deterministic transition goes from m to e
			marking_t mrp_i = smc.smcToMrpInds[s];
			MatRow deltaRow(mrp.Delta, mrp_i);
			MatRow::const_iterator delta_ij = deltaRow.begin();
			MatRow::const_iterator deltaRowEnd = deltaRow.end();
			for (; delta_ij != deltaRowEnd; ++delta_ij) {
				marking_t mrp_j(delta_ij.index());
				emcstate_t emc_j = emc.mrpToEmcInds[mrp_j];
				assert(emc_j != NOT_EMC_STATE);
				P_row[emc_j] += pi(s) * *delta_ij;
			}
		}
		else {
			marking_t mrp_s = smc.smcToMrpInds[s];
			emcstate_t emc_s = emc.mrpToEmcInds[mrp_s];			
			assert(emc_s != NOT_EMC_STATE);
			P_row[emc_s] += pi(s);
		}
	}
	
	// Set up the conversion factors needed to obtain the MRP stationary
	// probabilities from the EMC stationary probabilities.
	for (smcstate_t s=0; s<smc.numInternalStates; s++) {
		marking_t mrp_s = smc.smcToMrpInds[s];
		C_row[mrp_s] += pi_C(s) * delay;
	}
	
	// Prepare the IsoSMCSolution for the next iteration
	if (detectIsomorphism) {
		prevSmcSol.delay    = delay;
		prevSmcSol.isoClass = state.isoClass;
		prevSmcSol.S.swap(smc.S);
		prevSmcSol.pi.swap(pi);
		prevSmcSol.pi_C.swap(pi_C);
	}
	
	return isomorphicSolFound;
}

//=============================================================================

void SolveReducedMRP(const MRP& mrp, PetriNetSolution& sol, bool detectIsomorphism,
					 const SolverParams& spar, VerboseLevel verboseLvl)
{
	EMC emc;
	ClassifyMRPStates(mrp, emc, verboseLvl );
			
	const marking_t N_MRP(mrp.Q.size1());
	const emcstate_t N_EMC(emc.states.size());
	cout << "THE EMBEDDED MARKOV CHAIN HAS "<<N_EMC<<" STATES."<< endl;
	size_t numSmc = 0, numSmcComputed = 0, numIsoSolUsed = 0;
	for (emcstate_t i=0; i < N_EMC; i++)
		if (emc.states[i].kind == EMCSK_SMC)
			numSmc++;
	cout << numSmc << " SUBORDINATED MARKOV CHAINS WILL BE COMPUTED.\n";
	cout << endl;
	
	typedef ublas::matrix_row< const ublas::compressed_matrix<double> >  MatRow;
	typedef MatRow::const_iterator MatRowIter;
	
	// Isomorphism classes
	vector<Signature64> vertInvs;	
	vector<emcstate_t> evalOrder;
	if (detectIsomorphism) {
		cout << "COMPUTING ISOMORPHISM CLASSES..." << endl;
		clock_t startHash = clock();
		ComputeIsoClasses(mrp, emc, vertInvs, evalOrder, verboseLvl );
		double totalIsoClassTime = double(clock() - startHash) / CLOCKS_PER_SEC;
		cout << "HASH COMPUTATION REQUIRED "<<totalIsoClassTime;
		cout <<" SECONDS." << "\n\n";
	}
	else {
		evalOrder.resize(N_EMC);
		for (emcstate_t eval_i=0; eval_i < N_EMC; eval_i++)
			evalOrder[eval_i] = eval_i;
	}
	
	// Construct the two main matrices of the Markov Regenerative process:
	//   P = probabilities of the EMC states
	//   C = conversion factors (EMC -> MRP)
	numerical::matrix_map<double> P(N_EMC, N_EMC);
	numerical::matrix_map<double> C(N_EMC, N_MRP);
	long_interval_timer timer;
	
	IsoSMCSolution prevSmcSol;  // For isomorphism detection optimization
	prevSmcSol.delay = -1.0;
			
	// Fill in P and C accordingly to the EMC state kinds
	for (emcstate_t eval_i=0; eval_i < N_EMC; eval_i++) {
		emcstate_t emc_i = evalOrder[eval_i];
		EMCState& emcState = emc.states[emc_i];
		marking_t mrp_i = emcState.mrpIndex;
		
		if (emcState.kind == EMCSK_ABSORBING)
		{
			// P(i) = 0       C(i,i) = 1     It won't be normalized
			C(emc_i, mrp_i) = 1;
		}
		else if (emcState.kind == EMCSK_NO_DET_ENABLED)
		{
			// P(i) = (Q(i) / -q(i,i)) - I(i)
			// C(i,i) = 1 / -q(i,i)			
			MatRow Q_iRow(mrp.Q, mrp_i);
			double mQ_ii = - mrp.Q(mrp_i,mrp_i);
			for (MatRowIter Q_ij = Q_iRow.begin(); Q_ij != Q_iRow.end(); ++Q_ij) {
				const marking_t mrp_j(Q_ij.index());
				const emcstate_t emc_j = emc.mrpToEmcInds[mrp_j];
				if (emc_j != emc_i)
					P(emc_i, emc_j) = *Q_ij / mQ_ii;
			}
			if (mQ_ii != 0)
				C(emc_i, mrp_i) = 1 / mQ_ii;
			else { // Avoid problems with absorbing states
				throw program_exception("Should be a EMCSK_ABSORBING.");
			}
		}
		else if (emcState.kind == EMCSK_ONLY_DET_ENABLED) 
		{
			// P(i) = Delta(i)
			// C(i,i) = delay			
			double delay = mrp.Det[mrp.Subsets.subsetOf(mrp_i)].delay;
			MatRow Delta_iRow(mrp.Delta, mrp_i);
			for (MatRowIter Delta_ij = Delta_iRow.begin(); 
				 Delta_ij != Delta_iRow.end(); ++Delta_ij) 
			{
				const marking_t mrp_j(Delta_ij.index());
				const emcstate_t emc_j = emc.mrpToEmcInds[mrp_j];
				P(emc_i, emc_j) = *Delta_ij;
			}			
			C(emc_i, mrp_i) = delay;
		}
		else if (emcState.kind == EMCSK_DET_COMPETITIVELY_BUT_NOT_CONCURRENTLY)
		{
			// P(i,j) = exp(-delta * |Qbar(i)|) * Delta(i)
			// P(i,j) = (1-exp(-delta * |Qbar(i)|)) * Qbar(i,j)/(|Qbar(i)|)
			// C(i,i) = 1 - exp(-delta * |Qbar(i)|) / (|Qbar(i)|)
			// C(i,j) = 0
			double delay = mrp.Det[mrp.Subsets.subsetOf(mrp_i)].delay;
			double SumQ = -mrp.Q(mrp_i, mrp_i);
			
			const double MAX_EXP_VAL = 200.0;
			double expVal = 0.0;
			if (SumQ * delay < MAX_EXP_VAL) // Avoid underflows
				expVal = exp(-SumQ * delay);
				
			MatRow Delta_iRow(mrp.Delta, mrp_i);
			for (MatRowIter Delta_ij = Delta_iRow.begin(); 
				 Delta_ij != Delta_iRow.end(); ++Delta_ij) 
			{
				const marking_t mrp_j(Delta_ij.index());
				const emcstate_t emc_j = emc.mrpToEmcInds[mrp_j];
				P(emc_i, emc_j) += *Delta_ij * expVal;
			}
			
			MatRow Qbar_iRow(mrp.Qbar, mrp_i);
			for (MatRowIter Qbar_ij = Qbar_iRow.begin(); 
				 Qbar_ij != Qbar_iRow.end(); ++Qbar_ij) 
			{
				const marking_t mrp_j(Qbar_ij.index());
				const emcstate_t emc_j = emc.mrpToEmcInds[mrp_j];
				P(emc_i, emc_j) += (1-expVal) * (*Qbar_ij) / SumQ;
			}
			
			C(emc_i, mrp_i) = (1-expVal) / SumQ;
		}
		else if (emcState.kind == EMCSK_SMC)
		{
			numSmcComputed++;
			if (verboseLvl == VL_BASIC && timer) {
				timer.prepare_print();
				cout << "COMPUTING SMC " << numSmcComputed << "/" << numSmc << "... ";
				if (timer.get_elapsed_time() > 1.5) {
					timer.print_estimate(numSmcComputed, numSmc);
					cout << " remained.";
				}
				cout << "           " << endl;
			}
			
			// P(i) = Prob(i @ t) * Delta(i)    with transient probabilities
			// C(i) = AccumProb(i @ t)          with cumulative transient probs.
			// These lines are obtained with an intermediate SMC.
			SMC smc;
			MarkingSubset_t initEmcState(1, emcState.mrpIndex);
			ConstructSMC(mrp, initEmcState, smc, verboseLvl );			
			sparsevector<emcstate_t, double> P_row(N_EMC);
			sparsevector<marking_t, double> C_row(N_MRP);
			if(SolveSMC(mrp, emc, emcState, smc, detectIsomorphism, 
						prevSmcSol, P_row, C_row, spar, verboseLvl ))
				numIsoSolUsed++;
			
			copy_sparse_row(P, P_row, emc_i);
			copy_sparse_row(C, C_row, emc_i);
		}
	}
	
	if (verboseLvl == VL_BASIC) {
		timer.clear_any();
		cout << "ALL SMCs HAVE BEEN COMPUTED.                 "<<endl;
	}
	
	if (detectIsomorphism)
		cout << numIsoSolUsed << " ISOMORPHIC PAIRS OF SMCs FOUND.\n" << endl;

	// Compress the two matrices P and C
	ublas::compressed_matrix<double> cP, cC;
	compress_matrix(cP, P);
	compress_matrix(cC, C);
	cout << "P MATRIX ("<<P.size1()<<" x "<<P.size2()<<") ";
	cout << "HAS "<<P.nnz()<<" NONZERO ENTRIES."<<endl;
	cout << "C MATRIX ("<<C.size1()<<" x "<<C.size2()<<") ";
	cout << "HAS "<<C.nnz()<<" NONZERO ENTRIES."<<endl<<endl;
	P.clear();
	C.clear();
	
	// Get the initial vector pi0 of the EMC
	ublas::vector<double> pi0_emc(N_EMC, 0.0);
	for (marking_t n=0; n<N_MRP; n++) {
		if (mrp.pi0(n) != 0.0) {
			emcstate_t emc_n = emc.mrpToEmcInds[n];
			assert(emc_n != NOT_EMC_STATE);
			pi0_emc(emc_n) += mrp.pi0(n);
		}
	}
	
	if (verboseLvl >= VL_VERBOSE && N_MRP < marking_t(50)) {
		cout << "\n\n\nEMBEDDED DTMC P AND CONVERSION FACTORS C MATRICES:\n\n";
		print_matrix(cout, cP, "P");
		cout << endl;	
		print_matrix(cout, cC, "C");
		cout << endl;
		cout << "INITIAL EMC VECTOR pi[0] = " << pi0_emc << endl;
		cout << endl;
	}
	
	cout << "COMPUTING STEADY STATE SOLUTION OF THE EMBEDDED DTMC...\n" << endl;
	
	// Solve the Embedded EMC
	ublas::vector<double> emcSol;	
	if (!Solve_SteadyState_DTMC(cP, emcSol, pi0_emc, spar, VL_VERBOSE))
		throw program_exception("Solution method does not converge.");
		
	// Convert the EMC solution to the MRP solution with the C factors
	full_element_selector emc_elems(N_EMC), mrp_elems(N_MRP);
	prod_vecmat(sol.stateProbs, emcSol, cC, emc_elems, mrp_elems);
		
	// At this point sol.stateProbs contains the mean times in the MRP states,
	// before state transitions: normalize these mean times into probabilities.
	// Note that the normalization process must be done separately for each BSCC,
	// because each BSCC may have a different normalization coefficient. Also,
	// absorbing states requires a special treatment (because they have, at 
	// least in theory, an infinite mean time).
	vector<int> bscc;
	int numBscc = BottomStronglyConnectedComponents(mrp.Q + mrp.Qbar + mrp.Delta, bscc);
	for (int nc=-1; nc<numBscc; nc++) {
		double total_bscc_prob = 0.0;
		for (marking_t m = 0; m < N_MRP; m++) {
			if (bscc[m] == nc) {
				if (emc.mrpToEmcInds[m] != NOT_EMC_STATE) {
					total_bscc_prob += emcSol(emc.mrpToEmcInds[m]);
				}
			}
		}
		// Normalize this BSCC. 
		// Note that absorbing states of the EMC should be excluded 
		// from the normalization process (i.e. simply use the absorption probability)
		double sumNonAbs = 0.0, sumAbs = 0.0;
		for (marking_t m = 0; m < N_MRP; m++) {
			if (bscc[m] != nc)
				continue;
			emcstate_t emc_m = emc.mrpToEmcInds[m];
			bool isAbsorbingState = (emc_m != NOT_EMC_STATE && 
									 emc.states[emc_m].kind == EMCSK_ABSORBING);
			if (isAbsorbingState)
				sumAbs += sol.stateProbs[m];
			else sumNonAbs += sol.stateProbs[m];
		}
		if (sumNonAbs > 0.0) {
			// Normalize non-absorbing states
			double NormCoeff = (1.0 - sumAbs) / sumNonAbs;
			NormCoeff *= total_bscc_prob;
			for (marking_t m = 0; m < N_MRP; m++) {
				if (bscc[m] != nc)
					continue;
				emcstate_t emc_m = emc.mrpToEmcInds[m];
				bool isAbsorbingState = (emc_m != NOT_EMC_STATE && 
										 emc.states[emc_m].kind == EMCSK_ABSORBING);
				if (!isAbsorbingState)
					sol.stateProbs[m] *= NormCoeff;
			}
		}
	}
	
	/*/ Normalize. Note that absorbing states of the EMC should be excluded 
	// from the normalization process (i.e. simply use the absorption probability)
	double sumNonAbs = 0.0, sumAbs = 0.0;
	for (marking_t m = 0; m < N_MRP; m++) {
		emcstate_t emc_m = emc.mrpToEmcInds[m];
		bool isAbsorbingState = (emc_m != NOT_EMC_STATE && 
								 emc.states[emc_m].kind == EMCSK_ABSORBING);
		if (isAbsorbingState)
			sumAbs += sol.stateProbs[m];
		else sumNonAbs += sol.stateProbs[m];
	}
	if (sumNonAbs > 0.0) {
		// Normalize non-absorbing states
		double NormCoeff = (1.0 - sumAbs) / sumNonAbs;
		for (marking_t m = 0; m < N_MRP; m++) {
			emcstate_t emc_m = emc.mrpToEmcInds[m];
			bool isAbsorbingState = (emc_m != NOT_EMC_STATE && 
									 emc.states[emc_m].kind == EMCSK_ABSORBING);
			if (!isAbsorbingState)
				sol.stateProbs[m] *= NormCoeff;
		}
	}*/
	sol.stateProbs /= norm_1(sol.stateProbs);
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << "\nEMC SOLUTION: " << emcSol << endl;
		cout << "MRP SOLUTION: " << sol.stateProbs << endl;
		cout << endl;
	}
}

//=============================================================================

/*static void 
SaveInitTrnAsDOT(const string& NetName, const MRP& mrp,
				 const vector<int> Q_SCC,
				 const ublas::compressed_matrix<double>& Xi,
				 const vector<RegenerativeComponent>& components,
				 const vector<regencomp_t>& mark2comp)
{
	typedef ublas::compressed_matrix<double>::const_iterator1 row_iterator_t;
	typedef ublas::compressed_matrix<double>::const_iterator2 elem_iterator_t;
	const regencomp_t numRegenComps(components.size());
	const marking_t N_MRP(mrp.N());
	
	// Open the DOT file
	string DotName(NetName);
	DotName += "-NEMRP.dot";
	ofstream dot(DotName.c_str());
	
	dot << "digraph G {\n\n";
	dot << "  fontname=\"Arial\";\n";
	dot << "  rankdir=LR;\n";
	dot << "  init[shape=none, label=<>];\n";
	dot << "  node[shape=rectangle, fontname=\"Arial\"];\n";
	
	// Print component nodes
	typedef pair<statesubset_t, taubsccindex_t>  component_ind_t;
	const size_t MAX_LINE_ELEM = max(size_t(4), size_t(sqrt(double(N_MRP))/3));
	for (regencomp_t c(0); c<numRegenComps; ++c) {
		statesubset_t subset = components[c].subset;
		taubsccindex_t tauBscc = components[c].tauBscc;
		
		dot << "comp"<<c<<" [label=\"Comp"<<int(c)+1<<": ";
		if (subset == EXP_SUBSET)
			dot << "EXP Δ=∞\\n";
		else dot << "DET("<<subset<<") Δ="<<mrp.Det[subset].delay<<"\\n";
		if (tauBscc == taubsccindex_t(-1))
			dot << "Transient\\n\\n";
		else
			dot << "Recurrent ("<<tauBscc<<")\\n\\n";
		for (size_t m=0, cnt=0; m<N_MRP; m++) {
			if (mark2comp[m] == c) {
				if (cnt==MAX_LINE_ELEM) { cnt=0; dot<<"\\n"; }
				else if (cnt > 0) { dot<<", "; }
				cnt++;
				dot << "m"<<size_t(m)+1;
			}
		}
		dot << "\\n\\n";
		set<sccindex_t> usedScc;
		for (size_t m=0, cnt=0; m<N_MRP; m++) {
			if (mark2comp[m] == c && usedScc.count(Q_SCC[m]) == 0) {
				usedScc.insert(Q_SCC[m]);
				if (cnt==MAX_LINE_ELEM) { cnt=0; dot<<"\\n"; }
				else if (cnt > 0) { dot<<", "; }
				cnt++;
				dot << "scc"<<Q_SCC[m];
			}
		}
		dot << "\" color="<<(tauBscc == taubsccindex_t(-1) ? "blue" : "black")<<"];\n";
	}
	// Print arcs between components (Xi matrix)
	row_iterator_t Xi_rowIt(Xi.begin1());
	row_iterator_t Xi_rowItEnd(Xi.end1());
	while (Xi_rowIt != Xi_rowItEnd) {
		const regencomp_t comp_i(Xi_rowIt.index1());
		elem_iterator_t Xi_ij(Xi_rowIt.begin());
		elem_iterator_t iRowEnd(Xi_rowIt.end());
		while (Xi_ij != iRowEnd) {
			const regencomp_t comp_j(Xi_ij.index2());
			dot << "comp"<<comp_i<<" -> comp"<<comp_j;
			dot << " [color=black, style=bold];\n";			
			++Xi_ij;
		}
		++Xi_rowIt;
	}		
	
	set<regencomp_t> pi0comp;
	for (marking_t m=0; m<N_MRP; m++) {
		regencomp_t comp_m = mark2comp[m];
		if (mrp.pi0(m) > 0 && pi0comp.count(comp_m) == 0) {
			dot << "  init -> comp"<<comp_m<<" [len=0.5, color=dodgerblue4];\n";
			pi0comp.insert(comp_m);
		}
	}
	
	dot << "\n}\n";
	dot.close();
	
	ostringstream DotCmd;
	DotCmd << "dot -Tpdf " << NetName << "-NEMRP.dot ";
	DotCmd << "-o " << NetName << "-NEMRP.pdf 2> /dev/null";
	cout << "USING DOT TOOL..." << endl;
	cout << DotCmd.str() << endl;
	system(DotCmd.str().c_str());
	
	ostringstream DotOpenCmd;
	DotOpenCmd << "open " << NetName << "-NEMRP.pdf";
	system(DotOpenCmd.str().c_str());
}*/

//=============================================================================

// Returns the number of non-trivial BSCCs inside Tau
static taubsccindex_t 
ExtractTau(const MRP& mrp,  const vector<int>& Q_SCC, vector<int>& TauBSCC,
		   ublas::compressed_matrix<double>& Tau, VerboseLevel verboseLvl)
{
	typedef ublas::compressed_matrix<double>::const_iterator1 row_iterator_t;
	typedef ublas::compressed_matrix<double>::const_iterator2 elem_iterator_t;
	const sccindex_t numQ_SCC(Q_SCC.size());
	numerical::matrix_map<double> mmTau(numQ_SCC, numQ_SCC);
	for (size_t mat=0; mat<3; mat++) {
		const ublas::compressed_matrix<double> *pM = NULL;
		switch (mat) {
			case 0:	pM = &mrp.Q;     break;
			case 1:	pM = &mrp.Qbar;  break;
			case 2:	pM = &mrp.Delta; break;
		}
		
		row_iterator_t M_rowIt(pM->begin1());
		row_iterator_t M_rowItEnd(pM->end1());
		while (M_rowIt != M_rowItEnd) {
			const marking_t i = marking_t(M_rowIt.index1());
			const sccindex_t scc_i = Q_SCC[i];
			
			elem_iterator_t M_ij(M_rowIt.begin());
			elem_iterator_t iRowEnd(M_rowIt.end());
			while (M_ij != iRowEnd) {
				const marking_t j = marking_t(M_ij.index2());
				const sccindex_t scc_j = Q_SCC[j];				
				mmTau(scc_i, scc_j) = 1;				
				++M_ij;
			}
			++M_rowIt;
		}		
	}
	
	// Obtain the BSCCs of Tau
	compress_matrix(Tau, mmTau);
	const taubsccindex_t numTauBSCC(BottomStronglyConnectedComponents(Tau, TauBSCC));
	
	if (verboseLvl >= VL_VERBOSE) {
		mmTau.clear();
		print_matrix(cout, Tau, "Tau");
		cout << endl;
		cout << "THERE ARE "<<numTauBSCC<<" BSCC IN TAU:" << endl;
		cout << "  ";
		cout << print_vec_1based_ifnoteq(TauBSCC, -1, "*") << endl;
		cout << endl;
	}
	
	return numTauBSCC;
}

//=============================================================================

static void 
ExtractXi(const MRP& mrp, const vector<int>& Q_SCC, 
		  const vector<regencomp_t>& Qscc2comp,
		  const vector<RegenerativeComponent>& components,
		  ublas::compressed_matrix<double>& Xi, VerboseLevel verboseLvl)
{
	typedef ublas::compressed_matrix<double>::const_iterator1 row_iterator_t;
	typedef ublas::compressed_matrix<double>::const_iterator2 elem_iterator_t;
	const regencomp_t numRegenComps(components.size());
	numerical::matrix_map<double> mmXi(numRegenComps, numRegenComps);
	for (size_t mat=0; mat<3; mat++) {
		const ublas::compressed_matrix<double> *pM = NULL;
		switch (mat) {
			case 0:	pM = &mrp.Q;     break;
			case 1:	pM = &mrp.Qbar;  break;
			case 2:	pM = &mrp.Delta; break;
		}
		
		row_iterator_t M_rowIt(pM->begin1());
		row_iterator_t M_rowItEnd(pM->end1());
		while (M_rowIt != M_rowItEnd) {
			const marking_t i = marking_t(M_rowIt.index1());
			const sccindex_t scc_i = Q_SCC[i];
			const regencomp_t comp_i = Qscc2comp[scc_i];
			//const taubsccindex_t tbi_i = components[comp_i].tauBscc;
			
			elem_iterator_t M_ij(M_rowIt.begin());
			elem_iterator_t iRowEnd(M_rowIt.end());
			while (M_ij != iRowEnd) {
				const marking_t j = marking_t(M_ij.index2());
				const sccindex_t scc_j = Q_SCC[j];
				const regencomp_t comp_j = Qscc2comp[scc_j];
				//const taubsccindex_t tbi_j = components[comp_j].tauBscc;
				
				// Add Qbar and Delta self-loops, and Q+Qbar+Delta arcs
				if (mat>0 || comp_i != comp_j)
					mmXi(comp_i, comp_j) = 1;
				
				++M_ij;
			}
			++M_rowIt;
		}		
	}
	compress_matrix(Xi, mmXi);
	mmXi.clear();
	if (verboseLvl >= VL_VERBOSE) {
		cout << "\nCOMPUTING REGENERATIVE COMPONENT ADJACENCY MATRIX XI:\n\n";
		print_matrix(cout, Xi, "Xi");
		cout << endl;
	}
}

//=============================================================================

static void 
AddRCInputProb(vector<RegenerativeComponent*>& regenCompQueue,
			   const vector<regencomp_t>& mark2comp,
			   vector<RegenerativeComponent>& components,
			   const marking_t mark, double prob)
{
	if (prob == 0.0)
		return;
	const regencomp_t comp_n = mark2comp[mark];
	RegenerativeComponent* pRC = &components[comp_n];
	if (!pRC->belongsToInitTransient())
		return;
	if (!pRC->enqueued) {
		// Enqueue pRC for the initial transitory computation
		pRC->inputProb = prob;
		pRC->enqueued  = true;
		regenCompQueue.push_back(pRC);
	}
	else
		pRC->inputProb += prob;	
}

//=============================================================================

static void
ComputeRegenerativeComponent(const MRP& mrp, 
							 const regencomp_t selectedComponent, 
							 vector<RegenerativeComponent>& components,
							 const vector<regencomp_t>& mark2comp,
							 vector<RegenerativeComponent*>& regenCompQueue,
							 ublas::vector<double>& pi, double epsilon,
							 const SolverParams& spar, 
							 VerboseLevel verboseLvl)
{
	RegenerativeComponent& mpc = components[selectedComponent];
	if (verboseLvl >= VL_VERBOSE) {
		cout << "\n\n\n-----------------------------------\n";
		cout << "INITIAL TRANSIENT COMPONENT "<<size_t(selectedComponent)+1<<":\n";
	}
	
	// Prepare the Subordinated Markov Chain
	SMC mpcSmc;
	ConstructSMC(mrp, mpc.States, mpcSmc, VL_NONE);
	const smcstate_t N_SMC(mpcSmc.S.size1());
	
	// Extract the input probability vector
	ublas::vector<double> inPi(N_SMC, 0.0), outPi, *nullPi=NULL;
	for (size_t si=0; si<mpc.States.size(); si++) {
		const marking_t inMrpState = mpc.States[si];
		const smcstate_t inSmcState = mpcSmc.mrpToInternalSmcInd[inMrpState];
		assert(inSmcState != NOT_SMC_STATE);
		inPi(inSmcState) = pi(inMrpState);
		pi(inMrpState) = 0.0;
	}
	if (verboseLvl >= VL_VERBOSE)
		cout << "\nINPUT SMC PROBABILITY:\n  " << inPi << endl;
	
	// Compute the SMC probability
	if (mpc.subset == EXP_SUBSET) {
		// Solve the SMC with a steady-state iterative method
		if (!Solve_SteadyState_CTMC(mpcSmc.S, outPi, inPi,
									spar, VL_NONE))
			throw program_exception("Solution method does not converge.");
	}
	else {
		// Solve the SMC with a transient method
		full_element_selector all_elems(N_SMC);
		if(!Uniformization(mpcSmc.S, inPi, &outPi, nullPi, 
						   mrp.Det[mpc.subset].delay, epsilon, 
						   all_elems, all_elems, VL_NONE, spar.trAlgo))
			throw program_exception("Couldn't compute transient "
									"SMC probabilities.");
	}
	outPi /= norm_1(outPi);
	outPi *= norm_1(inPi);
	
	if (verboseLvl >= VL_VERBOSE)
		cout << "OUTPUT SMC PROBABILITY:\n  " << outPi << endl;
	// TODO: Verify outPi according to Xi
	
	// Distribute the output probability back into pi
	for (smcstate_t smc_i=0; smc_i<N_SMC; smc_i++) {
		if (outPi(smc_i) == 0.0)
			continue;
		
		const marking_t mrp_i = mpcSmc.smcToMrpInds[smc_i];
		if (smc_i < mpcSmc.numInternalStates && mpcSmc.subset != EXP_SUBSET) {
			// Distribute the output probability according to Delta[i]
			matRow_t deltaRow(mrp.Delta, mrp_i);
			rowIter_t delta_ij(deltaRow.begin()), deltaRowEnd(deltaRow.end());
			for (; delta_ij != deltaRowEnd; ++delta_ij) {
				marking_t mrp_j(delta_ij.index());
				double prob_ij = outPi(smc_i) * (*delta_ij);
				AddRCInputProb(regenCompQueue, mark2comp, components, 
							   mrp_j, prob_ij);
				pi(mrp_j) += prob_ij;
			}
		}
		else {
			// Absorbed probability due to a Qbar preemptive arc
			AddRCInputProb(regenCompQueue, mark2comp, components, 
						   mrp_i, outPi(smc_i));
			pi(mrp_i) += outPi(smc_i);
		}
	}
	if (verboseLvl >= VL_VERBOSE)
		cout << "MRP PROBABILITY:\n  " << pi << endl;
}

//=============================================================================

const marking_t NOT_SUBMRP_MARKING(size_t(-54));

void ExtractSubMRP(const MRP& mrp, const MarkingSubset_t& mark2submark,
				   const ublas::vector<double>& mrpPi, MRP& subMrp, 
				   VerboseLevel verboseLvl)
{
	const marking_t N_MRP(mrp.N());
	marking_t N_SUBMRP(0);
	
	// Count sub-MRP states
	for (marking_t m=0; m<N_MRP; m++)
		if (mark2submark[m] != NOT_SUBMRP_MARKING)
			++N_SUBMRP;
	assert(N_SUBMRP > marking_t(0));
	subMrp.pi0.resize(N_SUBMRP);
	
	// Conversion map for state subset indices
	const statesubset_t N_DET(mrp.Det.size()), NOT_SUBDET(size_t(-31));
	statesubset_t N_SUBDET(0);
	vector<statesubset_t> det2subdet(N_DET);
	fill(det2subdet.begin(), det2subdet.end(), NOT_SUBDET);
	
	vector<statesubset_t> markToSubset;
	markToSubset.resize(N_SUBMRP);
	fill(markToSubset.begin(), markToSubset.end(), EXP_SUBSET);	
	
	for (marking_t m(0); m<N_MRP; m++) {
		const marking_t sub_m = mark2submark[m];
		if (sub_m == NOT_SUBMRP_MARKING)
			continue;
		// Prepare State subsets
		statesubset_t ssm = mrp.Subsets.subsetOf(m);
		if (ssm == EXP_SUBSET)
			markToSubset[sub_m] = EXP_SUBSET;
		else {
			if (det2subdet[ssm] == NOT_SUBDET) {
				DetMarkingSubset dms;
				dms.index     = N_SUBDET++;
				dms.delay     = mrp.Det[ssm].delay;
				dms.detTrnInd = mrp.Det[ssm].detTrnInd;
				subMrp.Det.push_back(dms);
				det2subdet[ssm] = dms.index;
			}
			markToSubset[sub_m] = det2subdet[ssm];
		}
		// Initial vector pi0
		subMrp.pi0(sub_m) = mrpPi(m);
	}
	subMrp.Subsets.ConstructSubsetList(det2subdet.size()+2, markToSubset);
	
	// Filter out and copy the 3 MRP matrices
	typedef ublas::compressed_matrix<double>::const_iterator1 row_iterator_t;
	typedef ublas::compressed_matrix<double>::const_iterator2 elem_iterator_t;
	for (size_t mat=0; mat<3; mat++) {
		const ublas::compressed_matrix<double> *pSrc = NULL;
		ublas::compressed_matrix<double> *pDst = NULL;
		switch (mat) {
			case 0:	pSrc = &mrp.Q;     pDst = &subMrp.Q;	 break;
			case 1:	pSrc = &mrp.Qbar;  pDst = &subMrp.Qbar;  break;
			case 2:	pSrc = &mrp.Delta; pDst = &subMrp.Delta; break;
		}
		
		pDst->resize(size_t(N_SUBMRP), size_t(N_SUBMRP), false);
		row_iterator_t M_rowIt(pSrc->begin1());
		row_iterator_t M_rowItEnd(pSrc->end1());
		while (M_rowIt != M_rowItEnd) {
			const marking_t i = marking_t(M_rowIt.index1());
			const marking_t sub_i = mark2submark[i];
			if (sub_i != NOT_SUBMRP_MARKING) {
				// Copy row i elements
				elem_iterator_t M_ij(M_rowIt.begin());
				elem_iterator_t iRowEnd(M_rowIt.end());
				while (M_ij != iRowEnd) {
					const marking_t j = marking_t(M_ij.index2());
					const marking_t sub_j = mark2submark[j];
					
					// If this is really an ergodic terminal sub-MRP, than
					// any transition should be contained inside the sub-MRP
					assert(sub_j != NOT_SUBMRP_MARKING);
					pDst->insert_element(size_t(sub_i), size_t(sub_j), *M_ij);
					
					++M_ij;
				}
			}
			++M_rowIt;
		}		
	}
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << "\n\n\n\n\n===============================================\n\n";
		PrintMRP(subMrp);
	}
}

//=============================================================================

void SolveNonErgodicMRP(const MRP& mrp, PetriNetSolution& sol, 
						bool detectIsomorphism, const SolverParams& spar, 
						VerboseLevel verboseLvl)
{
	const marking_t N_MRP = mrp.N();
	
	// Compute Q SCC (SCC internal to the subordinated chains)
	vector<int> Q_SCC;
	const sccindex_t numQ_SCC(TarjanIterative(mrp.Q, Q_SCC));
	if (verboseLvl >= VL_VERBOSE) {
		cout << "THERE ARE "<<numQ_SCC<<" DISTINCT SCC INSIDE Q:" << endl;
		cout << "  " << print_vec(Q_SCC) << endl;
		cout << endl;
	}
	
	// Compute the SCC -> subset table
	vector<statesubset_t> scc2subset(numQ_SCC);
	for (marking_t m(0); m<N_MRP; m++) {
		scc2subset[Q_SCC[m]] = mrp.Subsets.subsetOf(m);
	}
	
	// Build up the Transient Component Matrix Tau
	ublas::compressed_matrix<double> Tau;
	vector<int> TauBSCC;
	const taubsccindex_t numTauBSCC(ExtractTau(mrp, Q_SCC, TauBSCC, Tau, verboseLvl ));
	
	if (numTauBSCC == taubsccindex_t(0)) {
		cout << "THE MRP IS NOT REDUCIBLE." << endl;
		SolveReducedMRP(mrp, sol, detectIsomorphism, spar, verboseLvl);
		return;
	}
	
	// Extract the Regenerative Component set
	// A regen. comp. is a non-empty subset of:  (TauBSCC U {-1}) x StateSubsets
	regencomp_t numRegenComps(0), numInitTrRegenComps(0);
	vector<regencomp_t> mark2comp(N_MRP);
	vector<regencomp_t> Qscc2comp(numQ_SCC, regencomp_t(-1));
	vector<RegenerativeComponent> components;
	map<pair<statesubset_t, taubsccindex_t>, regencomp_t> compMap;
	for (marking_t m(0); m<N_MRP; ++m) {
		statesubset_t subset = mrp.Subsets.subsetOf(m);
		taubsccindex_t tbscc_ind = TauBSCC[Q_SCC[m]];
		pair<statesubset_t, taubsccindex_t> desc = make_pair(subset, tbscc_ind);
		if (compMap.count(desc) == 0) {
			components.resize(components.size() + 1);
			RegenerativeComponent& rc = components.back();
			rc.index    = regencomp_t(components.size()-1);
			rc.subset   = subset;
			rc.tauBscc  = tbscc_ind;
			rc.enqueued = false;
			compMap.insert(make_pair(desc, numRegenComps++));
			if (rc.tauBscc == taubsccindex_t(-1))
				numInitTrRegenComps++;
		}
		RegenerativeComponent& rc = components[compMap.find(desc)->second];
		mark2comp[m] = rc.index;
		rc.States.push_back(m);
		assert(regencomp_t(-1)==Qscc2comp[Q_SCC[m]] || 
			   Qscc2comp[Q_SCC[m]]==rc.index);
		Qscc2comp[Q_SCC[m]] = rc.index;
	}
	
	// Print out some statistics
	cout << "THE MRP IS REDUCIBLE, WITH " << numTauBSCC << " BSCCs." << endl;
	cout << "THERE ARE "<<numRegenComps<<" REGENERATIVE COMPONENTS." << endl;
	/*cout << ""<<numInitTrRegenComps<<" REGENERATIVE COMPONENTS ARE THE ";
	cout << "INITIAL TRANSIENT.\n" << endl;*/
	if (verboseLvl >= VL_VERBOSE) {
		for (regencomp_t c(0); c<numRegenComps; ++c) {
			cout << "  Comp["<<int(c)+1<<"]: Tau Bscc=";
			if (components[c].tauBscc == taubsccindex_t(-1)) cout << "*";
			else cout << size_t(components[c].tauBscc)+1;
			cout << "  State Subset=";
			if (components[c].subset == EXP_SUBSET) cout << "EXP" << endl;
			else cout << "DET[" << size_t(components[c].subset)+1 << "]" << endl;
		}
		cout << "\nMRP -> COMP: "<<print_vec_1based(mark2comp)<<endl;
		cout << "Q_SCC -> COMP: "<<print_vec_1based(Qscc2comp)<<endl;
	}
	
	// Build up the Regenerative Component Adjacency matrix Xi
	ublas::compressed_matrix<double> Xi;
	ExtractXi(mrp, Q_SCC, Qscc2comp, components, Xi, verboseLvl );
	
	// TODO: verify Xi correctness in the transient computation process
	//if (verboseLvl >= VL_VERBOSE)
	//	SaveInitTrnAsDOT("RecurrenceStructure", mrp, Q_SCC, Xi, components, mark2comp);
	
	// Prepare the Component Queue with the initial probability vector pi0
	ublas::vector<double> pi = mrp.pi0;
	vector<RegenerativeComponent*> regenCompQueue;
	for (marking_t m=0; m<N_MRP; m++) {
		AddRCInputProb(regenCompQueue, mark2comp, components, m, pi(m));
	}
	
	// Process Initial Transient Regenerative Components of the Marking Process
	cout << "COMPUTING RECURRENCE CLASS PROBABILITIES.\n\n" << endl;
	time_t startTime;
	time(&startTime);
	double lastElapsedTime = -1;
	size_t numProcessedRegenComps = 0;
	while (!regenCompQueue.empty()) {
		size_t selCompInd = 0;
		double selInputProb = regenCompQueue[0]->inputProb;
		double totalInitTrProb = selInputProb;
		for (size_t i=1; i<regenCompQueue.size(); i++) {
			if (regenCompQueue[i]->inputProb > selInputProb) {
				selCompInd = i;
				selInputProb = regenCompQueue[i]->inputProb;
			}
			totalInitTrProb += regenCompQueue[i]->inputProb;
		}
		if (verboseLvl == VL_BASIC) {
			time_t currTime;
			time(&currTime);		
			double elapsedTime = difftime(currTime, startTime);
			if (elapsedTime != lastElapsedTime) {
				cout << one_line_up() << "REMAINED PROBABILITY: ";
				cout << fmtdbl(totalInitTrProb, "%.10lf") << "  WITH ";
				cout << numProcessedRegenComps << " PROCESSED CHAINS... " << endl;
				lastElapsedTime = elapsedTime;
			}
		}
		
		/*cout << "  # QUEUE: ";
		for (size_t i=0; i<regenCompQueue.size(); i++) {
			RegenerativeComponent& rc = *regenCompQueue[i];
			cout << size_t(rc.index)+1 << ": " << fmtdbl(rc.inputProb, "%.10lf") << "  ";
		}
		cout << "   SELECTING " << size_t(selCompInd)+1 << endl << endl;//*/
		RegenerativeComponent& rc = *regenCompQueue[selCompInd];
		regenCompQueue.erase(regenCompQueue.begin() + selCompInd);
		rc.enqueued  = false;
		
		// Ignore this component if the remained probability is too low
		if (rc.inputProb < spar.epsilon / numRegenComps) {
			//cout << "##IGNORING: "<<rc.inputProb<<" < "<<epsilon<<endl;
			for (size_t i=0; i<rc.States.size(); i++)
				pi(rc.States[i]) = 0.0;
			continue;
		}
		rc.inputProb = 0.0;
		
		// Process input probability of the selected Regenerative Component
		ComputeRegenerativeComponent(mrp, rc.index, components, mark2comp,
									 regenCompQueue, pi, 
									 spar.epsilon / numRegenComps,
									 spar, verboseLvl);
		numProcessedRegenComps++;
	}
	
	// Normalize the absorbing vector
	pi /= norm_1(pi);
	
	if (verboseLvl == VL_BASIC) {
		cout << one_line_up() << "RECURRENT COMPONENTS PROBABILITY HAVE BEEN ";
		cout << "COMPUTED.                       " << endl;
	}
	if (verboseLvl >= VL_VERBOSE)
		cout << "\nABSORBING PROBABILITY:\n  " << pi << endl;
	cout << numProcessedRegenComps << " REGENERATIVE COMPONENTS HAVE BEEN ";
	cout << "PROCESSED TO DETERMINE THE BSCC PROBABILITY VECTOR.\n" << endl;
	
	// At this point, solve the Ergodic Terminal MRPs
	sol.stateProbs.resize(N_MRP);
	fill(sol.stateProbs.begin(), sol.stateProbs.end(), 0.0);
	for (taubsccindex_t TauBSCCInd(0); TauBSCCInd < numTauBSCC; TauBSCCInd++) {
		MarkingSubset_t mark2submark(N_MRP);
		marking_t numSubMRPStates(0);
		fill(mark2submark.begin(), mark2submark.end(), NOT_SUBMRP_MARKING);
		bool isCTMC = true, hasStates = false;
		for (regencomp_t c(0); c<numRegenComps; ++c) {
			const RegenerativeComponent& rc = components[c];
			if (rc.tauBscc == TauBSCCInd) {
				hasStates = true;
				if (rc.subset != EXP_SUBSET)
					isCTMC = false;
				for (size_t s=0; s<rc.States.size(); s++) {
					marking_t m = rc.States[s];
					assert(mark2submark[m] == NOT_SUBMRP_MARKING);
					mark2submark[m] = numSubMRPStates++;
				}
			}
		}
		if (!hasStates)
			continue; // This Tau BSCC is not a Regenerative Component BSCC
		
		if (verboseLvl >= VL_VERBOSE) {
			const char *whatIs = (isCTMC ? "CTMC" : "MRP");
			cout << "\n\n";
			cout << "COMPUTING TERMINAL SUB-" << whatIs << " FOR TAU BSCC ";
			cout << int(TauBSCCInd)+1 << ".\n" << "SUB-" << whatIs;
			cout << " MARK -> SUBMARK:\n  ";
			cout << print_vec_1based_ifnoteq(mark2submark, NOT_SUBMRP_MARKING, "X");
			cout << endl << endl;
		}
		
		MRP subMrp;
		ExtractSubMRP(mrp, mark2submark, pi, subMrp, VL_NONE);
		double bsccProb = norm_1(subMrp.pi0);
		subMrp.pi0 /= bsccProb;
				
		// Solve the sub-MRP process
		PetriNetSolution bsccSol;	
		if (isCTMC) {
			if (!Solve_SteadyState_CTMC(subMrp.Q, bsccSol.stateProbs, 
										subMrp.pi0, spar, VL_NONE))
				throw program_exception("Solution method does not converge.");				
		}
		else {
			// Decomment the following line to do the explicit solution
			SolveReducedMRP(subMrp, bsccSol, detectIsomorphism, spar, VL_NONE);
			// TODO: USE THE IMPLICIT SOLVER
			/*MRPStateKinds stKinds;
			ClassifyMRPStates(subMrp, stKinds, verboseLvl);
			SolveSteadyStateReducibleMRP(subMrp, stKinds, bsccSol, 
										 spar, VL_BASIC);*/
		}
		
		// Copy back the solution vector
		for (marking_t m(0); m<N_MRP; m++) {
			if (mark2submark[m] == NOT_SUBMRP_MARKING)
				continue;
			sol.stateProbs[m] += bsccSol.stateProbs[mark2submark[m]] * bsccProb;
		}
	}
	
	// Normalize the solution vector
	sol.stateProbs /= norm_1(sol.stateProbs);
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << "MRP SOLUTION: " << sol.stateProbs << endl;
	}
}

//=============================================================================

void SolveSteadyStateMRP(RG& rg, PetriNetSolution& sol, bool detectIsomorphism,
						 bool reduceInitialTransient, 
						 const SolverParams& spar, VerboseLevel verboseLvl)
{
	MRP mrp;
	ExtractMRP(rg, mrp, verboseLvl );
	
	if (reduceInitialTransient)
		SolveNonErgodicMRP(mrp, sol, detectIsomorphism, spar, verboseLvl );
	else
		SolveReducedMRP(mrp, sol, detectIsomorphism, spar, verboseLvl );

	ComputeMeasures(rg, sol, verboseLvl);
}

//=============================================================================












