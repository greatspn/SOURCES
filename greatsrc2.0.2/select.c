/*
 * select.c
 */


#define DEBUG


#include "global.h"
#include "menuS.h"
#include "box.h"
#include "chop.h"
#include "draw.h"
#include "mark.h"
#include "lisp.h"
#include "rate.h"
#include "res.h"
#include "select.h"
#include "place.h"
#include "trans.h"
#include "arc.h"
#include "color.h"
#include "menuinit.h"




static struct images *new_i_p;
static int move_dim_x, move_dim_y;

int is_in_move(struct position pos) {

    if (pos.x > selected.nwcorner.x && pos.x < selected.secorner.x
            && pos.y > selected.nwcorner.y && pos.y < selected.secorner.y)
        return (TRUE);
    return (FALSE);
}

void redraw_selected() {
    if (selection_on) {
        PutNet(&selected, &mainDS, (int) 0, (int) 0);
        DrawRectBox(cur_x, cur_y, cur_x + select_width * zoom_level - 1, cur_y + select_height * zoom_level - 1, OR, &mainDS);
    }
}


void end_boxtracking(int x, int y) {
    struct place_object *place;
    struct place_object *temp_pl;
    struct arc_object *arc;
    struct arc_object *temp_ar;
    struct coordinate *point;
    struct trans_object *trans;
    struct trans_object *temp_tr;
    struct mpar_object *mpar;
    struct mpar_object *temp_mp;
    struct rpar_object *rpar;
    struct rpar_object *temp_rp;
    struct res_object *res;
    struct res_object *temp_re;
    struct lisp_object *lisp;
    struct lisp_object *temp_lisp;
    struct group_object *group;
    struct group_object *mv_group;
    struct position p_c;
    float i_x, i_y;

    move = left = middle = null_proc;
    DrawRectBox(fix_x, fix_y, cur_x, cur_y, XOR, &mainDS);
    /*

    for (new_i_p = moving_images; new_i_p != NULL; new_i_p = new_i_p->next)
    {
       xv_scrivo(&mainDS,
    	(int) (new_i_p->i_left * zoom_level),
    	(int) (new_i_p->i_top * zoom_level),
    	(int) (new_i_p->i_width * zoom_level),
    	(int) (new_i_p->i_height * zoom_level),
    	OR, new_i_p->i_p, 0, 0);
    }
    if (prev_x > -10000.0 || prev_y > -10000.0)
       for (new_i_p = moving_images; new_i_p != NULL; new_i_p = new_i_p->next)
       {
     xv_scrivo(&mainDS,
    	   (int) (cur_x - prev_x + new_i_p->i_left * zoom_level),
    	   (int) (cur_y - prev_y + new_i_p->i_top * zoom_level),
    	   (int) (new_i_p->i_width * zoom_level),
    	   (int) (new_i_p->i_height * zoom_level),
    	   OR, new_i_p->i_p, 0, 0);
       }
    */
    if (selection_on) {
        puts("Mark 2\n");
        DrawRectBox(prev_x, prev_y, prev_x + select_width * zoom_level - 1, prev_y + select_height * zoom_level - 1, XOR, &mainDS);
    }

    cur_x = (float) x;
    cur_y = (float) y;
    prev_x = MIN(cur_x, fix_x);
    prev_y = MIN(cur_y, fix_y);
    fix_x = ABS(cur_x - fix_x);
    move_dim_x = 1 + (int) fix_x;
    fix_y = ABS(cur_y - fix_y);
    move_dim_y = 1 + (int) fix_y;
    cur_x = prev_x;
    x = (int) cur_x;
    cur_y = prev_y;
    y = (int) cur_y;
    SetModify();

    /* create net object with all places, arcs and transitions within box */

    i_x = cur_x / zoom_level;
    i_y = cur_y / zoom_level;
    selected.nwcorner.x = i_x;
    selected.nwcorner.y = i_y;
    i_x = (cur_x + fix_x) / zoom_level;
    i_y = (cur_y + fix_y) / zoom_level;
    selected.secorner.x = i_x;
    selected.secorner.y = i_y;


    /* move places and arcs connected with them */

    for (temp_pl = NULL, place = netobj->places;
            place != NULL;)
        if (IsLayerListVisible(place->layer)
                && is_in_move(place->center)) {
            if (temp_pl == NULL)
                netobj->places = place->next;
            else
                temp_pl->next = place->next;
            place->next = selected.places;
            selected.places = place;
            ShowPlaceMark(place, CLEAR);
            DrawPlace(place, CLEAR);
            for (arc = netobj->arcs, temp_ar = NULL; arc != NULL;)
                if (arc->place == place) {
                    if (temp_ar == NULL)
                        netobj->arcs = arc->next;
                    else
                        temp_ar->next = arc->next;
                    arc->next = selected.arcs;
                    selected.arcs = arc;
                    DrawArc(arc, CLEAR);
                    if (!IsLayerListVisible(arc->trans->layer)
                            || !is_in_move(arc->trans->center)) {
                        for (point = arc->point->next; point->next != NULL;
                                point = point->next) {
                            p_c.x = point->x;
                            p_c.y = point->y;
                            if (!is_in_move(p_c)) {
                                point->x = -point->x;
                                point->y = -point->y;
                            }
                        }
                        arc->point->x = -arc->trans->center.x;
                        arc->point->y = -arc->trans->center.y;
                        point->x = place->center.x;
                        point->y = place->center.y;
                    }
                    if (temp_ar == NULL)
                        arc = netobj->arcs;
                    else
                        arc = temp_ar->next;
                }
                else {
                    temp_ar = arc;
                    arc = arc->next;
                }
            if (temp_pl == NULL)
                place = netobj->places;
            else
                place = temp_pl->next;
        }
        else {
            temp_pl = place;
            place = place->next;
        }

    /* move marking parameters */

    for (temp_mp = NULL, mpar = netobj->mpars;
            mpar != NULL;)
        if (IsLayerListVisible(mpar->layer) && is_in_move(mpar->center)) {
            if (temp_mp == NULL)
                netobj->mpars = mpar->next;
            else
                temp_mp->next = mpar->next;
            mpar->next = selected.mpars;
            selected.mpars = mpar;
            /* erase mpar from screen */
            ShowMpar(mpar, CLEAR);
            if (temp_mp == NULL)
                mpar = netobj->mpars;
            else
                mpar = temp_mp->next;
        }
        else {
            temp_mp = mpar;
            mpar = mpar->next;
        }

    /* move rate parameters */

    for (temp_rp = NULL, rpar = netobj->rpars;
            rpar != NULL;)
        if (IsLayerListVisible(rpar->layer) && is_in_move(rpar->center)) {
            if (temp_rp == NULL)
                netobj->rpars = rpar->next;
            else
                temp_rp->next = rpar->next;
            rpar->next = selected.rpars;
            selected.rpars = rpar;
            /* erase rpar from screen */
            ShowRpar(rpar, CLEAR);
            if (temp_rp == NULL)
                rpar = netobj->rpars;
            else
                rpar = temp_rp->next;
        }
        else {
            temp_rp = rpar;
            rpar = rpar->next;
        }

    /* move timed transitions */

    for (temp_tr = NULL, trans = netobj->trans; trans != NULL;)
        if (IsLayerListVisible(trans->layer)
                && is_in_move(trans->center)) {
            if (temp_tr == NULL)
                netobj->trans = trans->next;
            else
                temp_tr->next = trans->next;
            trans->next = selected.trans;
            selected.trans = trans;
            DrawTrans(trans, CLEAR);
            for (arc = netobj->arcs, temp_ar = NULL; arc != NULL;)
                if (IsLayerListVisible(arc->layer)
                        && (arc->trans == trans)) {
                    if (temp_ar == NULL)
                        netobj->arcs = arc->next;
                    else
                        temp_ar->next = arc->next;
                    arc->next = selected.arcs;
                    selected.arcs = arc;
                    DrawArc(arc, CLEAR);
                    for (point = arc->point->next;
                            point->next != NULL;
                            point = point->next) {
                        p_c.x = point->x;
                        p_c.y = point->y;
                        if (!is_in_move(p_c)) {
                            point->x = -point->x;
                            point->y = -point->y;
                        }
                    }
                    point->x = -arc->place->center.x;
                    point->y = -arc->place->center.y;
                    arc->point->x = trans->center.x;
                    arc->point->y = trans->center.y;
                    if (temp_ar == NULL)
                        arc = netobj->arcs;
                    else
                        arc = temp_ar->next;
                }
                else {
                    temp_ar = arc;
                    arc = arc->next;
                }
            if (temp_tr == NULL)
                trans = netobj->trans;
            else
                trans = temp_tr->next;
        }
        else {
            temp_tr = trans;
            trans = trans->next;
        }

    /* move immediate transitions */

    for (group = netobj->groups; group != NULL; group = group->next)
        for (mv_group = NULL, temp_tr = NULL, trans = group->trans;
                trans != NULL;)
            if (IsLayerListVisible(trans->layer)
                    && is_in_move(trans->center)) {
                if (mv_group == NULL) {
                    mv_group = (struct group_object *) emalloc(GRPOBJ_SIZE);
                    mv_group->tag = emalloc(3);
                    sprintf(mv_group->tag, "xx");
                    mv_group->pri = group->pri;
                    mv_group->trans = NULL;
                    mv_group->movelink = group;
                    mv_group->next = selected.groups;
                    selected.groups = mv_group;
                }
                if (temp_tr == NULL)
                    group->trans = trans->next;
                else
                    temp_tr->next = trans->next;
                trans->next = mv_group->trans;
                mv_group->trans = trans;
                DrawTrans(trans, CLEAR);
                for (arc = netobj->arcs, temp_ar = NULL; arc != NULL;)
                    if (IsLayerListVisible(arc->layer)
                            && (arc->trans == trans)) {
                        if (temp_ar == NULL)
                            netobj->arcs = arc->next;
                        else
                            temp_ar->next = arc->next;
                        arc->next = selected.arcs;
                        selected.arcs = arc;
                        DrawArc(arc, CLEAR);
                        for (point = arc->point->next;
                                point->next != NULL;
                                point = point->next) {
                            p_c.x = point->x;
                            p_c.y = point->y;
                            if (!is_in_move(p_c)) {
                                point->x = -point->x;
                                point->y = -point->y;
                            }
                        }
                        point->x = -arc->place->center.x;
                        point->y = -arc->place->center.y;
                        arc->point->x = trans->center.x;
                        arc->point->y = trans->center.y;
                        if (temp_ar == NULL)
                            arc = netobj->arcs;
                        else
                            arc = temp_ar->next;
                    }
                    else {
                        temp_ar = arc;
                        arc = arc->next;
                    }
                if (temp_tr == NULL)
                    trans = group->trans;
                else
                    trans = temp_tr->next;
            }
            else {
                temp_tr = trans;
                trans = trans->next;
            }

    /* move results */

    for (temp_re = NULL, res = netobj->results;
            res != NULL;)
        if (is_in_move(res->center)) {
            if (temp_re == NULL)
                netobj->results = res->next;
            else
                temp_re->next = res->next;
            res->next = selected.results;
            selected.results = res;
            /* erase res from screen */
            ShowRes(res, CLEAR);
            if (temp_re == NULL)
                res = netobj->results;
            else
                res = temp_re->next;
        }
        else {
            temp_re = res;
            res = res->next;
        }


    /* move color/function definitions */

    for (temp_lisp = NULL, lisp = netobj->lisps;
            lisp != NULL;)
        if (is_in_move(lisp->center)) {
            if (temp_lisp == NULL)
                netobj->lisps = lisp->next;
            else
                temp_lisp->next = lisp->next;
            lisp->next = selected.lisps;
            selected.lisps = lisp;
            /* erase lisp from screen */
            ShowLisp(lisp, CLEAR);
            if (temp_lisp == NULL)
                lisp = netobj->lisps;
            else
                lisp = temp_lisp->next;
        }
        else {
            temp_lisp = lisp;
            lisp = lisp->next;
        }

    limit_selected();
    menu_action();
}

void select_objects(XButtonEvent *ie) {
    struct net_object *near;
    int oty;
    float i_x, i_y;
    struct arc_object *arc, *temp_ar;
    struct coordinate *point, *temp_po = NULL;
    struct group_object *group;

    i_x = event_x(ie) / zoom_level;
    i_y = event_y(ie) / zoom_level;
    if ((near = near_obj(i_x, i_y, &oty)) == NULL)
        return;
    /*
    for (new_i_p = moving_images; new_i_p != NULL;
    new_i_p = new_i_p->next)
    {				 xv_scrivo( &mainDS, (int) (new_i_p->i_left *
    			  zoom_level), (int) (new_i_p->i_top *
    			  zoom_level), (int) (new_i_p->i_width *
    			  zoom_level), (int) (new_i_p->i_height *
    			  zoom_level), XOR, new_i_p->i_p, 0, 0);
    }

    if (prev_x > -10000.0 || prev_y > -10000.0)
    {
       puts("Mark 1\n");
       for (new_i_p = moving_images; new_i_p != NULL;
       new_i_p = new_i_p->next)
       {

      xv_scrivo(  &mainDS, (int) (cur_x - prev_x + new_i_p->i_left *
       zoom_level), (int) (cur_y - prev_y + new_i_p->i_top *
       zoom_level), (int) (new_i_p->i_width * zoom_level), (int)
       (new_i_p->i_height * zoom_level), XOR, new_i_p->i_p, 0, 0);

       }
    }
    */
    switch (oty) {
    case MPAR:
        RemoveMpar(near->mpars);
        near->mpars->next = selected.mpars;
        selected.mpars = near->mpars;
        break;
    case PLACE:
        RemovePlace(near->places);
        near->places->next = selected.places;
        selected.places = near->places;
        move_dim_x = move_dim_y = 4 * place_radius / zoom_level + 1;
        for (arc = selected.arcs; arc != NULL; arc = arc->next)
            if (arc->place == near->places) {
                for (point = arc->point; point->next != NULL;
                        point = point->next) {
                    point->x = ABS(point->x);
                    point->y = ABS(point->y);
                    if (point->next->next == NULL)
                        temp_po = point;
                }
                arc->point->x = arc->trans->center.x;
                arc->point->y = arc->trans->center.y;
                point->x = arc->place->center.x;
                point->y = arc->place->center.y;
                ChopTrans(&(arc->point->x), &(arc->point->y),
                          arc->point->next->x, arc->point->next->y,
                          arc->trans->orient);
                ChopPlace(&(point->x), &(point->y), temp_po->x, temp_po->y);
            }
        for (arc = netobj->arcs, temp_ar = NULL;
                arc != NULL;)
            if (arc->place == near->places) {
                DrawArc(arc, CLEAR);
                if (temp_ar == NULL)
                    netobj->arcs = arc->next;
                else
                    temp_ar->next = arc->next;
                arc->next = selected.arcs;
                selected.arcs = arc;
                for (point = arc->point; point->next != NULL;
                        point = point->next) {
                    point->x = -point->x;
                    point->y = -point->y;
                }
                arc->point->x = -arc->trans->center.x;
                arc->point->y = -arc->trans->center.y;
                point->x = arc->place->center.x;
                point->y = arc->place->center.y;
                if (temp_ar == NULL)
                    arc = netobj->arcs;
                else
                    arc = temp_ar->next;
            }
            else {
                temp_ar = arc;
                arc = arc->next;
            }
        break;
    case RPAR:
        RemoveRpar(near->rpars);
        near->rpars->next = selected.rpars;
        selected.rpars = near->rpars;
        break;
    case IMTRANS:
        for (group = selected.groups;
                group != NULL && group->movelink != near->groups;
                group = group->next);
        if (group == NULL) {
            group = (struct group_object *) emalloc(GRPOBJ_SIZE);
            group->tag = emalloc(3);
            sprintf(group->tag, "xx");
            group->trans = NULL;
            group->movelink = near->groups;
            group->pri = near->groups->pri;
            group->next = selected.groups;
            selected.groups = group;
        }
        RemoveTrans(near->trans);
        near->trans->next = group->trans;
        group->trans = near->trans;
    case EXTRANS:
    case DETRANS:
        if (oty != IMTRANS) {
            RemoveTrans(near->trans);
            near->trans->next = selected.trans;
            selected.trans = near->trans;
        }
        move_dim_x = move_dim_y = 2 * trans_length / zoom_level + 1;
        for (arc = selected.arcs; arc != NULL; arc = arc->next)
            if (arc->trans == near->trans) {
                for (point = arc->point; point->next != NULL;
                        point = point->next) {
                    point->x = ABS(point->x);
                    point->y = ABS(point->y);
                    if (point->next->next == NULL)
                        temp_po = point;
                }
                arc->point->x = arc->trans->center.x;
                arc->point->y = arc->trans->center.y;
                point->x = arc->place->center.x;
                point->y = arc->place->center.y;
                ChopTrans(&(arc->point->x), &(arc->point->y),
                          arc->point->next->x, arc->point->next->y,
                          arc->trans->orient);
                ChopPlace(&(point->x), &(point->y), temp_po->x, temp_po->y);
            }
        for (arc = netobj->arcs, temp_ar = NULL;
                arc != NULL;)
            if (arc->trans == near->trans) {
                DrawArc(arc, CLEAR);
                if (temp_ar == NULL)
                    netobj->arcs = arc->next;
                else
                    temp_ar->next = arc->next;
                arc->next = selected.arcs;
                selected.arcs = arc;
                for (point = arc->point; point->next != NULL;
                        point = point->next) {
                    point->x = -point->x;
                    point->y = -point->y;
                }
                arc->point->x = arc->trans->center.x;
                arc->point->y = arc->trans->center.y;
                point->x = -arc->place->center.x;
                point->y = -arc->place->center.y;
                if (temp_ar == NULL)
                    arc = netobj->arcs;
                else
                    arc = temp_ar->next;
            }
            else {
                temp_ar = arc;
                arc = arc->next;
            }
        break;
    case INH_ARC:
    case IO_ARC:
        RemoveArc(near->arcs);
        near->arcs->next = selected.arcs;
        selected.arcs = near->arcs;
        DrawArc(near->arcs, CLEAR);
        break;
    case RESULT:
        RemoveResult(near->results);
        near->results->next = selected.results;
        selected.results = near->results;
        break;
    case LISP:
        RemoveLisp(near->lisps);
        near->lisps->next = selected.lisps;
        selected.lisps = near->lisps;
        break;
    default:
        put_msg(2, "??? Unknown object !!!");
        break;
    }
    switch (oty) {
    case MPAR:
        ShowMpar(near->mpars, CLEAR);
        break;
    case PLACE:
        ShowPlaceMark(near->places, CLEAR);
        DrawPlace(near->places, CLEAR);
        break;
    case RPAR:
        ShowRpar(near->rpars, CLEAR);
        break;
    case IMTRANS:
    case EXTRANS:
    case DETRANS:
        DrawTrans(near->trans, CLEAR);
        break;
    case RESULT:
        ShowRes(near->results, CLEAR);
        break;
    case LISP:
        ShowLisp(near->lisps, CLEAR);
        break;
    default:
        break;
    }
    if (selection_on) {
        puts("Mark 2\n");
        DrawRectBox(cur_x, cur_y, cur_x + select_width * zoom_level - 1, cur_y + select_height * zoom_level - 1, XOR, &mainDS);
    }
    limit_selected();
}

void limit_selected() {
    struct mpar_object *mpar;
    struct place_object *place;
    struct rpar_object *rpar;
    struct trans_object *trans;
    struct group_object *group;
    struct arc_object *arc;
    struct coordinate *point;
    struct res_object *result;
    struct lisp_object *lisp;
    float newx, newy;

#ifdef DEBUG
    fprintf(stderr, "Start limit_selected\n");
#endif

    for (new_i_p = moving_images; new_i_p != NULL; new_i_p = moving_images) {
        if (new_i_p->i_p != (Pixmap)NULL) {
            XFreePixmap(XtDisplay(drawwin), new_i_p->i_p);
            new_i_p->i_p = (Pixmap)NULL;
        }
        moving_images = new_i_p->next;
        free((char *) new_i_p);
    }
    prev_x = prev_y = -10000.0;
    select_top = canvas_height;
    select_left = canvas_width;
    select_width = 0;
    select_height = 0;
    for (mpar = selected.mpars; mpar != NULL; mpar = mpar->next) {
        newx = mpar->center.x;
        newy = mpar->center.y;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }
    for (place = selected.places; place != NULL; place = place->next) {
        newx = place->center.x;
        newy = place->center.y;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }
    for (rpar = selected.rpars; rpar != NULL; rpar = rpar->next) {
        newx = rpar->center.x;
        newy = rpar->center.y;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }
    for (trans = selected.trans; trans != NULL; trans = trans->next) {
        newx = trans->center.x;
        newy = trans->center.y;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }
    for (group = selected.groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            newx = trans->center.x;
            newy = trans->center.y;
            if (newx < select_left)
                select_left = newx;
            if (newx > select_width)
                select_width = newx;
            if (newy < select_top)
                select_top = newy;
            if (newy > select_height)
                select_height = newy;
        }
    for (arc = selected.arcs; arc != NULL; arc = arc->next)
        for (point = arc->point; point != NULL; point = point->next)
            if ((newx = point->x) >= 0.0 && (newy = point->y) >= 0.0) {
                if (newx < select_left)
                    select_left = newx;
                if (newx > select_width)
                    select_width = newx;
                if (newy < select_top)
                    select_top = newy;
                if (newy > select_height)
                    select_height = newy;
            }
    for (result = selected.results; result != NULL; result = result->next) {
        newx = result->center.x;
        newy = result->center.y;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }
    for (lisp = selected.lisps; lisp != NULL; lisp = lisp->next) {
        newx = lisp->center.x;
        newy = lisp->center.y;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }
    select_left -= trans_length;
    select_top -= trans_length;
    select_width += trans_length;
    select_height += trans_length;
    select_width -= select_left;
    select_height -= select_top;
    if (select_width < 0 || select_height < 0) {
        cancel_selected_net();
        return;
    }
    recompute_images();

#ifdef DEBUG
    fprintf(stderr, "End limit_selected\n");
#endif
}


#define SELWINDOW_TOP -OVERVIEW_HEIGHT
#define SELWINDOW_LEFT -10
#define SELWINDOW_WIDTH 430
#define SELWINDOW_HEIGHT 270

DrawingSurfacePun selected_pw = NULL;

//static void  repaint_selectedCB(Widget w,
//				XtPointer client_data,
//				XtPointer call_data)
//{
//   DrawingSurfacePun DSp = (DrawingSurfacePun)client_data;
//
//   SetFunction(OR,DSp);
//
//   puts("repaint_selectedCB ...\n");
//   DrawNet(&selected,OR);
//   /* w_draw_net(&selected, DSp, (int) select_left, (int) select_top); */
//   DrawRectBox(0.0, 0.0,
//	      select_width * zoom_level - 1, select_height * zoom_level - 1,
//		COPY, DSp);
//   /* draw_rectbox(0.0, 0.0,
//	      select_width * zoom_level - 1, select_height * zoom_level - 1,
//		COPY, DSp);  */
//
//}
//

void display_selected_net() {


#ifdef DEBUG
    fprintf(stderr, "  start display_selected_net\n");
#endif
    /*
    if (selected_pw != NULL)
        quit_popup(selected_pw);

    selected_pw = new_popup(SELWINDOW_LEFT, SELWINDOW_TOP,
    		   (int) (select_width * zoom_level),
    		   (int) (select_height * zoom_level),
    		   SELWINDOW_WIDTH, SELWINDOW_HEIGHT,
    		   repaint_selectedCB, "GreatSPN: Selected Subnet");

    XSetPlaneMask(selected_pw->display,selected_pw->gc,1);
    */

    /* NO !!!!
    repaint_selectedCB(NULL,NULL,NULL);
    */
    /*popup_modify();     NO!!!*/

    UpdateMenuStatusTo(SELECTION_MSTATUS);
    UpdatePopupStatusTo(11);
#ifdef DEBUG
    fprintf(stderr, "  end display_selected_net\n");
#endif

}

void cancel_selected_net() {

    if (selected_pw != NULL)
        quit_popup(selected_pw);
    selected_pw = NULL;
    UpdateMenuStatusTo(NORMAL_MSTATUS);
    UpdatePopupStatusTo(0);
    selection_on = FALSE;

}


void my_get_pixmap(Pixmap         *pxpp,
                   DrawingSurfacePun ds,
                   int             xc, int yc, int w, int h) {
    DrawingSurface pxDS;

    pxDS = *ds;
    if (*pxpp != (Pixmap)NULL)
        XFreePixmap(ds->display, *pxpp);

    *pxpp = XCreatePixmap(ds->display, ds->drawable, w, h, gDepth);
    pxDS.drawable = *pxpp;

    SetFunction(COPY, &pxDS);
    XSetForeground(pxDS.display, pxDS.gc, gWhite);
    XFillRectangle(pxDS.display, *pxpp, pxDS.gc, 0, 0, w, h);
    XSetForeground(pxDS.display, pxDS.gc, gBlack);
    PutNet(&selected, &pxDS, (int) select_left, (int) select_top);
    /*draw_rectbox(0.0, 0.0,
          select_width * zoom_level - 1, select_height * zoom_level - 1,
    	OR, &pxDS); */
}

void recompute_images() {
    int x, y;

#ifdef DEBUG
    fprintf(stderr, "  Start recompute_images\n");
#endif

    moving_images = (struct images *) emalloc(sizeof(struct images));
    moving_images->next = NULL;
    moving_images->i_left = select_left;
    moving_images->i_top = select_top;
    moving_images->i_width = select_width;
    moving_images->i_height = select_height;
    moving_images->i_p = (Pixmap)NULL;
    display_selected_net();


    printf("recompute_images  select_width = %f   select_height=%f\n", select_width, select_height);
    x = select_width * zoom_level;
    y = select_height * zoom_level;

    my_get_pixmap(&(moving_images->i_p), &mainDS, 0, 0, x, y);

    cur_x = select_left * zoom_level;
    cur_y = select_top * zoom_level;


    /*   for (new_i_p = moving_images; new_i_p != NULL;
    new_i_p = new_i_p->next)
    {
       xv_scrivo(&mainDS,
    	(int) (new_i_p->i_left * zoom_level),
    	(int) (new_i_p->i_top * zoom_level),
    	(int) (new_i_p->i_width * zoom_level),
    	(int) (new_i_p->i_height * zoom_level),
    	OR, new_i_p->i_p, 0, 0);
    }
    */
    DrawRectBox(cur_x, cur_y, cur_x + select_width * zoom_level - 1, cur_y + select_height * zoom_level - 1, OR, &mainDS);

    PutNet(&selected, &mainDS, (int) 0, (int) 0);
    selection_on = TRUE;

#ifdef DEBUG
    fprintf(stderr, "  End recompute_images\n");
#endif
}

void zoom_selected_images() {
    struct images *im_p;

    if (selection_on) {
        puts("Mark 2\n");
        DrawRectBox(cur_x, cur_y, cur_x + select_width * zoom_level - 1, cur_y + select_height * zoom_level - 1, OR, &mainDS);
    }

    select_left = canvas_width;
    select_top = canvas_height;
    select_width = select_height = 0;
    for (im_p = moving_images; im_p != NULL; im_p = moving_images) {
        if (select_left > im_p->i_left)
            select_left = im_p->i_left;
        if (select_top > im_p->i_top)
            select_top = im_p->i_top;
        if (select_width < (im_p->i_left + im_p->i_width))
            select_width = (im_p->i_left + im_p->i_width);
        if (select_height < (im_p->i_top + im_p->i_height))
            select_height = (im_p->i_top + im_p->i_height);
        if (im_p->i_p != (Pixmap)NULL)
            XFreePixmap(XtDisplay(drawwin), im_p->i_p);
        im_p->i_p = (Pixmap)NULL;
        moving_images = im_p->next;
        free((char *) im_p);
    }
    select_width -= select_left;
    select_height -= select_top;
    recompute_images();
}
