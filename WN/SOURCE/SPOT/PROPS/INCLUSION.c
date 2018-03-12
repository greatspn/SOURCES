#include <stdlib.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/service.h"
#include "../../../INCLUDE/gspnlib.h"


extern TYPE_P AllocateBV();

extern char cache_string[];
extern char *lp ;
extern unsigned long d_ptr;
extern unsigned long length;
extern unsigned long f_mark;
extern Canonic_p sfl_h;
Tree_Esrg_p  *EVENT_ARRAY_LOCAL = NULL ;
int *SIZE_OF_EVENT_ARR_LOCAL = NULL;
extern STORE_STATICS_ARRAY Sym_StaticConf;


extern PART_MAT_P FindDisjParts3();
extern int ** *CREATE_STORE_CARD_STRUCTURE();
extern STORE_STATICS_ARRAY NewTabc();
extern void string_to_marking();
extern TO_MERGEP *TO_MERGE();
extern TO_MERGEP *ToMergArr();



void TRAITEMENT_4(TO_MERGEP *merg) {
    int i, j, cl;
    TO_MERGEP ptr = NULL;

    int *tot_sbc = calloc(ncl, sizeof(int));

    for (cl = 0; cl < ncl; cl++) {
        ptr = merg[cl];

        while (ptr) {
            for (i = 0; i < NB_DSC[cl][0]; i++)
                for (j = 0; j < ptr->NB; j++)
                    TEMP[cl][0][i][ptr->SSCs[j]] =  RESULT[cl][ptr->sbc_num][i][j];

            tot_sbc[cl] += ptr->NB;
            ptr = ptr->next;
        }

    }

    PARM_EVENTUALITIE_TO_STRING(1, tot_sbc, TEMP);
    ADD_TEMP_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE, TEMP_EVENT);

    (*EVENT_ARRAY_LOCAL) = realloc((*EVENT_ARRAY_LOCAL),
                                   ((*SIZE_OF_EVENT_ARR_LOCAL) + 1) *
                                   sizeof(struct TREE_ESRG));

    (*EVENT_ARRAY_LOCAL)[(*SIZE_OF_EVENT_ARR_LOCAL)].marking =
        malloc(sizeof(struct MARKING_INFO));
    (*EVENT_ARRAY_LOCAL)[(*SIZE_OF_EVENT_ARR_LOCAL)].marking->marking_as_string =
        FILE_POS_PTR;
    (*EVENT_ARRAY_LOCAL)[(*SIZE_OF_EVENT_ARR_LOCAL)].marking->length = LEGTH_OF_CACHE;
    (*EVENT_ARRAY_LOCAL)[(*SIZE_OF_EVENT_ARR_LOCAL)].enabled_head = NULL;
    (*EVENT_ARRAY_LOCAL)[(*SIZE_OF_EVENT_ARR_LOCAL)].marking->pri = UNKNOWN;
    (*EVENT_ARRAY_LOCAL)[(*SIZE_OF_EVENT_ARR_LOCAL)].Head_Next_Event = NULL;
    (*SIZE_OF_EVENT_ARR_LOCAL)++;

    free(tot_sbc);
}

void RAF_INIT_ARRAYS(TO_MERGEP *merg, STORE_STATICS_ARRAY NEW_CONF) {

    int cl, j;
    TO_MERGEP ptr = NULL;
    int nb_dsc ;

    for (cl = 0; cl < ncl; cl++) {
        nb_dsc = NB_DSC[cl][0];
        ptr = merg[cl];

        while (ptr) {
            for (j = 0; j < ptr->NB; j++)  STATICS[cl][ptr->sbc_num][j] =
                    NEW_CONF[cl].ptr[ptr->SSCs[j]].card;
            for (j = 0; j < nb_dsc ; j++)  DYNAMIC[cl][ptr->sbc_num][j] =
                    RESULT[cl][0][j][ptr->sbc_num];

            NB_DSC [cl][ptr->sbc_num]  = nb_dsc;
            NB_SSBC[cl][ptr->sbc_num]  = ptr->NB;
            ptr = ptr->next;

        }

    }

// INIT_4D_MATRIX(RESULT);
}

int get_refined_eventualities(TYPE_P **PM_mark , TYPE_P NbElPM_mark,
                              TYPE_P **PM , TYPE_P NbElPM,
                              Tree_Esrg_p   *EVENT_ARRAY_ ,
                              int *SIZE_OF_EVENT_ARR_) {

    TO_MERGEP *merg_old_new_part = NULL;
    TO_MERGEP *merg_all = NULL;

    STORE_STATICS_ARRAY old_StaticConf = NULL,
                        new_StaticConf = NULL;

    old_StaticConf = NewTabc(PM_mark , NbElPM_mark ,  MTCL);
    new_StaticConf = NewTabc(PM, NbElPM  ,  MTCL);

    merg_old_new_part = ToMergArr(PM, NbElPM, PM_mark, NbElPM_mark);

    RAF_INIT_ARRAYS(merg_old_new_part, new_StaticConf);

    EVENT_ARRAY_LOCAL = EVENT_ARRAY_ ;
    SIZE_OF_EVENT_ARR_LOCAL = SIZE_OF_EVENT_ARR_;

    EVENTUALITIES(0, 0, 1, 1, TRAITEMENT_4, merg_old_new_part, old_StaticConf);

    FreeMerg(merg_old_new_part);
    FreeMerg(merg_all);

}


int Inclusion(Tree_Esrg_p  event_array_inclued ,
              int size_event_array_inclued,
              Tree_Esrg_p  event_array_inclued_in ,
              int size_event_array_inclued_in) {
    int i, j;

    for (i = 0; i < size_event_array_inclued ; i++) {

        for (j = 0; j < size_event_array_inclued_in ; j++)
            if (NEW_COMPARE_EVENTUALITIES(
                        event_array_inclued[i].marking->marking_as_string,
                        event_array_inclued[i].marking->length,
                        event_array_inclued_in[j].marking->marking_as_string,
                        event_array_inclued_in[j].marking->length,
                        TEMP_EVENT) == EQUAL
               )
                break;

        if (j == size_event_array_inclued_in)
            return false;
    }


    return true;
}

Tree_Esrg_p Get_Diff_Mark(Tree_Esrg_p  event_array_inclued ,
                          int size_event_array_inclued,
                          Tree_Esrg_p  event_array_inclued_in ,
                          int size_event_array_inclued_in) {
    int i, j;
    Tree_Esrg_p list = NULL;

    for (i = 0; i < size_event_array_inclued_in ; i++) {
        for (j = 0; j < size_event_array_inclued ; j++)
            if (
                NEW_COMPARE_EVENTUALITIES(
                    event_array_inclued[j].marking->marking_as_string,
                    event_array_inclued[j].marking->length,
                    event_array_inclued_in[i].marking->marking_as_string,
                    event_array_inclued_in[i].marking->length,
                    TEMP_EVENT
                ) == EQUAL
            )
                break;

        if (j == size_event_array_inclued) {

            event_array_inclued_in[i].Head_Next_Event = NULL;

            if (list == NULL)
                list = &(event_array_inclued_in[i]);
            else {
                event_array_inclued_in[i].Head_Next_Event = list;
                list = &(event_array_inclued_in[i]);
            }

        }

    }


    return list;
}

Tree_Esrg_p
Get_Diff_Mark_DSRG(Tree_Esrg_p  event_array_inclued,
                   int size_event_array_inclued,
                   Tree_Esrg_p  event_array_inclued_in,
                   int size_event_array_inclued_in) {
    int i, j;
    Tree_Esrg_p list = NULL;

    for (i = 0; i < size_event_array_inclued_in ; i++) {
        for (j = 0; j < size_event_array_inclued ; j++)
            if (
                NEW_COMPARE_EVENTUALITIES(
                    event_array_inclued[j].marking->marking_as_string,
                    event_array_inclued[j].marking->length,
                    event_array_inclued_in[i].marking->marking_as_string,
                    event_array_inclued_in[i].marking->length,
                    TEMP_EVENT
                ) == EQUAL
            ) {
                free(event_array_inclued_in[i].marking);
                event_array_inclued_in[i].marking = NULL;
                break;
            }
        if (j == size_event_array_inclued) {

            event_array_inclued_in[i].Head_Next_Event = NULL;

            if (list == NULL)
                list = &(event_array_inclued_in[i]);
            else {
                event_array_inclued_in[i].Head_Next_Event = list;
                list = &(event_array_inclued_in[i]);
            }
        }

    }


    return list;
}


/***************** for ctmc solver  *********************************************/


STORE_STATICS_ARRAY
NewTabc__(TYPE_P **PM, TYPE_P NbElPM, char *** MTCL) {
    int offset = 0, v, i, j, card;
    Obj_p *Objlist = NULL;
    int ptr, ptr1;
    const int SIZE = 256;
    char *tmp = NULL;

    STORE_STATICS_ARRAY tabc = NULL;

    tabc = calloc(ncl, sizeof(struct  STORE_STATICs));
    for (i = 0; i < ncl; i++) {
        offset = 0;
        tabc[i].ptr = (Static_p)STATICSBC_ALLOCATION(NbElPM[i]);
        tabc[i].sbc_num = NbElPM[i];
        for (j = 0; j < tabc[i].sbc_num; j++) {
            tabc[i].ptr[j].obj_name = NULL;
            tabc[i].ptr[j].dynlist = NULL;
            tabc[i].ptr[j].offset = offset;
            card = 0;

            tmp = (char *)calloc(SIZE, sizeof(char)); tmp[0] = '{';
            Objlist = &(tabc[i].ptr[j].obj_list);
            for (v = 1; v <= BIT_TYPE * MAX_IND; v++)
                if (Val(PM[i][j], v) != 0) {

                    (*Objlist) = (Obj_p)malloc(sizeof(struct OBJ));
                    (*Objlist)->next = NULL;
                    (*Objlist)->nome = (char *) strdup((char *)GetObjFrInd(v - 1, MTCL, i));
                    strcat(tmp, (*Objlist)->nome);
                    Objlist = &((*Objlist)->next);
                    offset++; card++;

                }
            strcat(tmp, "}\0");

            tabc[i].ptr[j].name = tmp;
            tabc[i].ptr[j].card = card;

        }
    }

    return tabc;
}
int get_refined_eventualities_prob(TYPE_P **PM_mark ,
                                   TYPE_P NbElPM_mark,
                                   TYPE_P **PM , TYPE_P NbElPM,
                                   Tree_Esrg_p   *EVENT_ARRAY_ ,
                                   int *SIZE_OF_EVENT_ARR_) {

    TO_MERGEP *merg_old_new_part = NULL;
    TO_MERGEP *merg_all = NULL;

    STORE_STATICS_ARRAY old_StaticConf = NULL,
                        new_StaticConf = NULL;

    //INIT_4D_MATRIX(TEMP);

    old_StaticConf = NewTabc__(PM_mark , NbElPM_mark ,  MTCL);
    new_StaticConf = NewTabc__(PM, NbElPM  ,  MTCL);

    merg_old_new_part = ToMergArr(PM, NbElPM, PM_mark, NbElPM_mark);

    RAF_INIT_ARRAYS(merg_old_new_part, new_StaticConf);

    EVENT_ARRAY_LOCAL = EVENT_ARRAY_ ;
    SIZE_OF_EVENT_ARR_LOCAL = SIZE_OF_EVENT_ARR_;

    EVENTUALITIES(0, 0, 1, 1, TRAITEMENT_4, merg_old_new_part, old_StaticConf);

    FreeStoreStructs(old_StaticConf);
    FreeStoreStructs(new_StaticConf);
    FreeMerg(merg_old_new_part);
    FreeMerg(merg_all);

}
/*************************************************************************************/
