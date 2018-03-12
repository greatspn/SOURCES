#define ___TRANSDIALOG___
#	include <Mrm/MrmPublic.h>
#	include "global.h"
#	include "parse.h"
#	include "draw.h"
#	include "trans.h"
#	include "mdgrammardialog.h"
#	include "tag.h"
#	include "rate.h"
#	include "res.h"
#include "transdialog.h"

static	Widget Trans_dlg, trans_rc3, trans_ex, trans_im, trans_det,
        trans_help, trans_label1,  trans_text,
        trans_markdep, trans_infinite, trans_loaddep,
        trans_nserv, trans_pri, trans_tag, trans_color, trans_list, trans_scale;

static int trans_kind, extrans_kind;
static char  *trans_ratepar_str, *trans_markdep_str;
static float trans_values[127];
static int old_scale, last_selection;
static Boolean update_required, process_focus, process_one;
static struct rpar_object *has_ratepar;
static struct trans_object *dialog_trans;

static void GetNewPri(char *np) {
    struct group_object *group;
    struct group_object *auxgr;
    int             ppp;

    sscanf(np, "%d", &ppp);
    dialog_trans->kind = ppp;
    group = netobj->groups;
    if (group == NULL) {
        group = (struct group_object *) emalloc(GRPOBJ_SIZE);
        netobj->groups = group;
        group->tag = (char *) emalloc(TAG_SIZE);
        sprintf(group->tag, "G%d", ++group_num);
        group->next = NULL;
        group->movelink = NULL;
        group->pri = ppp;
        group->center.x = 0;
        group->center.y = 0;
        group->trans = dialog_trans;
        dialog_trans->next = NULL;
        DrawTrans(dialog_trans, OR);
        return;
    }
    if (ppp < group->pri) {
        group = (struct group_object *) emalloc(GRPOBJ_SIZE);
        group->tag = (char *) emalloc(TAG_SIZE);
        sprintf(group->tag, "G%d", ++group_num);
        group->next = netobj->groups;
        netobj->groups = group;
        group->movelink = NULL;
        group->pri = ppp;
        group->center.x = 0;
        group->center.y = 0;
        group->trans = dialog_trans;
        dialog_trans->next = NULL;
        DrawTrans(dialog_trans, OR);
        return;
    }
    while ((group->next != NULL) && (ppp >= group->next->pri))
        group = group->next;
    if (ppp == group->pri) {
        dialog_trans->next = group->trans;
        group->trans = dialog_trans;
        DrawTrans(dialog_trans, OR);
        return;
    }
    auxgr = (struct group_object *) emalloc(GRPOBJ_SIZE);
    auxgr->next = group->next;
    group = group->next = auxgr;
    group->tag = (char *) emalloc(TAG_SIZE);
    sprintf(group->tag, "G%d", ++group_num);
    group->movelink = NULL;
    group->center.x = 0;
    group->center.y = 0;
    group->pri = ppp;
    group->trans = dialog_trans;
    dialog_trans->next = NULL;
    DrawTrans(dialog_trans, OR);
    if (rate_visible)
        ShowTransRate(dialog_trans, OR);
}


static void ScaleCB(Widget w, XtPointer closure, XtPointer call_data) {
    int i, scale;
    char temp[LINEMAX];
    XmString strings[127];
    Arg args[1];

    XmScaleGetValue(trans_scale, &scale);
    sprintf(temp, "%d-Server", scale);
    strings[0] = XmStringCreate(temp, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[0], XmNlabelString, strings[0]);
    XtSetValues(trans_nserv, args, 1);
    XmStringFree(strings[0]);


    if (!update_required) {
        old_scale = scale;
        return;
    }

    printf("Scale %d      old_scale %d\n", scale, old_scale);
    if (scale > old_scale) {
        for (i = 0; i < scale - old_scale; i++) {
            sprintf(temp, "%4d --> %f", i + old_scale + 1, 1.0);
            strings[i] = XmStringCreate(temp, XmSTRING_DEFAULT_CHARSET);
        }
        XmListAddItems(trans_list, strings, scale - old_scale, 0);
        for (i = 0; i < scale - old_scale; i++)
            XmStringFree(strings[i]);
    }
    else
        XmListDeleteItemsPos(trans_list, old_scale - scale, scale + 1);

    old_scale = scale;
}

static void SelectOneCB(Widget w, XtPointer closure, XmListCallbackStruct *call_data) {
    char temp[LINEMAX];

    process_one = TRUE;
    if (call_data->item_position == 1) {
        if (has_ratepar != NULL)
            sprintf(temp, "%s", has_ratepar->tag);
        else
            sprintf(temp, "%e", trans_values[call_data->item_position - 1]);

    }
    else
        sprintf(temp, "%e", trans_values[call_data->item_position - 1]);
    last_selection = call_data->item_position;
    UpdateTransChangeDialogLabel(last_selection == 1 ? LABEL_RATE : LABEL_WEIGHT);
    XtSetSensitive(trans_text, TRUE);
    XmTextSetString(trans_text, temp);
    XmProcessTraversal(trans_text, XmTRAVERSE_CURRENT);
}

static void ProcessCRCB(Widget w, XtPointer closure, XmAnyCallbackStruct *call_data) {

    XmString strings[1];

    if (call_data->reason == XmCR_ACTIVATE /* process_one */) {
        char *temp, mytemp[LINEMAX];
        float rate;
        struct rpar_object *rp;
        process_one = FALSE;
        if (process_focus) {
            temp = XmTextGetString(trans_text);

            if (last_selection == 1) {
                if (ParseRate(temp, &rate)) {
                    if (ParseRatePar(temp, &rp)) {
                        ShowErrorDialog("Rate or existing rate parameter required", Trans_dlg);
                        XtFree(temp);
                        return ;
                    }
                    else {
                        has_ratepar = rp;
                        sprintf(mytemp, "Rate Par --> %s", rp->tag);
                        strcpy(trans_ratepar_str, rp->tag);
                    }
                }
                else {
                    has_ratepar = NULL;
                    sprintf(mytemp, "Rate --> %e", rate);
                }
            }
            else {
                if (ParseRate(temp, &rate)) {
                    ShowErrorDialog("Positive weight required", Trans_dlg);
                    XtFree(temp);
                    return ;
                }
                sprintf(mytemp, "%4d --> %e", last_selection, rate);
            }
            puts(mytemp);
            trans_values[last_selection - 1] = rate;
            strings[0] = XmStringCreate(mytemp, XmSTRING_DEFAULT_CHARSET);
            XmListReplaceItemsPos(trans_list, strings, 1, last_selection);
            XmStringFree(strings[0]);
            XmTextSetString(trans_text, "");
            XmProcessTraversal(trans_list, XmTRAVERSE_CURRENT);
            XtSetSensitive(trans_text, FALSE);
            XtFree(temp);
        }
    }

    if (process_focus)
        UpdateTransChangeDialogLabel(LABEL_EMPTY);

}

static void SizeEnablings(void) {
    Arg args[1];

    XtSetArg(args[0], XmNheight, HEIGHT_ENABLINGS);
    XtSetValues(Trans_dlg, args, 1);

}

static void SizeNoEnablings(void) {
    Arg args[1];

    XtSetArg(args[0], XmNheight, HEIGHT_NO_ENABLINGS);
    XtSetValues(Trans_dlg, args, 1);
}



void HideTransChangeDialog(void) {
    XtUnmanageChild(Trans_dlg);
    HideMDGrammarDialog();
    free(trans_ratepar_str);
    free(trans_markdep_str);
}


static void UpdateTransChangeDialogLabel(int p) {
    XmString labelTitle = NULL;
    Arg args[4];

    switch (p) {
    case LABEL_RATE:
        labelTitle = XmStringCreate("Rate or Rate Parameter:", XmSTRING_DEFAULT_CHARSET);
        break;
    case LABEL_MDEP:
        labelTitle = XmStringCreate("Marking Dependend Definition :", XmSTRING_DEFAULT_CHARSET);
        break;
    case LABEL_LOAD:
        labelTitle = XmStringCreate("Load :", XmSTRING_DEFAULT_CHARSET);
        break;
    case LABEL_EMPTY:
        labelTitle = XmStringCreate("", XmSTRING_DEFAULT_CHARSET);
        break;
    case LABEL_WEIGHT:
        labelTitle = XmStringCreate("Weight :", XmSTRING_DEFAULT_CHARSET);
        break;

    }
    XtSetArg(args[0], XmNlabelString, labelTitle);
    XtSetValues(trans_label1, args, 1);
    XmStringFree(labelTitle);
}

static void UpdateControls(int kind) {
    int i;
    XmString XmTemp;
    char temp[200];
    Arg args[1];

    process_focus = dialog_trans->enabl < 0;
    has_ratepar = dialog_trans->rpar;

    for (i = 0; i < 127; i++)
        trans_values[i] = 1.0;

    if (dialog_trans->tag != NULL)
        XmTextFieldSetString(trans_tag, dialog_trans->tag);
    else
        XmTextFieldSetString(trans_tag, "");

    if (dialog_trans->color != NULL)
        XmTextFieldSetString(trans_color, dialog_trans->color);
    else
        XmTextFieldSetString(trans_color, "");

    switch (kind) {
        Arg args[1];

    case TRANS_EXP:
        /*SizeEnablings();*/
        XmListDeleteAllItems(trans_list);
        XtMapWidget(trans_rc3);
        XtMapWidget(trans_help);
        XtUnmapWidget(trans_pri);

        if (dialog_trans->mark_dep != NULL) {
            XtSetSensitive(trans_scale, FALSE);
            XtSetSensitive(trans_list, FALSE);
            XtSetSensitive(trans_text, TRUE);
            XtSetArg(args[0], XmNeditMode, XmMULTI_LINE_EDIT);
            XtSetValues(trans_text, args, 1);
            XmToggleButtonGadgetSetState(trans_markdep, TRUE, TRUE);
            UpdateTransChangeDialogLabel(LABEL_MDEP);
            strcpy(trans_markdep_str, dialog_trans->mark_dep);
            XmTextSetString(trans_text, trans_markdep_str);
            extrans_kind = MARKDEP;
            dialog_trans->enabl = 1; /* 15/12/98 Force S.S. semantics in m.d. transitions */
        }
        else {
            XmToggleButtonGadgetSetState(trans_infinite, dialog_trans->enabl == 0, TRUE);
            XmToggleButtonGadgetSetState(trans_nserv, dialog_trans->enabl > 0, TRUE);
            XmToggleButtonGadgetSetState(trans_loaddep, dialog_trans->enabl < 0, TRUE);
            UpdateTransChangeDialogLabel(dialog_trans->enabl >= 0 ? LABEL_RATE : LABEL_EMPTY);
            XtSetArg(args[0], XmNeditMode, XmSINGLE_LINE_EDIT);
            XtSetValues(trans_text, args, 1);

            if (dialog_trans->enabl == 0) {
                XtSetSensitive(trans_scale, FALSE);
                XtSetSensitive(trans_list, FALSE);
                XtSetSensitive(trans_text, TRUE);
                if (dialog_trans->rpar == NULL) {
                    sprintf(trans_ratepar_str, "%e", dialog_trans->fire_rate.ff);
                    XmTextSetString(trans_text, trans_ratepar_str);
                }
                else {
                    strcpy(trans_ratepar_str, dialog_trans->rpar->tag);
                    XmTextSetString(trans_text, trans_ratepar_str);
                }
                extrans_kind = INFINITE;
            }
            else {
                if (dialog_trans->enabl > 0) {
                    XtSetSensitive(trans_scale, TRUE);
                    XtSetSensitive(trans_list, FALSE);
                    XtSetSensitive(trans_text, TRUE);
                    if (dialog_trans->rpar == NULL) {
                        sprintf(trans_ratepar_str, "%e", dialog_trans->fire_rate.ff);
                        XmTextSetString(trans_text, trans_ratepar_str);
                    }
                    else {
                        strcpy(trans_ratepar_str, dialog_trans->rpar->tag);
                        XmTextSetString(trans_text, trans_ratepar_str);
                    }
                    XmScaleSetValue(trans_scale, dialog_trans->enabl);
                    old_scale = dialog_trans->enabl;
                    extrans_kind = NSERVE;
                }
                else {
                    int i;
                    char temp[LINEMAX];
                    XmString strings[127];

                    XmScaleSetValue(trans_scale, -dialog_trans->enabl);
                    update_required = TRUE;
                    XtSetSensitive(trans_list, TRUE);
                    XtSetSensitive(trans_scale, TRUE);
                    XmTextSetString(trans_text, "");
                    XtSetSensitive(trans_text, FALSE);
                    XmListDeleteAllItems(trans_list);

                    for (i = 0; i < (-dialog_trans->enabl); i++) {
                        if (i != 0)
                            sprintf(temp, "%4d --> %e", i + 1, dialog_trans->fire_rate.fp[i]);
                        else {
                            if (dialog_trans->rpar == NULL)
                                sprintf(temp, "Rate --> %e", dialog_trans->fire_rate.fp[i]);
                            else
                                sprintf(temp, "Rate Par --> %s", dialog_trans->rpar->tag);
                        }

                        strings[i] = XmStringCreate(temp, XmSTRING_DEFAULT_CHARSET);
                        trans_values[i] = dialog_trans->fire_rate.fp[i];
                    }
                    XmListAddItems(trans_list, strings, -dialog_trans->enabl, 0);
                    for (i = 0; i < -dialog_trans->enabl; i++)
                        XmStringFree(strings[i]);
                    old_scale = -dialog_trans->enabl;
                    extrans_kind = LOADDEP;
                }
            }
        }
        break;
    default:
        XtSetSensitive(trans_text, TRUE);
        if (kind != TRANS_DET) {
            XtMapWidget(trans_pri);
            UpdateTransChangeDialogLabel(LABEL_WEIGHT);
            XmScaleSetValue(trans_pri, dialog_trans->kind == DETERMINISTIC ? 1 : dialog_trans->kind);
        }
        else {
            XtUnmapWidget(trans_pri);
            UpdateTransChangeDialogLabel(LABEL_RATE);
        }
        /*			SizeNoEnablings();*/
        XtUnmapWidget(trans_rc3);
        XtSetArg(args[0], XmNeditMode, XmSINGLE_LINE_EDIT);
        XtSetValues(trans_text, args, 1);
        XtUnmapWidget(trans_help);
        if (dialog_trans->rpar == NULL) {
            sprintf(trans_ratepar_str, "%e", dialog_trans->fire_rate.ff);
            XmTextSetString(trans_text, trans_ratepar_str);
        }
        else {
            strcpy(trans_ratepar_str, dialog_trans->rpar->tag);
            XmTextSetString(trans_text, trans_ratepar_str);
        }

    }
    XmScaleGetValue(trans_scale, &i);
    sprintf(temp, "%d-Server", i);
    XmTemp = XmStringCreate(temp, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[0], XmNlabelString, XmTemp);
    XtSetValues(trans_nserv, args, 1);
    XmStringFree(XmTemp);

}


void ShowTransChangeDialog(struct trans_object *trans, int focus) {
    Arg args[1];
    char *insert;
    int insertlen;
    Widget focusTo;
    int convert = 0;


    dialog_trans = trans;
    XtSetArg(args[0], XmNset, trans->kind == EXPONENTIAL);
    XtSetValues(trans_ex, args, 1);
    XtSetArg(args[0], XmNset, trans->kind != EXPONENTIAL && trans->kind != DETERMINISTIC);
    XtSetValues(trans_im, args, 1);
    XtSetArg(args[0], XmNset, trans->kind == DETERMINISTIC);
    XtSetValues(trans_det, args, 1);
    switch (focus) {
    case 0:
        focusTo = trans_tag;
        break;
    case 1:
        focusTo = trans_color;
        break;
    default:
        focusTo = trans_text;
    }

    trans_kind = trans->kind;

    trans_ratepar_str = emalloc(RATEPAR_MAX_SIZE);
    trans_markdep_str = emalloc(MDEP_MAX_SIZE);
    strcpy(trans_ratepar_str, "1.000000");
    strcpy(trans_markdep_str, "");
    old_scale = 1;

    XmScaleSetValue(trans_scale, old_scale);
    XtManageChild(Trans_dlg);
    if (trans->kind ==  EXPONENTIAL)
        SizeEnablings();
    else
        SizeNoEnablings();


    switch (trans_kind) {
    case EXPONENTIAL:
        convert = TRANS_EXP;
        break;
    case DETERMINISTIC:
        convert = TRANS_DET;
        break;
    case IMMEDIATE:
        convert = TRANS_IMM;
        break;
    };

    UpdateControls(convert);

    ShowMDGrammarDialog();
    XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);

    switch (focus) {
    case 0:
    case 1:
        insert = XmTextFieldGetString(focusTo);
        insertlen = strlen(insert);
        XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
        XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
        break;
    default:

        insert = XmTextGetString(focusTo);
        insertlen = strlen(insert);
        XmTextSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
        XmTextSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);

    }
    XtFree(insert);
}

static void MarkDepCB(Widget w, XtPointer closure, XmToggleButtonCallbackStruct *call_data) {
    char temp[200];
    Arg args[1];
    int i;
    XmString XmTemp;
    char *tmpstr;

    process_focus = FALSE;
    if ((extrans_kind == INFINITE) || (extrans_kind == NSERVE)) {
        tmpstr = XmTextGetString(trans_text);
        strcpy(trans_ratepar_str, tmpstr);
        XtFree(tmpstr);
    }

    if (call_data->set) {
        if (w == trans_markdep) {
            XtSetArg(args[0], XmNeditMode, XmMULTI_LINE_EDIT);
            XtSetValues(trans_text, args, 1);
            XtSetSensitive(trans_scale, FALSE);
            XtSetSensitive(trans_list, FALSE);
            XtSetSensitive(trans_text, TRUE);
            UpdateTransChangeDialogLabel(LABEL_MDEP);
            XmTextSetString(trans_text, trans_markdep_str);
            extrans_kind = MARKDEP;
        }
        else {
            XtSetArg(args[0], XmNeditMode, XmSINGLE_LINE_EDIT);
            XtSetValues(trans_text, args, 1);
            if (w == trans_infinite) {
                XtSetSensitive(trans_scale, FALSE);
                XtSetSensitive(trans_list, FALSE);
                XtSetSensitive(trans_text, TRUE);
                XmTextSetString(trans_text, trans_ratepar_str);
                extrans_kind = INFINITE;
                UpdateTransChangeDialogLabel(LABEL_RATE);
            }
            else if (w == trans_loaddep) {
                int enval;

                UpdateTransChangeDialogLabel(LABEL_EMPTY);
                update_required = TRUE;
                process_focus = TRUE;
                XmListDeleteAllItems(trans_list);
                XtSetSensitive(trans_scale, TRUE);
                XtSetSensitive(trans_list, TRUE);
                XtSetSensitive(trans_text, FALSE);
                XmTextSetString(trans_text, "");

                enval = old_scale;
                {
                    char temp[1000];
                    XmString strings[127];
                    int i;

                    for (i = 0; i < enval; i++) {
                        if (i)
                            sprintf(temp, "%4d --> %e", i + 1, trans_values[i]);
                        else {
                            if (has_ratepar != NULL)
                                sprintf(temp, "Rate Par --> %s", has_ratepar->tag);
                            else
                                sprintf(temp, "Rate --> %e", trans_values[i]);
                        }
                        strings[i] = XmStringCreate(temp, XmSTRING_DEFAULT_CHARSET);
                    }
                    XmListAddItems(trans_list, strings, enval, 0);
                    for (i = 0; i < enval; i++)
                        XmStringFree(strings[i]);
                    extrans_kind = LOADDEP;
                }
            }
            else if (w == trans_nserv) {
                update_required = FALSE;
                UpdateTransChangeDialogLabel(LABEL_RATE);
                XtSetSensitive(trans_scale, TRUE);
                XmScaleSetValue(trans_scale, old_scale);
                XtSetSensitive(trans_list, FALSE);
                XtSetSensitive(trans_text, TRUE);
                XmTextSetString(trans_text, trans_ratepar_str);
                extrans_kind = NSERVE;
            }

        }
        XmScaleGetValue(trans_scale, &i);
        sprintf(temp, "%d-Server", i);
        XmTemp = XmStringCreate(temp, XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[0], XmNlabelString, XmTemp);
        XtSetValues(trans_nserv, args, 1);
        XmStringFree(XmTemp);

    }

}

static void CancelTransDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideTransChangeDialog();
}



static void SetTransKindCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    if (!call_data->set)
        return;

    switch (closure) {
    case TRANS_EXP:
        trans_kind = EXPONENTIAL;
        break;
    case TRANS_DET:
        trans_kind = DETERMINISTIC;
        break;
    case TRANS_IMM:
        trans_kind = IMMEDIATE;
        break;
    }

    if (trans_kind ==  EXPONENTIAL)
        SizeEnablings();
    else
        SizeNoEnablings();

    UpdateControls(closure);
    XmProcessTraversal(trans_list, XmTRAVERSE_CURRENT);

}

static void ShowMDHelpCB(Widget w, XtPointer closure, XtPointer call_data) {
    SetVisibleMDGrammarDialog();
    ShowMDGrammarDialog();
}

static void SetTransTypeCB(Widget w, int closure, XtPointer call_data) {
    float rate;
    char *text;
    struct rpar_object *rp;
    int enabling, priority;
    char linebuf[LINEMAX];
    char temp[LINEMAX];
    char *tag, *color;

    if (closure == FLAG_ABORT && extrans_kind == LOADDEP && trans_kind == EXPONENTIAL)
        return;

    XmScaleGetValue(trans_scale, &enabling);

    if (trans_kind != EXPONENTIAL)
        enabling = 0;
    else
        switch (extrans_kind) {
        case MARKDEP: /* 15/12/98 Force enabling to 1 : s.s. semantics in MD trans.*/
            enabling = 1;
            break;
        case INFINITE:
            enabling = 0;
            break;
        case LOADDEP:
            enabling = -enabling;
            break;

        }
    printf("SetTransTypeCB  enabling=%d\n", enabling);

    text = XmTextFieldGetString(trans_tag);

    if (ParseTransTag(text, &tag, trans_tag, Trans_dlg, dialog_trans)) {
        XtFree(text);
        return;
    }
    tag = strdup(tag);
    XtFree(text);

    text = XmTextFieldGetString(trans_color);

    if (ParseColor(text, &color, trans_color, Trans_dlg)) {
        XtFree(text);
        free(tag);
        return;
    }
    if (color != NULL)
        color = strdup(color);
    XtFree(text);




    text = XmTextGetString(trans_text);
    XmScaleGetValue(trans_pri, &priority);
    printf("SetTransTypeCB  priority=%d\n", priority);

    printf("SetTransTypeCB  text=%s\n", text);
    rate = 1.;
    rp = NULL;

    printf("Tipo trans = %d", trans_kind);

    if ((trans_kind >= IMMEDIATE && trans_kind < DETERMINISTIC) || trans_kind == DETERMINISTIC || (trans_kind == EXPONENTIAL && (extrans_kind == INFINITE || extrans_kind == NSERVE))) {
        puts("Ci provo");
        if (ParseRate(text, &rate)) {
            puts("primo");
            if (ParseRatePar(text, &rp)) {
                puts("secondo");
                free(tag);
                if (color != NULL)
                    free(color);
                XtFree(text);
                ShowErrorDialog("Rate or existing rate parameter required", Trans_dlg);
                XmProcessTraversal(trans_text, XmTRAVERSE_CURRENT);

                return;
            }
        }
    }


    printf("SetTransTypeCB  rate=%e\n", rate);
    printf("SetTransTypeCB  ratepar=%p\n", rp);

    HideTransChangeDialog();

    RemoveTrans(dialog_trans);

    if (tag_visible)
        ShowTransTag(dialog_trans, &mainDS, 0.0, 0.0, CLEAR);

    DrawTrans(dialog_trans, CLEAR);

    if (rate_visible)
        ShowTransRate(dialog_trans, CLEAR);


    if (dialog_trans->mark_dep != NULL)
        free(dialog_trans->mark_dep);

    if (dialog_trans->tag != NULL)
        free(dialog_trans->tag);
    dialog_trans->tag = tag;

    dialog_trans->mark_dep = NULL;

    dialog_trans->rpar = NULL;

    if (dialog_trans->enabl < 0)
        free(dialog_trans->fire_rate.fp);

    if (dialog_trans->color != NULL) {
        free(dialog_trans->color);
    }
    else {
        dialog_trans->colpos.x = trans_length / 2 + 3;
        dialog_trans->colpos.y = -7;
    }
    dialog_trans->color = NULL;
    dialog_trans->lisp = NULL;
    if (color != NULL) {
        puts(color);
        dialog_trans->color = emalloc(strlen(color) + 1);
        strcpy(dialog_trans->color, color);
    }


    if (dialog_trans->kind != EXPONENTIAL && dialog_trans->kind != DETERMINISTIC) {
        struct group_object *g_p, *group;

        if ((group = netobj->groups) == NULL)
            return;
        while (group != NULL && group->trans == NULL) {
            g_p = group;
            group = group->next;
            netobj->groups = group;
            free(g_p);
        }
        while (group != NULL && group->next != NULL) {
            if (group->next->trans != NULL)
                group = group->next;
            else {
                g_p = group->next;
                group->next = g_p->next;
                free(g_p);
            }
        }
    }
    SetModify();
    ClearRes();
    switch (trans_kind) {
    case EXPONENTIAL:
        puts("Exponential");
        dialog_trans->kind = EXPONENTIAL;
        dialog_trans->next = netobj->trans;
        dialog_trans->enabl = enabling;
        if (extrans_kind == MARKDEP) {
            puts("MARKDEP");
            puts(text);
            dialog_trans->mark_dep = strdup(text);
        }
        else {
            puts("NON MARKDEP");
            if (enabling < 0) {
                dialog_trans->fire_rate.fp = (float *) emalloc(-enabling * sizeof(float));
                memcpy(dialog_trans->fire_rate.fp, trans_values, -enabling * sizeof(float));
                dialog_trans->rpar = has_ratepar;
            }
            else {
                dialog_trans->fire_rate.ff = rate;
                dialog_trans->rpar = rp;
            }
        }
        netobj->trans = dialog_trans;
        DrawTrans(dialog_trans, OR);
        if (rate_visible)
            ShowTransRate(dialog_trans, OR);
        sprintf(linebuf, "transition %s is now exponential (inf-server)", dialog_trans->tag);
        put_msg(1, linebuf);
        break;
    case DETERMINISTIC:
        dialog_trans->kind = DETERMINISTIC;
        dialog_trans->next = netobj->trans;
        netobj->trans = dialog_trans;
        dialog_trans->fire_rate.ff = rate;
        dialog_trans->rpar = rp;
        dialog_trans->enabl = 0;
        DrawTrans(dialog_trans, OR);
        if (rate_visible)
            ShowTransRate(dialog_trans, OR);
        sprintf(linebuf, "transition %s is now deterministic", dialog_trans->tag);
        put_msg(1, linebuf);
        break;
    case IMMEDIATE:
    default:
        sprintf(temp, "%d", priority);
        dialog_trans->enabl = 1;
        dialog_trans->fire_rate.ff = rate;
        dialog_trans->rpar = rp;
        sprintf(linebuf, "transition %s is now immediate", dialog_trans->tag);
        put_msg(1, linebuf);
        GetNewPri(temp);
        break;
    }
    XtFree(text);
    menu_action();
}

void RegisterTransChangeDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"ScaleCB"				, (XtPointer)ScaleCB},
        {"SelectOneCB"			, (XtPointer)SelectOneCB},
        {"ProcessCRCB"			, (XtPointer)ProcessCRCB},
        {"SetTransKindCB"		, (XtPointer)SetTransKindCB},
        {"MarkDepCB"			, (XtPointer)MarkDepCB},
        {"CancelTransDialogCB"	, (XtPointer)CancelTransDialogCB},
        {"SetTransTypeCB"		, (XtPointer)SetTransTypeCB},
        {"ShowMDHelpCB"			, (XtPointer)ShowMDHelpCB},
        {"FLAG_PROCESS"			, (XtPointer)FLAG_PROCESS},
        {"FLAG_ABORT"			, (XtPointer)FLAG_ABORT},
        {"TRANS_EXP"			, (XtPointer)TRANS_EXP},
        {"TRANS_DET"			, (XtPointer)TRANS_DET},
        {"TRANS_IMM"			, (XtPointer)TRANS_IMM},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterArcChangeDialog names\n");
}

void InitTransChangeDialog(void) {
    int i;


    Trans_dlg = XtNameToWidget(frame_w, "*TransChangeDialog");
    trans_im = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Imm");
    trans_ex = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Exp");
    trans_det = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Det");
    trans_help = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Help");
    trans_rc3 = XtNameToWidget(Trans_dlg, "*TransChangeDialog_RC3");
    trans_infinite = XtNameToWidget(trans_rc3, "*TransChangeDialog_Infinite");
    trans_markdep = XtNameToWidget(trans_rc3, "*TransChangeDialog_MarkDep");
    trans_nserv = XtNameToWidget(trans_rc3, "*TransChangeDialog_NServ");
    trans_loaddep = XtNameToWidget(trans_rc3, "*TransChangeDialog_LoadDep");

    trans_label1 = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Label1");
    trans_text = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Text");
    trans_pri = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Pri");
    trans_tag = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Tag");
    trans_color = XtNameToWidget(Trans_dlg, "*TransChangeDialog_Color");
    trans_list = XtNameToWidget(Trans_dlg, "*EnablingDialog_Scroller");
    trans_scale = XtNameToWidget(Trans_dlg, "*EnablingDialog_Scale");

    for (i = 0; i < 127; i++)
        trans_values[i] = 1.0;
    old_scale = 1;

    /*

    XtAddCallback(trans_scale, XmNvalueChangedCallback, ScaleCB, NULL);
    XtAddCallback(trans_list, XmNsingleSelectionCallback, SelectOneCB, NULL);
    XtAddCallback(trans_nserv, XmNvalueChangedCallback, MarkDepCB, 'r');
    XtAddCallback(trans_infinite, XmNvalueChangedCallback, MarkDepCB, 'r');
    XtAddCallback(trans_markdep, XmNvalueChangedCallback, MarkDepCB, 'r');
    XtAddCallback(trans_loaddep, XmNvalueChangedCallback, MarkDepCB, 'r');
    XtAddCallback(trans_ex, XmNvalueChangedCallback, SetTransKindCB, (int) EXPONENTIAL);
    XtAddCallback(trans_im, XmNvalueChangedCallback, SetTransKindCB, (int) IMMEDIATE);
    XtAddCallback(trans_det, XmNvalueChangedCallback, SetTransKindCB, (int) DETERMINISTIC);
    XtAddCallback(trans_text, XmNactivateCallback, ProcessCRCB, NULL);
    XtAddCallback(trans_text, XmNlosingFocusCallback, ProcessCRCB, NULL);

    XtAddCallback(trans_cancel, XmNactivateCallback, CancelTransDialogCB, NULL);
    XtAddCallback(trans_ok, XmNactivateCallback, SetTransTypeCB, FLAG_PROCESS);
    XtAddCallback(trans_tag, XmNactivateCallback, SetTransTypeCB, NULL);
    XtAddCallback(trans_color, XmNactivateCallback, SetTransTypeCB, FLAG_PROCESS);
    XtAddCallback(trans_text, XmNactivateCallback, SetTransTypeCB, FLAG_ABORT);
    XtAddCallback(trans_help, XmNactivateCallback, ShowMDHelpCB, NULL);
    */
}
