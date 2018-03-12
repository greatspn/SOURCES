/*
 *  Paris, February 13, 1990
 *  program: throughput.c
 *  purpose: Compute transition throughputs of a GSPN
 *  programmer: Giovanni Chiola
 *  notes:
 *  1) takes as input the TRG and its associated probability distribution
 *  2) No marking dependency is allowed for immediate transition weights.
 */


/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "const.h"
#include "../WN/INCLUDE/compact.h"


#include "grg.h"

int group_num, trans_num;

#include <stdio.h>

FILE *nfp, * rgfp, * auxfp, *aefp, *trsfp;
FILE *marfp;


int load_trs, transient, no_cod;

unsigned char *CMp;
unsigned short *DMp;


struct Cond_def {
    char op;
    char type;
    char top2;
    int p1;
    int op2;
};


union Val_type {
    double real;
    unsigned place;
};


struct Val_def {
    char op;
    char type;
    union Val_type val;
};


struct Clause_def {
    char type;
    int nocond;
    int noval;
    struct Val_def *vals;
    struct Cond_def *conds;
};


union MD_prob {
    double *prob;
    struct Clause_def *mdc;
};


struct Trans_weight {
    unsigned short noecs;
    unsigned short endep;
    double rate;
    union MD_prob md_p;
};


struct ECS_descr {
    unsigned num;
    unsigned *set;
};


/* TRANSITION ARRAY */ struct Trans_weight *t_list;
struct ECS_descr *aecs, *aecs_p;
double *t_t;
char **t_names;

unsigned lastan, toptan;

char *ecalloc(unsigned nitm, unsigned  sz) {
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(10);
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

unsigned long tmp;
unsigned nel;


void init() {
    int nm, Naecs, ii, ecs, nt, jj, pri;
    float rw;
    struct Trans_weight *trans_p;
    unsigned *ttt;
    double *d_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif

    fscanf(auxfp, "toptan= %d\n", &toptan);
    fscanf(auxfp, "topvan= %*d\n");
    fscanf(auxfp, "maxmark= %*d\n");
    fscanf(auxfp, "aecs_conf[%d]=", &group_num);
    aecs = (struct ECS_descr *)ecalloc(group_num + 1, sizeof(struct ECS_descr));
    aecs->num = 0;
    for (aecs_p = aecs + 1, Naecs = 0, nm = group_num ; nm-- ; aecs_p++) {
        fscanf(auxfp, " %d", &nt);
        aecs_p->num = nt;
        aecs_p->set = (unsigned *)ecalloc(nt + 1, sizeof(unsigned));
#ifdef DEBUG
        fprintf(stderr, "      ECS #%d  contains %d AECS\n", group_num - nm, nt);
#endif
        Naecs += nt;
    }

    /* read transitions */
    load_trs = FALSE;
    fscanf(nfp, "%d\n", &trans_num);
    t_list = (struct Trans_weight *)ecalloc(trans_num + 1,
                                            sizeof(struct Trans_weight));
    t_t = (double *)ecalloc(trans_num + 1, sizeof(double));
    t_names = (char **)ecalloc(trans_num + 1, sizeof(char *));
    for (d_p = t_t, ii = trans_num ; ii-- ; * (++d_p) = 0.0);
    for (trans_p = t_list + 1, nt = 0 ; nt++ < trans_num ; trans_p++) {
        fscanf(nfp, "%d %d %d %f %d", &ii, &ecs, &pri, &rw, &jj);
#ifdef DEBUG
        fprintf(stderr, "    reading transition %d\n", ii);
#endif
        if (jj) {
            struct Clause_def *clp;
            struct Cond_def *cop;
            struct Val_def *vap;
            char cc;
            int nco, nva, nn;
            double dd;

#ifdef DEBUG
            fprintf(stderr, "        m-d with %d clauses\n", jj);
#endif
            if (jj < 0) {
                fprintf(stderr, "ERROR: no general distribution allowed!\n");
                exit(1);
            }
            while ((cc = getc(nfp)) != '\n');
            load_trs = TRUE;
            trans_p->md_p.mdc = clp = (struct Clause_def *)ecalloc(jj,
                                      sizeof(struct Clause_def));
            while (jj--) {
                while ((cc = getc(nfp)) == ' ');
                fscanf(nfp, "%d %d\n", &nco, &nva);
#ifdef DEBUG
                fprintf(stderr, "          type=%c nco=%d nva=%d\n", cc, nco, nva);
#endif
                clp->type = cc;
                clp->nocond = nco;
                clp->noval = nva;
                clp->conds = cop = (struct Cond_def *)ecalloc(nco,
                                   sizeof(struct Cond_def));
                clp->vals = vap = (struct Val_def *)ecalloc(nva,
                                  sizeof(struct Val_def));
                while (nco--) {
                    while ((cc = getc(nfp)) == ' ');
                    if (cc == 'c') {
                        cop->op = cc;
                        while ((cc = getc(nfp)) == ' ');
                        cop->type = cc;
                        fscanf(nfp, "%d", &nn);
                        cop->p1 = nn;
                        while ((cc = getc(nfp)) == ' ');
                        cop->top2 = cc;
                        fscanf(nfp, "%d\n", &nn);
                        cop->op2 = nn;
#ifdef DEBUG
                        fprintf(stderr, "              c type=%c p1=%d top2=%c op2=%d\n", cop->type,
                                cop->p1, cop->top2, cop->op2);
#endif
                    }
                    else {
#ifdef DEBUG
                        fprintf(stderr, "              %c\n", cc);
#endif
                        cop->op = cc;
                        while (getc(nfp) != '\n');
                    }
                    ++cop;
                }
                while (nva--) {
                    while ((cc = getc(nfp)) == ' ');
                    if (cc == 'o') {
                        vap->op = cc;
                        while ((cc = getc(nfp)) == ' ');
                        if ((vap->type = cc) == 'p') {
                            fscanf(nfp, "%d\n", &nn);
                            vap->val.place = nn;
#ifdef DEBUG
                            fprintf(stderr, "              o p %d\n", nn);
#endif
                        }
                        else {
                            fscanf(nfp, "%lf\n", &dd);
                            vap->val.real = dd;
#ifdef DEBUG
                            fprintf(stderr, "              o r %lf\n", dd);
#endif
                        }
                    }
                    else {
                        vap->op = cc;
#ifdef DEBUG
                        fprintf(stderr, "              %c\n", cc);
#endif
                        while (getc(nfp) != '\n');
                    }
                    ++vap;
                }
                ++clp;
            }
        }
        fscanf(nfp, " %d", &jj);
#ifdef DEBUG
        fprintf(stderr, "        ecs=%d endep=%d rate=%f\n", ecs, jj, rw);
#endif
        trans_p->noecs = ecs;
        trans_p->endep = jj;
        trans_p->rate = rw;
        if (ecs) {
            trans_p->md_p.prob = (double *)ecalloc(aecs[ecs].num + 1,
                                                   sizeof(double));
        }
        else if (jj) {
            double *d_p;
            trans_p->md_p.prob = d_p = (double *)ecalloc(jj + 2,
                                       sizeof(double));
            *(++d_p) = rw;
            while (jj--) {
                fscanf(nfp, "%f", &rw);
                *(++d_p) = rw;
            }
        }
        while (getc(nfp) != '\n');
    }

    ttt = (unsigned *)ecalloc(trans_num + 1, sizeof(unsigned));
    /* read aecs */
    while (Naecs--) {
        fscanf(aefp, "%d %d %d", &ecs, &nm, &ii);
#ifdef DEBUG
        fprintf(stderr, "      ECS=%d AECS=%d nt=%d\n", ecs, nm, ii);
#endif
        aecs[ecs].set[nm] = ii;
        if (ii == 1)
            fscanf(aefp, "%*d\n");
        else {
            ttt[ii] = 0; rw = 0.0;
            while (ii--) {
                fscanf(aefp, "%d", &nt);
                ttt[ii] = nt;
                rw += t_list[nt].rate;
            }
            while (getc(aefp) != '\n');
            rw = 1.0 / rw;
            for (ii = 0; (nt = ttt[ii++]) ;) {
                trans_p = t_list + nt;
                trans_p->md_p.prob[nm] = trans_p->rate * rw;
#ifdef DEBUG
                fprintf(stderr, "          trans=%d prob=%f\n", nt, trans_p->md_p.prob[nm]);
#endif
            }
        }
    }
    free(ttt);

#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}

int place_num;


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


struct Byte_descr {
    unsigned char noitems;
    unsigned char max_conf;
    unsigned short first_item;
};

struct Byte_descr *encoding;


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


void init_markings() {
    int sub_num, group_num;
    int ii, nn, nt, jj, jj2, kk, ll, mm, np, n1, n2, n3, n4;
    struct Byte_descr *byte_p;
    struct Place_descr *place_p;
    struct Codbyt_descr *cods_p;
    char buf[256];

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init_markings\n");
#endif

    /* read number of objects in the net */
    fscanf(nfp, "%d %d %d", &sub_num, &place_num, &group_num);
    while (getc(nfp) != '\n');

    DMp = (unsigned short *)ecalloc(place_num + 1, sizeof(unsigned short));
    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));

    /* read codings */
    for (nn = 0 ; nn < sub_num ; nn++) {
        fscanf(nfp, "%d %d\n", &ii, &jj2);
#ifdef DEBUG
        fprintf(stderr, "  subnet #%d (%d bytes):\n",
                nn, jj2);
#endif
        if (nn == 0) {
            encoding = (struct Byte_descr *)ecalloc(jj2, sizeof(struct Byte_descr));
            CMp = (unsigned char *)ecalloc(jj2, sizeof(unsigned char));
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

    /* skip groups */
    for (ii = 0 ; ii++ < group_num;) {
        fscanf(nfp, "%*d %d\n", &kk);
        for (jj = 0 ; ++jj <= kk ;) {
            fscanf(nfp, " %*d %d", &n2);
            while (getc(nfp) != '\n')
            {}
            for (ll = 0 ; ll++ < n2 ;) {
                while (getc(nfp) != '\n');
            }
        }
    }

    /* read trans names */
    for (ii = 0 ; ii++ < trans_num;) {
        fscanf(nfp, "%d %*d %*d %*d %*d %d %d %s\n", &nt, &n1, &n2, buf);
        t_names[nt] = ecalloc(strlen(buf) + 1, 1);
        sprintf(t_names[nt], "%s", buf);
        while (n1--) {
            fscanf(nfp, " %*d %d %d\n", &n3, &n4);
            for (n3 += n4 ; n3-- ;)
                while (getc(nfp) != '\n');
        }
        for (n2 += 2 ; n2-- ;)
            while (getc(nfp) != '\n');
    }


#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init_markings\n");
#endif
}


struct T_list {
    int tran;
    struct T_list *next;
};

static struct T_list *first = NULL, * tfree = NULL, * last = NULL;


void insert_trans(int nt) {
    struct T_list *new;

    if (tfree == NULL) {
        new = (struct T_list *)ecalloc(1, sizeof(struct T_list));
    }
    else {
        new = tfree; tfree = tfree->next;
        if (tfree == NULL)
            last = NULL;
    }
    new->tran = nt;
    new->next = first;
    if (first == NULL)
        last = new;
    first = new;
}


void free_list() {
    if (last != NULL) {
        last->next = tfree;
        tfree = first;
    }
    last = first = NULL;
}


double
compute_immpath() {
    unsigned long ul;
    int plength, nt, ecs, aecsn;
    double ppp;
    struct Trans_weight *trans_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of compute_immpath\n");
#endif

    ppp = 1.0;
    load_compact(&ul, rgfp); plength = ul;
    while (plength--) {
        load_compact(&ul, rgfp); nt = ul;
        insert_trans(nt);
        load_compact(&ul, rgfp); aecsn = ul;
#ifdef DEBUG
        fprintf(stderr, "          trans %d aecs %d\n", nt, aecsn);
#endif
        if (aecsn) {
            aecs_p = aecs + (ecs = (trans_p = t_list + nt)->noecs);
#ifdef DEBUG
            fprintf(stderr, "              in ecs %d\n", ecs);
#endif
            if (((aecs_p->set)[aecsn]) > 1) {
                if (ppp == 1.0)
                    ppp = trans_p->md_p.prob[aecsn];
                else
                    ppp *= trans_p->md_p.prob[aecsn];
            }
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of compute_immpath %f\n", ppp);
#endif
    return (ppp);
}


void skip_initialmark() {
    unsigned long ul;
    int nm;
    double pp;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of skip_initialmark\n");
#endif
    load_compact(&ul, rgfp);
    if ((nm = ul)) {
#ifdef DEBUG
        fprintf(stderr, "      M1 with prob. 1.0\n");
#endif
    }
    else {
        for (load_compact(&ul, rgfp), nm = ul ; nm ;
                load_compact(&ul, rgfp), nm = ul) {
            pp = compute_immpath();
            free_list();
#ifdef DEBUG
            fprintf(stderr, "      M%d with prob. %f\n", nm, pp);
#endif
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of skip_initialmark\n");
#endif
}


static struct Cond_def *cond_stack;


int
evaluate_cond(unsigned short *Mp) {
    int val1, val2;
    char op = (++cond_stack)->op;

    if (cond_stack->op == 'c') {
        val1 = Mp[cond_stack->p1];
        val2 = cond_stack->op2;
        if (cond_stack->top2 == 'p')
            val2 = Mp[val2];
        switch (cond_stack->type) {
        case '=' : return (val1 == val2);
        case 'n' : return (val1 != val2);
        case 'g' : return (val1 >= val2);
        case '>' : return (val1 > val2);
        case 'l' : return (val1 <= val2);
        case '<' : return (val1 < val2);
        }
    }

    val1 = evaluate_cond(Mp);
    if (op == '~')
        return (! val1);
    val2 = evaluate_cond(Mp);
    if (op == '&')
        return (val1 && val2);
    return (val1 || val2);
}


static struct Val_def *val_stack;


double
evaluate_val(unsigned short *Mp) {
    struct Val_def *vap = val_stack++;
    double val1, val2;

    if (vap->op == 'o') {
        if (vap->type == 'p')
            return ((double)(Mp[vap->val.place]));
        return (vap->val.real);
    }
    val2 = evaluate_val(Mp);
    val1 = evaluate_val(Mp);
    switch (vap->op) {
    case '+' : return (val1 + val2);
    case '-' : return (val1 - val2);
    case '*' : return (val1 * val2);
    case '/' : return (val1 / val2);
    default : return 0;
    }
}


double
md_rate(struct Clause_def *mdc, unsigned short *Mp) {
    int cond = (mdc->type == 'e');
    int val1, val2;

    while (! cond) {
        struct Cond_def *cop = mdc->conds;
        if (cop->op == 'c') {
            val1 = Mp[cop->p1];
            val2 = cop->op2;
            if (cop->top2 == 'p')
                val2 = Mp[val2];
            switch (cop->type) {
            case '=' : cond = (val1 == val2);
                break;
            case 'n' : cond = (val1 != val2);
                break;
            case 'g' : cond = (val1 >= val2);
                break;
            case '>' : cond = (val1 > val2);
                break;
            case 'l' : cond = (val1 <= val2);
                break;
            case '<' : cond = (val1 < val2);
                break;
            }
        }
        else {
            cond_stack = cop;
            val1 = evaluate_cond(Mp);
            if (cop->op == '~')
                cond = ! val1;
            else if (cop->op == '|' && val1)
                cond = TRUE;
            else if (cop->op == '&' && ! val1)
                cond = FALSE;
            else {
                cond = evaluate_cond(Mp);
            }
        }
        if (! cond) {
            cond = ((++mdc)->type == 'e');
        }
    }
    val_stack = mdc->vals;
    return (evaluate_val(Mp));

}


void load_trg() {
    unsigned long ul;
    int no_trans, nt, nm;
    double pp, rr, prob;
    int next_print;
    struct Clause_def *mdc;
    int decoded = 0;
    struct T_list *tt;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of load_trg\n");
#endif
    for (load_compact(&ul, rgfp), lastan = ul, next_print = 0 ; lastan ;
            load_compact(&ul, rgfp), lastan = ul) {
        load_compact(&ul, rgfp); no_trans = ul;
        load_double(&prob, marfp);
        if (transient)
            load_double(&pp, marfp);
#ifdef DEBUG
        fprintf(stderr, "\nfrom  #%d  (%d timed trans) prob=%f\n",
                lastan, no_trans, prob);
#endif
        if (load_trs) {
            int ii = no_cod;
            unsigned char *cp = CMp;

            decoded = FALSE;
            while (ii--)
                *(cp++) = getc(trsfp);
        }
        while (no_trans--) {
            struct Trans_weight *t_p;

            load_compact(&ul, rgfp); nt = ul;
            load_compact(&ul, rgfp); nm = ul;
            t_p = &(t_list[nt]);
            if (nm > 1) {
                if (t_p->endep) {
                    rr = t_p->md_p.prob[nm];
#ifdef DEBUG
                    fprintf(stderr, "      timed #%d  is enabling-dependent %d, rate=%f\n",
                            nt, nm, rr);
#endif
                }
                else {
                    rr = t_p->rate;
#ifdef DEBUG
                    fprintf(stderr, "          t_p=%d t_p->rate=%f rr=%f\n",
                            (int)t_p, t_p->rate, rr);
#endif
                    rr *= (double)nm;
#ifdef DEBUG
                    fprintf(stderr, "      timed #%d  is multiple-server %d, rate=%f\n",
                            nt, nm, rr);
#endif
                }
            }
            else if (!(t_p->endep) && ((mdc = t_p->md_p.mdc) != NULL)) {
                if (! decoded) {
                    decoded = TRUE;
                    decode_mark(CMp, DMp);
                }
                rr = md_rate(mdc, DMp);
#ifdef DEBUG
                fprintf(stderr, "      timed #%d  is marking-dependent %d %f\n", nt, nm, rr);
#endif
            }
            else {
                rr = t_list[nt].rate;
#ifdef DEBUG
                fprintf(stderr, "      timed #%d  is single-server %d, rate=%f\n",
                        nt, nm, rr);
#endif
            }
            rr *= prob;
            t_t[nt] += rr;
            load_compact(&ul, rgfp);
            if (!(nm = ul)) {
                for (load_compact(&ul, rgfp), nm = ul ; nm ;
                        load_compact(&ul, rgfp), nm = ul) {
                    if ((pp = compute_immpath()) == 1.0)
                        pp = rr;
                    else
                        pp *= rr;
#ifdef DEBUG
                    fprintf(stderr, "        (V) to  #%d with rate %f\n", nm, pp);
#endif
                    for (tt = first ; tt != NULL ; tt = tt->next) {
                        nt = tt->tran;
                        t_t[nt] += pp;
                    }
                    free_list();
                }
            }
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of load_trg\n");
#endif
}


void store_throughputs() {
    int ii;
    double *d_p;

    for (d_p = t_t + 1, ii = trans_num ; ii-- ; ++d_p) {
        printf("Thru_%s = %f\n", t_names[trans_num - ii], *d_p);
        if (invoked_from_gui())
            fprintf(stderr, "#{GUI}# RESULT ALL THROUGHPUT %s %12.16lf\n", 
                    t_names[trans_num - ii], *d_p);

    }
}


static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[100];


int main(int argc, char **argv, char **envp) {
    double dd;

#ifdef DEBUG
    fprintf(stderr, "Start of throughputs\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii--) {
            s_p = *(a_p++);
            if (*s_p != '-' || *(s_p + 1) != 'e') {
                fprintf(stderr, "ERROR: unknown parameter '%s'\n", s_p);
                exit(33);
            }
        }
    }
    sprintf(filename, "%s.gmt", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((auxfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.aecs", argv[1]);
    if ((aefp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init();
    (void)fclose(nfp);
    (void)fclose(auxfp);
    (void)fclose(aefp);
    sprintf(filename, "%s.grg", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init_markings();
    (void)fclose(nfp);
    if (load_trs) {
        sprintf(filename, "%s.ctrs", argv[1]);
        if ((trsfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
    }
    sprintf(filename, "%s.crgr", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }

    skip_initialmark();
    sprintf(filename, "%s.mpd", argv[1]);
    if ((marfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_double(&dd, marfp);
    transient = (dd > 0.0);
    load_double(&dd, marfp);

    load_trg();
    (void) fclose(rgfp);
    (void) fclose(marfp);
    if (load_trs)
        (void) fclose(trsfp);

// end_main:
    store_throughputs();

#ifdef DEBUG
    fprintf(stderr, "End of throughputs\n");
#endif
    return 0;
}

