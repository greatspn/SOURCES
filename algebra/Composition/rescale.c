#include "global.h"

#define rescale_x(oldx) (center_x+((oldx)-center_x)*how_much)
#define rescale_y(oldy) (center_y+((oldy)-center_y)*how_much)

void RescaleNet(struct net_object *net, double how_much) {
    struct place_object *place;
    struct arc_object *arc;
    struct coordinate *point;
    struct trans_object *trans;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct res_object *res;
    struct lisp_object *lisp;
    struct group_object *group;

    double center_x = 0.0, center_y = 0.0;

    /* move marking parameters */

    for (mpar = net->mpars; mpar != NULL; mpar = mpar->next) {
        mpar->center.x = rescale_x(mpar->center.x);
        mpar->center.y = rescale_y(mpar->center.y);
    }

    /* move places */

    for (place = net->places; place != NULL; place = place->next) {
        place->center.x = rescale_x(place->center.x);
        place->center.y = rescale_y(place->center.y);
        place->tagpos.x *= how_much;
        place->tagpos.y *= how_much;
        place->colpos.x *= how_much;
        place->colpos.y *= how_much;
    }

    /* move rate parameters */

    for (rpar = net->rpars; rpar != NULL; rpar = rpar->next) {
        rpar->center.x = rescale_x(rpar->center.x);
        rpar->center.y = rescale_y(rpar->center.y);
    }

    /* move timed transitions */

    for (trans = net->trans; trans != NULL; trans = trans->next) {
        trans->center.x = rescale_x(trans->center.x);
        trans->center.y = rescale_y(trans->center.y);
        trans->tagpos.x *= how_much;
        trans->tagpos.y *= how_much;
        trans->pripos.x *= how_much;
        trans->pripos.y *= how_much;
        trans->colpos.x *= how_much;
        trans->colpos.y *= how_much;
        trans->ratepos.x *= how_much;
        trans->ratepos.y *= how_much;
    }

    /* move immediate transitions */

    for (group = net->groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            trans->center.x = rescale_x(trans->center.x);
            trans->center.y = rescale_y(trans->center.y);
            trans->tagpos.x *= how_much;
            trans->tagpos.y *= how_much;
            trans->pripos.x *= how_much;
            trans->pripos.y *= how_much;
            trans->colpos.x *= how_much;
            trans->colpos.y *= how_much;
            trans->ratepos.x *= how_much;
            trans->ratepos.y *= how_much;
        }

    /* move arcs */

    for (arc = net->arcs; arc != NULL; arc = arc->next) {
        if (net == netobj) {
            struct coordinate *temp_po;
            for (point = arc->point; point->next != NULL;
                    temp_po = point, point = point->next) {
                point->x = rescale_x(point->x);
                point->y = rescale_y(point->y);
            }
            arc->point->x = arc->trans->center.x;
            arc->point->y = arc->trans->center.y;
            point->x = arc->place->center.x;
            point->y = arc->place->center.y;
            //ChopPlace(&(point->x), &(point->y),
            //       temp_po->x, temp_po->y);
            //ChopTrans(&(arc->point->x), &(arc->point->y),
            //       arc->point->next->x, arc->point->next->y,
            //       arc->trans->orient);
            arc->colpos.x *= how_much;
            arc->colpos.y *= how_much;
        }
        else
            for (point = arc->point; point != NULL; point = point->next)
                if (point->x >= 0.0 && point->y >= 0.0) {
                    point->x = rescale_x(point->x);
                    point->y = rescale_y(point->y);
                }
    }

    /* move results */

    for (res = net->results; res != NULL; res = res->next) {
        res->center.x = rescale_x(res->center.x);
        res->center.y = rescale_y(res->center.y);
    }

    /* move color/function definitions */

    for (lisp = net->lisps; lisp != NULL; lisp = lisp->next) {
        lisp->center.x = rescale_x(lisp->center.x);
        lisp->center.y = rescale_y(lisp->center.y);
    }
}

