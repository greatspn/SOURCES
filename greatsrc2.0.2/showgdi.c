#define ___SHOWGDI___
#	include "global.h"
#	include "postscript.h"
#	include "overview.h"
#include "showgdi.h"

/*
#define DEBUG
 */

static Pixmap circle_pixmap = (Pixmap) NULL ;
static Pixmap imtranspix[4] = {(Pixmap)NULL, (Pixmap)NULL, (Pixmap)NULL, (Pixmap)NULL};
static Pixmap extranspix[4] = {(Pixmap)NULL, (Pixmap)NULL, (Pixmap)NULL, (Pixmap)NULL};
static Pixmap detranspix[4] = {(Pixmap)NULL, (Pixmap)NULL, (Pixmap)NULL, (Pixmap)NULL};
int pixdim;
static int XORfunction, CLEARfunction, ORfunction;

void InitDS(Widget w, DrawingSurfacePun dsp) {
    dsp->drawable = (Drawable)  XtWindow(w);
    dsp->display = (Display *)XtDisplay(w);
    dsp->gc = XCreateGC(dsp->display, dsp->drawable, 0, NULL);
    /*   XSetPlaneMask(dsp->display,dsp->gc,1);   */
    dsp->widget = w;
}

void DisposeDS(DrawingSurfacePun dsp) {
    XFreeGC(dsp->display, dsp->gc);
}

void gdiInitEngine(Widget appshell) {
    gDisplay = XtDisplay(appshell);
    gScreen = XDefaultScreenOfDisplay(gDisplay);
    gDepth = XDefaultDepthOfScreen(gScreen);

    gWhite = WhitePixelOfScreen(gScreen);
    gBlack = BlackPixelOfScreen(gScreen);

    if (gBlack) {
        XORfunction = GXxor;
        CLEARfunction = GXclear;
        ORfunction = GXor;
    }
    else {
        XORfunction = GXequiv;
        CLEARfunction = GXset;
        ORfunction = GXand;
    }
}

void gdiInitDrawingAreaGC(void) {
    InitDS(drawwin, &mainDS);

    /*	XSetForeground(mainDS.display,mainDS.gc,gBlack ^ gWhite);*/
    XSetForeground(mainDS.display, mainDS.gc, gBlack);
    XSetBackground(mainDS.display, mainDS.gc, gWhite);
#ifdef DEBUG_GC
    printf("\ngdiInitDrawingAreaGC : display %x screen %x  drawable %x  Black %d   White %d\n", mainDS.display, screen, mainDS.drawable, gBlack, gWhite);
#endif
}


void SetFunction(int op, DrawingSurfacePun ds) {
    switch (op) {
    case OR:
        XSetForeground(ds->display, ds->gc, gBlack);
        XSetFunction(ds->display, ds->gc, ORfunction);
        break;
    case CLEAR:
        XSetFunction(ds->display, ds->gc, CLEARfunction);
        break;
    case SET:
    case COPY:
        XSetForeground(ds->display, ds->gc, gBlack);
        XSetFunction(ds->display, ds->gc, GXcopy);
        break;
    case XOR:
        XSetFunction(ds->display, ds->gc, XORfunction);
        break;
    case INVERT:
        XSetFunction(ds->display, ds->gc, GXinvert);
        break;
    default:
        printf("\n SetFunction %d not yet implemented \n", op);

    }
}

void gdiDrawText(DrawingSurfacePun ds, int x, int  y, int  op, int  font, char *string) {
    XGCValues values;

    if (!PSflag) {
        SetFunction(op, ds);
        values.font = font;
        XChangeGC(ds->display, ds->gc, GCFont, &values);
        XDrawString(ds->display, ds->drawable, ds->gc, x, y, string, strlen(string));
    }
    else {
        PSText((float)x, (float)y, string);
    }
}


void gdiDrawPoint(DrawingSurfacePun ds, int x, int y, int op) {
    SetFunction(op, ds);
    XDrawPoint(ds->display, ds->drawable, ds->gc, x, y);
}


/* ?????????????????????????????????? */
// static conta_volte = 0;
// #define MAXARGS 20
//	static Arg arglist[MAXARGS];
/* ?????????????????????????????????? */

#ifdef NON_COMPILARE
void MWSet(va_alist)
va_dcl {
    /* #define MAXARGS 20 */

    va_list args;
    Widget w;
    int numargs;
    int i;
    /*	static Arg arglist[MAXARGS]; ??????????????? */

    va_start(args);
    numargs = va_arg(args, int);
#ifdef  DEBUG
    fprintf(stderr, "           ... MWSet:  numargs=%d\n", numargs);
#endif
    if (numargs > MAXARGS) {
        char temp[60];


        sprintf(temp, "\n\nWSet: Warning too many args (max %d)\nExceding arguments ignored\n"
        , MAXARGS);
        perror(temp);
        numargs = MAXARGS;
    }

    w = va_arg(args, Widget);

    for (i = 0; i < numargs; i++) {
        String   s = va_arg(args, String);
        XtArgVal x = va_arg(args, XtArgVal);

#ifdef  DEBUG
        fprintf(stderr, "           ... MWSet:  loop  i=%d\n", i);
#endif
        XtSetArg(arglist[i], s, x);
    }
    va_end(args);
    if (conta_volte++ > 5) {
#ifdef  DEBUG
        fprintf(stderr, "           ... MWSet:  calling  XtSetValues, w=%d\n",
        (int)w);
#endif
        /*	XtSetValues(w,arglist,numargs); */
        XtSetValues(w, arglist, numargs);
    }
#ifdef  DEBUG
    fprintf(stderr, "           End MWSet\n");
#endif

}
#endif /* NON_COMPILARE */


void gdiResizeDrawingArea(int width, int height) {
    Arg args[2];

#ifdef  DEBUG
    fprintf(stderr, "        ... gdiResizeDrawingArea:  calling MWSet  (drawwin=%p)\n",  drawwin);
#endif

    XtSetArg(args[0], XmNheight, height);
    XtSetArg(args[1], XmNwidth, width);
    XtSetValues(drawwin, args, 2);

    /*
      MWSet(2,drawwin,XmNwidth,width,XmNheight,height,NULL);
    */
#ifdef  DEBUG
    fprintf(stderr, "        End of gdiResizeDrawingArea\n");
#endif
}


void gdiClearDrawingArea(void) {
#ifdef DEBUG
    fprintf(stderr, "   ...gdiClearDrawingArea \n");
    fflush(stderr);
#endif // DEBUG

    XClearWindow(mainDS.display, mainDS.drawable);
    ClearOverview();
}

//void gdiClearRect(Widget pw,int dx,int dy,int w,int h)
void gdiClearRect(DrawingSurfacePun pw, int dx, int dy, int w, int h) {
//	GC              lgc;
//	Drawable  ldrawable;
//	Display * ldisplay;


#ifdef DEBUG
    fprintf(stderr, "   ...gdiClearRect  creating gc\n");
    fflush(stderr);
#endif // DEBUG
//	ldrawable = (Drawable)   XtWindow(pw);
//	ldisplay  =  (Display *) XtDisplay(pw);

//	lgc = XCreateGC(ldisplay, ldrawable,0,NULL);

//	XSetForeground(ldisplay,lgc,gWhite);
    XSetForeground(pw->display, pw->gc, gWhite);
    /*	XSetPlaneMask(ldisplay,lgc,1); */

//	XFillRectangle(ldisplay, ldrawable, lgc, dx, dy, w, h);
    XFillRectangle(pw->display, pw->drawable, pw->gc, dx, dy, w, h);

//	XFreeGC(ldisplay,lgc);
}


void gdiSetRect(DrawingSurfacePun pw, int dx, int dy, int w, int h) {
    XFillRectangle(pw->display, pw->drawable, pw->gc, dx, dy, w, h);
}

void gdiDrawingAreaClearRect(int dx, int dy, int w, int h) {
#ifdef DEBUG
    fprintf(stderr, "   ...gdiDrawingAreaClearRect  creating gc\n");
    fflush(stderr);
#endif // DEBUG

    SetFunction(CLEAR, &mainDS);
    gdiDrawingAreaFillRect(dx, dy, w, h);

}



void gdiDrawingAreaInvertRect(int dx, int dy, int w, int h) {
    SetFunction(XOR, &mainDS);
    gdiDrawingAreaFillRect(dx, dy, w, h);
}


static void gdiDrawingAreaFillRect(int dx, int dy, int w, int h) {
    XFillRectangle(mainDS.display, mainDS.drawable, mainDS.gc, dx, dy, w, h);
}


void gdiPreparePlaceModel(void) {
    float           xc, yc;
    int             dim, xd, yd, diam;

    place_dec = dim = (int)(place_radius + 1);
    xc = dim * 2 ;
    yc = dim * 2 ;

    xd = (int)(xc - dim);
    yd = (int)(yc - dim);
    dim += dim;
    dim++ ;
    place_dim = dim ;

    diam = (int) place_radius + place_radius;

    if (circle_pixmap != (Pixmap)NULL)
        XFreePixmap(mainDS.display, circle_pixmap);


    SetFunction(CLEAR, &mainDS);
    circle_pixmap = XCreatePixmap(mainDS.display, mainDS.drawable, place_dim, place_dim, gDepth);
    XFillRectangle(mainDS.display, circle_pixmap, mainDS.gc, 0, 0, place_dim, place_dim);
    SetFunction(COPY, &mainDS);

    XDrawArc(mainDS.display, circle_pixmap, mainDS.gc, 1, 1, diam, diam, 0, 23040);


#ifdef DEBUG
    printf("\nplace_dim= %d\ndiam = %d\n xd = %d\nyd = %d\ndiam = %d\n", place_dim, diam, xd, yd, diam);
#endif // DEBUG


}

void gdiRotateVertex(int angle, float tx, float ty, float *x, float *y) {
    switch (angle) {
    case HORIZONTAL:
        *x = tx;
        *y = ty;
        break;
    case FORTYFIVE:
        *x = (tx - ty) * ISQRT2;
        *y = (tx + ty) * ISQRT2;
        break;
    case MINUSFORTYFIVE:
        *x = (tx + ty) * ISQRT2;
        *y = (ty - tx) * ISQRT2;
        break;
    default:
        *x = -ty;
        *y = tx;
        break;
    }
}


void gdiRotatePolygon(short angle, XPoint *points, float cx, float cy, float halflenght, float halfheight) {
    float x1, x2, x3, x4, y1, y2, y3, y4;
#define X1  points[0].x
#define X2  points[1].x
#define X3  points[2].x
#define X4  points[3].x

#define Y1  points[0].y
#define Y2  points[1].y
#define Y3  points[2].y
#define Y4  points[3].y

    gdiRotateVertex(angle, halflenght, halfheight, &x1, &y1);
    gdiRotateVertex(angle, -halflenght, halfheight, &x2, &y2);
    gdiRotateVertex(angle, -halflenght, -halfheight, &x3, &y3);
    gdiRotateVertex(angle, halflenght, -halfheight, &x4, &y4);

    X1 = (short)x1 + cx;
    X2 = (short)x2 + cx;
    X3 = (short)x3 + cx;
    X4 = (short)x4 + cx;

    Y1 = (short)y1 + cy;
    Y2 = (short)y2 + cy;
    Y3 = (short)y3 + cy;
    Y4 = (short)y4 + cy;

#undef X1
#undef X2
#undef X3
#undef X4

#undef Y1
#undef Y2
#undef Y3
#undef Y4

}

void gdiPrepareTransModels(void) {
    static XPoint transpoly[5];
    register int ttype;
    register int i;
    float half_length, tihalf_height, imhalf_height, imcx, imcy, ticx, ticy;

    half_length   = trans_length   / 2.;
    tihalf_height = titrans_height / 2.;
    imhalf_height = imtrans_height / 2.;

    pixdim = 4 + (int)trans_length;

    ticx = ticy = ((float)pixdim) / 2.;
    imcx = imcy = ((float)pixdim) / 2.;


    for (ttype = IMTRANS; ttype <= DETRANS ; ttype++) {
        switch (ttype) {
        case IMTRANS:
            if (imtranspix[HORIZONTAL] != (Pixmap)NULL) {
                XFreePixmap(mainDS.display, imtranspix[HORIZONTAL]);
                XFreePixmap(mainDS.display, imtranspix[VERTICAL]);
                XFreePixmap(mainDS.display, imtranspix[FORTYFIVE]);
                XFreePixmap(mainDS.display, imtranspix[MINUSFORTYFIVE]);
            }

            for (i = HORIZONTAL; i <= MINUSFORTYFIVE; i++) {
                /* Creates & initializes Pixmaps */
                XSetForeground(mainDS.display, mainDS.gc, gWhite);  /* To initialize pixmap */
                imtranspix[i] = XCreatePixmap(mainDS.display, mainDS.drawable, pixdim, pixdim, gDepth);
                XFillRectangle(mainDS.display, imtranspix[i], mainDS.gc, 0, 0, pixdim, pixdim);
                /* Create Vertex ,rotate and write filled polygon to pixmap */
                XSetForeground(mainDS.display, mainDS.gc, gBlack);
                gdiRotatePolygon(i, transpoly, imcx, imcy, half_length, imhalf_height);
                XFillPolygon(mainDS.display, imtranspix[i], mainDS.gc, transpoly, 4, Convex, CoordModeOrigin);
            }
            break;
        case EXTRANS:
            if (extranspix[HORIZONTAL] != (Pixmap)NULL) {
                XFreePixmap(mainDS.display, extranspix[HORIZONTAL]);
                XFreePixmap(mainDS.display, extranspix[VERTICAL]);
                XFreePixmap(mainDS.display, extranspix[FORTYFIVE]);
                XFreePixmap(mainDS.display, extranspix[MINUSFORTYFIVE]);
            }

            for (i = HORIZONTAL; i <= MINUSFORTYFIVE; i++) {
                XSetForeground(mainDS.display, mainDS.gc, gWhite);  /* To initialize pixmap */
                extranspix[i] = XCreatePixmap(mainDS.display, mainDS.drawable, pixdim, pixdim, gDepth);
                XFillRectangle(mainDS.display, extranspix[i], mainDS.gc, 0, 0, pixdim, pixdim);
                /* Create Vertex ,rotate and write polygon to pixmap */
                XSetForeground(mainDS.display, mainDS.gc, gBlack);
                gdiRotatePolygon(i, transpoly, ticx, ticy, half_length, tihalf_height);
                transpoly[4] = transpoly[0];
                XDrawLines(mainDS.display, extranspix[i], mainDS.gc, transpoly, 5, CoordModeOrigin);
            }

            break;
        case DETRANS:
            if (detranspix[HORIZONTAL] != (Pixmap)NULL) {
                XFreePixmap(mainDS.display, detranspix[HORIZONTAL]);
                XFreePixmap(mainDS.display, detranspix[VERTICAL]);
                XFreePixmap(mainDS.display, detranspix[FORTYFIVE]);
                XFreePixmap(mainDS.display, detranspix[MINUSFORTYFIVE]);
            }

            for (i = HORIZONTAL; i <= MINUSFORTYFIVE; i++) {
                XSetForeground(mainDS.display, mainDS.gc, gWhite);
                detranspix[i] = XCreatePixmap(mainDS.display, mainDS.drawable, pixdim, pixdim, gDepth);
                XFillRectangle(mainDS.display, detranspix[i], mainDS.gc, 0, 0, pixdim, pixdim);

                XSetForeground(mainDS.display, mainDS.gc, gBlack);
                gdiRotatePolygon(i, transpoly, ticx, ticy, half_length, tihalf_height);
                XFillPolygon(mainDS.display, detranspix[i], mainDS.gc, transpoly, 4, Convex, CoordModeOrigin);
            }
        }
    }
}

void gdiCopyTransModel(struct trans_object *trans, int xd, int  yd, int op, DrawingSurfacePun ds) {
    Pixmap ToDrawPix;

    SetFunction(op, ds);

    switch (trans->kind) {
    case EXPONENTIAL:
        ToDrawPix = extranspix[trans->orient];
        break;
    case  DETERMINISTIC:
        ToDrawPix = detranspix[trans->orient];
        break;
    default:
        ToDrawPix = imtranspix[trans->orient];
    }

    XCopyArea(ds->display, ToDrawPix, ds->drawable, ds->gc, 0, 0, pixdim, pixdim, xd, yd);
}


void gdiCopyPlaceModel(int xd, int  yd, int op, DrawingSurfacePun ds) {
    SetFunction(op, ds);
    XCopyArea(ds->display, circle_pixmap, ds->drawable, ds->gc, 0, 0, place_dim, place_dim, xd, yd);
}

void gdiXorPlaceModel(int xd, int yd, DrawingSurfacePun ds) {
    SetFunction(XOR, ds);
    XCopyArea(ds->display, circle_pixmap, ds->drawable, ds->gc, 0, 0, place_dim, place_dim, xd, yd);
}


void gdiReleaseGCs(void) {
    DisposeDS(&mainDS);
}

