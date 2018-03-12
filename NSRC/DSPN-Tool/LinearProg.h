/*
 *  Semiflows.h
 *
 *  Computation of minimal P/T-semiflows
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __LINEAR_PROG_H__
#define __LINEAR_PROG_H__
//=============================================================================

// Place bound determined using ILP
#define ILPBND_UNBOUNDED   (ssize_t(-1))
#define ILPBND_UNKNOWN     (ssize_t(-2))
#define ILPBND_SKIPPED     (ssize_t(-3))

typedef std::vector<ssize_t> place_ilp_bounds_t;

// Compute bounds using the Silva-Colom88 algorithm.
// If opt_bnds!=nullptr, the already known bounds are skipped
void ComputeILPBounds(const PN& pn, const place_bounds_t* opt_bnds,
				      place_ilp_bounds_t& bounds, VerboseLevel verboseLvl);

void SaveILPBounds(const place_ilp_bounds_t& bounds, ofstream& file);

void UpperBoundsMCC(const PN& pn, VerboseLevel verboseLvl, ifstream& UB);

//=============================================================================
#endif // __LINEAR_PROG_H__
