#define ___PRINTAREA___
#	include "global.h"
#	include "box.h"
#	include "draw.h"
#	include "showgdi.h"
#	include "arcdialog.h"
#	include "placedialog.h"
#	include "transdialog.h"
#	include "mark.h"
#	include "lisp.h"

static float ex = 144., ey = 144., sx = 0., sy = 0.;

static void (* oldm)(), (*oldl)(), (*oldmi)();
static Boolean printarea_visible;

Boolean IsPrintAreaVisible(void) {
    return printarea_visible;
}

void GetTruePrintArea(float *a1, float *a2, float *a3, float *a4) {
    *a1 = sx;
    *a2 = sy;
    *a3 = ex - sx;
    *a4 = ey - sy;
}

void GetPrintArea(int *a1, int *a2, int *a3, int *a4) {
    *a1 = (int)(sx / 5.);
    *a2 = (int)(sy / 5.);
    *a3 = (int)((ex - sx) / 5.);
    *a4 = (int)((ey - sy) / 5.);
}

void SetPrintArea(float a1, float a2, float a3, float a4) {
    if (printarea_visible)
        DrawRectBox(sx * zoom_level, sy * zoom_level, ex * zoom_level, ey * zoom_level, XOR, &mainDS);
    sx = a1;
    sy = a2;
    ex = a3 + a1;
    ey = a4 + a2;
    if (printarea_visible)
        DrawRectBox(sx * zoom_level, sy * zoom_level, ex * zoom_level, ey * zoom_level, XOR, &mainDS);

}
void DrawPrintArea(void) {
    if (!printarea_visible)
        return;

    DrawRectBox(sx * zoom_level, sy * zoom_level, ex * zoom_level, ey * zoom_level, OR, &mainDS);
}

void SetPrintAreaVisible(Boolean v) {
    if (printarea_visible != v)
        DrawRectBox(sx * zoom_level, sy * zoom_level, ex * zoom_level, ey * zoom_level, XOR, &mainDS);
    printarea_visible = v;
}

void EndPrintArea(int x, int y) {
    float tmpx, tmpy;

    oldm = move;
    oldl = left;
    oldmi = middle;
    move = left = middle = null_proc;
    DrawRectBox(fix_x, fix_y, cur_x, cur_y, XOR, &mainDS);
    sx = fix_x / zoom_level;
    sy = fix_y / zoom_level;
    ex = cur_x / zoom_level;
    ey = cur_y / zoom_level;

    tmpx = MIN(sx, ex);
    tmpy = MIN(sy, ey);
    ex = MAX(sx, ex);
    ey = MAX(sy, ey);
    sx = tmpx;
    sy = tmpy;
    printf("sx=%f  sy=%f  ex=%f  ey=%f\n", sx, sy, ex, ey);
    DrawRectBox(sx * zoom_level, sy * zoom_level, ex * zoom_level, ey * zoom_level, OR, &mainDS);

    printarea_visible = TRUE;
    reset_canvas_reader();
    update_mode();
    SetMouseHelp("DEFINE Top Left corner", NULL, "ACTIONS");

}


void InitPrintArea(XButtonEvent *ie) {
    oldm = move;
    oldl = left;
    oldmi = middle;

    if (printarea_visible)
        DrawRectBox(sx * zoom_level, sy * zoom_level, ex * zoom_level, ey * zoom_level, XOR, &mainDS);
    cur_x = fix_x = (float)(ie->x);
    cur_y = fix_y = (float)(ie->y);
    move = ElasticBox;
    left = middle = EndPrintArea;
    SetMouseHelp("DEFINE Right Bottom corner", NULL, NULL);
    set_canvas_tracker();
}
