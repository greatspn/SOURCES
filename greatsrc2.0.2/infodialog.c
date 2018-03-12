#define ___INFODIALOG___
#	include "global.h"
#include "infodialog.h"

static Widget infoDialog;

static void KillInfoDlgCB(Widget w, XtPointer closure, XtPointer call_data) {
    XtDestroyWidget(XtParent(w));
}

void   ShowInfoDialog(String message, Widget parent) {
    Widget temp;
    XmString string;
    Arg args[1];

    infoDialog = FetchWidget(parent, "InfoDialog");
    temp = XmMessageBoxGetChild(infoDialog, XmDIALOG_OK_BUTTON);
    XtAddCallback(temp, XmNactivateCallback, KillInfoDlgCB, NULL);
    string = XmStringCreateLtoR(message, XmFONTLIST_DEFAULT_TAG);
    XtSetArg(args[0], XmNmessageString, string);
    XtSetValues(infoDialog, args, 1);
    XmStringFree(string);
    XtManageChild(infoDialog);
}
