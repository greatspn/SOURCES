# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"
# include "../../INCLUDE/macros.h"


#ifdef SIMULATION
extern int events_made;
extern void out_cur_simulation_state();
#endif

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_error(int  code,  int  pl,  int  tr,  int  cl,  int  sb,  char  *name1,  char  *name2) {
    /* Init out_error */
    fprintf(stdout, "\nERROR : ");
    switch (code) {
        /* Tipo di errore */
#ifdef SWN
    case ERROR_UNKNOWN_FUNCTION_TYPE :
        fprintf(stdout, "unknown arc type from transition %s to place %s\n",
                TRANS_NAME(tr), PLACE_NAME(pl));
        break;
#endif
    case ERROR_LOW_MAX_TRANS :
        fprintf(stdout, "increase constant MAX_TRANS\n");
        break;
#ifdef SWN
    case ERROR_LOW_MAX_PLACE :
        fprintf(stdout, "increase constant MAX_PLACE_DOMAIN due to place %s\n",
                PLACE_NAME(pl));
        break;
    case ERROR_LOW_MAX_DOMAIN :
        fprintf(stdout, "increase constant MAX_DOMAIN due to transition %s\n",
                TRANS_NAME(tr));
        break;
    case ERROR_LOW_MAX_COL_RIP :
        fprintf(stdout, "increase constant MAX_COL_RIP due to transition %s\n",
                TRANS_NAME(tr));
        break;
#endif
    case ERROR_INITIAL_DEAD_MARKING :
        fprintf(stdout, "initial dead marking !\n");
        break;
    case ERROR_INITIAL_VANISHING_MARKING :
        fprintf(stdout, "initial vanishing marking !\n");
        break;
#ifdef SIMULATION
    case ERROR_DEAD_MARKING :
        fprintf(stdout, "Dead marking reached at simulation step %d\n", events_made);
        out_cur_simulation_state(stdout);
        fprintf(stdout, "Set trace option.\n");
        break;
#endif
#ifdef SIMULATION
    case ERROR_NEGATIVE_MARKING	:
        fprintf(stdout, "negative marking of place %s due to transition %s occurred at step %d\n",
                PLACE_NAME(pl), TRANS_NAME(tr), events_made);
#else
    case ERROR_NEGATIVE_MARKING	:
        fprintf(stdout, "negative marking of place %s due to transition %s \n", PLACE_NAME(pl), TRANS_NAME(tr));
#endif
        break;
    case ERROR_UNKNOWN_PLACE_NAME :
        fprintf(stdout, "unknown place name %s in file .stat\n", name1);
        break;
    case ERROR_UNKNOWN_TRANSITION_NAME :
        fprintf(stdout, "unknown transition name %s in file .stat\n", name1);
        break;
#ifdef SWN
    case ERROR_UNKNOWN_STATIC_SUBCLASS_NAME :
        fprintf(stdout, "unknown static subclass name %s\n", name1);
        break;
    case ERROR_REDECLARATION :
        fprintf(stdout, "redeclaration of static subclass %s of class %s \n",
                name1, GET_CLASS_NAME(cl));
        break;
    case ERROR_INTERSECTION :
        fprintf(stdout, "static subclasses %s and %s of class %s are not disjoint\n",
                name1, name2, GET_CLASS_NAME(cl));
        break;
    case ERROR_INCOMPLETE_DEFINITION :
        fprintf(stdout, "incomplete static subclasses definition\n");
        break;
    case ERROR_UNEXISTING_SUBCLASS_IN_DYNAMIC :
        fprintf(stdout, "static subclass %s does not exist in declaration of dynamic subclass %s\n",
                name1, name2);
        break;
    case ERROR_REDECLARATION_DYNAMIC :
        fprintf(stdout, "dynamic subclass redeclaration (%s) of static subclass %s\n",
                name1, name2);
        break;
    case ERROR_INCONSISTENT_CARDINALITIES :
        fprintf(stdout, "sum of cardinalities of dynamic subclasses of static subclass %s exceeds cardinality of class %s\n",
                GET_STATIC_NAME(cl, sb), GET_CLASS_NAME(cl));
        break;
    case ERROR_REPETITIONS_AND_NO_FUNCTION :
        fprintf(stdout, "color domain of place %s has color repetitions\n",
                PLACE_NAME(pl));
        fprintf(stdout, "but the arc to (from) transition %s has no associated function\n",
                TRANS_NAME(tr));
        break;
    case ERROR_AMBIGUOUS_DEFINITION :
        fprintf(stdout, "ambiguous definition for the arc from place %s to transition %s\n",
                PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_UNKNOWN_PLACE_IN_FUNCTION :
        fprintf(stdout, "unknown place name %s in function from place %s to transition %s\n",
                name1, PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_NEUTRAL_PLACE_AND_FUNCTION :
        fprintf(stdout, "color function on arc from neutral place %s to transition %s\n",
                PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_UNEXISTING_CLASS	:
        fprintf(stdout, "colour class %s does not exist in colour domain of place %s\n",
                name1, PLACE_NAME(pl));
        break;
    case ERROR_CLASS_REDECLARATION :
        fprintf(stdout, "colour class %s has been redeclared\n", name1);
        break;
    case ERROR_TWICE_THE_SAME_STATIC :
        fprintf(stdout, "static subclass %s has been declared twice\n", name1);
        break;
    case ERROR_REDEFINITION :
        fprintf(stdout, "redefinition of static subclass %s of class %s \n",
                GET_STATIC_NAME(cl, sb), GET_CLASS_NAME(cl));
        break;
    case ERROR_OBJECT_DESCRIPTION :
        fprintf(stdout, "incorrect objects description of static subclass %s\n", name1);
        break;
    case ERROR_OBJECT_REDECLARATION :
        fprintf(stdout, "object %s has been redeclared\n", name1);
        break;
    case ERROR_DYNAMIC_REDECLARATION :
        fprintf(stdout, "dynamic subclass %s has been redeclared\n", name1);
        break;
    case ERROR_MISSING_CLASS :
        fprintf(stdout, "colour class %s has not been declared\n", name1);
        break;
    case ERROR_MISSING_STATIC :
        fprintf(stdout, "static subclass %s has not been declared\n", name1);
        break;
    case ERROR_LESSER_MARKING_COMPONENTS :
        fprintf(stdout, "wrong number components (<) for marking of place %s\n",
                PLACE_NAME(pl));
        break;
    case ERROR_GREATER_MARKING_COMPONENTS :
        fprintf(stdout, "wrong number components (>) for marking of place %s\n",
                PLACE_NAME(pl));
        break;
    case ERROR_UNKNOWN_ELEMENT_IN_MARKING :
#ifdef SYMBOLIC
        fprintf(stdout, "unknown dynamic subclass %s in marking of place %s of class %s\n",
                name1, PLACE_NAME(pl), GET_CLASS_NAME(cl));
#endif
#ifdef COLOURED
        fprintf(stdout, "unknown object %s in marking of place %s of class %s\n",
                name1, PLACE_NAME(pl), GET_CLASS_NAME(cl));
#endif
        break;
    case ERROR_LESSER_FUNCTION_COMPONENTS :
        fprintf(stdout, "number components (<) for function of place %s and transition %s\n",
                PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_GREATER_FUNCTION_COMPONENTS :
        fprintf(stdout, "number components (>) for function of place %s and transition %s\n",
                PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_WRONG_STATIC_INDEX_IN_MARKING :
        fprintf(stdout, "out of range index for class %s in marking of place %s\n",
                GET_CLASS_NAME(cl), PLACE_NAME(pl));
        break;
    case ERROR_WRONG_STATIC_NAME_IN_MARKING:
        fprintf(stdout, "unknown static subclass name %s of class %s in marking of place %s\n",
                name1, GET_CLASS_NAME(cl), PLACE_NAME(pl));
        break;
    case ERROR_WRONG_STATIC_INDEX_IN_FUNCTION :
        fprintf(stdout, "out of range index for class %s in function of place %s and transition %s\n",
                GET_CLASS_NAME(cl), PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_WRONG_STATIC_NAME_IN_FUNCTION:
        fprintf(stdout, "unknown static subclass name %s of class %s in function of place %s and transition %s\n",
                name1, GET_CLASS_NAME(cl), PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_AMBIGUOUS_ID :
        fprintf(stdout, "ambiguous use of ID for arc function from place %s to transition %s\n",
                PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_ILLEGAL_PREDECESSOR :
        fprintf(stdout, "illegal use of predecessor for class %s function in function of place %s and transition %s\n" ,
                GET_CLASS_NAME(cl), PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_ILLEGAL_SUCCESSOR :
        fprintf(stdout, "illegal use of successor for class %s function in function of place %s and transition %s\n" ,
                GET_CLASS_NAME(cl), PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_UNKNOWN_STATIC_IN_PREDICATE :
        fprintf(stdout, "unknown static subclass %s in a predicate\n", name1);
        break;
#endif
    case ERROR_NEGATIVE_INITIAL_MARKING	:
        fprintf(stdout, "negative initial marking of place %s\n", PLACE_NAME(pl));
        break;
    case ERROR_MISSING_MDRATE_DEFINITION :
        fprintf(stdout, "marking dependent rate definition of transition %s does not exist\n",
                TRANS_NAME(tr));
        break;
#ifdef SWN
    case ERROR_SHARED_NAME :
        fprintf(stdout, "function name \"%s\" used by different colour classes in function of place %s and transition %s\n",
                name1, PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_AMBIGUITY_IN_ARC:
        fprintf(stdout, "ambiguous identity or empty arc for place %s and transition %s\n",
                PLACE_NAME(pl), TRANS_NAME(tr));
        break;
#endif
    case ERROR_MARKING_CODING:
        fprintf(stdout, "marking of place %s >= 255\n", PLACE_NAME(pl));
        break;
    case ERROR_UNKNOWN_MARKING_PARAMETER :
        fprintf(stdout, "unknown marking parameter \"%s\" in function of place %s and transition %s\n",
                name1, PLACE_NAME(pl), TRANS_NAME(tr));
        break;
    case ERROR_UNKNOWN_PLACE_IN_PREDICATE :
        fprintf(stdout, "unknown place name %s in a predicate\n", name1);
        break;
    default :
        fprintf(stdout, "unknown error code %d\n", code);
        break;
    }/* Tipo di errore */
    exit(1);
}/* End out_error */
