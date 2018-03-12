#ifndef __DMCGSPN_LIB_H__
#define __DMCGSPN_LIB_H__

#include <stdlib.h>
#include <stdio.h>



/* Header file defining functions for DMC API */


#ifdef __cplusplus
extern "C" {
#endif
//#include "struct.h"

// Do not include "struct.h", we only need the Tree_p definition.
// (struct.h will include const.h which is different for each
// mode of compilation: sg, srg, ...).
#ifndef __GSPN_STRUCT_H__
typedef struct TREE *Tree_p;
#endif


/* -------------- Interface type definitions -------------- */
typedef struct dmcState {
    char *data;
    size_t length;
    size_t d_ptr;
} dmcState;

typedef struct dmcState *pdmcState;

/******** Utilities to manipulate dmcState */
/// a constructor
pdmcState dmcStateNew(char *data, size_t d_ptr, size_t length);
/// deep copy of a dmcState
pdmcState dmcStateClone(const pdmcState s) ;
///  deallocate a dmcState
void dmcStateFree(pdmcState s);
/// compare two dmcState
int dmcStateEq(const pdmcState a, const pdmcState b);


/************ STATE SPACE CONSTRUCTION */

/* Initialize data structures to work with model and properties
   MANDATORY : should be called before any other function in this library
   Non 0 return correspond to errors in initialization
   Call with arguments similar to WNRG/WNSRG/WNERG

   i.e. : First argument should be model name no extension
   initialize(1,["model"]);
*/
int dmcInitialize(int argc, char **argv) ;
/* Close and cleanup after using the library */
int dmcFinalize();

/* Prints the state "s" specified to "fd"
NB : also sets current state to s !!
 */
extern int dmc_print_state(const pdmcState s, char **st);

/** returns the number of concrete states associated to a symbolic state */
extern long dmc_concrete_state_count(const pdmcState s) ;


/// set current state
extern void dmc_set_state(const pdmcState s, int force);

/// returns initial marking
extern int dmc_initial_state(pdmcState *M0);

/* Calculates successors of a state "s"

   Each successor is returned in an array of pdmcState
   NB : "succ" vector is allocated in the function, use succ_free for memory release
*/
extern int dmc_succ(const pdmcState s,
                    pdmcState *succ, size_t *succ_size) ;

// call me to get rid of succ tab
void dmc_succ_free(pdmcState *tab);

#ifdef __cplusplus
}
#endif


#endif
