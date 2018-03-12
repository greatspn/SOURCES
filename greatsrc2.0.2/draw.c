#define ___DRAW___
#	include "global.h"
#	include "postscript.h"
#	include "circle.h"
#	include "spline.h"
#	include "arc.h"
#	include "showgdi.h"
#	include "color.h"
#	include "box.h"
#	include "overview.h"
#	include "tag.h"
#	include "mark.h"
#	include "lisp.h"
#	include "rate.h"
#	include "res.h"
#include "draw.h"

void RedisplayNet(void) {
    if (netobj != NULL)
        DrawNet(netobj, OR);
}

void DrawArrow(float x1, float y1, float x2, float y2, int op, DrawingSurfacePun ds) {
    float           x, y, xb, yb, dx, dy, l, sina, cosa;
    float             xc, yc, xd, yd, ix2, iy2;


    SetFunction(op, ds);

    dx = x2 - x1;
    dy = y1 - y2;
    if ((l = dx * dx + dy * dy) > 0.0)
        l = (float) sqrt(l);
    sina = dy / l;
    cosa = dx / l;
    xb = x2 * cosa - y2 * sina;
    yb = x2 * sina + y2 * cosa;
    x = xb - arrowht;
    y = yb - arrowwid / 2;
    xc = (x * cosa + y * sina);
    yc = (-x * sina + y * cosa);
    y = yb + arrowwid / 2;
    xd = (x * cosa + y * sina);
    yd = (-x * sina + y * cosa);
    ix2 = (x2);
    iy2 = (y2);
    if (!PSflag) {
        XDrawLine(ds->display, ds->drawable, ds->gc, (int)xc, (int)yc, (int)ix2, (int)iy2);
        XDrawLine(ds->display, ds->drawable, ds->gc, (int)xd, (int)yd, (int)ix2, (int)iy2);
    }
    else {
        PSline(xc, yc, ix2, iy2);
        PSline(xd, yd, ix2, iy2);
    }

    if (!PSflag) {
        XDrawLine(ds->display, ds->drawable, ds->gc, (int)x1, (int)y1, (int)ix2, (int)iy2);
    }
    else {
        PSline(x1, y1, ix2, iy2);
    }
}



void DrawPlace(struct place_object *place, int op) {
    if (!IsLayerListVisible(place->layer))
        return;

    PutPlace(place, op, &mainDS, 0.0, 0.0);
    DrawOverPlace(place, op);

}

void PutTrans(struct trans_object *trans, int op, DrawingSurfacePun ds, float x_off, float y_off) {
    int     xd = (int)(((trans->center.x - x_off) * zoom_level) - ((float)pixdim) / 2);
    int     yd = (int)(((trans->center.y - y_off) * zoom_level) - ((float)pixdim) / 2);

    if (!PSflag) {
        gdiCopyTransModel(trans, xd, yd, op, ds);
    }
    else {
        float angle;
        switch (trans->orient) {
        case HORIZONTAL:
            angle = 0.0;
            break;
        case FORTYFIVE:
            angle = 45.0;
            break;
        case MINUSFORTYFIVE:
            angle = -45.0;
            break;
        default:
            angle = 90.0;
            break;
        }
        switch (trans->kind) {
        case EXPONENTIAL:
            PSTransition((trans->center.x - x_off) * zoom_level, (trans->center.y - y_off) * zoom_level, trans_length, titrans_height, angle, 0);
            break;
        case  DETERMINISTIC:
            PSTransition((trans->center.x - x_off) * zoom_level, (trans->center.y - y_off) * zoom_level, trans_length, titrans_height, angle, 1);
            break;
        default:
            PSTransition((trans->center.x - x_off) * zoom_level, (trans->center.y - y_off) * zoom_level, trans_length, imtrans_height, angle, 1);
        }
    }
    if (tag_visible)
        ShowTransTag(trans, ds, x_off, y_off, op);

    if (trans->kind != DETERMINISTIC) {
        if (trans->kind > 1) {
            float           xx = (trans->center.x + trans->tagpos.x - x_off) * zoom_level;
            float           yy = (trans->center.y + trans->tagpos.y - y_off) * zoom_level;
            ShowTransPri(xx, yy, trans->kind, ds, op);
        }
    }

    ShowTransColor(trans, ds, x_off, y_off, op);
    if (rate_visible)
        ShowTransRate(trans, op);

}



void PutPlace(struct place_object *place, int op, DrawingSurfacePun ds, float x_off, float y_off) {
    float             xd = ((place->center.x - x_off) * zoom_level) - place_dec;
    float            yd = ((place->center.y - y_off) * zoom_level) - place_dec;

    if (!PSflag) {
        gdiCopyPlaceModel((int)xd, (int)yd, op, ds);
    }
    else
        PSCircle((place->center.x - x_off) * zoom_level, (place->center.y - y_off) * zoom_level, place_radius);

    if (tag_visible)
        ShowPlaceTag(place, ds, x_off, y_off, op);

    ShowPlaceColor(place, ds, x_off, y_off, op);



}




static void DrawTokenRow(int num, double f_x, int yloc, int action, int space, double diam) {
    int             xloc;
    int             n;
    int             ddd = diam;

    xloc = (int)(f_x + diam - (num * space + 1) * 0.51);
    for (n = 1; n <= num; n++) {
        Dot(xloc, yloc, action, ddd);
        xloc += space;
    }
}

void DrawTokens(int num, double f_x, double f_y, int action, double diam) {
    int             xloc, yloc, space;
    char            sss[10];

    space = (int)(1.5 * diam + 0.55);
    if (num <= 0)
        return;
    if (num > 7) {
        xloc = (int)(f_x - space - 1);
        yloc = (int)(f_y + space - 1);
        sprintf(sss, "%d", num);
        gdiDrawText(&mainDS, xloc, yloc, OR, canvas_font, sss);
        return;
    }
    if (num > 4) {
        yloc = (int)(f_y - diam + space * 0.5 + 1.1);
        DrawTokenRow(3, f_x, yloc, action, space, diam);
        if (num > 5) {
            DrawTokenRow(2, f_x, yloc + space, action, space, diam);
            num -= 5;
        }
        else {
            DrawTokenRow(1, f_x, yloc + space, action, space, diam);
            num -= 4;
        }
        DrawTokenRow(num, f_x, yloc - space, action, space, diam);
        return;
    }
    if (num > 2) {
        yloc = (int)(f_y - diam + space + 0.55);
        DrawTokenRow(2, f_x, yloc, action, space, diam);
        DrawTokenRow(num - 2, f_x, yloc - space, action, space, diam);
        return;
    }
    /* else num <= 2 */
    yloc = (int)(f_y - diam + (space + 1) * 0.51);
    DrawTokenRow(num, f_x, yloc, action, space, diam);
}


void DrawMarking(struct place_object *place, int action) {

    if (!IsLayerListVisible(place->layer))
        return;
    DrawTokens((int)(place->tokens), (double)(place->center.x * zoom_level),
               (double)(place->center.y * zoom_level), action, token_diameter);
}

void DrawTrans(struct trans_object *trans, int op) {

    if (!IsLayerListVisible(trans->layer))
        return;
    PutTrans(trans, op, &mainDS, 0.0, 0.0);
    DrawOverTrans(trans, op);
}


void ShowTransPri(float xx, float yy, int npri, DrawingSurfacePun ds, int op) {
    char            sss[5];
    char            ssp[5];
    int             xloc, yloc;

    sprintf(sss, "%3d", npri);
    ssp[0] = (char)112;
    ssp[1] = '\0';
    xloc = xx - 14 * zoom_level;
    yloc = yy + 20 * zoom_level;

    gdiDrawText(ds, xloc + 7, yloc, op, canvas_font, sss);
    if (!PSflag) {
        gdiDrawText(ds, xloc, yloc, op, sym_font, ssp);
    }
    else {
        PSPGreek(xloc, yloc);
    }
}

void DrawArc(struct arc_object *arc, int op) {

    if (!IsLayerListVisible(arc->layer))
        return;

    if (spline_on)
        DrawSpline(arc, op, &mainDS, 0, 0);
    else
        PutArc(arc, op, &mainDS, 0, 0);
}

void PutArc(struct arc_object *arc, int op, DrawingSurfacePun ds, int x_off, int y_off) {
    struct coordinate *point;
    float           xloc, yloc;
    char            string[LINEMAX];


    point = arc->point;
    if (point == NULL)
        return;
    if (point->next == NULL)
        return;
    switch (arc->type) {
    case TO_TRANS:
        DrawArrow((float)((ABS(point->next->x) - (float) x_off) * zoom_level),
                  (float)((ABS(point->next->y) - (float) y_off) * zoom_level),
                  (float)((ABS(point->x) - (float) x_off) * zoom_level),
                  (float)((ABS(point->y) - (float) y_off) * zoom_level),
                  op, ds);
        break;

    case INHIBITOR:
        DrawInhibitor(ABS(point->next->x), ABS(point->next->y),
                      ABS(point->x), ABS(point->y),
                      op, ds, x_off, y_off);
        break;

    default:
        if (point->next->next != NULL)
            DrawFloatVector((ABS(point->x) - x_off) * zoom_level,
                            (ABS(point->y) - y_off) * zoom_level,
                            (ABS(point->next->x) - x_off) * zoom_level,
                            (ABS(point->next->y) - y_off) * zoom_level,
                            op, ds);
    }
    if (point->next->next != NULL)
        point = point->next;
    if (arc->mult < 0) {
        xloc = (ABS(point->x) - x_off) * zoom_level;
        yloc = (ABS(point->y) - y_off) * zoom_level;
        if (arc->mult == -1)
            sprintf(string, "%s", arc->place->tag);
        else
            sprintf(string, "%1d*%s", -(arc->mult), arc->place->tag);

        gdiDrawText(ds, (int) xloc, (int) yloc, op, canvas_font, string);

        for (; point->next->next != NULL; point = point->next);

        xloc = (ABS(point->x) - x_off) * zoom_level;
        yloc = (ABS(point->y) - y_off) * zoom_level;

        if (arc->mult == -1)
            sprintf(string, "%s", arc->trans->tag);
        else
            sprintf(string, "%1d*%s", -(arc->mult), arc->trans->tag);

        gdiDrawText(ds, (int) xloc, (int) yloc, op, canvas_font, string);
    }
    else {
        for (; point->next->next != NULL; point = point->next)
            DrawFloatVector((ABS(point->x) - x_off) * zoom_level,
                            (ABS(point->y) - y_off) * zoom_level,
                            (ABS(point->next->x) - x_off) * zoom_level,
                            (ABS(point->next->y) - y_off) * zoom_level, op, ds);

        if (arc->color != NULL || arc->lisp != NULL) {
            xloc = ((ABS(point->x) + ABS(point->next->x)) * 0.5
                    + arc->colpos.x - x_off) * zoom_level;
            yloc = ((ABS(point->y) + ABS(point->next->y)) * 0.5
                    + arc->colpos.y - y_off) * zoom_level;
            if (arc->color != NULL) {
                gdiDrawText(ds, (int) xloc, (int) yloc, op, canvas_font, arc->color);
            }
            /*	 else
            	{
            	gdiDrawText(ds, (int) xloc, (int) yloc, op, canvas_font, arc->lisp->tag);
            	}*/
        }
        if (arc->mult > 1) {
            xloc = ((ABS(point->x) + ABS(point->next->x)) * 0.5 - x_off) * zoom_level;
            yloc = ((ABS(point->y) + ABS(point->next->y)) * 0.5 - y_off) * zoom_level;
            sprintf(string, "_%1d", arc->mult);
            gdiDrawText(ds, (int) xloc, (int) yloc, op, canvas_font, string);
        }
    }
    if (arc->type == TO_PLACE)
        DrawArrow((ABS(point->x) - x_off) * zoom_level,
                  (ABS(point->y) - y_off) * zoom_level,
                  (ABS(point->next->x) - x_off) * zoom_level,
                  (ABS(point->next->y) - y_off) * zoom_level, op, ds);
    else if (arc->point->next->next != NULL)
        DrawFloatVector((ABS(point->x) - x_off) * zoom_level,
                        (ABS(point->y) - y_off) * zoom_level,
                        (ABS(point->next->x) - x_off) * zoom_level,
                        (ABS(point->next->y) - y_off) * zoom_level, op, ds);
}


void DrawNet(struct net_object *net, int op) {
    struct mpar_object *last_mpar;
    struct rpar_object *last_rpar;
    struct group_object *last_group;
    struct res_object *last_res;
    struct lisp_object *last_lisp;

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing mpars\n");
#endif
    for (last_mpar = net->mpars; last_mpar != NULL; last_mpar = last_mpar->next)
        ShowMpar(last_mpar, op);

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing places\n");
#endif

    for (last_place = net->places; last_place != NULL; last_place = last_place->next) {
        DrawPlace(last_place, op);
        ShowPlaceMark(last_place, op);/*COPY*/
    }

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing rpars\n");
#endif

    for (last_rpar = net->rpars; last_rpar != NULL; last_rpar = last_rpar->next)
        ShowRpar(last_rpar, op);

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing timed transitions\n");
#endif

    for (last_trans = net->trans; last_trans != NULL; last_trans = last_trans->next)
        DrawTrans(last_trans, op);

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing immediate transitions\n");
#endif

    for (last_group = net->groups; last_group != NULL; last_group = last_group->next)
        for (last_trans = last_group->trans; last_trans != NULL; last_trans = last_trans->next)
            DrawTrans(last_trans, op);

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing arcs\n");
#endif

    for (last_arc = net->arcs; last_arc != NULL; last_arc = last_arc->next)
        DrawArc(last_arc, op);

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing results\n");
#endif

    for (last_res = net->results; last_res != NULL; last_res = last_res->next)
        ShowRes(last_res, op);

#ifdef DEBUG
    fprintf(stderr, "draw_net: showing lisps\n");
#endif

    for (last_lisp = net->lisps; last_lisp != NULL; last_lisp = last_lisp->next)
        ShowLisp(last_lisp, op); /*COPY*/
    /*    draw_vector(0., 0., 451., 0., OR, &mainDS);
     draw_vector(451., 0., 451., 698., OR, &mainDS);
     draw_vector(451., 698., 0., 698., OR, &mainDS);
     draw_vector(0., 698., 0., 0., OR, &mainDS);*/

#ifdef DEBUG
    fprintf(stderr, "End draw_net\n");
#endif
}

void PutNet(struct net_object *net, DrawingSurfacePun ds, int x_off, int y_off) {
    struct place_object *last_place;
    struct trans_object *last_trans;
    struct group_object *last_group;
    struct arc_object *last_arc;
    struct res_object *last_res;
    struct lisp_object *last_lisp;

    for (last_place = net->places; last_place != NULL; last_place = last_place->next)
        PutPlace(last_place, OR, ds, (float) x_off, (float) y_off);

    for (last_trans = net->trans; last_trans != NULL; last_trans = last_trans->next)
        PutTrans(last_trans, OR, ds, (float) x_off, (float) y_off);

    for (last_group = net->groups; last_group != NULL; last_group = last_group->next)
        for (last_trans = last_group->trans; last_trans != NULL; last_trans = last_trans->next)
            PutTrans(last_trans, OR, ds, (float) x_off, (float) y_off);

    for (last_arc = net->arcs; last_arc != NULL; last_arc = last_arc->next)
        if (spline_on)
            DrawSpline(last_arc, OR, ds, x_off, y_off);
        else
            PutArc(last_arc, OR, ds, x_off, y_off);

    for (last_res = net->results; last_res != NULL; last_res = last_res->next)
        PutRes(last_res, OR, ds, x_off, y_off);

    for (last_lisp = net->lisps; last_lisp != NULL; last_lisp = last_lisp->next)
        PutLisp(last_lisp, OR, ds, x_off, y_off);
}
/*
static void EraseNetMarking(net)
struct net_object *net;
{
   for (last_place = net->places; last_place != NULL;
	last_place = last_place->next)
      show_place_mark(last_place, CLEAR);
}

draw_net_marking(net)
struct net_object *net;
{
   for (last_place = net->places; last_place != NULL;
	last_place = last_place->next)
      show_place_mark(last_place, XOR);
}
*/

static void DrawInhibitor(float x0, float y0, float x1, float y1, int op, DrawingSurfacePun ds, int x_off, int y_off) {
    struct position p0, p1;
    float           float_x, float_y, chop_fract;
    float           loc_x, loc_y;

    float_x = x0 - x1;
    float_y = y0 - y1;
    if ((chop_fract = float_x * float_x + float_y * float_y) > 0.0)
        chop_fract = (float) sqrt(chop_fract);
    chop_fract = not_radius / chop_fract;
    float_x = float_x * chop_fract;
    float_y = float_y * chop_fract;
    loc_x = float_x + float_x + x1;
    loc_y = float_y + float_y + y1;
    p0.x = (x0 - x_off) * zoom_level;
    p0.y = (y0 - y_off) * zoom_level;
    p1.x = (loc_x - x_off) * zoom_level;
    p1.y = (loc_y - y_off) * zoom_level;
    DrawFloatVector(p0.x, p0.y, p1.x, p1.y, op, ds);
    loc_x = float_x + x1 - x_off;
    loc_y = float_y + y1 - y_off;
    p0.x = loc_x * zoom_level;
    p0.y = loc_y * zoom_level;
    Circle(p0.x, p0.y, not_radius, op, ds);
}

void DrawIntVector(int x0, int y0, int x1, int y1, int op, DrawingSurfacePun ds) {
    XDrawLine(ds->display, ds->drawable, ds->gc, x0, y0, x1, y1);
}


void DrawFloatVector(float x0, float y0, float x1, float y1, int op, DrawingSurfacePun ds) {

    if (!PSflag) {
        SetFunction(op, ds);
        XDrawLine(ds->display, ds->drawable, ds->gc, (int) x0, (int)y0, (int)x1, (int) y1);
    }
    else {
        PSline(x0, y0, x1, y1);
    }
}

void DrawSeg(struct arc_object *arc) {
    struct coordinate *point;

    for (point = arc->point; point->next != NULL; point = point->next)
        InvertLine(point->x, point->y, point->next->x, point->next->y);
}

void InvertLine(float x0, float y0, float x1, float y1) {
    if (!PSflag) {
        SetFunction(XOR, &mainDS);
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, (int)(x0 * zoom_level), (int)(y0 * zoom_level), (int)(x1 * zoom_level), (int)(y1 * zoom_level));
    }
    else {
        puts("InvertLine  !!!! TERIBBILE TERIBBILE !!!!");
    }
}
