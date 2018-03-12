/****** Declaration of colored part of SWN *****/
#include "set-adt.h"
#include "lista-adt.h"
#include "util-transl.h"

#ifndef SWN
#define SWN

// Class Type
enum ClassTypes { ORD_CLASS_TYPE, UNORD_CLASS_TYPE, SUBCLASS_TYPE,
                  DYN_SUBCLASS_TYPE, UNCOLORED_CLASS_TYPE
                };

// Class element Type
typedef struct Class_element_tag {
    char *name;                  /*Element name*/
} Class_elementTYPE;
typedef Class_elementTYPE *Class_elementPTR;


// Class Type
typedef struct Class_tag {
    char *name; /*Class name*/
    Class_elementPTR *elements;  /* Array of Class_elementPTR */
    list sub_classes;            /* List of ClassObjPTR (subclasses of Class)*/
    enum ClassTypes type;        /* Type of Class*/
    int num_el;                  /* Number of element of Class*/
    int min_idx;
    int max_idx;
} ClassObjTYPE;
typedef ClassObjTYPE *ClassObjPTR;



#define UNCOLORED_CLASS_NAME "$_Uncolored";
#define LAST_ELEMENT_INDEX(C) (((C)->num_el)-1)

typedef struct Domain_tag {
    list class_list;             /* List of ClassObjPTR.
	                                Example D = C x C x C, class_list={C,C,C}       */
    int num_el;                  /* Number of Class. Example  num_el = 3            */
    char *place_name;            /* Domain identified by place name of original net */
    set *create_place;           /* created place of unfolded net  */
} DomainObjTYPE;
typedef DomainObjTYPE *DomainObjPTR;



Class_elementPTR NewClassElement(char *name);
ClassObjPTR NewClass(char *name, enum ClassTypes type, list sub_classes, list elements, int num_el);
ClassObjPTR NewUncoloredClass(char *name);
bool CmpClassName(char *name, ClassObjPTR c);
bool inSubClass(int *p_index, ClassObjPTR c);
DomainObjPTR NewDomain(char *place_name, list class_list, int num_el);
void PrintDomain(DomainObjPTR d);
bool CmpDomainName(char *name, DomainObjPTR d);

#endif


