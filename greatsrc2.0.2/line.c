#define ___LINE___
#	include "global.h"
#	include "draw.h"
#	include "arc.h"
#	include "chop.h"
#include "line.h"

static struct coordinate *p1, *p2;

void LineTrack(int x, int y) {

    InvertLine(fix_x, fix_y, cur_x, cur_y);
    cur_x = x / zoom_level;
    cur_y = y / zoom_level;
    InvertLine(fix_x, fix_y, cur_x, cur_y);
}


static void ElasticLine(int x, int y) {

    InvertLine(p1->x, p1->y, cur_x, cur_y);
    InvertLine(p2->x, p2->y, cur_x, cur_y);
    cur_x = x / zoom_level;
    cur_y = y / zoom_level;
    InvertLine(p1->x, p1->y, cur_x, cur_y);
    InvertLine(p2->x, p2->y, cur_x, cur_y);
}

static void AddPoint(int x, int y) {
    struct coordinate *np;

    move = null_proc;
    left = null_proc;
    middle = null_proc;
    InvertLine(p1->x, p1->y, cur_x, cur_y);
    InvertLine(p2->x, p2->y, cur_x, cur_y);
    cur_x = x / zoom_level;
    cur_y = y / zoom_level;
    InvertLine(p1->x, p1->y, cur_x, cur_y);
    InvertLine(p2->x, p2->y, cur_x, cur_y);
    np = (struct coordinate *) emalloc(COORD_SIZE);
    np->next = p2;
    p1->next = np;
    np->x = cur_x;
    np->y = cur_y;
    DrawSeg(cur_arc);
    for (p1 = cur_arc->point; p1->next->next != NULL; p1 = p1->next);
    ChopTrans(&(cur_arc->point->x), &(cur_arc->point->y),
              cur_arc->point->next->x, cur_arc->point->next->y,
              cur_arc->trans->orient);
    ChopPlace(&(p1->next->x), &(p1->next->y), p1->x, p1->y);
    DrawArc(cur_arc, SET);
    reset_canvas_reader();
    menu_action();
}

static void DelPoint(int x, int y) {
    float           d_x, d_y, dist1, dist2;

    move = null_proc;
    left = null_proc;
    middle = null_proc;
    InvertLine(p1->x, p1->y, cur_x, cur_y);
    InvertLine(p2->x, p2->y, cur_x, cur_y);
    InvertLine(p1->x, p1->y, p2->x, p2->y);
    cur_x = x / zoom_level;
    cur_y = y / zoom_level;
    DrawSeg(cur_arc);
    d_x = (float)(p1->x) - (float) cur_x;
    d_y = (float)(p1->y) - (float) cur_y;
    if ((dist1 = d_x * d_x + d_y * d_y) > 0.0)
        dist1 = sqrt(dist1);
    d_x = (float)(p2->x) - (float) cur_x;
    d_y = (float)(p2->y) - (float) cur_y;
    if ((dist2 = d_x * d_x + d_y * d_y) > 0.0)
        dist2 = sqrt(dist2);
    if (dist1 < dist2 || p2->next == NULL) {
        if (p1 != cur_arc->point) {
            for (p2 = cur_arc->point; p2->next != p1; p2 = p2->next);
            p2->next = p1->next;
            free((char *) p1);
        }
        else if (p2->next != NULL) {
            p1->next = p2->next;
            free((char *) p2);
        }
    }
    else {
        p1->next = p2->next;
        free((char *) p2);
    }
    for (p1 = cur_arc->point; p1->next->next != NULL; p1 = p1->next);
    ChopTrans(&(cur_arc->point->x), &(cur_arc->point->y),
              cur_arc->point->next->x, cur_arc->point->next->y,
              cur_arc->trans->orient);
    ChopPlace(&(p1->next->x), &(p1->next->y), p1->x, p1->y);
    DrawArc(cur_arc, SET);
    SetMouseHelp("DELETE arcs", "CHANGE intermediate points", "ACTIONS");
    /*
    put_msg(0,"Delete arcs with left button");
    next_msg(0,"Change intermediate points of arcs with middle button");
    */
    reset_canvas_reader();
    menu_action();
}

void StartLineTracking(XButtonEvent  *ie) {
    struct coordinate *pp;

    action_on = TRUE;
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = cur_arc->next)
        for (p1 = cur_arc->point; p1->next != NULL; p1 = p2) {
            p2 = p1->next;
            if (close_to_vector(p1->x, p1->y, p2->x, p2->y,
                                fix_x, fix_y, 3.0, &cur_x, &cur_y)) {
                goto found;
            }
        }
    action_on = FALSE;
    return;

found:
    DrawArc(cur_arc, CLEAR);
    for (pp = cur_arc->point->next; pp->next != NULL; pp = pp->next);
    cur_arc->point->x = cur_arc->trans->center.x;
    cur_arc->point->y = cur_arc->trans->center.y;
    pp->x = cur_arc->place->center.x;
    pp->y = cur_arc->place->center.y;
    DrawSeg(cur_arc);
    InvertLine(p1->x, p1->y, p2->x, p2->y);
    cur_x = fix_x;
    cur_y = fix_y;
    InvertLine(p1->x, p1->y, cur_x, cur_y);
    InvertLine(p2->x, p2->y, cur_x, cur_y);
    SetMouseHelp("ADD intermediate point", "DELETE intermediate point", NULL);
    /*
    put_msg(0,"ADD    intermediate point with LEFT button");
    next_msg(0,"DELETE intermediate point with MIDDLE button");
    */
    move = ElasticLine;
    left = AddPoint;
    middle = DelPoint;
    set_canvas_tracker();
}

void StartMoveLine(XButtonEvent *ie) {
    float           d_x, d_y, dist1, dist2;

    action_on = TRUE;
    cur_x = event_x(ie) / zoom_level;
    cur_y = event_y(ie) / zoom_level;
    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = cur_arc->next)
        for (p1 = cur_arc->point; p1->next != NULL; p1 = p2) {
            p2 = p1->next;
            if (close_to_vector(p1->x, p1->y, p2->x, p2->y,
                                cur_x, cur_y, 3.0, &fix_x, &fix_y)) {
                goto found;
            }
        }
    action_on = FALSE;
    return;

found:
    if (p1 != cur_arc->point && p2->next != NULL) {
        /* INTERMEDIATE POINT  */
        action_on = FALSE;
        return;
    }
    DrawArc(cur_arc, CLEAR);
    if (p1 == cur_arc->point) {
        if (p2->next == NULL) {
            /* NO INTERMEDIATE POINTS  */
            d_x = p1->x - fix_x;
            d_y = p1->y - fix_y;
            if ((dist1 = d_x * d_x + d_y * d_y) > 0.0)
                dist1 = (float) sqrt(dist1);
            d_x = p2->x - fix_x;
            d_y = p2->y - fix_y;
            if ((dist2 = d_x * d_x + d_y * d_y) > 0.0)
                dist2 = (float) sqrt(dist2);
            if (dist1 < dist2) {
                /* CHANGE TRANSITION  */
                fix_x = p2->x = cur_arc->place->center.x;
                fix_y = p2->y = cur_arc->place->center.y;
                cur_arc->trans = NULL;
                cur_arc->point = p2;
                free((char *) p1);
            }
            else {
                /* CHANGE PLACE  */
                fix_x = p1->x = cur_arc->trans->center.x;
                fix_y = p1->y = cur_arc->trans->center.y;
                cur_arc->place = NULL;
                p1->next = NULL;
                free((char *) p2);
            }
        }
        else {
            /* CHANGE TRANSITION  */
            cur_arc->trans = NULL;
            cur_arc->point = p2;
            free((char *) p1);
            fix_x = p2->x;
            fix_y = p2->y;
        }
    }
    else {
        /* CHANGE PLACE  */
        cur_arc->place = NULL;
        p1->next = NULL;
        free((char *) p2);
        fix_x = p1->x;
        fix_y = p1->y;
    }
    DrawSeg(cur_arc);
    InvertLine(fix_x, fix_y, cur_x, cur_y);
    SetMouseHelp("ADD points", "ADJUST points", NULL);
    /*
    put_msg(0,"ADD    points with LEFT button");
    next_msg(0,"ADJUST points with MIDDLE button");
    */
    move = LineTrack;
    left = AddSegment;
    middle = SeePoints;
    set_canvas_tracker();
}


