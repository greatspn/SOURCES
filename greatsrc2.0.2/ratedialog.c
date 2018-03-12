#define ___RATEDIALOG___
#	include "global.h"
#	include "parse.h"
#	include "rate.h"
#	include "res.h"
#	include <Mrm/MrmPublic.h>
#include "ratedialog.h"

static struct rpar_object *cur_rpar = NULL;

static Widget   rate_dlg, rate_rate, rate_tag;


void HideRateChangeDialog(void) {
    XtUnmanageChild(rate_dlg);
}

static void SetRateCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *text, *tag;
    float rateVal;
    Boolean UpdateRequired;


    text = XmTextFieldGetString(rate_tag);
    puts(text);
    if (ParseRateParTag(text, &tag, rate_tag, rate_dlg, cur_rpar)) {
        XtFree(text);
        return ;
    }

    tag = strdup(tag);
    XtFree(text);

    text = XmTextFieldGetString(rate_rate);

    if (ParseRate(text, &rateVal)) {
        free(tag);
        XtFree(text);
        ShowErrorDialog("Rate must be a positive number !", rate_dlg);
        XmProcessTraversal(rate_rate, XmTRAVERSE_CURRENT);
        return ;
    }

    XtFree(text);

    if (cur_rpar == NULL) {
        struct rpar_object *l_rpar;

        cur_rpar = (struct rpar_object *) emalloc(RPAOBJ_SIZE);
        cur_rpar->tag = NULL;
        cur_rpar->center.x = fix_x;
        cur_rpar->center.y = fix_y;
        cur_rpar->next = NULL;
        cur_rpar->layer = NewLayerList(CURRENTVIEWANDWHOLE, NULL);

        if (netobj->rpars == NULL)
            l_rpar = NULL;
        else
            for (l_rpar = netobj->rpars; l_rpar->next != NULL; l_rpar = l_rpar->next);

        if (l_rpar != NULL)
            l_rpar->next = cur_rpar;
        else
            netobj->rpars = cur_rpar;

        UpdateRequired = FALSE;

    }
    else
        ShowRpar(cur_rpar, CLEAR);

    XFlush(XtDisplay(rate_dlg));

    if (UpdateRequired) {
        ShowRate(FALSE);
    }

    if (cur_rpar->tag != NULL)
        free(cur_rpar->tag);

    cur_rpar->tag = tag;
    cur_rpar->value = rateVal;

    if (UpdateRequired) {
        if (rate_visible)
            ShowRate(TRUE);
    }

    ShowRpar(cur_rpar, OR);
    ClearRes();
    menu_action();
    SetModify();
    HideRateChangeDialog();
}

static void CancelRateDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideRateChangeDialog();
}

void ShowRateChangeDialog(struct rpar_object *rp, int focus) {
    Arg          args[1];
    char		tempString[LINEMAX];
    Widget		focusTo;
    XmString    XmTemp;
    char *insert;
    int insertlen;

    cur_rpar = rp;
    if (cur_rpar != NULL) {
        XmTemp = XmStringCreate("Change Rate Parameter", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);
        XtSetValues(rate_dlg, args, 1);

        if (cur_rpar->tag != NULL)
            XmTextFieldSetString(rate_tag, cur_rpar->tag);
        else
            XmTextFieldSetString(rate_tag, "");

        sprintf(tempString, "%f", cur_rpar->value);
        XmTextFieldSetString(rate_rate, tempString);

        switch (focus) {
        case 0:
            focusTo = rate_tag;
            break;
        default:
            focusTo = rate_rate;
        }

    }
    else {
        XmTemp = XmStringCreate("Create Rate Parameter", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNdialogTitle, XmTemp);
        XtSetValues(rate_dlg, args, 1);

        XmTextFieldSetString(rate_tag, "");
        XmTextFieldSetString(rate_rate, "");
        focusTo = rate_tag;
    }
    XmStringFree(XmTemp);
    XtManageChild(rate_dlg);

    XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);

    insert = XmTextFieldGetString(focusTo);
    insertlen = strlen(insert);
    XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
    XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
    XtFree(insert);

}

void RegisterRateDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"CancelRateDialogCB", (XtPointer)CancelRateDialogCB},
        {"SetRateCB"		, (XtPointer)SetRateCB},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterArcChangeDialog names\n");
}

void InitRateChangeDialog(void) {
    rate_dlg = XtNameToWidget(frame_w, "*RateChangeDialog");
    rate_rate = XtNameToWidget(rate_dlg, "*RateChangeDialog_Rate");
    rate_tag = XtNameToWidget(rate_dlg, "*RateChangeDialog_Tag");
}


