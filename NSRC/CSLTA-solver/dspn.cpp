/*
 *  dspn.cpp
 *  CslTA-Solver
 *
 *  Implementation of a Deterministic and Stochastic Petri Net
 *
 */

#include "precompiled.pch"

using namespace boost;
using namespace std;

#include "common-defs.h"
#include "dspn.h"

//-----------------------------------------------------------------------------

const size_t DSPN::FIXED_ARC_MULTIPLICITY = (size_t)-1;

DSPN::Place::Place(const string& _name, size_t _initMark, 
				   const string& _cluster) 
: name(_name), initMark(_initMark), cluster(_cluster)
{ }


DSPN::Transition::Transition(const string& _name, TrKind _kind, 
							 double _delay, size_t _priority, 
							 TrSrvc _service, const string& _cluster)
: name(_name), kind(_kind), delay(_delay), 
  priority(_priority), service(_service), cluster(_cluster)
{ }


DSPN::Arc::Arc(size_t _place, size_t _tran, ArcKind _kind, 
			   size_t _mult, size_t _mdPlace)
: place(_place), tran(_tran), kind(_kind), mult(_mult), mdPlace(_mdPlace)
{ }


size_t DSPN::AddPlace(const string& name, size_t initMark, 
					  const string& cluster) 
{
	assert(int(initMark) >= 0);
	places.push_back(Place(name, initMark, cluster));
	return places.size() - 1;
}


size_t DSPN::AddImmTran(const string& name, double weight, 
						size_t priority, const string& cluster) 
{
	assert(weight > 0 && priority >= 1);
	transitions.push_back(Transition(name, IMM, weight, priority, 
									 SingleSrv, cluster));
	return transitions.size() - 1;
}


size_t DSPN::AddExpTran(const string& name, double delay, 
						TrSrvc srv, const string& cluster) 
{
	assert(delay > 0);
	transitions.push_back(Transition(name, EXP, delay, 0, srv, cluster));
	return transitions.size() - 1;
}


size_t DSPN::AddDetTran(const string& name, double delay, 
						TrSrvc srv, const string& cluster) 
{
	assert(delay > 0);
	transitions.push_back(Transition(name, DET, delay, 0, srv, cluster));
	return transitions.size() - 1;
}


size_t DSPN::AddInputArc(size_t srcPlace, size_t dstTran, size_t mult) {
	assert(srcPlace < places.size() && dstTran < transitions.size());
	arcs.push_back(Arc(srcPlace, dstTran, INPUT, mult, FIXED_ARC_MULTIPLICITY));
	return arcs.size() - 1;
}


size_t DSPN::AddOutputArc(size_t srcTran, size_t dstPlace, size_t mult) {
	assert(srcTran < transitions.size() && dstPlace < places.size());
	arcs.push_back(Arc(dstPlace, srcTran, OUTPUT, mult, FIXED_ARC_MULTIPLICITY));
	return arcs.size() - 1;
}


size_t DSPN::AddInhibArc(size_t srcPlace, size_t dstTran, size_t mult) {
	assert(srcPlace < places.size() && dstTran < transitions.size());
	arcs.push_back(Arc(srcPlace, dstTran, INHIB, mult, FIXED_ARC_MULTIPLICITY));
	return arcs.size() - 1;
}


pair<size_t, size_t> 
DSPN::AddInputOutputArcs(size_t place, size_t tran, size_t mult) {
	assert(place < places.size() && tran < transitions.size());
	return make_pair(AddInputArc(place, tran, mult),
					 AddOutputArc(tran, place, mult));
}


void DSPN::MergeWithPlaceSuperpos(const DSPN& dspn, const vector<string>& spPlcs)
{
	size_t startPl2 = places.size();
	size_t startTr2 = transitions.size();
	// Place Union:
	//   P(out) = (P1-P*) U (P2-P*) U (P*)
	map<size_t, size_t> mergedPlaces;
	for (size_t n=0; n<spPlcs.size(); n++) {
		size_t pl1 = FindPlaceByName(spPlcs[n].c_str());
		size_t pl2 = dspn.FindPlaceByName(spPlcs[n].c_str());
		if (pl1 != size_t(-1) && pl2 != size_t(-1)) {
			// Place dspn[pl2] will be merged with [pl1] - remap its index
			mergedPlaces[pl2] = pl1;
		}
	}
	
	// Copy (P2-P*) places from dspn
	vector<size_t> remapPlaces(dspn.places.size());
	for (size_t i=0, j=0; i<dspn.places.size(); i++) {
		if (mergedPlaces.count(i))
			remapPlaces[i] = mergedPlaces[i];
		else {
			places.push_back(dspn.places[i]);
			remapPlaces[i] = startPl2 + j;
			j++;
		}
	}
	
	// Keep clustering for places, if any
	map<size_t, size_t>::const_iterator mrgIt;
	for (mrgIt = mergedPlaces.begin(); mrgIt != mergedPlaces.end(); ++mrgIt) {
		if (dspn.places[mrgIt->first].cluster.size() > 0 &&
			places[mrgIt->second].cluster.size() == 0)
		{
			places[mrgIt->second].cluster = 
				dspn.places[mrgIt->first].cluster;
			places[mrgIt->second].initMark = 
				dspn.places[mrgIt->first].initMark;
		}
	}

	// Copy the transitions
	copy(dspn.transitions.begin(), dspn.transitions.end(), 
		 back_inserter(transitions));

	// Copy the arcs, while remapping place indices
	VectorOfArcs::const_iterator arcIt;
	for (arcIt = dspn.arcs.begin(); arcIt != dspn.arcs.end(); ++arcIt) {
		arcs.push_back(*arcIt);
		arcs[arcs.size()-1].place = remapPlaces[arcs[arcs.size()-1].place];
		arcs[arcs.size()-1].tran += startTr2;
	}
}


void DSPN::WriteAsGmlFile(ostream& os, const char *name, bool writeUID) const
{
	VectorOfArcs::const_iterator arcit;
	VectorOfPlaces::const_iterator plcit;
	VectorOfTransitions::const_iterator trnit;
	size_t trnStart = places.size();
	size_t clustStart = trnStart + transitions.size();
	
	os << "graph [ hierarchic 1"<<endl;

	// Extract cluster names from places and transitions
	map<string, size_t> clusterIndex;
	clusterIndex[""] = clustStart;
	for (plcit = places.begin(); plcit != places.end(); ++plcit)
		if (clusterIndex.count(plcit->cluster) == 0)
			clusterIndex[plcit->cluster] = clusterIndex.size() + clustStart;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit)
		if (clusterIndex.count(trnit->cluster) == 0)
			clusterIndex[trnit->cluster] = clusterIndex.size() + clustStart;
		
	// Write clusters
	map<string, size_t>::const_iterator clsit;
	for (clsit = clusterIndex.begin(); clsit != clusterIndex.end(); ++clsit) {
		os << (format("node [ id %1% label \"%2%\" graphics [ fill \"#F8F8F8\" "
					  "  type \"roundrectangle\" outline \"#000000\" "
					  "  outlineStyle \"dashed\" ] LabelGraphics [ text \"%2% \" "
					  "  fill \"#EBEBEB\" fontSize 15 fontName \"Dialog\" "
					  "alignment \"right\" autoSizePolicy \"node_width\" "
					  "anchor \"t\" borderDistance 0 ] isGroup 1 ")
			   % clsit->second 
			   % htmlize_utf8_str(clsit->first!="" ? clsit->first : name));
		if (clsit->first != "")
			os << "gid "<<clusterIndex[""];
		os << " ]" << endl;
	}
	
	
	// Write places
	for (plcit = places.begin(); plcit != places.end(); ++plcit) {
		ostringstream label;
		if (writeUID)
			label << 'P' << (plcit - places.begin()) << "-";
		label << plcit->name << "\n\n";
		if (plcit->initMark == 1)
			label << "&#x2022;";
		else if (plcit->initMark > 1)
			label << plcit->initMark;
		else label << " ";
		label << "\n\n ";
		os << (format("node [ id %1% label \"%2%\" graphics [ w 40 h 40 "
					  " type \"ellipse\" fill \"#FFFFFF\" outline \"#000000\" ] "
					  " LabelGraphics [ text \"%2%\" fontSize 12 "
					  "  fontname \"Dialog\" anchor \"c\" ] gid %3% ]\n")
			   % (plcit - places.begin()) 
			   % htmlize_utf8_str(label.str())
			   % clusterIndex[plcit->cluster]) << endl;
	}
	
	// Write transitions
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit) {
		ostringstream label;
		if (writeUID)
			label << 'T' << (trnit - transitions.begin()) << "-";
		if (!trnit->name.empty())  
			label << " \n";
		if (trnit->kind == IMM)  
			label << " \n\n\n     "<<(trnit->priority>=10 ? " ":"")<<trnit->priority;
		else label << " \n\n\n\n\nλ=" << trnit->delay;
		if (!trnit->name.empty())  
			label << "\n" << trnit->name;
		os << (format("node [ id %1% label \"%2%\" graphics [ w %3% h %4% "
					  " type \"rectangle\" fill \"%5%\" outline \"#000000\" "
					  " outlineWidth %6% ] LabelGraphics [ text \"%2%\" fontSize 10 "
					  "  fontname \"Dialog\" anchor \"c\" ] gid %7% ]\n")
			   % ((trnit - transitions.begin()) + trnStart)
			   % htmlize_utf8_str(label.str())
			   % (trnit->kind==IMM ? 4 : 16)
			   % 40
			   % (trnit->kind==EXP ? "#FFFFFF" : "#000000")
			   % (trnit->kind==EXP ? 1 : 2)
			   % clusterIndex[trnit->cluster]) << endl;		
	}
	
	// Write arcs
	for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		ostringstream label;
		if (writeUID)
			label << 'A' << (arcit - arcs.begin()) << "-";
		if (arcit->mdPlace != FIXED_ARC_MULTIPLICITY)
			label << "#" << places[arcit->mdPlace].name << " - " << arcit->mult;
		else if (arcit->mult > 1)  
			label << arcit->mult;
		os << (format("edge [ source %1% target %2% label \"%3%\" graphics [ "
					  "fill \"%4%\" targetArrow \"%5%\" ] LabelGraphics [ "
					  "text \"%3%\" fontsize 12 fontname \"Dialog\" "
					  "model \"six_pos\" position \"thead\" ] ] ")
			   % (arcit->getSource() + (arcit->isSourceAPlace() ? 0 : trnStart))
			   % (arcit->getTarget() + (arcit->isTargetAPlace() ? 0 : trnStart))
			   % htmlize_utf8_str(label.str())
			   % (places[arcit->place].cluster == 
				  transitions[arcit->tran].cluster ? "#000000" : "#A0A0A0")
			   % (arcit->kind==INHIB ? "white_diamond" : "delta")) << endl;
	}
	
	os << "]";
}


void DSPN::PrintDSPN() const
{
	VectorOfArcs::const_iterator arcit;
	VectorOfPlaces::const_iterator plcit;
	VectorOfTransitions::const_iterator trnit;
	cout << "------------------------------------------"<<endl;
	cout << "Places: "<<places.size()<<endl;
	for (plcit = places.begin(); plcit != places.end(); ++plcit) {
		cout << " (" << (plcit - places.begin()) << ") \"";
		cout << plcit->name << "\"  " << plcit->initMark<<endl;
	}		 
	cout << "\nTransitions: "<<transitions.size()<<endl;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit) {
		cout << " (" << (trnit - transitions.begin()) << ") \"";
		const char* tk[] = { "IMM", "EXP", "DET" };
		cout << trnit->name << "\"  " << tk[trnit->kind] << "  d=" << trnit->delay;
		cout << "  pri=" << trnit->priority << endl;
	}		 
	cout << "\nArcs: "<<arcs.size()<<endl;
	for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		cout << " (" << (arcit - arcs.begin()) << ") ";
		cout << (arcit->kind==OUTPUT ? "TR(" : "PL(");
		cout << arcit->getSource() << ") -> ";
		cout << (arcit->kind!=OUTPUT ? "TR(" : "PL(");
		cout << arcit->getTarget() << ")   ";
		const char* ak[] = { "INP", "OUT", "HIB" };
		cout << ak[arcit->kind]<< "  " << arcit->mult << " #" << arcit->mdPlace << endl;
	}
	cout << "------------------------------------------"<<endl<<endl<<endl;
}

void DSPN::WriteAsGmlFile(const char *filename, const char *name, 
						  bool writeUID) const
{
	ofstream out(filename);
	WriteAsGmlFile(out, name, writeUID);
	out.close();
}


void DSPN::WriteAsDSPNexpressFile(ostream& net, ostream& def, 
								  const char *name, const char *measures) const
{
	VectorOfArcs::const_iterator arcit;
	VectorOfPlaces::const_iterator plcit;
	VectorOfTransitions::const_iterator trnit;
	const double LABEL_XOFFSET = 0.16, LABEL_YOFFSET = -0.09;
	
	// Optimize arc searches
	multimap<size_t, const Arc*> arcsByTrn;
	for (VectorOfArcs::const_iterator arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		arcsByTrn.insert(make_pair(arcit->tran, &(*arcit)));
	}

	// DSPNexpress model.NET file
	net << "# FILE <MODEL>.NET CONTAINING STRUCTURAL DESCRIPTION OF A DSPN\n\n";
	net << "PLACES: "<<places.size()<<"\n";
	net << "TRANSITIONS: "<<transitions.size()<<"\n";
	net << "DELAY_PARAMETERS: 0\n";
	net << "MARKING_PARAMETERS: 0\n";
	
	net << "\n# LIST OF MARKING PARAMETERS (NAME, VALUE, (X,Y)-POSITION):\n";
	net << "\n# LIST OF PLACES (NAME, MARKING, (X,Y)-POSITION (PLACE & TAG))\n\n";
	size_t plcCnt = 0;
	for (plcit = places.begin(); plcit != places.end(); ++plcit) {
		net << (format("PLACE %1% %2% %3% %4% %5% %6% \n")
				% GetDSPNexprName(plcit->name, 'P', plcCnt)
				% plcit->initMark
				% plcit->pos.X 
				% plcit->pos.Y 
				% (plcit->pos.X + LABEL_XOFFSET) 
				% (plcit->pos.Y + LABEL_YOFFSET));
		plcCnt++;
	}
	
	net << "\n# LIST OF DELAY PARAMETERS (NAME, VALUE, (X,Y)-POSITION):\n";
	
	net << "\n# LIST OF TRANSITIONS\n";
	net << "# 	(NAME, DELAY, ENABLING DEPENDENCE, KIND, PRIORITY, ORIENTATION,\n";
	net << "# 	(X,Y)-POSITION (TRANSITION, TAG & DELAY), ARCS)\n\n";
	const char *kinds[] = { "IM", "EXP", "DET" };
	size_t trnCnt = 0;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit) {
		net << (format("TRANSITION %1% %2$1.15e %3% %4% %5% 0 %6% %7% %8% %9% %10% %11% \n")
				% GetDSPNexprName(trnit->name, 'T', trnCnt)
				% (trnit->kind==EXP ? 1.0/trnit->delay : trnit->delay)
				% (trnit->service==SingleSrv ? "SS" : "IS")
				% kinds[trnit->kind]
				% trnit->priority
				% trnit->pos.X 
				% trnit->pos.Y 
				% (trnit->pos.X + LABEL_XOFFSET) 
				% (trnit->pos.Y + LABEL_YOFFSET)
				% 0.18 % 0.18);
		const char *arcKinds[] = { "INPARCS", "OUTPARCS", "INHARCS" };
		for (size_t ak=0; ak<3; ak++) {
			size_t num = 0;
			ostringstream arcss;
			multimap<size_t, const Arc*>::const_iterator arcit, arcit_end;
			arcit     = arcsByTrn.lower_bound(trnCnt);
			arcit_end = arcsByTrn.upper_bound(trnCnt);
			while (arcit != arcit_end) {
				const Arc* pArc = arcit->second;
				if (pArc->kind == (ArcKind)ak && pArc->tran == trnCnt) 
				{
					if (pArc->mdPlace == FIXED_ARC_MULTIPLICITY)
						arcss << pArc->mult << " ";
					else
						arcss << "<MD> ";
					arcss << GetDSPNexprName(places[pArc->place].name,
											 'P', pArc->place);
					arcss << " " << pArc->line.size() << "\n";
					for (size_t lc=0; lc<pArc->line.size(); lc++) {
						arcss << (pArc->line[lc].X) << " ";
						arcss << (pArc->line[lc].Y) << "\n";
					}
					num++;
				}
				++arcit;
			}
			net << arcKinds[ak] << " " << num << "\n";
			net << arcss.str();
		}
		trnCnt++;
		net << "\n";
	}
	
	net << "# END OF DSPN FILE" << endl;
	
	// DSPNexpress model.DEF file
	def << "# FILE <MODEL>.DEF CONTAINING TEXTUAL DESCRIPTION OF A DSPN\n\n";
	def << "# DEFINITION OF PARAMETERS:\n\n";
	def << "# MARKING DEPENDENT FIRING DELAYS FOR EXP. TRANSITIONS:\n\n";
	def << "# MARKING DEPENDENT FIRING DELAYS FOR DET. TRANSITIONS:\n\n";
	def << "# MARKING DEPENDENT WEIGHTS FOR IMMEDIATE TRANSITIONS:\n\n";
	def << "# ENABLING FUNCTIONS FOR IMMEDIATE TRANSITIONS:\n\n";
	def << "# MARKING DEPENDENT ARC CARDINALITIES:\n\n";
	const char *arcMdKinds[] = { "INPARC", "OUTPARC", "INHARC" };
	for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		if (arcit->mdPlace != FIXED_ARC_MULTIPLICITY) {
			def << "CARDINALITY "<<arcMdKinds[arcit->kind]<< " ";
			def << GetDSPNexprName(transitions[arcit->tran].name, 'T', arcit->tran) << " ";
			def << GetDSPNexprName(places[arcit->place].name, 'P', arcit->place) << "\n";
			def << " #" << GetDSPNexprName(places[arcit->mdPlace].name, 'P', arcit->mdPlace);
			def << " - " << arcit->mult << ";\n";
		}
	}
	def << "\n";
	def << "# REWARD MEASURES:\n";
	def << measures << "\n";
	def << "# END OF DSPN-SPECIFICATION FILE" << endl;
}

string DSPN::GetDSPNexprName(const string& str, char pre, size_t count) const
{
	struct TrTblEntry { const char *from, *to; };
	const TrTblEntry TranslTbl[] = {
		{" ", ""}, {"(", "Op"}, {")", "Cl"}, {"<", "Less"}, 
		{">", "Great"}, {"∧", "And"}, {"∨", "Or"}, {"¬", "Not"},
		{"{", "Op"}, {"}", "Cl"}, {"⊥", "Bottom"}, {"=", "Eq"}, 
		{"->", "Arrow"}, {".", "Dot"}, {"-", "Minus"}, {"+", "Plus"},
		{"α", "Alpha"}, {"β", "Beta"}, {"γ", "Gamma"}, {"δ", "Delta"}, 
		{"ε", "Epsilon"}, {"ζ", "Zeta"}, {"η", "Eta"}, {"θ", "Theta"}, 
		{"ι", "Iota"}, {"κ", "Kappa"}, {"λ", "Lambda"},{"μ", "Mu"}, 
		{"ν", "Nu"}, {"ξ", "Xi"}, {"ο", "Omicron"}, {"π", "Pi"}, 
		{"ρ", "Rho"}, {"σ", "Sigma"},{"τ", "Tau"}, {"υ", "Upsilon"}, 
		{"φ", "Phi"}, {"χ", "Chi"}, {"ψ", "Psi"}, {"ω", "Omega"}, 
		{"ϕ", "Phi"},  
	};
	const size_t NUM_TRTBL_ENTRIES = sizeof(TranslTbl) / sizeof(TranslTbl[0]);
	
	// Special cases
	if (pre=='P') {
		if (str == "T")			return "PxTop";
		if (str == "⊥")			return "PxBot";
		if (str == "NonDet")	return "PxNonDet";
	}
	
	ostringstream out;
	out << pre << count << "x";
	for (size_t i=0; i<str.size(); i++) {
		if (isalnum(str[i]))
			out << str[i];
		else {
			// Try translating...
			bool translationDone = false;
			for (size_t t=0; t<NUM_TRTBL_ENTRIES; t++) {
				if (0 == strncmp(TranslTbl[t].from, 
								 str.c_str() + i,
								 strlen(TranslTbl[t].from)))
				{
					// Substitute the non-alphanumeric string with it's
					// alfanumeric (readable) equivalent
					out << TranslTbl[t].to;
					translationDone = true;
					i += strlen(TranslTbl[t].from)-1;
					break;
				}
			}
			// Write the character as an octal number
			if (!translationDone) {
				out << char('0' + ((str[i] & 0300) >> 6));
				out << char('0' + ((str[i] &  070) >> 3));
				out << char('0' +  (str[i] &  007));
			}
		}
	}
	
	string name = out.str();
	if (name.size() > 20)
		name.resize(20);
	return name;
}



void DSPN::WriteAsTimeNetFile(ostream& net, const char *name) const
{
	VectorOfArcs::const_iterator arcit;
	VectorOfPlaces::const_iterator plcit;
	VectorOfTransitions::const_iterator trnit;
	const double LABEL_XOFFSET = 0.16, LABEL_YOFFSET = -0.09;
	
	// Optimize arc searches
	multimap<size_t, const Arc*> arcsByTrn;
	for (VectorOfArcs::const_iterator arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		arcsByTrn.insert(make_pair(arcit->tran, &(*arcit)));
	}

	// DSPNexpress model.NET file
	net << "-- FILE "<<name<<".TN CONTAINING STRUCTURAL DESCRIPTION OF A NET\n\n";
	net << "NET_TYPE:       eDSPN\n";
	net << "DESCRIPTION:    ?\n";
	net << "PLACES: "<<places.size()<<"\n";
	net << "TRANSITIONS: "<<transitions.size()<<"\n";
	net << "DELAY_PARAMETERS: 0\n";
	net << "MARKING_PARAMETERS: 0\n";
	net << "REWARD_MEASURES: 0\n";
	
	net << "\n-- LIST OF MARKING PARAMETERS (NAME, VALUE, (X,Y)-POSITION):\n";
	net << "\n-- LIST OF PLACES (NAME, MARKING, (X,Y)-POSITION (PLACE & TAG)):\n\n";
	size_t plcCnt = 0;
	for (plcit = places.begin(); plcit != places.end(); ++plcit) {
		net << (format("PLACE %1% %2% %3% %4% %5% %6% \n")
				% GetDSPNexprName(plcit->name, 'P', plcCnt)
				% plcit->initMark
				% plcit->pos.X 
				% plcit->pos.Y 
				% (plcit->pos.X + LABEL_XOFFSET) 
				% (plcit->pos.Y + LABEL_YOFFSET));
		plcCnt++;
	}
	
	net << "\n-- LIST OF DELAY PARAMETERS (NAME, VALUE, (X,Y)-POSITION):\n\n";
	
	net << "-- LIST OF TRANSITIONS\n";
	net << "-- 	(NAME, DELAY, ENABLING DEPENDENCE, KIND, FIRING POLICY, PRIORITY,\n";
	net << "-- 	ORIENTATION, PHASE, GROUP, GROUP_WEIGHT,\n";
	net << "-- 	(X,Y)-POSITION (TRANSITION, TAG & DELAY), ARCS)\n\n";
	const char *kinds[] = { "IM", "EXP", "DET" };
	size_t trnCnt = 0;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit) {
		net << (format("TRANSITION %1% %2% %3% %4% RE %5% 0 1 0 1.0 %6% %7% %8% %9% %10% %11% \n")
				% GetDSPNexprName(trnit->name, 'T', trnCnt)
				% trnit->delay
				% (trnit->service==SingleSrv ? "SS" : "IS")
				% kinds[trnit->kind]
				% trnit->priority
				% trnit->pos.X 
				% trnit->pos.Y 
				% (trnit->pos.X + LABEL_XOFFSET) 
				% (trnit->pos.Y + LABEL_YOFFSET)
				% 0 % 0);
		const char *arcKinds[] = { "INPARCS", "OUTPARCS", "INHARCS" };
		for (size_t ak=0; ak<3; ak++) {
			size_t num = 0;
			ostringstream arcss;
			multimap<size_t, const Arc*>::const_iterator arcit, arcit_end;
			arcit     = arcsByTrn.lower_bound(trnCnt);
			arcit_end = arcsByTrn.upper_bound(trnCnt);
			while (arcit != arcit_end) {
				const Arc* pArc = arcit->second;
				if (pArc->kind == (ArcKind)ak && pArc->tran == trnCnt) 
				{
					if (pArc->mdPlace == FIXED_ARC_MULTIPLICITY)
						arcss << pArc->mult << " ";
					else
						arcss << "<MD> ";
					arcss << GetDSPNexprName(places[pArc->place].name,
											 'P', pArc->place);
					arcss << " " << pArc->line.size() << "\n";
					for (size_t lc=0; lc<pArc->line.size(); lc++) {
						arcss << (pArc->line[lc].X) << " ";
						arcss << (pArc->line[lc].Y) << "\n";
					}
					num++;
				}
				++arcit;
			}
			net << arcKinds[ak] << " " << num << "\n";
			net << arcss.str();
		}
		trnCnt++;
		net << "\n";
	}
	
	net << "-- DEFINITION OF PARAMETERS:\n\n";
	net << "-- MARKING DEPENDENT FIRING DELAYS FOR EXP. TRANSITIONS:\n\n\n";
	net << "-- MARKING DEPENDENT FIRING DELAYS FOR DET. TRANSITIONS:\n\n\n";
	net << "-- PROBABILITY MASS FUNCTION DEFINITIONS FOR GEN. TRANSITIONS:\n\n\n";
	net << "-- MARKING DEPENDENT WEIGHTS FOR IMMEDIATE TRANSITIONS:\n\n\n";
	net << "-- ENABLING FUNCTIONS FOR IMMEDIATE TRANSITIONS:\n\n\n";
	net << "-- MARKING DEPENDENT ARC CARDINALITIES:\n\n\n";
	const char *arcMdKinds[] = { "INPARC", "OUTPARC", "INHARC" };
	for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		if (arcit->mdPlace != FIXED_ARC_MULTIPLICITY) {
			net << "CARDINALITY "<<arcMdKinds[arcit->kind]<< " ";
			net << GetDSPNexprName(transitions[arcit->tran].name, 'T', arcit->tran) << " ";
			net << GetDSPNexprName(places[arcit->place].name, 'P', arcit->place) << "\n";
			net << " #" << GetDSPNexprName(places[arcit->mdPlace].name, 'P', arcit->mdPlace);
			net << " - " << arcit->mult << ";\n";
		}
	}
	net << "\n";
	net << "-- REWARD MEASURES:\n\n\n";
	net << "-- END OF SPECIFICATION FILE" << endl;
}


void DSPN::WriteAsGreatSPNFile(ostream& net, ostream& def, 
							   const char *name, const char *measures) const
{
	VectorOfPlaces::const_iterator plcit;
	VectorOfTransitions::const_iterator trnit;

	net << "|0|\n";
	net << "Generated by CSL-TA Solver.\n";
	net << "|\n";
	
	// Extract groups: a group is a set of immediate transitions that shares
	// the same priority.
	typedef list< pair<const Transition*, size_t> >  TrnGroupList_t;
	typedef map<size_t, TrnGroupList_t>  GroupMap_t;
	GroupMap_t groups;
	size_t trnCnt = 0;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit) {
		if (trnit->kind == IMM) {
			if (groups.count(trnit->priority) == 1) {
				// Add *trnit to the existing group
				groups[trnit->priority].push_back(make_pair(&(*trnit), trnCnt));
			}
			else {
				// Create a new group for *trnit
				TrnGroupList_t ng;
				ng.push_back(make_pair(&(*trnit), trnCnt));
				groups[trnit->priority] = ng;
			}
		}
		trnCnt++;
	}

	// Write header with place/transition/group counts
	net << "f 0 "<<places.size()<<" 0 "<<transitions.size()<<" ";
	net << groups.size()<<" 0 0\n";

	// Write places
	size_t plcCnt = 0;
	for (plcit = places.begin(); plcit != places.end(); ++plcit) {
		string plcName = GetDSPNexprName(plcit->name, 'P', plcCnt+1);
		net << setw(20)<<left<<plcName<<" "<<plcit->initMark<<" ";
		net << plcit->pos.X<<" "<<plcit->pos.Y<<" ";
		net << (plcit->pos.X + 0.1)<<" "<<(plcit->pos.Y - 0.0333)<<" ";
		net << "0\n";
		plcCnt++;
	}
	
	// Write priority groups
	GroupMap_t::const_iterator grpit;
	for (grpit = groups.begin(); grpit != groups.end(); ++grpit) {
		net << "G"<<setw(3)<<left<<grpit->first<<" ";
		net << "0.000 0.000  "<<grpit->first<<"\n";
	}
	
	// Optimize arc searches
	multimap<size_t, const Arc*> arcsByTrn;
	VectorOfArcs::const_iterator arcit;
	for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		arcsByTrn.insert(make_pair(arcit->tran, &(*arcit)));
	}
	
	// Write Timed Transitions
	trnCnt = 0;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit) {
		if (trnit->kind != IMM) {
			WriteGreatSPNTransition(net, *trnit, trnCnt, 
									(trnit->kind==EXP ? 1 : 0),
									(trnit->kind==EXP ? 0 : 127), arcsByTrn);
		}
		trnCnt++;
	}
	
	// Write immediate Transitions, order by priority
	size_t grpNum = 1;
	for (grpit = groups.begin(); grpit != groups.end(); ++grpit) {
		const TrnGroupList_t& trnGrp = grpit->second;
		TrnGroupList_t::const_iterator grtrnit;
		for (grtrnit = trnGrp.begin(); grtrnit != trnGrp.end(); ++grtrnit) {
			WriteGreatSPNTransition(net, *(grtrnit->first), grtrnit->second, 
									1, grpNum, arcsByTrn);
		}
		grpNum++;
	}
	
	// Write the def file
	def << "|256\n";
	def << "%\n";
	def << measures;
	def << "|\n";
}


void DSPN::WriteGreatSPNTransition(ostream& net, const Transition& trn, 
								   size_t trnCnt, size_t enabl, size_t knd,
								   const multimap<size_t, const Arc*>& arcsByTrn) const
{
	char type = (trn.kind == EXP ? 'T' : (trn.kind == DET ? 'D' : 't'));
	char delBuf[32];
	sprintf(delBuf, "%e", trn.delay);
	string trnName = GetDSPNexprName(trn.name, type, trnCnt+1);
	
	// Write input, output and inhibitor arcs
	for (size_t ak=0; ak<3; ak++) {
		size_t num = 0;
		ostringstream arcss;
		multimap<size_t, const Arc*>::const_iterator arcit, arcit_end;
		arcit     = arcsByTrn.lower_bound(trnCnt);
		arcit_end = arcsByTrn.upper_bound(trnCnt);
		while (arcit != arcit_end) {
			const Arc* pArc = arcit->second;
			if (pArc->kind == (ArcKind)ak && pArc->tran == trnCnt) 
			{
				assert(pArc->mdPlace == FIXED_ARC_MULTIPLICITY);
				arcss << setw(4) << pArc->mult << " ";
				arcss << (pArc->place + 1) << " ";
				arcss << (pArc->line.size()) << " 0\n";
				for (size_t lc=0; lc<pArc->line.size(); lc++) {
					arcss << (pArc->line[lc].X) << " ";
					arcss << (pArc->line[lc].Y) << "\n";
				}
				num++;
			}
			arcit++;
		}
		if (ak == 0) {
			// Write transition header, with input arc count
			net << setw(20)<<left<<trnName<<" "<<delBuf<<" ";
			net << enabl << " " << knd << " " << num << " 1 ";
			net << trn.pos.X<<" "<<trn.pos.Y<<" ";
			net << (trn.pos.X - 0.1666)<<" "<<(trn.pos.Y - 0.1166)<<" ";
			net << (trn.pos.X + 0.1666)<<" "<<(trn.pos.Y + 0.0833)<<" ";
			net << " 0\n";
		}
		else {
			// Write only the arc count
			net << setw(4) << right << num << "\n";
		}
		net << arcss.str();
	}
}



size_t DSPN::AddInitialTangibleMark(double lambda)
{
	size_t start = AddPlace("Start", 1, "");
	size_t initT = AddExpTran("init", lambda, SingleSrv, "");
	AddInputArc(start, initT, 1);
	
	places[start].pos.set(0.2, 0.2);
	transitions[initT].pos.set(0.6, 0.2);
	
	// Set-up the initial tangible marking
	for (size_t p=0; p<places.size(); p++) {
		if (places[p].initMark != 0 && p!=start) {
			AddOutputArc(initT, p, places[p].initMark);
			places[p].initMark = 0;
		}
	}
	
	return start;
}


void DSPN::AddAbsorbingLoops(const char *absorbingPlaceNames[], size_t numAbsPlaces)
{
	// Add a looping transition in each absorbing places
	for (size_t i=0; i<numAbsPlaces; i++) {
		size_t p = FindPlaceByName(absorbingPlaceNames[i]);
		if (p != size_t(-1)) {
			size_t loop = AddExpTran("", 1);
			AddInputOutputArcs(p, loop);
		}
	}
}


bool DSPN::CloseBSCCs(ifstream &bscc, const char *placeName,
					  double expLambda, size_t startPlace)
{	
	size_t numPlaces = places.size();
	
	// Read the *.BSCC file
	string token1, token2;
	size_t numTransient, numBscc;
	bscc >> token1 >> numTransient >> token2 >> numBscc;
	if (token1 != "TRANSIENT-MARKINGS" || token2 != "NUM-BSCC")
		return false;
	
	if (numBscc == 1 && numTransient == 0) {
		// This TRG is ergodic, so there's no need for extra manipulations.
		return true;
	}
	
	// Create the BSCC Sink
	size_t bsccSink = AddPlace(placeName, 0);
	size_t backTrn = AddExpTran("bscc restart", expLambda);
	AddInputArc(bsccSink, backTrn);
	AddOutputArc(backTrn, startPlace);
		
	// Get the max immediate priority
	size_t priority = 1;
	for (size_t t=0; t<transitions.size(); t++)
		if (transitions[t].kind == IMM)
			priority = std::max(priority, transitions[t].priority);
	
	/*size_t fireBsccN = AddPlace("fire Bscc N", 0);
	size_t emptyFireBsccN = AddImmTran("empty fire Bscc N", 1.0, ++priority);
	AddInputArc(fireBsccN, emptyFireBsccN);
	AddInhibArc(bsccSink, emptyFireBsccN);
	size_t bsccCnt = 1;*/
		
	for (size_t i=0; i<numBscc; i++) {
		size_t bsccCount, numMarkings;
		bscc >> token1 >> bsccCount >> token2 >> numMarkings;
		if (token1 != "BSCC" || token2 != "MARKING-COUNT")
			return false;
		
		for (size_t m=0; m<numMarkings; m++) {
			// Add a transition that moves from this marking to the sink
			size_t trn = AddImmTran("", 1.0, ++priority);
			//size_t trn = AddExpTran("", 1.0);
			AddOutputArc(trn, bsccSink);
			//AddOutputArc(trn, fireBsccN, bsccCnt++);
			for (size_t p=0; p<numPlaces; p++) {
				int mult;
				bscc >> mult;
				if (mult != 0)
					AddInputArc(p, trn, mult);
				AddInhibArc(p, trn, mult+1);
			}
		}
	}

	return true;
}


void DSPN::RemoveDoubleArcs(size_t *Nplaces, size_t numNplaces)
{
	// Reserve priority 1 for emptyFireN
	for (size_t t=0; t<transitions.size(); t++) {
		if (transitions[t].kind == DSPN::IMM)
			transitions[t].priority += 1;
	}
	
	size_t fireN = AddPlace("fire N", 0);
	size_t emptyFireN = AddImmTran("empty fire N", 1.0, 1);
	//size_t efN = AddInputArc(fireN, emptyFireN, 1);	
	//arcs[efN].mdPlace = fireN;  // remove #(fireN)-1 tokens
	
	AddInputArc(fireN, emptyFireN, 1);
	
	for (size_t i=0; i<numNplaces; i++)
		if (Nplaces[i]!= size_t(-1))
			AddInhibArc(Nplaces[i], emptyFireN);
	
	size_t expCnt = 1;
	for (size_t t=0; t<transitions.size(); t++) {
		if (transitions[t].kind != IMM)
			AddOutputArc(t, fireN, expCnt++);
	}
}


void DSPN::WriteAsSPNicaFile(ostream& net, const char *name) const
{
	VectorOfArcs::const_iterator arcit;
	VectorOfPlaces::const_iterator plcit;
	VectorOfTransitions::const_iterator trnit;

	net << "(* SPNica definition file of DSPN <"<<name<<"> *)\n";
	
	// Places
	net << "P={ ";
	size_t plcCnt = 0;
	for (plcit = places.begin(); plcit != places.end(); ++plcit, ++plcCnt) {
		if (plcCnt > 0)
			net << ",\n    ";
		net << (GetDSPNexprName(plcit->name, 'P', plcCnt));
	}
	net << " };\n";
	
	// Transitions
	net << "T={ ";
	size_t trnCnt = 0;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit, ++trnCnt) {
		if (trnCnt > 0)
			net << ",\n    ";
		net << "{" << (GetDSPNexprName(trnit->name, 'T', trnCnt)) << ", ";
		switch (trnit->kind) {
			case IMM:
				net << (format("{type->\"imm\", prio->%1%, weight->%2%}")
						% trnit->priority % trnit->delay);
				break;
			case EXP:
				net << (format("{dist->exp[%1%]}") % trnit->delay);
				break;
			case DET:
				net << (format("{dist->I[%1%]}") % trnit->delay);
				break;
		}
		net << "}";
	}
	net << " };\n";
	
	// Input, Output & Inhibitor Arcs
	for (size_t arcType=0; arcType<3; arcType++) {
		const char *varName[] = { "IA", "OA", "HA" };
		net << varName[arcType] << "={ ";
		size_t arcCnt = 0;
		for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
			if (arcit->kind == (ArcKind)arcType) {
				if (arcCnt > 0)
					net << ",\n     ";
				string plcName = GetDSPNexprName(places[arcit->place].name, 
												 'P', arcit->place);
				string trnName = GetDSPNexprName(transitions[arcit->tran].name, 
												 'T', arcit->tran);
				ostringstream mult;
				if (arcit->mdPlace == FIXED_ARC_MULTIPLICITY)
					mult << arcit->mult;
				else {
					mult << "rr[ "<<GetDSPNexprName(places[arcit->mdPlace].name, 
													'P', arcit->mdPlace);
					mult << " ] - " << arcit->mult;
				}
				if (OUTPUT == (ArcKind)arcType)
					net << (format("{%1%, %2%, %3%}") 
							% trnName % plcName % mult.str());
				else
					net << (format("{%1%, %2%, %3%}") 
							% plcName % trnName % mult.str());
				arcCnt++;
			}
		}
		net << " };\n";
	}
	
	// Initial marking M0
	net << "M0={ ";
	plcCnt = 0;
	for (plcit = places.begin(); plcit != places.end(); ++plcit, ++plcCnt) {
		if (plcCnt > 0)
			net << ",\n     ";
		net << (GetDSPNexprName(plcit->name, 'P', plcCnt));
		net << "->" << plcit->initMark;
	}
	net << " };\n";
	
	// Measures
	net << "measures={ ";
	plcCnt = 0;
	for (plcit = places.begin(); plcit != places.end(); ++plcit, ++plcCnt) {
		if (plcCnt > 0)
			net << ",\n           ";
		string name = GetDSPNexprName(plcit->name, 'P', plcCnt);
		net << (format("Mean%1%->mean[rr[ %1% ]]") % name);
	}
	net << " };\n";
	
	// Place names
	net << "PN={ ";
	plcCnt = 0;
	for (plcit = places.begin(); plcit != places.end(); ++plcit, ++plcCnt) {
		if (plcCnt > 0)
			net << ",\n     ";
		string name = GetDSPNexprName(plcit->name, 'P', plcCnt);
		net << name << " -> \"P" << (plcCnt+1) << "[" << plcit->name << "]\"";
	}
	net << " };\n";
	
	// Transition names
	net << "TN={ ";
	trnCnt = 0;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit, ++trnCnt) {
		if (trnCnt > 0)
			net << ",\n     ";
		string name = GetDSPNexprName(trnit->name, 'T', trnCnt);
		net << name << " -> \"T" << (trnCnt+1) << "[" << trnit->name << "]\"";
	}
	net << " };\n";
		
	
	// Final Definitions
	net << "SPN={P,T,IA,OA,HA,M0,measures};\n\n";
	net << "(* SPNstationary[SPN] *)\n" << endl;
}


void DSPN::SimpleGraphicPlanarization()
{
	const double BASE_X = 1, BASE_Y = 1;
	const double HORIZ_INCR = 0.9, VERT_INCR = 3.0;
	
	double X = BASE_X;
	double Y = BASE_Y;
	
	// Assign sequential (X, Y) for each place
	for (size_t p=0; p<places.size(); p++) {
		places[p].pos.set(X, Y);
		X += HORIZ_INCR;
	}
	
	X = BASE_X;
	Y += VERT_INCR;
	// Do the same sequential assignement for tarnsitions, too
	for (size_t t=0; t<transitions.size(); t++) {
		transitions[t].pos.set(X, Y);
		X += HORIZ_INCR;		
		
		size_t trnArcs=0;
		for (size_t ak=0; ak<3; ak++) {
			size_t num = 0;
			for (size_t a=0; a<arcs.size(); a++) {
				if (arcs[a].kind == (ArcKind)ak && arcs[a].tran == t) 
				{
					double placeX = BASE_X + (arcs[a].place+0.2) * HORIZ_INCR;
					double tranX  = BASE_X + (t+0.2) * HORIZ_INCR;
					double ax, ay;
					if (arcs[a].isSourceAPlace()) {
						ax = (placeX*.85 + tranX*0.15 + trnArcs*0.02);
						ay = (BASE_Y + VERT_INCR*.33);
					} else {
						ax = (placeX*.15 + tranX*0.85 + trnArcs*0.02);
						ay = (BASE_Y + VERT_INCR*.67);
					}
					arcs[a].line.push_back(dPoint(ax, ay));
					num++;
					trnArcs++;
				}
			}
		}
	}
}



vector<string> DSPN::_ReadGmlTokens(istream& gml) const
{
	char buffer[1024];
	bool isInString = false;  // reading token surrounded by "".
	string token;
	
	vector<string> tokenRead;
	bool readNextToo = false;
	size_t parCount = 0;
	
	while (gml.getline(buffer, sizeof(buffer))) {
		istringstream strLine(buffer);
		
		while (strLine) {
			if (isInString) {
				int ch = 0;
				while (ch != '\"') {
					ch = strLine.get();
					if (!strLine) {
						token += '\n';
						break;
					}
					token += char(ch);
				}
				if (ch == '\"')
					isInString = false;
				else
					continue; // read next line
			}
			else {
				strLine >> token;
				if (token.size()>0 && token[0]=='\"') {
					if ( !(token.size()>1 && token[token.size()-1]=='\"') ) {
						isInString = true;
						continue;
					}
				}
			}
			
			// Consume the token 
			if (readNextToo) {
				tokenRead.push_back(token);
				readNextToo = false;
			}
			else if (token == "edge" ||
					 token == "node")
			{
				tokenRead.push_back(token);
				parCount = 1;
			}
			else if (parCount && token == "[") {
				parCount++;
			}
			else if (parCount && token == "]") {
				parCount--;
				if (parCount == 1) {
					tokenRead.push_back("end");
					parCount = 0;
				}
			}
			else if (parCount && (token == "label" ||
								  token == "id" ||
								  token == "x" ||
								  token == "y" ||
								  token == "source" ||
								  token == "target") )
			{
				tokenRead.push_back(token);
				readNextToo = true;
			}
			
			token = "";
		}
	}
	
	return tokenRead;
}

void DSPN::ImportGmlCoords(istream& gml)
{
	vector<string> tokenRead = _ReadGmlTokens(gml);
	// At this point, the tokenRead[] vector contains a sequence of token in 
	// this form:
	// <node> <id> <...> <label> <...> <x> <...> <y> <...> <end>
	// <edge> <source> <...> <target> <...> <label> <...> <x> <...> <y> <...> <end>
	
	ClearAllCoords();
	vector<string>::const_iterator it;
	map<int, int> idToPlace, idToTran;
	for (it = tokenRead.begin(); it != tokenRead.end(); ++it) {
		if (*it == "node") {
			it++;
			double X =0, Y =0;
			int id=-1, placeId, tranId;
			string label;
			
			while (*it != "end") {
				if (*it == "label")		label = *(++it);
				else if (*it == "id")	id = atoi((++it)->c_str());
				else if (*it == "x")	X = atof((++it)->c_str());
				else if (*it == "y")	Y = atof((++it)->c_str());
				else ++it;
			}
			
			if (1 == sscanf(label.c_str(), "\"P%d", &placeId)) {
				// we found a place
				if (placeId < (int)places.size()) {
					places[placeId].pos.set(X, Y);
					idToPlace[id] = placeId;
				}
			}
			else if (1 == sscanf(label.c_str(), "\"T%d", &tranId)) {
				// we found a transition
				if (tranId < (int)transitions.size()) {
					transitions[tranId].pos.set(X, Y);
					idToTran[id] = tranId;
				}
			}
		}
		if (*it == "edge") {
			it++;
			vector<double> Xs, Ys;
			int src =0, dst =0, arcId;
			string label;
			
			while (*it != "end") {
				if (*it == "label")			label = *(++it);
				else if (*it == "source")	src = atoi((++it)->c_str());
				else if (*it == "target")	dst = atof((++it)->c_str());
				else if (*it == "x")		Xs.push_back(atof((++it)->c_str()));
				else if (*it == "y")		Ys.push_back(atof((++it)->c_str()));
				else ++it;
			}
			
			if (1 == sscanf(label.c_str(), "\"A%d", &arcId) && 
				Xs.size()==Ys.size()) 
			{
				// we found an arc
				if (arcId < (int)arcs.size()) {
					int pl, tr;
					bool invertArc = false;
					if (idToPlace.count(src) && idToTran.count(dst)) {
						pl = idToPlace[src];
						tr = idToTran[dst];
						invertArc = true;
					}
					else if (idToPlace.count(dst) && idToTran.count(src)) {
						pl = idToPlace[dst];
						tr = idToTran[src];
					}
					else continue;
					
					arcs[arcId].line.clear();
					for (size_t c=0; c<Xs.size(); c++) {
						arcs[arcId].line.push_back(dPoint(Xs[c], Ys[c]));
					}
					if (invertArc)
						reverse(arcs[arcId].line.begin(),
								arcs[arcId].line.end());
					
					// Remove endpoints inside places or transitions
					if (arcs[arcId].line.size() > 0 &&
						arcs[arcId].line[0].distSq(transitions[tr].pos) < 1.0)
						arcs[arcId].line.erase(arcs[arcId].line.begin());
					
					if (arcs[arcId].line.size() > 0 &&
						arcs[arcId].line[arcs[arcId].line.size()-1]
						.distSq(places[pl].pos) < 1.0)
						arcs[arcId].line.pop_back();
				}
			}
			
		}
	}
	
	RescaleAndFixCoords(1.0 / 160.0);
}


bool DSPN::Planarize()
{
	VectorOfArcs::iterator arcit;
	VectorOfPlaces::iterator plcit;
	VectorOfTransitions::iterator trnit;
	
	// Count non-empty cluster names
	map<string, size_t> clusters;
	bool usingDefaultCluster = false;
	bool dontWriteClusters = false;
	for (plcit = places.begin(); plcit != places.end(); ++plcit) {
		if (plcit->cluster == "")
			usingDefaultCluster = true;
		else if(clusters.count(plcit->cluster) == 0)
			clusters[plcit->cluster] = clusters.size() + 1;
	}
	if (clusters.size() == 1 && !usingDefaultCluster) { // there's a single named cluster
		clusters[ clusters.begin()->first ] = 0;
		dontWriteClusters = true;
	}
	clusters[""] = 0;
	
	// Prepare two temporary files
	char gdefFile[L_tmpnam], goutFile[L_tmpnam];
	tmpnam (gdefFile);
	tmpnam (goutFile);
	
	// Write out the planarizer input file
	ofstream gdef(gdefFile);
	gdef << "CLUSTERS " << (dontWriteClusters ? 0 : clusters.size()) << "\n\n";
	
	// write nodes
	gdef << "NODES " << places.size() + transitions.size() << "\n";
	for (plcit = places.begin(); plcit != places.end(); ++plcit) {
		gdef << clusters[plcit->cluster] << " c 40 40 \n";
	}
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit) {
		gdef << clusters[trnit->cluster] << " r ";
		switch (trnit->kind) {
			case IMM:	gdef << "4 40\n";	break;
			default:	gdef << "16 40\n";	break;
		}
	}
	
	// write arcs
	gdef << "\nEDGES " << arcs.size() << "\n";
	for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		if (arcit->isSourceAPlace()) {
			gdef << arcit->place << " " << (arcit->tran + places.size()) << "\n";
		}
		else {
			gdef << (arcit->tran + places.size()) << " " << arcit->place << "\n";
		}
	}	
	gdef.close();
	
	
	// Invoke the OGDF graph planarizer
	ostringstream cmdLine;
	cmdLine << "/Users/elvio/Desktop/Tesi/CslTA-Solver/Planarize "; // FIXME
	cmdLine << gdefFile << " " << goutFile;
	//cmdLine << " /Users/elvio/Desktop/planarized.gml";  //TODO remove
	//cout << "cmdline:" << cmdLine.str() << endl;
	system(cmdLine.str().c_str());
	
	
	// Read back coordinates
	ifstream gin(goutFile);
	for (plcit = places.begin(); plcit != places.end(); ++plcit)
		gin >> plcit->pos.X >> plcit->pos.Y;
	for (trnit = transitions.begin(); trnit != transitions.end(); ++trnit)
		gin >> trnit->pos.X >> trnit->pos.Y;
	for (arcit = arcs.begin(); arcit != arcs.end(); ++arcit) {
		size_t numPoints;
		gin >> numPoints;
		arcit->line.resize(numPoints);
		for (size_t i=0; i<numPoints; i++)
			gin >> arcit->line[i].X >> arcit->line[i].Y;
		if (arcit->isSourceAPlace())
			std::reverse(arcit->line.begin(), arcit->line.end());
	}
	bool allOk = (bool)gin;
	
	RescaleAndFixCoords(1.0 / 160);

	
	// Remove temporary files
	remove(gdefFile);
	remove(goutFile);
	return allOk;
}


void DSPN::RescaleAndFixCoords(double rescale)
{
	dPoint cmin(0,0);
	
	// Find the minimum point
	for (size_t p=0; p<places.size(); p++)
		cmin.min(places[p].pos);
	for (size_t t=0; t<transitions.size(); t++)
		cmin.min(transitions[t].pos);
	for (size_t a=0; a<arcs.size(); a++)
		for (size_t lp=0; lp<arcs[a].line.size(); lp++)
			cmin.min(arcs[a].line[lp]);
	
	// Do a rescale & translate step
	cmin.X *= -1;
	cmin.Y *= -1;
	for (size_t p=0; p<places.size(); p++)
		places[p].pos.translateRescale(cmin, rescale);
	for (size_t t=0; t<transitions.size(); t++)
		transitions[t].pos.translateRescale(cmin, rescale);
	for (size_t a=0; a<arcs.size(); a++)
		for (size_t lp=0; lp<arcs[a].line.size(); lp++)
			arcs[a].line[lp].translateRescale(cmin, rescale);
}


void DSPN::ClearAllCoords()
{
	for (size_t p=0; p<places.size(); p++)
		places[p].pos.set(0, 0);
	for (size_t t=0; t<transitions.size(); t++)
		transitions[t].pos.set(0, 0);
	for (size_t a=0; a<arcs.size(); a++)
		for (size_t lp=0; lp<arcs[a].line.size(); lp++)
			arcs[a].line[lp].set(0, 0);
}


size_t DSPN::FindPlaceByName(const char *name) const
{
	VectorOfPlaces::const_iterator it;
	for (it = places.begin(); it != places.end(); ++it) {
		if (it->name == name)
			return it - places.begin();
	}
	return (size_t)-1;
}





