//
//  Utility for Compositional Algebra of SWN Petri-Nets
//

#include<string.h>

#include "global.h"
#include "layer.h"
#include "load.h"
#include "save.h"
#include "alloc.h"

#define DEBUG
#define SHIFT 25.0

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
struct net_object *Op1, *Op2;    // Two operands
struct net_object *Result;       // Result
int Nlayer1, Nlayer2;            // Number of layers in nets
int Ntr1, Ntr2, NtrR;            // Number of transitions in the nets, result
int Npl1, Npl2, NplR;            // Number of places in the nets, result
double shifty, shiftx;           // second net is shifted by shift
int **JoinTr;                    // Matrix indicating which transitions to join
int **JoinPl;                    // Matrix indicating which places to join
struct Desc *Op1TrD, *Op2TrD;    // to describe transition labelling
struct Desc *Op1PlD, *Op2PlD;    // to describe place labelling
struct RestSet PlaceRest;        // Labels of places to be sybchronised
struct RestSet TransRest;        // Labels of transitions to be sybchronised
int Tr_or_Pl;                    // Tr. or Pl under parsing
int Tr_Rest, Pl_Rest;            // Is there any restrictions?
int *MapTr1, *MapTr2;            // Mapping transitions from Operand_i to Result
int *MapPl1, *MapPl2;            // Mapping places from Operand_i to Result
int GlCountTr1, GlCountTr2;      // Used to build Map vectors for tr.s
struct Desc *PDesc;              // structure filled by parser
char *lexstr;                    // string analysed by parser
char **Translations;             // redefinition info for variables
int TLength = 0;                   // Number of Translations
int Mode;
int ArcCount;

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
    Op2 = (struct net_object *) emalloc(NETOBJ_SIZE);
    Op2->comment = NULL;
    Op2->mpars = NULL;
    Op2->places = NULL;
    Op2->groups = NULL;
    Op2->results = NULL;
    Op2->texts = NULL;
    Op2->next = NULL;
    Op2->trans = NULL;
    Op2->lisps = NULL;
    Op2->arcs = NULL;
    Op2->rpars = NULL;
    Result = (struct net_object *) emalloc(NETOBJ_SIZE);
    Result->comment = NULL;
    Result->mpars = NULL;
    Result->places = NULL;
    Result->groups = NULL;
    Result->results = NULL;
    Result->texts = NULL;
    Result->next = NULL;
    Result->trans = NULL;
    Result->lisps = NULL;
    Result->arcs = NULL;
    Result->rpars = NULL;
}


// -----------
// memory free
// -----------
void mem_free(void) {
    // TODO: THE WHOLE NETSTRUCUTURE SHOULD BE DESTROYED !!!;

    free(Op1);
    free(Op2);
    free(Result);
}

// -------------------
// find bottom of net1
// -------------------
float MaxYCoor(struct net_object *N) {
    float max = 0.0;

    struct trans_object *tr;
    struct place_object *place;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct lisp_object *lisp;
    struct group_object *group;
    struct arc_object *arc;
    struct coordinate *cc;

    arc = N->arcs;
    while (arc != NULL) {
        cc = arc->point;
        while (cc != NULL) {
            if (cc->y > max) max = cc->y;
            cc = cc->next;
        }
        arc = arc->next;
    }

    tr = N->trans;
    while (tr != NULL) {
        if (tr->center.y > max) max = tr->center.y;
        tr = tr->next;
    }
    group = N->groups;
    while (group != NULL) {
        tr = group->trans;
        while (tr != NULL) {
            if (tr->center.y > max) max = tr->center.y;
            tr = tr->next;
        }
        group = group->next;
    }
    mpar = N->mpars;
    while (mpar != NULL) {
        if (mpar->center.y > max) max = mpar->center.y;
        mpar = mpar->next;
    }
    rpar = N->rpars;
    while (rpar != NULL) {
        if (rpar->center.y > max) max = rpar->center.y;
        rpar = rpar->next;
    }
    lisp = N->lisps;
    while (lisp != NULL) {
        if (lisp->center.y > max) max = lisp->center.y;
        lisp = lisp->next;
    }
    place = N->places;
    while (place != NULL) {
        if (place->center.y > max) max = place->center.y;
        place = place->next;
    }

//  printf("Max: %e, %e\n", PIX_TO_IN(max), max);
    return max + 30.0;
}


// -----------------------
// find right side of net1
// -----------------------
float MaxXCoor(struct net_object *N) {
    float max = 0.0;

    struct trans_object *tr;
    struct place_object *place;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct lisp_object *lisp;
    struct group_object *group;
    struct arc_object *arc;
    struct coordinate *cc;

    arc = N->arcs;
    while (arc != NULL) {
        cc = arc->point;
        while (cc != NULL) {
            if (cc->x > max) max = cc->x;
            cc = cc->next;
        }
        arc = arc->next;
    }
    tr = N->trans;
    while (tr != NULL) {
        if (tr->center.x > max) max = tr->center.x;
        tr = tr->next;
    }
    group = N->groups;
    while (group != NULL) {
        tr = group->trans;
        while (tr != NULL) {
            if (tr->center.x > max) max = tr->center.x;
            tr = tr->next;
        }
        group = group->next;
    }
    mpar = N->mpars;
    while (mpar != NULL) {
        if (mpar->center.x > max) max = mpar->center.x;
        mpar = mpar->next;
    }
    rpar = N->rpars;
    while (rpar != NULL) {
        if (rpar->center.x > max) max = rpar->center.x;
        rpar = rpar->next;
    }
    lisp = N->lisps;
    while (lisp != NULL) {
        if (lisp->center.x > max) max = lisp->center.x;
        lisp = lisp->next;
    }
    place = N->places;
    while (place != NULL) {
        if (place->center.x > max) max = place->center.x;
        place = place->next;
    }

//  printf("Max: %e, %e\n", PIX_TO_IN(max), max);
    return max + 30.0;
}

// ----------------------
// concatanating comments
// ----------------------
char *ConcComments(char *c1, char *c2) {
    char *c;

    c = (char *)emalloc(sizeof(char) * (strlen(c1) + strlen(c2) + 1));
    strcpy(c, c1);
    strcat(c, c2);

#ifdef DEBUG
    printf("    The joined comment is:\n\n%s\n", c);
#endif

    return c;
}


// ------------------------------------------------
// Fill matrix that describe which tr.s are to join
// ------------------------------------------------
int FillJoinTr(int **J, struct Desc *dd1, struct Desc *dd2) {
    int i = 0, j, k, l, join, count = 0;
    struct Desc *d1, *d2;

    d1 = dd1;
    while (d1 != NULL) {
        j = 0;
        d2 = dd2;
        while (d2 != NULL) {
            join = 0;
            for (k = 0; k < d1->nl; k++)
                for (l = 0; l < d2->nl; l++)
                    if (strcmp(d1->labels[k], d2->labels[0]) == 0) join = 1;
            J[i][j] = join;
            if (join) count++;
            d2 = d2->next;
            j++;
        }
        d1 = d1->next;
        i++;
    }
    return count;
}

// ------------------------------------------------
// Fill matrix that describe which pl.s are to join
// ------------------------------------------------
int FillJoinPl(int **J, struct Desc *dd1, struct Desc *dd2) {
    int i = 0, j, k, l, join, count = 0;
    struct Desc *d1, *d2;

    d1 = dd1;
    while (d1 != NULL) {
        j = 0;
        d2 = dd2;
        while (d2 != NULL) {
            join = 0;
            for (k = 0; k < d1->nl; k++)
                for (l = 0; l < d2->nl; l++)
                    if (strcmp(d1->labels[k], d2->labels[0]) == 0) join = 1;
            J[i][j] = join;
            if (join) count++;
            d2 = d2->next;
            j++;
        }
        d1 = d1->next;
        i++;
    }
    return count;
}

// ----------------------------------------
// Filling structures to describe labelling
// ----------------------------------------
struct Desc *FillDescTr(struct trans_object *tr, struct group_object *group) {
    struct trans_object *t = tr;
    struct group_object *gr = group;
    struct Desc *d = NULL, *tmp = NULL;
    int first = 1, count = 0, i;

    Tr_or_Pl = 1;

    // goto the first non-empty group;
    while (gr != NULL && gr->trans == NULL) gr = gr->next;

    if (t == NULL) {
        t = gr->trans;
        gr = gr->next;
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
        lexstr = tmp->wholetag;
        PDesc = tmp;
        yyparse();

#ifdef DEBUG
        printf(" %d. Full Tag: '%s'\n", count, t->tag);
        printf("  Tag: '%s'\n  NofLabels: %d, Labels: ", tmp->tag, tmp->nl);
        if (tmp->labels != NULL)
            for (i = 0; i < tmp->nl; i++) printf("'%s', ", tmp->labels[i]);
        printf("\n  End: ");
        if (tmp->end != NULL) printf("'%s'", tmp->end);
        printf("\n");
        count++;
#endif DEBUG

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
    tmp->next = NULL;
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
        lexstr = tmp->wholetag;
        PDesc = tmp;
        yyparse();

#ifdef DEBUG
        printf(" %d.  FullTag: %s\n  Tag: '%s'\n  NofLabels: %d, Labels: ", count, tmp->wholetag, tmp->tag, tmp->nl);
        if (tmp->labels != NULL)
            for (i = 0; i < tmp->nl; i++) printf("'%s', ", tmp->labels[i]);
        printf("\n  End: ");
        if (tmp->end != NULL) printf("'%s'", tmp->end);
        printf("\n");
        count++;
#endif DEBUG

        t = t->next;
    }
    tmp->next = NULL;
    return d;
}

// ------------------------------------------------------------
// Join Marking Parameters Objects (matching names are deleted)
// ------------------------------------------------------------
struct mpar_object *JoinMPars(struct mpar_object *mpar1, struct mpar_object *mpar2) {
    struct mpar_object *mp1, *mp2, *mp3, *r;
    int first = 1, i, toadd;

    r = NULL;
    if (mpar1 != NULL || mpar2 != NULL) {
        mp1 = mpar1;
        while (mp1 != NULL) {
            if (first) {
                r = (struct mpar_object *)emalloc(sizeof(struct mpar_object));
                mp2 = r;
                first = 0;
            }
            else {
                mp2->next = (struct mpar_object *)emalloc(sizeof(struct mpar_object));
                mp2 = mp2->next;
            }
            mp2->tag = (char *)emalloc(sizeof(char) * (strlen(mp1->tag) + 1));
            strcpy(mp2->tag, mp1->tag);
            mp2->value = mp1->value;
            mp2->center.x = mp1->center.x;
            mp2->center.y = mp1->center.y;
            mp2->layer = NewLayerList(WHOLENET, NULL);
            for (i = 1; i <= Nlayer1; i++)
                if (TestLayer(i, mp1->layer))
                    SetLayer2LayerList(i + Nlayer2, mp2->layer);
            mp2->next = NULL;

            mp1 = mp1->next;
        }
        mp1 = mpar2;
        while (mp1 != NULL) {
            toadd = 1;
            mp3 = r;
            while (mp3 != NULL) {
                if (strcmp(mp3->tag, mp1->tag) == 0) toadd = 0;
                mp3 = mp3->next;
            }

            if (toadd) {
                if (first) {
                    r = (struct mpar_object *)emalloc(sizeof(struct mpar_object));
                    mp2 = r;
                    first = 0;
                }
                else {
                    mp2->next = (struct mpar_object *)emalloc(sizeof(struct mpar_object));
                    mp2 = mp2->next;
                }
                mp2->tag = (char *)emalloc(sizeof(char) * (strlen(mp1->tag) + 1));
                strcpy(mp2->tag, mp1->tag);
                mp2->value = mp1->value;
                mp2->center.x = mp1->center.x + shiftx;
                mp2->center.y = mp1->center.y + shifty;
                mp2->layer = NewLayerList(WHOLENET, NULL);
                for (i = 1; i <= Nlayer2; i++)
                    if (TestLayer(i, mp1->layer))
                        SetLayer2LayerList(i, mp2->layer);
                mp2->next = NULL;
            }
            mp1 = mp1->next;
        }
    }

#ifdef DEBUG
    printf("  Marking Parameters in Result:\n");
    mp1 = r;
    while (mp1 != NULL) {
        printf("    %s: %d, Layers: ", mp1->tag, mp1->value);
        for (i = 1; i <= Nlayer1 + Nlayer2; i++)
            if (TestLayer(i, mp1->layer)) printf(" %s", GetLayerName(i));
        mp1 = mp1->next;
        printf("\n");
    }
    printf("\n");
#endif

    return r;
}


// ------------------------------------------------------------
// Join Rate Parameters Objects (matching names are deleted)
// ------------------------------------------------------------
struct rpar_object *JoinRPars(struct rpar_object *rpar1, struct rpar_object *rpar2) {
    struct rpar_object *rp1, *rp2, *rp3, *r;
    int first = 1, i, toadd;

    r = NULL;
    if (rpar1 != NULL || rpar2 != NULL) {
        rp1 = rpar1;
        while (rp1 != NULL) {
            if (first) {
                r = (struct rpar_object *)emalloc(sizeof(struct rpar_object));
                rp2 = r;
                first = 0;
            }
            else {
                rp2->next = (struct rpar_object *)emalloc(sizeof(struct rpar_object));
                rp2 = rp2->next;
            }
            rp2->tag = (char *)emalloc(sizeof(char) * (strlen(rp1->tag) + 1));
            strcpy(rp2->tag, rp1->tag);
            rp2->value = rp1->value;
            rp2->center.x = rp1->center.x;
            rp2->center.y = rp1->center.y;
            rp2->layer = NewLayerList(WHOLENET, NULL);
            for (i = 1; i <= Nlayer1; i++)
                if (TestLayer(i, rp1->layer))
                    SetLayer2LayerList(i + Nlayer2, rp2->layer);
            rp2->next = NULL;

            rp1 = rp1->next;
        }
        rp1 = rpar2;
        while (rp1 != NULL) {
            toadd = 1;
            rp3 = r;
            while (rp3 != NULL) {
                if (strcmp(rp3->tag, rp1->tag) == 0) toadd = 0;
                rp3 = rp3->next;
            }

            if (toadd) {
                if (first) {
                    r = (struct rpar_object *)emalloc(sizeof(struct rpar_object));
                    rp2 = r;
                    first = 0;
                }
                else {
                    rp2->next = (struct rpar_object *)emalloc(sizeof(struct rpar_object));
                    rp2 = rp2->next;
                }
                rp2->tag = (char *)emalloc(sizeof(char) * (strlen(rp1->tag) + 1));
                strcpy(rp2->tag, rp1->tag);
                rp2->value = rp1->value;
                rp2->center.x = rp1->center.x + shiftx;
                rp2->center.y = rp1->center.y + shifty;
                rp2->layer = NewLayerList(WHOLENET, NULL);
                for (i = 1; i <= Nlayer2; i++)
                    if (TestLayer(i, rp1->layer))
                        SetLayer2LayerList(i, rp2->layer);
                rp2->next = NULL;
            }
            rp1 = rp1->next;
        }
    }

#ifdef DEBUG
    printf("  Rate Parameters in Result:\n");
    rp1 = r;
    while (rp1 != NULL) {
        printf("    %s: %f, Layers: ", rp1->tag, rp1->value);
        for (i = 1; i <= Nlayer1 + Nlayer2; i++)
            if (TestLayer(i, rp1->layer)) printf(" %s", GetLayerName(i));
        rp1 = rp1->next;
        printf("\n");
    }
    printf("\n");
#endif
    return r;
}

// ------------------------------------------------------------
// Join Lisp Objects (matching names are deleted)
// ------------------------------------------------------------
struct lisp_object *JoinLisps(struct lisp_object *lisp1, struct lisp_object *lisp2) {
    struct lisp_object *li1, *li2, *li3, *r;
    int first = 1, toadd;

    r = NULL;
    if (lisp1 != NULL || lisp2 != NULL) {
        li1 = lisp1;
        while (li1 != NULL) {
            if (first) {
                r = (struct lisp_object *)emalloc(sizeof(struct lisp_object));
                li2 = r;
                first = 0;
            }
            else {
                li2->next = (struct lisp_object *)emalloc(sizeof(struct lisp_object));
                li2 = li2->next;
            }
            li2->tag = (char *)emalloc(sizeof(char) * (strlen(li1->tag) + 1));
            strcpy(li2->tag, li1->tag);
            li2->text = (char *)emalloc(sizeof(char) * (strlen(li1->text) + 1));
            strcpy(li2->text, li1->text);
            li2->type = li1->type;
            li2->center.x = li1->center.x;
            li2->center.y = li1->center.y;
            li2->next = NULL;

            li1 = li1->next;
        }
        li1 = lisp2;
        while (li1 != NULL) {
            toadd = 1;
            li3 = r;
            while (li3 != NULL) {
                if (strcmp(li3->tag, li1->tag) == 0) toadd = 0;
                li3 = li3->next;
            }

            if (toadd) {
                if (first) {
                    r = (struct lisp_object *)emalloc(sizeof(struct lisp_object));
                    li2 = r;
                    first = 0;
                }
                else {
                    li2->next = (struct lisp_object *)emalloc(sizeof(struct lisp_object));
                    li2 = li2->next;
                }
                li2->tag = (char *)emalloc(sizeof(char) * (strlen(li1->tag) + 1));
                strcpy(li2->tag, li1->tag);
                li2->text = (char *)emalloc(sizeof(char) * (strlen(li1->text) + 1));
                strcpy(li2->text, li1->text);
                li2->type = li1->type;
                li2->center.x = li1->center.x + shiftx;
                li2->center.y = li1->center.y + shifty;
                li2->next = NULL;
            }
            li1 = li1->next;
        }
    }

#ifdef DEBUG
    printf("  Lisps in Result:\n");
    li1 = r;
    while (li1 != NULL) {
        printf("    %s: %s %c\n", li1->tag, li1->text, li1->type);
        li1 = li1->next;
    }
    printf("\n");
#endif
    return r;
}


// -----------------------------
// Copy Properties of transition
// -----------------------------
void CopyTrProperties(struct trans_object *tr1, struct trans_object *tr2, int j, int in_op1) {
    struct lisp_object *li;
    struct rpar_object *mp;
    int i;

    tr2->tag = (char *)emalloc(sizeof(char) * (strlen(tr1->tag) + 1));
    strcpy(tr2->tag, tr1->tag);
    if (tr1->color != NULL) {
        tr2->color = (char *)emalloc(sizeof(char) * (strlen(tr1->color) + 1));
        strcpy(tr2->color, tr1->color);
    }
    else {
        tr2->color = NULL;
    }
    if (tr1->mark_dep != NULL) {
        tr2->mark_dep = (char *)emalloc(sizeof(char) * (strlen(tr1->mark_dep) + 1));
        strcpy(tr2->mark_dep, tr1->mark_dep);
    }
    else {
        tr2->mark_dep = NULL;
    }
    tr2->center.x = tr1->center.x + j * 1.2 * SHIFT + (1 - in_op1) * shiftx;
    tr2->center.y = tr1->center.y + j * SHIFT + (1 - in_op1) * shifty;
    tr2->tagpos.x = tr1->tagpos.x;
    tr2->tagpos.y = tr1->tagpos.y;
    tr2->colpos.x = tr1->colpos.x;
    tr2->colpos.y = tr1->colpos.y;
    tr2->ratepos.x = tr1->ratepos.x;
    tr2->ratepos.y = tr1->ratepos.y;
    tr2->pripos.x = tr1->pripos.x;
    tr2->pripos.y = tr1->pripos.y;
    tr2->orient = tr1->orient;
    tr2->kind = tr1->kind;
    tr2->enabl = tr1->enabl;
    tr2->Lbound = tr1->Lbound;
    tr2->Ebound = tr1->Ebound;
    tr2->Rbound = tr1->Rbound;
    tr2->fire_rate.ff = tr1->fire_rate.ff;
    tr2->layer = NewLayerList(WHOLENET, NULL);
    for (i = 1; i <= Nlayer2; i++)
        if (TestLayer(i, tr1->layer))
            SetLayer2LayerList(i + Nlayer2 * in_op1, tr2->layer);
    tr2->lisp = NULL;
    if (tr1->lisp != NULL) {
        li = Result->lisps;
        while (strcmp(li->tag, tr1->lisp->tag) != 0) li = li->next;
        tr2->lisp = li;
    }
    tr2->rpar = NULL;
    if (tr1->rpar != NULL) {
        mp = Result->rpars;
        while (strcmp(mp->tag, tr1->rpar->tag) != 0) mp = mp->next;
        tr2->rpar = mp;
    }
    tr2->next = NULL;
}

// ------------------------------------------
// Compute how many copy of tr. is neccessary
// ------------------------------------------
int TrNeed(int tr) {
    struct Desc *d1, *d2;
    int i, *count, m;

    if (Mode == 2) return 1;
    d1 = Op1TrD;
    for (i = 0; i < tr; i++) d1 = d1->next;
    count = (int *)emalloc(sizeof(int) * d1->nl);
    for (i = 0; i < d1->nl; i++) count[i] = 0;
    d2 = Op2TrD;
    while (d2 != NULL) {
        for (i = 0; i < d1->nl; i++)
            if (d2->nl > 0)
                if (strcmp(d1->labels[i], d2->labels[0]) == 0) count[i]++;
        d2 = d2->next;
    }
    m = 1;
    for (i = 0; i < d1->nl; i++)
        if (count[i] != 0) m *= count[i];
    free(count);
    return m;
}


// -------------------------------------------------
// Compute how many copy of tr. is neccessary indeed
// -------------------------------------------------
int TrNeed2(int tr, int tr2) {
    struct Desc *d1, *d2;
    int i, j = 0, *count, m;
    char *tag;

    if (Mode == 2) return 1;
    d2 = Op2TrD;
    for (i = 0; i < tr2; i++) d2 = d2->next;
    tag = (char *)emalloc(sizeof(char) * (strlen(d2->labels[0]) + 1));
    strcpy(tag, d2->labels[0]);

    d1 = Op1TrD;
    for (i = 0; i < tr; i++) d1 = d1->next;
    count = (int *)emalloc(sizeof(int) * d1->nl);
    for (i = 0; i < d1->nl; i++) count[i] = 0;
    d2 = Op2TrD;
    j = 0;
    while (d2 != NULL) {
        for (i = 0; i < d1->nl; i++)
            if (d2->nl > 0)
                if (strcmp(d2->labels[0], tag) != 0)
                    if (strcmp(d1->labels[i], d2->labels[0]) == 0) count[i]++;
        d2 = d2->next;
        j++;
    }

    //for(i=0;i<d1->nl;i++) printf("%d ",count[i]);
    //printf("\n");

    m = 1;
    for (i = 0; i < d1->nl; i++)
        if (count[i] != 0) m *= count[i];
    free(count);
    free(tag);
    return m;
}

// ------------------------------------------
// Compute how many copy of pl. is neccessary
// ------------------------------------------
int PlNeed(int pl) {
    struct Desc *d1, *d2;
    int i, *count, m;

    if (Mode == 1) return 1;
    d1 = Op1PlD;
    for (i = 0; i < pl; i++) d1 = d1->next;
    count = (int *)emalloc(sizeof(int) * d1->nl);
    for (i = 0; i < d1->nl; i++) count[i] = 0;
    d2 = Op2PlD;
    while (d2 != NULL) {
        for (i = 0; i < d1->nl; i++)
            if (d2->nl > 0)
                if (strcmp(d1->labels[i], d2->labels[0]) == 0) count[i]++;
        d2 = d2->next;
    }
    m = 1;
    for (i = 0; i < d1->nl; i++)
        if (count[i] != 0) m *= count[i];
    free(count);
    return m;
}


// -------------------------------------------------
// Compute how many copy of pl. is neccessary indeed
// -------------------------------------------------
int PlNeed2(int pl, int pl2) {
    struct Desc *d1, *d2;
    int i, j = 0, *count, m;
    char *tag;

    if (Mode == 1) return 1;

    d2 = Op2PlD;
    for (i = 0; i < pl2; i++) d2 = d2->next;
    tag = (char *)emalloc(sizeof(char) * (strlen(d2->labels[0]) + 1));
    strcpy(tag, d2->labels[0]);

    d1 = Op1PlD;
    for (i = 0; i < pl; i++) d1 = d1->next;
    count = (int *)emalloc(sizeof(int) * d1->nl);
    for (i = 0; i < d1->nl; i++) count[i] = 0;
    d2 = Op2PlD;
    j = 0;
    while (d2 != NULL) {
        for (i = 0; i < d1->nl; i++)
            if (d2->nl > 0)
                if (strcmp(d2->labels[0], tag) != 0)
                    if (strcmp(d1->labels[i], d2->labels[0]) == 0) count[i]++;
        d2 = d2->next;
        j++;
    }
    m = 1;
    for (i = 0; i < d1->nl; i++)
        if (count[i] != 0) m *= count[i];
    free(count);
    return m;
}

// -------------------
// Copy Arc Properties
// -------------------
void CopyArcProperties(struct arc_object *a1, struct arc_object *a2, int in_op1) {
    struct lisp_object *li;
    int j;

    a2->type = a1->type;
    a2->color = NULL;
    a2->colpos.x = a1->colpos.x;
    a2->colpos.y = a1->colpos.y;
    a2->mult = a1->mult;
    if (a1->color != NULL) {
        a2->color = (char *)emalloc(sizeof(char) * (strlen(a1->color) + 1));
        strcpy(a2->color, a1->color);
    }
    a2->lisp = NULL;
    if (a1->lisp != NULL) {
        li = Result->lisps;
        while (strcmp(li->tag, a1->lisp->tag) != 0) li = li->next;
        a2->lisp = li;
    }
    a2->layer = NewLayerList(WHOLENET, NULL);
    for (j = 1; j <= Nlayer2; j++)
        if (TestLayer(j, a1->layer))
            SetLayer2LayerList(j + Nlayer2 * in_op1, a2->layer);
}


// ------------------------
// Determine Index of Place
// ------------------------
int DeterminePlIndex(struct place_object *pl, struct net_object *n) {
    int index = 0;
    struct place_object *p;

    p = n->places;
    while (p != pl) {
        index++;
        p = p->next;
    }
    return index;
}

// ---------------------------------------------
// Compute Shift with respect to pl in operand 1
// ---------------------------------------------
int ComputePlIndex(int i1, int i2, int c) {
    int index, i, pos, m;
    int *count, *run, blabla = 0;
    struct Desc *d1, *d2;

    d1 = Op1PlD;
    for (i = 0; i < i1; i++) d1 = d1->next;
    d2 = Op2PlD;
    for (i = 0; i < i2; i++) d2 = d2->next;
    pos = 0;
    while (strcmp(d2->labels[0], d1->labels[pos++]) != 0);
    pos--;
    m = 1;
    d2 = Op2PlD;
    i = 0;
    while (i < i2) {
        if (JoinPl[i1][i] == 1)
            if (strcmp(d1->labels[pos], d2->labels[0]) == 0) m++;
        d2 = d2->next;
        i++;
    }

    count = (int *)emalloc(sizeof(int) * d1->nl);
    run = (int *)emalloc(sizeof(int) * d1->nl);
    for (i = 0; i < d1->nl; i++) count[i] = 0;
    for (i = 0; i < d1->nl; i++) run[i] = 1;
    d2 = Op2PlD;
    while (d2 != NULL) {
        for (i = 0; i < d1->nl; i++)
            if (d2->nl > 0)
                if (strcmp(d1->labels[i], d2->labels[0]) == 0) count[i]++;
        d2 = d2->next;
    }
    for (i = 0; i < d1->nl; i++) if (count[i] == 0) count[i] = 1;

    index = -1;
    while (blabla <= c) {
        if (run[pos] == m) blabla++;

        i = 0;
        while (i < d1->nl) {
            run[i] = (run[i] % count[i]) + 1;
            if (run[i] != 1) break;
            i++;
        }
        index++;
    }

    free(count);
    free(run);
    return index + MapPl1[i1];
}

// ===================
// Define Arc Position
// ===================
void ArcPosition(struct position *p, int pos, int in) {
    int deg;
    div_t d;

    deg = 10 + 15 * (int)(pos / 2);
    d = div(pos, 2);
    deg += 45 * d.rem;
    d = div(deg, 90);
    deg = d.rem;

    //printf(" pos : %d, deg: %d\n", pos, deg);

    if (!in) {
        p->x = SHIFT * sin((double)deg / 180.0 * 3.14);
        p->y = -SHIFT * cos((double)deg / 180.0 * 3.14);
    }
    else {
        p->x = SHIFT * cos((double)deg / 180.0 * 3.14);
        p->y = SHIFT * sin((double)deg / 180.0 * 3.14);
    }
}

// =====================================================
// Adding Simple arcs (no redefinition of colour needed)
// =====================================================
void SimpleArcs(struct trans_object *tr1, struct trans_object *tr2, int op1) {
    struct arc_object *arcs, *new;
    struct place_object *place;
    int p, i1, i2, np, mul, simplepl, plindex1, PlIndex, i3, j, mulmul, l;
    struct coordinate *ccp1, *ccp2;
    struct position pospl;
    double length;

    if (op1) arcs = Op1->arcs;
    if (!op1) arcs = Op2->arcs;
    while (arcs != NULL) {
        if (arcs->trans == tr1) {
            simplepl = 1;
            if (op1) {
                p = DeterminePlIndex(arcs->place, Op1);
                mul = PlNeed(p);
                p = MapPl1[p];
                place = Result->places;
                for (i1 = 0; i1 < p; i1++) place = place->next;
            }
            if (!op1) {
                p = DeterminePlIndex(arcs->place, Op2);
                mul = 0;
                simplepl = 0;
                for (i1 = 0; i1 < Npl1; i1++) if (JoinPl[i1][p] == 1) mul++;
                if (mul == 0) {
                    mul++;
                    simplepl = 1;
                }
            }

            for (i2 = 0; i2 < mul; i2++) {
                mulmul = 1;
                if (!op1 && !simplepl) {
                    j = 0; plindex1 = 0;
                    while (j <= i2) if (JoinPl[plindex1++][p] == 1) j++;
                    plindex1--;
                    //printf(" plindex1: %d\n",plindex1);
                    mulmul = PlNeed2(plindex1, p);
                    // printf(" realmulpl: %d\n", realmulpl);
                }
                for (i3 = 0; i3 < mulmul; i3++) {
                    if (Result->arcs == NULL) {
                        Result->arcs = (struct arc_object *)emalloc(sizeof(struct arc_object));
                        new = Result->arcs;
                        new->next = NULL;
                    }
                    else {
                        new = (struct arc_object *)emalloc(sizeof(struct arc_object));
                        new->next = Result->arcs;
                        Result->arcs = new;
                    }
                    new->trans = tr2;
                    if (op1) {
                        new->place = place;
                        place = place->next;
                    }
                    if (!op1) {
                        if (!simplepl) {
                            PlIndex = ComputePlIndex(plindex1, p, i3);
                        }
                        else {
                            PlIndex = MapPl2[p];
                        }
                        place = Result->places;
                        for (l = 0; l < PlIndex; l++) place = place->next;
                        new->place = place;
                    }

                    CopyArcProperties(arcs, new, op1);
                    ArcCount++;

                    new->point = (struct coordinate *) emalloc(sizeof(struct coordinate));
                    ccp2 = new->point;
                    if (simplepl) {
                        np = 0;
                        ccp1 = arcs->point;
                        while (ccp1 != NULL) { ccp1 = ccp1->next; np++;}
                        ccp1 = arcs->point->next;
                        for (i1 = 0; i1 < np - 2; i1++) {
                            ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                            ccp2 = ccp2->next;
                            ccp2->x = ccp1->x + (!op1) * shiftx;
                            ccp2->y = ccp1->y + (!op1) * shifty;
                            ccp1 = ccp1->next;
                        }
                        ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                        ccp2 = ccp2->next;
                        ccp2->next = NULL;
                    }
                    else {
                        ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                        ccp2 = ccp2->next;

                        // Adding broken arc properties next to the transition;
                        length = (arcs->point->next->x - arcs->point->x) * (arcs->point->next->x - arcs->point->x);
                        length += (arcs->point->next->y - arcs->point->y) * (arcs->point->next->y - arcs->point->y);
                        length = sqrt(length);
                        ccp2->x = tr2->center.x + (arcs->point->next->x - arcs->point->x) / length * SHIFT * 1.5;
                        ccp2->y = tr2->center.y + (arcs->point->next->y - arcs->point->y) / length * SHIFT * 1.5;

                        // Adding broken arc properties next to the place;
                        ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                        ccp2 = ccp2->next;

                        if (arcs->type == 'i') {
                            ArcPosition(&pospl, place->brokenin, 1);
                            place->brokenin++;
                        }
                        else {
                            ArcPosition(&pospl, place->brokenout, 0);
                            place->brokenout++;
                        }
                        ccp2->x = place->center.x + pospl.x;
                        ccp2->y = place->center.y + pospl.y;

                        if (new->mult > 0)
                            new->mult *= -1;

                        ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                        ccp2->next->next = NULL;
                    }
                }
            }
        }
        arcs = arcs->next;
    }
}

// ===============================================
// Count number of transitions to synchronize with
// ===============================================
int CountPlayers(int index) {
    int *count, i1, i2, n = 0;
    struct Desc *d1, *d2;

    d1 = Op1TrD;
    for (i1 = 0; i1 < index; i1++) d1 = d1->next;
    count = (int *)emalloc(sizeof(int) * d1->nl);
    for (i1 = 0; i1 < d1->nl; i1++) count[i1] = 0;

    d2 = Op2TrD; i2 = 0;
    while (d2 != NULL) {
        for (i1 = 0; i1 < d1->nl; i1++)
            if (d2->nl > 0 && JoinTr[index][i2]) if (strcmp(d1->labels[i1], d2->labels[0]) == 0) count[i1]++;
        d2 = d2->next;
        i2++;
    }
    for (i1 = 0; i1 < d1->nl; i1++) if (count[i1] > 0) n++;
    free(count);

    printf("Number of transitions synchronized over %d. tr is %d\n", index, n);

    return n;
}

// =============================================================
// Looking for transitions involved in specimen. tr of tr. index
// =============================================================
void LookforPlayers(int index, int specimen, int *players) {
    int *count, i1, i2, i3, i4, n = 0, *v;
    struct Desc *d1, *d2;

    d1 = Op1TrD;
    for (i1 = 0; i1 < index; i1++) d1 = d1->next;
    count = (int *)emalloc(sizeof(int) * d1->nl);
    v = (int *)emalloc(sizeof(int) * d1->nl);
    for (i1 = 0; i1 < d1->nl; i1++) count[i1] = 0;

    d2 = Op2TrD; i2 = 0;
    while (d2 != NULL) {
        for (i1 = 0; i1 < d1->nl; i1++)
            if (d2->nl > 0 && JoinTr[index][i2]) if (strcmp(d1->labels[i1], d2->labels[0]) == 0) count[i1]++;
        d2 = d2->next;
        i2++;
    }
    for (i1 = 0; i1 < d1->nl; i1++) {
        v[i1] = (count[i1] > 0);
        n += (count[i1] > 0);
    }
    for (i1 = 0; i1 < specimen; i1++) {
        for (i2 = 0; i2 < d1->nl; i2++)
            if (count[i2] > 0) {
                v[i2]++;
                if (v[i2] > count[i2]) v[i2] = 1;
                else i2 = d1->nl;
            }
    }
    printf(" Player vector is: \n");
    for (i2 = 0; i2 < d1->nl; i2++) printf(" %d", v[i2]);
    printf("\n");
    i2 = -1;
    for (i1 = 0; i1 < n; i1++) {
        i2++;
        while (v[i2] == 0) i2++;
        //printf(" %s, i2: %d\n", d1->labels[i2], v[i2]);

        i3 = 0;
        i4 = 0;
        d2 = Op2TrD;
        while (i3 != v[i2]) {
            if (d2->nl > 0) {
                //printf(" %s\n", d2->labels[0]);
                if (strcmp(d2->labels[0], d1->labels[i2]) == 0) i3++;
            }
            if (i3 != v[i2]) {
                i4++;
                d2 = d2->next;
            }
        }
        players[i1] = i4;
    }
    printf(" Players are: \n");
    for (i1 = 0; i1 < n; i1++) printf(" %d", players[i1]);
    printf("\n");

    free(count);
    free(v);
}

// ===================================
// Check character in color expression
// ===================================
int CharSet(char c) {
    return (c == '(' || c == ')' ||	c == '[' || c == ']' ||
            c == '<' || c == '>' || c == '+' || c == '-' ||
            c == 'S' || c == '!' || c == '=' || c == '*' ||
            c == ' ' || c == '^');
}

// ======================================================
// Collecting number of variables connected to transition
// ======================================================
char **CollectVariables(struct trans_object *tr, int *n) {
    struct arc_object *a;
    int count = 0, pos, i1, search, variable, toadd, trpred = 1;
    char *var, **res = NULL, *str;

    var = (char *) emalloc(sizeof(char) * 100);
    a = Result->arcs;
    while (a != NULL || trpred) {
        if (!trpred && a->trans == tr && a->color != NULL || trpred && tr->color != NULL) {
            if (!trpred) str = a->color;
            if (trpred) str = tr->color;
            pos = 0;
            printf(" Searching variables in: %s\n", str);
            while (str[pos] != '\0') {
                search = 1;
                while (search) {
                    search = 0;
                    if (CharSet(str[pos]) ||
                            (str[pos] >= '0' && str[pos] <= '9')) {
                        pos++;
                        search = 1;
                    }
                    if (str[pos] == 'd')
                        if (str[pos + 1] == '(') pos += 2;
                    if (str[pos] == '\0') search = 0;
                }
                variable = 1;
                if (str[pos] == '\0') variable = 0;
                i1 = 0;
                while (variable) {
                    var[i1] = str[pos];
                    pos++;
                    i1++;
                    var[i1] = '\0';
                    if (CharSet(str[pos]) || str[pos] == '\0') {
                        variable = 0;
                        toadd = 1;
                        for (i1 = 0; i1 < count; i1++)
                            if (strcmp(var, res[i1]) == 0) toadd = 0;
                        if (toadd) {
                            printf("  Variable %s is added to list\n", var);
                            res = (char **)realloc(res, (count + 1) * sizeof(char *));
                            res[count] = (char *)emalloc(sizeof(char) * strlen(var) + 1);
                            strcpy(res[count], var);
                            count++;
                        }
                    }
                }
            }
        }
        if (!trpred) a = a->next;
        trpred = 0;
    }

    printf(" Variables:\n");
    for (i1 = 0; i1 < count; i1++) printf("  %s\n", res[i1]);
    printf("\n");

    *n = count;
    free(var);
    return res;
}


// ======================================================
// Redefine variables in order to avoid unwanted matching
// ======================================================
char *RedefineVariables(int nofvar, char **varset, char *oldcolor) {
    char *newcolor, *newvar, *var, *end, *num;
    int pos, pos2, variable, change, tochange, search, i1, i2;

    printf(" Old color is %s\n", oldcolor);

    newcolor = (char *)emalloc(sizeof(char) * 100);
    newvar = (char *)emalloc(sizeof(char) * 100);
    var = (char *)emalloc(sizeof(char) * 100);
    end = (char *)emalloc(sizeof(char) * 6);
    end += 5;

    pos = 0;
    pos2 = 0;
    while (oldcolor[pos] != '\0') {
        search = 1;
        while (search) {
            search = 0;
            if (CharSet(oldcolor[pos]) || (oldcolor[pos] >= '0' && oldcolor[pos] <= '9')) {
                newcolor[pos2] = oldcolor[pos];
                pos++;
                pos2++;
                search = 1;
            }
            if (oldcolor[pos] == 'd')
                if (oldcolor[pos + 1] == '(') {
                    newcolor[pos2] = 'd';
                    newcolor[pos2 + 1] = '(';
                    pos += 2;
                    pos2 += 2;
                }
            if (oldcolor[pos] == '\0') search = 0;
        }
        variable = 1;
        if (oldcolor[pos] == '\0') variable = 0;
        i1 = 0;
        while (variable) {
            var[i1] = oldcolor[pos];
            pos++;
            i1++;
            var[i1] = '\0';
            if (CharSet(oldcolor[pos]) || oldcolor[pos] == '\0') {
                printf("  Variable is: %s\n", var);
                variable = 0;
                tochange = 0;
                for (i1 = 0; i1 < nofvar; i1++)
                    if (strcmp(var, varset[i1]) == 0) tochange = 1;
                if (strcmp(var, "or") == 0) tochange = 0;
                if (strcmp(var, "and") == 0) tochange = 0;
                if (var[0] == '#') tochange = 0;
                if (tochange) {
                    printf("  Has to be changed\n");
                    // look among translations;
                    for (i1 = 0; i1 < TLength; i1++)
                        if (strcmp(var, Translations[2 * i1]) == 0) {
                            tochange = 0;
                            strcpy(newvar, Translations[2 * i1 + 1]);
                            printf("  Found among translations\n");
                        }
                    i2 = 1;
                    while (tochange) {
                        strcpy(newvar, "x");
                        num = lltostr((long)i2, end);
                        end[0] = '\0';
                        strcat(newvar, num);
                        //printf(" New Var: %s\n", newvar);
                        tochange = 0;
                        for (i1 = 0; i1 < nofvar; i1++)
                            if (strcmp(newvar, varset[i1]) == 0) tochange = 1;
                        for (i1 = 0; i1 < TLength; i1++)
                            if (strcmp(newvar, Translations[i1 * 2 + 1]) == 0) tochange = 1;
                        if (!tochange) {
                            printf("  Translated to %s, nooftranslations is %d\n", newvar, TLength);
                            Translations = (char **)realloc(Translations, (1 + TLength) * 2 * sizeof(char *));
                            Translations[TLength * 2] = (char *)emalloc(sizeof(char *) * (strlen(var) + 1));
                            Translations[TLength * 2 + 1] = (char *)emalloc(sizeof(char *) * (strlen(newvar) + 1));
                            strcpy(Translations[TLength * 2], var);
                            strcpy(Translations[TLength * 2 + 1], newvar);
                            TLength++;
                        }
                        i2++;
                    }
                    newcolor[pos2] = '\0';
                    strcat(newcolor, newvar);
                    pos2 += strlen(newvar);
                }
                else {
                    newcolor[pos2] = '\0';
                    strcat(newcolor, var);
                    pos2 += strlen(var);
                }
            }
        }
    }
    newcolor[pos2] = '\0';
    printf(" New color is: %s\n\n", newcolor);

    free(newvar);
    free(var);
    end -= 5;
    free(end);
    return newcolor;
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


// ================================================================
// Adding arcs of argument 2 that are connected to sync transitions
// ================================================================
void ArcsofArg2(struct trans_object *tr2, int op1index, int specimen) {
    struct arc_object *arcs, *new, *a3;
    struct place_object *place;
    struct trans_object *trans;
    int p, i1, i2, np, mul, simplepl, plindex1, PlIndex, i3, j, mulmul, l, doublearc;
    struct coordinate *ccp1, *ccp2;
    struct position pospl;
    double length;
    int *players, nofplayers, i4, i5, nofvar;
    char **varset, *newcolor, *newpredicate, *num, *end;

    newpredicate = (char *)emalloc(sizeof(char) * 1000);
    end = (char *)emalloc(sizeof(char) * 10);
    end += 5;
    nofplayers = CountPlayers(op1index);
    players = (int *)emalloc(sizeof(int) * nofplayers);
    LookforPlayers(op1index, specimen, players);

    for (i4 = 0; i4 < nofplayers; i4++) {
        varset = CollectVariables(tr2, &nofvar);
        trans = GotoTrIndex(players[i4], Op2);

        for (i1 = 0; i1 < TLength * 2; i1++) free(Translations[i1]);
        free(Translations);
        Translations = NULL;
        TLength = 0;

        if (trans->color != NULL) {
            newcolor = RedefineVariables(nofvar, varset, trans->color);
            if (tr2->color != NULL) {
                strcpy(newpredicate, "[(");
                tr2->color[strlen(tr2->color) - 1] = '\0';
                strcat(newpredicate, &(tr2->color[1]));
                newcolor[strlen(newcolor) - 1] = '\0';
                strcat(newpredicate, ") and (");
                strcat(newpredicate, &(newcolor[1]));
                strcat(newpredicate, ")]");
            }
            else {
                strcpy(newpredicate, newcolor);
            }
            free(tr2->color);
            tr2->color = (char *)emalloc(sizeof(char) * (strlen(newpredicate) + 1));
            strcpy(tr2->color, newpredicate);
        }

        arcs = Op2->arcs;
        while (arcs != NULL) {
            if (arcs->trans == trans) {
                p = DeterminePlIndex(arcs->place, Op2);
                mul = 0;
                simplepl = 0;
                for (i1 = 0; i1 < Npl1; i1++) if (JoinPl[i1][p] == 1) mul++;
                if (mul == 0) {
                    mul++;
                    simplepl = 1;
                }

                for (i2 = 0; i2 < mul; i2++) {
                    mulmul = 1;
                    if (!simplepl) {
                        j = 0; plindex1 = 0;
                        while (j <= i2) if (JoinPl[plindex1++][p] == 1) j++;
                        plindex1--;
                        //printf(" plindex1: %d\n",plindex1);
                        mulmul = PlNeed2(plindex1, p);
                        printf(" p in op2: %d, p in op1: %d, mulmul: %d\n", p, plindex1, mulmul);
                    }
                    for (i3 = 0; i3 < mulmul; i3++) {

                        if (!simplepl) {
                            PlIndex = ComputePlIndex(plindex1, p, i3);
                        }
                        else {
                            PlIndex = MapPl2[p];
                        }
                        place = Result->places;
                        for (l = 0; l < PlIndex; l++) place = place->next;

                        // check if it cause double arc problem;
                        doublearc = 0;
                        if (!simplepl) {
                            a3 = Result->arcs;
                            while (a3 != NULL) {
                                if (a3->trans == tr2 && a3->place == place) {
                                    doublearc = 1;
                                    if (a3->color == NULL && arcs->color == NULL) {
                                        if (a3->mult > 0) a3->mult += arcs->mult;
                                        if (a3->mult < 0) a3->mult -= arcs->mult;
                                    }
                                    else {
                                        if (arcs->color != NULL && a3->color != NULL) {
                                            newcolor = RedefineVariables(nofvar, varset, arcs->color);
                                            strcpy(newpredicate, a3->color);
                                            strcat(newpredicate, "+");
                                            strcat(newpredicate, newcolor);
                                        }
                                        if (arcs->color == NULL && a3->color != NULL) {
                                            strcpy(newpredicate, a3->color);
                                            strcat(newpredicate, "+");
                                            num = lltostr((long)abs(arcs->mult), end);
                                            end[0] = '\0';
                                            strcat(newpredicate, num);
                                        }
                                        if (arcs->color != NULL && a3->color == NULL) {
                                            strcpy(newpredicate, arcs->color);
                                            strcat(newpredicate, "+");
                                            num = lltostr((long)abs(a3->mult), end);
                                            end[0] = '\0';
                                            strcat(newpredicate, num);
                                        }
                                        free(a3->color);
                                        a3->color = (char *)emalloc(sizeof(char) * (strlen(newpredicate) + 1));
                                        strcpy(a3->color, newpredicate);
                                    }

                                    ArcCount++;
                                }
                                a3 = a3->next;
                            }
                        }

                        if (!doublearc) {
                            if (Result->arcs == NULL) {
                                Result->arcs = (struct arc_object *)emalloc(sizeof(struct arc_object));
                                new = Result->arcs;
                                new->next = NULL;
                            }
                            else {
                                new = (struct arc_object *)emalloc(sizeof(struct arc_object));
                                new->next = Result->arcs;
                                Result->arcs = new;
                            }
                            new->trans = tr2;
                            new->place = place;

                            CopyArcProperties(arcs, new, 0);
                            if (new->color != NULL && nofvar > 0) {
                                newcolor = RedefineVariables(nofvar, varset, new->color);
                                free(new->color);
                                new->color = newcolor;
                            }
                            ArcCount++;

                            ccp2 = (struct coordinate *) emalloc(sizeof(struct coordinate));
                            new->point = ccp2;

                            // Adding broken arc properties next to the transition;
                            ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                            ccp2 = ccp2->next;
                            if (arcs->type == 'i') {
                                ArcPosition(&pospl, tr2->brokenin, 0);
                                tr2->brokenin++;
                            }
                            else {
                                ArcPosition(&pospl, tr2->brokenout, 1);
                                tr2->brokenout++;
                            }
                            ccp2->x = tr2->center.x + pospl.x;
                            ccp2->y = tr2->center.y + pospl.y;

                            // Adding broken arc properties next to the place;
                            ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                            ccp2 = ccp2->next;
                            if (simplepl) {
                                ccp1 = arcs->point;
                                while (ccp1->next->next != NULL) ccp1 = ccp1->next;
                                length = (ccp1->x - ccp1->next->x) * (ccp1->x - ccp1->next->x);
                                length += (ccp1->y - ccp1->next->y) * (ccp1->y - ccp1->next->y);
                                length = sqrt(length);
                                ccp2->x = place->center.x + (ccp1->x - ccp1->next->x) / length * SHIFT * 1.5;
                                ccp2->y = place->center.y + (ccp1->y - ccp1->next->y) / length * SHIFT * 1.5;
                            }
                            else {
                                if (arcs->type == 'i') {
                                    ArcPosition(&pospl, place->brokenin, 1);
                                    place->brokenin++;
                                }
                                else {
                                    ArcPosition(&pospl, place->brokenout, 0);
                                    place->brokenout++;
                                }
                                ccp2->x = place->center.x + pospl.x;
                                ccp2->y = place->center.y + pospl.y;
                            }
                            ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                            ccp2->next->next = NULL;

                            if (new->mult > 0)
                                new->mult *= -1;
                        }
                    }
                }
            }
            arcs = arcs->next;
        }

        for (i5 = 0; i5 < nofvar; i5++) free(varset[i5]);
        free(varset);
    }

    for (i1 = 0; i1 < TLength * 2; i1++) free(Translations[i1]);
    free(Translations);
    Translations = NULL;
    TLength = 0;
    free(players);
    free(newpredicate);
    end -= 5;
    free(end);
}

// -----------------------------------
// Join Exponential Transitions Objects
// -----------------------------------
struct trans_object *JoinExpTrans(struct trans_object *trans1, struct trans_object *trans2) {
    struct trans_object *tr1, *tr2, *tr3, *r;
    int first = 1, i, j, k, l, mul, toadd, simple;

    ArcCount = 0;
    GlCountTr1 = 0;
    GlCountTr2 = 0;
    r = NULL;
    NtrR = 0;
    if (trans1 != NULL || trans2 != NULL) {
        tr1 = trans1;
        k = 0;
        while (tr1 != NULL) {
            mul = 0;
            for (j = 0; j < Ntr2; j++) if (JoinTr[k][j] == 1) mul++;
            simple = 1;
            if (mul > 0) {
                simple = 0;
                mul = TrNeed(GlCountTr1);
            }
            else {
                mul++;
            }

            MapTr1[GlCountTr1] = NtrR;
            for (j = 0; j < mul; j++) {
                if (first) {
                    r = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    tr2 = r;
                    first = 0;
                }
                else {
                    tr2->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    tr2 = tr2->next;
                }
                tr2->next = NULL;
                NtrR++;
                CopyTrProperties(tr1, tr2, j, 1);
                tr2->brokenin = 0;
                tr2->brokenout = 0;
                SimpleArcs(tr1, tr2, 1);
                if (!simple) ArcsofArg2(tr2, GlCountTr1, j);
            }
            k++;
            tr1 = tr1->next;
            GlCountTr1++;
        }
        tr1 = trans2;
        k = 0;
        while (tr1 != NULL) {
            toadd = 1;
            for (j = 0; j < Ntr1; j++) if (JoinTr[j][k] == 1) toadd = 0;
            if (toadd) {
                MapTr2[GlCountTr2] = NtrR;
                if (first) {
                    r = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    tr2 = r;
                    first = 0;
                    NtrR++;
                }
                else {
                    tr2->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    tr2 = tr2->next;
                    NtrR++;
                }
                tr2->next = NULL;
                CopyTrProperties(tr1, tr2, 0, 0);
                tr2->brokenin = 0;
                tr2->brokenout = 0;
                SimpleArcs(tr1, tr2, 0);
            }
            k++;
            tr1 = tr1->next;
            GlCountTr2++;
        }
    }

#ifdef DEBUG
    printf("  Exp Transitions in Result:\n");
    tr1 = r;
    j = 0;
    while (tr1 != NULL) {
        printf(" %d.   %s: ", j, tr1->tag);
        if (tr1->color != NULL)
            printf(" %s ", tr1->color);
        printf(" Layers: ");
        for (i = 1; i <= Nlayer1 + Nlayer2; i++)
            if (TestLayer(i, tr1->layer)) printf(" %s", GetLayerName(i));
        tr1 = tr1->next;
        j++;
        printf("\n");
    }
    printf("\n");
#endif
    return r;
}

// ---------------------
// Copy Place Properties
// ---------------------
void CopyPlProperties(struct place_object *pl1, struct place_object *pl2, int j, int in_op1) {
    struct lisp_object *li;
    struct mpar_object *mp;
    int i;

    pl2->tag = (char *)emalloc(sizeof(char) * (strlen(pl1->tag) + 1));
    strcpy(pl2->tag, pl1->tag);

    if (pl1->color != NULL) {
        pl2->color = (char *)emalloc(sizeof(char) * (strlen(pl1->color) + 1));
        strcpy(pl2->color, pl1->color);
    }
    else {
        pl2->color = NULL;
    }
    pl2->center.x = pl1->center.x + j * 1.2 * SHIFT + shiftx * (1 - in_op1);
    pl2->center.y = pl1->center.y + j * SHIFT + shifty * (1 - in_op1);
    pl2->tagpos.x = pl1->tagpos.x;
    pl2->tagpos.y = pl1->tagpos.y;
    pl2->colpos.x = pl1->colpos.x;
    pl2->colpos.y = pl1->colpos.y;
    pl2->tokens = pl1->tokens;
    pl2->m0 = pl1->m0;
    pl2->cmark = NULL;
    if (pl1->cmark != NULL) {
        li = Result->lisps;
        while (strcmp(li->tag, pl1->cmark->tag) != 0) li = li->next;
        pl2->cmark = li;
    }
    pl2->lisp = NULL;
    if (pl1->lisp != NULL) {
        li = Result->lisps;
        while (strcmp(li->tag, pl1->lisp->tag) != 0) li = li->next;
        pl2->lisp = li;
    }
    pl2->mpar = NULL;
    if (pl1->mpar != NULL) {
        mp = Result->mpars;
        while (strcmp(mp->tag, pl1->mpar->tag) != 0) mp = mp->next;
        pl2->mpar = mp;
    }
    pl2->next = NULL;

    pl2->layer = NewLayerList(WHOLENET, NULL);
    for (i = 1; i <= Nlayer2; i++)
        if (TestLayer(i, pl1->layer))
            SetLayer2LayerList(i + Nlayer2 * in_op1, pl2->layer);
    pl2->next = NULL;
}


// =====================
// Join Colors of places
// =====================
char *JoinPlColors(char *str1, char *str2) {
    char *result, *newcolor, *var1, *var2;
    int pos1, pos2, varpos1, varpos2, newvar, toadd;

    newcolor = (char *)emalloc(sizeof(char) * 1000);
    var1 = (char *)emalloc(sizeof(char) * 100);
    var2 = (char *)emalloc(sizeof(char) * 100);

    strcpy(newcolor, str1);
    if (str1 != NULL) {
        pos2 = 0;
        while (str2[pos2] != '\0') {
            while (str2[pos2] == ',' || str2[pos2] == ' ') pos2++;
            varpos2 = 0;
            newvar = 0;
            while (str2[pos2] != '\0' && str2[pos2] != ',' && str2[pos2] != ' ') {
                var2[varpos2] = str2[pos2];
                varpos2++;
                pos2++;
                newvar = 1;
            }
            var2[varpos2] = '\0';
            if (newvar) printf(" Var2 is %s\n", var2);
            if (newvar) {
                pos1 = 0;
                toadd = 1;
                while (str1[pos1] != '\0') {
                    while (str1[pos1] == ',' || str1[pos1] == ' ') pos1++;
                    varpos1 = 0;
                    newvar = 0;
                    while (str1[pos1] != '\0' && str1[pos1] != ',' && str1[pos1] != ' ') {
                        var1[varpos1] = str1[pos1];
                        varpos1++;
                        pos1++;
                        newvar = 1;
                    }
                    var1[varpos1] = '\0';
                    if (newvar) printf(" Var1 is %s\n", var1);
                    if (newvar)
                        if (strcmp(var1, var2) == 0) toadd = 0;
                }
                if (toadd) {
                    strcat(newcolor, ",");
                    strcat(newcolor, var2);
                }
            }
        }
    }

    result = (char *)emalloc(sizeof(char) * (strlen(newcolor) + 1));
    strcpy(result, newcolor);
    free(newcolor);
    free(var1);
    free(var2);

    return result;
}

// ---------------------------------------
// Join Places Objects According to JoinPl
// ---------------------------------------
struct place_object *JoinPlaces(struct place_object *place1, struct place_object *place2) {
    struct place_object *pl1, *pl2, *r, *tmp, *tmp2;
    int first = 1, i, j, j2, j3, index, k, l, mul, toadd, simple, GlCountPl1 = 0, GlCountPl2 = 0;
    char *newcolor;

    r = NULL;
    NplR = 0;
    if (place1 != NULL || place2 != NULL) {
        pl1 = place1;
        k = 0;
        while (pl1 != NULL) {
            mul = 0;
            for (j = 0; j < Npl2; j++) if (JoinPl[k][j] == 1) mul++;
            simple = 1;
            if (mul > 0) {
                simple = 0;
                mul = PlNeed(GlCountPl1);
            }
            else {
                mul++;
            }

            MapPl1[GlCountPl1] = NplR;
            for (j = 0; j < mul; j++) {
                if (first) {
                    r = (struct place_object *)emalloc(sizeof(struct place_object));
                    pl2 = r;
                    first = 0;
                    NplR++;
                }
                else {
                    pl2->next = (struct place_object *)emalloc(sizeof(struct place_object));
                    pl2 = pl2->next;
                    NplR++;
                }
                CopyPlProperties(pl1, pl2, j, 1);
                pl2->brokenin = 0;
                pl2->brokenout = 0;
            }

            // Join color defininitions;
            tmp = Op2->places;
            j = 0;
            //printf(" GlCountPl1: %d, k: %d\n", GlCountPl1,k);
            while (tmp != NULL) {
                //printf(" hehe %d\n", JoinPl[GlCountPl1][j]);
                if (JoinPl[GlCountPl1][j] == 1) {
                    mul = PlNeed2(GlCountPl1, j);
                    printf(" nul: %d\n", mul);
                    for (j2 = 0; j2 < mul; j2++) {
                        index = ComputePlIndex(GlCountPl1, j, j2);
                        tmp2 = r;
                        for (j3 = 0; j3 < index; j3++) tmp2 = tmp2->next;
                        printf(" tr: %s\n", tmp2->tag);
                        if (tmp->color != NULL) {
                            newcolor = JoinPlColors(tmp2->color, tmp->color);
                            free(tmp2->color);
                            tmp2->color = newcolor;
                        }
                    }
                }
                j++;
                tmp = tmp->next;
            }

            k++;
            pl1 = pl1->next;
            GlCountPl1++;
        }
        pl1 = place2;
        k = 0;
        while (pl1 != NULL) {
            toadd = 1;
            for (j = 0; j < Npl1; j++) if (JoinPl[j][k] == 1) toadd = 0;
            if (toadd) {
                MapPl2[GlCountPl2] = NplR;
                if (first) {
                    r = (struct place_object *)emalloc(sizeof(struct place_object));
                    pl2 = r;
                    first = 0;
                    NplR++;
                }
                else {
                    pl2->next = (struct place_object *)emalloc(sizeof(struct place_object));
                    pl2 = pl2->next;
                    NplR++;
                }
                CopyPlProperties(pl1, pl2, 0, 0);
            }
            k++;
            pl1 = pl1->next;
            GlCountPl2++;
        }
    }

#ifdef DEBUG
    printf("Npl1: %d, Npl2: %d, NplR: %d\n", Npl1, Npl2, NplR);
    printf("  Places in Result:\n");
    pl1 = r;
    j = 0;
    while (pl1 != NULL) {
        printf(" %d.   %s: ", j, pl1->tag);
        if (pl1->color != NULL)
            printf(" %s ", pl1->color);
        printf(" Layers: ");
        for (i = 1; i <= Nlayer1 + Nlayer2; i++)
            if (TestLayer(i, pl1->layer)) printf(" %s", GetLayerName(i));
        pl1 = pl1->next;
        j++;
        printf("\n");
    }
    printf("\n");
    printf("  Mapping of places s:\n");
    for (i = 0; i < Npl1; i++) printf("     %d %d\n", i, MapPl1[i]);
    printf("\n");
    for (i = 0; i < Npl2; i++) printf("     %d %d\n", i, MapPl2[i]);
#endif

    return r;
}


// -----------------------------------
// Join Immitdiate Transitions Objects
// -----------------------------------
struct group_object *JoinImmTrans(struct group_object *group1, struct group_object *group2) {
    struct trans_object *tr1, *tr2, *tr3;
    struct group_object *gr1, *gr2, *gr3, *gr4, *r;
    struct lisp_object *li;
    struct rpar_object *mp;
    int first = 1, i, j, l, *k, mul, toadd, c1, c2, newgroup, firstgroup = 1, simple;

    r = NULL;
    c1 = GlCountTr1;
    c2 = GlCountTr2;

    printf(" Imm Start\n");

    if (group1 != NULL || group2 != NULL) {
        // look for the first non-empty group;
        gr1 = group1;
        gr2 = group2;

        while (gr1 != NULL && gr1->trans == NULL) gr1 = gr1->next;
        while (gr2 != NULL && gr2->trans == NULL) gr2 = gr2->next;

        while (gr1 != NULL || gr2 != NULL) {
            if (gr1 == NULL) {
                gr3 = gr2;
                k = &c2;
            }
            if (gr2 == NULL) {
                gr3 = gr1;
                k = &c1;
            }
            if (gr1 != NULL && gr2 != NULL) {
                gr3 = gr1;
                k = &c1;
                if (gr1->pri > gr2->pri) {
                    gr3 = gr2;
                    k = &c2;
                }
            }
            if (firstgroup) {
                r = (struct group_object *)emalloc(sizeof(struct group_object));
                gr4 = r;
                gr4->trans = (struct trans_object *)emalloc(sizeof(struct trans_object));
                tr2 = gr4->trans;
                firstgroup = 0;
                first = 1;
            }
            else {
                if (gr4->pri < gr3->pri) {
                    gr4->next = (struct group_object *)emalloc(sizeof(struct group_object));
                    gr4 = gr4->next;
                    gr4->trans = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    tr2 = gr4->trans;
                    first = 1;
                }
            }
            gr4->pri = gr3->pri;
            gr4->tag = (char *)emalloc((strlen(gr3->tag) + 1) * sizeof(char));
            strcpy(gr4->tag, gr3->tag);
            gr4->next = NULL;

            tr1 = gr3->trans;
            while (tr1 != NULL) {
                mul = 0;
                simple = 1;
                if (gr3 == gr1) {
                    for (j = 0; j < Ntr2; j++) if (JoinTr[*k][j] == 1) mul++;
                    MapTr1[GlCountTr1] = NtrR;
                    if (mul > 0) {
                        simple = 0;
                        mul = TrNeed(GlCountTr1);
                    }
                    else {
                        mul++;
                    }
                }
                if (gr3 == gr2) {
                    for (j = 0; j < Ntr1; j++) if (JoinTr[j][*k] == 1) mul++;
                    mul = (mul == 0);
                    MapTr2[GlCountTr2] = NtrR;
                }
                NtrR += mul;
                for (j = 0; j < mul; j++) {
                    if (!first) {
                        tr2->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                        tr2 = tr2->next;
                    }
                    first = 0;
                    CopyTrProperties(tr1, tr2, j, gr3 != gr2);
                    tr2->brokenin = 0;
                    tr2->brokenout = 0;
                    if (gr3 == gr1) {
                        SimpleArcs(tr1, tr2, 1);
                        if (!simple) ArcsofArg2(tr2, GlCountTr1, j);
                    }
                    else {
                        if (simple) SimpleArcs(tr1, tr2, 0);
                    }
                }
                (*k)++;
                tr1 = tr1->next;
                if (gr3 == gr1) GlCountTr1++;
                if (gr3 == gr2) GlCountTr2++;
            }

            // step to an non-empty group
            if (gr3 == gr2) {
                gr2 = gr2->next;
                while (gr2 != NULL && gr2->trans == NULL)
                    gr2 = gr2->next;
            }
            if (gr3 == gr1) {
                gr1 = gr1->next;
                while (gr1 != NULL && gr1->trans == NULL)
                    gr1 = gr1->next;
            }
            if (gr3 == gr1) GlCountTr1++;
            if (gr3 == gr2) GlCountTr2++;
        }
    }

#ifdef DEBUG
    printf("  Imm Transitions in Result:\n");
    gr1 = r;
    j = 1;
    while (gr1 != NULL) {
        printf("   %s: %d\n", gr1->tag, gr1->pri);
        tr1 = gr1->trans;
        while (tr1 != NULL) {
            printf(" %d.   %s: ", j, tr1->tag);
            if (tr1->color != NULL)
                printf(" %s ", tr1->color);
            printf(" Layers: ");
            for (i = 1; i <= Nlayer1 + Nlayer2; i++)
                if (TestLayer(i, tr1->layer)) printf(" %s", GetLayerName(i));
            tr1 = tr1->next;
            j++;
            printf("\n");
        }
        gr1 = gr1->next;
    }
    printf("\n");
    printf(" Number of tr.s: %d\n", NtrR);
    printf("  Mapping of transitions s:\n");
    for (i = 0; i < Ntr1; i++) printf("     %d %d\n", i, MapTr1[i]);
    printf("\n");
    for (i = 0; i < Ntr2; i++) printf("     %d %d\n", i, MapTr2[i]);
#endif

    return r;
}

// -------------------------------------------
// Coordinate of shift positioning broken arcs
// -------------------------------------------
struct position DefineShift(int count) {
    struct position p;
    int mul;

    mul = (int)floor((double)count / 8) + 1;
    p.x = 0;
    if (count % 8 == 1 || count % 8 == 2 || count % 8 == 3) p.x = mul;
    if (count % 8 == 7 || count % 8 == 6 || count % 8 == 5) p.x = -mul;
    p.y = 0;
    if (count % 8 == 7 || count % 8 == 0 || count % 8 == 1) p.y = -mul;
    if (count % 8 == 5 || count % 8 == 4 || count % 8 == 3) p.y = mul;
    if (count % 8 == 1 || count % 8 == 3 || count % 8 == 5 || count % 8 == 7) {
        p.x *= 0.8;
        p.y *= 0.6;
    }

    return p;
}

// -----------------------------
// Determine index of transition
// -----------------------------
int DetermineTrIndex(struct trans_object *tofind, struct net_object *n) {
    int tr = 0;
    struct trans_object *t;
    struct group_object *g;

    t = n->trans;
    while (t != NULL) {
        if (t == tofind) return tr;
        tr++;
        t = t->next;
    }
    g = n->groups;
    while (g != NULL) {
        t = g->trans;
        while (t != NULL) {
            if (t == tofind) return tr;
            tr++;
            t = t->next;
        }
        g = g->next;
    }

    return tr;
}


// ---------------------------------------------
// Compute Shift with respect to tr in operand 1
// ---------------------------------------------
int ComputeTrIndex(int i1, int i2, int c) {
    int index, i, pos, m;
    int *count, *run, blabla = 0;
    struct Desc *d1, *d2;

    d1 = Op1TrD;
    for (i = 0; i < i1; i++) d1 = d1->next;
    d2 = Op2TrD;
    for (i = 0; i < i2; i++) d2 = d2->next;
    pos = 0;
    while (strcmp(d2->labels[0], d1->labels[pos++]) != 0);
    pos--;
    m = 1;
    d2 = Op2TrD;
    i = 0;
    while (i < i2) {
        if (JoinTr[i1][i] == 1) {
            if (strcmp(d1->labels[pos], d2->labels[0]) == 0) m++;
        }
        d2 = d2->next;
        i++;
    }

    count = (int *)emalloc(sizeof(int) * d1->nl);
    run = (int *)emalloc(sizeof(int) * d1->nl);
    for (i = 0; i < d1->nl; i++) count[i] = 0;
    for (i = 0; i < d1->nl; i++) run[i] = 1;
    d2 = Op2TrD;
    while (d2 != NULL) {
        for (i = 0; i < d1->nl; i++)
            if (d2->nl > 0)
                if (strcmp(d1->labels[i], d2->labels[0]) == 0) count[i]++;
        d2 = d2->next;
    }
    for (i = 0; i < d1->nl; i++) if (count[i] == 0) count[i] = 1;

    index = -1;
    while (blabla <= c) {
        if (run[pos] == m) blabla++;

        i = 0;
        while (i < d1->nl) {
            run[i] = (run[i] % count[i]) + 1;
            if (run[i] != 1) break;
            i++;
        }
        index++;
    }

    free(count);
    free(run);
    return index + MapTr1[i1];
}


// ==========================================
// Ez itt mar nem hasznalatos, baro regi cucc
// ---------------------
// Build Arcs for Result
// ---------------------
struct arc_object *Build_Arcs(struct arc_object *arc1, struct arc_object *arc2) {
    struct arc_object *a1, *a2, *a3, *r;
    struct place_object *p, *p2, *p3, *dp;
    struct trans_object *t, *t2, *t3, *dt;
    struct coordinate *ccp1, *ccp2;
    struct position pospl, postr;
    int first = 1, mul2, tr, pl, np, i, ii, iii, iiii, j, k, l, tomap, trindex1, plindex1;
    int multr, mulpl, simpletr, simplepl, realmultr, realmulpl, doublearc;
    int TrIndex, PlIndex;
    double length;
    int *PosTr, *PosPl;              // Position of last broken arc around transitions and places;

    PosTr = (int *)emalloc(NtrR * sizeof(int));
    for (i = 0; i < NtrR; i++) PosTr[i] = 1;
    PosPl = (int *)emalloc(NplR * sizeof(int));
    for (i = 0; i < NplR; i++) PosPl[i] = 1;

    r = NULL;
    if (arc1 != NULL || arc2 != NULL) {
        a1 = arc1;
        // adding arcs of Operand1
        while (a1 != NULL) {
            tr = DetermineTrIndex(a1->trans, Op1);
            pl = DeterminePlIndex(a1->place, Op1);
            // printf("\nPl: %s, Tr: %s, tr: %d, pl: %d\n", a1->place->tag, a1->trans->tag,tr,pl);

            multr = TrNeed(tr);
            mulpl = PlNeed(pl);
            // printf("\t\t multr: %d, mulpl: %d\n", multr, mulpl);

            for (i = 0; i < multr; i++)
                for (ii = 0; ii < mulpl; ii++) {
                    if (first) {
                        r = (struct arc_object *)emalloc(sizeof(struct arc_object));
                        a2 = r;
                        first = 0;
                    }
                    else {
                        a2->next = (struct arc_object *)emalloc(sizeof(struct arc_object));
                        a2 = a2->next;
                    }
                    a2->next = 0;

                    // looking for its place
                    p = Result->places;
                    for (j = 0; j < MapPl1[pl] + ii; j++) p = p->next;
                    a2->place = p;
                    // printf("  PlIndex: %d, Its place: %s\n", pl, a2->place->tag);

                    // looking for its trans
                    t2 = GotoTrIndex(MapTr1[tr] + i, Result);
                    a2->trans = t2;
                    // printf("  TrIndex: %d, Its trans: %s\n", tr, a2->trans->tag);

                    CopyArcProperties(a1, a2, 1);

                    np = 0;
                    ccp1 = a1->point;
                    while (ccp1 != NULL) { ccp1 = ccp1->next; np++;}
                    a2->point = (struct coordinate *) emalloc(sizeof(struct coordinate));
                    ccp2 = a2->point;
                    ccp1 = a1->point->next;
                    for (j = 0; j < np - 2; j++) {
                        ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                        ccp2 = ccp2->next;
                        ccp2->x = ccp1->x;
                        ccp2->y = ccp1->y;
                        ccp1 = ccp1->next;
                    }
                    ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                    ccp2 = ccp2->next;
                    ccp2->next = 0;
                }

            a1 = a1->next;
        }
        a1 = arc2;
        // adding arcs of Operand2;
        while (a1 != NULL) {
            tr = DetermineTrIndex(a1->trans, Op2);
            pl = DeterminePlIndex(a1->place, Op2);
            // printf("\nPl: %s, Tr: %s, tr: %d, pl: %d\n", a1->place->tag, a1->trans->tag,tr,pl);

            multr = 0;
            for (i = 0; i < Ntr1; i++) if (JoinTr[i][tr] == 1) multr++;
            mulpl = 0;
            for (i = 0; i < Npl1; i++) if (JoinPl[i][pl] == 1) mulpl++;

            simpletr = 0;
            simplepl = 0;
            if (multr == 0) {
                multr++;
                simpletr = 1;
            }
            if (mulpl == 0) {
                mulpl++;
                simplepl = 1;
            }

            // printf("multr: %d, simpletr:%d, mulpl: %d, simplepl: %d, \n", multr, simpletr, mulpl, simplepl);

            for (i = 0; i < multr; i++)
                for (ii = 0; ii < mulpl; ii++) {
                    realmultr = 1;
                    if (!simpletr) {
                        j = 0; trindex1 = 0;
                        while (j <= i) if (JoinTr[trindex1++][tr] == 1) j++;
                        trindex1--;
                        //printf(" trindex1: %d\n",trindex1);
                        realmultr = TrNeed2(trindex1, tr);
                        // printf(" realmultr: %d\n", realmultr);
                    }
                    realmulpl = 1;
                    if (!simplepl) {
                        j = 0; plindex1 = 0;
                        while (j <= ii) if (JoinPl[plindex1++][pl] == 1) j++;
                        plindex1--;
                        //printf(" plindex1: %d\n",plindex1);
                        realmulpl = PlNeed2(plindex1, pl);
                        // printf(" realmulpl: %d\n", realmulpl);
                    }
                    for (iii = 0; iii < realmultr; iii++)
                        for (iiii = 0; iiii < realmulpl; iiii++) {
                            // looking for its place;
                            if (!simplepl) {
                                PlIndex = ComputePlIndex(plindex1, pl, iiii);
                            }
                            else {
                                PlIndex = MapPl2[pl];
                            }
                            p = Result->places;
                            for (l = 0; l < PlIndex; l++) p = p->next;

                            // looking for its trans;
                            if (!simpletr) {
                                TrIndex = ComputeTrIndex(trindex1, tr, iii);
                                t2 = GotoTrIndex(TrIndex, Result);
                                //printf("Goes to this Transition Index: %d\n", TrIndex);
                            }
                            else {
                                t2 = GotoTrIndex(MapTr2[tr], Result);
                                TrIndex = MapTr2[tr];
                            }

                            // check if it cause double arc problem;
                            doublearc = 0;
                            if (!simplepl && !simpletr) {
                                a3 = r;
                                while (a3 != NULL) {
                                    if (a3->trans == t2 && a3->place == p) {
                                        doublearc = 1;
                                        a3->mult += (a3->mult > 0) - (a3->mult < 0);
                                    }
                                    a3 = a3->next;
                                }
                            }

                            if (!doublearc) {
                                if (first) {
                                    r = (struct arc_object *)emalloc(sizeof(struct arc_object));
                                    a2 = r;
                                    first = 0;
                                }
                                else {
                                    a2->next = (struct arc_object *)emalloc(sizeof(struct arc_object));
                                    a2 = a2->next;
                                }
                                a2->next = NULL;
                                a2->trans = t2;
                                // printf("  TrIndex: %d, Its trans: %s\n", TrIndex, a2->trans->tag);
                                a2->place = p;
                                // printf("  PlIndex: %d, Its place: %s\n", PlIndex, a2->place->tag);

                                CopyArcProperties(a1, a2, 0);

                                np = 0;
                                ccp1 = a1->point;
                                while (ccp1 != NULL) { ccp1 = ccp1->next; np++;}
                                a2->point = (struct coordinate *) emalloc(sizeof(struct coordinate));
                                ccp2 = a2->point;
                                ccp1 = a1->point->next;
                                if (simpletr && simplepl) {
                                    for (j = 0; j < np - 2; j++) {
                                        ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                                        ccp2 = ccp2->next;
                                        ccp2->x = ccp1->x + shiftx;
                                        ccp2->y = ccp1->y + shifty;
                                        ccp1 = ccp1->next;
                                    }
                                }
                                else {
                                    ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                                    ccp2 = ccp2->next;
                                    postr = DefineShift(PosTr[TrIndex]);
                                    pospl = DefineShift(PosPl[PlIndex]);

                                    if (!simpletr && !simplepl) {
                                        ccp2->x = t2->center.x + SHIFT * postr.x;
                                        ccp2->y = t2->center.y + SHIFT * postr.y;
                                        PosTr[TrIndex]++;
                                    }
                                    else {
                                        if (!simplepl) {
                                            length = (a1->point->next->x - a1->point->x) * (a1->point->next->x - a1->point->x);
                                            length += (a1->point->next->y - a1->point->y) * (a1->point->next->y - a1->point->y);
                                            length = sqrt(length);
                                            ccp2->x = t2->center.x + (a1->point->next->x - a1->point->x) / length * SHIFT * 1.5;
                                            ccp2->y = t2->center.y + (a1->point->next->y - a1->point->y) / length * SHIFT * 1.5;
                                        }
                                        else {
                                            ccp2->x = t2->center.x + SHIFT * postr.x;
                                            ccp2->y = t2->center.y + SHIFT * postr.y;
                                            PosTr[TrIndex]++;
                                        }
                                    }
                                    ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                                    ccp2 = ccp2->next;

                                    if (!simpletr && !simplepl) {
                                        ccp2->x = p->center.x + SHIFT * pospl.x;
                                        ccp2->y = p->center.y + SHIFT * pospl.y;
                                        PosPl[PlIndex]++;
                                    }
                                    else {
                                        if (!simplepl) {
                                            ccp2->x = p->center.x + SHIFT * pospl.x;
                                            ccp2->y = p->center.y + SHIFT * pospl.y;
                                            PosPl[PlIndex]++;
                                        }
                                        else {
                                            ccp1 = a1->point;
                                            while (ccp1->next->next != NULL) ccp1 = ccp1->next;
                                            length = (ccp1->x - ccp1->next->x) * (ccp1->x - ccp1->next->x);
                                            length += (ccp1->y - ccp1->next->y) * (ccp1->y - ccp1->next->y);
                                            length = sqrt(length);
                                            ccp2->x = p->center.x + (ccp1->x - ccp1->next->x) / length * SHIFT * 1.5;
                                            ccp2->y = p->center.y + (ccp1->y - ccp1->next->y) / length * SHIFT * 1.5;
                                        }
                                    }
                                    if (a2->mult > 0)
                                        a2->mult *= -1;
                                }
                                ccp2->next = (struct coordinate *) emalloc(sizeof(struct coordinate));
                                ccp2 = ccp2->next;
                                ccp2->next = 0;
                            }
                        }
                }
            a1 = a1->next;
        }
    }

    free(PosPl);
    free(PosTr);

#ifdef DEBUG
    a1 = r;
    printf(" Arcs:\n");
    while (a1 != NULL) {
        printf("  Place: %s, Transition: %s, type: %c\n", a1->place->tag, a1->trans->tag, a1->type);
        a1 = a1->next;
    }
#endif

    return r;
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

// -----------
// Common part
// -----------
void CommonStart(void) {
    int i, j;

    Op1PlD = NULL;
    Op2PlD = NULL;
    Op1TrD = NULL;
    Op2TrD = NULL;

    Ntr1 = CountTransitions(Op1);
    Ntr2 = CountTransitions(Op2);
    Npl1 = CountPlaces(Op1);
    Npl2 = CountPlaces(Op2);

    Result->comment = ConcComments(Op1->comment, Op2->comment);
    Result->mpars = JoinMPars(Op1->mpars, Op2->mpars);
    Result->rpars = JoinRPars(Op1->rpars, Op2->rpars);
    Result->lisps = JoinLisps(Op1->lisps, Op2->lisps);

    JoinTr = (int **)emalloc(Ntr1 * sizeof(int));
    for (i = 0; i < Ntr1; i++) {
        JoinTr[i] = (int *)emalloc(sizeof(int) * Ntr2);
        for (j = 0; j < Ntr2; j++) JoinTr[i][j] = 0;
    }
    JoinPl = (int **)emalloc(Npl1 * sizeof(int));
    for (i = 0; i < Npl1; i++) {
        JoinPl[i] = (int *)emalloc(sizeof(int) * Npl2);
        for (j = 0; j < Npl2; j++) JoinPl[i][j] = 0;
    }

    MapTr1 = (int *)emalloc(sizeof(int) * Ntr1);
    for (i = 0; i < Ntr1; i++) MapTr1[i] = 0;
    MapTr2 = (int *)emalloc(sizeof(int) * Ntr2);
    for (i = 0; i < Ntr2; i++) MapTr2[i] = 0;
    MapPl1 = (int *)emalloc(sizeof(int) * Npl1);
    for (i = 0; i < Npl1; i++) MapPl1[i] = 0;
    MapPl2 = (int *)emalloc(sizeof(int) * Npl2);
    for (i = 0; i < Npl2; i++) MapPl2[i] = 0;

}

// -------
// Destroy
// -------
void CommonDestroy(void) {
    int i, j;

    // destroy structures;
    while (Op2PlD != NULL) {
        for (i = 0; i < Op2PlD->nl; i++) free(Op2PlD->labels[i]);
        if (Op2PlD->tag != NULL) free(Op2PlD->tag);
        if (Op2PlD->labels != NULL) free(Op2PlD->labels);
        if (Op2PlD->end != NULL) free(Op2PlD->end);
        Op2PlD = Op2PlD->next;
    }
    while (Op1PlD != NULL) {
        for (i = 0; i < Op1PlD->nl; i++) free(Op1PlD->labels[i]);
        if (Op1PlD->tag != NULL) free(Op1PlD->tag);
        if (Op1PlD->labels != NULL) free(Op1PlD->labels);
        if (Op1PlD->end != NULL) free(Op1PlD->end);
        Op1PlD = Op1PlD->next;
    }

    while (Op2TrD != NULL) {
        for (i = 0; i < Op2TrD->nl; i++) free(Op2TrD->labels[i]);
        if (Op2TrD->tag != NULL) free(Op2TrD->tag);
        if (Op2TrD->labels != NULL) free(Op2TrD->labels);
        if (Op2TrD->end != NULL) free(Op2TrD->end);
        Op2TrD = Op2TrD->next;
    }
    while (Op1TrD != NULL) {
        for (i = 0; i < Op1TrD->nl; i++) free(Op1TrD->labels[i]);
        if (Op1TrD->tag != NULL) free(Op1TrD->tag);
        if (Op1TrD->labels != NULL) free(Op1TrD->labels);
        if (Op1TrD->end != NULL) free(Op1TrD->end);
        Op1TrD = Op1TrD->next;
    }

    for (i = 0; i < Ntr1; i++) free(JoinTr[i]);
    free(JoinTr);
    for (i = 0; i < Npl1; i++) free(JoinPl[i]);
    free(JoinPl);
    free(MapTr1);
    free(MapTr2);
    free(MapPl1);
    free(MapPl2);
}

// ------------------------------------------------
// Join Transitions of Op1 and Op2, write result to R
// ------------------------------------------------
void Transitions(void) {
    int i, j;

    printf(" Superposition over Transitions Started\n");

    CommonStart();

#ifdef DEBUG
    printf("    Ntr1: %d, Ntr2: %d\n", Ntr1, Ntr2);
    printf(" Transitions of 1st Operand\n");
#endif

    Op1TrD = FillDescTr(Op1->trans, Op1->groups);

#ifdef DEBUG
    printf(" Transitions of 2nd Operand\n");
#endif

    Op2TrD = FillDescTr(Op2->trans, Op2->groups);

    FillJoinTr(JoinTr, Op1TrD, Op2TrD);

#ifdef DEBUG
    printf(" Join Matrice:\n      ");
    for (i = 0; i < Ntr1; i++) {
        for (j = 0; j < Ntr2; j++) printf("%d ", JoinTr[i][j]);
        printf("\n      ");
    }
    printf("\n");
#endif

    Result->places = JoinPlaces(Op1->places, Op2->places);
    Result->trans = JoinExpTrans(Op1->trans, Op2->trans);
    Result->groups = JoinImmTrans(Op1->groups, Op2->groups);
    //Result->arcs=Build_Arcs(Op1->arcs, Op2->arcs);

    CommonDestroy();

    printf(" Superposition over Transitions Finished\n");
}


// ------------------------------------------------
// Join Places of Op1 and Op2, write result to R
// ------------------------------------------------
void Places(void) {
    int i, j;

    printf(" Superposition over Places Started\n");

    CommonStart();


#ifdef DEBUG
    printf("    Npl1: %d, Npl2: %d\n", Npl1, Npl2);
    printf(" Places of 1st Operand\n");
#endif

    Op1PlD = FillDescPl(Op1->places);

#ifdef DEBUG
    printf(" Places of 2nd Operand\n");
#endif

    Op2PlD = FillDescPl(Op2->places);

    FillJoinPl(JoinPl, Op1PlD, Op2PlD);

#ifdef DEBUG
    printf(" Join Matrice:\n      ");
    for (i = 0; i < Npl1; i++) {
        for (j = 0; j < Npl2; j++) printf("%d ", JoinPl[i][j]);
        printf("\n      ");
    }
    printf("\n");
#endif

    Result->places = JoinPlaces(Op1->places, Op2->places);
    Result->trans = JoinExpTrans(Op1->trans, Op2->trans);
    Result->groups = JoinImmTrans(Op1->groups, Op2->groups);
    //Result->arcs=Build_Arcs(Op1->arcs, Op2->arcs);

    CommonDestroy();

    printf(" Superposition over Places Finished\n");
}

// ---------------------------------------------------------
// Join Places & Transitions of Op1 and Op2, write result to R
// ---------------------------------------------------------
void Transitions_Places() {
    int i, j;

    printf(" Superposition over Places & Transitions Started\n");

    CommonStart();

#ifdef DEBUG
    printf("    Ntr1: %d, Ntr2: %d\n", Ntr1, Ntr2);
    printf(" Transitions of 1st Operand\n");
#endif
    Op1TrD = FillDescTr(Op1->trans, Op1->groups);

#ifdef DEBUG
    printf(" Transitions of 2nd Operand\n");
#endif
    Op2TrD = FillDescTr(Op2->trans, Op2->groups);

    FillJoinTr(JoinTr, Op1TrD, Op2TrD);

#ifdef DEBUG
    printf(" Join Matrice:\n      ");
    for (i = 0; i < Ntr1; i++) {
        for (j = 0; j < Ntr2; j++) printf("%d ", JoinTr[i][j]);
        printf("\n      ");
    }
    printf("\n");
#endif

#ifdef DEBUG
    printf("    Npl1: %d, Npl2: %d\n", Npl1, Npl2);
    printf(" Places of 1st Operand\n");
#endif
    Op1PlD = FillDescPl(Op1->places);

#ifdef DEBUG
    printf(" Places of 2nd Operand\n");
#endif
    Op2PlD = FillDescPl(Op2->places);

    FillJoinPl(JoinPl, Op1PlD, Op2PlD);

#ifdef DEBUG
    printf(" Join Matrice:\n      ");
    for (i = 0; i < Npl1; i++) {
        for (j = 0; j < Npl2; j++) printf("%d ", JoinPl[i][j]);
        printf("\n      ");
    }
    printf("\n");
#endif

    Result->places = JoinPlaces(Op1->places, Op2->places);
    Result->trans = JoinExpTrans(Op1->trans, Op2->trans);
    Result->groups = JoinImmTrans(Op1->groups, Op2->groups);
    //Result->arcs=Build_Arcs(Op1->arcs, Op2->arcs);

    CommonDestroy();

    printf(" Superposition over Transitions & Places Finished\n");
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
            lexstr = ss;
            yyparse();
        }
    }

    printf("\n  Synchronization will be performed over the labels:\n");
    printf("   Places: ");
    for (i = 0; i < PlaceRest.nl; i++) printf("'%s', ", PlaceRest.labels[i]);
    if (!Pl_Rest) printf(" No restrictions applied");
    printf("\n");
    printf("   Transitions: ");
    for (i = 0; i < TransRest.nl; i++) printf("'%s', ", TransRest.labels[i]);
    if (!Tr_Rest) printf(" No restrictions applied");
    printf("\n\n");

    free(ss);
    fclose(f);
}

// ----------------------------------------------
// Read Operands and build common layer structure
// ----------------------------------------------
void ReadOperands(char *op1, char *op2) {
    int i;
    char **NameLayer1, **NameLayer2;    // Name of layers in nets;

    // read Operand1;
    netobj = Op1;
    read_file(op1);
    Nlayer1 = HowManyLayers();
    if (Nlayer1 > 0) {
        NameLayer1 = (char **)emalloc(sizeof(char *)*Nlayer1);
        for (i = 0; i < Nlayer1; i++) {
            NameLayer1[i] = (char *)emalloc(sizeof(char) * (strlen(GetLayerName(i + 1)) + 1));
            strcpy(NameLayer1[i], GetLayerName(i + 1));
        }
    }

    // read Operand2;
    netobj = Op2;
    read_file(op2);
    Nlayer2 = HowManyLayers();
    if (Nlayer2 > 0) {
        NameLayer2 = (char **)emalloc(sizeof(char *)*Nlayer2);
        for (i = 0; i < Nlayer2; i++) {
            NameLayer2[i] = (char *)emalloc(sizeof(char) * (strlen(GetLayerName(i + 1)) + 1));
            strcpy(NameLayer2[i], GetLayerName(i + 1));
        }
    }

    // build common layer structure;
    for (i = 0; i < Nlayer1; i++)
        NewLayer(NameLayer1[i]);

#ifdef DEBUG
    printf(" \nThere are %d layers in the result:\n", Nlayer1 + Nlayer2);
    for (i = 1; i <= Nlayer1 + Nlayer2; i++) printf("   %s\n", GetLayerName(i));
#endif DEBUG
}


// ================================
// Making tags different for places
// ================================
void PlaceTags(void) {
    struct Desc *D, *act, *tmp;
    char *newtag, *newname, *end, *num;
    int n, ok, plindex = 0, i1, c;
    struct place_object *pl;

    newtag = (char *)emalloc(sizeof(char) * 100);
    newname = (char *)emalloc(sizeof(char) * 100);
    end = (char *)emalloc(sizeof(char) * 6);
    end += 5;

    D = FillDescPl(Result->places);
    act = D;

    while (act != NULL) {
        ok = 0;
        n = 0;
        while (!ok) {
            strcpy(newtag, act->tag);
            if (n != 0) {
                num = lltostr((long)n, end);
                end[0] = '\0';
                strcat(newtag, num);
            }
            ok = 1;
            tmp = D;
            c = 0;
            while (tmp != NULL && ok) {
                if (c != plindex && strcmp(newtag, tmp->tag) == 0) ok = 0;
                tmp = tmp->next;
                c++;
            }
            n++;
        }

        if (n > 1) {
            free(act->tag);
            act->tag = (char *)emalloc(sizeof(char) * (strlen(newtag) + 1));
            strcpy(act->tag, newtag);
            strcpy(newname, newtag);
            for (i1 = 0; i1 < act->nl; i1++) {
                strcat(newname, "|");
                strcat(newname, act->labels[i1]);
            }
            if (act->end != NULL) {
                strcat(newname, "||");
                strcat(newname, act->end);
            }
            pl = Result->places;
            for (i1 = 0; i1 < plindex; i1++) pl = pl->next;
            free(pl->tag);
            pl->tag = (char *)emalloc(sizeof(char) * (strlen(newname) + 1));
            strcpy(pl->tag, newname);
        }
        act = act->next;
        plindex++;
    }

    while (D != NULL) {
        for (i1 = 0; i1 < D->nl; i1++) free(D->labels[i1]);
        if (D->tag != NULL) free(D->tag);
        if (D->labels != NULL) free(D->labels);
        if (D->end != NULL) free(D->end);
        D = D->next;
    }

    free(newname);
    end -= 5;
    free(end);
    free(newtag);
}


// =====================================
// Making tags different for transitions
// =====================================
void TransitionTags(void) {
    struct Desc *D, *act, *tmp;
    char *newtag, *newname, *end, *num;
    int n, ok, trindex = 0, i1, c;
    struct trans_object *tr;

    newtag = (char *)emalloc(sizeof(char) * 100);
    newname = (char *)emalloc(sizeof(char) * 100);
    end = (char *)emalloc(sizeof(char) * 6);
    end += 5;

    D = FillDescTr(Result->trans, Result->groups);
    act = D;

    while (act != NULL) {
        ok = 0;
        n = 0;
        while (!ok) {
            strcpy(newtag, act->tag);
            if (n != 0) {
                num = lltostr((long)n, end);
                end[0] = '\0';
                strcat(newtag, num);
            }
            ok = 1;
            tmp = D;
            c = 0;
            while (tmp != NULL && ok) {
                if (c != trindex && strcmp(newtag, tmp->tag) == 0) ok = 0;
                tmp = tmp->next;
                c++;
            }
            n++;
        }

        if (n > 1) {
            free(act->tag);
            act->tag = (char *)emalloc(sizeof(char) * (strlen(newtag) + 1));
            strcpy(act->tag, newtag);
            strcpy(newname, newtag);
            for (i1 = 0; i1 < act->nl; i1++) {
                strcat(newname, "|");
                strcat(newname, act->labels[i1]);
            }
            if (act->end != NULL) {
                strcat(newname, "||");
                strcat(newname, act->end);
            }
            tr = GotoTrIndex(trindex, Result);
            free(tr->tag);
            tr->tag = (char *)emalloc(sizeof(char) * (strlen(newname) + 1));
            strcpy(tr->tag, newname);
        }
        act = act->next;
        trindex++;
    }

    while (D != NULL) {
        for (i1 = 0; i1 < D->nl; i1++) free(D->labels[i1]);
        if (D->tag != NULL) free(D->tag);
        if (D->labels != NULL) free(D->labels);
        if (D->end != NULL) free(D->end);
        D = D->next;
    }

    free(newname);
    end -= 5;
    free(end);
    free(newtag);
}


// ====
// MAIN
// ====
int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("\n Usage:\n");
        printf("  algebra net1 net2 operator restfile resultname [placement shiftx shifty]\n\n");
        printf(" Operators: 't': Superposition Over Transitions\n");
        printf("            'p': Superposition Over Places\n");
        printf("            'b': Superposition Over Places & Transitions\n");
        printf(" restfile: contains the labels to be used for synchronization\n");
        printf(" placement: 1 ---> net1 net2\n");
        printf("            2 ---> net1\n");
        printf("                   net2\n");
        printf("            3 ---> net2 is shifted by (shiftx,shifty)\n");
        exit(0);
    }

    mem_alloc();

    ReadOperands(argv[1], argv[2]);
    Restrictions(argv[4]);

    // define shifting of net2
    shifty = MaxYCoor(Op1);
    shiftx = MaxXCoor(Op1);
    if (argc > 6) {
        switch (atoi(argv[6])) {
        case 1:
            shifty = 0;
            break;
        case 2:
            shiftx = 0;
            break;
        case 3:
            shiftx = atof(argv[7]);
            shifty = atof(argv[8]);
        }
    }
    else shifty = 0;

    switch (argv[3][0]) {
    case 't':
        Mode = 1;
        Transitions();
        break;
    case 'p':
        Mode = 2;
        Places();
        break;
    case 'b':
        Mode = 3;
        Transitions_Places();
        break;
    default:  printf("\n Unknown operator, type 'algebra' to see list of operators.\n\n");
        exit(0);
    }

    // printf(" AC: %d\n", ArcCount);

    TransitionTags();
    PlaceTags();

    netobj = Result;
    write_file(argv[5]);

    mem_free();

    return 0;
}







