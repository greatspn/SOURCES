#define ___TOOLKIT___
#include "global.h"
#include "cursorsbits.h"
#include <X11/cursorfont.h>
#include <Mrm/MrmPublic.h>
#include "GREAT_C_REVISION"

static XColor          XBlack, XWhite;
static MrmHierarchy	s_MrmHierarchy;

static char		*vec[] = {
    "GreatSPN.uid"
};

static MrmType		class ;
static Cursor cursors[18], waitCursor, currentCursor = 0;

static void CreaCursore(Cursor *n, char *d, int x, int y, int w, int h) {
    Window          drawable;
    Pixmap          si_xx;
    /*
    drawable = XtWindow(drawwin);
    */

    drawable = RootWindow(gDisplay, DefaultScreen(gDisplay));
    si_xx = XCreatePixmapFromBitmapData(gDisplay, drawable, d, w, h, 1, 0, 1);

    if (!si_xx)
        XtError("can'create cursor Pixmap\n");

    *n = XCreatePixmapCursor(gDisplay, si_xx, si_xx, &XBlack, &XWhite, x, y);
    /*
    if (temp == NULL)
    	XtError("can'create cursor\n");
    */
}

void SetWaitCursor(Widget w) {
    XUndefineCursor(gDisplay, XtWindow(drawwin));
    XDefineCursor(gDisplay, XtWindow(mainwin), waitCursor);
    if (w != mainwin) {
        XDefineCursor(XtDisplay(w), XtWindow(w), waitCursor);
    }
    XFlush(gDisplay);
}

void ClearWaitCursor(Widget w) {

    XUndefineCursor(gDisplay, XtWindow(mainwin));
    XDefineCursor(gDisplay, XtWindow(drawwin), currentCursor);
    if (w != mainwin) {
        XUndefineCursor(XtDisplay(w), XtWindow(w));
    }
    XFlush(gDisplay);
}


void SetCursor(int f) {
    currentCursor = cursors[f];

    XDefineCursor(XtDisplay(drawwin), XtWindow(drawwin), currentCursor);
}

void InitCursors(void) {
    XBlack.pixel = gBlack;
    XWhite.pixel = gWhite;

    CreaCursore(&cursors[WAIT_CURSOR], wait_cursor_bits, wait_cursor_x_hot, wait_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[PLACE_CURSOR], place_cursor_bits, place_cursor_x_hot, place_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);

    CreaCursore(&cursors[HIMTRANS_CURSOR], h_imtrans_cursor_bits, h_imtrans_cursor_x_hot, h_imtrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[VIMTRANS_CURSOR], v_imtrans_cursor_bits, v_imtrans_cursor_x_hot, v_imtrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[FIMTRANS_CURSOR], f_imtrans_cursor_bits, f_imtrans_cursor_x_hot, f_imtrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[MIMTRANS_CURSOR], m_imtrans_cursor_bits, m_imtrans_cursor_x_hot, m_imtrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);

    CreaCursore(&cursors[HEXTRANS_CURSOR], h_extrans_cursor_bits, h_extrans_cursor_x_hot, h_extrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[VEXTRANS_CURSOR], v_extrans_cursor_bits, v_extrans_cursor_x_hot, v_extrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[FEXTRANS_CURSOR], f_extrans_cursor_bits, f_extrans_cursor_x_hot, f_extrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[MEXTRANS_CURSOR], m_extrans_cursor_bits, m_extrans_cursor_x_hot, m_extrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);

    CreaCursore(&cursors[HDETRANS_CURSOR], h_detrans_cursor_bits, h_detrans_cursor_x_hot, h_detrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[VDETRANS_CURSOR], v_detrans_cursor_bits, v_detrans_cursor_x_hot, v_detrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[FDETRANS_CURSOR], f_detrans_cursor_bits, f_detrans_cursor_x_hot, f_detrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[MDETRANS_CURSOR], m_detrans_cursor_bits, m_detrans_cursor_x_hot, m_detrans_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);

    CreaCursore(&cursors[POINTER_CURSOR], pointer_cursor_bits, pointer_cursor_x_hot, pointer_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);
    CreaCursore(&cursors[CROSS_CURSOR], cross_cursor_bits, cross_cursor_x_hot, cross_cursor_y_hot, CURSOR_WIDTH, CURSOR_HEIGHT);

    waitCursor = XCreateFontCursor(XtDisplay(drawwin), XC_watch);
}

void SetBackground(Widget w, String cname) {
    XrmValue    from, to;
    Arg		arg;

    from.addr = cname;
    from.size = strlen(cname);
    to.addr   = NULL;
    to.size   = 0;

    XtConvert(w, XmRString, &from, XmRPixel, &to);
    if (to.addr != NULL) {
        XtSetArg(arg, XmNbackground, *(Pixel *)to.addr);
        XtSetValues(w, &arg, 1);
    }
}


void SetForeground(Widget w, String cname) {
    XrmValue    from, to;
    Arg		arg;

    from.addr = cname;
    from.size = strlen(cname);
    to.addr   = NULL;
    to.size   = 0;

    XtConvert(w, XmRString, &from, XmRPixel, &to);
    if (to.addr != NULL) {
        XtSetArg(arg, XmNforeground, *(Pixel *)to.addr);
        XtSetValues(w, &arg, 1);
    }
}


void OpenHierarchy(void) {
#if defined(Linux)
    extern Widget  appshell;

    if (MrmOpenHierarchyPerDisplay(
                XtDisplay(appshell),
                1,
                vec,
                NULL,
                &s_MrmHierarchy)
            != MrmSUCCESS)
#else
    if (MrmOpenHierarchy(1,
                         vec,
                         NULL,
                         &s_MrmHierarchy)
            != MrmSUCCESS)
#endif
    {
        XtError("can't open hierarchy\n");
    }
}

Widget FetchWidget(Widget father, String name) {
    Widget	fdWidget;
    char message[300];

    if (MrmFetchWidget(s_MrmHierarchy,
                       name,
                       father,
                       &fdWidget,
                       &class)
            != MrmSUCCESS) {
        sprintf(message, "Sorry cannot fetch %s widget\n", name);
        XtError(message);
    }

    return fdWidget;
}

void RegisterNames(MrmRegisterArg *regvec, int regnum) {
    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterNames names\n");
}

void SetApplicationTitle(char *title) {
    Arg args[1];
    char buffer[128];

    sprintf(buffer, "GreatSPN %s : ", PROGRAM_REVISION);
    strncat(buffer, title, sizeof(buffer) - strlen(buffer));

    XtSetArg(args[0], XmNtitle, buffer);
    XtSetValues(appshell, args, 1);
}
