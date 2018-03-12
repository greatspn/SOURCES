#define ___MARKDIALOG___
#	include "global.h"
#	include "parse.h"
#	include "mark.h"
#	include "res.h"
#	include <Mrm/MrmPublic.h>
#include "markdialog.h"

static struct mpar_object *dialog_mpar;
static Widget   mark_dlg, mark_marking, mark_tag;

void HideMarkChangeDialog(void) {
    XtUnmanageChild(mark_dlg);
}

static void SetMarkCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *text, *tag;
    int markVal;
    Boolean UpdateRequired;


    text = XmTextFieldGetString(mark_tag);

    if (ParseMarkParTag(text, &tag, mark_tag, mark_dlg, dialog_mpar)) {
        XtFree(text);
        return ;
    }

    tag = strdup(tag);
    XtFree(text);

    text = XmTextFieldGetString(mark_marking);

    if (ParseNatural(text, &markVal)) {
        free(tag);
        XtFree(text);
        ShowErrorDialog("Marking must be a positive integer !", mark_dlg);
        XmProcessTraversal(mark_marking, XmTRAVERSE_CURRENT);
        return ;
    }

    XtFree(text);

    if (dialog_mpar == NULL) {
        struct mpar_object *l_mpar;

        dialog_mpar = (struct mpar_object *) emalloc(MPAOBJ_SIZE);
        dialog_mpar->center.x = fix_x;
        dialog_mpar->center.y = fix_y;
        dialog_mpar->next = NULL;
        dialog_mpar->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);

        if (netobj->mpars == NULL)
            l_mpar = NULL;
        else
            for (l_mpar = netobj->mpars; l_mpar->next != NULL; l_mpar = l_mpar->next);

        if (l_mpar != NULL)
            l_mpar->next = dialog_mpar;
        else
            netobj->mpars = dialog_mpar;

        UpdateRequired = FALSE;

    }
    else
        ShowMpar(dialog_mpar, CLEAR);


    if (UpdateRequired) {
        struct place_object *last_place;

        for (last_place = netobj->places; last_place != NULL; last_place = last_place->next)
            if (last_place->mpar == dialog_mpar)
                ShowPlaceMark(last_place, CLEAR);
    }

    if (dialog_mpar->tag != NULL)
        free(dialog_mpar->tag);

    dialog_mpar->tag = tag;
    dialog_mpar->value = markVal;

    if (UpdateRequired) {
        struct place_object *last_place;

        for (last_place = netobj->places; last_place != NULL; last_place = last_place->next)
            if (last_place->mpar == dialog_mpar)
                ShowPlaceMark(last_place, OR);

    }


    ShowMpar(dialog_mpar, OR);
    ClearRes();
    menu_action();
    SetModify();
    HideMarkChangeDialog();
}


static void CancelMarkDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideMarkChangeDialog();
}


void ShowMarkChangeDialog(struct mpar_object *mp, int focus) {
    Arg          args[1];
    char		tempString[LINEMAX];
    Widget		focusTo;
    XmString XmTemp;
    char *insert;
    int insertlen;

    dialog_mpar = mp;
    if (dialog_mpar != NULL) {
        XmTemp = XmStringCreate("Change Marking Parameter", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);
        XtSetValues(mark_dlg, args, 1);

        if (dialog_mpar->tag != NULL)
            XmTextFieldSetString(mark_tag, dialog_mpar->tag);
        else
            XmTextFieldSetString(mark_tag, "");

        sprintf(tempString, "%d", dialog_mpar->value);
        XmTextFieldSetString(mark_marking, tempString);

        switch (focus) {
        case 0:
            focusTo = mark_tag;
            break;
        default:
            focusTo = mark_marking;
        }
    }
    else {
        XmTemp = XmStringCreate("Create Marking Parameter", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);
        XtSetValues(mark_dlg, args, 1);

        XmTextFieldSetString(mark_tag, "");
        XmTextFieldSetString(mark_marking, "");
        focusTo = mark_tag;
    }
    XmStringFree(XmTemp);
    XtManageChild(mark_dlg);

    XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
    insert = XmTextFieldGetString(focusTo);
    insertlen = strlen(insert);
    XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
    XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
    XtFree(insert);
}

void RegisterMarkDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"SetMarkCB"			, (XtPointer)SetMarkCB},
        {"CancelMarkDialogCB"	, (XtPointer)CancelMarkDialogCB},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterMarkDialog names\n");
}

void InitMarkChangeDialog(void) {

    mark_dlg = XtNameToWidget(frame_w, "*MarkChangeDialog");
    mark_marking = XtNameToWidget(mark_dlg, "*MarkChangeDialog_Marking");
    mark_tag = XtNameToWidget(mark_dlg, "*MarkChangeDialog_Tag");

    /*
    XtAddCallback(mark_marking, XmNactivateCallback, set_mark_typeCB, NULL);
    XtAddCallback(mark_tag, XmNactivateCallback, set_mark_typeCB, NULL);
    XtAddCallback(mark_cancel, XmNactivateCallback, CancelMarkDialogCB, 'x');
    XtAddCallback(mark_ok, XmNactivateCallback, set_mark_typeCB, NULL);
    */
}



