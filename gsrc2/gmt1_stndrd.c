/*
 *  Paris, January 26, 1990
 *  program: gmt_stndrd.c
 *  purpose: Conversion of the TRG of a GSPN into a MC
 *           without net-dependent files compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No general marking dependency is allowed for transition rates.
 *      Only enabling dependency for timed transitions is implemented.
 */


/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif


#define THRESHOLD_ARRAY 32

#include <math.h>
#include "const.h"

#include "grg.h"

void (*terminate)();

group_num, trans_num;

#include <stdio.h>

FILE *nfp, * rgfp, * auxfp, *aefp;
FILE *matfp, * doufp, * marfp;


struct Trans_weight {
    unsigned noecs;
    double rate;
    double *prob;
};

struct ECS_descr {
    unsigned num;
    unsigned *set;
};


/* TRANSITION ARRAY */ struct Trans_weight *t_list;
struct ECS_descr *aecs, *aecs_p;

unsigned lastan, toptan;

char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *       calloc();
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        (*terminate)();
    }
    return (callptr);
}


init() {
    int nm, Naecs, ii, ecs, nt;
    float rw;
    struct Trans_weight *trans_p;
    unsigned *ttt;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif

    fscanf(auxfp, "toptan= %d\n", &toptan);
    fscanf(auxfp, "toptvan= %*d\n");
    fscanf(auxfp, "maxmark= %*d\n");
    fscanf(auxfp, "aecs_conf[%d]=", &group_num);
    aecs = (struct ECS_descr *)ecalloc(group_num + 1, sizeof(struct ECS_descr));
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
    fscanf(nfp, "%d\n", &trans_num);
    t_list = (struct Trans_weight *)ecalloc(trans_num + 1,
                                            sizeof(struct Trans_weight));
    for (trans_p = t_list + 1, nt = 0 ; nt++ < trans_num ; trans_p++) {
        fscanf(nfp, "%d %d %f\n", &ii, &ecs, &rw);
        trans_p->noecs = ecs;
        trans_p->rate = rw;
        trans_p->prob = (double *)ecalloc(aecs[ecs].num + 1, sizeof(double));
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
                trans_p->prob[nm] = trans_p->rate * rw;
#ifdef DEBUG
                fprintf(stderr, "          trans=%d prob=%f\n", nt, trans_p->prob[nm]);
#endif
            }
        }
    }
    free(ttt);

#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}


extern load_compact(/* lp, fp */);
/*
  unsigned long * lp;
  FILE * fp;
*/

double
compute_immpath() {
    unsigned long ul;
    int plength, nt, ecs, aecsn;
    unsigned short *usp;
    double ppp;
    struct Trans_weight *trans_p;

#ifdef DEBUGCALLS
    printf("      Start of compute_immpath\n");
#endif

    ppp = 1.0;
    load_compact(&ul, rgfp); plength = ul;
    while (plength--) {
        load_compact(&ul, rgfp); nt = ul;
        load_compact(&ul, rgfp); aecsn = ul;
#ifdef DEBUG
        printf("          trans %d aecs %d\n", nt, aecsn);
#endif
        if (aecsn) {
            aecs_p = aecs + (ecs = (trans_p = t_list + nt)->noecs);
#ifdef DEBUG
            printf("              in ecs %d\n", ecs);
#endif
            if (((aecs_p->set)[aecsn]) > 1) {
                if (ppp == 1.0)
                    ppp = trans_p->prob[aecsn];
                else
                    ppp *= trans_p->prob[aecsn];
            }
        }
    }
#ifdef DEBUGCALLS
    printf("      End of compute_immpath %f\n", ppp);
#endif
    return (ppp);
}


load_initialmark() {
    unsigned long ul;
    int nm;
    double pp;

#ifdef DEBUGCALLS
    printf("    Start of load_initialmark\n");
#endif
    load_compact(&ul, rgfp);
    if ((nm = ul)) {
        printf("Initial marking is #%d with prob 1.0\n", nm);
    }
    else {
        for (load_compact(&ul, rgfp), nm = ul ; nm ;
                load_compact(&ul, rgfp), nm = ul) {
            pp = compute_immpath();
            printf("Initial marking is #%d with prob %f\n", nm, pp);
        }
    }
#ifdef DEBUGCALLS
    printf("    End of load_initialmark\n");
#endif
}


load_trg() {
    unsigned long ul;
    int no_trans, nt, nm;
    double pp, rr;

#ifdef DEBUGCALLS
    printf("    Start of load_trg\n");
#endif
    printf("\n------------------------\n\n");
    for (load_compact(&ul, rgfp), lastan = ul ; lastan ;
            load_compact(&ul, rgfp), lastan = ul) {
        printf("\nfrom  #%d\n", lastan);
        load_compact(&ul, rgfp); no_trans = ul;
        while (no_trans--) {
            load_compact(&ul, rgfp); nt = ul;
            rr = t_list[nt].rate;
            load_compact(&ul, rgfp);
            if ((nt = ul) > 1)
                rr *= (double)nt;
            load_compact(&ul, rgfp);
            if ((nm = ul)) {
                printf("    (T) to  #%d with rate %f\n", nm, rr);
            }
            else {
                for (load_compact(&ul, rgfp), nm = ul ; nm ;
                        load_compact(&ul, rgfp), nm = ul) {
                    if ((pp = compute_immpath()) == 1.0)
                        pp = rr;
                    else
                        pp *= rr;
                    printf("        (V) to  #%d with rate %f\n", nm, pp);
                }
            }
        }
    }
#ifdef DEBUGCALLS
    printf("    End of load_trg\n");
#endif
}


static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[100];

main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
    extern FILE *fopen();

#ifdef DEBUG
    printf("Start of gmt_stndrd\n");
#endif
    if (0)
        goto end_main;
    terminate = (void (*)())end_main;
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
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
    sprintf(filename, "%s.crgr", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }

    load_initialmark();
    load_trg();

end_main:
    (void) fclose(rgfp);
#ifdef DEBUG
    printf("End of gmt_stndrd\n");
#endif
}


