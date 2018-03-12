/*
 *  asmc.cpp
 *  CslTA-Solver
 *
 *  Implementation of the Action and State -labeled Markovian Chain
 *
 */

#include "precompiled.pch"

using namespace boost;
using namespace std;

#include "common-defs.h"
#include "asmc.h"



//-----------------------------------------------------------------------------

ASMC::ASMC()
{ 
	vertPMap = get(VertexPropertyKind(), graph);
	edgePMap = get(EdgePropertyKind(), graph);
}

AtomicPropID ASMC::AddAtomicProposition(const string& name)
{
	if (AtomicProps.HasSymbol(name))
		return AtomicProps.GetIDFromName(name);
	else {
		// Add the AP to the symbol map, then resize every vector<> in the 
		// vertex list.
		AtomicPropID apID = AtomicProps.AddNewSymbol(name);
		vertex_iter_t it, it_end;
		for (tie(it, it_end) = vertices(graph); it != it_end; it++) {
			VertexP& vp = get(vertPMap, *it);
			vp.labelVals.push_back(0);
			assert(vp.labelVals.size() == apID+1);
			put(vertPMap, *it, vp);
		}
		const size_t TOO_MANY_AP_WARN_LIMIT = 200;
		if (AtomicProps.NumSymbols() == TOO_MANY_AP_WARN_LIMIT) {
			cout << "WARNING: The ASMC has a huge number of Atomic Propositions.\n";
			cout << "         This could impact the tool performance." << endl;
		}

		return apID;
	}
}

bool ASMC::AddState(const string& stName, const list<IdValuePair>& APs)
{	
	if (FindVertex(stName) != vertices(graph).second)
		return false;
	
	/*if ((num_vertices(graph) % 100) == 0)
		cout << "NUM VERTICES = " << num_vertices(graph) << endl;*/
	
	if (graph.m_vertices.size() == graph.m_vertices.capacity())
		graph.m_vertices.reserve(graph.m_vertices.size() * 4);
	VertexP vp;
	vp.vertexName = stName;
	vp.labelVals.resize(AtomicProps.NumSymbols(), false);
	list<IdValuePair>::const_iterator ap_it;
	for (ap_it = APs.begin(); ap_it != APs.end(); ++ap_it) {
		AtomicPropID apID = AddAtomicProposition(ap_it->ID);
		if (apID >= vp.labelVals.size()) {
			vp.labelVals.resize(apID+1);
		}
		vp.labelVals[apID] = ap_it->val;
	}
	
	vertex_t vert = add_vertex(graph);
	put(vertPMap, vert, vp);
	vertexName2Index[stName] = vert;
	return true;
}

bool ASMC::AddEdge(const string& s0, const string& s1, 
				   double lambda, const string& action)
{	
	vertex_iter_t vs0 = FindVertex(s0);
	vertex_iter_t vs1 = FindVertex(s1);
	if (vs0 == vertices(graph).second || vs1 == vertices(graph).second)
		return false;

	/*if ((num_edges(graph) % 100) == 0)
		cout << "NUM EDGES = " << num_edges(graph) << endl;*/

	edge_t e;
	bool inserted;
	tie(e, inserted) = add_edge(*vs0, *vs1, graph);
	if (!inserted)
		return false;

	EdgeP ep;
	ep.lambda = lambda;
	if (!Actions.HasSymbol(action))
		Actions.AddNewSymbol(action);
	ep.actionID = Actions.GetIDFromName(action);
	put(edgePMap, e, ep);
	return true;
}

bool ASMC::isValidObject(string& reason) const
{
	// An ASMC is valid if it has at least one state, and for every state
	// there is at least one transition. Absorbing states ar not accepted.
	if (num_vertices(graph) == 0) {
		reason = "The ASMC has no states.";
		return false;
	}
	
	vector<bool> has_out_edge(num_vertices(graph), false);
	edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(graph); eit != eit_end; ++eit) {
		has_out_edge[source(*eit, graph)] = true;
	}
	for (size_t i=0; i<has_out_edge.size(); i++) {
		if (!has_out_edge[i]) {
			reason = (format("State at index %1% is an absorbing state.")
					  % i).str();
			return false;
		}
	}
	return true;
}

bool ASMC::WriteAsGmlFile(ostream& os, const char *name, const char *args) 
{
	os << "Creator \"CSL-TA Solver\"" << endl;
	os << "graph [" << endl;
	os << "hierarchic 1 \nlabel \""<<name<<"\"\ndirected 1" << endl;
	
	// Write the states
	vertex_iter_t it, it_end;
	for (tie(it, it_end) = vertices(graph); it != it_end; ++it) {
		const VertexP& vp = get(vertPMap, *it);
		vertex_t vid = *it;
		string label = htmlize_utf8_str(GetGmlNameForVertex(vp));
		os << "node [ \n id "<<vid<<"\n label "<<label<<endl;
		os << " graphics [\n  type \"roundrectangle\"\n  fill \"#FFFFFF\"";
		os << "\n  outline \"#000000\"\n";
		os << "  w 70\n  h 50\n  x "<<(vid%5)*90<<"\n  y "<<(vid/5)*70<<"\n ]" << endl;
		os << " LabelGraphics [\n  text "<<label<<"\n  fontsize 12\n";
		os << "  fontname \"Dialog\"\n  anchor \"c\"\n ]" << endl;
		os << "]" << endl;
	}
	// Write the transitions
	edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(graph); eit != eit_end; ++eit) {
		const EdgeP& ep = get(edgePMap, *eit);
		vertex_t s = source(*eit, graph);
		vertex_t t = target(*eit, graph);
		ostringstream label;
		label << "\"(" << htmlize_utf8_str(Actions.GetSymbolName(ep.actionID));
		label << ", " << ep.lambda << ")\"";
		os << "edge [\n source "<<s<<"\n target "<<t<<" label "<<label.str()<<endl;
		os << " graphics [\n  fill \"#000000\"\n  targetArrow \"delta\"\n ]\n";
		os << " LabelGraphics [\n  text "<<label.str()<<"\n  fontSize 12"<<endl;
		os << "  fontName \"dialog\"\n  model \"six_pos\"\n  position \"tail\"";
		os << "\n ]\n]" << endl;		
	}
	
	os << "]" << endl;
	return true;
}


string ASMC::GetGmlNameForVertex(const VertexP& vp) const {
	string s("\"");
	s += vp.vertexName + "\n";
	size_t numAP = 0;
	for (size_t i=0; i<vp.labelVals.size(); i++) {
		if (vp.labelVals[i] > 0) {
			if (++numAP > 1)
				s += ", ";
			else
				s += "{ ";
			s += AtomicProps.GetSymbolName(i);
			if (vp.labelVals[i] > 1) {
				s += "=";
				s += vp.labelVals[i];
			}
		}
	}
	if (numAP > 0)
		s += " }\"";
	else
		s += "∅\"";
	return s;
}


ASMC::vertex_iter_t ASMC::FindVertex(const string& stName)
{
	vertex_iter_t it, it_end;
	tie(it, it_end) = vertices(graph);
	
	map<string, vertex_t>::const_iterator mapIt;
	mapIt = vertexName2Index.find(stName);
	if (mapIt != vertexName2Index.end())
		return it + ptrdiff_t(mapIt->second);
	return it_end;
	
	/*vertex_iter_t it, it_end;
	for (tie(it, it_end) = vertices(graph); it != it_end; ++it) {
		if (stName == get(vertPMap, *it).vertexName)
			return it;
	}
	return it_end;*/
}


int ASMC::FindBSCCs(vector<int>& outBSCCs) const
{
	outBSCCs.resize(0);
	outBSCCs.resize(num_vertices(graph), -1);
	
	// Extract the SCC
	int num = strong_components(graph, &outBSCCs[0]);
	
	// Find out which SCC aren't Bottom SCC
	vector<bool> notBottomSCC(num, false);
	edge_iter_t eit, eit_end;
	for (tie(eit, eit_end) = edges(graph); eit != eit_end; ++eit) {
		vertex_t s = source(*eit, graph);
		vertex_t t = target(*eit, graph);
		// If there's an edge between the SCC of s and the SCC of t, then
		// the first is not a BSCC.
		if (outBSCCs[s] != outBSCCs[t])
			notBottomSCC[ outBSCCs[s] ] = true;
	}
	
	// Remap indices in outBSCCs
	vector<int> remap(num, -1);
	int numBSCC = 0;
	for (int i=0; i<num; i++) {
		if (notBottomSCC[i])
			remap[i] = -1;
		else
			remap[i] = numBSCC++;
	}
	for (int i=0; i<(int)outBSCCs.size(); i++)
		outBSCCs[i] = remap[ outBSCCs[i] ];
	
	/*vertex_iter_t it, it_end;
	for (tie(it, it_end) = vertices(graph); it != it_end; ++it) {
		cout << "  BSCC[" << get(vertPMap, *it).vertexName << "] = ";
		cout << outBSCCs[*it] << endl;
	}
	cout << "  numBSCC = " << numBSCC << endl;*/
	
	return numBSCC;
}





//-----------------------------------------------------------------------------

PUBLIC_API void AsmcState_Delete(AsmcState *st) {
	delete st;
}
PUBLIC_API AsmcState* AsmcState_New(const char* name, IDValList *idvl) {
	return new AsmcState(idvl == NULL ? new IDValList : idvl, name);
}
PUBLIC_API void AsmcTransition_Delete(AsmcTransition *aarc) {
	delete aarc;
}
PUBLIC_API AsmcTransition* AsmcTransition_New(const char *from, const char *to, 
											  const char *act, double lambda) 
{
	return new AsmcTransition(from, to, act, lambda);
}
PUBLIC_API void	Asmc_Delete(ASMC *asmc) {
	delete asmc;
}
PUBLIC_API ASMC* Asmc_New() {
	return new ASMC;
}
PUBLIC_API ASMC* Asmc_AddState(ASMC *asmc, AsmcState *ast) {
	if (!asmc->AddState(ast->name, ast->apl->idValList))
		GS_SetError("couldn't add the ASMC state");
	delete ast;
	return asmc;
}
PUBLIC_API ASMC* Asmc_AddTransition(ASMC *asmc, AsmcTransition *atrn) {
	if (atrn->lambda <= 0)
		GS_SetError("lambda must be a positive number.");
	else if (!asmc->AddEdge(atrn->from, atrn->to, atrn->lambda, atrn->action))
		GS_SetError("couldn't add the ASMC transition");
	delete atrn;
	return asmc;
}


