#include "SWN-types.h"


typedef struct {
    short value;
    char *str_index;
} ms_elementTYPE;

#define VALUE(T) ((T).value)
#define STR_INDEX(T) ((T).str_index)

typedef  struct {
    int cardinality;
    DomainObjPTR dominio;
    ms_elementTYPE *p_table;
} multiset;
typedef multiset *multisetPTR;

#define CARD(T) ((T)->cardinality)
#define SET(T) ((T)->p_table)
#define DOM(T) ((T)->dominio)

status alloc_multiset(multisetPTR *p_T);
status create_multiset(DomainObjPTR dom, multisetPTR *p_T);
status copymultiset(multisetPTR p_source, multisetPTR p_dest);
void printMultiset(multisetPTR p_T);
status destroy_multiset(multisetPTR p_T);
status init_multiset(multisetPTR p_T, ms_elementTYPE *table);
int position(list val, multisetPTR p_T);
char *index_elements(int pos, multisetPTR p_T);
status cartesian_product(multisetPTR p_A, multisetPTR p_B, multisetPTR *p_C);
status scalar_product(multisetPTR p_A, int num);
status sum(multisetPTR p_A, multisetPTR p_B, multisetPTR p_C);

