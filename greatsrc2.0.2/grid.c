#define ___GRID___

#include	"global.h"
#include	"showgdi.h"
#include "menuinit.h"
#include	"grid.h"
#include    "menuvalues.h"
#include "Canvas.h"
#include <Mrm/MrmPublic.h>

static int grid_display_flag = FALSE;
static XPoint GridPoints[404];
static int GridRows, GridCols;
static int grila_level = 1;

int GetGridLevel(void) {
    return grila_level;
}

void ResetGrill(void) {
    SetGrill(GRILL_1);
    SetGridMenuItemValue(TRUE);
}


static void SetGrill(int level) {
    int gl;
    register int i;
    switch (level) {
    case GRILL_5 : grill_level =  5; grila_level = 1; break;
    case GRILL_10: grill_level = 10; grila_level = 2; break;
    case GRILL_20: grill_level = 20; grila_level = 4; break;
    case GRILL_30: grill_level = 30; grila_level = 6; break;
    case GRILL_40: grill_level = 40; grila_level = 8; break;
    case GRILL_50: grill_level = 50; grila_level = 10; break;
    default      : grill_level =  grila_level = 1; break;
    }
    gl = grill_level < 10 ? 10 : grill_level;

    GridRows = canvas_height / gl;
    GridCols = canvas_width  / gl;
    for (i = 1 ; i < GridCols; i++)
        GridPoints[i].x = gl;
}

void OLDDisplayGrid(void) {
    register int y, g_l = grill_level;
    Widget clip, sb1, sb2;
    XWindowAttributes	attr;
    Arg args[2];
    Window  win;
    int x1, x2, x3, x4;
    int y1, y2, y3, y4;
    int np, ly, sy;

    if ((!grid_display_flag) || (g_l == 1))
        return;

    if (g_l < 10)
        g_l = 10;

    SetWaitCursor(frame_w);

    SetFunction(COPY, &mainDS);

    XtSetArg(args[0], XmNclipWindow, &clip);
    XtGetValues(workwin, args, 1);
    win = XtWindow(clip);
    XGetWindowAttributes(XtDisplay(clip), win, &attr);

    XtSetArg(args[0], XmNhorizontalScrollBar, &sb1);
    XtSetArg(args[1], XmNverticalScrollBar, &sb2);
    XtGetValues(workwin, args, 2);

    XmScrollBarGetValues(sb1, &x1, &x2, &x3, &x4);
    XmScrollBarGetValues(sb2, &y1, &y2, &y3, &y4);

    np = attr.width / g_l + 1;
    ly = ((y1 + attr.height) / g_l + 1) * g_l;
    sy = (y1 / g_l) * g_l;

    /*	printf("Grid : Width= %d  Height=%d offsetx %d  offsety %d\n",attr.width,attr.height,x1,y1);*/

    GridPoints[0].x = (x1 / g_l) * g_l;

    for (y = sy; y < ly; y += g_l) {
        GridPoints[0].y = y;
        XDrawPoints(mainDS.display, mainDS.drawable, mainDS.gc, GridPoints, np, CoordModePrevious);
    }

    ClearWaitCursor(frame_w);
}

void DisplayGrid(void) {

    Widget clip, sb1, sb2;
    XWindowAttributes	attr;
    Arg args[2];
    Window  win;
    int x1, x2, x3, x4;
    int y1, y2, y3, y4;


    XtSetArg(args[0], XmNclipWindow, &clip);
    XtGetValues(workwin, args, 1);
    win = XtWindow(clip);
    XGetWindowAttributes(XtDisplay(clip), win, &attr);

    XtSetArg(args[0], XmNhorizontalScrollBar, &sb1);
    XtSetArg(args[1], XmNverticalScrollBar, &sb2);
    XtGetValues(workwin, args, 2);

    XmScrollBarGetValues(sb1, &x1, &x2, &x3, &x4);
    XmScrollBarGetValues(sb2, &y1, &y2, &y3, &y4);

    DisplayGridRect(x1, y1, attr.width, attr.height);

}

void DisplayGridRect(int ox, int oy, int w, int h) {
    register int y, g_l = grill_level;
    int np, ly, sy;

    /*	printf("grid_display_flag %d  g_l %d\n",grid_display_flag,g_l);*/

    if ((!grid_display_flag) || (g_l == 1))
        return;

    if (g_l < 10)
        g_l = 10;

    SetWaitCursor(frame_w);

    SetFunction(COPY, &mainDS);

    np = w / g_l + 1;
    ly = ((oy + h) / g_l + 1) * g_l;
    sy = (oy / g_l) * g_l;

    /*printf("Grid : Width= %d  Height=%d offsetx %d  offsety %d\n",w,h,ox,oy);*/

    GridPoints[0].x = (ox / g_l) * g_l;

    for (y = sy; y < ly; y += g_l) {
        GridPoints[0].y = y;
        XDrawPoints(mainDS.display, mainDS.drawable, mainDS.gc, GridPoints, np, CoordModePrevious);
    }
    ClearWaitCursor(frame_w);
}


static void GridCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    int             z_l;

    /*	if (receiving_msg || action_on || !call_data->set)
    {
    	StatusDisplay(cant_interrupt);
    	return;
    }*/

    if (!call_data->set)
        return;

    grid_display_flag = 1;

    switch (closure) {

    case GRID_NONE_MITEM:
        grid_display_flag = 0;
        break;
    case GRID_5_MITEM:
        z_l = GRILL_5;
        break;
    case GRID_10_MITEM:
        z_l = GRILL_10;
        break;
    case GRID_20_MITEM:
        z_l = GRILL_20;
        break;
    case GRID_30_MITEM:
        z_l = GRILL_30;
        break;
    case GRID_40_MITEM:
        z_l = GRILL_40;
        break;
    case GRID_50_MITEM:
        z_l = GRILL_50;
        break;
    default:
        z_l = GRILL_1;
        break;
    }
    SetGrill(z_l);
    gdiClearDrawingArea();
    redisplay_canvas();
}

void RegisterGrid(void) {
    static MrmRegisterArg	regvec[] = {
        {"GridCB", (XtPointer)	GridCB}
    };

    RegisterNames(regvec, sizeof(regvec) / sizeof(MrmRegisterArg));
}
