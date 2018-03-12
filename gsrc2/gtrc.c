/*
 *  Paris, February 1, 1990
 *  program: gtrc.c
 *  purpose: Transient solution of a GSPN MC with matrix exponential algorithm.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) NOT YET COMPLETELY DEBUGGED
 */

/*
#define DEBUG
*/


#define EPS 0.0000001
#define MAXITER 10000

#include <stdio.h>
#include <stdlib.h>
#include "../WN/INCLUDE/compact.h"

FILE *emcfp, * dbfp, * sspfp;



double totime;
double fromtime;

double delta;
double eps;
unsigned long nosteps;

unsigned long maxiter;

unsigned long frm, tmp;
/****************** ALEX Linux ***************/
/*unsigned nel;*/
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
double *mprob, * improb;
double *vect1, * vect2;


struct mtrx_col *m_c_p;

unsigned long frm, nel, rr;

void
load_mat() {
    register struct mtrx_itm *r_p;
    register unsigned i;
    double ddd;

#ifdef DEBUG
    fprintf(stderr, "  Start of load_mat\n");
#endif
    ++m_c_p;
    load_compact(&rr, emcfp);
    if ((ddd = dval[rr]) > 0.0)
        ddd = m_c_p->diag = 1.0 / ddd;
    else
        m_c_p->diag = ddd = 0.0;
    if (ddd > delta)
        delta = ddd;
#ifdef DEBUG
    fprintf(stderr, "    to mark #%d, diag #%d (%f), %d entries\n",
            top_tan, rr, -(m_c_p->diag), nel);
#endif
    m_c_p->ninn = nel;
    m_c_p->first = r_p =
                       (struct mtrx_itm *)calloc(nel, sizeof(struct mtrx_itm));
    for (i = nel ; i-- ; ++r_p) {
        load_compact(&frm, emcfp);
        load_compact(&rr, emcfp);
#ifdef DEBUG
        fprintf(stderr, "      from mark #%d, rate #%d\n", frm, rr);
#endif
        r_p->fm = frm;  r_p->rt = dval[rr];
    }
#ifdef DEBUG
    fprintf(stderr, "  End of load_mat\n");
#endif
}

void
denorm_mat() {
    register struct mtrx_itm *r_p;
    register unsigned i;

#ifdef DEBUG
    fprintf(stderr, "  Start of denorm_mat\n");
#endif
    for (m_c_p = (column + 1), nel = top_tan ; nel-- ; ++m_c_p) {
#ifdef DEBUG
        fprintf(stderr, "    column %d\n", top_tan - nel);
#endif
        for (i = m_c_p->ninn, r_p = m_c_p->first ; i-- ; ++r_p) {
            frm = r_p->fm;
            r_p->rt *= column[frm].diag;
#ifdef DEBUG
            fprintf(stderr, "         fm=%d, rt=%f\n", frm, r_p->rt);
#endif
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End of denorm_mat\n");
#endif
}



void
init() {
    double *d_p, * i_d_p;
    double dd;

#ifdef DEBUG
    fprintf(stderr, "  Start of init\n");
#endif
    load_compact(&limit_tan, emcfp);
    if ((column = (struct mtrx_col *)calloc(limit_tan + 1,
                                            sizeof(struct mtrx_col))) == NULL ||
            (vect1 = (double *)calloc(limit_tan + 1,
                                      sizeof(double))) == NULL ||
            (vect2 = (double *)calloc(limit_tan + 1,
                                      sizeof(double))) == NULL ||
            (mprob = (double *)calloc(limit_tan + 1,
                                      sizeof(double))) == NULL ||
            (improb = (double *)calloc(limit_tan + 1,
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
#ifdef DEBUG
    fprintf(stderr, "    %d rate values:\n", (nel = frm));
#endif
    for (d_p = dval ; frm-- ;)
#ifdef DEBUG
    {
#endif
        load_double(++d_p, dbfp);
#ifdef DEBUG
        fprintf(stderr, "    %d -> %f\n", (nel - frm), *d_p);
    }
#endif
    delta = 0.0;
    for (top_tan = 0, m_c_p = column, load_compact(&nel, emcfp) ;
            top_tan++ < limit_tan ;
            load_compact(&nel, emcfp)) {
        load_mat();
    }
    top_tan = limit_tan;
    denorm_mat();
    free(dval);
    load_double(&dd, sspfp); load_double(&dd, sspfp);
    if (dd < 0.0 || (totime >= 0.0 && dd > totime)) {
        fromtime = 0.0;
        if (totime < 0.0)
            totime = -totime;
        for (d_p = mprob, i_d_p = improb, nel = top_tan ;
                nel-- ;) {
            *(++d_p) = 0.0; *(++i_d_p) = 0.0;
        }
        mprob[1] = 1.0;
    }
    else {
        fromtime = dd;
        totime = (totime < 0.0) ? (-totime) : (totime - fromtime);
        for (d_p = mprob, i_d_p = improb, nel = top_tan ;
                nel-- ;) {
            load_double(++d_p, sspfp);
            load_double(++i_d_p, sspfp);
        }
    }
    delta = 1.0 / delta;
    nosteps = 1 + (unsigned)(totime / delta);
    delta = totime / (double)(nosteps);
    totime += fromtime;
#ifdef DEBUG
    fprintf(stderr, "  End of init\n");
#endif
}

double err;
int save;

#define HILIM 1.0
#define LOLIM 0.01

double maxr;

// void
// save_prob()
//     {
// }

void
integrate() {
    register unsigned ii;
    struct mtrx_itm *r_p;
    double *mp_p, * imp_p, * v1_p, * v2_p;
    register unsigned long i;
    struct mtrx_col *c_p;
    double r, absr;
    double sum, isum;
    double fact, ifact;
    unsigned long noiter;
    unsigned long max_i = top_tan + 1;
    unsigned step = 0;
#define abs(x) (((x)>0.0)?(x):(-(x)))

#ifdef DEBUG
    fprintf(stderr, "  Start of integrate\n");
#endif
    maxr = 0.5 * (HILIM + LOLIM);
    do {  /*  COMPUTE STATE AFTER TIME "delta"  */
        if (maxr < LOLIM || maxr > HILIM) { /*  ADJUST STEP LENGTH  */
            fromtime += step * delta; step = 0;
            delta = (maxr < LOLIM) ? (delta + delta) : (0.5 * delta);
            nosteps = 1 + (unsigned)((totime - fromtime) / delta);
            delta = (totime - fromtime) / (double)(nosteps);
        }
        ++step; maxr = 0.0; ifact = delta;
#ifdef DEBUG
        fprintf(stderr, "   fromtime=%f,totime=%f,delta=%f,nosteps=%d,step=%d\n",
                fromtime, totime, delta, nosteps, step);
#endif
        for (i = max_i, mp_p = (mprob + 1), imp_p = (improb + 1), v1_p = (vect1 + 1) ;
                --i ; ++mp_p, ++imp_p, ++v1_p) {
            *(v1_p) = r = *(mp_p);
            *(imp_p) += r * ifact;
        }
        for (noiter = 1 ; ;) {    /*  MATRIX EXPONENTIAL  */
            ++noiter; err = 0.0;
            fact = ifact; ifact = delta / noiter;
            /*  ROW BY COLUMN PRODUCT  */
            for (i = max_i, c_p = (column + 1),
                    mp_p = (mprob + 1), imp_p = (improb + 1),
                    v1_p = (vect1 + 1), v2_p = (vect2 + 1) ;
                    --i ; ++mp_p, ++imp_p, ++v1_p, ++v2_p, ++c_p) {
                r = -(*v1_p) * (c_p->diag);
                for (ii = c_p->ninn, r_p = c_p->first ; ii-- ; ++r_p) {
                    r += r_p->rt * vect1[(unsigned)(r_p->fm) ];
                }
                r *= fact;  absr = abs(r);  err += absr;
                if (absr > maxr)
                    maxr = absr;
                *v2_p = r;  *mp_p += r;
                *imp_p += r * ifact;
            }
            fprintf(stderr, " step %6d (%lu) time %f iteration %6lu error %f\n",
                    step, nosteps, fromtime + step * delta, noiter, err);
            if (err <= eps || noiter >= maxiter)
                break;
            for (i = max_i, v1_p = vect1, v2_p = vect2 ;
                    --i ; *++v1_p = *++v2_p) ;
        }
    }
    while
    (step < nosteps);
    /*  PROBABILITY RENORMALIZATION  */
    sum = isum = 0.0;
    for (i = max_i, mp_p = mprob, imp_p = improb ;
            --i ;) {
        sum += *++mp_p; isum += *++imp_p;
    }
    sum = 1.0 / sum;  isum = 1.0 / isum;
    for (i = max_i, mp_p = mprob, imp_p = improb ;
            --i ;) {
        *++mp_p *= sum; *++imp_p *= isum;
    }
    fprintf(stderr, " step %6d (%lu) time %f iteration %6lu error %f\n\n",
            step, nosteps, totime, noiter, err);
}


int
main(argc, argv)
int argc;
char **argv;
{
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[100];
    unsigned long ii;
    double *m_p, * i_m_p;
    double dd;

#ifdef DEBUG
    fprintf(stderr, "Start of gtrc\n");
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
    eps = EPS; maxiter = MAXITER;
    if (argc > 3) {
        char **a_p = &(argv[3]);
        char *s_p;
        unsigned ii = argc - 2;
        while (--ii) {
            s_p = *(a_p++);
            if (*(s_p++) != '-' ||
                    (*s_p != 'e' && *s_p != 'i' && *s_p != 's' && *s_p != 'S')) {
                fprintf(stderr, "ERROR: unknown parameter '%s'\n", s_p - 1);
                exit(33);
            }
            switch (*s_p) {
            case 'e' : {
                float ff;
                sscanf(s_p + 1, "%f", &ff);
                eps = ff;
            }
            break;
            case 'i' : sscanf(s_p + 1, "%ld", &maxiter);
                break;
            case 's' : sscanf(s_p + 1, "%d", &save);
                break;
            }
        }
    }
    init();
#ifdef DEBUG
    fprintf(stderr, "eps=%f, maxiter=%d\n", eps, maxiter);
#endif
    (void) fclose(emcfp);
    (void) fclose(dbfp);
    (void) fclose(sspfp);
    integrate();
    sprintf(filename, "%s.epd", argv[1]);
    if ((sspfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    dd = -err; store_double(&dd, sspfp);
    dd = totime; store_double(&dd, sspfp);
    for (m_p = mprob, i_m_p = improb, ii = top_tan ; ii-- ;) {
        store_double(++m_p, sspfp);
        store_double(++i_m_p, sspfp);
    }
    (void) fclose(sspfp);

#ifdef DEBUG
    fprintf(stderr, "End of gtrc\n");
#endif
    return 0;
}

