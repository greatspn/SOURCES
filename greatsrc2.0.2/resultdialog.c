#define ___RESULTDIALOG___
#	include "global.h"
#	include "parse.h"
#	include "res.h"
#	include <Mrm/MrmPublic.h>
#include "resultdialog.h"

static Widget   res_dlg, res_tag, res_res;
static struct res_object *dialog_res;


static void ShowResHelpCB(Widget w, XtPointer closure, XtPointer call_data) {
    SetVisibleResGrammarDialog();
    ShowResGrammarDialog();
}

void HideResChangeDialog(void) {
    XtUnmanageChild(res_dlg);
}


static void SetResCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *text, *res, *tag;
    Boolean Create;

    Create = (dialog_res == NULL);
    text = XmTextFieldGetString(res_tag);
    if (ParseResTag(text, &tag, res_tag, res_dlg, dialog_res)) {
        XtFree(text);
        return ;
    }

    tag = strdup(tag);
    XtFree(text);

    text = XmTextGetString(res_res);

    if (ParseRes(text, &res, res_res, res_dlg)) {
        XtFree(text);
        free(tag);
        return ;
    }

    if (res == NULL) {
        ShowErrorDialog("Error: you must enter a result definition !", res_dlg);
        free(tag);
        XtFree(text);
        return ;
    }

    res = strdup(res);
    XtFree(text);


    if (Create) {
        struct res_object *l_res;

        if (netobj->results == NULL)
            l_res = NULL;
        else
            for (l_res = netobj->results;
                    l_res->next != NULL; l_res = l_res->next);
        dialog_res = (struct res_object *) emalloc(RESOBJ_SIZE);
        if (l_res != NULL)
            l_res->next = dialog_res;
        else
            netobj->results = dialog_res;
        dialog_res->center.x = fix_x;
        dialog_res->center.y = fix_y;
        dialog_res->value1 = -1.0;
        dialog_res->next = NULL;
        dialog_res->text = NULL;
        dialog_res->tag = NULL;
    }
    else {
        ShowRes(dialog_res, CLEAR);
    }


    HideResChangeDialog();



    if (dialog_res->tag != NULL) {
        free(dialog_res->tag);
    }
    if (dialog_res->text != NULL) {
        free(dialog_res->text);
    }

    dialog_res->text = res;
    dialog_res->tag = tag;

    ShowRes(dialog_res, SET);
    menu_action();
    SetModify();
    TypeResDef(dialog_res);

}




static void CancelResDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideResChangeDialog();
}


void ShowResChangeDialog(struct res_object *co, int focus) {
    Arg          args[1];
    char		 *insert;
    Widget focusTo;
    XmString XmTemp;
    int insertlen;


    dialog_res = co;

    if (co != NULL) {
        XmTemp = XmStringCreate("Change Result Definition", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);

        if (dialog_res->text != NULL)
            XmTextSetString(res_res, dialog_res->text);
        else
            XmTextSetString(res_res, "");

        if (dialog_res->tag != NULL)
            XmTextFieldSetString(res_tag, dialog_res->tag);
        else
            XmTextFieldSetString(res_tag, "");

        switch (focus) {
        case 1:
            focusTo = res_res;
            break;
        default:
            focusTo = res_tag;
        }
    }
    else {
        XmTemp = XmStringCreate("Create Res Definition", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);
        focusTo = res_tag;
        XmTextFieldSetString(res_tag, "");
        XmTextSetString(res_res, "");
    }
    XtSetValues(res_dlg, args, 1);
    XmStringFree(XmTemp);
    XtManageChild(res_dlg);
    XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
    switch (focus) {
    case 1:
        insert = XmTextGetString(focusTo);
        insertlen = strlen(insert);
        XmTextSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
        XmTextSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
        break;
    default:
        insert = XmTextFieldGetString(focusTo);
        insertlen = strlen(insert);
        XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
        XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
    }

    XtFree(insert);

}

void RegisterResultDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"CancelResDialogCB", (XtPointer)CancelResDialogCB},
        {"SetResCB"		, (XtPointer)SetResCB},
        {"ShowResHelpCB"		, (XtPointer)ShowResHelpCB},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterArcChangeDialog names\n");
}

void InitResChangeDialog(void) {
    res_dlg = XtNameToWidget(frame_w, "*ResChangeDialog");
    res_res = XtNameToWidget(res_dlg, "*ResChangeDialog_Def");
    res_tag = XtNameToWidget(res_dlg, "*ResChangeDialog_Tag");
}


