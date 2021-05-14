/*
 *  Torino, September 3, 1987
 *  program: comp_main.c
 *  purpose: prepare compilation module for RG construction of a GSPN
 *           using M-invariants information.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) the net description must be in GreatSPN format
 */


/*
#define DEBUG
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *nfp, *dfp, * ifp, * ccfp, * scfp, * ofp, * rafp, *rsfp, *ssfp;

#include "const.h"

#include "comp.h"

struct place_obj *p_list;

struct I_header *M_invars, * unused;
int no_M_invars, no_alloc_inv, no_unused;
int no_alloc_pl;

struct C_mark *encoding, * cccp;
int no_cod = 0, no_byte = 0;

int	place_num, trans_num;


extern char   **p_names;

extern int nomp;
extern ROWP MP;
extern char   **m_names;

extern int norp;
extern float 	*RP;
extern char   **r_names;


unsigned int nobits(n)
unsigned int n;
{
    if (n <= 1)
        return ((unsigned)1);
    if (n <= 3)
        return ((unsigned)2);
    if (n <= 7)
        return ((unsigned)3);
    if (n <= 15)
        return ((unsigned)4);
    if (n <= 31)
        return ((unsigned)5);
    if (n <= 63)
        return ((unsigned)6);
    return ((unsigned)8);
}

char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *	calloc();
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}

void
load_places() {
    extern void getname();

    struct place_obj 	*place;
    struct matrix 	*m_p;
    struct matrix 	*prev_m_p;
    char		linebuf[LINEMAX];
    float		ftemp, x, y;
    int                 group_num, i, j, knd, noar, mark, orien;
#define VBAR '|'

#ifdef DEBUG
    printf("  Start of load_places\n");
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
    p_list = (struct place_obj *)ecalloc(place_num, POB_SIZE);
    p_names = (char **)ecalloc(place_num, sizeof(char *));
    m_names = (char **)ecalloc(nomp, sizeof(char *));
    MP = (ROWP)ecalloc(nomp, ROWT_SIZE);
    r_names = (char **)ecalloc(norp, sizeof(char *));
    RP = (float *)ecalloc(norp, sizeof(float));

#ifdef DEBUG
    printf("    %d marking parameters\n", nomp);
#endif

    /* read marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        while (i < nomp) {
            getname(linebuf);
            m_names[i] = ecalloc(strlen(linebuf) + 1, sizeof(char));
            sprintf(m_names[i], linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
            printf("    %d %s=%d\n", i, linebuf, mark);
#endif
            MP[i++] = mark;
        }
    }

    /* read places */
    i = 0; place = p_list;
    while (i < place_num) {
        getname(linebuf);
        p_names[i] = ecalloc(strlen(linebuf) + 1, sizeof(char));
        sprintf(p_names[i], linebuf);
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
        place->covered = FALSE;
        place->next = NULL;
        if (mark >= 0)
            place->mark = mark;
        else
            place->mark = MP[-1 - mark];
#ifdef DEBUG
        printf("    place %s %d\n", linebuf, place->mark);
#endif
        ++place; ++i;
    }

    /* read rate parameters */
    i = 0;
    while (i < norp) {
        getname(linebuf);
        r_names[i] = ecalloc(strlen(linebuf) + 1, sizeof(char));
        sprintf(r_names[i], linebuf);
        fscanf(nfp, "%f", &ftemp); while (getc(nfp) != '\n');
        RP[i++] = ftemp;
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        fscanf(nfp, "%f %f", &x, &y); while (getc(nfp) != '\n');
    }
#ifdef DEBUG
    printf("  End of load_places\n");
#endif
}

void
getname(name_pr)
char 	*name_pr;
{
#define	BLANK ' '
#define	EOLN  '\0'
    unsigned		i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != NULL &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}

void
load_Pinvars() {
    extern int inv_cmp();
    struct I_header *rp;
    ROWP cp;
    int i, j;
    int num, np, mult;
    unsigned tc;

#ifdef DEBUG
    printf("  Start of load_Pinvars\n");
#endif
    fscanf(ifp, "%d\n", &no_M_invars);
#ifdef DEBUG
    printf("    no_M_invars=%d\n", no_M_invars);
#endif
    M_invars = (struct I_header *)ecalloc(no_M_invars, I_H_SIZE);
    for (rp = M_invars, i = no_M_invars; i-- ; rp++) {
        rp->p_l = NULL;
        fscanf(ifp, "%d", &num);
        rp->nouncov = num;
#ifdef DEBUG
        printf("      reading %d-th one\n", (no_M_invars - i));
#endif
        rp->rowp = cp = (ROWP)ecalloc((2 * num + 1), ROWT_SIZE);
        for (*(cp++) = (j = num), tc = 0 ; j-- ;) {
            fscanf(ifp, "%d %d", &mult, &np);
            *(cp++) = mult;
            *(cp++) = np;
            tc += mult * (unsigned)(p_list[np - 1].mark);
        }
        fscanf(ifp, "\n");
        rp->tcount = tc;
#ifdef DEBUG
        for (cp = rp->rowp, j = num + num + 1 ; j-- ; cp++)
            printf(" %d", (int)(*(cp)));
        printf("\n");
#endif
    }
    no_alloc_pl = 0; no_unused = no_M_invars;
#ifdef DEBUG
    printf("    sorting\n");
#endif
    qsort(M_invars, no_M_invars, I_H_SIZE, inv_cmp);
#ifdef DEBUG
    printf("\nEnd of load_Pinvars :\n");
    {
        int i, j; struct I_header *rp; ROWP cp;
        for (rp = M_invars, i = no_M_invars; i-- ; rp++) {
            printf("\ntc=%d, uncov=%d\n", (int)rp->tcount, (int)rp->nouncov);
            for (cp = rp->rowp, j = (int)(*(cp++)) ; j-- ; cp++) {
                if ((int)(*cp) != 1)
                    printf(" %d*", (int)(*cp));
                else
                    printf("   ");
                printf("%d", (int)(*(++cp)));
            }
        }
        printf("\n");
    }
#endif
}

int inv_cmp(p1, p2)
struct I_header *p1, * p2;
{
    if (p1->nouncov == 0)
        return ((p2->nouncov == 0) ? 0 : 1);
    if (p2->nouncov == 0)
        return (-1);
    if (p1->tcount == p2->tcount) {
        if (p1->nouncov == p2->nouncov)
            return (0);
        return ((p2->nouncov > p1->nouncov) ? 1 : -1);
    }
    return ((p2->tcount > p1->tcount) ? -1 : 1);
}

extern void alloc_places();

extern void encode_markings();

extern void gen_mark();

extern void encode_trans();


int
main(argc, argv)
int argc;
char **argv;
{
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[LINEMAX];

#ifdef DEBUG
    printf("Start\n");
#endif
    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_places();
    sprintf(filename, "%s.pin", argv[1]);
    if ((ifp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_Pinvars();
    (void) fclose(ifp);
    sprintf(filename, "%s_csimctr.c", argv[1]);
    if ((ssfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    alloc_places();
    encode_markings();
    sprintf(filename, "%s_cmark.c", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    gen_mark();
    (void) fclose(ofp);
    sprintf(filename, "%s.def", argv[1]);
    if ((dfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.cc", argv[1]);
    if ((ccfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.sc", argv[1]);
    if ((scfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s_ctran.c", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s_crate.c", argv[1]);
    if ((rafp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s_crsim.c", argv[1]);
    if ((rsfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    encode_trans();
    (void) fclose(nfp);
    (void) fclose(dfp);
    (void) fclose(ccfp);
    (void) fclose(scfp);
    (void) fclose(ofp);
    (void) fclose(rafp);
    (void) fclose(rsfp);
    (void) fclose(ssfp);
#ifdef DEBUG
    printf("End\n");
#endif
    return 0;
}

