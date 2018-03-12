#include <stdio.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"

#ifdef REACHABILITY
extern Tree_p treenode_pop();

extern Tree_p reached_marking;
extern int cur_priority;

extern char cache_string[];
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void inqueue_stack(Tree_p  *top,  Tree_p  *bottom,  Tree_p  ins_node) {
    /* Init inqueue_stack */
    ins_node->last = NULL;
    if (*top == NULL)
        *top = ins_node;
    else
        (*bottom)->last = ins_node;
    *bottom = ins_node;
}/* End inqueue_stack */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void pop(Tree_p  *top) {
    /* Init pop */
    *top = (*top)->last;
}/* End pop */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int my_strcmp(to_compare_ptr, length_to_compare, to_insert_ptr, length_to_insert)
unsigned long to_compare_ptr;
unsigned long length_to_compare;
unsigned long to_insert_ptr;
unsigned long length_to_insert;
{
    /* Init my_strcmp */
    unsigned long op1;
    unsigned long op2;
    unsigned long ii, min;
    char compare[MAX_CACHE];
    char *pp = cache_string;
    char *cp = compare;

    if (length_to_insert < length_to_compare)
        return (1);
    else if (length_to_insert > length_to_compare)
        return (-1);
    else {
        /*return(0);
          min = MIN(length_to_insert,length_to_compare); */
        fseek(mark, to_compare_ptr, 0);
        fread(compare, 1, length_to_compare, mark);
        for (ii = length_to_insert; ii; ii--) {
            /*load_compact(&op1,string);
            load_compact(&op2,mark);*/
            op1 = *pp;
            pp++;
            op2 = *cp;
            cp++;
            if (op1 < op2)
                return (1);
            if (op1 > op2)
                return (-1);
        }
        return (0);
    }
}/* End my_strcmp */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void insert_tree(Tree_p  *root,  int  *h,  unsigned long  mark_ptr,  unsigned long  ilength,  unsigned long  id_ptr) {
    /* Init insert_tree */
    int marking_pri;
    Tree_p p1;
    Tree_p p2;
    Tree_p ins_node;

    if (*root == NULL) {
        /* Albero vuoto */
        ins_node = treenode_pop(mark_ptr, ilength, id_ptr);
        (*root) = ins_node;
        reached_marking = current_marking = ins_node;
        *h = 1;
        (*root)->left = (*root)->right = NULL;
        (*root)->balance = '0';
        ++marcatura;
#ifdef SYMBOLIC
        (*root)->marking->ordinary = mark_ordinarie;
#endif
        tro = NEW_MARKING;
#ifndef GREATSPN
        fprintf(stdout, " %6d\r", marcatura);
#endif
    }/* Albero vuoto */
    else if (my_strcmp((*root)->marking->marking_as_string, (*root)->marking->length, mark_ptr, ilength) < 0) {
        /* Albero pieno */
        insert_tree(&((*root)->left), h, mark_ptr, ilength, id_ptr);
        if (*h) {
            /* il ramo sinistro e' piu' pesante */
            if ((*root)->balance == '+') {
                (*root)->balance = '0';
                *h = 0;
            }
            else if ((*root)->balance == '0')
                (*root)->balance = '-';
            else {
                /* ribilanciamento */
                p1 = (*root)->left;
                if (p1->balance == '-') {
                    /* singola rotazione LL */
                    (*root)->left = p1->right;
                    p1->right = (*root);
                    (*root)->balance = '0';
                    (*root) = p1;
                } /* singola rotazione LL */
                else {
                    /* doppia rotazione LR */
                    p2 = p1->right;
                    p1->right = p2->left;
                    p2->left = p1;
                    (*root)->left = p2->right;
                    p2->right = (*root);
                    if (p2->balance == '-')
                        (*root)->balance = '+';
                    else
                        (*root)->balance = '0';
                    if (p2->balance == '+')
                        p1->balance = '-';
                    else
                        p1->balance = '0';
                    (*root) = p2;
                } /* doppia rotazione LR */
                (*root)->balance = '0';
                *h = 0;
            }/* ribilanciamento */
        }/* il ramo sinistro e' piu' pesante */
    }/* Albero pieno */
    else if (my_strcmp((*root)->marking->marking_as_string, (*root)->marking->length, mark_ptr, ilength) > 0) {
        /* Albero pieno */
        insert_tree(&((*root)->right), h, mark_ptr, ilength, id_ptr);
        if (*h) {
            /* il ramo destro e' piu' pesante */
            if ((*root)->balance == '-') {
                (*root)->balance = '0';
                *h = 0;
            }
            else if ((*root)->balance == '0')
                (*root)->balance = '+';
            else {
                /* ribilanciamento */
                p1 = (*root)->right;
                if (p1->balance == '+') {
                    /* singola rotazione RR */
                    (*root)->right = p1->left;
                    p1->left = (*root);
                    (*root)->balance = '0';
                    (*root) = p1;
                } /* singola rotazione RR */
                else {
                    /* doppia rotazione RL */
                    p2 = p1->left;
                    p1->left = p2->right;
                    p2->right = p1;
                    (*root)->right = p2->left;
                    p2->left = (*root);
                    if (p2->balance == '+')
                        (*root)->balance = '-';
                    else
                        (*root)->balance = '0';
                    if (p2->balance == '-')
                        p1->balance = '+';
                    else
                        p1->balance = '0';
                    (*root) = p2;
                } /* doppia rotazione RL */
                (*root)->balance = '0';
                *h = 0;
            }/* ribilanciamento */
        }/* il ramo destro e' piu' pesante */
    }/* Albero pieno */
    else {
        reached_marking = current_marking = (*root);
        marking_pri = reached_marking->marking->pri;
        if (IS_VANISHING(marking_pri)) {
            /* Vanishing marking */
            if (reached_marking->enabled_head == NULL)
                tro = VANISHING_OLD;
            else
                tro = VANISHING_LOOP;
        }/* Vanishing marking */
        else if (IS_TANGIBLE(marking_pri)) {
            tro = TANGIBLE_OLD;
            cont_tang = reached_marking->marking->cont_tang;
        }
        else {
            cont_tang = reached_marking->marking->cont_tang;
            tro = DEAD_OLD;
        }
        *h = 0;
    }
}/* End insert_tree */
#endif
