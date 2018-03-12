#include "assignment.h"

int compare_variable(void *a, void *b) {
    Assignment_el_PTR El_a, El_b;
    int res;

    El_a = (Assignment_el_PTR) a;
    El_b = (Assignment_el_PTR) b;
    res = strcmp(El_a->var->name, El_b->var->name);
    return (strcmp(El_a->var->name, El_b->var->name));
}




status create_assignment(AssignmentPTR  *gamma, list elements) {
    int num_el, i;
    list curr = NULL;

    num_el = 0;
    while ((curr = list_iterator(elements, curr)) != NULL)
        num_el++;

    *gamma = (AssignmentPTR) Emalloc(sizeof(AssignmentTYPE));
    ELEMENTS(*gamma) = (Assignment_el_PTR *) Ecalloc(num_el, sizeof(Assignment_el_PTR));
    NUM(*gamma) = num_el;

    curr = NULL;
    i = 0;
    while ((curr = list_iterator(elements, curr)) != NULL) {
        ELEMENTS(*gamma)[i] = (Assignment_el_PTR) Emalloc(sizeof(AssignmentTYPE));
        VAR(ELEMENTS(*gamma)[i]) = (VariablePTR) DATA(curr);
        INDEX((ELEMENTS(*gamma)[i++])) = 0;
    }
//   DA SISTEMARE IL SORT !!!!!!!!!!!!

//  qsort((generic_ptr) ELEMENTS(*gamma), num_el, sizeof(Assignment_el_PTR), compare_variable);

    return (OK);
}


char *print_assignment(AssignmentPTR  gamma) {
    int i;
    char *string, *name_var, *name_element;

    string = (char *) Ecalloc(500, sizeof(char));
    for (i = 0; i < NUM(gamma) ; i++) {
        name_var = VAR(ELEMENTS(gamma)[i])->name;
        name_element = (((VAR(ELEMENTS(gamma)[i])->class)->elements)[INDEX(ELEMENTS(gamma)[i])])->name;

        if (i == 0)
            sprintf(string, "%s <-- %s", name_var, name_element);
        else
            sprintf(string, "%s, %s <-- %s", string, name_var, name_element);
    }
    return (string);
}


status first_assignment(AssignmentPTR  gamma) {
    int i;

    i = 0;
    while (i < NUM(gamma))
        INDEX((ELEMENTS(gamma)[i++])) = 0;
    return (OK);
}

status next_assignment(AssignmentPTR gamma) {
    int i, j;
    bool last;

    i = 0;
    last = TRUE;
    while (i < NUM(gamma)) {
        last = (INDEX((ELEMENTS(gamma)[i])) == LAST_ELEMENT_INDEX((VAR((ELEMENTS(gamma)[i])))->class)) && last;
        i++;
    }

    if (last == TRUE)
        return (ERROR);
    else {

        i = 0;
        while ((i < NUM(gamma)) &&
                (INDEX((ELEMENTS(gamma)[i])) == LAST_ELEMENT_INDEX((VAR((ELEMENTS(gamma)[i++])))->class)));

        INDEX((ELEMENTS(gamma)[--i])) += 1;
        j = 0;
        while (j < i)
            INDEX((ELEMENTS(gamma)[j++])) = 0;

        return (OK);
    }
}

status evaluate_variable(AssignmentPTR gamma, char *var_name, Assignment_el_PTR *assign) {
    int i = 0;


    *assign = (Assignment_el_PTR) malloc(sizeof(Assignment_el_TYPE));

    while ((i < NUM(gamma)) &&
            (CmpVarName(var_name, VAR((ELEMENTS(gamma)[i]))) == FALSE))
        i++;
    if (i == NUM(gamma))
        return (ERROR);
    else
        *assign = ELEMENTS(gamma)[i];
    return (OK);
}

