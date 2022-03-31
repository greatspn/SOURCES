/*
 *  DTA.cpp
 *
 *  Implementation of the DTA interface.
 *
 *  Created by Elvio Amparore
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <climits>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <functional>
using namespace std;

#include <boost/algorithm/string/replace.hpp>  // boost::replace_all
#include <boost/optional.hpp>

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
//#include "ParserDefs.h"
#include "PetriNet.h"
#include "Measure.h"
#include "Language.h"
#include "DTA.h"
#include "newparser.lyy.h"

//=============================================================================

const char *CLOCK_VALUE_NAME_0         = "0";
const char *CLOCK_VALUE_NAME_INFINITY  = "INFINITY";

//=============================================================================

DTA::DTA() {
    clkvals.push_back(ClockValue(clockval_t(clkvals.size()), CLOCK_VALUE_NAME_0));
    clkvals.back().boundedValue = make_shared<ConstValExpr<double> >(0.0);
    clkvals.push_back(ClockValue(clockval_t(clkvals.size()), CLOCK_VALUE_NAME_INFINITY));
    clkvals.back().boundedValue = make_shared<ConstValExpr<double> >
                                  /**/	(std::numeric_limits<double>::infinity());
    isBound = false;
    hasClockZones = false;
    isZoned = false;
}

//=============================================================================

static int iscolon(int ch)  		  {  return (ch == ':');  }
static int isnotsemicolon(int ch)  	  {  return (ch != ';');  }
static int is_ident_ltr(int ch)       {  return isalnum(ch) || (ch == '_'); }

//=============================================================================

void LoadDTAFromFile(istream &is, DTA &dta, VerboseLevel verboseLvl) {
    simple_tokenizer st(&is, skip_spaces_comments);
    st.readtok("DTA").gettok(dta.name, is_ident_ltr).readtok("=").readtok("{");

    // Read clock values and clock constraints
    map<string, clockval_t> clkvalInds;
    st.readtok("CLOCKVALUESET").readtok("=").readtok("{");
    string sep, token;
    do {
        string name;
        st.gettok(name, is_ident_ltr);
        if (name.size() != 0) {
            if (clkvalInds.find(name) != clkvalInds.end())
                throw program_exception(st.pos() + ": Clock value symbol \"" + name + "\" redeclared.");
            if (verboseLvl >= VL_VERBOSE)
                cout << "  CLOCK VALUE: " << name << endl;
            dta.clkvals.push_back(ClockValue(clockval_t(dta.clkvals.size()), name.c_str()));
            clkvalInds[name] = dta.clkvals.back().index;
        }
        st.gettok(sep, ispunct);
    }
    while (sep == ",");
    if (sep != "}")
        throw program_exception(st.pos() + ": Expected comma or \"}\".");

    st.gettok(token, isalpha);
    if (token == "WITH") {
        // read clock constraints
        st.readtok("{");
        do {
            string cv1, cv2;
            st.gettok(cv1, is_ident_ltr);
            if (cv1.size() != 0) {
                if (clkvalInds.find(cv1) == clkvalInds.end())
                    throw program_exception(st.pos() + ": Unknown clock value \"" + cv1 + "\".");

                st.readtok("<").gettok(cv2, is_ident_ltr);
                if (clkvalInds.find(cv2) == clkvalInds.end())
                    throw program_exception(st.pos() + ": Unknown clock value \"" + cv2 + "\".");

                if (verboseLvl >= VL_VERBOSE)
                    cout << "  CLOCK VALUE CONSTRAINT: " << cv1 << " < " << cv2 << endl;
                dta.cvc.push_back(ClockValueConstr(clkvalInds[cv1], clkvalInds[cv2]));
            }
            st.gettok(sep, ispunct);
        }
        while (sep == ",");
        if (sep != "}")
            throw program_exception(st.pos() + ": Expected comma or \"}\".");

        st.gettok(token, isalpha);
    }

    // Read the set of action names
    if (token != "ACTIONSET")
        throw program_exception(st.pos() + ": Expected token \"ACTIONSET\".");
    st.readtok("=").readtok("{");
    map<string, actname_t> actInds;
    do {
        string name;
        st.gettok(name, is_ident_ltr);
        if (name.size() != 0) {
            if (actInds.find(name) != actInds.end())
                throw program_exception(st.pos() + ": Action name \"" + name + "\" redeclared.");
            if (verboseLvl >= VL_VERBOSE)
                cout << "  ACTION NAME: " << name << endl;
            dta.acts.push_back(ActionName(actname_t(dta.acts.size()), name.c_str()));
            actInds[name] = dta.acts.back().index;
        }
        st.gettok(sep, ispunct);
    }
    while (sep == ",");
    if (sep != "}")
        throw program_exception(st.pos() + ": Expected comma or \"}\".");

    // Read the set of state propositions
    map<string, atomicprop_t> atomicPropInds;
    st.readtok("ATOMICPROPOSITIONSET").readtok("=").readtok("{");
    do {
        string name;
        st.gettok(name, is_ident_ltr);
        if (name.size() != 0) {
            if (atomicPropInds.find(name) != atomicPropInds.end())
                throw program_exception(st.pos() + ": State proposition symbol \"" + name + "\" redeclared.");
            if (verboseLvl >= VL_VERBOSE)
                cout << "  ATOMIC PROPOSITION: " << name << endl;
            dta.aps.push_back(AtomicProp(atomicprop_t(dta.aps.size()), name.c_str()));
            atomicPropInds[name] = dta.aps.back().index;
        }
        st.gettok(sep, ispunct);
    }
    while (sep == ",");
    if (sep != "}")
        throw program_exception(st.pos() + ": Expected comma or \"}\".");


    // Read the set of DTA locations
    map<string, location_t> locInds;
    st.readtok("LOCATIONS").readtok("=").readtok("{");
    while (true) {
        // read a qualified name
        FinalLocationType finalType = FLT_NOT_FINAL;
        bool isInit = false;
        string name;
        st.gettok(name, isgraph);
        if (name == "}")
            break; // Read all the locations
        while (true) {
            if (name == "INITIAL")
                isInit = true;
            else if (name == "FINAL" || name == "ACCEPT")
                finalType = FLT_FINAL_ACCEPTING;
            else if (name == "REJECT")
                finalType = FLT_FINAL_REJECTING;
            else
                break; // Read the name of this location

            st.gettok(name, is_ident_ltr);
        }
        if (name.size() != 0) {
            if (locInds.find(name) != locInds.end())
                throw program_exception(st.pos() + ": Location name \"" + name + "\" redeclared.");
            string spexpr;
            st.gettok(spexpr, iscolon).gettok(spexpr, isnotsemicolon).readtok(";");
            shared_ptr<AtomicPropExpr> spExprObj;
            if (spexpr.size() == 0)
                spExprObj = make_shared<AtomicPropExpr_Const>(true);
            else {
                shared_ptr<ParserObj> pObj;
                pObj = ParseObjectExpr(nullptr, &dta, spexpr.c_str(), TOKEN_DTA_STATE_PROPOSITION,
                                       ("State proposition expression: " + st.pos()).c_str(), verboseLvl);
                spExprObj = shared_polymorphic_downcast<AtomicPropExpr>(pObj);
            }
            if (verboseLvl >= VL_VERBOSE) {
                cout << "  LOCATION: " << name << " : ";
                spExprObj->Print(cout, true);
                cout << endl;
            }
            dta.locs.push_back(Location(location_t(dta.locs.size()), isInit, finalType,
                                        name.c_str(), spExprObj));
            locInds[name] = dta.locs.back().index;
        }
    }

    // Read DTA edges
    st.readtok("EDGES").readtok("=").readtok("{");
    while (true) {
        string loc_from, loc_to;
        st.gettok(loc_from, isgraph);
        if (loc_from == "}")
            break;
        if (loc_from.size() != 0) {
            if (locInds.find(loc_from) == locInds.end())
                throw program_exception(st.pos() + ": Source location \"" + loc_from + "\" does not exist.");
            st.readtok("->").gettok(loc_to, is_ident_ltr);
            if (locInds.find(loc_to) == locInds.end())
                throw program_exception(st.pos() + ": Destination location \"" + loc_to + "\" does not exist.");

            string edgedef;
            st.gettok(edgedef, isnotsemicolon).readtok(";");
            shared_ptr<DtaEdge> pEdgeDef;
            shared_ptr<ParserObj> pObj;
            pObj = ParseObjectExpr(nullptr, &dta, edgedef.c_str(), TOKEN_DTA_EDGE_DEF,
                                   ("DTA edge definition: " + st.pos()).c_str(), verboseLvl);
            pEdgeDef = shared_polymorphic_downcast<DtaEdge>(pObj);
            pEdgeDef->destLoc = locInds[loc_to];

            Location &srcLoc = dta.locs[locInds[loc_from]];
            if (pEdgeDef->isBoundary())
                srcLoc.boundEdges.push_back(*pEdgeDef);
            else
                srcLoc.innerEdges.push_back(*pEdgeDef);

            if (verboseLvl >= VL_VERBOSE) {
                cout << "  EDGE: " << loc_from << " -> " << loc_to << " : ";
                if (pEdgeDef->isBoundary())
                    cout << "x = " << dta.clkvals[pEdgeDef->alpha].name << ", #";
                else {
                    cout << dta.clkvals[pEdgeDef->alpha].name << " < x < ";
                    cout << dta.clkvals[pEdgeDef->beta].name << ", ";
                    cout << (pEdgeDef->actset.type == AT_REJECTED_ACTIONS ? "ACT \\ " : "");
                    cout << print_indexed_names(pEdgeDef->actset.actInds, dta.acts);
                }
                cout << (pEdgeDef->reset ? ", RESET" : "") << endl;
            }
        }
    }

    // End of DTA declaration
    st.readtok("}");
}

//=============================================================================

bool BindDTAToPN(DTA &dta, const PN *pPn, const string &rules, VerboseLevel verboseLvl) {
    shared_ptr<DtaBinding> binding;
    do {
        shared_ptr<ParserObj> pObj;
        pObj = ParseObjectExpr(pPn, &dta, rules.c_str(), TOKEN_DTA_BINDING_RULES,
                               "DTA Binding Rules: ", verboseLvl);
        binding = shared_polymorphic_downcast<DtaBinding>(pObj);
    }
    while (false);

    return BindDTA(dta, binding, verboseLvl);
}

//=============================================================================

bool BindDTA(DTA &dta, const shared_ptr<DtaBinding> bindings, VerboseLevel verboseLvl) {
    // STEP 1: Read clock bindings
    // size_t clkval_i = 0;
    for (ClockValue &clkval : dta.clkvals) {
        if (!clkval.isBounded()) {
            for (auto &clk_bind : bindings->clock_values) {
                if (clk_bind.first == clkval.name) {
                    // Assing the named clock value to the unbound clock constant
                    clkval.boundedValue = clk_bind.second;
                    break;
                }
            }
            if (!clkval.isBounded()) {
                ostringstream oss;
                oss << "No binding found for clock value " + clkval.name + ".";
                throw program_exception(oss.str());
            }
            // if (clkval_i >= bindings->clock_values.size())
            // 	throw program_exception("Not enough clock value bindings.");
            // // Assing the parsed clock value to the unbound clock constant
            // clkval.boundedValue = bindings->clock_values[clkval_i++];
        }
        if (verboseLvl >= VL_VERBOSE) {
            cout << "  CLOCK VALUE " << clkval.name;
            if (clkval.name == CLOCK_VALUE_NAME_0 || clkval.name == CLOCK_VALUE_NAME_INFINITY)
                cout << " IS CONSTANT." << endl;
            else {
                cout << " BOUND TO: " << print_petrinetobj(clkval.boundedValue);
                cout << " WITH VALUE: " << clkval.boundedValue->Evaluate() << endl;
            }
        }
    }
    // if (clkval_i != bindings->clock_values.size())
    // 	throw program_exception("Too many clock value bindings.");

    // Verify that the clock contraints are satisfied
    for (const ClockValueConstr &cvc : dta.cvc) {
        double v1 = dta.clkvals[cvc.smaller].boundedValue->Evaluate();
        double v2 = dta.clkvals[cvc.bigger].boundedValue->Evaluate();
        if (!(v1 < v2)) {
            ostringstream reason;
            reason << "Clock constraint: " << dta.clkvals[cvc.smaller].name;
            reason << " < " << dta.clkvals[cvc.bigger].name << " violated (";
            reason << v1 << ", " << v2 << ").";
            throw program_exception(reason.str());
        }
    }

    // Verify that inner clock constraints respect the normal a < x < b order
    for (const Location &loc : dta.locs) {
        for (const DtaEdge &innerEdge : loc.innerEdges) {
            double v1 = innerEdge.alphaValue(dta);
            double v2 = innerEdge.betaValue(dta);
            if (v1 > v2)
                throw program_exception("Clock value binding generates an inner constraint "
                                        + to_string(v1) + " < x < " + to_string(v2));
        }
    }

    // Construct the list of clock zones
    dta.zones.clear();
    std::set<pair<double, clockval_t> > clkconst;
    for (const ClockValue &clkval : dta.clkvals)
        clkconst.insert(make_pair(clkval.boundedValue->Evaluate(), clkval.index));
    std::set<pair<double, clockval_t> >::const_iterator ccit = clkconst.begin();
    while (!std::isinf(ccit->first)) {
        pair<double, clockval_t> val = *ccit++;
        dta.zones.push_back(ClockZone(clockzone_t(dta.zones.size()),
                                      val.second, ccit->second,
                                      val.first, ccit->first));
        if (verboseLvl >= VL_VERBOSE) {
            cout << "  ZONE: " << dta.zones.back().v1 << " - ";
            if (dta.zones.back().isInfinite())
                cout << "INF" << endl;
            else
                cout << dta.zones.back().v2 << endl;
        }
        if (dta.zones.back().duration() <= 0.0)
            throw program_exception("DTA has a clock zone of non-positive duration.");
    }
    dta.hasClockZones = true;

    // Decide if the binding is partial (only clock constants) or complete.
    if (bindings->trn_actions.size() == 0 && bindings-> ap_exprs.size() == 0) {
        if (dta.acts.size() > 0 || dta.aps.size() > 0)
            return false;
    }

    // STEP 2: Read action name bindings
    // size_t trnAct_i = 0;
    for (ActionName &act : dta.acts) {
        bool bound = false;
        for (auto &act_bind : bindings->trn_actions) {
            if (act_bind.first == act.name) {
                const BoundAction &ba = act_bind.second;

                act.boundedTrn = ba.trnIndex;
                if (verboseLvl >= VL_VERBOSE)
                    cout << "  ACTION NAME " << act.name << " BOUND TO TRANSITION "
                         << ba.trnName << " (INDEX: " << size_t(ba.trnIndex) + 1 << ")" << endl;

                bound = true;
                break;
            }
        }
        if (!bound) {
            ostringstream oss;
            oss << "No binding found for action name " + act.name + ".";
            throw program_exception(oss.str());
        }
        // if (trnAct_i >= bindings->trn_actions.size())
        // 	throw program_exception("Not enough action name bindings.");

        // const BoundAction& ba = bindings->trn_actions[trnAct_i++];
        // if (pPn == nullptr)
        // 	throw program_exception("Full DTA binding requires a Petri net object.");
        // if (pPn->trnInds.count(trn_name) == 0)
        // 	throw program_exception("Transition \"" + trn_name + "\" does not exists.");
        // act.boundedTrn = ba.trnIndex;
        // if (verboseLvl >= VL_VERBOSE)
        // 	cout << "  ACTION NAME " << act.name << " BOUND TO TRANSITION "
        // 		 << ba.trnName << " (INDEX: " << size_t(ba.trnIndex)+1 << ")" << endl;
    }
    // if (trnAct_i != bindings->trn_actions.size())
    // 	throw program_exception("Too many action name bindings.");

    // STEP 3: Read state proposition expressions
    // size_t ap_i = 0;
    for (AtomicProp &ap : dta.aps) {
        bool bound = false;
        for (auto &ap_bind : bindings->ap_exprs) {
            if (ap.boundedExpr->get_name() == ap_bind.first) {
                ap.boundedExpr->bind(ap_bind.second);
                if (verboseLvl >= VL_VERBOSE) {
                    cout << "  ATOMIC PROPOSITION " << ap.boundedExpr->get_name() << " BOUND TO EXPRESSION ";
                    cout << print_petrinetobj(ap.boundedExpr) << endl;
                }
                bound = true;
            }
        }
        if (!bound) {
            ostringstream oss;
            oss << "No binding found for atomic proposition " + ap.boundedExpr->get_name() + ".";
            throw program_exception(oss.str());
        }
        // if (ap_i >= bindings->ap_exprs.size())
        // 	throw program_exception("Not enough atomic proposition bindings.");

        // ap.boundedExpr->bind(bindings->ap_exprs[ap_i++]);
        // if (verboseLvl >= VL_VERBOSE) {
        // 	cout << "  ATOMIC PROPOSITION " << ap.boundedExpr->get_name() << " BOUND TO EXPRESSION ";
        // 	cout << print_petrinetobj(ap.boundedExpr) << endl;
        // }
    }
    // if (ap_i != bindings->ap_exprs.size())
    // 	throw program_exception("Too many atomic proposition bindings.");

    dta.isBound = true;
    return true;
}

//=============================================================================

bool IsAtomicPropositionId(const char *name, const DTA *pDTA) {
    for (const AtomicProp &ap : pDTA->aps)
        if (ap.boundedExpr->get_name() == name)
            return true;
    return false;
}

//=============================================================================

shared_ptr<AtomicPropExpr>
FindAtomicProposition(const char *name, const DTA *pDTA) {
    for (const AtomicProp &ap : pDTA->aps)
        if (ap.boundedExpr->get_name() == name)
            return ap.boundedExpr;
    throw program_exception("FindAtomicProposition: Internal error.");
}

//=============================================================================

AtomicProp::AtomicProp(atomicprop_t i, const char *s)
    : index(i), boundedExpr(make_shared<PtrToAtomicPropExpr>(s))
{ }

//=============================================================================



//=============================================================================

bool IsActionId(const char *name, const DTA *pDTA) {
    for (const ActionName &act : pDTA->acts)
        if (act.name == name)
            return true;
    return false;
}

//=============================================================================

actname_t FindActionInd(const char *name, const DTA *pDTA) {
    for (const ActionName &act : pDTA->acts)
        if (act.name == name)
            return act.index;
    throw program_exception("FindActionInd: Internal error.");
}

//=============================================================================

bool IsClockValueId(const char *name, const DTA *pDTA) {
    for (const ClockValue &clkval : pDTA->clkvals)
        if (clkval.name == name)
            return true;
    return false;
}

//=============================================================================

clockval_t FindClockValue(const char *name, const DTA *pDTA) {
    for (const ClockValue &clkval : pDTA->clkvals)
        if (clkval.name == name)
            return clkval.index;
    throw program_exception("FindClockValue: Internal error.");
}

//=============================================================================

// Converts non-HTML characherts to their corresponding XML entities
static string to_html(const string &in) {
    string out(in);
    static const size_t NUM_REPS = 5;
    static const pair<const char *, const char *> subst_list[NUM_REPS] = {
        make_pair("&", "&amp;"), make_pair(">", "&gt;"),
        make_pair("<", "&lt;"), make_pair("\"", "&quot;"),
        make_pair("\'", "&apos;"), //make_pair("#", "&amp;"),
    };
    for (size_t i = 0; i < NUM_REPS; i++) {
        const pair<const char *, const char *> &p = subst_list[i];
        boost::replace_all(out, p.first, p.second);
    }
    return out;
}

// Prints an ostream functor and filters its characters with to_html()
template<class T>
static string functor_to_html(const T &in) {
    ostringstream oss;
    oss << in;
    return to_html(oss.str());
}

// converts a clockval_t into a printable expression
static string to_string(const clockval_t cvind, const DTA &dta) {
    const ClockValue &cv = dta.clkvals[cvind];
    if (cv.boundedValue == nullptr)
        return to_html(cv.name);
    if (std::isinf(cv.boundedValue->Evaluate()))
        return "INF";
    return functor_to_html(print_petrinetobj(cv.boundedValue));
}

// printable expression of an inner bound
static string inner_bound(const clockval_t c1, const clockval_t c2, const DTA &dta) {
    string s1 = to_string(c1, dta), s2 = to_string(c2, dta);
    if (s2 == "INF")  //  x > alpha,  x > 0
        return "x &gt; " + s1;
    if (s1 == "0")    //  x < beta
        return "x &lt; " + s2;
    return s1 + " &lt; x &lt; " + s2;  // alpha < x < beta
}

static string sp_expr_to_html(const shared_ptr<AtomicPropExpr> &spExpr) {
    size_t pos = 0, n1, n2;
    string str = functor_to_html(print_petrinetobj(spExpr));
    for (;;) {
        n1 = str.find("&lt;&lt;", pos);
        if (string::npos != n1) {
            n2 = str.find("&gt;&gt;", n1);
            if (string::npos != n2) {
                str.replace(n2, 8, "</i></font>");
                str.replace(n1, 8, "<font color=\"#6D12B3\"><i>");
                pos = n2;
            }
            else break;
        }
        else break;
    }
    return str;
}

static string
fmt_subscripts(const string &str) {
    istringstream in(str);
    ostringstream out;
    string line;

    while (true) {
        int ch = in.get(), ch2;
        switch (ch) {
        case EOF:
            return out.str();

        case '_':
        case '^':
            out << (ch == '_' ? "<sub>" : "<sup>");
            if ((ch2 = in.get()) == '{') {
                std::getline(in, line, '}');
                out << line;
                in.ignore(1);
            }
            else out << (char)ch2;
            out << (ch == '_' ? "</sub>" : "</sup>");
            break;

        default:
            out << (char)ch;
            break;
        }
    }

    throw program_exception("fmt_subscripts: error.");
}

//=============================================================================

void SaveDTA_AsDotFile(const DTA &dta, ofstream &dot) {
    dot << "digraph RRG {\n  rankdir=TB;\n  ratio=compress;\n";
    dot << "  edge[len=2.5, fontsize=10, fontname=Arial];\n";

    // Write an entering edge to each initial location
    for (const auto &loc : dta.locs) {
        if (loc.isInitial)
            dot << "  init -> loc" << loc.index << " [len=0.5, color=navy];\n";
    }
    dot <<  "\n";

    // Write locations
    dot << "  node[shape=ellipse, fontname=Arial];\n  init[shape=none, label=<>];\n";
    for (const auto &loc : dta.locs) {
        dot << "  loc" << loc.index << " [ label=<";

        dot << "<font face=\"Times\" color=\"#083C9E\">" << fmt_subscripts(to_html(loc.label)) << "</font> - ";
        dot << "<font point-size=\"10\">index: " << loc.index << "</font><br/>";
        dot << "<font point-size=\"8\"> </font><br/>";
        dot << sp_expr_to_html(loc.spExpr);
        dot << (loc.finalType == FLT_FINAL_REJECTING ? "<br/><font color=\"#9E0808\">REJECT</font>" : "");
        dot << (loc.finalType == FLT_FINAL_ACCEPTING ? "<br/><font color=\"#089E0A\">ACCEPT</font>" : "");

        dot << ">, " << (loc.isFinal() ? "peripheries=2, " : "");
        dot << (dta.isZoned && loc.zoneType != CZT_TIMED ? "style=dashed, " : "");
        dot << "penwidth=1.5, fontname=Arial ];\n";
    }
    dot <<  "\n";

    // Write edges
    for (const auto &loc : dta.locs) {
        for (size_t k = 0; k < 2; k++) {
            const std::vector<DtaEdge> &edges = (k == 0 ? loc.innerEdges : loc.boundEdges);
            for (const auto &edge : edges) {
                dot << "  loc" << loc.index << " -> loc" << edge.destLoc << " [ label=<";
                if (edge.isBoundary())
                    dot << "#";
                else {
                    if (edge.actset.type == AT_REJECTED_ACTIONS)
                        dot << "Act" << (edge.actset.actInds.empty() ? "" : " &#47; ");
                    if (!edge.actset.actInds.empty())
                        dot << print_indexed_names(edge.actset.actInds, dta.acts);
                }
                dot << "<br/>";
                dot << "<font color=\"#6060A0\">";
                if (edge.isBoundary())
                    dot << "x = " << to_string(edge.alpha, dta);
                else
                    dot << inner_bound(edge.alpha, edge.beta, dta);
                dot << "</font>";
                if (edge.reset)
                    dot << "<br/><font color=\"#8C1212\">RESET</font>";
                dot << ">, " << (edge.isBoundary() ? "style=dashed, " : "");
                dot << "penwidth=1 ];\n";
            }
        }
    }

    dot <<  "\n\n}\n\n" << flush;
}

//=============================================================================

typedef std::tuple<location_t, clockzone_t, ClockZoneType> lcz_tuple_t;

// inline static location_t
// GetOrInsertZonedLocation(DTA& zdta, const Location& oldLoc, clockzone_t cz,
// 						 ClockZoneType zoneType, bool isInitial,
// 						 std::map<lcz_tuple_t, location_t>& lczToInd,
// 						 std::queue<location_t>& location_queue,
// 						 VerboseLevel verboseLvl)
// {
// 	lcz_tuple_t lcz_key(oldLoc.index, cz, zoneType);
// 	auto lczIndIt = lczToInd.find(lcz_key);
// 	if (lczIndIt != lczToInd.end()) {
// 		// The location already exists
// 		return lczIndIt->second;
// 	}

// 	// Insert a new location
// 	ostringstream nlabel;
// 	nlabel << oldLoc.label << " * ";
// 	switch (zoneType) {
// 		case CZT_TIMED:
// 			nlabel << "(" << zdta.zones[cz].v1 << ",";
// 			if (std::isinf(zdta.zones[cz].v2))
// 				nlabel << "INF)";
// 			else
// 				nlabel << zdta.zones[cz].v2 << ")";
// 			break;
// 		case CZT_ENTER_IMMED:
// 			nlabel << "[" << zdta.zones[cz].v1 << "]";
// 			break;
// 		case CZT_EXIT_IMMED:
// 			nlabel << "[" << zdta.zones[cz].v1 << ", EXIT]";
// 			break;
// 		default:
// 			throw program_exception("InsertZonedLocation: internal error.");
// 	}
// 	Location loc(location_t(zdta.locs.size()), isInitial, oldLoc.finalType,
// 				 nlabel.str().c_str(), oldLoc.spExpr);
// 	loc.prevIndex = oldLoc.index;
// 	loc.zone = cz;
// 	loc.zoneType = zoneType;
// 	zdta.locs.push_back(loc);
// 	lczToInd[lcz_key] = loc.index;
// 	location_queue.push(loc.index);

// 	if (verboseLvl >= VL_VERBOSE)
// 		cout << "  ZONED LOCATION Z" << size_t(loc.index)+1 << ": " << loc.label << endl;

// 	return loc.index;
// }

//=============================================================================

void MakeZonedDTA(const DTA &dta, DTA &zdta, VerboseLevel verboseLvl) {
    if (!dta.hasClockZones)
        throw program_exception("The construction of the zone graph requires the DTA clock constants to be bound.");
    if (dta.isZoned)
        throw program_exception("DTA is already zoned.");

    if (verboseLvl >= VL_VERBOSE)
        cout << "PARTITIONING DTA LOCATIONS INTO ZONES.\n" << endl;

    // Copy all the fields except the location/edges
    zdta.name    = "Z(" + dta.name + ")";
    zdta.aps     = dta.aps;
    zdta.acts    = dta.acts;
    zdta.clkvals = dta.clkvals;
    zdta.cvc     = dta.cvc;
    zdta.zones   = dta.zones;
    zdta.isBound = dta.isBound;
    zdta.hasClockZones = dta.hasClockZones;
    zdta.isZoned = true;

    std::map<lcz_tuple_t, location_t> lczToInd;  // location_t index from a <l,cz> tuple.
    std::queue<location_t> location_queue;		 // Queue of unexplored locations.

    // Helper lambda function that inserts a new location if it does not already exist.
    auto GetOrInsertZonedLocation = [&](const Location & oldLoc, clockzone_t cz,
    ClockZoneType zoneType, bool isInitial) -> location_t {
        lcz_tuple_t lcz_key(oldLoc.index, cz, zoneType);
        auto lczIndIt = lczToInd.find(lcz_key);
        if (lczIndIt != lczToInd.end()) {
            // The location already exists
            return lczIndIt->second;
        }

        // Insert a new location
        ostringstream nlabel;
        nlabel << oldLoc.label << " * ";
        switch (zoneType) {
        case CZT_TIMED:
            nlabel << "(" << zdta.zones[cz].v1 << ",";
            if (std::isinf(zdta.zones[cz].v2))
                nlabel << "INF)";
            else
                nlabel << zdta.zones[cz].v2 << ")";
            break;
        case CZT_ENTER_IMMED:
            nlabel << "[" << zdta.zones[cz].v1 << "]";
            break;
        case CZT_EXIT_IMMED:
            nlabel << "[" << zdta.zones[cz].v1 << ", EXIT]";
            break;
        default:
            throw program_exception("InsertZonedLocation: internal error.");
        }
        Location loc(location_t(zdta.locs.size()), isInitial, oldLoc.finalType,
        nlabel.str().c_str(), oldLoc.spExpr);
        loc.prevIndex = oldLoc.index;
        loc.zone = cz;
        loc.zoneType = zoneType;
        zdta.locs.push_back(loc);
        lczToInd[lcz_key] = loc.index;
        location_queue.push(loc.index);

        if (verboseLvl >= VL_VERBOSE)
            cout << "  ZONED LOCATION Z" << size_t(loc.index) + 1 << ": " << loc.label << endl;

        return loc.index;
    };

    // Start from all the initial location in zone [0, enter]
    for (const Location &loc : dta.locs) {
        if (loc.isInitial)
            GetOrInsertZonedLocation(loc, zdta.c0(), CZT_ENTER_IMMED, true);
    }

    // Explore all the non-final zoned locations in the location_queue
    while (!location_queue.empty()) {
        location_t srcLocInd = location_queue.front();
        location_queue.pop();

        // CANNOT STORE THIS REFERENCE DUE TO RESIZEs OF zdta.locs[] VECTOR.
        auto srcZLoc = [&]() -> Location& { return zdta.locs[srcLocInd]; };
        const Location &srcLoc = dta.locs[srcZLoc().prevIndex];
        const ClockZone &srcCZ = zdta.zones[srcZLoc().zone];
        //cout << "visiting " << srcZLoc().label << " ..." << endl;

        if (srcLoc.isFinal())
            continue;

        if (srcZLoc().zoneType == CZT_TIMED) {
            // Timed zoned location, explore innerEdges + boundary [ x=beta ]

            // Timed edges
            for (const DtaEdge &innerEdge : srcLoc.innerEdges) {
                // Check if the edge is enabled in this clock zone
                if (!(innerEdge.alphaValue(dta) <= srcCZ.v1 && srcCZ.v2 <= innerEdge.betaValue(dta)))
                    continue;

                const Location &dstLoc = dta.locs[innerEdge.destLoc];
                const clockzone_t dstZone = (innerEdge.reset ? zdta.c0() : srcCZ.index);
                const ClockZoneType dstZoneType = (innerEdge.reset ? CZT_ENTER_IMMED : CZT_TIMED);
                location_t dstLocInd = GetOrInsertZonedLocation(dstLoc, dstZone, dstZoneType, false);
                DtaEdge ne(innerEdge.actset, innerEdge.reset, srcCZ.cv1, srcCZ.cv2, dstLocInd);
                srcZLoc().innerEdges.push_back(ne);
            }

            // Boundary edges for time elapse
            if (srcCZ.index == zdta.cInf())
                continue; // No boundary edges on x=INF

            const clockzone_t cNext = srcCZ.index + clockzone_t(1);
            location_t elapseLocInd = GetOrInsertZonedLocation(srcLoc, cNext, CZT_ENTER_IMMED, false);
            DtaEdge ne(ActSet(AT_BOUNDARY), false, srcCZ.cv2, srcCZ.cv2, elapseLocInd);
            srcZLoc().boundEdges.push_back(ne);
        }
        else {
            // Immediate zone, explore boundary [ x=alpha ]
            std::vector< std::reference_wrapper<const DtaEdge >> enabledBoundEdges;
            for (const DtaEdge &boundEdge : srcLoc.boundEdges) {
                if (boundEdge.betaValue(dta) ==  srcCZ.v1) // left boundary
                    enabledBoundEdges.push_back(boundEdge);
            }
            bool hasEnterExit = (enabledBoundEdges.size() > 0);
            bool isEnter = (srcZLoc().zoneType == CZT_ENTER_IMMED);

            if (isEnter) {
                // Boundary edges are followed only in the ENTER immediate location
                for (size_t ebe = 0; ebe < enabledBoundEdges.size(); ebe++) {
                    const DtaEdge &boundEdge = enabledBoundEdges[ebe];
                    const Location &dstLoc = dta.locs[boundEdge.destLoc];
                    const clockzone_t dstZone = (boundEdge.reset ? zdta.c0() : srcCZ.index);
                    //const ClockZoneType dstZoneType = (hasEnterExit && isEnter) ? CZT_EXIT_IMMED : CZT_TIMED;
                    location_t dstLocInd;
                    dstLocInd = GetOrInsertZonedLocation(dstLoc, dstZone, CZT_ENTER_IMMED, false);
                    DtaEdge ne(boundEdge.actset, boundEdge.reset, srcCZ.cv1, srcCZ.cv1, dstLocInd);
                    srcZLoc().boundEdges.push_back(ne);
                }
            }
            if (hasEnterExit && isEnter) {
                // Add the catch-all location (fall-through when no boundary edge my fire),
                // and go directly to it if all the other enabled bondary edges do not fire
                shared_ptr<AtomicPropExpr>  spexprdef = srcLoc.spExpr;
                for (size_t ebe = 0; ebe < enabledBoundEdges.size(); ebe++)
                    spexprdef = make_shared<AtomicPropExpr_BOP>
                                (spexprdef, make_shared<AtomicPropExpr_NOT>
                                 (dta.locs[enabledBoundEdges[ebe].get().destLoc].spExpr), BBF_AND);

                location_t defLocInd = GetOrInsertZonedLocation(srcLoc, srcCZ.index, CZT_EXIT_IMMED, false);
                zdta.locs[defLocInd].spExpr = spexprdef;
                DtaEdge ne(ActSet(AT_BOUNDARY), false, srcCZ.cv1, srcCZ.cv1, defLocInd);
                srcZLoc().boundEdges.push_back(ne);
            }
            else {
                // No bondary edges, exit to the timed clock zone
                location_t dstLocInd = GetOrInsertZonedLocation(srcLoc, srcCZ.index, CZT_TIMED, false);
                DtaEdge ne(ActSet(AT_BOUNDARY), false, srcCZ.cv1, srcCZ.cv1, dstLocInd);
                srcZLoc().boundEdges.push_back(ne);
            }
        }
    }

    if (verboseLvl >= VL_VERBOSE)
        cout << "\nMARKING LOCATIONS THAT CANNOT REACH TOP AS REJECTING ..." << endl;

    // Optimize the Zoned DTA
    // Make rejecting all the locations that cannot reach a top location
    vector<bool> mayAccept(zdta.locs.size());
    for (location_t l(0); l < location_t(zdta.locs.size()); l++)
        mayAccept[l] = zdta.locs[l].finalType == FLT_FINAL_ACCEPTING;

    bool isFixPoint = false;
    while (!isFixPoint) {
        isFixPoint = true;
        for (location_t l(0); l < location_t(zdta.locs.size()); l++) {
            if (mayAccept[l])
                continue;
            if (zdta.locs[l].finalType == FLT_FINAL_REJECTING)
                continue;
            // Check inner edges
            for (size_t e = 0; e < zdta.locs[l].innerEdges.size(); e++)
                if (mayAccept[ zdta.locs[l].innerEdges[e].destLoc ]) {
                    mayAccept[l] = true;
                    isFixPoint = false;
                }
            // Check boundary edges
            for (size_t e = 0; e < zdta.locs[l].boundEdges.size(); e++)
                if (mayAccept[ zdta.locs[l].boundEdges[e].destLoc ]) {
                    mayAccept[l] = true;
                    isFixPoint = false;
                }
        }
    }
    // Make non-accepting locations final/rejecting.
    for (location_t l(0); l < location_t(zdta.locs.size()); l++)
        if (!mayAccept[l]) {
            zdta.locs[l].finalType = FLT_FINAL_REJECTING;
            //zdta.locs[l].innerEdges.clear();
            //zdta.locs[l].boundEdges.clear();
            if (verboseLvl >= VL_VERBOSE)
                cout << "  ZONED LOCATION Z" << size_t(l) + 1 << " IS MARKED AS FINAL REJECTING." << endl;
        }

    if (verboseLvl >= VL_BASIC) {
        size_t numTimedZdtaLocs = std::count_if(zdta.locs.begin(), zdta.locs.end(), 
                                                [](const Location& l){ return l.isTimed(); });
        cout << "DTA PARTITIONED INTO ZONES (timed locations = "<<numTimedZdtaLocs<<")." << endl;
    }
}

//=============================================================================

void DtaBinding::Print(ostream &os, bool expandParamNames) const {
    for (size_t i = 0; i < clock_values.size(); i++) {
        os << clock_values[i].first << " = ";
        clock_values[i].second->Print(os, expandParamNames);
        os << (i == clock_values.size() - 1 ? " " : ", ");
    }
    os << "| ";
    for (size_t i = 0; i < trn_actions.size(); i++) {
        os << trn_actions[i].first << " = ";
        os << trn_actions[i].second.trnName;
        os << (i == trn_actions.size() - 1 ? " " : ", ");
    }
    os << "| ";
    for (size_t i = 0; i < ap_exprs.size(); i++) {
        os << ap_exprs[i].first << " = ";
        ap_exprs[i].second->Print(os, expandParamNames);
        os << (i == ap_exprs.size() - 1 ? "" : ", ");
    }
}

//=============================================================================















//=============================================================================

SAT::~SAT() { }

//=============================================================================

SAT SAT::operator !() const {
    SAT res(N(), state);
    std::transform(sat.begin(), sat.end(), res.sat.begin(), logical_not<bool>());
    return res;
}

//=============================================================================

SAT SAT::operator && (const SAT &s) const {
    assert(N() == s.N());
    assert(state == s.state);
    assert(!state || N() == 1);
    SAT res(N(), state);
    for (size_t i = 0; i < N(); i++)
        res.sat[i] = (sat[i] && s.sat[i]);
    return res;
}

//=============================================================================

SAT SAT::operator || (const SAT &s) const {
    assert(N() == s.N());
    assert(state == s.state);
    assert(!state || N() == 1);
    SAT res(N(), state);
    for (size_t i = 0; i < N(); i++)
        res.sat[i] = (sat[i] || s.sat[i]);
    return res;
}

//=============================================================================

void SAT::Print(ostream &os, bool expandParamNames) const {
    os << "SAT";
    if (state)
        os << "(state=" << size_t(*state) + 1 << "): ";
    else
        os << "(CTMC): ";
    os << print_vec(sat);
}

//=============================================================================

void SAT::WriteToFile(ofstream &ofs) const {
    if (state)
        ofs << *state << " ";
    else
        ofs << "ALL ";
    ofs << (probs.empty() ? " SAT " : "SAT+PROBS ") << N() << "\n";

    ofs << print_vec(sat) << "\n";
    if (!probs.empty())
        PrintUblasVector(ofs, probs);
}

//=============================================================================

bool SAT::ReadFromFile(ifstream &ifs) {
    string s, p, n;
    simple_tokenizer st(&ifs);
    st.gettok(s).gettok(p).gettok(n);
    if (s == "ALL")
        state.reset();
    else
        state.reset(stoi(s));

    size_t Num = stoi(n);
    sat.resize(Num);
    st.readtok("{");
    for (size_t i = 0; i < Num; i++) {
        int v;
        st.read(v);
        sat[i] = v;
        if (i != Num - 1)
            st.readtok(",");
    }
    st.readtok("}");

    if (p == "SAT+PROBS") {
        ifs >> probs;
    }
    return ifs.good();
}

//=============================================================================

bool CompareSATs(const SAT &s1, const SAT &s2, double epsilon) {
    if (s1.state != s2.state)
        return false;
    if (s1.N() != s2.N())
        return false;
    if (s1.probs.size() != s2.probs.size())
        return false;
    if (s1.sat != s2.sat)
        return false;

    for (size_t i = 0; i < s1.probs.size(); i++)
        if (std::abs(s1.probs[i] - s2.probs[i]) > epsilon)
            return false;

    return true;
}

//=============================================================================












//=============================================================================

void AtomicPropExpr::StoreEvaluation(const RG2 &rg, const SolverParams &spar,
                                     boost::optional<marking_t> state,
                                     VerboseLevel verboseLvl) {
    if (!isStored) {
        storedSat = Evaluate(rg, spar, state, verboseLvl);
        isStored = true;
    }
}

//=============================================================================



// //=============================================================================

// SAT CslTaExpr_STEADY::EvaluateSat(const RG2& rg, const SolverParams& spar,
// 								  VerboseLevel verboseLvl) const
// {
// 	/*SAT sat = expr->EvaluateSat(rg, spar, verboseLvl);

// 	// Compute the steady state probabilities of the CTMC of rg2
// 	CTMC ctmc;
// 	ExtractCTMC(rg, ctmc, verboseLvl);

// 	PetriNetSolution pnsol;
// 	SolveSteadyStateCTMC(ctmc, pnsol, KED_FORWARD, nullptr, spar, verboseLvl);*/
// 	throw program_exception("STEADY operator is unimplemented.");
// }
// void CslTaExpr_STEADY::Print(ostream& os, bool expandParamNames) const {
// 	os << "STEADY " << pc << " (";
// 	expr->Print(os, expandParamNames);
// 	os << ")";
// }

// //=============================================================================

// SAT CslTaExpr_PROB_TA::EvaluateSat(const RG2& rg, const SolverParams& spar,
// 								   VerboseLevel verboseLvl) const
// {
// 	// First, load the DTA from the file
// 	ifstream ifsDta;
// 	for (const string& pathDir : spar.dtaSearchPaths) {
// 		string path = pathDir + "/" + dtaName;
// 		ifsDta.open(path.c_str());
// 		if (ifsDta.is_open())
// 			break;
// 		path += ".dta";
// 		ifsDta.open(path.c_str());
// 		if (ifsDta.is_open())
// 			break;
// 	}
// 	if (!ifsDta.is_open()) {
// 		ostringstream reason;
// 		reason << "Could not open a DTA file \"" << dtaName << "\" in the DTA search path.";
// 		throw program_exception(reason.str());
// 	}

// 	DTA dta;
// 	LoadDTAFromFile(ifsDta, dta, verboseLvl);

// 	// Bind the DTA using the binding rules.
// }
// void CslTaExpr_PROB_TA::Print(ostream& os, bool expandParamNames) const {
// 	os << "PROB_TA " << pc << " (" << dtaName << ": " << bindingRules << ")";
// }

// //=============================================================================





































