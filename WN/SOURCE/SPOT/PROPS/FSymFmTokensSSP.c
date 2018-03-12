#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/gspnlib.h"

void Afficher(TYPE_P **PM, TYPE_P NbElPM, int *ClArr, TYPE NbCl) {
    TYPE j, cl, ind, v;
    TYPE_P val;

    for (cl = 0; cl < NbCl; cl++)
        for (j = 0; j < NbElPM[ClArr[cl]]; j++) {
            printf("\n La partition :%d de la classe %s  : ", j, tabc[ClArr[cl]].col_name);
            for (v = 1; v <= BIT_TYPE * MAX_IND; v++)
                if (Val(PM[ClArr[cl]][j], v) != 0)
                    printf(" %d ", v);


        }
    printf("\n");

}


CLASS_P AllocateCl(int size, CLASS_P ptr) {
    CLASS_P pt;
    pt = (CLASS *)realloc(ptr, (size + 1) * sizeof(CLASS));
    pt[size].Set = (TYPE_P)AllocateBV();
    return pt;
}

void FreeArrOfData(TYPE_P *DM, TYPE NbTokens) {
    TYPE j;
    for (j = 0; j < NbTokens; j++)
        free(DM[j]);
    free(DM);
}

void FreePartMAt(TYPE_P **PM, TYPE_P NbElPM) {
    int cl, sb;

    for (cl = 0; cl < ncl; cl++) {
        for (sb = 0; sb < NbElPM[cl]; sb++)
            free(PM[cl][sb]);
        free(PM[cl]);
    }
    free(PM);
    free(NbElPM);
}

void TreatOrdredClasses(TYPE_P **PM, TYPE_P NbElPM) {
    int cl, sb, c;

    for (cl = 0; cl < ncl; cl++)
        if (IS_ORDERED(cl) && NbElPM[cl] > 1) {
            for (sb = 0; sb < NbElPM[cl]; sb++) free(PM[cl][sb]);
            free(PM[cl]);

            PM[cl] = NULL;
            NbElPM[cl] = 0;
            for (c = 0; c < tabc[cl].card; c++) {
                PM[cl] = realloc(PM[cl], (NbElPM[cl] + 1) * sizeof(TYPE_P));
                PM[cl][NbElPM[cl]] = (TYPE_P)AllocateBV();
                Add(PM[cl][NbElPM[cl]], c + 1);
                NbElPM[cl]++;
            }

        }
}


void FreeClasses(CLASS_P Cls, TYPE NbCL) {
    TYPE i;
    for (i = 0; i < NbCL; i++)
        free(Cls[i].Set);
    free(Cls);
}
void InitArray(TYPE_P *DM, TYPE NbTokens) {
    TYPE i;
    for (i = 0; i < NbTokens; i++)
        DM[i][0] = 0;
}

int GetIndObj(char *id, char *** MTCL, int *Cl) {
    int cl, i;

    for (cl = 0; cl < ncl; cl++)
        for (i = 0; i < tabc[cl].card; i++)
            if (strcmp(id, MTCL[cl][i]) == 0) { (*Cl) = cl; return (i + 1);}

    return UNKNOWN;
}

char *GetObjFrInd(int ind, char *** MTCL, int Cl) {
    return MTCL[Cl][ind];
}

int GetStaticInd(char *id, STORE_STATICS_ARRAY ASYM_CONF, int Cl) {
    int sb;
    Obj_p ptr;
    for (sb = 0; sb < ASYM_CONF[Cl].sbc_num; sb++)
        for (ptr = ASYM_CONF[Cl].ptr[sb].obj_list; ptr != NULL ; ptr = ptr->next)
            if (strcmp(ptr->nome, id)) return sb;

    return UNKNOWN;
}

char ** *GetMatRepOfClObj() {
    const int str_size = 256;
    int cl, i, k, j, m;
    char *tmp1;
    char *tmp2 = (char *)calloc(str_size, sizeof(char));
    Obj_p Obj_list;
    char *** MTCL = NULL;

    MTCL = (char ** *)calloc(ncl, sizeof(char **));
    for (cl = 0; cl < ncl; cl++) {
        MTCL[cl] = (char **)calloc(tabc[cl].card, sizeof(char *));
        k = 0;
        for (i = 0; i < tabc[cl].sbc_num; i++) {

            if (tabc[cl].sbclist[i].obj_list == NULL)
                for (j = tabc[cl].sbclist[i].low; j < tabc[cl].sbclist[i].low + tabc[cl].sbclist[i].card; j++) {
                    tmp1 = strdup(tabc[cl].sbclist[i].obj_name);
                    bzero(temp2, str_size * sizeof(char));
                    sprintf(tmp2, "%d", j);
                    MTCL[cl][k] = (char *)calloc(strlen(tmp1) + strlen(tmp2) + 1, sizeof(char));
                    strcat(MTCL[cl][k], tmp1); strcat(MTCL[cl][k], tmp2);
                    free(tmp1);
                    k++;
                }
            else
                for (Obj_list = tabc[cl].sbclist[i].obj_list; Obj_list != NULL; Obj_list = Obj_list->next) {

                    MTCL[cl][k] = strdup(Obj_list->nome);
                    k++;
                }
        }
    }
    free(tmp2);
    return MTCL;
}

TYPE_P **GetMatRepOfClParts(char *** MTCL, TYPE_P NbElPM) {
    const int SIZE = 256;
    int cl, j, i, CL;
    Obj_p Obj_list;
    char *temp = (char *)calloc(SIZE, sizeof(char)) ;
    TYPE_P **Parts_Mat = (TYPE_P **)calloc(ncl, sizeof(TYPE_P *));

    for (cl = 0; cl < ncl; cl++)
        for (i = 0; i < tabc[cl].sbc_num; i++) {
            Parts_Mat[cl] = (TYPE_P *)realloc(Parts_Mat[cl], (NbElPM[cl] + 1) * sizeof(TYPE_P));
            Parts_Mat[cl][i] = (TYPE_P)AllocateBV();

            if (tabc[cl].sbclist[i].obj_list == NULL)
                for (j = tabc[cl].sbclist[i].low; j < tabc[cl].sbclist[i].low + tabc[cl].sbclist[i].card; j++) {
                    bzero(temp, SIZE * sizeof(char));
                    strcat(temp, tabc[cl].sbclist[i].obj_name);
                    sprintf(temp + (strlen(temp)*sizeof(char)), "%d", j);
                    Add(Parts_Mat[cl][i], GetIndObj(temp, MTCL, &CL)) ;

                }
            else
                for (Obj_list = tabc[cl].sbclist[i].obj_list, j = 1; Obj_list != NULL; Obj_list = Obj_list->next, j++)
                    Add(Parts_Mat[cl][i], GetIndObj(Obj_list->nome, MTCL, &CL)) ;

            NbElPM[cl]++;
        }
    free(temp);
    return Parts_Mat;
}



PART_MAT_P GetMatRepOfTransDom(char *** MTCL, Token_Domains dom) {

    int cl, pcl = dom.Domain[0], i;
    PART_MAT_P  Parts_Mat = (PART_MAT_P)malloc(sizeof(struct Part_Mat));
    Parts_Mat->PART_MAT = (TYPE_P **)calloc(ncl, sizeof(TYPE *));
    Parts_Mat->NbElPM = (TYPE_P)calloc(ncl, sizeof(TYPE));

    for (cl = 0; cl < ncl; cl++) {
        if (cl != pcl) {
            Parts_Mat->PART_MAT[cl] = (TYPE_P *)malloc(sizeof(TYPE_P));
            Parts_Mat->PART_MAT[cl][0] = (TYPE_P)AllocateBV();
            Parts_Mat->NbElPM[cl] = 1;
            for (i = 0; i < tabc[cl].card; i++)
                Add(Parts_Mat->PART_MAT[cl][0], i + 1) ;
        }
        else {
            Parts_Mat->PART_MAT[cl] = (TYPE_P *)calloc(2, sizeof(TYPE_P));
            Parts_Mat->PART_MAT[cl][0] = (TYPE_P)AllocateBV();
            Parts_Mat->PART_MAT[cl][1] = (TYPE_P)AllocateBV();
            Parts_Mat->NbElPM[cl] = 2;

            for (i = 0; i < dom.NbCl; i++)
                Add(Parts_Mat->PART_MAT[cl][0], dom.Token[i]);

            for (i = 0; i < tabc[cl].card; i++)
                if (Val(Parts_Mat->PART_MAT[cl][0], i + 1))
                    Add(Parts_Mat->PART_MAT[cl][1], i + 1) ;

        }
    }
    return Parts_Mat;
}

PART_MAT_P GetMatRepOfPartTransDom(char *** MTCL, int pcl) {

    int cl, i;
    PART_MAT_P  Parts_Mat = (PART_MAT_P)malloc(sizeof(struct Part_Mat));
    Parts_Mat->PART_MAT = (TYPE_P **)calloc(ncl, sizeof(TYPE *));
    Parts_Mat->NbElPM = (TYPE_P)calloc(ncl, sizeof(TYPE));

    for (cl = 0; cl < ncl; cl++) {
        if (cl != pcl) {
            Parts_Mat->PART_MAT[cl] = (TYPE_P *)malloc(sizeof(TYPE_P));
            Parts_Mat->PART_MAT[cl][0] = (TYPE_P)AllocateBV();
            Parts_Mat->NbElPM[cl] = 1;
            for (i = 0; i < tabc[cl].card; i++)
                Add(Parts_Mat->PART_MAT[cl][0], i + 1) ;
        }
        else {
            Parts_Mat->PART_MAT[cl] = (TYPE_P *)calloc(tabc[cl].card, sizeof(TYPE_P));
            Parts_Mat->NbElPM[cl] = tabc[cl].card;

            for (i = 0; i < tabc[cl].card; i++) {
                Parts_Mat->PART_MAT [cl][i] = (TYPE_P)AllocateBV();
                Add(Parts_Mat->PART_MAT[cl][i], i + 1) ;
            }

        }
    }

    return Parts_Mat;
}

CLASS_P InitArrOfCl(TYPE_P Cls, TYPE size) {
    TYPE indx, i;
    CLASS_P ptr = NULL;
    for (indx = 0; indx < size; indx++) {
        ptr = AllocateCl(indx, ptr);
        ptr[indx].Cl = Cls[indx];
        ptr[indx].Card = tabc[Cls[indx]].card;
        for (i = 0; i < tabc[Cls[indx]].card; i++)
            Add(ptr[indx].Set, i + 1);
    }
    return ptr;
}

TYPE_P *InitArrOfData(char *** MTCL, CToken_p pToken, TYPE_P NbTokens, TYPE_P Cls, TYPE size) {
    TYPE_P *Data_Mat = NULL;
    CToken_p pt;
    TYPE NbToken, i, CL;
    for (pt = pToken, NbToken = 0; pt != NULL; pt = pt->next, NbToken++) {
        Data_Mat = (TYPE_P *)realloc(Data_Mat, (NbToken + 1) * sizeof(TYPE_P));
        Data_Mat[NbToken] = (TYPE_P)calloc(size + 3, sizeof(TYPE));
        Data_Mat[NbToken][0] = 0;
        Data_Mat[NbToken][1] = 1;
        for (i = 2; i < (size + 2); i++)
            Data_Mat[NbToken][i] = (pt->tok_dom).Token[i - 2];
        Data_Mat[NbToken][size + 2] = pt->mult;
    }
    (*NbTokens) = NbToken;
    return Data_Mat;
}

void DynDistOfObjects(TYPE_P **PM, TYPE *NbElPM, int *** Card, int **num, int **DynDis) {
    int i, j, k, s, ds, card, cd;

    for (i = 0; i < ncl; i++)
        for (j = 0; j < tabc[i].card; j++) {
            for (k = 0; (k < NbElPM[i]) && (Val(PM[i][k], j + 1) == 0); k++);
            card = 1; cd = 0;
            for (s = 1; s < j + 1; s++)
                if (Val(PM[i][k], s) != 0)
                    card++;
            for (ds = 0; (ds < num[i][k]) && (cd < card) ; ds++) {
                cd += Card[i][k][ds];
                if (cd >= card) DynDis[i][j] = tabc[i].sbclist[k].offset + ds;
            }
        }
}

void SymRepOfToken(Token_Domains tk, Token_Domains *Simbtk, int **DynDis) {
    int i;

    Simbtk->Token = (int *)calloc(tk.NbCl, sizeof(int));
    Simbtk->Domain = (int *)calloc(tk.NbCl, sizeof(int));
    Simbtk->NbCl = tk.NbCl;
    for (i = 0; i < tk.NbCl; i++) {
        Simbtk->Domain[i] = tk.Domain[i];
        Simbtk->Token[i] = DynDis[ Simbtk->Domain[i]][tk.Token[i] - 1];
    }
}

int TestSymRepWtToken(Token_Domains Simbtk, Token_Domains tk, int **DynDis) {
    int i;
    for (i = 0; i < tk.NbCl; i++)
        if (Simbtk.Token[i] != DynDis[Simbtk.Domain[i]][tk.Token[i] - 1])
            return false;
    return true;
}


TYPE_P CvtoBitArr(TYPE_P *DM, TYPE NbTokens, TYPE Cl) {
    TYPE_P temp = NULL;
    TYPE i;
    temp = (TYPE_P)AllocateBV();
    for (i = 0; i < NbTokens; i++)
        if (DM[i][0] == 1)
            Add(temp, DM[i][Cl + 2]) ;

    return temp;
}

TYPE FindPosInPm(TYPE_P el, TYPE_P **PM, TYPE_P NbElPM, CLASS_P ClArr, TYPE CL) {
    TYPE j;

    for (j = 0; j < NbElPM[ClArr[CL].Cl]; j++)
        if (COMP(PM[ClArr[CL].Cl][j], el) == true) {
            free(el);
            return (j + ClArr[CL].Card + 1);
        }

    PM[ClArr[CL].Cl] = (TYPE_P *) realloc(PM[ClArr[CL].Cl], (NbElPM[ClArr[CL].Cl] + 1) * sizeof(TYPE_P));
    PM[ClArr[CL].Cl][ NbElPM[ClArr[CL].Cl]] = el;
    NbElPM[ClArr[CL].Cl]++;

    return (ClArr[CL].Card + NbElPM[ClArr[CL].Cl]);
}

TYPE CompTokens(TYPE ref, TYPE toCmp, TYPE Cur, TYPE_P *DM, TYPE NbCL) {
    int i;
    for (i = 0; i < Cur; i++)
        if (DM[ref][i + 2] != DM[toCmp][i + 2])
            return false;

    for (i = Cur + 1; i <= NbCL; i++)
        if (DM[ref][i + 2] != DM[toCmp][i + 2])
            return false;

    return true;
}

TYPE GetNextInd(TYPE ind, TYPE_P *DM, TYPE NbTokens) {
    TYPE i;

    for (i = ind + 1; i < NbTokens; i++)
        if (DM[i][1] > 0) return i;

    return NbTokens ;
}



void FindNonDisjParts(CLASS_P ClArr, TYPE NbClas, TYPE_P *DM, TYPE NbTokens, TYPE_P **PM, TYPE_P NbElPM) {
    TYPE ind, j, NewInd, v, in;
    TYPE_P el;
    TYPE CuTrCl;

    for (CuTrCl = 0; CuTrCl < NbClas; CuTrCl++) {
        ind = 0;
        while (ind < NbTokens) {

            DM[ind][0] = 1;
            j = ind + 1;

            while (j < NbTokens) {
                if (DM[j][1] == DM[ind][1])
                    if (CompTokens(ind, j, CuTrCl, DM, NbClas) == true) {
                        DM[j][0] = 1;
                        DM[j][1] = 0;
                    }
                j++;
            }

            el = CvtoBitArr(DM, NbTokens, CuTrCl);
            NewInd = FindPosInPm(el, PM, NbElPM, ClArr, CuTrCl);
            DM[ind][CuTrCl + 2] = NewInd;
            DM[ind][1] = NewInd;
            ind = GetNextInd(ind, DM, NbTokens);
            InitArray(DM, NbTokens);
        }

    }
}

void FindDisjParts(CLASS_P ClArr, TYPE NbClas, TYPE_P **PM, TYPE_P NbElPM) {
    TYPE v, j, k, i, necl, flag;
    TYPE_P var;
    INTERS_P NewArr = NULL;

    for (i = 0; i < NbClas; i++) {
        necl = 0;
        for (v = 1; v <= BIT_TYPE * MAX_IND; v++)
            if (Val(ClArr[i].Set, v) != 0) {
                var = (TYPE_P)AllocateBV();
                for (j = 0; j < NbElPM[ClArr[i].Cl]; j++)
                    if (Val(PM[ClArr[i].Cl][j], v) != 0) {
                        Minus(PM[ClArr[i].Cl][j], v);
                        Add(var, j + 1);
                    }
                flag = false;
                for (k = 0; k < necl && !flag; k++)
                    if (COMP(var, NewArr[k].Set) == true) {
                        flag = true;
                        Add(NewArr[k].Indx, v);
                        free(var);
                    }
                if (!flag) {
                    NewArr = (INTERS *)realloc(NewArr, (necl + 1) * sizeof(INTERS));
                    NewArr[necl].Set = var;
                    NewArr[necl].Indx = (TYPE_P)AllocateBV();
                    Add(NewArr[necl].Indx, v);
                    necl++;
                }
            }
        for (j = 0; j < NbElPM[ClArr[i].Cl]; j++) free(PM[ClArr[i].Cl][j]);
        free(PM[ClArr[i].Cl]);
        PM[ClArr[i].Cl] = calloc(necl, sizeof(sizeof(TYPE_P *)));

        //   PM[ClArr[i].Cl]=realloc( PM[ClArr[i].Cl],necl*sizeof(TYPE_P*));

        for (j = 0; j < necl; j++) {
            //if (j<NbElPM[ClArr[i].Cl]) free( PM[ClArr[i].Cl][j]);

            PM[ClArr[i].Cl][j] = NewArr[j].Indx;
            NewArr[j].Indx = NULL;
            free(NewArr[j].Set);
        }
        NbElPM[ClArr[i].Cl] = necl;
    }
    free(NewArr);
}

PART_MAT_P FindDisjParts3(PART_MAT_P PMS, TYPE NbElPMs) {
    TYPE i, j, k;
    int *ClArr = NULL;
    TYPE NbCls = 0;

    PART_MAT_P PMR = (PART_MAT_P)malloc(sizeof(PART_MAT));
    PMR->PART_MAT  = (TYPE_P **)calloc(ncl, sizeof(TYPE_P *));
    PMR->NbElPM    = (TYPE_P)calloc(ncl, sizeof(TYPE));

    for (i = 0 ; i < NbElPMs ; i++)
        for (j = 0 ; j < ncl ; j++)
            for (k = 0 ; k < PMS[i].NbElPM[j] ; k++) {
                PMR->PART_MAT[j] = (TYPE_P *)realloc(PMR->PART_MAT[j], (PMR->NbElPM[j] + 1) * sizeof(TYPE_P));
                PMR->PART_MAT[j][PMR->NbElPM[j]] = (TYPE_P) AllocateBV();
                memcpy(PMR->PART_MAT[j][PMR->NbElPM[j]], PMS[i].PART_MAT[j][k], MAX_IND * sizeof(TYPE));
                PMR->NbElPM[j]++;
            }

    for (i = 0; i < ncl; i++)
        if (PMR->NbElPM[i] > 1) {
            ClArr = realloc(ClArr, (NbCls + 1) * sizeof(int));
            ClArr[NbCls] = i;
            NbCls++;
        }

    CLASS_P CLAS = InitArrOfCl(ClArr, NbCls);
    FindDisjParts(CLAS, NbCls, PMR->PART_MAT, PMR->NbElPM);
    FreeClasses(CLAS, NbCls);
    free(ClArr);
    return (PMR);

}

TYPE_P **FindDisjParts2(int *ClArr, TYPE NbClas, TYPE_P **PMF, TYPE_P NbElPMF, TYPE_P **PMM, TYPE_P NbElPMM) {
    int i, j;

    CLASS_P CLAS = InitArrOfCl(ClArr, NbClas);

    for (i = 0; i < NbClas; i++) {
        PMM[ClArr[i]] = (TYPE_P *)realloc(PMM[ClArr[i]], (NbElPMM[ClArr[i]] + NbElPMF[ClArr[i]]) * sizeof(TYPE_P));
        for (j = NbElPMM[ClArr[i]]; j < (NbElPMM[ClArr[i]] + NbElPMF[ClArr[i]]); j++) {
            PMM[ClArr[i]][j] = PMF[ClArr[i]][j - NbElPMM[ClArr[i]]];
            PMF[ClArr[i]][j - NbElPMM[ClArr[i]]] = NULL;
        }
        NbElPMM[ClArr[i]] += NbElPMF[ClArr[i]];
        free(PMF[ClArr[i]]);
    }

    free(PMF);
    free(NbElPMF);
    FindDisjParts(CLAS, NbClas, PMM, NbElPMM);
    FreeClasses(CLAS, NbClas);
    return PMM;
}


TYPE_P **ComputeLocalPartitions(char *** MTCL, TYPE_P Cls, TYPE NbCLs, CToken_p pToken, TYPE_P *NbElPM) {
    CLASS_P CLAS = InitArrOfCl(Cls, NbCLs);
    TYPE NbTokens = 0;
    TYPE_P *Data_Mat = InitArrOfData(MTCL, pToken, &NbTokens, Cls, NbCLs);
    TYPE_P **Part_Mat = (TYPE_P **)calloc(ncl, sizeof(TYPE_P *));
    (*NbElPM) = (TYPE_P)calloc(ncl, sizeof(TYPE));
    FindNonDisjParts(CLAS, NbCLs, Data_Mat, NbTokens, Part_Mat, *NbElPM);
    FindDisjParts(CLAS, NbCLs, Part_Mat, *NbElPM);
    FreeClasses(CLAS, NbCLs);
    FreeArrOfData(Data_Mat, NbTokens);
    return Part_Mat;
}

PART_MAT_P GetPartitions(State s) {
    int i, j, nb = 0;
    PART_MAT_P PMS = NULL;
    PART_MAT_P PM = NULL;

    PMS = (PART_MAT_P)realloc(PMS, (nb + 1) * sizeof(PART_MAT));
    PMS[0].PART_MAT = s->PM;
    PMS[0].NbElPM = s->NbElPM;
    nb++;

    for (i = 0; i < NbProp; i++)
        if ((_CONS_[i] != -1) && (PROPOSITIONS[i].prop->PART_MAT != NULL)) {
            PMS = (PART_MAT_P)realloc(PMS, (nb + 1) * sizeof(PART_MAT));
            PMS[nb].PART_MAT = PROPOSITIONS[i].prop->PART_MAT;
            PMS[nb].NbElPM = PROPOSITIONS[i].prop->NbElPM;
            nb++;
        }

    PM = FindDisjParts3(PMS, nb);
    TreatOrdredClasses(PM->PART_MAT, PM->NbElPM);
    free(PMS);
    return PM;
}

