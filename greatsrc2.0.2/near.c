/*
 * near.c
 *
 * routines for locating objects near a point
 *
 * near_obj will find any object near a point
 *
 * near_xxx_obj  will find an specific xxx object near a point
 *
 * all return NULL if nothing within tolerance distance
 */

#include "global.h"


struct mpar_object *near_mpar_obj(float x_loc, float y_loc) {
    struct mpar_object *near_mpar;

    for (near_mpar = netobj->mpars; near_mpar != NULL;
            near_mpar = near_mpar->next)
        if (IsLayerListVisible(near_mpar->layer))
            if (x_loc < near_mpar->center.x + 2 * place_radius &&
                    y_loc <= near_mpar->center.y &&
                    x_loc >= near_mpar->center.x &&
                    y_loc > near_mpar->center.y - 2 * place_radius)
                return (near_mpar);
    return (NULL);
}

struct place_object *near_place_obj(float  x_loc, float  y_loc) {
    struct place_object *near_place;

    for (near_place = netobj->places; near_place != NULL;
            near_place = near_place->next)
        if (IsLayerListVisible(near_place->layer))
            if (x_loc < near_place->center.x + 2 * place_radius &&
                    y_loc < near_place->center.y + 2 * place_radius &&
                    x_loc > near_place->center.x - 2 * place_radius &&
                    y_loc > near_place->center.y - 2 * place_radius)
                return (near_place);
    return (NULL);
}

struct place_object *n_to_place(int n_p) {
    struct place_object *pp;

    for (pp = netobj->places; --n_p && (pp != NULL); pp = pp->next);
    return (pp);
}


struct rpar_object *near_rpar_obj(float x_loc, float y_loc) {
    struct rpar_object *near_rpar;

    for (near_rpar = netobj->rpars; near_rpar != NULL;
            near_rpar = near_rpar->next)
        if (IsLayerListVisible(near_rpar->layer))
            if (x_loc < near_rpar->center.x + 2 * place_radius &&
                    y_loc <= near_rpar->center.y + place_radius &&
                    x_loc >= near_rpar->center.x &&
                    y_loc > near_rpar->center.y - 2 * place_radius)
                return (near_rpar);
    return (NULL);
}

struct trans_object *near_titrans_obj(float x_loc, float y_loc) {
    struct trans_object *near_trans;

    for (near_trans = netobj->trans; near_trans != NULL;
            near_trans = near_trans->next)
        if (IsLayerListVisible(near_trans->layer))
            if (x_loc < near_trans->center.x + trans_length &&
                    y_loc < near_trans->center.y + trans_length &&
                    x_loc > near_trans->center.x - trans_length &&
                    y_loc > near_trans->center.y - trans_length)
                return (near_trans);
    return (NULL);
}

struct trans_object *near_imtrans_obj(float x_loc, float y_loc,
                                      struct group_object **group) {
    struct trans_object *near_trans;

    for (*group = netobj->groups; *group != NULL; *group = (*group)->next)
        for (near_trans = (*group)->trans; near_trans != NULL;
                near_trans = near_trans->next)
            if (IsLayerListVisible(near_trans->layer))
                if (x_loc < near_trans->center.x + trans_length &&
                        y_loc < near_trans->center.y + trans_length &&
                        x_loc > near_trans->center.x - trans_length &&
                        y_loc > near_trans->center.y - trans_length)
                    return (near_trans);
    return (NULL);
}


struct trans_object *n_to_trans(int n_t) {
    struct trans_object *tp;
    struct group_object *gp;

    for (tp = netobj->trans; (tp != NULL) && --n_t; tp = tp->next);
    for (gp = netobj->groups; (tp == NULL) && (gp != NULL); gp = gp->next) {
        for (tp = gp->trans; (tp != NULL) && --n_t; tp = tp->next);
    }
    return (tp);
}


int trans_to_n(struct trans_object *tt) {
    int             nn = 1;
    struct trans_object *tp;
    struct group_object *gp;

    if (tt == NULL)
        return (0);
    for (tp = netobj->trans; (tp != NULL); tp = tp->next)
        if (tp == tt)
            return (nn);
        else
            ++nn;
    for (gp = netobj->groups; (tp == NULL) && (gp != NULL); gp = gp->next) {
        for (tp = gp->trans; (tp != NULL); tp = tp->next)
            if (tp == tt)
                return (nn);
            else
                ++nn;
    }
    return (0);
}


struct arc_object *near_arc_obj(float x_loc, float y_loc, char *arc_type) {
    struct arc_object *near_arc;
    struct coordinate *p1, *p2;
    float           x, y;

    for (near_arc = netobj->arcs; near_arc != NULL; near_arc = near_arc->next)
        if (IsLayerListVisible(near_arc->layer))
            for (p1 = near_arc->point; p1->next != NULL; p1 = p2) {
                p2 = p1->next;
                if (close_to_vector(p1->x, p1->y, p2->x, p2->y,
                                    x_loc, y_loc, 3.0, &x, &y)) {
                    *arc_type = near_arc->type;
                    return (near_arc);
                }
            }
    arc_type = NULL;
    return (NULL);
}

struct res_object *near_res_obj(float x_loc, float y_loc) {
    struct res_object *near_res;

    for (near_res = netobj->results; near_res != NULL;
            near_res = near_res->next)
        if (x_loc < near_res->center.x + 2 * place_radius &&
                y_loc <= near_res->center.y &&
                x_loc >= near_res->center.x &&
                y_loc > near_res->center.y - 2 * place_radius)
            return (near_res);
    return (NULL);
}


struct lisp_object *near_lisp_obj(float x_loc, float y_loc) {
    struct lisp_object *near_lisp;

    for (near_lisp = netobj->lisps; near_lisp != NULL;
            near_lisp = near_lisp->next)
        if (x_loc < near_lisp->center.x + 2 * place_radius &&
                y_loc <= near_lisp->center.y &&
                x_loc >= near_lisp->center.x &&
                y_loc > near_lisp->center.y - 2 * place_radius)
            return (near_lisp);
    return (NULL);
}

int close_to_vector(float x1, float  y1, float  x2, float  y2, float  xp, float  yp, float  tol,
                    float  *px, float  *py) {
    /*
     * The returned value in (px, py) is the closest point on the vector to
     * point (xp, yp)
     */

    float           xmin, ymin, xmax, ymax;
    float           dist2;
    float           x, y, slope, D2, dx, dy;
    float           err = 1.0;

    dist2 = tol * tol;
    if (ABS(xp - x1) <= tol && ABS(yp - y1) <= tol) {
        *px = x1;
        *py = y1;
        return (1);
    }
    if (ABS(xp - x2) <= tol && ABS(yp - y2) <= tol) {
        *px = x2;
        *py = y2;
        return (1);
    }
    if (x1 < x2) {
        xmin = x1 - tol;
        xmax = x2 + tol;
    }
    else {
        xmin = x2 - tol;
        xmax = x1 + tol;
    }
    if (xp < xmin || xmax < xp)
        return (0);

    if (y1 < y2) {
        ymin = y1 - tol;
        ymax = y2 + tol;
    }
    else {
        ymin = y2 - tol;
        ymax = y1 + tol;
    }
    if (yp < ymin || ymax < yp)
        return (0);

    if ((ABS(x2 - x1)) <= err) {
        x = x1;
        y = yp;
    }
    else if ((ABS(y1 - y2)) <= err) {
        x = xp;
        y = y1;
    }
    else {
        slope = (x2 - x1) / (y2 - y1);
        y = (slope * (xp - x1 + slope * y1) + yp) / (1 + slope * slope);
        x = x1 + slope * (y - y1);
    }
    dx = xp - x;
    dy = yp - y;
    D2 = dx * dx + dy * dy;
    if (D2 < dist2) {
        *px = x;
        *py = y;
        return (1);
    }
    return (0);
}

struct net_object *near_node_obj(float           x_loc, float y_loc,
                                 int            *object_type) {
    static struct net_object near;

    near.places = NULL;
    near.trans = NULL;
    near.groups = NULL;
    near.arcs = NULL;
    if ((near.places = near_place_obj(x_loc, y_loc)) != NULL) {
        *object_type = PLACE;
        return (&near);
    }
    if ((near.trans = near_titrans_obj(x_loc, y_loc)) != NULL) {
        if (near.trans->kind == EXPONENTIAL)
            *object_type = EXTRANS;
        else
            *object_type = DETRANS;
        return (&near);
    }
    if ((near.trans = near_imtrans_obj(x_loc, y_loc, &(near.groups)))
            != NULL) {
        *object_type = IMTRANS;
        return (&near);
    }
    return (NULL);
}

struct net_object *near_obj(float x_loc, float y_loc,
                            int  *object_type) {
    static struct net_object near;
    char            arctype;

    near.mpars = NULL;
    near.places = NULL;
    near.rpars = NULL;
    near.trans = NULL;
    near.groups = NULL;
    near.arcs = NULL;
    near.results = NULL;
    near.lisps = NULL;
    if ((near.arcs = near_arc_obj(x_loc, y_loc, &arctype)) != NULL) {
        if (near.arcs->type == INHIBITOR)
            *object_type = INH_ARC;
        else
            *object_type = IO_ARC;
        return (&near);
    }
    if ((near.places = near_place_obj(x_loc, y_loc)) != NULL) {
        *object_type = PLACE;
        return (&near);
    }
    if ((near.trans = near_titrans_obj(x_loc, y_loc)) != NULL) {
        if (near.trans->kind == EXPONENTIAL)
            *object_type = EXTRANS;
        else
            *object_type = DETRANS;
        return (&near);
    }
    if ((near.trans = near_imtrans_obj(x_loc, y_loc, &(near.groups)))
            != NULL) {
        *object_type = IMTRANS;
        return (&near);
    }
    if ((near.mpars = near_mpar_obj(x_loc, y_loc)) != NULL) {
        *object_type = MPAR;
        return (&near);
    }
    if ((near.rpars = near_rpar_obj(x_loc, y_loc)) != NULL) {
        *object_type = RPAR;
        return (&near);
    }
    if ((near.results = near_res_obj(x_loc, y_loc)) != NULL) {
        *object_type = RESULT;
        return (&near);
    }
    if ((near.lisps = near_lisp_obj(x_loc, y_loc)) != NULL) {
        *object_type = LISP;
        return (&near);
    }
    return (NULL);
}
