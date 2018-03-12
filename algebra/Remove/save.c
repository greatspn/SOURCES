/*
 * save.c
 *
 * Routines to translate a GSPN internal data structures used by the editor into
 * the definition in file  "~/nets/netname.net"
 */

#include <string.h>

#include "global.h"
#include "layer.h"

//#define DEBUG

/*********************** local variables **************************************/
char            filename[100];

#define VBAR '|'

struct com_object *comm;    /* temp. comment */

/*****************************************************************************/

save_file(netobj)
struct net_object *netobj;  /* pointer to net */
{
    struct mpar_object *mpar;   /* temp. mpar_obj */
    struct place_object *place; /* temp. place_obj */
    struct rpar_object *rpar;   /* temp. rpar_obj */
    struct group_object *group; /* temp. group_obj */
    struct trans_object *trans; /* temp. trans_obj */
    struct res_object *res;
    struct lisp_object *lisp;
    int             i, knd, mark, nomp, nopl, norp, notr, nogr, nolay;


    /* write comment */

    fprintf(nfp, "|0|\n");
    if (netobj->comment != NULL)
        fprintf(nfp, "%s\n", netobj->comment);
    fprintf(nfp, "|\n");

    /* count the number of objects in the net */

    for (mpar = netobj->mpars, nomp = 0; mpar != NULL;
            mpar = mpar->next, nomp++);

    for (place = netobj->places, nopl = 0; place != NULL;
            place = place->next, nopl++);

    for (rpar = netobj->rpars, norp = 0; rpar != NULL;
            rpar = rpar->next, norp++);

    for (trans = netobj->trans, notr = 0; trans != NULL;
            trans = trans->next, notr++);

    for (group = netobj->groups, nogr = 0; group != NULL;
            group = group->next, nogr++)
        for (trans = group->trans; trans != NULL;
                trans = trans->next, notr++);

    nolay = HowManyLayers();
    fprintf(nfp, "f %3d %3d %3d %3d %3d %3d %3d\n", nomp, nopl, norp, notr, nogr, cset_num, nolay);

#ifdef DEBUG
    printf("  In save: Num. of objects counted\n");
#endif

    /* write marking parameters */
    for (mpar = netobj->mpars; mpar != NULL; mpar = mpar->next) {
        putname(nfp, mpar->tag);
        fprintf(nfp, " %3d %f %f", mpar->value,
                PIX_TO_IN(mpar->center.x), PIX_TO_IN(mpar->center.y));
        print_layer(mpar->layer);
        fprintf(nfp, "\n");
    }

#ifdef DEBUG
    printf("  In save: Marking Parameters written\n");
#endif

    /* write places */
    for (place = netobj->places; place != NULL; place = place->next) {
        putname(nfp, place->tag);
        if (place->cmark != NULL) {
            for (mark = -10001, lisp = netobj->lisps;
                    lisp != place->cmark;
                    lisp = lisp->next, mark--);
        }
        else if (place->mpar == NULL)
            mark = place->m0;
        else {
            for (mark = -1, mpar = netobj->mpars; mpar != place->mpar;
                    mpar = mpar->next, mark--);
        }
        fprintf(nfp, " %3d %f %f %f %f", mark,
                PIX_TO_IN(place->center.x), PIX_TO_IN(place->center.y),
                PIX_TO_IN(place->tagpos.x + place->center.x),
                PIX_TO_IN(place->tagpos.y + place->center.y));
        print_layer(place->layer);
        if (place->lisp != NULL) {
            struct lisp_object *lisp = netobj->lisps;
            int             nl = 1;
            for (; lisp != place->lisp; lisp = lisp->next)
                ++nl;
            fprintf(nfp, " %f %f @%d",
                    PIX_TO_IN(place->colpos.x + place->center.x),
                    PIX_TO_IN(place->colpos.y + place->center.y),
                    nl);
        }
        else if (place->color != NULL)
            fprintf(nfp, " %f %f %s",
                    PIX_TO_IN(place->colpos.x + place->center.x),
                    PIX_TO_IN(place->colpos.y + place->center.y),
                    place->color);
        fprintf(nfp, "\n");
    }

#ifdef DEBUG
    printf("  In save: Places written\n");
#endif

    /* write rate parameters */
    for (rpar = netobj->rpars; rpar != NULL; rpar = rpar->next) {
        putname(nfp, rpar->tag);
        fprintf(nfp, " %f %f %f", rpar->value,
                PIX_TO_IN(rpar->center.x), PIX_TO_IN(rpar->center.y));
        print_layer(rpar->layer);
        fprintf(nfp, "\n");
    }

#ifdef DEBUG
    printf("  In save: Rate Parameters written\n");
#endif

    /* write groups */
    for (group = netobj->groups; group != NULL; group = group->next) {
        putname(nfp, group->tag);
        fprintf(nfp, " %f %f %d\n",
                PIX_TO_IN(group->center.x), PIX_TO_IN(group->center.y),
                group->pri);
    }

#ifdef DEBUG
    printf("  In save: Groups written\n");
#endif

    /* write timed transitions */
    for (trans = netobj->trans, notr = 0; trans != NULL; trans = trans->next)
        put_trans(netobj, trans, trans->kind, ++notr);

#ifdef DEBUG
    printf("  In save: Timed transitions written\n");
#endif

    /* write immediate transitions */
    for (group = netobj->groups, knd = 1; group != NULL; group = group->next, knd++)
        for (trans = group->trans; trans != NULL; trans = trans->next)
            put_trans(netobj, trans, (short) knd, ++notr);

#ifdef DEBUG
    printf("  In save: Immediate transitions written\n");
#endif

    /* write layer names */
    for (i = 0; i++ < nolay;)
        fprintf(nfp, "%s\n", GetLayerName(i));

    /* close net file */

    /*    (void) fclose(nfp); */
    fprintf(dfp, "|256\n%%\n");

    /* write result definitions */

    for (res = netobj->results; res != NULL; res = res->next) {
        putc('|', dfp);
        putname(dfp, res->tag);
        fprintf(dfp, "%f %f : ",
                PIX_TO_IN(res->center.x), PIX_TO_IN(res->center.y));

        fprintf(dfp, "%s\n", res->text);

        /*
        for (comm = res->text; comm != NULL; comm = comm->next)
            fprintf(dfp, "%s\n", comm->line);
            */
    }
    fprintf(dfp, "|\n");

    /* write color-set/function definitions */

    if ((lisp = netobj->lisps) != NULL) {
        for (; lisp != NULL; lisp = lisp->next) {
            putc('(', dfp);
            putname(dfp, lisp->tag);
            fprintf(dfp, "%c %f %f (@%c\n", lisp->type,
                    PIX_TO_IN(lisp->center.x), PIX_TO_IN(lisp->center.y),
                    lisp->type);
            /*
            for (comm = lisp->text; comm != NULL; comm = comm->next)
            fprintf(dfp, "%s\n", comm->line);
            */
            fprintf(dfp, "%s\n", lisp->text);
            fprintf(dfp, "))\n");
        }
    }

    if (ferror(nfp) || ferror(dfp)) {
        fclose(nfp);
        fclose(dfp);
        return 1;
    }
    fclose(nfp);
    fclose(dfp);

    //put_msg(1,"Net saved on file %s", filename);
    //ResetModify();
    //rescale_modified = 0;
    return 0;
}

print_layer(layer)
LayerPun layer;
{
    int i;

    for (i = HowManyLayers(); i > 0; i--)
        if (TestLayer(i, layer)) {
            fprintf(nfp, "%3d", i);
        }
    fprintf(nfp, " 0");
}

put_trans(netobj, trans, knd, num)
struct net_object *netobj;
struct trans_object *trans;
short           knd;
int             num;
{
    struct rpar_object *rpar;
    struct arc_object *arc;
    int             i, j;
    float           ftemp;
    int             endep;

    /* write name */
    putname(nfp, trans->tag);

#ifdef DEBUG
    printf("  In save in put_trans: name of tr written\n");
#endif

    /* search for rate */
    endep = trans->enabl;
    if (trans->mark_dep != NULL) {
        ftemp = -510;
        fprintf(dfp, "|%d\n", num);
        /*
        for (comm = trans->mark_dep; comm != NULL; comm = comm->next)
            fprintf(dfp, "%s\n", comm->line);
         */
        fprintf(dfp, "%s\n", trans->mark_dep);
        printf("Save Mark Dep %s\n", trans->mark_dep);
    }
    else if (trans->rpar == NULL) {
        if (endep < 0)
            ftemp = trans->fire_rate.fp[0];
        else
            ftemp = trans->fire_rate.ff;
    }
    else {
        for (rpar = netobj->rpars, i = -1;
                rpar != trans->rpar; rpar = rpar->next, i--);
        ftemp = i;
    }

#ifdef DEBUG
    printf("  In save in put_trans: rate of tr written\n");
#endif

    /* count input arcs */
    for (arc = netobj->arcs, j = 0; arc != NULL; arc = arc->next)
        if (arc->trans == trans && arc->type == TO_TRANS)
            j++;

#ifdef DEBUG
    printf("  In save in put_trans: number of input arcs of tr. %s: %d\n", trans->tag, j);
#endif

    /* write first line */
    fprintf(nfp, " %f %3d %3d %3d %1d %f %f %f %f %f %f",
            ftemp, trans->enabl, knd, j, trans->orient,
            PIX_TO_IN(trans->center.x), PIX_TO_IN(trans->center.y),
            PIX_TO_IN(trans->tagpos.x + trans->center.x),
            PIX_TO_IN(trans->tagpos.y + trans->center.y),
            PIX_TO_IN(trans->ratepos.x + trans->center.x),
            PIX_TO_IN(trans->ratepos.y + trans->center.y));

#ifdef DEBUG
    printf("  In save in put_trans: first line is written for tr\n");
#endif

    print_layer(trans->layer);

    if (trans->lisp != NULL) {
        struct lisp_object *lisp = netobj->lisps;
        int             nl = 1;
        for (; lisp != trans->lisp; lisp = lisp->next)
            ++nl;
        fprintf(nfp, " %f %f @%d",
                PIX_TO_IN(trans->colpos.x + trans->center.x),
                PIX_TO_IN(trans->colpos.y + trans->center.y),
                nl);
    }
    else if (trans->color != NULL)
        fprintf(nfp, " %f %f %s",
                PIX_TO_IN(trans->colpos.x + trans->center.x),
                PIX_TO_IN(trans->colpos.y + trans->center.y),
                trans->color);
    fprintf(nfp, "\n");
    if (endep < 0) {
        i = endep = -endep;
        while (--i)
            fprintf(nfp, " %f\n", trans->fire_rate.fp[(endep - i)]);
    }

    putarcs(netobj, TO_TRANS, trans);

#ifdef DEBUG
    printf("  In save in put_trans: input arcs tr written\n");
#endif

    /* count output arcs */
    for (arc = netobj->arcs, j = 0; arc != NULL; arc = arc->next)
        if (arc->trans == trans && arc->type == TO_PLACE)
            j++;
    fprintf(nfp, "%4d\n", j);
    putarcs(netobj, TO_PLACE, trans);

    /* count inhibition arcs */
    for (arc = netobj->arcs, j = 0; arc != NULL; arc = arc->next)
        if (arc->trans == trans && arc->type == INHIBITOR)
            j++;
    fprintf(nfp, "%4d\n", j);
    putarcs(netobj, INHIBITOR, trans);
}


putname(outfile, name_pr)
FILE           *outfile;
char           *name_pr;
{
#define BLANK ' '
#define EOLN  '\0'

    while ((*name_pr) != EOLN)
        putc(*(name_pr++), outfile);
    putc(BLANK, outfile);
}


putarcs(netobj, kind, trans)
struct net_object *netobj;
char            kind;
struct trans_object *trans;
{
    short           pl, ip;
    struct place_object *place;
    struct arc_object *arc_pr;  /* temp. arc_obj */
    struct coordinate *ccp;

    for (arc_pr = netobj->arcs; arc_pr != NULL; arc_pr = arc_pr->next)
        if (arc_pr->trans == trans && arc_pr->type == kind) {
            for (place = netobj->places, pl = 1; place != arc_pr->place;
                    place = place->next, pl++);
            //printf("itt tart\n");
            for (ccp = arc_pr->point->next, ip = 0; ccp->next != NULL;
                    ccp = ccp->next, ip++);
            fprintf(nfp, "%4d %3d %3d", arc_pr->mult, pl, ip);
            print_layer(arc_pr->layer);
            if (arc_pr->lisp != NULL) {
                struct lisp_object *lisp = netobj->lisps;
                int             nl = 1;
                for (; lisp != arc_pr->lisp; lisp = lisp->next)
                    ++nl;
                fprintf(nfp, " %f %f @%d\n",
                        PIX_TO_IN(arc_pr->colpos.x), PIX_TO_IN(arc_pr->colpos.y),
                        nl);
            }
            else if (arc_pr->color == NULL)
                fprintf(nfp, "\n");
            else
                fprintf(nfp, " %f %f %s\n",
                        PIX_TO_IN(arc_pr->colpos.x), PIX_TO_IN(arc_pr->colpos.y),
                        arc_pr->color);
            for (ccp = arc_pr->point->next; ccp->next != NULL;
                    ccp = ccp->next)
                fprintf(nfp, "%f %f\n",
                        PIX_TO_IN(ccp->x), PIX_TO_IN(ccp->y));
        }
}

static char name[LINEMAX];

write_file(netname)
char           *netname;
{
    while (netname != NULL && *netname == ' ')
        netname++;
    if (netname == NULL || *netname == '\0')
        netname = edit_file;
    if (*netname == '\0')
        printf("You must enter some net name\n");
    else {
        strcpy(name, netname);
        return put_file();
    }
}

put_file() {
    char message[300];

    sprintf(filename, "%s.def", name);
    if ((dfp = fopen(filename, "w")) == NULL) {
        /*      sprintf(message,"Can't open file %s for write", filename);
        ShowErrorDialog(message,frame_w);*/
        return (1);
    }
    else {
        //SetApplicationTitle(name);
        sprintf(filename, "%s.net", name);
        if ((nfp = fopen(filename, "w")) == NULL) {
            fclose(dfp);
            /*      sprintf(message,"Can't open file %s for write", filename);
            ShowErrorDialog(message,frame_w);*/
            return (1);
        }
        else {
            sprintf(edit_file, "%s", name);
            return save_file(netobj);
        }
    }
}
