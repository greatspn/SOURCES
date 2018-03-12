/*
 *  Paris, February 12, 1990
 *  program: strong_con.c
 *  purpose: Compute the livelocks and dead states of the TRG of a GSPN
 *  programmer: Giovanni Chiola
 *  notes:
 */


/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif



#include "const.h"
#include <stdlib.h>
#include <stdio.h>
#include "../WN/INCLUDE/compact.h"

FILE *rgfp, * auxfp, *lvlkfp;


unsigned lastan, toptan;
unsigned maxmark;


/* predeclaration */
void skip_initialmark();
void load_trg();

char *ecalloc(unsigned nitm, unsigned sz) {
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(10);
    }
    return (callptr);
}


unsigned row_l[MAX_ROW], * glp;

unsigned long frm, tmp;
unsigned nel;




void insert_row(unsigned long to) {
    unsigned *glp2 = row_l;
    register unsigned ii = glp - row_l;

#ifdef DEBUGCALLS
    fprintf(stderr, "            Start of insert_row to=%d\n", (int)to);
#endif
    for (; ii-- ; ++glp2)
        if (*glp2 == to) {
#ifdef DEBUGCALLS
            fprintf(stderr, "            End of insert_row (already present)\n");
#endif
            return;
        }
    *glp2 = to; ++nel; ++glp;
#ifdef DEBUGCALLS
    fprintf(stderr, "            End of insert_row (normal)\n");
#endif
    if (nel >= MAX_ROW) {
        fprintf(stderr, "Sorry, increase constant MAX_ROW\n");
        exit(17);
    }
}


struct Stack_def {
    unsigned tmark;
    unsigned *nx;
};

struct Stack_def *stack;
unsigned **trg;
unsigned *number;
unsigned *low;
unsigned *tag;
unsigned *scl;

struct Livelock_def {
    int tag;
    struct Livelock_def *next;
};

static struct Livelock_def *livelocks = NULL;
static struct Livelock_def *deadlocks = NULL;

int norecurrent;

int edge, lastag, norec, num, top, topstk, sch, v, w, z;
int no_livelocks, no_deadlocks;

void add_livelock(int tag, struct Livelock_def **head) {
    struct Livelock_def *new;

    new = (struct Livelock_def *)ecalloc(1, sizeof(struct Livelock_def));
    new->tag = tag;
    new->next = *head;
    if (head == (&livelocks))
        livelocks = new;
    else
        deadlocks = new;

}


void checkrecur() {
    int found, goon, count, i, low_sch;
    unsigned *u_p;

#ifdef DEBUG
    fprintf(stderr, "  Start checkrecur  sch=%d\n", sch);
#endif
    count = 0; i = sch; goon = i; found = TRUE;
    for (low_sch = low[sch] ; i  ; i = scl[i]) {

        if (low[i] < low_sch) {
#ifdef DEBUG
            fprintf(stderr, "    no\n");
#endif
            tag[i] = maxmark;
        }
    }
    for (i = sch; goon ;) {
#ifdef DEBUG
        fprintf(stderr, "     going on i=%d\n", i);
#endif
        if (low[i] >= low_sch) {
            ++count; u_p = trg[i];
            while (found && *u_p)
                found = (tag[*(u_p++)] == lastag);
        }
        i = scl[i]; goon = (i && found);
    }
    if (found) {
        if (count > 1) {
            ++no_livelocks;
            add_livelock(lastag, &livelocks);
        }
        else {
            ++no_deadlocks;
            add_livelock(lastag, &deadlocks);
        }
        lastag--; ++norec;
        norecurrent += count;
    }
    else {
        for (i = sch ; i  ; i = scl[i]) {
            tag[i] = maxmark;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End checkrecur  norec=%d\n", norec);
#endif
}


void checkstrongconn() {
#ifdef DEBUG
    fprintf(stderr, " Start checkstrongconn  toptan=%d\n", toptan);
#endif
    no_livelocks = no_deadlocks = norecurrent = norec = sch = num = 0;
    lastag = maxmark - 1;
    topstk = v = 0;
    while (num++ < toptan) {
        struct Stack_def *stk = &(stack[++topstk]);

        /* TAKE NEXT VERTEX */ do { ++v; }
        while (number[v]);
        number[v] = low[v] = num;
        /* STACK IT */ stk->tmark = v; stk->nx = trg[v];
        do { /* WHILE THE STACK IS NOT EMPTY */
            stk = &(stack[topstk]);
            top = stk->tmark; edge = *(stk->nx);
            /* FORWARD DEPTH-FIRST */
            while (edge) {   /* TAKE THE NEXT EDGE */
#ifdef DEBUG
                fprintf(stderr, "   edge=%d top=%d\n", edge, top);
#endif
                w = edge; edge = *(++(stk->nx));
#ifdef DEBUG
                fprintf(stderr, "     edge=%d w=%d\n", edge, w);
#endif
                if (number[w]) {   /* "w" IS ALREADY IN THE TREE */
                    if (tag[w] <= lastag) {
                        /* "w" DOES NOT BELONG TO ANOTHER STRONG COMPONENT */
                        if (low[top] > number[w]) {
                            low[top] = number[w];
#ifdef DEBUG
                            fprintf(stderr, "       low[%d] = %d\n", top, low[top]);
#endif
                        }
#ifdef DEBUG
                        else fprintf(stderr, "       else low\n");
#endif
                    }
#ifdef DEBUG
                    else fprintf(stderr, "       else tag\n");
#endif
                }
                else { /* "w" IS NOT YET IN THE TREE */
#ifdef DEBUG
                    fprintf(stderr, "       not yet\n");
#endif
                    low[w] = number[w] = ++num;
                    ++topstk; ++stk;
                    stk->tmark = top = w;
                    edge = *(stk->nx = trg[w]);
                }
            }
#ifdef DEBUG
            fprintf(stderr, "   backup topstk=%d num=%d top=%d, low[top]=%d\n",
                    topstk, num, top, low[top]);
#endif
            /* BACKUP */ scl[top] = sch; sch = top; tag[top] = lastag;
            if (low[top] == number[top]) {
                /* FOUND A STRONG COMPONENT */
                checkrecur();
                sch = 0;
            }
            else {
                z = (stk - 1)->tmark;
#ifdef DEBUG
                fprintf(stderr, "       else z=%d  low[z]=%d\n", z, low[z]);
#endif
                if (low[z] > low[top])
                    low[z] = low[top];
            }
            /* POP THE STACK */ --topstk; --stk;
        }
        while (topstk);
    }

#ifdef DEBUG
    fprintf(stderr, " End checkstrongconn  toptan=%d\n", toptan);
#endif
}



void init() {

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of init\n");
#endif

    fscanf(auxfp, "toptan= %d\n", &toptan);
    fscanf(auxfp, "topvan= %*d\n");
    fscanf(auxfp, "maxmark= %d\n", &maxmark);

    stack = (struct Stack_def *)ecalloc(toptan + 1,
                                        sizeof(struct Stack_def));
    trg = (unsigned **)ecalloc(toptan + 1, sizeof(unsigned *));
    number = (unsigned *)ecalloc(toptan + 1, sizeof(unsigned));
    low = (unsigned *)ecalloc(toptan + 1, sizeof(unsigned));
    tag = (unsigned *)ecalloc(toptan + 1, sizeof(unsigned));
    scl = (unsigned *)ecalloc(toptan + 1, sizeof(unsigned));
    skip_initialmark();
    load_trg();
#ifdef DEBUGCALLS
    fprintf(stderr, "  End of init\n");
#endif
}


void skip_immpath() {
    unsigned long ul;
    int plength, nt, aecsn;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of skip_immpath\n");
#endif

    load_compact(&ul, rgfp); plength = ul;
    while (plength--) {
        load_compact(&ul, rgfp); nt = ul;
        load_compact(&ul, rgfp); aecsn = ul;
#ifdef DEBUG
        fprintf(stderr, "          trans %d aecs %d\n", nt, aecsn);
#endif
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of skip_immpath\n");
#endif
}


void skip_initialmark() {
    unsigned long ul;
    int nm;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of skip_initialmark\n");
#endif
    load_compact(&ul, rgfp);
    if ((nm = ul)) {
#ifdef DEBUG
        fprintf(stderr, "      M1 with prob. 1.0\n");
#endif
    }
    else {
        for (load_compact(&ul, rgfp), nm = ul ; nm ;
                load_compact(&ul, rgfp), nm = ul) {
            skip_immpath();
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
    unsigned long ul;
    int no_trans, nt, nm;
    unsigned **trg_p = trg;
    unsigned *u_p;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of load_trg\n");
#endif
    for (load_compact(&ul, rgfp), lastan = ul ; lastan ;
            load_compact(&ul, rgfp), lastan = ul) {
        load_compact(&ul, rgfp); no_trans = ul;
#ifdef DEBUG
        fprintf(stderr, "\nfrom  #%d  (%d timed trans)\n", lastan, no_trans);
#endif
        nel = 0; glp = row_l;
        while (no_trans--) {
            load_compact(&ul, rgfp); nt = ul;
            load_compact(&ul, rgfp);
#ifdef DEBUG
            fprintf(stderr, "  timed #%d  (enabl=%d)\n", nt, ul);
#endif
            load_compact(&ul, rgfp);
            if ((nm = ul)) {
#ifdef DEBUG
                fprintf(stderr, "    (T) to  #%d\n", nm);
#endif
                insert_row(nm);    /* INCREMENTS "nel" and "glp" */
            }
            else {
                for (load_compact(&ul, rgfp), nm = ul ; nm ;
                        load_compact(&ul, rgfp), nm = ul) {
                    skip_immpath();
#ifdef DEBUG
                    fprintf(stderr, "        (V) to  #%d\n", nm);
#endif
                    insert_row(nm);    /* INCREMENTS "nel" and "glp" */
                }
            }
        }
        *(++trg_p) = u_p = (unsigned *)ecalloc(nel + 1, sizeof(unsigned));
        glp = row_l;
        while (nel--) {
            *(u_p++) = *(glp++);
        }
        *u_p = 0;
        number[lastan] = tag[lastan] = 0;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of load_trg\n");
#endif
}


void store_livelocks() {
    int i;
    unsigned long ul;

    store_compact((unsigned long)no_livelocks, lvlkfp);
    store_compact((unsigned long)no_deadlocks, lvlkfp);
#ifdef DEBUGCALLS
    fprintf(stderr, "%d livelocks %d deadlocks\n",
            no_livelocks, no_deadlocks);
#endif
    if (no_livelocks && no_deadlocks) {
        while (livelocks != NULL) {
            ul = maxmark - livelocks->tag;
#ifdef DEBUGCALLS
            fprintf(stderr, "      live #%d\n", ul);
#endif
            store_compact(ul, lvlkfp);
            livelocks = livelocks->next;
        }
        while (deadlocks != NULL) {
            ul = maxmark - deadlocks->tag;
#ifdef DEBUGCALLS
            fprintf(stderr, "      dead #%d\n", ul);
#endif
            store_compact(ul, lvlkfp);
            deadlocks = deadlocks->next;
        }
    }
    store_compact((unsigned long)(toptan - norecurrent), lvlkfp);
#ifdef DEBUGCALLS
    fprintf(stderr, "%d markings non recurrent\n",
            toptan - norecurrent);
#endif
    if (norec == 1 && no_livelocks == 1 && norecurrent == toptan) {
        fprintf(stderr, "\nThe initial marking is a Home State\n");
        return;
    }
    fprintf(stderr, "\n***   The initial marking is NOT a Home State !!!\n");
    if (norec > 1 && no_livelocks)
        fprintf(stderr,
                "\n***   The TRG contains %d Livelocks and %d Deadlocks !!!\n",
                no_livelocks, no_deadlocks);
    else if (norec == 1 && no_livelocks == 1)
        fprintf(stderr, "\nThe TRG contains a Home Space\n");
    else
        fprintf(stderr,
                "\n***   The TRG contains %d Deadlocks and NO Livelock !!!\n",
                no_deadlocks);
    for (i = 0 ; i++ < toptan ;) {
        ul = maxmark - tag[i];
#ifdef DEBUGCALLS
        fprintf(stderr, "   marking %d in livelock %d\n", i, ul);
#endif
        store_compact(ul, lvlkfp);
    }
}


static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[100];


int main(int argc,
         char **argv,
         char **envp) {

#ifdef DEBUG
    fprintf(stderr, "Start of strong_con\n");
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
    init();
    (void)fclose(auxfp);
    (void) fclose(rgfp);

    checkstrongconn();

    sprintf(filename, "%s.livlck", argv[1]);
    if ((lvlkfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    store_livelocks();
    (void) fclose(lvlkfp);

#ifdef DEBUG
    fprintf(stderr, "End of strong_con\n");
#endif
    return 0;
}

