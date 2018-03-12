#include "attrib-yacc.h"


typedef struct Assignment_el_tag {
    VariablePTR var;
    int index;
} Assignment_el_TYPE;
typedef Assignment_el_TYPE *Assignment_el_PTR;

typedef struct Assignment_tag {
    Assignment_el_PTR *elements;
    int num_el;
} AssignmentTYPE;
typedef AssignmentTYPE *AssignmentPTR;

#define VAR(T) ((T)->var)
#define INDEX(T) ((T)->index)
#define ELEMENTS(T) ((T)->elements)
#define NUM(T) ((T)->num_el)


int compare_variable(generic_ptr a, generic_ptr b);
status create_assignment(AssignmentPTR *gamma, list elements);
char  *print_assignment(AssignmentPTR  gamma);
status first_assignment(AssignmentPTR  gamma);
status next_assignment(AssignmentPTR  gamma);
status evaluate_variable(AssignmentPTR gamma, char *var_name, Assignment_el_PTR *assign);



