#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "CONS.h"
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"

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
