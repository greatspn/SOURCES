/*
 *  Paris, March 31, 1994
 *  program: ntrs.c
 *  purpose: Transient solution of a GSPN MC with new summation algorithm
 *           for Randomization formula
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) Running much faster than previous algorithm: CPU time increases
 *       linearly as a function of the integration time.
 *    2) Same (very good) numerical stability properties as previous
 *       implementation in case of stiff matrices.
 */

#include <stdlib.h>
#include <string.h>
#include "../INCLUDE/compact.h"

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
#define DEBUGrowcol_prod
*/

#define DEBUGinit
/*
*/

/*
#define DEBUGsum_power
*/

#endif /* DEBUG */

#define EPS 0.000000000001

/*
#define CHECK_SUM
*/


#define MIN_NORMALIZED 0.999999999999
#define MAX_NORMALIZED 1.000000000001

#define test_normalized(val) ((val) < MIN_NORMALIZED || (val) > MAX_NORMALIZED)


#include <stdio.h>
#include "../INCLUDE/const.h"
#include "../INCLUDE/struct.h"
#include "../INCLUDE/var_ext_solve.h"
#include "../INCLUDE/macros.h"


FILE *emcfp, * dbfp, * sspfp;
FILE *stafp, * altth, * nmrk;
FILE *clmfp, * mptfp;
FILE *ratesfp, * denom, *outtype;

static int symbolic;
static double *rates = NULL;


#ifndef TRUE
#    define TRUE 1
#    define FALSE 0
#endif

double totime;
double fromtime;

double eps;

double nu = 0;
unsigned long int_nut, k1, k2;
unsigned long index_i = 0;
double coef, icoef;

unsigned long frm, tmp;
unsigned long nel;
unsigned long top_tan = 0;

double *dval;

struct mtrx_itm {
    unsigned long fm;
    double rt;
};

struct mtrx_col {
    double diag;
    unsigned short ninn;
    struct mtrx_itm *first;
};

unsigned long limit_tan;

struct mtrx_col *column;
double *mprob, * nextprob;
double *sumprob, * sumiprob;
double *mymp, * sta;


struct mtrx_col *m_c_p;

unsigned long rr;

static int ntr;
static char temp[MAXSTRING];

static int toptan = 0;
static int topvan = 0;

int exp_set = FALSE;


void load_mat() {
    register unsigned i;
    double ddd;
    unsigned long f_p;
    unsigned long from;
    unsigned long rate;
    unsigned long next;
    unsigned long nel;
    unsigned long rr;

    ++m_c_p;
    load_compact(&nel, clmfp);
    load_compact(&rr, clmfp);
    load_compact(&f_p, mptfp);
#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of load_mat\n");
#endif
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
    if ((int)(m_c_p->ninn = nel)) {
        m_c_p->first = (struct mtrx_itm *)calloc(nel, sizeof(struct mtrx_itm));
        fseek(emcfp, f_p, 0);
        for (i = 0 ; i < nel ; i++) {
            load_compact(&from, emcfp);
            load_compact(&rate, emcfp);
            load_compact(&next, emcfp);
            m_c_p->first[nel - i - 1].fm = from;
            m_c_p->first[nel - i - 1].rt = dval[rate];
            fseek(emcfp, next, 0);
        }
    }
    else
        m_c_p->first = NULL;
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
    for (m_c_p = (column + 1), nel = top_tan ; nel-- ; ++m_c_p) {
        m_c_p->diag *= coeff;
    }
    for (m_c_p = (column + 1), nel = top_tan ; nel-- ; ++m_c_p) {
#ifdef DEBUGdenorm_mat
        fprintf(stderr, "    column %d\n", top_tan - nel);
#endif
        for (i = m_c_p->ninn, r_p = m_c_p->first ; i-- ; ++r_p) {
            frm = r_p->fm;
            r_p->rt *= column[frm].diag;
#ifdef DEBUGdenorm_mat
            fprintf(stderr, "         fm=%d, rt=%lg\n", frm, r_p->rt);
#endif
        }
    }
    /* compute U = I + A/nu */
    for (m_c_p = (column + 1), nel = top_tan ; nel-- ; ++m_c_p) {
        m_c_p->diag = 1.0 - m_c_p->diag;
#ifdef DEBUGdenorm_mat
        fprintf(stderr, "         diag[%d] =%lg\n", top_tan - nel, m_c_p->diag);
#endif
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of denorm_mat\n");
#endif
}


void rowcol_prod(struct mtrx_col *c_p,
                 double *v1_p,
                 double *v2_p) {
    unsigned ii = top_tan;

#ifdef DEBUGCALLS
    fprintf(stderr, "        Start of rowcol_prod()\n");
#endif
    while (ii--) {
        register double r = (*(++v1_p)) * ((++c_p)->diag);
        register unsigned jj = c_p->ninn;
        register struct mtrx_itm *r_p = c_p->first;
        while (jj--) {
            register unsigned ff = r_p->fm;
            r += (r_p++)->rt * mprob[ff];
        }
#ifdef DEBUGrowcol_prod
        fprintf(stderr, "          ... rowcol_prod: item %d = %lg\n", top_tan - ii, r);
#endif
        *(++v2_p) = r;
    }
    v1_p = mprob; mprob = nextprob; nextprob = v1_p;
#ifdef DEBUGCALLS
    fprintf(stderr, "        End of rowcol_prod\n");
#endif
}


static int from_GUI = -1;
int invoked_from_gui() { // Is invoked from the new Java-based GUI?
    const char *env;
    if (from_GUI == -1) { // Not yet determined
        env = getenv("FROM_GUI");
        from_GUI = (env != NULL && 0 == strcmp(env, "1"));
    }
    return (from_GUI != 0);
}

#include <sys/time.h>
struct timeval tv;
struct timezone tz;
unsigned next_print, print_interv, secs, usecs;

void init() {
    double *d_p;
    double dd;
    double eps_coef;
    double bound_coef;
    double coef2 = 1.0;
    int ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif
    fscanf(outtype, "%d", &symbolic);
    fgets(temp, MAXSTRING - 1, altth);
    ntr = atoi(temp);
    load_compact(&limit_tan, clmfp);
    if (exp_set) {
        /* Opzione per set di esperimenti */
        rates = (double *)calloc(ntr, sizeof(double));
        for (ii = 0; ii < ntr; ii++)
            fscanf(ratesfp, "%lg\n", &rates[ii]);
    }/* Opzione per set di esperimenti */

    if ((column = (struct mtrx_col *)calloc(limit_tan + 1,
                                            sizeof(struct mtrx_col))) == NULL ||
            (mprob = (double *)calloc(limit_tan + 1,
                                      sizeof(double))) == NULL ||
            (mymp = (double *)calloc(limit_tan + 1,
                                     sizeof(double))) == NULL ||
            (sta = (double *)calloc(ntr,
                                    sizeof(double))) == NULL ||
            (nextprob = (double *)calloc(limit_tan + 1,
                                         sizeof(double))) == NULL ||
            (sumprob = (double *)calloc(limit_tan + 1,
                                        sizeof(double))) == NULL ||
            (sumiprob = (double *)calloc(limit_tan + 1,
                                         sizeof(double))) == NULL) {
        fprintf(stderr, "calloc: couldn't get enough memory\n");
        exit(35);
    }
    load_compact(&frm, dbfp);
    if ((dval = (double *)calloc(frm + 1,
                                 sizeof(double))) == NULL) {
        fprintf(stderr, "calloc: couldn't get enough memory\n");
        exit(35);
    }
#ifdef DEBUGinit
    fprintf(stderr, "    %ld rate values:\n", (nel = frm));
#endif
    for (d_p = dval ; frm-- ;)
#ifdef DEBUGinit
    {
#endif
        load_double(++d_p, dbfp);
#ifdef DEBUGinit
        fprintf(stderr, "    %ld -> %lg\n", (nel - frm), *d_p);
    }
#endif

#ifdef VECCHIA_LOAD
    for (top_tan = 0, m_c_p = column, load_compact(&nel, emcfp) ;
            top_tan++ < limit_tan ;
            load_compact(&nel, emcfp)) {
        load_mat();
    }
#endif
    /** Inizio nuova LOAD */
    top_tan = 0;
    for (m_c_p = column ; top_tan++ < limit_tan ;) {
        load_mat();
    }
    /** Fine nuova LOAD */

    top_tan = limit_tan;
    denorm_mat();
    free(dval);
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
    nu *= totime;
#ifdef DEBUGinit
    fprintf(stderr, "    ... init computing k1,  nut=%lg\n", nu);
#endif
    coef = 1.0;
    if ((int_nut = (unsigned long)(nu)) > 0) {
        eps_coef = eps * 0.25;
        bound_coef = eps_coef * (nu - (double)(int_nut) + 1.0);
#ifdef DEBUGinit
        fprintf(stderr, "    ... init: int_nut=%ld, bound_coef=%lg\n", int_nut, bound_coef);
#endif
        for (k1 = int_nut - 1 ; k1 > 0 ; k1--) {
            coef2 = coef * (double)(k1);
            coef = coef2 / nu;
            if (coef2 <= bound_coef)
                break;
            bound_coef += eps_coef;
        }
    }
    eps_coef = eps * 0.5;
    k2 = int_nut + 1;
    bound_coef = eps_coef * ((double)(k2) - nu);
    for (coef2 = 1.0 ; TRUE ; k2++) {
        coef2 *= nu;
        if (coef2 <= bound_coef)
            break;
        coef2 /= (double)(k2);
        bound_coef += eps_coef;
    }
#ifdef DEBUGinit
    fprintf(stderr, "          init: k1=%ld, k2=%ld, coef=%lg\n", k1, k2, coef);
#endif
    if (k1 > 1) {
        unsigned ii;
        fprintf(stderr, "  ... skipping first %ld terms, then adding %ld terms\n",
                k1 - 1, k2 - k1 + 1);
        index_i = 1;
        gettimeofday(&tv, &tz);
        secs = tv.tv_sec; usecs = tv.tv_usec;
        rowcol_prod(column, mprob, nextprob);
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

        for (index_i++ ; index_i < k1 ; index_i++) {
            rowcol_prod(column, mprob, nextprob);
            if (index_i >= next_print) {
                next_print = index_i + print_interv;
                /* fprintf(stderr," term %8d\r", index_i); fflush(stderr); */
                fprintf(stderr, " term %8ld\n", index_i); fflush(stderr);
            }
        }
        fprintf(stderr, " term %8ld\n", index_i);
        for (dd = 0.0, d_p = mprob, ii = top_tan ; ii-- ;)
            dd += *(++d_p);
        if (test_normalized(dd)) {
            dd = 1.0 / dd;
            fprintf(stderr, "... rowcol_prod renormalization coefficient %lg\n", dd);
            for (d_p = mprob + 1, ii = top_tan ; ii-- ; d_p++)
                *d_p *= dd;
        }
    }
    else
        index_i = 0;
#ifdef DEBUGinit
    fprintf(stderr, "    ... init coef=%lg, index_i=%ld\n", coef, index_i);
    fprintf(stderr, "    ... init: initial distribution\n");
#endif
    {
        register unsigned ii = top_tan; /* init sum */
        double *v_p = mprob + 1;
        for (d_p = sumprob + 1 ; ii-- ; d_p++, v_p++) {
#ifdef DEBUGinit
            fprintf(stderr, "              mprob[%ld]=%lg\n", top_tan - ii, *v_p);
#endif
            (*d_p) = coef * (*v_p);
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}


void sum_power(struct mtrx_col *c_p,
               double *v1_p, double *v2_p, double *v3_p, double *v4_p) {
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
        register unsigned jj = c_p->ninn;
        register struct mtrx_itm *r_p = c_p->first;
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
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void compute_throughput() {
    /* Init of compute_throughput */
    int current_pos;
    int last_pos;
    unsigned long ntrans;
    unsigned long tr;
    unsigned long nfatt, ndenom, trf, en_degree, ordinary_m = 1, denom_p;
    unsigned long tr_d, ordinary_m_d = 1;
    double denomin;
    double prob;

    fscanf(nmrk, "toptan= %d\n", &toptan);
    fscanf(nmrk, "topvan= %d\n", &topvan);

#ifndef GREATSPN
    fprintf(stdout, "SRG has %d tangible markings\n\n", top_tan);
#endif

    for (tr = 0; tr < ntr; tr++)
        sta[tr] = 0.0;
    for (current_pos = 0; current_pos < top_tan; current_pos++) {
#ifndef GREATSPN
        fprintf(stdout, "Marking Number %d\r", current_pos);
#endif
        load_compact(&ntrans, altth);
        for (last_pos = 1; last_pos <= ntrans; last_pos++) {
            load_compact(&tr, altth);
            if (exp_set) {
                /* Opzione per set di esperimenti */

                load_compact(&nfatt, altth);
                prob = 1.0;
                for (; nfatt ; --nfatt) {
                    /* Per ogni fattore */
                    load_compact(&trf, altth);
                    if (rates[trf] > 0.0)
                        load_compact(&en_degree, altth);
                    else
                        en_degree = 1;
                    if (symbolic)
                        load_compact(&ordinary_m, altth);
                    load_compact(&denom_p, altth);
                    if (denom_p != 0) {
                        denomin = 0.0;
                        fseek(denom, denom_p, 0);
                        load_compact(&ndenom, denom);
                        for (; ndenom ; --ndenom) {
                            load_compact(&tr_d, denom);
                            /*load_compact(&en_degree_d,denom); */
                            if (symbolic)
                                load_compact(&ordinary_m_d, denom);
                            denomin += -rates[tr_d] * ordinary_m_d;
                        }
                    }
                    else
                        denomin = 1.0;
                    if (rates[trf] > 0.0)
                        prob *= ((rates[trf] * ordinary_m * en_degree) / denomin);
                    else
                        prob *= ((-rates[trf] * ordinary_m * en_degree) / denomin);
                }/* Per ogni fattore */
            }/* Opzione per set di esperimenti */
            if (!exp_set) {
                /* Opzione per unico run */
                load_double(&prob, altth);
            }/* Opzione per unico run */

            sta[tr] += mymp[current_pos] * prob;
        }
    }
    for (tr = 0; tr < ntr; tr++) {
        fscanf(altth, "%s", temp);
        fprintf(stafp, "Thru_%s = %12.12f\n", temp, sta[tr]);
        if (invoked_from_gui())
            fprintf(stderr, "#{GUI}# RESULT ALL THROUGHPUT %s = %12.16lf\n", temp, sta[tr]);
    }
    /*for(tr=0;tr<toptan;tr++)
     fprintf(stdout,"P(%d)=%lg\n",tr+1,mymp[tr]);*/
}/* End of compute_throughput */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int main(int argc,
         char **argv) {
    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[1024];
    unsigned long ii;
    double *m_p, * i_m_p;
    double dd, dd2;
    double stop_sum, stop_sum2;
    unsigned long nindex_i;
    int kj;

#ifdef DEBUG
    fprintf(stderr, "Start of ntrs\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    if (argc < 3) {
        fprintf(stderr, "ERROR: no time interval !\n");
        exit(1);
    }
    sprintf(filename, "%s.outtype", argv[1]);
    if ((outtype = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.clm", argv[1]);
    if ((clmfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.mpt", argv[1]);
    if ((mptfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
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
    sprintf(filename, "%s.sta", argv[1]);
    if ((stafp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.throu", argv[1]);
    if ((altth = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((nmrk = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.rates", argv[1]);
    if ((ratesfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.denom", argv[1]);
    if ((denom = fopen(filename, "r")) == NULL) {
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
                    (*s_p != 'e' && *s_p != 'i' && *s_p != 's' && *s_p != 'b')) {
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
            case 'b': exp_set = TRUE;
                break;
            default:
                break;
            }
        }
    }
    init();
#ifdef DEBUG
    fprintf(stderr, "eps=%lg\n", eps);
#endif
    (void) fclose(outtype);
    (void) fclose(clmfp);
    (void) fclose(mptfp);
    (void) fclose(emcfp);
    (void) fclose(dbfp);
    (void) fclose(sspfp);
    gettimeofday(&tv, &tz);
    secs = tv.tv_sec; usecs = tv.tv_usec;
    next_print = 1; print_interv = 0;
    nindex_i = index_i + 1;
    icoef = coef * nu / (double)(nindex_i);
    while (index_i <= k2 && coef > 0.0) {
        index_i = nindex_i; nindex_i++;
        coef = icoef;
        icoef *= nu / (double)(nindex_i);
        sum_power(column, mprob, nextprob, sumprob, sumiprob);
#ifdef DEBUG
        fprintf(stderr, " term %8ld coef %lg\n", index_i, coef);
#else
        if (print_interv) {
            if (index_i >= next_print) {
                /* fprintf(stderr," term %8d coef %lg\r", index_i, coef); */
                fprintf(stderr, " term %8ld coef %lg\n", index_i, coef);
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
            /* fprintf(stderr," term %8d coef %lg\r", index_i, coef); */
            fprintf(stderr, " term %8ld coef %lg\n", index_i, coef);
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
    if ((sspfp = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    dd = eps; store_double(&dd, sspfp);
    dd = totime; store_double(&dd, sspfp);
    kj = top_tan;
    for (m_p = sumprob, i_m_p = sumiprob, ii = top_tan ; ii-- ;) {
        ++m_p;
        *m_p *= stop_sum;
        ++i_m_p;
        *i_m_p *= stop_sum2;
        mymp[top_tan - kj] = *m_p;
        kj--;
        store_double(m_p, sspfp);
        store_double(i_m_p, sspfp);
    }
    (void) fclose(sspfp);
    compute_throughput();
    (void) fclose(altth);
    (void) fclose(stafp);
    (void) fclose(nmrk);
    (void) fclose(denom);
    (void) fclose(ratesfp);



#ifdef DEBUG
    fprintf(stderr, "End of ntrs\n");
#endif
    return 0;
}

