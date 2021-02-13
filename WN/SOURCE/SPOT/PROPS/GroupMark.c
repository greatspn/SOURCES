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


extern int GET_NB_SOLUTIONS_SYS_EQUA();
extern int get_max_cardinality();
extern int GetStaticInd();
extern void TreatOrdredClasses();
extern int GetIndObj(char *id, char *** MTCL, int *Cl);

#ifdef LIBMCDSRG
extern double probability();
#endif
MARK_GROUPp CurTreat = NULL, CurTreat_tail = NULL;
MarkAsEventp  *CurList = NULL; int NbCurList = 0;
MarkAsEventp GROUPED = NULL, NOT_GROUPED = NULL;
int Gwith = 0, ToGroup = 0, CLAS = 0;
int **CurGroup = NULL; int *NbGroup = NULL;
int *Index = NULL;
STORE_STATICS_ARRAY CONF;
int *DSC = NULL;
int *SSC = NULL;
int *DSCBIS1 = NULL;
int *SSCBIS1 = NULL;
int *DSCBIS2 = NULL;
int *SSCBIS2 = NULL;
static int count = 0, count1 = 0;;

void DSC_DSCBIS1() {
    DSCBIS1 = DSC;
    SSCBIS1 = SSC;
}

void DSC_DSCBIS2() {
    DSCBIS2 = DSC;
    SSCBIS2 = SSC;
}
void DSCBIS1_DSC() {
    DSC = DSCBIS1;
    SSC = SSCBIS1;
}

void DSCBIS2_DSC() {
    DSC = DSCBIS2;
    SSC = SSCBIS2;
}


void Free_DSC_SSC() {
    free(DSC);
    free(SSC);
}




void InitCurTreattemp(Tree_Esrg_p EventArr,
                      STORE_STATICS_ARRAY STORED_CONF,
                      STORE_STATICS_ARRAY SYM_CONF) {
    int i, j, k, s, max_card = get_max_cardinality();

    CONF = STORED_CONF;
    DSC = calloc(ncl, sizeof(int));
    SSC = calloc(ncl, sizeof(int));

    CurTreat = (MARK_GROUPp)malloc(sizeof(MARK_GROUP));
    CurTreat->next_group = NULL;
    CurTreat->group = calloc(ncl, sizeof(int *));
    CurTreat->NbGroup = calloc(ncl, sizeof(int));
    CurTreat->indx = calloc(max_card, sizeof(int *));
    CurTreat->cl = 0;
    //CurTreat->i=0;
    // CurTreat->j=1;
    CurTreat->i = CONF[0].sbc_num - 1;
    CurTreat->j = CONF[0].sbc_num - 2;
    for (i = 0; i < max_card; i++) CurTreat->indx[i] = 1;

    for (i = 0; i < ncl; i++) {
        CurTreat->group[i] = calloc(CONF[i].sbc_num, sizeof(int));
        for (j = 0; j < CONF[i].sbc_num; j++)
            CurTreat->group[i][j] = j ;
    }

    MarkAsEventp *ptr = &(CurTreat->list);

    while (EventArr) {
        (*ptr) = (MarkAsEventp)malloc(sizeof(MarkAsEvent));
        (*ptr)->Event = calloc(ncl, sizeof(int **));
        (*ptr)->next = NULL;

        GET_TEMP_EVENTUALITIE_FROM_FILE(EventArr->marking->marking_as_string,
                                        EventArr->marking->length, TEMP_EVENT);
        COPY_CACHE(EventArr->marking->length);
        STRING_TO_EVENTUALITIE(SYM_CONF);

        for (j = 0; j < ncl; j++) {
            DSC[j] = NB_DSC[j][0]; /**** A refaire *************/
            SSC[j] = NB_SSBC[j][0];
            (*ptr)->Event[j] = calloc(NB_SSBC[j][0], sizeof(int *));

            for (k = 0; k < NB_SSBC[j][0]; k++) {
                (*ptr)->Event[j][k] = calloc(NB_DSC[j][0], sizeof(int));
                for (s = 0; s < NB_DSC[j][0]; s++)
                    (*ptr)->Event[j][k][s] = RESULT[j][0][s][k];
            }
        }


        ptr = &((*ptr)->next);
        EventArr = EventArr->Head_Next_Event;
    }

}


void InitCurTreat(Tree_Esrg_p EventArr,
                  STORE_STATICS_ARRAY STORED_CONF,
                  STORE_STATICS_ARRAY SYM_CONF) {
    int i, j, k, s, max_card = get_max_cardinality();

    CONF = STORED_CONF;
    DSC = calloc(ncl, sizeof(int));
    SSC = calloc(ncl, sizeof(int));

    CurTreat = (MARK_GROUPp)malloc(sizeof(MARK_GROUP));
    CurTreat->next_group = NULL;
    CurTreat->group = calloc(ncl, sizeof(int *));
    CurTreat->NbGroup = calloc(ncl, sizeof(int));
    CurTreat->indx = calloc(max_card, sizeof(int *));
    CurTreat->cl = 0;
    //CurTreat->i=0;
    //CurTreat->j=1;
    CurTreat->i = CONF[0].sbc_num - 1;
    CurTreat->j = CONF[0].sbc_num - 2;
    for (i = 0; i < max_card; i++) CurTreat->indx[i] = 1;

    for (i = 0; i < ncl; i++) {
        CurTreat->group[i] = calloc(CONF[i].sbc_num, sizeof(int));
        for (j = 0; j < CONF[i].sbc_num; j++)
            CurTreat->group[i][j] = j ;
    }

    MarkAsEventp *ptr = &(CurTreat->list);

    while (EventArr) {
        (*ptr) = (MarkAsEventp)malloc(sizeof(MarkAsEvent));
        (*ptr)->Event = calloc(ncl, sizeof(int **));
        (*ptr)->next = NULL;

        GET_EVENTUALITIE_FROM_FILE(EventArr->marking->marking_as_string,
                                   EventArr->marking->length);
        COPY_CACHE(EventArr->marking->length);
        STRING_TO_EVENTUALITIE(SYM_CONF);

        for (j = 0; j < ncl; j++) {
            DSC[j] = NB_DSC[j][0]; /**** A refaire *************/
            SSC[j] = NB_SSBC[j][0];
            (*ptr)->Event[j] = calloc(NB_SSBC[j][0], sizeof(int *));

            for (k = 0; k < NB_SSBC[j][0]; k++) {
                (*ptr)->Event[j][k] = calloc(NB_DSC[j][0], sizeof(int));
                for (s = 0; s < NB_DSC[j][0]; s++)
                    (*ptr)->Event[j][k][s] = RESULT[j][0][s][k];
            }
        }


        ptr = &((*ptr)->next);
        EventArr = EventArr->Head_Next_Event;
    }

}
#ifdef LIBMCDSRG
void InitCurTreat_Mc(Tree_Esrg_p EventArr,
                     STORE_STATICS_ARRAY STORED_CONF,
                     STORE_STATICS_ARRAY SYM_CONF,
                     int gr) {
    int i, j, k, s, max_card = get_max_cardinality();

    CONF = STORED_CONF;
    DSC = calloc(ncl, sizeof(int));
    SSC = calloc(ncl, sizeof(int));

    CurTreat = (MARK_GROUPp)malloc(sizeof(MARK_GROUP));
    CurTreat->next_group = NULL;
    CurTreat->group = calloc(ncl, sizeof(int *));
    CurTreat->NbGroup = calloc(ncl, sizeof(int));
    CurTreat->indx = calloc(max_card, sizeof(int *));
    CurTreat->cl = 0;
    CurTreat->i = CONF[0].sbc_num - 1;
    CurTreat->j = CONF[0].sbc_num - 2;
    for (i = 0; i < max_card; i++) CurTreat->indx[i] = 1;

    for (i = 0; i < ncl; i++) {
        CurTreat->group[i] = calloc(CONF[i].sbc_num, sizeof(int));
        for (j = 0; j < CONF[i].sbc_num; j++)
            CurTreat->group[i][j] = j ;
    }

    MarkAsEventp *ptr = &(CurTreat->list);

    while (EventArr) {
        if (EventArr->gr == gr) {
            count++;
            (*ptr) = (MarkAsEventp)malloc(sizeof(MarkAsEvent));
            (*ptr)->Event = calloc(ncl, sizeof(int **));
            (*ptr)->next = NULL;

            GET_EVENTUALITIE_FROM_FILE(EventArr->marking->marking_as_string,
                                       EventArr->marking->length);
            COPY_CACHE(EventArr->marking->length);
            STRING_TO_EVENTUALITIE(SYM_CONF);

            for (j = 0; j < ncl; j++) {
                DSC[j] = NB_DSC[j][0]; /**** A refaire *************/
                SSC[j] = NB_SSBC[j][0];
                (*ptr)->Event[j] = calloc(NB_SSBC[j][0], sizeof(int *));

                for (k = 0; k < NB_SSBC[j][0]; k++) {
                    (*ptr)->Event[j][k] = calloc(NB_DSC[j][0], sizeof(int));
                    for (s = 0; s < NB_DSC[j][0]; s++)
                        (*ptr)->Event[j][k][s] = RESULT[j][0][s][k];
                }
            }
            (*ptr)->ord  = EventArr->marking->ordinary;
            // (*ptr)->rate =probability(EventArr);
            (*ptr)->rate = probability(EventArr) / (*ptr)->ord ;
            ptr = &((*ptr)->next);
        }
        EventArr = EventArr->Head_Next_Event;
    }
}

void InitCurTreat_Mc_Part(Tree_Esrg_p EventArr,
                          int nb,
                          STORE_STATICS_ARRAY STORED_CONF,
                          STORE_STATICS_ARRAY SYM_CONF,
                          int gr) {
    int i, j, k, s, max_card = get_max_cardinality();

    CONF = STORED_CONF;
    DSC = calloc(ncl, sizeof(int));
    SSC = calloc(ncl, sizeof(int));

    CurTreat = (MARK_GROUPp)malloc(sizeof(MARK_GROUP));
    CurTreat->next_group = NULL;
    CurTreat->group = calloc(ncl, sizeof(int *));
    CurTreat->NbGroup = calloc(ncl, sizeof(int));
    CurTreat->indx = calloc(max_card, sizeof(int *));
    CurTreat->cl = 0;
    CurTreat->i = CONF[0].sbc_num - 1;
    CurTreat->j = CONF[0].sbc_num - 2;
    for (i = 0; i < max_card; i++) CurTreat->indx[i] = 1;

    for (i = 0; i < ncl; i++) {
        CurTreat->group[i] = calloc(CONF[i].sbc_num, sizeof(int));
        for (j = 0; j < CONF[i].sbc_num; j++)
            CurTreat->group[i][j] = j ;
    }

    MarkAsEventp *ptr = &(CurTreat->list);
    int c;
    for (c = 0; c < nb; c++) {
        if (EventArr[c].gr == gr) {
            count++;
            (*ptr) = (MarkAsEventp)malloc(sizeof(MarkAsEvent));
            (*ptr)->Event = calloc(ncl, sizeof(int **));
            (*ptr)->next = NULL;
            GET_TEMP_EVENTUALITIE_FROM_FILE(EventArr[c].marking->marking_as_string,
                                            EventArr[c].marking->length, TEMP_EVENT);
            COPY_CACHE(EventArr[c].marking->length);
            STRING_TO_EVENTUALITIE(SYM_CONF);

            for (j = 0; j < ncl; j++) {
                DSC[j] = NB_DSC[j][0]; /**** A refaire *************/
                SSC[j] = NB_SSBC[j][0];
                (*ptr)->Event[j] = calloc(NB_SSBC[j][0], sizeof(int *));

                for (k = 0; k < NB_SSBC[j][0]; k++) {
                    (*ptr)->Event[j][k] = calloc(NB_DSC[j][0], sizeof(int));
                    for (s = 0; s < NB_DSC[j][0]; s++)
                        (*ptr)->Event[j][k][s] = RESULT[j][0][s][k];
                }
            }
            (*ptr)->ord  = EventArr[c].marking->ordinary;
            ptr = &((*ptr)->next);
        }
    }

}
#endif

InitCurList(MarkAsEventp list) {
    int i;

    if (CurList) free(CurList);
    CurList = NULL;
    NbCurList = 0;

    while (list) {
        CurList = realloc(CurList, (NbCurList + 1) * sizeof(MarkAsEventp));
        CurList[NbCurList] = list;
        NbCurList++;
        list = list->next;
    }
}

int Valid_Matrix_cl(int **mat1, int **mat2, int NbSbc,
                    int NbDsc, int first, int second, int cl) {
    int i;

    for (i = 0; i < second; i++)
        if (memcmp(mat1[i], mat2[i], NbDsc * sizeof(int)) != 0)
            return false;
    for (i = second + 1; i < first; i++)
        if (memcmp(mat1[i], mat2[i], NbDsc * sizeof(int)) != 0)
            return false;
    for (i = first + 1; i < NbSbc; i++)
        if (memcmp(mat1[i], mat2[i], NbDsc * sizeof(int)) != 0)
            return false;

    return true;
}

int Valid_Matrix(int *** mat1, int *** mat2,
                 int *NbSbc, int *NbDsc,
                 int first , int second, int cl, int ncl) {
    int i;

    for (i = 0; i < ncl; i++)
        if (i == cl) {
            if (!Valid_Matrix_cl(mat1[i], mat2[i], NbSbc[i],
                                 NbDsc[i], first, second, cl))
                return false;
        }
        else if (!Valid_Matrix_cl(mat1[i], mat2[i],
                                  NbSbc[i], NbDsc[i], -1, -1 , cl))
            return false;

    return true;
}

int GetSecondMember(int **mat, int line, int nbcol, int first, int second) {
    int j, card = 0;
    return (mat[first][line] + mat[second][line]);
}

int  Get_Card(int *group, int ind, int cl) {
    int i;
    int card = 0;


    for (i = ind; i < CONF[cl].sbc_num; i++)
        if (group[i] == group[ind])
            card += CONF[cl].ptr[i].card;

    return card;
    /********** A refaire *******/
}

void GetEquationSystem(int **mat, int *DSC,
                       int *group, int Nbgroup,
                       int first, int second, int cl) {
    int j;

    STATICS[0][0][0] = Get_Card(group, first, cl);
    STATICS[0][0][1] = Get_Card(group, second, cl);
    for (j = 0; j < DSC[cl]; j++)
        DYNAMIC[0][0][j] = GetSecondMember(mat, j, CONF[cl].sbc_num, first, second);
    NB_DSC[0][0] = DSC[cl];
    NB_SSBC[0][0] = 2;


}
int NB_SOLUTIONS(int **mat, int *DSC, int *group,
                 int Nbgroup, int first, int second, int cl) {
    GetEquationSystem(mat, DSC, group, Nbgroup, first, second, cl);
    return GET_NB_SOLUTIONS_SYS_EQUA();
}

void GROUP_MAT_ELEM(int **mat , int NbDsc, int first, int second) {
    int i;
    for (i = 0; i < NbDsc; i++) {
        mat[second][i] += mat[first][i];
        mat[first][i] = 0;
    }
}
void  LIBERER_ELEM(MarkAsEventp ptr , int *ord, double *rate) {
    int cl, sb;
    MarkAsEventp next;
    while (ptr) {
        (*ord) += ptr->ord;
        next = ptr->next;
        for (cl = 0; cl < ncl; cl++) {
            for (sb = 0; sb < SSC[cl]; sb++)
                free(ptr->Event[cl][sb]);
            free(ptr->Event[cl]);
        }
        free(ptr->Event);
        free(ptr);
        ptr = next;
    }
}


void Search_Valid_Group(int *DSC, int I, int J, int cl) {
    int i, j, NB_SOL, NB_EXIST;
    MarkAsEventp *ptr;
    MarkAsEventp *Agr = & GROUPED;
    MarkAsEventp *Angr = & NOT_GROUPED;

    GROUPED = NULL;
    NOT_GROUPED = NULL;

    if (CONF[cl].sbc_num != 1) {
        for (i = 0; i < NbCurList; i++)
            if (CurList[i]) {
                CurList[i]->next = NULL;
                ptr = &(CurList[i]->next);

                NB_SOL = NB_SOLUTIONS(
                             CurList[i]->Event[cl],
                             DSC,
                             CurGroup[cl],
                             NbGroup[cl],
                             I, J, cl);
                NB_EXIST = 1;

                for (j = i + 1; j < NbCurList; j++)
                    if (CurList[j])
                        if (Valid_Matrix(CurList[i]->Event,
                                         CurList[j]->Event,
                                         SSC, DSC, I, J, cl, ncl)) {
                            (*ptr) = CurList[j];
                            CurList[j]->next = NULL;
                            ptr = &(CurList[j]->next);
                            CurList[j] = NULL;
                            NB_EXIST++;
                        }

                if (NB_SOL == NB_EXIST) {
                    GROUP_MAT_ELEM(CurList[i]->Event[cl], DSC[cl], I, J);
                    LIBERER_ELEM(CurList[i]->next,
                                 &(CurList[i]->ord),
                                 &(CurList[i]->rate));
                    CurList[i]->next = NULL;
                    (*Agr) = CurList[i];
                    Agr = &(CurList[i]->next);
                }
                else {
                    (*Angr) = CurList[i];
                    Angr = ptr;
                }
            }
    }
    else (*Agr) = CurList[0];

}
void Free_Globals() {
    int i;
    if (CurGroup) {
        for (i = 0; i < ncl; i++)
            free(CurGroup[i]);
        free(CurGroup);
    }
    if (NbGroup) free(NbGroup);
    if (Index) free(Index);
}
void INIT_GLOBAL_VARIABLES(MARK_GROUPp *CurTreat) {

    MARK_GROUPp ptr = (* CurTreat);

    InitCurList(ptr->list);
    Gwith = ptr->i;
    ToGroup = ptr->j;
    CLAS = ptr->cl;
    Free_Globals();
    CurGroup = ptr-> group;
    NbGroup = ptr-> NbGroup;
    Index = ptr->indx;
    (* CurTreat) = (* CurTreat)->next_group;
    free(ptr);

}

MARK_GROUPp New_Element(int I, int J, int Cl, int flag) {
    MARK_GROUPp ptr;
    int i, j, max_card = get_max_cardinality();

    ptr = malloc(sizeof(MARK_GROUP));
    ptr->next_group = NULL;
    ptr->group = calloc(ncl, sizeof(int *));
    ptr->NbGroup = calloc(ncl, sizeof(int));;
    ptr->indx = calloc(get_max_cardinality(), sizeof(int *));
    ptr->cl = Cl;
    ptr->i = I;
    ptr->j = J;

    if (flag)
        for (i = 0; i < max_card; i++)
            ptr->indx[i] = Index[i];
    else
        for (i = 0; i < max_card; i++)
            ptr->indx[i] = 1;

    for (i = 0; i < ncl; i++) {
        ptr->NbGroup[i] = NbGroup[i];
        ptr->group[i] = calloc(CONF[i].sbc_num, sizeof(int));
        for (j = 0; j < CONF[i].sbc_num; j++)
            ptr->group[i][j] = CurGroup[i][j] ;
    }

    return ptr;
}

int NEXT_INDEX(int ind) {
    int k;

    for (k = ind - 1 ; k >= 0 && !Index[k] ; --k);
    if (k >= 0)
        return k;
    else
        return -1;
}
int NEXT_CLASS(int CL) {
    int k;
    for (k = CL + 1; k < ncl ; k++)
        if (CONF[k].sbc_num > 1)
            return k;
    return -1;
}

int NEXT_COMBINATION(MARK_GROUPp *ptr, int flag) {
    int i, j;

    if ((j = NEXT_INDEX(ToGroup)) != -1) {
        if (!flag)
            (*ptr) = New_Element(Gwith, j, CLAS, true);
        else {
            Gwith = ToGroup;
            ToGroup = j;
        }
        return true;
    }
    else if ((i = NEXT_INDEX(Gwith)) != -1)
        if ((j = NEXT_INDEX(i)) != -1) {
            if (!flag)
                (*ptr) = New_Element(i, j, CLAS, true);
            else {
                Gwith = i;
                ToGroup = j;
            }

            return true;
        }
        else {

            if (NEXT_CLASS(CLAS) != -1) {
                if (!flag)
                    (*ptr) = New_Element(CONF[CLAS + 1].sbc_num - 1,
                                         CONF[CLAS + 1].sbc_num - 2, CLAS + 1, false);
                else {
                    Gwith = CONF[CLAS + 1].sbc_num - 1;
                    ToGroup = CONF[CLAS + 1].sbc_num - 2;
                    CLAS++;
                    int i;
                    for (i = 0; i < get_max_cardinality(); i++)
                        Index[i] = 1;

                }
                return true;
            }
        }
    else if (NEXT_CLASS(CLAS) != -1) {
        if (!flag)
            (*ptr) = New_Element(CONF[CLAS + 1].sbc_num - 1,
                                 CONF[CLAS + 1].sbc_num - 2, CLAS + 1, false);
        else {
            Gwith = CONF[CLAS + 1].sbc_num - 1;
            ToGroup = CONF[CLAS + 1].sbc_num - 2;
            CLAS++;
            int i;
            for (i = 0; i < get_max_cardinality(); i++)
                Index[i] = 1;

        }
        return true;
    }

    return false;
}

void GROUP_VARS() {
    int j ;
    Index[Gwith] = 0;

    for (j = 0; j < CONF[CLAS].sbc_num; j++)
        if (CurGroup[CLAS][j] == Gwith)
            CurGroup[CLAS][j] = ToGroup;

}

void Add_CurTreat(MARK_GROUPp ptr, MarkAsEventp list) {
    if (!CurTreat) {

        CurTreat = CurTreat_tail = ptr;
        CurTreat->list = list;
    }
    else {
        /*
          CurTreat_tail=ptr;
          CurTreat->list=list;
        */
        CurTreat_tail->next_group = ptr;
        CurTreat_tail = ptr;
        CurTreat_tail->list = list;
    }
}

void Add_Res(MARK_GROUPp ptr, MarkAsEventp list) {
    /**** for tests ****/
    MarkAsEventp list1;
    for (list1 = list; list1 != NULL; list1 = list1->next) count1++;
    /******************/
    ResultList = realloc(ResultList, (NbResList + 1) * sizeof(MARK_GROUPp));
    ResultList[NbResList] = ptr;
    ResultList[NbResList]->list = list;

    NbResList++;
}

void Affich_Res() {
    int i, u, v;
    MarkAsEventp list;
    for (i = 0; i < NbResList; i++) {
        list = ResultList[i]->list;
        while (list) {
            for (u = 0; u < 4; u++) {
                for (v = 0; v < 3; v++)
                    printf("%d, ,", list->Event[0][u][v]);

                printf("\n");
            }
            list = list->next;
        }
    }
}

void Search_All_Groups() {
    MARK_GROUPp ptr;
    int flag = true, behav;
    while (CurTreat) {

        INIT_GLOBAL_VARIABLES(&CurTreat);

        do {

            Search_Valid_Group(DSC, Gwith, ToGroup, CLAS);

            if (NOT_GROUPED)
                if (NEXT_COMBINATION(&ptr, false))
                    Add_CurTreat(ptr, NOT_GROUPED);
                else {
                    ptr = New_Element(Gwith, ToGroup, CLAS, false);
                    Add_Res(ptr, NOT_GROUPED);
                }

            if (GROUPED) {
                if (CONF[CLAS].sbc_num != 1)
                    GROUP_VARS();
                if ((flag = NEXT_COMBINATION(&ptr, true)) == true)
                    InitCurList(GROUPED);
                else {

                    ptr = New_Element(Gwith, ToGroup, CLAS, false);
                    Add_Res(ptr, GROUPED);
                }
            }

        }
        while (GROUPED && flag);

    }

}

void GET_PARTITIONS_AFTER_GROUPING(STORE_STATICS_ARRAY ASYM_CONF, char *** MTCL) {
    /* cette fonction est � revoir */
    int i, cl, sb, k, CL, gr;
    Obj_p ptr;
    for (i = 0; i < NbResList; i++) {

        ResultList[i]->PART_MAT = calloc(ncl, sizeof(TYPE_P *));
        ResultList[i]->NbElPM = calloc(ncl, sizeof(TYPE));

        for (cl = 0; cl < ncl; cl++)
            for (sb = 0; sb < ASYM_CONF[cl].sbc_num; sb++)
                if (ResultList[i]->group[cl][sb] != -1) {
                    gr = ResultList[i]->group[cl][sb];
                    ResultList[i]->PART_MAT[cl] = realloc(ResultList[i]->PART_MAT[cl],
                                                          (ResultList[i]->NbElPM[cl] + 1) * sizeof(TYPE_P));
                    ResultList[i]->PART_MAT[cl][ResultList[i]->NbElPM[cl]] = (TYPE_P)AllocateBV();

                    for (k = sb; k < ASYM_CONF[cl].sbc_num; k++)
                        if (ResultList[i]->group[cl][k] == gr) {
                            ResultList[i]->group[cl][k] = -1;

                            for (ptr = ASYM_CONF[cl].ptr[k].obj_list; ptr != NULL; ptr = ptr->next)
                                Add(ResultList[i]->PART_MAT[cl][ResultList[i]->NbElPM[cl]], GetIndObj(ptr->nome, MTCL, &CL));
                        }

                    (ResultList[i]->NbElPM[cl])++;
                }

        new_part(&(ResultList[i]->PART_MAT) , &(ResultList[i]->NbElPM), MTCL);

    }
}


void INIT_RESULT_STRUCT(int *** event) {


    int cl, sb, nsb, ds, cd;

// INIT_4D_MATRIX(RESULT);

    for (cl = 0; cl < ncl; cl++) {
        nsb = 0;
        for (sb = 0; sb < SSC[cl]; sb++) {
            cd = 0;
            for (ds = 0; ds < DSC[cl]; ds++) {
                cd += event[cl][sb][ds];
                RESULT[cl][0][ds][nsb] = event[cl][sb][ds];
            }
            if (cd) nsb++;
        }
    }
}

void GROUP_MARKINGS(Tree_Esrg_p EventArr,
                    STORE_STATICS_ARRAY ASYM_CONF,
                    STORE_STATICS_ARRAY SYM_CONF, char *** MTCL) {
    int i;
    ResultList = NULL;
    NbResList = 0;

    InitCurTreat(EventArr, ASYM_CONF, SYM_CONF);
    Search_All_Groups();
    GET_PARTITIONS_AFTER_GROUPING(ASYM_CONF, MTCL);

    for (i = 0; i < NbResList; i++)
        TreatOrdredClasses(ResultList[i]->PART_MAT, ResultList[i]->NbElPM);

}

void TEMP_GROUP_MARKINGS(Tree_Esrg_p EventArr,
                         STORE_STATICS_ARRAY ASYM_CONF,
                         STORE_STATICS_ARRAY SYM_CONF, char *** MTCL) {
    int i;
    ResultList = NULL;
    NbResList = 0;

    InitCurTreattemp(EventArr, ASYM_CONF, SYM_CONF);
    Search_All_Groups();
    GET_PARTITIONS_AFTER_GROUPING(ASYM_CONF, MTCL);

    for (i = 0; i < NbResList; i++)
        TreatOrdredClasses(ResultList[i]->PART_MAT, ResultList[i]->NbElPM);

}


#ifdef LIBMCDSRG
void GROUP_MARKINGS_MC(Tree_Esrg_p EventArr,
                       STORE_STATICS_ARRAY ASYM_CONF,
                       STORE_STATICS_ARRAY SYM_CONF,
                       char *** MTCL, int gr) {
    int i;
    ResultList = NULL;
    NbResList = 0;
    mcgroup = count = count1 = 0;

    InitCurTreat_Mc(EventArr, ASYM_CONF, SYM_CONF, gr);
    Search_All_Groups();
    GET_PARTITIONS_AFTER_GROUPING(ASYM_CONF, MTCL);

    for (i = 0; i < NbResList; i++)
        TreatOrdredClasses(ResultList[i]->PART_MAT, ResultList[i]->NbElPM);

    if (count > count1)
        mcgroup = 1;
}
void GET_PARTITIONS_AFTER_GROUPING_PART(STORE_STATICS_ARRAY ASYM_CONF, char *** MTCL) {
    /* cette fonction est � revoir */
    int i, cl, sb, k, CL, gr;
    Obj_p ptr;
    for (i = 0; i < NbResList; i++) {

        ResultList[i]->PART_MAT = calloc(ncl, sizeof(TYPE_P *));
        ResultList[i]->NbElPM = calloc(ncl, sizeof(TYPE));

        for (cl = 0; cl < ncl; cl++)
            for (sb = 0; sb < ASYM_CONF[cl].sbc_num; sb++)
                if (ResultList[i]->group[cl][sb] != -1) {
                    gr = ResultList[i]->group[cl][sb];
                    ResultList[i]->PART_MAT[cl] = realloc(ResultList[i]->PART_MAT[cl],
                                                          (ResultList[i]->NbElPM[cl] + 1) * sizeof(TYPE_P));
                    ResultList[i]->PART_MAT[cl][ResultList[i]->NbElPM[cl]] = (TYPE_P)AllocateBV();

                    for (k = sb; k < ASYM_CONF[cl].sbc_num; k++)
                        if (ResultList[i]->group[cl][k] == gr) {
                            ResultList[i]->group[cl][k] = -1;
                            for (ptr = ASYM_CONF[cl].ptr[k].obj_list; ptr != NULL; ptr = ptr->next)
                                Add(ResultList[i]->PART_MAT[cl][ResultList[i]->NbElPM[cl]],
                                    GetIndObj(ptr->nome, MTCL, &CL));
                        }

                    (ResultList[i]->NbElPM[cl])++;
                }

    }
}

void GROUP_MARKINGS_MC_PART(Tree_Esrg_p EventArr,
                            int nb,
                            STORE_STATICS_ARRAY ASYM_CONF,
                            STORE_STATICS_ARRAY SYM_CONF,
                            char *** MTCL, int gr) {
    int i;
    ResultList = NULL;
    NbResList = 0;
    mcgroup = count = count1 = 0;

    InitCurTreat_Mc_Part(EventArr, nb, ASYM_CONF, SYM_CONF, gr);
    Search_All_Groups();
    GET_PARTITIONS_AFTER_GROUPING_PART(ASYM_CONF, MTCL);

    //  for(i=0;i<NbResList;i++)
    //  TreatOrdredClasses(ResultList[i]->PART_MAT,ResultList[i]->NbElPM);

    if (count > count1)
        mcgroup = 1;
}
#endif
