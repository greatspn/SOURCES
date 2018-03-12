#ifndef __GSPN_LIB_H__
#define __GSPN_LIB_H__

#include <stdlib.h>
#include <stdio.h>


/* Header file defining functions for Spot API */


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

typedef  unsigned long AtomicProp;
typedef AtomicProp *pAtomicProp;
typedef enum  {STATE_PROP, EVENT_PROP} AtomicPropKind;
typedef AtomicPropKind *pAtomicPropKind ;

typedef Tree_p State;
typedef State *pState ;

typedef struct EventPropSucc {
    State s;
    AtomicProp p;
} EventPropSucc ;

typedef struct arc_ident {
    //void* curr_acc_conds;
    int  curr_acc_conds;
    int  curr_state;
} Arc_Ident_;

typedef struct props_ {
    signed char **prop;
    int nb_conj;
    Arc_Ident_ *arc;
} Props_;

typedef struct succ_ {
    State succ_;
    Arc_Ident_ *arc;
} Succ_;

#define EVENT_TRUE -1


/* ----------------- Utility Functions --------------------*/

/* Initialize data structures to work with model and properties
   MANDATORY : should be called before any other function in this library
   Non 0 return correspond to errors in initialization
   Call with arguments similar to WNRG/WNSRG/WNERG

   i.e. : First argument should be model name no extension
   initialize(1,["model"]);
*/
int initialize(int argc, char **argv) ;
/* Close and cleanup after using the library */
int finalize(void);

/* Prints the state "s" specified to "fd"
NB : also sets current state to s !!
 */
//int print_state (const State s, FILE * fd);
int print_state(const State s, char **st);



/* ----------------- Property related Functions ----------------*/

/* Returns the index of the property given as "name"
   Non zero return = error codes
   return = 1 => property not found
*/
int prop_index(const char *name, pAtomicProp  propindex) ;

/* Returns the type of "prop" in "kind"
   non zero return = error codes
*/
int prop_kind(const AtomicProp prop, pAtomicPropKind  kind) ;


/* ------------------ State Space Exploration -----------------*/

/* Returns the identifier of the initial marking state
   Additional information codes :
   0 : no problem
   1 : INITIAL_VANISHING_MARKING
   2 : INITIAL_DEAD_MARKING
*/
int initial_state(pState M0);

/* Given a state "s" and a list of "props_size" property indexes "props" checks the truth value of
   every atomic property in "props" and returns in "truth" the truth value of these properties
   in the same order as the input, ONE CHAR PER TRUTH VALUE (i.e. sizeof(truth[]) = props_size   NB : the vector "truth" is allocated in this function
*/
int satisfy(const State s, const AtomicProp  props [],  unsigned char **truth, size_t props_size);

/* free the "truth" vector allocated by satisfy
   !!! NB: Don't forget to call me, or you will get a memory leak !!!
*/
int satisfy_free(unsigned char *truth);

void MY_INIT_ALLOC();

/* Calculates successors of a state "s"

   In our first implementation enabled
   events is discarded, and ALL successors will be returned.

   Each successor is returned in a struct that gives the Event property verified by the transition
   fired to reach this marking;
   A RIVEDERE
   (   If a marking is reached by firing a transition observed by more than one event property, it will
   be returned in many copies:
   i.e. E1 and E2 observe different firngs of transition t1 ; M1 is reached from M0 by firing t1 with
   a binding observable by both E1 and E2 :
   succ (M0, [E1,E2] , ...)
   will return {[M1,E1],[M1,E2]}
   )
   A RIVEDERE end

   NB : "succ" vector is allocated in the function, use succ_free for memory release
*/
#ifndef ESYMBOLIC
int succ(const State s,
         EventPropSucc **succ, size_t *succ_size);

/* free the "succ" vector allocated by succ
   !!! NB: Don't forget to call me, or you will get a memory leak !!!
*/
int succ_free(EventPropSucc *succ);
#else

int succ(State s, Props_* prop, size_t prop_size, Succ_ **succ, size_t *succ_size);

int succ_free(Succ_ * succ);

int diff_succ_free(State *succ);

int spot_inclusion(State sym_mark1, State sym_mark2);

void Diff_succ(State sym_mark1, State sym_mark2, State **succ, size_t *succ_size);
void inclusion_version();

void *container(const State s);

#endif


#ifdef TYPE_P
extern TYPE_P AllocateBV();
void FreePartMAt(TYPE_P **PM, TYPE_P NbElPM);
// regroup.c
TYPE *ALLOCATION();
void VideC(TYPE *CONJ);
void Add(TYPE *CONJ, TYPE e);
void XOR(TYPE *CONJ, TYPE e);
void InitC(TYPE *CONJ, int Max_ind);
int COMP(TYPE *CONJ1, TYPE *CONJ2);
TYPE Val(TYPE *CONJ, TYPE e);
void ADD_LIST(int ele , int entry);
void FREE_LIST();
int INIT();
int GROUPING(int i , int j);
void TREE_CONSTRUCTION(int i, int j);
// int GROUP_CHOICE(PLIST ele);
// int SEARCH_VALIDE_GROUP(int Max_Group,int ToGroup,PLIST NEXT);

void *STATICSBC_ALLOCATION(int NB);

char *GetObjFrInd(int ind, char *** MTCL, int Cl);
void FreeStoreStructs(STORE_STATICS_ARRAY tab);
void Minus(TYPE_P CONJ, TYPE e);
PART_MAT_P new_part(TYPE_P *** PM, TYPE_P *NbElPM, char **MTCL);
void TreatOrdredClasses(TYPE_P **PM, TYPE_P NbElPM);
#endif

void hash_init();
void cache_init();
void raff_init();

#ifdef ESYMBOLIC
Tree_Esrg_p TREE_EVENT_ALLOCATION(int buff, int length);
#endif

#ifdef __cplusplus
}
#endif


#endif


