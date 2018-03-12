/*
 * mirror.c
 */

/*
#define DEBUG
*/

#include "global.h"
#include "chop.h"
#include "draw.h"
#	include "res.h"
#include "mark.h"

/****************************  local variables  *************************/
static float    m_center;
static int      mirror_vert;
static struct arc_object *head;
#undef event_x
#undef event_y
#define event_x(e) (e->x)
#define event_y(e) (e->y)




void mirror_horizontal(XButtonEvent          *ie) {
    mirror_vert = FALSE;
    m_center = 2.0 * event_y(ie) / zoom_level;
    mirror_drop();
}

void mirror_vertical(XButtonEvent          *ie) {
    mirror_vert = TRUE;
    m_center = 2.0 * event_x(ie) / zoom_level;
    mirror_drop();
}

void mirror_drop() {
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
    struct images  *im_p;
    int             ii;

#ifdef DEBUG
    fprintf(stderr, "\nStart of mirror_drop\n");
#endif
    SetModify();
    for (im_p = moving_images; im_p != NULL; im_p = im_p->next) {
        if (prev_x > -10000.0 || prev_y > -10000.0) {
            xv_scrivo(&mainDS,
                      (int)(cur_x - prev_x + im_p->i_left * zoom_level),
                      (int)(cur_y - prev_y + im_p->i_top * zoom_level),
                      (int)(im_p->i_width * zoom_level),
                      (int)(im_p->i_height * zoom_level),
                      XOR, im_p->i_p, 0, 0);
        }
    }
    prev_x = prev_y = -10000.0;

    /* copy arcs */

#ifdef DEBUG
    fprintf(stderr, "  copy arcs\n");
#endif
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

#ifdef DEBUG
    fprintf(stderr, "  copy places\n");
#endif
    for (place = selected.places; place != NULL; place = place->next) {
        temp_pl = (struct place_object *) emalloc(PLAOBJ_SIZE);
#ifdef DEBUG
        fprintf(stderr, "    emalloc\n");
#endif
        temp_pl->tag = emalloc(TAG_SIZE);
        sprintf(temp_pl->tag, "P%d", ++place_num);
        temp_pl->tokens = place->tokens;
        temp_pl->layer =  NewLayerList(CURRENTVIEWANDWHOLE, NULL);
        temp_pl->m0 = place->m0;
        temp_pl->mpar = place->mpar;
        temp_pl->cmark = place->cmark;
        temp_pl->distr = NULL;
        if (mirror_vert) {
            temp_pl->center.x = m_center - place->center.x;
            temp_pl->center.y = place->center.y;
        }
        else {
            temp_pl->center.x = place->center.x;
            temp_pl->center.y = m_center - place->center.y;
        }
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
        netobj->places = temp_pl;
#ifdef DEBUG
        fprintf(stderr, "      drawing\n");
#endif
        DrawPlace(temp_pl, OR);
        ShowPlaceMark(temp_pl, OR);
#ifdef DEBUG
        fprintf(stderr, "      arcs\n");
#endif
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
                            if (mirror_vert) {
                                point->x = m_center - point->x;
                                point->y = point->y;
                            }
                            else {
                                point->x = point->x;
                                point->y = m_center - point->y;
                            }
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
                    DrawArc(arc, OR);
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

#ifdef DEBUG
    fprintf(stderr, "  copy timed transitions\n");
#endif
    for (trans = selected.trans; trans != NULL; trans = trans->next) {
        temp_tr = (struct trans_object *) emalloc(TRNOBJ_SIZE);
        temp_tr->tag = emalloc(TAG_SIZE);
        sprintf(temp_tr->tag, "T%d", ++trans_num);
        temp_tr->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
        temp_tr->enabled = FALSE;
        temp_tr->kind = trans->kind;

        if ((ii = temp_tr->enabl = trans->enabl) >= 0)
            temp_tr->fire_rate.ff = trans->fire_rate.ff;
        else {
            ii = -ii;
            temp_tr->fire_rate.fp = (float *) emalloc(ii * sizeof(float));
            while (ii--)
                temp_tr->fire_rate.fp[ii] = trans->fire_rate.fp[ii];
        }

        temp_tr->rpar = trans->rpar;
        if (trans->mark_dep == NULL)
            temp_tr->mark_dep = NULL;
        else {
            /*
             for (m_d = trans->mark_dep, temp_co = NULL;
             m_d != NULL; m_d = m_d->next)
             {
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

        if (mirror_vert) {
            temp_tr->orient = rot_trans((int)(trans->orient),
                                        FLIP_XAXIS);
            temp_tr->center.x = m_center - trans->center.x;
            temp_tr->center.y = trans->center.y;
        }
        else {
            temp_tr->orient = rot_trans((int)(trans->orient),
                                        FLIP_YAXIS);
            temp_tr->center.x = trans->center.x;
            temp_tr->center.y = m_center - trans->center.y;
        }
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
        DrawTrans(temp_tr, OR);
        mirror_drop_arcs(trans, temp_tr);
    }

    /* copy immediate transitions */

#ifdef DEBUG
    fprintf(stderr, "  copy immediate transitions\n");
#endif
    for (group = selected.groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            temp_tr = (struct trans_object *) emalloc(TRNOBJ_SIZE);
            temp_tr->tag = emalloc(TAG_SIZE);
            sprintf(temp_tr->tag, "t%d", ++trans_num);
            temp_tr->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
            temp_tr->orient = trans->orient;
            temp_tr->enabl = trans->enabl;
            temp_tr->enabled = FALSE;
            temp_tr->kind = trans->kind;
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

            if (mirror_vert) {
                temp_tr->orient = rot_trans((int)(trans->orient),
                                            FLIP_XAXIS);
                temp_tr->center.x = m_center - trans->center.x;
                temp_tr->center.y = trans->center.y;
            }
            else {
                temp_tr->orient = rot_trans((int)(trans->orient),
                                            FLIP_YAXIS);
                temp_tr->center.x = trans->center.x;
                temp_tr->center.y = m_center - trans->center.y;
            }
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
            DrawTrans(temp_tr, OR);
            mirror_drop_arcs(trans, temp_tr);
        }

#ifdef DEBUG
    fprintf(stderr, "  clear results\n");
#endif
    if (some_add)
        ClearRes();
#ifdef DEBUG
    fprintf(stderr, "End of mirror_drop\n");
#endif
}

void mirror_drop_arcs(struct trans_object *oldtrans, struct trans_object  *trans) {
    struct arc_object *arc;
    struct arc_object *temp_ar;
    struct coordinate *point;
    struct coordinate *temp_po = NULL;
    int             recompute;

#ifdef DEBUG
    fprintf(stderr, "Start of mirror_drop_arcs\n");
#endif
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
                    if (mirror_vert) {
                        point->x = m_center - point->x;
                        point->y = point->y;
                    }
                    else {
                        point->x = point->x;
                        point->y = m_center - point->y;
                    }
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
            DrawArc(arc, OR);
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
#ifdef DEBUG
    fprintf(stderr, "End of mirror_drop_arcs\n");
#endif
}
