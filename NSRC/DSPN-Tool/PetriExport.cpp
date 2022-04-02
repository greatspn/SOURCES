/*
 *  PetriExport.cpp
 *
 *  Implementation of the PetriNet/TRG export functionalities.
 *
 *  Created by Elvio Amparore
 *  COSMOS exporter has been implemented by Djafri Hilal.
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
#include <cfloat>
#include <climits>
#include <memory>
#include <unordered_map>
#include <functional>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Measure.h"
#include "ReachabilityGraph.h"
#include "PackedMemoryPool.h"
#include "NewReachabilityGraph.h"
#include "PetriExport.h"
#include "SolveCTMC.h"

//=============================================================================
//     Function implementations
//=============================================================================

// Write a given TRG marking as a input line in the Graphviz dot format
void WriteStateInDotFormat(const PN &pn, const RG2 &rg,
                           const RgState &rs, ofstream &dot) {
    bool isVanish = rs.selfIndex.type() == VANISHING;
    bool isDead = (rs.firstEdge == INVALID_RGEDGE_INDEX);
    const char *titleClr = (isVanish ? "#CCCCCC" : "#DDAAAA");
    const char *enabDetClr = (isVanish ? "#BBBBBB" : "#999999");
    dot << "  " << print_rgstate(rs.selfIndex) << " [ ";
    dot << (isVanish ? "color=gray50, style=dotted, " : "");
    dot << "label=< <font color=\"" << titleClr << "\">";
    dot << print_rgstate(rs.selfIndex) << ":</font> <br/>";

    SparseMarking::const_iterator smit = rs.marking.begin();
    size_t lineChCnt = 0, LINE_LIMIT = 25;
    for (; smit != rs.marking.end(); ++smit) {
        size_t wordChCnt = pn.plcs[smit->index].name.size();
        if (smit->value > tokencount_t(1))
            wordChCnt += 4;
        if (lineChCnt + wordChCnt > LINE_LIMIT && lineChCnt > 0) {
            dot << "<br/>";
            lineChCnt = 0;
        }
        else if (lineChCnt > 0)
            dot << ", ";
        if (smit->value > tokencount_t(1))
            dot << smit->value << " @ ";
        dot << pn.plcs[smit->index].name;
        lineChCnt += wordChCnt;
    }
    bool hasDetEnabled = (rs.enabledGen.size() > 0);
    if (hasDetEnabled) {
        dot << "<br/><font point-size=\"5\"> </font>";
        dot << "<br/><font color=\"" << enabDetClr << "\" point-size=\"10\">enabled={";
        set<transition_t>::const_iterator trnIt;
        size_t cnt;
        for (cnt = 0, trnIt = rs.enabledGen.begin();
                trnIt != rs.enabledGen.end(); ++cnt, ++trnIt)
            dot << (cnt > 0 ? ", " : " ") << pn.trns[*trnIt].name;
        dot << " }</font>";
    }
    if (isDead)
        dot << "<br/><font color=\"#EE0000\">DEAD</font>";
    dot << ">, penwidth=" << (hasDetEnabled && !isVanish ? 3 : 1) << " ];\n";
}


// Writes an arc between two marking in Graphviz dot format
void WriteEdgeInDotFormat(const PN &pn, const RG2 &rg, const rgstate_t srcState,
                          const RgEdge &edge, ofstream &dot) {
    const Transition &tt = pn.trns[edge.firedTrn];
    dot << "  " << print_rgstate(srcState) << " -> " << print_rgstate(edge.dest);
    dot << " [label=<" << tt.name;
    dot << "<br/><font face=\"Courier\" color=\"#6060A0\">";
    dot << TrnDistrib_names_lowercase[tt.distrib];
    if (edge.hasDelay()) {
        if (pn.trns[edge.firedTrn].isGeneral())
            dot << "[" << pn.trns[edge.firedTrn].generalPdf() << "]";
        else {
            if (edge.prob != 1.0)
                dot << "[" << edge.delay << "*" << edge.prob << "]";
            else
                dot << "[" << edge.delay << "]";
        }
    }
    // TODO: I'm not printing the immediate weight
    /*if (tt.kind == IMM)	dot << "[" << edge. << "]";*/
    if (edge.hasProb()) {
        dot << "[" << edge.prob << "]";
    }
    bool isPreem = edge.preempted != EMPTY_TRANSITION_SET;
    if (isPreem) {
        dot << "<br/>preem={";
        transition_set_table::const_seq_iterator trnIt;
        size_t cnt;
        for (cnt = 0, trnIt = rg.trnSetTbl.begin_seq(edge.preempted);
                trnIt != rg.trnSetTbl.end(); ++cnt, ++trnIt)
            dot << (cnt > 0 ? ",<br/>" : "") << pn.trns[*trnIt].name;
        dot << "}";
    }
    dot << "</font>>, penwidth=" << (tt.isGeneral() || isPreem ? 3 : 1);
    dot << (tt.isGeneral() ? ", color=\"#000070\"" : "");
    dot << (isPreem ? ", style=dashed, color=\"#700000\"" : "");
    dot << (tt.isImmediate() ? ", style=dotted, color=dimgray, arrowhead=vee" : "");
    dot << (isPreem && tt.isImmediate() ? ", color=\"#703030\"" : "");
    dot << " ];\n";
}


void SaveRG_AsDotFile(const PN &pn, const RG2 &rg,
                      bool drawClusters, ofstream &dot,
                      const size_t maxSavedMarkings) 
{
    // Decide which markings will be saved in the dot file
    enum { EXCLUDED, INCLUDED, FRONTIER };
    size_t numSavedMarks = 0;
    vector<uint8_t> saved[NUM_RG_STATE_TYPES];
    for (size_t k = 0; k < 2; k++) {
        saved[k].resize(rg.stateSets[k].size());
        std::fill(saved[k].begin(), saved[k].end(), EXCLUDED);
    }
    for (const InitialFrontierEntry& st : rg.initStates) {
        saved[st.state.type()][st.state.index()] = INCLUDED;
        numSavedMarks++;
    }
    size_t frontierSize = 0;
    while (numSavedMarks < maxSavedMarkings) {
        // Bring the frontier in the included set
        for (size_t k = 0; k < 2; k++) 
            for (size_t st = 0; st < saved[k].size(); ++st)
                if (saved[k][st] == FRONTIER) {
                    saved[k][st] = INCLUDED;
                    numSavedMarks++;
                    frontierSize--;
                    if (numSavedMarks >= maxSavedMarkings)
                        goto end_frontier_cycle;
                }
        end_frontier_cycle:
        // if (frontierSize > 0)
        //     break;

        // Visit the TRG frontier
        RgEdge edge;
        for (size_t k = 0; k < 2; k++) {
            RgStateType type = (k == 0 ? VANISHING : TANGIBLE);
            for (size_t s = 0; s < rg.stateSets[type].size(); s++) {
                rgedge_t edgeList = rg.stateSets[type].get_first_edge(s);
                while (edgeList != INVALID_RGEDGE_INDEX) {
                    GetEdgeByIndex(rg, edgeList, edge);
                    if (saved[type][s] == INCLUDED) {
                        if (saved[edge.dest.type()][edge.dest.index()] == EXCLUDED) {
                            saved[edge.dest.type()][edge.dest.index()] = FRONTIER;
                            frontierSize++;
                        }
                    }
                    edgeList = edge.nextEdge;
                }
            }
        }
        if (frontierSize == 0)
            break; // All markings have been included
    }
    //cout << "Will save " << numSavedMarks << "/" << rg.NumStates() << " markings." << endl;

    dot << "digraph RRG {\n  rankdir=TB;\n  ratio=compress;\n";
    dot << "  node[shape=ellipse, fontname=Arial];\n";

    drawClusters = drawClusters && (rg.rgClass == RGC_MarkovRenewalProcess);
    bool hasSingleInitMark = (rg.initStates.size() == 1);

    if (frontierSize > 0)
        drawClusters = false; // Disable clusters if the TRG is incomplete.

    // Initial marking(s)
    if (rg.initStates.size() > 1) {
        dot << "  pi0 [ shape=record, color=blue2, label=\"Initial Probability:";
        for (size_t p = 0; p < rg.initStates.size(); p++)
            dot << "|<v" << p << ">" << rg.initStates[p].initProb;
        dot << "\" ];\n";
        for (size_t p = 0; p < rg.initStates.size(); p++) {
            dot << "  pi0:v" << p << " -> ";
            dot << print_rgstate(rg.initStates[p].state);
            dot << " [ len=0.5, color=skyblue4, arrowhead=vee ];\n";
        }
    }
    else {
        if (!drawClusters) {
            dot << "  init[shape=none, label=< >];\n";
            dot << "  init -> ";
            if (rg.initState.selfIndex.type() == TANGIBLE)
                dot << print_rgstate(rg.initState.selfIndex);
            else
                dot << print_rgstate(rg.initStates[0].state);
            dot << " [len=0.5, color=navy];\n";
        }
    }

    // RG Markings
    RgState rs;
    if (drawClusters) {
        // Subdivide the markings in multiple clusters
        dot << "\nsubgraph cluster_exp {\n  color=blue; label=\"EXP\"\n";
        for (size_t i = 0; i < rg.NumStates(); i++) {
            GetIthState(rg, i, rs);
            if (rs.enabledGen.size() == 0) {
                WriteStateInDotFormat(pn, rg, rs, dot);
                if (hasSingleInitMark && rs.selfIndex == rg.initState.selfIndex) {
                    dot << "  init[shape=none, label=<>];\n";
                    dot << "  init -> " << print_rgstate(rg.initState.selfIndex);
                    dot << " [len=0.5, color=navy];\n";
                }
            }
        }
        dot << "}\n\n";

        vector<transition_t>::const_iterator detTrnIt = pn.detTrnInds.begin();
        for (; detTrnIt != pn.detTrnInds.end(); ++detTrnIt) {
            dot << "\nsubgraph cluster_det" << *detTrnIt << " {\n  color=blue;";
            dot << " label=\"Det[" << pn.trns[*detTrnIt].name << "]\"\n";
            for (size_t i = 0; i < rg.NumStates(); i++) {
                GetIthState(rg, i, rs);
                if (rs.enabledGen.size() == 1 &&
                        *rs.enabledGen.begin() == *detTrnIt) {
                    WriteStateInDotFormat(pn, rg, rs, dot);
                    if (hasSingleInitMark && rs.selfIndex == rg.initState.selfIndex) {
                        dot << "  init[shape=none, label=<>];\n";
                        dot << "  init -> " << print_rgstate(rg.initState.selfIndex);
                        dot << " [len=0.5, color=navy];\n";
                    }
                }
            }
            dot << "}\n\n";
        }
    }
    else {
        // Write all the markings
        for (size_t i = 0; i < rg.NumStates(); i++) {
            GetIthState(rg, i, rs);
            switch (saved[rs.selfIndex.type()][rs.selfIndex.index()]) {
                case INCLUDED:
                    WriteStateInDotFormat(pn, rg, rs, dot);
                    break;

                case FRONTIER:
                    dot << " " << print_rgstate(rs.selfIndex) << "[shape=none, label=<...>];\n";
                    break;

                case EXCLUDED:
                    break;
            }
        }
    }

    // RG edges
    dot << "\n\n  edge[len=1.5, fontsize=10, fontname=Arial];\n";
    RgEdge edge;
    for (size_t k = 0; k < 2; k++) {
        RgStateType type = (k == 0 ? VANISHING : TANGIBLE);
        for (size_t s = 0; s < rg.stateSets[type].size(); s++) {
            rgedge_t edgeList = rg.stateSets[type].get_first_edge(s);
            while (edgeList != INVALID_RGEDGE_INDEX) {
                GetEdgeByIndex(rg, edgeList, edge);
                if (saved[type][s] == INCLUDED)
                    WriteEdgeInDotFormat(pn, rg, rgstate_t(s, type), edge, dot);
                edgeList = edge.nextEdge;
            }
        }
    }

    dot << "\n}\n";
}

//=============================================================================

// Write a given TRG marking as a input line in the Graphviz dot format
void WriteMarkingInDotFormat(const PN &pn, const ReachableMarking &rm,
                             ofstream &dot) {
    bool isVanish = rm.isVanishing;
    bool isDead = (rm.timedPaths.size() == 0);
    const char *titleClr = (isVanish ? "#CCCCCC" : "#DDAAAA");
    const char *enabDetClr = (isVanish ? "#BBBBBB" : "#999999");
    dot << "  mark" << rm.index << " [ ";
    dot << (isVanish ? "color=gray50, style=dotted, " : "");
    dot << "label=< <font color=\"" << titleClr << "\">";
    dot << "Marking " << (size_t(rm.index) + 1) << ":</font> <br/>";

    SparseMarking::const_iterator smit = rm.marking.begin();
    size_t lineChCnt = 0, LINE_LIMIT = 25;
    for (; smit != rm.marking.end(); ++smit) {
        size_t wordChCnt = pn.plcs[smit->index].name.size();
        if (smit->value > tokencount_t(1))
            wordChCnt += 4;
        if (lineChCnt + wordChCnt > LINE_LIMIT && lineChCnt > 0) {
            dot << "<br/>";
            lineChCnt = 0;
        }
        else if (lineChCnt > 0)
            dot << ", ";
        if (smit->value > tokencount_t(1))
            dot << smit->value << " @ ";
        dot << pn.plcs[smit->index].name;
        lineChCnt += wordChCnt;
    }
    bool hasDetEnabled = (rm.enabledDetInds.size() > 0);
    bool hasVanishPreemptables = (rm.vanishPreemptables.size() > 0);
    bool hasVanishPreempted = (rm.vanishPreempted.size() > 0);
    set<transition_t>::const_iterator trnIt;
    size_t cnt;
    if (hasDetEnabled || hasVanishPreemptables || hasVanishPreempted)
        dot << "<br/><font point-size=\"5\"> </font>";
    if (hasDetEnabled) {
        dot << "<br/><font color=\"" << enabDetClr << "\" point-size=\"10\">enabled={";
        for (cnt = 0, trnIt = rm.enabledDetInds.begin();
                trnIt != rm.enabledDetInds.end(); ++cnt, ++trnIt)
            dot << (cnt > 0 ? ", " : " ") << pn.trns[*trnIt].name;
        dot << " }</font>";
    }
    if (hasVanishPreemptables) {
        dot << "<br/><font color=\"#7080C0\" point-size=\"10\">preemptable={";
        for (cnt = 0, trnIt = rm.vanishPreemptables.begin();
                trnIt != rm.vanishPreemptables.end(); ++cnt, ++trnIt)
            dot << (cnt > 0 ? ", " : " ") << pn.trns[*trnIt].name;
        dot << " }</font>";
    }
    if (hasVanishPreempted) {
        dot << "<br/><font color=\"#C07080\" point-size=\"10\">preempted={";
        for (cnt = 0, trnIt = rm.vanishPreempted.begin();
                trnIt != rm.vanishPreempted.end(); ++cnt, ++trnIt)
            dot << (cnt > 0 ? ", " : " ") << pn.trns[*trnIt].name;
        dot << " }</font>";
    }
    if (isDead)
        dot << "<br/><font color=\"#EE0000\">DEAD</font>";
    dot << ">, penwidth=" << (hasDetEnabled && !isVanish ? 3 : 1) << " ];\n";
}


// Writes an arc between two marking in Graphviz dot format
void WriteArcInDotFormat(const PN &pn, const marking_t srcMarkIndex,
                         const RGArc &arc, ofstream &dot) {
    const Transition &tt = pn.trns[arc.timedTrnInd];
    dot << "  mark" << srcMarkIndex << " -> mark" << arc.destMarking;
    dot << " [label=<" << tt.name;
    dot << "<br/><font face=\"Courier\" color=\"#6060A0\">(";
    dot << TrnDistrib_names_lowercase[tt.distrib];
    dot << "[" << arc.delay << "]";
    if (arc.prob != 1.0) {
        dot << ",<br/>π=" << arc.prob;
    }
    bool isPreemExp = (tt.distrib == EXP && arc.preempted.size() > 0);
    if (isPreemExp) {
        dot << ",<br/>preem={";
        set<transition_t>::const_iterator trnIt;
        size_t cnt;
        for (cnt = 0, trnIt = arc.preempted.begin();
                trnIt != arc.preempted.end(); ++cnt, ++trnIt)
            dot << (cnt > 0 ? ",<br/>" : "") << pn.trns[*trnIt].name;
        dot << "}";
    }
    dot << ")</font>>, penwidth=" << (tt.isGeneral() || isPreemExp ? 3 : 1);
    dot << (tt.isGeneral() ? ", color=\"#000070\"" : "");
    dot << (isPreemExp ? ", style=dashed, color=\"#700000\"" : "");
    dot << (tt.isImmediate() ? ", style=dotted, color=dimgray, arrowhead=vee" : "");
    dot << " ];\n";
}


void SaveRG_AsDotFile(const PN &pn, const RG &rg,
                      bool drawClusters, ofstream &dot) {
    dot << "digraph RRG {\n  rankdir=TB;\n  ratio=compress;\n";
    dot << "  node[shape=ellipse, fontname=Arial];\n";

    drawClusters = drawClusters && (rg.rgClass == RGC_MarkovRenewalProcess);
    bool hasSingleInitMark = (!rg.hasMultipleInitMarks());

    // Initial marking(s)
    if (rg.hasMultipleInitMarks()) {
        dot << "  pi0 [ shape=record, color=blue2, label=\"Initial Probability:";
        for (size_t p = 0; p < rg.initMark.timedPaths.size(); p++)
            dot << "|<v" << p << ">" << rg.initMark.timedPaths[p].prob;
        dot << "\" ];\n";
        for (size_t p = 0; p < rg.initMark.timedPaths.size(); p++) {
            dot << "  pi0:v" << p << " -> mark";
            dot << rg.initMark.timedPaths[p].destMarking;
            dot << " [ len=0.5, color=skyblue4, arrowhead=vee ];\n";
        }
    }
    else {
        if (!drawClusters) {
            dot << "  init[shape=none, label=<>];\n";
            dot << "  init -> mark0[len=0.5, color=navy];\n";
        }
    }

    // RG Markings
    vector<ReachableMarking>::const_iterator rmit;
    if (drawClusters) {
        // Subdivide the markings in multiple clusters
        dot << "\nsubgraph cluster_exp {\n  color=blue; label=\"EXP\"\n";
        for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
            if (rmit->enabledDetInds.size() == 0) {
                WriteMarkingInDotFormat(pn, *rmit, dot);
                if (hasSingleInitMark && rmit->index == marking_t(0)) {
                    dot << "  init[shape=none, label=< >];\n";
                    dot << "  init -> mark0[len=0.5, color=navy];\n";
                }
            }
        }
        dot << "}\n\n";

        vector<transition_t>::const_iterator detTrnIt = pn.detTrnInds.begin();
        for (; detTrnIt != pn.detTrnInds.end(); ++detTrnIt) {
            dot << "\nsubgraph cluster_det" << *detTrnIt << " {\n  color=blue;";
            dot << " label=\"Det[" << pn.trns[*detTrnIt].name << "]\"\n";
            for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
                if (rmit->enabledDetInds.size() == 1 &&
                        *rmit->enabledDetInds.begin() == *detTrnIt) {
                    WriteMarkingInDotFormat(pn, *rmit, dot);
                    if (hasSingleInitMark && rmit->index == marking_t(0)) {
                        dot << "  init[shape=none, label=<>];\n";
                        dot << "  init -> mark0[len=0.5, color=navy];\n";
                    }
                }
            }
            dot << "}\n\n";
        }
    }
    else {
        // Write all the marking togheter
        for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
            WriteMarkingInDotFormat(pn, *rmit, dot);
        }

    }

    // RG Arcs
    dot << "\n\n  edge[len=1.5, fontsize=10, fontname=Arial];\n";
    for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
        vector<RGArc>::const_iterator rgArcIt = rmit->timedPaths.begin();
        for (; rgArcIt != rmit->timedPaths.end(); ++rgArcIt) {
            WriteArcInDotFormat(pn, rmit->index, *rgArcIt, dot);
        }
    }

    dot << "\n}\n";
}

//=============================================================================

void SaveAsSPNicaPetriNet(const PN &pn, ostream &nbf) {
    nbf << "(* SPNica definition of the Petri Net *)\n";

    // Write Marking and Rate Parameters
    for (size_t i = 0; i < pn.mpars.size(); i++) {
        nbf << pn.mpars[i].param->GetName() << " = ";
        pn.mpars[i].param->PrintInSPNicaFormat(nbf, pn);
        nbf << ";\n";
    }
    for (size_t i = 0; i < pn.rpars.size(); i++) {
        nbf << pn.rpars[i].param->GetName() << " = ";
        pn.rpars[i].param->PrintInSPNicaFormat(nbf, pn);
        nbf << ";\n";
    }

    // Write Places
    vector<Place>::const_iterator plcit;
    for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit)
        nbf << (plcit->index == place_t(0) ? "P={ " : ",\n    ") << plcit->name;
    nbf << " };\n";

    // Write Transitions
    vector<Transition>::const_iterator trnit;
    for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit) {
        nbf << (trnit->index == transition_t(0) ? "T={ " : ",\n    ");
        nbf << "{" << trnit->name << ", {";
        switch (trnit->distrib) {
        case IMM:
            nbf << "type->\"imm\", prio->" << trnit->prio;
            nbf << ", weight->";
            trnit->delayFn->PrintInSPNicaFormat(nbf, pn, *trnit);
            break;
        case EXP:
            nbf << "dist->exp[";
            trnit->delayFn->PrintInSPNicaFormat(nbf, pn, *trnit);
            nbf << "]";
            break;
        case DET:
            nbf << "dist->I[";
            trnit->delayFn->PrintInSPNicaFormat(nbf, pn, *trnit);
            nbf << "]";
            break;
        default:
            assert(0);
        }
        if (trnit->guardFn != nullptr) {
            nbf << ",guard->";
            trnit->guardFn->PrintInSPNicaFormat(nbf, pn);
        }
        nbf << "}}";
    }
    nbf << " };\n";

    // Write Input, Output and Inhibitors arcs
    for (size_t ak = 0; ak < 3; ak++) {
        nbf << ArcKind_names[ak] << "={ ";
        size_t cnt = 0;
        for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit) {
            vector<Arc>::const_iterator arcit = trnit->arcs[ak].begin();
            for (; arcit != trnit->arcs[ak].end(); ++arcit) {
                if (cnt++ > 0)
                    nbf << ",\n     ";
                const string &trNam = pn.trns[trnit->index].name;
                const string &plNam = pn.plcs[arcit->plc].name;
                if (ak == OA)
                    nbf << "{" << trNam << ", " << plNam << ", ";
                else
                    nbf << "{" << plNam << ", " << trNam << ", ";
                arcit->multFn->PrintInSPNicaFormat(nbf, pn);
                nbf << "}";
            }
        }
        nbf << " };\n";
    }

    // Write the Initial Marking
    for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit) {
        nbf << (plcit->index == place_t(0) ? "M0={ " : ",\n     ");
        nbf << plcit->name << "->";
        plcit->initMarkFn->PrintInSPNicaFormat(nbf, pn);
    }
    nbf << " };\n";

    // Write measures
    nbf << "measures={ ";
    vector<Measure>::const_iterator measit;
    for (measit = pn.measures.begin(); measit != pn.measures.end(); ++measit) {
        nbf << (measit == pn.measures.begin() ? "" : ",\n           ");
        nbf << measit->name << " -> mean[";
        measit->expr->PrintInSPNicaFormat(nbf, pn);
        nbf << "]";
    }
    nbf << " };\n";

    // TODO: remove this code
    /*for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit)
    	cout << "|Mean_" << plcit->name << "  0 0: E{#"<< plcit->name<<"};\n";
    for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit)
    	if (trnit->kind != IMM)
    		cout << "|Thr_" << trnit->name << "  0 0: X{"<< trnit->name<<"};\n";
     */

    nbf << "PN={};\nTN={};\n";
    nbf << "\nSPN={P,T,IA,OA,HA,M0,measures};\n\n";
    nbf << "(* SPNstationary[SPN] *)\n";
}

//=============================================================================

// Save the Petri Net in the GSPN input format of COSMOS
void SaveAsCosmosPetriNet(const PN &pn, const string &NetName, ostream &out) {
    // Write Marking and Rate Parameters
    for (size_t i = 0; i < pn.mpars.size(); i++) {
        out << "const int " << pn.mpars[i].param->GetName() << " = ";
        out << pn.mpars[i].param->Evaluate() << ";\n";
    }
    for (size_t i = 0; i < pn.rpars.size(); i++) {
        out << "const double " << pn.rpars[i].param->GetName() << " = ";
        out << pn.rpars[i].param->Evaluate() << ";\n";
    }
    out << "NbPlaces= " << pn.plcs.size() << ";" << endl;
    out << "NbTransitions= " << pn.trns.size() << ";\n" << endl;

    // Write Places List
    out << "\nPlacesList ={ ";
    vector<Place>::const_iterator plcit;
    size_t sz;
    for (sz = 0, plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++sz, ++plcit)
        out << (sz == 0 ? "" : ", ") << plcit->name;
    out << " };\n" << endl;

    // Write Transitions List
    out << "TransitionsList ={ ";
    vector<Transition>::const_iterator trnit;
    for (sz = 0, trnit = pn.trns.begin(); trnit != pn.trns.end(); ++sz, ++trnit)
        out << (sz == 0 ? "" : ", ") << trnit->name;
    out << " };\n" << endl;

    // Write the Initial Marking
    out << "Marking ={\n";
    for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit) {
        out << "\t(" << plcit->name << " , ";
        plcit->initMarkFn->PrintInCosmosFormat(out, pn);
        out << ");\n";
    }
    out << "};\n" << endl;

    // Write Transitions
    out << "Transitions ={\n";
    for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit) {
        out << "\t(" << trnit->name << ", ";
        trnit->delayFn->PrintInCosmosFormat(out, pn, *trnit);
        out << ");\n";
    }
    out << "};\n" << endl;

    // Write Input, Output and Inhibitors arcs
    for (size_t ak = 0; ak < 3; ak++) {
        bool AtLeastOneArcForKind_k = false;

        for (trnit = pn.trns.begin(); trnit != pn.trns.end(); trnit++) {
            if (trnit->arcs[ak].size() > 0) {
                AtLeastOneArcForKind_k = true;
                break;
            }
        }
        if (AtLeastOneArcForKind_k) {
            switch (ak) {
            case IA:   out << "InArcs={\n";			break;
            case OA:   out << "OutArcs={\n";			break;
            case HA:   out << "InhibitorArcs={\n";		break;
            }
            for (trnit = pn.trns.begin(); trnit != pn.trns.end(); ++trnit) {
                vector<Arc>::const_iterator arcit = trnit->arcs[ak].begin();
                for (; arcit != trnit->arcs[ak].end(); ++arcit) {
                    const string &trNam = pn.trns[trnit->index].name;
                    const string &plNam = pn.plcs[arcit->plc].name;
                    if (ak == OA)
                        out << "\t(" << trNam << ", " << plNam << ", ";
                    else
                        out << "\t(" << plNam << ", " << trNam << ", ";
                    arcit->multFn->PrintInCosmosFormat(out, pn);
                    out << ");\n";
                }
            }
            out << "};\n" << endl;
        }
    }
}

//=============================================================================

// Save the TRG as an input Action and State-labeled CTMC (ASMC file)
void SaveAsCsltaASMC(const PN &pn, const RG2 &rg,
                     const char *name, ofstream &asmc) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("Only CTMCs can be saved as ASMCs.");
    if (rg.initStates.size() > 1)
        throw program_exception("RG with multiple initial markings cannot be saved as ASMCs.");

    // Write Marking and Rate Parameters
    asmc << "\n";
    for (size_t i = 0; i < pn.mpars.size(); i++) {
        asmc << "CONST " << pn.mpars[i].param->GetName() << " = ";
        asmc << pn.mpars[i].param->Evaluate() << ";\n";
    }
    for (size_t i = 0; i < pn.rpars.size(); i++) {
        asmc << "CONST " << pn.rpars[i].param->GetName() << " = ";
        asmc << pn.rpars[i].param->Evaluate() << ";\n";
    }

    asmc << "\nASMC " << name << " = (\n";

    // Write ASMC states with Atomic Propositions
    RgState state;
    for (size_t i = 0; i < rg.NumStates(); i++) {
        GetIthState(rg, i, state);
        asmc << "\tSTATE s" << state.selfIndex.index() << " : ";
        SparseMarking::const_iterator smit;
        size_t apCnt = 0;
        for (smit = state.marking.begin(); smit != state.marking.end(); ++smit) {
            asmc << (apCnt++ > 0 ? ", " : "") << pn.plcs[smit->index].name;
            if (smit->value != tokencount_t(1))
                asmc << "=" << smit->value;
        }
        asmc << "\n";
    }
    asmc << "\n";

    // Write ASMC labeled transitions
    RgEdge edge;
    for (size_t s = 0; s < rg.stateSets[TANGIBLE].size(); s++) {
        rgedge_t edgeList = rg.stateSets[TANGIBLE].get_first_edge(s);
        rgstate_t srcSt(s, TANGIBLE);
        while (edgeList != INVALID_RGEDGE_INDEX) {
            GetEdgeByIndex(rg, edgeList, edge);
            UnpackState(rg, srcSt, state);
            const Transition &trn = pn.trns[edge.firedTrn];
            asmc << "\tTRANSITION s" << srcSt.index() << " -> s" << edge.dest.index();
            asmc << " (" << trn.name << ", ";
            trn.delayFn->PrintInMarking(asmc, pn, state.marking, trn);
            asmc << ")\n";
            edgeList = edge.nextEdge;
        }
    }
    asmc << ");\n\n";
}

//=============================================================================

// Save the TRG as an input Action and State-labeled CTMC (ASMC file)
void SaveAsCsltaASMC(const PN &pn, const RG &rg,
                     const char *name, ofstream &asmc) {
    if (rg.rgClass != RGC_CTMC)
        throw program_exception("Only CTMCs can be saved as ASMCs.");
    if (rg.hasMultipleInitMarks())
        throw program_exception("RG with multiple initial markings cannot be saved as ASMCs.");

    // Write Marking and Rate Parameters
    asmc << "\n";
    for (size_t i = 0; i < pn.mpars.size(); i++) {
        asmc << "CONST " << pn.mpars[i].param->GetName() << " = ";
        asmc << pn.mpars[i].param->Evaluate() << ";\n";
    }
    for (size_t i = 0; i < pn.rpars.size(); i++) {
        asmc << "CONST " << pn.rpars[i].param->GetName() << " = ";
        asmc << pn.rpars[i].param->Evaluate() << ";\n";
    }

    asmc << "\nASMC " << name << " = (\n";

    // Write ASMC states with Atomic Propositions
    vector<ReachableMarking>::const_iterator rmit;
    for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
        asmc << "\tSTATE s" << rmit->index << " : ";
        SparseMarking::const_iterator smit;
        size_t apCnt = 0;
        for (smit = rmit->marking.begin(); smit != rmit->marking.end(); ++smit) {
            asmc << (apCnt++ > 0 ? ", " : "") << pn.plcs[smit->index].name;
            if (smit->value != tokencount_t(1))
                asmc << "=" << smit->value;
        }
        asmc << "\n";
    }
    asmc << "\n";

    // Write ASMC labeled transitions
    for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
        vector<RGArc>::const_iterator arcit;
        for (arcit = rmit->timedPaths.begin(); arcit != rmit->timedPaths.end(); ++arcit) {
            const Transition &trn = pn.trns[arcit->timedTrnInd];
            asmc << "\tTRANSITION s" << rmit->index << " -> s" << arcit->destMarking;
            asmc << " (" << trn.name << ", ";
            trn.delayFn->PrintInMarking(asmc, pn, rmit->marking, trn);
            asmc << ")\n";
        }
    }
    asmc << ");\n\n";
}

//=============================================================================

// Save the TRG of a regular CTMC in the Prism file format
void SaveAsPrismCTMC(const PN &pn, const RG &rg, ofstream &ctmcFile,
                     ofstream &states, const char *pi0distrFileName) {
    // Save the transition matrix
    size_t numArcs = 0;
    for (size_t phase = 0; phase < 2; phase++) {
        vector<ReachableMarking>::const_iterator rmit;
        for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
            vector<RGArc>::const_iterator arcit = rmit->timedPaths.begin();
            for (; arcit != rmit->timedPaths.end(); ++arcit) {
                if (phase == 0) {
                    // Count arcs
                    numArcs++;
                }
                else {
                    // write arcs in the ctmc file
                    ctmcFile << rmit->index << " " << arcit->destMarking << " ";
                    ctmcFile << arcit->delay << "\n";
                }
            }
        }
        if (phase == 0)
            ctmcFile << rg.markSet.size() << " " << numArcs << "\n";
    }

    // Save the state markings
    for (size_t i = 0; i < pn.plcs.size(); i++)
        states << (i > 0 ? "," : "(") << pn.plcs[i].name;
    states << ")\n";
    vector<ReachableMarking>::const_iterator rmit;
    for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
        states << rmit->index << ":";
        for (size_t i = 0; i < pn.plcs.size(); i++)
            states << (i > 0 ? "," : "(") << rmit->marking[i];
        states << ")\n";
    }

    // Save the pi0 vector (if non-trivial)
    if (rg.hasMultipleInitMarks()) {
        cout << "The initial distribution pi0 is divided in more than a single ";
        cout << "state.\nAn initial distribution vector will also be saved." << endl;

        ofstream pi0distr(pi0distrFileName);
        ublas::vector<double> pi0;
        pi0.resize(rg.markSet.size());
        fill(pi0.begin(), pi0.end(), 0.0);
        for (size_t n = 0; n < rg.initMark.timedPaths.size(); n++) {
            const RGArc &arc = rg.initMark.timedPaths[n];
            pi0(arc.destMarking) = arc.prob;
        }
        pi0distr << pi0 << endl;
        pi0distr.close();
    }
}

//=============================================================================

void SaveReachabilityGraph(const PN &pn, const RG &rg, ostream &rgf) {
    size_t arcSetSize = 0;
    size_t expSetSize = 0;
    size_t numMarks = rg.markSet.size();

    // Save the RG class
    rgf << "# RG Class (one of CTMC, MRP, GSMP, UNCLASSIFIED):\n";
    rgf << "  " << RGClass_ShortNames[rg.rgClass] << "\n\n";

    // Save Place Names
    // TODO

    // Save Transition Names
    // TODO

    // Save the Marking Set
    rgf << "# Marking Set (T/V, sparseMarking, [vanishPreempted]):\n";
    rgf << "  " << rg.markSet.size() << "\n";
    vector<ReachableMarking>::const_iterator rmit;
    for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
        rgf << "  " << (rmit->isVanishing ? 'V' : 'T') << " ";
        rgf << setw(2) << right << rmit->marking.nonzeros() << ": ";
        SparseMarking::const_iterator smit;
        for (smit = rmit->marking.begin(); smit != rmit->marking.end(); ++smit) {
            rgf << smit->value << "@" << (size_t(smit->index) + 1) << " ";
        }
        if (rmit->isVanishing) {
            rgf << " \t" << rmit->vanishPreempted.size() << ": ";
            set<transition_t>::const_iterator trnIt = rmit->vanishPreempted.begin();
            for (; trnIt != rmit->vanishPreempted.end(); ++trnIt) {
                rgf << size_t(*trnIt) + 1 << " ";
            }
        }
        rgf << "\n";
        arcSetSize += rmit->timedPaths.size();
        if (rmit->enabledDetInds.size() == 0  && !rmit->isVanishing)
            expSetSize ++;
    }
    rgf << "\n";

    // Save the Arc Set
    rgf << "# Arc Set (src, dst, transition, dist, prob, preemptSet):\n";
    rgf << "  " << arcSetSize << "\n";
    for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
        vector<RGArc>::const_iterator arcit;
        for (arcit = rmit->timedPaths.begin(); arcit != rmit->timedPaths.end(); ++arcit) {
            rgf << "  " << setw(2) << left << size_t(rmit->index) + 1 << " ";
            rgf << setw(2) << left << size_t(arcit->destMarking) + 1 << "  ";
            rgf << setw(2) << left << size_t(arcit->timedTrnInd) + 1 << "  ";
            rgf << TrnDistrib_names[arcit->distrib] << " ";
            rgf << setw(9) << left << arcit->delay << "  ";
            rgf << "  " << arcit->preempted.size() << ": ";
            set<transition_t>::const_iterator trnIt = arcit->preempted.begin();
            for (; trnIt != arcit->preempted.end(); ++trnIt) {
                rgf << size_t(*trnIt) + 1 << " ";
            }
            rgf << "\n";
        }
    }
    rgf << "\n";

    // Save the MRP Exponential Set
    rgf << "# Tangible states with no Deterministic transitions enabled:\n";
    rgf << "  " << expSetSize << ": ";
    for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
        if (rmit->enabledDetInds.size() == 0 && !rmit->isVanishing)
            rgf << size_t(rmit->index) + 1 << " ";
    }
    rgf << "\n\n";

    // Deterministic Sets
    ostringstream ds;
    size_t numDetUsed = 0;
    for (size_t d = 0; d < pn.detTrnInds.size(); d++) {
        const Transition &detTrn = pn.trns[pn.detTrnInds[d]];
        size_t detSetSize = 0;
        for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
            if (rmit->enabledDetInds.size() > 0 && !rmit->isVanishing) {
                set<transition_t>::const_iterator trnIt = rmit->enabledDetInds.begin();
                for (; trnIt != rmit->enabledDetInds.end(); ++trnIt)
                    if (*trnIt == detTrn.index)
                        detSetSize++;
            }
        }

        if (detSetSize == 0)
            continue; // This Deterministic transition is never used

        // Add detTrn to the Deterministic Transition Sets
        numDetUsed++;
        if (detTrn.delayFn->IsMarkingDep())
            throw program_exception("Marking-dependent Deterministic delay is not supported.");

        // Transition distribution description
        ds << "  " << size_t(detTrn.index) + 1 << " constant ";
        ds << detTrn.delayFn->EvaluateConstantDelay() << "\n";

        // Markings in which detTrn is enabled
        ds << "  " << detSetSize << ": ";
        for (rmit = rg.markSet.begin(); rmit != rg.markSet.end(); ++rmit) {
            if (rmit->enabledDetInds.size() > 0 && !rmit->isVanishing)
                if (exists(rmit->enabledDetInds.begin(),
                           rmit->enabledDetInds.end(), detTrn.index)) {
                    ds << size_t(detTrn.index) + 1 << " ";
                }
        }
        ds << "\n";
    }

    // Save the MRP Deterministic Sets
    rgf << "# Tangible states in which a deterministic transition (transition,distrib,delay) is enabled:\n";
    rgf << "  " << numDetUsed << "\n";
    rgf << ds.str() << "\n";

    // Save Measures
    rgf << "# Measures (expr \\n rateReward):\n";
    rgf << "  " << rg.ppMeasures.size() << "\n";
    for (size_t m = 0; m < rg.ppMeasures.size(); m++) {
        rgf << "  " << rg.ppMeasures[m].name << "\n  ";
        rg.ppMeasures[m].expr->Print(rgf, false);
        rgf << "\n";
    }
    rgf << "\n";

    // Save the initial probability vector
    rgf << "# Initial probability vector:\n  [" << numMarks << "](";
    vector<double> pi0(numMarks, 0.0);
    for (size_t i = 0; i < rg.initMark.timedPaths.size(); i++)
        pi0[rg.initMark.timedPaths[i].destMarking] = rg.initMark.timedPaths[i].prob;
    for (size_t i = 0; i < numMarks; i++)
        rgf << (i > 0 ? "," : "") << pi0[i];
    rgf << ")\n\n";
}

//=============================================================================







