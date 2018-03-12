#define ___OVERVIEW___
#	include <X11/cursorfont.h>
#	include <Mrm/MrmPublic.h>
#	include "global.h"
#	include "chop.h"
#include "overview.h"
#include "showgdi.h"


static Boolean init_drag;
Window Xoverview, Xshadow, Xarea;
extern Widget drawwin, sb1, sb2, workwin;
static Display *display;
static Window drawable;
static int rx, ry, rw, rh;
int cx, cy;
int cw, ch;
static	GC wgc;
static Boolean dragging;
static DrawingSurface overDS;


static Widget over_dlg, over_draw;


void DrawOverPlace(struct place_object *place, int op) {
    SetFunction(op, &overDS);
    over_draw_place(place, overDS.gc, overDS.drawable);
}
/*********************** !!!!!!!!!!!!!!!!!!!!!!! **********************/
int over_draw_place(struct place_object *place, GC gc, Window drawable) {
    int     xd = (int)(place->center.x / 5);
    int     yd = (int)(place->center.y / 5);

    XDrawLine(display, drawable, gc, xd + 1, yd + 2, xd - 1, yd + 2);
    XDrawLine(display, drawable, gc, xd + 1, yd - 2, xd - 1, yd - 2);
    XDrawLine(display, drawable, gc, xd - 2, yd + 1, xd - 2, yd - 1);
    XDrawLine(display, drawable, gc, xd + 2, yd + 1, xd + 2, yd - 1);
    return 0;
}


void DrawOverTrans(struct trans_object *trans, int op) {
    SetFunction(op, &overDS);
    over_draw_trans(trans, overDS.gc, overDS.drawable);
}

/*********************** !!!!!!!!!!!!!!!!!!!!!!! **********************/

int over_draw_trans(struct trans_object *trans, GC gc, Window drawable) {
    float           x1, y1, x2, y2;
    float           xc, yc;

    xc = trans->center.x / 5;
    yc = trans->center.y / 5;
    FNormTrans(trans, trans->center.x / 5, trans->center.y / 5, &xc, &yc);

    if (trans->kind != EXPONENTIAL && trans->kind != DETERMINISTIC) {
        RNormTrans(trans, xc - 2, yc, &x1, &y1);
        RNormTrans(trans, xc + 2, yc, &x2, &y2);
        XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
    }
    else {

        RNormTrans(trans, xc - 2, yc - 1, &x1, &y1);
        RNormTrans(trans, xc + 2, yc - 1, &x2, &y2);
        XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
        if (trans->kind == EXPONENTIAL) {
            RNormTrans(trans, xc - 2, yc, &x1, &y1);
            RNormTrans(trans, xc - 2, yc, &x2, &y2);
            XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
            RNormTrans(trans, xc + 2, yc, &x1, &y1);
            RNormTrans(trans, xc + 2, yc, &x2, &y2);
            XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
        }
        else {
            RNormTrans(trans, xc - 2.5, yc - 1, &x1, &y1);
            RNormTrans(trans, xc + 1.5, yc - 1, &x2, &y2);
            XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
            RNormTrans(trans, xc - 2, yc, &x1, &y1);
            RNormTrans(trans, xc + 2, yc, &x2, &y2);
            XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
            RNormTrans(trans, xc - 2.5, yc, &x1, &y1);
            RNormTrans(trans, xc + 1.5, yc, &x2, &y2);
            XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
            RNormTrans(trans, xc - 2.5, yc + 1, &x1, &y1);
            RNormTrans(trans, xc + 1.5, yc + 1, &x2, &y2);
            XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
        }
        RNormTrans(trans, xc - 2, yc + 1, &x1, &y1);
        RNormTrans(trans, xc + 2, yc + 1, &x2, &y2);
        XDrawLine(display, drawable, gc, (int) x1, (int) y1, (int) x2, (int) y2);
    }
    return 0;
}



void over_draw_net(struct net_object *net, GC gc, Window drawable) {
    struct place_object *last_place;
    struct trans_object *last_trans;
    struct group_object *last_group;

    for (last_place = net->places; last_place != NULL;
            last_place = last_place->next) {
        if (IsLayerListVisible(last_place->layer))
            over_draw_place(last_place, gc, drawable);
    }
    for (last_trans = net->trans; last_trans != NULL;
            last_trans = last_trans->next)
        if (IsLayerListVisible(last_trans->layer))
            over_draw_trans(last_trans, gc, drawable);


    for (last_group = net->groups; last_group != NULL;
            last_group = last_group->next)
        for (last_trans = last_group->trans; last_trans != NULL;
                last_trans = last_trans->next)
            if (IsLayerListVisible(last_trans->layer))
                over_draw_trans(last_trans, gc, drawable);

}

static void CalcRect(void) {
    float fcx, fcy, fcw, fch;
    Arg args[4];
    Widget clipwin;
    XWindowAttributes	attr;


    XtSetArg(args[0], XmNclipWindow, &clipwin);
    XtGetValues(workwin, args, 1);

    XGetWindowAttributes(XtDisplay(clipwin), XtWindow(clipwin), &attr);
    cw = attr.width;
    ch = attr.height;

    fcw = (float)cw;
    fch = (float)ch;

    rw = (int)(fcw / (5.*zoom_level));
    rh = (int)(fch / (5.*zoom_level));

    XtSetArg(args[0], XmNvalue, &cx);
    XtGetValues(sb1, args, 1);
    XtSetArg(args[0], XmNvalue, &cy);
    XtGetValues(sb2, args, 1);
    fcx = (float)cx;
    fcy = (float)cy;

    rx = (int)(fcx / (5.*zoom_level));
    ry = (int)(fcy / (5.*zoom_level));

}


void ShowOverview(void) {
    XWindowChanges conf;
    XSetWindowAttributes	sattr;

    CalcRect();

    conf.x = cw / 2 - 200 + cx;
    conf.y = ch / 2 - 120 + cy;
    XConfigureWindow(display, Xoverview, CWX | CWY , &conf);
    conf.x += 8;
    conf.y += 8;
    XConfigureWindow(display, Xshadow, CWX | CWY , &conf);
    sattr.background_pixel = gWhite;
    XChangeWindowAttributes(display, Xshadow, CWBackPixel, &sattr);

    XMapWindow(display, Xshadow);
    XMapWindow(display, Xoverview);

    sattr.background_pixel = gBlack;
    XChangeWindowAttributes(display, Xshadow, CWBackPixel, &sattr);
    XClearWindow(display, Xshadow);

    wgc = XCreateGC(display, Xoverview, 0, NULL);
    XSetBackground(display, wgc, gWhite);
    XSetForeground(display, wgc, gBlack);
    /*	XSetPlaneMask(display,wgc,1l);*/
    XSetFunction(display, wgc, gBlack ? GXor : GXand);

    over_draw_net(netobj, wgc, Xoverview);

    XSetFunction(display, wgc, gBlack ? GXxor : GXequiv);
    XDrawRectangle(display, Xoverview, wgc, rx, ry, rw, rh);
}

static void InitDragCB(Widget w, int closure, XmScrollBarCallbackStruct *call_data) {
    int orx, ory;

    if (!init_drag) {
        orx = rx;
        ory = ry;
        init_drag = TRUE;
        SetFunction(XOR, &overDS);
        ShowOverview();
        XFlush(overDS.display);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, orx, ory, rw, rh);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
        XFlush(overDS.display);
    }
    if (init_drag) {
        float fc;
        int nr;

        orx = rx;
        ory = ry;

        fc = (float)call_data->value;
        switch (closure) {
        case X_SCROLL:
            nr = (int)(fc / (5.*zoom_level));
            XFlush(display);
            XDrawRectangle(display, Xoverview, wgc, rx, ry, rw, rh);
            XDrawRectangle(display, Xoverview, wgc, nr, ry, rw, rh);
            XFlush(display);
            rx = nr;
            break;
        case Y_SCROLL:
            nr = (int)(fc / (5.*zoom_level));
            XFlush(display);
            XDrawRectangle(display, Xoverview, wgc, rx, ry, rw, rh);
            XDrawRectangle(display, Xoverview, wgc, rx, nr, rw, rh);
            XFlush(display);
            ry = nr;
            break;
        }
        XFlush(overDS.display);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, orx, ory, rw, rh);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
        XFlush(overDS.display);
    }

}


static void EndDragCB(Widget w, XtPointer closure, XtPointer call_data) {
    XSetWindowAttributes	sattr;
    if (init_drag) {
        init_drag = FALSE;
        XFreeGC(display, wgc);
        sattr.background_pixel = gWhite;
        XChangeWindowAttributes(display, Xshadow, CWBackPixel, &sattr);
        XClearWindow(display, Xshadow);

        XUnmapWindow(display, Xoverview);
        XUnmapWindow(display, Xshadow);
        SetFunction(OR, &overDS);
    }
}

static void OverSynchCB(Widget w, int closure, XmScrollBarCallbackStruct *call_data) {
    float fc;
    int nr;

    fc = (float)call_data->value;
    SetFunction(XOR, &overDS);
    switch (closure) {
    case X_SCROLL:
        nr = (int)(fc / (5.*zoom_level));
        XFlush(overDS.display);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, nr, ry, rw, rh);
        XFlush(overDS.display);
        rx = nr;
        break;
    case Y_SCROLL:
        nr = (int)(fc / (5.*zoom_level));
        XFlush(overDS.display);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
        XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, nr, rw, rh);
        XFlush(overDS.display);
        ry = nr;
        break;
    }
    SetFunction(OR, &overDS);

}

static void OverResizeEH(Widget w, XtPointer closure, XConfigureEvent *ev, Boolean *continue_to_dispatch) {
    if (ev->type != ConfigureNotify)
        return;

    SetFunction(XOR, &overDS);
    XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
    rw = (int)(ev->width / (5.*zoom_level));
    rh = (int)(ev->height / (5.*zoom_level));
    XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
    SetFunction(OR, &overDS);
}


void InitOverview(void) {
    XSetWindowAttributes	attr;
    Arg args[1];
    Widget clipwin;
    Cursor fleur;

    display = XtDisplay(drawwin);
    drawable = XtWindow(drawwin);

    attr.save_under = TRUE;
    Xshadow = XCreateSimpleWindow(display, drawable, 0, 0, 400, 240, 1, gBlack, gWhite);
    Xoverview = XCreateSimpleWindow(display, drawable, 0, 0, 400, 240, 1, gBlack, gWhite);
    XChangeWindowAttributes(display, Xshadow, CWSaveUnder, &attr);
    XChangeWindowAttributes(display, Xoverview, CWSaveUnder, &attr);

    init_drag = FALSE;
    XtAddCallback(sb1, XmNdragCallback, (XtCallbackProc)InitDragCB, X_SCROLL);
    XtAddCallback(sb1, XmNvalueChangedCallback, (XtCallbackProc)EndDragCB, NULL);
    XtAddCallback(sb2, XmNdragCallback, (XtCallbackProc)InitDragCB, (XtPointer)Y_SCROLL);
    XtAddCallback(sb2, XmNvalueChangedCallback, (XtCallbackProc)EndDragCB, NULL);

    XtAddCallback(sb1, XmNpageIncrementCallback, (XtCallbackProc)OverSynchCB, X_SCROLL);
    XtAddCallback(sb1, XmNpageDecrementCallback, (XtCallbackProc)OverSynchCB, X_SCROLL);
    XtAddCallback(sb1, XmNincrementCallback, (XtCallbackProc)OverSynchCB, X_SCROLL);
    XtAddCallback(sb1, XmNdecrementCallback, (XtCallbackProc)OverSynchCB, X_SCROLL);
    XtAddCallback(sb2, XmNpageIncrementCallback, (XtCallbackProc)OverSynchCB, (XtPointer)Y_SCROLL);
    XtAddCallback(sb2, XmNpageDecrementCallback, (XtCallbackProc)OverSynchCB, (XtPointer)Y_SCROLL);
    XtAddCallback(sb2, XmNincrementCallback, (XtCallbackProc)OverSynchCB, (XtPointer)Y_SCROLL);
    XtAddCallback(sb2, XmNdecrementCallback, (XtCallbackProc)OverSynchCB, (XtPointer)Y_SCROLL);

    InitDS(over_draw, &overDS);

    XSetBackground(overDS.display, overDS.gc, gWhite);
    XSetForeground(overDS.display, overDS.gc, gBlack);
    /*	XSetPlaneMask(overDS.display,overDS.gc,1l);*/
    SetFunction(OR, &overDS);

    XtSetArg(args[0], XmNclipWindow, &clipwin);
    XtGetValues(workwin, args, 1);

    XtAddEventHandler(clipwin, StructureNotifyMask, FALSE, (XtEventHandler)OverResizeEH, NULL);

    fleur = XCreateFontCursor(overDS.display, XC_fleur);
    XDefineCursor(overDS.display, overDS.drawable, fleur);
}

static Boolean IsInDragRect(int x, int y) {
    return (x >= rx && x <= (rx + rw) && y >= ry && y <= (ry + rh));
}


static void OverHandler(Widget w, XtPointer closure, XEvent *ev, Boolean *continue_to_dispatch) {
    int x, y;
    static int xdb, ydb;

    switch (ev->type) {
    case ButtonPress:
        x = ev->xbutton.x;
        y = ev->xbutton.y;
        if (IsInDragRect(x, y)) {
            dragging = TRUE;
            xdb = x;
            ydb = y;
            SetFunction(XOR, &overDS);
        }
        break;
    case ButtonRelease: {
        int vr, sr, ir, pi;

        x = ev->xbutton.x;
        y = ev->xbutton.y;
        dragging = FALSE;
        SetFunction(OR, &overDS);
        XmScrollBarGetValues(sb1, &vr, &sr, &ir, &pi);
        vr = (int)(((float)rx) * (5.*zoom_level));
        XmScrollBarSetValues(sb1, vr, sr, ir, pi, TRUE);
        XmScrollBarGetValues(sb2, &vr, &sr, &ir, &pi);
        vr = (int)(((float)ry) * (5.*zoom_level));
        XmScrollBarSetValues(sb2, vr, sr, ir, pi, TRUE);
        break;
    }
    case MotionNotify:
        x = ev->xmotion.x;
        y = ev->xmotion.y;
        if (dragging) {
            int	newx, newy;
            newx = rx + (x - xdb);
            newy = ry + (y - ydb);

            XDrawRectangle(display, overDS.drawable, overDS.gc, rx, ry, rw, rh);

            if ((newx + rw) < 400 && newx >= 0)
                rx += (x - xdb);
            if ((newy + rh) < 240 && newy >= 0)
                ry += (y - ydb);

            XDrawRectangle(display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
            xdb = x;
            ydb = y;
        }
        break;
    }

}

void ClearOverview(void) {
    CalcRect();
    XClearWindow(overDS.display, overDS.drawable);
    XDrawRectangle(overDS.display, overDS.drawable, overDS.gc, rx, ry, rw, rh);
}

void RedisplayOverview(DrawingSurfacePun ds) {
    CalcRect();
    XClearWindow(ds->display, ds->drawable);
    SetFunction(OR, ds);
    over_draw_net(netobj, ds->gc, ds->drawable);
    SetFunction(XOR, ds);
    XDrawRectangle(ds->display, ds->drawable, ds->gc, rx, ry, rw, rh);
    SetFunction(OR, ds);
}

static void RedrawOverviewCB(Widget w, DrawingSurfacePun closure, XmDrawingAreaCallbackStruct *call_data) {
    if (((XExposeEvent *)call_data->event)->count != 0)
        return;
    RedisplayOverview(closure);
}

void ShowOverviewDialog(void) {
    XtMapWidget(XtParent(over_dlg));
}

void HideOverviewDialog(void) {
    XtUnmapWidget(XtParent(over_dlg));
}

void RegisterOverviewDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"RedrawOverviewCB"		, (XtPointer)RedrawOverviewCB},
        {"OVERVIEW_ADDRESS"		, (XtPointer) &overDS}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterOverviewDialog names\n");
}

void InitOverviewDialog(void) {
    over_dlg = XtNameToWidget(frame_w, "*OverDialog");
    over_draw = XtNameToWidget(over_dlg, "*OverDialog_Draw");
    XtMapWidget(over_dlg);
    InitOverview();
    XtAddCallback(over_draw, XmNexposeCallback, (XtCallbackProc) RedrawOverviewCB, &overDS);
    XtAddEventHandler(over_draw, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, FALSE, OverHandler, NULL);
}


