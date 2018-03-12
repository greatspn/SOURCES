
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <glib.h>

const int size_string = 2048;
GHashTable *hash_table = NULL;

StrPrt *alloc_StrPrt() {
    StrPrt *part = (StrPrt *) malloc(sizeof(StrPrt));
    part->part = (char *) calloc(size_string, sizeof(char));
    return part;
}


void store_char_partition(unsigned long nval, char **lp_part, char *string_part) {
    register unsigned char cc;

    int space_left = size_string - (int)((*lp_part) - string_part)  - 1;

    if (nval < 64) {
        if (space_left < 1) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }

        cc = (unsigned char)(0x3F & nval);
        (**lp_part) = cc;
        (*lp_part) ++;
    }
    else if (nval < 16384) {
        if (space_left < 2) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0x40 | ((0x3F00 & nval) >> 8));
        (**lp_part)  = cc; (*lp_part) ++;
        cc = (unsigned char)(0xFF & nval);
        (**lp_part)  = cc; (*lp_part) ++;
    }
    else if (nval < 4194304) {
        if (space_left < 3) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0x80 | ((0x3F0000 & nval) >> 16));
        (**lp_part)  = cc; (*lp_part) ++;
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        (**lp_part)  = cc; (*lp_part) ++;
        cc = (unsigned char)(0xFF & nval);
        (**lp_part)  = cc; (*lp_part) ++;
    }
    else if (nval < 1073741824) {
        if (space_left < 4) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0xC0 | ((0x3F000000 & nval) >> 24));
        (**lp_part)  = cc; (*lp_part) ++;
        cc = (unsigned char)((0xFF0000 & nval) >> 16);
        (**lp_part)  = cc; (*lp_part) ++;
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        (**lp_part)  = cc; (*lp_part) ++;
        cc = (unsigned char)(0xFF & nval);
        (**lp_part)  = cc; (*lp_part) ++;

    }
    else {
        fprintf(stderr, "char_store_compact error: value overflow\n");
        exit(1);
    }
}

void char_load_compat_part(unsigned long *pp , char **lp_part) {
    register unsigned char cc0  ;
    register unsigned long uu = (cc0 & 0x3F);
    register unsigned long ii = (cc0 & 0xC0) >> 6;

    cc0 = **lp_part;
    (*lp_part)++;
    uu = (cc0 & 0x3F);
    ii = (cc0 & 0xC0) >> 6;

    for (; ii ; ii--) {
        cc0 = **lp_part;
        (*lp_part)++;
        uu = uu << 8;
        uu = uu + cc0;
    }
    *pp = uu;
}

int GetStringFromColorPartition(int **group, int *nbgroup, STORE_STATICS_ARRAY ASYM_CONF,
                                char *** MTCL, char *string_part) {
    int i, cl, sb, k, CL, gr;
    Obj_p ptr;
    TYPE_P value = (TYPE_P)AllocateBV();
    char *lp_part = string_part;

    for (cl = 0; cl < ncl; ++cl) {
        store_char_partition((unsigned long) nbgroup[cl] , &lp_part, string_part);

        for (sb = 0; sb < ASYM_CONF[cl].sbc_num; ++sb)
            if (group[cl][sb] != -1) {
                gr = group[cl][sb];
                for (k = sb; k < ASYM_CONF[cl].sbc_num; ++k)
                    if (group[cl][k] == gr) {
                        group[cl][k] = -1;
                        for (ptr = ASYM_CONF[cl].ptr[k].obj_list; ptr != NULL; ptr = ptr->next)
                            Add(value, GetIndObj(ptr->nome, MTCL, &CL));
                    }

                for (i = 0; i < MAX_IND; ++i)
                    store_char_partition((unsigned long) value[i] , &lp_part, string_part);

                VideC(value);
            }
    }

    free(value);
    return (int)(lp_part - string_part);

}

STORE_STATICS_ARRAY GetColorPartitionFromString(char *string_part, char **MTCL) {
    const int SIZE = 256;
    int    offset, v, cl, j, k, card;
    char  *lp_part = string_part;
    Obj_p *Objlist = NULL;
    int    ptr, ptr1;
    char  *tmp = NULL;
    TYPE_P value = (TYPE_P)AllocateBV();

    STORE_STATICS_ARRAY tabc = calloc(ncl, sizeof(struct  STORE_STATICs));

    for (cl = 0; cl < ncl; ++cl) {
        offset = 0;
        char_load_compat_part((unsigned long *)&tabc[cl].sbc_num , &lp_part);
        tabc[cl].ptr = (Static_p)STATICSBC_ALLOCATION(tabc[cl].sbc_num);

        for (j = 0; j < tabc[cl].sbc_num; j++) {
            tabc[cl].ptr[j].obj_name = NULL;
            tabc[cl].ptr[j].dynlist = NULL;
            tabc[cl].ptr[j].offset = offset;
            card = 0;
            tmp = (char *)calloc(SIZE, sizeof(char)); tmp[0] = '{';
            Objlist = &(tabc[cl].ptr[j].obj_list);

            for (k = 0; k < MAX_IND; ++k)
                char_load_compat_part((unsigned long *) &value[k] , &lp_part);

            for (v = 1; v <= BIT_TYPE * MAX_IND; v++)
                if (Val(value, v) != 0) {

                    (*Objlist) = (Obj_p)malloc(sizeof(struct OBJ));
                    (*Objlist)->next = NULL;
                    (*Objlist)->nome = strdup((char *)GetObjFrInd(v - 1, MTCL, cl));
                    strcat(tmp, (*Objlist)->nome);
                    Objlist = &((*Objlist)->next);
                    ++offset;
                    ++card;

                }
            strcat(tmp, "}\0");

            tabc[cl].ptr[j].name = tmp;
            tabc[cl].ptr[j].card = card;
            VideC(value);

        }
    }
    free(value);
    return tabc;
}

int GetStringRepFromTabc(char *string_part, char **MTCL) {
    const int SIZE = 256;
    int cl, j, i, CL;
    Obj_p Obj_list;
    char *temp = (char *)calloc(SIZE, sizeof(char)) ;
    TYPE_P value = (TYPE_P)AllocateBV();
    char *lp_part = string_part;

    for (cl = 0; cl < ncl; cl++)
        for (i = 0; i < tabc[cl].sbc_num; i++) {

            store_char_partition((unsigned long)tabc[cl].sbc_num  , &lp_part, string_part);

            if (tabc[cl].sbclist[i].obj_list == NULL)
                for (j = tabc[cl].sbclist[i].low; j < tabc[cl].sbclist[i].low + tabc[cl].sbclist[i].card; j++) {
                    bzero(temp, SIZE * sizeof(char));
                    strcat(temp, tabc[cl].sbclist[i].obj_name);
                    sprintf(temp + (strlen(temp)*sizeof(char)), "%d", j);
                    Add(value, GetIndObj(temp, MTCL, &CL)) ;

                }
            else
                for (Obj_list = tabc[cl].sbclist[i].obj_list, j = 1; Obj_list != NULL; Obj_list = Obj_list->next, j++)
                    Add(value, GetIndObj(Obj_list->nome, MTCL, &CL)) ;

            for (j = 0; j < MAX_IND; ++j)
                store_char_partition((unsigned long) value[j] , &lp_part, string_part);

            VideC(value);

        }
    free(temp);
    return (int)(lp_part - string_part);
}

PART_MAT_P GetMatRepFromString(char *string_part, char **MTCL) {

    int cl, j, i, k;
    PART_MAT_P  Parts_Mat = (PART_MAT_P)malloc(sizeof(struct Part_Mat));
    Parts_Mat->PART_MAT = (TYPE_P **)calloc(ncl, sizeof(TYPE *));
    Parts_Mat->NbElPM = (TYPE_P)calloc(ncl, sizeof(TYPE));
    char *lp_part = string_part;

    for (cl = 0; cl < ncl; cl++) {
        char_load_compat_part((unsigned long *) & (Parts_Mat->NbElPM[cl]) , &lp_part);
        Parts_Mat->PART_MAT[cl] = (TYPE_P *)calloc(Parts_Mat->NbElPM[cl], sizeof(TYPE_P));

        for (i = 0; i < Parts_Mat->NbElPM[cl]; i++) {

            Parts_Mat->PART_MAT[cl][i] = (TYPE_P)AllocateBV();

            for (k = 0; k < MAX_IND; ++k)
                char_load_compat_part((unsigned long *) &  Parts_Mat->PART_MAT[cl][i][k] , &lp_part);
        }
    }

    return Parts_Mat;
}




static guint
hash_key(gconstpointer value) {

    guint key = 0;
    StrPrt *v = (StrPrt *) value;
    key = v->part[0];
    key << 8;
    key += v->part[v->size - 1];
    key << 8;
    key += v->part[(v->size - 1) / 2 ];
    key << 8;
    key += v->part[ 3 * (v->size - 1) / 4 ];

    return  key;
}


static gboolean
hash_equal(gconstpointer a,
           gconstpointer b) {

    StrPrt  *ca = (StrPrt *) a;
    StrPrt  *cb = (StrPrt *) b;

    if (ca->size != cb->size)
        return FALSE;
    else {
        unsigned long ii;
        unsigned long op1;
        unsigned long op2;
        char *cp;
        char *pp;
        cp = ca-> part;
        pp = cb-> part;
        for (ii = ca->size ; ii ; ii--) {
            op1 = *cp; cp++;
            op2 = *pp; pp++;
            if (op1 != op2)
                return FALSE;

        }
    }

    return TRUE;

}

static void
key_destroy(gpointer key) {
    //    StrPrt* k = (StrPrt*) key;
    // free(k->part);
    // free(k);
}

static void
value_destroy(gpointer data) {

}
static gboolean
hash_table_lookup(StrPrt *key, StrPrt **org_key, STORE_STATICS_ARRAY *tab) {

    return g_hash_table_lookup_extended(hash_table,
                                        (gconstpointer) key,
                                        (gpointer *) org_key,
                                        (gpointer *) org_value);

}

static void
hash_insert(StrPrt *key, STORE_STATICS_ARRAY tab) {
    g_hash_table_insert(hash_table, (gpointer)key, (gpointer)tab) ;
}


static void
hash_init() {
    hash_table =
        g_hash_table_new_full(hash_key,
                              hash_equal,
                              key_destroy,
                              value_destroy) ;
}


STORE_STATICS_ARRAY
get_tabc(StrPrt *key, char **MTCL) {
    STORE_STATICS_ARRAY tab = NULL;
    StrPrt *org_key;

    (void) hash_table_lookup(hash_table, key, (gpointer *) &org_key, (gpointer *) &tab);

    if (!tab)

    {
        tab = GetColorPartitionFromString(key->part, MTCL)  ;
        hash_insert(hash_table, org_key, tab);
    }

    return tabc;
}


int main() {
    /*
    StrPrt* key1= (StrPrt* ) malloc(sizeof( StrPrt)) ;
    key1->part = calloc(3, sizeof(char));
    key1->part[0] = '1';
    key1->part[1] = '1';
    key1->part[2] = '\0';
    key1->size = 3;

    StrPrt* key2= (StrPrt* ) malloc(sizeof( StrPrt)) ;
    key2->part = calloc(3, sizeof(char));
    key2->part[0] = '1';
    key2->part[1] = '1';
    key2->part[2] = '\0';
    key2->size = 3;

    StrPrt* key3= (StrPrt* ) malloc(sizeof( StrPrt)) ;
    key3->part = calloc(3, sizeof(char));
    key3->part[0] = '1';
    key3->part[1] = '3';
    key3->part[2] = '\0';
    key3->size = 3;

    GHashTable *hash_table = hash_init();

    hash_insert(hash_table, key1);

    hash_insert(hash_table, key2);

    hash_insert(hash_table, key3);

    StrPrt* org_key=NULL;

    hash_table_lookup(hash_table, key1,&org_key ) ;

    printf("size : %d , %s ",g_hash_table_size(hash_table), org_key->part);
    */

}
