/*
 *  MRP_Explicit.h
 *
 *  Steady state solution of MRP with explicit EMC construction.
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __MRP_EXPLICIT_H__
#define __MRP_EXPLICIT_H__
//=============================================================================

void SteadyStateMRPExplicit(const MRP &mrp, const MRPStateKinds &stKinds,
                            PetriNetSolution &sol, const SolverParams &spar,
                            VerboseLevel verboseLvl);

void SteadyStateMRPExplicitBackward(const MRP &mrp, const MRPStateKinds &stKinds,
                                    const ublas::vector<double> &rho,
                                    ublas::vector<double> &xi,
                                    const SolverParams &spar,
                                    VerboseLevel verboseLvl);

//=============================================================================
#endif   // __MRP_EXPLICIT_H__
