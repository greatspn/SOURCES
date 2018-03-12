/*
 *  Torino, April 3, 1991
 *  program: liveness.c
 *  purpose: Compute transition enabling and liveness bounds of a GSPN
 *  programmer: Giovanni Chiola
 *  notes:
 *  1) takes the TRG and its associated livelock description as input
 *  2) computes actual bounds only for infinite-server (timed) transitions.
 */

#include <stdlib.h>

/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif



#include "const.h"
#include "../WN/INCLUDE/compact.h"
#include <stdio.h>

FILE *rgfp, * auxfp, * nfp, *lvlkfp, *outfp;


int toptan;
int trans_num;
size_t **t_bounds;

/* predeclaration */
void skip_initialmark();

char *
ecalloc(unsigned nitm,  unsigned sz) {
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(10);
    }
    return (callptr);
}


int transient, nolivelocks, nodeadlocks, norec, M0homestate;
unsigned char *is_live;


void init() {
    unsigned long ul;
    size_t *usp;
    size_t **uspp;
    int ii;


#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif

    fscanf(auxfp, "toptan= %d\n", &toptan);
    fscanf(nfp, "%*d %*d %*d %d\n", &trans_num);

    t_bounds = (size_t **)ecalloc(trans_num + 1, sizeof(size_t *));
    skip_initialmark();

    load_compact(&ul, lvlkfp); nolivelocks = ul;
    load_compact(&ul, lvlkfp); nodeadlocks = ul;
#ifdef DEBUG
    fprintf(stderr, "%d livelocks, %d deadlocks\n", nolivelocks, nodeadlocks);
#endif
    norec = nolivelocks + nodeadlocks;
    is_live = (unsigned char *)ecalloc(norec + 1, sizeof(unsigned char));
    if (nolivelocks && nodeadlocks) {
        for (ii = nolivelocks ; ii-- ;) {
            load_compact(&ul, lvlkfp);
#ifdef DEBUG
            fprintf(stderr, "     ul=%d\n", ul);
#endif
            is_live[(int)ul] = 1;
        }
#ifdef DEBUG
        fprintf(stderr, " livelocks read\n");
#endif
        for (ii = nodeadlocks ; ii-- ;) {
            load_compact(&ul, lvlkfp);
#ifdef DEBUG
            fprintf(stderr, "     ul=%d\n", ul);
#endif
            is_live[(int)ul] = 0;
        }
#ifdef DEBUG
        fprintf(stderr, " deadlocks read\n");
#endif
    }
    else {
        unsigned char live = (nolivelocks > 0);
        for (ii = 0 ; ii++ < norec ; is_live[ii] = live);
    }
    load_compact(&ul, lvlkfp); transient = ul;
    M0homestate = (nolivelocks == 1 && ! nodeadlocks && ! transient);
#ifdef DEBUG
    fprintf(stderr, "   %d markings non recurrent, M0homestate=%d\n",
            transient, M0homestate);
#endif
    for (ul = trans_num, uspp = t_bounds ; ul-- ;) {
        usp = (size_t *)ecalloc(norec + 1,
                                sizeof(size_t));
        *(usp) = (size_t)(++uspp);
        *uspp = usp;
        for (ii = norec ; ii-- ;)
            *(++usp) = 0;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}


void check_immpath(int nolvlck,
                   int is_M0) {
    register size_t *usp;
    unsigned long ul;
    int plength, nt;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of check_immpath %d\n", nolvlck);
#endif

    load_compact(&ul, rgfp); plength = ul;
    while (plength--) {
        load_compact(&ul, rgfp); nt = ul;
        load_compact(&ul, rgfp);
#ifdef DEBUG
        fprintf(stderr, "          trans %d aecs %d\n", nt, ul);
#endif
        if (is_M0) {
            if (!(usp = (t_bounds[nt]))) {
                usp = (size_t *)1;
            }
        }
        else {
            if (! *(usp = (t_bounds[nt] + nolvlck))) {
                *usp = 1;
#ifdef DEBUG
                fprintf(stderr, "            setting t_bounds[%d][%d]\n", nt, nolvlck);
#endif
            }
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of check_immpath\n");
#endif
}


void skip_initialmark() {
    unsigned long ul;
    int nm;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of skip_initialmark\n");
#endif
    load_compact(&ul, rgfp);
    for (nm = 0 ; nm++ < trans_num ; t_bounds[nm] = (size_t *)0);
    if ((nm = ul)) {
#ifdef DEBUG
        fprintf(stderr, "      M1 with prob. 1.0\n");
#endif
    }
    else {
        for (load_compact(&ul, rgfp), nm = ul ; nm ;
                load_compact(&ul, rgfp), nm = ul) {
            check_immpath(0, TRUE);
#ifdef DEBUG
            fprintf(stderr, "      M%d with prob. >0\n", nm);
#endif
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of skip_initialmark\n");
#endif
}


void load_trg() {
    register size_t *usp;
    unsigned long ul;
    int no_trans, nt, nm, nolvlck, lastan;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of load_trg\n");
#endif
    for (load_compact(&ul, rgfp), lastan = ul ; lastan ;
            load_compact(&ul, rgfp), lastan = ul) {
        if (M0homestate)
            nolvlck = 1;
        else {
            load_compact(&ul, lvlkfp); nolvlck = ul;
        }
        load_compact(&ul, rgfp); no_trans = ul;
#ifdef DEBUG
        fprintf(stderr, "\nfrom  #%d  (%d timed trans)\n", lastan, no_trans);
#endif
        while (no_trans--) {
            load_compact(&ul, rgfp); nt = ul;
            load_compact(&ul, rgfp);
            if (*(usp = (t_bounds[nt] + nolvlck)) < ul) {
#ifdef DEBUG
                fprintf(stderr, "            setting t_bounds[%d][%d] = %d\n", nt, nolvlck, ul);
#endif
                *usp = ul;
            }
#ifdef DEBUG
            fprintf(stderr, "  timed #%d  (enabl=%d)\n", nt, ul);
#endif
            load_compact(&ul, rgfp);
            if ((nm = ul)) {
#ifdef DEBUG
                fprintf(stderr, "    (T) to  #%d\n", nm);
#endif
            }
            else {
                for (load_compact(&ul, rgfp), nm = ul ; nm ;
                        load_compact(&ul, rgfp), nm = ul) {
                    check_immpath(nolvlck, FALSE);
#ifdef DEBUG
                    fprintf(stderr, "        (V) to  #%d\n", nm);
#endif
                }
            }
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of load_trg\n");
#endif
}


void store_liveness() {
    int ii, jj, max, min;
    size_t **uspp;
    size_t *usp;
    int livenet = TRUE;

    fprintf(outfp, "%d", trans_num);
    if (M0homestate) {
        fprintf(outfp, " 0\n");
        for (uspp = t_bounds + 1, ii = trans_num ; ii-- ; ++uspp) {
            if (!(*uspp)[1]) {
                fprintf(stderr, "\n***   transition #%d IS DEAD !!!\n",
                        trans_num - ii);
                livenet = FALSE;
            }
            fprintf(outfp, "%zu\n", (*uspp)[1]);
        }
        if (livenet)
            fprintf(stderr, "\nThe net is live\n");
        return;
    }
    fprintf(outfp, " %d\n", norec + 1);
    for (uspp = t_bounds + 1, ii = trans_num ; ii-- ; ++uspp) {
        max = *(usp = *uspp);
        if ((min = *(++usp)) > max)
            max = min;
        for (jj = norec ; --jj ;) {
            if (*(++usp) > max)
                max = *usp;
            else if (*usp < min)
                min = *usp;
        }
        if (! max) {
            fprintf(stderr, "\n***   transition #%d IS DEAD !!!\n",
                    trans_num - ii);
            livenet = FALSE;
        }
        else if (! min) {
            fprintf(stderr, "\n***   transition #%d IS NOT LIVE !!!\n",
                    trans_num - ii);
            livenet = FALSE;
        }
        fprintf(outfp, " %d %d", max, min);
        for (usp = *uspp, jj = norec + 1 ; jj-- ; ++usp) {
            if (jj)
                fprintf(outfp, " %zu", *usp);
            else
                fprintf(outfp, " %zu\n", *usp);
        }
    }
    if (livenet)
        fprintf(stderr, "\nThe net is live\n");
}


static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[100];


int main(int argc,
         char **argv,
         char **envp) {

#ifdef DEBUG
    fprintf(stderr, "Start of liveness\n");
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
    sprintf(filename, "%s.grg", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.crgr", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((auxfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.livlck", argv[1]);
    if ((lvlkfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init();
    (void)fclose(nfp);
    (void)fclose(auxfp);

    load_trg();
    (void) fclose(lvlkfp);
    (void) fclose(rgfp);

    sprintf(filename, "%s.liveness", argv[1]);
    if ((outfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    store_liveness();
    (void)fclose(outfp);

#ifdef DEBUG
    fprintf(stderr, "End of liveness\n");
#endif
    return 0;
}

