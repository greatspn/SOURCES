/*
 *  PetriNet.cpp
 *
 *  Basic methods for Petri Net management.
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
#include <cstring>
#include <climits>
#include <cfloat>
using namespace std;

#include <boost/algorithm/string/replace.hpp> // replace_all

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "ParserDefs.h"
#include "PetriNet.h"
#include "Measure.h"
#include "Language.h"
#include "newparser.lyy.h"
// #include "../alphaFactory/alphaFactory.h"

//=============================================================================

const char *ArcKind_names[] = { "IA", "OA", "HA" };
const char *TrnKind_names[] = { "TIMED", "IMMED" };
const char *TrnDistrib_names[] = { "IMM", "EXP", "GEN" };
const char *TrnDistrib_names_lowercase[] = { "imm", "exp", "gen" };


//=============================================================================
//    Function implementations
//=============================================================================

shared_ptr<ParserObj>
ParseObjectExpr(const PN *pn, const DTA *dta, const char *exprStr,
                int initialToken, const char *whatIs, VerboseLevel verboseLvl) {
    // Prepare the parser
    Token token;
    ExtraParserArgs parserArgs(pn, dta);
    PnParserRef parser;
    static const char *tracePrefix = " * ";
    if (verboseLvl >= VL_VERY_VERBOSE) {
        cout << "  ParseObjectExpr: \"" << exprStr << "\", whatIs: " << whatIs << endl;
        parser.Trace(stderr, const_cast<char *>(tracePrefix));
    }

    // Prepare the lexer
    stringstream stream(exprStr);
    STRINGSTREAM inLexerStream(&stream, false);
    ExtraLexerArgs lexerArgs(pn, dta, initialToken);
    PnLexerRef lexer(&inLexerStream, &lexerArgs);

    // First, pass the initialToken to the parser, to orient the starting rule
    parser.Parse(initialToken, &token, &parserArgs);

    int reachedEnd = false;
    while (parserArgs.status == PS_RUNNING && !reachedEnd) {
        // Now feed the parser with one token at a time
        int tokenId = lexer.GetNextToken(&token);
        if (verboseLvl >= VL_VERY_VERBOSE)
            cerr << "\n # tokenId = " << tokenId << endl;
        parser.Parse(tokenId, &token, &parserArgs);
        reachedEnd = (tokenId <= 0) || (tokenId == TOKEN_END);
    }
    if (verboseLvl >= VL_VERY_VERBOSE)
        cerr << "\n # Pushing token 0 to end." << endl;
    parser.Parse(0, &token, &parserArgs);

    if (parserArgs.status == PS_FAILED || !reachedEnd) {
        ostringstream msg;
        msg << "Syntax error for " << whatIs << ": \"" << exprStr << "\".";
        throw program_exception(msg.str());
    }
    else if (parserArgs.status == PS_RUNNING || parserArgs.pOutObj == nullptr) {
        ostringstream msg;
        msg << "Cannot parse the incomplete " << whatIs << ": \"" << exprStr << "\".";
        throw program_exception(msg.str());
    }

    assert(parserArgs.pOutObj != nullptr);
    return parserArgs.pOutObj;
}

//=============================================================================

shared_ptr<MdepExpr<double> >
ParseMarkDepDelayExpr(const PN &pn, const char *exprStr, VerboseLevel verboseLvl) {
    shared_ptr<ParserObj> pObj;
    pObj = ParseObjectExpr(&pn, nullptr, exprStr, TOKEN_GREATSPN_MDEPDELAY,
                           "Marking dependent Delay", verboseLvl);
    return shared_polymorphic_downcast<MdepExpr<double> >(pObj);
}

//=============================================================================

shared_ptr<MdepExpr<int> >
ParseMarkDepMultExpr(const PN &pn, const char *exprStr, VerboseLevel verboseLvl) {
    shared_ptr<ParserObj> pObj;
    pObj = ParseObjectExpr(&pn, nullptr, exprStr, TOKEN_GREATSPN_MDEARCMULT,
                           "Marking dependent arc multiplicity", verboseLvl);
    return shared_polymorphic_downcast<MdepExpr<int> >(pObj);
}

//=============================================================================

shared_ptr<MdepExpr<bool> >
ParseTransitionGuardExpr(const PN &pn, const char *exprStr, VerboseLevel verboseLvl) {
    shared_ptr<ParserObj> pObj;
    pObj = ParseObjectExpr(&pn, nullptr, exprStr, TOKEN_GREATSPN_TRNGUARD,
                           "Transition guard", verboseLvl);
    return shared_polymorphic_downcast<MdepExpr<bool> >(pObj);
}

//=============================================================================

shared_ptr<MeasureExpr>
ParseMeasureExpr(const PN &pn, const char *exprStr, VerboseLevel verboseLvl) {
    shared_ptr<ParserObj> pObj;
    pObj = ParseObjectExpr(&pn, nullptr, exprStr, TOKEN_GREATSPN_MEASURE,
                           "Measure expression", verboseLvl);
    return shared_polymorphic_downcast<MeasureExpr>(pObj);
}

//=============================================================================

shared_ptr<Expr<double> >
ParseGeneralPdfExpr(const PN &pn, const char *exprStr, VerboseLevel verboseLvl) {
    shared_ptr<ParserObj> pObj;
    pObj = ParseObjectExpr(&pn, nullptr, exprStr, TOKEN_GREATSPN_GENERALPDF,
                           "Marking dependent Delay", verboseLvl);
    return shared_polymorphic_downcast<Expr<double>>(pObj);
}

//=============================================================================

static inline
shared_ptr<Expr<int> > GetIntOrMarkPar(const PN &pn, int v) {
    if (v >= 0)
        return make_shared<ConstValExpr<int> >(v);
    size_t mparInd = size_t(-1 - v);
    if (mparInd >= pn.mpars.size())
        throw program_exception("Marking parameter index is out of range.");
    return pn.mpars[mparInd].param;
}

static inline
shared_ptr<Expr<double> > GetRealOrRatePar(const PN &pn, double v) {
    if (v >= 0)
        return make_shared<ConstValExpr<double> >(v);
    size_t rparInd = size_t(-1 - v);
    if (rparInd >= pn.rpars.size())
        throw program_exception("Rate parameter index is out of range.");
    return pn.rpars[rparInd].param;
}

static inline
shared_ptr<Expr<double> > DelayObject(const PN &pn, double delay) {
    if (delay >= 0.0)
        return make_shared<ConstValExpr<double> >(delay);;
    return pn.rpars[size_t(-1 - delay)].param;
}

static inline string read_mdep_fn_from_def(simple_tokenizer& def, size_t trn_index) {
    // Read a marking dependent function from the def file
    size_t trnIndex;
    def.readtok("|").read(trnIndex);
    if (trnIndex != trn_index + 1)
        throw program_exception("Wrong transition index in .def file");
    def.skiplines(1);
    string fn;
    def.getline(fn);
    return fn;
};

//=============================================================================

void ReadGreatSPN_File(ifstream &ifsNet, ifstream &ifsDef, PN &pn, 
                       bool readAsPT, VerboseLevel verboseLvl) 
{
    simple_tokenizer net(&ifsNet), def(&ifsDef);
    string line, textBuf;

    // Skip the initial header
    int csetNum, layerNum;
    net.skiplines(1);
    do {
        net.getline(line);
    }
    while (net && (line.empty() || line[0] != '|'));

    // Header with object counters
    size_t  numMarkPars, numPlcs, numRatePars, numTrns, numGrps;
    net.readtok("f");
    net.read(numMarkPars).read(numPlcs).read(numRatePars);
    net.read(numTrns).read(numGrps).read(csetNum).read(layerNum); //readtok("0")).read(readtok("0");

    if (verboseLvl >= VL_VERBOSE) {
        cout << "numMarkPars = " << numMarkPars << endl;
        cout << "numPlcs     = " << numPlcs << endl;
        cout << "numRatePars = " << numRatePars << endl;
        cout << "numTrns     = " << numTrns << endl;
        cout << "numGrps     = " << numGrps << endl;
    }

    // Read Marking Parameters
    pn.mpars.resize(numMarkPars);
    for (size_t i = 0; i < numMarkPars; i++) {
        MarkPar &mpar = pn.mpars[i];
        int mpVal;
        string mpName;
        net.read(mpName).read(mpVal);
        mpar.index = markpar_t(i);
        mpar.param = make_shared<ParamValExpr<int> >(mpName.c_str(),
                     make_shared<ConstValExpr<int> >(mpVal));
        pn.mparInds[mpar.param->GetName()] = mpar.index;
        net.skiplines(1);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "MARKPAR " << mpar.index << ": " << mpar.param->GetName();
            cout << " = " << print_petrinetobj(mpar.param) << endl;
        }
    }

    // Read Places
    pn.plcs.resize(numPlcs);
    for (size_t i = 0; i < numPlcs; i++) {
        Place &p = pn.plcs[i];
        int initMarkVal;
        net.read(p.name).read(initMarkVal);
        p.index = place_t(i);
        p.initMarkFn = GetIntOrMarkPar(pn, initMarkVal);
        pn.plcInds[p.name] = p.index;
        net.skiplines(1);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "PLACE " << p.index << ": " << p.name << ", initMark=";
            p.initMarkFn->Print(cout, false);
            cout << endl;
        }
    }

    // Read Rate Parameters
    pn.rpars.resize(numRatePars);
    for (size_t i = 0; i < numRatePars; i++) {
        RatePar &rpar = pn.rpars[i];
        double rpVal;
        string rpName;
        net.read(rpName).read(rpVal);
        rpar.index = ratepar_t(i);
        rpar.param = make_shared<ParamValExpr<double> >(rpName.c_str(),
                     make_shared<ConstValExpr<double> >(rpVal));
        pn.rparInds[rpar.param->GetName()] = rpar.index;
        net.skiplines(1);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "RATEPAR " << rpar.index << ": " << rpar.param->GetName();
            cout << " = " << print_petrinetobj(rpar.param) << endl;
        }
    }

    // Read Groups
    vector<priority_t> prioGroups(numGrps);
    for (size_t i = 0; i < numGrps; i++) {
        string gname;
        double skipDbl;
        net.read(gname).read(skipDbl).read(skipDbl).read(prioGroups[i]);
        if (verboseLvl >= VL_VERBOSE)
            cout << "GROUP " << i << ": " << gname << ", priority=" << prioGroups[i] << endl;
    }

    // Transitions
    pn.trns.resize(numTrns);
    for (size_t i = 0; i < numTrns; i++) {
        Transition &trn = pn.trns[i];
        int enabDegree, trnPrGroup;
        string delayText;
        bool loadDependent = false, delayExpr = false;
        size_t orientation, numArcs;
        net.read(trn.name).read(delayText).read(enabDegree).read(trnPrGroup).read(numArcs).read(orientation);
        // Read transition guard (if any)
        net.getline(textBuf);
        // Read delay expression (if requested)
        if (delayText == "*") {
            net.getline(delayText);
            delayExpr = true;
        }

        if (enabDegree == 1 && trnPrGroup > 0) {
            trn.distrib = IMM;
            trn.prio = prioGroups[trnPrGroup - 1];

            double delay = -1;
            if (!delayExpr) {
                delay = atof(delayText.c_str());
                if (delay == -5.100000e+02) {
                    // Read the marking-dependent function from the .def file
                    delayText = read_mdep_fn_from_def(def, i);
                    delayExpr = true;
                }
            }
            if (readAsPT && delayExpr)
                delayText = "1.0";
            if (delayExpr) {
                trn.delayFn = NewMultipleServerDelayFn(1, ParseMarkDepDelayExpr(pn, delayText.c_str(), verboseLvl));
            }
            else {
                trn.delayFn = NewMultipleServerDelayFn(1, DelayObject(pn, delay));
            }
        }
        else if (enabDegree == 0 && trnPrGroup == 127) {
            if (!delayExpr) { // It is a deterministic event with duration = delayText
                double duration = atof(delayText.c_str());
                if (duration == -5.100000e+02) {
                    // Read the marking-dependent function from the .def file
                    delayText = read_mdep_fn_from_def(def, i);
                }
                else { // constant duration
                    ostringstream oss;
                    oss << "I[";
                    GetRealOrRatePar(pn, duration)->Print(oss, false);
                    oss << "]";
                    delayText = oss.str();
                }
            }
            if (readAsPT)
                delayText = "I[1.0]";
            // double delay = atof(delayText.c_str());
            trn.distrib = DET;
            trn.prio = 0;
            pn.detTrnInds.push_back(i);
            trn.delayFn = NewGeneralPdfFn(ParseGeneralPdfExpr(pn, delayText.c_str(), verboseLvl), delayText.c_str());
            if (trn.delayFn->IsMarkingDep()) {
                throw program_exception("Marking dependent general functions are not supported.");
            }
            string fgx = trn.generalPdf();
            // const char *msg = verify_alpha_factors_expr(fgx.c_str());
            // if (msg != nullptr) {
            //     cerr << "parsed as: " << fgx << endl;
            //     ostringstream msg2;
            //     msg2 << "Function: " << delayText << " of transition " << trn.name << " is not valid.\n" << msg;
            //     throw program_exception(msg2.str());
            // }
        }
        else if (trnPrGroup == 0) {
            double delay = atof(delayText.c_str());
            trn.distrib = EXP;
            trn.prio = 0;
            if (enabDegree == 0)
                trn.delayFn = NewMultipleServerDelayFn(numeric_limits<size_t>::max(),
                                                       DelayObject(pn, delay));
            else if (enabDegree > 0) {
                if (delay == -5.100000e+02) {
                    // Read the marking dependent function from the def file
                    string mdepfn = read_mdep_fn_from_def(def, i);
                    if (readAsPT)
                        mdepfn = "1.0";
                    shared_ptr<MdepExpr<double> > mdepFn =
                        ParseMarkDepDelayExpr(pn, mdepfn.c_str(), verboseLvl);
                    trn.delayFn = NewMarkingDependentDelayFn(mdepFn);
                }
                else
                    trn.delayFn = NewMultipleServerDelayFn(enabDegree,
                                                           DelayObject(pn, delay));
            }
            else { // enabDegree < 0
                loadDependent = true;
            }
        }
        else
            throw program_exception("Unrecognized Transition type.\n");
        trn.index = transition_t(i);
        pn.trnInds[trn.name] = trn.index;
        
        if (textBuf.size() > 0) {
            istringstream guardFnExpr(textBuf);
            double trnx, trny, tagx, tagy, ratex, ratey, guardx, guardy;
            int nLayers, ll;
            guardFnExpr >> trnx >> trny >> tagx >> tagy >> ratex >> ratey >> nLayers;
            for (int l = nLayers; l > 0; l--) {
                guardFnExpr >> ll;
            }
            guardFnExpr >> guardx >> guardy;
            /*double cx;
            for (size_t k=0; k<9; k++)
            	 guardFnExpr >> cx;*/
            if (!guardFnExpr.eof()) {
                string expr;
                getline(guardFnExpr, expr);
                trn.guardFn = ParseTransitionGuardExpr(pn, expr.c_str(), verboseLvl);
            }
        }
        // Read Load dependent weights (if any)
        if (loadDependent) {
            vector<double> delays(size_t(-enabDegree));
            delays[0] = atof(delayText.c_str());
            for (size_t n = 1; n < delays.size(); n++) {
                net.read(delays[n]);
                if (delays[n] <= 0)
                    throw program_exception("Negative load-dependent delays");
                net.skiplines(1);
            }
            trn.delayFn = NewLoadDependentDelayFn(delays);
        }
        if (verboseLvl >= VL_VERBOSE) {
            cout << "TRANSITION " << trn.index << ": " << trn.name << ", ";
            cout << TrnDistrib_names[trn.distrib] << ", ";
            cout << (trn.isTimed() ? "delay=" : "weight=");
            trn.delayFn->Print(cout, false);
            if (trn.guardFn != nullptr) {
                cout << " GUARD=";
                trn.guardFn->Print(cout, false);
            }
            cout << endl;
        }
        // Read Arcs
        for (size_t ak = 0; ak < 3; ak++) {
            trn.arcs[ak].resize(numArcs);
            for (size_t n = 0; n < numArcs; n++) {
                Arc &a = (trn.arcs[ak])[n];
                int nPoints, mult, nLayers, ll;
                net.read(mult).read(a.plc).read(nPoints).read(nLayers);
                if (mult < 0) // Broken arc
                    mult = -mult;
                if (mult == 0)
                    throw program_exception("A constant arc multiplicity of 0 "
                                            "is not accepted.");
                if (mult >= 20000 && mult < 20000 + int(pn.mpars.size())) {
                    // 24/3/2015 Add support for marking parameters on transition arcs.
                    // cout << "Using marking parameter " << pn.mpars[mult - 20000].param->GetName() 
                    //      << " on transition arc." << endl;
                    a.multFn = make_shared<ExprMdepExpr<int> >(pn.mpars[mult - 20000].param);
                }
                else // Constant integer value
                    a.multFn = make_shared<ExprMdepExpr<int> >(make_shared<ConstValExpr<int> >(mult));
                if (a.plc <= place_t(0) || a.plc > place_t(pn.plcs.size())) {
                    ostringstream msg;
                    msg << "Place index out of bound in transition "<<trn.name<<".";
                    throw program_exception(msg.str());
                }
                a.plc--;
                net.getline(textBuf);
                if (textBuf.size() > 0) {
                    istringstream mdFnExpr(textBuf);
                    for (int l = nLayers; l > 0; l--) {
                        mdFnExpr >> ll;
                    }
                    double cx, cy;
                    mdFnExpr >> cx >> cy;
                    if (!mdFnExpr.eof()) {
                        string expr;
                        getline(mdFnExpr, expr);
                        shared_ptr<MdepExpr<int> > mdFn(ParseMarkDepMultExpr(pn, expr.c_str(),
                                                        verboseLvl));
                        mdFn.swap(a.multFn);
                    }
                }
                net.skiplines(nPoints);
                if (verboseLvl >= VL_VERBOSE) {
                    const char *arcKinds[] = { "INPUT", "OUTPUT", "INHIBITOR" };
                    cout << "   " << arcKinds[ak] << " ARC " << (n + 1) << "/" << (numArcs) << ": ";
                    cout << " place=" << a.plc << ", mult=";
                    a.multFn->Print(cout, false);
                    cout << endl;
                }
            }
            if (ak != 2)
                net.read(numArcs);
        }
    }

    //-------------------------
    // Read the .def file
    //-------------------------
    def.readtok("|256");
    def.skiplines(1);
    def.readtok("%");
    def.skiplines(1);
    do {
        def.readtok("|");
        string measExpr, measName;
        def.read(measName);
        if (measName.size() == 0)
            break; // End of the measures section
        double x, y;
        def.read(x).read(y).readtok(":");
        def.getline(measExpr);
        const Measure &m = AddNewMeasure(pn, measName.c_str(), measExpr.c_str(), verboseLvl);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "MEASURE " << measName << ": " << measExpr << "\n   PARSED AS: ";
            m.expr->Print(cout, false);
            cout << endl << endl;
        }
    }
    while (def);
}

//=============================================================================

void BuildPossiblyEnabledTransitionLookupTable(const PN &pn, PETLT &petlt,
        VerboseLevel verboseLvl) {
    // Count how many arcs are connected with each place. This will be used
    // in the PETLT construction heuristic
    vector<int> plPoints(pn.plcs.size(), 0);
    vector<Transition>::const_iterator trnit;
    for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit)
        for (size_t a = 0; a < trnit->arcs[IA].size(); a++)
            ++plPoints[ trnit->arcs[IA][a].plc ];

    // Scan each transition in the Petri Net
    for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit) {
        TrnDistrib tbl = trnit->distrib;
        place_t pl = place_t(size_t(-1));
        int curPoints = INT_MIN;

        // Choose the best input place
        for (size_t a = 0; a < trnit->arcs[IA].size(); a++) {
            const Arc &inputArc = trnit->arcs[IA][a];
            // Heuristic
            int points = int(petlt.placePE[tbl].count(inputArc.plc)) * -5;
            points += plPoints[inputArc.plc] * -3;
            if (inputArc.isMultMarkingDep())
                // Exclude Marking dependent arcs, because they may have a
                // multiplicity of 0. Eventually, a range detection for md arc
                // function results could be implemented.
                continue;

            points += trnit->arcs[IA][a].getConstantMult();

            if (points > curPoints) { // inputArc.plc is better
                pl = inputArc.plc;
                curPoints = points;
            }
        }

        if (pl != place_t(size_t(-1))) {
            petlt.placePE[tbl].insert(make_pair(pl, trnit->index));
        }
        else { // Couldn't choose a good dependent place for this transition
            petlt.alwaysPE[tbl].push_back(trnit->index);
        }
    }

    if (verboseLvl >= VL_VERBOSE) {
        cout << "\n\n";
        for (size_t tbl = 0; tbl < NumTrnDistribs; tbl++) {
            cout << "\nPETLT " << TrnKind_names[tbl] << " TABLE:\n";
            vector<Place>::const_iterator plcit;
            for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit) {
                PETLT::TrnsByPlcTbl_t::const_iterator tblit1, tblit2;
                tblit1 = petlt.placePE[tbl].lower_bound(plcit->index);
                tblit2 = petlt.placePE[tbl].upper_bound(plcit->index);
                cout << setw(20) << right << plcit->name << " =>  ";
                size_t cnt = 0;
                while (tblit1 != tblit2) {
                    cout << (cnt++ > 0 ? ", " : "") << pn.trns[tblit1->second].name;
                    ++tblit1;
                }
                cout << endl;
            }
            cout << "UNASSIGNED " << TrnKind_names[tbl] << " TRANSITIONS:\n";
            size_t cnt = 0;
            vector<transition_t>::const_iterator utit = petlt.alwaysPE[tbl].begin();
            for (; utit != petlt.alwaysPE[tbl].end(); ++utit) {
                cout << (cnt++ > 0 ? ", " : "") << pn.trns[*utit].name;
            }
            cout << endl;
        }
    }
}

//=============================================================================

size_t TransitionEnablingDegree(const PN &pn, const Transition &trn,
                                const SparseMarking &mark) {
    const size_t INFINITE_ENABDEG = numeric_limits<size_t>::max();
    size_t enabDeg = INFINITE_ENABDEG;
    vector<Arc>::const_iterator iaIt = trn.arcs[IA].begin();
    for (; iaIt != trn.arcs[IA].end(); ++iaIt) {
        tokencount_t tokens = mark[iaIt->plc];
        tokencount_t mult = iaIt->multiplicity(pn, mark);
        // Zero Arc multiplicities are ignored
        if (mult > tokencount_t(0))
            min_assign(enabDeg, size_t(tokens / mult));
    }
    if (enabDeg == INFINITE_ENABDEG) {
        ostringstream msg;
        msg << "Transition " << trn.name << " has an infinite enabling ";
        msg << "degree in marking: " << print_marking(pn, mark);
        throw program_exception(msg.str());
    }
    return enabDeg;
}

//=============================================================================

bool HasTransitionConcessionInMarking(const PN &pn, const transition_t trnInd,
                                      const SparseMarking &marking) {
    const Transition &trn = pn.trns[trnInd];
    vector<Arc>::const_iterator arcit;

    // Evaluate input arcs
    for (arcit = trn.arcs[IA].begin(); arcit != trn.arcs[IA].end(); ++arcit) {
        tokencount_t mult = arcit->multiplicity(pn, marking);
        if (marking[ arcit->plc ] < mult)
            return false;
    }
    // Evaluate inhibitor arcs
    for (arcit = trn.arcs[HA].begin(); arcit != trn.arcs[HA].end(); ++arcit) {
        tokencount_t mult = arcit->multiplicity(pn, marking);
        if (marking[ arcit->plc ] >= mult)
            return false;
    }
    // Evaluate transition guard
    if (trn.guardFn != nullptr) {
        if (!trn.guardFn->EvaluateInMarking(marking))
            return false;
    }
    // Verify a non-zero delay/weight in marking (due to marking-dependence)
    // General functions are excluded from this test, since their delay is time-dependent
    if (!trn.delayFn->IsPDF() &&
        trn.delayFn->EvaluateDelay(pn, marking, pn.trns[trnInd]) <= 0.0)
        return false;

    return true;
}

//=============================================================================

void FireTransition(const PN &pn, const transition_t &trnInd,
                    const SparseMarking &oldMarking, SparseMarking &newMarking) {
    const Transition &trn = pn.trns[trnInd];
    vector<Arc>::const_iterator arcit;

    newMarking = oldMarking;

    // Remove input tokens
    for (arcit = trn.arcs[IA].begin(); arcit != trn.arcs[IA].end(); ++arcit) {
        tokencount_t mult = arcit->multiplicity(pn, oldMarking);
        if (mult > tokencount_t(0)) {
            tokencount_t newVal = newMarking.add_element(arcit->plc, -mult);
            if (newVal < tokencount_t(0)) {
                ostringstream msg;
                msg << "Negative Token Count of " << newVal << " after the ";
                msg << "firing of " << pn.trns[trnInd].name << " in marking ";
                msg << print_marking(pn, oldMarking);
                throw program_exception(msg.str());
            }
        }
    }

    // Add output tokens
    for (arcit = trn.arcs[OA].begin(); arcit != trn.arcs[OA].end(); ++arcit) {
        tokencount_t mult = arcit->multiplicity(pn, oldMarking);
        if (mult > tokencount_t(0))
            newMarking.add_element(arcit->plc, mult);
    }
}

//=============================================================================

int ListEnabledTransitionsInMarking(const PN &pn, const PETLT &petlt,
                                    const SparseMarking &marking,
                                    TrnDistrib tbl, set<transition_t> &enabled,
                                    double *pTotalImmWeight) {
    enabled.clear();

    // Finding enabled timed transitions is a bit different than finding
    // immediate transitions, because we need to evaluate concessions & priorities.
    bool immLookup = (tbl == IMM);
    priority_t maxprio = priority_t(0);

    // Evaluate PETLT transitions: iterate over marked places
    SparseMarking::const_iterator smit;
    for (smit = marking.begin(); smit != marking.end(); ++smit) {
        PETLT::TrnsByPlcTbl_t::const_iterator tblit1, tblit2;
        tblit1 = petlt.placePE[tbl].lower_bound(smit->index);
        tblit2 = petlt.placePE[tbl].upper_bound(smit->index);
        for (; tblit1 != tblit2; ++tblit1) {
            transition_t trnInd = tblit1->second;
            if (HasTransitionConcessionInMarking(pn, trnInd, marking)) {
                // Test if transition trnInd is enabled in marking
                if (immLookup) {
                    priority_t prio = pn.trns[trnInd].prio;
                    if (prio > maxprio) {
                        maxprio = prio;
                        // Disable lower-level immediate transitions
                        enabled.clear();
                    }
                    else if (prio < maxprio)
                        continue;
                }
                enabled.insert(trnInd);
            }
        }
    }

    // Evaluate PETLT-unassigned transitions
    vector<transition_t>::const_iterator utit;
    for (utit = petlt.alwaysPE[tbl].begin(); utit != petlt.alwaysPE[tbl].end(); ++utit) {
        transition_t trnInd = *utit;
        if (HasTransitionConcessionInMarking(pn, trnInd, marking)) {
            // Test if transition trnInd is enabled in marking
            if (immLookup) {
                priority_t prio = pn.trns[trnInd].prio;
                if (prio > maxprio) {
                    maxprio = prio;
                    // Disable lower-level immediate transitions
                    enabled.clear();
                }
                else if (prio < maxprio)
                    continue;
            }
            enabled.insert(trnInd);
        }
    }

    if (tbl == IMM && pTotalImmWeight != nullptr) {
        *pTotalImmWeight = 0.0;
        set<transition_t>::const_iterator it;
        for (it = enabled.begin(); it != enabled.end(); ++it)
            *pTotalImmWeight += pn.trns[*it].weight(pn, marking);
    }

#ifndef NDEBUG
    // Verify enabled list correctness
    assert(maxprio == priority_t(0) || enabled.size() > 0);
    vector<Transition>::const_iterator trnit;
    for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit) {
        if (trnit->distrib != tbl)
            continue; // We are not interested in this kind of transitions
        bool shouldBeFound = false;
        if (HasTransitionConcessionInMarking(pn, trnit->index, marking)) {
            if (trnit->isImmediate() && trnit->prio < maxprio)
                shouldBeFound = false; // preempted by higher priority immediate transitions
            else
                shouldBeFound = true;
        }

        bool found = enabled.count(trnit->index);
        if (found != shouldBeFound) {
            cout << "\n\nfound = " << found << endl;
            cout << "shouldBeFound = " << shouldBeFound << endl;
            cout << pn.trns[trnit->index].name << endl;
            throw program_exception("PETLT algorithm is wrong, or wrong usage!");
        }
    }
#endif

    return enabled.size();
}

//=============================================================================

bool ChangeMarkPar(PN &pn, const string &mpName, int newVal) {
    map<string, markpar_t>::iterator it = pn.mparInds.find(mpName);
    if (it == pn.mparInds.end())
        return false;
    pn.mpars[it->second].param->SetValue(make_shared<ConstValExpr<int> >(newVal));
    return true;
}

//=============================================================================

bool ChangeRatePar(PN &pn, const string &rpName, double newVal) {
    map<string, ratepar_t>::iterator it = pn.rparInds.find(rpName);
    if (it == pn.rparInds.end())
        return false;
    pn.rpars[it->second].param->SetValue(make_shared<ConstValExpr<double> >(newVal));
    return true;
}

//=============================================================================

bool ValidateMarkRatePars(const PN& pn) {
    bool all_ok = true;
    for (const MarkPar& mpar : pn.mpars) {
        if (mpar.param->GetValue()->Evaluate() == -7134) {
            cout << "Marking parameter " << mpar.param->GetName() << " needs a value.\n";
            cout << "  You may assign it a value using the option -mpar "
                 << mpar.param->GetName() << " <value>" << endl;
            all_ok = false;
        }
    }
    for (const RatePar& rpar : pn.rpars) {
        if (rpar.param->GetValue()->Evaluate() == -7134.0) {
            cout << "Rate parameter " << rpar.param->GetName() << " needs a value.\n";
            cout << "  You may assign it a value using the option -rpar "
                 << rpar.param->GetName() << " <value>" << endl;
            all_ok = false;
        }
    }
    return all_ok;
}

//=============================================================================

string Transition::generalPdf() const {
    ostringstream os;
    os << setprecision(numeric_limits<double>::digits10);
    delayFn->Print(os, true);
    string fgx = os.str();
    boost::replace_all(fgx, "(double)", ""); // remove casts
    return fgx;
}

//=============================================================================

void PrintParameters(const PN &pn) {
    cout << pn.mpars.size() << " MARKING PARAMETER";
    cout << (pn.mpars.size() != 1 ? "S" : "") << ":" << endl;
    map<string, markpar_t>::const_iterator mpIt;
    for (mpIt = pn.mparInds.begin(); mpIt != pn.mparInds.end(); ++mpIt) {
        cout << "  " << setw(12) << right << mpIt->first << " = ";
        ostringstream value;
        value << print_petrinetobj(pn.mpars[mpIt->second].param->GetValue());
        if (value.str() != "-7134")
            cout << value.str() << endl;
        else
            cout << "<NOT ASSIGNED YET>" << endl;
    }
    cout << pn.rpars.size() << " RATE PARAMETER";
    cout << (pn.rpars.size() != 1 ? "S" : "") << ":" << endl;
    map<string, ratepar_t>::const_iterator rpIt;
    for (rpIt = pn.rparInds.begin(); rpIt != pn.rparInds.end(); ++rpIt) {
        cout << "  " << setw(12) << right << rpIt->first << " = ";
        ostringstream value;
        value << print_petrinetobj(pn.rpars[rpIt->second].param->GetValue());
        if (value.str() != "-7134")
            cout << value.str() << endl;
        else
            cout << "<NOT ASSIGNED YET>" << endl;
    }
}

//=============================================================================

// Print a Sparse Marking object
std::function<ostream& (ostream &)>
print_marking(const PN &pn, const SparseMarking &sm) {
    return [&](ostream & os) -> ostream& {
        os << "[";
        size_t cnt = 0;
        for (SparseMarking::const_iterator it = sm.begin(); it != sm.end(); ++it) {
            os << (cnt++ > 0 ? ", " : " ");
            if (it->value != tokencount_t(1))
                os << it->value << " @ ";
            os << pn.plcs[it->index].name;
        }
        return os << " ]";
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_place(const PN &pn, place_t plc) {
    return [&pn, plc](ostream & os) -> ostream& {
        return os << pn.plcs[plc].name;
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_transition(const PN &pn, transition_t trn) {
    return [&pn, trn](ostream & os) -> ostream& {
        return os << pn.trns[trn].name;
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_petrinetobj(const PetriNetObject &pnobj, bool expandParamNames) {
    return [&pnobj, expandParamNames](ostream & os) -> ostream& {
        pnobj.Print(os, expandParamNames);
        return os;
    };
}

std::function<ostream& (ostream &)>
print_petrinetobj(const shared_ptr<PetriNetObject> pnobj, bool expandParamNames) {
    return print_petrinetobj(*pnobj, expandParamNames);
}

//=============================================================================

const Measure &AddNewMeasure(PN &pn, const char *name, const char *expr, VerboseLevel verboseLvl) {
    for (size_t i = 0; i < pn.measures.size(); i++)
        if (pn.measures[i].name == name) {
            ostringstream oss;
            oss << "A measure named \'" << name << "\' already exists.";
            throw program_exception(oss.str());
        }

    pn.measures.resize(pn.measures.size() + 1);
    Measure &m = pn.measures.back();
    m.name = name;
    // Parse the measure expression into a MeasureExpr* tree
    m.expr = ParseMeasureExpr(pn, expr, verboseLvl);
    return m;
}

//=============================================================================

transition_t
LoadMRMC_ModelAsPN(simple_tokenizer &tra, simple_tokenizer &lab,
                   PN &pn, VerboseLevel verboseLvl) {
    // Read state labels (which becomes PN places)
    shared_ptr<Expr<int>> zeroExprI = make_shared<ConstValExpr<int>>(0);
    shared_ptr<Expr<double>> zeroExprD = make_shared<ConstValExpr<double>>(0.0);
    string label;
    lab.readtok("#DECLARATION").read(label);
    while (lab && label != "#END") {
        Place pl;
        pl.index      = pn.plcs.size();
        pl.name       = label;
        pl.initMarkFn = zeroExprI;
        pn.plcInds.insert(make_pair(label, pl.index));
        pn.plcs.emplace_back(pl);

        lab.read(label);
    }

    // Create a single empty EXP transition
    transition_t expTrn = pn.trns.size();
    Transition trn;
    trn.index   = expTrn;
    trn.name    = "MRMC_Action";
    trn.distrib = EXP;
    trn.prio    = 0;
    trn.guardFn = make_shared<ExprMdepExpr<bool>>(make_shared<ConstValExpr<bool>>(true));
    trn.delayFn = NewMarkingDependentDelayFn(make_shared<ExprMdepExpr<double>>(zeroExprD));
    pn.trnInds.insert(make_pair(trn.name, trn.index));
    pn.trns.emplace_back(trn);

    return expTrn;
}

//=============================================================================

int iscommaorparen(int ch) { return (ch == ',' || ch == ')'); }
int isnotcommaorparen(int ch) { return !iscommaorparen(ch); }

transition_t
LoadPrismModelAsPN(simple_tokenizer &tra, simple_tokenizer &lab,
                   simple_tokenizer &sta, PN &pn, VerboseLevel verboseLvl) {
    shared_ptr<Expr<int>> zeroExprI = make_shared<ConstValExpr<int>>(0);
    shared_ptr<Expr<double>> zeroExprD = make_shared<ConstValExpr<double>>(0.0);

    // Read the state description from the states file
    string label, sep;
    sta.readtok("(");
    do {
        sta.gettok(label, isnotcommaorparen).gettok(sep, iscommaorparen);
        if (label.size() == 0)
            throw program_exception(sta.pos() + ": Cannot read variable name.");

        Place pl;
        pl.index      = pn.plcs.size();
        pl.name       = label;
        pl.initMarkFn = zeroExprI;
        pn.plcInds.insert(make_pair(label, pl.index));
        pn.plcs.emplace_back(pl);
    }
    while (sta && sep == ",");

    // Add a dummy exponential transition
    transition_t expTrn = pn.trns.size();
    Transition trn;
    trn.index   = expTrn;
    trn.name    = "PrismAction";
    trn.distrib = EXP;
    trn.prio    = 0;
    trn.guardFn = make_shared<ExprMdepExpr<bool>>(make_shared<ConstValExpr<bool>>(true));
    trn.delayFn = NewMarkingDependentDelayFn(make_shared<ExprMdepExpr<double>>(zeroExprD));
    pn.trnInds.insert(make_pair(trn.name, trn.index));
    pn.trns.emplace_back(trn);

    return expTrn;
}

//=============================================================================















