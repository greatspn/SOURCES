#define ___COLORDIALOG___
#	include "global.h"
#	include "parse.h"
#	include "color.h"
#	include "draw.h"
#	include "mark.h"
#	include "lisp.h"
#	include "res.h"
#	include <Mrm/MrmPublic.h>
#include "colordialog.h"

static Widget   color_dlg, color_tag, color_color,
       color_colorset, color_marking, color_function;

static struct lisp_object *dialog_color;
static char dialog_color_type;




void HideColorChangeDialog(void) {
    XtUnmanageChild(color_dlg);
}

static void ChangeColorTypeCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    if (call_data->set)
        switch (closure) {
        case COLORSET:
            dialog_color_type = 'c';
            break;
        case COLORMARK:
            dialog_color_type = 'm';
            break;
        default:
            dialog_color_type = 'f';
            break;
        }
}

static void SetColorTypeCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *text, *color, *tag;
    char old_type;
    Boolean Create;
    struct arc_object *last_arc;
    struct group_object *group;
    char *oldtag;


    old_type = '?';
    Create = (dialog_color == NULL);
    text = XmTextFieldGetString(color_tag);

    if (ParseColorTag(text, &tag, color_tag, color_dlg, dialog_color)) {
        XtFree(text);
        return ;
    }

    tag = strdup(tag);
    XtFree(text);

    text = XmTextGetString(color_color);
    if (ParseColor(text, &color, color_color, color_dlg)) {
        XtFree(text);
        free(tag);
        return ;
    }
    if (color == NULL) {
        ShowErrorDialog("Error: you must enter a color Definition !", color_dlg);
        free(tag);
        XtFree(text);
        return ;
    }

    color = strdup(color);
    XtFree(text);


    if (Create) {
        struct lisp_object *l_lisp;
        if (netobj->lisps == NULL)
            l_lisp = NULL;
        else
            for (l_lisp = netobj->lisps; l_lisp->next != NULL;
                    l_lisp = l_lisp->next);

        dialog_color = (struct lisp_object *) emalloc(LISPOBJ_SIZE);
        if (l_lisp != NULL)
            l_lisp->next = dialog_color;
        else
            netobj->lisps = dialog_color;
        dialog_color->center.x = fix_x;
        dialog_color->center.y = fix_y;
        dialog_color->next = NULL;
        dialog_color->text = NULL;
        dialog_color->tag = NULL;
    }
    else {
        old_type = dialog_color->type;
        ShowLisp(dialog_color, CLEAR);
    }


    dialog_color->type = dialog_color_type;

    HideColorChangeDialog();

    oldtag = dialog_color->tag;

    if (!Create) {
        for (last_place = netobj->places; last_place != NULL;
                last_place = last_place->next)
            if (last_place->cmark == dialog_color)
                ShowPlaceMark(last_place, CLEAR);
        for (last_place = netobj->places; last_place != NULL;
                last_place = last_place->next)
            if (last_place->color != NULL)
                if (!strcmp(last_place->color, oldtag)) {
                    ShowPlaceColor(last_place, &mainDS, 0.0, 0.0, CLEAR);
                    free(last_place->color);
                    last_place->color = strdup(tag);
                    ShowPlaceColor(last_place, &mainDS, 0.0, 0.0, OR);
                }
        for (last_trans = netobj->trans; last_trans != NULL;
                last_trans = last_trans->next)
            if (last_trans->color != NULL)
                if (!strcmp(last_trans->color, oldtag)) {
                    ShowTransColor(last_trans, &mainDS, 0.0, 0.0, CLEAR);
                    free(last_trans->color);
                    last_trans->color = strdup(tag);
                    ShowTransColor(last_trans, &mainDS, 0.0, 0.0, OR);
                }
        for (group = netobj->groups; group != NULL; group = group->next)
            for (last_trans = group->trans; last_trans != NULL;
                    last_trans = last_trans->next)
                if (last_trans->color != NULL)
                    if (!strcmp(last_trans->color, oldtag)) {
                        ShowTransColor(last_trans, &mainDS, 0.0, 0.0, CLEAR);
                        free(last_trans->color);
                        last_trans->color = strdup(tag);
                        ShowTransColor(last_trans, &mainDS, 0.0, 0.0, OR);
                    }
        for (last_arc = netobj->arcs; last_arc != NULL;
                last_arc = last_arc->next)
            if (last_arc->color != NULL)
                if (!strcmp(last_arc->color, oldtag)) {
                    DrawArc(last_arc, CLEAR);
                    free(last_arc->color);
                    last_arc->color = strdup(tag);
                    DrawArc(last_arc, OR);
                }
    }
    if (dialog_color->tag != NULL) {
        free(dialog_color->tag);
    }
    if (dialog_color->text != NULL) {
        free(dialog_color->text);
    }

    dialog_color->text = color;
    dialog_color->tag = tag;

    if (!Create) {
        for (last_place = netobj->places; last_place != NULL;
                last_place = last_place->next)
            if (last_place->cmark == dialog_color)
                ShowPlaceMark(last_place, OR);
    }

    ShowLisp(dialog_color, SET);
    menu_action();
    SetModify();
    ClearRes();

    TypeLispDef(dialog_color);
}




static void CancelColorDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideColorChangeDialog();
}

void ShowColorChangeDialog(struct lisp_object *co, int focus) {
    Arg          args[1];
    char		 *insert;
    int insertlen;
    Widget focusTo;
    XmString XmTemp;


    dialog_color = co;

    if (co != NULL) {
        XmTemp = XmStringCreate("Change Color Definition", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);

        if (dialog_color->text != NULL)
            XmTextSetString(color_color, dialog_color->text);
        else
            XmTextSetString(color_color, "");

        if (dialog_color->tag != NULL)
            XmTextFieldSetString(color_tag, dialog_color->tag);
        else
            XmTextFieldSetString(color_tag, "");

        switch (focus) {
        case 1:
            focusTo = color_color;
            break;
        default:
            focusTo = color_tag;
        }
        XmToggleButtonGadgetSetState(color_colorset, dialog_color->type == 'c', TRUE);
        XmToggleButtonGadgetSetState(color_marking, dialog_color->type == 'm', TRUE);
        XmToggleButtonGadgetSetState(color_function, dialog_color->type == 'f', TRUE);
        dialog_color_type = dialog_color->type;
    }
    else {
        XmTemp = XmStringCreate("Create Color Definition", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);
        XmToggleButtonGadgetSetState(color_colorset, TRUE, TRUE);
        XmToggleButtonGadgetSetState(color_marking, FALSE, TRUE);
        XmToggleButtonGadgetSetState(color_function, FALSE, TRUE);
        dialog_color_type = 'c';
        focusTo = color_tag;
        XmTextFieldSetString(color_tag, "");
        XmTextSetString(color_color, "");
    }
    XtSetValues(color_dlg, args, 1);
    XmStringFree(XmTemp);
    XtManageChild(color_dlg);
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

void RegisterColorChangeDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"ChangeColorTypeCB"	, (XtPointer)ChangeColorTypeCB},
        {"CancelColorDialogCB"	, (XtPointer)CancelColorDialogCB},
        {"SetColorTypeCB"		, (XtPointer)SetColorTypeCB},
        {"COLORSET"				, (XtPointer)COLORSET},
        {"COLORMARK"			, (XtPointer)COLORMARK},
        {"COLORFUN"				, (XtPointer)COLORFUN}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterColorChangeDialog names\n");
}

void InitColorChangeDialog(void) {
    color_dlg = XtNameToWidget(frame_w, "*ColorChangeDialog");
    color_color = XtNameToWidget(color_dlg, "*ColorChangeDialog_Color");
    color_tag = XtNameToWidget(color_dlg, "*ColorChangeDialog_Tag");
    color_colorset = XtNameToWidget(color_dlg, "*ColorChangeDialog_Colorset");
    color_marking = XtNameToWidget(color_dlg, "*ColorChangeDialog_Marking");
    color_function = XtNameToWidget(color_dlg, "*ColorChangeDialog_Function");
}






