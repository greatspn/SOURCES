/************************************************************/
/*   A recursive version of the algorithme which compute    */
/*   the eventualities and the functions which compute      */
/*             their compacte représentations               */
/************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"

extern MATRIX *MATRIX_3D_ALLOCATION();
/**********************************************************************/


#define  GET_MIN(FIRST,SECEND) (FIRST<=SECEND)?FIRST:SECEND
#define  GET_MAX(FIRST,SECEND) (FIRST>=SECEND)?FIRST:SECEND

/*********************************************************************/

void INIT_2D_MATRIX(MATRIX tmp) {
    int i, j, k, s;
    for (i = 0; i < MAX_CARD; i++)
        for (j = 0; j < MAX_CARD; j++)
            tmp[i][j] = 0;

}
void INIT_4D_MATRIX(MATRIX **tmp) {
    int i, j, k, s;
    for (i = 0; i < ncl; i++)
        for (j = 0; j < MAX_CARD; j++)
            for (k = 0; k < MAX_CARD; k++)
                for (s = 0; s < MAX_CARD; s++)
                    tmp[i][j][k][s] = 0;

}


void INIT_DSC_SSBC(void) {
    int i;
    TO_MERGEP ptr = NULL;

    for (i = 0; i < ncl; i++) {
        ptr = MERG[i];

        while (ptr) {
            NB_DSC[i][ptr->sbc_num] = num[i][ptr->sbc_num];
            NB_SSBC[i][ptr->sbc_num] = ptr->NB;
            ptr = ptr->next;
        }

    }

}

int C_L_PLUS_1(int col, int class, int sbc) {
    int i, tmp = 0;
    for (i = col; i < NB_SSBC[class][sbc]; i++) tmp += STATICS[class][sbc][i];
    return tmp;
}

int X_2(int col, int lin, int class, int sbc)

{
    int i, j, tmp = 0;
    for (i = 0 ; i < lin - 1 ; i++)
        for (j = col ; j < NB_SSBC[class][sbc] ; j++)
            tmp += RESULT[class][sbc][i][j];

    return tmp;
}

int X_L_MOINS_1(int col, int lin, int class, int sbc) {
    int j, tmp = 0;
    for (j = 0; j < col - 1; j++) tmp += RESULT[class][sbc][lin - 1][j];
    return (tmp);
}

int X_I_MOINS_1(int col, int lin, int class, int sbc) {
    int i, tmp = 0;
    for (i = 0; i < lin - 1; i++) tmp += RESULT[class][sbc][i][col - 1];
    return tmp;
}

void MY_CHAR_STORE(unsigned long nval) {
    register unsigned char cc;

    int space_left = MAX_CACHE - (int)(LP_ESRG - CACHE_STRING_ESRG)  - 1;

    if (nval < 64) {
        if (space_left < 1) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }

        cc = (unsigned char)(0x3F & nval);
        *LP_ESRG = cc;
        LP_ESRG++;
    }
    else if (nval < 16384) {
        if (space_left < 2) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0x40 | ((0x3F00 & nval) >> 8));
        *LP_ESRG = cc; LP_ESRG++;
        cc = (unsigned char)(0xFF & nval);
        *LP_ESRG = cc; LP_ESRG++;
    }
    else if (nval < 4194304) {
        if (space_left < 3) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0x80 | ((0x3F0000 & nval) >> 16));
        *LP_ESRG = cc; LP_ESRG++;
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        *LP_ESRG = cc; LP_ESRG++;
        cc = (unsigned char)(0xFF & nval);
        *LP_ESRG = cc; LP_ESRG++;
    }
    else if (nval < 1073741824) {
        if (space_left < 4) {
            fprintf(stdout, "Sorry: increase constant MAX_CACHE in file INCLUDE/const.h\n");
            exit(1);
        }
        cc = (unsigned char)(0xC0 | ((0x3F000000 & nval) >> 24));
        *LP_ESRG = cc; LP_ESRG++;
        cc = (unsigned char)((0xFF0000 & nval) >> 16);
        *LP_ESRG = cc; LP_ESRG++;
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        *LP_ESRG = cc;   LP_ESRG++;
        cc = (unsigned char)(0xFF & nval);
        *LP_ESRG = cc;  LP_ESRG++;
    }
    else {
        fprintf(stderr, "char_store_compact error: value overflow\n");
        exit(1);
    }
}/* End char_store_compact */

void CHAR_LOAD_COMPACT(unsigned long *pp) {
    register unsigned char cc0  ;
    register unsigned long uu = (cc0 & 0x3F);
    register unsigned long ii = (cc0 & 0xC0) >> 6;

    cc0 = *LP_ESRG;
    LP_ESRG++;
    uu = (cc0 & 0x3F);
    ii = (cc0 & 0xC0) >> 6;

    for (; ii ; ii--) {
        cc0 = *LP_ESRG;
        LP_ESRG++;
        uu = uu << 8;
        uu = uu + cc0;
    }
    *pp = uu;
}

#ifndef LIBSPOT
void Pertmutation_Store(int class) {

    int i;

    INIT_2D_MATRIX(RESULT [class][0]);

    for (i = 0; i < NB_DSC[class][0]; i++)
        RESULT [class][0][i][i] = DYNAMIC[class][0][i];


}
void EVENTUALITIES(int class, int sbc, int lin, int col, void (*traitement)()) {
    int Inf_born = 0;
    int Sup_born = 0;
    int i, j;


    int XLMOINS1 = X_L_MOINS_1(col, lin, class, sbc);
    Inf_born = GET_MAX(0, DYNAMIC[class][sbc][lin - 1] - XLMOINS1 - C_L_PLUS_1(col, class, sbc) + X_2(col, lin, class, sbc));
    Sup_born = GET_MIN(STATICS[class][sbc][col - 1] - X_I_MOINS_1(col, lin, class, sbc), DYNAMIC[class][sbc][lin - 1] - XLMOINS1);

    for (i = Sup_born; i >= Inf_born; i--) {

        RESULT[class][sbc][lin - 1][col - 1] = i;
        if ((lin == NB_DSC[class][sbc]) && (col == NB_SSBC[class][sbc])) {
            if ((class == ncl - 1) && (sbc == SYM_STATIC_STORE[class].sbc_num - 1))(*traitement)();
            if ((class != ncl - 1) && (sbc == SYM_STATIC_STORE[class].sbc_num - 1))   EVENTUALITIES(class + 1, 0, 1, 1, (*traitement));
            if (sbc != SYM_STATIC_STORE[class].sbc_num - 1)                     EVENTUALITIES(class, sbc + 1, 1, 1, (*traitement));
        }
        if ((lin != NB_DSC[class][sbc]) && (col == NB_SSBC[class][sbc]))          EVENTUALITIES(class, sbc, lin + 1, 1, (*traitement));
        if (col != NB_SSBC[class][sbc])                                       EVENTUALITIES(class, sbc, lin, col + 1, (*traitement));
    }


}
#else

void EVENTUALITIES(int class, int sbc, int lin, int col, void (*traitement)(), TO_MERGEP *merg, STORE_STATICS_ARRAY OLD_STATIC_CONF) {
    int Inf_born = 0;
    int Sup_born = 0;
    int i, j;
    int XLMOINS1 = X_L_MOINS_1(col, lin, class, sbc);

    Inf_born = GET_MAX(0, DYNAMIC[class][sbc][lin - 1] - XLMOINS1 - C_L_PLUS_1(col, class, sbc) + X_2(col, lin, class, sbc));
    Sup_born = GET_MIN(STATICS[class][sbc][col - 1] - X_I_MOINS_1(col, lin, class, sbc), DYNAMIC[class][sbc][lin - 1] - XLMOINS1);

    for (i = Sup_born; i >= Inf_born; i--) {

        RESULT[class][sbc][lin - 1][col - 1] = i;
        if ((lin == NB_DSC[class][sbc]) && (col == NB_SSBC[class][sbc])) {
            if ((class == ncl - 1) && (sbc == OLD_STATIC_CONF[class].sbc_num - 1))(*traitement)(merg, OLD_STATIC_CONF);
            if ((class != ncl - 1) && (sbc == OLD_STATIC_CONF[class].sbc_num - 1))   EVENTUALITIES(class + 1, 0, 1, 1, (*traitement), merg, OLD_STATIC_CONF);
            if (sbc != OLD_STATIC_CONF[class].sbc_num - 1)                     EVENTUALITIES(class, sbc + 1, 1, 1, (*traitement), merg, OLD_STATIC_CONF);
        }
        if ((lin != NB_DSC[class][sbc]) && (col == NB_SSBC[class][sbc]))        EVENTUALITIES(class, sbc, lin + 1, 1, (*traitement), merg, OLD_STATIC_CONF);
        if (col != NB_SSBC[class][sbc])                                     EVENTUALITIES(class, sbc, lin, col + 1, (*traitement), merg, OLD_STATIC_CONF);
    }

}

#endif


int NB_OF_EVENTUALITIES(int class, int sbc, int lin, int col) {
    int Inf_born = 0;
    int Sup_born = 0;
    int i, number = 0;
    int XLMOINS1 = X_L_MOINS_1(col, lin, class, sbc);


    Inf_born = GET_MAX(0, DYNAMIC[class][sbc][lin - 1] - XLMOINS1 - C_L_PLUS_1(col, class, sbc) + X_2(col, lin, class, sbc));
    Sup_born = GET_MIN(STATICS[class][sbc][col - 1] - X_I_MOINS_1(col, lin, class, sbc), DYNAMIC[class][sbc][lin - 1] - XLMOINS1);

    if ((lin == NB_DSC[class][sbc]) && (col == NB_SSBC[class][sbc])) return (1 + (Sup_born - Inf_born));
    else
        for (i = Sup_born; i >= Inf_born; i--) {
            RESULT[class][sbc][lin - 1][col - 1] = i;
            if (col == NB_SSBC[class][sbc])   number += NB_OF_EVENTUALITIES(class, sbc, lin + 1, 1);
            else                            number += NB_OF_EVENTUALITIES(class, sbc, lin, col + 1);
        }

    return number;
}

int ALL_NB_OF_EVENTUALITIES() {
    int i, j, res = 1;
    for (i = 0; i < ncl; i++)
        for (j = 0; j < SYM_STATIC_STORE[i].sbc_num; j++)
            res *= NB_OF_EVENTUALITIES(i, j, 1, 1);

    return res;
}

int GET_NB_SOLUTIONS_SYS_EQUA() {
    return NB_OF_EVENTUALITIES(0, 0, 1, 1);
}

#ifndef LIBSPOT

void EVENTUALITIE_TO_STRING() {
    int i, j, k, s;

    LP_ESRG = CACHE_STRING_ESRG;

    for (k = 0; k < ncl; k++)
        for (s = 0; s < SYM_STATIC_STORE[k].sbc_num; s++) {

            MY_CHAR_STORE((unsigned long)NB_DSC[k][s]);
            MY_CHAR_STORE((unsigned long)NB_SSBC[k][s]);
            for (i = 0; i < NB_DSC[k][s]; i++)
                for (j = 0; j < NB_SSBC[k][s]; j++)
                    MY_CHAR_STORE((unsigned long)RESULT[k][s][i][j]);

        }


    LEGTH_OF_CACHE = LP_ESRG - CACHE_STRING_ESRG;
}
#else

void EVENTUALITIE_TO_STRING(STORE_STATICS_ARRAY OLD_STATIC_CONF) {
    int i, j, k, s;

    LP_ESRG = CACHE_STRING_ESRG;

    for (k = 0; k < ncl; k++)
        for (s = 0; s < OLD_STATIC_CONF[k].sbc_num; s++) {
            MY_CHAR_STORE((unsigned long)NB_DSC[k][s]);
            MY_CHAR_STORE((unsigned long)NB_SSBC[k][s]);

            for (i = 0; i < NB_DSC[k][s]; i++)
                for (j = 0; j < NB_SSBC[k][s]; j++)
                    MY_CHAR_STORE((unsigned long)RESULT[k][s][i][j]);

        }
    LEGTH_OF_CACHE = LP_ESRG - CACHE_STRING_ESRG;
}
#endif

void PARM_EVENTUALITIE_TO_STRING(int sbc, int *tot_sbc,  MATRIX **EVENT) {
    int i, j, k, s;


    LP_ESRG = CACHE_STRING_ESRG;

    for (k = 0; k < ncl; k++)
        for (s = 0; s < sbc; s++) {

            MY_CHAR_STORE((unsigned long)NB_DSC[k][s]);
            MY_CHAR_STORE((unsigned long)tot_sbc[k]);
            for (i = 0; i < NB_DSC[k][s]; i++)
                for (j = 0; j < tot_sbc[k]; j++)
                    MY_CHAR_STORE((unsigned long)EVENT[k][s][i][j]);

        }


    LEGTH_OF_CACHE = LP_ESRG - CACHE_STRING_ESRG;
}


#ifndef LIBSPOT
void STRING_TO_EVENTUALITIE() {
    int i, j, k, s;

    INIT_4D_MATRIX(RESULT);
    LP_ESRG = CACHE_STRING_ESRG;

    for (k = 0; k < ncl; k++)

        for (s = 0; s < SYM_STATIC_STORE[k].sbc_num; s++)

        {
            CHAR_LOAD_COMPACT((unsigned long *)&NB_DSC[k][s]);
            CHAR_LOAD_COMPACT((unsigned long *)&NB_SSBC[k][s]);
            for (i = 0; i < NB_DSC[k][s]; i++)
                for (j = 0; j < NB_SSBC[k][s]; j++)
                    CHAR_LOAD_COMPACT((unsigned long *)&RESULT[k][s][i][j])  ;

        }
}

#else
void STRING_TO_EVENTUALITIE(STORE_STATICS_ARRAY OLD_STATIC_CONF) {
    int i, j, k, s;

// INIT_4D_MATRIX(RESULT);
    LP_ESRG = CACHE_STRING_ESRG;

    for (k = 0; k < ncl; k++)
        for (s = 0; s < OLD_STATIC_CONF[k].sbc_num; s++) {
            CHAR_LOAD_COMPACT((unsigned long *)&NB_DSC[k][s]);
            CHAR_LOAD_COMPACT((unsigned long *)&NB_SSBC[k][s]);
            for (i = 0; i < NB_DSC[k][s]; i++)
                for (j = 0; j < NB_SSBC[k][s]; j++)
                    CHAR_LOAD_COMPACT((unsigned long *)&RESULT[k][s][i][j])  ;

        }
}
#endif
void ADD_EVENTUALITIE_TO_FILE(int size) {
    fseek(EVENT_MARK, 0, SEEK_END);
    FILE_POS_PTR = ftell(EVENT_MARK);
    fwrite(CACHE_STRING_ESRG, 1, size, EVENT_MARK);

    COMPT_STORED_EVENT++;
}

void ADD_TEMP_EVENTUALITIE_TO_FILE(int size, FILE *fd) {
    fseek(fd, 0, SEEK_END);
    FILE_POS_PTR = ftell(fd);
    fwrite(CACHE_STRING_ESRG, 1, size, fd);
}
void GET_TEMP_EVENTUALITIE_FROM_FILE(int pos_ptr, int size, FILE *fd) {
    fseek(fd, pos_ptr, SEEK_SET);
    fread(compare, 1, size, fd);
}

void GET_EVENTUALITIE_FROM_FILE(int pos_ptr, int size) {
    fseek(EVENT_MARK, pos_ptr, SEEK_SET);
    fread(compare, 1, size, EVENT_MARK);
}

void COPY_CACHE(int size) {
    int i;

    for (i = 0; i < size; i++)CACHE_STRING_ESRG[i] = compare[i];
    LEGTH_OF_CACHE = size;
}

int COMPARE_EVENTUALITIES(int to_compare_ptr, int length_to_compare) {
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    if (LEGTH_OF_CACHE != length_to_compare) return NOT_EQUAL ;
    else {
        GET_EVENTUALITIE_FROM_FILE(to_compare_ptr, length_to_compare);
        cp = compare;
        pp = CACHE_STRING_ESRG;
        for (ii = length_to_compare; ii; ii--) {
            op1 = *cp; cp++;
            op2 = *pp; pp++;
            if (op1 != op2) return NOT_EQUAL;

        }
    }

    return EQUAL;
}

void NEW_EVENTUALITIE_TO_STRING(MATRIX **TEMP) {
    int i, j, k, s;


    LP_ESRG = CACHE_STRING_ESRG;

    for (k = 0; k < ncl; k++)
        for (s = 0; s < MAX_CARD; s++)
            for (i = 0; i < MAX_CARD; i++)
                for (j = 0; j < MAX_CARD; j++)
                    MY_CHAR_STORE((unsigned long)TEMP[k][s][i][j]);

    LEGTH_OF_CACHE = LP_ESRG - CACHE_STRING_ESRG;
}

int NEW_COMPARE_EVENTUALITIES(int ptr1, int length_ptr1,
                              int ptr2, int length_ptr2, FILE *fd) {
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    if (length_ptr1 != length_ptr2) return NOT_EQUAL ;
    else {
        fseek(fd, ptr1, SEEK_SET);
        fread(compare, 1, length_ptr1, fd);
        fseek(fd, ptr2, SEEK_SET);
        fread(CACHE_STRING_ESRG, 1, length_ptr2, fd);

        cp = compare;
        pp = CACHE_STRING_ESRG;
        for (ii = length_ptr1; ii; ii--) {
            op1 = *cp; cp++;
            op2 = *pp; pp++;
            if (op1 != op2) return NOT_EQUAL;

        }
    }

    return EQUAL;
}

int NEW_COMPARE_EVENTUALITIES_TEMP(int ptr1, int length_ptr1, FILE *fd1, int ptr2, int length_ptr2, FILE *fd2) {
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    if (length_ptr1 != length_ptr2) return NOT_EQUAL ;
    else {
        fseek(fd1, ptr1, SEEK_SET);
        fread(compare, 1, length_ptr1, fd1);
        fseek(fd2, ptr2, SEEK_SET);
        fread(CACHE_STRING_ESRG, 1, length_ptr2, fd2);

        cp = compare;
        pp = CACHE_STRING_ESRG;
        for (ii = length_ptr1; ii; ii--) {
            op1 = *cp; cp++;
            op2 = *pp; pp++;
            if (op1 != op2) return NOT_EQUAL;

        }
    }

    return EQUAL;
}
