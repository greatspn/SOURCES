#ifndef __refDSRG__
#define __refDSRG__

#include <stdio.h>
#include <stdlib.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/service.h"
#include "../../../INCLUDE/gspnlib.h"

#undef  NORMAL


#define true 1
#define false 0
#define Unk   -1
// Strutures used for the refinement
// of a symbolic marking.
typedef struct TREE_ESRG  Decomp;
typedef struct TREE_ESRG *pDecomp;

// Structure used for
// the represention of an
// arc.
struct dtmc_node;
typedef struct dtmc_arc {
    int    ev;
    double pb;
    int    inc;
    int    treated;
    struct dtmc_node *sr;
    struct dtmc_node *ds;
} DtmcA;
typedef DtmcA *pDtmcA;


// Structure used for
// the representation of
// a node.
typedef struct dtmc_node {
    int  st;  unsigned long fstpos;
    int  sr;  unsigned long fsrpos;
    int ord;
    pDtmcA *in;  int nbin;
    pDtmcA *out;  int nbout;
    struct dtmc_node *nxt_list;

    // these fields are used
    // during the refinement
    struct dtmc_node *nxt_inc;
    pDecomp   cur;
    int       nbcur;
    double   *gl_out;
    pDtmcA  **inforcur;
    int      *nbinforcur;
    int treated;
    int first;
} DtmcN;
typedef struct dtmc_node *pDtmcN;


#endif
