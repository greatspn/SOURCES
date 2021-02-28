//
//  Utility for Compositional Algebra of SWN Petri-Nets
//

#include<string.h>
#include<stdlib.h>

#include "rescale.h"
#include "global.h"
#include "layer.h"
#include "load.h"
#include "save.h"
#include "alloc.h"

int yyparse();

//#define DEBUG
#define SHIFT 25.0

extern FILE *yyin;
FILE *fp;
char *pfname;

// Structure to describe labeling of transition and places
struct Desc {
    char *wholetag;             // whole tag
    char *tag;                  // tag of transition (first part)
    char **labels;              // labels of transition
    int nl;                     // number of labels
    char *end;                  // string to distinguish
    struct place_object *pl_obj; // place being described
    struct trans_object *tr_obj; // transition being described
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
int **JoinTr;                    // Matrix indicatinandg which transitions to join
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
const char *lexstr = "alma|bela"; // string analysed by parser
char **Translations;             // redefinition info for variables
int TLength = 0;                 // Number of Translations
int Mode;
int ArcCount;

// Variables given by switches
int no_ba = 0;                   // To use broken arcs or not
float rs = 1.0;        // Rescale factor for operands
int g_compose_tags = 0;

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

    c = (char *)emalloc(sizeof(char) * (strlen(c1) + strlen(c2) + 10));
    strcpy(c, c1);
    strcat(c, c2);

#ifdef DEBUG
    printf("    The joined comment is:\n\n%s\n", c);
#endif

    return c;
}


// // 26/02/2021 Added by Elvio
// // Decompose a tag string:   name|tag1|tag2|...|tagN   into a linked list
struct tag_s { char*tag; struct tag_s* next; int shared; };
struct tag_s* DecomposeTag(const char* tag) {
    struct tag_s *lr = NULL, *l_curr = NULL, *l = NULL;
    // printf("DecomposeTag %s  ", tag);
    // fflush(stdout);

    const char *start = tag, *end = tag;
    while (*start != '\0') {
        while (*end != '|' && *end != '\0')
            ++end;
        if (start != end) {
            l = (struct tag_s*)emalloc(sizeof(struct tag_s));
            if (lr == NULL)
                lr = l_curr = l;
            else {
                l_curr->next = l;
                l_curr = l;
            }
            l->next = NULL;
            l->shared = 0;
            l->tag = (char*)emalloc(sizeof(char) * (end - start + 1));
            strncpy(l->tag, start, end-start);
            l->tag[ end - start ] = '\0';
            // printf("%s ", l->tag);
            // fflush(stdout);
        }
        if (*end == '\0')
            break;
        ++end;
        start = end;
    }
    // putchar('\n');
    return lr;
}
int ShareCommonTag(struct tag_s* t1, struct tag_s* t2, struct RestSet* restset) {
    struct tag_s *l1=t1, *l2; 
    while (l1 != NULL) {
        l2 = t2;
        while (l2 != NULL) {
            if (0 == strcmp(l1->tag, l2->tag)) {
                for (int n=0; n<restset->nl; n++)
                    if (0 == strcmp(l1->tag, restset->labels[n]))
                        return 1;
            }

            l2 = l2->next;
        }
        l1 = l1->next;
    }
    return 0;
}
void FreeDecomposedTag(struct tag_s* l) {
    while (l != NULL) {
        struct tag_s* next = l->next;
        free(l->tag);
        free(l);
        l = next;
    }
}
// 26/02/2021 Added by Elvio
// ------------------------------------------------
// Join common tags
// ------------------------------------------------
char* CombineTags(struct tag_s *tag1, struct tag_s *tag2, size_t lenmax) {
    char *combined = (char*)emalloc(sizeof(char) * (lenmax + 1));
    char *c = combined;

    // Take name from tag1
    c = stpcpy(c, tag1->tag);
    c = stpcpy(c, "_");
    c = stpcpy(c, tag2->tag);
    tag1 = tag1->next;
    tag2 = tag2->next;

    struct tag_s *t1, *t2;
    // reset shared flags
    t2 = tag2;
    while (t2) {
        t2->shared = 0;
        t2 = t2->next;
    }
    // determine shared tags
    t1 = tag1;
    while (t1) {
        t2 = tag2;
        while (t2) {
            if (0 == strcmp(t1->tag, t2->tag)) {
                t2->shared = 1;
                break;
            }
            t2 = t2->next;
        }
        t1 = t1->next;
    }
    t1 = tag1;
    while (t1) {
        c = stpcpy(c, "|");
        c = stpcpy(c, t1->tag);
        t1 = t1->next;
    }
    t2 = tag2;
    while (t2) {
        if (!t2->shared) {
            c = stpcpy(c, "|");
            c = stpcpy(c, t2->tag);
        }
        t2 = t2->next;
    }
    return combined;
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

// ========================================
// Check color domains of superposed places
// ========================================
void CheckColor(int p1, int p2) {
    struct place_object *pp1, *pp2;
    int i1, pos1 = 0, pos2 = 0, posvar1, posvar2, more = 1, error;
    char *var1, *var2;

    pp1 = Op1->places;
    for (i1 = 0; i1 < p1; i1++) pp1 = pp1->next;
    pp2 = Op2->places;
    for (i1 = 0; i1 < p2; i1++) pp2 = pp2->next;

    if (pp1->color != NULL || pp2->color != NULL) {
        var1 = (char *)emalloc(sizeof(char) * 100);
        var2 = (char *)emalloc(sizeof(char) * 100);

        if ((pp2->color == NULL && pp1->color != NULL) ||
                (pp1->color == NULL && pp2->color != NULL)) {
            printf("   *** Error ***\n");
            printf("     Colordomains of places %s and %s are different\n",
                   pp1->tag, pp2->tag);
            exit(0);
        }
        while (pp1->color[pos1] == ' ') pos1++;
        while (pp2->color[pos1] == ' ') pos2++;
        while (more) {
            posvar1 = 0;
            posvar2 = 0;
            while (pp1->color[pos1] != ' ' && pp1->color[pos1] != ',' &&  pp1->color[pos1] != '\0') {
                var1[posvar1] = pp1->color[pos1];
                pos1++;
                posvar1++;
            }
            var1[posvar1] = '\0';
            while (pp2->color[pos2] != ' ' && pp2->color[pos2] != ',' &&  pp2->color[pos2] != '\0') {
                var2[posvar2] = pp2->color[pos2];
                pos2++;
                posvar2++;
            }
            var2[posvar2] = '\0';
            error = 0;
            //printf(" Variables: %s, %s\n", var1,var2);
            if (strcmp(var1, var2) != 0) error = 1;
            while (pp1->color[pos1] == ' ' || pp1->color[pos1] == ',') pos1++;
            while (pp2->color[pos2] == ' ' || pp2->color[pos2] == ',') pos2++;
            //printf(" pos1: %d, pos2: %d\n", pos1 ,pos2);
            if (pp1->color[pos1] == '\0' && pp2->color[pos2] == '\0') more = 0;
            if ((pp1->color[pos1] != '\0' && pp2->color[pos2] == '\0') ||
                    (pp2->color[pos2] != '\0' && pp1->color[pos1] == '\0')) error = 1;
            if (error) {
                printf("   *** Error ***\n");
                printf("     Colordomains of places %s and %s are different\n",
                       pp1->tag, pp2->tag);
                exit(0);
            }
        }


        free(var1);
        free(var2);
    }
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
            if (join) CheckColor(i, j);
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
        if (gr != NULL) {
            t = gr->trans;
            gr = gr->next;
        }
    }
    else {
        //printf("\n Exponential Transitions:\n");
    }
    while (t != NULL) {
        //printf("1\n");
        if (first) {
            d = (struct Desc *) emalloc(sizeof(struct Desc));
            tmp = d;
            first = 0;
        }
        else {
            //printf("12\n");
            tmp->next = (struct Desc *)emalloc(sizeof(struct Desc));
            //printf("13\n");
            tmp = tmp->next;
        }
        //printf("2\n");
        tmp->nl = 0;
        tmp->labels = NULL;
        tmp->end = NULL;
        tmp->next = NULL;
        tmp->pl_obj = NULL;
        tmp->tr_obj = t;

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

        //printf("Step for next\n");
        if (t->next == NULL && gr != NULL) {
            t = gr->trans;
            if (t != NULL) {
                //printf("\n Group %s: %d\n", gr->tag, gr->pri);
            }
            gr = gr->next;
            while (t == NULL && gr != NULL) {
                t = gr->trans;
                if (t != NULL) {
                    //printf("\n Group %s: %d\n", gr->tag, gr->pri);
                }
                gr = gr->next;
            }
        }
        else {
            t = t->next;
        }
        //printf("Step for next done\n");

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
        tmp->pl_obj = t;
        tmp->tr_obj = NULL;

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

                        if (!no_ba)
                            if (new->mult > 0) new->mult *= -1;

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

    //printf("Number of transitions synchronized over %d. tr is %d\n", index,n);

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
    //printf(" Player vector is: \n");
    //for(i2=0;i2<d1->nl;i2++) printf(" %d", v[i2]);
    //printf("\n");
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
    //printf(" Players are: \n");
    //for(i1=0;i1<n;i1++) printf(" %d", players[i1]);
    //printf("\n");

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
            c == ' ' || c == '^' || c == ',');
}

// ======================================================
// Collecting number of variables connected to transition
// ======================================================
char **CollectVariables(struct trans_object *tr, int *n, struct net_object *net) {
    struct arc_object *a;
    int count = 0, pos, i1, search, variable, toadd, trpred = 1;
    char *var, **res = NULL, *str;

    //printf(" Collect\n");
    var = (char *) emalloc(sizeof(char) * 100);
    a = net->arcs;
    while (a != NULL || trpred) {
        if ((!trpred && a->trans == tr && a->color != NULL) || (trpred && tr->color != NULL)) {
            if (!trpred) str = a->color;
            if (trpred) str = tr->color;
            pos = 0;
            //printf(" Searching variables in: %s\n", str);
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
                            //printf("  Variable %s is added to list\n",var);
                            res = (char **)realloc(res, (count + 1) * sizeof(char *));
                            res[count] = (char *)emalloc(sizeof(char) * (strlen(var) + 1));
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

    //printf(" Variables:\n");
    //for(i1=0;i1<count;i1++) printf("  %s\n", res[i1]);
    //printf("\n");

    *n = count;
    free(var);
    return res;
}


// ================================
// convert an integer into a string
// ================================
void mylltostr(int i, char *s) {
    int digits, i1, num = i, pos = 0;

    digits = (int)floor(log10((double)i));
    for (i1 = digits; i1 >= 0; i1--, pos++) {
        s[pos] = '0';
        s[pos] += (int)floor((double)num / pow(10, i1));
        num -= (int)pow(10, i1) * (int)floor((double)num / pow(10, i1));
    }
    s[pos] = '\0';
}


// ======================================================
// Redefine variables in order to avoid unwanted matching
// ======================================================
char *RedefineVariables(int nofvar, char **varset, char *oldcolor) {
    char *newcolor, *newvar, *var, *end, *num;
    int pos, pos2, variable, change, tochange, search, i1, i2;

    //printf(" Old color is %s\n", oldcolor);

    newcolor = (char *)emalloc(sizeof(char) * 100);
    newvar = (char *)emalloc(sizeof(char) * 100);
    var = (char *)emalloc(sizeof(char) * 100);
    end = (char *)emalloc(sizeof(char) * 6);

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
                //printf("  Variable is: %s\n", var);
                variable = 0;
                tochange = 0;
                for (i1 = 0; i1 < nofvar; i1++)
                    if (strcmp(var, varset[i1]) == 0) tochange = 1;
                if (strcmp(var, "or") == 0) tochange = 0;
                if (strcmp(var, "and") == 0) tochange = 0;
                if (var[0] == '#') tochange = 0;
                if (tochange) {
                    //printf("  Has to be changed\n");
                    // look among translations;
                    for (i1 = 0; i1 < TLength; i1++)
                        if (strcmp(var, Translations[2 * i1]) == 0) {
                            tochange = 0;
                            strcpy(newvar, Translations[2 * i1 + 1]);
                            //printf("  Found among translations\n");
                        }
                    i2 = 1;
                    while (tochange) {
                        strcpy(newvar, "x");
                        mylltostr(i2, end);
                        strcat(newvar, end);
                        //printf(" New Var: %s\n", newvar);
                        tochange = 0;
                        for (i1 = 0; i1 < nofvar; i1++)
                            if (strcmp(newvar, varset[i1]) == 0) tochange = 1;
                        for (i1 = 0; i1 < TLength; i1++)
                            if (strcmp(newvar, Translations[i1 * 2 + 1]) == 0) tochange = 1;
                        if (!tochange) {
                            //printf("  Translated to %s, nooftranslations is %d\n", newvar, TLength);
                            Translations = (char **)realloc(Translations, (1 + TLength) * 2 * sizeof(char *));
                            Translations[TLength * 2] = (char *)emalloc(sizeof(char) * (strlen(var) + 1));
                            Translations[TLength * 2 + 1] = (char *)emalloc(sizeof(char) * (strlen(newvar) + 1));
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
    //printf(" New color is: %s\n\n", newcolor);

    free(newvar);
    free(var);
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


// =====================================================================
// Redefine variables in order to avoid unwanted matching in protections
// =====================================================================
char *SelfRedefineVariables(int nofvar, char **varset, char *oldcolor) {
    char *newcolor, *newvar, *var, *end, *num;
    int pos, pos2, variable, change, tochange, search, i1, i2;

    //printf(" Protection: Old color is %s\n", oldcolor);

    newcolor = (char *)emalloc(sizeof(char) * 100);
    newvar = (char *)emalloc(sizeof(char) * 100);
    var = (char *)emalloc(sizeof(char) * 100);
    end = (char *)emalloc(sizeof(char) * 6);

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
                //printf("  Variable is: %s\n", var);
                variable = 0;
                tochange = 0;
                for (i1 = 0; i1 < nofvar; i1++)
                    if (varset[i1][0] == '#' && strcmp(var, &(varset[i1][1])) == 0) tochange = 1;
                if (strcmp(var, "or") == 0) tochange = 0;
                if (strcmp(var, "and") == 0) tochange = 0;
                if (var[0] == '#') tochange = 0;
                if (tochange) {
                    //printf("  Has to be changed\n");
                    // look among translations;
                    for (i1 = 0; i1 < TLength; i1++)
                        if (strcmp(var, Translations[2 * i1]) == 0) {
                            tochange = 0;
                            strcpy(newvar, Translations[2 * i1 + 1]);
                            //printf("  Found among translations\n");
                        }
                    i2 = 1;
                    while (tochange) {
                        strcpy(newvar, "x");
                        mylltostr(i2, end);
                        strcat(newvar, end);
                        //printf(" New Var: %s\n", newvar);
                        tochange = 0;
                        for (i1 = 0; i1 < nofvar; i1++)
                            if ((varset[i1][0] == '#' && strcmp(newvar, &(varset[i1][1])) == 0) ||
                                    (varset[i1][0] != '#' && strcmp(newvar, varset[i1]) == 0)) tochange = 1;
                        for (i1 = 0; i1 < TLength; i1++)
                            if (strcmp(newvar, Translations[i1 * 2 + 1]) == 0) tochange = 1;
                        if (!tochange) {
                            //printf("  Translated to %s, nooftranslations is %d\n", newvar, TLength);
                            Translations = (char **)realloc(Translations, (1 + TLength) * 2 * sizeof(char *));
                            Translations[TLength * 2] = (char *)emalloc(sizeof(char) * (strlen(var) + 1));
                            Translations[TLength * 2 + 1] = (char *)emalloc(sizeof(char) * (strlen(newvar) + 1));
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
    //printf(" New color is: %s\n\n", newcolor);

    free(newvar);
    free(var);
    free(end);
    return newcolor;
}


// ============================================
// Clear Up Variable protection inconsistencies
// ============================================
void ClearProtection(struct trans_object *t, struct net_object *n) {
    char **varset, *newcolor, *newpredicate;
    int i1, nofvar;
    struct arc_object *a = n->arcs;

    varset = CollectVariables(t, &nofvar, n);

    if (t->color != NULL) {
        newcolor = SelfRedefineVariables(nofvar, varset, t->color);
        free(t->color);
        t->color = newcolor;
    }
    while (a != NULL) {
        if (a->trans == t && a->color != NULL) {
            newcolor = SelfRedefineVariables(nofvar, varset, a->color);
            free(a->color);
            a->color = newcolor;
        }
        a = a->next;
    }

    if (TLength > 0) {
        printf("\n Warning:\n");
        printf("  The following variable(s) appear(s) protected and unprotected\n");
        printf("  connected to transition %s:\n ", t->tag);
        for (i1 = 0; i1 < TLength; i1++)
            printf("     %s renamed to: %s\n", Translations[2 * i1], Translations[2 * i1 + 1]);
    }

    for (i1 = 0; i1 < TLength * 2; i1++) free(Translations[i1]);
    free(Translations);
    Translations = NULL;
    TLength = 0;

    for (i1 = 0; i1 < nofvar; i1++) free(varset[i1]);
    free(varset);
}


// ================================
// Look for inconsistent protection
// ================================
int BuildTranslations(int nv, char **vs) {
    int i1, i2, i3, problem, r = 0;
    char *end, *newvar, *num;

    newvar = (char *)emalloc(sizeof(char) * 100);
    end = (char *)emalloc(sizeof(char) * 6);

    for (i1 = 0; i1 < TLength * 2; i1++) free(Translations[i1]);
    free(Translations);
    Translations = NULL;
    TLength = 0;

    for (i1 = 0; i1 < nv; i1++)
        if (vs[i1][0] != '#') {
            //printf(" %s\n", vs[i1]);
            problem = 0;
            for (i2 = 0; i2 < nv; i2++)
                if (i2 != i1 && vs[i2][0] == '#' && strcmp(vs[i1], &(vs[i2][1])) == 0)
                    problem = 1;
            i3 = 1;
            while (problem) {
                //printf(" Problem with %s\n", vs[i1]);
                strcpy(newvar, "x");
                mylltostr(i3, end);
                strcat(newvar, end);
                //printf(" New Var: %s\n", newvar);
                problem = 0;
                for (i2 = 0; i2 < nv; i2++)
                    if ((vs[i2][0] == '#' && strcmp(newvar, &(vs[i2][1])) == 0) ||
                            (vs[i2][0] != '#' && strcmp(newvar, vs[i2]) == 0)) problem = 1;
                for (i2 = 0; i2 < TLength; i2++)
                    if (strcmp(newvar, Translations[i2 * 2 + 1]) == 0) problem = 1;
                if (!problem) {
                    printf("   *** Warning ***\n");
                    printf("     Variable %s appears protected and unprotected\n", vs[i1]);
                    printf("      ---> Renamed to %s\n", newvar);
                    Translations = (char **)realloc(Translations, (1 + TLength) * 2 * sizeof(char *));
                    Translations[TLength * 2] = (char *)emalloc(sizeof(char) * (strlen(vs[i1]) + 1));
                    Translations[TLength * 2 + 1] = (char *)emalloc(sizeof(char) * (strlen(newvar) + 1));
                    strcpy(Translations[TLength * 2], vs[i1]);
                    strcpy(Translations[TLength * 2 + 1], newvar);
                    TLength++;
                    r = 1;
                }
                i3++;
            }
        }
    free(newvar);
    free(end);
    return r;
}


// ======================================================
// Change variables according to translations
// ======================================================
char *ChangeTranslations(char *oldcolor) {
    char *newcolor, *var, *newvar;
    int pos, pos2, variable, change, tochange, search, i1, i2;

    //printf(" Old color is %s\n", oldcolor);

    newcolor = (char *)emalloc(sizeof(char) * 100);
    var = (char *)emalloc(sizeof(char) * 100);
    newvar = (char *)emalloc(sizeof(char) * 100);

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
                //printf("  Variable is: %s\n", var);
                variable = 0;
                tochange = 0;
                for (i1 = 0; i1 < TLength; i1++)
                    if (strcmp(var, Translations[2 * i1]) == 0) tochange = 1;
                if (strcmp(var, "or") == 0) tochange = 0;
                if (strcmp(var, "and") == 0) tochange = 0;
                if (var[0] == '#') tochange = 0;
                if (tochange) {
                    //printf("  Has to be changed\n");
                    // look among translations;
                    for (i1 = 0; i1 < TLength; i1++)
                        if (strcmp(var, Translations[2 * i1]) == 0) {
                            tochange = 0;
                            strcpy(newvar, Translations[2 * i1 + 1]);
                            //printf("  Found among translations\n");
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
    //printf(" New color is: %s\n\n", newcolor);

    free(newvar);
    free(var);
    return newcolor;
}


// ==================
// Apply translations
// ==================
void ApplyTranslation(struct trans_object *t, struct net_object *n) {
    char *newcolor, *newpredicate;
    int i1, nofvar;
    struct arc_object *a = n->arcs;

    if (t->color != NULL) {
        newcolor = ChangeTranslations(t->color);
        free(t->color);
        t->color = newcolor;
    }
    while (a != NULL) {
        if (a->trans == t && a->color != NULL) {
            newcolor = ChangeTranslations(a->color);
            free(a->color);
            a->color = newcolor;
        }
        a = a->next;
    }
}

// ================================================================
// Adding arcs of argument 2 that are connected to sync transitions
// ================================================================
void ArcsofArg2(struct trans_object *tr2, int op1index, int specimen, struct trans_object *tr1) {
    struct arc_object *arcs, *new, *a3;
    struct place_object *place;
    struct trans_object *trans;
    int p, i1, i2, np, mul, simplepl, plindex1, PlIndex, i3, j, mulmul, l, doublearc;
    struct coordinate *ccp1, *ccp2;
    struct position pospl;
    double length;
    int *players, nofplayers, i4, i5, i6, nofvar = 0, nv, found;
    char **varset, *newcolor, *newpredicate, *num, *end;
    char **vs, *warn;

    newpredicate = (char *)emalloc(sizeof(char) * 1000);
    warn = (char *)emalloc(sizeof(char) * 1000);
    end = (char *)emalloc(sizeof(char) * 10);
    nofplayers = CountPlayers(op1index);
    players = (int *)emalloc(sizeof(int) * nofplayers);
    LookforPlayers(op1index, specimen, players);

    // here all variables are collected;
    strcpy(warn, "     when transition ");
    strcat(warn, tr1->tag);
    strcat(warn, " of 1st operand is superposed with\n     ");
    varset = CollectVariables(tr1, &nofvar, Op1);
    for (i4 = 0; i4 < nofplayers; i4++) {
        trans = GotoTrIndex(players[i4], Op2);
        vs = CollectVariables(trans, &nv, Op2);
        for (i5 = 0; i5 < nv; i5++) {
            found = 0;
            for (i6 = 0; i6 < nofvar; i6++)
                if (strcmp(vs[i5], varset[i6]) == 0) found = 1;
            if (!found) {
                varset = (char **)realloc(varset, (nofvar + 1) * sizeof(char *));
                varset[nofvar] = (char *)emalloc(sizeof(char) * (strlen(vs[i5]) + 1));
                strcpy(varset[nofvar], vs[i5]);
                nofvar++;
            }
        }
        strcat(warn, trans->tag);
        if (i4 != nofplayers - 1) strcat(warn, ", ");
    }
    strcat(warn, " of 2nd operand\n\n");

    //for(i5=0;i5<nofvar;i5++) printf(" %s,", varset[i5]);
    //printf("\n");
    if ((BuildTranslations(nofvar, varset)) == 1) {
        printf("%s", warn);
    }
    ApplyTranslation(tr1, Op1);
    for (i4 = 0; i4 < nofplayers; i4++) {
        trans = GotoTrIndex(players[i4], Op2);
        ApplyTranslation(trans, Op2);
    }

    for (i5 = 0; i5 < nofvar; i5++) free(varset[i5]);
    free(varset);
    nofvar = 0;

    for (i4 = 0; i4 < nofplayers; i4++) {
        varset = CollectVariables(tr2, &nofvar, Result);
        trans = GotoTrIndex(players[i4], Op2);

        for (i1 = 0; i1 < TLength * 2; i1++) free(Translations[i1]);
        free(Translations);
        Translations = NULL;
        TLength = 0;

        if (trans->color != NULL) {
            newcolor = RedefineVariables(nofvar, varset, trans->color);
            if (tr2->color != NULL) {
                strcpy(newpredicate, tr2->color);
                strcat(newpredicate, "and");
                strcat(newpredicate, newcolor);
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
                        //printf(" p in op2: %d, p in op1: %d, mulmul: %d\n", p,plindex1,mulmul);
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
                                            mylltostr((int)abs(arcs->mult), end);
                                            strcat(newpredicate, end);
                                        }
                                        if (arcs->color != NULL && a3->color == NULL) 			 	{
                                            strcpy(newpredicate, arcs->color);
                                            strcat(newpredicate, "+");
                                            mylltostr((int)abs(a3->mult), end);
                                            strcat(newpredicate, end);
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

                            if (!no_ba)
                                if (new->mult > 0) new->mult *= -1;
                        }
                    }
                }
            }
            arcs = arcs->next;
        }

        for (i5 = 0; i5 < nofvar; i5++) free(varset[i5]);
        free(varset);
        nofvar = 0;
    }

    for (i1 = 0; i1 < TLength * 2; i1++) free(Translations[i1]);
    free(Translations);
    Translations = NULL;
    TLength = 0;
    free(players);
    free(newpredicate);
    free(warn);
    free(end);
}

// -----------------------------------
// Join Exponential Transitions Objects
// -----------------------------------
struct trans_object *JoinExpTrans(struct trans_object *trans1, struct trans_object *trans2) {
    struct trans_object *tr1, *tr2, *trr, *r;
    int first = 1, i, j, k, l, mul, toadd, simple, index2;

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
            if (mul > 0) { // have @mul transitions of net2 to combine with
                simple = 0;
                mul = TrNeed(GlCountTr1);
            }
            else {
                mul++; // no transitions to combine in net2
            }

            MapTr1[GlCountTr1] = NtrR;
            index2 = 0;
            tr2 = trans2;
            for (j = 0; j < mul; j++) {
                if (first) {
                    r = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    trr = r;
                    first = 0;
                }
                else {
                    trr->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    trr = trr->next;
                }
                trr->next = NULL;
                NtrR++;
                CopyTrProperties(tr1, trr, j, 1);
                trr->brokenin = 0;
                trr->brokenout = 0;
                SimpleArcs(tr1, trr, 1);
                if (!simple) 
                    ArcsofArg2(trr, GlCountTr1, j, tr1);
                if (!simple) {
                    if (g_compose_tags) {
                        while (!JoinTr[k][index2]) { // get the index of the other transition in net2
                            ++index2;
                            tr2 = tr2->next;
                        }
                        // printf("CopyTrProperties[1] tr1->tag=%s  tr2->tag=%s  trr->tag=%s j=%d\n", tr1->tag, tr2->tag, trr->tag, j);
                        struct tag_s *tp1 = DecomposeTag(tr1->tag);
                        struct tag_s *tp2 = DecomposeTag(tr2->tag);
                        char *newtag = CombineTags(tp1, tp2, strlen(tr1->tag) + strlen(tr2->tag));
                        // printf("replacing %s with %s\n", trr->tag, newtag);
                        free(trr->tag);
                        trr->tag = newtag;
                        FreeDecomposedTag(tp1);
                        FreeDecomposedTag(tp2);
                        // advance to next transition
                        ++index2;
                        tr2 = tr2->next;
                    }
                }
                // else {
                //     printf("CopyTrProperties[1] tr1->tag=%s  trr->tag=%s j=%d  simple\n", tr1->tag, trr->tag, j);
                // }
            }
            k++;
            tr1 = tr1->next;
            GlCountTr1++;
        }
        tr2 = trans2;
        k = 0;
        while (tr2 != NULL) {
            toadd = 1;
            for (j = 0; j < Ntr1; j++) if (JoinTr[j][k] == 1) toadd = 0;
            if (toadd) {
                MapTr2[GlCountTr2] = NtrR;
                if (first) {
                    r = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    trr = r;
                    first = 0;
                    NtrR++;
                }
                else {
                    trr->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                    trr = trr->next;
                    NtrR++;
                }
                trr->next = NULL;
                // printf("CopyTrProperties[2] tr2->tag=%s\n", tr2->tag);
                CopyTrProperties(tr2, trr, 0, 0);
                trr->brokenin = 0;
                trr->brokenout = 0;
                SimpleArcs(tr2, trr, 0);
            }
            k++;
            tr2 = tr2->next;
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
void CopyPlProperties(struct place_object *pl1, struct place_object *plOut, int j, int in_op1) {
    struct lisp_object *li;
    struct mpar_object *mp;
    int i;

    plOut->tag = (char *)emalloc(sizeof(char) * (strlen(pl1->tag) + 1));
    strcpy(plOut->tag, pl1->tag);

    if (pl1->color != NULL) {
        plOut->color = (char *)emalloc(sizeof(char) * (strlen(pl1->color) + 1));
        strcpy(plOut->color, pl1->color);
    }
    else {
        plOut->color = NULL;
    }
    plOut->center.x = pl1->center.x + j * 1.2 * SHIFT + shiftx * (1 - in_op1);
    plOut->center.y = pl1->center.y + j * SHIFT + shifty * (1 - in_op1);
    plOut->tagpos.x = pl1->tagpos.x;
    plOut->tagpos.y = pl1->tagpos.y;
    plOut->colpos.x = pl1->colpos.x;
    plOut->colpos.y = pl1->colpos.y;
    plOut->tokens = pl1->tokens;
    plOut->m0 = pl1->m0;
    plOut->cmark = NULL;
    if (pl1->cmark != NULL) {
        li = Result->lisps;
        while (strcmp(li->tag, pl1->cmark->tag) != 0) li = li->next;
        plOut->cmark = li;
    }
    plOut->lisp = NULL;
    if (pl1->lisp != NULL) {
        li = Result->lisps;
        while (strcmp(li->tag, pl1->lisp->tag) != 0) li = li->next;
        plOut->lisp = li;
    }
    plOut->mpar = NULL;
    if (pl1->mpar != NULL) {
        mp = Result->mpars;
        while (strcmp(mp->tag, pl1->mpar->tag) != 0) mp = mp->next;
        plOut->mpar = mp;
    }
    plOut->next = NULL;

    plOut->layer = NewLayerList(WHOLENET, NULL);
    for (i = 1; i <= Nlayer2; i++)
        if (TestLayer(i, pl1->layer))
            SetLayer2LayerList(i + Nlayer2 * in_op1, plOut->layer);
    plOut->next = NULL;
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
            //if(newvar) printf(" Var2 is %s\n",var2);
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
                    //if(newvar) printf(" Var1 is %s\n",var1);
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
    struct place_object *pl1, *pl2, *plr, *r, *tmp, *tmp2;
    int first = 1, i, j, k, l, mul, toadd, simple, GlCountPl1 = 0, GlCountPl2 = 0, index2;
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
            if (mul > 0) { // have @mul places of net2 to combine with
                simple = 0;
                mul = PlNeed(GlCountPl1);
            }
            else {
                mul++; // no place to combine in net2
            }

            MapPl1[GlCountPl1] = NplR;
            index2 = 0;
            pl2 = place2;
            for (j = 0; j < mul; j++) {
                if (first) {
                    r = (struct place_object *)emalloc(sizeof(struct place_object));
                    plr = r;
                    first = 0;
                    NplR++;
                }
                else {
                    plr->next = (struct place_object *)emalloc(sizeof(struct place_object));
                    plr = plr->next;
                    NplR++;
                }
                CopyPlProperties(pl1, plr, j, 1);
                plr->brokenin = 0;
                plr->brokenout = 0;
                if (!simple) {
                    if (g_compose_tags) {
                        while (!JoinPl[k][index2]) { // get the index of the other place in net2
                            ++index2;
                            pl2 = pl2->next;
                        }
                        // printf("CopyPlProperties[1] pl1->tag=%s  pl2->tag=%s  plr->tag=%s j=%d\n", 
                        //        pl1->tag, pl2->tag, plr->tag, j);
                        struct tag_s *tp1 = DecomposeTag(pl1->tag);
                        struct tag_s *tp2 = DecomposeTag(pl2->tag);
                        char *newtag = CombineTags(tp1, tp2, strlen(pl1->tag) + strlen(pl2->tag));
                        // printf("replacing %s with %s\n", plr->tag, newtag);
                        free(plr->tag);
                        plr->tag = newtag;
                        FreeDecomposedTag(tp1);
                        FreeDecomposedTag(tp2);

                        // advance to next place in p2.
                        ++index2;
                        pl2 = pl2->next;
                    }
                }
                // else {
                //     printf("CopyPlProperties[1] pl1->tag=%s  plr->tag=%s j=%d  simple\n", pl1->tag, plr->tag, j);
                // }
            }
            k++;
            pl1 = pl1->next;
            GlCountPl1++;
        }
        pl2 = place2;
        k = 0;
        while (pl2 != NULL) {
            toadd = 1;
            for (j = 0; j < Npl1; j++) if (JoinPl[j][k] == 1) toadd = 0;
            if (toadd) {
                MapPl2[GlCountPl2] = NplR;
                if (first) {
                    r = (struct place_object *)emalloc(sizeof(struct place_object));
                    plr = r;
                    first = 0;
                    NplR++;
                }
                else {
                    plr->next = (struct place_object *)emalloc(sizeof(struct place_object));
                    plr = plr->next;
                    NplR++;
                }
                // printf("CopyPlProperties[2] pl2->tag=%s\n", pl2->tag);
                CopyPlProperties(pl2, plr, 0, 0);
            }
            k++;
            pl2 = pl2->next;
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

// forward declaration
struct group_object *JoinImmTrans2(struct group_object *group1, struct group_object *group2);

// -----------------------------------
// Join Immediate Transitions Objects
// -----------------------------------
struct group_object *JoinImmTrans(struct group_object *group1, struct group_object *group2) {
    struct trans_object *tr1, *trr, *tr2;
    struct group_object *gr1, *gr2, *gr3, *grr, *r;
    struct lisp_object *li;
    struct rpar_object *mp;
    struct Desc *desc2;
    int first = 1, i, j, l, *k, mul, toadd, c1, c2, newgroup, firstgroup = 1, simple, index2, *trp;

    // if (g_compose_tags)
    //     return JoinImmTrans2(group1, group2);

    r = NULL;
    c1 = GlCountTr1;
    c2 = GlCountTr2;

    //printf(" Imm Start\n");

    if (group1 != NULL || group2 != NULL) {
        trp = (int*)emalloc(sizeof(int) * MAX(Ntr1, Ntr2));
        // look for the first non-empty group;
        gr1 = group1;
        gr2 = group2;

        while (gr1 != NULL && gr1->trans == NULL) gr1 = gr1->next;
        while (gr2 != NULL && gr2->trans == NULL) gr2 = gr2->next;

        while (gr1 != NULL || gr2 != NULL) {
            if (gr1 == NULL) {
                gr3 = gr2;
                k = &c2;
                // printf("[A] gr1==NULL, gr2->pri=%d, k=%d\n", gr2->pri, *k);
            }
            if (gr2 == NULL) {
                gr3 = gr1;
                k = &c1;
                // printf("[B] gr1->pri=%d, gr2==NULL, k=%d\n", gr1->pri, *k);
            }
            if (gr1 != NULL && gr2 != NULL) {
                if (gr1->pri > gr2->pri) {
                    gr3 = gr2;
                    k = &c2;
                    // printf("[C1] gr1->pri=%d > gr2->pri=%d, k=%d\n", gr1->pri, gr2->pri, *k);                    
                }
                else {
                    gr3 = gr1;
                    k = &c1;
                    // printf("[C2] gr1->pri=%d <= gr2->pri=%d, k=%d\n", gr1->pri, gr2->pri, *k);                    
                }
            }
            if (firstgroup) {
                //printf("\n\n ****** First group allocated ******\n\n");
                r = (struct group_object *)emalloc(sizeof(struct group_object));
                grr = r;
                grr->trans = NULL;
                //grr->trans=(struct trans_object *)emalloc(sizeof(struct trans_object));
                //tr2=grr->trans;
                firstgroup = 0;
                first = 1;
            }
            else {
                //printf("\n\n ***** grr->pri: %d, gr3->pri: %d ********\n\n", grr->pri, gr3->pri);
                if (grr->pri < gr3->pri) {
                    //printf(" A New group allocated\n");
                    grr->next = (struct group_object *)emalloc(sizeof(struct group_object));
                    grr = grr->next;
                    grr->trans = NULL;
                    //grr->trans=(struct trans_object *)emalloc(sizeof(struct trans_object));
                    //tr2=grr->trans;
                    first = 1;
                }
            }
            grr->pri = gr3->pri;
            grr->tag = (char *)emalloc((strlen(gr3->tag) + 1) * sizeof(char));
            strcpy(grr->tag, gr3->tag);
            grr->next = NULL;

            tr1 = gr3->trans;
            while (tr1 != NULL) {
                mul = 0;
                simple = 1;
                index2 = 0;
                if (gr3 == gr1) {
                    for (j = 0; j < Ntr2; j++) if (JoinTr[*k][j] == 1) { mul++; trp[index2++] = j; }
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
                    for (j = 0; j < Ntr1; j++) if (JoinTr[j][*k] == 1) { mul++; trp[index2++] = j; }
                    mul = (mul == 0);
                    MapTr2[GlCountTr2] = NtrR;
                }
                NtrR += mul;
                // index2 = (gr3 == gr1) ? GlCountTr2 : GlCountTr1;
                // tr2 = (gr3 == gr1) ? (gr2==NULL?NULL:gr2->trans) : (gr1==NULL?NULL:gr1->trans);
                for (j = 0; j < mul; j++) {
                    if (!first) {
                        trr->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                        trr = trr->next;
                    }
                    else {
                        grr->trans = (struct trans_object *)emalloc(sizeof(struct trans_object));
                        trr = grr->trans;
                        first = 0;
                    }
                    CopyTrProperties(tr1, trr, j, gr3 != gr2);
                    trr->brokenin = 0;
                    trr->brokenout = 0;
                    if (gr3 == gr1) {
                        SimpleArcs(tr1, trr, 1);
                        if (!simple) ArcsofArg2(trr, GlCountTr1, j, tr1);
                    }
                    else {
                        if (simple) SimpleArcs(tr1, trr, 0);
                    }
                    if (!simple) {
                        if (g_compose_tags) {
                            if (gr3 == gr1) desc2 = Op2TrD;
                            if (gr3 == gr2) desc2 = Op1TrD;
                            // printf("j=%d trp[j]=%d, Ntr1=%d Ntr2=%d op1=%d\n", j, trp[j], Ntr1, Ntr2, gr3 == gr1);
                            for (int ii=0; ii<trp[j]; ii++) 
                                desc2 = desc2->next;
                            tr2 = desc2->tr_obj;
                        }
                        // printf("COMBINE tr1->tag=%s  tr2->tag=%s  trr->tag=%s\n", 
                        //         tr1->tag, tr2->tag, trr->tag);
                        struct tag_s *tp1 = DecomposeTag(tr1->tag);
                        struct tag_s *tp2 = DecomposeTag(tr2->tag);
                        char *newtag = CombineTags(tp1, tp2, strlen(tr1->tag) + strlen(tr2->tag));
                        // printf("replacing %s with %s\n", trr->tag, newtag);
                        free(trr->tag);
                        trr->tag = newtag;
                        FreeDecomposedTag(tp1);
                        FreeDecomposedTag(tp2);
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
    free(trp);

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

// -----------------------------------
// Join Immediate Transitions Objects
// -----------------------------------
struct group_object *JoinImmTrans2(struct group_object *group1, struct group_object *group2) {
    struct trans_object *tr1, *trr, *tr2, *trA, *trB;
    struct group_object *gr1, *gr2, *grA, *grB, *grr, *r;
    struct lisp_object *li;
    struct rpar_object *mp;
    int i, j, l, *kA, *kB, mul, toadd, c1, c2, newgroup, firstgroup = 1, simple, index2;

    r = NULL;
    c1 = GlCountTr1;
    c2 = GlCountTr2;

    //printf(" Imm Start\n");

    if (group1 != NULL || group2 != NULL) {
        // look for the first non-empty group;
        gr1 = group1;
        gr2 = group2;

        while (gr1 != NULL && gr1->trans == NULL) gr1 = gr1->next;
        while (gr2 != NULL && gr2->trans == NULL) gr2 = gr2->next;

        while (gr1 != NULL || gr2 != NULL) {
            int adv_gr1 = 0, adv_gr2 = 0;
            grA = grB = kA = kB = NULL;
            if (gr1 == NULL) {
                grA = gr2;
                kA = &c2;
                adv_gr2 = 1;
                //printf("[A] gr1==NULL, gr2->pri=%d, kA=%d\n", gr2->pri, *kA);
            }
            if (gr2 == NULL) {
                grA = gr1;
                kA = &c1;
                adv_gr1 = 1;
                //printf("[B] gr1->pri=%d, gr2==NULL, kA=%d\n", gr1->pri, *kA);
            }
            if (gr1 != NULL && gr2 != NULL) {
                if (gr1->pri > gr2->pri) {
                    grA = gr2;
                    kA = &c2;
                    adv_gr2 = 1;
                    //printf("[C1] gr1->pri=%d > gr2->pri=%d, kA=%d\n", gr1->pri, gr2->pri, *kA);                    
                }
                else if (gr1->pri < gr2->pri) {
                    grA = gr1;
                    kA = &c1;
                    adv_gr1 = 1;
                    //printf("[C2] gr1->pri=%d < gr2->pri=%d, kA=%d\n", gr1->pri, gr2->pri, *kA);                    
                }
                else {
                    grA = gr1;
                    kA = &c1;
                    grB = gr2;
                    kB = &c2;
                    adv_gr1 = adv_gr2 = 1;
                    //printf("[C2] gr1->pri=%d == gr2->pri=%d, kA=%d, kB=%d\n", gr1->pri, gr2->pri, *kA, *kB);                    
                }
            }


            if (firstgroup) {
                r = (struct group_object *)emalloc(sizeof(struct group_object));
                grr = r;
                grr->trans = NULL;
                firstgroup = 0;
                // first = 1;
            }
            else {
                grr->next = (struct group_object *)emalloc(sizeof(struct group_object));
                grr = grr->next;
                grr->trans = NULL;
            }
            grr->pri = grA->pri;
            grr->tag = (char *)emalloc((strlen(grA->tag) + 1) * sizeof(char));
            strcpy(grr->tag, grA->tag);
            grr->next = NULL;

            if (grB == NULL) { // No cross-product, just copy transitions as-is
                //printf("  no cross product, copy.\n");
                trA = grA->trans;
                while (trA != NULL) {
                    if (grr->trans != NULL) {
                        trr->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                        trr = trr->next;
                    }
                    else {
                        grr->trans = (struct trans_object *)emalloc(sizeof(struct trans_object));
                        trr = grr->trans;
                        // first = 0;
                    }
                    CopyTrProperties(trA, trr, 0, grA == gr1);
                    trr->brokenin = 0;
                    trr->brokenout = 0;
                    SimpleArcs(trA, trr, grA == gr1);
                    //printf("  SIMPLE COPY %s\n", trr->tag);
                    (*kA)++;
                    trA = trA->next;
                    if (grA == gr1) GlCountTr1++;
                    if (grA == gr2) GlCountTr2++;
                }
            }
            else {
                //printf("  cross product, copy.\n");
                int nA = 0, nB = 0, iA, iB;
                for (trA = grA->trans; trA != NULL; trA = trA->next) nA++;
                for (trB = grB->trans; trB != NULL; trB = trB->next) nB++;
                int *selA = (int*)emalloc(sizeof(int) * nA);
                memset(selA, 0, sizeof(int) * nA);
                int *selB = (int*)emalloc(sizeof(int) * nB);
                memset(selB, 0, sizeof(int) * nB);
                struct tag_s **tpA = (struct tag_s*)emalloc(sizeof(struct tag_s*) * nA);
                for (iA = 0, trA = grA->trans; trA != NULL; trA = trA->next) 
                    tpA[iA++] = DecomposeTag(trA->tag);
                struct tag_s **tpB = (struct tag_s*)emalloc(sizeof(struct tag_s*) * nB);
                for (iB = 0, trB = grB->trans; trB != NULL; trB = trB->next) 
                    tpB[iB++] = DecomposeTag(trB->tag);


                trA = grA->trans;
                iA = 0;
                while (trA != NULL) {
                    mul = 0;
                    simple = 1;
                    trB = grB->trans;
                    iB = 0;
                    j = 0;
                    while (trB != NULL) {
                        if (ShareCommonTag(tpA[iA], tpB[iB], &TransRest)) {
                            selA[iA] = selB[iB] = 1;

                            if (grr->trans != NULL) {
                                trr->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                                trr = trr->next;
                            }
                            else {
                                grr->trans = (struct trans_object *)emalloc(sizeof(struct trans_object));
                                trr = grr->trans;
                            }
                            CopyTrProperties(trA, trr, j++, 1);
                            trr->brokenin = 0;
                            trr->brokenout = 0;
                            SimpleArcs(trA, trr, 1);
                            //ArcsofArg2(trr, GlCountTr1, j, trA);
                            if (g_compose_tags) {
                                char *newtag = CombineTags(tpA[iA], tpB[iB], strlen(trA->tag) + strlen(trB->tag));
                                //printf("  COMBINE trA->tag=%s  trB->tag=%s  trr->tag=%s  newtag=%s\n", 
                                        // trA->tag, trB->tag, trr->tag, newtag);
                                free(trr->tag);
                                trr->tag = newtag;
                            }
                            (*kA)++;
                            GlCountTr1++;
                            (*kB)++;
                            GlCountTr2++;
                        }
                        trB = trB->next;
                        iB++;
                    }
                    trA = trA->next;
                    iA++;
                }

                for (iA = 0, trA = grA->trans; trA != NULL; trA = trA->next) {
                    if (!selA[iA]) {
                        if (grr->trans != NULL) {
                            trr->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                            trr = trr->next;
                        }
                        else {
                            grr->trans = (struct trans_object *)emalloc(sizeof(struct trans_object));
                            trr = grr->trans;
                        }
                        CopyTrProperties(trA, trr, 0, 1);
                        trr->brokenin = 0;
                        trr->brokenout = 0;
                        SimpleArcs(trA, trr, 1);
                        //printf("  SIMPLE COPY %s\n", trr->tag);
                        (*kA)++;
                        GlCountTr1++;
                    }
                    iA++;
                }
                for (iB = 0, trB = grB->trans; trB != NULL; trB = trB->next) {
                    if (!selB[iB]) {
                        if (grr->trans != NULL) {
                            trr->next = (struct trans_object *)emalloc(sizeof(struct trans_object));
                            trr = trr->next;
                        }
                        else {
                            grr->trans = (struct trans_object *)emalloc(sizeof(struct trans_object));
                            trr = grr->trans;
                        }
                        CopyTrProperties(trB, trr, 0, 0);
                        trr->brokenin = 0;
                        trr->brokenout = 0;
                        SimpleArcs(trB, trr, 0);
                        //printf("  SIMPLE COPY %s\n", trr->tag);
                        (*kB)++;
                        GlCountTr2++;
                    }
                    iB++;
                }

                free(selA);
                for (iA=0; iA<nA; iA++) FreeDecomposedTag(tpA[iA]);
                free(tpA);
                free(selB);
                for (iB=0; iB<nB; iB++) FreeDecomposedTag(tpB[iB]);
                free(tpB);
            }

            // step to an non-empty group
            if (adv_gr1) {
                gr1 = gr1->next;
                while (gr1 != NULL && gr1->trans == NULL)
                    gr1 = gr1->next;
            }
            if (adv_gr2) {
                gr2 = gr2->next;
                while (gr2 != NULL && gr2->trans == NULL)
                    gr2 = gr2->next;
            }
            // if (gr3 == gr1) GlCountTr1++;
            // if (gr3 == gr2) GlCountTr2++;
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
        //ClearProtection(tr1,n);
        tr1 = tr1->next;
    }
    groups = n->groups;
    while (groups != NULL) {
        tr1 = groups->trans;
        while (tr1 != NULL) {
            tr++;
            //ClearProtection(tr1,n);
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


    JoinTr = (int **)emalloc(Ntr1 * sizeof(int *));
    for (i = 0; i < Ntr1; i++) {
        JoinTr[i] = (int *)emalloc(sizeof(int) * Ntr2);
        for (j = 0; j < Ntr2; j++) JoinTr[i][j] = 0;
    }
    JoinPl = (int **)emalloc(Npl1 * sizeof(int *));
    for (i = 0; i < Npl1; i++) {
        JoinPl[i] = (int *)emalloc(sizeof(int) * Npl2);
        for (j = 0; j < Npl2; j++) {
            JoinPl[i][j] = 0;
        }
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

    printf(" Superposition over Transitions Started\n\n");

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

    printf("\n Superposition over Transitions Finished\n");
}


// ------------------------------------------------
// Join Places of Op1 and Op2, write result to R
// ------------------------------------------------
void Places(void) {
    int i, j;

    printf(" Superposition over Places Started\n\n");

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

    printf("\n Superposition over Places Finished\n");
}

// ---------------------------------------------------------
// Join Places & Transitions of Op1 and Op2, write result to R
// ---------------------------------------------------------
void Transitions_Places() {
    int i, j;

    printf(" Superposition over Places & Transitions Started\n\n");

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

    printf("\n Superposition over Transitions & Places Finished\n");
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
        printf("\n Unable to open file: %s\n", fn);
        printf("  ---> Synchronization over all matching labels\n\n");
        return;
    }

    ss = (char *)emalloc(2000);
    while (!feof(f)) {
        if (fscanf(f, "%s\n", ss) != 0) {
            // printf(" Parsed '%s'\n", ss);

            fp = fopen(pfname, "w");
            fprintf(fp, "%s", ss);
            fclose(fp);
            fp = fopen(pfname, "r");
            yyin = fp;

            yyparse();
            fclose(fp);

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
#endif // DEBUG
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

    Pl_Rest = 0;

    D = FillDescPl(Result->places);
    act = D;

    while (act != NULL) {
        ok = 0;
        n = 0;
        while (!ok) {
            strcpy(newtag, act->tag);
            if (n != 0) {
                mylltostr(n, end);
                strcat(newtag, end);
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

    //printf("Hihi1\n");
    newtag = (char *)emalloc(sizeof(char) * 100);
    newname = (char *)emalloc(sizeof(char) * 100);
    end = (char *)emalloc(sizeof(char) * 6);
    end += 5;
    //printf("Hihi2\n");

    // printf( "Making Tags Different for Transitions\n);
    Tr_Rest = 0;
    //printf("Hihi3\n");

    D = FillDescTr(Result->trans, Result->groups);
    //printf("Hihi4\n");
    act = D;

    while (act != NULL) {
        ok = 0;
        n = 0;
        while (!ok) {
            strcpy(newtag, act->tag);
            if (n != 0) {
                mylltostr(n, end);
                strcat(newtag, end);
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
    int ns = 0, i1 = 1, found = 1, more = 1;

    if (i1 != argc)
        while (argv[i1][0] == '-' && found && more) {
            found = 0;
            if (strcmp(argv[i1], "-no_ba") == 0) {
                ns++;
                found = 1;
                no_ba = 1;
                printf("\n -no_ba: no broken arcs will be used between subnets\n");
            }
            if (strcmp(argv[i1], "-g") == 0) {
                ns++;
                found = 1;
                g_compose_tags = 1;
                printf("\n -g: tags will be composed\n");
            }
            if (strcmp(argv[i1], "-rs") == 0) {
                ns += 2;
                found = 1;
                if (i1 + 1 == argc) more = 0;
                if (more) {
                    i1++;
                    if (sscanf(argv[i1], "%f", &rs) != 1) found = 0;
                    if (found)
                        printf("\n -rs %f: rescale factor for result is %f\n", rs, rs);
                }
            }
            if (i1 + 1 == argc) more = 0;
            if (found && more)
                i1++;
        }

    if (argc - ns < 6 || !found) {
        printf("\n Tool to Compose SWN Nets\n");
        printf("\n Usage:\n");
        printf("  algebra [switches] net1 net2 operator restfile resultname [placement shiftx shifty]\n\n");
        printf(" Switches: -no_ba: no broken arcs will be used between subnets\n");
        printf("           -rs number: result will be rescaled by number\n");
        printf("           -g: tags will be composed (EXPERIMENTAL)\n"); // Added by Elvio 27/02/2021
        printf(" Operators: 't': Superposition Over Transitions\n");
        printf("            'p': Superposition Over Places\n");
        printf("            'b': Superposition Over Places & Transitions\n");
        printf(" restfile: contains the labels to be used for synchronization.\n");
        printf("           Sample syntax for this file:\n");
        printf("                 transition={tl1|tl2}\n");
        printf("                 place={pl1|pl2|pl3}\n");
        printf("\n");
        printf(" placement: 1 ---> net1 net2\n");
        printf("            2 ---> net1\n");
        printf("                   net2\n");
        printf("            3 ---> net2 is shifted by (shiftx,shifty)\n\n");
        exit(0);
    }

    mem_alloc();

    ReadOperands(argv[ns + 1], argv[ns + 2]);
    pfname = (char *)emalloc(sizeof(char) * strlen(argv[ns + 1]) + 10);
    strcpy(pfname, argv[ns + 1]);
    strcat(pfname, ".parsing");
    Restrictions(argv[ns + 4]);

    // define shifting of net2
    shifty = MaxYCoor(Op1);
    shiftx = MaxXCoor(Op1);
    if (argc - ns > 6) {
        switch (atoi(argv[ns + 6])) {
        case 1:
            shifty = 0;
            break;
        case 2:
            shiftx = 0;
            break;
        case 3:
            shiftx = IN_TO_PIX(atof(argv[ns + 7]));
            shifty = IN_TO_PIX(atof(argv[ns + 8]));
        }
    }
    else shifty = 0;

    switch (argv[ns + 3][0]) {
    case 't':
        Mode = 1;
        Transitions();
        TransitionTags();
        break;
    case 'p':
        Mode = 2;
        Places();
        PlaceTags();
        break;
    case 'b':
        Mode = 3;
        Transitions_Places();
        TransitionTags();
        PlaceTags();
        break;
    default:  printf("\n Unknown operator, type 'algebra' to see list of operators.\n\n");
        exit(0);
    }

    // printf(" AC: %d\n", ArcCount);

    netobj = Result;
    RescaleNet(Result, (double)rs);
    write_file(argv[ns + 5]);
    mem_free();
    free(pfname);
    printf("Done.\n");
    return 0;
}







