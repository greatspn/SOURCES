#include "multiset-adt.h"


status alloc_multiset(multisetPTR *p_T) {
    *p_T = (multisetPTR) malloc(sizeof(multiset));
    return (OK);
}



status create_multiset(DomainObjPTR dom, multisetPTR *p_T) {
    int dimension = 1;
    list curr = NULL;
    int i;

    while ((curr = list_iterator(dom->class_list, curr)) != NULL)
        dimension = ((ClassObjPTR) DATA(curr))->num_el * dimension;

    *p_T = (multisetPTR) malloc(sizeof(multiset));

    CARD(*p_T) = dimension;
    DOM(*p_T) = dom;
    SET(*p_T) = (ms_elementTYPE *) Ecalloc(CARD(*p_T), sizeof(ms_elementTYPE));
    for (i = 0; i < CARD(*p_T); i++) {
        VALUE(SET(*p_T)[i]) = 0;
        STR_INDEX(SET(*p_T)[i]) = "";
        STR_INDEX(SET(*p_T)[i]) = index_elements(i, *p_T);
//     printf("prr %s \n", STR_INDEX(SET(*p_T)[i]));
    }
//     printf("out create_ms\n");
//     printMultiset(*p_T);
    return (OK);
}



// status copymultiset(multisetPTR p_source, multisetPTR p_dest)
// {
//   int i;
//
//
//   CARD(p_dest) = CARD(p_source);
//   DOM(p_dest) = DOM(p_source);
//   SET(p_dest) = (int *) malloc(CARD(p_source) * sizeof(int));
//   for(i=0; i< CARD(p_source); SET(p_dest)[i] = SET(p_source)[i++] );
//   return(OK);
// }





void printMultiset(multisetPTR p_T) {
    int i;

    printf("Num El %d\n", CARD(p_T));
    for (i = 0; i < CARD(p_T);)
        printf("%d) ", i++);
    printf("\n");

    printf("El \n");
    for (i = 0; i < CARD(p_T);)
        printf("%s ", STR_INDEX(SET(p_T)[i++]));
    printf("\n");


    for (i = 0; i < CARD(p_T);)
        printf("%d ", VALUE(SET(p_T)[i++]));
    printf("\n\n");

}


status destroy_multiset(multisetPTR p_T) {

    free(SET(p_T));
    free(p_T);
    return (OK);
}

status init_multiset(multisetPTR p_T, ms_elementTYPE *table) {
    SET(p_T) = table;
    return (OK);
}



int position(list val, multisetPTR p_T) {
    int temp, pos;
    list curr_val = NULL;
    list curr_dom = NULL;

    temp = CARD(p_T);
    pos = 0;

    while ((curr_val = list_iterator(val, curr_val)) != NULL) {
        curr_dom = list_iterator(DOM(p_T)->class_list, curr_dom);
        temp /= ((ClassObjPTR) DATA(curr_dom))->num_el;
        pos += temp **((int *)DATA(curr_val));
    }
    return (pos);
}

char *index_elements(int pos, multisetPTR p_T) {
    int temp;
    int div_i;
    char *result, *tmp;
    list curr_dom = NULL;
    ClassObjPTR c;

//   printf("in ind_el %d \n", pos);
//   printMultiset(p_T);
    temp = CARD(p_T);
    result = (char *) Ecalloc(20, sizeof(char));
    tmp = (char *) Ecalloc(20, sizeof(char));
    while ((curr_dom = list_iterator(DOM(p_T)->class_list, curr_dom)) != NULL) {
        c = (ClassObjPTR) DATA(curr_dom);
        temp /=  c->num_el;
        div_i = (int) pos / temp;
        sprintf(tmp, "_%s", (c->elements[div_i])->name);
//     printf("Parz res %s\n", tmp);

        result = (char *) NewStringCat(result, tmp);
        pos -= temp * div_i;
    }
//   printf("out ind_el\n");
//   printf("res %s\n", result);
    return (result);
}

status cartesian_product(multisetPTR p_A, multisetPTR p_B, multisetPTR *p_C) {
    int i, j, k;
    list curr = NULL;
    DomainObjPTR d;

//   printf("Fattore1\n");
//   printMultiset(p_A);
//   printf("Fattore2\n");
//   printMultiset(p_B);
//


    d = NewDomain("tmp", NULL, 0);

    create_multiset(d, p_C);

    CARD(*p_C) = CARD(p_A) * CARD(p_B);
    init_list(&DOM(*p_C)->class_list);

    while ((curr = list_iterator(DOM(p_A)->class_list, curr)) != NULL)
        head_insert(&DOM(*p_C)->class_list, DATA(curr));
    while ((curr = list_iterator(DOM(p_B)->class_list, curr)) != NULL)
        head_insert(&DOM(*p_C)->class_list, DATA(curr));
    reverse(&DOM(*p_C)->class_list);

    DOM(*p_C)->num_el = DOM(p_A)->num_el + DOM(p_B)->num_el;

    SET(*p_C) = (ms_elementTYPE *) malloc(CARD(*p_C) * sizeof(ms_elementTYPE));
    k = 0;
    for (i = 0 ; i < CARD(p_A) ; i++)
        for (j = 0 ; j < CARD(p_B) ; j++)
            VALUE(SET(*p_C)[k++]) = VALUE(SET(p_A)[i]) * VALUE(SET(p_B)[j]);

    return (OK);
}

status scalar_product(multisetPTR  p_A, int num) {
    int i;

    for (i = 0 ; i < CARD(p_A) ; i++)
        VALUE(SET(p_A)[i]) *= num;

    return (OK);
}


status sum(multisetPTR p_A, multisetPTR p_B, multisetPTR p_C) {
    int i;


//   printf("Addendo 1 \n");
//   printMultiset(p_A);
//
//   printf("Addendo 2 \n");
//   printMultiset(p_B);

//   Si deve verificare che la somma sia eseguita su due multiset con stesso dominio

    CARD(p_C) = CARD(p_A);
    DOM(p_C) = DOM(p_A);

    SET(p_C) = (ms_elementTYPE *) malloc(CARD(p_C) * sizeof(ms_elementTYPE));
    for (i = 0 ; i < CARD(p_C) ; i++) {
        VALUE(SET(p_C)[i]) = VALUE(SET(p_A)[i]) + VALUE(SET(p_B)[i]);
        if (VALUE(SET(p_C)[i]) < 0) {
            VALUE(SET(p_C)[i]) = 0;
            printf("Warning: negative result\n");
        }
    }
    /*  printf("Risultato \n");
      printMultiset(p_C);*/
    return (OK);
}


