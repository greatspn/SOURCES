/*
 *  Paris, February 1, 1990
 *  program: ggsc.c
 *  purpose: Steady-state solution of a GSPN EMC with Gauss-Seidel algorithm.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) NOT YET COMPLETELY DEBUGGED
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../INCLUDE/const.h"
#include "../INCLUDE/struct.h"
#include "../INCLUDE/var_ext_solve.h"
#include "../INCLUDE/macros.h"
#include "compact.h"


#define EPS 0.00000000000000000001
#define MAXITER 1000

static int ntr;

static FILE *emcfp, * dbfp, * sspfp;
static FILE *altth, *clmfp, *mptfp;
static FILE *stafp = NULL;
static FILE *nmrk = NULL;
static double *rates = NULL;
FILE *denom, *ratesfp, *outtype;


static char temp[MAXSTRING];

static int toptan = 0;
static int topvan = 0;


static double eps;
static unsigned long maxiter;

static unsigned long frm;

static unsigned long top_tan = 0;

static double *dval;

struct mtrx_itm {
    unsigned long fm;
    unsigned long rt;
};

struct mtrx_col {
    unsigned long weight;
    unsigned long ninn;
    struct mtrx_itm *first;
};

static unsigned long limit_tan;

static struct mtrx_col *column;
static double *mprob, * wprob;
static double *mp, *sta, *mymp;
static int symbolic;

int exp_set = FALSE;

static struct mtrx_col *m_c_p;

static unsigned long frm, nel;
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //    //extern char *       calloc();
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(1);
    }
    return (callptr);
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

void print_mat(unsigned long to) {
    register struct mtrx_itm *r_p;
    register unsigned i;

    ++m_c_p;
    fprintf(stdout, "PESO %lg ELEMENTI %ld:\n", dval[m_c_p->weight], m_c_p->ninn);
    for (i = m_c_p->ninn , r_p = m_c_p->first; i-- ; ++r_p) {
        fprintf(stdout, "DA T%ld A T%ld RATE %lg\n", r_p->fm, to, dval[r_p->rt]);
    }

}

void load_mat() {
    register unsigned i;
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
    m_c_p->weight = rr;
    if ((int)(m_c_p->ninn = nel)) {
        m_c_p->first = (struct mtrx_itm *)ecalloc(nel, sizeof(struct mtrx_itm));
        fseek(emcfp, f_p, 0);
        for (i = 0 ; i < nel ; i++) {
            load_compact(&from, emcfp);
            load_compact(&rate, emcfp);
            load_compact(&next, emcfp);
            m_c_p->first[nel - i - 1].fm = from;
            m_c_p->first[nel - i - 1].rt = rate;
            fseek(emcfp, next, 0);
        }
    }
    else
        m_c_p->first = NULL;
}


void init() {
    double *d_p;
    double dd, sum;
    int transient, ii;

    fscanf(outtype, "%d", &symbolic);
    load_compact(&limit_tan, clmfp);
    fgets(temp, MAXSTRING - 1, altth);
    ntr = atoi(temp);
    if (exp_set) {
        rates = (double *)ecalloc(ntr, sizeof(double));
        for (ii = 0; ii < ntr; ii++)
            fscanf(ratesfp, "%lg\n", &rates[ii]);
    }

    if ((column = (struct mtrx_col *)ecalloc(limit_tan + 1,
                  sizeof(struct mtrx_col))) == NULL ||
            (mprob = (double *)ecalloc(limit_tan + 1,
                                       sizeof(double))) == NULL ||
            (mymp = (double *)ecalloc(limit_tan + 1,
                                      sizeof(double))) == NULL ||
            (mp = (double *)ecalloc(limit_tan + 1,
                                    sizeof(double))) == NULL ||
            (sta = (double *)ecalloc(ntr,
                                     sizeof(double))) == NULL ||
            (wprob = (double *)ecalloc(limit_tan + 1,
                                       sizeof(double))) == NULL) {
        fprintf(stderr, "ecalloc: couldn't get enough memory in OR\n");
        exit(35);
    }
    top_tan = 0;
    for (m_c_p = column ; top_tan++ < limit_tan ;) {
        load_mat();
    }
    top_tan = limit_tan;
    load_compact(&frm, dbfp);
    if ((dval = (double *)ecalloc(frm + 1,
                                  sizeof(double))) == NULL) {
        fprintf(stderr, "ecalloc: couldn't get enough memory dopo OR\n");
        exit(35);
    }
    for (d_p = dval ; frm-- ;)
        load_double(++d_p, dbfp);
    load_double(&dd, sspfp);
    load_double(&dd, sspfp);
    transient = (dd > 0.0);
    sum = 0.0;
    if (dd == 0.0) {
        dd = 1.0 / (double)top_tan;
        for (d_p = mprob, nel = top_tan ; nel-- ; * (++d_p) = dd);
    }
    else {
        for (d_p = mprob, nel = top_tan, m_c_p = column; nel-- ;) {
            load_double(++d_p, sspfp);
            if (transient)
                load_double(&dd, sspfp);
            frm = (++m_c_p)->weight;
            if ((dd = dval[frm]) > 0.0)
                * d_p /= dd;
            sum += *d_p;
        }
        dd = 1.0 / sum;
        for (d_p = mprob, nel = top_tan ; nel-- ;) {
            *++d_p *= dd;
        }
    }

    /*    top_tan = 0;

        for ( m_c_p = column ; top_tan++ < limit_tan ; ) {
    	print_mat(top_tan);
          }
        top_tan = limit_tan; */
}

static char netname[400];
static char   *can_t_open = "Can't open file %s for %c\n";
static char  filename[410];

double err;


extern FILE *fopen();

void save_distrib_human() {
    double rr, sum, dd;
    unsigned rn;
    double *res_p, * m_p;
    struct mtrx_col *c_p;
    int ii;

    // printf("save_distrib_human %s\n", filename);
    FILE *f_mpdh;
    if ((f_mpdh = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sum = 0.0;
    for (res_p = mprob, m_p = wprob, ii = top_tan, c_p = column; ii-- ;) {
        rn = (++c_p)->weight;
        rr = *++res_p;
        if ((dd = dval[rn]) > 0.0)
            rr *= dd;
        *++m_p = rr;
        sum += rr;
    }
    dd = -err; fprintf(f_mpdh, "%g ", dd);
    dd = -1.0; fprintf(f_mpdh, "%g \n", dd);
    dd = 1.0 / sum;
    for (ii = top_tan, m_p = wprob ; ii-- ;) {
        rr = *++m_p * dd;
        mymp[top_tan - ii - 1] = rr;
    }
    fclose(f_mpdh);
}

void save_distrib() {
    double rr, sum, dd;
    unsigned rn;
    double *res_p, * m_p;
    struct mtrx_col *c_p;
    int ii;
    char command[820];

    sprintf(filename, "%s.mpd", netname);
    FILE *f_mpd;
    if ((f_mpd = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sum = 0.0;
    for (res_p = mprob, m_p = wprob, ii = top_tan, c_p = column; ii-- ;) {
        rn = (++c_p)->weight;
        rr = *++res_p;
        if ((dd = dval[rn]) > 0.0)
            rr *= dd;
        *++m_p = rr;
        sum += rr;
    }
    dd = -err; store_double(&dd, f_mpd);
    dd = -1.0; store_double(&dd, f_mpd);
    // printf("save_distrib %lf %lf", -err, dd);
    dd = 1.0 / sum;
    for (ii = top_tan, m_p = wprob ; ii-- ;) {
        rr = *++m_p * dd;
        store_double(&rr, f_mpd);
        //fprintf(stdout,"STATO %d RIS %lg\n",ii+1,rr);
    }
    fclose(f_mpd);

    // sprintf(command, "/bin/cp \"%s.mpd\" \"%s.epd\"", netname, netname);
    // int e = system(command);
    // printf("command(%d): %s\n", e, command);
}


int save = 0;

void gauss_seidel() {
    register unsigned long ii;
    struct mtrx_itm *r_p;
    double *res_p;
    register unsigned long rn;
    register unsigned long i;
    struct mtrx_col *c_p;
    double rr;
    double r;
    double sum;
    unsigned long noiter = 0;
    unsigned long max_i = top_tan + 1;
    int next_save = save;

    do {
        /* Loop until accuracy is met */
        i = max_i; ++noiter;  err = sum = 0.0;
        for (res_p = mprob + 1, c_p = column + 1 ; --i ; ++res_p, ++c_p) {
            /* For each unknown */
            if ((ii = c_p->ninn)) {
                r_p = c_p->first;
                rn = r_p->rt;
                r = dval[rn];
                rn = r_p->fm;
                r *= mprob[rn];
                while (--ii) {
                    rn = (++r_p)->rt;
                    rr = dval[rn];
                    rn = r_p->fm;
                    rr *= mprob[rn];
                    r += rr;
                }
                if (r == 0.0)
                    r = 0.01 * *res_p;
            }
            else
                r = 0.01 * *res_p;
            rr = r - *res_p;
            err += ((rr >= 0.0) ? rr : -rr);
            *res_p = r;
            sum += r;
        }/* For each unknown */
        if ((rr = sum - 1.0) > 0.01 || rr < -0.01) {
            rr = 1.0 / sum;
            for (i = max_i, res_p = mprob ; --i ;) {
                *++res_p *= rr;
            }
        }
        if (noiter >= next_save) {
            next_save += save;
            save_distrib();
//fprintf(stderr," iteration %9d error %1.20f\n", noiter, err);
        }
    } /* Loop until accuracy is met */
    while (err > eps && noiter < maxiter);
    save_distrib();
    fprintf(stderr, " iteration %9ld error %1.20f\n\n", noiter, err);
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
    unsigned long nfatt, ndenom, trf, en_degree = 1, ordinary_m = 1, denom_p;
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
        fprintf(stafp, "Thru_%s = %12.16lf\n", temp, sta[tr]);
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

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(netname, "%s", argv[1]);
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
    sprintf(filename, "%s.emc", argv[1]);
    if ((emcfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.mpt", argv[1]);
    if ((mptfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.doubles", argv[1]);
    if ((dbfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.mpd", argv[1]);
    if ((sspfp = fopen(filename, "rb")) == NULL) {
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


    eps = EPS; maxiter = MAXITER; save = 1;
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii--) {
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
                eps = ff;
            }
            break;
            case 'i' : sscanf(s_p + 1, "%ld", &maxiter);
                break;
            case 's' : sscanf(s_p + 1, "%d", &save);
                break;
            case 'b' : exp_set = TRUE;
                break;
            }
        }
    }
    if (save <= 0)
        save = 1;
    init();
    (void) fclose(outtype);  outtype=NULL;
    (void) fclose(clmfp);    clmfp=NULL;
    (void) fclose(emcfp);    emcfp=NULL;
    (void) fclose(mptfp);    mptfp=NULL;
    (void) fclose(dbfp);     dbfp=NULL;
    (void) fclose(sspfp);    sspfp=NULL;
    gauss_seidel();

    // sprintf(filename, "%s.mpdh", argv[1]);
    // save_distrib_human();

    compute_throughput();
    (void) fclose(altth);    altth=NULL;
    (void) fclose(stafp);    stafp=NULL;
    (void) fclose(nmrk);     nmrk=NULL;
    (void) fclose(denom);    denom=NULL;
    (void) fclose(ratesfp);  ratesfp=NULL;

    return 0;
}
