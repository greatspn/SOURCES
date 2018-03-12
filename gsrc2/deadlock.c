/*
 *  Paris, February 22, 1990
 *  program: deadlock.c
 *  purpose: Compute deadlocks or traps of a Petri net using Alaiwan-Toudic
 *           Algorithm
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) inhibitor arcs and arc multiplicities are just ignored.
 *    2) By default and with option "-d" computes minimal deadlocks.
 *       With option "-t" as second parameter after the net name
 *       computes minimal traps (by reversing the net).
 */

/*
#define DEBUG_TRACE to print trace informations
#define PRINT_MATRIX intermediate states of the incidence matrix transformation
*/

#ifdef DEBUG_TRACE
#define NON_MINIMAL_ALSO
#endif

#include <stdlib.h>
#include <stdio.h>
#include "const.h"

#define POB_SIZE (unsigned)(sizeof(struct place_obj))
#define MAT_SIZE (unsigned)(sizeof(struct matrix))
#define ITM_SIZE (unsigned)(sizeof(short))

/* predeclaration */
void getname(char 	*name_pr);
void skiparcs(int	noar);
void getinarcs(int trans, int noar);
void getoutarcs(int trans);

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
int	place_num, max_place, trans_num, max_trans;
unsigned     row_size;
int 	*elim_trans;	/* already eliminated transitions */
int 	*n_pos;	/* number of positive entries in each column */
int 	*n_neg;	/* number of negative entries in each column */
int 	*out_arcs;	/* number of output arcs for each transition */
int low_trans;


int compute_trap;

char *
emalloc(nbytes)
unsigned nbytes;
{
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

void read_file() {
    char		linebuf[LINEMAX];
    float		ftemp;
    int                 group_num, i, knd, noar, mark, nomp, norp;

#define VBAR '|'

#ifdef DEBUG_TRACE
    printf("\nStart of read_file\n");
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
    while ((getc(nfp)) != '\n');
    out_arcs = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));

    /* skip marking parameters */
    i = 1;
    while (i++ <= nomp) {
        getname(linebuf);
        fscanf(nfp, "%d", &mark);
        while ((getc(nfp)) != '\n');
    }

    /* skip places */
    i = 1;
    while (i <= place_num) {
        getname(linebuf);
        fscanf(nfp, "%d", &mark);
        while ((getc(nfp)) != '\n');
        i++;
    }

    /* skip rate parameters */
    i = 1;
    while (i++ <= norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp);
        while ((getc(nfp)) != '\n');
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        while ((getc(nfp)) != '\n');
    }

    max_trans = 0;
    /* read transitions */
    for (i = 0 ; i++ < trans_num ;) {
        getname(linebuf);
        fscanf(nfp, "%f %d %d %d", &ftemp, &mark, &knd, &noar);
        while ((getc(nfp)) != '\n');
        if (mark < 0) {
            int ii;
            ii = mark = -mark;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        if (compute_trap) {
            max_trans += ((noar > 0) ? noar : 1);
            out_arcs[i] = noar;
        }
        skiparcs(noar);
        fscanf(nfp, "%d\n", &noar);
        if (! compute_trap) {
            max_trans += ((noar > 0) ? noar : 1);
            out_arcs[i] = noar;
        }
        skiparcs(noar);
        fscanf(nfp, "%d\n", &noar);
        skiparcs(noar);
    }

    max_place = place_num + max_trans;

#ifdef DEBUG_TRACE
    printf("\n  max_place=%d, max_trans=%d\n", max_place, max_trans);
    printf("\nEnd of read_file\n");
#endif
}

void skiparcs(int	noar) {
    int		i, j, pl, mlt, ip;

    for (i = 1 ; i++ <= noar ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while ((getc(nfp)) != '\n');
        for (j = 1; j++ <= ip;)
            while ((getc(nfp)) != '\n');
    }
}


void load_file() {
    struct place_obj 	*place;
    struct matrix 	*m_p;
    struct matrix 	*prev_m_p = NULL;
    char		linebuf[LINEMAX];
    float		ftemp;
    int                 group_num, i, knd, noar, mark, nomp, norp;

#define VBAR '|'

#ifdef DEBUG_TRACE
    printf("\nStart of load_file\n");
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
    while ((getc(nfp)) != '\n');
    p_list = (struct place_obj *)emalloc((unsigned)(place_num * POB_SIZE));
    row_size = (unsigned)(2 * (max_place + max_trans + 1) * ITM_SIZE);
    newrow = (ROWP)emalloc(row_size);
    row_size = (unsigned)(2 * (max_trans + 2) * ITM_SIZE);
    elim_trans = (int *)emalloc((unsigned)(sizeof(int) * (1 + max_trans)));
    n_pos = (int *)emalloc((unsigned)(sizeof(int) * (1 + max_trans)));
    n_neg = (int *)emalloc((unsigned)(sizeof(int) * (1 + max_trans)));
    {
        register int *ii, * ip, * in;
        for (i = 0, ii = elim_trans, ip = n_pos, in = n_neg;
                i++ < max_trans ;
                * (++ii) = FALSE, *(++ip) = 0, *(++in) = 0);
    }

    /* skip marking parameters */
    i = 1;
    while (i++ <= nomp) {
        getname(linebuf);
        fscanf(nfp, "%d", &mark);
        while ((getc(nfp)) != '\n');
    }

#ifdef DEBUG_TRACE
    printf("\n  reading places\n");
#endif
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
            for (j = 0 ; j++ < max_trans ; * (p++) = 0, *(p++) = 0);
        }
        fscanf(nfp, "%d", &mark);
        while ((getc(nfp)) != '\n');
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

    /* create dummy places */
    while (i <= max_place) {
        m_p = (struct matrix *) emalloc(MAT_SIZE);
        m_p -> rowp = (ROWP) emalloc(row_size);
        {
            register int j; register ROWP p = m_p->rowp;
            *(p++) = 1; *(p++) = i; *(p++) = 1;
            for (j = 0 ; j++ < max_trans ; * (p++) = 0, *(p++) = 0);
        }
        if (i == 1) {
            mat = m_p;
        }
        else {
            prev_m_p -> next = m_p;
        }
        prev_m_p = m_p;
        if (i++ == max_place) {
            m_p -> next = NULL;
        }
    }
    oldmp = oldmm = NULL;

    /* skip rate parameters */
    i = 1;
    while (i++ <= norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp);
        while ((getc(nfp)) != '\n');
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        while ((getc(nfp)) != '\n');
    }

#ifdef DEBUG_TRACE
    printf("\n  reading transitions\n");
#endif
    low_trans = 1;
    /* read transitions */
    for (i = 0 ; i++ < trans_num ;) {
        getname(linebuf);
        fscanf(nfp, "%f %d %d %d", &ftemp, &mark, &knd, &noar);
        while ((getc(nfp)) != '\n');
        if (compute_trap) {
            int j;
            int low_trans2 = low_trans;
            for (j = 0; j++ < out_arcs[i] ; ++low_trans2) {
                int		j2;
                struct matrix 	*place;
                ROWP		rp;

                getoutarcs(low_trans2);
                for (place = mat , j2 = 1 ; (j2++ < (place_num + low_trans2)) ;
                        place = place->next);
                rp = place->rowp + (2 * low_trans2 + 1);
                *(rp++) = (short)low_trans2;
                *rp = (short)1;
            }
        }
        else
            getinarcs(i, noar);
        fscanf(nfp, "%d\n", &noar);
        if (compute_trap) {
            getinarcs(i, noar);
            low_trans += out_arcs[i];
        }
        else {
            int j;
            for (j = 0; j++ < out_arcs[i] ; ++low_trans) {
                int		j2;
                struct matrix 	*place;
                ROWP		rp;

                getoutarcs(low_trans);
                for (place = mat , j2 = 1 ; (j2++ < (place_num + low_trans)) ;
                        place = place->next);
                rp = place->rowp + (2 * low_trans + 1);
                *(rp++) = (short)low_trans;
                *rp = (short)1;
            }

        }
        fscanf(nfp, "%d\n", &noar);
        skiparcs(noar);
    }
    /* compact rows */
    for (m_p = mat ; m_p != NULL ; m_p = m_p->next) {
        register ROWP pp, np; register int i;
        for (pp = np = (m_p->rowp + 3), i = max_trans ; i-- > 0 ; np += 2)
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

#ifdef DEBUG_TRACE
    printf("\nEnd of load_file\n");
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


void getinarcs(int trans, int noar) {
    int			i, j, pl, mlt, ip;
    struct matrix 	*place;
    ROWP		rp;
    int from;

    for (i = 0 ; i++ < noar ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while ((getc(nfp)) != '\n');
        for (place = mat , j = 1 ; j++ < pl ; place = place->next);
        from = low_trans;
        rp = place->rowp + (2 * from + 1);
        for (j = 0 ; j++ < out_arcs[trans] ; from++) {
            *(rp++) = (short)from;
            *(rp++) = - (short)1;
        }
        for (j = 1; j++ <= ip;)
            while ((getc(nfp)) != '\n');
    }
}

void getoutarcs(int trans) {
    int			j, pl, mlt, ip;
    struct matrix 	*place;
    ROWP		rp;

    fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
    while ((getc(nfp)) != '\n');
    for (place = mat , j = 1 ; j++ < pl ; place = place->next);
    rp = place->rowp + (2 * trans + 1);
    *(rp++) = (short)trans;
    if ((int)(*rp) == 0)
        *rp = (short)1;
    for (j = 1; j++ <= ip;)
        while ((getc(nfp)) != '\n');
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
#ifdef DEBUG_TRACE
    printf("\nStart of elim(%d)\n", (int)trans);
#endif

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
#ifdef DEBUG_TRACE
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

#ifdef DEBUG_TRACE
    printf("\nEnd of elim(%d)\n", (int)trans);
#endif
}

void destroy_mat(struct matrix *point) {
    free(point->rowp); free(point);
}

void deadlocks() {
    int i = 0, j;
    int from_trans = 1;
    ROWP cp, cp2;
    struct matrix *rp, * nrp, * prp;

#ifdef DEBUG_TRACE
    printf("\nStart of deadlocks\n");
#endif
    for (j = 0 ; j++ < max_trans ;) {
#ifdef DEBUG_TRACE
#ifdef PRINT_MATRIX
#else
        if (j == max_trans)
#endif
        {
            ROWP cp; struct matrix *rp; int i, j;
            printf("\nMATRIX\n");
            for (rp = mat; rp != NULL ; rp = rp->next) {
                for (i = 0, j = *(cp = rp->rowp), cp++ ; i++ < max_place ;)
                    if ((j > 0) && (i == (int)(*cp))) {
                        j--; cp++;
                        printf(" %2d", (int) * (cp++));
                    }
                    else printf("  0");
                for (i = 0 ; i++ < max_trans ;)
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
            int first, minval = 0, mm, ii;
            int *ip, * pp, * np;

#ifdef DEBUG_TRACE
            printf("  from_trans=%d\n", from_trans);
#endif
            for (first = TRUE, ii = from_trans, ip = (elim_trans + from_trans),
                    pp = (n_pos + from_trans), np = (n_neg + from_trans);
                    ii <= max_trans;
                    ip++, pp++, np++, ii++) {
                while (*ip) {
                    ip++; pp++; np++;
                    if (++ii > max_trans) goto end_test;
                }
                mm = (*pp) * (*np) - (*pp) - (*np);
#ifdef DEBUG_TRACE
                printf("  testing trans %d, mm=%d", ii, mm);
#endif
                if (first) {
#ifdef DEBUG_TRACE
                    printf(" first\n");
#endif
                    i = ii; minval = mm; first = FALSE;
                }
                else if (mm < minval) {
#ifdef DEBUG_TRACE
                    printf(" better\n");
#endif
                    i = ii; minval = mm;
                }
#ifdef DEBUG_TRACE
                else printf("\n");
#endif
            }
        }
end_test:
#ifdef DEBUG_TRACE
        printf("\neliminating transition #%d\n", i);
#endif
        elim_trans[i] = TRUE;
        if (i == from_trans)
            while (elim_trans[from_trans])
                ++from_trans;
#ifdef DEBUG_TRACE
        printf("  from_trans=%d\n", from_trans);
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

#ifdef DEBUG_TRACE
    printf("\nEnd of deadlocks\n");
#endif
}


struct matrix *non_min = NULL;

int non_minimal(ROWP newr) {
    struct matrix *rp;
    register int i, j;
    register ROWP cp, val;

    for (rp = mat; rp != NULL ; rp = rp->next) {
        if ((i = (int)(*(cp = rp->rowp))) != (j = (int)(*(val = newr))))
            goto notminimal;
        for (cp++ , val++ ; i-- > 0 ; cp += 2, val += 2) {
            if ((short)(*val) != (short)(*cp))
                goto notminimal;
        }
        return (FALSE);
notminimal:
        continue;
    }
    for (rp = non_min; rp != NULL ; rp = rp->next) {
        if ((i = (int)(*(cp = rp->rowp))) != (j = (int)(*(val = newr))))
            goto notequal;
        for (cp++ , val++ ; i-- > 0 ; cp += 2, val += 2) {
            if ((short)(*val) != (short)(*cp))
                goto notequal;
        }
        return (FALSE);
notequal:
        continue;
    }
    return (TRUE);
}


void reduce() {
    struct matrix *rp, * next_rp, * prp, * oldmat;
    register int i, j = 0, new_i;
    register ROWP cp;

#ifdef DEBUG_TRACE
    printf("\nStart of reduce\n");
#endif
    for (prp = NULL, rp = NULL, next_rp = mat ; next_rp != NULL ;) {
        for ((i = (int)(*(cp = next_rp->rowp))) , new_i = 0; i-- > 0 ; cp++)
                if ((int)(*(++cp)) <= place_num)  new_i++; else  i = 0;
        if (new_i == 0) {
            next_rp = next_rp->next;
            if (prp == NULL) {
                destroy_mat(mat);
                mat = next_rp;
            }
            else {
                destroy_mat(prp->next);
                prp->next = next_rp;
            }
        }
        else {
            *(next_rp->rowp) = (short)new_i;
            if (prp == NULL || new_i < j) {
                j = new_i; rp = ((prp == NULL) ? next_rp : prp);
            }
            prp = next_rp; next_rp = next_rp->next;
        }
    }
    if (mat == NULL)  return;
    if (rp == mat)  oldmat = mat->next;
    else {
        prp = rp; next_rp = rp->next;
        oldmat = mat; prp->next = next_rp->next;
        mat = next_rp;
    }
    for (mat->next = NULL ; oldmat != NULL ; j = new_i) {
        new_i = place_num + 1;
        for (rp = oldmat, prp = NULL ; rp != NULL ; rp = next_rp) {
            next_rp = rp->next;
            if ((i = (int)(*(rp->rowp))) > j) {
                if (i < new_i)  new_i = i;
                prp = rp;
            }
            else {
                if (rp == oldmat)  oldmat = next_rp;
                else  prp->next = next_rp;
                if (test_min(rp->rowp)) {
                    rp->next = mat; mat = rp;
                }
                else if (non_min == NULL || non_minimal(rp->rowp)) {
                    rp->next = non_min; non_min = rp;
                }
                else  destroy_mat(rp);
            }
        }
    }

#ifdef DEBUG_TRACE
    printf("\nEnd of reduce\n");
#endif
}

void show_res(char *netname) {
    struct matrix *rp;
    ROWP cp;
    struct place_obj *pp;
    int nocov = 0;
    int i, printed, j;

#ifdef DEBUG_TRACE
    printf("\nStart of show_res\n");
#endif
    if (compute_trap)
        printf("\nMinimal traps for net %s:\n\n", netname);
    else
        printf("\nMinimal deadlocks for net %s:\n\n", netname);
    for (i = 0, rp = mat; rp != NULL ; rp = rp->next)
        if ((int)(*(cp = rp->rowp)))
            ++i;
    fprintf(nfp, "%d\n", i);
    for (rp = mat; rp != NULL ; rp = rp->next) {
        printed = FALSE;
        if ((i = *(cp = rp->rowp))) {
            fprintf(nfp, "%d", i);
            for (cp++ ; i-- > 0 ; cp += 2) {
                j = (int)(*cp);
                fprintf(nfp, " %d", j);
                pp = &(p_list[ j - 1 ]);
                if (! pp->covered) {
                    pp->covered = TRUE; nocov++;
                }
                printf(" %s", pp->tag);
            }
            printf("\n"); fprintf(nfp, "\n");
        }
    }
    fprintf(nfp, "0\n");
#ifdef NON_MINIMAL_ALSO
    printf("\nOther deadlocks for net %s:\n\n", netname);
    for (rp = non_min; rp != NULL ; rp = rp->next) {
        printed = FALSE;
        for (i = *(cp = rp->rowp), cp++ ; i-- > 0 ; cp += 2) {
            pp = &(p_list[(int)(*cp) - 1 ]);
            if (! pp->covered) {
                pp->covered = TRUE; nocov++;
            }
            printf(" %s", pp->tag); printed = TRUE;
        }
        if (printed)  printf("\n");
    }
    if (nocov < place_num) {
        printf("\nWARNING: only %d places out of %d", nocov, place_num);
        printf(" are contained in some deadlock.\n\n");
        printf("The following places are not contained in any deadlock:\n\n");
        for (pp = p_list, i = place_num ; i-- > 0 ; pp++)
            if (! pp->covered) printf(" %s", pp->tag);
        printf("\n\n");
    }
#endif

#ifdef DEBUG_TRACE
    printf("\nEnd of show_res\n");
#endif
}

int main(int argc, char **argv) {
    char  filename[LINEMAX];

    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    compute_trap = FALSE;
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii--) {
            s_p = *(a_p++);
            if (*(s_p++) != '-' ||
                    (*s_p != 't' && *s_p != 'd')) {
                fprintf(stderr, "ERROR: unknown parameter '%s'\n", s_p - 1);
                exit(33);
            }
            compute_trap = (int)(*s_p == 't');
        }
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        printf("Can't open file %s\n", filename);
        exit(2);
    }
    read_file();
    (void) fclose(nfp);
    if ((nfp = fopen(filename, "r")) == NULL) {
        printf("Can't open file %s\n", filename);
        exit(3);
    }
    load_file();
    (void) fclose(nfp);
    deadlocks();
    reduce();
    if (compute_trap)
        sprintf(filename, "%s.mtrap", argv[1]);
    else
        sprintf(filename, "%s.mdead", argv[1]);
    if ((nfp = fopen(filename, "w")) == NULL) {
        printf("Can't open file %s\n", filename);
        exit(3);
    }
    show_res(argv[1]);
    (void) fclose(nfp);
    return 0;
}

