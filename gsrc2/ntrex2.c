/*
 *  Paris, May 12, 1994
 *  program: ntrexp.c
 *  purpose: Transient solution of a GSPN MC with new summation algorithm
 *           for Randomization formula
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) Running much faster than previous algorithm: CPU time increases
 *       linearly as a function of the integration time.
 *    2) Same (excellent) numerical stability properties as previous
 *       implementation in case of stiff matrices.
 */

#include <stdlib.h>
#include "../WN/INCLUDE/compact.h"

/*
#define DEBUG
*/

#ifdef DEBUG

/*
#define DEBUGCALLS
*/

/*
#define DEBUGload_mat
*/

/*
#define DEBUGdenorm_mat
*/

/*
#define DEBUGsparse_vect_mat_prod
*/

/*
#define DEBUGsparse_square_mat
*/

/*
#define DEBUGdense_square_mat
*/

/*
#define DEBUGdense_vect_mat_prod
*/

/*
#define DEBUGinit
*/

/*
#define DEBUGsum_power
*/

#endif /* DEBUG */

#define EPS 0.000001

/*
#define CHECK_SUM
*/


#define MIN_NORMALIZED 0.99999999999999
#define MAX_NORMALIZED 1.00000000000001

#define test_normalized(val) ((val) < MIN_NORMALIZED || (val) > MAX_NORMALIZED)


#include <stdio.h>

/* predeclarations */
int switch_sparse2dense();
void dense_square_mat();

FILE *emcfp, * dbfp, * sspfp;

#ifndef TRUE
#    define TRUE 1
#    define FALSE 0
#endif

double totime;
double fromtime;

double eps;

double nu = 0;
double nut = 0;
unsigned long int_nut, k1, k2;
unsigned long index_i = 0;
double coef, icoef;

unsigned long top_tan = 0;

double *dval;

struct mtrx_itm {
    double rt;
    unsigned fm, tm;
    struct mtrx_itm *next;
    void *dummy;  /* for alignment on double boundaries */
};

struct mtrx_itm *mat_mem_1, *mat_mem_2;
unsigned first_mem, last_mem;

unsigned bytes_mem = 0x80000;


struct mtrx_col {
    double diag;
    unsigned ninn_c;
    struct mtrx_itm *first_c;
    struct mtrx_itm *last_r;
};

unsigned long limit_tan;

struct mtrx_col *mat_1, *mat_2;
double *mprob, * nextprob;
double *sumprob, * sumiprob;


struct mtrx_col *m_c_p;

unsigned long frm, nel, rr;
double sparse_factor;
unsigned long no_items;

void load_mat() {
    register struct mtrx_itm *r_p = NULL;
    register unsigned i;
    double ddd;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of load_mat\n");
#endif
    ++m_c_p;
    load_compact(&rr, emcfp);
    if ((ddd = dval[rr]) > 0.0) {
        if ((ddd = m_c_p->diag = 1.0 / ddd) > nu)
            nu = ddd;
    }
    else
        m_c_p->diag = ddd = 0.0;
#ifdef DEBUGload_mat
    fprintf(stderr, "    to mark #%d, diag #%d (%lg), %d entries\n",
            top_tan, rr, -(m_c_p->diag), nel);
#endif
    no_items += nel;
    m_c_p->ninn_c = nel;
    if (nel > 0)
        m_c_p->first_c = r_p = mat_mem_1 + first_mem;
    else
        m_c_p->first_c = NULL;
    if ((first_mem += nel) > last_mem) {
        fprintf(stderr,
                "load_mat: too many matrix items %ld: raise memory size!\n",
                no_items);
        exit(40);
    }
    for (i = nel ; i-- ; ++r_p) {
        register struct mtrx_col *m_r_p;
        load_compact(&frm, emcfp);
        load_compact(&rr, emcfp);
#ifdef DEBUGload_mat
        fprintf(stderr, "      from mark #%d, rate #%d\n", frm, rr);
#endif
        r_p->tm = top_tan; r_p->fm = frm;  r_p->rt = dval[rr];
        m_r_p = mat_1 + frm;
        if (m_r_p->last_r == NULL) {
            r_p->next = r_p;
            m_r_p->last_r = r_p;
#ifdef DEBUGload_mat
            fprintf(stderr, "          first item (%d) in row %d\n", top_tan, frm);
#endif
        }
        else {
#ifdef DEBUGload_mat
            fprintf(stderr, "          adding item %d in row %d containing", top_tan, frm);
            {
                struct mtrx_itm *pp = m_r_p->last_r;
                do {
                    pp = pp->next;
                    fprintf(stderr, " %d;", pp->tm);
                }
                while (pp != m_r_p->last_r);
            }
            fprintf(stderr, "\n");
#endif
            r_p->next = m_r_p->last_r->next;
            m_r_p->last_r->next = r_p;
            m_r_p->last_r = r_p;
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of load_mat\n");
#endif
}


void denorm_mat() {
    register struct mtrx_itm *r_p;
    register unsigned i;
    double coeff = 1.0 / nu;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of denorm_mat,  coeff=%lg\n", coeff);
#endif
    for (m_c_p = (mat_1 + 1), nel = top_tan ; nel-- ; ++m_c_p) {
        m_c_p->diag *= coeff;
    }
    for (m_c_p = (mat_1 + 1), nel = top_tan ; nel-- ; ++m_c_p) {
#ifdef DEBUGdenorm_mat
        fprintf(stderr, "    column %d\n", top_tan - nel);
#endif
        for (i = m_c_p->ninn_c, r_p = m_c_p->first_c ; i-- ; ++r_p) {
            frm = r_p->fm;
            r_p->rt *= mat_1[frm].diag;
#ifdef DEBUGdenorm_mat
            fprintf(stderr, "         fm=%d, rt=%lg\n", frm, r_p->rt);
#endif
        }
    }
    /* compute U = I + A/nu */
    for (m_c_p = (mat_1 + 1), nel = top_tan ; nel-- ; ++m_c_p) {
        m_c_p->diag = 1.0 - m_c_p->diag;
#ifdef DEBUGdenorm_mat
        fprintf(stderr, "         diag[%d] =%lg\n", top_tan - nel, m_c_p->diag);
#endif
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of denorm_mat\n");
#endif
}


void sparse_vect_mat_prod(struct mtrx_col *c_p, double *v1_p, double *v2_p) {
    unsigned ii = top_tan;

#ifdef DEBUGCALLS
    fprintf(stderr, "        Start of sparse_vect_mat_prod()\n");
#endif
    while (ii--) {
        register double r = (*(++v1_p)) * ((++c_p)->diag);
        register unsigned jj = c_p->ninn_c;
        register struct mtrx_itm *r_p = c_p->first_c;
#ifdef DEBUGsparse_vect_mat_prod
        fprintf(stderr, "            ... column %d with %d items\n", top_tan - ii, jj);
#endif
        while (jj-- > 0) {
            register unsigned ff = r_p->fm;
            r += (r_p++)->rt * mprob[ff];
        }
#ifdef DEBUGsparse_vect_mat_prod
        fprintf(stderr, "          ... sparse_vect_mat_prod: item %d = %lg\n", top_tan - ii, r);
#endif
        *(++v2_p) = r;
    }
    v1_p = mprob; mprob = nextprob; nextprob = v1_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "        End of sparse_vect_mat_prod\n");
#endif
}


int sparse_square_mat() {
    struct mtrx_col *row_p, *col_p, *res_p;
    unsigned nr, nc;
    register double sum;
    register struct mtrx_itm *rp, * cp, *sp;
    register unsigned ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of sparse_square_mat\n");
#endif
    sp = mat_mem_2; first_mem = 0;
    for (res_p = mat_2, nc = 0 ; nc++ < top_tan ;) {
        (++res_p)->last_r = NULL;
    }
    for (col_p = mat_1 + 1, res_p = mat_2, nc = 0 ; nc++ < top_tan ;
            col_p++) {
        unsigned nin = 0;
        (++res_p)->first_c = sp;
        for (row_p = mat_1 + 1, nr = 0 ; nr++ < top_tan ; row_p++) {
            unsigned nexc_n = 0;
            unsigned nexr_n = 0;
#ifdef DEBUGsparse_square_mat
            fprintf(stderr, "      ... sparse_square_mat: nc=%d, nr=%d\n", nc, nr);
#endif
            sum = 0.0;
            if ((rp = row_p->last_r) != NULL) {
#ifdef DEBUGsparse_square_mat
                fprintf(stderr, "      ... row element list is:\n          ");
                do {
                    rp = rp->next;
                    fprintf(stderr, " %d;", rp->tm);
                }
                while (rp != row_p->last_r);
                fprintf(stderr, "\n");
#endif
                rp = rp->next;
            }
            for (ii = col_p->ninn_c + 1, cp = col_p->first_c ; ii-- > 0 ;) {
                double nexc_v;
                double nexr_v = 0;
#ifdef DEBUGsparse_square_mat
                fprintf(stderr,
                        "         ... sparse_square_mat: ii=%d, nexc_n=%d, nexr_n=%d\n",
                        ii, nexc_n, nexr_n);
#endif
                if (nexc_n < nc && (ii == 0 || cp->fm > nc)) {
                    nexc_n = nc; nexc_v = col_p->diag;
                }
                else {
                    nexc_n = cp->fm; nexc_v = cp->rt; cp++;
                }
                while (nexr_n < nexc_n) {
#ifdef DEBUGsparse_square_mat
                    fprintf(stderr,
                            "            ... sparse_square_mat: nexc_n=%d, nexr_n=%d\n",
                            nexc_n, nexr_n);
#endif
                    if (nexr_n < nr && (rp == NULL || rp->tm > nr)) {
                        nexr_n = nr; nexr_v = row_p->diag;
                    }
                    else {
                        if (rp != NULL && (rp->tm > nexr_n)) {
                            nexr_n = rp->tm; nexr_v = rp->rt;
                            if (rp != row_p->last_r)
                                rp = rp->next;
                        }
                        else {
                            nexr_n = top_tan + 1;
                        }
                    }
                }
                if (nexc_n == nexr_n) {
                    sum += nexc_v * nexr_v;
                }
            }
#ifdef DEBUGsparse_square_mat
            fprintf(stderr, "        sparse_square_mat: nc=%d, nr=%d, sum=%lg\n",
                    nc, nr, sum);
#endif
            if (nc == nr) {
                res_p->diag = sum;
            }
            else if (sum > 0.0) {
                if ((++first_mem) > last_mem) {
                    fprintf(stderr,
                            "sparse_square_mat: too many matrix items %d: switching to dense\n",
                            first_mem + 1);
                    goto error;
                }
                nin++;
                sp->rt = sum;
                sp->fm = nr;
                sp->tm = nc;
                if (mat_2[nr].last_r == NULL) {
                    mat_2[nr].last_r = sp->next = sp;
                }
                else {
                    sp->next = mat_2[nr].last_r->next;
                    mat_2[nr].last_r->next = sp;
                    mat_2[nr].last_r = sp;
                }
                sp++;
            }
        }
        if ((res_p->ninn_c = nin) == 0)
            res_p->first_c = NULL;
    }
    res_p = mat_2; mat_2 = mat_1; mat_1 = res_p;
    sp = mat_mem_2; mat_mem_2 = mat_mem_1; mat_mem_1 = sp;
    no_items = top_tan + first_mem;
    sparse_factor = (double)(top_tan * top_tan) / no_items;
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of sparse_square_mat, sparse_factor=%lg\n",
            sparse_factor);
#endif
    return (FALSE);
error:
    if (switch_sparse2dense())
        return (TRUE);
    dense_square_mat();
    return (FALSE);
}


#include <sys/time.h>
struct timeval tv;
struct timezone tz;
unsigned next_print, print_interv, secs, usecs;

static double *MAT1, * MAT2;

void dense_square_mat() {
    double *row_p, *col_p, *res_p;
    unsigned nr, nc;
    register double sum;
    register double *rp, * cp;
    register unsigned ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of dense_square_mat\n");
#endif
    no_items = 0;
    for (col_p = MAT1, res_p = MAT2, nc = 0 ; nc++ < top_tan ;) {
        for (row_p = MAT1, nr = 0 ; nr++ < top_tan ;) {
            sum = 0.0;
            for (ii = 0, rp = row_p, cp = col_p ; ii++ < top_tan ;) {
                if (*rp > 0.0 && *cp > 0)
                    sum += *rp **cp;
                rp += top_tan; cp++;
            }
#ifdef DEBUGdense_square_mat
            fprintf(stderr, "        dense_square_mat: nc=%d, nr=%d, sum=%lg\n",
                    nc, nr, sum);
#endif
            if ((*(res_p++) = sum) > 0.0)
                no_items++;
            row_p++;
        }
        col_p += top_tan;
    }
    res_p = MAT2; MAT2 = MAT1; MAT1 = res_p;
    sparse_factor = (double)(limit_tan * limit_tan) / no_items;
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of dense_square_mat\n");
#endif
}


void dense_vect_mat_prod() {
    double *res_p;
    unsigned nc;
    register double sum;
    register double *rp, * cp;
    register unsigned ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of dense_vect_mat_prod\n");
#endif
    for (cp = MAT1, res_p = nextprob + 1, nc = 0 ;
            nc++ < top_tan ;) {
        sum = 0.0;
        for (ii = 0, rp = mprob + 1 ; ii++ < top_tan ;) {
            if (*rp > 0.0 && *cp > 0)
                sum += *rp **cp;
            rp++; cp++;
        }
#ifdef DEBUGdense_vect_mat_prod
        fprintf(stderr, "        dense_vect_mat_prod: nc=%d, sum=%lg\n", nc, sum);
#endif
        *(res_p++) = sum;
    }
    res_p = mprob; mprob = nextprob; nextprob = res_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of dense_vect_mat_prod\n");
#endif
}


unsigned using_sparse = TRUE;

int switch_sparse2dense() {
    struct mtrx_col *c_p = mat_1;
    register double *mat_p;
    unsigned ii = top_tan * top_tan;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of switch_sparse2dense\n");
#endif
    if ((ii * sizeof(double)) > bytes_mem) {
#ifdef DEBUG
        fprintf(stderr,
                "      End of switch_sparse2dense: not enough memory to switch (%d)\n",
                ii * sizeof(double));
#endif
        return (TRUE);
    }
    free(mat_2); free(mat_mem_2);
    mat_p = MAT1 = (double *)calloc(ii, sizeof(double));
    /* INIT MAT1 to U */
    ii = top_tan;
    while (ii--) {
        unsigned jj = (++c_p)->ninn_c;
        register struct mtrx_itm *r_p = c_p->first_c;
        register unsigned kk = top_tan;
        while (kk--) {
            if (kk == ii)
                *(mat_p++) = c_p->diag;
            else {
                if (jj > 0) {
                    register unsigned ff = top_tan - r_p->fm;
                    if (ff == kk) {
                        *(mat_p++) = (r_p++)->rt;
                        jj--;
                    }
                    else
                        *(mat_p++) = 0.0;
                }
                else {
                    *(mat_p++) = 0.0;
                }
            }
        }
    }
    free(mat_1);
    free(mat_mem_1);
    MAT2 = (double *)calloc(top_tan * top_tan, sizeof(double));
    using_sparse = FALSE;
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of switch_sparse2dense NORMAL\n");
#endif
    return (FALSE);
}


void read_in_mat() {
    double *d_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of read_in_mat\n");
#endif
    load_compact(&frm, dbfp);
    if ((dval = (double *)calloc(frm + 1,
                                 sizeof(double))) == NULL) {
        fprintf(stderr, "calloc: couldn't get enough memory\n");
        exit(35);
    }
#ifdef DEBUGread_in_mat
    fprintf(stderr, "    %d rate values:\n", (nel = frm));
#endif
    for (d_p = dval ; frm-- ;)
#ifdef DEBUGread_in_mat
    {
#endif
        load_double(++d_p, dbfp);
#ifdef DEBUGread_in_mat
        fprintf(stderr, "    %d -> %lg\n", (nel - frm), *d_p);
    }
#endif

    for (top_tan = 0, m_c_p = mat_1 ; top_tan++ < limit_tan ;) {
        (++m_c_p)->last_r = NULL;
    }
    first_mem = 0;
    for (top_tan = 0, m_c_p = mat_1, load_compact(&nel, emcfp) ;
            top_tan++ < limit_tan ;
            load_compact(&nel, emcfp)) {
        load_mat();
    }
    top_tan = limit_tan;
    denorm_mat();
    free(dval);
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of read_in_mat\n");
#endif
}


unsigned square_computed = FALSE;

void init() {
    double *d_p;
    double dd;
    double eps_coef;
    double bound_coef;
    double coef2 = 1.0;
    unsigned long exp_cf;
    unsigned left_to_k1, worth_log;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif
    last_mem = bytes_mem / sizeof(struct mtrx_itm);
#ifdef DEBUGCALLS
    fprintf(stderr, "    last_mem=%d\n", last_mem);
#endif
    load_compact(&limit_tan, emcfp);
    if ((mprob = (double *)calloc(limit_tan + 1,
                                  sizeof(double))) == NULL ||
            (nextprob = (double *)calloc(limit_tan + 1,
                                         sizeof(double))) == NULL ||
            (sumprob = (double *)calloc(limit_tan + 1,
                                        sizeof(double))) == NULL ||
            (sumiprob = (double *)calloc(limit_tan + 1,
                                         sizeof(double))) == NULL ||
            (mat_1 = (struct mtrx_col *)calloc(limit_tan + 1,
                     sizeof(struct mtrx_col))) == NULL ||
            (mat_mem_1 = (struct mtrx_itm *)calloc(last_mem,
                         sizeof(struct mtrx_itm))) == NULL) {
        fprintf(stderr, "calloc: couldn't get enough memory\n");
        exit(35);
    }
    no_items = limit_tan;
    read_in_mat();
    sparse_factor = (double)(limit_tan * limit_tan) / no_items;
    fprintf(stderr, "    init: sparse_factor=%lg\n", sparse_factor);

    load_double(&dd, sspfp); load_double(&dd, sspfp);
    if (dd < 0.0 || (totime >= 0.0 && dd > totime)) {
        fromtime = 0.0;
        if (totime < 0.0)
            totime = -totime;
        for (d_p = mprob, /* i_d_p = improb, */ nel = top_tan ;
                nel-- ;) {
            *(++d_p) = 0.0; /*  *(++i_d_p) = 0.0; */
        }
        mprob[1] = 1.0;
    }
    else {
        fromtime = dd;
        totime = (totime < 0.0) ? (-totime) : (totime - fromtime);
        for (d_p = mprob, /* i_d_p = improb, */ nel = top_tan ;
                nel-- ;) {
            double ddd;
            load_double(++d_p, sspfp);
            load_double(&ddd /* ++i_d_p */, sspfp);
        }
    }
    if (totime == 0.0) {
        fprintf(stderr, "\n RESULT ALREADY AVAILABLE !\n");
        exit(0);
    }
    nut = totime * nu;
#ifdef DEBUGinit
    fprintf(stderr, "    ... init computing k1,  nut=%lg\n", nut);
#endif
    coef = 1.0;
    if ((int_nut = (unsigned long)(nut)) > 0) {
        eps_coef = eps * 0.5;
        bound_coef = eps_coef * (nut - (double)(int_nut) + 1.0);
#ifdef DEBUGinit
        fprintf(stderr, "    ... init: int_nut=%d, bound_coef=%lg\n", int_nut, bound_coef);
#endif
        for (k1 = int_nut - 1 ; k1 > 0 ; k1--) {
            coef2 = coef * (double)(k1);
            coef = coef2 / nut;
            if (coef2 <= bound_coef)
                break;
            bound_coef += eps_coef;
        }
    }
    eps_coef = eps * 0.5;
    k2 = int_nut + 1;
    bound_coef = eps_coef * ((double)(k2) - nut);
    for (coef2 = 1.0 ; TRUE ; k2++) {
        coef2 *= nut;
        if (coef2 <= bound_coef)
            break;
        coef2 /= (double)(k2);
        bound_coef += eps_coef;
    }
    worth_log = ((top_tan << 1) + 16) * (unsigned)(sparse_factor);
    fprintf(stderr,
            "  ... skipping first %ld terms, then adding %ld terms (worth_log=%d)\n",
            k1, k2 - k1 + 1, worth_log);
    exp_cf = 0;
    if ((left_to_k1 = k1) > worth_log) {
        exp_cf = 1;
        if ((mat_2 = (struct mtrx_col *)calloc(limit_tan + 1,
                                               sizeof(struct mtrx_col))) == NULL ||
                (mat_mem_2 = (struct mtrx_itm *)calloc(last_mem,
                             sizeof(struct mtrx_itm))) == NULL) {
            fprintf(stderr, "calloc: couldn't get enough memory\n");
            goto linear;
        }
        if (left_to_k1 & 1) {
            left_to_k1 -= 1;
            sparse_vect_mat_prod(mat_1, mprob, nextprob);
        }
        do {
            exp_cf = (exp_cf << 1);
#ifdef DEBUGinit
            fprintf(stderr, "            init: exp_cf=%ld\n", exp_cf);
#else
            fprintf(stderr, "            init: computing U**%ld\r", exp_cf);
#endif
            if (using_sparse && sparse_factor < 4.0) {
                if (switch_sparse2dense())
                    goto linear;
            }
            if (using_sparse) {
                if (sparse_square_mat())
                    goto linear;
            }
            else
                dense_square_mat();
#ifdef DEBUGinit
            fprintf(stderr, "            init: new sparse_factor=%lg\n", sparse_factor);
#else
            fprintf(stderr, "            init: computed  U**%ld sparsity=%lg\r",
                    exp_cf, sparse_factor);
#endif
            worth_log = ((top_tan >> 1) + 1) * (unsigned)(sparse_factor);
            square_computed = TRUE;
            left_to_k1 = (left_to_k1 >> 1);
            if (left_to_k1 & 1) {
                left_to_k1 -= 1;
                if (using_sparse)
                    sparse_vect_mat_prod(mat_1, mprob, nextprob);
                else
                    dense_vect_mat_prod();
            }
        }
        while (left_to_k1 > worth_log);
    }

linear:
#ifndef DEBUGinit
    fprintf(stderr, "\n\n");
#endif
    if (exp_cf > 0) {
        if (using_sparse) {
            free(mat_2); free(mat_mem_2);
        }
        else {
            free(MAT2);
        }
    }
    else
        exp_cf = 1;
    if (left_to_k1 > 0) {

        fprintf(stderr,
                "  ... doing %d vect by U**%ld products\n",
                left_to_k1, exp_cf);
        index_i = 1;
        gettimeofday(&tv, &tz);
        secs = tv.tv_sec; usecs = tv.tv_usec;
        if (using_sparse)
            sparse_vect_mat_prod(mat_1, mprob, nextprob);
        else
            dense_vect_mat_prod();
        gettimeofday(&tv, &tz);
        if (tv.tv_sec > secs)
            print_interv = 1;
        else {
            secs = tv.tv_usec - usecs;
            print_interv = (unsigned)(1500000.0 / ((float)(secs)));
            if (print_interv < 1)
                print_interv = 1;
        }
        next_print = print_interv;

        for (; index_i < left_to_k1 ; index_i++) {
            if (using_sparse)
                sparse_vect_mat_prod(mat_1, mprob, nextprob);
            else
                dense_vect_mat_prod();
            if (index_i >= next_print) {
                next_print = index_i + print_interv;
                fprintf(stderr, " term %8ld\r", index_i); fflush(stderr);
            }
        }
        fprintf(stderr, " term %8ld\n", index_i);
    }
    if ((index_i = k1) > 0) {
        unsigned ii;
        for (dd = 0.0, d_p = mprob, ii = top_tan ; ii-- ;)
            dd += *(++d_p);
        if (test_normalized(dd)) {
            dd = 1.0 / dd;
            fprintf(stderr, "... init: renormalization coefficient %lg\n", dd);
            for (d_p = mprob + 1, ii = top_tan ; ii-- ; d_p++)
                *d_p *= dd;
        }
        if (square_computed) {
            if (! using_sparse) {
                free(MAT2);
                if ((mat_1 = (struct mtrx_col *)calloc(top_tan + 1,
                                                       sizeof(struct mtrx_col))) == NULL ||
                        (mat_mem_1 = (struct mtrx_itm *)calloc(last_mem,
                                     sizeof(struct mtrx_itm))) == NULL) {
                    fprintf(stderr, "calloc: couldn't get enough memory\n");
                    exit(35);
                }
            }
        }
    }

#ifdef DEBUGinit
    fprintf(stderr, "    ... init coef=%lg, index_i=%d\n", coef, index_i);
    fprintf(stderr, "    ... init: initial distribution\n");
#endif
    {
        register unsigned ii = top_tan; /* init sum */
        double *v_p = mprob + 1;
        for (d_p = sumprob + 1 ; ii-- ; d_p++, v_p++) {
#ifdef DEBUGinit
            fprintf(stderr, "              mprob[%d]=%lg\n", top_tan - ii, *v_p);
#endif
            (*d_p) = coef * (*v_p);
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}


void sum_power(struct mtrx_col *c_p, double *v1_p, double   *v2_p, double   *v3_p, double   *v4_p) {
    register unsigned ii = top_tan;
#ifdef CHECK_SUM
    double sum = 0.0;
#endif /* CHECK_SUM */

#ifdef DEBUGCALLS
    fprintf(stderr, "          Start of sum_power(%lg)\n", coef);
#endif
#ifdef DEBUGsum_power
    fprintf(stderr, "          ... sum_power: current distribution (index_i=%d)\n", index_i);
#endif
    while (ii--) {
        register double r = (*(++v1_p)) * ((++c_p)->diag);
        register unsigned jj = c_p->ninn_c;
        register struct mtrx_itm *r_p = c_p->first_c;
#ifdef DEBUGsum_power
        fprintf(stderr, "              sum_power: item #%d diag %lg\n", top_tan - ii, r);
#endif
        while (jj--) {
            register unsigned ff = r_p->fm;
#ifdef DEBUGsum_power
            /* fprintf(stderr,"                sum_power: product with item #%d\n", ff); */
#endif
            r += (r_p++)->rt * mprob[ff];
        }
#ifdef DEBUGsum_power
        fprintf(stderr, "              nextprob[%d]=%lg\n", top_tan - ii, r);
#endif
        *(++v2_p) = r;
#ifdef CHECK_SUM
        sum += r;
#endif /* CHECK_SUM */
        r *= coef;
        r += *(++v3_p);
        *v3_p = r;
        r = *v2_p;
        r *= icoef;
        r += *(++v4_p);
        *v4_p = r;
    }
#ifdef CHECK_SUM
    if (test_normalized(sum)) {
        sum = 1.0 / sum;
        fprintf(stderr, "... sum_power renormalization coefficient %lg\n", sum);
        for (v1_p = nextprob, ii = top_tan ; ii-- ;)
            *(++v1_p) *= sum;
    }
#endif /* CHECK_SUM */
    v1_p = mprob; mprob = nextprob; nextprob = v1_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "          End of sum_power\n");
#endif
}


int main(int argc, char **argv) {
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[100];
    unsigned long ii;
    double *m_p, * i_m_p;
    double dd, dd2;
    double stop_sum, stop_sum2;
    unsigned long nindex_i;

#ifdef DEBUG
    fprintf(stderr, "Start of ntrexp\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    if (argc < 3) {
        fprintf(stderr, "ERROR: no time interval !\n");
        exit(1);
    }
    sprintf(filename, "%s.emc", argv[1]);
    if ((emcfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.doubles", argv[1]);
    if ((dbfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.epd", argv[1]);
    if ((sspfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "  files opened\n");
#endif
    sscanf(argv[2], "%lf", &totime);
#ifdef DEBUG
    fprintf(stderr, "    totime=%f\n", totime);
#endif
    eps = EPS;
    if (argc > 3) {
        char **a_p = &(argv[3]);
        char *s_p;
        unsigned ii = argc - 2;
        while (--ii) {
            s_p = *(a_p++);
            if (*(s_p++) != '-' ||
                    (*s_p != 'e' && *s_p != 'M')) {
                fprintf(stderr, "ERROR: unknown parameter '%s'\n", s_p - 1);
                exit(33);
            }
            switch (*s_p) {
            case 'e' : {
                float ff;
                sscanf(s_p + 1, "%f", &ff);
                if ((eps = ff) > 0.01 || eps <= 0.0)
                    eps = 0.01;
            }
            break;
            case 'M' : {
                unsigned uu;
                sscanf(s_p + 1, "%d", &uu);
                if (uu < 1)
                    uu = 1;
                if (uu > 64)
                    uu = 64;
                bytes_mem = (uu << 19);
            }
            break;
            }
        }
    }
    init();
#ifdef DEBUG
    fprintf(stderr, "bytes_mem=%d, eps=%lg\n", bytes_mem, eps);
#endif
    (void) fclose(emcfp);
    (void) fclose(dbfp);
    (void) fclose(sspfp);
    if (square_computed) {
        sprintf(filename, "%s.emc", argv[1]);
        if ((emcfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
        load_compact(&limit_tan, emcfp);
        sprintf(filename, "%s.doubles", argv[1]);
        if ((dbfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
        read_in_mat();
        (void) fclose(emcfp);
        (void) fclose(dbfp);
    }
    gettimeofday(&tv, &tz);
    secs = tv.tv_sec; usecs = tv.tv_usec;
    next_print = 1; print_interv = 0;
    nindex_i = index_i + 1;
    icoef = coef * nut / (double)(nindex_i);
    while (index_i <= k2 && coef > 0.0) {
        index_i = nindex_i; nindex_i++;
        coef = icoef;
        icoef *= nut / (double)(nindex_i);
        sum_power(mat_1, mprob, nextprob, sumprob, sumiprob);
#ifdef DEBUG
        fprintf(stderr, " term %8d coef %lg\n", index_i, coef);
#else
        if (print_interv) {
            if (index_i >= next_print) {
                fprintf(stderr, " term %8ld coef %lg\r", index_i, coef);
                fflush(stderr);
                next_print = index_i + print_interv;
            }
        }
        else {
            gettimeofday(&tv, &tz);
            if (tv.tv_sec > secs)
                print_interv = 1;
            else {
                secs = tv.tv_usec - usecs;
                print_interv = (unsigned)(1000000.0 / ((float)(secs)));
                if (print_interv < 1)
                    print_interv = 1;
            }
            fprintf(stderr, " term %8ld coef %lg\r", index_i, coef);
            fflush(stderr);
            next_print = index_i + print_interv;
        }
#endif
    }
    fprintf(stderr, " term %8ld coef %lg\n\n", index_i, coef);
    for (dd = dd2 = 0.0, m_p = sumprob, i_m_p = sumiprob, ii = top_tan ;
            ii-- ;) {
        dd += *(++m_p);
        dd2 += *(++i_m_p);
    }
    stop_sum = 1.0 / dd; stop_sum2 = 1.0 / dd2;
    sprintf(filename, "%s.epd", argv[1]);
    if ((sspfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    dd = eps; store_double(&dd, sspfp);
    dd = totime; store_double(&dd, sspfp);
    for (m_p = sumprob, i_m_p = sumiprob, ii = top_tan ; ii-- ;) {
        ++m_p; *m_p *= stop_sum;
        ++i_m_p; *i_m_p *= stop_sum2;
        store_double(m_p, sspfp);
        store_double(i_m_p, sspfp);
    }
    (void) fclose(sspfp);

#ifdef DEBUG
    fprintf(stderr, "End of ntrexp\n");
#endif

    return 0;
}

