#define ___MENUCALLBACKS___

#include "global.h"
#include "menuvalues.h"
#include "grid.h"
#include "menuH.h"
#include "aboutdialog.h"
#include "overview.h"
#include "showgdi.h"
#include "layereditdialog.h"
#include "layerviewdialog.h"
#include "commentdialog.h"
#include "printdialog.h"
#include "optionsdialog.h"
#include "printarea.h"
#include "filer.h"
#include "swn.h"
#include "Canvas.h"
#include "command.h"
#include "afire.h"
#include "select.h"
#include "unfold.h"
#include <Mrm/MrmPublic.h>

Boolean redrawPending;
extern int blink_on;
extern Widget mainwin, popup, drawwin;
extern void RemoveResProc();

static void AboutCB(Widget w, XtPointer closure, XtPointer call_data) {
    ShowAboutDialog();
}


static void ViewModeCB(Widget w, int closure, XmToggleButtonCallbackStruct *str) {
    static int overview_visible = FALSE;

    switch (closure) {
    case VIEW_PRINTAREA_MITEM:
        SetPrintAreaVisible(!IsPrintAreaVisible());
        break;
    case VIEW_SPLINE_MITEM:
        spline_on = !spline_on;
        break;
    case VIEW_TAG_MITEM:
        tag_visible = !tag_visible;
        break;
    case VIEW_RATE_MITEM:
        rate_visible = !rate_visible;
        break;
    case VIEW_OVERVIEW_MITEM:
        overview_visible = !overview_visible;
        if (overview_visible)
            ShowOverviewDialog();
        else
            HideOverviewDialog();
    }
    if (closure != VIEW_OVERVIEW_MITEM) {
        gdiClearDrawingArea();
        redisplay_canvas();
    }
}


static void ViewMenuCB(Widget w, int closure, XtPointer call_data) {
    switch (closure) {
    case VIEW_LAYERS_MITEM:
        ShowViewLayerDialog();
        break;
    case VIEW_SEARCH_MITEM:
        break;
    }
}


static void NetMenuCB(Widget w, int closure, XtPointer call_data) {
    switch (closure) {
    case NET_STRUCT_PINV_MITEM:
        MainSolve(0);
        break;
    case NET_STRUCT_TINV_MITEM:
        MainSolve(1);
        break;
    case NET_STRUCT_MINDEAD_MITEM:
        MainSolve(2);
        break;
    case NET_STRUCT_MINTRAP_MITEM:
        MainSolve(3);
        break;
    case NET_STRUCT_ECS_MITEM:
        MainSolve(4);
        break;
    case NET_STRUCT_STRUBOU_MITEM:
        MainSolve(5);
        break;
    case NET_SOLVE_TRG_MITEM:
        MainSolve(6);
        break;
    case NET_SOLVE_EMC_MITEM:
        MainSolve(7);
        break;
    case NET_SOLVE_GSPN_STST_MITEM:
        MainSolve(8);
        break;
    case NET_SOLVE_GSPN_TRAN_MITEM:
        MainSolve(9);
        break;
    case NET_SIMULATION_MITEM:
        open_f_proc();
    }
}

#include "calc.h"

static void SwnMenuCB(Widget w, int closure, XtPointer call_data) {
    switch (closure) {
    case SWN_SYM_SIMULATION:
        MainSolve(10);
        break;
    case SWN_SYM_COMPUTERG:
        MainSolve(11);
        break;
    case SWN_SYM_TRANSIENT:
        MainSolve(_SWN_SYM_TRANSIENT);
        break;
    case SWN_ORD_SIMULATION:
        MainSolve(12);
        break;
    case SWN_ORD_COMPUTERG:
        MainSolve(13);
        break;
    case SWN_ORD_TRANSIENT:
        MainSolve(_SWN_ORD_TRANSIENT);
        break;
    case SWN_ORD_UNFOLD:
        MainSolve(_SWN_ORD_UNFOLD);
        break;
    /****17/7/08 ESRG by Marco***/
    case SWN_ESRG:
        MainSolve(_SWN_ESRG);
        break;
    case SWN_DSRG:
        MainSolve(_SWN_DSRG);
        break;
    /****17/7/08 ESRG by Marco***/
    /****11/6/10 MDD by Marco***/
    case GSPN_MDD:
        MainSolve(_GSPN_MDD);
        break;
        /****11/6/10 MDD by Marco***/
    }
}

static void EGSPNMenuCB(Widget w, int closure, XtPointer call_data) {
    switch (closure) {
    case EGSPN_SIMULATION:
        MainSolve(_EGSPN_SIMULATION);
        break;
    case EGSPN_COMPUTERG:
        MainSolve(_EGSPN_COMPUTERG);
        break;
    case EGSPN_TRANSIENT:
        MainSolve(_EGSPN_TRANSIENT);
        break;
    }
}

static void FileMenuCB(Widget w, int closure, XtPointer call_data) {
    SetWaitCursor(mainwin);

    switch (closure) {
    case FILE_OPTIONS_MITEM:
        ShowOptionsDialog();
        break;
    case FILE_COMMENT_MITEM:
        ShowCommentDialog(netobj->comment);
        break;
    case FILE_EXIT_MITEM:
        ExitRequest();
        break;
    case FILE_SAVEAS_MITEM:
        SaveAsRequest();
        break;
    case FILE_SAVE_MITEM:
        SaveRequest();
        break;
    case FILE_REMOVE_RESULTS_MITEM:
        RemoveResProc(0);

        break;
    case FILE_REMOVE_ALL_MITEM:
        RemoveResProc(1);
        break;
    case FILE_OPEN_MITEM:
        OpenRequest();
        break;
    case FILE_NEW_MITEM:
        NewRequest();
        break;
    case FILE_PRINT_MITEM:
        ShowPrintDialog();
        break;
    case FILE_MERGE_MITEM:
        MergeRequest();
        break;
    }
    ClearWaitCursor(mainwin);

}


static void EditMenuCB(Widget w, int closure, XtPointer call_data) {

    switch (closure) {
    case EDIT_MIRROR_MITEM:
        mirror_proc();
        break;
    case EDIT_UNDO_MITEM:
        puts("EDIT_UNDO_MITEM");
        restore_proc();
        break;
    case EDIT_CUT_MITEM:
        break;
    case EDIT_PASTE_MITEM:
        reset_object();
        paste_proc();
        break;
    case EDIT_MOVE_MITEM:
        break;
    case EDIT_LAYERS_MITEM:
        ShowEditLayerDialog();

        break;

    }
}

static void RedrawDrawingAreaCB(Widget w, XtPointer closure, XmDrawingAreaCallbackStruct *str) {
    if (((XExposeEvent *)str->event)->count != 0)
        return;
    if (blink_on) {
        redrawPending = TRUE;
        return;
    }
    redisplay_canvas();
    redraw_selected();
}


static void ClearStatusCB(Widget w, XtPointer closure, XtPointer str) {
    StatusDisplay("");
}


static void FileHelpMenuCB(Widget w, int closure, XtPointer call_data) {

    static char *FileMessage[] = {
        "Create a new net from scratch",
        "Open an existing net file",
        "Merge the current net with an existing file",
        "Save current net to file",
        "Save current net to a new file",
        "Remove Result files",
        "Remove THE CURRENT NET and Result files",
        "Comment current net file",
        "Modify user Options",
        "Print current net",
        "Exit from GreatSPN"
    };

    StatusDisplay(FileMessage[closure]);
}


static void EditHelpMenuCB(Widget w, int closure, XtPointer call_data) {
    static char *EditMessage[] = {

        "Undo previous action",
        "Clonate Selection",
        "Delete Selection",
        "Cancel Selection",
        "Move Selection",
        "Modify selection",
        "Show Edit Layers Dialog",
        "Rotate Selection",
        "X Axis Flip selection",
        "Y Axis Flip selection",
        "Mirror selection",
    };

    StatusDisplay(EditMessage[closure]);

}



static void NetHelpMenuCB(Widget w, int closure, XtPointer call_data) {
    static char *NetMessage[] = {

        "Compute Place Invariants",
        "Compute Transition Invariants",
        "Compute Minimal Deadlocks",
        "Compute Minimal Traps",
        "Compute ECS for GSPN",
        "Check Structural Boundedness",
        "Compute Tangible Reachability Graph for GSPN",
        "Compute Embedded Markov Chain for GSPN",
        "Steady-state Solution for GSPN",
        "Transient Solution for GSPN",
        "Net Simulation",
    };

    StatusDisplay(NetMessage[closure]);

}


static void GridHelpMenuCB(Widget w, XtPointer closure, XtPointer call_data) {
    static char *NetMessage[] = {

        "Hide/Show/Size Grid"

    };

    StatusDisplay(NetMessage[0]);
}

static void ZoomHelpMenuCB(Widget w, XtPointer closure, XtPointer call_data) {
    static char *NetMessage[] = {

        "Set Zoom Level"

    };

    StatusDisplay(NetMessage[0]);

}


static void RescaleHelpMenuCB(Widget w, XtPointer closure, XtPointer call_data) {
    static char *NetMessage[] = {

        "Rescale Net",

    };

    StatusDisplay(NetMessage[0]);

}

static void ViewHelpMenuCB(Widget w, int closure, XtPointer call_data) {
    static char *ViewMessage[] = {

        "Show Spline arcs",
        "Show Tags",
        "Show Rates",
        "Show/Hide Net Overview",
        "Not yet Implemented",   /* "Search items",*/
        "Show/Hide Print Area",
        "Show View Layers Dialog"
    };

    StatusDisplay(ViewMessage[closure]);
}

static void SwnHelpMenuCB(Widget w, int closure, XtPointer call_data) {
    static char *HelpMessage[] = {

        "Start Symbolic Simulation",
        "Compute Symbolic Reachability Graph",
        "Compute Symbolic Trasient Solution",
        "Start Ordinary Simulation",
        "Compute Ordinary Reachability Graph",
        "Compute Ordinary Trasient Solution",
        "Compute Ordinary Unfolded Net"
    };

    StatusDisplay(HelpMessage[closure]);
}
static void EGSPNHelpMenuCB(Widget w, int closure, XtPointer call_data) {
    static char *HelpMessage[] = {

        "Start Simulation",
        "Compute Reachability Graph",
        "Compute Transient Solution"
    };

    StatusDisplay(HelpMessage[closure]);
}

static void HelpHelpMenuCB(Widget w, int closure, XtPointer call_data) {
    static char *HelpMessage[] = {

        "Hide/Show Mouse Help",
        "Show About Dialog"

    };

    StatusDisplay(HelpMessage[closure]);
}



void RegisterMenuCallbacks(void) {
    static MrmRegisterArg	regvec[] = {
        {"ViewModeCB"			, (XtPointer)ViewModeCB},
        {"ViewMenuCB"			, (XtPointer)ViewMenuCB},
        {"AboutCB"				, (XtPointer)AboutCB},
        {"NetMenuCB"			, (XtPointer)NetMenuCB},
        {"FileMenuCB"			, (XtPointer)FileMenuCB},
        {"RedrawDrawingAreaCB"	, (XtPointer)RedrawDrawingAreaCB},
        {"ClearStatusCB"		, (XtPointer)ClearStatusCB},
        {"EditHelpMenuCB"		, (XtPointer)EditHelpMenuCB},
        {"NetHelpMenuCB"		, (XtPointer)NetHelpMenuCB},
        {"SwnHelpMenuCB"		, (XtPointer)SwnHelpMenuCB},
        {"GridHelpMenuCB"		, (XtPointer)GridHelpMenuCB},
        {"ZoomHelpMenuCB"		, (XtPointer)ZoomHelpMenuCB},
        {"RescaleHelpMenuCB"	, (XtPointer)RescaleHelpMenuCB},
        {"ViewHelpMenuCB"		, (XtPointer)ViewHelpMenuCB},
        {"HelpHelpMenuCB"		, (XtPointer)HelpHelpMenuCB},
        {"EditMenuCB"			, (XtPointer)EditMenuCB},
        {"SwnMenuCB"			, (XtPointer)SwnMenuCB},
        {"FileHelpMenuCB"		, (XtPointer)FileHelpMenuCB},
        {"EGSPNMenuCB"			, (XtPointer)EGSPNMenuCB},
        {"EGSPNHelpMenuCB"		, (XtPointer)EGSPNHelpMenuCB}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterMenuCallbacks names\n");
}
