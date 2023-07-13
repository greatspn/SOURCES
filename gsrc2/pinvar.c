/*
 *  Paris, February 22, 1990
 *  program: pinvar.c
 *  purpose: Compute P-invariants of a Petri net using Martinez-Silva Algorithm
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
#include <limits.h>
#include <sys/types.h>

#include "const.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define POB_SIZE (unsigned)(sizeof(struct place_obj))
#define MAT_SIZE (unsigned)(sizeof(struct matrix))
#define ITM_SIZE (unsigned)(sizeof(short))
#define short int

/* predeclaration */
void getname(char 	*name_pr);
void getarcs(char kind, int trans, int noar, int* mark_pars, int nomp);

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
int 	*n_pos;	/* number of positive entries in each column */
int 	*n_neg;	/* number of negative entries in each column */

// Statistics
const char *net_name;
int      mat_rows_count = 0; // number of rows in *mat
int      mat_rows_peak = 0; // maximum # of rows in *mat in any moment
int      num_tested_combinations = 0; // count of all paired combinations of rows

// Control the maximum amount of peak rows & total tested combinations before leaving up
int MAX_ROWS_PEAK = INT_MAX;
int MAX_TESTED_COMBS = INT_MAX;



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

// support to modifiable marking parameters
#define MAX_MPARS_TO_CHANGE   32
struct {
    const char *mpar_name;
    int new_val;
} mpars_to_change[MAX_MPARS_TO_CHANGE];
int num_mpars_to_change = 0;


void load_file() {

    struct place_obj 	*place;
    struct matrix 	*m_p;
    struct matrix 	*prev_m_p = NULL;
    char		linebuf[LINEMAX], c, fieldbuf[LINEMAX], *bufptr;
    float		ftemp;
    int                 group_num, i, knd, noar, mark, nomp, norp;
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
    while ((c = getc(nfp)) != '\n');
    p_list = (struct place_obj *)emalloc((unsigned)(place_num * POB_SIZE));
    row_size = (unsigned)(2 * (place_num + trans_num + 1) * ITM_SIZE);
    newrow = (ROWP)emalloc(row_size);
    row_size = (unsigned)(2 * (trans_num + 2) * ITM_SIZE);
    elim_trans = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));
    n_pos = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));
    n_neg = (int *)emalloc((unsigned)(sizeof(int) * (1 + trans_num)));
    {
        register int *ii, * ip, * in;
        for (i = 0, ii = elim_trans, ip = n_pos, in = n_neg;
                i++ < trans_num ;
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
                // printf("pinvar: Changing mpar %s value to %d.\n", linebuf, mark);
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
        fscanf(nfp, "%s %d %d %d", fieldbuf, &mark, &knd, &noar);
        while ((c = getc(nfp)) != '\n');
        // 13/04/2015: net/def extension - deterministic transition function 
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
        getarcs(TO_TRANS, i, noar, mark_pars, nomp);
        fscanf(nfp, "%d\n", &noar);
        getarcs(TO_PLACE, i, noar, mark_pars, nomp);
        fscanf(nfp, "%d\n", &noar);
        getarcs(INHIBITOR, i, noar, mark_pars, nomp);
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
        mat_rows_count++;
        mat_rows_peak = MAX(mat_rows_peak, mat_rows_count);
    }
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


void getarcs(char kind, int trans, int noar, int* mark_pars, int nomp) {
    int			i, j, pl, mlt, ip;
    float		x, y;
    struct matrix 	*place;
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

        for (place = mat , j = 1 ; j++ < pl ; place = place->next);
        rp = place->rowp + (2 * trans + 1);
        if (kind == TO_PLACE) {
            *(rp++) = (short)trans;
            *rp += (short)(ABS(mlt));
        }
        else if (kind == TO_TRANS) {
            *(rp++) = (short)trans;
            *rp -= (short)(ABS(mlt));
        }
        for (j = 1; j++ <= ip; fscanf(nfp, "%f %f\n", &x, &y));
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

short calcAndTestOverflow(short val2, short** ppoint1, short val1, short** ppoint2, short div){
    ssize_t sum = ((ssize_t)val2 * (**ppoint1) + (ssize_t)val1 * (**ppoint2))/div;    

    if(sum < INT_MIN || sum > INT_MAX){
        fprintf(stderr, "Integer Overflow\n");
        exit(1);
    }
    (*ppoint1)++;
    (*ppoint2)++;
    return sum;
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
            int j, j1, j2; short val; /*register*/ ROWP point, point1, point2;
            for (j = 0, gcd1 = 0, point = newrow + 1,
                    j1 = *(point1 = oldmp->rowp), point1++,
                    j2 = *(point2 = rp->rowp), point2++;
                    j1 > 0 && j2 > 0 ;) {
                if (*point1 == *point2) {
                    j1--; j2--;
                    *(point++) = *(point1++); point2++;
                    /*if ((short)(val =
                                    (val2 **(point1++) + val1 **(point2++)) / div
                               ) == (short)0)*/
                    val = calcAndTestOverflow(val2, &point1, val1, &point2, div);
                    if((short)val == (short)0)
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
                mat_rows_count++;
                mat_rows_peak = MAX(mat_rows_peak, mat_rows_count);
                if (mat_rows_peak > MAX_ROWS_PEAK) {
                    fprintf(stderr, "Model %s has exceeded P-invariant peak rows. Quit.\n", net_name);
                    exit(-15);
                }
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

void place_invar() {
    int i = 0, j;
    int from_trans = 1;
    ROWP cp, cp2;
    struct matrix *rp, * nrp, * prp;
    int num_positives, num_negatives;

    for (j = 0 ; j++ < trans_num ;) {
        // printf("place_invar: mat_rows_count=%d\n", mat_rows_count);
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
            int first, minval = 0, mm, ii;
            int *ip, * pp, * np;

            for (first = TRUE, ii = from_trans, ip = (elim_trans + from_trans),
            pp = (n_pos + from_trans), np = (n_neg + from_trans);
            ii <= trans_num;
            ip++, pp++, np++, ii++) {
                while (*ip) {
                    ip++; pp++; np++;
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
        num_positives = num_negatives = 0;
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
                    num_positives++;
                }
                else {
                    rp->next = oldmm;
                    oldmm = rp;
                    num_negatives++;
                }
                mat_rows_count--;
            }
            else
                prp = rp;
        }
        // Track the number of row combinations that will be tested.
        num_tested_combinations += num_positives * num_negatives;
        if (num_tested_combinations > MAX_TESTED_COMBS) {
            fprintf(stderr, "Model %s has exceeded P-invariant combination tests. Quit.\n", net_name);
            exit(-15);
        }
        // printf("  LOOP: %s P=%d T=%d  #P-inv=%d  peak_rows=%d combinations=%d\n", 
        //    net_name, place_num, trans_num,
        //    mat_rows_count, mat_rows_peak, num_tested_combinations);

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
    // printf("END: %s P=%d T=%d  #P-inv=%d  peak_rows=%d combinations=%d\n", 
    //        net_name, place_num, trans_num,
    //        mat_rows_count, mat_rows_peak, num_tested_combinations);
}

void show_res(int show_to_stdout) {
    struct matrix *rp;
    ROWP cp;
    struct place_obj *pp;
    int nocov = 0;
    int i ;
    int num, np, mult;

    for (num = 0, rp = mat; rp != NULL ; num++, rp = rp->next);
    fprintf(nfp, "%d\n", num);
    for (rp = mat; rp != NULL ; rp = rp->next) {
        num = i = *(cp = rp->rowp);
        fprintf(nfp, "%d", num);
        for (cp++ ; i-- > 0 ; cp++) {
            np = (int)(*cp);
            pp = &(p_list[ np - 1 ]);
            if ((short)(*(++cp)) != (short)1) {
                if (show_to_stdout) printf(" %d*", (int)*cp);
            }
            else {
                if (show_to_stdout) printf("   ");
            }
            mult = (int) * cp;
            fprintf(nfp, " %d %d", mult, np);
            if (! pp->covered) {
                pp->covered = TRUE; nocov++;
            }
            if (show_to_stdout) printf("%s", pp->tag);
        }
        fprintf(nfp, "\n"); 
        if (show_to_stdout) printf("\n");
    }
    fprintf(nfp, "0\n");
    if (show_to_stdout) {
        if (nocov == place_num)
            printf("\nALL places are covered by some P-invariant\n\n");
        else {
            printf("\nOnly %d places out of %d", nocov, place_num);
            printf(" are covered by some P-invariant.\n\n");
            printf("The following places are not covered by any P-invariant:\n\n");
            for (pp = p_list, i = place_num ; i-- > 0 ; pp++)
                if (! pp->covered) printf(" %s", pp->tag);
            printf("\n\n");
        }
    }
}

int main(int argc, char **argv) {
    char  filename[LINEMAX];
    int ii, show=TRUE, detect_exp=FALSE;

    puts("pinv is known to generate incorrect P-semiflows matrices.");
    puts("(see MCC model SafeBus). Do not use pinv.");
    puts("Use instead:  DSPN-Tool -load <model_name> -pinv");

    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    net_name = argv[1];
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
        else if (0 == strcmp(argv[ii], "-only-bnd")) {
            // Used in conjunction with struct, do not print the results on stdout
            show = FALSE;
        }
        else if (0 == strcmp(argv[ii], "-detect-exp")) {
            // Auto-detect exponential growth in the algorithm, and quit if too many
            // rows are being generated.
            detect_exp = TRUE;
        }
        else {
            fprintf(stderr, "Unknown command line option: %s\n", argv[ii]);
            exit(1);
        }
    }

    load_file();
    (void) fclose(nfp);
    if (detect_exp) {
        // Setup maximum peak/combinations values
        // Peak row bound is just slightly > 1 for most non-exp nets, so 2*|P|
        // seems a reasonable bound
        MAX_ROWS_PEAK = place_num * 2; 
        // As before, 10*|P| is a gross overestimation of the candidate solutions count.
        MAX_TESTED_COMBS = place_num * 10;
    }
    place_invar();
    if (show)
        printf("\nPlace invariants for net %s:\n\n", argv[1]);
    sprintf(filename, "%s.pin", argv[1]);
    if ((nfp = fopen(filename, "w")) == NULL) {
        printf("Can't open file %s\n", filename);
        exit(1);
    }
    show_res(show);
    (void) fclose(nfp);
    return 0;
}

