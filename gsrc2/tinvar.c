/*
 *  Paris, February 22, 1990
 *  program: tinvar.c
 *  purpose: Compute T-invariants of a Petri net using Martinez-Silva Algorithm
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) the net description must be in GreatSPN format
 *    2) self loops (impure nets) and inhibitor arcs are just ignored
 */

/*
#define DEBUG to print debug informations
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "const.h"


#define TOB_SIZE (unsigned)(sizeof(struct trans_obj))
#define MAT_SIZE (unsigned)(sizeof(struct matrix))
#define ITM_SIZE (unsigned)(sizeof(short))


typedef short *ROWP;

struct trans_obj {
    char tag[TAG_SIZE];
    int covered;
};

struct matrix {
    ROWP rowp;
    struct matrix *next;
};

FILE *nfp;

struct trans_obj *t_list;

struct matrix *mat, * newm, * oldmp, * oldmm;
ROWP newrow;
int	place_num, trans_num;
unsigned     row_size;
int 	*elim_place;	/* already eliminated places */
int 	*n_pos;	/* number of positive entries in each column */
int 	*n_neg;	/* number of negative entries in each column */

/* predeclarations */
void getname(char *name_pr);
void getarcs(char kind, ROWP trans, int noar, int* mark_pars, int nomp);

char *
emalloc(unsigned nbytes) {
    char 		*mallptr;

    if ((mallptr = malloc(nbytes)) == NULL) {
        fprintf(stderr, "emalloc: couldn't fill request for %d bytes\n",
                nbytes);
        exit(2);
    }
    return (mallptr);
}

short gcd(n1, n2)
short n1, n2;
{
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

// support to modifiable marking parameters
#define MAX_MPARS_TO_CHANGE   32
struct {
    const char *mpar_name;
    int new_val;
} mpars_to_change[MAX_MPARS_TO_CHANGE];
int num_mpars_to_change = 0;


void load_file() {
    struct trans_obj 	*trans;
    struct matrix 	*m_p;
    struct matrix 	*prev_m_p = NULL;
    char		linebuf[LINEMAX], fieldbuf[LINEMAX], *bufptr;
    float		ftemp;
    int                 group_num, i, knd, noar, mark, nomp, norp;
    int		row_l;
    int *mark_pars, k;
    size_t      bufsz;

#define VBAR '|'

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
    t_list = (struct trans_obj *)emalloc((unsigned)(trans_num * TOB_SIZE));
    row_size = (unsigned)(2 * (place_num + trans_num + 1) * ITM_SIZE);
    newrow = (ROWP)emalloc(row_size);
    elim_place = (int *)emalloc((unsigned)(sizeof(int) * (1 + place_num)));
    n_pos = (int *)emalloc((unsigned)(sizeof(int) * (1 + place_num)));
    n_neg = (int *)emalloc((unsigned)(sizeof(int) * (1 + place_num)));
    {
        register int *ii, * ip, * in;
        for (i = 0, ii = elim_place, ip = n_pos, in = n_neg;
                i++ < place_num ;
                * (++ii) = FALSE, *(++ip) = 0, *(++in) = 0);
    }

    /* read marking parameters */
    mark_pars = (int*)emalloc(sizeof(int) * nomp);
    for (i=0; i<nomp; i++) {
        getname(linebuf);
        fscanf(nfp, "%d", &mark);
        for (k = 0; k < num_mpars_to_change; k++) {
            if (0 == strcmp(mpars_to_change[k].mpar_name, linebuf)) {
                mark = mpars_to_change[k].new_val;
                // printf("tinvar: Changing mpar %s value to %d.\n", linebuf, mark);
                break;
            }
        }
        if (mark == -7134) {
            printf("Marking parameter %s needs a value. Specify it with -mpar %s <value>\n", linebuf, linebuf);
            exit(1);
        }
        mark_pars[i] = mark;
        while (getc(nfp) != '\n');
    }

    /* skip places */
    i = 1;
    while (i++ <= place_num) {
        getname(linebuf);
        fscanf(nfp, "%d", &mark);
        while ((getc(nfp)) != '\n');
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

    /* read transitions */
    for (i = 0, trans = t_list ; i++ < trans_num ; trans++) {
        trans -> covered = FALSE;
        getname(trans -> tag);
        fscanf(nfp, "%s %d %d %d", fieldbuf, &mark, &knd, &noar);
        while ((getc(nfp)) != '\n');
        // 13/04/2015:  net/def extension - deterministic transition function 
        // is written after the transition line
        if (0 == strcmp(fieldbuf, "*")) {
            bufptr = linebuf;
            bufsz = LINEMAX-1;
            getline(&bufptr, &bufsz, nfp);
            printf("Skip line: %s\n", linebuf);
        }
        if (mark < 0) {
            int ii;
            ii = mark = -mark;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        {
            register int j; register ROWP p; int maxj = 2 * place_num;
            for (j = 0, p = newrow; j++ < maxj ; * (p++) = 0);
        }
        m_p = (struct matrix *) emalloc(MAT_SIZE);
        if (i == 1) mat = m_p;
        else prev_m_p -> next = m_p;
        prev_m_p = m_p;
        if (i == trans_num) m_p->next = NULL;
        getarcs(TO_TRANS, newrow, noar, mark_pars, nomp);
        fscanf(nfp, "%d\n", &noar);
        getarcs(TO_PLACE, newrow, noar, mark_pars, nomp);
        fscanf(nfp, "%d\n", &noar);
        getarcs(INHIBITOR, newrow, noar, mark_pars, nomp);
        {
            register int j; register ROWP p; int maxj = 2 * place_num;
            for (j = 0, p = newrow, row_l = 0; j++ < maxj ; p++)
                if ((short)(*(p++)) != (short)0) {
                    if ((short)(*p) == (short)0) *(p - 1) = (short)0;
                    else row_l++;
                }
        }
        row_size = ITM_SIZE * 2 * (2 + row_l);;
        m_p->rowp = (ROWP) emalloc(row_size);
        {
            register int j; register ROWP p, p1;
            p = m_p->rowp; *(p++) = 1; *(p++) = i; *(p++) = 1;
            for (j = 0, p1 = newrow ; j++ < row_l ;) {
                while ((short)(*p1) == (short)0) p1 += 2;
                if ((int)(*(p1 + 1)) > 0)
                    ++(n_pos[(int)(*p1)]);
                else
                    ++(n_neg[(int)(*p1)]);
                *(p++) = *(p1++); *(p++) = *(p1++);
            }
            *(p) = 0;
        }
    }
    oldmp = oldmm = NULL;

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


void getarcs(char kind, ROWP trans, int noar, int* mark_pars, int nomp) {
    int			i, j, pl, mlt, ip;
    ROWP		rp;

    for (i = 1 ; i++ <= noar ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while ((getc(nfp)) != '\n');

        // 24/3/2015 Add support for marking parameters on transition arcs.
        if (mlt >= 20000 && mlt < 20000 + nomp) {
            printf("Using marking parameter value %d for arc multiplicity.\n", 
                mark_pars[ mlt - 20000 ]);
            mlt = mark_pars[ mlt - 20000 ];
        }

        rp = trans + 2 * (pl - 1);
        if (kind == TO_PLACE) {
            *(rp++) = (short)pl;
            *rp += (short)(ABS(mlt));
        }
        else if (kind == TO_TRANS) {
            *(rp++) = (short)pl;
            *rp -= (short)(ABS(mlt));
        }
        for (j = 1; j++ <= ip;)
            while ((getc(nfp)) != '\n');
    }
}


/* THE FOLLOWING FUNCTION TESTS FOR MINIMAL SUPPORT */
int test_min(newr)
ROWP newr;
{
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


void elim(short place) {
    extern short gcd();

    struct matrix *rp;
    ROWP pp, ppp;
    short val1, val2, div, gcd1;
#define myABS(A) ( ( (short)(A) >= (short)0 ) ? (A) : -(A) )

    pp = oldmp->rowp;
    for (ppp = pp + (2 * (*pp) + 1) ; (short)(*(ppp++)) != (short)place ; ppp++);
    val1 = *ppp;
    for (rp = oldmm; rp != NULL; rp = rp->next) {
        pp = rp->rowp;
        for (ppp = pp + (2 * (*pp) + 1) ;
                (short)(*(ppp++)) != (short)place ; ppp++);
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

void trans_invar() {
    int i = 0, j;
    int from_place = 1;
    ROWP cp, cp2;
    struct matrix *rp, * nrp, * prp;

    for (j = 0 ; j++ < place_num ;) {
#ifdef DEBUG
        {
            ROWP cp; struct matrix *rp; int i, j;
            printf("\nMATRIX\n");
            for (rp = mat; rp != NULL ; rp = rp->next) {
                for (i = 0, j = *(cp = rp->rowp), cp++ ; i++ < trans_num ;)
                    if ((j > 0) && (i == (int)(*cp))) {
                        j--; cp++;
                        printf(" %2d", (int) * (cp++));
                    }
                    else printf("  0");
                for (i = 0 ; i++ < place_num ;)
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

            for (first = TRUE, ii = from_place, ip = (elim_place + from_place),
            pp = (n_pos + from_place), np = (n_neg + from_place);
            ii <= place_num;
            ip++, pp++, np++, ii++) {
                while (*ip) {
                    ip++; pp++; np++;
                    if (++ii > place_num) goto end_test;
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
        elim_place[i] = TRUE;
        if (i == from_place)
            while (elim_place[from_place])
                ++from_place;
#ifdef DEBUG
        printf("\neliminating place #%d\n", i);
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
}

void show_res() {
    struct matrix *rp;
    ROWP cp;
    struct trans_obj *tp;
    int nocov = 0;
    int i, num, mult, nt;

    for (num = 0, rp = mat; rp != NULL ; num++, rp = rp->next);
    fprintf(nfp, "%d\n", num);
    for (rp = mat; rp != NULL ; rp = rp->next) {
        num = i = *(cp = rp->rowp); fprintf(nfp, "%d", num);
        for (cp++ ; i-- > 0 ; cp++) {
            nt = (int)(*cp);
            tp = &(t_list[ nt - 1 ]);
            if ((short)(*(++cp)) != (short)1) printf(" %d*", (int)*cp);
            else printf(" ");
            mult = (int) * cp;
            fprintf(nfp, " %d %d", mult, nt);
            if (! tp->covered) {
                tp->covered = TRUE; nocov++;
            }
            printf("%s", tp->tag);
        }
        fprintf(nfp, "\n"); printf("\n");
    }
    fprintf(nfp, "0\n");
    if (nocov == trans_num)
        printf("\nALL transitions are covered by some T-invariant\n\n");
    else {
        printf("\nOnly %d transitions out of %d", nocov, trans_num);
        printf(" are covered by some T-invariant.\n\n");
        printf("The following transitions are not covered by any T-invariant:\n\n");
        for (tp = t_list, i = trans_num ; i-- > 0 ; tp++)
            if (! tp->covered) printf(" %s", tp->tag);
        printf("\n\n");
    }
}

int main(int argc, char **argv) {
    char  filename[LINEMAX];
    int ii;

    puts("tinv is known to generate incorrect T-semiflows matrices.");
    puts("(see MCC model SafeBus). Do not use tinv.");
    puts("Use instead:  DSPN-Tool -load <model_name> -tinv");

    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        printf("Can't open file %s\n", filename);
        exit(1);
    }
    for (ii = 2; ii < argc; ii++) {
        if (0 == strcmp(argv[ii], "-mpar")) {
            if (ii + 2 < argc) {
                if (num_mpars_to_change > MAX_MPARS_TO_CHANGE) {
                    fprintf(stderr, "\nError: too many -mpar switches. Raise the value of MAX_MPARS_TO_CHANGE.\n");
                    exit(1);
                }
                mpars_to_change[num_mpars_to_change].mpar_name = argv[++ii];
                mpars_to_change[num_mpars_to_change].new_val = atoi(argv[++ii]);
                num_mpars_to_change++;
            }
            else {
                fprintf(stderr, "\nError:  -mpar  <marking parameter name>  <new value>\n");
                exit(1);
            }
        }
    }
    
    load_file();
    (void) fclose(nfp);
    trans_invar();
    printf("\nTransition invariants for net %s:\n\n", argv[1]);
    sprintf(filename, "%s.tin", argv[1]);
    if ((nfp = fopen(filename, "w")) == NULL) {
        printf("Can't open file %s\n", filename);
        exit(1);
    }
    show_res();
    (void) fclose(nfp);
    return 0;
}

