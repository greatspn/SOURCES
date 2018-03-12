#define ___SOLVEDIALOG___
#	include "global.h"
#include "solvedialog.h"

static Widget solveDialog;

void ShowSolveDialog(char *message, XtCallbackProc CB1) {
    Arg args[1];
    XmString string;
    static void (*OkCB)() = NULL;

    string = XmStringCreate(message, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[0], XmNmessageString, string);
    XtSetValues(solveDialog, args, 1);
    if (OkCB != NULL) {
        XtRemoveCallback(solveDialog, XmNokCallback, OkCB, 0);
    }
    XtAddCallback(solveDialog, XmNokCallback, CB1, 0);
    OkCB = CB1;
    XmStringFree(string);
    XtManageChild(solveDialog);
}

void InitSolveDialog(void) {
    solveDialog = XtNameToWidget(mainwin, "*SOLVE_DIALOG");
}
