/*
 *  Paris, February 22, 1990
 *  program: implp.c
 *  purpose: Test whether a place of a Petri net is implicit
 *           (using Silva's Algorithm)
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) the net description must be in GreatSPN format
 *    2) self loops and inhibitor arcs are just ignored
 */

/*
#define DEBUG to print debug informations
*/

#include <stdlib.h>
#include <stdio.h>

#include "const.h"

#define POB_SIZE (unsigned)(sizeof(struct place_obj))
#define MAT_SIZE (unsigned)(sizeof(struct matrix))
#define ITM_SIZE (unsigned)(sizeof(short))

/* predeclaration */
void getname(char 	*name_pr);
void getarcs(char kind, int trans, int noar);

int testplace;



typedef short *ROWP;

struct place_obj {
    char tag[TAG_SIZE];
    int covered;
};

struct matrix {
    ROWP rowp;
    struct matrix *next;
};

FILE *nfp;

struct place_obj *p_list;

struct matrix *mat, * newm, * oldmp, * oldmm;
ROWP newrow;
int	place_num, trans_num;
unsigned     row_size;
int 	*elim_trans;	/* already eliminated transitions */
int 	*poss_trans;	/* eliminable transitions */
int 	*n_pos;	/* number of positive entries in each column */
int 	*n_neg;	/* number of negative entries in each column */


char *emalloc(unsigned nbytes) {
    char 		*mallptr;

    if ((mallptr = malloc(nbytes)) == NULL) {
        fprintf(stderr, "emalloc: couldn't fill request for %d bytes\n",
                nbytes);
        exit(2);
    }
    return (mallptr);
}

short gcd(short n1, short n2) {
    short r;

    if (n2 > n1) {
        r = n1; n1 = n2; n2 = r;
    }
    while (n2 > (short)0) {
        r = n1 % n2;
        n1 = n2;
        n2 = r;
    }
    return (n1);
}

void load_file() {
    struct place_obj 	*place;
    struct matrix 	*m_p;
    struct matrix 	*prev_m_p = NULL;
    char		linebuf[LINEMAX], c;
    float		ftemp;
    int                 group_num, i, knd, noar, mark, nomp, norp;

#define VBAR '|'

#ifdef DEBUG
    fprintf(stderr, "  Start of load_file\n");
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
    fscanf(nfp, "%d %d", &trans_num, &group_num);
    if (testplace > place_num) {
        fprintf(stderr, "ERROR: place %d does not exist !\n", testplace);
        exit(1);
    }
    while ((c = getc(nfp)) != '\n');
    p_list = (struct place_obj *)emalloc((unsigned)(place_num * POB_SIZE));
    row_size = (unsigned)(2 * (place_num + trans_num + 1) * ITM_SIZE);
    newrow = (ROWP)emalloc(row_size);
    row_size = (unsigned)(2 * (trans_num + 2) * ITM_SIZE);
    elim_trans = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));
    poss_trans = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));
    n_pos = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));
    n_neg = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));
    {
        register int *ii, * jj, * ip, * in;
        for (i = 0, ii = elim_trans, jj = poss_trans, ip = n_pos, in = n_neg;
                i++ < trans_num ;
                * (++jj) = FALSE, *(++ii) = FALSE, *(++ip) = 0, *(++in) = 0);
    }

    /* skip marking parameters */
    i = 1;
    while (i++ <= nomp) {
        getname(linebuf);
        fscanf(nfp, "%d", &mark);
        while ((c = getc(nfp)) != '\n');
    }

    /* read places */
    i = 1; place = p_list;
    while (i <= place_num) {
        place->covered = FALSE;
        getname(place -> tag);
        m_p = (struct matrix *) emalloc(MAT_SIZE);
        m_p -> rowp = (ROWP) emalloc(row_size);
        {
            register int j; register ROWP p = m_p->rowp;
            *(p++) = 1; *(p++) = i; *(p++) = 1;
            for (j = 0 ; j++ < trans_num ; * (p++) = 0, *(p++) = 0);
            *p = 0;
        }
        fscanf(nfp, "%d", &mark);
        while ((c = getc(nfp)) != '\n');
        if (i == 1) {
            mat = m_p;
        }
        else {
            prev_m_p -> next = m_p;
        }
        prev_m_p = m_p;
        if (i++ == place_num) {
            m_p -> next = NULL;
        }
        place++;
    }
    oldmp = oldmm = NULL;

    /* skip rate parameters */
    i = 1;
    while (i++ <= norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp);
        while ((c = getc(nfp)) != '\n');
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        while ((c = getc(nfp)) != '\n');
    }

    /* read transitions */
    for (i = 0 ; i++ < trans_num ;) {
        getname(linebuf);
        fscanf(nfp, "%f %d %d %d",
               &ftemp, &mark, &knd, &noar);
        while ((c = getc(nfp)) != '\n');
        if (mark < 0) {
            int ii;
            ii = mark = -mark;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        getarcs(TO_TRANS, i, noar);
        fscanf(nfp, "%d\n", &noar);
        getarcs(TO_PLACE, i, noar);
        fscanf(nfp, "%d\n", &noar);
        getarcs(INHIBITOR, i, noar);
    }
    /* compact rows */
    for (m_p = mat ; m_p != NULL ; m_p = m_p->next) {
        register ROWP pp, np; register int i;
        for (pp = np = (m_p->rowp + 3), i = trans_num ; i-- > 0 ; np += 2)
            if (((short)(*np) != (short)0)
                    && ((short)(*(np + 1)) != (short)0)) {
                *(pp++) = *np;
                if ((int)(*(pp++) = *(np + 1)) > 0)
                    ++(n_pos[(int)(*np)]);
                else
                    ++(n_neg[(int)(*np)]);
            }
        *pp = 0;
    }
#ifdef DEBUG
    fprintf(stderr, "  End of load_file\n");
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


void getarcs(char kind, int trans, int noar) {
    int			i, j, pl, mlt, ip;
    struct matrix 	*place;
    ROWP		rp;

    for (i = 1 ; i++ <= noar ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while ((getc(nfp)) != '\n');
        for (place = mat , j = 1 ; j++ < pl ; place = place->next);
        rp = place->rowp + (2 * trans + 1);
        if (kind == TO_PLACE) {
            *(rp++) = (short)trans;
            if (pl == testplace)
                *rp -= (short)(ABS(mlt));
            else
                *rp += (short)(ABS(mlt));
        }
        else if (kind == TO_TRANS) {
            *(rp++) = (short)trans;
            if (pl == testplace)
                *rp += (short)(ABS(mlt));
            else
                *rp -= (short)(ABS(mlt));
        }
        for (j = 1; j++ <= ip;)
            while ((getc(nfp)) != '\n');
    }
}

/* THE FOLLOWING FUNCTION TESTS FOR MINIMAL SUPPORT */
int test_min(ROWP newr) {
    struct matrix *rp;
    register int i, j;
    register ROWP cp, val;

    for (rp = mat; rp != NULL ; rp = rp->next) {
        if ((i = *(cp = rp->rowp)) > (j = *(val = newr)))
            goto notincluded;
        for (cp++ , val++ ; i-- > 0 ; cp += 2) {
            while ((j > 0) && (*val < *cp)) {
                j--; val += 2;
            }
            if ((j <= 0) || (*val != *cp))
                goto notincluded;
        }
        return (FALSE);
notincluded:
        continue;
    }
    return (TRUE);
}


void elim(short trans) {
    struct matrix *rp;
    ROWP pp, ppp;
    short val1, val2, div, gcd1;
#define myABS(A) ( ( (short)(A) >= (short)0 ) ? (A) : -(A) )

    pp = oldmp->rowp;
    for (ppp = pp + (2 * (*pp) + 1) ; (short)(*(ppp++)) != (short)trans ; ppp++);
    val1 = *ppp;
    for (rp = oldmm; rp != NULL; rp = rp->next) {
        pp = rp->rowp;
        for (ppp = pp + (2 * (*pp) + 1) ;
                (short)(*(ppp++)) != (short)trans ; ppp++);
        val2 = - *ppp;
        div = gcd(val1, val2);
        {
            int j, j1, j2; short val; register ROWP point, point1, point2;
            for (j = 0, gcd1 = 0, point = newrow + 1,
                    j1 = *(point1 = oldmp->rowp), point1++,
                    j2 = *(point2 = rp->rowp), point2++;
                    j1 > 0 && j2 > 0 ;) {
                if (*point1 == *point2) {
                    j1--; j2--;
                    *(point++) = *(point1++); point2++;
                    if ((short)(val =
                                    (val2 **(point1++) + val1 **(point2++)) / div
                               ) == (short)0)
                        * (--point) = 0;
                    else {
                        *(point++) = val; j++;
                        if (gcd1 != (short)1) {
                            if (gcd1 == (short)0)  gcd1 = myABS(val);
                            else  gcd1 = gcd(myABS(val), gcd1);
                        }
                    }
                }
                else if (*point1 > *point2) {
                    j++; j2--;
                    *(point++) = *(point2++);
                    val = *(point++) = (short)((val1 **(point2++)) / div);
                    if (gcd1 != (short)1) {
                        if (gcd1 == (short)0)  gcd1 = myABS(val);
                        else  gcd1 = gcd(myABS(val), gcd1);
                    }
                }
                else {
                    j++; j1--;
                    *(point++) = *(point1++);
                    val = *(point++) = (short)((val2 **(point1++)) / div);
                    if (gcd1 != (short)1) {
                        if (gcd1 == (short)0)  gcd1 = myABS(val);
                        else  gcd1 = gcd(myABS(val), gcd1);
                    }
                }
            }
            while (j1-- > 0) {
                j++;
                *(point++) = *(point1++);
                val = *(point++) = (short)((val2 **(point1++)) / div);
                if (gcd1 != (short)1) {
                    if (gcd1 == (short)0)  gcd1 = myABS(val);
                    else  gcd1 = gcd(myABS(val), gcd1);
                }
            }
            while (j2-- > 0) {
                j++;
                *(point++) = *(point2++);
                val = *(point++) = (short)((val1 **(point2++)) / div);
                if (gcd1 != (short)1) {
                    if (gcd1 == (short)0)  gcd1 = myABS(val);
                    else  gcd1 = gcd(myABS(val), gcd1);
                }
            }
            *newrow = j;
            while (((short)(*point1) != (short)0) &&
                    ((short)(*point2) != (short)0))  {
                if (*point1 == *point2) {
                    *(point++) = *(point1++); point2++;
                    if ((short)(val =
                                    (val2 **(point1++) + val1 **(point2++)) / div
                               ) == (short)0)
                        * (--point) = 0;
                    else {
                        *(point++) = val; j++;
                        if (gcd1 != (short)1) {
                            if (gcd1 == (short)0)  gcd1 = myABS(val);
                            else  gcd1 = gcd(myABS(val), gcd1);
                        }
                    }
                }
                else if (*point1 > *point2) {
                    *(point++) = *(point2++); j++;
                    val = *(point++) = (short)((val1 **(point2++)) / div);
                    if (gcd1 != (short)1) {
                        if (gcd1 == (short)0)  gcd1 = myABS(val);
                        else  gcd1 = gcd(myABS(val), gcd1);
                    }
                }
                else {
                    *(point++) = *(point1++); j++;
                    val = *(point++) = (short)((val2 **(point1++)) / div);
                    if (gcd1 != (short)1) {
                        if (gcd1 == (short)0)  gcd1 = myABS(val);
                        else  gcd1 = gcd(myABS(val), gcd1);
                    }
                }
            }
            while ((short)(*point1) != (short)0) {
                j++; *(point++) = *(point1++);
                val = *(point++) = (short)((val2 **(point1++)) / div);
                if (gcd1 != (short)1) {
                    if (gcd1 == (short)0)  gcd1 = myABS(val);
                    else  gcd1 = gcd(myABS(val), gcd1);
                }
            }
            while ((short)(*point2) != (short)0) {
                j++; *(point++) = *(point2++);
                val = *(point++) = (short)((val1 **(point2++)) / div);
                if (gcd1 != (short)1) {
                    if (gcd1 == (short)0)  gcd1 = myABS(val);
                    else  gcd1 = gcd(myABS(val), gcd1);
                }
            }
            *point = 0;
            if (test_min(newrow)) {
                newm = (struct matrix *)emalloc(MAT_SIZE);
                newm->next = mat; mat = newm;
                newm->rowp = point = (ROWP)emalloc((unsigned)(2 * (j + 1) * ITM_SIZE));
                for (j = *(point1 = newrow), *(point++) = j, point1++;
                        j-- > 0 ;
                        * (point++) = *(point1++), *(point++) = *(point1++) / gcd1);
                while ((short)(*point1) != (short)0) {
                    *(point++) = *(point1++);
                    if ((*(point++) = *(point1++) / gcd1) > 0)
                        ++(n_pos[(int) * (point - 2)]);
                    else
                        ++(n_neg[(int) * (point - 2)]);
                }
                *(point) = 0;
#ifdef DEBUG
                printf("new row ");
                for (j = *(point = newm->rowp), printf(" %d :\n", (int)j), point++; j-- > 0 ;
                        printf("  %d", (int)(*(point++))), printf(" %d,", (int)(*(point++))));
                while ((short)(*point) != (short)0) {
                    printf(";  %d", (int)(*(point++)));
                    printf(" %d", (int)(*(point++)));
                }
                printf("\n");
#endif
            }
        }
    }
}

void destroy_mat(struct matrix *point) {
    free(point->rowp); free(point);
}

void place_support() {
    int i, j;
    int from_trans = 1;
    ROWP cp, cp2;
    struct matrix *rp, * nrp, * prp;

#ifdef DEBUG
    fprintf(stderr, "  Start of place_support\n");
#endif
    for (j = 0 ; j++ < trans_num ;) {
#ifdef DEBUG
        {
            ROWP cp; struct matrix *rp; int i, j;
            printf("\nMATRIX\n");
            for (rp = mat; rp != NULL ; rp = rp->next) {
                for (i = 0, j = *(cp = rp->rowp), cp++ ; i++ < place_num ;)
                    if ((j > 0) && (i == (int)(*cp))) {
                        j--; cp++;
                        printf(" %2d", (int) * (cp++));
                    }
                    else printf("  0");
                for (i = 0 ; i++ < trans_num ;)
                    if ((i == (int)(*cp))) {
                        cp++;
                        printf(" %2d", (int) * (cp++));
                    }
                    else printf("  0");
                printf("\n");
            }
        }
#endif
        {
            int ii, ntr = 0; ROWP cp; struct matrix *rp; int *ip;

            for (ii = trans_num + 1, ip = poss_trans ; ii-- ; * (ip++) = FALSE);
            for (rp = mat; rp != NULL ; rp = rp->next) {
                for (i = *(cp = rp->rowp), cp++ ; i-- && *cp <= testplace ;
                cp += 2)
                    if (*cp == testplace) {
                        i = 0;
                        ii = *(cp = rp->rowp); cp += (2 * ii + 1);
                        while ((ii = *cp)) {
                            cp += 2;
                            ip = (poss_trans + ii);
                            if (!(*ip) && !(*(elim_trans + ii))) {
#ifdef DEBUG
                                printf("transition %d possible\n", ii);
#endif
                                *ip = TRUE; ntr++;
                            }
                        }
                    }
            }
            if (! ntr) {
#ifdef DEBUG
                fprintf(stderr, "  End of place_support\n");
#endif
                return;
            }
        } {
            int first, minval = 0, mm, ii;
            int *ip, * it, * pp, * np;

            for (first = TRUE, ii = from_trans, ip = (elim_trans + from_trans),
            it = (poss_trans + from_trans),
            pp = (n_pos + from_trans), np = (n_neg + from_trans);
            ii <= trans_num;
            ip++, it++, pp++, np++, ii++)
                if (*it) {
                    while (*ip) {
                        it++; ip++; pp++; np++;
                        if (++ii > trans_num) goto end_test;
                    }
                    mm = (*pp) * (*np) - (*pp) - (*np);
                    if (first) {
                        i = ii; minval = mm; first = FALSE;
                    }
                    else if (mm < minval) {
                        i = ii; minval = mm;
                    }
                }
        }
end_test:
        elim_trans[i] = TRUE;
        if (i == from_trans)
            while (elim_trans[from_trans])
                ++from_trans;
#ifdef DEBUG
        printf("\neliminating transition #%d\n", i);
#endif
        for (prp = NULL, rp = mat ; rp != NULL ; rp = nrp) {
            nrp = rp->next;
            cp2 = cp = (rp->rowp) + (2 * *(rp->rowp) + 1);
            while (((int)(*cp) < i) && ((int)(*cp) > 0))
                cp += 2;
            if ((int)(*cp) == i) {   /* TAKE THE ROW OUT FROM MATRIX LIST */
                while ((short)(*cp2) != (short)0) {
                    if ((short)(*(cp2 + 1)) > (short)0)
                        --(n_pos[(int)(*cp2)]);
                    else
                        --(n_neg[(int)(*cp2)]);
                    cp2 += 2;
                }
                if (rp == mat)
                    mat = nrp;
                else
                    prp->next = nrp;
                if ((short)(*(++cp)) > (short)0) {
                    rp->next = oldmp;
                    oldmp = rp;
                }
                else {
                    rp->next = oldmm;
                    oldmm = rp;
                }
            }
            else
                prp = rp;
        }
        if (oldmp != NULL && oldmm != NULL) {
            while ((rp = oldmp) != NULL) {
                elim((short)i);
                oldmp = oldmp->next;
                destroy_mat(rp);
            }
        }
        else
            while ((rp = oldmp) != NULL) {
                oldmp = oldmp->next;
                destroy_mat(rp);
            }
        while ((rp = oldmm) != NULL) {
            oldmm = oldmm->next;
            destroy_mat(rp);
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End of place_support\n");
#endif
}

void show_res() {
    struct matrix *rp, * prp = NULL;
    ROWP cp;
    struct place_obj *pp;
    int nocov = 0;
    int i;
    int num, np, mult;
    int mmm = 0;

#ifdef DEBUG
    fprintf(stderr, "  Start of show_res\n");
#endif
    for (num = 0, rp = mat; rp != NULL ; rp = rp->next) {
        for (i = *(cp = rp->rowp), cp++ ; i && *cp < testplace ;
                cp += 2, i--);
        if (i && (*cp == testplace)) {
            prp = rp; num++;
        }
        else {
            if (rp == mat)
                mat = rp->next;
            else
                prp->next = rp->next;
        }
    }
    fprintf(nfp, "%d\n", num);
    if (num) {
        printf("\nPlace %s is implied by:\n\n",
               p_list[testplace - 1].tag);
        for (rp = mat; rp != NULL ; rp = rp->next) {
            i = *(cp = rp->rowp); num = i - 1;
            fprintf(nfp, "%d", num + 1);
            for (cp++ ; i-- > 0 ; cp++) {
                if ((np = (int)(*(cp++))) == testplace)
                    mmm = (int) * cp;
                else {
                    pp = &(p_list[ np - 1 ]);
                    if ((short)(*cp) != (short)1)
                        printf(" %d*", (int)*cp);
                    else
                        printf("   ");
                    mult = (int) * cp;
                    fprintf(nfp, " %d %d", mult, np);
                    if (! pp->covered) {
                        pp->covered = TRUE; nocov++;
                    }
                    printf("%s", pp->tag);
                }
            }
            fprintf(nfp, " %d %d\n", mmm, testplace);
            if (mmm > 1)
                printf(" ==> %d %s\n", mmm, p_list[testplace - 1].tag);
            else
                printf("\n");
        }
    }
    else
        printf("\nPlace %s is NOT structurally implicit.\n\n",
               p_list[testplace - 1].tag);
    fprintf(nfp, "0\n");
#ifdef DEBUG
    fprintf(stderr, "  End of show_res\n");
#endif
}

int main(int argc, char **argv) {
    char  filename[LINEMAX], * nn;

#ifdef DEBUG
    fprintf(stderr, "Start of implp, argc=%d\n", argc);
#endif
    if (argc < 3 ||
            (nn = argv[2], sscanf(nn, "%d", &testplace), testplace < 1)) {
        fprintf(stderr, "USAGE: implp netpathname placenumber\n");
        exit(1);
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Can't open file %s\n", filename);
        exit(1);
    }
    load_file();
    (void) fclose(nfp);
    place_support();
    sprintf(filename, "%s.impl", argv[1]);
    if ((nfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "Can't open file %s\n", filename);
        exit(1);
    }
    show_res();
    (void) fclose(nfp);
#ifdef DEBUG
    fprintf(stderr, "End of implp\n");
#endif
    return 0;
}

