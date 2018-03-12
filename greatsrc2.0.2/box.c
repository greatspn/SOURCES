#define ___BOX___
#include "global.h"
#include "showgdi.h"
#include "box.h"

void InitBoxTracking(XButtonEvent *ie) {

    prev_x = cur_x;
    prev_y = cur_y;
    cur_x = fix_x = (float)(ie->x);
    cur_y = fix_y = (float)(ie->y);
    move = ElasticBox;
    left = middle = end_boxtracking;
    DrawRectBox(fix_x, fix_y, cur_x, cur_y, XOR, &mainDS);
    set_canvas_tracker();

}

void DrawRectBox(float x1, float y1, float x2, float y2, int op, DrawingSurfacePun ds) {
    int             ix1 = (int)(x1);
    int             iy1 = (int)(y1);
    int             ix2 = (int)(x2);
    int             iy2 = (int)(y2);

    SetFunction(op, ds);
    XSetLineAttributes(ds->display, ds->gc, 1, LineOnOffDash, CapButt, JoinMiter);
    XDrawLine(ds->display, ds->drawable, ds->gc, ix1, iy1, ix1, iy2);
    XDrawLine(ds->display, ds->drawable, ds->gc, ix1 + 1, iy2, ix2 - 1, iy2);
    XDrawLine(ds->display, ds->drawable, ds->gc, ix2, iy2, ix2, iy1);
    XDrawLine(ds->display, ds->drawable, ds->gc, ix2 - 1, iy1, ix1 + 1, iy1);
    XSetLineAttributes(ds->display, ds->gc, 1, LineSolid, CapButt, JoinMiter);
    XFlush(ds->display);

}

void ElasticBox(int x, int y) {
    DrawRectBox(fix_x, fix_y, cur_x, cur_y, INVERT, &mainDS);
    cur_x = (float) x;
    cur_y = (float) y;
    DrawRectBox(fix_x, fix_y, cur_x, cur_y, INVERT, &mainDS);
}
