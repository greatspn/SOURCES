#include "SWN-types.h"


/**************************************************************/
/* NAME :            NewClassElement                          */
/* DESCRIPTION :     create a new element of a class          */
/* PARAMETERS :      name of element                          */
/* RETURN VALUE :    pointer to created element               */
/**************************************************************/
Class_elementPTR NewClassElement(char *name) {
    Class_elementPTR e;

    e = (Class_elementPTR) Ecalloc(1, sizeof(Class_elementTYPE));
    e->name = (char *) Estrdup(name);

    return (e);
}



/**************************************************************/
/* NAME :            NewClass                                 */
/* DESCRIPTION :     create a new class                       */
/* PARAMETERS :      name of class                            */
/*                   type                                     */
/*                   list of subclasses                       */
/*		     list of elements                         */
/*                   number of elements                       */
/* RETURN VALUE :    pointer to created class                 */
/**************************************************************/
ClassObjPTR NewClass(char *name, enum ClassTypes type, list sub_classes, list elements, int num_el) {
    ClassObjPTR c;
    int i;
    list curr = NULL;


    c = (ClassObjPTR)Ecalloc(1, sizeof(ClassObjTYPE));
    c->name = (char *) Estrdup(name);
    c->type = type;
    c->sub_classes = sub_classes;
    c->num_el = num_el;
    c->min_idx = 0;
    c->max_idx = num_el;
    c->elements = (Class_elementPTR *) Ecalloc(c->num_el, sizeof(Class_elementPTR));
    i = 0;
    while ((curr = list_iterator(elements, curr)) != NULL)
        c->elements[i++] = DATA(curr);

    return (c);
}


/**************************************************************/
/* NAME : NewUnoloredClass                                    */
/* DESCRIPTION :        create a new uncolored class          */
/*                      with only one element                 */
/* PARAMETERS :         name                                  */
/* RETURN VALUE :       pointer to created class              */
/**************************************************************/
ClassObjPTR NewUncoloredClass(char *name) {
    ClassObjPTR c;
    Class_elementPTR e;


    c = (ClassObjPTR)Ecalloc(1, sizeof(ClassObjTYPE));
    c->name = (char *) Estrdup(name);
    c->type = UNCOLORED_CLASS_TYPE;
    c->sub_classes = NULL;
    c->min_idx = 0;
    c->max_idx = c->num_el = 1;

    c->elements = (Class_elementPTR *) Ecalloc(c->num_el, sizeof(Class_elementPTR));
    e = NewClassElement("");
    c->elements[0] = e;

    return (c);
}



/**************************************************************/
/* NAME : CmpClassName                                        */
/* DESCRIPTION : compare a string and a class                 */
/* PARAMETERS  : name                                         */
/*               c class                                      */
/* RETURN VALUE : TRUE if name equal class name,              */
/*                FALSE otherwise                             */
/**************************************************************/
bool CmpClassName(char *name, ClassObjPTR c) {

    if (strcmp(name, c->name) == 0)
        return (TRUE);
    else
        return (FALSE);
}


bool inSubClass(int *p_index, ClassObjPTR c) {
    int index;

    index = *((int *) p_index);
    if (index >= c->min_idx && index < c->max_idx)
        return (TRUE);
    else
        return (FALSE);
}



/**************************************************************/
/* NAME :            NewDomain                                */
/* DESCRIPTION :     create a new domain                      */
/* PARAMETERS :      name of place associated with domain     */
/*                   list of classes of domain                */
/*                   number of class                          */
/* RETURN VALUE :    pointer to created domain                */
/**************************************************************/
DomainObjPTR NewDomain(char *place_name, list class_list, int num_el) {
    DomainObjPTR d;

    d = (DomainObjPTR)Ecalloc(1, sizeof(DomainObjTYPE));
    d->place_name = (char *) Estrdup(place_name);
    d->class_list = class_list;
    d->num_el = num_el;
    d->create_place = (set *) Emalloc(sizeof(set));
    return (d);
}


/**************************************************************/
/* NAME : NewClassElement */
/* DESCRIPTION : */
/* PARAMETERS : name*/
/* RETURN VALUE : */
/**************************************************************/
void PrintDomain(DomainObjPTR d) {


    printf("Domain name: %s (%d) \n", d->place_name, d->num_el);
//   while((curr= list_iterator(d->class_list, curr))!=NULL)
//     printf("%s ", ((ClassObjPTR)DATA(curr))->name);
//
}


/**************************************************************/
/* NAME : CmpDoamianName                                      */
/* DESCRIPTION : compare a string and a domain                */
/* PARAMETERS  : name                                         */
/*               d domain                                     */
/* RETURN VALUE : TRUE if name equal domain name,             */
/*                FALSE otherwise                             */
/**************************************************************/
bool CmpDomainName(char *name, DomainObjPTR d) {

    if (strcmp(name, d->place_name) == 0)
        return (TRUE);
    else
        return (FALSE);
}

