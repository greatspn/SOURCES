#include <stdlib.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"

extern void *STATICSBC_ALLOCATION();
extern char *GetObjFrInd();
extern int GetIndObj();
extern int INTER_Truth();

void FreeStTabc() {
    Obj_p ptr, next;
    int i = 0, j;
    for (i = 0; i < ncl; i++) {
        for (j = 0; j < tabc[i].sbc_num; j++) {
            ptr = tabc[i].sbclist[j].obj_list;
            while (ptr) {
                next = ptr->next;
                free(ptr->nome);

                free(ptr);
                ptr = next;
            }
            if (tabc[i].sbclist[j].name)
                free(tabc[i].sbclist[j].name);
            if (tabc[i].sbclist[j].obj_name)
                free(tabc[i].sbclist[j].obj_name);
        }
        free(tabc[i].sbclist);
    }
}
void FreeStoreStructs(STORE_STATICS_ARRAY tab) {
    Obj_p ptr, next;
    int i, j;
    if (tab) {
        for (i = 0; i < ncl; i++) {
            for (j = 0; j < tab[i].sbc_num; j++) {
                ptr = tab[i].ptr[j].obj_list;
                while (ptr) {
                    next = ptr->next;
                    free(ptr->nome);
                    free(ptr);
                    ptr = next;
                }
                if (tab[i].ptr[j].name)
                    free(tab[i].ptr[j].name);
                if (tab[i].ptr[j].obj_name)
                    free(tab[i].ptr[j].obj_name);
            }
            free(tab[i].ptr);
        }
        free(tab);
    }
}

void FreeMerg(TO_MERGEP *merg) {
    int cl;
    TO_MERGEP ptr, next;
    if (merg) {
        for (cl = 0; cl < ncl; cl++) {
            ptr = merg[cl];
            while (ptr) {
                next = ptr->next;
                free(ptr->SSCs);
                free(ptr);
                ptr = next;
            }
        }
        free(merg);
    }
}

/* STORE_STATICS_ARRAY NewTabc(TYPE_P** PM,TYPE_P NbElPM,char** MTCL ) */
/* { */
/*   int offset=0,v,i,j,card; */
/*   Obj_p* Objlist=NULL; */
/*   int ptr,ptr1; */
/*   const int SIZE=256; */
/*   char* tmp=NULL; */
/*   STORE_STATICS_ARRAY tabc=calloc(ncl,sizeof(struct  STORE_STATICs)); */

/*   for(i=0;i<ncl;i++) */
/*     { */
/*       offset=0; */
/*       tabc[i].ptr=(Static_p)STATICSBC_ALLOCATION( NbElPM[i]); */
/*       tabc[i].sbc_num=NbElPM[i]; */
/*       for(j=0;j<tabc[i].sbc_num;j++) */
/* 	{ */
/* 	  tabc[i].ptr[j].obj_name=NULL; */
/* 	  tabc[i].ptr[j].dynlist=NULL; */
/* 	  tabc[i].ptr[j].offset=offset; */
/*           card=0; */

/* 	  tmp=(char*)calloc(SIZE,sizeof(char));tmp[0]='{'; */
/* 	  Objlist=&(tabc[i].ptr[j].obj_list); */
/* 	  for(v=1;v<=BIT_TYPE*MAX_IND;v++) */
/* 	    if(Val(PM[i][j],v)!=0) */
/* 	      { */

/* 	        (*Objlist)=(Obj_p)malloc(sizeof(struct OBJ )); */
/* 		(*Objlist)->next=NULL; */
/* 		(*Objlist)->nome=strdup(GetObjFrInd(v-1,MTCL, i)); */
/* 		strcat(tmp,(*Objlist)->nome); */
/* 		Objlist=&((*Objlist)->next); */
/* 		offset++;card++; */

/* 	      } */
/* 	  strcat(tmp,"}\0"); */

/* 	  tabc[i].ptr[j].name=tmp; */
/* 	  tabc[i].ptr[j].card=card; */

/* 	} */
/*     } */
/*   return tabc; */
/* } */


TO_MERGEP *ToMergArr(TYPE_P **PMDis, TYPE_P NbElDis, TYPE_P **PMCur, TYPE_P NbElCur) {
    int i, j, k;

    TO_MERGEP *merg = NULL;
    TO_MERGEP var = NULL, tmpp;


    merg = (TO_MERGEP *)calloc(ncl, sizeof(TO_MERGEP));

    for (i = 0; i < ncl; i++) {
        tmpp = NULL;
        for (j = 0; j < NbElCur[i]; j++) {
            var = (TO_MERGEP)malloc(sizeof(struct TO_MERGE));
            var->sbc_num = j;
            var->NB = 0;
            var->SSCs = NULL;
            var->next = NULL;

            if (tmpp == NULL) {
                merg[i] = tmpp = var;
                var->prev = NULL;
            }
            else {
                tmpp->next = var;
                var->prev = tmpp;
                tmpp = var;
            }

            for (k = 0; k < NbElDis[i] ; k++)
                if (INTER_Truth(PMDis[i][k], PMCur[i][j]) == true) {
                    var->SSCs = (int *)realloc(var->SSCs, (var->NB + 1) * sizeof(int));
                    var->SSCs[var->NB] = k;
                    var->NB++;

                }

        }
    }
    return merg;
}





