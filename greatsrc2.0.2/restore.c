/*
 * restore.c  -  restores the last deleted objects or undoes selection
 *
 * the last deleted objects are contained into the special net bkup_netobj[0]
 */

#include "global.h"
#include "Canvas.h"

void restore_obj() {
    struct net_object *bkup_net0;
    struct mpar_object *mpar, *mpar_tmp;
    struct place_object *place, *place_tmp;
    struct rpar_object *rpar, *rpar_tmp;
    struct trans_object *trans, *trans_tmp;
    struct group_object *group, *group_tmp, *g_p, *auxgr;
    struct arc_object *arc, *arc_tmp;
    struct res_object *res, *res_tmp;
    struct lisp_object *lisp, *lisp_tmp;
    int             i;

    if ((bkup_net0 = bkup_netobj[0]) != NULL) {
        if ((mpar = bkup_net0->mpars) != NULL) {
            for (mpar_tmp = bkup_net0->mpars; mpar_tmp->next != NULL;
                    mpar_tmp = mpar_tmp->next);
            mpar_tmp->next = netobj->mpars;
            netobj->mpars = mpar;
            bkup_net0->mpars = NULL;
        }
        if ((place = bkup_net0->places) != NULL) {
            for (place_tmp = bkup_net0->places; place_tmp->next != NULL;
                    place_tmp = place_tmp->next);
            place_tmp->next = netobj->places;
            netobj->places = place;
            bkup_net0->places = NULL;
        }
        if ((rpar = bkup_net0->rpars) != NULL) {
            for (rpar_tmp = bkup_net0->rpars; rpar_tmp->next != NULL;
                    rpar_tmp = rpar_tmp->next);
            rpar_tmp->next = netobj->rpars;
            netobj->rpars = rpar;
            bkup_net0->rpars = NULL;
        }
        if ((trans = bkup_net0->trans) != NULL) {
            for (trans_tmp = bkup_net0->trans; trans_tmp->next != NULL;
                    trans_tmp = trans_tmp->next);
            trans_tmp->next = netobj->trans;
            netobj->trans = trans;
            bkup_net0->trans = NULL;
        }
        if ((group = bkup_net0->groups) != NULL) {
            for (group_tmp = NULL; group != NULL;)
                if (group->movelink == NULL) {
                    auxgr = netobj->groups;
                    g_p = group;
                    group = group->next;
                    if (auxgr == NULL) {
                        netobj->groups = auxgr = g_p;
                        auxgr->next = NULL;
                    }
                    else {
                        while ((auxgr->next != NULL) &&
                                (g_p->pri > auxgr->next->pri))
                            auxgr = auxgr->next;
                        if (auxgr->pri > g_p->pri) {
                            g_p->next = auxgr;
                            netobj->groups = g_p;
                        }
                        else if (auxgr->pri == g_p->pri) {
                            for (trans = g_p->trans;
                                    trans->next != NULL;
                                    trans = trans->next);
                            trans->next = auxgr->trans;
                            auxgr->trans = g_p->trans;
                            g_p->trans = NULL;
                            g_p->next = group_tmp;
                            group_tmp = g_p;
                        }
                        else {
                            g_p->next = auxgr->next;
                            auxgr->next = g_p;
                        }
                    }
                }
                else {
                    g_p = group;
                    group = group->next;
                    for (trans = g_p->trans;
                            trans->next != NULL;
                            trans = trans->next);
                    trans->next = g_p->movelink->trans;
                    g_p->movelink->trans = g_p->trans;
                    g_p->trans = NULL;
                    g_p->next = group_tmp;
                    group_tmp = g_p;
                }
            bkup_net0->groups = group_tmp;
        }
        if ((res = bkup_net0->results) != NULL) {
            for (res_tmp = bkup_net0->results; res_tmp->next != NULL;
                    res_tmp = res_tmp->next);
            res_tmp->next = netobj->results;
            netobj->results = res;
            bkup_net0->results = NULL;
        }
        if ((lisp = bkup_net0->lisps) != NULL) {
            for (lisp_tmp = bkup_net0->lisps; lisp_tmp->next != NULL;
                    lisp_tmp = lisp_tmp->next);
            lisp_tmp->next = netobj->lisps;
            netobj->lisps = lisp;
            bkup_net0->lisps = NULL;
        }
        if ((arc = bkup_net0->arcs) != NULL) {
            for (arc_tmp = bkup_net0->arcs; arc_tmp->next != NULL;
                    arc_tmp = arc_tmp->next);
            arc_tmp->next = netobj->arcs;
            netobj->arcs = arc;
            bkup_net0->arcs = NULL;
        }
        free_netobj(bkup_net0);
    }
    for (i = 0; ++i < MAX_BKUP; bkup_netobj[i - 1] = bkup_netobj[i]);
    bkup_netobj[MAX_BKUP - 1] = NULL;
    redisplay_canvas();
}
