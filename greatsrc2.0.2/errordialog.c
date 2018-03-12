
#include "global.h"

static void KillErrorDlgCB(Widget w, XtPointer closure, XtPointer call_data) {
    XtDestroyWidget(XtParent(w));
}

void  ShowErrorDialog(char *message, Widget parent) {
    Arg args[1];
    XmString string;
    Widget temp;
    Widget errorDialog;

    errorDialog = FetchWidget(parent, "ErrorDialog");
    temp = XmMessageBoxGetChild(errorDialog, XmDIALOG_OK_BUTTON);
    XtAddCallback(temp, XmNactivateCallback, KillErrorDlgCB, NULL);

    string = XmStringCreateLtoR(message, XmFONTLIST_DEFAULT_TAG);
    XtSetArg(args[0], XmNmessageString, string);
    XtSetValues(errorDialog, args, 1);
    XmStringFree(string);
    XtManageChild(errorDialog);
}
