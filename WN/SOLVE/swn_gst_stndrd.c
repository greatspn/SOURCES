/*
 *  Paris, February 8, 1990
 *  program: gst_stndrd.c
 *  purpose: Computation of Performance Indices for a GSPN without compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) NOT YET COMPLETELY DEBUGGED
 */


/*
#define DEBUG
*/


#define MAX_TOKEN 32767

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../INCLUDE/compact.h"

FILE *nfp, * trsfp, * mpdfp, * tpdfp;

#include "sconst.h"

#include "grg.h"


unsigned place_num, group_num, trans_num;

struct Codbyt_descr {
    unsigned char shift;
    unsigned char mask;
    unsigned short num_byte;
    unsigned short link;
};

struct Place_descr {
    unsigned short lbound;
    unsigned short rubound;
    struct Codbyt_descr cod;
};

/*  PLACE ARRAY  */ struct Place_descr *p_list;

unsigned sub_num;

/* CODING BYTES */ unsigned no_cod;


struct Byte_descr {
    unsigned char noitems;
    unsigned char max_conf;
    unsigned short first_item;
};

struct Byte_descr *encoding;


/* DECODED MARKING */ unsigned short *DM;

unsigned lastan, toptan = 0;


unsigned char *cmarks, * c_m_p;

struct p_bound_s {
    unsigned short min_t;
    unsigned short max_t;
};

struct p_bound_s *p_bound, * p_b_p;
double **t_prob, * *t_p_p;


struct Cond_descr {
    char op;
    char type;
    char top1;
    char top2;
    int op1;
    int op2;
};


struct Prob_descr {
    char type;
    int place;
    struct Cond_descr *cond;
    double val;
};

unsigned maxprob = 0;
unsigned maxres = 0;
struct Prob_descr *Prob;

double *r_p;


char *ecalloc(unsigned nitm,  unsigned sz) {
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
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


void decode_mark(unsigned char *Mp, unsigned short *dmp) {
    struct Place_descr *place_p = p_list;
    unsigned np, nb, val;

    for (np = place_num ; np-- ; place_p++) {
        if ((nb = place_p->cod.num_byte)) {
            nb--;
            val = Mp[nb] & place_p->cod.mask;
            val = val >> (place_p->cod.shift);
            val += place_p->lbound;
            *(++dmp) = val;
        }
        else
            *(++dmp) = 0;
    }
}


double tr_mean_tok(unsigned n_p) {
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
char  netname[1024];
char  filename[1024+48];

void init() {
    char cc;
    unsigned char *uc_p;
    unsigned short *us_p;
    int ii, nn, nt, jj, jj2, kk, ll, mm, np, n1, n2, n3, n4;
    struct Byte_descr *byte_p;
    struct Place_descr *place_p;
    struct Codbyt_descr *cods_p;

#ifdef DEBUG
    fprintf(stderr, "  Start of init\n");
#endif

    sprintf(filename, "%s.rgr_aux", netname);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    fscanf(nfp, "toptan= %d\n", &toptan);
    (void) fclose(nfp);

    sprintf(filename, "%s.grg", netname);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }

    /* read number of objects in the net */
    fscanf(nfp, "%d %d %d %d\n", &sub_num, &place_num, &group_num, &trans_num);

    DM = (unsigned short *)ecalloc(place_num + 1, sizeof(unsigned short));
    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));

#ifdef DEBUG
    fprintf(stderr, "    toptan=%d, place_num=%d, no_cod=%d, maxprob=%d\n",
            toptan, place_num, no_cod, maxprob);
#endif

    /* read codings */
    for (nn = 0 ; nn < sub_num ; nn++) {
        fscanf(nfp, "%d %d\n", &ii, &jj2);
#ifdef DEBUG
        fprintf(stderr, "  subnet #%d (%d bytes):\n",
                nn, jj2);
#endif
        if (nn == 0) {
            encoding = (struct Byte_descr *)ecalloc(jj2, sizeof(struct Byte_descr));
            no_cod = jj2;
        }
        for (byte_p = encoding, ii = 0 ;
                ++ii <= jj2 ; byte_p++) {
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &kk, &ll, &np, &mm);
            if (nn == 0) {
                byte_p->max_conf = ll;
                byte_p->noitems = np;
            }
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &n1, &n2, &n3, &n4);
            if (nn == 0)
                byte_p->first_item = jj;
            for (kk = 1 ; ++kk <= np ;) {
                fscanf(nfp, "%d", &ll);
                if (nn == 0) {
                    place_p = p_list + (jj - 1);
                    cods_p = &(place_p->cod);
                    cods_p->link = ll;
                    jj = ll;
                    cods_p->num_byte = ii;
                    cods_p->mask = n1;
                    cods_p->shift = n2;
                }
                fscanf(nfp, "%d %d %d %d\n", &n1, &n2, &n3, &n4);
            }
            if (nn == 0) {
                place_p = p_list + (jj - 1);
                cods_p = &(place_p->cod);
                cods_p->link = 0;
                cods_p->num_byte = ii;
                cods_p->mask = n1;
                cods_p->shift = n2;
            }
#ifdef DEBUG
            fprintf(stderr, "    coding #%d (%d configurations)\n",
                    ii, jj2);
#endif
        }
    }

    /* read places */
#ifdef DEBUG
    fprintf(stderr, "   places:\n");
#endif
    for (ii = 0, place_p = p_list; ++ii <= place_num; place_p++) {
        fscanf(nfp, "%d %d %d %d %d %d %d",
               &np, &jj, &kk, &ll, &nt, &nn, &mm);
        place_p->lbound = nt;
        place_p->rubound = nn - nt;
        while (getc(nfp) != '\n');
#ifdef DEBUG
        fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                ii, place_p->cod.mask, place_p->cod.shift, place_p->cod.num_byte);
        fprintf(stderr, "            lbound=%d rubound=%d notok=%d link=%d\n",
                place_p->lbound, place_p->rubound, mm, place_p->cod.link);
#endif
    }
    (void) fclose(nfp);

    sprintf(filename, "%s.gst", netname);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    fscanf(nfp, "%d %d\n", &maxprob, &maxres);
    Prob = (struct Prob_descr *)ecalloc(maxprob + 1,
                                        sizeof(struct Prob_descr));
    for (ii = maxprob ; ii-- ;) {
        fscanf(nfp, "%d %d", &jj, &ll);
        while ((cc = getc(nfp)) == ' ');
        Prob[jj].val = 0.0;
        Prob[jj].type = cc;
        switch (cc) {
        case 'e' :
        case 'E' :
        case 'c' :
        case 'C' :
            fscanf(nfp, "%d\n", &kk);
            break;
        case 'p' :
        case 'P' :
            while (getc(nfp) != '\n');
            kk = 0;
            break;
        }
        Prob[jj].place = kk;
#ifdef DEBUG
        fprintf(stderr, "   prob #%d type=%c place=%d nocond=%d\n",
                jj, cc, kk, ll);
#endif
        if (ll) {
            struct Cond_descr *ccp;
            Prob[jj].cond = ccp = (struct Cond_descr *)
                                  ecalloc(ll, sizeof(struct Cond_descr));
            while (ll--) {
                while ((cc = getc(nfp)) == ' ');
                if ((ccp->op = cc) == 'c') {
                    while ((cc = getc(nfp)) == ' ');
                    ccp->type = cc;
                    while ((cc = getc(nfp)) == ' ');
                    ccp->top1 = cc;
                    fscanf(nfp, "%d", &kk);
                    ccp->op1 = kk;
                    while ((cc = getc(nfp)) == ' ');
                    ccp->top2 = cc;
                    fscanf(nfp, "%d\n", &kk);
                    ccp->op2 = kk;
#ifdef DEBUG
                    fprintf(stderr, "     cond (%c %d) %c (%c %d)\n",
                            ccp->top1, ccp->op1, ccp->type, ccp->top2, ccp->op2);
#endif
                }
                else {
#ifdef DEBUG
                    fprintf(stderr, "     op %c\n", cc);
#endif
                    while (getc(nfp) != '\n');
                }
                ++ccp;
            }
        }
        else
            Prob[jj].cond = NULL;
    }

    sprintf(filename, "%s.ctrs", netname);
    if ((trsfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.tpd", netname);
    if ((tpdfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    p_bound = (struct p_bound_s *)ecalloc(place_num, sizeof(struct p_bound_s));
    for (p_b_p = p_bound, ii = place_num ; ii-- ; ++p_b_p) {
        p_b_p->min_t = MAX_TOKEN;
        p_b_p->max_t = 0;
    }
    t_prob = (double **)ecalloc(place_num, sizeof(double *));
    cmarks = (unsigned char *)ecalloc(toptan, no_cod);
    for (c_m_p = cmarks, ii = toptan ; ii-- ;
            c_m_p += no_cod) {
        for (uc_p = c_m_p, jj = no_cod ; jj-- ;)
            *(uc_p++) = getc(trsfp);
        decode_mark(c_m_p, DM);
#ifdef DEBUG
        for (us_p = DM, p_b_p = p_bound, jj = place_num ; jj-- ; ++p_b_p)
            fprintf(stderr, "%d ", (int) *++us_p);
        fprintf(stderr, "\n");
#endif
        for (us_p = DM, p_b_p = p_bound, jj = place_num ; jj-- ; ++p_b_p) {
            if (*++us_p > p_b_p->max_t)
                p_b_p->max_t = *us_p;
            if (*us_p < p_b_p->min_t)
                p_b_p->min_t = *us_p;
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


static struct Cond_descr *cond_stack;


int evaluate_next(unsigned short *mrk) {
    int val, val2;
    char op = (++cond_stack)->op;

#ifdef DEBUG
    fprintf(stderr, "      Start of evaluate_next %c\n", op);
#endif
    if (op == 'c') {
        val = cond_stack->op1;
        if (cond_stack->top1 == 'p')
            val = mrk[val];
        val2 = cond_stack->op2;
        if (cond_stack->top2 == 'p')
            val2 = mrk[val2];
#ifdef DEBUG
        fprintf(stderr, "      End of evaluate_next c %d %c %d\n",
                val, cond_stack->type, val2);
#endif
        switch (cond_stack->type) {
        case '=' :
            return (val == val2);
        case 'n' :
            return (val != val2);
        case 'g' :
            return (val >= val2);
        case '>' :
            return (val > val2);
        case 'l' :
            return (val <= val2);
        case '<' :
            return (val < val2);
        }
    }
    val = evaluate_next(mrk);
    if (op == '~') {
#ifdef DEBUG
        fprintf(stderr, "      End of evaluate_next ~ %d\n", val);
#endif
        return (! val);
    }
    val2 = evaluate_next(mrk);
#ifdef DEBUG
    fprintf(stderr, "      End of evaluate_next %d %c %d\n", val, op, val2);
#endif
    if (op == '|')
        return (val || val2);
    else
        return (val && val2);
}


int evaluate_cond(struct Cond_descr *cond, unsigned short *mrk) {
    int val, val2;

#ifdef DEBUG
    fprintf(stderr, "    Start of evaluate_cond %c\n", cond->op);
#endif
    if (cond->op == 'c') {
        val = cond->op1;
        if (cond->top1 == 'p')
            val = mrk[val];
        val2 = cond->op2;
        if (cond->top2 == 'p')
            val2 = mrk[val2];
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond c %d %c %d\n",
                val, cond->type, val2);
#endif
        switch (cond->type) {
        case '=' :
            return (val == val2);
        case 'n' :
            return (val != val2);
        case 'g' :
            return (val >= val2);
        case '>' :
            return (val > val2);
        case 'l' :
            return (val <= val2);
        case '<' :
            return (val < val2);
        }
    }
    cond_stack = cond;
    val = evaluate_next(mrk);
    if (cond->op == '~') {
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond ~ %d\n", val);
#endif
        return (! val);
    }
    if ((cond->op == '|') && val) {
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond fast-or TRUE\n");
#endif
        return (TRUE);
    }
    if ((cond->op == '&') && ! val) {
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond fast-and FALSE\n");
#endif
        return (FALSE);
    }
    val = evaluate_next(mrk);
#ifdef DEBUG
    fprintf(stderr, "    End of evaluate_cond %c val=%d\n", cond->op, val);
#endif
    return (val);
}


void compute_pro(double mp, double imp,
                 unsigned short *mrk) {
    int ii, nt;
    struct Prob_descr *pp;

#ifdef DEBUG
    fprintf(stderr, "  Start of compute_pro\n");
#endif
    for (pp = Prob, ii = maxprob ; ii-- ;) {
        switch ((++pp)->type) {
        case 'e' :
            if ((nt = mrk[pp->place])) {
                if (nt == 1)
                    pp->val += mp;
                else
                    pp->val += (mp * nt);
            }
            break;
        case 'E' :
            if ((nt = mrk[pp->place])) {
                if (nt == 1)
                    pp->val += imp;
                else
                    pp->val += (imp * nt);
            }
            break;
        case 'c' :
            if ((nt = mrk[pp->place])
                    && evaluate_cond(pp->cond, mrk)) {
                if (nt == 1)
                    pp->val += mp;
                else
                    pp->val += (mp * nt);
            }
            break;
        case 'C' :
            if ((nt = mrk[pp->place])
                    && evaluate_cond(pp->cond, mrk)) {
                if (nt == 1)
                    pp->val += imp;
                else
                    pp->val += (imp * nt);
            }
            break;
        case 'p' :
            if (evaluate_cond(pp->cond, mrk)) {
                pp->val += mp;
            }
            break;
        case 'P' :
            if (evaluate_cond(pp->cond, mrk)) {
                pp->val += imp;
            }
            break;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End of compute_pro\n");
#endif
}


double evaluate_res() {
    char cc;
    double dd, val1, val2;
    int nn;

    while ((cc = getc(nfp)) == ' ');
#ifdef DEBUG
    fprintf(stderr, "  Start evaluate_res %c\n", cc);
#endif
    if (cc == 'o') {
        while ((cc = getc(nfp)) == ' ');
        if (cc == 'r') {
            fscanf(nfp, "%lf\n", &dd);
#ifdef DEBUG
            fprintf(stderr, "   End evaluate_res r %f\n", dd);
#endif
            return (dd);
        }
        fscanf(nfp, "%d\n", &nn);
#ifdef DEBUG
        fprintf(stderr, "   End evaluate_res %c %d %f\n", cc, nn, Prob[nn].val);
#endif
        return (Prob[nn].val);
    }
    while (getc(nfp) != '\n');
    val2 = evaluate_res();
    val1 = evaluate_res();
#ifdef DEBUG
    fprintf(stderr, "   End evaluate_res %f %c %f\n", val1, cc, val2);
#endif
    switch (cc) {
    case '+' :
        return (val1 + val2);
    case '-' :
        return (val1 - val2);
    case '*' :
        return (val1 * val2);
    case '/' :
        return (val1 / val2);
    default :
        return 0;
    }
}


void compute_res() {
    char cc;
    int ii;
    double resval;

#ifdef DEBUG
    fprintf(stderr, "Start compute_res\n");
#endif
    for (ii = maxres ; ii-- ;) {
#ifdef DEBUG
        fprintf(stderr, "\nname:");
#endif
        fscanf(nfp, "%*d %*d ");

        if (invoked_from_gui())
            fprintf(stderr, "#{GUI}# RESULT ");

        // Read measure name
        while ((cc = getc(nfp)) != '\n') {
// #ifdef DEBUG
//             putc(cc, stderr);
// #endif
            putchar(cc);

            if (invoked_from_gui())
                putc(cc, stderr);
        }
// #ifdef DEBUG
//         fprintf(stderr, "\n\n");
// #endif
        resval = evaluate_res();
        printf(" = %12.12f\n", resval);

        if (invoked_from_gui())
            fprintf(stderr, " = %.16lf\n", resval);
// #ifdef DEBUG
//         fprintf(stderr, " result #%d = %f\n", maxres - ii, resval);
// #endif
    }
#ifdef DEBUG
    fprintf(stderr, "End compute_res\n");
#endif
}


int main(int argc,
         char **argv) {
    register unsigned long ii, jj, kk;
    unsigned short *us_p;
    double mp, imp, place_mean;
    double err;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    // Read other command line arguments
    for (ii = 2; ii < argc; ii++) {
        // Append results to an existing file instead of using the stdout
        if (0 == strcmp(argv[ii], "-append") && ii+1 < argc) {
            freopen(argv[ii + 1], "a+", stdout);
            if (ferror(stdout)) {
                fprintf(stderr, "Could not append to file %s.\n", argv[ii + 1]);
                exit(-1);
            }
            ii++;
        }
        else {
            fprintf(stderr, "Could not understand command line argument %s.\n", argv[ii]);
            exit(1);
        }
    }

    sprintf(netname, "%s", argv[1]);
    init();

    sprintf(filename, "%s.mpd", netname);
    if ((mpdfp = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_double(&mp, mpdfp); err = -mp;
    load_double(&mp, mpdfp);
    // printf("load_distrib %lf %lf\n", err, mp);
    transient = (mp >= 0.0);
#ifdef DEBUG
    fprintf(stderr, "\nerror=%f, time=%f\n\n", err, mp);
#endif
    for (c_m_p = cmarks, ii = toptan ; ii-- ; c_m_p += no_cod) {
        decode_mark(c_m_p, DM);
        load_double(&mp, mpdfp);
        if (transient)
            load_double(&imp, mpdfp);
        else
            imp = mp;
#ifdef DEBUG
        fprintf(stderr, "  #%ld, mp=%f imp=%f\n", toptan - ii, mp, imp);
#endif
        compute_pro(mp, imp, DM);
        for (t_p_p = t_prob, p_b_p = p_bound, us_p = DM , jj = place_num ;
                jj-- ; ++t_p_p, ++p_b_p) {
            kk = *++us_p - p_b_p->min_t;
            (*t_p_p)[kk] += mp;
        }
    }
    (void) fclose(mpdfp);

    // Write tpd file (token probability distributions)
    for (t_p_p = t_prob, p_b_p = p_bound , ii = place_num ;
            ii-- ; ++t_p_p, ++p_b_p) {
        mp = p_b_p->min_t; store_double(&mp, tpdfp);
        mp = p_b_p->max_t; store_double(&mp, tpdfp);
        for (jj = p_b_p->max_t - p_b_p->min_t + 1, r_p = *t_p_p ; jj-- ;)
            store_double(r_p++, tpdfp);
    }
    (void) fclose(tpdfp);

    compute_res();
    (void) fclose(nfp);

    if (invoked_from_gui()) {
        // Write token distribution probabilities to the standard output
        for (t_p_p = t_prob, p_b_p = p_bound , ii = 0 ;
            ii < place_num ; ++t_p_p, ++p_b_p, ++ii) 
        {
            place_mean = 0.0;
            fprintf(stderr, "#{GUI}# RESULT ALL DISTRIB #%ld %d =", ii, p_b_p->max_t + 1);
            for (jj = 0; jj < p_b_p->min_t; jj++)
                fprintf(stderr, " 0");
            r_p = *t_p_p;
            for (jj = p_b_p->min_t; jj <= p_b_p->max_t; jj++, r_p++) {
                place_mean += jj * *r_p;
                fprintf(stderr, " %.16lf", *r_p);
            }
            fprintf(stderr, "\n");
            fprintf(stderr, "#{GUI}# RESULT ALL MEAN #%ld = %.16lf\n", ii, place_mean);
        }
    }


#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
    return 0;
}

