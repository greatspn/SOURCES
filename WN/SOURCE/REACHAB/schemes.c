#include <stdio.h>
#include <stdlib.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/service.h"
#include "../../INCLUDE/ealloc.h"

#ifdef SWN
#ifdef REACHABILITY
#ifdef SYMBOLIC
static int *pl_rip = NULL;
static int *first = NULL;
static int off_rep = 0;
static int *rep = NULL;

int tot_rip = 0;
static int *max_rip = NULL;
static int *class_rip = NULL;
static int **offset_rip = NULL;

static Canonic_p rep_ptr = NULL;
static Canonic_p temp_list_h = NULL;
static Canonic_p temp_list_t = NULL;

static int P[10], M[10], D[10], POS[10];
static int N = 5;

extern Canonic_p fl_h;
extern Canonic_p fl_t;
extern Canonic_p old_fl_h;
extern Canonic_p old_fl_t;
extern Canonic_p free_min;

extern Cart_p rip;

extern FILE *f_cap_fp;
extern FILE *value_fp;
extern FILE *min_value_fp;

extern int first_computation;

extern void min_push();
extern void push_list();
extern void insert_first();
extern void get_marqsp();

extern Canonic_p min_pop();

extern int file_cmp();

#define LEFT -1
#define RIGHT 1
#define RIP 2
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
/* static int file_cap_cmp(op1,op2) */
/*  int *op1; */
/*  int *op2; */
/*  {/ * Init file_cap_cmp */
/*   char cc1,ccn1; */
/*   char cc2,ccn2; */
/*   int v = EQUAL_TO; */
/*   int val1,val2; */
/*   int op_v1,op_v2; */

/*   if(first_computation) */
/*    { */
/*     first_computation = FALSE; */
/*     v = LESSER_THAN; */
/*     goto ret; */
/*    } */
/*   op_v1 = *(op1) + 2; */
/*   op_v2 = *(op2) + 2; */


/*   while(TRUE) */
/*    { */
/*     if(fseek(f_cap_fp,op_v1,SEEK_SET)== -1 ) */
/*       perror("op_v1 incorrecte ");  */
/*     if( fscanf(f_cap_fp,"%d",&val1)==0)  */
/*       { */
/* 	char s[256]; */
/*         int pos=ftell(f_cap_fp); */
/* 	fscanf(f_cap_fp,"%s",s); */
/* 	printf("val1 : %s \n",s); */
/* 	perror("val1 incorrecte "); */
/*       } */
/*     cc1 = getc(f_cap_fp); */
/*     ccn1 = getc(f_cap_fp); */

/*     if(ccn1 != '*') ungetc(ccn1,f_cap_fp); */
/*     op_v1 = ftell(f_cap_fp); */

/*     if( fseek(f_cap_fp,op_v2,SEEK_SET)== -1) */
/*        perror("op_v2 incorrecte ");  */
/*     if(  fscanf(f_cap_fp,"%d",&val2)==0) */
/*       { */
/* 	char s[256]; */
/*         int pos=ftell(f_cap_fp); */
/* 	fscanf(f_cap_fp,"%s",s); */
/* 	printf("val2 : %s \n",s); */

/*        perror("val2 incorrecte"); */
/*       } */
/*     cc2 = getc(f_cap_fp); */
/*     ccn2 = getc(f_cap_fp); */
/*     if(ccn2 != '*') ungetc(ccn2,f_cap_fp); */
/*     op_v2 = ftell(f_cap_fp); */

/*     if(val1 < val2) */
/*      { */
/*       v = LESSER_THAN; */
/*       goto ret; */
/*      } */
/*     else if (val1 > val2) */
/*      { */
/*       v = GREATER_THAN; */
/*       goto ret; */
/*      } */
/*     if(ccn1 == '\n' || ccn2 == '\n') */
/*      break; */
/*    } */
/* ret:return(v); */
/*  } *//* End file_cap_cmp */

static int value(char **ptr) {
    int v = 0, r = 0;
    char c[1] = "\0" ;

    c[0] = (*ptr)[0]; (*ptr)++;
    v = atoi(c);
    while ((*ptr)[0] != '*') {
        c[0] = (*ptr)[0];
        r = atoi(c);
        v = (v * 10) + r;
        (*ptr)++;
    }

    return (v);
}

static int next_tok(char **ptr) {

    while ((*ptr)[0] == '*')
        (*ptr) ++;

    if ((*ptr)[0] == '\n')
        return TRUE ;
    else
        return FALSE;
}

/* static int file_cap_cmp(op1,op2) */
/*  int *op1; */
/*  int *op2; */
/*  {/ * Init file_cap_cmp */
/*   char cc1,ccn1; */
/*   char cc2,ccn2; */
/*   int v = EQUAL_TO; */
/*   int val1,val2; */
/*   int op_v1,op_v2; */

/*   if(first_computation) */
/*    { */
/*     first_computation = FALSE; */
/*     v = LESSER_THAN; */
/*     return (v); */
/*    } */

/*   int size=6*1024; */
/*   char* str1=(char*)ecalloc(size,sizeof(char)) ; */
/*   char* str2=(char*)ecalloc(size,sizeof(char)); */
/*   char* str_op1=str1; */
/*   char* str_op2=str2; */

/*   fseek(f_cap_fp,*op1,SEEK_SET); */
/*   fgets(str_op1,size ,f_cap_fp); */
/*   fseek(f_cap_fp,*op2,SEEK_SET); */
/*   fgets(str_op2,size ,f_cap_fp); */

/*   val1=value(&str_op1);str_op1++; */
/*   val2=value(&str_op2);str_op2++; */

/*   while(TRUE) */
/*    { */

/*      val1=value(&str_op1); */
/*      val2=value(&str_op2); */

/*     if(val1 < val2) */
/*      { */
/*       v = LESSER_THAN; */
/*       break; */
/*      } */
/*     else if (val1 > val2) */
/*      { */
/*       v = GREATER_THAN; */
/*       break; */
/*      } */
/*     if(next_tok(&str_op1) || next_tok(&str_op2)) */
/*      break; */
/*    } */
/*   free(str1); */
/*   free(str2); */
/*   return(v); */
/*}*/
static int file_cap_cmp(op1, op2)
int *op1;
int *op2;
{
    char cc1, ccn1;
    char cc2, ccn2;
    int v = EQUAL_TO;
    int val1, val2;
    int op_v1, op_v2;
    char *str1 = NULL;
    size_t size_str1 = 0;
    char *str2 = NULL;
    size_t size_str2 = 0;

    if (first_computation) {
        first_computation = FALSE;
        v = LESSER_THAN;
        return (v);
    }


    fseek(f_cap_fp, *op1, SEEK_SET);
    getline(&str1, &size_str1, f_cap_fp);
    fseek(f_cap_fp, *op2, SEEK_SET);
    getline(&str2, &size_str2, f_cap_fp);
    char *str_op1 = str1;
    char *str_op2 = str2;
    val1 = value(&str_op1); str_op1++;
    val2 = value(&str_op2); str_op2++;

    while (TRUE) {

        val1 = value(&str_op1);
        val2 = value(&str_op2);

        if (val1 < val2) {
            v = LESSER_THAN;
            break;
        }
        else if (val1 > val2) {
            v = GREATER_THAN;
            break;
        }
        if (next_tok(&str_op1) || next_tok(&str_op2))
            break;
    }
    free(str1);
    free(str2);
    return (v);
}

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void sort(a, n)
int *a;
int n;
{
    /* Init sort */
    int i, j, k, min;

    if (n > 1) {

        for (i = 0; i < n; i++) {
            min = i;
            for (j = i + 1; j < n; j++) {
                if (file_cap_cmp(&a[j], &a[min]) == LESSER_THAN)
                    min = j;
            }
            if (i != min)
            { k = a[i]; a[i] = a[min]; a[min] = k; }
        }
    }
}/* End sort */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void encode_rot(ptr, fp, cl)
Canonic_p ptr;
FILE *fp;
int cl;
{
    /* Init encode_rot */
    int ds;

    rewind(fp);
    for (ds = 0 ; ds < GET_NUM_CL(cl) ; ds++)
        fprintf(fp, "%d*", GET_CARD(cl, 0, ptr->min[cl][ds]));
}/* End encode_rot */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void evaluate_ord_schemes(int  cl) {
    /* Init evaluate_ord_schemes */
    Canonic_p mp;
    Canonic_p nxt;
    Canonic_p cmp_ptr;
    FILE *swap = NULL;
    int f_min, f_max, status;
    int nof = MAX_INT, pos;

    for (pos = 1 ; pos < GET_NUM_CL(cl) && nof > 1 ; pos++) {
        old_fl_h = fl_h;
        old_fl_t = fl_t;
        fl_h = NULL;
        fl_t = NULL;
        first_computation = TRUE;
        for (mp = old_fl_h ; mp != NULL; mp = nxt) {
            /* Per ogni valore minimo della lista */
            nxt = mp->next;
            f_max = mp->file_pos[pos];
            status = file_cap_cmp(&f_max, &f_min);
            switch (status) {
            /* Funzione marqsp <,= o > */
            case LESSER_THAN : push_list(mp);
                f_min = f_max;
                nof = 1;
                break;
            case EQUAL_TO : insert_first(mp);
                nof++;
                break;
            case GREATER_THAN : min_push(mp);
                break;
            }/* Funzione marqsp <,= o > */
        }/* Per ogni valore minimo della lista */
    }
#ifdef DARIVEDERE
    if (nof > 1 && nof != MAX_INT) {
        /* Piu' di uno schema */
        mp = fl_h;
        encode_rot(mp, min_value_fp, cl);
        old_fl_h = fl_h;
        old_fl_t = fl_t;
        fl_h = NULL;
        fl_t = NULL;
        cmp_ptr = old_fl_h->next;
        push_list(mp);
        for (; cmp_ptr != NULL; cmp_ptr = nxt) {
            nxt = cmp_ptr->next;
            encode_rot(cmp_ptr, value_fp, cl);
            status = file_cmp(value_fp, min_value_fp);
            switch (status) {
            case GREATER_THAN : swap = value_fp;
                value_fp = min_value_fp;
                min_value_fp = swap;
                push_list(cmp_ptr);
                break;
            case EQUAL_TO : insert_first(cmp_ptr);
                break;
            case LESSER_THAN : min_push(cmp_ptr);
                break;
            }
        }
    }/* Piu' di uno schema */
#endif
}/* End evaluate_ord_schemes */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_repetitions(cl, sb, base)
int cl;
int sb;
int base;
{
    /* Init get_repetitions */
    int i, status;
    int f_1, f_2;
    int cnt = FALSE;

    off_rep = 0;
    for (i = 0 ; i < max_cardinality ; rep[i++] = 1);
    for (i = 0 ; i < GET_NUM_SS(cl, sb) - 1 - (sb == 0) ; i++) {
        f_1 = fl_h->file_pos[base + i];
        f_2 = fl_h->file_pos[base + i + 1];
        status = file_cap_cmp(&f_1, &f_2);
        switch (status) {
        /* Funzione marqsp <,= o > */
        case EQUAL_TO : rep[off_rep]++;
            cnt = TRUE;
            break;
        default : off_rep++;
            break;
        }/* Funzione marqsp <,= o > */
    }
    return (cnt);
}/* End get_repetitions */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void copy_schemes(pt, ptr, base, ss, bs_rep, cl)
Canonic_p pt;
Canonic_p ptr;
int base;
int ss;
int bs_rep;
int cl;
{
    /* Init copy_schemes */
    int jj, ds;
    /********************** COPIA SCHEMI ******************************/
    /* Copia per le classi gia' ordinate (precedenti) */
    for (jj = 0 ; jj < cl ; jj++)
        for (ds = 0 ; ds < GET_NUM_CL(jj) ; ds++)
            pt->min[jj][ds] = rep_ptr->min[jj][ds];
    /* Copia per le sottoclassi statiche gia' ordinate (precedenti) */
    for (jj = 0 ; jj < base ; jj++)
        pt->min[cl][jj] = rep_ptr->min[cl][jj];
    /* Copia per le sottoclassi dinamiche di sb gia' permutate */
    for (jj = 0 ; jj < ss ; jj++)
        for (ds = 0 ; ds < bs_rep ; ds++)
            pt->min[cl][ds + base] = rep_ptr->min[cl][ds + base];
    /* Copia dei minimi delle classi successive */
    for (jj = cl; jj < ncl; jj++)
        pt->min[jj][0] = ptr->min[jj][0];
    /**** Permutazione identita' *******/
    for (jj = 0 ; jj < rep[ss] ; jj++)
        pt->min[cl][base + bs_rep + jj] = ptr->min[cl][base + bs_rep + P[jj + 1] - 1];
    /**** Aggancio alla lista dei minimi' *******/
    if (temp_list_h == NULL) {
        temp_list_h = temp_list_t = pt;
        temp_list_h->next = NULL;
    }
    else {
        pt->next = temp_list_h;
        temp_list_h = pt;
    }
}/* End copy_schemes */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void replicate_schemes(ptr, cl, sb, base)
Canonic_p ptr;
int cl;
int sb;
int base;
{
    /* Init replicate_schemes */

    int ii, jj, kk, mm, ss;
    int buf, not_found;
    int id_1, id_2, bs_rep, ds;

    Canonic_p buf_list_h = NULL;
    Canonic_p buf_list_t = NULL;
    Canonic_p nxt_ptr = NULL;
    Canonic_p pt = NULL;

    rep_ptr = NULL;
    temp_list_h = NULL;
    temp_list_t = NULL;
    N = 5;
    for (ii = 0 ; ii < GET_NUM_SS(cl, sb) - (sb == 0) ; ii++) {
        fseek(f_cap_fp, ptr->file_pos[base + ii], SEEK_SET);
        fscanf(f_cap_fp, "%d", &ds);
        ptr->min[cl][base + ii] = ds;
    }
    buf_list_h = buf_list_t = min_pop();
    buf_list_t->next = NULL;
    for (ii = 0 ; ii < cl ; ii++)
        for (jj = 0 ; jj < GET_NUM_CL(ii) ; jj++)
            buf_list_h->min[ii][jj] = ptr->min[ii][jj];
    for (jj = 0 ; jj < base ; jj++)
        buf_list_h->min[cl][jj] = ptr->min[cl][jj];
    for (ss = 0 , bs_rep = 0 ; ss <= off_rep ; ss++) {
        for (rep_ptr = buf_list_h ; rep_ptr != NULL; rep_ptr = nxt_ptr) {
            nxt_ptr = rep_ptr->next;
            N = rep[ss];
            for (ii = 1 ; ii <= N ; ii++) {
                P[ii] = ii;
                POS[ii] = ii;
                M[ii] = TRUE;
                D[ii] = LEFT;
            }
            P[0] = P[N + 1] = N + 1;
            M[1] = FALSE;
            pt = min_pop();
            copy_schemes(pt, ptr, base, ss, bs_rep, cl);
            not_found = FALSE;
            for (; !not_found;) {
                not_found = TRUE;
                for (ii = N ; ii >= 1 && not_found ; ii--)
                    if (M[ii]) {
                        mm = ii;
                        not_found = FALSE;
                    }
                if (!not_found) {
                    buf = P[POS[mm] + D[mm]];
                    id_1 = buf;
                    id_2 = P[POS[mm]];
                    P[POS[mm] + D[mm]] = P[POS[mm]];
                    P[POS[mm]] = buf;
                    buf = POS[id_1];
                    POS[id_1] = POS[id_2];
                    POS[id_2] = buf;
                    pt = min_pop();
                    copy_schemes(pt, ptr, base, ss, bs_rep, cl);
                    for (kk = mm + 1 ; kk <= N ; kk++)
                        D[kk] = -D[kk];
                    for (kk = 1 ; kk <= N ; kk++)
                        if (P[POS[kk] + D[kk]] < P[POS[kk]])
                            M[kk] = TRUE;
                        else
                            M[kk] = FALSE;
                }
            }
            min_push(rep_ptr);
        }
        buf_list_h = temp_list_h;
        buf_list_t = temp_list_t;
        if (ss < off_rep) {
            temp_list_h = NULL;
            temp_list_t = NULL;
        }
        bs_rep += rep[ss];
    }
    if (fl_h == NULL) {
        fl_h = temp_list_h;
        fl_t = temp_list_t;
        fl_t->next = NULL;
    } // int desc= fileno(f_cap_fp);
    // ftruncate(desc,0);
    else {
        fl_t->next = temp_list_h;
        fl_t = temp_list_t;
    }
    min_push(ptr);
}/* End replicate_schemes */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void evaluate_schemes(int  cl,  int  sb,  int  base) {
    /* Init evaluate_schemes */
    Canonic_p mp;
    Canonic_p nxt;
    int f_min, f_max, status, ds;
    int nof = 0 , pos;

    for (mp = fl_h ; mp != NULL ; mp = mp->next) {
        /* Per ogni valore minimo della lista */
        /*qsort((char *)mp->file_pos,GET_NUM_SS(cl,sb)-(sb==0),sizeof(int),file_cap_cmp); */
        if (sb == 0)
            sort(mp->file_pos + base, GET_NUM_SS(cl, sb) - 1);
        else
            sort(mp->file_pos + base, GET_NUM_SS(cl, sb));
        nof++;
    }/* Per ogni valore minimo della lista */
    for (pos = 0 ; pos < GET_NUM_SS(cl, sb) - (sb == 0) && nof > 1; pos++) {
        /* Per ogni sottoclasse dinamica */
        old_fl_h = fl_h;
        old_fl_t = fl_t;
        fl_h = NULL;
        fl_t = NULL;
        first_computation = TRUE;
        for (mp = old_fl_h ; mp != NULL ; mp = nxt) {
            /* Per ogni valore minimo della lista */
            nxt = mp->next;
            f_max = mp->file_pos[base + pos]; /* Aggiunto base a parigi */
            status = file_cap_cmp(&f_max, &f_min);
            switch (status) {
            /* Funzione marqsp <,= o > */
            case LESSER_THAN : push_list(mp);
                f_min = f_max;
                nof = 1;
                break;
            case EQUAL_TO : insert_first(mp);
                nof++;
                break;
            case GREATER_THAN : min_push(mp);
                break;
            }/* Funzione marqsp <,= o > */
        }/* Per ogni valore minimo della lista */
    }/* Per ogni sottoclasse dinamica */
    if (get_repetitions(cl, sb, base)) {
        old_fl_h = fl_h;
        old_fl_t = fl_t;
        fl_h = NULL;
        fl_t = NULL;
        for (mp = old_fl_h ; mp != NULL ; mp = nxt) {
            /* Per ogni valore minimo della lista */
            nxt = mp->next;
            replicate_schemes(mp, cl, sb, base);
        }/* Per ogni valore minimo della lista */
    }
    else {
        for (mp = fl_h ; mp != NULL ; mp = nxt) {
            /* Per ogni valore minimo della lista */
            nxt = mp->next;
            for (pos = 0 ; pos < GET_NUM_SS(cl, sb) - (sb == 0) ; pos++) {
                fseek(f_cap_fp, mp->file_pos[base + pos], SEEK_SET);
                fscanf(f_cap_fp, "%d", &ds);
                mp->min[cl][base + pos] = ds;
            }
        }/* Per ogni valore minimo della lista */
    }
}/* End evaluate_schemes */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void fill_RIP(Cart_p  dyn,  Canonic_p  ptr,  Cart_p  token,  int  pl) {
    /* Init fill_RIP */
    int ii, cl;

    for (ii = ncl ; ii ; first[--ii] = 0);
    for (ii = GET_PLACE_COMPONENTS(pl) ; ii ; ii--) {
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
        token->low[ii - 1] = token->up[ii - 1] = 0;
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
            token->mark[ii - 1][0] = ptr->min[cl][dyn->low[offset_rip[cl][first[cl]]]];
        else
            token->mark[ii - 1][0] = dyn->low[cl];
        first[cl]++;
    }
}/* End fill_RIP */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void evaluate_RIP(Canonic_p  mp) {
    /* Init evaluate_RIP */
    Canonic_p ptr;
    Canonic_p prec;
    FILE *swap;
    int ii, status;

    for (ii = 0 ; ii < tot_rip; ii++) {
        if (ii < ncl) {
            if (IS_UNORDERED(class_rip[ii]) || (IS_ORDERED(class_rip[ii]) && GET_STATIC_SUBCLASS(class_rip[ii]) == 1))
                rip->low[ii] = GET_NUM_CL(class_rip[ii]) - 1;
            else
                rip->low[ii] = tabc[class_rip[ii]].card - 1;
        }
        else
            rip->low[ii] = 0;
        if (IS_UNORDERED(class_rip[ii]) || (IS_ORDERED(class_rip[ii]) && GET_STATIC_SUBCLASS(class_rip[ii]) == 1))
            rip->up[ii] = GET_NUM_CL(class_rip[ii]) - 1;
        else
            rip->up[ii] = tabc[class_rip[ii]].card - 1;
    }
    /*nextvec(rip,tot_rip);*/
    do {
        prec = NULL;
        first_computation = TRUE;
        for (ptr = fl_h; ptr != NULL; ptr = ptr->next) {
            /* per tutti i minimi della lista dei minimi */
            get_marqsp(rip, ptr, value_fp, RIP);
            status = file_cmp(value_fp, min_value_fp);
            switch (status) {
            /* Funzione marqsp <,= o > */
            case LESSER_THAN:
                if (prec != NULL) {
                    prec->next = free_min;
                    free_min = fl_h;
                    fl_h = ptr;
                }
                prec = ptr;
                swap = value_fp;
                value_fp = min_value_fp;
                min_value_fp = swap;
                break;
            case EQUAL_TO :
                prec = ptr;
                break;
            case GREATER_THAN :
                if (prec == NULL) {
                    fprintf(stderr, "ERRORE in evaluate_RIP() \n");
                    exit(1);
                }
                if (ptr == fl_t)
                    fl_t = prec;
                prec->next = ptr->next;
                ptr->next = free_min;
                free_min = ptr;
                ptr = prec;

                break;
            }/* Funzione marqsp <,= o > */
        }/* Per ogni valore minimo della lista */
    }
    while (nextvec(rip, tot_rip));
}/* End evaluate_RIP */
/**************************************************************/
/* NAME :         init_rip()*/
/* DESCRIPTION :    Inizializza le strutture dati per le classi ripetute */
/*                  utilizzate nel calcolo della marcatura canonica */
/* PARAMETERS :     Nessuno*/
/* RETURN VALUE :   Nessuno*/
/**************************************************************/
void init_rip() {
    /* Inizio init_rip */
    int pl, cl, k;
    int all_zero, offset;

    rep = (int *)ecalloc(max_cardinality, sizeof(int));
    max_rip = (int *)ecalloc(ncl, sizeof(int));
    pl_rip = (int *)ecalloc(ncl, sizeof(int));
    first = (int *)ecalloc(ncl, sizeof(int));
    for (k = 0; k < ncl; max_rip[k++] = 0);
    for (pl = 0; pl < npl; pl++) {
        for (k = 0; k < ncl; pl_rip[k++] = 0);
        for (k = 0; k < GET_PLACE_COMPONENTS(pl); k++) {
            cl = tabp[pl].dominio[k];
            pl_rip[cl]++;
        }
        for (k = 0; k < ncl; k++)
            if (pl_rip[k] > max_rip[k])
                max_rip[k] = pl_rip[k];
    }
    for (k = 0, tot_rip = 0; k < ncl; tot_rip += max_rip[k++]);
    class_rip = (int *)ecalloc(tot_rip, sizeof(int));
    all_zero = TRUE;
    offset_rip = (int **)ecalloc(ncl, sizeof(int *));
    for (k = 0; k < ncl; k++) {
        offset_rip[k] = (int *) ecalloc(max_rip[k], sizeof(int));
        offset_rip[k][0] = k;
        class_rip[k] = k;
        pl_rip[k] = max_rip[k] - 1;
        all_zero = (all_zero & (pl_rip[k] == 0));
    }
    offset = ncl;
    while (!all_zero) {
        all_zero = TRUE;
        for (k = 0; k < ncl; k++)
            if (pl_rip[k] > 0) {
                offset_rip[k][max_rip[k] - pl_rip[k]] = offset;
                class_rip[offset++] = k;
                pl_rip[k]--;
                all_zero &= (pl_rip[k] == 0);
            }
    }
}
#endif
#endif
#endif
