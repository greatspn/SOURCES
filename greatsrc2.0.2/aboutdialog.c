#define ___ABOUTDIALOG___
#include "global.h"
#include "aboutdialog.h"

static Widget about_dialog, about_net_pix, about_net_frame1, about_net_frame2;


void ShowAboutDialog(void) {
    XtManageChild(about_dialog);
}

static void ShowFaceEH(Widget w, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch) {
    static short counter = 0;

    counter++;
    if (counter == 2) {
        XtUnmapWidget(about_net_frame1);
        XtMapWidget(about_net_frame2);
    }
}

void InitAboutDialog(void) {
    about_dialog = XtNameToWidget(frame_w, "*AboutDialog");
    about_net_frame1 = XtNameToWidget(about_dialog, "*AboutNetFrame1");
    about_net_frame2 = XtNameToWidget(about_dialog, "*AboutNetFrame2");
    about_net_pix = XtNameToWidget(about_net_frame1, "*AboutNetPixmap");

    XtAddEventHandler(about_net_pix, ButtonReleaseMask, FALSE, (XtEventHandler)ShowFaceEH, NULL);
}
