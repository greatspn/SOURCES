#define ___WARNINGDIALOG___

#include "global.h"

static Widget warningDialog;

static void NullCB(Widget w, XtPointer closure, XtPointer call_data) {
}

void   ShowWarningDialog(char *message, XtCallbackProc CB1, XtCallbackProc CB2, char *Label1, char *Label2, int def) {
    Arg args[4];
    XmString string, okstring, cancelstring;

    static void (*OkCB)() = NULL;
    static void (*CancelCB)() = NULL;

    string = XmStringCreateLtoR(message, XmFONTLIST_DEFAULT_TAG);
    okstring = XmStringCreateLtoR(Label1 == NULL ? "Yes" : Label2, XmFONTLIST_DEFAULT_TAG);
    cancelstring = XmStringCreateLtoR(Label2 == NULL ? "No" : Label1, XmFONTLIST_DEFAULT_TAG);

    if (OkCB != NULL && CancelCB != NULL) {
        XtRemoveCallback(warningDialog, XmNokCallback, OkCB, 0);
        XtRemoveCallback(warningDialog, XmNcancelCallback, CancelCB, 0);
    }

    OkCB = CB1 == NULL ? NullCB : CB1;
    CancelCB = CB2 == NULL ? NullCB : CB2;
    XtAddCallback(warningDialog, XmNokCallback, OkCB, 0);
    XtAddCallback(warningDialog, XmNcancelCallback, CancelCB, 0);

    XtSetArg(args[0], XmNmessageString, string);
    XtSetArg(args[1], XmNcancelLabelString, okstring);
    XtSetArg(args[2], XmNokLabelString, cancelstring);
    XtSetArg(args[3], XmNdefaultButtonType, def ? XmDIALOG_CANCEL_BUTTON : XmDIALOG_OK_BUTTON);
    XtSetValues(warningDialog, args, 4);
    XmStringFree(string);
    XmStringFree(okstring);
    XmStringFree(cancelstring);
    XtManageChild(warningDialog);
}

void InitWarningDialog() {
    warningDialog = XtNameToWidget(mainwin, "*WarningDialog");
}
