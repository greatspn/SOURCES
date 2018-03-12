#define ___RESCALE___
#include "global.h"
#include "menuvalues.h"
#include "chop.h"
#include "rescale.h"
#include "showgdi.h"
#include "Canvas.h"
#include <Mrm/MrmPublic.h>

static double   center_x, center_y, how_much;
int rescale_modified ;

static void RescaleNet(struct net_object *net) {
    struct place_object *place;
    struct arc_object *arc;
    struct coordinate *point;
    struct trans_object *trans;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct res_object *res;
    struct lisp_object *lisp;
    struct group_object *group;

    rescale_modified = 1;

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
            struct coordinate *temp_po = NULL;
            for (point = arc->point; point->next != NULL;
                    temp_po = point, point = point->next) {
                point->x = rescale_x(point->x);
                point->y = rescale_y(point->y);
            }
            arc->point->x = arc->trans->center.x;
            arc->point->y = arc->trans->center.y;
            point->x = arc->place->center.x;
            point->y = arc->place->center.y;
            ChopPlace(&(point->x), &(point->y),
                      temp_po->x, temp_po->y);
            ChopTrans(&(arc->point->x), &(arc->point->y),
                      arc->point->next->x, arc->point->next->y,
                      arc->trans->orient);
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

    RescaleList(how_much);
}

static void RescaleCB(Widget w, int closure, XtPointer call_data) {
    struct images  *im_p;
    Display   *display;
    extern Widget selected_pw;

    switch (closure) {
    case RESCALE_05_MITEM:
        how_much = 0.5;
        break;
    case RESCALE_06_MITEM:
        how_much = 0.6;
        break;
    case RESCALE_07_MITEM:
        how_much = 0.7;
        break;
    case RESCALE_08_MITEM:
        how_much = 0.8;
        break;
    case RESCALE_09_MITEM:
        how_much = 0.9;
        break;
    case RESCALE_10_MITEM:
        return;
    case RESCALE_11_MITEM:
        how_much = 1.1;
        break;
    case RESCALE_12_MITEM:
        how_much = 1.2;
        break;
    case RESCALE_13_MITEM:
        how_much = 1.3;
        break;
    case RESCALE_14_MITEM:
        how_much = 1.4;
        break;
    case RESCALE_15_MITEM:
        how_much = 1.5;
        break;
    case RESCALE_16_MITEM:
        how_much = 1.6;
        break;
    case RESCALE_17_MITEM:
        how_much = 1.7;
        break;
    case RESCALE_18_MITEM:
        how_much = 1.8;
        break;
    case RESCALE_19_MITEM:
        how_much = 1.9;
        break;
    case RESCALE_20_MITEM:
        how_much = 2.0;
    }
    if (moving_images == NULL) {
        center_x = 0.0;
        center_y = 0.0;
        printf("\n\nRescale : how_much %f\n", how_much);
        RescaleNet(netobj);
        gdiClearDrawingArea();
        redisplay_canvas();
        return;
    }
    center_x = select_left + (double) select_width * 0.5;
    center_y = select_top + (double) select_height * 0.5;
    select_left += trans_length;
    select_top += trans_length;
    select_width -= 2 * trans_length;
    select_height -= 2 * trans_length;
    select_top = rescale_y(select_top);
    select_left = rescale_x(select_left);
    select_width *= how_much;
    select_height *= how_much;
    select_left -= trans_length;
    select_top -= trans_length;
    select_width += 2 * trans_length;
    select_height += 2 * trans_length;
    for (im_p = moving_images; im_p != NULL; im_p = moving_images) {
        xv_scrivo(drawwin,
                  (int)(im_p->i_left * zoom_level),
                  (int)(im_p->i_top * zoom_level),
                  (int)(im_p->i_width * zoom_level),
                  (int)(im_p->i_height * zoom_level),
                  INV_PAINT_OP, im_p->i_p, 0, 0);
        if (prev_x > -10000.0 || prev_y > -10000.0) {
            xv_scrivo(drawwin,
                      (int)(cur_x - prev_x + im_p->i_left * zoom_level),
                      (int)(cur_y - prev_y + im_p->i_top * zoom_level),
                      (int)(im_p->i_width * zoom_level),
                      (int)(im_p->i_height * zoom_level),
                      INV_PAINT_OP, im_p->i_p, 0, 0);
        }
        if (im_p->i_p != (Pixmap)NULL) {
            display = (Display *) XtDisplay(selected_pw);
            XFreePixmap(display, im_p->i_p);
            im_p->i_p = (Pixmap)NULL;
        }
        moving_images = im_p->next;
        free((char *) im_p);
    }
    prev_x = prev_y = -10000.0;
    RescaleNet(&selected);
    recompute_images(1);

}

void RegisterRescale(void) {
    static MrmRegisterArg	regvec[] = {
        {"RescaleCB", (XtPointer)	RescaleCB}
    };

    RegisterNames(regvec, sizeof(regvec) / sizeof(MrmRegisterArg));
}
