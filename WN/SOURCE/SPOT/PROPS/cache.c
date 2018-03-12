#include <stdlib.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/gspnlib.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<glib.h>

typedef struct CKey {
    TYPE_P p1;
    TYPE_P p2;
} Ckey;

typedef struct ToFree {
    Ckey *key;
    PART_MAT_P res;
} tofree;

typedef struct Raff {
    //  TYPE_P p1;
    // TYPE_P p2;
    int  *nb_OStat;
    int ** *Stat;
    int ** *Dyn;
    int   **nb_Stat;
    int   **nb_Dyn;
} raff;

typedef struct Raff_arr {
    Tree_Esrg_p events;
    int nb;
} raff_arr;



GHashTable *hash_table = NULL;
GHashTable *cache_table = NULL;
GHashTable *raff_table = NULL;
PART_MAT_P part = NULL;
int nb_part = 0;
tofree *tofreeCache = NULL;
raff **tofreeraff = NULL;
int nb_tofreeraff = 0;
int nb_tofree = 0;
int *masque = NULL;
int *masque2 = NULL;



extern Tree_Esrg_p  *EVENT_ARRAY_LOCAL ;
extern int *SIZE_OF_EVENT_ARR_LOCAL;
extern void TRAITEMENT_4(TO_MERGEP *merg) ;
extern TO_MERGEP *ToMergArr(TYPE_P **PMDis, TYPE_P NbElDis, TYPE_P **PMCur, TYPE_P NbElCur);
extern PART_MAT_P FindDisjParts3(PART_MAT_P PMS, TYPE NbElPMs);
int ct_tr = 0;
int ct_ntr = 0;



/******************************************** begin cache implem ***************************/
void
add_free_candidate_cache(Ckey *key, PART_MAT_P res) {
    int i;
    for (i = 0; i < nb_tofree; ++i)
        if (tofreeCache[i].key == key)
            return;

    tofreeCache = (tofree *)realloc(tofreeCache, (nb_tofree + 1) * sizeof(*tofreeCache));
    tofreeCache[nb_tofree].key = key ;
    tofreeCache[nb_tofree].res = res;
    nb_tofree++;
}



inline
static guint
cache_key(gconstpointer value) {
    Ckey *ca = (Ckey *)  value;
    guint key = 0, pt1 = (guint)ca->p1 , pt2 = (guint)ca->p2 ;

    pt2  = (pt2 >> 3) * 2654435761U ;
    key  = pt1 ^ pt2;

    return  key;
}

inline
static gboolean
cache_key_equal(gconstpointer a,
                gconstpointer b) {

    Ckey *ca = (Ckey *)  a;
    Ckey *cb = (Ckey *)  b;

    if ((ca->p1 == cb->p1) &&
            (ca->p2 == cb->p2))
        return TRUE;

    return FALSE;
}

static void
cache_key_destroy(gpointer key) {
    free((Ckey *)  key);
}

static void
cache_data_destroy(gpointer data) {

}

inline
static gboolean
cache_table_lookup(Ckey *key, Ckey **org_key, PART_MAT_P *org_data) {

    gboolean bool =
        g_hash_table_lookup_extended(cache_table,
                                     (gconstpointer) key,
                                     (gpointer *)     org_key,
                                     (gpointer *)     org_data);


    return bool;
}



static void
cache_insert(Ckey  *key, PART_MAT_P data) {
    g_hash_table_insert(cache_table, (gpointer)key, (gpointer)data) ;
}

void
cache_init() {
    cache_table =
        g_hash_table_new_full(cache_key,
                              cache_key_equal,
                              cache_key_destroy,
                              cache_data_destroy) ;
}

inline
gboolean
cache_remove_cond(gpointer key, gpointer value, gpointer user_data) {
    Ckey *ckey = (Ckey *) key;
    PART_MAT_P cvalue = (PART_MAT_P) value;
    PART_MAT_P  cuser_data = (PART_MAT_P) user_data;
    if (
        ckey->p1 == cuser_data->NbElPM ||
        ckey->p2 == cuser_data->NbElPM ||
        cvalue->NbElPM ==  cuser_data->NbElPM
    )
        return TRUE;

    return FALSE;
}

PART_MAT_P
GetRefinedPartitions(TYPE_P **sym_mark1_PM , TYPE_P sym_mark1_NbElPM,
                     TYPE_P **sym_mark2_PM , TYPE_P sym_mark2_NbElPM) {
    Ckey *org_key = NULL;
    PART_MAT_P org_data = NULL;

    Ckey *key = malloc(sizeof(Ckey));

    if (sym_mark1_NbElPM > sym_mark2_NbElPM) {
        key->p1 = sym_mark1_NbElPM;
        key->p2 = sym_mark2_NbElPM;
    }
    else {
        key->p2 = sym_mark1_NbElPM;
        key->p1 = sym_mark2_NbElPM;
    }

    if (cache_table_lookup(key , &org_key, &org_data) == TRUE) {
        free(key);
        add_free_candidate_cache(org_key, org_data);
        return org_data;
    }
    else {

        PART_MAT_P PMS = (PART_MAT_P)calloc(2 , sizeof(PART_MAT));;
        PART_MAT_P PM  = NULL;

        PMS[0].PART_MAT = sym_mark1_PM;
        PMS[0].NbElPM = sym_mark1_NbElPM;

        PMS[1].PART_MAT = sym_mark2_PM;
        PMS[1].NbElPM = sym_mark2_NbElPM;

        PM = FindDisjParts3(PMS, 2);
        TreatOrdredClasses(PM->PART_MAT, PM->NbElPM);
        cache_insert(key, PM);
        add_free_candidate_cache(key, PM);

        free(PMS);
        return PM;
    }
}


/*

PART_MAT_P GetRefinedPartitions(TYPE_P** sym_mark1_PM ,TYPE_P sym_mark1_NbElPM,
				TYPE_P** sym_mark2_PM ,TYPE_P sym_mark2_NbElPM )
{

  PART_MAT_P PMS = ( PART_MAT_P)calloc(2 , sizeof(PART_MAT));;
  PART_MAT_P PM  = NULL;

  PMS[0].PART_MAT=sym_mark1_PM;
  PMS[0].NbElPM=sym_mark1_NbElPM;

  PMS[1].PART_MAT=sym_mark2_PM;
  PMS[1].NbElPM=sym_mark2_NbElPM;

  PM=FindDisjParts3( PMS, 2);
  TreatOrdredClasses(PM->PART_MAT, PM->NbElPM);
  PM->Ref=0;
  free(PMS);
  return PM;
}

*/
/***************************************** end of cache implem*******************************/



guint
hash_key(gconstpointer value) {
    guint key = 0;

    PART_MAT_P  v = (PART_MAT_P) value;


    key = (v->PART_MAT[0][0][0]) ^
          (3 * (v->PART_MAT[ncl - 1][v->NbElPM[ncl - 1] - 1][0])) ^
          (7 * (v->PART_MAT[(ncl - 1) / 2][(v->NbElPM[(ncl - 1) / 2] - 1) / 2][0])) ^
          (11 * (v->PART_MAT[3 * (ncl - 1) / 4][(v->NbElPM[3 * (ncl - 1) / 4] - 1) / 4][0]));


    return  key;
}



gboolean
hash_equal(gconstpointer a,
           gconstpointer b) {

    gint i, j;

    PART_MAT_P ca = (PART_MAT_P)  a;
    PART_MAT_P cb = (PART_MAT_P)  b;

    if (bcmp(ca->NbElPM, cb->NbElPM, ncl * sizeof(TYPE)) != 0)
        return FALSE;

    for (i = 0; i < ncl; ++i)
        for (j = 0; j < ca->NbElPM[i]; ++j)
            if (bcmp(ca->PART_MAT[i][j], cb->PART_MAT[i][j],
                     MAX_IND * sizeof(TYPE)) != 0)
                return FALSE;

    return TRUE;

}



static void
key_destroy(gpointer key) {
    /*
     PART_MAT_P k = (PART_MAT_P) key ;
     FreePartMAt(k->PART_MAT,k->NbElPM );
     free(k);
    */
}

void
value_destroy(gpointer data) {
    /*
     STORE_STATICS_ARRAY tabc=(STORE_STATICS_ARRAY)data ;
     if(tabc)
       FreeStoreStructs (tabc);
    */
}

gboolean
hash_table_lookup(PART_MAT_P key, PART_MAT_P *org_key, STORE_STATICS_ARRAY *tab) {

    gboolean bool = g_hash_table_lookup_extended(hash_table,
                    (gconstpointer) key,
                    (gpointer *) org_key,
                    (gpointer *) tab);

    return bool;
}

static void
hash_insert(PART_MAT_P  key, STORE_STATICS_ARRAY tab) {
    g_hash_table_insert(hash_table, (gpointer)key, (gpointer)tab) ;

}

void
set_ref(TYPE_P **PM, TYPE_P NbElPM) {

    STORE_STATICS_ARRAY tab;
    PART_MAT_P org_key;
    PART_MAT_P key = (PART_MAT_P) malloc(sizeof(PART_MAT));
    key->PART_MAT = PM ;
    key->NbElPM = NbElPM;
    key->Ref = 0;
    if (hash_table_lookup(key, &org_key, &tab)) {
        org_key->Ref++;
        free(key);
    }
    else {
        hash_insert(key, NULL);
        key->Ref++;
    }

}




void
hash_init() {
    hash_table =
        g_hash_table_new_full(hash_key,
                              hash_equal,
                              key_destroy,
                              value_destroy) ;
}


PART_MAT_P
new_part(TYPE_P *** PM, TYPE_P *NbElPM, char **MTCL) {

    STORE_STATICS_ARRAY tab;
    PART_MAT_P org_key;
    PART_MAT_P key = (PART_MAT_P) malloc(sizeof(PART_MAT));
    key->PART_MAT = (*PM) ;
    key->NbElPM = (*NbElPM);
    key->Ref = 0;

    if (hash_table_lookup(key, &org_key, &tab)) {
        FreePartMAt(*PM, * NbElPM);
        free(key);
        (*PM) = org_key->PART_MAT;
        (*NbElPM) = org_key->NbElPM;
        return org_key;
    }
    else {
        hash_insert(key, NULL);
        return key;
    }
}

void tabc_aff(STORE_STATICS_ARRAY tabc) {
    int i, j;
    Obj_p Objlist ;

    for (i = 0; i < ncl; i++) {
        printf("classe %d : ", i);
        for (j = 0; j < tabc[i].sbc_num; j++)

            printf("%s ", tabc[i].ptr[j].name);
    }
}

STORE_STATICS_ARRAY
NewTabc(TYPE_P **PM, TYPE_P NbElPM, char *** MTCL) {
    int offset = 0, v, i, j, card;
    Obj_p *Objlist = NULL;
    int ptr, ptr1;
    const int SIZE = 256;
    char *tmp = NULL;

    STORE_STATICS_ARRAY tabc = NULL;
    PART_MAT_P  org_key;
    PART_MAT_P key = (PART_MAT_P) malloc(sizeof(PART_MAT));
    key->PART_MAT = PM ;
    key->NbElPM = NbElPM;
    key->Ref = 0;

    if (hash_table_lookup(key, &org_key, &tabc))
        if (!tabc) {
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

            hash_insert(org_key, tabc);
            free(key);

        }
        else {
            free(key);
        }
    else {
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

        hash_insert(key, tabc);

    }

    return tabc;
}


/******************************** begin raff cache implem **********************************/
/*
void
add_free_candidate_raff(raff* key){
  int i;
   for (i=0;i<nb_tofreeraff;++i)
    if( tofreeraff[i] == key )
      return;

  tofreeraff= (raff**)realloc( tofreeraff, (nb_tofreeraff+1)*sizeof(*tofreeraff));
  tofreeraff[nb_tofreeraff] = key ;
  nb_tofreeraff++;
}
*/

void
free_event(int ** **event) {

    int i, j, k;

    for (i = 0; i < ncl; i++) {
        for (j = 0; j < MAX_CARD; j++) {
            for (k = 0; k < MAX_CARD; ++k)
                free(event[i][j][k]);
            free(event[i][j]);
        }
        free(event[i]);
    }
    free(event);
}

void copy_params(raff *key) {

    int i, s, k;
    TO_MERGEP ptr;

    key->nb_OStat = (int *) calloc(ncl, sizeof(int));
    key->Stat      = (int ** *)calloc(ncl, sizeof(int **));
    key->Dyn       = (int ** *)calloc(ncl, sizeof(int **));
    key->nb_Stat   = (int **) calloc(ncl, sizeof(int *));
    key->nb_Dyn    = (int **) calloc(ncl, sizeof(int *));

    memcpy(key->nb_OStat, SBC_ARR, ncl * sizeof(int));

    for (i = 0; i < ncl; ++i) {
        key->Stat[i]    = (int **)calloc(key->nb_OStat[i], sizeof(int *));
        key->Dyn[i]     = (int **)calloc(key->nb_OStat[i], sizeof(int *));
        key->nb_Stat[i] = (int *) calloc(key->nb_OStat[i], sizeof(int));
        key->nb_Dyn[i]  = (int *) calloc(key->nb_OStat[i], sizeof(int));

        memcpy(key->nb_Stat[i], NB_SSBC[i], key->nb_OStat[i] *sizeof(int));
        memcpy(key->nb_Dyn[i], NB_DSC[i], key->nb_OStat[i] *sizeof(int));

        for (s = 0; s < key->nb_OStat[i]; ++s) {
            key->Stat[i][s] = (int *)calloc(NB_SSBC[i][s] , sizeof(int));
            key->Dyn[i][s]  = (int *)calloc(NB_DSC[i][s], sizeof(int));
            memcpy(key->Stat[i][s], STATICS[i][s], NB_SSBC[i][s] *sizeof(int));
            memcpy(key->Dyn[i][s], DYNAMIC[i][s], NB_DSC[i][s]*sizeof(int));
        }
    }

}

inline
static guint
raff_key(gconstpointer value) {
    unsigned int i, cl, k;
    raff *ca     = (raff *)value;
    int   *nb    = ca->nb_OStat;
    int *** st    = ca->Stat;
    int *** dn    = ca->Dyn ;
    int  **nb_st = ca->nb_Stat;
    int  **nb_dn = ca->nb_Dyn;
    guint key    = 0;


    for (cl = 0; cl < ncl; ++cl) {
        key += (2551 * nb[cl]);
        key += (4603 * nb_st[cl][0])               + (3851 * nb_st[cl][nb_st[cl][0 ] % nb[cl]]) ;
        key += (4451 * nb_st[cl][nb[cl] / 2])        + (5821 * nb_st[cl][nb_st[cl][nb[cl] / 2]  % nb[cl]]) ;
        key += (9049 * nb_st[cl][nb[cl] - 1])        + (4649 * nb_st[cl][nb_st[cl][nb[cl] - 1]  % nb[cl]]) ;
        key += (6011 * nb_dn[cl][nb[cl] / 2])        + (4493 * nb_dn[cl][nb_dn[cl][nb[cl] / 2] % nb[cl]]) ;
        key += (5801 * nb_dn[cl][nb[cl] - 1])        + (4919 * nb_dn[cl][nb_dn[cl][nb[cl] - 1 ] % nb[cl]]) ;
        key += (6997 * nb_dn[cl][0])               + (4051 * nb_dn[cl][nb_dn[cl][0] % nb[cl]]) ;

        key += (7577 * st[cl][0][0])               + (2999 * st[cl][0][st[cl][0][0] % nb_st[cl][0] ]);
        key += (6203 * st[cl][(nb[cl] - 1) / 2 ][nb_st[cl][(nb[cl] - 1) / 2] - 1])  + (5501 * st[cl][(nb[cl] - 1) / 2 ][st[cl][(nb[cl] - 1) / 2 ][nb_st[cl][(nb[cl] - 1) / 2] - 1 ] % nb_st[cl][(nb[cl] - 1) / 2 ]]);
        key += (7331 * st[cl][nb[cl] - 1][0])        + (7703 * st[cl][nb[cl] - 1][st[cl][nb[cl] - 1 ][0] % nb_st[cl][nb[cl] - 1 ] ]);

        key += (9151 * dn[cl][0][0])               + (8353 * dn[cl][0][dn[cl][0][0] % nb_dn[cl][0]]);
        key += (4243 * dn[cl][(nb[cl] - 1) / 2 ][nb_dn[cl][(nb[cl] - 1) / 2] - 1  ])  + (3877 * dn[cl][(nb[cl] - 1) / 2 ][dn[cl][(nb[cl] - 1) / 2][nb_dn[cl][(nb[cl] - 1) / 2] - 1 ] % nb_dn[cl][(nb[cl] - 1) / 2 ] ]);
        key += (6983 * dn[cl][nb[cl] - 1 ][0])       + (6763 * dn[cl][nb[cl] - 1 ][dn[cl][nb[cl] - 1][0] % nb_dn[cl][nb[cl] - 1 ] ]);
    }
    return  key;
}

int cnt = 0;
inline
static gboolean
raff_key_equal(gconstpointer a,
               gconstpointer b) {
    //  printf(" equal key \n");

    int cl, i, j;
    cnt++;
    raff *ca = (raff *)  a;
    raff *cb = (raff *)  b;

    if (bcmp(ca->nb_OStat, cb->nb_OStat, ncl * sizeof(int)) != 0)return FALSE;

    for (cl = 0; cl < ncl; ++cl) {

        if (bcmp(ca->nb_Stat[cl], cb->nb_Stat[cl], ca->nb_OStat[cl] *sizeof(int)) != 0)return FALSE;
        if (bcmp(ca->nb_Dyn[cl] , cb->nb_Dyn[cl] , ca->nb_OStat[cl] *sizeof(int)) != 0) return FALSE ;

        for (j = 0; j < ca->nb_OStat[cl] ; ++j) {
            if (bcmp(ca->Dyn[cl][j], cb->Dyn[cl][j], ca->nb_Dyn[cl][j]*sizeof(int)) != 0) return FALSE;
            if (bcmp(ca->Stat[cl][j], cb->Stat[cl][j], ca->nb_Stat[cl][j]*sizeof(int)) != 0) return FALSE;
        }
    }

    return TRUE;

}

static void
raff_key_destroy(gpointer key) {

}

static void
raff_data_destroy(gpointer data) {

}
int toto = 0;
int lu = 0;
inline
static gboolean
raff_table_lookup(raff *key, raff **org_key, raff_arr  **org_data) {
    //  printf("begin luckup \n");
    lu++;
    cnt = 0;
    gboolean bool =
        g_hash_table_lookup_extended(raff_table,
                                     (gconstpointer) key,
                                     (gpointer *)     org_key,
                                     (gpointer *)     org_data);
    toto += cnt;
    //  printf(" end luckup :%d\n, avg bounce %lf",cnt, (double)toto/(double)lu );

    return bool;
}



static void
raff_insert(raff  *key, raff_arr *data) {
    g_hash_table_insert(raff_table, (gpointer)key, (gpointer)data) ;
}

void
raff_init() {
    raff_table =
        g_hash_table_new_full(raff_key,
                              raff_key_equal,
                              raff_key_destroy,
                              raff_data_destroy) ;
}







/******************************** end raff cache impllem **********************************/

void
hash_remove_cond(gpointer key, gpointer value) {

    PART_MAT_P k = (PART_MAT_P) key;
    Ckey *org_key;
    PART_MAT_P org_data;
    raff *org_key1;
    raff_arr *org_data1;
    int i;

    if (k->Ref == 0) {
        g_hash_table_remove(hash_table, key);

        for (i = 0; i < nb_tofree; ++i)
            if (masque[i] == 0)
                if (
                    k->NbElPM == tofreeCache[i].key->p1 ||
                    k->NbElPM == tofreeCache[i].key->p2 ||
                    k->NbElPM == tofreeCache[i].res->NbElPM
                )
                    if (cache_table_lookup(tofreeCache[i].key, &org_key, &org_data)) {
                        g_hash_table_remove(cache_table, org_key);
                        masque[i] = 1;

                    }

        STORE_STATICS_ARRAY tabc = (STORE_STATICS_ARRAY)value ;
        if (tabc) FreeStoreStructs(tabc);
        FreePartMAt(k->PART_MAT, k->NbElPM);
        free(k);

    }

}
/*
gboolean limit_size()
{
  if(g_hash_table_size(hash_table) >= max_table)
    return TRUE;
  else return FALSE;
}
*/
void
add_free_candidate_part(TYPE_P **PART_MAT, TYPE_P NbElPM) {
    int i;

    for (i = 0; i < nb_part; ++i)
        if (PART_MAT == part[i].PART_MAT)
            return;

    part = (PART_MAT_P)realloc(part, (nb_part + 1) * sizeof(*part));

    part[nb_part].PART_MAT = PART_MAT ;
    part[nb_part].NbElPM   = NbElPM   ;
    nb_part++;

}


void
hash_table_garbage() {
    //  printf(" before garbage collect : hash %d, cache %d \n", g_hash_table_size(hash_table),g_hash_table_size(cache_table) );
    // int g= g_hash_table_size(hash_table);
    PART_MAT_P org_key = NULL;
    STORE_STATICS_ARRAY tab = NULL;
    int i;

    masque         = (int *) calloc(nb_tofree, sizeof(int));
    PART_MAT_P key = (PART_MAT_P) malloc(sizeof(PART_MAT));

    for (i = 0; i < nb_part; ++i) {
        key->PART_MAT = part[i].PART_MAT;
        key->NbElPM   = part[i].NbElPM;
        if (hash_table_lookup(key, &org_key, &tab)) {
            // (void) hash_table_lookup(key,&org_key, &tab );
            (void) hash_remove_cond((gpointer) org_key, (gpointer) tab);
        }
        else
            printf("introuvable");
    }

    free(key);
    free(part);
    part = NULL;
    nb_part = 0;
    free(tofreeCache);
    tofreeCache = NULL;
    nb_tofree = 0;
    //  free(tofreeraff);
    //  tofreeraff=NULL;
    //  nb_tofreeraff=0;
    free(masque);
    masque = NULL;
    // free(masque2);
    // masque2=NULL;
    // printf(" after garbage collect :  hash %d, cache %d \n", g_hash_table_size(hash_table),g_hash_table_size(cache_table) );
    // getchar();
}

/* int get_refined_eventualities( TYPE_P** PM_mark ,TYPE_P NbElPM_mark, TYPE_P** PM ,TYPE_P NbElPM, */
/* 				Tree_Esrg_p *  EVENT_ARRAY_ ,int* SIZE_OF_EVENT_ARR_  ) */
/* { */

/*   TO_MERGEP* merg_old_new_part =NULL; */
/*   TO_MERGEP* merg_all =NULL; */

/*   STORE_STATICS_ARRAY old_StaticConf=NULL, */
/*                       new_StaticConf=NULL; */

/*   raff* org_key=NULL; */
/*   raff_arr*  org_data=NULL; */
/*   raff* key= (raff*)malloc(sizeof(raff)); */

/*   old_StaticConf = NewTabc(PM_mark , NbElPM_mark ,  MTCL); */
/*   new_StaticConf = NewTabc(PM, NbElPM  ,  MTCL); */

/*   merg_old_new_part=ToMergArr(PM,NbElPM,PM_mark,NbElPM_mark); */

/*   RAF_INIT_ARRAYS(merg_old_new_part, new_StaticConf); */

/*   key->nb_OStat=SBC_ARR; */
/*   key->Stat=STATICS; */
/*   key->Dyn=DYNAMIC; */
/*   key->nb_Stat=NB_SSBC; */
/*   key->nb_Dyn=NB_DSC; */

/*   if(raff_table_lookup( key, &org_key,&org_data )) */
/*    { */
/*      ct_tr++; */
/*      real_eventualities(merg_old_new_part ,org_data->events,org_data->nb, */
/*      	EVENT_ARRAY_ ,SIZE_OF_EVENT_ARR_, old_StaticConf ); */

/*        //  (*EVENT_ARRAY_) =  org_data->events ; */
/*        // (*SIZE_OF_EVENT_ARR_) = org_data->nb ; */
/*      free(key); */
/*    } */
/*   else { */
/*    ct_ntr++; */
/*    EVENT_ARRAY_LOCAL= EVENT_ARRAY_ ; */
/*    SIZE_OF_EVENT_ARR_LOCAL= SIZE_OF_EVENT_ARR_; */
/*    EVENTUALITIES(0,0,1,1,TRAITEMENT_4, merg_old_new_part,old_StaticConf ); */
/*    org_data = (raff_arr*) malloc(sizeof(raff_arr)); */
/*    org_data->events = (* EVENT_ARRAY_); */
/*    org_data->nb= (*SIZE_OF_EVENT_ARR_); */
/*    (*EVENT_ARRAY_)=NULL ; */
/*    (*SIZE_OF_EVENT_ARR_)=0; */
/*    real_eventualities(merg_old_new_part ,org_data->events,org_data->nb, */
/* 		      EVENT_ARRAY_ ,SIZE_OF_EVENT_ARR_, old_StaticConf ); */

/*    copy_params(key); */
/*    raff_insert (key, org_data ); */

/*  } */

/*   FreeMerg(merg_old_new_part ); */
/*   FreeMerg(merg_all ); */
/*   //    printf(" trouver : %d , non trouver : %d \n",ct_tr,ct_ntr); */
/* } */
