/*
 *  dta.cpp
 *  CslTA-Solver
 *
 *  Implementation of the Deterministic Timed Automata
 *
 */

#include "precompiled.pch"

using namespace boost;
using namespace std;

#include <float.h>

#include "common-defs.h"
#include "asmc.h"
#include "dta.h"




//-----------------------------------------------------------------------------

DTA::DTA()
{
	instantiated = false;
	detKind      = DDK_NOT_YET_VERIFIED;
	locationPMap = get(LocationPropertyKind(), graph);
	edgePMap     = get(EdgePropertyKind(), graph);
}


StatePropID DTA::AddStateProposition(const string& spName) 
{
	assert(num_vertices(graph) == 0);
	if (SPs.HasSymbol(spName))
		return -1;
	return SPs.AddNewSymbol(spName);
}


ActionID DTA::AddActionSymbol(const string& actName) 
{
	assert(num_vertices(graph) == 0);
	if (Acts.HasSymbol(actName))
		return -1;
	return Acts.AddNewSymbol(actName);
}


ClockValID DTA::AddClockConstrName(const string& cvName) 
{
	assert(num_vertices(graph) == 0);
	if (CCNs.HasSymbol(cvName))
		return -1;
	return CCNs.AddNewSymbol(cvName);
}


bool DTA::AddClockConstrOrder(const string& cc1, const string& cc2)
{
	assert(num_vertices(graph) == 0);
	assert(CCNs.HasSymbol(cc1));
	assert(CCNs.HasSymbol(cc2));
	if (!CCNs.HasSymbol(cc1)) {
		cout << "Couldn't find a clock constraint named "<<cc1<<"." << endl;
		return false;
	} else if (!CCNs.HasSymbol(cc2)) {
		cout << "Couldn't find a clock constraint named "<<cc2<<"." << endl;
		return false;
	}
	ClockValID v1 = CCNs.GetIDFromName(cc1);
	ClockValID v2 = CCNs.GetIDFromName(cc2);
	PartialOrd.push_back(make_pair(v1, v2));
	return true;
}


bool DTA::AddLocation(const string& locName, DtaLocFlags flags, 
					  std::shared_ptr<DtaLocLabel>& label)
{
	if (FindLocation(locName) != location_t(-1))
		return false;
	LocationP lp;
	lp.locName = locName;
	lp.flags = flags;
	lp.label = label;
		
	location_t location = add_vertex(graph);
	put(locationPMap, location, lp);
	return true;
}


bool DTA::AddEdge(const string& from, const string& to, ActionSetType actType,
				  const vector<string>& actions, GuardFunction gf,
				  const string& alpha, const string& beta, bool reset)
{
	location_t f = FindLocation(from);
	location_t t = FindLocation(to);
	if (f == location_t(-1) || t == location_t(-1))
		return false;

	EdgeP ep;
	ep.actSetType = actType;
	ep.funct = gf;
	ep.reset = reset;
	vector<string>::const_iterator it;
	for (it = actions.begin(); it != actions.end(); ++it) {
		if (!Acts.HasSymbol(*it))
			return false;
		ep.actions.push_back(Acts.GetIDFromName(*it));
	}
	assert (CCNs.HasSymbol(alpha));
	assert (CCNs.HasSymbol(beta));
	ep.alpha = CCNs.GetIDFromName(alpha);
	ep.beta = CCNs.GetIDFromName(beta);
	if (ep.alpha == ep.beta) {
		if (actType != AST_BOUNDARY) {
			cout << "Invalid Inner guard: X="<<alpha<<"." << endl;
			return false;
		}
	}
	else if (AreCCNsOrdered(ep.alpha, ep.beta) != 1) {
		cout << "Couldn't infer the clock constraint value order betweeen ";
		cout << alpha << " and " << beta << "." << endl;
		cout << " This happened because there's no partial order between these" << endl;
		cout << " two symbols. Add a partial order constraint ";
		cout << alpha << " < " << beta << "." << endl;
		return false;
	}
	
	edge_t e;
	bool inserted;
	tie(e, inserted) = add_edge(f, t, ep, graph);
	return (inserted);
}


// This is used to extract the subgraph with only boundary edges from a DTA
template <class EdgePMap> 
struct filter_out_inner_edges {
	filter_out_inner_edges() {}
	filter_out_inner_edges(const EdgePMap& _epm) : epm(_epm) {}
	template <class Edge> bool operator() (const Edge& e) const {
		return !get(epm, e).isInnerEdge();
	}
	EdgePMap epm;
};
		   
		   
string DTA::VerifyDeterminism(const DtaStatePropValues* optSPVals) const
{
	ostringstream report;
	detKind = DDK_ALWAYS_DET;
	
	// 1) Verify that Initial locations have mutually exclusive state props. expressions
	location_iter_t it1, it2, it_end;
	size_t initialLocationsCount = 0;
	for (tie(it1, it_end) = locations(graph); it1 != it_end; ++it1) {
		const LocationP& lp1 = get(locationPMap, *it1);
		if (lp1.flags & DLF_INITIAL) {
			for (it2 = it1+1; it2 != it_end; ++it2) {
				const LocationP& lp2 = get(locationPMap, *it2);
				if (lp2.flags & DLF_INITIAL) {
					// lp1 and lp2 are 2 initial location - verify mutually exclusiveness
					ostringstream reason;
					if (!AreLocationLabelsMutuallyExcl(lp1, lp2, optSPVals, reason)) {
						report << (format("Initial locations %1% and %2% have intersecting "
										  "state proposition formulas:")
								   % lp1.locName % lp2.locName).str() << endl;
						report << reason.str() << endl;
						if (optSPVals != NULL) {
							detKind = DDK_NEVER_DET;
							return "The DTA is not deterministic due to intersecting\n"
								   "state proposition formulas in the initial locations.";
						}
						detKind = DDK_SEMI_DET;
					}
				}
			}
			initialLocationsCount++;
		}
	}
	
	if (initialLocationsCount == 0) {
		detKind = DDK_NEVER_DET;
		return "A valid DTA should have at least one initial location.\n";
	}
	
	// 2) Verify determinism for Inner & Boundary edges
	for (tie(it1, it_end) = locations(graph); it1 != it_end; ++it1) {
		graph_traits<DTAGraph>::out_edge_iterator oeit1, oeit2, oeit_end;
		for (tie(oeit1, oeit_end) = out_edges(*it1, graph); 
			 oeit1 != oeit_end; ++oeit1)
		{
			edge_t edge1 = *oeit1;
			for (oeit2 = oeit1+1; oeit2 != oeit_end; ++oeit2) {
				edge_t edge2 = *oeit2;
				
				// (oeit1, oeit2) is a couple of edges starting in *it1 location
				// Verify that these edges have mutually exclusive conditions.
				
				const EdgeP& ep1 = get(edgePMap, edge1);
				const EdgeP& ep2 = get(edgePMap, edge2);
				const LocationP& lp0 = get(locationPMap, source(edge1, graph));
				const LocationP& lp1 = get(locationPMap, target(edge1, graph));
				const LocationP& lp2 = get(locationPMap, target(edge2, graph));
				
				// TODO - RIMUOVERE
				/*cout << "  * ep1.isInnerEdge() = "<<ep1.isInnerEdge()<<endl;
				cout << "  * ep2.isInnerEdge() = "<<ep2.isInnerEdge()<<endl;
				cout << "  * AreLocationLabelsMutuallyExcl(lp1, lp2) = ";
				cout << AreLocationLabelsMutuallyExcl(lp1, lp2) << endl;
				cout << "  * AreActionSetsMutuallyExcl(ep1, ep2) = ";
				cout << AreActionSetsMutuallyExcl(ep1, ep2) << endl;
				cout << "  * AreGuardsMutuallyExcl(ep1, ep2) = ";
				cout << AreGuardsMutuallyExcl(ep1, ep2) << endl;//*/
				
				// Test mutually exclusiveness only on edges of the same kind
				if (ep1.isInnerEdge() == ep2.isInnerEdge())
				{
					ostringstream reason;
					if (!AreActionSetsMutuallyExcl(ep1, ep2, reason) &&
						!AreGuardsMutuallyExcl(ep1, ep2, reason) &&
						!AreLocationLabelsMutuallyExcl(lp1, lp2, optSPVals, reason))
					{
						report << (format ("The two edges (%1% -> %2%) and "
										   "(%1% -> %3%) are not mutually exclusive:")
								   % lp0.locName % lp1.locName	% lp2.locName).str();
						report << endl << reason.str() << endl;
						detKind = DDK_SEMI_DET;
					}
				}
			}
		}
	}
		
	// 3) No Boundary-edge loops : there is a boundary loop if the transitive
	//  closure of the graph with only boundary edges has loop edges.
	typedef property_map<DTAGraph, EdgePropertyKind>::type EdgePropMap;
	typedef filter_out_inner_edges<EdgePropMap> InnerEdgeFilter;
	InnerEdgeFilter filter(edgePMap);
	filtered_graph<DTAGraph, InnerEdgeFilter> fg(graph, filter);
	// Get the transitive closure of the DTA (with only boundary edges)
	DTAGraph tc_graph;
	transitive_closure(fg, tc_graph);
	edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(tc_graph); eit != eit_end; ++eit) {
		if (source(*eit, tc_graph) == target(*eit, tc_graph)) {
			detKind = DDK_NEVER_DET;
			//print_graph(tc_graph, "012345");
			return "There is at least one boundary edge loop in the DTA.";
		}
	}
	
	return report.str();
}


DTA::location_t DTA::FindLocation(const string& locName) const 
{
	location_iter_t it, it_end;
	for (tie(it, it_end) = locations(graph); it != it_end; ++it) {
		if (locName == get(locationPMap, *it).locName)
			return *it;
	}
	return -1;
}


// Verify if 2 locations have mutually exclusive state propositions.
bool DTA::AreLocationLabelsMutuallyExcl(const LocationP& lp1, 
										const LocationP& lp2,
										const DtaStatePropValues* optSPVals,
										ostream& reason) const
{
	set<StatePropID> spUsed;
	lp1.label->GatherSPsUsed(spUsed);
	lp2.label->GatherSPsUsed(spUsed);
	size_t numSP = spUsed.size();

	if (optSPVals != NULL) {
		assert(instantiated);
		// We have all the State Propositions values, for every ASMC state.
		vector<bool> spVals(SPs.NumSymbols(), false);
		for (size_t i=0; i<num_vertices(optSPVals->pAsmc->graph); i++) {
			// fill-in the sp vector
			for (size_t sp=0; sp<SPs.NumSymbols(); sp++)
				spVals[sp] = (*optSPVals->SPValues[sp].second)[i];

			// Test mutual exclusion in the i-th vertex of the ASMC
			if (lp1.label->Evaluate(spVals) && lp2.label->Evaluate(spVals)) {
				// Tell the user that SP ar not mutually exclusive
				reason << "  State propositions of "<<lp1.locName<<" and "<<lp2.locName;
				reason << " are not mutually exclusive in state ";
				reason << get(optSPVals->pAsmc->vertPMap, i).vertexName;
				reason << "." << endl;
				reason << "    SpExpr("<<lp1.locName<<") = "; 
				lp1.label->PrintExpr(reason, SPs); reason << endl;
				reason << "    SpExpr("<<lp2.locName<<") = "; 
				lp2.label->PrintExpr(reason, SPs); reason << endl;
				reason << "  State propositions have these values:  ";
				
				size_t j;
				set<StatePropID>::const_iterator it;
				for (j=0, it = spUsed.begin(); it != spUsed.end(); it++, j++)
					reason << "     " << SPs.GetSymbolName(*it) << ": " 
						   << optSPVals->SPValues[j].first
						   << " = " << spVals[*it] << "\n";
				reason << endl << endl;
				
				return false;  // not mutually exclusive
			}
			
		}		
	}
	else {
		// We want to prove mut. exclusion of guard without having the values
		// that the State Propositions really will have: we assume that every
		// combination of truth value is possible.
		
		// Proving boolean expression mut. excl. is a difficult problem, because
		// it's NP-complete and potentially very very slow to compute. 
		// However, DTAs are built by humans, and have few state propositions, so 
		// we can use a brute force approach.

		if (numSP > 30) {
			reason << "  Error: too many SP associated to a DTA (limit = 30)." << endl;
			return false;
		}
		else if (numSP > 8) {
			reason << "  Warning: determinism verification can be very very slow!" << endl;
		}

		// Boolean values for state propositions
		vector<bool> spVals(SPs.NumSymbols(), false);
		for (size_t i = 0; i < size_t(1 << numSP); i++) {
			
			// Prepare the i-th combination of boolean values, using the numSP
			// bits of variable i as truth values for the state propositions.
			size_t j;
			set<StatePropID>::const_iterator it;
			for (j=0, it = spUsed.begin(); it != spUsed.end(); it++, j++) {
				spVals[*it] = (i & (1 << j)) != 0;
			}
			
			// Evaluate: expressions cannot be both true 
			if (lp1.label->Evaluate(spVals) && lp2.label->Evaluate(spVals)) {
				// Tell the user that SP ar not mutually exclusive
				reason << "  State propositions of "<<lp1.locName<<" and "<<lp2.locName;
				reason << " are not mutually exclusive."<< endl;
				reason << "    SpExpr("<<lp1.locName<<") = "; 
				lp1.label->PrintExpr(reason, SPs); reason << endl;
				reason << "    SpExpr("<<lp2.locName<<") = "; 
				lp2.label->PrintExpr(reason, SPs); reason << endl;
				reason << "  Counter-Example found:  ";
				
				for (j=0, it = spUsed.begin(); it != spUsed.end(); it++, j++)
					reason << SPs.GetSymbolName(*it) << " = " << spVals[*it] << "   ";
				reason << endl << endl;
				
				return false;  // not mutually exclusive
			}
		}
	}

	return true;
}


bool DTA::AreActionSetsMutuallyExcl(const EdgeP& ep1, const EdgeP& ep2,
									ostream& reason) const
{
	if (ep1.actSetType == AST_BOUNDARY || ep2.actSetType == AST_BOUNDARY)
		return false;
	
	if (ep1.actSetType == AST_EXCLUDED_ACTs) {
		// If both edges have actionSet of type AST_EXCLUDED_ACTs, disjunction 
		// cannot be determined without knowing what the Act set is.
		if (ep2.actSetType == AST_EXCLUDED_ACTs) {
			reason << (format("  Cannot verify determinism if both ActExprs "
							  "have form: Act - {...}\n    %1%  <>  %2%")
					   % ep1.GetActSetDescription(Acts)
					   % ep2.GetActSetDescription(Acts))<< endl;
			return false;
		}
		
		return AreActionSetsMutuallyExcl(ep2, ep1, reason);
	}
	else {
		if (ep2.actSetType == AST_EXCLUDED_ACTs) {
			// actions in ep1 must appear in ep2 excluded actionSet
			vector<ActionID>::const_iterator it;
			for (it = ep1.actions.begin(); it != ep1.actions.end(); ++it)
				if (ep2.actions.end() == find(ep2.actions.begin(), 
											  ep2.actions.end(), *it))
				{
					reason << (format("  ActExpr: action %1% of %2% should be in %3% "
									  "to be fully deterministic.")
							   % Acts.GetSymbolName(*it)
							   % ep1.GetActSetDescription(Acts)
							   % ep2.GetActSetDescription(Acts)) << endl;
					return false;
				}
		}
		else {
			// both actionSets should have no actions in common
			vector<ActionID>::const_iterator it;
			for (it = ep1.actions.begin(); it != ep1.actions.end(); ++it)
				if (ep2.actions.end() != find(ep2.actions.begin(), 
											  ep2.actions.end(), *it))
				{
					reason << (format("  ActExpr: action %1% of %2% shouldn't be in %3% "
									  "to be fully deterministic.")
							   % Acts.GetSymbolName(*it)
							   % ep1.GetActSetDescription(Acts)
							   % ep2.GetActSetDescription(Acts)) << endl;
					return false;
				}
		}
	}
	return true;
}


bool DTA::AreGuardsMutuallyExcl(const EdgeP& ep1, const EdgeP& ep2, 
								ostream& reason) const
{
	if (ep1.isBoundEdge()) { // Boundary edges
		assert(ep2.isBoundEdge());
		assert(ep1.funct == GF_EQUAL && ep2.funct == GF_EQUAL);
		assert(ep1.alpha == ep1.beta && ep2.alpha == ep2.beta);
		if (ep1.alpha == ep2.alpha) {
			reason << (format("  Boundary Guards intersection on: X=%1%")
					   % CCNs.GetSymbolName(ep1.alpha)) << endl;
			return false;
		}
		return true;
	}
	// Inner edges
	bool b = !(IsCCNsContained(ep1.alpha, ep2.alpha, ep1.beta) ||
			   IsCCNsContained(ep1.alpha, ep2.beta,  ep1.beta) ||
			   IsCCNsContained(ep2.alpha, ep1.alpha, ep2.beta) ||
			   IsCCNsContained(ep2.alpha, ep1.beta,  ep2.beta) ||
			   (ep1.alpha == ep2.alpha && ep1.beta == ep2.beta));
	
	if (b == false) {
		reason << (format("  Inner Guards: %1% < X < %2%  AND  %3% < X < %4% :\n"
						  "        (%1% < %3% < %2%) = %5%\n"
						  "        (%1% < %4% < %2%) = %6%\n"
						  "        (%3% < %1% < %4%) = %7%\n"
						  "        (%3% < %2% < %4%) = %8%\n"
						  "        (%1% == %3%)    = %9%\n"
						  "        (%2% == %4%)    = %10%\n"
						  "        Are mutually exclusive? %11%")
				   % CCNs.GetSymbolName(ep1.alpha) % CCNs.GetSymbolName(ep1.beta)
				   % CCNs.GetSymbolName(ep2.alpha) % CCNs.GetSymbolName(ep2.beta)
				   % IsCCNsContained(ep1.alpha, ep2.alpha, ep1.beta)
				   % IsCCNsContained(ep1.alpha, ep2.beta,  ep1.beta)
				   % IsCCNsContained(ep2.alpha, ep1.alpha, ep2.beta)
				   % IsCCNsContained(ep2.alpha, ep1.beta,  ep2.beta)
				   % (ep1.alpha == ep2.alpha) 
				   % (ep1.beta == ep2.beta)
				   % (b ? "true" : "false")) << endl;
		return false;
	}
	return true;
}


int DTA::AreCCNsOrdered(ClockValID v1, ClockValID v2) const
{
	assert(CCNs.IsValidID(v1) && CCNs.IsValidID(v2));
	// This method may return the following codes:
	//   1  =>  (v1 < v2)
	//   0  =>  (v1 >= v2) OR don't know (cannot infer partial order)
	
	if (v1 == v2)  return 0;
	if (v1==0 && v2==0) return 0;  // '0' == '0'
	if (v1==0 && v2>0)  return 1;  // '0' < alpha
	if (v1>0  && v2==0) return 0;  // alpha > '0'
	const ClockValID INFCV = CCNs.NumSymbols()-1;
	assert(v1!=INFCV || v2!=INFCV);
	if (v1==INFCV && v2<INFCV)  return 0;  // inf > alpha
	if (v1<INFCV  && v2==INFCV) return 1;  // alpha < inf
	
	if (instantiated) {
		return (CCVals[v1] < CCVals[v2]) ? 1 : 0;
	}
	
	// Find out if the pair (v1, v2) is in the partial order set
	CCPair_t vPair(v1, v2), vPairInv(v2, v1);
	for (size_t i=0; i<PartialOrd.size(); i++) {
		if (PartialOrd[i] == vPair)		return 1;
		if (PartialOrd[i] == vPairInv)	return 0;
	}
	
	// Try to infer partial order between CC names v1 & v2
	list<CCPair_t> graySet;
	set<CCPair_t> visited;
	for (size_t i=0; i<PartialOrd.size(); i++)
		if (PartialOrd[i].first == v1)
			graySet.push_back(PartialOrd[i]);
	while (!graySet.empty()) {
		CCPair_t p = graySet.front();
		graySet.pop_front();		

		if (p.first == v1 && p.second == v2)
			return 1;

		if (visited.count(p))   continue;
		else visited.insert(p);		
		
		for (size_t i=0; i<PartialOrd.size(); i++)
			if (PartialOrd[i].first == p.second)  // A<B, B<C  =>  A<C
				graySet.push_back(make_pair(p.first, PartialOrd[i].second));
	}	
	return 0; // couldn't infer anything
}


bool DTA::IsCCNsContained(ClockValID A, ClockValID V, ClockValID B) const
{
	// There are 2 different conditions under which v must be considered 
	// between A and B: 
	//   (1) there exists two partial order rules: A<V and V<B
	//   (2) there's no known order between the CCNs, so we cannot say for sure
	//       if A<V<B can be true. In this second case, the response is a
	//       "probably inside A and B".
	
	//bool AV = (AreCCNsOrdered(A, V)==1);
	//bool VB = (AreCCNsOrdered(V, B)==1);
	bool VA = (AreCCNsOrdered(V, A)==1);
	bool BV = (AreCCNsOrdered(B, V)==1);
	
	/*cout << (format("   Testing: %1% < %2% < %3%   [%1%<%2%=%4%, %2%<%3%=%5%, "
					"%2%<%1%=%6%, %3%<%2%=%7%]")
			 % CCNs.GetSymbolName(A)
			 % CCNs.GetSymbolName(V)
			 % CCNs.GetSymbolName(B)
			 % AV % VB % VA % BV) << endl;//*/
	
	return !(VA || BV) && (A!=V) && (V!=B);
}


bool DTA::IsPartialOrderValid(string& reason) const
{
	// Partial Order vector is valid if it's non-contradictory, i.e. there's no
	// self loop in the ordered CCNames (like:  A<B, B<C, C<A),

	list<CCPair_t> graySet(PartialOrd.begin(), PartialOrd.end());
	set<CCPair_t> visited;
	
	while (!graySet.empty()) {
		CCPair_t p = graySet.front();
		graySet.pop_front();
		
		if (visited.count(make_pair(p.second, p.first))) {
			reason = (format("Partial order information of Clock Constraint values\n"
							 "contains at least a contradictory definition:\n"
							 "   %1% < %2%  AND  %2% < %1%\n"
							 "Check your DTA definition.")
					  % CCNs.GetSymbolName(p.first)
					  % CCNs.GetSymbolName(p.second)).str();
			return false; // There's a loop!
		}
		
		if (visited.count(p))  continue;
		else visited.insert(p);
		
		for (size_t i=0; i<PartialOrd.size(); i++)
			if (PartialOrd[i].first == p.second)  // A<B, B<C  =>  A<C
				graySet.push_back(make_pair(p.first, PartialOrd[i].second));
	}
	
	return true;
}

bool DTA::isValidObject(string& reason) const
{
	reason = VerifyDeterminism(NULL);
	if (detKind==DDK_SEMI_DET) {
		cout << endl << reason << endl;
		cout << ("The supplied DTA will be considered semi-deterministic.\n"
				 "A semi-deterministic DTA can be used inside a CSLTA expression, but\n"
				 "it's deterministic behaviour depends on the expression itself.\n"
				 "If the DTA behaves non-deterministically, the CSLTA expression will\n"
				 "be rejected.\n") << endl;
	}
	else if (detKind==DDK_NEVER_DET) {
		cout << endl << reason << endl;
		cout << ("Cannot accept this DTA.\n") << endl;
	}
	return (detKind==DDK_ALWAYS_DET || detKind==DDK_SEMI_DET);
}


bool DTA::WriteAsGmlFile(ostream& os, const char *name, const char *args)
{
	size_t numLocs = num_vertices(graph);
	os << "Creator \"CSL-TA Solver\"" << endl;
	os << "graph [" << endl;
	os << "hierarchic 1 \nlabel \""<<name<<"\"\ndirected 1" << endl;
	
	// Write the locations
	location_iter_t it, it_end;
	for (tie(it, it_end) = locations(graph); it != it_end; ++it) {
		const LocationP& lp = get(locationPMap, *it);
		location_t lnum = *it;
		string label = GetGmlNameForLocation(lp);
		os << "node [ \n id "<<lnum<<"\n label "<<label<<endl;
		os << " graphics [\n  type \"ellipse\"\n  fill \"#FFFFFF\"";
		os << "\n  outline \"#000000\"\n";
		os << (lp.flags & DLF_FINAL ? "  outlineWidth 3\n" :"");
		os << "  w 70\n  h 70\n  x "<<(lnum%5)*90<<"\n  y "<<(lnum/5)*70<<"\n ]" << endl;
		os << " LabelGraphics [\n  text "<<label<<"\n  fontsize 12\n";
		os << "  fontname \"Dialog\"\n  anchor \"c\"\n ]" << endl;
		os << "]" << endl;
		
		// Entering arc for initial locations
		if (lp.flags & DLF_INITIAL) {
			os << (format("node [ id %1% label \"\" graphics [ type \"diamond\" "
						  "fill \"#FFFFFF\" outline \"#FFFFFF\" ] LabelGraphics ["
						  "] ]\n edge [ source %1% target %2% graphics [ "
						  "fill \"#000000\" targetArrow \"standard\" ] ]\n")
				   % (lnum + numLocs) % lnum) << endl;			
		}
	}
	
	// Write the edges
	edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(graph); eit != eit_end; ++eit) {
		const EdgeP& ep = get(edgePMap, *eit);
		string label = GetGmlNameForEdge(ep);
		os << (format("edge [\n source %1%\n target %2%\n label %3%\n"
					  " graphics [\n  fill \"#000000\"\n"
					  "  targetArrow \"standard\"\n  smoothBends 1\n ]\n"
					  " LabelGraphics [\n  text %3%\n  fontSize 10\n"
					  "  fontName \"Dialog\"\n"
					  " ]\n]")
			   % source(*eit, graph) % target(*eit, graph) % label) << endl;
	}
	
	os << "]" << endl;
	return true;
}


string DTA::GetGmlNameForLocation(const LocationP& lp) const
{
	ostringstream s;
	s << "\"" << lp.locName << "\n";
	lp.label->PrintExpr(s, SPs);
	s << "\"";
	return htmlize_utf8_str(s.str());
}


string DTA::GetGmlNameForEdge(const EdgeP& ep) const 
{
	ostringstream s;
	s << "\"";
	
	const string& alpha = CCNs.GetSymbolName(ep.alpha);
	const string& beta = CCNs.GetSymbolName(ep.beta);
	switch (ep.funct) {
		case GF_EQUAL:   s << "x = " << alpha; break;
		case GF_LESS:    s << "x ≤ " << beta; break;
		case GF_GREATER: s << "x ≥ " << alpha; break;
		case GF_BETWEEN: s << alpha << " ≤ x ≤ " << beta; break;
	}
	
	s << ";\n" << ep.GetActSetDescription(Acts);	
	s << "; " << (ep.reset ? "{x}" : "∅") << "\"";
	return htmlize_utf8_str(s.str());
}



bool DTA::Instantiate(const ASMC& asmc, const vector<double>& InstCCVals, 
					  const vector< vector<string> >& InstActSets,
					  DTA& instDta, string& failReason) const
{
	assert(num_vertices(instDta.graph) == 0);
	
	instDta.graph = graph;
	instDta.detKind = detKind;
	instDta.SPs = SPs;
	
	// --------------------------------
	// Instantiate Action Sets and remap DTA ActionIDs with the same numbers
	// of the ASMC ActionIDs
	// --------------------------------
	if (InstActSets.size() != Acts.NumSymbols()) {
		failReason = ("Error: the number of action sets does not match the \n"
					  "       number of action parameters in the DTA.");
		return false;
	}
	vector< vector<ActionID> > remapActIds(Acts.NumSymbols());
	for (size_t nSet=0; nSet<InstActSets.size(); nSet++) {
		const vector<string>& actSet = InstActSets[nSet];
		
		if (actSet.size()==0) {
			failReason = "Error: empty action set.";
			return false;
		}
		
		for (size_t i=0; i<actSet.size(); i++) {
			if (!asmc.Actions.HasSymbol(actSet[i])) {
				failReason = (format("Error: no action named \"%1%\" in the ASMC.")
							  % actSet[i]).str();
				return false;
			}
			// Verify that all the other action sets have different action names
			for (size_t jSet=nSet+1; jSet<InstActSets.size(); jSet++) {
				for (size_t j=0; j<InstActSets[jSet].size(); j++) {
					if (actSet[i] == InstActSets[jSet][j]) {
						failReason = (format("Error: action named \"%1%\" is shared "
											 "by multiple action sets.\n"
											 "       Action sets must be disjointed.")
									  % actSet[i]).str();
						return false;
					}
				}
			}
			
			remapActIds[nSet].push_back(asmc.Actions.GetIDFromName(actSet[i]));
		}
	}
	// Remap DTA Action IDs
	edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(instDta.graph); eit != eit_end; ++eit) {
		EdgeP& ep = get(instDta.edgePMap, *eit);
		vector<ActionID> nActIds;
		for (size_t i=0; i<ep.actions.size(); i++) {
			const vector<ActionID>& remapActSet = remapActIds[ep.actions[i]];
			for (size_t j=0; j<remapActSet.size(); j++)
				nActIds.push_back(remapActSet[j]);
		}
		ep.actions = nActIds;
	}
	instDta.Acts = asmc.Actions;
	
	
	// --------------------------------
	// Instantiate Clock Values
	// --------------------------------
	instDta.CCNs       = CCNs;
	instDta.CCVals     = InstCCVals;
	instDta.PartialOrd = PartialOrd;
	
	// Add the numeric constant of CCNs inside CCVals
	for (size_t i = 0; i < CCNs.NumSymbols(); i++) {
		const string& sym = CCNs.GetSymbolName(i);
		if (sym.size() > 0 && isdigit(sym[0])) {
			// The i-th symbol is a number - translate it directly
			instDta.CCVals.insert(instDta.CCVals.begin() + i,
								  atof(sym.c_str()));
		}
	}
	/*for (size_t i=0; i<instDta.CCNs.NumSymbols(); i++)
		cout << "CCN["<<i<<"] = "<<instDta.CCNs.GetSymbolName(i)<<endl;
	for (size_t i=0; i<instDta.CCVals.size(); i++)
		cout << "CCVals["<<i<<"] = "<<instDta.CCVals[i]<<endl;
	for (size_t i=0; i<instDta.PartialOrd.size(); i++)
		cout << "PartialOrd["<<i<<"] = ("<<instDta.PartialOrd[i].first<<", "
		<< instDta.PartialOrd[i].second<<")"<<endl;*/
	
	// Test correctness of the clock constraints
	if (instDta.CCVals.size() + 1 != CCNs.NumSymbols()) {
		failReason = ("Error: the number of clock values does not match the number\n"
					  "       of clock parameters in the DTA.");
		return false;
	}	

	// Remap the indices, to be sure that CCNs are in increasing value order
	vector< pair<double, size_t> > Sorted(instDta.CCVals.size());
	for (size_t i=0; i<instDta.CCVals.size(); i++)
		Sorted[i] = make_pair(instDta.CCVals[i], i);
	Sorted.push_back(make_pair(DBL_MAX, instDta.CCVals.size()));
	std::sort(Sorted.begin(), Sorted.end());
	vector<size_t> RemapTable(Sorted.size());
	for (size_t i=0; i<Sorted.size(); i++) {
		RemapTable[i] = Sorted[i].second;
		if (i>0 && Sorted[i].first <= Sorted[i-1].first) {
			failReason = (format("Error: clock values must be different and "
								 "in increasing order.\n(%1% is not less than %2%).")
						  % Sorted[i].first % Sorted[i-1].first).str();
			return false;			
		}
	}	
	for (size_t i=0; i<instDta.CCVals.size(); i++)
		instDta.CCVals[i] = Sorted[i].first;
	
	// Remap all the ClockValueIDs
	instDta.CCNs.RemapSymbolIDs(RemapTable);
	for (tie(eit, eit_end) = edges(instDta.graph); eit != eit_end; ++eit) {
		EdgeP& ep = get(instDta.edgePMap, *eit);
		ep.alpha = RemapTable[ep.alpha];
		ep.beta = RemapTable[ep.beta];
	}

	// Remap and test Partial order constraints, too
	for (size_t i=0; i<instDta.PartialOrd.size(); i++) {
		instDta.PartialOrd[i].first  = (instDta.CCNs.GetIDFromName
										(CCNs.GetSymbolName(instDta.PartialOrd[i].first)));
		instDta.PartialOrd[i].second = (instDta.CCNs.GetIDFromName
										(CCNs.GetSymbolName(instDta.PartialOrd[i].second)));
		if (instDta.PartialOrd[i].first >= instDta.PartialOrd[i].second) {
			failReason = (format("Error: partial order constraint %1% < %2% violated: "
								 "%1% = %3%, %2% = %4%.")
						  % instDta.CCNs.GetSymbolName(instDta.PartialOrd[i].first)
						  % instDta.CCNs.GetSymbolName(instDta.PartialOrd[i].second)
						  % instDta.CCVals[ instDta.PartialOrd[i].first ]
						  % instDta.CCVals[ instDta.PartialOrd[i].second ]).str();
			return false;
		}
	}
		

	instDta.instantiated = true;
	return true;
}







string DTA::EdgeP::GetActSetDescription(const SymbolTable& Acts) const
{
	ostringstream s;
	switch (actSetType) {
		case AST_BOUNDARY: s << "#"; break;
		case AST_EXCLUDED_ACTs:
			s << "Act";
			if (actions.size() == 0)
				break;
			s << " - ";
			case AST_INCLUDED_ACTs:
			if (actions.size() == 0) {
				s << "∅";  break;
			}
			for (size_t i=0; i<actions.size(); i++) {
				s << (i==0 ? "{" : ", ") << Acts.GetSymbolName(actions[i]);
			}
			s << "}";
	}
	return s.str();
}




//-----------------------------------------------------------------------------

bool DtaLocLabel_True::Evaluate(const vector<bool>& SPVals) const {
	return true;
}
bool DtaLocLabel_SP::Evaluate(const vector<bool>& SPVals) const {
	return SPVals[spID];
}
bool DtaLocLabel_Not::Evaluate(const vector<bool>& SPVals) const {
	return !expr->Evaluate(SPVals);
}
bool DtaLocLabel_BinOp::Evaluate(const vector<bool>& SPVals) const {
	bool b1 = expr1->Evaluate(SPVals);
	bool b2 = expr2->Evaluate(SPVals);
	switch (bop) {
		case BOP_AND:   return b1 && b2;
		case BOP_OR:	return b1 || b2;
		case BOP_IMPLY: return !b1 || b2;
	}
	assert(false);
	return false;
}


void DtaLocLabel_True::PrintExpr(ostream& os, const SymbolTable& SPs) const {
	os << "true";
}
void DtaLocLabel_SP::PrintExpr(ostream& os, const SymbolTable& SPs) const {
	os << SPs.GetSymbolName(spID);
}
void DtaLocLabel_Not::PrintExpr(ostream& os, const SymbolTable& SPs) const {
	os << UnaryLogicOp_Not;
	expr->PrintExpr(os, SPs);
}
void DtaLocLabel_BinOp::PrintExpr(ostream& os, const SymbolTable& SPs) const {
	os << "("; 
	expr1->PrintExpr(os, SPs);
	os << " " << BinaryLogicOp_Str[bop] << " ";
	expr2->PrintExpr(os, SPs);
	os << ")";
}


void DtaLocLabel_True::GatherSPsUsed(set<StatePropID>& spUsed) const { }
void DtaLocLabel_SP::GatherSPsUsed(set<StatePropID>& spUsed) const {
	spUsed.insert(spID);
}
void DtaLocLabel_Not::GatherSPsUsed(set<StatePropID>& spUsed) const { 
	expr->GatherSPsUsed(spUsed);
}
void DtaLocLabel_BinOp::GatherSPsUsed(set<StatePropID>& spUsed) const { 
	expr1->GatherSPsUsed(spUsed);
	expr2->GatherSPsUsed(spUsed);
}








//-----------------------------------------------------------------------------

PUBLIC_API void DtaLoc_Delete(DtaLoc *dl) {
	delete dl;
}
PUBLIC_API DtaLoc* DtaLoc_New(DtaLocFlags fl, const char *locName, DtaLocLabel *ll) {
	return new DtaLoc(fl, locName, ll);
}

PUBLIC_API void DtaGuard_Delete(DtaGuard *dcc) {
	delete dcc;
}
PUBLIC_API DtaGuard* DtaGuard_NewA(GuardFunction f, const char *a) {
	switch (f) {
		case GF_LESS:    return new DtaGuard(f, "0", a);
		case GF_GREATER: return new DtaGuard(f, a, "inf");
		case GF_EQUAL:   return new DtaGuard(a);
		default: assert(0); return NULL;
	}
}
PUBLIC_API DtaGuard* DtaGuard_NewAB(GuardFunction f, const char *a, const char *b) {
	return new DtaGuard(f, a, b);
}

PUBLIC_API void DtaActSet_Delete(DtaActSet *das) {
	delete das;
}
PUBLIC_API DtaActSet* DtaActSet_NewEmpty() {
	return new DtaActSet;
}
PUBLIC_API DtaActSet* DtaActSet_NewID(const char *actID) {
	return DtaActSet_AddAction(DtaActSet_NewEmpty(), actID);
}
PUBLIC_API DtaActSet* DtaActSet_AddAction(DtaActSet *das, const char *actID) {	
	das->actions.push_back(string(actID));
	return das;
}
PUBLIC_API DtaActSet* DtaActSet_SetType(DtaActSet *das, ActionSetType ty) {
	das->type = ty;
	return das;
}

PUBLIC_API void DtaCCNamOrd_Delete(DtaCCNamOrd *dtaccno) {
	delete dtaccno;
}
PUBLIC_API DtaCCNamOrd *DtaCCNamOrd_New() {
	return new DtaCCNamOrd;
}
PUBLIC_API DtaCCNamOrd* DtaCCNamOrd_AddSymbols(DtaCCNamOrd *dtaccno, IDList* idl, 
											   BOOL sym_are_ordered) 
{
	list<string>::const_iterator it;
	for (it = idl->idList.begin(); it != idl->idList.end(); ++it)
		dtaccno->CCNames.push_back(*it);
	
	if (sym_are_ordered && idl->idList.size()>1)
		DtaCCNamOrd_AddPartialOrder(dtaccno, idl);
	return dtaccno;
}
PUBLIC_API DtaCCNamOrd* DtaCCNamOrd_AddPartialOrder(DtaCCNamOrd *dtaccno, IDList* idl) {
	list<string>::const_iterator it1, it2;
	it1 = idl->idList.begin();
	size_t count = 0;
	while (it1 != idl->idList.end()) {
		if (count>0) 
			dtaccno->PartialOrds.push_back(make_pair(*it2, *it1));
		it2 = it1;
		++it1;
		++count;
	}
	if (count < 2) {
		cout << "Empty partial Set of clock constraints order given." << endl;
	}
	delete idl;
	return dtaccno;
}


PUBLIC_API void DtaEdge_Delete(DtaEdge *de) {
	delete de;
}
PUBLIC_API DtaEdge* DtaEdge_New(const char *from, const char *to,
								DtaGuard *dcc, DtaActSet *das, BOOL reset) 
{
	if (das == NULL)
		das = DtaActSet_SetType(DtaActSet_NewEmpty(), AST_BOUNDARY);
	return new DtaEdge(from, to, dcc, das, reset);
}


PUBLIC_API void DtaLocLabel_Delete(DtaLocLabel *dll) {
	delete dll;
}
PUBLIC_API DtaLocLabel* DtaLocLabel_NewTrue() {
	return new DtaLocLabel_True();
}
PUBLIC_API DtaLocLabel* DtaLocLabel_NewSpExpr(const char *sp) {
	// Find the StatePropID
	StatePropID spID = 0;
	const list<string>& sps = g_status.dtaSPExprs->idList;
	list<string>::const_iterator it;
	for (it = sps.begin(); it != sps.end(); ++it, spID++)
		if (*it == sp)
			return new DtaLocLabel_SP(spID);
	
	GS_SetError((format("couldn't find the state proposition %1%") 
				 % sp).str().c_str());	
	return new DtaLocLabel_True();
}
PUBLIC_API DtaLocLabel* DtaLocLabel_NewNotExpr(DtaLocLabel *dll) {
	return new DtaLocLabel_Not(dll);
}
PUBLIC_API DtaLocLabel* DtaLocLabel_NewAndExpr(DtaLocLabel *d1, DtaLocLabel *d2) {
	return new DtaLocLabel_BinOp(d1, d2, BOP_AND);
}
PUBLIC_API DtaLocLabel* DtaLocLabel_NewOrExpr(DtaLocLabel *d1, DtaLocLabel *d2) {
	return new DtaLocLabel_BinOp(d1, d2, BOP_OR);
}
PUBLIC_API DtaLocLabel* DtaLocLabel_NewImplyExpr(DtaLocLabel *d1, DtaLocLabel *d2) {
	return new DtaLocLabel_BinOp(d1, d2, BOP_IMPLY);
}






PUBLIC_API void	Dta_Delete(DTA *dta) {
	delete dta;
}
PUBLIC_API DTA* Dta_New() {
	DTA * dta = new DTA();
	list<string>::const_iterator it;
	assert(g_status.dtaCCNamOrds != NULL && 
		   g_status.dtaActs != NULL &&
		   g_status.dtaSPExprs != NULL);
	
	dta->AddClockConstrName("0");
	const list<string>& ccns = g_status.dtaCCNamOrds->CCNames;
	for (it = ccns.begin(); it != ccns.end(); ++it)
		if (dta->AddClockConstrName(*it) == ClockValID(-1))
			GS_SetError("couldn't add an action symbol");
	if (dta->AddClockConstrName("inf") == ClockValID(-1))
		GS_SetError("couldn't add an action symbol");
	for (size_t i=0; i<g_status.dtaCCNamOrds->PartialOrds.size(); i++) {
		if (!dta->AddClockConstrOrder(g_status.dtaCCNamOrds->PartialOrds[i].first,
									  g_status.dtaCCNamOrds->PartialOrds[i].second))
			GS_SetError("invalid partial order sequence");
	}
	string reason;
	if (!dta->IsPartialOrderValid(reason)) {
		GS_SetError(reason);
	}
	
	const list<string>& acts = g_status.dtaActs->idList;
	for (it = acts.begin(); it != acts.end(); ++it)
		if (dta->AddActionSymbol(*it) == ActionID(-1))
			GS_SetError("couldn't add an action symbol");
	
	const list<string>& sps = g_status.dtaSPExprs->idList;
	for (it = sps.begin(); it != sps.end(); ++it)
		if (dta->AddStateProposition(*it) == StatePropID(-1))
			GS_SetError("couldn't add an action symbol");

	return dta;
}
PUBLIC_API DTA* Dta_AddLocation(DTA *dta, DtaLoc *loc) {
	if (!dta->AddLocation(loc->locName, loc->flags, loc->label))
		GS_SetError("couldn't add the location");
	delete loc;
	return dta;
}
PUBLIC_API DTA* Dta_AddEdge(DTA *dta, DtaEdge *edge) {
	assert(edge->das != NULL);
	if (!dta->AddEdge(edge->from, edge->to, 
					  (edge->das != NULL ? edge->das->type : AST_BOUNDARY),
					  (edge->das != NULL ? edge->das->actions : vector<string>()),
					  edge->dcc->funct, edge->dcc->alpha, edge->dcc->beta,
					  edge->reset))
		GS_SetError("couldn't add the edge");
	delete edge;
	return dta;
}





