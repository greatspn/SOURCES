/*
 * load.c
 *
 * Routines to translate a GSPN definition in file  "~/nets/netname.net" into
 * the internal data structures used by the editor.
 */

/*#define DEBUG*/


#define VBAR '|'

#include "global.h"
#include "menuS.h"
#include "chop.h"
#include "trap.h"
#include "cc.h"
#include "deadl.h"
#include "ecs.h"
#include "res.h"
#include "tinv.h"
#include "pinv.h"
#include "sc.h"
#include "unbound.h"
#include "me.h"
#include "funct.h"
#include "Canvas.h"
#include "showgdi.h"
#include "layer.h"
#include "layerviewdialog.h"


struct net_object merge_net;
struct net_object *merge_netobj = &merge_net;
int merging;

static Layer maxlay;

void load_file(struct net_object *netobj) {
    char           *first;	/* 1st word on line */
    char           *c_p;
    char 			*temp;
    float           ftemp;	/* temporary float */
    char            linebuf[LINEMAX];	/* line buf */
//    struct com_object *tail;	/* temp. comment */
    struct mpar_object *mpar;	/* temp. mpar_obj */
    struct mpar_object *prev_mpar = NULL;	/* temp. mpar_obj */
    struct place_object *prev_place = NULL;	/* temp. place_obj */
    struct place_object *place;	/* temp. place_obj */
    struct rpar_object *rpar;	/* temp. rpar_obj */
    struct rpar_object *prev_rpar = NULL;	/* temp. rpar_obj */
    struct group_object *group;	/* temp. group_obj */
    struct group_object *prev_group = NULL;	/* temp. group_obj */
    struct trans_object *trans;	/* temp. trans_obj */
    struct trans_object *prev_trans;	/* temp. trans_obj */
    struct res_object *res;
    struct res_object *prev_res;
    struct lisp_object *lisp;
    struct lisp_object *prev_lisp;
    float           x, tagx, ratex, y, tagy, ratey;
    int             i, j, k, knd, ln, noar, mark, nomp, norp, nolay, orien;
    int             goon;
    char            c;
    Boolean		versionFlag;


    /* skip first line containing '|0|' */
    fgets(linebuf, LINEMAX, nfp);

#ifdef DEBUG
    fprintf(stderr, "reading comment\n");
#endif
    /* get comment */
    first = (char *) XtMalloc(1000);
    c_p = (char *) emalloc(1000);
    *first = '\0';
    do {
        fgets(c_p, 1000, nfp);
        if (*c_p == VBAR)
            break;
        strcat(first, c_p);
    }
    while (1);
    free(c_p);
    netobj->comment = first;

    /* read number of objects in the net */

#ifdef DEBUG
    fprintf(stderr, "reading object numbers\n");
#endif
    nolay = -1;
    maxlay = 0;
    fscanf(nfp, "f %d %d %d", &nomp, &place_num, &norp);
    fscanf(nfp, "%d %d %d %d\n", &trans_num, &group_num, &cset_num, &nolay);
    versionFlag = nolay == -1;
    if (versionFlag)
        nolay = 31;
    printf("Numero Layers Caricati %d\n", nolay);
    SetInitialLayerDimension(nolay);

#ifdef DEBUG
    fprintf(stderr, "reading Marking Parameters\n");
#endif
    /* read marking parameters */
    i = 1;
    while (i <= nomp) {
        mpar = (struct mpar_object *) emalloc(MPAOBJ_SIZE);
        mpar->tag = (char *) emalloc(TAG_SIZE);
        getname(mpar->tag);
        fscanf(nfp, "%d %f %f", &mark, &x, &y);
        mpar->value = mark;
        mpar->center.x = IN_TO_PIX(x);
        mpar->center.y = IN_TO_PIX(y);
        mpar->layer = NewLayerList(WHOLENET, NULL);
        if ((goon = fscanf(nfp, "%d", &ln))) {
            for (; goon && ln > 0; goon = fscanf(nfp, "%d", &ln)) {
                SetLayer2LayerList(ln, mpar->layer);
                if (maxlay < ln)
                    maxlay = ln;
            }
            while (getc(nfp) != '\n');
        }
        if (i == 1)
            netobj->mpars = mpar;
        else
            prev_mpar->next = mpar;
        prev_mpar = mpar;
        if (i++ == nomp)
            mpar->next = NULL;
    }

#ifdef DEBUG
    fprintf(stderr, "reading Places\n");
#endif
    /* read places */
    i = 1;
    while (i <= place_num) {
        place = (struct place_object *) emalloc(PLAOBJ_SIZE);
        place->tag = (char *) emalloc(TAG_SIZE);
        getname(place->tag);
        place->color = NULL;
        place->lisp = NULL;
        place->distr = NULL;
        place->layer = NewLayerList(WHOLENET, NULL);
        fscanf(nfp, "%d %f %f", &mark, &x, &y);
        place->center.x = IN_TO_PIX(x);
        place->center.y = IN_TO_PIX(y);
        if (!fscanf(nfp, "%f %f", &tagx, &tagy)) {
            place->tagpos.x = place_radius + 5;
            place->tagpos.y = -2;
        }
        else {
            place->tagpos.x = IN_TO_PIX(tagx) - place->center.x;
            place->tagpos.y = IN_TO_PIX(tagy) - place->center.y;
            if ((goon = fscanf(nfp, "%d", &ln))) {
                for (; goon && ln > 0; goon = fscanf(nfp, "%d", &ln)) {
                    SetLayer2LayerList(ln, place->layer);
                    if (maxlay < ln)
                        maxlay = ln;
                }
                while ((c = getc(nfp)) == ' ');
                if (c == '\n') {
                    place->colpos.x = place_radius + 5;
                    place->colpos.y = 8;
                }
                else {
                    char           *cp;
                    ungetc(c, nfp);
                    fscanf(nfp, "%f %f", &x, &y);
                    place->colpos.x = IN_TO_PIX(x) - place->center.x;
                    place->colpos.y = IN_TO_PIX(y) - place->center.y;
                    fgets(linebuf, LINEMAX, nfp);
                    for (cp = linebuf; *cp != '\n' && *cp != '\0'; cp++);
                    *cp = '\0';
                    for (cp = linebuf; *cp == ' '; cp++);
                    if (*cp == '@') {
                        size_t             ii;
                        sscanf(cp + 1, "%zu", &ii);
                        place->lisp = (struct lisp_object *) ii;
                    }
                    else {
                        place->color = (char *) emalloc(strlen(cp) + 1);
                        strcpy(place->color, cp);
                    }
                }
            }
            else
                while (getc(nfp) != '\n');
        }
        if (mark >= 0) {
            place->m0 = mark;
            place->tokens = mark;
            place->mpar = NULL;
            place->cmark = NULL;
        }
        else {
            if (mark < -10000) {
                place->m0 = 0;
                place->tokens = 0;
                place->mpar = NULL;
                place->cmark = (struct lisp_object *)((size_t)(-10000 - mark));
            }
            else {
                place->m0 = -1;
                place->tokens = -1;
                place->cmark = NULL;
                for (mpar = netobj->mpars, j = -1;
                        j > mark; mpar = mpar->next, j--);
                place->mpar = mpar;
            }
        }
        if (i == 1)
            netobj->places = place;
        else
            prev_place->next = place;
        prev_place = place;
        if (i++ == place_num)
            place->next = NULL;
    }

#ifdef DEBUG
    fprintf(stderr, "reading Rate Parameters\n");
    fflush(stderr);
#endif
    /* read rate parameters */
    i = 1;
    while (i <= norp) {
        rpar = (struct rpar_object *) emalloc(RPAOBJ_SIZE);
        rpar->tag = (char *) emalloc(TAG_SIZE);
        getname(rpar->tag);
        fscanf(nfp, "%f %f %f", &ftemp, &x, &y);
        rpar->layer = NewLayerList(WHOLENET, NULL);
        if ((goon = fscanf(nfp, "%d", &ln))) {
            for (; goon && ln > 0; goon = fscanf(nfp, "%d", &ln)) {
                SetLayer2LayerList(ln, rpar->layer);
                if (maxlay < ln)
                    maxlay = ln;
            }
            while (getc(nfp) != '\n');
        }
        rpar->value = ftemp;
        rpar->center.x = IN_TO_PIX(x);
        rpar->center.y = IN_TO_PIX(y);
        if (i == 1)
            netobj->rpars = rpar;
        else
            prev_rpar->next = rpar;
        prev_rpar = rpar;
        if (i++ == norp)
            rpar->next = NULL;
    }

#ifdef DEBUG
    fprintf(stderr, "reading Priorities\n");
    fflush(stderr);
#endif
    /* read groups */
    i = 1;
    while (i <= group_num) {
        group = (struct group_object *) emalloc(GRPOBJ_SIZE);
        group->tag = (char *) emalloc(TAG_SIZE);
        getname(group->tag);
        fscanf(nfp, "%f %f", &x, &y);
        if ((c = getc(nfp)) == ' ') {
            fscanf(nfp, "%d\n", &knd);
            group->pri = knd;
        }
        else
            group->pri = i;
        group->trans = NULL;
        group->center.x = IN_TO_PIX(x);
        group->center.y = IN_TO_PIX(y);
        group->movelink = NULL;
        if (i == 1)
            netobj->groups = group;
        else
            prev_group->next = group;
        prev_group = group;
        if (i++ == group_num)
            group->next = NULL;
    }

#ifdef DEBUG
    fprintf(stderr, "reading Transitions\n");
    fflush(stderr);
#endif
    /* read transitions */
    fgets(linebuf, LINEMAX, dfp);
    for (i = 1; i <= trans_num; i++) {
#ifdef DEBUG
        fprintf(stderr, "\n    trans %d\n", i);
        fflush(stderr);
#endif
        trans = (struct trans_object *) emalloc(TRNOBJ_SIZE);
        trans->tag = (char *) emalloc(TAG_SIZE);
        trans->color = NULL;
        trans->lisp = NULL;
        getname(trans->tag);
        fscanf(nfp, "%f %d %d %d %d %f %f",
               &ftemp, &mark, &knd, &noar, &orien, &x, &y);
#ifdef DEBUG
        fprintf(stderr, "          %s enabl=%d\n", trans->tag, mark);
        fflush(stderr);
#endif
        trans->layer = NewLayerList(WHOLENET, NULL);
        trans->center.x = IN_TO_PIX(x);
        trans->center.y = IN_TO_PIX(y);
        while ((c = getc(nfp)) == ' ');
        if (c == '\n') {
            trans->tagpos.x = -(trans_length / 2 + 10);
            trans->tagpos.y = -18;
            trans->ratepos.x = trans_length / 2 + 3;
            trans->ratepos.y = titrans_height;
        }
        else {
            ungetc(c, nfp);
            fscanf(nfp, "%f %f", &tagx, &tagy);
            trans->tagpos.x = IN_TO_PIX(tagx) - trans->center.x;
            trans->tagpos.y = IN_TO_PIX(tagy) - trans->center.y;
            while ((c = getc(nfp)) == ' ');
            if (c == '\n') {
                trans->ratepos.x = trans_length / 2 + 3;
                trans->ratepos.y = titrans_height;
            }
            else {
                ungetc(c, nfp);
                fscanf(nfp, "%f %f", &ratex, &ratey);
                trans->ratepos.x = IN_TO_PIX(ratex) - trans->center.x;
                trans->ratepos.y = IN_TO_PIX(ratey) - trans->center.y;
                while ((c = getc(nfp)) == ' ');
                if (c != '\n') {
                    ungetc(c, nfp);
                    for (fscanf(nfp, "%d", &ln); ln > 0; fscanf(nfp, "%d", &ln)) {
                        SetLayer2LayerList(ln, trans->layer);
                        if (maxlay < ln)
                            maxlay = ln;
                    }
                    while ((c = getc(nfp)) == ' ');
                    if (c == '\n') {
                        trans->colpos.x = trans_length / 2 + 3;
                        trans->colpos.y = -7;
                    }
                    else {
                        char           *cp;
                        ungetc(c, nfp);
                        fscanf(nfp, "%f %f", &x, &y);
                        trans->colpos.x = IN_TO_PIX(x) - trans->center.x;
                        trans->colpos.y = IN_TO_PIX(y) - trans->center.y;
                        fgets(linebuf, LINEMAX, nfp);
                        for (cp = linebuf; *cp != '\n' && *cp != '\0'; cp++);
                        *cp = '\0';
                        for (cp = linebuf; *cp == ' '; cp++);
                        if (*cp == '@') {
                            size_t             ii;
                            sscanf(cp + 1, "%zu", &ii);
                            trans->lisp = (struct lisp_object *) ii;
                        }
                        else {
                            trans->color = (char *) emalloc(strlen(cp) + 1);
                            strcpy(trans->color, cp);
                        }
                    }
                }
            }
        }
        if (ftemp > 0.0) {
            trans->fire_rate.ff = ftemp;
            trans->rpar = NULL;
            trans->mark_dep = NULL;
        }
        else {
            trans->fire_rate.ff = 1.0;
            k = (int)((-ftemp) + 0.5);
            if (k > norp) {
                trans->rpar = NULL;
                trans->mark_dep = NULL;
                first = (char *) emalloc(LINEMAX);
                fgets(first, LINEMAX, dfp);
                if (*first != VBAR) {
                    char *temp = (char *)emalloc(100000);

                    strcpy(temp, first);

                    for (;;) {
                        fgets(first, LINEMAX, dfp);
                        if (*first == VBAR)
                            break;
                        strcat(temp, first);
                    }
                    trans->mark_dep = (char *)emalloc(strlen(temp) + 1);
                    strcpy(trans->mark_dep, temp);
                    free(temp);
                }
                free(first);
            }
            else {
                for (rpar = netobj->rpars, j = 1; j++ < k; rpar = rpar->next);
                trans->rpar = rpar;
                trans->mark_dep = NULL;
                trans->fire_rate.ff = ftemp = rpar->value;
            }
        }
        if (mark < 0) {
            j = -mark;
            trans->fire_rate.fp = (float *) emalloc(j * sizeof(float));
            trans->fire_rate.fp[0] = ftemp;
            while (--j) {
                fscanf(nfp, "%f\n", &ftemp);
                trans->fire_rate.fp[-(mark + j)] = ftemp;
            }
        }
        trans->enabl = mark;
        trans->enabled = FALSE;
        trans->orient = orien;
#ifdef DEBUG
        fprintf(stderr, "      %d inputs\n", noar);
        fflush(stderr);
#endif
        getarcs(netobj, TO_TRANS, trans, noar);
        fscanf(nfp, "%d\n", &noar);
#ifdef DEBUG
        fprintf(stderr, "      %d outputs\n", noar);
        fflush(stderr);
#endif
        getarcs(netobj, TO_PLACE, trans, noar);
        fscanf(nfp, "%d\n", &noar);
#ifdef DEBUG
        fprintf(stderr, "      %d inhibitors\n", noar);
        fflush(stderr);
#endif
        getarcs(netobj, INHIBITOR, trans, noar);
        if (knd == EXPONENTIAL || knd == DETERMINISTIC) {
            if (netobj->trans == NULL) {
                netobj->trans = trans;
            }
            else {
                for (prev_trans = netobj->trans; prev_trans->next != NULL;
                        prev_trans = prev_trans->next);
                prev_trans->next = trans;
            }
            trans->kind = knd;
            trans->next = NULL;
        }
        else {
            for (group = netobj->groups, j = 1; j < knd;
                    j++, group = group->next);
            if (group->trans == NULL) {
                group->trans = trans;
            }
            else {
                for (prev_trans = group->trans; prev_trans->next != NULL;
                        prev_trans = prev_trans->next);
                prev_trans->next = trans;
            }
            trans->kind = group->pri;
            trans->next = NULL;
        }
    }

#ifdef DEBUG
    fprintf(stderr, "reading Result Definitions\n");
    fflush(stderr);
#endif
    /* read result definitions */
    fgets(linebuf, LINEMAX, dfp);
    fgets(linebuf, LINEMAX, dfp);
    for (prev_res = netobj->results = NULL;;) {
        first = &(linebuf[1]);
        if ((*first < 'a' || *first > 'z')
                && (*first < 'A' || *first > 'Z'))
            break;
        res = (struct res_object *) emalloc(RESOBJ_SIZE);
        res->next = NULL;
        if (netobj->results == NULL)
            netobj->results = res;
        else
            prev_res->next = res;
        prev_res = res;
        res->tag = emalloc(TAG_SIZE);
        for (c_p = res->tag; *first != ' ';)
            *(c_p++) = *(first++);
        *c_p = '\0';
        res->value1 = -1.0;

        temp = (char *) emalloc(100000);
        /*
        com->line = emalloc(LINEMAX);
        com->next = NULL;

        */
        sscanf(first, "%f %f :", &x, &y);
        while (*(first++) != ':');
        while (*first == ' ')
            first++;
        for (c_p = first; *c_p != '\n' && *c_p != '\0'; c_p++);
        *c_p = '\0';

        /*		strcpy(com->line, first); */
        strcpy(temp, first);
        res->center.x = IN_TO_PIX(x);
        res->center.y = IN_TO_PIX(y);
        for (fgets(linebuf, LINEMAX, dfp); linebuf[0] != VBAR;
                fgets(linebuf, LINEMAX, dfp)) {
            /*
            com = com->next = (struct com_object *) emalloc(CMMOBJ_SIZE);
            com->next = NULL;
            com->line = emalloc(LINEMAX);
            */
            for (c_p = &(linebuf[0]); *c_p != '\n' && *c_p != '\0'; c_p++);
            *c_p = '\0';

            /*		strcpy(com->line, &(linebuf[0]));  */
            strcat(temp, "\n");
            strcat(temp, &(linebuf[0]));
        }
        res->text = strdup(temp);
        free(temp);
    }

    /* read color-set/function definitions */

#ifdef DEBUG
    fprintf(stderr, "reading Colourset Definitions\n");
    fflush(stderr);
#endif
    fgets(linebuf, LINEMAX, dfp);
    netobj->lisps = NULL;

    if (linebuf[0] == '(') {
        int             nl = 0;
        struct arc_object *last_arc;
        struct group_object *group;
        char *templisp;
        for (prev_lisp = NULL; linebuf[0] == '(';) {
            ++nl;
            first = &(linebuf[1]);
            lisp = (struct lisp_object *) emalloc(LISPOBJ_SIZE);
            for (last_place = netobj->places; last_place != NULL;
                    last_place = last_place->next)
                if ((size_t)(last_place->cmark) == nl) {
                    last_place->cmark = lisp;
                }
            for (last_place = netobj->places; last_place != NULL;
                    last_place = last_place->next)
                if ((size_t)(last_place->lisp) == nl) {
                    last_place->lisp = lisp;
                }
            for (last_trans = netobj->trans; last_trans != NULL;
                    last_trans = last_trans->next)
                if ((size_t)(last_trans->lisp) == nl) {
                    last_trans->lisp = lisp;
                }
            for (group = netobj->groups; group != NULL; group = group->next)
                for (last_trans = group->trans; last_trans != NULL;
                        last_trans = last_trans->next)
                    if ((size_t)(last_trans->lisp) == nl) {
                        last_trans->lisp = lisp;
                    }
            for (last_arc = netobj->arcs; last_arc != NULL;
                    last_arc = last_arc->next)
                if ((size_t)(last_arc->lisp) == nl) {
                    last_arc->lisp = lisp;
                }
            lisp->next = NULL;
            if (netobj->lisps == NULL)
                netobj->lisps = lisp;
            else
                prev_lisp->next = lisp;
            prev_lisp = lisp;
            for (c_p = first, j = 1; *c_p != ' '; ++c_p, ++j);
            lisp->tag = emalloc(j);
            for (c_p = lisp->tag; *first != ' ';)
                *(c_p++) = *(first++);
            *c_p = '\0';
            while (*first == ' ')
                ++first;
            lisp->type = *(first++);
            while (*first == ' ')
                ++first;
            sscanf(first, "%f %f", &x, &y);
            lisp->center.x = IN_TO_PIX(x);
            lisp->center.y = IN_TO_PIX(y);
            templisp = (char *) emalloc(100000);

            /*	    com->line = emalloc(LINEMAX);
            com->next = NULL;*/

            fgets(linebuf, LINEMAX, dfp);
            for (c_p = &(linebuf[0]); *c_p != '\n' && *c_p != '\0'; c_p++);
            *c_p = '\0';
            strcpy(templisp, linebuf);

            /*	    strcpy(com->line, &(linebuf[0])); */

            for (fgets(linebuf, LINEMAX, dfp);
                    linebuf[0] != ')' && linebuf[1] != ')';
                    fgets(linebuf, LINEMAX, dfp)) {
                /*
                 com = com->next = (struct com_object *) emalloc(CMMOBJ_SIZE);
                com->next = NULL;
                com->line = emalloc(LINEMAX);
                      */

                for (c_p = &(linebuf[0]); *c_p != '\n' && *c_p != '\0'; c_p++);
                *c_p = '\0';
                strcat(templisp, "\n");
                strcat(templisp, linebuf);

                /*		strcpy(com->line, &(linebuf[0])); */
            }
            lisp->text = strdup(templisp);
            free(templisp);
            fgets(linebuf, LINEMAX, dfp);
        }
    }
#ifdef DEBUG
    fprintf(stderr, "reading Layers\n");
    fflush(stderr);
#endif
    /* read layer names  */
    SetLayersNumber(versionFlag ?  maxlay  : nolay);
    for (ln = 1; ln <= (versionFlag ?  maxlay  : nolay); ln++) {
        fgets(linebuf, LINEMAX, nfp);
        for (c_p = linebuf; *c_p != '\n' && *c_p != '\0'; c_p++);
        if (*c_p == '\n')
            *c_p = '\0';
        SetLayerName(ln, linebuf);
        MakeLayerVisible(ln);
    }

    UpdateLayerDialogs();

    /* read subnets  */
#ifdef DEBUG
    fprintf(stderr, "End loading\n");
    fflush(stderr);
#endif
}


void getname(char           *name_pr) {
#define	BLANK ' '
#define	EOLN  '\0'
    short           i;

    for ((*name_pr) = fgetc(nfp), i = 1;
            (*name_pr) != BLANK && (*name_pr) != '\n' &&
            (*name_pr) != (char)0 && i++ <= TAG_SIZE;
            (*(++name_pr)) = fgetc(nfp));
    if (*name_pr != BLANK) {
        char            c;
        for (c = fgetc(nfp);
                c != BLANK && c != '\n' && c != (char)0;
                c = fgetc(nfp));
    }
    (*name_pr) = EOLN;
}


void getarcs(struct net_object *netobj,
             char            kind,
             struct trans_object *trans,
             int             noar) {
    int             i, j, ln, pl, mlt, ip;
    float           x, y;
    struct place_object *place;
    struct arc_object *arc_pr;
    struct coordinate *cph, *cpt, *ccp;
    char            linebuf[LINEMAX];
    char           *cp1, c;

    for (i = 1; i <= noar; i++) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        for (place = netobj->places, j = 1; j < pl; j++) {
            place = place->next;
        }
        arc_pr = (struct arc_object *) emalloc(ARCOBJ_SIZE);
        arc_pr->layer = NewLayerList(WHOLENET, NULL);
        arc_pr->color = NULL;
        arc_pr->lisp = NULL;
        while ((c = getc(nfp)) == ' ');
        if (c != '\n') {
            ungetc(c, nfp);
            for (fscanf(nfp, "%d", &ln); ln > 0; fscanf(nfp, "%d", &ln)) {
                SetLayer2LayerList(ln, arc_pr->layer);
                if (maxlay < ln)
                    maxlay = ln;
            }
            while ((c = getc(nfp)) == ' ');
            if (c != '\n') {
                /* arc with tag */
                ungetc(c, nfp);
                fscanf(nfp, "%f %f", &x, &y);
                arc_pr->colpos.x = IN_TO_PIX(x);
                arc_pr->colpos.y = IN_TO_PIX(y);
                fgets(linebuf, LINEMAX, nfp);
                for (cp1 = linebuf; *cp1 != '\n' && *cp1 != '\0'; cp1++);
                *cp1 = '\0';
                for (cp1 = linebuf; *cp1 == ' '; cp1++);
                if (*cp1 == '@') {
                    size_t             ii;
                    sscanf(cp1 + 1, "%zu", &ii);
                    arc_pr->lisp = (struct lisp_object *) ii;
                }
                else {
                    arc_pr->color = emalloc((unsigned)(strlen(cp1) + 1));
                    strcpy(arc_pr->color, cp1);
                }
            }
        }
        arc_pr->type = kind;
        arc_pr->mult = mlt;
        arc_pr->place = place;
        arc_pr->trans = trans;
        cph = (struct coordinate *) emalloc(COORD_SIZE);
        cpt = (struct coordinate *) emalloc(COORD_SIZE);
        cpt->x = place->center.x;
        cpt->y = place->center.y;
        cph->x = trans->center.x;
        cph->y = trans->center.y;
        arc_pr->point = cph;
        for (ccp = cph, j = 1; j++ <= ip; ccp = ccp->next) {
            ccp->next = (struct coordinate *) emalloc(COORD_SIZE);
            fscanf(nfp, "%f %f\n", &x, &y);
            (ccp->next)->x = IN_TO_PIX(x);
            (ccp->next)->y = IN_TO_PIX(y);
        }
        ccp->next = cpt;
        cpt->next = NULL;

        ChopPlace(&(cpt->x), &(cpt->y), ccp->x, ccp->y);
        ChopTrans(&(cph->x), &(cph->y),
                  cph->next->x, cph->next->y, trans->orient);

        arc_pr->next = netobj->arcs;
        netobj->arcs = arc_pr;
    }
}


void read_file(char           *netname) {
    char            filename[LINEMAX];
    /*
    while (netname != NULL && *netname == ' ')
    netname++;
    if (netname == NULL || *netname == '\0')
    netname = edit_file;
    if (*netname == '\0')
      StatusDisplay("You must enter some net name !!");
    else
    */
    {
        sprintf(filename, "%s.def", netname);
        if ((dfp = fopen(filename, "r")) == NULL)
            StatusPrintf("Can't open file %s", filename);
        else {
            sprintf(filename, "%s.net", netname);
            if ((nfp = fopen(filename, "r")) == NULL)
                StatusPrintf("Can't open file %s", filename);
            else {
                clear_proc_f();
                sprintf(edit_file, "%s", netname);
                StatusDisplay("Reading .....");
                load_file(netobj);
#ifdef DEBUG
                fprintf(stderr, "file loaded\n");
#endif
                (void) fclose(nfp);
                (void) fclose(dfp);
                SetApplicationTitle(netname);
#ifdef DEBUG
                fprintf(stderr, ".net and .def files closed\n");
#endif
                if (netobj->comment == NULL);
                StatusPrintf("File %s read in.", filename);
                UpdateMenuStatusTo(NETLOADED_MSTATUS);
#ifdef DEBUG
                fprintf(stderr, "collecting results\n");
#endif

                collect_all();
                figure_modified = 0;
#ifdef DEBUG
                fprintf(stderr, "redisplaying canvas\n");
#endif
                gdiClearDrawingArea();
                redisplay_canvas();
#ifdef DEBUG
                fprintf(stderr, "canvas redisplayed\n");
#endif
            }
        }
    }
}


void merge_file(char           *netname) {
    char            filename[LINEMAX];

    puts("Merging File ..");
    sprintf(filename, "%s.def", netname);
    if ((dfp = fopen(filename, "r")) == NULL)
        StatusPrintf("Can't open file %s", filename);
    else {
        sprintf(filename, "%s.net", netname);
        if ((nfp = fopen(filename, "r")) == NULL)
            StatusPrintf("Can't open file %s", filename);
        else {
            clear_proc_m();
            sprintf(edit_file, "%s", netname);
            StatusDisplay("Merging .....");
            memset(merge_netobj, 0, sizeof(selected));
            load_file(merge_netobj);

#ifdef DEBUG
            fprintf(stderr, "file loaded\n");
#endif
            (void) fclose(nfp);
            (void) fclose(dfp);
            merging = 1;
            end_boxtracking(10000, 10000);
            /*         limit_selected(0);
                     force_selection();*/
        }
    }
}

/*
#ifdef DEBUG
		fprintf(stderr, ".net and .def files closed\n");
#endif
		if (netobj->comment == NULL);
		    StatusPrintf("File %s read in.", filename);
		    UpdateMenuStatusTo(NETLOADED_MSTATUS);
#ifdef DEBUG
		fprintf(stderr, "collecting results\n");
#endif

		collect_all();
		figure_modified = 0;
#ifdef DEBUG
		fprintf(stderr, "redisplaying canvas\n");
#endif
		gdiClearDrawingArea();
		redisplay_canvas();
#ifdef DEBUG
		fprintf(stderr, "canvas redisplayed\n");
#endif
      */

void collect_all() {

    CollectRes(FALSE);
    CollectPinvar(FALSE);
    CollectDeadl(FALSE);
    CollectTrap(FALSE);
    CollectTinvar(FALSE);
    CollectEcsSets(FALSE);
    CollectMesets(FALSE);
    CollectScsSets(FALSE);
    CollectCcSets(FALSE);
    CollectUnbound(FALSE);

}
