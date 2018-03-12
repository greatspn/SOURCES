#define ___PLACE___
#	include "global.h"
#	include "draw.h"
#	include "placedialog.h"
#	include "mark.h"
#	include "arc.h"
#	include "res.h"
#include "place.h"
#include "grid.h"

void NewPlace(XButtonEvent *ie) {
    int             object;
    int             a, b;

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

    if (near_obj(fix_x, fix_y, &object) != NULL)
        return;

    cur_place = (struct place_object *) emalloc(PLAOBJ_SIZE);
    cur_place->tag = (char *) emalloc(TAG_SIZE);

loop:	sprintf(cur_place->tag, "P%d", ++place_num);

    for (last_place = netobj->places; last_place != NULL;
            last_place = last_place->next)
        if (strcmp(last_place->tag, cur_place->tag) == 0) {
            goto loop;
        }
    if (netobj->places == NULL)
        netobj->places = cur_place;
    else {
        for (last_place = netobj->places;
                last_place->next != NULL; last_place = last_place->next);
        last_place->next = cur_place;
    }
    cur_place->next = NULL;
    cur_place->center.x = fix_x;
    cur_place->center.y = fix_y;
    cur_place->m0 = 0;
    cur_place->tokens = 0;
    cur_place->mpar = NULL;
    cur_place->cmark = NULL;
    cur_place->distr = NULL;
    cur_place->tagpos.x = place_radius + 5;
    cur_place->tagpos.y = -2;
    cur_place->color = NULL;
    cur_place->lisp = NULL;
    cur_place->colpos.x = place_radius + 5;
    cur_place->colpos.y = 8;
    cur_place->layer =  NewLayerList(CURRENTVIEWANDWHOLE, NULL);
    DrawPlace(cur_place, OR);
    SetModify();
    ClearRes();
}

void EditPlace(XButtonEvent   *ie) {
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;

    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL)
        return;

    ShowPlaceChangeDialog(cur_place, 0);
}

void DeletePlace(XButtonEvent *ie) {
    float           x_loc, y_loc;
    int             object;
    struct net_object *near;
    struct arc_object *bkup_arc, *arc, *arc_tmp;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if ((near = near_node_obj(x_loc, y_loc, &object)) == NULL
            || object != PLACE) {
        return;
    }
    if (IsLayerVisible(WHOLE_NET_LAYER)) {
        free_bkup_netobj();
        SetModify();
        ClearRes();
        DrawPlace(near->places, CLEAR);
        bkup_netobj[0] = create_net();
        ShowPlaceMark(near->places, CLEAR);
        bkup_netobj[0]->places = near->places;
        RemovePlace(near->places);
        near->places->next = NULL;
        bkup_arc = bkup_netobj[0]->arcs = NULL;
        for (arc = netobj->arcs; arc != NULL;) {
            if (arc->place == near->places) {
                DrawArc(arc, CLEAR);
                DrawTrans(arc->trans, OR);
                arc_tmp = arc;
                arc = arc->next;
                RemoveArc(arc_tmp);
                arc_tmp->next = NULL;
                if (bkup_arc == NULL)
                    bkup_netobj[0]->arcs = bkup_arc = arc_tmp;
                else
                    bkup_arc = bkup_arc->next = arc_tmp;
            }
            else
                arc = arc->next;
        }
        DisplayGrid();
        return;
    }
    DisplayGrid();
    ResetVisibleLayers(near->places->layer);
}

void RemovePlace(struct place_object *place) {

    if (netobj->places == place) {
        netobj->places = place->next;
        return;
    }
    for (last_place = netobj->places; last_place->next != NULL;
            last_place = last_place->next)
        if (last_place->next == place) {
            last_place->next = place->next;
            return;
        }
}

