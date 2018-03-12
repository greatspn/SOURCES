#define ___COLOR___
#	include "global.h"
#	include "draw.h"
#	include "showgdi.h"
#	include "arcdialog.h"
#	include "placedialog.h"
#	include "transdialog.h"
#	include "mark.h"
#	include "lisp.h"
#include "color.h"

static struct net_object *near;
static int      object;
static char movingLispTag[2 * TAG_SIZE + 2];

void ShowPlaceColor(struct place_object *place, DrawingSurfacePun ds, float x_off, float y_off, int op) {
    float           xloc, yloc;

    if (place->color == NULL && place->lisp == NULL)
        return;
    xloc = (place->center.x + place->colpos.x - x_off) * zoom_level;
    yloc = (place->center.y + place->colpos.y - y_off) * zoom_level;
    if (place->color != NULL)
        gdiDrawText(ds, (int)(xloc), (int)(yloc),	op, canvas_font, place->color);
    else
        gdiDrawText(ds, (int)(xloc), (int)(yloc),	op, canvas_font, place->lisp->tag);
}

void ShowTransColor(struct trans_object *trans, DrawingSurfacePun ds, float x_off, float y_off, int op) {
    float xloc, yloc;

    if (trans->color == NULL && trans->lisp == NULL)
        return;
    xloc = (trans->center.x + trans->colpos.x - x_off) * zoom_level;
    yloc = (trans->center.y + trans->colpos.y - y_off) * zoom_level;
    if (trans->color != NULL)
        gdiDrawText(ds, (int)(xloc), (int)(yloc), op, canvas_font, trans->color);
    else
        gdiDrawText(ds, (int)(xloc), (int)(yloc), op, canvas_font, trans->lisp->tag);
}


void DeleteLisp(XButtonEvent *ie) {
    float           x_loc, y_loc;
    int             object;
    struct net_object *near;
    struct arc_object *last_arc;
    struct group_object *group;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if ((near = near_obj(x_loc, y_loc, &object)) == NULL
            || object != LISP) {
        return;
    }
    free_bkup_netobj();
    SetModify();
    bkup_netobj[0] = create_net();
    bkup_netobj[0]->lisps = near->lisps;
    RemoveLisp(near->lisps);
    near->lisps->next = NULL;
    ShowLisp(near->lisps, CLEAR);
    for (last_place = netobj->places; last_place != NULL;
            last_place = last_place->next)
        if (last_place->cmark == near->lisps) {
            ShowPlaceMark(last_place, CLEAR);
            last_place->cmark = NULL;
        }
    for (last_place = netobj->places; last_place != NULL;
            last_place = last_place->next)
        if (last_place->lisp == near->lisps) {
            ShowPlaceColor(last_place, &mainDS, 0.0, 0.0, CLEAR);
            last_place->lisp = NULL;
        }
    for (last_trans = netobj->trans; last_trans != NULL;
            last_trans = last_trans->next)
        if (last_trans->lisp == near->lisps) {
            ShowTransColor(last_trans, &mainDS, 0.0, 0.0, CLEAR);
            last_trans->lisp = NULL;
        }
    for (group = netobj->groups; group != NULL; group = group->next)
        for (last_trans = group->trans; last_trans != NULL;
                last_trans = last_trans->next)
            if (last_trans->lisp == near->lisps) {
                ShowTransColor(last_trans, &mainDS, 0.0, 0.0, CLEAR);
                last_trans->lisp = NULL;
            }
    for (last_arc = netobj->arcs; last_arc != NULL;
            last_arc = last_arc->next)
        if (last_arc->lisp == near->lisps) {
            DrawArc(last_arc, CLEAR);
            last_arc->lisp = NULL;
            DrawArc(last_arc, OR);
        }
}

void RemoveLisp(struct lisp_object *lisp) {
    struct lisp_object *last_lisp;

    if (netobj->lisps == lisp) {
        netobj->lisps = lisp->next;
        return;
    }
    for (last_lisp = netobj->lisps; last_lisp->next != NULL;
            last_lisp = last_lisp->next)
        if (last_lisp->next == lisp) {
            last_lisp->next = lisp->next;
            return;
        }
}

static void ColorTracking(int x, int y) {

    switch (object) {
    case PLACE:
        if (near->places->color == NULL && near->places->lisp == NULL)
            return;
        if (near->places->color != NULL)
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->places->color);
        else
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->places->lisp->tag);
        fix_x = x;
        fix_y = y;
        if (near->places->color != NULL)
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->places->color);
        else
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->places->lisp->tag);
        break;
    case IMTRANS:
    case EXTRANS:
    case DETRANS:
        if (near->trans->color == NULL && near->trans->lisp == NULL)
            return;
        if (near->trans->color != NULL)
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->trans->color);
        else
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->trans->lisp->tag);
        fix_x = x;
        fix_y = y;
        if (near->trans->color != NULL)
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->trans->color);
        else
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->trans->lisp->tag);
        break;
    case IO_ARC:
    case INH_ARC:
        if (near->arcs->color == NULL && near->arcs->lisp == NULL)
            return;
        if (near->arcs->color != NULL)
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->arcs->color);
        else
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->arcs->lisp->tag);
        fix_x = x;
        fix_y = y;
        if (near->arcs->color != NULL)
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->arcs->color);
        else
            gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, near->arcs->lisp->tag);
        break;
    case LISP:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, movingLispTag);
        fix_x = x;
        fix_y = y;
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, movingLispTag);
        break;
    default:
        break;
    }
}

static void DropColor(int x, int y) {
    float  xloc, yloc;
    struct coordinate *point;
    char   *sss;

    xloc = x / zoom_level;
    yloc = y / zoom_level;

    switch (object) {
    case PLACE:
        sss =	((near->places->color != NULL) ?
                 (near->places->color) :
                 (near->places->lisp->tag));
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, sss);
        near->places->colpos.x = xloc - near->places->center.x;
        near->places->colpos.y = yloc - near->places->center.y;
        gdiDrawText(&mainDS, x, y, XOR, canvas_font, sss);
        break;
    case IMTRANS:
    case EXTRANS:
    case DETRANS:
        sss = ((near->trans->color != NULL) ?
               (near->trans->color) :
               (near->trans->lisp->tag));
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, sss);
        near->trans->colpos.x = xloc - near->trans->center.x;
        near->trans->colpos.y = yloc - near->trans->center.y;
        gdiDrawText(&mainDS, x, y, XOR, canvas_font, sss);
        break;
    case IO_ARC:
    case INH_ARC:
        sss =	((near->arcs->color != NULL) ?
                 (near->arcs->color) :
                 (near->arcs->lisp->tag));
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, sss);
        for (point = near->arcs->point; point->next->next != NULL; point = point->next);
        near->arcs->colpos.x = xloc - (point->x + point->next->x) / 2;
        near->arcs->colpos.y = yloc - (point->y + point->next->y) / 2;
        gdiDrawText(&mainDS, x, y, XOR, canvas_font, sss);
        break;
    case LISP:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, movingLispTag);
        near->lisps->center.x = xloc;
        near->lisps->center.y = yloc;
        gdiDrawText(&mainDS, x, y, XOR, canvas_font, movingLispTag);
        break;
    default:
        break;
    }
    reset_canvas_reader();
    menu_action();
}

void MoveColor(XButtonEvent  *ie) {
    float           xloc, yloc;
    struct coordinate *point;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    near = near_obj(xloc, yloc, &object);
    if ((near == NULL) || ((object != PLACE) && (object != DETRANS) && (object != IMTRANS) && (object != EXTRANS) && (object != IO_ARC) && (object != INH_ARC) && (object != LISP))) {
        return;
    }
    switch (object) {
    case PLACE:
        if (near->places->color == NULL && near->places->lisp == NULL)
            return;
        fix_x = (near->places->center.x + near->places->colpos.x) * zoom_level;
        fix_y = (near->places->center.y + near->places->colpos.y) * zoom_level;
        SetMouseHelp("DROP the Place Color", NULL, NULL);
        break;
    case IMTRANS:
    case EXTRANS:
    case DETRANS:
        if (near->trans->color == NULL && near->trans->lisp == NULL)
            return;
        fix_x = (near->trans->center.x + near->trans->colpos.x) * zoom_level;
        fix_y = (near->trans->center.y + near->trans->colpos.y) * zoom_level;
        SetMouseHelp("DROP the Transition Color", NULL, NULL);
        break;
    case IO_ARC:
    case INH_ARC:
        if (near->arcs->color == NULL && near->arcs->lisp == NULL)
            return;
        for (point = near->arcs->point; point->next->next != NULL;
                point = point->next);
        fix_x = ((point->x + point->next->x) / 2 + near->arcs->colpos.x) * zoom_level;
        fix_y = ((point->y + point->next->y) / 2 + near->arcs->colpos.y) * zoom_level;
        SetMouseHelp("DROP the Arc Color", NULL, NULL);
        break;
    case LISP:
        sprintf(movingLispTag, "%s:%c", near->lisps->tag, near->lisps->type);

        fix_x = near->lisps->center.x * zoom_level;
        fix_y = near->lisps->center.y * zoom_level;
        SetMouseHelp("DROP the Color Definition", NULL, NULL);

    default:
        break;
    }
    move = ColorTracking;
    left = DropColor;
    set_canvas_tracker();
}

void EditColor(XButtonEvent *ie) {
    float           xloc, yloc;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    if ((near = near_obj(xloc, yloc, &object)) == NULL
            || (object != PLACE
                && object != DETRANS && object != IMTRANS && object != EXTRANS
                && object != IO_ARC && object != INH_ARC && object != LISP))
        return;
    if (object == LISP) {
        EditLisp(ie);
        return;
    }
    switch (object) {
    case IO_ARC:
    case INH_ARC:
        ShowArcChangeDialog(near->arcs, 1);
        break;
    case PLACE:
        ShowPlaceChangeDialog(near->places, 1);
        break;
    case EXTRANS:
    case DETRANS:
    case IMTRANS:
        ShowTransChangeDialog(near->trans, 1);
        break;


    }
}


