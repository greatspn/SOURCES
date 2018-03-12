#include <stdio.h>
#include "bdd.h"
#include "CONS.h"

extern int  TEST_GROUPING();
extern int  GROUP_VALIDE();

typedef struct liste {
    //int Usd;
    int Pos;
    struct liste *nxt;
} LIST;


typedef  LIST *PLIST;
typedef  bdd  *Pbdd;
/*
typedef struct Array{
  PLIST Head,Curr;
}PARRAY
*/
PLIST   INDEX_MAT[VAL_NUMB];
int    *ELEMENTS = NULL ;
int     END_INITIAL = 0;
int     LaTable[STR_SIZE];
int     flag[PLACES];
int     NB_ENTRIES = 0;
int     END_FILE_POS = 0;
int     NB_GROUP = 0;
int     MAX_GROUP = 0;
int     MAX_SSBC = 4;
int     indice = 0;
FILE   *f = NULL;

int     FIRST = true;
char   *Str1 = NULL;
char   *Str2 = NULL;
char   *Ptr_Cur = NULL;
int     Lim_Pos = 0;
TYPE   *VERIFIED;
TYPE   *GROUP1;
TYPE   *INITIAL;
Pbdd    marq1, marq2;

TYPE *ALLOCATION() { return (TYPE *)calloc(MAX_IND, SIZE_TYPE); }

void VideC(TYPE *CONJ) {
    int i; for (i = 0; i < MAX_IND; i++) CONJ[i] = 0;
}

void Add(TYPE *CONJ, TYPE e) {
    CONJ[IND(e)] |= (1 << POS(e));
}

void XOR(TYPE *CONJ, TYPE e) {
    CONJ[IND(e)] ^= (1 << POS(e));
}

void InitC(TYPE *CONJ, int Max_ind) {
    int i;
    for (i = 1; i <= Max_ind; i++) Add(CONJ, i);
}

int COMP(TYPE *CONJ1, TYPE *CONJ2) {
    if (memcmp(CONJ1, CONJ2, MAX_IND) == 0) return true;
    else return false;
}

TYPE Val(TYPE *CONJ, TYPE e) {
    TYPE mask = 0;
    mask = (1 << POS(e));
    return (CONJ[IND(e)] & mask);
}


void ADD_LIST(int ele , int entry)

{
    PLIST Ptr;

    Ptr = (PLIST)malloc(sizeof(LIST));
    Ptr->Pos = ele;
    //  Ptr->Usd=false;
    Ptr->nxt = INDEX_MAT[entry];
    INDEX_MAT[entry] = Ptr;

}

void FREE_LIST() {
    PLIST ptr, ptr1;
    int i;
    for (i = 0; i < MAX_SSBC; i++) {
        ptr = INDEX_MAT[i];
        while (ptr) {
            ptr1 = ptr->nxt;
            free(ptr);
            ptr = ptr1;
        }
    }
}

int INIT() {
    char c;

    c = fgetc(f);

    while (c != EOF) {
        if (c == '[') {
            LaTable[NB_ENTRIES] = ftell(f) - 1 ;
            ADD_LIST(NB_ENTRIES, 0);
            NB_ENTRIES++;
        }
        c = fgetc(f);
    }

    END_INITIAL = NB_ENTRIES - 1;
    END_FILE_POS = ftell(f);

}

int GROUPING(int i , int j) {
    int k, l;

    fseek(f, LaTable[i], 0);
    memset(Str1, '\0', STR_SIZE);

    if (i == NB_ENTRIES - 1)
        fread(Str1, (END_FILE_POS - LaTable[i])*sizeof(char), 1, f);
    else fread(Str1, (LaTable[i + 1] - LaTable[i])*sizeof(char), 1, f);


    fseek(f, LaTable[j], 0);
    memset(Str2, '\0', STR_SIZE);

    if (j == NB_ENTRIES - 1)
        fread(Str2, (END_FILE_POS - LaTable[j])*sizeof(char), 1, f);
    else fread(Str2, (LaTable[j + 1] - LaTable[j])*sizeof(char), 1, f);

    Str1 = strcat(Str1, Str2);

    Ptr_Cur = Str1;
    Lim_Pos = (int)Str1 + strlen(Str1);

    k = TEST_GROUPING();

    if (k != NO_GROUP) {
        LaTable[NB_ENTRIES] = k;
        if (NB_GROUP > MAX_GROUP) MAX_GROUP = NB_GROUP;
        ADD_LIST(NB_ENTRIES, NB_GROUP - 1);
        NB_ENTRIES++;

    }

    return k;
}

void TREE_CONSTRUCTION(int i, int j) {
    int k = 0;


    if (j != END_INITIAL) {
        TREE_CONSTRUCTION(i, j + 1);

        if (flag[j] == true) {
            flag[j] = false;
            TREE_CONSTRUCTION(j, j + 1);
        }

        if ((k = GROUPING(i, j)) != NO_GROUP)

            TREE_CONSTRUCTION(NB_ENTRIES - 1, j + 1);

    }

    else   GROUPING(i, j);

}

int GROUP_CHOICE(PLIST ele) {
    int i;

    memset(Str1, '\0', STR_SIZE);

    fseek(f, LaTable[ele->Pos], 0);

    if (ele->Pos == NB_ENTRIES - 1)
        fread(Str1, (END_FILE_POS - LaTable[ele->Pos])*sizeof(char), 1, f);
    else fread(Str1, (LaTable[(ele->Pos) + 1] - LaTable[ele->Pos])*sizeof(char), 1, f);

    Str2[indice] = '('; indice++;
    i = 2; while (Str1[i] != ')') {Str2[indice] = Str1[i]; i++; indice++;}
    Str2[indice] = ')'; indice++;




}

int SEARCH_VALIDE_GROUP(int Max_Group, int ToGroup, PLIST NEXT) {
    int   Grouped;
    PLIST Ptr;
    int   find = false;
    int   ind, k;

    Grouped = min(Max_Group, ToGroup);

    if (!NEXT) Ptr = INDEX_MAT[Grouped - 1];
    else Ptr = NEXT;

    while ((Ptr) && (!find)) {

        ind = indice;
        GROUP_CHOICE(Ptr);

        if (ToGroup - Grouped == 0) {

            Ptr_Cur = Str2;
            Lim_Pos = (int)Str2 + strlen(Str2);
            if (GROUP_VALIDE() == true) {find = true;    printf("le meilleur regroupement est :%s \n", Str2); getchar();}

        }
        else if (min(Max_Group, ToGroup - Grouped) == Grouped) find = SEARCH_VALIDE_GROUP(Max_Group, ToGroup - Grouped, Ptr->nxt);
        else find = SEARCH_VALIDE_GROUP(Max_Group, ToGroup - Grouped, NULL);

        if (!find)  {

            for (k = ind; k < indice; k++) Str2[k] = '\0';
            indice = ind;
            printf("Str2:%s", Str2); getchar();
        }

        Ptr = Ptr->nxt;
    }

    if ((!find) && (Max_Group - 1 >= 0)) find = SEARCH_VALIDE_GROUP(Max_Group - 1, ToGroup, NULL) ;

    return find;
}


int main() {
    int i;

    Str1 = (char *)malloc(STR_SIZE * sizeof(char));
    Str2 = (char *)malloc(STR_SIZE * sizeof(char));
    marq1 = (Pbdd)malloc(nodeenum * sizeof(bdd)) ;
    marq2 = (Pbdd)malloc(nodeenum * sizeof(bdd)) ;
    for (i = 0; i < MAX_SSBC; i++) INDEX_MAT[i] = NULL;

    bdd_init(nodeenum, cachesize);
    bdd_setvarnum(VAL_NUMB);
    VERIFIED = ALLOCATION();
    GROUP1  = ALLOCATION();
    INITIAL = ALLOCATION();

    VideC(INITIAL);
    InitC(INITIAL, MAX_SSBC);

    for (i = 0; i < PLACES; i++)flag[i] = true;

    f = fopen("GROUP.txt", "r+");

    INIT();
    TREE_CONSTRUCTION(0, 1);
    memset(Str2, '\0', STR_SIZE);
    SEARCH_VALIDE_GROUP(MAX_GROUP, MAX_SSBC, NULL);

    free(Str1);
    free(Str2);
    free(marq1);
    free(marq2);
    free(VERIFIED);
    free(GROUP1);
    free(INITIAL);
    FREE_LIST();

    fclose(f);

}
