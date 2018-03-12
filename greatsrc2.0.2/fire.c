/*
 * fire.c  - manually change markings at places and fire transitions.
 */

/*
#define DEBUG
*/


#include "global.h"
#include "draw.h"
#include "mark.h"

#define TOKENS 0
#define PARAM 1

void setup_mark(int mode) {
    struct place_object *place;

    /* cico */
    for (place = netobj->places; place ; place = place->next) {
        ShowPlaceMark(place, CLEAR);
        if (place->mpar) {
            place->tokens = (mode == PARAM) ? -1 : place->mpar->value;
            place->m0 = -1;
        }
        else
            place->tokens = place->m0;
        ShowPlaceMark(place, SET);
    }
}


void incr_token(XButtonEvent *ie) {
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL)
        return;
    dehighlight_trans();
    DrawMarking(cur_place, CLEAR);
    ++(cur_place->tokens);
    DrawMarking(cur_place, SET);
    set_enabled();
}

void decr_token(XButtonEvent *ie) {
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL)
        return;
    if (cur_place->tokens <= 0)
        return;
    dehighlight_trans();
    DrawMarking(cur_place, CLEAR);
    --(cur_place->tokens);
    DrawMarking(cur_place, SET);
    set_enabled();
}

static struct coordinate *en_list = NULL;


void highlight_place(struct place_object *place) {
    struct coordinate *c_p;

    if (!IsLayerListVisible(place->layer))
        return;
    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = en_list;
    en_list = c_p;
    c_p->x = place->center.x;
    c_p->y = place->center.y;
}


void highlight_trans(struct trans_object *trans) {
    struct coordinate *c_p;

    if (!IsLayerListVisible(trans->layer))
        return;
    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = en_list;
    en_list = c_p;
    c_p->x = trans->center.x;
    c_p->y = trans->center.y;
}


void dehighlight_trans() {
    struct trans_object *trans;
    struct group_object *group;
    int             vanishing = TRUE;

    for (trans = netobj->trans; trans != NULL; trans = trans->next)
        if (trans->enabled) {
            vanishing = FALSE;
            trans->enabled = FALSE;
        }
    if (vanishing)
        for (group = netobj->groups; group != NULL; group = group->next)
            for (trans = group->trans; trans != NULL; trans = trans->next)
                if (trans->enabled) {
                    trans->enabled = FALSE;
                }
    reset_entranslist();
}


void reset_entranslist() {
    struct coordinate *n_p;

    light_on = FALSE;
    if (en_list == NULL)
        return;
    dehighlight_list(en_list, (int) trans_length);
    for (; en_list != NULL; en_list = n_p) {
        n_p = en_list->next;
        free((char *) en_list);
    }
}


struct trans_object *trans_to_fire;

void fire_trans(XButtonEvent          *ie) {
    struct group_object *group;

#ifdef DEBUG
    fprintf(stderr, "\n*** Start of fire_trans\n");
#endif
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_trans = near_titrans_obj(fix_x, fix_y)) == NULL) {
        if ((cur_trans = near_imtrans_obj(fix_x, fix_y, &group)) == NULL) {
            incr_token(ie);
            return;
        }
    }
    if (!cur_trans->enabled)
        return;
    trans_to_fire = cur_trans;
#ifdef DEBUG
    fprintf(stderr, "\n  *** trans_to_fire %d\n", trans_to_n(trans_to_fire));
#endif
    if (timed_flag)
        set_t_fire_name(cur_trans->tag);
    else {
        do_fire(trans_to_fire);
        set_enabled();
    }
}


#define FORW 1
#define BACK 2

void highlight_entranslist() {
    if (en_list == NULL)
        return;
    highlight_list(en_list, (int) trans_length, FALSE, NULL);
}


int             vanishing = FALSE;

void set_immenabled(struct group_object *gp) {
    struct trans_object *trans;

    if (gp->next != NULL)
        set_immenabled(gp->next);
    if (vanishing) {
        for (trans = gp->trans; trans != NULL; trans = trans->next)
            trans->enabled = FALSE;
    }
    else {
        for (trans = gp->trans; trans != NULL; trans = trans->next)
            if (trans->enabled) {
                highlight_trans(trans);
                vanishing = TRUE;
            }
    }
}

void compute_enabled() {
    struct arc_object *arc;
    struct group_object *group;
    struct trans_object *trans;

    vanishing = FALSE;
    for (trans = netobj->trans; trans != NULL; trans = trans->next)
        trans->enabled = TRUE;
    for (group = netobj->groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next)
            trans->enabled = TRUE;
    for (arc = netobj->arcs; arc != NULL; arc = arc->next)
        switch (arc->type) {
        case TO_TRANS:
            if (arc->place->tokens < (ABS(arc->mult)))
                arc->trans->enabled = FALSE;
            continue;
        case INHIBITOR:
            if (arc->place->tokens >= (ABS(arc->mult)))
                arc->trans->enabled = FALSE;
            continue;
        default:
            continue;
        }
    reset_entranslist();
    if ((group = netobj->groups) != NULL)
        set_immenabled(group);
    if (vanishing)
        for (trans = netobj->trans; trans != NULL; trans = trans->next)
            trans->enabled = FALSE;
    else
        for (trans = netobj->trans; trans != NULL; trans = trans->next)
            if (trans->enabled) {
                highlight_trans(trans);
                vanishing = TRUE;
            }
    if (en_list == NULL)
        return;
    highlight_list(en_list, (int) trans_length, FALSE, NULL);
}

int             some_enabled;

void backwrd_imm_enabled(struct group_object *gp) {
    struct trans_object *trans;
    struct arc_object *arc;
    int             cur_pri = gp->pri;

#ifdef DEBUG
    fprintf(stderr, "      Start of backwrd_imm_enabled\n");
#endif
    if (gp->next != NULL)
        backwrd_imm_enabled(gp->next);
    if (!some_enabled) {
        for (trans = gp->trans; trans != NULL; trans = trans->next) {
            trans->enabled = TRUE;
        }
        for (arc = netobj->arcs; arc != NULL; arc = arc->next) {
            trans = arc->trans;
            if (trans->kind == cur_pri)
                switch (arc->type) {
                case TO_TRANS:
                    if (arc->place->tokens < (ABS(arc->mult)))
                        arc->trans->enabled = FALSE;
                    continue;
                case INHIBITOR:
                    if (arc->place->tokens >= (ABS(arc->mult)))
                        arc->trans->enabled = FALSE;
                    continue;
                default:
                    continue;
                }
        }
        for (trans = gp->trans; trans != NULL; trans = trans->next) {
            if (trans->enabled) {
                some_enabled = TRUE;
                trans->enabled = FALSE;
            }
        }
    }
#ifdef DEBUG
    fprintf(stderr, "      End of backwrd_imm_enabled\n");
#endif
}


void set_bw_enabl(struct trans_object *trans,
                  struct group_object *group) {
    struct arc_object *arc;
    struct place_object *place;

#ifdef DEBUG
    fprintf(stderr, "    Start of set_bw_enabl %p %p\n",  trans,  group);
#endif
    trans->enabled = TRUE;
    for (arc = netobj->arcs; arc != NULL; arc = arc->next)
        if (arc->trans == trans) {
#ifdef DEBUG
            fprintf(stderr, "         ..arc with place %p, type=%c, mult = %d\n",
                    arc->place, arc->type, arc->mult);
#endif
            if (arc->type == TO_TRANS)
                arc->place->tokens += (ABS(arc->mult));
            else if (arc->type == TO_PLACE)
                arc->place->tokens -= (ABS(arc->mult));
        }
#ifdef DEBUG
    fprintf(stderr, "     .. transition unfired\n");
#endif
    for (place = netobj->places; place != NULL;) {
#ifdef DEBUG
        fprintf(stderr, "       .. place %p\n",  place);
        fprintf(stderr, "       .. place->tokens=%d\n", (int) place->tokens);
#endif
        if (place->tokens < 0) {
#ifdef DEBUG
            fprintf(stderr, "       .. disabling transition\n");
#endif
            trans->enabled = FALSE;
            place = NULL;
        }
        else
            place = place->next;
    }
#ifdef DEBUG
    fprintf(stderr, "     .. places checked\n");
#endif
    if (trans->enabled) {
#ifdef DEBUG
        fprintf(stderr, "      .. trans is enabled 1\n");
#endif
        for (arc = netobj->arcs; arc != NULL; arc = arc->next)
            if (arc->trans == trans)
                switch (arc->type) {
                case TO_TRANS:
                    if (arc->place->tokens < (ABS(arc->mult)))
                        trans->enabled = FALSE;
                    continue;
                case INHIBITOR:
                    if (arc->place->tokens >= (ABS(arc->mult)))
                        trans->enabled = FALSE;
                    continue;
                default:
                    continue;
                }
#ifdef DEBUG
        fprintf(stderr, "     .. I and H arcs checked\n");
#endif
        if (trans->enabled) {
#ifdef DEBUG
            fprintf(stderr, "      .. trans is enabled 2\n");
#endif
            if (group != NULL) {
                some_enabled = FALSE;
                backwrd_imm_enabled(group);
                trans->enabled = (!some_enabled);
            }
            if (trans->enabled) {
#ifdef DEBUG
                fprintf(stderr, "      .. trans is enabled 3\n");
#endif
                highlight_trans(trans);
                vanishing = TRUE;
            }
#ifdef DEBUG
            else
                fprintf(stderr, "      .. trans is NOT enabled 3\n");
#endif
        }
#ifdef DEBUG
        else
            fprintf(stderr, "      .. trans is NOT enabled 2\n");
#endif
    }
#ifdef DEBUG
    else
        fprintf(stderr, "      .. trans is NOT enabled 1\n");
#endif
    for (arc = netobj->arcs; arc != NULL; arc = arc->next)
        if (arc->trans == trans) {
            if (arc->type == TO_TRANS)
                arc->place->tokens -= (ABS(arc->mult));
            else if (arc->type == TO_PLACE)
                arc->place->tokens += (ABS(arc->mult));
        }
#ifdef DEBUG
    fprintf(stderr, "    End of set_bw_enabl\n");
#endif
}


void backwrd_enabled() {
    struct trans_object *trans;
    struct group_object *group = netobj->groups;

#ifdef DEBUG
    fprintf(stderr, "  Start of backwrd_enabled\n");
#endif
    reset_entranslist();
    vanishing = FALSE;
    for (trans = netobj->trans; trans != NULL; trans = trans->next) {
        set_bw_enabl(trans, group);
    }
    for (; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next)
            set_bw_enabl(trans, group->next);
    if (en_list != NULL)
        highlight_list(en_list, (int) trans_length, FALSE, NULL);
#ifdef DEBUG
    fprintf(stderr, "  End of backwrd_enabled\n");
#endif
}

#define DEBUG
void set_enabled() {

#ifdef DEBUG
    fprintf(stderr, "  Start of set_enabled (direction=%d)\n", direction);
#endif
    if (direction == FORW)
        compute_enabled();
    else
        backwrd_enabled();
    if (vanishing) {
        put_msg(0, "FIRE transitions with LEFT button");
        next_msg(0, "ADD    tokens to   places with LEFT   button");
        next_msg(0, "REMOVE tokens from places with MIDDLE button");
    }
    else {			/* absorbing marking */
        put_msg(2, " ... DEAD MARKING !");
        next_msg(0, "INCREMENT/DECREMENT place markings with LEFT/MIDDLE button");
    }
#ifdef DEBUG
    fprintf(stderr, "  End of set_enabled\n");
#endif
}
