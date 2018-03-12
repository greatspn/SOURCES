#define ___ARCDIALOG___
#include "global.h"
#include "draw.h"
#include "res.h"
#include "parse.h"
#include "arcdialog.h"
#include <Mrm/MrmPublic.h>

static Widget   Arc_dlg, arc_in, arc_out, arc_inh,
       arc_broken, arc_label3, arc_iscolor, arc_color, arc_place, arc_trans;
static	struct arc_object *dialog_arc;
static char color_buffer[LINEMAX];
static char mult_buffer[LINEMAX];
static char	arc_kind;
static Boolean initDone;


static void SetArcKindCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    switch (closure) {
    case ARCIN:
        arc_kind = TO_TRANS;
        break;
    case ARCOUT:
        arc_kind = TO_PLACE;
        break;
    default:
        arc_kind = INHIBITOR;
        break;
    }
}

static void SetArcTypeCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *color, *text;
    int multVal;


    text = XmTextFieldGetString(arc_color);

    if (XmToggleButtonGadgetGetState(arc_iscolor)) {

        if (ParseColor(text, &color, arc_color, Arc_dlg)) {
            XtFree(text);
            return ;
        }

        if (color == NULL) {
            ShowErrorDialog("Error: you must enter a color label !", Arc_dlg);
            XtFree(text);
            return ;
        }

        color = strdup(color);
        multVal = 1;
    }
    else {
        if (ParseNatural(text, &multVal)) {
            ShowErrorDialog("Error: multiplicity must be a positive number.", Arc_dlg);
            XtFree(text);
            return;
        }
        color = NULL;
    }
    XtFree(text);

    HideArcChangeDialog();

    DrawArc(dialog_arc, CLEAR);	/* erase arc */


    if (dialog_arc->color) {
        free(dialog_arc->color);
    }
    else {
        dialog_arc->colpos.x = 0.0;
        dialog_arc->colpos.y = 0.0;
    }

    dialog_arc->color = color;
    dialog_arc->type = arc_kind;
    dialog_arc->mult = XmToggleButtonGadgetGetState(arc_broken) ? -multVal : multVal;
    DrawArc(dialog_arc, OR);
    menu_action();
    SetModify();
    ClearRes();

}


static void SetColorCB(Widget w, XtPointer closure, XmToggleButtonCallbackStruct *call_data) {
    Arg args[1];
    char *temp;
    XmString XmTemp;

    if (!initDone)
        return ;
    if (call_data->set) {
        temp = XmTextFieldGetString(arc_color);
        strcpy(mult_buffer, temp);
        XmTextFieldSetString(arc_color, color_buffer);
        XmTemp = XmStringCreateSimple("Color :");
    }
    else {
        temp = XmTextFieldGetString(arc_color);
        strcpy(color_buffer, temp);
        XmTextFieldSetString(arc_color, mult_buffer);
        XmTemp = XmStringCreateSimple("Multiplicity :");
    }
    free(temp);
    XtSetArg(args[0], XmNlabelString, XmTemp);
    XtSetValues(arc_label3, args, 1);

    XmStringFree(XmTemp);

}

static void CancelArcDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideArcChangeDialog();
}

void HideArcChangeDialog(void) {
    XtUnmanageChild(Arc_dlg);
}

void ShowArcChangeDialog(struct arc_object *arc, int focus) {
    Arg          args[1];
    char		multString[40];
    char buf[256];
    Widget focusTo;
    char *insert;
    int insertlen;
    XmString XmTemp;

    dialog_arc = arc;

    XtSetArg(args[0], XmNset, dialog_arc->type == TO_TRANS);
    XtSetValues(arc_in, args, 1);
    XtSetArg(args[0], XmNset, dialog_arc->type == TO_PLACE);
    XtSetValues(arc_out, args, 1);
    XtSetArg(args[0], XmNset, dialog_arc->type == INHIBITOR);
    XtSetValues(arc_inh, args, 1);

    printf("focus=%d\n", focus);

    focusTo = arc_color;
    initDone = FALSE;
    arc_kind = dialog_arc->type;

    strcpy(color_buffer, "");
    sprintf(multString, "%d", abs(dialog_arc->mult));
    strcpy(mult_buffer, multString);
    XtManageChild(Arc_dlg);

    if (dialog_arc->color != NULL || dialog_arc->place->color != NULL || focus) {
        if (dialog_arc->color == NULL) {
            XmTextFieldSetString(arc_color, "");
        }
        else {
            XmTextFieldSetString(arc_color, dialog_arc->color);
            strcpy(color_buffer, dialog_arc->color);
        }
        XmTemp = XmStringCreateSimple("Color :");
    }
    else {
        XmTextFieldSetString(arc_color, multString);
        XmTemp = XmStringCreateSimple("Multiplicity :");
    }
    XmToggleButtonGadgetSetState(arc_broken, dialog_arc->mult < 0, TRUE);
    XmToggleButtonGadgetSetState(arc_iscolor, dialog_arc->color != NULL || dialog_arc->place->color != NULL || focus, TRUE);
    initDone = TRUE;

    XtSetArg(args[0], XmNlabelString, XmTemp);
    XtSetValues(arc_label3, args, 1);
    XmStringFree(XmTemp);

    sprintf(buf, "Place : %s", dialog_arc->place->tag);
    XmTemp = XmStringCreateSimple(buf);
    XtSetArg(args[0], XmNlabelString, XmTemp);
    XtSetValues(arc_place, args, 1);
    XmStringFree(XmTemp);


    sprintf(buf, "Transition : %s", dialog_arc->trans->tag);
    XmTemp = XmStringCreateSimple(buf);
    XtSetArg(args[0], XmNlabelString, XmTemp);
    XtSetValues(arc_trans, args, 1);
    XmStringFree(XmTemp);


    XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);

    insert = XmTextFieldGetString(focusTo);
    insertlen = strlen(insert);
    XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
    XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
    XtFree(insert);
}

void RegisterArcChangeDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"SetArcKindCB"		, (XtPointer)SetArcKindCB},
        {"CancelArcDialogCB", (XtPointer)CancelArcDialogCB},
        {"SetArcTypeCB"		, (XtPointer)SetArcTypeCB},
        {"SetColorCB"		, (XtPointer)SetColorCB},
        {"ARCIN"			, (XtPointer)ARCIN},
        {"ARCOUT"			, (XtPointer)ARCOUT},
        {"ARCINHIB"			, (XtPointer)ARCINHIB}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterArcChangeDialog names\n");
}

void InitArcChangeDialog(void) {
    Arc_dlg = XtNameToWidget(frame_w, "*ArcChangeDialog");
    arc_in = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Input");
    arc_out = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Output");
    arc_inh = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Inhibitor");
    arc_broken = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Broken");
    arc_iscolor = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_IsColor");
    arc_label3 = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Label3");
    arc_color = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Color");
    arc_place = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Place");
    arc_trans = XtNameToWidget(Arc_dlg, "*ArcChangeDialog_Trans");
}


