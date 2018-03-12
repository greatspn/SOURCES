/*
 *  L.A. 9/3/92
 *  program: seemtx.c
 *  purpose: show the contents of emc and epd files
 *  programmer: Giuly
 *  notes:
 *           derived from ggsc.c
 */

/*
#define DEBUG
*/


#define EPS 0.000001
#define MAXITER 1000

#include <stdio.h>
#include <stdlib.h>
#include "../WN/INCLUDE/compact.h"

FILE *emcfp, * dbfp, * sspfp;

double eps;
unsigned long maxiter;

unsigned long frm, tmp;
/*unsigned nel;*/
unsigned long top_tan = 0;

double *dval;

struct mtrx_itm {
    unsigned long fm;
    unsigned short rt;
};

struct mtrx_col {
    unsigned short weight;
    unsigned short ninn;
    struct mtrx_itm *first;
};

unsigned long limit_tan;

struct mtrx_col *column;
double *mprob, * wprob;


struct mtrx_col *m_c_p;

unsigned long frm, nel, rr;

load_mat() {
    register struct mtrx_itm *r_p;
    register unsigned i;

#ifdef DEBUG
    fprintf(stderr, "  Start of load_mat\n");
#endif
    ++m_c_p;
    load_compact(&rr, emcfp);
    m_c_p->weight = rr;
    /* Print column header */
    fprintf(stderr, ">>    to mark #%d, weight #%d, %d entries\n",
            top_tan, rr, nel);
    if ((int)(m_c_p->ninn = nel)) {
        m_c_p->first = r_p =
                           (struct mtrx_itm *)calloc(nel, sizeof(struct mtrx_itm));
        for (i = nel ; i-- ; ++r_p) {
            load_compact(&frm, emcfp);
            load_compact(&rr, emcfp);
            /* Print row element for column top_tan */
            fprintf(stderr, "      from mark #%d, rate #%d\n", frm, rr);
            r_p->fm = frm;  r_p->rt = rr;
        }
    }
    else
        m_c_p->first = NULL;
#ifdef DEBUG
    fprintf(stderr, "  End of load_mat\n");
#endif
}


init() {
    double *d_p;
    double dd, sum;
    int transient;

#ifdef DEBUG
    fprintf(stderr, "  Start of init\n");
#endif
    load_compact(&limit_tan, emcfp);
#ifdef DEBUG
    fprintf(stderr, "    limit_tan=%d\n", limit_tan);
#endif
    if ((column = (struct mtrx_col *)calloc(limit_tan + 1,
                                            sizeof(struct mtrx_col))) == NULL ||
            (mprob = (double *)calloc(limit_tan + 1,
                                      sizeof(double))) == NULL ||
            (wprob = (double *)calloc(limit_tan + 1,
                                      sizeof(double))) == NULL) {
        fprintf(stderr, "calloc: couldn't get enough memory\n");
        exit(35);
    }
    /* Load one column at a time */
    for (m_c_p = column, load_compact(&nel, emcfp) ; top_tan++ < limit_tan ;
            load_compact(&nel, emcfp)) {
        load_mat();
    }
    top_tan = limit_tan;
    load_compact(&frm, dbfp);
    if ((dval = (double *)calloc(frm + 1,
                                 sizeof(double))) == NULL) {
        fprintf(stderr, "calloc: couldn't get enough memory\n");
        exit(35);
    }
    fprintf(stderr, "    %d rate values:\n", (nel = frm));
    for (d_p = dval ; frm-- ;) {
        load_double(++d_p, dbfp);
        /* Print doubles values*/
        fprintf(stderr, "Double# %d -> %f\n", (nel - frm), *d_p);
    }
    load_double(&dd, sspfp);
#ifdef DEBUG
    fprintf(stderr, "\nPrevious solution err=%f", dd);
#endif
    load_double(&dd, sspfp);
#ifdef DEBUG
    fprintf(stderr, " time=%f\n\n", dd);
#endif
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
            /* print probability distribution */
            fprintf(stderr, " #%d prob=%f", top_tan - nel, *d_p);
            fprintf(stderr, "\n");
            frm = (++m_c_p)->weight;
            /* fprintf(stderr," (weight #%d =%f)", frm, dval[frm] ); */
            if ((dd = dval[frm]) > 0.0)
                * d_p /= dd;
            /* fprintf(stderr,"  normalized: %f\n", *d_p ); */
            sum += *d_p;
        }
        dd = 1.0 / sum;
#ifdef DEBUG
        fprintf(stderr, "\nsum=%f dd=%f\n", sum, dd);
#endif
        for (d_p = mprob, nel = top_tan ; nel-- ;) {
            *++d_p *= dd;
            /*  fprintf(stderr,"  normalized: %f\n", *d_p ); */ /* Or before normalization? */
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End of init\n");
#endif
}

char netname[200];
static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[200];

double err;


extern FILE *fopen();

save_distrib() {
    double rr, sum, dd;
    unsigned rn;
    double *res_p, * m_p;
    struct mtrx_col *c_p;
    int ii;
    char command[400];

#ifdef DEBUG
    fprintf(stderr, "  Start of save_distr\n");
#endif
    sprintf(filename, "%s.mpd", netname);
    if ((sspfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sum = 0.0;
#ifdef DEBUG
    fprintf(stderr, "    top_tan = %d\n", top_tan);
#endif
    for (res_p = mprob, m_p = wprob, ii = top_tan, c_p = column; ii-- ;) {
#ifdef DEBUG
        fprintf(stderr, "        normalizing %d\n", top_tan - ii);
#endif
        rn = (++c_p)->weight;
        rr = *++res_p;
        if ((dd = dval[rn]) > 0.0)
            rr *= dd;
        *++m_p = rr;
        sum += rr;
    }
#ifdef DEBUG
    fprintf(stderr, "  sum=%f\n", sum);
#endif
    dd = -err; store_double(&dd, sspfp);
    dd = -1.0; store_double(&dd, sspfp);
    dd = 1.0 / sum;
    for (ii = top_tan, m_p = wprob ; ii-- ;) {
#ifdef DEBUG
        fprintf(stderr, "         store  %d\n", top_tan - ii);
#endif
        rr = *++m_p * dd;
        store_double(&rr, sspfp);
    }
#ifdef DEBUG
    fprintf(stderr, "    normalization done\n");
#endif
    (void) fclose(sspfp);
    sprintf(command, "/bin/cp %s.mpd %s.epd", netname, netname);
    system(command);
#ifdef DEBUG
    fprintf(stderr, "  End of save_distr\n");
#endif
}


int save = 0;

/* print_mtx()
    {
     }
*/
main(argc, argv)
int argc;
char **argv;
{
    unsigned long ii;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
#endif
    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(netname, argv[1]);
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
    eps = EPS; maxiter = MAXITER; save = 1;
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii--) {
            s_p = *(a_p++);
            if (*(s_p++) != '-' ||
                    *s_p != 'e' && *s_p != 'i' && *s_p != 's') {
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
            case 'i' : sscanf(s_p + 1, "%d", &maxiter);
                break;
            case 's' : sscanf(s_p + 1, "%d", &save);
                break;
            }
        }
    }
    if (save <= 0)
        save = 1;
    init();
#ifdef DEBUG
    fprintf(stderr, "eps=%f, maxiter=%d\n", eps, maxiter);
#endif
    (void) fclose(emcfp);
    (void) fclose(dbfp);
    (void) fclose(sspfp);
    /*     print_mtx(); */

#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
}

