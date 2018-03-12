#define ___MOVE___
#	include "global.h"
#	include "chop.h"
#	include "draw.h"
#	include "mark.h"
#	include "rate.h"
#	include "lisp.h"
#	include "arc.h"
#	include "res.h"
#include "move.h"
#include "grid.h"

static struct net_object allign;
static int      obj_type, o_ty;
static struct arc_object *move_arc_list = NULL;



static void VerticalLineUp(int x, int y) {
    struct net_object *ob_p;
    struct arc_object *arc_p;
    struct coordinate *point;
    float           cur_x, cur_y;

    cur_x = x / zoom_level;
    cur_y = y / zoom_level;
    if ((ob_p = near_obj(cur_x, cur_y, &o_ty)) == NULL
            || o_ty == IO_ARC || o_ty == INH_ARC)
        return;
    move = null_proc;
    left = null_proc;
    middle = null_proc;
    switch (o_ty) {
    case MPAR:
        cur_x = ob_p->mpars->center.x;
        break;
    case PLACE:
        cur_x = ob_p->places->center.x;
        break;
    case RPAR:
        cur_x = ob_p->rpars->center.x;
        break;
    case EXTRANS:
    case DETRANS:
    case IMTRANS:
        cur_x = ob_p->trans->center.x;
        break;
    case RESULT:
        cur_x = ob_p->results->center.x;
        break;
    case LISP:
        cur_x = ob_p->lisps->center.x;
        break;
    default:
        break;
    }
    switch (obj_type) {
    case MPAR:
        ShowMpar(allign.mpars, CLEAR);
        allign.mpars->center.x = cur_x;
        ShowMpar(allign.mpars, OR);
        break;
    case PLACE:
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->place == allign.places)
                DrawArc(arc_p, CLEAR);
        ShowPlaceMark(allign.places, CLEAR);
        DrawPlace(allign.places, CLEAR);
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->place == allign.places) {
                for (point = arc_p->point; point->next->next != NULL;
                        point = point->next);
                point->next->x = cur_x;
                point->next->y = arc_p->place->center.y;
                if (arc_p->point->next->next == NULL) {
                    point->x = arc_p->trans->center.x;
                    point->y = arc_p->trans->center.y;
                    ChopTrans(&(point->x), &(point->y),
                              point->next->x, point->next->y,
                              arc_p->trans->orient);
                }
                ChopPlace(&(point->next->x), &(point->next->y),
                          point->x, point->y);
            }
        allign.places->center.x = cur_x;
        DrawPlace(allign.places, SET);
        ShowPlaceMark(allign.places, SET);
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->place == allign.places)
                DrawArc(arc_p, SET);
        break;
    case RPAR:
        ShowRpar(allign.rpars, CLEAR);
        allign.rpars->center.x = cur_x;
        ShowRpar(allign.rpars, OR);
        break;
    case EXTRANS:
    case DETRANS:
    case IMTRANS:
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->trans == allign.trans)
                DrawArc(arc_p, CLEAR);
        DrawTrans(allign.trans, CLEAR);
        allign.trans->center.x = cur_x;
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->trans == allign.trans) {
                point = arc_p->point;
                point->x = cur_x;
                point->y = arc_p->trans->center.y;
                if (point->next->next == NULL) {
                    point->next->x = arc_p->place->center.x;
                    point->next->y = arc_p->place->center.y;
                    ChopPlace(&(point->next->x), &(point->next->y),
                              point->x, point->y);
                }
                ChopTrans(&(point->x), &(point->y),
                          point->next->x, point->next->y,
                          arc_p->trans->orient);
            }
        DrawTrans(allign.trans, SET);
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->trans == allign.trans)
                DrawArc(arc_p, SET);
        break;
    case RESULT:
        ShowRes(allign.results, CLEAR);
        allign.results->center.x = cur_x;
        ShowRes(allign.results, SET);
        break;
    case LISP:
        ShowLisp(allign.lisps, CLEAR);
        allign.lisps->center.x = cur_x;
        ShowLisp(allign.lisps, SET);
        break;
    default:
        break;
    }
    DisplayGrid();
    reset_canvas_reader();
    menu_action();
}



static void HorizontalLineUp(int x, int y) {
    struct net_object *ob_p;
    struct arc_object *arc_p;
    struct coordinate *point;
    float           cur_x, cur_y;

    cur_x = x / zoom_level;
    cur_y = y / zoom_level;
    if ((ob_p = near_obj(cur_x, cur_y, &o_ty)) == NULL
            || o_ty == IO_ARC || o_ty == INH_ARC)
        return;
    move = null_proc;
    left = null_proc;
    middle = null_proc;
    switch (o_ty) {
    case MPAR:
        cur_y = ob_p->mpars->center.y;
        break;
    case PLACE:
        cur_y = ob_p->places->center.y;
        break;
    case RPAR:
        cur_y = ob_p->rpars->center.y;
        break;
    case EXTRANS:
    case DETRANS:
    case IMTRANS:
        cur_y = ob_p->trans->center.y;
        break;
    case RESULT:
        cur_y = ob_p->results->center.y;
        break;
    case LISP:
        cur_y = ob_p->lisps->center.y;
        break;
    default:
        break;
    }
    switch (obj_type) {
    case MPAR:
        ShowMpar(allign.mpars, CLEAR);
        allign.mpars->center.y = cur_y;
        ShowMpar(allign.mpars, OR);
        break;
    case PLACE:
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->place == allign.places)
                DrawArc(arc_p, CLEAR);
        ShowPlaceMark(allign.places, CLEAR);
        DrawPlace(allign.places, CLEAR);
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->place == allign.places) {
                for (point = arc_p->point; point->next->next != NULL;
                        point = point->next);
                point->next->x = arc_p->place->center.x;
                point->next->y = cur_y;
                if (arc_p->point->next->next == NULL) {
                    point->x = arc_p->trans->center.x;
                    point->y = arc_p->trans->center.y;
                    ChopTrans(&(point->x), &(point->y),
                              point->next->x, point->next->y,
                              arc_p->trans->orient);
                }
                ChopPlace(&(point->next->x), &(point->next->y),
                          point->x, point->y);
            }
        allign.places->center.y = cur_y;
        DrawPlace(allign.places, SET);
        ShowPlaceMark(allign.places, SET);
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->place == allign.places)
                DrawArc(arc_p, SET);
        break;
    case RPAR:
        ShowRpar(allign.rpars, CLEAR);
        allign.rpars->center.y = cur_y;
        ShowRpar(allign.rpars, OR);
        break;
    case EXTRANS:
    case DETRANS:
    case IMTRANS:
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->trans == allign.trans)
                DrawArc(arc_p, CLEAR);
        DrawTrans(allign.trans, CLEAR);
        allign.trans->center.y = cur_y;
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->trans == allign.trans) {
                point = arc_p->point;
                point->x = arc_p->trans->center.x;
                point->y = cur_y;
                if (point->next->next == NULL) {
                    point->next->x = arc_p->place->center.x;
                    point->next->y = arc_p->place->center.y;
                    ChopPlace(&(point->next->x), &(point->next->y),
                              point->x, point->y);
                }
                ChopTrans(&(point->x), &(point->y),
                          point->next->x, point->next->y,
                          arc_p->trans->orient);
            }
        DrawTrans(allign.trans, SET);
        for (arc_p = netobj->arcs; arc_p != NULL; arc_p = arc_p->next)
            if (arc_p->trans == allign.trans)
                DrawArc(arc_p, SET);
        break;
    case RESULT:
        ShowRes(allign.results, CLEAR);
        allign.results->center.y = cur_y;
        ShowRes(allign.results, SET);
        break;
    case LISP:
        ShowLisp(allign.lisps, CLEAR);
        allign.lisps->center.y = cur_y;
        ShowLisp(allign.lisps, SET);
        break;
    default:
        break;
    }
    DisplayGrid();
    reset_canvas_reader();
    menu_action();
}

void LineUp(XButtonEvent *ie) {
    float           fix_x, fix_y;
    struct net_object *n_p;

    /*
    a = event_x(ie) / grill_level;
    b = event_x(ie) % grill_level;
    if (b > grill_level / 2)
    fix_x = (a * grill_level + grill_level) / zoom_level;
    else
    fix_x = (a * grill_level) / zoom_level;
    a = event_y(ie) / grill_level;
    b = event_y(ie) % grill_level;
    if (b > grill_level / 2)
    fix_y = (a * grill_level + grill_level) / zoom_level;
    else
    fix_y = (a * grill_level) / zoom_level;
    */
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((n_p = near_obj(fix_x, fix_y, &obj_type)) == NULL
            || obj_type == IO_ARC || obj_type == INH_ARC)
        return;
    allign.mpars = NULL;
    allign.places = NULL;
    allign.rpars = NULL;
    allign.trans = NULL;
    allign.results = NULL;
    allign.lisps = NULL;
    switch (obj_type) {
    case MPAR:
        allign.mpars = n_p->mpars;
        break;
    case PLACE:
        allign.places = n_p->places;
        break;
    case RPAR:
        allign.rpars = n_p->rpars;
        break;
    case EXTRANS:
    case DETRANS:
    case IMTRANS:
        allign.trans = n_p->trans;
        break;
    case RESULT:
        allign.results = n_p->results;
        break;
    case LISP:
        allign.lisps = n_p->lisps;
        break;
    default:
        break;
    }
    move = null_proc;
    left = VerticalLineUp;
    middle = HorizontalLineUp;
    SetMouseHelp("VERTICAL line-up", "HORIZONTAL line-up", NULL);
    /*
    put_msg(0,"Pick an object with LEFT   button for VERTICAL   line-up");
    next_msg(0,"Pick an object with MIDDLE button for HORIZONTAL line-up");
    */
    set_canvas_tracker();
}


static void ShowMovingPlace(void) {
    PutPlace(cur_place, XOR, &mainDS, 0.0, 0.0);
    for (cur_arc = move_arc_list; cur_arc != NULL; cur_arc = cur_arc->next)
        DrawSeg(cur_arc);
}

static void PlaceTracking(int x, int y) {
    struct coordinate *pp;
    int             a, b;

    ShowMovingPlace();
    a = x / grill_level;
    b = x % grill_level;
    if (b > grill_level / 2)
        fix_x = (a * grill_level + grill_level) / zoom_level;
    else
        fix_x = (a * grill_level) / zoom_level;
    a = y / grill_level;
    b = y % grill_level;
    if (b > grill_level / 2)
        fix_y = (a * grill_level + grill_level) / zoom_level;
    else
        fix_y = (a * grill_level) / zoom_level;
    cur_place->center.x = fix_x;
    cur_place->center.y = fix_y;
    for (cur_arc = move_arc_list; cur_arc != NULL; cur_arc = cur_arc->next) {
        for (pp = cur_arc->point; pp->next != NULL; pp = pp->next);
        pp->x = fix_x;
        pp->y = fix_y;
    }
    ShowMovingPlace();
}

static void PlaceDrop(int x, int y) {
    struct arc_object *a_p = NULL;
    struct coordinate *pp, *ppp = NULL;
    int             a, b;

    ShowMovingPlace();
    a = x / grill_level;
    b = x % grill_level;
    if (b > grill_level / 2)
        fix_x = (a * grill_level + grill_level) / zoom_level;
    else
        fix_x = (a * grill_level) / zoom_level;
    a = y / grill_level;
    b = y % grill_level;
    if (b > grill_level / 2)
        fix_y = (a * grill_level + grill_level) / zoom_level;
    else
        fix_y = (a * grill_level) / zoom_level;
    cur_place->center.x = fix_x;
    cur_place->center.y = fix_y;
    DrawPlace(cur_place, SET);
    for (cur_arc = move_arc_list; cur_arc != NULL; cur_arc = cur_arc->next) {
        a_p = cur_arc;
        for (pp = cur_arc->point; pp->next != NULL;
                ppp = pp, pp = pp->next);
        pp->x = fix_x;
        pp->y = fix_y;
        if (ppp == cur_arc->point) {
            ChopTrans(&(ppp->x), &(ppp->y), pp->x, pp->y,
                      cur_arc->trans->orient);
        }
        ChopPlace(&(pp->x), &(pp->y), ppp->x, ppp->y);
        DrawArc(cur_arc, SET);
    }
    if (a_p != NULL) {
        a_p->next = netobj->arcs;
        netobj->arcs = move_arc_list;
    }
    move_arc_list = NULL;
    selection_on = FALSE;
    DisplayGrid();
    menu_action();
}


static void ShowMovingTrans(void) {
    PutTrans(cur_trans, XOR, &mainDS, 0.0, 0.0);

    for (cur_arc = move_arc_list; cur_arc != NULL; cur_arc = cur_arc->next)
        DrawSeg(cur_arc);

}

static void TransTracking(int x, int y) {
    struct coordinate *pp;
    int             a, b;

    ShowMovingTrans();
    a = x / grill_level;
    b = x % grill_level;
    if (b > grill_level / 2)
        fix_x = (a * grill_level + grill_level) / zoom_level;
    else
        fix_x = (a * grill_level) / zoom_level;
    a = y / grill_level;
    b = y % grill_level;
    if (b > grill_level / 2)
        fix_y = (a * grill_level + grill_level) / zoom_level;
    else
        fix_y = (a * grill_level) / zoom_level;
    cur_trans->center.x = fix_x;
    cur_trans->center.y = fix_y;
    for (cur_arc = move_arc_list; cur_arc != NULL; cur_arc = cur_arc->next) {
        pp = cur_arc->point;
        pp->x = fix_x;
        pp->y = fix_y;
    }
    ShowMovingTrans();
}

static void TransDrop(int x, int y) {
    struct arc_object *a_p = NULL;
    struct coordinate *pp, *ppp;
    int             a, b;

    ShowMovingTrans();
    a = x / grill_level;
    b = x % grill_level;
    if (b > grill_level / 2)
        fix_x = (a * grill_level + grill_level) / zoom_level;
    else
        fix_x = (a * grill_level) / zoom_level;
    a = y / grill_level;
    b = y % grill_level;
    if (b > grill_level / 2)
        fix_y = (a * grill_level + grill_level) / zoom_level;
    else
        fix_y = (a * grill_level) / zoom_level;
    cur_trans->center.x = fix_x;
    cur_trans->center.y = fix_y;
    DrawTrans(cur_trans, SET);
    for (cur_arc = move_arc_list; cur_arc != NULL; cur_arc = cur_arc->next) {
        a_p = cur_arc;
        ppp = cur_arc->point;
        pp = ppp->next;
        ppp->x = fix_x;
        ppp->y = fix_y;
        ChopTrans(&(ppp->x), &(ppp->y), pp->x, pp->y,
                  cur_arc->trans->orient);
        if (pp->next == NULL) {
            ChopPlace(&(pp->x), &(pp->y), ppp->x, ppp->y);
        }
        DrawArc(cur_arc, SET);
    }
    if (a_p != NULL) {
        a_p->next = netobj->arcs;
        netobj->arcs = move_arc_list;
    }
    move_arc_list = NULL;
    selection_on = FALSE;
    DisplayGrid();
    menu_action();
}

void MovePlace(XButtonEvent *ie) {
    struct arc_object *a_p;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL)
        return;
    selection_on = TRUE;
    DrawPlace(cur_place, CLEAR);
    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = a_p) {
        a_p = cur_arc->next;
        if (cur_arc->place == cur_place) {
            DrawArc(cur_arc, CLEAR);
            if (cur_arc->point->next->next == NULL) {
                cur_arc->point->x = cur_arc->trans->center.x;
                cur_arc->point->y = cur_arc->trans->center.y;
            }
            RemoveArc(cur_arc);
            cur_arc->next = move_arc_list;
            move_arc_list = cur_arc;
        }
    }
    ShowMovingPlace();
    SetMouseHelp("DROP the place", NULL, NULL);
    /* put_msg(0,"DROP the place with LEFT mouse"); */
    move = PlaceTracking;
    left = PlaceDrop;
    middle = null_proc;
    set_canvas_tracker();
}

void MoveTrans(XButtonEvent *ie) {
    struct arc_object *a_p;
    struct group_object *group;


    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;

    if ((cur_trans = near_titrans_obj(fix_x, fix_y)) == NULL &&
            (cur_trans = near_imtrans_obj(fix_x, fix_y, &group)) == NULL)
        return;
    selection_on = TRUE;
    DrawTrans(cur_trans, CLEAR);
    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = a_p) {
        a_p = cur_arc->next;
        if (cur_arc->trans == cur_trans) {
            DrawArc(cur_arc, CLEAR);
            if (cur_arc->point->next->next == NULL) {
                cur_arc->point->next->x = cur_arc->place->center.x;
                cur_arc->point->next->y = cur_arc->place->center.y;
            }
            RemoveArc(cur_arc);
            cur_arc->next = move_arc_list;
            move_arc_list = cur_arc;
        }
    }
    ShowMovingTrans();
    SetMouseHelp("DROP the transition", NULL, NULL);
    move = TransTracking;
    left = TransDrop;
    middle = null_proc;
    set_canvas_tracker();
}
