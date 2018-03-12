/*
 * drag.c  -  move an object or groups of objects on the screen
 */

/*
#define DEBUG
#define DEBUG_GC
*/

#include "global.h"
#include "chop.h"
#include "draw.h"
#include "mark.h"
#include "res.h"
#include "Canvas.h"
#include "showgdi.h"

/****************************  local variables  *************************/

static float    delta_x, delta_y;
static struct arc_object *head;
static struct images *im_p;


static Display *display;
static Drawable drawable;
static GC       gc = 0;
static XGCValues gc_val;


void cont_dragging(int x, int y) {
    for (im_p = moving_images; im_p != NULL; im_p = im_p->next) {

        int oldx = (int)(cur_x - prev_x + im_p->i_left * zoom_level);
        int oldy = (int)(cur_y - prev_y + im_p->i_top * zoom_level);
        XCopyArea(display, im_p->i_p, drawable, gc,
                  0, 0,
                  (int)(im_p->i_width * zoom_level),
                  (int)(im_p->i_height * zoom_level),
                  oldx, oldy);
        /*	   draw_rectbox((float)oldx,(float)oldy,
              select_width * zoom_level - 1 + (float)oldx, select_height * zoom_level - 1 + (float)oldy,
        	XOR, &mainDS); 	  */

    }
    cur_x = (float) x;
    cur_y = (float) y;
    for (im_p = moving_images; im_p != NULL; im_p = im_p->next) {
        int oldx = (int)(cur_x - prev_x + im_p->i_left * zoom_level);
        int oldy = (int)(cur_y - prev_y + im_p->i_top * zoom_level);
        XCopyArea(display, im_p->i_p, drawable, gc,
                  0, 0,
                  (int)(im_p->i_width * zoom_level),
                  (int)(im_p->i_height * zoom_level),
                  oldx, oldy);
        /*	   draw_rectbox((float)oldx,(float)oldy,
              select_width * zoom_level - 1 + (float)oldx, select_height * zoom_level - 1 + (float)oldy,
        	XOR, &mainDS); */
    }
}

void init_drag_gc() {
    if (gc != 0)
        return;
    display = (Display *) XtDisplay(drawwin);
    drawable = (Drawable) XtWindow(drawwin);
    gc_val.function = gBlack ? GXxor : GXequiv ;

#ifdef DEBUG_GC
    fprintf(stderr, "   ...init_drag_gc:  creating gc\n");
#endif /* DEBUG_GC */

    gc = XCreateGC(display, drawable, GCFunction, &gc_val);
    XSetForeground(display, gc, gBlack ^ gWhite);
    /*	XSetPlaneMask(display,gc,1);*/

}

void drag(int x, int  y) {
    move = drag;
    if (prev_x > -10000.0 || prev_y > -10000.0) {
        cont_dragging(x, y);
    }
    else {
        init_drag_gc();
        prev_x = select_left * zoom_level;
        prev_y = select_top * zoom_level;
        for (im_p = moving_images; im_p != NULL; im_p = im_p->next) {
            int oldx = (int)(cur_x - prev_x + im_p->i_left * zoom_level);
            int oldy = (int)(cur_y - prev_y + im_p->i_top * zoom_level);
            XCopyArea(display, im_p->i_p, drawable, gc,
                      0, 0,
                      (int)(im_p->i_width * zoom_level),
                      (int)(im_p->i_height * zoom_level),
                      oldx, oldy);
        }
    }
}

int trans_moving(struct arc_object *arc) {
    struct trans_object *tt;
    struct group_object *gg;

    for (tt = selected.trans; tt != NULL; tt = tt->next)
        if (tt == arc->trans)
            return (TRUE);
    for (gg = selected.groups; gg != NULL; gg = gg->next)
        for (tt = gg->trans; tt != NULL; tt = tt->next)
            if (tt == arc->trans)
                return (TRUE);
    return (FALSE);
}


void clonate(int x, int y) {
    struct place_object *place;
    struct place_object *temp_pl;
    struct trans_object *trans;
    struct trans_object *temp_tr;
    struct group_object *group;
    struct arc_object *arc;
    struct arc_object *temp_ar;
    struct coordinate *point;
    struct coordinate *temp_po = NULL;
    int             some_add = FALSE;
    int             ii;

    SetModify();
    cur_x = x;
    cur_y = y;
    delta_x = (cur_x - prev_x) / zoom_level;
    delta_y = (cur_y - prev_y) / zoom_level;


    for (im_p = moving_images; im_p != NULL; im_p = im_p->next) {
        int oldx = (int)(cur_x - prev_x + im_p->i_left * zoom_level);
        int oldy = (int)(cur_y - prev_y + im_p->i_top * zoom_level);
        XCopyArea(display, im_p->i_p, drawable, gc,
                  0, 0,
                  (int)(im_p->i_width * zoom_level),
                  (int)(im_p->i_height * zoom_level),
                  oldx, oldy);
    }

    /* copy arcs */

    for (head = NULL, arc = selected.arcs; arc != NULL; arc = arc->next) {
        some_add = TRUE;
        temp_ar = (struct arc_object *) emalloc(ARCOBJ_SIZE);
        temp_ar->next = head;
        head = temp_ar;
        temp_ar->place = arc->place;
        temp_ar->trans = arc->trans;
        temp_ar->type = arc->type;
        temp_ar->mult = arc->mult;
        temp_ar->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
        temp_ar->color = NULL;
        temp_ar->lisp = NULL;
        if (arc->color != NULL) {
            temp_ar->color = emalloc((unsigned)(strlen(arc->color) + 1));
            strcpy(temp_ar->color, arc->color);
            temp_ar->colpos.x = arc->colpos.x;
            temp_ar->colpos.y = arc->colpos.y;
        }
        else if (arc->lisp != NULL) {
            temp_ar->lisp = arc->lisp;
            temp_ar->colpos.x = arc->colpos.x;
            temp_ar->colpos.y = arc->colpos.y;
        }
        temp_ar->point = NULL;
        for (point = arc->point; point != NULL; point = point->next) {
            if (temp_ar->point == NULL) {
                temp_ar->point = temp_po =
                                     (struct coordinate *) emalloc(COORD_SIZE);
            }
            else {
                temp_po = temp_po->next =
                              (struct coordinate *) emalloc(COORD_SIZE);
            }
            temp_po->next = NULL;
            temp_po->x = point->x;
            temp_po->y = point->y;
        }
    }

    /* copy places */

    for (place = selected.places; place != NULL; place = place->next) {
        temp_pl = (struct place_object *) emalloc(PLAOBJ_SIZE);
        temp_pl->tag = emalloc(TAG_SIZE);
        sprintf(temp_pl->tag, "P%d", ++place_num);
        temp_pl->tokens = place->tokens;
        temp_pl->m0 = place->m0;
        temp_pl->mpar = place->mpar;
        temp_pl->cmark = place->cmark;
        temp_pl->distr = NULL;
        temp_pl->center.x = place->center.x + delta_x;
        temp_pl->center.y = place->center.y + delta_y;
        temp_pl->tagpos.x = place->tagpos.x;
        temp_pl->tagpos.y = place->tagpos.y;
        temp_pl->color = NULL;
        temp_pl->lisp = NULL;
        if (place->color != NULL) {
            temp_pl->color = emalloc((unsigned)(strlen(place->color) + 1));
            strcpy(temp_pl->color, place->color);
            temp_pl->colpos.x = place->colpos.x;
            temp_pl->colpos.y = place->colpos.y;
        }
        else if (place->lisp != NULL) {
            temp_pl->lisp = place->lisp;
            temp_pl->colpos.x = place->colpos.x;
            temp_pl->colpos.y = place->colpos.y;
        }
        temp_pl->next = netobj->places;
        temp_pl->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
        netobj->places = temp_pl;
        DrawPlace(temp_pl, SET);
        ShowPlaceMark(temp_pl, SET);
        for (temp_ar = NULL, arc = head; arc != NULL;)
            if (arc->place == place) {
                arc->place = temp_pl;
                if (!trans_moving(arc)) {
                    for (point = arc->point; point != NULL;
                            point = point->next) {
                        if (point->x < 0) {
                            point->x = -point->x;
                            point->y = -point->y;
                        }
                        else {
                            point->x += delta_x;
                            point->y += delta_y;
                        }
                        if (point->next != NULL &&
                                point->next->next == NULL)
                            temp_po = point;
                    }
                    temp_po->next->x = arc->place->center.x;
                    temp_po->next->y = arc->place->center.y;
                    arc->point->x = arc->trans->center.x;
                    arc->point->y = arc->trans->center.y;
                    ChopPlace(&(temp_po->next->x),
                              &(temp_po->next->y), temp_po->x, temp_po->y);
                    ChopTrans(&(arc->point->x), &(arc->point->y),
                              arc->point->next->x, arc->point->next->y,
                              arc->trans->orient);
                    DrawArc(arc, SET);
                    if (temp_ar == NULL) {
                        head = arc->next;
                        arc->next = netobj->arcs;
                        netobj->arcs = arc;
                        arc = head;
                    }
                    else {
                        temp_ar->next = arc->next;
                        arc->next = netobj->arcs;
                        netobj->arcs = arc;
                        arc = temp_ar->next;
                    }
                }
                else {
                    temp_ar = arc;
                    arc = arc->next;
                }
            }
            else {
                temp_ar = arc;
                arc = arc->next;
            }
    }

    /* copy timed transitions */

    for (trans = selected.trans; trans != NULL; trans = trans->next) {
        temp_tr = (struct trans_object *) emalloc(TRNOBJ_SIZE);
        temp_tr->tag = emalloc(TAG_SIZE);
        sprintf(temp_tr->tag, "T%d", ++trans_num);
        temp_tr->orient = trans->orient;
        if ((ii = temp_tr->enabl = trans->enabl) >= 0)
            temp_tr->fire_rate.ff = trans->fire_rate.ff;
        else {
            ii = -ii;
            temp_tr->fire_rate.fp = (float *) emalloc(ii * sizeof(float));
            while (ii--)
                temp_tr->fire_rate.fp[ii] = trans->fire_rate.fp[ii];
        }
        temp_tr->enabled = FALSE;
        temp_tr->kind = trans->kind;
        temp_tr->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
        temp_tr->rpar = trans->rpar;
        if (trans->mark_dep == NULL)
            temp_tr->mark_dep = NULL;
        else {
            /*		for (m_d = trans->mark_dep, temp_co = NULL;
             m_d != NULL; m_d = m_d->next) {
            com = (struct com_object *) emalloc(CMMOBJ_SIZE);
            if (temp_co == NULL)
            	temp_tr->mark_dep = com;
            else
            	temp_co->next = com;
            temp_co = com;
            com->next = NULL;
            com->line = emalloc(LINEMAX);
            strcpy(com->line, m_d->line);
            */
            temp_tr->mark_dep = (char *) emalloc(strlen(trans->mark_dep) + 1);
            strcpy(temp_tr->mark_dep, trans->mark_dep);
        }

        temp_tr->center.x = trans->center.x + delta_x;
        temp_tr->center.y = trans->center.y + delta_y;
        temp_tr->tagpos.x = trans->tagpos.x;
        temp_tr->tagpos.y = trans->tagpos.y;
        temp_tr->color = NULL;
        temp_tr->lisp = NULL;
        if (trans->color != NULL) {
            temp_tr->color = emalloc((unsigned)(strlen(trans->color) + 1));
            strcpy(temp_tr->color, trans->color);
            temp_tr->colpos.x = trans->colpos.x;
            temp_tr->colpos.y = trans->colpos.y;
        }
        else if (trans->lisp != NULL) {
            temp_tr->lisp = trans->lisp;
            temp_tr->colpos.x = trans->colpos.x;
            temp_tr->colpos.y = trans->colpos.y;
        }
        temp_tr->ratepos.x = trans->ratepos.x;
        temp_tr->ratepos.y = trans->ratepos.y;
        temp_tr->next = netobj->trans;
        netobj->trans = temp_tr;
        DrawTrans(temp_tr, SET);
        clonate_drop_arcs(trans, temp_tr);
    }

    /* copy immediate transitions */

    for (group = selected.groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            temp_tr = (struct trans_object *) emalloc(TRNOBJ_SIZE);
            temp_tr->tag = emalloc(TAG_SIZE);
            sprintf(temp_tr->tag, "t%d", ++trans_num);
            temp_tr->orient = trans->orient;
            temp_tr->enabl = trans->enabl;
            temp_tr->enabled = FALSE;
            temp_tr->kind = trans->kind;
            temp_tr->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
            temp_tr->fire_rate.ff = trans->fire_rate.ff;
            temp_tr->rpar = trans->rpar;
            if (trans->mark_dep == NULL)
                temp_tr->mark_dep = NULL;
            else {
                /*
                for (m_d = trans->mark_dep, temp_co = NULL;
                	 m_d != NULL; m_d = m_d->next) {
                	com = (struct com_object *) emalloc(CMMOBJ_SIZE);
                	if (temp_co == NULL)
                	temp_tr->mark_dep = com;
                	else
                	temp_co->next = com;
                	temp_co = com;
                	com->next = NULL;
                	com->line = emalloc(LINEMAX);
                	strcpy(com->line, m_d->line);
                */
                temp_tr->mark_dep = (char *) emalloc(strlen(trans->mark_dep) + 1);
                strcpy(temp_tr->mark_dep, trans->mark_dep);

            }

            temp_tr->center.x = trans->center.x + delta_x;
            temp_tr->center.y = trans->center.y + delta_y;
            temp_tr->tagpos.x = trans->tagpos.x;
            temp_tr->tagpos.y = trans->tagpos.y;
            temp_tr->color = NULL;
            temp_tr->lisp = NULL;
            if (trans->color != NULL) {
                temp_tr->color = emalloc((unsigned)(strlen(trans->color) + 1));
                strcpy(temp_tr->color, trans->color);
                temp_tr->colpos.x = trans->colpos.x;
                temp_tr->colpos.y = trans->colpos.y;
            }
            else if (trans->lisp != NULL) {
                temp_tr->lisp = trans->lisp;
                temp_tr->colpos.x = trans->colpos.x;
                temp_tr->colpos.y = trans->colpos.y;
            }
            temp_tr->ratepos.x = trans->ratepos.x;
            temp_tr->ratepos.y = trans->ratepos.y;
            temp_tr->next = group->movelink->trans;
            group->movelink->trans = temp_tr;
            DrawTrans(temp_tr, SET);
            clonate_drop_arcs(trans, temp_tr);
        }

    if (some_add)
        ClearRes();
    for (im_p = moving_images; im_p != NULL; im_p = im_p->next) {
        int oldx = (int)(cur_x - prev_x + im_p->i_left * zoom_level);
        int oldy = (int)(cur_y - prev_y + im_p->i_top * zoom_level);

        /*gc_val.stipple = (Pixmap) im_p->i_p;
        gc_val.ts_x_origin = oldx;
        gc_val.ts_y_origin = oldy;
        XChangeGC(display, gc, GCTileStipXOrigin | GCTileStipYOrigin |
        							GCStipple, &gc_val );
        XFillRectangle(display, drawable, gc,
        	  oldx, oldy,
        	  (int) (im_p->i_width * zoom_level),
        	  (int) (im_p->i_height * zoom_level) );
        */
        XCopyArea(display, im_p->i_p, drawable, gc,
                  0, 0,
                  (int)(im_p->i_width * zoom_level),
                  (int)(im_p->i_height * zoom_level),
                  oldx, oldy);
    }

}

void clonate_drop_arcs(struct trans_object *oldtrans, struct trans_object *trans) {
    struct arc_object *arc;
    struct arc_object *temp_ar;
    struct coordinate *point;
    struct coordinate *temp_po = NULL;
    int             recompute;

    for (temp_ar = NULL, arc = head; arc != NULL;)
        if (arc->trans == oldtrans) {
            arc->trans = trans;
            recompute = TRUE;
            for (point = arc->point; point != NULL;
                    point = point->next) {
                if (point->x < 0) {
                    point->x = -point->x;
                    point->y = -point->y;
                    recompute = TRUE;
                }
                else {
                    point->x += delta_x;
                    point->y += delta_y;
                }
                if (point->next != NULL &&
                        point->next->next == NULL)
                    temp_po = point;
            }
            if (recompute) {
                temp_po->next->x = arc->place->center.x;
                temp_po->next->y = arc->place->center.y;
                arc->point->x = arc->trans->center.x;
                arc->point->y = arc->trans->center.y;
                ChopPlace(&(temp_po->next->x),
                          &(temp_po->next->y), temp_po->x, temp_po->y);
                ChopTrans(&(arc->point->x), &(arc->point->y),
                          arc->point->next->x, arc->point->next->y,
                          arc->trans->orient);
            }
            DrawArc(arc, SET);
            if (temp_ar == NULL) {
                head = arc->next;
                arc->next = netobj->arcs;
                netobj->arcs = arc;
                arc = head;
            }
            else {
                temp_ar->next = arc->next;
                arc->next = netobj->arcs;
                netobj->arcs = arc;
                arc = temp_ar->next;
            }
        }
        else {
            temp_ar = arc;
            arc = arc->next;
        }
}

void drop(int x, int y) {
    drop_selected(x, y, netobj);
    cur_command = SELECT;
    menu_action();
}

void drop_selected(int x, int y,
                   struct net_object *net) {
    struct place_object *place;
    struct place_object *temp_pl;
    struct arc_object *arc;
    struct arc_object *temp_ar;
    struct coordinate *point;
    struct coordinate *temp_po = NULL;
    struct trans_object *trans;
    struct trans_object *temp_tr = NULL;
    struct mpar_object *mpar;
    struct mpar_object *temp_mp;
    struct rpar_object *rpar;
    struct rpar_object *temp_rp;
    struct res_object *res;
    struct res_object *temp_re;
    struct lisp_object *lisp;
    struct lisp_object *temp_lisp;
    struct group_object *group;
    struct group_object *temp_gr;
    int             recompute;

    cur_x = x;
    cur_y = y;
    for (im_p = moving_images; im_p != NULL; im_p = moving_images) {
        if (im_p->i_p != (Pixmap)NULL) {
            Display *display = (Display *) XtDisplay(frame_w);
            XFreePixmap(display, im_p->i_p);
            im_p->i_p = (Pixmap)NULL;
        }
        moving_images = im_p->next;
        free((char *) im_p);
    }

    if (gc != 0) {
        XFreeGC(display, gc);
        gc = 0;
    }

    fix_x = (cur_x - prev_x) / zoom_level;
    fix_y = (cur_y - prev_y) / zoom_level;

    /* move marking parameters */

    if ((mpar = selected.mpars) != NULL) {
        while (mpar != NULL) {
            mpar->center.x += fix_x;
            mpar->center.y += fix_y;
            temp_mp = mpar;
            mpar = mpar->next;
        }
        temp_mp->next = net->mpars;
        net->mpars = selected.mpars;
    }
    /* move places */

    if ((place = selected.places) != NULL) {
        while (place != NULL) {
            place->center.x += fix_x;
            place->center.y += fix_y;
            temp_pl = place;
            place = place->next;
        }
        temp_pl->next = net->places;
        net->places = selected.places;
    }
    /* move rate parameters */

    if ((rpar = selected.rpars) != NULL) {
        while (rpar != NULL) {
            rpar->center.x += fix_x;
            rpar->center.y += fix_y;
            temp_rp = rpar;
            rpar = rpar->next;
        }
        temp_rp->next = net->rpars;
        net->rpars = selected.rpars;
    }
    /* move timed transitions */

    if ((trans = selected.trans) != NULL) {
        while (trans != NULL) {
            trans->center.x += fix_x;
            trans->center.y += fix_y;
            temp_tr = trans;
            trans = trans->next;
        }
        temp_tr->next = net->trans;
        net->trans = selected.trans;
    }
    /* move immediate transitions */

    group = selected.groups;
    while (group != NULL) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            trans->center.x += fix_x;
            trans->center.y += fix_y;
            temp_tr = trans;
        }
        temp_gr = group->next;
        if (net == netobj) {
            temp_tr->next = group->movelink->trans;
            group->movelink->trans = group->trans;
            free((char *) group);
        }
        else {
            group->next = net->groups;
            net->groups = group;
        }
        group = temp_gr;
    }

    /* move arcs */

    if ((arc = selected.arcs) != NULL) {
        while (arc != NULL) {
            recompute = TRUE;
            for (point = arc->point; point != NULL;
                    point = point->next) {
                if (point->x < 0) {
                    point->x = -point->x;
                    point->y = -point->y;
                    recompute = TRUE;
                }
                else {
                    point->x += fix_x;
                    point->y += fix_y;
                }
                if (point->next != NULL &&
                        point->next->next == NULL)
                    temp_po = point;
            }
            if (recompute) {
                temp_po->next->x = arc->place->center.x;
                temp_po->next->y = arc->place->center.y;
                arc->point->x = arc->trans->center.x;
                arc->point->y = arc->trans->center.y;
                ChopPlace(&(temp_po->next->x),
                          &(temp_po->next->y), temp_po->x, temp_po->y);
                ChopTrans(&(arc->point->x), &(arc->point->y),
                          arc->point->next->x, arc->point->next->y,
                          arc->trans->orient);
            }
            temp_ar = arc;
            arc = arc->next;
        }
        temp_ar->next = net->arcs;
        net->arcs = selected.arcs;
    }
    /* move results */

    if ((res = selected.results) != NULL) {
        while (res != NULL) {
            res->center.x += fix_x;
            res->center.y += fix_y;
            temp_re = res;
            res = res->next;
        }
        temp_re->next = net->results;
        net->results = selected.results;
    }
    /* move color/function definitions */

    if ((lisp = selected.lisps) != NULL) {
        while (lisp != NULL) {
            lisp->center.x += fix_x;
            lisp->center.y += fix_y;
            temp_lisp = lisp;
            lisp = lisp->next;
        }
        temp_lisp->next = net->lisps;
        net->lisps = selected.lisps;
    }
    cancel_selected_net();
    reset_net_object(&selected);
    gdiClearDrawingArea();
    redisplay_canvas();
    prev_x = prev_y = -1.0;
}
