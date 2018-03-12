#define ___PLACEDIALOG___
#	include "global.h"
#	include "parse.h"
#	include "draw.h"
#	include "mark.h"
#	include "printarea.h"
#	include "res.h"
#	include <Mrm/MrmPublic.h>
#include "placedialog.h"



static Widget   place_dlg, place_marking, place_tag, place_color;
static struct place_object *dialog_place;

void HidePlaceChangeDialog(void) {
    XtUnmanageChild(place_dlg);
}

static void SetPlaceTypeCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *text, *color, *tag;
    int markVal;
    struct mpar_object *mp;
    struct lisp_object *lp;

    text = XmTextFieldGetString(place_tag);
    if (ParsePlaceTag(text, &tag, place_tag, place_dlg, dialog_place)) {
        XtFree(text);
        return;
    }

    tag = strdup(tag);
    XtFree(text);

    text = XmTextFieldGetString(place_color);
    if (ParseColor(text, &color, place_color, place_dlg)) {
        XtFree(text);
        free(tag);
        return ;
    }
    if (color != NULL)
        color = strdup(color);
    XtFree(text);

    mp = NULL;
    lp = NULL;
    text = XmTextFieldGetString(place_marking);

    if (ParseNatural(text, &markVal)) {
        if (ParseMarkPar(text, &mp, &lp)) {
            free(tag);
            if (color != NULL)
                free(color);
            XtFree(text);
            ShowErrorDialog("Positive Marking or existing marking parameter required", place_dlg);
            XmProcessTraversal(place_marking, XmTRAVERSE_CURRENT);

            return ;
        }
    }
    XtFree(text);

    DrawPlace(dialog_place, CLEAR);	/* erase place */
    ShowPlaceMark(dialog_place, CLEAR);

    if (dialog_place->tag != NULL)
        free(dialog_place->tag);

    if (dialog_place->color != NULL)
        free(dialog_place->color);
    else {
        dialog_place->colpos.x = place_radius + 5;
        dialog_place->colpos.y = 8;
    }

    dialog_place->tag = tag;
    dialog_place->color = color;
    dialog_place->cmark = NULL;
    dialog_place->mpar = NULL;
    dialog_place->tokens = markVal;
    dialog_place->m0 = markVal;

    if (mp != NULL) {
        dialog_place->mpar = mp;
        dialog_place->tokens = -1;
        dialog_place->m0 = -1;
    }
    else if (lp != NULL) {
        dialog_place->cmark = lp;
        dialog_place->tokens = 0;
        dialog_place->m0 = 0;
    }


    HidePlaceChangeDialog();


    DrawPlace(dialog_place, SET);
    menu_action();
    SetModify();
    ClearRes();
}

static void CancelPlaceDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HidePlaceChangeDialog();
}

void ShowPlaceChangeDialog(struct place_object *place, int focus) {
    char		tempString[LINEMAX];
    char		 *insert;
    int insertlen;
    Widget focusTo;

    dialog_place = place;
    switch (focus) {
    case 0:
        focusTo = place_tag;
        break;
    case 1:
        focusTo = place_color;
        break;
    default:
        focusTo = place_marking;
    }

    if (dialog_place->color != NULL)
        XmTextFieldSetString(place_color, dialog_place->color);
    else
        XmTextFieldSetString(place_color, "");

    if (dialog_place->tag != NULL)
        XmTextFieldSetString(place_tag, dialog_place->tag);
    else
        XmTextFieldSetString(place_tag, "");

    if (dialog_place->mpar != NULL)
        strcpy(tempString, dialog_place->mpar->tag);
    else if (dialog_place->cmark != NULL)
        strcpy(tempString, dialog_place->cmark->tag);
    else
        sprintf(tempString, "%d", abs(dialog_place->tokens));

    XmTextFieldSetString(place_marking, tempString);

    XtManageChild(place_dlg);
    XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);

    insert = XmTextFieldGetString(focusTo);
    insertlen = strlen(insert);
    XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
    XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
    XtFree(insert);
}
void RegisterPlaceChangeDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"CancelPlaceDialogCB", (XtPointer)CancelPlaceDialogCB},
        {"SetPlaceTypeCB"		, (XtPointer)SetPlaceTypeCB},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterArcChangeDialog names\n");
}

void InitPlaceChangeDialog(void) {
    place_dlg = XtNameToWidget(frame_w, "*PlaceChangeDialog");
    place_marking = XtNameToWidget(place_dlg, "*PlaceChangeDialog_Marking");
    place_color = XtNameToWidget(place_dlg, "*PlaceChangeDialog_Color");
    place_tag = XtNameToWidget(place_dlg, "*PlaceChangeDialog_Tag");
}








