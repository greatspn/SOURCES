#define ___INPUTDIALOG___
#include "global.h"
#include "inputdialog.h"


static Widget inputDialog, inputText;
static char *bufaddress;

static void KillInputDlgCB(Widget w, XtPointer closure, XtPointer call_data) {
    if (closure == INPUT_OK) {
        bufaddress = XmTextFieldGetString(inputText);
    }
    XtDestroyWidget(inputDialog);
}

void  ShowInputDialog(char *message, XtCallbackProc cancelCB, XtCallbackProc okCB, Widget parent) {
    Arg args[1];
    XmString string;
    Widget temp;

    inputDialog = FetchWidget(parent, "InputDialog");
    temp = XmSelectionBoxGetChild(inputDialog, XmDIALOG_OK_BUTTON);
    XtAddCallback(temp, XmNactivateCallback, KillInputDlgCB, (XtPointer)INPUT_OK);

    if (okCB != NULL)
        XtAddCallback(temp, XmNactivateCallback, okCB, (XtPointer) &bufaddress);

    temp = XmSelectionBoxGetChild(inputDialog, XmDIALOG_CANCEL_BUTTON);
    XtAddCallback(temp, XmNactivateCallback, KillInputDlgCB, INPUT_CANCEL);

    if (cancelCB != NULL)
        XtAddCallback(temp, XmNactivateCallback, cancelCB, NULL);

    string = XmStringCreateLtoR(message, XmFONTLIST_DEFAULT_TAG);
    XtSetArg(args[0], XmNselectionLabelString, string);
    XtSetValues(inputDialog, args, 1);
    XmStringFree(string);
    inputText = XmSelectionBoxGetChild(inputDialog, XmDIALOG_TEXT);
    XtManageChild(inputDialog);
}
