#include "lista-adt.h"

status allocate_node(list *p_L, generic_ptr data) {
    list L = (list) malloc(sizeof(node));

    if (L == NULL) return ERROR;

    *p_L = L;
    DATA(L) = data;
    NEXT(L) = NULL;
    return OK;
}


void free_node(list *p_L) {
    free(*p_L);
    *p_L = NULL;
}


status init_list(list *p_L) {
    *p_L = NULL;
    return OK;
}


bool empty_list(list L) {
    return (L == NULL) ? TRUE : FALSE;
}


status head_insert(list *p_L, generic_ptr data) {
    list L;

    if (allocate_node(&L, data) == ERROR) return ERROR;
    NEXT(L) = *p_L;
    *p_L = L;
    return OK;
}


status append(list *p_L, generic_ptr data) {
    list L;

    if (allocate_node(&L, data) == ERROR) return ERROR;

    if (empty_list(*p_L) == TRUE)
        *p_L = L;
    else {
        list tmplist;
        for (tmplist = *p_L; NEXT(tmplist) != NULL; tmplist = NEXT(tmplist));
        NEXT(tmplist) = L;
    }
    return OK;
}


status delete_node(list *p_L, list node) {
    if (empty_list(*p_L))
        return ERROR;

    if (*p_L == node)
        *p_L = NEXT(*p_L);
    else {
        list L;
        for (L = *p_L; L != NULL && NEXT(L) != node; L = NEXT(L));

        if (L == NULL)
            return ERROR;
        else
            NEXT(L) = NEXT(node);
    }
    free_node(&node);
    return OK;
}


status head_delete(list *p_L, generic_ptr *p_data) {
    if (empty_list(*p_L))
        return ERROR;

    *p_data = DATA(*p_L);
    return delete_node(p_L, *p_L);
}



status traverse(list L, status(*p_func_f)()) {
    if (empty_list(L))
        return OK;

    if ((*p_func_f)(DATA(L)) == ERROR)
        return ERROR;
    else
        return traverse(NEXT(L), p_func_f);
}



status reverse(list *p_L) {
    list prec, succ;

    prec = NULL;

    while (*p_L != NULL) {
        succ = NEXT(*p_L);
        NEXT(*p_L) = prec;
        prec = *p_L;
        *p_L = succ;
    }

    *p_L = prec;
    return (OK);
}

list list_iterator(list L, list lastreturn) {
    return (lastreturn == NULL) ? L : NEXT(lastreturn);
}



status find_key(list L, generic_ptr key, bool (*p_cmp_f)(), list *p_keynode) {
    list curr = NULL;

    while ((curr = list_iterator(L, curr)) != NULL) {


        if ((*p_cmp_f)(key, DATA(curr)) == TRUE) {
            *p_keynode = curr;
            return OK;
        }
    }
    return ERROR;
}



status destroy(list *p_L, void (*p_func_f)()) {
    if (empty_list(*p_L) == FALSE) {
        destroy(&NEXT(*p_L), p_func_f);
        if (p_func_f != NULL)
            (*p_func_f)(DATA(*p_L));
        free_node(p_L);
    }
    return OK;
}
