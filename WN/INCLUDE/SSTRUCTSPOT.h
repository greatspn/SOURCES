#ifndef _STRUCTSPOT_H
#define _STRUCTSPOT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct Token_Domains {
    int *Token;
    int *Domain;
    int NbCl;
} Token_Domains;

typedef struct CTOKEN {

    int mult;
    Token_Domains tok_dom;
    struct CTOKEN *next;
} CToken;
typedef CToken *CToken_p ;

typedef struct Class {
    TYPE Cl;
    TYPE Card;
    TYPE_P Set;

} CLASS;
typedef CLASS *CLASS_P;

typedef struct Inter {
    TYPE_P Set;
    TYPE_P Indx;
} INTERS;
typedef INTERS *INTERS_P;

struct TREE_NODE {
    int op;
    int type;
    CToken_p FONCT;
    TYPE_P **PART_MAT;
    TYPE_P NbElPM;
    struct TREE_NODE *filsg;
    struct TREE_NODE *filsd;

};
typedef struct TREE_NODE *TreeNode_p;

typedef struct Prop_Atom {
    char *id;
    TreeNode_p prop;
} PropAtomic;

typedef struct Part_Mat {
    TYPE_P **PART_MAT;
    TYPE_P NbElPM;
    int Ref;
} PART_MAT;
typedef struct Part_Mat *PART_MAT_P;

typedef struct MarkAsEvent {
    int    ** *Event;
    double rate;
    int ord;
    struct MarkAsEvent *next;
} MarkAsEvent;
typedef MarkAsEvent *MarkAsEventp;

typedef struct Mark_Group {
    int i, j, cl;
    int *indx;
    int **group;
    int *NbGroup;
    TYPE_P **PART_MAT;
    TYPE_P NbElPM;
    MarkAsEventp list;
    struct Mark_Group *next_group;
} MARK_GROUP;
typedef MARK_GROUP *MARK_GROUPp;

typedef struct Support {
    TYPE_P ** *Supp;
    int **Nds   ;
} SUPPORT;
typedef SUPPORT *PSUPPORT ;

typedef struct StringPart {
    char *part;
    int size;
} StrPrt;
typedef   StrPrt *StrPrt_P;


typedef struct Perf_Index {
    int pt_index;
    int type;
    int op_type;
    TreeNode_p cond;
    double mean;
    int sum;
} PerfIndex  ;

typedef  PerfIndex *PerfIndex_P;

#endif
