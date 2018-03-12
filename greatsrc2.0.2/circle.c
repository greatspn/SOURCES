#define ___CIRCLE___
#	include "global.h"
#	include "postscript.h"
#include "circle.h"

void Circle(float center_x, float center_y, float rad, int op, DrawingSurfacePun ds) {
    int             ulc_x = center_x - rad;
    int             ulc_y = center_y - rad;
    int             diam = rad + rad;


    SetFunction(op, ds);
    if (!PSflag) {
        XDrawArc(ds->display, ds->drawable, ds->gc, ulc_x, ulc_y, diam, diam, 0, 23040 /* 360*64 */);
    }
    else
        PSCircle(center_x, center_y, rad);
}

void Dot(int x, int y, int op, int diam) {

    if (PSflag) {
        PSToken((float)x, (float)y, (float)diam);
        return;
    }
    SetFunction(op, &mainDS);


    if (diam <= 1) {
        XDrawPoint(mainDS.display, mainDS.drawable, mainDS.gc, x, y);

        return;
    }
    if (diam == 2) {
        XDrawPoint(mainDS.display, mainDS.drawable, mainDS.gc, x, y - 1);
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x - 1, y, x + 1, y);
        return;
    }
    if (diam == 3) {
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x, y + 1, x + 1, y + 1);
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x - 1, y, x + 1, y);
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x - 1, y - 1, x, y - 1);
        return;
    }
    if (diam == 4) {
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x, y + 1, x + 1, y + 1);
        XFillRectangle(mainDS.display, mainDS.drawable, mainDS.gc, x - 1, y - 1, 4, 2);
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x, y - 2, x + 1, y - 2);

        return;
    }
    else {
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x - 1, y + 2, x + 1, y + 2);
        XFillRectangle(mainDS.display, mainDS.drawable, mainDS.gc, x - 2, y - 1, 5, 3);
        XDrawLine(mainDS.display, mainDS.drawable, mainDS.gc, x - 1, y - 2, x + 1, y - 2);

    }
}
