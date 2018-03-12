/*
 * tokens.c  - display token movements on arcs during transition firing.
 */

/*
#define DEBUG
*/


#include "global.h"
#include "draw.h"
#include <unistd.h>

#define TOKENS 0
#define PARAM 1

struct arc_token {
    short           num_t;
    struct coordinate *coo_l;
    struct arc_token *next;
};

static struct arc_token *arc_l = NULL;
static struct arc_token *arc_free = NULL;
static struct coordinate *coo_free = NULL;

struct arc_token *get_arc_struct() {
    struct arc_token *arc_p;

    if ((arc_p = arc_free) == NULL) {
        arc_p = (struct arc_token *) emalloc(sizeof(struct arc_token));
        return (arc_p);
    }
    arc_free = arc_p->next;
    return (arc_p);
}


struct coordinate *get_coo_struct() {
    struct coordinate *coo_p;

    if ((coo_p = coo_free) == NULL) {
        coo_p = (struct coordinate *) emalloc(sizeof(struct coordinate));
        return (coo_p);
    }
    coo_free = coo_p->next;
    return (coo_p);
}


void add_arc_moving(struct arc_object *cur_arc,
                    int             to_delete) {
    int             nt;
    double          ll, ss, dist, length, seg_l;
    double          px, py, nx, ny, dirx = 0, diry = 0;
    struct coordinate *ncp, *acp, *lcp;
    struct arc_token *arc_p;
    int             again;

#ifdef DEBUG
    fprintf(stderr, "          start of add_arc_moving (to_delete=%d)\n", to_delete);
#endif
    nt = ABS(cur_arc->mult);
    if (to_delete) {
        cur_place = cur_arc->place;
        if (IsLayerListVisible(cur_place->layer)) {
            DrawMarking(cur_place, CLEAR);
        }
        (cur_place->tokens) -= nt;
        if (IsLayerListVisible(cur_place->layer)) {
            DrawMarking(cur_place, SET);
            highlight_place(cur_arc->place);
        }
    }
    if ((!number_of_shots) || !IsLayerListVisible(cur_arc->layer)) {
#ifdef DEBUG
        fprintf(stderr, "          end of add_arc_moving (NO SHOTS)\n");
#endif
        return;
    }
#ifdef DEBUG
    fprintf(stderr, "            .. computing %d shot points\n", number_of_shots);
#endif
    arc_p = get_arc_struct();
    arc_p->next = arc_l;
    arc_l = arc_p;
    arc_p->num_t = nt;
    arc_p->coo_l = NULL;
#ifdef DEBUG
    fprintf(stderr, "            .. arc_struct created\n");
#endif
    cur_trans = cur_arc->trans;
    for (length = 0.0, px = cur_trans->center.x, py = cur_trans->center.y,
            ncp = cur_arc->point;
            ncp != NULL; ncp = ncp->next) {
        nx = ncp->x;
        ny = ncp->y;
        ss = px - nx;
        ss *= ss;
        ll = py - ny;
        ll *= ll;
        if ((ss += ll) > 0.0)
            length += sqrt(ss);
        px = nx;
        py = ny;
    }
    cur_place = cur_arc->place;
    ss = px - cur_place->center.x;
    ss *= ss;
    ll = py - cur_place->center.y;
    ll *= ll;
    if ((ss += ll) > 0.0)
        length += sqrt(ss);
    dist = length / (double)(number_of_shots + 1);
#ifdef DEBUG
    fprintf(stderr, "            .. length=%f, dist=%f\n", length, dist);
#endif
    again = TRUE;
    for (nt = number_of_shots, length = dist, ncp = cur_arc->point,
            px = cur_trans->center.x, py = cur_trans->center.y,
            lcp = NULL, seg_l = 0.0;
            nt--; length = dist) {
#ifdef DEBUG
        fprintf(stderr, "              .. point number %d\n", number_of_shots - nt);
#endif
        while (again && seg_l <= length) {
            if (ncp == NULL) {
#ifdef DEBUG
                fprintf(stderr, "               .. last segment\n");
#endif
                cur_place = cur_arc->place;
                nx = cur_place->center.x;
                ny = cur_place->center.y;
                again = FALSE;
            }
            else {
#ifdef DEBUG
                fprintf(stderr, "               .. next segment\n");
#endif
                nx = ncp->x;
                ny = ncp->y;
                ncp = ncp->next;
            }
            dirx = nx - px;
            diry = ny - py;
            if ((seg_l = dirx * dirx + diry * diry) > 0.0) {
                if ((ll = seg_l = sqrt(seg_l)) <= length) {
#ifdef DEBUG
                    fprintf(stderr, "               .. change segment\n");
#endif
                    length -= seg_l;
                    seg_l = 0.0;
                    px = nx;
                    py = ny;
                }
            }
        }
#ifdef DEBUG
        fprintf(stderr, "              .. segment found\n");
#endif
        acp = get_coo_struct();
        if (lcp == NULL) {
            arc_p->coo_l = lcp = acp;
            acp->next = NULL;
        }
        else if (to_delete) {
            acp->next = arc_p->coo_l;
            arc_p->coo_l = acp;
        }
        else {
            lcp->next = acp;
            acp->next = NULL;
            lcp = acp;
        }
        if (length > 0.0) {
            ss = length / ll;
            seg_l -= length;
            acp->x = (px += dirx * ss);
            acp->y = (py += diry * ss);
        }
        else {
            acp->x = px;
            acp->y = py;
        }
        ss = 3.51 / ll;
        if (dirx < 0.0 || (dirx == 0.0 && diry < 0.0)) {
            acp->x -= ss * diry;
            acp->y += ss * dirx;
        }
        else {
            acp->x += ss * diry;
            acp->y -= ss * dirx;
        }
#ifdef DEBUG
        fprintf(stderr, "              .. point added %f %f\n", px, py);
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "          end of add_arc_moving\n");
#endif
}


void show_one_shot(int             n) {
    struct arc_token *arc_p;
    struct coordinate *coo_p;

#ifdef DEBUG
    fprintf(stderr, "          start of show_one_shot (n=%d)\n", n);
#endif
    if ((arc_p = arc_l) == NULL) {
#ifdef DEBUG
        fprintf(stderr, "          end of show_one_shot (NO ARCS)\n");
#endif
        return;
    }
#ifdef DEBUG
    fprintf(stderr, "            scanning arcs\n");
#endif
    for (; arc_p != NULL; arc_p = arc_p->next) {
        coo_p = arc_p->coo_l;
        if (n < number_of_shots) {
            DrawTokens((int)(arc_p->num_t), (coo_p->x) * zoom_level,
                       /*	(coo_p->y) * zoom_level, CLEAR, token_diameter + 2); */
                       (coo_p->y) * zoom_level, XOR, token_diameter + 2);
#ifdef DEBUG
            fprintf(stderr, "            tokens erased\n");
#endif
            arc_p->coo_l = coo_p->next;
            coo_p->next = coo_free;
            coo_free = coo_p;
            coo_p = arc_p->coo_l;
        }
        if (n) {
            DrawTokens((int)(arc_p->num_t), (coo_p->x) * zoom_level,
                       /*	(coo_p->y) * zoom_level, SET, token_diameter + 2); */
                       (coo_p->y) * zoom_level, XOR, token_diameter + 2);
#ifdef DEBUG
            fprintf(stderr, "            tokens drawn\n");
#endif
        }
        if (!n && arc_p->next == NULL) {
            arc_p->next = arc_free;
            arc_free = arc_l;
            arc_l = NULL;
#ifdef DEBUG
            fprintf(stderr, "          end of show_one_shot\n");
#endif
            return;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "          end of show_one_shot\n");
#endif
}


#define FORW 1
#define BACK 2


void do_fire(struct trans_object *t_p) {
    int             ns;


#ifdef DEBUG
    fprintf(stderr, "    start of do_fire\n");
#endif
    dehighlight_trans();
    highlight_trans(t_p);
    highlight_entranslist();
    XSync(mainDS.display, FALSE);
    usleep(100000);
    set_no_shots();
    reset_entranslist();
#ifdef DEBUG
    fprintf(stderr, "      .. transition flashed (first time)\n");
#endif
    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = cur_arc->next) {
        if (cur_arc->trans == t_p) {
            if (direction == FORW) {
                if (cur_arc->type == TO_TRANS)
                    add_arc_moving(cur_arc, TRUE);
                else if (cur_arc->type == TO_PLACE &&
                         IsLayerListVisible(cur_arc->layer))
                    add_arc_moving(cur_arc, FALSE);
            }
            else {
                if (cur_arc->type == TO_PLACE)
                    add_arc_moving(cur_arc, TRUE);
                else if (cur_arc->type == TO_TRANS &&
                         IsLayerListVisible(cur_arc->layer))
                    add_arc_moving(cur_arc, FALSE);
            }
        }
    }
#ifdef DEBUG
    fprintf(stderr, "      .. input arcs identified\n");
#endif
    highlight_entranslist();
    XSync(mainDS.display, FALSE);
    usleep(100000);

    reset_entranslist();
    for (ns = number_of_shots + 1; ns--;) {
        show_one_shot(ns);
        XSync(mainDS.display, FALSE);
        if (ns)
            usleep(20000);
    }

    reset_entranslist();

    for (cur_arc = netobj->arcs; cur_arc != NULL; cur_arc = cur_arc->next) {
        if (cur_arc->trans == t_p) {
            if (direction == FORW) {
                if (cur_arc->type == TO_PLACE) {
                    if (IsLayerListVisible(cur_arc->place->layer))
                        DrawMarking(cur_arc->place, XOR);
                    (cur_arc->place->tokens) += (ABS(cur_arc->mult));
                    if (IsLayerListVisible(cur_arc->place->layer)) {
                        DrawMarking(cur_arc->place, XOR);
                        highlight_place(cur_arc->place);
                    }
                }
            }
            else {
                if (cur_arc->type == TO_TRANS) {
                    if (IsLayerListVisible(cur_arc->place->layer))
                        DrawMarking(cur_arc->place, XOR);
                    (cur_arc->place->tokens) += (ABS(cur_arc->mult));
                    if (IsLayerListVisible(cur_arc->place->layer)) {
                        DrawMarking(cur_arc->place, XOR);
                        highlight_place(cur_arc->place);
                    }
                }
            }
        }
    }
    highlight_entranslist();
    XSync(mainDS.display, FALSE);
    usleep(100000);

    reset_entranslist();
#ifdef DEBUG
    fprintf(stderr, "    end of do_fire\n");
#endif
}
