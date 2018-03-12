#define ___SHOWDIALOG___
#	include "global.h"
#	include <Mrm/MrmPublic.h>
#include "showdialog.h"

static Widget   show_dlg, show_done, show_text;
static Boolean Shown;
// static Pixmap	circle_pixmap ;

void HideShowDialog(void) {
    XtUnmanageChild(show_dlg);
}

static void HideShowDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    Shown = FALSE;
    HideShowDialog();
}

void ShowShowDialog(char *string) {
    XmProcessTraversal(show_done, XmTRAVERSE_CURRENT);
    XmTextSetString(show_text, string);
    if (!Shown)
        XtManageChild(show_dlg);
    Shown = TRUE;
}

void RegisterShowDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"HideShowDialogCB", (XtPointer)HideShowDialogCB},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterArcChangeDialog names\n");
}

void InitShowDialog(void) {

    show_dlg = XtNameToWidget(frame_w, "*ShowDialog");
    show_text = XtNameToWidget(show_dlg, "*ShowDialog_Text");
    show_done = XtNameToWidget(show_dlg, "*ShowDialog_Done");

}



