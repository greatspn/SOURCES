#include <stdio.h>
#include <unistd.h> // for ftruncate
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/ealloc.h"
#include "../../INCLUDE/service.h"

#ifdef SWN
#ifdef REACHABILITY
#ifdef SYMBOLIC
#ifdef DEBUG_malloc
static int n_mlc_min = 0;
static int n_psh_min = 0;
static int n_pop_min = 0;
#endif

void fill_RIP(Cart_p  dyn,  Canonic_p  ptr,  Cart_p  token,  int  pl);

static int *first = NULL;
static int *fix = NULL;

static int value_limit = 0;

Canonic_p sfl_h = NULL;

Canonic_p fl_h = NULL;
Canonic_p fl_t = NULL;

Canonic_p old_fl_h = NULL;
Canonic_p old_fl_t = NULL;

Canonic_p free_min = NULL;

int first_computation = TRUE;

Cart_p token = NULL;   /* Da creare in get_canonic_data .. () */
Cart_p dyn = NULL;     /* Da creare in get_canonic_data .. () */
Cart_p rip = NULL;	      /* Da creare in get_canonic_data .. () */

extern void evaluate_schemes();
extern void evaluate_ord_schemes();
extern void evaluate_RIP();

extern void init_rip();

extern int get_dynamic_from_total();
extern int nextvec_with_fix();
extern int tot_rip;

extern FILE *f_cap_fp;
extern FILE *value_fp;
extern FILE *min_value_fp;

#define LST 0
#define CAP 1
#define RIP 2

#ifdef SYMBOLIC
double mark_ordinarie;      /* num. marc. ord. per marc. simbolica  */
#endif

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Cart_p create_cartesian_product(int  l,  int  w) {
    /* Init create_cartesian_product */
    int ii;
    Cart_p p = NULL;

    p = (Cart_p)emalloc(sizeof(struct CARTESIAN_PRODUCT));
    p->low = p->up = NULL;
    p->mark = NULL;
    if (l > 0) {
        p->low = (int *)ecalloc(l, sizeof(int));
        p->up = (int *)ecalloc(l, sizeof(int));
        if (w > 0) {
            p->mark = (int **)ecalloc(l, sizeof(int *));
            for (ii = 0; ii < l; ii++)
                p->mark[ii] = (int *)ecalloc(w, sizeof(int));
        }
    }
    return (p);
}/* End create_cartesian_product */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void create_canonical_data_structure() {
    /* Init create_canonical_data_structure */
    int dim;

    if (ncl != 0) {
        first = (int *)ecalloc(ncl, sizeof(int));
        fix = (int *)ecalloc(ncl, sizeof(int));
    }
    dim = MAX(ncl, max_place_domain);
    dyn = create_cartesian_product(dim, 0);
    token = create_cartesian_product(max_place_domain, 1);
    init_rip();
    rip = create_cartesian_product(tot_rip, 0);
}/* End create_canonical_data_structure */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void min_push(Canonic_p  ptr) {
    /* Init min_push */

    ptr->next = free_min;
    free_min = ptr;
#ifdef DEBUG_malloc
    n_psh_min++;
#endif
}/* End min_push */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
Canonic_p min_pop() {
    /* Init min_pop */
    Canonic_p p;

#ifdef DEBUG_malloc
    n_pop_min++;
#endif
    if (free_min == NULL) {
        int ii;

#ifdef DEBUG_malloc
        n_mlc_min++;
#endif
        p = (Canonic_p)emalloc(sizeof(struct MIN_STR));
        p->min = (int **)ecalloc(ncl, sizeof(int *));
        for (ii = 0; ii < ncl; ii++)
            p->min[ii] = (int *)ecalloc(tabc[ii].card, sizeof(int));
        p->ord = (int *)ecalloc(max_cardinality, sizeof(int));
        p->file_pos = (int *)ecalloc(max_cardinality, sizeof(int)); //original verson
        //   p->file_pos = (int *)malloc(max_cardinality*sizeof(int));
    }
    else {
        p = free_min;
        free_min = free_min->next;
    }
    p->next = NULL;
    p->first = 0;
    return (p);
}/* End min_pop */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void fill_diagonal(dyn, tok, pl)
Cart_p dyn;
Cart_p tok;
int pl;
{
    /* Init fill_diagonal */
    int ii, cl;

    for (ii = GET_PLACE_COMPONENTS(pl); ii ; ii--) {
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
        tok->low[ii - 1] = tok->up[ii - 1] = 0;
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
            tok->mark[ii - 1][0] = get_dynamic_from_total(cl, dyn->low[cl]);
        else
            tok->mark[ii - 1][0] = dyn->low[cl];
    }
}/* End fill_diagonal */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void fill_CAP(dn, ptr, tok, pl)
Cart_p dn;
Canonic_p ptr;
Cart_p tok;
int pl;
{
    /* Init fill_CAP */
    int ii, cl;

    for (ii =  ncl; ii ; first[--ii] = 0);
    for (ii = GET_PLACE_COMPONENTS(pl) ; ii ; ii--) {
        cl = GET_COLOR_COMPONENT(ii - 1, pl);
        tok->low[ii - 1] = tok->up[ii - 1] = 0;
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
            tok->mark[ii - 1][0] = first[cl] ? ptr->min[cl][0] : ptr->min[cl][dn->low[cl]];
        else
            tok->mark[ii - 1][0] = dn->low[cl];
        first[cl]++;
    }
}/* End fill_CAP */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/* NOTE : Check if it is always necessary to "rewind" the file */
/***************************************************************/
void get_marqsp(Cart_p  dn,  Canonic_p  ptr,  FILE   *fp,  int  type) {
    /* Init get_marqsp */
    int pl;

    if (type != CAP)
        rewind(fp);
    value_limit = 0;
    for (pl = 0 ; pl < npl ; pl++) {
        /* Calcolo del valore per ogni posto */
        if (IS_COLOURED(pl)) {
            switch (type) {
            case LST: fill_diagonal(dn, token, pl);
                break;
            case CAP: fill_CAP(dn, ptr, token, pl);
                break;
            case RIP: fill_RIP(dn, ptr, token, pl);
                break;
            }
            int v = get_marking_of(token, pl, TRUE);
            value_limit += fprintf(fp, "*%d", v);
        }
    }/* Calcolo del valore per ogni posto */
}/* End get_marqsp */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void push_list(Canonic_p  pt) {
    /* Init push_list */

    if (fl_h == NULL) {
        fl_h = fl_t = pt;
        fl_t->next = NULL;
    }
    else {
        fl_t->next = free_min;
        free_min = fl_h;
        fl_h = fl_t = pt;
        fl_t->next = NULL;
    }
}/* End push_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void push_list_of_first(dn)
Cart_p dn;
{
    /* Init push_list_of_first */
    int ii;
    Canonic_p pt = NULL;

    pt = min_pop();
    push_list(pt);
    for (ii = ncl ; ii ; ii--)
        if (IS_UNORDERED(ii - 1) || (IS_ORDERED(ii - 1) && GET_STATIC_SUBCLASS(ii - 1) == 1))
            fl_h->min[ii - 1][0] = get_dynamic_from_total(ii - 1, dn->low[ii - 1]);
        else
            fl_h->min[ii - 1][0] = dn->low[ii - 1];
}/* End push_list_of_first */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void insert_first(Canonic_p  pt) {
    /* Init insert_first */

    if (fl_h == NULL) {
        fl_h = fl_t = pt;
        fl_h->next = NULL;
    }
    else {
        pt->next = fl_h;
        fl_h = pt;
    }
}/* End insert_first */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void in_MIN_list(dn)
Cart_p dn;
{
    /* Init in_MIN_list */
    int ii;
    Canonic_p pt = NULL;

    pt = min_pop();
    for (ii = ncl ; ii ; ii--)
        if (IS_UNORDERED(ii - 1) || (IS_ORDERED(ii - 1) &&
                                     GET_STATIC_SUBCLASS(ii - 1) == 1))
            pt->min[ii - 1][0] = get_dynamic_from_total(ii - 1, dn->low[ii - 1]);
        else
            pt->min[ii - 1][0] = dn->low[ii - 1];
    insert_first(pt);
}/* End in_MIN_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/* NOTE : The content of both files has the same length.      */
/**************************************************************/
int file_cmp(FILE   *fp1,  FILE   *fp2) {
    /* Init file_cmp */
    int v = EQUAL_TO;
    int val1, val2;

    if (first_computation) {
        first_computation = FALSE;
        v = LESSER_THAN;
        goto ret;
    }
    rewind(fp1); rewind(fp2);
    getc(fp1); getc(fp2);
    while (TRUE) {
        fscanf(fp1, "%d", &val1);
        fscanf(fp2, "%d", &val2);
        if (val1 < val2) {
            v = LESSER_THAN;
            goto ret;
        }
        else if (val1 > val2) {
            v = GREATER_THAN;
            goto ret;
        }
        getc(fp1);
        getc(fp2);
        if (feof(fp1) || feof(fp2))
            break;
    }
ret: return (v);
}/* End file_cmp */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void get_min_value(dn)
Cart_p dn;
{
    /* Init get_min_value */
    FILE *swap = NULL;
    int status, nof;

    int desc = fileno(value_fp);
    ftruncate(desc, 0);
    desc = fileno(min_value_fp);
    ftruncate(desc, 0);

    first_computation = TRUE;
    do {
        /* Per ogni ennupla tra cui cercare il minimo */
        get_marqsp(dn, NULL, value_fp, LST);
        status = file_cmp(value_fp, min_value_fp);
        switch (status) {
        /* Funzione marqsp <,= o > */
        case LESSER_THAN : push_list_of_first(dn);
            swap = min_value_fp;
            min_value_fp = value_fp;
            value_fp = swap;
            nof = 1;
            break;
        case EQUAL_TO : in_MIN_list(dn);
            nof++;
            break;
        }/* Funzione marqsp <,= o > */
    }/* Per ogni ennupla tra cui cercare il minimo */
    while (nextvec(dn, ncl));
}/* End get_min_value */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void f_cap(ptr, ds, cl, base, fp)
Canonic_p ptr;
int ds;
int cl;
int base;
FILE *fp;
{
    /* Init f_cap */
    int i;

    for (i = cl; i ; i--) {
        if (IS_UNORDERED(i - 1) || (IS_ORDERED(i - 1) &&
                                    GET_STATIC_SUBCLASS(i - 1) == 1))
            dyn->up[i - 1] = GET_NUM_CL(i - 1) - 1;
        else
            dyn->up[i - 1] = tabc[i - 1].card - 1;
        dyn->low[i - 1] = 0;
        fix[i - 1] = FALSE;
    }
    dyn->low[cl] = dyn->up[cl] = base;
    fix[cl] = TRUE;
    for (i = cl + 1 ; i < ncl ; i++) {
        fix[i] = FALSE;
        dyn->low[i] = dyn->up[i] = 0;
    }

    int sbc = get_static_subclass(cl, ds);
    int cardinal = card[cl][sbc][ds - tabc[cl].sbclist[sbc].offset];

    fprintf(fp, "%d", ds);
    do {
        /* Concatenazione delle marqsp */
        get_marqsp(dyn, ptr, fp, CAP);
        fprintf(fp, "*"); // old_version
        // fprintf(fp,"*%d*",cardinal);
    }/* Concatenazione delle marqsp */
    while (nextvec_with_fix(dyn, ncl, fix));
    fprintf(fp, "*%d*", cardinal);
    fprintf(fp, "\n");

}/* End f_cap */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_all_value() {
    /* Init get_all_value */
    Canonic_p ptr = NULL;
    int sb, cl, ds, i, id, base, K;

    for (cl = 0; cl < ncl; cl++) {
        /* Per ogni colore */
        if (IS_UNORDERED(cl)) {
            /* per classi non ordinate */
            base = 1;
            for (sb = 0 ; sb < GET_STATIC_SUBCLASS(cl) ; sb++) {
                /* Per ogni sottoclasse statica */
                int desc = fileno(f_cap_fp);
                ftruncate(desc, 0);
                rewind(f_cap_fp);
                fprintf(f_cap_fp, "0*999*\n");
                for (ptr = fl_h; ptr != NULL; ptr = ptr->next) {
                    /* Per ogni schema */
                    i = 0;
                    for (ds = 0; ds < GET_NUM_SS(cl, sb) ; ds++) {
                        id = GET_STATIC_OFFSET(cl, sb) + ds;
                        if (id != ptr->min[cl][0]) {
                            ptr->file_pos[base + i] = ftell(f_cap_fp);
                            ptr->min[cl][base] = id;
                            f_cap(ptr, id, cl, base, f_cap_fp);
                            i++;
                        }
                    }
                }/* Per ogni schema */
                evaluate_schemes(cl, sb, base);
                /*base += GET_NUM_SS(cl,sb) - 1;*/
                if (sb == 0)
                    base = GET_NUM_SS(cl, sb);
                else
                    base += GET_NUM_SS(cl, sb);
            }/* Per ogni sottoclasse statica */
        } /* per classi non ordinate */
        else if (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1) {
            /* per classi ordinate */
            int desc = fileno(f_cap_fp);
            ftruncate(desc, 0);
            rewind(f_cap_fp);
            fprintf(f_cap_fp, "0*999*\n");
            for (ptr = fl_h; ptr != NULL; ptr = ptr->next) {
                /* Per ogni schema */
                base = 1;
                i = ptr->min[cl][0];
                for (ds = i + 1; ds < GET_NUM_CL(cl); ds++) {
                    ptr->min[cl][base] = ds;
                    ptr->file_pos[base] = ftell(f_cap_fp);
                    f_cap(ptr, ds, cl, base, f_cap_fp);
                    base++;
                }
                for (ds = 0 ; ds < i; ds++) {
                    ptr->min[cl][base] = ds;
                    ptr->file_pos[base] = ftell(f_cap_fp);
                    f_cap(ptr, ds, cl, base, f_cap_fp);
                    base++;
                }
            }/* Per ogni schema */
            evaluate_ord_schemes(cl);
        }/* per classi ordinate */
    }/* Per ogni colore */
    for (ptr = fl_h , K = 0 ; ptr != NULL ; K++ , ptr = ptr->next);
    if ((K > 1) && (tot_rip > ncl)) {
        /* eliminazione di schemi quando si controllano le ripetizioni */
        evaluate_RIP(fl_h);
        for (ptr = fl_h , K = 0 ; ptr != NULL ; K++ , ptr = ptr->next);
    }
    return (K);
}/* End get_all_value */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void release_canonic_list() {
    /* Init release_canonic_list */

    sfl_h = fl_h;
    fl_t->next = free_min;
    free_min = fl_h;
    fl_h = fl_t = NULL;
}/* End release_canonic_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static void fill_dynamic_subclasses(dn)
Cart_p dn;
{
    /* Init fill_dynamic_subclasses */
    int cc;

    for (cc = 0; cc < ncl ; cc++) {
        /* Per ogni classe */
        dn->low[cc] = 0;
        if (IS_UNORDERED(cc) || (IS_ORDERED(cc) && GET_STATIC_SUBCLASS(cc) == 1))
            dn->up[cc] = GET_NUM_SS(cc, 0) - 1;
        else
            dn->up[cc] = 0;
        /*    if( IS_ORDERED(cc) )
             dn->up[cc] = 0;
            else
             dn->up[cc] = GET_NUM_SS(cc,0) - 1;
            if( IS_ORDERED(cc)  &&  GET_STATIC_SUBCLASS(cc) > 1)
             dn->up[cc] = 0; */
    }/* Per ogni classe */
}/* End fill_dynamic_subclasses */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static double factorial(n)
int n;
{
    /* Init factorial */
    double m = 1;

//   if (n > 0 && n <= 32) {<--- Perche' questa soglia a 30!!!!!!
    if (n > 0) {
        while (n) {
            m *= n;
            n--;
        }
    }
    return (m);
}/* End factorial */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
double get_ordinary_marking(k)
int k;
{
    /* Init get_ordinary_marking */
    double sb_f;
    int sb;
    double prod_dsc, prod_sb, prod_cl;
    int i, j;

    prod_cl = 1;
    for (i = 0; i < ncl; i++) {
        /* per ogni classe */
        if (tabc[i].type == UNORDERED) {
            /* per classi non ordinate */
            prod_sb = 1;
            for (sb = 0; sb < tabc[i].sbc_num; sb++) {
                sb_f = factorial(tabc[i].sbclist[sb].card);
                prod_dsc = 1;
                for (j = 0; j < GET_NUM_SS(i, sb); j++)
                    prod_dsc *= factorial(GET_CARD_BY_TOTAL(i, sb, j));
                prod_sb *= sb_f / prod_dsc;
            }
        }/* per classi non ordinate */
        else {
            /* per classi ordinate */
            if (tabc[i].sbc_num > 1 || (tabc[i].sbc_num == 1 && GET_NUM_CL(i) == 1))
                prod_sb = 1;
            else
                prod_sb = tabc[i].card;
        } /* per classi ordinate */
        prod_cl *= prod_sb;
    } /* per ogni classe */
#ifdef TOGLIERE
    if ((int)(prod_cl - (int)((prod_cl / k))*k) != 0) {
        /* Errore nella canonica */
        fprintf(stdout, "error: non integral division\n");
        /*fprintf(stdout,"%d\n",(int) ((prod_cl - (int) (prod_cl/k))*k) );
        exit(1);*/
    }/* Errore nella canonica */
#endif

    return (prod_cl / k);
}/* End get_ordinary_marking */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_canonical_marking() {
    /* Init get_canonical_marking */
    int v;

    fill_dynamic_subclasses(dyn);
    get_min_value(dyn);
    v = get_all_value();
    mark_ordinarie = get_ordinary_marking(v);
    release_canonic_list();

}/* End get_canonical_marking */
#ifdef DEBUG_malloc
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_canonic_info() {
    /* Init out_canonic_info */
    fprintf(stdout, "--------------- CANONIC STRUCTURE ------------------\n");
    fprintf(stdout, "MALLOC %d\nPUSH %d\nPOP %d\n"
            , n_mlc_min, n_psh_min, n_pop_min);
}/* End out_canonic_info */
#endif
#endif
#endif
#endif
