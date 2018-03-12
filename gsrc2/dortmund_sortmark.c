/*
 *  Torino, November 23, 1991
 *  program: dortmund_sortmark.c
 *  purpose: Rearrange marking order and produce matrix in the format
 *           requested by the new aggregation technique specified
 *           by Dortmund University.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) Each class within the submodel must be identified by a layer.
 *      Each layer must contain all places and the "arrival" transition
 *      for the corresponding class in the subsystem.
 */


/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "const.h"
#include "../WN/INCLUDE/compact.h"

#include "grg.h"

int show_TRG, show_TRS, show_RTRG;

/* void (*terminate)(); */
void terminate() { exit(1); }

unsigned place_num, group_num, trans_num;

FILE *nfp, * rgfp, * tmfp, * dbfp;


struct Codbyt_descr {
    unsigned char shift;
    unsigned char mask;
    unsigned short num_byte;
    unsigned short link;
};

struct Place_descr {
    unsigned char subsystem;
    unsigned short lbound;
    unsigned short rubound;
    struct Codbyt_descr cod;
    char *name;
};

/*  PLACE ARRAY  */ struct Place_descr *p_list;

unsigned sub_num;

/* CODING BYTES */ unsigned no_cod;

/* CODED MARKING */ unsigned char **CM;

struct Byte_descr {
    unsigned char noitems;
    unsigned char max_conf;
    unsigned short first_item;
};

struct Byte_descr *encoding;

struct Enabl_descr {
    unsigned char notestge;
    unsigned char notestlt;
    unsigned short nobyte;
    struct Test_descr *test_list;
};

struct Trans_descr {
    unsigned char noecs;
    char *name;
};

/* TRANSITION ARRAY */ struct Trans_descr *t_list;

/* DECODED MARKING */ unsigned short *DM;

unsigned lastan, toptan = 0, loadedtan = 0;


struct Msort_descr {
    unsigned num;
    double diag;
    struct Msort_descr *next;
};

struct Msort_descr *Msort;

union pop {
    unsigned ntok;
    unsigned *distr;
};

struct List_descr {
    union pop class;
    unsigned num_mark;
    struct Msort_descr *m_last;
    struct List_descr *prev;
    struct List_descr *next;
};

struct List_descr *centerlist = NULL;
int list_bal = 0;
int max_classes = 0;

char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *       calloc();
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        /*        (*terminate)(); */
        terminate();
    }
    return (callptr);
}


int classify_mark(Mp, class)
unsigned char *Mp;
int class;
{
    struct Place_descr *place_p = p_list;
    unsigned np, nb, val;
    int nsubsys = 0;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of classify_mark\n");
#endif
    for (np = place_num ; np-- ; place_p++) {
        if (place_p->subsystem
                && ((!class) || (place_p->subsystem == class))
                && (nb = place_p->cod.num_byte)) {
            nb--;
            val = Mp[nb] & place_p->cod.mask;
            val = val >> (place_p->cod.shift);
            val += place_p->lbound;
            nsubsys += val;
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of classify_mark <- %d\n", nsubsys);
#endif
    return (nsubsys);
}


void decode_mark(Mp, dmp)
unsigned char *Mp;
unsigned short *dmp;
{
    struct Place_descr *place_p = p_list;
    unsigned np, nb, val;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of decode_mark\n");
#endif
    for (np = place_num ; np-- ; place_p++) {
        if ((nb = place_p->cod.num_byte)) {
            nb--;
            val = Mp[nb] & place_p->cod.mask;
            val = val >> (place_p->cod.shift);
            val += place_p->lbound;
            *(++dmp) = val;
        }
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of decode_mark\n");
#endif
}


double *dval;

unsigned long ul;

void init() {
    int ii, jj, kk, ll, mm, nn, np, ns, nt, ss;
    struct Byte_descr *byte_p;
    struct Place_descr *place_p;
    unsigned char *ucp;
    unsigned short *usp;
    struct Trans_descr *trans_p;
    int na, nb, nge, nlt, nc, n1, n2, n3, n4;
    char linebuf[LINEMAX];
    int jj2;
    struct Codbyt_descr *cods_p;
    unsigned short **uspp;
    double *d_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif

    /* read number of objects in the net */
    fscanf(nfp, "%d %d %d %d\n", &sub_num, &place_num, &group_num, &trans_num);

    CM = (unsigned char **)ecalloc(toptan + 1, sizeof(unsigned char *));
    Msort = (struct Msort_descr *)ecalloc(toptan + 1,
                                          sizeof(struct Msort_descr));
    DM = (unsigned short *)ecalloc(place_num + 1, sizeof(unsigned short));
    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));

    for (ii = 0, place_p = p_list; ++ii <= place_num; place_p++) {
        place_p->cod.num_byte = 0;
    }

    /* read codings */
    for (nn = 0 ; nn < sub_num ; nn++) {
        fscanf(nfp, "%d %d\n", &ii, &jj2);
#ifdef DEBUG
        fprintf(stderr, "  subnet #%d (%d bytes):\n",
                nn, jj2);
#endif
        if (nn == 0) {
            encoding = (struct Byte_descr *)ecalloc(jj2, sizeof(struct Byte_descr));
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
        fscanf(nfp, "%d %d %d %d %d %d %d %s\n",
               &np, &jj, &kk, &ll, &nt, &nn, &mm, linebuf);
        place_p->lbound = nt;
        place_p->rubound = nn - nt;
        place_p->subsystem = FALSE;
        place_p->name = ecalloc(strlen(linebuf) + 1, 1);
        sprintf(place_p->name, linebuf);
#ifdef DEBUG
        fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                ii, place_p->cod.mask, place_p->cod.shift, place_p->cod.num_byte);
        fprintf(stderr, "            lbound=%d rubound=%d notok=%d link=%d\n",
                place_p->lbound, place_p->rubound, mm, place_p->cod.link);
#endif
    }

    /* read doubles */
    load_compact(&ul, dbfp);
    d_p = dval = (double *)ecalloc(ul + 1, sizeof(double));
    while (ul--)
        load_double(++d_p, dbfp);
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}


void getname(name_pr)
char   *name_pr;
{
#define BLANK ' '
#define EOLN  '\0'
    short               i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK && (*name_pr) != '\n' &&
            (*name_pr) != NULL && i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    if (*name_pr != BLANK) {
        char c;
        for (c = fgetc(nfp) ;
                c != BLANK && c != '\n' && c != NULL ;
                c = fgetc(nfp));
    }
    (*name_pr) = EOLN;
}


double *class_rates;
unsigned *max_pop_vector;


void init_subsystem() {
    int i, nomp, place_num, norp, trans_num, group_num, ll;
    struct Place_descr *place_p;
    char linebuf[LINEMAX];
    double *RP;
#define VBAR '|'

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init_subsystem\n");
#endif


    /* skip first line containing '|0|' */
    fgets(linebuf, LINEMAX, nfp);
    /* skip comment */
    for (; ;) {
        fgets(linebuf, LINEMAX, nfp);
        if (linebuf[0] == VBAR) break;
    }
    /* read number of objects in the net */
    fscanf(nfp, "f %d %d %d %d %d", &nomp, &place_num, &norp, &trans_num,
           &group_num);
    while (getc(nfp) != '\n') {}

    /* skip marking parameters */
    for (i = 0 ; i < nomp ; ++i) {
        getname(linebuf);
        fscanf(nfp, "%*d"); while (getc(nfp) != '\n');
    }

    /* read layers of places */
    for (i = 0, place_p = p_list; i < place_num ; ++i, ++place_p) {
        getname(linebuf);
        fscanf(nfp, "%*d %*f %*f %*f %*f %d", &ll);
        while (ll) {
            place_p->subsystem = ll;
            if (ll > max_classes)
                max_classes = ll;
#ifdef DEBUG
            fprintf(stderr, "      place %d (%s) belongs to layer %d\n",
                    i + 1, p_list[i].name, ll);
#endif
            fscanf(nfp, "%d", &ll);
        }
        while (getc(nfp) != '\n');
    }
    RP = (double *)ecalloc(norp + 1, sizeof(double));
    class_rates = (double *)ecalloc(max_classes + 1, sizeof(double));
    max_pop_vector = (unsigned *)ecalloc(max_classes + 1, sizeof(unsigned));
    for (i = 1 ; i <= max_classes ; ++i) {
        class_rates[i] = 0.0;
        max_pop_vector[i] = 0;
    }

    /* read rate params */
    for (i = 0 ; i++ < norp ;) {
        getname(linebuf);
        fscanf(nfp, "%lf", RP + i);
        while (getc(nfp) != '\n');
    }

    /* skip priorities */
    for (i = 0 ; i++ < group_num ;) {
        getname(linebuf);
        while (getc(nfp) != '\n');
    }

    /* read class_rates */
    for (i = 0 ; i++ < trans_num ;) {
        int noarcs, nopoints;
        double ftemp;
        getname(linebuf);
        fscanf(nfp, "%lf %*d %*d %d %*d %*f %*f %*f %*f %*f %*f %d",
               &ftemp, &noarcs, &ll);
        while (ll) {
            if (ll > max_classes) {
                fprintf(stderr,
                        "\nERROR: Inconsistent layers partition: t%d (%s) l=%d\n",
                        i, linebuf, ll);
                exit(1);
            }
            if (ftemp <= 0.0) {
                int k = (int)((-ftemp) + 0.5);
                if (k > norp) {
                    fprintf(stderr,
                            "\nERROR: t%d (%s) has marking dependent firing rate\n",
                            i, linebuf);
                    exit(1);
                }
                ftemp = RP[k];
            }
            class_rates[ll] = ftemp;
            fscanf(nfp, "%d", &ll);
        }
        while (getc(nfp) != '\n');
        while (noarcs--) {
            fscanf(nfp, "%*d %*d %d", &nopoints); while (getc(nfp) != '\n');
            while (nopoints--)
                while (getc(nfp) != '\n');
        }
        fscanf(nfp, "%d", &noarcs); while (getc(nfp) != '\n');
        while (noarcs--) {
            fscanf(nfp, "%*d %*d %d", &nopoints); while (getc(nfp) != '\n');
            while (nopoints--)
                while (getc(nfp) != '\n');
        }
        fscanf(nfp, "%d", &noarcs); while (getc(nfp) != '\n');
        while (noarcs--) {
            fscanf(nfp, "%*d %*d %d", &nopoints); while (getc(nfp) != '\n');
            while (nopoints--)
                while (getc(nfp) != '\n');
        }
    }
    free(RP);

#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init_subsystem\n");
#endif
}


struct List_descr *newlist(Ms_p, pop_distr)
struct Msort_descr *Ms_p;
unsigned *pop_distr;
{
    struct List_descr *nl_p;
    int ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of newlist\n");
#endif
    nl_p = (struct List_descr *)ecalloc(1, sizeof(struct List_descr));
    if (max_classes > 1) {
#ifdef DEBUG
        fprintf(stderr, "        ... max_classes=%d\n", max_classes);
#endif
        nl_p->class.distr = (unsigned *)ecalloc(max_classes + 1,
                                                sizeof(unsigned));
        for (ii = 0 ; ii <= max_classes ; ++ii) {
            nl_p->class.distr[ii] = pop_distr[ii];
            if (pop_distr[ii] > max_pop_vector[ii])
                max_pop_vector[ii] = pop_distr[ii];
        }
    }
    else {
#ifdef DEBUG
        fprintf(stderr, "        ... single class\n");
#endif
        nl_p->class.ntok = *pop_distr;
        if (*pop_distr > *max_pop_vector)
            *max_pop_vector = *pop_distr;
    }
    nl_p->m_last = Ms_p;
    Ms_p->next = Ms_p;
    nl_p->num_mark = 1;
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of newlist\n");
#endif
    return (nl_p);
}


void addlist(l_p, Ms_p)
struct List_descr *l_p;
struct Msort_descr *Ms_p;
{
#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of addlist\n");
#endif
    Ms_p->next = l_p->m_last->next;
    l_p->m_last->next = Ms_p;
    l_p->m_last = Ms_p;
    ++(l_p->num_mark);
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of addlist\n");
#endif
}


int compare_classes(class, pop)
union pop class;
unsigned *pop;
{
    int nc;

    if (max_classes == 1) {
        if (class.ntok == *pop)
            return (0);
        if (class.ntok < *pop)
            return (-1);
        /* else */
        return (1);
    }
    for (nc = 0 ; nc <= max_classes ; ++nc) {
        if (class.distr[nc] < pop[nc])
            return (-1);
        if (class.distr[nc] > pop[nc])
            return (1);
    }
    return (0);
}


void load_marking() {
    unsigned char *ucp;
    int nb, np, prev;
    struct Place_descr *place_p;
    struct Msort_descr *Ms_p;
    int subsys_class;
    struct List_descr *test_l;
    struct List_descr *nl_p;
    int test_v;
    unsigned *pop_vector;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of load_marking\n");
#endif
    pop_vector = (unsigned *) ecalloc(max_classes + 1, sizeof(unsigned));
    for (loadedtan = 0 ; toptan > loadedtan ;) {
        ucp = CM[++loadedtan] = (unsigned char *)ecalloc(no_cod,
                                sizeof(unsigned char));
#ifdef DEBUG
        fprintf(stderr, "    loading mark #%d\n", loadedtan);
#endif
        for (nb = no_cod ; nb-- ; * (ucp++) = getc(tmfp)) {}
        Ms_p = &(Msort[loadedtan]);
        if (max_classes == 1) {
            *pop_vector = subsys_class = classify_mark(CM[loadedtan], 0);
        }
        else {
            int class;

            *pop_vector = 0;
            for (class = 0 ; class++ < max_classes ;)
                pop_vector[class] = classify_mark(CM[loadedtan], class);
        }
        if ((test_l = centerlist) == NULL) {
            centerlist = newlist(Ms_p, pop_vector);
            centerlist->prev = NULL;
            centerlist->next = NULL;
        }
        else {
            if ((test_v = compare_classes(test_l->class, pop_vector)) == 0) {
                addlist(test_l, Ms_p);
            }
            else {
                if (test_v < 0) {
                    while (test_l->next != NULL && (test_v =
                                                        compare_classes(test_l->next->class, pop_vector))
                            < 0)
                        test_l = test_l->next;
                    if (test_v == 0)
                        addlist(test_l->next, Ms_p);
                    else {
                        nl_p = newlist(Ms_p, pop_vector);
                        nl_p->prev = test_l;
                        if ((nl_p->next = test_l->next) != NULL)
                            test_l->next->prev = nl_p;
                        test_l->next = nl_p;
                        if ((++list_bal) > 1) {
                            list_bal -= 2;
                            centerlist = centerlist->next;
                        }
                    }
                }
                else { /* if ( test_v > 0 ) */
                    while (test_l->prev != NULL && (test_v =
                                                        compare_classes(test_l->prev->class, pop_vector))
                            > 0)
                        test_l = test_l->prev;
                    if (test_v == 0)
                        addlist(test_l->prev, Ms_p);
                    else {
                        nl_p = newlist(Ms_p, pop_vector);
                        nl_p->next = test_l;
                        if ((nl_p->prev = test_l->prev) != NULL)
                            test_l->prev->next = nl_p;
                        test_l->prev = nl_p;
                        if ((--list_bal) < -1) {
                            list_bal += 2;
                            centerlist = centerlist->prev;
                        }
                    }
                }
            }
        }
    }
    while (centerlist->prev != NULL) {
        centerlist = centerlist->prev;
        list_bal += 2;
    }
    ++list_bal;
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of load_marking: %d classes\n", list_bal);
#endif
}


static char   *can_t_open = "Can't open file %s for %c\n";

void save_sorted(filename)
char *filename;
{
    extern FILE *fopen();
    FILE *matfp;
    unsigned char *ucp;
    int nb, np, prev;
    struct Place_descr *place_p;
    struct Msort_descr *Ms_p;
    int subsys_class;
    struct List_descr *nl_p;
    int test_v;
    int M_index;
    int nc;
    int nel;
    double w;
    int cur_mark;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of save_sorted\n");
#endif
    fprintf(rgfp, "%d\n", max_classes);
    if (max_classes == 1) {
        fprintf(rgfp, "%lg\n%d\n", class_rates[1], *max_pop_vector);
    }
    else {
        for (nc = 0 ; nc++ < max_classes ;)
            fprintf(rgfp, "%lg\n", class_rates[nc]);
        for (nc = 0 ; nc++ < max_classes ;)
            fprintf(rgfp, "%d\n", max_pop_vector[nc]);
    }
    fprintf(tmfp, "The TRS contains %d markings and %d aggregates\n",
            toptan, list_bal);
    for (nl_p = centerlist, cur_mark = 0 ; nl_p != NULL ;
            nl_p = nl_p->next) {
        fprintf(rgfp, "%d\n", nl_p->num_mark);
        if (max_classes == 1) {
#ifdef DEBUG
            fprintf(stderr, "    aggregate with %d tokens\n", nl_p->class.ntok);
#endif
            fprintf(tmfp, "\nAggregate of %d markings with %d tokens:\n",
                    nl_p->num_mark, nl_p->class.ntok);
        }
        else {
#ifdef DEBUG
            fprintf(stderr, "\nAggregate of %d markings with tokens (",
                    nl_p->num_mark);
#endif
            fprintf(tmfp, "\nAggregate of %d markings with tokens (",
                    nl_p->num_mark);
            for (nc = 0 ; nc++ < max_classes ;) {
#ifdef DEBUG
                fprintf(stderr, "%d", nl_p->class.distr[nc]);
#endif
                fprintf(tmfp, "%d", nl_p->class.distr[nc]);
                if (nc < max_classes) {
#ifdef DEBUG
                    fprintf(stderr, ",");
#endif
                    fprintf(tmfp, ",");
                }
                else {
#ifdef DEBUG
                    fprintf(stderr, ") tokens :\n");
#endif
                    fprintf(tmfp, ") tokens :\n");
                }
            }
        }
        for (Ms_p = nl_p->m_last->next ; ; Ms_p = Ms_p->next) {
            M_index = Ms_p - Msort;
            Ms_p->num = ++cur_mark;
#ifdef DEBUG
            fprintf(stderr, "      marking # %d (now # %d)\n", M_index, cur_mark);
#endif
            fprintf(tmfp, "  Marking #%d (now #%d): ", M_index, cur_mark);
            decode_mark(CM[M_index], DM);
            fprintf(tmfp, " [ ");
            for (prev = 0, np = 0, place_p = p_list ; np++ < place_num ;
                    place_p++) {
                if ((nb = DM[np])) {
                    if (prev)
                        fprintf(tmfp, ", ");
                    prev = 1;
                    if (nb == 1)
                        fprintf(tmfp, place_p->name);
                    else
                        fprintf(tmfp, "%d @ %s", nb, place_p->name);
                }
            }
            if (! prev)
                fprintf(tmfp, "... no tokens ...");
            fprintf(tmfp, " ]\n");

            if (Ms_p == nl_p->m_last)
                break;
        }
    }
    if ((matfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_compact(&ul, matfp);
    for (Ms_p = Msort + 1 ; ul-- ; ++Ms_p) {
        load_compact(&ul, matfp); nel = ul;
        load_compact(&ul, matfp); /* diagonal */
        if ((w = dval[ul]) > 0.0)
            w = 1.0 / w;
        Ms_p->diag = w;
        while (nel--) {
            load_compact(&ul, matfp); /* from */
            load_compact(&ul, matfp); /* rate */
        }
    }
    (void)fclose(matfp);
    for (nl_p = centerlist ; nl_p != NULL ; nl_p = nl_p->next) {

        if ((matfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
        load_compact(&ul, matfp);
        lastan = 1;
        for (Ms_p = nl_p->m_last->next ; ; Ms_p = Ms_p->next) {
            M_index = Ms_p - Msort;
#ifdef DEBUG
            fprintf(stderr, "      marking # %d (now #%d)\n", M_index, Ms_p->num);
#endif
            while (lastan++ < M_index) {
                load_compact(&ul, matfp); nel = ul;
                load_compact(&ul, matfp); /* diagonal */
                while (nel--) {
                    load_compact(&ul, matfp); /* from */
                    load_compact(&ul, matfp); /* rate */
                }
            }
            load_compact(&ul, matfp); nel = ul;
            fprintf(rgfp, "%d\n", nel);
            load_compact(&ul, matfp); w = - Ms_p->diag; /* diagonal */
            fprintf(rgfp, "%lg\n", w);
            while (nel--) {
                load_compact(&ul, matfp); /* from */
                w = Msort[ul].diag; /* denormalization */
                ul = Msort[ul].num;
                fprintf(rgfp, "%d\n", ul);
                load_compact(&ul, matfp); /* rate */
                fprintf(rgfp, "%lg\n", dval[ul] * w);
            }

            if (Ms_p == nl_p->m_last)
                break;
        }
        (void)fclose(matfp);
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of save_sorted\n");
#endif
}


char  filename[100];

int main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
    extern FILE *fopen();
    int no_trans;
    unsigned long ul;
    int nm, nt;
    unsigned short *usp;

#ifdef DEBUG
    fprintf(stderr, "Start of dortmund_sortmark\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    fscanf(rgfp, "toptan= %d\n", &toptan);
    (void) fclose(rgfp);
    sprintf(filename, "%s.grg", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.doubles", argv[1]);
    if ((dbfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init();
    (void)fclose(nfp);
    (void)fclose(dbfp);
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init_subsystem();
    (void)fclose(nfp);
    sprintf(filename, "%s.ctrs", argv[1]);
    if ((tmfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_marking();
    (void) fclose(tmfp);
    sprintf(filename, "%s.dortmund_markings", argv[1]);
    if ((tmfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s.dortmund_matrix", argv[1]);
    if ((rgfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s.emc", argv[1]);
    save_sorted(filename);
    (void) fclose(tmfp);
    (void) fclose(rgfp);

#ifdef DEBUG
    fprintf(stderr, "End of dortmund_sortmark\n");
#endif
    return 0;
}

