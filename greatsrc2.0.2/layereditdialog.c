#define ___LAYEREDITDIALOG___
#include "global.h"
#include "showgdi.h"
#include "layer.h"
#include "layerviewdialog.h"
#include "draw.h"
#include <Mrm/MrmPublic.h>
#include <ctype.h>
#include "parse.h"
#include "layereditdialog.h"

static Widget  layer_edit_text, layer_edit_add,
       layer_edit_delete, layer_edit_done, layer_edit_rename,
       layer_edit_list, layer_edit_dlg, layer_edit_ok,
       layer_edit_cancel, layer_edit_view;

static LayerPun editLayerList;
static int skip;

Boolean TestLayerInEditLayerList(int which) {
    return	TestLayer(which, editLayerList);
}

void ReallocEditLayerList(void) {
    editLayerList = LayerRealloc(editLayerList);
}


void HideEditLayerDialog(void) {
    XtUnmanageChild(layer_edit_dlg);
}

void SetAddAsDefault(void) {
    Arg args[1];

    XtSetArg(args[0], XmNdefaultButton, layer_edit_add);
    XtSetValues(layer_edit_dlg, args, 1);
    XtSetArg(args[0], XmNshowAsDefault, FALSE);
    XtSetValues(layer_edit_ok, args, 1);
    XtSetArg(args[0], XmNshowAsDefault, TRUE);
    XtSetValues(layer_edit_add, args, 1);
    XmProcessTraversal(layer_edit_add, XmTRAVERSE_CURRENT);
}

void SetOkAsDefault(void) {
    Arg args[1];

    XtSetArg(args[0], XmNdefaultButton, layer_edit_ok);
    XtSetValues(layer_edit_dlg, args, 1);
    XtSetArg(args[0], XmNshowAsDefault, FALSE);
    XtSetValues(layer_edit_add, args, 1);
    XtSetArg(args[0], XmNshowAsDefault, TRUE);
    XtSetValues(layer_edit_ok, args, 1);
}

void ShowEditLayerDialog(void) {
    XtSetSensitive(layer_edit_text, 0);
    XtSetSensitive(layer_edit_ok, 0);
    XtSetSensitive(layer_edit_cancel, 0);
    XtSetSensitive(layer_edit_rename, 0);
    XtSetSensitive(layer_edit_delete, 0);
    XtManageChild(layer_edit_dlg);
    SetAddAsDefault();
}

static void EditLayerDoneCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideEditLayerDialog();
}

static void LayerEditViewCB(Widget w, XtPointer closure, XtPointer call_data) {
    ShowViewLayerDialog();
}

void EditLayerListSelectCB(Widget w, XtPointer closure, XmListCallbackStruct *call_data) {

    XtSetSensitive(layer_edit_delete, call_data->selected_item_count > 0);
    XtSetSensitive(layer_edit_rename, call_data->selected_item_count == 1);

    if (XmListPosSelected(layer_edit_list, call_data->item_position))
        SetLayer2LayerList((Layer) call_data->item_position - 1, editLayerList);
    else
        ResetLayer2LayerList((Layer) call_data->item_position - 1, editLayerList);

}


static void LayerEditDeleteCB(Widget w, XtPointer closure, XtPointer call_data) {
    Arg args[1];
    int selectedItemCount;
    int itemCount;
    XmStringTable toDelete;

    XmListDeselectPos(layer_edit_list, 1);

    ResetLayer2LayerList(WHOLE_NET_LAYER, editLayerList);

    XtSetArg(args[0], XmNselectedItemCount, &selectedItemCount);
    XtGetValues(layer_edit_list, args, 1);
    XtSetArg(args[0], XmNitemCount, &itemCount);
    XtGetValues(layer_edit_list, args, 1);
    XtSetArg(args[0], XmNselectedItems, &toDelete);
    XtGetValues(layer_edit_list, args, 1);
    XmListDeleteItems(layer_edit_list, toDelete, selectedItemCount);

    DeleteLayers();

    XtSetSensitive(layer_edit_delete, 0);
    XtSetSensitive(layer_edit_add, 1);
    XtSetSensitive(layer_edit_rename, 0);

    ResetAllLayersInViewLayerList();
    SetLayer2ViewLayerList(WHOLE_NET_LAYER);
    ResetAllLayersInList(editLayerList);

    UpdateViewDialog();
    gdiClearDrawingArea();
    RedisplayNet();
    SetAddAsDefault();
}

static void LayerEditCancelCB(Widget w, int closure, XtPointer call_data) {
    Arg args[1];
    int selectedItemCount;
    int itemCount;

    XtSetArg(args[0], XmNselectedItemCount, &selectedItemCount);
    XtGetValues(layer_edit_list, args, 1);
    XtSetArg(args[0], XmNitemCount, &itemCount);
    XtGetValues(layer_edit_list, args, 1);

    XtSetSensitive(layer_edit_delete, selectedItemCount > 0);
    XtSetSensitive(layer_edit_add, TRUE);
    XtSetSensitive(layer_edit_rename, selectedItemCount == 1);
    XtSetSensitive(layer_edit_done, 1);
    XtSetSensitive(layer_edit_list, 1);
    XtSetSensitive(layer_edit_text, 0);
    XtSetSensitive(layer_edit_cancel, 0);
    XtSetSensitive(layer_edit_ok, 0);
    XmTextSetString(layer_edit_text, "");
    SetAddAsDefault();

    if (closure == ACTION_ADD) {
        XtRemoveCallback(layer_edit_ok, XmNactivateCallback, LayerEditTrueAddCB, NULL);
        XtRemoveCallback(layer_edit_text, XmNactivateCallback, LayerEditTrueAddCB, NULL);
    }
    else {
        XtRemoveCallback(layer_edit_ok, XmNactivateCallback, LayerEditTrueRenameCB, NULL);
        XtRemoveCallback(layer_edit_text, XmNactivateCallback, LayerEditTrueRenameCB, NULL);
    }

    XtRemoveCallback(layer_edit_cancel, XmNactivateCallback, (XtCallbackProc)LayerEditCancelCB, (XtPointer)((size_t)closure));
    skip = 0;
}



static void LayerEditTrueAddCB(Widget w, XtPointer closure, XtPointer call_data) {
    XmString XmLayerName = NULL;
    Arg args[1];
    int selectedItemCount;
    int itemCount;
    char *newLayerName;
    char *newLayerNameScan;

    XtSetArg(args[0], XmNselectedItemCount, &selectedItemCount);
    XtGetValues(layer_edit_list, args, 1);
    XtSetArg(args[0], XmNitemCount, &itemCount);
    XtGetValues(layer_edit_list, args, 1);

    newLayerNameScan = LTrim(newLayerName = XmTextFieldGetString(layer_edit_text));
    if (strlen(newLayerNameScan) != 0) {
        XmLayerName = XmStringCreate(newLayerNameScan, XmSTRING_DEFAULT_CHARSET);

        if (!XmListItemExists(layer_edit_list, XmLayerName)) {
            Layer new;

            XmListAddItemUnselected(layer_edit_list, XmLayerName, itemCount + 1);
            new = NewLayer(newLayerNameScan);

            ResetAllLayersInViewLayerList();
            SetLayer2ViewLayerList(WHOLE_NET_LAYER);
            SetLayer2ViewLayerList(new);

            UpdateViewDialog();

        }
        else {
            char message[100];
            sprintf(message, "Layer %s already exist !", newLayerNameScan);
            ShowErrorDialog(message, layer_edit_dlg);
        }
    }
    else {
        char message[100];
        strcpy(message, "This is not a valid layer name !");
        ShowErrorDialog(message, layer_edit_dlg);
    }


    XtSetSensitive(layer_edit_delete, selectedItemCount > 0);
    XtSetSensitive(layer_edit_add, TRUE);
    XtSetSensitive(layer_edit_rename, selectedItemCount == 1);
    XtSetSensitive(layer_edit_done, 1);
    XtSetSensitive(layer_edit_list, 1);
    XtSetSensitive(layer_edit_text, 0);
    XtSetSensitive(layer_edit_cancel, 0);
    XtSetSensitive(layer_edit_ok, 0);
    XmTextFieldSetString(layer_edit_text, "");

    XtRemoveCallback(layer_edit_ok, XmNactivateCallback, (XtCallbackProc) LayerEditTrueAddCB, NULL);
    XtRemoveCallback(layer_edit_text, XmNactivateCallback, (XtCallbackProc) LayerEditTrueAddCB, NULL);
    XtRemoveCallback(layer_edit_cancel, XmNactivateCallback, (XtCallbackProc) LayerEditCancelCB, (XtPointer)TRUE);

    XmStringFree(XmLayerName);
    XtFree(newLayerName);

    SetAddAsDefault();
    skip = 1;
}


static void LayerEditTrueRenameCB(Widget w, XtPointer closure, XtPointer call_data) {
    XmString XmLayerName;
    Arg args[1];
    int selectedItemCount, itemCount;
    char *newLayerName;
    char *newLayerNameScan;
    XmStringTable toRename;

    XtSetArg(args[0], XmNselectedItemCount, &selectedItemCount);
    XtGetValues(layer_edit_list, args, 1);
    XtSetArg(args[0], XmNitemCount, &itemCount);
    XtGetValues(layer_edit_list, args, 1);
    XtSetArg(args[0], XmNselectedItems, &toRename);
    XtGetValues(layer_edit_list, args, 1);

    newLayerNameScan = LTrim(newLayerName = XmTextFieldGetString(layer_edit_text));
    XmLayerName = XmStringCreate(newLayerNameScan, XmSTRING_DEFAULT_CHARSET);

    if (!XmListItemExists(layer_edit_list, XmLayerName)) {
        int cur_layer;

        XmListReplaceItems(layer_edit_list, toRename, 1, &XmLayerName);
        XmListSelectItem(layer_edit_list, XmLayerName, 1);
        cur_layer = XmListItemPos(layer_edit_list, XmLayerName) - 1;
        ReleaseLayerName(cur_layer);
        NewLayerName(cur_layer, newLayerNameScan);
        UpdateViewDialog();
    }
    else {
        char message[100];
        sprintf(message, "The layer %s already exist !", newLayerNameScan);
        ShowErrorDialog(message, layer_edit_dlg);
    }

    XtSetSensitive(layer_edit_delete, selectedItemCount > 0);
    XtSetSensitive(layer_edit_add, TRUE);
    XtSetSensitive(layer_edit_rename, selectedItemCount == 1);
    XtSetSensitive(layer_edit_done, 1);
    XtSetSensitive(layer_edit_list, 1);
    XtSetSensitive(layer_edit_text, 0);
    XtSetSensitive(layer_edit_cancel, 0);
    XtSetSensitive(layer_edit_ok, 0);
    XmTextFieldSetString(layer_edit_text, "");

    XtRemoveCallback(layer_edit_ok, XmNactivateCallback, LayerEditTrueRenameCB, NULL);
    XtRemoveCallback(layer_edit_text, XmNactivateCallback, LayerEditTrueRenameCB, NULL);
    XtRemoveCallback(layer_edit_cancel, XmNactivateCallback, (XtCallbackProc) LayerEditCancelCB, (XtPointer)FALSE);

    XmStringFree(XmLayerName);
    XtFree(newLayerName);
    SetAddAsDefault();
    skip = 1;

}


static void EditLayerActionCB(Widget w, int closure, XtPointer call_data) {
    Arg args[1];

    if (skip == 1) {
        skip = 0;
        return;
    }

    XtSetSensitive(layer_edit_delete, 0);
    XtSetSensitive(layer_edit_add, 0);
    XtSetSensitive(layer_edit_rename, 0);
    XtSetSensitive(layer_edit_done, 0);
    XtSetSensitive(layer_edit_list, 0);
    XtSetSensitive(layer_edit_text, 1);
    XtSetSensitive(layer_edit_cancel, 1);
    XtSetSensitive(layer_edit_ok, 1);
    XmProcessTraversal(layer_edit_text, XmTRAVERSE_CURRENT);

    SetOkAsDefault();

    if (closure == ACTION_ADD) {
        XtAddCallback(layer_edit_ok, XmNactivateCallback, LayerEditTrueAddCB, NULL);
        XtAddCallback(layer_edit_text, XmNactivateCallback, LayerEditTrueAddCB, NULL);
    }
    else {
        char *textString;
        XmStringTable toRename;


        XtSetArg(args[0], XmNselectedItems, &toRename);
        XtGetValues(layer_edit_list, args, 1);
        XmStringGetLtoR(toRename[0], XmSTRING_DEFAULT_CHARSET, &textString);
        XmTextFieldSetString(layer_edit_text, textString);
        XmTextFieldSetInsertionPosition(layer_edit_text, strlen(textString));

        XtAddCallback(layer_edit_ok, XmNactivateCallback, LayerEditTrueRenameCB, NULL);
        XtAddCallback(layer_edit_text, XmNactivateCallback, LayerEditTrueRenameCB, NULL);

    }
    XtAddCallback(layer_edit_cancel, XmNactivateCallback, (XtCallbackProc)LayerEditCancelCB, (XtPointer)((size_t)closure));
}



void ReinitializeEditDialog(void) {
    XmString temp;

    XmListDeleteAllItems(layer_edit_list);

    temp = XmStringCreate(GetLayerName(0), XmSTRING_DEFAULT_CHARSET);
    XmListAddItem(layer_edit_list, temp, 0);
    XmStringFree(temp);

    editLayerList = LayerRealloc(editLayerList);
    ResetAllLayersInList(editLayerList);
}

void UpdateEditDialog(void) {
    register int i, ln;

    SetUpdate();;

    XmListDeleteAllItems(layer_edit_list);

    for (i = 0, ln = GetLayersNumber(); i < ln; i++) {
        XmString temp;

        temp = XmStringCreate(GetLayerName(i), XmSTRING_DEFAULT_CHARSET);
        XmListAddItem(layer_edit_list, temp, 0);
        XmStringFree(temp);
    }
    ResetUpdate();;
}

void RegisterEditLayerDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"EditLayerListSelectCB"	, (XtPointer)EditLayerListSelectCB},
        {"EditLayerActionCB"		, (XtPointer)EditLayerActionCB},
        {"EditLayerDoneCB"			, (XtPointer)EditLayerDoneCB},
        {"LayerEditDeleteCB"		, (XtPointer)LayerEditDeleteCB},
        {"LayerEditViewCB"			, (XtPointer)LayerEditViewCB},
        {"ACTION_ADD"				, (XtPointer)ACTION_ADD},
        {"ACTION_RENAME"			, (XtPointer)ACTION_RENAME},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterEditLayerDialog names\n");
}

void InitEditLayerDialog(void) {

    layer_edit_dlg = XtNameToWidget(frame_w, "*LayerEditDialog");
    layer_edit_list = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Scroller");
    layer_edit_add = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Add");
    layer_edit_done = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Done");
    layer_edit_rename = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Rename");
    layer_edit_delete = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Delete");
    layer_edit_view = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_View");
    layer_edit_ok = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Ok");
    layer_edit_cancel = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Cancel");
    layer_edit_text = XtNameToWidget(layer_edit_dlg, "*LayerEditDialog_Text");

    editLayerList = NewLayerList(ZEROINIT, NULL);
    SetLayer2LayerList(WHOLE_NET_LAYER, editLayerList);
    skip = 0;
}
