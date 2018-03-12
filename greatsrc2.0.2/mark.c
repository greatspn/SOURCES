#define ___MARK___
#	include "global.h"
#	include "draw.h"
#	include "showgdi.h"
#	include "markdialog.h"
#	include "placedialog.h"
#	include "res.h"
#include "mark.h"

struct net_object *near;
static int             object;
static char movingMParString[2 * TAG_SIZE + 2];

void ShowMpar(struct mpar_object *mpar, int op) {
    char            string[2 * TAG_SIZE + 2];
    float           xloc, yloc;

    if (!IsLayerListVisible(mpar->layer))
        return;
    xloc = mpar->center.x * zoom_level;
    yloc = mpar->center.y * zoom_level;
    sprintf(string, "%s=%3d", mpar->tag, mpar->value);
    gdiDrawText(&mainDS, (int) xloc, (int) yloc, op, canvas_font, string);
}


void DeleteMpar(XButtonEvent *ie) {
    float           x_loc, y_loc;
    int             object;
    struct net_object *near;
    struct place_object *place;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if ((near = near_obj(x_loc, y_loc, &object)) == NULL
            || object != MPAR) {
        return;
    }
    ShowMpar(near->mpars, CLEAR);
    if (!IsLayerVisible(WHOLE_NET_LAYER)) {
        ResetVisibleLayers(near->mpars->layer);
        return;
    }
    free_bkup_netobj();
    SetModify();
    bkup_netobj[0] = create_net();
    for (place = netobj->places; place != NULL; place = place->next)
        if (place->mpar == near->mpars) {
            ShowPlaceMark(place, CLEAR);
            place->mpar = NULL;
            place->m0 = 0;
            place->tokens = 0;
            ClearRes();
        }
    bkup_netobj[0]->mpars = near->mpars;
    RemoveMpar(near->mpars);
    near->mpars->next = NULL;
}

void RemoveMpar(struct mpar_object *mpar) {
    struct mpar_object *last_mpar;

    if (netobj->mpars == mpar) {
        netobj->mpars = mpar->next;
        return;
    }
    for (last_mpar = netobj->mpars; last_mpar->next != NULL;
            last_mpar = last_mpar->next)
        if (last_mpar->next == mpar) {
            last_mpar->next = mpar->next;
            return;
        }
}
void ShowPlaceMark(struct place_object *place, int mode) {
    float           xloc, yloc;

    if (!IsLayerListVisible(place->layer))
        return;
    if (place->cmark != NULL) {
        xloc = place->center.x * zoom_level - place_radius / 2;
        yloc = place->center.y * zoom_level + place_radius / 2 + 2;
        gdiDrawText(&mainDS, (int) xloc, (int) yloc, mode, canvas_font,
                    place->cmark->tag);
    }
    else if (place->tokens >= 0)
        DrawMarking(place, mode);
    else {
        xloc = place->center.x * zoom_level - place_radius / 2;
        yloc = place->center.y * zoom_level + place_radius / 2 + 2;
        gdiDrawText(&mainDS, (int) xloc, (int) yloc, mode,
                    canvas_font, place->mpar->tag);
    }
}

void NewMpar(XButtonEvent   *ie) {

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) != NULL)
        return;

    ShowMarkChangeDialog(NULL, 0);

}

void EditMarking(XButtonEvent   *ie) {
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) == NULL
            || (object != PLACE && object != MPAR))
        return;

    switch (object) {
    case MPAR:
        ShowMarkChangeDialog(near->mpars, 2);
        break;
    case PLACE:
        ShowPlaceChangeDialog(near->places, 2);
        break;
    }
}

static void MParTracking(int x, int y) {

    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                XOR, canvas_font, movingMParString);
    fix_x = x;
    fix_y = y;
    gdiDrawText(&mainDS, x, y ,
                XOR, canvas_font, movingMParString);
}

static void DropMPar(int x, int y) {
    float           xloc, yloc;

    xloc = x / zoom_level;
    yloc = y / zoom_level;
    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                XOR, canvas_font, movingMParString);
    near->mpars->center.x = xloc;
    near->mpars->center.y = yloc;
    ShowMpar(near->mpars, OR);
    menu_action();
}

void MoveMPar(XButtonEvent *ie) {
    float           xloc, yloc;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    if ((near = near_obj(xloc, yloc, &object)) == NULL
            || (object !=  MPAR))
        return;
    fix_x = near->mpars->center.x * zoom_level;
    fix_y = near->mpars->center.y * zoom_level;

    sprintf(movingMParString, "%s=%3d", near->mpars->tag, near->mpars->value);

    move = MParTracking;
    left = DropMPar;
    middle = null_proc;
    SetMouseHelp("DROP the Marking Parameter", NULL, NULL);
    set_canvas_tracker();
}
