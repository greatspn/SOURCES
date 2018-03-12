#define ___TAG___
#	include "global.h"
#	include "showgdi.h"
#	include "draw.h"
#	include "transdialog.h"
#	include "placedialog.h"
#	include "markdialog.h"
#	include "ratedialog.h"
#	include "resultdialog.h"
#	include "colordialog.h"
#include "tag.h"


static struct net_object *near;
static int      object;


void ShowTag(int op) {
    struct group_object *group;

    for (last_place = netobj->places; last_place != NULL; last_place = last_place->next)
        if (IsLayerListVisible(last_place->layer))
            ShowPlaceTag(last_place, &mainDS, 0.0, 0.0, op);
    for (last_trans = netobj->trans; last_trans != NULL; last_trans = last_trans->next)
        if (IsLayerListVisible(last_trans->layer))
            ShowTransTag(last_trans, &mainDS, 0.0, 0.0, op);
    for (group = netobj->groups; group != NULL; group = group->next)
        for (last_trans = group->trans; last_trans != NULL; last_trans = last_trans->next)
            if (IsLayerListVisible(last_trans->layer))
                ShowTransTag(last_trans, &mainDS, 0.0, 0.0, op);

}

void ShowPlaceTag(struct place_object *place, DrawingSurfacePun ds, float x_off, float y_off, int op) {
    float           xloc, yloc;

    xloc = (place->center.x + place->tagpos.x - x_off) * zoom_level;
    yloc = (place->center.y + place->tagpos.y - y_off) * zoom_level;
    gdiDrawText(ds, (int)(xloc), (int)(yloc), op, canvas_font, place->tag);
}

void ShowTransTag(struct trans_object *trans, DrawingSurfacePun ds, float x_off, float y_off, int op) {
    float           xloc, yloc;

    xloc = (trans->center.x + trans->tagpos.x - x_off) * zoom_level;
    yloc = (trans->center.y + trans->tagpos.y - y_off) * zoom_level;

    gdiDrawText(ds, (int)(xloc), (int)(yloc), op, canvas_font, trans->tag);
}


static void TagTracking(int x, int y) {

    switch (object) {
    case PLACE:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                    XOR, canvas_font, near->places->tag);
        fix_x = x;
        fix_y = y;
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                    XOR, canvas_font, near->places->tag);
        break;
    case IMTRANS:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                    XOR, canvas_font, near->trans->tag);
        if (near->trans->kind > 1) {
            ShowTransPri(fix_x, fix_y, (int) near->trans->kind, &mainDS, XOR);
            ShowTransPri((float) x, (float) y,
                         (int) near->trans->kind, &mainDS, XOR);
        }
        fix_x = x;
        fix_y = y;
        gdiDrawText(&mainDS, x, y, XOR, canvas_font, near->trans->tag);
        break;
    case EXTRANS:
    case DETRANS:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                    XOR, canvas_font, near->trans->tag);
        fix_x = x;
        fix_y = y;
        gdiDrawText(&mainDS, x, y, XOR, canvas_font, near->trans->tag);
        break;
    default:
        break;
    }
}

static void DropTag(int x, int y) {
    float           xloc, yloc;

    xloc = x / zoom_level;
    yloc = y / zoom_level;
    switch (object) {
    case PLACE:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                    XOR, canvas_font, near->places->tag);
        near->places->tagpos.x = xloc - near->places->center.x;
        near->places->tagpos.y = yloc - near->places->center.y;
        gdiDrawText(&mainDS, x, y,
                    XOR, canvas_font, near->places->tag);
        break;
    case IMTRANS:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                    XOR, canvas_font, near->trans->tag);
        near->trans->tagpos.x = xloc - near->trans->center.x;
        near->trans->tagpos.y = yloc - near->trans->center.y;
        gdiDrawText(&mainDS, x, y,
                    XOR, canvas_font, near->trans->tag);
        if (near->trans->kind > 1) {
            ShowTransPri((float) near->trans->tagpos.x,
                         (float) near->trans->tagpos.y,
                         (int) near->trans->kind, &mainDS, XOR);
        }
        break;
    case EXTRANS:
    case DETRANS:
        gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                    XOR, canvas_font, near->trans->tag);
        near->trans->tagpos.x = xloc - near->trans->center.x;
        near->trans->tagpos.y = yloc - near->trans->center.y;
        gdiDrawText(&mainDS, x, y,
                    XOR, canvas_font, near->trans->tag);
        break;
    default:
        break;
    }
    reset_canvas_reader();
    menu_action();

}

void MoveTag(XButtonEvent   *ie) {
    float           xloc, yloc;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    if ((near = near_obj(xloc, yloc, &object)) == NULL
            || (object != PLACE
                && object != DETRANS && object != IMTRANS && object != EXTRANS))
        return;
    switch (object) {
    case PLACE:
        fix_x = (near->places->center.x + near->places->tagpos.x) * zoom_level;
        fix_y = (near->places->center.y + near->places->tagpos.y) * zoom_level;
        break;
    case IMTRANS:
    case EXTRANS:
    case DETRANS:
        fix_x = (near->trans->center.x + near->trans->tagpos.x) * zoom_level;
        fix_y = (near->trans->center.y + near->trans->tagpos.y) * zoom_level;
        break;
    default:
        break;
    }
    move = TagTracking;
    left = DropTag;
    put_msg(0, "DROP the tag with LEFT button");
    set_canvas_tracker();
}

void EditTag(XButtonEvent   *ie) {
    float           xloc, yloc;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    if ((near = near_obj(xloc, yloc, &object)) == NULL
            || (object != PLACE
                && object != DETRANS && object != IMTRANS && object != EXTRANS
                && object != MPAR && object != RPAR && object != RESULT
                && object != LISP))
        return;
    switch (object) {
    case PLACE:
        ShowPlaceChangeDialog(near->places, 0);
        break;
    case DETRANS:
    case IMTRANS:
    case EXTRANS:
        ShowTransChangeDialog(near->trans, 0);
        break;
    case MPAR:
        ShowMarkChangeDialog(near->mpars, 0);
        break;
    case RPAR:
        ShowRateChangeDialog(near->rpars, 0);
        break;
    case RESULT:
        ShowResChangeDialog(near->results, 0);
        break;
    case LISP:
        ShowColorChangeDialog(near->lisps, 0);
        break;
    }
}
