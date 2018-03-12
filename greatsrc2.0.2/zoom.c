#define ___ZOOM___
#include "global.h"
#include "showgdi.h"
#include "menuvalues.h"
#include "menuinit.h"
#include "zoom.h"
#include "Canvas.h"
#include <Mrm/MrmPublic.h>


/*
#define DEBUG
 */

void ResetZoom(void) {
    SetZoom(ZOOM_1_0);
#ifdef	DEBUG
    fprintf(stderr, "    ResetZoom: calling SetZoomMenuItemValue(TRUE)\n");
#endif
    SetZoomMenuItemValue(TRUE);
}

static void SetZoom(int level) {
#ifdef	DEBUG
    fprintf(stderr, "  Start set_zoom\n");
#endif
    SetWaitCursor(frame_w);
#ifdef	DEBUG
    fprintf(stderr, "     ... set_zoom switch level=%d\n", level);
#endif
    switch (level) {
    case ZOOM_0_5:
        zoom_level = 0.5;
        place_radius = 3.5;
        trans_length = 7;
        titrans_height = 3;
        imtrans_height = 2;
        token_diameter = 1;
        arrowht = 3;
        arrowwid = 2;
        not_radius = 1;
        canvas_width = 1000;
        canvas_height = 600;
        canvas_font = sail_font;
        z_coef = 0.4;
        break;
    case ZOOM_0_75:
        zoom_level = 0.75;
        place_radius = 5.25;
        trans_length = 10.5;
        titrans_height = 4;
        imtrans_height = 3;
        token_diameter = 2;
        arrowht = 4;
        arrowwid = 3;
        not_radius = 1.5;
        canvas_width = 1500;
        canvas_height = 900;
        canvas_font = sail_font;
        z_coef = 0.266667;
        break;
    case ZOOM_1_5:
        zoom_level = 1.5;
        place_radius = 10.5;
        trans_length = 21;
        titrans_height = 9;
        imtrans_height = 6;
        token_diameter = 3;
        arrowht = 9;
        arrowwid = 6;
        not_radius = 3;
        canvas_width = 3000;
        canvas_height = 1800;
        canvas_font = cour_font;
        z_coef = 0.133333;
        break;
    case ZOOM_2_0:
        zoom_level = 2.0;
        place_radius = 14;
        trans_length = 28;
        titrans_height = 12;
        imtrans_height = 8;
        token_diameter = 4;
        arrowht = 12;
        arrowwid = 8;
        not_radius = 4;
        canvas_width = 4000;
        canvas_height = 2400;
        canvas_font = gachab_font;
        z_coef = 0.1;
        break;
    default:
        zoom_level = 1.0;
        place_radius = 7;
        trans_length = 14;
        titrans_height = 6;
        imtrans_height = 4;
        token_diameter = 3;
        arrowht = 6;
        arrowwid = 4;
        not_radius = 2;
        canvas_width = 2000;
        canvas_height = 1200;
        canvas_font = cour_font;
        z_coef = 0.2;
        break;
    }

#ifdef	DEBUG
    fprintf(stderr, "     ... set_zoom:  calling reset_canvas_scroll\n");
#endif
    reset_canvas_scroll();
#ifdef	DEBUG
    fprintf(stderr, "     ... set_zoom:  calling gdiPreparePlaceModel\n");
#endif
    gdiPreparePlaceModel();
#ifdef	DEBUG
    fprintf(stderr, "     ... set_zoom:  calling gdiPrepareTransModels\n");
#endif
    gdiPrepareTransModels();
#ifdef	DEBUG
    fprintf(stderr, "     ... set_zoom:  calling gdiResizeDrawingArea\n");
#endif
    gdiResizeDrawingArea(canvas_width, canvas_height);
    if (moving_images != NULL) {
        /*  redisplay_canvas();
        	zoom_selected_images(); */
    }
#ifdef	DEBUG
    fprintf(stderr, "     ... set_zoom:  calling ClearWaitCursor\n");
#endif
    ClearWaitCursor(frame_w);
#ifdef	DEBUG
    fprintf(stderr, "  End set_zoom\n");
#endif
}


void GetPixmap(Pixmap *pxpp, DrawingSurfacePun ds, int xc, int yc, int w, int h) {
    /*	Display * display;
    Drawable drawable;
    GC gc;
    XGCValues gc_val;

    gc_val.function = GXcopy;
    display = XtDisplay(ds);
    drawable = XtWindow(ds);
    */
    if (*pxpp != (Pixmap)NULL)
        XFreePixmap(ds->display, *pxpp);

    *pxpp = XCreatePixmap(ds->display, ds->drawable, w, h, gDepth);

    SetFunction(COPY, ds);
    XSetForeground(ds->display, ds->gc, gWhite);
    XFillRectangle(ds->display, *pxpp, ds->gc, 0, 0, w, h);
    XSetForeground(ds->display, ds->gc, gBlack);
    XFlush(ds->display);
    XCopyArea(ds->display, ds->drawable, *pxpp, ds->gc, xc, yc, w, h, 0, 0);
    XFlush(ds->display);
    /*   XCopyArea(ds->display,*pxpp, ds->drawable, ds->gc, 0, 0, w, h, 50, 50);*/
    XFlush(ds->display);

}

void RegisterZooomMenuCallbacks(void) {
    static MrmRegisterArg	regvec[] = {
        {"ZoomCB"	, (XtPointer)ZoomCB}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterZooomMenuCallbacks names\n");
}

static void ZoomCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    int             z_l;

    if (receiving_msg || inib_flag || !call_data->set)
        return;
    if (action_on) {
        StatusDisplay(cant_interrupt);
        /*
        if (bell_global_on);
        window_bell(menu_sw);*/
        return;
    }
    switch (closure) {
    case ZOOM_1_MITEM:
        z_l = ZOOM_1_0;
        break;
    case ZOOM_3_2_MITEM:
        z_l = ZOOM_1_5;
        break;
    case ZOOM_2_MITEM:
        z_l = ZOOM_2_0;
        break;
    case ZOOM_1_2_MITEM:
        z_l = ZOOM_0_5;
        break;
    case ZOOM_3_4_MITEM:
        z_l = ZOOM_0_75;
        break;
    default:
        z_l = ZOOM_1_0;
        break;
    }

    SetZoom(z_l);
    if (moving_images == NULL) {
        gdiClearDrawingArea();
        redisplay_canvas();
    }
}





