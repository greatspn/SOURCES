#define ___ARC___
#	include "global.h"
#	include "menuS.h"
#	include "arcdialog.h"
#	include "chop.h"
#	include "draw.h"
#	include "line.h"
#	include "res.h"
#include "grid.h"
#include "arc.h"

static float p2x, p2y;
static int   change_p2 = FALSE;
static struct coordinate *pp2;

void ChangeArcType(XButtonEvent   *ie) {
    char            type;
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_arc = near_arc_obj(fix_x, fix_y, &type)) == NULL)
        return;

    ShowArcChangeDialog(cur_arc, 0);
}

void NewLine(XButtonEvent   *ie) {

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if (netobj->places != NULL && (netobj->trans != NULL || netobj->groups != NULL))
        NewArc(' ');
    else
        ShowErrorDialog("Sorry : you must create at least one place and one transition to get an arc !!", frame_w);
}


void NewHinib(XButtonEvent *ie) {

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if (netobj->places != NULL && (netobj->trans != NULL || netobj->groups != NULL))
        NewArc(INHIBITOR);
    else
        ShowErrorDialog("Sorry : you must create at least one place and one transition to get an arc !!", frame_w);
}

void DeleteArc(XButtonEvent *ie) {
    float           x_loc, y_loc;
    char            type;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if ((cur_arc = near_arc_obj(x_loc, y_loc, &type)) == NULL) {
        action_on = 0;
        return;
    }
    if (IsLayerVisible(WHOLE_NET_LAYER)) {
        free_bkup_netobj();
        SetModify();
        ClearRes();
        DrawArc(cur_arc, CLEAR);	/* erase the arc on the screen */
        DrawPlace(cur_arc->place, OR);
        DrawTrans(cur_arc->trans, OR);
        bkup_netobj[0] = create_net();
        bkup_netobj[0]->arcs = cur_arc;
        RemoveArc(cur_arc);
        cur_arc->next = NULL;
        DisplayGrid();
        return;
    }
    ResetVisibleLayers(cur_arc->layer);
}

void RemoveArc(struct arc_object *arc) {
    struct arc_object *arc_pr;

    if (netobj->arcs == arc) {
        netobj->arcs = arc->next;
        return;
    }
    for (arc_pr = netobj->arcs; arc_pr->next != NULL;
            arc_pr = arc_pr->next)
        if (arc_pr->next == arc) {
            arc_pr->next = arc->next;
            return;
        }
}
/*
 * partizione dei settori di 90 gradi in angoli  * di 15 gradi di ampiezza
 * usati per il calcolo dei riaggiustamenti dei punti
 */
static void NewPoints(float sinalpha, float *beta) {
    if (sinalpha >= -S450 && sinalpha < -S375)
        *beta = -1.0;
    else if (sinalpha >= -S375 && sinalpha < -S225)
        *beta = -(T300);
    else if (sinalpha >= -S225 && sinalpha < -S075)
        *beta = -(T150);
    else if (sinalpha >= -S075 && sinalpha < S075)
        *beta = 0.0;
    else if (sinalpha >= S075 && sinalpha < S225)
        *beta = (T150);
    else if (sinalpha >= S225 && sinalpha < S375)
        *beta = (T300);
    else if (sinalpha >= S375 && sinalpha <= S450)
        *beta = 1.0;
}

/*
 * aggiustamento dei punti intermedi    * dell'arco tranne il penultimo in     *
 * quanto il punto di partenza e quello * di arrivo sono fissi
 */
static void AdjustPoint(struct net_object *near) {
    float           delta_x, delta_y;
    float           p1x, p1y, p3x, p3y, d2x, d2y;
    float           cosalfa, sinalfa, lung;
    float           beta = 0;
    struct coordinate *point;

    p2x = fix_x;
    p2y = fix_y;
    start_x = fix_x;
    start_y = fix_y;
    if (cur_arc->point->next == NULL && near != NULL)
        return;
    if (near == NULL) {
        if (cur_x == fix_x || cur_y == fix_y)
            return;
        InvertLine(fix_x, fix_y, cur_x, cur_y);
        if (cur_arc->point->next == NULL && cur_arc->trans != NULL) {
            ChopTrans(&start_x, &start_y, cur_x, cur_y, cur_arc->trans->orient);
        }
        delta_x = cur_x - start_x;
        delta_y = cur_y - start_y;
        d2x = delta_x * delta_x;
        d2y = delta_y * delta_y;
        if ((lung = d2x + d2y) > 0.0)
            lung = (float) sqrt(lung);
        sinalfa = delta_y / lung;
        cosalfa = delta_x / lung;
        if (ABS(cosalfa) >= S450) {
            /* primo settore */
            NewPoints(sinalfa, &beta);
            cur_y = start_y + ABS(delta_x) * beta;
        }
        else {
            /* secondo settore */
            NewPoints(cosalfa, &beta);
            cur_x = start_x + ABS(delta_y) * beta;
        }
        InvertLine(start_x, start_y, cur_x, cur_y);
        /*
        * zoom ( cur_x, cur_y, &n_x, &n_y ); win_setmouseposition(cswfd,
        * (int)n_x, (int)n_y);
        */
        return;
    }
    if (cur_arc->trans != NULL) {
        for (point = cur_arc->point;
                point->next != NULL && point->next->next != NULL;
                point = point->next);
        pp2 = point->next;	/* pointer alle coord. del pto 2 riaggiustato */
        p3x = near->places->center.x;
        p3y = near->places->center.y;
        ChopPlace(&p3x, &p3y, p2x, p2y);
    }
    else {
        pp2 = cur_arc->point;
        point = pp2->next;
        p3x = near->trans->center.x;
        p3y = near->trans->center.y;
        ChopTrans(&p3x, &p3y, p2x, p2y, near->trans->orient);
    }
    /* procedura di riaggiustamento dei punti */
    InvertLine(fix_x, fix_y, cur_x, cur_y);
    p1x = point->x;
    p1y = point->y;
    Readjust(p1x, p1y, &p2x, &p2y, p3x, p3y);
    change_p2 = TRUE;
}


/*
 * partizione dei settori di 90 gradi in  * settori di 7.5 gradi per il
 * calcolo del * riaggiustamento del penultimo punto dell'arco
 */
static void CalcTang(float sinalpha, float *beta3m) {
    if (sinalpha >= -S450 && sinalpha < -S375)
        *beta3m = -T300;
    else if (sinalpha >= -S375 && sinalpha < -S300)
        *beta3m = -1.0;
    else if (sinalpha >= -S300 && sinalpha < -S225)
        *beta3m = -T150;
    else if (sinalpha >= -S225 && sinalpha < -S150)
        *beta3m = -T300;
    else if (sinalpha >= -S150 && sinalpha < -S075)
        *beta3m = 0.0;
    else if (sinalpha >= -S075 && sinalpha < 0.0)
        *beta3m = -T150;
    else if (sinalpha > 0.0 && sinalpha <= S075)
        *beta3m = T150;
    else if (sinalpha > S075 && sinalpha <= S150)
        *beta3m = 0.0;
    else if (sinalpha > S150 && sinalpha <= S225)
        *beta3m = T300;
    else if (sinalpha > S225 && sinalpha <= S300)
        *beta3m = T150;
    else if (sinalpha > S300 && sinalpha <= S375)
        *beta3m = 1.0;
    else if (sinalpha > S375 && sinalpha <= S450)
        *beta3m = T300;
}


/* riaggiustamento del penultimo punto */
/* dell'arco               */
static void Readjust(float x1, float y1, float *x2, float *y2, float x3, float y3) {
    float           dx, dy, dxp, dyp, dxm, dym;
    float           x2p, y2p, x2m, y2m;
    float           sinbeta, cosbeta, d32;
    float           beta3p = 0, beta12, beta3m = 0;
    float           distp, distm;

    InvertLine(x1, y1, *x2, *y2);
    dx = *x2 - x3;
    dy = *y2 - y3;
    if ((d32 = dx * dx + dy * dy) > 0.0)
        d32 = (float) sqrt(d32);
    sinbeta = dy / d32;
    cosbeta = dx / d32;
    /* procedura che calcola le coord. del nuovo punto */
    if (ABS(cosbeta) >= S450) {
        /* primo settore */
        NewPoints(sinbeta, &beta3p);
        CalcTang(sinbeta, &beta3m);
        if (x1 == *x2) {
            *y2 = y3 + ABS(dx) * beta3p;
        }
        else {
            beta12 = (float)(*y2 - y1) / (float)(*x2 - x1);
            if (dx < 0.0)
                beta3p = -beta3p;
            if (beta12 == beta3p) {
                *x2 = (x1 + x3) / 2;
                *y2 = (y1 + y3) / 2;
            }
            else {
                x2p = (y3 - y1 + beta12 * x1 - beta3p * x3) / (beta12 - beta3p);
                y2p = y3 - beta3p * (x3 - x2p);
                x2m = (y3 - y1 + beta12 * x1 - beta3m * x3) / (beta12 - beta3m);
                y2m = y3 - beta3m * (x3 - x2m);
                dxp = (x1 - x2p) * (x1 - x2p);
                dyp = (y1 - y2p) * (y1 - y2p);
                dxm = (x1 - x2m) * (x1 - x2m);
                dym = (y1 - y2m) * (y1 - y2m);
                if ((distp = dxp + dyp) > 0.0)
                    distp = (float) sqrt(distp);
                if ((distm = dxm + dym) > 0.0)
                    distm = (float) sqrt(distm);
                if (distp >= distm) {
                    *x2 = x2m;
                    *y2 = y2m;
                }
                else {
                    *x2 = x2p;
                    *y2 = y2p;
                }
            }
        }
    }
    else {
        /* secondo settore */
        NewPoints(cosbeta, &beta3p);
        CalcTang(-cosbeta, &beta3m);
        if (y1 == *y2) {
            *x2 = x3 + ABS(dy) * beta3p;
        }
        else {
            beta12 = (*x2 - x1) / (*y2 - y1);
            if (dy < 0.0)
                beta3p = -beta3p;
            if (beta12 == beta3p) {
                *x2 = (x1 + x3) / 2;
                *y2 = (y1 + y3) / 2;
            }
            else {
                y2p = (x3 - x1 + beta12 * y1 - beta3p * y3) / (beta12 - beta3p);
                x2p = x3 - beta3p * (y3 - y2p);
                y2m = (x3 - x1 + beta12 * y1 - beta3m * y3) / (beta12 - beta3m);
                x2m = x3 - beta3m * (y3 - y2m);
                dxp = (x1 - x2p) * (x1 - x2p);
                dyp = (y1 - y2p) * (y1 - y2p);
                dxm = (x1 - x2m) * (x1 - x2m);
                dym = (y1 - y2m) * (y1 - y2m);
                if ((distp = dxp + dyp) > 0.0)
                    distp = (float) sqrt(distp);
                if ((distm = dxm + dym) > 0.0)
                    distm = (float) sqrt(distm);
                if (distp >= distm) {
                    *x2 = x2m;
                    *y2 = y2m;
                }
                else {
                    *x2 = x2p;
                    *y2 = y2p;
                }
            }
        }
    }
    InvertLine(x1, y1, *x2, *y2);
    InvertLine(*x2, *y2, x3, y3);
    p2x = *x2;
    p2y = *y2;
    cur_x = x3;
    cur_y = y3;
}


/* usata per la creazione dinamica degli archi */
void SeePoints(int x, int y) {
    struct net_object *near;
    int             object_type;

    change_p2 = FALSE;
    move = null_proc;		/* opzioni del mouse */
    left = null_proc;
    middle = null_proc;
    if ((near = near_node_obj(cur_x, cur_y, &object_type)) != NULL &&
            (((cur_arc->place == NULL && object_type != PLACE) ||
              (cur_arc->trans == NULL && object_type != IMTRANS
               && object_type != EXTRANS && object_type != DETRANS)))) {
        move = LineTrack;
        left = AddSegment;
        middle = SeePoints;
        return;
    }
    AdjustPoint(near);
    move = LineTracking;
    left = ChangePoint;
    middle = SeePoints;
}

/* traccia la linea che segue i movimenti del mouse */
static void LineTracking(int x, int y) {

    if (change_p2) {
        puts("Change_P2");
        change_p2 = FALSE;
        InvertLine(p2x, p2y, cur_x, cur_y);
    }
    else
        InvertLine(cur_x, cur_y, start_x, start_y);
    InvertLine(fix_x, fix_y, p2x, p2y);
    move = LineTrack;
    left = AddSegment;
    middle = SeePoints;
    cur_x = x / zoom_level;
    cur_y = y / zoom_level;
    InvertLine(cur_x, cur_y, fix_x, fix_y);
}

/* effettua i cambiamenti delle coordinate per il */
static void ChangePoint(int x, int y) {
    if (change_p2) {
        fix_x = pp2->x = p2x;
        fix_y = pp2->y = p2y;
        change_p2 = FALSE;
    }
    else {
        fix_x = start_x;
        fix_y = start_y;
    }
    AddSegment(x, y);
}

/* memorizza le coordinate del punto che e'stato corretto */
void ClearLine(float x0, float y0, float x1, float y1) {
    DrawFloatVector((x0 * zoom_level), (y0 * zoom_level), (x1 * zoom_level), (y1 * zoom_level), CLEAR, &mainDS);
}

/*void set_line(x0, y0, x1, y1)
float           x0, y0, x1, y1;
{
   xvf_vector(&mainDS,  (x0 * zoom_level),  (y0 * zoom_level),  (x1 * zoom_level),  (y1 * zoom_level), SET);
}
*/

static void ClearSeg(struct arc_object *arc) {
    struct coordinate *point;

    for (point = arc->point; point->next != NULL; point = point->next)
        DrawFloatVector((point->x * zoom_level), (point->y * zoom_level), (point->next->x * zoom_level), (point->next->y
                        * zoom_level), CLEAR, &mainDS);
    /*      ClearLine(point->x, point->y, point->next->x, point->next->y);*/
}

void AddSegment(int x, int y) {
    struct net_object dum_near;
    struct coordinate *next_point, *point;
    struct net_object *near;
    int             object_type;
    struct arc_object *near_arc;
    struct coordinate *p1, *p2;
    float           xx, yy;

    change_p2 = FALSE;
    if ((ABS((cur_x - fix_x)) < 2.0) && (ABS((cur_y - fix_y)) < 2.0))
        return;
    move = null_proc;
    left = null_proc;
    middle = null_proc;

    if ((near = near_node_obj(cur_x, cur_y, &object_type)) != NULL
            && ((cur_arc->place == NULL && object_type != PLACE) ||
                (cur_arc->trans == NULL && object_type != IMTRANS
                 && object_type != EXTRANS && object_type != DETRANS))) {
        move = LineTrack;
        left = AddSegment;
        middle = SeePoints;
        return;
    }
    if (cur_arc->point->next == NULL) {
        /* second point of the arc */
        /* chop start */
        InvertLine(fix_x, fix_y, cur_x, cur_y);
        if (near != NULL) {
            if (object_type == PLACE) {
                puts("PLACE");
                cur_x = near->places->center.x;
                cur_y = near->places->center.y;
            }
            else {
                puts("TRANS");
                cur_x = near->trans->center.x;
                cur_y = near->trans->center.y;
            }
        }
        if (cur_arc->trans == NULL)
            ChopPlace(&(cur_arc->point->x), &(cur_arc->point->y),
                      cur_x, cur_y);
        else
            ChopTrans(&(cur_arc->point->x), &(cur_arc->point->y),
                      cur_x, cur_y, cur_arc->trans->orient);
        if (near == NULL)
            InvertLine(cur_arc->point->x, cur_arc->point->y,
                       cur_x, cur_y);	/* draw line to be erased */
    }
    near_arc = NULL;
    if (near == NULL)
        for (near_arc = netobj->arcs; near_arc != NULL; near_arc = near_arc->next)
            if ((near_arc != cur_arc) && (near_arc->type == cur_arc->type) && IsLayerListVisible(near_arc->layer))
                for (p1 = near_arc->point; p1->next != NULL; p1 = p2) {
                    p2 = p1->next;
                    if (close_to_vector(p1->x, p1->y, p2->x, p2->y,	cur_x, cur_y, 3.0, &xx, &yy)) {
                        goto out_loop;
                    }
                }
out_loop:
    if (near_arc != NULL) {
        puts("near_arc != NULL");
        next_point = (struct coordinate *) emalloc(COORD_SIZE);
        next_point->x = xx;
        next_point->y = yy;
        near = &dum_near;
        if (cur_arc->place == NULL) {
            puts("cur_arc->place == NULL");
            for (point = cur_arc->point; point->next != NULL;
                    point = point->next);
            point->next = next_point;
            for (p1 = p2, p2 = p2->next; p2 != NULL;
                    p1 = p2, p2 = p2->next) {
                next_point->next = (struct coordinate *) emalloc(COORD_SIZE);
                next_point = next_point->next;
                next_point->x = p1->x;
                next_point->y = p1->y;
            }
            next_point->next = NULL;
            cur_x = p1->x;
            cur_y = p1->y;
            dum_near.places = near_arc->place;
            object_type = PLACE;
        }
        else {
            puts("cur_arc->place != NULL");
            next_point->next = cur_arc->point;
            if (p1 == near_arc->point) {
                cur_arc->point = next_point;
            }
            else {
                cur_arc->point = point = (struct coordinate *) emalloc(COORD_SIZE);
                p1 = near_arc->point->next;
                point->x = p1->x;
                point->y = p1->y;
                for (p1 = p1->next; p2 != p1; p1 = p1->next) {
                    point->next = (struct coordinate *) emalloc(COORD_SIZE);
                    point = point->next;
                    point->x = p1->x;
                    point->y = p1->y;
                }
                point->next = next_point;
            }
            cur_x = near_arc->point->x;
            cur_y = near_arc->point->y;
            dum_near.trans = near_arc->trans;
            object_type = EXTRANS;
        }
    }
    next_point = (struct coordinate *) emalloc(COORD_SIZE);
    next_point->x = cur_x;
    next_point->y = cur_y;
    if (cur_arc->place != NULL) {
        next_point->next = point = cur_arc->point;
        cur_arc->point = next_point;
    }
    else {
        for (point = cur_arc->point; point->next != NULL;
                point = point->next);
        point->next = next_point;
        next_point->next = NULL;
    }
    if (near != NULL) {
        /* attach to target object */
        switch (object_type) {
        case PLACE:
            ClearSeg(cur_arc);
            cur_arc->place = near->places;
            DrawPlace(near->places, OR);
            next_point->x = (near->places)->center.x;
            next_point->y = (near->places)->center.y;
            ChopPlace(&(next_point->x), &(next_point->y),
                      point->x, point->y);
            CleanupArc(cur_arc);
            DrawArc(cur_arc, COPY);
            break;

        case IMTRANS:
        case EXTRANS:
        case DETRANS:
            ClearSeg(cur_arc);
            cur_arc->trans = near->trans;
            DrawTrans(near->trans, OR);
            next_point->x = (near->trans)->center.x;
            next_point->y = (near->trans)->center.y;
            ChopTrans(&(next_point->x), &(next_point->y),
                      point->x, point->y, cur_arc->trans->orient);
            CleanupArc(cur_arc);
            DrawArc(cur_arc, COPY);	/* draw the chopped line */
            break;

        default:
            break;
        }
        for (last_arc = netobj->arcs; last_arc != NULL;
                last_arc = last_arc->next)
            if (last_arc != cur_arc && last_arc->type == cur_arc->type
                    && last_arc->place == cur_arc->place
                    && last_arc->trans == cur_arc->trans) {
                DrawArc(cur_arc, CLEAR);
                DrawArc(last_arc, CLEAR);
                DrawTrans(cur_arc->trans, OR);
                DrawPlace(cur_arc->place, OR);
                if (last_arc->mult < 0)
                    last_arc->mult -= cur_arc->mult;
                else
                    last_arc->mult += cur_arc->mult;
                DrawArc(last_arc, COPY);
                RemoveArc(cur_arc);
                for (point = cur_arc->point; point != NULL;
                        point = next_point) {
                    next_point = point->next;
                    free((char *) point);
                }
                free((char *) cur_arc);
                break;
            }
        UpdateMenuStatusTo(NORMAL_MSTATUS);
        reset_canvas_reader();
        menu_action();
    }
    else {
        p2x = fix_x = cur_x;
        p2y = fix_y = cur_y;
        move = LineTrack;
        left = AddSegment;
        middle = SeePoints;
    }
}


static void CleanupArc(struct arc_object *arc) {
    struct coordinate *p1, *p2, *p3;
    float           d12, d23 = 0, x, y;

    p1 = arc->point;
    p2 = p1->next;
    p3 = p2->next;
    if (p3 == NULL)
        return;
    x = p1->x - p2->x;
    y = p1->y - p2->y;
    if ((d12 = x * x + y * y) > 0.0)
        d12 = (float) sqrt(d12);
    while ((d12 < trans_length) && (p3 != NULL)) {
        p1->next = p3;
        free(p2);
        p2 = p3;
        p3 = p3->next;
        x = p1->x - p2->x;
        y = p1->y - p2->y;
        if ((d12 = x * x + y * y) > 0.0)
            d12 = (float) sqrt(d12);
    }
    if (p3 == NULL)
        return;
    while (p3->next != NULL) {
        if (d12 > MIN_D) {
            x = p3->x - p2->x;
            y = p3->y - p2->y;
            if ((d23 = x * x + y * y) > 0.0)
                d23 = (float) sqrt(d23);
        }
        if (d12 < MIN_D || d23 < MIN_D) {
            p1->next = p3;
            free(p2);
            p2 = p3;
            p3 = p3->next;
        }
        else {
            p1 = p2;
            p2 = p3;
            p3 = p3->next;
        }
        x = p1->x - p2->x;
        y = p1->y - p2->y;
        if ((d12 = x * x + y * y) > 0.0)
            d12 = (float) sqrt(d12);
    }
    if (d12 > MIN_D) {
        x = p3->x - p2->x;
        y = p3->y - p2->y;
        if ((d23 = x * x + y * y) > 0.0)
            d23 = (float) sqrt(d23);
    }
    if (d12 < MIN_D || d23 < trans_length) {
        p1->next = p3;
        free(p2);
    }
    for (p1 = arc->point, p2 = p1->next; p2 != p3;) {
        x = p3->x - p2->x;
        y = p3->y - p2->y;
        if ((d23 = x * x + y * y) > 0.0)
            d23 = (float) sqrt(d23);
        if (d23 < trans_length) {
            p1->next = p2->next;
            free(p2);
            p2 = p1->next;
        }
        else {
            p1 = p2;
            p2 = p2->next;
        }
    }
}

static void NewArc(char type) {
    struct net_object *near;
    int             object_type;

    puts("NewArc");

    p2x = cur_x = fix_x;
    p2y = cur_y = fix_y;
    if ((near = near_node_obj(cur_x, cur_y, &object_type)) == NULL)
        return;
    if (object_type != PLACE && object_type != IMTRANS
            && object_type != DETRANS && object_type != EXTRANS)
        return;
    if (netobj->arcs == NULL)
        last_arc = NULL;
    else
        for (last_arc = netobj->arcs; last_arc->next != NULL;
                last_arc = last_arc->next);
    cur_arc = (struct arc_object *) emalloc(ARCOBJ_SIZE);
    cur_arc->mult = 1;
    cur_arc->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
    if (last_arc != NULL)
        last_arc->next = cur_arc;
    else if (netobj->arcs == NULL)
        netobj->arcs = cur_arc;
    cur_arc->color = NULL;
    cur_arc->lisp = NULL;
    cur_arc->next = NULL;
    switch (object_type) {
    case PLACE:
        if (type == INHIBITOR) {
            cur_arc->type = INHIBITOR;
        }
        else {
            cur_arc->type = TO_TRANS;
        }
        cur_arc->place = near->places;
        cur_arc->trans = NULL;
        cur_x = near->places->center.x;
        cur_y = near->places->center.y;
        break;

    case DETRANS:
    case EXTRANS:
    case IMTRANS:
        if (type == INHIBITOR) {
            cur_arc->type = INHIBITOR;
        }
        else {
            cur_arc->type = TO_PLACE;
        }
        cur_arc->place = NULL;
        cur_arc->trans = near->trans;
        cur_x = near->trans->center.x;
        cur_y = near->trans->center.y;
        break;

    default:
        break;
    }
    cur_arc->point = (struct coordinate *) emalloc(COORD_SIZE);
    cur_arc->point->x = cur_x;
    cur_arc->point->y = cur_y;
    cur_arc->point->next = NULL;
    p2x = fix_x = cur_x;
    p2y = fix_y = cur_y;
    SetModify();
    action_on = 1;
    ClearRes();
    InvertLine(fix_x, fix_y, cur_x, cur_y);
    SetMouseHelp("ADD points", "ADJUST points", NULL);
    move = LineTrack;
    left = AddSegment;
    middle = SeePoints;
    set_canvas_tracker();
    UpdateMenuStatusTo(NOOP_MSTATUS);
}




