#define ___COMMENTDIALOG___
#include "global.h"
#include <Mrm/MrmPublic.h>
#include "commentdialog.h"


static Widget commentDialog, comment_text;

void ShowCommentDialog(char *text) {
    if (text == NULL)
        text = "";
    XmTextSetString(comment_text, text);	XtManageChild(commentDialog);
    XmProcessTraversal(comment_text, XmTRAVERSE_CURRENT);
}

static void EditCommentOKCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *temp;

    temp = XmTextGetString(comment_text);

    if (temp != NULL && netobj->comment != NULL) {
        if (strcmp(netobj->comment, temp))
            SetModify();
        else
            ResetModify();
    }
    else {
        if (netobj->comment != temp)
            SetModify();
        else
            ResetModify();
    }

    if (netobj->comment != NULL)
        XtFree(netobj->comment);
    netobj->comment = temp;
}

void RegisterCommentDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"EditCommentOKCB"	, (XtPointer)EditCommentOKCB}

    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterCommentDialog names\n");
}

void InitCommentDialog(void) {
    commentDialog = XtNameToWidget(mainwin, "*CommentDialog");
    comment_text = XtNameToWidget(commentDialog, "*CommentDialogScrolledText");
}







