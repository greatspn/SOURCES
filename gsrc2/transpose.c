/*
 *  Torino, September 9, 1987
 *  program: transpose.c
 *  purpose: Transpose of the EMC of a GSPN.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) NOT YET COMPLETELY DEBUGGED
 */

#include <stdlib.h>
#include <stdio.h>
#include "../WN/INCLUDE/compact.h"

FILE *emcfp, * sspfp;

/*
#define DEBUG
*/

#define MRKSLOT 100

struct grph_itm_l {
    unsigned long	fm;
    unsigned short	rt;
    struct grph_itm_l 	*link;
};

struct grph_itm_l *free_p = NULL;

struct grph_itm_l **tail;
unsigned short *weight;
unsigned long top_tan = 0, max_tan = 0;
unsigned long limit_tan = 0;


struct grph_itm_l *new_grph() {
    struct grph_itm_l *g_p = free_p;

    if (g_p == NULL) {
        struct grph_itm_l *ret_p;
        register unsigned ii = MRKSLOT;
        if ((g_p = (struct grph_itm_l *)calloc(ii,
                                               sizeof(struct grph_itm_l))) == NULL) {
            fprintf(stderr, "calloc: couldn't get enough memory\n");
            exit(50);
        }
        ret_p = g_p;
        free_p = ++g_p;
        for (--ii ; --ii ; ++g_p)
            g_p->link = g_p + 1;
        g_p->link = NULL;
        return (ret_p);
    }
    free_p = g_p->link;
    return (g_p);
}

void insert_graph(unsigned long from, unsigned long to, unsigned short rate) {
    struct grph_itm_l *g_p;

    if (to > max_tan) {    /* NEW COLUMN */
        register unsigned ii = limit_tan - max_tan;
        struct grph_itm_l **g_pp = tail + max_tan;
        unsigned long incr = ((ii > MRKSLOT) ? MRKSLOT : ii);
        incr = ii + to - max_tan;
        for (++ii, max_tan = to + incr ; --ii ;)
            *++g_pp = NULL;
        g_pp = tail + to;  *g_pp = g_p = new_grph();
        g_p->link = g_p; g_p->fm = from; g_p->rt = rate;
        return;
    }
    if ((g_p = tail[to]) == NULL) {    /* INSERT FIRST ITEM */
        tail[to] = g_p = new_grph();
        g_p->link = g_p; g_p->fm = from; g_p->rt = rate;
        return;
    }
    if (g_p->fm < from) {    /* TAIL INSERTION */
        struct grph_itm_l *g_p2 = new_grph();
        tail[to] = g_p2;
        g_p2->link = g_p->link; g_p2->fm = from; g_p2->rt = rate;
        g_p->link = g_p2;
        return;
    }
    {
        struct grph_itm_l *g_p2 = g_p->link;
        while (g_p2->fm < from) {
            g_p = g_p2;  g_p2 = g_p2->link;
        }
        g_p = g_p->link = new_grph();
        g_p->link = g_p2; g_p->fm = from; g_p->rt = rate;
    }
}



unsigned long tmp;
unsigned long nel;


void load_mat() {
    unsigned short rate;
    unsigned long rr;
    register unsigned i;

#ifdef DEBUG
    fprintf(stderr, "  Start of load_mat\n");
#endif
    ++top_tan;
    load_compact((&tmp), stdin);
    weight[top_tan] = tmp;
    for (i = nel ; i-- ;) {
        load_compact((&tmp), stdin);
        load_compact((&rr), stdin); rate = rr;
        insert_graph(top_tan, tmp, rate);
    }
#ifdef DEBUG
    fprintf(stderr, "  End of load_mat\n");
#endif
}


unsigned short *w_p;
struct grph_itm_l **t_p;

void store_transpose() {
    unsigned long tmp = *w_p;
    unsigned long nel = 0;
    struct grph_itm_l *g_p = *t_p;

    if (g_p == NULL) {
        store_compact(nel, emcfp);
        return;
    }
    for (++nel, g_p = g_p->link ; g_p != *t_p ;
            ++nel, g_p = g_p->link);
    store_compact(nel, emcfp);
    store_compact(tmp, emcfp);
    for (g_p = g_p->link ; nel-- ; g_p = g_p->link) {
        store_compact(g_p->fm, emcfp);
        tmp = g_p->rt;  store_compact(tmp, emcfp);
    }
}


int main(int argc, char **argv) {
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[256];
    unsigned long ii;
    double dd;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
#endif
    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.emc", argv[1]);
    if ((emcfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    load_compact(&limit_tan, stdin);
    if ((tail = (struct grph_itm_l **)calloc(limit_tan + 1,
                sizeof(struct grph_itm_l *))) == NULL ||
            (weight = (unsigned short *)calloc(limit_tan + 1,
                      sizeof(unsigned short))) == NULL) {
        fprintf(stderr, "calloc: couldn't get enough memory\n");
        exit(50);
    }

    for (load_compact(&nel, stdin) ; nel > 0 ;
            load_compact(&nel, stdin)) {
        load_mat();
    }
    store_compact(limit_tan, emcfp);
    for (ii = top_tan, w_p = &(weight[1]), t_p = &(tail[1]) ;
            ii-- ; ++w_p, ++t_p) {
        store_transpose();
    }
    nel = 0;  store_compact(nel, emcfp);
    (void) fclose(emcfp);
    sprintf(filename, "%s.epd", argv[1]);
    if ((sspfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    dd = 0.0; store_double(&dd, sspfp);
    dd = -1.0; store_double(&dd, sspfp);
    dd = 1.0 / top_tan;
    for (ii = top_tan ; ii-- ;)
        store_double(&dd, sspfp);
    (void) fclose(sspfp);
#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
    return 0;
}
