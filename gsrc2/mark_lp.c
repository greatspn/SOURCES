/*
 *  program: mark_lp.c
 *  date: July 29, 1993
 *  programmer: Giovanni Chiola
 *  address:	Dipartimento di Informatica, Universita' di Torino
 *		corso Svizzera 185, 10149 Torino, Italy.
 *
 *  purpose: produce equations of type M = M0 + C.sigma in "lp_solve"
	     format for a predefined list of marking vectors.
 *  notes:
 *    1) the net description must be in GreatSPN 1.6 format
 */


/*
#define DEBUG
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "const.h"

FILE *nfp, * ofp;

typedef unsigned short ROWT;
#define ROWT_SIZE (unsigned)(sizeof(ROWT))

typedef ROWT *ROWP;
#define ROWP_SIZE (unsigned)(sizeof(ROWP))

typedef ROWP *ARCM;
#define ARCM_SIZE (unsigned)(sizeof(ARCM))

ARCM p_inp, p_out;



int	place_num, trans_num;
static int integ_sol = FALSE;

/* predeclaration */
void getname(char 	*name_pr);
void getarcs(ARCM	p_arc,
             int	noar, int notr,
             char *trans,
             int type);


char *
ecalloc(unsigned nitm, unsigned sz) {
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}

ROWP	mparp;
ROWP	M0;
char **place_nam;
char **trans_nam;
char **mark_nam;
int no_marks;

void load_net() {
    char **cpp;
    unsigned int l;

    ARCM		pp1, pp2;
    ROWP		p_inh, rwp;
    char		linebuf[LINEMAX];
    float		ftemp, x, y;
    int                 group_num, i, j, knd, noar, mark, nomp, norp;
#define VBAR '|'

#ifdef DEBUG
    printf("  Start of load_net\n");
#endif

    /* skip first line containing '|0|' */
    fgets(linebuf, LINEMAX, nfp);

    /* skip comment */
    for (; ;) {
        fgets(linebuf, LINEMAX, nfp);
        if (linebuf[0] == VBAR) break;
    }

    /* read number of objects in the net */

    fscanf(nfp, "f %d %d %d", &nomp, &place_num, &norp);
    fscanf(nfp, "%d %d %d %d\n", &trans_num, &group_num, &i, &j);
    l = trans_num + trans_num + 1;

    mparp = (ROWP)ecalloc(nomp, short_SIZE);
    pp1 = p_inp = (ARCM)ecalloc(place_num, ROWP_SIZE);
    pp2 = p_out = (ARCM)ecalloc(place_num, ROWP_SIZE);
    for (i = place_num ; i-- ; pp1++, pp2++) {
        register ROWP cp;
        *pp1 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
        *pp2 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
    }
    p_inh = (ROWP)ecalloc(l, ROWT_SIZE); *p_inh = 0;

#ifdef DEBUG
    printf("    %d marking parameters\n", nomp);
#endif
    /* read marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        while (i < nomp) {
            getname(linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
            printf("    %d %s=%d\n", i, linebuf, mark);
#endif
            mparp[i++] = (ROWT)mark;
        }
    }

    /* read places */
    cpp = place_nam = (char **)ecalloc(place_num, sizeof(char *));
    rwp = M0 = (ROWP)ecalloc(place_num, short_SIZE);
    for (i = place_num ; i-- ; cpp++, rwp++) {
        getname(linebuf);
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
        if (mark < 0)
            mark = mparp[-1 - mark];
#ifdef DEBUG
        printf("    place %s %d\n", linebuf, mark);
#endif
        *cpp = ecalloc(strlen(linebuf) + 1, 1);
        strcpy(*cpp, linebuf);
        *rwp = (short)mark;
    }

    /* skip rate parameters */
    i = 1;
    while (i++ <= norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp); while (getc(nfp) != '\n');
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        fscanf(nfp, "%f %f", &x, &y); while (getc(nfp) != '\n');
    }

    /* read transitions */
    cpp = trans_nam = (char **)ecalloc(trans_num + 1, sizeof(char *));
    for (i = trans_num, cpp++ ; i-- ; cpp++) {
        getname(linebuf);
        *cpp = ecalloc(strlen(linebuf) + 1, 1);
        strcpy(*cpp, linebuf);
        fscanf(nfp, "%f %d %d %d ", &ftemp, &mark, &knd, &noar);
        while (getc(nfp) != '\n');
        if (mark < 0) {
            int ii;
            ii = mark = -mark;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        getarcs(p_out, noar, trans_num - i, *cpp, 1);
        fscanf(nfp, "%d\n", &noar);
        getarcs(p_inp, noar, trans_num - i, *cpp, 0);
        fscanf(nfp, "%d\n", &noar);
        getarcs(&p_inh, noar, trans_num - i, *cpp, -1);
        *p_inh = 0;
    }
#ifdef DEBUG
    printf("  End of load_net\n");
#endif
}

void getname(char 	*name_pr) {
#define	BLANK ' '
#define	EOLN  '\0'
    short		i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != (char)0 &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}


void getarcs(ARCM	p_arc,
             int	noar, int notr,
             char *trans,
             int type) {
    ROWP	pp;
    int		i, j, pl, mlt, ip;
    float	x, y;

    for (i = noar ; i-- ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while (getc(nfp) != '\n');
        for (j = 1; j++ <= ip; fscanf(nfp, "%f %f\n", &x, &y));
        pp = *(p_arc + (pl - 1));
        while (*pp > 0)
            pp += 2;
        *(pp++) = (ABS(mlt));
        *(pp++) = notr;
        *pp = 0;
        if (integ_sol) {
            if (type > 0)
                fprintf(ofp, "      e_%s <= %g M_%s;\n",
                        trans, 1.0 / ABS(mlt), place_nam[pl - 1]);
            else if (type < 0) {
                fprintf(ofp, "        e_%s <= 10000 h_%s;\n", trans, trans);
                fprintf(ofp, "       h_%s <= %d -M_%s;\n",
                        trans, ABS(mlt), place_nam[pl - 1]);
            }
        }
    }
}

void produce_lp() {
    char **mnp;
    int nmn;
    char **place;
    int npl;
    ARCM pia, poa;
    ROWP m0p;
    ROWP arcp;

    for (place = place_nam, npl = place_num, pia = p_inp, poa = p_out, m0p = M0 ;
            npl-- ; place++, pia++, poa++, m0p++) {
        for (mnp = mark_nam, nmn = no_marks ; nmn-- ; mnp++) {
            int plus = FALSE;
            fprintf(ofp, "    M%s_%s =", *mnp, *place);
            if (*m0p > 0) {
                plus = TRUE;
                if (*m0p > 1 || no_marks == 1)
                    fprintf(ofp, " %d", *m0p);
                if (no_marks > 1)
                    fprintf(ofp, " a_%s", *mnp);
            }
            for (arcp = *pia ; *arcp ; arcp++) {
                int nt;
                if (plus)
                    fprintf(ofp, " +");
                else {
                    fprintf(ofp, " ");
                    plus = TRUE;
                }
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                nt = *++arcp;
                fprintf(ofp, "s%s_%s", *mnp, trans_nam[nt]);
            }
            for (arcp = *poa ; *arcp ; arcp++) {
                int nt;
                fprintf(ofp, " -");
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                nt = *++arcp;
                fprintf(ofp, "s%s_%s", *mnp, trans_nam[nt]);
            }
            fprintf(ofp, ";\n");
        }
        if (no_marks > 1) {
            fprintf(ofp, "  M_%s = M%s_%s", *place, *mark_nam, *place);
            for (mnp = mark_nam + 1, nmn = no_marks ; --nmn ; mnp++) {
                fprintf(ofp, " +M%s_%s", *mnp, *place);
            }
            fprintf(ofp, ";\n");
        }
    }
    if (no_marks > 1) {
        fprintf(ofp, "a_%s", *mark_nam);
        for (mnp = mark_nam + 1, nmn = no_marks ; --nmn ; mnp++) {
            fprintf(ofp, " +a_%s", *mnp);
        }
        fprintf(ofp, " = 1;\n\n");
    }
    else if (integ_sol) {
        fprintf(ofp, "  int M_%s", *place_nam);
        for (place = place_nam + 1, npl = place_num ; --npl ; place++) {
            fprintf(ofp, ", M_%s", *place);
        }
        for (place = trans_nam + 1, npl = trans_num ; npl-- ; place++) {
            fprintf(ofp, ", e_%s", *place);
        }
        fprintf(ofp, ";\n\n");
    }
}


int main(int argc, char **argv) {

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[1000];
    static char *cp = "";

#ifdef DEBUG
    printf("Start\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.lp_mark", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    if (argc == 2 || (integ_sol = !strcmp(argv[2], "-i"))) {
        mark_nam = &cp;
        no_marks = 1;
    }
    else {
        no_marks = argc - 2;
        mark_nam = argv + 2;
    }
    load_net();
    (void) fclose(nfp);
    produce_lp();
    (void) fclose(ofp);
#ifdef DEBUG
    printf("End\n");
#endif
    return 0;
}

