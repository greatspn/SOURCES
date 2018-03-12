/*
 * net.c
 */
#include "global.h"
#	include "res.h"
#include "layer.h"

int init_net() {
    place_num = 0;
    trans_num = 0;
    group_num = 0;
    netobj = create_net();
    SetApplicationTitle("Untitled");
    return (1);
}

struct net_object *create_net() {
    struct net_object *net;

    net = (struct net_object *) emalloc(NETOBJ_SIZE);
    reset_net_object(net);
    return (net);
}

void reset_net_object(struct net_object *net) {
    net->comment = NULL;
    net->nwcorner.x = 0.0;
    net->nwcorner.y = 0.0;
    net->secorner.x = canvas_width;
    net->secorner.y = canvas_height;
    net->boxht = titrans_height;
    net->boxwid = trans_length;
    net->circlerad = place_radius;
    net->mpars = NULL;
    net->places = NULL;
    net->rpars = NULL;
    net->trans = NULL;
    net->groups = NULL;
    net->arcs = NULL;
    net->results = NULL;
    net->lisps = NULL;
    net->texts = NULL;
    net->next = NULL;
}

void free_bkup_netobj() {
    int             i;
    struct net_object *bkup_netMAX;

    if ((bkup_netMAX = bkup_netobj[MAX_BKUP - 1]) != NULL)
        free_netobj(bkup_netMAX);
    for (i = MAX_BKUP; --i > 0; bkup_netobj[i] = bkup_netobj[i - 1]);
    bkup_netobj[0] = NULL;
}

void free_netobj(struct net_object *net) {
    purge_netobj(net);
    free((char *) net);
}

void delete_selected() {

    if (moving_images == NULL)
        return;
    if (!IsLayerVisible(WHOLE_NET_LAYER)) {
        ResetSelectedLayer(&selected);
        put_msg(1, "Selected objects DELETED from current layers");
        return;
    }
    free_bkup_netobj();
    SetModify();
    ClearRes();
    bkup_netobj[0] = create_net();
    prev_x = select_left;
    prev_y = select_top;
#ifdef DEBUG
    fprintf(stderr, "... starting remove_selected\n");
#endif
    remove_selected();
#ifdef DEBUG
    fprintf(stderr, "... end; starting delete_sel_params\n");
#endif
    delete_sel_params();
#ifdef DEBUG
    fprintf(stderr, "... end; starting delete_unsel_arcs\n");
#endif
    delete_unsel_arcs();
#ifdef DEBUG
    fprintf(stderr, "... end; starting drop_selected\n");
#endif
    drop_selected((int) select_left, (int) select_top, bkup_netobj[0]);
#ifdef DEBUG
    fprintf(stderr, "... end drop_selected\n");
#endif
    put_msg(1, "Selected objects DELETED");
}

void remove_selected() {
    struct group_object *g_p, *group, *auxsl;

    if ((group = netobj->groups) == NULL)
        return;
    while (group != NULL && group->trans == NULL) {
        g_p = group;
        group = group->next;
        netobj->groups = group;
        auxsl = selected.groups;
        while (auxsl != NULL) {
            if (auxsl->movelink == g_p) {
                auxsl->movelink = NULL;
            }
            auxsl = auxsl->next;
        }
        free(g_p);
    }
    while (group != NULL && group->next != NULL) {
        if (group->next->trans != NULL)
            group = group->next;
        else {
            g_p = group->next;
            group->next = g_p->next;
            auxsl = selected.groups;
            while (auxsl != NULL) {
                if (auxsl->movelink == g_p) {
                    auxsl->movelink = NULL;
                }
                auxsl = auxsl->next;
            }
            free(g_p);
        }
    }
}

void delete_sel_params() {
    struct mpar_object *mpar;
    struct place_object *place;
    struct rpar_object *rpar;
    struct trans_object *trans;
    struct group_object *group;

    for (mpar = selected.mpars; mpar != NULL; mpar = mpar->next)
        for (place = netobj->places; place != NULL; place = place->next)
            if (place->mpar == mpar) {
                place->mpar = NULL;
                place->tokens = 0;
                place->m0 = 0;
            }
    for (rpar = selected.rpars; rpar != NULL; rpar = rpar->next) {
        for (trans = netobj->trans; trans != NULL; trans = trans->next)
            if (trans->rpar == rpar) {
                trans->rpar = NULL;
                trans->mark_dep = NULL;
            }
        for (group = netobj->groups; group != NULL; group = group->next)
            for (trans = group->trans; trans != NULL; trans = trans->next)
                if (trans->rpar == rpar) {
                    trans->rpar = NULL;
                    trans->mark_dep = NULL;
                }
    }
}

void delete_unsel_arcs() {
    struct arc_object *arc, *arc_tmp;
    struct place_object *place;
    struct trans_object *trans;
    struct group_object *group;

    for (place = selected.places; place != NULL; place = place->next)
        for (arc_tmp = NULL, arc = netobj->arcs; arc != NULL;)
            if (arc->place != place) {
                arc_tmp = arc;
                arc = arc->next;
            }
            else if (arc_tmp == NULL) {
                netobj->arcs = arc->next;
                arc->next = selected.arcs;
                selected.arcs = arc;
                arc = netobj->arcs;
            }
            else {
                arc_tmp->next = arc->next;
                arc->next = selected.arcs;
                selected.arcs = arc;
                arc = arc_tmp->next;
            }
    for (trans = selected.trans; trans != NULL; trans = trans->next)
        for (arc_tmp = NULL, arc = netobj->arcs; arc != NULL;)
            if (arc->trans != trans) {
                arc_tmp = arc;
                arc = arc->next;
            }
            else if (arc_tmp == NULL) {
                netobj->arcs = arc->next;
                arc->next = selected.arcs;
                selected.arcs = arc;
                arc = netobj->arcs;
            }
            else {
                arc_tmp->next = arc->next;
                arc->next = selected.arcs;
                selected.arcs = arc;
                arc = arc_tmp->next;
            }
    for (group = selected.groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL; trans = trans->next)
            for (arc_tmp = NULL, arc = netobj->arcs; arc != NULL;)
                if (arc->trans != trans) {
                    arc_tmp = arc;
                    arc = arc->next;
                }
                else if (arc_tmp == NULL) {
                    netobj->arcs = arc->next;
                    arc->next = selected.arcs;
                    selected.arcs = arc;
                    arc = netobj->arcs;
                }
                else {
                    arc_tmp->next = arc->next;
                    arc->next = selected.arcs;
                    selected.arcs = arc;
                    arc = arc_tmp->next;
                }
}

void purge_netobj(struct net_object *net) {
    struct mpar_object *mpar, *mpar_tmp;
    struct probability *prob, *prob_tmp;
    struct place_object *place, *place_tmp;
    struct rpar_object *rpar, *rpar_tmp;
    struct trans_object *trans, *trans_tmp;
    struct group_object *group, *group_tmp;
    struct arc_object *arc, *arc_tmp;
    struct res_object *res, *res_tmp;
    struct lisp_object *lisp, *lisp_tmp;
    struct coordinate *cur_point, *last_point;
    char           *cp;
    /*
    com = net->comment;
    while (com != NULL) {
    com_tmp = com->next;
    if ((cp = com->line) != NULL)
        free((char *) cp);
    free((char *) com);
    com = com_tmp;
    }
    */
    XtFree(net->comment);
    mpar = net->mpars;
    while (mpar != NULL) {
        mpar_tmp = mpar->next;
        if ((cp = mpar->tag) != NULL)
            free((char *) cp);
        free((char *) mpar->layer);
        free((char *) mpar);
        mpar = mpar_tmp;
    }
    place = net->places;
    while (place != NULL) {
        place_tmp = place->next;
        if ((cp = place->tag) != NULL)
            free((char *) cp);
        if ((cp = place->color) != NULL)
            free((char *) cp);
        for (prob = place->distr, place->distr = NULL; prob != NULL;
                prob = prob_tmp) {
            prob_tmp = prob->next;
            free((char *) prob);
        }
        free((char *) place->layer);
        free((char *) place);
        place = place_tmp;
    }
    rpar = net->rpars;
    while (rpar != NULL) {
        rpar_tmp = rpar->next;
        if ((cp = rpar->tag) != NULL)
            free((char *) cp);
        free((char *) rpar->layer);
        free((char *) rpar);
        rpar = rpar_tmp;
    }
    trans = net->trans;
    while (trans != NULL) {
        trans_tmp = trans->next;
        if ((cp = trans->tag) != NULL)
            free((char *) cp);
        if ((cp = trans->color) != NULL)
            free((char *) cp);
        /*
        for (com = trans->mark_dep; com != NULL; com = com_tmp) {
            com_tmp = com->next;
            if (com->line != NULL)
        	free((char *) (com->line));
            free((char *) com);

        }
        */
        if (trans->mark_dep != NULL)
            free(trans->mark_dep);
        if (trans->enabl < 0)
            free((char *)(trans->fire_rate.fp));
        free((char *) trans->layer);
        free((char *) trans);
        trans = trans_tmp;
    }
    group = net->groups;
    while (group != NULL) {
        group_tmp = group->next;
        trans = group->trans;
        while (trans != NULL) {
            trans_tmp = trans->next;
            if ((cp = trans->tag) != NULL)
                free((char *) cp);
            if ((cp = trans->color) != NULL)
                free((char *) cp);
            /*
            for (com = trans->mark_dep; com != NULL; com = com_tmp) {
            com_tmp = com->next;
            if (com->line != NULL)
                free((char *) (com->line));
            free((char *) com);
            }
            */
            if (trans->mark_dep != NULL)
                free(trans->mark_dep);
            free((char *) trans);
            trans = trans_tmp;
        }
        if ((cp = group->tag) != NULL)
            free((char *) cp);
        free((char *) group);
        group = group_tmp;
    }
    arc = net->arcs;
    while (arc != NULL) {
        if ((cp = arc->color) != NULL)
            free((char *) cp);
        arc_tmp = arc->next;
        last_point = arc->point;
        while (last_point != NULL) {
            cur_point = last_point->next;
            free((char *) last_point);
            last_point = cur_point;
        }
        free((char *) arc->layer);
        free((char *) arc);
        arc = arc_tmp;
    }
    res = net->results;
    while (res != NULL) {
        res_tmp = res->next;

        if ((cp = res->tag) != NULL)
            free((char *) cp);
        /*
        for (com = res->text; com != NULL; com = com_tmp) {
        com_tmp = com->next;
        if ((cp = com->line) != NULL)
        free((char *) cp);
        free((char *) com);
        }
        */
        free((char *) res->text);
        free((char *) res);

        res = res_tmp;
    }
    lisp = net->lisps;
    while (lisp != NULL) {
        lisp_tmp = lisp->next;
        if ((cp = lisp->tag) != NULL)
            free((char *) cp);
        /*
        for (com = lisp->text; com != NULL; com = com_tmp)
        {
            com_tmp = com->next;
            if ((cp = com->line) != NULL)
        	free((char *) cp);
            free((char *) com);
        }
        */
        free(lisp->text);
        free((char *) lisp);
        lisp = lisp_tmp;
    }
}
