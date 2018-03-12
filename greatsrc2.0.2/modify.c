/*
 * modify.c
 */

#include "global.h"
#include "box.h"
#include "showgdi.h"
#include "draw.h"

/****************************  local variables  *************************/
static double   center_x, center_y;


void transform_coord(int             how,
                     float           oldx, float oldy,
                     float          *newx, float *newy) {
    switch (how) {
    case FLIP_XAXIS:
        *newx = center_x + center_x - oldx;
        *newy = oldy;
        break;
    case FLIP_YAXIS:
        *newx = oldx;
        *newy = center_y + center_y - oldy;
        break;
    case ROTATE45:
        *newx = (oldx - center_x - oldy + center_y) * ISQRT2 + center_x;
        *newy = (oldy - center_y + oldx - center_x) * ISQRT2 + center_y;
        break;
    case ROTATE90:
        *newx = -oldy + center_y + center_x;
        *newy = oldx - center_x + center_y;
        break;
    case ROTATE135:
        *newx = (-oldx + center_x - oldy + center_y) * ISQRT2 + center_x;
        *newy = (-oldy + center_y + oldx - center_x) * ISQRT2 + center_y;
        break;
    case ROTATE180:
        *newx = center_x + center_x - oldx;
        *newy = center_y + center_y - oldy;
        break;
    case ROTATEM135:
        *newx = (-oldx + center_x + oldy - center_y) * ISQRT2 + center_x;
        *newy = (-oldy + center_y - oldx + center_x) * ISQRT2 + center_y;
        break;
    case ROTATEM90:
        *newx = oldy - center_y + center_x;
        *newy = -oldx + center_x + center_y;
        break;
    case ROTATEM45:
        *newx = (oldx - center_x + oldy - center_y) * ISQRT2 + center_x;
        *newy = (oldy - center_y - oldx + center_x) * ISQRT2 + center_y;
        break;
    default:
        *newx = oldx;
        *newy = oldy;
        break;
    }
}

void modify_selected(int             how) {
    struct place_object *place;
    struct arc_object *arc;
    struct coordinate *point;
    struct trans_object *trans;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct res_object *res;
    struct lisp_object *lisp;
    struct group_object *group;
    float           newx, newy;

    DrawRectBox(cur_x, cur_y, cur_x + select_width * zoom_level - 1, cur_y + select_height * zoom_level - 1, XOR, &mainDS);

    SetModify();
    center_x = select_left + (double) select_width * 0.5;
    center_y = select_top + (double) select_height * 0.5;
    select_top = canvas_height;
    select_left = canvas_width;
    select_width = 0;
    select_height = 0;

    /*		for (im_p = moving_images; im_p != NULL; im_p = moving_images)
    		{
    	xv_scrivo( &mainDS,
    	(int)(im_p->i_left*zoom_level), (int)(im_p->i_top*zoom_level),
    	(int)(im_p->i_width*zoom_level), (int)(im_p->i_height*zoom_level),
    	XOR, im_p->i_p, 0, 0);
    	if (prev_x > -10000.0 || prev_y > -10000.0)
    		{
    		xv_scrivo( &mainDS,
    	(int)(cur_x-prev_x+im_p->i_left*zoom_level),
    			(int)(cur_y-prev_y+im_p->i_top*zoom_level),
    	(int)(im_p->i_width*zoom_level),
    		(int)(im_p->i_height*zoom_level),
    			XOR, im_p->i_p, 0, 0);
    	}
    		if ( im_p->i_p != (Pixmap)NULL) {
    			XFreePixmap(mainDS.display, im_p->i_p);
    			im_p->i_p = (Pixmap)NULL;
    		}
    		moving_images = im_p->next;
    		free((char *) im_p);
    	}
    */
    prev_x = prev_y = -10000.0;
    gdiClearDrawingArea();
    RedisplayNet();
    /* move marking parameters */

    for (mpar = selected.mpars; mpar != NULL; mpar = mpar->next) {
        transform_coord(how, mpar->center.x, mpar->center.y, &newx, &newy);
        mpar->center.x = newx;
        mpar->center.y = newy;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }

    /* move places */

    for (place = selected.places; place != NULL; place = place->next) {
        transform_coord(how, place->center.x, place->center.y, &newx, &newy);
        place->center.x = newx;
        place->center.y = newy;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }

    /* move rate parameters */

    for (rpar = selected.rpars; rpar != NULL; rpar = rpar->next) {
        transform_coord(how, rpar->center.x, rpar->center.y, &newx, &newy);
        rpar->center.x = newx;
        rpar->center.y = newy;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }

    /* move timed transitions */

    for (trans = selected.trans; trans != NULL; trans = trans->next)
        modify_trans(how, trans);

    /* move immediate transitions */

    for (group = selected.groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next)
            modify_trans(how, trans);

    /* move arcs */

    for (arc = selected.arcs; arc != NULL; arc = arc->next)
        for (point = arc->point; point != NULL; point = point->next)
            if (point->x >= 0.0 && point->y >= 0.0) {
                transform_coord(how, point->x, point->y, &newx, &newy);
                point->x = newx;
                point->y = newy;
                if (newx < select_left)
                    select_left = newx;
                if (newx > select_width)
                    select_width = newx;
                if (newy < select_top)
                    select_top = newy;
                if (newy > select_height)
                    select_height = newy;
            }
    /* move results */

    for (res = selected.results; res != NULL; res = res->next) {
        transform_coord(how, res->center.x, res->center.y, &newx, &newy);
        res->center.x = newx;
        res->center.y = newy;
        if (newx < select_left)
            select_left = newx;
        if (newx > select_width)
            select_width = newx;
        if (newy < select_top)
            select_top = newy;
        if (newy > select_height)
            select_height = newy;
    }

    /* move color/function definitions */

    for (lisp = selected.lisps; lisp != NULL; lisp = lisp->next) {
        transform_coord(how, lisp->center.x, lisp->center.y, &newx, &newy);
        lisp->center.x = newx;
        lisp->center.y = newy;
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
    recompute_images(1);
}

void modify_trans(int how, struct trans_object *trans) {
    float           newx, newy;

    trans->orient = rot_trans((int)(trans->orient), how);
    transform_coord(how, trans->center.x, trans->center.y, &newx, &newy);
    trans->center.x = newx;
    trans->center.y = newy;
    if (newx < select_left)
        select_left = newx;
    if (newx > select_width)
        select_width = newx;
    if (newy < select_top)
        select_top = newy;
    if (newy > select_height)
        select_height = newy;
}

int rot_trans(int             cur_orient, int how_much) {
    switch (how_much) {
    case ROTATE45:
    case ROTATEM135:
        switch (cur_orient) {
        case HORIZONTAL:
            cur_orient = FORTYFIVE;
            break;
        case FORTYFIVE:
            cur_orient = VERTICAL;
            break;
        case VERTICAL:
            cur_orient = MINUSFORTYFIVE;
            break;
        default:
            cur_orient = HORIZONTAL;
            break;
        }
        break;
    case ROTATE90:
    case ROTATEM90:
        switch (cur_orient) {
        case HORIZONTAL:
            cur_orient = VERTICAL;
            break;
        case FORTYFIVE:
            cur_orient = MINUSFORTYFIVE;
            break;
        case VERTICAL:
            cur_orient = HORIZONTAL;
            break;
        default:
            cur_orient = FORTYFIVE;
            break;
        }
        break;
    case ROTATE135:
    case ROTATEM45:
        switch (cur_orient) {
        case HORIZONTAL:
            cur_orient = MINUSFORTYFIVE;
            break;
        case FORTYFIVE:
            cur_orient = HORIZONTAL;
            break;
        case VERTICAL:
            cur_orient = FORTYFIVE;
            break;
        default:
            cur_orient = VERTICAL;
            break;
        }
        break;
    case FLIP_XAXIS:
    case FLIP_YAXIS:
        switch (cur_orient) {
        case FORTYFIVE:
            cur_orient = MINUSFORTYFIVE;
            break;
        case MINUSFORTYFIVE:
            cur_orient = FORTYFIVE;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return (cur_orient);
}
