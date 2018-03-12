/*
 *  PetriExport.h
 *
 *  Function for saving the Petri Net or the TRG in various formats.
 *
 *  Created by Elvio Amparore
 *
 */

//=============================================================================
#ifndef __PETRI_EXPORT_H__
#define __PETRI_EXPORT_H__
//=============================================================================

//=============================================================================
//     Function Prototypes
//=============================================================================

void SaveRG_AsDotFile(const PN &pn, const RG &rg,
                      bool drawClusters, ofstream &dot);

void SaveRG_AsDotFile(const PN &pn, const RG2 &rg,
                      bool drawClusters, ofstream &dot,
                      const size_t maxSavedMarkings);

void SaveAsSPNicaPetriNet(const PN &pn, ostream &nbf);

void SaveAsCosmosPetriNet(const PN &pn, const string &NetName, ostream &out);

void SaveAsCsltaASMC(const PN &pn, const RG &rg,
                     const char *name, ofstream &asmc);

void SaveAsCsltaASMC(const PN &pn, const RG2 &rg,
                     const char *name, ofstream &asmc);

void SaveAsPrismCTMC(const PN &pn, const RG &rg, ofstream &ctmc,
                     ofstream &states, const char *pi0distrFileName);

void SaveReachabilityGraph(const PN &pn, const RG &rg, ostream &rgf);

//=============================================================================
#endif   // __PETRI_EXPORT_H__

