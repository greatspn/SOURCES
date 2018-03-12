#define ___MDGRAMMARDIALOG___
#	include <Mrm/MrmPublic.h>
#	include "global.h"
#include "mdgrammardialog.h"

static Widget mdgrammar_dialog, md_done, res_done, resgrammar_dialog;
static Boolean md_grammar_managed;
static Boolean res_grammar_managed;

void ShowMDGrammarDialog(void) {
    if (md_grammar_managed) {
        XtManageChild(mdgrammar_dialog);
        XmProcessTraversal(md_done, XmTRAVERSE_CURRENT);
    }
}

void ShowResGrammarDialog(void) {
    if (res_grammar_managed) {
        XtManageChild(resgrammar_dialog);
        XmProcessTraversal(res_done, XmTRAVERSE_CURRENT);
    }
}

void HideMDGrammarDialog(void) {
    XtUnmanageChild(mdgrammar_dialog);
}

void HideResGrammarDialog(void) {
    XtUnmanageChild(resgrammar_dialog);
}

static void HideMDGrammarDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    md_grammar_managed = FALSE;
    HideMDGrammarDialog();
}

static void HideResGrammarDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    res_grammar_managed = FALSE;
    HideResGrammarDialog();
}
void SetVisibleMDGrammarDialog(void) {
    md_grammar_managed = TRUE;
}

void SetVisibleResGrammarDialog(void) {
    res_grammar_managed = TRUE;
}

void RegisterMDGrammarDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"HideMDGrammarDialogCB", (XtPointer)HideMDGrammarDialogCB},
        {"HideResGrammarDialogCB", (XtPointer)HideResGrammarDialogCB}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterMDGrammarDialog names\n");
}

void InitMDGrammarDialog(void) {
    mdgrammar_dialog = XtNameToWidget(frame_w, "*MDGrammarDialog");
    md_done = XtNameToWidget(mdgrammar_dialog, "*MDGrammarDialog_Done");
    resgrammar_dialog = XtNameToWidget(frame_w, "*ResGrammarDialog");
    res_done = XtNameToWidget(resgrammar_dialog, "*MDGrammarDialog_Done");

    XtAddCallback(md_done, XmNactivateCallback, HideMDGrammarDialogCB, NULL);
    XtAddCallback(res_done, XmNactivateCallback, HideResGrammarDialogCB, NULL);

    md_grammar_managed = FALSE;
    res_grammar_managed = FALSE;

}




