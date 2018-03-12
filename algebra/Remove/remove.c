//
//  Utility for Compositional Algebra of SWN Petri-Nets
//

#include<string.h>

#include "global.h"
#include "layer.h"
#include "load.h"
#include "save.h"
#include "alloc.h"

//#define DEBUG

extern FILE *yyin;
FILE *fp;
char *pfname;
int yyparse();

// Structure to describe labeling of transition and places
struct Desc {
    char *wholetag;             // whole tag
    char *tag;                  // tag of transition (first part)
    char **labels;              // labels of transition
    int nl;                     // number of labels
    char *end;                  // string to distinguish
    struct Desc *next;
};

// Structure to contain labels to be synchronized
struct RestSet {
    char **labels;     // Labels that are synchronized
    int nl;            // number of labels
};

// =======
// Globals
// =======
struct net_object *Op1;     // operand
int Ntr1;            // Number of transitions in the nets, result
int Npl1;            // Number of places in the nets, result
struct Desc *Op1TrD;    // to describe transition labelling
struct Desc *Op1PlD;    // to describe place labelling
struct RestSet PlaceRest;        // Labels of places to be sybchronised
struct RestSet TransRest;        // Labels of transitions to be sybchronised
int Tr_or_Pl;                    // Tr. or Pl under parsing
int Tr_Rest, Pl_Rest;            // Is there any restrictions?
struct Desc *PDesc;              // structure filled by parser
char *lexstr;                    // string analysed by parser

void yyerror(char *s) {
    printf("%s\n", s);
}

// -----------------
// memory allocation
// -----------------
void mem_alloc(void) {
    Op1 = (struct net_object *) emalloc(NETOBJ_SIZE);
    Op1->comment = NULL;
    Op1->mpars = NULL;
    Op1->places = NULL;
    Op1->groups = NULL;
    Op1->results = NULL;
    Op1->texts = NULL;
    Op1->next = NULL;
    Op1->trans = NULL;
    Op1->lisps = NULL;
    Op1->arcs = NULL;
    Op1->rpars = NULL;
}

// -----------
// memory free
// -----------
void mem_free(void) {
    // TODO: THE WHOLE NETSTRUCUTURE SHOULD BE DESTROYED !!!;

    free(Op1);
}

// ------------------------------
// Goto transition given by index
// ------------------------------
struct trans_object *GotoTrIndex(int index, struct net_object *n) {
    struct trans_object *t;
    struct group_object *g;
    int i;

    i = 0;
    t = n->trans;
    while (t != NULL) {
        //printf(" %d, %s\n", i, t->tag);
        if (i == index) return t;
        t = t->next;
        i++;
    }
    g = n->groups;
    while (g != NULL) {
        t = g->trans;
        while (t != NULL) {
            //printf(" %d, %s\n", i, t->tag);
            if (i == index) return t;
            t = t->next;
            i++;
        }
        g = g->next;
    }

    return t;
}


// -------------------------------------------------------
// Filling structures to describe labelling of transitions
// -------------------------------------------------------
struct Desc *FillDescTr(struct trans_object *tr, struct group_object *group) {
    struct trans_object *t = tr;
    struct group_object *gr = group;
    struct Desc *d = NULL, *tmp = NULL;
    int first = 1, count = 0, i;

    Tr_or_Pl = 1;

    // goto the first non-empty group;
    while (gr != NULL && gr->trans == NULL) gr = gr->next;

    if (t == NULL) {
        if (gr != NULL) {
            t = gr->trans;
            gr = gr->next;
        }
    }
    while (t != NULL) {
        if (first) {
            d = (struct Desc *) emalloc(sizeof(struct Desc));
            tmp = d;
            first = 0;
        }
        else {
            tmp->next = (struct Desc *) emalloc(sizeof(struct Desc));
            tmp = tmp->next;
        }
        tmp->nl = 0;
        tmp->labels = NULL;
        tmp->end = NULL;

        // copy whole tag;
        tmp->wholetag = (char *)emalloc(sizeof(char) * (strlen(t->tag) + 1));
        strcpy(tmp->wholetag, t->tag);

        // parse whole tag;

        fp = fopen(pfname, "w");
        fprintf(fp, "%s", t->tag);
        fclose(fp);
        fp = fopen(pfname, "r");
        yyin = fp;

        PDesc = tmp;
        yyparse();
        fclose(fp);

#ifdef DEBUG
        printf(" %d. Full Tag: '%s'\n", count, t->tag);
        printf("  Tag: '%s'\n  NofLabels: %d, Labels: ", tmp->tag, tmp->nl);
        if (tmp->labels != NULL)
            for (i = 0; i < tmp->nl; i++) printf("'%s', ", tmp->labels[i]);
        printf("\n  End: ");
        if (tmp->end != NULL) printf("'%s'", tmp->end);
        printf("\n");
        count++;
#endif // DEBUG

        if (t->next == NULL && gr != NULL) {
            t = gr->trans;
            gr = gr->next;
            while (t == NULL && gr != NULL) {
                t = gr->trans;
                gr = gr->next;
            }
        }
        else {
            t = t->next;
        }
    }
    if (d != NULL) tmp->next = NULL;
    return d;
}


// --------------------------------------------------
// Filling structures to describe labelling of places
// --------------------------------------------------
struct Desc *FillDescPl(struct place_object *pl) {
    struct place_object *t = pl;
    struct Desc *d = NULL, *tmp = NULL;
    int first = 1, count = 0, i;

    Tr_or_Pl = 0;

    while (t != NULL) {
        if (first) {
            d = (struct Desc *) emalloc(sizeof(struct Desc));
            tmp = d;
            first = 0;
        }
        else {
            tmp->next = (struct Desc *) emalloc(sizeof(struct Desc));
            tmp = tmp->next;
        }
        tmp->nl = 0;
        tmp->labels = NULL;
        tmp->end = NULL;

        // copy whole tag;
        tmp->wholetag = (char *)emalloc(sizeof(char) * (strlen(t->tag) + 1));
        strcpy(tmp->wholetag, t->tag);

        // parse whole tag;

        fp = fopen(pfname, "w");
        fprintf(fp, "%s", t->tag);
        fclose(fp);
        fp = fopen(pfname, "r");
        yyin = fp;

        PDesc = tmp;
        yyparse();
        fclose(fp);

#ifdef DEBUG
        printf(" %d.  FullTag: %s\n  Tag: '%s'\n  NofLabels: %d, Labels: ", count, tmp->wholetag, tmp->tag, tmp->nl);
        if (tmp->labels != NULL)
            for (i = 0; i < tmp->nl; i++) printf("'%s', ", tmp->labels[i]);
        printf("\n  End: ");
        if (tmp->end != NULL) printf("'%s'", tmp->end);
        printf("\n");
        count++;
#endif // DEBUG

        t = t->next;
    }
    if (d != NULL) tmp->next = NULL;
    return d;
}


// ------------------------
// Count Places in Net
// ------------------------
int CountPlaces(struct net_object *n) {
    struct place_object *pl;
    int npl = 0;

    pl = n->places;
    while (pl != NULL) {
        pl = pl->next;
        npl++;
    }

    return npl;
}

// ------------------------
// Count Transitions in Net
// ------------------------
int CountTransitions(struct net_object *n) {
    struct trans_object *tr1;
    struct group_object *groups;
    int tr = 0;

    tr1 = n->trans;
    while (tr1 != NULL) {
        tr++;
        tr1 = tr1->next;
    }
    groups = n->groups;
    while (groups != NULL) {
        tr1 = groups->trans;
        while (tr1 != NULL) {
            tr++;
            tr1 = tr1->next;
        }
        groups = groups->next;
    }
    return tr;
}

// -------------
// Remove LAbels
// -------------
void RemoveLabels(void) {
    int i, j, k, toadd;
    struct Desc *d;
    struct trans_object *tr;
    struct place_object *pl;
    char *newtag;

    Op1PlD = NULL;
    Op1TrD = NULL;
    newtag = (char *)emalloc(sizeof(char) * 1000);

    Ntr1 = CountTransitions(Op1);
    Npl1 = CountPlaces(Op1);

    Op1TrD = FillDescTr(Op1->trans, Op1->groups);
    Op1PlD = FillDescPl(Op1->places);

    //printf("%d\n", Tr_Rest);
    d = Op1TrD;
    for (i = 0; i < Ntr1; i++) {
        strcpy(newtag, d->tag);
        if (Tr_Rest)
            for (j = 0; j < d->nl; j++) {
                toadd = 1;
                for (k = 0; k < TransRest.nl; k++)
                    if (strcmp(TransRest.labels[k], d->labels[j]) == 0) toadd = 0;
                if (toadd) {
                    strcat(newtag, "|");
                    strcat(newtag, d->labels[j]);
                }
            }
        if (d->end != NULL) {
            strcat(newtag, "||");
            strcat(newtag, d->end);
        }
        //printf(" %s\n",newtag);
        tr = GotoTrIndex(i, Op1);
        free(tr->tag);
        tr->tag = (char *)emalloc(sizeof(char) * (strlen(newtag) + 1));
        strcpy(tr->tag, newtag);
        d = d->next;
    }

    d = Op1PlD;
    pl = Op1->places;
    for (i = 0; i < Npl1; i++) {
        strcpy(newtag, d->tag);
        if (Pl_Rest)
            for (j = 0; j < d->nl; j++) {
                toadd = 1;
                for (k = 0; k < PlaceRest.nl; k++)
                    if (strcmp(PlaceRest.labels[k], d->labels[j]) == 0) toadd = 0;
                if (toadd) {
                    strcat(newtag, "|");
                    strcat(newtag, d->labels[j]);
                }
            }
        if (d->end != NULL) {
            strcat(newtag, "||");
            strcat(newtag, d->end);
        }
        //printf(" %s\n",newtag);
        free(pl->tag);
        pl->tag = (char *)emalloc(sizeof(char) * (strlen(newtag) + 1));
        strcpy(pl->tag, newtag);
        d = d->next;
        pl = pl->next;
    }



    // destroy structures;
    while (Op1PlD != NULL) {
        for (i = 0; i < Op1PlD->nl; i++) free(Op1PlD->labels[i]);
        if (Op1PlD->tag != NULL) free(Op1PlD->tag);
        if (Op1PlD->labels != NULL) free(Op1PlD->labels);
        if (Op1PlD->end != NULL) free(Op1PlD->end);
        d = Op1PlD;
        Op1PlD = Op1PlD->next;
        free(d);
    }
    while (Op1TrD != NULL) {
        for (i = 0; i < Op1TrD->nl; i++) free(Op1TrD->labels[i]);
        if (Op1TrD->tag != NULL) free(Op1TrD->tag);
        if (Op1TrD->labels != NULL) free(Op1TrD->labels);
        if (Op1TrD->end != NULL) free(Op1TrD->end);
        d = Op1PlD;
        Op1TrD = Op1TrD->next;
        free(d);
    }
    free(newtag);
}


// =============================
// Read restriction from file fn
// =============================
void Restrictions(char *fn) {
    FILE *f;
    char *ss;
    int i;

    Tr_Rest = 0;
    Pl_Rest = 0;
    if ((f = fopen(fn, "r")) == NULL) {
        printf("\n Unable to open file %s\n", fn);
        printf("  Synchronization over all mathing labels\n\n");
        return;
    }

    ss = (char *)emalloc(2000);
    while (!feof(f)) {
        if (fscanf(f, "%s\n", ss) != 0) {
            // printf(" Parsed '%s'\n", ss);
            //lexstr=ss;

            fp = fopen(pfname, "w");
            fprintf(fp, "%s", ss);
            fclose(fp);
            fp = fopen(pfname, "r");
            yyin = fp;

            yyparse();
        }
    }

    printf("\n  Deleting the following labels:\n");
    printf("   Places: ");
    for (i = 0; i < PlaceRest.nl; i++) printf("'%s', ", PlaceRest.labels[i]);
    if (!Pl_Rest) printf("\n All Labels are Deleted\n");
    printf("\n");
    printf("   Transitions: ");
    for (i = 0; i < TransRest.nl; i++) printf("'%s', ", TransRest.labels[i]);
    if (!Tr_Rest) printf("\n All Labels are Deleted");
    printf("\n\n");

    free(ss);
    fclose(f);
}

// ----------------------------------------------
// Read Operands and build common layer structure
// ----------------------------------------------
void ReadOperands(char *op1) {
    // read Operand1;
    netobj = Op1;
    read_file(op1);
}

// ===============================
// Remove # characters from colors
// ===============================
void RemoveDCross() {
    struct trans_object *tr;
    struct arc_object *arc;
    int i1, i2;

    Ntr1 = CountTransitions(Op1);

    for (i1 = 0; i1 < Ntr1; i1++) {
        tr = GotoTrIndex(i1, Op1);
        if (tr->color != NULL)
            for (i2 = 0; i2 < strlen(tr->color); i2++)
                if (tr->color[i2] == '#') {
                    tr->color[i2] = '\0';
                    strcat(tr->color, &(tr->color[i2 + 1]));
                }
    }
    arc = Op1->arcs;
    while (arc != NULL) {
        if (arc->color != NULL)
            for (i2 = 0; i2 < strlen(arc->color); i2++)
                if (arc->color[i2] == '#') {
                    arc->color[i2] = '\0';
                    strcat(arc->color, &(arc->color[i2 + 1]));
                }
        arc = arc->next;
    }

}

// ====
// MAIN
// ====
int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("\n Utility to remove labels and '#'-s\n");
        printf("\n Usage:\n");
        printf("  remove net1 net2 function [labelfile]\n\n");
        printf(" net1: Net to work on\n");
        printf(" net2: Resulting Net\n");
        printf(" Functions: 'l': Remove labels\n");
        printf("            '#': Remove '#'-s\n");
        printf("            'b': Do both\n");
        printf(" labelfile: contains the labels to be removed\n");
        printf("            (if not given all labels are removed)\n\n");
        exit(0);
    }

    switch (argv[3][0]) {
    case '#':
        mem_alloc();
        ReadOperands(argv[1]);
        pfname = (char *)emalloc(sizeof(char) * (strlen(argv[1]) + 10));
        strcpy(pfname, argv[1]);
        strcat(pfname, ".parsing");
        printf("\n");
        RemoveDCross();
        break;
    case 'l':
        mem_alloc();
        ReadOperands(argv[1]);
        pfname = (char *)emalloc(sizeof(char) * (strlen(argv[1]) + 10));
        strcpy(pfname, argv[1]);
        strcat(pfname, ".parsing");
        printf("\n");
        if (argc > 4) {
            Restrictions(argv[4]);
        }
        else {
            printf("\n All labels will be deleted\n");
            Tr_Rest = 0;
            Pl_Rest = 0;
        }
        RemoveLabels();
        break;
    case 'b':
        mem_alloc();
        ReadOperands(argv[1]);
        pfname = (char *)emalloc(sizeof(char) * (strlen(argv[1]) + 10));
        strcpy(pfname, argv[1]);
        strcat(pfname, ".parsing");
        printf("\n");
        if (argc > 4) {
            Restrictions(argv[4]);
        }
        else {
            printf("\n All labels will be deleted\n");
            Tr_Rest = 0;
            Pl_Rest = 0;
        }
        RemoveLabels();
        RemoveDCross();
        break;
    default:
        printf(" Unknown function (type 'remove' for help)\n");
        exit(0);
    }

    write_file(argv[2]);
    mem_free();
    return 0;
}







