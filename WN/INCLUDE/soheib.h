//Data structures requires by an efficient implementation of  Paige and Tarjan's algorithm with arcs weight

//***************//
//MACRO
//**************//
#define OK 0
#define ERROR -1
#define TRUE 1
#define FALSE 0
#define AGAIN -1
#define MAX 10000000
#define E_B 1
#define MAXSTRING 100
#define MACROELEM 1
#define ELEM 2
#define COLORDEPENDENT 1
#define ZERO 0
//***************//
//MACRO
//**************//



//***************************************************************************************//
//Data structures//
//****************************************************************************************//



/***** ARC LIST *****/

struct L_P_NODO {
    double weight;//rate or prob (or generic).
    int id_trans;//id transition
    int col;
    struct L_NODO *nodo; //dest. nodo (yEx,generic) or source nodo (prim)
    struct P_ELEM *next; //next arc
};
typedef struct L_P_NODO *l_p_nodo;
/***** ARC LIST *****/



//***LIST OF THE ELEMENT (ENVENTUALITIES OR SIMBOLIC RAPPRESENTATION)***/
struct L_NODO {
    int id; //id nodo
    int type; // type nodo (istance,macroc,macrop)
    l_p_nodo prim; // in arc
    l_p_nodo yEx; //out arc
    l_p_nodo generic //generic arc
    struct SET *part_q;  //nodo points to the block of Q containing it
    int count; // count(x,U)={|E({x})|}
    double weight;//it only uses  computing E^-1(B)
    struct L_NODO *next; //next element
};
typedef struct L_NODO *list_nodo;
//***LIST OF THE ELEMENT (ENVENTUALITIES OR SIMBOLIC RAPPRESENTATION)***/



/***** LIST OF NODE IN EACH BLOCK OF Q*****/
struct NODE_SET {
    int count;
    list_nodo p_nodo;//nodo
    struct NODE_SET *next; //next nodo
    struct NODE_SET *prev; //ptrev nodo
};
typedef struct ELEM_SET elem_set;
/***** LIST OF NODE IN EACH BLOCK OF Q*****/



/***** EACH BLOCK OF X  POINTS TO  THE BLOCKS OF Q CONTAINING IN IT*****/
struct L_SUBSET {
    struct SET *p_subset; // block of Q
    struct L_SUBSET *next; // next block
};
typedef struct L_SUBSET l_subset;
/***** EACH BLOCK OF X  POINTS TO  THE BLOCKS OF Q CONTAINING IN IT*****/



/***** C LIST OF COMPOUND BLOCKS OF  X*****/
struct C_SET {
    struct SET *p_set; //compound block of X
    struct C_SET *next; //next block
};
typedef struct C_SET *c_l_setx;
/***** LIST OF COMPOUND BLOCKS OF  X*****/



/***** PARTITION X or Q*****/
struct SET {
    int id; //id
    int num_elem; //number of elements (Q)
    int num_subset;//number of subsets (X)
    elem_set *l_elem_set;  //list of nodes (Q)
    struct SET *p_setX; // block of X containing the block of Q (Q)
    l_subset *subset;  //list of subset (X)
    struct SET *next; //next block
    struct SET *prev; //prev block
};
typedef struct SET *set; *
/***** PARTITION X or Q*****/



///***************************************************************************************//
//Data structures//
//****************************************************************************************//
