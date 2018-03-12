/*
 *  cslta.cpp
 *  CslTA-Solver
 *
 *  Implementation of the CSL over Timed Automata model checker
 *
 */

#include "precompiled.pch"

using namespace boost;
using namespace std;

#include "common-defs.h"
#include "asmc.h"
#include "dta.h"
#include "dspn.h"
#include "cslta.h"



// DSPNexpress file extensions
const char *DSPNexpressTmpFileExts[] = { 
	"NET", "DEF", "AUX", "ECS", "INV", "PROBTOK", "PTNAMES",
	"RESULTS", "THROUGHPUT", "URG", "BSCC", "DOUBLEARCS"
};
// const size_t DSPNexpressTmpFileExts_NUM = (sizeof(DSPNexpressTmpFileExts) /
// 										   sizeof(DSPNexpressTmpFileExts[0]));


// Names of absorbing "places", that requires a loop in SPNica
const char *s_absorbPlaces[] = { 
	ProbTaSolver::PLN_TOP, ProbTaSolver::PLN_BOT, ProbTaSolver::PLN_NONDET
};
const size_t NUM_ABSORBING_PLACES = sizeof(s_absorbPlaces)/sizeof(s_absorbPlaces[0]);


//-----------------------------------------------------------------------------

const char* ProbabilityCmpOp_Str[] = {
	"<", "<=", ">", ">="
};

const char* APCmpOp_Str[] = {
	"==", "!=", "<", "<=", ">", ">=", "error"
};






//-----------------------------------------------------------------------------

bool CslTaResult::WriteAsGmlFile(ostream& os, const char *name, const char *args)
{
	if (solverUsed.get() != NULL) {
		return solverUsed->WriteSolverDspnAsGml(os, name, args);
	}	
	return false;
}

bool CslTaResult::LoadDspnCoords(istream& is, const char *args)
{
	if (solverUsed.get() != NULL) {
		return solverUsed->LoadDspnCoords(is, args);
	}	
	return false;
}

bool CslTaResult::PlanarizeDspn(const char *args)
{
	if (solverUsed.get() != NULL) {
		return solverUsed->PlanarizeDspn(args);
	}	
	return false;
}

bool CslTaResult::GetDspnToSave(const char *args, DSPN& outDspn)
{
	if (solverUsed.get() != NULL) {
		return solverUsed->GetDspnToSave(args, outDspn);
	}	
	return false;
}








//-----------------------------------------------------------------------------

bool APValExpr_Const::PrepareEvaluation(ASMC& asmc) const  { 
	return true; 
}
bool APValExpr_AP::PrepareEvaluation(ASMC& asmc) const  { 
	if (!asmc.AtomicProps.HasSymbol(ap)) {
		cout << "Error: couldn't find atomic proposition \"" << ap;
		cout << "\" in the ASMC."<<endl;
		return false;
	}
	return true;
}
bool APValExpr_UnOp::PrepareEvaluation(ASMC& asmc) const  { 
	return expr->PrepareEvaluation(asmc);
}
bool APValExpr_BinOp::PrepareEvaluation(ASMC& asmc) const  { 
	return expr1->PrepareEvaluation(asmc) && expr2->PrepareEvaluation(asmc);
}


bool APValExpr_Const::Evaluate(ASMC& asmc, const vector<bool>& evalStates,
							   vector<int>& exprVal) const 
{
	assert(exprVal.size() == 0);
	exprVal.resize(num_vertices(asmc.graph), -1000);
	for (size_t n=0; n<exprVal.size(); n++)
		if (evalStates[n])
			exprVal[n] = val;
	return true;
}

bool APValExpr_AP::Evaluate(ASMC& asmc, const vector<bool>& evalStates,
							vector<int>& exprVal) const 
{
	assert (asmc.AtomicProps.HasSymbol(ap));
	SymbolID apID = asmc.AtomicProps.GetIDFromName(ap);
	exprVal.resize(num_vertices(asmc.graph), -1000);
	
	// Get the Atomic Proposition values
	ASMC::vertex_iter_t it, it_end;
	for (tie(it, it_end) = vertices(asmc.graph); it != it_end; ++it) {
		if (evalStates[*it]) 
			exprVal[*it] = get(asmc.vertPMap, *it).labelVals[apID];
	}
	return true;
}

bool APValExpr_UnOp::Evaluate(ASMC& asmc, const vector<bool>& evalStates,
							  vector<int>& exprVal) const 
{
	if (!expr->Evaluate(asmc, evalStates, exprVal))
		return false;
	for (size_t n=0; n<exprVal.size(); n++) {
		if (evalStates[n]) {
			switch (op) {
				case '-':   exprVal[n] = -exprVal[n];		break;
				default:    assert(0);
			}
		}
	}
	return true;
}

bool APValExpr_BinOp::Evaluate(ASMC& asmc, const vector<bool>& evalStates,
							   vector<int>& exprVal) const 
{
	vector<int> exprVal2;
	if (!expr1->Evaluate(asmc, evalStates, exprVal))
		return false;
	if (!expr2->Evaluate(asmc, evalStates, exprVal2))
		return false;
	for (size_t n=0; n<exprVal.size(); n++) {
		if (evalStates[n]) {
			switch (op) {
				case '+':   exprVal[n] += exprVal2[n];		break;
				case '-':   exprVal[n] -= exprVal2[n];		break;
				case '*':   exprVal[n] *= exprVal2[n];		break;
				case '/':
					if (exprVal2[n] == 0) {
						cout << "Division by zero." << endl;
						return false;
					}
					exprVal[n] /= exprVal2[n];
					break;
				case '%':
					if (exprVal2[n] == 0) {
						cout << "Modulus by zero." << endl;
						return false;
					}
					exprVal[n] %= exprVal2[n];
					break;
				default:    assert(0);
			}
		}
	}
	return true;	
}


void APValExpr_Const::PrintExpr(ostream& os) const {
	os << val;
}
void APValExpr_AP::PrintExpr(ostream& os) const {
	os << ap;
}
void APValExpr_UnOp::PrintExpr(ostream& os) const {
	os << op << *expr;
}
void APValExpr_BinOp::PrintExpr(ostream& os) const {
	os << "(" << *expr1 << " " << op << " " << *expr2 << ")";
}








//-----------------------------------------------------------------------------

bool CslTaExpr_TF::PrepareEvaluation(ASMC& asmc) { 
	return true;
}

bool CslTaExpr_AP::PrepareEvaluation(ASMC& asmc) {
	return apExpr->PrepareEvaluation(asmc);
}

bool CslTaExpr_NOT::PrepareEvaluation(ASMC& asmc) { 
	return expr->PrepareEvaluation(asmc);
}

bool CslTaExpr_BOP::PrepareEvaluation(ASMC& asmc) { 
	return expr1->PrepareEvaluation(asmc) && expr2->PrepareEvaluation(asmc);
}

bool CslTaExpr_STEADY::PrepareEvaluation(ASMC& asmc) {
	return expr->PrepareEvaluation(asmc);
}

bool CslTaExpr_PROB_TA::PrepareEvaluation(ASMC& asmc) { 
	// Find out the parameterized DTA
	DTA *d = (DTA*)GS_RetrieveMCObjectByName(params->DtaName.c_str(), MCOT_DTA);
	if (NULL == d) {
		cout << "Error: couldn't find a DTA named \""<<params->DtaName<<"\"."<<endl;
		return false;
	}
	
	// Try to instantiate the DTA
	string reason;
	if (!d->Instantiate(asmc, params->CCVals, params->ActSets, instDta, reason)) {
		cout << reason << endl;
		cout << "Couldn't instantiate the DTA." << endl;
		return false;
	}
	
	// Test correctness of the state propositions
	if (params->SPsExpr.size() != d->SPs.NumSymbols()) {
		cout << "Error: the number of CSL-TA subexpressions does not match the" << endl;
		cout << "       number of state propositions in the DTA." << endl;
		return false;
	}
	
	/*cout << "CCV: " << print_vectors(d->CCVs.symbols, dtap->CCVals) << endl;
	cout << "SPs: " << print_vectors(d->SPs.symbols, dtap->SPsExpr) << endl;
	cout << "Act: " << print_vectors(d->Acts.symbols, dtap->Actions) << endl;//*/
	
	// Prepare subexpressions
	vector<std::shared_ptr<CslTaExpr> >::const_iterator eit;
	for (eit = params->SPsExpr.begin(); eit != params->SPsExpr.end(); ++eit)
		if (!(*eit)->PrepareEvaluation(asmc))
			return false;
		
	return true;
}




bool CslTaExpr_TF::Evaluate(ASMC& asmc, const vector<bool>& evalStates, 
							CslTaResult& res) const 
{
	res.probs.clear();
	res.eval.resize(num_vertices(asmc.graph), val);
	std::fill(res.eval.begin(), res.eval.end(), val);
	return true;
}

bool CslTaExpr_AP::Evaluate(ASMC& asmc, const vector<bool>& evalStates, 
							CslTaResult& res) const 
{
	// Prepare the data structure for the evaluation result
	res.eval.resize(num_vertices(asmc.graph), false);
	res.probs.clear();
	
	// Values of the AP sub-expression
	vector<int> apExprVals;
	if (!apExpr->Evaluate(asmc, evalStates, apExprVals))
		return false;
	assert(apExprVals.size() == res.eval.size());

	/*cout << *apExpr << "  = [";
	for (size_t n=0; n<apExprVals.size(); n++)
		cout << " " << apExprVals[n];
	cout << " ]" << endl;*/

	// Test every ASMC state value with the comparing value
	ASMC::vertex_iter_t it, it_end;
	for (tie(it, it_end) = vertices(asmc.graph); it != it_end; ++it) {
		if (evalStates[*it]) {
			int val = apExprVals[*it];
			bool b;
			switch (op) {
				case APCO_EQ:			b = (val == cmpVal); break;
				case APCO_NOT_EQ_IMPLICIT:
				case APCO_NOT_EQ:		b = (val != cmpVal); break;
				case APCO_LESS:			b = (val <  cmpVal); break;
				case APCO_LESS_EQ:		b = (val <= cmpVal); break;
				case APCO_GREATER:		b = (val >  cmpVal); break;
				case APCO_GREATER_EQ:	b = (val >= cmpVal); break;
				default: exit(-1);
			}
			res.eval[*it] = b;
		}
	}	
	
	return true;
}

bool CslTaExpr_NOT::Evaluate(ASMC& asmc, const vector<bool>& evalStates, 
							 CslTaResult& res) const 
{
	if (!expr->Evaluate(asmc, evalStates, res))
		return false;
	vector<bool>::iterator it;
	for (it = res.eval.begin(); it != res.eval.end(); ++it)
		if (evalStates[it - res.eval.begin()])
			*it = !*it; // negate the evaluation
	return true;
}

bool CslTaExpr_BOP::Evaluate(ASMC& asmc, const vector<bool>& evalStates, 
							 CslTaResult& res) const 
{
	CslTaResult res2;
	if (!expr1->Evaluate(asmc, evalStates, res))
		return false;
	vector<bool> secondOpEvalStates(num_vertices(asmc.graph), false);
	for (size_t s=0; s<num_vertices(asmc.graph); s++) {
		if (evalStates[s])
			switch(op) {
				case BOP_AND:    secondOpEvalStates[s] =  res.eval[s]; break;
				case BOP_OR:     secondOpEvalStates[s] = !res.eval[s]; break;
				case BOP_IMPLY:  secondOpEvalStates[s] =  res.eval[s]; break;
			}
	}
	
	if (!expr2->Evaluate(asmc, secondOpEvalStates, res2))
		return false;
	
	vector<bool>::iterator it1 = res.eval.begin();
	vector<bool>::iterator it2 = res2.eval.begin();
	for (; it1 != res.eval.end(); ++it1, ++it2) {
		size_t s = it1 - res.eval.begin();
		if (evalStates[s]) {
			if (secondOpEvalStates[s]) {
				switch (op) {
					case BOP_AND:   *it1 =  (*it1) & (*it2); break;
					case BOP_OR:    *it1 =  (*it1) | (*it2); break;
					case BOP_IMPLY: *it1 = !(*it1) | (*it2); break;
				}
			} else {
				switch (op) {
					case BOP_AND:   *it1 = false;	break;
					case BOP_OR:    *it1 = true;	break;
					case BOP_IMPLY: *it1 = !(*it1); break;
					default: exit(-1);
				}
			}
		}
	}
	if (res.probs.size() == 0)
		res.probs.swap(res2.probs);
	if (res.solverUsed.get()==NULL && res2.solverUsed.get()!=NULL)
		res.solverUsed.swap(res2.solverUsed);
	return true;
}

bool CslTaExpr_STEADY::Evaluate(ASMC& asmc, const vector<bool>& evalStates, 
								CslTaResult& res) const 
{
	// Evaluate the subexpression
	CslTaResult subExprRes;
	vector<bool> subExprEvalMask(num_vertices(asmc.graph), true);
	if (!expr->Evaluate(asmc, subExprEvalMask, subExprRes))
		return false;
	
	res.eval.clear();
	res.probs.clear();
	res.eval.resize(num_vertices(asmc.graph), false);
	res.probs.resize(num_vertices(asmc.graph), -10.0);

	// Create the DSPN
	std::shared_ptr<SteadySolver> solver(new SteadySolver(&asmc, &res));
	solver->CreateDSPN();
	
	// Extract all the BSCC from the ASMC graph
	vector<int> bsccIds;
	int numBscc = asmc.FindBSCCs(bsccIds);
	vector< pair<bool,double> > resultForBscc(numBscc, make_pair(false,-10));
	
	// For every state s in the ASMC not in a BSCC and one time for every BSCC,
	// get steady state probabilities of the SPN.
	res.eval.resize(num_vertices(asmc.graph), false);
	for (size_t s=0; s<num_vertices(asmc.graph); s++) {
		if (!evalStates[s])
			continue;		
		if (bsccIds[s] != -1 && resultForBscc[ bsccIds[s] ].second >= 0) {
			res.eval[s]  = resultForBscc[ bsccIds[s] ].first;
			res.probs[s] = resultForBscc[ bsccIds[s] ].second;
		}
		else {
			// Evaluate the S(phi) operator in state s
			if (!solver->SolveInState(s, subExprRes.eval, numBscc, res.probs[s]))
				return false;
			
			// Set the success value
			bool spnResult = false;
			switch (pc.pco) {
				case PCO_LESS:		 spnResult = (res.probs[s] <  pc.value);  break;
				case PCO_LESS_EQ:	 spnResult = (res.probs[s] <= pc.value);  break;
				case PCO_GREATER:	 spnResult = (res.probs[s] >  pc.value);  break;
				case PCO_GREATER_EQ: spnResult = (res.probs[s] >= pc.value);  break;
			}
			res.eval[s] = spnResult;			
			if (bsccIds[s] != -1) {
				resultForBscc[ bsccIds[s] ].first  = res.eval[s];
				resultForBscc[ bsccIds[s] ].second = res.probs[s];
			}
		}
	}	
	
	// Set this solver in the returning result
	std::shared_ptr<DspnSolver> steadySolverUsed(solver);
	res.solverUsed.swap(steadySolverUsed);
	return true;
}

bool CslTaExpr_PROB_TA::Evaluate(ASMC& asmc, const vector<bool>& evalStates, 
								 CslTaResult& res) const 
{
	// First, evaluate sub-expressions
	vector<CslTaResult> subExprRes(params->SPsExpr.size());
	vector<bool> subExprEvalMask(num_vertices(asmc.graph), true);
	for (size_t i=0; i<params->SPsExpr.size(); i++)
		if (!params->SPsExpr[i]->Evaluate(asmc, subExprEvalMask, subExprRes[i]))
			return false;
	
	// Re-test DTA determinism now that we have all the SP values used inside
	DtaStatePropValues spVals;
	spVals.pAsmc = &asmc;
	spVals.SPValues.resize(params->SPsExpr.size());
	for (size_t i=0; i<params->SPsExpr.size(); i++) {
		ostringstream subExprName;
		params->SPsExpr[i]->PrintExpr(subExprName);
		spVals.SPValues[i] = make_pair(subExprName.str(), &subExprRes[i].eval);
	}
	string reason = instDta.VerifyDeterminism(&spVals);
	if (instDta.detKind == DDK_NEVER_DET) {
		cout << "Couldn't evaluate this DTA.\n" << reason << endl << endl;
		return false;
	}
	
	res.eval.clear();
	res.probs.clear();
	res.eval.resize(num_vertices(asmc.graph), false);
	res.probs.resize(num_vertices(asmc.graph), -10.0);

	// Create a resolving DSPN
	std::shared_ptr<ProbTaSolver> solver(new ProbTaSolver(&asmc, &instDta, 
													 (instDta.detKind == DDK_SEMI_DET),
													 &subExprRes));
	solver->CreateDSPN();

	// Send a DSPN to the Mathematica Kernel
	
	for (size_t s=0; s<num_vertices(asmc.graph); s++) {
		if (!evalStates[s])
			continue;
		
		if(!solver->SolveInState(s, res.probs[s]))
			return false;
				
		// Set the success value
		switch (pc.pco) {
			case PCO_LESS:		res.eval[s] = (res.probs[s] <  pc.value);  break;
			case PCO_LESS_EQ:	res.eval[s] = (res.probs[s] <= pc.value);  break;
			case PCO_GREATER:	res.eval[s] = (res.probs[s] >  pc.value);  break;
			case PCO_GREATER_EQ:res.eval[s] = (res.probs[s] >= pc.value);  break;
		}
	}
		
	// Set this solver in the returning result
	std::shared_ptr<DspnSolver> probTaSolverUsed(solver);
	res.solverUsed.swap(probTaSolverUsed);
	return true;
}







void CslTaExpr_TF::PrintExpr(ostream& os) const   
{   os << (val ? "True" : "False");   }
void CslTaExpr_AP::PrintExpr(ostream& os) const {
	os << *apExpr;
	if (op != APCO_NOT_EQ_IMPLICIT)
		os << APCmpOp_Str[op] << cmpVal;
}
void CslTaExpr_NOT::PrintExpr(ostream& os) const  
{   os << UnaryLogicOp_Not << *expr;   }
void CslTaExpr_BOP::PrintExpr(ostream& os) const  
{   os <<"("<<*expr1<<" "<<BinaryLogicOp_Str[op]<<" "<<*expr2<<")";   }

void CslTaExpr_STEADY::PrintExpr(ostream& os) const {   
	os <<"STEADY "<<ProbabilityCmpOp_Str[pc.pco]<<" ";
	os<<pc.value<<" ("<<*expr<<")"; 
}
void CslTaExpr_PROB_TA::PrintExpr(ostream& os) const  
{   
	os <<"PROB_TA ";
	os << ProbabilityCmpOp_Str[pc.pco]<<" "<<pc.value<<" (";
	os << params->DtaName << "[";
	
	size_t cnt, cnt3;
	vector<double>::const_iterator it1 = params->CCVals.begin();
	for (cnt = 0; it1 != params->CCVals.end(); ++it1, ++cnt)
		os << (cnt > 0 ? ", " : "") << (*it1);
	os << " | ";	
	
	vector< vector<string> >::const_iterator it3 = params->ActSets.begin();
	for (cnt3 = 0; it3 != params->ActSets.end(); ++it3, ++cnt3) {
		os << (cnt3 > 0 ? ", " : "") << (it3->size()==1 ? "" : "{");
		
		vector<string>::const_iterator it4 = it3->begin();
		for (cnt = 0; it4 != it3->end(); ++it4, ++cnt)
			os << (cnt > 0 ? "," : "") << (*it4);
		
		os << (it3->size()==1 ? "" : "}");
	}
	os << " | ";

	vector<std::shared_ptr<CslTaExpr> >::const_iterator it2 = params->SPsExpr.begin();
	for (cnt = 0; it2 != params->SPsExpr.end(); ++it2, ++cnt)
		os << (cnt > 0 ? ", " : "") << (**it2);
	os << "])";
}













//-----------------------------------------------------------------------------

const char* ProbTaSolver::PLN_RUNNING   = "Running";
const char* ProbTaSolver::PLN_TOP		= "T";
const char* ProbTaSolver::PLN_BOT		= "⊥";
const char* ProbTaSolver::PLN_NONDET	= "NonDet";
const char* ProbTaSolver::PLN_ASMCMOVED = "ASMC Moved";
const char* ProbTaSolver::PLN_CLOCK     = "Clock";
const char* ProbTaSolver::PLN_DTAMOVED	= "DTA Moved";
const char* ProbTaSolver::PLN_BSCCSINK  = "Bscc Sink";

const char* ProbTaSolver::CLNET_SP	  = "SP Expr: ";
const char* ProbTaSolver::CLNET_FIN	  = "Final states: ";
const char* ProbTaSolver::CLNET_ASMC  = "ASMC: ";
const char* ProbTaSolver::CLNET_ACT	  = "Action Set Expr: ";
const char* ProbTaSolver::CLNET_CLK	  = "Clock Net: ";
const char* ProbTaSolver::CLNET_DTA	  = "DTA: ";



// Create the DSPN that can solve the PROB_TA problem for a single ASMC state
void ProbTaSolver::CreateDSPN()
{
	// Set up shared name vectors (will be used for SPN place superposition)
	
	// * SP expression used as DTA location constraints
	DTA::location_iter_t lit, lit_end;
	for (tie(lit, lit_end) = locations(dta->graph); lit != lit_end; ++lit) {
		const DTA::LocationP& lp = get(dta->locationPMap, *lit);
		ostringstream spExprStr;
		lp.label->PrintExpr(spExprStr, dta->SPs);
		if (spExprDtaLocs.count(spExprStr.str()) == 0) {
			// Add this new SP expression string
			spExprDtaLocs[spExprStr.str()] = *lit;
			spExprNames.push_back(spExprStr.str());
		}
	}
	// * Action Set Expressions
	DTA::edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(dta->graph); eit != eit_end; ++eit) {
		const DTA::EdgeP& ep = get(dta->edgePMap, *eit);
		if (ep.isInnerEdge()) {
			string actSetDescr = ep.GetActSetDescription(dta->Acts);
			if (actSetDtaEdges.count(actSetDescr) == 0) {
				// Add a new action set expression
				actSetDtaEdges[actSetDescr] = *eit;
				actSetNames.push_back(actSetDescr);
			}
		}
	}
	// * Clock X reaching at clock constant CV
	for (size_t i=0; i<dta->CCVals.size(); i++) {
		double CV = dta->CCVals[i];
		ostringstream s;
		s << "x = " << CV;
		clkBoundNames.push_back(s.str());
	}
	
	// Setup Immediate Transition priorities order
	size_t p = 1;
	Pri_BOT				= p++;
	Pri_Fin				= p;	 p += 2;
	Pri_Clk0			= p++;
	Pri_Clk				= p++;
	Pri_DtaAEdgesBase	= p;     p += num_edges(dta->graph);
	Pri_DtaSEdgesBase	= p;     p += num_edges(dta->graph);
	Pri_ActDrainBase	= p;     p += actSetNames.size();
	Pri_ResetClk		= p++;
	Pri_DetTest			= p++;
	Pri_StopSPs			= p;     p += spExprNames.size();
	Pri_StopClkBase		= p;	 p += clkBoundNames.size()+1;
	Pri_StopAsmcBase	= p;	 p++; //TODO:p += num_vertices(asmc->graph);
	Pri_StopDtaLocBase	= p;	 p += num_vertices(dta->graph);
	Pri_StopDtaSelBase	= p;     p += num_edges(dta->graph);
		
	// Create Intermediate nets
	CreateStatePropDSPN(spDspn);
	CreateFinalStatesDSPN(finDspn);
	CreateActionSetExpressionDSPN(actDspn);
	CreateAsmcDSPN(asmcDspn);
	CreateClockDSPN(clkDspn);
	CreateDtaDSPN(dtaDspn);
	
	// Name of places used in DSPN composition
	vector<string> mergedPlaceNames;
	mergedPlaceNames.push_back(PLN_RUNNING);
	mergedPlaceNames.push_back(PLN_ASMCMOVED);
	mergedPlaceNames.push_back(PLN_TOP);
	mergedPlaceNames.push_back(PLN_BOT);
	mergedPlaceNames.push_back(PLN_CLOCK);
	mergedPlaceNames.push_back(PLN_NONDET);
	copy(spExprNames.begin(), spExprNames.end(), back_inserter(mergedPlaceNames));
	copy(actSetNames.begin(), actSetNames.end(), back_inserter(mergedPlaceNames));
	copy(clkBoundNames.begin(), clkBoundNames.end(), back_inserter(mergedPlaceNames));
	
	// Compose the intermediate DSPN together (using place superposition)
	dspn.MergeWithPlaceSuperpos(spDspn, mergedPlaceNames);
	dspn.MergeWithPlaceSuperpos(finDspn, mergedPlaceNames);
	dspn.MergeWithPlaceSuperpos(actDspn, mergedPlaceNames);
	dspn.MergeWithPlaceSuperpos(clkDspn, mergedPlaceNames);
	dspn.MergeWithPlaceSuperpos(dtaDspn, mergedPlaceNames);
	dspn.MergeWithPlaceSuperpos(asmcDspn, mergedPlaceNames);
	
	// Prepare initMarks vector
	PrepareInitMarkVector();
}


// Create State Proposition DSPN
void ProbTaSolver::CreateStatePropDSPN(DSPN& spDspn)
{
	size_t p0 = spDspn.AddPlace(PLN_RUNNING, 1);

	size_t spCount = 0;
	vector<string>::const_iterator it;
	for (it = spExprNames.begin(); it != spExprNames.end(); ++it)
	{
		size_t p = spDspn.AddPlace(*it, 0, CLNET_SP);
		size_t t = spDspn.AddImmTran("stop", 1, Pri_StopSPs + spCount, CLNET_SP);
		spDspn.AddInputArc(p, t);
		spDspn.AddInhibArc(p0, t);
		spCount++;
	}
}


// Create a DSPN with the final (Top, Bottom) states
void ProbTaSolver::CreateFinalStatesDSPN(DSPN& finDspn)
{
	size_t run = finDspn.AddPlace(PLN_RUNNING, 1/*, CLNET_FIN*/);
	size_t bot  = finDspn.AddPlace(PLN_BOT, 0, CLNET_FIN);
	size_t top  = finDspn.AddPlace(PLN_TOP, 0, CLNET_FIN);
	size_t tB  = finDspn.AddImmTran("", 1, Pri_Fin, CLNET_FIN);
	size_t tT  = finDspn.AddImmTran("", 1, Pri_Fin+1, CLNET_FIN);
	finDspn.AddInputArc(run, tB);
	finDspn.AddInputArc(run, tT);
	finDspn.AddInputOutputArcs(bot, tB);
	finDspn.AddInputOutputArcs(top, tT);
	if (isDtaSemiDet) {
		size_t ndet = finDspn.AddPlace(PLN_NONDET, 0, CLNET_FIN);
		size_t tND = finDspn.AddImmTran("", 1, Pri_Fin, CLNET_FIN);
		finDspn.AddInputArc(run, tND);
		finDspn.AddInputOutputArcs(ndet, tND);
	}
}


// Create the Action Set Expression DSPN
void ProbTaSolver::CreateActionSetExpressionDSPN(DSPN& actDspn)
{
	size_t mov = actDspn.AddPlace(PLN_ASMCMOVED, 0, CLNET_ACT);
	size_t bot = actDspn.AddPlace(PLN_BOT, 0);
	size_t mov2bot = actDspn.AddImmTran("", 1, Pri_BOT, CLNET_ACT);
	actDspn.AddInputArc(mov, mov2bot);
	actDspn.AddOutputArc(mov2bot, bot);
	
	for (size_t i=0; i<actSetNames.size(); i++)
	{
		// Add a new action set expr group in the DSPN
		size_t p = actDspn.AddPlace(actSetNames[i], 0, CLNET_ACT);
		size_t d = actDspn.AddImmTran("drain", 1, Pri_ActDrainBase + i, CLNET_ACT);
		actDspn.AddInputArc(p, d);
		actDspn.AddInhibArc(mov, d);
	}
}


// Create the ASMC DSPN as a Markovian automata
void ProbTaSolver::CreateAsmcDSPN(DSPN& asmcDspn)
{
	size_t run = asmcDspn.AddPlace(PLN_RUNNING, 1);
	size_t mov = asmcDspn.AddPlace(PLN_ASMCMOVED, 0);
	
	// add State Propositions and ActSetExpr places
	vector<string>::const_iterator it;
	vector<size_t> SPExprs;
	vector<size_t> ActSets;
	for (it = spExprNames.begin(); it != spExprNames.end(); ++it)
		SPExprs.push_back(asmcDspn.AddPlace(*it, 0));
	for (it = actSetNames.begin(); it != actSetNames.end(); ++it)
		ActSets.push_back(asmcDspn.AddPlace(*it, 0));

	// add ASMC states as places into the DSPN
	vector<size_t> states(num_vertices(asmc->graph));
	ASMC::vertex_iter_t sit, sit_end;
	for (tie(sit, sit_end) = vertices(asmc->graph); sit != sit_end; ++sit) 
	{
		const ASMC::VertexP& vp = get(asmc->vertPMap, *sit);
		states[*sit] = asmcDspn.AddPlace(vp.vertexName, 0, CLNET_ASMC);
		size_t t = asmcDspn.AddImmTran("stop", 1, Pri_StopAsmcBase /*+ *sit*/, CLNET_ASMC); // TODO: rimettere a posto
		asmcDspn.AddInputArc(states[*sit], t);
		asmcDspn.AddInhibArc(run, t);
	}
	
	// add the ASMC transitions
	ASMC::edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(asmc->graph); eit != eit_end; ++eit) 
	{
		const ASMC::EdgeP& ep = get(asmc->edgePMap, *eit);
		const ASMC::VertexP& srcVp = get(asmc->vertPMap, source(*eit, asmc->graph));
		const ASMC::VertexP& dstVp = get(asmc->vertPMap, target(*eit, asmc->graph));
		string trnName = (format("%1% -> %2%") % srcVp.vertexName % dstVp.vertexName).str();
		size_t trn = asmcDspn.AddExpTran(trnName, ep.lambda, DSPN::SingleSrv, CLNET_ASMC);
		
		// arcs s0 -> trn -> s1
		asmcDspn.AddInputArc(states[source(*eit, asmc->graph)], trn);
		asmcDspn.AddOutputArc(trn, states[target(*eit, asmc->graph)]);
		
		// arc to "ASMC moved"
		asmcDspn.AddOutputArc(trn, mov);
		
		// arcs that adds or removes valid SP from s0 to s1
		vector<string>::const_iterator spExprIt;
		for (size_t spi = 0; spi < spExprNames.size(); spi++) {
			DTA::location_t loc = spExprDtaLocs[spExprNames[spi]];
			const DtaLocLabel *spExpr = get(dta->locationPMap, loc).label.get();
			bool spi_before = isSpExprSatisfiedInAsmcState(spExpr, source(*eit, asmc->graph));
			bool spi_after  = isSpExprSatisfiedInAsmcState(spExpr, target(*eit, asmc->graph));
			if (spi_before == false && spi_after == true)
				asmcDspn.AddOutputArc(trn, SPExprs[spi]);
			else if (spi_before == true && spi_after == false)
				asmcDspn.AddInputArc(SPExprs[spi], trn);
		}
		
		// arcs that triggers the action sets activated by ep.actionID
		for (size_t asi = 0; asi < actSetNames.size(); asi++)
		{
			const string& actSetExprString = actSetNames[asi];
			const DTA::EdgeP& actEp = get(dta->edgePMap, actSetDtaEdges[actSetExprString]);
			
			// Find if the Action set "actions" of this dta edge contains the action
			// triggered by tha asmc move. Note that in the DTA we save the action
			// names as "parameterized actions", while the real asmc action names
			// are in the dtapar structure.
			vector<ActionID>::const_iterator actIdIt;
			bool haveAction = false;
			for (actIdIt = actEp.actions.begin(); actIdIt != actEp.actions.end(); ++actIdIt)
				if (*actIdIt == ep.actionID) {
					assert(dta->Acts.GetSymbolName(*actIdIt) == asmc->Actions.GetSymbolName(ep.actionID));
					haveAction = true;
					break;
				}
			
			if ((actEp.actSetType == AST_INCLUDED_ACTs && haveAction) ||
				(actEp.actSetType == AST_EXCLUDED_ACTs && !haveAction))
			{
				asmcDspn.AddOutputArc(trn, ActSets[asi]);
			}
		}
	}
}


// Create the Clock Net DSPN
void ProbTaSolver::CreateClockDSPN(DSPN& clkDspn)
{
	size_t clk = clkDspn.AddPlace(PLN_CLOCK, 0, CLNET_CLK);
	size_t run = clkDspn.AddPlace(PLN_RUNNING, 1);

	for (size_t i=0; i<clkBoundNames.size(); i++) {
		size_t xEqCv = clkDspn.AddPlace(clkBoundNames[i], i==0, CLNET_CLK);
		size_t tstop = clkDspn.AddImmTran("stop", 1, Pri_StopClkBase+i+1, CLNET_CLK);
		clkDspn.AddInputArc(xEqCv, tstop);
		clkDspn.AddInhibArc(run, tstop);
		
		if (i == 0) { // Place fo X = 0
			size_t reset = clkDspn.AddImmTran("reset X", 1, Pri_ResetClk, CLNET_CLK);
			size_t tc = clkDspn.AddImmTran("", 1, Pri_Clk0, CLNET_CLK);
			clkDspn.AddInputArc(clk, reset);
			clkDspn.AddInputOutputArcs(xEqCv, reset);
			clkDspn.AddInputArc(xEqCv, tc);
			clkDspn.AddOutputArc(tc, clk, 1);
		}
		else { // Place for X = CV
			size_t tdet = clkDspn.AddDetTran((format("%1% < X < %2%")
											  % dta->CCVals[i-1]
											  % dta->CCVals[i]).str(), 
											 dta->CCVals[i] - dta->CCVals[i-1],
											 DSPN::SingleSrv, CLNET_CLK);
			size_t tc = clkDspn.AddImmTran("", 1, Pri_Clk, CLNET_CLK);
			clkDspn.AddInputOutputArcs(run, tdet);
			clkDspn.AddInputArc(clk, tdet, i);
			//clkDspn.AddInputOutputArcs(stop, tdet);
			clkDspn.AddInhibArc(clk, tdet, i+1);
			clkDspn.AddOutputArc(tdet, xEqCv);
			clkDspn.AddInputArc(xEqCv, tc);
			clkDspn.AddOutputArc(tc, clk, i+1);
		}
	}
	
	size_t t = clkDspn.AddImmTran("stop", 1, Pri_StopClkBase, CLNET_CLK);
	clkDspn.AddInputArc(clk, t);
	clkDspn.AddInhibArc(run, t);
	
}


// Create the DTA DSPN
void ProbTaSolver::CreateDtaDSPN(DSPN& dtaDspn)
{
	size_t run = dtaDspn.AddPlace(PLN_RUNNING, 1);
	size_t mov = dtaDspn.AddPlace(PLN_ASMCMOVED, 0);
	size_t top = dtaDspn.AddPlace(PLN_TOP, 0);
	size_t clk = dtaDspn.AddPlace(PLN_CLOCK, 0);
	size_t dtamoved = -1;
	if (isDtaSemiDet) {
		dtamoved = dtaDspn.AddPlace(PLN_DTAMOVED, 0, CLNET_DTA);
		size_t ndet = dtaDspn.AddPlace(PLN_NONDET, 0);
		size_t idt = dtaDspn.AddImmTran("Inner Det Test", 1, Pri_DetTest, CLNET_DTA);
		dtaDspn.AddInputArc(dtamoved, idt, 2);
		dtaDspn.AddInputArc(mov, idt);
		dtaDspn.AddOutputArc(idt, ndet);
		size_t bdt = dtaDspn.AddImmTran("Boundary Det Test", 1, Pri_DetTest, CLNET_DTA);
		dtaDspn.AddInputArc(dtamoved, bdt, 2);
		dtaDspn.AddOutputArc(bdt, ndet);
		dtaDspn.AddInhibArc(mov, bdt);
	}
	
	// add State Proposition Expr, ActSetExpr and clock(X=K) places
	vector<string>::const_iterator it;
	vector<size_t> SPExprs;
	vector<size_t> ActSets;
	vector<size_t> ClkAtX;
	for (it = spExprNames.begin(); it != spExprNames.end(); ++it)
		SPExprs.push_back(dtaDspn.AddPlace(*it, 0));
	for (it = actSetNames.begin(); it != actSetNames.end(); ++it)
		ActSets.push_back(dtaDspn.AddPlace(*it, 0));
	for (it = clkBoundNames.begin(); it != clkBoundNames.end(); ++it)
		ClkAtX.push_back(dtaDspn.AddPlace(*it, 0));
	
	// add DTA Locations
	DTA::location_iter_t lit, lit_end;
	vector<size_t> locPlaces(num_vertices(dta->graph));
	for (tie(lit, lit_end) = locations(dta->graph); lit != lit_end; ++lit) {
		const DTA::LocationP& lp = get(dta->locationPMap, *lit);
		locPlaces[*lit] = dtaDspn.AddPlace(lp.locName, 0, CLNET_DTA);
		size_t t = dtaDspn.AddImmTran("stop", 1, Pri_StopDtaLocBase + *lit, CLNET_DTA);
		dtaDspn.AddInputArc(locPlaces[*lit], t);
		if (lp.flags & DLF_FINAL)
			dtaDspn.AddOutputArc(t, top);
		else
			dtaDspn.AddInhibArc(run, t);
	}

	// add DTA edges
	size_t edgeCount = 0;
	DTA::edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(dta->graph); eit != eit_end; ++eit) 
	{
		const DTA::EdgeP& ep = get(dta->edgePMap, *eit);
		const DTA::LocationP& srcLp = get(dta->locationPMap, source(*eit, dta->graph));
		const DTA::LocationP& dstLp = get(dta->locationPMap, target(*eit, dta->graph));
		string trnName = (format("%1% -> %2%") % srcLp.locName % dstLp.locName).str();
		size_t tA = dtaDspn.AddImmTran(trnName, 1, Pri_DtaAEdgesBase + edgeCount, CLNET_DTA);
		
		// Find the SP-expression label of the target location
		const DTA::LocationP& dstLoc = get(dta->locationPMap, 
										   target(*eit, dta->graph));
		ostringstream spExpr;
		dstLoc.label->PrintExpr(spExpr, dta->SPs);
		size_t spePlace = (find(spExprNames.begin(), spExprNames.end(), 
								spExpr.str()) - spExprNames.begin());
		assert(spePlace < spExprNames.size());
		
		// Find the ActExpr required
		size_t actSetReq = size_t(-1);
		if (ep.isInnerEdge())
			actSetReq = (find(actSetNames.begin(), actSetNames.end(),
							  ep.GetActSetDescription(dta->Acts)) -
						 actSetNames.begin());
		
		
		if (!isDtaSemiDet) {
			// arcs:  l0 -> edge -> l1
			dtaDspn.AddInputArc(locPlaces[source(*eit, dta->graph)], tA);
			dtaDspn.AddOutputArc(tA, locPlaces[target(*eit, dta->graph)]);
			
			if (ep.isInnerEdge()) {
				// arc from "ASMC moved"
				dtaDspn.AddInputArc(mov, tA);
				
				// guard test arcs:  A<X<B
				dtaDspn.AddInputOutputArcs(clk, tA, ep.alpha+1);
				if (ep.beta < dta->CCVals.size())
					dtaDspn.AddInhibArc(clk, tA, ep.beta+1);
				
				// arc: from the action set expression required
				dtaDspn.AddInputArc(ActSets[actSetReq], tA);
			}
			else { // ep is a boundary edge
				// guard test arc:  X=a
				dtaDspn.AddInputOutputArcs(ClkAtX[ep.alpha], tA);
			}
			
			// SP expression required in the destination location
			dtaDspn.AddInputOutputArcs(SPExprs[spePlace], tA);
		}
		else { // semi-deterministic DTA
			// l0 --[tS]--> placeS --[tA]--> l1
			size_t placeS = dtaDspn.AddPlace((format("S(%1%)")%trnName).str(),
											 0, CLNET_DTA);
			size_t tS = dtaDspn.AddImmTran((format("Select(%1%)")%trnName).str(), 
										   1, Pri_DtaSEdgesBase + edgeCount, CLNET_DTA);
			
			dtaDspn.AddInputOutputArcs(locPlaces[source(*eit, dta->graph)], tS);
			dtaDspn.AddOutputArc(tS, placeS);
			dtaDspn.AddInhibArc(placeS, tS);
			dtaDspn.AddInputArc(placeS, tA);
			dtaDspn.AddInputArc(locPlaces[source(*eit, dta->graph)], tA);
			dtaDspn.AddOutputArc(tA, locPlaces[target(*eit, dta->graph)]);
			size_t tStopS = dtaDspn.AddImmTran("stop", 1, Pri_StopDtaSelBase + edgeCount, 
											   CLNET_DTA);
			dtaDspn.AddInputArc(placeS, tStopS);
			dtaDspn.AddInhibArc(run, tStopS);
			
			// DTA moved condition
			dtaDspn.AddOutputArc(tS, dtamoved);
			dtaDspn.AddInputArc(dtamoved, tA);
			
			// SP-expression test
			dtaDspn.AddInputOutputArcs(SPExprs[spePlace], tS);
			
			if (ep.isInnerEdge()) {
				// arc from "ASMC moved"
				dtaDspn.AddInputOutputArcs(mov, tS);
				dtaDspn.AddInputArc(mov, tA);
				
				// guard test arcs:  A<X<B
				dtaDspn.AddInputOutputArcs(clk, tS, ep.alpha+1);
				if (ep.beta < dta->CCVals.size())
					dtaDspn.AddInhibArc(clk, tS, ep.beta+1);
				
				// arc: from the action set expression required
				dtaDspn.AddInputOutputArcs(ActSets[actSetReq], tS);
				dtaDspn.AddInputArc(ActSets[actSetReq], tA);
			}
			else { // ep is a boundary edge
				// guard test arc:  X=a
				dtaDspn.AddInputOutputArcs(ClkAtX[ep.alpha], tS);
			}
		}
		
		// reset arc
		if (ep.reset) {
			dtaDspn.AddOutputArc(tA, ClkAtX[0]);
		}
		
		edgeCount++;
	}
}


void ProbTaSolver::PrepareInitMarkVector()
{
	// Every initial mark in this DSPN should have this form:
	//
	//   M0 = < Running, X=0, s(a), l(j), TrueSPExpr(i) >
	//
	// where s(a) is the evaluated ASMC state, l(j) the corresponding
	// initial location in the DTA, and TrueSPExpr(i) are the SPExpr
	// true in s(a).
	// Note that <Running, X=0> is already set in the DSPN, so we don't 
	// write them in the initMarks vector.
	initMarks.resize(num_vertices(asmc->graph));
	for (size_t a = 0; a < num_vertices(asmc->graph); a++) {
		const ASMC::VertexP& vp = get(asmc->vertPMap, a);
		// Add the ASMC state
		size_t plSa = dspn.FindPlaceByName(vp.vertexName.c_str());
		assert(plSa != (size_t)-1);
		initMarks[a].push_back(plSa);
		
		// Add the corresponding Initial location in the DTA
		for (size_t l = 0; l < num_vertices(dta->graph); l++) {
			const DTA::LocationP& lp = get(dta->locationPMap, l);
			if (!(lp.flags & DLF_INITIAL))
				continue;
			if (isSpExprSatisfiedInAsmcState(lp.label.get(), a)) {
				size_t plLj = dspn.FindPlaceByName(lp.locName.c_str());
				assert(plLj != (size_t)-1);
				initMarks[a].push_back(plLj);
			}
		}
		assert(initMarks[a].size() <= 2);  // Otherwise the DTA is not deterministic
		if (initMarks[a].size() == 1) {
			// There is no initial location in the DTA that satisifies s(a)
			// Start the DSPN in the bottom location
			initMarks[a].push_back(dspn.FindPlaceByName(PLN_BOT));
		}
		
		// Now add the initial marks for the SP expressions that are true in plSa
		for (size_t spi = 0; spi < spExprNames.size(); spi++) {
			const DTA::location_t loc = spExprDtaLocs[spExprNames[spi]];
			DtaLocLabel *spExpr = get(dta->locationPMap, loc).label.get();
			if (isSpExprSatisfiedInAsmcState(spExpr, a)) {
				size_t plSp = dspn.FindPlaceByName(spExprNames[spi].c_str());
				assert(plSp != (size_t)-1);
				initMarks[a].push_back(plSp);
			}
		}
	}
}


bool ProbTaSolver::isSpExprSatisfiedInAsmcState(const DtaLocLabel* spExpr, 
												ASMC::vertex_t asmcState)
{
	vector<bool> spTruthVals(dta->SPs.NumSymbols());
	
	// Setup the boolean values assumed by State Props when the ASMC is
	// in state st
	for (size_t i=0; i<dta->SPs.NumSymbols(); i++) {
		spTruthVals[i] = (*subExprRes)[i].eval[asmcState];
	}
	
	// Evaluate the DTA locaton label
	return spExpr->Evaluate(spTruthVals);
}


bool ProbTaSolver::WriteSolverDspnAsGml(ostream& os, const char *name, 
										const char *args)
{
	DSPN *p = NULL;
	bool writeUID = false;
	
	string arguments(args);
	tokenizer<> tok(arguments);
	for(tokenizer<>::iterator beg=tok.begin(); beg!=tok.end();++beg) {
		if (!_GetDspnByName((*beg).c_str(), p)) {
			if (*beg == "UID")
				writeUID = true;
			else {
				cout << "Unrecognized option \""<<*beg<<"\"." << endl;
				return false;
			}
		}
	}
	if (p == NULL) {
		cout << "Missing valid SAVE_GML arguments." << endl;
		return false;
	}


	for (size_t i=0; i<initMarks[0].size(); i++)
		dspn.places[ initMarks[0][i] ].initMark = 1;
	
	p->WriteAsGmlFile(os, name, writeUID);	
	
	for (size_t i=0; i<initMarks[0].size(); i++)
		dspn.places[ initMarks[0][i] ].initMark = 0;
	
	return true;
}


bool ProbTaSolver::LoadDspnCoords(istream& is, const char *args)
{
	DSPN *p;
	if (!_GetDspnByName(args, p)) {
		cout << "Unrecognized DSPN name \""<<args<<"\"." << endl;
		return false;
	}	
	p->ImportGmlCoords(is);
	return true;
}


bool ProbTaSolver::PlanarizeDspn(const char *args)
{
	DSPN *p;
	if (!_GetDspnByName(args, p)) {
		cout << "Unrecognized DSPN name \""<<args<<"\"." << endl;
		return false;
	}	
	return p->Planarize();
}	

bool ProbTaSolver::GetDspnToSave(const char *args, DSPN& outDspn)
{
	DSPN *p;
	if (!_GetDspnByName(args, p)) {
		cout << "Unrecognized DSPN name \""<<args<<"\"." << endl;
		return false;
	}
		
	outDspn = *p;
	if (p == &dspn) {
		for (size_t i=0; i<initMarks[0].size(); i++)
			outDspn.places[ initMarks[0][i] ].initMark = 1;
	}
	
	return true;
}


bool ProbTaSolver::_GetDspnByName(const char *name, DSPN* &p) {
	if      (0==strcmp(name, "DSPN"))		p = &dspn;
	else if (0==strcmp(name, "SP"))			p = &spDspn;
	else if (0==strcmp(name, "FINAL"))		p = &finDspn;
	else if (0==strcmp(name, "ACT"))		p = &actDspn;
	else if (0==strcmp(name, "ASMC"))		p = &asmcDspn;
	else if (0==strcmp(name, "DTA"))		p = &dtaDspn;
	else if (0==strcmp(name, "CLOCK"))		p = &clkDspn;
	else return false;
	
	return true;
}


bool ProbTaSolver::SolveInState(size_t s, double &outProb) const
{
	// --------------------------------
	//             SPNica
	// --------------------------------
	if (g_status.extSolverUsed == EDS_SPNica) 
	{
		Pipe& mathKernel = g_status.GetMathKernel();
		DSPN prepDspn = dspn;
		
		// Setup Initial Marking
		for (size_t m=0; m<initMarks[s].size(); m++)
			prepDspn.places[ initMarks[s][m] ].initMark = 1;
		
		// Prepare the SPNica DSPN starting in s
		prepDspn.AddInitialTangibleMark();
		prepDspn.AddAbsorbingLoops((const char**)s_absorbPlaces, 
								   NUM_ABSORBING_PLACES);
		ostringstream os;
		prepDspn.WriteAsSPNicaFile(os, "prob_ta_dspn");
		
		os << "SOL = SPNstationary[SPN];\n";
		os << "pT = N[(MeanPxTop /. SOL)];\n";
		os << "pND = (MeanPxNonDet /. SOL) /. MeanPxNonDet -> 0;\n";
		os << "Print[DETERMINISTIC];\n";
		os << "Print[(pND == 0)];\n";
		os << "Print[SUCCESS_PROB];\n";
		os << "Print[N[pT,10]];\n";
		os << "Print[ALLDONE];\n";
		os << "\n";
		if (!mathKernel.Write(os.str())) {
			cout << "Coudn't send commands to the MathKernel." << endl;
			return false;
		}
		
		// Read back the solutions
		bool det;
		char tokBuf[256];
		mathKernel.SkipUntil("DETERMINISTIC");
		mathKernel.ReadLine(tokBuf, sizeof(tokBuf), true);
		det = (0==strcmp(tokBuf, "True"));
		mathKernel.SkipUntil("SUCCESS_PROB");
		mathKernel.ReadDouble(&outProb);
		mathKernel.SkipUntil("ALLDONE");
		if (!mathKernel) {
			cout << "Couldn't read back the MathKernel output." << endl;
			return false;
		}
		
		if (!det) {
			cout << "The DTA is not deterministic. Couldn't evaluate this CSL-TA expression." << endl;
			return false;
		}
		
		return true;
	}
	// --------------------------------
	//          MRP Solver (BuildTRG)
	// --------------------------------
	else if (g_status.extSolverUsed == EDS_MRPSolver)
	{
		DSPN prepDspn = dspn;
		
		// Setup Initial Marking
		for (size_t m=0; m<initMarks[s].size(); m++)
			prepDspn.places[ initMarks[s][m] ].initMark = 1;
		
		// Prepare temporary files
		char baseNetName [L_tmpnam], netName[256], defName[256], solName[256];
#ifdef WIN32
		char *tmpFile = tempnam(TEMPNAM_DIR, "cslta-tmp-");
		if (tmpFile == NULL) {
			cout << "ERROR: Couldn't create temporary Petri Net files." << endl;
			return false;
		}
		strcpy(baseNetName, tmpFile);
		free(tmpFile);
#else
		tmpnam(baseNetName);
#endif
		sprintf(netName, "%s.net", baseNetName);
		sprintf(defName, "%s.def", baseNetName);
		sprintf(solName, "%s.results", baseNetName);

		// Write the DSPN in the GreatSPN format
		ofstream gspnNet(netName);
		ofstream gspnDef(defName);
		const char *measures[2] = {
			"|Success 2.0 0.2 : E{#PxTop};\n"
			"|Failure 3.5 0.2 : E{#PxBot};\n",
			"|Success 2.0 0.2 : E{#PxTop};\n"
			"|Failure 3.5 0.2 : E{#PxBot};\n"
			"|NonDet  5.0 0.2 : E{#PxNonDet};\n",
		};
		prepDspn.WriteAsGreatSPNFile(gspnNet, gspnDef, "net",
									 measures[isDtaSemiDet ? 1 : 0]);	
		gspnNet.close();
		gspnDef.close();
		
		// Invoke the MRP Solver
		ostringstream cmd;
		cmd << g_status.mrpPath << " " << g_status.extraArgs;
		cmd << " -load " << baseNetName << " -trg -s -wr ";
		if (g_status.mrpLogFile.size() > 0) {
			cmd << " > " << g_status.mrpLogFile;
#ifndef WIN32
			cmd << " 2> " << g_status.mrpLogFile;
#endif
		}
		system(cmd.str().c_str());
		
		// Read back the solutions
		ifstream dspnSol(solName);
		double failProb, nDetProb;
		dspnSol >> outProb >> failProb;
		if (isDtaSemiDet)
			dspnSol >> nDetProb;
		dspnSol.close();
		
		// Clean up
		remove(netName);
		remove(defName);
		remove(solName);//*/
		
		if (!dspnSol) {
			cout << "Couldn't read back the solutions of the DSPN." << endl;
			return false;
		}		
		if (isDtaSemiDet) {
			dspnSol >> nDetProb;
			if (nDetProb != 0) {
				cout << "The DTA is not deterministic. Couldn't evaluate this CSL-TA expression." << endl;
				return false;
			}
		}
		return true;
	}
	// --------------------------------
	//          Test Solver
	// --------------------------------
	else if (g_status.extSolverUsed == EDS_TestSolver)
	{
		DSPN prepDspn = dspn;
		string NetNamePrefix = g_status.testSolverDspnName + "DSPN";
		const char *baseNetName = NetNamePrefix.c_str();
		
		// Setup Initial Marking	
		for (size_t m=0; m<initMarks[s].size(); m++)
			prepDspn.places[ initMarks[s][m] ].initMark = 1;
		
		// Save in GreatSPN format
		do {
			char greatSpnNameNet[256], greatSpnNameDef[256];
			sprintf(greatSpnNameNet, "%s-GreatSPN.net", baseNetName);
			sprintf(greatSpnNameDef, "%s-GreatSPN.def", baseNetName);
			ofstream gspnNet(greatSpnNameNet);
			ofstream gspnDef(greatSpnNameDef);
			const char *measures[2] = {
				"|Success 2.0 0.2 : E{#PxTop};\n"
				"|Failure 3.5 0.2 : E{#PxBot};\n",
				"|Success 2.0 0.2 : E{#PxTop};\n"
				"|Failure 3.5 0.2 : E{#PxBot};\n"
				"|NonDet  5.0 0.2 : E{#PxNonDet};\n",
			};
			prepDspn.WriteAsGreatSPNFile(gspnNet, gspnDef, "net",
										 measures[isDtaSemiDet ? 1 : 0]);	
			gspnNet.close();
			gspnDef.close();
		} while(0);	
		
		size_t startPlace = prepDspn.AddInitialTangibleMark(1.0);
		
		// Save in SPNica format
		do {
			DSPN dspn2 = prepDspn;
			dspn2.AddAbsorbingLoops((const char**)s_absorbPlaces, 
									NUM_ABSORBING_PLACES);
			char spnicaName[256];
			sprintf(spnicaName, "%s-SPNica.nb", baseNetName);
			ofstream spnica(spnicaName);
			dspn2.WriteAsSPNicaFile(spnica, "net");
			spnica.close();
		} while(0);
		
		// Close TOP, BOTTOM and NONDET markings
		size_t backTop = prepDspn.AddExpTran("T restart", 1.0);
		size_t backBot = prepDspn.AddExpTran("⊥ restart", 1.0);
		prepDspn.AddInputArc(prepDspn.FindPlaceByName(PLN_TOP), backTop);
		prepDspn.AddOutputArc(backTop, startPlace);
		prepDspn.AddInputArc(prepDspn.FindPlaceByName(PLN_BOT), backBot);
		prepDspn.AddOutputArc(backBot, startPlace);
		if (isDtaSemiDet) {
			size_t backNDet = prepDspn.AddExpTran("NonDet restart", 1);
			prepDspn.AddInputArc(prepDspn.FindPlaceByName(PLN_NONDET), backNDet);
			prepDspn.AddOutputArc(backNDet, startPlace);
		}
		
		// Save in TimeNET format
		do {
			char timeNetName[256];
			sprintf(timeNetName, "%s-TimeNET.TN", baseNetName);
			ofstream timeNet(timeNetName);
			prepDspn.WriteAsTimeNetFile(timeNet, "net");
			timeNet.close();
		} while(0);
		
		// Save in DSPNexpress format
		do {
			char dspnExprNet[256], dspnExprDef[256];
			sprintf(dspnExprNet, "%s-DSPNExpress.net", baseNetName);
			sprintf(dspnExprDef, "%s-DSPNExpress.def", baseNetName);
			ofstream net(dspnExprNet);
			ofstream def(dspnExprDef);
			prepDspn.WriteAsDSPNexpressFile(net, def, "prob_ta");
			net.close();
			def.close();
		} while(0);
		
		cout << " >>> Test Solver: DSPN saved in varius formats as ";
		cout << baseNetName << "* <<<" << endl;
		return false;
	}
	// --------------------------------
	//          DSPNexpress
	// --------------------------------
	else if (g_status.extSolverUsed == EDS_DSPNexpress)
	{
		cout << "ProbTaSolver::SolveInState("<<s<<")" << endl;
		DSPN prepDspn = dspn;

		// STEP1: Do the TestRun, reading back the TRG.
		do {		
			// Base name for the DSPN files
			char baseNetName [L_tmpnam], netName[256], defName[256];
			tmpnam(baseNetName);
			sprintf(netName, "%s-WriteUrgOnly.NET", baseNetName);
			sprintf(defName, "%s-WriteUrgOnly.DEF", baseNetName);

			// Setup Initial Marking	
			for (size_t m=0; m<initMarks[s].size(); m++)
				prepDspn.places[ initMarks[s][m] ].initMark = 1;
			
			// TODO: rimuovere
			do {
				char greatSpnNameNet[256], greatSpnNameDef[256];
				sprintf(greatSpnNameNet, "%s-GreatSPN.net", baseNetName);
				sprintf(greatSpnNameDef, "%s-GreatSPN.def", baseNetName);
				ofstream gspnNet(greatSpnNameNet);
				ofstream gspnDef(greatSpnNameDef);
				const char *measures[2] = {
					"|Success 2.0 0.2 : E{#PxTop};\n"
					"|Failure 3.5 0.2 : E{#PxBot};\n",
					"|Success 2.0 0.2 : E{#PxTop};\n"
					"|Failure 3.5 0.2 : E{#PxBot};\n"
					"|NonDet  5.0 0.2 : E{#PxNonDet};\n",
				};
				prepDspn.WriteAsGreatSPNFile(gspnNet, gspnDef, "net",
											 measures[isDtaSemiDet ? 1 : 0]);	
				gspnNet.close();
				gspnDef.close();
				//return false;
			} while(0);			
				
			size_t startPlace = prepDspn.AddInitialTangibleMark(1.0);

			// TODO: rimuovere
			do {
				DSPN dspn2 = prepDspn;
				dspn2.AddAbsorbingLoops((const char**)s_absorbPlaces, 
										NUM_ABSORBING_PLACES);
				char spnicaName[256];
				sprintf(spnicaName, "%s-X.nb", baseNetName);
				ofstream spnica(spnicaName);
				dspn2.WriteAsSPNicaFile(spnica, "net");
				spnica.close();
			} while(0);

			// Close TOP, BOTTOM and NONDET markings
			size_t backTop = prepDspn.AddExpTran("T restart", 1.0);
			size_t backBot = prepDspn.AddExpTran("⊥ restart", 1.0);
			prepDspn.AddInputArc(prepDspn.FindPlaceByName(PLN_TOP), backTop);
			prepDspn.AddOutputArc(backTop, startPlace);
			prepDspn.AddInputArc(prepDspn.FindPlaceByName(PLN_BOT), backBot);
			prepDspn.AddOutputArc(backBot, startPlace);
			if (isDtaSemiDet) {
				size_t backNDet = prepDspn.AddExpTran("NonDet restart", 1);
				prepDspn.AddInputArc(prepDspn.FindPlaceByName(PLN_NONDET), backNDet);
				prepDspn.AddOutputArc(backNDet, startPlace);
			}
			
			// TODO: rimuovere
			do {
				char timeNetName[256];
				sprintf(timeNetName, "%s-X.TN", baseNetName);
				ofstream timeNet(timeNetName);
				prepDspn.WriteAsTimeNetFile(timeNet, "net");
				timeNet.close();
			} while(0);//*/
						
			// Write the DSPN files
			ofstream net(netName);
			ofstream def(defName);
			prepDspn.WriteAsDSPNexpressFile(net, def, "prob_ta");
			if (!net || !def) {
				cout << "Couldn't write DSPNexpress net files." << endl;
				return false;
			}
			net.close();
			def.close();
			return false; // TODO: rimuovere
			
			// Invoke DSPNexpress
			ostringstream cmd;
			cmd << g_status.dspnExpressPath << DIRSEP << "SOLVE ";
			cmd << baseNetName << "-WriteUrgOnly";
			cmd << " > /dev/null 2> /dev/null";
			system(cmd.str().c_str());
			
			// Invoke readURG
			ostringstream readUrgCmd;
			char urgName[256], bsccName[256];
			sprintf(urgName, "%s-WriteUrgOnly.URG", baseNetName);
			sprintf(bsccName, "%s-WriteUrgOnly.BSCC", baseNetName);
			readUrgCmd << g_status.readUrgPath << " " << urgName << " savebscc";
			//readUrgCmd << " doublearcs";
			system(readUrgCmd.str().c_str());
						
			// Write the names of places and transitions
			// TODO: comment out
			char ptnamesName[256];
			sprintf(ptnamesName, "%s-WriteUrgOnly.PTNAMES", baseNetName);
			ofstream ptnames(ptnamesName);
			for (size_t p=0; p<prepDspn.places.size(); p++)
				ptnames << prepDspn.places[p].name << "\n";
			for (size_t t=0; t<prepDspn.transitions.size(); t++)
				ptnames << prepDspn.transitions[t].name << "\n";
			ptnames.close();

			// Read back the BSCC file and modify the DSPN accordingly
			ifstream inBscc(bsccName);
			prepDspn.CloseBSCCs(inBscc, PLN_BSCCSINK, 1.0, startPlace);
			inBscc.close();
			
			// Remove double arcs
			/*char dblArcName[256];
			sprintf(dblArcName, "%s-WriteUrgOnly.DOUBLEARCS", baseNetName);
			ifstream dblArcList(dblArcName);*/
			size_t Nplaces[4];
			Nplaces[0] = prepDspn.FindPlaceByName(PLN_TOP);
			Nplaces[1] = prepDspn.FindPlaceByName(PLN_BOT);
			Nplaces[2] = prepDspn.FindPlaceByName(PLN_NONDET);
			Nplaces[3] = prepDspn.FindPlaceByName(PLN_BSCCSINK);
			prepDspn.RemoveDoubleArcs(Nplaces, 4);
			//dblArcList.close();
			
			// Clean all the DSPNexpress files
			/*char removedName[256];
			for (size_t i=0; i<DSPNexpressTmpFileExts_NUM; i++) {
				sprintf(removedName, "%s-WriteUrgOnly.%s", baseNetName, 
						DSPNexpressTmpFileExts[i]);
				remove(removedName);
			}*/
		} while(0);
		
		// STEP2: now the prepDspn is ergodic, it can be solved with DSPNexpress
		do {
			char baseNetName [L_tmpnam], netName[256], defName[256];
			tmpnam(baseNetName);
			sprintf(netName, "%s.NET", baseNetName);
			sprintf(defName, "%s.DEF", baseNetName);
			
			// Prepare the Reward
			ostringstream reward;
			size_t plTop = prepDspn.FindPlaceByName(PLN_TOP);
			size_t plBot = prepDspn.FindPlaceByName(PLN_BOT);
			size_t plNDet = prepDspn.FindPlaceByName(PLN_NONDET);
			size_t plBscc = prepDspn.FindPlaceByName(PLN_BSCCSINK);
			string TopName =  prepDspn.GetDSPNexprName(prepDspn.places[plTop].name, 'P', plTop);
			string BotName =  prepDspn.GetDSPNexprName(prepDspn.places[plBot].name, 'P', plBot);

			reward << "\nMEASURE Top\n";
			reward << "E{#" << TopName << "} / (";
			reward << "E{#" << TopName << "}";
			reward << " + E{#" << BotName << "}";
			if (plBscc != size_t(-1))
				reward << "+ E{#" 
					   << prepDspn.GetDSPNexprName(prepDspn.places[plBscc].name, 
												   'P', plBscc) << "}";
			reward << ");\n";
			if (isDtaSemiDet)
				reward << "\nMEASURE NonDet\nE{#" 
					   << prepDspn.GetDSPNexprName(prepDspn.places[plNDet].name, 
												   'P', plNDet) << "};\n";

			// TODO: DEBUG Remove all this code
			char gmlName[256], spnicaName[256];
			sprintf(gmlName, "%s-XUID.GML", baseNetName);
			prepDspn.WriteAsGmlFile(gmlName, "", true);
			sprintf(gmlName, "%s-X.GML", baseNetName);
			prepDspn.WriteAsGmlFile(gmlName, "", false);
			sprintf(spnicaName, "%s-X.nb", baseNetName);
			ofstream spnica(spnicaName);
			prepDspn.WriteAsSPNicaFile(spnica, "net");
			spnica.close();

			// TODO: rimuovere
			do {
				char timeNetName[256];
				sprintf(timeNetName, "%s-X.TN", baseNetName);
				ofstream timeNet(timeNetName);
				prepDspn.WriteAsTimeNetFile(timeNet, "net");
				timeNet.close();
			} while(0);//*/

			// Write the DSPN files
			ofstream net(netName);
			ofstream def(defName);
			prepDspn.WriteAsDSPNexpressFile(net, def, "prob_ta", 
											reward.str().c_str());
			if (!net || !def) {
				cout << "Couldn't write DSPNexpress net files." << endl;
				return false;
			}
			net.close();
			def.close();
			
			// Call DSPNexpress
			ostringstream cmd;
			cmd << g_status.dspnExpressPath << DIRSEP << "SOLVE -d " << baseNetName;
			cmd << " > /dev/null";// 2> /dev/null";
			system(cmd.str().c_str());
			
			// read back the solution
			bool success = false;
			char resultName[256];
			sprintf(resultName, "%s.RESULTS", baseNetName);
			FILE *resultFile = fopen(resultName, "r");
			if (resultFile != NULL) {
				if (1 == fscanf(resultFile, "Top = %lf", &outProb)) {
					double nondetProb = 0.0;
					if (isDtaSemiDet)
						fscanf(resultFile, "NonDet = %lf", &nondetProb);
					if (nondetProb > 0.0)
						cout << ("The DTA is not deterministic. Couldn't evaluate"
								 " this CSL-TA expression.") << endl;
					else success = true;
				}
				fclose(resultFile);
			}
			
			// Write the names of places and transitions
			// TODO: comment out
			char ptnamesName[256];
			sprintf(ptnamesName, "%s.PTNAMES", baseNetName);
			ofstream ptnames(ptnamesName);
			for (size_t p=0; p<prepDspn.places.size(); p++)
				ptnames << prepDspn.places[p].name << "\n";
			for (size_t t=0; t<prepDspn.transitions.size(); t++)
				ptnames << prepDspn.transitions[t].name << "\n";
			ptnames.close();
			
			// Clean all the DSPNexpress files
			/*char removedName[256];
			for (size_t i=0; i<DSPNexpressTmpFileExts_NUM; i++) {
				sprintf(removedName, "%s.%s", baseNetName, DSPNexpressTmpFileExts[i]);
				remove(removedName);
			}*/
			
			if (!success)
				cout << "Couldn't read back DSPNexpress solutions." << endl;
			return success;
		} while (0);
	}
	
	
	cout << "Don't known how to solve a P() espression with ";
	cout << "the current DSPN solver." << endl;
	return false;
}

//-----------------------------------------------------------------------------












//-----------------------------------------------------------------------------

SteadySolver::SteadySolver(ASMC *a, CslTaResult *r)
/**/ : asmc(a), phi(r)
{ }


void SteadySolver::CreateDSPN()
{
	const char *CLUSTER = "ASMC";
	// The DSPN needed for the STEADY operator is very straightforward: each
	// state/transition in the ASMC becames a place/transition in the DSPN.
	
	// create the places
	vector<size_t> places(num_vertices(asmc->graph));
	ASMC::vertex_iter_t sit, sit_end;
	for (tie(sit, sit_end) = vertices(asmc->graph); sit != sit_end; ++sit) 
	{
		const ASMC::VertexP& vp = get(asmc->vertPMap, *sit);
		places[*sit] = dspn.AddPlace(vp.vertexName, 0, CLUSTER);
	}
	
	// create the transitions
	ASMC::edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(asmc->graph); eit != eit_end; ++eit) 
	{
		const ASMC::EdgeP& ep = get(asmc->edgePMap, *eit);
		const ASMC::VertexP& srcVp = get(asmc->vertPMap, source(*eit, asmc->graph));
		const ASMC::VertexP& dstVp = get(asmc->vertPMap, target(*eit, asmc->graph));
		string name = (format("%1% -> %2%") % srcVp.vertexName % dstVp.vertexName).str();
		
		size_t trn = dspn.AddExpTran(name, ep.lambda, DSPN::SingleSrv, CLUSTER);
		dspn.AddInputArc(places[source(*eit, asmc->graph)], trn);
		dspn.AddOutputArc(trn, places[target(*eit, asmc->graph)]);
	}
}


bool SteadySolver::WriteSolverDspnAsGml(ostream& os, const char *name, 
										const char *args)
{
	bool writeUID = false;
	if (0==strcmp(args, "UID"))
		writeUID = true;
	
	dspn.places[0].initMark = 1;
	dspn.WriteAsGmlFile(os, name, writeUID);
	dspn.places[0].initMark = 0;
	return true;
}


bool SteadySolver::LoadDspnCoords(istream& is, const char *args) 
{
	dspn.ImportGmlCoords(is);
	return true;
}


bool SteadySolver::PlanarizeDspn(const char *args)
{
	return dspn.Planarize();
}


bool SteadySolver::GetDspnToSave(const char *args, DSPN& outDspn)
{
	outDspn = dspn;
	outDspn.places[0].initMark = 1;
	return true;
}


bool SteadySolver::SolveInState(size_t s, const vector<bool>& Phi, 
								int numBscc, double &outProb)
{
	// --------------------------------
	//             SPNica
	// --------------------------------
	if (g_status.extSolverUsed == EDS_SPNica) 
	{
		Pipe& mathKernel = g_status.GetMathKernel();
		
		// Evaluate the S(phi) operator in state s
		dspn.places[s].initMark = 1;
		
		// Write the DSPN into the command pipe
		ostringstream os;
		dspn.WriteAsSPNicaFile(os, "steady_dspn");
		dspn.places[s].initMark = 0;
		
		os << "SOL = SPNstationary[SPN];\n";
		os << "pT = N[";
		for (size_t i=0; i<Phi.size(); i++) {
			if (Phi[i]) {
				string plName = dspn.GetDSPNexprName(dspn.places[i].name, 'P', i);
				os << "(Mean"<<plName<<" /. SOL) + \n       ";
			}
		}
		os << "0];\n";
		os << "Print[STEADY_PROB];\n";
		os << "Print[N[pT,10]];\n";
		os << "Print[ALLDONE];\n";
		
		if (!mathKernel.Write(os.str())) {
			cout << "Coudn't send commands to the MathKernel." << endl;
			return false;
		}
		
		// Read back the solutions for the steady operator
		mathKernel.SkipUntil("STEADY_PROB");
		mathKernel.ReadDouble(&outProb);
		mathKernel.SkipUntil("ALLDONE");
		if (!mathKernel) {
			cout << "Couldn't read back the MathKernel output." << endl;
			return false;
		}
		
		return true;
	}
	// --------------------------------
	//          MRPSolver
	// --------------------------------
	else if (g_status.extSolverUsed == EDS_MRPSolver)
	{
		// Evaluate the S(phi) operator in state s
		dspn.places[s].initMark = 1;
		
		char *tmpName = tempnam(TEMPNAM_DIR, "cslta-tmp-");
		if (tmpName == NULL) {
			cout << "Couldn't create temporary files." << endl;
			return false;
		}
		string netName(tmpName), defName(tmpName), resName(tmpName);
		netName += ".net";
		defName += ".def";
		resName += ".results";
		
		// Write the Petri Net 
		ostringstream reward;
		reward << "|Success 2.0 0.2 : ";
		for (size_t i=0, cnt=0; i<Phi.size(); i++) {
			if (Phi[i]) {
				string plName = dspn.GetDSPNexprName(dspn.places[i].name, 'P', i+1);
				reward << (cnt++ == 0 ? "" : " + ") << "E{#" << plName << "}";
			}
		}
		reward << ";\n";		
		ofstream net(netName.c_str()), def(defName.c_str());
		dspn.WriteAsGreatSPNFile(net, def, "", reward.str().c_str());
		net.close();
		def.close();
		
		// Execute the solver
		ostringstream cmd;
		cmd << g_status.mrpPath << " -load "<<tmpName<<" -trg -s -wr " << NO_CMD_OUTPUT;
		system(cmd.str().c_str());
		remove(netName.c_str());
		remove(defName.c_str());
		
		// Read back the solution 
		ifstream ifr(resName.c_str());
		ifr >> outProb;
		bool success = ifr.good();
		ifr.close();
		remove(resName.c_str());
		free(tmpName);
		
		return success;
	}
	// --------------------------------
	//          DSPNexpress
	// --------------------------------
	else if (g_status.extSolverUsed == EDS_DSPNexpress)
	{
		if (numBscc > 1) {
			cout << "The DSPNexpress DSPN solver couldn't be used to solve\n";
			cout << "an S() espression over a non-ergodic ASMC." << endl;
			return false;
		}
		
		// Base name for the DSPN files
		char baseNetName [L_tmpnam], netName[256], defName[256];
		tmpnam(baseNetName);
		sprintf(netName, "%s.NET", baseNetName);
		sprintf(defName, "%s.DEF", baseNetName);
		
		// Prepare the reward measure
		ostringstream reward;
		reward << "\nMEASURE TotalProb\n0";
		for (size_t i=0; i<Phi.size(); i++) {
			if (Phi[i]) {
				string plName = dspn.GetDSPNexprName(dspn.places[i].name, 'P', i);
				reward << " + E{#" << plName << "}";
			}
		}
		reward << ";\n";
				
		// Write the DSPN files (.NET & .DEF) in the /tmp directory
		ofstream net(netName);
		ofstream def(defName);
		dspn.places[s].initMark = 1;
		dspn.WriteAsDSPNexpressFile(net, def, "steady", reward.str().c_str());
		dspn.places[s].initMark = 0;
		net.close();
		def.close();
		if (!net || !def) {
			cout << "Couldn't write intermediate DSPNexpress files." << endl;
			return false;
		}
		
		// Call DSPNexpress
		ostringstream cmd;
		cmd << g_status.dspnExpressPath << DIRSEP << "SOLVE " << baseNetName;
		cmd << " > /dev/null 2> /dev/null";
		system(cmd.str().c_str());
		
		// read back the solution
		bool success = false;
		char resultName[256];
		sprintf(resultName, "%s.RESULTS", baseNetName);
		FILE *resultFile = fopen(resultName, "r");
		if (resultFile != NULL) {
			if (1 == fscanf(resultFile, "TotalProb = %lf", &outProb))
				success = true;
			fclose(resultFile);
		}
		
		// Clean all the DSPNexpress files
		/*char removedName[256];
		for (size_t i=0; i<DSPNexpressTmpFileExts_NUM; i++) {
			sprintf(removedName, "%s.%s", baseNetName, DSPNexpressTmpFileExts[i]);
			remove(removedName);
		}*/
		
		if (!success)
			cout << "Couldn't read back DSPNexpress solutions." << endl;
		return success;
	}
	
	
	cout << "Don't known how to solve an S() espression with ";
	cout << "the current DSPN solver." << endl;
	return false;
}


//-----------------------------------------------------------------------------

















//-----------------------------------------------------------------------------

PUBLIC_API void APValExpr_DeleteExpr(APValExpr *expr) {
	delete expr;
}
PUBLIC_API APValExpr* APValExpr_NewConst(const char *val) {
	return new APValExpr_Const(atoi(val));
}
PUBLIC_API APValExpr* APValExpr_NewAtomProp(const char *ap) {
	return new APValExpr_AP(ap);
}
PUBLIC_API APValExpr* APValExpr_NewUnOp(char op, APValExpr *expr) {
	return new APValExpr_UnOp(op, expr);
}
PUBLIC_API APValExpr* APValExpr_NewBinOp(APValExpr *expr1, char op, APValExpr *expr2) {
	return new APValExpr_BinOp(expr1, op, expr2);
}



PUBLIC_API void CslTa_DeleteExpr(CslTaExpr *expr) {
	delete expr;
}
PUBLIC_API CslTaExpr* CslTa_NewTrueFalseExpr(int val) {
	return new CslTaExpr_TF(val != 0);
}
PUBLIC_API CslTaExpr* CslTa_NewAPValExpr(APValExpr* expr, APCmpOp op, const char *cmpVal) {
	return new CslTaExpr_AP(expr, op, atoi(cmpVal));
}
PUBLIC_API CslTaExpr* CslTa_NewNotExpr(CslTaExpr *expr) {
	return new CslTaExpr_NOT(expr);
}
PUBLIC_API CslTaExpr* CslTa_NewAndExpr(CslTaExpr *e1, CslTaExpr *e2) {
	return new CslTaExpr_BOP(e1, e2, BOP_AND);
}
PUBLIC_API CslTaExpr* CslTa_NewOrExpr(CslTaExpr *e1, CslTaExpr *e2) {
	return new CslTaExpr_BOP(e1, e2, BOP_OR);
}
PUBLIC_API CslTaExpr* CslTa_NewImplyExpr(CslTaExpr *e1, CslTaExpr *e2) {
	return new CslTaExpr_BOP(e1, e2, BOP_IMPLY);
}
PUBLIC_API CslTaExpr* CslTa_NewSteadyExpr(ProbabilityConstr pc, CslTaExpr *e1) {
	return new CslTaExpr_STEADY(pc, e1);
}
PUBLIC_API CslTaExpr* CslTa_NewProbTaExpr(ProbabilityConstr pc, DTAParams *dtap) {
	return new CslTaExpr_PROB_TA(pc, dtap);
}




//-----------------------------------------------------------------------------

PUBLIC_API void DTAParams_Delete(DTAParams *dtap) {
	delete dtap;
}
PUBLIC_API DTAParams* DTAParams_New() {
	return new DTAParams;
}

PUBLIC_API DTAParams* DTAParams_AddCCVal(DTAParams *dtap, const char* val) 
{
	double v = atof(val);
	if (v <= 0) {
		GS_SetError((format("Cannot add value %1% in the list of ordered clock values.")
					% val).str().c_str());
		return dtap;
	}		
	/*if (dtap->CCVals.size() > 0) {
		if (dtap->CCVals[dtap->CCVals.size()-1] >= v) {
			GS_SetError((format("Cannot add value %1% in the list after clock value "
								"%2%.\n. The list should have increasing clock values.")
						 % val % dtap->CCVals[dtap->CCVals.size()-1]).str().c_str());
			return dtap;
		}
	}*/
	
	dtap->CCVals.push_back(v);
	return dtap;
}

PUBLIC_API DTAParams* DTAParams_AddActSetByName(DTAParams *dtap, const char* actName) {
	vector<string> as;
	as.push_back(actName);
	dtap->ActSets.push_back(as);
	return dtap;
}

PUBLIC_API DTAParams* DTAParams_AddActSetByIdList(DTAParams *dtap, IDList* idl) {
	vector<string> as;
	list<string>::const_iterator it;
	for (it = idl->idList.begin(); it != idl->idList.end(); ++it)
		as.push_back(*it);
	dtap->ActSets.push_back(as);
	IDList_Delete(idl);
	return dtap;
}

PUBLIC_API DTAParams* DTAParams_AddCslTaExpr(DTAParams *dtap, CslTaExpr* expr) {
	std::shared_ptr<CslTaExpr> cslPtr(expr);
	dtap->SPsExpr.push_back(cslPtr);
	return dtap;
}

PUBLIC_API DTAParams* DTAParams_MergeParams(const char *dtaName, DTAParams *ccvs,
											DTAParams *acts, DTAParams* expr)
{
	ccvs->DtaName = dtaName;
	ccvs->SPsExpr.swap(expr->SPsExpr);
	ccvs->ActSets.swap(acts->ActSets);
	delete expr;
	delete acts;
	return ccvs;
}

