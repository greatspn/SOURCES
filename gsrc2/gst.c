/*
 *  Torino, September 11, 1987
 *  program: gst.c
 *  purpose: Computation of Performance Indices for a GSPN.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) NOT YET COMPLETELY DEBUGGED
 */


/*
#define DEBUG
*/


#define MAX_TOKEN 32767


#include <stdio.h>

FILE *trsfp, * mpdfp, * tpdfp;

#include "const.h"

#include "grg.h"

MARKP rootm;

/* DECODED MARKING */ extern unsigned char DP[];

/* CODING BYTES */ extern unsigned no_cod;

extern unsigned place_num, trans_num;


unsigned top_tan;

unsigned char *cmarks, * c_m_p;

struct p_bound_s {
    unsigned short min_t;
    unsigned short max_t;
};

struct p_bound_s *p_bound, * p_b_p;
double **t_prob, * *t_p_p;

extern unsigned maxprob;
extern unsigned maxres;
extern double PRO[];
extern double PROD[];
extern double STIMA[];
extern double ERROR[];
double *r_p;


char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *	calloc();
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}


extern load_compact(/* pp, fp */);
/*
  unsigned long * pp;
  FILE * fp;
*/

extern store_compact(/* nval, fp */);
/*
  unsigned long nval;
  FILE * fp;
*/

extern store_double(/* valp, fp */);
/*
  double * valp;
  FILE * fp;
*/

extern load_double(/* valp, fp */);
/*
  double * valp;
  FILE * fp;
*/

double tr_mean_tok(n_p)
unsigned n_p;
{
    struct p_bound_s *pb_p =  p_bound + (--n_p);
    register unsigned ntok = pb_p->min_t;
    register unsigned mtok = pb_p->max_t;
    double *d_p = *(t_prob + n_p);
    double res = ntok **d_p;

    while (++ntok <= mtok)
        res += ntok **++d_p;
    return (res);
}

static unsigned transient = FALSE;

#include <sys/types.h>
#include <sys/stat.h>

static char   *can_t_open = "Can't open file %s for %c\n";
char  netname[100];
char  filename[100];
extern FILE *fopen();

init() {
    struct stat stb, stb2;
    unsigned ii;
    register unsigned jj;
    unsigned char *uc_p;

#ifdef DEBUG
    fprintf(stderr, "  Start of init\n");
#endif
    sprintf(filename, "%s.ctrs", netname);
    if (stat(filename, &stb) < 0) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(33);
    }
    top_tan = stb.st_size / no_cod;
#ifdef DEBUG
    fprintf(stderr, "    top_tan=%d, place_num=%d, no_cod=%d, maxprob=%d\n",
            top_tan, place_num, no_cod, maxprob);
#endif
    if ((trsfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.mpd", netname);
    if ((stat(filename, &stb2) < 0)
            || (stb2.st_mtime <= stb.st_mtime)) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(35);
    }
    if ((mpdfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.tpd", netname);
    if ((tpdfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    for (r_p = PRO, ii = maxprob ; ii-- ;)
        *(r_p++) = 0.0;
    p_bound = (struct p_bound_s *)ecalloc(place_num, sizeof(struct p_bound_s));
    for (p_b_p = p_bound, ii = place_num ; ii-- ; ++p_b_p) {
        p_b_p->min_t = MAX_TOKEN;
        p_b_p->max_t = 0;
    }
    t_prob = (double **)ecalloc(place_num, sizeof(double *));
    cmarks = (unsigned char *)ecalloc(top_tan, no_cod);
    for (c_m_p = cmarks, ii = top_tan ; ii-- ;
            c_m_p += no_cod) {
        for (uc_p = c_m_p, jj = no_cod ; jj-- ;)
            *(uc_p++) = getc(trsfp);
        decode_mark(c_m_p);
        for (uc_p = DP, p_b_p = p_bound, jj = place_num ; jj-- ; ++p_b_p) {
            if (*++uc_p > p_b_p->max_t)
                p_b_p->max_t = *uc_p;
            if (*uc_p < p_b_p->min_t)
                p_b_p->min_t = *uc_p;
        }
    }
    (void) fclose(trsfp);
    for (t_p_p = t_prob, p_b_p = p_bound, ii = place_num ; ii-- ;
            ++p_b_p, ++t_p_p) {
        jj = p_b_p->max_t - p_b_p->min_t + 1;
#ifdef DEBUG
        fprintf(stderr, "      place #%d, min_t=%d, max_t=%d, %d items\n",
                place_num - ii, p_b_p->min_t, p_b_p->max_t, jj);
#endif
        *t_p_p = r_p = (double *)ecalloc(jj, sizeof(double));
        while (jj--)
            *(r_p++) = 0.0;
    }
#ifdef DEBUG
    fprintf(stderr, "  End of init\n");
#endif
}



main(argc, argv)
int argc;
char **argv;
{
    register unsigned long ii, jj, kk;
    unsigned char *uc_p;
    double mp, imp;
    double err;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    sprintf(netname, "%s", argv[1]);
    init();
    load_double(&mp, mpdfp); err = -mp;
    load_double(&mp, mpdfp);
    transient = (mp > 0.0);
    for (c_m_p = cmarks, ii = top_tan ; ii-- ; c_m_p += no_cod) {
        decode_mark(c_m_p);
        load_double(&mp, mpdfp);
        if (transient)
            load_double(&imp, mpdfp);
        else
            imp = mp;
        compute_pro(mp, imp);
        for (t_p_p = t_prob, p_b_p = p_bound, uc_p = DP , jj = place_num ;
                jj-- ; ++t_p_p, ++p_b_p) {
            kk = *++uc_p - p_b_p->min_t;
            (*t_p_p)[kk] += mp;
        }
    }
    (void) fclose(mpdfp);
    for (t_p_p = t_prob, p_b_p = p_bound , ii = place_num ;
            ii-- ; ++t_p_p, ++p_b_p) {
        mp = p_b_p->min_t; store_double(&mp, tpdfp);
        mp = p_b_p->max_t; store_double(&mp, tpdfp);
        for (jj = p_b_p->max_t - p_b_p->min_t + 1, r_p = *t_p_p ; jj-- ;)
            store_double(r_p++, tpdfp);
    }
    (void) fclose(tpdfp);
    compute_sim();
    for (r_p = &(PROD[1]), ii = 0 ; ii++ < maxres ; ++r_p) {
        STIMA[ii] = *r_p;
        ERROR[ii] = (*r_p) * err;
    }
    print_res(stdout);
#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
    return 0;
}

