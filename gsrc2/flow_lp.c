/*
 *  program: flow_lp.c
 *  date: July 30, 1993
 *  programmer: Giovanni Chiola
 *  address:	Dipartimento di Informatica, Universita' di Torino
 *		corso Svizzera 185, 10149 Torino, Italy.
 *
 *  purpose: produce equations of type "forall p, sum_{t in *p}
	     x_t W(t,p) >= sum_{t in p*} x_t W(p,t)"
	     in "lp_solve" format.
 *  notes:
 *    1) the net description must be in GreatSPN 1.6 format
 */


/*
#define DEBUG
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "const.h"

FILE *nfp, * ofp;

typedef unsigned short ROWT;
#define ROWT_SIZE (unsigned)(sizeof(ROWT))

typedef ROWT *ROWP;
#define ROWP_SIZE (unsigned)(sizeof(ROWP))

typedef ROWP *ARCM;
#define ARCM_SIZE (unsigned)(sizeof(ARCM))

char **place_nam;
ARCM p_inp, p_out;

float *rpars;

char **trans_nam;
ROWP t_no_inp;
float *ftimes;


unsigned place_num, trans_num;

/* predeclaration */
void getname(char 	*name_pr);
void getarcs(ARCM	p_arc,
             int	noar, int notr, int no_serv,
             float S,
             char *t_nam);


char *ecalloc(unsigned nitm, unsigned sz) {
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}


static int race_policy = TRUE;

void load_net() {
    char **cpp;
    unsigned int l;

    ARCM		pp1, pp2;
    ROWP		p_inh, rwp;
    char		linebuf[LINEMAX];
    float		ftemp, x, y;
    float *pf;
    int                 group_num, i, j, knd, noar, mark, nomp, norp;
#define VBAR '|'

#ifdef DEBUG
    fprintf(stderr, "  Start of load_net\n");
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

    pp1 = p_inp = (ARCM)ecalloc(place_num, ROWP_SIZE);
    pp2 = p_out = (ARCM)ecalloc(place_num, ROWP_SIZE);
    for (i = place_num ; i-- ; pp1++, pp2++) {
        register ROWP cp;
        *pp1 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
        *pp2 = cp = (ROWP)ecalloc(l, ROWT_SIZE); *cp = 0;
    }
    p_inh = (ROWP)ecalloc(l, ROWT_SIZE); *p_inh = 0;

#ifdef DEBUG
    fprintf(stderr, "    %d marking parameters\n", nomp);
#endif
    /* skip marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        while (i++ < nomp) {
            getname(linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
            fprintf(stderr, "    %d %s=%d\n", i, linebuf, mark);
#endif
        }
    }

    /* read places */
    cpp = place_nam = (char **)ecalloc(place_num, sizeof(char *));
    for (i = place_num ; i-- ; cpp++) {
        getname(linebuf);
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
        fprintf(stderr, "    place %s %d\n", linebuf, mark);
#endif
        *cpp = ecalloc(strlen(linebuf) + 1, 1);
        strcpy(*cpp, linebuf);
    }

    /* read rate parameters */
    pf = rpars = (float *)ecalloc(norp, sizeof(float));
    i = 1;
    while (i++ <= norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp); while (getc(nfp) != '\n');
        *(pf++) = ftemp;
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        fscanf(nfp, "%f %f", &x, &y); while (getc(nfp) != '\n');
    }

    /* read transitions */
    l -= trans_num;
    cpp = trans_nam = (char **)ecalloc(l, sizeof(char *));
    pf = ftimes = (float *)ecalloc(l, sizeof(float));
    rwp = t_no_inp = (ROWP)ecalloc(l, ROWT_SIZE);
    for (i = trans_num, cpp++, pf++, rwp++ ; i-- ; cpp++, pf++, rwp++) {
        int nrp = 0;
        getname(linebuf);
        *cpp = ecalloc(strlen(linebuf) + 1, 1);
        strcpy(*cpp, linebuf);
        fscanf(nfp, "%f %d %d %d ", &ftemp, &mark, &knd, &noar);
        while (getc(nfp) != '\n');
        if ((ftemp < 0.0 && (nrp = (int)(-ftemp)) > norp) || mark < 0) {
            fprintf(stderr, "Sorry, no marking dependency allowed (transition %s)\n",
                    linebuf);
            exit(1);
        }
        if (nrp)
            ftemp = rpars[nrp - 1];
        if (knd == 0)
            *pf = 1.0 / ftemp;
        else if (knd == DETERM)
            *pf = ftemp;
        else
            *pf = 0.0;
        if (mark)
            *rwp = noar + 1;
        else
            *rwp = noar;
        if (*pf > 0.0 && mark)
            fprintf(ofp, "   %g >= x_%s;\n", mark / (*pf), linebuf);
        getarcs(p_out, noar, trans_num - i, *pf, mark, linebuf);
        fscanf(nfp, "%d\n", &noar);
        getarcs(p_inp, noar, trans_num - i, -1.0, 0, linebuf);
        fscanf(nfp, "%d\n", &noar);
        if (noar) {
            *rwp += noar;
            getarcs(&p_inh, noar, trans_num - i, -1.0, 0, linebuf);
            *p_inh = 0;
        }
    }
    fprintf(ofp, "\n");
#ifdef DEBUG
    fprintf(stderr, "  End of load_net\n");
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
             int	noar, int notr, int no_serv,
             float S,
             char *t_nam) {
    ROWP	pp;
    int		i, j = 0, pl, mlt, ip;
    float	x, y;
    char compar[3];
    char *p_nam;

#ifdef DEBUG
    fprintf(stderr, "    Start of getarcs\n");
#endif
    for (i = noar ; i-- ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        pl--;
        p_nam = place_nam[pl];
        if (S > 0.0 && race_policy) {
            float coef;
            if (noar == 1 && no_serv == 0) {
                if ((j = ABS(mlt)) > 1) {
                    fprintf(ofp, "    M_%s <= %d +", p_nam, j - 1);
                    if ((coef = j * S) != 1.0)
                        fprintf(ofp, "%g ", coef);
                    fprintf(ofp, "x_%s;\n", t_nam);
                    sprintf(compar, ">=");
                }
                else
                    sprintf(compar, "=");
            }
            else
                sprintf(compar, ">=");
            fprintf(ofp, "    M_%s %s ", p_nam, compar);
            if ((coef = j * S) != 1.0)
                fprintf(ofp, "%g ", coef);
            fprintf(ofp, "x_%s;\n", t_nam);
        }
        else if (S == 0.0 && noar == 1)
            fprintf(ofp, "    M_%s <= %d;\n", p_nam, ABS(mlt) - 1);
        while (getc(nfp) != '\n');
        for (j = 1; j++ <= ip; fscanf(nfp, "%f %f\n", &x, &y));
        pp = *(p_arc + pl);
        while (*pp > 0)
            pp += 2;
        *(pp++) = (ABS(mlt));
        *(pp++) = notr;
        *pp = 0;
    }
#ifdef DEBUG
    fprintf(stderr, "    End of getarcs\n");
#endif
}


void produce_lp() {
    int npl;
    ARCM pia, poa;
    ROWP arcp;
    char **place;

#ifdef DEBUG
    fprintf(stderr, "  Start of produce_lp\n");
#endif
    for (place = place_nam, npl = place_num, pia = p_inp, poa = p_out ;
            npl-- ; pia++, poa++, place++) {
        if (**poa > 0) {
            int nt;
            if (*(arcp = *pia) > 0) {
                if (! race_policy) {
                    char comp[3];
                    float *Sp;
                    int ii = *(arcp = *poa);
                    int timed = FALSE;
                    int jj;
                    int sum = 0;
                    while (sum >= 0 && ii > 0) {
                        jj = *(++arcp);
                        if (*(Sp = ftimes + jj) > 0.0) {
                            timed = TRUE;
                            if (t_no_inp[jj] == 1) {
                                if (ii > 1)
                                    sum += ii - 1;
                                ii = *(++arcp);
                            }
                            else
                                sum = -1;
                        }
                    }
                    if (timed) {
                        float coef;
                        if (sum >= 0) {
                            if (sum) {
                                fprintf(ofp, "    M_%s <= %d ", *place, sum);
                                for (ii = *(arcp = *poa) ; ii ; ii = *++arcp) {
                                    coef = ii;
                                    ii = *++arcp;
                                    if (*(Sp = ftimes + ii) > 0.0) {
                                        fprintf(ofp, " +");
                                        if ((coef *= *Sp) != 1.0)
                                            fprintf(ofp, "%g ", coef);
                                        fprintf(ofp, "x_%s", trans_nam[ii]);

                                    }
                                }
                                fprintf(ofp, ";\n");
                                strcpy(comp, ">=");
                            }
                            else
                                strcpy(comp, "=");
                        }
                        else
                            strcpy(comp, ">=");
                        fprintf(ofp, "    M_%s %s ", *place, comp);
                        for (ii = *(arcp = *poa), timed = FALSE ;
                                ii ; ii = *++arcp) {
                            coef = ii;
                            ii = *++arcp;
                            if (*(Sp = ftimes + ii) > 0.0) {
                                if (timed)
                                    fprintf(ofp, " +");
                                else
                                    timed = TRUE;
                                if ((coef *= *Sp) != 1.0)
                                    fprintf(ofp, "%g ", coef);
                                fprintf(ofp, "x_%s", trans_nam[ii]);

                            }
                        }
                        fprintf(ofp, ";\n");
                    }
                    arcp = *pia;
                }
                fprintf(ofp, "   ");
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                nt = *++arcp;
                fprintf(ofp, "x_%s", trans_nam[nt]);
                for (arcp++ ; *arcp ; arcp++) {
                    fprintf(ofp, " +");
                    if (*arcp > 1)
                        fprintf(ofp, "%d ", *arcp);
                    nt = *++arcp;
                    fprintf(ofp, "x_%s", trans_nam[nt]);
                }
                fprintf(ofp, " >= ");
            }
            else
                fprintf(ofp, "   0 = ");
            if (*(arcp = *poa) > 1)
                fprintf(ofp, "%d ", *arcp);
            nt = *++arcp;
            fprintf(ofp, "x_%s", trans_nam[nt]);
            for (arcp++ ; *arcp ; arcp++) {
                fprintf(ofp, " +");
                if (*arcp > 1)
                    fprintf(ofp, "%d ", *arcp);
                nt = *++arcp;
                fprintf(ofp, "x_%s", trans_nam[nt]);
            }
            fprintf(ofp, ";\n");
        }
    }
    fprintf(ofp, "\n");
#ifdef DEBUG
    fprintf(stderr, "  Start of produce_lp\n");
#endif
}


int main(int argc, char **argv) {

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[1000];

#ifdef DEBUG
    fprintf(stderr, "Start\n");
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
    sprintf(filename, "%s.lp_flow", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    if (argc >= 3 && !strcmp(argv[2], "-p")) {
        race_policy = FALSE;
    }
    load_net();
    (void) fclose(nfp);
    produce_lp();
    (void) fclose(ofp);
#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
    return 0;
}

