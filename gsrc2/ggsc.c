/*
 *  Paris, February 1, 1990
 *  program: ggsc.c
 *  purpose: Steady-state solution of a GSPN EMC with Gauss-Seidel algorithm.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) NOT YET COMPLETELY DEBUGGED
 */


#define DEBUG 0



#define EPS 0.000001
#define MAXITER 1000

#include <stdlib.h>
#include <stdio.h>
#include "../WN/INCLUDE/compact.h"

FILE *emcfp, * dbfp, * sspfp;

double eps;
unsigned long maxiter;

unsigned long frm, tmp;
/****************ALEX LINUX ****************/
/* unsigned nel; */
unsigned long top_tan = 0;

double *dval;

struct mtrx_itm {
    unsigned long fm;
    unsigned long rt;
};

struct mtrx_col {
    unsigned long weight;
    unsigned long ninn;
    struct mtrx_itm *first;
};

unsigned long limit_tan;

struct mtrx_col *column;
double *mprob, * wprob;


struct mtrx_col *m_c_p;

unsigned long frm, nel, rr;

char  filename[200];

/****Marco 31/3/2012****/
int PRINT = 0;
double _prec = 0.01;
int _iter = 10000;
double _min = 0.0, _max = 10.0, _step = 1.0;
/****Marco 31/3/2012****/

void tofileSMART(char *netname) {
    struct mtrx_itm *item;
    short int *ctmc;
    struct mtrx_col *last_column;
    int i, j;
    //FILE *ctmcfile;
    double qq;
    int count_arcs = 0;
    int first = 0;
    FILE *ctmcfile;
    int for_step = (_max - _min) / _step;
    double increm = _min;
    sprintf(filename, "%s.sm", netname);
    ctmcfile = fopen(filename, "w");
    if (ctmcfile == NULL) {
        printf("Error opening file: %s", filename);
        exit(1);
    }
    fprintf(ctmcfile, "# MaxNumericalIters %d\n# NumericalPrecision %f \n\n", _iter, _prec);
    fprintf(ctmcfile, "ctmc c := {\n");
    fprintf(ctmcfile, "for (int i in {0..%lu})\n\t{\n\tstate s[i];\n\t}\n", limit_tan - 1);
    fprintf(ctmcfile, "init(s[0]:1.0);\n");
    fprintf(ctmcfile, "arcs(");
    /* limit_tan number of tangible states */
    last_column = column + limit_tan + 1;
    for (m_c_p = column + 1 ; m_c_p < last_column ; m_c_p++) {
        int indice = m_c_p - column;
        count_arcs++;
        for (i = m_c_p->ninn, item = m_c_p->first ; i > 0 ; i--, item++) {
            if (first == 0) {
                first = 1;
                fprintf(ctmcfile, "s[%lu]:s[%d]:%f", item->fm - 1, indice - 1, dval[item->rt] / dval[(column + item->fm)->weight]);
            }
            else
                fprintf(ctmcfile, ", s[%lu]:s[%d]:%f", item->fm - 1, indice - 1, dval[item->rt] / dval[(column + item->fm)->weight]);
            count_arcs++;
        }
    }
    fprintf(ctmcfile, ");\n"/*,limit_tan,count_arcs*/);
    //steady state for each states
    fprintf(ctmcfile, "for (int i in {0..%lu})\n\t{\n\treal m[i] := prob_ss(in_state(s[i]));\n\t}\n", limit_tan - 1);

    for (j = 0; j <= for_step; j++) {
        fprintf(ctmcfile, "for (int i in {0..%lu})\n\t{\n\treal t%d[i] := prob_at(in_state(s[i]),%f);\n\t}\n", limit_tan - 1, j, increm);
        increm = increm + _step;
    }
    fprintf(ctmcfile, "};\n\n");
    fprintf(ctmcfile, "print(\"-------------------------------------------\\n\");\n");
    fprintf(ctmcfile, "print(\"STEADY STATE SOLUTION:\\n\");");
    fprintf(ctmcfile, "for (int i in {0..%lu})\n\t{\n\tprint(\"\\tState \",i, \"= \",c.m[i],\"\\n\");\n\t}\n", limit_tan - 1);
    fprintf(ctmcfile, "print(\"-------------------------------------------\\n\\n\");\n");
    fprintf(ctmcfile, "print(\"-------------------------------------------\\n\");\n");
    fprintf(ctmcfile, "print(\"TRANSIENT SOLUTION:\\n\");");
    increm = _min;
    for (j = 0; j <= for_step; j++) {
        fprintf(ctmcfile, "print(\"Time:%f \\n\");\n", increm);
        fprintf(ctmcfile, "for (int i in {0..%lu})\n\t{\n\tprint(\"\\tState \",i, \"= \",c.t%d[i],\"\\n\");\n\t}\n", limit_tan - 1, j);
        increm = increm + _step;
    }
    fprintf(ctmcfile, "print(\"-------------------------------------------\\n\");\n");
    fclose(ctmcfile);
}

void tofileDot(char *netname) {
    struct mtrx_itm *item;
    short int *ctmc;
    struct mtrx_col *last_column;
    int i, j;
    FILE *ctmcfile;
    double qq;
    int count_arcs = 0;

    sprintf(filename, "%s.ctmc", netname);
    ctmcfile = fopen(filename, "w");
    if (ctmcfile == NULL) {
        printf("Error opening file: %s", filename);
        exit(1);
    }
    /* limit_tan number of tangible states */
    fprintf(ctmcfile, "digraph CTMC {\n");
    last_column = column + limit_tan + 1;
    for (m_c_p = column + 1 ; m_c_p < last_column ; m_c_p++) {
        int indice = m_c_p - column;
        //printf("column %d %d %f: ", indice, m_c_p->weight,dval[m_c_p->weight]);
        fprintf(ctmcfile, "T%d [label=\"T%d \"]\n", indice, indice);
        //fprintf(stdout,"T%d -> T%d [label=\" %f \"]\n",indice,indice,-1/dval[m_c_p->weight]);
        count_arcs++;
        for (i = m_c_p->ninn, item = m_c_p->first ; i > 0 ; i--, item++) {
            fprintf(ctmcfile, "T%lu -> T%d [label=\" %f\"]\n", item->fm, indice, dval[item->rt] / dval[(column + item->fm)->weight]);
            count_arcs++;
        }
    }
    /*       fprintf(stdout,"states %d arcs %d \n",limit_tan,count_arcs); */
    fprintf(ctmcfile, "}");
    fclose(ctmcfile);
}

//   fprintf(ctmcfile, "\n");
void tofileText(char *netname) {
    struct mtrx_itm *item;
    short int *ctmc;
    struct mtrx_col *last_column;
    int i, j;
    FILE *ctmcfile;
    double qq;


    /* limit_tan number of tangible states */
    ctmc = malloc(limit_tan * limit_tan * sizeof(short int));
    last_column = column + limit_tan + 1;
    // azzero matrice
    dval[0] = 0.0;
    for (i = 0; i < limit_tan; i++)
        for (j = 0; j < limit_tan; j++)
            ctmc[i * limit_tan + j] = 0;
    fflush(stdout);
    for (m_c_p = column + 1 ; m_c_p < last_column ; m_c_p++) {
        int indice = m_c_p - column;
        wprob[indice] = dval[m_c_p->weight];
        for (i = m_c_p->ninn, item = m_c_p->first ; i > 0 ; i--, item++) {
            ctmc[(item->fm - 1)*limit_tan + (m_c_p - column - 1)] = item->rt;
        }
    }

    sprintf(filename, "%s.ctmc", netname);
    ctmcfile = fopen(filename, "w");
    if (ctmcfile == NULL) {
        printf("Error opening file: %s", filename);
        exit(1);
    }
    fprintf(ctmcfile, "# name: Q\n# type: global matrix\n# rows: %lu\n# columns: %lu\n", limit_tan, limit_tan);


    for (i = 0 ; i < limit_tan ; i++) {
        fprintf(ctmcfile, "\n");
        qq = 0;
        for (j = 0 ; j < limit_tan ; j++) {
            qq += dval[ ctmc[i * limit_tan + j]] / wprob[i + 1];
        }
        for (j = 0 ; j < limit_tan ; j++)
            if (i != j)
                fprintf(ctmcfile, "%e ", dval[ ctmc[i * limit_tan + j] ] / wprob[i + 1]);
            else
                fprintf(ctmcfile, "-%e ", qq);
    }

    fprintf(ctmcfile, "\n");
    printf("\toutput file: %s\n", filename);
    fclose(ctmcfile);
}



void load_mat() {
    register struct mtrx_itm *r_p;
    register unsigned i;

#if  DEBUG > 1
    fprintf(stderr, "  Start of load_mat\n");
#endif
    ++m_c_p;
    load_compact(&rr, emcfp);
    m_c_p->weight = rr;
#if  DEBUG > 1
    fprintf(stderr, "    to mark #%d, weight #%d, %d entries\n",
            top_tan, rr, nel);
#endif
    if ((int)(m_c_p->ninn = nel)) {
        m_c_p->first = r_p =
                           (struct mtrx_itm *)calloc(nel, sizeof(struct mtrx_itm));
        for (i = nel ; i-- ; ++r_p) {
            load_compact(&frm, emcfp);
            load_compact(&rr, emcfp);
#if  DEBUG > 1
            fprintf(stderr, "      from mark #%d, rate #%d\n", frm, rr);
#endif

            r_p->fm = frm;  r_p->rt = rr;
        }
    }
    else
        m_c_p->first = NULL;
#if  DEBUG > 1
    fprintf(stderr, "  End of load_mat\n");
#endif
}


void init() {
    double *d_p;
    double dd, sum;
    int transient;

#if  DEBUG > 1
    fprintf(stderr, "  Start of init\n");
#endif
    load_compact(&limit_tan, emcfp);
#if  DEBUG > 1
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
#if  DEBUG > 1
    fprintf(stderr, "    %d rate values:\n", (nel = frm));
#endif
    for (d_p = dval ; frm-- ;)
#if  DEBUG > 1
    {
#endif
        load_double(++d_p, dbfp);
#if  DEBUG > 1
        fprintf(stderr, "    %d -> %f\n", (nel - frm), *d_p);
    }
#endif
    load_double(&dd, sspfp);
#if  DEBUG > 1
    fprintf(stderr, "\nPrevious solution err=%f", dd);
#endif
    load_double(&dd, sspfp);
#if  DEBUG > 1
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
#if  DEBUG > 1
            fprintf(stderr, " #%d prob=%f", top_tan - nel, *d_p);
#endif
            frm = (++m_c_p)->weight;
#if  DEBUG > 1
            fprintf(stderr, " (weight #%d =%f)", frm, dval[frm]);
#endif
            if ((dd = dval[frm]) > 0.0)
                * d_p /= dd;
#if  DEBUG > 1
            fprintf(stderr, "  normalized: %f\n", *d_p);
#endif
            sum += *d_p;
        }
        dd = 1.0 / sum;
#if  DEBUG > 1
        fprintf(stderr, "\nsum=%f dd=%f\n", sum, dd);
#endif
        for (d_p = mprob, nel = top_tan ; nel-- ;) {
            *++d_p *= dd;
        }
    }
#if  DEBUG > 1
    fprintf(stderr, "  End of init\n");
#endif

}

char netname[200];
static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[200];

double err;


extern FILE *fopen();

void save_distrib() {
    double rr, sum, dd;
    unsigned rn;
    double *res_p, * m_p;
    struct mtrx_col *c_p;
    int ii;
    char command[400];

#if  DEBUG > 1
    fprintf(stderr, "  Start of save_distr\n");
#endif
    sprintf(filename, "%s.mpd", netname);
    if ((sspfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sum = 0.0;
#if  DEBUG > 1
    fprintf(stderr, "    top_tan = %d\n", top_tan);
#endif
    for (res_p = mprob, m_p = wprob, ii = top_tan, c_p = column; ii-- ;) {
#if  DEBUG > 1
        fprintf(stderr, "        normalizing %d\n", top_tan - ii);
#endif
        rn = (++c_p)->weight;
        rr = *++res_p;
        if ((dd = dval[rn]) > 0.0)
            rr *= dd;
        *++m_p = rr;
        sum += rr;
    }
#if  DEBUG > 1
    fprintf(stderr, "  sum=%f\n", sum);
#endif
    dd = -err; store_double(&dd, sspfp);
    dd = -1.0; store_double(&dd, sspfp);
    dd = 1.0 / sum;
    for (ii = top_tan, m_p = wprob ; ii-- ;) {
#if  DEBUG > 1
        fprintf(stderr, "         store  %d\n", top_tan - ii);
#endif
        rr = *++m_p * dd;
        store_double(&rr, sspfp);
        //fprintf(stdout,"%d = %lg\n",ii+1,rr);
    }
#if  DEBUG > 1
    fprintf(stderr, "    normalization done\n");
#endif
    (void) fclose(sspfp);
    sprintf(command, "/bin/cp %s.mpd %s.epd", netname, netname);
    system(command);
#if  DEBUG > 1
    fprintf(stderr, "  End of save_distr\n");
#endif
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
        i = max_i; ++noiter;  err = sum = 0.0;
#if  DEBUG > 1
        fprintf(stderr, "    gauss_seidel  noiter=%d, i=%d\n", noiter, i);
#endif
        for (res_p = mprob + 1, c_p = column + 1 ; --i ; ++res_p, ++c_p) {
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
#if  DEBUG > 1
            fprintf(stderr, "          item %d r=%f prev=%f rr=%f err=%f\n", max_i - i,
                    r, *res_p, rr, err);
#endif
            *res_p = r;
            sum += r;
        }
        if ((rr = sum - 1.0) > 0.01 || rr < -0.01) {
            rr = 1.0 / sum;
#if  DEBUG > 1
            fprintf(stderr, "      RENORMALIZE   sum=%f rr=%f\n", sum, rr);
#endif
            for (i = max_i, res_p = mprob ; --i ;) {
                *++res_p *= rr;
            }
        }
        if (noiter >= next_save) {
            next_save += save;
            save_distrib();
//#ifndef DEBUG
            fprintf(stderr, " iteration %5ld error %f\n", noiter, err);
//#endif
        }
    }
    while
    (err > eps && noiter < maxiter);
#if  DEBUG > 1
    fprintf(stderr, "end iterations err=%f noiter=%d\n", err, noiter);
#endif
    save_distrib();
    fprintf(stderr, " iteration %5ld error %f\n\n", noiter, err);
}


int main(int argc, char **argv) {

#if  DEBUG > 1
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
        unsigned jj = 1;
        while (ii--) {
            s_p = *(a_p++);
            jj++;
            /****Marco 31/3/2012****/
            if (*(s_p++) != '-' || (*s_p != 'e' && *s_p != 'i' && *s_p != 's' && *s_p != 'S' && *s_p != 'T'   && *s_p != 'D')) {
                fprintf(stderr, "ERROR: unknown parameter '%s'\n", s_p - 1);
                exit(33);
            }
            /****Marco 31/3/2012****/
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
            /****Marco 31/3/2012****/
            case 'S' : {
                printf("\nSMART OUTPUT %d\n", jj);
                PRINT = 1;
                if (jj + 5 < argc) {
                    jj++;
                    _prec = atof(argv[jj]);
                    jj++;
                    _iter = atoi(argv[jj]);
                    jj++;
                    _min = atof(argv[jj]);
                    jj++;
                    _max = atof(argv[jj]);
                    jj++;
                    _step = atof(argv[jj]);
                    a_p = a_p + 5;
                    ii = ii - 5;

                }
                else {
                    fprintf(stderr, "ERROR: -S <double_precision> <integer_iteration> <transient_double_min> <transient_double_max> <transient_int_step> \n");
                    exit(33);
                }
            }
            break;
            case 'T' : {

                PRINT = 2;
            }
            break;
            case 'D' : {

                PRINT = 3;
            }
            break;
                /****Marco 31/3/2012****/
            }
        }
    }
    if (save <= 0)
        save = 1;
    init();
    /****Marco 31/3/2012****/
    if (PRINT) {
        printf("\n-------------------------------------------------------------------------\n");
        printf("|\t\t\tPRINT CTMC IN A FILE\t\t\t\t|");
        printf("\n_________________________________________________________________________\n\n");
        switch (PRINT) {
        case 1:
            printf("\tSMART OUTPUT\n");
            printf("\tPrecision:%f\n\tMax Iterations:%d\n\tTime range:%f to %f", _prec, _iter, _min, _max);
            tofileSMART(argv[1]);
            break;
        case 2:
            printf("\tTextual OUTPUT\n");
            tofileText(argv[1]);
            break;
        case 3:
            printf("\tDOT OUTPUT\n");
            tofileDot(argv[1]);
        }
        printf("\n-------------------------------------------------------------------------\n\n");
        return 0;
    }
    /****Marco 31/3/2012****/
#if  DEBUG > 1
    fprintf(stderr, "eps=%f, maxiter=%d\n", eps, maxiter);
#endif
    (void) fclose(emcfp);
    (void) fclose(dbfp);
    (void) fclose(sspfp);
    gauss_seidel();

#if  DEBUG > 1
    fprintf(stderr, "End\n");
#endif
    return 0;
}

