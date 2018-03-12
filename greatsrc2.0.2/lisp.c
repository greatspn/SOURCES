#define ___LISP___
#	include "global.h"
#	include "colordialog.h"
#	include "showdialog.h"
#	include "showgdi.h"
#include "lisp.h"


void PutLisp(struct lisp_object *lisp, int op, DrawingSurfacePun ds, int x_off, int y_off) {
    char            string[2 * TAG_SIZE + 2];
    float           loc_x, loc_y;

    loc_x = (lisp->center.x - x_off) * zoom_level;
    loc_y = (lisp->center.y - y_off) * zoom_level;
    sprintf(string, "%s:%c", lisp->tag, lisp->type);

    gdiDrawText(ds, (int)loc_x, (int)loc_y, op, canvas_font, string);
}


void ShowLisp(struct lisp_object *lisp, int op) {
    char            string[2 * TAG_SIZE + 2];
    float           loc_x, loc_y;

    loc_x = lisp->center.x * zoom_level;
    loc_y = lisp->center.y * zoom_level;
    sprintf(string, "%s:%c", lisp->tag, lisp->type);

    gdiDrawText(&mainDS, (int)loc_x, (int)loc_y, op, canvas_font,
                string);
}

void TypeLispDef(struct lisp_object *lisp) {
    char            string[10000];

    if (lisp->text == NULL)
        return;
    sprintf(string, "%s : %c = \n%s", lisp->tag, lisp->type, lisp->text);
    ShowShowDialog(string);
}

void TypeLisp(XButtonEvent *ie) {
    struct net_object *near;
    int             object;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) == NULL
            || object != LISP)
        return;
    TypeLispDef(near->lisps);
}

void NewLisp(XButtonEvent *ie) {
    struct net_object *near;
    int             object;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) != NULL)
        return;

    ShowColorChangeDialog(NULL, 0);

}

void EditLisp(XButtonEvent *ie) {
    struct net_object *near;
    int             object;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) == NULL
            || object != LISP)
        return;

    ShowColorChangeDialog(near->lisps, 1);
}
