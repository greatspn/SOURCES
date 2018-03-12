#include "generic-def.h"


#ifndef LISTA
#define LISTA

typedef struct node node, *list;

struct node {
    generic_ptr datapointer;
    list next;
};

#define DATA(L) ((L)->datapointer)
#define NEXT(L) ((L)->next)


status allocate_node(list *p_L, generic_ptr data);
void free_node(list *p_L);
status init_list(list *p_L);
bool empty_list(list L);
status head_insert(list *p_L, generic_ptr data);
status append(list *p_L, generic_ptr data);
status delete_node(list *p_L, list node);
status head_delete(list *p_L, generic_ptr *p_data);
status traverse(list L, status(*p_func_f)());
status reverse(list *p_L);
list list_iterator(list L, list lastreturn);
status find_key(list L, generic_ptr key, bool (*p_cmp_f)(), list *p_keynode);
status destroy(list *p_L, void (*p_func_f)());

#endif



