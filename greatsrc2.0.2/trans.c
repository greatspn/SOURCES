#define ___TRANS___
#	include "global.h"
#	include "transdialog.h"
#	include "draw.h"
#	include "chop.h"
#	include "arc.h"
#	include "res.h"
#include "trans.h"
#include "grid.h"

void NewImtrans(XButtonEvent *ie) {
    struct group_object *group;
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
    cur_trans = (struct trans_object *) emalloc(TRNOBJ_SIZE);
    cur_trans->tag = (char *) emalloc(TAG_SIZE);
loop: sprintf(cur_trans->tag, "t%d", ++trans_num);
    for (last_trans = netobj->trans; last_trans != NULL;
            last_trans = last_trans->next)
        if (strcmp(last_trans->tag, cur_trans->tag) == 0) {
            goto loop;
        }
    for (group = netobj->groups; group != NULL; group = group->next)
        for (last_trans = group->trans; last_trans != NULL;
                last_trans = last_trans->next)
            if (strcmp(last_trans->tag, cur_trans->tag) == 0) {
                goto loop;
            }
    cur_trans->center.x = fix_x;
    cur_trans->center.y = fix_y;
    cur_trans->orient = cur_orient;
    cur_trans->enabl = 1;
    cur_trans->enabled = FALSE;
    cur_trans->kind = IMMEDIATE;
    cur_trans->fire_rate.ff = 1.0;
    cur_trans->rpar = NULL;
    cur_trans->mark_dep = NULL;
    cur_trans->tagpos.x = -(trans_length / 2 + 10);
    cur_trans->tagpos.y = -18;
    cur_trans->color = NULL;
    cur_trans->lisp = NULL;
    cur_trans->colpos.x = trans_length / 2 + 3;
    cur_trans->colpos.y = -7;
    cur_trans->ratepos.x = trans_length / 2 + 3;
    cur_trans->ratepos.y = titrans_height / 2 + 2;
    cur_trans->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
    DrawTrans(cur_trans, OR);
    SetModify();
    ClearRes();
    group = netobj->groups;
    if ((group == NULL) || (netobj->groups->pri != 1)) {
        /* create a group */
        group = (struct group_object *) emalloc(GRPOBJ_SIZE);
        group->next = netobj->groups;
        netobj->groups = group;
        group->tag = (char *) emalloc(TAG_SIZE);
        sprintf(group->tag, "G%d", ++group_num);
        group->pri = 1;
        group->movelink = NULL;
        group->center.x = fix_x;
        group->center.y = fix_y;
        group->trans = cur_trans;
        cur_trans->next = NULL;
    }
    else {
        cur_trans->next = group->trans;
        group->trans = cur_trans;
    }
}


void NewTitrans(XButtonEvent          *ie) {
    int             object;
    int             a, b;
    struct group_object *group;

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
    cur_trans = (struct trans_object *) emalloc(TRNOBJ_SIZE);
    cur_trans->tag = (char *) emalloc(TAG_SIZE);
loop: sprintf(cur_trans->tag, "T%d", ++trans_num);
    for (last_trans = netobj->trans; last_trans != NULL;
            last_trans = last_trans->next)
        if (strcmp(last_trans->tag, cur_trans->tag) == 0) {
            goto loop;
        }
    for (group = netobj->groups; group != NULL; group = group->next)
        for (last_trans = group->trans; last_trans != NULL;
                last_trans = last_trans->next)
            if (strcmp(last_trans->tag, cur_trans->tag) == 0) {
                goto loop;
            }
    if (netobj->trans == NULL)
        netobj->trans = cur_trans;
    else {
        for (last_trans = netobj->trans; last_trans->next != NULL;
                last_trans = last_trans->next);
        last_trans->next = cur_trans;
    }
    cur_trans->next = NULL;
    cur_trans->center.x = fix_x;
    cur_trans->center.y = fix_y;
    cur_trans->orient = cur_orient;
    cur_trans->enabl = 0;
    cur_trans->enabled = FALSE;
    if (cur_object == DETRANS)
        cur_trans->kind = DETERMINISTIC;
    else
        cur_trans->kind = EXPONENTIAL;
    cur_trans->fire_rate.ff = 1.0;
    cur_trans->rpar = NULL;
    cur_trans->mark_dep = NULL;
    cur_trans->tagpos.x = -(trans_length / 2 + 3);
    cur_trans->tagpos.y = -7;
    cur_trans->color = NULL;
    cur_trans->lisp = NULL;
    cur_trans->colpos.x = trans_length / 2 + 3;
    cur_trans->colpos.y = -7;
    cur_trans->ratepos.x = trans_length / 2 + 3;
    cur_trans->ratepos.y = titrans_height / 2 + 2;
    cur_trans->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);
    DrawTrans(cur_trans, OR);
    SetModify();
    ClearRes();
}

void ChangeTransKind(XButtonEvent  *ie) {
    float           x_loc, y_loc;
    struct group_object *group;

    x_loc = ie->x / zoom_level;
    y_loc = ie->y / zoom_level;
    if ((cur_trans = near_titrans_obj(x_loc, y_loc)) == NULL
            && (cur_trans = near_imtrans_obj(x_loc, y_loc, &group)) == NULL)
        return;

    ShowTransChangeDialog(cur_trans, 0);
}

void RotateTrans(XButtonEvent  *ie) {
    struct coordinate *t_p, *n_p;
    float           x_loc, y_loc;
    struct group_object *group;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if ((cur_trans = near_titrans_obj(x_loc, y_loc)) == NULL) {
        if ((cur_trans = near_imtrans_obj(x_loc, y_loc, &group)) == NULL) {
            cur_orient = rot_trans(cur_orient, ROTATE45);
            menu_action();
            return;
        }
    }
    DrawTrans(cur_trans, CLEAR);
    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = cur_arc->next)
        if (cur_arc->trans == cur_trans)
            DrawArc(cur_arc, CLEAR);
    cur_trans->orient = rot_trans((int)(cur_trans->orient), ROTATE45);
    DrawTrans(cur_trans, OR);
    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = cur_arc->next)
        if (cur_arc->trans == cur_trans) {
            t_p = cur_arc->point;
            n_p = t_p->next;
            t_p->x = cur_trans->center.x;
            t_p->y = cur_trans->center.y;
            ChopTrans(&(t_p->x), &(t_p->y), n_p->x, n_p->y, cur_trans->orient);
            DrawArc(cur_arc, OR);
        }
    SetModify();

}


void DeleteTrans(XButtonEvent *ie) {
    float           x_loc, y_loc;
    int             object;
    struct net_object *near;
    struct arc_object *bkup_arc, *arc, *arc_tmp;
    struct group_object *group, *group_tmp;
    struct trans_object *trans;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if (((near = near_node_obj(x_loc, y_loc, &object)) == NULL)	||
            (object != EXTRANS && object != DETRANS && object != IMTRANS)) {
        return;
    }

    if (!IsLayerVisible(WHOLE_NET_LAYER)) {
        ResetVisibleLayers(near->trans->layer);
        return;
    }
    free_bkup_netobj();
    SetModify();
    ClearRes();
    DrawTrans(near->trans, CLEAR);
    bkup_netobj[0] = create_net();
    switch (object) {
    case EXTRANS:
    case DETRANS:
        bkup_netobj[0]->trans = near->trans;
        RemoveTrans(near->trans);
        break;
    case IMTRANS:
        for (group_tmp = NULL, group = netobj->groups;
                group != NULL;
                group_tmp = group, group = group->next)
            for (trans = group->trans; trans != NULL;
                    trans = trans->next)
                if (trans == near->trans)
                    goto endsearch;
endsearch:
        if (group->trans == near->trans
                && group->trans->next == NULL) {
            if (group_tmp == NULL)
                netobj->groups = group->next;
            else
                group_tmp->next = group->next;
            group->next = NULL;
            group->movelink = NULL;
            bkup_netobj[0]->groups = group;
        }
        else {
            group_tmp = (struct group_object *) emalloc(GRPOBJ_SIZE);
            group_tmp->tag = emalloc(3);
            sprintf(group_tmp->tag, "xx");
            group_tmp->next = NULL;
            group_tmp->movelink = group;
            group_tmp->pri = group->pri;
            group_tmp->trans = near->trans;
            bkup_netobj[0]->groups = group_tmp;
            RemoveTrans(near->trans);
        }
        break;
    }
    near->trans->next = NULL;
    bkup_arc = bkup_netobj[0]->arcs;
    arc = netobj->arcs;
    while (arc != NULL) {
        if (arc->trans == near->trans) {
            DrawArc(arc, CLEAR);
            DrawPlace(arc->place, OR);
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
    action_on = 0;
    DisplayGrid();
}

void RemoveTrans(struct trans_object *trans) {
    struct group_object *group;

    if (trans->kind == EXPONENTIAL || trans->kind == DETERMINISTIC) {
        if (netobj->trans == trans) {
            netobj->trans = trans->next;
            return;
        }
        for (last_trans = netobj->trans; last_trans->next != NULL;
                last_trans = last_trans->next)
            if (last_trans->next == trans) {
                last_trans->next = trans->next;
                return;
            }
    }
    else
        for (group = netobj->groups; group != NULL;
                group = group->next) {
            if (group->trans == NULL)
                continue;
            if (group->trans == trans) {
                group->trans = trans->next;
                return;
            }
            for (last_trans = group->trans; last_trans->next != NULL;
                    last_trans = last_trans->next)
                if (last_trans->next == trans) {
                    last_trans->next = trans->next;
                    return;
                }
        }
}
