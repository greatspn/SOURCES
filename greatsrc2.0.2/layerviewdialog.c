#define ___LAYERVIEWDIALOG___
#include "global.h"
#include "layer.h"
#include "layereditdialog.h"
#include "showgdi.h"
#include "draw.h"
#include <Mrm/MrmPublic.h>
#include <assert.h>
#include "layerviewdialog.h"
#include "Canvas.h"

/*
#define assert(expr)
*/

static Widget  layer_view_rc, layer_view_dlg,
       *layers ;

static Boolean updating;
static int maxLayersButtons;
static int limit2ReallocViewButtons;


void ReallocViewLayerButton(void) {

#ifdef DEBUG_LAYERS
    printf("ReallocViewLayerButton Entering ....\n");
#endif

    if (limit2ReallocViewButtons < GetLayerListElements()) {

#ifdef DEBUG_LAYERS
        printf("ReallocViewLayerButton Realloc to  %d....\n", GetLayerListElements() * LAYERS_PER_ELEMENT);
#endif

        limit2ReallocViewButtons = GetLayerListElements();
        layers = (Widget *) realloc(layers, sizeof(Widget) * GetLayerListElements() * LAYERS_PER_ELEMENT);
        assert(layers != NULL);
    }

#ifdef DEBUG_LAYERS
    printf("ReallocViewLayerButton Exiting ....\n");
#endif
}

void SetUpdate(void) {
    updating = TRUE;
}

void ResetUpdate(void) {
    updating = FALSE;
}


void HideViewLayerDialog(void) {
    XtUnmanageChild(layer_view_dlg);
}

void ShowViewLayerDialog(void) {
    XtManageChild(layer_view_dlg);
}

static void ViewLayerUpdateViewCB(Widget w, Layer layer, XmToggleButtonCallbackStruct *call_data) {
    if (updating)
        return;

    if (call_data->set)
        SetLayer2ViewLayerList(layer);
    else
        ResetLayer2ViewLayerList(layer);


    gdiClearDrawingArea();
    redisplay_canvas();
}

static void ViewLayerEditCB(Widget w, XtPointer closure, XtPointer call_data) {
    ShowEditLayerDialog();
}

static void ViewLayerDoneCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideViewLayerDialog();
}

static void ViewLayerSelectAllCB(Widget w, XtPointer closure, XtPointer call_data) {
    register int i, ln;

    updating = TRUE;
    for (i = 0, ln = GetLayersNumber(); i < ln; i++)
        XmToggleButtonGadgetSetState(layers[i], TRUE, TRUE);
    updating = FALSE;

    SetAllLayersInViewLayerList();
    gdiClearDrawingArea();
    RedisplayNet();

}


static void ViewLayerDeselectAllCB(Widget w, XtPointer closure, XtPointer call_data) {
    register int i, ln;

    updating = TRUE;

    for (i = 0, ln = GetLayersNumber(); i < ln; i++)
        XmToggleButtonGadgetSetState(layers[i], FALSE, TRUE);
    updating = FALSE;
    ResetAllLayersInViewLayerList();
    gdiClearDrawingArea();
    RedisplayNet();
}

void NewViewLayerButton(int layer) {
    char temp[100];
    Arg args[1], getargs[1];
    XmString string;

    assert(layer < GetLayersNumber());
    string = XmStringCreate(GetLayerName(layer), XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[0], XmNlabelString, string);

    if (layer > maxLayersButtons) {
        sprintf(temp, "LayerViewDialog_Layer%d", layer);
        /*layers[layer] = XmCreateToggleButtonGadget(layer_view_rc, temp, args, 1);*/

        layers[layer] = FetchWidget(layer_view_rc, "LayerViewDialog_Layer0");
        XtSetValues(layers[layer], args, 1);
        XtAddCallback(layers[layer], XmNvalueChangedCallback, (XtCallbackProc) ViewLayerUpdateViewCB, (XtPointer)((size_t)layer));
        maxLayersButtons = layer;
    }
    else {
        XmString temp;

        XtSetArg(getargs[0], XmNlabelString, &temp);
        XtGetValues(layers[layer], getargs, 1);
        XmStringFree(temp);
        XtSetValues(layers[layer], args, 1);
    }

    XmToggleButtonGadgetSetState(layers[layer], IsLayerVisible(layer), 1);
    XmStringFree(string);
}

void UpdateViewDialog(void) {
    register int i, ln;

    updating = TRUE;
    for (i = 0, ln = GetLayersNumber(); i < ln; i++)
        NewViewLayerButton(i);

    XtManageChildren(layers, ln);

    XtUnmanageChildren(layers + ln, maxLayersButtons - ln + 1);

    updating = FALSE;
}

void ReinitializeViewDialog(void) {
    updating = TRUE;

    if (maxLayersButtons > 0)
        XtUnmanageChildren(layers + 1, maxLayersButtons);

    ReallocViewLayerList();
    ResetAllLayersInViewLayerList();
    SetLayer2ViewLayerList(WHOLE_NET_LAYER);

    XmToggleButtonGadgetSetState(layers[WHOLE_NET_LAYER], TRUE, TRUE);

    updating = FALSE;
}

void RegisterViewLayerDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"ViewLayerDoneCB"	, (XtPointer)ViewLayerDoneCB},
        {"ViewLayerEditCB"		, (XtPointer)ViewLayerEditCB},
        {"ViewLayerSelectAllCB"			, (XtPointer)ViewLayerSelectAllCB},
        {"ViewLayerDeselectAllCB"		, (XtPointer)ViewLayerDeselectAllCB}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterEditLayerDialog names\n");
}
void InitViewLayerDialog(void) {
    /*
    layer_view_dlg = XmCreateFormDialog(frame_w, "LayerViewDialog", NULL, 0);
    layer_view_list = XmCreateScrolledWindow(layer_view_dlg, "LayerViewDialog_Scroller", NULL, 0);
    XtManageChild(layer_view_list);
    layer_view_rc = XmCreateRowColumn(layer_view_list, "LayerViewDialog_RC", NULL, 0);
    XtManageChild(layer_view_rc);
    layer_view_done = XmCreatePushButton(layer_view_dlg, "LayerViewDialog_Done", NULL, 0);
    XtManageChild(layer_view_done);
    layer_view_edit = XmCreatePushButton(layer_view_dlg, "LayerViewDialog_Edit", NULL, 0);
    XtManageChild(layer_view_edit);
    layer_view_selectall = XmCreatePushButton(layer_view_dlg, "LayerViewDialog_SelectAll", NULL, 0);
    XtManageChild(layer_view_selectall);
    layer_view_deselectall = XmCreatePushButton(layer_view_dlg, "LayerViewDialog_DeselectAll", NULL, 0);
    XtManageChild(layer_view_deselectall);
    */
    updating = FALSE;
    limit2ReallocViewButtons = 1;

    layer_view_dlg = XtNameToWidget(frame_w, "*LayerViewDialog");
    layer_view_rc = XtNameToWidget(layer_view_dlg, "*.LayerViewDialog_RC");
    maxLayersButtons = -1;

    layers = (Widget *) calloc(LAYERS_PER_ELEMENT, sizeof(Widget));
    NewViewLayerButton(WHOLE_NET_LAYER);
    XtManageChild(layers[WHOLE_NET_LAYER]);
    /*
    XtAddCallback(layer_view_done, XmNactivateCallback, ViewLayerDoneCB, NULL);
    XtAddCallback(layer_view_edit, XmNactivateCallback, ViewLayerEditCB, NULL);
    XtAddCallback(layer_view_selectall, XmNactivateCallback, ViewLayerSelectAllCB, NULL);
    XtAddCallback(layer_view_deselectall, XmNactivateCallback, ViewLayerDeselectAllCB, NULL);
    */

    UpdateViewDialog();


}

void UpdateLayerDialogs(void) {
    UpdateViewDialog();
    UpdateEditDialog();
}





