#define ___SWN___
#include "global.h"
#include "res.h"
#include <Mrm/MrmPublic.h>
#include "unfold.h"
#include "consoledialog.h"

static int whichClevel;
static int whichType;
static Widget unfold_dialog, unfold_PX, unfold_PY, unfold_TX, unfold_TY;
//extern  ScheduleSwn(int,int,int,int,int,int,int,int,int);

#include "calc.h"

void ShowSwnUnfoldOptionsDialog(int t) {
    Arg args[1];
    XmString xms;

    whichType = t;
    xms = XmStringCreateSimple("Unfolding Options");

    XtSetArg(args[0], XmNdialogTitle, xms);
    XtSetValues(unfold_dialog, args, 1);
    XmStringFree(xms);
    XtManageChild(unfold_dialog);
}

void HideSwnUnfoldOptionsDialog(void) {
    XtUnmanageChild(unfold_dialog);
}

static void OkSwnUnfoldDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *errmess = NULL;
    Widget focusTo;
    int px, py, tx, ty;
    /*int p;*/
    char *tmp;

    tmp = XmTextFieldGetString(unfold_PX);
    if ((px = atoi(tmp)) >= 0) {
        XtFree(tmp);
        tmp = XmTextFieldGetString(unfold_PY);
        if ((py = atoi(tmp)) >= 0) {
            XtFree(tmp);
            tmp = XmTextFieldGetString(unfold_TX);
            if ((tx = atoi(tmp)) >= 0) {
                XtFree(tmp);
                tmp = XmTextFieldGetString(unfold_TY);
                if ((ty = atoi(tmp)) >= 0) {
                    XtFree(tmp);
                    ScheduleSwn(whichType, px, py, tx, ty, 0, 0, 0, 0);
                }
                else {
                    errmess = "Incorrect Vertical Transition Displacement [ > 0]";
                    focusTo = unfold_TY;
                }
            }
            else {
                errmess = "Incorrect Orizonthal Transition Displacement [ > 0]";
                focusTo = unfold_TX;
            }
        }
        else {
            errmess = "Incorrect Vertical Place Displacement [ > 0]";
            focusTo = unfold_PY;
        }
    }
    else {
        errmess = "Incorrect Orizonthal Place Displacement [ > 0]";
        focusTo = unfold_PX;
    }

    if (errmess != NULL) {
        int insertlen;

        ShowErrorDialog(errmess, unfold_dialog);
        tmp = XmTextFieldGetString(focusTo);
        insertlen = strlen(tmp);
        XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
        XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
        XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
        XtFree(tmp);
    }
    else
        HideSwnUnfoldOptionsDialog();

}

static void CancelSwnUnfoldDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    ResetConsoleDialog();
    HideSwnUnfoldOptionsDialog();
}

void RegisterSwnUnfoldOptionsDialog(void) {

    static MrmRegisterArg	regvec[] = {
        {"OkSwnUnfoldDialogCB"	, (XtPointer)OkSwnUnfoldDialogCB},
        {"CancelSwnUnfoldDialogCB"	, (XtPointer)CancelSwnUnfoldDialogCB}
    };

    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);
    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterSwnUnfoldOptionsDialog names\n");
}


void InitSwnUnfoldOptionsDialog(void) {

    unfold_dialog = XtNameToWidget(frame_w, "*SwnUnfoldOptionsDialog");
    unfold_PX = XtNameToWidget(unfold_dialog, "*SwnUnfoldOptionsDialog_PX");
    unfold_PY = XtNameToWidget(unfold_dialog, "*SwnUnfoldOptionsDialog_PY");
    unfold_TX = XtNameToWidget(unfold_dialog, "*SwnUnfoldOptionsDialog_TX");
    unfold_TY = XtNameToWidget(unfold_dialog, "*SwnUnfoldOptionsDialog_TY");
    XmTextFieldSetString(unfold_PX, "0");
    XmTextFieldSetString(unfold_PY, "0");
    XmTextFieldSetString(unfold_TX, "0");
    XmTextFieldSetString(unfold_TY, "0");
}

